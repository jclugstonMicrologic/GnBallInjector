/*
*|**************************************************************************
*|PROJECT:  Compact Compression CCU
*|Module:   Memory Hi
*|Description:
*|**************************************************************************
*/

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
#include "spiHi.h"
#include "memoryHi.h"

#include "timer.h"

#include "string.h"

/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/

/* Include the following to enable debug messages. It's structured to allow*/
/* debugging to be turned on or off for just this module but in the DEBUG  */
/* module, all debugging can also be turned off */
#define DEBUG_MEMORY_HI_ON   0

#if DEBUG_MEMORY_HI_ON ==1
 #define DEBUG_MEMORY_HI(message)   DEBUG(message, THIS_MODULE'S_TASK_ID)
#else
 #define DEBUG_MEMORY_HI(message) /* */
#endif


#define WAIT_FOR_PROGRAMMING(numBytes_, verify_) WaitForProgramming(numBytes_, verify_);


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
UINT32 LinearAddress;

BANK2 UINT8 DeviceNum;

union
{
   UINT16 SectorNum;

   struct
   {
      UINT8 SectorNumLo;
      UINT8 SectorNumHi;
   };
};

union
{
   UINT16 ByteAddress;

   struct
   {
      UINT8 ByteAddressLo;
      UINT8 ByteAddressHi;
   };
};



/* some variables used by SectorIsBad() */
UINT8 DeviceChecked;
UINT16 SectorChecked;
BOOL PreviousCheck;

// this should be a local, not global variable
//UINT8 TempArray[100];
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
BOOL WaitForProgramming( UINT16 numBytes_, BOOL verify_);

/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/


void FlashWriteEnable()
{
   UINT8 flashCommand;

   #define FLASH_WRITE_ENABLE_COMMAND		0x06		// Enable writes

   spiInit(SPI_FLASH, 1000000);
   
   flashCommand = FLASH_WRITE_ENABLE_COMMAND;
  
   spiSendByteD( &flashCommand, &flashCommand, 1, SPI_FLASH);
   
}

/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: FlashMemInit
*|  Description:
*|----------------------------------------------------------------------------
*/
BANK0_FUNC BOOL FlashMemInit
(
   void
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
   DeviceChecked=0xFF;
   SectorChecked= 0xFFFF;
   PreviousCheck=false;

   return true;

}/* FlashMemInit() */



/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: EraseFlash
*|  Description:
*!    currently hardcode to erase entire 32/64 Mbit chip
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL EraseFlash
(
   UINT32 startAddress_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 tempArray[10];
   
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
   /* from the supplied address, determine the device and sector and */
   /* verify sector is not marked as 'bad' */
#ifdef MEM64_MBIT
   #define NUM_BYTES_PER_BLOCK ((UINT16)8448)
#else
   #define NUM_BYTES_PER_BLOCK ((UINT16)4224)
#endif

   if( !MapAddress(startAddress_) )
   {
      /* no memory available */
      return false;
   }

   spiInit(SPI_FLASH, 1000000);

   tempArray[0] =0xd8; //COMMAND_BLOCK_ERASE;
   tempArray[1] =SectorNumHi;
   tempArray[2] =SectorNumLo;
   tempArray[3] =0x00;

   FlashWriteEnable();
   
   AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_SYS);
   spiSendByteD((UINT8 *)tempArray, tempArray, 4, SPI_FLASH);

   while( (StatusRegRead() &0x01) ==0x01 )
   {
      /* hold up */
      KickWdt();
      
//break;

   }

   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

   return true;

}/* EraseFlash() */




