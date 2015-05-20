
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

/* board_cfg.c - functions reporting board specific architecture
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/amc1x.msd/board_cfg.c,v 1.9 2015-02-27 16:18:04 mgostling Exp $
 * $Log: board_cfg.c,v $
 * Revision 1.9  2015-02-27 16:18:04  mgostling
 * Use board specific GPIO strap readings to detect RAM size
 *
 * Revision 1.8  2014-11-10 09:32:07  mgostling
 * Add new function to initialise the RTC if it has bad time and date values.
 *
 * Revision 1.7  2014-11-04 15:10:28  mgostling
 * Check for both occurrences of the TSI721 in brdCheckDualSrio.
 * Allow both i210 Ethernet Controllers to be tested in MTH mode.
 *
 * Revision 1.6  2014-10-15 11:18:29  mgostling
 * Revise brdGetSRIOInfo to pass GPIO information for which SRIO devices are fitted.
 *
 * Revision 1.5  2014-10-09 09:47:13  chippisley
 * Added function to read SRIO mode (master or slave) from status register.
 *
 * Revision 1.4  2014-10-03 15:11:38  chippisley
 * Corrected compiler warning in brdSkipEthInterface().
 *
 * Revision 1.3  2014-10-03 14:40:42  chippisley
 * Corrected bus/dev numbers.
 * Revised brdSkipEthInterface() to skip i210 devices if assigned to the network (Soak Master)
 *
 * Revision 1.2  2014-09-19 13:09:33  mgostling
 * Fixed compiler warning message
 *
 * Revision 1.1  2014-09-19 10:41:42  mgostling
 * Initial check in to CVS
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <string.h>
#include <stdio.h>
#include <private/port_io.h>
#include <private/sys_delay.h>
#include <private/debug.h>
#include <private/cpu.h>
#include <bit/bit.h>
#include <bit/msr.h>
#include <bit/io.h>
#include <bit/pci.h>
#include <bit/delay.h>
#include <bit/board_service.h>
#include <bit/console.h>
#include <bit/mem.h>
#include <devices/eth2.h>
#include "cctboard.h"
#include "config.h"

/* defines */

#undef DEBUG
//#define DEBUG

#define PORT_61		0x61		/* NMI Status & Control register */
#define SPKR_EN		0x02
#define SPKR_GATE	0x01

#define COM1		0			/* array index for numbered COM ports */
#define COM2		1

#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

#define TSI721_VID_DID          0x80AB111D

#define SPEEDSTEP_ERROR				(E__BIT + 0x1000)
#define SPEEDSTEP_PROCHOT_ERROR     (E__BIT + 0x1010)
#define PORT_NTMTBLADDR		0x004D0
#define PORT_NTMTBLSTS		0x004D4
#define PORT_NTMTBLDATA		0x004D8
#define PORT_NTREQIDCAP		0x004DC

#define PORT0_NT_REG_BASE	0x01000
#define PORT8_NT_REG_BASE	0x11000
#define PORT12_NT_REG_BASE	0x19000
#define PORT16_NT_REG_BASE	0x21000

#define PORT_NTCTL			0x00400
#define  USER_JUMPER		0x002	/*0 = MTH 1 = SOAK */

// PCH GPIO registers

#define GPIO_BASE			0x48
#define GP_LVL				0x0C
#define GP_LVL2				0x38

/* typedefs */

typedef struct{
	UINT16 regOffset;
	UINT32 dwValue;
} PCI_REG_DATA;

typedef struct {
	UINT8 bBus, bDevice, bFunc;
	UINT8 nNumRegs;
	PCI_REG_DATA pciRegData[8];
} PCI_REG_CONFIG;

/* constants */

const UINT8			bDebugPort = COM1;	/* COM1 is debug console */
const UINT8			bConsolePort = COM1;	/* COM1 is console */
//const UINT8			bDebugPort = COM2;	/* COM2 is debug console */
//const UINT8			bConsolePort = COM2;	/* COM2 is console */

const SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO 	  */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-    */
	{ 0,	"",		0,		0 }
};

const SRIO_DEVICE_SMBUS_ADDRESS	srioDeviceSmbusAddressList[] =
{
	{0x64, 0xA4},	/* TSI721 #1 */
	{0x66, 0xA6},	/* TSI721 #2 */
};

/* locals */

PCI_REG_CONFIG pciStoredConfig;

static SRIO_DEVICE_SMBUS_INFO srioDeviceSmbusInfo;

/* globals */

/* externals */

/* forward declarations */
#ifdef DEBUG
UINT32 getCPUTemp (void);
#endif

/*****************************************************************************
 * brdGetConsolePort: report the stdin/stdout console device
 * ptr	  : pointer to a UINT16 for base address of port
 * RETURNS: E__OK
 */

UINT32 brdGetConsolePort (void* ptr)
{
	// serial port selection is a BIOS setup item, so use COM1

	*((UINT16*)ptr) = asSccParams[bConsolePort].wBaseAddr;

	return E__OK;

} /* brdGetConsolePort () */


