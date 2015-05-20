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
 * vpxArch.c
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/arch/vpx/vpxArch.c,v 1.2 2013-11-25 10:23:24 mgostling Exp $
 * $Log: vpxArch.c,v $
 * Revision 1.2  2013-11-25 10:23:24  mgostling
 * Added support for PLX PEX8717
 *
 *
 *  Created on: 19 Apr 2010
 *      Author: engineer
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

#include <private/cpu.h>
#include <private/debug.h>
#include <private/port_io.h>

#include <cute/arch.h>
#include <cute/packet.h>
#include <cute/interconnect.h>
#include <bit/board_service.h>

//#define DEBUG_SOAK
#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

/* globals */
extern sSlave	asSlots[MAX_SLAVES];
extern UINT8	abSlaveDev[MAX_SLOTS];
extern UINT8	bSlaves;
extern UINT8 	my_slot;

/* externals */
extern UINT8   bControllerType;


/* locals */
static PCI_PFA gpfa;
static UINT32	dBar;				/* saved BAR for HB6 downstream image */

static volatile UINT32 	mapRegSpace1;		//PCI registers
static UINT32 dGlobalBarAddr;


struct s_slotInfo {
	UINT16 wAddr;
	char	achName[10];
};


void vpxPreInit (void)
{
	board_service(SERVICE__BRD_CONFIG_VPX_BACKPLANE,NULL,NULL);

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
		//sysDebugWriteString ("bReadIcByte MASTER\n");
		dRegOffset = bReg ;

		return (vReadByte((0x20000+dRegOffset)));
	}
	else
	{
		//sysDebugWriteString ("bReadIcByte SLAVE\n");
	    dRegOffset = (bReg ) + asSlots[bSlot].dBar;

		return (vReadByte(dRegOffset));
	}

} /* bReadIcByte */


void vWriteIcByte (UINT8 bSlot, UINT8 bReg, UINT8 bData)
{
	UINT32 dRegOffset;

	/* Master mode */
	if ((bSlot == my_slot) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		dRegOffset = bReg ;
		//sysDebugWriteString ("vWriteIcByte MASTER\n");
		vWriteByte((0x20000+dRegOffset), (bData));
	}
	else/* Slave mode */
	{
		dRegOffset = (bReg ) + asSlots[bSlot].dBar;
		//sysDebugWriteString ("vWriteIcByte SLAVE\n");
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
		//sysDebugWriteString ("bReadIcByte MASTER\n");
		dRegOffset = bReg ;

		return (vReadByte((0x20000+dRegOffset)));
	}
	else
	{
		//sysDebugWriteString ("bReadIcByte SLAVE\n");
	    dRegOffset = (bReg ) + asSlots[bSlot].dBar;

		return (vReadByte(dRegOffset));
	}

} /* bReadIcByte */


void vWriteIcReg (UINT8 bSlot, UINT8 bReg, UINT8 bData)
{
	UINT32 dRegOffset;

	/* Master mode */
	if ((bSlot == my_slot) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		dRegOffset = bReg ;
		//sysDebugWriteString ("vWriteIcByte MASTER\n");
		vWriteByte((0x20000+dRegOffset), (bData));
	}
	else/* Slave mode */
	{
		dRegOffset = (bReg ) + asSlots[bSlot].dBar;
		//sysDebugWriteString ("vWriteIcByte SLAVE\n");
		vWriteByte((dRegOffset), (bData));
	}
	


} /* vWriteIcByte */

/****************************************************************************
 * vCfgHbEnd: board-specific host bridge configuration
 *
 * RETURNS: none
 */
void vCfgHbEnd(	UINT8 bHostBridge, UINT8 bSubBus, UINT32 dMemAddr, UINT16 wIoAddr)
{
	UINT16	wCntr;              /* general word-wide counter 			*/
	UINT8	bMemLow;            /* low and high memory size components  */
	UINT8	bMemHi;             /*                  -"-                 */
	UINT8	bMemSizeLow;        /* low and high dp ram 					*/
	UINT8	bMemSizeHi;         /*        			-"-          		*/
	UINT8	bTemp;
	UINT32	dMemSize;

    PCI_PFA pfa;

   	pfa = PCI_MAKE_PFA(0,0x0,0);
	bTemp = PCI_READ_BYTE (pfa, 0xA0);
	dMemSize = bTemp * 0x800;

	dMemAddr &= 0xf8000000;	/* 128 MB alignment */
	dMemAddr = dMemAddr >> 16;

	if (dMemSize > dMemAddr)
		dMemSize = dMemAddr;

	/* write the TOLM register */
	PCI_WRITE_WORD (pfa, 0xB0, (UINT16)(dMemSize));
	/* adjust interconnect image memory regs */

	/* adjust interconnect image memory regs */
	wCntr = 0x40;

	/* Fill in upper memory limit */
	/* Find the number of 64MB/128MB blocks on board,
	 * we report number of 64kByte - 1, blocks through interconnect
	 */

	bMemSizeHi  = dMemSize >> 8;
	bMemSizeLow = dMemSize & 0xFF;

	dMemSize--;

	bMemHi  = dMemSize >> 8;
	bMemLow = dMemSize & 255;

	while (bTemp != LOCAL_MEMORY_RECORD)
	{
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2));
		wCntr += (bTemp * 2) + 4;
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	}

	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(END_ADDRESS_LOW  * 2)), bMemLow);
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(END_ADDRESS_HIGH * 2)), bMemHi );

	/* Find the memory record and insert the memory end address.  */
	bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	while (bTemp != MEMORY_RECORD)
	{
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2));
		wCntr += (bTemp * 2) + 4;
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	}
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(MEM_SIZE_LOW  * 2)), bMemSizeLow);
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(MEM_SIZE_HIGH * 2)), bMemSizeHi);

} /* vCfgHbEnd () */




