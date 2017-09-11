/*|***************************************************************************/
/*|PROJECT:  OnStream Pipeline Inspection Gage                               */
/*|Module:   UsbComFd                                                        */
/*|Description:                                                              */
/*|                                                                          */
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
#ifndef USBCOM_FD_H
 #define USBCOM_FD_H


/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/



/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/

#define CCU_VERSION_INFO "1.02a  Feb/29/2012"
//                        | |||
//                        | |||-- Test Version, space indicates general release,
//                        | ||    a letter indicates a test version  (a-z. A-Z)
//                        | ||
//                        | ||--- Minor Version
//                        |
//                        |------ Major Version

// Note if you change the test version to space (valid release), ensure you replace the
// letter with a space (thus there is always three characters between minor version and date)
// three characters are either 'space' 'space' 'space' OR 'letter' 'space' 'space'


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

void InitUsbCom( void );


/*
*|----------------------------------------------------------------------------
*|  Module: UsbComFd
*|  Routine: UsbCommand
*|  Description:
*|----------------------------------------------------------------------------
*/
void UsbComm
(
   void
);

BOOL UsbEnumeration(void);
void AT91F_USB_Open(void);
UINT16 CalcCrc( UINT8 *dataBufPtr_, UINT16 numBytes_ );


#endif








