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
 * eib.h
 *
 * include file for eib.c
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2016-01-03	PA1	khw	inception;
 * 2016-02-01	PA2	khw	added frame type enumeration;
 *
 */
#ifndef eib_INCLUDED
#define eib_INCLUDED

#include	<sys/sem.h>

/**
 * description of a knx bus message (as going though the internal message queues)
 *
 * byte #	signficance
 *      0	control byte must be of binary shape b10x1xx00,
 *		i.e. BYTE & 0xd3 == 0x90 matches a valid control byte
 *      1	sender address high byte
 *      2	sender address low byte+
 *      3	receiver address high byte
 *      4	receiver address low byte
 *      5	info, bits bxxxx1111 signfiy length of packet, 0= length 1, 1= length 2 etc,
 *      6	message[0]info, buts bxxxx1111 signfiy length if packe, 0= length 1, 1= length 2 etc,
 *  7..21	message[1]..[15], message
 *  8..22	checksum
 */

typedef	enum	{
		eibDataFrame
	,	eibAckFrame
	,	eibNackFrame
	,	eibBusyFrame
	,	eibPollDataFrame
	,	eibDataConfirmFrame
	,	eibOtherFrame
	,	eibTPUARTStateIndication
	,	bridgeCtrlFrame				// bridge control frame (inter-IP-Bridge)
	}	eibFrameType ;

typedef struct {
                		int	apn ;		// message priority
							//	1= top priority,
							//	2= medium priority,
							//	3= low priority
		eibFrameType		frameType ;
		unsigned	char	control ;	// control byte
							//  b=..x.....     0= do not repeat
							//		   1= repeat
							//  b=....xx..    00= system function
							//		  01= alarm function
							//		  02= high priority data
							//		  03= low priority data
		unsigned	int	sndAddr ;	// sender address, here: H/W address
		unsigned	int	rcvAddr ;	// receiver address, here: group address
                unsigned	char	info ;		// N_PDU message information:
							//   b=x.......	   0= Individual address
							//		   1= Group address
							//   b=.xxx....	0..7=	Routing counter
							//   b=....xxxx 0..15= useful message length
		unsigned	int	length ;	// length of mtext (( = info & 0x0f) + 1)
                unsigned	char	mtext[16] ;	// useful message
                unsigned	char	checksum ;	// received checksum
                unsigned	char	ownChecksum ;	// own calculated checksum (during reception)
		/**
		 * the following values are crteated during disassembly
		 */
		unsigned	char	repeat ;	// repeate message
		unsigned	char	prio ;		// priority
		unsigned	char	tlc ;		// from mtext[0]:xx......
							//	b00	UDP
							//	b01	NDP
							//	b10	UCD
							//	b11	NCD
		unsigned	char	seqNo ;		// sequence number:
							// from mtext[0]:..xxxx..
		unsigned	char	apci ;		// command identification
							// from mtext[0]:......xx and
							//      mtext[1]:xx......
		unsigned	char	ppCmd ;		// p2p command
							//	b00	open P2P commuincation
							//	b01	close/break down p2p
							//		communication
		unsigned	char	ppConf ;	// p2p Comfirmation
							//	b10	confirm ok
							//	b11	confirm not ok
	} knxMsg ;
/**
 *
 */
typedef	enum	apnMode		{
		APN_RDWR	=	0
	,	APN_RDONLY	=	1
	,	APN_WRONLY	=	2
	} apnMode ;
typedef	struct	apnDescr	{
		char	name[32] ;
		apnMode	mode ;
		pid_t	PID ;
		int	wdCount ;
	} apnDescr ;

#define	KNX_ORIGIN_TP	0x01
#define	KNX_ORIGIN_IP	0x02
#define	KNX_ORIGIN_INT	0x03

#define	KNX_MSG_SIZE	sizeof( knxMsg)

#define	EIB_QUEUE_SIZE	750
#define	EIB_MAX_APN		32

/**
 * the following structure will be located in shared memory!
 */
typedef	struct	knxBus	{
		int	writeRcvIndex ;
		int	readRcvIndex ;
		knxMsg	rcvMsg[EIB_QUEUE_SIZE] ;		// messages received through the bus
		int	apns[EIB_MAX_APN] ;
		apnDescr	apnDesc[EIB_MAX_APN] ;
		key_t	sems ;					// semaphores to "trigger" the reading processes
		int	wdIncr ;
	}	knxBus ;

#define	KNX_BUS_SIZE	sizeof( knxBus)

