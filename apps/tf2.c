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
 * tf2.c
 *
 * test routine for the 2-byte half-float conversion to and from
 * 2-byte char[]
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	userId	inception;
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include	"debug.h"
#include	"nodeinfo.h"
#include	"eib.h"

int	debugLevel ;

int main( int argc, char *argv[]) {
	float	val ;
	int	i ;
	int	n ;
	unsigned	char	buf[8] ;

	val	=	atof( argv[ 1]) ;

	fthfb( val, buf) ;
	printf( "%8.2f -> %02x %02x\n", val, buf[0], buf[1]) ;
	printf( "%02x %02x -> %8.2f\n", buf[0], buf[1], hfbtf( buf)) ;

	exit( 0);
}
