
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

/*  $Header: /home/cvs/cvsroot/CCT_BIT_2/ibit/util.c,v 1.2 2013-11-25 10:41:41 mgostling Exp $
 *
 *  $Log: util.c,v $
 *  Revision 1.2  2013-11-25 10:41:41  mgostling
 *  Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 *  Revision 1.1  2013-09-04 07:27:43  chippisley
 *  Import files into new source repository.
 *
 *  Revision 1.14  2012/02/17 11:29:05  hchalla
 *  Added support for PP 81x and reading the version string using temp_info.buffer.
 *
 *  Revision 1.13  2011/06/07 14:59:57  hchalla
 *  Added support for signon for BIOS data.
 *
 *  Revision 1.12  2011/01/20 10:01:25  hmuneer
 *  CA01A151
 *
 *  Revision 1.11  2010/05/05 15:46:14  hmuneer
 *  Cute rev 0.1
 *
 *  Revision 1.10  2009/05/21 12:45:44  swilson
 *  Implement the help system.
 *
 *  Revision 1.9  2008/09/29 10:22:10  swilson
 *  Rename BIT error action functions.
 *
 *  Revision 1.8  2008/09/19 14:45:58  swilson
 *  Modify 'reset' command to use a board-specific function.
 *
 *  Revision 1.7  2008/09/17 15:51:51  swilson
 *  Remove use of global variables in IBIT directory - a hang over from the MON2 code
 *   this was based on.
 *
 *  Revision 1.6  2008/09/17 13:46:44  swilson
 *  Improvements to Local IBIT handler: add test execution counters and functions for display and zeroing them. Add continue on error function. Improve layout of results displayed when a test executes; include run and fail counts. Add support for VT100 compatible cursor-key handling within command-line editor. Move BIT execution support functions to a separate file BIT_UTIL.C. Make 'reset' command more robust.
 *
 *  Revision 1.5  2008/06/02 10:31:28  swilson
 *  Add cctExecuteBit() interface and modify IBIT to use this. Add support for complex
 *   BIT execution expressions and BIT parameters.
 *
 *  Revision 1.4  2008/05/16 14:36:22  swilson
 *  Add test list management and test execution capabilities.
 *
 *  Revision 1.3  2008/05/15 15:32:40  swilson
 *  Add build and version tracking. Implement & test missing c-library functions.
 *
 *  Revision 1.2  2008/05/14 09:59:01  swilson
 *  Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 *  Revision 1.1  2008/05/12 14:46:43  swilson
 *  Local Interactive BIT command-line interface and utility functions - based on MON2
 *   0.2.-13.
 *
 */

/* includes */

#include <stdio.h>
#include <errors.h>

#include <private/port_io.h>
#include <private/cpu.h>
#include <bit/conslib.h>
 
#include <bit/board_service.h>

#include "ibit.h"

extern UINT8 sysGetTestHandler (void);

/***************************************************************************
 * vZeroCounters: continue testing if error reported
 *
 * RETURNS: none
 */

void vZeroCounters (void)
{
	vZeroTestStats ();

	puts ("RUN/FAIL counters have been reset to zero");

} /* vZeroCounters () */


/***************************************************************************
 * vHelpScreen: display the help information
 *
 * RETURNS: none
 */

void vHelpScreen (void)
{
	int		iMaxPage;
	int		iPage;


	/* Display help pages until user quits */

	iMaxPage = iHelpGetMaxPage ();

	for (iPage = 1; iPage <= iMaxPage; iPage++)
	{
		vHelpDisplayHeader (iPage);
		vHelpDisplayPage (iPage);

		if (iPage < iMaxPage)
		{
			if (iHelpPromptNext () != 0)
				return;
		}
	}

} /* vHelpScreen () */


/***************************************************************************
 * vVersionInfo: display firmware version and build information
 *
 * RETURNS: none
 */
void vVersionInfo (void)
{
	UINT8 mode;
	DISPLAY_SIGNON_INFO info;
	Temp_String_Info temp_info;
	char    vBuffer[32];
	char	achBuffer[80];

	mode = 	sysGetTestHandler();
	if((mode == TH__LBIT) || mode == (TH__PBIT))
	{
		info.iLevel = FWID_FULL;
		info.mode   = FWMODE_BIT;
		board_service(SERVICE__BRD_DISPLAY_SIGNON, NULL, &info);

		temp_info.size   = 32;
		temp_info.buffer = (UINT8 *)vBuffer;
		if(board_service(SERVICE__BRD_GET_BOARD_NAME, NULL, &temp_info) == E__OK)
		{
			sprintf (achBuffer, "NAME: %s", temp_info.buffer);
			puts (achBuffer);
		}

		temp_info.size   = 32;
		temp_info.buffer = (UINT8 *)vBuffer;
		if(board_service(SERVICE__BRD_GET_TEMP_GRADE_STRING, NULL, &temp_info) == E__OK)
		{
			sprintf (achBuffer, "TYPE: %s", temp_info.buffer);
			puts (achBuffer);
		}

		temp_info.size   = 32;
		temp_info.buffer = (UINT8 *)vBuffer;
		if(board_service(SERVICE__BRD_GET_BIOS_VERSION, NULL, vBuffer) == E__OK)
		{
			sprintf (achBuffer, "BIOS: %s", vBuffer);
			puts (achBuffer);
		}
	}
	else
	{
		info.iLevel = FWID_FULL;
		info.mode   = FWMODE_CUTE;
		board_service(SERVICE__BRD_DISPLAY_SIGNON, NULL, &info);
		temp_info.size   = 32;
		temp_info.buffer = (UINT8 *)vBuffer;
		if(board_service(SERVICE__BRD_GET_BOARD_NAME, NULL, &temp_info) == E__OK)
		{
			sprintf (achBuffer, "NAME: %s", temp_info.buffer);
			puts (achBuffer);
		}

		temp_info.size   = 32;
		temp_info.buffer = (UINT8 *)vBuffer;
		if(board_service(SERVICE__BRD_GET_TEMP_GRADE_STRING, NULL, &temp_info) == E__OK)
		{
			sprintf (achBuffer, "TYPE: %s", temp_info.buffer);
			puts (achBuffer);
		}

		if(board_service(SERVICE__BRD_GET_BIOS_VERSION, NULL, vBuffer) == E__OK)
		{
			sprintf (achBuffer, "BIOS: %s", vBuffer);
			puts (achBuffer);
		}
	}
} /* vVersionInfo () */



