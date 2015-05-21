
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


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>

 
#include <bit/bit.h>
#include <bit/delay.h>
#include <bit/board_service.h>

#include <private/cpu.h>
#include <private/debug.h>

#include <cute/arch.h>
#include <cute/interconnect.h>
#include <cute/cute_errors.h>
#include <cute/idx.h>
#include <cute/packet.h>

#define RECORD_OFFSET_TBL_SIZE	255
#define MAX_RETRYS				3		/* interconnect write retrys */


typedef struct tagRecordOffset
{
	UINT8	bType;
	UINT16	wOffset;

} RECORD_OFFSET;


static RECORD_OFFSET	asRecordOffset[RECORD_OFFSET_TBL_SIZE];
static UINT8 			bMySlotId;


/*****************************************************************************\
 *
 *  TITLE:  initializeInterconnect ()
 *
 *  ABSTRACT:  configure local interconnect data by scanning the interconnect
 *  		   table of record type vs register offset.
 *
 * 	RETURNS:  E_OK always
 *
\*****************************************************************************/
UINT32 initializeInterconnect (void)
{
	UINT16	wOffset;
	UINT8	bType;
	UINT8	bCount;
	UINT8	bTemp;

	/* Scan Interconnect and build a table of record type vs register offset */

	board_service(SERVICE__BRD_INIT_ICR, NULL, NULL);

	wOffset = INTERCONNECT_FIRST_FUNCTION_RECORD;
	bCount  = 0;

	do
	{
		interconnect_ReadByte( INTERCONNECT_LOCAL_SLOT, wOffset, &bType );

		asRecordOffset[bCount].bType   = bType;
		asRecordOffset[bCount].wOffset = wOffset;

		if (bType != INTERCONNECT_END_OF_TEMPLATE)
		{
			/* Move offset to the next record */
			interconnect_ReadByte( INTERCONNECT_LOCAL_SLOT, (wOffset + 1), &bTemp );/* length */
			wOffset += (bTemp + 2);
			bCount++;
		}

	} while ((bType != INTERCONNECT_END_OF_TEMPLATE) && (bCount < 0xFF));

	return (E__OK);

} /* initializeInterconnect () */


/*****************************************************************************\
 *
 *  TITLE:  interconnect_extended_address ()
 *
 *  ABSTRACT:  Sets the global access permissions for all records
 * 			   beyond the header record.
 *
 * 	RETURNS:  E_OK always
 *
\*****************************************************************************/
UINT32 interconnect_extended_address (UINT8 EA_enable)
{
	UINT16	wOffset,  dRegOffset;
	UINT8	bType,    bLength;
	UINT8	bCounter, bFlags;
	UINT8	bMask;

	if (EA_enable == 1)
		bMask = 0x04;
	else
		bMask = 0x00;

	wOffset = INTERCONNECT_FIRST_FUNCTION_RECORD;

	interconnect_ReadByte( INTERCONNECT_LOCAL_SLOT, wOffset, &bType );


	while ((bType != INTERCONNECT_END_OF_TEMPLATE) && (wOffset < 0xFF))
	{
		wOffset++;
		interconnect_ReadByte( INTERCONNECT_LOCAL_SLOT, wOffset, &bLength );

		wOffset++;
		dRegOffset = (wOffset * 2) + 1;	// offset to flags field

		for( bCounter = 1;  bCounter <= bLength; bCounter++)
		{
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, dRegOffset, &bFlags);
			bFlags = (bFlags & 0xFB)|bMask;
			bIcWriteByte( INTERCONNECT_LOCAL_SLOT, dRegOffset, &bFlags);
			dRegOffset = dRegOffset + 2;	// next register's flags field
		}

		// Get type of next record
		wOffset = wOffset + bLength;
		interconnect_ReadByte( INTERCONNECT_LOCAL_SLOT, wOffset, &bType );
	}

	return (E__OK);

} /* interconnect_extended_address () */

/*****************************************************************************\
 *
 *  TITLE:  interconnect_ProcessorReset ()
 *
 *  ABSTRACT:  Generate a reset to the local CPU
 *
 * 	RETURNS:  E_OK always
 *
\*****************************************************************************/
UINT32 interconnect_ProcessorReset (void)
{
	board_service(SERVICE__BRD_RESET, NULL, NULL);/* call-out to H/W specific CPU reset */

	return (E__OK);

} /* interconnect_ProcessorReset () */


