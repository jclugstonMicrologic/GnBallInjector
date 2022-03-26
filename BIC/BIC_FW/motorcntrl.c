
/*************************************************************************/
/* "I N C L U D E   F I L E S                                            */  
/*************************************************************************/
#include<mk_std.h>
#include<stdlib.h>
#include"lcdFd.h"  
#include"motorcntrl.h"
#include"timer.h"
#include "PwmDriverFd.h"
#include "String.h"

/*************************************************************************/
/* D E F I N I T I O N S                                                 */
/*************************************************************************/ 
#define DIAGNOSTIC

enum
{ 
   MOTOR_IDLE_STATE =0,
   MOTOR_RUN_STATE,   
   MOTOR_WAIT_STATE,   
   MOTOR_UNJAM_STATE,
   MOTOR_JAM_STATE,
   MOTOR_STARTUP_DELAY_STATE,
   
   MOTOR_LAST
};

/*************************************************************************/
/* G L O B A L   D A T A                                                 */
/*************************************************************************/

UINT32 TheTimer;
UINT32 WaitTimer;
UINT32 MotorStartTime;

BALL_COUNT BallCount;

MOTOR Motor;
MOTOR_CONTROL MotorControl;
MOTOR_PARAMETERS *ModeSelect;

INT16 *BallCntSelect;
INT16 *BallIndexSelect;

volatile INT16 BallCounter;

typedef struct
{
   float dState;
   float iState;

   float iMax;
   float iMin;

   float iGain;  // integral gain
   float pGain;  // proportional gain
   float dGain;  // derivative gain

}BIC_PID;

BIC_PID BicPid;

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/ 
EDGES_DETECTED Edge;
VANES_DETECTED Vane;


/*************************************************************************/
/* P R O T O T Y P E S                                                   */
/*************************************************************************/
void SensorIsr(void);
BOOL MotorCheckStartConditions(MOTOR_PARAMETERS *, UINT8 mode);

void InitPid(void);
float UpdatePid( BIC_PID *pidPtr_, float error_, float position_);


/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: motorStrtUpInit
  |  Description: 
  |------------------------------------------------------------------------
*/         
void MotorStrtUpInit
(
   void
)
{
   // open external IRQ0 interrupt
   #define IRQ0_INTERRUPT_LEVEL	7
   AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_IRQ0, IRQ0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, SensorIsr);
   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_IRQ0);

   MotorControl.machState =MOTOR_IDLE_STATE;
   MotorControl.startRequested =0;   
   MotorControl.speed =0xffff;
   MotorControl.nbrVanes =0;   
   MotorControl.indexing =false;
//   MotorControl.allowRev =false;
      
   MotorControl.indexState =INDEX_IDLE_STATE;
            
   OsTimer[TIMER_JAM_BUZZER].callBackPtr =BuzzerToggle;        
    
   Motor.injection.rate =100;
   Motor.load.rate =30;
   Motor.index.rate =150;
        
   BallCount.load =0;
   BallCount.index =MAX_BALL_COUNT;
   BallCount.increment =true;   
   
   BallCounter =0;   
   
   InitPid();
   
}      


/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: MotorStart
  |  Description: 
  |------------------------------------------------------------------------
*/         
void MotorStart
(
   UINT8 rate_
)
{
   WaitTimer =15000/rate_;

   MotorControl.edgeDetect =0;

   MotorControl.timeStamp =TicksLeft;
   
   MotorControl.nbrVanes =0;   
   
   MotorStartTime =TicksLeft;
   
   // start motor
   AT91F_PIO_SetOutput( AT91C_BASE_PIOA, PWM_SPEED_DRIVE );   
   
   DisplayBallsPerMin =rate_;   
}

/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: MotorStop
  |  Description: 
  |------------------------------------------------------------------------
*/         
void MotorStop
(
   void
)
{
   // stop motor
   AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, PWM_SPEED_DRIVE );  
   MOTOR_DIR_BRAKE;  
   OsStopTimer( &OsTimer[TIMER_JAM_BUZZER] );
   MotorControl.ballsPerMin =0;
   ModeSelect=NULL_PTR;
   MotorControl.machState =MOTOR_IDLE_STATE;
}


/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: GetJamRate
  |  Description: 
  |------------------------------------------------------------------------
