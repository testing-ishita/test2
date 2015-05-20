/*******************************************************************************
*
* Filename:	 packet.c
*
* Description:	Core packet functions.
*
* $Revision: 1.4 $
*
* $Date: 2015-01-29 10:27:33 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/xlib/sm3/packet.c,v $
*
* Copyright 2008-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

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

// #define DEBUG_SOAK

#if defined(DEBUG_SOAK)
#warning "***** DEBUG ON *****"
#endif

/* globals */
sSlave			asSlots[MAX_SLAVES];
sSlaveMemBar	asSlotsM[MAX_SLAVES];
UINT8			abSlaveDev[MAX_SLOTS];
UINT8			bSlaves;
UINT8   		bControllerType;

static volatile UINT32 	mMapSpace1[MAX_SLAVES];		//PCI registers

static volatile UINT32 	mMapSpaceM[MAX_SLAVES];		//PCI registers


#define MAX_BOARDS (MAX_SLAVES/2)
extern UINT32 dGetPhysPtr
(
	UINT32		dBase,		/* base address */
	UINT32		dLength,	/* buffer length in bytes */
	PTR48*		pMemory,	/* pointer to the allocated block */
	void*		dlogaddr	/* Logical address to access memory contents */
);

#ifdef INCLUDE_LWIP_SMC
extern int smcReady( void );
extern int smcDetectMsg( void );
extern void smcSetMsgWaiting( void );
extern int smcPostMsg( void *pMsg, UINT32 uTimeOut );
extern int smcFetchMsg( void *ppMsg, UINT32 uTimeOut );
#endif

