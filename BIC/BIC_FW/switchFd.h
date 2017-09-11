/*
*|***************************************************************************
*|PROJECT: Compact Compression CCU
*|Module:  Switch function driver
*|Description:
*|   The module that detects the user input switch.  It uses callback
*|   routines to allow different actions to be invoked.  Even though
*|   only one state machine is used, this module handles N independent switches.
*|**************************************************************************
*/

/*
Tools:
   Compiler: IAR Kickstart development environment for AT91SAM7Sxxx
****************************************************************************
*/


/*---------------------------------------------------------------------------*/
/* Copyright (C) 2001, Micrologic Limited.                                   */
/* Any information contained in this document which was hitherto unknown by  */
/* the recipient or in which there subsists a proprietary right must not be  */
/* disclosed to any other person, nor used for manufacturing purposes,       */
/* without the permission of Micrologic Limited and the acceptance of this   */
/* document will be construed as acceptance of the foregoing.                */
/*---------------------------------------------------------------------------*/
#ifndef SWITCH_FD_H
 #define SWITCH_FD_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "mk_Std.h"


/*
*****************************************************************************
 P U B L I C    M A C R O S
*****************************************************************************
*/
/* the number of switches */
#define MAX_NUM_SWITCHES ((UINT8)10)

/* the number of counts that must elapse otherwise the switch is
   is considered transient */
#define BOUNCE_COUNTS          ((UINT32)20)
#define REPEAT_COUNTS          ((UINT32)1000)
#define HELD_ASSERT_COUNTS     ((UINT32)2)
#define REPEAT_NEW_RATE_COUNTS ((UINT32)40)

#define OFFSET ((UINT16)0x0001)

/* states of the state machine */
enum SWITCH_MACHINE_STATES
{
   SWITCH_IDLE_STATE =0,
   NORMAL_NEGATED_STATE,
   DEBOUNCE_ASSERTING_STATE,
   ASSERTED_STATE,
   DEBOUNCE_NEGATING_STATE,
   HELD_ASSERTED_STATE,
   HELD_ASSERTED_NEW_RATE_STATE,
   DEBOUNCE_RELEASING_HELD_STATE
};


/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
typedef struct
{
   UINT8 modeType;
   UINT8 machineState;
   UINT8 assertCounts;
   UINT32 bounce;
   UINT32 start;
   void (*callBackPtr1)();
   void (*callBackPtr2)();
   void (*callBackPtr3)();
   void (*callBackPtr4)();

}SWITCH_DATA;


/*
*****************************************************************************
 P U B L I C   D A T A
****************************************************************************
*/
/* context data for each switch */
extern SWITCH_DATA SwitchData[MAX_NUM_SWITCHES];

extern UINT16 RealSwitchState;
/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/


/*
*|----------------------------------------------------------------------------
*|  Module: switchFd
*|  Routine: InitSwitch
*|  Description
*|   Initialises all hardware and software for the switches
*|----------------------------------------------------------------------------
*/
void InitSwitch           /*| returns false if error detected,                */
(                         /*|                                                 */
   UINT8 switchId_,       /*| identify the switch being set up                */
   void *callBackPtr1_,   /*| function to run when switch is pulsed           */
   void *callBackPtr2_,   /*| function that runs every N cycles thru state    */
                          /*|    machine when switch is held asserted         */
   void *callBackPtr3_,   /*| function that is run when switch is just        */
                          /*|    entering the "held-asserted" state           */
   void *callBackPtr4_    /*| function that is run when switch is just        */
                          /*|    leaving the "held-asserted" state            */
);


/*
*|----------------------------------------------------------------------------
*|  Module: switchFd
*|  Routine: SwitchMachine
*|  Description
*|   The state machine that detects the condition of the toggle switches.
*|----------------------------------------------------------------------------
*/
void SwitchMachine
(
   UINT8 switchId_
);

BOOL GetSwitchStatus
(
   UINT8 switchId_
);

#endif
/* switchFd.h */
