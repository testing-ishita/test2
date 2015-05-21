
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

 /* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/mth/util.c,v 1.3 2013-10-08 07:14:58 chippisley Exp $
 * $Log: util.c,v $
 * Revision 1.3  2013-10-08 07:14:58  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.2  2013/09/26 12:42:10  chippisley
 * Added support for network Soak Master Client.
 *
 * Revision 1.1  2013/09/04 07:44:08  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2012/11/02 14:18:00  chippisley
 * Added vMTHTaskList().
 *
 * Revision 1.3  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.2  2010/06/24 16:13:51  hchalla
 * Code Cleanup, Added headers and comments.
 *
 */



/* includes */

#include <stdio.h>
#include <errors.h>
#include <string.h>

#include <private/port_io.h>
#include <private/cpu.h>
#include <bit/conslib.h>
 
#include <bit/board_service.h>

#include "mth.h"
#include "config.h"

extern void vTaskList( signed char *pcWriteBuffer );

#ifdef INCLUDE_DBGLOG
extern void dbgLogPrint( const UINT32 dbLevel );
extern void dbgLogClear( void );
#endif

#ifdef INCLUDE_NETWORK
extern long netStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen );
#endif

#ifdef INCLUDE_LWIP_SMC
extern size_t smcGetStats( signed char *pcWriteBuffer, size_t xWriteBufferLen );
#endif

static char pcDispBuffer[ (80 * 100) ];

/*****************************************************************************\
 *
 *  TITLE:  vMTHZeroCounters ()
 *
 *  ABSTRACT:  continue testing if error reported, Reset
 *  		   Run/Fail stats for all tests
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHZeroCounters (void)
{
	vMTHZeroTestStats ();

	puts ("RUN/FAIL counters have been reset to zero");

} /* vMTHZeroCounters () */

/*****************************************************************************\
 *
 *  TITLE:  vMTHHelpScreen ()
 *
 *  ABSTRACT:  display the help information
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHHelpScreen (void)
{
	int		iMaxPage;
	int		iPage;

	/* Display help pages until user quits */
	iMaxPage = iMTHHelpGetMaxPage();

	for (iPage = 1; iPage <= iMaxPage; iPage++)
	{
		vMTHHelpDisplayHeader (iPage);
		vMTHHelpDisplayPage (iPage);

		if (iPage < iMaxPage)
		{
			if (iMTHHelpPromptNext () != 0)
				return;
		}
	}

} /* vMTHHelpScreen () */


/*****************************************************************************\
 *
 *  TITLE:  vMTHVersionInfo ()
 *
 *  ABSTRACT:  display firmware version and build information
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHVersionInfo (void)
{
	DISPLAY_SIGNON_INFO info;

	info.iLevel = FWID_FULL;
	info.mode   = FWMODE_CUTE;
	board_service(SERVICE__BRD_DISPLAY_SIGNON, NULL, &info);

} /* vMTHVersionInfo () */


/*****************************************************************************\
 *
 *  TITLE:  vMTHTaskList ()
 *
 *  ABSTRACT:  display task list
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHTaskList (void)
{
	vTaskList( (signed char *) pcDispBuffer );
	puts( pcDispBuffer );
} /* vMTHTaskList () */


#ifdef INCLUDE_DBGLOG
/*****************************************************************************\
 *
 *  TITLE:  vMTHDblPrint ()
 *
 *  ABSTRACT:  display debug log
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHDblPrint (void)
{
	dbgLogPrint( 0 );
}


/*****************************************************************************\
 *
 *  TITLE:  vMTHDblClear ()
 *
 *  ABSTRACT:  clear debug log
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHDblClear (void)
{
	dbgLogClear();
}
#endif

#ifdef INCLUDE_NETWORK
/*****************************************************************************\
 *
 *  TITLE:  vMTHNetStats ()
 *
 *  ABSTRACT:  display network statistics
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHNetStats (void)
{
	while ( netStatsCommand( (signed char *) pcDispBuffer, sizeof(pcDispBuffer) ) )
	{
		puts( pcDispBuffer );
	}
	
}

#ifdef INCLUDE_LWIP_SMC
/*****************************************************************************\
 *
 *  TITLE:  vMTHSmcStats ()
 *
 *  ABSTRACT:  display SM Client statistics
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHSmcStats (void)
{
	if (smcGetStats( (signed char *) pcDispBuffer, sizeof(pcDispBuffer) ))
	{
		puts( pcDispBuffer );
	}
}
#endif

#endif /* INCLUDE_NETWORK */

