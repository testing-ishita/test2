
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
* Filename:	 packet.c
*
* Description:	Core packet functions.
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/xlib/idx/packet.c,v 1.2 2015-01-29 10:28:37 mgostling Exp $
* $Log: packet.c,v $
* Revision 1.2  2015-01-29 10:28:37  mgostling
* Added support for ethernet soak master.
* Added CVS headers and tidied up the source code.
*
*
*******************************************************************************/




#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <string.h>

#include <bit/io.h>
#include <bit/bit.h>
#include <bit/delay.h>

#include <private/cpu.h>
#include <private/debug.h>

#include <comm/serial.h>
#include <cute/arch.h>
#include <cute/interconnect.h>
#include <cute/packet.h>
#include <bit/board_service.h>

#include "config.h"
#include "../debug/dbgLog.h"

//#define DEBUG_SOAK

#if defined(DEBUG_SOAK)
#warning "***** DEBUG ON *****"
#endif

#define MAX_BOARDS (MAX_SLAVES/2)

#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

/* globals */
sm3vmeSlave		asSlots[MAX_SLAVES];
UINT8			abSlaveDev[MAX_SLOTS];
UINT8			bSlaves;
UINT8   		bControllerType;


#ifdef INCLUDE_LWIP_SMC
extern int smcReady( void );
extern int smcDetectMsg( void );
extern void smcSetMsgWaiting( void );
extern int smcPostMsg( void *pMsg, UINT32 uTimeOut );
extern int smcFetchMsg( void *ppMsg, UINT32 uTimeOut );
#endif

/*******************************************************************************
*
* vGetSlaveBoards
*
* This routine finds the first HB8 bridge and scans its secondary
* bus for devices with the CCT_SUBSYSID sub system id.
* The base address of any devices found is added to the array asSlots.
*
* RETURNS: None.
* 
*******************************************************************************/
void vGetSlaveBoards (void)
{
	UINT32	 dTemp,i;
	VMEBARS VMEMemBars;

#ifdef DEBUG_SOAK
	char buffer[ 128 ];
#endif
	
	GetVmeBars (&VMEMemBars);

	for (i = 0; i < MAX_SLAVES; i++)
	{
		asSlots[i].dBar = NULL;
	}

	//Universe Boards
    for (i =0; i< 10; i++)
    {
		dTemp  = vReadByte (VMEMemBars.Intercon + (4096*i));
		if(dTemp == 0x25)
		{
			asSlots[i].dBar = VMEMemBars.Intercon + (4096*i);

#ifdef DEBUG_SOAK
			sprintf (buffer, "UNIVERSE-II Board @ 0x%x, slot %d, data %x\n",
						asSlots[i].dBar, i, vReadByte(asSlots[i].dBar));
			sysDebugWriteString (buffer);
#endif
		}
    }

    //TSI148 Boards
    for (i = 1; i < 19; i++)
    {
		dTemp = vReadByte (VMEMemBars.Intercon + (65536*i));
		if(dTemp == 0x25)
		{
			asSlots[i+9].dBar = VMEMemBars.Intercon + (65536*i);
#ifdef DEBUG_SOAK
			sprintf (buffer, "TSI148 Board @ 0x%x, slot %d, data %x\n",
						asSlots[i+9].dBar, (i+9), vReadByte(asSlots[i+9].dBar));
			sysDebugWriteString (buffer);
#endif
		}
    }
}



void vCommInit (void)
{

#ifdef INCLUDE_LWIP_SMC
	bControllerType = NET_SLAVE;

#ifdef DEBUG_SOAK
	sysDebugWriteString ("vCommInit: Network mode\n");
#else
	DBLOG ("%s: Network mode\n", __FUNCTION__);
#endif

#else
	/* Check if the board is in peripheral mode */
	bControllerType = IdentifyAgent();
	vGetSlaveBoards();
	InitSerialComm();
#endif /* INCLUDE_LWIP_SMC */
}


/*******************************************************************************
*
* wMessageGet
*
* Attempts to read a message from the Soak Master or interconnect slave.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
UINT32 wMessageGet
(
	UINT8  bSlot,			/* which slot to get message from		*/
	UINT8 *pbMessage,		/* Pointer to message structure			*/
	UINT16 wMax				/* Maximum number of chars to receive	*/
)
{
	UINT16	wLength;
	UINT32	wStatus = E__OK;

	wLength = 0;

#ifdef INCLUDE_LWIP_SMC
	if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		if (bSlot == INTERCONNECT_LOCAL_SLOT)
		{
			wStatus = smcFetchMsg (pbMessage, 100);
		}
		else
		{
			wStatus = E__NOT_FOUND;
		}

		return wStatus;
	}
	else 
