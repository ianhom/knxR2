/**
 * Copyright (c) 2016 wimtecc, Karl-Heinz Welter
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
 * knxwebbridge.c
 *
 * KNX web socket process
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
#include	<netinet/in.h>
#include	<netdb.h>
#include	<errno.h>
#include	<openssl/sha.h>

#include	"debug.h"
#include	"nodeinfo.h"
#include	"knxlog.h"
#include	"knxwebbridge.h"
#include	"eib.h"
#include	"mylib.h"
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
extern		void	hdlSocket( eibHdl *, int) ;
extern	unsigned char	*getPacket( unsigned char *, unsigned char *, int *) ;
extern		void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;
knxLogHdl	*myKnxLogger ;
char	GUID[]	=	"258EAFA5-E914-47DA-95CA-C5AB0DC85B11" ;
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
	pid_t	childProcessId, waitProcessId ;
	int	opt ;
	int	queueKey	=	10031 ;
	int	portNo	=	10002 ;
	int	sleepTimer	=	0 ;
	char	serverName[64]	=	"" ;
	unsigned	char	buf, bufp ;
	unsigned	char	*rcvData ;
	unsigned	char	*sndData ;
	ipbridgeMode	myMode	=	eibClient ;
	/**
	 *
	 */
	int		pid ;
	int		rootSockfd ;
	int		workSockfd ;
	socklen_t	clilen;
	char		buffer[2048];
	struct		sockaddr_in serv_addr, cli_addr;
	int		n;
	struct		hostent	*server ;
	/**
	 * setup the shared memory for EIB Receiving Buffer
	 */
//	signal( SIGINT, sigHandler) ;
	setbuf( stdout, NULL) ;
	strcpy( progName, *argv) ;
	_debug( 0, progName, "starting up ...") ;
	/**
	 * get command line options
	 */
	strcpy( serverName, "") ;
	while (( opt = getopt( argc, argv, "D:P:Q:?")) != -1) {
		switch ( opt) {
		case	'D'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'P'	:
			portNo	=	atoi( optarg) ;
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
	myEIB	=	eibOpen( 0x1051, 0, queueKey) ;
	/**
	 *
	 */
	knxLog( myKnxLogger, progName, "in server mode") ;
	myMode	=	eibServer ;
	rootSockfd	=	socket( AF_INET, SOCK_STREAM, 0) ;
	if ( rootSockfd < 0) {
		_debug( 0, progName, "Can not open socket");
		_debug( 0, progName, "Exiting with -1");
		return( -1);
	}
	bzero((char *) &serv_addr, sizeof( serv_addr)) ;
	serv_addr.sin_family		=	AF_INET ;
	serv_addr.sin_addr.s_addr	=	INADDR_ANY ;
	serv_addr.sin_port		=	htons( portNo) ;
	if ( bind( rootSockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr)) < 0) {
		_debug( 0, progName, "Can not bind socket");
		_debug( 0, progName, "Exiting with -2");
		return( -2);
	}
	_debug( 0, progName, "Listening on socket");
	listen( rootSockfd, 5) ;
	clilen	=	sizeof( cli_addr) ;
	/**
	 *
	 */
	while ( debugLevel >= 0) {
		workSockfd	=	accept( rootSockfd, (struct sockaddr *) &cli_addr, &clilen) ;
		if ( workSockfd < 0) {
			_debug( 0, progName, "Can not accept");
			_debug( 0, progName, "Exiting with -3");
			return( -3);
		}
		pid	=	fork() ;
		if ( pid == 0) {
			hdlSocket( myEIB, workSockfd) ;
			exit( 0) ;
		} else {
			close( workSockfd) ;
		}
	}
	/**
	 *
	 */
	knxLog( myKnxLogger, progName, "shutting down ...") ;
	eibClose( myEIB) ;
	knxLogClose( myKnxLogger) ;
	exit( 0) ;
}

