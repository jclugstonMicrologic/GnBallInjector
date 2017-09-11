/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*|Module:   Timer                                                           */
/*|Description:                                                              */
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

#include "board.h"			// Hardware specific header
#include "timer.h"	

#include "cpuPorts.h"

#include "string.h"
#include "time.h"
#include "motorcntrl.h"
#include "PwmDriverFd.h"

#define TIMER0_INTERRUPT_LEVEL	7
#define TIMER1_INTERRUPT_LEVEL	7
#define TIMER2_INTERRUPT_LEVEL	7
#define SOFT_INTERRUPT_LEVEL    0

// for a 32.768kHz clock the count should be 0x8000, but the following
// value gives exactly 1Hz
#define ONE_SECOND_TICK_COUNT (0x8300) //(0x83d2)

#define ONE_MSEC_TICK_COUNT (0x980) //(0xe10)

UINT32 volatile TicksLeft, SecondsCount;

// flag indicating input capture timer has wrapped (odom speed too slow)
UINT8 CounterOverFlowMask;

int AicMaskReg;

int Q =0;
    
OS_TIMER OsTimer[TIMER_LAST];

volatile BOOL SpiAvailable;

extern void AT91F_SpuriousInt(void);
extern BOOL SendByte( UINT8 );

UINT16 TotalVaneCount=0;

#define WDT_WDV_TIMER_BASE  (0x400)  // WDT
#define WDT_WDD_KICK_WINDOW (0x800)  // WDD, where  0<WDT<WDD

