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
 * hdlheating.c
 *
 * handler for "our" heating system.
 * this file is not yet
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	userId	inception;
 *
 */
#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<time.h>
#include	<sys/types.h>
#include	<sys/ipc.h> 
#include	<sys/shm.h> 
#include	<sys/msg.h> 

#include	"debug.h"
#include	"knxlog.h"
#include	"nodeinfo.h"
#include	"mylib.h"
#include	"eib.h"

#define	CYCLE_TIME	2		// run in 10-seconds cycles

extern	void	setOff( eibHdl *, int) ;
extern	void	setOn( eibHdl *, int) ;

extern	void	dumpData( node *, int, int, void *) ;
extern	void	dumpValveTable( node *, int *, float *) ;

char	progName[64] ;
int	debugLevel ;
knxLogHdl	*myKnxLogger ;

typedef	struct	{
		int		id ;
		char		name[32] ;
		int		gaTempAct ;
		int		gaTempTarget ;
		int		gaTempComfort ;
		int		gaTempNight ;
		int		gaTempOut ;
		int		gaTempFreeze ;
		int		gaValve ;
		float		temp[4] ;
	}	floorValve ;

floorValve	valves[]	=	{
		{	0, "UG_TECH"	,6657	,6663	,0	,0	,0	,0	,8449	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "UG_OFCL"	,6667	,6673	,0	,0	,0	,0	,8450	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "UG_OFCR"	,6677	,6683	,0	,0	,0	,0	,8451	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "UG_STO1"	,6687	,6693	,0	,0	,0	,0	,8452	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "UG_STO2"	,6697	,6703	,0	,0	,0	,0	,8453	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "UG_HALL"	,6707	,6713	,0	,0	,0	,0	,8454	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "EG_HWR"	,6913	,6919	,0	,0	,0	,0	,8961	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "EG_GUEST"	,6923	,6929	,0	,0	,0	,0	,8962	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "EG_LIV"	,6933	,6939	,0	,0	,0	,0	,8963	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "EG_DIN"	,6943	,6949	,0	,0	,0	,0	,8964	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "EG_KITCH"	,6953	,6959	,0	,0	,0	,0	,8965	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "EG_BATH"	,6963	,6969	,0	,0	,0	,0	,8966	, 7.0, 21.0, 22.0, 19.0}
	,	{	0, "EG_HALL"	,6973	,6979	,0	,0	,0	,0	,8967	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "OG_MBATH"	,7169	,7175	,0	,0	,0	,0	,9217	, 7.0, 21.0, 22.0, 19.0}
	,	{	0, "OG_MBR"	,7179	,7185	,0	,0	,0	,0	,9218	, 7.0, 19.0, 18.0, 19.0}
	,	{	0, "OG_BATH"	,7189	,7195	,0	,0	,0	,0	,9219	, 7.0, 21.0, 22.0, 19.0}
	,	{	0, "OG_BRR"	,7199	,7205	,0	,0	,0	,0	,9220	, 7.0, 19.0, 21.0, 19.0}
	,	{	0, "OG_BRF"	,7209	,7215	,0	,0	,0	,0	,9221	, 7.0, 19.0, 18.0, 19.0}
	,	{	0, "OG_HALL"	,7219	,7225	,0	,0	,0	,0	,9222	, 7.0, 19.0, 20.0, 19.0}
	,	{	-1, ""		,0	,0	,0	,0	,0	,0	,0	, 7.0, 19.0, 21.0, 19.0}
	} ;


extern	void	help() ;

