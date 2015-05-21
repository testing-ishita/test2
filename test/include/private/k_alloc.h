#ifndef __k_alloc_h__
	#define __k_alloc_h__

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
 *																		*
 ************************************************************************/

/* k_alloc.h - kernel interfaces to malloc() and free()
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/k_alloc.h,v 1.1 2013-09-04 07:40:42 chippisley Exp $
 *
 * $Log: k_alloc.h,v $
 * Revision 1.1  2013-09-04 07:40:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2010/06/23 11:00:13  hmuneer
 * CUTE V1.01
 *
 * Revision 1.1  2010/04/23 09:03:54  swilson
 * Include file for kernel interface to clib\alloc.c
 *
 */
 

/* includes */

/* defines */

#define HEAP_SIZE		0x800000	// size of each heap

#define K_HEAP			(void*)(0x01000000)
#define U_HEAP			(void*)(0x01800000)

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern void sysInitMalloc (void* pKH, void* pUH);

extern void* k_malloc (UINT32 size);
extern void k_free (void* p);

/* forward declarations */


#endif


