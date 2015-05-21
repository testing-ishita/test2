
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/board_cfg.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: board_cfg.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
 * Import files into new source repository.
 *
 * Revision 1.6  2013/02/04 11:04:26  madhukn
 * CUTE 1.03
 *
 * Revision 1.5  2012/10/10 06:47:23  madhukn
 * Changes for VP91x PBIT V1.01-01
 *
 * Revision 1.4  2012/08/09 08:46:00  madhukn
 * *** empty log message ***
 *
 * Revision 1.3  2012/08/07 15:16:22  madhukn
 * *** empty log message ***
 *
 * Revision 1.2  2012/08/07 03:22:09  madhukn
 * *** empty log message ***
 *
 * Revision 1.1  2012/06/22 11:41:59  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
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

#include "cctboard.h"

/* defines */

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


#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

//#define DEBUG
/* typedefs */

/* constants */

const UINT8			bDebugPort = COM1;	/* COM1 is debug console */

const SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO     */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-    */
	{ 3,	"Ch 2", 0x03E8, 11 },	/* COM3 Port C     -"-    */
	{ 0,	"",		0,		0 }
};

typedef struct{
	UINT16 regOffset;
	UINT32 dwValue;
} PCI_REG_DATA;

typedef struct {
	UINT8 bBus,bDevice,bFunc;
	UINT8 nNumRegs;
	PCI_REG_DATA pciRegData[8];
} PCI_REG_CONFIG;


/* locals */

PCI_REG_CONFIG pciStoredConfig;

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
//	UINT8	bConsPort;

	/* Serial console can be COM1 or CPM2 or COM3 */
/*#ifdef CUTE
	*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;
#else *///for PBIT
	//bConsPort = ((sysInPort8(0x225))>>5)&0x3;

	//if (bConsPort == 1)
		*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;
	//else if(bConsPort == 2)
		//*((UINT16*)ptr) = asSccParams[COM2].wBaseAddr;
	//else if(bConsPort == 3)
		//*((UINT16*)ptr) = asSccParams[COM3].wBaseAddr;
	//else{//default: COM1
		//*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;
	//}
//#endif
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

	bTemp = sysInPort8 (0x312);
	bTemp = bTemp & ~(1<<6);

	while (1)
	{
		sysOutPort8 (0x312, bTemp | (1<<6));		/* 300mS on */
		sysDelayMilliseconds (300);

		sysOutPort8 (0x312, bTemp);				/* 200mS off */
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

	umswitch = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));

	if ((umswitch & USER_JUMPER) == 0)
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
 * brdGetFlashDriveParams: report parameters for the Flash Drive module
 *
 * RETURNS: base address of port
 */

UINT32 brdGetFlashDriveParams (void *ptr)
{
	((FLASH_DRV_PARAMS*)ptr)->bInstance = 2;
	((FLASH_DRV_PARAMS*)ptr)->bport     = 1;

	return E__OK;

} /* brdGetFlashDriveParams () */






/*****************************************************************************
 * brdSelectEthInterface: selects front or rear Ethernet interface
 *
 * RETURNS: success or error code
 */
#define VID_INTEL		0x8086
#define DID_82579		0x1502
#define DIDVID_82579    ((DID_82579 << 16) | VID_INTEL)

UINT32 brdSkipEthInterface (void *ptr)
{
	if( (bGetTestHandler() == TH__LBIT) ||
		(bGetTestHandler() == TH__PBIT) ||
		(bGetTestHandler() == TH__STH ) )
	{		
		if((((SkipEthIf_PARAMS*)ptr)->dDidVid   == DIDVID_82579) &&
		   (((SkipEthIf_PARAMS*)ptr)->bInstance == 1) )
			return E__OK;
		else
			return 0x01;
	}
	else
		return E__FAIL;

} /* brdSelectEthInterface () */
 


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
	UINT32 msrHigh=0,msrLow=0,timeout;
	UINT8 maxRatio=0,minRatio=0,i=0;
	char achBuffer[80];

    /*Get the min & max ratio supported*/
    vReadMsr(0xce,&msrHigh,&msrLow);
	if(((msrHigh>>2)&3)>0)
	{
#ifdef DEBUG
		 sprintf ( achBuffer, "Configurable TDP-SKU\n");
		 vConsoleWrite (achBuffer);
#endif

		vReadMsr(0x648,&msrHigh,&msrLow);
		maxRatio = msrLow&0xff;
		vReadMsr(0xce,&msrHigh,&msrLow);
		minRatio = (msrHigh>>8) & 0xff;
	}
	else
	{
#ifdef DEBUG
		 sprintf ( achBuffer, "Nominal TDP-SKU\n");
		 vConsoleWrite (achBuffer);
#endif

		maxRatio = (msrLow>>8) & 0xff;
		minRatio = (msrHigh>>8) & 0xff;
	}
#ifdef DEBUG
	sprintf ( achBuffer, "minRatio=%d, maxRatio=%d\n",minRatio,maxRatio);
	vConsoleWrite (achBuffer);
#endif
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

UINT32 brdEnableWdLpcIoWindow(void* ptr)
{
	UINT8	i;
	UINT32  bIoWindow;
	PCI_PFA	 pfa;
	pfa = PCI_MAKE_PFA (0, 31, 0);
	for (i = 0; i < 4; ++i)
	{
		bIoWindow = PCI_READ_DWORD(pfa, 0x84+i*4);
		if((bIoWindow & 0xfff) == 0xC01)//601 For BIOS change
		{
			//Already Io Window is enabled, nothing to be done
			return E__OK;
		}
	}
	PCI_WRITE_DWORD(pfa, 0x90,0x00FC0601);
	return E__OK;

} /* brdEnableWdLpcIoWindow() */

UINT32 brdIsTPMFitted(void *ptr)
{	
	UINT8 reg;
	reg = ((UINT8)dIoReadReg(0x230, REG_8));
	if((reg & 0x02) == 0x02)
	{
		return E__OK;
	}
	else{
		return (E__FAIL);
	}
    return (E__OK);
}

