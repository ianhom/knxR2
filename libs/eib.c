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
 * eib.c
 *
 * some high level functional description
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-12-04	PA1	khw	inception;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<errno.h>
#include	<time.h>
#include	<math.h>
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<unistd.h>
#include	<sys/signal.h>
/**
 *
 */
#include "eib.h"
#include "debug.h"
/**
 * define knxBus buffer (shared memory segment)
 */
/**
 *
 */
eibHdl	*eibOpen( unsigned int _sndAddr, int flags, key_t _key, char *_name, apnMode _mode) {
	eibHdl	*this ;
	int	i ;
	/**
	 *
	 */
	this	=	(eibHdl *) malloc( sizeof( eibHdl)) ;
	this->flags	=	flags ;
	/**
	 * general setup
	 */
	this->sndAddr	=	_sndAddr ;
	this->shmKey	=	_key ;
	this->semKey	=	_key ;
	this->apn	=	0 ;
	/**
	 * create the simulated knx-bus
	 */
//	_debug( 1, "eib.c", "trying to attach send message queue ... \n") ;
	if (( this->shmKnxBusId = shmget ( this->shmKey, KNX_BUS_SIZE, flags | 0666)) < 0) {
		_debug( 0, "eib.c", "%s: could not create knxBus shared memory ...\n") ;
		exit( -1) ;
	}
	if (( this->shmKnxBus = (knxBus *) shmat( this->shmKnxBusId, NULL, 0)) == (knxBus *) -1) {
		_debug( 0, "eib.c", "%s: could not attach knxBus shared memory ...\n") ;
		exit( -1) ;
	}
	if ( flags == IPC_CREAT) {
		this->shmKnxBus->writeRcvIndex	=	0 ;	// write pointer is common to all "users"
                this->shmKnxBus->wdIncr =       0 ;
		/**
		 * create a semaphore set to control read access to the queue
		 */
		if (( this->shmKnxBus->sems = semget( this->semKey + 1, EIB_MAX_APN, flags | 0666)) < 0) {
			_debug( 0, "eib.c", "could not create knxBus queue semaphore per APN ...\n") ;
			exit( -1) ;
		}
		/**
		 *
		 */
		_debug( 0, "eib.c", "resetting per APN semaphore\n") ;
		for ( i=0 ; i < EIB_MAX_APN ; i++) {
			this->shmKnxBus->apns[i]	=	0 ;
			semctl( this->shmKnxBus->sems, i, SETVAL, 0) ;		// reset the semaphore to 0
		}
		eibDumpIPCData( this) ;
	} else {
                _eibAssignAPN( this, _name, _mode) ;
        }
	this->readRcvIndex	=	this->shmKnxBus->writeRcvIndex ;	// these are for each instance of this library
	/**
	 * create the semaphore for the send queue
	 */
	if (( this->semKnxBus = semget( this->semKey, 1, flags | 0666)) < 0) {
		_debug( 0, "eib.c", "%s: could not create knxBus queue semaphore ...\n") ;
		exit( -1) ;
	}
	if ( flags == IPC_CREAT) {
		semctl( this->semKnxBus, 0, SETVAL, (int) 1) ;		// make the semaphore available
	}
	return this ;
}
void	eibDumpIPCData( eibHdl *this) {
	int	i ;
	printf( "Semaphore id ..... : %08lx %d \n", (unsigned long) this->semKey + 1, this->shmKnxBus->sems) ;
	for ( i=0 ; i<EIB_MAX_APN; i++) {
		printf( "   APN: %02d, semVal: %3d, used as APN: %02d, PID: %5d, wdCount: %3d, name: %-32s\n"
                        ,       i
                        ,       semctl( this->shmKnxBus->sems, i, GETVAL)
                        ,       this->shmKnxBus->apns[i]
                        ,       this->shmKnxBus->apnDesc[i].PID
                        ,       this->shmKnxBus->apnDesc[i].wdCount
                        ,       this->shmKnxBus->apnDesc[i].name
                        ) ;
	}
}
/**
 *
 */
