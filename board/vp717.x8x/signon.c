
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp717.x8x/signon.c,v 1.1 2015-04-02 11:41:18 mgostling Exp $
 *
 * $Log: signon.c,v $
 * Revision 1.1  2015-04-02 11:41:18  mgostling
 * Initial check-in to CVS
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.4  2012/03/29 14:13:15  cdobson
 * Eliminate compiler warning.
 *
 * Revision 1.3  2011/10/27 15:48:10  hmuneer
 * no message
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
 * Revision 1.3  2009/05/19 08:09:53  cvsuser
 * no message
 *
 * Revision 1.2  2008/06/02 12:27:17  swilson
 * Typo and layout fixes
 *
 * Revision 1.1  2008/05/15 15:32:40  swilson
 * Add build and version tracking. Implement & test missing c-library functions.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>


#include <bit/board_service.h>
#include <bit/mem.h>

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



