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
 * knxipbridge.c
 *
 * IP bridge process
 *
 * ipbridge bridges a "simulated" knx-bus to the IP network
 *
 * Revision history
 *
 * Date		Rev.	Who	what
 * -----------------------------------------------------------------------------
 * 2016-01-20	PA1	khw	inception;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<unistd.h>
#include	<time.h>
#include	<math.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/socket.h>
#include	<sys/signal.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<errno.h>

#include	"debug.h"
#include	"knxlog.h"
#include	"knxipbridge.h"
#include	"eib.h"
#include	"eib.h"
#include	"inilib.h"
/**
 *
 */
#define	MAX_SLEEP	1
/**
 *
 */
typedef	enum	{
		eibServer
	,	eibClient
	}	ipbridgeMode ;
/**
 *
 */
extern	void	hdlSocket( int, int) ;
extern	void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;
knxLogHdl	*myKnxLogger	=	NULL ;
ipbridgeMode	myMode	=	eibClient ;
/**
 *
 */
void	sigHandler( int _sig) {
	knxLog( myKnxLogger, progName, "%d: mode %d ", getpid(), myMode) ;
	debugLevel	=	-1 ;
}
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
	} else if ( strcmp( _block, "[knxipbridge]") == 0) {
		if ( strcmp( _para, "senderAddr") == 0) {
			cfgSenderAddr	=	atoi( _value) ;
		}
	}
}
/**
 *
 */
