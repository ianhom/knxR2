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
 * eiblog.c
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
#include "eiblog.h"
/**
 *
 */
eibLogHdl	*eibLogOpen( char *_caller, int _flags) {
	eibLogHdl	*this ;
	/**
	 *
	 */
	this	=	(eibLogHdl *) malloc( sizeof( eibLogHdl)) ;
	this->flags	=	_flags ;
	this->semEibLogKey	=	SEM_EIBLOG_KEY ;
	strcpy( this->caller, _caller) ;
	/**
	 * create the semaphore for the receive queue
	 */
	if (( this->semEibLog = semget( this->semEibLogKey, 1, this->flags | 0600)) < 0) {
		if (( this->semEibLog = semget( this->semEibLogKey, 1, IPC_CREAT | 0600)) < 0) {
			exit( -1) ;
		}
		semctl( this->semEibLog, 0, SETVAL, (int) 1) ;		// make the semaphore available
	}
	/**
	 *
	 */
	this->semEibLogOp->sem_op		=	1 ;
	this->semEibLogOp->sem_flg	=	SEM_UNDO ;
	if( semop ( this->semEibLog, this->semEibLogOp, 1) == -1) {
		perror( " semop ") ;
		exit ( EXIT_FAILURE) ;
	}
	eibLog( this, "started ...") ;
	return this ;
}
/**
 *
 */
void	eiLogClose( eibLogHdl *this) {
	/**
	 * if we are the process who originally created the communication stuff
	 */
	if (( this->flags & IPC_CREAT) == IPC_CREAT) {
		/**
		 * destroy the semaphores
		 */
		semctl( this->semEibLog, 1, IPC_RMID, NULL) ;
	}
	free( this) ;
}
/**
 * eibLog
 *
 * put an outbound message into the internal queue "to-be-send"
 */
void	eibLog( eibLogHdl *this, char *msg) {
	time_t		t ;
	struct	tm	tm ;
	FILE		*file ;
	char		cTime[64] ;
	/**
	 * we need to w:ait for the semaphore
	 */
	t	=	time( NULL) ;
	tm	=	*localtime( &t) ;
	sprintf( cTime, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	if (( file = fopen( "/var/log/eib.log", "a+")) != NULL) {
		fprintf( file, "%s:[%s] %s\n", this->caller, cTime, msg) ;
		fclose( file) ;
	}
}
