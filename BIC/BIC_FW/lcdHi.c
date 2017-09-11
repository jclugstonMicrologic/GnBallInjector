/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*|Module:   lcdHi                                                           */
/*|Description: hardware interface for the OLED parallel lcd module          */
/*|                                                                          */
/*|***************************************************************************/

/*
Tools:
   Compiler: IAR Kickstart development environment for AT91SAM7Sxxx
   ***************************************************************************
   Copyright (C) 2012, Micrologic Limited.
   Any information contained in this document which was hitherto unknown by
   the recipient or in which there subsists a proprietary right must not be
   disclosed to any other person, nor used for manufacturing purposes,
   without the permission of Micrologic Limited and the acceptance of this
   document will be construed as acceptance of the foregoing.
   ***************************************************************************
*/

/*
************************************************************************
  "I N C L U D E   F I L E S
************************************************************************
*/

#include "timer.h"
#include "lcdHi.h"


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
 P R I V A T E   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/
void clockByte( UINT8 );
void resetLcd( void );
void programDelay( UINT16 );
void swapBits( UINT8 *bytePtr_ );

/*|------------------------------------------------------------------------
  |  Module: lcdHi.c
  |  Routine: InitLcd
  |  Description: initializes the lcd
  |------------------------------------------------------------------------
*/
void lcdInit
(
   void
)
{
   /*
   **********************
   L O C A L   D A T A
   **********************
   */

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */
		
   /*
   ************************
   C O D E

   ************************
   */
#ifdef TEST_HW
//test all LCD command/data lines
CLEAR_RD;       
SET_RD;       
CLEAR_WR;
SET_WR;        
CLEAR_DC;     
SET_DC;       
CLEAR_RESET;  
SET_RESET;    
ASSERT_LCD_CS;
NEGATE_LCD_CS;
AT91C_BASE_PIOB->PIO_ODSR =0x55;
AT91C_BASE_PIOB->PIO_ODSR =0xaa;
#endif

   // reset the lcd
   resetLcd();

   writeCommand(0xae); // display off
   
   writeCommand(0xb3); // set
   writeCommand(0x91); // set
  
   writeCommand(0xa8); // set
   writeCommand(0x3f); // set
   
   writeCommand(0xa2); // set
   writeCommand(0x4c); // set
      
   writeCommand(0xa1); // set
   writeCommand(0x00); // set
    	
   writeCommand(0xad); // set
   writeCommand(0x02); // set
   
   writeCommand(0xa0); // set
   
   writeCommand(0x50); // set
   
   writeCommand(0x86); // set

   writeCommand(0xB8); // Set Gray Scale Table
   writeCommand(0x01); // Gray Scale Level 1
   writeCommand(0x11); // Gray Scale Level 3 & 2
   writeCommand(0x22); // Gray Scale Level 5 & 4
   writeCommand(0x32); // Gray Scale Level 7 & 6
   writeCommand(0x43); // Gray Scale Level 9 & 8
   writeCommand(0x54); // Gray Scale Level 11 & 10
   writeCommand(0x65); // Gray Scale Level 13 & 12
   writeCommand(0x76);

   writeCommand(0x81); // set brightness
   writeCommand(0x7f); // set brightness
   
   writeCommand(0xb2); // set
   writeCommand(0x51); // set
   
   writeCommand(0xb1); // set
   writeCommand(0x55); // set
   
   writeCommand(0xbc); // set
   writeCommand(0x10); // set

   writeCommand(0xb4); // set
   writeCommand(0x02); // set
   
   writeCommand(0xb0); // set
   writeCommand(0x28); // set
        
   writeCommand(0xbe); // set
   writeCommand(0x1c); // set

   writeCommand(0xbf); // set
   writeCommand(0x0f); // set

   writeCommand(0xa4); // set display mode normal   
   
   clearLcd();

   writeCommand(0xaf); // display on   
  
} /* end lcdInit() */


void lcdDisplayOff(void)
{
   clearLcd();
   
   writeCommand(0xae);
}


void lcdDisplayOn(void)
{
   lcdInit();  
}


/*|------------------------------------------------------------------------
  |  Module: lcdHi.c
  |  Routine: writeCommand
  |  Description: Write a command byte to the LCD
  |------------------------------------------------------------------------
*/
void writeCommand
(
   UINT8 cmdByte_
)
{

   // enable output write, this allows us to write a byte to the port
   // using the ODSR register, otherwise we need to set the bits in SODR,
   // and clear the bits in CODR, hence we save an instruction cycle
   AT91C_BASE_PIOB->PIO_OWER =IO_BYTE;

   SET_RD;

   // command
   CLEAR_DC;   
   
   ASSERT_LCD_CS;   
   
   // clock the command byte
   clockByte(cmdByte_);

   NEGATE_LCD_CS;      

} // end writeCommand()


