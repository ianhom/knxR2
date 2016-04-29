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
 * knxbackbone.c
 *
 * EIB/KNX backbone process
 *
 * knxbackbone as such does not do more but create a shared memory segment
 * which serves as the central buffer for all EIB messages.
 * Accessing this central buffer happens through the usage of functions in
 * the "eib" library, which provides all necessary functions for opening
 * and closing connections to the EIB messagign world as well as sending
 * and receiving messages.
 *
 * When a programm wants to gain access to EIB messages, be it for sending,
 * writing or both, it opens a connection to the knx-backbone through a call to
 * "eibOpen".
 *
 *
 * Revision history
 *
 * Date		Rev.	Who	what
 * -----------------------------------------------------------------------------
 * 2016-03-02	PA1	khw	inception;
 * 2016-04-26	PA2	khw	added explanation of the mechanism;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<unistd.h>
#include	<time.h>
#include	<math.h>
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/signal.h>

#include	"debug.h"
#include	"knxlog.h"
#include	"rs232.h"
#include	"eib.h"
#include	"nodeinfo.h"
#include	"mylib.h"
#include	"knxtpbridge.h"
#include	"inilib.h"
/**
 *
 */
#define	MAX_SLEEP	1
#define	SLEEP_TIME	1
/**
 *
 */
extern	void	help() ;
/**
 *
 */
char	progName[64]  ;
pid_t	ownPID ;
knxLogHdl	*myKnxLogger ;
/**
 *
 */
int	cfgQueueKey	=	10031 ;
/**
 *
 */
void	iniCallback( char *_block, char *_para, char *_value) {
	_debug( 1, progName, "receive ini value block/paramater/value ... : %s/%s/%s\n", _block, _para, _value) ;
	if ( strcmp( _block, "[knxglobals]") == 0) {
		if ( strcmp( _para, "queueKey") == 0) {
			cfgQueueKey	=	atoi( _value) ;
		}
	}
}
/**
 *
 */
int	main( int argc, char *argv[]) {
	eibHdl	*myEIB ;
	int	opt ;
	int	i ;
	char	iniFilename[]	=	"knx.ini" ;
	/**
	 * setup the shared memory for EIB Receiving Buffer
	 */
	setbuf( stdout, NULL) ;
	strcpy( progName, *argv) ;
	myKnxLogger	=	knxLogOpen( 0) ;
	knxLog( myKnxLogger, progName, "starting up ...") ;
	/**
	 *
	 */
	debugLevel	=	0 ;
	iniFromFile( iniFilename, iniCallback) ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "MQ:SD:?")) != -1) {
		switch ( opt) {
		case	'D'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'Q'	:
			cfgQueueKey	=	atoi( optarg) ;
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
	myEIB	=	eibOpen( 0x0000, 0, cfgQueueKey, progName, 0) ;
	eibDumpIPCData( myEIB) ;
	eibClose( myEIB) ;
	/**
	 * close virtual EIB bus
	 * close KNX Level logger
	 */
	knxLogClose( myKnxLogger) ;
	/**
	 *
	 */
	exit( 0) ;
}

void	help() {
	printf( "%s: %s [-D <debugLevel>] [-Q=<queueIdf>] [-M] [-S] \n\n", progName, progName) ;
	printf( "Start a TPUART<->SimEIB/KNX bridge with id queueId.\n") ;
}
