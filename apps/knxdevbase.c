/**
 *
 * knxdevbase.c
 *
 * KNX basic device simulation
 *
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
#include	"nodedata.h"
#include	"knxprot.h"
#include	"knxbridge.h"

#include	"eib.h"		// rs232.c will differentiate:
				// ifdef  __MAC__
				// 	simulation
				// else
				// 	real life
#include	"mylib.h"

#define	MAX_SLEEP	2

extern	void	help() ;

char	progName[64] ;
int	debugLevel	=	0 ;

int	main( int argc, char *argv[]) {
		eibHdl	*myEIB ;
		int		myAPN	=	0 ;
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
			int	monitor	=	0 ; 		// default: no message monitoring
			knxMsg	myMsgBuf ;
			knxMsg	*myMsg ;
	/**
	 *
	 */
	setbuf( stdout, NULL) ;				// disable output buffering on stdout
	strcpy( progName, *argv) ;
	knxLogOpen() ;
	knxLogC( progName, "starting up ...") ;
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
	myEIB	=	eibOpen( 0x0002, 0) ;
	myAPN	=	eibAssignAPN( myEIB, progName) ;
	sleepTimer	=	0 ;
	while ( 1) {
		myMsg	=	eibReceive( &myMsgBuf) ;
		if ( myMsg != NULL) {
			eibDisect( myMsg) ;
			sleepTimer	=	0 ;
			switch ( myMsg->tlc) {
			case	0x00	:		// UDP
			case	0x01	:		// NDP
				switch ( myMsg->apci) {
				case	0x02	:	// groupValueWrite
					printf( "GroupValueWrite\n") ;
					break ;
				}
				break ;
			case	0x02	:		// UCD
				break ;
			case	0x03	:		// NCD
				break ;
			}
		} else {
			sleepTimer++ ;
			if ( sleepTimer > MAX_SLEEP)
				sleepTimer	=	MAX_SLEEP ;
			_debug( 1, progName, "will go to sleep ... for %d seconds", sleepTimer) ;
			sleep( sleepTimer) ;
		}
	}
	exit( status) ;
}

void	help() {
}
