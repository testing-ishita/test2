
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/trb1x.msd/board_cfg.c,v 1.9 2015-02-27 17:20:33 mgostling Exp $
 *
 * $Log: board_cfg.c,v $
 * Revision 1.9  2015-02-27 17:20:33  mgostling
 * Use board specific GPIO strap readings to detect RAM size
 *
 * Revision 1.8  2014-12-17 12:55:03  mgostling
 * Do not skip ethernet interface in LBIT mode.
 *
 * Revision 1.7  2014-12-15 14:34:43  mgostling
 * Do not skip ethernet interface in MTH mode.
 *
 * Revision 1.6  2014-07-29 11:39:29  mgostling
 * Implement SERVICE__BRD_SKIP_ETH_IF so that the network interface
 * to the Universal Soak Master is not modified during ethernet device testing.
 *
 * Revision 1.5  2014-06-13 09:59:20  mgostling
 * Corrected speedstep test to allow for build configuration that restricts the maximum CPU speed.
 *
 * Revision 1.4  2014-03-27 11:56:46  mgostling
 * Added a new test to vefify XMC differention IO signals.
 *
 * Revision 1.3  2014-01-10 10:01:47  mgostling
 * Added new service to obtain DRAM size.
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

const UINT8			bDebugPort = COM1;		/* COM1 is debug console */

const SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO        */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-       */
	{ 0,	"",		0,		0 }
};

// XMC GPIO loopback signal pairs 
static ENXMC_GPIO_SIGNAL_PAIRS xmcGpioSignals[] = 
{
	{0, 2}, {1, 3}, {4, 6}, {5, 7}, {8, 10}, {9, 11}, {12, 14},
	{13, 15}, {16, 18}, {17, 19}, {20, 22}, {21, 23}, 
	{0xFF, 0xFF}											// end of list
};

// board specific XMC GPIO loopback settings
static ENXMC_GPIO_LOOPBACK_INFO xmcLoopbackInfo = 
{
	3,											// plx8619Instance
	&xmcGpioSignals[0],							// gpio table
	0x0A0A0A0AL,								// PLX PEX819 GPIO_CONFIG1 register: GPIO 0,1,4,5,8,9,12,13 outputs, 2,3,6,7,10,11,14,15 inputs
	0x00000A0AL,								// PLX PEX819 GPIO_CONFIG2 register: GPIO 16,17,20,21, outputs, 18,19,22,23, inputs
	0x00333333L,								// bitmap of GPIOs configured as outputs
	0x00CCCCCCL,								// bitmap of GPIOs configured as inputs
};


/* locals */
static volatile UINT32  mapRegSpace;            //PCI registers
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
	UINT8	bConsPort;

	bConsPort = dIoReadReg(0x210, REG_8) & 0x04;		// COM1 or COM2 from SW1-3
	
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
	int		i = 0;
	UINT8   bPeri;
	UINT16  consp;

	//check if board is in peripheral mode
	bPeri = dIoReadReg(0x31E, REG_8);
	bPeri = bPeri & 0x04;

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
		{
			if( (bGetTestHandler() == TH__STH) && (bPeri == 0x04))
			{
				i++;
				continue;
			}
		}
		//skip debug port, COM1 for TR501, if debug channel open
		if(asSccParams[i].bPort == 1)
		{
			if(sysIsDebugopen() == 1)
			{
				i++;
				continue;
			}
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

	if ((umswitch & 0x01) == 0x01)
		*((UINT8*)ptr) =  SW__USER;
	else
		*((UINT8*)ptr) =  SW__TEST;

	return E__OK;
} /* brdCheckUserMthSwitch () */




/*****************************************************************************
 * brdGetSokePort: report the stdin/stdout console device
 *
 * RETURNS: base address of port
 */

