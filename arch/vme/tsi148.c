
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

/*
 * tsi148.c
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/arch/vme/tsi148.c,v 1.15 2015-03-17 10:39:19 hchalla Exp $
 * $Log: tsi148.c,v $
 * Revision 1.15  2015-03-17 10:39:19  hchalla
 * Fixed compiler warnings and initialized variables.
 *
 * Revision 1.14  2015-03-13 10:35:51  mgostling
 * Fixed compiler warning
 *
 * Revision 1.13  2015-03-12 15:22:44  hchalla
 * Increased Universe DMA Buffer size by half MB.
 *
 * Revision 1.12  2015-03-10 16:19:28  mgostling
 * Added CVS headers and disabled debug flags
 *
 * Created on: 19 Apr 2010
 * Author: Haroon
 * TSI148 initialisation, PCI and VME memory windows setup, and geographic
 * address based slot addressing are working now [22-04-2010]
 *
 */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
 
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>

#include <bit/board_service.h>

#include <devices/tsi148.h>
#include <devices/universe.h>
#include <private/debug.h>
#include <private/port_io.h>
#include <private/cpu.h>

#include <cute/arch.h>
#include <cute/packet.h>

#include "config.h"

//#define DEBUG_SOAK
//#define DEBUG

#define INT_PIN_NUM			16

static volatile UINT32	mapRegSpace;		// local PCI registers
static volatile UINT32	interconnectMem;	// PCI->VME image (remote slots)
static volatile UINT32	crCsrMem;			// remote PCI registers

static UINT32			dNextCrCsrBase;		//CR/CSR base allocation counter
static UINT8			bSlotNumber;		//the logical slot number from Auto-ID/geographic address

UINT32 globalVMEID = 0;

/*****************************************************************************
 * GetVmeBars: Identify board as Monarch or slave
 *
 * RETURNS: board type
 */
void GetVmeBars (VMEBARS *vme_Bars)
{
	vme_Bars->LocalDev = mapRegSpace;
	vme_Bars->Intercon = interconnectMem;
	vme_Bars->CrCsr    = crCsrMem;
}

/*****************************************************************************
 * IdentifyAgent: Identify board as Monarch or slave
 *
 * RETURNS: board type
 */
UINT8 IdentifyAgent (void)
{
	UINT8	bAgent;

	/* Auto-SYSCON has a bug in the silicon - so is not supported yet. The SYSCON
	 * link is used instead and the SYSCON function forced by this */

#ifdef INCLUDE_LWIP_SMC
	bAgent = ID__MONARCH;			// always a MONARCH when using the network
#else
	UINT32	dMiscCtl;

	if (globalVMEID == TSI148_DEVID_VENID)
	{
		dMiscCtl = vReadDWord( (mapRegSpace+ VSTAT) );

		if ((dMiscCtl & SCONS) != 0)
		{
			bAgent = ID__MONARCH;
		}
		else
		{
			bAgent = ID__SLAVE;
		}
	}
	else if (globalVMEID == UNIV_DEVID_VENID)
	{
		dMiscCtl = vReadDWord( (mapRegSpace+ MISC_CTL) );

		if ((dMiscCtl & UNIV_SCONS) != 0)
		{
			bAgent = ID__MONARCH;
		}
		else
		{
			bAgent = ID__SLAVE;
		}
	}
#endif

	return (bAgent);

} /* IdentifyAgent () */


/*****************************************************************************
 * bReadRtcSeconds: read current rtc time in seconds
 *
 * RETURNS: rtc seconds reading
 */
UINT8 bReadRtcSeconds (void)
{
	UINT8	bSeconds;
	UINT8	bTemp;

	do
	{
		sysOutPort8 (0x70, 0x8A);			/* check for update */
		bTemp = 0x80 & sysInPort8 (0x71);

	} while (bTemp != 0);					/* wait for update to complete */

	do
	{
		sysOutPort8 (0x70, 0x80);			/* get current time */
		bSeconds = sysInPort8 (0x71);

		sysOutPort8 (0x70, 0x8A);			/* check for update */
		bTemp = 0x80 & sysInPort8 (0x71);

	} while (bTemp != 0);					/* repeat if update occurred */

	return (bSeconds);

} /* bReadRtcSeconds () */



#ifdef INCLUDE_LWIP_SMC
/******************************************************************************
 *
 * 						BACKPLANE/PCI MEMORY ORGANIZATION
 * 						---------------------------------
 *
 * 1 - Each VME device allocates 80MB of PCI space
 *
 * 2 - BAR0 of each TSI148 is manipulated in a such a way that
 * 	   - BAR0 points to a 15 MB region
 * 	   - At offset 0 is the local TSI148 CR/CSR occupying 512KB
 *     - Each CR/CSR is only 4KB but 512KB is allocated by each device
 *     - Each successive 512KB block can be used to map peripheral
 *       TSI148s or UNIVERSE CR/CSR
 *
 * 3 - BAR0 - 1 MB , points to a 1MB region which is organised as
 *     follows
 *     - The first 64KB block is divided in 16 4KB blocks, which
 *     	 are used to map UNIVERSE Interconnect templates, the
 *     	 UNIVERSE chips can have a minimum window of 4KB hence the
 *       4KB windows in the first 64KB block
 *     - The remaining 960KB is divided into 15 64KB blocks, which
 *     	 are used to map TSI148 Interconnect templates, the
 *     	 TSI148 chips can have a minimum window of 64KB hence the
 *       64KB windows in the remaining 960KB
 *
 * 3 - BAR0 - 65 MB , points to a 64MB region which is used as a
 *     buffer for running cooperating memory tests
 *
 *						 ---------------------------------------
 * 						|										|
 * 						|										|
 * 						|		  15 MB for CR/CSR blocks 		|
 * 						|			  for all TSI148			|
 * 						|										|
 * TSI148 BAR0 ------>	|---------------------------------------|
 * 						|										|
 * 						|		  1 MB for Interconnects		|
 * 						|---------------------------------------|
 * 						|										| VME 0xAAFFFFFF
 * 						|										|
 * 						|				16 MB Block				|
 * 						|			  For Cooperating			|
 * 						|					Tests				|
 * 						|										|
 * 						|										|
 * 						|										| VME 0xAA000000
 * 						|---------------------------------------|
 * 						|										| VME 0x55FFFFFF
 * 						|										|
 * 						|				16 MB Block				|
 * 						|			  For Cooperating			|
 * 						|					Tests				|
 * 						|										|
 * 						|										|
 * 						|										| VME 0x55000000
 * 						|---------------------------------------|
 * 						|										| VME 0x99FFFFFF
 * 						|										|
 * 						|				16 MB Block				|
 * 						|			  For Cooperating			|
 * 						|				DMA Tests				|
 * 						|										|
 * 						|										|
 * 						|										| VME 0x99000000
 * 						|---------------------------------------|
 * 						|										| VME 0x66FFFFFF
 * 						|										|
 * 						|				16 MB Block				|
 * 						|			  For Cooperating			|
 * 						|				DMA Tests				|
 * 						|										|
 * 						|										|
 * 						|										| VME 0x66000000
 *  					 --------------------------------------- 
 *
 *****************************************************************************/

