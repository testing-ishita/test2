/************************************************************************/
/*                                                                      */
/*      Copyright 1992 Concurrent Technologies, all rights reserved.    */
/*                                                                      */
/*      The program below is supplied by Concurrent Technologies        */
/*      on the understanding that no responsibility is assumed by       */
/*      Concurrent Technologies for any errors contained therein.       */
/*      Furthermore, Concurrent Technologies makes no commitment to     */
/*      update or keep current the program code, and reserves the       */
/*      right to change its specifications at any time.                 */
/*                                                                      */
/*      Concurrent Technologies assumes no responsibility either for    */
/*      the use of this code or for any infringements of the patent     */
/*      or other rights of third parties which may result from its use. */
/*                                                                      */
/************************************************************************/

/*******************************************************************************
*
* Filename:	 autoid.c
*
* Description:	This file contains code for managing the initialization and 
*               auto-ID processes for Compact PCI boards.
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/xlib/idx/autoid.c,v 1.3 2015-02-27 09:05:17 mgostling Exp $
* $Log: autoid.c,v $
* Revision 1.3  2015-02-27 09:05:17  mgostling
* Ignore all slave boards for VME ethernet cute
*
* Revision 1.2  2015-01-29 10:42:15  mgostling
* Added support for ethernet soak master.
* Added CVS headers and tidied up the source code.
*
*
*******************************************************************************/


/* includes */
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <string.h>

 
#include <bit/bit.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>

#include <private/port_io.h>
#include <private/cpu.h>
#include <private/debug.h>

#include <comm/serial.h>
#include <cute/arch.h>
#include <cute/interconnect.h>
#include <cute/packet.h>

#include "config.h"
#include "../debug/dbgLog.h"

//#define DEBUG_SOAK

#if defined(DEBUG_SOAK)
#warning "***** DEBUG ON *****"
#endif


#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr)  = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define CPU		INTERCONNECT_LOCAL_SLOT

/* globals */
extern sm3vmeSlave	asSlots   [MAX_SLAVES];
extern UINT8		abSlaveDev[MAX_SLOTS ];
extern UINT8		bSlaves;

/* externals */
extern UINT8   board_type;
extern UINT8   bControllerType;
extern UINT8   bVsaGlobalIc[];


/* locals */
struct s_slotInfo
{
	UINT16 wAddr;
	char   achName[10];
};

static uPacket sPkt;
static uPacket sRxPkt;


/*******************************************************************************
*
* iGetNumSlaves
*
* Get the number of CPCI intelligent slave boards.
*
*
* RETURNS: the number of slaves found.
* 
*******************************************************************************/
static UINT8 iGetNumSlaves (void)
{
	UINT16	wCount;
	UINT8	slavecount;

#ifdef DEBUG_SOAK
	char	buffer[64];
	sysDebugWriteString ("GetNumSlaves()\n");
#endif

	slavecount = 0;
	bSlaves    = 0;

	for (wCount = 0; wCount < MAX_SLAVES; wCount++)
	{
		abSlaveDev[wCount] = 0;
		asSlots[wCount].bFitted  = FALSE;

		if (asSlots[wCount].dBar != NULL)
		{
#ifdef DEBUG_SOAK
			sprintf (buffer, "asSlots[%d].dBar %x ", wCount, asSlots[wCount].dBar);
			sysDebugWriteString (buffer);
#endif

			if (vReadByte((asSlots[wCount].dBar)) == 0x25)
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString ("Found Slave\n");
#endif
				asSlots[wCount].bFitted = TRUE;
				abSlaveDev[slavecount]  = wCount;
				slavecount++;
			}
			else
			{
#ifdef DEBUG_SOAK
				sprintf (buffer, "No IC data %x\n", vReadByte(asSlots[wCount].dBar));
				sysDebugWriteString (buffer);
#endif
			}
		}
	}

	bSlaves = slavecount;
	return (slavecount);
}


/*******************************************************************************
*
* iGetSlaveInfo
*
* Gets the slave information for the requested number of slave boards.
*
*
* RETURNS: the number of bytes filled in the given buffer.
* 
*******************************************************************************/
static int iGetSlaveInfo (UINT8 *data, UINT8 bNumSlaves)
{
	struct s_slotInfo*	pbData;
	int 	n;
	UINT16	wCount;
#ifdef DEBUG_SOAK
	char    achbuff[64];
#endif

#ifdef DEBUG_SOAK
	sprintf (achbuff, "GetSlaveInfo for %d slaves\n", bNumSlaves);
	sysDebugWriteString (achbuff);
#endif

	pbData = (struct s_slotInfo*)data;

	for (wCount = 0; wCount < bNumSlaves; wCount++)
	{
		UINT8 bDev;

		bDev = abSlaveDev[wCount];
		pbData->wAddr = (UINT16)bDev;
		for (n = 0; n < 10; n++)
		{
			pbData->achName[n] = vReadByte (asSlots[bDev].dBar + 2 + (2*n));
#ifdef DEBUG_SOAK
			sprintf (achbuff, "%i:%c\t", n, pbData->achName[n]);
			sysDebugWriteString (achbuff);
#endif
		}
#ifdef DEBUG_SOAK
		sysDebugWriteString ("\n");
#endif
		pbData++;
	}

	return ((UINT8*)pbData) - data;		/* return bytes filled in */
}