/*************************************************************************
 * bIsPeripheral - Returns TRUE if board is in peripheral mode
 *
 */
UINT8 bIsPeripheral (void)
{
	UINT8 bPeri;

	bPeri = dIoReadReg(0x31E, REG_8);

	if ( (bPeri & 0x04) != 0x04)
		return TRUE;
	else
		return FALSE;
}



/*************************************************************************
 * vVpxInit - Find PCI-PCI bridge and save its address
 *
 */
void vVpxInit (void)
{
	UINT32 dInstance = 0;
	memset (&gpfa, 0, sizeof(gpfa));

	if (bIsPeripheral ())
	{
		/*iPciFindDeviceById (3,0x10B5,0x8619,&gpfa);*/ /*Hari*/

		board_service(SERVICE__BRD_GET_VPX_INSTANCE,NULL,&dInstance);

		if (iPciFindDeviceById (dInstance,0x10B5,0x8619,&gpfa) == E__OK)
		{
			dGlobalBarAddr = PCI_READ_DWORD (gpfa, 0x10);
    		dGlobalBarAddr &= ~(0x0000000f);

		    mapRegSpace1 =  sysMemGetPhysPtrLocked((UINT64)dGlobalBarAddr, 0x20000);
			if (mapRegSpace1 == 0)
			{
				return;
			}
		}
		else if (iPciFindDeviceById (dInstance,0x111d,0x808C,&gpfa) == E__OK)
		{
		     dGlobalBarAddr = 0xe0101000;
     		 mapRegSpace1 =  sysMemGetPhysPtrLocked((UINT64)dGlobalBarAddr, 0x20000);
			 if (mapRegSpace1 == 0)
			 {
					return;
			 }
		}
		else if (iPciFindDeviceById (dInstance, 0x10B5, 0x8717, &gpfa) == E__OK)
		{
            dGlobalBarAddr = PCI_READ_DWORD (gpfa, 0x10);
            dGlobalBarAddr &= ~(0x0000000f);

            mapRegSpace1 =  sysMemGetPhysPtrLocked((UINT64)dGlobalBarAddr, 0x40000);
            if (mapRegSpace1 == 0)
            {
                return;
            }
        }
	}
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
	UINT16 wDevID = 0;
#ifdef DEBUG_SOAK
	UINT8 buffer[64];
#endif
	/* peripheral (HB6 non-transparent) only */

	if (!bIsPeripheral ())
		return dAddress;

	wDevID = vReadWord(mapRegSpace1+0x02);

	if (wDevID == 0x8619)
	{
 	 		dBar = vReadDWord(mapRegSpace1+0x11000+0x1c);
		#ifdef DEBUG_SOAK
			sprintf(buffer,"vVpxInit:dBar  : %x\n", dBar);
			sysDebugWriteString(buffer);
		#endif
    
			vWriteDWord(mapRegSpace1+0x11000+0xEC,(~(1<<31)) & (vReadDWord(mapRegSpace1+0x11000+0xEC)));
		#ifdef DEBUG_SOAK
			sprintf(buffer, "0xEC : %x\n", vReadDWord(mapRegSpace1+0x11000+0xEC));
			sysDebugWriteString(buffer);
		#endif
			vWriteDWord(mapRegSpace1+0x11000+0xC40,dAddress);
		#ifdef DEBUG_SOAK
			sprintf(buffer, "0xC40 : %x\n", vReadDWord(mapRegSpace1+0x11000+0xC40));
			sysDebugWriteString(buffer);
		#endif
			vWriteDWord(mapRegSpace1+0x11000+0xEC,0xFFC00000);
		#ifdef DEBUG_SOAK
			sprintf(buffer, "0xEC : %x\n", vReadDWord(mapRegSpace1+0x11000+0xEC));
			sysDebugWriteString(buffer);
		#endif
			vWriteDWord(mapRegSpace1+0x11000+0xEC,((1<<31)) | (vReadDWord(mapRegSpace1+0x11000+0xEC)));
		#ifdef DEBUG_SOAK
			sprintf(buffer, "0xEC : %x\n", vReadDWord(mapRegSpace1+0x11000+0xEC));
			sysDebugWriteString(buffer);
		#endif
			/* re-write BAR because top bit will be reset when image is disabled */
			vWriteDWord(mapRegSpace1+0x11000+0x1c,dBar);
    
			/* enable the LUT Tables */
			vWriteDWord(mapRegSpace1+0x11000+0xDB4,0x00010001);
	}
	else if (wDevID == 0x8717)
    {
        //dBar = dGlobalBarAddr;
                dBar = vReadDWord(mapRegSpace1+0x3F000+0x1c);
        #ifdef DEBUG_SOAK
                sprintf(buffer,"vVpxInit:dBar  : %x\n", dBar);
                sysDebugWriteString(buffer);
        #endif
    
                vWriteDWord(mapRegSpace1+0x3F000+0xEC,(~(1<<31)) & (vReadDWord(mapRegSpace1+0x3F000+0xEC)));
        #ifdef DEBUG_SOAK
                sprintf(buffer, "0xEC : %x\n", vReadDWord(mapRegSpace1+0x3F000+0xEC));
                sysDebugWriteString(buffer);
        #endif
                vWriteDWord(mapRegSpace1+0x3F000+0xC40,dAddress);
        #ifdef DEBUG_SOAK
                sprintf(buffer, "0xC40 : %x\n", vReadDWord(mapRegSpace1+0x3F000+0xC40));
                sysDebugWriteString(buffer);
        #endif
                vWriteDWord(mapRegSpace1+0x3F000+0xEC,0xFFC00000);
        #ifdef DEBUG_SOAK
                sprintf(buffer, "0xEC : %x\n", vReadDWord(mapRegSpace1+0x3F000+0xEC));
                sysDebugWriteString(buffer);
        #endif
                vWriteDWord(mapRegSpace1+0x3F000+0xEC,((1<<31)) | (vReadDWord(mapRegSpace1+0x3F000+0xEC)));
        #ifdef DEBUG_SOAK
                sprintf(buffer, "0xEC : %x\n", vReadDWord(mapRegSpace1+0x3F000+0xEC));
                sysDebugWriteString(buffer);
        #endif
                /* re-write BAR because top bit will be reset when image is disabled */
                vWriteDWord(mapRegSpace1+0x3F000+0x1c,dBar);
    
                /* enable the LUT Tables */
                vWriteDWord(mapRegSpace1+0x3E000+0xD94,0x01010001);
                vWriteDWord(mapRegSpace1+0x3F000+0xDB4,0x01010001);
    }
	else if (wDevID == 0x808C)
	{

		vWriteDWord(mapRegSpace1+0xff8,0x21018);
		dBar = vReadDWord(mapRegSpace1+0xffc);

		#ifdef DEBUG_SOAK
			sprintf(buffer,"vVpxInit:dBar  : %x\n", dBar);
			sysDebugWriteString(buffer);
		#endif

			vWriteDWord(mapRegSpace1+0xff8,0x21490);
			vWriteDWord(mapRegSpace1+0xffc,(~(1<<31)) & (vReadDWord(mapRegSpace1+0xffc)));

		#ifdef DEBUG_SOAK
			sprintf(buffer, "0x490 : %x\n", (vReadDWord(mapRegSpace1+0xffc)));
			sysDebugWriteString(buffer);
		#endif
			vWriteDWord(mapRegSpace1+0xff8,0x21498);
			vWriteDWord(mapRegSpace1+0xffc,dAddress);
		#ifdef DEBUG_SOAK
			sprintf(buffer, "0x498 : %x\n", vReadDWord(mapRegSpace1+0xffc));
			sysDebugWriteString(buffer);
		#endif
			vWriteDWord(mapRegSpace1+0xff8,0x21494);
			vWriteDWord(mapRegSpace1+0xffc,0xFFC00000);
		#ifdef DEBUG_SOAK
			sprintf(buffer, "0x494 : %x\n",vReadDWord(mapRegSpace1+0xffc));
			sysDebugWriteString(buffer);
		#endif
			vWriteDWord(mapRegSpace1+0xff8,0x21490);
			vWriteDWord(mapRegSpace1+0xffc,((1<<31)) | (vReadDWord(mapRegSpace1+0xffc)));
		#ifdef DEBUG_SOAK
			sprintf(buffer, "0x490 : %x\n", vReadDWord(mapRegSpace1+0x21000+0x490));
			sysDebugWriteString(buffer);
		#endif
			/* re-write BAR because top bit will be reset when image is disabled */
			vWriteDWord(mapRegSpace1+0xff8,0x21018);
			vWriteDWord(mapRegSpace1+0xffc,dBar);
			dBar = vReadDWord(mapRegSpace1+0xffc);

			/* enable the NT Mapping Table address and data */
			vWriteDWord(mapRegSpace1+0x4D0,0x00000002);
			vWriteDWord(mapRegSpace1+0x4D8,0x00020001);
	}

	return dBar;
}




