
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

UINT8 *BallCntSelect;
UINT8 *BallIndexSelect;

UINT8 BallCounter;

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
   
   OsTimer[TIMER_JAM_BUZZER].callBackPtr =BuzzerToggle;        
    
   Motor.injection.rate =100;
   Motor.load.rate =30;
   Motor.index.rate =150;
      
   // from internal NV memory (flash address 0x20000)
//   MotorControl.ballsPerRev =8;   
//   MotorControl.vanesPerBall =4;
   
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
   if( BallCount.increment )   
   {
      if( dir_ ==MOTOR_FWD)
         *BallCntSelect +=BallCounter;             
      else
         *BallCntSelect -=BallCounter; 
      
// not sure either this     
//      BallCount.index -=BallCounter;
//or this
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


#define ENABLE_BALL_COUNT_BUZZER   
#ifdef ENABLE_BALL_COUNT_BUZZER   
   if(BallCounter)  
   {
      BUZZER_ON;      
      OsStartOneShotTimer( &OsTimer[TIMER_KEYPAD_BUZZER], 75 ); 
      
      if( mode_ !=4 )
      {
         if( mode_ ==5 ||
             dir_ ==MOTOR_REV
           ) //manual
         {
            ModeSelect->start =0;
            MotorStop();
      
            BallCounter =0; 
            return true;
         }
      }
   }
#endif   
   
   BallCounter =0;  

   if( !BallCount.increment && *BallCntSelect ==0 )
   {
      ModeSelect->start =0;
      MotorStop();
      
      MotorControl.indexing =false;
      
      return true;
   }                   
   else if( BallCount.increment && *BallCntSelect ==MAX_BALL_COUNT )
   {
      MotorControl.indexing =false;
      ModeSelect->start =0;
      MotorStop();
      
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
   UINT16 vanes;
   float ballsPerMin;     
   float rateError;

   switch( MotorControl.machState ) 
   {
      case MOTOR_IDLE_STATE:
         if( Motor.injection.start )
         {           
            mode =1;
            ModeSelect = &Motor.injection; 
            
            BallCount.increment =false;
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
            mode =2;
            BallCount.increment =true;               
            ModeSelect = &Motor.load; 
            BallCntSelect = &BallCount.load;               
         }
         else if( Motor.index.start )           
         {
            mode =3;           
            BallCount.increment =false;            
            ModeSelect = &Motor.index; 
            BallCntSelect = &BallCount.index;            
         }
         else if( Motor.test.start )
         {
            mode =4;           
            ModeSelect = &Motor.test; 
         }
         else if( Motor.manual.start )
         {
            mode =5;           
            ModeSelect = &Motor.manual; 
         }
         
         if( ModeSelect !=NULL_PTR )
         {
            // check start conditions (each MotoStartRequest has different conditions)                     
            if( !MotorCheckStartConditions(ModeSelect, mode) )
            {
               ModeSelect->start =0;
               return;
            }
            // if pass then set direction, and move to next state           
            SetDirection(ModeSelect->direction);
              
            prevVaneCnt =0;
            
            MotorStart( ModeSelect->rate );

            TheTimer =TicksLeft;
            
            MotorControl.machState =MOTOR_WAIT_STATE; //MOTOR_RUN_STATE;
         }
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
            if( HandleBallCounter( mode, ModeSelect->direction ) )
            {
               // ball count limits met
               // motor stopped
               return;
            }

            if( PrevRate !=ModeSelect->rate ) 
            {
               // if the rate is being changed, reset waitTimer
               WaitTimer =15000/ModeSelect->rate;            
               
               MotorControl.nbrVanes =1;
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
               
//               rateError =MotorControl.ballsPerMin -ModeSelect->rate;
               rateError =DisplayBallsPerMin -ModeSelect->rate;
               
               if( (rateError < 10 && 
                   rateError > -10)
                       ||
                   (rateError < 25  || 
                   rateError > -25)
                  
                 )
               {
//                  MotorControl.ballsPerMin =ModeSelect->rate;
                  DisplayBallsPerMin =ModeSelect->rate;
               }
                                                   
               BicPid.pGain =AdjustPidCoeff(ModeSelect->rate);
               
#ifdef jeff              
               if( ModeSelect->rate<20 ||
                   ModeSelect->rate>150
                 )
               {
                  // if low speed, no PID control, use a specific on/off timer
                  if( MotorControl.ballsPerRev ==8 )
                    WaitTimer =15000/ModeSelect->rate;
                  else
                    WaitTimer =7500/ModeSelect->rate;                    
               }
               else
#endif                 
               {                   
                  // use a PID control
                  WaitTimer +=UpdatePid( &BicPid, (MotorControl.ballsPerMin-ModeSelect->rate), MotorControl.ballsPerMin);
               } 
                             
               if( WaitTimer ==0 )
               {
                  // absolute min, use a specific on/off timer
                  WaitTimer =15000/ModeSelect->rate;
               }
               
               MotorControl.machState =MOTOR_RUN_STATE;           
            }

            // timeout is based on motor rate
            if( TicksLeft -MotorControl.timeStamp >GetJamRate(ModeSelect->rate) ) 
            {
               if( JamFailCounter ++ >3 )
               {               
                  // we have not seen a pulse
                  MotorControl.ballsPerMin =0;
                  MotorControl.speed =TIMER_CLOCK;
          
                  ModeSelect->direction =MOTOR_JAM;               
                
                  OsStartPeriodicTimer( &OsTimer[TIMER_JAM_BUZZER], 200);
                  MotorControl.machState =MOTOR_UNJAM_STATE;           
                  
                  JamFailCounter =0;
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
         if( ModeSelect->start ==0 )
         {
            ModeSelect->direction =MOTOR_FWD;
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
      case 1:
         break;
      case 2: //load
         if( BallCount.load ==MAX_BALL_COUNT && theMode->direction ==MOTOR_FWD )                   
         {   
            MotorControl.indexing =false;
            return false;                      
         }
         if( BallCount.load ==0 && theMode->direction ==MOTOR_REV ) 
            return false;
         break;
      case 3: //index
         if( BallCount.index ==0 && theMode->direction ==MOTOR_FWD )        
            return false;        
         break;         
      case 4: //test
            return true;
   }
   
   if( theMode->direction ==MOTOR_REV && (BallCount.load + BallCount.index) ==MAX_BALL_COUNT )    
      return false;

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



