/*
*|***************************************************************************
*|PROJECT: GN Ball Injector Controller (BIC)                              
*|Module:   Keypad function driver
*|   NOTES:
*|**************************************************************************
*/
/*
Tools:
   Compiler: IAR Kickstart development environment for AT91SAM7Sxxx
   ***************************************************************************
   Copyright (C) 2001, Micrologic Limited.
   Any information contained in this document which was hitherto unknown by
   the recipient or in which there subsists a proprietary right must not be
   disclosed to any other person, nor used for manufacturing purposes,
   without the permission of Micrologic Limited and the acceptance of this
   document will be construed as acceptance of the foregoing.
   ***************************************************************************
*/


/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include <stddef.h>

#include "flash.h"

#include "LcdFd.h"
#include "keypadFd.h"
#include "Timer.h"
#include "String.h"
#include "motorcntrl.h"


/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/
#define ROW_START (0)
#define ROW_END   (2)

#define NBR_KEYPAD_SWITCHES (16)

#define KEYPAD_UP_BIT_MASK   (0x01)
#define KEYPAD_DOWN_BIT_MASK (0x02)
#define KEYPAD_DUAL_ACK      (0x04)

#pragma segment="BOOTLOADEREND"
#define APP_START_ADDRESS ((void *)0x00020000)
/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
enum
{
   KEYPAD_MAIN_STATE =0,

   KEYPAD_RUN_STATE,
   KEYPAD_LOAD_STATE,
   KEYPAD_SETUP_STATE,

   KEYPAD_RUN_AUTO_DIR_STATE,
   KEYPAD_RUN_AUTO_RATE_STATE,
   
   KEYPAD_RUN_MANUAL_STATE,

   KEYPAD_LOAD_INDEX_STATE,

   KEYPAD_SETUP_CANNISTER_STATE,
   KEYPAD_SETUP_BATTERY_STATE,
     
  
   KEYPAD_TEST_DIR_STATE,
   KEYPAD_TEST_RATE_STATE,   
   
   KEYPAD_DROPPER_STATE,
   
   KEYPAD_LAST_STATE
};

typedef struct
{
   UINT8 currentState;
   UINT8 prevState;
   UINT32 waitTimer;
}KEY_PAD_INFO;

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
UINT8 KeyActionRequest;

/*
*****************************************************************************
 P R I V A T E   G L O B A L   D A T A
*****************************************************************************
*/
KEY_PAD_INFO KeypadInfo;

UINT8 KeyDualAction;

INT8 LcdRow;
INT8 LcdSelection;

