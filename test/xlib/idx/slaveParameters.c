/************************************************************************
 *                                                                      *
 *      Copyright 2010 Concurrent Technologies, all rights reserved.    *
 *                                                                      *
 *      The program below is supplied by Concurrent Technologies        *
 *      on the understanding that no responsibility is assumed by       *
 *      Concurrent Technologies for any errors contained therein.       *
 *      Furthermore, Concurrent Technologies makes no commitment to     *
 *      update or keep current the program code, and reserves the       *
 *      right to change its specifications at any time.                 *
 *                                                                      *
 *      Concurrent Technologies assumes no responsibility either for    *
 *      the use of this code or for any infringements of the patent     *
 *      or other rights of third parties which may result from its use  *
 *                                                                      *
 ************************************************************************/

/*
 * slaveParameters.c
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/xlib/idx/slaveParameters.c,v 1.2 2013-11-25 13:18:05 mgostling Exp $
 * $Log: slaveParameters.c,v $
 * Revision 1.2  2013-11-25 13:18:05  mgostling
 * Added missing CVS headers.
 * Added support for TRB1x.
 *
 *
 */

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
#include <cute/idx.h>


//#define DEBUG_SOAK

#define LF				10
#define CR				13
#define MAX_PARAM_BUFF  128

static UINT16 wBufIndex;
static char*  pchParams[MAX_PARAMS];

extern sm3vmeSlave asSlots[MAX_SLAVES];
extern sm3vmesyscon syscon;

UINT32 dParams[MAX_PARAMS];
UINT16 wNumParams;


static void vParseSavedCommandTail( UINT8 bSlot);
static UINT32 dGetParam( char **pchBuffer );

