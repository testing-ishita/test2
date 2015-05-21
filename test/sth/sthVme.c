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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/sth/sthVme.c,v 1.12 2015-03-13 10:35:06 mgostling Exp $
 * $Log: sthVme.c,v $
 * Revision 1.12  2015-03-13 10:35:06  mgostling
 * Removed redundant test
 *
 * Revision 1.11  2015-03-12 15:29:44  hchalla
 * Fixed issue with parameters passing to doTest, where its corrupting when
 * executing tests because of NULL pointer.
 *
 * Revision 1.10  2015-03-10 15:32:46  mgostling
 * Pass parameters from SOAK master to individual tests.
 *
 * Revision 1.9  2015-02-27 14:01:34  mgostling
 * Fixed compiler warning.
 *
 * Revision 1.8  2015-02-27 11:52:11  mgostling
 * VME bridge initilisation for network VME cute now handled in main.c
 * Setting board name for STH fixed for bridgeless boards
 *
 * Revision 1.7  2015-02-25 18:01:49  hchalla
 * Added support for new STH VME testing using ethernet for soak, returns
 * Board MAC address for board serial number.
 *
 * Revision 1.6  2015-01-29 10:18:31  mgostling
 * Added support for ethernet soak master.
 *
 * Revision 1.5  2014-08-05 08:35:49  mgostling
 * Corrected compiler warnings.
 *
 * Revision 1.4  2014-01-29 13:34:09  hchalla
 * Added new variable count.
 *
 * Revision 1.3  2014-01-10 09:41:05  mgostling
 * Corrected variable declarations in conditional code.
 *
 * Revision 1.2  2013-11-25 11:23:06  mgostling
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013-09-04 07:45:36  chippisley
 * Import files into new source repository.
 *
 * Revision 1.9  2012/03/21 16:29:31  cdobson
 * Eliminate build error and compiler warnings.
 *
 * Revision 1.8  2011/11/21 11:17:06  hmuneer
 * no message
 *
 * Revision 1.7  2011/10/27 15:50:21  hmuneer
 * no message
 *
 * Revision 1.6  2011/05/16 14:31:59  hmuneer
 * Info Passing Support
 *
 * Revision 1.5  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.4  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.3  2010/11/04 17:45:27  hchalla
 * Added debug info.
 *
 * Revision 1.2  2010/06/24 13:48:41  hchalla
 * Code Cleanup, Added headers and comments.
 *
 */


#include <stdtypes.h>
#include <stdio.h>
#include <string.h>
#include <errors.h>
#include <error_msg.h>
 
#include <bit/bit.h>
#include <bit/delay.h>
#include <bit/board_service.h>

#include <private/cpu.h>
#include <private/debug.h>
#include <private/atomic.h>

#include <cute/arch.h>
#include <cute/interconnect.h>
#include <cute/cute_errors.h>
#include <cute/sth.h>
#include <cute/packet.h>
#include <cute/sm3.h>
#include <devices/ipmi.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"


#include "config.h"
#include "../debug/dbgLog.h"

#ifdef INCLUDE_DEBUG_VGA
#include "../vgalib/vga.h"
#endif

#ifdef INCLUDE_LWIP_SMC
/* Don't include <devices/ipmi.h> as some defines clash! e.g. E__TIMEOUT */
/* #include <devices/ipmi.h> */
/* From ipmi.h */
#define NFC_STORAGE_REQUEST	0x0A
#define LUN_BMC	0x00
#endif

//#define DEBUG_SOAK

#if defined(DEBUG_SOAK)
#warning "***** DEBUG ON *****"
#endif

UINT8			slot;
static UINT16	wSthTestId;	/* Currently executing test */
static uPacket	sPkt;
static uPacket	sReplyPacket;
static UINT8	P3_packet_Flag;

#ifdef INCLUDE_LWIP_SMC
#define IPMI_BUF_SIZE	255
#define MASK_ASCII_LEN	0x3F	/* Mask for length of FRU data ASCII string */

static char		sSthBoardName[12];
static char		sSthSerNumber[12];

extern UINT32 dIpmiReady( UINT32 dTimeout );
extern void vIpmiDeinit( void );

extern UINT32 dIpmiSendRequest(	UINT8 *pbRequest, UINT8 bReqLength,
								UINT8 *pbResponse, UINT8 *pbRespLength );
#endif


extern UINT8 bControllerType;

extern UINT8	abSlaveDev[MAX_SLOTS];
extern UINT8	bSlaves;

extern UINT32	dParams[MAX_PARAMS];
extern UINT16	wNumParams;

extern sm3vmeSlave asSlots[MAX_SLAVES];
extern UINT8	   abSlaveDev[MAX_SLOTS];
extern sm3vmesyscon syscon;

#ifdef INCLUDE_DEBUG_VGA
extern int startLine;
#endif

static char  achBuffer[80];


static void  sth_help (void);
static void  sth_execute (void);
static UINT8 sthInit (void);
static void  vProcessPackets (UINT8	bQueue);
static void  vSthClock (uPacket *sPkt);
static void  vSthExecute (uPacket *psPkt);
static void  vSthHelp (uPacket *psPkt);

#ifdef INCLUDE_LWIP_SMC
static void  vSthSetBoardInfo (void);
static void  vSthSlaveInfo (uPacket *psPkt);
static void  vSthAssignSlaveAddr (uPacket *psPkt);
#endif

extern UINT32 dDoTest (int iSupNotFound, int wTestNum, UINT32* adTestParams);

