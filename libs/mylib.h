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

#ifndef mylib_INCLUDED
#define mylib_INCLUDED

/**
 *
 */

typedef	enum	{
		dtBit		=	1
	,	dtInt1		=	11
	,	dtUInt1		=	12
	,	dtInt2		=	13
	,	dtUInt2		=	14
	,	dtInt4		=	15
	,	dtUInt4		=	16
	,	dtFloat2	=	21
	,	dtFloat4	=	22
	,	dtString	=	31
	,	dtTime		=	41
	,	dtDate		=	42
	,	dtDateTime	=	43
	}	dataType ;

typedef	enum	{
		srcKNX		=	1
	,	srcODS		=	2
	,	srcOTHER	=	3
	}	dataOrigin ;

typedef	union	{
		int	i ;
		float	f ;
		double	d ;
		char	s[64] ;
	}	value ;

typedef	struct	{
		int		id ;
		char		name[64] ;
		char		alias[32] ;
		dataType	type ;
		int		knxHWAddr ;
		int		knxGroupAddr ;
		value		val ;
		int		monitor ;
		dataOrigin	origin ;
		int		log ;
	}	node ;

extern	void	dumpData( node *, int, int, void *) ;
extern	void	dumpDataAll( node *, int, void *) ;
extern	void	createCRF( node *, int, int *, void *) ;
extern	int	getEntry( node *, int, char *) ;

extern	char *base64_encode(const unsigned char *, size_t, size_t *) ;
extern	unsigned	char *base64_decode(const char *, size_t, size_t *) ;
extern	void	build_decoding_table() ;
extern	void	base64_cleanup() ;

extern	int	createPIDFile( char *, char *, pid_t) ;
extern	void	deletePIDFile( char *, char *, pid_t) ;

#endif
