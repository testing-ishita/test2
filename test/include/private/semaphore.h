#ifndef __semaphore_h__
	#define __semaphore_h__

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

/* semaphore.h - access control semaphore support (semaphore.S)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/semaphore.h,v 1.1 2013-09-04 07:40:42 chippisley Exp $
 *
 * $Log: semaphore.h,v $
 * Revision 1.1  2013-09-04 07:40:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2008/07/24 14:56:08  swilson
 * System-level functions moved here from include\bit, as they shouldn't be used there.
 *
 * Revision 1.1  2008/07/18 12:47:04  swilson
 * Semaphore support.
 *
 */


/* includes */

#include <stdtypes.h>


/* defines */

/* typedefs */

typedef UINT32		SEMAPHORE;


/* constants */

/* locals */

/* globals */

/* externals */

extern void sysSemaphoreLock (SEMAPHORE* s);
extern void sysSemaphoreRelease (SEMAPHORE* s);
extern int sysSemaphoreTest (SEMAPHORE* s);


/* forward declarations */


#endif

