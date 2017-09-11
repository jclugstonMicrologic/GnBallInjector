/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*|Module:   LcdFd                                                           */
/*|Description: function driver for the lcd module                           */
/*|                                                                          */
/*|***************************************************************************/

/*
Tools:
   Compiler: IAR Kickstart development environment for AT91SAM7Sxxx
   ***************************************************************************
   Copyright (C) 2006, Micrologic Limited.
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

#include "lcdFd.h"
#include "adcFd.h"
#include "keypadFd.h"
#include "timer.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "motorcntrl.h"

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/
#define KEY_TEXT_ROW 57

#define KEY_LEFT_COL   0
#define KEY_UP_COL    17
#define KEY_DOWN_COL  35
#define KEY_RIGHT_COL 49

/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/

UINT8 LcdMachineState;
UINT16 DisplayBallsPerMin;
/*
*****************************************************************************
 P R I V A T E   G L O B A L   D A T A
*****************************************************************************
*/


/*
*****************************************************************************
 P R I V A T E   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/
void DecTimeToDisplay( UINT32 timeInSecs_, char *resultStrPtr_ );
void DisplayLogo(void);

/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/

/*|------------------------------------------------------------------------
  |  Module: lcdFd.c
  |  Routine: InitLcd
  |  Description:
  |------------------------------------------------------------------------
*/
void InitLcd
(
   void
)
{
   /*
   **********************
   L O C A L   D A T A
   **********************
   */

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   /*
   ************************
   C O D E
   ************************
   */
   lcdInit();
    
   LcdMachineState =LCD_MAIN_SCREEN;
}

void DisplayLcdOff(void)
{
   lcdDisplayOff();
}

void DisplayLcdOn(void)
{
   InitLcd();
}

/*|------------------------------------------------------------------------
  |  Module: lcdFd.c
  |  Routine: DisplayLcdMessage
  |  Description: sends the message(lcdMsgPtr_) to the lcd
  |               one character at a time, starting at cursorPos_
  |------------------------------------------------------------------------
*/
void DisplayLcdMessage
(
   UINT8 *lcdMsgPtr_, /* message string to be sent */
   UINT8 row_,        /* start row position of message */
   UINT8 col_,        /* start col (page) position of message */
   UINT8 intensity_
)
{
   /*
   **********************
   L O C A L   D A T A
   **********************
   */
   UINT8 j;

   UINT8 length;
   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   /*
   ************************
   C O D E
   ************************
   */
   length =strlen( (char *)lcdMsgPtr_ );   
   
   for( j =0; j<length; j++)
   {
      displayChar( *(lcdMsgPtr_++),  row_, col_+j*3, intensity_);
   }

}/* end DisplayLcdMessage() */



/*|------------------------------------------------------------------------
  |  Module: lcdFd.c
  |  Routine: LcdMachine
  |  Description:
  |------------------------------------------------------------------------*/
