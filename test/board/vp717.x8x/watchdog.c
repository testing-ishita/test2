
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

/* watchdog.c - board-specific watchdog functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp717.x8x/watchdog.c,v 1.1 2015-04-02 11:41:18 mgostling Exp $
 *
 * $Log: watchdog.c,v $
 * Revision 1.1  2015-04-02 11:41:18  mgostling
 * Initial check-in to CVS
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.4  2011/05/11 15:25:07  hchalla
 * Added new service to get watchdog info.
 *
 * Revision 1.3  2011/03/22 13:36:46  hchalla
 * Added board services for watchdog.
 *
 * Revision 1.2  2011/01/20 10:01:23  hmuneer
 * CA01A151
 *
 * Revision 1.1  2010/06/23 10:49:08  hmuneer
 * CUTE V1.01
 *
 * Revision 1.1  2010/01/19 12:03:15  hmuneer
 * vx511 v1.0
 *
 * Revision 1.1  2009/09/29 12:16:45  swilson
 * Create VP417 Board
 *
 * Revision 1.2  2009/05/19 08:09:53  cvsuser
 * no message
 *
 * Revision 1.1  2008/09/19 14:55:13  swilson
 * Template for board-specific watchdog support.
 *
 */


/* includes */

#include <stdtypes.h>
#include <bit/board_service.h>
#include <bit/io.h>
#include <private/port_io.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

//static UINT8	dPattingToken;

/* globals */

/* externals */

/* forward declarations */


WATCHDOG_INFO  localWatchdog = {
									0x7,
									0x211,
									0x08,
									SIO
								};

/*****************************************************************************
 * brdWatchdogEnable: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogEnable (void *ptr)
{
	return (E__OK);
} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableNmi: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogEnableNmi (void *ptr)
{
	return (E__OK);
} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableReset: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogEnableReset (void *ptr)
{
		vIoWriteReg(localWatchdog.wWdogIoPort,REG_8,(dIoReadReg(localWatchdog.wWdogIoPort, REG_8) | 0x04) );
		return (E__OK);
} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableReset: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogCheckSwitch (void *ptr)
{
	UINT8 bSwitch;

	bSwitch = dIoReadReg(localWatchdog.wWdogIoPort, REG_8);
	if ( (bSwitch & (localWatchdog.bWdogEnBit)) == (localWatchdog.bWdogEnBit) )
	{
		return (E__OK);
	}
	else
	{
		return (E__FAIL);
	}
} /* brdWatchdogEnable () */


/*****************************************************************************
 * brdWatchdogDisable: disable the watchdog to prevent timeout 
 *
 * RETURNS: none
 */

UINT32 brdWatchdogDisable (void *ptr)
{
	return (E__OK);
} /* brdWatchdogDisable () */


/*****************************************************************************
 * brdWatchdogPat: pat the watchdog to prevent timeout while enabled
 *
 * RETURNS: none
 */

UINT32 brdWatchdogPat (void *ptr)
{
	return (E__OK);
} /* brdWatchdogPat () */


/*****************************************************************************
 * brdGetWdogInfo: returns the WATCHDOG_INFO global data structure
 * RETURNS: WATCHDOG_INFO* */
UINT32 brdGetWdogInfo(void *ptr)
{
	*((WATCHDOG_INFO**)ptr) = &localWatchdog;

	return E__OK;
}