static UINT16  wSendParcel(UINT8 bSlot, const UINT8* pbBuffer, UINT16 wSize);
static UINT16  wReceiveParcel(UINT8 bSlot, UINT8* pbBuffer, UINT16 dMax, UINT16*	pdActual);
static UINT16  wSendByte (UINT8 bSlot, UINT8 bData, BOOL bEndOfParcel);
static UINT16  wReceiveByte (UINT8 bSlot, UINT8* bData);



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
void vGetSlaveBoards(void)
{
	UINT8	bDevice;

#if defined(CPCI)
	UINT8	bBus, eDevice = 0;
	UINT16	wVenId;
	UINT32	dBar, dTemp;
	PCI_PFA pfa;
	UINT32  dLocalBar = 0;
#endif
	
#if defined(VPX)
//	UINT32  dBar0;
//	PTR48  tPtr;
//	UINT32 mHandle;
//	UINT32 pointer;
#endif

	#if defined(CPCI)
	UINT32  bOccurance=0;
	#endif

	#if defined(VPX)
//		UINT8   switchConf, switchCount, counter;
	#endif

#if defined(VPX) || defined(CPCI)
	#ifdef DEBUG_SOAK
		char    buffer[128];
	#endif
#endif

	for (bDevice = 0; bDevice < MAX_SLAVES; bDevice++)
	{
		asSlots[bDevice].dBar = NULL;
	}

#if defined(VPX)
#if 0
	//board_service(SERVICE__BRD_GET_VPX_NT_COUNT,NULL, &switchConf);

	switchConf = (((UINT8)dIoReadReg(0x31E, REG_8)) >> 4) & 0x03;
	switch(switchConf)
	{
		case 0x00:
				switchCount = 8;
				break;

			case 0x01:
				switchCount = 5;
				break;

			case 0x02:
				switchCount = 2;
				break;

			case 0x03:
				switchCount = 1;
				break;
		}

	switchCount = 8;
	for(counter = 0; counter < switchCount; counter++)
	{
		/* first find the bus number for CPCI bus (HB8 sub-bus) */
		if ((iPciFindDeviceById ( 3+counter ,0x10B5,0x8619,&pfa) == E__OK)      ||
		    (iPciFindDeviceById ( 2+counter ,0x10B5,0x8717,&pfa) == E__OK))
		{
#ifdef DEBUG_SOAK
			sprintf(buffer, "\nvGetSlaveBoards(): found bridge instance %d\n", counter);
			sysDebugWriteString(buffer);
#endif

			/* get secondary bus number */
			bBus = dPciReadReg(pfa,0x19, REG_8);
			/*sPciBridge.bBus = bBus;*/

#ifdef DEBUG_SOAK
			sprintf(buffer, "SBUS : %d\n", bBus);
			sysDebugWriteString(buffer);
#endif

			/* now scan the secondary bus and remap devices */
			for (bDevice = 0; bDevice < MAX_BOARDS; bDevice++)
			{
				pfa = PCI_MAKE_PFA(bBus, bDevice, 0);
				/* check the subsystem ID */
				wVenId = dPciReadReg(pfa,0x00, REG_16);
#ifdef DEBUG_SOAK
				sprintf(buffer, "wVenId : %x\n", wVenId);
				sysDebugWriteString(buffer);
#endif
				eDevice = bDevice + 1 + counter;

				if ( wVenId == 0x10B5 )
				{
#ifdef DEBUG_SOAK
					sprintf(buffer, "vGetSlaveBoards(): found CCT slave at B:%d D:%d\n",
							(int)bBus, (int)bDevice);
					sysDebugWriteString(buffer);
#endif
					/* enable if */
					dTemp = dPciReadReg (pfa, 4, REG_16);
					dTemp |= 7;
					vPciWriteReg(pfa, 4, REG_16, dTemp);

					/* save the base address */
					dBar = dPciReadReg (pfa, 0x18, REG_32);

					if((dBar != NULL) && (dBar != 0xffffffff))
					{
						dLocalBar = (dBar & 0xffffff00);
						mMapSpace1[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x100000);
						asSlots[eDevice].dBar = mMapSpace1[eDevice] + 0x20000;
#ifdef DEBUG_SOAK
						sprintf (buffer,"vGetSlaveBoards(): CCT slave's BAR = %x, Mem1 %x,  Mem %x, Slave ID %x, Intrcon %x\n",
								dBar, mMapSpace1[eDevice], asSlots[eDevice].dBar, *((UINT32*)mMapSpace1[eDevice]),
								*((UINT32*)(mMapSpace1[eDevice]+0x20000)) );
						sysDebugWriteString(buffer);
#endif
						/* save the base address */
						dBar = dPciReadReg (pfa, 0x1c, REG_32);
						if((dBar != NULL) && (dBar != 0xffffffff))
						{
							asSlotsM[eDevice].dBar = dBar;
							dLocalBar = (dBar & 0xffffff00);
							mMapSpaceM[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x400000);
							asSlotsM[eDevice].dBarM = mMapSpaceM[eDevice];
#ifdef DEBUG_SOAK
							sprintf (buffer,"vGetSlaveBoards(): CCT slave's MEM BAR = %x, Mem %x,  DATA %x\n",
									dBar, mMapSpaceM[eDevice], *((UINT32*)mMapSpaceM[eDevice]));
							sysDebugWriteString(buffer);
#endif
						}
						else
						{
							asSlotsM[eDevice].dBarM = 0;
#ifdef DEBUG_SOAK
							sysDebugWriteString("MEM BAR not assigned correctly\n");
#endif
						}
					}
					else
					{
						asSlots[eDevice].dBar = 0;
#ifdef DEBUG_SOAK
						sysDebugWriteString("BAR not assigned correctly\n");
#endif
					}
					break;
				}
				else if (wVenId == 0x111D)
				{
#ifdef DEBUG_SOAK
					sprintf(buffer, "vGetSlaveBoards(): found CCT slave at B:%d D:%d\n",
							(int)bBus, (int)bDevice);
					sysDebugWriteString(buffer);
#endif
					/* enable if */
					dTemp = dPciReadReg (pfa, 4, REG_16);
					dTemp |= 7;
					vPciWriteReg(pfa, 4, REG_16, dTemp);



					/* save the base address */
					dBar = dPciReadReg (pfa, 0x14, REG_32);

					if((dBar != NULL) && (dBar != 0xffffffff))
					{
						dLocalBar = (dBar & 0xffffff00);
						mMapSpace1[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x100000);
						asSlots[eDevice].dBar = mMapSpace1[eDevice] + 0x20000;
#ifdef DEBUG_SOAK
						sprintf (buffer,"vGetSlaveBoards(): CCT slave's BAR = %x, Mem1 %x,  Mem %x, Slave ID %x, Intrcon %x\n",
								dBar, mMapSpace1[eDevice], asSlots[eDevice].dBar, *((UINT32*)mMapSpace1[eDevice]),
								*((UINT32*)(mMapSpace1[eDevice]+0x20000)) );
						sysDebugWriteString(buffer);
#endif
						/* save the base address */
						dBar = dPciReadReg (pfa, 0x18, REG_32);
						if((dBar != NULL) && (dBar != 0xffffffff))
						{
							asSlotsM[eDevice].dBar = dBar;
							dLocalBar = (dBar & 0xffffff00);
							mMapSpaceM[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x400000);
							asSlotsM[eDevice].dBarM = mMapSpaceM[eDevice];
#ifdef DEBUG_SOAK
							sprintf (buffer,"vGetSlaveBoards(): CCT slave's MEM BAR = %x, Mem %x,  DATA %x\n",
									dBar, mMapSpaceM[eDevice], *((UINT32*)mMapSpaceM[eDevice]));
							sysDebugWriteString(buffer);
#endif
						}
						else
						{
							asSlotsM[eDevice].dBarM = 0;
#ifdef DEBUG_SOAK
							sysDebugWriteString("MEM BAR not assigned correctly\n");
#endif
						}
					}
					else
					{
						asSlots[eDevice].dBar = 0;
#ifdef DEBUG_SOAK
						sysDebugWriteString("BAR not assigned correctly\n");
#endif
					}
					break;
				}
				else
					asSlots[eDevice].dBar = 0;
			}
		}

		/* first find the bus number for CPCI bus (HB8 sub-bus) */
		if (iPciFindDeviceById ( 5+counter ,0x111D,0x808C,&pfa) == E__OK)
		{
#ifdef DEBUG_SOAK
			sprintf(buffer, "\nvGetSlaveBoards(): found bridge instance %d\n", counter);
			sysDebugWriteString(buffer);

#endif

			/* get secondary bus number */
			bBus = dPciReadReg(pfa,0x19, REG_8);
			/*sPciBridge.bBus = bBus;*/

#ifdef DEBUG_SOAK
			sprintf(buffer, "SBUS : %d\n", bBus);
			sysDebugWriteString(buffer);
#endif

			/* now scan the secondary bus and remap devices */
			for (bDevice = 0; bDevice < MAX_BOARDS; bDevice++)
			{
				pfa = PCI_MAKE_PFA(bBus, bDevice, 0);
				/* check the subsystem ID */
				wVenId = dPciReadReg(pfa,0x00, REG_16);
#ifdef DEBUG_SOAK
				if (wVenId != 0xFFFF)
				{
					sprintf(buffer, "wVenId : %x\n", wVenId);
					sysDebugWriteString(buffer);
					sprintf(buffer, "\nInside IDT Switch Loop: bDevice: %d\n", bDevice);
					sysDebugWriteString(buffer);
				}
#endif
				eDevice = bDevice + 1 + counter;

				if ( wVenId == 0x10B5 )
				{
#ifdef DEBUG_SOAK
					sprintf (buffer,"++++eDevice:%d++++\n",eDevice);
					sysDebugWriteString(buffer);
					sprintf(buffer, "vGetSlaveBoards(): found CCT slave at B:%d D:%d\n",
							(int)bBus, (int)bDevice);
					sysDebugWriteString(buffer);
#endif

					/* enable if */
					dTemp = dPciReadReg (pfa, 4, REG_16);
					dTemp |= 7;
					vPciWriteReg(pfa, 4, REG_16, dTemp);

					/* save the base address */
					dBar = dPciReadReg (pfa, 0x18, REG_32);

					if((dBar != NULL) && (dBar != 0xffffffff))
					{
						dLocalBar = (dBar & 0xffffff00);
						mMapSpace1[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x80000);
						asSlots[eDevice].dBar = mMapSpace1[eDevice] + 0x20000;
#ifdef DEBUG_SOAK
						sprintf (buffer,"vGetSlaveBoards(): CCT slave's BAR = %x, Mem1 %x,  Mem %x, Slave ID %x, Intrcon %x\n",
								dBar, mMapSpace1[eDevice], asSlots[eDevice].dBar, *((UINT32*)mMapSpace1[eDevice]),
								*((UINT32*)(mMapSpace1[eDevice]+0x20000)) );
						sysDebugWriteString(buffer);
#endif
						/* save the base address */
						dBar = dPciReadReg (pfa, 0x1c, REG_32);
						if((dBar != NULL) && (dBar != 0xffffffff))
						{
							asSlotsM[eDevice].dBar = dBar;
							dLocalBar = (dBar & 0xffffff00);
							mMapSpaceM[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x400000);
							asSlotsM[eDevice].dBarM = mMapSpaceM[eDevice];
#ifdef DEBUG_SOAK
							sprintf (buffer,"vGetSlaveBoards(): CCT slave's MEM BAR = %x, Mem %x,  DATA %x\n",
									dBar, mMapSpaceM[eDevice], *((UINT32*)mMapSpaceM[eDevice]));
							sysDebugWriteString(buffer);
#endif
						}
						else
						{
							asSlotsM[eDevice].dBarM = 0;
#ifdef DEBUG_SOAK
							sysDebugWriteString("MEM BAR not assigned correctly\n");
#endif
						}
					}
					else
					{
						asSlots[eDevice].dBar = 0;
#ifdef DEBUG_SOAK
						sysDebugWriteString("BAR not assigned correctly\n");
#endif
					}
					break;
				}
				else if (wVenId == 0x111D)
				{
#ifdef DEBUG_SOAK
					sprintf(buffer, "vGetSlaveBoards(): found CCT slave at B:%d D:%d\n",
							(int)bBus, (int)bDevice);
					sysDebugWriteString(buffer);
#endif
					/* enable if */
					dTemp = dPciReadReg (pfa, 4, REG_16);
					dTemp |= 7;
					vPciWriteReg(pfa, 4, REG_16, dTemp);


					dBar0 = dPciReadReg (pfa, 0x10, REG_32);
					mHandle  = dGetPhysPtr(dBar0, 0x1000, &tPtr,(void*)(&pointer));
					*((UINT32*)(pointer + 0x400)) = 0x00000002;
					vDelay(2);

#ifdef DEBUG_SOAK
					sprintf(buffer, "pfa:%#x dBar0:%#x\n",pfa,dBar0);
					sysDebugWriteString(buffer);
					sprintf (buffer,"ptr:0x400:%#x\n",*((UINT32*)(pointer + 0x400)));
					sysDebugWriteString(buffer);
#endif
					/*vFreePtr(mHandle);*/

					/* save the base address */
					dBar = dPciReadReg (pfa, 0x14, REG_32);

					if((dBar != NULL) && (dBar != 0xffffffff))
					{
						dLocalBar = (dBar & 0xffffff00);
						mMapSpace1[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x80000);
						asSlots[eDevice].dBar = mMapSpace1[eDevice] + 0x20000;
#ifdef DEBUG_SOAK
						sprintf (buffer,"***eDevice:%d****\n",eDevice);
						sysDebugWriteString(buffer);

						sprintf (buffer,"vGetSlaveBoards(): CCT slave's BAR = %x, Mem1 %x,  Mem %x, Slave ID %x, Intrcon %x\n",
								dBar, mMapSpace1[eDevice], asSlots[eDevice].dBar, *((UINT32*)mMapSpace1[eDevice]),
								*((UINT32*)(mMapSpace1[eDevice]+0x20000)) );
						sysDebugWriteString(buffer);
#endif
						/* save the base address */
						dBar = dPciReadReg (pfa, 0x18, REG_32);
						if((dBar != NULL) && (dBar != 0xffffffff))
						{
							asSlotsM[eDevice].dBar = dBar;
							dLocalBar = (dBar & 0xffffff00);
							mMapSpaceM[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x400000);
							asSlotsM[eDevice].dBarM = mMapSpaceM[eDevice];
#ifdef DEBUG_SOAK
							sprintf (buffer,"vGetSlaveBoards(): CCT slave's MEM BAR = %x, Mem %x,  DATA %x\n",
									dBar, mMapSpaceM[eDevice], *((UINT32*)mMapSpaceM[eDevice]));
							sysDebugWriteString(buffer);
#endif
						}
						else
						{
							asSlotsM[eDevice].dBarM = 0;
#ifdef DEBUG_SOAK
							sysDebugWriteString("MEM BAR not assigned correctly\n");
#endif
						}
					}
				}
				else
					asSlots[eDevice].dBar = 0;
		    }
		}
	}

#ifdef DEBUG_SOAK
	for (bDevice = 0; bDevice < MAX_BOARDS; bDevice++)
	{
		if (asSlots[bDevice].dBar != 0)
		{

			sprintf(buffer, "bDevice:%d asSlots[eDevice].dBar: %#x\n",bDevice,asSlots[bDevice].dBar);
			sysDebugWriteString(buffer);
			sprintf(buffer, "asSlots[bDevice].dBar: %#x\n",*((UINT32*)(asSlots[bDevice].dBar)));
			sysDebugWriteString(buffer);

			sprintf (buffer,"ptr:0x400:%#x\n",*((UINT32*)(pointer + 0x400)));
			sysDebugWriteString(buffer);
		}
	}
#endif /*DEBUG_SOAK*/
#endif	// #if 0
#endif	// #if defined(VPX)

#if defined(CPCI)
	/* first find the bus number for CPCI bus (HB8 sub-bus) */
		/*bOccurance = 1;*/
		board_service(SERVICE__BRD_GET_CPCI_INSTANCE,NULL,&bOccurance);

		if (iPciFindDeviceById ( bOccurance ,0x12D8,0xE130,&pfa) == E__OK)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString("vGetSlaveBoards(): found bridge\n");

			sprintf(buffer, "vGetSlaveBoards(): bOccurance: %d\n",bOccurance);
			sysDebugWriteString(buffer);
#endif

			/* get secondary bus number */
			bBus = dPciReadReg (pfa, 0x19, REG_8);

			/* now scan the secondary bus and remap devices */
			for (bDevice = 0; bDevice < MAX_BOARDS; bDevice++)
			{
				pfa = PCI_MAKE_PFA(bBus, bDevice, 0);
				/* check the subsystem ID */
				wVenId = dPciReadReg(pfa,0x00, REG_16);
				/* check the subsystem ID */

				eDevice = bDevice + 1;

				if (wVenId == 0x12D8 || wVenId == 0x10B5 || wVenId == 0x3388)
				{
#ifdef DEBUG_SOAK
					sprintf(buffer, "vGetSlaveBoards(): found CCT slave at B:%d D:%d\n",
							(int)bBus, (int)bDevice);
					sysDebugWriteString(buffer);
#endif
					/* enable if */
					dTemp = dPciReadReg (pfa, 4, REG_16);
					dTemp |= 7;
					vPciWriteReg(pfa, 4, REG_16, dTemp);

					/* save the base address */
					dBar = dPciReadReg (pfa, 0x10, REG_32);
					if (wVenId == 0x12D8)
					{
						dLocalBar = (dBar & 0xffffff00);
						mMapSpace1[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x100000);
						asSlots[eDevice].dBar = mMapSpace1[eDevice] + 0x21000; 
#ifdef DEBUG_SOAK
						sprintf (buffer,"vGetSlaveBoards(): CCT slave's BAR = %x, Mem1 %x,  Mem %x, Slave ID %x, Intrcon %x\n",
								dBar, mMapSpace1[eDevice], asSlots[eDevice].dBar, *((UINT32*)mMapSpace1[eDevice]),
								*((UINT32*)(mMapSpace1[eDevice]+0x21000)) ); 
						sysDebugWriteString(buffer);
#endif
						/* save the base address */
						dBar = dPciReadReg (pfa, 0x18, REG_32);
						if((dBar != NULL) && (dBar != 0xffffffff))
						{
							asSlotsM[eDevice].dBar = dBar;
							dLocalBar = (dBar & 0xffffff00);
							asSlotsM[eDevice].dBarM = 0;
							mMapSpaceM[eDevice] = 0;
#ifdef DEBUG_SOAK
							sprintf (buffer,"vGetSlaveBoards(): CCT slave's MEM BAR = %x\n",dBar);
							sysDebugWriteString(buffer);
#endif
						}
						else
						{
							asSlotsM[eDevice].dBarM = 0;
#ifdef DEBUG_SOAK
							sysDebugWriteString("MEM BAR not assigned correctly\n");
#endif
						}
					}
					else
					{
						dLocalBar = (dBar & 0xffffff00);
						mMapSpace1[eDevice]=  sysMemGetPhysPtrLocked((UINT64)dLocalBar, 0x100000);
						asSlots[eDevice].dBar = mMapSpace1[eDevice] + 0x1000;
	#ifdef DEBUG_SOAK
						sprintf (buffer,"vGetSlaveBoards(): CCT slave's BAR = %x, Mem1 %x,  Mem %x, Slave ID %x, Intrcon %x\n",
								 dBar, mMapSpace1[eDevice], asSlots[eDevice].dBar, *((UINT32*)mMapSpace1[eDevice]),
								 *((UINT32*)(mMapSpace1[eDevice]+0x1000)) );
						sysDebugWriteString(buffer);
	#endif
						/* save the base address */
						dBar = dPciReadReg (pfa, 0x18, REG_32);
						if((dBar != NULL) && (dBar != 0xffffffff))
						{
							asSlotsM[eDevice].dBar = dBar;
							dLocalBar = (dBar & 0xffffff00);
							asSlotsM[eDevice].dBarM = 0;
							mMapSpaceM[eDevice] = 0;
#ifdef DEBUG_SOAK
							sprintf (buffer,"vGetSlaveBoards(): CCT slave's MEM BAR = %x\n",dBar);
							sysDebugWriteString(buffer);
#endif
						}
						else
						{
							asSlotsM[eDevice].dBarM = 0;
#ifdef DEBUG_SOAK
							sysDebugWriteString("MEM BAR not assigned correctly\n");
#endif
						}
					}
				}
				else
					asSlots[eDevice].dBar = 0;
			}
		}

