#ifndef __sys_delay_h__
	#define __sys_delay_h__

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

/* delay.h - calibrated delay functions (delay.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/sys_delay.h,v 1.1 2013-09-04 07:40:42 chippisley Exp $
 *
 * $Log: sys_delay.h,v $
 * Revision 1.1  2013-09-04 07:40:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/09/26 13:15:03  swilson
 * Add function for reading the TSC.
 *
 * Revision 1.1  2008/07/24 14:56:08  swilson
 * System-level functions moved here from include\bit, as they shouldn't be used there.
 *
 * Revision 1.1  2008/07/18 12:47:54  swilson
 * Calibrated delay functions.
 *
 */


/* includes */

#include <stdtypes.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT32 sysGetCpuFrequency (void);

extern void sysDelayMilliseconds (UINT32 dMsDelay);
extern void sysDelayMicroseconds (UINT32 dUsDelay);

extern UINT64 sysGetTimestamp (void);


/* forward declarations */


#endif