void	eibClose( eibHdl *this) {
	/**
	 * if we are the process who originally created the communication stuff
	 */
	_eibReleaseAPN( this) ;
	if (( this->flags & IPC_CREAT) == IPC_CREAT) {
		/**
		 * destroy the semaphores
		 */
		semctl( this->semKnxBus, 2, IPC_RMID, NULL) ;
		/**
		 * destroy shared memory
		 */
		shmdt( this->shmKnxBus) ;					// detach shared memory of knxBus
		shmctl( this->shmKnxBusId, IPC_RMID, NULL) ;
	}
	this->sndAddr	=	0 ;
	free( this) ;
}
void    eibForceCloseAPN( eibHdl *this, int _apn) {
        if ( this->shmKnxBus->apns[ _apn] == _apn) {
		this->shmKnxBus->apns[ _apn]	=	0 ;
                strcpy( this->shmKnxBus->apnDesc[ _apn].name, "(force released)") ;
                this->shmKnxBus->apnDesc[ _apn].wdCount     =       0 ;
                this->shmKnxBus->apnDesc[ _apn].PID     =       0 ;
		semctl( this->shmKnxBus->sems, _apn, SETVAL, (int) 0) ;		// reset the semaphore to 0
        }
}
int	_eibAssignAPN( eibHdl *this, char *_name, apnMode _mode) {
	int	i ;
	/**
	 * skip apn[0] for future special purpose
	 */
	for ( i=1 ; i < EIB_MAX_APN && this->apn == 0 ; i++) {
		if ( this->shmKnxBus->apns[i] == 0) {
			this->apn	=	i ;
                        strcpy( this->shmKnxBus->apnDesc[i].name, _name) ;
                        this->shmKnxBus->apnDesc[i].wdCount     =       0 ;
                        this->shmKnxBus->apnDesc[i].PID     =       getpid() ;
                        this->shmKnxBus->apnDesc[i].mode        =       _mode ;
			this->shmKnxBus->apns[this->apn]	=	i ;
			semctl( this->shmKnxBus->sems, i, SETVAL, (int) 0) ;		// reset the semaphore to 0
		}
	}
	return this->apn ;
}
void	_eibReleaseAPN( eibHdl *this) {
	if ( this->apn >= 0 && this->apn < EIB_MAX_APN) {
		this->shmKnxBus->apns[ this->apn]	=	0 ;
                strcpy( this->shmKnxBus->apnDesc[ this->apn].name, "") ;
                this->shmKnxBus->apnDesc[ this->apn].wdCount     =       0 ;
                this->shmKnxBus->apnDesc[ this->apn].PID     =       0 ;
		semctl( this->shmKnxBus->sems, this->apn, SETVAL, (int) 0) ;		// reset the semaphore to 0
	}
	this->apn	=	0 ;
}
int	eibGetAddr( eibHdl *this) {
	return this->sndAddr ;
}
void	eibSetAddr( eibHdl *this, unsigned int _sndAddr) {
	this->sndAddr	=	_sndAddr ;
}
/**
 * take a knx mesage from the receive queue
 */