UINT32 brdGetSokePort (void *ptr)
{
	*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;

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
	UINT8	bTestHandler;

	bTestHandler = bGetTestHandler();
	if ((bTestHandler == TH__PBIT) || (bTestHandler == TH__STH ))
	{
		bDev = PCI_PFA_DEV((((SkipEthIf_PARAMS*)ptr)->pfa));
		bFunc = PCI_PFA_FUNC((((SkipEthIf_PARAMS*)ptr)->pfa));

		if ((((SkipEthIf_PARAMS*)ptr)->dDidVid == DIDVID_i350AMx_C) && 
			(((SkipEthIf_PARAMS*)ptr)->bInstance == 2) && (bDev == 0) && (bFunc == 1))
			return E__OK;
		else
			return 0x01;
	}
	else
		return E__FAIL;
}


/*****************************************************************************
 * brdConfigureVPXNTPorts: configures NT ports for VPX Backplane communication
 * RETURNS: success or error code
 * */
UINT32 brdConfigureVPXNTPorts(void *ptr)
{


  UINT8       bTemp,bSysCon;
      PCI_PFA pfa;
      UINT32 dBarAddress;
#ifdef DEBUG_SOAK
      char    buffer[64];
#endif


#if 0 //for debug code to fill buffers with pattern
	UINT32 i;
#endif


	/* - figure out if the both ports are configured as NT
	 * - configure memory windows for both NT ports, this happens
	 *   before PCI scan so appropriate PCI memory is allocated
	 */


    bSysCon = dIoReadReg(0x31E, REG_8);

    if ( (bSysCon & 0x04) != 0x04)
    {

        /*PLX Switch  bus allocation */
        pfa = PCI_MAKE_PFA(0, 0x1, 0);
        PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
        PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x1);
        PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x4);
        PCI_WRITE_WORD (pfa, 0x20, 0xF000);//F000
        PCI_WRITE_WORD (pfa, 0x22, 0xF450);//F450
        bTemp = PCI_READ_BYTE (pfa, 0x04);
        bTemp |= 0x07;
        PCI_WRITE_BYTE (pfa, 0x04, bTemp);


        pfa = PCI_MAKE_PFA(1, 0x0, 0);
        PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x1);
        PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x2);
        PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x4);
        //PCI_WRITE_WORD (pfa, 0x20, 0xDFA0);//F000
        //PCI_WRITE_WORD (pfa, 0x22, 0xDFD0);//F440
        bTemp = PCI_READ_BYTE (pfa, 0x04);
        bTemp |= 0x07;
        PCI_WRITE_BYTE (pfa, 0x04, bTemp);

        pfa = PCI_MAKE_PFA(2, 0x1, 0);
        PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x2);
        PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x3);
        PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x3);
        bTemp = PCI_READ_BYTE (pfa, 0x04);
        bTemp |= 0x07;
        PCI_WRITE_BYTE (pfa, 0x04, bTemp);

        pfa = PCI_MAKE_PFA(3, 0x0, 0);
        bTemp = PCI_READ_BYTE (pfa, 0x04);
        bTemp |= 0x07;
        PCI_WRITE_BYTE (pfa, 0x04, bTemp);

#if 0
        pfa = PCI_MAKE_PFA(2, 0x1, 0);
        PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x2);
        PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x3);
        PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x3);
        PCI_WRITE_WORD (pfa, 0x20, 0xF000);
        PCI_WRITE_WORD (pfa, 0x22, 0xF440);//440
        bTemp = PCI_READ_BYTE (pfa, 0x04);
        bTemp |= 0x07;
        PCI_WRITE_BYTE (pfa, 0x04, bTemp);

        pfa = PCI_MAKE_PFA(3, 0x0, 0);
        PCI_WRITE_DWORD (pfa, 0x10, 0xF4400000);
        bTemp = PCI_READ_BYTE (pfa, 0x04);
        bTemp |= 0x07;
        PCI_WRITE_BYTE (pfa, 0x04, bTemp);
#endif
#if 0
         pfa = PCI_MAKE_PFA(2, 0x1, 0);
         PCI_WRITE_DWORD (pfa, 0x10, 0xDFE80000);//F4400000
         bTemp = PCI_READ_BYTE (pfa, 0x04);
         bTemp |= 0x07;
         PCI_WRITE_BYTE (pfa, 0x04, bTemp);