UINT32 InitialiseTsi148VMEDevice(UINT8 bInitMode, PCI_PFA pfa)
{
	UINT32	dTsi148Addr;
	UINT32	dTemp;
	UINT32	dTemp1;
	UINT32	dSysfail;		/* sysfail still asserted */
	UINT8	bSeconds;		/* current seconds count  */
	UINT8	bEndSeconds;	/* ending seconds count   */
	UINT32  regVal;
	UINT64	l64Temp;
	VME_DRV_INFO *vmeDrvInfo;

#ifdef DEBUG_SOAK
	char	achBuffer[80];
	sysDebugWriteString ("\nInitialiseTsi148VMEDevice()\n");
#endif

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	vmeDrvInfo->deviceId = TSI148_DEVID_VENID;

	// Enable the bus mastering and Memory/IO space access
	regVal=PCI_READ_WORD(pfa,PCI_COMMAND);
	regVal |= 0x06;
	PCI_WRITE_WORD(pfa,PCI_COMMAND,regVal);

	//Get the memory mapped CRG for the local TSI148 512KB
	regVal = PCI_READ_DWORD(pfa,PCI_BAR0);
	regVal &= ~0xf;
	dTsi148Addr = regVal;

	//CRG is only 4KB so allocating 4KB
	l64Temp = regVal;
	mapRegSpace = sysMemGetPhysPtrLocked(l64Temp, 0x1000);
	if(mapRegSpace == NULL)
	{
		return E__TEST_NO_MEM;
	}

	vmeDrvInfo->mapRegSpace = mapRegSpace;

	//verify we can access our own TSI148
	if ((vReadDWord(mapRegSpace)) != TSI148_DEVID_VENID)
	{
#ifdef DEBUG_SOAK
		sprintf(achBuffer,"Unable to read from mapped CRG(%x):%x\n",
				mapRegSpace, vReadDWord(mapRegSpace));
		sysDebugWriteString(achBuffer);
#endif

		//vFreePtr( mRegSpaceHandle ); <---------------------------------------------
		return E__TSI148_ACCESS;
	}
	
#ifdef DEBUG_SOAK
	sprintf(achBuffer,"TSI148 register mapped address: %08X\n", mapRegSpace);
	sysDebugWriteString(achBuffer);
#endif

	/*Sanity initialisations*/
	vWriteDWord((mapRegSpace+CSRBCR), 0x40000000);		// clear System Fail Enable
	vWriteDWord((mapRegSpace+INTEN) , 0x00000000);		// disable all interrupts
	vWriteDWord((mapRegSpace+INTC)  , 0xFFFFFFFF);		// clear all interrupts 
	vWriteDWord((mapRegSpace+INTEO) , 0x00000000);		// disable all interrupt out pins
	vWriteDWord((mapRegSpace+VICR)  , 0x00000000);

	/* Clear the VMEBus exception Status bit */
	regVal = TSI148_VEAT_VESCL;
	regVal = BYTE_SWAP(regVal);
	vWriteDWord((mapRegSpace+VEAT),regVal);

	/* Configure device registers
	 * Set VME Bus timeout to 512uS, Set Round robin mode
	 * rest of the things remain same
	 */
	dTemp  = vReadDWord((mapRegSpace + VCTRL));
	dTemp &= BYTE_SWAP((0xFFFFFF00));
	dTemp |= BYTE_SWAP((0x000000C6));
	vWriteDWord((mapRegSpace+VCTRL), dTemp);

#ifdef DEBUG_SOAK
	sysDebugWriteString ("\nWaiting so all boards are ready \n");
#endif

	vDelay(2000);

	/* PCI->VME outbound 0x55000000
	 * - map 16MB from PCI to VME at 0x55000000
	 */
	dTemp = BYTE_SWAP( (dTsi148Addr - 0x02100000) );
	vWriteDWord( (mapRegSpace+OTSAL0), dTemp);
	dTemp = BYTE_SWAP( ((dTsi148Addr - 0x02100000) + 0x00FFFFFF) );
	vWriteDWord((mapRegSpace+OTEAL0), dTemp );
	dTemp = BYTE_SWAP( (0x55000000 - (dTsi148Addr - 0x02100000)) );
	vWriteDWord((mapRegSpace+OTOFL0), dTemp);
	dTemp = BYTE_SWAP((0x80000042));
	vWriteDWord((mapRegSpace+OTAT0), dTemp);

	/* PCI->VME outbound 0xAA000000
	 * - map 16MB from PCI to VME at 0xAA000000
	 */
	dTemp = BYTE_SWAP( (dTsi148Addr - 0x01100000) );
	vWriteDWord( (mapRegSpace+OTSAL1), dTemp);
	dTemp = BYTE_SWAP( ((dTsi148Addr - 0x01100000) + 0x00FFFFFF) );
	vWriteDWord((mapRegSpace+OTEAL1), dTemp );
	dTemp = BYTE_SWAP( (0xAA000000 - (dTsi148Addr - 0x01100000)) );
	vWriteDWord((mapRegSpace+OTOFL1), dTemp);
	dTemp = BYTE_SWAP((0x80000042));
	vWriteDWord((mapRegSpace+OTAT1), dTemp);

	/* PCI->VME outbound 0x66000000
	 * - map 16MB from PCI to VME at 0x66000000 for DMA
	 */
	dTemp = BYTE_SWAP( (dTsi148Addr - 0x04100000) );
	vWriteDWord( (mapRegSpace+OTSAL2), dTemp);
	dTemp = BYTE_SWAP( ((dTsi148Addr - 0x04100000) + 0x00FFFFFF) );
	vWriteDWord((mapRegSpace+OTEAL2), dTemp );
	dTemp = BYTE_SWAP( (0x66000000 - (dTsi148Addr - 0x04100000)) );
	vWriteDWord((mapRegSpace+OTOFL2), dTemp);
	dTemp = BYTE_SWAP((0x80000144));
	vWriteDWord((mapRegSpace+OTAT2), dTemp);

	/* PCI->VME outbound 0x99000000
	 * - map 16MB from PCI to VME at 0x99000000 for DMA
	 */
	dTemp = BYTE_SWAP( (dTsi148Addr - 0x03100000) );
	vWriteDWord( (mapRegSpace+OTSAL3), dTemp);
	dTemp = BYTE_SWAP( ((dTsi148Addr - 0x03100000) + 0x00FFFFFF) );
	vWriteDWord((mapRegSpace+OTEAL3), dTemp );
	dTemp = BYTE_SWAP( (0x99000000 - (dTsi148Addr - 0x03100000)) );
	vWriteDWord((mapRegSpace+OTOFL3), dTemp);
	dTemp = BYTE_SWAP((0x80000144));
	vWriteDWord((mapRegSpace+OTAT3), dTemp);

//-----------------------------------------------------------------------------

	l64Temp = (dTsi148Addr - 0x02100000);
	vmeDrvInfo->outboundAddr1 = (UINT32 *)sysMemGetPhysPtrLocked( l64Temp, 0x01000000);
	if (vmeDrvInfo->outboundAddr1 == NULL)
	{
		sysDebugWriteString("No handle to read mapped window!\n");
		//vFreePtr(mCrCsrHandle); <---------------------------------------------
		return E__FAIL;
	}

	l64Temp = (dTsi148Addr - 0x01100000);
	vmeDrvInfo->outboundAddr2 = (UINT32 *)sysMemGetPhysPtrLocked( l64Temp, 0x01000000);
	if (vmeDrvInfo->outboundAddr2 == NULL)
	{
		sysDebugWriteString("No handle to read mapped window!\n");
		//vFreePtr(mCrCsrHandle); <---------------------------------------------
		return E__FAIL;
	}

	l64Temp = (dTsi148Addr - 0x04100000);
	vmeDrvInfo->outboundAddr3 = (UINT32 *)sysMemGetPhysPtrLocked( l64Temp, 0x01000000);
	if (vmeDrvInfo->outboundAddr3 == NULL)
	{
		sysDebugWriteString("No handle to read mapped window!\n");
		//vFreePtr(mCrCsrHandle); <---------------------------------------------
		return E__FAIL;
	}

	l64Temp = (dTsi148Addr - 0x03100000);
	vmeDrvInfo->outboundAddr4 = (UINT32 *)sysMemGetPhysPtrLocked( l64Temp, 0x01000000);
	if (vmeDrvInfo->outboundAddr4 == NULL)
	{
		sysDebugWriteString("No handle to read mapped window!\n");
		//vFreePtr(mCrCsrHandle); <---------------------------------------------
		return E__FAIL;
	}

//-----------------------------------------------------------------------------

	/* get slot number from Geographic address */
	dTemp = BYTE_SWAP((vReadDWord((mapRegSpace + VSTAT)) & 0x0F000000));

	/* make slot number 0 based */
	bSlotNumber = (UINT8) --dTemp;
	vmeDrvInfo->slot = bSlotNumber;

#ifdef DEBUG_SOAK
	sprintf(achBuffer, "Slot %x\n", bSlotNumber);
	sysDebugWriteString (achBuffer) ;
#endif

	/* Enable inbound image1 */
	dTemp  = 0x55000000 + (bSlotNumber * 0x00100000);
	dTemp1 = BYTE_SWAP((dTemp));
	vWriteDWord((mapRegSpace+ITSAL0), dTemp1);
	dTemp1 = BYTE_SWAP((dTemp + 0x000FFFFF));
	vWriteDWord((mapRegSpace+ITEAL0), dTemp1);

	l64Temp = 0x10000000;
	l64Temp = l64Temp -((UINT64)dTemp & 0x00000000FFF00000);
	dTemp1  = l64Temp;
	dTemp1  = BYTE_SWAP(dTemp1);
	vWriteDWord((mapRegSpace+ITOFL0), dTemp1);
	dTemp1  = (l64Temp >> 32);
	dTemp1  = BYTE_SWAP((dTemp1));
	vWriteDWord((mapRegSpace+ITOFU0), dTemp1);

	dTemp1  = BYTE_SWAP(0x80000025);
	vWriteDWord((mapRegSpace+ITAT0),  dTemp1);

	/* Enable inbound image2 */
	dTemp  = 0xAA000000 + (bSlotNumber * 0x00100000);
	dTemp1 = BYTE_SWAP((dTemp));
	vWriteDWord((mapRegSpace+ITSAL1), dTemp1);
	dTemp1 = BYTE_SWAP((dTemp + 0x000FFFFF));
	vWriteDWord((mapRegSpace+ITEAL1), dTemp1);

	l64Temp = 0x10100000;
	l64Temp = l64Temp -((UINT64)dTemp & 0x00000000FFF00000);
	dTemp1  = l64Temp;
	dTemp1  = BYTE_SWAP((dTemp1));
	vWriteDWord((mapRegSpace+ITOFL1), dTemp1);
	dTemp1  = (l64Temp >> 32);
	dTemp1  = BYTE_SWAP((dTemp1));
	vWriteDWord((mapRegSpace+ITOFU1), dTemp1);

	dTemp1  = BYTE_SWAP((0x80000025));
	vWriteDWord((mapRegSpace+ITAT1),  dTemp1);

	/* Enable inbound image3 */
	dTemp  = 0x66000000 + (bSlotNumber * 0x00100000);
	dTemp1 = BYTE_SWAP((dTemp));
	vWriteDWord((mapRegSpace+ITSAL2), dTemp1);
	dTemp1 = BYTE_SWAP((dTemp + 0x000FFFFF));
	vWriteDWord((mapRegSpace+ITEAL2), dTemp1);

	l64Temp = 0x10200000;
	l64Temp = l64Temp -((UINT64)dTemp & 0x00000000FFF00000);
	dTemp1  = l64Temp;
	dTemp1  = BYTE_SWAP(dTemp1);
	vWriteDWord((mapRegSpace+ITOFL2), dTemp1);
	dTemp1  = (l64Temp >> 32);
	dTemp1  = BYTE_SWAP((dTemp1));
	vWriteDWord((mapRegSpace+ITOFU2), dTemp1);

	dTemp1  = BYTE_SWAP(0x800000C5);
	vWriteDWord((mapRegSpace+ITAT2),  dTemp1);

	/* Enable inbound image4 */
	dTemp  = 0x99000000 + (bSlotNumber * 0x00100000);
	dTemp1 = BYTE_SWAP((dTemp));
	vWriteDWord((mapRegSpace+ITSAL3), dTemp1);
	dTemp1 = BYTE_SWAP((dTemp + 0x000FFFFF));
	vWriteDWord((mapRegSpace+ITEAL3), dTemp1);

	l64Temp = 0x10300000;
	l64Temp = l64Temp -((UINT64)dTemp & 0x00000000FFF00000);
	dTemp1  = l64Temp;
	dTemp1  = BYTE_SWAP((dTemp1));
	vWriteDWord((mapRegSpace+ITOFL3), dTemp1);
	dTemp1  = (l64Temp >> 32);
	dTemp1  = BYTE_SWAP((dTemp1));
	vWriteDWord((mapRegSpace+ITOFU3), dTemp1);

	dTemp1  = BYTE_SWAP(0x800000C5);
	vWriteDWord((mapRegSpace+ITAT3),  dTemp1);

	/* Enable the VCSR image */
	dTemp = vReadDWord((mapRegSpace + CRAT));
	dTemp = (dTemp | 0x80000000);
	vWriteDWord((mapRegSpace+CRAT),   dTemp);

	/* Release LOCAL sysfail */
	dTemp = vReadDWord((mapRegSpace + VCTRL));
	dTemp = dTemp & (~SFAILAI);
	vWriteDWord( (mapRegSpace+VCTRL), dTemp);

	vWriteDWord( (mapRegSpace+INTM2), 0x00000000);

	/* Wait for all boards to release SYSFAIL, or timeout (9 sec) */
	bEndSeconds = 0x0F & bReadRtcSeconds ();	/* get current RTC time */
	bEndSeconds = (bEndSeconds + 9) % 10;		/* compute 9S timeout */

	do
	{
		bSeconds = 0x0F & bReadRtcSeconds ();	/* get current time */
		dSysfail = vReadDWord((mapRegSpace + VSTAT));

	} while ((bSeconds != bEndSeconds) && ((dSysfail & SYSFLS) != 0));

	return E__OK;
}