#define TIMER_GET_TIME TicksLeft


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: SysTimer
*|  Description: Increments the 1 msec timer ticks
*|
*|  Note: SYS consist of 6 peripherals, so if AIC_IPR is set, then
*|        each of the 6 individual status registers must be checked
*|        for a pending interrupt (if they are enabled)
*|
*|        6 peripherals are
*|        PIT   RTT   WDT   DBGU   PMC   RSTC
*|
*|        These are relatively slow interrupts so polling should be
*|        able to keep up
*|----------------------------------------------------------------------------
*/
void SysTimer
(
   void
)
{
   UINT8 timerNbr;  
   UINT32 dummy;

   // check if a system interrupt is pending
//   if( (AT91C_BASE_AIC->AIC_IPR & (1<<AT91C_ID_SYS)) == (1<<AT91C_ID_SYS))
   {
      // determine which interrupt(s) is pending
      if( (AT91C_BASE_PITC->PITC_PISR & AT91C_PITC_PITS) ==AT91C_PITC_PITS )
      {
         // increment system 1msec tick timer by number of PIT occurences
         // should only be one, unless some were missed
         // reading PIVR will clear the PIT interrupt as well
         TicksLeft ++;

         dummy = ((AT91C_BASE_PITC->PITC_PIVR &0xfff00000)>>20);

         for( timerNbr=0; timerNbr<TIMER_LAST; timerNbr++ )
         {
            OsTimerTimeout( &OsTimer[timerNbr] );
         }       

      } // PIT interrupt


      if( (AT91C_BASE_RTTC->RTTC_RTSR & 0x02) ==0x02 )
      {
         // RTVR keeps a running count of seconds passed
         // so just equate seconds count to RTVR
         dummy =AT91C_BASE_RTTC->RTTC_RTVR;

         // do not reload counter
         // AT91C_BASE_RTTC->RTTC_RTMR =0x40000 | ONE_SECOND_TICK_COUNT;

      } // RTT interrupt


      // other SYS peripherals
      dummy =AT91C_BASE_WDTC->WDTC_WDSR;
      dummy =AT91C_BASE_DBGU->DBGU_CSR;
      dummy =AT91C_BASE_PMC->PMC_SR;
      dummy =AT91C_BASE_RSTC->RSTC_RSR;

      dummy =dummy;
   }

   SpiAvailable =true;
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: OsStartPeriodicTimer
*|  Description:
*|----------------------------------------------------------------------------
*/
void OsStartPeriodicTimer
(
   OS_TIMER *timerPtr_,
   UINT32 timeOut_
)
{
   timerPtr_->startTime =TIMER_GET_TIME;
   timerPtr_->timePeriod =timeOut_;
   timerPtr_->periodic =true;
   timerPtr_->timeOut =false;
   timerPtr_->running =true;
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: OsStartOneShotTimer
*|  Description: This timer times out once, must be restarted
*|----------------------------------------------------------------------------
*/
void OsStartOneShotTimer
(
   OS_TIMER *timerPtr_,
   UINT32 timeOut_
)
{
   timerPtr_->startTime =TIMER_GET_TIME;
   timerPtr_->timePeriod =timeOut_;
   timerPtr_->periodic =false;
   timerPtr_->timeOut =false;
   timerPtr_->running =true;
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: OsTimerTimeout
*|  Description: This routine is monitored within the timer isr, if the
*|               timer is running, the associated callback routine
*|               will be invoked
*|
*|               note the user should not call this to check if a timer
*|               has timed out, instead use OsCheckTimerTimeout
*|----------------------------------------------------------------------------
*/
BOOL OsTimerTimeout
(
   OS_TIMER *timerPtr_
)
{
   if( !timerPtr_->running )
      return false;

   if( (TIMER_GET_TIME - timerPtr_->startTime) > timerPtr_->timePeriod )
   {
#ifdef jeff
      if( !timerPtr_->periodic )
      {
         // disable the timer, one shot timer
         timerPtr_->running =false;
      }
      else
      {
         // restart the timer, periodic timer
         timerPtr_->startTime =TIMER_GET_TIME;
      }
#endif
      if( timerPtr_->periodic )
      {
         // restart the timer, periodic timer
         timerPtr_->startTime =TIMER_GET_TIME;
      }

      if( timerPtr_->callBackPtr !=NULL_PTR )
      {
         // this timer has a call back associated with it,
         // so we do not need to count timeouts because
         // we know the timeout is being serviced now
         timerPtr_->callBackPtr();

         if( !timerPtr_->periodic )
         {
            // one shot timer
            // because the callback is invoked rather than
            // check time, we need to disable the timer here
            timerPtr_->running =false;
         }
      }
      else
      {
         // no call back, so count timeouts, and flag as timed out
         timerPtr_->nbrTimeOuts ++;
         timerPtr_->timeOut =true;
      }
      return true;
   }

   return false;
}


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: OsCheckTimerTimeout
*|  Description:
*|----------------------------------------------------------------------------
*/
BOOL OsCheckTimerTimeout
(
   OS_TIMER *timerPtr_
)
{
   if( !timerPtr_->running )
      return false;

   if( (TIMER_GET_TIME - timerPtr_->startTime) > timerPtr_->timePeriod ||
       timerPtr_->timeOut
     )
   {
      timerPtr_->running =false;
      timerPtr_->timeOut =false;
      timerPtr_->nbrTimeOuts =0;
      return true;
   }
   else
      return false;
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: OsRestartTimer
*|  Description:
*|----------------------------------------------------------------------------
*/
void OsRestartTimer
(
   OS_TIMER *timerPtr_
)
{
   timerPtr_->startTime =TIMER_GET_TIME;
   timerPtr_->timeOut =false;
   timerPtr_->nbrTimeOuts =0;
   timerPtr_->running =true;
}


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: OsSimpleTimerInit
*|  Description:
*|----------------------------------------------------------------------------
*/
void OsSimpleTimerInit
(
   UINT32 *startTimePtr_
)
{
   *startTimePtr_ =TIMER_GET_TIME;
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: OsSimpleTimerTimeout
*|  Description:
*|----------------------------------------------------------------------------
*/
BOOL OsSimpleTimerTimeout
(
   UINT32 timeout_,
   UINT32 *startTimePtr_
)
{
   if( (TIMER_GET_TIME- *startTimePtr_) >timeout_ )
   {
      *startTimePtr_ =TIMER_GET_TIME;
      return true;
   }
   else
      return false;
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: OsStopTimer
*|  Description:
*|----------------------------------------------------------------------------
*/
void OsStopTimer
(
   OS_TIMER *timerPtr_
)
{
   timerPtr_->running =false;
   timerPtr_->timeOut =false;
   timerPtr_->nbrTimeOuts =0;
}


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: timer7_c_irq_handler
*|  Description:
*|----------------------------------------------------------------------------
*/
void timer7_c_irq_handler(void)
{
   int aicMaskReg;
   
   // get all enabled interrupts
   aicMaskReg =AT91C_BASE_AIC->AIC_IMR;

   MotorControl.speed =AT91C_BASE_TC7->TC_RA;    

   MotorControl.nbrVanes ++;
   MotorControl.edgeDetect =1;
   MotorControl.timeStamp =TicksLeft;
  
   if( (MotorControl.nbrVanes % MotorControl.vanesPerBall) ==0 )
   {
      BallCounter ++;            
   }     
   

#ifdef BALL_DROPPER      
   //   Pwm_SetDuty(0);
//   AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, PWM_SPEED_DRIVE );  
//   MOTOR_DIR_BRAKE;

#endif   
   TotalVaneCount ++;
   
   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC7->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x80);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x80);
   }
     
   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =aicMaskReg;
   
}


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: timer6_c_irq_handler
*|  Description:
*|----------------------------------------------------------------------------
*/
void timer6_c_irq_handler(void)
{
   int aicMaskReg;

   // get all enabled interrupts
   aicMaskReg =AT91C_BASE_AIC->AIC_IMR;

//   x =AT91C_BASE_TC6->TC_RA;    

   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC6->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x40);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x40);
   }

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =aicMaskReg;

}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: timer5_c_irq_handler
*|  Description:
*|----------------------------------------------------------------------------
*/
void timer5_c_irq_handler(void)
{
   int aicMaskReg;

   // get all enabled interrupts
   aicMaskReg =AT91C_BASE_AIC->AIC_IMR;

//   x =AT91C_BASE_TC5->TC_RA;    

   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC5->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x20);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x20);
   }

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =aicMaskReg;

}


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: timer4_c_irq_handler
*|  Description:
*|----------------------------------------------------------------------------
*/
void timer4_c_irq_handler(void)
{
   int aicMaskReg;

   // get all enabled interrupts
   aicMaskReg =AT91C_BASE_AIC->AIC_IMR;

   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC4->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x10);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x10);
   }

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =aicMaskReg;

}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: timer2_c_irq_handler
*|  Description:
*|----------------------------------------------------------------------------
*/
void timer2_c_irq_handler(void)
{
   int aicMaskReg;

   // get all enabled interrupts
   aicMaskReg =AT91C_BASE_AIC->AIC_IMR;

   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC2->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x04);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x04);
   }

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =aicMaskReg;

}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: timer1_c_irq_handler
*|  Description:
*|----------------------------------------------------------------------------
*/
void timer1_c_irq_handler(void)
{
   int aicMaskReg;

   // get all enabled interrupts
   aicMaskReg =AT91C_BASE_AIC->AIC_IMR;

   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC1->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x02);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x02);
   }

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =aicMaskReg;

} // end timer1_c_irq_handler()


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: timer0_c_irq_handler
*|  Description:
*|----------------------------------------------------------------------------
*/
void timer0_c_irq_handler(void)
{
//   AT91PS_TC TC_pt = AT91C_BASE_TC0;
   //* Acknowledge interrupt status
//   dummy = TC_pt->TC_SR;
   //* Suppress warning variable "dummy" was set but never used
//   dummy = dummy;

   int aicMaskReg;
   
   // get all enabled interrupts
   aicMaskReg =AT91C_BASE_AIC->AIC_IMR;

   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC0->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x01);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x01);
   }

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =aicMaskReg;
   
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: InputCapInit
*|  Description: enable input capture on one of the input cap pins
*|               set interrupt service routine
*|----------------------------------------------------------------------------
*/
void InputCapInit
(
   UINT8 inputPin_
)
{
   unsigned int dummy;

   switch( inputPin_ )
   {
      case TIOA0_IN:
      case TIOB0_IN:
         // First, enable the clock of the TIMER
         AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< AT91C_ID_TC0 ) ;

         // Disable the clock and the interrupts
         AT91C_BASE_TC0->TC_CCR = AT91C_TC_CLKDIS;
         AT91C_BASE_TC0->TC_IDR = 0xFFFFFFFF;

         // Clear status bit
         dummy = AT91C_BASE_TC0->TC_SR;

         // Suppress warning variable "dummy" was set but never used
         dummy = dummy;

         // Open Timer 0 interrupt
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC0, TIMER0_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, timer0_c_irq_handler);
         AT91C_BASE_TC0->TC_IER  = AT91C_TC_CPCS;  //  IRQ enable CPC
         AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);

         // Generate interrupt by software
         AT91F_AIC_Trig (AT91C_BASE_AIC,AT91C_ID_TC0);

         // enable timer clock, and swtrigger
         AT91C_BASE_TC0->TC_CCR =AT91C_TC_CLKEN;

         // Load RA on rising edge of TIOA0, Reset counter on RC compare,
         // TIOA0 as external trigger, rising edge trigger
         AT91C_BASE_TC0->TC_CMR =CAPTURE_CLOCK|AT91C_TC_ETRGEDG_RISING|AT91C_TC_ABETRG|AT91C_TC_LDRA_RISING|AT91C_TC_CPCTRG;

         // enable external trigger interrupt
         AT91C_BASE_TC0->TC_IER =AT91C_TC_ETRGS;
         break;
      case TIOA1_IN:
      case TIOB1_IN:
         // First, enable the clock of the TIMER
         AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< AT91C_ID_TC1 ) ;

         // Disable the clock and the interrupts
         AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
         AT91C_BASE_TC1->TC_IDR = 0xFFFFFFFF;

         // Clear status bit
         dummy = AT91C_BASE_TC1->TC_SR;

         // Suppress warning variable "dummy" was set but never used
         dummy = dummy;

         // Open Timer 1 interrupt
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC1, TIMER1_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, timer1_c_irq_handler);
         AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC1);

         // Generate interrupt by software
         AT91F_AIC_Trig (AT91C_BASE_AIC,AT91C_ID_TC1);

         // enable timer clock, and swtrigger
         AT91C_BASE_TC1->TC_CCR =AT91C_TC_CLKEN | AT91C_TC_SWTRG;

         if( inputPin_ ==TIOA1_IN )
         {
            // Load RA on rising edge of TIOA1, Reset counter on RC compare,
            // TIOA1 as external trigger(ABETRG), both edge trigger
            AT91C_BASE_TC1->TC_CMR =CAPTURE_CLOCK | AT91C_TC_ETRGEDG_RISING | AT91C_TC_ABETRG |
                                    AT91C_TC_LDRA_BOTH | AT91C_TC_CPCTRG;
         }
         else
         {
            // Load RB on rising edge of TIOB1, Reset counter on RC compare,
            // TIOB1 as external trigger, both edge trigger
            AT91C_BASE_TC1->TC_CMR =CAPTURE_CLOCK | AT91C_TC_ETRGEDG_RISING |
                                    AT91C_TC_LDRB_BOTH | AT91C_TC_CPCTRG;
         }

         // enable external trigger interrupt
         AT91C_BASE_TC1->TC_IER =AT91C_TC_ETRGS;
         break;
      case TIOA2_IN:
      case TIOB2_IN:
         // First, enable the clock of the TIMER
         AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< AT91C_ID_TC2 ) ;

         // Disable the clock and the interrupts
         AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKDIS;
         AT91C_BASE_TC2->TC_IDR = 0xFFFFFFFF;

         // Clear status bit
         dummy = AT91C_BASE_TC2->TC_SR;

         // Suppress warning variable "dummy" was set but never used
         dummy = dummy;

         // Open Timer 2 interrupt
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC2, TIMER2_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, timer2_c_irq_handler );
         AT91C_BASE_TC2->TC_IER = AT91C_TC_CPCS;  //  IRQ enable CPC
         AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC2);

         // Generate interrupt by software
         AT91F_AIC_Trig (AT91C_BASE_AIC,AT91C_ID_TC2);

         // enable timer clock, and swtrigger
         AT91C_BASE_TC2->TC_CCR =AT91C_TC_CLKEN;// | AT91C_TC_SWTRG;

         if( inputPin_ ==TIOA2_IN )
         {
#define RISING_EDGE_TRIGGER
#ifdef RISING_EDGE_TRIGGER
            // Load RA on rising edge of TIOA2, Reset counter on RC compare,
            // TIOA2 as external trigger(ABETRG), rising edge trigger
            AT91C_BASE_TC2->TC_CMR =CAPTURE_CLOCK | AT91C_TC_ETRGEDG_RISING | AT91C_TC_ABETRG |
                                                    AT91C_TC_LDRA_RISING | AT91C_TC_CPCTRG;
#else
 // edge trigger
            AT91C_BASE_TC2->TC_CMR =CAPTURE_CLOCK | AT91C_TC_ETRGEDG_BOTH | AT91C_TC_ABETRG |
                                    AT91C_TC_LDRA_BOTH | AT91C_TC_CPCTRG;
#endif
         }
         else
         {
            // Load RB on rising edge of TIOB1, Reset counter on RC compare,
            // TIOB1 as external trigger, both edge trigger
            AT91C_BASE_TC2->TC_CMR =CAPTURE_CLOCK | AT91C_TC_ETRGEDG_RISING |
                                    AT91C_TC_LDRB_BOTH | AT91C_TC_CPCTRG;
         }

         // enable external trigger interrupt
         AT91C_BASE_TC2->TC_IER =AT91C_TC_ETRGS;
         break;
      case TIOA4_IN:
      case TIOB4_IN:
         // First, enable the clock of the TIMER
         AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< AT91C_ID_TC4 ) ;

         // Disable the clock and the interrupts
         AT91C_BASE_TC4->TC_CCR = AT91C_TC_CLKDIS;
         AT91C_BASE_TC4->TC_IDR = 0xFFFFFFFF;

         // Clear status bit
         dummy = AT91C_BASE_TC4->TC_SR;

         // Suppress warning variable "dummy" was set but never used
         dummy = dummy;

         // Open Timer 1 interrupt
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC4, TIMER1_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, timer4_c_irq_handler);
         AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC4);

         // Generate interrupt by software
         AT91F_AIC_Trig (AT91C_BASE_AIC,AT91C_ID_TC4);

         // enable timer clock, and swtrigger
         AT91C_BASE_TC4->TC_CCR =AT91C_TC_CLKEN | AT91C_TC_SWTRG;

         if( inputPin_ ==TIOA4_IN )
         {
            // Load RA on rising edge of TIOA1, Reset counter on RC compare,
            // TIOA1 as external trigger(ABETRG), both edge trigger
            AT91C_BASE_TC4->TC_CMR =CAPTURE_CLOCK | AT91C_TC_ETRGEDG_RISING | AT91C_TC_ABETRG |
                                    AT91C_TC_LDRA_BOTH | AT91C_TC_CPCTRG;
         }
         else
         {
            // Load RB on rising edge of TIOB1, Reset counter on RC compare,
            // TIOB1 as external trigger, both edge trigger
            AT91C_BASE_TC4->TC_CMR =CAPTURE_CLOCK | AT91C_TC_ETRGEDG_RISING |
                                    AT91C_TC_LDRB_BOTH | AT91C_TC_CPCTRG;
         }

         // enable external trigger interrupt
         AT91C_BASE_TC4->TC_IER =AT91C_TC_ETRGS;
         break;
      case TIOA5_IN:
      case TIOB5_IN:
         // First, enable the clock of the TIMER
         AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< AT91C_ID_TC5 ) ;

         // Disable the clock and the interrupts
         AT91C_BASE_TC5->TC_CCR = AT91C_TC_CLKDIS;
         AT91C_BASE_TC5->TC_IDR = 0xFFFFFFFF;

         // Clear status bit
         dummy = AT91C_BASE_TC5->TC_SR;

         // Suppress warning variable "dummy" was set but never used
         dummy = dummy;

         // Open Timer 1 interrupt
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC5, TIMER1_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, timer5_c_irq_handler);
         AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC5);

         // Generate interrupt by software
         AT91F_AIC_Trig (AT91C_BASE_AIC,AT91C_ID_TC5);

         // enable timer clock, and swtrigger
         AT91C_BASE_TC5->TC_CCR =AT91C_TC_CLKEN | AT91C_TC_SWTRG;

         // Load RA on rising edge of TIOA5, Reset counter on RC compare,
         // TIOA5 as external trigger, rising edge trigger         
         AT91C_BASE_TC5->TC_CMR =CAPTURE_CLOCK|AT91C_TC_ETRGEDG_RISING|AT91C_TC_ABETRG|AT91C_TC_LDRA_RISING|AT91C_TC_CPCTRG;                  

         // enable external trigger interrupt
         AT91C_BASE_TC5->TC_IER =AT91C_TC_ETRGS;
         break;
      case TIOA6_IN:
      case TIOB6_IN:
         // First, enable the clock of the TIMER
         AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< AT91C_ID_TC6 ) ;

         // Disable the clock and the interrupts
         AT91C_BASE_TC6->TC_CCR = AT91C_TC_CLKDIS;
         AT91C_BASE_TC6->TC_IDR = 0xFFFFFFFF;

         // Clear status bit
         dummy = AT91C_BASE_TC6->TC_SR;

         // Suppress warning variable "dummy" was set but never used
         dummy = dummy;

         // Open Timer 1 interrupt
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC6, TIMER1_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, timer6_c_irq_handler);
         AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC6);

         // Generate interrupt by software
         AT91F_AIC_Trig (AT91C_BASE_AIC,AT91C_ID_TC6);

         // enable timer clock, and swtrigger
         AT91C_BASE_TC6->TC_CCR =AT91C_TC_CLKEN | AT91C_TC_SWTRG;

         // Load RA on rising edge of TIOA6, Reset counter on RC compare,
         // TIOA6 as external trigger, rising edge trigger         
         AT91C_BASE_TC6->TC_CMR =CAPTURE_CLOCK|AT91C_TC_ETRGEDG_RISING|AT91C_TC_ABETRG|AT91C_TC_LDRA_RISING|AT91C_TC_CPCTRG;         
         
         // enable external trigger interrupt
         AT91C_BASE_TC6->TC_IER =AT91C_TC_ETRGS;
         break;
      case TIOA7_IN:
      case TIOB7_IN:
         // First, enable the clock of the TIMER
         AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< AT91C_ID_TC7 ) ;

         // Disable the clock and the interrupts
         AT91C_BASE_TC7->TC_CCR = AT91C_TC_CLKDIS;
         AT91C_BASE_TC7->TC_IDR = 0xFFFFFFFF;

         // Clear status bit
         dummy = AT91C_BASE_TC7->TC_SR;

         // Suppress warning variable "dummy" was set but never used
         dummy = dummy;

         // Open Timer 1 interrupt
         AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_TC7, TIMER1_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, timer7_c_irq_handler);
         AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC7);

         // Generate interrupt by software
         AT91F_AIC_Trig (AT91C_BASE_AIC,AT91C_ID_TC7);

         // enable timer clock, and swtrigger
         AT91C_BASE_TC7->TC_CCR =AT91C_TC_CLKEN | AT91C_TC_SWTRG;

         // Load RA on rising edge of TIOA7, Reset counter on RC compare,
         // TIOA7 as external trigger, rising edge trigger
         AT91C_BASE_TC7->TC_CMR =CAPTURE_CLOCK|AT91C_TC_ETRGEDG_FALLING|AT91C_TC_ABETRG|AT91C_TC_LDRA_FALLING|AT91C_TC_CPCTRG;
         
         // enable external trigger interrupt
         AT91C_BASE_TC7->TC_IER =AT91C_TC_ETRGS;
         break;
   }
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: TimerInit
*|  Description:
*|----------------------------------------------------------------------------
*/
void TimerInit(void)
{
   TicksLeft =0;
   SecondsCount =0;

   SpiAvailable =false;

#define ENABLE_PIT_INTERRUPT
#ifdef ENABLE_PIT_INTERRUPT
/********** PIT ****************/
   // enable PIT interrupt
 #define DEBUG_TIMER_ISR
 #ifdef DEBUG_TIMER_ISR
   AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_SYS, SOFT_INTERRUPT_LEVEL,(0<<5), SysTimer);
 #else
   AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_SYS, SOFT_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_EDGE_TRIGGERED, SysTimer);
 #endif

   AT91C_BASE_PITC->PITC_PIMR =(0x03000000 | ONE_MSEC_TICK_COUNT);  // (enable PIT interrupt)

   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

