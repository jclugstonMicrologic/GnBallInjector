/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*|Module:   pwmHi                                                           */
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


/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "cpuPorts.h"

#include "pwmHi.h"

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/
/*    PWM Clock source Domain */
#define         MCKtoPWM      0
#define         MCK_2toPWM    1
#define         MCK_4toPWM    2
#define         MCK_8toPWM    3
#define         MCK_16toPWM   4
#define         MCK_32toPWM   5
#define         MCK_64toPWM   6
#define         MCK_128toPWM  7
#define         MCK_256toPWM  8
#define         MCK_512toPWM  9
#define         MCK_1024toPWM 10
#define         CLHAtoPWM     11
#define         CLHBtoPWM     12

#define         CALG_OFF       0
#define         CALG_ON       1<<8  // The period is center aligned (OFF, left aligned)

#define         CPOL_OFF      0<<0  // The ouput waveform starts at a high level (OFF, low level)
#define         CPOL_ON       1<<9  // The ouput waveform starts at a high level (OFF, low level)

#define         CPD_ON        1<<10 // The period will be modified at the next period start event
#define         CPD_OFF       0<<10

/* Select the wanted frequency in hertz.
This choice has to be in accordance with the selected clock source */
#define   FREQUENCY    4792  // In Hertz

/* In our example, we are targetting to get out 100 Hertz frequency. In this goal, we use MCK_256toPWM */
#define   PWM_PERIOD_VALUE            (MCK/(1*FREQUENCY))

/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/

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

void pwmInit
(
   void
)
{
   // First, enable the clock of the PWM
   AT91F_PMC_EnablePeriphClock( AT91C_BASE_PMC, 1<< AT91C_ID_PWMC );

   AT91C_BASE_PWMC->PWMC_MR =0x08010801;

   // disable pwm channel
   AT91C_BASE_PWMC->PWMC_DIS =0x000000ff;
 
   // disable pwm interrupts
   AT91C_BASE_PWMC->PWMC_IDR =0x0000000f;

} // end pwmInit()


void pwmInitCh
(
   UINT8 ch_
)
{
   // enable pwm channels
   AT91C_BASE_PWMC->PWMC_ENA |=(0x00000001 <<ch_);

   // enable output for pwm channels
   AT91C_BASE_PWMC->PWMC_SR |=(0x00000001 <<ch_);
   
   AT91C_BASE_PWMC->PWMC_CH[ch_].PWMC_CMR =0x00000200;  

   // set the duty cycle
   AT91C_BASE_PWMC->PWMC_CH[ch_].PWMC_CDTYR =48000; //0x000013c0;

   // set the period
   // period =(48000 * 1)/48000000
   AT91C_BASE_PWMC->PWMC_CH[ch_].PWMC_CPRDR =9600000;

   // channel update register (change duty cycle)
   AT91C_BASE_PWMC->PWMC_CH[ch_].PWMC_CUPDR =0;   

} // end pwmInitCh()


void adjustPwmFreq
(
   UINT8 ch_,
   UINT8 freq_
)
{
   // set the period
   // period =(48000 * 1)/48000000
   AT91C_BASE_PWMC->PWMC_CH[ch_].PWMC_CPRDR =48000000/freq_;
}

void adjustPwmDuty
(
   UINT8 ch_,
   UINT32 duty_
)
{
   duty_ =(UINT32)(duty_*AT91C_BASE_PWMC->PWMC_CH[ch_].PWMC_CPRDR)/100;       
   // Write 0 in CPD Field will allow to update duty cycle register by copying the content
   // of the up-date register
   AT91C_BASE_PWMC->PWMC_CH[ch_].PWMC_CMR &= ~(CPD_ON);
   // set the duty cycle
   AT91C_BASE_PWMC->PWMC_CH[ch_].PWMC_CUPDR =duty_;

   // It is forbidden to continue before the complete PWM period
   while( (AT91C_BASE_PWMC->PWMC_ISR & (0x01<<ch_) ) == (0x01<<ch_)) {}   
}