#else

/******************************************************************************
 *
 * 						BACKPLANE/PCI MEMORY ORGANIZATION
 * 						---------------------------------
 *
 * 1 - Each VME device allocates 80MB of PCI space
 *
 * 2 - BAR0 of each TSI148 is manipulated in a such a way that
 * 	   - BAR0 points to a 15 MB region
 * 	   - At offset 0 is the local TSI148 CR/CSR occupying 512KB
 *     - Each CR/CSR is only 4KB but 512KB is allocated by each device
 *     - Each successive 512KB block can be used to map peripheral
 *       TSI148s or UNIVERSE CR/CSR
 *
 * 3 - BAR0 - 1 MB , points to a 1MB region which is organised as
 *     follows
 *     - The first 64KB block is divided in 16 4KB blocks, which
 *     	 are used to map UNIVERSE Interconnect templates, the
 *     	 UNIVERSE chips can have a minimum window of 4KB hence the
 *       4KB windows in the first 64KB block
 *     - The remaining 960KB is divided into 15 64KB blocks, which
 *     	 are used to map TSI148 Interconnect templates, the
 *     	 TSI148 chips can have a minimum window of 64KB hence the
 *       64KB windows in the remaining 960KB
 *
 * 3 - BAR0 - 65 MB , points to a 64MB region which is used as a
 *     buffer for running cooperating memory tests
 *
 *						 ---------------------------------------
 * 						|										|
 * 						|										|
 * 						|		  15 MB for CR/CSR blocks 		|
 * 						|			  for all TSI148			|
 * 						|										|
 * TSI148 BAR0 ------>	|---------------------------------------|
 * 						|										|
 * 						|		  1 MB for Interconnects		|
 * 						|---------------------------------------|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|				64 MB Block				|
 * 						|			  For Cooperating			|
 * 						|					Tests				|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 * 						|										|
 *  					 ---------------------------------------
 *
 *****************************************************************************/

/*****************************************************************************
 * InitialiseTsi148VMEDevice: Initialise TSI148
 *
 * RETURNS: success or error code
 */
