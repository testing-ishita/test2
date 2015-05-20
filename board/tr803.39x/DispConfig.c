 
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

/*
 * DispConfig.c - Display Board Configuration
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/tr803.39x/DispConfig.c,v 1.1 2015-01-29 11:07:37 mgostling Exp $
 * $Log: DispConfig.c,v $
 * Revision 1.1  2015-01-29 11:07:37  mgostling
 * Import files into new source repository.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011/08/26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>	
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <bit/board_service.h>

#include <private/sys_delay.h>
#include "version.h"
#include "cctboard.h"


/* defines */
/*defines*/
#define I7_DEV   0
#define I7_VID   0x8086
#define I7_DID1  0x0044    /*host bridge*/
#define I7_DID2  0x0104    /*host bridge*/



#define  TOLUD        0xB0  /*top of low usable DRAM*/
#define  TOUUD        0xA2  /*top of upper usable DRAM*/
#define	SIZE_1MB		0x0100000L

#define TOLUD_SANDYBRIDGE  0xBC
#define TOUUD_SANDYBRIDGE  0xA8

//#define DEBUG
#define SCR_ERROR		(E__BIT + 0x1000)
#define  GPIO_BASE	    0x48
#define LPC_BRIDGE_DEV	31

#define GP_LVL          0x0C
#define GP_LVL2         0x38
/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT8	bCpuCount;
extern void 	vGetEthMacAddr(	UINT8  bInstance, UINT8* pbAddress);

/* forward declarations */


static PTR48 tPtr1;
static volatile UINT32 mapRegSpace=0;
static UINT32 mHandle;
static void vI7_MemSize_Sandybridge(UINT32* pdMemSize)
{
    UINT32 TOLUD_bRegVal;
    UINT32 TOUUD_bRegVal_Lower;
    UINT32 TOUUD_bRegVal_Higher=0;
    UINT32  g4=0xFFFFFFFFL;
    UINT32 PCI_addr;
    PCI_PFA	pfa;


	pfa = PCI_MAKE_PFA (0, 0, 0);

    /*read the TOLUD register values*/
    TOLUD_bRegVal = (UINT32)dPciReadReg(pfa,TOLUD_SANDYBRIDGE,REG_32);

     /*read the TOUUD lower register values*/
    TOUUD_bRegVal_Lower = (UINT32)dPciReadReg(pfa,TOUUD_SANDYBRIDGE,REG_32);

     /*read the TOUUD upper register values*/
     TOUUD_bRegVal_Higher = (UINT32)dPciReadReg(pfa,(TOUUD+4),REG_32);

     PCI_addr = g4- TOLUD_bRegVal+1;

     if(  TOUUD_bRegVal_Higher != 0  )
     {
     			*pdMemSize=  TOUUD_bRegVal_Higher * 4096;
     }
     else if ( TOUUD_bRegVal_Higher == 0 && TOUUD_bRegVal_Lower != 0 )
     {
     		  *pdMemSize = (TOUUD_bRegVal_Lower - PCI_addr)/0x100000;
     }
     else
     {
     		  *pdMemSize =  TOLUD_bRegVal/0x100000;
     }
 }