knxMsg	*eibReceiveMsg( eibHdl *this, knxMsg *msgBuf) {
	knxMsg	*msgRcvd ;
	/**
	 * we do this in plain priority sequence
	 * if there's something in the internal queue waiting to be sent to the real bus
	 *	send it immediately back to the receive queue
	 * else
	 *	increment a sleep timer up to a max value of 3 seconds
	 *	and goto sleep
	 */
	msgRcvd	=	NULL ;
	/**
	 * we need to wait for the semaphore
	 */
//	_debug( 1, "eib.c", "waiting for receive buffer semaphore\n") ;
	this->semKnxBusOp->sem_op		=	-1 ;
	this->semKnxBusOp->sem_num		=	this->apn ;
	this->semKnxBusOp->sem_flg	=	SEM_UNDO ;
	if ( this->shmKnxBus->apnDesc[this->apn].mode == APN_RDONLY) {
		if( semop ( this->shmKnxBus->sems, this->semKnxBusOp, 1) == -1) {
			perror( " semop ") ;
			exit ( EXIT_FAILURE) ;
		}
	}
        this->shmKnxBus->apnDesc[ this->apn].wdCount     =       0 ;
	this->semKnxBusOp->sem_num		=	0 ;
	if( semop ( this->semKnxBus, this->semKnxBusOp, 1) == -1) {
		perror( " semop ") ;
		exit ( EXIT_FAILURE) ;
	}
	/**
	 * try to receive a message is there is one
	 */
	if (this->readRcvIndex != this->shmKnxBus->writeRcvIndex) {
		memcpy( msgBuf, &this->shmKnxBus->rcvMsg[ this->readRcvIndex], KNX_MSG_SIZE) ;
//		printf( "%d>G:...: read ...: %5d ... write ...: %5d\n", msgBuf->apn, this->readRcvIndex, this->shmKnxBus->writeRcvIndex) ;
		this->readRcvIndex	+=	1 ;
		/**
		 * if we are at the end of the buffer
		 *	reset to beginning
		 */
		if ( this->readRcvIndex >= EIB_QUEUE_SIZE) {
			this->readRcvIndex	=	0 ;
		}
		msgRcvd	=	msgBuf ;
	}
	/**
	 * and release the semaphore
	 */
//	_debug( 1, "eib.c", "releasing receive buffer semaphore\n") ;
	this->semKnxBusOp->sem_num		=	0 ;
	this->semKnxBusOp->sem_op		=	1 ;
	this->semKnxBusOp->sem_flg	=	SEM_UNDO ;
	if( semop ( this->semKnxBus, this->semKnxBusOp, 1) == -1) {
		perror( " semop ") ;
		exit ( EXIT_FAILURE) ;
	}
	/**
	 *
	 */
	if ( msgRcvd != NULL) {
		msgRcvd->tlc	=	msgRcvd->mtext[0] >> 6 ;
		msgRcvd->apci	=	(( msgRcvd->mtext[0] & 0x03) << 2) | (( msgRcvd->mtext[1] & 0xc0) >> 6) ;
	}
	return( msgRcvd) ;
}
/**
 * eibQueueMsg
 *
 * put a message into the internal receive buffer
 *
 */
void	eibQueueMsg( eibHdl *this, knxMsg *msg) {
	int	i ;
	/**
	 * we need to wait for the semaphore
	 */
	this->semKnxBusOp->sem_num		=	0 ;
	this->semKnxBusOp->sem_op		=	-1 ;
	this->semKnxBusOp->sem_flg	=	SEM_UNDO ;
//	_debug( 0, "eib.c", "waiting for semaphore\n");
	if( semop ( this->semKnxBus, this->semKnxBusOp, 1) == -1) {
		_debug( 0, "eib.c", "Can not perform semaphore operation\n");
		exit ( EXIT_FAILURE) ;
	}
        this->shmKnxBus->wdIncr =       5 ;
	/**
	 * put the message in the queue
	 */
//printf( "%d>P:...: read ...: %5d ... write ...: %5d %5d %5d\n", msg->apn, this->readRcvIndex, this->shmKnxBus->writeRcvIndex, msg->sndAddr, msg->rcvAddr) ;
	memcpy( &this->shmKnxBus->rcvMsg[ this->shmKnxBus->writeRcvIndex], msg, KNX_MSG_SIZE) ;
	this->shmKnxBus->writeRcvIndex	+=	1 ;
	/**
	 * if we are at the end of the buffer
	 *	reset to beginning
	 */
	if ( this->shmKnxBus->writeRcvIndex >= EIB_QUEUE_SIZE) {
		this->shmKnxBus->writeRcvIndex	=	0 ;
	}
	/**
	 * and release the semaphore
	 */
	this->semKnxBusOp->sem_op		=	1 ;
	this->semKnxBusOp->sem_flg	=	SEM_UNDO ;
//	_debug( 0, "eib.c", "releasing for semaphore\n");
	if( semop ( this->semKnxBus, this->semKnxBusOp, 1) == -1) {
		perror( " semop ") ;
		_debug( 0, "eib.c", "Can not perform semaphore operation\n");
		exit ( EXIT_FAILURE) ;
	}
	this->semKnxBusOp->sem_op		=	1 ;
	this->semKnxBusOp->sem_flg	=	0 ;
	for ( i=1 ; i < EIB_MAX_APN ; i++) {
		if ( this->shmKnxBus->apns[i] == i && this->shmKnxBus->apnDesc[ i].mode != APN_WRONLY) {
			this->semKnxBusOp->sem_num	=	i ;
			if ( semop ( this->shmKnxBus->sems, this->semKnxBusOp, 1) == -1) {
				_debug( 0, "eib.c", "Can not perform semaphore operation\n");
				exit ( EXIT_FAILURE) ;
			}
		}
	}
//	eibDumpIPCData( this) ;
}
/**
 *
 */
