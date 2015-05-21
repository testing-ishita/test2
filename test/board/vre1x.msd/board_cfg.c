
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/board_cfg.c,v 1.15 2015-02-25 17:57:17 hchalla Exp $
 *
 * $Log: board_cfg.c,v $
 * Revision 1.15  2015-02-25 17:57:17  hchalla
 * Added GPIO strap detection for detecting DRAM size moved to board services.
 *
 * Revision 1.14  2014-11-18 10:06:10  mgostling
 * Do not exclude the i210 ethernet controller used by the Universal Soak Master in MTH mode.
 *
 * Revision 1.13  2014-09-19 09:08:01  mgostling
 * In speedstep test, check for maximum speed configured by BIOS
 *
 * Revision 1.12  2014-07-29 11:39:29  mgostling
 * Implement SERVICE__BRD_SKIP_ETH_IF so that the network interface
 * to the Universal Soak Master is not modified during ethernet device testing.
 *
 * Revision 1.11  2014-07-25 13:41:51  mgostling
 * Fix pointer assignment compiler warning in brdGetHdaList().
 *
 * Revision 1.10  2014-07-25 10:51:31  mgostling
 * Added brdGetSlotId() to support Universal Soak Master interface.
 *
 * Revision 1.9  2014-03-27 11:56:46  mgostling
 * Added a new test to vefify XMC differention IO signals.
 *
 * Revision 1.8  2014-03-21 13:27:00  mgostling
 * Removed brdConfigureVPXNTPorts as the VPX backplane connections are tested with a new test.
 *
 * Revision 1.7  2014-03-20 16:02:18  mgostling
 * Add new service to get switch connections for ADVR3 loopback test
 *
 * Revision 1.6  2014-03-18 14:18:48  cdobson
 * Changed some hard coded values to use cctboard.h definitions.
 * Corrected brdCheckUserMthSwitch code.
 *
 * Revision 1.5  2014-03-13 16:20:29  cdobson
 * Corrected Quad serial port interrupt.
 *
 * Revision 1.4  2014-03-12 11:04:58  cdobson
 * Corrected test NID number.
 *
 * Revision 1.3  2014-03-07 09:57:05  cdobson
 * Added AlwaysFitted field to HDA_CODEC. So we dont error on optional
 * codecs which are not fitted.
 *
 * Revision 1.2  2014-03-06 16:35:08  cdobson
 * Added new service to get audio configuration.
 *
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.2  2013-12-10 11:54:33  mgostling
 * Updated speedstep test.
 * Added addotional debugging to determine TDP straping from GPIO.
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.5  2013-10-03 12:45:32  mgostling
 * Added missing return parameter to brdConfigureVPXNTPorts
 *
 * Revision 1.4  2013-10-02 11:21:54  mgostling
 * Allow selection of COM port for CUTE using SW1-3
 *
 * Revision 1.3  2013-09-30 13:35:12  jthiru
 * Removed debug defines
 *
 * Revision 1.2  2013-09-13 10:23:34  jthiru
 * Updates for SOAK TESTING
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.4.2.2  2012-11-13 17:09:37  hchalla
 * Changed the backplane window configuration from 4MB to 1MB for PCI memory allocation.
 *
 * Revision 1.4.2.1  2012-11-05 15:55:03  hchalla
 * Added support to test 5 tr 80x in a 3U VPX daisy chained backplane.
 *
 * Revision 1.4  2011-11-14 17:18:29  hchalla
 * Added new test for tr803 idt reading temperature test and other updates.
 *
 * Revision 1.3  2011/09/29 14:38:37  hchalla
 * Minor Changes for release of TR 80x V1.01.
 *
 * Revision 1.2  2011/08/26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
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
#include <bit/pci.h>

#include <devices/hda.h>
#include <devices/enxmc8619.h>
#undef	E__TEST_NO_MEM			// E_TEST_NO_MEM not used in this file
#include <devices/eth.h>

#include "cctboard.h"

/* defines */
//#define DEBUG
//#define DEBUG_SOAK

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

