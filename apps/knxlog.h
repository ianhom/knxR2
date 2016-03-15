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
 * knxlog.h
 *
 * include file for ib.c
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-12-27	PA1	khw	inception;
 * 2016-01-27	PA2	khw	removed knxLogC function and added va_list;
 *
 */
#ifndef knxlog_INCLUDED
#define knxlog_INCLUDED

#include	<sys/ipc.h> 
#include	<sys/shm.h> 
#include	<sys/msg.h> 
#include	<sys/sem.h> 

#define	SEM_KNXLOG_KEY	10041

typedef	struct	{
			int	flags ;
			key_t	semKnxLogKey ;
			int	semKnxLog ;
		struct	sembuf	semKnxLogOp[1] ;
			char	caller[64] ;
	}	knxLogHdl ;

extern	knxLogHdl	*knxLogOpen( int) ;
extern	void	knxLogClose( knxLogHdl *) ;
extern	void	knxLogRelease( knxLogHdl *) ;
extern	void	knxLog( knxLogHdl *, char *mesg, char *, ...)  ;
/**
 *
 */

#endif
