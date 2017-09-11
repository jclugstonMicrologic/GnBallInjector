/*|***************************************************************************/
/*|PROJECT:  Compact Compressor CCU                                          */
/*|Module:   spiHi                                                           */
/*|Description:                                                              */
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
#ifndef SPI_HI_H
 #define SPI_HI_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "cpuPorts.h"

/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/


/*
*****************************************************************************
 P U B L I C    M A C R O S
*****************************************************************************
*/


/* enumerate the recognised SPI device */
enum
{
   SPI_SR3 =0,
   SPI_SR1,     // shift register

   SPI_DAC,

   SPI_ADC1,
   SPI_ADC2,

   SPI_RTC,
   SPI_FLASH,

   SPI_SR2
};


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/


/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/

/*
*|------------------------------------------------------------------------
*|  Module:
*|  Routine: spiInit
*|  Description:
*|------------------------------------------------------------------------
*/
void spiInit                /*|                                         */
(                           /*|                                         */
   UINT8 peripheral_,    /*|                                         */
   UINT32 baudRate_
);                          /*|                                         */




/*
*|----------------------------------------------------------------------------
*|  Module: SPI
*|  Routine: spiSendByte
*|  Description:
*|   Sends the requested byte over the SPI bus and waits for the
*|   transfer to complete.  The byte read back from the SPI is also available
*|----------------------------------------------------------------------------
*/
BOOL spiSendByteD     /*| TRUE is transfer succeeded, FALSE otherwise         */
(                     /*|                                                     */
   UINT8 *txByte_,     /*| byte to be sent out                                 */
   UINT8 *rxBytePtr_, /*| data read back from SPI stored here if not NULL_PTR */
   UINT16 numBytes_,  /*|  number of bytes to transfer before CS deasserted   */
   UINT8 spiSelect_
);

/* spiHi.h */

#endif



