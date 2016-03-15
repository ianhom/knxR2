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
 *
 */
#ifndef eiblog_INCLUDED
#define eiblog_INCLUDED

#define	SEM_EIBLOG_KEY	10042

typedef	struct	{
			int	flags ;
			key_t	semEibLogKey ;
			int	semEibLog ;
		struct	sembuf	semEibLogOp[1] ;
			char	caller[64] ;
			key_t	shmKey ;
			key_t	semKey ;
	}	eibLogHdl ;

extern	eibLogHdl	*eibLogOpen( char *, int) ;
extern	void		eibLogClose( eibLogHdl *) ;
extern	void		eibLog( eibLogHdl *, char *mesg)  ;
/**
 *
 */

#endif
