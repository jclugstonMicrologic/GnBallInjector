/*|***************************************************************************/
/*|PROJECT: Compact Compression CCU                                          */
/*|Module:  UsbComFd                                                         */
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


/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "board.h"
#include "dbgu.h"
#include "cdc_enumerate.h"
#include "cpuports.h"
#include "rtcFd.h"
#include "extAdcFd.h"
#include "memoryFd.h"
#include "PidFd.h"
#include "CompressorMachineFd.h"

#include "C:\Software\CompactCompressors\CCU\commonCCU\parseFd.h"

#include "UsbComFd.h"
#include "DacFd.h"

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

struct _AT91S_CDC 	pCDC;


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
TOOL_CONFIG_HEADER ToolConfigHeader;

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
*|  Module: UsbComFd
*|  Routine: InitUsbCom
*|  Description:
*|----------------------------------------------------------------------------
*/
void InitUsbCom
(
   void
)
{
   // populate the fw version info.
   strcpy(ToolConfigHeader.fwVersion, CCU_VERSION_INFO);

   // populate the cpu serial number (this actually suppose to be set by the programmer)
   // programmer is whoever is programming the tool with the latest fw
   strcpy(ToolConfigHeader.cpuSerialNum, "1234ABCD");

   // initialize USB device
   AT91F_USB_Open();

} // end InitUsbCom()

/*
*|----------------------------------------------------------------------------
*|  Module: UsbComFd
*|  Routine: UsbEnumeration
*|  Description: checks if the USB device has been enumerated
*|----------------------------------------------------------------------------
*/
BOOL UsbEnumeration(void)
{
   if (pCDC.IsConfigured(&pCDC))
      return true;
      return false;
}


//#define DEBUG_MODE
/*
*|----------------------------------------------------------------------------
*|  Module: UsbComFd
*|  Routine: AT91F_USB_Open
*|  Description: This function Open the USB device
*|----------------------------------------------------------------------------
*/
void AT91F_USB_Open(void)
{
//#define EVAL_BOARD

#ifndef DEBUG_MODE
    // Set the PLL USB Divider
   AT91C_BASE_CKGR->CKGR_PLLR |= AT91C_CKGR_USBDIV_1 ;

   // Specific Chip USB Initialisation
   // Enables the 48MHz USB clock UDPCK and System Peripheral USB Clock
   AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_UDP;
   AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_UDP);

   // Enable UDP PullUp (USB_DP_PUP) : enable & Clear of the corresponding PIO
   // Set in PIO mode and Configure in Output
#ifdef EVAL_BOARD
   AT91F_PIO_CfgOutput(AT91C_BASE_PIOA,AT91C_PIO_PA16);
   // set the Pull up resistor
   AT91F_PIO_ClearOutput(AT91C_BASE_PIOA,AT91C_PIO_PA16);
#else
   AT91F_PIO_CfgOutput(AT91C_BASE_PIOA,AT91C_PIO_PA1);
//   AT91F_PIO_SetOutput(AT91C_BASE_PIOA,AT91C_PIO_PA1);
   AT91F_PIO_ClearOutput(AT91C_BASE_PIOA,AT91C_PIO_PA1);
#endif

   // CDC Open by structure initialization
   AT91F_CDC_Open(&pCDC, AT91C_BASE_UDP);
#endif
}

/*
*|----------------------------------------------------------------------------
*|  Module: UsbComFd
*|  Routine: BuildHeader
*|  Description: Builds the serial message header to transmit to PC
*|----------------------------------------------------------------------------
*/
void BuildHeader(UINT16 msgLength_, char *headerDataPtr_, UINT8 theCommand_, UINT32 startAddr_)
{
#ifndef DEBUG_MODE

    // copy header into txData buffer
   *(headerDataPtr_ ++) =0x23; //COM_HEADER[0];
   *(headerDataPtr_ ++) =0x08; //COM_HEADER[1];
   *(headerDataPtr_ ++) =(msgLength_ &0xff00)>>8;
   *(headerDataPtr_ ++) =(msgLength_ &0x00ff);

   // echo back command received
   *(headerDataPtr_ ++) =theCommand_;

   *(headerDataPtr_ ++) =(startAddr_ &0xff000000)>>24;
   *(headerDataPtr_ ++) =(startAddr_ &0x00ff0000)>>16;
   *(headerDataPtr_ ++) =(startAddr_ &0x0000ff00)>>8;
   *(headerDataPtr_ ++) =(startAddr_ &0x000000ff);

   *(headerDataPtr_ ++) =ACK;

#endif

}