#endif

}


/*******************************************************************************
*
* vCommInit
*
* Sets the controller type and initializes communications if necessary.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void vCommInit (void)
{

#ifdef INCLUDE_LWIP_SMC
	bControllerType = NET_SLAVE;
	
#ifdef DEBUG_SOAK
	sysDebugWriteString ("vCommInit: Network mode\n");
#else
	DBLOG( "%s: Network mode\n", __FUNCTION__ );
#endif
	
#else
	/* Check if the board is in peripheral mode */
	if (bIsPeripheral() == TRUE)
		bControllerType = CPCI_SLAVE;	// peripheral mode
	else
		bControllerType = CPCI_MASTER;	// SYS CON

	if (bControllerType == CPCI_MASTER)
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString ("vCommInit: Master mode\n");
#else
		DBLOG( "%s: Master mode\n", __FUNCTION__ );
#endif
		vGetSlaveBoards();
		InitSerialComm();
	}
	else
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString ("vCommInit: Slave mode\n");
#else
		DBLOG( "%s: Slave mode\n", __FUNCTION__ );
#endif
#if defined(VPX)
		vVpxInit ();
#endif
#if defined(CPCI)
		vCpciInit ();
#endif
	}
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
	
#ifdef DEBUG_SOAK
	sysDebugWriteString ("wMessageGet\n");
