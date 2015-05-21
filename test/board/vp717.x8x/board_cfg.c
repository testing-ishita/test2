
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp717.x8x/board_cfg.c,v 1.1 2015-04-02 11:41:18 mgostling Exp $
 *
 * $Log: board_cfg.c,v $
 * Revision 1.1  2015-04-02 11:41:18  mgostling
 * Initial check-in to CVS
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.11  2011/10/27 15:48:10  hmuneer
 * no message
 *
 * Revision 1.10  2011/06/07 15:38:50  hchalla
 * Added new service for setting the intel speed step ratio for speed step tests.
 *
 * Revision 1.9  2011/01/20 10:01:23  hmuneer
 * CA01A151
 *
 * Revision 1.7  2010/10/26 15:05:16  jthiru
 * Added delay to board reset -- Last test Flash access?
 *
 * Revision 1.6  2010/10/08 12:14:05  jthiru
 * Board reset twice 0xE written to 0xCF9 - Rev A
 *
 * Revision 1.5  2010/10/06 14:19:01  jthiru
 * Board reset changed to 0xE written to 0xCF9
 *
 * Revision 1.4  2010/10/06 10:12:08  jthiru
 * PBIT coding for VP717
 *
 * Revision 1.3  2010/09/15 12:32:42  hchalla
 * Fixed Build errors with VP 717
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
 * Revision 1.10  2009/06/02 11:58:06  cvsuser
 * Modified Board reset routine to be based on 0xCF9 to remove conflicts with Keyboard self test
 *
 * Revision 1.9  2009/05/19 08:09:53  cvsuser
 * no message
 *
 * Revision 1.7  2008/09/19 14:52:14  swilson
 * Add missing header and footer comments to memory functions. Add other hardware-specific
 *  functions.
 *
 * Revision 1.6  2008/07/24 14:37:36  swilson
 * Move PCI code out to pci_cfg.c
 *
 * Revision 1.5  2008/07/18 13:14:04  swilson
 * *** empty log message ***
 *
 * Revision 1.4  2008/07/02 10:52:42  swilson
 * Enable >4GB memory access via paged memory mechanism (PSE-36).
 *
 * Revision 1.3  2008/06/20 12:16:51  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.2  2008/05/14 09:59:06  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/05/12 14:50:07  swilson
 * Board-specific function implementations.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <string.h>

#include <private/port_io.h>
#include <private/sys_delay.h>
#include <private/debug.h>

#include <bit/bit.h>
#include <bit/console.h>
#include <bit/delay.h>
#include <bit/io.h>
#include <bit/board_service.h>
#include <bit/msr.h>

#include <cute/arch.h>

#include "cctboard.h"

/* defines */

#define PORT_61		0x61		/* NMI Status & Control register */
#define SPKR_EN		0x02
#define SPKR_GATE	0x01

#define COM1		0			/* array index for numbered COM ports */
#define COM2		1
#define COM3		2

#define SPEEDSTEP_ERROR				(E__BIT + 0x1000)
#define SPEEDSTEP_PROCHOT_ERROR     (E__BIT + 0x1010)

/* typedefs */

typedef struct{
	UINT16 regOffset;
	UINT32 dwValue;
} PCI_REG_DATA;

typedef struct {
	UINT8 bBus,bDevice,bFunc;
	UINT8 nNumRegs;
	PCI_REG_DATA pciRegData[8];
} PCI_REG_CONFIG;

/* constants */

const UINT8			bDebugPort = COM1;	/* COM1 is debug console
//const UINT8			bDebugPort = COM3;	/* COM3 is debug console - restore to COM1 for release
										 *
										 * IMPORTANT:
										 * If the debug console is changed
										 * to a different port, then "COMPort"
										 * at the start of  boot_real.S must be
										 * changed accordingly as well
										 **/

/*const*/ SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO     */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-    */
	{ 3,	"Ch 2", 0x03E8, 11 },	/* COM3 Port C     -"-    */
	{ 0,	"",		0,		0 }
};

/* locals */

#ifdef DEBUG
UINT32 getCPUTemp (void);
#endif

static PCI_REG_CONFIG pciStoredConfig;

static VME_DRV_INFO vmeDrvInfo;

/* globals */

/* externals */

extern unsigned int lwIPAppsIpAddr( void );
extern unsigned char * lwIPAppsMacAddr( void );

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

	bConsPort = sysInPort8 (STATUS_REG_2) & COM_PORT;

	if (bConsPort == 0)
		*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;
	else
		*((UINT16*)ptr) = asSccParams[COM3].wBaseAddr;

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

		//skip STH port
		if( asSccParams[i].bPort == 2)
		{
			if(bGetTestHandler() == TH__STH)
			{
				i++;
				continue;
			}
		}

		//skip debug port, COM1 for VP717, if debug channel open
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
	sysOutPort8 (0x0CF9, 0xE);
	sysOutPort8 (0x92, ((sysInPort8(0x92)) | 0x1));

	/* If that fails, try keyboard controller */
	sysOutPort8 (0x0064, 0xFE);

	return E__OK; // should never return

} /* brdReset () */




