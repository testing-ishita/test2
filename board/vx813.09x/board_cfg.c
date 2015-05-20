
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx813.09x/board_cfg.c,v 1.5 2014-11-14 17:05:43 mgostling Exp $
 *
 * $Log: board_cfg.c,v $
 * Revision 1.5  2014-11-14 17:05:43  mgostling
 * Corrected brdCheckUserMthSwitch to use correct register and bit to detect switch setting.
 * Updated brdGetXmcGpioInfo to correctly handle passed parameter from xmc_gpio_loopback_ADCR6.
 *
 * Revision 1.4  2014-07-11 08:48:53  mgostling
 * Updated XMC GPIO loopback definitions for ADCR6.
 *
 * Revision 1.3  2014-06-20 14:30:49  mgostling
 * Fixed parameter passing and retrieval for brdGetXmcGpioInfo function.
 * Added some debugging messages.
 *
 * Revision 1.2  2014-03-27 11:56:46  mgostling
 * Added a new test to vefify XMC differention IO signals.
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.6  2011/11/21 11:15:59  hmuneer
 * no message
 *
 * Revision 1.5  2011/10/27 15:48:23  hmuneer
 * no message
 *
 * Revision 1.4  2011/06/07 15:04:34  hchalla
 * Initial version of PBIT for VX813.
 *
 * Revision 1.3  2011/05/16 14:30:16  hmuneer
 * Info Passing Support
 *
 * Revision 1.2  2011/03/22 13:15:13  hchalla
 * Removed COM3 as this board does not support, added support to map RS485 port mapping as COM2 supports RS485/RS422.
 *
 * Revision 1.1  2011/02/28 11:57:43  hmuneer
 * no message
 *
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
#include <bit/mem.h>
#include <bit/msr.h>
#include <bit/console.h>

#include <devices/enxmc8619.h>

#include "cctboard.h"

/* defines */

//#define DEBUG

#define PORT_61		0x61		/* NMI Status & Control register */
#define SPKR_EN		0x02
#define SPKR_GATE	0x01

#define COM1		0			/* array index for numbered COM ports */
#define COM2		1
#define COM3		2
#define SPEEDSTEP_ERROR				(E__BIT + 0x1000)
#define SPEEDSTEP_PROCHOT_ERROR     (E__BIT + 0x1010)
/* typedefs */

/* constants */



const UINT8			bDebugPort = COM2;	/* COM1 is debug console
										 *
										 * IMPORTANT:
										 * If the debug console is changed
										 * to a different port, then "COMPort"
										 * at the start of  boot_real.S must be
										 * changed accordingly as well
										 **/

const SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO 	  */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-    */
	{ 0,	"",		0,		0 }
};



// ADCR6 XMC site 1 GPIO loopback signal pairs 
static ENXMC_GPIO_SIGNAL_PAIRS xmcSite1GpioSignals[] = 
{
	{22, 23}, {24, 25}, {26, 27}, {28, 29}, 
	{0xFF, 0xFF}								// end of list
};

// board specific XMC GPIO loopback settings
static ENXMC_GPIO_LOOPBACK_INFO xmcSite1LoopbackInfo = 
{
	3,											// plx8619Instance
	&xmcSite1GpioSignals[0],					// site 1 gpio table
	0x00000000L,								// PLX PEX819 GPIO_CONFIG1 register: GPIO 0 to 15 not used, set as inputs
	0x02222000L,								// PLX PEX819 GPIO_CONFIG2 register: GPIO 16 to 21 not used, 22,24,26,28 outputs, 23,25,27,29 inputs
	0x15400000L,								// bitmap of GPIOs configured as outputs
	0x2A800000L,								// bitmap of GPIOs configured as inputs
};

// ADCR6 XMC site 2 2GPIO loopback signal pairs 
static ENXMC_GPIO_SIGNAL_PAIRS xmcSite2GpioSignals[] = 
{
	{0, 2}, {1, 3}, {4, 6}, {5, 7}, {8, 10}, {9, 11},
	{12, 14}, {13, 15}, {16, 18}, {17, 19}, {20, 22}, 
	{21, 23}, {24, 26}, {25, 27}, 
	{0xFF, 0xFF}											// end of list
};

// board specific XMC GPIO loopback settings
static ENXMC_GPIO_LOOPBACK_INFO xmcSite2LoopbackInfo = 
{
	3,											// plx8619Instance
	&xmcSite2GpioSignals[0],					// site 2 gpio table
	0x0A0A0A0AL,								// PLX PEX819 GPIO_CONFIG1 register: GPIO 0,1,4,5,8,9,12,13 outputs, 2,3,6,7,10,11,14,15 inputs
	0x000A0A0AL,								// PLX PEX819 GPIO_CONFIG2 register: GPIO 16,17,20,21,24,25 outputs, 18,19,22,23,26,27,28,29 inputs
	0x03333333L,								// bitmap of GPIOs configured as outputs
	0x0CCCCCCCL,								// bitmap of GPIOs configured as inputs
};


