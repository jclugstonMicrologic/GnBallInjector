/*
*|***************************************************************************
*| PROJECT:  Compact Compression CCU
*| Module:   PID function driver
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

#ifndef PID_FD_H
 #define PID_FD_H

/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "mk_std.h"

#include "C:\Software\CompactCompressors\CCU\commonCCU\parseFd.h"
/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
typedef struct
{
   UINT8 machState;
   UINT32 steadyStateTimer;
}PID_WINDUP;

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
extern PID_WINDUP InletPidWindup;
extern PID_WINDUP LpRecycleWindup;
extern PID_WINDUP HpRecycleWindup;

extern float SuctionPressSdSetPnt;
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
 P R O C E D U R E S
*****************************************************************************
*/

/*
*|----------------------------------------------------------------------------
*|  Module: PID Module
*|  Routine: InitPid
*|  Description:
*|   Initialisation for this module
*|----------------------------------------------------------------------------
*/
void InitPid( void );

float UpdatePid
(
   COMPRESSOR_PID *pidPtr_,
   float error_,
   float position_
);

BOOL Pid_CheckSteadyState
(
   float error_
);


BOOL Pid_WindupStateMachine
(
   float error_,
   float *iStatePtr_,
   UINT8 cntrlDrive_,           // 0%-100% control
   PID_WINDUP *pidWindupPtr_
);

#endif



