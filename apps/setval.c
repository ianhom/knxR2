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
 * setval.c
 *
 * set a value immediately in the knxmon shared memory
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-11-20	PA1	userId	inception;
 *
 */
#include	<stdio.h>
#include	<unistd.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<time.h>
#include	<sys/types.h>
#include	<sys/ipc.h> 
#include	<sys/shm.h> 
#include	<sys/msg.h> 

#include	"nodeinfo.h"

extern	void	help() ;

char	progName[32] ;

int	main( int argc, char *argv[]) {
		int	status		=	0 ;
		time_t	actTime ;
	struct	tm	*myTime ;
		char	timeBuffer[64] ;
	/**
	 * define application specific variables
	 */
		int	i ;
		value	myData ;
	/**
	 * define shared memory segment #0: COM Table
 	 */
		key_t	shmCOMKey	=	SHM_COM_KEY ;
		int	shmCOMFlg	=	IPC_CREAT | 0600 ;
		int	shmCOMId ;
		int	shmCOMSize	=	256 ;
		int	*sizeTable ;
	/**
	 * define shared memory segment #1: OPC Table with buffer
 	 */
		key_t	shmOPCKey	=	SHM_OPC_KEY ;
		int	shmOPCFlg	=	IPC_CREAT | 0600 ;
		int	shmOPCId ;
		int	shmOPCSize ;
		node	*data ;
	/**
	 * define shared memory segment #2: KNX Table with buffer
 	 */
		key_t	shmKNXKey	=	SHM_KNX_KEY ;
		int	shmKNXFlg	=	IPC_CREAT | 0600 ;
		int	shmKNXId ;
		int	shmKNXSize	=	65536 * sizeof( float) ;
		float	*floats ;
		int	*ints ;
	/**
	 * define shared memory segment #2: KNX Table with buffer
 	 */
		key_t	shmCRFKey	=	SHM_CRF_KEY ;
		int	shmCRFFlg	=	IPC_CREAT | 0600 ;
		int	shmCRFId ;
		int	shmCRFSize	=	65536 * sizeof( int) ;
		int	*crf ;
	/**
	 * general preparation
	 */
	strcpy( progName, *argv++) ;
	/**
	 * setup the shared memory for COMtable
	 */
	if ( argc < 3) {
		help() ;
		exit( -1) ;
	}
	printf( "trying to obtain shared memory COMtable ... \n") ;
	if (( shmCOMId = shmget( shmCOMKey, shmCOMSize, 0600)) < 0) {
		perror( "knxmon: shmget failed for COMtable");
		exit( -1) ;
	}
	printf( "trying to attach shared memory COMtable \n") ;
	if (( sizeTable = (int *) shmat( shmCOMId, NULL, 0)) == (int *) -1) {
		perror( "knxmon: shmat failed for COMtable");
		exit( -1) ;
	}
	shmCOMSize	=	sizeTable[ SIZE_COM_TABLE] ;
	shmOPCSize	=	sizeTable[ SIZE_OPC_TABLE] ;
	shmKNXSize	=	sizeTable[ SIZE_KNX_TABLE] ;
	shmCRFSize	=	sizeTable[ SIZE_CRF_TABLE] ;
	/**
	 *
	 */
#include	"_shmblock.c"
	/**
	 *
	 */	
	i	=	atoi( *argv++) ;
	if ( i < 0 || i >= lastDataIndexC) {
		printf( "%s: index out of range\n", progName) ;
		printf( "%s: valid range 0...%d\n", progName, (int) ( lastDataIndexC-1)) ;
		exit( -2) ;
	}
	switch( data[i].type) {
	case	dtBit	:
	case	dtInt1	:
	case	dtUInt1	:
	case	dtInt2	:
	case	dtUInt2	:
	case	dtInt4	:
	case	dtUInt4	:
		myData.i	=	atoi( *argv++) ;
		data[ i].val.i	=	myData.i ;
		if ( data[i].knxGroupAddr != 0) {
			printf( "setting value at KNX group address %d to %d\n", data[i].knxGroupAddr, myData.i) ;
			ints[data[i].knxGroupAddr]	=	myData.i ;
		}
		break ;
	case	dtFloat2	:
	case	dtFloat4	:
		myData.f	=	atof( *argv++) ;
		data[ i].val.f	=	myData.f ;
		if ( data[i].knxGroupAddr != 0) {
			printf( "setting value at KNX group address %d to %f\n", data[i].knxGroupAddr, myData.f) ;
			floats[data[i].knxGroupAddr]	=	myData.f ;
		}
		break ;
	case	dtString	:
		break ;
	case	dtTime	:
		break ;
	case	dtDate	:
		break ;
	case	dtDateTime	:
		break ;
	default:
		printf( "invalid datatype at index %d\n", i) ;
	}
	exit( status) ;
}

void	help() {
	printf( "%s: setting an arbitrary value index by <nodeIndex> to <value>\n", progName) ;
	printf( "%s: <value> can be either of integer or float type\n", progName) ;
	printf( "%s: syntax: setval <nodeIndex> <value>\n", progName) ;
}
