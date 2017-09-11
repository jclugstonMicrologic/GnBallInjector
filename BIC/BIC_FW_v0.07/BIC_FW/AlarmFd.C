/*
*|***************************************************************************
*| PROJECT:  Compact Compression CCU
*| Module:   AlarmFd
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


/******************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include <stddef.h>

#include "LcdFd.h"
#include "AlarmFd.h"
#include "Timer.h"
#include "RtcFd.h"
#include "memoryFd.h"
#include "extAdcFd.h"
#include "string.h"
#include "DigitalInFd.h"
#include "UsbComFd.h"
#include "CompressorMachineFd.h"

/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/

enum
{
   ALARM_INIT_STATE =0,
   ALARM_IDLE_STATE,
   ALARM_START_STATE,
   ALARM_ACK_STATE,
   ALARM_LATCH_STATE,
   ALARM_MOMENTARY_STATE,
   ALARM_SD_RESET_STATE,
   ALARM_SD_BYPASS_STATE,
   ALARM_WARNING_BYPASS_STATE,
   ALARM_NONE_STATE,

   ALARM_LAST_STATE
};

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/
#define ENGINE_TEMP_MAX   62400  // (3900 12bit)
#define COMPR_DC_TEMP_MAX 62400  // (3900 12bit)

// hardware inputs SR1-SR16
#define SENSOR1_ALARM_NAME  ("Scrubber Level High")
#define SENSOR2_ALARM_NAME  ("Compr Oil Level Low")
#define SENSOR3_ALARM_NAME  ("Coal Filter DP     ")
#define SENSOR4_ALARM_NAME  ("Compr Oil Filter DP")
#define SENSOR5_ALARM_NAME  ("Low Eng Oil Level  ")
#define SENSOR6_ALARM_NAME  ("Low Eng Oil Pres   ")
#define SENSOR7_ALARM_NAME  ("ESD Alarm          ")
#define SENSOR8_ALARM_NAME  ("Gas Detected       ")

#define SENSOR9_ALARM_NAME  ("Spare1/Alarm9      ")
#define SENSOR10_ALARM_NAME ("Spare2/Alarm10     ")
#define SENSOR11_ALARM_NAME ("Spare3/Alarm11     ")
#define SENSOR12_ALARM_NAME ("Spare4/Alarm12     ")
#define SENSOR13_ALARM_NAME ("Spare5/Alarm13     ")
#define SENSOR14_ALARM_NAME ("Spare6/Alarm14     ")
#define SENSOR15_ALARM_NAME ("Spare7/Alarm15     ")
#define SENSOR16_ALARM_NAME ("Spare8/Alarm16     ")

// software inputs, trip on trigger levels
#define SENSOR17_ALARM_NAME ("Low Suction Pres   ")
#define SENSOR18_ALARM_NAME ("High Suction Pres  ")
#define SENSOR19_ALARM_NAME ("Low Discharge Pres ")
#define SENSOR20_ALARM_NAME ("High Discharge Pres")
#define SENSOR21_ALARM_NAME ("High Engine Temp   ")
#define SENSOR22_ALARM_NAME ("High Discharge Temp")
#define SENSOR23_ALARM_NAME ("Stop Key           ")

#define SENSOR_UNKNOWN_NAME ("Unknown Alarm      ")

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
ALARM_PARAMETER AlarmParameter[MAX_ALARMS];

// for diagnostic debug
BOOL AlarmStateChange;

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
void CheckInternalAlarms( void );
int CheckAlarmStatus( COMPRESSOR_STATUS *compressorStatusPtr_ );
void SetAlarmTimeouts( UINT32 alarmTimerPtr[32] );

/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/

/*
*|----------------------------------------------------------------------------
*|  Module: AlarmFd Module
*|  Routine: InitAlarm
*|  Description:
*|   Initialisation for this module
*|----------------------------------------------------------------------------
*/
void InitAlarm
(
   void
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 j;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */


   /*
   *************************
    C O D E
   *************************
   */

   
   // enable all contanct id alarm switches
   SwitchData[SCRUBBER_LEVEL_HIGH].callBackPtr1 =NULL_PTR;
   SwitchData[SCRUBBER_LEVEL_HIGH].callBackPtr2 =ScrubberLevelHiAlarm;
   SwitchData[SCRUBBER_LEVEL_HIGH].callBackPtr3 =ScrubberLevelHiAlarm;
   SwitchData[SCRUBBER_LEVEL_HIGH].callBackPtr4 =ScrubberLevelHiNegateAlarm;
   
   SwitchData[COMP_OIL_LEVEL_LOW].callBackPtr1 =NULL_PTR;
   SwitchData[COMP_OIL_LEVEL_LOW].callBackPtr2 =CompOilLevelAlarm;
   SwitchData[COMP_OIL_LEVEL_LOW].callBackPtr3 =CompOilLevelAlarm;
   SwitchData[COMP_OIL_LEVEL_LOW].callBackPtr4 =CompOilLevelNegateAlarm;

   SwitchData[COAL_FILTER_DP].callBackPtr1 =NULL_PTR;
   SwitchData[COAL_FILTER_DP].callBackPtr2 =HiCoalescerDpAlarm;
   SwitchData[COAL_FILTER_DP].callBackPtr3 =HiCoalescerDpAlarm;
   SwitchData[COAL_FILTER_DP].callBackPtr4 =HiCoalescerDpNegateAlarm;

   SwitchData[COMP_OIL_FILTER_DP].callBackPtr1 =NULL_PTR;
   SwitchData[COMP_OIL_FILTER_DP].callBackPtr2 =HiCompOilFilterDpAlarm;
   SwitchData[COMP_OIL_FILTER_DP].callBackPtr3 =HiCompOilFilterDpAlarm;
   SwitchData[COMP_OIL_FILTER_DP].callBackPtr4 =HiCompOilFilterDpNegateAlarm;
 
   SwitchData[LOW_ENGINE_OIL_LEVEL].callBackPtr1 =NULL_PTR;
   SwitchData[LOW_ENGINE_OIL_LEVEL].callBackPtr2 =EngineOilLevelAlarm;
   SwitchData[LOW_ENGINE_OIL_LEVEL].callBackPtr3 =EngineOilLevelAlarm;
   SwitchData[LOW_ENGINE_OIL_LEVEL].callBackPtr4 =EngineOilLevelNegateAlarm;

   SwitchData[LOW_ENG_OIL_PRESS].callBackPtr1 =NULL_PTR;
   SwitchData[LOW_ENG_OIL_PRESS].callBackPtr2 =LowEngOilPresAlarm;
   SwitchData[LOW_ENG_OIL_PRESS].callBackPtr3 =LowEngOilPresAlarm;
   SwitchData[LOW_ENG_OIL_PRESS].callBackPtr4 =LowEngOilPresNegateAlarm;
   
   SwitchData[ESD_ALARM].callBackPtr1 =NULL_PTR;
   SwitchData[ESD_ALARM].callBackPtr2 =EsdAlarm;
   SwitchData[ESD_ALARM].callBackPtr3 =EsdAlarm;
   SwitchData[ESD_ALARM].callBackPtr4 =EsdNegateAlarm;

   SwitchData[GAS_DETECTED].callBackPtr1 =NULL_PTR;
   SwitchData[GAS_DETECTED].callBackPtr2 =GasDetectedAlarm;
   SwitchData[GAS_DETECTED].callBackPtr3 =GasDetectedAlarm;
   SwitchData[GAS_DETECTED].callBackPtr4 =GasDetectedNegateAlarm;

   // spares
   SwitchData[ALARM9].callBackPtr1 =NULL_PTR;
   SwitchData[ALARM9].callBackPtr2 =Spare1Alarm;
   SwitchData[ALARM9].callBackPtr3 =Spare1Alarm;
   SwitchData[ALARM9].callBackPtr4 =Spare1NegateAlarm;

   SwitchData[ALARM10].callBackPtr1 =NULL_PTR;
   SwitchData[ALARM10].callBackPtr2 =Spare2Alarm;
   SwitchData[ALARM10].callBackPtr3 =Spare2Alarm;
   SwitchData[ALARM10].callBackPtr4 =Spare2NegateAlarm;

   SwitchData[ALARM11].callBackPtr1 =NULL_PTR;
   SwitchData[ALARM11].callBackPtr2 =Spare3Alarm;
   SwitchData[ALARM11].callBackPtr3 =Spare3Alarm;
   SwitchData[ALARM11].callBackPtr4 =Spare3NegateAlarm;

   SwitchData[ALARM12].callBackPtr1 =NULL_PTR;
   SwitchData[ALARM12].callBackPtr2 =Spare4Alarm;
   SwitchData[ALARM12].callBackPtr3 =Spare4Alarm;
   SwitchData[ALARM12].callBackPtr4 =Spare4NegateAlarm;

   SwitchData[ALARM13].callBackPtr1 =NULL_PTR;
   SwitchData[ALARM13].callBackPtr2 =Spare5Alarm;
   SwitchData[ALARM13].callBackPtr3 =Spare5Alarm;
   SwitchData[ALARM13].callBackPtr4 =Spare5NegateAlarm;

   SwitchData[ALARM14].callBackPtr1 =NULL_PTR;
   SwitchData[ALARM14].callBackPtr2 =Spare6Alarm;
   SwitchData[ALARM14].callBackPtr3 =Spare6Alarm;
   SwitchData[ALARM14].callBackPtr4 =Spare6NegateAlarm;

   SwitchData[ALARM15].callBackPtr1 =NULL_PTR;
   SwitchData[ALARM15].callBackPtr2 =Spare7Alarm;
   SwitchData[ALARM15].callBackPtr3 =Spare7Alarm;
   SwitchData[ALARM15].callBackPtr4 =Spare7NegateAlarm;

   SwitchData[ALARM16].callBackPtr1 =NULL_PTR;
   SwitchData[ALARM16].callBackPtr2 =Spare8Alarm;
   SwitchData[ALARM16].callBackPtr3 =Spare8Alarm;
   SwitchData[ALARM16].callBackPtr4 =Spare8NegateAlarm;


   for(j =CONTACT_ID0; j<(CONTACT_ID0 +NBR_ALARM_SWITCHES); j++)
   {
      SwitchData[j].bounce = 0;
      SwitchData[j].start  = 0;
      SwitchData[j].machineState = SWITCH_IDLE_STATE;

      AlarmParameter[j].machineState =ALARM_INIT_STATE;
      AlarmParameter[j].assert =0;
   }

#ifdef SETUP_BY_PC
   // current active alarms, and shutdownns are
   // Ao, A1, A2, A3, A4, A5, A6, A7
   AlarmParameter[0].type =ALARM_TYPE_WARNING;
   AlarmParameter[1].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[2].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[3].type =ALARM_TYPE_WARNING;
   AlarmParameter[4].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[5].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[6].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[7].type =ALARM_TYPE_SHUTDOWN;

   // current inactive alarms, and shutdownns are
   // A2, A3, A4, A5, A9, A10, A12, A13
   AlarmParameter[8].type =ALARM_TYPE_NONE;
   AlarmParameter[9].type =ALARM_TYPE_NONE;
   AlarmParameter[10].type =ALARM_TYPE_NONE;
   AlarmParameter[11].type =ALARM_TYPE_NONE;
   AlarmParameter[12].type =ALARM_TYPE_NONE;
   AlarmParameter[13].type =ALARM_TYPE_NONE;
   AlarmParameter[14].type =ALARM_TYPE_NONE;
   AlarmParameter[15].type =ALARM_TYPE_NONE;

   // internal alarms, not digital inputs
   // high, or low suctions pressure
   // high, or low discharge pressure
   // high compressor discharge temperature
   // high enginge temperature
   AlarmParameter[16].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[17].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[18].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[19].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[20].type =ALARM_TYPE_SHUTDOWN;
   AlarmParameter[21].type =ALARM_TYPE_SHUTDOWN;
#endif

   AlarmStateChange =0;

}/* end InitAlarm() */



