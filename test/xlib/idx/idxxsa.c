/************************************************************************
 *                                                                      *
 *      Copyright 2008 Concurrent Technologies, all rights reserved.    *
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

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/xlib/idx/idxxsa.c,v 1.1 2013-09-04 08:03:03 chippisley Exp $
 * $Log: idxxsa.c,v $
 * Revision 1.1  2013-09-04 08:03:03  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2012/03/27 08:01:33  cdobson
 * Reduce the number of compiler warnings.
 *
 * Revision 1.7  2011/11/21 11:20:26  hmuneer
 * no message
 *
 * Revision 1.6  2011/10/27 15:51:20  hmuneer
 * no message
 *
 * Revision 1.5  2011/03/22 13:49:35  hchalla
 * Clean up for spaces and tabs.
 *
 * Revision 1.4  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.3  2010/11/04 17:47:51  hchalla
 * Added debug info.
 *
 * Revision 1.2  2010/06/28 10:05:50  hchalla
 * Code Cleanup, Added headers and comments.
 *
 */


#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>

#include <bit/bit.h>
#include <bit/delay.h>
#include <bit/board_service.h>
#include <bit/console.h>
#include <private/debug.h>

#include <cute/arch.h>
#include <cute/idx.h>
#include <cute/interconnect.h>
#include <cute/sth.h>
#include <cute/cute_errors.h>
#include <cute/packet.h>
#include <cute/sm3.h>

//#define DEBUG_SOAK

#define NULL_CHAR			((char)0x00)

static void		vSendHeader (UINT8 bParcelType, UINT16 wParcelLength);
static void		vReceiveHeader (UINT8* pbParcelType, UINT16* pwResponseLength,
							UINT16* pwResponseStatus);
static UINT16	wLengthParser (UINT16* pwParamIndex, UINT8* pbParams, UINT8* abString,
							UINT16*	pwStatus);


/*****************************************************************************\
 *
 *  TITLE:  vSendHeader ()
 *
 *  ABSTRACT: send the parcel header and length fields.
 *
 * 	RETURNS:  NONE
 *
\*****************************************************************************/

static void vSendHeader( UINT8 bParcelType,
	UINT16	wParcelLength
)
{
	/* Send parcel header and length fields */

	interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, bParcelType, FALSE);
	interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, (UINT8)(wParcelLength),	  FALSE);
	interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, (UINT8)(wParcelLength >> 8), FALSE);

} /* vSendHeader () */


/*****************************************************************************
 *  TITLE:  vReceiveHeader ()
 *  ABSTRACT: Receive the parcel header and length fields.
 * 	RETURNS:  NONE
 *****************************************************************************/
static void vReceiveHeader(	UINT8*	pbParcelType,
	UINT16*	pwResponseLength,
	UINT16*	pwResponseStatus
)
{
	UINT8	bLoByte;
	UINT8	bHiByte;

	interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, pbParcelType);
	interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, &bLoByte);
	interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, &bHiByte);
	*pwResponseLength = (UINT16)(bHiByte << 8) | bLoByte;

	interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, &bLoByte);
	interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, &bHiByte);
	*pwResponseStatus = (UINT16)(bHiByte << 8) | bLoByte;

} /* vReceiveHeader () */



/*****************************************************************************
 *  TITLE:  wLengthParser ()
 *  ABSTRACT: Get string length accounting for <B>, <W> type indicators
 * 	RETURNS:  string length in bytes
 *****************************************************************************/
static UINT16	wLengthParser( UINT16*	pwParamIndex,
	UINT8*	pbParams,
	UINT8*	abString,
	UINT16*	pwStatus
)
{
	UINT16	wStringIndex;
	UINT16	wParamIndex;
	UINT8	bCountEnable;
	char	ch;


	wParamIndex	 = 0;
	wStringIndex = 0;
	bCountEnable = FALSE;

	/* Parse string until a NULL terminator is encountered */

	ch = abString[wStringIndex];	/* read first charachter, it may be NULL */

	while (ch != NULL_CHAR)
	{
		if (ch == '<')					/* "<" = start counting parameters */
			bCountEnable = TRUE;

		else if (ch == '>')				/* ">" = stop counting parameters */
			bCountEnable = FALSE;

		else if ((ch > 0x40) && (ch < 0x7B))	/* convert to uppercase */
			ch = ch &  0xDF;

		/* If enabled, count the parameters associated with the string */

		if (bCountEnable == TRUE)
		{
			/* If parameter pointer is NULL, flag error, but continue counting */

			if ((pbParams == NULL) && (pwStatus != NULL))
				*pwStatus = (UINT16)E__CUTE;

			switch (ch)		/* process */
			{
				case 'B':	/* UINT8 */
				case 'C':
				case '#':
					wParamIndex += 1;
					break;

				case 'W':	/* UINT16 */
					wParamIndex += 2;
					break;

				case 'D':	/* UINT32 */
					wParamIndex += 4;
					break;

				case 'S':	/* don't support string - code seems buggy */
					break;

				default:
					break;

			}; /* switch (ch) */

		} /* if (count enabled) */

		/* Read the next chararcter */

		wStringIndex++;
		ch = abString[wStringIndex];
	}

	*pwParamIndex = wParamIndex;

	return (wStringIndex);

} /* wLengthParser () */



