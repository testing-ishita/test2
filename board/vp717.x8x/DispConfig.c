 
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

/* DispConfig.c - Display Board Configuration
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp717.x8x/DispConfig.c,v 1.1 2015-04-02 11:41:18 mgostling Exp $
 * $Log: DispConfig.c,v $
 * Revision 1.1  2015-04-02 11:41:18  mgostling
 * Initial check-in to CVS
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.4  2012/08/30 16:01:24  hmuneer
 * name fix
 *
 * Revision 1.3  2011/01/20 10:01:23  hmuneer
 * CA01A151
 *
 * Revision 1.2  2010/07/26 13:14:10  hmuneer
 * Source Cleanup
 *
 * Revision 1.1  2010/06/23 10:49:08  hmuneer
 * CUTE V1.01
 *
 * Revision 1.1  2010/01/19 12:03:15  hmuneer
 * vx511 v1.0
 *
 * Revision 1.1  2009/09/29 12:16:45  swilson
 * Create VP417 Board
 *
 * Revision 1.5  2009/06/08 16:13:16  swilson
 * Remove 'Core Execution' to 'tests' folder.
 *
 * Revision 1.4  2009/06/03 07:08:23  hmuneer
 * error code revision
 *
 * Revision 1.3  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.2  2009/05/19 08:09:53  cvsuser
 * no message
 *
 * Revision 1.1  2009/05/18 09:35:08  hmuneer
 * no message
 *
 * Revision 1.0  Feb 9 2009
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

//#define DEBUG
#define SCR_ERROR		(E__BIT + 0x1000)

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT8	bCpuCount;
extern void vGetEthMacAddr(	UINT8  bInstance, UINT8* pbAddress);

/* forward declarations */


static PTR48 tPtr1;
static volatile UINT32 mapRegSpace=0;
static UINT32 mHandle;
/*****************************************************************************
 * TEST_INTERFACE: Display board configuration
 * RETURNS: None
 * */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	temp32, addr, rt = E__OK;
	UINT16	mem;
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
	else if(temp32 == 0x00448086)
	{
		vConsoleWrite("Arrandale\n");
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
	sprintf (achBuffer, "COM Port            : %s\n", ((reg & 0x10) == 0x10?"COM3":"COM1"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VME SYSRST          : %s\n", ((reg & 0x20) == 0x20?"Enable":"Disable"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "User/Test           : %s\n", ((reg & 0x40) == 0x40?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Mode                : %s\n", ((reg & 0x80) == 0x80?"BIOS":"VSA"));
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

		case 0x05:
			sprintf (achBuffer, "PMC Frequency       : PCI 33 MHz\n");
			break;

		case 0x06:
			sprintf (achBuffer, "PMC Frequency       : PCI 66 MHz\n");
			break;

		case 0x07:
			sprintf (achBuffer, "PMC Frequency       : PCI-X 66 MHz\n");
			break;

		case 0x08:
			sprintf (achBuffer, "PMC Frequency       : UNKNOWN\n");
			break;
	}	
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x312, REG_8));
	sprintf (achBuffer, "CF Card             : %s\n", ((reg & 0x10) == 0x10?"Present":"Not Present"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x31e, REG_8));
	sprintf (achBuffer, "VME Slot ID         : %x\n", (reg & 0x1f));
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x31c, REG_8));
	vConsoleWrite ("\nGPIO\n-------------------\n");	
	sprintf (achBuffer, "GPIO Direction      : GPIO1:%s\tGPIO2:%s\tGPIO2:%s \n",
			                                   ((reg & 0x10) == 0x10?"OUTPUT":"INPUT"),
											   ((reg & 0x20) == 0x20?"OUTPUT":"INPUT"),
											   ((reg & 0x40) == 0x40?"OUTPUT":"INPUT")
											   );
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "GPIO Current Data   : GPIO1:%x    \tGPIO2:%x    \tGPIO2:%x\n",
			                                   ((reg & 0x01) == 0x01?1:0),
											   ((reg & 0x02) == 0x02?1:0),
											   ((reg & 0x04) == 0x04?1:0)
											   );
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


