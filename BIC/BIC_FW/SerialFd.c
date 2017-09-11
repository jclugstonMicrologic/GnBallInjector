/*|***************************************************************************/
/*|PROJECT:  Id Technologies Hermes3                                         */
/*|Module:   SerialFd                                                        */
/*|Description: function driver for the module                               */
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
#include "cpuPorts.h"
#include "string.h"
#include "timer.h"
#include "SerialFd.h"
#include "GsmRadioFd.h"
#include "C:\Software\CompactCompressors\CCU\commonCCU\serialProtocol.h"

#include <time.h>

// external functions
extern BOOL GetByte( UINT8 *, UINT8);
extern BOOL SendByte_Uart0( UINT8 );
extern BOOL SendByte_Uart1( UINT8 );
extern BOOL SendByte_Uart2( UINT8 );

extern void Usart0_init ( UINT16 );
extern void Usart1_init ( UINT16 );
extern void Usart2_init ( UINT16 );

extern void AT91F_US_Put( char *buffer); // \arg pointer to a string ending by \0

/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
#define IRQ0_INTERRUPT_LEVEL	7
#define PIO_INTERRUPT_LEVEL	6

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/
enum
{
   MSGING_IDLE =0,
   MSGING_TX_QUED,

   MSGING_LAST
};

#define GET_BYTE(rxBytePtr, uart)  GetByte(rxBytePtr, 0)
#define SEND_BYTE(txBytePtr)  SendByte_Uart0(txBytePtr)
/*
*****************************************************************************
 P U B L I C   G L O B A L   D A T A
*****************************************************************************
*/


/*
*****************************************************************************
 P R I V A T E   G L O B A L   D A T A
*****************************************************************************
*/
   /* an rx sequence is terminated if this amount of time passes by without */
   /* next character being received; units are milliseconds */
   #define MAX_MSG_TIME ((UINT32) 500)

   /* the number of bytes to receive per iternation of the state machine */
   #define RX_BYTES_PER_CYCLE ((UINT16)10)

   /* structure used to hold rx state machine context */
   typedef struct
   {
      int pollRetries;
      int rxBytesPerCycle;
      int rxBufIndex;
      int rxByteCount;
      char rxBuf[MAX_RX_MSG_LENGTH];
      UINT32 msgStartTime;
      int SerialRxMachState;
      UINT8 msgType;
      int bytesToFollow;  /* includes message bytes and CRC bytes */

   }RX_CONTEXT;

   /* structure used to hold tx state machine context */
   typedef struct
   {
      void (* txFailNotifyPtr)(void);
      char txBuf[MAX_TX_MSG_LENGTH];
      int SerialTxMachState;

      UINT16 bytesToTx;
   }TX_CONTEXT;


   /* structures to hold context of each serial port. */
   static RX_CONTEXT RxContext[1];
   static TX_CONTEXT TxContext[1];

   /* used to start the tx state machine processing */
   static BOOL TxStart;

   UINT8 CrFlag;

/*
*****************************************************************************
 P R I V A T E   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/
UINT16 CalcSerialCrc( char *dataBufPtr_, UINT16 numBytes_ );


/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/

/*
*|----------------------------------------------------------------------------
*|  Module: SerialCom
*|  Routine: InitSerialCom
*|  Description:
*|    Initialise the serial communication data structures
*|----------------------------------------------------------------------------
*/
BOOL InitSerialCom
(
   UINT8 portNbr_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 portIndex;

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

   /* initialise state machines */
   for( portIndex =0; portIndex<3; portIndex++)
   {
      RxContext[portIndex].SerialRxMachState =SERIAL_RX_IDLE_STATE;
   }

   switch( portNbr_ )   
   {
      case 0:
         Usart0_init( 38400 );
//for(int j=0; j<1000; j++)         
//   SendByte_Uart0('A');         
         break;
      case 1:
         Usart1_init( 38400 );
        
ENABLE_SERIAL_RX;
ENABLE_SERIAL_TX;

for(int j=0; j<1000; j++)
   SendByte_Uart1('B');                  
         break;
      case 2:
         Usart2_init( 38400 );
for(int j=0; j<1000; j++)         
   SendByte_Uart2('C');
         break;         
   }
     
   return true;

} /* InitSerialCom() */



/*
*|----------------------------------------------------------------------------
*|  Module: SerialFd
*|  Routine: SerialRxMachine
*|  Description:
*|----------------------------------------------------------------------------
*/
int SerialRxMachine               /*| returns state machine state  */
(                                 /*|                              */
   void                           /*|                              */
)                                 /*|                              */
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 rxByte;
   UINT16 j;
   UINT8 portIndex;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */
   /* Check for uninitialized pointers, out of bounds inputs, etc */


   /*
   *************************
    C O D E
   *************************
   */
