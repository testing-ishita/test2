
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

/* includes */

#include <stdtypes.h>
#include <string.h>
#include <stdio.h>
#include <private/port_io.h>
#include <private/sys_delay.h>
#include <private/debug.h>
#include <bit/bit.h>
#include <bit/io.h>
#include <bit/pci.h>
#include <bit/delay.h>
#include <bit/board_service.h>
#include "cctboard.h"
#include <bit/mem.h>
#include <bit/msr.h>
#include <devices/eth.h>

/* defines */

//#define DEBUG

#define PORT_61		0x61		/* NMI Status & Control register */
#define SPKR_EN		0x02
#define SPKR_GATE	0x01

#define COM1		0			/* array index for numbered COM ports */
#define COM2		1
#define COM3		2

#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

#define VID_INTEL		0x8086
#define DID_82580_EB    0x1510
#define DIDVID_82580_EB  ((DID_82580_EB << 16) | VID_INTEL)

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

#define DEBUG
/* typedefs */

/* constants */

const UINT8			bDebugPort = COM1;	/* COM1 is debug console */

const SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO 	  */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-    */
	{ 3,	"Ch 2", 0x03E8, 11 },
	{ 0,	"",		0,		0 }
};

/* locals */
typedef struct{
	UINT16 regOffset;
	UINT32 dwValue;
}PCI_REG_DATA;
typedef struct {
	UINT8 bBus,bDevice,bFunc;
	UINT8 nNumRegs;
	PCI_REG_DATA pciRegData[8];
}PCI_REG_CONFIG;

PCI_REG_CONFIG pciStoredConfig;

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * brdGetConsolePort: report the stdin/stdout console device
 * ptr	  : pointer to a UINT16 for base address of port
 * RETURNS: E__OK
 */

UINT32 brdGetConsolePort (void* ptr)
{
	
	*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;

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

		//skip debug port, COM1 for vx813, if debug channel open
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
	(void)ptr;

	/* First try the PCI reset port */
	sysDelayMilliseconds(200);
	sysOutPort8 (0x0CF9, 0x6);
	

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

	umswitch = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8));

	if ((umswitch & USER_TEST_MTH_STH) == USER_TEST_MTH_STH)
		*((UINT8*)ptr) =  1; /*STH*/
	else
		*((UINT8*)ptr) =  0; /*MTH*/

	return E__OK;

} /* brdCheckUserMthSwitch () */


UINT32 brdGetIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8));
    if (reg & IPMI_NMI)
		*((UINT8*)ptr) =1;
	else
		*((UINT8*)ptr) =0;

	return E__OK;
}

UINT32 brdclearIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8)) & (~IPMI_NMI);
    vIoWriteReg(STATUS_REG_1, REG_8, reg );

	return E__OK;
}


/*****************************************************************************
 * brdSkipEthInterface: selects front or rear Ethernet interface
 *
 * RETURNS: success or error code
 */

UINT32 brdSkipEthInterface (void *ptr)
{
	UINT8 val;

	brdCheckUserMthSwitch( &val );

	if(val == 1)
	{
		if(((SkipEthIf_PARAMS*)ptr)->dDidVid   == DIDVID_82580_EB)
			return E__OK;
		else
			return 0x01;
	}
	else
	{
		return 0x01;
	}

} /* brdSelectEthInterface () */


UINT32 brdConfigCpciBackplane (void *ptr)
{
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA (0, 1, 0);
	//PCI- PCI Bridge
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
    PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x1);
    PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0xe); //A


	PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);
    PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);


	// PLX Switch - upstream
	pfa = PCI_MAKE_PFA (1,0,0);
    PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x1);
    PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x2);
    PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0xE);//A


	PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);
    PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);

	// PLX Switch - Down Stream
	pfa = PCI_MAKE_PFA (2, 1, 0);
    PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x2);
    PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x3);
    PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0xc); //9


	PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);
    PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);


	// PLX 8624
	pfa = PCI_MAKE_PFA (3, 0, 0);
    PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x3);
    PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x4);
    PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x9); //9


	PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);
    PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);


	// Pericom
	pfa = PCI_MAKE_PFA (4, 6, 0);
    PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x4); //4
    PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x7);//7
    PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x7); //7


	PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);
    PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);

	pfa = PCI_MAKE_PFA (0x7, 0, 0);

	PCI_WRITE_DWORD(pfa, 0xEC, 0xFF000000);
	PCI_WRITE_DWORD(pfa, 0xE8, 0x4000000);

	*((PCI_PFA*)ptr) = pfa;

    return (E__OK);

}

UINT32 brdDeConfigCpciBackplane (void *ptr)
{
	(void)ptr;
	 PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA (0, 1, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);

	pfa = PCI_MAKE_PFA (2, 1, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);

	pfa = PCI_MAKE_PFA (3, 0, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);

	pfa = PCI_MAKE_PFA (1,0,0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);

	pfa = PCI_MAKE_PFA (4, 6, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);

    return (E__OK);

}

UINT32 brdGetCpciBrInstance (void *ptr)
{
	UINT32 dInstance = 0;
	(void)ptr;

	dInstance = 1;
	*((UINT32*)ptr) = dInstance;

	return (E__OK);
}

UINT32 brdCheckCpciIsSyscon (void *ptr)
{

	*((UINT8*)ptr) = 1;
	 return (E__OK);


}

UINT32 brdGetAmcPcieLinkInfo( void *ptr )
{
	AMC_PCIE_LINK_INFO *lInfo=(AMC_PCIE_LINK_INFO*)ptr;
	UINT8  brdMode;

	brdCheckUserMthSwitch( &brdMode );

	lInfo->Pfa = PCI_MAKE_PFA (2, 1, 0);

	if( brdMode == 0  )
	{
		lInfo->gen = 2;
	}
	else
	{
		lInfo->gen = 0;
	}

	return (E__OK);
}
