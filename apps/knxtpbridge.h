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
 * knxbridge.h
 *
 * some high level functional description
 *
 * Revision history
 *
 * date		rev.	who	what
 * ----------------------------------------------------------------------------
 * 2015-12-07	PA1	khw	inception;
 *
 */

#ifndef	knxtpbridge_INCLUDED
#define	knxtpbridge_INCLUDED

/**
 * define the various states for the serial receiver
 */
typedef	enum	{
		opModeMaster
	,	opModeSlave
	}	knxOpMode ;
/**
 * define the various states for the serial receiver
 */
typedef	enum	{
	waiting_for_control		,
		waiting_for_hw_adr	,
		waiting_for_group_adr	,
		waiting_for_info	,
		waiting_for_data	,
		waiting_for_checksum
}	bridgeModeRcv ;

/**
 * define the various states for the serial sender
 */
typedef	enum	{
	sending_idle		,
	sending_control		,
		sending_hw_adr	,
		sending_group_adr	,
		sending_info	,
		sending_data	,
		sending_checksum
}	bridgeModeSnd ;

#endif

