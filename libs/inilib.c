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
 * mylib.c
 *
 * some useful functions
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	userId	inception;
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<string.h>
#include	<strings.h>
#include	<time.h>
#include	<math.h>
#include	<sys/types.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>

#include	"debug.h"
#include	"inilib.h"

ini	*iniFromFile( char *_file) {
		int	status = -1 ;
		int	i ;
		ini	*this ;
		int	lc ;
		char	line[128] ;
	this	=	malloc( sizeof( ini)) ;
	if ( this) {
		this->iniFile	=	fopen( _file, "r") ;
		if ( this->iniFile) {
			lc	=	0 ;
			while ( fgets( line, 128, this->iniFile)) {
				lc++ ;
				printf( "%5d : Line ... : '%s", lc, line) ;
			}
			*this->lines	=	malloc( sizeof( char *) * ( lc+1)) ;
			this->lines[lc]	=	NULL ;
			rewind( this->iniFile) ;
			lc	=	0 ;
			while ( fgets( line, 128, this->iniFile)) {
				this->lines[lc]	=	malloc( strlen( line)+2) ;
				strcpy( this->lines[lc], line) ;
				printf( "%5d : Line .>. : '%s", lc, this->lines[lc]) ;
				lc++ ;
 			}
			fclose( this->iniFile) ;
		} else {
			printf( "inilib.c: could not open(find?) ini file\n") ;
		}
	} else {
		printf( "inilib.c: could not allocate buffer\n") ;
	}
	return this ;
}

char	*getPara( ini *this, char *_block, char *_name, char *_buf) {
	char	**lp ;
	int	inBlock	=	0 ;
	int	tc ;
	char	*p, para[64] ;
	lp	=	this->lines ;
	while ( *lp != NULL) {
		printf( "%s", *lp) ;
		if ( strncmp( _block, *lp, strlen( _block)) == 0 && inBlock == 0) {
			inBlock	=	1 ;
			printf( "found block ... \n") ;
		} else if ( inBlock == 1) {
			if ( *lp[0] == '[') {
				printf( "found end-of-block\n") ;
				inBlock	=	0 ;
			} else {
				tc	=	0 ;
				for ( p=strtok( *lp, " \t\n") ; p != NULL ; p=strtok( NULL, " \t\n")) {
					switch ( tc) {
					case	0	:
						strcpy( para, p) ;
						break ;
					case	1	:
						break ;
					case	2	:
						if ( strcmp( _name, para) == 0) {
							strcpy( _buf, p) ;
						}
						break ;
					default	:
						break ;
					}
					tc++ ;
				}
			}
		}
		lp++ ;
	}
	return _buf ;
}

void dump( ini *this) {
	char	**lp ;
	lp	=	this->lines ;
	while ( *lp != NULL) {
		printf( "%s", *lp++) ;
	}
}

ini	*release( ini *this) {
	char	**lp ;
	if ( this) {
		lp	=	this->lines ;
		while ( *lp != NULL) {
			free( *lp++) ;
		}
//		free( *this->lines) ;
		free( this) ;
		this	=	NULL ;
	} else {
	}
	return this ;
}