/*
*|----------------------------------------------------------------------------
*|  Module: UsbComFd
*|  Routine: CalcCrc
*|  Description:
*|   Calculates CRC on a buffer of data.
*|----------------------------------------------------------------------------
*/
UINT16 CalcCrc                     /*| returns CRC value            */
(                                  /*|                              */
   UINT8 *dataBufPtr_,             /*|                              */
   UINT16 numBytes_                /*|                              */
)                                  /*|                              */
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   int i;
   int x;
   unsigned int crc, feed;
   unsigned char *tempPtr;

#ifndef DEBUG_MODE
   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */
   /* Check for uninitialized pointers, out of bounds inputs, etc */


   /*
   *************************
    C O D E
   *************************
   */

   crc = 0;
   for ( tempPtr = dataBufPtr_;
         tempPtr < (dataBufPtr_ + numBytes_);
         tempPtr++
       )
   {
      feed = (unsigned int)(*tempPtr);
      for ( i=0; i<8; i++)
      {
         x = crc & 0x0001;
         crc= crc>>1;
         if ( feed & 0x0001 )
         {
            crc = crc | 0x8000;
         }
         feed = feed>>1;
         if ( x )
         {
            crc = crc ^ 0x1021;
         }
      }
   }

#endif
   return crc;
}/* end CalcCrc() */



