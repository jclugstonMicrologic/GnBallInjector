/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*|Module:  CpuPorts                                                         */
/*|Description:                                                              */
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


/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/

#include "timer.h"
#include "cpuPorts.h"

#include "dbgu.h"
/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/


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



/*
*|----------------------------------------------------------------------------
*|  Module: CpuPorts
*|  Routine: InitCpuPorts
*|  Description:
*|   Initialise the cpu ports used for
*|    general input/output
*|    peripheral A or B designation
*|
*|   The ports are initialised as follows:
*|
*|----------------------------------------------------------------------------
*/
void InitCpuPorts(void)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */

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
   // enable peripheral control of pin
   AT91C_BASE_PIOB->PIO_PDR =MOTOR_SENSOR;

   // enable peripheral control of pin
   AT91C_BASE_PIOA->PIO_PDR =//PWM_SPEED_DRIVE      |
                             SPI0_SPCK |
                             SPI0_MOSI |
                             SPI0_MISO |
                             SPI0_SS0  |
                             SPI0_SS1;

	
   // disable output on I/O line
   AT91C_BASE_PIOA->PIO_ODR =SPI0_SPCK |
                             SPI0_MOSI |
                             SPI0_MISO |
                             SPI0_SS0;


   // assign I/O line to peripheral A
   AT91C_BASE_PIOA->PIO_ASR =//PWM_SPEED_DRIVE |
                             SPI0_SPCK |
                             SPI0_MOSI |
                             SPI0_MISO |
                             SPI0_SS0;


   // assign I/O line to peripheral B
   AT91C_BASE_PIOB->PIO_ASR =MOTOR_SENSOR;

   // enable input glitch filter
   AT91C_BASE_PIOB->PIO_IFER =MOTOR_SENSOR;


   // disable all interrupts
   AT91C_BASE_PIOA->PIO_IDR =0xffffffff;


   // to be outputs. No need to set these pins to be driven by the PIO because it is GPIO pins only.
   AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, PIEZO_CONTROL | PWM_SPEED_DRIVE );

   AT91F_PIO_CfgOutput( AT91C_BASE_PIOB, IO_BYTE |                                         
                                         SWE |
                                         IO_CONTROL |
                                         MOTOR_DIRECTION1 |
                                         MOTOR_DIRECTION2 |
                                         POWER_SWITCH
                      );
   
   AT91F_PIO_CfgInput( AT91C_BASE_PIOB,KEYPAD_MASK); 

   // turn off buzzer, and ground SWE to make keypad work
   AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, PIEZO_CONTROL | SWE );
   
   // Clear the parallel IO data port
   AT91F_PIO_ClearOutput( AT91C_BASE_PIOB, IO_BYTE);

   // set power control
   AT91F_PIO_SetOutput( AT91C_BASE_PIOB, POWER_SWITCH );

   // disable pull up on these pins
   AT91C_BASE_PIOA->PIO_PPUDR |=USB_DETECT;

   // enable input change interrupt
   AT91C_BASE_PIOA->PIO_IER =USB_DETECT;

   // enable pull up
   AT91C_BASE_PIOB->PIO_PPUER =(IO_BYTE | IO_CONTROL);

   // set the parallel control bits accordingly
   AT91C_BASE_PIOB->PIO_CODR =IO_RST;

   // set LCD mode
   AT91C_BASE_PIOB->PIO_SODR =IO_BS1;
   AT91C_BASE_PIOB->PIO_SODR =IO_BS2;

   AT91C_BASE_PIOB->PIO_SODR =IO_RD;
   AT91C_BASE_PIOB->PIO_SODR =IO_WR;

   // enable the clock of the PIOA
   AT91F_PMC_EnablePeriphClock( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA ) ;

   // enable the clock of the PIOB
   AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOB ) ;

     
} /* InitCpuPorts() */


void BuzzerOff(void)
{
   BUZZER_OFF;  
}


void BuzzerToggle(void)
{
   if( (AT91C_BASE_PIOA->PIO_PDSR &PIEZO_CONTROL) ==PIEZO_CONTROL ) 
      BUZZER_OFF;  
   else
      BUZZER_ON;       
}


/* end cpuPorts.c  */








