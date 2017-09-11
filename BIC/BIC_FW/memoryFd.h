/*
*|**************************************************************************
*|PROJECT:  Weatherford Mini Gauge recorder
*|Module:   Memory
*|Description:
*|   Header file for Memory module.  It deals with the FD layer of
*|   sw needed for accessing flash memory.  The flash memory must be erased
*|   prior to being used.
*|
*|   Memory devices are normally powered up even when not being used.
*|   This module makes use of the following types of addresses:
*|   - "NV" address space: Part of the map is reserved for use
*|     as random storage area for coefficients, text, etc.
*|   - "LOG" address space: Most of the map is reserved for use
*|     as sequentially accessed storage of the RunDataTable.
*|   - No 'bad' sectors or 'tag' bytes are needed for the Atmel devices but
*|     because this design was based on the nexFlash devices, 'tag' byte locations
*|     are always skipped over in the Atmel design.
*|   - Note erasure and pushing data into log mem does not affect the 'nv' memory.
*|
*|   NV memory:
*|   This memory is used to store config data and other data that does not
*|    change often.  Do not access this memory while doing logging, only
*|    in the command mode and after log memory is initialised using StartMemory,
*|    but before log memory is used by PushLogData.  You can pull out
*|    the NV data contents prior to erasure and then restore it after erasure.
*|   Note that this memory is organized by sectors (a sector is the smallest
*|    size that can be allocated).  The 'TEXT' sector is used for storing
*|    ascii strings that are transmitted out when needed.  The 'COEFFICIENTS'
*|    sectors stores the transducer coefficients.  Note the actual sectors used
*|    are fixed and determined by the WMG GUI.
*|   The data in NV memory is not always used by the WMG firmware but may be
*|   just storage used by the WMG GUI.
*|
*|   LOG MEMORY:
*|   This memory is used to log data.  Once the data is logged, that data
*|   never changes until the log is to be initialised at which time ALL data
*|   in log memory is erased.  New data is always appended to end of log memory.
*|   The log data may contain runDataHeader info, data (pressure and temperature)
*|   records, and error records.  Error records are structured to allow up to
*|   the first 6 most signif bytes to be lost and still be detectable.  The error
*|   codes consist of 12 bytes, with only the LSbyte containing a real error
*|   identifier.  All other bytes contain only the error preamble byte 0xEE. The
*|   GUI must locate an error code by detecting the LAST 5 preamble bytes. Then
*|   the next byte is the error identifier and the next byte after that is the
*|   start location of either a runDataHeader or more data records.
*|
*|   During mem init, all of log memory is erased to the filler byte pattern.  This
*|   is used to detect the end of used log memory.
*|
*|
*|
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
#ifndef MEMORY_FD_H
 #define MEMORY_FD_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "mk_Std.h"
//#include "tgStd.h"
#include "memoryHi.h"
#include "memoryFd.h"


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

/* the number of characters of the 'filler' bytes used to identify unused memory */
#define NUM_CHAR_LOG_MEM_FILLER ((UINT8)4)

/* the pattern stored into flash to mark unused locations */
#define LOG_MEM_FILLER ((UINT8)0xFF)

/* the first and last addresses of log memory, the bytes beyond the end of
   log memory is used for storage of NV data by the PC GUI */
#define START_OF_LOG_MEMORY ((UINT32)0x000000)

#ifdef MEM64_MBIT

  /* 64MBit devices, each has 1024 sectors but leave a few sectors of 'buffer'
     zone between log mem end and start of NV mem.  Each sector is 8448 bytes. */
  #ifdef DUAL_FLASH
    /* for two 64Mbit devices */
    #define END_OF_LOG_MEMORY   ((UINT32)2046*8448)
  #else
    /* for one 64Mbit device */
    #define END_OF_LOG_MEMORY   ((UINT32)1022*8448)
  #endif

   #define START_OF_LOG_MEM_OFFSET ((UINT32)0x00000000);
#endif

#ifdef MEM32_MBIT
  /* 32MBit devices, each has 1024 sectors but leave a few sectors of 'buffer'
     zone between log mem end and start of NV mem.  Each sector is 4224 bytes. */
  #ifdef DUAL_FLASH
    #define END_OF_LOG_MEMORY   ((UINT32)2046*4224)
  #else
    #define END_OF_LOG_MEMORY   ((UINT32)1022 *4224)
  #endif

   #define START_OF_LOG_MEM_OFFSET ((UINT32)0x00000000);
#endif

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
/* Keep track of the address where data should be appended. Note it does not
   necessarily match with actual address where data is to be burned into flash
   if the SRAM buffer is used for buffering(its not).
*/
extern  UINT32 LogMemWriteAddress;

/* flag used to indicate if SRAM buffer has unburned data */
extern  BOOL LogBufferHasData;


/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/


/*
*|----------------------------------------------------------------------------
*|  Module: Memory
*|  Routine: InitMemory
*|  Description:
*|   The initialisation for the module
*|----------------------------------------------------------------------------
*/
BOOL InitMemory       /*| returns X else Y        */
(                                  /*|                         */
   void                            /*|*/
);                                 /*|                         */




/*
*|----------------------------------------------------------------------------
*|  Module: MemoryFd
*|  Routine: StartMemory
*|  Description:
*|   Used to do a complete re-init of memory or to try to recover the last
*|   known state of memory pointers by examining logged data.
*|   If an erasure is invoked, this erases the flash to the 0xFF state.
*|----------------------------------------------------------------------------
*/
BOOL StartMemory
(
   BOOL erase_  /*| TRUE to erase and re-initialise all memory, else recover the */
                /*| last memory state */
);



