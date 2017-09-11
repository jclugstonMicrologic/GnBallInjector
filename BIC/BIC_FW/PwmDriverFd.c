/*|***************************************************************************/
/*|PROJECT: GN Ball Injector Controller (BIC)                                */
/*|Module:   Heater Control                                                  */
/*|Description:                                                              */
/*|                                                                          */
/*|***************************************************************************/


/*
Tools:
   Compiler: IAR Kickstart development environment for AT91SAM7Sxxx
   ***************************************************************************
   Copyright (C) 2001, Micrologic Limited.
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
#include "PwmDriverFd.h"
#include "pwmHi.h"

#include "timer.h"


/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/


/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
INT8 PwmDuty;

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

/*
*|----------------------------------------------------------------------------
*|  Module: PwmDriverFd
*|  Routine: Pwm_InitDriver
*|  Description:
*|   Initialisation for this module
*|----------------------------------------------------------------------------
*/
void Pwm_InitDriver
(
   void
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
   pwmInitCh(3);  
   
   PwmDuty =0;
//x=TicksLeft;
//while( (TicksLeft-x)<2000 ){}            
}

/*
*|----------------------------------------------------------------------------
*|  Module: PwmDriverFd
*|  Routine: Pwm_Control
*|  Description:
*|----------------------------------------------------------------------------
*/
void Pwm_SetDuty
(
   INT8 duty_
)
{
   duty_ =100-duty_;
   
   
   if( duty_==100 )
      duty_=0;
   else if( duty_==0 )
      duty_=1;   
       
   adjustPwmDuty(3, duty_);
}