/*****************************************************************************
 *  TITLE:  wIDX_GetParams ()
 *  ABSTRACT: Get the hcon parameters
 * 	RETURNS:
 *****************************************************************************/
UINT8 paramBuff[1024];

UINT32 wIDX_GetParams( EXPECTED_TYPES*	psExpectedTypes,
	UINT8*	pbDest
)
{
	UINT32  rt;
	UINT16	wDataCount, wtemp;
	UINT16	wStatus=E__OK;
	UINT16	wIndex;
	UINT16	wLength;
	UINT16	wMaxSize;
	UINT8	bNumParams;
	UINT8	bData;
	UINT8	bTemp;

	if( IdentifyAgent() != ID__MONARCH )
	{
	/* See if remote master present that supports test primitives parcels */

	interconnect_ReadByte (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, &bTemp);

	if ((bTemp & BIST_REMOTE_PARCEL) != BIST_REMOTE_PARCEL)
        {
#ifdef DEBUG_SOAK
                sysDebugWriteString ("Remote parcel screw up");
#endif
		return (E__CUTE);
        }
	}
	else
	{
		memset(paramBuff, 0x00, 1024);
	}

	if ((psExpectedTypes == NULL) || (pbDest == NULL ))
        {
#ifdef DEBUG_SOAK
                sysDebugWriteString ("Invalid parameters from the caller");
#endif
		return (E__CUTE);
        }

	/* Count the parameter bytes. Byte swap 16 and 32 bit values in
	 * the outgoing parcel.
	 */

	wDataCount = 0;
	bNumParams = psExpectedTypes->bNumParams;

	for (wIndex = 0; wIndex < bNumParams; wIndex++)
	{
		if (psExpectedTypes->pbTypes[wIndex] == IDX_UINT8_TYPE)
			wDataCount = wDataCount + 1;

		else if (psExpectedTypes->pbTypes[wIndex] == IDX_UINT16_TYPE)
		{
			wDataCount = wDataCount + 2;
		}

		else if (psExpectedTypes->pbTypes[wIndex] == IDX_UINT32_TYPE)
		{
			wDataCount = wDataCount + 4;
		}

		else						/* error if unsupported type */
		{
#ifdef DEBUG_SOAK
                        sysDebugWriteString ("Unsupported type!!");
#endif
                        return (E__CUTE);
                }
	}

	if( IdentifyAgent() != ID__MONARCH )
	{
		/* Send the request parcel, header */
	vSendHeader (GET_PARAMETERS_REQUEST, wDataCount + bNumParams + 1);

	/* Send the number of parameters */

	interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, bNumParams, FALSE);

	/* Send the parameter types */

	for (wIndex = 0; wIndex < bNumParams; wIndex++)
		interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, psExpectedTypes->pbTypes[wIndex], FALSE);

	/* Send the default parameters as a byte stream */

	for (wIndex = 0; wIndex < (wDataCount - 1); wIndex++)
		interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, pbDest[wIndex], FALSE);

	/* Send the last byte of the parcel */
	interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, pbDest[wIndex], TRUE);
	}
	else
	{
		wLength = wDataCount + bNumParams + 1;

		paramBuff[0] = (UINT8)wLength;
		paramBuff[1] = (UINT8)(wLength>>8);
		paramBuff[2] = bNumParams;

		/* copy the parameter types */
		for( wIndex = 0; wIndex < bNumParams; wIndex++ )
			paramBuff[3 + wIndex] = psExpectedTypes->pbTypes[wIndex];

		wtemp = wIndex;

		/* copy the default parameters as a byte stream */
		for( wIndex = 0; wIndex < (wDataCount - 1); wIndex++ )
			paramBuff[3 + wtemp + wIndex] = pbDest[wIndex];

		rt = pParamReq( SysCon_ID, paramBuff, 1024, (wLength+2) );
	}

	if( IdentifyAgent() != ID__MONARCH )
	{

		/* Receive the response parcel */
	vReceiveHeader (&bTemp, &wLength, &wStatus);

		wLength -= 2;			/* set length of rest of parcel */
	wMaxSize = wLength;

	if (wStatus == E__OK)
		wIDX_ReceiveParcel (INTERCONNECT_LOCAL_SLOT, pbDest, wMaxSize, &wLength);

	else	/* flush buffer if error detected in header */
	{
		interconnect_ReadBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS,
								BIST_INPUT_PENDING, &bData);

		while (bData == BIST_INPUT_PENDING)
		{
			interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, &bTemp);

			interconnect_ReadBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS,
									BIST_INPUT_PENDING, &bData);
		}
	}
	}
	else
	{
		if(rt == E__OK)
		{
			#ifdef DEBUG_SOAK
				sysDebugWriteString ("SysCon Params Received");
			#endif

			/* copy the default parameters as a byte stream */
			for( wIndex = 0; wIndex < (wDataCount - 1); wIndex++ )
				pbDest[wIndex] = paramBuff[wIndex];
		}
	}

	return (wStatus);

} /* wIDX_GetParams () */


