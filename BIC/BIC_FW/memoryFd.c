/*
*|**************************************************************************
*|PROJECT:  Compact Compression CCU
*|Module:   Memory
*|Description:
*|   The routine associated accesing atmel flash memory (8Mbits, SPI,
*|   internal RAM buffer).
*|**************************************************************************
*/

/*
Tools:
   Compiler: IAR Kickstart development environment for AT91SAM7Sxxx
   ***************************************************************************
   Copyright (C) 2002, Micrologic Limited.
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
#include "memoryFd.h"
#include "memoryHi.h"
#include "timer.h"

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
#define DEBUG_MEMORY_FD_ON   0

#if DEBUG_MEMORY_FD_ON ==1
 #define DEBUG_MEMORY_FD(message)   DEBUG(message, THIS_MODULE'S_TASK_ID)
#else
 #define DEBUG_MEMORY_FD(message) /* */
#endif


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
/* keep track of Log memory 'write' location */
UINT32 LogMemWriteAddress;

/* indicates if the Log mem's SRAM buffer has data or not */
BOOL LogBufferHasData;


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
*|  Module: MemoryHi
*|  Routine: InitMemory
*|  Description:
*|   Initialise flash memory driver
*|----------------------------------------------------------------------------
*/
BOOL InitMemory
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


#ifdef jeff
   MEMORY0_CS_DIRECTION = 0;/* make pin an output */
   MEMORY1_CS_DIRECTION = 0;/* make pin an output */
   MEMORY2_CS_DIRECTION = 0;/* make pin an output */
#endif

//   MEMORY1_PULL_UP_DIRECTION =0;

   /* set initial state of memory chip pins to HI to maintain power savings */
   /* until we need to access memory */

   FlashMemInit();

   return true;
}/* InitMemory() */




/*
*|----------------------------------------------------------------------------
*|  Module: MemoryFd
*|  Routine: StartMemory
*|  Description:
*|   Initialises Log memory
*|----------------------------------------------------------------------------
*/
BOOL StartMemory
(
   BOOL erase_
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

   if( erase_ )
   {
      spiInit(SPI_FLASH, 0);

      /* invoke erasure */
      EraseFlash(0x0000);

      /* init the 'write' pointer, add offset of one byte for 'tag' byte
         that was needed for NexFlash and carried into the Atmel design
         even though atmel mems don't have tag bytes */
      LogMemWriteAddress = START_OF_LOG_MEMORY+1;
   }
   else
   {
      /* must find the where LogMemWriteAddress is located by searching
         all of log memory for the end byte patterns */
      if( !FindLogMemoryEnd( START_OF_LOG_MEMORY, &LogMemWriteAddress) )
      {
         /* error finding log mem end, means corruption so a manual InitMem needed */
         return false;
      }
   }

   /* indicate that SRAM buffer has no queued data right now; note this feature
      was and is disabled because initially the mems were always powered down
      between ticks so could not be used to queue data */
   LogBufferHasData =false;

   /* pull out the sector into SRAM buffer */
   SectorSram(LogMemWriteAddress, true);

   return true;
}/* StartMemory() */


