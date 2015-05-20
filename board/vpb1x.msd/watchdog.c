
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/watchdog.c,v 1.2 2015-03-17 10:41:49 hchalla Exp $
 *
 * $Log: watchdog.c,v $
 * Revision 1.2  2015-03-17 10:41:49  hchalla
 * Changed watchdog timeout to 100milliseconds from 10 seconds due to FPGA change.
 *
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.2  2012/08/07 15:16:23  madhukn
 * *** empty log message ***
 *
 * Revision 1.1  2012/06/22 11:42:02  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
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


static WATCHDOG_INFO  localWatchdog = {
										2,
										0x22B,
										0x80,
										CPLD_FPGA
								};

/*****************************************************************************
 * brdWatchdogEnable: enable the board's watchdog function
 *
 * RETURNS: none
 */
UINT32 brdWatchdogEnable (void *ptr)
{
	vIoWriteReg (localWatchdog.wWdogIoPort,REG_8,(dIoReadReg(localWatchdog.wWdogIoPort, REG_8) & ~(localWatchdog.bWdogEnBit) ));
	return E__OK;
} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableNmi: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogEnableNmi (void *ptr)
{
	//vIoWriteReg(localWatchdog.wWdogIoPort,REG_8,(dIoReadReg(localWatchdog.wWdogIoPort, REG_8) & 0x28) );
	//vIoWriteReg(localWatchdog.wWdogIoPort,REG_8,(dIoReadReg(localWatchdog.wWdogIoPort, REG_8) | 0x28) );
	return E__OK;
} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableReset: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogEnableReset (void *ptr)
{
	vIoWriteReg(0x211, REG_8, (dIoReadReg(0x211, REG_8) | 0x04));
	return E__OK;
} /* brdWatchdogEnable () */

/*****************************************************************************
 * brdWatchdogEnableReset: enable the board's watchdog function
 *
 * RETURNS: none
 */

UINT32 brdWatchdogCheckSwitch (void *ptr)
{
	UINT8 bSwitch;
	bSwitch = dIoReadReg(0x211, REG_8);
	if ( (bSwitch & (0x08)) != 0)
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
	vIoWriteReg (localWatchdog.wWdogIoPort,REG_8,(dIoReadReg(localWatchdog.wWdogIoPort, REG_8) | (localWatchdog.bWdogEnBit)));
	return E__OK;
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


