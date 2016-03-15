/**
 *
 * knxmon.c
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
 * 2015-11-20	PA1	khw	inception;
 * 2015-11-24	PA2	khw	added semaphores for shared memory
 *				and message queue;
 * 2015-11-26	PA3	khw	added mylib function for half-float
 *				conversions;
 * 2015-12-17	PA4	khw	added MySQL for logging data;
 * 2016-02-01	PA5	khw	added XML capabilities for the object table;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<unistd.h>
#include	<time.h>
#include	<math.h>
#include	<sys/types.h>
#include	<sys/ipc.h> 
#include	<sys/shm.h> 
#include	<sys/msg.h>
#include	<sys/signal.h>
#include	<mysql.h>

#include	"debug.h"
#include	"knxlog.h"
#include	"nodeinfo.h"
#include	"knxprot.h"
#include	"knxtpbridge.h"
#include	"mylib.h"
#include	"myxml.h"

#include	"eib.h"		// rs232.c will differentiate:
				// ifdef  __MAC__
				// 	simulation
				// else
				// 	real life
#include	"mylib.h"
#include	"myxml.h"		// short xml-reader wrapper

#define	MAX_SLEEP	2

typedef struct msgbuf {
                long	mtype;
                long	group;
                value	val;
                char	mtext[32] ;
        } msgBuf;

extern	void	help() ;
/**
 *
 */
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
		int	opt ;
		int	queueKey	=	10031 ;
		int	status		=	0 ;
		int	sleepTimer	=	0 ;
		int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	int	cycleCounter ;
	/**
	 * define shared memory segment #0: COM Table
	 *	this segment holds information about the sizes of the other tables
 	 */
		key_t	shmCOMKey	=	SHM_COM_KEY ;
		int	shmCOMFlg	=	IPC_CREAT | 0600 ;
		int	shmCOMId ;
		int	shmCOMSize	=	256 ;
		int	*sizeTable ;
	/**
	 * define shared memory segment #1: OPC Table with buffer
	 *	this segment holds the structure defined in nodedata.h
 	 */
		key_t	shmOPCKey	=	SHM_OPC_KEY ;
		int	shmOPCFlg	=	IPC_CREAT | 0600 ;
		int	shmOPCId ;
		int	shmOPCSize ;
		node	*data ;
	/**
	 * define shared memory segment #2: KNX Table with buffer
	 *	this segment holds the KNX table defined in nodedata.h
 	 */
		key_t	shmKNXKey	=	SHM_KNX_KEY ;
		int	shmKNXFlg	=	IPC_CREAT | 0666 ;
		int	shmKNXId ;
		int	shmKNXSize	=	65536 * sizeof( float) ;
		float	*floats ;
		int	*ints ;
	/**
	 * define shared memory segment #3: CRF Table with buffer
	 *	this segment holds the cross-reference-table
 	 */
		key_t	shmCRFKey	=	SHM_CRF_KEY ;
		int	shmCRFFlg	=	IPC_CREAT | 0600 ;
		int	shmCRFId ;
		int	shmCRFSize	=	65536 * sizeof( int) ;
		int	*crf ;
	/**
	 * variables needed for the reception of EIB message
	 */
			FILE	*file ;
	unsigned	char	buf, myBuf[64] ;
			node	*actData ;
			node	*opcData ;
			int	opcDataSize ;
			int	rcvdBytes ;
			int	objectCount ;
			int	checksumError ;
			int	adrBytes ;
			int	n; 				// holds number of received characters
			int	monitor	=	0 ; 		// default: no message monitoring
	unsigned        int     control ;
	unsigned        int     addressType ;
	unsigned        int     routingCount ;
		int     expectedLength ;
			float	value ;
	unsigned        int     checkSum ;
	unsigned        char    checkS1 ;
			char    *ptr ;
		msgBuf  buffer ;
			knxMsg	myMsgBuf ;
			knxMsg	*myMsg ;
			char	xmlObjFile[128]	=	"/etc/knx.d/baos.xml" ;
			time_t		t ;
			struct	tm	tm ;
			int	lastSec	=	0 ;
			int	lastMin	=	0 ;
	/**
	 * variables related to MySQL database connection
	 */
			MYSQL	*mySql ;
			MYSQL_RES	*result ;
			MYSQL_ROW	row ;
	/**
	 *	END OF TEST SECTION
	 */
	signal( SIGINT, sigHandler) ;			// setup the signal handler for ctrl-C
	setbuf( stdout, NULL) ;				// disable output buffering on stdout
	strcpy( progName, *argv) ;
	_debug( 1, progName, "starting up ...") ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "D:Q:mx:?")) != -1) {
		switch ( opt) {
		case	'Q'	:
			queueKey	=	atoi( optarg) ;
			break ;
		case	'D'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'm'	:
			monitor	=	1 ;
			break ;
		case	'x'	:
			strcpy( xmlObjFile, optarg) ;
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
	 *	TEXT SECTION
	 */
	opcData	=	getNodeTable( xmlObjFile, &objectCount) ;
	opcDataSize	=	objectCount * sizeof( node) ;
	for ( i=0 ; i<objectCount ; i++) {
		_debug( 1, "%s ... %d \n", opcData[i].name, opcData[i].knxGroupAddr) ;
	}
	/**
	 * setup the MySQL connection
	 * fixed:
	 *	host		=	localhost
	 *	dbName		=	mas_knx_00000001	mas::KH private::00000001
	 *	dbUser		=	abc
	 *	dbPassword	=	cba
	 */
	_debug( 1, progName, "MySQL client version: %s", mysql_get_client_info());
//	if (( mySql = mysql_init( NULL)) == NULL) {
//		_debug( 0, progName, "could not connect to MySQL Server") ;
//		_debug( 0, progName, "Exiting with -1");
//		exit( -1) ;
//	}
//	if ( mysql_real_connect( mySql, "localhost", "abc", "cba", "mas_knx_00000001", 0, NULL, 0) == NULL) {
//		_debug( 0, progName, "mysql error := '%s'", mysql_error( mySql)) ;
//		_debug( 0, progName, "Exiting with -2");
//		exit( -2) ;
//	}
//	if ( mysql_query( mySql, "SELECT * FROM GroupAddress")) {
//		_debug( 0, progName, "mysql error := '%s'", mysql_error( mySql)) ;
//		_debug( 0, progName, "Exiting with -3");
//		exit( -3) ;
//	}
//	result	=	mysql_store_result( mySql) ;
//	while (( row = mysql_fetch_row( result))) {
//		_debug( 1, progName, "%s", row[1]) ;
//	}
//	mysql_free_result( result) ;
//	mysql_close( mySql) ;
	/**
	 * setup the shared memory for COMtable
	 */
	_debug( 10, progName, "shmCOMKey........: %d", shmCOMKey) ;
	_debug( 10, progName, "  shmCOMSize.....: %d", shmCOMSize) ;
	_debug( 10, progName, "  shmCOMId.......: %d", shmCOMId) ;
	_debug( 1, progName, "trying to obtain shared memory COMtable ...") ;
	if (( shmCOMId = shmget( shmCOMKey, shmCOMSize, IPC_CREAT | 0600)) < 0) {
		_debug( 0, progName, "shmget failed for COMtable");
		exit( -1) ;
	}
	_debug( 1, progName, "trying to attach shared memory COMtable") ;
	if (( sizeTable = (int *) shmat( shmCOMId, NULL, 0)) == (int *) -1) {
		_debug( 0, progName, "shmat failed for COMtable");
		exit( -1) ;
	}
	_debug( 10, progName, "  Address..: %8lx", (unsigned long) sizeTable) ;
	sizeTable[ SIZE_COM_TABLE]	=	256 ;
	sizeTable[ SIZE_KNX_TABLE]	=	shmKNXSize ;
	sizeTable[ SIZE_CRF_TABLE]	=	shmCRFSize ;
	/**
	 * setup the shared memory for OPCtable
	 */
	_debug( 1, progName, "trying to obtain shared memory OPCtable ...") ;
//	shmOPCSize	=	sizeof( _data) ;
	shmOPCSize	=	opcDataSize ;
	sizeTable[ SIZE_OPC_TABLE]	=	shmOPCSize ;
	if (( shmOPCId = shmget (shmOPCKey, shmOPCSize, shmOPCFlg)) < 0) {
		_debug( 0, progName, "shmget failed for OPCtable");
		exit(1);
	}
	if ( debugLevel > 0) {
		_debug( 1, progName, "trying to attach shared memory OPCtable") ;
	}
	if (( data = (node *) shmat(shmOPCId, NULL, 0)) == (node *) -1) {
		_debug( 0, progName, "shmat failed for OPCtable");
		exit(1);
	}
	if ( debugLevel > 10) {
		_debug( 1, progName, "shmOPCKey........: %d", shmOPCKey) ;
		_debug( 1, progName, "  shmOPCSize.....: %d", shmOPCSize) ;
		_debug( 1, progName, "  shmOPCId.......: %d", shmOPCId) ;
		_debug( 1, progName, "  Address..: %8lx", (unsigned long) data) ;
	}
//	memcpy( data, _data, sizeof( _data)) ;
	memcpy( data, opcData, opcDataSize) ;
	/**
	 * setup the shared memory for KNXtable
	 */
	if ( debugLevel > 0) {
		_debug( 1, progName, "trying to obtain shared memory KNXtable ...") ;
	}
	if (( shmKNXId = shmget( shmKNXKey, shmKNXSize, IPC_CREAT | 0600)) < 0) {
		_debug( 0, progName, "shmget failed for KNXtable");
		exit( -1) ;
	}
	if ( debugLevel > 0) {
		_debug( 1, progName, "trying to attach shared memory KNX table") ;
	}
	if (( floats = (float *) shmat( shmKNXId, NULL, 0)) == (float *) -1) {
		_debug( 0, progName, "shmat failed for KNXtable");
		exit( -1) ;
	}
	if ( debugLevel > 10) {
		_debug( 1, progName, "shmKNXKey........: %d", shmKNXKey) ;
		_debug( 1, progName, "  shmKNXSize.....: %d", shmKNXSize) ;
		_debug( 1, progName, "  shmKNXId.......: %d", shmKNXId) ;
	}
	ints	=	(int *) floats ;
	/**
	 * setup the shared memory for CRFtable
	 */
	if ( debugLevel > 0) {
		_debug( 1, progName, "trying to obtain shared memory CRFtable ...") ;
	}
	if (( shmCRFId = shmget( shmCRFKey, shmCRFSize, IPC_CREAT | 0600)) < 0) {
		_debug( 0, progName, "shmget failed for CRFtable");
		exit( -1) ;
	}
	if ( debugLevel > 0) {
		_debug( 1, progName, "trying to attach shared memory CRFtable...") ;
	}
	if (( crf = (int *) shmat( shmCRFId, NULL, 0)) == (int *) -1) {
		_debug( 0, progName, "shmat failed for CRFtable");
		exit( -1) ;
	}
	if ( debugLevel > 10) {
		_debug( 1, progName, "shmCRFKey........: %d", shmCRFKey) ;
		_debug( 1, progName, "  shmCRFSize.....: %d", shmCRFSize) ;
		_debug( 1, progName, "  shmCRFId.......: %d", shmCRFId) ;
	}
	/**
	 * build the cross-reference table for the KNX group numbers
	 */
	createCRF( data, objectCount, crf, (void *) floats) ;
	for ( i=0 ; i < 65536 ; i++) {
		if ( crf[i] != 0) {
			_debug( 1, progName, "KNX Group Address %d is assigned", i) ;
		}
	}
	/**
	 *
	 */
	if ( debugLevel > 0) {
		dumpDataAll( data, objectCount, (void *) floats) ;
	}
	myEIB	=	eibOpen( 0x0002, 0, queueKey) ;
	sleepTimer	=	0 ;
	cycleCounter	=	0 ;
	while ( debugLevel >= 0) {
		_debug( 1, progName, "cycleCounter := %d", cycleCounter++) ;
		if ( debugLevel > 10) {
			t	=	time( NULL) ;
			tm	=	*localtime( &t) ;
			if ( tm.tm_sec != lastSec) {
				lastSec	=	tm.tm_sec ;
				dumpDataAll( data, objectCount, (void *) floats) ;
			}
		}
		myMsg	=	eibReceive( myEIB, &myMsgBuf) ;
		if ( myMsg != NULL) {
			_debug( 1, progName, "received from origin ... %d", myMsg->apn) ;
			sleepTimer	=	0 ;
			if ( monitor == 1) {
				eibDump( "Monitor received ...:", myMsg) ;
			}
			/**
			 * only process if apn != 0, ie. skip messages from "local" messengers
			 * and we know about the receiving group address
			 */
			if ( myMsg->apn != 0 && crf[ myMsg->rcvAddr] != 0) {
				if ( debugLevel >= 3) {
					_debug( 1, progName, "received a known group address ... %d", myMsg->rcvAddr) ;
				}
				actData	=	&data[ crf[ myMsg->rcvAddr]] ;
				if ( actData->log != 0) {
					_debug( 1, progName, "will be logged ...") ;
				} else {
					_debug( 1, progName, "will *** NOT *** be logged ...") ;
				}
				switch ( myMsg->tlc) {
				case	0x00	:		// UDP
				case	0x01	:		// NDP
					switch ( myMsg->apci) {
					case	0x02	:	// groupValueWrite
						_debug( 1, progName, "WRITING ... ") ;
						_debug( 1, progName, ".......%s", actData->name) ;
						actData->knxHWAddr	=	myMsg->sndAddr ;
						switch ( actData->type) {
						case	dtBit	:
							_debug( 1, progName, "Assigning BIT") ;
							value	=	myMsg->mtext[1] & 0x01 ;
							actData->val.i	=	value ;
							ints[ myMsg->rcvAddr]	=	value ;
							break ;
						case	dtFloat2	:
							_debug( 1, progName, "Assigning HALF-FLOAT") ;
							value	=	hfbtf( &myMsg->mtext[2]) ;
							actData->val.f	=	value ;
							floats[ myMsg->rcvAddr]	=	value ;
							break ;
						default	:
							break ;
						}
						break ;
					}
					break ;
				case	0x02	:		// UCD
					break ;
				case	0x03	:		// NCD
					break ;
				}
			} else {
				_debug( 1, progName, "received an un-known group address [%5d]...", myMsg->rcvAddr) ;
				_debug( 1, progName, "will *** NOT *** be logged ...") ;
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
	 * close EIB port
	 */
	knxLog( myKnxLogger, progName, "shutting down up ...") ;
	eibClose( myEIB) ;
	knxLogClose( myKnxLogger) ;
	exit( status) ;
}

void	help() {
}