/*****************************************************************************
 * brdCheckUserMthSwitch:
 *
 * RETURNS: switch Flag
 *
 */
UINT32 brdCheckUserMthSwitch (void *ptr)
{
	UINT8	umswitch;

	umswitch = ((UINT8)dIoReadReg(0x211, REG_8));
	
	if ((umswitch & 0x40) == 0x00)
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
	*((UINT16*)ptr) = asSccParams[COM2].wBaseAddr;

	return E__OK;

} /* brdGetConsolePort () */


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
 * brdSkipEthInterface: skips front or rear Ethernet interface
 *
 * RETURNS: success or error code
 */
#define VID_INTEL		0x8086
#define DID_82576		0x10C9
#define DIDVID_82576    ((DID_82576 << 16) | VID_INTEL)

UINT32 brdSkipEthInterface (void *ptr)
{
	PCI_PFA	pfa;
	UINT16	wGPIOBase;
	UINT32	gtemp;
	UINT8	bTestHandler;
	UINT8	bInstance;

	// Read GPIO 48 (RC Variant) - to determine which interface to use
	// 0 = Not RC , 1 = RC

	pfa = PCI_MAKE_PFA (0, 31, 0);
	wGPIOBase = 0xFFFE & PCI_READ_WORD (pfa, 0x48);
	gtemp = sysInPort32 (wGPIOBase + 0x48);
	if ((gtemp & 0x00010000) == 0x00)
	{
		bInstance = 2;
	}
	else
	{
		bInstance = 1;
	}

	bTestHandler = bGetTestHandler();
	if ((bTestHandler == TH__PBIT) || (bTestHandler == TH__STH ))
	{
		if((((SkipEthIf_PARAMS*)ptr)->dDidVid   == DIDVID_82576) &&
		   (((SkipEthIf_PARAMS*)ptr)->bInstance == bInstance) )
			return E__OK;
		else
			return 0x01;
	}
	else
		return E__FAIL;

} /* brdSkipEthInterface () */
 



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



