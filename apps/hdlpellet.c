/**
 * Copyright (c) 2015, 2016 wimtecc, Karl-Heinz Welter
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/**
 *
 * hdlpellet.c
 *
 * handling for "our" pellet stove
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	khw	inception;
 * 2016-01-12	PA2	khw	adapted to new architecture of eib.c;
 * 2016-01-25	PA3	khw	added option for waiting time after startup;
 * 2016-03-30	PA4	khw	added timing per day;
 *
 */
#include	<stdio.h>
#include	<string.h>
#include	<strings.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<time.h>
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<sys/msg.h>

#include	"eib.h"
#include	"debug.h"
#include	"nodeinfo.h"
#include	"mylib.h"
#include	"knxlog.h"
#include	"inilib.h"

typedef	enum	modePS	{
			MODE_INVALID	=	-1
		,	MODE_STOPPED	=	0
		,	MODE_WATER	=	1
		,	MODE_BUFFER	=	2
	}	modePS ;

#define	GAP_OFFON	600		// may not be switched on before this amount of seconds has elapsed
#define	GAP_ONOFF	600		// may not be switched off before this amount of seconds has elapsed

#define	TEMP_WW_ON	53
#define	TEMP_WW_OFF	58
#define	TEMP_HB_ON	33
#define	TEMP_HB_OFF	36

extern	void	setModeStopped( eibHdl *, node *) ;
extern	void	setModeWater( eibHdl *, node *) ;
extern	void	setModeBuffer( eibHdl *, node *) ;

extern	void	help() ;

char	progName[64] ;
knxLogHdl	*myKnxLogger ;
modePS	currentMode ;

int	pelletStove ;
int	valvePelletStove ;

char	*modeText[3]	=	{
		"not working"
	,	"heating water tank"
	,	"heating buffer"
	} ;
/**
 *
 */
int	cfgQueueKey	=	10031 ;
int	cfgSenderAddr	=	1 ;
int	cfgConsiderTime	=	1 ;
/**
 *
 */
void	iniCallback( char *_block, char *_para, char *_value) {
	_debug( 1, progName, "receive ini value block/paramater/value ... : %s/%s/%s\n", _block, _para, _value) ;
	if ( strcmp( _block, "[knxglobals]") == 0) {
		if ( strcmp( _para, "queueKey") == 0) {
			cfgQueueKey	=	atoi( _value) ;
		}
	} else if ( strcmp( _block, "[hdlpellet]") == 0) {
		if ( strcmp( _para, "senderAddr") == 0) {
			cfgSenderAddr	=	atoi( _value) ;
		} else if ( strcmp( _para, "considerTime") == 0) {
			cfgConsiderTime	=	atoi( _value) ;
		}
	}
}
/**
 *
 */
