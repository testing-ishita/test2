
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

/*******************************************************************************
*
* Filename:	 serialPacket.c
*
* Description:	This file contains code for managing packet send and receive.
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/xlib/idx/serialPacket.c,v 1.2 2015-01-29 10:24:45 mgostling Exp $
* $Log: serialPacket.c,v $
* Revision 1.2  2015-01-29 10:24:45  mgostling
* Added support for ethernet soak master.
* Added CVS headers and tidied up the source code.
*
*
*******************************************************************************/


#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <string.h>

 
#include <bit/bit.h>
#include <bit/delay.h>

#include <private/cpu.h>
#include <private/debug.h>

#include <comm/serial.h>
#include <cute/arch.h>
#include <cute/interconnect.h>
#include <cute/packet.h>
#include <cute/sm3.h>

#include "config.h"

//#define DEBUG_SOAK

#if defined(DEBUG_SOAK)
#warning "***** DEBUG ON *****"
#endif

#define FREE_PACKETS			32

extern UINT16 slot; // comes from cpci autoid

static uPacket sAckPacket;
static uPacket asPktList[FREE_PACKETS];
static UINT8   abPktUsed[FREE_PACKETS];

/*******************************************************************************
*
* bAckThis
*
* Determines if the given packet should be acknowledged.
*
*
* RETURNS: TRUE or FALSE.
* 
*******************************************************************************/
static BOOL bAckThis (uPacket *psPkt)
{
	/* check for ACK/NAK packet */

	if (psPkt->hdr.wFlags & FLAG_ACK_PKT)
		return FALSE;

	/* check for RBL	*/
	if (psPkt->hdr.wFlags & FLAG_HAS_DTA)
	{
		if (psPkt->hdr.dType == (UINT32) id_RBL)
			return FALSE;
	}

	return TRUE;
}


/*******************************************************************************
*
* vSendAck
*
* Send an acknowledge packet.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void vSendAck (UINT8 bSlot, UINT16 wSourceID, UINT16 wDestID, BOOL bNak)
{
	uPacket sPacket;

	memset (&sPacket, 0, sizeof (sPacket));

	sPacket.hdr.wSourceID = wSourceID;
	sPacket.hdr.wDestID   = wDestID;

	sPacket.hdr.wFlags = FLAG_ACK_PKT | FLAG_SYS_PKT;
	if (bNak)
		sPacket.hdr.wFlags |= FLAG_ACK_NAK;
	sPacket.hdr.wLength     = 10;

	wPacketSend (bSlot, &sPacket, NO_ACK_WAIT, PKT_TIMEOUT_NORM);
}


/*******************************************************************************
*
* wPacketSend
*
* Send the given packet.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
UINT16 wPacketSend (UINT8 bSlot, uPacket *psPkt, UINT8 bAckWait, UINT32 dTimeout)
{
	UINT16		wChkSum;
	UINT16		wStatus;
	int			idx;


	/* checksum the packet */
	wChkSum = 0;
	for(idx=0; idx<8; idx++)
		wChkSum += psPkt->bData[idx];

	/* skip the checksum itself (at bytes 8..9) */
	for(idx=10; idx<psPkt->hdr.wLength; idx++)
    	wChkSum += psPkt->bData[idx];

	psPkt->hdr.wChecksum = wChkSum;

#ifdef INCLUDE_LWIP_SMC
	/* don't wait for ACK packet if using network */
	bAckWait = FALSE;
#else
	/* if this is a ACK packet then don't wait for ACK */
	if (!bAckThis (psPkt))
		bAckWait = FALSE;
#endif

	/* send it */
	do
	{
		wStatus = wMessagePut (bSlot, (UINT8 *)psPkt, psPkt->hdr.wLength, dTimeout);

		if (wStatus == (UINT16)E__TIMEOUT)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString ("wPacketSend MessagePut Timed out chk\n");
#endif
			return wStatus;
		}

		/* wait for ACK /NAK */
		if (bAckWait)
		{
			memset (&sAckPacket, 0, 12);
			wStatus = wMessageGet (bSlot, (UINT8 *)&sAckPacket,
							        (UINT16)sizeof(sAckPacket));
			if (wStatus == (UINT16)E__TIMEOUT)
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString ("Timout waiting for Ack\n");
#endif
			}
		}

	} while (bAckWait && ((sAckPacket.hdr.wFlags &
			  (FLAG_ACK_PKT | FLAG_ACK_NAK)) != FLAG_ACK_PKT));

	return E__OK;
}




/*******************************************************************************
*
* wPacketReceive
*
* Receive a packet.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
UINT16	wPacketReceive(UINT8 bSlot, uPacket *psPkt, BOOL bAckSend)
{

	UINT32	idx;					/* array index 			*/
	UINT16	wChkSum, wErr;

#ifdef DEBUG_SOAK
	char    achBuffer[64];
