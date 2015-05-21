
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

/* hal_mem.c - BIT access to physical and virtual memory
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/hal/hal_mem.c,v 1.1 2013-09-04 07:26:55 chippisley Exp $
 *
 * $Log: hal_mem.c,v $
 * Revision 1.1  2013-09-04 07:26:55  chippisley
 * Import files into new source repository.
 *
 * Revision 1.6  2011/01/20 10:01:25  hmuneer
 * CA01A151
 *
 * Revision 1.5  2010/09/15 12:16:13  hchalla
 * Implemented new API for locking the memory after allocation.
 *
 * Revision 1.4  2009/02/09 17:36:28  jthiru
 * Added logical address to member of structure pointer of CCT_MEM_PTR
 *
 * Revision 1.3  2008/07/18 12:22:04  swilson
 * Revise parameters to sysMmuMapBlock and sysMmuUnmapBlock.
 *
 * Revision 1.2  2008/05/29 10:09:47  swilson
 * Add local implementation of HAL memory functions. Modify loader to configure the runtime GDT and IDT.
 *
 */


/* includes */

#include <stdtypes.h>
#include <string.h>

#include <bit/hal.h>

#include <private/cpu.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * spMemGetPhysPtr: get a pointer to a block of physical memory
 *
 * RETURNS: None
 */

void spMemGetPhysPtr
(
	CCT_MEM_PTR*	psParams
)
{
	UINT64	qDesc;
	UINT32	dBaseAddr;
	UINT16	wSelector;
#ifdef DEBUG
	UINT8   buffer[64];
#endif

	/* Find a free slot, if none - return an invalid handle */
#ifdef DEBUG
	sprintf(buffer, "spMemGetPhysPtr qPhysicalAddress %x, dBlockSize %x\n", (UINT32)psParams->qPhysicalAddress, psParams->dBlockSize);
		sysDebugWriteString(buffer);
#endif

	wSelector = sysGdtFindFreeSlot ();

	if (wSelector == 0)		
	{
		psParams->pBlock = NULL;			/* pointer = NULL */
		psParams->dHandle = (UINT32)-1;		/* return an invalid handle */

		return;
	}


	/* Map the request into 4GB address space, then create a descriptor for 
	 * the request.
	 */

	dBaseAddr = sysMmuMapBlock (wSelector, psParams->qPhysicalAddress, psParams->dBlockSize);

	if(dBaseAddr != NULL)
	{
		qDesc = sysCreateDescriptor (dBaseAddr, psParams->dBlockSize, GDT_TYPE_RW);

		sysGdtWriteSlot (wSelector, qDesc);

		/* Create a pointer to the allocation */

		psParams->pBlock = MK_PTR48 (wSelector, 0);
		psParams->dHandle = (UINT32)wSelector;
		psParams->dlogaddr = dBaseAddr;
	}
	else
	{
		psParams->pBlock = NULL;			/* pointer = NULL */
		psParams->dHandle = (UINT32)-1;		/* return an invalid handle */
	}

} /* spMemGetPhysPtr () */


/*****************************************************************************
 * spMemGetBuffer: get a pointer to a block memory to use as a buffer
 *
 * RETURNS: None
 */

void spMemGetBuffer
(
	CCT_MEM_PTR*	psParams
)
{
	/* Get a block of memory for use as a buffer */

	psParams->qPhysicalAddress = (UINT64)sysMallocBlock (psParams->dBlockSize);


	/* If unable to allocate - return an invalid handle */

	if (psParams->qPhysicalAddress == NULL)
	{
		psParams->pBlock = NULL;			/* pointer = NULL */
		psParams->dHandle = (UINT32)-1;		/* return an invalid handle */

		return;
	}


	/* Now allocate this as if it were a physical block */

	spMemGetPhysPtr (psParams);

} /* spMemGetBuffer () */


/*****************************************************************************
 * spMemFreeMem: free an allocated block for re-use
 *
 * RETURNS: None
 */

void spMemFreeMem
(
	CCT_MEM_PTR*	psParams
)
{
	sysMmuUnmapBlock ((UINT16)psParams->dHandle);
	sysGdtWriteSlot ((UINT16)psParams->dHandle, 0);

} /* spMemFreeMem () */


/*****************************************************************************
 * spMemFreeAll: free all previous memory allocations
 *
 * RETURNS: None
 */

void spMemFreeAll
(
	CCT_MEM_PTR*	psParams
)
{
	sysMmuFreeAll ();
	sysGdtDeleteAllSlots ();

} /* spMemFreeAll () */