int	main( int argc, char *argv[]) {
		eibHdl	*myEIB ;
		int	status		=	0 ;
		int		opt ;
		char	timeBuffer[64] ;
	/**
	 *
	 */
		float	tempWWOn	=	TEMP_WW_ON ;	// low temp. when water heating
								// needs to start
		float	tempWWOff	=	TEMP_WW_OFF ;	// high temp. when water heating
								// can stop
		float	tempHBOn	=	TEMP_HB_ON ;	// low temp. when buffer heating
								// needs to start
		float	tempHBOff	=	TEMP_HB_OFF ;	// high temp. when buffer heating
								// can stop
		float	tempWW ;
		float	tempHB ;
		int	lastMode	=	MODE_INVALID ;
		int	mode	=	MODE_STOPPED ;
		int	changeMode ;
		int	hdlWater	=	0 ;		// handle Water Tank
		int	hdlBuffer	=	0 ;		// handle Buffer
		int	startupDelay	=	5 ;		// default 0 seconds startup delay
		int	tempWWu ;				// point to node["TempWWu"],
								// WarmWater
		int	tempHBu ;				// point to node["TempHBu"],
								// HeatingBuffer
		time_t	lastOffTime	=	0L ;
		time_t	lastOnTime	=	0L ;
	/**
	 * define shared memory segment #0: COM Table
 	 */
		key_t	shmCOMKey	=	SHM_COM_KEY ;
		int	shmCOMFlg	=	IPC_CREAT | 0600 ;
		int	shmCOMId ;
		int	shmCOMSize	=	256 ;
		int	*sizeTable ;
	/**
	 * define shared memory segment #1: OPC Table with buffer
 	 */
		key_t	shmOPCKey	=	SHM_OPC_KEY ;
		int	shmOPCFlg	=	IPC_CREAT | 0600 ;
		int	shmOPCId ;
		int	shmOPCSize ;
		node	*data ;
	/**
	 * define shared memory segment #2: KNX Table with buffer
 	 */
		key_t	shmKNXKey	=	SHM_KNX_KEY ;
		int	shmKNXFlg	=	IPC_CREAT | 0600 ;
		int	shmKNXId ;
		int	shmKNXSize	=	65536 * sizeof( float) ;
		float	*floats ;
		int	*ints ;
	/**
	 * define shared memory segment #3: Cross-Reference table
 	 */
			key_t		shmCRFKey	=	SHM_CRF_KEY ;
			int		shmCRFFlg	=	IPC_CREAT | 0600 ;
			int		shmCRFId ;
			int		shmCRFSize	=	65536 * sizeof( int) ;
			int		*crf ;
			time_t		actTime ;
	struct	tm			myTime ;
			int		timerMode	=	0 ;
			char		iniFilename[]	=	"knx.ini" ;
	/**
	 *
	 */
	strcpy( progName, *argv) ;
	_debug( 0, progName, "starting up ...") ;
	/**
	 *
	 */
	iniFromFile( iniFilename, iniCallback) ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "D:Q:d:wb?")) != -1) {
		switch ( opt) {
		/**
		 * general options
		 */
		case	'D'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'Q'	:
			cfgQueueKey	=	atoi( optarg) ;
			break ;
		/**
		 * application specific options
		 */
		case	'b'	:
			hdlBuffer	=	1 ;
			break ;
		case	'd'	:
			startupDelay	=	atoi( optarg) ;
			break ;
		case	'w'	:
			hdlWater	=	1 ;
			break ;
		case	'?'	:
			help() ;
			exit(0) ;
			break ;
		default	:
			help() ;
			exit( -1) ;
			break ;
		}
	}
	myKnxLogger	=	knxLogOpen( 0) ;
	knxLog( myKnxLogger, progName, "starting up ...") ;
	sleep( startupDelay) ;
	/**
	 * get and attach the shared memory for COMtable
	 */
	_debug( 1, progName, "trying to obtain shared memory COMtable ...") ;
	if (( shmCOMId = shmget( shmCOMKey, shmCOMSize, 0600)) < 0) {
		_debug( 0, progName, "shmget failed for COMtable");
		_debug( 0, progName, "Exiting with -1");
		exit( -1) ;
	}
	_debug( 1, progName, "trying to attach shared memory for COMtable") ;
	if (( sizeTable = (int *) shmat( shmCOMId, NULL, 0)) == (int *) -1) {
		_debug( 0, progName, "shmat failed for COMtable");
		_debug( 0, progName, "Exiting with -1");
		exit( -1) ;
	}
	shmCOMSize	=	sizeTable[ SIZE_COM_TABLE] ;
	shmOPCSize	=	sizeTable[ SIZE_OPC_TABLE] ;
	shmKNXSize	=	sizeTable[ SIZE_KNX_TABLE] ;
	shmCRFSize	=	sizeTable[ SIZE_CRF_TABLE] ;
	/**
	 *
	 */