/*
*|----------------------------------------------------------------------------
*|  Module: UsbComFd
*|  Routine: UsbCommand
*|  Description: Process received commands
*|               Act on received commands
*|               Return a serial response to master controller
*|----------------------------------------------------------------------------
*/
void UsbComm
(
   void
)
{

#ifndef DEBUG_MODE

   UINT16 crc;
   char rxData[RX_BUF_SIZE +SIZEOF_COM_HEADER+sizeof(crc)];
   char txData[TX_BUF_SIZE +SIZEOF_COM_HEADER+sizeof(crc)];
   UINT8 theCommand;
   UINT8 ackByte;

   UINT32 length;

   UINT32 theStartAddr; // flash address

   UINT32 endOfLogAddress;

   // length of received msg
   length = pCDC.Read(&pCDC, rxData, RX_BUF_SIZE);

   if( length ==0 )
      return;

   theCommand =rxData[SERIAL_COMMAND];

   theStartAddr =rxData[MSB_START_ADDR]<<24;
   theStartAddr +=rxData[MSB_START_ADDR+1]<<16;
   theStartAddr +=rxData[MSB_START_ADDR+2]<<8;
   theStartAddr +=rxData[MSB_START_ADDR+3];

   ackByte =(rxData[ACK_BYTE]);

   // suppress compiler warning
   ackByte =ackByte;

   // clear the tx buffer
   memset(txData, 0x00, sizeof(txData));

   switch( theCommand )
   {
      case MSG_GET_TIME: // get real time
         GetRtcTime( &RealTime );

         length =SIZEOF_COM_HEADER + sizeof(RealTime) +sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr); 

         // copy data to transmit into txData
         memcpy( &txData[START_OF_DATA], &RealTime, sizeof(RealTime) );
         break;
      case MSG_SET_TIME: //set real time
         // copy the time received into RealTime structure
         memcpy(&RealTime, &rxData[START_OF_DATA], sizeof(RealTime));

         // set RTC hardware with new PC time
         SetRtcTime(&RealTime);

         length =SIZEOF_COM_HEADER + sizeof(RealTime) +sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

         // copy data to transmit into txData
         memcpy( &txData[START_OF_DATA], &RealTime, sizeof(RealTime) );
         break;
      case MSG_READ_FLASH_SECTOR: // read local flash
         length =FLASH_BYTES_PER_SECTOR+SIZEOF_COM_HEADER+sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

         // read flash starting at theStartAddr, populate read data into txData
         ReadFromSector(theStartAddr, (UINT8 *)&txData[START_OF_DATA], FLASH_BYTES_PER_SECTOR);
         break;
      case MSG_WRITE_FLASH_SECTOR: // write local flash
         length =SIZEOF_COM_HEADER + sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

         if( theStartAddr  ==0 )
            EraseFlash(theStartAddr);
            
         if( WriteToSector(theStartAddr, (UINT8 *)&rxData[START_OF_DATA], FLASH_BYTES_PER_SECTOR) )
         {           
            // flash write passed
            txData[ACK_BYTE] =ACK;
            
            // reinit pid parameters
            InitPid();
         }
         else
         {
            // flash write failed
            txData[ACK_BYTE] =NO_ACK;
         }        
         break;
      case MSG_ERASE_FLASH_BLOCK:
         length =SIZEOF_COM_HEADER + sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

         EraseFlash(theStartAddr);
         break;
      case MSG_ERASE_FLASH_ALL:
         length =SIZEOF_COM_HEADER + sizeof(crc);
         BuildHeader( length, txData, theCommand, theStartAddr);
//         EraseFlashAll();
         break;
      case MSG_GET_EXT_ADC_SENSORS: // get ext ADC readings
         length =SIZEOF_COM_HEADER + sizeof(AdcReading) +sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

// overwrite these values for now, debug purposes
//AdcReading[2] =pTerm;
//AdcReading[3] =iTerm;
//AdcReading[4] =dTerm;
//AdcReading[5] =inletPressAdjustValue;
//AdcReading[6] =loPressRecycleAdjustValue;
//AdcReading[7] =hiPressRecycleAdjustValue;

         // transmit all AdcReadings to the master controller
         memcpy( &txData[START_OF_DATA], &AdcReading, sizeof(AdcReading));
         break;
      case MSG_SET_DAC0:
         memcpy(&DacAdjustValue[0], &rxData[START_OF_DATA], sizeof(DacAdjustValue[0]) );

         length =SIZEOF_COM_HEADER + sizeof(DacAdjustValue[0]) +sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

         // copy data to transmit into txData
         memcpy( &txData[START_OF_DATA], &DacAdjustValue[0], sizeof(DacAdjustValue[0]) );

         AdjustPressure(0, DacAdjustValue[0]);
         break;
      case MSG_SET_DAC1:
         memcpy(&DacAdjustValue[1], &rxData[START_OF_DATA], sizeof(DacAdjustValue[1]) );

         length =SIZEOF_COM_HEADER + sizeof(DacAdjustValue[1]) +sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

         // copy data to transmit into txData
         memcpy( &txData[START_OF_DATA], &DacAdjustValue[1], sizeof(DacAdjustValue[1]) );

         AdjustPressure(1, DacAdjustValue[1]);
         break;
      case MSG_RUN_COMPRESSOR:
         CompressorData.comprMachState =5; //COMPRESSOR_RUN_STATE

         length =SIZEOF_COM_HEADER + sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

         txData[ACK_BYTE] =ACK;
         break;
      case MSG_GET_LOG_MEM_END:
         length =SIZEOF_COM_HEADER + sizeof(endOfLogAddress) + sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

//         endOfLogAddress =FindLogMemoryEnd(0);

         // copy data to transmit into txData
         memcpy( &txData[START_OF_DATA], &endOfLogAddress, sizeof(endOfLogAddress) );
         break;
      case MSG_GET_MICRO_FW_SN:
         length =SIZEOF_COM_HEADER + sizeof(ToolConfigHeader.cpuSerialNum)+
                                     sizeof(ToolConfigHeader.fwVersion) + sizeof(crc);

         BuildHeader( length, txData, theCommand, theStartAddr);

         memcpy( &txData[START_OF_DATA], &ToolConfigHeader.cpuSerialNum, sizeof(ToolConfigHeader.cpuSerialNum) );
         memcpy( &txData[START_OF_DATA+sizeof(ToolConfigHeader.cpuSerialNum)], &ToolConfigHeader.fwVersion, sizeof(ToolConfigHeader.fwVersion) );
         break;
      case MSG_SW_RESET:
//         while(true){}
         break;

   } // end switch(theCommand)


   // calculate crc value based on message string to transmit
   crc =CalcCrc( (UINT8 *)&txData, (length-sizeof(crc)) );

   // append the crc to the message string
   memcpy( &txData[length-sizeof(crc)], &crc, sizeof(crc));

   pCDC.Write(&pCDC, txData, length); //length);

#endif

}// end UsbCommand




