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
#ifndef EXTERNAL_ADC_FD_H
 #define EXTERNAL_ADC_FD_H



/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "mk_std.h"

/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
enum
{
   ADC_SUCTION_PRESS =0,
   ADC_DISCHARGE_PRESS,
   ADC_SPARE1,
   ADC_SPARE2,   
   ADC_GAS_TEMPERATURE,
   ADC_ENGINE_VIBRATION,
   ADC_COMP_VIBRATION,
   ADC_SPARE3,
   ADC_SPARE4,
   ADC_SPARE5,
   ADC_SPARE6,
   ADC_SPARE7,
   ADC_SPARE8,   
   ADC_COMPR_DC_TEMPERATURE,
   ADC_SPARE9,
   ADC_ENGINE_TEMPERATURE     

};

/*
*****************************************************************************
 P U B L I C    M A C R O S
*****************************************************************************
*/
#define ADC_16_BIT  (65536)
#define ADC_12_BIT  (4096)

#define SUCTION_PRESSURE   (float)( (AdcReading[ADC_SUCTION_PRESS]/(float)3145.728)   -4)*6.25 //(float)( (AdcReading[ADC_SUCTION_PRESS]/(float)196.608)   -4)*6.25
#define DISCHARGE_PRESSURE (float)( (AdcReading[ADC_DISCHARGE_PRESS]/(float)3145.728) -4)*18.75 //(float)( (AdcReading[ADC_DISCHARGE_PRESS]/(float)196.608) -4)*18.75
#define ENGINE_TEMP        (float)( (AdcReading[ADC_ENGINE_TEMPERATURE]/(float)163.84) -273 )*1.8 +32    // 1uA/Kelvin //(float)( (AdcReading[ADC_ENGINE_TEMPERATURE]/(float)10.24) -273 )*1.8 +32    // 1uA/Kelvin
#define COMPR_DC_TEMP      (float)( (AdcReading[ADC_COMPR_DC_TEMPERATURE]/(float)163.84) -273 )*1.8 +32  // 1uA/Kelvin //(float)( (AdcReading[ADC_COMPR_DC_TEMPERATURE]/(float)10.24) -273 )*1.8 +32  // 1uA/Kelvin
#define GAS_TEMP           AdcReading[ADC_GAS_TEMPERATURE]
#define ENGINE_VIBRATION   AdcReadingm[ADC_ENGINE_VIBRATION]
#define COMP_VIBRATION     AdcReading[ADC_COMP_VIBRATION]
#define SPARE              AdcReading[ADC_SPARE1]

#define SUCTION_PRESSURE_RAW    AdcReading[ADC_SUCTION_PRESS]
#define DISCHARGE_PRESSURE_RAW  AdcReading[ADC_DISCHARGE_PRESS]
#define ENGINE_TEMP_RAW         AdcReading[ADC_ENGINE_TEMPERATURE]
#define COMPR_DC_TEMP_RAW       AdcReading[ADC_COMPR_DC_TEMPERATURE]


#define ADC_READING9   AdcReading[ADC_SPARE2]
#define ADC_READING10  AdcReading[ADC_SPARE3]
#define ADC_READING11  AdcReading[ADC_SPARE4]
#define ADC_READING12  AdcReading[ADC_SPARE5]
#define ADC_READING13  AdcReading[ADC_SPARE6]
#define ADC_READING14  AdcReading[ADC_SPARE7]
#define ADC_READING15  AdcReading[ADC_SPARE8]
#define ADC_READING16  AdcReading[ADC_SPARE9]


#define PSI_TO_KPA         ((float)6.895)
#define PSI_TO_KPA_ROUNDED ((float)7.0)

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
extern UINT16 AdcReading[16];

/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/

void InitExtAdc
(
   void
);

/*
*|----------------------------------------------------------------------------
*|  Module: extAdcFd
*|  Routine: MeasureExtAnalog
*|  Description:
*|----------------------------------------------------------------------------
*/
UINT16 MeasureExtAnalog
(
   UINT8 channel_
);

void GetAllAnalog
(
   UINT16 *
);


#endif




