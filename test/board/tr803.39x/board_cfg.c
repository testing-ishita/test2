
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/tr803.39x/board_cfg.c,v 1.1 2015-01-29 11:07:37 mgostling Exp $
 *
 * $Log: board_cfg.c,v $
 * Revision 1.1  2015-01-29 11:07:37  mgostling
 * Import files into new source repository.
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
#include <stdio.h>
#include <string.h>

#include <private/port_io.h>
#include <private/sys_delay.h>
#include <private/debug.h>

#include <bit/bit.h>
#include <bit/console.h>
#include <bit/delay.h>
#include <bit/io.h>
#include <bit/mem.h>
#include <bit/msr.h>
#include <bit/board_service.h>

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

//#define DEBUG
/* typedefs */

/* constants */

const UINT8			bDebugPort = COM2;	/* COM2 is debug console */

const SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO        */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-       */
	{ 0,	"",		0,		0 }
};

/* locals */

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

#warning "*****  FORCING COM2 instead of COM1: restore COM1 when debugging complete   *****"
//	*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;
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
			if( (bGetTestHandler() == TH__STH) && (bPeri == 0x04))
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

	umswitch = ((UINT8)dIoReadReg(0x210, REG_8));

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

	reg = ((UINT8)dIoReadReg(0x212, REG_8));
    if (reg & 0x20)
		*((UINT8*)ptr) =1;
	else
		*((UINT8*)ptr) =0;

	return E__OK;
}

UINT32 brdClearIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8)dIoReadReg(0x212, REG_8));

	reg = reg & ~(0x20);

	vIoWriteReg (0x212, REG_8, reg);
	
	return E__OK;
}

UINT32 brdGetVpxBrInstance (void *ptr)
{
	((VPX_INSTANCE_PARAMS*)ptr)->bInstance[0] = 2;
	((VPX_INSTANCE_PARAMS*)ptr)->bInstance[1] = 6;
	return (E__OK);
}



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
	if ((bGetTestHandler() == TH__LBIT) ||
		(bGetTestHandler() == TH__PBIT) ||
		(bGetTestHandler() == TH__STH ))
	{
		if((((SkipEthIf_PARAMS*)ptr)->dDidVid   == DIDVID_82579) &&
		   (((SkipEthIf_PARAMS*)ptr)->bInstance == 1))
			return E__OK;
		else
			return 0x01;
	}
	else
		return E__FAIL;

} /* brdSelectEthInterface () */


/*****************************************************************************
 * brdConfigureVPXNTPorts: configures NT ports for VPX Backplane communication
 * RETURNS: success or error code
 * */
UINT32 brdConfigureVPXNTPorts(void *ptr)
{

#ifdef DEBUG
	UINT8  buffer[64];
#endif

	UINT8	bSysCon;

	//UINT32 dBarAddress;

	PTR48  tPtr;
	UINT32 mHandle;
	UINT32 pointer;
	UINT32 memRegAddr;
	PCI_PFA pfa;
	UINT8	bTemp;
	UINT32  DIDVID;

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



	return E__OK;

} /* brdConfigureVxsNTPorts () */


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
