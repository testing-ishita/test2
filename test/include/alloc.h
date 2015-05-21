#ifndef __alloc_h__
	#define __alloc_h__

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

/* alloc.h - include file for alloc.c
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/alloc.h,v 1.1 2013-09-04 07:28:50 chippisley Exp $
 *
 * $Log: alloc.h,v $
 * Revision 1.1  2013-09-04 07:28:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2010/04/23 09:03:06  swilson
 * Include file for clib\alloc.c
 *
 */
 

/* includes */

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern void* malloc (UINT32 size);
extern void free (void* p);
extern void free_all (void);

/* forward declarations */


#endif


