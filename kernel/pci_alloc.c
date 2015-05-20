
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

/* pci_alloc.c - PCI resource (re)allocation
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/pci_alloc.c,v 1.17 2015-03-10 09:57:54 hchalla Exp $
 *
 * $Log: pci_alloc.c,v $
 * Revision 1.17  2015-03-10 09:57:54  hchalla
 * Added Universe VME memory allocation for VME image windows for upto 128MB.
 *
 * Revision 1.16  2015-03-05 11:46:01  hchalla
 * Added reserve PCI Memory space for VME backplane tests.
 *
 * Revision 1.15  2015-02-27 11:29:58  mgostling
 * Corrected alignment of forced memory allocation for TSI148
 *
 * Revision 1.14  2015-02-25 18:00:55  hchalla
 * Added support for new VME Universe Chip II pci alloc configuration memory.
 *
 * Revision 1.13  2015-01-29 10:27:01  mgostling
 * Fix compiler warning messages.
 *
 * Revision 1.12  2015-01-21 17:57:36  gshirasat
 * Removed residual  PEX8624 checks.
 * Fixed an issue with BAR allocation wherein the lowest nibble of the BAR was not masked before checking of valid resource allocation.
 *
 * Revision 1.11  2014-12-16 10:20:35  mgostling
 * Allocate resources for bus 0 before scanning subordinate busses.
 * Allocate largest memory resource first for each controller.
 * Only allocate memory resources on subordinate busses if needed.
 *
 * Revision 1.10  2014-10-03 14:54:30  chippisley
 * Added extra DEBUG messages.
 *
 * Revision 1.9  2014-09-19 09:26:50  mgostling
 * Add support for two TSI721 SRIO devices
 *
 * Revision 1.6.2.1  2014-06-04 15:48:12  hchalla
 * Added support for dual srio.
 *
 * Revision 1.8  2014-06-13 10:20:41  mgostling
 * Resolve compiler warnings
 *
 * Revision 1.7  2014-05-12 16:10:15  jthiru
 * Adding support for VX91x and fixes to existing bugs
 *
 * Revision 1.6  2014-03-06 13:40:30  hchalla
 * Added SRIO TSI721 flag to allocate only BARS 0 and 2 required by CUTE.
 *
 * Revision 1.5  2014-03-04 11:37:10  mgostling
 * Only allocate I/O resources through a bridge if they are needed
 * by a controller behind it.
 *
 * Revision 1.4  2014-01-29 13:27:54  hchalla
 * Modified the allocation algorithm for TSI 721 SRIO chip.
 *
 * Revision 1.3  2013-11-25 11:10:11  mgostling
 * Allocate resources for all six PCI Memory BARs.
 *
 * Revision 1.2  2013-10-08 07:09:46  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:41:29  chippisley
 * Import files into new source repository.
 *
 * Revision 1.16  2012/03/21 15:25:24  cdobson
 * Added some #ifdef DEBUG to reduce the number of compiler warnings.
 * Because all PCI memory allocations are treated as non-prefetchable,
 * close all prefetchable bridge windows. The old code overlaps
 * non-prefetchable and prefetchable memory allocations!
 *
 * Revision 1.15  2011/12/01 17:47:40  hchalla
 * Fixed PCI IO Allocation problem with SMBUS tests.
 *
 * Revision 1.13  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.12  2010/11/04 17:38:23  hchalla
 * Added Debug Statements.
 *
 * Revision 1.11  2010/09/10 10:59:00  cdobson
 * Added code to get PCI memory limit (dPciGetTopOfMem).
 *
 * Revision 1.10  2010/06/23 11:00:42  hmuneer
 * CUTE V1.01
 *
 * Revision 1.9  2010/05/05 15:48:53  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.8  2010/02/26 10:37:41  jthiru
 * Modification to PCI test case
 *
 * Revision 1.7  2010/01/19 12:04:22  hmuneer
 * vx511 v1.0
 *
 * Revision 1.6  2009/06/10 10:05:53  swilson
 * Fix PCI allocation algorithm to prevent board hang during PBIT. Bridge
 *  resource windows need to remain closed until allocation is complete on all subordinate
 *  busses, so set base > limit.
 *
 * Revision 1.5  2009/05/18 09:25:29  hmuneer
 * no message
 *
 * Revision 1.4  2008/09/29 10:14:11  swilson
 * Use vDebugWrite() for debug output.
 *
 * Revision 1.3  2008/09/22 16:15:20  swilson
 * Add parameter-passing to makefile so additional warnings can be passed to build and
 *  board type can be specified.
 *
 * Revision 1.2  2008/07/24 14:48:09  swilson
 * Fixed syntax and implementation details. Moved PCI interrupt codeout to a separate
 *  file - to be implemented at a later date.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <string.h>
#include <bit/bit.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/board_service.h>
#include <devices/pci_devices.h>
#include <private/debug.h>

/* defines */

//#define DEBUG

#define _1MB					0x100000
#define _4KB					0x1000

#define TSI721_VID_DID          0x80AB111D
#define INTEL_PCIE_VID_DID		0x01518086
#define TSI148_VID_DID			0x014810e3
#define UNIV_II_VID_DID    		0x000010e3
#define ATIVGA_VID_DID       	0x94CB1002


/* Force allocation for TSI148 */
#define TSI148_FORCED_SIZE		0x08000000		// (_1MB * (16 + 64)) rounded up to nearest boundary
#define TSI148_GLOBAL_IC_SIZE	_1MB
#define TSI148_MEM_WINDOW		(_1MB * 128)

/* Force allocation for UNIVERSE II */
#define UNIV_FORCED_SIZE			(_1MB * (128))	/* Force allocation for UNIV II	*/
#define UNIV_GLOBAL_IC_SIZE	_1MB
#define UNIV_MEM_WINDOW		(_1MB * 128) //was 64M

