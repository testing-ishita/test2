
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx813.09x/signon.c,v 1.1 2013-11-28 15:13:54 mgostling Exp $
 *
 * $Log: signon.c,v $
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.3  2011/06/08 08:48:05  hchalla
 * Removed extra stray colon for signon bitid display.
 *
 * Revision 1.2  2011/03/22 13:32:49  hchalla
 * Initial Version.
 *
 * Revision 1.1  2011/02/28 11:57:43  hmuneer
 * no message
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <string.h>


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

	 memset(achBuffer,0,sizeof(0));
	/* Additional details: date and build message */
	if (((DISPLAY_SIGNON_INFO*)ptr)->iLevel == FWID_FULL)
	{
		sprintf (achBuffer, "%s  %s",
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



