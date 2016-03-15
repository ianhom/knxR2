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
 * knxlog.c
 *
 * some high level functional description
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-12-27	PA1	khw	inception;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdarg.h>
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
/**
 *
 */
#include "knxlog.h"
/**
 *
 */
knxLogHdl	*knxLogOpen( int _flags) {
	knxLogHdl	*this ;
	/**
	 *
	 */
	this	=	(knxLogHdl *) malloc( sizeof( knxLogHdl)) ;
	this->flags	=	_flags ;
	this->semKnxLogKey	=	SEM_KNXLOG_KEY ;
	/**
	 * create the semaphore for the receive queue
	 */
	if (( this->semKnxLog = semget( this->semKnxLogKey, 1, this->flags | 0600)) < 0) {
		if (( this->semKnxLog = semget( this->semKnxLogKey, 1, IPC_CREAT | 0600)) < 0) {
			exit( -1) ;
		}
		semctl( this->semKnxLog, 0, SETVAL, (int) 1) ;		// make the semaphore available
	}
	/**
	 *
	 */
	this->semKnxLogOp->sem_op		=	1 ;
	this->semKnxLogOp->sem_flg	=	SEM_UNDO ;
	if( semop ( this->semKnxLog, this->semKnxLogOp, 1) == -1) {
		perror( " semop ") ;
		exit ( EXIT_FAILURE) ;
	}
	return this ;
}
/**
 *
 */
void	knxLogClose( knxLogHdl *this) {
	/**
	 * if we are the process who originally created the communication stuff
	 */
	if (( this->flags & IPC_CREAT) == IPC_CREAT) {
		/**
		 * destroy the semaphores
		 */
		if ( semctl( this->semKnxLog, 1, IPC_RMID, NULL) < 0) {
			printf( "................> FAIL \n") ;
		}
	}
	free( this) ;
}
/**
 *
 */
void	knxLogRelease( knxLogHdl *this) {
	/**
	 * if we are the process who originally created the communication stuff
	 */
	this->flags	=	0 ;
}
/**
 * knxLog
 *
 * put an outbound message into the internal queue "to-be-send"
 */
void	knxLog( knxLogHdl *this, char *_progName, char *_msg, ...) {
	time_t		t ;
	struct	tm	tm ;
	FILE		*file ;
	char		cTime[64] ;
	char		buffer[256] ;
	va_list		ap ;
	/**
	 * we need to wait for the semaphore
	 */
	va_start( ap, _msg) ;
	t	=	time( NULL) ;
	tm	=	*localtime( &t) ;
	sprintf( cTime, "%4d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	vsprintf( buffer, _msg, ap) ;
	if (( file = fopen( "/var/log/knx.log", "a+")) != NULL) {
		fprintf( file, "%s:[%s] %s\n", cTime, _progName, buffer) ;
		fclose( file) ;
	}
}
