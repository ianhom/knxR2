void	disasSysFunc( FILE *_oFile, eibHdl *, knxMsg *) ;
void	disasAlarmFunc( FILE *_oFile, eibHdl *, knxMsg *) ;
void	disasHPFunc( FILE *_oFile, eibHdl *, knxMsg *) ;
void	disasLPFunc( FILE *_oFile, eibHdl *, knxMsg *) ;
/**
 *
 */
void	disasMsg( FILE *_oFile, eibHdl *_myEIB, knxMsg *myMsg) {
	
	fprintf( _oFile, "Message........: \n") ;
	fprintf( _oFile, "  From.........: %04x\n", myMsg->sndAddr) ;
	fprintf( _oFile, "  To...........: %04x\n", myMsg->rcvAddr) ;
	switch ( myMsg->prio) {
	case	0x00	:		// System function
		disasSysFunc( _oFile, _myEIB, myMsg) ;
		break ;
	case	0x01	:		// System function
		disasAlarmFunc( _oFile, _myEIB, myMsg) ;
		break ;
	case	0x02	:		// System function
		disasHPFunc( _oFile, _myEIB, myMsg) ;
		break ;
	case	0x03	:		// System function
		disasLPFunc( _oFile, _myEIB, myMsg) ;
		break ;
	}
	fprintf( _oFile, "\n") ;
}
/**
 *
 */
