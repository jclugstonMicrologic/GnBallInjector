/*|***************************************************************************/
/*|PROJECT: Compact Compression CCU                                          */
/*|Module:   extAdcHi                                                        */
/*|Description: function driver interface module to the Analog Devices AD7888*/
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
*|  Module: extAdcHi
*|  Routine: extAdcInit
*|  Description:
*|----------------------------------------------------------------------------
*/
void extAdcInit
(
   UINT8 periphNum_
)
{
   spiInit(periphNum_, 2000000);
} // end extAdcInit()


/*
*|----------------------------------------------------------------------------
*|  Module: extAdcHi
*|  Routine: readExtAdc
*|  Description:
*|----------------------------------------------------------------------------
*/
void readExtAdc
(
   UINT8 channel_,
   UINT8 *rxDataPtr_,
   UINT8 numBytes_
)
{
//   UINT8 adcCh;
   UINT8 adcCh[2];  
   UINT16 cfg;     
   UINT8 periphNum;

   if( channel_ >7 )
   {
      // adc2
      periphNum =SPI_ADC2;
      extAdcInit( periphNum );
   }
   else
   {
      periphNum =SPI_ADC1;
      extAdcInit( periphNum );
   }

   // convert channel_ number back to 0-7
   channel_ =(channel_ % 8);

#ifndef jeff 
   // request next channels data, while reading
   // current channels data
   if( channel_ ==7 )
   {
      // return to channel 0
      channel_ =0;
   }
   else
   {
      // get next channel
      channel_ +=1;
   }
#endif
   cfg =0x3C7F | (channel_<<7);
    
   adcCh[0] =((cfg &0xff00)>>8);
   adcCh[1] =(cfg &0x00ff); // (channel_ <<3) & 0x38;
   spiSendByteD( (UINT8 *)&adcCh, rxDataPtr_, numBytes_, periphNum );      

}











