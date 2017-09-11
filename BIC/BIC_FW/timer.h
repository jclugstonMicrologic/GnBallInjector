
/*****************************************************************************
 *
 * Timer
 *
 *****************************************************************************
*/
#include "mk_Std.h"

#define	MCK_FOSC		48000000 // Frequency of the processor clock
#define	TIMER_PRESCALE		1024	 // The desired prescale


#if TIMER_PRESCALE ==2
 #define CAPTURE_CLOCK         0x0 //TIMER_CLOCK1 =48000000/2    =24000000
#elif TIMER_PRESCALE ==8
 #define CAPTURE_CLOCK         0x1 //TIMER_CLOCK2 =48000000/8    =6000000
#elif TIMER_PRESCALE ==32
 #define CAPTURE_CLOCK         0x2 //TIMER_CLOCK3 =48000000/32   =1500000
#elif TIMER_PRESCALE ==128
 #define CAPTURE_CLOCK         0x3 //TIMER_CLOCK4 =48000000/128  =375000
#else
 #define CAPTURE_CLOCK         0x4 //TIMER_CLOCK5 =48000000/1024 =46875
#endif

// internal clock selection
#define TIMER_CLOCK (MCK_FOSC/TIMER_PRESCALE)

enum
{
   TIOA0_IN =0,
   TIOA1_IN,
   TIOA2_IN,
   TIOA3_IN,
   TIOA4_IN,
   TIOA5_IN,
   TIOA6_IN,
   TIOA7_IN,

   TIOB0_IN,
   TIOB1_IN,
   TIOB2_IN,
   TIOB3_IN,
   TIOB4_IN,
   TIOB5_IN,
   TIOB6_IN,
   TIOB7_IN

};


typedef struct
{
   // offset value, to be added/subtract from GMT hours
   // this offset value is set by the PC
   UINT8 timeZone;

   UINT8 tenthSec;

   UINT8 second;
   UINT8 minute;
   UINT8 hour;

   UINT8 date;
   UINT8 month;
   UINT8 year;

}SYSTEM_TIME;

typedef struct
{
   UINT32 timer1;
   UINT32 timer2;
}TIMER;

enum
{
   TIMER_KEYPAD_BUZZER =0,
   TIMER_JAM_BUZZER, 

   TIMER_LAST
};

typedef struct
{
   UINT32 startTime;
   UINT32 timePeriod;

   BOOL running;
   BOOL periodic;

   BOOL timeOut;

   UINT8 nbrTimeOuts;  // number of timeouts without being checked

   void (*callBackPtr)();

}OS_TIMER;

extern OS_TIMER OsTimer[TIMER_LAST];

extern SYSTEM_TIME SystemTime;

extern volatile UINT32 TicksLeft;
extern volatile UINT32 SecondsCount;

extern UINT8 CounterOverFlowMask;

extern volatile BOOL SpiAvailable;

extern UINT16 TotalVaneCount;
/*********************************************************************
 * Function:        void TimerInit(void)
 *
 * PreCondition:    none
 *
 * Input:       	none	
 *
 * Output:      	none
 *
 * Side Effects:    None
 *
 * Overview:        Initializes Timer0 for use.
 *
 * Note:            None
 ********************************************************************/
void TimerInit(void);
void DisableSysTimer(void);
void DisableInputCaptureIsr( void );
void InputCapInit( UINT8 inputPin_ );

/*
*|----------------------------------------------------------------------------
*|  Module: tgSystmr
*|  Routine: SysTimer
*|  Description:
*|----------------------------------------------------------------------------
*/
void SysTimer( void );

void OsStartPeriodicTimer( OS_TIMER *timerPtr_ , UINT32 timeOut_ );
void OsStartOneShotTimer( OS_TIMER *timerPtr_, UINT32 timeOut_ );

BOOL OsTimerTimeout( OS_TIMER *timerPtr_ );
BOOL OsCheckTimerTimeout(OS_TIMER *timerPtr_);
void OsRestartTimer( OS_TIMER *timerPtr_ );
void OsStopTimer( OS_TIMER *timerPtr_ );
void OsSimpleTimerInit( UINT32 *startTimePtr_ );
BOOL OsSimpleTimerTimeout( UINT32 timeout_, UINT32 *startTimePtr_ );

void InitWdt( BOOL );
void KickWdt( void );

void StartInputCapture1Isr( void );
void EndInputCapture1Isr( void );

void StartInputCapture2Isr( void );
void EndInputCapture2Isr( void );