/*****************************************************************************
 * brdGetDebugPort: report the debug console device
 * ptr	  : pointer to a UINT16 for base address of port
 * RETURNS: E__OK
 */
UINT32 brdGetDebugPort (void* ptr)
{
	*((UINT16*)ptr) = asSccParams[bDebugPort].wBaseAddr;

	return E__OK;

} /* brdGetDebugPort () */

/*****************************************************************************
 * brdGetSccPortMap: provide a map of ports available for testing
 * ptr	  : pointer to a UINT32 bitmap - when bit set indicates port available,
 *          bit-0 = COM1
 * RETURNS: E__OK
 */
UINT32 brdGetSccPortMap (void* ptr)
{
	UINT32	dPortMap = 0;
	int		i = 0;
	UINT16  consp;

	//get console port base address
	brdGetConsolePort (&consp);

/*
 * - check soak/MTH mode, if not peripheral and in soak then soak port can't be tested,
 *   if peripheral mode then soak port can be tested, if MTH then console port can't be tested
 * - Check if in IBIT, if in IBIT then console port can't be tested
 * - check debug mode, if debug mode enabled then debug port can't be tested
 * */

	/* Build a port map */
	while (asSccParams[i].bPort != 0)
	{
		//skip console port in MTH and IBIT mode
		if( asSccParams[i].wBaseAddr == consp)
		{
			if( (bGetTestHandler() == TH__MTH) || (bGetTestHandler() == TH__LBIT ) )
			{
				i++;
				continue;
			}
		}

		//skip debug port, COM1 for AMC1x, if debug channel open
		if(asSccParams[i].bPort == bConsolePort)
			if(sysIsDebugopen() == bConsolePort)
			{
				i++;
				continue;
			}

		dPortMap |= (UINT32)1 << asSccParams[i].bPort;
		i++;
	}

	*((UINT32*)ptr) = dPortMap;

	return E__OK;
} /* brdGetSccPortMap() */

/*****************************************************************************
 * brdGetSokePort: report the stdin/stdout console device
 *
 * RETURNS: base address of port
 */

UINT32 brdGetSokePort (void *ptr)
{
	*((UINT16*)ptr) = asSccParams[bConsolePort].wBaseAddr;

	return E__OK;

} /* brdGetConsolePort () */


/*****************************************************************************
 * brdGetSccParams: report parameters for specified SCC port
 *
 *SCC_PARAMS*
 *UINT8		bPort
 * RETURNS: pointer to port data
 */
UINT32 brdGetSccParams(void* ptr )
{
	int		i = 0;

	/* Find data for port */
	while (asSccParams[i].bPort != 0)
	{
		if (asSccParams[i].bPort == ((SCC_PARAMS*)ptr)->bPort)
		{
			memcpy(ptr, &asSccParams[i], sizeof(SCC_PARAMS) );
			return E__OK;
		}

		i++;
	}
	return E__FAIL;

} /* brdGetSccParams () */



/*****************************************************************************
 * brdGetMaxDramBelow4GB: get size of DRAM below 4GB
 *
 * RETURNS: 1 + highest addressable DRAM, below 4GB
 */
UINT32 brdGetMaxDramBelow4GB (void* ptr)
{
	*((UINT32*)ptr) = 0x40000000; /* fixed at 'safe' 1GB */

	return E__OK;

} /* brdGetMaxDramBelow4GB () */



/*****************************************************************************
 * brdFlashLed: flash a suitable board specific LED - forever
 *
 * RETURNS: none
 */
UINT32 brdFlashLed (void* ptr)
{
	UINT8	bTemp;

	/* Flash LED at 2Hz */

	bTemp = sysInPort8 (PORT_61);
	bTemp = (bTemp & ~SPKR_EN) | SPKR_GATE;			/* Gate enable, Spkr disable */

	while (1)
	{
		sysOutPort8 (PORT_61, bTemp | SPKR_EN);		/* 300mS on */
		sysDelayMilliseconds (300);

		sysOutPort8 (PORT_61, bTemp);				/* 200mS off */
		sysDelayMilliseconds (200);
	}

	return E__OK;

} /* brdFlashLed () */



/*****************************************************************************
 * brdReset:
 *
 * RETURNS:
 */
UINT32 brdReset (void *ptr)
{

	/* First try the PCI reset port */
	sysDelayMilliseconds(200);
	sysOutPort8 (0x0CF9, 0x6);
	//sysOutPort8 (0x92, ((sysInPort8(0x92)) | 0x1));

	/* If that fails, try keyboard controller */
	sysOutPort8 (0x0064, 0xFE);

	return E__OK; // should never return

} /* brdReset () */




/*****************************************************************************
 * brdCheckUserMthSwitch:
 *
 * RETURNS: switch Flag
 * */
