/**
 *
 * progadr.c
 *
 * Programm physical address of KNX device
 *
 * Top-Level steps towards programming:
 *
 *  00010	Broadcast address enquriy
 *  00020	Device in programming mode answers with its physical addres
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
#include	"inilib.h"
/**
 *
 */
typedef	enum	_stateProg	{
			idle
		,	openCon
		,	waitCon		// wait for connection confirm
		,	closeCon
		,	readMask
		,	enqAddr		// enquire address
		,	waitAddr	// wait for address reqwponse
		,	writeAddr
		,	done
		}	stateProg ;

#define	MAX_SLEEP	1

extern	void	help() ;
extern	void	hdlMsg( eibHdl *, knxMsg *) ;

extern	void	cbOpenP2P( eibHdl *, knxMsg *) ;
extern	void	cbCloseP2P( eibHdl *, knxMsg *) ;
extern	void	cbConfP2P( eibHdl *, knxMsg *) ;
extern	void	cbRejectP2P( eibHdl *, knxMsg *) ;
extern	void	cbGroupValueRead( eibHdl *, knxMsg *) ;
extern	void	cbGroupValueResponse( eibHdl *, knxMsg *) ;
extern	void	cbGroupValueWrite( eibHdl *, knxMsg *) ;
extern	void	cbIndividualAddrWrite( eibHdl *, knxMsg *) ;
extern	void	cbIndividualAddrRequest( eibHdl *, knxMsg *) ;
extern	void	cbIndividualAddrResponse( eibHdl *, knxMsg *) ;
extern	void	cbAdcRead( eibHdl *, knxMsg *) ;
extern	void	cbAdcResponse( eibHdl *, knxMsg *) ;
extern	void	cbMemoryRead( eibHdl *, knxMsg *) ;
extern	void	cbMemoryResponse( eibHdl *, knxMsg *) ;
extern	void	cbMemoryWrite( eibHdl *, knxMsg *) ;
extern	void	cbUserMessage( eibHdl *, knxMsg *) ;
extern	void	cbMaskVersionRead( eibHdl *, knxMsg *) ;
extern	void	cbMaskVersionResponse( eibHdl *, knxMsg *) ;
extern	void	cbRestart( eibHdl *, knxMsg *) ;
extern	void	cbEscape( eibHdl *, knxMsg *) ;

eibCallbacks	myCB	=	{
			cbOpenP2P
		,	cbCloseP2P
		,	cbConfP2P
		,	cbRejectP2P
		,	cbGroupValueRead
		,	cbGroupValueResponse
		,	cbGroupValueWrite
		,	cbIndividualAddrWrite
		,	cbIndividualAddrRequest
		,	cbIndividualAddrResponse
		,	cbAdcRead
		,	cbAdcResponse
		,	cbMemoryRead
		,	cbMemoryResponse
		,	cbMemoryWrite
		,	cbUserMessage
		,	cbMaskVersionRead
		,	cbMaskVersionResponse
		,	cbRestart
		,	cbEscape
		} ;

char	progName[64] ;
int	debugLevel	=	0 ;
knxLogHdl	*myKnxLogger ;

int	deviceToProg	=	0x1234 ;		// thats the device we want to program
int	connectTo	=	0 ;
int	connectedTo	=	0 ;
int	talkingTo	=	0 ;
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
	} else if ( strcmp( _block, "[knxprogaddr]") == 0) {
		if ( strcmp( _para, "senderAddr") == 0) {
			cfgSenderAddr	=	atoi( _value) ;
		}
	}
}
/**
 *
 */
