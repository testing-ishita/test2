
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

/* board_cfg.c - functions reporting board specific architecture
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/board_cfg.c,v 1.3 2015-04-02 16:51:56 hchalla Exp $
 *
 * $Log: board_cfg.c,v $
 * Revision 1.3  2015-04-02 16:51:56  hchalla
 * Added COM3 port support which previously not present.
 *
 * Revision 1.2  2015-04-01 16:44:29  hchalla
 * Initial release of CUTE firmware 02.02.01.
 *
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
 * Revision 1.1  2013-09-04 07:13:48  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2012/04/12 12:31:05  hchalla
 * Added flag for ethernet to display the mac address while running t,999 test.
 *
 * Revision 1.2  2012/02/21 11:11:54  hchalla
 * Changes to the  cpci backplane configuration bus numbering.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
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
#include <bit/console.h>
#include <bit/delay.h>
#include <bit/io.h>
#include <bit/board_service.h>
#include <bit/msr.h>
#include <devices/eth.h>

#include "../ppb1x.msd/cctboard.h"

/* defines */

//#define DEBUG
/* defines */
#define LPC_BRIDGE_DEV	31
#define GPIO_BASE	    0x48

#define PORT_NTMTBLADDR		0x004D0
#define PORT_NTMTBLSTS		0x004D4
#define PORT_NTMTBLDATA		0x004D8
#define PORT_NTREQIDCAP		0x004DC

#define PORT0_NT_REG_BASE	0x01000
#define PORT8_NT_REG_BASE	0x11000
#define PORT12_NT_REG_BASE	0x19000
#define PORT16_NT_REG_BASE	0x21000

#define PORT_NTCTL			0x00400

#define SPEEDSTEP_ERROR				(E__BIT + 0x1000)
#define SPEEDSTEP_PROCHOT_ERROR     (E__BIT + 0x1010)


#define PORT_61		0x61		/* NMI Status & Control register */
#define SPKR_EN		0x02
#define SPKR_GATE	0x01

#define COM1		0			/* array index for numbered COM ports */
#define COM2		1
#define COM3		2

#define GP_LVL          0x0C
#define GP_LVL2        0x38
#define GP_LVL3       0x48

#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

//#define DEBUG
#define DEBUG_SOAK
/* typedefs */

/* constants */

const UINT8			bDebugPort = COM1;	/* COM1 is debug console */

const SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO        */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-       */
//	{ 3,	"Ch 2", 0x03E8, 11 },	/* COM3 Port C     -"-    */
	{ 0,	"",		0,		0 }
};

/* locals */
static CPCI_DRV_INFO cpciDrvInfo;
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
 extern UINT8	bCpuCount;
