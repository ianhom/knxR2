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
 * knxbackbone bridges a "simulated" knx-bus to a "real-world" knx-bus
 * through a TPUART or, in
 * a purely simulated mode, e.g. on Mac OS, acts as a virtual TPUART which copies
 * everything supposed to be transmitted to the real-orld to the incoming side.
 * the communication towards other modules happens through two separate message queue.
 * the message queues contain a complete message (see: eib.h)
 *
 * Revision history
 *
 * Date		Rev.	Who	what
 * -----------------------------------------------------------------------------
 * 2016-03-02	PA1	khw	inception;
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
void	sigHandler( int _sig) {
	debugLevel	=	-1 ;
}
void	iniCallback( char *_block, char *_para, char *_value) {
	_debug( 1, progName, "receive ini value block/paramater/value ... : %s/%s/%s\n", _block, _para, _value) ;
}
/**
 *
 */
int	main( int argc, char *argv[]) {
	eibHdl	*myEIB ;
	int	myAPN	=	0 ;
	knxMsg	*msgToSnd, msgBuf ;
	knxMsg	msgRcv ;
	int	sendingByte ;
	int	opt ;
	int	sleepTimer	=	0 ;
	int	incompleteMsg ;
	int	rcvdLength ;
	int	sentLength ;
	int	expLength ;
	int	queueKey	=	10031 ;;
	bridgeModeRcv	rcvMode ;
	bridgeModeSnd	sndMode ;
	char	mode[]={'8','e','1',0};
	int	cport_nr	=	22 ;	// /dev/ttyS0 (COM1 on windows) */
	int	bdrate		=	19200 ;	// 9600 baud */
	int	rcvdBytes ;
	unsigned	char	buf, bufp ;
	unsigned	char	*rcvData ;
	unsigned	char	*sndData ;
	ini	*myIni ;
	int	cycleCounter ;
	knxOpMode	opMode	=	opModeMaster ;
	char	iniFilename[]	=	"/etc/knx.d/knx.ini" ;
	/**
	 * setup the shared memory for EIB Receiving Buffer
	 */
	ownPID	=	getpid() ;
	signal( SIGINT, sigHandler) ;
	setbuf( stdout, NULL) ;
	strcpy( progName, *argv) ;
	myKnxLogger	=	knxLogOpen( 0) ;
	knxLog( myKnxLogger, progName, "starting up ...") ;
	/**
	 *
	 */
	debugLevel	=	99 ;
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
			queueKey	=	atoi( optarg) ;
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
	if ( createPIDFile( progName, "", ownPID)) {
		myEIB	=	eibOpen( 0x0000, IPC_CREAT, queueKey) ;
		while ( debugLevel >= 0) {
			sleep( 0) ;
		}
		eibClose( myEIB) ;
		deletePIDFile( progName, "", ownPID) ;
	} else {
		knxLogRelease( myKnxLogger) ;
		knxLog( myKnxLogger, progName, "%d: process already running ...", ownPID) ;
		_debug( 0, progName, "process already running ...") ;
		_debug( 0, progName, "remove the file '/tmp/knxtpbridge_' if you believe to know what you are doin'") ;
	}
	knxLog( myKnxLogger, progName, "%d: shutting down ...", ownPID) ;
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
