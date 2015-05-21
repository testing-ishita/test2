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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/sth/sthVpx.c,v 1.6 2014-09-19 09:37:34 mgostling Exp $
 * $Log: sthVpx.c,v $
 * Revision 1.6  2014-09-19 09:37:34  mgostling
 * Fixed compiler warning
 *
 * Revision 1.5  2014-07-24 16:08:44  mgostling
 * Updated debugging conditionals to resolve compiler warnings.
 *
 * Revision 1.4  2014-07-11 10:43:44  mgostling
 * Fix compiler warnings.
 *
 * Revision 1.3  2014-07-11 08:51:13  mgostling
 * Add support for ethernet Soak Master interface
 *
 * Revision 1.2  2013-11-25 11:26:22  mgostling
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013-09-04 07:45:36  chippisley
 * Import files into new source repository.
 *
 * Revision 1.6  2011/05/16 14:31:59  hmuneer
 * Info Passing Support
 *
 * Revision 1.5  2011/03/22 13:47:45  hchalla
 * *** empty log message ***
 *
 * Revision 1.4  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.3  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.2  2010/11/04 17:46:27  hchalla
 * Added Debug info.
 *
 * Revision 1.1  2010/09/15 12:23:26  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.2  2010/06/24 13:28:23  hchalla
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

static UINT8 	slot;
static UINT16	wSthTestId;	/* Currently executing test */
static uPacket	sPkt;
static uPacket	sReplyPacket;
static UINT8	P3_packet_Flag;
UINT16 	my_slot;


#ifdef INCLUDE_LWIP_SMC

#define IPMI_BUF_SIZE	255
#define MASK_ASCII_LEN	0x3F	/* Mask for length of FRU data ASCII string */

static char sSthBoardName[12];
static char sSthSerNumber[12];

extern UINT32 dIpmiReady( UINT32 dTimeout );
extern void vIpmiDeinit( void );

extern UINT32 dIpmiSendRequest(	UINT8 *pbRequest, UINT8 bReqLength,
								UINT8 *pbResponse, UINT8 *pbRespLength );
#endif


extern UINT8 bControllerType;

extern UINT8	abSlaveDev[MAX_SLOTS];
extern UINT8	bSlaves;

extern sSlave		asSlots[MAX_SLAVES];
extern sSlaveMemBar	asSlotsM[MAX_SLAVES];

extern UINT32	dParams[MAX_PARAMS];
extern UINT16	wNumParams;

#ifdef INCLUDE_DEBUG_VGA
extern int startLine;
#endif

static void  vProcessPackets (UINT8	bQueue);
static UINT8 bAddrToSlot (UINT16 wAddr);
static void  vSthClock (uPacket *sPkt);
static void  vSthExecute (uPacket *psPkt);
static void  vSthHelp (uPacket *psPkt);

#ifdef INCLUDE_LWIP_SMC
static void vSthSetBoardInfo( void );
static void vSthSlaveInfo( uPacket *psPkt );
static void vSthAssignSlaveAddr( uPacket *psPkt );
#endif

#undef DEBUG_SOAK

#if defined(DEBUG_SOAK)
#warning "***** DEBUG ON *****"
#endif


/*******************************************************************************
*
* vpxSthInit
*
* Slave Test Handler initialization.
*
*
* RETURNS: our slot number.
* 
*******************************************************************************/
UINT16 vpxSthInit(void)
{
#ifdef DEBUG_SOAK
	char buff[64];
#endif

	initializeInterconnect ();
	interconnect_extended_address (1);

	/* Post the BIST support level. */
	interconnect_SetBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SUPPORT_LEVEL, BIST_LEVEL_3);

	vCommInit ();

	slot = 0;
	vFreeAllPackets();

	my_slot = wPerformAutoId();

#ifdef DEBUG_SOAK
	sprintf(buff, "my_slot: 0x%x\n", my_slot);
	sysDebugWriteString (buff);
#else
	DBLOG( "%s: my_slot: 0x%04x\n", __FUNCTION__, my_slot );
#endif


#ifdef INCLUDE_LWIP_SMC
	vSthSetBoardInfo();
#endif

	P3_packet_Flag = 0;

	return my_slot;
}