MOTOR_CONTROL TempMotorControl;
/*
*****************************************************************************
 P R I V A T E   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/

void MainDisplay(void);

void KeyRunState(int key);
void KeyLoadState(int key);
void KeySetupState(int key);
void KeyRunAutoDirState(int key);
void KeyRunAutoRateState(int key);
void KeyRunManualState(int key);                 
void KeyLoadIndexState(int key_);        
void KeySetupCannisterState(int key_);
void KeySetupBatteryState(int key_);
void KeyTestDirState(int key_);
void KeyTestRateState(int key_);

void KeyDropperState(int key_);

/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/

void *SaveToInternalMemory(void * vStart)
{
   char *pCurrent;
   pCurrent = (char *)vStart;

//   memory_unlock(APP_START_ADDRESS, NULL);  
   flash_write(pCurrent, (char *)&MotorControl);
   
   return pCurrent;  
}

void *ReadFromInternalMemory(void * vStart)
{
   char *pCurrent;
   pCurrent = (char *)vStart;

   memcpy( &MotorControl, (char *)pCurrent, sizeof(TempMotorControl) );   
   
   MotorControl.vanesPerBall =6;
   MotorControl.ballsPerRev =6;   
   
   return pCurrent;  
}

/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: InitKeypad
*|  Description:
*|   Initialisation for this module
*|
*|   *callbackPtr1 =assert, then negate
*|   *callbackPtr2 =held asserted, periodic event
*|   *callbackPtr3 =held asserted
*|   *callbackPtr4 =held asserted, then released
*|----------------------------------------------------------------------------
*/
void InitKeypad
(
   void
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 j;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */


   /*
   *************************
    C O D E
   *************************
   */

   // enable all the keys
   SwitchData[KEY_LEFT].callBackPtr1 =NULL_PTR;
   SwitchData[KEY_LEFT].callBackPtr2 =NULL_PTR;
   SwitchData[KEY_LEFT].callBackPtr3 =KeyRight;
   SwitchData[KEY_LEFT].callBackPtr4 =NULL_PTR;

   SwitchData[KEY_UP].callBackPtr1 =NULL_PTR;
   SwitchData[KEY_UP].callBackPtr2 =KeyUpHeldAssert;
   SwitchData[KEY_UP].callBackPtr3 =KeyUp;
   SwitchData[KEY_UP].callBackPtr4 =NULL_PTR;

   SwitchData[KEY_DOWN].callBackPtr1 =NULL_PTR;
   SwitchData[KEY_DOWN].callBackPtr2 =KeyDownHeldAssert;
   SwitchData[KEY_DOWN].callBackPtr3 =KeyDown;
   SwitchData[KEY_DOWN].callBackPtr4 =NULL_PTR;

   SwitchData[KEY_ENTER].callBackPtr1 =NULL_PTR;
   SwitchData[KEY_ENTER].callBackPtr2 =NULL_PTR;
   SwitchData[KEY_ENTER].callBackPtr3 =KeyEnter;
   SwitchData[KEY_ENTER].callBackPtr4 =NULL_PTR;

   SwitchData[KEY_5].callBackPtr1 =NULL_PTR;
   SwitchData[KEY_5].callBackPtr2 =NULL_PTR;
   SwitchData[KEY_5].callBackPtr3 =Key5;
   SwitchData[KEY_5].callBackPtr4 =NULL_PTR;


   for(j =0; j<MAX_NUM_SWITCHES; j++)
   {
      SwitchData[j].bounce = 0;
      SwitchData[j].start  = 0;
      SwitchData[j].machineState = SWITCH_IDLE_STATE;
   }

   KeypadInfo.currentState =KEYPAD_MAIN_STATE;

   KeyActionRequest =KEY_NONE;

   KeyDualAction =0x00;
   
   OsTimer[TIMER_KEYPAD_BUZZER].callBackPtr =BuzzerOff;  

   
   ReadFromInternalMemory( APP_START_ADDRESS );
         
}/* end InitKeypad */

