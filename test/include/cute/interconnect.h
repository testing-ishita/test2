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
 * interconnect.h
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/cute/interconnect.h,v 1.3 2014-01-29 13:21:42 hchalla Exp $
 * $Log: interconnect.h,v $
 * Revision 1.3  2014-01-29 13:21:42  hchalla
 * *** empty log message ***
 *
 * Revision 1.2  2013-11-25 10:55:57  mgostling
 * Added missing CVS headers and VPX semaphores.
 *
 *
 */
#ifndef INTERCONENCT_H_
#define INTERCONENCT_H_


#define LOCAL_MEMORY_RECORD       0x11
#define MEMORY_RECORD             0x01
#define WINDOW_RECORD             0x0C
#define WINDOW_LOW                0x02
#define WINDOW_HIGH               0x03
#define END_ADDRESS_LOW           0x04
#define END_ADDRESS_HIGH          0x05
#define MEM_SIZE_LOW              0x02
#define MEM_SIZE_HIGH             0x03
#define BOARD_SPECIFIC_RECORD     0xFE
#define HARDWARE_REVISION         0x02
#define FIRMWARE_REVISION         0x03
#define FIRMWARE_COMMS            0x0F
#define BYTE_SWAP                 0xF0


#define INTERCONNECT_FIRST_FUNCTION_RECORD		0x20
#define INTERCONNECT_MAX_CARD_SLOTS             0x14
#define INTERCONNECT_END_OF_TEMPLATE            0xFF

/**************************************************************************
 *							PARCEL LITERALS								  *
 **************************************************************************/

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


#define INTERCONNECT_BIST_SUPPORT_LEVEL		0x1A
	#define BIST_SUPPORT_LEVEL_BITS			0x07
	#define	BIST_LEVEL_0					0
	#define	BIST_LEVEL_1					1
	#define	BIST_LEVEL_2					2
	#define	BIST_LEVEL_3					3
	#define IDX_SUPPORT						0x20
	#define LOCAL_MASTER					0x40
	#define POTENTIAL_MASTER				0x80

#define INTERCONNECT_BIST_DATA_IN			0x1B
#define INTERCONNECT_BIST_DATA_OUT			0x1C

#define INTERCONNECT_BIST_SLAVE_STATUS		0x1D
	#define BIST_OUT_DATA_VALID				0x01
	#define BIST_IN_DATA_ACCEPT				0x02
	#define BIST_RUNNING					0x04
	#define BIST_OUTPUT_PENDING				0x08
	#define INITIALIZATION_CHECK			0x10
	#define RSTNC_TIME_OUT					0x20
	#define BIST_ABORT						0x40
	#define BIST_PASS_FAIL_STATUS			0x80

#define INTERCONNECT_BIST_MASTER_STATUS		0x1E
	#define BIST_IN_DATA_VALID				0x01
	#define BIST_OUT_DATA_ACCEPT			0x02
	#define BIST_ATTENTION					0x04
	#define BIST_INPUT_PENDING				0x08
	#define BIST_COMPLETE					0x20
	#define BIST_REMOTE_PARCEL				0x80

#define INTERCONNECT_MASTER_SEMAPHORE  0x4D
#define BIST_MASTER_BUSY_FLAG  0x01


#define INTERCONNECT_SLAVE_SEMAPHORE     0x4E
#define BIST_SLAVE_BUSY_FLAG  0x01

#define INTERCONNECT_BIST_TEST_ID           0x1f


/* BIST SLAVE STATUS REGISTER BIT MASKS
   ------------------------------------ */

#define INTERCONNECT_BIST_OUT_DATA_VALID         0x01
#define INTERCONNECT_BIST_IN_DATA_ACCEPT         0x02
#define INTERCONNECT_BIST_RUNNING                0x04
#define INTERCONNECT_BIST_OUTPUT_PENDING         0x08
#define INTERCONNECT_INITIALIZATION_CHECK        0x10
#define INTERCONNECT_RSTNC_TIME_OUT              0x20
#define INTERCONNECT_BIST_ABORT                  0x40
#define INTERCONNECT_BIST_PASS_FAIL_STATUS       0x80


#define BIST_PASS                       0x0000
#define BIST_FAIL                       0x0080


UINT32 initializeInterconnect (void);
UINT32 interconnect_LocalSlotId( UINT8*	pbSlotId );
UINT32 interconnect_set_logical_slot_ID( UINT8	bSlotId );
UINT32 interconnect_extended_address ( UINT8 EA_enable);
UINT32 interconnect_ProcessorReset (void);
UINT32 interconnect_ReadByte( UINT8 bSlot, UINT16 wOffset, UINT8* pbData );
UINT32 interconnect_WriteByte( UINT8 bSlot, UINT16 wOffset, UINT8 bData );
UINT32 interconnect_WriteVerify( UINT8 bSlot, UINT16 wOffset, UINT8	bData);

UINT32 interconnect_RegisterOffset( UINT8 bSlot, UINT8 bRecordType, UINT8 bOccurrence,
						            UINT8 bRegIndex, UINT16* pwReqOffset );

UINT32 interconnect_FindRecord( UINT8 bSlot, UINT8 bRecordType,	UINT8 bOccurrence,
		                        UINT16* pwReqOffset);

UINT32 interconnect_ReadBit(UINT8 bSlot, UINT16	wOffset, UINT8 bMask, UINT8* pbData );
UINT32 interconnect_SetBit( UINT8 bSlot, UINT16 wOffset, UINT8 bMask );
UINT32 interconnect_ClearBit( UINT8	bSlot, UINT16 wOffset, UINT8 bMask );
UINT32 interconnect_ReceiveByte( UINT8	bSlot, UINT8* pbData );
UINT32 interconnect_SendByte( UINT8	bSlot, UINT8 bData, UINT8 bEndFlag );

#endif