/*******************************************************************************
*
* vpxSthLoop
*
* Slave Test Handler command loop. Does not exit.
*
*
* RETURNS: E__OK.
* 
*******************************************************************************/
UINT32 vpxSthLoop(void)
{
	UINT8 bDev;
	UINT32	status;
	UINT32 pktCount;
	UINT32 errCount;
	UINT32 pktRecd;

#if defined (INCLUDE_DEBUG_VGA) || defined (INCLUDE_DBGLOG) || defined (DEBUG_SOAK)
	int cpuNo;
#endif

#ifdef INCLUDE_DEBUG_VGA 
	int currentLine;
	UINT32 taskNum;
	UINT32 loopCount = 0;
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

	pktCount = 0;
	errCount = 0;
	pktRecd = 0;

	while (1)
	{
		bDev = abSlaveDev[slot];

		/* try to receive all messages from this device */
		if (IDX_detect_message (bDev) == E__OK)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString ("Rx slave messages\n");
#endif
			do
			{
				/* Get slave board message */
				status = wPacketReceive (bDev, &sPkt, ACK_WAIT);
				
				if (status == E__OK)
				{
					wPacketAddQueue (Q_TX, &sPkt);
					pktRecd++;
				}
			} 
			while (status == E__OK);
		}

		/* if message waiting from the Soak Master  */
		if  (IDX_detect_message (INTERCONNECT_LOCAL_SLOT) !=  E_NOT_PENDING)
		{
			do
			{
				/* Get Soak Master message */
				status = wPacketReceive (INTERCONNECT_LOCAL_SLOT, &sPkt, ACK_WAIT);
				
				if (status == E__OK)
				{
#ifdef DEBUG_SOAK
					sysDebugPrintf("%s: Rx SM message: %u\n", __FUNCTION__, pktRecd);
#endif
					wPacketAddQueue (Q_RX, &sPkt);
					pktRecd++;
				}
				else
				{
					if (status != E__TIMEOUT)
					{
						errCount++;
					}
				}
			} 
			while (status == E__OK);

			vProcessPackets (Q_RX);
		}

		if (pktRecd > 0)
		{
			pktCount += pktRecd;
			/* alert the Soak Master or send any queued packets */
			wPacketSendQueue(Q_TX, INTERCONNECT_LOCAL_SLOT);
			pktRecd = 0;
		}
		else
		{
			vTaskDelay( 500 );
		}

#ifdef INCLUDE_DEBUG_VGA
		loopCount++;
				
		vgaPrinfXY( 1, currentLine, "CPU:%d T%02u: sth    Count: %u Slot: %u PKT: %u ERR: %u", 
					cpuNo, taskNum, loopCount, slot, pktCount, errCount );
#endif

//		if (++slot >= bSlaves)
		if (slot++ >= bSlaves)
		{
			slot = 0;
		}
	}

	return E__OK;
}


/*******************************************************************************
*
* vSthClock
*
* Does nothing useful!.
*
*
* RETURNS: None.
* 
*******************************************************************************/
static void vSthClock (uPacket *sPkt)
{
    /* just ack the packet for now */
#ifdef DEBUG_SOAK
	sysDebugWriteString ("vSthClock()\n");
#endif
}


/*******************************************************************************
*
* bAddrToSlot
*
* Searches for the given slave address and returns the matching slave number (slot). 
*
*
* RETURNS: slave number if found else MAX_SLAVES.
* 
*******************************************************************************/
static UINT8 bAddrToSlot (UINT16 wAddr)
{
	UINT8 n;

#ifdef DEBUG_SOAK
	sysDebugWriteString ("bAddrToSlot()\n");
#endif

	for (n = 0; n < MAX_SLAVES; n++)
	{
		if (asSlots[n].wAddr == wAddr)
			break;
	}
	return n;
}


/*******************************************************************************
*
* bAddrToMem
*
* Searches for the given base address and returns the matching slave mapped 
* memory address.
*
*
* RETURNS: slave mapped memory address if found else 0.
* 
*******************************************************************************/
UINT32 bAddrToMem (UINT32 dBar)
{
	UINT8 n;
#ifdef DEBUG_SOAK
	char buff[64];
#endif

#ifdef DEBUG_SOAK
    for (n = 0; n < MAX_SLAVES; n++)
    {
        sprintf(buff, "asSlotsM.dBar %x n: %d\n", asSlotsM[n].dBar, n);
        sysDebugWriteString (buff);
    }
#endif

	for (n = 0; n < MAX_SLAVES; n++)
	{
		if (asSlotsM[n].dBar == dBar)
			break;
	}

	if (n < MAX_SLAVES)
	{
		return asSlotsM[n].dBarM;
	}
	else
	{
		return 0;
	}
}