#endif

	if (bControllerType == CPCI_MASTER)/* Master mode */
	{
		wLength = 0;

		if (bSlot != INTERCONNECT_LOCAL_SLOT)// read from local interconnect
		{
			wStatus = wReceiveParcel (bSlot, pbMessage, wMax, &wLength);
		}
		else // read from the soak master over the comm link
		{
			wStatus = wSerialMessageGet( pbMessage,wMax, &wLength );
		}

		if (wLength < 2)
			wStatus = (UINT16)E__TOO_SHORT;
	}
#ifdef INCLUDE_LWIP_SMC
	else if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		if (bSlot == INTERCONNECT_LOCAL_SLOT)
		{
			wStatus = smcFetchMsg( pbMessage, 100 );
		}
		else
		{
			wStatus = E__NOT_FOUND;
		}
	}
#endif
	else/* Slave mode: read from the interconnect */
	{
		wStatus = wReceiveParcel (INTERCONNECT_LOCAL_SLOT, pbMessage, wMax, &wLength);
	}

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

#ifdef DEBUG_SOAK
	sysDebugWriteString ("wMessagePut\n");
#endif

	if (bControllerType == CPCI_MASTER) /* Master mode */
	{
		if (bSlot != INTERCONNECT_LOCAL_SLOT)
		{
			wSendParcel (bSlot, pbMessage, wLength);
		}
		else
		{
			wStatus = wSerialMessagePut( pbMessage,	wLength, dTimeout );
		}

		return wStatus;
	}