UINT32 brdSetSpeedStep(void *ptr)
{
	UINT32 msrHigh=0,msrLow=0,timeout;
	UINT8 maxRatio=0,minRatio=0,i=0;
	char achBuffer[80];

    /*Get the min & max ratio supported*/
    vReadMsr(0xce,&msrHigh,&msrLow);

    maxRatio = (msrLow>>8) & 0xff;
    minRatio = (msrHigh>>8) & 0xff;

    for( i=maxRatio; i >= minRatio; i-- )
     {

        /*Set the intended ratio*/
        vReadMsr(0x199,&msrHigh,&msrLow);
		#ifdef DEBUG
			 sprintf ( achBuffer, "0x199 before=%x,%x\n",msrHigh,msrLow);
			 vConsoleWrite (achBuffer);
		#endif
        msrLow &= 0xffff0000;
        msrLow |= i;
        vWriteMsr(0x199,msrHigh,msrLow);
		#ifdef DEBUG
			sprintf ( achBuffer, "0x199 After=%x,%x\n",msrHigh,msrLow);
			vConsoleWrite (achBuffer);
		#endif

       //timeout=1000;
			timeout=10;

        /*Check if the ratio has been set*/
        while( --timeout )
        {
       	 vDelay(10);
           vReadMsr(0x198,&msrHigh,&msrLow);

           if( (msrLow & 0xffff) == i )
           {
#ifdef DEBUG
           sprintf ( achBuffer, "Current P-State ratio=%d\n",(msrLow & 0xffff));
           vConsoleWrite (achBuffer);
#endif
             break;
           }
        }

        if( !timeout )
        {
#ifdef DEBUG
				sprintf ( achBuffer, "Unable to set ratio=%d, %x\n", i, (msrLow & 0xffff));
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
           sprintf(achBuffer,"\n4. **FAIL**:Speed Step CPU Temperature:%d\n", getCPUTemp());
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
 * brdCheckVMEBridgePresent:
 *
 * RETURNS: zero when VME bridge is absent
 */
UINT32 brdCheckVMEBridgePresent (void *ptr)
{
	UINT8	reg;

	reg = ((UINT8) dIoReadReg(STATUS_REG_4, REG_8));
	*((UINT8*)ptr) = reg & 0x01;

	return E__OK;
}

/*****************************************************************************
 * brdGetVMEBridgelessSlot:
 *
 * RETURNS: VME Geographic Address
 */
UINT32 brdGetVMEBridgelessSlot (void *ptr)
{
	UINT8	reg;

	reg = ((UINT8) dIoReadReg(0x31E, REG_8));
	*((UINT8*)ptr) = reg & 0x1F;

	return E__OK;
}

/*****************************************************************************
 * brdGetSlotId: Provide the VME Slot ID for the USM 
 *
 * RETURNS: success
 */
 
UINT32 brdGetSlotId( void* ptr )
{
	UINT16 vmeSlotId;
	UINT32	ipAddr;

	ipAddr = lwIPAppsIpAddr();							// fetch IP address allocated by the DHCP Server
	vmeSlotId = (UINT16) ((ipAddr >> 24) & 0xFF);		// use the last byte of the IP address for the slot ID

#ifdef DEBUG
	sysDebugPrintf("%s: ipAddr: 0x%08X = %u.%u.%u.%u\n", 
						__FUNCTION__, ipAddr,
						(UINT8) (ipAddr & 0xFF),
						(UINT8) ((ipAddr >> 8) & 0xFF),
						(UINT8) ((ipAddr >> 16) & 0xFF),
						(UINT8) ((ipAddr >> 24) & 0xFF));
	sysDebugPrintf("%s: vmeSlotId: %d (0x%02X)\n", __FUNCTION__, vmeSlotId, vmeSlotId);
#endif

	*((UINT16*)ptr) = vmeSlotId;

	return E__OK;
}

/*****************************************************************************
 * brdGetSerialNumber: Provide the board serial number for the USM 
 *
 * RETURNS: success
 */
 
UINT32 brdGetSerialNumber( void * ptr )
{
	UINT8 *	macAddr;
	char *	str;
	char	mac[12];

	// use the last five bytes of the MAC address for the serial number

	macAddr = lwIPAppsMacAddr();				// fetch MAC address
	str = ((char *)ptr);
	sprintf (mac,"%02x%02x%02x%02x%02x",macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
	strcpy(str,mac);
#ifdef DEBUG
	sysDebugPrintf("%s: macAddr: %02X:%02X:%02X:%02X:%02X:%02X\n", 
						__FUNCTION__,
						macAddr[0], macAddr[1], macAddr[2],
						macAddr[3], macAddr[4], macAddr[5]);
#endif

	return E__OK;
}

/*****************************************************************************
 * brdGetVmeDeviceImages: Provide the VME Outbound image buffer addresses 
 *
 * RETURNS: success
 */
 
UINT32 brdGetVmeDeviceImages( void* ptr )
{
	*((VME_DRV_INFO**)ptr) = &vmeDrvInfo;
 
	return E__OK;
}

/*****************************************************************************
 * brdGetVmeBridgelessReg: Provide the VME Bridgeless status register address
 *
 * RETURNS: success
 */
 
UINT32 brdGetVmeBridgelessReg( void* ptr )
{
	// return the address of the VME Bridgeless status register
	*((UINT16*)ptr) = STATUS_REG_4;
 
	return E__OK;
}


UINT32 brdStoreConfigChanges (void* ptr)
{
	UINT8 i;
	UINT32  bIoWindow;
	PCI_PFA	 pfa;

	//Storing initial generic LPC IO decode ranges to be restored later
	pfa = PCI_MAKE_PFA (0, 31, 0);
	pciStoredConfig.bBus = 0;
	pciStoredConfig.bDevice = 31;
	pciStoredConfig.bFunc = 0;
	for (i = 0; i < 4; ++i)
	{
		pciStoredConfig.pciRegData[i].regOffset = 0x84 + (i * 4);
		bIoWindow = PCI_READ_DWORD(pfa, 0x84 + (i * 4));
		pciStoredConfig.pciRegData[i].dwValue = bIoWindow;
	}
	pciStoredConfig.nNumRegs=4;
	return E__OK;

} /* brdStoreConfigChanges () */

UINT32 brdRestoreConfigChanges (void* ptr)
{
	UINT8 i;
	PCI_PFA	 pfa;

	if (pciStoredConfig.nNumRegs >= 8)
	{
		//Something freaky. might have been called without storing anything before
		return E__OK;
	}
	//Restoring generic LPC IO decode ranges back to initial values
	pfa = PCI_MAKE_PFA (pciStoredConfig.bBus, pciStoredConfig.bDevice, pciStoredConfig.bFunc);
	for (i = 0; i < pciStoredConfig.nNumRegs; ++i)
	{
		PCI_WRITE_DWORD(pfa, 
			pciStoredConfig.pciRegData[i].regOffset,
			pciStoredConfig.pciRegData[i].dwValue);

	}
	pciStoredConfig.nNumRegs = 0;//Resetting to be on safer side

	return E__OK;

} /* brdRestoreConfigChanges () */

UINT32 brdEnableWdLpcIoWindow(void* ptr)
{
	UINT8	i;
	UINT32  bIoWindow;
	PCI_PFA	 pfa;
	
	pfa = PCI_MAKE_PFA (0, 31, 0);
	for (i = 0; i < 4; ++i)
	{
		bIoWindow = PCI_READ_DWORD(pfa, 0x84 + (i * 4));
		if ((bIoWindow & 0xfff) == 0xC01)	//601 For BIOS change
		{
			//Already Io Window is enabled, nothing to be done
			return E__OK;
		}
	}
	
	PCI_WRITE_DWORD(pfa, 0x88, 0x00FC0C01);	//601 For BIOS change
	return E__OK;

} /* brdEnableWdLpcIoWindow() */