/* typedefs */

typedef struct mem_Req {
	UINT32	barAlign;		// memory map alignment
	UINT32	barSize;		// required size
	UINT8	barType;		// 64 or 32 bit memory
	UINT8	barReg;			// bar register
} MEMORY_RESOURCE;

/* locals */

static PCI_PARAMS*	psPciParams;

static UINT32	dPciTopOfMem;			/* highest free memory address */
static UINT16	wPciTopOfIo;			/* highest free I/O address */

/* globals */

/* externals */

/* forward declarations */

static void		vPciConfigureHostBus (UINT8* pbHighestBus, UINT8 bHostBridge);
static UINT8	bPciScanBus (UINT8 bStartBus);

static void		vAllocateBarResources (PCI_PFA pfa, UINT8 bRegCount);
static void		vAllocateExpansionRom (PCI_PFA pfa, UINT8 bRegCount);
static void		vSetLatencyTimer (PCI_PFA pfa);
static void		vSetCacheLineSize (PCI_PFA pfa);

static void		vSetStartMapping (PCI_PFA pfa, UINT8 bSecBus, UINT32* dTempTopOfMem, UINT16* wTempTopOfIo);
static void		vSetEndMapping (PCI_PFA pfa, UINT8 bSubBus, UINT32* dTempTopOfMem, UINT16* wTempTopOfIo);

static UINT16	wSaveAndDisableDevice (PCI_PFA pfa);
static void		vRestoreDevice (PCI_PFA pfa, UINT16 wSaved);



/****************************************************************************
 * sysPciInitialize: depth-first PCI resource allocation
 *
 * RETURNS: none
 */
void sysPciInitialize (void)
{
	UINT8	bHostBridge;
	UINT8	bHighBus;

#ifdef DEBUG
	char buffer[80];
#endif

	if(board_service(SERVICE__BRD_GET_PCI_PARAMS, NULL, &psPciParams) != E__OK)
	{
		sysDebugWriteString ("brdPciGetParams returned NULL.\n");
		return;
	}

	/* Set the resource allocation start points */

	dPciTopOfMem = psPciParams->dPciMemBase;		/* highest free memory address */
	wPciTopOfIo	 = psPciParams->wPciIoBase;			/* highest free I/O address */

#ifdef DEBUG
	sprintf(buffer,"sysPciInitialize: started  wPciTopOfIo: 0x%04x dPciTopOfMem: 0x%08x\n",
				wPciTopOfIo, dPciTopOfMem);
	sysDebugWriteString (buffer);
#endif

	// align top of memory map
	++dPciTopOfMem;

	/* For each host bridge on the board, allocate PCI resources */

	bHighBus = 0;

	for (bHostBridge = 0; bHostBridge < psPciParams->bHostBridges; bHostBridge++)
	{
		vPciConfigureHostBus (&bHighBus, bHostBridge);
		bHighBus++;
	}

#ifdef DEBUG
	sprintf(buffer,"sysPciInitialize: finished wPciTopOfIo: 0x%04x dPciTopOfMem: 0x%08x\n",
				wPciTopOfIo, dPciTopOfMem);
	sysDebugWriteString (buffer);
#endif

	sysDebugWriteString ("Rajan--> Step A\n");


	/* Connect the PIRQ lines to their IRQs */
	board_service(SERVICE__BRD_PCI_ROUTE_PIRQ_IRQ, NULL, NULL);

	sysDebugWriteString ("Rajan--> Step B\n");

} /* sysPciInitialize () */


/****************************************************************************
 * vPciConfigureHostBus: configure all PCI devices behind a Host Bridge
 *
 * Memory and I/O allocation runs towards lower addresses
 *
 * RETURNS: none
 */
static void vPciConfigureHostBus
(
	UINT8*	pbHighestBus,
	UINT8	bHostBridge
)
{
	UINT8	bBus;
	UINT8	bSubBus;
	PCI_HUB_START_PARAMS 	hubStartParams;
	PCI_HUB_END_PARAMS   	hubEndParams;

	/* Get the start bus */

	bBus = *pbHighestBus;

	/* Configure the host bridge. This will set the upper limit for any bridge
	 * windows and write the bus number.
	 */

	hubStartParams.bHostBridge = bHostBridge;
	hubStartParams.bRootBus    = bBus;
	hubStartParams.pdMemAddr   = &dPciTopOfMem;
	hubStartParams.pwIoAddr    = &wPciTopOfIo;
	board_service(SERVICE__BRD_PCI_CFG_HUB_START, NULL, &hubStartParams);

	/* Do a depth first scan of this host bus */

	bSubBus = bPciScanBus (bBus);

	/* Configure the host bridge. This will set the lower limit for any bridge
	 * windows and the subordinate bus number.
 	 */

	hubEndParams.bHostBridge = bHostBridge;
	hubEndParams.bSubBus     = bSubBus;
	hubEndParams.pdMemAddr   = &dPciTopOfMem;
	hubEndParams.pwIoAddr    = &wPciTopOfIo;
	board_service(SERVICE__BRD_PCI_CFG_HUB_END, NULL, &hubEndParams);

	/* Return the new highest bus number */

	*pbHighestBus = bSubBus;

} /* vPciConfigureHostBus () */


/****************************************************************************
 * finddev: scan a bus for a PCI device
 *
 * RETURNS: 0 if not found and 1 if  a device is found
 */

