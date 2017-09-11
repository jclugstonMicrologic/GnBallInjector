/*|***************************************************************************/
/*|PROJECT: Compact Compression CCU                                          */
/*|Module:   PID Control                                                     */
/*|Description:                                                              */
/*|                                                                          */
/*|***************************************************************************/


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
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "PidFd.h"
#include "memoryFd.h"
#include "AlarmFd.h"
#include "board.h"
#include "Timer.h"
#include "UsbComFd.h"

#include "string.h"

/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
enum
{
   WINDUP_IDLE_STATE =0,
   WINDUP_POTENTIAL_FULLY_OPEN_STEADY_STATE,
   WINDUP_POTENTIAL_FULLY_CLOSED_STEADY_STATE,
   WINDUP_FULLY_OPENED_STEADY_STATE,
   WINDUP_FULLY_CLOSED_STEADY_STATE,
   WINDUP_LAST_STATE
};

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/
#define PROPORTIONAL_BAND_ERROR (2.0)

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
PID_WINDUP InletPidWindup;
PID_WINDUP LpRecycleWindup;
PID_WINDUP HpRecycleWindup;

float SuctionPressSdSetPnt;
/*
*****************************************************************************
 P R I V A T E   G L O B A L   D A T A
*****************************************************************************
*/
BOOL PidStable;

/*
*****************************************************************************
 P R I V A T E   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/
void AdjustControlSettings(UINT8 *tempBufPtr_, CONTROL_SETTINGS *cntrlSettingsPtr_);


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
*|   Initialization for this module
*|----------------------------------------------------------------------------
*/
void InitPid
(
   void
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT16 tempCrc;
   UINT32 flashStartAddr;

   float tempInletDState;
   float tempInletIState;

   float templpDState;
   float templpIState;

   float temphpDState;
   float temphpIState;

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
   // retain these states
   tempInletDState =ControlSettings.inletPres.pidParam.dState;
   tempInletIState =ControlSettings.inletPres.pidParam.iState;

   templpDState =ControlSettings.lpRecycle.pidParam.dState;
   templpIState =ControlSettings.lpRecycle.pidParam.iState;

   temphpDState =ControlSettings.hpRecycle.pidParam.dState;
   temphpIState =ControlSettings.hpRecycle.pidParam.iState;

   // read flash starting at the ControlSettings address
   flashStartAddr =FLASH_ADDR_SETPOINT_DATA; //sizeof(ToolInspctnHeader) + sizeof(ToolConfigHeader);
//   ReadFromSector(flashStartAddr, tempBuf, sizeof(ControlSettings));
//   AdjustControlSettings( tempBuf, &ControlSettings);

   ReadFromSector(flashStartAddr, (UINT8 *)&ControlSettings, sizeof(ControlSettings));   

   tempCrc =CalcCrc( (UINT8 *)&ControlSettings, sizeof(ControlSettings)-2);

   if( tempCrc !=ControlSettings.crc )
   {
      ControlSettings.inletPres.adjustFreq =10;
      ControlSettings.inletPres.pidParam.pGain =6.00;
      ControlSettings.inletPres.pidParam.iGain =0.02;
      ControlSettings.inletPres.pidParam.dGain =0.00;
      ControlSettings.inletPres.setPoint =5.0;

      ControlSettings.lpRecycle.adjustFreq =10;
      ControlSettings.lpRecycle.pidParam.pGain =10.00;
      ControlSettings.lpRecycle.pidParam.iGain =0.05;
      ControlSettings.lpRecycle.pidParam.dGain =0.00;
      ControlSettings.lpRecycle.setPoint =2.0;

      ControlSettings.hpRecycle.adjustFreq =10;
      ControlSettings.hpRecycle.pidParam.pGain =10.00;
      ControlSettings.hpRecycle.pidParam.iGain =0.05;
      ControlSettings.hpRecycle.pidParam.dGain =0.00;
      ControlSettings.hpRecycle.setPoint =150.0;
   }

   // if setpoint of 5.0 is not changed by user, revert to this value when entering run mode
//   SuctionPressSdSetPnt =ControlSettings.inletPres.setPoint;

   // always default to 5.0psi (requested Feb, 2009)
   ControlSettings.inletPres.setPoint =5.0;

   ControlSettings.inletPres.pidParam.dState =tempInletDState;
   ControlSettings.inletPres.pidParam.iState =tempInletIState;

   ControlSettings.lpRecycle.pidParam.dState =templpDState;
   ControlSettings.lpRecycle.pidParam.iState =templpIState;

   ControlSettings.hpRecycle.pidParam.dState =temphpDState;
   ControlSettings.hpRecycle.pidParam.iState =temphpIState;

   InletPidWindup.machState =0;
   LpRecycleWindup.machState =0;
   HpRecycleWindup.machState =0;

   flashStartAddr =FLASH_ADDR_SETPOINT_DATA +sizeof(ControlSettings);
   ReadFromSector(flashStartAddr, (UINT8 *)&MiscSettings, sizeof(MiscSettings));

   flashStartAddr =FLASH_ADDR_STATIC_DATA + sizeof(ToolInspctnHeader) + sizeof(ToolConfigHeader);
   ReadFromSector(flashStartAddr, (UINT8 *)&AlarmParameter, sizeof(AlarmParameter));

   flashStartAddr =FLASH_ADDR_STARTLOG_MEMORY;
//   ReadFromSector(flashStartAddr, (UINT8 *)&CompressorStatus, sizeof(CompressorStatus));
   ReadFromSector(flashStartAddr, (UINT8 *)&ShutdownParameters, sizeof(ShutdownParameters));
   CompressorStatus =ShutdownParameters;

   if( CHECK(CompressorStatus.type, ALARM_SHUTDOWN) )
   {
      UNSET(CompressorStatus.type, ALARM_SHUTDOWN);
      SET(CompressorStatus.type, ALARM_SD_RESET);
   }

   // out of reset we are not interested in any previous warnings
   UNSET(CompressorStatus.type, ALARM_WARNING);

   // !!! just use these for now, I am not sure what they should be
   ControlSettings.inletPres.pidParam.iMax =100/ControlSettings.inletPres.pidParam.iGain;;
   ControlSettings.inletPres.pidParam.iMin =0;

   ControlSettings.lpRecycle.pidParam.iMax =100/ControlSettings.lpRecycle.pidParam.iGain;
   ControlSettings.lpRecycle.pidParam.iMin =0;

   ControlSettings.hpRecycle.pidParam.iMax =100/ControlSettings.hpRecycle.pidParam.iGain;
   ControlSettings.hpRecycle.pidParam.iMin =0;

} // end InitPid()