UINT32 InitialiseTsi148VMEDevice( UINT8 bInitMode, PCI_PFA pfa)
{
	UINT32	dTsi148Addr;
	UINT32	dTemp;
	UINT32  regVal;
	UINT64	l64Temp;
#ifdef DEBUG_SOAK
	char	achBuffer[80];
	sysDebugWriteString ("\nInitialiseTsi148VMEDevice() \n");
#endif

	// Enable the bus mastering and Memory/IO space access
	regVal=PCI_READ_WORD(pfa,PCI_COMMAND);
	regVal |= 0x06;
	PCI_WRITE_WORD(pfa,PCI_COMMAND,regVal);

	//Get the memory mapped CRG for the local TSI148 512KB
	regVal = PCI_READ_DWORD(pfa,PCI_BAR0);
	regVal &= ~0xf;
	dTsi148Addr = regVal;

	//CRG is only 4KB so allocating 4KB
	l64Temp = regVal;
	mapRegSpace = sysMemGetPhysPtrLocked(l64Temp, 0x1000);
	if(mapRegSpace == NULL)
	{
		return E__TEST_NO_MEM;
	}

	//verify we can access our own TSI148
	if( (vReadDWord(mapRegSpace)) != TSI148_DEVID_VENID )
	{
#ifdef DEBUG_SOAK
		sprintf(achBuffer,"Unable to read from mapped CRG(%x):%x\n",
				mapRegSpace, vReadDWord(mapRegSpace));
		sysDebugWriteString(achBuffer);
#endif

		//vFreePtr( mRegSpaceHandle ); <---------------------------------------------
		return E__TSI148_ACCESS;
	}
	else
	{
		/*Sanity initialisations*/
		vWriteDWord((mapRegSpace+CSRBCR), 0x40000000);
		vWriteDWord((mapRegSpace+INTEN) , 0x00000000);
		vWriteDWord((mapRegSpace+INTC)  , 0xFFFFFFFF);
		vWriteDWord((mapRegSpace+INTEO) , 0x00000000);
		vWriteDWord((mapRegSpace+VICR)  , 0x00000000);

		/* Clear the VMEBus exception Status bit */
		regVal = TSI148_VEAT_VESCL;
		regVal = BYTE_SWAP(regVal);
		vWriteDWord((mapRegSpace+VEAT),regVal);
	}

	if (bInitMode == INIT_BASIC)
			return E__OK;

	/* Configure device registers
	 * Set VME Bus timeout to 512uS, Set Round robin mode
	 * rest of the things remain same
	 */
	dTemp  = vReadDWord((mapRegSpace + VCTRL));
	dTemp &= BYTE_SWAP(0xFFFFFF00);
	dTemp |= BYTE_SWAP(0x000000C6);
	vWriteDWord((mapRegSpace+VCTRL), dTemp);

   if( IdentifyAgent() == ID__MONARCH )
   {
#ifdef DEBUG_SOAK
		sysDebugWriteString ("\nID__MONARCH \n");
		sysDebugWriteString ("\nWaiting so all boards are ready \n");
#endif

		vDelay(2000);

		/* PCI Slave-0  PCI->VME
		 * - maps 1024KB from PCI to VME at 0xD0000000
		 * - maps to 16*4KB + 15*64KB byte slots for Interconnects
		 * - Peripheral boards are allocated a slot during auto ID
		 */
		dTemp = BYTE_SWAP( (dTsi148Addr - 0x100000) );
		vWriteDWord( (mapRegSpace+OTSAL0), dTemp);
		dTemp = BYTE_SWAP( (dTsi148Addr - 0x10000) );
		vWriteDWord((mapRegSpace+OTEAL0), dTemp );
		dTemp = BYTE_SWAP( (0xD0000000 - (dTsi148Addr - 0x100000)) );
		vWriteDWord((mapRegSpace+OTOFL0), dTemp );
		dTemp = BYTE_SWAP(0x80000042);
		vWriteDWord((mapRegSpace+OTAT0), dTemp);


		/* PCI Slave-1  PCI->VME
		 * - maps to the CR/CSR space on VME
		 * - maps the whole 15MB region containing 512KB slots
		 * - Peripheral boards are allocated a slot during auto ID
		 */
		dTemp   = BYTE_SWAP( (dTsi148Addr + 0x80000) );
		vWriteDWord( (mapRegSpace+OTSAL1), dTemp);
		dTemp   = BYTE_SWAP( (dTsi148Addr + 0xf00000) );
		vWriteDWord((mapRegSpace+OTEAL1), dTemp );
		l64Temp = (0 - (dTsi148Addr + 0x80000));
		dTemp   = (UINT32)l64Temp;
		dTemp   = BYTE_SWAP(dTemp);
		vWriteDWord((mapRegSpace+OTOFL1), dTemp );
		dTemp   = (UINT32)(l64Temp >> 32);
		dTemp   = BYTE_SWAP(dTemp);
		vWriteDWord((mapRegSpace+OTOFU1), dTemp );
		dTemp   = BYTE_SWAP(0x80000045);
		vWriteDWord((mapRegSpace+OTAT1), dTemp);


		// Initialise Auto ID counters
		dNextCrCsrBase = CSR_SIZE;// CR/CSR base allocation counter
		l64Temp = (dTsi148Addr + CSR_SIZE);
		crCsrMem = sysMemGetPhysPtrLocked( l64Temp, (0xf00000 - CSR_SIZE));
		if(crCsrMem == NULL)
		{
			sysDebugWriteString("No handle to read mapped CR/CSR \n");
			return E__FAIL;
		}

		l64Temp = (dTsi148Addr - 0x100000);
		interconnectMem = sysMemGetPhysPtrLocked( l64Temp, 0x100000);
		if(interconnectMem == NULL)
		{
			sysDebugWriteString("No handle to read mapped window!\n");
			//vFreePtr(mCrCsrHandle); <---------------------------------------------
			return E__FAIL;
		}

	}
	else	/*********************** PERIPHERAL BOARD ****************************/
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString ("\n PERIPHERAL \n");
#endif

		/* PCI Slave-0  PCI->VME
		 * - maps 64kb from PCI to VME at 0xD0000000 initially
		 * - maps to a 64k byte slot for Interconnect because thats the minimum
		 *   TSI148 supports
		 * - moved by MONARCH/Self during auto ID process
		 */
		dTemp = BYTE_SWAP( (dTsi148Addr - 0x100000) );
		vWriteDWord( (mapRegSpace+OTSAL0), dTemp);
		dTemp = BYTE_SWAP( (dTsi148Addr - 0x10000) );
		vWriteDWord((mapRegSpace+OTEAL0), dTemp );
		dTemp = BYTE_SWAP( (0xD0000000 - (dTsi148Addr - 0x100000)) );
		vWriteDWord((mapRegSpace+OTOFL0), dTemp );
		dTemp = BYTE_SWAP(0x80000042);
		vWriteDWord((mapRegSpace+OTAT0), dTemp);


		/* PCI Slave-1  PCI->VME
		 * - maps to first CR/CSR space slot on VME
		 * - maps the whole 15MB region because the MONARCH/Self will rearrange
		 * - slots during auto ID process
		 */
		dTemp = BYTE_SWAP( (dTsi148Addr + 0x80000) );
		vWriteDWord( (mapRegSpace+OTSAL1), dTemp);
		dTemp = BYTE_SWAP( (dTsi148Addr + 0xf00000) );
		vWriteDWord((mapRegSpace+OTEAL1), dTemp );
		l64Temp = (0 - (dTsi148Addr + 0x80000));
		dTemp = l64Temp;
		dTemp = BYTE_SWAP(dTemp );
		vWriteDWord((mapRegSpace+OTOFL1), dTemp );
		dTemp = l64Temp>>32;
		dTemp = BYTE_SWAP(dTemp );
		vWriteDWord((mapRegSpace+OTOFU1), dTemp );
		dTemp = BYTE_SWAP(0x80000045);
		vWriteDWord((mapRegSpace+OTAT1), dTemp);


		/* VME Slave-0  VME->PCI
		 * - At 0xD0000000 +(X*64kBytes) on VME
		 * - maps one 64kByte Slot to 0x20000 on PCI
		 * - moved by MONARCH/Self during auto ID process
		 */
		dTemp = BYTE_SWAP( 0xD0000000 );
		vWriteDWord( (mapRegSpace+ITSAL0), dTemp);
		dTemp = BYTE_SWAP( 0xD0000000 );
		vWriteDWord((mapRegSpace+ITEAL0), dTemp );
		dTemp = BYTE_SWAP( 0xffffffff);
		vWriteDWord((mapRegSpace+ITOFU0), dTemp );
		dTemp = BYTE_SWAP( 0x30020000 );
		vWriteDWord((mapRegSpace+ITOFL0), dTemp );
		dTemp = BYTE_SWAP(0x80000025);
		vWriteDWord((mapRegSpace+ITAT0), dTemp);

		// Set CR/CSR PCI address
		dTemp = vReadDWord((mapRegSpace + 0x10));
		dTemp = BYTE_SWAP(dTemp);
		vWriteDWord( (mapRegSpace+CROL), dTemp);

		// Enable the CR/CSR image
		dTemp = vReadDWord((mapRegSpace + CRAT));
		dTemp = (dTemp | 0x80000000);
		vWriteDWord((mapRegSpace+CRAT), dTemp );

		// Allocate 1MB to access the Interconnect Region
		l64Temp = (dTsi148Addr - 0x100000);
		interconnectMem = sysMemGetPhysPtrLocked( l64Temp, 0x100000);
		if(interconnectMem == NULL)
		{
			sysDebugWriteString("No handle to read mapped window!\n");
			return E__FAIL;
		}
	}

	return E__OK;

} /* InitialiseTsi148VMEDevice () */
#endif

