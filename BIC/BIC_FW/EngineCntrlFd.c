/*|***************************************************************************/
/*| PROJECT: Compact Compression CCU                                         */
/*| Module:   EngineCntrlFd                                                  */
/*| Description:                                                             */
/*|                                                                          */
/*|***************************************************************************/

/*
Tools:
   Compiler: IAR Kickstart development environment for AT91SAM7Sxxx
   ***************************************************************************
   Copyright (C) 2000, Micrologic Limited.
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
#include "board.h"			// Hardware specific header
#include "timer.h"
#include "EngineCntrlFd.h"


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
ENGINE_CONTROL EngineControl;

/*
*|----------------------------------------------------------------------------
*|  Module: EngineCntrlFd
*|  Routine: EngineCntrl_Init
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
void EngineCntrl_Init
(
   void
)
{
   // measurable frequency (22 -???)Hz
   InputCapInit(TIOA0_IN);
   InputCapInit(TIOA4_IN);

   EngineControl.engineSpeed1 =0xffff;
   EngineControl.engineSpeed2 =0xffff;   
   
   EngineControl.engineGov =2050; // ~ half scale of the DAC output
}


void EngineCntrl_Disable
(
   void
)
{
   // Disable the clock and the interrupts
   AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
   AT91C_BASE_TC0->TC_IDR = 0xFFFFFFFF;

   AT91C_BASE_TC4->TC_CCR = AT91C_TC_CLKDIS;
   AT91C_BASE_TC4->TC_IDR = 0xFFFFFFFF;   
   
// disable these as well, they will not be reenabled until reset, 
// they are spares right now, so no problem
AT91C_BASE_TC5->TC_CCR = AT91C_TC_CLKDIS;
AT91C_BASE_TC5->TC_IDR = 0xFFFFFFFF;      

AT91C_BASE_TC6->TC_CCR = AT91C_TC_CLKDIS;
AT91C_BASE_TC6->TC_IDR = 0xFFFFFFFF;      

AT91C_BASE_TC7->TC_CCR = AT91C_TC_CLKDIS;
AT91C_BASE_TC7->TC_IDR = 0xFFFFFFFF;      
}


/*
*|----------------------------------------------------------------------------
*|  Module: EngineCntrlFd
*|  Routine: EngineCntrl_Speed1Isr (TIOa4)
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
void EngineCntrl_Speed1Isr
(
   void
)
{
   StartInputCapture1Isr();
   EngineControl.engineSpeed1 =AT91C_BASE_TC4->TC_RA;

   if( CHECK(CounterOverFlowMask, 0x10) )
      EngineControl.engineSpeed1 =0xffff;

   EndInputCapture1Isr();
}


/*
*|----------------------------------------------------------------------------
*|  Module: EngineCntrlFd
*|  Routine: EngineCntrl_Speed2Isr (TIOa0)
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
void EngineCntrl_Speed2Isr
(
   void
)
{
   StartInputCapture2Isr();
   EngineControl.engineSpeed2 =AT91C_BASE_TC0->TC_RA;

   if( CHECK(CounterOverFlowMask, 0x01) )
      EngineControl.engineSpeed2 =0xffff;

   EndInputCapture2Isr();
}

