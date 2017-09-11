/*|***************************************************************************/
/*|PROJECT: Compact Compression CCU                                          */
/*|Module:   extAdcHi                                                        */
/*|Description: function driver module to the Analog Devices AD7888          */
/*|                                                                          */
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
#include "extAdcHi.h"
#include "extAdcFd.h"
#include "timer.h"

#include "C:\Software\CompactCompressors\CCU\commonCCU\parseFd.h"


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


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
UINT16 AdcReading[16];

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
*|  Module: extAdcFd
*|  Routine: InitExtAdc
*|  Description:
*|----------------------------------------------------------------------------
*/
void InitExtAdc
(
   void
)
{
   UINT16 cfg;
   int j;
   int waitTimer;
   
   cfg =0x3FFF;

   extAdcInit(SPI_ADC1);   
   spiSendByteD( (UINT8 *)&cfg, NULL_PTR, 2, SPI_ADC1 );
   spiSendByteD( (UINT8 *)&cfg, NULL_PTR, 2, SPI_ADC1 );   

   for( j=0; j<9; j++)
      MeasureExtAnalog( 0 );  

   // delay before initializing next ADC
   waitTimer =TicksLeft;
//   while( TicksLeft -waitTimer <100){}
     
   extAdcInit(SPI_ADC2);   
   spiSendByteD( (UINT8 *)&cfg, NULL_PTR, 2, SPI_ADC2 );         
   spiSendByteD( (UINT8 *)&cfg, NULL_PTR, 2, SPI_ADC2 );

   for( j=0; j<9; j++)
      MeasureExtAnalog( 8 );     
} // end InitExtAdc()


/*
*|----------------------------------------------------------------------------
*|  Module: extAdcFd
*|  Routine: MeasureExtAnalog
*|  Description:
*|----------------------------------------------------------------------------
*/
UINT16 MeasureExtAnalog
(
   UINT8 channel_
)
{
   UINT8 tempData[4];

   readExtAdc(channel_, (UINT8 *)&tempData, 2);

   return ( ((tempData[0] &0xff)<<8) + (tempData[1] &0xff) );
}


void GetAllAnalog
(
   UINT16 *adcReadingPtr_
)
{   
   // get all adc channels, chip1
   *(adcReadingPtr_++) =MeasureExtAnalog( 0 );  
   *(adcReadingPtr_++) =MeasureExtAnalog( 1 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 2 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 3 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 4 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 5 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 6 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 7 );
   
   // get all adc channels, chip2
   *(adcReadingPtr_++) =MeasureExtAnalog( 8 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 9 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 10 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 11 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 12 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 13 );
   *(adcReadingPtr_++) =MeasureExtAnalog( 14 );
   *adcReadingPtr_     =MeasureExtAnalog( 15 );
}











