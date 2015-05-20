 
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/DispConfig.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: DispConfig.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
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
//static void vGetSpeedString (UINT8 bCode, UINT8 *abText);
/* forward declarations */


//static PTR48 tPtr1;
static volatile UINT32 mapRegSpace=0;
//static UINT32 mHandle;

#if 0
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
#endif

/*****************************************************************************
 * TEST_INTERFACE: Display board configuration
 * RETURNS: None
 * */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	reg32,temp32, rt = E__OK;
	UINT16	mem;
	UINT16  gpioBase=0x500;
	UINT8	reg, temp;	
	char	achBuffer[80];	/* text formatting buffer */
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	temp32 = dPciReadReg(pfa, 0x00, REG_32);

	if( temp32 == 0x2a408086 )
	{		
		vConsoleWrite("GS45\n");				
		mem = dPciReadReg (pfa, 0xa2, REG_16)/1024;
	}
	else if(temp32 == 0x01548086)
	{
		vConsoleWrite("Ivy Bridge\n");

		//Determine Memory Fitted

		//read GIO base
		pfa = PCI_MAKE_PFA (0, 31, 0);
		gpioBase = dPciReadReg(pfa, 0x48, REG_32);		
		gpioBase &= 0xFF80;

		//Read GPIO pins 49, 71, and 70
		temp=0;
		reg32 = dIoReadReg(gpioBase+0x38,REG_32);//GP_LVL[63:32]
		reg = (reg32>>17)&1;
		temp|=reg;
		
		temp = temp <<1;
		reg32 = dIoReadReg(gpioBase+0x48,REG_32);//GP_LVL[75:64]
		reg = (reg32>>7)&1;
		temp|=reg;

		temp = temp <<1;
		reg = (reg32>>6)&1;
		temp|=reg;

		switch(temp)
		{
			case 0:
			case 5:
				mem = 4;//4GB
				break;
			case 1:
			case 6:
				mem = 8;//8GB
				break;
			case 2:
				mem = 16;//16GB
				break;
			case 3:
			case 7:
				mem = 0;//Reserved
				break;
			case 4:
				mem = 2;//2GB
				break;
			default:
				mem = 0;//Unknown
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

	sprintf (achBuffer, "\nNum CPUs            : %d\n", (int)bCpuCount);
	vConsoleWrite (achBuffer);	
	sprintf (achBuffer, "CPU Frequency       : %i.%02iGHz\n", sysGetCpuFrequency () / 1000,
															 (sysGetCpuFrequency () % 1000) / 10);
	vConsoleWrite (achBuffer);	
	sprintf (achBuffer, "Fitted RAM          : %dGB\n", mem);
	vConsoleWrite (achBuffer);	

	reg = ((UINT8)dIoReadReg(0x210, REG_8));
	sprintf (achBuffer, "Hardware Revision   : %c\n", ('A' + (reg  & 0x07)));
	vConsoleWrite (achBuffer);


	sprintf (achBuffer, "VME Master Swap     : %s\n", ((reg & 0x08) == 0x08?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VME Slave Swap      : %s\n", (reg & 0x10) ==0x010?"ON":"OFF");
	vConsoleWrite (achBuffer);	
	sprintf (achBuffer, "VME Fast Swap       : %s\n", ((reg & 0x20) == 0x20?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "BIOS Defaults       : %s\n", ((reg & 0x40) == 0x40?"Restore User Defaults":"Restore Factory Defaults"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Console Mode        : %s\n", ((reg & 0x80) == 0x80?"Serial":"VGA"));
	vConsoleWrite (achBuffer);	



	reg = ((UINT8)dIoReadReg(0x212, REG_8));

	sprintf (achBuffer, "PMC1 Present        : %s\n", ((reg & 0x01) == 0x01?"Yes":"No"));
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "PMC2 Present        : %s\n", ((reg & 0x02) == 0x02?"Yes":"No"));
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "Exp PMC1 Present    : %s\n", ((reg & 0x04) == 0x04?"Yes":"No"));
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "Exp PMC2 Present    : %s\n", ((reg & 0x08) == 0x08?"Yes":"No"));
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "Tsi148 NMI          : Has %s\n", ((reg & 0x40) == 0x40?"Occurred":"Not Occurred"));
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "Front Panel NMI     : Has %s\n", ((reg & 0x80) == 0x80?"Occurred":"Not Occurred"));
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x211, REG_8));
	sprintf (achBuffer, "PPMC EREADY         : %s\n", ((reg & 0x02) == 0x02?"Ready":"Not Ready"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Watchdog Reset      : %s\n", ((reg & 0x04) == 0x04?"Enable":"Disable"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Watchdog            : %s\n", ((reg & 0x08) == 0x08?"Enable":"Disable"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VME SYSRST          : %s\n", ((reg & 0x20) == 0x20?"Enable":"Disable"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "User/Test           : %s\n", ((reg & 0x40) == 0x40?"SOAK":"MTH"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Mode                : %s\n", ((reg & 0x80) == 0x80?"BIOS":"CUTE"));
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x21D, REG_8));
	sprintf (achBuffer, "XMC1 Present        : %s\n", ((reg & 0x01) == 0x01?"Yes":"NO"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC1 MBIST          : %s\n", ((reg & 0x02) == 0x02?"Complete":"BIST in progress"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC2 Present        : %s\n", ((reg & 0x04) == 0x04?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC2 MBIST          : %s\n", ((reg & 0x08) == 0x08?"Complete":"BIST in progress"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x214, REG_8));
	temp = (reg >> 5) & 0x07;
	switch ( temp )
	{
		case 0x00:
			sprintf (achBuffer, "PMC Frequency       : PCI 25 MHz\n");
			break;

		case 0x01:
			sprintf (achBuffer, "PMC Frequency       : PCI 50 MHz\n");
			break;

		case 0x02:
			sprintf (achBuffer, "PMC Frequency       : PCI-X 50 MHz\n");
			break;

		case 0x03:
			sprintf (achBuffer, "PMC Frequency       : PCI-X 100 MHz\n");
			break;

		case 0x04:
			sprintf (achBuffer, "PMC Frequency       : PCI 33 MHz\n");
			break;

		case 0x05:
			sprintf (achBuffer, "PMC Frequency       : PCI 66 MHz\n");
			break;

		case 0x06:
			sprintf (achBuffer, "PMC Frequency       : PCI-X 66 MHz\n");
			break;

		default:
			sprintf (achBuffer, "PMC Frequency       : UNKNOWN\n");
			break;
	}	
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x312, REG_8));
	sprintf (achBuffer, "CF Card             : %s\n", ((reg & 0x10) == 0x10?"Present":"Not Present"));
	//sprintf (achBuffer, "CF Card             : %s\n", ((reg & 0x08) == 0x08?"Present":"Not Present"));
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



