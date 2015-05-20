/************************************************************************
 *                                                                      *
 *      Copyright 2013 Concurrent Technologies, all rights reserved.    *
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
 * srioArch.c - functions reporting board specific architecture
 */
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/arch/srio/srioArch.c,v 1.6 2014-10-09 10:51:59 chippisley Exp $
 *
 * $Log: srioArch.c,v $
 * Revision 1.6  2014-10-09 10:51:59  chippisley
 * Put debug error message for device not found inside DEBUG_SOAK conditional.
 *
 * Revision 1.5  2014-10-09 09:40:12  chippisley
 * Added description and comments to srioPreInit()
 *
 * Revision 1.4  2014-10-03 14:37:24  chippisley
 * Added check for Tsi721 in srioPreInit() to prevent wrong device being accessed.
 *
 * Revision 1.3  2014-08-04 15:24:58  mgostling
 * Corrected compiler warnings.
 *
 * Revision 1.2  2014-03-06 13:22:59  hchalla
 * Srio bar memory allocation is reduced from 16MB to 8MB.
 *
 * Revision 1.1  2014-01-29 13:07:35  hchalla
 * Initial SRIO Arch checkin.
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


#undef DEBUG_SOAK

#define TSI721_PCIECFG_EPCTL	0x400
#define TSI721_PCIECFG_BARSETUP0 0x440
#define TSI721_PCIECFG_BARSETUP1 0x444
#define TSI721_PCIECFG_BARSETUP2 0x448
#define TSI721_PCIECFG_BARSETUP3 0x44C
#define TSI721_PCIECFG_BARSETUP4 0x450
#define TSI721_PCIECFG_BARSETUP5 0x454
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



/*******************************************************************************
*
* srioPreInit
*
* Perform early SRIO device initialization, which must be done prior to the
* PCI device configuration.
*
*
* RETURNS: None
* 
*******************************************************************************/
void srioPreInit (UINT8 bFlag)
{
	PCI_PFA pfa;
	UINT8 bTemp;
	UINT16 wVId = 0, wDId=0;	
#ifdef DEBUG_SOAK
	char buffer[80];
#endif


	if (bFlag == 1)
	{
		board_service(SERVICE__BRD_CONFIG_SRIO_DEVICE,NULL,&pfa);
	}
	else
	{
		board_service(SERVICE__BRD_CONFIG_SRIO_DEVICE_2,NULL,&pfa);
	}
	
#ifdef DEBUG_SOAK
	sprintf(buffer,"%s: %03u:%02u:%02u\n",
			__FUNCTION__, PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa));
	sysDebugWriteString(buffer);
#endif

	wVId = PCI_READ_WORD(pfa,0x00);
	wDId = PCI_READ_WORD(pfa,0x02);

#ifdef DEBUG_SOAK
	sprintf(buffer,"SRIO TSI721 wVId:%#x wDId:%#x\n",wVId, wDId);
	sysDebugWriteString(buffer);
#endif

	// Make sure we're going to access right device (Tsi721)
	if ( (wVId == 0x111d) && (wDId == 0x80ab) )
	{
		bTemp = PCI_READ_BYTE (pfa, 0x04);
		bTemp |= 0x07;
		PCI_WRITE_BYTE (pfa, 0x04, bTemp);
	
		// Configure the BAR Setup Registers, setting size and enable/disable.
		// This must be done prior to PCI device configuration where the actual
		// BAR memory will be allocated and configured
		PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_BARSETUP0,(UINT32)0x80000130);	/* memory mapped registers */		
		PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_BARSETUP1,(UINT32)0x00000000);
		PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_BARSETUP2,(UINT32)0x80000178); 	/* prefetchable memory */
		PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_BARSETUP3,(UINT32)0x00000000);
		PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_BARSETUP4,(UINT32)0x00000000); 	/* non prefetchable memory (not used) */
		PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_BARSETUP5,(UINT32)0x00000000);
		
#ifdef DEBUG_SOAK
		sprintf(buffer,"TSI721_PCIECFG_BARSETUP0:%08x\n",PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP0));
		sysDebugWriteString(buffer);
		
		sprintf(buffer,"TSI721_PCIECFG_BARSETUP1:%08x\n",PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP1));
		sysDebugWriteString(buffer);
		
		sprintf(buffer,"TSI721_PCIECFG_BARSETUP2:%08x\n",PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP2));	 
		sysDebugWriteString(buffer);
		
		sprintf(buffer,"TSI721_PCIECFG_BARSETUP3:%08x\n",PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP3));	 
		sysDebugWriteString(buffer);
		
		sprintf(buffer,"TSI721_PCIECFG_BARSETUP4:%08x\n",PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP4));	 
		sysDebugWriteString(buffer);
		
		sprintf(buffer,"TSI721_PCIECFG_BARSETUP5:%08x\n",PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP5));	 
		sysDebugWriteString(buffer);
#endif

		if (bFlag == 1)
		{
			board_service(SERVICE__BRD_DECONFIG_SRIO_DEVICE,NULL,NULL);
		}
		else
		{
			board_service(SERVICE__BRD_DECONFIG_SRIO_DEVICE_2,NULL,NULL);
		}
	}
#ifdef DEBUG_SOAK
	else
	{
		sysDebugWriteString("SRIO preinit: TSI721 not found\n");
	}
#endif

} /* srioPreInit () */


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