UINT8 finddev(PCI_PFA pfa)
{
	UINT8 flag = 0, iDev=0, ifunc=0, bus=0;
	PCI_PFA	temppfa;
	UINT32  wVenId;

	bus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);

	/* scan bus to see if there are any devices/bridges below this bridge *
	 * if there is, only then assign memory/IO resource                   */
	for (iDev=0; iDev<32; iDev++)
	{
		for (ifunc=0; ifunc<8; ifunc++)
		{
			temppfa = PCI_MAKE_PFA (bus, iDev, ifunc);
			wVenId  = PCI_READ_DWORD (temppfa, 0);
			if (wVenId != 0xFFFFFFFF)
			{
				flag = 1;
				break;
			}
		}
	}

	return flag;
}

/****************************************************************************
 * vAllocateDeviceResources: allocate resources for a conventional PCI device
 *
 * RETURNS: none
 */
void vAllocateDeviceResources
(
	PCI_PFA pfa
)
{
	UINT16	wSaved;
	UINT8	bTemp;

#ifdef DEBUG
	char	buffer[80];
	UINT16	wDeviceId;
	UINT16	wTemp;
#endif

#ifdef DEBUG
	wTemp = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
	wDeviceId = PCI_READ_WORD (pfa, PCI_DEVICE_ID);
	sprintf(buffer,"%03d:%02d:%02d: Device Configure (%04X:%04X)\n",
			PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wTemp, wDeviceId);
	sysDebugWriteString (buffer);
#endif

	wSaved = wSaveAndDisableDevice (pfa);
	bTemp = PCI_READ_BYTE (pfa, PCI_BASE_CLASS);
	if(bTemp == 0x01)
	{
		bTemp = PCI_READ_BYTE (pfa, PCI_SUB_CLASS);
		/* if SATA device then do memory allocation to 6 BAR */
		if(bTemp == 0x6)
		{
			vAllocateBarResources (pfa, 6);
		}
		else
		{
			vAllocateBarResources (pfa, 5);
		}
	}
	else
	{
		vAllocateBarResources (pfa, 6);
	}

	vAllocateExpansionRom (pfa, PCI_ROM_BAR);
	vSetLatencyTimer (pfa);
	vSetCacheLineSize (pfa);

	vRestoreDevice (pfa, wSaved);
}

/****************************************************************************
 * vAllocateCardbusResources: allocate resources for a Cardbus Bridge
 *
 * RETURNS: none
 */
void vAllocateCardbusResources
(
	PCI_PFA pfa
)
{
	UINT16	wSaved;

#ifdef DEBUG
	char	buffer[80];
	UINT16	wDeviceId;
	UINT16	wTemp;
#endif

#ifdef DEBUG
	wTemp = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
	wDeviceId = PCI_READ_WORD (pfa, PCI_DEVICE_ID);
	sprintf(buffer,"%03d:%02d:%02d: CardBus Bridge Configure (%04X:%04X)\n",
			PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wTemp, wDeviceId);
	sysDebugWriteString (buffer);
#endif

	wSaved = wSaveAndDisableDevice (pfa);
	vAllocateBarResources (pfa, 1);
	vSetLatencyTimer (pfa);
	vSetCacheLineSize (pfa);

	/* Write the current bus number to PCI bus number register */
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS, PCI_PFA_BUS (pfa));

	vRestoreDevice (pfa, wSaved);
}

/****************************************************************************
 * bPciScanSubordinateBus: scan a bridge and its subordinate busses and allocate PCI resources
 *
 * RETURNS: new highest bus number
 */
/*static*/ UINT8 bPciScanSubordinateBus
(
	UINT8 bStartBus
)
{
	PCI_PFA	pfa;
	UINT16	wSaved;
	UINT16	wTemp;
	UINT8	bTemp;
	UINT8	bSubBus;
	UINT8	bMultiFunc;
	UINT16	wTempTopOfIo;
	UINT32	dTempTopOfMem;

#ifdef DEBUG
	char	buffer[80];
	UINT16	wDeviceId;
#endif

	/* Set start conditions */

	bMultiFunc = 0;
	bSubBus = bStartBus;	/* at start, highest bus is current bus */

	pfa = PCI_MAKE_PFA (bStartBus, 0, 0);

	/* Scan current bus for devices, recurse on PCI bridges */

	do {
		/* Read VID to see if this function is present */

		wTemp = PCI_READ_WORD (pfa, PCI_VENDOR_ID);

		if (wTemp != 0xFFFF) 	/* if (function present) */
		{
			/* Get PCI config header type (including multifunction bit) */

			bTemp = PCI_READ_BYTE (pfa, PCI_HEADER_TYPE);

			/* If function 0, get multifunction flag */

			if (PCI_PFA_FUNC (pfa) == 0)
			{
				bMultiFunc = (bTemp & 0x80);
			}

			/* Process device based on header type */

			switch (bTemp & 0x7F)
			{
				case 0 :		/* conventional device */
					vAllocateDeviceResources (pfa);
					break;

				case 1 :		/* PCI-to-PCI bridge */
#ifdef DEBUG
					wDeviceId = PCI_READ_WORD (pfa, PCI_DEVICE_ID);
					sprintf(buffer,"%03d:%02d:%02d: PCI Bridge Configure [Bus %d] (%04X:%04X)\n",
							PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa),  bSubBus + 1, wTemp, wDeviceId);
					sysDebugWriteString (buffer);
#endif

					wSaved = wSaveAndDisableDevice (pfa);
					vAllocateBarResources (pfa, 2);
					vAllocateExpansionRom (pfa, PCI_BR_ROM_BAR);
					vSetLatencyTimer (pfa);
					vSetCacheLineSize (pfa);

					/* Set bus mapping and memory windows, then assign
					 * PCI resources to devices on the secondary bus and those beyond.
					 */
					vSetStartMapping (pfa, bSubBus + 1, &dTempTopOfMem, &wTempTopOfIo);
					bSubBus = bPciScanSubordinateBus (bSubBus + 1);
					vSetEndMapping (pfa, bSubBus, &dTempTopOfMem, &wTempTopOfIo);

					/* Enable the bridge */
					PCI_WRITE_WORD (pfa, PCI_COMMAND, wSaved | PCI_BMEN | PCI_MEMEN | PCI_IOEN);

					if (finddev(pfa) != 1)
					{
						wSaveAndDisableDevice (pfa);
					}
					break;

				case 2 :		/* CardBus bridge */
					vAllocateCardbusResources (pfa);
					break;

				default :			/* unknown type (or device error) */
					break;
			}
		}

		/* If device is multi-function, increment function */

		if (bMultiFunc != 0)
		{
			pfa = PCI_PFA_NEXT_FUNC (pfa);
		}

		/* If function# is zero, increment device */

		if (PCI_PFA_FUNC (pfa) == 0)
		{
			pfa = PCI_PFA_NEXT_DEV (pfa);
		}

	} while ((PCI_PFA_DEV (pfa) != 0) || (PCI_PFA_FUNC (pfa) != 0));

	return (bSubBus);

} /* bPciScanSubordinateBus () */