/*****************************************************************************\
 *
 *  TITLE:  interconnect_LocalSlotId ()
 *
 *  ABSTRACT:  Get the actual slot number for this position
 *
 * 	RETURNS:  E_OK always
 *
\*****************************************************************************/
UINT32 interconnect_LocalSlotId(	UINT8*	pbSlotId )
{
	*pbSlotId = bMySlotId;

	return (E__OK);

} /* interconnect_LocalSlotId () */


/*****************************************************************************\
 *
 *  TITLE:  interconnect_set_logical_slot_ID ()
 *
 *  ABSTRACT:  Set the actual slot number for this position
 *
 * 	RETURNS:  E_OK always
 *
\*****************************************************************************/
UINT32 interconnect_set_logical_slot_ID(	UINT8	bSlotId )
{
	bMySlotId = bSlotId;

	return (E__OK);

} /* interconnect_set_logical_slot_ID () */


/*****************************************************************************\
 *
 *  TITLE:  interconnect_ReadByte ()
 *
 *  ABSTRACT:  Read a byte from specified slot's interconnect
 *
 * 	RETURNS:  E_OK always
 *
\*****************************************************************************/
UINT32 interconnect_ReadByte(UINT8 bSlot, UINT16	wOffset, UINT8*	pbData )
{
	return (bIcReadByte( bSlot, (wOffset<<1), pbData));

} /* interconnect_ReadByte () */


/*****************************************************************************\
 *
 *  TITLE:  interconnect_WriteByte ()
 *
 *  ABSTRACT:  Write a byte to the specified slot's interconnect
 *
 * 	RETURNS:  E_OK always
 *
\*****************************************************************************/
UINT32 interconnect_WriteByte( UINT8	bSlot, UINT16 wOffset, UINT8 bData )
{
	UINT8 temp;
	temp = bData;
	return (bIcWriteByte( bSlot, (wOffset<<1), &temp));

} /* interconnect_WriteByte () */


/*****************************************************************************\
 *
 *  TITLE:  interconnect_WriteVerify ()
 *
 *  ABSTRACT:  write and verify byte data if first write fails,
 *  		    we retry the write 2 times before issuing an error
 *
 * 	RETURNS:  E_OK, E__RETRY or the GSR value on error
 *
\*****************************************************************************/
UINT32 interconnect_WriteVerify(	UINT8 bSlot, UINT16	wOffset, UINT8	bData)
{
	UINT32	Status;
	UINT8		bVerify;
	UINT8		bTrys;

	Status = E__OK;
	bTrys     = 0;

	interconnect_WriteByte (bSlot, wOffset, bData);
	interconnect_ReadByte(bSlot, wOffset, &bVerify );

	while ((bVerify != bData) && (bTrys <= MAX_RETRYS))
	{
		Status = E__RETRY_OK;	/* assume retry will succeed */

		interconnect_WriteByte (bSlot, wOffset, bData);
		interconnect_ReadByte(bSlot, wOffset, &bVerify );

		if (bTrys == MAX_RETRYS)
		{
			Status = E__FAIL;
			sysDebugWriteString("\ninterconnect_WriteVerify() FAILED\n");
		}

		bTrys++;
	}

	return (Status);

} /* interconnect_WriteVerify () */


/*****************************************************************************\
 *
 *  TITLE:  interconnect_FindRecord ()
 *
 *  ABSTRACT:  Find the offset to specified record type/occurrence
 *
 * 	RETURNS:  E_OK or E__RECORD_NOT_FOUND
 *
\*****************************************************************************/
UINT32 interconnect_FindRecord( UINT8 bSlot,	UINT8 bRecordType,	UINT8 bOccurrence,
		               UINT16*	pwReqOffset)
{
	UINT32	Status;	/* scan result */
	UINT16	wOffset;		/* interconnext offset */
	UINT8	bType;			/* record type during scan */
	UINT8	bLength;		/* length of record during scan */
	UINT8	bCount;			/* found occurrences count */
	UINT8	bIndex;			/* table scan index */


	Status = E__RECORD_NOT_FOUND;
	wOffset   = 0;
	bCount    = 0;

	if ((bSlot == bMySlotId) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		bIndex = 0;

		do {
			bType = asRecordOffset[bIndex].bType;

			if (bType == bRecordType)
			{
				bCount++;
				wOffset = asRecordOffset[bIndex].wOffset;
			}

			bIndex++;

			if (bIndex == RECORD_OFFSET_TBL_SIZE)	/* force end of template */
				bType = INTERCONNECT_END_OF_TEMPLATE;

		} while ((bCount != bOccurrence) && (bType != INTERCONNECT_END_OF_TEMPLATE));
	}

	else	/* off-board slot */
	{
		wOffset = INTERCONNECT_FIRST_FUNCTION_RECORD;

		do {
			interconnect_ReadByte(bSlot, wOffset, &bType );
			interconnect_ReadByte(bSlot, wOffset, &bLength );

			if (bType == bRecordType)
				bCount++;

			if (bCount < bOccurrence)
				wOffset += (bLength + 2);

		} while ((bCount != bOccurrence) && (bType != INTERCONNECT_END_OF_TEMPLATE));
	}

	/* If we found the type/occurrence, return the offset to the type */

	if (bCount == bOccurrence)
	{
		*pwReqOffset = wOffset;
		Status = E__OK;
	}

	return (Status);

} /* interconnect_FindRecord () */