#ifdef INCLUDE_LWIP_SMC
	else if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		if (bSlot == INTERCONNECT_LOCAL_SLOT)
		{
			return smcPostMsg( (void *) pbMessage, 100 );
		}
		else
		{
			return E__NOT_FOUND;
		}
	}
#endif
	else/* Slave mode */
	{
		wSendParcel (INTERCONNECT_LOCAL_SLOT, pbMessage, wLength);
		return E__OK;
	}
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
	if (bControllerType == CPCI_MASTER)/* Master mode */
	{
		if (bSlot == INTERCONNECT_LOCAL_SLOT)
		{
			return Serial_detect_message ();
		}
		else
		{
			if (asSlots[bSlot].bFitted)
			{
				if ((bReadIcReg (bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
					INTERCONNECT_BIST_OUT_DATA_VALID) == INTERCONNECT_BIST_OUT_DATA_VALID)
					return E__OK;
			}
			return E_NOT_PENDING;

		}
	}
#ifdef INCLUDE_LWIP_SMC
	else if (bControllerType == NET_SLAVE)	/* Network slave mode */
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
#endif
	else/* Slave mode */
	{
		if (bSlot == INTERCONNECT_LOCAL_SLOT)
		{
			if ((bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
				BIST_IN_DATA_VALID) != BIST_IN_DATA_VALID)
				return E_NOT_PENDING;

			return E__OK;
		}
		else
			return E_NOT_PENDING;
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
	UINT8	bTmp;

	if (bControllerType == CPCI_MASTER)/* Master mode */
	{
		return serialSoakMasterReady();
	}
#ifdef INCLUDE_LWIP_SMC
	else if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		return smcReady();
	}
#endif
	else/* Slave mode */
	{
		bTmp = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS);
		if ((bTmp & INTERCONNECT_BIST_OUTPUT_PENDING) == 0)
			return FALSE;
		else
			return TRUE ;
	}
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
	UINT8	bTmp;

	if (bControllerType == CPCI_MASTER)/* Master mode */
	{
		serialSetPacketWaiting();
	}