/*****************************************************************************
*  TITLE:  vmeSthInit ()
*
* Slave Test Handler initialization.
*
*
* 	RETURNS: Assigned slot number.
*****************************************************************************/
UINT8 vmeSthInit(void)
{
	UINT8	bridgePresent;

	memset (asSlots,	0x00, (sizeof(sm3vmeSlave)) * MAX_SLAVES);
	memset (abSlaveDev,	0x00, (sizeof(UINT8)) * MAX_SLAVES);
	memset (&syscon,	0x00, sizeof(sm3vmesyscon));

	initializeInterconnect();			// obtains board name for STH

	// returns zero if bridge is not present
	board_service(SERVICE__BRD_VME_BRIDGELESS_MODE, NULL, &bridgePresent);
	if (bridgePresent != 0)
	{
		// VME bridge present

#ifndef INCLUDE_LWIP_SMC
		interconnect_extended_address (1); 

		// should these be set differently for MASTER??
		interconnect_SetBit( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SUPPORT_LEVEL, IDX_SUPPORT );
		interconnect_SetBit( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SUPPORT_LEVEL, POTENTIAL_MASTER );
		interconnect_SetBit( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SUPPORT_LEVEL, BIST_LEVEL_3 );
#endif
	}

	slot = sthInit();
	interconnect_set_logical_slot_ID (slot);

#ifdef DEBUG_SOAK
	sprintf (achBuffer,"\n slot %x\n", slot);
	sysDebugWriteString (achBuffer);
#endif

#ifdef INCLUDE_LWIP_SMC
	vSthSetBoardInfo();
#endif

	return slot;
}



/*****************************************************************************
*  TITLE:  vmeSthLoop ()
*
* Slave Test Handler command loop. Does not exit.
*
*

* 	RETURNS: E__OK
*****************************************************************************/
UINT32  vmeSthLoop(void)
{
	UINT8	value, parcel_type;
	UINT32	status, inprogress, i;
	UINT8   bDev;
	UINT8   lslot;
//	UINT32	pktCount;
//	UINT32	errCount;
//	UINT32	pktRecd;
	UINT8	bridgePresent;

#ifdef DEBUG_SOAK
	UINT64  count;
#endif

#if defined (INCLUDE_DEBUG_VGA) || defined (INCLUDE_DBGLOG) || defined (DEBUG_SOAK)
	int cpuNo;
#endif

#ifdef INCLUDE_DEBUG_VGA
	int currentLine;
	UINT32 taskNum;
//	UINT32 loopCount = 0;
#endif

#if defined (INCLUDE_DEBUG_VGA) || defined (INCLUDE_DBGLOG) || defined (DEBUG_SOAK)
	cpuNo = sPortGetCurrentCPU();
#endif

#ifdef INCLUDE_DEBUG_VGA
	currentLine = atomic32Inc( (UINT32 *) &startLine );
	taskNum = uxTaskNumGet( cpuNo, NULL );
#endif

#ifdef DEBUG_SOAK
	sysDebugPrintf( "%s: started on CPU:%d, packet size: %d\n", __FUNCTION__, cpuNo, sizeof(uPacket) );
#endif
#ifdef INCLUDE_DBGLOG
	DBLOG( "%s: started on CPU:%d, packet size: %d\n", __FUNCTION__, cpuNo, sizeof(uPacket) );
#endif

//	pktCount = 0;
//	errCount = 0;
//	pktRecd = 0;

	lslot = 0;

	board_service(SERVICE__BRD_VME_BRIDGELESS_MODE, NULL, &bridgePresent);

	// VME Master, code to interact with CPCI SM3 soak master
	if ((slot == 0) || (bridgePresent == 0))
	{
		while (1)
		{
			bDev = abSlaveDev[lslot];

			/* try to receive all messages from this device */
			if (IDX_detect_message(bDev) == E__OK)
			{
#ifdef DEBUG_SOAK
				sprintf (achBuffer, "Rx slave msg from slot %d\n", bDev);
				sysDebugWriteString (achBuffer);
#endif

				processSlaveRxPackets (bDev);
			}
			else
				vDelay (1);

			/* if instructions from the Master Test Handler. */
			if (IDX_detect_message (INTERCONNECT_LOCAL_SLOT) != E_NOT_PENDING)
			{
				do
				{
					/* get waiting message */
#ifdef DEBUG_SOAK
					sysDebugWriteString ("Rx message from SM\n");
#endif

					status = wPacketReceive (INTERCONNECT_LOCAL_SLOT, &sPkt, ACK_WAIT);
					if (status == E__OK)
						wPacketAddQueue (Q_RX, &sPkt);

				} while (status == E__OK);
			}

			inprogress = 0;
			if (bridgePresent != 0)
			{
				for (i = 10; i < MAX_SLAVES-1; i++)
				{
					if ((asSlots[i].coop[COOP_LM_WR].status == 1) ||
						(asSlots[i].coop[COOP_LM_DA].status == 1))
					{
						inprogress = 1;
					}
				}
			}
			if (inprogress == 0)
				vProcessPackets(Q_RX);


			/* alert the soak master or send any queued packets */
			wPacketSendQueue (Q_TX, INTERCONNECT_LOCAL_SLOT);

			if (++lslot >= bSlaves)
				lslot = 0;
		}
	}
	else // VME SLAVE, this will keep the board compatible with old VME soak master
	{
		interconnect_ReadBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, BIST_COMPLETE, &value);

#ifdef DEBUG_SOAK
		count = 0;
#endif
		while (value != BIST_COMPLETE)
		{
#ifdef DEBUG_SOAK
			if (count == 0)
			{
				sysDebugWriteString ("r");			// test still running
				vDelay(100);
			}
			else
			{
				count++;
				if (count == 0xffffffff)
					count = 0;
			}
#endif

			status = wIDX_DetectParcel (INTERCONNECT_LOCAL_SLOT, &parcel_type);

			// Wait for instructions from the Master Test Handler.
			while( status == E__NOT_PENDING)
			{
#ifdef DEBUG_SOAK
				if(count == 0)
				{
					sysDebugWriteString ("w");			// waiting for Soak Master
					vDelay(100);
				}
				else
				{
					count++;
					if(count == 0xffffffff)
						count = 0;
				}
#endif

				// Check for the MTH wants the STH to abort.
				status = interconnect_ReadBit ( INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS,
												BIST_COMPLETE, &value );
				if( value == BIST_COMPLETE)
					break;//break out of loop

				status = wIDX_DetectParcel (INTERCONNECT_LOCAL_SLOT, &parcel_type);
			}

			if( value == BIST_COMPLETE)
				break;//break out loops

			if( parcel_type == TEST_HELP_REQUEST )
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString ("\nTEST_HELP_REQUEST\n");
#endif
				sth_help();
			}
			else if( parcel_type == EXECUTE_TEST_REQUEST )
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString ("\nEXECUTE_TEST_REQUEST\n");
#endif
				sth_execute();
			}
			else
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString ("\nBad parcel\n");
#endif

				wIDX_BadParcel(INTERCONNECT_LOCAL_SLOT, value, TRUE);
			}

			interconnect_ReadBit(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS, BIST_COMPLETE, &value);
		}