void	disasSysFunc( FILE *_oFile, eibHdl *_myEIB, knxMsg *myMsg) {
	knxMsg	*myReply, myReplyBuf ;
	int	i ;
	int	byteCount ;
	int	address ;
	myReply	=	&myReplyBuf ;
	fprintf( _oFile, "  Priority....: System function\n") ;
	switch ( myMsg->tlc) {
	case	0x00	:		// UDP
	case	0x01	:		// NDP
		fprintf( _oFile, "    tlc.......: %02x\n", myMsg->tlc) ;
		switch ( myMsg->tlc) {
		case	0x00	:		// UDP
			fprintf( _oFile, "    N_PDU Type...: UDP\n") ;
			break ;
		case	0x01	:		// NDP
			fprintf( _oFile, "    N_PDU Type...: NDP\n") ;
			fprintf( _oFile, "    Sequence no..: %d\n", myMsg->seqNo) ;
			break ;
		}
		fprintf( _oFile, "      apci......: %02x\n", myMsg->apci) ;
		switch ( myMsg->apci) {
		case	0x00	:	// GroupValueRead
			fprintf( _oFile, "      GroupValueRead\n") ;
			break ;
		case	0x01	:	// GroupValueResponse
			fprintf( _oFile, "      GroupValueResponse\n") ;
			break ;
		case	0x02	:	// GroupValueWrite
			fprintf( _oFile, "      GroupValueWrite\n") ;
			break ;
		case	0x03	:	// IndividualAddressWrite
			fprintf( _oFile, "      IndividualAddrWrite\n") ;
			if ( myCB.cbIndividualAddrWrite != NULL) {
				myCB.cbIndividualAddrWrite( _myEIB, myMsg) ;
			}
			break ;
		case	0x04	:	// IndividualAddressRequest
			fprintf( _oFile, "      IndividualAddrRequest\n") ;
			if ( myCB.cbIndividualAddrRequest != NULL) {
				myCB.cbIndividualAddrRequest( _myEIB, myMsg) ;
			}
			break ;
		case	0x05	:	// IndividualAddressResponse
			fprintf( _oFile, "      IndividualAddrResponse\n") ;
			if ( myCB.cbIndividualAddrResponse != NULL) {
				myCB.cbIndividualAddrResponse( _myEIB, myMsg) ;
			}
			break ;
		case	0x06	:	// AdcRead
			fprintf( _oFile, "      AdcRead\n") ;
			break ;
		case	0x07	:	// AdcResponse
			fprintf( _oFile, "      AdcResponse\n") ;
			break ;
		case	0x08	:	// MemoryRead
			byteCount	=	myMsg->mtext[1] & 0x0f ;
			address		=	(myMsg->mtext[2] << 8) | myMsg->mtext[3] ;
			fprintf( _oFile, "      MemoryRead\n") ;
			fprintf( _oFile, "        Length.....:  %d\n", byteCount) ;
			fprintf( _oFile, "        Start......:  %04x\n", address) ;
			break ;
		case	0x09	:	// MemoryResponse
			byteCount	=	myMsg->mtext[1] & 0x0f ;
			address	=	(myMsg->mtext[2] << 8) | myMsg->mtext[3] ;
			fprintf( _oFile, "      MemoryResponse\n") ;
			fprintf( _oFile, "        Length.....:  %d\n", byteCount) ;
			fprintf( _oFile, "        Start......:  %04x\n", address) ;
			fprintf( _oFile, "        Data.......:  ") ;
			for ( i=0 ; i < byteCount ; i++) {
				fprintf( _oFile, "%02x ", myMsg->mtext[4+i]) ;
			}
			fprintf( _oFile, "\n") ;
			break ;
		case	0x0a	:	// MemoryWrite
			byteCount	=	myMsg->mtext[1] & 0x0f ;
			address	=	(myMsg->mtext[2] << 8) | myMsg->mtext[3] ;
			fprintf( _oFile, "      MemoryWrite\n") ;
			fprintf( _oFile, "        Length.....:  %d\n", byteCount) ;
			fprintf( _oFile, "        Start......:  %04x\n", address) ;
			fprintf( _oFile, "        Data.......:  ") ;
			for ( i=0 ; i < byteCount ; i++) {
				fprintf( _oFile, "%02x ", myMsg->mtext[4+i]) ;
			}
			fprintf( _oFile, "\n") ;
			break ;
		case	0x0b	:	// UserMessage
			fprintf( _oFile, "      UserMessage\n") ;
			break ;
		case	0x0c	:	// MaskVersionRead
			fprintf( _oFile, "      MaskVersionRead\n") ;
			break ;
		case	0x0d	:	// MaskVersionResponse
			fprintf( _oFile, "      MaskVersionResponse\n") ;
			fprintf( _oFile, "        Medium profile.....:  %d\n", ((myMsg->mtext[3] & 0xf0) >> 4)) ;
			fprintf( _oFile, "        Main profile.......:  %d\n", myMsg->mtext[3] & 0x0f) ;
			fprintf( _oFile, "        Mask version.......:  %d.%d\n"
							,	((myMsg->mtext[4] & 0xf0) >> 4)
							,	myMsg->mtext[4] & 0x0f) ;
			break ;
		case	0x0e	:	// Restart
			fprintf( _oFile, "      Restart\n") ;
			break ;
		case	0x0f	:	// Escape
			fprintf( _oFile, "      Escape\n") ;
			switch ( myMsg->mtext[1] & 0x003f) {
			case	0x0010	:
				fprintf( _oFile, "        M_BitWrite\n") ;
				break ;
			case	0x0011	:
				fprintf( _oFile, "        M_BitAuthorizeRequest\n") ;
				break ;
			case	0x0012	:
				fprintf( _oFile, "        M_BitAuthorizeResponse\n") ;
				break ;
			case	0x0013	:
				fprintf( _oFile, "        M_SetkeyRequest\n") ;
				break ;
			case	0x0014	:
				fprintf( _oFile, "        M_SetKeyResponse\n") ;
				break ;
			default	:
				fprintf( _oFile, "        %02x\n", myMsg->mtext[1] & 0x003f) ;
				break ;
			}
			break ;
		}
		break ;
	case	0x02	:		// UCD
	case	0x03	:		// UCD
		fprintf( _oFile, "      tlc.......: %02x\n", myMsg->tlc) ;
		switch ( myMsg->tlc) {
		case	0x02	:		// UCD
			fprintf( _oFile, "    N_PDU Type...: UCD\n") ;
			break ;
		case	0x03	:		// NCD
			fprintf( _oFile, "    N_PDU Type...: NCD\n") ;
			break ;
		}
		fprintf( _oFile, "      ppCmd.....: %02x\n", myMsg->ppCmd) ;
		switch ( myMsg->ppCmd) {
		case	0x00	:
			fprintf( _oFile, "      open P2P connection\n") ;
			if ( myCB.cbOpenP2P != NULL) {
				myCB.cbOpenP2P( _myEIB, myMsg) ;
			}
			break ;
		case	0x01	:
			fprintf( _oFile, "      terminate P2P connection\n") ;
			if ( myCB.cbCloseP2P != NULL) {
				myCB.cbCloseP2P( _myEIB, myMsg) ;
			}
			break ;
		case	0x02	:
			fprintf( _oFile, "      Accept P2P\n") ;
			if ( myCB.cbConfP2P != NULL) {
				myCB.cbConfP2P( _myEIB, myMsg) ;
			}
			break ;
		case	0x03	:
			fprintf( _oFile, "      Reject P2P\n") ;
			if ( myCB.cbRejectP2P != NULL) {
				myCB.cbRejectP2P( _myEIB, myMsg) ;
			}
			break ;
		}
		break ;
	}
}

void	disasAlarmFunc( FILE *_oFile, eibHdl *_myEIB, knxMsg *myMsg) {
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

void	disasHPFunc( FILE *_oFile, eibHdl *_myEIB, knxMsg *myMsg) {
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

void	disasLPFunc( FILE *_oFile, eibHdl *_myEIB, knxMsg *myMsg) {
	fprintf( _oFile, "  Priority....: Low Priority\n") ;
	switch ( myMsg->tlc) {
	case	0x00	:		// UDP
		switch ( myMsg->apci) {
		case	0x02	:	// groupValueWrite
			fprintf( _oFile, "GroupValueWrite\n") ;
			break ;
		}
		break ;
	case	0x01	:		// NDP
		switch ( myMsg->apci) {
		case	0x02	:	// groupValueWrite
			fprintf( _oFile, "GroupValueWrite\n") ;
			break ;
		}
		break ;
	case	0x02	:		// UCD
		break ;
	case	0x03	:		// NCD
		break ;
	}
}
