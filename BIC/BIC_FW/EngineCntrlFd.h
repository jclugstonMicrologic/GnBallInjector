/*|***************************************************************************/
/*| PROJECT: Compact Compression CCU                                         */
/*| Module:   EngineCntrlFd                                                  */
/*| Description:                                                             */
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


/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/


/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
typedef struct
{
   UINT16 avg[80];
   UINT32 avgSpeed;
   UINT8 nbrSamples;
   UINT8 smplCnt;
}RUN_AVG;

typedef struct
{
   RUN_AVG runAvg;
   
   UINT16 speed;     
   
   // these are not part of engine control, they are spares
   // to be used in a different structure when the time arrives
  
}ENGINE_CONTROL;


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
extern ENGINE_CONTROL EngineControl;

void EngineCntrl_Init
(
   void
);


void EngineCntrl_Disable
(
   void
);

void EngineCntrl_Speed1Isr
(
   void
);

void EngineCntrl_Speed2Isr
(
   void
);