*/         
UINT16 GetJamRate
(
   UINT8 rate_
)
{
   return 30000/(rate_+1);    
}

/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: SetDirection
  |  Description: 
  |------------------------------------------------------------------------
*/         
void SetDirection
(
   UINT8 dir_
)
{  
   if( dir_ ==MOTOR_FWD )
   {
      // set hw for forward direction
      MOTOR_DIR_FWD;
   }
   else
   {
      MOTOR_DIR_REV;
   }
}


/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: AdjustPidCoeff
  |  Description: 
  |------------------------------------------------------------------------
*/         
float AdjustPidCoeff 
(
   UINT8 rate_
)
{
   if(rate_ <15 )                  return 100.0;                   
   else if(ModeSelect->rate <35 )  return 10.0;
   else if(ModeSelect->rate <60 )  return 5.0;
   else if(ModeSelect->rate <120 ) return 2.0;
   else if(ModeSelect->rate <200 ) return 1.0;
   else                            return 0.2; 
}

/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: HandleBallCounter
  |  Description: 
  |------------------------------------------------------------------------
*/         
BOOL HandleBallCounter
(
   UINT8 mode_,
   UINT8 dir_
)
{
   if( !BallCounter )
      return false;
     
   if( BallCount.increment )   
   {
      if( dir_ ==MOTOR_FWD)
         *BallCntSelect +=BallCounter;             
      else
         *BallCntSelect -=BallCounter; 
      
      BallCount.index =(MAX_BALL_COUNT - *BallCntSelect);
   }
   else
   {
      if( dir_ ==MOTOR_FWD)
      {
         // indexing, decrement the count
         *BallCntSelect -=BallCounter;   
      }
      else
         *BallCntSelect +=BallCounter;       
   }
    
   if( BallCounter || mode_ ==MODE_JOG )  
   {
      BallCounter =0;      
      
      BUZZER_ON;      
      OsStartOneShotTimer( &OsTimer[TIMER_KEYPAD_BUZZER], 75 ); 
      
      if( mode_ !=MODE_TEST )
      {
         if( mode_ ==MODE_MAN ||
             dir_ ==MOTOR_REV
           ) //manual
         {
            ModeSelect->start =0;
            MotorStop();
            
            if( *BallCntSelect <0 ) // wrapped via decrement (man mode), set to 0
               *BallCntSelect =0; 
            
            if( *BallCntSelect >=MAX_BALL_COUNT )
               *BallCntSelect =MAX_BALL_COUNT;
                             
            BallCounter =0; 
            return true;
         }
      }                   
   }
 
   if( *BallCntSelect <0 )
   {
      // went negative
      *BallCntSelect =0;
      
      if( mode_ !=MODE_TEST )
      {
         ModeSelect->start =0;
         MotorStop();
      }
                   
      BallCounter =0; 
      return true;      
   }
  
  
   if( mode_ ==MODE_TEST )
   {
      return false;
   }   

   if( !BallCount.increment && *BallCntSelect ==0 )
   {
      // prevent motor from moving  
      ModeSelect->start =0;
      MotorStop();
      
      MotorControl.indexing =false;
      MotorControl.indexState =INDEX_COMPLETE_STATE;
      
      return true;
   }                   
   else if( BallCount.increment && *BallCntSelect >=MAX_BALL_COUNT )
   {
      // prevent motor from moving  
      MotorControl.indexing =false;
      ModeSelect->start =0;
      MotorStop();
      
      *BallCntSelect =MAX_BALL_COUNT;         
      return true;
   }               
   else
      return false;
}


/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: MotorRunMachine
  |  Description: 
  |------------------------------------------------------------------------