/*
*|----------------------------------------------------------------------------
*|  Module: MemoryFd
*|  Routine: PushLogData
*|  Description:
*|   Appends data to next free address in Log memory and updates the marker used
*|   to mark the end of used Log memory.
*|
*|   It uses the internal SRAM buffer of the memories as scratchpad memory.
*|----------------------------------------------------------------------------
*/
BOOL PushLogData
(
   UINT32 address_,
   UINT8 *sourcePtr_,
   UINT32 numBytes_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT16 bytesWritten;
   volatile UINT8 i,j,k;


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
   if( address_ != 0xFFFFFFFF)
   {
      LogMemWriteAddress = address_;
   }

   if( (LogMemWriteAddress+numBytes_) >= END_OF_LOG_MEMORY )
   {
      /* prevents writes beyond the allowed end of log memory */
      return false;
   }

   /* configure the SPI for flash memory accesses */
   spiInit(SPI_FLASH, 0);


   /* convert linear address to the device, sector, and byte numbers */
   if( !MapAddress(LogMemWriteAddress) )
   {
      /* no memory available */
      return false;
   }

   /* while sector spanning is needed */
   while( numBytes_ >= (BYTES_PER_SECTOR - ByteAddress) )
   {
      /* number of new bytes to write will fill or exceeds the amount of bytes left */
      /* in SRAM buffer so copy enough bytes to fill up rest of SRAM   */
      /* and burn it to flash */
      bytesWritten = (BYTES_PER_SECTOR - ByteAddress );

      /* reload ram buffer with flash data if flash is being turned off*/
#ifndef yee
/* replace SectorSram for calscan timing */
      SectorSram(LogMemWriteAddress, true);
#else
      TempArray[0] =COMMAND_SECTOR_TO_SRAM1;
      TempArray[1] =(SectorNum &0xff00) >>8;
      TempArray[2] =(SectorNum &0x00ff);
      TempArray[3] =0x00; /* don't care */

      /* reset the flash chip for read or writes to flash array */
      RESET_MEMORY;

      spiSendByteD(TempArray, NULL_PTR, 4 );


      while( (StatusRegRead() &0x80) !=0x80 )
      {
         /* hold-up */
        asm("CLRWDT");
      }
#endif

      /* poke  new data into SRAM */
#ifndef yee
/* replace McuSram for calscan timing */
      McuSram( true, LogMemWriteAddress, bytesWritten, sourcePtr_);
#else
      /* assert the chip select of selected device */

      TempArray[0] =COMMAND_MICRO_TO_SRAM1;
      TempArray[1] =0x00; /* don't care */
      TempArray[2] =(ByteAddressHi & 0x07);
      TempArray[3] =ByteAddressLo;


      /* start SRAM load */
      spiSendByteD( (UINT8 *)TempArray, NULL_PTR, 4 );

      /* load data byte */
      //aPtr = errorCode;
      aPtr = sourcePtr_;
      for(j=0; j<bytesWritten; j++)
      {
         for(k=0;k<50;k++)
         {
            asm("CLRWDT");
         }
         spiSendByteD( aPtr , NULL_PTR, 1 );
         aPtr++;
      }



#endif

      /* burn flash sector with contents of SRAM */

#ifndef yee
/* replace SectorSram for calscan timing */
      FlashWrites(LogMemWriteAddress, true);
      SectorSram(LogMemWriteAddress, false);
#else
      /* burn same locations in memory 3 times to overburn */
      for(i=0; i<3; i++)
      {
         /* the replacement for SectorSram() to burn flash with SRAM  */
         TempArray[0] =COMMAND_SRAM1_TO_SECTOR_WO_ERASE;
         TempArray[1] =(SectorNum &0xff00) >>8;
         TempArray[2] =(SectorNum &0x00ff);
         TempArray[3] =0x00; /* don't care */

         RESET_MEMORY;


         spiSendByteD( (UINT8 *)TempArray, NULL_PTR, 4 );

         while( (StatusRegRead() &0x80) !=0x80 )
         {
            /* hold-up */
            asm("CLRWDT");
         }
      }

#endif

      /* update number of bytes written by that many bytes */
      numBytes_ = numBytes_ - bytesWritten;

      /* increment linear address by that many bytes */
      LogMemWriteAddress += bytesWritten;

      /* update position to get data from */
      sourcePtr_ += bytesWritten;

      /* re-map the linear address */
      if( !MapAddress(LogMemWriteAddress) )
      {
         /* no memory available */
         /*! must clobber last bytes of data with the end marker */
         return false;
      }
   }

   /* now there are only enough data to partially fill SRAM so do that but
      don't burn flash since SRAM is not full */
   if( numBytes_ != 0 )
   {
      if(!LogBufferHasData )
      {
         /* SRAM buffer currently has no valid data, init it to filler pattern */
#ifndef yee
/* replace SectorSram() to get CalScan timing */
         SectorSram(LogMemWriteAddress, true);
#else
         TempArray[0] =COMMAND_SECTOR_TO_SRAM1;
         TempArray[1] =(SectorNum &0xff00) >>8;
         TempArray[2] =(SectorNum &0x00ff);
         TempArray[3] =0x00; /* don't care */

         /* reset the flash chip for read or writes to flash array */
         RESET_MEMORY;

         spiSendByteD(TempArray, NULL_PTR, 4 );


         while( (StatusRegRead() &0x80) !=0x80 )
         {
            /* hold-up */
           asm("CLRWDT");
         }
#endif
      }/* if log buffer is empty */

      /* transfer remaining data from PIC to SRAM */
#ifndef yee
/* replace McuSram()and SectorSram() to get Calscan timing */
      McuSram( true, LogMemWriteAddress, numBytes_, sourcePtr_);

      /* always burn even partially filled buffer */
      SectorSram(LogMemWriteAddress, false);

#else
      /* write SRAM then burn back to flash 3 times overburn, each
         with prior reset but without prior erase...  */

      TempArray[0] =COMMAND_MICRO_TO_SRAM1;
      TempArray[1] =0x00; /* don't care */
      TempArray[2] =(ByteAddressHi & 0x07);
      TempArray[3] =ByteAddressLo;


      /* start SRAM load */
      spiSendByteD( (UINT8 *)TempArray, NULL_PTR, 4 );

      /* load data byte */
      //aPtr = errorCode;
      aPtr = sourcePtr_;
      for(j=0; j<numBytes_;j++)
      {
         for(k=0;k<50;k++)
         {
            asm("CLRWDT");
         }
         spiSendByteD( aPtr , NULL_PTR, 1 );
         aPtr++;
      }


      /* burn same locations in memory 3 times to overburn */
      for(i=0; i<3; i++)
      {
         /* the replacement for SectorSram() to burn flash with SRAM  */
         TempArray[0] =COMMAND_SRAM1_TO_SECTOR_WO_ERASE;
         TempArray[1] =(SectorNum &0xff00) >>8;
         TempArray[2] =(SectorNum &0x00ff);
         TempArray[3] =0x00; /* don't care */

         RESET_MEMORY;


         spiSendByteD( (UINT8 *)TempArray, NULL_PTR, 4 );


         while( (StatusRegRead() &0x80) !=0x80 )
         {
            /* hold-up */
            asm("CLRWDT");
         }
      }

#endif /* end of read-modify-write flash */



      LogMemWriteAddress += numBytes_;

      /* set flag to indicate SRAM has valid data that is to be burned */
      /* into flash array when buffer becomes full */
      LogBufferHasData = false;
   }

   return true;

}/* PushLogData() */


#ifdef SOME_DEBUG_STUFF
/* configure the SPI for flash memory access */
spiInit(SPI_FLASH, 1000000);

aAddress =8448;

//read
memset( aTemp, 0x00, sizeof(aTemp));
SectorSram(aAddress, 1);
McuSram( 0, aAddress, 4, aTemp );

// write
EraseFlash(aAddress, (aAddress+1056));
memset( aTemp, 0x55, sizeof(aTemp));
McuSram( 1, aAddress, 10, aTemp );
SectorSram(aAddress, 0);

//read
memset( aTemp, 0x00, sizeof(aTemp));
SectorSram(aAddress, 1);
McuSram( 0, aAddress, 10, aTemp );

memset( aTemp, 0x00, sizeof(aTemp));
ReadFromSector(aAddress, aTemp, 10);

memset( aTemp, 0xaa, sizeof(aTemp));
WriteToSector(aAddress, aTemp, 10);

memset( aTemp, 0x00, sizeof(aTemp));
ReadFromSector(aAddress, aTemp, 10);
#endif


/* End memoryFd.c  */