#define PCIE_SWITCH_NO_EEPROM_ERROR			(E__BIT + 0x6201)
#define PCIE_SWITCH_EEPROM_RSVD_ERROR		(E__BIT + 0x6202)
#define PCIE_SWITCH_EEPROM_TIMEOUT_ERROR	(E__BIT + 0x6203)

#define PORT_61		0x61		/* NMI Status & Control register */
#define SPKR_EN		0x02
#define SPKR_GATE	0x01

#define COM1		0			/* array index for numbered COM ports */
#define COM2		1
#define COM3		2
#define COM4		3

#define HDA_DEV			27
#define LPC_BRIDGE_DEV	31
#define GEN2_DEC		0x88	/* LPC I/F Generic decode range 2 register*/

#define vReadByte(regAddr)                      (*(UINT8*)(regAddr))
#define vReadWord(regAddr)                      (*(UINT16*)(regAddr))
#define vReadDWord(regAddr)                     (*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)       (*(UINT8*)(regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define PCI_PRI_BUS_NUM                         0x18
#define PCI_SEC_BUS_NUM                         0x19
#define PCI_SUB_BUS_NUM                         0x1A

/* typedefs */

/* constants */

//const UINT8			bDebugPort = COM1;	/* COM1 is debug console */
const UINT8			bDebugPort = COM2;	/* COM2 is debug console */

SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO        */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-       */
	{ 3,	"Ch 2",		0,	0 },	/* COM3 Port 1 in P17C9X7954 */
	{ 4,	"Ch 3",		0,	0 },	/* COM4 Port 2     -"-       */
	{ 5,	"Ch 4",		0,	0 },	/* COM5 Port 3     -"-       */
	{ 6,	"Ch 5",		0,	0 },	/* COM6 Port 4     -"-       */
	{ 0,	"",		0,		0 }
};

// Audio codec definitions
const HDA_WIDGET WidgetList[] = {
	{0x0a, HDA_MICROPHONE},
	{0x0e, HDA_LINE_OUT},
	{0x0f, HDA_LINE_IN},
	{0x00, 0x00}
};

const HDA_CODEC CodecList[] = {
	{0, 0x111d76d5, TRUE, 0x21, WidgetList},
	{0, 0x0000, 0}
};

const HDA_DEVICE HdaList[] = {
	{PCI_MAKE_PFA (0, 27, 0), CodecList},
	{0, NULL}
};

static RTM_PCIE_SWITCH_PORT_INFO rtmSwitchPortInfo[7] = 
{
	{1,  7, 9,  0x10B5, 0x8733},	// RTM port 1 is connected to downstream port 9 of 7th instance of PEX8733 (upstream port)
	{2,  1, 8,  0x10B5, 0x8733},	// RTM port 2 is connected to downstream port 8 of 1st instance of PEX8733 (upstream port)
	{3,  1, 9,  0x10B5, 0x8733},	// RTM port 3 is connected to downstream port 9 of 1st instance of PEX8733 (upstream port)
	{8,  7, 10, 0x10B5, 0x8733},	// RTM port 8 is connected to downstream port 10 of 7th instance of PEX8733 (upstream port)
	{9,  7, 11, 0x10B5, 0x8733},	// RTM port 9 is connected to downstream port 11 of 7th instance of PEX8733 (upstream port)
	{10, 1, 10, 0x10B5, 0x8733},	// RTM port 10 is connected to downstream port 10 of 1st instance of PEX8733 (upstream port)
	{11, 1, 11, 0x10B5, 0x8733}		// RTM port 11 is connected to downstream port 11 of 1st instance of PEX8733 (upstream port)
};

static RTM_PCIE_SWITCH_INFO rtmSwitchInfo = 
{
	13,						// RTM switch is the 13th instance of PEX8733
	20,						// 20 ports in total for all switches on both RTM and VRE1x
	7,						// 7 ports to test on RTM
	7,						// RTM switch is connected upstream port of the 7th instance of PEX8733
	8,						// RTM upstream port is connected to this downstream port of the host switch on VRE1x
	0x10B5,					// host switch vendor id
	0x8733,					// host switch device id
	&rtmSwitchPortInfo[0]
};