/*****************************************************************************
 * TEST_INTERFACE: Display board configuration
 * RETURNS: None
 * */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	temp32, addr, rt = E__OK;
	UINT16	mem;
	UINT8	reg;
	char	achBuffer[80];	/* text formatting buffer */
	PCI_PFA	pfa;
	UINT32 dMem = 0,dTemp = 0;
	UINT16 wGpioBase = 0;
	UINT8 bVal = 0;
	UINT8 bLanAddress[6];


	pfa = PCI_MAKE_PFA (0, 0, 0);
	temp32 = dPciReadReg(pfa, 0x00, REG_32);

	if( temp32 == 0x2a408086 )
	{		
		vConsoleWrite("GS45\n");				
		mem = dPciReadReg (pfa, 0xa2, REG_16)/1024;
	}
	else if(temp32 == 0x00448086 || temp32 == 0x01008086)
	{
		vConsoleWrite("Ibex Peak (or) Sandy Bridge\n");
		addr = dPciReadReg (pfa, 0x48, REG_32);
		addr &= 0xfffffff0;

		mHandle = dGetPhysPtr(addr,0x1000,&tPtr1,(void*)&mapRegSpace);

		if(mHandle == E__FAIL)
		{
			return E__FAIL;
		}

		mem =  (((*((UINT16*)(mapRegSpace+0x206))) & 0x03ff)*64)/1024;
		mem +=  (((*((UINT16*)(mapRegSpace+0x606))) & 0x03ff)*64)/1024;

		vFreePtr(mHandle);
	}
	else if (temp32 == 0x01048086)
	{
	    pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
		wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
		dTemp = dIoReadReg (wGpioBase+ GP_LVL, REG_32);
		if ( (dTemp & 0x100) == 0x100)
		{
			bVal = 0x1;
		}
		else
		{
		    bVal = 0x0;
		}

	 vI7_MemSize_Sandybridge(&dMem);
#ifdef DEBUG
	sprintf (achBuffer, "MEMORY               : %dGB\n", dMem/1024);
	vConsoleWrite (achBuffer);
#endif
		switch (bVal)
		{
			case 0:
				dMem = dMem/1024;
#ifdef DEBUG
				vConsoleWrite("8GB Fitted\n");
#endif
				mem = 8;
				break;
			case 1:
				dMem = dMem/1024;
#ifdef DEBUG
				vConsoleWrite("4GB Fitted\n");
#endif
				mem = 4;
				break;
			default:
				dMem = 0;
				mem = 0;
#ifdef DEBUG
				vConsoleWrite("0GB Fitted\n");
#endif
				break;
		}
	}
	else
	{
		vConsoleWrite("other\n");
		mem = PCI_READ_WORD (pfa, 0xcc);	
		mem = mem & 0x1ff;
		mem = (mem * 128)/1024;
	}

	sprintf (achBuffer, "Num CPUs               : %d\n", (int)bCpuCount);
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "CPU Frequency          : %i.%02iGHz\n", sysGetCpuFrequency () / 1000,
															 (sysGetCpuFrequency () % 1000) / 10);
	vConsoleWrite (achBuffer);	
	sprintf (achBuffer, "Fitted RAM             : %dGB\n", mem);
	vConsoleWrite (achBuffer);	

	if (dMem == mem)
	{
		sprintf (achBuffer, "Memory Detected        : Valid\n");
		vConsoleWrite (achBuffer);
	}
	else
	{
		sprintf (achBuffer, "Memory Detected        : In-Valid\n");
		vConsoleWrite (achBuffer);
	}


	reg = ((UINT8)dIoReadReg(0x210, REG_8));
	sprintf (achBuffer, "Hardware Revision      : %c\n", ('A' +  ((reg >> 5)  & 0x07)) );
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "CUTE Mode              : %s\n", ((reg & 0x01) == 0x01?"MTH":"STH"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Mode                   : %s\n", ((reg & 0x02) == 0x01?"BIOS":"CUTE"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Console Select         : %s\n", ((reg & 0x08) == 0x08?"Console":"DVI"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "NVMRO Jumper           : %s\n", ((reg & 0x10) == 0x10?"Fitted":"Not Fitted"));
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x211, REG_8));
	sprintf (achBuffer, "User LED               : %s\n", ((reg & 0x01) == 0x01?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "NAND Flash WriteProtect: %s\n", ((reg & 0x02) == 0x02?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "NVMRO Signal Status    : %s\n", ((reg & 0x20) == 0x20?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC  Present           : %s\n", ((reg & 0x40) == 0x40?"Yes":"NO"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x212, REG_8));
	sprintf (achBuffer, "IPMI NMI Flag          : %s\n", ((reg & 0x20) == 0x20?"Asserted":"False"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VPX SYS Reset Enable   : %s\n", ((reg & 0x80) == 0x80?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x215, REG_8));
	sprintf (achBuffer, "SMIC Not Busy Interrupt: %s\n", ((reg & 0x01) == 0x01?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "SMS ATN Interrupt      : %s\n", ((reg & 0x02) == 0x02?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "GP Interrupt           : %s\n", ((reg & 0x04) == 0x04?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "IPMI Mode              : %s\n", ((reg & 0x80) == 0x80?"ISP":"Normal"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x31E, REG_8));
	sprintf (achBuffer, "COM2 UART Mode         : %s\n", ((reg & 0x01) == 0x01?"RS485":"RS232"));
	vConsoleWrite (achBuffer);
	if ((reg & 0x01) == 0x01)
	{
		sprintf (achBuffer, "COM2 RS485 Mode    : %s\n", ((reg & 0x02) == 0x02?"Half Duplex":"Full Duplex"));
		vConsoleWrite (achBuffer);
	}
	sprintf (achBuffer, "Board Mode             : %s\n", ((reg & 0x04) == 0x04?"System Controller":"Peripheral"));
	vConsoleWrite (achBuffer);

	/*
	 *  Lock again the registers 0x31x register.
	 */

	vConsoleWrite ("\nETHERNET\n-------------------\n");
	vGetEthMacAddr(1, bLanAddress);
	sprintf (achBuffer, "Ethernet 1 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

	vGetEthMacAddr(2, bLanAddress);
	sprintf (achBuffer, "Ethernet 2 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

	vGetEthMacAddr(3, bLanAddress);
	sprintf (achBuffer, "Ethernet 3 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

	vGetEthMacAddr(4, bLanAddress);
	sprintf (achBuffer, "Ethernet 4 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

	return rt;
}


void brdCuteInfo(brd_info *info)
{
	UINT8	reg;
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	info->memory = dPciReadReg (pfa, 0xB0, REG_16);

	reg = ((UINT8)dIoReadReg(0x210, REG_8));
	info->HardwareRev = ('A' + ((reg >> 5) & 0x07));

	info->firmwareVersion = FW_VERSION_NUMBER;
}