#define BUZZER_ON_TIME (150)
/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeypadMachine
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
void KeypadMachine
(
   UINT8 key_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   static BOOL LcdOff =false;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */


   /*
   *************************
    C O D E
   *************************
   */  
    
   switch( KeypadInfo.currentState )
   {
      case KEYPAD_MAIN_STATE:

         if( LcdOff && key_ <=KEY_ENTER )
         {
            // any hw key back on
            LcdOff =false;
            DisplayLcdOn();                         
           
            break;
         }
         
         switch( key_ )
         {
            case KEY_LEFT:
               // display/unit on/off
               if( !LcdOff )
               {
                  LcdOff =true; 
                  DisplayLcdOff();
               }
               break;
            case KEY_UP:             
               if( --LcdRow <ROW_START )
               {
                  LcdRow =ROW_END;
               }              
               Display(LcdRow);
               break;
            case KEY_DOWN:
               if( ++LcdRow >ROW_END )
               {
                  LcdRow =ROW_START;
               }                            
               Display(LcdRow);               
               break;
            case KEY_ENTER:
               switch( LcdRow )
               {
                  case 0:
                     #ifndef BALL_DROPPER
                     // run mode
                     KeypadInfo.currentState =KEYPAD_RUN_STATE;
                     LcdMachineState =LCD_RUN_SCREEN;
                     #else 
                     // dropper mode
                     KeypadInfo.currentState =KEYPAD_DROPPER_STATE;
                     LcdMachineState =LCD_DROPPER_SCREEN;                        
                     #endif
                     break;                     
                  case 1:
                     if( MotorControl.indexState !=INDEX_COMPLETE_STATE )
                     {
                        // load mode                    
                        KeypadInfo.currentState =KEYPAD_LOAD_STATE;
                        LcdMachineState =LCD_LOAD_SCREEN;                     
                     }
                     else
                     {
                        // index complete mode
                        KeypadInfo.currentState =KEYPAD_LOAD_INDEX_STATE;
                        LcdMachineState =LCD_LOAD_INDEX_SCREEN;                                                
                     }
                     break;
                  case 2:
                     // setup mode 
                     KeypadInfo.currentState =KEYPAD_SETUP_STATE;
                     LcdMachineState =LCD_SETUP_SCREEN;
                     break;                     
               }
               break;
            case KEY_5:
               break;                             
         }
         break;
      case KEYPAD_RUN_STATE:
         KeyRunState(key_);
         break;
      case KEYPAD_LOAD_STATE:
         KeyLoadState(key_);
         break;
      case KEYPAD_SETUP_STATE:
         KeySetupState(key_);
         break;
      case KEYPAD_RUN_AUTO_DIR_STATE:
         KeyRunAutoDirState(key_);
         break;         
      case KEYPAD_RUN_AUTO_RATE_STATE:
         KeyRunAutoRateState(key_);
         break;
      case KEYPAD_RUN_MANUAL_STATE:
         KeyRunManualState(key_);                         
         break;         
      case KEYPAD_LOAD_INDEX_STATE:
         KeyLoadIndexState(key_);
         break;
      case KEYPAD_SETUP_CANNISTER_STATE:
         KeySetupCannisterState(key_);
         break;
      case KEYPAD_SETUP_BATTERY_STATE:
         KeySetupBatteryState(key_);
         break;         
      case KEYPAD_TEST_DIR_STATE:
         KeyTestDirState(key_);        
         break; 
      case KEYPAD_TEST_RATE_STATE:
         KeyTestRateState(key_);
         break;           
      case KEYPAD_DROPPER_STATE:         
         KeyDropperState(key_);
         break;
   }

   if( KeyActionRequest !=KEY_NONE )
   {
      BUZZER_ON;      
      OsStartOneShotTimer( &OsTimer[TIMER_KEYPAD_BUZZER], BUZZER_ON_TIME );      
   }
   
   KeyActionRequest =KEY_NONE;

} // end KeypadMachine()


void MainDisplay
(
   void
)
{
   //stop all motor control    
   Motor.injection.start =0;
   Motor.load.start =0;
   Motor.index.start =0;
   Motor.test.start =0;
   Motor.manual.start =0;     
     
   LcdMachineState =LCD_MAIN_SCREEN;              
   KeypadInfo.currentState =KEYPAD_MAIN_STATE;     
}
 
   
   
/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: CenterKey
*|  Description: Switch module has called this routine
*|----------------------------------------------------------------------------
*/
void KeyRight
(
   void
)
{
   KeyActionRequest =KEY_LEFT;
} // end KeyRight()

void KeyUp
(
   void
)
{
   KeyActionRequest =KEY_UP;
} // end KeyUp()

void KeyUpHeldAssert
(
   void
)
{
   KeyActionRequest =KEY_HELD_UP;
} // end KeyUp()

void KeyDown
(
   void
)
{
   KeyActionRequest =KEY_DOWN;
} // end KeyDown()

void KeyDownHeldAssert
(
   void
)
{
   KeyActionRequest =KEY_HELD_DOWN;
} // end KeyDown()

void KeyEnter
(
   void
)
{
   KeyActionRequest =KEY_ENTER;
} // end KeyEnter()

void Key5
(
   void
)
{
   KeyActionRequest =KEY_NONE; //KEY_5;
} // end Key5()



