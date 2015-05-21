 
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/DispConfig.c,v 1.2 2015-02-26 10:05:15 hchalla Exp $
 *
 * $Log: DispConfig.c,v $
 * Revision 1.2  2015-02-26 10:05:15  hchalla
 * Removed Soak Flags for IDE Flash Drive test from test_list.c and Removed PMC frequency
 * reading from 0x214.
 *
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.2  2012/08/07 03:24:44  madhukn
 * *** empty log message ***
 *
 * Revision 1.1  2012/06/22 11:41:58  madhukn
 * Initial release
 *
 * Revision 1.1  2012/06/14 08:30:00  madhukn
 * Initial version of VP 91x sources.
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


/* defines */
/*defines*/


#define	SIZE_1MB		0x0100000L

#define TOLUD_HASWELL	0xBC
#define TOUUD_HASWELL	0xA8


#define I7_DEV   0
#define I7_VID   0x8086
#define I7_DID1  0x0044    /*host bridge*/
#define I7_DID2  0x0104    /*host bridge*/



#define  TOLUD        0xB0  /*top of low usable DRAM*/
#define  TOUUD        0xA2  /*top of upper usable DRAM*/
#define	SIZE_1MB		0x0100000L

#define TOLUD_IVYBRIDGE  0xBC
#define TOUUD_IVYBRIDGE  0xA8

//#define DEBUG
#define SCR_ERROR		(E__BIT + 0x1000)


#define LPC_BRIDGE_DEV	31
#define GPIO_BASE	    0x48

#define GP_LVL          0x0C
#define GP_LVL2        0x38
#define GP_LVL3       0x48

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT8	bCpuCount;
extern void 	vGetEthMacAddr(	UINT8  bInstance, UINT8* pbAddress);
//static void vGetSpeedString (UINT8 bCode, UINT8 *abText);
/* forward declarations */


//static PTR48 tPtr1;
static volatile UINT32 mapRegSpace=0;
//static UINT32 mHandle;


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


UINT8 bGetVarInfo (void)
{

	PCI_PFA pfa;
	UINT16 wGpioBase = 0;
	UINT8 bVal   = 0;
	UINT32 dTemp = 0;
	UINT8 bTemp = 0;
	char achBuffer[80];
	UINT8 bVarInfo=0;

	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

	// GPIO_7
	dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

	bVal = (UINT8)((dTemp >> 7) & 0x01);

	//GPIO_69 and GPIO68
	dTemp = dIoReadReg (wGpioBase + GP_LVL3, REG_32);
	dTemp =(UINT8) (dTemp>> 4) & 0x03;


	 bVarInfo = (UINT8) (dTemp<<1) |(bVal);

	 return bVarInfo;
}
/*****************************************************************************
 * TEST_INTERFACE: Display board configuration
 * RETURNS: None
 * */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	reg32,temp32, rt = E__OK,dMem=0;
	UINT16	mem;
	UINT16  gpioBase=0x500;
	UINT8	reg, temp,bVal=0;
	char	achBuffer[80];	/* text formatting buffer */
	PCI_PFA	pfa;
	UINT8 bVarInfo = 0;

	    // obtain ram size in GB from IO strap settings
		// output parameter in bVal is GPIO strap setting
		// returned parameter in bVal is memory size
		board_service (SERVICE__BRD_GET_DRAM_SIZE_FROM_GPIO, NULL, (void *)&bVal);

#ifdef DEBUG
		sprintf (achBuffer, "bVal                  : %dGB\n", bVal);
		vConsoleWrite (achBuffer);
