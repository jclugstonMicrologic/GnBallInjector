/*
*|***************************************************************************
*|PROJECT:  Compact Compressor CCU
*|Module:   KeypadFd
*|   NOTES:
*|**************************************************************************
*/
/*
Tools:
   Compiler: IAR Kickstart development environment for AT91SAM7Sxxx
   ***************************************************************************
   Copyright (C) 2001, Micrologic Limited.
   Any information contained in this document which was hitherto unknown by
   the recipient or in which there subsists a proprietary right must not be
   disclosed to any other person, nor used for manufacturing purposes,
   without the permission of Micrologic Limited and the acceptance of this
   document will be construed as acceptance of the foregoing.
   ***************************************************************************
*/

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
#ifndef KEYPAD_FD_H
 #define KEYPAD_FD_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "switchFd.h"
#include "PidFd.h"

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


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
extern UINT8 KeyActionRequest;
extern INT8 LcdRow;
extern INT8 LcdSelection;
/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/


/*
*|----------------------------------------------------------------------------
*|  Module: KeypadFd Module
*|  Routine: InitKeypad
*|  Description:
*|   Defines the characteristics for each switch.
*|----------------------------------------------------------------------------
*/
void InitKeypad
(
   void
);

void KeypadMachine
(
   UINT8 switchId_
);

void CenterKey
(
   void
);

void KeyRight(void);
void KeyUp(void);
void KeyUpHeldAssert(void);
void KeyDown(void);
void KeyDownHeldAssert(void);
void KeyEnter(void);
void Key5(void);

enum
{
   CONTACT_ID =0,
   KEYBOARD,
   FEEDBACK

};

enum
{    
   KEY_LEFT,
   KEY_UP,     
   KEY_DOWN,                      
   KEY_ENTER,      
   KEY_5,  

   KEY_HELD_UP,
   KEY_HELD_DOWN,
     
   KEY_UPDATE_SCREEN,
   KEY_VIRTUAL,
     
   KEY_NONE,
};

#endif
