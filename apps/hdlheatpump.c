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
 * hdlheatpump.c
 *
 * handling for "our" pellet stove
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-12-14	PA1	khw	copied form hdlpellet.c and modified;
 * 2016-01-12	PA2	khw	adapted to new architecture of eib.c;
 * 2016-01-25	PA2	khw	added option for waiting time after startup;
 *
 */
#include	<stdio.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>
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

typedef	enum	modeHP	{
			MODE_INVALID	=	-1
		,	MODE_STOPPED	=	0
		,	MODE_WATER	=	1
		,	MODE_BUFFER	=	2
	}	modeHP ;

#define	GAP_OFFON	600		// may not be switched on before this amount of seconds has elapsed
#define	GAP_ONOFF	600		// may not be switched off before this amount of seconds has elapsed

#define	TEMP_WW_ON	50
#define	TEMP_WW_OFF	55
#define	TEMP_HB_ON	30
#define	TEMP_HB_OFF	35

extern	void	setModeStopped( eibHdl *, node *) ;
extern	void	setModeWater( eibHdl *, node *) ;
extern	void	setModeBuffer( eibHdl *, node *) ;

extern	void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;
knxLogHdl	*myKnxLogger ;
int	currentMode ;

int	heatPump ;
int	valvePelletStove ;

int	main( int argc, char *argv[]) {
		eibHdl	*myEIB ;
		int	status		=	0 ;
			int		opt ;
		time_t	actTime ;
	struct	tm	*myTime ;
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
	 * define shared memory segment #2: KNX Table with buffer
 	 */
		key_t	shmCRFKey	=	SHM_CRF_KEY ;
		int	shmCRFFlg	=	IPC_CREAT | 0600 ;
		int	shmCRFId ;
		int	shmCRFSize	=	65536 * sizeof( int) ;
		int	*crf ;
	/**
	 *
	 */
	strcpy( progName, *argv) ;
	printf( "%s: starting up ... \n", progName) ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "D:d:wb?")) != -1) {
		switch ( opt) {
		case	'D'	:
			startupDelay	=	atoi( optarg) ;
			break ;
		case	'd'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'w'	:
			hdlWater	=	1 ;
			break ;
		case	'b'	:
			hdlBuffer	=	1 ;
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
	 * setup the shared memory for COMtable
	 */
	printf( "trying to obtain shared memory COMtable ... \n") ;
	if (( shmCOMId = shmget( shmCOMKey, shmCOMSize, 0600)) < 0) {
		perror( "knxmon: shmget failed for COMtable");
		exit( -1) ;
	}
	printf( "trying to attach shared memory for COMtable \n") ;
	if (( sizeTable = (int *) shmat( shmCOMId, NULL, 0)) == (int *) -1) {
		perror( "knxmon: shmat failed for COMtable");
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
	heatPump		=	getEntry( data, lastDataIndexC, "HeatPump") ;
	valvePelletStove	=	getEntry( data, lastDataIndexC, "ValvePelletStove") ;
	_debug( 1, progName, "Heatpump at index ..............: %d", heatPump) ;
	_debug( 1, progName, "valvePelletStove at index ......: %d", valvePelletStove) ;
	tempWWu		=	getEntry( data, lastDataIndexC, "TempWWu") ;
	tempHBu		=	getEntry( data, lastDataIndexC, "TempPSu") ;
	/**
	 * try to determine the current mode of the pellet-module
	 */
	printf( "trying to determine current status\n") ;
	if ( data[heatPump].val.i == 1) {
		if ( data[valvePelletStove].val.i == VALVE_PS_WW) {
			mode	=	MODE_BUFFER ;
		} else {
			mode	=	MODE_WATER ;
		}
	} else {
		mode	=	MODE_STOPPED ;
	}
	currentMode	=	mode ;
	/**
	 *
	 */
	while ( 1) {
		/**
		 * dump all input data for this "MES"
		 */
		if ( debugLevel > 1) {
			dumpData( data, lastDataIndexC, MASK_PELLET, (void *) floats) ;
		}
		/**
		 *
		 */
		changeMode	=	1 ;
		lastMode	=	mode ;
		tempWW	=	data[tempWWu].val.f ;
		tempHB	=	data[tempHBu].val.f ;
		while ( changeMode) {
			changeMode	=	0 ;
			switch( mode) {
			case	MODE_STOPPED	:
				if ( tempWW <= tempWWOn && hdlWater) {
					mode	=	MODE_WATER ;
				} else if ( tempHB <= tempHBOn && hdlBuffer) {
					mode	=	MODE_BUFFER ;
				} else {
					mode	=	MODE_STOPPED ;
				}
				break ;
			case	MODE_WATER	:
				if ( tempWW >= tempWWOff) {
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
	} else if ( data[heatPump].val.i != 0) {
		knxLog( myKnxLogger, progName, "ALERT ... Pellet Stove Settings are WRONG ...") ;
		reset	=	1 ;
	}
	if ( reset) {
		knxLog( myKnxLogger, progName, "Setting mode OFF") ;
		eibWriteBit( _myEIB, data[heatPump].knxGroupAddr, 0, 1) ;
		currentMode	=	MODE_STOPPED ;
	}
}

void	setModeWater( eibHdl *_myEIB, node *data) {
	int	reset	=	0 ;
	if ( currentMode != MODE_WATER) {
		reset	=	1 ;
	} else if ( data[heatPump].val.i != 1 || data[valvePelletStove].val.i != VALVE_PS_WW) {
		knxLog( myKnxLogger, progName, "ALERT ... Pellet Stove Settings are WRONG ...") ;
		reset	=	1 ;
	}
	if ( reset) {
		knxLog( myKnxLogger, progName, "Setting mode WATER") ;
		eibWriteBit( _myEIB, data[heatPump].knxGroupAddr, 1, 1) ;
		currentMode	=	MODE_WATER ;
	}
}

void	setModeBuffer( eibHdl *_myEIB, node *data) {
	int	reset	=	0 ;
	if ( currentMode != MODE_BUFFER) {
		reset	=	1 ;
	} else if ( data[heatPump].val.i != 1 || data[valvePelletStove].val.i != VALVE_PS_HB) {
		knxLog( myKnxLogger, progName, "ALERT ... Pellet Stove Settings are WRONG ...") ;
		reset	=	1 ;
	}
	if ( reset) {
		knxLog( myKnxLogger, progName, "Setting mode BUFFER") ;
		eibWriteBit( _myEIB, data[heatPump].knxGroupAddr, 1, 1) ;
		currentMode	=	MODE_BUFFER ;
	}
}

void	help() {
	printf( "%s: %s -d=<debugLevel> -w -b \n\n", progName, progName) ;
	printf( "-w handle water tank\n") ;
	printf( "-b handle heating buffer\n") ;
}