#else
   // (do not enable PIT interrupt, enable PIT timer )
   AT91C_BASE_PITC->PITC_PIMR =(0x01000000 | ONE_MSEC_TICK_COUNT);
#endif


/********** RTC ****************/
//AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_SYS, SOFT_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_EXT_POSITIVE_EDGE, RTC_c_irq_handler);
//AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);

//   AT91C_BASE_RTTC->RTTC_RTMR =0x0060000 | ONE_SECOND_TICK_COUNT;  // (enable RTT interrupt)

   // (do not enable RTT interrupt, reload and restart clock divider)
   AT91C_BASE_RTTC->RTTC_RTMR =0x0040000 | ONE_SECOND_TICK_COUNT;

   AT91C_BASE_AIC->AIC_SPU  = (int) AT91F_SpuriousInt;
} // end TimerInit()


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: DisableSysTimer
*|  Description:
*|----------------------------------------------------------------------------
*/
void DisableSysTimer
(
   void
)
{
   // disable the PIT timer
   AT91C_BASE_PITC->PITC_PIMR =0x00;

   // disable the RTT timer
   AT91C_BASE_RTTC->RTTC_RTMR =0x00;
}


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: DisableInputCaptureIsr
*|  Description:
*|----------------------------------------------------------------------------
*/
void DisableInputCaptureIsr
(
   void
)
{
   // Disable the clock and the interrupts
   AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
   AT91C_BASE_TC1->TC_IDR = 0xFFFFFFFF;

   // Disable the clock and the interrupts
   AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKDIS;
   AT91C_BASE_TC2->TC_IDR = 0xFFFFFFFF;
}

