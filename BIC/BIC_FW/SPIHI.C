/*|***************************************************************************/
/*|PROJECT:  Compact Compression CCU                                         */
/*|Module:   spiHi                                                           */
/*|Description:                                                              */
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
#include "mk_Std.h"
#include "cpuPorts.h"
#include "timer.h"
#include "spiHi.h"

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/
#define ENABLE_DECODER  AT91C_BASE_PIOB->PIO_SODR =DECODER_ENABLE;
#define DISABLE_DECODER AT91C_BASE_PIOB->PIO_CODR =DECODER_ENABLE;

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
*|------------------------------------------------------------------------
*|  Module: SpiHi
*|  Routine: spiInit
*|  Description:
*|   Initializes IO ports and SPI registers as per the device selected
*|
*|   Note Delay Between Consecutive Transfers =32*DLYBCT/MCK + SCBR/2MCK (FDIV =0)
*|------------------------------------------------------------------------
*/
void spiInit
(
   UINT8 peripheral_,
   UINT32 spiBaudRate_
)
{
   UINT8 baudScaler =0;
   UINT8 dummyRead =0;   

   // First, enable the clock of the SPI
   AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_SPI0) ;

   // enable SPI to transmit and receive data
   AT91C_BASE_SPI0->SPI_CR =AT91C_SPI_SPIEN | AT91C_SPI_LASTXFER;

   // SPI master mode, MCK/N, NPCS0, variable periph select, chip select decoding
   AT91C_BASE_SPI0->SPI_MR =(AT91C_SPI_MSTR | AT91C_SPI_PCSDEC |  AT91C_SPI_PS );

   if( spiBaudRate_ < (MCK_FOSC/256) )
   {
      // baudrate low, first divide master clock by 32
      AT91C_BASE_SPI0->SPI_MR |=AT91C_SPI_FDIV;

      // calculate baud scaler, shift by 8 for register position
      baudScaler =(MCK_FOSC/32/spiBaudRate_);
   }
   else
   {
      // calculate baud scaler, shift by 8 for register position
      baudScaler =(MCK_FOSC/spiBaudRate_);
   }


   // need to adjust CPOL, and clock phase on a per device basis
   switch( peripheral_ )
   {
      case SPI_SR3:
      case SPI_SR1:
         // 48.5uSec delay between transfer (0x48<<24)
         // using formula in description
         // Delay =32*72/48000000 + baudScaler/(2*48000000)
         // this is this delay after a byte has transferred but before the micro negates the CS
         // if burst mode is used, the next clock cycle must be started before this delay expires

         // clock phase, chip select active, 8 bits/transfer, set baud
         AT91C_BASE_SPI0->SPI_CSR[0] =(0x04<<24) | AT91C_SPI_BITS_8 | (baudScaler<<8)  | AT91C_SPI_NCPHA;
         // CPOL high
         AT91C_BASE_SPI0->SPI_CSR[0] |=AT91C_SPI_CPOL;
         // delay before SPCK (after CS)
         AT91C_BASE_SPI0->SPI_CSR[0] |=(0x40<<16);

         //NPCS0
         AT91C_BASE_SPI0->SPI_MR |=(0x00<<16);

         AT91C_BASE_PIOA->PIO_PDR |=SPI0_SS0 | SPI0_SS1 | SPI0_SS2;
         break;
      case SPI_DAC:
         // 48.5uSec delay between transfer (0x48<<24)
         // using formula in description
         // Delay =32*72/48000000 + baudScaler/(2*48000000)
         // this is this delay after a byte has transferred but before the micro negates the CS
         // if burst mode is used, the next clock cycle must be started before this delay expires

         // clock phase, chip select active, 8 bits/transfer, set baud
         AT91C_BASE_SPI0->SPI_CSR[0] =(0x02<<24) | AT91C_SPI_BITS_8 | (baudScaler<<8) | AT91C_SPI_NCPHA;
         // CPOL high
         AT91C_BASE_SPI0->SPI_CSR[0] |=AT91C_SPI_CPOL;
         // delay before SPCK (after CS)
         AT91C_BASE_SPI0->SPI_CSR[0] |=(0x40<<16);

         //NPCS0
         AT91C_BASE_SPI0->SPI_MR |=(0x00<<16);

         AT91C_BASE_PIOA->PIO_PDR |=SPI0_SS0 | SPI0_SS1 | SPI0_SS2;
         break;
      case SPI_ADC1:
         // 48.5uSec delay between transfer (0x48<<24)
         // using formula in description
         // Delay =32*72/48000000 + baudScaler/(2*48000000)
         // this is this delay after a byte has transferred but before the micro negates the CS
         // if burst mode is used, the next clock cycle must be started before this delay expires

         // clock phase, chip select active, 8 bits/transfer, set baud
         AT91C_BASE_SPI0->SPI_CSR[0] =(0x04<<24) | AT91C_SPI_BITS_8 | (baudScaler<<8);
         // CPOL idle high
//         AT91C_BASE_SPI0->SPI_CSR[0] |=AT91C_SPI_CPOL;
         // delay before SPCK (after CS)
         AT91C_BASE_SPI0->SPI_CSR[0] |=(0x40<<16);
         
         AT91C_BASE_SPI0->SPI_CSR[0] |=0x02;

         //NPCS0
         AT91C_BASE_SPI0->SPI_MR |=(0x00<<16);

         AT91C_BASE_PIOA->PIO_PDR |=SPI0_SS0 | SPI0_SS1 | SPI0_SS2;
         
dummyRead =AT91C_BASE_SPI0->SPI_RDR;                 
         break;
      case SPI_ADC2:
         // 48.5uSec delay between transfer (0x48<<24)
         // using formula in description
         // Delay =32*72/48000000 + baudScaler/(2*48000000)
         // this is this delay after a byte has transferred but before the micro negates the CS
         // if burst mode is used, the next clock cycle must be started before this delay expires

         // CPOL high, clock phase, chip select active, 8 bits/transfer, set baud
         AT91C_BASE_SPI0->SPI_CSR[1] =(0x04<<24) | AT91C_SPI_BITS_8 | (baudScaler<<8);


//         AT91C_BASE_SPI0->SPI_CSR[1] |=AT91C_SPI_CPOL;
         // delay before SPCK (after CS)
         AT91C_BASE_SPI0->SPI_CSR[1] |=(0x40<<16);

         AT91C_BASE_SPI0->SPI_CSR[1] |=0x02;
         
         //NPCS0
         AT91C_BASE_SPI0->SPI_MR |=(0x00<<16);

         AT91C_BASE_PIOA->PIO_PDR |=SPI0_SS0 | SPI0_SS1 | SPI0_SS2;
         
dummyRead =AT91C_BASE_SPI0->SPI_RDR; 
         break;
      case SPI_FLASH:
         // 48.5uSec delay between transfer (0x48<<24)
         // using formula in description
         // Delay =32*72/48000000 + baudScaler/(2*48000000)
         // this is this delay after a byte has transferred but before the micro negates the CS
         // if burst mode is used, the next clock cycle must be started before this delay expires

         // CPOL high, clock phase, chip select active, 8 bits/transfer, set baud
         AT91C_BASE_SPI0->SPI_CSR[1] =(0x08<<24) | AT91C_SPI_BITS_8 | (baudScaler<<8);

         AT91C_BASE_SPI0->SPI_CSR[1] |=AT91C_SPI_CPOL;

//AT91C_BASE_SPI->SPI_CSR[1] |=AT91C_SPI_NCPHA;

         // delay before SPCK (after CS)
         AT91C_BASE_SPI0->SPI_CSR[1] |=(0x80<<16); // | AT91C_SPI_CSAAT;

         //NPCS0
         AT91C_BASE_SPI0->SPI_MR |=(0x00<<16);

         AT91C_BASE_PIOA->PIO_PDR |=SPI0_SS0 | SPI0_SS1 | SPI0_SS2;
         break;
      case SPI_RTC:

         // 48.5uSec delay between transfer (0x48<<24)
         // using formula in description
         // Delay =32*72/48000000 + baudScaler/(2*48000000)
         // this is this delay after a byte has transferred but before the micro negates the CS
         // if burst mode is used, the next clock cycle must be started before this delay expires

         // CPOL high, clock phase, chip select active, 8 bits/transfer, set baud
         AT91C_BASE_SPI0->SPI_CSR[1] =(0x24<<24) | AT91C_SPI_BITS_8 | (baudScaler<<8);

//         AT91C_BASE_SPI0->SPI_CSR[1] |=AT91C_SPI_CPOL;

//AT91C_BASE_SPI0->SPI_CSR[1] |=AT91C_SPI_NCPHA;
AT91C_BASE_SPI0->SPI_CSR[1] |=0x02;

         // delay before SPCK (after CS)
         AT91C_BASE_SPI0->SPI_CSR[1] |=(0x80<<16);

         //NPCS0
         AT91C_BASE_SPI0->SPI_MR |=(0x00<<16);

         AT91C_BASE_PIOA->PIO_PDR |=SPI0_SS0 | SPI0_SS1 | SPI0_SS2;
         break;
      case SPI_SR2:

         // 48.5uSec delay between transfer (0x48<<24)
         // using formula in description
         // Delay =32*72/48000000 + baudScaler/(2*48000000)
         // this is this delay after a byte has transferred but before the micro negates the CS
         // if burst mode is used, the next clock cycle must be started before this delay expires

         // clock phase, chip select active, 8 bits/transfer, set baud
         AT91C_BASE_SPI0->SPI_CSR[1] =(0x04<<24) | AT91C_SPI_BITS_8 | (baudScaler<<8) | AT91C_SPI_NCPHA;
         // CPOL high
         AT91C_BASE_SPI0->SPI_CSR[1] |=AT91C_SPI_CPOL;
         // delay before SPCK (after CS)
         AT91C_BASE_SPI0->SPI_CSR[1] |=(0x40<<16);

         //NPCS0
         AT91C_BASE_SPI0->SPI_MR |=(0x00<<16);

         AT91C_BASE_PIOA->PIO_PDR |=SPI0_SS0 | SPI0_SS1 | SPI0_SS2;
         break;
                           
   }