#ifdef DEBUG_SOAK
		sysDebugWriteString ("\nOut of loop\n");
#endif
	}

	return E__OK;
}



/*****************************************************************************
 *  TITLE:  sthInit ()
 *  ABSTRACT: Initialises the VME device and performs the auto ID to get the
 *  		  slot number.
 * 	RETURNS: Assigned slot number.
 *****************************************************************************/
static UINT8 sthInit(void)
{
	UINT8	slot;
	UINT8	bridgePresent;

#ifdef DEBUG_SOAK
	sysDebugPrintf ("%s\n", __FUNCTION__);
#endif

	board_service(SERVICE__BRD_VME_BRIDGELESS_MODE, NULL, &bridgePresent);
	if (bridgePresent != 0)
	{
		// VME bridge present
#ifdef INCLUDE_LWIP_SMC
		// slot always initially 0 for NETWORK_SLAVE
		// updated by USM later
		// VME bridge already initialised
		slot = 0;		// ID__MONARCH
#else
		InitialiseVMEDevice(1);
		slot = PerformVmeAutoId();
#endif
	}

#ifdef DEBUG_SOAK
	sprintf(achBuffer, "slot %x\n", slot);
	sysDebugWriteString (achBuffer);
#endif

	//If we are ID__MONARCH or bridgeless then initialise interface
	if ((slot == 0) || (bridgePresent == 0))
	{
		vCommInit ();
#ifndef INCLUDE_LWIP_SMC
		if (bridgePresent != 0)
		{
			configureCoopWindows(asSlots, syscon.syscon_coop);
		}
#endif
		wPerformAutoId();
		vFreeAllPackets();
		P3_packet_Flag = 0;
	}

	return slot;
}



/*****************************************************************************
 *  TITLE:    sth_help ()												[VME]*
 *  ABSTRACT: Initialises the help request structure, and sends the test help*
 *  		  information to the master.									 *
 * 	RETURNS:  None.															 *
 *****************************************************************************/
static void sth_help()
{
//	UINT32 rt;
	help_request_struct 	help_request;
	UINT8 max_size;
	TEST_ITEM*	psTestList;
	UINT16 length, x, counter, temp, tosend, sent;

#ifdef DEBUG_SOAK
	sysDebugPrintf ("%s\n", __FUNCTION__);
#endif

	board_service(SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

	/* Initialise the expected length of the parcel. */
	length   = 2;
	max_size = length;
	memset(&help_request,0x00, sizeof(help_request_struct));

	/* Read in the parcel and store in a buffer. */
	/*rt =*/ wIDX_ReceiveParcel (INTERCONNECT_LOCAL_SLOT, (UINT8*)&help_request, max_size, &length);

	if ((help_request.first_test_id <= help_request.last_test_id) &&
		(help_request.first_test_id > 0) &&  
		(help_request.last_test_id > 0))
	{
		length  = 0;
		counter = 0;
		tosend  = 0;
		while( (psTestList[counter].wTestNum != 0) &&
			   (psTestList[counter].wTestNum <= help_request.last_test_id))
		{
			if ((psTestList[counter].dFlags & BITF__CUTE) &&
				(psTestList[counter].wTestNum >= help_request.first_test_id))
			{
				tosend++;
				length += 4;
				temp = strlen(psTestList[counter].achTestName);
				if(temp <= 24)
					length += temp;
				else
					length += 24;
			}
			counter++;
		}

#ifdef DEBUG_SOAK
		sprintf(achBuffer,"\n length=%d\n", length );
		sysDebugWriteString (achBuffer);
#endif

		interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, TEST_HELP_RESPONSE, FALSE);
		if(tosend>0)
		{
			interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, (UINT8)length, FALSE);
			interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, (UINT8)(length>>8), FALSE);
		}
		else
		{
			interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, 0, FALSE);
			interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, 0, TRUE);
			return;
		}

		counter = 0;
		sent    = 0;
		while( (psTestList[counter].wTestNum != 0) &&
			   (psTestList[counter].wTestNum <= help_request.last_test_id))
		{
			if( (psTestList[counter].dFlags & BITF__CUTE) &&
				(psTestList[counter].wTestNum >= help_request.first_test_id))
			{
				sent++;
				interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, ((UINT8)psTestList[counter].wTestNum), FALSE);
				interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, 0x1f, FALSE);
				interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, 0x03, FALSE);

				temp = strlen(psTestList[counter].achTestName);
				if(temp > 24)
					temp = 24;

				for(x = 0; x<temp; x++)
					interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, psTestList[counter].achTestName[x], FALSE);

				if(sent == tosend)
					interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, 0, TRUE);
				else
					interconnect_SendByte (INTERCONNECT_LOCAL_SLOT, 0,  FALSE);
			}
			counter++;
		}
	}
}


/*****************************************************************************
 *  TITLE:     sth_execute												[VME]*
*  ABSTRACT:  "sth_execute" completes reading in the TEST EXECUTE REQUEST
*             PARCEL, invokes test one if test_init flag is set to 0FFH,
*             and then invokes the test via the Test Handler Service
*             "IDX_test_execute".  After a test has completed or the init
*             test failed, sth_execute sends a TEST EXECUTE RESPONSE PARCEL,
*             and returns to "sth".
 *****************************************************************************/
