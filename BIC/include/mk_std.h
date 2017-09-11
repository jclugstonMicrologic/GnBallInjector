/*|***************************************************************************/
/*|PROJECT:  Micrologic Kernel                                               */
/*|Module:   Standard headers                                                */
/*|Description:                                                              */
/*|   The header file for the kernel                                         */
/*|***************************************************************************/

/*
   ***************************************************************************
   Copyright (C) 2000, Micrologic Limited.
   Any information contained in this document which was hitherto unknown by
   the recipient or in which there subsists a proprietary right must not be
   disclosed to any other person, nor used for manufacturing purposes,
   without the permission of Micrologic Limited and the acceptance of this
   document will be construed as acceptance of the foregoing.
   ***************************************************************************
*/
#ifndef MK_STD_H
 #define MK_STD_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/


/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
typedef int BOOL;
typedef unsigned char UINT8;
typedef unsigned short int UINT16;
typedef unsigned long int UINT32;
typedef signed char INT8;
typedef short INT16;
typedef long INT32;


typedef enum
{
   OS_FIRST_STATUS_MSG = 0,
   OS_STATUS_OK,
   OS_TARGET_QUE_IS_FULL,
   OS_CMD_QUE_IS_FULL,
   OS_FUNCTION_NOT_IMPLEMENTED,
   OS_LAST_STATUS_MSG
}OS_STATUS;

typedef UINT32 OS_TASK;



/******
    PFV_PV:    Pointer to a function which returns void and takes
               a pointer to void.   Used to cast OS_TASK which is
               UINT32 (and can not be called using strict type checking).
******/        
typedef void (*PFV_PV)( void * ptr_ );

/******
    PFV_V:     Pointer to a function which returns void and takes
               void. 
******/
typedef void (*PFV_V)( void );


typedef enum
{
   NO_PARITY   = (UINT8)0,
   ODD_PARITY  = (UINT8)1,
   EVEN_PARITY = (UINT8)2
} PARITY_TYPE;


/*
*****************************************************************************
 P U B L I C    M A C R O S
*****************************************************************************
*/
/* describe what to do when DEBUG macro is invoked */
#define DEBUG(message, taskId) printf(message);printf("\n")

/* define macro that can be used to enable DEBUG macro in all modules.  */
/*  Set to 1 to turn debugging on, 0 to allow debugging to be enabled   */
/*  on a per-module basis */
#define DEBUG_ALL_ON (1)


/* General purpose definitions */

#define NULL_PTR  ((void *)0)
#define OK ((int)1)
#define MBYTE ((UINT32)0x0FFFFF)


/* Macros to set and clear bits */
#define   SET( REG, BITS )  ((REG) |= (BITS))
#define UNSET( REG, BITS )  ((REG) &= ~(BITS))

/* check if a bit in REG is high or low */
/* returns zero if the bit is zero, returns non-zero if bit is high*/
#define CHECK(REG, ONE_BIT_MASK)  ((REG) & (ONE_BIT_MASK))

/* Macro to write bits into certain positions of a register; only the bits */
/* specified in BITMASK can be changed regardless of the value of BITS     */
#define OVERWRITE( REG, BITMASK, BITS)    ( REG = ((REG & ~(BITMASK)) | ((BITS) & (BITMASK))) )

#define OVERWRITE_UINT16( REG, BITMASK, BITS)   REG = (UINT16)(  (REG & ~(BITMASK)) | ( (BITS) & (BITMASK) )  )

#define OVERWRITE_UINT8( REG, BITMASK, BITS)  REG = (UINT8)(  (REG & ~(BITMASK)) | ( (BITS) & (BITMASK) )  )

#define OVERWRITE32( REG, BITMASK, BITS)   REG = (UINT32)(  (REG & ~(BITMASK)) | ( (BITS) & (BITMASK) )  )


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



#endif
/* mk_std.h */


