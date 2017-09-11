/*
*|**************************************************************************
*|PROJECT:  Weatherford Mini Gauge recorder
*|Module:   Memory HI header file
*|Description:
*|   Header file for Memory module.  It deals with the  HI layer of
*|   sw needed for accessing flash memory ( ie Atmel 45DB642 and 45DB322
*|   devices).
*|   The atmel devices uses the SPI.
*|   The devices have an internal SRAM buffer enough for one complete sector
*|
*|   Memory devices are always powered up.
*|   The routines assume that power is already on and the SPI is already
*|   configured.
*|
*|   The memories are organized into sectors, no bad sectors exist. The
*|   design is based on the nexFlash devices that had bad sectors
*|   and 'tag' bytes to mark bad sectors. Thus 'tag' byte locations are
*|   not used but remains the atmel design.
*|
*|   All accesses to the flash memories use real addresses presented to
*|   the hw chips themselves.  These addresses can access any sector in
*|   the flash chips
*|
*|   Compile-time flags allow the selection of type and quantity of the
*|   flash chips.
*|
*|   All routines require the SPI be configured and that power be already supplied
*|   to flash prior to invocation of the routine
*|
*|
*|
*|
*|   The following data transfer paths are available:
*|
*|   MCU       FLASH_SRAM     FLASH_ARRAY      RoutineName
*|    |-------------------------->|            WriteToSector(same sector)
*|    |<--------------------------|            ReadFromSector(same sector)
*|    |            |<------------>|            SectorSram
*|    |<---------->|                           McuSram
*|**************************************************************************
*/


/*
   ***************************************************************************
   Copyright (C) 2002, Micrologic Limited.
   Any information contained in this document which was hitherto unknown by
   the recipient or in which there subsists a proprietary right must not be
   disclosed to any other person, nor used for manufacturing purposes,
   without the permission of Micrologic Limited and the acceptance of this
   document will be construed as acceptance of the foregoing.
   ***************************************************************************
*/
#ifndef MEMORY_HI_H
 #define MEMORY_HI_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "mk_Std.h"


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

#define MEM64_MBIT


/* identify the type of Atmel memory chip size, either define
   MEM64_MBIT or MEM32_MBIT not both. This is done via the
   compile-time flags availabe with the IAR compiler, see
   toolFd.h */
#define BYTES_PER_SECTOR   ((UINT16)1024)

#define BLOCKS_PER_DEVICE  ((UINT16)1024)
#define SECTORS_PER_BLOCK  ((UINT16)8)
#define SECTORS_PER_DEVICE ((UINT16)8192)

#ifdef DUAL_FLASH
  #define NUM_FLASH_DEVICES  ((UINT8)2)
#else
  #define NUM_FLASH_DEVICES  ((UINT8)1)
#endif


#define BYTES_PER_DEVICE   ((UINT32)BYTES_PER_SECTOR * (UINT32)SECTORS_PER_DEVICE)
#define BYTES_PER_BOARD    ((UINT32)NUM_FLASH_DEVICES * (UINT32)BYTES_PER_SECTOR * (UINT32)SECTORS_PER_DEVICE)
#define BYTES_PER_BLOCK    ((UINT16) (BYTES_PER_SECTOR * SECTORS_PER_BLOCK))

#define FLASH_NOT_BUSY     ((UINT8)0x99)
#define FLASH_VERIFIED     ((UINT8)0xFF)


#define BANK0      /**/
#define BANK0_FUNC /**/

#define BANK1      /**/
#define BANK1_FUNC /**/

#define BANK2      /**/
#define BANK2_FUNC /**/


#define NUM_TAG_BYTES ((UINT8)1)



/* value for the tag/sync byte that indicates state of a sector */
#define SECTOR_IS_OK ((UINT8)0xC9)

/* convert a real address to device number, sector number, or byte in a sector */
#define ADDRESS_TO_DEVICE(addr_) ( ( addr_ /BYTES_PER_DEVICE ) )
#define ADDRESS_TO_SECTOR(addr_) ( ( addr_ / BYTES_PER_SECTOR)%SECTORS_PER_DEVICE)
#define ADDRESS_TO_BYTE(addr_)   ( ( addr_ % BYTES_PER_DEVICE)%BYTES_PER_SECTOR)