/****************************************************************************
 * bPciScanBus: scan a bus and allocate PCI resources
 *
 * RETURNS: new highest bus number
 */
/*static*/ UINT8 bPciScanBus
(
	UINT8 bStartBus
)
{
	PCI_PFA	pfa;
	UINT16	wSaved;
	UINT16	wTemp;
	UINT8	bTemp;
	UINT8	bSubBus;
	UINT8	bMultiFunc;
	UINT16	wTempTopOfIo;
	UINT32	dTempTopOfMem;

#ifdef DEBUG
	char	buffer[80];
	UINT16	wDeviceId;
#endif

	/* Set start conditions */

	bMultiFunc = 0;
	bSubBus = bStartBus;	/* at start, highest bus is current bus */

	pfa = PCI_MAKE_PFA (bStartBus, 0, 0);

	/****************************************************************************
	 *
	 * Allocate resources for all devices on bus zero first
	 *
	 ****************************************************************************/


	do {
		/* Read VID to see if this function is present */

		wTemp = PCI_READ_WORD (pfa, PCI_VENDOR_ID);

		if (wTemp != 0xFFFF) 	/* if (function present) */
		{
			/* Get PCI config header type (including multifunction bit) */

			bTemp = PCI_READ_BYTE (pfa, PCI_HEADER_TYPE);

			/* If function 0, get multifunction flag */

			if (PCI_PFA_FUNC (pfa) == 0)
			{
				bMultiFunc = (bTemp & 0x80);
			}

			/* Process device based on header type */

			switch (bTemp & 0x7F)
			{
				case 0 :		/* conventional device */
					vAllocateDeviceResources (pfa);
					break;

				case 1 :		/* PCI-to-PCI bridge */
					/****************************************************************************
					*
					* only allocate local resources for this bridge
					*
					****************************************************************************/
#ifdef DEBUG
					wDeviceId = PCI_READ_WORD (pfa, PCI_DEVICE_ID);
					sprintf(buffer,"%03d:%02d:%02d: PCI Bridge Configure [Bus %d] (%04X:%04X)\n",
							PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), bSubBus, wTemp, wDeviceId);
					sysDebugWriteString (buffer);
#endif

					wSaved = wSaveAndDisableDevice (pfa);
					vAllocateBarResources (pfa, 2);
					vAllocateExpansionRom (pfa, PCI_BR_ROM_BAR);
					vSetLatencyTimer (pfa);
					vSetCacheLineSize (pfa);

					vRestoreDevice (pfa, wSaved);
					break;

				case 2 :		/* CardBus bridge */
					vAllocateCardbusResources (pfa);
					break;

				default :			/* unknown type (or device error) */
					break;
			};
		}

		/* If device is multi-function, increment function */

		if (bMultiFunc != 0)
		{
			pfa = PCI_PFA_NEXT_FUNC (pfa);
		}

		/* If function# is zero, increment device */

		if (PCI_PFA_FUNC (pfa) == 0)
		{
			pfa = PCI_PFA_NEXT_DEV (pfa);
		}

	} while ((PCI_PFA_DEV (pfa) != 0) || (PCI_PFA_FUNC (pfa) != 0));

	/****************************************************************************
	 *
	 * now find and recurse through PCI-to-PCI bridges and allocate resources
	 *
	 ****************************************************************************/

	pfa = PCI_MAKE_PFA (bStartBus, 0, 0);

	do {
		/* Read VID to see if this function is present */

		wTemp = PCI_READ_WORD (pfa, PCI_VENDOR_ID);

		if (wTemp != 0xFFFF) 	/* if (function present) */
		{
			/* Get PCI config header type (including multifunction bit) */

			bTemp = PCI_READ_BYTE (pfa, PCI_HEADER_TYPE);

			/* If function 0, get multifunction flag */

			if (PCI_PFA_FUNC (pfa) == 0)
			{
				bMultiFunc = (bTemp & 0x80);
			}

			if ((bTemp & 0x7F) == 1)
			{
				/****************************************************************************
				*
				* only process PCI-to-PCI bridges on bus zero
				*
				****************************************************************************/
#ifdef DEBUG
				wDeviceId = PCI_READ_WORD (pfa, PCI_DEVICE_ID);
				sprintf(buffer,"%03d:%02d:%02d: PCI Bridge Configure [Bus %d] (%04X:%04X)\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa),  bSubBus + 1, wTemp, wDeviceId);
				sysDebugWriteString (buffer);
#endif
				wSaved = wSaveAndDisableDevice (pfa);

				/* Set bus mapping and memory windows, then assign
				 * PCI resources to devices on the secondary bus and those beyond.
				 */
				vSetStartMapping (pfa, bSubBus + 1, &dTempTopOfMem, &wTempTopOfIo);
				bSubBus = bPciScanSubordinateBus (bSubBus + 1);
				vSetEndMapping (pfa, bSubBus, &dTempTopOfMem, &wTempTopOfIo);

				/* Enable the bridge */
				PCI_WRITE_WORD (pfa, PCI_COMMAND, wSaved | PCI_BMEN | PCI_MEMEN | PCI_IOEN);

				if(finddev(pfa) != 1)
				{
					wSaveAndDisableDevice (pfa);
				}
			}
		}

		/* If device is multi-function, increment function */

		if (bMultiFunc != 0)
		{
			pfa = PCI_PFA_NEXT_FUNC (pfa);
		}

		/* If function# is zero, increment device */

		if (PCI_PFA_FUNC (pfa) == 0)
		{
			pfa = PCI_PFA_NEXT_DEV (pfa);
		}

	} while ((PCI_PFA_DEV (pfa) != 0) || (PCI_PFA_FUNC (pfa) != 0));

	return (bSubBus);

} /* bPciScanBus() */

