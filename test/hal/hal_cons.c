
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

/* hal_cons.c - BIT interface to the console device
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/hal/hal_cons.c,v 1.1 2013-09-04 07:26:54 chippisley Exp $
 *
 * $Log: hal_cons.c,v $
 * Revision 1.1  2013-09-04 07:26:54  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>

#include <bit/hal.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * spConsMessage: HAL interface to cputs()
 *
 * RETURNS: None
 */

void spConsMessage 
(
	char*	psParams	/* pointer to NULL terminated string */
)
{
	cputs (psParams);

} /* spConsMessage () */



