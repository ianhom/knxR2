/**
 *
 * knxsim.c
 *
 * KNX bus monitor with buffer
 *
 * thius module monitors the messages on the knx bus and stores all group address write
 * value in a table stored in shared memory.
 *
 * Shared Memory Segment #0:	COM Table with sizes of the following three
 *				shared memory segments
 *				-> COMtable, -> int	*sizeTable
 * Shared Memory Segment #1:	OPC Table with value buffer
 *				-> OPCtable, -> node	*data (copy from _data)
 * Shared Memory Segment #2:	KNX Group Address value buffer
 *				-> KNXtable, -> int	*ints AND float	*floats
 * Shared Memory Segment #3:	Fixes size buffer of 256 bytes to communicate
 *				buffer sizes for Shared Memory Segment #1 and #2.
 *				->CRFtable, int		*crf
 *
 * Revision history
 *
 * Date		Rev.	Who	what
 * -----------------------------------------------------------------------------
 * 2016-01-13	PA1	khw	inception;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<time.h>
#include	<math.h>
#include	<getopt.h>
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<sys/msg.h>

#include	"debug.h"
#include	"knxlog.h"
#include	"nodeinfo.h"
#include	"knxprot.h"

#include	"eib.h"		// rs232.c will differentiate:
				// ifdef  __MAC__
				// 	simulation
				// else
				// 	real life
#include	"mylib.h"

#define	MAX_SLEEP	2

extern	void	help() ;

void	hdlMsg( eibHdl *, knxMsg *) ;

extern	void	cbOpenP2P( eibHdl *, knxMsg *) ;
extern	void	cbCloseP2P( eibHdl *, knxMsg *) ;
extern	void	cbConfP2P( eibHdl *, knxMsg *) ;
extern	void	cbRejectP2P( eibHdl *, knxMsg *) ;
extern	void	cbIndividualAddrRequest( eibHdl *, knxMsg *) ;
extern	void	cbIndividualAddrResponse( eibHdl *, knxMsg *) ;
extern	void	cbIndividualAddrWrite( eibHdl *, knxMsg *) ;

eibCallbacks	myCB	=	{
			cbOpenP2P
		,	cbCloseP2P
		,	cbConfP2P
		,	cbRejectP2P
		,	NULL
		,	NULL
		,	NULL
		,	cbIndividualAddrWrite
		,	cbIndividualAddrRequest
		,	cbIndividualAddrResponse
		,	NULL
		,	NULL
		,	NULL
		,	NULL
		,	NULL
		,	NULL
		,	NULL
		,	NULL
		,	NULL
		,	NULL
		} ;


			char	progName[64] ;
			int	debugLevel	=	0 ;
			knxLogHdl	*myKnxLogger ;

			int	progMode	=	1 ;
			int	connectedTo	=	0 ;
unsigned	int	deviceAddress	=	0x1101 ;

int	main( int argc, char *argv[]) {
		eibHdl	*myEIB ;
		int	opt ;
		int	status		=	0 ;
		int	sleepTimer	=	0 ;
		int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	/**
	 * variables needed for the reception of EIB message
	 */
			FILE	*file ;
	unsigned	char	buf, myBuf[64] ;
			node	*actData ;
			int	rcvdBytes ;
			int	monitor	=	0 ; 		// default: no message monitoring
	unsigned        int     control ;
	unsigned        int     addressType ;
	unsigned        int     routingCount ;
			char    *ptr ;
			knxMsg	myMsgBuf ;
			knxMsg	*myMsg ;
	/**
	 *
	 */
	setbuf( stdout, NULL) ;				// disable output buffering on stdout
	strcpy( progName, *argv) ;
	myKnxLogger	=	knxLogOpen( 0) ;
	knxLog( myKnxLogger, progName, "starting up ...") ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "d:m?")) != -1) {
		switch ( opt) {
		case	'd'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'm'	:
			monitor	=	1 ;
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
	myEIB	=	eibOpen( deviceAddress, 0, 10031) ;
	sleepTimer	=	0 ;
	while ( 1) {
		printf( "%s: deviceAddress %04x, connectedTo %04x \n", progName, deviceAddress, connectedTo) ;
		myMsg	=	eibReceive( myEIB, &myMsgBuf) ;
		if ( myMsg != NULL) {
			sleepTimer	=	0 ;
			/**
			 * ply care about broadcast or MY device address
			 */
			if ( myMsg->rcvAddr == 0x0000 || myMsg->rcvAddr == myEIB->sndAddr) {
				eibDisect( myMsg) ;
				hdlMsg( myEIB, myMsg) ;
			}
		} else {
			sleepTimer++ ;
			if ( sleepTimer > MAX_SLEEP)
				sleepTimer	=	MAX_SLEEP ;
			_debug( 1, progName, "will go to sleep ... for %d seconds", sleepTimer) ;
			sleep( sleepTimer) ;
		}
	}
	/**
	 *
	 */
	eibClose( myEIB) ;
	knxLogClose( myKnxLogger) ;
	exit( status) ;
}

#include	"_knxfsm.c"

void	cbOpenP2P( eibHdl *_myEIB, knxMsg *myMsg) {
	knxMsg	*myReply, myReplyBuf ;
	myReply	=	&myReplyBuf ;
	printf( "received ConnectionRequest ... \n") ;
	if ( connectedTo == 0) {
		myReply	=	getConfP2PMsg( _myEIB, myReply, myMsg->sndAddr) ;
//		eibSend( _myEIB, myReply) ;
		_eibPutReceive( _myEIB, myReply) ;
		connectedTo	=	myReply->rcvAddr ;
	} else {
	}
}

void	cbCloseP2P( eibHdl *_myEIB, knxMsg *myMsg) {
	printf( "%s: received ConnectionClose ... \n", progName) ;
}

void	cbConfP2P( eibHdl *_myEIB, knxMsg *myMsg) {
	printf( "%s: received ConnectionConfirm ... \n", progName) ;
}

void	cbRejectP2P( eibHdl *_myEIB, knxMsg *myMsg) {
	printf( "%s: received ConnectionReject ... \n", progName) ;
}

void	cbIndividualAddrRequest( eibHdl *_myEIB, knxMsg *myMsg) {
	knxMsg	*myReply, myReplyBuf ;
	myReply	=	&myReplyBuf ;
	/**
	 *
	 */
	printf( "%s: received IndividualAddrRequest ... \n", progName) ;
	myReply	=	getIndividualAddrResponseMsg( _myEIB, myReply, 0x0000) ;
//	eibSend( _myEIB, myReply) ;
	_eibPutReceive( _myEIB, myReply) ;
}

void	cbIndividualAddrResponse( eibHdl *_myEIB, knxMsg *myMsg) {
	printf( "%s: received IndividualAddrResponse ... \n", progName) ;
}

void	cbIndividualAddrWrite( eibHdl *_myEIB, knxMsg *myMsg) {
	printf( "%s: received IndividualAddrWrite ... \n", progName) ;
	if ( progMode == 1) {
		deviceAddress	=	( myMsg->mtext[2] << 8) | ( myMsg->mtext[3]) ;
	}
}

void	help() {
}