//   for( portIndex=0; portIndex<NumberOfPorts; portIndex++)
   portIndex =0;

   {
   switch( RxContext[portIndex].SerialRxMachState )
   {
      case SERIAL_RX_BUILD_MSG_STATE:
         /* wait a bit for data from serial port */
         for( j=0; j<1000; j++)
         {
            while( !GetByte(&rxByte, 0) )
            {
               /* no additional data received, waited long enough? */
               if( TicksLeft - RxContext[portIndex].msgStartTime > MAX_MSG_TIME )
               {
                  Gsm_ProcessResponse( &GsmData, RxContext[portIndex].rxBuf );
                  memset( RxContext[portIndex].rxBuf, 0x00, sizeof(RxContext[portIndex].rxBuf) );
                  RxContext[portIndex].SerialRxMachState = SERIAL_RX_IDLE_STATE;
                  return RxContext[portIndex].SerialRxMachState;
               }
            }
//            else
            {
               /* count bytes received so far */
               RxContext[portIndex].rxByteCount++;
               RxContext[portIndex].rxBuf[ RxContext[portIndex].rxBufIndex++] =rxByte;

#ifdef jeff
               if( rxByte ==0x0d )
               {
                  CrFlag =1;
               }
               else if( rxByte ==0x0a && CrFlag )
                  Gsm_ProcessResponse( &GsmData, RxContext[portIndex].rxBuf );
#endif
            }
         }/* for num of char to rx per state machine cycle */

         return RxContext[portIndex].SerialRxMachState = SERIAL_RX_IDLE_STATE;

      case SERIAL_RX_IDLE_STATE:
      default:
         {
            /* set up number of times to poll serial port per state change*/
            RxContext[portIndex].pollRetries = 3;

            /* point to start of received data buffer */
            RxContext[portIndex].rxBufIndex = 0;

            /* set up number of bytes to rx per state machine cycle */
            RxContext[portIndex].rxBytesPerCycle = 0;

            /* set up number of bytes rx'ed during this state machine cycle */
            RxContext[portIndex].rxByteCount = 0;

            while( !GetByte(&rxByte, 0) )
            {
               /* no data received */
               if( RxContext[portIndex].pollRetries-- == 0 )
               {
                  return RxContext[portIndex].SerialRxMachState;
               }
            }

//            if( rxByte =='A' )
//                rxByte ==TX_MSG_HEADER
//              )
            {
               RxContext[portIndex].rxBuf[RxContext[portIndex].rxBufIndex++] = rxByte;
               RxContext[portIndex].rxByteCount++;
               RxContext[portIndex].rxBytesPerCycle++;
               RxContext[portIndex].msgStartTime = TicksLeft;
               RxContext[portIndex].SerialRxMachState = SERIAL_RX_BUILD_MSG_STATE;

            }

            CrFlag =0;
            return RxContext[portIndex].SerialRxMachState;
         }

   }/* switch: on machine state */
   }/* for each com port */


} /* SerialRxMachine */


/*
*|----------------------------------------------------------------------------
*|  Module: SerialFd
*|  Routine: SerialTxMachine
*|  Description:
*|----------------------------------------------------------------------------
*/
int SerialTxMachine                /*| returns state machine state  */
(                                  /*|                              */
   void
)                                  /*|                              */
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   int j, portIndex, numTxBytes;

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
//   for( portIndex=0; portIndex<1; portIndex++)
   portIndex =0;
   {
      switch( TxContext[portIndex].SerialTxMachState )
      {
      case SERIAL_TX_SENDING_STATE:
         /* send each and all bytes of the tx buffer   */
         /* thru serial port; all tasks blocked during this "send" */

         numTxBytes =TxContext[portIndex].bytesToTx;

         for( j=0; j<numTxBytes; j++)
         {
            KickWdt();

            if( !SEND_BYTE( TxContext[portIndex].txBuf[j]) )
            {
               /* error with serial port, stop tx, notify if needed */
               if( TxContext[portIndex].txFailNotifyPtr != NULL )
               {
                  /* invoke the requested routine */
                  (*TxContext[portIndex].txFailNotifyPtr)();
               }
               TxStart =0;

               return TxContext[portIndex].SerialTxMachState = SERIAL_TX_IDLE_STATE;
            }
         }

         TxStart =0;
         TxContext[portIndex].SerialTxMachState = SERIAL_TX_IDLE_STATE;

         break;



      case SERIAL_TX_IDLE_STATE:
      default:
         if( !TxStart )
         {
            /* nothing to transmit */
            return SERIAL_TX_IDLE_STATE;
         }

         TxContext[portIndex].SerialTxMachState = SERIAL_TX_SENDING_STATE;

         break;
      }/* switch: on machine state */

   }/* for: each serial port */
   return TxContext[portIndex].SerialTxMachState;

} /* SerialTxMachine */