/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: AT91F_SpuriousInt
*|  Description: Trap, and service spurious interrupts
*|----------------------------------------------------------------------------
*/
void AT91F_SpuriousInt(void)
{
//   SendByte('S');

   AT91C_BASE_AIC->AIC_EOICR =0xff;
}



/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: InitWtd
*|  Description: Initialize the WatchDog timer, this is a write once register
*|  Time base is 12 bit value, where 0xfff 16 seconds
*|----------------------------------------------------------------------------
*/
void InitWdt
(
   BOOL enable_   // enable/disable WDT
)
{
   if( enable_ )
      AT91C_BASE_WDTC->WDTC_WDMR =0x30000000 | (WDT_WDD_KICK_WINDOW<<16) | 0x3000 | WDT_WDV_TIMER_BASE;
   else
      AT91C_BASE_WDTC->WDTC_WDMR= AT91C_WDTC_WDDIS;
}


/*
*|----------------------------------------------------------------------------
*|  Module: Timer
*|  Routine: KickWdt
*|  Description: Service the watch dog timer
*|----------------------------------------------------------------------------
*/
void KickWdt
(
   void
)
{
   // write KEY password (0xa5), restart the wdt
   AT91C_BASE_WDTC->WDTC_WDCR =0xa5000001;
}


void StartInputCapture1Isr
(
   void
)
{
   // get all enabled interrupts
   AicMaskReg =AT91C_BASE_AIC->AIC_IMR;
}



void EndInputCapture1Isr
(
   void
)
{
   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC4->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x10);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x10);
   }

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =AicMaskReg;
}


void StartInputCapture2Isr
(
   void
)
{
   // get all enabled interrupts
   AicMaskReg =AT91C_BASE_AIC->AIC_IMR;
}



void EndInputCapture2Isr
(
   void
)
{
   // check overflow flag, reading this reg also acknowledges int
   if( AT91C_BASE_TC0->TC_SR &0x01 ==0x01)
   {
      // counter overflow, set flag
      SET(CounterOverFlowMask, 0x01);
   }
   else
   {
      // no overflow unset flag
      UNSET(CounterOverFlowMask, 0x01);
   }

   // reenable all interrupts
   AT91C_BASE_AIC->AIC_IECR =AicMaskReg;
}