#ifdef INCLUDE_LWIP_SMC
	else if (bControllerType == NET_SLAVE)	/* Network slave mode */
	{
		smcSetMsgWaiting();
	}
#endif
	else/* Slave mode */
	{
		/* Indicate that a byte is to be sent by a slave. */
		bTmp  = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS);
		bTmp |= INTERCONNECT_BIST_OUTPUT_PENDING;
		vWriteIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, bTmp);
	}
}


/*******************************************************************************
*
* wReceiveByte
*
* Receives a byte from a remote agent via interconnect and stores it in memory 
* at a pointer location specified by the caller.
* Use bSlot = INTERCONNECT_LOCAL_SLOT in slave mode.  
*
*
* RETURNS: E__OK.
* 
*******************************************************************************/
static UINT16 wReceiveByte (UINT8 bSlot, UINT8* bData)
{
	UINT16	wStatus;
	UINT8	bValue;

	wStatus = E__OK;

	/* Master mode */

	if (bControllerType == CPCI_MASTER)
	{
		/* Wait for data from master to become valid */
		while ((bReadIcReg(bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
				INTERCONNECT_BIST_OUT_DATA_VALID) != INTERCONNECT_BIST_OUT_DATA_VALID)
			vDelay(1);

		/* Read a parcel byte */
		*bData = bReadIcReg (bSlot, INTERCONNECT_BIST_DATA_OUT);

		/* Confirm byte was transfered to slave. */
		bValue = bReadIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS);
		bValue |= BIST_OUT_DATA_ACCEPT;
		vWriteIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS, bValue);

		/* Wait for master to to clear BIST Data In Valid. */
		while ((bReadIcReg(bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
			   INTERCONNECT_BIST_OUT_DATA_VALID) == INTERCONNECT_BIST_OUT_DATA_VALID)
			vDelay(1);

		/* Reset the BIST In Data Accept bit. */
		bValue = bReadIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS);
		bValue &= ~BIST_OUT_DATA_ACCEPT;
		vWriteIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS, bValue);

		return wStatus;
	}

	/* Slave mode */

	else
	{
		/* Initialize the status. */
		wStatus = E__OK;

		/* Wait for data from master to become valid before continuing. */
		while ((bReadIcReg(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
				BIST_IN_DATA_VALID) !=  BIST_IN_DATA_VALID)
			vDelay(1);

		/* Read a parcel byte. */
		*bData = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_DATA_IN);

		/* Confirm byte was transfered to slave. */
		bValue = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS);
		bValue |= INTERCONNECT_BIST_IN_DATA_ACCEPT;
		vWriteIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, bValue);

		/* Wait for master to to clear BIST Data In Valid. */
		while ((bReadIcReg(INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
				BIST_IN_DATA_VALID) == BIST_IN_DATA_VALID)
			vDelay(1);

		/* Reset the BIST In Data Accept bit. */
		bValue = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS);
		bValue &= ~INTERCONNECT_BIST_IN_DATA_ACCEPT;
		vWriteIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, bValue);

		return wStatus;
	}
} /* wReceiveByte */