/*****************************************************************************
 *  TITLE:  wIDX_Pwrite ()
 *  ABSTRACT: Byte, word, double word and c are only supported data types.
 * 	RETURNS:  E_OK or E__IDX_PARAM
 *****************************************************************************/
UINT32 wIDX_Pwrite(	PRINT_CONTROL*	psPrintControl,
	char*	achMessage,
	UINT8*	pbParams
)
{
	UINT16	wParamIndex;		 /* amount of data in params */
	UINT16	wStringLength;
	UINT16	wIndex;
	UINT16	wStatus;
	UINT8	abPrintCont[3];		/* print controls to send out */
	UINT8	bTemp;


	if (achMessage == NULL)
		return (E__CUTE);

	/* Initialise the status returned */
	wStatus = E__OK;

	interconnect_ReadByte (INTERCONNECT_LOCAL_SLOT,INTERCONNECT_BIST_MASTER_STATUS, &bTemp);

	if ((bTemp & BIST_REMOTE_PARCEL) != BIST_REMOTE_PARCEL)
		return (wStatus);


	/* Parse the input string to get it's length including the zero
	 * terminators and the amount of data it requires out of the parameter
	 * block.
	 */

	wStringLength = wLengthParser (&wParamIndex, pbParams, (UINT8 *)achMessage, &wStatus);
	wStringLength++;

	if (wStatus != E__OK)
		return (wStatus);


	/* Send request parcel header.	Add 3 bytes for print controls */

	vSendHeader (PAD_WRITE_REQUEST, wStringLength + wParamIndex + 3);

	/* If the pointer is valid, get the print control parameters */

	if (psPrintControl != NULL)
	{
		abPrintCont[0] = psPrintControl->bErl;
		abPrintCont[1] = psPrintControl->bThisLine;
		abPrintCont[2] = psPrintControl->bColumn;
	}

	else	/* use default values */
	{
		abPrintCont[0] = 0;		/* Print at any reporting level */
		abPrintCont[1] = 0;		/* next line */
		abPrintCont[2] = 2;
	}

	/* Send the print controls */

	for (wIndex = 0; wIndex <= 2; wIndex++)
		interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, abPrintCont[wIndex], FALSE);


	/* Send the ASCII message string, except the zero terminator */

	for (wIndex = 0; wIndex < (wStringLength - 1); wIndex++)
		interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, achMessage[wIndex], FALSE);

	/* If no parameters present, zero terminator is last byte of message */

	if ((pbParams == NULL) || (wParamIndex == 0))
		 interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, NULL_CHAR, TRUE);

	else	/* Send zero terminator followed by the parameter information */
	{
		interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, NULL_CHAR, FALSE);

		for (wIndex = 0; wIndex < (wParamIndex - 1); wIndex++)
			interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, pbParams[wIndex], FALSE);

		interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, pbParams[wIndex], TRUE); /* last byte */
	}

	return (wStatus);

} /* wIDX_Pwrite () */