UINT32 brdCheckUserMthSwitch (void *ptr)
{
	UINT8	umswitch;

	umswitch = ((UINT8) dIoReadReg (STATUS_REG_1, REG_8));

	if ((umswitch & USER_TEST_MTH_STH) == USER_TEST_MTH_STH)
		*((UINT8*)ptr) = 1; /*STH*/
	else
		*((UINT8*)ptr) = 0; /*MTH*/

	return E__OK;

} /* brdCheckUserMthSwitch () */


UINT32 brdGetIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8) dIoReadReg (STATUS_REG_1, REG_8));
	*((UINT8*)ptr) = (reg & IPMI_NMI) >> 6;

	return E__OK;
}

UINT32 brdclearIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8) dIoReadReg (STATUS_REG_1, REG_8)) & (~IPMI_NMI);
	vIoWriteReg (STATUS_REG_1, REG_8, reg );

	return E__OK;
}


/*****************************************************************************
 * brdSkipEthInterface: skip ethernet controller used for network interface
 *
 * RETURNS: success or error code
 */

UINT32 brdSkipEthInterface (void *ptr)
{
#ifdef INCLUDE_NETWORK
	UINT8 bBus, bDev, bFunc;
#if 0
	PCI_PFA	pfa;
	UINT32 dTemp;
#endif

#ifdef DEBUG
	char achBuffer[80];
#endif

	if (bGetTestHandler() == TH__MTH)
	{
		// No network, so don't skip any interfaces
		return 1;
	}

	if ((bGetTestHandler() == TH__LBIT) ||
		(bGetTestHandler() == TH__PBIT) ||
		(bGetTestHandler() == TH__STH ))
	{
		bBus = PCI_PFA_BUS((((SkipEthIf_PARAMS*)ptr)->pfa));
		bDev = PCI_PFA_DEV((((SkipEthIf_PARAMS*)ptr)->pfa));
		bFunc = PCI_PFA_FUNC((((SkipEthIf_PARAMS*)ptr)->pfa));

#if 0
		pfa = PCI_MAKE_PFA(3,0,0);
		dTemp = PCI_READ_DWORD (pfa, 0);
#endif
		// exclude both i210 devices
		if ((((SkipEthIf_PARAMS*)ptr)->dDidVid == DIDVID_i210_S)
			&& ((bBus == 4) || (bBus == 5))
			&& (bDev == 0)
			&& (bFunc == 0))
		{
#ifdef DEBUG
			sprintf (achBuffer, "Device: %d:%d:%d, %#X:%#X skipped\n",
						bBus, bDev, bFunc,
						((SkipEthIf_PARAMS*)ptr)->dDidVid & 0xFFFF, ((SkipEthIf_PARAMS*)ptr)->dDidVid >> 16);
			vConsoleWrite (achBuffer);
#endif
			return E__OK;
		}
#if 0
		else if (dTemp == TSI721_VID_DID)
		{
			if ( (((SkipEthIf_PARAMS*)ptr)->dDidVid == DIDVID_i210_S) &&
				(bBus == 5) && (bDev == 0) && (bFunc == 0) )
			{
#ifdef DEBUG
				sprintf (achBuffer, "Device: %d:%d:%d, %#X:%#X skipped\n",
						bBus, bDev, bFunc,
						((SkipEthIf_PARAMS*)ptr)->dDidVid & 0xFFFF, ((SkipEthIf_PARAMS*)ptr)->dDidVid >> 16);
				vConsoleWrite (achBuffer);
#endif
				return E__OK;
			}
			else
			{
				return 1;
			}
		}
#endif
		else
		{
			return 1;
		}
	}
	else
	{
		return E__FAIL;
	}
#else
	// No network, so don't skip any interfaces
	return 1;

#endif

} /* brdSelectEthInterface () */


/*****************************************************************************
 * brdGetDramSize: get size of DRAM from IO straps
 *
 * RETURNS: RAM size in GB
 */
UINT32 brdGetDramSize (void* ptr)
{
	UINT8	bVal;
	UINT8	ramSize;
	PCI_PFA	pfa;
	UINT16	wGpioBase;
	UINT32	dTemp;

	pfa = PCI_MAKE_PFA (0, 0x1F, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
	// GPIO_16 & GPIO_15
	dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

	bVal = (UINT8)((dTemp >> 15) & 0x03);

	// GPIO_50 & GPIO_52
	dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);
	dTemp >>= 16;
	bVal |= (UINT8)(((dTemp & 0x10) == 0 ? 0 : 0x04) + ((dTemp & 0x04) == 0 ? 0 : 0x08));

	// obtain ram size in GB from IO strap settings
	switch (bVal)
	{
		case 4:
			ramSize = 8;
			break;

		case 5:
			ramSize = 4;
			break;

		case 6:
			ramSize = 16;
			break;
	}

	*((UINT8*)ptr) = ramSize;

	return E__OK;

} /* brdGetDramSize () */


UINT32 brdGetCpciBrInstance (void *ptr)
{
	UINT32 dInstance = 0;

	dInstance = 1;
	*((UINT32*)ptr) = dInstance;

	return (E__OK);
}

UINT32 brdCheckCpciIsSyscon (void *ptr)
{

	*((UINT8*)ptr) = 1;
	 return (E__OK);


}