#endif

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

	bVarInfo = bGetVarInfo();

	switch ( bVarInfo )
	{
		case 0x00:
			sprintf (achBuffer, "Board Variant          : N-Grade \n");
			break;

		case 0x01:
			sprintf (achBuffer, "Board Variant          : E-Grade \n");
			break;

		case 0x02:
			sprintf (achBuffer, "Board Variant          : K-Grade \n");
			break;

		case 0x04:
			sprintf (achBuffer, "Board Variant          : RC-Grade \n");
			break;

		default:
			sprintf (achBuffer, "Board Variant           : UNKNOWN\n");
			break;
	}
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x210, REG_8));
	sprintf (achBuffer, "Hardware Revision      : %c\n", ('A' + (reg  & 0x07)));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "FPGA Revision          : %c\n", ('A' + ((reg>>3)  & 0x07)));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "BIOS Defaults          : %s\n", ((reg & 0x40) == 0x40?"Restore User Defaults":"Restore Factory Defaults"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Console Mode           : %s\n", ((reg & 0x80) == 0x80?"Serial":"VGA"));
	vConsoleWrite (achBuffer);	


	reg = ((UINT8)dIoReadReg(0x212, REG_8));
	sprintf (achBuffer, "PMC1 Present           : %s\n", ((reg & 0x01) == 0x01?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "PMC2 Present           : %s\n", ((reg & 0x02) == 0x02?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Universe II NMI        : Has %s\n", ((reg & 0x40) == 0x40?"Occurred":"Not Occurred"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Front Panel NMI        : Has %s\n", ((reg & 0x80) == 0x80?"Occurred":"Not Occurred"));
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x211, REG_8));
	sprintf (achBuffer, "PMC PCI Freq           : %s\n", ((reg & 0x01) == 0x01?"66MHz":"33MHz"));
	vConsoleWrite (achBuffer);
    sprintf (achBuffer, "PPMC EREADY            : %s\n", ((reg & 0x02) == 0x02?"Ready":"Not Ready"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Watchdog               : %s\n", ((reg & 0x08) == 0x08?"Enable":"Disable"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VME SYSRST             : %s\n", ((reg & 0x20) == 0x20?"Enable":"Disable"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "User/Test              : %s\n", ((reg & 0x40) == 0x40?"SOAK":"MTH"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Mode                   : %s\n", ((reg & 0x80) == 0x80?"BIOS":"CUTE"));
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x21D, REG_8));
	sprintf (achBuffer, "XMC1 Present           : %s\n", ((reg & 0x01) == 0x01?"Yes":"NO"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC1 MBIST             : %s\n", ((reg & 0x02) == 0x02?"Complete":"BIST in progress"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC2 Present           : %s\n", ((reg & 0x04) == 0x04?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC2 MBIST             : %s\n", ((reg & 0x08) == 0x08?"Complete":"BIST in progress"));
	vConsoleWrite (achBuffer);

#if 0
	reg = ((UINT8)dIoReadReg(0x214, REG_8));
	temp = (reg >> 5) & 0x07;
	switch ( temp )
	{
		case 0x00:
			sprintf (achBuffer, "PMC Frequency          : PCI 25 MHz\n");
			break;

		case 0x01:
			sprintf (achBuffer, "PMC Frequency          : PCI 50 MHz\n");
			break;

		case 0x02:
			sprintf (achBuffer, "PMC Frequency          : PCI-X 50 MHz\n");
			break;

		case 0x03:
			sprintf (achBuffer, "PMC Frequency          : PCI-X 100 MHz\n");
			break;

		case 0x04:
			sprintf (achBuffer, "PMC Frequency          : PCI 33 MHz\n");
			break;

		case 0x05:
			sprintf (achBuffer, "PMC Frequency          : PCI 66 MHz\n");
			break;

		case 0x06:
			sprintf (achBuffer, "PMC Frequency          : PCI-X 66 MHz\n");
			break;

		default:
			sprintf (achBuffer, "PMC Frequency          : UNKNOWN\n");
			break;
	}	
	vConsoleWrite (achBuffer);
#endif

	reg = ((UINT8)dIoReadReg(0x312, REG_8));
	sprintf (achBuffer, "CF Card                : %s\n", ((reg & 0x04) == 0x10?"Present":"Not Present"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x31e, REG_8));
	sprintf (achBuffer, "VME Slot ID            : %x\n", (reg & 0x1f));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x310, REG_8));
	vConsoleWrite ("\nGPIO\n-------------------\n");	
	sprintf (achBuffer, "GPIO Direction: \nGPIO8:%s\nGPIO7:%s\nGPIO6:%s\nGPIO5:%s\nGPIO4:%s\nGPIO3:%s\nGPIO2:%s\nGPIO1:%s \n",
			                                   ((reg & 0x80) == 0x80?"OUTPUT":"INPUT"),
											   ((reg & 0x40) == 0x40?"OUTPUT":"INPUT"),
											   ((reg & 0x20) == 0x20?"OUTPUT":"INPUT"),
											   ((reg & 0x10) == 0x10?"OUTPUT":"INPUT"),
											   ((reg & 0x08) == 0x08?"OUTPUT":"INPUT"),
											   ((reg & 0x04) == 0x04?"OUTPUT":"INPUT"),
											   ((reg & 0x02) == 0x02?"OUTPUT":"INPUT"),
											   ((reg & 0x01) == 0x01?"OUTPUT":"INPUT")
											   );
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x31C, REG_8));
	sprintf (achBuffer, "GPIO Data: \nGPIO1:%x\nGPIO2:%x\nGPIO3:%x\nGPIO4:%x\nGPIO5:%x\nGPIO6:%x\nGPIO7:%x\nGPIO8:%x\n",
			                                   ((reg & 0x01) == 0x01?1:0),
											   ((reg & 0x02) == 0x02?1:0),
											   ((reg & 0x04) == 0x04?1:0),
											   ((reg & 0x08) == 0x08?1:0),
									           ((reg & 0x10) == 0x10?1:0),
											   ((reg & 0x20) == 0x20?1:0),
											   ((reg & 0x40) == 0x40?1:0),
											   ((reg & 0x80) == 0x80?1:0)
											   );
	vConsoleWrite (achBuffer);

	rt = E__OK;

	return rt;
}


void brdCuteInfo(brd_info *info)
{
	UINT8	reg;
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	info->memory = (UINT16)(dPciReadReg (pfa, 0xBC, REG_32) >> 16);		// Ivy Bridge TOLUD

	reg = ((UINT8)dIoReadReg(0x210, REG_8));
	info->HardwareRev = ('A' + (reg & 0x07));

	info->firmwareVersion = FW_VERSION_NUMBER;
}



