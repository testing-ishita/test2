
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/signon.c,v 1.1 2015-03-17 10:37:55 hchalla Exp $
 *
 * $Log: signon.c,v $
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
 * Revision 1.1  2013-09-04 07:13:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>


#include <bit/board_service.h>

#include "../tpb1x.msd/version.h"



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
	((FW_VERSION*)ptr)->chXorV	  = FW_X_OR_V;
	((FW_VERSION*)ptr)->iVersion  = FW_VERSION_NUMBER;
	((FW_VERSION*)ptr)->iRevision = FW_REVISION_NUMBER;
	((FW_VERSION*)ptr)->iSpin	  = FW_SPIN_NUMBER;
	((FW_VERSION*)ptr)->iBuild	  = FW_BUILD_NUMBER;

	return E__OK;
} /* brdGetVersionInfo () */