// XMC GPIO loopback signal pairs 
static ENXMC_GPIO_SIGNAL_PAIRS xmcGpioSignals[] = 
{
	{0, 2}, {1, 3}, {4, 6}, {5, 7}, {8, 10}, {9, 11}, 
	{12, 14}, {13, 15},	{16, 18}, {17, 19}, {20, 22}, 
	{21, 23}, {24, 26}, {25, 27},
	{0xFF, 0xFF}											// end of list
};

// board specific XMC GPIO loopback settings
static ENXMC_GPIO_LOOPBACK_INFO xmcLoopbackInfo = 
{
	3,											// plx8619Instance
	&xmcGpioSignals[0],							// gpio table
	0x0A0A0A0AL,								// PLX PEX819 GPIO_CONFIG1 register: GPIO 0,1,4,5,8,9,12,13 outputs, 2,3,6,7,10,11,14,15 inputs
	0x000A0A0AL,								// PLX PEX819 GPIO_CONFIG2 register: GPIO 16,17,20,21,24,25 outputs, 18,19,22,23,26,27 inputs
	0x03333333L,								// bitmap of GPIOs configured as outputs
	0x0CCCCCCCL,								// bitmap of GPIOs configured as inputs
};

/* locals */
static volatile UINT32  mapRegSpace;            //PCI registers

/* globals */

/* externals */

extern unsigned int lwIPAppsIpAddr( void );

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

	UINT8	bConsPort;

	bConsPort = dIoReadReg(STATUS_REG_1, REG_8) & CONS_PORT;		// COM1 or COM2 from SW2-1
	
	if (bConsPort == 0)
		*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;
	else
		*((UINT16*)ptr) = asSccParams[COM2].wBaseAddr;

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
	int		i;
	UINT8	bPeri;
	UINT8	bIrq;
	UINT16	wPcicmd;
	UINT16	consp;
	UINT16	wBaseAddr;
	PCI_PFA	pfa;
#ifdef DEBUG
	char	achBuffer[80];
#endif


	//check if board is in peripheral mode
	bPeri = dIoReadReg(STATUS_REG_0, REG_8);
	bPeri = bPeri & SYS_CON;

	// get COM3 - COM6 I/O address and interrupt
	if (iPciFindDeviceById (1, 0x12D8, 0x7954, &pfa) == E__OK)
	{
		wBaseAddr = ((UINT16) PCI_READ_DWORD (pfa, 0x10)) & 0xFFFE;
#ifdef DEBUG
        sprintf(achBuffer,"P17C9X7954 BAR I/O Address  : %04X\n", wBaseAddr);
        vConsoleWrite(achBuffer);
#endif
		bIrq = PCI_READ_BYTE (pfa, 0x3C);
#ifdef DEBUG
        sprintf(achBuffer,"P17C9X7954 Interrupt         : %02X\n", bIrq);
        vConsoleWrite(achBuffer);
#endif
		if (bIrq == 0)
		{
			bIrq = 16;
			PCI_WRITE_WORD(pfa, 0x3C, bIrq);			// select PIRQ16
		}

		wPcicmd = PCI_READ_BYTE(pfa,0x4);
#ifdef DEBUG
		sprintf(achBuffer, "pcicmd %04x\n", wPcicmd);
		vConsoleWrite (achBuffer);
#endif

		wPcicmd |= PCI_BMEN | PCI_MEMEN | PCI_IOEN;
		PCI_WRITE_WORD(pfa, 0x4, wPcicmd);
	}
	else
		return E__FAIL;

	// update asSccParams structure
	for (i = 2; i < 6; ++i)
	{
		if (i == 5)
		{
			asSccParams[i].wBaseAddr = wBaseAddr + 0x38;
		}
		else
		{
			asSccParams[i].wBaseAddr = wBaseAddr + ((i - 2) * 8);
		}
		asSccParams[i].bIrq = bIrq;
#ifdef DEBUG
        sprintf(achBuffer,"P17C9X7954 COM%d Address  : %04X\n", (i + 1), asSccParams[i].wBaseAddr);
        vConsoleWrite(achBuffer);
#endif
	}

	//get console port base address
	brdGetConsolePort (&consp);