/*****************************************************************************\
 *
 *  TITLE:  interconnect_RegisterOffset ()
 *
 *  ABSTRACT:  Find offset to register within record type/occurrence
 *
 * 	RETURNS:  E_OK, E__RECORD_NOT_FOUND or E__RECORD_LENGTH
 *
\*****************************************************************************/
UINT32 interconnect_RegisterOffset( UINT8 bSlot,	UINT8 bRecordType,	UINT8 bOccurrence,
						   UINT8 bRegIndex,	UINT16*	pwReqOffset )
{
	UINT32	Status;
	UINT16	wOffset;
	UINT8	bLength;

	/* Find the start of the requested type/occurrence */
	Status = interconnect_FindRecord (bSlot, bRecordType, bOccurrence, &wOffset);

	/* If we found the type/occurrence, check register is inside that record */
	if (Status == E__OK)
	{
		*pwReqOffset = wOffset + bRegIndex;
		interconnect_ReadByte(bSlot, wOffset+1, &bLength );

		if (bRegIndex > (bLength + 1))
			Status = E__RECORD_LENGTH;
	}

	return (Status);

} /* interconnect_RegisterOffset () */


/*****************************************************************************\
 *
 *  TITLE:  interconnect_ReadBit ()
 *
 *  ABSTRACT:  Interconnect read with masking for specified bit(s)
 *
 * 	RETURNS:  E_OK always
 *
\*****************************************************************************/
UINT32 interconnect_ReadBit(	UINT8 bSlot, UINT16	wOffset, UINT8 bMask, UINT8* pbData )
{
	UINT8 bRegValue;

	interconnect_ReadByte(bSlot, wOffset, &bRegValue );
	*pbData   = bRegValue & bMask;

	return (E__OK);

} /* interconnect_ReadBit () */



/*****************************************************************************\
 *
 *  TITLE:  interconnect_SetBit ()
 *
 *  ABSTRACT:  Set specified bit(s) in interconnect
 *
 * 	RETURNS:  E_OK, E__RETRY or GSR value on error
 *
\*****************************************************************************/
UINT32 interconnect_SetBit( UINT8 bSlot,	UINT16 wOffset,	UINT8 bMask )
{
	UINT32	Status;
	UINT8	bRegValue;

	interconnect_ReadByte(bSlot, wOffset, &bRegValue );
	Status = interconnect_WriteVerify (bSlot, wOffset, (bRegValue | bMask));

	return (Status);

} /* interconnect_SetBit () */



/*****************************************************************************\
 *
 *  TITLE:  interconnect_ClearBit ()
 *
 *  ABSTRACT:  Clear specified bit(s) in interconnect
 *
 * 	RETURNS: E_OK, E__RETRY or GSR value on error
 *
\*****************************************************************************/
UINT32 interconnect_ClearBit( UINT8	bSlot, UINT16 wOffset, UINT8 bMask )
{
	UINT32	Status;
	UINT8	bRegValue;

	interconnect_ReadByte(bSlot, wOffset, &bRegValue );
	Status = interconnect_WriteVerify (bSlot, wOffset, bRegValue & (UINT8)~bMask);

	return (Status);

} /* interconnect_ClearBit () */



/*****************************************************************************\
 *
 *  TITLE:  interconnect_ReceiveByte ()
 *
 *  ABSTRACT:  Receive a byte from some agent via BIST comms. mechanism
 *
 * 	RETURNS: E_OK, E__RETRY_OK or the GSR error value
 *
\*****************************************************************************/