#include	"_shmblock.c"
	/**
	 * get some indices from the
	 */
	pelletStove		=	getEntry( data, lastDataIndexC, "PelletStove") ;
	valvePelletStove	=	getEntry( data, lastDataIndexC, "ValvePelletStove") ;
	_debug( 1, progName, "pelletStove at index .......... : %d (knx: %05d)", pelletStove, data[pelletStove].knxGroupAddr) ;
	_debug( 1, progName, "valvePelletStove at index ..... : %d (knx: %05d)", valvePelletStove, data[valvePelletStove].knxGroupAddr) ;
	tempWWu		=	getEntry( data, lastDataIndexC, "TempWWu") ;
	tempHBu		=	getEntry( data, lastDataIndexC, "TempPSu") ;
	/**
	 * try to determine the current mode of the pellet-module
	 */
	_debug( 1, progName, "trying to determine current status") ;
	if ( data[pelletStove].val.i == 1) {
		if ( data[valvePelletStove].val.i == VALVE_PS_WW) {
			mode	=	MODE_WATER ;
		} else {
			mode	=	MODE_BUFFER ;
		}
	} else {
		mode	=	MODE_STOPPED ;
	}
	currentMode	=	mode ;
	_debug( 1, progName, "current status ... %0d:'%s'", currentMode, modeText[currentMode]) ;
	/**
	 *
	 */
	myEIB	=	eibOpen( cfgSenderAddr, 0, cfgQueueKey, progName, APN_WRONLY) ;
	while ( debugLevel >= 0) {
		/**
		 *
		 */
		if ( cfgConsiderTime != 0) {
			actTime	=	time( NULL) ;
			myTime	=	*localtime( &actTime) ;
			timerMode	=	0 ;
			switch ( myTime.tm_wday) {
			case	6	:		// saturday
				if ( myTime.tm_hour >= 5 && myTime.tm_hour <= 12) {
					timerMode	=	1 ;
				}
				break ;
			case	0	:		// sunday
				if ( myTime.tm_hour >= 7 && myTime.tm_hour <= 12) {
					timerMode	=	1 ;
				}
				break ;
			default	:			// monday - friday
				if ( myTime.tm_hour >= 4 && myTime.tm_hour <= 8) {
					timerMode	=	1 ;
				} else if ( myTime.tm_hour >= 19 && myTime.tm_hour <= 21) {
					timerMode	=	1 ;
				}
				break ;
			}
			_debug( 1, progName, "timer mode .................... : %d", timerMode) ;
		} else {
			_debug( 1, progName, "timer mode .................... : not considered", timerMode) ;
			timerMode	=	1 ;
		}
		/**
		 *
		 */
		changeMode	=	1 ;
		lastMode	=	mode ;
		tempWW	=	data[tempWWu].val.f ;
		tempHB	=	data[tempHBu].val.f ;
		_debug( 1, progName, "week day (0= sun, ... 6=sat)... : %d", myTime.tm_wday) ;
		_debug( 1, progName, "hour .......................... : %02d:%02d", myTime.tm_hour, myTime.tm_min) ;
		_debug( 1, progName, "timer mode .................... : %d", timerMode) ;
		_debug( 1, progName, "current mode .................. : %d:'%s'", currentMode, modeText[currentMode]) ;
		_debug( 1, progName, "temp. warm water, actual ...... : %5.1f ( %5.1f ... %5.1f)", tempWW, tempWWOn, tempWWOff) ;
		_debug( 1, progName, "temp. buffer, actual .......... : %5.1f ( %5.1f ... %5.1f)", tempHB, tempHBOn, tempHBOff) ;
		while ( changeMode) {
			changeMode	=	0 ;
			switch( mode) {
			case	MODE_STOPPED	:
				if ( tempWW <= tempWWOn && hdlWater && timerMode == 1) {
					mode	=	MODE_WATER ;
				} else if ( tempHB <= tempHBOn && hdlBuffer) {
					mode	=	MODE_BUFFER ;
				} else {
					mode	=	MODE_STOPPED ;
				}
				break ;
			case	MODE_WATER	:
				if ( tempWW >= tempWWOff || timerMode == 0) {
					changeMode	=	1 ;
					mode	=	MODE_STOPPED ;
				}
				break ;
			case	MODE_BUFFER	:
				if ( tempWW <= tempWWOn && hdlWater) {
					mode	=	MODE_STOPPED ;
					changeMode	=	1 ;
				} else if ( tempHB >= tempHBOff && hdlBuffer) {
					mode	=	MODE_STOPPED ;
					changeMode	=	1 ;
				}
				break ;
			}
		}
		lastMode	=	mode ;
		switch ( mode) {
		case	MODE_STOPPED	:
			setModeStopped( myEIB, data) ;
			break ;
		case	MODE_WATER	:
			setModeWater( myEIB, data) ;
			break ;
		case	MODE_BUFFER	:
			setModeBuffer( myEIB, data) ;
			break ;
		}
		_debug( 1, progName, "going to sleep ... \n") ;
		sleep( 5) ;
	}
	knxLog( myKnxLogger, progName, "terminating ...") ;
	knxLogClose( myKnxLogger) ;
	exit( status) ;
}

