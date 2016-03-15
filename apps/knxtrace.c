/*
 *
 * knxtrace.c
 *
 * EIB/KNX bus monitor with buffer
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
extern	void	logDisas( eibHdl *, knxMsg *) ;
extern	void	logBin( eibHdl *, knxMsg *) ;
extern	void	logHex( eibHdl *, knxMsg *) ;
extern	void	logCtrl( eibHdl *, knxMsg *, char *txt) ;

char	progName[64] ;
int	debugLevel	=	0 ;
knxLogHdl	*myKnxLogger ;

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
		} ;

typedef	enum	modeTrace	{
		KNX_TRC_DISAS	=	1
	,	KNX_TRC_LOG_BIN	=	2
	,	KNX_TRC_LOG_HEX	=	3
	,	KNX_TRC_HANDLE	=	4
	}	modeTrace ;

int	ownAddr		=	0xfff0 ;
int	progMode	=	1 ;
int	connectedTo	=	0 ;
int	myTraceMode	=	KNX_TRC_DISAS ;
char	myTraceFileName[128] ;

int	main( int argc, char *argv[]) {
		eibHdl	*myEIB ;
		int	opt ;
		int	status		=	0 ;
		int	sleepTimer	=	0 ;
		int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	int	cycleCounter ;
	/**
	 * variables needed for the reception of EIB message
	 */
			FILE	*file ;
	unsigned	char	buf, myBuf[64] ;
			node	*actData ;
			int	rcvdBytes ;
			int	monitor	=	0 ; 		// default: no message monitoring
			int	queueKey	=	10031 ;
	unsigned        int     control ;
	unsigned        int     addressType ;
	unsigned        int     routingCount ;
			char    *ptr ;
			knxMsg	myMsgBuf ;
			knxMsg	*myMsg ;
	unsigned	char	*cp ;
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
	while (( opt = getopt( argc, argv, "D:Q:f:m:?")) != -1) {
		switch ( opt) {
		case	'D'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'Q'	:
			queueKey	=	atoi( optarg) ;
			break ;
		case	'f'	:
			strcpy( myTraceFileName, optarg) ;
			break ;
		case	'm'	:
			myTraceMode	=	atoi( optarg) ;
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
	printf( "opening queue key %d\n", queueKey) ;
	myEIB	=	eibOpen( 0x0002, 0, queueKey) ;
	sleepTimer	=	0 ;
	cycleCounter	=	0 ;
	while ( debugLevel >= 0) {
		_debug( 1, progName, "cycleCounter := %d", cycleCounter++) ;
		myMsg	=	eibReceive( myEIB, &myMsgBuf) ;
		if ( myMsg != NULL) {
			if ( myMsg->apn != 0) {
				switch ( myMsg->frameType) {
				case	eibDataFrame	:
					sleepTimer	=	0 ;
					eibDisect( myMsg) ;
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						logDisas( myEIB, myMsg) ;
						break ;
					case	KNX_TRC_LOG_BIN	:
						logBin( myEIB, myMsg) ;
						break ;
					case	KNX_TRC_LOG_HEX	:
						logHex( myEIB, myMsg) ;
						break ;
					case	KNX_TRC_HANDLE	:
						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				case	eibAckFrame	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "ACK") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				case	eibNackFrame	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "NACK") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				case	eibBusyFrame	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "BUSY") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				case	eibPollDataFrame	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "::eibPollDataFrame") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				case	eibDataConfirmFrame	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "::eibDataConfirmFrame") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				case	eibOtherFrame	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "::eibOtherFrame") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				case	eibTPUARTStateIndication	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "::eibTPUARTStateIndication") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				case	bridgeCtrlFrame	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "::bridgeCtrlFrame") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				default	:
					switch ( myTraceMode) {
					case	KNX_TRC_DISAS	:
						break ;
					case	KNX_TRC_LOG_BIN	:
						break ;
					case	KNX_TRC_LOG_HEX	:
						logCtrl( myEIB, myMsg, "::??????????") ;
						break ;
					case	KNX_TRC_HANDLE	:
//						hdlMsg( myEIB, myMsg) ;
						break ;
					default	:
						break ;
					}
					break ;
				}
			}
		} else {
			sleepTimer++ ;
			if ( sleepTimer > MAX_SLEEP)
				sleepTimer	=	MAX_SLEEP ;
			_debug( 1, progName, "will go to sleep ... for %d seconds", sleepTimer) ;
			sleep( sleepTimer) ;
		}
	}
	knxLog( myKnxLogger, progName, "shutting down ...") ;
	eibClose( myEIB) ;
	knxLogClose( myKnxLogger) ;
	exit( status) ;
}

#include	"_knxfsm.c"
#include	"_disasMsg.c"