int	main( int argc, char *argv[]) {
	pid_t	ownPID ;
	pid_t	childProcessId, waitProcessId ;
	int	opt ;
	int	sleepTimer	=	0 ;
	char	serverName[64]	=	"" ;
	unsigned	char	buf, bufp ;
	unsigned	char	*rcvData ;
	unsigned	char	*sndData ;
	/**
	 *
	 */
	int		pid ;
	int		rootSockfd ;
	int		workSockfd ;
	int		portno;
	socklen_t	clilen;
	char		buffer[256];
	struct		sockaddr_in serv_addr, cli_addr;
	int		n;
	struct		hostent	*server ;
			char		iniFilename[]	=	"knx.ini" ;
	/**
	 * setup the shared memory for EIB Receiving Buffer
	 */
	ownPID	=	getpid() ;
	signal( SIGINT, sigHandler) ;
	setbuf( stdout, NULL) ;
	strcpy( progName, *argv) ;
	_debug( 0, progName, "starting up ...") ;
	/**
	 *
	 */
	iniFromFile( iniFilename, iniCallback) ;
	/**
	 * get command line options
	 */
	strcpy( serverName, "") ;
	while (( opt = getopt( argc, argv, "D:H:Q:?")) != -1) {
		switch ( opt) {
		case	'D'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'H'	:
			strcpy( serverName, optarg) ;
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
	_debug( 0, progName, "starting up ...") ;
	myKnxLogger	=	knxLogOpen( 0) ;
	knxLog( myKnxLogger, progName, "%d: starting up ...", ownPID) ;
	/**
	 * IF no "servername" is provided, we ARE the server
	 * ELSE
	 *	we will contact the server named "serverName"
	 */
	if ( strlen( serverName) == 0) {
		_debug( 0, progName, "starting up ...") ;
		knxLog( myKnxLogger, progName, "%d: in server mode", ownPID) ;
		myMode	=	eibServer ;
		rootSockfd	=	socket( AF_INET, SOCK_STREAM, 0) ;
		fcntl( rootSockfd, F_SETFL, fcntl( rootSockfd, F_GETFL, 0) | O_NONBLOCK) ;
		if ( rootSockfd < 0) {
			_debug( 0, progName, "Can not open socket");
			_debug( 0, progName, "Exiting with -1");
			return( -1);
		}
		bzero((char *) &serv_addr, sizeof( serv_addr)) ;
		portno	=	10001 ;
		serv_addr.sin_family		=	AF_INET ;
		serv_addr.sin_addr.s_addr	=	INADDR_ANY ;
		serv_addr.sin_port		=	htons( portno) ;
		if ( bind( rootSockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr)) < 0) {
			_debug( 0, progName, "Can not bind socket");
			_debug( 0, progName, "Exiting with -2");
			return( -2);
		}
		listen( rootSockfd, 5) ;
		clilen	=	sizeof( cli_addr) ;
		/**
		 *
		 */
		while ( debugLevel >= 0) {
			workSockfd	=	accept( rootSockfd, (struct sockaddr *) &cli_addr, &clilen) ;
			if ( workSockfd < 0 && errno != EAGAIN) {
				_debug( 0, progName, "Can not accept");
				_debug( 0, progName, "Exiting with -3");
				return( -3);
			} else if ( workSockfd >= 0) {
				pid	=	fork() ;
				if ( pid == 0) {
					hdlSocket( cfgQueueKey, workSockfd) ;
					exit( 0) ;
				} else {
					close( workSockfd) ;
				}
			} else {
				sleep( 5) ;
			}
		}
		knxLog( myKnxLogger, progName, "%d: closing rootSocket ...", ownPID) ;
		close( rootSockfd) ;
	} else {
		knxLog( myKnxLogger, progName, "%d: in client mode", ownPID) ;
		myMode	=	eibClient ;
		/**
		 * client mode ...
		 */
		portno	=	10001 ;
		workSockfd	=	socket( AF_INET, SOCK_STREAM, 0) ;
		if ( workSockfd < 0) {
			_debug( 0, progName, "Can not open socket");
			_debug( 0, progName, "Exiting with -1");
			return( -1);
		}
		server	=	gethostbyname( serverName) ;
		if ( server == NULL) {
			_debug( 0, progName, "Can not resolve server name to ip-address");
			_debug( 0, progName, "Exiting with -2");
			return( -2);
		}
		bzero((char *) &serv_addr, sizeof( serv_addr)) ;
		serv_addr.sin_family	=	AF_INET ;
		bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length) ;
		serv_addr.sin_port	=	htons( portno);
		if ( connect( workSockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			_debug( 0, progName, "Can not connect to server");
			_debug( 0, progName, "Exiting with -3");
			return( -3);
		}
		hdlSocket( cfgQueueKey, workSockfd) ;
	}
	/**
	 *
	 */
	knxLog( myKnxLogger, progName, "%d: shutting down ...", ownPID) ;
	knxLogClose( myKnxLogger) ;
	exit( 0) ;
}

void	hdlSocket( int cfgQueueKey, int workSockfd) {
	eibHdl	*myEIB ;
	int	rcvCount ;		// length of message received through socket
	int	sndCount ;		// length of message sent through socket
	int	cycleCounter ;
	int	flags ;
	int	myAPN ;
	pid_t	ownPID ;
	pid_t	parentPID ;
	knxMsg	*msgToSnd ;
	knxMsg	msgRcv ;
	knxMsg	msgBuf ;
	/**
	 *
	 */
	ownPID	=	getpid() ;
	parentPID	=	getppid() ;
	knxLog( myKnxLogger, progName, "%d: socket handler started", parentPID) ;
	signal( SIGINT, sigHandler) ;
	myEIB	=	eibOpen( cfgSenderAddr, 0x00, cfgQueueKey, progName, APN_RDWR) ;
	myAPN	=	myEIB->apn ;
	knxLog( myKnxLogger, progName, "%d->%d: myAPN := %d", parentPID, ownPID, myAPN) ;
	/**
	 * server-mode, primarily runnign on the Raspberry end
	 *	read a message to-be send from the to-be-send queue
	 */
	/**
	 * read a message from the internal receive queue
	 * if it's coming from a different apn than the own apn (myEIB->apn)
	 *	it needs to be send towards the IP world
	 */
	fcntl( workSockfd, F_SETFL, fcntl( workSockfd, F_GETFL, 0) | O_NONBLOCK) ;
	cycleCounter	=	0 ;
	while ( debugLevel >= 0) {
		_debug( 1, progName, "cycleCounter := %d", cycleCounter++) ;
		msgToSnd	=	eibReceiveMsg( myEIB, &msgBuf) ;
		if ( msgToSnd != NULL) {
			if ( msgToSnd->apn != myAPN && msgToSnd->apn != 0) {
				_debug( 1, progName, "got message through receive-queue (apn: %d), will forward to socket",
								 msgToSnd->apn) ;
				sndCount	=	write( workSockfd, msgToSnd, KNX_MSG_SIZE) ;
				_debug( 1, progName, "wrote %d to socket", sndCount) ;
				if ( sndCount <= 0) {
					_debug( 11, progName, "could not send data through socket; receiver might be dead");
				}
			} else {
				_debug( 1, progName, "got message through receive-queue (apn: %d), will not forward",
								 msgToSnd->apn) ;
			}
		}
		/**
		 * read message from the socket
		 */
		while (( rcvCount = read( workSockfd, (void *) &msgBuf, 64)) > 0) {
			if ( rcvCount == KNX_MSG_SIZE) {
				_debug( 1, progName, "received a message through the socket, will add to receive-queue") ;
				msgBuf.apn	=	myAPN ;
				eibQueueMsg( myEIB, &msgBuf) ;
			} else if ( rcvCount == 1) {
				_debug( 1, progName, "received single byte, will close socket") ;
				debugLevel	=	-1 ;
			} else {
				_debug( 1, progName, "Invalid packet received");
			}
		}
		sleep( 1) ;
	}
	/**
	 *
	 */
	knxLog( myKnxLogger, progName, "%d: socket handler ended", ownPID) ;
	write( workSockfd, "X", 1) ;
	close( workSockfd) ;
	eibClose( myEIB) ;
}

void	help() {
}