#endif

	do
	{
		memset (psPkt, 0, sizeof(uPacket));
		wErr = wMessageGet (bSlot, (UINT8 *)psPkt, (UINT16)sizeof (uPacket));

		switch (wErr)
		{
			case E__READ_OVERFLOW:
#ifdef DEBUG_SOAK
				sprintf (achBuffer,"wPacketReceive() Recieved packet too big\n");
				sysDebugWriteString (achBuffer);
#endif
				return wErr;
				break;

			case E__TIMEOUT:
#ifdef DEBUG_SOAK
				sprintf (achBuffer,"wPacketReceive() Rx timeout\n");
				sysDebugWriteString (achBuffer);
#endif
				return wErr;
				break;

			case E__TOO_SHORT:
#ifdef DEBUG_SOAK
				sprintf (achBuffer, "wPacketReceive() Short packet\n");
				sysDebugWriteString (achBuffer);
#endif
				return wErr;
				break;
		}

		if (wErr == E__OK)
		{
			/* Checksum the incoming data */
			wChkSum = 0;
			for (idx=0; idx<8; idx++)
				wChkSum += psPkt->bData[idx];

			/* skip the checksum itself (at bytes 8..9) */
			for (idx=10; idx<psPkt->hdr.wLength; idx++)
				wChkSum += psPkt->bData[idx];

			if (wChkSum != psPkt->hdr.wChecksum)
			{
				//sysDebugWriteString ("Rx checksum error\n");
				wErr = (UINT16)E__CHECKSUM;
				if (bAckSend && bAckThis(psPkt))
				{
					vSendAck(bSlot, psPkt->hdr.wSourceID, psPkt->hdr.wDestID, TRUE);
				}
			}
			else
			{
				//sysDebugWriteString("wPacketReceive() Rx packet OK\n");
				wErr = (UINT16)E__OK;
				if (bAckSend && bAckThis(psPkt))
				{
					vSendAck (bSlot, psPkt->hdr.wSourceID, psPkt->hdr.wDestID, FALSE);
				}
				bAckSend = FALSE;
			}
		}
	} while (bAckSend);

	return wErr;
}


/*******************************************************************************
*
* psPacketAlloc
*
* Allocate a packet from the free pool.
*
*
* RETURNS: Pointer to the allocated packet or NULL.
* 
*******************************************************************************/
static uPacket* psPacketAlloc (UINT8 bQueue)
{
	UINT8	bCount = 0;

	/* scan packet list for unused packets */
	while ((bCount < FREE_PACKETS) && abPktUsed[bCount])
	{
		bCount++;
	}

	if (bCount == FREE_PACKETS)
	{
		return (uPacket*)NULL;
	}
	else
	{
		abPktUsed[bCount] = bQueue;
		return (&asPktList[bCount]);
	}
}


/*******************************************************************************
*
* vFreeAllPackets
*
* Return all packets to the pool.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void vFreeAllPackets ( void )
{
	UINT8	bCount = 0;

	while (bCount < FREE_PACKETS)
		abPktUsed[bCount++] = 0;
}


/*******************************************************************************
*
* wPacketFree
*
* Return the given packets to the pool.
*
*
* RETURNS: E__OK or E__NOT_FOUND.
* 
*******************************************************************************/
static UINT16 wPacketFree (uPacket *psPkt)
{
	UINT8	bCount = 0;

	/* scan packet list for matching packet */
	while ((bCount < FREE_PACKETS) && (psPkt != &asPktList[bCount]))
	{
		bCount++;
	}

	if( bCount == FREE_PACKETS )
	{
		return (UINT16)E__NOT_FOUND;
	}
	else
	{
		abPktUsed[bCount] = 0;
		return (UINT16)E__OK;
	}
}


/*******************************************************************************
*
* wPacketAddQueue
*
* Add a packet to the given queue.
*
*
* RETURNS: E__OK or E__NO_PACKETS.
* 
*******************************************************************************/
UINT16 wPacketAddQueue (UINT8 bQueue, uPacket* psPkt)
{
	uPacket *psNewPkt;

	psNewPkt = psPacketAlloc(bQueue);
	if (psNewPkt != (uPacket*)NULL)
	{
		memcpy (psNewPkt, psPkt, sizeof (uPacket));

		if (bQueue == Q_TX)
		{
			/* signal we have packets to send */
			vSetPacketWaiting();
		}
		return (UINT16)E__OK;
	}
	return (UINT16)E__NO_PACKETS;
}


/*******************************************************************************
*
* psPacketGetQueue
*
* Get next packet from the given queue.
*
*
* RETURNS: Pointer to the packet or NULL.
* 
*******************************************************************************/
uPacket* psPacketGetQueue (UINT8 bQueue)
{
	UINT8	bCount;
	uPacket* psPkt;

	/* if this is the receive Q then process slave packets first */
	if (bQueue == Q_RX)
	{
		/* get the next slave board packet */
		for (bCount = 0; bCount < FREE_PACKETS; bCount++)
		{
			if (abPktUsed[bCount] == bQueue)
			{
				psPkt = &asPktList[bCount];
				if (psPkt->hdr.wDestID != slot)
				{
					wPacketFree (psPkt);
					return psPkt;
				}
			}
		}
	}

	/* OK now just the next packet */
	for (bCount = 0; bCount < FREE_PACKETS; bCount++)
	{
		if (abPktUsed[bCount] == bQueue)
		{
			psPkt = &asPktList[bCount];
			wPacketFree (psPkt);
			return psPkt;
		}
	}

	return (uPacket*) NULL;
}


/*******************************************************************************
*
* wPacketSendQueue
*
* Send all the packets in the given queue.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
UINT16 wPacketSendQueue (UINT8 bQueue, UINT8 bSlot)
{
	UINT8	 bCount;
	uPacket* psPkt;
	UINT16   wStatus;

	/* if packet to send then tell smaster */
	if (!bSoakMasterReady())
	{
		for (bCount = 0; bCount < FREE_PACKETS; bCount++)
		{
			if (abPktUsed[bCount] == bQueue)
			{
				vSetPacketWaiting();
				break;
			}
		}
	}

	/* nobody waiting then exit */
	if (!bSoakMasterReady())
		return E__OK;

	for (bCount = 0; bCount < FREE_PACKETS; bCount++)
	{
		if (abPktUsed[bCount] == bQueue)
		{
			psPkt = &asPktList[bCount];
			wStatus = wPacketSend (bSlot, psPkt, ACK_WAIT, PKT_TIMEOUT_NORM);
			if (wStatus == (UINT16)E__OK)
				wPacketFree (psPkt);
			else
			{
				break;
			}
		}
	}

	return wStatus;
}