UINT32 InitialiseUniverseIIVMEDevice (UINT8 bInitMode, PCI_PFA pfa)
{
	UINT32	dUniverse48Addr;
	UINT32	dTemp;
	UINT32  regVal;
	UINT64	l64Temp;
	VME_DRV_INFO *vmeDrvInfo;
	UINT32	dSysfail;		/* sysfail still asserted */
	UINT8	bSeconds;		/* current seconds count  */
	UINT8	bEndSeconds;	/* ending seconds count   */

#ifdef DEBUG_SOAK
	char	achBuffer[80];
	sysDebugWriteString ("\nInitialiseUniverseIIVMEDevice() \n");
#endif

	// Enable the bus mastering and Memory/IO space access
	regVal=PCI_READ_WORD(pfa,PCI_COMMAND);
	regVal &= ~(0x07);
	regVal |= 0x06;
	PCI_WRITE_WORD(pfa,PCI_COMMAND,regVal);

	//Get the memory mapped CRG for the local TSI148 512KB
	regVal = PCI_READ_DWORD(pfa,PCI_BAR0);
	//regVal &= ~0xf;
	dUniverse48Addr = regVal;

	//CRG is only 4KB so allocating 4KB
	l64Temp = dUniverse48Addr;
	mapRegSpace = sysMemGetPhysPtrLocked(l64Temp, 0x1000);
	if(mapRegSpace == NULL)
	{
		return E__TEST_NO_MEM;
	}

	vmeDrvInfo->mapRegSpace = mapRegSpace;

	//verify we can access our own Universe bridge
	if( (vReadDWord(vmeDrvInfo->mapRegSpace)) != UNIV_DEVID_VENID )
	{
#ifdef DEBUG_SOAK
		sprintf(achBuffer,"Unable to read from mapped CRG(%x):%x\n",
				vmeDrvInfo->mapRegSpace, vReadDWord(vmeDrvInfo->mapRegSpace));
		sysDebugWriteString(achBuffer);
#endif
		return E__TSI148_ACCESS;
	}
	else
	{

		board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
		vmeDrvInfo->deviceId = UNIV_DEVID_VENID;
		vmeDrvInfo->mapRegSpace = mapRegSpace;
	}

	board_service(SERVICE__BRD_VME_BRIDGELESS_SLOTID, NULL, &vmeDrvInfo->slot);

	bSlotNumber = vmeDrvInfo->slot;


	/* Release LOCAL sysfail */
	vWriteDWord((mapRegSpace+VCSR_CLR), LOC_SYSFAIL);
	vWriteDWord((mapRegSpace+LINT_MAP0), 0x00000000);/* map to LINT0 */

	dTemp = vReadDWord((mapRegSpace + LINT_STAT));

#ifdef DEBUG_SOAK
		sprintf(achBuffer,"VME LINT STAT:(%08X)\n",dTemp);
		sysDebugWriteString(achBuffer);
#endif

	vWriteDWord((mapRegSpace+LINT_STAT), dTemp|0x00000004);/* clear VIRQ2 */

	dTemp = vReadDWord((mapRegSpace + MISC_CTL));
	dTemp &= 0x0fefffff;

	if (bSlotNumber == 1)
	{
		dTemp |= 0x60000000;
		 vWriteDWord((mapRegSpace+MISC_CTL), dTemp);
#if 0
		/* Wait for all boards to release SYSFAIL, or timeout (9 sec) */

		bEndSeconds = 0x0F & bReadRtcSeconds ();	/* get current RTC time */
		bEndSeconds = (bEndSeconds + 9) % 10;		/* compute 9S timeout */

		do
		{
			vWriteDWord((mapRegSpace+LINT_STAT), GLB_SYSFAIL); /* clear */
			bSeconds = 0x0F & bReadRtcSeconds ();	/* get current time */
			dSysfail = vReadDWord((mapRegSpace + LINT_STAT));
		} while ((bSeconds != bEndSeconds) && ((dSysfail & GLB_SYSFAIL) != 0));
#endif
	}
	else
	{
		dTemp |= 0x60000000;
		vWriteDWord((mapRegSpace+MISC_CTL), dTemp);
	}




#ifdef DEBUG_SOAK
		sprintf(achBuffer,"VME SlotID:(%x)\n",vmeDrvInfo->slot);
		sysDebugWriteString(achBuffer);
#endif

	if (bInitMode == INIT_BASIC)
		return E__OK;

	/* PCI->VME outbound 0x55000000
	 * - map 16MB from PCI to VME at 0x55000000
	 */
	vWriteDWord((mapRegSpace+LSI0_BS), (dUniverse48Addr-0x02100000));
	vWriteDWord((mapRegSpace+LSI0_BD), ((dUniverse48Addr-0x02100000)+0x1000000));
	vWriteDWord((mapRegSpace+LSI0_TO), (0x55000000 - (dUniverse48Addr - 0x02100000)));
	//vWriteDWord((mapRegSpace+LSI0_CTL), 0x80820000);
	vWriteDWord((mapRegSpace+LSI0_CTL), 0xC0820000);


	/* PCI->VME outbound 0xAA000000
	 * - map 16MB from PCI to VME at 0xAA000000
	 */
	vWriteDWord((mapRegSpace+LSI1_BS), (dUniverse48Addr -0x01100000));
	vWriteDWord((mapRegSpace+LSI1_BD),((dUniverse48Addr -0x01100000)+ 0x1000000));
	vWriteDWord((mapRegSpace+LSI1_TO), (0xAA000000-(dUniverse48Addr -0x01100000)));
	vWriteDWord((mapRegSpace+LSI1_CTL), 0xC0820000);


	/* PCI->VME outbound 0x66000000 for DMA
	 * - map 16MB from PCI to VME at 0x66000000
	 */
	vWriteDWord((mapRegSpace+LSI2_BS), (dUniverse48Addr-0x04180000));
	vWriteDWord((mapRegSpace+LSI2_BD), ((dUniverse48Addr-0x04180000)+0x1000000));
	vWriteDWord((mapRegSpace+LSI2_TO), (0x66000000 - (dUniverse48Addr - 0x04180000)));
	vWriteDWord((mapRegSpace+LSI2_CTL), 0xC0820000);


	/* PCI->VME outbound 0x99000000 for DMA
	 * - map 16MB from PCI to VME at 0x99000000
	 */
	vWriteDWord((mapRegSpace+LSI3_BS), (dUniverse48Addr-0x03180000));
	vWriteDWord((mapRegSpace+LSI3_BD), ((dUniverse48Addr-0x03180000)+0x1000000));
	vWriteDWord((mapRegSpace+LSI3_TO), (0x99000000 - (dUniverse48Addr - 0x03180000)));
	vWriteDWord((mapRegSpace+LSI3_CTL), 0xC0820000);


	l64Temp = (dUniverse48Addr - 0x02100000);
	vmeDrvInfo->outboundAddr1 = (UINT32 *)sysMemGetPhysPtrLocked( l64Temp, 0x01000000);
	if (vmeDrvInfo->outboundAddr1 == NULL)
	{
		sysDebugWriteString("No handle to read mapped window!\n");
		//vFreePtr(mCrCsrHandle); <---------------------------------------------
		return E__FAIL;
	}

	l64Temp = (dUniverse48Addr - 0x01100000);
	vmeDrvInfo->outboundAddr2 = (UINT32 *)sysMemGetPhysPtrLocked( l64Temp, 0x01000000);
	if (vmeDrvInfo->outboundAddr2 == NULL)
	{
		sysDebugWriteString("No handle to read mapped window!\n");
		//vFreePtr(mCrCsrHandle); <---------------------------------------------
		return E__FAIL;
	}


	l64Temp = (dUniverse48Addr - 0x04180000);
		vmeDrvInfo->outboundAddr3 = (UINT32 *)sysMemGetPhysPtrLocked( l64Temp, 0x01000000);
		if (vmeDrvInfo->outboundAddr3 == NULL)
		{
			sysDebugWriteString("No handle to read mapped window!\n");
			//vFreePtr(mCrCsrHandle); <---------------------------------------------
			return E__FAIL;
		}

		l64Temp = (dUniverse48Addr - 0x03180000);
		vmeDrvInfo->outboundAddr4 = (UINT32 *)sysMemGetPhysPtrLocked( l64Temp, 0x01000000);
		if (vmeDrvInfo->outboundAddr4 == NULL)
		{
			sysDebugWriteString("No handle to read mapped window!\n");
			//vFreePtr(mCrCsrHandle); <---------------------------------------------
			return E__FAIL;
		}

	/* Enable inbound image1 */
	vWriteDWord((mapRegSpace+VSI0_BS), 0x55000000 + (bSlotNumber * 0x00100000));
	vWriteDWord((mapRegSpace+VSI0_BD), (0x55000000 + (bSlotNumber * 0x00100000)+0x100000)); //was 0x100000
	l64Temp = 0x10000000 - (0x55000000 + (bSlotNumber * 0x00100000));
	vWriteDWord((mapRegSpace+VSI0_TO), l64Temp);
	vWriteDWord((mapRegSpace+VSI0_CTL), 0xC0F20000);


	/* Enable inbound image2 */
	vWriteDWord((mapRegSpace+VSI1_BS), 0xAA000000 + (bSlotNumber * 0x00100000));
	vWriteDWord((mapRegSpace+VSI1_BD), (0xAA000000 + (bSlotNumber * 0x00100000)+0x100000));
	l64Temp = 0x10200000 - (0xAA000000 + (bSlotNumber * 0x00100000));
	vWriteDWord((mapRegSpace+VSI1_TO), l64Temp);
	vWriteDWord((mapRegSpace+VSI1_CTL), 0xC0F20000);


	/* Enable inbound image3 */
	vWriteDWord((mapRegSpace+VSI2_BS), 0x66000000 + (bSlotNumber * 0x00100000));
	vWriteDWord((mapRegSpace+VSI2_BD), (0x66000000 + (bSlotNumber * 0x00100000)+0x180000));
	l64Temp = 0x10400000 - (0x66000000 + (bSlotNumber * 0x00100000));
	vWriteDWord((mapRegSpace+VSI2_TO), l64Temp);
	vWriteDWord((mapRegSpace+VSI2_CTL), 0xC0F20000);


	/* Enable inbound image4 */
	vWriteDWord((mapRegSpace+VSI3_BS), 0x99000000 + (bSlotNumber * 0x00100000));
	vWriteDWord((mapRegSpace+VSI3_BD), (0x99000000 + (bSlotNumber * 0x00100000)+0x180000));
	l64Temp = 0x10600000 - (0x99000000 + (bSlotNumber * 0x00100000));
	vWriteDWord((mapRegSpace+VSI3_TO), l64Temp);
	vWriteDWord((mapRegSpace+VSI3_CTL), 0xC0F20000);

		/* Wait for all boards to release SYSFAIL, or timeout (9 sec) */

		bEndSeconds = 0x0F & bReadRtcSeconds ();	/* get current RTC time */
		bEndSeconds = (bEndSeconds + 9) % 10;		/* compute 9S timeout */

		do
		{
			vWriteDWord((mapRegSpace+LINT_STAT), GLB_SYSFAIL); /* clear */
			bSeconds = 0x0F & bReadRtcSeconds ();	/* get current time */
			dSysfail = vReadDWord((mapRegSpace + LINT_STAT));
		} while ((bSeconds != bEndSeconds) && ((dSysfail & GLB_SYSFAIL) != 0));

	// Initialise Auto ID counters
	//dNextCrCsrBase = CSR_SIZE;// CR/CSR base allocation counter


	return E__OK;
}