/****************************************************************************
 * vSortMemoryResource: bubble sort resources, largest first
 *
 * RETURNS: none
 */

static void vSortMemoryResource
(
	UINT8 memBarCnt,
	MEMORY_RESOURCE * resource
)
{
	UINT8 swap;
	UINT8 cnt;
	MEMORY_RESOURCE tempResource;

	do {
		swap = FALSE;
		for (cnt = 0; cnt < memBarCnt; ++cnt)
		{
			if (resource[cnt].barSize < resource[cnt + 1].barSize)
			{
				tempResource.barReg = resource[cnt].barReg;
				tempResource.barType = resource[cnt].barType;
				tempResource.barSize = resource[cnt].barSize;
				tempResource.barAlign = resource[cnt].barAlign;

				resource[cnt].barReg = resource[cnt + 1].barReg;
				resource[cnt].barType = resource[cnt + 1].barType;
				resource[cnt].barSize = resource[cnt + 1].barSize;
				resource[cnt].barAlign = resource[cnt + 1].barAlign;

				resource[cnt + 1].barReg = tempResource.barReg;
				resource[cnt + 1].barType = tempResource.barType;
				resource[cnt + 1].barSize = tempResource.barSize;
				resource[cnt + 1].barAlign = tempResource.barAlign;

				swap = TRUE;
			}
		}
	} while (swap == TRUE);
}


/****************************************************************************
 * vAllocateBarResources: allocate resources to a block of BAR registers
 *
 * RETURNS: none
 */