int	main( int argc, char *argv[]) {
			eibHdl		*myEIB ;
			int	opt ;
			int	status		=	0 ;
			int	sleepTimer	=	0 ;
			int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
		stateProg	myState ;
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
			knxMsg	myCmdBuf ;
			knxMsg	*myCmd ;
			int	waiting ;
	char	iniFilename[]	=	"knx.ini" ;
	/**
	 *
	 */
	setbuf( stdout, NULL) ;		// disable output buffering on stdout
	strcpy( progName, *argv) ;
	myKnxLogger	=	knxLogOpen( 0) ;
	knxLog( myKnxLogger, progName, "starting up ...") ;
	/**
	 *
	 */
	iniFromFile( iniFilename, iniCallback) ;
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
	myCmd	=	&myCmdBuf ;
	myState	=	enqAddr ;
	myEIB	=	eibOpen( cfgSenderAddr, 0x00, cfgQueueKey, progName, APN_RDWR) ;
	myState	=	idle ;
	while ( myState != done) {
		switch ( myState) {
		case	idle	:
			connectTo	=	deviceToProg ;
			myState	=	openCon ;
			break ;
		case	openCon	:
			myCmd	=	getOpenP2PMsg( myEIB, myCmd, connectTo) ;
//			eibSend( myEIB, myCmd) ;
			eibQueueMsg( myEIB, myCmd) ;
			myState	=	waitCon ;
			waiting	=	0 ;
			break ;
		case	waitCon		:
			if ( connectedTo != 0) {
				myState	=	done ;
			} else if ( waiting > 5) {
				myState	=	enqAddr ;
			}
			waiting++ ;
			break ;
		case	closeCon	:
//			eibSend( myEIB, myCmd) ;
			eibQueueMsg( myEIB, myCmd) ;
			myState	=	enqAddr ;
			break ;
		case	readMask	:
//			eibSend( myCmd) ;
			eibQueueMsg( myEIB, myCmd) ;
			myState	=	done ;
			break ;
		case	enqAddr	:
			myCmd	=	getIndividualAddrRequestMsg( myEIB, myCmd, 0x0000) ;
//			eibSend( myEIB, myCmd) ;
			eibQueueMsg( myEIB, myCmd) ;
			myState	=	waitAddr ;
			waiting	=	0 ;
			break ;
		case	waitAddr	:
			if ( talkingTo != 0) {
				myState	=	writeAddr ;
			} else if ( waiting > 5) {
				myState	=	done ;
			}
			waiting++ ;
			break ;
		case	writeAddr	:
			myCmd	=	getIndividualAddrWriteMsg( myEIB, myCmd, 0x0000) ;
//			eibSend( myEIB, myCmd) ;
			eibQueueMsg( myEIB, myCmd) ;
			myState	=	openCon ;
			break ;
		case	done	:
			break ;
		}
		myMsg	=	eibReceiveMsg( myEIB, &myMsgBuf) ;
		if ( myMsg != NULL) {
			sleepTimer	=	0 ;
			/**
			 * ply care about broadcast or MY device address
			 */
			if ( ( myMsg->rcvAddr == 0x0000 && myMsg->sndAddr != myEIB->sndAddr) || myMsg->rcvAddr == myEIB->sndAddr) {
				_debug( 1, progName, "......> %04x %04x", myMsg->sndAddr, myMsg->rcvAddr) ;
				eibDisect( myMsg) ;
				hdlMsg( myEIB, myMsg) ;
			}
		} else {
			sleepTimer++ ;
			if ( sleepTimer > MAX_SLEEP)
				sleepTimer	=	MAX_SLEEP ;
			_debug( 1, progName, "will go to sleep ... for %d seconds\n", sleepTimer) ;
			sleep( sleepTimer) ;
		}
	}
	eibClose( myEIB) ;
	knxLogClose( myKnxLogger) ;
	exit( status) ;
}

#include	"_knxfsm.c"

void	cbOpenP2P( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received ConnectionRequest ...") ;
}

void	cbCloseP2P( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received ConnectionClose ...") ;
}

void	cbConfP2P( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received ConnectionConfirm ...") ;
	_debug( 1, progName, "   sender......: %04x\n", progName, myMsg->sndAddr) ;
	_debug( 1, progName, "   rceiver.....: %04x\n", progName, myMsg->rcvAddr) ;
	connectedTo	=	myMsg->sndAddr ;
}

void	cbRejectP2P( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received ConnectionReject ...") ;
	_debug( 1, progName, "   sender......: %04x\n", progName, myMsg->sndAddr) ;
	_debug( 1, progName, "   rceiver.....: %04x\n", progName, myMsg->rcvAddr) ;
	connectedTo	=	myMsg->sndAddr ;
}

void	cbGroupValueRead( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received GroupValueRead ...") ;
}

void	cbGroupValueResponse( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received GroupValueResponse ...") ;
}

void	cbGroupValueWrite( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received GroupValueWrite ...") ;
}

void	cbIndividualAddrWrite( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received IndividualAddrWrite ...") ;
}

void	cbIndividualAddrRequest( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received IndividualAddrRequest ...") ;
}

void	cbIndividualAddrResponse( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received IndividualAddrResponse ...") ;
	_debug( 1, progName, "   sender......: %04x\n", progName, myMsg->sndAddr) ;
	_debug( 1, progName, "   rceiver.....: %04x\n", progName, myMsg->rcvAddr) ;
	talkingTo	=	myMsg->sndAddr ;
}

void	cbAdcRead( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received AdcRead ...") ;
}

void	cbAdcResponse( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received AdcResponse ...") ;
}

void	cbMemoryRead( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received MemoryRead ...") ;
}

void	cbMemoryResponse( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received MemoryResponse ...") ;
}

void	cbMemoryWrite( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received MemoryWrite ...") ;
}

void	cbUserMessage( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received UserMessage ...") ;
}

void	cbMaskVersionRead( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received MaAskVersionRead ...") ;
}

void	cbMaskVersionResponse( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received MaskVersionResponse ...") ;
}

void	cbRestart( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received Restart ...") ;
}

void	cbEscape( eibHdl *_myEIB, knxMsg *myMsg) {
	_debug( 1, progName, "received Escape ...") ;
}

void	help() {
}
