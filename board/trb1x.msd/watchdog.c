
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/trb1x.msd/watchdog.c,v 1.1 2013-11-25 14:21:34 mgostling Exp $
 *
 * $Log: watchdog.c,v $
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011-08-26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
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
										0x2,
										0x213,
										0x10
								};

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
	 vIoWriteReg(localWatchdog.wWdogIoPort,REG_8,(dIoReadReg(localWatchdog.wWdogIoPort, REG_8) & 0x28) );
	 vIoWriteReg(localWatchdog.wWdogIoPort,REG_8,(dIoReadReg(localWatchdog.wWdogIoPort, REG_8) | 0x28) );

	return (E__OK);
} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableReset: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogEnableReset (void *ptr)
{
	vIoWriteReg(localWatchdog.wWdogIoPort,REG_8,(dIoReadReg(localWatchdog.wWdogIoPort, REG_8) | 0x24) );
		
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
	if ( (bSwitch & (localWatchdog.bWdogEnBit)) == 0x00)
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
	*((WATCHDOG_INFO**)ptr) = &localWatchdog;

	return E__OK;
}