/*|------------------------------------------------------------------------
  |  Module: lcdHi.c
  |  Routine: writeData
  |  Description: Write a data byte to the LCD
  |------------------------------------------------------------------------
*/
void writeData
(
   UINT8 dataByte_
)
{ 
   // enable output write, this allows us to write a byte to the port
   // using the ODSR register, otherwise we need to set the bits in SODR,
   // and clear the bits in CODR, hence we save an instruction cycle
   AT91C_BASE_PIOB->PIO_OWER =IO_BYTE;

   SET_RD;
	
   // data
   SET_DC;

   ASSERT_LCD_CS;   
   
   // clock a data byte
   clockByte(dataByte_);
  
   NEGATE_LCD_CS;
   
} // end writeData()


/*|------------------------------------------------------------------------
  |  Module: lcdHi.c
  |  Routine: clockByte
  |  Description: clears the WR pin, outputs a byte, sets the WR pin (clock)
  |------------------------------------------------------------------------
*/
void clockByte
(
   UINT8 aByte_
)
{
   CLEAR_WR;

   AT91C_BASE_PIOB->PIO_ODSR =aByte_;

   SET_WR;

} // end clockByte()



/*|------------------------------------------------------------------------
  |  Module: lcdHi.c
  |  Routine: resetLcd
  |  Description: reset the lcd by toggling the reset pin
  |------------------------------------------------------------------------
*/
void resetLcd
(
   void
)
{
   // clear reset pin
   CLEAR_RESET;

   programDelay(1);

   // set reset pin
   SET_RESET;

} // end resetLcd()