/*****************************************************************************
 * brdGetFlashDriveParams: report parameters for the Flash Drive module
 *
 * RETURNS: base address of port
 */

UINT32 brdGetFlashDriveParams (void *ptr)
{
	((FLASH_DRV_PARAMS*)ptr)->bInstance	= 1;
	((FLASH_DRV_PARAMS*)ptr)->bport		= 3;

	return E__OK;

} /* brdGetFlashDriveParams () */

/*****************************************************************************
 * brdSetSpeedStep: Check all speed ratios can be set
 * ptr	  : pointer for speedstep test result
 * RETURNS: E__OK
 */
UINT32 brdSetSpeedStep (void *ptr)
{
	UINT32 msrHigh = 0, msrLow = 0, timeout;
	UINT8 maxRatio = 0, minRatio = 0, i = 0;
#ifdef DEBUG
	CPUID_REGS sRegs;
	char achBuffer[80];
#endif

#ifdef DEBUG
	// check if speedsteep is supported and enabled
	sysCpuid (1, (void*) &sRegs);
	sprintf (achBuffer, "CPUID 1 [ECX]: %08x - Speedstep Feature Flag = %08x\n", sRegs.ecx, sRegs.ecx & 0x00000080);
	sysDebugWriteString (achBuffer);

	sysCpuid (6, (void*) &sRegs);
	sprintf (achBuffer, "CPUID 6 [EAX]: %08x - Factory Turbo Mode = %08x\n", sRegs.eax, sRegs.ecx & 0x00000002);
	sysDebugWriteString (achBuffer);
	sprintf (achBuffer, "CPUID 6 [EAX]: %08x - Turbo Mode Feature Flag = %08x\n", sRegs.eax, sRegs.ecx & 0x00000004);
	sysDebugWriteString (achBuffer);

	vReadMsr (0x1A0 ,&msrHigh, &msrLow);		// IA32_MISC_ENABLES
	sprintf (achBuffer, "MSR 0x1A0: %08x:%08x\n", msrHigh, msrLow);
	sysDebugWriteString (achBuffer);
	sprintf (achBuffer, "Turbo Mode Disable %08x = %08x\n", msrHigh, msrHigh & 0x00000040);
	sysDebugWriteString (achBuffer);
	sprintf (achBuffer, "Speedstep Enable %08x = %08x\n", msrLow, msrLow & 0x00010000);
	sysDebugWriteString (achBuffer);
#endif

	// Get the max ratio allowed
	// speed may have been restricted by the BIOS due to board build option
	vReadMsr (0x64C,&msrHigh,&msrLow);		// TURBO_ACTIVATION_RATIO (Maximum Non-Turbo ratio))

#ifdef DEBUG
	sprintf (achBuffer, "\nMSR 0x64C: %08x:%08x\n", msrHigh, msrLow);
	sysDebugWriteString (achBuffer);
#endif

	maxRatio = msrLow & 0xff;

	// Get the min & max ratios available for this CPU
	vReadMsr(0xce, &msrHigh, &msrLow);	// PLATFORM_INFO

#ifdef DEBUG
	sprintf (achBuffer, "MSR 0xCE: %08x:%08x\n", msrHigh, msrLow);
	sysDebugWriteString (achBuffer);
#endif

	if (maxRatio == 0)
	{
		maxRatio = (msrLow >> 8) & 0xff;	// no speed restrictions by the BIOS
	}
	else
	{
		++maxRatio;							// max ratio restricted by the BIOS
	}

	minRatio = (msrHigh >> 8) & 0xff;

#ifdef DEBUG
	sprintf (achBuffer, "minRatio: %x\nmaxRatio: %x\n", minRatio, maxRatio);
	sysDebugWriteString (achBuffer);
#endif

	for( i = minRatio; i <= maxRatio; i++ )
	{
		/*Set the intended ratio*/
		vReadMsr (0x199, &msrHigh, &msrLow);

#ifdef DEBUG
		sprintf (achBuffer, "MSR 0x199: %08x:%08x\n", msrHigh, msrLow);
		sysDebugWriteString (achBuffer);
#endif

#ifdef DEBUG
		sprintf (achBuffer, "0x199 before=%x,%x\n", msrHigh, msrLow);
		sysDebugWriteString (achBuffer);
#endif

		msrLow &= 0xffff80ff;
		msrLow |= (i << 8) & 0x7f00;
		vWriteMsr (0x199, msrHigh, msrLow);
#ifdef DEBUG
		sprintf (achBuffer, "0x199 After=%x,%x\n", msrHigh, msrLow);
		sysDebugWriteString (achBuffer);
#endif

		timeout=15;

		/*Check if the ratio has been set*/
		while (--timeout)
		{
			vDelay (10);
			vReadMsr (0x198, &msrHigh, &msrLow);

			if (((msrLow >> 8) & 0xff) == i)
			{
#ifdef DEBUG
				sprintf (achBuffer, "Current P-State ratio=%x\n", ((msrLow >> 8) & 0xff));
				sysDebugWriteString (achBuffer);
#endif
				break;
			}
		}

		if( !timeout )
		{
#ifdef DEBUG
			sprintf (achBuffer, "Unable to set ratio=%x, Current %x\n", i, ((msrLow >> 8) & 0xff));
			sysDebugWriteString (achBuffer);

			vReadMsr (0x199, &msrHigh, &msrLow);
			sprintf (achBuffer, "Perf CTL=%x:%x\n",msrHigh,msrLow);
			sysDebugWriteString (achBuffer);

			vReadMsr (0x198, &msrHigh, &msrLow);
			sprintf (achBuffer, "Perf STS=%x:%x\n", msrHigh, msrLow);
			sysDebugWriteString (achBuffer);
#endif

			vReadMsr (0x19C, &msrHigh, &msrLow);
			if ((msrLow & 0x02) == 0x02)
			{
#ifdef DEBUG
				sprintf (achBuffer, "Therm STS=%x:%x\n", msrHigh, msrLow);
				sysDebugWriteString (achBuffer);
#endif
				*((UINT32*)ptr) =  SPEEDSTEP_PROCHOT_ERROR;
				return (E__OK);
			}

#ifdef DEBUG
			sprintf (achBuffer,"**FAIL** : Speed Step CPU Temperature: %d\n\n", getCPUTemp());
			sysDebugWriteString (achBuffer);
#endif

			*((UINT32*)ptr) =  SPEEDSTEP_ERROR + i;
			return (E__OK);
		}
	}

	*((UINT32*)ptr) =  E__OK;
	return (E__OK);
}