#endif
	if (bSlot != INTERCONNECT_LOCAL_SLOT)// read from local interconnect
	{
		// will have to use VME parcel functions here!

		//wStatus = wReceiveParcel (bSlot, pbMessage, wMax, &wLength);
	}
	else // read from the soak master over the comm link
	{
		wStatus = wSerialMessageGet (pbMessage, wMax, &wLength);
	}


	if (wLength < 2)
		wStatus = (UINT16)E__TOO_SHORT;

	return wStatus;
}


/*******************************************************************************
*
* wMessagePut
*
* Attempts to send a message to the Soak Master or interconnect slave.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
UINT32 wMessagePut
(	
	UINT8  bSlot,
	const UINT8 *pbMessage,
	UINT16 wLength,
	UINT32 dTimeout
)
{
	UINT32	wStatus = E__OK;

#ifdef INCLUDE_LWIP_SMC
	if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		if (bSlot == INTERCONNECT_LOCAL_SLOT)
		{
			return smcPostMsg ((void *) pbMessage, 100);
		}
		else
		{
			return E__NOT_FOUND;
		}
	}
	else
#endif
	if (bSlot != INTERCONNECT_LOCAL_SLOT)
	{
		// will have to use VME parcel functions here!
		// wSendParcel (bSlot, pbMessage, wLength);
	}
	else
	{
		wStatus = wSerialMessagePut (pbMessage,	wLength, dTimeout);
	}

	return wStatus;
}


/*******************************************************************************
*
* IDX_detect_message
*
* Attempts to detect a message from the Soak Master or interconnect slave.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
UINT16 IDX_detect_message (UINT8 bSlot)
{
	UINT8 temp, done;
#ifdef DEBUG_SOAK
	char buff[64];
#endif

#ifdef INCLUDE_LWIP_SMC
	if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		if (bSlot == INTERCONNECT_LOCAL_SLOT)
		{
			return smcDetectMsg();
		}
		else
		{
			return E_NOT_PENDING;
		}
	}
	else 
#endif
	if( bSlot == INTERCONNECT_LOCAL_SLOT )
	{
		return Serial_detect_message();
	}
	else
	{
		if (asSlots[bSlot].bFitted)
		{
			interconnect_ReadByte( bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &temp );

			if(temp == 0xff)
			{
#ifdef DEBUG_SOAK
				sprintf(buff, "\nIDX_detect_message() slot %d received 0xff \n\n", bSlot);
				sysDebugWriteString(buff);
#endif
				return (E_NOT_PENDING);
			}

			done = temp & BIST_OUT_DATA_VALID;	/* out data valid? */
			if (done)
				return (E__OK);
			else
				return (E_NOT_PENDING);
		}

		return (E_NOT_PENDING);
	}
}



/*******************************************************************************
*
* bSoakMasterReady
*
* Checks if the Soak Master communication is ready.
*
*
* RETURNS: TRUE or FALSE.
* 
*******************************************************************************/
UINT8 bSoakMasterReady (void)
{
#ifdef INCLUDE_LWIP_SMC
	if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		return smcReady();
	}
#endif
	return serialSoakMasterReady();
}


/*******************************************************************************
*
* vSetPacketWaiting
*
* Sets packet waiting for the Soak Master or interconnect slave.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void vSetPacketWaiting (void)
{
#ifdef INCLUDE_LWIP_SMC
	if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		smcSetMsgWaiting();
	}
	else
#endif
	serialSetPacketWaiting();
}



/*****************************************************************************
 *  TITLE:  bAddrToSlot()												[SM3]*
 *****************************************************************************/
UINT8 bAddrToSlot (UINT16 wAddr)
{
	UINT8 n;
#if 0
	UINT8 i;
	char  buff[64];
#endif

	for (n = 0; n < MAX_SLAVES; n++)
	{
		if (asSlots[n].wAddr == wAddr)
			break;
	}

	if (n == MAX_SLAVES)
	{
#if 0//#ifdef DEBUG_SOAK
		sysDebugWriteString ("\n\n");
		for (i = 0; i < MAX_SLAVES; i++)
		{
			sprintf (buff, "Slot %d, fitted %x, addr %x, bar %x\n", i, asSlots[i].bFitted, asSlots[i].wAddr,
							asSlots[i].dBar);
					sysDebugWriteString (buff);
		}
		sysDebugWriteString ("\n\n");
#endif

		return (0xff);
	}
	else
		return (n);
}



/*****************************************************************************
 *  TITLE:  bAddrToSlot ()												[SM3]*
 *****************************************************************************/
UINT32 bSlotToBar (UINT8 slot)
{
	return (asSlots[ slot ].dBar);
}
