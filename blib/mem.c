
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

/* mem.c - BIT direct memory access functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/mem.c,v 1.2 2013-09-26 12:58:11 chippisley Exp $
 *
 * $Log: mem.c,v $
 * Revision 1.2  2013-09-26 12:58:11  chippisley
 * Removed obsolete HAL.
 *
 * Revision 1.1  2013/09/04 07:11:17  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2011/10/27 15:47:12  hmuneer
 * no message
 *
 * Revision 1.7  2011/06/07 15:02:56  hchalla
 *  Added support for 64-bit memory allocation to test the ram memory aove 4GB. Checked in on behalf of Haroon Muneer.
 *
 * Revision 1.6  2011/01/11 17:24:36  hchalla
 * New board service function added to the code base and new board VX813.
 *
 * Revision 1.5  2010/09/15 12:06:43  hchalla
 * Added Debug print.
 *
 * Revision 1.4  2010/06/23 10:47:28  hmuneer
 * CUTE V1.01
 *
 * Revision 1.3  2009/02/02 17:10:03  jthiru
 * Added parameter dlogaddr to dGetPhysPtr to access logical address, invalidate TLB page before accessing logical mem.
 *
 * Revision 1.2  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 * Revision 1.1  2008/05/21 13:27:54  swilson
 * Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/hal.h>
#include <bit/mem.h>
#include <private/cpu.h>

#include "config.h"
#include "../debug/dbgLog.h"

/* defines */

#undef INCLUDE_MEM_DEBUG

#ifdef INCLUDE_MEM_DEBUG
#warning "***** DEBUG ON *****"
#endif

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * dGetPhysPtr: get a pointer to a block of physical memory below 4GB
 *
 * RETURNS: handle used to identify the allocation, E__FAIL on error
 */

UINT32 dGetPhysPtr
(
	UINT32		dBase,		/* base address */
	UINT32		dLength,	/* buffer length in bytes */
	PTR48*		pMemory,	/* pointer to the allocated block */
	void*		dlogaddr	/* Logical address to access memory contents */
)
{
	UINT64	qDesc;
	UINT32	dBaseAddr;
	UINT16	wSelector;
	UINT32	dHandle;


#ifdef INCLUDE_MEM_DEBUG
	DBLOG( "%s: dBase 0x%x, dLength 0x%x\n", __FUNCTION__, dBase, dLength );
#endif

	sysInvalidateTLB ();

	wSelector = sysGdtFindFreeSlot();

	if (wSelector == 0)		
	{
		dBaseAddr = 0;
		*pMemory = NULL;		/* pointer = NULL */
		dHandle = E__FAIL;		/* return an invalid handle */
	}
	else
	{
		/* Map the request into 4GB address space, then create a descriptor for 
		 * the request.
		 */
	
		dBaseAddr = sysMmuMapBlock( wSelector, (UINT64)dBase, dLength );
	
		if(dBaseAddr != NULL)
		{
			qDesc = sysCreateDescriptor( dBaseAddr, dLength, GDT_TYPE_RW );
	
			sysGdtWriteSlot( wSelector, qDesc );
			
			/* Create a pointer to the allocation */
			
			*pMemory = MK_PTR48( wSelector, 0 );
	
			/* Obtain the logical address containing the physical address contents */
	
			*(UINT32*)dlogaddr = dBaseAddr;
			
			dHandle = (UINT32) wSelector;
			
			/* Invalidate the page table entry in TLB containing the logical address 
			 * we just obtained 
			 */
			
			__asm__ volatile("invlpg %0"::"m" (dlogaddr));
		}
		else
		{
			*pMemory = NULL;		/* pointer = NULL */
			dHandle = E__FAIL;		/* return an invalid handle */
		}
	}

#ifdef INCLUDE_MEM_DEBUG	
	DBLOG( "%s: dBaseAddr 0x%x, dHandle 0x%x\n", __FUNCTION__, dBaseAddr, dHandle );
#endif

	return dHandle;

} /* dGetPhysPtr () */


/*****************************************************************************
 * dGetPhysPtr64: get a pointer to a block of physical memory beyond 4GB
 *
 * RETURNS: handle used to identify the allocation, E__FAIL on error
 */