void	dumpMsg( char *header, knxMsg *msg) {
	eibDump( header, msg) ;
}
void	eibDump( char *header, knxMsg *msg) {
	int	length ;
	int	apci ;
	int	il0 ;
	/**
	 *
	 */
	msg->repeat	=	(( msg->control & 0x20) >> 5) ;
	msg->prio	=	(( msg->control & 0x0c) >> 2) ;
	msg->tlc	=	msg->mtext[0] >> 6 ;
	/**
	 *
	 */
	printf( "%s", header) ;
	printf( "  APN......................:  %3d \n", msg->apn) ;
	printf( "  control byte.............:  %02x \n", msg->control) ;
	printf( "    repeat.................:  %02x \n", msg->repeat) ;
	printf( "    priority...............:  %02x \n", msg->prio) ;
	printf( "  physical address.........:  %04x \n", msg->sndAddr) ;
	printf( "  for address..............:  %5d \n", msg->rcvAddr) ;
	printf( "  Info.....................:  %02x \n", msg->info) ;
	length	=	( msg->info & 0x0f) + 1 ;
	printf( "    Length.................:  %02x \n", length) ;
	printf( "    Data...................:  ") ;
	for ( il0=0 ; il0<length ; il0++) {
		printf( "%02x ", msg->mtext[il0]) ;
	}
	printf( "\n") ;
	switch ( msg->tlc) {
	case	0x00	:	// UDP
		msg->apci	=	(( msg->mtext[0] & 0x03) << 2) | (( msg->mtext[1] & 0xc0) >> 6) ;
		printf( "    Type...................:  UDP \n") ;
		printf( "      APCI.................:  %04x \n", msg->apci) ;
		switch ( msg->apci) {
		case	0x00	:
			printf( "        Group Value Read \n") ;
			break ;
		case	0x01	:
			printf( "        Group Value Response \n") ;
			break ;
		case	0x02	:
			printf( "        Group Value Write \n") ;
			break ;
		case	0x03	:
			printf( "        Individual Address Write \n") ;
			break ;
		case	0x04	:
			printf( "        Individual Address Request \n") ;
			break ;
		case	0x05	:
			printf( "        Individual Address Reply \n") ;
			break ;
		case	0x06	:
			printf( "        Adc Read \n") ;
			break ;
		case	0x07	:
			printf( "        Adc Response \n") ;
			break ;
		case	0x08	:
			printf( "        Memory Read \n") ;
			break ;
		case	0x09	:
			printf( "        Memory Response \n") ;
			break ;
		case	0x0a	:
			printf( "        Memory Write \n") ;
			break ;
		case	0x0b	:
			printf( "        User Message \n") ;
			break ;
		case	0x0c	:
			printf( "        Mask Version Read \n") ;
			break ;
		case	0x0d	:
			printf( "        Mask Version Response \n") ;
			break ;
		case	0x0e	:
			printf( "        Restart \n") ;
			break ;
		case	0x0f	:
			printf( "        Escape \n") ;
			break ;
		}
		break ;
	case	0x01	:	// NDP
		msg->seqNo	=	(( msg->mtext[0] & 0x3c) >> 2) ;
		msg->apci	=	(( msg->mtext[0] & 0x03) << 2) | (( msg->mtext[1] & 0xc0) >> 6) ;
		printf( "    Type...................:  NDP \n") ;
		printf( "      Sequence no..........:  %02x \n", msg->seqNo) ;
		printf( "      APCI.................:  %04x \n", msg->apci) ;
		break ;
	case	0x02	:	// UCD
		msg->ppCmd	=	msg->mtext[0] & 0x03 ;
		printf( "    Type...................:  UCD \n") ;
		printf( "      Command..............:  %02x \n", msg->ppCmd) ;
		break ;
	case	0x03	:	// NCD
		msg->seqNo	=	(( msg->mtext[0] & 0x3c) >> 2) ;
		msg->ppConf	=	msg->mtext[0] & 0x03 ;
		printf( "    Type...................:  NCD \n") ;
		printf( "      Sequence no..........:  %02x \n", msg->seqNo) ;
		printf( "      Confirm..............:  %02x \n", msg->ppConf) ;
		break ;
	}
	printf( "    Checksum...............:  %02x \n", msg->checksum) ;
	printf( "    own Checksum...........:  %02x \n", msg->ownChecksum) ;
}
void	eibDisect( knxMsg *msg) {
	int	length ;
	int	apci ;
	int	il0 ;
	/**
	 *
	 */
	msg->repeat	=	(( msg->control & 0x20) >> 5) ;
	msg->prio	=	(( msg->control & 0x0c) >> 2) ;
	msg->length	=	(( msg->info & 0x0f) + 1) ;
	msg->tlc	=	msg->mtext[0] >> 6 ;
	msg->apci	=	(( msg->mtext[0] & 0x03) << 2) | (( msg->mtext[1] & 0xc0) >> 6) ;
	msg->seqNo	=	(( msg->mtext[0] & 0x3c) >> 2) ;
	msg->ppCmd	=	msg->mtext[0] & 0x03 ;
	msg->ppConf	=	msg->mtext[0] & 0x03 ;
}
/**
 *
 */
