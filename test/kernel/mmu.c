
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

/* mmu.c - miscellaneous functions supporting memory access management
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/mmu.c,v 1.4 2014-03-06 13:39:19 hchalla Exp $
 *
 * $Log: mmu.c,v $
 * Revision 1.4  2014-03-06 13:39:19  hchalla
 * Added more reserved memory from physical address (0x05000000 - 0x053FFFFF).
 *
 * Revision 1.3  2013-11-07 12:11:43  mgostling
 * Heap size increased to 32MB
 *
 * Revision 1.2  2013-11-07 11:39:17  mgostling
 * Updated reserved memory blocks
 *
 * Revision 1.1  2013-09-04 07:41:29  chippisley
 * Import files into new source repository.
 *
 * Revision 1.10  2011/10/27 15:49:58  hmuneer
 * no message
 *
 * Revision 1.9  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.8  2010/09/15 12:22:07  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.7  2010/06/23 11:00:42  hmuneer
 * CUTE V1.01
 *
 * Revision 1.6  2010/04/23 09:07:11  swilson
 * Add reserved pages used by dynamic heaps. Increase the start of the static heap as
 *  a consequence.
 *
 * Revision 1.5  2008/09/17 15:51:51  swilson
 * Remove use of global variables in IBIT directory - a hang over from the MON2 code
 *  this was based on.
 *
 * Revision 1.4  2008/07/24 14:32:12  swilson
 * semaphore.h moved to private.
 *
 * Revision 1.3  2008/07/18 12:38:59  swilson
 * Add implementation of paged, 36-bit addressing.
 *
 * Revision 1.2  2008/07/02 10:52:46  swilson
 * Enable >4GB memory access via paged memory mechanism (PSE-36).
 *
 * Revision 1.1  2008/05/29 10:09:47  swilson
 * Add local implementation of HAL memory functions. Modify loader to configure the runtime GDT and IDT.
 *
 */


/* includes */

#include <stdtypes.h>
#include <errors.h>

 
#include <bit/board_service.h>

#include <private/cpu.h>
#include <private/semaphore.h>
#include <private/debug.h>


/* defines */

#define _4MB			0x400000

#define MMU_MAX_PAGES	1024		/* 1024*4MB = 4GB */
#define MMU_RESERVED	0xFFFF		/* reserved page */
#define START_OF_HEAP   0x1000000   /*Start address of heap*/
#define END_OF_HEAP		0x3000000   /*End of heap*/


/* typedefs */

/* constants */

/* locals */

static UINT32	dTopOfHeap;
static UINT32	dPhysicalDram;

static UINT16	wPageMap[MMU_MAX_PAGES];

/* globals */

/* externals */

/* forward declarations */

static void vFindFreeBlock (UINT32 dSize, UINT32 dOffset, int* dStart, int* dEnd);
static void vMapPhysToPage (UINT64 qBase, int i);



/*****************************************************************************
 * sysCreateDescriptor: format descriptor parameters for GDT insertion
 *
 * RETURNS: 64-bit encoded descriptor data
 */

UINT64 sysCreateDescriptor
(
	UINT32	dBase,
	UINT32	dLength,
	UINT16	wAttribs
)
{
	UINT64	qDesc = 0;
	UINT32	dLimit;

	/* Convert limit to 4kB granularity */

	dLimit = (dLength - 1) >> 12;
	wAttribs |= DESC_G;

	/* Set the base address */

	qDesc |= (UINT64)(dBase & 0x0000FFFF) << 16 |
			 (UINT64)(dBase & 0x00FF0000) << 16 |
			 (UINT64)(dBase & 0xFF000000) << 32;

	/* Set the limit */

	qDesc |= (UINT64)(dLimit & 0x0000FFFF) |
			 (UINT64)(dLimit & 0x000F0000) << 32;

	/* Set the attributes */

	qDesc |= (UINT64)(wAttribs & 0xF0FF) << 40;

	return (qDesc);

} /* sysCreateDescriptor () */


/*****************************************************************************
 * sysMmuInitialize: remove page entries used by supplied handle
 *
 * RETURNS: none
 */