UINT32 brdGetConsolePort (void* ptr)
{

	*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr; 

 #if 1 //following added to have correct nukmber of CPU cores
 	UINT32 msrHigh=0,msrLow=0;
 	 /* read the CORE_THREAD_COUNT MSR */
	 vReadMsr(0x35,&msrHigh,&msrLow); 
	 bCpuCount = (UINT8) ((msrLow >> 16) & 0x0000ffff);
 #endif
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
	UINT8   bPeri;
	UINT16  consp;

	//check if board is in peripheral mode
	bPeri = dIoReadReg(0x214, REG_8);
	bPeri = bPeri & 0x01;

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

		//skip soak port, COM1 for tr80x, in STH mode on system controller
		if(asSccParams[i].bPort == 1)
			if( (bGetTestHandler() == TH__STH) && (bPeri == 0x01))
			{
				i++;
				continue;
			}

		//skip debug port, COM1 for TR501, if debug channel open
		if(asSccParams[i].bPort == 1)
			if(sysIsDebugopen() == 1)
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
		if (asSccParams[i].bPort == ((SCC_PARAMS*)ptr)->bPort  )
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

	(void)ptr;

	/* Flash LED at 2Hz */

	bTemp = sysInPort8 (0x211);
	bTemp = bTemp & ~0x01;

	while (1)
	{
		sysOutPort8 (0x211, bTemp | 0x01);		/* 300mS on */
		sysDelayMilliseconds (300);

		sysOutPort8 (0x211, bTemp);				/* 200mS off */
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
	(void)ptr;

	/* First try the PCI reset port */

	sysOutPort8 (0x0CF9, 0x6);
	
	/* If that fails, try keyboard controller */

	sysOutPort8 (0x0064, 0xFE);

	return E__OK;// should never return
} /* brdReset () */
/*****************************************************************************
 * brdCheckUserMthSwitch:
 *
 * RETURNS: switch Flag
 */
UINT32 brdCheckUserMthSwitch (void *ptr)
{

	UINT8 umswitch = 0;

	umswitch = ((UINT8)dIoReadReg(STATUS_REG_0, REG_8));

	if ((umswitch & USER_JUMPER) == USER_JUMPER)
		*((UINT8*)ptr) =  SW__TEST;
	else
		*((UINT8*)ptr) =  SW__USER;

	//*((UINT8*)ptr) =  SW__TEST; //forced to SOAK mode for testing. Need to remove this line after debug  Rajan--15-05-2015

	return E__OK;
} /* brdCheckUserMthSwitch () */




/*****************************************************************************
 * brdGetSokePort: report the stdin/stdout console device
 *
 * RETURNS: base address of port
 */

UINT32 brdGetSokePort (void *ptr)
{
	*((UINT16*)ptr) = asSccParams[COM2].wBaseAddr;

	return E__OK;

} /* brdGetConsolePort () */


/*****************************************************************************
 * brdGetFlashDriveParams: report parameters for the Flash Drive module
 *
 * RETURNS: base address of port
 */

UINT32 brdGetFlashDriveParams (void *ptr)
{
	((FLASH_DRV_PARAMS*)ptr)->bInstance = 1;
	((FLASH_DRV_PARAMS*)ptr)->bport     = 1;

	return E__OK;

} /* brdGetFlashDriveParams () */


UINT32 brdGetIPMINMI(void *ptr)
{
	UINT8 reg=0;

	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));
    if (reg & IPMI_NMI)
		*((UINT8*)ptr) =1;
	else
		*((UINT8*)ptr) =0;

	return E__OK;
}

UINT32 brdClearIPMINMI(void *ptr)
{
	UINT8 reg=0;

	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));

	reg = reg & ~(IPMI_NMI);

	vIoWriteReg (0x212, REG_8, reg);
	
	return E__OK;
}





/*****************************************************************************
 * brdSelectEthInterface: selects front or rear Ethernet interface
 *
 * RETURNS: success or error code
 */
#define VID_INTEL		0x8086
#define DID_82579		0x1502
#define DIDVID_82579    ((DID_82579 << 16) | VID_INTEL)


/*****************************************************************************
 * brdSkipEthInterface: skip ethernet controller used for network interface
 *
 * RETURNS: success or error code
 */

UINT32 brdSkipEthInterface (void *ptr)
{
	UINT8	bDev, bFunc;
	PCI_PFA	pfa;
	UINT16	wGpioBase;
	UINT32	gtemp,dTemp=0;
	UINT32	didVid;
	UINT8	bTestHandler;
	UINT8	bReqInstance;


#if 0
	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

	// GPIO_27
	dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

	if ((dTemp & 0x8000000) == 0x8000000)
	{
		didVid = DIDVID_i350AMx_C;
		bReqInstance = 1;
	}
	else
	{
		didVid = DIDVID_i350AMx_C;
		bReqInstance = 3;
	}

#endif

    bReqInstance = 1; //Added and fixed to one instance for TPB1X.msd
	
	bTestHandler = bGetTestHandler();
	if ((bTestHandler == TH__LBIT) ||
		(bTestHandler == TH__PBIT) ||
		(bTestHandler == TH__STH ))
	{
		bDev = PCI_PFA_DEV((((SkipEthIf_PARAMS*)ptr)->pfa));
		bFunc = PCI_PFA_FUNC((((SkipEthIf_PARAMS*)ptr)->pfa));

		if ((((SkipEthIf_PARAMS*)ptr)->dDidVid == didVid) &&
			(((SkipEthIf_PARAMS*)ptr)->bInstance == bReqInstance) && (bDev == 0) && (bFunc == 0))
			return E__OK;
		else
			return 0x01;
	}
	else
		return E__FAIL;
}

UINT32 brdConfigCpciBackplane (void *ptr)
{
	PCI_PFA	pfa;
	char buffer[80];
	(void)ptr;


	pfa = PCI_MAKE_PFA (0, 28, 4);

    /* To make the other boards connected via backplane visible in soak mode. */
    PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x22);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x23);

	pfa = PCI_MAKE_PFA (0x22, 0, 0);
    *((PCI_PFA*)ptr) = pfa;

    return (E__OK);
}

