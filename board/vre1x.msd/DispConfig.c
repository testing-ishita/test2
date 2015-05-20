 
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/DispConfig.c,v 1.3 2014-08-04 08:40:21 mgostling Exp $
 * $Log: DispConfig.c,v $
 * Revision 1.3  2014-08-04 08:40:21  mgostling
 * Return correct memory size from TOLUD register.
 *
 * Revision 1.2  2014-04-22 16:43:50  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. 
   This tag was also designed to allow search+sort operations that output a list of functions and test names; 
   if the tag is used in comments then we get spurious lines included in this list.
 *
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
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
#include "config.h"

/* defines */
//#define DEBUG

/*defines*/

#define	SIZE_1MB		0x0100000L

#define TOLUD_HASWELL	0xBC
#define TOUUD_HASWELL	0xA8

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

extern UINT8 bCpuCount;
extern void vGetEthMacAddr(	UINT8  bInstance, UINT8* pbAddress, UINT8 bMode);
#ifdef INCLUDE_NETWORK
extern int geifGetMacAddr( char *pName, UINT8 *pMacAddr );
#endif

/* forward declarations */


static volatile UINT32 mapRegSpace=0;

static void vI7_MemSize_Haswell(UINT32* pdMemSize)
{
    UINT32 TOLUD_bRegVal;
    UINT32 TOUUD_bRegVal_Lower;
    UINT32 TOUUD_bRegVal_Higher=0;
    UINT32 g4=0xFFFFFFFFL;
    UINT32 PCI_addr;
    PCI_PFA	pfa;


	pfa = PCI_MAKE_PFA (0, 0, 0);

    /*read the TOLUD register values*/
    TOLUD_bRegVal = (UINT32)dPciReadReg(pfa,TOLUD_HASWELL,REG_32);

     /*read the TOUUD lower register values*/
    TOUUD_bRegVal_Lower = (UINT32)dPciReadReg(pfa,TOUUD_HASWELL,REG_32);

     /*read the TOUUD upper register values*/
     TOUUD_bRegVal_Higher = (UINT32)dPciReadReg(pfa,(TOUUD_HASWELL + 4),REG_32);

     PCI_addr = g4 - TOLUD_bRegVal + 1;

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
 * DispConfig: Display board configuration
 * RETURNS: None
 * */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	rt = E__OK;
	UINT16	mem;
	UINT8	reg;
	char	achBuffer[80];	/* text formatting buffer */
	PCI_PFA	pfa;
	UINT32 dMem = 0, dTemp = 0;
	UINT16 wGpioBase = 0;
	UINT8 bVal = 0;
//	UINT8 bLanAddress[6];

	pfa = PCI_MAKE_PFA (0, 0, 0);
	dTemp = dPciReadReg(pfa, 0x00, REG_32);				// vendor & product ids
	if (dTemp == 0x0C048086)
	{
	    pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
		wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
		// GPIO_16 & GPIO_15
		dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

		bVal = (UINT8)((dTemp >> 15) & 0x03);

		// GPIO_50 & GPIO_52
		dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);
		dTemp >>= 16;
		bVal |= (UINT8)(((dTemp & 0x10) == 0 ? 0 : 0x04) + ((dTemp & 0x04) == 0 ? 0 : 0x08));

		// obtain ram size in GB from IO strap settings
		// output parameter in bVal is GPIO strap setting
		// returned parameter in bVal is memory size
		board_service (SERVICE__BRD_GET_DRAM_SIZE_FROM_GPIO, NULL, (void *)&bVal);

		vI7_MemSize_Haswell(&dMem);

#ifdef DEBUG
		sprintf (achBuffer, "MEMORY                  : %dGB\n", dMem/1024);
		vConsoleWrite (achBuffer);
#endif
		switch (bVal)
		{
			case 4:
			case 8:
			case 16:
			case 32:
				dMem = dMem/1024;
				mem = bVal;
#ifdef DEBUG
				sprintf (achBuffer, "%dGB Fitted\n", mem);
				vConsoleWrite (achBuffer);
#endif
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
		mem = 0;
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
	}
	else
	{
		sprintf (achBuffer, "Memory Detected        : In-Valid\n");
	}
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x210, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 210 = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	sprintf (achBuffer, "Board Mode             : %s\n", ((reg & 0x80) == 0x80?"Peripheral":"System Controller"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Flash Bank             : %s\n", ((reg & 0x40) == 0x40?"Device 2":"Device 1"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "COM2 Routing           : %s\n", ((reg & 0x20) == 0x20?"Front":"Rear"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VPX NVMRO Status       : %s\n", ((reg & 0x10) == 0x10?"Active":"Inactive"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "User LED               : %s\n", ((reg & 0x08) == 0x08?"On":"Off"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Hardware Revision      : %c\n", ('A' +  (reg & 0x07)));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x211, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 211 = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	sprintf (achBuffer, "User LED Function      : %s\n", ((reg & 0x80) == 0x80?"Processor Hot":"User LED"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Serial Console Port    : %s\n", ((reg & 0x20) == 0x20?"COM2":"COM1"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "PMC1 High Speed        : %s\n", ((reg & 0x10) == 0x10?"Higher Speed":"Lower Speed"));
	vConsoleWrite (achBuffer);
	switch (reg & 0x0C)
	{
		case 0x00:
			sprintf (achBuffer, "PMC1 Speed             : %s\n", "PCI 25MHz or 33MHz");
			break;

		case 0x04:
			sprintf (achBuffer, "PMC1 Speed             : %s\n", "PCI 50MHz or 66MHz");
			break;

		case 0x08:
			sprintf (achBuffer, "PMC1 Speed             : %s\n", "PCI-X 50MHz or 66MHz");
			break;

		case 0x0C:
			sprintf (achBuffer, "PMC1 Speed             : %s\n", "PCI-X 100MHz");
			break;
	}
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VPX SYS Reset Enable   : %s\n", ((reg & 0x02) == 0x02?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "PMC M66E               : %s\n", ((reg & 0x01) == 0x01?"50MHz or 66MHz":"25MHz or 33MHz"));

	reg = ((UINT8)dIoReadReg(0x212, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 212 = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	sprintf (achBuffer, "XMC2 Fitted            : %s\n", ((reg & 0x80) == 0x80?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC1 Fitted            : %s\n", ((reg & 0x40) == 0x40?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Micro-Controller NMI   : %s\n", ((reg & 0x10) == 0x10?"Asserted":"False"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "INIT Done              : %s\n", ((reg & 0x04) == 0x04?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "PMC2 Fitted            : %s\n", ((reg & 0x02) == 0x02?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "PMC1 Fitted            : %s\n", ((reg & 0x01) == 0x01?"Yes":"No"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x213, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 213 = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	sprintf (achBuffer, "VPX GDISC Flag         : %s\n", ((reg & 0x04) == 0x04?"Asserted":"False"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VPX GDISC Int Enable   : %s\n", ((reg & 0x02) == 0x02?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VPX GDISC Status       : %s\n", ((reg & 0x01) == 0x01?"High":"Low"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x214, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 214 = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	sprintf (achBuffer, "IPMI NMI Flag          : %s\n", ((reg & 0x20) == 0x20?"Asserted":"False"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "PLX Switch Reset       : %s\n", ((reg & 0x04) == 0x04?"Enabled":"Controlled by PCH"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "MLVDS Clock            : %s\n", ((reg & 0x01) == 0x01?"100MHz":"25MHz"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x215, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 215 = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	sprintf (achBuffer, "IPMI Mode              : %s\n", ((reg & 0x80) == 0x80?"ISP":"Normal"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "GPI Int Status         : %s\n", ((reg & 0x40) == 0x40?"Asserted":"False"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "SMS ATN Int Status     : %s\n", ((reg & 0x20) == 0x20?"Asserted":"False"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "SMIC Not Busy Status   : %s\n", ((reg & 0x10) == 0x10?"Asserted":"False"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "GPI Interrupt          : %s\n", ((reg & 0x04) == 0x04?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "SMS ATN Interrupt      : %s\n", ((reg & 0x02) == 0x02?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "SMIC Not Busy Interrupt: %s\n", ((reg & 0x01) == 0x01?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x216, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 216 = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	
	sprintf (achBuffer, "CPU temperature        : %d\n", (INT8)reg);
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x217, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 217 = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	
	sprintf (achBuffer, "Ambient temperature    : %d\n", (INT8)reg);
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x31D, REG_8));
#ifdef DEBUG
	sprintf (achBuffer, "\nRegister 31D = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
#endif
	sprintf (achBuffer, "COM1 UART              : %s\n", ((reg & 0x40) == 0x40?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "COM1 UART Mode         : %s\n", ((reg & 0x08) == 0x08?"RS485":"RS232"));
	vConsoleWrite (achBuffer);
	if ((reg & 0x08) == 0x08)
	{
		sprintf (achBuffer, "COM1 Mode              : %s\n", ((reg & 0x04) == 0x04?"Half Duplex":"Full Duplex"));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "COM1 TX Control        : %s\n", ((reg & 0x02) == 0x02?"RTS":"DTR"));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "COM1 Slew Rate         : %s\n", ((reg & 0x01) == 0x01?"Fast":"Normal"));
		vConsoleWrite (achBuffer);
	}

//#ifdef DEBUG
	reg = ((UINT8)dIoReadReg(0x31F, REG_8));
	sprintf (achBuffer, "\nRegister 31F = %02X\n\n", reg);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "FPGA Version           : %d\n", (INT8)reg);
	vConsoleWrite (achBuffer);
//#endif

	/*
	 *  Lock again the registers 0x31x register.
	 */
#if 0
	vConsoleWrite ("\nETHERNET\n-------------------\n");
#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	       geifGetMacAddr( "ge0", bLanAddress );
#else
		   vGetEthMacAddr(1, bLanAddress, ETH_MAC_DISP_MODE);
#endif
	sprintf (achBuffer, "Ethernet 1 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	       geifGetMacAddr( "ge1", bLanAddress );
#else
	  	   vGetEthMacAddr(2, bLanAddress, ETH_MAC_DISP_MODE);
#endif
	sprintf (achBuffer, "Ethernet 2 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	       geifGetMacAddr( "ge2", bLanAddress );
#else
		   vGetEthMacAddr(3, bLanAddress,ETH_MAC_DISP_MODE);
#endif
	sprintf (achBuffer, "Ethernet 3 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	       geifGetMacAddr( "ge0", bLanAddress );
#else
		   vGetEthMacAddr(4, bLanAddress, ETH_MAC_DISP_MODE);
#endif
	sprintf (achBuffer, "Ethernet 4 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	       geifGetMacAddr( "ge1", bLanAddress );
#else
	  	   vGetEthMacAddr(5, bLanAddress, ETH_MAC_DISP_MODE);
#endif
	sprintf (achBuffer, "Ethernet 5 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	       geifGetMacAddr( "ge2", bLanAddress );
#else
		   vGetEthMacAddr(6, bLanAddress,ETH_MAC_DISP_MODE);
#endif
	sprintf (achBuffer, "Ethernet 6 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);
#endif

	return rt;
}


UINT32 brdCuteInfo(brd_info *info)
{
	UINT8	reg;
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	info->memory = (UINT16) (dPciReadReg (pfa, 0xBC, REG_32) >> 16);			// TOLUD

	reg = ((UINT8)dIoReadReg(0x210, REG_8));
	info->HardwareRev = ('A' + ((reg >> 5) & 0x07));

	info->firmwareVersion = FW_VERSION_NUMBER;

	return E__OK;
}