void sysMmuInitialize (void)
{
	int	i;
	UINT32 temp;

	/* Mark all pages as free */

	for (i = 0; i < MMU_MAX_PAGES; i++)
		wPageMap[i] = 0;

	/* Mark identity mapped pages as reserved */

	wPageMap[0]    = MMU_RESERVED;	/* first 16MB */
	wPageMap[1]    = MMU_RESERVED;
	wPageMap[2]    = MMU_RESERVED;
	wPageMap[3]    = MMU_RESERVED;

	wPageMap[4]    = MMU_RESERVED;	/* next 32MB for malloc heaps */
	wPageMap[5]    = MMU_RESERVED;
	wPageMap[6]    = MMU_RESERVED;
	wPageMap[7]    = MMU_RESERVED;
	wPageMap[8]    = MMU_RESERVED;
	wPageMap[9]    = MMU_RESERVED;
	wPageMap[10]   = MMU_RESERVED;
	wPageMap[11]   = MMU_RESERVED;

	wPageMap[12]   = MMU_RESERVED;	/* next 32MB for system */
	wPageMap[13]   = MMU_RESERVED;
	wPageMap[14]   = MMU_RESERVED;
	wPageMap[15]   = MMU_RESERVED;
	wPageMap[16]   = MMU_RESERVED;	/* 0x04000000 - 0x043FFFFF	Usage:                             */
									/* 0x04000000 - 0x0400FFFF	FreeRTOS Kernel Heap               */
									/* 0x04100000 - 0x042FFFFF	GEIF Driver Descriptor/Buffer Pool */
									/* 0x04300000 - 0x043FFFFF	lwIP Heap                          */
	wPageMap[17]   = MMU_RESERVED;	/*  0x04400000 - 0x047FFFFF	lwIP Memory Pool                   */
	wPageMap[18]   = MMU_RESERVED;
	wPageMap[19]   = MMU_RESERVED;	/*  0x04C00000 - 0x04FFFFFF	Debug Log                          */

	wPageMap[20]   = MMU_RESERVED; /* 0x05000000 - 0x053FFFFF*/
	wPageMap[21]   = MMU_RESERVED;
	wPageMap[22]   = MMU_RESERVED;
	wPageMap[23]   = MMU_RESERVED;
	wPageMap[24]   = MMU_RESERVED;
	wPageMap[1019] = MMU_RESERVED;	/* last 20MB */
	wPageMap[1020] = MMU_RESERVED;
	wPageMap[1021] = MMU_RESERVED;
	wPageMap[1022] = MMU_RESERVED;
	wPageMap[1023] = MMU_RESERVED;

	/* Initialise heap tracking parameters */
	dTopOfHeap    = START_OF_HEAP;					/* starts at 16MB  */
	board_service(SERVICE__BRD_GET_MAX_DRAM_BELOW_4GB, NULL, &temp);
	dPhysicalDram = temp;

} /* sysMmuInitialize () */


/*****************************************************************************
 * sysMmuInitialize: remove page entries used by supplied handle
 *
 * RETURNS: none
 */

void sysMmuFreeAll (void)
{
	int	i;
	UINT32 temp;

	/* Mark all pages as free */

	for (i = 0; i < MMU_MAX_PAGES; i++)
	{
		if(wPageMap[i] != MMU_RESERVED)
			wPageMap[i] = 0;
	}

	dTopOfHeap    = START_OF_HEAP;					/* starts at 16MB  */
	board_service(SERVICE__BRD_GET_MAX_DRAM_BELOW_4GB, NULL, &temp);
	dPhysicalDram = temp;

} /* sysMmuInitialize () */



/*****************************************************************************
 * sysMmuMapBlock: map a 64-bit physical address to a 32-bit logical address
 *
 * The first 16MB and last 20MB are identity mapped, the remainder is 
 * translated through the page directory and tables.
 *
 * The maximum allocation through this mechanism is 256MB, i.e. 64, 4MB pages
 *
 * An array is used to track allocated pages, using the GDT seelctor as a 
 * handle.
 *
 * RETURNS: 32-bit logical address used to access physical block, NULL on error
 */

static SEMAPHORE semaMapBlock;