#endif
        pfa = PCI_MAKE_PFA(1, 0x0, 0);
        PCI_WRITE_DWORD (pfa, 0x10, 0xF4400000);//F4400000
        dBarAddress = PCI_READ_DWORD (pfa, 0x10);
#ifdef DEBUG_SOAK
        sprintf(buffer,"dBarAddress  : %x %x\n", dBarAddress, PCI_READ_DWORD (pfa, 0x0));
        sysDebugWriteString(buffer);
#endif

        dBarAddress &= ~(0xf);
        mapRegSpace =  sysMemGetPhysPtrLocked((UINT64)dBarAddress, 0x40000); //<---------------------- not locked
        if(mapRegSpace == 0)
        {
#ifdef DEBUG_SOAK
            sysDebugWriteString("PHY Pointer FAIL!\n");
#endif
            return E__FAIL;
        }

#ifdef DEBUG_SOAK
        sprintf(buffer,"mapRegSpace  : %x\n", mapRegSpace);
        sysDebugWriteString(buffer);


        sprintf(buffer, "Vendor ID : %x\n", vReadDWord(mapRegSpace));
        sysDebugWriteString(buffer);
#endif

        // Setup BAR 2 NT Port Registers for Image 1 Interconnect
        vWriteDWord(mapRegSpace+0x3F000+0xE8,0xFFE00000);
#ifdef DEBUG_SOAK
        sprintf(buffer, "0xE8 : %x\n", vReadDWord(mapRegSpace+0x3F000+0xE8));
        sysDebugWriteString(buffer);
#endif
        vWriteDWord(mapRegSpace+0x3F000+0xC3C,0x00);
#ifdef DEBUG_SOAK
        sprintf(buffer, "0xC3C : %x\n", vReadDWord(mapRegSpace+0x3F000+0xC3C));
        sysDebugWriteString(buffer);
#endif

        // Setup BAR 3 NT Port Registers for Image 2 4MB RAM
        vWriteDWord(mapRegSpace+0x3F000+0xEC,0xFFC00000);
#ifdef DEBUG_SOAK
        sprintf(buffer, "0xEC : %x\n", vReadDWord(mapRegSpace+0x3F000+0xEC));
        sysDebugWriteString(buffer);
#endif
        vWriteDWord(mapRegSpace+0x3F000+0xC40,0x4000000);
#ifdef DEBUG_SOAK
        sprintf(buffer, "0xC40 : %x\n", vReadDWord(mapRegSpace+0x3F000+0xc40));
        sysDebugWriteString(buffer);
#endif


        // Enter Requester ID Translation Lookup Table
        //vWriteDWord(mapRegSpace+0x3E000+0xD94,0x80000000);
        //vWriteDWord(mapRegSpace+0x3F000+0xDB4,0x00010001);
        vWriteDWord(mapRegSpace+0x3E000+0xD94,0x01010001);
        vWriteDWord(mapRegSpace+0x3F000+0xDB4,0x01010001);
#ifdef DEBUG_SOAK
        sprintf(buffer, " Control Register 0x1DC : %x\n", vReadDWord(mapRegSpace+0x1DC));
        sysDebugWriteString(buffer);
#endif
        //vWriteDWord(mapRegSpace+0x1DC,(vReadDWord(mapRegSpace+0x1DC) | 0x20000000));
        vWriteDWord(mapRegSpace+0xA30,(vReadDWord(mapRegSpace+0xA30) | (1<<25)));

        pfa = PCI_MAKE_PFA(1, 0x0, 0);
        PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
        PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
        PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);

        pfa = PCI_MAKE_PFA(0, 0x1, 0);
        PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
        PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
        PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);
	}
    else
    {
        UINT8 bTemp, bCount;

        /* system controller, so delay a short while to allow */
        /* peripheral boards to configure themselves.             */
        bTemp = dIoReadReg (0x61,REG_8) & (0xfd);
        for (bCount = 0; bCount < 5; bCount++)
        {
                bTemp = dIoReadReg (0x211,REG_8) & (~0x01);
                vIoWriteReg (0x211,REG_8, bTemp | 0x01);
                //vIoWriteReg(0x61, REG_8, bTemp|0x02);
                vDelay (500);
                vIoWriteReg (0x211,REG_8,bTemp);
                //vIoWriteReg(0x61, REG_8, bTemp);
                vDelay (500);
        }
    }

    /* flash the user LED */
    bTemp = dIoReadReg (0x211,REG_8) & (~0x01);
    vIoWriteReg (0x211,REG_8, bTemp | 0x01);
    vDelay(500);
    vIoWriteReg (0x211,REG_8,bTemp);

