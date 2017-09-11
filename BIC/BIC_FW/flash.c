//------------------------------------------------------------------------------
// File:          flash.c
// Function:      Flash functions
// Supported chip(s):
//    - AT91SAM7XC128
//    - AT91SAM7XC256
// Supported toolchain(s):
//    - IAR Embedded Workbench
// Date created:  02 May 2006
// Created by:    JJo
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------

#include "flash.h"

#define OK          0x00000000
#define ERROR       0xFFFFFFFF

#ifdef USE_FLASH

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

/**
 * Name:      flash_init
 * Purpose:   Initializes the flash (nothing to do)
 */
void flash_init() {

//  debug_printf("Flash: Initializing ...\n");
//  debug_printf("Flash: Initialization done.\n");
}

/**
 * Name:      flash_cleanup
 * Purpose:   Cleans up the flash (nothing to do)
 */
void flash_cleanup() {

//  debug_printf("Flash: Cleaning up ...\n");
//  debug_printf("Flash: Cleanup done.\n");
}

/**
 * Name:      flash_write
 * Purpose:   Writes a page in the flash memory
 * Inputs:
 *  - Destination address in the flash
 *  - Source data buffer
 * Output: OK if write is successful, ERROR otherwise
 */
__ramfunc unsigned int flash_write(void * vAddress, void * vData) {

  // Local variables
  unsigned int page;
  unsigned int mask;
  unsigned int status;
  unsigned int * pAddress = (unsigned int *) vAddress;
  unsigned int * pData = (unsigned int *) vData;

  // Program FMCN field in Flash Mode Register
  AT91C_BASE_MC->MC_FMR = (((MCK / 666666) << 16) & AT91C_MC_FMCN) | AT91C_MC_FWS_1FWS;

  // Calculate page number and flash address
//  page = ((unsigned int) pAddress - (unsigned int) MEMORY_START_ADDRESS) & AT91C_MC_PAGEN;
   page = ((unsigned int) pAddress ) & (0x3FF80);
//   page =page<<1;

  // Copy page in write buffer
  for (unsigned int j=0; j <MEMORY_PAGE_SIZE_32; j++) {

    pAddress[j] = pData[j];
  }

  // Start flash write operation and wait for completion (IT disabled to avoid flash access)
  //while (!(AT91C_BASE_MC->MC_FSR & AT91C_MC_EOP));
  mask = AT91C_BASE_AIC->AIC_IMR;
  AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;

  AT91C_BASE_MC->MC_FCR = page | AT91C_MC_FCMD_START_PROG | AT91C_MC_CORRECT_KEY;

  do {

    status = AT91C_BASE_MC->MC_FSR;

  } while (!(status & AT91C_MC_EOP));
  AT91C_BASE_AIC->AIC_IECR = mask;

  if (status & (AT91C_MC_LOCKE | AT91C_MC_PROGE)) {

    return ERROR;
  }
  else {

    return OK;
  }
}

/**
 * Name:      flash_next
 * Purpose:   Computes next page address after (or starting at) specified address
 * Input:
 *  - Address
 * Output:    Next page address
 */
void * flash_next(void * vAddress) {

  // Local variables
  unsigned int address = (unsigned int) vAddress;

  if (address%MEMORY_PAGE_SIZE != 0) {

    address += (MEMORY_PAGE_SIZE - address%MEMORY_PAGE_SIZE);
  }

  // Check if address is remapped
  if (address < (unsigned int) MEMORY_START_ADDRESS) {

    address += (unsigned int) MEMORY_START_ADDRESS;
  }

  return (void *) address;
}

/**
 * Name:      flash_lock
 * Purpose:   Sets the lock bit for the specified memory region
 * Input:
 *  - Start address of region
 *  - End address of region
 * Output:    OK if lock is successful, ERROR otherwise
 */
__ramfunc unsigned int flash_lock(void * vStart, void * vEnd) {

  // Local variables
  unsigned int page;
  unsigned int mask;
  unsigned int status;
  void * vLast;

  // Compute page number of start address and last address in region to lock
  page = ((unsigned int) vStart - (unsigned int) MEMORY_START_ADDRESS) & AT91C_MC_PAGEN;
  vLast = (void *) ((unsigned int) MEMORY_START_ADDRESS +
          ((page & MEMORY_REGION_MASK) + MEMORY_REGION_SIZE) * MEMORY_PAGE_SIZE);

  // Program FMCN field in Flash Mode Register
  AT91C_BASE_MC->MC_FMR = (((MCK / 1000000) << 16) & AT91C_MC_FMCN) | AT91C_MC_FWS_1FWS;

  // Wait for EFC to be ready and perform command (interrupts disabled)
  while (!(AT91C_BASE_MC->MC_FSR & AT91C_MC_EOP));
  mask = AT91C_BASE_AIC->AIC_IMR;
  AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;

  AT91C_BASE_MC->MC_FCR = page | AT91C_MC_FCMD_LOCK | AT91C_MC_CORRECT_KEY;

  do {

    status = AT91C_BASE_MC->MC_FSR;

  } while (!(status & AT91C_MC_EOP));
  AT91C_BASE_AIC->AIC_IECR = mask;

  // Result
  if (status & (AT91C_MC_LOCKE | AT91C_MC_PROGE)) {

    return ERROR;
  }
  else {

    // Lock next region (if range too big)
    if (vLast < vEnd) {

      return flash_lock(vLast, vEnd);
    }
    return OK;
  }
}

/**
 * Name:      flash_unlock
 * Purpose:   Clears the lock bit for the specified memory region
 * Input:
 *  - Start address of the region to unlock
 *  - End address of the region to lock
 * Output:    OK if lock is successful, ERROR otherwise
 */
__ramfunc unsigned int flash_unlock(void * vStart, void * vEnd) {

  // Local variables
  unsigned int page;
  unsigned int mask;
  unsigned int status;
  void * vLast;

  // Compute page number and last address of the region which will be locked
  page = ((unsigned int) vStart - (unsigned int) MEMORY_START_ADDRESS) & AT91C_MC_PAGEN;
  page =page<<1;

  vLast = (void *) ((unsigned int) MEMORY_START_ADDRESS +
          (page & MEMORY_REGION_MASK + MEMORY_REGION_SIZE) * MEMORY_PAGE_SIZE);

  // Program FMCN field in Flash Mode Register
  AT91C_BASE_MC->MC_FMR = (((MCK / 1000000) << 16) & AT91C_MC_FMCN) | AT91C_MC_FWS_1FWS;

  // Wait for EFC to be ready and perform command (interrupts disabled)
  while (!(AT91C_BASE_MC->MC_FSR & AT91C_MC_EOP));
  mask = AT91C_BASE_AIC->AIC_IMR;
  AT91C_BASE_AIC->AIC_IDCR = 0xFFFFFFFF;

  AT91C_BASE_MC->MC_FCR = page | AT91C_MC_FCMD_UNLOCK | AT91C_MC_CORRECT_KEY;

  do {

    status = AT91C_BASE_MC->MC_FSR;

  } while (!(status & AT91C_MC_EOP));
  AT91C_BASE_AIC->AIC_IECR = mask;

  // Result
  if (status & (AT91C_MC_LOCKE | AT91C_MC_PROGE)) {

    return ERROR;
  }
  else {

    // Unlock next region if range is too large
    if (vLast < vEnd) {

      return flash_unlock(vLast, vEnd);
    }
    return OK;
  }
}

#endif // USE_FLASH
