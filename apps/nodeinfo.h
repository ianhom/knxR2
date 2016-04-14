/**
 *
 */

#define	MY_HW_ADDRES_KNXS	0.0.1
#define	MY_HW_ADDRESS_DEC	1

#define	MASK_SOLAR	0x01
#define	MASK_PELLET	0x02
#define	MASK_HEATPUMP	0x04
#define	MASK_FLOORHEAT	0x08

#define	PUMP_FLOORHEATING	22
#define	PUMP_RADIATOR		23
#define	PUMP_CIRCULATION	25

#define	HEAT_PUMP	64

#define	VALVE_SOLAR_HB	0
#define	VALVE_SOLAR_WW	1

#define	VALVE_PS_HB	0
#define	VALVE_PS_WW	1

#define	VALVE_HP	21		
#define	VALVE_HP_WW	1
#define	VALVE_HP_HB	0

/**
 * define the indices of the various size infos in the COMtable shaered memory segment
 */

#define	SIZE_COM_TABLE		0
#define	SIZE_OPC_TABLE		1
#define	SIZE_KNX_TABLE		2
#define	SIZE_CRF_TABLE		3

/**
 * define the keys for the various shaered memory segments and message queues
 */

#define	SHM_COM_KEY	10000
#define	SHM_OPC_KEY	10001
#define	SHM_KNX_KEY	10002
#define	SHM_CRF_KEY	10003
#define	SHM_MSG_KEY	10004

/**
 * define macro to determine the highest index of the data table
 */

#define	lastDataIndexC	( shmOPCSize / sizeof( node))