static UINT32 dGetParam( char **pchBuffer )
{
	UINT32 value;
	char*  pchTmp;
	char*  pchLimit;
	char   X_char;

	/* skip leading white space */
	pchTmp = *pchBuffer;
	while( (*pchTmp != '\0') && (*pchTmp == ' ' ) )
		pchTmp++;

	*pchBuffer = pchTmp;
	value = 0;

	/* find the end of the parameter */
	pchTmp = *pchBuffer;
	while( (*pchTmp != '\0') && (*pchTmp != ' ') && (*pchTmp != ',' ) )
		pchTmp++;

	pchLimit = pchTmp - 1;
	X_char   = *(pchTmp - 1);

	/* skip to start of next parameter */
	while( (*pchTmp != ',') && (*pchTmp != '\0') )
		pchTmp++;

	if( *pchTmp == ',' )
		 pchTmp++;

	switch( X_char )
	{
		case 'H' :
		case 'h' :
			while( *pchBuffer < pchLimit )
			{ 	/* HEX */
				X_char = *((*pchBuffer)++);
				if( (X_char >= 'A') && (X_char <= 'F') )
					X_char -= 'A' - 10;

				else if( (X_char >= 'a') && (X_char <= 'f') )
					 X_char -= 'a' - 10;

				else if( (X_char >= '0') && (X_char <= '9') )
					 X_char -= '0';
				else
				{
					*pchBuffer = pchTmp;
					return 0;
				}
				value = 16 * value + (UINT32)X_char;
			}
			break;

		case 'B' :
		case 'b' :
			while( *pchBuffer < pchLimit )
			{ /* binary */
				X_char = *((*pchBuffer)++);
				if( (X_char == '0') || (X_char == '1') )
					 value = 2 * value + (UINT32)(X_char - '0');
				else
				{
					*pchBuffer = pchTmp;
					return 0;
				}
			}
			break;

		case 'D': /* discard the 'D'/'d' suffix and follow default */
		case 'd': /* discard the 'D'/'d' suffix and follow default */
			// sysDebugWriteString( "Dec\n" );
			pchLimit--;

		default :
			pchLimit++;
			while( (*pchBuffer < pchLimit) && (*(*pchBuffer) != NULL ) )
			{
				X_char = *((*pchBuffer)++);
				if ((X_char >= '0') && (X_char <= '9'))
					value = 10 * value + (UINT32)(X_char - '0');
				else
				{
					if(*pchTmp != NULL)
					   *(pchTmp - 1) = '\0';

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



/**********************************************************************
 * vParseCommandTail												  *
 * Break down the command tail into separate parameters               *
 **********************************************************************/
static void vParseSavedCommandTail( UINT8 bSlot)
{
	char*	pChar;
	UINT16	i;
	
	#ifdef DEBUG_SOAK
	UINT8  *flag, *ptr;

	if(bSlot == SysCon_ID)
	{
		ptr  = syscon.params;
		flag = &syscon.pramFlag;
	}
	else
	{
		ptr  = asSlots[bSlot].params;
		flag = &asSlots[bSlot].pramFlag;
	}
	#endif

	wNumParams = 0;
	memset( dParams, 0x00, ( sizeof(UINT32) * MAX_PARAMS ) );

	wBufIndex = 0;							/* reset output buffer */


	#ifdef DEBUG_SOAK
	pChar     = (char*)ptr;
	{
		char achDbgTxt[ 160 ];
		sprintf( achDbgTxt, "%d, flag %d, Cmd Tail : %s\n", bSlot, *flag, ptr );
		sysDebugWriteString( achDbgTxt );
	}
	#endif

	/* skip any leading spaces */
	while(*pChar == ' ')
		   pChar++;

	/* get each of the parameters */
	i = 0;

	while (*pChar != '\0')
	{
		pchParams[ i+1 ] = pChar;
		dParams[ i+1 ]   = dGetParam( &pChar );
		i++;
		#ifdef DEBUG_SOAK
		{
			char achDbgTxt[ 128 ];
			sprintf( achDbgTxt, "Param[ %d ] = %x,\n", (i-1), dParams[ i ] );
			sysDebugWriteString( achDbgTxt );
		}
		#endif
	}

	wNumParams   = i;
	dParams[ 0 ] = wNumParams;

	#ifdef DEBUG_SOAK
	{
		char achDbgTxt[ 80 ];
		sprintf( achDbgTxt, "wNumParams = %d\n", (int)wNumParams );
		sysDebugWriteString( achDbgTxt );
	}
	#endif
}


UINT32 vProcessSavedParams( UINT8 bSlot, saved_params *params )
{
	UINT32 rt, i;

	vParseSavedCommandTail( bSlot);

	if(dParams[0] == params->count )
	{
		for(i = 0; i<params->count; i++)
			params->data[i] = dParams[ i+1 ];

		rt = 1;
	}
	else
		rt = 0;

	return rt;
}


UINT32 vSaveCommandTail( UINT8 bSlot, UINT8* pchTail )
{
	int    i;
	UINT32 rt = E__OK;
	UINT8  *flag, *ptr;
#ifdef DEBUG_SOAK
	char   buff[160];
#endif

	if(bSlot == SysCon_ID)
	{
		ptr  = syscon.params;
		flag = &syscon.pramFlag;
	}
	else
	{
		ptr  =  asSlots[bSlot].params;
		flag = &asSlots[bSlot].pramFlag;
	}

	memset( ptr, 0x00 , MAX_PARAM_BUFF );

	i = 0;
	while( (pchTail[i] != '\0') && (i < (MAX_PARAM_BUFF-1)) )
	{
		ptr[i] = pchTail[i];
		i++;
	}

	if(i != 0)
	{
		*flag  = 1;
		ptr[i] = '\0';

		#ifdef DEBUG_SOAK
			sprintf( buff, "vSaveCommandTail() bSlot %d, %d bytes copies\n",
					 bSlot, i );
			sysDebugWriteString( buff );
			sprintf( buff, "Tail %s\n", ptr );
			sysDebugWriteString( buff );
		#endif
	}
	else
	{
		rt = E__FAIL;
		*flag = 0;
		#ifdef DEBUG_SOAK
			sysDebugWriteString( "vSaveCommandTail() NO Params!\n" );
		#endif
	}

	return rt;
}