/*
*|----------------------------------------------------------------------------
*|  Module: AlarmFd Module
*|  Routine: AlarmMachine
*|  Description:
*|
*|----------------------------------------------------------------------------
*/
void AlarmMachine
(
   COMPRESSOR_STATUS *compressorStatusPtr_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 alarmId;
   UINT32 alarmTimer[MAX_ALARMS];
   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   /*
   *************************
    C O D E
   *************************
   */

   if( CheckAlarmStatus( compressorStatusPtr_ ) )
      return;
   
   CheckInternalAlarms();

   // use either run mode timers, or bypass timers (if applicable)
   SetAlarmTimeouts( alarmTimer );

   for( alarmId =0; alarmId <MAX_ALARMS; alarmId++ )
   {
//#define FAKE_TIMEOUTS
#ifdef FAKE_TIMEOUTS
 // make all alarms 5 seconds for debug
 alarmTimer[alarmId] =5000;
#endif

      switch( AlarmParameter[alarmId].machineState )
      {
         case ALARM_INIT_STATE:
            if( (AlarmParameter[alarmId].type &ENABLE_ALARM) ==ENABLE_ALARM )
            {
               AlarmParameter[alarmId].machineState =ALARM_IDLE_STATE;
            }
            else if( (AlarmParameter[alarmId].type ==ALARM_TYPE_NONE) )
            {
               AlarmParameter[alarmId].machineState =ALARM_NONE_STATE;
            }
            else
            {
               // shutdown bypass timer
               AlarmParameter[alarmId].timer =TicksLeft;

               if( AlarmParameter[alarmId].bypassType ==2 )
                  AlarmParameter[alarmId].machineState =ALARM_SD_BYPASS_STATE;
               else
                  AlarmParameter[alarmId].machineState =ALARM_WARNING_BYPASS_STATE;
            }
            break;
         case ALARM_IDLE_STATE:
            if( AlarmParameter[alarmId].assert )
            {
               // start timing length of alarm assertion
               AlarmParameter[alarmId].timer =TicksLeft;
               AlarmParameter[alarmId].machineState =ALARM_START_STATE;
            }
            break;
         case ALARM_START_STATE:
            if( (TicksLeft-AlarmParameter[alarmId].timer) >alarmTimer[alarmId] )
            {
               // alarm has remained asserted for time out period, it is now time to take action
               AlarmParameter[alarmId].machineState =ALARM_ACK_STATE;
            }
            if( !AlarmParameter[alarmId].assert )
            {
               // alarm has negated, do nothing (return idle)
               AlarmParameter[alarmId].machineState =ALARM_IDLE_STATE;
            }
            break;
         case ALARM_ACK_STATE:
            // do shutdown, warning, or lockout based on alarm type
            switch( (AlarmParameter[alarmId].type &0x7F) )
            {
               case ALARM_TYPE_NONE:
                  AlarmParameter[alarmId].machineState =ALARM_MOMENTARY_STATE;
                  return;
               case ALARM_TYPE_WARNING:
                  // do warning
                  SET(compressorStatusPtr_->type, ALARM_WARNING);
                  compressorStatusPtr_->sensorWarn[CompressorStatus.nbrWarn++] =alarmId;

                  if( !CHECK(compressorStatusPtr_->type, ALARM_SHUTDOWN) )
                     LcdMachineState =LCD_WARNING_MODE_MENU_NO_CLEAR_MSG;
                  break;
               case ALARM_TYPE_SHUTDOWN:
                  // do shutdown
                  SET(compressorStatusPtr_->type, ALARM_SHUTDOWN);
                  LcdMachineState =LCD_ALARM_SHUTDOWN_MSG;
                  break;
               case ALARM_TYPE_LOCKOUT:
                  // do lockout
                  SET(compressorStatusPtr_->type, ALARM_LOCKOUT);
                  compressorStatusPtr_->sensorSd =alarmId;

                  LcdMachineState =LCD_ALARM_LOCKOUT_MSG;
                  break;
            }

            ShutdownParameters =*compressorStatusPtr_;

            ShutdownParameters.glycolTemp =DisplayCompressorStatus.glycolTemp;
            ShutdownParameters.comprDcTemp = DisplayCompressorStatus.comprDcTemp =COMPR_DC_TEMP;
            ShutdownParameters.suctionPres =DisplayCompressorStatus.suctionPres =SUCTION_PRESSURE;
            ShutdownParameters.dischargePres =DisplayCompressorStatus.dischargePres =DISCHARGE_PRESSURE;
            ShutdownParameters.sensorSd =DisplayCompressorStatus.sensorSd =alarmId;
            memcpy( &ShutdownParameters.sdTime, &RealTime, sizeof(ShutdownParameters.sdTime) );
            ShutdownParameters.crc =CalcCrc( (UINT8 *)&ShutdownParameters, sizeof(ShutdownParameters)-2);

            // save alarm status to memory
            WriteToSector( FLASH_ADDR_STARTLOG_MEMORY, (UINT8 *)&ShutdownParameters, sizeof(COMPRESSOR_STATUS) );

            if( compressorStatusPtr_->type !=ALARM_WARNING )
            {
               // unset any shutdown alarms now, this is OK because it
               // has been saved to memory, and we cannot leave the alarm
               // state until user intervention
               // go to latch state
               AlarmParameter[alarmId].machineState =ALARM_LATCH_STATE;
            }
            else
            {
               // go to momentary state
               AlarmParameter[alarmId].machineState =ALARM_MOMENTARY_STATE;
            }
            break;
         case ALARM_MOMENTARY_STATE:
            if( !AlarmParameter[alarmId].assert )
            {
               // alarm has negated, negate any user acknowledgements
               UNSET(compressorStatusPtr_->type, ALARM_WARNING);
               AlarmParameter[alarmId].machineState =ALARM_IDLE_STATE;
            }
            break;
         case ALARM_LATCH_STATE:
            // wait for user acknowledgement
            break;
         case ALARM_SD_RESET_STATE:
            LcdMachineState =LCD_ALARM_SHUTDOWN_MSG;
            // go to latch state
            AlarmParameter[alarmId].machineState =ALARM_LATCH_STATE;
            break;
         case ALARM_SD_BYPASS_STATE:
            if( (TicksLeft -AlarmParameter[alarmId].timer) >alarmTimer[alarmId] ||
                !CHECK( CompressorStatus.type, ALARM_BYPASS_ENABLE )
              )
            {
               // bypass timer expired, go to idle state
               AlarmParameter[alarmId].machineState =ALARM_IDLE_STATE;
            }
            break;
         case ALARM_WARNING_BYPASS_STATE:
            if( (TicksLeft -AlarmParameter[alarmId].timer) >alarmTimer[alarmId] )
            {
               // bypass timer expired, go to idle state
               AlarmParameter[alarmId].machineState =ALARM_IDLE_STATE;
            }
            break;
         case ALARM_NONE_STATE:
         default:
            break;
      } // end switch(alarm machine state)

   } // end for all alarms
} // end AlarmMachine()