void	eibWriteBit( eibHdl *this, unsigned int receiver, unsigned char value, unsigned char repeat) {
	knxMsg	myMsg ;
printf( "eib.c::eibWritebit: APN .....: %d %d %d \n", this->apn, this->sndAddr, receiver) ;
	myMsg.apn	=	this->apn ;
	myMsg.frameType	=	eibDataFrame ;
	myMsg.control	=	0x9c | ( repeat << 5);
	myMsg.sndAddr	=	this->sndAddr ;
	myMsg.rcvAddr	=	receiver ;
	myMsg.info	=	0xe0 | 0x01 ;
	myMsg.mtext[0]	=	0x00 ;
	myMsg.mtext[1]	=	0x80 | ( value & 0x01) ;
	eibQueueMsg( this, &myMsg) ;
}
/**
 *
 */
void	eibWriteHalfFloat( eibHdl *this, unsigned int receiver, float value, unsigned char repeat) {
	knxMsg	myMsg ;
	myMsg.apn	=	this->apn ;
	myMsg.frameType	=	eibDataFrame ;
	myMsg.control	=	0x9c | ( repeat << 5);
	myMsg.sndAddr	=	this->sndAddr ;
	myMsg.rcvAddr	=	receiver ;
	myMsg.info	=	0xe0 | 0x03 ;
	myMsg.mtext[0]	=	0x00 ;
	myMsg.mtext[1]	=	0x80 ;
	fthfb( value, &myMsg.mtext[2]) ;
	eibQueueMsg( this, &myMsg) ;
}
/**
 *
 *	data[0]=	weekday
 *	data[1]=	hour
 *	data[2]=	minute
 *	data[3]=	second
 */
