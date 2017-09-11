/*|***************************************************************************/
/*|PROJECT: Compact Compressor CCU                                           */
/*|Module:  TaskManager                                                      */
/*|Description:                                                              */
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

#ifndef TASKMANAGER_H
 #define TASKMANAGER_H


/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/


/*
*****************************************************************************
 P U B L I C    M A C R O S
*****************************************************************************
*/

// main system monitor
#define MODULE_SYSTEM

// external ADC present
#define MODULE_EXTERNAL_ADC

// internal ADC present
#define MODULE_INTERNAL_ADC

// exteranal DAC present
#define MODULE_EXTERNAL_DAC

// LCD display present
#define MODULE_LCD_DISPLAY

// switch debounce required
//#define MODULE_SWITCH_DEBOUNCE

// kepad present
//#define MODULE_KEYPAD

// real time clock present
#define MODULE_RTC

// external flash memory present
#define MODULE_EXTERNAL_FLASH

// external socket modem present
//#define MODULE_SOCKET_MODEM

// digital inputs from shift register present
#define MODULE_DIGITAL_IN

// USB hardware detect enabled
#define MODULE_USB_DETECT

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
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/
void TaskManager( void );
void InitTargetHw( void );


#endif