/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyRunState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyRunState(int key_)
{
   switch( key_ )
   {
      case KEY_LEFT:
         // go back
         LcdRow =0;
         MainDisplay();
         break;
      case KEY_UP:
         if( ++LcdRow >1 )
         {
            LcdRow =ROW_START;
         }
         DisplayRun(LcdRow);                
         break;
      case KEY_DOWN:
         if( --LcdRow <ROW_START )
         {
            LcdRow =1;
         }
         DisplayRun(LcdRow);                              
         break;
      case KEY_ENTER:
         switch( LcdRow )
         {
            case 0:
               // auto mode                    
               KeypadInfo.currentState =KEYPAD_RUN_AUTO_DIR_STATE;
               LcdMachineState =LCD_RUN_AUTO_DIR_SCREEN;
               break;           
            case 1:
               // manual mode                    
               KeypadInfo.currentState =KEYPAD_RUN_MANUAL_STATE;
               LcdMachineState =LCD_RUN_MANUAL_SCREEN;                     
               break;
         }
         break;               
   }  
}


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyLoadState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyLoadState(int key_)
{
   switch( key_ )        
   {
      case KEY_LEFT:
//         if( MotorControl.indexing ==1 )              
//         if( BallCount.load !=0 )    
         if( MotorControl.indexing ==1 || BallCount.load !=0 )  
         {
            // loading has been started, going back is no longer an option
            // key is now reverse           
            Motor.load.direction =MOTOR_REV;
            
            if( Motor.load.start ==0 )
            {
               // start motor           
               Motor.load.start =1;
               MotorControl.indexing =1; 
            }
            else
            {
               // stop motor
               Motor.load.start =0;                 
            }                     
         }
         else
            MainDisplay(); // go back              
         break;
      case KEY_UP:
      case KEY_HELD_UP:              
         if( (MotorControl.indexing ==1 || BallCount.load !=0) && Motor.load.start ==0 )  
         {
            // loading has been started, going back is no longer an option
            if( Motor.load.start ==0 ) // only change menu if motor stopped                 
            {
               KeypadInfo.currentState =KEYPAD_LOAD_INDEX_STATE;
               LcdMachineState =LCD_LOAD_INDEX_SCREEN;                                
                
               if( BallCount.load ==MAX_BALL_COUNT)                
                  MotorControl.indexState =INDEX_COMPLETE_STATE;
            }           
         }
         else
         {
            // adjust speed if available to do so
//            if( Motor.load.start )
            {
               if( ++Motor.load.rate >MAX_MOTOR_RATE )
               {
                  Motor.load.rate =MAX_MOTOR_RATE;
               }
            }
         }                  
         break;
      case KEY_DOWN:
      case KEY_HELD_DOWN:
         // adjust speed if available to do so
//         if( Motor.load.start )
         {        
            if( --Motor.load.rate ==0 )
            {
               Motor.load.rate =MIN_MOTOR_RATE;
            }
         }
         break;
      case KEY_ENTER:
         if( !Motor.load.start &&
              MotorControl.indexState !=INDEX_COMPLETE_STATE            
           )
         {
            // start motor      
            Motor.load.direction =MOTOR_FWD;
            
            Motor.load.start =true;
            MotorControl.indexing =1; 
            
            MotorControl.indexState =INDEX_IDLE_STATE;            
         }
         else
         {
            // stop motor
            Motor.load.start =false;                 
         }         
         break; 
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_LOAD_UPDATE_SCREEN;
         break;                 
   }                
}