/*
*|----------------------------------------------------------------------------
*|  Module: MemoryFd
*|  Routine: FindLogMemoryEnd
*|  Description:
*|   Find the next address in Log memory that can be written to.
*|
*|   In addition, the flash chip device holding this location has it RAM buffer
*|   updated to reflect the contents of the corresponding flash array sector.
*|
*|   Power to the flash chips is assumed to be ON.
*|
*|   The  SPI interface is internally reconfigured as needed.
*|
*|   The search algorithm linearly searches forward for the filler pattern
*|   on a sector by sector basis. Then it linearly searches backwards on a
*|   byte by byte basis for a non-filler pattern. A quicker search scheme
*|   was not chosen because this design was based on the nexFlash devices
*|   that had 'bad' sectors and 'tag' bytes that would greatly complicate
*|   a binary tree search algorithm.  A binary tree search could be used
*|   with the atmel devices.
*|----------------------------------------------------------------------------
*/
BOOL FindLogMemoryEnd
(
   UINT32 startAddress_, /*| address in memory to begin the search at */
   UINT32 * resultPtr_   /*| points to location to store address found */
);






/*
*|----------------------------------------------------------------------------
*|  Module: MemoryFd
*|  Routine: PushLogData
*|  Description:
*|   Appends data to the Log memory at the location indicated by the class
*|   variable LogMemWriteAddress. Cannot access memory addresses > END_OF_LOG_MEMORY
*|
*|   If there is not enough memory to store requested data, the data is not
*|   written to log memory.
*|
*|   The transfers support chip and sector spanning.
*|
*|   The flashchips must have 'writes' already enabled else no burning actually
*|   occurs.
*|
*|   Power to the flash chips is assumed always ON.
*|
*|   The SPI is internally reconfigured for the flash as needed.
*|
*|   The SRAM buffer is not used to queue data to reduce the number of 'burn'
*|   cycles because the initial design started with memory chips always being
*|   powered down in between taking samples.  This feature can be re-enabled
*|   and retested with the latest design that always has powered mem chips.
*|----------------------------------------------------------------------------
*/
BOOL PushLogData
(
   UINT32 address_,  /*| Affects internal 'write' pointer: LogMemWriteAddress */
                     /*| 0xFFFFFFFF: use the internal 'write' pointer as is.  */
                     /*| Any other value: Sets the write pointer to that   */
                     /*|  value prior to writing data                         */
   UINT8 *sourcePtr_,/*| location to get the data to be stored                */
   UINT32 numBytes_  /*| the number of bytes to store                         */
);




/*
*|----------------------------------------------------------------------------
*|  Module: MemoryFd
*|  Routine: WriteLogData
*|  Description: OBSOLETE, use PushLogData() (with restrictions) instead.
*|   Writes one or more bytes from a specified source to the flash array.
*|   This routine supports device and sector spanning.
*|
*|   This routine is used to write data to any address of Linear address space rather
*|   than appending it (see PushLogData for appending data). Burning of flash
*|   array is not buffered so each invocation will cause one or more
*|   erase/write cycle.
*|
*|   Any valid data contained in the log memory's SRAM buffer is first burned
*|   into flash array so queued log data is not lost.  However, afterwards the
*|   SRAM will no longer contain correct data for the LOG memory unless the
*|   sector is read back.  This is not currently done due to code space limits.
*|   Thus, make sure you use this routine in the "Command" mode and then re-init
*|   Log memory before using log memory.
*|
*|   Then the sector data is read into SRAM buffer, overwritten in the needed
*|   locations, and then SRAM is burned back into flash array.
*|
*!   If there is not enough memory to store requested data, the data is written
*!   as much as possible then the rest of memory is filled with the endMarker
*!   plus N filler bytes after it.
*|
*|   Power to the flash chips are controlled externally as needed, but the SPI
*|   is internally reconfigured as needed.
*|
*|----------------------------------------------------------------------------
*/
BOOL WriteLogData
(
   UINT32 linearAddress_, /*| the linear address of memory to start writing to */
   UINT8 *sourcePtr_,     /*| location to get data to store */
   UINT32 numBytes_       /*| the number of bytes to store  */
);



/*
*|----------------------------------------------------------------------------
*|  Module: MemoryFd
*|  Routine: ReadLogData
*|  Description: OBSOLETE
*|   Reads one or more bytes from the flash to a specified destination.
*|   If the destination is NULL_PTR, the read data is not stored but the SRAM
*|   buffer of the affected flash device is populated with the corresponding
*|   sector.
*|
*|   This routine supports device and sector spanning but cannot read the
*|   sectors tagged as 'bad'.
*|
*|   This routine is used to read data from any address of log memory.
*|
*|   Power to the flash chips is not affected and must already be ON.
*|
*|   The  SPI interface is internally reconfigured as needed.
*|
*|----------------------------------------------------------------------------
*/
BOOL ReadLogData
(
   UINT32 address_,    /*| the linear address of memory to read from */
   UINT8 *destPtr_,    /*| location to store extracted data */
   UINT32 numBytes_    /*| the number of bytes to extract  */
);




#endif

/* End memoryFd.h */