/*******************************************************************************
*
* wPerformAutoId
*
* Runs the Auto-ID process to determine slot ID (board address).
* General Process:-
*		Scan for slave boards.
*		Send AA_SL packet.
*		Wait for AA_PORT packet.
*		assign and return slot ID.
*
* RETURNS: address of this board.
* 
*******************************************************************************/
UINT16 wPerformAutoId (void)
{
	int		n;
	UINT16	wStatus;
	UINT8*  pbData;
	int		dCount;

#ifdef DEBUG_SOAK
	int		i;
	char    achbuff[64];
#endif

#ifdef DEBUG_SOAK
	sysDebugWriteString ("wPerformAutoId\n");
#endif

#ifdef INCLUDE_LWIP_SMC
	if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString ("Network slave\n");
#else
		DBLOG ("%s: Network slave\n", __FUNCTION__);
#endif	

		wStatus = INTERCONNECT_LOCAL_SLOT;
	}
	else
	{
#endif
		memset (&sPkt, 0, sizeof(sPkt));
		n = iGetNumSlaves();

#ifdef DEBUG_SOAK
		sprintf (achbuff, "Slave count = %d\n", n);
		sysDebugWriteString (achbuff);
#endif

		pbData  = &sPkt.bData[14];
		*pbData = (UINT8)(n + 1);
		pbData++;
		*(UINT16 *)pbData = 0;
		pbData += 2;
		for (dCount = 2; dCount < 12; dCount++)
		{
			interconnect_ReadByte (INTERCONNECT_LOCAL_SLOT, dCount, pbData++);
		}
		n = iGetSlaveInfo (pbData, n ) + 15;

#ifdef DEBUG_SOAK
		sprintf (achbuff, "Num Bytes = %d\n", n);
		sysDebugWriteString (achbuff);
#endif

		sPkt.hdr.wLength   = 15 + n;
		sPkt.hdr.wSourceID = 0;
		sPkt.hdr.wDestID   = SOAK_ID;
		sPkt.hdr.wFlags	   = FLAG_SYS_PKT | FLAG_HAS_DTA |
							FLAG_WHO_AMI | FLAG_PKT_PRI;
		sPkt.hdr.dType	   = (UINT32)id_AA_SL;

#ifdef DEBUG_SOAK
		sprintf (achbuff, "sPkt.hdr.wLength: %d \n", sPkt.hdr.wLength);
		sysDebugWriteString (achbuff);
	
		for (i = 0; i<sPkt.hdr.wLength; i++)
		{
			sprintf (achbuff, "%x ", sPkt.bData[i]);
			sysDebugWriteString (achbuff);
		}
		sysDebugWriteString ("\n\n");
#endif

		do
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString ("Master send AA_SL\n");
#endif

			do
			{
				wStatus = wPacketSend (INTERCONNECT_LOCAL_SLOT, &sPkt, NO_ACK_WAIT, 1000);
			} while( wStatus == (UINT16) E__TIMEOUT );
	
			/* wait for AA_PORT packet to be received */
#ifdef DEBUG_SOAK
			sysDebugWriteString ("Master wait PORT\n");
#endif

			wStatus = wPacketReceive (INTERCONNECT_LOCAL_SLOT, &sRxPkt, NO_ACK_WAIT);
	
			if (wStatus == 0)
			{
				/* packet received OK	*/
				if (sRxPkt.hdr.dType == (UINT32)id_AA_PORT)
				{
					UINT8 bIndex = 0;
					wStatus = 0xFFFE;	/* special case for slave only test */
#ifdef DEBUG_SOAK
					sysDebugWriteString ("Master got AA_PORT\n");
#endif

					for (n = 0; (n < sRxPkt.bData[14]) && (n < MAX_SLAVES); n++)
					{
						if (n == 0)
						{
#ifdef DEBUG_SOAK
							sysDebugWriteString ("Assign System controller address\n");
#endif

							wStatus = (UINT16)sRxPkt.bData[15] + ((UINT16)sRxPkt.bData[16] << 8);
						}
						else
						{
#ifdef DEBUG_SOAK
							sysDebugWriteString ("Master Assign slave address\n");
#endif

							asSlots[abSlaveDev[bIndex++]].wAddr =  (UINT16)sRxPkt.bData[15 + (n * 12)] +
																((UINT16)sRxPkt.bData[16 + (n * 12)] << 8);
#ifdef DEBUG_SOAK
							sprintf (achbuff, "bSlaves %d asSlots[abSlaveDev[bIndex++]].wAddr %x, mem %x\n",
									bSlaves, asSlots[abSlaveDev[bIndex-1]].wAddr, asSlots[abSlaveDev[bIndex-1]].dBar);
							sysDebugWriteString (achbuff);
#endif
						}
					}
				}
				else
					wStatus = 0xffff;
			}
			else
				wStatus = 0xffff;
	
		} while (wStatus == 0xffff);
	
		/* post AA packets to slave boards */
		for (n = 0; n < bSlaves; n++)
		{
			UINT8 bDev;
			bDev = abSlaveDev[n];
			if (asSlots[bDev].bFitted)
			{
#ifdef DEBUG_SOAK
				char achBuf[80];
				sprintf (achBuf, "Master send slave %d AA_PORT ", bDev);
				sysDebugWriteString (achBuf);
#endif

				sPkt.hdr.wLength    = 17;
				sPkt.hdr.wSourceID  = wStatus;
				sPkt.hdr.wDestID    = 0;
				sPkt.hdr.wFlags     = FLAG_SYS_PKT | FLAG_HAS_DTA |
									FLAG_WHO_AMI | FLAG_PKT_PRI;
				sPkt.hdr.dType      = (UINT32)id_AA_PORT;
				*(UINT16*)(&sPkt.bData[15]) = asSlots[bDev].wAddr;
				wPacketSend (bDev, &sPkt, NO_ACK_WAIT, 5);//PKT_TIMEOUT_NONE

#ifdef DEBUG_SOAK
				sysDebugWriteString ("Done.\n");
#endif
			}
		}
#ifdef INCLUDE_LWIP_SMC
	}
#endif

	return wStatus;
} /* PerformAutoId () */
