#ifndef __delay_h__
	#define __delay_h__

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

/* delay.h - BIT calibrated delay function (delay.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/delay.h,v 1.1 2013-09-04 07:35:26 chippisley Exp $
 *
 * $Log: delay.h,v $
 * Revision 1.1  2013-09-04 07:35:26  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/07/24 14:51:26  swilson
 * Convert to HAL delay function, instead of system level delay, which has moved to be kernel private.
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

extern void vDelay (UINT32 dMsDelay);


/* forward declarations */


#endif