/*
*|----------------------------------------------------------------------------
*|  Module: SerialFd
*|  Routine: SendMessage
*|  Description:
*|   Serial communications transmission request.
*|----------------------------------------------------------------------------
*/
BOOL SendMessage
(
   UINT8 msgType_,
   UINT8 portIndex_,
   UINT16 numBytes_,
   UINT8 *txBufPtr_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   int i, j;
   UINT16 tempCrc;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   if( numBytes_ > MAX_QUE_SIZE )
   {
      /* not allowed */
      return false;
   }

   /*
   *************************
    C O D E
   *************************
   */
   if( TxContext[portIndex_].SerialTxMachState == SERIAL_TX_IDLE_STATE )
   {
      /* build up the actual tx bytes into Tx Buffer... */
      /* set the header bytes */
      TxContext[portIndex_].txBuf[SERIAL_HEADER_INDEX] =TX_MSG_HEADER;

      /* set the message type */
      TxContext[portIndex_].txBuf[SERIAL_TYPE_INDEX] =msgType_;

      /* set the MSbyte of bytesToFollow of buffer*/
      TxContext[portIndex_].txBuf[SERIAL_BYTES_TO_FOLLOW_MSB_INDEX] =
         (UINT8)((numBytes_ + SIZE_OF_CRC_VALUE)>>8);

      /* set the LSbyte of bytesToFollow of buffer*/
      TxContext[portIndex_].txBuf[SERIAL_BYTES_TO_FOLLOW_LSB_INDEX] =
         (UINT8)((numBytes_ + SIZE_OF_CRC_VALUE) & 0x00FF );

      /* get data bytes of the real message */
      for( (i=SIZE_OF_HEADER + SIZE_OF_BYTES_TO_FOLLOW +1) && (j=0);
           j < numBytes_;
           i++ && j++
         )
      {
         TxContext[portIndex_].txBuf[i] = txBufPtr_[j];
      }

      /* get the CRC of entire buffer except the CRC bytes*/
      tempCrc = CalcSerialCrc( &TxContext[portIndex_].txBuf[SERIAL_HEADER_INDEX],
                          numBytes_ + SIZE_OF_HEADER + SIZE_OF_MSG_TYPE + SIZE_OF_BYTES_TO_FOLLOW
                       );

      TxContext[portIndex_].txBuf[i] = (UINT8)(tempCrc>>8); /* MSbyte */
      TxContext[portIndex_].txBuf[i+1]=(UINT8)(tempCrc & 0x00FF); /*LSbyte */

      TxContext[portIndex_].bytesToTx =numBytes_+6;

      /* tell state machine something to do */
      TxStart = true;

      return true;
   }
   else
   {
      return false;
   }

} /* end SendMessage() */



BOOL SendSerialString
(
   char *txBufPtr_
)
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
   if( TxContext[0].SerialTxMachState == SERIAL_TX_IDLE_STATE )
   {
      strcpy( TxContext[0].txBuf, txBufPtr_ );

      TxContext[0].bytesToTx =strlen( TxContext[0].txBuf);

      /* tell state machine something to do */
      TxStart = true;

      return true;
   }
   else
   {
      return false;
   }

} /* end SendString() */



/*
*|----------------------------------------------------------------------------
*|  Module: Serial Communications
*|  Routine: CalcSerialCrc
*|  Description:
*|   Calculates CRC on a buffer of data.
*|----------------------------------------------------------------------------
*/
UINT16 CalcSerialCrc               /*| returns CRC value            */
(                                  /*|                              */
   char *dataBufPtr_,             /*|                              */
   UINT16 numBytes_                 /*|                              */
)                                  /*|                              */
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   int i;
   int x;
   unsigned int crc, feed;
   char *tempPtr;


   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */
   /* Check for uninitialized pointers, out of bounds inputs, etc */


   /*
   *************************
    C O D E
   *************************
   */

   crc = 0;
   for ( tempPtr = dataBufPtr_;
         tempPtr < (dataBufPtr_ + numBytes_);
         tempPtr++
       )
   {
      feed = (unsigned int)(*tempPtr);
      for ( i=0; i<8; i++)
      {
         x = crc & 0x0001;
         crc= crc>>1;
         if ( feed & 0x0001 )
         {
            crc = crc | 0x8000;
         }
         feed = feed>>1;
         if ( x )
         {
            crc = crc ^ 0x1021;
         }
      }
   }
   return crc;

}/* end CalcCrc() */









