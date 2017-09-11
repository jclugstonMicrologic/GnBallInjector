/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*|Module:  TaskManager                                                      */
/*|Description: executes all predefined modules within an infinite loop      */
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

* Author               Date        Comment
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Jeff Clugston	      Dec. 02/2012
*
*****************************************************************************
*/

/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "lcdFd.h"
#include "timer.h"
#include "TaskManager.h"
#include "SwitchFd.h"
#include "KeypadFd.h"
#include "PwmDriverFd.h"
#include "AdcFd.h"
#include "motorcntrl.h"

#include "string.h"

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/


/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/

enum
{
   SYSTEM_IDLE_STATE,
   SYSTEM_USB_DETECTED_STATE,
   SYSTEM_CONTROLLER_STATE,

   SYSTEM_LAST_STATE
};


/*
*****************************************************************************
 P R I V A T E   G L O B A L   D A T A
*****************************************************************************
*/

// main system state (usb connect/disconeect, compressor monitor, etc
UINT8 SystemState;

UINT32 DebugTimer;

/*
*****************************************************************************
 P R I V A T E   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/
void SystemMachine( void );

/*
*|----------------------------------------------------------------------------
*|  Module: TaskManager
*|  Routine: TaskManager
*|  Description:
*|----------------------------------------------------------------------------
*/

void TaskManager
(
   void
)
{
   int j=0;  
   UINT32 lcdUpdateTimer;   
  
   lcdUpdateTimer =TicksLeft;
   
   while( true )
   {
      SystemMachine();

      // get all enabled channels in raw format
      MeasureAnalog(0, true, false);

      if( (TicksLeft -lcdUpdateTimer) >100 )
      {
         // 0xff no switch
         lcdUpdateTimer =TicksLeft;
         KeypadMachine(KEY_UPDATE_SCREEN);         
      }
      
      KeypadMachine(KeyActionRequest);            
      
      for(j=0; j<MAX_NUM_SWITCHES; j++)
      {
         // debounce the switch input
         SwitchMachine(j);
      }

      LcdMachine();

      MotorRunMachine();
      
      // service the WDT
      KickWdt();

      
   } // end while(true)

} // end TaskManager()


/*
*|----------------------------------------------------------------------------
*|  Module: TaskManager
*|  Routine:SystemMachine
*|  Description: monitors the mains of the system such as USB plug in
*|----------------------------------------------------------------------------
*/
void SystemMachine
(
   void
)
{

   switch( SystemState )
   {
      case SYSTEM_IDLE_STATE:
         // check for USB plugin
         if( (AT91C_BASE_PIOA->PIO_PDSR & USB_DETECT ) ==USB_DETECT )
         {
            // USB plugged in

            // enable USB 48 MHz clock
            AT91C_BASE_PMC->PMC_SCER |= AT91C_PMC_UDP;
         
            SystemState =SYSTEM_USB_DETECTED_STATE;
         }
         else // USB not plugged in
         {
            // disable USB 48 MHz clock
            AT91C_BASE_PMC->PMC_SCER &= ~AT91C_PMC_UDP;
      
            // just reuse this for start up buzz
            BUZZER_ON;      
            OsStartOneShotTimer( &OsTimer[TIMER_KEYPAD_BUZZER], 500 );      
      
            SystemState =SYSTEM_CONTROLLER_STATE;
         }
         break;
      case SYSTEM_USB_DETECTED_STATE:
         break;
      case SYSTEM_CONTROLLER_STATE:
         break;

   } // end switch( SystemState )


} // end SystemMachine()


/*
*|----------------------------------------------------------------------------
*|  Module: TaskManager
*|  Routine: InitTargetHw
*|  Description: Initialize all external and internal hardware
*|               Initialize all modules
*|----------------------------------------------------------------------------
*/
void InitTargetHw
(
   void
)
{
   // enable brownout detect, set GPNVM bit0

   // cannot be done in app. code, must be performed at flash program time
   // AT91C_BASE_MC->MC_FCR =0x5a00000b;

   // Enable User Reset and set its minimal assertion to 960 us
//   AT91C_BASE_RSTC->RSTC_RMR = AT91C_SYSC_URSTEN | (0x4<<8) | (unsigned int)(0xA5<<24);

   // initialize the WDT, currently set for 4second timeout when enabled
   InitWdt(false);

   // initialize timers
   TimerInit();
 	
   // initialize cpu ports for, GPIO, peripheral designations
   InitCpuPorts();

   // initialize the LCD
   InitLcd();

   // initialize keypad callback routines
   InitKeypad();

   // initialize internal(on board) ADC
   InitIntrnlAdc();
   
   // initialize the PWM driver
//   InitPwm();
  
   InputCapInit(TIOA7_IN);
   
//   Pwm_InitDriver();
//   Pwm_SetDuty(0);
   
   MotorStrtUpInit();
 
}






