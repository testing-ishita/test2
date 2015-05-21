
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

/* cpciArch.c - functions reporting board specific architecture
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/arch/cpci/cpciArch.c,v 1.3 2015-04-01 16:41:54 hchalla Exp $
 *
 * $Log: cpciArch.c,v $
 * Revision 1.3  2015-04-01 16:41:54  hchalla
 * Added support for CPCI backplane configuration and testing.
 *
 * Revision 1.2  2013-10-08 07:18:39  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:09:46  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2012/02/17 11:27:55  hchalla
 * Added support for PP81x and clearing bit from the NT side of the pericom bridge for CPCI presensce of pericom bridge.
 *
 * Revision 1.7  2011/10/27 15:46:51  hmuneer
 * no message
 *
 * Revision 1.6  2011/04/05 13:03:41  hchalla
 * Added Board Service to find the CPCI bridge instance and for configuring the CPCI backplane.
 *
 *
 */
//cleanup
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
 
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>
#include <private/debug.h>
#include <private/port_io.h>

#include <cute/arch.h>
#include <cute/packet.h>
#include <cute/interconnect.h>
#include <bit/board_service.h>

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)	(*(UINT8*) (regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

#define	CPCI_STATUS					0x214
#define  UC_MODE					0x80
#define	 CPCI_FAL					0x40	/* 1 = asserted */
#define	 CPCI_DEG					0x20	/* 1 = asserted */
#define	 CPCI_VIO					0x10	/* 1 = VIO present and valid */
#define	 CPCI_FREQ					0x08	/* 0 = 33MHz, 1 = 66MHz */
#define	 FORCE_SAT					0x04	/* 1 = forced satellite mode */
#define	 CPCI_PRESENT				0x02	/* 1 = CPCI bus present */
#define	 CPCI_SYSEN					0x01	/* 1 = System Controller */

#define PI7X130_DEVID_VENID	 0xE13012D8
//#define DEBUG_SOAK
/* globals */
extern sSlave	asSlots[MAX_SLAVES];
extern UINT8	abSlaveDev[MAX_SLOTS];
extern UINT8	bSlaves;
extern UINT8 	my_slot;

/* externals */
extern UINT8   bControllerType;


/* locals */
static PCI_PFA 			gpfa;
static UINT32			dBar;				/* saved BAR for HB6 downstream image */
static volatile UINT32 	mapRegSpace;		//PCI registers
static volatile UINT32 	mapRegSpace1;		//PCI registers

struct s_slotInfo
{
	UINT16 wAddr;
	char   achName[10];
};


void cpciPreInit (UINT8 bFlag)
{
	PCI_PFA pfa;
	UINT16 slotId=0;
	UINT32 upStreamMemAddr = 0;
	CPCI_DRV_INFO *cpciDrvInfo;

#ifdef DEBUG_SOAK
	char    buffer[64];
#endif


	board_service(SERVICE__BRD_CONFIG_CPCI_BACKPLANE,NULL,&pfa);
	if (bFlag == 1)
	{
			UINT8	bTemp;
#ifdef DEBUG_SOAK
			UINT16 wVId = 0,wDId=0;
#endif
			bTemp = PCI_READ_BYTE (pfa, 0x04);
			bTemp |= 0x07;
			PCI_WRITE_BYTE (pfa, 0x04, bTemp);
			if (board_service(SERVICE__BRD_GET_SLOT_ID, NULL, &slotId) == E__OK)
			{
				board_service(SERVICE__BRD_GET_CPCI_BRIDGE_IMAGES, NULL, &cpciDrvInfo);
				cpciDrvInfo->deviceId = PI7X130_DEVID_VENID;
				cpciDrvInfo->pfa = pfa;
				cpciDrvInfo->Currnetslot = slotId;

#ifdef DEBUG_SOAK
				wVId = PCI_READ_WORD(pfa,0x00);
				sprintf(buffer,"Pericom wVId:%#x\n",wVId);
				sysDebugWriteString(buffer);

				wDId = PCI_READ_WORD(pfa,0x02);
				sprintf(buffer,"Pericom wDId:%#x\n",wDId);
				sysDebugWriteString(buffer);

				sprintf(buffer,"In Peripheral Mode Configuring CPCI\n");
				sysDebugWriteString(buffer);
#endif
				PCI_WRITE_DWORD(pfa, 0x40, 0x09200020);
				PCI_WRITE_DWORD(pfa, 0x44, 0x02B00007);
				//PCI_WRITE_DWORD(pfa, 0x9c, 0xFFE00000);
				//PCI_WRITE_DWORD(pfa, 0x98, 0x00);

				/* create downstream 1MB memory test window */
				PCI_WRITE_DWORD(pfa, 0xAC, 0xFFF00008);
				PCI_WRITE_DWORD(pfa, 0xA8, 0x10000000);
				PCI_WRITE_BYTE(pfa, 0x43, 0x7F & PCI_READ_BYTE(pfa, 0x43));

				/* create upstream 1MB memory test window */
				upStreamMemAddr = (0x11000000 * slotId);
				PCI_WRITE_DWORD(pfa, 0xEC, 0xFFF00008);
				PCI_WRITE_DWORD(pfa, 0xE8,upStreamMemAddr);
			}
			else
			{

			}
			/* set bus master enable */
			bTemp  = PCI_READ_BYTE (pfa, 0x04);
			bTemp |= 0x04;
			PCI_WRITE_BYTE (pfa, 0x04, bTemp);
	}
	else
	{
		UINT8 bTemp, bCount;
		slotId = 1;

		board_service(SERVICE__BRD_GET_CPCI_BRIDGE_IMAGES, NULL, &cpciDrvInfo);
		cpciDrvInfo->deviceId = PI7X130_DEVID_VENID;
		cpciDrvInfo->pfa = pfa;
		cpciDrvInfo->Currnetslot = slotId;

		/* system controller, so delay a short while to allow */
		/* peripheral boards to configure themselves.		  */
		bTemp = dIoReadReg (0x61,REG_8) & (0xfd);
		for (bCount = 0; bCount < 5; bCount++)
		{
			bTemp = dIoReadReg (0x211,REG_8) & (~0x01);
			vIoWriteReg (0x211,REG_8, bTemp | 0x01);
			vDelay (1000);
			vIoWriteReg (0x211,REG_8,bTemp);
			vDelay (1000);
		}
		/* flash the user LED */
		bTemp = dIoReadReg (0x211,REG_8) & (~0x01);
		vIoWriteReg (0x211,REG_8, bTemp | 0x01);
		vDelay(1000);
		vIoWriteReg (0x211,REG_8,bTemp);
	}

	board_service(SERVICE__BRD_DECONFIG_CPCI_BACKPLANE,NULL,NULL);


} /* IDX_post_pmode_init () */


/***********************************************************************
 * Simulated interconnect read/write functions
 *
 * Use bSlot = 0 in when call function in slave mode
 */
UINT8 bReadIcByte (UINT8 bSlot, UINT8 bReg)
{
    UINT32 dRegOffset;

	/* Master mode */
	if ((bSlot == my_slot) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		dRegOffset = bReg;

		return (vReadByte((0x21000+dRegOffset)));
	}
	else
	{
	    dRegOffset = bReg + asSlots[bSlot].dBar;

		return (vReadByte(dRegOffset));
	}

} /* bReadIcByte */


void vWriteIcByte (UINT8 bSlot, UINT8 bReg, UINT8 bData)
{
	UINT32 dRegOffset;

	/* Master mode */
	if ((bSlot == my_slot) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		dRegOffset = bReg; //Added Multiply by 2
		vWriteByte((0x21000+dRegOffset), (bData));
	}
	else/* Slave mode */
	{
		dRegOffset = bReg + asSlots[bSlot].dBar;
		vWriteByte((dRegOffset), (bData));
	}

} /* vWriteIcByte */


/***********************************************************************
 * Simulated interconnect read/write functions
 *
 * Use bSlot = 0 in when call function in slave mode
 */
UINT8 bReadIcReg (UINT8 bSlot, UINT8 bReg)
{
    UINT32 dRegOffset;

	/* Master mode */
	if ((bSlot == my_slot) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		dRegOffset = bReg *2; //Added Multiply by 2

		return (vReadByte((0x21000+dRegOffset)));
	}
	else
	{
	    dRegOffset = (bReg *2) + asSlots[bSlot].dBar;

		return (vReadByte(dRegOffset));
	}

} /* bReadIcByte */


void vWriteIcReg (UINT8 bSlot, UINT8 bReg, UINT8 bData)
{
	UINT32 dRegOffset;

	/* Master mode */
	if ((bSlot == my_slot) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		dRegOffset = bReg * 2; //Added Multiply by 2
		vWriteByte((0x21000+dRegOffset), (bData));
	}
	else/* Slave mode */
	{
		dRegOffset = (bReg *2) + asSlots[bSlot].dBar;
		vWriteByte((dRegOffset), (bData));
	}

} /* vWriteIcByte */




/*************************************************************************
 * bIsPeripheral - Returns TRUE if board is in peripheral mode
 *
 */
UINT8 bIsPeripheral (void)
{
	UINT8 bPeri;

	bPeri = dIoReadReg(0x214, REG_8);

	if ( (bPeri & 0x01) != 0x01)
		return TRUE;
	else
		return FALSE;
}



/*************************************************************************
 * vVpxInit - Find PCI-PCI bridge and save its address
 *
 */
void vCpciInit (void)
{
	UINT32 dInstance = 0;
#ifdef DEBUG_SOAK
	sysDebugWriteString ("vCpciInit\n");
#endif

	board_service(SERVICE__BRD_GET_CPCI_INSTANCE,NULL,&dInstance);
	if (bIsPeripheral ())
	{
		memset (&gpfa, 0, sizeof(gpfa));
		if( iPciFindDeviceById (dInstance,0x12D8,0xE130,&gpfa) == E__OK)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString ("Device Found\n");
#endif
		}
	}
}


/*************************************************************************
 * vDisableMemWindow - close window into PCI space
 *
 */

void vDisableMemWindow (void)
{
	// peripheral (HB6 non-transparent) only
	UINT32 dTemp = 0;

	if (!bIsPeripheral ())
		return;

    // disable the downstream BAR1 window
	dTemp = PCI_READ_DWORD (gpfa, 0xEC);
	dTemp = ~(1<<31) & dTemp;
	PCI_WRITE_DWORD(gpfa, 0xEC,dTemp);
}




/*************************************************************************
 * dSetMemWindow - Set up PCI-PCI bridge to access another board
 *
 * Input : dAddress		- PCI address of other board
 *		   dLen			- Size of window needed
 *
 */
UINT32 dSetMemWindow (UINT32 dAddress, UINT32 dLen)
{
#ifdef DEBUG_SOAK
	UINT8 buffer[64];
#endif

	UINT32 dTemp = 0;
	/* peripheral (HB6 non-transparent) only */

	if (!bIsPeripheral ())
		return dAddress;

	dBar = PCI_READ_DWORD (gpfa, 0x58);

	dTemp = PCI_READ_DWORD (gpfa, 0xEC);
	dTemp = ~(1<<31) & dTemp;
	PCI_WRITE_DWORD(gpfa, 0xEC,dTemp);

	PCI_WRITE_DWORD(gpfa, 0xE8, dAddress);
	PCI_WRITE_DWORD(gpfa, 0xEC, 0xFC000000);

	/* re-write BAR because top bit will be reset when image is disabled */
	PCI_WRITE_DWORD (gpfa, 0x58,dBar);

	return dBar;
}


void vSetMemAccess (UINT32 dAddr)
{
	UINT32	dTemp = 0;

	if (!bIsPeripheral ())
		return;

	/* make sure downstream image is disabled */

	vDisableMemWindow ();

	/* disable upstream BAR1 window */
	dTemp = PCI_READ_DWORD (gpfa, 0xAC);
	dTemp = ~(1<<31) & dTemp;
	PCI_WRITE_DWORD(gpfa, 0xAC,dTemp);
	PCI_WRITE_DWORD(gpfa, 0xA8,dAddr);
	PCI_WRITE_DWORD(gpfa, 0xAC, (1<<31)|(PCI_READ_DWORD(gpfa, 0xAC)));
}


/*************************************************************************
 * dPrimaryBusAddr - Return PCI address of dual port memory
 *
 * Output : PCI address used by master to access the local memory.
 */
UINT32 dPrimaryBusAddr (void)
{
	if (!bIsPeripheral ())
		return 0;

	return (PCI_READ_DWORD (gpfa, 0x58));
}



