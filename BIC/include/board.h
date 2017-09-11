/*----------------------------------------------------------------------------
*         ATMEL Microcontroller Software Support  -  ROUSSET  -
*----------------------------------------------------------------------------
* The software is delivered "AS IS" without warranty or condition of any
* kind, either express, implied or statutory. This includes without
* limitation any warranty or condition with respect to merchantability or
* fitness for any particular purpose, or against the infringements of
* intellectual property rights of others.
*----------------------------------------------------------------------------
* File Name           : Board.h
* Object              : AT91SAMA3 Evaluation Board Features Definition File.
*
* Creation            : JPP   08/Mar/2005
*  1.1 14/Oct/05 JPP  : Change MCK
*----------------------------------------------------------------------------
*/
#ifndef Board_h
#define Board_h

#include "AT91SAM7A3.h"
#define __inline inline
#include "lib_AT91SAM7A3.h"

//#define AT91SAM7A3_DK

#define true	-1
#define false	0

/*-------------------------------*/
/* SAM7Board Memories Definition */
/*-------------------------------*/
// The AT91SAM7A3 embeds a 32-Kbyte SRAM bank, and 256 K-Byte Flash

#define  INT_SARM           0x00200000
#define  INT_SARM_REMAP	    0x00000000

#define  INT_FLASH          0x00000000
#define  INT_FLASH_REMAP    0x01000000

#define  FLASH_PAGE_NB		1024
#define  FLASH_PAGE_LOCK    64
#define  FLASH_PAGE_SIZE	256

/*-----------------*/
/* Leds Definition */
/*-----------------*/
/*                                 PIO             PIN */

#define NB_LEB			4


/*-------------------------*/
/* Push Buttons Definition */
/*-------------------------*/
#define SW1_MASK        (1<<8)	/*BP 5  PB8  / TF1  & FIQ     9 */
#define SW2_MASK        (1<<12)	/*BP 6  PB12 / TD1  & TIOA0   5 */
#define SW3_MASK        (1<<13)	/*BP 7  PB13 / RD1  & TIOB0   4 */
#define SW4_MASK        (1<<9)	/*BP 8  PB9  / TK1  & TCLK0   8 */

#ifdef AT91SAM7A3_DK

// AT91SAM7A3-DK Development Kit
#define SW5_MASK  (1<<5)      /*BP 9  PB5  / PCK2 & RD0     12 */
#define BP_5     (1<<8)	      // PB8
#define BP_6     (1<<12)      // PB12
#define BP_7 	 (1<<13)      // PB13
#define BP_8 	 (1<<9)	      // PB9
#define BP_9 	 (1<<5)	      // PB5
#else

// AT91SAM7A3-EK evaluation Kit
#define SW5_MASK        (1<<14)	/*PUSH  PB14 / AD00& PWM0 &TIOA1  80 */
#endif

#define SW1_UP  	SW1_MASK	// PB8
#define SW2_LEFT 	SW2_MASK	// PB12
#define SW3_RIGHT 	SW3_MASK	// PB13
#define SW4_DOWN 	SW4_MASK	// PB9
#define SW5_PUSH 	SW5_MASK	// PB5 // or PB 14

#define SW_MASK         (SW1_MASK|SW2_MASK|SW3_MASK|SW4_MASK|SW5_MASK)

/*------------------*/
/* USART Definition */
/*------------------*/
/* SUB-D 9 points DBGU*/
#define DBGU_RXD		AT91C_PA30_DRXD
#define DBGU_TXD		AT91C_PA31_DTXD
#define AT91C_DBGU_BAUD	   115200   // Baud rate


/*--------------*/
/* Master Clock */
/*--------------*/

#define EXT_OC          18432000   // Exetrnal ocilator MAINCK
#define MCK             48054857   // MCK (PLLRC div by 2)
#define MCKKHz          (MCK/1000) //

#endif /* Board_h */