void LcdMachine
(
   void
)
{
   char aStr[26];
   UINT8 intensity1 =1;
   UINT8 intensity2 =1;
            
   strcpy(aStr, "                          ");
   if( LcdMachineState !=LCD_MSG_DISPLAYED ) 
   {
      if( (LcdMachineState &0x80) !=0x80 )
      {
         // indicates this is not 
         // an update/refresh screen, so clear the LCD
         clearLcd();
            
         DisplayLogo();
      
         DisplayLcdMessage("GN CORP ", 0, 10, 1);
         
         if( LcdMachineState ==LCD_MAIN_SCREEN )
         {
            DisplayLcdMessage("BALL INJ", 8, 10, 1);         
         }
         else
            DisplayLcdMessage("BALL INJECTOR", 8, 10, 1);                    
      }

#ifdef LCD_DISPLAY_SPEED_STUFF      
      sprintf(aStr, "V %d  ", MotorControl.nbrVanes); 
      DisplayLcdMessage((UINT8 *)&aStr, 0, 40, 1);                   
      sprintf(aStr, "B %d  ", MotorControl.ballsPerMin);                   
      DisplayLcdMessage((UINT8 *)&aStr, 8, 40, 1);                    
#endif         
   }
   
   // display a message
   switch( LcdMachineState )
   {
      case LCD_START_SCREEN:         
         break;
      case LCD_MAIN_SCREEN:                  
         strcpy(aStr, GN_VERSION);
         DisplayLcdMessage((UINT8 *)&aStr, 0, 40, 1);      
         strcpy(aStr, GN_DATE);         
         DisplayLcdMessage((UINT8 *)&aStr, 8, 40, 1);         

         Display(LcdRow);

         DisplayLcdMessage("OFF",  KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("UP",   KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("DN",   KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         DisplayLcdMessage("ENTER",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         break;         
      case LCD_RUN_SCREEN:                   
         DisplayLcdMessage("RUN",   24, 0, 2);         

         DisplayRun(LcdRow);
         
         DisplayLcdMessage("BACK", KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("UP",   KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("DN",   KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         DisplayLcdMessage("ENTER",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         break;                  
      case LCD_LOAD_SCREEN:           
      case LCD_LOAD_UPDATE_SCREEN:
         DisplayLcdMessage("LOAD",        24, 0, 2);         
                                        
         if( LcdMachineState ==LCD_LOAD_SCREEN ||
             LcdMachineState ==LCD_LOAD_UPDATE_SCREEN
           )         
         {
            sprintf(aStr, "BALL COUNT  %d  ", BallCount.load);                     
         }         
         else
         {
            sprintf(aStr, "BALL COUNT  %d  ", BallCount.index);                       
         }
         DisplayLcdMessage((UINT8 *)&aStr, 24, 15, 1);                
         
         sprintf(aStr, "RATE        %d  ", Motor.load.rate);           
         DisplayLcdMessage((UINT8 *)&aStr, 32, 15, 2);   
         
         if( Motor.load.direction ==MOTOR_JAM )
            DisplayLcdMessage("DIRECTION  JAM",       40, 16, 2);            
         else
            DisplayLcdMessage("              ",       40, 16, 1); 

         if( MotorControl.indexing ==1 || BallCount.load !=0 )         
         {  
            // loading has been started, going back is no longer an option
            DisplayLcdMessage("REV ", KEY_TEXT_ROW, 0, 1);                                           
         
            if( Motor.load.start )
            {
               DisplayLcdMessage("UP   ",    KEY_TEXT_ROW, KEY_UP_COL, 1);            
               DisplayLcdMessage("DN   ",    KEY_TEXT_ROW, KEY_DOWN_COL, 1);                 
               DisplayLcdMessage("STOP ", KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
            }
            else
            {
               DisplayLcdMessage("INDEX", KEY_TEXT_ROW, KEY_UP_COL, 1);            
               DisplayLcdMessage("     ", KEY_TEXT_ROW, KEY_DOWN_COL, 1);               

               if( MotorControl.indexState ==INDEX_COMPLETE_STATE )
                  DisplayLcdMessage("     ", KEY_TEXT_ROW, KEY_RIGHT_COL, 1);                                
               else
                  DisplayLcdMessage("START", KEY_TEXT_ROW, KEY_RIGHT_COL, 1);                          
            }
         }
         else
         {
            // no loading has started, user may leave the screen
            DisplayLcdMessage("BACK ", KEY_TEXT_ROW, 0, 1);                       
            DisplayLcdMessage("UP   ",    KEY_TEXT_ROW, KEY_UP_COL, 1);            
            DisplayLcdMessage("DN   ",    KEY_TEXT_ROW, KEY_DOWN_COL, 1);              
            DisplayLcdMessage("START", KEY_TEXT_ROW, KEY_RIGHT_COL, 1); 
         }
         break;                           
      case LCD_LOAD_INDEX_SCREEN:
      case LCD_LOAD_INDEX_UPDATE_SCREEN:
         DisplayLcdMessage("INDEX",     24, 0, 2);         
         
         sprintf(aStr, "BALL COUNT  %d ", BallCount.index);                       
         DisplayLcdMessage((UINT8 *)&aStr, 24, 16, 1);                
         
         sprintf(aStr, "RATE        %d  ", Motor.index.rate);           
         DisplayLcdMessage((UINT8 *)&aStr, 32, 16, 2);
         
         if( Motor.index.direction ==MOTOR_JAM )
            DisplayLcdMessage("DIRECTION   JAM",    40, 16, 2);            
         else
            DisplayLcdMessage("               ",    40, 16, 1);
         
         switch( MotorControl.indexState )
         {
            case INDEX_STOP_STATE: 
            case INDEX_IDLE_STATE:               
            case INDEX_START_STATE:                             
               DisplayLcdMessage("UP   ",   KEY_TEXT_ROW, KEY_UP_COL, 1);            
               DisplayLcdMessage("DN   ",   KEY_TEXT_ROW, KEY_DOWN_COL, 1);          
               
               if( MotorControl.indexState ==INDEX_IDLE_STATE )
               {                 
                  DisplayLcdMessage("BACK ", KEY_TEXT_ROW, 0, 1);                          
                  DisplayLcdMessage("START",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);                                           
               }
               else if( MotorControl.indexState ==INDEX_START_STATE)
               {
                  DisplayLcdMessage("REV  ", KEY_TEXT_ROW, 0, 1);                          
                  DisplayLcdMessage("STOP ",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);                                                            
               }
               else
               {
                  DisplayLcdMessage("REV  ", KEY_TEXT_ROW, 0, 1);                                           
                  DisplayLcdMessage("START",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);                             
               }
               break;
            case INDEX_COMPLETE_STATE:
               DisplayLcdMessage("REV  ", KEY_TEXT_ROW, 0, 1);                                        

               if( BallCount.index !=0 )
                  DisplayLcdMessage("FWD  ", KEY_TEXT_ROW, KEY_UP_COL, 1);            
               else
                  DisplayLcdMessage("     ", KEY_TEXT_ROW, KEY_UP_COL, 1);                           
               DisplayLcdMessage("BACK ", KEY_TEXT_ROW, KEY_DOWN_COL, 1);
               DisplayLcdMessage("STOP ", KEY_TEXT_ROW, KEY_RIGHT_COL, 1);                                                         
               break;
         }        
         break;                                    
      case LCD_SETUP_SCREEN:                    
      case LCD_SETUP_UPDATE_SCREEN:
         DisplayLcdMessage("SETUP",    24, 0, 2);         
         DisplayLcdMessage("CANISTER", 24, 18, 1);         
         DisplayLcdMessage("SIZE",     24, 50, 1);            
                
         sprintf(aStr, "BALL COUNT %d  ", BallCount.load);           
         DisplayLcdMessage((UINT8 *)&aStr, 32, 18, 1);              

         DisplayLcdMessage("EXIT",  KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("UP",   KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("DN",   KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         DisplayLcdMessage("NEXT",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         break;                                    
      case LCD_RUN_AUTO_DIR_SCREEN:         
      case LCD_RUN_AUTO_RATE_SCREEN:                 
      case LCD_RUN_AUTO_UPDATE_DIR_SCREEN:
      case LCD_RUN_AUTO_UPDATE_RATE_SCREEN:        
         DisplayLcdMessage("AUTO",       24, 0, 2);         

         if( !MotorControl.indexing )
         {
            sprintf(aStr, "BALL COUNT %d  ", BallCount.load);           
            DisplayLcdMessage((UINT8 *)&aStr, 24, 16, 1);                  
         }
         else
         {
            sprintf(aStr, "INDX COUNT %d  ", BallCount.index);           
            DisplayLcdMessage((UINT8 *)&aStr, 24, 16, 1);                             
         }
        
         if( LcdMachineState ==LCD_RUN_AUTO_DIR_SCREEN ||
             LcdMachineState ==LCD_RUN_AUTO_UPDATE_DIR_SCREEN
            )
         {
            intensity1 =2;
            intensity2 =1;
         }
         else
         {
            intensity1 =1;
            intensity2 =2;
         }         
         if( Motor.injection.direction ==MOTOR_FWD )
            DisplayLcdMessage("DIRECTION  FWD",       32, 16, intensity1);            
         else if( Motor.injection.direction ==MOTOR_REV )
            DisplayLcdMessage("DIRECTION  REV",       32, 16, intensity1);           
         else
            DisplayLcdMessage("DIRECTION  JAM",       32, 16, 2);                      

         sprintf(aStr, "RATE       %d  ", Motor.injection.rate);  
         DisplayLcdMessage((UINT8 *)&aStr, 40, 16, intensity2);                             

         DisplayLcdMessage("BACK",  KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("UP",   KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("DN",   KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         
         if( LcdMachineState ==LCD_RUN_AUTO_UPDATE_DIR_SCREEN ||
             LcdMachineState ==LCD_RUN_AUTO_DIR_SCREEN
           )
         { 
            DisplayLcdMessage("ENTER",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         }
         else
         {
            if( Motor.injection.start )
            {
               // motor is running/started, so display "STOP"
               DisplayLcdMessage("STOP ",KEY_TEXT_ROW, KEY_RIGHT_COL, 1); 
            }
            else
               DisplayLcdMessage("START",KEY_TEXT_ROW, KEY_RIGHT_COL, 1); 
         }
         break;                                    
      case LCD_RUN_MANUAL_SCREEN:         
      case LCD_RUN_MANUAL_UPDATE_SCREEN:                 
         DisplayLcdMessage("MAN",     24, 0, 2);         
        
         if( !MotorControl.indexing )
         {
            sprintf(aStr, "BALL COUNT %3d  ", BallCount.load);           
            DisplayLcdMessage((UINT8 *)&aStr, 24, 16, 1);                  
         }
         else
         {
            sprintf(aStr, "INDX COUNT %d  ", BallCount.index);           
            DisplayLcdMessage((UINT8 *)&aStr, 24, 16, 1);                             
         }
           

         if( Motor.manual.direction ==MOTOR_FWD )
            DisplayLcdMessage("DIRECTION  FWD",       32, 16, 2);            
         else if( Motor.manual.direction ==MOTOR_REV )
            DisplayLcdMessage("DIRECTION  REV",       32, 16, 2);
         else
            DisplayLcdMessage("DIRECTION  JAM",       32, 16, 2);                      
         
         DisplayLcdMessage("BACK",  KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("UP",    KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("DN",    KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         
         if( Motor.manual.start )
            DisplayLcdMessage("STOP ",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         else
            DisplayLcdMessage("START",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         break;                                             
      case LCD_SETUP_CANNISTER_SCREEN:         
      case LCD_SETUP_CANNISTER_UPDATE_SCREEN:
         DisplayLcdMessage("SETUP",      24, 0, 2);         
         DisplayLcdMessage("CANNISTER",  24, 18, 1);         
         DisplayLcdMessage("TYPE",       24, 49, 1);            

         sprintf(aStr, "BALLS/REV  %d  ", MotorControl.ballsPerRev);  
         DisplayLcdMessage((UINT8 *)&aStr, 32, 18, 2);
         
         DisplayLcdMessage("EXIT", KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("UP",   KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("DN",   KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         DisplayLcdMessage("NEXT", KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         break;
      case LCD_SETUP_BATTERY_SCREEN:
      case LCD_SETUP_BATTERY_UPDATE_SCREEN:                
         DisplayLcdMessage("SETUP",   24, 0, 2);         
         
         sprintf(aStr, "BATTERY   %2.1fV ", (float)IntrnlAdcReading.battery/1000.0);
         DisplayLcdMessage( (UINT8 *)&aStr, 24, 18, 1);

         sprintf(aStr, "MIN       %2.1fV ", (float)IntrnlMinAdcReading.battery/1000.0);
         DisplayLcdMessage( (UINT8 *)&aStr, 32, 18, 1);
                         
         if( Motor.test.direction ==0 )
            DisplayLcdMessage("DIRECTION  FWD",       40, 18, 2);            
         else if( Motor.test.direction ==1 )
            DisplayLcdMessage("DIRECTION  REV",       40, 18, 2);   
         else
            DisplayLcdMessage("DIRECTION  JAM",       40, 18, 2);                      

#ifdef OLD         
         DisplayLcdMessage("NEXT", KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("UP",   KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("DN",   KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         
         if( Motor.test.start )
            DisplayLcdMessage("STOP ",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);                                  
         else
            DisplayLcdMessage("START",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);                       
#else       
         if( Motor.test.start )
         {
            DisplayLcdMessage("    ", KEY_TEXT_ROW, 0, 1);            
            DisplayLcdMessage("    ", KEY_TEXT_ROW, KEY_UP_COL, 1);                                  
            DisplayLcdMessage("STOP ",KEY_TEXT_ROW, KEY_DOWN_COL, 1);                                  
         }
         else
         {
            DisplayLcdMessage("UP  ", KEY_TEXT_ROW, 0, 1);            
            DisplayLcdMessage("DN  ", KEY_TEXT_ROW, KEY_UP_COL, 1);                       
            DisplayLcdMessage("START",KEY_TEXT_ROW, KEY_DOWN_COL, 1);       
         }
         
         DisplayLcdMessage(" NEXT", KEY_TEXT_ROW, KEY_RIGHT_COL, 1);            
#endif         
         break;        
      case LCD_SETUP_TEST_DIR_SCREEN:
      case LCD_SETUP_TEST_RATE_SCREEN:        
      case LCD_SETUP_TEST_UPDATE_DIR_SCREEN:
      case LCD_SETUP_TEST_UPDATE_RATE_SCREEN:        
         DisplayLcdMessage("SETUP",   24, 0, 2);         

         sprintf(aStr, "TEST SPEED %d  ", DisplayBallsPerMin) ;//MotorControl.ballsPerMin); // );               
         DisplayLcdMessage((UINT8 *)&aStr, 24, 18, 1);                    
         
         if( LcdMachineState ==LCD_SETUP_TEST_DIR_SCREEN ||
             LcdMachineState ==LCD_SETUP_TEST_UPDATE_DIR_SCREEN
            )
         {
            intensity1 =2;
            intensity2 =1;
         }
         else
         {
            intensity1 =1;
            intensity2 =2;
         }
           
         if( Motor.test.direction ==MOTOR_FWD )
            DisplayLcdMessage("DIRECTION  FWD ",       32, 18, intensity1);            
         else if( Motor.test.direction ==MOTOR_REV )
            DisplayLcdMessage("DIRECTION  REV ",       32, 18, intensity1);    
         else
            DisplayLcdMessage("DIRECTION  JAM ",       32, 18, intensity1);             
                  
         sprintf(aStr, "RATE       %d  ", Motor.test.rate);  
         DisplayLcdMessage((UINT8 *)&aStr, 40, 18, intensity2);                                     

         DisplayLcdMessage("BACK", KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("UP",   KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("DN",   KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         
         if( LcdMachineState ==LCD_SETUP_TEST_UPDATE_DIR_SCREEN ||
             LcdMachineState ==LCD_SETUP_TEST_DIR_SCREEN
           )
            DisplayLcdMessage("ENTER",KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         else
         {
            if( Motor.test.start )
            {
               // motor is running/started, so display "STOP"
               DisplayLcdMessage("STOP ",KEY_TEXT_ROW, KEY_RIGHT_COL, 1); 
            }
            else
               DisplayLcdMessage("START",KEY_TEXT_ROW, KEY_RIGHT_COL, 1); 
         }         
         break;                 
      case LCD_DROPPER_SCREEN: 
      case LCD_DROPPER_UPDATE_SCREEN:
         DisplayLcdMessage("DROP",   24, 0, 2);         

         sprintf(aStr, "BALL COUNT  %d  ", BallCount.load); 

         DisplayLcdMessage((UINT8 *)&aStr, 32, 15, 1);                         
         
         DisplayLcdMessage("BACK", KEY_TEXT_ROW, 0, 1);            
         DisplayLcdMessage("JOGF", KEY_TEXT_ROW, KEY_UP_COL, 1);            
         DisplayLcdMessage("JOGR", KEY_TEXT_ROW, KEY_DOWN_COL, 1);   
         DisplayLcdMessage("DROP", KEY_TEXT_ROW, KEY_RIGHT_COL, 1);               
         break;                           
   }

   // always move state to message has been displayed (prevents constant refreshing)
   LcdMachineState =LCD_MSG_DISPLAYED;

} // end LcdMachine()


void Display
(
   UINT8 row
)
{
   UINT8 intensity1=1;
   UINT8 intensity2=1;
   UINT8 intensity3=1;   
   
   switch(row)
   {
      case 0:     
         intensity1=2;
         break;
      case 1:     
         intensity2=2;        
         break;
      case 2:     
         intensity3=2;
         break;        
   }
   
   DisplayLcdMessage("RUN                  ", 24, 10, intensity1);                 
   DisplayLcdMessage("LOAD                 ", 32, 10, intensity2);         
   DisplayLcdMessage("SETUP                ", 40,  10, intensity3);                    
}

void DisplayRun
(
   UINT8 row
)
{
   UINT8 intensity1=1;
   UINT8 intensity2=1;
   
   switch(row)
   {
      case 0:     
         intensity1=2;
         break;
      case 1:     
         intensity2=2;        
         break;
   }

   DisplayLcdMessage("AUTO                 ", 32, 10, intensity1);         
   DisplayLcdMessage("MANUAL               ", 40, 10, intensity2);            
   
}

/*|------------------------------------------------------------------------
  |  Module: lcdFd.c
  |  Routine: DisplayLcdGraphic
  |  Description:
  |------------------------------------------------------------------------
*/
void DisplayLcdGraphic
(
   UINT8 character_,
   UINT8 row_,
   UINT8 col_
)
{
//   setLcdRow(row_, col_);
//   displaySpecialChar(character_, row_, col_);
}

/*|------------------------------------------------------------------------
  |  Module: lcdFd.c
  |  Routine: SendAsAscii
  |  Description: sends a real value as ASCII chars to the LCD
  |------------------------------------------------------------------------
*/
void SendAsAscii
(
   UINT16 theValue_,
   UINT8 row_,
   BOOL invert_
)
{
   /*
   **********************
   L O C A L   D A T A
   **********************
   */
   int j;
   UINT8 aChar[5];
   UINT8 digitCnt;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   /*
   ************************
   C O D E
   ************************
   */
   digitCnt =0;

   if( theValue_ == 0 )
   {
      displayChar(' ', row_, 0, invert_);
      displayChar(' ', row_, 0, invert_);
      displayChar('0', row_, 0, invert_);

      return;
   }

   for(j=4; j>=0; j--)
   {
//      if( theValue_ %10 )
      if( theValue_ !=0 )
      {
         aChar[j] = theValue_%10 + '0';
         theValue_ /= 10;

         digitCnt++;
      }
      else
      {
//!!! FIX ME, this for loop only works for non 0 digits
// i.e. 10, 100, 105 do not work
         break;
      }
   }

   switch( digitCnt )
   {
      case 1:
         // display two ' ''s
         displayChar(' ', row_, 0, invert_);
         displayChar(' ', row_, 0, invert_);
      break;

      case 2:
         // display ' '
         displayChar(' ', row_, 0, invert_);
      break;
   }

   // display the digits
   for(j=(5-digitCnt); j<=4; j++)
   {
      displayChar(aChar[j], row_, 0, invert_);
   }

} // end SendAsAscii()


/*|------------------------------------------------------------------------
  |  Module: lcdFd.c
  |  Routine: SendAsAsciiF
  |  Description:
  |------------------------------------------------------------------------
*/
void SendAsAsciiF
(
   float theValue_,
   UINT8 row_,
   BOOL invert_
)
{
   int j;
   UINT8 aChar[6];
   UINT8 digitCnt;
   UINT32 tempValue;
   UINT8 negFlag;

   negFlag =0;

   memset(aChar, 0x30, sizeof(aChar));

   if( theValue_ <0 )
   {
      theValue_ *=-1;

      // need to display a negative
      negFlag =1;
   }

   tempValue =theValue_ *100;

   digitCnt =0;

//   for(i=4; i>=0; i--)
   for(j=5; j>=0; j--)
   {
      if( tempValue %10 ||
          tempValue >=10
        )
      {
         aChar[j] = tempValue%10 + '0';
         tempValue /= 10;

         digitCnt++;
      }
      else
         break;
   }

   switch( digitCnt )
   {
      case 0:
      case 1:
      case 2:
         if( negFlag )
         {
            // display two ' '  + '-' + one leading 0
            displayChar(' ', row_, 0, invert_);
            displayChar(' ', row_, 0, invert_);
            displayChar('-', row_, 0, invert_);
            displayChar('0', row_, 0, invert_);
         }
         else
         {
            // display three ' ''s  + one leading 0
            displayChar(' ', row_, 0, invert_);
            displayChar(' ', row_, 0, invert_);
            displayChar(' ', row_, 0, invert_);
            displayChar('0', row_, 0, invert_);
         }

         if( digitCnt ==1 )
            digitCnt =0;
         break;
      case 3:
         if( negFlag )
         {
            // display two ' ' + '-'
            displayChar(' ', row_, 0, invert_);
            displayChar(' ', row_, 0, invert_);
            displayChar('-', row_, 0, invert_);
         }
         else
         {
            // display three ' '
            displayChar(' ', row_, 0, invert_);
            displayChar(' ', row_, 0, invert_);
            displayChar(' ', row_, 0, invert_);
         }
         break;
      case 4:
         if( negFlag )
         {
            // display one ' ' +'-'
            displayChar(' ', row_, 0, invert_);
            displayChar('-', row_, 0, invert_);
         }
         else
         {
            // display two ' ''
            displayChar(' ', row_, 0, invert_);
            displayChar(' ', row_, 0, invert_);
         }
         break;
      case 5:
         if( negFlag )
         {
            // display '-'
            displayChar('-', row_, 0, invert_);
         }
         else
         {
            // display one ' '
            displayChar(' ', row_, 0, invert_);
         }
         break;

   }

   // display the digits
//   for(j=(5-digitCnt); j<=2; j++)
   for(j=(6-digitCnt); j<=3; j++)
   {
      displayChar(aChar[j], row_, 0, invert_);
   }

   displayChar('.', row_, 0, invert_);

#ifdef TWO_SIG_DIGITS
   for(i=3; i<=4; i++)
   {
      displayChar(aChar[i], row_, 0, invert_);
   }

   if( digitCnt ==0 )
   {
      // display 2 sig digits if value =0.0
      displayChar('0', row_, 0, invert_);
      displayChar('0', row_, 0, invert_);
   }
#else
   // else 1 sig digit
//   displayChar(aChar[3], row_, 0, invert_);
   displayChar(aChar[4], row_, 0, invert_);
#endif


} // end SendAsAsciiF()


/*
*|----------------------------------------------------------------------------
*|  Module: Tool
*|  Routine: IntToAscii
*|  Description:
*|   Convert a hex into a null-terminated capital ASCII string hex
*|   eg 0x12aabb33 becomes "12AABB33"
*|----------------------------------------------------------------------------
*/
BOOL IntToAscii
(
   UINT32 intValue_,
   UINT8 * resultPtr_,
   UINT8 numOfAscii_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 i;
   UINT8 aByte;
   UINT8 aResult[9];

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
   /* create an 9 char ASCII representation, includes null terminator */
   for( i=0; i<4; i++)
   {
      /* pick out each byte of the integer, convert to integer ... */
      /* convert the MSNibble of the byte */
      aByte = ( *(  (UINT8 *)&intValue_ + i )) >>4;
      aResult[i*2] =  aByte<=9?(aByte + 0x30):(aByte + 0x37);

      /* convert the LSNibble of the byte */
      aByte = ( *(  (UINT8 *)&intValue_ + i )) & 0x0F;
      aResult[1 + (i*2)] = aByte<=9?(aByte + 0x30):(aByte + 0x37);
   }
   aResult[8] = 0; /* null terminator */

   /* left justify the ascii string to form required number of characters */
   if( ( numOfAscii_ == 2 ) ||
       ( numOfAscii_ == 4 ) ||
       ( numOfAscii_ == 6 ) ||
       ( numOfAscii_ == 8 )
     )
   {
      for( i=0; i<=numOfAscii_; i++ )
      {
         resultPtr_[i]=aResult[i+(8-numOfAscii_)];
      }
   }
   else
   {
      return 0;
   }
   return 1;

} /* HexToAscii() */




/*
*|----------------------------------------------------------------------------
*|  Module: Tool
*|  Routine: CharToAscii
*|  Description:
*|   Convert a byte into two ascii representation of it.
*|   eg 0x1b becomes "1B"
*|----------------------------------------------------------------------------
*/
BOOL CharToAscii
(
   UINT8 theChar_,      /*| the byte to be converted into ASCII */
   UINT8 * resultPtr_    /*| location to store the most signif byte of result  */
                        /*| the next location stores least signif byte        */
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 aByte;
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
   /* handle MSbyte of result */
   aByte = theChar_>>4;
   *resultPtr_ = aByte<=9?(aByte + 0x30):(aByte + 0x37);


   /* handle LSbyte of result */
   aByte = theChar_ & 0x0F;
   *(resultPtr_+1) = aByte<=9?(aByte + 0x30):(aByte + 0x37);

   return 1;
}/* CharToAscii() */

/*
*|----------------------------------------------------------------------------
*|  Module: LcdFd
*|  Routine: HexToDec
*|  Description:
*|----------------------------------------------------------------------------
*/
BOOL HexToDec
(
   UINT8 theChar_,      /*| the byte to be converted into ASCII */
   UINT8 * resultPtr_    /*| location to store the most signif byte of result  */
                        /*| the next location stores least signif byte        */
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */

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
   /* handle MSbyte of result */
   *(resultPtr_) =theChar_/10;

   *(resultPtr_++) +=0x30;

   *(resultPtr_) =theChar_%10;
   *(resultPtr_) +=0x30;

   return 1;

}/* HexToDec() */

void DecTimeToDisplay
(
   UINT32 timeInSecs_,
   char *resultStrPtr_
)
{
   // only good for <3600seconds, fix later if hours needed
   *resultStrPtr_ ++ ='0';// ((timeInSecs_/36000))+'0';
   *resultStrPtr_ ++ ='0'; //((timeInSecs_/3600)%10)+'0';
   *resultStrPtr_ ++ =':';

   // only good for <3600seconds, fix later if hours needed
   *resultStrPtr_ ++ =((timeInSecs_/600))+'0';
   *resultStrPtr_ ++ =((timeInSecs_/60)%10)+'0';
   *resultStrPtr_ ++ =':';

   // only good for <3600seconds, fix later if hours needed
   *resultStrPtr_ ++ =((timeInSecs_%60)/10)+'0';
   *resultStrPtr_ ++ =((timeInSecs_%60)%10)+'0';

   *resultStrPtr_ ='\0';
}


void DisplayLogo
(
   void
)
{
#define INTENSITY_LOW 0x0a

   UINT8 j,k, l;

   UINT8 displayByte[200];
   
//#define ORIG_LOGO   
#ifdef ORIG_LOGO
   displayByte[0] =0x00;
   displayByte[1] =0x00;   
   displayByte[2] =0x00;
   
   displayByte[3] =0x00;
   displayByte[4] =0xf0;
   displayByte[5] =0x0f;
      
   displayByte[6] =0x00;
   displayByte[7] =0xf0;
   displayByte[8] =0x0f;     
   
   displayByte[9] =0x00;
   displayByte[10] =0xf0;
   displayByte[11] =0x0f;
      
   displayByte[12] =0x00;
   displayByte[13] =0xf0;
   displayByte[14] =0x0f;
   
   displayByte[15] =0x00;
   displayByte[16] =0xf0; 
   displayByte[17] =0x0f;         
   
   displayByte[18] =0x00;               
   displayByte[19] =0xff;            
   displayByte[20] =0x0f;            
   
   displayByte[21] =0xf0;
   displayByte[22] =0x0f;
   displayByte[23] =0x0f;
   
   displayByte[24] =0xff;
   displayByte[25] =0x00;
   displayByte[26] =0x00;   
    
   displayByte[27] =0x0f;
   displayByte[28] =0xf0;
   displayByte[29] =0x0f;      
   
   displayByte[30] =0x0f;
   displayByte[31] =0xff;
   displayByte[32] =0xff;      

   displayByte[33] =0x0f;
   displayByte[34] =0xf0;
   displayByte[35] =0x0f;      

   displayByte[36] =0xff;
   displayByte[37] =0x00;
   displayByte[38] =0x00;      

   displayByte[39] =0xf0;
   displayByte[40] =0x0f;
   displayByte[41] =0xff;      

   displayByte[42] =0x00;
   displayByte[43] =0xff;
   displayByte[44] =0x0f;         
   
   for(k=0; k<15; k++)
   {
      writeCommand(0x15); //  set column address
      writeCommand(0); //  
      writeCommand(0x3f); //    
   
      writeCommand(0x75); // set row address
      writeCommand(k); //  
      writeCommand(0x3f); //  

      for(j=0; j<3; j++)
      {     
         writeData(displayByte[j+3*k] );
      }       
   }
#else
   l =0;
     
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;      
   displayByte[l++] =0xf0;
   displayByte[l++] =0x0f;
   displayByte[l++] =0x00;        
   
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;
   displayByte[l++] =0xff;      
   displayByte[l++] =0xff;
   displayByte[l++] =0x00;   
   
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;            
   displayByte[l++] =0xff;
   displayByte[l++] =0xff;
   displayByte[l++] =0x00;   
   
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;            
   displayByte[l++] =0xf0;
   displayByte[l++] =0x0f;
   displayByte[l++] =0x00;        

   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;            
   displayByte[l++] =0xf0;
   displayByte[l++] =0x0f;
   displayByte[l++] =0x00;         
   
   displayByte[l++] =0x00;
   displayByte[l++] =0x0a;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;            
   displayByte[l++] =0xff;
   displayByte[l++] =0xff;
   displayByte[l++] =0x00;         
     
   displayByte[l++] =0xa0;
   displayByte[l++] =0x00;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0x00;            
   displayByte[l++] =0xf0;
   displayByte[l++] =0x0f;
   displayByte[l++] =0x00;            
   
   displayByte[l++] =0xaa;
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0x00;            
   displayByte[l++] =0xff;
   displayByte[l++] =0x0f;
   displayByte[l++] =0x00;            

   displayByte[l++] =0x0a;
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xf0;            
   displayByte[l++] =0x0f;
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;            
   
/*** start row 10  ******/      
   displayByte[l++] =0x0a;
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xff;            
   displayByte[l++] =0x00;
   displayByte[l++] =0x0f;
   displayByte[l++] =0x0f;

   displayByte[l++] =0x0a;
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xff;            
   displayByte[l++] =0x00;
   displayByte[l++] =0xff;
   displayByte[l++] =0x00;            

   displayByte[l++] =0x0a;
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xff;            
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;
   displayByte[l++] =0xf0;            

   displayByte[l++] =0xa0;
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xf0;            
   displayByte[l++] =0x0f;
   displayByte[l++] =0xf0;
   displayByte[l++] =0x0f;            

   displayByte[l++] =0x00;
   displayByte[l++] =0x0a;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0x00;            
   displayByte[l++] =0xff;
   displayByte[l++] =0xff;
   displayByte[l++] =0x00;               

   
/*****************************************/      
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xaa;   
   displayByte[l++] =0x00;            
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;            

   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;            
   displayByte[l++] =0xaa;
   displayByte[l++] =0x0a;
   displayByte[l++] =0x00;            

   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0xa0;   
   displayByte[l++] =0x0a;   
   displayByte[l++] =0xaa;            
   displayByte[l++] =0xaa;
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;               

   displayByte[l++] =0x00;
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;   
   displayByte[l++] =0x00;            
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;
   displayByte[l++] =0x00;            

   for(k=0; k<18; k++)
   {
      writeCommand(0x15); //  set column address
      writeCommand(0); //  
      writeCommand(0x3f); //    
   
      writeCommand(0x75); // set row address
      writeCommand(k); //  
      writeCommand(0x3f); //  

      for(j=0; j<9; j++)
      {     
         writeData(displayByte[j+9*k] );
      }
   }
#endif   

}

