
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


/* defines */
/*defines*/
#define I7_DEV   0
#define I7_VID   0x8086
#define I7_DID1  0x0044    /*host bridge*/
#define I7_DID2  0x0104    /*host bridge*/
#define DEBUG



#define  TOLUD        0xB0  /*top of low usable DRAM*/
#define  TOUUD        0xA2  /*top of upper usable DRAM*/
#define	SIZE_1MB		0x0100000L

#define TOLUD_IVYBRIDGE  0xBC
#define TOUUD_IVYBRIDGE  0xA8

//#define DEBUG
#define SCR_ERROR		(E__BIT + 0x1000)

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT8 bCpuCount;
extern void vGetEthMacAddr(	UINT8  bInstance, UINT8* pbAddress);
#ifdef INCLUDE_NETWORK
extern int geifGetMacAddr( char *pName, UINT8 *pMacAddr );
#endif

/* forward declarations */

static volatile UINT32 mapRegSpace=0;

static void vI7_MemSize_Ivybridge(UINT32* pdMemSize)
{
    UINT32 TOLUD_bRegVal;
    UINT32 TOUUD_bRegVal_Lower;
    UINT32 TOUUD_bRegVal_Higher=0;
    UINT32  g4=0xFFFFFFFFL;
    UINT32 PCI_addr;
    PCI_PFA	pfa;


	pfa = PCI_MAKE_PFA (0, 0, 0);

    /*read the TOLUD register values*/
    TOLUD_bRegVal = (UINT32)dPciReadReg(pfa,TOLUD_IVYBRIDGE,REG_32);

     /*read the TOUUD lower register values*/
    TOUUD_bRegVal_Lower = (UINT32)dPciReadReg(pfa,TOUUD_IVYBRIDGE,REG_32);

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
 * DispConfig: Display board configuration
 *
 * RETURNS: None
 */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	rt = E__OK;
	UINT8	reg;
	char	achBuffer[80];	/* text formatting buffer */
	UINT8 bLanAddress[6];
	UINT32 dMem = 0;

	sprintf (achBuffer, "Num CPUs               : %d\n", (int)bCpuCount);
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "CPU Frequency          : %i.%02iGHz\n", sysGetCpuFrequency () / 1000,(sysGetCpuFrequency () % 1000) / 10);
	vConsoleWrite (achBuffer);

	vI7_MemSize_Ivybridge(&dMem);
	sprintf (achBuffer, "Fitted RAM			: %dGB\n", dMem/1024);
	vConsoleWrite (achBuffer);	


	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));
	sprintf (achBuffer, "Hardware Revision      : %c\n", ('A' + ((reg >>4) & 0x03)));
	vConsoleWrite (achBuffer);
	
	reg = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8));

	sprintf (achBuffer, "Console Mode           : %s\n", ((reg & CONS_JUMPER) != CONS_JUMPER?"Serial":"VGA"));
	vConsoleWrite (achBuffer);


	vConsoleWrite ("\nETHERNET\n-------------------\n");
#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	geifGetMacAddr( "ge0", bLanAddress );
#else
	vGetEthMacAddr(1, bLanAddress);
#endif
	sprintf (achBuffer, "Ethernet 1 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	geifGetMacAddr( "ge1", bLanAddress );
#else
	vGetEthMacAddr(2, bLanAddress);
#endif
	sprintf (achBuffer, "Ethernet 2 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	geifGetMacAddr( "ge2", bLanAddress );
#else
	vGetEthMacAddr(3, bLanAddress);
#endif
	sprintf (achBuffer, "Ethernet 3 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);	

#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	geifGetMacAddr( "ge3", bLanAddress );
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
	info->HardwareRev = ('A' + ((reg >> 4) & 0x07));

	info->firmwareVersion = FW_VERSION_NUMBER;

	return E__OK;
}



