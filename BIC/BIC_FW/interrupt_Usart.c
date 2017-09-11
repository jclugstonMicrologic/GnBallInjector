
//*----------------------------------------------------------------------------
//*      ATMEL Microcontroller Software Support  -  ROUSSET  -
//*----------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*----------------------------------------------------------------------------
//* File Name           : interrupt_Usart.c
//* Object              : USART Interrupt Management
//*
//* 1.0 14/Dec/04 JPP   : Creation
//*----------------------------------------------------------------------------


// Include Standard LIB  files
#include "Board.h"

#include "cdc_enumerate.h"
#include "mk_std.h"
#include "uartSettings.h"

#define USART_INTERRUPT_LEVEL 1

AT91PS_USART COM0;
AT91PS_USART COM1;
AT91PS_USART COM2;

extern struct _AT91S_CDC 	pCDC;
//static char buff_rx[100];

/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
/* stuff to enable debug printing for this module */
#define DEBUG_SERIAL_HI_ON 0

#if (DEBUG_ALL_ON==1) || DEBUG_SERIAL_HI_ON==1
 #define DEBUG_SERIAL_HI(message) DEBUG(message, SERIAL_HI_TASK_ID)
#else
 #define DEBUG_SERIAL_HI(message) /* */
#endif


/* Typedef for tx and rx queue structures */
typedef struct
{
   UINT16 in; /* Queue input/write pointer */
   UINT16 full; /* Queue full flag */
   UINT16 out; /* Queue output/read pointer */
   UINT8 q[UART_MAX_RX_QUE_SIZE]; /* Queue array */
} QUE_STRUCT;


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
/* Transmit and receive queue structures */
QUE_STRUCT RxBuff[3];//, TxBuff;