void CheckInternalAlarms
(
   void
)
{
   // low suction pressure
   if( SUCTION_PRESSURE <MiscSettings.suctionPressSd.setpointLo )
   {
      // SUCTION_PRESSURE_RAW =(Pres/6.25+4)*196.608
      AlarmParameter[16].assert =1;
   }
   else
      AlarmParameter[16].assert =0;

   // high suction pressure
   if( SUCTION_PRESSURE > MiscSettings.suctionPressSd.setpointHi )
   {
      // SUCTION_PRESSURE_RAW =(Pres/6.25+4)*196.608
      AlarmParameter[17].assert =1;
   }
   else
      AlarmParameter[17].assert =0;

   // low discharge pressure
   if( DISCHARGE_PRESSURE < MiscSettings.suctionPressSd.setpointLo ) //(1153 +2.2*AdcReading[ADC_SUCTION_PRESS]) ) //35psi +2.2*SuctionPressure
   {
      // DISCHARGE_PRESSURE_RAW =(Pres/18.75+4)*196.608
      AlarmParameter[18].assert =1;
   }
   else
      AlarmParameter[18].assert =0;

   // high discharge pressure
   if( DISCHARGE_PRESSURE > MiscSettings.dischargePressSd.setpointHi )
   {
      // DISCHARGE_PRESSURE_RAW =(Pres/18.75+4)*196.608
      AlarmParameter[19].assert =1;
   }
   else
      AlarmParameter[19].assert =0;

   // high engine temperature
   if( ENGINE_TEMP_RAW >ENGINE_TEMP_MAX ||
ENGINE_TEMP_RAW <1000      
     )
   {
      // ENGINE_TEMP_RAW 1uA/Kelvin on a 6250ohm resistor
      AlarmParameter[20].assert =1;
   }
   else
      AlarmParameter[20].assert =0;

   // high compressor discharge temperature
   if( COMPR_DC_TEMP_RAW >COMPR_DC_TEMP_MAX )
   {
      // COMPR_DC_TEMP_RAW 1uA/Kelvin on a 6250ohm resistor
      AlarmParameter[21].assert =1;
   }
   else
      AlarmParameter[21].assert =0;
}