/*
*|----------------------------------------------------------------------------
*|  Module: PID Module
*|  Routine: UpdatePid
*|  Description: PID control algorithm
*|----------------------------------------------------------------------------
*/
float UpdatePid
(
   COMPRESSOR_PID *pidPtr_,
   float error_,
   float position_
)
{
   float pTerm;
   float dTerm;
   float iTerm;


   // ensure max term is related to iGain
   pidPtr_->iMax =100/pidPtr_->iGain;

   // calculate the proportional term
   pTerm =pidPtr_->pGain * error_;

   // calculate the integral state with appropriate limiting
   pidPtr_->iState +=error_;

   if( pidPtr_->iState > pidPtr_->iMax )
   {
      pidPtr_->iState =pidPtr_->iMax;
   }
   else if( pidPtr_->iState < pidPtr_->iMin )
   {
      pidPtr_->iState =pidPtr_->iMin;
   }

   // calculate the integral term
   iTerm =pidPtr_->iGain * pidPtr_->iState;

   // calculate the derivative term
   dTerm =pidPtr_->dGain * (position_ - pidPtr_->dState);
   pidPtr_->dState =position_;

   // return drive value
   return pTerm + iTerm - dTerm;

} // end UpdatePid()


/*
*|----------------------------------------------------------------------------
*|  Module: PID Module
*|  Routine: Pid_CheckSteadyState
*|  Description: PID control algorithm
*|----------------------------------------------------------------------------
*/
BOOL Pid_CheckSteadyState
(
   float error_
)
{
   if( error_ < 0 &&
       error_ > -PROPORTIONAL_BAND_ERROR
     )
   {
      return true;
   }
   else if( error_ >= 0 &&
            error_ < PROPORTIONAL_BAND_ERROR
     )
   {
      // if actual error (diff in setpoint -actual)
      // exceeds the PROPORTIONAL_BAND_ERROR, )

      // PROPORTIONAL_BAND_ERROR arbitrarily picked
      return true;
   }
   else
      return false;
}