*/         
void MotorRunMachine
(
   void
)
{         
   static UINT8 mode;
   static UINT16 prevVaneCnt =0;
   static UINT8 JamFailCounter =0;
   static UINT8 PrevRate =0;   
   static UINT8 SubState =0;   
   static UINT32 RelayEngageTimer =0;      
   static UINT8 PrevMotorDir =0;
   UINT16 vanes;
   float ballsPerMin;     
   float rateError;
   INT16 error =0;

   switch( MotorControl.machState ) 
   {
      case MOTOR_IDLE_STATE:
         if( Motor.injection.start )
         {           
            mode =MODE_INJECT;
            BallCount.increment =false;            
            ModeSelect = &Motor.injection; 
            
            if( MotorControl.indexing )
            {
               BallCntSelect = &BallCount.index;
            }
            else
            {
               BallCntSelect = &BallCount.load;
            }
         }
         else if( Motor.load.start )
         {           
            mode =MODE_LOAD;
            BallCount.increment =true;               
            ModeSelect = &Motor.load; 
            BallCntSelect = &BallCount.load;               
         }
         else if( Motor.index.start )           
         {
            mode =MODE_INDEX;           
            BallCount.increment =false;            
            ModeSelect = &Motor.index; 
            BallCntSelect = &BallCount.index;            
         }
         else if( Motor.test.start )
         {
            mode =MODE_TEST;           
            ModeSelect = &Motor.test; 
         }
         else if( Motor.manual.start )
         {
            mode =MODE_MAN;           
            ModeSelect = &Motor.manual; 

            #ifdef BALL_DROPPER               
            BallCount.increment =true;               
            BallCntSelect = &BallCount.load; 
            #endif
         }
         else if( Motor.jog.start )
         {
            mode =MODE_JOG;           
            ModeSelect = &Motor.jog; 
         }
         
         if( ModeSelect !=NULL_PTR )
         {
            // check start conditions (each MotorStartRequest has different conditions)                     
            if( !MotorCheckStartConditions(ModeSelect, mode) )
            {
               ModeSelect->start =0;
               return;
            }
            // if pass then set direction, and move to next state           
            SetDirection(ModeSelect->direction);
              
            prevVaneCnt =0;
                             
            TheTimer =TicksLeft;
         
            MotorControl.machState =MOTOR_STARTUP_DELAY_STATE; //MOTOR_WAIT_STATE; //MOTOR_RUN_STATE;
         }
         break;
      case MOTOR_STARTUP_DELAY_STATE:         
         MotorStart( ModeSelect->rate );

         TheTimer =TicksLeft;

         SubState =0;
         
         MotorControl.machState =MOTOR_WAIT_STATE;
         break;
      case MOTOR_RUN_STATE:
         if( MotorControl.edgeDetect ==1 )
         {
            prevVaneCnt =MotorControl.nbrVanes;           
                       
            MotorControl.edgeDetect =0;            
            MotorControl.timeStamp =TicksLeft;

            if( ModeSelect->start !=0 )
            {
               SetDirection(ModeSelect->direction);         
               // start motor again
               AT91F_PIO_SetOutput( AT91C_BASE_PIOA, PWM_SPEED_DRIVE );
            }
           
            JamFailCounter =0;
            
            SubState =0;
         }

         if( ModeSelect->start ==0 )
         {
            // stop motor
            MotorStop();
         }
         else
         {
            MotorControl.machState =MOTOR_WAIT_STATE;                       
            TheTimer =TicksLeft;
         }
         break;         
      case MOTOR_WAIT_STATE:
         if( ModeSelect->rate !=0 )
         {        
            if( MotorControl.edgeDetect !=1 )
            {
               if( ModeSelect->start ==0 ) // user hit stop              
                  MotorControl.machState =MOTOR_RUN_STATE;
               
#ifdef jeff               
               // this mimics the jog function
               if( FailCount ++ <10000 )
               {           
                  // if no edge, there is an issue break from here
                  // after 10000 loop counts
                  return;
               }                             
#endif
            }
//            FailCount =0;            
            
            if( HandleBallCounter( mode, ModeSelect->direction ) )
            {
               // ball count limits met
               // motor stopped              
               MotorControl.machState =MOTOR_IDLE_STATE;
               return;
            }

            if( PrevRate !=ModeSelect->rate ) 
            {
               // if the rate is being changed, reset waitTimer
               WaitTimer =15000/ModeSelect->rate;            
               
              // this stops ball counting, very bad when holding rate adjust button               
              // remove it
 //              MotorControl.nbrVanes =1; 
               MotorStartTime =TicksLeft;
               
               MotorControl.ballsPerMin =ModeSelect->rate;
            }
          
            PrevRate =ModeSelect->rate;
            
            if( MotorControl.edgeDetect ==1 )            
            {
               // edge detected
               switch( SubState )
               {
                  case 0:
                     // turn off power to motor
                     AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, PWM_SPEED_DRIVE );                                   
                     
                     #ifdef BALL_DROPPER
                     MOTOR_DIR_BRAKE;
                     #endif
                     SubState ++;
                     break;
                  case 1:    
                     if( (TicksLeft -MotorControl.timeStamp) >35 )
                     {
                        // current should have dropped, brake the motor
                        MOTOR_DIR_BRAKE;                                    
                        
                        RelayEngageTimer =TicksLeft; 
                        SubState ++;
                     }
                     break;                    
                  case 2:
                     if( (TicksLeft -RelayEngageTimer) >5 )
                     {
                        // energize relays here
                        SetDirection(ModeSelect->direction);
                        
                        SubState ++;
                     }
                     break;
                  case 3:
                     // nothing to do until motor starts again                     
                     break;
               }               
            }

            if( ( (TicksLeft-TheTimer)>WaitTimer && MotorControl.edgeDetect) 
                           &&
                  SubState !=2 // motor has been braked, cannot continue on until brake timeout expires
              ) 
            {
               // energize relays here in case we need to move on before
               // RelayEngageTimer expires
               SetDirection(ModeSelect->direction);
               
               vanes=(MotorControl.nbrVanes-prevVaneCnt);           
               
               ballsPerMin =(float)( (float)(vanes*60000)/WaitTimer )/MotorControl.vanesPerBall;

               DisplayBallsPerMin =(float)( (float)(MotorControl.nbrVanes*60000)/(TicksLeft -MotorStartTime) )/MotorControl.vanesPerBall;
   
               ballsPerMin =DisplayBallsPerMin;
   
               if( ballsPerMin >250 )
               {
                  // absolute max, prevent unreasonable values
                  ballsPerMin =ModeSelect->rate;
                  DisplayBallsPerMin =ModeSelect->rate;
               }

               MotorControl.ballsPerMin =ballsPerMin;
               
               rateError =DisplayBallsPerMin -ModeSelect->rate;
               
               if( (rateError < 10 && 
                   rateError > -10)
                       ||
                   (rateError < 25  || 
                   rateError > -25)
                  
                 )
               {
                  DisplayBallsPerMin =ModeSelect->rate;
               }
                                                   
               BicPid.pGain =AdjustPidCoeff(ModeSelect->rate);
               
               // use a PID control
//               WaitTimer +=UpdatePid( &BicPid, (MotorControl.ballsPerMin-ModeSelect->rate), MotorControl.ballsPerMin);
                                           
               error =(ModeSelect->rate-MotorControl.ballsPerMin);
               
               // keep error within reason
               if( error > 10 )
               {
                  error =10;
               }
               else if( error< -10 )
               {
                  error = -10;
               }           
               
               WaitTimer =15000/(ModeSelect->rate + error );
               
               if( mode ==MODE_JOG )
               {
                  ModeSelect->start =0;
                  MotorControl.machState =MOTOR_IDLE_STATE;  
               }
               else
                  MotorControl.machState =MOTOR_RUN_STATE;           
            }

            // timeout is based on motor rate
            if( TicksLeft -MotorControl.timeStamp >GetJamRate(ModeSelect->rate) ) 
            {
               // try to give motor a kick
               AT91F_PIO_SetOutput( AT91C_BASE_PIOA, PWM_SPEED_DRIVE );              
               if( JamFailCounter ++ >3 )
               {               
                  // we have not seen a pulse
                  MotorControl.ballsPerMin =0;
                  MotorControl.speed =TIMER_CLOCK;
          
                  PrevMotorDir =ModeSelect->direction;
                  ModeSelect->direction =MOTOR_JAM;               
                
                  OsStartPeriodicTimer( &OsTimer[TIMER_JAM_BUZZER], 200);
                  MotorControl.machState =MOTOR_JAM_STATE;           
                  
                  JamFailCounter =0;
                  
                  AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, PWM_SPEED_DRIVE );  
                  MOTOR_DIR_BRAKE;  
               }
               
               MotorControl.timeStamp =TicksLeft;
            }                       
         }
         break;
      case MOTOR_UNJAM_STATE:
         if( mode ==1 )
         {
            // only mode attempt to unjam       
            // try to unjam the motor
            if( ModeSelect->start ==0 )
            {
               ModeSelect->direction =MOTOR_FWD;
               MotorStop();
            }        
         }
         else
         {          
            MotorControl.machState =MOTOR_JAM_STATE;
         }
         break;                  
      case MOTOR_JAM_STATE:
         // motor is jammed
         if( ModeSelect->start ==0 ) // user has selected stop
         {
            ModeSelect->direction =PrevMotorDir;
            MotorStop();
         }
         break;                           
   }
}


