
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

/*
 * sm3lib.c: sm3 core functions
 */

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
#include <cute/interconnect.h>

#define RECORD_OFFSET_TBL_SIZE	255
#define MAX_RETRYS				3		/* interconnect write retrys */


typedef struct tagRecordOffset
{
	UINT8	bType;
	UINT16	wOffset;

} RECORD_OFFSET;


static RECORD_OFFSET	asRecordOffset[RECORD_OFFSET_TBL_SIZE];
extern UINT8 	my_slot;

/*****************************************************************************
 * initializeInterconnect: configure local interconnect data
 *
 * RETURNS: E_OK allways
 */
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




/*****************************************************************************
 * interconnect_extended_address: Sets the global access permissions for all records
 * 						 beyond the header record.
 * RETURNS: E_OK always
 */

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
			bFlags = bReadIcByte( INTERCONNECT_LOCAL_SLOT, dRegOffset);
			bFlags = (bFlags & 0xFB)|bMask;
			vWriteIcByte( INTERCONNECT_LOCAL_SLOT, dRegOffset, bFlags);
			dRegOffset = dRegOffset + 2;	// next register's flags field
		}

		// Get type of next record
		wOffset = wOffset + bLength;
		interconnect_ReadByte( INTERCONNECT_LOCAL_SLOT, wOffset, &bType );
	}

	return (E__OK);

} // interconnect_extended_address ()



/*****************************************************************************
 * interconnect_ProcessorReset: generate a reset to the local CPU
 * RETURNS: E_OK allways
 */
UINT32 interconnect_ProcessorReset (void)
{
	board_service(SERVICE__BRD_RESET, NULL, NULL); /* call-out to H/W specific CPU reset */

	return (E__OK);

} /* interconnect_ProcessorReset () */





/*****************************************************************************
 * interconnect_ReadByte: read a byte from specified slot's interconnect
 * RETURNS: E_OK allways
 */
UINT32 interconnect_ReadByte(UINT8 bSlot, UINT16	wOffset, UINT8*	pbData )
{
	*pbData = bReadIcByte( bSlot, (wOffset<<1));
	return (E__OK);

} /* interconnect_ReadByte () */




/*****************************************************************************
 * interconnect_WriteByte: write a byte to the specified slot's interconnect
 * RETURNS: E_OK allways
 */
UINT32 interconnect_WriteByte( UINT8	bSlot, UINT16 wOffset, UINT8 bData )
{
	vWriteIcByte( bSlot, (wOffset<<1), bData);
	return E__OK;

} /* interconnect_WriteByte () */




/*****************************************************************************
 * interconnect_WriteVerify: write and verify byte data
 * If first write fails, we retry the write 2 times before issuing an error
 * RETURNS: E_OK, E__RETRY or the GSR value on error
 */
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
		}

		bTrys++;
	}

	return (Status);

} /* interconnect_WriteVerify () */




/*****************************************************************************
 * interconnect_FindRecord: find the offset to specified record type/occurrence
 * RETURNS: E_OK or E__RECORD_NOT_FOUND
 */
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

	if ((bSlot == my_slot) || (bSlot == INTERCONNECT_LOCAL_SLOT))
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


/*****************************************************************************
 * interconnect_RegisterOffset: find offset to register within record type/occurrence
 * RETURNS: E_OK, E__RECORD_NOT_FOUND or E__RECORD_LENGTH
 */
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




/*****************************************************************************
 * interconnect_ReadBit: interconnect read with masking for specified bit(s)
 * RETURNS: E_OK always
 */
UINT32 interconnect_ReadBit(	UINT8 bSlot, UINT16	wOffset, UINT8 bMask, UINT8* pbData )
{
	UINT8 bRegValue;

	interconnect_ReadByte(bSlot, wOffset, &bRegValue );
	*pbData   = bRegValue & bMask;

	return (E__OK);

} /* interconnect_ReadBit () */



/*****************************************************************************
 * interconnect_SetBit: set specified bit(s) in interconnect
 * RETURNS: E_OK, E__RETRY or GSR value on error
 */
UINT32 interconnect_SetBit( UINT8 bSlot,	UINT16 wOffset,	UINT8 bMask )
{
	UINT32	Status;
	UINT8	bRegValue;

	interconnect_ReadByte(bSlot, wOffset, &bRegValue );
	Status = interconnect_WriteVerify (bSlot, wOffset, (bRegValue | bMask));

	return (Status);

} /* interconnect_SetBit () */



/*****************************************************************************
 * interconnect_ClearBit: clear specified bit(s) in interconnect
 * RETURNS: E_OK, E__RETRY or GSR value on error
 */
UINT32 interconnect_ClearBit( UINT8	bSlot, UINT16 wOffset, UINT8 bMask )
{
	UINT32	Status;
	UINT8	bRegValue;

	interconnect_ReadByte(bSlot, wOffset, &bRegValue );
	Status = interconnect_WriteVerify (bSlot, wOffset, bRegValue & (UINT8)~bMask);

	return (Status);

} /* interconnect_ClearBit () */