#ifdef DEBUG
UINT32 getCPUTemp (void)
{
	UINT32 msrHigh = 0, msrLow = 0, dTemp = 0;
	vReadMsr (0x19C, &msrHigh, &msrLow);	// Therm Status
	dTemp = (msrLow >> 16) & (0x000000FF);
	return (dTemp);
}
#endif

UINT32 brdStoreConfigChanges (void* ptr)
{
	UINT8 i;
	UINT32  bIoWindow;
	PCI_PFA	 pfa;

	//Storing initial generic LPC IO decode ranges to be restored later
	pfa = PCI_MAKE_PFA (0, 31, 0);
	pciStoredConfig.bBus=0;
	pciStoredConfig.bDevice=31;
	pciStoredConfig.bFunc=0;
	for(i=0;i<4;i++)
	{
		pciStoredConfig.pciRegData[i].regOffset=0x84+i*4;
		bIoWindow = PCI_READ_DWORD(pfa, 0x84+i*4);
		pciStoredConfig.pciRegData[i].dwValue=bIoWindow;
	}
	pciStoredConfig.nNumRegs=4;
	return E__OK;

} /* brdStoreConfigChanges () */

UINT32 brdRestoreConfigChanges (void* ptr)
{
	UINT8 i;
	PCI_PFA	 pfa;

	if(pciStoredConfig.nNumRegs>=8){
		//Something freaky. might have been called without storing anything before
		return E__OK;
	}
	//Restoring generic LPC IO decode ranges back to initial values
	pfa = PCI_MAKE_PFA (pciStoredConfig.bBus, pciStoredConfig.bDevice, pciStoredConfig.bFunc);
	for(i=0;i<pciStoredConfig.nNumRegs;i++)
	{
		PCI_WRITE_DWORD(pfa,
			pciStoredConfig.pciRegData[i].regOffset,
			pciStoredConfig.pciRegData[i].dwValue);

	}
	pciStoredConfig.nNumRegs=0;//Resetting to be on safer side
	return E__OK;

} /* brdRestoreConfigChanges () */

UINT32 brdEnableSmicLpcIoWindow(void* ptr)
{
	UINT8	i;
	UINT32  bIoWindow;
	PCI_PFA	 pfa;
#ifdef DEBUG
	char achBuffer[80];
#endif

	pfa = PCI_MAKE_PFA (0, 31, 0);
	for(i=0;i<4;i++)
	{
		bIoWindow = PCI_READ_DWORD(pfa, 0x84+i*4);
		if ((bIoWindow & 0xfff) == 0xCA1)
		{
			//Already Io Window is enabled, nothing to be done
			return E__OK;
		}
	}
	PCI_WRITE_DWORD(pfa, 0x90,0x00FC0CA1);
#ifdef DEBUG
	sprintf(achBuffer,"value in Register 0x90 :%#x\n\n", PCI_READ_DWORD(pfa, 0x90));
	vConsoleWrite(achBuffer);
#endif
	return E__OK;

} /* brdEnableWdLpcIoWindow() */


UINT32 brdConfigSrioDevice (void *ptr)
{
	PCI_PFA	pfa;
#ifdef DEBUG
	char achBuffer[80];
#endif


	UINT8	bTemp;

	//pfa = PCI_MAKE_PFA (0, 1, 0);
	pfa = PCI_MAKE_PFA (0, 1, 1);
#ifdef DEBUG
	sprintf(achBuffer,"%s: PCI bridge: %03d:%02d:%02d\n",
			__FUNCTION__, PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa));
	sysDebugWriteString(achBuffer);