int	main( int argc, char *argv[]) {
		eibHdl	*myEIB ;
		int	status		=	0 ;
		int		opt ;
		int	i ;
		int	comfort ;
		int	night ;
		int	modeCurrent ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	/**
	 *
	 */
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
		char	logMsg[256] ;
	/**
	 *
	 */
	strcpy( progName, *argv) ;
	_debug( 0, progName, "starting up ...") ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "d:?")) != -1) {
		switch ( opt) {
		case	'd'	:
			debugLevel	=	atoi( optarg) ;
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
	/**
	 *
	 */
	myKnxLogger	=	knxLogOpen( 0) ;
	sprintf( logMsg, "%s: starting up ...", progName) ;
	knxLog( myKnxLogger, progName, logMsg) ;
	/**
	 * setup the shared memory for COMtable
	 */
	_debug( 1, progName,  "trying to obtain shared memory COMtable ...") ;
	if (( shmCOMId = shmget( shmCOMKey, shmCOMSize, 0600)) < 0) {
		perror( "knxmon: shmget failed for COMtable");
		exit( -1) ;
	}
	_debug( 1, progName,  "trying to attach shared memory for COMtable") ;
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
	 * open the knx bus
	 */
	comfort		=	getEntry( data, lastDataIndexC, "SwitchCO") ;
	night		=	getEntry( data, lastDataIndexC, "SwitchNI") ;
	myEIB	=	eibOpen( 33, 0x00, 10031) ;
	while ( 1) {
		modeCurrent	=	data[comfort].val.i << 1 | data[night].val.i ;
		_debug( 1, progName, "Day/Night node %02x", modeCurrent) ;
		/**
		 * dump all input data for this "MES"
		 */
		if ( debugLevel >= 1) {
			dumpValveTable( data, ints, floats) ;
		}
		/**
		 *
		 */
		for ( i=0 ; valves[i].id != -1 ; i++) {
			if ( floats[ valves[ i].gaTempAct] > 1.0 && floats[ valves[ i].gaTempTarget] >= 1.0) {
				_debug( 1, progName, "valid target temperature for valve %d", valves[ i].gaValve) ;
				if ( floats[ valves[ i].gaTempAct] >= floats[ valves[ i].gaTempTarget] &&
					ints[ valves[ i].gaValve] != 0) {
					_debug( 1, progName, "switching off valve %d", valves[ i].gaValve) ;
					setOff( myEIB, valves[ i].gaValve) ;
				} else if ( floats[ valves[ i].gaTempAct] < ( floats[ valves[ i].gaTempTarget] - 0.5) &&
					ints[ valves[ i].gaValve] != 1) {
					_debug( 1, progName, "switching on valve %d", valves[ i].gaValve) ;
					setOn( myEIB, valves[ i].gaValve) ;
				}
			} else if ( floats[ valves[ i].gaTempAct] > 1.0) {
				_debug( 1, progName, "NO valid target temperature for valve %d", valves[ i].gaValve) ;
				if ( floats[ valves[ i].gaTempAct] >= valves[ i].temp[modeCurrent] &&
					ints[ valves[ i].gaValve] != 0) {
					_debug( 1, progName, "switching off valve %d", valves[ i].gaValve) ;
					setOff( myEIB, valves[ i].gaValve) ;
				} else if ( floats[ valves[ i].gaTempAct] < ( valves[ i].temp[modeCurrent] - 0.5) &&
					ints[ valves[ i].gaValve] != 1) {
					_debug( 1, progName, "switching on valve %d", valves[ i].gaValve) ;
					setOn( myEIB, valves[ i].gaValve) ;
				}
			} else {
				_debug( 1, progName, "NO valid actual temperature for valve %d", valves[ i].gaValve) ;
				if ( ints[ valves[ i].gaValve] != 0) {
					_debug( 1, progName, "NO valid temperatures for valve %d", valves[ i].gaValve) ;
					setOff( myEIB, valves[ i].gaValve) ;
				}
			}
		}
		/**
		 *
		 */
		sleep( CYCLE_TIME) ;
	}
	eibClose( myEIB) ;
	knxLogClose( myKnxLogger) ;
	exit( status) ;
}

void	dumpValveTable( node *data, int *ints, float *floats) {
	int	i ;
	for ( i=0 ; valves[i].id != -1 ; i++) {
		printf( "%s: %-20s => Actual := %5.2f, Target := %5.2f, Valve := %s\n",
				progName,
				valves[i].name,
				floats[ valves[ i].gaTempAct],
				floats[ valves[ i].gaTempTarget],
				(ints[ valves[ i].gaValve] == 1 ? "on" : "off")
			) ;
	}
}

void	setOff( eibHdl *_myEIB, int groupAddr) {
	_debug( 1, progName,  "... will switch off valve ...") ;
	eibWriteBit( _myEIB, groupAddr, 0, 1) ;
}

void	setOn( eibHdl *_myEIB, int groupAddr) {
	_debug( 1, progName,  "... will switch on valve ...") ;
	eibWriteBit( _myEIB, groupAddr, 1, 1) ;
}

void	help() {
	printf( "%s: %s -d=<debugLevel> -w -b \n\n", progName, progName) ;
	printf( "-w handle water tank\n") ;
	printf( "-b handle heating buffer\n") ;
}