/*****************************************************************************
 * InitialiseVMEDevice: Initialise VME Bridge
 *
 * RETURNS: success or error code
 */
UINT32 InitialiseVMEDevice(void)
{
	PCI_PFA	pfa;
	UINT8	bridgePresent;

	board_service(SERVICE__BRD_VME_BRIDGELESS_MODE, NULL, &bridgePresent);
	if (bridgePresent != 0)
	{
		sysDebugWriteString ("Initializing: VME bridge\n");

		/* find tsi 148 device */
		if (iPciFindDeviceById(1, VID_TUNDRA, DID_TSI148, &pfa) != E__OK)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString ("\nTSI148 not found\n");
#endif
			/* find universe device */
			if (iPciFindDeviceById(1, VID_TUNDRA, DID_UNIVERSE, &pfa) != E__OK)
			{
#ifdef DEBUG_SOAK
			sysDebugWriteString ("\nUniverse II Device not found\n");
#endif
				return E__FAIL;
			}
			else
			{
				globalVMEID = PCI_READ_DWORD(pfa, PCI_VENDOR_ID);
			}
		}
		else
		{
			globalVMEID = PCI_READ_DWORD(pfa, PCI_VENDOR_ID);
		}
	
		if (globalVMEID == TSI148_DEVID_VENID)
		{
			if (InitialiseTsi148VMEDevice(1, pfa) == E__FAIL)
			{
				return E__FAIL;
			}
		}
		else if (globalVMEID == UNIV_DEVID_VENID)
		{
			if (InitialiseUniverseIIVMEDevice(1, pfa) == E__FAIL)
			{
				return E__FAIL;
			}
		}
	}

	return E__OK;
} /* InitialiseVMEDevice () */


/*****************************************************************************
 * configureCoopWindows: this configures 4MB windows from PCI->VME
 *
 * RETURNS: E__OK or E__FAIL
 */


#if 0
static UINT32 COOP_MEM_BUFF =  0x00000000;
#endif

void configureCoopWindows(sm3vmeSlave *asSlots, sm3vme_COOP *syscon_coop)
{
#if 0
	UINT32 i;
	UINT32 dID, dBAR, dTemp/*, dTemp2*/;
	UINT32 offset;
	UINT64 l64Temp;
	UINT32 present = 0;
	UINT32 COOP_MEM_BUFF_TEMP;
#ifdef DEBUG_SOAK
	char  buff[80];
#endif

#ifdef DEBUG_SOAK
	sysDebugWriteString("\n\nconfigureCoopWindows() \n\n");
#endif

	COOP_MEM_BUFF_TEMP = COOP_MEM_BUFF;

	/* 					SYSCON
	 * 				Inbound Window
	 * Map 64MB from 64MB offset to VME for COOP testing
	 * Map at COOP_MEM_BUFF on VME
	 *  */
	dTemp = BYTE_SWAP( COOP_MEM_BUFF_TEMP );
	vWriteDWord( (mapRegSpace+ITSAL0), dTemp);
	dTemp = BYTE_SWAP( (COOP_MEM_BUFF_TEMP + 0x4000000) );
	vWriteDWord((mapRegSpace+ITEAL0), dTemp );
	// Map this chunk to 64MB RAM at offset 0x4000000
	l64Temp = 0x4000000 - COOP_MEM_BUFF_TEMP;
	dTemp = l64Temp;
	dTemp = BYTE_SWAP( dTemp);
	vWriteDWord((mapRegSpace+ITOFL0), dTemp );
	dTemp = (l64Temp>>32);
	dTemp = BYTE_SWAP( dTemp );
	vWriteDWord((mapRegSpace+ITOFU0), dTemp );
	//Enable in-bound transactions on this window
	dTemp = BYTE_SWAP(0x80000025);
	vWriteDWord((mapRegSpace+ITAT0), dTemp);



	/* 				   PERIPHERALS
	 * 				Outbound Window
	 * Map 4MB VME to VME COOP_MEM_BUFF (offset by 4MB * slotnumber)
	 *  */
	for(i =10; i< MAX_SLAVES-1; i++)
	{
		dID  = NULL;
		dBAR = NULL;

		if(asSlots[i].dBar != NULL)
		{
			offset = crCsrMem + (CSR_SIZE*i);
			dID  = vReadDWord(offset);
			dBAR = (vReadDWord(offset + 0x10)) & 0xfffffff0;

#ifdef DEBUG_SOAK
			sprintf( buff, "\nSlot %d, offset = 0x%x, 512K = 0x%x,  Bar = 0x%x\n",
					i, offset, dID, dBAR);
			sysDebugWriteString(buff);
#endif
	}

		if(dID == 0x14810e3)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString("TSI148 Board, configuring coop window\n");
#endif
			present++;

			dTemp   = BYTE_SWAP( (dBAR - 0x500000) );
			vWriteDWord( (offset+OTSAL2), dTemp);
			dTemp   = BYTE_SWAP( (dBAR - 0x100000) );
			vWriteDWord((offset+OTEAL2), dTemp );
	
			l64Temp = (COOP_MEM_BUFF_TEMP - (dBAR - 0x500000) );
			dTemp   = l64Temp;
			dTemp   = BYTE_SWAP( dTemp );
			vWriteDWord((offset+OTOFL2), dTemp );
			dTemp   = l64Temp>>32;
			dTemp   = BYTE_SWAP(dTemp );
			vWriteDWord((offset+OTOFU2), dTemp );
	
			dTemp   = BYTE_SWAP(0x80000042);
			vWriteDWord((offset+OTAT2), dTemp);

			asSlots[i].coop[COOP_LM_WR].offset = dBAR - 0x500000;
			asSlots[i].coop[COOP_LM_WR].length = 0x400000;
			asSlots[i].coop[COOP_LM_DA].offset = dBAR - 0x500000;
			asSlots[i].coop[COOP_LM_DA].length = 0x400000;

#ifdef DEBUG_SOAK
			sprintf( buff, "VME offset = 0x%x, local offset = 0x%x, length = 0x%x\n",
					COOP_MEM_BUFF_TEMP,
					asSlots[i].coop[COOP_LM_WR].offset,
					asSlots[i].coop[COOP_LM_WR].length );
			sysDebugWriteString(buff);
#endif

			COOP_MEM_BUFF_TEMP += 0x400000;
		}
	}


	/* 					SYSCON
	 * 				Outbound Window
	 * Map 4MB VME to VME COOP_MEM_BUFF (offset by 4MB * slotnumber)
	 *  */
	if(present > 0)
	{
#ifdef DEBUG_SOAK
		sysDebugWriteString("SysCon, configuring coop window\n");
#endif

		dBAR = (vReadDWord(mapRegSpace + 0x10)) & 0xfffffff0;

		dTemp   = BYTE_SWAP( (dBAR - 0x500000) );
		vWriteDWord( (mapRegSpace+OTSAL2), dTemp);
		dTemp   = BYTE_SWAP( (dBAR - 0x100000) );
		vWriteDWord((mapRegSpace+OTEAL2), dTemp );

		l64Temp = (COOP_MEM_BUFF_TEMP - (dBAR - 0x500000) );
		dTemp   = l64Temp;
		dTemp   = BYTE_SWAP( dTemp );
		vWriteDWord((mapRegSpace+OTOFL2), dTemp );
		dTemp   = l64Temp>>32;
		dTemp   = BYTE_SWAP(dTemp );
		vWriteDWord((mapRegSpace+OTOFU2), dTemp );

		dTemp   = BYTE_SWAP(0x80000042);
		vWriteDWord((mapRegSpace+OTAT2), dTemp);


		syscon_coop[COOP_LM_WR].offset = dBAR - 0x500000;
		syscon_coop[COOP_LM_WR].length = 0x400000;
		syscon_coop[COOP_LM_DA].offset = dBAR - 0x500000;
		syscon_coop[COOP_LM_DA].length = 0x400000;

#ifdef DEBUG_SOAK
		sprintf( buff, "VME offset = 0x%x, local offset = 0x%x, length = 0x%x\n\n\n",
				COOP_MEM_BUFF_TEMP,
				syscon_coop[COOP_LM_WR].offset,
				syscon_coop[COOP_LM_WR].length );
		sysDebugWriteString(buff);
#endif

		COOP_MEM_BUFF_TEMP += 0x400000;
	}

	COOP_MEM_BUFF += 0x10000000;
	if(COOP_MEM_BUFF >= 0xD0000000)
		COOP_MEM_BUFF = 0x00000000;
#endif
}

/*****************************************************************************
 * Int2Handler: this is the interrupt service routine for the TSI148 PCI
 * interrupt  - specifically it handles Level-2 interrupts for Auto-ID
 *
 * RETURNS: none
 */