#endif

	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);
	//PCI- PCI Bridge
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	// PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x01);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x02);
	//PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x06); //A
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x02);


	// pfa = PCI_MAKE_PFA(0x1, 0x0, 0);
	pfa = PCI_MAKE_PFA(0x2, 0x0, 0);
#ifdef DEBUG
	sprintf(achBuffer,"%s: SRIO device: %03d:%02d:%02d\n",
			__FUNCTION__, PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa));
	sysDebugWriteString(achBuffer);
#endif
	*((PCI_PFA*)ptr) = pfa;

	return (E__OK);

}


 UINT32 brdDeConfigSrioDevice (void *ptr)
{
	PCI_PFA	pfa;
	UINT8   bTemp;
#ifdef DEBUG
	char achBuffer[80];
#endif

	//pfa = PCI_MAKE_PFA (0, 1, 0);
	pfa = PCI_MAKE_PFA (0, 1, 1);
#ifdef DEBUG
	sprintf(achBuffer,"%s: PCI bridge: %03d:%02d:%02d\n",
			__FUNCTION__, PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa));
	sysDebugWriteString(achBuffer);
#endif

	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp &= ~0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);

	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x0);

	return (E__OK);
}


UINT32 brdConfigSrioDevice2 (void *ptr)
{
 	PCI_PFA	pfa;
 	UINT8   bTemp;
#ifdef DEBUG
	char achBuffer[80];
#endif

 	//pfa = PCI_MAKE_PFA (0, 1, 1);
 	pfa = PCI_MAKE_PFA (0, 1, 2);
#ifdef DEBUG
	sprintf(achBuffer,"%s: PCI bridge: %03d:%02d:%02d\n",
			__FUNCTION__, PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa));
	sysDebugWriteString(achBuffer);
#endif

 	bTemp = PCI_READ_BYTE (pfa, 0x04);
 	bTemp |= 0x07;
 	PCI_WRITE_BYTE (pfa, 0x04, bTemp);
 	//PCI- PCI Bridge
 	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
 	// PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x02);
 	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x03);
 	// PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x02);
 	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x03);


 	//pfa = PCI_MAKE_PFA(0x2, 0x0, 0);
 	pfa = PCI_MAKE_PFA(0x3, 0x0, 0);
#ifdef DEBUG
	sprintf(achBuffer,"%s: SRIO device: %03d:%02d:%02d\n",
			__FUNCTION__, PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa));
	sysDebugWriteString(achBuffer);
#endif
 	*((PCI_PFA*)ptr) = pfa;

 	return (E__OK);
}


UINT32 brdDeConfigSrioDevice2 (void *ptr)
{
   	(void)ptr;
   	PCI_PFA	pfa;
#ifdef DEBUG
	char achBuffer[80];
#endif

 	// pfa = PCI_MAKE_PFA (0, 1, 1);
 	pfa = PCI_MAKE_PFA (0, 1, 2);
#ifdef DEBUG
	sprintf(achBuffer,"%s: PCI bridge: %03d:%02d:%02d\n",
			__FUNCTION__, PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa));
	sysDebugWriteString(achBuffer);
#endif

 	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
 	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
 	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);

    return (E__OK);
}



/*****************************************************************************
 * brdCheckDualSrio: Check if two TSI721 are fitted
 *
 * RETURNS: E__OK
 */
UINT32 brdCheckDualSrio(void *ptr)
{
	(void)ptr;
	PCI_PFA	pfa;
	UINT32 dev1;
	UINT32 dev2;

	pfa = PCI_MAKE_PFA(0x2, 0x0, 0);
	dev1 = PCI_READ_DWORD (pfa, 0);
	pfa = PCI_MAKE_PFA(0x3, 0x0, 0);
	dev2 = PCI_READ_DWORD (pfa, 0);

	if ((TSI721_VID_DID == dev1) && (TSI721_VID_DID == dev2))
	{
		return (E__OK);
	}
	else
	{
		return (E__FAIL);
	}
}


/*****************************************************************************
 * brdSelectSpiFwh: Select primary or backup SPI FWH
 * RETURNS: E__OK
 */
