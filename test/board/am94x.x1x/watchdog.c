
/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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

/* watchdog.c - board-specific watchdog functions
 */

/* includes */

#include <stdtypes.h>
#include <bit/board_service.h>
#include <private/port_io.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

//static UINT8	dPattingToken;

/* globals */

/* externals */

/* forward declarations */

/*****************************************************************************
 * brdWatchdogEnable: enable the board's watchdog function
 *
 * RETURNS: none
 */

void brdWatchdogEnable (void)
{

} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableNmi: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogEnableNmi (void *ptr)
{

	return E__OK;

} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableReset: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogEnableReset (void *ptr)
{

	return E__OK;

} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableReset: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogCheckSwitch (void *ptr)
{

	return E__OK;
} /* brdWatchdogEnable () */


/*****************************************************************************
 * brdWatchdogDisable: disable the watchdog to prevent timeout
 *
 * RETURNS: none
 */

void brdWatchdogDisable (void)
{

} /* brdWatchdogDisable () */


/*****************************************************************************
 * brdWatchdogPat: pat the watchdog to prevent timeout while enabled
 *
 * RETURNS: none
 */

void brdWatchdogPat (void)
{

} /* brdWatchdogPat () */


/*****************************************************************************
 * brdGetWdogInfo: returns the WATCHDOG_INFO global data structure
 * RETURNS: WATCHDOG_INFO* */
UINT32 brdGetWdogInfo(void *ptr)
{

	return E__OK;
}


