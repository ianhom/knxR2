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
 * simswitch.c
 *
 * simulates pressing a "switch" periodically
 * useful for testing the core functions
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2016-01-25	PA1	khw	inception;
 *
 */
#include	<stdio.h>
#include	<string.h>
#include	<strings.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<time.h>
#include	<sys/types.h>
#include	<sys/ipc.h> 
#include	<sys/shm.h> 
#include	<sys/msg.h> 
#include	<sys/signal.h> 

#include	"eib.h"
#include	"debug.h"
#include	"nodeinfo.h"
#include	"mylib.h"
#include	"knxlog.h"

extern	void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;
knxLogHdl	*myKnxLogger ;
/**
 *
 */
void	sigHandler( int _sig) {
	debugLevel	=	-1 ;
}
/**
 *
 */
int	main( int argc, char *argv[]) {
		eibHdl	*myEIB ;
		int	status		=	0 ;
		int		opt ;
		int	queueKey	=	10031 ;
	/**
	 *
	 */
	signal( SIGINT, sigHandler) ;
	strcpy( progName, *argv) ;
	_debug( 0, progName, "starting up ...") ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "D:Q:?")) != -1) {
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
	myKnxLogger	=	knxLogOpen( 0) ;
	knxLog( myKnxLogger, progName, "starting up ...") ;
	/**
	 *
	 */
	myEIB	=	eibOpen( 0x1234, 0, queueKey) ;
	printf( "myAPN ..... %d \n", myEIB->apn) ;
	while ( debugLevel >= 0) {
		eibWriteBit( myEIB, 10001, 0, 1) ;
		sleep( 5) ;
	}
	knxLog( myKnxLogger, progName, "terminating ...") ;
	knxLogClose( myKnxLogger) ;
	exit( status) ;
}

void	help() {
	printf( "%s: %s -d=<debugLevel> -w -b \n\n", progName, progName) ;
	printf( "-w handle water tank\n") ;
	printf( "-b handle heating buffer\n") ;
}