UINT32 brdSelectSpiFwh (void *ptr)
{
	UINT8	selectDev;
	UINT8	backupDev;
	UINT8	hwStatus;
	UINT8	reg;
	UINT32	rt;

	rt = E__OK;

	// unlock hidden registers
	vIoWriteReg(STATUS_REG_2, REG_8, 0xFF);
	vDelay(5);
	vIoWriteReg(STATUS_REG_2, REG_8, 0);
	vDelay(5);
	vIoWriteReg(STATUS_REG_2, REG_8, 'C');
	vDelay(5);
	vIoWriteReg(STATUS_REG_2, REG_8, 'C');
	vDelay(5);
	vIoWriteReg(STATUS_REG_2, REG_8, 'T');
	vDelay(5);

	reg = dIoReadReg(STATUS_REG_1, REG_8);
	if ((reg & REGISTER_LOCK) == 0)
	{
		rt = E__FAIL;
	}
	else
	{
		// select 'reset defaults'
		backupDev = 0;

		/*
			get required device:
				0 = primary device
				1 = backup device
		*/
		selectDev = *((UINT8*)ptr);

		if (selectDev == 1)		// backup device?
		{
			// check SPI FWH  switch setting
			hwStatus = dIoReadReg(CCT_HIDDEN_HW_STATUS, REG_8);
			if ((hwStatus & SPI_BOOT_DEV) == 0)
			{
				// switch is set for device 1, so select device 2
				backupDev = 2;
			}
			else
			{
				// switch is set for device 2, so select device 1
				backupDev = 1;
			}
		}

		reg = dIoReadReg(CCT_HIDDEN_HW_CONTROL, REG_8);
		reg &= ~SPI_DEV_SELECT;				// preserve all other bits in this register
		reg |= backupDev << 6;

		// select the device we want
		vIoWriteReg(CCT_HIDDEN_HW_CONTROL, REG_8, reg);

		// lock the hidden registers again
		vIoWriteReg(STATUS_REG_2, REG_8, 0xFF);
	}

	return (rt);
}

/*****************************************************************************
 * brdGetSRIOInfo: Select primary or backup SPI FWH
 * RETURNS: E__OK
 */
UINT32 brdGetSRIOInfo (void *ptr)
{
	PCI_PFA	pfa;
	UINT16	wGpioBase;
	UINT32	dTemp;


	srioDeviceSmbusInfo.numDev = 2;
	srioDeviceSmbusInfo.srioDeviceList = &srioDeviceSmbusAddressList[0];

	// check GPIO inputs to see which devices are fitted

	pfa = PCI_MAKE_PFA (0, 0x1F, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
	// GPIO_17 (SRIO #1) & GPIO_24 (SRIO #2)
	dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

	srioDeviceSmbusInfo.devFitted = 0;		// srt with none fitted

	// check GPIO17
	if (dTemp & 0x00020000L)
		srioDeviceSmbusInfo.devFitted |= 1;

	// check GPIO24
	if (dTemp & 0x01000000L)
		srioDeviceSmbusInfo.devFitted |= 2;

	*((SRIO_DEVICE_SMBUS_INFO**)ptr) = &srioDeviceSmbusInfo;

	return E__OK;
}

/*****************************************************************************
 * brdCheckSRIOMaster: Read SRIO mode (master or slave) from status register
 *
 * RETURNS: E__OK if master or E__FAIL
 */
UINT32 brdCheckSRIOMaster (void *ptr)
{
	UINT8 reg;
#ifdef DEBUG
	char achBuffer[80];
#endif

	// unlock hidden registers
	vIoWriteReg(STATUS_REG_2, REG_8, 0);
	vDelay(5);
	vIoWriteReg(STATUS_REG_2, REG_8, 'C');
	vDelay(5);
	vIoWriteReg(STATUS_REG_2, REG_8, 'C');
	vDelay(5);
	vIoWriteReg(STATUS_REG_2, REG_8, 'T');
	vDelay(5);

	reg = dIoReadReg(0x21E, REG_8);

	if ((reg & 0x80) == 0)
	{
#ifdef DEBUG
		sprintf(achBuffer,"SRIO Master (reg: 0x%02x)\n", reg);
		sysDebugWriteString (achBuffer);
#endif
		return E__OK;
	}
	else
	{
#ifdef DEBUG
		sprintf(achBuffer,"SRIO Slave (reg: 0x%02x)\n", reg);
		sysDebugWriteString (achBuffer);
#endif
		return E__FAIL;
	}
}

/*****************************************************************************
 * rtcBcdToDecimal: Convert hex BCD value to decimal
 *
 * RETURNS: decimal value
 */
static UINT8 rtcBcdToDecimal(UINT8 hex)
{
	UINT8  temp, temp2;

	temp  = hex & 0x0f;
	if(temp > 0x09)
		return 0xff;

	temp2 = (hex >> 4) & 0x0f;
	if(temp2 > 0x09)
		return 0xff;

	temp += (temp2  * 10);

	return temp;
}

/*****************************************************************************
 * rtcUip: Check if RTC update in progress
 *
 * RETURNS: E__OK if master or E__FAIL
 */
static UINT32 rtcUip(UINT32 err)
{
	UINT8  uip, tm;

	tm = 255;
	vIoWriteReg(0x70, REG_8, 0x0a);
	uip = ((UINT8)dIoReadReg(0x71, REG_8));
	while( ((uip & 0x80) == 0x80)  && (tm > 0))
	{
		vDelay(1); //was 5
		vIoWriteReg(0x70, REG_8, 0x0a);
		uip = ((UINT8)dIoReadReg(0x71, REG_8));
		tm--;
	}

	if(tm >0)
		return E__OK;
	else
		return err;
}

/*****************************************************************************
 * brdInitRtc: External management power my not reset the RTC properly
 *
 * If a reset error is detected, the faulty register is initialised appropriately
 *
 * RETURNS: always returns E_OK as there is no way to recover from this if there is a real problem
 */
UINT32 brdInitRtc (void *ptr)
{
	UINT32	rt;
	UINT8	temp1;
	UINT8	temp2;
	
#ifdef DEBUG
	char achBuffer[80];
#endif

#ifdef DEBUG
	sprintf (achBuffer, "%s\n", __FUNCTION__);
	sysDebugWriteString (achBuffer);
#endif

	rt = rtcUip(E__FAIL);
	if (rt ==  E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0);
		temp1 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp1 = rtcBcdToDecimal(temp1);
#ifdef DEBUG
		sprintf (achBuffer, "Seconds Register Value :%d\n", temp1);
		sysDebugWriteString (achBuffer);
#endif
		if (temp1 > 59)
		{
			vIoWriteReg(0x71, REG_8, 0);		// reset the seconds value
#ifdef DEBUG
			sprintf (achBuffer, "Seconds Register reset, Value was :%d\n", temp1);
			sysDebugWriteString (achBuffer);
#endif
		}
	}

	rt = rtcUip(E__FAIL);
	if (rt ==  E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x02);
		temp1 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp1 = rtcBcdToDecimal(temp1);
#ifdef DEBUG
		sprintf (achBuffer, "Minutes Register Value :%d\n", temp1);
		sysDebugWriteString (achBuffer);
#endif
		if (temp1 > 59)
		{
			vIoWriteReg(0x71, REG_8, 0);		// reset the minutes value
#ifdef DEBUG
			sprintf (achBuffer, "Minutes Register reset, Value was :%d\n", temp1);
			sysDebugWriteString (achBuffer);
#endif
		}
	}

	rt = rtcUip(E__FAIL);
	if (rt ==  E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x0b);				// get the clock mode
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		vIoWriteReg(0x70, REG_8, 0x04);				// get current hour
		temp1 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp1 = rtcBcdToDecimal(temp1);