int CheckAlarmStatus
(
   COMPRESSOR_STATUS *compressorStatusPtr_
)
{
   if( CHECK(compressorStatusPtr_->type, ALARM_SD_DISABLE) ||
       CHECK(compressorStatusPtr_->type, ALARM_SHUTDOWN)
     )
   {
      // alarms are to be disabled during this period
      return 1;
   }
   else if( CHECK(compressorStatusPtr_->type, ALARM_SD_RESET) )
   {
      // we were in shutdown, and have been reset
      UNSET(compressorStatusPtr_->type, ALARM_SD_RESET);
      SET(compressorStatusPtr_->type, ALARM_SHUTDOWN);
      AlarmParameter[compressorStatusPtr_->sensorSd].machineState =ALARM_SD_RESET_STATE;
   }
   return 0;
}

void SetAlarmTimeouts
(
   UINT32 alarmTimerPtr[MAX_ALARMS]
)
{
   int j;

   for(j=0; j<MAX_ALARMS; j++)
   {
      if( AlarmParameter[j].machineState !=ALARM_SD_BYPASS_STATE &&
          AlarmParameter[j].machineState !=ALARM_WARNING_BYPASS_STATE
        )
      {
         alarmTimerPtr[j] =AlarmParameter[j].timeout;
      }
      else
      {
         switch( AlarmParameter[j].bypassType )
         {
            case 0:
               // no bypass timer
               alarmTimerPtr[j] =AlarmParameter[j].timeout;
               break;
            case 2:
               // 2 minute bypass timer
               alarmTimerPtr[j] =120000;
               break;
            case 3:
               // 10 minute bypass timer
               alarmTimerPtr[j] =600000;
               break;
            default:
               alarmTimerPtr[j] =AlarmParameter[j].timeout;
               break;
         }
      }
   }
}


