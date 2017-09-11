/*|***************************************************************************/
/*|PROJECT:  OnStream Pipeline Inspection Gage                               */
/*|Module:   cpuPorts                                                        */
/*|Description:                                                              */
/*| CPU ports are assigned to functions as needed                            */
/*|***************************************************************************/

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
#ifndef CPU_PORTS_H
 #define CPU_PORTS_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "AT91SAM7A3.h"
#include "board.h"			// Hardware specific header
#include "mk_Std.h"

/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
****************************************************************************
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

// AT91SAM7S64 Pin description, and definition
#define PA0            (1<<0)	/* PA0 / PGMEN0 & PWM0 TIOA0  48 */
#define PA1            (1<<1)	/* PA1 / PGMEN1 & PWM1 TIOB0  47 */
#define PA2            (1<<2)	/* PA2          & PWM2 SCK0   44 */
#define PA3            (1<<3)	/* PA3          & TWD  NPCS3  43 */

#define PA4            (1<<4)	/* PA4  */
#define PA5            (1<<5)   /* PA5  */
#define PA6            (1<<6)	/* PA6  */
#define PA7            (1<<7)	/* PA7  */

#define PA8            (1<<8)   /* PA8/PGMM0   CTS0   ADTRG Pin31 */
#define PA9            (1<<9)	
#define PA10           (1<<10)	
#define PA11           (1<<11)	/* PA11/PGMM3  NPCS0 PWM0   Pin28 */

#define PA12           (1<<12)	/* PA12/PGMD0  MISO  PWM1   Pin27 */
#define PA13           (1<<13)	/* PA13/PGMD1  MOSI  PWM2   Pin22 */
#define PA14           (1<<14)	/* PA14/PGMD2  SPCK  PWM3   Pin21 */
#define PA15           (1<<15)  /* PA15/PGMD3  TF  TIOA1    Pin20 */

#define PA16           (1<<16)  /* PA16/PGMD4  TK  TIOB1    Pin19 */	
#define PA17           (1<<17)	
#define PA18           (1<<18)	
#define PA19           (1<<19)	

#define PA20           (1<<20)	
#define PA21           (1<<21)	
#define PA22           (1<<22)	
#define PA23           (1<<23)	/* PA23/PGMD11  SCK1  PWM0   Pin15 */

#define PA24           (1<<24)	
#define PA25           (1<<25)	
#define PA26           (1<<26)  /* PA26/PGMD14  DCD1  TIOA2  Pin26 */	
#define PA27           (1<<27)	

#define PA28           (1<<28)	
#define PA29           (1<<29)	
#define PA30           (1<<30)  /* PA30        IRQ1   NPCS2  Pin42 */
#define PA31           (1<<31)	


#define PB0  (PA0)
#define PB1  (PA1)
#define PB2  (PA2)
#define PB3  (PA3)
#define PB4  (PA4)
#define PB5  (PA5)
#define PB6  (PA6)
#define PB7  (PA7)

#define PB8  (PA8)
#define PB9  (PA9)
#define PB10 (PA10)
#define PB11 (PA11)
#define PB12 (PA12)
#define PB13 (PA13)
#define PB14 (PA14)
#define PB15 (PA15)

#define PB16 (PA16)
#define PB17 (PA17)
#define PB18 (PA18)
#define PB19 (PA19)
#define PB20 (PA20)
#define PB21 (PA21)
#define PB22 (PA22)
#define PB23 (PA23)

#define PB24 (PA24)
#define PB25 (PA25)
#define PB26 (PA26)
#define PB27 (PA27)
#define PB28 (PA28)
#define PB29 (PA29)
#define PB30 (PA30)
#define PB31 (PA31)


/******* PWM ********/
// heater control
#define PWM_SPEED_DRIVE   (PA21)

#define PIEZO_CONTROL     (PA0)

#define MOTOR_DIRECTION1  (PB24)
#define MOTOR_DIRECTION2  (PB25)

#define MOTOR_SENSOR      (PB26)

#define POWER_SWITCH      (PB27)

/******* SCI *********/

/******* SPI *********/
// spi data/clock/cs
#define SPI0_SPCK (PA17)
#define SPI0_MOSI (PA16)
#define SPI0_MISO (PA15)
#define SPI0_SS0  (PA11)
#define SPI0_SS1  (PA12)


/******* LCD *********/
// parallel I/O (lcd)
#define IO_D0 (PB0)
#define IO_D1 (PB1)
#define IO_D2 (PB2)
#define IO_D3 (PB3)
#define IO_D4 (PB4)
#define IO_D5 (PB5)
#define IO_D6 (PB6)
#define IO_D7 (PB7)

#define IO_BYTE  IO_D7|IO_D6|IO_D5|IO_D4|IO_D3|IO_D2|IO_D1|IO_D0


#define MOTOR_DIR_FWD   AT91F_PIO_SetOutput(AT91C_BASE_PIOB, MOTOR_DIRECTION1); AT91F_PIO_SetOutput(AT91C_BASE_PIOB, MOTOR_DIRECTION2)
#define MOTOR_DIR_REV   AT91F_PIO_SetOutput(AT91C_BASE_PIOB, MOTOR_DIRECTION1); AT91F_PIO_ClearOutput(AT91C_BASE_PIOB, MOTOR_DIRECTION2)
#define MOTOR_DIR_BRAKE AT91F_PIO_ClearOutput(AT91C_BASE_PIOB, MOTOR_DIRECTION1); AT91F_PIO_ClearOutput(AT91C_BASE_PIOB, MOTOR_DIRECTION2)

#define BUZZER_ON      AT91F_PIO_SetOutput(AT91C_BASE_PIOA, PIEZO_CONTROL)
#define BUZZER_OFF     AT91F_PIO_ClearOutput(AT91C_BASE_PIOA, PIEZO_CONTROL)

// parallel control lines
#define IO_BS1   (PB14)
#define IO_BS2   (PB13)
#define IO_CS    (PB12)
#define IO_RST   (PB11)
#define IO_RD    (PB10)
#define IO_WR    (PB9)
#define IO_DC    (PB8)

#define IO_CONTROL  IO_RST |IO_BS1 |IO_BS2 | IO_CS | IO_RD |IO_WR | IO_DC

/******* KEYPAD *********/
#define SWE (PB19)
#define SWD (PB18)
#define SWC (PB17)
#define SWB (PB16)
#define SWA (PB15)

//#define KEYPAD_MASK (SWE | SWD | SWC | SWB | SWA )
#define KEYPAD_MASK (SWD | SWC | SWB | SWA )

// radio power on/off control

// regulator on/off control

// flash write protect pin

// flash write protect pin

// usb detect pin
#define USB_DETECT (PA4)

// backlight control pin

/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/
/*
*|----------------------------------------------------------------------------
*|  Module: cpuPorts
*|  Routine: InitCpuPorts
*|  Description:
*|    Initialise the CPU ports used as general purpose I/O
*|----------------------------------------------------------------------------
*/

void InitCpuPorts
(                       /*|                                                 */
   void
);                      /*|                                                 */

void BuzzerOff(void);
void BuzzerToggle(void);
#endif

/* End cpuPorts.h */