void	eibWriteTime( eibHdl *this, unsigned int receiver, int *data, unsigned char repeat) {
	knxMsg	myMsg ;
	myMsg.apn	=	this->apn ;
	myMsg.frameType	=	eibDataFrame ;
	myMsg.control	=	0x9c | ( repeat << 5);
	myMsg.sndAddr	=	this->sndAddr ;
	myMsg.rcvAddr	=	receiver ;
	myMsg.info	=	0xe0 | 0x04 ;
	myMsg.mtext[ 0]	=	0x00 ;
	myMsg.mtext[ 1]	=	0x80 ;
	myMsg.mtext[ 2]	=	(( data[0] & 0x07) << 5) | ( data[1] & 0x1f) ;
	myMsg.mtext[ 3]	=	data[2] & 0x3f ;
	myMsg.mtext[ 4]	=	data[3] & 0x3f ;
	eibQueueMsg( this, &myMsg) ;
}
/**
 * hfbtf	- half-float binary to float
 */
float	hfbtf( const unsigned char buf[]) {
	float	val ;
	int	mant ;
	int	exp ;
	if ( buf[0] & 0x80) {
		mant	=	( 0xfffff800 | ((buf[0] & 0x07) << 8) | buf[1]) ;
		exp	=	(( buf[0] & 0x78) >> 3) ;
		val	=	mant * 0.01 * pow( 2, exp) ;
	} else {
		mant	=	( buf[1] | ((buf[0] & 0x07) << 8)) ;
		exp	=	(( buf[0] & 0x78) >> 3) ;
		val	=	mant * 0.01 * pow( 2, exp) ;
	}
	return( val) ;
}
/**
 * hfbtf	- float to half-float binary
 *
 * hfbtf converts the 4-byte floating point number <val> into a 2-byte half-float
 * and puts the 2-byte represenmtation into <buf>
 */
void	fthfb( float val, unsigned char buf[]) {
	float	rem ;
	int	mant ;
	int	exp ;
	/**
	 *
	 */
	rem	=	val * 100.0 ;
	exp	=	0 ;
	while ( fabs( rem) > 2047) {
		rem	=	rem / 2 ;
		exp++ ;
//		printf( "rem := %f, exp := %d \n", rem, exp) ;
	}
	mant	=	round( rem) ;

//	printf( "mant := %d \n", mant) ;

	buf[ 0]	=	((( mant & 0x80000000) >> 24) | (( mant & 0x0700) >> 8) | (( exp & 0x0f) << 3)) ;
	buf[ 1]	=	mant & 0x00ff ;
}
/**
 *
 */
knxMsg	*getOpenP2PMsg( eibHdl *this, knxMsg *_myMsg, unsigned int _rcv) {
	_myMsg->apn	=	this->apn ;
	_myMsg->control	=	0x90 ;
	_myMsg->sndAddr		=	this->sndAddr ;
	_myMsg->rcvAddr	=	_rcv ;
	_myMsg->info	=	0x60 ;		// 0.......	individual address
						// .xxx....	routing counter
						// ....0000	length = 1
	_myMsg->mtext[0]	=	0x80 ;		// 10......	UCD
						// ..****..	(opt. sequence no.)n
						// ......00	establish connection
	return _myMsg ;
}
/**
 *
 */
knxMsg	*getCloseP2PMsg( eibHdl *this, knxMsg *_myMsg, unsigned int _rcv) {
	_myMsg->apn	=	this->apn ;
	_myMsg->control	=	0x90 ;
	_myMsg->sndAddr		=	this->sndAddr ;
	_myMsg->rcvAddr	=	_rcv ;
	_myMsg->info	=	0x60 ;		// 0.......	individual address
						// .xxx....	routing counter
						// ....0000	length = 1
	_myMsg->mtext[0]	=	0x81 ;		// 10......	UCD
						// ..****..	(opt. sequence no.)n
						// ......01	terminate connection
	return _myMsg ;
}
/**
 *
 */