UINT32 sysMmuMapBlock 
(
	UINT16	wHandle,
	UINT64	qBase,
	UINT32	dSize
)
{
	UINT32	dLogicalAddr;
	UINT32	dOffset4MB;
	int		iStart;
	int		iEnd;
	int		i;
	int 	count;
	#ifdef DEBUG
	UINT8   buffer[64];
	#endif

	/* Return NULL if zero-sized allocation */

	if (dSize == 0)
		return (0);

#ifdef DEBUG
	sprintf(buffer, "sysMmuMapBlock wHandle %x, qBase %x, dSize %x\n", wHandle, (UINT32)qBase, dSize);
	sysDebugWriteString(buffer);
#endif
	sysSemaphoreLock (&semaMapBlock);

	/* Get physical address offset within a 4MB page */

	dOffset4MB = (UINT32)(qBase % _4MB);

	/* Find a suitable free block in the page table */

	vFindFreeBlock (dSize, dOffset4MB, &iStart, &iEnd);

	if (iStart == E__FAIL)
	{
		sysSemaphoreRelease (&semaMapBlock);
		sysDebugWriteString("FAIL block not found!!\n");
		return (NULL);
	}

	/* Reserve this block using the supplied handle and Map the physical address
	 * range into the page-table
	 */
	count = 0;
	for (i = iStart; i <= iEnd; i++)
	{
		wPageMap[i] = wHandle;
		vMapPhysToPage ( (qBase + (_4MB * count)), i);
		count++;
	}
	
	/* Return the logical address (= address of page + offset within 4MB page) */

	dLogicalAddr = (iStart * _4MB) + dOffset4MB;

	sysSemaphoreRelease (&semaMapBlock);
#ifdef DEBUG
	sprintf(buffer, "sysMmuMapBlock iStart %x, dOffset4MB %x, dLogicalAddr %x\n", iStart, dOffset4MB, dLogicalAddr);
		sysDebugWriteString(buffer);
#endif
	return (dLogicalAddr);

} /* sysMmuMapBlock () */



/*****************************************************************************
 * spMemGetPhysPtr: get a pointer to a block of physical memory
 *
 * RETURNS: None
 */

UINT32 sysMemGetPhysPtrLocked
(
		UINT64		qPhysicalAddress,	/* physical address of allocated block */
		UINT32		dBlockSize			/* size of required block */

)
{
	UINT64	qDesc;
	UINT32	dBaseAddr;
	UINT16	wSelector;
	int i;

	#ifdef DEBUG
	UINT8   buffer[64];
	#endif

	/* Find a free slot, if none - return an invalid handle */
#ifdef DEBUG
	sprintf(buffer, "sysMemGetPhysPtrLocked qPhysicalAddress 0x%x_%x, dBlockSize %x\n", (UINT32)(qPhysicalAddress>>32), (UINT32)qPhysicalAddress, dBlockSize);
			sysDebugWriteString(buffer);
#endif

	sysInvalidateTLB ();

	wSelector = sysGdtFindFreeSlotRes ();

	if (wSelector == 0)
	{
		return 0;
	}

	/* Map the request into 4GB address space, then create a descriptor for
	 * the request.
	 */

	dBaseAddr = sysMmuMapBlock (wSelector, qPhysicalAddress, dBlockSize);

	qDesc = sysCreateDescriptor (dBaseAddr, dBlockSize, GDT_TYPE_RW);

	sysGdtWriteSlot (wSelector, qDesc);


	/* Create a pointer to the allocation */

	for (i = 0; i < MMU_MAX_PAGES; i++)
	{
		if (wPageMap[i] == wSelector)
			wPageMap[i] = MMU_RESERVED;
	}

	__asm__ volatile("invlpg %0"::"m" (dBaseAddr));

	return dBaseAddr;

} /* spMemGetPhysPtr () */



/*****************************************************************************
 * sysMmuUnmapBlock: remove page entries used by supplied handle
 *
 * RETURNS: none
 */

void sysMmuUnmapBlock
(
	UINT16	wHandle
)
{
	int		i;


	/* Don't allow unmapping of reserved pages */

	if (wHandle == MMU_RESERVED)
		return;

	/* Traverse the page table releasing pages matching this selector */

	for (i = 0; i < MMU_MAX_PAGES; i++)
	{
		if (wPageMap[i] == wHandle)
			wPageMap[i] = 0;
	}

}/* sysMmuUnmapBlock () */