/*
 * - check soak/MTH mode, if not peripheral and in soak then soak port can't be tested,
 *   if peripheral mode then soak port can be tested, if MTH then console port can't be tested
 * - Check if in IBIT, if in IBIT then console port can't be tested
 * - check debug mode, if debug mode enabled then debug port can't be tested
 * */

	/* Build a port map */
	i = 0;
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
/*
		//skip soak port, COM1 for tr80x, in STH mode on system controller
		if(asSccParams[i].bPort == 1)
		{
			if( (bGetTestHandler() == TH__STH) && (bPeri == 0x080))
			{
				i++;
				continue;
			}
		}
*/

		//skip debug port

		if(asSccParams[i].bPort == bDebugPort)
			if(sysIsDebugopen() == bDebugPort)
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

	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

	// GPIO_16 & GPIO_15
	dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

	bVal = (UINT8)((dTemp >> 15) & 0x03);

	// GPIO_50 & GPIO_52
	dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);
	dTemp >>= 16;
	bVal |= (UINT8)(((dTemp & 0x10) == 0 ? 0 : 0x04) + ((dTemp & 0x04) == 0 ? 0 : 0x08));

	//ioStraps = *((UINT8*)ptr);
	switch (bVal)
	{
		case 4:
			ramSize = 16;
			break;

		case 5:
			ramSize = 8;
			break;

		case 6:
			ramSize = 32;
			break;

		case 0x0D:
			ramSize = 4;
			break;
			
		default:
			ramSize = 0;
			break;
	}

	*((UINT8*)ptr) = ramSize;

	return E__OK;

} /* brdGetDramSize () */



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

	bTemp = sysInPort8 (STATUS_REG_0);
	bTemp = bTemp & ~USER_LED;

	while (1)
	{
		sysOutPort8 (STATUS_REG_0, bTemp | USER_LED);		/* 300mS on */
		sysDelayMilliseconds (300);

		sysOutPort8 (STATUS_REG_0, bTemp);				/* 200mS off */
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
	PCI_PFA	pfa;
	UINT16	wGPIOBase;
	UINT32  gtemp;
	
	// Read GPIO 46 - 0 = STH, 1 = MTH

	pfa = PCI_MAKE_PFA (0, 31, 0);
	wGPIOBase = 0xFFFE & PCI_READ_WORD (pfa, 0x48);

	gtemp = sysInPort32 (wGPIOBase + 0x38);

	if ((gtemp & 0x00004000) == 0x00)
		*((UINT8*)ptr) =  SW__TEST;
	else
		*((UINT8*)ptr) =  SW__USER;

	return E__OK;
} /* brdCheckUserMthSwitch () */


/*****************************************************************************
 * brdGetSokePort: report the stdin/stdout console device
 *
 * RETURNS: base address of port
 */

UINT32 brdGetSokePort (void *ptr)
{
	UINT16 wBaseAddr;

	brdGetConsolePort (&wBaseAddr);	
	*((UINT16*)ptr) = wBaseAddr;

	return E__OK;

} /* brdGetSoakPort () */


/*****************************************************************************
 * brdGetFlashDriveParams: report parameters for the Flash Drive module
 *
 * RETURNS: base address of port
 */

UINT32 brdGetFlashDriveParams (void *ptr)
{
	((FLASH_DRV_PARAMS*)ptr)->bInstance = 1;
	((FLASH_DRV_PARAMS*)ptr)->bport     = 3;

	return E__OK;

} /* brdGetFlashDriveParams () */


UINT32 brdGetIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));
    if (reg & 0x20)
		*((UINT8*)ptr) =1;
	else
		*((UINT8*)ptr) =0;

	return E__OK;
}

UINT32 brdClearIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));

	reg = reg & ~(0x20);

	vIoWriteReg (0x212, REG_8, reg);
	
	return E__OK;
}