#if 0
	bSysCon = dIoReadReg(0x31E, REG_8);

	if ( (bSysCon & 0x04) != 0x04)
	{
		/*PLX Switch  bus allocation */
		pfa = PCI_MAKE_PFA(0, 0x1, 0);
		PCI_WRITE_BYTE (pfa, PCI_PRI_BUS, 0);
		PCI_WRITE_BYTE (pfa, PCI_SEC_BUS, 0x1);
		PCI_WRITE_BYTE (pfa, PCI_SUB_BUS, 0x2);
		bTemp = PCI_READ_BYTE (pfa, 0x04);
		bTemp |= 0x07;
		PCI_WRITE_BYTE (pfa, 0x04, bTemp);

		pfa = PCI_MAKE_PFA(1, 0x0, 0);
		PCI_WRITE_BYTE (pfa, PCI_PRI_BUS, 0x1);
		PCI_WRITE_BYTE (pfa, PCI_SEC_BUS, 0x2);
		PCI_WRITE_BYTE (pfa, PCI_SUB_BUS, 0x2);
		bTemp = PCI_READ_BYTE (pfa, 0x04);
		bTemp |= 0x07;
		PCI_WRITE_BYTE (pfa, 0x04, bTemp);

		pfa = PCI_MAKE_PFA(1, 0x0, 1);
		bTemp = PCI_READ_BYTE (pfa, 0x04);
		bTemp |= 0x07;
		PCI_WRITE_BYTE (pfa, 0x04, bTemp);

		DIDVID = (PCI_READ_DWORD (pfa, 0));

		#ifdef DEBUG
			sprintf(buffer, "PCI Read %x\n", DIDVID );
			sysDebugWriteString (buffer);
		#endif

		if(DIDVID != 0xffffffff)
		{

			mHandle  = dGetPhysPtr(0xe0101000, 0x1000, &tPtr,(void*)(&pointer));

			#ifdef DEBUG
				sprintf(buffer, "Extended Read %x\n", *((UINT32*)pointer));
				sysDebugWriteString (buffer);
			#endif


			// SET Port B window 1 to size 1MB at offset 132MB of RAM
			//-------------------------------------------------------

			*((UINT32*)(pointer + 0xff8)) = 0x21470;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x800004C0;	// data


			*((UINT32*)(pointer + 0xff8)) = 0x21474;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0xFFFFFC00;	// data


			*((UINT32*)(pointer + 0xff8)) = 0x21478;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x00000000;	// data


			*((UINT32*)(pointer + 0xff8)) = 0x2147C;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x00000000;	// data


		  /* Set the BAR1 register to communicate with
		   * Interconnect Image for 4MB
		   */

			*((UINT32*)(pointer + 0xff8)) = 0x21480;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x80000140;	// data

			*((UINT32*)(pointer + 0xff8)) = 0x21484;	// addr
			//*((UINT32*)(pointer + 0xffc)) = 0xFFE000000;	// data
			*((UINT32*)(pointer + 0xffc)) = 0xFFB00000;	// data


			*((UINT32*)(pointer + 0xff8)) = 0x21488;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x00000000;	// data

			/* Set the BAR2 register to Co-perative Memory Tests
			 * Interconnect Image for 4MB
			 */
			*((UINT32*)(pointer + 0xff8)) = 0x21490;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x80000140;	// data /*0x800001A0*/

			*((UINT32*)(pointer + 0xff8)) = 0x21494;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0xFFB00000;	// data
			//*((UINT32*)(pointer + 0xffc)) = 0xFFE00000;	// data

			*((UINT32*)(pointer + 0xff8)) = 0x21498;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x4000000;	// data


			*((UINT32*)(pointer + 0xff8)) = 0x21400;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x0000002;	// data


			//Set the completion enable bit for the CPU upstream port
			memRegAddr = pointer + 0xFF8;
			*(UINT32*)memRegAddr = (PORT0_NT_REG_BASE+PORT_NTCTL);
			memRegAddr = pointer + 0xFFC;
			*(UINT32*)memRegAddr |= 0x2;

			//Set the completion enable bit
			memRegAddr = pointer + 0xFF8;
			*(UINT32*)memRegAddr = (PORT16_NT_REG_BASE+PORT_NTCTL);
			memRegAddr = pointer + 0xFFC;
			*(UINT32*)memRegAddr |= 0x2;


			//Lets add  NT mapping table entry for the CPU upstream port.
			memRegAddr = pointer + PORT_NTMTBLADDR;
			*(UINT32*)memRegAddr = 2;

			memRegAddr = pointer + PORT_NTMTBLDATA;
			*(UINT32*)memRegAddr = 0x20001;

			*((UINT32*)(pointer + 0xff8)) = 0x21400;	// addr
			*((UINT32*)(pointer + 0xffc)) = 0x0000002;	// data

#ifdef DEBUG
	sprintf(buffer, "Extended Read %x\n", *((UINT32*)(pointer+0xff8)));
	sysDebugWriteString (buffer);
	sprintf(buffer, "Extended Read %x\n", *((UINT32*)(pointer+0xffc)));
	sysDebugWriteString (buffer);
#endif

			vFreePtr(mHandle);
		}

		pfa = PCI_MAKE_PFA(1, 0x0, 0);
		PCI_WRITE_BYTE (pfa, PCI_PRI_BUS, 0);
		PCI_WRITE_BYTE (pfa, PCI_SEC_BUS, 0);
		PCI_WRITE_BYTE (pfa, PCI_SUB_BUS, 0);

		/*PLX Switch  bus allocation */
		pfa = PCI_MAKE_PFA(0, 0x1, 0);
		PCI_WRITE_BYTE (pfa, PCI_PRI_BUS, 0);
		PCI_WRITE_BYTE (pfa, PCI_SEC_BUS, 0);
		PCI_WRITE_BYTE (pfa, PCI_SUB_BUS, 0);
	}
	else
	{
		UINT8 bTemp, bCount;

		/* system controller, so delay a short while to allow */
		/* peripheral boards to configure themselves.		  */
		bTemp = dIoReadReg (0x61,REG_8) & (0xfd);
		for (bCount = 0; bCount < 5; bCount++)
		{
			bTemp = dIoReadReg (0x211,REG_8) & (~0x01);
			vIoWriteReg (0x211,REG_8, bTemp | 0x01);
			//vIoWriteReg(0x61, REG_8, bTemp|0x02);
			vDelay (500);
			vIoWriteReg (0x211,REG_8,bTemp);
			//vIoWriteReg(0x61, REG_8, bTemp);
			vDelay (500);
		}

		/* flash the user LED */
		bTemp = dIoReadReg (0x211,REG_8) & (~0x01);
		vIoWriteReg (0x211,REG_8, bTemp | 0x01);
		vDelay(500);
		vIoWriteReg (0x211,REG_8,bTemp);
	}
#endif


	return E__OK;

} /* brdConfigureVxsNTPorts () */


/*****************************************************************************
 * brdGetRS485PortMap: provide a map of ports available for testing
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

		//skip debug port, COM1 for TRB1x, if debug channel open
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

	pfa = PCI_MAKE_PFA(0, HDA_DEV, 0);
	PCI_WRITE_WORD(pfa, 0x04, 0x0006);						// enable BAR memory accesses
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