/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeySetupState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeySetupState(int key_)
{
   switch( key_ )        
   {
      case KEY_LEFT:
         // go back
         MainDisplay();         
         break;
      case KEY_UP:
      case KEY_HELD_UP:                
         if( ++BallCount.load >MAX_BALL_COUNT )
         {
            BallCount.load =MAX_BALL_COUNT;                 
         }
         
         BallCount.index =0;
         MotorControl.indexState =INDEX_COMPLETE_STATE;
         break;
      case KEY_DOWN:
      case KEY_HELD_DOWN:              
         if( --BallCount.load<0 )// ==255 )
         {
            BallCount.load =0;                
         }        
         break;
      case KEY_ENTER:
         // go to cannister screen
         KeypadInfo.currentState =KEYPAD_SETUP_CANNISTER_STATE;
         LcdMachineState =LCD_SETUP_CANNISTER_SCREEN;          
         break;    
      case KEY_UPDATE_SCREEN:       
         // update LCD values
         LcdMachineState =LCD_SETUP_UPDATE_SCREEN;         
         break;                
   }
}


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyRunAutoDirState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyRunAutoDirState(int key_)
{
   switch( key_ )        
   {
      case KEY_LEFT:
         // go back
         KeypadInfo.currentState =KEYPAD_RUN_STATE;
         LcdMachineState =LCD_RUN_SCREEN;               
         break;
      case KEY_UP:               
      case KEY_HELD_UP:              
      case KEY_DOWN:
      case KEY_HELD_DOWN:              
         if( ++Motor.injection.direction >1 )
         {
            Motor.injection.direction =MOTOR_FWD;                 
         }
         break;
      case KEY_ENTER:             
         KeypadInfo.currentState =KEYPAD_RUN_AUTO_RATE_STATE;
         LcdMachineState =LCD_RUN_AUTO_RATE_SCREEN;                   
         break;   
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_RUN_AUTO_UPDATE_DIR_SCREEN;
         break;        
   }
} // end KeyRunAutoDirState(int key_)


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyRunAutoRateState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyRunAutoRateState(int key_)
{
   switch( key_ )        
   {
      case KEY_LEFT:
         // stop motor, go back
         Motor.injection.start =0;
                            
         // go back to direction adjust
         KeypadInfo.currentState =KEYPAD_RUN_AUTO_DIR_STATE;
         LcdMachineState =LCD_RUN_AUTO_DIR_SCREEN;
               
         break;
      case KEY_UP:               
      case KEY_HELD_UP:              
         if( ++Motor.injection.rate >MAX_MOTOR_RATE )
         {
            Motor.injection.rate =MAX_MOTOR_RATE;                 
         }
         break;
      case KEY_DOWN:
      case KEY_HELD_DOWN:              
         if( --Motor.injection.rate ==0 )
         {
            Motor.injection.rate =MIN_MOTOR_RATE;
         }
         break;
      case KEY_ENTER:
         // start motor
         if( Motor.injection.start ==0 )
            Motor.injection.start =1;
         else
         {
            Motor.injection.start =0;                 
         }
         break;   
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_RUN_AUTO_UPDATE_RATE_SCREEN;
         break;        
   } 
} // end KeyRunAutoRateState(int key_)


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyRunManualState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyRunManualState(int key_)
{
   switch( key_ )        
   {
       case KEY_LEFT:
          // stop motor, go back
          Motor.manual.start =0;
          KeypadInfo.currentState =KEYPAD_RUN_STATE;
          LcdMachineState =LCD_RUN_SCREEN;                
          break;
      case KEY_UP:
      case KEY_HELD_UP:              
      case KEY_DOWN:
      case KEY_HELD_DOWN:                             
         if( ++Motor.manual.direction >1 )
         {
            Motor.manual.direction =MOTOR_FWD;                 
         }
         break;
      case KEY_ENTER:
         // manual mode      
         Motor.manual.rate =100;
               
         if( Motor.manual.start ==0 )              
            Motor.manual.start =1;
         else
            Motor.manual.start =0; 
         break;
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_RUN_MANUAL_UPDATE_SCREEN;
         break;                       
   }        
}


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyLoadIndexState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyLoadIndexState(int key_)
{
   switch( key_ )        
   {
      case KEY_LEFT:
         if( MotorControl.indexState ==INDEX_IDLE_STATE )
         {
            // stop motor, go back a screen
            Motor.manual.start =0;
            KeypadInfo.currentState =KEYPAD_LOAD_STATE;
            LcdMachineState =LCD_LOAD_SCREEN;                                             
         }
         else
         {
            // reverse
            Motor.index.direction =MOTOR_REV;           
            
            if( Motor.index.start ==0 )
            {
               // start motor           
               Motor.index.start =1;            
            }
            else
            {
               Motor.index.start =0;
            }
         }
         break;
      case KEY_UP:
      case KEY_HELD_UP:   
         if( MotorControl.indexState ==INDEX_COMPLETE_STATE )
         {
            if( Motor.index.start ==0 && BallCount.index !=0 )
            {
               // start motor           
               Motor.index.start =1;
            
               Motor.index.direction =MOTOR_FWD;            
            }  
            else
            {
               Motor.index.start =0;                 
            }
         }
         else
         {
            if( ++Motor.index.rate >MAX_MOTOR_RATE )
            {
               Motor.index.rate =MAX_MOTOR_RATE;                 
            }
         }
         break;
      case KEY_DOWN:
      case KEY_HELD_DOWN:               
         if( MotorControl.indexState ==INDEX_COMPLETE_STATE )
         {
            MainDisplay();        
         }
         else
         {
            if( --Motor.index.rate ==0 )
            {
               Motor.index.rate =MIN_MOTOR_RATE;                
            }
         }
         break;
      case KEY_ENTER:
         if( MotorControl.indexState ==INDEX_COMPLETE_STATE )
         {
            // still allow motor to stop, just no start
            Motor.index.start =0;
            break;
         }
         
         if( Motor.index.start ==0 )
         {
            // start motor           
            Motor.index.start =1;
            
            Motor.index.direction =MOTOR_FWD;
              
            MotorControl.indexState =INDEX_START_STATE;
         }
         else
         {
            // stop motor
            Motor.index.start =0;                 
            
            MotorControl.indexState =INDEX_STOP_STATE;            
         }
         break;       
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_LOAD_INDEX_UPDATE_SCREEN;
         break;                                
   }                
}


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeySetupCannisterState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeySetupCannisterState(int key_)
{     
   switch( key_ )        
   {
      case KEY_LEFT:
         // go back
         MainDisplay();         
         break;
#ifndef SIX_BALLS_PER_REV    
      case KEY_UP:        
         if( MotorControl.ballsPerRev ==8 )
         {
            MotorControl.ballsPerRev =4;
            MotorControl.vanesPerBall =8;
         }
         else
         {
            MotorControl.ballsPerRev =8;
            MotorControl.vanesPerBall =4;
         }                 
         
         SaveToInternalMemory( APP_START_ADDRESS );
         break;
      case KEY_DOWN:
         if( MotorControl.ballsPerRev ==8 )
         {
            MotorControl.ballsPerRev =4;
            MotorControl.vanesPerBall =8;
         }
         else
         {
            MotorControl.ballsPerRev =8;
            MotorControl.vanesPerBall =4;
         }                          
         
         SaveToInternalMemory( APP_START_ADDRESS );                  
         break;
#endif         
      case KEY_ENTER:
         // go back
         KeypadInfo.currentState =KEYPAD_SETUP_BATTERY_STATE;
         LcdMachineState =LCD_SETUP_BATTERY_SCREEN;
         break;  
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_SETUP_CANNISTER_UPDATE_SCREEN;
         break;                          
   }                
}