static void sth_execute()
{
//	UINT32 rt;
	execute_request_struct 	execute_request;
	execute_response_struct execute_response;
	UINT32	test_status;
	UINT16   max_size, length;

#ifdef DEBUG_SOAK
	sysDebugPrintf ("%s\n", __FUNCTION__);
#endif

	length = 5;
	max_size = length;

	memset(&execute_request,0x00, sizeof(execute_request_struct));

	/* Read in the parcel and store in a buffer. */
	/*rt =*/ wIDX_ReceiveParcel (INTERCONNECT_LOCAL_SLOT, (UINT8*)&execute_request, max_size, &length);

	//test_status = IDX_test_execute (IDX_SLAVE, execute_request.test_id);
	if(wNumParams > 0 )
		test_status = dDoTest (	0,	wSthTestId, &dParams[0] );
	else
		test_status = dDoTest (	0,	wSthTestId, NULL );

#ifdef DEBUG_SOAK
	sprintf(achBuffer,"Test ID=%x, Result %x\n", execute_request.test_id, test_status );
	sysDebugWriteString (achBuffer);
#endif

	/* Store the response parcel in the parcel buffer. */
	execute_response.parcel_type = EXECUTE_TEST_RESPONSE;
	execute_response.test_id     = execute_request.test_id;
	execute_response.test_status = test_status;

	if(getErrorMsg(achBuffer) == 1)
	{
		wIDX_Pwrite (NULL, achBuffer, NULL);
	}

	/* Send an execute test request response parcel. */
	/*rt =*/ wIDX_SendParcel( INTERCONNECT_LOCAL_SLOT, (UINT8*)&execute_response, 4 );
}



/*****************************************************************************
 *  TITLE:  vSthClock ()												[SM3]*
 *****************************************************************************/
static void vSthClock( uPacket *sPkt )
	{
    /* just Ack the packet for now */
#ifdef DEBUG_SOAK
	sysDebugWriteString ("vSthClock()\n");
#endif
}


/*******************************************************************************
*
* vProcessPackets
*
* Process packets in the given queue.
*
*
* RETURNS: None.
* 
*******************************************************************************/
static void vProcessPackets (UINT8	bQueue)
{
	uPacket* psPkt;
	UINT8	 bSlot;

#ifdef DEBUG_SOAK
	static UINT32 pCount = 0;

	if(pCount > 1000)
	{
		pCount = 0;
		sysDebugWriteString( "p" );				// processing packets
	}
	else
		pCount++;
#endif

	while ((psPkt = psPacketGetQueue(bQueue)) != NULL)
	{
#ifdef DEBUG_SOAK
		sprintf (achBuffer, "board slot number %x, destination slot %x\n", slot, psPkt->hdr.wDestID);
		sysDebugWriteString (achBuffer);
#endif
		/* check packet address and route to slave if not for us */
#ifdef INCLUDE_LWIP_SMC
		if (!((psPkt->hdr.wDestID == INTERCONNECT_LOCAL_SLOT) || (psPkt->hdr.wDestID == slot)))
#else
		if (psPkt->hdr.wDestID != slot)
#endif
		{
#ifdef DEBUG_SOAK
			sprintf (achBuffer, "Not for master, slot %x\n", psPkt->hdr.wDestID);
			sysDebugWriteString (achBuffer);
#endif

			bSlot = bAddrToSlot (psPkt->hdr.wDestID);
			if (bSlot != 0xff)
			{
			   processSlavePackets( bSlot, psPkt );
			}
			continue;
		}

#ifdef DEBUG_SOAK
		sprintf (achBuffer, "packet header flags %04X\n", psPkt->hdr.wFlags);
		sysDebugWriteString (achBuffer);
		sprintf (achBuffer, "packet header type %08X\n", psPkt->hdr.dType);
		sysDebugWriteString (achBuffer);
#endif

		/* check packet flags */
		if (psPkt->hdr.wFlags & FLAG_TST_ENT)
		{
			vSthExecute (psPkt);
		}
		else if (psPkt->hdr.wFlags & FLAG_CLK_SET)
		{
			vSthClock (psPkt);
		}
		else
		{
			switch (psPkt->hdr.dType)
			{
				case id_RBL:				/* request test list    */
					vSthHelp (psPkt);
					break;

				case id_BGO:				/* execute test         */
					vSthExecute (psPkt);
					break;

				case id_SCT:				/* receive command tail	*/
					break;

#ifdef INCLUDE_LWIP_SMC
				case id_AA_SL:				/* request slave information */
					vSthSlaveInfo( psPkt );
					break;

				case id_AA_PORT:			/* assign slave address */
					vSthAssignSlaveAddr( psPkt );
					break;
#endif

				default:
#ifdef DEBUG_SOAK
					sysDebugWriteString("Unknown packet\n");
#endif
					break;
			}
		}
	} /* while */
}



/*******************************************************************************
*
* vSthHelp
*
* Creates response to request test list.
*
*
* RETURNS: None.
* 
*******************************************************************************/
static void vSthHelp (uPacket *psPkt)
{
	int 	   i;
	UINT8	   bTmp;
	UINT8	   quit;
	TEST_ITEM* psTestList;
	uPacket*   psBistList;
	UINT8	   test_id;
	UINT8	   *test_entry;

#ifdef DEBUG_SOAK
	sysDebugWriteString ("vSthHelp()\n");
#else
	DBLOG ("%s: Request test list\n", __FUNCTION__);
#endif

	memset (&sReplyPacket, 0, sizeof(sReplyPacket));

	board_service (SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

	psBistList  = &sReplyPacket;
	test_entry  = (UINT8 *)&psBistList->hdr.dType;
	test_entry += sizeof (psBistList->hdr.dType);

	for (i=0; i < 10; i++)
	{
		interconnect_ReadByte (INTERCONNECT_LOCAL_SLOT, 2 + i, &bTmp);
		*test_entry++ = bTmp;
	}

	test_id = 0;
	quit = FALSE;

	/* Parse test help for length of the test help menu. */
	while (!quit)
	{
		/* If the help information is valid, copy the id and name to
			the bist list packet */
        if ((psTestList[test_id].wTestNum != 0)     &&
			 (psTestList[test_id].wTestNum < 0xff)  &&
			 (psTestList[test_id].dFlags & BITF__CUTE))
		{
        	if (psTestList[test_id].wTestNum == 127)
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString ("SysCon test 127 SKIPPED\n");
#endif
				/* If the maximum test_id is reached, quit counting. */
				if (psTestList[test_id].wTestNum == 0x00)
					quit = TRUE;
				else						/* Else continue counting. */
				{
					test_id = test_id + 1;
				}
				continue;
			}

			*((UINT16*)test_entry) = psTestList[test_id].wTestNum;
			test_entry+=2;
			if (psTestList[test_id].dFlags & BITF__COOP)
			{
				*((UINT16*)test_entry) = 0x85aa;
			}
			else
			{
				*((UINT16*)test_entry) = 0x331E;
			}

			test_entry += 2;

			memcpy (test_entry, &psTestList[test_id].achTestName[0], strlen(psTestList[test_id].achTestName));

			test_entry   += strlen (psTestList[test_id].achTestName);
			*test_entry++ = '\0';
		}

		/* If the maximum test_id is reached, quit counting. */
		if (psTestList[test_id].wTestNum == 0x00)
			quit = TRUE;
		else						/* Else continue counting. */
			test_id = test_id + 1;
	}

#ifdef DEBUG_SOAK
	*((UINT16*)test_entry) = 310;
	test_entry += 2;

	*((UINT16*)test_entry) = 0x331E;
	test_entry += 2;

	memcpy (test_entry, "Scan Back Plane", strlen("Scan Back Plane"));

	test_entry += strlen ("Scan Back Plane");
	*test_entry++ = '\0';
#endif

	*test_entry = '\0';

	/* Send the bist list packet. */
	psBistList->hdr.wSourceID   = slot;
	psBistList->hdr.wDestID   = SOAK_ID;
	psBistList->hdr.wFlags    = 0xa0;
	psBistList->hdr.wLength   = test_entry - (UINT8 *)psBistList + 1;
	psBistList->hdr.dType     = (UINT32)id_SBL;

	wPacketAddQueue (Q_TX, psBistList);
}



