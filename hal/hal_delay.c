
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

/* hal_delay.c - BIT calibrated delay functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/hal/hal_delay.c,v 1.1 2013-09-04 07:26:54 chippisley Exp $
 *
 * $Log: hal_delay.c,v $
 * Revision 1.1  2013-09-04 07:26:54  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2009/05/15 10:58:07  jthiru
 * Added SYS_UDELAY for microsec delay
 *
 * Revision 1.1  2008/07/24 14:41:17  swilson
 * HAL interface to system delay functions.
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/hal.h>

#include <private/sys_delay.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * spDelay: HAL interface to sysDelayMilliseconds()
 *
 * RETURNS: None
 */

void spDelay 
(
	UINT32*	pdDelay		/* pointer to delay duration */
)
{
	sysDelayMilliseconds (*pdDelay);

} /* spDelay () */


/*****************************************************************************
 * spUDelay: HAL interface to sysDelayMicroseconds()
 *
 * RETURNS: None
 */

void spUDelay 
(
	UINT32* pdDelay 	/* pointer to delay duration */
)
{
	sysDelayMicroseconds (*pdDelay);

} /* spUDelay () */


