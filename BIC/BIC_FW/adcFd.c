/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*| Module:   adcFd                                                          */
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
#include "adcFd.h"


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
INTRNL_ADC_READING IntrnlAdcReading;
INTRNL_ADC_READING IntrnlMinAdcReading;

/*
*|----------------------------------------------------------------------------
*|  Module: adcFd
*|  Routine: InitIntrnlAdc
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
void InitIntrnlAdc
(
   void
)
{
   adcInit();
   
   IntrnlMinAdcReading.battery =20000;
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: MeasureAnalog
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
BOOL MeasureAnalog
(
   UINT8 channel_,
   BOOL trigHw_,    // false allows us to request ADC readings immediately
   BOOL rawValue_   // get raw or real value
)
{
   if( trigHw_ )
   {
      // start a conversion (hw trigger)
      AT91C_BASE_ADC1->ADC_MR |=0x0d;
   }
   else
   {
      // disable hw triger
      AT91C_BASE_ADC1->ADC_MR &=0xfffffff2;

      // start a conversion (software trigger)
      AT91C_BASE_ADC1->ADC_CR =AT91C_ADC_START;
   }

   GetAdcValue(channel_, (UINT16 *)&IntrnlAdcReading, rawValue_);

   if( IntrnlAdcReading.battery <IntrnlMinAdcReading.battery)
      IntrnlMinAdcReading.battery =IntrnlAdcReading.battery;
   
   return true;
}


