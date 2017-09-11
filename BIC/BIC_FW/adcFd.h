/*|***************************************************************************/
/*| PROJECT: Compact Compression CCU                                         */
/*| Module:   adcFd                                                          */
/*| Description:                                                             */
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
#include "adcHi.h"

/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
typedef struct
{
   UINT16 battery;
   UINT16 spare1;
   UINT16 spare2;
   UINT16 spare3;

   UINT16 spare4;
   UINT16 spare5;
   UINT16 spare6;
   UINT16 spare7;

}INTRNL_ADC_READING;


/*
*****************************************************************************
 P U B L I C    M A C R O S
*****************************************************************************
*/

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
extern INTRNL_ADC_READING IntrnlAdcReading;

extern INTRNL_ADC_READING IntrnlMinAdcReading;


void InitIntrnlAdc
(
   void
);

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: MeasureAnalog
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
BOOL MeasureAnalog( UINT8, BOOL, BOOL);