/*****************************************************************************\
*
*  TITLE:  vSthExecute
*
*  ABSTRACT:  "vSthExecute" completes reading in the TEST EXECUTE REQUEST
*             PARCEL, invokes test one if test_init flag is set to 0FFH,
*             and then invokes the test via the Test Handler Service
*             "CPCI_test_execute".  After a test has completed or the init
*             test failed, vSthExecute sends a TEST EXECUTE RESPONSE PARCEL,
*             and returns to "sth".
*
\*****************************************************************************/
static void vSthExecute (uPacket *psPkt)
{
	UINT32	test_status = E__OK/*, rt*/;
	char*	ERROR_TEXT = "FAIL";
	char	buffer[64];

#ifdef DEBUG_SOAK
	sysDebugWriteString ("vSthExecute()\n");
#else
	DBLOG ("%s: Test: %d\n", __FUNCTION__, psPkt->sBgo.wTestId);
#endif

	wSthTestId = psPkt->sBgo.wTestId;

	/* Inform agents this is BIST executing by clearing the Initialisation Check Bit. */
	interconnect_ClearBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, INTERCONNECT_INITIALIZATION_CHECK);

 	/* Clear the BIST Abort Bit in the BIST Slave Status Register. */
	interconnect_ClearBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, INTERCONNECT_BIST_ABORT);

	/* Clear the pass/fail status in interconnect to pass. */
	interconnect_ClearBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, INTERCONNECT_BIST_PASS_FAIL_STATUS);

	/* Put the current test id in the BIST Test ID register. */
	interconnect_WriteByte (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_TEST_ID, (UINT8)wSthTestId);

	/* Flag interconnect, test execution has started. */
	interconnect_ClearBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, INTERCONNECT_BIST_RUNNING);

	memset (syscon.errMsg, 0x00, 64);
	/*rt = */vSaveCommandTail ( SysCon_ID, &psPkt->bData[12]);

	vParseCommandTail (&psPkt->bData[12]);
#ifdef DEBUG_SOAK
	sprintf (achBuffer, "MAX_PARAM:%d wNumParams: %d\n", MAX_PARAMS, wNumParams);
	sysDebugWriteString (achBuffer);
#endif
//	if (wSthTestId == 310)
//	{
//		test_status = reScanBackPlane();
//	}
//	else
//	{
		test_status = dDoTest (0, wSthTestId, &dParams[0]); //Hari was NULL
//	}

	memset(dParams, 0x00, (sizeof(UINT32) * MAX_PARAMS) );

#ifdef DEBUG_SOAK
	sprintf (achBuffer, "wSthTestId %d, result %x\n", wSthTestId, test_status);
	sysDebugWriteString (achBuffer);
	sprintf (achBuffer, "dParams[0] %d, dParams[1] %x\n", dParams[0], dParams[1]);
	sysDebugWriteString (achBuffer);
#endif



	/* Initialise the input buffer. */
	memset (&sReplyPacket, 0, sizeof(sReplyPacket));

	sReplyPacket.hdr.wSourceID   = slot;
	sReplyPacket.hdr.wDestID   = SOAK_ID;

	if (test_status == E__OK)
	{
		if (bControllerType == NET_SLAVE)	/* Network slave mode */
		{
#ifdef DEBUG_SOAK
			sprintf (achBuffer, "Sending on Network\n");
			sysDebugWriteString (achBuffer);
#endif
			sReplyPacket.hdr.wFlags  = 0x8A0;
			sReplyPacket.wData[5]    = wSthTestId;
			sReplyPacket.wData[6]    = (UINT16) (test_status & 0xffff);
			sReplyPacket.hdr.wLength = 14;
		}
		else
		{
#ifdef DEBUG_SOAK
			sprintf (achBuffer, "Sending on Serial Port\n");
			sysDebugWriteString (achBuffer);
#endif
			sReplyPacket.hdr.wFlags  = 0x820;
			sReplyPacket.hdr.wLength = 10;
		}
	}
	else
	{
		if (bControllerType == NET_SLAVE)	/* Network slave mode */
		{
#ifdef DEBUG_SOAK
			sprintf (achBuffer, "Sending on Network\n");
			sysDebugWriteString (achBuffer);
#endif
			sReplyPacket.hdr.wFlags  = 0x8A4;
			sReplyPacket.wData[5]    = wSthTestId;
			sReplyPacket.wData[6]    = (UINT16) (0x8000 + (test_status & 0xffff));
			
			strcpy ((char *)&sReplyPacket.bData[14], ERROR_TEXT);
			sReplyPacket.hdr.wLength = 15 + strlen (ERROR_TEXT) + 1;
			interconnect_SetBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, BIST_FAIL);
		}
		else
		{
#ifdef DEBUG_SOAK
			sprintf (achBuffer, "Sending on Serial Port\n");
			sysDebugWriteString (achBuffer);
#endif
			sReplyPacket.hdr.wFlags  = 0x8A4;
			sReplyPacket.wData[5]    = 0x8000 + test_status;

			if( getErrorMsg(buffer) == 1 )
			{
				strcpy ((char *)&sReplyPacket.bData[ 12 ], (char *)buffer);
				sReplyPacket.hdr.wLength = 13 + strlen ((char *)buffer) + 1;
			}
			else
			{
				strcpy ((char *)&sReplyPacket.bData[ 12 ], ERROR_TEXT);
				sReplyPacket.hdr.wLength = 13 + strlen (ERROR_TEXT) + 1;
			}
			interconnect_SetBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, BIST_FAIL);
		}
	}

	/* Flag interconnect, test execution has completed. */
	interconnect_ClearBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, INTERCONNECT_BIST_RUNNING);