knxMsg	*getConfP2PMsg( eibHdl *this, knxMsg *_myMsg, unsigned int _rcv) {
	_myMsg->apn	=	this->apn ;
	_myMsg->control	=	0x90 ;
	_myMsg->sndAddr		=	this->sndAddr ;
	_myMsg->rcvAddr	=	_rcv ;
	_myMsg->info	=	0x60 ;		// 0.......	individual address
						// .xxx....	routing counter
						// ....0000	length = 1
	_myMsg->mtext[0]	=	0x82 ;		// 10......	UCD
						// ..****..	(opt. sequence no.)n
						// ......00	establish connection
	return _myMsg ;
}
/**
 *
 */
knxMsg	*getRejectP2PMsg( eibHdl *this, knxMsg *_myMsg, unsigned int _rcv) {
	_myMsg->apn	=	this->apn ;
	_myMsg->control	=	0x90 ;
	_myMsg->sndAddr		=	this->sndAddr ;
	_myMsg->rcvAddr	=	_rcv ;
	_myMsg->info	=	0x60 ;		// 0.......	individual address
						// .xxx....	routing counter
						// ....0000	length = 1
	_myMsg->mtext[0]	=	0x83 ;		// 10......	UCD
						// ..****..	(opt. sequence no.)n
						// ......00	establish connection
	return _myMsg ;
}
/**
 *
 */
knxMsg	*getIndividualAddrRequestMsg( eibHdl *this, knxMsg *_myMsg, unsigned int _rcv) {
	_myMsg->apn	=	this->apn ;
	_myMsg->control		=	0x90 ;		// 10......	MUST BE
							// ..0.....	no repeat
							// ..1.....	MUST BE
							// ....00..	System function
	_myMsg->sndAddr		=	this->sndAddr ;
	_myMsg->rcvAddr		=	_rcv ;
	_myMsg->info		=	0xe1 ;		// 1.......	group address
							// .110....	routing counter = 6
							// ....0001	length = 2
	_myMsg->mtext[0]	=	0x01 ;		// 00......	UDP	==> APCI needs to be there
							// ..xxxx..	not needed seq.no. 0
							// ......01	APCI
	_myMsg->mtext[1]	=	0x00 ;		// 00......	APCI = 04 = IndividualAddressRequest
							// ..******	seq.no. 0
	return _myMsg ;
}
/**
 *
 */
knxMsg	*getIndividualAddrWriteMsg( eibHdl *this, knxMsg *_myMsg, unsigned int _rcv) {
	_myMsg->apn	=	this->apn ;
	_myMsg->control		=	0x90 ;
	_myMsg->sndAddr		=	this->sndAddr ;
	_myMsg->rcvAddr		=	0x0000 ;
	_myMsg->info		=	0xe3 ;		// 1.......	group address
							// .110....	routing counter = 6
							// ....0011	length = 4
	_myMsg->mtext[0]	=	0x00 ;		// 00......	UDP
							// ..xxxx..	not needed seq.no. 0
							// ......00	APCI
	_myMsg->mtext[1]	=	0xc0 ;		// 11......	APCI = 03 = IndividualAddressWrite
							// ..******	seq.no. 0
	_myMsg->mtext[2]	=	( _rcv >> 8) & 0x00ff ;		// data high-byte
	_myMsg->mtext[3]	=	_rcv & 0x00ff ;		// data: low-byte
	return _myMsg ;
}
/**
 *
 */
knxMsg	*getIndividualAddrResponseMsg( eibHdl *this, knxMsg *_myMsg, unsigned int _rcv) {
	_myMsg->apn	=	this->apn ;
	_myMsg->control	=	0x90 ;
	_myMsg->sndAddr		=	this->sndAddr ;
	_myMsg->rcvAddr		=	_rcv ;
	_myMsg->info		=	0xe1 ;	// 1.......	group address
						// .110....	routing counter = 6
						// ....0001	length = 2
	_myMsg->mtext[0]	=	0x01 ;	// 11......	NCP
						// ..0000..	seq.no. 0
						// ......10	connectionConfirmPositiv
	_myMsg->mtext[1]	=	0x40 ;	// 11......	NCP
						// ..0000..	seq.no. 0
						// ......10	connectionConfirmPositiv
	return _myMsg ;
}