void	hdlSocket( eibHdl *myEIB, int workSockfd) {
	int	rcvCount ;
	int	cycleCounter ;
	int	flags ;
	int	established ;
	int	i ;
	knxMsg	*msgToSnd ;
	knxMsg	msgRcv ;
	knxMsg	msgBuf ;
	char	buffer[64] ;
	char	clientBuffer[2048] ;
	unsigned	char *cbP ;
	unsigned	char wsData[512], *wsDataP ;
	int		wsDataLen ;
	char	*line, *sLine, lineBuf[256] ;
	char	*word, *sWord, wordBuf[256], lastWord[64] ;
	char	key[256] ;
	char	replyKey[256] ;
	char	replyKeySHA1[256] ;
	unsigned	char	*sha1Str ;
	char	*base64Str ;
	unsigned	long	base64Len ;
	/**
	 *
	 */
	_debug( 1, progName, "%s: cycleCounter := %d", cycleCounter++) ;
	/**
	 * server-mode, primarily runnign on the Raspberry end
	 *	read a message to-be send from the to-be-send queue
	 */
	/**
	 * read a message from the internal receive queue
	 * if it's coming from a different apn than KNX_ORIGIN_IP
	 *	it needs to be send towards the IP world
	 */
	flags	=	fcntl( workSockfd, F_GETFL, 0) ;
	fcntl( workSockfd, F_SETFL, flags | O_NONBLOCK) ;
	cycleCounter	=	0 ;
	established	=	0 ;
	while ( debugLevel >= 0) {
		if ( established == 1) {
			msgToSnd	=	eibReceive( myEIB, &msgBuf) ;
			if ( msgToSnd != NULL) {
				if ( msgToSnd->apn != 0) {
					_debug( 1, progName, "%s: received a message through the receive-queue (apn: %d), will forward to socket", msgToSnd->apn) ;
					
					/**
					 *
					 */
					sprintf( buffer, "EIB(KNX Packet Disassembly....:<br/>") ;
					wsDataLen	=	strlen( buffer) ;
					wsDataP	=	getPacket((unsigned char *) buffer, (unsigned char *)wsData, &wsDataLen) ;
					rcvCount	=	write( workSockfd, wsData, wsDataLen) ;
					/**
					 *
					 */
					sprintf( buffer, "  From device addr...........: %d <br/>", msgToSnd->sndAddr) ;
					wsDataLen	=	strlen( buffer) ;
					wsDataP	=	getPacket((unsigned char *) buffer, (unsigned char *)wsData, &wsDataLen) ;
					rcvCount	=	write( workSockfd, wsData, wsDataLen) ;
					/**
					 *
					 */
					sprintf( buffer, "  From addr..................: %d <br/>", msgToSnd->rcvAddr) ;
					wsDataLen	=	strlen( buffer) ;
					wsDataP	=	getPacket((unsigned char *) buffer, (unsigned char *)wsData, &wsDataLen) ;
					rcvCount	=	write( workSockfd, wsData, wsDataLen) ;
					/**
					 *
					 */
					sprintf( buffer, "<br/>") ;
					wsDataLen	=	strlen( buffer) ;
					wsDataP	=	getPacket((unsigned char *) buffer, (unsigned char *)wsData, &wsDataLen) ;
					rcvCount	=	write( workSockfd, wsData, wsDataLen) ;
				}
			}
		}
		/**
		 * read message from the socket
		 */
		while (( rcvCount = read( workSockfd, (void *) clientBuffer, 2048)) > 0) {
			printf( "........................................................................>>>>>>>>>") ;
			if ( established == 0) {
				for ( line = strtok_r( clientBuffer, "\r", &sLine) ; line != NULL ; line = strtok_r( NULL, "\r", &sLine)) {
					printf( "%s", line) ;
					strcpy( lineBuf, line) ;
					for ( word = strtok_r( lineBuf, " ", &sWord) ; word != NULL ; word = strtok_r( NULL, " ", &sWord)) {
						printf( "     %s", word) ;
						if ( strcmp( lastWord, "Sec-WebSocket-Key:") == 0) {
							strcpy( key, word) ;
							printf( ">>>>>>>\"%s\"", key) ;
						}
						strcpy( lastWord, word) ;
					}
				}
			} else {
				cbP	=	(unsigned char *) &clientBuffer[0] ;
				for ( i=0 ; i<rcvCount ; i++) {
					printf( "0x%02x ", (int) *cbP++) ;
					if (( i & 0x0f) == 0x0f)
						printf( "") ;
				}
				printf( "") ;
			}
		}
		if ( established == 0 && strlen( key) == 24) {
			strcpy( replyKey, key) ;
			strcat( replyKey, GUID) ;
			printf( ">>>>>>>REPLY-KEY>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\"%s\"", replyKey) ;
			sha1Str	=	SHA1( (unsigned char *) replyKey, (unsigned long) strlen( replyKey), (unsigned char *) NULL) ;
			printf( ">>>>>>>REPLY-KEY-SHA1>>>>>>>>>>>>>>>>>>>>>>>>>\"%s\"", sha1Str) ;
			base64Str	=	base64_encode( sha1Str, strlen((char *)  sha1Str), &base64Len) ;
			base64Str[base64Len]	=	'\0' ;
			printf( ">>>>>>>REPLY-KEY-BASE64>>>>>>>>>>>>>>>>>>>>>>>\"%s\"", base64Str) ;
			/**
			 *
			 */
			sprintf( buffer, "HTTP/1.1 101 Switching Protocols\r") ;
			printf( "<<<<<<<%s", buffer) ;
			rcvCount        =       write( workSockfd, buffer, strlen( buffer)) ;
			sprintf( buffer, "Upgrade: websocket\r") ;
			printf( "<<<<<<<%s", buffer) ;
			rcvCount        =       write( workSockfd, buffer, strlen( buffer)) ;
			sprintf( buffer, "Connection: Upgrade\r") ;
			printf( "<<<<<<<%s", buffer) ;
			rcvCount        =       write( workSockfd, buffer, strlen( buffer)) ;
			sprintf( buffer, "Sec-WebSocket-Accept: %s\r", base64Str) ;
			printf( "<<<<<<<%s", buffer) ;
			rcvCount        =       write( workSockfd, buffer, strlen( buffer)) ;
			sprintf( buffer, "\r") ;
			printf( "<<<<<<<%s", buffer) ;
			rcvCount        =       write( workSockfd, buffer, strlen( buffer)) ;
			/**
			 *
			 */
			free( base64Str) ;
			established	=	1 ;
			strcpy( key, "") ;
		}
		/**
		 *
		 */
		sleep( 1) ;
	}
}

unsigned char	*getPacket( unsigned char *_data, unsigned char *_buffer, int *_len) {
	unsigned	char	*bp, *dp ;
			int	i ;
	/**
	 *
	 */
	dp	=	_data ;
	bp	=	_buffer ;
	*bp++	=	0x81 ;					// 1.......	last frame
								// .xxx....	???
								// ....0001	text frame
	*bp++	=	0x00 | ( *_len & 0x7f) ;
	for ( i=0 ; i<*_len ; i++) {
		*bp++	=	*dp++ ;
	}
	*_len	+=	2 ;
	/**
	 *
	 */
	printf( "---> ") ;
	dp	=	_buffer ;
	for ( i=0 ; i<*_len ; i++) {
		printf( "0x%02x ", *dp++) ;
		if (( i & 0x0f) == 0x0f) {
			printf( "\n") ;
		}
	}
	printf( "\n") ;
	/**
	 *
	 */
	return _buffer ;
}

void	help() {
}