UINT32 interconnect_ReceiveByte(	UINT8	bSlot,	UINT8*	pbData )
{
	UINT32	ExitStatus;	/* overall status, indicates any retrys */
	UINT32	Status;		/* general status 						*/
	UINT8	bTemp;		/* general data   						*/
	UINT32  tmOut;

	ExitStatus = E__OK;

	/* See where data is coming from, local slot or off-board */
	if ((bSlot == INTERCONNECT_LOCAL_SLOT) || (bSlot == bMySlotId))
	{
		/* Wait for valid data in BIST_IN_DATA register, and read it */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, &bTemp );

			if((bTemp & BIST_IN_DATA_VALID) != BIST_IN_DATA_VALID )
			{
				vDelay(1);
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						sysDebugWriteString("\ninterconnect_ReceiveByte() TIMED OUT 1\n\n");
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING);
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID);
						return (E__COMMUNICATION_ERROR);
					}
				}
			}
		} while ((bTemp & BIST_IN_DATA_VALID) != BIST_IN_DATA_VALID);

		interconnect_ReadByte(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_DATA_IN, pbData );

		/* Confirm we have read the data, record any retrys, exit on error */
		Status = interconnect_SetBit ( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS,
						         BIST_IN_DATA_ACCEPT );

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

		/* Wait for BIST_IN_DATA_VALID bit to be reset */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, &bTemp );

			if((bTemp & BIST_IN_DATA_VALID) == BIST_IN_DATA_VALID )
			{
				vDelay(1);
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						sysDebugWriteString("\ninterconnect_ReceiveByte() TIMED OUT 2\n\n");
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING);
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID);
						return (E__COMMUNICATION_ERROR);
					}
				}
			}

		} while ((bTemp & BIST_IN_DATA_VALID) == BIST_IN_DATA_VALID);

		/* Clear the BIST_IN_DATA_ACCEPT bit, record any retrys, exit on error */

		Status = interconnect_ClearBit ( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS,
						           BIST_IN_DATA_ACCEPT );

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

	}	/* local agent */

	else /* data from off-board agent */
	{
		/* Wait for valid data in BIST_OUT_DATA register, and read it */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &bTemp );

			if((bTemp & BIST_OUT_DATA_VALID) != BIST_OUT_DATA_VALID )
			{
				vDelay(1);
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						sysDebugWriteString("\ninterconnect_ReceiveByte() TIMED OUT 3\n\n");
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING);
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID);
						return (E__COMMUNICATION_ERROR);
					}
				}
			}

		} while ((bTemp & BIST_OUT_DATA_VALID) != BIST_OUT_DATA_VALID);

		interconnect_ReadByte(bSlot, INTERCONNECT_BIST_DATA_OUT, pbData );

		/* Confirm we have read the data, record any retrys, exit on error */
		Status = interconnect_SetBit( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_OUT_DATA_ACCEPT );

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

		/* Wait for BIST_OUT_DATA_VALID bit to be reset */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &bTemp );

			if((bTemp & BIST_OUT_DATA_VALID) == BIST_OUT_DATA_VALID)
			{
				vDelay(1);
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						sysDebugWriteString("\ninterconnect_ReceiveByte() TIMED OUT 4\n\n");
						interconnect_ClearBit( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING );
						interconnect_ClearBit( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID );
						return( E__COMMUNICATION_ERROR );
					}
				}
			}

		} while ((bTemp & BIST_OUT_DATA_VALID) == BIST_OUT_DATA_VALID);

		/* Clear the BIST_OUT_DATA_ACCEPT bit, record any retries, exit on error */

		Status = interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS,
						           BIST_OUT_DATA_ACCEPT);

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

	}	/* off-board agent */

	return (ExitStatus);		/* E_OK or E__RETRY_OK */

} /* interconnect_ReceiveByte () */





