/*|***************************************************************************/
/*|PROJECT:  Tuboscope Pipeline Inspection Gage                              */
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
#ifndef UART_SETTINGS_H
 #define UART_SETTINGS_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/

/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/

/* max rx circ buf size  */
#define UART_MAX_RX_QUE_SIZE ((UINT16)250)          // hw uart0-2

#define UART0_MAX_RX_QUE_SIZE (UART_MAX_RX_QUE_SIZE)          // hw uart0-2
#define UART1_MAX_RX_QUE_SIZE (UART_MAX_RX_QUE_SIZE)          // hw uart0-2
#define UART2_MAX_RX_QUE_SIZE (UART_MAX_RX_QUE_SIZE)          // hw uart0-2


#define UART0 (0) // spi uart0 ChA
#define UART1 (1) // spi uart0 ChB
#define UART2 (2) // spi uart1 ChA
#define UART3 (3) // spi uart1 ChB
#define UART4 (0) // hardware uart0

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
/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/

#endif

/* End uartSettings.h */