/*****************************************************************************
 *  TITLE:  wIDX_ReceiveParcel ()
 *  ABSTRACT:  receive a data buffr from an agent
 * 	RETURNS: E_OK, E__RETRY_OK, E__LENGTH_OVERFLOW or GSR error value
 *****************************************************************************/
UINT32 wIDX_ReceiveParcel( UINT8   bSlot,		/* agent ID                         	 */
	UINT8*	pbData,			/* buffer for receive data        */
	UINT16	wMaxSize,		/* maximum size of receive buffer */
	UINT16* pwActualSize	/* actual length of received data */
)
{
	UINT32	ExitStatus;		/* overall status, indicates any retries */
	UINT32	Status;		/* general status                       */
	UINT16	wIndex;		/* receive buffer index/data count      */
	UINT8	bTemp;		/* general data                         */
	UINT8   bMySlotId;		/*										 */
	UINT32  tmOut;

	ExitStatus = E__OK;

	/* See where data is coming from, local slot or off-board */
	interconnect_LocalSlotId(&bMySlotId );
	if ((bSlot == INTERCONNECT_LOCAL_SLOT) || (bSlot == bMySlotId))
	{
		/* Wait until start of packet available */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, &bTemp );

			if((bTemp & BIST_INPUT_PENDING) != BIST_INPUT_PENDING )
			{
				vDelay(1);
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						#ifdef DEBUG_SOAK
							sysDebugWriteString("\nwIDX_ReceiveParcel() TIMED OUT 1000\n\n");
						#endif
						return E__FAIL;
					}
				}
			}

		} while ((bTemp & BIST_INPUT_PENDING) != BIST_INPUT_PENDING);

		/* Read all bytes of the packet, continue to read if extra data
		 * available, but discard it */
		wIndex = 0;
		do
		{
			if (wIndex < wMaxSize)
				Status = interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, &pbData[wIndex]);
			else
				Status = interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, &bTemp);

			wIndex++;

			if( Status == E__RETRY_OK )	/* record retries    */
				ExitStatus = Status;

			else if (Status != E__OK) 	/* exit now if error */
			{
				*pwActualSize = wIndex;
				return (Status);
			}

			interconnect_ReadByte(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, &bTemp );

		} while ((bTemp & BIST_INPUT_PENDING) == BIST_INPUT_PENDING);

	}	/* local agent */
	else /* data from off-board agent */
	{
		/* Wait until start of packet available */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &bTemp );
			if( (bTemp & BIST_OUTPUT_PENDING) != BIST_OUTPUT_PENDING)
			{
				vDelay(1);
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						#ifdef DEBUG_SOAK
							sysDebugWriteString("\nwIDX_ReceiveParcel() TIMED OUT 1000 \n\n");
						#endif
						return E__FAIL;
					}
				}
			}
		} while ((bTemp & BIST_OUTPUT_PENDING) != BIST_OUTPUT_PENDING);

		/* Read all bytes of the packet, continue to read if extra data
		 * available, but discard it
		 */
		wIndex = 0;
		do
		{
			if (wIndex < wMaxSize)
				Status = interconnect_ReceiveByte (bSlot, &pbData[wIndex]);
			else
				Status = interconnect_ReceiveByte (bSlot, &bTemp);

			wIndex++;

			if (Status == E__RETRY_OK)	/* record re-tries */
				ExitStatus = Status;

			else if (Status != E__OK) 	/* exit now if error */
			{
				*pwActualSize = wIndex;
				return (Status);
			}

			interconnect_ReadByte(bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &bTemp );

		} while ((bTemp & BIST_OUTPUT_PENDING) == BIST_OUTPUT_PENDING);

	}	/* off-board agent */

	/* Check for an overrun condition */
	*pwActualSize = wIndex;

	if (wIndex > wMaxSize)
		ExitStatus = E__LENGTH_OVERRUN;

	return (ExitStatus);

} /* wIDX_ReceiveParcel () */




/*****************************************************************************
 *  TITLE:  wIDX_SendParcel ()
 *  ABSTRACT: Send a data buffer to an agent
 * 	RETURNS: E_OK, E__RETRY_OK or GSR error value
 *****************************************************************************/