void	setModeStopped( eibHdl *_myEIB, node *data) {
	int	reset	=	0 ;
	if ( currentMode != MODE_STOPPED) {
		reset	=	1 ;
	} else if ( data[pelletStove].val.i != 0) {
		_debug( 1, progName, "ALERT ... Pellet Stove Setting (on/off) is WRONG ...") ;
		knxLog( myKnxLogger, progName, "ALERT ... Pellet Stove Setting (on/off) is WRONG ...") ;
		reset	=	1 ;
	} else if ( data[valvePelletStove].val.i != VALVE_PS_WW) {
		_debug( 1, progName, "ALERT ... Pellet Stove Setting (valve) is WRONG WRONG ...") ;
		knxLog( myKnxLogger, progName, "ALERT ... Pellet Stove Setting (valve) is WRONG ...") ;
		reset	=	1 ;
	}
	if ( reset) {
		knxLog( myKnxLogger, progName, "Setting mode OFF") ;
		eibWriteBit( _myEIB, data[pelletStove].knxGroupAddr, 0, 0) ;
		sleep( 1) ;
		eibWriteBit( _myEIB, data[valvePelletStove].knxGroupAddr, VALVE_PS_WW, 0) ;
		currentMode	=	MODE_STOPPED ;
	}
}

void	setModeWater( eibHdl *_myEIB, node *data) {
	int	reset	=	0 ;
	if ( currentMode != MODE_WATER) {
		reset	=	1 ;
	} else if ( data[pelletStove].val.i != 1 || data[valvePelletStove].val.i != VALVE_PS_WW) {
		_debug( 1, progName, "ALERT ... Pellet Stove Settings are WRONG ...") ;
		knxLog( myKnxLogger, progName, "ALERT ... Pellet Stove Settings are WRONG ...") ;
		reset	=	1 ;
	}
	if ( reset) {
		knxLog( myKnxLogger, progName, "Setting mode WATER") ;
		eibWriteBit( _myEIB, data[pelletStove].knxGroupAddr, 1, 0) ;
		sleep( 1) ;
		eibWriteBit( _myEIB, data[valvePelletStove].knxGroupAddr, VALVE_PS_WW, 0) ;
		currentMode	=	MODE_WATER ;
	}
}

void	setModeBuffer( eibHdl *_myEIB, node *data) {
	int	reset	=	0 ;
	if ( currentMode != MODE_BUFFER) {
		reset	=	1 ;
	} else if ( data[pelletStove].val.i != 1 || data[valvePelletStove].val.i != VALVE_PS_HB) {
		_debug( 1, progName, "ALERT ... Pellet Stove Settings are WRONG ...") ;
		knxLog( myKnxLogger, progName, "ALERT ... Pellet Stove Settings are WRONG ...") ;
		reset	=	1 ;
	}
	if ( reset) {
		knxLog( myKnxLogger, progName, "Setting mode BUFFER") ;
		eibWriteBit( _myEIB, data[pelletStove].knxGroupAddr, 1, 0) ;
		sleep( 1) ;
		eibWriteBit( _myEIB, data[valvePelletStove].knxGroupAddr, VALVE_PS_HB, 0) ;
		currentMode	=	MODE_BUFFER ;
	}
}

void	help() {
	printf( "%s: %s -d=<debugLevel> -w -b \n\n", progName, progName) ;
	printf( "-w handle water tank\n") ;
	printf( "-b handle heating buffer\n") ;
}
