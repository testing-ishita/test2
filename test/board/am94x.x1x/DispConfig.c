
/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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


/*****************************************************************************
 * DispConfig: Display board configuration
 * RETURNS: None
 * */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	/* temp32, addr,*/ rt = E__OK;
	/*UINT16	mem=0;*/
	UINT8	reg/*, temp*/;
	char	achBuffer[80];	/* text formatting buffer */

	UINT8 bLanAddress[6];
	/*UINT32 dMem = 0;*/
	UINT32 memCfg;
	
#ifdef INCLUDE_NETWORK
	struct netif *pNetif;
#endif


	sprintf (achBuffer, "Num CPUs               : %d\n", (int)bCpuCount);
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "CPU Frequency          : %i.%02iGHz\n", sysGetCpuFrequency () / 1000,(sysGetCpuFrequency () % 1000) / 10);
	vConsoleWrite (achBuffer);
	brdMemfromGPIO(&memCfg);
	
	switch(memCfg)
	{
		case 0x0:
				sprintf (achBuffer, "Memory                 : %dGB Fitted\n", 4);
				vConsoleWrite (achBuffer);
				break;
		case 0x1:
				sprintf (achBuffer, "Memory                 : %dGB Fitted\n", 2);
				vConsoleWrite (achBuffer);
				break;

		case 0x2:
				sprintf (achBuffer, "Memory                 : %dGB Fitted\n", 8);
				vConsoleWrite (achBuffer);
				break;

		case 0x3:
				sprintf (achBuffer, "Memory                 : %dGB Fitted\n", 16);
				vConsoleWrite (achBuffer);
				break;
		default:
			sprintf (achBuffer, "Invalid Memory Configuration\n");
			vConsoleWrite (achBuffer);
			break;



	};


	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));
	sprintf (achBuffer, "Hardware Revision      : %c\n", ('A' + ((reg >>4) & 0x03)));
	vConsoleWrite (achBuffer);

	//sprintf (achBuffer, "BIOS Defaults       : %s\n", ((reg & CMOS_RESTORE)?"Restore User Defaults":"Restore Factory Defaults"));
	//vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8));

	sprintf (achBuffer, "Mode                   : %s\n", ((reg & CUTE_MODE) == CUTE_MODE?"CUTE":"BIOS"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "User/Test              : %s\n", ((reg & USER_TEST_MTH_STH) == USER_TEST_MTH_STH?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Console Mode           : %s\n", ((reg & CONS_JUMPER) != CONS_JUMPER?"Serial":"VGA"));
	vConsoleWrite (achBuffer);

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

	return rt;
}


UINT32 brdCuteInfo(void *ptrParam)
{
	UINT8	reg;
	PCI_PFA	pfa;
	brd_info *info =(brd_info *) ptrParam;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	info->memory = dPciReadReg (pfa, 0xB0, REG_16);

	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));
	info->HardwareRev = ('A' + ((reg >> 5) & 0x07));

	info->firmwareVersion = FW_VERSION_NUMBER;

	return E__OK;
}