#define COMMAND_WRITE_MEMORY_B1 ((UINT8)0x82)
#define COMMAND_WRITE_MEMORY_B2 ((UINT8)0x85)

#define COMMAND_MICRO_TO_SRAM1 ((UINT8)0x84)
#define COMMAND_MICRO_TO_SRAM2 ((UINT8)0x87)

#define COMMAND_SRAM1_TO_SECTOR_W_ERASE  ((UINT8)0x83) /* with erase */
#define COMMAND_SRAM2_TO_SECTOR_W_ERASE  ((UINT8)0x86)

#define COMMAND_SRAM1_TO_SECTOR_W_FERASE  ((UINT8)0x93) /* with erase */
#define COMMAND_SRAM2_TO_SECTOR_W_FERASE  ((UINT8)0x96)

#define COMMAND_SRAM1_TO_SECTOR_WO_ERASE ((UINT8)0x88) /* without erase */
#define COMMAND_SRAM2_TO_SECTOR_WO_ERASE ((UINT8)0x89)

#define COMMAND_MEMORY_PAGE_READ ((UINT8)0xd2)
#define COMMAND_STATUS_REG_READ  ((UINT8)0xD7)

#define COMMAND_SECTOR_TO_SRAM1  ((UINT8)0x53)
#define COMMAND_SECTOR_TO_SRAM2  ((UINT8)0x55)

#define COMMAND_SECTOR_SRAM1_COMPARE  ((UINT8)0x60)
#define COMMAND_SECTOR_SRAM2_COMPARE  ((UINT8)0x61)

#define COMMAND_PAGE_ERASE  ((UINT8)0x81)
#define COMMAND_BLOCK_ERASE ((UINT8)0x50)

#define COMMAND_SRAM1_TO_MICRO ((UINT8)0xd4)
#define COMMAND_SRAM2_TO_MICRO ((UINT8)0xd6)


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
/* LinearAddress is general purpose variable used by various routines */
extern UINT32 LinearAddress;

/* Populated by MapAddress(), a real address must be broken down into the
   device, sector, and byte number so hw can be activated */

extern BANK2 UINT8 DeviceNum;

extern BANK2 union
{
   UINT16 SectorNum;
   struct
   {
      UINT8 SectorNumLo;
      UINT8 SectorNumHi;
   };
};
extern BANK2 union
{
   UINT16 ByteAddress;
   struct
   {
      UINT8 ByteAddressLo;
      UINT8 ByteAddressHi;
   };
};

/* used by SectorIsBad to remember last address checked so it does not
   have to be checked again...increases performance */
extern BANK2 UINT8 DeviceChecked;
extern BANK2 UINT16 SectorChecked;
extern BANK2 BOOL PreviousCheck;



/* enable or disable the use of the tag byte:
  TRUE: treats tag bytes just as any other byte in flash.  Bad sectors will
        thus be used if accessed.
  FALSE:the tag bytes are never overwritten and it is interpreted as an
        indicator of whether the tagged sector is used or skipped */
extern BANK2 BOOL IgnoreTagByte;

/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: FlashMemInit
*|  Description:
*|   Initialises memory.
*|----------------------------------------------------------------------------
*/
BANK0_FUNC BOOL FlashMemInit
(
   void
);




/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: EraseFlash
*|  Description:
*|   Currently erases all of Log memory only (not NV data)
*|
*|   Memory chip power is not affected but power must already be applied.
*|
*|   The devices uses an SPI interface so the SPI must already be
*|   configured prior to calling this routine.
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL EraseFlash
(
   UINT32 startAaddress_
);





/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: ReadFromSector
*|  Description:
*|   Reads bytes from ONE sector of the flash array to a specified
*|   destination; if destination is NULL_PTR, the read data is not stored
*|   but the sector is still read into SRAM buffer of the flash chip.
*|
*|   Any sector and any byte of a sector is accessable without restriction.
*|
*|   Power must be controlled outside of this routine.
*|
*|   The devices uses an SPI interface so the SPI must already be
*|   configured prior to calling this routine.
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL ReadFromSector
(
   UINT32 source_,   /*| the real address of memory to start reading from */
   UINT8 *destPtr_,  /*| location to store the extracted data */
   UINT16 numBytes_  /*| the number of bytes to read */
);

