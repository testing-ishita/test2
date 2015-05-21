
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

/* signon.c - display BIT firmware version and build details.
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/signon.c,v 1.1 2015-02-25 17:50:45 hchalla Exp $
 *
 * $Log: signon.c,v $
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.1  2012/06/22 11:42:01  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>


#include <bit/board_service.h>

#include "VERSION.H"



/*****************************************************************************
 * brdDisplaySignon: display version information for the installed firmware 
 *
 * RETURNS: none
 */
UINT32 brdDisplaySignon(void *ptr)
{
	char	achBuffer[80];


	/* Display: board-type, F/W revision and build number */
	if(((DISPLAY_SIGNON_INFO*)ptr)->mode == FWMODE_BIT)
	{
		sprintf (achBuffer, "%s %s, %s",
				 BOARD_TYPE_STRING,
				 FW_REVISION_STRING,
				 FW_BUILD_STRING);
		puts (achBuffer);
	}
	else
	{
		sprintf (achBuffer, "%s %s, %s",
				 BOARD_CUTE_STRING,
				 FW_REVISION_STRING,
				 FW_BUILD_STRING);
		puts (achBuffer);

	}

	/* Additional details: date and build message */
	if (((DISPLAY_SIGNON_INFO*)ptr)->iLevel == FWID_FULL)
	{
		sprintf (achBuffer, "%s : %s", 
					TIME_DATE_STRING,
					FW_MESSAGE_STRING);
		puts (achBuffer);
	}

	return E__OK;

} /* brdDisplaySignon () */


/*****************************************************************************
 * brdGetVersionInfo: return F/W version and build
 *
 * RETURNS: none
 */
UINT32 brdGetVersionInfo(void *ptr)
{
	//((FW_VERSION*)ptr)->chXorV	  = FW_X_OR_V;
	((FW_VERSION*)ptr)->iVersion  = FW_VERSION_NUMBER;
	((FW_VERSION*)ptr)->iRevision = FW_REVISION_NUMBER;
	((FW_VERSION*)ptr)->iSpin	  = FW_SPIN_NUMBER;
	((FW_VERSION*)ptr)->iBuild	  = FW_BUILD_NUMBER;

	return E__OK;
} /* brdGetVersionInfo () */


