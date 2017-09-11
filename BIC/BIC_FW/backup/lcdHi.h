/*|***************************************************************************/
/*|PROJECT:  PROJECT:  MarkeTel Global Dialer                                */
/*|Module:   Application:                                                    */
/*|Description:                                                              */
/*|   The header file                                                        */
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
#ifndef LCDHI_H
 #define LCDHI_H

/*************************************************************************/
/* "I N C L U D E   F I L E S                                            */
/*************************************************************************/
#include "cpuports.h"


/*****************************************************************************/
/* P U B L I C   D E F I N I T I O N S                                       */
/*****************************************************************************/


/*****************************************************************************/
/* P R I V A T E   D E F I N I T I O N S                                     */
/*****************************************************************************/
enum
{
   RIGHT_ARROW =0,
   LEFT_ARROW,
   UP_ARROW,
   DOWN_ARROW,

   LEFT_RIGHT_ARROW,
   UP_DOWN_ARROW,

   ENTER_KEY_ARROW,

   DEGREE_CELSIUS_SYMBOL,

   RIGHT_ARROW_INVERT
};


/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/
// defines the top left origin
// currently TOP_LEFT_B starts closest connector CN1
// TOP_LEFT_A would require the LCD to be rotated 180 degrees
#define TOP_LEFT_B

#define ASSERT_LCD_CS1   AT91C_BASE_PIOB->PIO_CODR =IO_BS1
#define NEGATE_LCD_CS1   AT91C_BASE_PIOB->PIO_SODR =IO_BS1

#define ASSERT_LCD_CS2   AT91C_BASE_PIOB->PIO_CODR =IO_BS2
#define NEGATE_LCD_CS2   AT91C_BASE_PIOB->PIO_SODR =IO_BS2

#define CLEAR_RD         AT91C_BASE_PIOB->PIO_CODR =IO_RD;
#define SET_RD           AT91C_BASE_PIOB->PIO_SODR =IO_RD;

#define CLEAR_WR         AT91C_BASE_PIOB->PIO_CODR =IO_WR;
#define SET_WR           AT91C_BASE_PIOB->PIO_SODR =IO_WR;

#define CLEAR_DC         AT91C_BASE_PIOB->PIO_CODR =IO_DC;
#define SET_DC           AT91C_BASE_PIOB->PIO_SODR =IO_DC;

#define CLEAR_RESET      AT91C_BASE_PIOB->PIO_CODR =IO_RST;
#define SET_RESET        AT91C_BASE_PIOB->PIO_SODR =IO_RST;

#define ASSERT_LCD_CS    AT91C_BASE_PIOB->PIO_CODR =IO_CS
#define NEGATE_LCD_CS    AT91C_BASE_PIOB->PIO_SODR =IO_CS

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/


/*
*****************************************************************************
 P R I V A T E  F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/

void lcdInit
(
   void
);

void displayChar
(
   UINT8 char_,
   UINT8 row_,
   UINT8 col_,
   BOOL invert_
);


void setLcdRow
(
   UINT8 row_,
   UINT8 col_
);

void displaySpecialChar
(
   UINT8 char_,
   UINT8 rowPos_,
   UINT8 rightPos_
);

void clearLcd
(
   void
);


void writeCommand( UINT8 );
//void writeParameter( UINT8 );
void writeData( UINT8 );

#endif