static void vAllocateBarResources
(
	PCI_PFA	pfa,
	UINT8	bRegCount
)
{
	UINT32	dTemp;
	UINT16	wSize;
	UINT8	bCount;
	UINT8	bReg;
	UINT8	skipStatus = 0;
	UINT8	memBarCnt = 0;
	UINT8	bIsTsi148 = 0;
	UINT8	bIsUnivII = 0;
	UINT8	bIsTsi721 = 0;
//	UINT8	bIsIntelPcie = 0;
//	UINT8	bIsATIVGA;
//	UINT32	dTsi148Size;
	UINT32	dIoMask = 0;
	MEMORY_RESOURCE	memResource[7];	// add a null entry at the end
	PCI_CFG_SKIP_IO_ALLOC_PARAMS params;
#ifdef DEBUG
	char buffer [80];
#endif

	params.pfa = pfa;
	params.skipStatus = 0;
	board_service(SERVICE__BRD_PCI_CFG_SKIP_IO_ALLOCATION, NULL, &params);
	skipStatus = params.skipStatus;
	dTemp = PCI_READ_DWORD (pfa, 0);

	if ((dTemp == TSI148_VID_DID) )
		bIsTsi148 = 1;
	else
		bIsTsi148 = 0;

	if ((dTemp == UNIV_II_VID_DID))
	{
		bIsUnivII = 1;
	}
	else
	{
		bIsUnivII = 0;
	}

	if (dTemp == TSI721_VID_DID)
	{
		bIsTsi721 = 1;
	}
	else
	{
		bIsTsi721 = 0;
	}
/*
	if (dTemp == INTEL_PCIE_VID_DID)
	{
		bIsIntelPcie = 1;

	}
	else
	{
		bIsIntelPcie = 0;
	}

	if (dTemp == ATIVGA_VID_DID)
	{
		bIsATIVGA = 1;
	}
	else
	{
		bIsATIVGA = 0;
	}
*/
	memset (&memResource, 0, sizeof(memResource));

	/* Allocate resource based on register type */

	for (bCount = 0; bCount < bRegCount; bCount++)
	{

		bReg = PCI_BAR0 + (bCount * 4);

		/* Write all 1's to get size and type */

		PCI_WRITE_DWORD (pfa, bReg, 0xFFFFFFFF);
		dTemp = PCI_READ_DWORD (pfa, bReg);

#ifdef DEBUG
		sprintf(buffer,"%03d:%02d:%02d: BAR: %04x dTemp: 0x%08x\n",
			PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), bReg, dTemp);
		sysDebugWriteString (buffer);
#endif

		if (bIsTsi148 && (bReg == 0x10))
		{
			// dTsi148Size = (~(dTemp & 0xfffffff0)) + 1;
			dTemp = (~(TSI148_FORCED_SIZE - 1)) | 4;
		}

		if (bIsUnivII && (bReg == 0x10))
		{
			// dTsi148Size = (~(dTemp & 0xfffffff0)) + 1;
			dTemp = (~(UNIV_FORCED_SIZE - 1)) | 0x4;
		}


		if ((dTemp & ~0xf) != 0)		// bar register needs to be configured
		{
			if ((dTemp & 0x00000001) == 1)		/* 16-bit I/O */
			{
				/*if (bIsATIVGA)
				{
#ifdef DEBUG
					sprintf(buffer,"ATI VGA \n");
					vConsoleWrite(buffer);
					sysDebugWriteString (buffer);
#endif
				}
				else*/
				if(skipStatus != 1)
				{
					dTemp = dTemp & 0xFFFFFFFC;		/* mask fixed */
					wSize = (~dTemp) + 1;			/* get 16-bit size */

					wPciTopOfIo -= wSize;
					wPciTopOfIo &= (dTemp);
					wPciTopOfIo &= (dTemp & 0xFFFC);

					if(board_service(SERVICE__BRD_GET_PCI_IOMASK, NULL, &dIoMask) != E__OK)
					{
							//sysDebugWriteString ("PCI IO MASK\n");
							dIoMask = 0xFFFF;
					}
					wPciTopOfIo &= (dTemp & dIoMask);

#ifdef DEBUG
					sprintf(buffer,"%03d:%02d:%02d: IO allocated: %04x size: %u\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wPciTopOfIo, wSize);
					sysDebugWriteString (buffer);
#endif
					PCI_WRITE_DWORD (pfa, bReg, (UINT32)wPciTopOfIo);
				}
			}
			else
			{
				// memory BAR - save resource requirements for sorting
				if (!((bReg == 0x14 || bReg == 0x20) && (bIsTsi721 == 1)))
				{
					if ((dTemp & 0x00000006) == 4)
					{
						bCount++;		// include upper 32-bit register in count
					}

					memResource[memBarCnt].barReg = bReg;
					memResource[memBarCnt].barType = (UINT8)(dTemp & 0x00000007);
					dTemp &= 0xFFFFFFF0;								// create alignment mask
					memResource[memBarCnt].barAlign = dTemp;
					memResource[memBarCnt].barSize = (~dTemp) + 1;		// get 32-bit size
					++memBarCnt;
				}
			}
		}
	}

	// now allocate memory resources, largest first
	if (memBarCnt > 0)
	{
		if (memBarCnt > 1)
		{
			vSortMemoryResource(memBarCnt, &memResource[0]);		// order the resource requests, largest first
		}

		for (bCount = 0; bCount < memBarCnt; ++bCount)
		{
			dPciTopOfMem -= memResource[bCount].barSize;
			dPciTopOfMem &= memResource[bCount].barAlign;

			if ((memResource[bCount].barType & 0x06) == 0)		/* 32-bit mem, locate anywhere */
			{
#ifdef DEBUG
				sprintf(buffer,"%03d:%02d:%02d: 32-bit mem allocated: %#x size: %u(%#x)\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), dPciTopOfMem, memResource[bCount].barSize, memResource[bCount].barSize);
				sysDebugWriteString (buffer);
#endif
				PCI_WRITE_DWORD (pfa, memResource[bCount].barReg, dPciTopOfMem);
			}
			else	/* 64-bit mem, locate anywhere */
			{
#ifdef DEBUG
				sprintf(buffer,"%03d:%02d:%02d: 64-bit mem allocated: %#x size: %u(%#x)\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), dPciTopOfMem, memResource[bCount].barSize, memResource[bCount].barSize);
				sysDebugWriteString (buffer);
#endif

				if ((memResource[bCount].barReg == 0x10) && bIsTsi148)
				{
					/* offset for global_ic */
					PCI_WRITE_DWORD (pfa, memResource[bCount].barReg, dPciTopOfMem + TSI148_GLOBAL_IC_SIZE + TSI148_MEM_WINDOW);
				}
				else  if ((memResource[bCount].barReg == 0x10) && bIsUnivII)
				{
					/* offset for global_ic */
					PCI_WRITE_DWORD (pfa, memResource[bCount].barReg, dPciTopOfMem + UNIV_GLOBAL_IC_SIZE + UNIV_MEM_WINDOW);
				}
				else
				{
					PCI_WRITE_DWORD (pfa, memResource[bCount].barReg, dPciTopOfMem);
					PCI_WRITE_DWORD (pfa, memResource[bCount].barReg + 4, 0);
				}
			}
		}
	}
} /* vAllocateBarResources () */


/****************************************************************************
 * vAllocateExpansionRom: allocate resources to the expansion ROM register
 *
 * RETURNS: none
 */
static void vAllocateExpansionRom
(
	PCI_PFA	pfa,
	UINT8	bReg
)
{
	UINT32	dTemp;
	UINT32	dSize;
#ifdef DEBUG
	char buffer [80];
#endif

	/* Write all 1's to get size, don't set enable bit */

	PCI_WRITE_DWORD (pfa, bReg, 0xFFFFFFFE);
	dTemp = PCI_READ_DWORD (pfa, bReg);

	dTemp = dTemp & 0xFFFFFC00;		/* mask fixed */
	dSize = (~dTemp) + 1;			/* get 32-bit size */

	if (dSize > 0)	/* If reg exists, allocate aligned address */
	{
		dPciTopOfMem -= dSize;
		dPciTopOfMem &= dTemp;


#ifdef DEBUG
		sprintf(buffer,"%s\n", __FUNCTION__);
		sysDebugWriteString (buffer);
		sprintf(buffer,"%03d:%02d:%02d: 32-bit mem allocated: %#x size: %u(%#x)\n",
			PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), dPciTopOfMem, dSize, dSize);
		sysDebugWriteString (buffer);
#endif
		PCI_WRITE_DWORD (pfa, bReg, dPciTopOfMem);
	}

} /* vAllocateExpansionRom () */


/****************************************************************************
 * vSetLatencyTimer: program the latency timer register
 *
 * RETURNS: none
 */
static void vSetLatencyTimer( PCI_PFA pfa )
{
	PCI_WRITE_BYTE (pfa, PCI_LATENCY, psPciParams->bCsLatency);

} /* vSetLatencyTimer () */



/****************************************************************************
 * vSetCacheLineSize: program the cache line size register
 *
 * RETURNS: none
 */
static void vSetCacheLineSize(	PCI_PFA	pfa )
{
	PCI_WRITE_BYTE (pfa, PCI_CACHE_LINE, psPciParams->bCsCacheLine);

} /* vSetCacheLineSize () */



/****************************************************************************
 * vSetStartMapping:
 *
 * This function programs the end address of Memory and I/O windows for a
 * bridge device. The lowest allocated Memory and I/O addresses are used for
 * these parameters. Subsequent to this function, a call will be made to
 * allocate resources to all devices on, or beyond the bridge's secondary bus.
 * Primary and secondary bus numbers are also set here.
 *
 * RETURNS: none
 */
static void vSetStartMapping( PCI_PFA pfa, UINT8 bSecBus, UINT32* dTempTopOfMem, UINT16* wTempTopOfIo )
{
	/* Set upper limit of windows */
#ifdef DEBUG
	char buffer[80];
	UINT16 wTemp;
	UINT8 bTemp;
#endif
//	UINT8 bIsATIVGA;

#ifdef DEBUG
	sprintf(buffer,"%s: \n", __FUNCTION__);
	sysDebugWriteString (buffer);
	sprintf(buffer,"%03d:%02d:%02d: on entry: dPciTopOfMem: %08x: wPciTopOfIo: %04x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), dPciTopOfMem, wPciTopOfIo);
	sysDebugWriteString (buffer);
#endif

	/* Set primary and secondary bus numbers, sub = 255 for present */

	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS, PCI_PFA_BUS (pfa));
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS, bSecBus);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS, 0xFF);

	// disable all memory access downstream of current bridge
	PCI_WRITE_WORD (pfa, PCI_MEM_LIMIT, 0);
	PCI_WRITE_WORD (pfa, PCI_MEM_BASE, 0xFFFF);

	// disable all I/O access downstream of current bridge
	PCI_WRITE_BYTE (pfa, PCI_IO_LIMIT, 0);
	PCI_WRITE_BYTE (pfa, PCI_IO_BASE, 0xFF);

	// align memory and IO adresses for this bus
	if ((dPciTopOfMem & (_1MB - 1)) != 0)
	{
		dPciTopOfMem &= (~(_1MB - 1));					/* align to next 1-MB boundary */
	}

	if ((wPciTopOfIo & (_4KB - 1)) != 0)
	{
		wPciTopOfIo &= (~(_4KB - 1));					/* align to next 4-KB boundary */
	}

	// set start of aligned memory & IO addresses for devices on this bus
	*dTempTopOfMem = dPciTopOfMem;
	*wTempTopOfIo = wPciTopOfIo;

	// now check for any devices on the secondary bus
	if (finddev(pfa))
	{
#ifdef DEBUG
		UINT8 dTemp;
		dTemp = PCI_READ_DWORD (pfa, 0);

		sprintf(buffer,"%03d:%02d:%02d: aligned for new allocation: dPciTopOfMem: %08x\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), dPciTopOfMem);
		sysDebugWriteString (buffer);
		sprintf(buffer,"%03d:%02d:%02d: updated dTempTopOfMem: %08x\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), *dTempTopOfMem);
		sysDebugWriteString (buffer);
		wTemp = PCI_READ_WORD (pfa, PCI_MEM_BASE);
		sprintf(buffer,"%03d:%02d:%02d: MEM BASE =%04x\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wTemp);
		sysDebugWriteString (buffer);
		wTemp = PCI_READ_WORD (pfa, PCI_MEM_LIMIT);
		sprintf(buffer,"%03d:%02d:%02d: MEM LIMIT=%04x\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wTemp);
		sysDebugWriteString (buffer);

		sprintf(buffer,"%03d:%02d:%02d: aligned for new allocation: wPciTopOfIo: %04x\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wPciTopOfIo);
		sysDebugWriteString (buffer);
		sprintf(buffer,"%03d:%02d:%02d: updated wTempTopOfIo: %04x\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), *wTempTopOfIo);
		sysDebugWriteString (buffer);
		bTemp = PCI_READ_BYTE (pfa, PCI_IO_BASE);
		sprintf(buffer,"%03d:%02d:%02d: IO BASE =%02x\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), bTemp);
		sysDebugWriteString (buffer);
		bTemp = PCI_READ_BYTE (pfa, PCI_IO_LIMIT);
		sprintf(buffer,"%03d:%02d:%02d: IO LIMIT=%02x\n",
						PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), bTemp);
		sysDebugWriteString (buffer);
#endif
	}

} /* vSetStartMapping () */



/****************************************************************************
 * vSetEndMapping:
 *
 * This function sets the start addresses of the Memory and I/O windows of a
 * bridge device. This is done after all the assignments have been completed
 * for devices on the bridge's secondary bus (including any further bridges
 * and busses). The subordinate bus number is also set.
 *
 * RETURNS: none
 */
static void vSetEndMapping( PCI_PFA	pfa, UINT8 bSubBus, UINT32* dTempTopOfMem, UINT16* wTempTopOfIo )
{
	UINT32	dTemp;
	UINT16	wTemp;
#ifdef DEBUG
	char buffer[80];
	UINT8	bTemp;
#endif

#ifdef DEBUG
	sprintf(buffer,"%s: \n", __FUNCTION__);
	sysDebugWriteString (buffer);
#endif

	dTemp = PCI_READ_DWORD (pfa, 0);

#ifdef DEBUG
	sprintf(buffer,"%03d:%02d:%02d: dPciTopOfMem: %08x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), dPciTopOfMem);
	sysDebugWriteString (buffer);
	sprintf(buffer,"%03d:%02d:%02d: dTempTopOfMem: %08x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), *dTempTopOfMem);
	sysDebugWriteString (buffer);

	sprintf(buffer,"%03d:%02d:%02d: wPciTopOfIo: %04x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wPciTopOfIo);
	sysDebugWriteString (buffer);
	sprintf(buffer,"%03d:%02d:%02d: wTempTopOfIo: %04x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), *wTempTopOfIo);
	sysDebugWriteString (buffer);
#endif

	if (*dTempTopOfMem != dPciTopOfMem)
	{
		PCI_WRITE_WORD (pfa, PCI_MEM_BASE, dPciTopOfMem >> 16);		// set current window

		*dTempTopOfMem &= ~(_1MB - 1);								// align to next lower 1MB boundary
		PCI_WRITE_WORD (pfa, PCI_MEM_LIMIT, (*dTempTopOfMem - 1) >> 16);
	}

	if (*wTempTopOfIo != wPciTopOfIo)
	{
		PCI_WRITE_BYTE (pfa, PCI_IO_BASE, wPciTopOfIo >> 8);		// set current 4K window

		*wTempTopOfIo &= (~(_4KB - 1));								// align to next 4K boundary
		PCI_WRITE_BYTE (pfa, PCI_IO_LIMIT, (*wTempTopOfIo - 1) >> 8);
	}

#ifdef DEBUG
	wTemp = PCI_READ_WORD (pfa, PCI_MEM_BASE);
	sprintf(buffer,"%03d:%02d:%02d: MEM BASE =%04x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wTemp);
	sysDebugWriteString (buffer);
	wTemp = PCI_READ_WORD (pfa, PCI_MEM_LIMIT);
	sprintf(buffer,"%03d:%02d:%02d: MEM LIMIT=%04x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), wTemp);
	sysDebugWriteString (buffer);

	bTemp = PCI_READ_BYTE (pfa, PCI_IO_BASE);
	sprintf(buffer,"%03d:%02d:%02d: IO BASE =%02x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), bTemp);
	sysDebugWriteString (buffer);
	bTemp = PCI_READ_BYTE (pfa, PCI_IO_LIMIT);
	sprintf(buffer,"%03d:%02d:%02d: IO LIMIT=%02x\n",
					PCI_PFA_BUS(pfa),PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa), bTemp);
	sysDebugWriteString (buffer);
#endif


		/* Write memory base/limit to pre-fetch memory base/limit */
	/* dTemp = PCI_READ_DWORD (pfa, PCI_MEM_BASE);
	PCI_WRITE_DWORD (pfa, PCI_PREF_BASE, dTemp); */
	/* all memory is treated as non-prefetchable so close prefetchable window */
	if (dTemp == INTEL_PCIE_VID_DID)
	{
		PCI_WRITE_WORD (pfa, PCI_PREF_BASE, 0x0);
	}
	else
	{
		PCI_WRITE_WORD (pfa, PCI_PREF_BASE, 0xFFFF);
	}


	/* Zero upper bits of prefetch memory base and limit */
	PCI_WRITE_DWORD (pfa, PCI_PREF_UBASE, 0);
	PCI_WRITE_DWORD (pfa, PCI_PREF_ULIMIT, 0);

	/* Zero upper bits of I/O base and limit */
	PCI_WRITE_DWORD (pfa, PCI_IO_UBASE, 0);

	/* Set subordinate bus number to correct value */
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS, bSubBus);

	/* Enable I/O and memory accesses through device */
	wTemp = PCI_READ_DWORD (pfa, PCI_COMMAND);
	PCI_WRITE_DWORD (pfa, PCI_COMMAND, wTemp | 0x0003);

} /* vSetEndMapping () */



/****************************************************************************
 * wSaveAndDisableDevice: save PCI command register and disable device
 *
 * RETURNS: original command register contents
 */
static UINT16 wSaveAndDisableDevice( PCI_PFA pfa )
{
	UINT16	wTemp;
	UINT16	wCmd;

	/* Clear bus-master, memory and I/O bits */
	wCmd = PCI_READ_WORD (pfa, PCI_COMMAND);
	PCI_WRITE_WORD (pfa, PCI_COMMAND, wCmd & ~(PCI_BMEN | PCI_MEMEN | PCI_IOEN));

	/* If this is a PLX 8114 bridge then enable bus mastering  */
	/* to fix a silicon bug! 								   */
	wTemp = PCI_READ_WORD (pfa, PCI_VENDOR_ID);

	if (wTemp == PCI_VID_PLX)
	{
		wTemp = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

		if (wTemp == PCI_DID_8114)
		{
			wTemp = PCI_READ_WORD (pfa, PCI_COMMAND);
			PCI_WRITE_WORD (pfa, PCI_COMMAND, wTemp | PCI_BMEN);
		}
	}

	/* Return unmodified register state */
	return (wCmd);

} /* wSaveAndDisableDevice () */


/****************************************************************************
 * vRestoreDevice: restore PCI command register status
 *
 * RETURNS: none
 */
static void vRestoreDevice(	PCI_PFA	pfa, UINT16	wSaved )
{
	PCI_WRITE_WORD (pfa, PCI_COMMAND, wSaved);

} /* dRestoreDevice () */


/****************************************************************************
 * sysPciGetTopOfMem: depth-first PCI resource allocation
 *
 * RETURNS: none
 */

UINT32 dPciGetTopOfMem (void)
{
	return dPciTopOfMem ;

}