/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: ReadFromSector
*|  Description:
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL ReadFromSector
(
   UINT32 address_,
   UINT8 *destPtr_,
   UINT16 numBytes_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 tempArray[1100];
   INT16 bytesToRead;

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
   bytesToRead =numBytes_;

while( bytesToRead >0 )
{
   /*! always ignore tag bytes during reads */
   /* from the supplied address, determine the device and sector and */
   /* verify sector is not marked as 'bad' */
   if( !MapAddress(address_) )
   {
      /* no memory available */
      return false;
   }

   spiInit(SPI_FLASH, 1000000);
   
   tempArray[0] =0x03; //COMMAND_MEMORY_PAGE_READ;
   tempArray[1] =SectorNumHi;
   tempArray[2] =SectorNumLo | ByteAddressHi;
   tempArray[3] =ByteAddressLo;


   AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_SYS);
   
   if( bytesToRead >=256 )   
      spiSendByteD((UINT8 *)tempArray, tempArray, (4+256), SPI_FLASH);
   else
      spiSendByteD((UINT8 *)tempArray, tempArray, (4+bytesToRead), SPI_FLASH);     

   if( bytesToRead >=256 )      
      memcpy( destPtr_, &tempArray[4], 256 );     
   else
      memcpy( destPtr_, &tempArray[4], bytesToRead );

#ifdef jeff   
   while( (StatusRegRead() &0x80) !=0x80 )
   {
      /* hold up */
      KickWdt();
   }
#endif
   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);
   
   bytesToRead -=256;
   address_ +=256;
   
//   if( bytesToRead >=256 )   
      destPtr_ +=256;
//   else
//      destPtr_ +=bytesToRead;     

}
   return true;

}/* ReadFromSector() */


/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: WriteToSector
*|  Description:
*|   Writes one or more bytes to the flash array from a specified
*|   source. The transfers must be within the SAME sector.
*|
*|   Any sector and any byte of a sector can be written to but this behavior
*|   can be easily changed as follows:
*|
*|   By setting IgnoreTagBytes to TRUE, sectors tagged as 'bad' and the tag
*|   bytes can be written.  This allows PC GUI to refresh the tag bytes as per
*|   the DeviceInformationSector.
*|
*|   By setting IgnoreTagBytes to false, sectors tagged as 'bad' and the tag
*|   bytes will be skipped and the actual address used is stored in the global
*|   LinearAddress.  This allows "Pushing" of log data into log memory.
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL WriteToSector
(
   UINT32 address_,   /*| linear address used to determine device & sector for writing to */
   UINT8 *sourcePtr_, /*| the locations to get sequential 'write' data from */
   UINT16 numBytes_   /*| the number of bytes to read */
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 tempArray[1100];
   INT16 bytesToWrite;
   
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
   bytesToWrite =numBytes_;      
   
while( bytesToWrite>0 )
{
   /* from the supplied address, determine the device and sector and */
   /* verify sector is not marked as 'bad' */
   if( !MapAddress(address_))
   {
      /* no memory available */
      return false;
   }

   spiInit(SPI_FLASH, 1000000);

   FlashWriteEnable();

   tempArray[0] =0x02; //COMMAND_WRITE_MEMORY_B1;
   tempArray[1] =SectorNumHi;
   tempArray[2] =SectorNumLo | ByteAddressHi;
   tempArray[3] =ByteAddressLo;

   if( bytesToWrite >=256 )   
      memcpy( &tempArray[4], sourcePtr_, 256);     
   else
      memcpy( &tempArray[4], sourcePtr_, bytesToWrite);

   AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_SYS);
   
   if( bytesToWrite >=256 )
      spiSendByteD((UINT8 *)(tempArray), tempArray, (4+256), SPI_FLASH);
   else
      spiSendByteD((UINT8 *)(tempArray), tempArray, (4+bytesToWrite), SPI_FLASH);     

   /*! wait forever until flash is programmed */
   WAIT_FOR_PROGRAMMING(numBytes_, true);
   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);
   
   bytesToWrite -=256;
   address_ +=256;
   if( bytesToWrite >=256 )   
      sourcePtr_ +=256;     
   else
      sourcePtr_ +=bytesToWrite;
}

   return true;

}/* WriteToSector() */




/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: SectorSram
*|  Description:
*|   Transfers one sector between internal flash array and internal
*|   SRAM buffer. This process uses the SRAM inside the flash chips and not
*|   the PIC's limited registers.
*|----------------------------------------------------------------------------
*/
BOOL SectorSram
(
   UINT32 address_,
   BOOL toSram_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 tempArray[10];

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


   if( !MapAddress(address_) )
   {
      /* no memory available */
      return false;
   }

   /* set up commands sequence for this operation */
   tempArray[0] =COMMAND_SRAM1_TO_SECTOR_WO_ERASE;
   tempArray[1] =SectorNumHi;
   tempArray[2] =SectorNumLo;
   tempArray[3] =0x00; /* don't care */

   if( !toSram_ )
   {
      spiSendByteD( (UINT8 *)tempArray, tempArray, 4, SPI_FLASH);
   }
   else
   {
      /* issue the "sectorToSram" sequence to device */
      tempArray[0] =COMMAND_SECTOR_TO_SRAM1;

      spiSendByteD(tempArray, tempArray, 4, SPI_FLASH);
   }

   while( (StatusRegRead() &0x80) !=0x80 )
   {
      /* hold-up */
   }

   KickWdt();

   return true;

}/* SectorSram() */

