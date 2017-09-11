/*
*|***************************************************************************
*|PROJECT:  Compact Compressor CCU
*|Module:   AlarmFd
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
#ifndef ALARM_FD_H
 #define ALARM_FD_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "switchFd.h"
#include "C:\Software\CompactCompressors\CCU\commonCCU\parseFd.h"
/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/

#define NBR_ALARM_SWITCHES (16)

#define ALARM_NONE          (0x00)
#define ALARM_WARNING       (0x01)
#define ALARM_SHUTDOWN      (0x02)
#define ALARM_LOCKOUT       (0x04)
#define ALARM_SD_RESET      (0x08)
#define ALARM_SD_DISABLE    (0x10)
#define ALARM_BYPASS_ENABLE (0x20)
#define ALARM_DIAGNOSTIC    (0x40)

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
extern BOOL AlarmStateChange;

/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/


/*
*|----------------------------------------------------------------------------
*|  Module: AlarmFd Module
*|  Routine: InitAlarm
*|  Description:
*|   Defines the characteristics for each switch.
*|----------------------------------------------------------------------------
*/
void InitAlarm
(
   void
);

void AlarmMachine
(
   COMPRESSOR_STATUS *compressorStatusPtr_
);

void HiCompOilFilterDpAlarm(void);
void HiCompOilFilterDpNegateAlarm(void);
void CompOilLevelAlarm(void);
void CompOilLevelNegateAlarm(void);
void ScrubberLevelHiAlarm(void);
void ScrubberLevelHiNegateAlarm(void);
void HiCoalescerDpAlarm(void);
void HiCoalescerDpNegateAlarm(void);
void EngineOilLevelAlarm(void);
void EngineOilLevelNegateAlarm(void);
void GasDetectedAlarm(void);
void GasDetectedNegateAlarm(void);
void LowEngOilPresAlarm(void);
void LowEngOilPresNegateAlarm(void);
void EsdAlarm(void);
void EsdNegateAlarm(void);

void Spare1Alarm(void);
void Spare1NegateAlarm(void);
void Spare2Alarm(void);
void Spare2NegateAlarm(void);
void Spare3Alarm(void);
void Spare3NegateAlarm(void);
void Spare4Alarm(void);
void Spare4NegateAlarm(void);
void Spare5Alarm(void);
void Spare5NegateAlarm(void);
void Spare6Alarm(void);
void Spare6NegateAlarm(void);
void Spare7Alarm(void);
void Spare7NegateAlarm(void);
void Spare8Alarm(void);
void Spare8NegateAlarm(void);

void LookupAlarmName( UINT8 alarmId_, UINT8 *aBufPtr_ );

#endif

/* End modeFd.h */