/*
*****************************************************************************
 P R I V A T E   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/
void queInit
(
   QUE_STRUCT *quePtr
);



/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: GetByte
*|  Description:
*|   Checks serial rx register for data. This checking occurs for a specified
*|   duration or until data is found, whichever comes first.
*|----------------------------------------------------------------------------
*/
BOOL GetByte
(
   UINT8 *rxBytePtr_,
   UINT8 uart_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT16 rxIn, rxFull, rxOut;


   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   /* Read rx buffer variables into locals */
   rxIn = RxBuff[uart_].in;
   rxFull = RxBuff[uart_].full;
   rxOut = RxBuff[uart_].out;
   if( (rxIn!=rxOut)||rxFull )
   {
      *rxBytePtr_ = RxBuff[uart_].q[rxOut]; /* read data byte */
      rxOut++; /* update pointer..*/
      if( rxOut>(UART_MAX_RX_QUE_SIZE-1) ) rxOut = 0;/* check for wraparound */

      RxBuff[uart_].full =0x00;
      RxBuff[uart_].out =rxOut;

      return true; /* return 1 for successful read */
   }
   else
   {
      return false; /* return 0 as no byte available */
   }

}/* GetByte() */


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: SendByte_Uart0
*|  Description:
*|----------------------------------------------------------------------------
*/
BOOL SendByte_Uart0
(
   UINT8 txByte_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 status;

   /*
   ***********************
    C O D E
   ***********************
   */
   status =AT91C_BASE_US0->US_IMR;
   status =AT91C_BASE_US0->US_CSR ;

   while( !(status & AT91C_US_TXRDY) )	
   {
      status =AT91C_BASE_US0->US_CSR ;
   }

   AT91C_BASE_US0->US_THR =txByte_;

   return true;
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: SendByte_Uart1
*|  Description:
*|----------------------------------------------------------------------------
*/
BOOL SendByte_Uart1
(
   UINT8 txByte_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 status;

   /*
   ***********************
    C O D E
   ***********************
   */
   status =AT91C_BASE_US1->US_IMR;
   status =AT91C_BASE_US1->US_CSR ;

   while( !(status & AT91C_US_TXRDY) )	
   {
      status =AT91C_BASE_US1->US_CSR ;
   }

   AT91C_BASE_US1->US_THR =txByte_;

   return true;
}


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: SendByte_Uart2
*|  Description:
*|----------------------------------------------------------------------------
*/
BOOL SendByte_Uart2
(
   UINT8 txByte_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 status;

   /*
   ***********************
    C O D E
   ***********************
   */
   status =AT91C_BASE_US2->US_IMR;
   status =AT91C_BASE_US2->US_CSR ;

   while( !(status & AT91C_US_TXRDY) )	
   {
      status =AT91C_BASE_US2->US_CSR ;
   }

   AT91C_BASE_US2->US_THR =txByte_;

   return true;
}


//*----------------------------------------------------------------------------
//* Function Name       : Usart0_c_irq_handler
//* Object              : C handler interrupt function called by the interrupts
//*                       assembling routine
//*----------------------------------------------------------------------------
void Usart0_c_irq_handler(void)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 sciData;
   UINT16 status, qIn, qFull, qOut;
   UINT32 qLong;

   AT91PS_USART USART_pt =COM0;

   //* get Usart status register and active interrupt
   status = USART_pt->US_CSR;
   status &= USART_pt->US_IMR;


   if( status & AT91C_US_RXRDY )	
   {
      /* interrupt is because rx data is available */
      sciData =(AT91C_BASE_US0->US_RHR & 0x1FF);

// loop out Tx for debug
//AT91F_US_PutChar(USART_pt, sciData);

      /* Read rx buffer variables into locals */
      qIn = RxBuff[0].in;
      qFull = RxBuff[0].full;
      qOut = RxBuff[0].out;

      if( (qIn!=qOut) || !qFull )
      {
         /* rx buffer is not full and data is available */
         RxBuff[0].q[qIn] = (UINT8)sciData;
         qIn++;
         if( qIn>(UART0_MAX_RX_QUE_SIZE-1) )
         {
            qIn = 0; /* check for wraparound */
         }

         if( qIn==qOut )
         {
            /* Buffer now full, so set FULL flag coherently with update of qin */
            /* Also generate warning that buffer is now full */
            qLong = (((UINT32)qOut)<<16)|1;/*! prepare for coherent write..*/
            *(UINT32 *)(&RxBuff[0].in) = qLong;/*of qIn with rxqFull non-zero*/
         }
         else
         {
            /* Buffer not full yet, so don't set FULL */
            RxBuff[0].in = qIn;
         }
      }
   }

   if( status & AT91C_US_TIMEOUT )
   {
      sciData =AT91F_US_GetChar(USART_pt);
   }

#ifdef OTHER_ERRORS
   if( status & AT91C_US_OVRE )
   {
X =1;
   }

   if( status & AT91C_US_FRAME )
   {
X =2;
   }

   if( status & AT91C_US_PARE )
   {
X =3;
   }
#endif

   USART_pt->US_CR = AT91C_US_RSTSTA;
}

//*----------------------------------------------------------------------------
//* Function Name       : Usart1_c_irq_handler
//* Object              : C handler interrupt function called by the interrupts
//*                       assembling routine
//*----------------------------------------------------------------------------
void Usart1_c_irq_handler(void)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 sciData;
   UINT16 status, qIn, qFull, qOut;
   UINT32 qLong;

   AT91PS_USART USART_pt =COM1;

   //* get Usart status register and active interrupt
   status = USART_pt->US_CSR;
   status &= USART_pt->US_IMR;


   if( status & AT91C_US_RXRDY )	
   {
      /* interrupt is because rx data is available */
      sciData =(AT91C_BASE_US1->US_RHR & 0x1FF);

// loop out Tx for debug
//AT91F_US_PutChar(USART_pt, sciData);

      /* Read rx buffer variables into locals */
      qIn = RxBuff[1].in;
      qFull = RxBuff[1].full;
      qOut = RxBuff[1].out;

      if( (qIn!=qOut) || !qFull )
      {
         /* rx buffer is not full and data is available */
         RxBuff[1].q[qIn] = (UINT8)sciData;
         qIn++;
         if( qIn>(UART1_MAX_RX_QUE_SIZE-1) )
         {
            qIn = 0; /* check for wraparound */
         }

         if( qIn==qOut )
         {
            /* Buffer now full, so set FULL flag coherently with update of qin */
            /* Also generate warning that buffer is now full */
            qLong = (((UINT32)qOut)<<16)|1;/*! prepare for coherent write..*/
            *(UINT32 *)(&RxBuff[1].in) = qLong;/*of qIn with rxqFull non-zero*/
         }
         else
         {
            /* Buffer not full yet, so don't set FULL */
            RxBuff[1].in = qIn;
         }
      }
   }

   if( status & AT91C_US_TIMEOUT )
   {
      sciData =AT91F_US_GetChar(USART_pt);
   }


   USART_pt->US_CR = AT91C_US_RSTSTA;
}


