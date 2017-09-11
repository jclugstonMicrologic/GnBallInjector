/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*| Module:   adcHi                                                          */
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
#include "cpuPorts.h"
#include "adcHi.h"

#include "string.h"

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/


/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
enum
{
   ADC_CH0 =0,
   ADC_CH1,
   ADC_CH2,
   ADC_CH3,
   ADC_CH4,
   ADC_CH5,
   ADC_CH6,
   ADC_CH7,
   ADC_CHALL
};

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/

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



/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/

/*
*|----------------------------------------------------------------------------
*|  Module: adcHi
*|  Routine: adcInit
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
void adcInit
(
   void
)
{
   AT91C_BASE_ADC1->ADC_CR =AT91C_ADC_SWRST;

   // select 10 bit resolution, hw trigger enabled (rising edge ADTRG), sleep mode
   AT91C_BASE_ADC1->ADC_MR =AT91C_ADC_LOWRES_10_BIT |0x2000 | 0x0d;

   // enable ADC channel(s)
   AT91C_BASE_ADC1->ADC_CHER =AT91C_ADC_CH0 | AT91C_ADC_CH1;                              
                           //   AT91C_ADC_CH2 | AT91C_ADC_CH3 |
                           //   AT91C_ADC_CH4 | AT91C_ADC_CH5 |
                           //   AT91C_ADC_CH6 | AT91C_ADC_CH7;

// disable ADC channel(s)
//   AT91C_BASE_ADC->ADC_CHDR =AT91C_ADC_CH4;


} // end adcInit()


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: GetAdcValue
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
BOOL GetAdcValue
(
   UINT8 channel_,
   UINT16 *adcValuePtr_,
   BOOL rawValue_  // return raw or real value
)
{
   UINT16 tempData[8];

   #define VOLTAGE_DIV0_VREF_CONV ((UINT32)7331) // 1/(10k/(10k +20k)) *2500/1023 *1000
   #define VOLTAGE_DIV1_VREF_CONV ((UINT32)5147) // 1/(4.7k/(4.7k + 4.7k+0.499k)) *2500/1023*1000
   #define VOLTAGE_DIV2_VREF_CONV ((UINT32)244)  // 2500/1023*100 (100 DegC max)
   #define VOLTAGE_DIV3_VREF_CONV ((UINT32)4888) // 1/(4.7k/(4.7k + 4.7k))*2500/1023*1000

   switch( channel_ )
   {
      case ADC_CH0:
         AT91C_BASE_ADC1->ADC_CR =AT91C_ADC_START;
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC0) !=AT91C_ADC_EOC0  ) { /*wait for conversion to complete*/ }

         tempData[0] =AT91C_BASE_ADC1->ADC_CDR0;
      break;
      case ADC_CH1:
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC1) !=AT91C_ADC_EOC1 ) { /*wait for conversion to complete*/ }

         tempData[1] =AT91C_BASE_ADC1->ADC_CDR1;
      break;
      case ADC_CH2:
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC2) !=AT91C_ADC_EOC2 ) { /*wait for conversion to complete*/ }

         tempData[2] =AT91C_BASE_ADC1->ADC_CDR2;
      break;
      case ADC_CH3:
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC3) !=AT91C_ADC_EOC3 ) { /*wait for conversion to complete*/ }

         tempData[3] =AT91C_BASE_ADC1->ADC_CDR3;
      break;
      case ADC_CH4:
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC4) !=AT91C_ADC_EOC4 ) { /*wait for conversion to complete*/ }

         tempData[4] =AT91C_BASE_ADC1->ADC_CDR4;
      break;
      case ADC_CH5:
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC5) !=AT91C_ADC_EOC5 ) { /*wait for conversion to complete*/ }

         tempData[5] =AT91C_BASE_ADC1->ADC_CDR5;
      break;
      case ADC_CH6:
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC6) !=AT91C_ADC_EOC6 ) { /*wait for conversion to complete*/ }

         tempData[6] =AT91C_BASE_ADC1->ADC_CDR6;
         break;
      case ADC_CH7:
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC7) !=AT91C_ADC_EOC7 ) { /*wait for conversion to complete*/ }

         tempData[7] =AT91C_BASE_ADC1->ADC_CDR7;
         break;
      case ADC_CHALL: // do all
         AT91C_BASE_ADC1->ADC_CR =AT91C_ADC_START;

         // wait for channel7 to complete (!!!must be enabled!!!)
         while( (AT91C_BASE_ADC1->ADC_SR &AT91C_ADC_EOC7) !=AT91C_ADC_EOC7 ) { /*wait for conversion to complete*/ }

         // get channels 0-7
         tempData[0] =AT91C_BASE_ADC1->ADC_CDR0;
         tempData[1] =AT91C_BASE_ADC1->ADC_CDR1;
         tempData[2] =AT91C_BASE_ADC1->ADC_CDR2;
         tempData[3] =AT91C_BASE_ADC1->ADC_CDR3;
         tempData[4] =AT91C_BASE_ADC1->ADC_CDR4;
         tempData[5] =AT91C_BASE_ADC1->ADC_CDR5;
         tempData[6] =AT91C_BASE_ADC1->ADC_CDR6;
         tempData[7] =AT91C_BASE_ADC1->ADC_CDR7;
         break;
   }

   if( !rawValue_ )
   {
      tempData[1] = tempData[1];
      tempData[2] = tempData[2];
      tempData[3] = tempData[3];
      tempData[4] = tempData[4];
      tempData[5] = tempData[5];
      tempData[6] = tempData[6];
      tempData[7] = tempData[7];
      
      tempData[0] =(UINT32)((UINT32)tempData[0]*16200)/1024;      
   }

   memcpy(adcValuePtr_, tempData, sizeof(tempData));

   return true;

} // end GetAdcValue()