/*|------------------------------------------------------------------------
  |  Module: motorcntrl.c
  |  Routine: SensorIsr
  |  Description: interrupt for the input sensor (any edge detect)
  |  determines when a vane has passed based on the balls/rev and
  |  number of vanes selected by the user 
  |------------------------------------------------------------------------
*/   
void SensorIsr
(
   void
)
{ 
   /***********************/
   /* L O C A L   D A T A */
   /***********************/
   
   int dummy;

   int aicMaskReg;
   UINT32 intStatus;
   
   /***********************/
   /* C O D E             */
   /***********************/
   
   intStatus =AT91C_BASE_PIOA->PIO_ISR;

   // get all enabled interrupts
   aicMaskReg =AT91C_BASE_AIC->AIC_IMR;
   // disable all interrupts
   AT91C_BASE_AIC->AIC_IDCR =0xffffffff;

   // enable the next PIO IRQ
   dummy =AT91C_BASE_PIOA->PIO_ISR;

   // suppress the compilation warning
   dummy =dummy;

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =aicMaskReg;
   
} /* end sensorIsr() */



/*
*|----------------------------------------------------------------------------
*|  Module:  motorcntrl.c
*|  Routine: MotorCheckStartConditions
*|  Description: 
*|----------------------------------------------------------------------------
*/
BOOL MotorCheckStartConditions
(
   MOTOR_PARAMETERS *theMode,
   UINT8 mode
)
{  
   switch(mode)
   {
      case MODE_INJECT:
         if( BallCount.load ==0 )
         {
            theMode->direction =MOTOR_FWD;
            return true;
         }           
         break;
      case MODE_LOAD: //load
         if( BallCount.load ==MAX_BALL_COUNT && theMode->direction ==MOTOR_FWD )                   
         {   
            MotorControl.indexing =false;
            return true; //false;                      
         }
         if( BallCount.load ==0 && theMode->direction ==MOTOR_REV ) 
            return false;
         break;
      case MODE_INDEX: //index
         if( BallCount.index ==MAX_BALL_COUNT && theMode->direction ==MOTOR_REV ) 
            return false;        
         break;         
      case MODE_TEST: //test
         return true;
      case MODE_MAN: //man, allow motor to move no matter what
         return true;            
   }

// prevent motor from moving   
//   if( theMode->direction ==MOTOR_REV && (BallCount.load + BallCount.index) ==MAX_BALL_COUNT )    
//      return false;

   if( theMode->rate ==0 )
      return false;
   
   return true;
}


