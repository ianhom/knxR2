/**
 *
 */

node	_data[]	=	{
				{	7, "Date"		,dtDate		,0	,   1	,0	,0	,srcOTHER, 0}
			,	{	7, "Time"		,dtTime		,0	,   2	,0	,0	,srcOTHER, 0}
			,	{	7, "DateTime"		,dtDateTime	,0	,   3	,0	,0	,srcOTHER, 0}
			,	{	7, "SwitchDN"		,dtBit		,0	,  10	,0	,0	,srcOTHER, 0}
			,	{	7, "SwitchCO"		,dtBit		,0	,  11	,0	,0	,srcOTHER, 0}
			,	{	7, "SwitchNI"		,dtBit		,0	,  12	,0	,0	,srcOTHER, 0}
			,	{	5, "TempCol1"		,dtFloat2	,0	,6144	,0	,0x01	,srcOTHER, 0}
			,	{	5, "TempCol2"		,dtFloat2	,0	,6145	,0	,0x01	,srcOTHER, 0}
			,	{	5, "TempCol3"		,dtFloat2	,0	,6146	,0	,0x01	,srcOTHER, 0}
			,	{	5, "TempCol4"		,dtFloat2	,0	,6147	,0	,0x01	,srcOTHER, 0}
			,	{	5, "TempCol5"		,dtFloat2	,0	,6148	,0	,0x01	,srcOTHER, 0}
			,	{	6, "TempWWu"		,dtFloat2	,0	,6164	,0	,0x07	,srcOTHER, 0}
			,	{	6, "TempWWm"		,dtFloat2	,0	,6165	,0	,0x07	,srcOTHER, 0}
			,	{	6, "TempWWo"		,dtFloat2	,0	,6166	,0	,0x07	,srcOTHER, 0}
			,	{	7, "TempPSu"		,dtFloat2	,0	,6184	,0	,0x07	,srcOTHER, 0}
			,	{	7, "TempPSm"		,dtFloat2	,0	,6185	,0	,0x07	,srcOTHER, 0}
			,	{	7, "TempPSo"		,dtFloat2	,0	,6186	,0	,0x07	,srcOTHER, 0}
			,	{	1, "ValveSolar"		,dtBit		,0	,6400	,0	,0x07	,srcOTHER, 1}
			,	{	1, "ValvePelletStove"	,dtBit		,0	,6410	,0	,0x07	,srcOTHER, 0}
			,	{	1, "ValveFloorHeating"	,dtBit		,0	,6420	,0	,0x07	,srcOTHER, 0}
			,	{	2, "ValveRadiator"	,dtBit		,0	,6430	,0	,0x07	,srcOTHER, 0}
			,	{	0, "PumpFloorHeating"	,dtBit		,0	,6510	,0	,0x01	,srcOTHER, 0}	// off/on
			,	{	0, "PumpRadiator"	,dtBit		,0	,6515	,0	,0x01	,srcOTHER, 0}	// off/on
			,	{	0, "PumpSolar"		,dtBit		,0	,6520	,0	,0x07	,srcOTHER, 0}	// off/on
			,	{	0, "PumpCirculation"	,dtBit		,0	,6525	,0	,0x01	,srcOTHER, 0}	// off/on
			,	{	7, "TempUG01"		,dtFloat2	,0	,6657	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG02"		,dtFloat2	,0	,6667	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG03"		,dtFloat2	,0	,6677	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG04"		,dtFloat2	,0	,6687	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG05"		,dtFloat2	,0	,6697	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG06"		,dtFloat2	,0	,6707	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG01"		,dtFloat2	,0	,6913	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG02"		,dtFloat2	,0	,6923	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG03"		,dtFloat2	,0	,6933	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG04"		,dtFloat2	,0	,6943	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG05"		,dtFloat2	,0	,6953	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG06"		,dtFloat2	,0	,6963	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG07"		,dtFloat2	,0	,6973	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG01"		,dtFloat2	,0	,7169	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG02"		,dtFloat2	,0	,7179	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG03"		,dtFloat2	,0	,7189	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG04"		,dtFloat2	,0	,7199	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG05"		,dtFloat2	,0	,7209	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG06"		,dtFloat2	,0	,7219	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG01Target"	,dtFloat2	,0	,6663	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG02Target"	,dtFloat2	,0	,6673	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG03Target"	,dtFloat2	,0	,6683	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG04Target"	,dtFloat2	,0	,6693	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG05Target"	,dtFloat2	,0	,6703	,0	,0	,srcOTHER, 0}
			,	{	7, "TempUG06Target"	,dtFloat2	,0	,6713	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG01Target"	,dtFloat2	,0	,6919	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG02Target"	,dtFloat2	,0	,6929	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG03Target"	,dtFloat2	,0	,6939	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG04Target"	,dtFloat2	,0	,6949	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG05Target"	,dtFloat2	,0	,6959	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG06Target"	,dtFloat2	,0	,6969	,0	,0	,srcOTHER, 0}
			,	{	7, "TempEG07Target"	,dtFloat2	,0	,6979	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG01Target"	,dtFloat2	,0	,7175	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG02Target"	,dtFloat2	,0	,7185	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG03Target"	,dtFloat2	,0	,7195	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG04Target"	,dtFloat2	,0	,7205	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG05Target"	,dtFloat2	,0	,7215	,0	,0	,srcOTHER, 0}
			,	{	7, "TempOG06Target"	,dtFloat2	,0	,7225	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeUG01"		,dtBit		,0	,8449	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeUG02"		,dtBit		,0	,8450	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeUG03"		,dtBit		,0	,8451	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeUG04"		,dtBit		,0	,8452	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeUG05"		,dtBit		,0	,8453	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeUG06"		,dtBit		,0	,8454	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeEG01"		,dtBit		,0	,8961	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeEG02"		,dtBit		,0	,8962	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeEG03"		,dtBit		,0	,8963	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeEG04"		,dtBit		,0	,8964	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeEG05"		,dtBit		,0	,8965	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeEG06"		,dtBit		,0	,8966	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeEG07"		,dtBit		,0	,8967	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeOG01"		,dtBit		,0	,9217	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeOG02"		,dtBit		,0	,9218	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeOG03"		,dtBit		,0	,9219	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeOG04"		,dtBit		,0	,9220	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeOG05"		,dtBit		,0	,9221	,0	,0	,srcOTHER, 0}
			,	{	7, "ValeOG06"		,dtBit		,0	,9222	,0	,0	,srcOTHER, 0}
			,	{	3, "PelletStove"	,dtBit		,0	,9473	,0	,0x02	,srcOTHER, 0}	// off/on
			,	{	4, "HeatPump"		,dtBit		,0	,9482	,0	,0x04	,srcOTHER, 0}	// off/on
			} ;

#define	HEAT_PUMP	64

#define	VALVE_SOLAR_WW	0
#define	VALVE_SOLAR_HB	1

#define	VALVE_HP	21
#define	VALVE_HP_WW	1
#define	VALVE_HP_HB	0


/**
 * define macro to determine the highest index of the data table
 */

#define	lastDataIndex	( sizeof( _data) / sizeof( node))
