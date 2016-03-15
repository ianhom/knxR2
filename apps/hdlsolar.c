/**
 * Copyright (c) 2015 wimtecc, Karl-Heinz Welter
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
 * hdlsolar.c
 *
 * handle "our" solar collector
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	khw	inception;
 * 2016-03-04	PA2	khw	adapted to latest changes;
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

typedef	enum	modeSolar	{
			MODE_INVALID	=	-1
		,	MODE_STOPPED	=	0
		,	MODE_WATER	=	1
		,	MODE_BUFFER	=	2
	}	modeSolar ;

#define	TEMP_WW_ON	50
#define	TEMP_WW_OFF	58
#define	TEMP_HB_ON	30
#define	TEMP_HB_OFF	35

extern	void	setModeStopped( eibHdl *, node *) ;
extern	void	setModeWater( eibHdl *, node *) ;
extern	void	setModeBuffer( eibHdl *, node *) ;

extern	void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;
knxLogHdl	*myKnxLogger ;
modeSolar	currentMode ;

int	pumpSolar ;
int	valveSolar ;

char    *modeText[3]    =       {
		"not working"
	,       "heating water tank"
	,       "heating buffer"
	} ;

int	main( int argc, char *argv[]) {
		eibHdl	*myEIB ;
		int	status		=	0 ;
		int	opt ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	/**
	 * define application specific variables
	 */
		float	tempWWOn	=	TEMP_WW_ON ;	// low temp. when water heating needs to start
		float	tempWWOff	=	TEMP_WW_OFF ;	// high temp. when water heating can stop
		float	tempHBOn	=	TEMP_HB_ON ;	// low temp. when buffer heating needs to start
		float	tempHBOff	=	TEMP_HB_OFF ;	// high temp. when buffer heating can stop
		float	diffTempCollHB ;
		float	diffTempCollWW ;
		float	tempWW ;
		float	tempHB ;
		float	tempCol ;
		int	lastMode	=	MODE_INVALID ;
		int	mode	=	MODE_STOPPED ;
		int	changeMode ;
		int	hdlWater	=	0 ;
		int	hdlBuffer	=	0 ;
		int	startupDelay	=	5 ;
		int	tempWWu ;				// point to node["TempWWu"], WarmWater
		int	tempHBu ;				// point to node["TempHBu"], HeatingBuffer
		int	tempCol1 ;				// point to node["TempCol1"], SolarCollector
		int	queueKey	=	10031 ;
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
	 * define shared memory segment #3: cross-referecne table
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
	_debug( 0, progName, "starting up ...") ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "D:Q:d:wb?")) != -1) {
		switch ( opt) {
		/**
		 * general options
		 */
		case    'D'     :
			debugLevel      =       atoi( optarg) ;
			break ;
		case    'Q'     :
			queueKey        =       atoi( optarg) ;
			break ;
		/**
		 * application specific options
		 */
		case    'b'     :
			hdlBuffer       =       1 ;
			break ;
		case    'd'     :
			startupDelay    =       atoi( optarg) ;
			break ;
		case    'w'     :
			hdlWater        =       1 ;
			break ;
		case    '?'     :
			help() ;
			exit(0) ;
			break ;
		default :
			help() ;
			exit( -1) ;
			break ;
		}
	}
	myKnxLogger     =       knxLogOpen( 0) ;
	knxLog( myKnxLogger, progName, "starting up ...") ;
	sleep( startupDelay) ;
	/**
	 * setup the shared memory for COMtable
	 */
	_debug( 1, progName, "trying to obtain shared memory COMtable ... ") ;
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
	shmCOMSize      =       sizeTable[ SIZE_COM_TABLE] ;
	shmOPCSize      =       sizeTable[ SIZE_OPC_TABLE] ;
	shmKNXSize      =       sizeTable[ SIZE_KNX_TABLE] ;
	shmCRFSize      =       sizeTable[ SIZE_CRF_TABLE] ;
	/**
	 *
	 */
