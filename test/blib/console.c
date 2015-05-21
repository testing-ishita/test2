
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

/* console.c - BIT console I/O functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/console.c,v 1.2 2013-09-26 12:58:11 chippisley Exp $
 *
 * $Log: console.c,v $
 * Revision 1.2  2013-09-26 12:58:11  chippisley
 * Removed obsolete HAL.
 *
 * Revision 1.1  2013/09/04 07:11:17  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/01/11 17:24:36  hchalla
 * New board service function added to the code base and new board VX813.
 *
 * Revision 1.4  2010/06/23 10:47:28  hmuneer
 * CUTE V1.01
 *
 * Revision 1.3  2008/07/24 14:30:01  swilson
 * semaphore.h moved to private.
 *
 * Revision 1.2  2008/07/18 12:18:40  swilson
 * Add semaphore access control.
 *
 * Revision 1.1  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/hal.h>
#include <bit/console.h>

#include "FreeRTOS.h"
#include "semphr.h"


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */
extern xSemaphoreHandle globalMutexPool[MAX_CUTEBIT_MUTEX];

/* forward declarations */


/*****************************************************************************
 * vConsoleWrite: write a NULL terminated string to the console device
 *
 * NOTE: this function does not automatically append a newline character.
 *
 * RETURNS: none
 */

void vConsoleWrite
(
	char*	achMessage
)
{
	xSemaphoreTake(globalMutexPool[MUTEX_CONSOLE_PRINT],portMAX_DELAY);

	cputs( achMessage );

	xSemaphoreGive(globalMutexPool[MUTEX_CONSOLE_PRINT]);

} /* vConsoleWrite () */

