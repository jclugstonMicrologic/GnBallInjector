
/*****************************************************************************/
/* D E F I N I T I O N S                                                     */
/*****************************************************************************/
#define UPDOWNCHAR        ((UINT8)0x00)
#define ONE_SECOND_TICKS ((UINT32)977) /*(977*1.024mSec)=1000.45mSec, app.=1second */
#define DUTY_FULL_SPEED 50
#define MOTOR_FREQ 3
#define NO_START 10
#define COUNT 7
#define RUNNING 0
#define NOT_RUNNING 1

#define MAX_BALL_COUNT 255 //200
#define MAX_MOTOR_RATE 250
#define MIN_MOTOR_RATE 1

/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
typedef struct
{
   UINT8 valid;
   UINT8 slip;  
   UINT8 overshoot; 
}EDGES_DETECTED;

typedef struct
{
   UINT16 valid;
   UINT16 slip;
   UINT8 time;
}VANES_DETECTED;


typedef struct
{
   UINT8 rate;
   UINT8 direction;
   BOOL start;
}MOTOR_PARAMETERS;

typedef struct
{
   MOTOR_PARAMETERS injection;
   MOTOR_PARAMETERS load;   
   MOTOR_PARAMETERS index;
   MOTOR_PARAMETERS test;   
   MOTOR_PARAMETERS manual;
   MOTOR_PARAMETERS jog;   
}MOTOR;

typedef struct
{
   UINT16 speed;
   UINT16 nbrVanes; 
   UINT16 ballsPerMin;    
   UINT32 timeStamp;      
   UINT8 startRequested;
   UINT8 edgeDetect;
   UINT8 machState;  
   UINT8 ballsPerRev;  
   UINT8 vanesPerBall; 
   UINT8 indexState;     
   BOOL indexing;  
   
}MOTOR_CONTROL;

typedef struct
{
   INT16 load;
   INT16 index;
   BOOL increment;     
}BALL_COUNT;

/*****************************************************************************/
/* P U B L I C    M A C R O S                                                */
/*****************************************************************************/

enum
{ 
   SPEED_RATE =0,
   SPEED_POSITION
};               

enum
{            
   TOTAL =0, 
   TIME_LIMIT 
}; 


enum
{            
   MOTOR_FWD=0, 
   MOTOR_REV,
   MOTOR_JAM
}; 


enum
{            
   MODE_IDLE =0,   
   MODE_INJECT, 
   MODE_LOAD,
   MODE_INDEX,
   MODE_TEST,
   MODE_MAN,
   MODE_JOG,
     
   MODE_LAST
     
}; 

enum
{
   INDEX_IDLE_STATE =0,
   INDEX_START_STATE,
   INDEX_STOP_STATE,
   INDEX_COMPLETE_STATE
};


/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
extern EDGES_DETECTED Edge;
extern VANES_DETECTED Vane;

extern BALL_COUNT BallCount;
extern MOTOR Motor;
extern MOTOR_CONTROL MotorControl;

extern volatile INT16 BallCounter;

extern MOTOR_PARAMETERS *ModeSelect;

/*************************************************************************/
/* P R O T O T Y P E S                                                   */
/*************************************************************************/

void turnMotorOn
(
   void
);       

void testSpeed
(
   void
);

void SpeedCompensate
(
   void
);

void MeasureSpeed
(
   void
);

void MotorStrtUpInit
(
   void
); 

BOOL MonitorSlip
(
   UINT8 type_
);

void MotorControlBic
(
   UINT8 speed /* user selected motor speed 1-250 balls/min */
);

void ResetSpeedVar
(
   void
);

void initMotorVariables
(
   void
);

void AdjustMotorSpeed
(
   UINT16
);

void Diagnostics
(
   void
);

void SetNewRate
(
   UINT8 value_
);

void MotorRunMachine
(
   void
);