#ifdef DEBUG_SOAK
	sprintf(achBuffer, "P3_packet_Flag: %x\n", P3_packet_Flag);
	sysDebugWriteString (achBuffer);
#endif
	if (P3_packet_Flag == 0)
	{
#ifdef DEBUG_SOAK
		sprintf(achBuffer, "Adding packet wPacketAddQueue()t\n");
		sysDebugWriteString (achBuffer);
#endif
		wPacketAddQueue (Q_TX, &sReplyPacket);
	}
	else
	{
		P3_packet_Flag = 0;
	}
}


/*******************************************************************************
*
* IDX_p3_bist_exit
*
* Cooperative RAM test exit function.
* Note: This bypasses the normal response generated by vSthExecute()!!
*       Bad idea, should be replaced with a more consistent solution.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void IDX_p3_bist_exit (UINT16 wErrCode, char* pchTxt)
{
	UINT16 wLen;
	char   *pchTmp;

	/* flush text output */
	P3_packet_Flag = 1;

    memset (&sReplyPacket, 0, sizeof(sReplyPacket));

	sReplyPacket.hdr.wSourceID   = slot;
    sReplyPacket.hdr.wDestID     = SOAK_ID;
    if (wErrCode == E__OK)
    {
 		if (bControllerType == NET_SLAVE)
		{
			sReplyPacket.hdr.wFlags  = 0x8A0;
			sReplyPacket.wData[5]    = wSthTestId;
			sReplyPacket.wData[6]    = wErrCode == 1 ? (UINT16) E__OK : wErrCode;
			sReplyPacket.hdr.wLength = 14;
		}
		else
		{
    		sReplyPacket.hdr.wFlags  = 0x820;
    		sReplyPacket.hdr.wLength = 10;
    	}
    }
    else
    {
    	if (bControllerType == NET_SLAVE)
		{
			sReplyPacket.hdr.wFlags  = 0x8A4;
			sReplyPacket.wData[5]    = wSthTestId;
			sReplyPacket.wData[6]    = wErrCode == 1 ? (UINT16) E__OK : wErrCode;
			
			if (pchTxt != NULL)
			{
				strcpy ((char *)&sReplyPacket.bData[14], pchTxt);
				sReplyPacket.hdr.wLength = 15 + strlen (pchTxt) + 1;
			}
			else
			{
				sReplyPacket.hdr.wLength = 14;
			}
		}
		else
		{
 	       sReplyPacket.hdr.wFlags  = 0x8A4;
 	       sReplyPacket.wData[5]    = wErrCode == 1 ? (UINT16) E__OK : wErrCode;
			if (pchTxt != NULL)
			{
				wLen = strlen (pchTxt) + 1;
				pchTmp = (char *)&sReplyPacket.bData[12];
				while (*pchTxt != '\0')
				{
					*pchTmp++ = *pchTxt++;
				}
				*pchTmp++ = '\0';
			}
			else
			{
				wLen = 1;
			}

        	sReplyPacket.hdr.wLength = 13 + wLen;
		}
	}

	wPacketAddQueue (Q_TX, &sReplyPacket);
}


#ifdef INCLUDE_LWIP_SMC

/*******************************************************************************
*
* sthIpmiReadFruData
*
* Read data from the FRU from the given offset.
*
*
* RETURNS: E__OK or an error code.
* 
*******************************************************************************/
static UINT32 sthIpmiReadFruData( UINT8 fruDevId, UINT16 offset, UINT8 readCount, 
							UINT8 *pRespData, UINT8 *pRespDataLen )
{
	int i;
	UINT8 len;
	UINT8 maxLen;
	UINT32 status;
	UINT8 reqBuf[6];
	UINT8 respBuf[20]; /* buffer must be at least 16 + 4 max. */ 


	/* Setup FRU data request */
	reqBuf[0] = (NFC_STORAGE_REQUEST << 2) |(LUN_BMC & 0x03);
	reqBuf[1] = 0x11; 				/* Get FRU Data */
	reqBuf[2] = fruDevId;
	
	maxLen = *pRespDataLen;
	*pRespDataLen = 0;
	
	// DBLOG( "%s: offset: 0x%x  readCount: %u\n", __FUNCTION__, offset, readCount );	

	do
	{
		/* Setup the offset and length */
		reqBuf[3] = offset & 0x00FF;	/* Inventory offset LSB */
		reqBuf[4] = offset >> 8;		/* Inventory offset MSB */
		
		if (readCount > 16)
		{
			reqBuf[5] = 16;
		}
		else
		{
			reqBuf[5] = readCount;
		}

		/* Read FRU data */
		status = dIpmiSendRequest( reqBuf, 6, respBuf, &len );

		if (status == E__OK)
		{
			// DBLOG( "%s: reqlen: %2u resplen: %2u\n", __FUNCTION__, reqBuf[5], respBuf[3] );
			
			/* Check data length is as expected */				
			if ((len > 4) && (respBuf[3] == reqBuf[5]))
			{	
				/* Copy data from buffer excluding the first byte, */
				/* which is the length and not included */
				len -= 4;
				
				for (i = 0; i < len; i++)
				{
					pRespData[*pRespDataLen + i] = respBuf[i+4];
				}
				
				*pRespDataLen += len;
				
				offset += len;
				readCount -= len;
			}
			else
			{
				status = E__FAIL;
			}						
		}
	}
	while ((status == E__OK) && (readCount > 0) && (*pRespDataLen < maxLen));
	
	if (status != E__OK)
	{
#ifdef DEBUG_SOAK
		sysDebugPrintf ("%s: failed, status: 0x%x\n", __FUNCTION__, status);
#else
		DBLOG("%s: failed, status: 0x%x\n", __FUNCTION__, status);
#endif
	}
	
	return status;
}


