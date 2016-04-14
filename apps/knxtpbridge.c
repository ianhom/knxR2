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
 * knxtpbridge.c
 *
 * KNX TP (Twisted Pair) bridge process
 *
 * knxbridge bridges a "simulated" knx-bus to a "real-world" knx-bus 
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
 * 2015-11-20	PA1	khw	inception;
 * 2015-11-24	PA2	khw	added semaphores for shared memory
 *				and message queue;
 * 2015-11-26	PA3	khw	added mylib function for half-float
 *				conversions;
 * 2015-12-03	PA4	khw	derived from knx R1;
 * 2015-12-17	PA5	khw	added semaphores for send- and receive-queue;
 * 2016-01-26	PA6	khw	renamed to knxtpbridge;
 *				added apn as routing decision;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<strings.h>
#include	<unistd.h>
#include	<time.h>
#include	<math.h>
#include	<unistd.h>
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
	int	myAPN	=	0 ;
	knxMsg	*msgToSnd, msgBuf ;
	knxMsg	msgRcv ;
	int		pid ;
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
	int	cycleCounter ;
	int	sndConfirmed	=	0 ;
	int	sending	=	0 ;
	int	sndTimeout	=	0 ;
	knxOpMode	opMode	=	opModeMaster ;
	/**
	 * setup the shared memory for EIB Receiving Buffer
	 */
	ownPID	=	getpid() ;
	signal( SIGINT, sigHandler) ;
	setbuf( stdout, NULL) ;
	strcpy( progName, *argv) ;
	_debug( 0, progName, "starting up ...") ;
	/**
	 * get command line options
	 */
	while (( opt = getopt( argc, argv, "MQ:SD:?")) != -1) {
		switch ( opt) {
		case	'D'	:
			debugLevel	=	atoi( optarg) ;
			break ;
		case	'M'	:
			opMode	=	opModeMaster ;
			break ;
		case	'Q'	:
			queueKey	=	atoi( optarg) ;
			break ;
		case	'S'	:
			opMode	=	opModeSlave ;
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
	myKnxLogger	=	knxLogOpen( IPC_CREAT) ;
	knxLog( myKnxLogger, progName, "%d: starting up ...", ownPID) ;
	if ( createPIDFile( progName, "", ownPID)) {
		myEIB	=	eibOpen( 0x0000, 0, queueKey) ;
		myAPN	=	eibAssignAPN( myEIB) ;
		printf( "myAPN ..... %d \n", myAPN) ;
		knxLog( myKnxLogger, progName, "%d: myAPN := %d", ownPID, myAPN) ;
		/**
		 * open communication port
		 */
		RS232_PrepComport(cport_nr, bdrate, mode) ;
		if ( RS232_OpenComport( cport_nr, bdrate, mode)) {
			_debug( 1, progName, "Can not open comport");
			return(0);
		}
		RS232_SendByte( cport_nr, 0x01);
		rcvdBytes	=	RS232_PollComport(cport_nr, (unsigned char *) &buf, 1);
		_debug( 1, progName, "-->%02x", buf) ;
#ifdef	__MACH__
		/**
		 * running in plain simulation mode
		 * the only thing we do here is to copy every message from the transmit buffer
		 * to the receive buffer. this is actually what happens if we are in TPUART mode,
		 * when the transmitted bytes are immediately received back
		 */
		cycleCounter	=	0 ;
		knxLog( myKnxLogger, progName, "%d: running in SIMULATION mode ... (on MacOS?)", ownPID) ;
		while ( debugLevel >= 0) {
			_debug( 1, progName, "cycleCounter := %d", cycleCounter++) ;
			/**
			 * read a message to be send from the send queue
			 * msgBuf collects messages
			 */
			if (( msgToSnd = eibReceive( myEIB, &msgBuf)) != NULL && debugLevel >= 0) {
				/**
				 * and put it into the received queue
				 */
				if ( msgToSnd->apn == 0) {
					_debug( 1, progName, "received message through the send-queue, will loop-back") ;
					msgToSnd->apn	=	myAPN ;
					msgToSnd->frameType	=	eibDataFrame ;
					_eibPutReceive( myEIB, msgToSnd) ;
				} else {
					_debug( 1, progName, "received message through receive-queue (apn: %d), will not loop-back",
									 msgToSnd->apn) ;
				}
			}
			sleepTimer	=	SLEEP_TIME ;
			_debug( 1, progName, "will go to sleep ... for %d seconds", sleepTimer) ;
			sleep( sleepTimer) ;
		}
#else
		knxLog( myKnxLogger, progName, "%d: running in real bridging mode ... (on Raspberry?) RECEIVER", ownPID) ;
		rcvMode	=	waiting_for_control ;
		incompleteMsg	=	0 ;
		while ( debugLevel >= 0) {
			/**
			 * as long as there's something coming from the serial port or an incomplete message
			 *	put it into the receive buffer
			 */
			rcvMode	=	waiting_for_control ;
			rcvdBytes	=	RS232_PollComport(cport_nr, (unsigned char *) &buf, 1) ;
			while ( rcvdBytes > 0 || incompleteMsg) {
				if ( rcvdBytes > 0) {
					switch ( rcvMode) {
					case	waiting_for_control	:
						/**
						 *  check for start of L_DATA frame
						 */
						if (( buf & 0xd3) == 0x90) {
							msgRcv.control	=	buf ;
							rcvMode	=	waiting_for_hw_adr ;
							rcvdLength	=	0 ;
							incompleteMsg	=	1 ;
							msgRcv.sndAddr	=	0x0000 ;
							msgRcv.ownChecksum	=	0x00 ;
						/**
						 *  check for start of L_LONG_DATA frame
						 */
						} else if (( buf & 0xd3) == 0x10) {
							msgRcv.control	=	buf ;
							rcvMode	=	waiting_for_hw_adr ;
							rcvdLength	=	0 ;
							incompleteMsg	=	1 ;
							msgRcv.sndAddr	=	0x0000 ;
							msgRcv.ownChecksum	=	0x00 ;
						/**
						 *  check for ACK frame
						 */
						} else if ( buf == 0xcc) {
							msgRcv.apn	=	myAPN ;
							msgRcv.control	=	buf ;
							msgRcv.frameType	=	eibAckFrame ;
							_eibPutReceive( myEIB, &msgRcv) ;
						/**
						 *  check for NACK frame
						 */
						} else if ( buf == 0x0c) {
							msgRcv.apn	=	myAPN ;
							msgRcv.control	=	buf ;
							msgRcv.frameType	=	eibNackFrame ;
							_eibPutReceive( myEIB, &msgRcv) ;
						/**
						 *  check for BUSY frame
						 */
						} else if ( buf == 0xc0) {
							msgRcv.apn	=	myAPN ;
							msgRcv.control	=	buf ;
							msgRcv.frameType	=	eibBusyFrame ;
							_eibPutReceive( myEIB, &msgRcv) ;
						/**
						 *  check for L_POLL_DATA frame
						 */
						} else if ( buf == 0xf0) {
							msgRcv.apn	=	myAPN ;
							msgRcv.control	=	buf ;
							msgRcv.frameType	=	eibPollDataFrame ;
						/**
						 *  check for L_DATA.confirm frame
						 */
						} else if ( buf == 0x8b) {
							msgRcv.apn	=	myAPN ;
							msgRcv.control	=	buf ;
							msgRcv.frameType	=	eibDataConfirmFrame ;
							_eibPutReceive( myEIB, &msgRcv) ;
							sndConfirmed	=	1 ;
							knxLog( myKnxLogger, progName, "received positive confirm") ;
						/**
						 *  check for L_DATA.confirm frame
						 */
						} else if ( buf == 0x0b) {
							msgRcv.apn	=	myAPN ;
							msgRcv.control	=	buf ;
							msgRcv.frameType	=	eibDataConfirmFrame ;
							_eibPutReceive( myEIB, &msgRcv) ;
							sndConfirmed	=	1 ;
							knxLog( myKnxLogger, progName, "received negative confirm") ;
						/**
						 *  check for L_DATA.confirm frame
						 */
						} else if ( ( buf & 0x07) == 0x07) {
							msgRcv.apn	=	myAPN ;
							msgRcv.control	=	buf ;
							msgRcv.frameType	=	eibTPUARTStateIndication ;
							_eibPutReceive( myEIB, &msgRcv) ;
						/**
						 *  no valid frame start, consider data to be spurious
						 */
						} else {
							_debug( 0, progName, "SPURIOUS BYTE ... %02x", buf) ;
						}
						break ;
					case	waiting_for_hw_adr	:
						rcvdLength++ ;
						if ( rcvdLength >= 2) {
							rcvMode	=	waiting_for_group_adr ;
							rcvdLength	=	0 ;
							msgRcv.rcvAddr	=	0x0000 ;
						}
						msgRcv.sndAddr	<<=	8 ;
						msgRcv.sndAddr	|=	buf ; ;
						break ;
					case	waiting_for_group_adr	:
						rcvdLength++ ;
						if ( rcvdLength >= 2) {
							rcvMode	=	waiting_for_info ;
						}
						msgRcv.rcvAddr	<<=	8 ;
						msgRcv.rcvAddr	|=	buf ; ;
						break ;
					case	waiting_for_info	:
						msgRcv.info	=	buf ;
						expLength	=	(int) (buf & 0x07) + 1 ;
						rcvdLength	=	0 ;
						rcvMode	=	waiting_for_data ;
						rcvData	=	msgRcv.mtext ;
						break ;
					case	waiting_for_data	:
						*rcvData++	=	buf ;
						rcvdLength++ ;
						if ( rcvdLength >= expLength) {
							rcvMode	=	waiting_for_checksum ;
						}
						break ;
					case	waiting_for_checksum	:
						incompleteMsg	=	0 ;
						rcvMode	=	waiting_for_control ;
						msgRcv.checksum	=	buf ;
						msgRcv.apn	=	myAPN ;
						msgRcv.frameType	=	eibDataFrame ;
						if (( msgRcv.checksum + msgRcv.ownChecksum) == 0xff) {
							_debug( 1, progName, "Checksum ok ...") ;
							_eibPutReceive( myEIB, &msgRcv) ;
						} else {
							_debug( 1, progName, "Checksum crappy ...") ;
							_debug( 1, progName, "will not bridge this message") ;
						}
						break ;
					}
					msgRcv.ownChecksum	^=	buf ;
				}
				rcvdBytes	=	RS232_PollComport(cport_nr, (unsigned char *) &buf, 1) ;
			}
			/**
			 * IF there's a message to send
			 */
			if (( msgToSnd = eibReceive( myEIB, &msgBuf)) != NULL && debugLevel >= 0) {
				/**
				 * IF this message is not coming from my own port
				 */
				if ( msgToSnd->apn != myAPN && sndConfirmed == 1) {
					sndMode	=	sending_control ;
				 	msgToSnd->checksum	=	0x00 ;
					sendingByte	=	0 ;
					while ( sndMode != sending_idle) {
						switch ( sndMode) {
						case	sending_control	:
							_debug( 1, progName, "sending control") ;
							sndConfirmed	=	0 ;
							bufp	=	0x80 + sendingByte ;
							buf	=	msgToSnd->control ;
					 		msgToSnd->checksum	^=	buf ;
							sndMode	=	sending_hw_adr ;
							sentLength	=	0 ;
							break ;
						case	sending_hw_adr	:
							_debug( 1, progName, "sending hw address") ;
							bufp	=	0x80 + sendingByte ;
							if ( sentLength == 0)
								buf	=	msgToSnd->sndAddr >> 8 ;
							else if ( sentLength == 1)
								buf	=	msgToSnd->sndAddr & 0xff ;
				 			msgToSnd->checksum	^=	buf ;
							sentLength++ ;
							if ( sentLength >= 2) {
								sndMode	=	sending_group_adr ;
								sentLength	=	0 ;
							}
							break ;
						case	sending_group_adr	:
							_debug( 1, progName, "sending group address") ;
							bufp	=	0x80 + sendingByte ;
							if ( sentLength == 0)
								buf	=	msgToSnd->rcvAddr >> 8 ;
							else if ( sentLength == 1)
								buf	=	msgToSnd->rcvAddr & 0xff ;
				 			msgToSnd->checksum	^=	buf ;
							sentLength++ ;
							if ( sentLength >= 2) {
								sndMode	=	sending_info ;
							}
							break ;
						case	sending_info	:
							_debug( 1, progName, "sending info") ;
							bufp	=	0x80 + sendingByte ;
							buf	=	msgToSnd->info ;
				 			msgToSnd->checksum	^=	buf ;
							expLength	=	(int) ( msgToSnd->info & 0x07)  + 1 ;
							sentLength	=	0 ;
							sndMode	=	sending_data ;
							break ;
						case	sending_data	:
							if ( debugLevel > 1) {
								_debug( 1, progName, "sending data") ;
							}
							bufp	=	0x80 + sendingByte ;
							buf	=	msgToSnd->mtext[sentLength] ;
				 			msgToSnd->checksum	^=	buf ;
							sentLength++ ;
							if ( sentLength >= expLength) {
								sndMode	=	sending_checksum ;
							}
							break ;
						case	sending_checksum	:
							_debug( 1, progName, "sending checksum %2x", msgToSnd->checksum) ;
							bufp	=	0x40 + sendingByte ;
				 			msgToSnd->checksum	^=	0xff ;
							buf	=	msgToSnd->checksum ;
							sndMode	=	sending_idle ;
							break ;
						}
						RS232_SendBytes( cport_nr, bufp, buf) ;
						sendingByte++ ;
					}
//					dumpMsg( "Sending Message...:\n", msgToSnd) ;
					sndTimeout	=	0 ;
				} else if ( sndConfirmed == 0) {
					knxLog( myKnxLogger, progName, "waiting for confirm") ;
					sndTimeout++ ;
					if ( sndTimeout > 10) {
						sndConfirmed	=	1 ;
						knxLog( myKnxLogger, progName, "timeout waiting for confirm") ;
					}
				} else {
//					sleep( 1) ;
				}
			}
		}
#endif
		eibClose( myEIB) ;
		deletePIDFile( progName, "", ownPID) ;
	} else {
		knxLogRelease( myKnxLogger) ;
		knxLog( myKnxLogger, progName, "%d: process already running ...", ownPID) ;
		_debug( 0, progName, "process already running ...") ;
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