/*******************************************************************************
*
* wSendByte
*
* Sends a byte to a remote agent via interconnect.  
*
*
* RETURNS: E__OK.
* 
*******************************************************************************/
static UINT16 wSendByte (UINT8 bSlot, UINT8 bData, BOOL bEndOfParcel)
{
	UINT16	wStatus;
	UINT8	bValue;

	/* Initialize variables. */
	wStatus = E__OK;


	/* Master mode */

	if (bControllerType == CPCI_MASTER)
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString ("wSendByte MASTER\n");
#endif
		/* Indicate that a byte is to be sent by a slave. */
		bValue = bReadIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS);
		bValue |= BIST_INPUT_PENDING;
		vWriteIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS, bValue);

		/* Write a parcel byte. */
		vWriteIcReg (bSlot, INTERCONNECT_BIST_DATA_IN, bData);

		bValue = bReadIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS);
		bValue |= BIST_IN_DATA_VALID;
		vWriteIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS, bValue);

		/* Wait for master to acknowledge the data. */
		while ((bReadIcReg (bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
			   INTERCONNECT_BIST_IN_DATA_ACCEPT) != INTERCONNECT_BIST_IN_DATA_ACCEPT)
			vDelay(1);

		/* Check if there are any more bytes to send. */
		if (bEndOfParcel == TRUE)
		{

			/* Indicate that the last byte of the parcel has been sent by
			 * the slave. */
			bValue = bReadIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS);
			bValue &= ~BIST_INPUT_PENDING;
			vWriteIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS, bValue);
		}

		/* clear the data valid bit */
		bValue = bReadIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS);
		bValue &= ~BIST_IN_DATA_VALID;
		vWriteIcReg (bSlot, INTERCONNECT_BIST_MASTER_STATUS, bValue);

		/* Wait for master to acknowledge reading data not valid */
		while ((bReadIcReg (bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
			   INTERCONNECT_BIST_IN_DATA_ACCEPT) == INTERCONNECT_BIST_IN_DATA_ACCEPT)
		vDelay(1);

	}
	else/* Slave mode */
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString ("wSendByte SLAVE\n");
#endif
		/* Indicate that a byte is to be sent by a slave. */
		bValue = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS);
		bValue |= INTERCONNECT_BIST_OUTPUT_PENDING;
		vWriteIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, bValue);

		/* Write a parcel byte */
		vWriteIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_DATA_OUT, bData);

		bValue = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS);
		bValue |= INTERCONNECT_BIST_OUT_DATA_VALID;
		vWriteIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, bValue);

		/* Wait for master to acknowledge the data */
		while ((bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
			   BIST_OUT_DATA_ACCEPT) != BIST_OUT_DATA_ACCEPT)
			vDelay(1);

		/* Check if there are any more bytes to send. */
		if (bEndOfParcel == TRUE)
		{
			/* Indicate that the last byte of the parcel has been sent by
			 * the slave.
			 */
			bValue = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS);
			bValue &= ~INTERCONNECT_BIST_OUTPUT_PENDING;
			vWriteIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, bValue);
		}

		/* clear the data valid bit */
		bValue = bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS);
		bValue &= ~INTERCONNECT_BIST_OUT_DATA_VALID;
		vWriteIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_SLAVE_STATUS, bValue);


		/* Wait for master to acknowledge reading data not valid */
		while ((bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
			   BIST_OUT_DATA_ACCEPT) == BIST_OUT_DATA_ACCEPT)
			vDelay(1);

	}

#ifdef DEBUG_SOAK
	sysDebugWriteString ("wSendByte DONE\n");
#endif
	return wStatus;

} /* wSendByte */