//*----------------------------------------------------------------------------
//* Function Name       : Usart2_c_irq_handler
//* Object              : C handler interrupt function called by the interrupts
//*                       assembling routine
//*----------------------------------------------------------------------------
void Usart2_c_irq_handler(void)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   UINT8 sciData;
   UINT16 status, qIn, qFull, qOut;

   UINT32 qLong;

   AT91PS_USART USART_pt =COM2;

   //* get Usart status register and active interrupt
   status = USART_pt->US_CSR;
   status &= USART_pt->US_IMR;


   if( status & AT91C_US_RXRDY )	
   {
      /* interrupt is because rx data is available */
      sciData =(AT91C_BASE_US2->US_RHR & 0x1FF);

// loop out Tx for debug
//AT91F_US_PutChar(USART_pt, sciData);

      /* Read rx buffer variables into locals */
      qIn = RxBuff[2].in;
      qFull = RxBuff[2].full;
      qOut = RxBuff[2].out;

      if( (qIn!=qOut) || !qFull )
      {
         /* rx buffer is not full and data is available */
         RxBuff[2].q[qIn] = (UINT8)sciData;
         qIn++;
         if( qIn>(UART2_MAX_RX_QUE_SIZE-1) )
         {
            qIn = 0; /* check for wraparound */
         }

         if( qIn==qOut )
         {
            /* Buffer now full, so set FULL flag coherently with update of qin */
            /* Also generate warning that buffer is now full */
            qLong = (((UINT32)qOut)<<16)|1;/*! prepare for coherent write..*/
            *(UINT32 *)(&RxBuff[2].in) = qLong;/*of qIn with rxqFull non-zero*/
         }
         else
         {
            /* Buffer not full yet, so don't set FULL */
            RxBuff[2].in = qIn;
         }
      }
   }

   if( status & AT91C_US_TIMEOUT )
   {
      sciData =AT91F_US_GetChar(USART_pt);
   }


   USART_pt->US_CR = AT91C_US_RSTSTA;
}



//*-------------------------- External Function -------------------------------

//*----------------------------------------------------------------------------
//* \fn    AT91F_US_Printk
//* \brief This function is used to send a string through the US channel
//*----------------------------------------------------------------------------
void AT91F_US_Put( char *buffer) // \arg pointer to a string ending by \0
{
	while(*buffer != '\0') {
		while (!AT91F_US_TxReady(COM0));
		AT91F_US_PutChar(COM0, *buffer++);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : Usart0_init
//* Object              : USART initialization
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void Usart0_init
(
   UINT16 baudRate_
)
//* Begin
{
//   queInit(&TxBuff);
   queInit(&RxBuff[0]);

   COM0= AT91C_BASE_US0;

   //* Define RXD and TXD as peripheral
   // Configure PIO controllers to periph mode
   AT91F_PIO_CfgPeriph(
	 AT91C_BASE_PIOA, // PIO controller base address
	 ((unsigned int) AT91C_PA2_RXD0    ) |
	 ((unsigned int) AT91C_PA3_TXD0    ) , // Peripheral A
	 0 ); // Peripheral B

   //* First, enable the clock of the PIOB
   AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_US0 ) ;

   //* Usart Configure
   AT91F_US_Configure(COM0, MCK,AT91C_US_ASYNC_MODE, baudRate_ , 0);

   //* Enable usart
   COM0->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;

   //* open Usart interrupt
   AT91F_AIC_ConfigureIt (AT91C_BASE_AIC, AT91C_ID_US0, USART_INTERRUPT_LEVEL,
                          AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, Usart0_c_irq_handler);

   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US0);
   // Set the PDC
   AT91F_PDC_Open (AT91C_BASE_PDC_US0);

//   COM0->US_RPR = (unsigned int) buff_rx;
   COM0->US_RCR = 10;
   COM0->US_RTOR = 10;
   //* Enable USART IT error and AT91C_US_ENDRX
//    AT91F_US_EnableIt(COM0, AT91C_US_RXBUFF | AT91C_US_TIMEOUT );

    AT91F_US_EnableIt(COM0, AT91C_US_RXRDY);// | AT91C_US_TIMEOUT );// | AT91C_US_TXRDY );
}