UINT32 brdGetVpxBrInstance (void *ptr)
{
	((VPX_INSTANCE_PARAMS*)ptr)->bInstance[0] = 1;
	return (E__OK);
}



/*****************************************************************************
 * brdSelectEthInterface: selects front or rear Ethernet interface
 *
 * RETURNS: success or error code
 */


UINT32 brdSelectEthInterface (void *ptr)
{
	return E__FAIL;
}


/*****************************************************************************
 * brdSkipEthInterface: skip ethernet controller used for network interface
 *
 * RETURNS: success or error code
 */

UINT32 brdSkipEthInterface (void *ptr)
{
	UINT8	bDev, bFunc;
	PCI_PFA	pfa;
	UINT16	wGPIOBase;
	UINT32	gtemp;
	UINT32	didVid;
	UINT8	bTestHandler;
	UINT8	bReqInstance;

	// Read GPIO 35 (RC Variant) - to determine which interface is used for USM
	// 0 = Not RC (i210), 1 = RC (i350)

	pfa = PCI_MAKE_PFA (0, 31, 0);
	wGPIOBase = 0xFFFE & PCI_READ_WORD (pfa, 0x48);

	gtemp = sysInPort32 (wGPIOBase + 0x38);

	// check GPIO35 for RC variant
	if ((gtemp & 0x00000008) == 0x00)
	{
		didVid = DIDVID_i210;					// Non-RC variant uses first i210
		bReqInstance = 1;
	}
	else
	{
		didVid = DIDVID_i350AMx_C;				// RC variant uses first i350 Copper
		bReqInstance = 2;
	}

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

/*****************************************************************************
 * brdGetRS485PortMap: provide a map of ports available for testing
 * ptr	  : pointer to a UINT32 bitmap - when bit set indicates port available,
 *          bit-0 = COM1
 * RETURNS: E__OK
 */
UINT32 brdGetRS485PortMap (void* ptr)
{
	UINT32	dPortMap = 0;
//	int		i = 0;
	UINT16  consp;

	//get console port base address
	brdGetConsolePort (&consp);

/*
 * - check soak/MTH mode, if not peripheral and in soak then soak port can't be tested,
 *   if peripheral mode then soak port can be tested, if MTH then console port can't be tested
 * - Check if in IBIT, if in IBIT then console port can't be tested
 * - check debug mode, if debug mode enabled then debug port can't be tested
 * */

	// only COM1 supports RS485
	if( asSccParams[0].wBaseAddr != consp)
		dPortMap |= (UINT32)1 << asSccParams[0].bPort;

	*((UINT32*)ptr) = dPortMap;

	return E__OK;
} /* brdGetRS485PortMap() */

/*****************************************************************************
 * brdSetSpeedStep: Check all speed ratios can be set
 * ptr	  : pointer for speedstep test result
 * RETURNS: E__OK
 */
UINT32 brdSetSpeedStep(void *ptr)
{
	UINT32 msrHigh=0,msrLow=0,timeout;
	UINT8 maxRatio=0,minRatio=0,i=0;
#ifdef DEBUG
	CPUID_REGS sRegs;
	char achBuffer[80];
#endif

#ifdef DEBUG
	// check if speedsteep is supported and enabled
	sysCpuid (1, (void*)&sRegs);
	sprintf (achBuffer, "CPUID 1 [ECX]: %08x - Speedstep Feature Flag = %08x\n", sRegs.ecx, sRegs.ecx & 0x00000080);
	sysDebugWriteString(achBuffer);

	sysCpuid (6, (void*)&sRegs);
	sprintf (achBuffer, "CPUID 6 [EAX]: %08x - Factory Turbo Mode = %08x\n", sRegs.eax, sRegs.ecx & 0x00000002);
	sysDebugWriteString(achBuffer);
	sprintf (achBuffer, "CPUID 6 [EAX]: %08x - Turbo Mode Feature Flag = %08x\n", sRegs.eax, sRegs.ecx & 0x00000004);
	sysDebugWriteString(achBuffer);

	vReadMsr(0x1A0,&msrHigh,&msrLow);		// IA32_MISC_ENABLES
	sprintf (achBuffer, "MSR 0x1A0: %08x:%08x\n", msrHigh, msrLow);
	sysDebugWriteString(achBuffer);
	sprintf (achBuffer, "Turbo Mode Disable %08x = %08x\n", msrHigh, msrHigh & 0x00000040);
	sysDebugWriteString(achBuffer);
	sprintf (achBuffer, "Speedstep Enable %08x = %08x\n", msrLow, msrLow & 0x00010000);
	sysDebugWriteString(achBuffer);
#endif

	// Get the max ratio allowed
	// speed may have been restricted by the BIOS due to board build option
	vReadMsr(0x64C,&msrHigh,&msrLow);		// TURBO_ACTIVATION_RATIO (Maximum Non-Turbo ratio))
	
#ifdef DEBUG
	sprintf (achBuffer, "\nMSR 0x64C: %08x:%08x\n", msrHigh, msrLow);
	sysDebugWriteString(achBuffer);
#endif

	maxRatio = msrLow & 0xff;

	// Get the min & max ratios available for this CPU
	vReadMsr(0xce, &msrHigh, &msrLow);	// PLATFORM_INFO
	
#ifdef DEBUG
	sprintf (achBuffer, "MSR 0xCE: %08x:%08x\n", msrHigh, msrLow);
	sysDebugWriteString(achBuffer);
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
	sysDebugWriteString(achBuffer);
#endif

	for( i=minRatio; i <= maxRatio; i++ )
	{
		/*Set the intended ratio*/
		vReadMsr(0x199,&msrHigh,&msrLow);
	
#ifdef DEBUG
		sprintf (achBuffer, "MSR 0x199: %08x:%08x\n", msrHigh, msrLow);
		sysDebugWriteString(achBuffer);
#endif

#ifdef DEBUG
		sprintf ( achBuffer, "0x199 before=%x,%x\n",msrHigh,msrLow);
		sysDebugWriteString(achBuffer);
#endif

		msrLow &= 0xffff80ff;
		msrLow |= (i << 8) & 0x7f00;
		vWriteMsr(0x199,msrHigh,msrLow);
#ifdef DEBUG
		sprintf ( achBuffer, "0x199 After=%x,%x\n",msrHigh,msrLow);
		sysDebugWriteString(achBuffer);
#endif

		timeout=15;

		/*Check if the ratio has been set*/
		while( --timeout )
		{
			vDelay(10);
			vReadMsr(0x198,&msrHigh,&msrLow);

			if( ( (msrLow >> 8) & 0xff) == i )
			{
#ifdef DEBUG
				sprintf ( achBuffer, "Current P-State ratio=%x\n",((msrLow >> 8) & 0xff));
				sysDebugWriteString(achBuffer);
#endif
				break;
			}
		}

		if( !timeout )
		{
#ifdef DEBUG
			sprintf ( achBuffer, "Unable to set ratio=%x, Current %x\n", i, ((msrLow >> 8) & 0xff));
			sysDebugWriteString(achBuffer);

			vReadMsr(0x199,&msrHigh,&msrLow);
			sprintf ( achBuffer, "Perf CTL=%x:%x\n",msrHigh,msrLow);
			sysDebugWriteString(achBuffer);

			vReadMsr(0x198,&msrHigh,&msrLow);
			sprintf ( achBuffer, "Perf STS=%x:%x\n",msrHigh,msrLow);
			sysDebugWriteString(achBuffer);
#endif

			vReadMsr(0x19C,&msrHigh,&msrLow);
			if ((msrLow & 0x02) == 0x02)
			{
#ifdef DEBUG
				sprintf ( achBuffer, "Therm STS=%x:%x\n",msrHigh,msrLow);
				sysDebugWriteString(achBuffer);
#endif
				*((UINT32*)ptr) =  SPEEDSTEP_PROCHOT_ERROR;
				return (E__OK);
			}

#ifdef DEBUG
			sprintf(achBuffer,"**FAIL** : Speed Step CPU Temperature: %d\n\n", getCPUTemp());
			sysDebugWriteString(achBuffer);
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
	UINT32 msrHigh=0,msrLow=0, dTemp = 0;
	vReadMsr(0x19C,&msrHigh,&msrLow);	// Therm Status
	dTemp = (msrLow >> 16) & (0x000000FF);
	return (dTemp);
}
#endif

/*****************************************************************************
 * brdDecodeTpmIo: Decode IO address for TPM registers
 *
 * RETURNS: success or error code
 */
UINT32 brdDecodeTpmIo (void *ptr)
{
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA(0, LPC_BRIDGE_DEV, 0);
	PCI_WRITE_DWORD(pfa, GEN2_DEC, 0x000C03B1);					// address decode 2 (3b0-3b3)
	return E__OK;
}

/*****************************************************************************
 * brdDecodeSmicIo: Decode SMIC registers for IPMI interface
 *
 * RETURNS: success or error code
 */
UINT32 brdDecodeSmicIo (void *ptr)
{
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA(0, LPC_BRIDGE_DEV, 0);
	PCI_WRITE_DWORD(pfa, GEN2_DEC, 0x000C0CA9);					// address decode 2 (ca0-caf)
	return E__OK;
}

/*****************************************************************************
 * brdEnableHDA: Decode HDA memory BARs
 *
 * RETURNS: success or error code
 */
UINT32 brdEnableHDA (void *ptr)
{
	PCI_PFA	pfa;
	
	if (ptr != NULL)
		pfa = *(PCI_PFA *)ptr;
	else
		pfa = PCI_MAKE_PFA(0, HDA_DEV, 0);
		
	PCI_WRITE_WORD(pfa, 0x04, 0x0006);						// enable BAR memory accesses
	return E__OK;
}

/*****************************************************************************
 * brdGetCodecList: Get List of expected codecs and widgets
 *
 * RETURNS: success or error code
 */
UINT32 brdGetHdaList (void *ptr)
{
	*((HDA_DEVICE **)ptr) = &HdaList[0];
	return E__OK;
}

/*****************************************************************************
 * brdGetRtmPex8733: Provide the instance of the root port of the RTM PEX8733
 *
 * RETURNS: success
 */
UINT32 brdGetRtmPex8733 (void *ptr)
{
	// root port of PLX8733 on RTM is the 13th PEX8733 device found by iPciFindDeviceById()
	*((RTM_PCIE_SWITCH_INFO**)ptr) = &rtmSwitchInfo;
	return E__OK;
}



/*****************************************************************************
 * brdGetXmcGpioInfo: Provide the XMC GPIO loopback configuration 
 *
 * RETURNS: success
 */
UINT32 brdGetXmcGpioInfo (void *ptr)
{
	// provide XMC loopback configuration
	*((ENXMC_GPIO_LOOPBACK_INFO**)ptr) = &xmcLoopbackInfo;
	return E__OK;
}


/*****************************************************************************
 * brdGetSlotId: Provide the VPX Slot ID for the USM 
 *
 * RETURNS: success
 */
 
UINT32 brdGetSlotId( void* ptr )
{
	UINT16 vpxSlotId;
	UINT32	ipAddr;

	ipAddr = lwIPAppsIpAddr();							// fetch IP address allocated by the DHCP Server
	vpxSlotId = (UINT16) ((ipAddr >> 24) & 0xFF);		// use the last byte of the IP address for the slot ID

#ifdef DEBUG
	sysDebugPrintf("%s: ipAddr: 0x%08X = %u.%u.%u.%u\n", 
						__FUNCTION__, ipAddr,
						(UINT8) (ipAddr & 0xFF),
						(UINT8) ((ipAddr >> 8) & 0xFF),
						(UINT8) ((ipAddr >> 16) & 0xFF),
						(UINT8) ((ipAddr >> 24) & 0xFF));
	sysDebugPrintf("%s: vpxSlotId: 0x%03X\n", __FUNCTION__, vpxSlotId);
#endif

	*((UINT16*)ptr) = vpxSlotId;

	return E__OK;
}
