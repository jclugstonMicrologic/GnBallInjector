/*|***************************************************************************/
/*|PROJECT: Compact Compressor CCU                                           */
/*|Module:   pwmHi                                                           */
/*|Description:                                                              */
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

#ifndef PWM_HI_H
 #define PWM_HI_H

/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "mk_std.h"

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/

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
void pwmInit
(
   void
);

void pwmInitCh
(
   UINT8 ch_
);

void adjustPwmFreq
(
   UINT8 ch_,
   UINT8 freq_
);

void adjustPwmDuty
(
   UINT8 ch_,
   UINT32 duty_
);


#endif


