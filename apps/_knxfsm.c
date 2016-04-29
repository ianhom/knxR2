void	hdlSysFunc( eibHdl *, knxMsg *) ;
void	hdlAlarmFunc( eibHdl *, knxMsg *) ;
void	hdlHPFunc( eibHdl *, knxMsg *) ;
void	hdlLPFunc( eibHdl *, knxMsg *) ;
/**
 *
 */
void	hdlMsg( eibHdl *_myEIB, knxMsg *myMsg) {
	printf( "Message ....... : \n") ;
	printf( "  From ........ : %04x\n", myMsg->sndAddr) ;
	printf( "  To .......... : %04x\n", myMsg->rcvAddr) ;
	printf( "  Priority .... : %04x\n", myMsg->prio) ;
	switch ( myMsg->prio) {
	case	0x00	:		// System function
		hdlSysFunc( _myEIB, myMsg) ;
		break ;
	case	0x01	:		// System function
		hdlAlarmFunc( _myEIB, myMsg) ;
		break ;
	case	0x02	:		// System function
		hdlHPFunc( _myEIB, myMsg) ;
		break ;
	case	0x03	:		// System function
		hdlLPFunc( _myEIB, myMsg) ;
		break ;
	}
	printf( "\n") ;
}
/**
 *
 */
void	hdlSysFunc( eibHdl *_myEIB, knxMsg *myMsg) {
	knxMsg	*myReply, myReplyBuf ;
	myReply	=	&myReplyBuf ;
	printf( "  Priority....: System function\n") ;
	switch ( myMsg->tlc) {
	case	0x00	:		// UDP
	case	0x01	:		// NDP
		switch ( myMsg->tlc) {
		case	0x00	:		// UDP
			printf( "  N_PDU Type...: UDP\n") ;
			break ;
		case	0x01	:		// NDP
			printf( "  N_PDU Type...: NDP\n") ;
			printf( "  Sequence no..: %d\n", myMsg->seqNo) ;
			break ;
		}
		switch ( myMsg->apci) {
		case	0x00	:	// GroupValueRead
			printf( "GroupValueRead\n") ;
			break ;
		case	0x01	:	// GroupValueResponse
			printf( "GroupValueResponse\n") ;
			break ;
		case	0x02	:	// GroupValueWrite
			printf( "GroupValueWrite\n") ;
			if ( myCB.cbGroupValueWrite != NULL) {
				myCB.cbGroupValueWrite( _myEIB, myMsg) ;
			}
			break ;
		case	0x03	:	// IndividualAddressWrite
			printf( "IndividualAddrWrite\n") ;
			if ( myCB.cbIndividualAddrWrite != NULL) {
				myCB.cbIndividualAddrWrite( _myEIB, myMsg) ;
			}
			break ;
		case	0x04	:	// IndividualAddressRequest
			printf( "IndividualAddrRequest\n") ;
			if ( myCB.cbIndividualAddrRequest != NULL) {
				myCB.cbIndividualAddrRequest( _myEIB, myMsg) ;
			}
			break ;
		case	0x05	:	// IndividualAddressResponse
			printf( "IndividualAddrResponse\n") ;
			if ( myCB.cbIndividualAddrResponse != NULL) {
				myCB.cbIndividualAddrResponse( _myEIB, myMsg) ;
			}
			break ;
		case	0x06	:	// AdcRead
			printf( "AdcRead\n") ;
			break ;
		case	0x07	:	// AdcResponse
			printf( "AdcResponse\n") ;
			break ;
		case	0x08	:	// MemoryRead
			printf( "MemoryRead\n") ;
			break ;
		case	0x09	:	// MemoryResponse
			printf( "MemoryResponse\n") ;
			break ;
		case	0x0a	:	// MemoryWrite
			printf( "MemoryWrite\n") ;
			break ;
		case	0x0b	:	// UserMessage
			printf( "UserMessage\n") ;
			break ;
		case	0x0c	:	// MaskVersionRead
			printf( "MaskVersionread\n") ;
			break ;
		case	0x0d	:	// MaskVersionResponse
			printf( "MaskVersionResponse\n") ;
			break ;
		case	0x0e	:	// Restart
			printf( "Restart\n") ;
			break ;
		case	0x0f	:	// Escape
			printf( "Escape\n") ;
			break ;
		}
		break ;
	case	0x02	:		// UCD
		switch ( myMsg->tlc) {
		case	0x02	:		// UCD
			printf( "  N_PDU Type...: UCD\n") ;
			break ;
		case	0x03	:		// NCD
			printf( "  N_PDU Type...: NCD\n") ;
			break ;
		}
		switch ( myMsg->ppCmd) {
		case	0x00	:
			printf( "  Command......: open P2P connection\n") ;
			if ( myCB.cbOpenP2P != NULL) {
				myCB.cbOpenP2P( _myEIB, myMsg) ;
			}
			break ;
		case	0x01	:
			printf( "  Command......: terminate P2P connection\n") ;
			if ( myCB.cbCloseP2P != NULL) {
				myCB.cbCloseP2P( _myEIB, myMsg) ;
			}
			break ;
		case	0x02	:
			printf( "  Confirm......: positiv\n") ;
			if ( myCB.cbConfP2P != NULL) {
				myCB.cbConfP2P( _myEIB, myMsg) ;
			}
			break ;
		case	0x03	:
			printf( "  Reject......: negativ\n") ;
			if ( myCB.cbRejectP2P != NULL) {
				myCB.cbRejectP2P( _myEIB, myMsg) ;
			}
			break ;
		}
		break ;
	}
}

void	hdlAlarmFunc( eibHdl *_myEIB, knxMsg *myMsg) {
	switch ( myMsg->tlc) {
	case	0x00	:		// UDP
		break ;
	case	0x01	:		// NDP
		break ;
	case	0x02	:		// UCD
		break ;
	case	0x03	:		// NCD
		break ;
	}
}

void	hdlHPFunc( eibHdl *_myEIB, knxMsg *myMsg) {
	switch ( myMsg->tlc) {
	case	0x00	:		// UDP
		break ;
	case	0x01	:		// NDP
		break ;
	case	0x02	:		// UCD
		break ;
	case	0x03	:		// NCD
		break ;
	}
}

void	hdlLPFunc( eibHdl *_myEIB, knxMsg *myMsg) {
	switch ( myMsg->tlc) {
	case	0x00	:		// UDP
		switch ( myMsg->apci) {
		case	0x02	:	// groupValueWrite
			printf( "GroupValueWrite\n") ;
			if ( myCB.cbGroupValueWrite != NULL) {
				myCB.cbGroupValueWrite( _myEIB, myMsg) ;
			}
			break ;
		}
		break ;
	case	0x01	:		// NDP
		switch ( myMsg->apci) {
		case	0x02	:	// groupValueWrite
			printf( "GroupValueWrite\n") ;
			if ( myCB.cbGroupValueWrite != NULL) {
				myCB.cbGroupValueWrite( _myEIB, myMsg) ;
			}
			break ;
		}
		break ;
	case	0x02	:		// UCD
		break ;
	case	0x03	:		// NCD
		break ;
	}
}
