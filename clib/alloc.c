
/************************************************************************
 *                                                                      *
 *      Copyright 2010 Concurrent Technologies, all rights reserved.    *
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

/* alloc.c - reusable memory heap management
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/alloc.c,v 1.1 2013-09-04 07:17:57 chippisley Exp $
 *
 * $Log: alloc.c,v $
 * Revision 1.1  2013-09-04 07:17:57  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2011/01/20 10:01:25  hmuneer
 * CA01A151
 *
 * Revision 1.2  2010/06/23 10:51:04  hmuneer
 * CUTE V1.01
 *
 * Revision 1.1  2010/04/23 09:04:54  swilson
 * Dynamic heap management for kernel and user code.
 *
*/


/* includes */

#include <stdtypes.h>
#include <private\k_alloc.h>
#include "FreeRTOS.h"
#include "semphr.h"

/* defines */

#define MH_GRAN			8			/* minimum block allocation size */
#define MH_HEAD_SIZE	4			/* header size */

	/* Header defines */

#define MH_IN_USE		0x00000001	/* 1 = block in use	*/
#define MH_1			0x00000002	/* unused			*/
#define MH_2			0x00000004	/* unused			*/
#define MH_FLAGS_MASK	0x00000007	/*					*/
#define MH_SIZE_MASK	0xFFFFFFF8	/* 8-byte granular	*/

/* typedefs */

typedef struct tagHeapMarker
{
	UINT8*	ptr;
	UINT32	size;

} HEAP_MARKER;

/* constants */

/* locals */

static void* pKernHeap;
static void* pUserHeap;

static HEAP_MARKER	sKernLargestFree;
static HEAP_MARKER	sKernUnusedPool;

static HEAP_MARKER	sUserLargestFree;
static HEAP_MARKER	sUserUnusedPool;

/* globals */

/* forward declarations */

static void* cct_malloc (UINT32 dReqSize, void* pHeap, HEAP_MARKER* psLargestFree, HEAP_MARKER* psUnusedPool);
static void cct_free (void* p, void* pHeap, HEAP_MARKER* psLargestFree, HEAP_MARKER* psUnusedPool);

/* externals */
extern xSemaphoreHandle globalMutexPool[MAX_CUTEBIT_MUTEX];

/*****************************************************************************
 * sysInitMalloc: initialize the memory heap and management structures
 *
 * RETURNS: none
 */
void sysInitMalloc 
(
	void*	pKH,
	void*	pUH
)
{
	/* Kernel Heap */

	pKernHeap = pKH;

	sKernLargestFree.ptr  = NULL;		/* Free pool is empty */
	sKernLargestFree.size = 0;

	sKernUnusedPool.ptr  = pKernHeap;	/* Unused pool is full */
	sKernUnusedPool.size = HEAP_SIZE;

	/* User Heap */

	pUserHeap = pUH;

	sUserLargestFree.ptr  = NULL;		/* Free pool is empty */
	sUserLargestFree.size = 0;

	sUserUnusedPool.ptr  = pUserHeap;	/* Unused pool is full */
	sUserUnusedPool.size = HEAP_SIZE;

} /* sysInitMalloc () */


/*****************************************************************************
 * malloc: allocate a block of memory from the User Heap
 *
 * RETURNS: pointer to the allocated block, NULL on error (or no free)
 */
void* malloc 
(
	UINT32	size
)
{
	void* ptr;

	xSemaphoreTake(globalMutexPool[MUTEX_MALLOC],portMAX_DELAY);
	ptr= cct_malloc (size, pUserHeap, &sUserLargestFree, &sUserUnusedPool);
	xSemaphoreGive(globalMutexPool[MUTEX_MALLOC]);

	return ptr;

} /* malloc () */


/*****************************************************************************
 * free: release User Heap memory block pointed by 'p'
 *
 * RETURNS: none
 */
void free
(
	void* p
)
{
	xSemaphoreTake(globalMutexPool[MUTEX_MALLOC],portMAX_DELAY);
	cct_free (p, pUserHeap, &sUserLargestFree, &sUserUnusedPool);
	xSemaphoreGive(globalMutexPool[MUTEX_MALLOC]);

} /* free () */


/*****************************************************************************
 * free_all: reset the User Heap, freeing all allocations
 *
 * RETURNS: none
 */