/* locals */
#ifdef DEBUG
UINT32 getCPUTemp (void);
#endif


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
	UINT8	bConsPort;

	/* Serial console can be COM1 or COM3 */
	bConsPort = sysInPort8 (STATUS_REG_0) & COM_PORT;

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
	sysOutPort8 (0x0CF9, 0xe);
	sysOutPort8 (0x92, ((sysInPort8(0x92)) | 0x1));

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

	umswitch = ((UINT8)dIoReadReg(0x210, REG_8));
	
	if ((umswitch & 0x02) == 0x00)
		*((UINT8*)ptr) =  SW__USER;
	else
		*((UINT8*)ptr) =  SW__TEST;

	return E__OK;

} /* brdCheckUserMthSwitch () */


/*****************************************************************************
 * brdCheckUniverseJumper:
 *
 * RETURNS:
 * */
UINT32 brdCheckUniverseJumper (void *ptr)
{
	*((UINT8*)ptr) = 0;

	return E__OK;

} /* brdCheckUniverseJumper () */


/*****************************************************************************
 * brdSelectEthInterface: selects front or rear Ethernet interface
 * cnt 1/2  Iface 0/1
 * RETURNS: success or error code
 * */
UINT32 brdSelectEthInterface (void *ptr)
{
	PCI_PFA	pfa;
	UINT16	wGPIOBase;
	UINT32  gtemp;

	if( ((SelEthIf_PARAMS*)ptr)->cnt != 2)
		return E__TEST_WRONG_CONTROLLER;

	pfa = PCI_MAKE_PFA (0, 31, 0);
	wGPIOBase = 0xFFFE & PCI_READ_WORD (pfa, 0x48);

	//make pin GPIO15 & 57 GPIO output
	gtemp = sysInPort32 (wGPIOBase + 0x04);
	gtemp = gtemp & ~0x00008000;
	sysOutPort32 (wGPIOBase + 0x04, gtemp);

	gtemp = sysInPort32 (wGPIOBase + 0x30);
	gtemp = gtemp | 0x02000000;
	sysOutPort32 (wGPIOBase + 0x30, gtemp);
	gtemp = sysInPort32 (wGPIOBase + 0x34);
	gtemp = gtemp & ~0x02000000;
	sysOutPort32 (wGPIOBase + 0x34, gtemp);

	if(((SelEthIf_PARAMS*)ptr)->Iface == 0)
	{
		gtemp = sysInPort32 (wGPIOBase + 0x0c);
		gtemp = gtemp & ~0x00008000;
		sysOutPort32 (wGPIOBase + 0x0c, gtemp);
	}
	else if( ((SelEthIf_PARAMS*)ptr)->Iface == 1 )
	{
		gtemp = sysInPort32 (wGPIOBase + 0x0c);
		gtemp = gtemp | 0x00008000;
		sysOutPort32 (wGPIOBase + 0x0c, gtemp);
	}
	else
		return E__TEST_WRONG_INTERFACE;

	gtemp = sysInPort32 (wGPIOBase + 0x38);
	gtemp = gtemp  & ~0x02000000;
	sysOutPort32 (wGPIOBase + 0x38, gtemp);

	vDelay(1);

	gtemp = sysInPort32 (wGPIOBase + 0x38);
	gtemp = gtemp | 0x02000000;
	sysOutPort32 (wGPIOBase + 0x38, gtemp);

	return E__OK;

} /* brdSelectEthInterface () */


#define PORT_NTMTBLADDR		0x004D0
#define PORT_NTMTBLSTS		0x004D4
#define PORT_NTMTBLDATA		0x004D8
#define PORT_NTREQIDCAP		0x004DC

#define PORT0_NT_REG_BASE	0x01000
#define PORT8_NT_REG_BASE	0x11000
#define PORT12_NT_REG_BASE	0x19000

#define PORT_NTCTL			0x00400


/*****************************************************************************
 * brdConfigureVxsNTPorts: configures NT ports for VXS loop-back Test
 * RETURNS: success or error code
 * */