void	logDisas( eibHdl *_myEIB, knxMsg *_msg) {
	time_t		t ;
	struct	tm	tm ;
	char		cTime[64] ;
	FILE	*traceFile ;
	int	i, len ;
	/**
	 *
	 */
	/**
	 *
	 */
	if ( strlen( myTraceFileName) == 0) {
		traceFile	=	stdout ;
	} else {
		traceFile	=	fopen( myTraceFileName, "a+") ;
	}
	if ( traceFile != NULL) {
		disasMsg( traceFile, _myEIB, _msg) ;
		if ( strlen( myTraceFileName) == 0) {
		} else {
			fclose( traceFile) ;
		}
	}
}
void	logBin( eibHdl *_myEIB, knxMsg *_msg) {
	time_t		t ;
	struct	tm	tm ;
	char		cTime[64] ;
	FILE	*traceFile ;
	int	i, len ;
	/**
	 *
	 */
	/**
	 *
	 */
	if ( strlen( myTraceFileName) == 0) {
		traceFile	=	stdout ;
	} else {
		traceFile	=	fopen( myTraceFileName, "a+") ;
	}
	if ( traceFile != NULL) {
		if ( strlen( myTraceFileName) == 0) {
		} else {
			fclose( traceFile) ;
		}
	} else {
		_debug( 0, progName, "can't open log file [%s] for appending ... ", myTraceFileName) ;
	}
}

void	logHex( eibHdl *_myEIB, knxMsg *_msg) {
	time_t		t ;
	struct	tm	tm ;
	char		cTime[64] ;
	FILE	*traceFile ;
	int	i, len ;
	/**
	 *
	 */
	/**
	 *
	 */
	if ( strlen( myTraceFileName) == 0) {
		traceFile	=	stdout ;
	} else {
		traceFile	=	fopen( myTraceFileName, "a+") ;
	}
	if ( traceFile != NULL) {
		/**
		 * start with a time stamp
		 */
		t	=	time( NULL) ;
		tm	=	*localtime( &t) ;
		fprintf( traceFile, "%4d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		/**
		 * add the message
		 */
		fprintf( traceFile, "APN:%02d (Len: %2d)%02x %02x %02x %02x %02x %02x "
			,	_msg->apn
			,	_msg->length
			,	_msg->control
			,	(_msg->sndAddr >> 8) & 0x00ff
			,	_msg->sndAddr & 0x00ff
			,	(_msg->rcvAddr >> 8) & 0x00ff
			,	_msg->rcvAddr & 0x00ff
			,	_msg->info
			) ;
		len	=	_msg->length ;
		for ( i=0 ; i<len ; i++) {
			fprintf( traceFile, "%02x ", _msg->mtext[i]) ;
		}
		fprintf( traceFile, "%02x\n", _msg->checksum) ;
		if ( strlen( myTraceFileName) == 0) {
		} else {
			fclose( traceFile) ;
		}
	} else {
		_debug( 0, progName, "can't open log file [%s] for appending ... ", myTraceFileName) ;
	}
}

void	logCtrl( eibHdl *_myEIB, knxMsg *_msg, char *txt) {
	time_t		t ;
	struct	tm	tm ;
	char		cTime[64] ;
	FILE	*traceFile ;
	int	i, len ;
	/**
	 *
	 */
	/**
	 *
	 */
	if ( strlen( myTraceFileName) == 0) {
		traceFile	=	stdout ;
	} else {
		traceFile	=	fopen( myTraceFileName, "a+") ;
	}
	if ( traceFile != NULL) {
		/**
		 * start with a time stamp
		 */
		t	=	time( NULL) ;
		tm	=	*localtime( &t) ;
		fprintf( traceFile, "%4d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		/**
		 * add the message
		 */
		fprintf( traceFile, "APN:%02d %s (%02x)\n", _msg->apn, txt, _msg->control) ;
		if ( strlen( myTraceFileName) == 0) {
		} else {
			fclose( traceFile) ;
		}
	} else {
		_debug( 0, progName, "can't open log file [%s] for appending ... ", myTraceFileName) ;
	}
}

void	cbOpenP2P( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbCloseP2P( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbConfP2P( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbRejectP2P( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbGroupValueRead( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbGroupValueResponse( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbGroupValueWrite( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbIndividualAddrWrite( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbIndividualAddrRequest( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbIndividualAddrResponse( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbAdcRead( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbAdcResponse( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbMemoryRead( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbMemoryResponse( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbMemoryWrite( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbUserMessage( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbMaskVersionRead( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbMaskVersionResponse( eibHdl *_myEIB, knxMsg *myMsg) {
}

void	cbRestart( eibHdl *_myEIB, knxMsg *myMsg) {
}


void	help() {
	printf( "%s: %s [-D <debugLevel>] [-Q=<queueIdf>] [-f filename] [-b] [-m 1|2|3|4]\n\n", progName, progName) ;
	printf( "Start a tracer on the internal EIB/KNX busn") ;
	printf( "-f filename\tredirect oiutput to given file.\n") ;
	printf( "-m mode\t1 = disassembly\n") ;
	printf( "       \t2 = log binary\n") ;
	printf( "       \t3 = log hexadecimal\n") ;
	printf( "       \t4 = handle\n") ;
}