/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: McuSram
*|  Description:
*|   Transfers data between the internal SRAM buffer of flash chip and MCU
*|----------------------------------------------------------------------------
*/
BOOL McuSram
(
   BOOL write_,
   UINT32 address_,
   UINT16 numBytes_,
   UINT8 *aPtr_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 tempArray[1100];

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
   /* from the supplied address, determine the device and sector and */
   /* verify sector is not marked as 'bad' */
   if( !MapAddress(address_) )
   {
      /* no memory available */
      return false;
   }

   if( !write_ )
   {
      /* issue the "readSram" sequence to device */
      tempArray[0] =COMMAND_SRAM1_TO_MICRO;
      tempArray[1] =0x00; /* don't care */
      tempArray[2] =(ByteAddressHi & 0x07);
      tempArray[3] =ByteAddressLo;

      tempArray[4] =0x00; /* don't care */

      spiSendByteD( (UINT8 *)tempArray, tempArray, (5+numBytes_), SPI_FLASH);

      memcpy( aPtr_, &tempArray[5], numBytes_);
   }
   else
   {
      KickWdt();

      tempArray[0] =COMMAND_MICRO_TO_SRAM1;
      tempArray[1] =0x00; /* don't care */
      tempArray[2] =(ByteAddressHi & 0x07);
      tempArray[3] =ByteAddressLo;

      // copy data to send into TempArray
      memcpy( &tempArray[4], aPtr_, numBytes_);

      /* start SRAM load */
      spiSendByteD( (UINT8 *)tempArray, tempArray, (4+numBytes_), SPI_FLASH);

   }

   return true;
}/* McuSram() */

/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: StatusRegRead
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
UINT8 StatusRegRead
(
   void
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 tempArray[10];

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
   tempArray[0] =0x05; // COMMAND_STATUS_REG_READ;
   tempArray[1] =0x00;
   
   spiSendByteD( &tempArray[0], tempArray, 2, SPI_FLASH);

   return (tempArray[1]);

} /* end StatusRegRead() */

/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: flashWrites
*|  Description:
*|   Enables or disables the flash device for 'writes'. Valid only for the
*|   NexFlash memory devices, not the Atmels.
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL FlashWrites
(
   UINT8 deviceNum_,
   BOOL enable_
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

   return true;

}/* FlashWrites() */







/*
*|----------------------------------------------------------------------------
*|  Module: memoryFd
*|  Routine: MapAddress
*|  Description:
*|    Converts a physical address of log memory into a device, sector, and byte
*|    address in the sector.  This routine accounts for the 'bad' sectors that
*|    exist in the device as determined by chip mfg.
*|----------------------------------------------------------------------------
*/
BOOL MapAddress
(
   UINT32  address_
)
{
   /* determine which device address_ is in */
//   DeviceNum =ADDRESS_TO_DEVICE( address_ );
   DeviceNum = (address_ >> 22);   
   /* 32 MBit atmel mem chip */
   SectorNum =(ADDRESS_TO_SECTOR( address_ ) <<2);

   /* determine which byte in the sector */
   ByteAddress = ADDRESS_TO_BYTE( address_ );

   return true;


}/* MapAddress() */


/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: WaitForProgramming
*|  Description:
*|   Waits for chip to signal that its completed the burining of its
*|   internal flash array. You can optionally specify if a verification should
*|   also be done or not.
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL WaitForProgramming
(
   UINT16 numBytes_,
   BOOL verify_
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

   while( (StatusRegRead() &0x01) ==0x01 )
   {
#ifndef CLEANUP1
      KickWdt();
#endif
          
//break;      
   }


   return true;
}/* WaitForProgramming() */



/* end memoryHi.c */









