/*
    RACAPOUN - RAil CArriage POsition UNit
    -----------------------------------
    (c) 2005 LEVEL Milan Vondracek
    -----------------------------------
    Hardrware configuration header
    -----------------------------------
    This file contains some HW configuration definitions, that is shared in all HW modules
*/

/*
//////////////////////////////////////////////////////////////////////
//Master clock at full speed mode
//////////////////////////////////////////////////////////////////////
// MCK = 47,9232 MHz, = 0,0208 us
// PIT clock is MCK / 16 = 2,9952 Mhz, = 0,333 us
*/
#define MCK (47923200) 


/*
/////////////////////////////////////////////////////////////////////////////////////
// ARM Program Status Register bits
/////////////////////////////////////////////////////////////////////////////////////
*/

#define PSR_Mode_USR       0x10
#define PSR_Mode_FIQ       0x11
#define PSR_Mode_IRQ       0x12
#define PSR_Mode_SVC       0x13
#define PSR_Mode_ABT       0x17
#define PSR_Mode_UND       0x1B
#define PSR_Mode_SYS       0x1F

#define PSR_I_Bit          0x80    /* when I bit is set, IRQ is disabled */
#define PSR_F_Bit          0x40    /* when F bit is set, FIQ is disabled */


/*
/////////////////////////////////////////////////////////////////////////////////////
// Memory configuration
/////////////////////////////////////////////////////////////////////////////////////
*/

#define Flash_Size 				0x00010000
#define Flash_Base 				0x00100000
#define Flash_End  				(Flash_Base+Flash_Size)

#define Ram_Size				0x00004000
#define Ram_Base 				0x00200000
#define Ram_End  				(Ram_Base+Ram_Size)

/*
/////////////////////////////////////////////////////////////////////////////////////
// Stack + exception data configuration
/////////////////////////////////////////////////////////////////////////////////////
*/

#define ExType					(Ram_End-0x04)
#define ExAddr					(Ram_End-0x08)
#define ExAbortStatus			(Ram_End-0x0C)
#define ExAbortAddr				(Ram_End-0x10)
#define	Top_Stack 				(Ram_End-0x14)

#define UND_Stack_Size			0x00000008
#define SVC_Stack_Size 			0x00000008
#define ABT_Stack_Size 			0x00000008
#define FIQ_Stack_Size 			0x00000008
#define IRQ_Stack_Size			0x00000100
#define USR_Stack_Size			0x00000400

/*
/////////////////////////////////////////////////////////////////////////////////////
// Definitiona of several HW registers used in startup
/////////////////////////////////////////////////////////////////////////////////////
*/

#define AIC_BASE				0xFFFFF000
#define AIC_IVR					0x00000100
#define AIC_EOI					0x00000130

