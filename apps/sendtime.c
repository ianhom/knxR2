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
 * sendbit.c
 *
 * send a bit value (DPT 1.001) to a given group address
 *
 * Revision history
 *
 * Date		Rev.	Who	what
 * ----------------------------------------------------------------------------
 * 2015-11-26	PA1	khw	inception; derived from sendfloat.c
 * 2015-12-10	PA2	khw	adopted to new structure
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<unistd.h>
#include	<time.h>
#include	<sys/msg.h>

#include	"debug.h"
#include	"eib.h"
#include	"knxtpbridge.h"
#include	"nodeinfo.h"
#include	"mylib.h"
#include	"inilib.h"
/**
 *
 */
/**
 *
 */
extern	void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;
/**
 *
 */
int	cfgQueueKey	=	10031 ;
int	cfgSenderAddr	=	1 ;
/**
 *
 */
void	iniCallback( char *_block, char *_para, char *_value) {
	_debug( 1, progName, "receive ini value block/paramater/value ... : %s/%s/%s\n", _block, _para, _value) ;
	if ( strcmp( _block, "[knxglobals]") == 0) {
		if ( strcmp( _para, "queueKey") == 0) {
			cfgQueueKey	=	atoi( _value) ;
		}
	} else if ( strcmp( _block, "[sendf16]") == 0) {
		if ( strcmp( _para, "senderAddr") == 0) {
			cfgSenderAddr	=	atoi( _value) ;
		}
	}
}
/**
 *
 */
int main( int argc, char *argv[]) {
			eibHdl		*myEIB ;
			short		group ;
	unsigned	char		buf[16] ;
			int		msgLen ;
			int		opt ;
			int		sender	=	0 ;
			int		receiver	=	0 ;
			int		value ;
			int		wday ;
			int		hour ;
			int		min ;
			int		sec ;
			int		repeat	=	0x20 ;
			int		data[4] ;
			time_t		t ;
			struct	tm	tm ;
			char		iniFilename[]	=	"knx.ini" ;
	/**
	 *
	 */
	strcpy( progName, *argv) ;
	printf( "%s: starting up ... \n", progName) ;
	/**
	 *
	 */
	iniFromFile( iniFilename, iniCallback) ;
	/**
	 *
	 */
	t	=	time( NULL) ;
	tm	=	*localtime( &t) ;
	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	wday	=	tm.tm_wday ;
	hour	=	tm.tm_hour ;
	min	=	tm.tm_min ;
	sec	=	tm.tm_sec ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "D:Q:s:r:v:n:W:H:M:S:?")) != -1) {
		switch ( opt) {
		case	'D'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'Q'	:
			cfgQueueKey	=	atoi( optarg) ;
			break ;
		case	's'	:
			sender	=	atoi( optarg) ;
			break ;
		case	'r'	:
			receiver	=	atoi( optarg) ;
			break ;
		case	'n'	:
			repeat	=	0x00 ;
			break ;
		case	'W'	:
			wday	=	atoi( optarg) ;
			break ;
		case	'H'	:
			hour	=	atoi( optarg) ;
			break ;
		case	'M'	:
			min	=	atoi( optarg) ;
			break ;
		case	'S'	:
			sec	=	atoi( optarg) ;
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
	if ( sender != 0 && receiver != 0) {
		/**
		 *
		 */
		myEIB	=	eibOpen( cfgSenderAddr, 0, cfgQueueKey, progName, APN_WRONLY) ;
		data[ 0]	=	wday ;
		data[ 1]	=	hour ;
		data[ 2]	=	min ;
		data[ 3]	=	sec ;
		eibWriteTime( myEIB, receiver, data, 1) ;
		eibClose( myEIB) ;
	} else {
		printf( "%s: invalid sender and/or receiver address\n", progName) ;
		help() ;
	}
	/**
	 *
	 */
	exit( 0);
}
void	help() {
	printf( "%s: %s -s=<senderAddr> -r=<receiverAddr> -v=<value> \n\n", progName, progName) ;
	printf( "sends an arbitrary bit <value> from node <senderAddr> to <receiverAddr>h \n") ;
	printf( "receiver address is basically the group address \n") ;
}