// other SPI delays
// AT91C_BASE_SPI->SPI_MR |=(100<<24);      // delay before switching CS (CS1 to CS2)

}// end spiInit()




/*
*|----------------------------------------------------------------------------
*|  Module: Spihi
*|  Routine: spiSendByteD
*|  Description:
*|   Sends a byte over the SPI and waits for completion of the transfer.
*|   As normal for the SPI, data can also be received during this process
*|   D is for spi decode chip select (4-16 decoder)
*|----------------------------------------------------------------------------
*/
BOOL spiSendByteD
(
   UINT8 *txByte_,
   UINT8 *rxBytePtr_,
   UINT16 numBytes_,     // number of bytes to transfer before CS deasserted
   UINT8 spiSelect_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT16 j;
   int dummy=0;


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

   // disable timer interrupt, interferes with SPI
//   AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

   ENABLE_DECODER;

   for(j=0; j<numBytes_; j++)
   {
      /* initiate the transmission, and select the device*/
      AT91C_BASE_SPI0->SPI_TDR =( *(txByte_ +j)  | ( spiSelect_<<16) );

//while( dummy ++ <100){}
//dummy =0;


      /* wait for the transmission */
      while( CHECK( AT91C_BASE_SPI0->SPI_SR, AT91C_SPI_TDRE )==0 )
      {
         /* transmission not completed */
         if( dummy++ > 1000) /*! arbitrary wait period */
         {
            // reenable timer interrupt
//            AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

            DISABLE_DECODER;
            return false;
         }
      }

      /* transfer completed, get any rx data and clear spif bit by reading SP0DR */
      if( rxBytePtr_ == NULL_PTR )
      {
         continue;
      }
      if( rxBytePtr_ != NULL_PTR)
      {
         dummy =0;
        
         while( CHECK( AT91C_BASE_SPI0->SPI_SR, AT91C_SPI_RDRF )==0 )
         {
            /* wait for received data */
            if( dummy++ > 1000) /*! arbitrary wait period */
            {
               // reenable timer interrupt
//               AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

               DISABLE_DECODER;
               return false;
            }
         }

         /* get value read from SPI bus */
         *(rxBytePtr_ ++) =AT91C_BASE_SPI0->SPI_RDR;
      }

   }

   while( CHECK( AT91C_BASE_SPI0->SPI_SR, AT91C_SPI_TXEMPTY )==0 )
   {
      // ensure last byte is transmitted before SPI access is allowed again
      if( dummy++ > 1000) /*! arbitrary wait period */
      {
         // reenable timer interrupt
//         AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

         DISABLE_DECODER;
         return false;
      }
   }

   dummy =dummy;

   DISABLE_DECODER;

   // reenable timer interrupt
//   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

   return true;

} /* end spiSendByteD() */


#ifdef jeff
/*|------------------------------------------------------------------------
  |  Module:
  |  Routine: spiIsr
  |  Description: Called after the eighth SCK cycle in a data transfer
  |  reset by reading the SP0SR register, followed by an access
  | (read or write) to the SPI data register
  |------------------------------------------------------------------------
*/
interrupt void spiInterrupt(void)
{
   UNSET(SP0CR1, SP0CR1_SPIE);
}/* spiInterrupt() */
#endif