/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeySetupBatteryState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeySetupBatteryState(int key_)
{     
#ifdef OLD  
   switch( key_ )        
   {
      case KEY_LEFT:
         Motor.test.start =0;  
         Motor.test.rate =100;
         KeypadInfo.currentState =KEYPAD_TEST_DIR_STATE;
         LcdMachineState =LCD_SETUP_TEST_DIR_SCREEN;               
         break;
      case KEY_UP:
      case KEY_DOWN:  
      case KEY_HELD_UP:
      case KEY_HELD_DOWN:
         if( !Motor.test.start )
         {
            // only switch direction when stopped
            if( ++Motor.test.direction >1 )
            {
               Motor.test.direction =MOTOR_FWD;                 
            }
         }
         break;
      case KEY_ENTER:
         Motor.test.rate =250;
         // start motor
         if( Motor.test.start ==0 )
            Motor.test.start =1;
         else
         {
            Motor.test.start =0;                 
         }              
         break;  
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_SETUP_BATTERY_UPDATE_SCREEN;
         break;               
   }                
#else
   switch( key_ )        
   {
      case KEY_LEFT:         
      case KEY_UP:
//      case KEY_HELD_LEFT:        
//      case KEY_HELD_UP:
         if( !Motor.test.start )
         {
            // only switch direction when stopped
            if( ++Motor.test.direction >1 )
            {
               Motor.test.direction =MOTOR_FWD;                 
            }
         }        
         break;
      case KEY_DOWN:
         Motor.test.rate =250;
         // start motor
         if( Motor.test.start ==0 )
            Motor.test.start =1;
         else
         {
            Motor.test.start =0;                 
         }              
         break;  
      case KEY_ENTER:       
         Motor.test.start =0;  
         Motor.test.rate =100;
         KeypadInfo.currentState =KEYPAD_TEST_DIR_STATE;
         LcdMachineState =LCD_SETUP_TEST_DIR_SCREEN;               
         break;         
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_SETUP_BATTERY_UPDATE_SCREEN;
         break;               
   }                   
#endif   
}


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyTestDirState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyTestDirState(int key_)
{     
   switch( key_ )        
   {
            case KEY_LEFT:
               // go back
               MainDisplay();
               break;
            case KEY_UP:
            case KEY_DOWN:  
            case KEY_HELD_UP:
            case KEY_HELD_DOWN:
               if( ++Motor.test.direction >1 )
               {
                  Motor.test.direction =MOTOR_FWD;                 
               }
               break;
            case KEY_ENTER:   
               KeypadInfo.currentState =KEYPAD_TEST_RATE_STATE;
               LcdMachineState =LCD_SETUP_TEST_RATE_SCREEN;              
               break;      
             case KEY_UPDATE_SCREEN:
               // update LCD values
               LcdMachineState =LCD_SETUP_TEST_UPDATE_DIR_SCREEN;
               break;                
   }                
}


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyTestRateState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyTestRateState(int key_)
{     
   switch( key_ )        
   {
            case KEY_LEFT:
               // stop motor, go back
               Motor.test.start =0;              
               
               // go back to mode where direction can be changed
               Motor.test.start =0;  
               KeypadInfo.currentState =KEYPAD_TEST_DIR_STATE;
               LcdMachineState =LCD_SETUP_TEST_DIR_SCREEN;                              
               break;
            case KEY_UP:               
            case KEY_HELD_UP:              
               if( ++Motor.test.rate >MAX_MOTOR_RATE )
               {
                  Motor.test.rate =MAX_MOTOR_RATE;                 
               }
               break;
            case KEY_DOWN:
            case KEY_HELD_DOWN:              
               if( --Motor.test.rate ==0 )
               {
                  Motor.test.rate =MIN_MOTOR_RATE;
               }
               break;
      case KEY_ENTER: 
         if( Motor.test.start ==0 )
            Motor.test.start =1;
         else
         {
            Motor.test.start =0;                 
         }                  
         break;      
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_SETUP_TEST_UPDATE_RATE_SCREEN;
         break;                
   }                
}


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: KeyDropperState
*|  Description:
*|----------------------------------------------------------------------------
*/
void KeyDropperState(int key_)
{
   switch( key_ )
   {
      case KEY_LEFT:
         // go back
         LcdRow =0;
         MainDisplay();
         break;
      case KEY_UP:
         // JOG+
         Motor.jog.rate =100;

         Motor.jog.direction =MOTOR_FWD;
                  
         if( Motor.jog.start ==0 )              
            Motor.jog.start =1;
         else
            Motor.jog.start =0;                 
         break;
      case KEY_DOWN:
         //JOG-
         Motor.jog.rate =100;

         Motor.jog.direction =MOTOR_REV;
                  
         if( Motor.jog.start ==0 )              
            Motor.jog.start =1;
         else
            Motor.jog.start =0;                 
         break;
      case KEY_ENTER:
         // DROP
         // manual mode      
         Motor.manual.rate =100;
                  
         if( Motor.manual.start ==0 )              
            Motor.manual.start =1;
         else
            Motor.manual.start =0;         
         break;               
      case KEY_UPDATE_SCREEN:
         // update LCD values
         LcdMachineState =LCD_DROPPER_UPDATE_SCREEN;
         break;         
   }  
}


// end KeypadFd.c




