/*|***************************************************************************/
/*|PROJECT: GN BIC                                                           */ 
/*|Module:   lcdFd                                                           */
/*|Description: function driver for the lcd module                           */
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

#ifndef LCDFD_H
 #define LCDFD_H

/*************************************************************************/
/* "I N C L U D E   F I L E S                                            */
/*************************************************************************/

#include "cpuPorts.h"
#include "lcdHi.h"
#include "C:\Software\CompactCompressors\CCU\commonCCU\parseFd.h"

/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
#define SIX_BALLS_PER_REV

//#define BALL_DROPPER

#ifndef BALL_DROPPER
#define GN_VERSION       "V1.00"     // "V0.12"  vesion 1.xx is 6balls/rev 250 max ball count
#define GN_DATE          "12/04/16"  // "08/24/13"      
#else
#define GN_VERSION       "V0.00"
#define GN_DATE          "05/04/14"      
#endif

//                        | |||
//                        | |||-- Test Version, space indicates general release,
//                        | ||    a letter indicates a test version  (a-z. A-Z)
//                        | ||
//                        | ||--- Minor Version
//                        |
//                        |------ Major Version

enum
{
   LCD_START_SCREEN =0,
   LCD_MAIN_SCREEN,
   
   LCD_RUN_SCREEN,
   LCD_LOAD_SCREEN,
   LCD_SETUP_SCREEN,

   LCD_RUN_AUTO_DIR_SCREEN,
   LCD_RUN_AUTO_RATE_SCREEN,
     
   LCD_RUN_MANUAL_SCREEN,

   LCD_LOAD_INDEX_SCREEN,

   LCD_SETUP_CANNISTER_SCREEN,
   LCD_SETUP_BATTERY_SCREEN,
         
   
   LCD_SETUP_TEST_DIR_SCREEN,
   LCD_SETUP_TEST_RATE_SCREEN,   

   LCD_DROPPER_SCREEN,
   
   LCD_MSG_DISPLAYED
};

// screen updates are enumed here, the 0x80 allows the LCD machine
// to know not to clear the lcd (if it did clear it, we would get flicker)
enum
{
   LCD_RUN_AUTO_UPDATE_DIR_SCREEN =0x80,   
   LCD_RUN_AUTO_UPDATE_RATE_SCREEN,
   
   LCD_RUN_MANUAL_UPDATE_SCREEN,
   
   LCD_LOAD_UPDATE_SCREEN,
   LCD_LOAD_INDEX_UPDATE_SCREEN,
     
   LCD_SETUP_BATTERY_UPDATE_SCREEN,
   
   LCD_SETUP_CANNISTER_UPDATE_SCREEN,
     
   LCD_SETUP_TEST_UPDATE_DIR_SCREEN,     
   LCD_SETUP_TEST_UPDATE_RATE_SCREEN,   
   
   LCD_LAST_UPDATE_SCREEN,
   LCD_SETUP_UPDATE_SCREEN,

   LCD_DROPPER_UPDATE_SCREEN,    
     
   LCD_UPDATE_LAST_SCREEN
};

enum
{
   ROW_MENU_SELECTION =0,
   ROW_TEXT1,
   ROW_TEXT2,
   ROW_TEXT3,
   ROW_TEXT4,

   ROW_LAST
};



/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/


extern UINT8 LcdMachineState;
extern UINT16 DisplayBallsPerMin;

/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/


/*|------------------------------------------------------------------------
  |  Module: lcdFd.c
  |  Routine: InitLcd
  |  Description:
  |------------------------------------------------------------------------
*/
void InitLcd
(
   void
);


void DisplayLcdOff
(
   void
);


void DisplayLcdOn(void);


void DisplayLcdMessage
(
   UINT8 *lcdMsgPtr_,    /* message string to be sent */
   UINT8 row_,
   UINT8 col_,
   UINT8 intensity_
);

void Display
(
   UINT8 row
);

void DisplayRun
(
   UINT8 row
);


/*|------------------------------------------------------------------------
  |  Module: lcdFd.c
  |  Routine: DisplayLcdGraphic
  |  Description:
  |------------------------------------------------------------------------
*/
void DisplayLcdGraphic
(
   UINT8 character_,
   UINT8 row_,
   UINT8 col_
);


void LcdMachine
(
   void
);

BOOL IntToAscii
(
   UINT32 intValue_,
   UINT8 * resultPtr_,
   UINT8 numOfAscii_
);

BOOL CharToAscii
(
   UINT8 theChar_,      /*| the byte to be converted into ASCII */
   UINT8 * resultPtr_    /*| location to store the most signif byte of result  */
                        /*| the next location stores least signif byte        */
);

BOOL HexToDec
(
   UINT8 theChar_,      /*| the byte to be converted into ASCII */
   UINT8 * resultPtr_    /*| location to store the most signif byte of result  */
                        /*| the next location stores least signif byte        */
);

void AnalogToEng
(
   CONTROL_SETTINGS *controlSettingsPtr_,
   COMPRESSOR_STATUS *compStatusPtr_,
   MISC_SETTINGS *miscSettingsPtr_
);

#endif