void InitPid
(
   void
)
{
   BicPid.pGain =1.00;
   BicPid.iGain =0.00;
   BicPid.dGain =0.00;
}



/*
*|----------------------------------------------------------------------------
*|  Module: PID Module
*|  Routine: UpdatePid
*|  Description: PID control algorithm
*|----------------------------------------------------------------------------
*/
float UpdatePid
(
   BIC_PID *pidPtr_,
   float error_,
   float position_
)
{
   float pTerm;
   float dTerm;
   float iTerm;


   // ensure max term is related to iGain
   pidPtr_->iMax =100/pidPtr_->iGain;

   // calculate the proportional term
   pTerm =pidPtr_->pGain * error_;

   // calculate the integral state with appropriate limiting
   pidPtr_->iState +=error_;

   if( pidPtr_->iState > pidPtr_->iMax )
   {
      pidPtr_->iState =pidPtr_->iMax;
   }
   else if( pidPtr_->iState < pidPtr_->iMin )
   {
      pidPtr_->iState =pidPtr_->iMin;
   }

   // calculate the integral term
   iTerm =pidPtr_->iGain * pidPtr_->iState;

   // calculate the derivative term
   dTerm =pidPtr_->dGain * (position_ - pidPtr_->dState);
   pidPtr_->dState =position_;

   // return drive value
   return pTerm + iTerm - dTerm;

} // end UpdatePid()