/*****************************************************************************\
 *
 *  TITLE:  interconnect_SendByte ()
 *
 *  ABSTRACT:  Send a byte to agent via BIST comms for off-board or on-board
 *
 * 	RETURNS: E_OK, E__RETRY_OK or the GSR error value
 *
\*****************************************************************************/
UINT32 interconnect_SendByte( UINT8	bSlot, UINT8	bData, UINT8	bEndFlag )
{
	UINT32	ExitStatus;	/* overall status, indicates any retrys */
	UINT32	Status;		/* general status 						*/
	UINT8	bTimeout;	/* timeout for remote send 				*/
	UINT8	bTemp;		/* general data 						*/
	UINT32  tmOut;

	ExitStatus = E__OK;

	/* See where data is going to, local slot or off-board */
	if ((bSlot == INTERCONNECT_LOCAL_SLOT) || (bSlot == bMySlotId))
	{
		/* Indicate that a byte is to be sent */
		Status = interconnect_SetBit( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, BIST_OUTPUT_PENDING);

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

		/* Write the data, indicate that data is available */
		interconnect_WriteByte (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_DATA_OUT, bData);

		Status = interconnect_SetBit( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, BIST_OUT_DATA_VALID);

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

		/* Wait for acknowledge of data received */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, &bTemp );
			vDelay (1);
			if((bTemp & BIST_OUT_DATA_ACCEPT) != BIST_OUT_DATA_ACCEPT)
			{
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						sysDebugWriteString("\ninterconnect_SendByte() TIMED OUT 1\n\n");
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING);
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID);
						return (E__COMMUNICATION_ERROR);
					}
				}
			}
		} while ((bTemp & BIST_OUT_DATA_ACCEPT) != BIST_OUT_DATA_ACCEPT);

		/* Check if there are more bytes in parcel to send */

		if (bEndFlag != 0)
		{
			Status = interconnect_ClearBit( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS,
					                  BIST_OUTPUT_PENDING );

			if (Status == E__RETRY_OK)
				ExitStatus = Status;

			else if (Status != E__OK)
				return (Status);
		}

		/* Indicate data is no longer valid, i.e. we could change it now */
		Status = interconnect_ClearBit ( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS,
						           BIST_OUT_DATA_VALID );

		if (Status == E__RETRY_OK)
		    ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

		/* Wait for agent to indicate 'data not valid' acknowledged */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, &bTemp );
			vDelay (1);
			if((bTemp & BIST_OUT_DATA_ACCEPT) == BIST_OUT_DATA_ACCEPT)
			{
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						sysDebugWriteString("\ninterconnect_SendByte() TIMED OUT 2\n\n");
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING);
						interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID);
						return (E__COMMUNICATION_ERROR);
					}
				}
			}

		} while ((bTemp & BIST_OUT_DATA_ACCEPT) == BIST_OUT_DATA_ACCEPT);

	}	/* local agent */


	else /* data for off-board agent */
	{
		/* Indicate that a byte is to be sent */
		Status = interconnect_SetBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING);

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

		/* Write the data, indicate that data is available */
		interconnect_WriteByte (bSlot, INTERCONNECT_BIST_DATA_IN, bData);

		Status = interconnect_SetBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID);

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

		/* Wait for acknowledge of data received, 200 mS timeout */
		bTimeout = 0;

		/* Wait for acknowledge of data received */
		do
		{
			interconnect_ReadByte(bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &bTemp );
			vDelay (1);

			bTimeout++;

			if (bTimeout == 200)
			{
				interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING);
				interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID);
				return (E__COMMUNICATION_ERROR);
			}

		} while ((bTemp & BIST_IN_DATA_ACCEPT) != BIST_IN_DATA_ACCEPT);

		/* Check if there are more bytes in parcel to send */
		if (bEndFlag != 0)
		{
			Status = interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING);

			if (Status == E__RETRY_OK)
				ExitStatus = Status;

			else if (Status != E__OK)
				return (Status);
		}

		/* Indicate data is no longer valid, i.e. we could change it now */
		Status = interconnect_ClearBit ( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID);

		if (Status == E__RETRY_OK)
			ExitStatus = Status;

		else if (Status != E__OK)
			return (Status);

		/* Wait for agent to indicate 'data not valid' acknowledged */
		tmOut = 0;
		do
		{
			interconnect_ReadByte(bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &bTemp );
			vDelay (1);
			if((bTemp & BIST_IN_DATA_ACCEPT) == BIST_IN_DATA_ACCEPT)
			{
				tmOut++;
				if(tmOut >= 1000)
				{
					if( IdentifyAgent() == ID__MONARCH )
					{
						sysDebugWriteString( "\ninterconnect_ReceiveByte() TIMED OUT 3\n\n" );
						interconnect_ClearBit( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_INPUT_PENDING );
						interconnect_ClearBit( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_IN_DATA_VALID );
						return( E__COMMUNICATION_ERROR );
					}
				}
			}

		} while ((bTemp & BIST_IN_DATA_ACCEPT) == BIST_IN_DATA_ACCEPT);

	}	/* off-board agent */

	return (ExitStatus);	/* E_OK or E__RETRY_OK */

} /* interconnect_SendByte () */