UINT32 brdConfigureVxsNTPorts(void *ptr)
{

#ifdef DEBUG
	UINT8  buffer[64];
#endif

	PTR48  tPtr;
	UINT32 mHandle;
	UINT32 pointer;
	UINT32 memRegAddr;
	PCI_PFA pfa;
	UINT8	bTemp;
	UINT32	regVal=0;
	UINT32  DIDVID;

#if 0 //for debug code to fill buffers with pattern
	UINT32 i;
#endif

	/* - figure out if the both ports are configured as NT
	 * - configure memory windows for both NT ports, this happens
	 *   before PCI scan so appropriate PCI memory is allocated
	 */

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

		// SET Port A window 1 to size 1MB at offset 128MB of RAM
		//-------------------------------------------------------
		//set Limit
		*((UINT32*)(pointer + 0xff8)) = 0x19484;	// addr
		*((UINT32*)(pointer + 0xffc)) = 0xfffffc00;	// data

		//set LTB
		*((UINT32*)(pointer + 0xff8)) = 0x19488;	// addr
		*((UINT32*)(pointer + 0xffc)) = 0x08000000;	// data

		//set UTB
		*((UINT32*)(pointer + 0xff8)) = 0x1948c;	// addr
		*((UINT32*)(pointer + 0xffc)) = 0x00000000;	// data

		//set UTB
		*((UINT32*)(pointer + 0xff8)) = 0x19480;	// addr
		*((UINT32*)(pointer + 0xffc)) = 0x80000180;	// data


		// SET Port B window 1 to size 1MB at offset 132MB of RAM
		//-------------------------------------------------------
		//set Limit
		*((UINT32*)(pointer + 0xff8)) = 0x11484;	// addr
		*((UINT32*)(pointer + 0xffc)) = 0xfffffc00;	// data

		//set LTB
		*((UINT32*)(pointer + 0xff8)) = 0x11488;	// addr
		*((UINT32*)(pointer + 0xffc)) = 0x08400000;	// data

		//set UTB
		*((UINT32*)(pointer + 0xff8)) = 0x1148c;	// addr
		*((UINT32*)(pointer + 0xffc)) = 0x00000000;	// data

		//set UTB
		*((UINT32*)(pointer + 0xff8)) = 0x11480;	// addr
		*((UINT32*)(pointer + 0xffc)) = 0x80000180;	// data


		//Set the completion enable bit for the CPU upstream port
		memRegAddr = pointer + 0xFF8;
		*(UINT32*)memRegAddr = (PORT0_NT_REG_BASE+PORT_NTCTL);
		memRegAddr = pointer + 0xFFC;
		*(UINT32*)memRegAddr |= 0x2;


		//Lets add  NT mapping table entry for the CPU upstream port.
		memRegAddr = pointer + PORT_NTMTBLADDR;
		*(UINT32*)memRegAddr = 0;

		memRegAddr = pointer + PORT_NTMTBLDATA;
		*(UINT32*)memRegAddr = 1;


		//Lets add  NT mapping table entry for the downstream port 12 (A).
		 memRegAddr = pointer + PORT_NTMTBLADDR;
		*(UINT32*)memRegAddr = 1;

		memRegAddr = pointer + PORT_NTMTBLDATA;
		regVal  = 0x1;						//Enable the valid bit
		regVal |= (0x2 << 17);				//Set the partition
		*(UINT32*)memRegAddr = regVal;

		//Set the completion enable bit
		memRegAddr = pointer + 0xFF8;
		*(UINT32*)memRegAddr = (PORT12_NT_REG_BASE+PORT_NTCTL);
		memRegAddr = pointer + 0xFFC;
		*(UINT32*)memRegAddr |= 0x2;


		//Lets add  NT mapping table entry for the downstream port 8 (B).
		memRegAddr = pointer + PORT_NTMTBLADDR;
		*(UINT32*)memRegAddr = 2;

		memRegAddr = pointer + PORT_NTMTBLDATA;
		regVal  = 0x1;						//Enable the valid bit
		regVal |= (0x1 << 17);				//Set the partition
		*(UINT32*)memRegAddr = regVal;

		//Set the completion enable bit
		memRegAddr = pointer + 0xFF8;
		*(UINT32*)memRegAddr = (PORT8_NT_REG_BASE+PORT_NTCTL);
		memRegAddr = pointer + 0xFFC;
		*(UINT32*)memRegAddr |= 0x2;

		vFreePtr(mHandle);

#if 0 //debug code to fill buffers with pattern
		mHandle  = dGetPhysPtr(0x08000000, 0x100000, &tPtr,(void*)(&pointer));
		for(i = 0; i<0x100000; i+=4)
			*((UINT32*)(pointer+i)) = 0xabcdabcd;
		sprintf(buffer, "window1 %x\n", *((UINT32*)pointer));
		sysDebugWriteString (buffer);
		vFreePtr(mHandle);

		mHandle  = dGetPhysPtr(0x08400000, 0x100000, &tPtr,(void*)(&pointer));
		for(i = 0; i<0x100000; i+=4)
		*((UINT32*)(pointer+i)) = 0x12341234;
		sprintf(buffer, "window1 %x\n", *((UINT32*)pointer));
		sysDebugWriteString (buffer);
		vFreePtr(mHandle);
#endif

	}