/*******************************************************************************
*
* wReceiveParcel
*
* Receives a parcel from a remote agent via interconnect and stores it in memory 
* at a pointer location specified by the caller.
* Use bSlot = INTERCONNECT_LOCAL_SLOT in slave mode.  
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
static UINT16 wReceiveParcel
(
	UINT8	bSlot,
	UINT8*	pbBuffer,
	UINT16	dMax,
	UINT16*	pdActual
)
{

	UINT16	wStatus;
	UINT8	bByteBucket;
	BOOL	bDone;
	UINT16	dIndex;
	UINT16	wTimeout;
	UINT32	dTimeout;

	wStatus = E__OK;

	/* Master mode */

	if (bControllerType == CPCI_MASTER)
	{
		dTimeout = TIMEOUT_PERIOD;
		/* Wait until the master is ready to send the packet. */
		while ((bReadIcReg (bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
				INTERCONNECT_BIST_OUTPUT_PENDING) != INTERCONNECT_BIST_OUTPUT_PENDING)
		{
			if (dTimeout--  == 0)
			{
				*pdActual = 0;
				return E__TIMEOUT;
			}
			vDelay(1);
		}

		dIndex = 0;
		bDone = FALSE;

		/* Read all bytes of the parcel from the MASTER. */
		while (((bReadIcReg (bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
			   INTERCONNECT_BIST_OUTPUT_PENDING) == INTERCONNECT_BIST_OUTPUT_PENDING) && !(bDone))
		{
			/* Read a parcel byte. */
			wStatus = wReceiveByte(bSlot, pbBuffer++);

			/* Get ready to read the next byte. */
			dIndex++;

			if (dIndex == dMax)
				bDone = TRUE;
		}

		/* Check for overrun condition. */
		if ((bReadIcReg (bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
			INTERCONNECT_BIST_OUTPUT_PENDING) == INTERCONNECT_BIST_OUTPUT_PENDING )
		{
			bDone = FALSE;

			/* Read in the rest of the parcel. */
			while (((bReadIcReg (bSlot, INTERCONNECT_BIST_SLAVE_STATUS) &
				   INTERCONNECT_BIST_OUTPUT_PENDING) == INTERCONNECT_BIST_OUTPUT_PENDING) &&
				   !(bDone))
			{
				/* Read a parcel byte. */
				wReceiveByte(bSlot, &bByteBucket);

				/* Get ready to read the next byte. */
				dIndex++;
			}


			/* Flag overrun condition. */
			wStatus = E_LENGTH_OVERRUN;

		}

		/* Store the actual size of the parcel received. */
		*pdActual = dIndex;

		return wStatus;

	}

	/* Slave mode */

	else
	{
		/* Initialize the status and variables. */

		wStatus = E__OK;
		wTimeout=TIMEOUT_PERIOD;

		/* Wait until the master is ready to send the packet. */
		while ((bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
			   BIST_INPUT_PENDING) != BIST_INPUT_PENDING)
		{
			if (wTimeout-- == 0)
				return E__TIMEOUT;
			vDelay(1);
		}

		dIndex = 0;
		bDone = FALSE;

		/* Read all bytes of the parcel from the MASTER. */
		while (((bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
			   BIST_INPUT_PENDING) == BIST_INPUT_PENDING) && (!bDone))
		{

			/* Read a parcel byte. */
			wStatus = wReceiveByte(INTERCONNECT_LOCAL_SLOT, pbBuffer++);

			/* Get ready to read the next byte. */
			dIndex++;

			if (dIndex == dMax)
				bDone = TRUE;

		}

		/* Check for overrun condition. */
		if ((bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
			BIST_INPUT_PENDING) == BIST_INPUT_PENDING )
		{
			//dprintf("wReceiveParcel(): overrun\n");
			bDone = FALSE;

			/* Read in the rest of the parcel. */
			while (((bReadIcReg (INTERCONNECT_LOCAL_SLOT, INTERCONNECT_BIST_MASTER_STATUS) &
				   BIST_INPUT_PENDING) == BIST_INPUT_PENDING) &&
				   (!bDone))
			{
				/* Read a parcel byte. */
				wReceiveByte(INTERCONNECT_LOCAL_SLOT, &bByteBucket);

				/* Get ready to read the next byte. */
				dIndex++;
			}

			/* Flag overrun condition. */
			wStatus = E_LENGTH_OVERRUN;

		}


		/* Store the actual size of the parcel received. */
		*pdActual = dIndex;

		return wStatus;
	}
} /* wReceiveParcel */


/*******************************************************************************
*
* wSendParcel
*
* Sends a parcel to a remote agent via interconnect.  
*
*
* RETURNS: E__OK.
* 
*******************************************************************************/
static UINT16 wSendParcel
(
	UINT8 			bSlot,
	const UINT8*	pbBuffer,
	UINT16			wSize
)
{
	UINT16	wStatus;
	UINT8	bEndOfParcel;
	UINT16	wIndex;


	/* Master mode */
	if (bControllerType == CPCI_MASTER)
	{
		wIndex = 0;
		wStatus = E__OK;
		bEndOfParcel = FALSE;

#ifdef DEBUG_SOAK
		sysDebugWriteString ("wSendParcel MASTER\n");
#endif
		/* Send all bytes of the parcel, unless an interconnect write fails */
		while ((wIndex < wSize) && (wStatus == E__OK))
		{
			if (wIndex == (wSize - 1))
				bEndOfParcel = TRUE;

			/* Send a parcel byte. */
			wSendByte (bSlot, *pbBuffer++, bEndOfParcel);

			wIndex++;
		}
#ifdef DEBUG_SOAK
		sysDebugWriteString ("wSendParcel DONE\n");
#endif
		return wStatus;
	}

	/* Slave mode */

	else
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString ("wSendParcel SLAVE\n");
#endif
		wIndex = 0;
		wStatus = E__OK;
		bEndOfParcel = FALSE;

		/* Send all bytes of the parcel, unless an interconnect write fails */
		while ((wIndex < wSize) && (wStatus == E__OK))
		{

			if (wIndex == (wSize - 1))
				bEndOfParcel = TRUE;

			/* Send a parcel byte. */
			wSendByte (INTERCONNECT_LOCAL_SLOT, *pbBuffer++, bEndOfParcel);

			wIndex++;

		}
#ifdef DEBUG_SOAK
		sysDebugWriteString ("wSendParcel DONE\n");
#endif
		return wStatus;
	}

} /* wSendParcel */