/*****************************************************************************
 * sysMmuUnmapAll: remove all page entries
 *
 * RETURNS: none
 */

void sysMmuUnmapAll (void)
{
	/* Re-initialize page table etc. */

	sysMmuInitialize ();

} /* sysMmuUnmapAll () */


/*****************************************************************************
 * sysMallocBlock: allocate block of local storage
 *
 * RETURNS: physical address of allocated block, NULL on error
 */

static SEMAPHORE semaMallocBlock;

UINT32 sysMallocBlock
(
	UINT32	dBufferSize
)
{
	UINT32	dAddr;


	/* Return NULL if zero-sized allocation */

	if (dBufferSize == 0)
		return (0);


	sysSemaphoreLock (&semaMallocBlock);

	/* Trivial allocation: virtual, paged memory starts at 16MB, so we simply
	 * allocate memory from that address onwards, aligned to a 4MB page.
	 */

	dAddr = dTopOfHeap; 			/* next allocation starts at top of heap */

	dTopOfHeap += dBufferSize;		/* increment heap pointer */
	dTopOfHeap += _4MB - 1;			/* round up and align to 4MB page */
	dTopOfHeap &= ~(_4MB - 1);


	/* Check allocation does not exceed physical DRAM - also checks for wrap */

	if ((dTopOfHeap > END_OF_HEAP) || (dTopOfHeap < dAddr))
	{
		dTopOfHeap = dAddr;			/* reset highest allocation */
		dAddr = 0;					/* report failed allocation */
	}

	sysSemaphoreRelease (&semaMallocBlock);

	return (dAddr);
	
} /* sysMallocBlock () */


/*****************************************************************************
 * vFindFreeBlock: find a suitable allocation block in the page table
 *
 * RETURNS: (via pointers) index of first and last page, -1 on error
 */

static void vFindFreeBlock 
(
	UINT32	dSize,		/* requested size */
	UINT32	dOffset,	/* start address offset into a 4MB page */
	int*	piStart,	/* first page of allocation */
	int*	piEnd		/* last page of allocation */
)
{
	int	iRequiredNum;	/* how many blocks we require */
	int	iSmallestNum;	/* smallest allocation >= required */
	int iSize;
	int	iStart;
	int	iEnd;



	*piStart = E__FAIL;		/* in case we fail to find a block */

	/* Determine number of 4MB pages required for this allocation. We include
	 * the offset into the first page as part of the size.
	 */

	iRequiredNum = (dSize + dOffset + _4MB - 1) / _4MB;


	/* Find the smallest-size, contiguous page table block that fits this
	 * size requirement
	 */

	iSmallestNum = MMU_MAX_PAGES;

	for (iStart = 0; iStart < MMU_MAX_PAGES; iStart++)
	{
		if (wPageMap[iStart] == 0)
		{
			for (iEnd = iStart + 1; iEnd < MMU_MAX_PAGES; iEnd++)
			{
				if (wPageMap[iEnd] != 0)
					break;
			}

			iSize = iEnd - iStart;

			if ((iSize < iSmallestNum) && (iSize >= iRequiredNum))
			{
				iSmallestNum = iSize;

				*piStart = iStart;
				*piEnd   = iStart + iRequiredNum - 1;
			}

			iStart = iEnd - 1;
		}
	}

} /* iFindFreeBlock () */


/*****************************************************************************
 * vMapPhysToPage: set page table entry
 *
 * RETURNS: none
 */

static void vMapPhysToPage
(
	UINT64	qBase,
	int		i
)
{
	UINT32	dAddr31_22;
	UINT32	dAddr35_32;
	UINT32*	pdPageDir;


	pdPageDir = (UINT32*)0x2000;		/* set pointer to page dir */


	/* Mask for physical address bits[31:22] */

	dAddr31_22 = (UINT32)qBase & 0xFFC00000;

	/* Get physical address bits[35:32], align to bit-13 */

	dAddr35_32 = ((UINT32)(qBase >> 32) & 0x0000000F) << 13;

	/* Combine address fields with G, PS, R/W and P bits set */

	pdPageDir[i] = dAddr31_22 | dAddr35_32 | 0x00000183;	

} /* vMapPhysToPage () */