UINT32 brdDeConfigCpciBackplane (void *ptr)
{
	(void)ptr;
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA (0, 28, 4);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);


    return (E__OK);

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
			hwStatus = dIoReadReg(0x310, REG_8);
			if ((hwStatus & 0x20) == 0x20)
			{
				// switch is set for device 1, so select device 2
				backupDev = 1;
			}
			else
			{
				// switch is set for device 2, so select device 1
				backupDev = 0;
			}
		}

		if (backupDev == 1)
		{
			reg = dIoReadReg(0x312, REG_8);
			reg &= ~0xC0;				// preserve all other bits in this register
			reg |= 0x80;
			vIoWriteReg(0x312, REG_8, reg);
		}
		else
		{
			reg = dIoReadReg(0x312, REG_8);
			reg &= ~0xC0;				// preserve all other bits in this register
			reg |= 0x40;
			vIoWriteReg(0x312, REG_8, reg);
		}

		//vIoWriteReg(STATUS_CTRL_REG_5, REG_8, reg);
	}

	return (rt);
}


UINT32 brdGetCpciBrInstance (void *ptr)
{
	UINT32 dInstance = 0;
	(void)ptr;
	UINT8 bTemp;
	PCI_PFA pfa;
	UINT16 wGpioBase = 0;
	UINT32 dTemp = 0;
	char buffer[80];


	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

	// GPIO_27
	dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

	if ((dTemp & 0x8000000) == 0x8000000 )
	{

		dInstance = 3;
	}
	else
	{
		dInstance = 2;
	}

	sprintf(buffer, "cpciBrInstance(): dInstance: %d\n",dInstance);
	sysDebugWriteString(buffer);

	*((UINT32*)ptr) = dInstance;

	return (E__OK);
}

UINT32 brdCheckCpciIsSyscon (void *ptr)
{
	UINT8 bFlag = 0;
	UINT8 bTemp1 = 0, bTemp2 = 0;
#ifdef DEBUG_SOAK
	char buffer[32];
#endif
	(void)ptr;

	bTemp1 = dIoReadReg(CPCI_STATUS, REG_8);


#ifdef DEBUG_SOAK
	sprintf(buffer,"1. CPCI_STATUS bTemp1:%#x \n",bTemp1);
	sysDebugWriteString(buffer);
	vDelay(10);
#endif

	bTemp1 = (bTemp1) & (FORCE_SAT|CPCI_PRESENT);

#ifdef DEBUG_SOAK
	vDelay(10);
	sprintf(buffer,"2. bTemp1:%#x \n",bTemp1);
	sysDebugWriteString(buffer);
	vDelay(10);
#endif

	bTemp2 = dIoReadReg(CPCI_STATUS, REG_8);
	bTemp2 = (bTemp2) & (CPCI_SYSEN);

#ifdef DEBUG_SOAK
	vDelay(1000);
	sprintf(buffer,"3. bTemp2:%#x \n",bTemp2);
	sysDebugWriteString(buffer);
	vDelay(10);
#endif

	if ( (bTemp1 == CPCI_PRESENT) && (bTemp2 != CPCI_SYSEN) )
	{
		bFlag = 1;
	}
	else
	{
		bFlag = 0;
	}

	*((UINT8*)ptr) = bFlag;

	 return (E__OK);
}



/*****************************************************************************
 * brdGetSccPortMap: provide a map of ports available for testing
 * ptr	  : pointer to a UINT32 bitmap - when bit set indicates port available,
 *          bit-0 = COM1
 * RETURNS: E__OK
 */
