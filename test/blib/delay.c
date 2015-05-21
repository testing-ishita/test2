
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

/* delay.c - BIT calibrated delay
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/delay.c,v 1.2 2013-09-26 12:58:11 chippisley Exp $
 *
 * $Log: delay.c,v $
 * Revision 1.2  2013-09-26 12:58:11  chippisley
 * Removed obsolete HAL.
 *
 * Revision 1.1  2013/09/04 07:11:17  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/07/24 15:08:49  swilson
 * Was passing value to delay function where pointer was expected.
 *
 * Revision 1.1  2008/07/24 14:54:15  swilson
 * Delay, via HAL, for use in BIT code.
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/hal.h>
#include <bit/delay.h>
#include <private/sys_delay.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * vDelay: delay for specified number of milliseconds
 *
 * RETURNS: none
 */

void vDelay
(
	UINT32	dMsDelay
)
{
	sysDelayMilliseconds( dMsDelay );

} /* vDelay () */