typedef	struct	eibHdl	{
			int	flags ;
			key_t	shmKey ;
			key_t	semKey ;
			int	shmKnxBusId ;
			knxBus	*shmKnxBus ;
			int	semKnxBus ;
	struct		sembuf	semKnxBusOp[1] ;
	unsigned	int	sndAddr ;
			int	readRcvIndex ;
//			int	readSndIndex ;
			int	apn ;				// access point no.
	}	eibHdl ;

typedef	struct	{
			void	(*cbOpenP2P)( eibHdl *, knxMsg *) ;
			void	(*cbCloseP2P)( eibHdl *, knxMsg *) ;
			void	(*cbConfP2P)( eibHdl *, knxMsg *) ;
			void	(*cbRejectP2P)( eibHdl *, knxMsg *) ;
			void	(*cbGroupValueRead)( eibHdl *, knxMsg *) ;
			void	(*cbGroupValueResponse)( eibHdl *, knxMsg *) ;
			void	(*cbGroupValueWrite)( eibHdl *, knxMsg *) ;
			void	(*cbIndividualAddrWrite)( eibHdl *, knxMsg *) ;
			void	(*cbIndividualAddrRequest)( eibHdl *, knxMsg *) ;
			void	(*cbIndividualAddrResponse)( eibHdl *, knxMsg *) ;
			void	(*cbAdcRead)( eibHdl *, knxMsg *) ;
			void	(*cbAdcResponse)( eibHdl *, knxMsg *) ;
			void	(*cbMemoryRead)( eibHdl *, knxMsg *) ;
			void	(*cbMemoryResponse)( eibHdl *, knxMsg *) ;
			void	(*cbMemoryWrite)( eibHdl *, knxMsg *) ;
			void	(*cbUserMessage)( eibHdl *, knxMsg *) ;
			void	(*cbMaskVersionRead)( eibHdl *, knxMsg *) ;
			void	(*cbMaskVersionResponse)( eibHdl *, knxMsg *) ;
			void	(*cbRestart)( eibHdl *, knxMsg *) ;
			void	(*cbEscape)( eibHdl *, knxMsg *) ;
	}	eibCallbacks ;

extern	eibHdl	*eibOpen( unsigned int, int, key_t, char *, apnMode) ;
extern	void	eibClose( eibHdl *) ;
extern	void	eibForceCloseAPN( eibHdl *, int) ;
extern	int	_eibAssignAPN( eibHdl *, char *_apnName, apnMode) ;
extern	void	_eibReleaseAPN( eibHdl *) ;
extern	void	eibSend( eibHdl *, knxMsg *) ;
extern	knxMsg	*eibReceiveMsg( eibHdl *, knxMsg *) ;
extern	void	eibWriteBit( eibHdl *, unsigned int, unsigned char, unsigned char) ;
extern	void	eibWriteHalfFloat( eibHdl *, unsigned int, float, unsigned char) ;
extern	void	eibWriteTime( eibHdl *, unsigned int, int *, unsigned char) ;
extern	void	dumpMsg( char *, knxMsg *) ;
extern	void	eibDump( char *, knxMsg *) ;
extern	void	eibDisect( knxMsg *) ;
extern	knxMsg	*_eibGetSend( eibHdl *, knxMsg *) ;
extern	void	eibQueueMsg(eibHdl *, knxMsg *)  ;
extern	int	eibGetAddr( eibHdl *) ;
extern	void	eibSetAddr( eibHdl *, unsigned int) ;
extern	void	eibDumpIPCData( eibHdl *) ;
/**
 *
 */
extern	knxMsg	*getOpenP2PMsg( eibHdl *hdl, knxMsg *, unsigned int ) ;
extern	knxMsg	*getConfP2PMsg( eibHdl *hdl, knxMsg *, unsigned int ) ;
extern	knxMsg	*getCloseP2PMsg( eibHdl *hdl, knxMsg *, unsigned int ) ;
extern	knxMsg	*getGroupValueReadMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getGroupValueResponseMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getGroupValueWriteMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getIndividualAddrWriteMsg( eibHdl *hdl, knxMsg *, unsigned int ) ;
extern	knxMsg	*getIndividualAddrRequestMsg( eibHdl *hdl, knxMsg *, unsigned int ) ;
extern	knxMsg	*getIndividualAddrResponseMsg( eibHdl *hdl, knxMsg *, unsigned int ) ;
extern	knxMsg	*getAdcReadMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getAdcResponseMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getMemoryReadMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getMemoryResponseMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getMemoryWriteMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getUserMessageMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getMaskVersionReadMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getMaskVersionResponseMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getRestartMsg( eibHdl *hdl, knxMsg *) ;
extern	knxMsg	*getEscapeMsg( eibHdl *hdl, knxMsg *) ;
/**
 *
 */
extern	float	hfbtf( const unsigned char []) ;
extern	void	fthfb( float, unsigned char []) ;

#endif