/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: WriteToSector
*|  Description:
*|   Writes one or more bytes to the flash array from a specified
*|   source. The transfers must be within the SAME sector.
*|
*|   This routine may write to ANY sector and ANY byte of a sector.
*|
*|   Power is not controlled by this routine but assumed ON.
*|
*|   This routine blocks while waiting for the flash chip to burn.
*|
*|   It indirectly uses the internal SRAM buffer of the memories.
*|
*|   The devices uses an SPI interface so the SPI must already be
*|   configured prior to calling this routine.
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL WriteToSector
(
   UINT32 realAddress,  /*| real address used to determine device & sector for writing to */
   UINT8 *sourcePtr_,   /*| the locations to get sequential 'write' data from */
   UINT16 numBytes_    /*| the number of bytes to read */
// always verified  BOOL verify_        /*| TRUE to request the write be verified */
);

/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: SectorSram
*|  Description:
*|   Transfers one sector between internal flash array and internal
*|   SRAM buffer. This process uses the SRAM inside the flash chips and not
*|   the PIC's limited registers. The flash memory is always given a reset
*|   before accesses. During SRAM-to-SECTOR burns, three attemps are made
*|   using the WriteWithOutErase mode ("overBurn").
*|----------------------------------------------------------------------------
*/
BOOL SectorSram
(
   UINT32 address_,
   BOOL toSram_
);


/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: StatusRegRead
*|  Description:
*|    Reads the status register of the memory chips to check if internal
*|    flash array operations are completed or not.
*|----------------------------------------------------------------------------
*/
UINT8 StatusRegRead   /*| returns the value of the status register */
(
   void
);



/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: FlashWrites
*|  Description:
*|   Needed only for the NexFlash chips, not this Atmel design
*|----------------------------------------------------------------------------
*/
BANK1_FUNC BOOL FlashWrites
(
   UINT8 deviceNum,  /*| flash device */
   BOOL enable_      /*| TRUE to enable writes, FALSE to disable writes     */
);



/*
*|----------------------------------------------------------------------------
*|  Module: MemoryHi
*|  Routine: McuSram
*|  Description:
*|
*|   Transfers data between the internal SRAM buffer of flash chip and PIC micro.
*|
*|   Tag bytes and bad sectors are irrelevant (IgnoreTagBytes == TRUE )
*|
*|   Power to flash device is not affected but must already be ON.
*|
*|   The  devices uses an SPI interface so the SPI must already be
*|   configured prior to calling this routine.
*|
*|   The flash memory is not reset in this sequence but the chip select line
*|   is toggled as needed.
*|----------------------------------------------------------------------------
*/
BOOL McuSram
(
   BOOL write_,      /*| FALSE to read SRAM, TRUE to write to SRAM    */
   UINT32 address_,  /*| used to identify device and byte address     */
   UINT16 numBytes_, /*| number of bytes to read or write             */
   UINT8 *aPtr_      /*| destination for 'reads', source for 'writes' */
);





/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: MapAddress
*|  Description:
*|  Converts the supplied address to a new address that skips over sectors
*|   or not, depending on IgnoreTagBytes. The new address is then split up into
*|   the actual physical address selected, the physical device, sector and byte number.
*|
*|   When tag byte handling is ignored, no skipping of sectors will occur.  Also, the
*|   tag bytes can also be accessed for read or write.  If tag byte handlng is not
*|   ignored, the actual physical address selected is written into LogMemWriteAddress.
*|   The value written accounts for skipped sector(s) and skipping of the tag byte.
*|
*|   Power to the flash chip must already be ON.
*|
*|   The devices uses an SPI interface so the SPI must already be
*|   configured prior to calling this routine.
*|
*|   The class variables LogMemWriteAddress, DeviceNum, SectorNum, and ByteAddress
*|   are used for passing data into and out of this routine.
*|----------------------------------------------------------------------------
*/
BOOL MapAddress  /*| if FALSE, there is no memory available */
(                             /*|                                             */
   UINT32  realAddress_       /*| the real address to be mapped               */
);


#endif






