/*|***************************************************************************/
/*|PROJECT:  OnStream Above Ground Marker                                    */
/*|Module:   extAdcHi                                                        */
/*|Description:                                                              */
/*|                                                                          */
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
#ifndef EXTERNAL_ADC_HI_H
 #define EXTERNAL_ADC_HI_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "spiHi.h"

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

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/

/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/
void extAdcInit
(
   UINT8 periphNum_
);

void readExtAdc
(
   UINT8 commandByte_,
   UINT8 *rxDataPtr_,
   UINT8 numBytes_
);





#endif

