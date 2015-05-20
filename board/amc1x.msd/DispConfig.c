/************************************************************************
 *                                                                      *
 *      Copyright 2014 Concurrent Technologies, all rights reserved.    *
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/amc1x.msd/DispConfig.c,v 1.3 2015-02-27 16:18:04 mgostling Exp $
 * $Log: DispConfig.c,v $
 * Revision 1.3  2015-02-27 16:18:04  mgostling
 * Use board specific GPIO strap readings to detect RAM size
 *
 * Revision 1.2  2014-10-15 11:46:47  mgostling
 * Fixed compiler warning.
 *
 * Revision 1.1  2014-09-19 10:41:42  mgostling
 * Initial check in to CVS
 *
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
#include "services.h"
#include "lwip/netif.h"

/* defines */

//#define DEBUG
#define SCR_ERROR		(E__BIT + 0x1000)

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

// already defined via inclusions from lwip/netif.h
// #define INCLUDE_NETWORK 1

extern UINT8	bCpuCount;
extern void 	vGetEthMacAddr(	UINT8  bInstance, UINT8* pbAddress);
#ifdef INCLUDE_NETWORK
extern int geifGetMacAddr( char *pName, UINT8 *pMacAddr );
#endif

/* forward declarations */



static volatile UINT32 mapRegSpace=0;

static void vI7_MemSize_Haswell(UINT32* pdMemSize)
{
	UINT32	TOLUD_bRegVal;
	UINT32	TOUUD_bRegVal_Lower;
	UINT32	TOUUD_bRegVal_Higher=0;
	UINT32	g4=0xFFFFFFFFL;
	UINT32	PCI_addr;
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
	PCI_PFA	pfa;
	char	achBuffer[80];
	UINT32	dMem = 0, dTemp = 0;
	UINT16	wGpioBase = 0;
	UINT8	bVal = 0;
	UINT8	reg;
#if 0
	UINT8	bLanAddress[6];

#ifdef INCLUDE_NETWORK
	struct netif *pNetif;
#endif
#endif


	pfa = PCI_MAKE_PFA (0, 0, 0);
	dTemp = dPciReadReg(pfa, 0x00, REG_32);				// vendor & product ids
	if (dTemp == 0x0C048086)
	{
		// obtain ram size in GB from IO strap settings
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

	sprintf (achBuffer, "Num CPUs               : %d\n", (int) bCpuCount);
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "CPU Frequency          : %i.%02iGHz\n", sysGetCpuFrequency() / 1000,(sysGetCpuFrequency() % 1000) / 10);
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


	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));
	sprintf (achBuffer, "Hardware Revision      : %c\n", ('A' + ((reg >>4) & 0x03)));
	vConsoleWrite (achBuffer);

	//sprintf (achBuffer, "BIOS Defaults       : %s\n", ((reg & CMOS_RESTORE)?"Restore User Defaults":"Restore Factory Defaults"));
	//vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8));

//	sprintf (achBuffer, "Mode                   : %s\n", ((reg & CUTE_MODE) == CUTE_MODE?"CUTE":"BIOS"));
//	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "User/Test              : %s\n", ((reg & USER_TEST_MTH_STH) == USER_TEST_MTH_STH?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Console Mode           : %s\n", ((reg & CONS_JUMPER) != CONS_JUMPER?"Serial":"VGA"));
	vConsoleWrite (achBuffer);

#if 0
	vConsoleWrite ("\nETHERNET\n-------------------\n");
#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	
	pNetif = netif_find( "ge0" );
	if (pNetif != NULL)
	{
		geifGetMacAddr( "ge0", bLanAddress );
	}
	else
	{
		vGetEthMacAddr(1, bLanAddress);
	}
	
#else
	vGetEthMacAddr(1, bLanAddress);
#endif
	sprintf (achBuffer, "Ethernet 1 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	pNetif = netif_find( "ge1" );
	if (pNetif != NULL)
	{
		geifGetMacAddr( "ge1", bLanAddress );
	}
	else
	{
		vGetEthMacAddr(2, bLanAddress);
	}
#else
	vGetEthMacAddr(2, bLanAddress);
#endif
	sprintf (achBuffer, "Ethernet 2 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	pNetif = netif_find( "ge2" );
	if (pNetif != NULL)
	{
		geifGetMacAddr( "ge2", bLanAddress );
	}
	else
	{
		vGetEthMacAddr(3, bLanAddress);
	}
#else
	vGetEthMacAddr(3, bLanAddress);
#endif
	sprintf (achBuffer, "Ethernet 3 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	pNetif = netif_find( "ge3" );
	if (pNetif != NULL)
	{
		geifGetMacAddr( "ge3", bLanAddress );
	}
	else
	{
		vGetEthMacAddr(4, bLanAddress);
	}
#else
	vGetEthMacAddr(4, bLanAddress);
#endif
	sprintf (achBuffer, "Ethernet 4 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);
#endif

	return rt;
}


UINT32 brdCuteInfo(void *ptrParam)
{
	UINT8	reg;
	PCI_PFA	pfa;
	brd_info *info =(brd_info *) ptrParam;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	info->memory = (UINT16) (dPciReadReg (pfa, 0xBC, REG_32) >> 16);			// TOLUD

	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));
	info->HardwareRev = ('A' + ((reg >> 5) & 0x07));

	info->firmwareVersion = FW_VERSION_NUMBER;

	return E__OK;
}



