#ifndef	knxProt_INCLUDED
#define	knxProt_INCLUDED

typedef	enum	{
		AT_INDIV	=	0x00
	,	AT_GROUP	=	0x01
	}	addressingType ;

typedef	enum	{
		CT_UDP		=	0x00
	,	CT_NDP		=	0x01
	,	CT_UCD		=	0x02
	,	CT_NCD		=	0x03
	}	communicationType ;

typedef	enum	{
		GroupValueRead		=	0x00
	,	GroupValueResponse	=	0x01
	,	GroupValueWrite		=	0x02
	,	IndividualAddrWrite	=	0x03
	,	IndividualAddrRequest	=	0x04
	,	IndividualAddrResponse	=	0x05
	,	AdcRead			=	0x06
	,	AdcResponse		=	0x07
	,	MemoryRead		=	0x08
	,	MemoryResponse		=	0x09
	,	MemoryWrite		=	0x0a
	,	UserMessage		=	0x0b
	,	MaskVersionRead		=	0x0c
	,	MaskVersionResponse	=	0x0d
	,	Restart			=	0x0e
	,	Escape			=	0x0f
	}	APCI ;

typedef	struct	{
		unsigned	char	cmd ;
	}	GROUP_VALUE_READ ;

typedef	struct	{
		unsigned	char	cmd ;
		unsigned	char	data[16] ;
	}	GROUP_VALUE_RESPONSE ;

typedef	struct	{
		unsigned	char	cmd ;
		unsigned	char	data[16] ;
	}	GROUP_VALUE_WRITE ;

typedef	struct	{
		unsigned	char	data[2] ;
	}	individualAddressWrite ;

typedef	struct	{
		unsigned	char	data ;
	}	individualAddressRequest ;

typedef	struct	{
		unsigned	char	data ;
	}	individualAddressResponse ;

typedef	struct	{
		unsigned	char	data[2] ;
	}	adcRead ;

typedef	struct	{
		unsigned	char	data[4] ;
	}	adcResponse ;

typedef	struct	{
		unsigned	char	length ;
		unsigned	char	adrHigh ;
		unsigned	char	adrLow ;
	}	memoryRead ;

typedef	struct	{
		unsigned	char	length ;
		unsigned	char	adrHigh ;
		unsigned	char	adrLow ;
		unsigned	char	data[14] ;
	}	memoryResponse ;

typedef	struct	{
		unsigned	char	length ;
		unsigned	char	adrHigh ;
		unsigned	char	adrLow ;
		unsigned	char	data[14] ;
	}	memoryWrite ;

typedef	struct	{
		unsigned	char	data[16] ;
	}	userMessage ;

typedef	struct	{
		unsigned	char	data ;
	}	maskVersionRead ;

typedef	struct	{
		unsigned	char	maskType ;
		unsigned	char	mainVersion ;
		unsigned	char	subVersion ;
	}	maskVersionResponse ;

typedef	struct	{
		unsigned	char	data ;
	}	restart ;

typedef	struct	{
	const	unsigned	char	type ;
		unsigned	char	bits[6] ;
	}	M_BitWrite ;

typedef	struct	{
	const	unsigned	char	type ;
		unsigned	char	number[4] ;
	}	M_AuthorizeRequest ;

typedef	struct	{
	const	unsigned	char	type ;
		unsigned	char	accessLevel ;
	}	M_AuthorizeResponse ;

typedef	struct	{
	const	unsigned	char	type ;
		unsigned	char	number[4] ;
	}	M_SetKeyRequest ;

typedef	struct	{
	const	unsigned	char	type ;
		unsigned	char	accessLevel ;
	}	M_SetKeyResponse ;

typedef	union	{
		M_BitWrite	bitWrite ;
		M_AuthorizeRequest	authRequest ;
		M_AuthorizeResponse	authResponse ;
		M_SetKeyRequest		setKeyRequest ;
		M_SetKeyResponse	setKeyResponse ;
	}	escapeData ;
		
typedef	struct	{
		unsigned	char	data ;
		escapeData	escData ;
	}	escape ;

typedef	struct	{		// un-numbered data packet
		unsigned	char	APCI ;
		union	{
			GROUP_VALUE_READ	groupValueREad ;
			GROUP_VALUE_RESPONSE	groupValueResponse ;
			GROUP_VALUE_WRITE	groupValueWrite ;
		}			data ;
	}	UDP ;
typedef	struct	{		// numbered data packet
		unsigned	char	APCI ;
		unsigned	char	data[16] ;
	}	NDP ;
typedef	struct	{		// un-numbered contraol data
		
	}	UCD ;
typedef	struct	{		// numbered contraol data
		
	}	NCD ;

typedef	struct	{
		communicationType	commType ;
		union	{
			UDP	udp ;
			NDP	ndp ;
			UCD	ucd ;
			NCD	ncd ;
		}			data ;
	}	T_PDU ;

typedef	struct	{
		addressingType		adrType ;
		unsigned	char	routingCounter ;
		unsigned	char	length ;
		T_PDU			tpdu ;
	}	N_PDU ;

typedef	struct	{
		unsigned	char	ctrlField ;
		unsigned	short	srcAddr ;
		unsigned	short	destAddr ;
				N_PDU	npdu ;
		unsigned	char	checkField ;
	}	knxTelegram ;
#endif