void free_all (void)
{
	xSemaphoreTake(globalMutexPool[MUTEX_MALLOC],portMAX_DELAY);
	sUserLargestFree.ptr  = NULL;		/* Free pool is empty */
	sUserLargestFree.size = 0;

	sUserUnusedPool.ptr  = pUserHeap;	/* Unused pool is full */
	sUserUnusedPool.size = HEAP_SIZE;
	xSemaphoreGive(globalMutexPool[MUTEX_MALLOC]);

} /* free_all () */


/*****************************************************************************
 * k_malloc: allocate a block of memory from the Kernel Heap
 *
 * RETURNS: pointer to the allocated block, NULL on error (or no free)
 */
void* k_malloc 
(
	UINT32	size
)
{
	void *ptr;

	xSemaphoreTake(globalMutexPool[MUTEX_MALLOC],portMAX_DELAY);
	ptr = cct_malloc (size, pKernHeap, &sKernLargestFree, &sKernUnusedPool);
	xSemaphoreGive(globalMutexPool[MUTEX_MALLOC]);

	return ptr;

} /* k_malloc () */


/*****************************************************************************
 * k_free: release Kernel Heap memory block pointed by 'p'
 *
 * RETURNS: none
 */
void k_free
(
	void* p
)
{
	xSemaphoreTake(globalMutexPool[MUTEX_MALLOC],portMAX_DELAY);
	cct_free (p, pKernHeap, &sKernLargestFree, &sKernUnusedPool);
	xSemaphoreGive(globalMutexPool[MUTEX_MALLOC]);

} /* free () */


/*****************************************************************************
 * cct_malloc: allocate a block of reusable memory from the specified heap
 *
 * If request is <= largest_free block, then we can reallocate an existing
 * block:
 *
 *	Scan through the whole of the free pool and find:
 *
 *		best_fit allocation (equal to or greater than the request)
 *		next_largest_free block (equal to or less than the largest_free)
 *
 *	If the best_fit block is larger than the request, break that block into
 *	two parts, updating size and flags fields for both
 *
 *	If the largest_free block has been selected, then next_largest_free 
 *	becomes the new largest_free (may be zero if all used)
 *
 * If request is > largest_free block, then we must allocate a block from
 * the unused pool:
 *
 *	Reduce unused pool by the request size
 *
 * RETURNS: pointer to the allocated block, NULL on error (or no free)
 */
static void* cct_malloc
(
	UINT32	dReqSize,
	void*	pHeap,
	HEAP_MARKER* psLargestFree,
	HEAP_MARKER* psUnusedPool
)
{
	HEAP_MARKER sNextLargestFree;
	HEAP_MARKER sBestFit;
	UINT32	size;
	UINT32	flags;
	UINT8*	ptr;
	UINT8*	p;

	/* Initialize the locals */

	sNextLargestFree.ptr  = NULL;
	sNextLargestFree.size = 0;

	sBestFit.ptr  = NULL;
	sBestFit.size = HEAP_SIZE;

	p = NULL;

	/* Check internal parameters are valid before starting */

	if  ((pHeap == NULL) || (psLargestFree == NULL) || (psUnusedPool == NULL))
		return (p);

	/*
	 * Compare request against a few metrics to ensure it is valid, but before
	 * that, adjust request to allow for header and alignment granularity
	 *
	 *		!= 0, <= largest_free or <= unused_free
	 */
	dReqSize = ((dReqSize + 4 + MH_GRAN - 1) / MH_GRAN) * MH_GRAN;

	if (dReqSize == 0)
		return p;
	
	if ( (dReqSize > psLargestFree->size) && (dReqSize > psUnusedPool->size) )
		return p;

	/* In preference we allocate from the free pool first */

	if (dReqSize <= psLargestFree->size)
	{
		ptr = pHeap;

		do {
			size  = MH_SIZE_MASK  & *(UINT32*)ptr;
			flags = MH_FLAGS_MASK & *(UINT32*)ptr;	

			if ((flags & MH_IN_USE) == 0)
			{
				/* Find the best-fit block */

				if ((size >= dReqSize) && (size < sBestFit.size))
				{
					sBestFit.ptr  = ptr;
					sBestFit.size = size;
				}

				/* Find the next largest free block */

				if (ptr != psLargestFree->ptr)
				{
					if (size > sNextLargestFree.size)
					{
						sNextLargestFree.ptr  = ptr;
						sNextLargestFree.size = size;
					}
				}
			}

			ptr = ptr + size;

		} while (ptr < psUnusedPool->ptr);

		/*
		 * If we are using the largest_free block, then next_largest_free becomes 
		 * largest free
		 */
		if (sBestFit.ptr == psLargestFree->ptr)
		{
			psLargestFree->ptr  = sNextLargestFree.ptr;
			psLargestFree->size = sNextLargestFree.size;
		}

		/*
		 * If best_fit allocation is larger than the request, we split it into two
		 * so remainder is free
		 */
		if (sBestFit.size > dReqSize)
		{
			ptr  = sBestFit.ptr  + dReqSize;			 /* start of remainder */
			size = sBestFit.size - dReqSize;			 /* length of remainder */

			*(UINT32*)ptr = size;						 /* create new entry */
		}

		/* Now have the required details, so make the allocation */

		p = sBestFit.ptr + MH_HEAD_SIZE;

		*(UINT32*)sBestFit.ptr = dReqSize | MH_IN_USE;
	}

	else	/* Allocate from the unused pool */
	{
		p = psUnusedPool->ptr + MH_HEAD_SIZE;

		*(UINT32*)psUnusedPool->ptr = dReqSize | MH_IN_USE;

		psUnusedPool->ptr  += dReqSize;
		psUnusedPool->size -= dReqSize;
	}

	return p;

} /* cct_malloc () */