UINT32 dGetPhysPtr64
(
	UINT64		qBase,		/* base address */
	UINT32		dLength,	/* buffer length in bytes */
	PTR48*		pMemory,	/* pointer to the allocated block */
	void*		dlogaddr	/* Logical address to access memory contents */
)
{
	UINT64	qDesc;
	UINT32	dBaseAddr;
	UINT16	wSelector;
	UINT32	dHandle;


	sysInvalidateTLB ();

	wSelector = sysGdtFindFreeSlot();

	if (wSelector == 0)		
	{
		dBaseAddr = 0;
		*pMemory = NULL;		/* pointer = NULL */
		dHandle = E__FAIL;		/* return an invalid handle */
	}
	else
	{
		/* Map the request into 4GB address space, then create a descriptor for 
		 * the request.
		 */
	
		dBaseAddr = sysMmuMapBlock( wSelector, qBase, dLength );
	
		if(dBaseAddr != NULL)
		{
			qDesc = sysCreateDescriptor( dBaseAddr, dLength, GDT_TYPE_RW );
	
			sysGdtWriteSlot( wSelector, qDesc );
			
			/* Create a pointer to the allocation */
			
			*pMemory = MK_PTR48( wSelector, 0 );
	
			/* Obtain the logical address containing the physical address contents */
	
			*(UINT32*)dlogaddr = dBaseAddr;
			
			dHandle = (UINT32) wSelector;
			
			/* Invalidate the page table entry in TLB containing the logical address 
			 * we just obtained 
			 */
			
			__asm__ volatile("invlpg %0"::"m" (dlogaddr));
		}
		else
		{
			*pMemory = NULL;		/* pointer = NULL */
			dHandle = E__FAIL;		/* return an invalid handle */
		}
	}
	
	return dHandle;

} /* dGetPhysPtr64 () */


/*****************************************************************************
 * dGetBufferPtr: get a pointer to a block of memory
 *
 * RETURNS: handle used to identify the allocation, E__FAIL on error
 */

UINT32 dGetBufferPtr
(
	UINT32		dLength,	/* buffer length in bytes */
	PTR48*		pMemory,	/* pointer to the allocated block */
	UINT32*		pdBase		/* physical base address of block */
)
{
	UINT64	qDesc;
	UINT32	dBaseAddr;
	UINT16	wSelector;
	UINT32	dHandle;
	UINT64 qPhysicalAddress;
	
	
	/* Allocate memory */
	qPhysicalAddress = (UINT64) sysMallocBlock( dLength );
	
	/* If unable to allocate - return an invalid handle */

	if (qPhysicalAddress == NULL)
	{
		*pMemory = NULL;		/* pointer = NULL */
		*pdBase = NULL;
		dHandle = E__FAIL;		/* return an invalid handle */
		
	}
	else
	{
		wSelector = sysGdtFindFreeSlot();

		if (wSelector == 0)		
		{
			*pMemory = NULL;		/* pointer = NULL */
			*pdBase = NULL;
			dHandle = E__FAIL;		/* return an invalid handle */
		}
		else
		{
			/* Map the request into 4GB address space, then create a descriptor for 
			 * the request.
			 */
		
			dBaseAddr = sysMmuMapBlock( wSelector, qPhysicalAddress, dLength );
		
			if(dBaseAddr != NULL)
			{
				qDesc = sysCreateDescriptor( dBaseAddr, dLength, GDT_TYPE_RW );
		
				sysGdtWriteSlot( wSelector, qDesc );
				
				/* Create a pointer to the allocation */
				
				*pMemory = MK_PTR48( wSelector, 0 );
		
				/* Obtain the physical address */
		
				*pdBase = (UINT32) qPhysicalAddress;
				
				dHandle = (UINT32) wSelector;
			}
			else
			{
				*pMemory = NULL;		/* pointer = NULL */
				dHandle = E__FAIL;		/* return an invalid handle */
			}
		}
	}
	
#ifdef INCLUDE_MEM_DEBUG
	DBLOG( "%s: pdBase 0x%x, dLength 0x%x, dHandle 0x%x\n", __FUNCTION__, *pdBase, dLength, dHandle );
#endif

	return dHandle;
		
} /* dGetBufferPtr () */


/*****************************************************************************
 * vFreePtr: release a memory pointer (frees the allocation slot)
 *
 * Note: the BIT handler will free all pointers on completion of the test.
 *
 * RETURNS: E__OK or E__FAIL
 */

void vFreePtr
(
	UINT32		dHandle		/* identifier */
)
{

#ifdef INCLUDE_MEM_DEBUG
	DBLOG( "%s: dHandle 0x%x\n", __FUNCTION__, dHandle );
#endif
	
	sysMmuUnmapBlock ((UINT16)dHandle);
	sysGdtWriteSlot ((UINT16)dHandle, 0);

} /* vFreePtr () */

