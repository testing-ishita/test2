#ifndef __mem_h__
	#define __mem_h__

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

/* mem.h - direct memory access functions (mem.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/mem.h,v 1.1 2013-09-04 07:35:27 chippisley Exp $
 *
 * $Log: mem.h,v $
 * Revision 1.1  2013-09-04 07:35:27  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2011/06/07 15:01:24  hchalla
 * Added support for 64-bit memory allocation to test the ram memory aove 4GB. Checked in on behalf of Haroon Muneer.
 *
 * Revision 1.2  2009/02/02 17:10:55  jthiru
 * Prototype updated for dGetPhysPtr
 *
 * Revision 1.1  2008/05/22 16:04:57  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 */


/* includes */

#include <stdtypes.h>
#include <string.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT32 dGetPhysPtr (UINT32 dBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);
extern UINT32 dGetPhysPtr64 (UINT64 qBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);
extern UINT32 dGetBufferPtr (UINT32 dLength, PTR48* pMemory, UINT32* pdBase);
extern void vFreePtr (UINT32 dHandle);


/* forward declarations */


#endif