/*
*|----------------------------------------------------------------------------
*|  Module: AlarmFd Module
*|  Routine:
*|  Description:
*|
*|----------------------------------------------------------------------------
*/

void HiCompOilFilterDpAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[COMP_OIL_FILTER_DP].assert =1;
}
void HiCompOilFilterDpNegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[COMP_OIL_FILTER_DP].assert =0;
}
void CompOilLevelAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[COMP_OIL_LEVEL_LOW].assert =1;
}
void CompOilLevelNegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[COMP_OIL_LEVEL_LOW].assert =0;
}
void ScrubberLevelHiAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[SCRUBBER_LEVEL_HIGH].assert =1;
}
void ScrubberLevelHiNegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[SCRUBBER_LEVEL_HIGH].assert =0;
}
void HiCoalescerDpAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[COAL_FILTER_DP].assert =1;
}
void HiCoalescerDpNegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[COAL_FILTER_DP].assert =0;
}
void EngineOilLevelAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[LOW_ENGINE_OIL_LEVEL].assert =1;
}
void EngineOilLevelNegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[LOW_ENGINE_OIL_LEVEL].assert =0;
}
void GasDetectedAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[GAS_DETECTED].assert =1;
}
void GasDetectedNegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[GAS_DETECTED].assert =0;
}
void LowEngOilPresAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[LOW_ENG_OIL_PRESS].assert =1;
}
void LowEngOilPresNegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[LOW_ENG_OIL_PRESS].assert =0;
}
void EsdAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ESD_ALARM].assert =1;
}
void EsdNegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ESD_ALARM].assert =0;
}

