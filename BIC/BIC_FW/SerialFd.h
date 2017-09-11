//---------------------------------------------------------------------------
#ifndef serialFdH
#define serialFdH
//---------------------------------------------------------------------------

/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "mk_Std.h"

/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/





/*
*****************************************************************************
 P U B L I C    M A C R O S
*****************************************************************************
*/
#define ENABLE_SERIAL_RX    AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, RS485_RX_ENABLE )
#define ENABLE_SERIAL_TX    AT91F_PIO_SetOutput( AT91C_BASE_PIOA, RS485_TX_ENABLE )
#define DISABLE_SERIAL_RX   AT91F_PIO_SetOutput( AT91C_BASE_PIOA, RS485_RX_ENABLE )
#define DISABLE_SERIAL_TX   AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, RS485_TX_ENABLE )

/* define the possible states for the rx machine */
enum
{
   SERIAL_RX_IDLE_STATE=0,
   SERIAL_RX_BUILD_MSG_STATE,

   SERIAL_RX_DATA_MSG_STATE

};


/* define the possible states for the tx machine */
enum
{
   SERIAL_TX_IDLE_STATE=0,
   SERIAL_TX_SENDING_STATE
};


enum
{
   MSG_IGNORED,                /* message ignored, incorrect address */
   ACK1,                       /* a reply: indicating un-corrupted message rx'ed */
   ACK2,                       /* a reply: msg rx'ed and successfully processed*/
   NACK1,                      /* a reply indicating corrupted message detected */
   NACK2,                      /* a reply indicating good message, processing failed */
   LAST_LINK_LAYER             /* marker for last enum value used for link-layer,as these */
                               /* must not be used by application command codes  */

};


/* types of messages supported: 8, 16, or 32 bit data */
#define SERIAL_MSG_TYPE8  ((UINT8)8)
#define SERIAL_MSG_TYPE16 ((UINT8)16)
#define SERIAL_MSG_TYPE32 ((UINT8)32)

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
BOOL InitSerialCom( UINT8 numOfPorts_);

int SerialRxMachine( void );
int SerialTxMachine( void );

void SerialMessagingMachine( void );

BOOL SendMessage
(
   UINT8 msgType_,
   UINT8 portIndex_,
   UINT16 numBytes_,
   UINT8 * txBufPtr_
);


BOOL SendSerialString
(
   char *txBufPtr_
);


#endif