BOOL Pid_WindupStateMachine
(
   float error_,
   float *iStatePtr_,
   UINT8 cntrlDrive_,           // 0%-100% control
   PID_WINDUP *pidWindupPtr_
)
{
//   BOOL steadyState;
//   steadyState =Pid_CheckSteadyState( error_ );
   BOOL status =false;

   switch( pidWindupPtr_->machState )
   {
      case WINDUP_IDLE_STATE:
         if( //steadyState ||
             cntrlDrive_ ==100
           )
         {
            pidWindupPtr_->machState =WINDUP_POTENTIAL_FULLY_OPEN_STEADY_STATE;
            pidWindupPtr_->steadyStateTimer =TicksLeft;
         }
         else if( //steadyState ||
                  cntrlDrive_ ==0
           )
         {
            pidWindupPtr_->machState =WINDUP_POTENTIAL_FULLY_CLOSED_STEADY_STATE;
            pidWindupPtr_->steadyStateTimer =TicksLeft;
         }
          break;
      case WINDUP_POTENTIAL_FULLY_OPEN_STEADY_STATE:
         if( (TicksLeft -pidWindupPtr_->steadyStateTimer) >5000 )
         {
            pidWindupPtr_->machState =WINDUP_FULLY_OPENED_STEADY_STATE;
         }
         else if( //!steadyState &&
                  cntrlDrive_ <100
                )
         {
            pidWindupPtr_->machState =WINDUP_IDLE_STATE;
         }
         break;
      case WINDUP_POTENTIAL_FULLY_CLOSED_STEADY_STATE:
         if( (TicksLeft -pidWindupPtr_->steadyStateTimer) >5000 )
         {
            pidWindupPtr_->machState =WINDUP_FULLY_CLOSED_STEADY_STATE;
         }
         else if( //!steadyState &&
                  cntrlDrive_ >0
                )
         {
            pidWindupPtr_->machState =WINDUP_IDLE_STATE;
         }
         break;
      case WINDUP_FULLY_OPENED_STEADY_STATE:
         if( //!steadyState &&
             cntrlDrive_ <100
           )
         {
            // this is invoked if the drive becomes less than 100%,
            // we may want to invoke if the drive is trying to reduce,
            // not when it actually does, for now seems OK
            // reduce the iTerm drastically (remove iState windup)
            *iStatePtr_ =0;
            pidWindupPtr_->machState =WINDUP_IDLE_STATE;
         }
         else
            status =true;
         break;
      case WINDUP_FULLY_CLOSED_STEADY_STATE:
         if( //!steadyState &&
             cntrlDrive_ >0
           )
         {
            // this is invoked if the drive becomes greater than 0%,
            // we may want to invoke if the drive is trying to increase,
            // not when it actually does, for now seems OK
            // reduce the iTerm drastically (remove iState windup)
            *iStatePtr_ =0;
            pidWindupPtr_->machState =WINDUP_IDLE_STATE;
         }
         else
            status =true;
         break;
   }

   return status;
}



void AdjustControlSettings
(
   UINT8 *tempBufPtr_,
   CONTROL_SETTINGS *cntrlSettingsPtr_
)
{
   cntrlSettingsPtr_->inletPres.overshoot = ( (*tempBufPtr_ ++) <<24);
   cntrlSettingsPtr_->inletPres.overshoot +=( (*tempBufPtr_ ++) <<16);
   cntrlSettingsPtr_->inletPres.overshoot +=( (*tempBufPtr_ ++) <<8);
   cntrlSettingsPtr_->inletPres.overshoot +=( (*tempBufPtr_ ++) );   
   
   cntrlSettingsPtr_->inletPres.adjustFreq = ( (*tempBufPtr_ ++) <<24);
   cntrlSettingsPtr_->inletPres.adjustFreq +=( (*tempBufPtr_ ++) <<16);
   cntrlSettingsPtr_->inletPres.adjustFreq +=( (*tempBufPtr_ ++) <<8);
   cntrlSettingsPtr_->inletPres.adjustFreq +=( (*tempBufPtr_ ++) );   
   
   cntrlSettingsPtr_->inletPres.settleTime = ( (*tempBufPtr_ ++) <<24);
   cntrlSettingsPtr_->inletPres.settleTime +=( (*tempBufPtr_ ++) <<16);
   cntrlSettingsPtr_->inletPres.settleTime +=( (*tempBufPtr_ ++) <<8);
   cntrlSettingsPtr_->inletPres.settleTime +=( (*tempBufPtr_ ++) );      
   
   cntrlSettingsPtr_->inletPres.setPoint = ( (*tempBufPtr_ ++) <<24);
   cntrlSettingsPtr_->inletPres.setPoint +=( (*tempBufPtr_ ++) <<16);
   cntrlSettingsPtr_->inletPres.setPoint +=( (*tempBufPtr_ ++) <<8);
   cntrlSettingsPtr_->inletPres.setPoint +=( (*tempBufPtr_ ++) );         
}


#ifdef jeff
   float dState;
   float iState;

   float iMax;
   float iMin;

   float iGain;  // integral gain
   float pGain;  // proportional gain
   float dGain;  // derivative gain

}COMPRESSOR_PID;

typedef struct
{
   float overshoot;
   float adjustFreq;
   float settleTime;

   float setPoint;

   COMPRESSOR_PID pidParam;

} PARAMETERS;

// these values are setup by the GUI (or the CCU) and transmitted to the tool
typedef struct
{
   PARAMETERS inletPres;
   PARAMETERS lpRecycle;
   PARAMETERS hpRecycle;

   UINT16 spare;

   UINT16 crc;

}CONTROL_SETTINGS;
#endif

