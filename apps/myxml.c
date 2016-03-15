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
 * myxml.c
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
#include	<libxml/xmlreader.h> 

#include	"debug.h"
#include	"mylib.h"
/**
 * processNode:
 * @reader: the xmlReader
 *
 * Dump information about the current node
 */
int	count( xmlTextReaderPtr reader ) {
	const	xmlChar *name, *value;
	int	count ;

	name = xmlTextReaderConstName(reader);
	if (name == NULL)
	name = BAD_CAST "--";

	count	=	0 ;
	while ( xmlTextReaderRead( reader) == 1) {
		switch ( xmlTextReaderNodeType(reader)) {
		case	XML_READER_TYPE_ELEMENT	:
			if ( strcmp((char *) xmlTextReaderName( reader), "groupobject") == 0) {
				count++ ;
			}
			break ;
		}
	}
//	printf( ".............: contains %d lines \n", count) ;
	return count ;
}

int	lookupDPT( char *_dpt) {
	int	ret	=	-1 ;
	if ( strncmp( _dpt, "1.", 2) == 0) {
		ret	=	dtBit ;
	} else if ( strncmp( _dpt, "9.", 2) == 0) {
		ret	=	dtFloat2 ;
	} else if ( strncmp( _dpt, "10.", 3) == 0) {
		ret	=	dtDate ;
	} else if ( strncmp( _dpt, "11.", 3) == 0) {
		ret	=	dtTime ;
	} else if ( strncmp( _dpt, "12.", 3) == 0) {
		ret	=	dtDateTime ;
	} else {
	}
	return ret ;
}

void	processNode( xmlTextReaderPtr reader, node *nodeTable) {
			int	index ;
	const xmlChar *name, *value;

	name = xmlTextReaderConstName(reader);
	if (name == NULL)
	name = BAD_CAST "--";

	index	=	0 ;
        while ( xmlTextReaderRead( reader) == 1) {
		switch ( xmlTextReaderNodeType(reader)) {
		case	XML_READER_TYPE_ELEMENT	:
			if ( strcmp((char *) xmlTextReaderName( reader), "groupobject") == 0) {
				strcpy( nodeTable[index].name, (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "name")) ;
				strcpy( nodeTable[index].alias, (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "alias")) ;
				nodeTable[index].knxGroupAddr	=	atoi( (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "knxGroupAddr")) ;
				nodeTable[index].type	=	lookupDPT( (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "dpt")) ;
//				printf( "Element node\n") ;
//				printf( "  Id............: %s \n", (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "id")) ;
//				printf( "  Name..........: '%s' \n", (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "name")) ;
//				printf( "  DPT...........: %s(%d) \n", (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "dpt"), nodeTable[index].type) ;
//				printf( "  Group addr....: %s \n", (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "groupAddr")) ;
//				printf( "  Trigger only..: %s \n", (char *) xmlTextReaderGetAttribute( reader, (xmlChar *) "trigger")) ;
				index++ ;
			}
			break ;
		}
	}
}

/**
 * streamFile:
 * @filename: the file name to parse
 *
 * Parse, validate and print information about an XML file.
 */
node	*getNodeTable( const char *filename, int *_count) {
	xmlTextReaderPtr reader;
	int ret;
	node	*nodeTable ;
	nodeTable	=	NULL ;
	/*
	* Pass some special parsing options to activate DTD attribute defaulting,
	* entities substitution and DTD validation
	*/
//	reader = xmlReaderForFile(filename, NULL,
//				XML_PARSE_DTDATTR |  /* default DTD attributes */
//				XML_PARSE_NOENT |    /* substitute entities */
//				XML_PARSE_DTDVALID); /* validate with the DTD */
	reader = xmlReaderForFile(filename, NULL, XML_PARSE_NOENT) ;
	if (reader != NULL) {
		*_count	=	count( reader);
		nodeTable	=	malloc( *_count * ( sizeof( node))) ;
		xmlFreeTextReader(reader) ;
		/**
		 * re-open the xml file and build the table
		 */
		reader	 =	xmlReaderForFile(filename, NULL, XML_PARSE_NOENT) ;
		processNode( reader, nodeTable) ;
		/**
		 * Once the document has been fully parsed check the validation results
		 */
		if ( xmlTextReaderIsValid( reader) != 1) {
			fprintf(stderr, "Document %s does not validate\n", filename);
		}
		xmlFreeTextReader(reader);
		if (ret != 0) {
			fprintf(stderr, "%s : failed to parse\n", filename);
		}
	} else {
		fprintf(stderr, "Unable to open %s\n", filename);
	}
	return nodeTable ;
}