/*|------------------------------------------------------------------------
  |  Module: lcdHi.c
  |  Routine: displayChar
  |  Description: Write pixels from top to bottom 5*7 character display
  |
  |  Create character 'A' and 'B'
  |
  |          ('A')           ('B')
  |         011100          111100
  |         100010          100010
  |         100010          100010
  |         111110          111100
  |         100010          100010
  |         100010          100010
  |         100010          111100
  |
  |   bitMap(A) =01 1100 1000 1010 0010 1111 1010 0010 1000 1010 0010
  |   bitMap =0x1c8a2fa28a2
  |
  |   bitMap(B) =11 1100 1000 1010 0010 1111 0010 0010 1000 1011 1100
  |   bitMap(B) =0x3c8a2f228bc
  |------------------------------------------------------------------------
*/
void displayChar
(
   UINT8 char_,
   UINT8 row_,
   UINT8 col_,
   BOOL intensity_
)
{
   /*
   **********************
   L O C A L   D A T A
   **********************
   */
   UINT8 displayByte[24], j, k;

   UINT8 pixelIntensity00;
   UINT8 pixelIntensity01;   
   UINT8 pixelIntensity10;   
   UINT8 pixelIntensity11;
   
   UINT32 result;     
   
   long long bitMap =0;
   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   /*
   ************************
   C O D E
   ************************
   */
   switch( intensity_ )  
   {
      case 0:
         pixelIntensity01 =0x40;   
         pixelIntensity10 =0x04;   
         pixelIntensity11 =0x44;                
         break;
      case 1:
         pixelIntensity01 =0x90;   
         pixelIntensity10 =0x09;   
         pixelIntensity11 =0x99;                
         break;
      case 2:
         pixelIntensity01 =0xf0;   
         pixelIntensity10 =0x0f;   
         pixelIntensity11 =0xff;        
         break;         
   }
   pixelIntensity00 =0x00;

   switch( char_ )
   {
      case 'A':        
         bitMap =0x1c8a2fa28a2;
         break;
      case 'B':   
         bitMap =0x3c8a2f228bc;
         break;
      case 'C':        
         bitMap =0x1c8a082089c;
         break;
       case 'D':
         bitMap =0x389228a2938;
         break;
      case 'E':       
         bitMap =0x3e820f2083e;
         break;
      case 'F':
         bitMap =0x3e820f20820;
         break;
      case 'G':  
         bitMap =0x1c8a0ba289e;
         break;
      case 'H':
         bitMap =0x228a2fa28a2;         
         break;
      case 'I':       
         bitMap =0x1c20820821c;
         break;
      case 'J':             
         bitMap =0x0e104104918;         
         break;
      case 'K':        
         bitMap =0x22928c28922;
         break;
      case 'L':       
          bitMap =0x2082082083e;
         break;
      case 'M':
         bitMap =0x22daaaa28a2;
         break;
      case 'N':  
         bitMap =0x228b2aa68a2;         
         break;
      case 'O':           
         bitMap =0x1c8a28a289c;
         break;
      case 'P':        
         bitMap =0x3c8a2f20820;         
         break;
      case 'Q':
          bitMap =0x1c8a28aa91a;
         break;
      case 'R':      
         bitMap =0x3c8a2f28922;         
         break;
      case 'S':        
         bitMap =0x1e8207020bc;
         break;
      case 'T':       
          bitMap =0x3e208208208;
         break;
      case 'U':        
          bitMap =0x228a28a289e;         
         break;
      case 'V':
          bitMap =0x228a28a2508;
         break;
      case 'W':
          bitMap =0x228a2aaaa94;
         break;
      case 'X':
          bitMap =0x228942148a2;
         break;
      case 'Y':
         bitMap =0x228a2508208;
         break;
      case 'Z': 
         bitMap =0x3e08421083e;
         break;
#ifdef LOWER_CASE         
      case 'a':
         break;
      case 'b':
         break;
      case 'c':
         break;
      case 'd':
         break;
      case 'e':
         break;
      case 'f':
         break;
      case 'g':
         break;
      case 'h':
         break;
      case 'i':
         break;
      case 'j':
         break;
      case 'k':
         break;
      case 'l':
         break;
      case 'm':
         break;
      case 'n':
         break;
      case 'o':
         break;
      case 'p':
         break;
      case 'q':
         break;
      case 'r':
         break;
      case 's':
         break;
      case 't':
         break;
      case 'u':
         break;
      case 'v':                                    
         bitMap =0x000228a2508;          
         break;
      case 'w':
         break;
      case 'x':
         break;
       case 'y':
         break;
      case 'z':
         break;
#endif         
      case ' ':        
         bitMap =0x00000000000;  
         break;
      case '1':       
         bitMap =0x0860820821c;   
         break;
      case '2': 
         bitMap =0x1c88210843e;
         break;
      case '3':        
         bitMap =0x3e10810289c;
         break;
      case '4':
         bitMap =0x0431493e104;
         break;
      case '5':       
         bitMap =0x3e83c08289c;
         break;
      case '6':
         bitMap =0x0c420f2289c;
         break;
      case '7':        
         bitMap =0x3e084210410;
         break;
      case '8':
         bitMap =0x1c8a272289c;
         break;
      case '9':
         bitMap =0x1c8a2782118;
         break;
      case '0':                                
         bitMap =0x1c8a6ab289c;
         break;
      case ':':
         break;
      case ';':
         break;
      case '.':       
          bitMap =0x00000000618;
         break;
      case ',':
         break;
      case '*':
         break;
      case '+':
         break;
      case '=':
         break;
      case '/':
         bitMap =0x00084210800;
         break;
      case '<':
         break;
      case '>':                
         bitMap =0x10204084210;  
         break;
      case '!':
         break;
      case '#':
         break;
      case '$':
         break;
      case '%':
         break;
      case '_':
         break;
      case '-':
         break;
      case '(':
         break;
      case ')':
         break;
      case '?':
         break;
      case '^':
         break;
      case ']':
         break;
      case '[':
         break;
      case '@':
         break;
      default:
         // invalid, or non coded character, indicate with the '.'
         bitMap =0x00000000618;
         break;
   }

   for(j=21; j>0; j--)
   {
      result =(bitMap&0x00000003);

      switch(result)
      {  
         case 0x03:
            displayByte[j-1] =pixelIntensity11;  
            break;
         case 0x02:
            displayByte[j-1] =pixelIntensity10;  
            break;            
         case 0x01:
            displayByte[j-1] =pixelIntensity01;  
            break;                        
         case 0x00:
            displayByte[j-1] =pixelIntensity00;              
            break;                               
         default:
            displayByte[j-1] =0x01; 
            break;
      }         
             
      bitMap =(bitMap>>2);
   }
   
   for(k=0; k<7; k++)
   {
      writeCommand(0x15); //  set column address
      writeCommand(col_); //  
      writeCommand(0x3f); //    
   
      writeCommand(0x75); // set row address
      writeCommand(row_+k); //  
      writeCommand(0x3f); //  

      for(j=0; j<3; j++)
      {     
         writeData(displayByte[j+3*k] );
      }
   }

}/* end displayChar() */


/*|------------------------------------------------------------------------
  |  Module: lcdHi.c
  |  Routine: programDelay
  |  Description: creates a delay in program execution in mSec (time_)
  |------------------------------------------------------------------------
*/
void programDelay
(
   UINT16 time_
)
{
   /*
   **********************
   L O C A L   D A T A
   **********************
   */
   UINT32 aTime;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   /*
   ************************
   C O D E
   ************************
   */
   aTime =TicksLeft;

   // Delay
   while( (TicksLeft-aTime) <time_ )
   {
      // TicksLeft incremented on timer ISR
   }

}/* end programDelay() */



void clearLcd
(
   void
)
{
   // clear screen   
   writeCommand(0x23); //
   writeCommand(0x01); //

   writeCommand(0x24); //
   writeCommand(0x00); //
   writeCommand(0x00); //
   writeCommand(0x3f); //
   writeCommand(0x5f); //
   writeCommand(0x00); //

   programDelay(2);
}