UINT32 wIDX_SendParcel( UINT8	bSlot,		/* agent ID */
	UINT8*	pbData,		/* data buffer */
	UINT16	wSize		/* length of data buffer */
)
{
	UINT32	ExitStatus;	/* overall status, indicates any retrys */
	UINT32	Status;		/* general status */
	UINT16	wIndex;			/* receive buffer index/data count */
	UINT8	bEndOfPacket;	/* last byte flag */

	ExitStatus = E__OK;
	bEndOfPacket  = 0;

	for (wIndex = 0; wIndex < wSize; wIndex++)
	{
		if (wIndex == (wSize - 1))		/* special case: last byte */
			bEndOfPacket = 1;

		Status = interconnect_SendByte (bSlot, pbData[wIndex], bEndOfPacket);

		if( Status == E__RETRY_OK )	/* record retries    */
			ExitStatus = Status;

		else if (Status != E__OK)		/* exit now if error */
			return (Status);
	}

	return (ExitStatus);

} /* wIDX_SendParcel () */




/*****************************************************************************
 *  TITLE:  wIDX_DetectParcel ()
 *  ABSTRACT: Detects the parcel from where data is coming from local slot
 *  		   or off-board.
 * 	RETURNS: E_OK, E__NOT_PENDING
 *****************************************************************************/
UINT32 wIDX_DetectParcel( UINT8	 bSlot,
						  UINT8* pbType
						 )
{
	UINT32	Status;
	UINT8	bData;
	UINT8   bMySlotId;
#ifdef DEBUG_SOAK
	UINT8   buff[64];
#endif

	/* See where data is coming from, local slot or off-board */
	interconnect_LocalSlotId(&bMySlotId );
	if ((bSlot == INTERCONNECT_LOCAL_SLOT) || (bSlot == bMySlotId))
	{
		interconnect_ReadByte(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, &bData );

		if ((bData & BIST_IN_DATA_VALID) == BIST_IN_DATA_VALID)
			Status = interconnect_ReceiveByte (INTERCONNECT_LOCAL_SLOT, pbType);
		else
			Status = E__NOT_PENDING;
	}	/* local agent */
	else /* data from off-board agent */
	{
		interconnect_ReadByte(bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &bData );

		if(bData == 0xff)
		{
			#ifdef DEBUG_SOAK
				sprintf(buff, "\nwIDX_DetectParcel() slot %d received 0xff \n\n", bSlot);
				sysDebugWriteString(buff);
			#endif
			return( E__NOT_PENDING);
		}

		if ((bData & BIST_OUT_DATA_VALID) == BIST_OUT_DATA_VALID)
		{
			Status = interconnect_ReceiveByte (bSlot, pbType);
		}
		else
			Status = E__NOT_PENDING;

	}	/* remote agent */

	return (Status);

} /* wIDX_DetectParcel () */




/*****************************************************************************
 *  TITLE:  wIDX_BadParcel ()
 *  ABSTRACT: Read in and discard an unsupported packet. If the reply flag
 *  		  is non-zero an INVALID_RQUEST_RESPONSE is returned to the
 * 			  sender.
 * 	RETURNS: E_OK, E__RETRY_OK or the GSR error code
 *****************************************************************************/
UINT32 wIDX_BadParcel( UINT8 bSlot,
					   UINT8 bType,
					   UINT8 bReplyFlag
					  )
{
	INVALID_RESPONSE	sInvalidResponse;
	UINT32	Status;
	UINT8	bData;
#ifdef DEBUG_SOAK
	UINT8   buff[64];
#endif
	Status = E__OK;

	#ifdef DEBUG_SOAK
		sprintf(buff, "\nwIDX_BadParcel() slot %d\n\n", bSlot);
		sysDebugWriteString(buff);
	#endif
	/* While data is available, read and dispose */
	interconnect_ReadByte(bSlot, INTERCONNECT_BIST_MASTER_STATUS, &bData );

	while ((bData & BIST_INPUT_PENDING) == BIST_INPUT_PENDING)
	{
		Status = interconnect_ReceiveByte (bSlot, &bData);
		interconnect_ReadByte( bSlot, INTERCONNECT_BIST_MASTER_STATUS, &bData );
	}

	/* If a reply is required, send one */
	if (bReplyFlag != 0)
	{
		sInvalidResponse.bParcelType    = INVALID_REQUEST_RESPONSE;
		sInvalidResponse.bReqParcelType = bType;

		Status = wIDX_SendParcel ( bSlot, (UINT8*)&sInvalidResponse,
									       sizeof (sInvalidResponse) );
	}

	return (Status);

} /* wIDX_BadParcel () */