/*********** SPARE ALARMS/SHUTDOWNS ***********/
void Spare1Alarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM9].assert =1;
}
void Spare1NegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM9].assert =0;
}
void Spare2Alarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM10].assert =1;
}
void Spare2NegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM10].assert =0;
}
void Spare3Alarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM11].assert =1;
}
void Spare3NegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM11].assert =0;
}
void Spare4Alarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM12].assert =1;
}
void Spare4NegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM12].assert =0;
}
void Spare5Alarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM13].assert =1;
}
void Spare5NegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM13].assert =0;
}
void Spare6Alarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM14].assert =1;
}
void Spare6NegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM14].assert =0;
}
void Spare7Alarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM15].assert =1;
}
void Spare7NegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM15].assert =0;
}
void Spare8Alarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM16].assert =1;
}
void Spare8NegateAlarm(void)
{
   AlarmStateChange =1;
   AlarmParameter[ALARM16].assert =0;
}


void LookupAlarmName
(
   UINT8 alarmId_,
   UINT8 *aBufPtr_
)
{
   switch( alarmId_ )
   {
      case 0:
         strcpy( (char *)aBufPtr_, SENSOR1_ALARM_NAME);
         break;
      case 1:
         strcpy( (char *)aBufPtr_, SENSOR2_ALARM_NAME);
         break;
      case 2:
         strcpy( (char *)aBufPtr_, SENSOR3_ALARM_NAME);
         break;
      case 3:
         strcpy( (char *)aBufPtr_, SENSOR4_ALARM_NAME);
         break;
      case 4:
         strcpy( (char *)aBufPtr_, SENSOR5_ALARM_NAME);
         break;
      case 5:
         strcpy( (char *)aBufPtr_, SENSOR6_ALARM_NAME);
         break;
      case 6:
         strcpy( (char *)aBufPtr_, SENSOR7_ALARM_NAME);
         break;
      case 7:
         strcpy( (char *)aBufPtr_, SENSOR8_ALARM_NAME);
         break;
      case 8:
         strcpy( (char *)aBufPtr_, SENSOR9_ALARM_NAME);
         break;
      case 9:
         strcpy( (char *)aBufPtr_, SENSOR10_ALARM_NAME);
         break;
      case 10:
         strcpy( (char *)aBufPtr_, SENSOR11_ALARM_NAME);
         break;
      case 11:
         strcpy( (char *)aBufPtr_, SENSOR12_ALARM_NAME);
         break;
      case 12:
         strcpy( (char *)aBufPtr_, SENSOR13_ALARM_NAME);
         break;
      case 13:
         strcpy( (char *)aBufPtr_, SENSOR14_ALARM_NAME);
         break;
      case 14:
         strcpy( (char *)aBufPtr_, SENSOR15_ALARM_NAME);
         break;
      case 15:
         strcpy( (char *)aBufPtr_, SENSOR16_ALARM_NAME);
         break;
      case 16:
         strcpy( (char *)aBufPtr_, SENSOR17_ALARM_NAME);
         break;
      case 17:
         strcpy( (char *)aBufPtr_, SENSOR18_ALARM_NAME);
         break;
      case 18:
         strcpy( (char *)aBufPtr_, SENSOR19_ALARM_NAME);
         break;
      case 19:
         strcpy( (char *)aBufPtr_, SENSOR20_ALARM_NAME);
         break;
      case 20:
         strcpy( (char *)aBufPtr_, SENSOR21_ALARM_NAME);
         break;
      case 21:
         strcpy( (char *)aBufPtr_, SENSOR22_ALARM_NAME);
         break;
      case 22:
         strcpy( (char *)aBufPtr_, SENSOR23_ALARM_NAME);
         break;
      default:
         strcpy( (char *)aBufPtr_, SENSOR_UNKNOWN_NAME);
         break;
   }
}

// end AlarmFd.c



#ifdef jeff
   if( (DigitalIn[FEEDBACK] & 0x1f00 ) != 0x1f00 )
   {
      // current flow failure (V to I failure)
      LcdMachineState =LCD_VI_FAILURE_MSG;
   }
   else
   {
      // current flowing (V to I OK)
      LcdMachineState =LCD_VI_OK_MSG;
   }
#endif

