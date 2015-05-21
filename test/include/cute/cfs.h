
#ifndef CFS_H_
#define CFS_H_

#define CFS_FIRST_FUNCTION_RECORD       	0x20
#define CFS_MAX_CARD_SLOTS              	0x14
#define CFS_END_OF_TEMPLATE             	0xFF

/***************************************************************************

    PARCEL LITERALS

***************************************************************************/

/* SUPPORT LEVEL 1 PARCELS
   ----------------------- */

#define TEST_HELP_REQUEST                   0x010
#define TEST_HELP_RESPONSE                  0x011
#define EXECUTE_TEST_REQUEST                0x012
#define EXECUTE_TEST_RESPONSE               0x013
#define INVALID_REQUEST_RESPONSE            0x0FF


/* SUPPORT LEVEL 2 PARCELS
   ----------------------- */

#define DOWNLOAD_TEST_REQUEST               0x014
#define DOWNLOAD_TEST_RESPONSE              0x015
#define DOWNLOAD_TEST_DATA                  0x016
#define DOWNLOAD_TEST_DATA_RESPONSE         0x017
#define PURGE_DOWNLOAD_TEST_REQUEST         0x018
#define PURGE_DOWNLOAD_TEST_RESPONSE        0x019


/* SUPPORT LEVEL 3 PARCELS
   ----------------------- */

#define HARDWARE_CONFIGURATION_REQUEST      0x01A
#define HARDWARE_CONFIGURATION_RESPONSE     0x01B
#define GET_PARAMETERS_REQUEST              0x01C
#define GET_PARAMETERS_RESPONSE             0x01D
#define PAD_WRITE_REQUEST                   0x01E
#define PROMPT_AND_READ_REQUEST             0x01F
#define PROMPT_AND_READ_RESPONSE            0x020


#define CFS_BIST_SUPPORT_LEVEL				0x1A
	#define BIST_SUPPORT_LEVEL_BITS			0x07
	#define	BIST_LEVEL_0					0
	#define	BIST_LEVEL_1					1
	#define	BIST_LEVEL_2					2
	#define	BIST_LEVEL_3					3
	#define IDX_SUPPORT						0x20
	#define LOCAL_MASTER					0x40
	#define POTENTIAL_MASTER				0x80

#define CFS_BIST_DATA_IN					0x1B
#define CFS_BIST_DATA_OUT					0x1C

#define CFS_BIST_SLAVE_STATUS				0x1D
	#define BIST_OUT_DATA_VALID				0x01
	#define BIST_IN_DATA_ACCEPT				0x02
	#define BIST_RUNNING					0x04
	#define BIST_OUTPUT_PENDING				0x08
	#define INITIALIZATION_CHECK			0x10
	#define RSTNC_TIME_OUT					0x20
	#define BIST_ABORT						0x40
	#define BIST_PASS_FAIL_STATUS			0x80

#define CFS_BIST_MASTER_STATUS				0x1E
	#define BIST_IN_DATA_VALID				0x01
	#define BIST_OUT_DATA_ACCEPT			0x02
	#define BIST_ATTENTION					0x04
	#define BIST_INPUT_PENDING				0x08
	#define BIST_COMPLETE					0x20
	#define BIST_REMOTE_PARCEL				0x80
#define CFS_BIST_TEST_ID           			0x1f


/* BIST SLAVE STATUS REGISTER BIT MASKS
   ------------------------------------ */

#define CFS_BIST_OUT_DATA_VALID         0x01
#define CFS_BIST_IN_DATA_ACCEPT         0x02
#define CFS_BIST_RUNNING                0x04
#define CFS_BIST_OUTPUT_PENDING         0x08
#define CFS_INITIALIZATION_CHECK        0x10
#define CFS_RSTNC_TIME_OUT              0x20
#define CFS_BIST_ABORT                  0x40
#define CFS_BIST_PASS_FAIL_STATUS       0x80



#define BIST_PASS                       0x0000
#define BIST_FAIL                       0x0080

UINT32 CFS_LocalSlotId(	UINT8*	pbSlotId );
UINT32 CFS_set_logical_slot_ID(	UINT8	bSlotId );
UINT32 CFS_InitializeInterconnect (void);
UINT32 CFS_extended_address (UINT8 EA_enable);
UINT32 CFS_ProcessorReset (void);
UINT32 CFS_ReadByte( UINT8 bSlot, UINT16 wOffset, UINT8* pbData );
UINT32 CFS_WriteByte( UINT8	bSlot, UINT16 wOffset, UINT8 bData );
UINT32 CFS_WriteVerify(	UINT8 bSlot, UINT16	wOffset, UINT8	bData);

UINT32 CFS_RegisterOffset( UINT8 bSlot,	UINT8 bRecordType, UINT8 bOccurrence,
						   UINT8 bRegIndex,	UINT16*	pwReqOffset );

UINT32 CFS_FindRecord( UINT8 bSlot,	UINT8 bRecordType,	UINT8 bOccurrence,
		               UINT16* pwReqOffset);

UINT32 CFS_ReadBit(	UINT8 bSlot, UINT16	wOffset, UINT8 bMask, UINT8* pbData );
UINT32 CFS_SetBit( UINT8 bSlot,	UINT16 wOffset,	UINT8 bMask );
UINT32 CFS_ClearBit( UINT8	bSlot, UINT16 wOffset, UINT8 bMask );
UINT32 CFS_ReceiveByte(	UINT8	bSlot,	UINT8*	pbData );
UINT32 CFS_SendByte( UINT8	bSlot, UINT8	bData, UINT8	bEndFlag );
UINT32 CFS_ReceiveParcel( UINT8	bSlot, UINT8* pbData, UINT16 wMaxSize, UINT16* pwActualSize);
UINT32 CFS_SendParcel( UINT8	bSlot, UINT8* pbData, UINT16 wSize	);
UINT32 CFS_DetectParcel( UINT8	bSlot,	UINT8*	pbType);
UINT32 CFS_BadParcel( UINT8	bSlot, UINT8 bType,	UINT8 bReplyFlag );

#endif