/*******************************************************************************
*
* sthIpmiGetBoardInfo
*
* Process the board information data in the given buffer.
*
*
* RETURNS: None.
* 
*******************************************************************************/
static void sthIpmiGetBoardInfo( UINT8 *pData )
{
	UINT8 offset;
	UINT8 endPos;
	UINT8 i, j;
	
	
	offset = 6 + (pData[6]  & MASK_ASCII_LEN) + 1;
	endPos = offset + (pData[offset]  & MASK_ASCII_LEN);
	
	/* Get the board name */
	for (i = offset + 1, j = 0; i <= endPos; i++, j++)
	{
		sSthBoardName[j] = ( (pData[i] > 31) && (pData[i] < 127) ) ? pData[i] : '?';
	}
	
	sSthBoardName[j] = '\0';
	
#ifdef DEBUG_SOAK
	sysDebugPrintf ("%s: Board name   : %s\n", __FUNCTION__, sSthBoardName);
#else
	DBLOG("%s: Board name   : %s\n", __FUNCTION__, sSthBoardName);
#endif 

	offset = endPos + 1;
	endPos = offset + (pData[offset]  & MASK_ASCII_LEN);
	
	/* Get the serial number */
	for (i = offset + 1, j = 0; i <= endPos; i++, j++)
	{
		sSthSerNumber[j] = ( (pData[i] > 31) && (pData[i] < 127) ) ? pData[i] : '?';
	}
	
	sSthSerNumber[j] = '\0';
	
#ifdef DEBUG_SOAK
	sysDebugPrintf ("%s: Serial number: %s\n", __FUNCTION__, sSthSerNumber);
#else
	DBLOG("%s: Serial number: %s\n", __FUNCTION__, sSthSerNumber);
#endif
}


/*******************************************************************************
*
* sthIpmiBoardInfo
*
* Attempt to read the board information from the IPMI FRU.
*
*
* RETURNS: E__OK or an error code.
* 
*******************************************************************************/
static UINT32 sthIpmiBoardInfo( void )
{
	UINT8 i;
	UINT8 respBuf[IPMI_BUF_SIZE];
	UINT8 bRsLen;
	UINT32 status;
	UINT8 chkSum;
	UINT16 offset;
	UINT16 count;
	
	status = E__NO_IPMI;
	return status;

	status = dIpmiReady( 1000 );
	
	if (status == E__OK)
	{
		memset( respBuf, 0, IPMI_BUF_SIZE );
		bRsLen = 8;
		
		status = sthIpmiReadFruData( 0, 0, bRsLen, respBuf, &bRsLen );
		
		if (status == E__OK)
		{	
			/* Check Common Header Checksum */
			for (i = 0, chkSum = 0; i < 8; i++)
			{
				chkSum += respBuf[i];
			}

			if (chkSum != 0x00)
			{
#ifdef DEBUG_SOAK
				sysDebugPrintf ("%s: Error: common header checksum failure\n", __FUNCTION__);
#else
				DBLOG("%s: Error: common header checksum failure\n", __FUNCTION__);
#endif
				status = E__FAIL;
			}
			else if (respBuf[3] == 0 )
			{
#ifdef DEBUG_SOAK
				sysDebugPrintf ("%s: Error: invalid board area offset: %u\n", __FUNCTION__, respBuf[3]);
#else
				DBLOG("%s: Error: invalid board area offset: %u\n", __FUNCTION__, respBuf[3]);
#endif
				status = E__FAIL;
			}
			else
			{
				/* Get Board Area offset */
				offset = respBuf[3] * 8;
				
				// DBLOG ("Format version   : %u\n", (respBuf[0] & 0x0F));
				// DBLOG( "Board area offset: %u\n", offset );
				
				memset( respBuf, 0, IPMI_BUF_SIZE );
				bRsLen = 2;
				
				status = sthIpmiReadFruData( 0, offset, bRsLen, respBuf, &bRsLen );
				
				if (status == E__OK)
				{
					count = respBuf[1] * 8;
					
					// DBLOG( "Board area size: %u\n", count );
					
					if (count < IPMI_BUF_SIZE)
					{ 
						memset( respBuf, 0, IPMI_BUF_SIZE );
						bRsLen = (UINT8) count;
						
						/* Read the Board Info Area */
						status = sthIpmiReadFruData( 0, offset, bRsLen, respBuf, &bRsLen );
						
						if (status == E__OK)
						{
							/* Check Board Info Area checksum */
							for (i = 0, chkSum = 0; i < bRsLen; i++)
							{
								chkSum += respBuf [i];
							}
				
							if (chkSum != 0)
							{
#ifdef DEBUG_SOAK
								sysDebugPrintf ("%s: Error: board info area checksum failure\n", __FUNCTION__);
#else
								DBLOG("%s: Error: board info area checksum failure\n", __FUNCTION__);
#endif
								status = E__FAIL;
							}
							else
							{
								sthIpmiGetBoardInfo( respBuf );
							}
						}
					}
					else
					{
#ifdef DEBUG_SOAK
						sysDebugPrintf ("%s: Error: board info area too large\n", __FUNCTION__);
#else
						DBLOG("%s: Error: board info area too large\n", __FUNCTION__);
#endif
						status = E__FAIL;
					}
				}
			}
		}
		
		// vIpmiDeinit();
	}
	else
	{
#ifdef DEBUG_SOAK
		sysDebugPrintf ("%s: Error: IPMI initialization failed\n", __FUNCTION__);
#else
		DBLOG("%s: Error: IPMI initialization failed\n", __FUNCTION__);
#endif
		}

	return status;
}