UINT32 brdGetRS485PortMap (void* ptr)
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
	while (asSccParams[i].bPort )
	{

		//skip console port in MTH and IBIT mode
		if( asSccParams[i].wBaseAddr == 0x03F8)
		{
				i++;
				continue;
		}

		//skip console port in MTH and IBIT mode
		if( asSccParams[i].wBaseAddr == consp)
		{
			if( (bGetTestHandler() == TH__MTH) || (bGetTestHandler() == TH__LBIT ) )
			{
				i++;
				continue;
			}
		}

		//skip debug port, COM1 for tr803, if debug channel open
		if(asSccParams[i].bPort == 1)
			if(sysIsDebugopen() == 1)
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

UINT32 brdSetSpeedStep(void *ptr)
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

/*****************************************************************************
 * brdGetDramSize: get size of DRAM from IO straps
 *
 * RETURNS: RAM size in GB
 */
UINT32 brdGetDramSize (void* ptr)
{
	UINT8	ioStraps;
	UINT8	ramSize;

	PCI_PFA pfa;
	UINT16 wGpioBase = 0;
	UINT8 bVal   = 0;
	UINT32 dTemp = 0;
	UINT32 dTemp2 = 0;
	UINT16 wTemp = 0;
	UINT8 bTemp = 0;
	UINT16 wMem=0;
	char achBuffer[80];

	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

	// GPIO_49
	dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);

	bVal = (UINT8)((dTemp >> 17) & 0x01);
#ifdef DEBUG
	sprintf(achBuffer,"1.GPIO bVal:%d\n\n", bVal);
	vConsoleWrite (achBuffer);
#endif
	//GPIO_52 and GPIO50
	dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);
	dTemp = (dTemp>> 18) & 0x01;

	dTemp2 = dIoReadReg (wGpioBase + GP_LVL2, REG_32);
	dTemp2 = (dTemp2>> 20) & 0x01;

	wTemp = (UINT16)( (dTemp2<<2) | (dTemp<<1) | bVal);



#ifdef DEBUG
	sprintf(achBuffer,"2.GPIO dTemp:%d\n\n", wTemp);
	vConsoleWrite (achBuffer);
#endif


#ifdef DEBUG
	sprintf(achBuffer,"3.GPIO bVal:%d\n\n", wTemp);
	vConsoleWrite (achBuffer);
#endif


	switch (wTemp)
	{
		case 0:
			ramSize = 4;
			break;

		case 1:
			ramSize = 8;
			break;

		case 2:
			ramSize = 16;
			break;

		case 0x03:
			ramSize = 32;
			break;

		case 0x04:
			ramSize = 2;
			break;

		case 0x05:
			ramSize = 4;
			break;

		case 0x06:
			ramSize = 8;
			break;

		default:
			ramSize = 0;
			break;
	}

	*((UINT8*)ptr) = ramSize;

	return E__OK;

} /* brdGetDramSize () */


#ifdef DEBUG
UINT32 getCPUTemp (void)
{
	UINT32 msrHigh=0,msrLow=0, dTemp = 0;
	vReadMsr(0x19C,&msrHigh,&msrLow);	// Therm Status
	dTemp = (msrLow >> 16) & (0x000000FF);
	return (dTemp);
}
#endif

UINT32 brdUnLockCCTIoRegs (void *ptr)
{

	vIoWriteReg(0x210,REG_8,0x43);
	vIoWriteReg(0x210,REG_8,0x43);
	vIoWriteReg(0x210,REG_8,0x54);
	vDelay(4);
	vIoWriteReg(0x210,REG_8,0x43);
	vIoWriteReg(0x210,REG_8,0x43);
	vIoWriteReg(0x210,REG_8,0x54);
	vDelay(4);

	if ((dIoReadReg (0x210, REG_8) & 0x10) == 0x10)
	{
		 return (E__OK);
	}
	else
		return (E__FAIL);
}

UINT32 brdLockCCTIoRegs (void *ptr)
{
	vIoWriteReg(0x210,REG_8,0x43);
	vIoWriteReg(0x210,REG_8,0x43);
	return (E__OK);
}

/*****************************************************************************
 * brdGetCpciDeviceImages: Provide the CPCI Outbound image buffer addresses
 *
 * RETURNS: success
 */

UINT32 brdGetCpciDeviceImages( void* ptr )
{
	*((VME_DRV_INFO**)ptr) = &cpciDrvInfo;

	return E__OK;
}

UINT8 bGetSlotFromDevId (UINT8 bDevice)
{
	 UINT8 bSlotNum = 0;

		  switch(bDevice)
		  {
			  case 1:
				  bSlotNum = 16;
				  break;

			  case 2:
				  bSlotNum = 15;
				  break;

			  case 3:
				  bSlotNum = 14;
				  break;
			  case 4:
				  bSlotNum = 13;
				  break;
			  case 5:
				  bSlotNum = 12;
				  break;
			  case 6:
				  bSlotNum = 11;
				  break;
			  case 7:
				  bSlotNum = 10;
				  break;
			  case 8:
				  bSlotNum = 9;
				  break;
			  case 9:
				  bSlotNum = 8;
				  break;
			  case 10:
				  bSlotNum = 7;
				  break;
			  case 11:
				  bSlotNum = 6;
				  break;
			  case 12:
				  bSlotNum = 5;
				  break;
			  case 13:
				  bSlotNum = 4;
				  break;
			  case 14:
				  bSlotNum = 3;
				  break;
			  case 15:
				  bSlotNum = 2;
				  break;
			  default:
				  bSlotNum = 0xFF;
				  break;
		  }

		  return bSlotNum;
}