//*----------------------------------------------------------------------------
//* Function Name       : Usart_init
//* Object              : USART initialization
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void Usart1_init
(
   UINT16 baudRate_
)
//* Begin
{
//   queInit(&TxBuff);
   queInit(&RxBuff[1]);

   COM1= AT91C_BASE_US1;

   //* Define RXD and TXD as peripheral
   // Configure PIO controllers to periph mode
   AT91F_PIO_CfgPeriph(
                        AT91C_BASE_PIOA, // PIO controller base address
                       ((unsigned int) AT91C_PA7_RXD1 ) |
                       ((unsigned int) AT91C_PA8_TXD1 ) , // Peripheral A
                      0 ); // Peripheral B

   //* First, enable the clock of the PIOB
   AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_US1 ) ;

   //* Usart Configure
   AT91F_US_Configure(COM1, MCK, AT91C_US_ASYNC_MODE,baudRate_, 0);

   //* Enable usart
   COM1->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;

   //* open Usart interrupt
   AT91F_AIC_ConfigureIt (AT91C_BASE_AIC, AT91C_ID_US1, USART_INTERRUPT_LEVEL,
                          AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, Usart1_c_irq_handler);
   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US1);
   // Set the PDC
   AT91F_PDC_Open (AT91C_BASE_PDC_US1);

//    COM1->US_RPR = (unsigned int) buff_rx;
   COM1->US_RCR =100;
   COM1->US_RTOR =10;
   //* Enable USART IT error and AT91C_US_ENDRX
//    AT91F_US_EnableIt(COM1, AT91C_US_RXBUFF | AT91C_US_TIMEOUT );
   AT91F_US_EnableIt(COM1, AT91C_US_RXRDY);// | AT91C_US_TIMEOUT );// | AT91C_US_TXRDY );
//* End
}


//*----------------------------------------------------------------------------
//* Function Name       : Usart_init
//* Object              : USART initialization
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void Usart2_init
(
   UINT16 baudRate_
)
//* Begin
{
//   queInit(&TxBuff);
   queInit(&RxBuff[2]);

   COM2= AT91C_BASE_US2;

   //* Define RXD and TXD as peripheral
   // Configure PIO controllers to periph mode
   AT91F_PIO_CfgPeriph(
                        AT91C_BASE_PIOA, // PIO controller base address
                       ((unsigned int) AT91C_PA9_RXD2 ) |
                       ((unsigned int) AT91C_PA10_TXD2 ) , // Peripheral A
                      0 ); // Peripheral B

   //* First, enable the clock of the PIOB
   AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<<AT91C_ID_US2 ) ;

   //* Usart Configure
   AT91F_US_Configure(COM2, MCK, AT91C_US_ASYNC_MODE,baudRate_, 0);

   //* Enable usart
   COM2->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;

   //* open Usart interrupt
   AT91F_AIC_ConfigureIt (AT91C_BASE_AIC, AT91C_ID_US2, USART_INTERRUPT_LEVEL,
                          AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, Usart2_c_irq_handler);
   AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US2);
   // Set the PDC
   AT91F_PDC_Open (AT91C_BASE_PDC_US2);

//    COM1->US_RPR = (unsigned int) buff_rx;
   COM2->US_RCR =100;
   COM2->US_RTOR =10;
   //* Enable USART IT error and AT91C_US_ENDRX
//    AT91F_US_EnableIt(COM1, AT91C_US_RXBUFF | AT91C_US_TIMEOUT );
   AT91F_US_EnableIt(COM2, AT91C_US_RXRDY);// | AT91C_US_TIMEOUT );// | AT91C_US_TXRDY );
//* End
}



/*
*|----------------------------------------------------------------------------
*|  Module: SerialCom
*|  Routine: queInit
*|  Description:
*|   Initialises a specified que
*|----------------------------------------------------------------------------
*/
void queInit
(
   QUE_STRUCT *quePtr
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
   quePtr->in   = 0x0002; /* Set in=out, and full=0, ie. buffer empty */
   quePtr->full = 0x0000;
   quePtr->out  = 0x0002;

}/* queInit() */