//not sure if this works must be tested in a chassis without geographic address

UINT32 intHappened = 0;

void Int2Handler (void)
{
//	UINT32	dIack;

	intHappened++;

	/* Mask interrupt and clear down */
	sysMaskPin (INT_PIN_NUM, SYS_IOAPIC0);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
	/* read the IACK register to clear the Interrupt */
	/*dIack =*/ vReadDWord((mapRegSpace + VIACK2));
#pragma GCC diagnostic pop

	/* Assign the CR/CSR address for this slot */
	vWriteDWord((mapRegSpace + 0x80000 + CR_CSR_OFS + VCSR_BS), dNextCrCsrBase);

	/* Prepare for next interrupting agent */
	dNextCrCsrBase += CSR_SIZE;

	sysUnmaskPin (INT_PIN_NUM, SYS_IOAPIC0);

} /* Int2Handler () */


#if 0
void UnivIntHandler (void)
{
//	UINT32	dIack;


	intHappened++;

	/* Mask interrupt and clear down */
	sysMaskPin (INT_PIN_NUM, SYS_IOAPIC0);
#if 1
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-value"
	/* read the IACK register to clear the Interrupt */
	vWriteDWord((mapRegSpace +LINT_EN), 0x00000000);
#pragma GCC diagnostic pop
#endif
	vWriteDWord((mapRegSpace +LINT_EN), 0x00000000);
	vWriteDWord((mapRegSpace +LINT_STAT), 0x00000004);

	/* Assign the CR/CSR address for this slot */
	/* Assign the CR/CSR address for this slot */
	vWriteDWord((mapRegSpace + 0x80000 + CR_CSR_OFS + VCSR_BS), dNextCrCsrBase);


	vWriteDWord((mapRegSpace +LINT_EN), 0x00000004);

	/* Prepare for next interrupting agent */
	dNextCrCsrBase += CSR_SIZE;

	sysUnmaskPin (INT_PIN_NUM, SYS_IOAPIC0);

} /* Int2Handler () */
#endif

/*****************************************************************************
 * PerformTsi148AutoId: this runs the Auto-ID process for either monarch or slave
 *
 * RETURNS: the logical slot number for this board
 */
UINT8 PerformTsi148AutoId (void)
{
	UINT32	dTemp, dTemp1;	/* DWORD temporary store  */
	UINT32	dSysfail;		/* sysfail still asserted */
	UINT8	bSeconds;		/* current seconds count  */
	UINT8	bEndSeconds;	/* ending seconds count   */
	int		vector ;
	UINT64	l64Temp;		/* 64bit temp var         */
#ifdef DEBUG_SOAK
	char	buff[80];
#endif

	if (IdentifyAgent () == ID__MONARCH)
	{
		/* work out our vector number ?*/

		/* Enable PCI slave image 1 */
		vWriteDWord((mapRegSpace+OTAT1), 0x45000080);

		/* Release LOCAL sysfail */
		dTemp = vReadDWord((mapRegSpace + VCTRL));
		dTemp = dTemp & (~SFAILAI);
		vWriteDWord( (mapRegSpace+VCTRL), dTemp);

		vWriteDWord( (mapRegSpace+INTM2), 0x00000000);
		vWriteDWord( (mapRegSpace+INTS ), 0x04000000);
		vWriteDWord( (mapRegSpace+INTEN), 0x04000000);
		vWriteDWord( (mapRegSpace+INTEO), 0x04000000);

		/* Wait for all boards to release SYSFAIL, or timeout (9 sec) */
		bEndSeconds = 0x0F & bReadRtcSeconds ();	/* get current RTC time */
		bEndSeconds = (bEndSeconds + 9) % 10;		/* compute 9S timeout */

		do
		{
			bSeconds = 0x0F & bReadRtcSeconds ();	/* get current time */
			dSysfail = vReadDWord((mapRegSpace + VSTAT));

		} while ((bSeconds != bEndSeconds) && ((dSysfail & SYSFLS) != 0));


		/* Install the Interrupt Handler */
		if ((vector = sysPinToVector (INT_PIN_NUM, SYS_IOAPIC0)) == -1)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString ("ERR: sysPinToVector (2) Failed !!!\n") ;
#endif
			return 0;//E__FAIL ; modify this
		}

		if (sysInstallUserHandler (vector, Int2Handler) != 0)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString ("ERR: sysInstallUserHandler() Failed !!!\n")  ;
#endif
			return 0;//E__FAIL ;
		}

		intHappened = 0;

		if (sysUnmaskPin (INT_PIN_NUM, SYS_IOAPIC0) == -1)
	    {
#ifdef DEBUG_SOAK
			sysDebugWriteString ("ERR: sysUnmaskPin() Failed !!!\n") ;
#endif
			sysInstallUserHandler (vector, 0) ;
			return 0;//E__FAIL ;
		}

		/* Give Auto-ID time to (hopefully) complete */
        bEndSeconds = 0x0F & bReadRtcSeconds ();
        bEndSeconds = (bEndSeconds + 5) % 10;   /* compute 5 second timeout */

		do
		{
			bSeconds = 0x0F & bReadRtcSeconds ();
		} while (bSeconds != bEndSeconds);


#ifdef DEBUG_SOAK
		sprintf(buff, "intHappened %x times\n", intHappened);
		sysDebugWriteString (buff) ;
#endif

		/* Now Disable Interrupts... */
		sysDisableInterrupts() ;
		sysMaskPin (INT_PIN_NUM, SYS_IOAPIC0);
		sysInstallUserHandler (vector, 0) ;

#ifdef INCLUDE_LWIP_SMC
		sysEnableInterrupts() ;
#endif

		vWriteDWord( (mapRegSpace+INTEN), 0x00000000);
		vWriteDWord( (mapRegSpace+INTEO), 0x00000000);
		vWriteDWord( (mapRegSpace+INTS) , 0x04000000);

		bSlotNumber = 0;	/* monarch is always slot-0 */
	}
	else	/* ID__SLAVE */
	{
		/* Initiate Auto-ID (releases SYSFAIL) only if not already setup */
		/* Release LOCAL sysfail */
		dTemp = vReadDWord((mapRegSpace + VCSR_BS));
		dTemp = dTemp >> 27;

		if (dTemp == 0)
		{
			dTemp = vReadDWord((mapRegSpace + VCTRL));
			dTemp = dTemp & (~SFAILAI);
			vWriteDWord( (mapRegSpace+VCTRL), dTemp);
#ifdef DEBUG_SOAK
			sysDebugWriteString ("Releasing Sys Fail \n") ;
#endif
		}

		/* Wait for CR/CSR to be reassigned */
		do
		{
			dTemp = vReadDWord((mapRegSpace + VCSR_BS));
			dTemp = BYTE_SWAP( dTemp );
			dTemp = dTemp >> 3;
			(void) bReadRtcSeconds ();	/* do something else as a delay */

		} while (dTemp == 0);

#ifdef DEBUG_SOAK
			sysDebugWriteString ("CR/CSR Reassigned \n") ;
#endif

		/* check if slot number is Geographic address or not */
		if (vReadDWord((mapRegSpace + VSTAT)) & 0x0f000000)
			dTemp--;		/* make slot number 0 based */

		bSlotNumber = dTemp;

#ifdef DEBUG_SOAK
		sprintf(buff, "Slot %x\n", bSlotNumber);
		sysDebugWriteString (buff) ;
#endif

		/* reassign CR/CSR to new slot number */
		/* TSI-148 boards start at slot 10    */
		dTemp = BYTE_SWAP( ((bSlotNumber+10) << 3) );
		vWriteDWord((mapRegSpace+VCSR_BS), dTemp );

		/* Adjust global slot -> local memory mapping */
		dTemp  = 0xD0000000 + 0x10000 + ((bSlotNumber) << 16);
		dTemp1 = BYTE_SWAP(dTemp);
		vWriteDWord((mapRegSpace+ITSAL0), dTemp1 );
		vWriteDWord((mapRegSpace+ITEAL0), dTemp1 );

		l64Temp = 0x20000;
		l64Temp = l64Temp -((UINT64)dTemp & 0x00000000ffffffff);
		dTemp1  = l64Temp;
		dTemp1  = BYTE_SWAP(dTemp1);
		vWriteDWord((mapRegSpace+ITOFL0), dTemp1 );
		dTemp1  = (l64Temp >> 32);
		dTemp1  = BYTE_SWAP(dTemp1);
		vWriteDWord((mapRegSpace+ITOFU0), dTemp1 );

		dTemp1  = BYTE_SWAP(0x80000025);
		vWriteDWord((mapRegSpace+ITAT0),  dTemp1 );

		/* Enable the VCSR image */
		dTemp = vReadDWord((mapRegSpace + CRAT));
		dTemp = (dTemp | 0x80000000);
		vWriteDWord((mapRegSpace+CRAT),   dTemp );
	}

	return (bSlotNumber);
}


