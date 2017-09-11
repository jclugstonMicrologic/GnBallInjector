/*
*|***************************************************************************
*|PROJECT: GN Ball Injector Controller (BIC)                                
*|Module:  Switch function driver
*|Description:
*|   The module that detects the user input switch.  It uses callback
*|   routines to allow different actions to be invoked.  Even though
*|   only one state machine is used, this module handles N independent switches.
*|**************************************************************************
*/
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
#include "switchFd.h"
#include "Timer.h"
#include "cpuPorts.h"

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
SWITCH_DATA SwitchData[MAX_NUM_SWITCHES];

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

#define SWITCH_ASSERTED(switchId_)  GetSwitchStatus (switchId_)

/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/


/*
*|----------------------------------------------------------------------------
*|  Module: switch
*|  Routine:  InitSwitch
*|  Description:
*|   Initialises associated software and hardware.  It sets up the callback
*|   functions that may be invoked too.
*|----------------------------------------------------------------------------
*/
void InitSwitch
(
   UINT8 switchId_,
   void *callBackPtr1_,
   void *callBackPtr2_,
   void *callBackPtr3_,
   void *callBackPtr4_
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


} /* InitSwitch()  */


/*
*|----------------------------------------------------------------------------
*|  Module: switch
*|  Routine:  SwitchMachine
*|  Description:
*|    The state machine that detects the condition of the toggle switches.
*|----------------------------------------------------------------------------
*/
void SwitchMachine
(
   UINT8 switchId_
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   int id;

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
   id = switchId_;

   switch( SwitchData[ id ].machineState )
   {
         case SWITCH_IDLE_STATE:
//            if (!SWITCH_ASSERTED(id) )
//            {
            SwitchData[id].machineState = NORMAL_NEGATED_STATE;
//            }
            break;
         case NORMAL_NEGATED_STATE:
            /* switch is negated */
            if( SWITCH_ASSERTED(id) )
            {
               /* switch looks asserted, start debounce counter */
//               SwitchData[id].bounce = 0;
               SwitchData[id].bounce =TicksLeft;
               SwitchData[id].machineState = DEBOUNCE_ASSERTING_STATE;
            }
            break;
         case DEBOUNCE_ASSERTING_STATE:
            /* switch may be changing to asserted state, wait for debounce */
            if( !SWITCH_ASSERTED(id) )
            {
               /* switch not asserted after all */
               SwitchData[id].machineState = NORMAL_NEGATED_STATE;
            }
            else if( (TicksLeft -SwitchData[id].bounce) >BOUNCE_COUNTS )
            {
               /* debounced, switch is asserted */
//               SwitchData[id].start = 0;
               SwitchData[id].bounce =TicksLeft;
               SwitchData[id].machineState = ASSERTED_STATE;
            }
            break;
         case ASSERTED_STATE:
            /* switch has just been asserted, measure how long it is asserted */
            if( !SWITCH_ASSERTED(id) )
            {
               SwitchData[id].bounce = 0;
               /* switch looks like its being negated */
               SwitchData[id].machineState = DEBOUNCE_NEGATING_STATE;
            }
            else if( (TicksLeft - SwitchData[id].bounce) >BOUNCE_COUNTS )
            {
               /* switch just detected as being held asserted */
               /* invoke callback */
               if( SwitchData[id].callBackPtr3 != NULL_PTR )
               {
                  SwitchData[id].callBackPtr3();
               }

               SwitchData[id].start =TicksLeft;
               SwitchData[id].assertCounts =0;
               SwitchData[id].machineState = HELD_ASSERTED_STATE;
            }
            break;
         case DEBOUNCE_NEGATING_STATE:
            /* switch may be negating */
            if( SWITCH_ASSERTED(id) )
            {
               /* bounce detected */
               SwitchData[id].start =0;// += SwitchData[id].bounce;

               SwitchData[id].machineState = ASSERTED_STATE;
            }
            else if( ++SwitchData[id].bounce > BOUNCE_COUNTS )
            {
               /* switch went to negated state */
               /* invoke callback */
               if( SwitchData[id].callBackPtr1 != NULL_PTR )
               {
                  SwitchData[id].callBackPtr1();
               }

               SwitchData[id].machineState = NORMAL_NEGATED_STATE;
            }
            break;
         case HELD_ASSERTED_STATE:
            /* switch is being held in asserted */
            if( !SWITCH_ASSERTED(id) )
            {
               /* switch looks like its negating */
               SwitchData[id].bounce = 0;
               SwitchData[id].machineState = DEBOUNCE_RELEASING_HELD_STATE;
            }
//            else if( ++SwitchData[id].start % REPEAT_COUNTS == 0)
            else if( (TicksLeft-SwitchData[id].start) >250 )
            {
               /* invoke callback periodically since switch held asserted */
               if( SwitchData[id].callBackPtr2 != NULL_PTR )                 
               {
                  SwitchData[id].callBackPtr2();
               }
               SwitchData[id].start =TicksLeft;
               
               if( ++SwitchData[id].assertCounts >HELD_ASSERT_COUNTS )
                  SwitchData[id].machineState =HELD_ASSERTED_NEW_RATE_STATE;
            }
            break;
         case HELD_ASSERTED_NEW_RATE_STATE:
            /* switch is being held in asserted */
            if( !SWITCH_ASSERTED(id) )
            {
               /* switch looks like its negating */
               SwitchData[id].bounce = 0;
               SwitchData[id].machineState = DEBOUNCE_RELEASING_HELD_STATE;
            }
            //else if( ++SwitchData[id].start % REPEAT_NEW_RATE_COUNTS == 0)
            else if( (TicksLeft-SwitchData[id].start) >20 )              
            {
               /* invoke callback periodically since switch held asserted */
               if( SwitchData[id].callBackPtr2 != NULL_PTR )
               {
                  // maybe add a new callback ( *callBackPtr5 )
                  SwitchData[id].callBackPtr2();
                  
                  SwitchData[id].start =TicksLeft;
               }
            }
            break;
         case DEBOUNCE_RELEASING_HELD_STATE:
            /* switch may be negating */
            if( SWITCH_ASSERTED(id) )
            {
               /* bounce detected */
               if( SwitchData[id].assertCounts >=HELD_ASSERT_COUNTS )
                  SwitchData[id].machineState =HELD_ASSERTED_NEW_RATE_STATE;
               else
                  SwitchData[id].machineState =HELD_ASSERTED_STATE;

               SwitchData[id].start += SwitchData[id].bounce;
            }
            else if( ++SwitchData[id].bounce > BOUNCE_COUNTS )
            {
               /* switch went to negated state */
               /* invoke callback for 'held' state being exited */
               if( SwitchData[id].callBackPtr4 != NULL_PTR )
               {
                  SwitchData[id].callBackPtr4();
               }

               SwitchData[id].machineState = NORMAL_NEGATED_STATE;
            }
            break;
         default:
            break;
      }

} /* end SwitchMachine()  */


/*
*|----------------------------------------------------------------------------
*|  Module: switchFd
*|  Routine:  GetSwitchStatus
*|  Description: Assert high/Negate low
*|
*|----------------------------------------------------------------------------
*/
BOOL GetSwitchStatus
(
   UINT8 switchId_
)
{
   UINT32 switchMask;
   switchMask =(1<<15);
   
   switchMask =(switchMask <<switchId_);
   
   if( (AT91C_BASE_PIOB->PIO_PDSR & switchMask ) ==switchMask ) 
      return false;
   else 
      return true;     
}


/* switchFd.c */