#ifdef DEBUG
		sprintf (achBuffer, "Hours Register Value :%d\n", temp1);
		sysDebugWriteString (achBuffer);
#endif
		if (temp2 & 0x02)
		{
			if (temp1 > 23)
			{
				vIoWriteReg(0x71, REG_8, 0);		// reset the 24 hour value
#ifdef DEBUG
				sprintf (achBuffer, "Hours Register reset, Value was :%d\n", temp1);
				sysDebugWriteString (achBuffer);
#endif
			}
		}
		else
		{
			if (temp1 > 12)
			{
				vIoWriteReg(0x71, REG_8, 0);		// reset the 12 hour value
#ifdef DEBUG
				sprintf (achBuffer, "Hours Register reset, Value was :%d\n", temp1);
				sysDebugWriteString (achBuffer);
#endif
			}
		}
	}

	rt = rtcUip(E__FAIL);
	if (rt ==  E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x07);
		temp1 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp1 = rtcBcdToDecimal(temp1);
#ifdef DEBUG
		sprintf (achBuffer, "Day Register Value :%d\n", temp1);
		sysDebugWriteString (achBuffer);
#endif
		if ((temp1 == 0) || (temp1 > 31))
		{
			vIoWriteReg(0x71, REG_8, 1);		// reset the day value
#ifdef DEBUG
			sprintf (achBuffer, "Day Register reset, Value was :%d\n", temp1);
			sysDebugWriteString (achBuffer);
#endif
		}
	}

	rt = rtcUip(E__FAIL);
	if (rt ==  E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x08);
		temp1 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp1 = rtcBcdToDecimal(temp1);
#ifdef DEBUG
		sprintf (achBuffer, "Month Register Value :%d\n", temp1);
		sysDebugWriteString (achBuffer);
#endif
		if ((temp1 == 0) || (temp1 > 12))
		{
			vIoWriteReg(0x71, REG_8, 1);		// reset the month value
#ifdef DEBUG
			sprintf (achBuffer, "Month Register reset, Value was :%d\n", temp1);
			sysDebugWriteString (achBuffer);
#endif
		}
	}

	rt = rtcUip(E__FAIL);
	if (rt ==  E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x09);
		temp1 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp1 = rtcBcdToDecimal(temp1);
#ifdef DEBUG
		sprintf (achBuffer, "Year Register Value :%d\n", temp1);
		sysDebugWriteString (achBuffer);
#endif
		if (temp1 > 99)
		{
			vIoWriteReg(0x71, REG_8, 14);		// reset the year value
#ifdef DEBUG
			sprintf (achBuffer, "Year Register reset, Value was :%d\n", temp1);
			sysDebugWriteString (achBuffer);
#endif
		}
	}

#ifdef DEBUG
			sprintf (achBuffer, "rt = 0x%08X\n", rt);
			sysDebugWriteString (achBuffer);
#endif
	return E__OK;
}
