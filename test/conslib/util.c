
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

/*
 * UTIL: implementation of simple utility commands
 */

/* includes */

#include <stdio.h>
#include <errors.h>

#include <private/port_io.h>
#include <private/cpu.h>


#include <bit/board_service.h>
#include <bit/conslib.h>

/* defines */

/* typedefs */

/* constants */

/* locals */



static int iRadix;				/* default utility parameter radix */
static int iRepeatIndex;		/* repeated command tracking */
static int iConfirmFlag;		/* write confirm mode */
static int iBitErrorAction;		/* halt or continue on error */

/* globals */

/* externals */

/* forward declarations */



/***************************************************************************
 * vMTHReset: Reset the board
 *
 * RETURNS: none
 */

void vReset (void)
{
	puts ("Resetting, please wait...");

	board_service(SERVICE__BRD_RESET, NULL, NULL);;

	puts ("Board reset failed!!!");

} /* vReset () */


/***************************************************************************
 * vBIOS: Reset the board
 *
 * RETURNS: none
 */
void vBIOS (void)
{
	NV_RW_Data	nvdata;

	puts ("Jumping To BIOS, please wait...");

	nvdata.dNvToken = NV__BIOS_BOOT;
	nvdata.dData    = 1;
	board_service(SERVICE__BRD_NV_WRITE, NULL, &nvdata);
	board_service(SERVICE__BRD_RESET, NULL, NULL);

	puts ("Board reset failed!!!");

} /* vBIOS () */


/***************************************************************************
 * vMTHRadix2: set default radix to binary
 *
 * RETURNS: none
 */

void vRadix2 (void)
{
	puts ("Radix: BINARY");

	iRadix = 2;

} /* vRadix2 () */


/***************************************************************************
 * vRadix10: set default radix to decimal
 *
 * RETURNS: none
 */

void vRadix10 (void)
{
	puts ("Radix: DECIMAL");

	iRadix = 10;

} /* vRadix10 () */


/***************************************************************************
 * vRadix16: set default radix to hexadecimal
 *
 * RETURNS: none
 */

void vRadix16 (void)
{
	puts ("Radix: HEXADECIMAL");

	iRadix = 16;

} /* vRadix16 () */


/***************************************************************************
 * iGetRadix: retrieve the current radix seelction
 *
 * RETURNS: radix (currently: 2, 10 or 16)
 */

int iGetRadix (void)
{
	return (iRadix);

} /* iGetRadix () */


/***************************************************************************
 * vConfirmOn: turn on write confirmation
 *
 * RETURNS: none
 */

void vConfirmOn (void)
{
	puts ("Write Confirm: ON");

	iConfirmFlag = 1;

} /* vConfrmOn () */


/***************************************************************************
 * vConfirmOff: turn off write confirmation
 *
 * RETURNS: none
 */

void vConfirmOff (void)
{
	puts ("Write Confirm: OFF");

	iConfirmFlag = 0;

} /* vConfirmOff () */


/***************************************************************************
 * iGetConfirmFlag: retrieve the confirm flag
 *
 * RETURNS: repeat index
 */

int iGetConfirmFlag (void)
{
	return (iConfirmFlag);

} /* iGetConfirmFlag () */


/***************************************************************************
 * vSetRepeatIndex: set the repeat index value
 *
 * RETURNS: none
 */

void vSetRepeatIndex
(
	int i
)
{
	iRepeatIndex = i;

} /* iSetRepeatIndex () */


/***************************************************************************
 * iGetRepeatIndex: retrieve the repeat index value
 *
 * RETURNS: repeat index
 */

int iGetRepeatIndex (void)
{
	return (iRepeatIndex);

} /* iGetRepeatIndex () */


/***************************************************************************
 * vCacheOn: turn on caching
 *
 * RETURNS: none
 */

void vCacheOn (void)
{
	sysCacheEnable ();

	puts ("Caching: ENABLED");

} /* vCacheOn () */


/***************************************************************************
 * vCacheOff: turn off caching
 *
 * RETURNS: none
 */

void vCacheOff (void)
{
	sysCacheDisable ();

	puts ("Caching: DISABLED");

} /* vCacheOff () */


/***************************************************************************
 * vBeaHalt: halt testing if error reported
 *
 * RETURNS: none
 */

void vBeaHalt (void)
{
	iBitErrorAction = BEA_HALT;

	puts ("Testing will HALT if a failure is generated");

} /* vBeaHalt () */


/***************************************************************************
 * vBeaContinue: continue testing if error reported
 *
 * RETURNS: none
 */

void vBeaContinue (void)
{
	iBitErrorAction = BEA_CONTINUE;

	puts ("Testing will CONTINUE if failures are generated");

} /* vBeaContinue () */


/***************************************************************************
 * iGetBeaFlag: retrieve the BIT Error Action
 *
 * RETURNS: BEA_HALT or BEA_CONTINUE
 */

int iGetBeaFlag (void)
{
	return (iBitErrorAction);

} /* iGetBeaFlag () */