/*****************************************************************************
 * cct_free() release previously allocated memory associated with a pointer
 *
 * Mark the specified block as free
 *
 * Run through the free pool and consolidate connected blocks; any found
 * adjascent to the unused pool should be returned to it
 *
 * During scan, update details for the largest free block, which may change
 *
 * Returns: none
 */
static void cct_free
(
	void*	p,
	void*	pHeap,
	HEAP_MARKER* psLargestFree,
	HEAP_MARKER* psUnusedPool
)
{
	HEAP_MARKER sPrev;
	UINT32	size;
	UINT32	flags;
	UINT8*	ptr;


	psLargestFree->ptr  = NULL;
	psLargestFree->size = 0;	

	sPrev.ptr  = NULL;
	sPrev.size = 0;

	ptr = pHeap;

	/* Check internal parameters are valid before starting */

	if  ((pHeap == NULL) || (psLargestFree == NULL) || (psUnusedPool == NULL))
		return;

	/* Check that pointer is valid */

	if  (p == NULL)
		return;

	/*
	 * Scan through the allocation pool and free the requested block. At the
	 * same time, merge adjacent free blocks.
	 */
	do {
		size  = MH_SIZE_MASK  & *(UINT32*)ptr;
		flags = MH_FLAGS_MASK & *(UINT32*)ptr;	

		/* See if ptr lies within the block, if yes free it */

		if ((flags & MH_IN_USE) != 0)
		{
			if ( ((UINT8*)p >= (ptr + MH_HEAD_SIZE)) && ((UINT8*)p < (ptr + size)) )
			{
				*(UINT32*)ptr = size;		 /* clear all flags */
				flags = 0;
			}
		}

		/* Look at the previous block and see if this one can be merged with it */

		if (sPrev.ptr != NULL)
		{
			if ( ((*(UINT32*)sPrev.ptr & MH_IN_USE) == 0) && 
				 ((flags & MH_IN_USE) == 0) )
			{
				ptr  =  sPrev.ptr;
				size += sPrev.size;

				*(UINT32*)ptr = size;
			}
		}

		/* Update the largest_free block details */

		if ((flags & MH_IN_USE) == 0)
		{
			if (size > psLargestFree->size)
			{
				psLargestFree->ptr  = ptr;
				psLargestFree->size = size;
			}
		}

		/* Update pointers */

		sPrev.ptr  = ptr;
		sPrev.size = size;

		ptr += size;

	} while (ptr < psUnusedPool->ptr);

	/*
	 * If the last block was free, merge it into the unused pool and rescan
	 * for largest_free (if the previous location was part of the merged block)
	 */
	if ((*(UINT32*)sPrev.ptr & MH_IN_USE) == 0)
	{
		psUnusedPool->ptr   = sPrev.ptr;
		psUnusedPool->size += sPrev.size;

		/* After the merge, we must re-scan the free pool to find largest_free */

		psLargestFree->ptr  = NULL;
		psLargestFree->size = 0;	

		ptr = pHeap;

		while (ptr < psUnusedPool->ptr)	 /* test first here as may have no free pool */
		{
			size  = MH_SIZE_MASK  & *(UINT32*)ptr;
			flags = MH_FLAGS_MASK & *(UINT32*)ptr;	

			if ( ((flags & MH_IN_USE) == 0) && (size > psLargestFree->size) )
			{
				psLargestFree->ptr  = ptr;
				psLargestFree->size = size;
			}

			ptr += size;
		} 
	}

}  /* cct_free () */