/*******************************************************************************
*
* vSthSetBoardInfo
*
* Setup the board information.
*
*
* RETURNS: None.
* 
*******************************************************************************/
static void vSthSetBoardInfo( void )
{
	int i, j;
	UINT8 bTmp;

	if (sthIpmiBoardInfo() != E__OK)
	{
		/* Get the board name from interconnect space */
		j = 0;
		
		for (i = 0; i < 9; i++)
		{
			interconnect_ReadByte( INTERCONNECT_LOCAL_SLOT, 2 + i, &bTmp );
			
			if ( i == 2 )
			{
				/* Add a space so names the same as IPMI */
				sSthBoardName[j++] = ' ';
			}
			
			sSthBoardName[j++] = ( (bTmp > 31) && (bTmp < 127) ) ? bTmp : '?';
		}
		
		sSthBoardName[j] = '\0';

		if (board_service(SERVICE__BRD_GET_SERIAL_NUMBER, NULL, &sSthSerNumber) != E__OK)
		{
			/* Unknown serial number */
			strcpy (sSthSerNumber, "?????\?/???");
		}
		sSthSerNumber[11] = '\0';
		//sysDebugPrintf ("%s: Serial number: %s\n", __FUNCTION__, sSthSerNumber);

	}

#ifdef DEBUG_SOAK
	sysDebugPrintf ("%s: Board name   : %s\n", __FUNCTION__, sSthBoardName);
	sysDebugPrintf ("%s: Serial number: %s\n", __FUNCTION__, sSthSerNumber);
#else
	DBLOG("%s: Board name   : %s\n", __FUNCTION__, sSthBoardName);
	DBLOG("%s: Serial number: %s\n", __FUNCTION__, sSthSerNumber);
#endif
}


/*******************************************************************************
*
* vSthSlaveInfo
*
* Creates response to request slave information.
*
*
* RETURNS: None.
* 
*******************************************************************************/
static void vSthSlaveInfo( uPacket *psPkt )
{
	int i;
	UINT16 slotId;


	memset (&sReplyPacket, 0, sizeof(sReplyPacket));

	if (board_service(SERVICE__BRD_GET_SLOT_ID, NULL, &slotId) == E__OK)
	{
#ifdef DEBUG_SOAK
		sysDebugPrintf("%s: slotId: 0x%04x\n", __FUNCTION__, slotId);
#else
		DBLOG("%s: slotId: 0x%04x\n", __FUNCTION__, slotId);
#endif

		sReplyPacket.hdr.wSourceID = slotId;

		// update our address
		slot = slotId;
	}
	else
	{
#ifdef DEBUG_SOAK
		sysDebugPrintf("%s: BRD_GET_SLOT_ID failed, slotId: 0x%04x\n", __FUNCTION__, slotId);
#else
		DBLOG("%s: BRD_GET_SLOT_ID failed, slotId: 0x%04x\n", __FUNCTION__, slotId);
#endif
		/* Indicate to SM client that Soak Master needs to assign board ID */
		sReplyPacket.hdr.wSourceID = 0xffff;
	}
	
	sReplyPacket.hdr.wDestID = psPkt->hdr.wSourceID;
	sReplyPacket.hdr.wFlags = (FLAG_HAS_DTA | FLAG_SYS_PKT | FLAG_PKT_PRI);
	sReplyPacket.hdr.dType = (UINT32) id_AA_SL;	/* 3 Slave Address Allocation Packet */
	
	sReplyPacket.bData[14] = 1;	/* number of slaves, 1 as we're the only board */
	sReplyPacket.hdr.wLength = 15;
	
	/* Copy the board name */
	for (i = 0; i < 11; i++)
	{
		sReplyPacket.bData[15 + i] = sSthBoardName[i];
		sReplyPacket.hdr.wLength++;		
	}

	/* Copy the serial number */
	for (i = 0; i < 11; i++)
	{
		sReplyPacket.bData[26 + i] = sSthSerNumber[i];
		sReplyPacket.hdr.wLength++;		
	}
	
	wPacketAddQueue (Q_TX, &sReplyPacket);
}


/*******************************************************************************
*
* vSthAssignSlaveAddr
*
* Creates response to assign slave address.
*
*
* RETURNS: None.
* 
*******************************************************************************/
static void vSthAssignSlaveAddr( uPacket *psPkt )
{
	UINT16 i;


	memset (&sReplyPacket, 0, sizeof(sReplyPacket));
	
	/* Echo the packet back to the SM Client so it can */
	/* update it's board address and send response to SM */
	sReplyPacket.hdr.wLength = 14;
	sReplyPacket.hdr.wSourceID = psPkt->hdr.wDestID;
	sReplyPacket.hdr.wDestID = psPkt->hdr.wSourceID;
	sReplyPacket.hdr.wFlags = (FLAG_HAS_DTA | FLAG_SYS_PKT | FLAG_PKT_PRI);
	sReplyPacket.hdr.dType = (UINT32) id_AA_PORT;	/* 4 Soak Master Address Allocation Packet */
	
	sReplyPacket.wData[7] = psPkt->wData[7];
	sReplyPacket.hdr.wLength += 2;
	
	for (i = 0; i < sReplyPacket.wData[7]; i++)
	{
		if (i == 0)
		{
			/* Update our address */
			slot = psPkt->wData[8 + i];

#ifdef DEBUG_SOAK
			sysDebugPrintf("%s: update slot: 0x%04x\n", __FUNCTION__, slot);
#else
			DBLOG("%s: update slot: 0x%04x\n", __FUNCTION__, slot);
#endif
			}
		
		sReplyPacket.wData[8 + i] = psPkt->wData[8 + i];
		sReplyPacket.hdr.wLength += 2;		
	}
	
	wPacketAddQueue (Q_TX, &sReplyPacket);
}

#endif /* INCLUDE_LWIP_SMC */