#include	"_shmblock.c"
	/**
	 * get some indices from the
	 */
	pumpSolar	=	getEntry( data, lastDataIndexC, "PumpSolar") ;
	valveSolar	=	getEntry( data, lastDataIndexC, "ValveSolar") ;
	tempWWu		=	getEntry( data, lastDataIndexC, "TempWWu") ;
	tempHBu		=	getEntry( data, lastDataIndexC, "TempPSu") ;
	tempCol1	=	getEntry( data, lastDataIndexC, "TempCol1") ;
	_debug( 1, progName, "pumpSolar at index ...... : %d", pumpSolar) ;
	_debug( 1, progName, "valveSolar at index ..... : %d", valveSolar) ;
	_debug( 1, progName, "tempWWu at index ........ : %d", tempWWu) ;
	_debug( 1, progName, "tempHBu at index ........ : %d", tempHBu) ;
	_debug( 1, progName, "tempCol1 at index ....... : %d", tempCol1) ;
	/**
	 * try to determine the current mode of the solar-module
	 */
	_debug( 1, progName, "trying to setermine current status") ;
	if ( data[pumpSolar].val.i == 1) {
		if ( data[valveSolar].val.i == VALVE_SOLAR_WW) {
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
	myEIB	=	eibOpen( 0x01234, 0, 10031) ;
	while ( 1) {
		/**
		 * dump all input data for this "MES"
		 */
		if ( debugLevel > 1) {
			dumpData( data, lastDataIndexC, MASK_SOLAR, (void *) floats) ;
		}
		/**
		 *
		 */
		changeMode	=	1 ;
		lastMode	=	mode ;
		tempWW	=	data[tempWWu].val.f ;
		tempHB	=	data[tempHBu].val.f ;
		tempCol	=	data[tempCol1].val.f ;
		diffTempCollWW	=	data[tempCol1].val.f - data[tempWWu].val.f ;
		diffTempCollHB	=	data[tempCol1].val.f - data[tempHBu].val.f ;
		_debug( 1, progName, "current mode .................. : %d:'%s'", currentMode, modeText[currentMode]) ;
		_debug( 1, progName, "temp. tank, actual ............ : %5.1f", tempWW) ;
		_debug( 1, progName, "temp. buffer, actual .......... : %5.1f", tempHB) ;
		_debug( 1, progName, "temp. solCol1, actual ......... : %5.1f %5d", tempCol, data[tempCol1].knxGroupAddr) ;
		_debug( 1, progName, "temp. diff. tank, actual ...... : %5.1f", diffTempCollWW, tempWWOn, tempWWOff) ;
		_debug( 1, progName, "temp. diff. buffer, actual .... : %5.1f", diffTempCollHB, tempWWOn, tempWWOff) ;
		while ( changeMode) {
			changeMode	=	0 ;
			switch( mode) {
			case	MODE_STOPPED	:
				if ( diffTempCollWW >= 10.0 && tempWW < tempWWOff) {
					mode	=	MODE_WATER ;
				} else if ( diffTempCollHB >= 10.0) {
					mode	=	MODE_BUFFER ;
				} else {
					mode	=	MODE_STOPPED ;
				}
				break ;
			case	MODE_WATER	:
				if ( diffTempCollWW <= 5.0 || tempWW >= tempWWOff) {
					changeMode	=	1 ;
					mode	=	MODE_STOPPED ;
				} else {
				}
				break ;
			case	MODE_BUFFER	:
				if ( diffTempCollWW >= 10.0 && tempWW < tempWWOn) {
					mode	=	MODE_STOPPED ;
					changeMode	=	1 ;
				} else if ( data[valveSolar].val.i == VALVE_SOLAR_HB && diffTempCollHB <= 5.0) {
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
		_debug( 1, progName, "going to sleep ... ") ;
		sleep( 5) ;
	}
	knxLog( myKnxLogger, progName, "terminating ...") ;
	knxLogClose( myKnxLogger) ;
	exit( status) ;
}

void	setModeStopped( eibHdl *_myEIB, node *data) {
        int     reset   =       0 ;
        if ( currentMode != MODE_STOPPED) {
                reset   =       1 ;
        } else if ( data[pumpSolar].val.i != 0) {
                knxLog( myKnxLogger, progName, "ALERT ... Solar Heating Setting (on/off) is WRONG ...") ;
                reset   =       1 ;
        } else if ( data[valveSolar].val.i != VALVE_SOLAR_WW) {
                knxLog( myKnxLogger, progName, "ALERT ... Solar Heating Setting (valve) is WRONG ...") ;
                reset   =       1 ;
        }
        if ( reset) {
                knxLog( myKnxLogger, progName, "Setting mode OFF") ;
                eibWriteBit( _myEIB, data[pumpSolar].knxGroupAddr, 0, 0) ;
                eibWriteBit( _myEIB, data[valveSolar].knxGroupAddr, VALVE_SOLAR_WW, 0) ;
                currentMode     =       MODE_STOPPED ;
        }
}

void	setModeWater( eibHdl *_myEIB, node *data) {
        int     reset   =       0 ;
        if ( currentMode != MODE_STOPPED) {
                reset   =       1 ;
        } else if ( data[pumpSolar].val.i != 1) {
                knxLog( myKnxLogger, progName, "ALERT ... Solar Heating Setting (on/off) is WRONG ...") ;
                reset   =       1 ;
        } else if ( data[valveSolar].val.i != VALVE_SOLAR_WW) {
                knxLog( myKnxLogger, progName, "ALERT ... Solar Heating Setting (valve) is WRONG ...") ;
                reset   =       1 ;
        }
        if ( reset) {
                knxLog( myKnxLogger, progName, "Setting mode OFF") ;
                eibWriteBit( _myEIB, data[pumpSolar].knxGroupAddr, 1, 0) ;
                eibWriteBit( _myEIB, data[valveSolar].knxGroupAddr, VALVE_SOLAR_WW, 0) ;
                currentMode     =       MODE_STOPPED ;
        }
}

void	setModeBuffer( eibHdl *_myEIB, node *data) {
        int     reset   =       0 ;
        if ( currentMode != MODE_STOPPED) {
                reset   =       1 ;
        } else if ( data[pumpSolar].val.i != 1) {
                knxLog( myKnxLogger, progName, "ALERT ... Solar Heating Setting (on/off) is WRONG ...") ;
                reset   =       1 ;
        } else if ( data[valveSolar].val.i != VALVE_SOLAR_HB) {
                knxLog( myKnxLogger, progName, "ALERT ... Solar Heating Setting (valve) is WRONG ...") ;
                reset   =       1 ;
        }
        if ( reset) {
                knxLog( myKnxLogger, progName, "Setting mode OFF") ;
                eibWriteBit( _myEIB, data[pumpSolar].knxGroupAddr, 1, 0) ;
                eibWriteBit( _myEIB, data[valveSolar].knxGroupAddr, VALVE_SOLAR_HB, 0) ;
                currentMode     =       MODE_STOPPED ;
        }
}

void	help() {
}