/*******************************************************************************
*
* bMemToSlot
*
* Searches for the given base address and returns the matching slave number (slot).
*
*
* RETURNS: slave number if found else MAX_SLAVES.
* 
*******************************************************************************/
UINT32 bMemToSlot (UINT32 dBar)
{
	UINT8 n;

	for (n = 0; n < MAX_SLAVES; n++)
	{
		if (asSlotsM[n].dBar == dBar)
			break;
	}

	return n;
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
	UINT8	bSlot;

#ifdef DEBUG_SOAK
	char   buff[64];
	sysDebugWriteString ("vProcessPackets()\n");
#endif

	while ((psPkt = psPacketGetQueue(bQueue)) != NULL)
	{

#ifdef DEBUG_SOAK
		sprintf( buff, "wLength: 0x%04x wSourceID: 0x%04x wDestID: 0x%04x\n", 
				psPkt->hdr.wLength, psPkt->hdr.wSourceID, psPkt->hdr.wDestID );
		sysDebugWriteString (buff);
		
		sprintf( buff, "wFlags : 0x%04x wChecksum: 0x%04x dType: 0x%08x\n", 
				psPkt->hdr.wFlags, psPkt->hdr.wChecksum, psPkt->hdr.dType );
		sysDebugWriteString (buff);
#endif

		/* check packet address and route to slave if not for us */
		if (psPkt->hdr.wDestID != my_slot)
		{
#ifdef DEBUG_SOAK
			sprintf(buff, "Not for master, slot %x\n", psPkt->hdr.wDestID);
			sysDebugWriteString (buff);
#endif

			/* send it to specific slave board */
			bSlot = bAddrToSlot (psPkt->hdr.wDestID);

			wPacketSend (bSlot, psPkt, ACK_WAIT, PKT_TIMEOUT_NORM);
			continue;
		}

		/* check packet flags */
		if (psPkt->hdr.wFlags & FLAG_TST_ENT)
		{
		   vSthExecute (psPkt);
		}
		else
		{
			if (psPkt->hdr.wFlags & FLAG_CLK_SET)
			{
		   	vSthClock (psPkt);
			}
			else
			{
				switch (psPkt->hdr.dType)
				{
					case id_RBL:				/* request bist list */
						vSthHelp(psPkt);
						break;

					case id_BGO:				/* execute bist */
						vSthExecute(psPkt);
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
	DBLOG( "%s: Request test list\n", __FUNCTION__ );
#endif

	memset (&sReplyPacket, 0, sizeof(sReplyPacket));

	board_service(SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

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
        if ( (psTestList[test_id].wTestNum != 0    )  &&
			 (psTestList[test_id].wTestNum  < 0xff )  &&
			 (psTestList[test_id].dFlags & BITF__CUTE)   )
		{
			*((UINT16*)test_entry) = psTestList[test_id].wTestNum;
			test_entry+=2;
			if(psTestList[test_id].dFlags & BITF__COOP)
			{
				*((UINT16*)test_entry) = 0x85aa;
			}
			else
			{
				*((UINT16*)test_entry) = 0x331E;
			}
			test_entry+=2;

			memcpy (test_entry, &psTestList[test_id].achTestName[0], strlen(psTestList[test_id].achTestName));

			test_entry += strlen(psTestList[test_id].achTestName);
			*test_entry++ = '\0';
		}

		/* If the maximum test_id is reached, quit counting. */
		if (psTestList[test_id].wTestNum == 0x00)
			quit = TRUE;
		else						/* Else continue counting. */
			test_id = test_id + 1;
	}

	*test_entry = '\0';

	/* Send the bist list packet. */
	psBistList->hdr.wSourceID = my_slot;
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

extern UINT32 dDoTest(int iSupNotFound, int	wTestNum, UINT32* adTestParams );

static void vSthExecute (uPacket *psPkt)
{
	UINT32	test_status = E__OK;
	char*  ERROR_TEXT = "FAIL";

#ifdef DEBUG_SOAK
	char   buffer[64];

	sysDebugWriteString ("vSthExecute()\n");
#else
	DBLOG( "%s: Test: %d\n", __FUNCTION__, psPkt->sBgo.wTestId );
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

	vParseCommandTail (&psPkt->bData[12]);

	if(wNumParams > 0 )
		test_status = dDoTest (	0,	wSthTestId, &dParams[0] );
	else
		test_status = dDoTest (	0,	wSthTestId, NULL );

#ifdef DEBUG_SOAK
	sprintf(buffer, "Test ID: %d,  result: %x\n", wSthTestId /*psPkt->sBgo.wTestId*/, test_status);
	sysDebugWriteString (buffer);
#else
	DBLOG( "%s: result %x\n", __FUNCTION__, test_status );
#endif

	/* Initialise the input buffer. */
	memset (&sReplyPacket, 0, sizeof(sReplyPacket));


	sReplyPacket.hdr.wSourceID = my_slot;
	sReplyPacket.hdr.wDestID   = SOAK_ID;

	if (test_status == E__OK)
	{
		if (bControllerType == NET_SLAVE)	/* Network slave mode */
		{
#ifdef DEBUG_SOAK
	sprintf(buffer, "Sending on Network\n");
	sysDebugWriteString (buffer);
#endif
			sReplyPacket.hdr.wFlags  = 0x8A0;
			sReplyPacket.wData[5]    = wSthTestId;
			sReplyPacket.wData[6]    = (UINT16) (test_status & 0xffff);
			sReplyPacket.hdr.wLength = 14;
		}
		else
		{
#ifdef DEBUG_SOAK
	sprintf(buffer, "Sending on Serial Port\n");
	sysDebugWriteString (buffer);
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
	sprintf(buffer, "Sending on Network\n");
	sysDebugWriteString (buffer);
#endif
		sReplyPacket.hdr.wFlags  = 0x8A4;
/*
		sReplyPacket.wData[5]    = 0x8000 +  test_status;
		if(getErrorMsg(buffer) == 1)
		{
			strcpy ((char *)&sReplyPacket.bData[12], buffer);
			sReplyPacket.hdr.wLength = 13 + strlen (buffer) + 1;
		}
		else
		{
			strcpy ((char *)&sReplyPacket.bData[12], ERROR_TEXT);
			sReplyPacket.hdr.wLength = 13 + strlen (ERROR_TEXT) + 1;
		}
		
		interconnect_SetBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, BIST_FAIL);
*/
			sReplyPacket.wData[5]    = wSthTestId;
			sReplyPacket.wData[6]    = (UINT16) (0x8000 + (test_status & 0xffff));
			
			strcpy ((char *)&sReplyPacket.bData[14], ERROR_TEXT);
			sReplyPacket.hdr.wLength = 15 + strlen (ERROR_TEXT) + 1;
			interconnect_SetBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, BIST_FAIL);
		}
		else
		{
#ifdef DEBUG_SOAK
	sprintf(buffer, "Sending on Serial Port\n");
	sysDebugWriteString (buffer);
#endif
			sReplyPacket.hdr.wFlags  = 0x8A4;
			sReplyPacket.wData[5]    = 0x8000 + test_status;
			strcpy ((char *)&sReplyPacket.bData[12], ERROR_TEXT);
			sReplyPacket.hdr.wLength = 13 + strlen (ERROR_TEXT) + 1;
			interconnect_SetBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, BIST_FAIL);
		}
	}

	/* Flag interconnect, test execution has completed. */
	interconnect_ClearBit (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, INTERCONNECT_BIST_RUNNING);

#ifdef DEBUG_SOAK
	sprintf(buffer, "P3_packet_Flag: %x\n", P3_packet_Flag);
	sysDebugWriteString (buffer);
#endif
	if(P3_packet_Flag == 0)
	{
#ifdef DEBUG_SOAK
	sprintf(buffer, "Adding packet wPacketAddQueue()t\n");
	sysDebugWriteString (buffer);
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
void  IDX_p3_bist_exit (UINT16 wErrCode, char* pchTxt)
{
	UINT16 wLen;
	char *pchTmp;


	/* Indicate to vSthExecute not to send response */
	P3_packet_Flag = 1;

	//vCloseText();

    memset (&sReplyPacket, 0, sizeof(sReplyPacket));

    sReplyPacket.hdr.wSourceID = my_slot;
    sReplyPacket.hdr.wDestID = SOAK_ID;

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
	        sReplyPacket.hdr.wFlags = 0x8A4;
    	    sReplyPacket.wData[5]   = wErrCode == 1 ? (UINT16) E__OK : wErrCode;
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
		DBLOG("%s: failed, status: 0x%x\n", __FUNCTION__, status);
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
#ifdef DEBUG_SOAK
	char   buffer[64];
#endif

	
	offset = 6 + (pData[6]  & MASK_ASCII_LEN) + 1;
	endPos = offset + (pData[offset]  & MASK_ASCII_LEN);
	
	/* Get the board name */
	for (i = offset + 1, j = 0; i <= endPos; i++, j++)
	{
		sSthBoardName[j] = ( (pData[i] > 31) && (pData[i] < 127) ) ? pData[i] : '?';
	}
	
	sSthBoardName[j] = '\0';
	
	DBLOG( "%s: Board name   : %s\n", __FUNCTION__, sSthBoardName );
	
	offset = endPos + 1;
	endPos = offset + (pData[offset]  & MASK_ASCII_LEN);
	
	/* Get the serial number */
	for (i = offset + 1, j = 0; i <= endPos; i++, j++)
	{
		sSthSerNumber[j] = ( (pData[i] > 31) && (pData[i] < 127) ) ? pData[i] : '?';
	}
	
	sSthSerNumber[j] = '\0';
	
	DBLOG( "%s: Serial number: %s\n", __FUNCTION__, sSthSerNumber );
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
				DBLOG( "%s: Error: common header checksum failure\n", __FUNCTION__ );
				status = E__FAIL;
			}
			else if (respBuf[3] == 0 )
			{
				DBLOG( "%s: Error: invalid board area offset: %u\n", __FUNCTION__, respBuf[3] );
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
								DBLOG( "%s: Error: board info area checksum failure\n", __FUNCTION__ );
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
						DBLOG( "%s: Error: board info area too large\n", __FUNCTION__ );
						status = E__FAIL;
					}
				}
			}
		}
		
		// vIpmiDeinit();
	}
	else
	{
		DBLOG( "%s: Error: IPMI initialization failed\n", __FUNCTION__ );
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
#ifdef DEBUG_SOAK
	char   buffer[64];
#endif
	
	
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
	
		/* Unknown serial number */
		strcpy( sSthSerNumber, "?????\?/???" );
	}

#ifdef DEBUG_SOAK
	sprintf(buffer, "%s: Board name   : %s\n", __FUNCTION__, sSthBoardName );
	sysDebugWriteString (buffer);
	sprintf(buffer, "%s: Serial number: %s\n", __FUNCTION__, sSthSerNumber );
	sysDebugWriteString (buffer);
#else
	DBLOG( "%s: Board name   : %s\n", __FUNCTION__, sSthBoardName );
	DBLOG( "%s: Serial number: %s\n", __FUNCTION__, sSthSerNumber );
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
	
	if(board_service(SERVICE__BRD_GET_SLOT_ID, NULL, &slotId) == E__OK)
	{
		DBLOG("%s: slotId: 0x%04x\n", __FUNCTION__, slotId);
		
		// sReplyPacket.hdr.wSourceID = psPkt->hdr.wDestID;
		
		sReplyPacket.hdr.wSourceID = slotId;
	}
	else
	{
		DBLOG("%s: BRD_GET_SLOT_ID failed, slotId: 0x%04x\n", __FUNCTION__, slotId);
		
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
			my_slot = psPkt->wData[8 + i];
			
			DBLOG( "%s: update my_slot: 0x%04x\n", __FUNCTION__, my_slot );
		}
		
		sReplyPacket.wData[8 + i] = psPkt->wData[8 + i];
		sReplyPacket.hdr.wLength += 2;		
	}

	wPacketAddQueue (Q_TX, &sReplyPacket);
}

#endif /* INCLUDE_LWIP_SMC */

