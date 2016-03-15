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
#include	"mylib.h"

void	dumpData( node *data, int lastDataIndex, int mask, void *knxData) {
		int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
		int	*ints	=	(int *) knxData ;
		float	*floats	=	(float *) knxData ;
	/**
	 *
	 */
	for ( i=0 ; i < lastDataIndex ; i++) {
		if ( data[i].monitor & mask) {
			actTime	=	time( NULL) ;
			myTime	=	gmtime( &actTime) ;
			strftime( timeBuffer,  sizeof( timeBuffer), "%F %T", myTime) ;
			switch ( data[i].type) {
			case	dtBit	:
				printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, bit:   %6d ",
					 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr, data[i].val.i) ;
				if ( data[i].knxGroupAddr != 0) {
					printf( "KNX...: %d", ints[data[i].knxGroupAddr]) ;
				}
				break ;
			case	dtInt1	:
			case	dtUInt1	:
			case	dtInt2	:
			case	dtUInt2	:
			case	dtInt4	:
			case	dtUInt4	:
				printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, int:   %6d ",
					 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr, data[i].val.i) ;
				if ( data[i].knxGroupAddr != 0) {
					printf( "KNX...: %d", ints[data[i].knxGroupAddr]) ;
				}
				break ;
			case	dtFloat2	:
			case	dtFloat4	:
				printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, float: %6.2f ",
					 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr, data[i].val.f) ;
				if ( data[i].knxGroupAddr != 0) {
					printf( "KNX...: %5.2f", floats[data[i].knxGroupAddr]) ;
				}
				break ;
			case	dtString	:
				break ;
			case	dtDate	:
				break ;
			case	dtTime	:
				break ;
			case	dtDateTime	:
				break ;
			}
			printf( "\n") ;
		}
	}
}

void	dumpDataAll( node *data, int lastDataIndex, void *knxData) {
		int	i ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
		int	*ints	=	(int *) knxData ;
		float	*floats	=	(float *) knxData ;
	/**
	 *
	 */
	for ( i=0 ; i < lastDataIndex ; i++) {
		actTime	=	time( NULL) ;
		myTime	=	gmtime( &actTime) ;
		strftime( timeBuffer,  sizeof( timeBuffer), "%F %T", myTime) ;
		switch ( data[i].type) {
		case	dtBit	:
			printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, bit:   %6d ",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr, data[i].val.i) ;
			if ( data[i].knxGroupAddr != 0) {
				printf( "KNX...: %d", ints[data[i].knxGroupAddr]) ;
			}
			break ;
		case	dtInt1	:
		case	dtUInt1	:
		case	dtInt2	:
		case	dtUInt2	:
		case	dtInt4	:
		case	dtUInt4	:
			printf( "Time: %s, Id: %3d/%3d Name: '%-25s', HW: %04x, KNX: %5d, int:   %6d ",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr, data[i].val.i) ;
			if ( data[i].knxGroupAddr != 0) {
				printf( "KNX...: %d", ints[data[i].knxGroupAddr]) ;
			}
			break ;
		case	dtFloat2	:
		case	dtFloat4	:
			printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, float: %6.2f ",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr, data[i].val.f) ;
			if ( data[i].knxGroupAddr != 0) {
				printf( "KNX...: %5.2f", floats[data[i].knxGroupAddr]) ;
			}
			break ;
		case	dtString	:
			break ;
		case	dtDate	:
			printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, >>>>Date",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr) ;
			break ;
		case	dtTime	:
			printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, >>>>Time",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr) ;
			break ;
		case	dtDateTime	:
			printf( "Time: %s, Id: %3d/%3d, Name: '%-25s', HW: %04x, KNX: %5d, >>>>DateTime",
				 timeBuffer, data[i].id, i, data[i].name, data[i].knxHWAddr, data[i].knxGroupAddr) ;
			break ;
		}
		printf( "\n") ;
	}
}

void	createCRF( node *data, int lastDataIndex, int *crf, void *knxData) {
		int	i ;
		int	*ints ;
		float	*floats ;
	/**
	 *
	 */
	ints	=	(int *) knxData ;
	floats	=	(float *) knxData ;
	/**
	 *
	 */
	for ( i=0 ; i < 65536 ; i++) {
		crf[i]	=	0 ;
	}
	for ( i=0 ; i < lastDataIndex ; i++) {
		if ( data[i].knxGroupAddr != 0) {
			crf[ data[i].knxGroupAddr]	=	i ;
			switch ( data[i].type) {
			case	dtBit	:
				data[i].val.i	=	ints[data[i].knxGroupAddr] ;
				break ;
			case	dtInt1	:
			case	dtUInt1	:
			case	dtInt2	:
			case	dtUInt2	:
			case	dtInt4	:
			case	dtUInt4	:
				data[i].val.i	=	ints[data[i].knxGroupAddr] ;
				break ;
			case	dtFloat2	:
			case	dtFloat4	:
				data[i].val.f	=	floats[data[i].knxGroupAddr] ;
				break ;
			case	dtString	:
				break ;
			case	dtDate	:
				break ;
			case	dtTime	:
				break ;
			case	dtDateTime	:
				break ;
			}
		}
	}
}

int	getEntry( node *data, int lastDataIndex, char *name) {
	int	i, j ;
	j	=	-1 ;
	for ( i=0 ; i<lastDataIndex && j == -1 ; i++) {
		if ( strcmp( data[i].name, name) == 0 || strcmp( data[i].alias, name) == 0) {
			j	=	i ;
		}
	}
	return j ;
}

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};


char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

	int	i, j ;

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for ( i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for ( i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}


unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length) {
	int	i, j ;

    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for ( i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}


void build_decoding_table() {
	int	i ;

    decoding_table = malloc(256);

    for ( i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}


void base64_cleanup() {
    free(decoding_table);
}

int	createPIDFile( char *_progName, char *_apdx, pid_t _pid) {
	FILE	*myFile ;
	char	filename[256] ;
	int	exitStatus	=	1 ;
	sprintf( filename, "/tmp/%s_%s", _progName, _apdx) ;
	if (( myFile = fopen( filename, "r")) == NULL) {
		if (( myFile = fopen( filename, "w+")) != NULL) {
			fprintf( myFile, "%d", _pid) ;
			fclose( myFile) ;
		}
	} else {
		exitStatus	=	0 ;
	}
	return exitStatus ;
		
}
void	deletePIDFile( char *_progName, char *_apdx, pid_t _pid) {
	FILE	*myFile ;
	char	filename[256] ;
	sprintf( filename, "/tmp/%s_%s", _progName, _apdx) ;
	if ( remove( filename) != 0) {
		_debug( 0, _progName, "can't delete PID file [`%s']", filename) ;
	}
}
