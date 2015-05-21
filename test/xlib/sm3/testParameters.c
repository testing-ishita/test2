/*******************************************************************************
*
* Filename:	 testParameters.c
*
* Description:	This file contains code for retrieving the test parameters. 
*
* $Revision: 1.3 $
*
* $Date: 2015-03-12 15:24:02 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/xlib/sm3/testParameters.c,v $
*
* Copyright 2010-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#include <stdtypes.h>
#include <stdio.h>
#include <string.h>
#include <errors.h>

 
#include <bit/bit.h>
#include <bit/delay.h>

#include <private/cpu.h>
#include <private/debug.h>

#include <cute/arch.h>
#include <cute/interconnect.h>
#include <cute/cute_errors.h>
#include <cute/sth.h>
#include <cute/packet.h>
#include <cute/sm3.h>


// #define DEBUG_SOAK

#if defined(DEBUG_SOAK)
#warning "***** DEBUG ON *****"
#endif

#define OP_BUF_SIZE		130
#define LF				10
#define CR				13

static UINT16	wBufIndex;
static char*	pchParams[MAX_PARAMS];

UINT32	dParams[MAX_PARAMS];
UINT16	wNumParams=0;


/*******************************************************************************
*
* dGetParam
*
* Get the next parameter value from the given string buffer.
*
*
* RETURNS: parameter value or 0.
* 
*******************************************************************************/
static UINT32 dGetParam
(
	char **pchBuffer
)
{
	UINT32 value;
	char*	pchTmp;
	char*	pchLimit;
	char	X_char;

	//sysDebugWriteString ("dGetParam\n");

	/* skip leading white space */
	pchTmp = *pchBuffer;
	while (*pchTmp != '\0' && *pchTmp == ' ')
		pchTmp++;

	*pchBuffer = pchTmp;

	value = 0;

	/* find the end of the parameter */
	pchTmp = *pchBuffer;
	while (*pchTmp != '\0' && *pchTmp != ' ' && *pchTmp != ',')
		pchTmp++;

	pchLimit = pchTmp - 1;

	X_char = *(pchTmp - 1);

	/* skip to start of next parameter */
	while (*pchTmp != ',' && *pchTmp != '\0')
		pchTmp++;

	if (*pchTmp == ',')
		pchTmp++;

	switch (X_char)
	{
		case 'H' :
		case 'h' :

			//sysDebugWriteString ("Hex\n");

			while (*pchBuffer < pchLimit)
			{ /* HEX */
				X_char = *((*pchBuffer)++);
				if ((X_char >= 'A') && (X_char <= 'F'))
					X_char -= 'A' - 10;
				else
				if ((X_char >= 'a') && (X_char <= 'f'))
					X_char -= 'a' - 10;
				else
				if ((X_char >= '0') && (X_char <= '9'))
					X_char -= '0';
				else
				{
					*pchBuffer = pchTmp;
					return 0;
				}
				value = 16 * value + (UINT32) X_char;
			}
			break;
		case 'B' :
		case 'b' :

			//sysDebugWriteString ("Bin\n");

			while (*pchBuffer < pchLimit)
			{ /* binary */
				X_char = *((*pchBuffer)++);
				if ((X_char == '0') || (X_char == '1'))
					value = 2 * value + (UINT32) (X_char - '0');
				else
				{
					*pchBuffer = pchTmp;
					return 0;
				}
			}
			break;
		case 'D' : /* discard the 'D'/'d' suffix and follow default */
		case 'd' : /* discard the 'D'/'d' suffix and follow default */

			//sysDebugWriteString ("Dec\n");

			pchLimit--;
		default :

			//sysDebugWriteString ("Default\n");

			pchLimit++;
			while (*pchBuffer < pchLimit && (*(*pchBuffer) != NULL ) )
			{
				X_char = *((*pchBuffer)++);
				if ((X_char >= '0') && (X_char <= '9'))
					value = 10 * value + (UINT32) (X_char - '0');
				else
				{
					if(*pchTmp != NULL)
						*( pchTmp - 1) = '\0';

					value = (UINT32)(*pchBuffer - 1);
					*pchBuffer = pchTmp;
					return value;
				}
			}
			break;
	}
	*pchBuffer = pchTmp;
	return value;
}


/*******************************************************************************
*
* vParseCommandTail
*
* Break down the command tail into separate parameters.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void vParseCommandTail (UINT8* pchTail)
{
	char*	pChar;
	UINT16	i;

#ifdef DEBUG_SOAK	
	char achDbgTxt[128];
#endif

	wNumParams = 0;
	memset(dParams, 0x00, (sizeof(UINT32) * MAX_PARAMS) );

	wBufIndex = 0;							/* reset output buffer */
	pChar = (char *)pchTail;

#ifdef DEBUG_SOAK
	sprintf (achDbgTxt,"Cmd Tail:%s\n", pchTail);
	sysDebugWriteString (achDbgTxt);
#endif

	/* skip any leading spaces */
	while (*pChar == ' ') pChar++;

	/* get each of the parameters */
	i = 0;

	while (*pChar != '\0')
	{
		pchParams[i+1] = pChar;
		dParams[i+1] = dGetParam(&pChar);
		i++;
		
#ifdef DEBUG_SOAK
		sprintf (achDbgTxt,"Param[%d] = %d,\n", i-1, dParams[i]);
		sysDebugWriteString (achDbgTxt);
#endif
	}

	wNumParams = i;
	dParams[0] = wNumParams;

#ifdef DEBUG_SOAK
	sprintf (achDbgTxt, "wNumParams = %d\n", (int)wNumParams);
	sysDebugWriteString (achDbgTxt);
#endif

}