UINT8 PerfromUniverseAutoId (void)
{
#if 0
	UINT32	dTemp = 0;		/* DWORD temporary store */
	UINT32	dSysfail;		/* sysfail still asserted */
	UINT8	bSlotNumber;	/* the logical slot number from Auto-ID */
	UINT8	bSeconds;		/* current seconds count */
	UINT8	bEndSeconds;	/* ending seconds count */
	int		vector ;
#ifdef DEBUG_SOAK
	char  buff[80];
#endif
#endif

	if (IdentifyAgent () == ID__MONARCH)
	{
#if 0
		/* work out our vector number */

		/* Enable PCI slave image 1 */

		vWriteDWord((mapRegSpace+LSI1_CTL), 0x80850000);


		/* Release LOCAL sysfail */

		vWriteDWord((mapRegSpace+VCSR_CLR), LOC_SYSFAIL);
		vWriteDWord((mapRegSpace+LINT_MAP0), 0x00000000);/* map to LINT0 */
		vWriteDWord((mapRegSpace+LINT_STAT), 0x00000004);/* clear VIRQ2 */
		vWriteDWord((mapRegSpace+LINT_EN), 0x00000004); /* enable VIRQ2 */


		/* Wait for all boards to release SYSFAIL, or timeout (9 sec) */

		bEndSeconds = 0x0F & bReadRtcSeconds ();	/* get current RTC time */
		bEndSeconds = (bEndSeconds + 9) % 10;		/* compute 9S timeout */

		do
		{
			vWriteDWord((mapRegSpace+LINT_STAT), GLB_SYSFAIL); /* clear */
			bSeconds = 0x0F & bReadRtcSeconds ();	/* get current time */
			dSysfail = vReadDWord((mapRegSpace + LINT_STAT));
		} while ((bSeconds != bEndSeconds) && ((dSysfail & GLB_SYSFAIL) != 0));

		/* Install the Interrupt Handler */
			if ((vector = sysPinToVector (INT_PIN_NUM, SYS_IOAPIC0)) == -1)
			{
	#ifdef DEBUG_SOAK
				sysDebugWriteString ("ERR: sysPinToVector (2) Failed !!!\n") ;
	#endif
				return 0;//E__FAIL ; modify this
			}

			if (sysInstallUserHandler (vector, UnivIntHandler) != 0)
			{
	#ifdef DEBUG_SOAK
				sysDebugWriteString ("ERR: sysInstallUserHandler() Failed !!!\n")  ;
	#endif
				return 0;//E__FAIL ;
			}

			intHappened = 0;

			if (sysUnmaskPin (INT_PIN_NUM, SYS_IOAPIC0) == -1)
		    {
	#ifdef DEBUG_SOAK
				sysDebugWriteString ("ERR: sysUnmaskPin() Failed !!!\n") ;
	#endif
				sysInstallUserHandler (vector, 0) ;
				return 0;//E__FAIL ;
			}

			/* Give Auto-ID time to (hopefully) complete */
	        bEndSeconds = 0x0F & bReadRtcSeconds ();
	        bEndSeconds = (bEndSeconds + 5) % 10;   /* compute 5 second timeout */

			do
			{
				bSeconds = 0x0F & bReadRtcSeconds ();
			} while (bSeconds != bEndSeconds);


	#ifdef DEBUG_SOAK
			sprintf(buff, "intHappened %x times\n", intHappened);
			sysDebugWriteString (buff) ;
	#endif

			/* Now Disable Interrupts... */
			sysDisableInterrupts() ;
			sysMaskPin (INT_PIN_NUM, SYS_IOAPIC0);
			sysInstallUserHandler (vector, 0) ;

	#ifdef INCLUDE_LWIP_SMC
			sysEnableInterrupts() ;
	#endif

			vWriteDWord((mapRegSpace+LINT_EN), 0x00000000); /* disable */
			vWriteDWord((mapRegSpace+LINT_STAT), 0x00000004); /* clear */
#endif
			bSlotNumber = 0;	/* monarch is always slot-0 */
	}
	else	/* ID__SLAVE */
	{
#if 0
		/* Initiate Auto-ID (releases SYSFAIL) only if not already setup */

		dTemp = vReadDWord((mapRegSpace + VCSR_BS));
		dTemp = dTemp >> 27;

		if (dTemp == 0)
		{
			dTemp = vReadDWord((mapRegSpace + MISC_CTL));
			vWriteDWord((mapRegSpace+MISC_CTL), (dTemp | V64AUTO));
		}

		if (dTemp == 32) /* Just here to generate code for use with ICE */
		{
			dTemp = vReadDWord((mapRegSpace + VCSR_CTL));
			vWriteDWord((mapRegSpace+MISC_CTL), (dTemp| 0x80000000));
		}

		/* Wait for CR/CSR to be reassigned */

		do {
				dTemp = vReadDWord((mapRegSpace + VCSR_CTL));
				dTemp = dTemp >> 27;

			(void) bReadRtcSeconds ();	/* do something else as a delay */

		} while (dTemp == 0);

		/* Identify the logical slot number assignment */

		bSlotNumber = dTemp;

		/* Adjust global slot -> local memory mapping */
		vWriteDWord((mapRegSpace+VSI0_BS),0xD0000000 + ((bSlotNumber) << 12));
		vWriteDWord((mapRegSpace+VSI0_BD),0xD0000000 + ((bSlotNumber+1) << 12));
		vWriteDWord((mapRegSpace+VSI0_TO), 0x30020000 - ((bSlotNumber) << 12));

		/* Enable the VCSR image */
		dTemp = vReadDWord((mapRegSpace + VCSR_CTL));
		vWriteDWord((mapRegSpace+VCSR_CTL), (dTemp| 0x80000000));
#endif

				// in bridgeless mode obtain slot id from FPGA
				board_service(SERVICE__BRD_VME_BRIDGELESS_SLOTID, NULL, &bSlotNumber);
	}

	return (bSlotNumber);
}

/*****************************************************************************
 * PerformVmeAutoId: this runs the Auto-ID process for either monarch or slave
 *
 * RETURNS: the logical slot number for this board
 */
UINT8 PerformVmeAutoId (void)
{

	UINT8 slotid;

	if ( globalVMEID ==TSI148_DEVID_VENID )
	{
		slotid = PerformTsi148AutoId ();
	}
	else if (globalVMEID == UNIV_DEVID_VENID)
	{
		slotid = PerfromUniverseAutoId();
	}
	else
	{
		return 0;
	}

	return slotid;
} /* PerformVmeAutoId () */


/*****************************************************************************
 * vmeCheckStatus: This function tests the status bits of TSI148
 * RETURNS: 1 if error condition set, otherwise 0;
 */
UINT8 vmeCheckStatus (void)
{
	UINT32 dStatus=0;

	dStatus = vReadDWord((mapRegSpace + PCI_CSR));

	if((dStatus & 0xf1000000) > 0)
	{
		//clear error bits
		dStatus |= 0xf1000000;
		vWriteDWord((mapRegSpace + PCI_CSR), dStatus);
		return 1;
	}

	return 0;
}



/*****************************************************************************
 * bIcReadByte: read interconnect byte
 *
 * RETURNS: data
 */
UINT32 bIcReadByte( UINT8 bSlot,	UINT16	offset, UINT8* data)
{
	UINT32	dOffset;
	UINT16	done;
	UINT16 	dTimeout;


	if ( (bSlot == bSlotNumber) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		*data = vReadByte((0x20000+offset) );
		return E__OK;
	}

	dOffset = bSlotToBar(bSlot) + offset;
	dTimeout = 10;
	done = 0;
	do
	{
		*data = vReadByte((dOffset));

		if(vmeCheckStatus() == 1)
		{
			dTimeout = dTimeout - 1;
			vDelay (1);
		}
		else
			done = 1;

	}while( (dTimeout>0) && (done == 0));

	if(dTimeout == 0)
	{
		*data = 0xff;
		return E__FAIL;
	}

	return E__OK;

} /* vIcReadReg () */


/*****************************************************************************
 * bIcReadByte: write interconnect byte
 *
 * RETURNS: success or error code
 */
UINT32 bIcWriteByte( UINT8 bSlot,	UINT16	offset, UINT8* data)
{
	UINT32	dOffset;
	UINT16	done;
	UINT16 	dTimeout;

	if ( (bSlot == bSlotNumber) || (bSlot == INTERCONNECT_LOCAL_SLOT))
	{
		vWriteByte((0x20000+offset), (*data));
		return E__OK;
	}

	dOffset = bSlotToBar(bSlot) + offset;
	dTimeout = 10;
	done = 0;
	do
	{
		vWriteByte((dOffset), (*data));

		if(vmeCheckStatus() == 1)
		{
			dTimeout = dTimeout - 1;
			vDelay (1);
		}
		else
			done = 1;

	}while( (dTimeout>0) && (done == 0));

	if(dTimeout == 0)
		return E__FAIL;

	return E__OK;

} /* vIcReadReg () */