#if 0	// only enable if the above debug code is enabled
	else
		vDelay(2000);
#endif

	pfa = PCI_MAKE_PFA(1, 0x0, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS, 0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS, 0);

	/*PLX Switch  bus allocation */
	pfa = PCI_MAKE_PFA(0, 0x1, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS, 0);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS, 0);


	return E__OK;

} /* brdConfigureVxsNTPorts () */


UINT32 brdSetSpeedStep(void *ptr)
{
	UINT32 msrHigh=0,msrLow=0,timeout;
	UINT8 maxRatio=0,minRatio=0,i=0;
	char achBuffer[80];

    /*Get the min & max ratio supported*/
    vReadMsr(0xce,&msrHigh,&msrLow);

    maxRatio = (msrLow>>8) & 0xff;
    minRatio = (msrHigh>>8) & 0xff;

	for( i=minRatio; i <= maxRatio; i++ )
     {
    	 /*Set the intended ratio*/
        vReadMsr(0x199,&msrHigh,&msrLow);

		#ifdef DEBUG
			 sprintf ( achBuffer, "0x199 before=%x,%x\n",msrHigh,msrLow);
			 vConsoleWrite (achBuffer);
		#endif

		msrLow &= 0xffff00ff;
        msrLow |= (i << 8) & 0xff00;
        vWriteMsr(0x199,msrHigh,msrLow);
		#ifdef DEBUG
			sprintf ( achBuffer, "0x199 After=%x,%x\n",msrHigh,msrLow);
			vConsoleWrite (achBuffer);
		#endif

		timeout=10;

         /*Check if the ratio has been set*/
         while( --timeout )
         {
        	 vDelay(10);
            vReadMsr(0x198,&msrHigh,&msrLow);

            if( ( (msrLow >> 8) & 0xff) == i )
            {
				 #ifdef DEBUG
					sprintf ( achBuffer, "Current P-State ratio=%x\n",((msrLow >> 8) & 0xff));
					vConsoleWrite (achBuffer);
				 #endif
				 break;
            }
         }

         if( !timeout )
         {
			#ifdef DEBUG
				  sprintf ( achBuffer, "Unable to set ratio=%x, Current %x\n", i,
						    ((msrLow >> 8) & 0xff));
				  vConsoleWrite (achBuffer);

				  vReadMsr(0x199,&msrHigh,&msrLow);
				  sprintf ( achBuffer, "Perf CTL=%x:%x\n",msrHigh,msrLow);
				  vConsoleWrite (achBuffer);

				  vReadMsr(0x198,&msrHigh,&msrLow);
				  sprintf ( achBuffer, "Perf STS=%x:%x\n",msrHigh,msrLow);
				  vConsoleWrite (achBuffer);
			#endif

            vReadMsr(0x19C,&msrHigh,&msrLow);
            if ((msrLow & 0x02) == 0x02)
            {
            	sprintf ( achBuffer, "Therm STS=%x:%x\n",msrHigh,msrLow);
            	sysDebugWriteString(achBuffer);
            	*((UINT32*)ptr) =  SPEEDSTEP_PROCHOT_ERROR;
            	return (E__OK);
            }

			#ifdef DEBUG
				sprintf(achBuffer,"**FAIL**:Speed Step CPU Temperature:%d\n\n", getCPUTemp());
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
 * brdGetXmcGpioInfo: Provide the XMC GPIO loopback configuration 
 *
 * RETURNS: success
 */
UINT32 brdGetXmcGpioInfo (void *ptr)
{
	// provide XMC loopback configuration
	ENXMC_GPIO_LOOPBACK_INFO * loopbackInfo;
	UINT8	site;

	 loopbackInfo = *((ENXMC_GPIO_LOOPBACK_INFO**)ptr);
	 site = loopbackInfo->plx8619Instance;
	 
	// only used for XMC slots, PMC slots do not pass a site id
	 if (site > 2)
		site = 1;

	if	(((ENXMC_GPIO_LOOPBACK_INFO*)ptr)->plx8619Instance == 1)
	{
		*((ENXMC_GPIO_LOOPBACK_INFO**)ptr) =  &xmcSite1LoopbackInfo;
	}
	else
	{
		*((ENXMC_GPIO_LOOPBACK_INFO**)ptr) =  &xmcSite2LoopbackInfo;
	}

	return E__OK;
}