UINT32 brdGetCPCIUpStreamImage (void *ptr)
{
	UINT8 bBus=0,bDevice=0,bOccurance=0,bIndex=0;
	UINT16 wVenId =0;
	UINT32 dTemp=0,dLocalBar=0,dBar=0;
	PCI_PFA pfa;

#ifdef DEBUG_SOAK
	char buffer[80];
#endif

	board_service(SERVICE__BRD_GET_CPCI_INSTANCE,NULL,&bOccurance);

	if (iPciFindDeviceById ( bOccurance ,0x12D8,0xE130,&pfa) == E__OK)
	{
		/* enable if */
		dTemp = dPciReadReg (pfa, 4, REG_16);
		dTemp |= 7;
		vPciWriteReg(pfa, 4, REG_16, dTemp);

		/* save the base address */
		dBar = dPciReadReg (pfa, 0x18, REG_32) & 0xffffff00;
		cpciDrvInfo.upStreamAddress = dBar;
	}

	return E__OK;
}

UINT32 brdGetCPCISlaveImages (void *ptr)
{

	UINT8 bBus=0,bOccurance=0,bIndex=0;
	char bDevice = 0;
	UINT16 wVenId =0;
	UINT32 dTemp=0,dLocalBar=0,dBar=0;
	PCI_PFA pfa;

#ifdef DEBUG_SOAK
	char buffer[80];
#endif

	board_service(SERVICE__BRD_GET_CPCI_INSTANCE,NULL,&bOccurance);
	sprintf(buffer, "vGetSlaveBoards(): bOccurance: %d\n",bOccurance);
	sysDebugWriteString(buffer);

	if (iPciFindDeviceById ( bOccurance ,0x12D8,0xE130,&pfa) == E__OK)
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString("vGetSlaveBoards(): found bridge\n");
		sprintf(buffer, "vGetSlaveBoards(): bOccurance: %d\n",bOccurance);
		sysDebugWriteString(buffer);
#endif

		/* get secondary bus number */
		bBus = dPciReadReg (pfa, 0x19, REG_8);

		/* now scan the secondary bus and remap devices */
		for (bDevice = (MAX_BOARDS-1); bDevice >= 0; bDevice--)
		{
			pfa = PCI_MAKE_PFA(bBus, bDevice, 0);
			/* check the subsystem ID */
			wVenId = dPciReadReg(pfa,0x00, REG_16);
			/* check the subsystem ID */
			if (wVenId == 0x12D8 || wVenId == 0x10B5 || wVenId == 0x3388)
			{
#ifdef DEBUG_SOAK
				sprintf(buffer, "vGetSlaveBoards(): found CCT slave at B:%d D:%d\n",
						(int)bBus, (int)bDevice);
				sysDebugWriteString(buffer);
#endif
				/* enable if */
				dTemp = dPciReadReg (pfa, 4, REG_16);
				dTemp |= 7;
				vPciWriteReg(pfa, 4, REG_16, dTemp);

				/* save the base address */
				dBar = dPciReadReg (pfa, 0x18, REG_32);
				if (wVenId == 0x12D8)
				{
					dLocalBar = (dBar & 0xffffff00);
					cpciDrvInfo.downStreamAddress[bIndex] =  dLocalBar;
				}
				else
				{
					dLocalBar = (dBar & 0xffffff00);
					cpciDrvInfo.downStreamAddress[bIndex] =  dLocalBar;
				}

				cpciDrvInfo.slaveBoardSlotId[bIndex] = bGetSlotFromDevId (bDevice);
				cpciDrvInfo.slaveBoardPresent[bIndex] = 1;
				bIndex++;
			}
			else
			{
				cpciDrvInfo.downStreamAddress[bIndex] = 0;
				cpciDrvInfo.slaveBoardPresent[bIndex] = 0;
				bIndex++;
			}
		}
	}

	return E__OK;
}

