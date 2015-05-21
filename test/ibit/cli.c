
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
 * cli.c: top-level program sequencing.
 */

/*  $Header: /home/cvs/cvsroot/CCT_BIT_2/ibit/cli.c,v 1.1 2013-09-04 07:27:43 chippisley Exp $
 *
 *  $Log: cli.c,v $
 *  Revision 1.1  2013-09-04 07:27:43  chippisley
 *  Import files into new source repository.
 *
 *  Revision 1.8  2010/05/05 15:46:13  hmuneer
 *  Cute rev 0.1
 *
 *  Revision 1.7  2010/01/19 12:03:57  hmuneer
 *  vx511 v1.0
 *
 *  Revision 1.6  2008/09/17 15:51:51  swilson
 *  Remove use of global variables in IBIT directory - a hang over from the MON2 code
 *   this was based on.
 *
 *  Revision 1.5  2008/09/17 13:46:44  swilson
 *  Improvements to Local IBIT handler: add test execution counters and functions for display and zeroing them. Add continue on error function. Improve layout of results displayed when a test executes; include run and fail counts. Add support for VT100 compatible cursor-key handling within command-line editor. Move BIT execution support functions to a separate file BIT_UTIL.C. Make 'reset' command more robust.
 *
 *  Revision 1.4  2008/07/18 12:22:49  swilson
 *  Add call to clean-up resources after command executed.
 *
 *  Revision 1.3  2008/06/20 12:16:54  swilson
 *  Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *   C-code.
 *
 *  Revision 1.2  2008/06/02 12:27:19  swilson
 *  Typo and layout fixes
 *
 *  Revision 1.1  2008/05/12 14:46:42  swilson
 *  Local Interactive BIT command-line interface and utility functions - based on MON2
 *   0.2.-13.
 *
 */


/* includes */

#include <stdio.h>
#include <errors.h>

#include <bit/bit.h>
#include <bit/conslib.h>
#include <private/cpu.h>

#include "ibit.h"

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/***************************************************************************
 * vIbitCli: entry point for IBIT CLI
 *
 * RETURNS: none
 */

void vIbitCli (void)
{
	UINT32	dExitCode;
	int		iStatus;
	int		iCmdIndex;
	int		iCmdCount;
	int		iCmdType;
	char*	achUserArgs[10];
	char*	achCmdArgs[10];
	char	achUserCmd[128];
	char	achBuffer[80];


	/* Set some parameter defaults and do the initialization */

	vSetDefaults ();
	vInitCmdHistory ();

	dExitCode = dInitTestCounters ();

	if (dExitCode != E__OK)
	{
		sprintf  (achBuffer, "dInitTestCounters() generated runtime error 0x%08X", dExitCode);
		puts (achBuffer);
		sysHalt ();
	}

	/* Process commands - forever */

	while (1)
	{
		iStatus = iGetUserCommand (achUserCmd);

		if (iStatus == E__OK)
		{
			vProcessUserCmd (achUserCmd, &iCmdCount, achUserArgs);

			iStatus = iProcessCommandLine (iCmdCount, achUserArgs,
											&iCmdCount, achCmdArgs);
			vSetRepeatIndex (0);
		}

		else if ((iStatus == E__REPEAT) && (iGetRepeatIndex () >= 0))
		{
			vSetRepeatIndex (iGetRepeatIndex () + 1);
			iStatus = E__OK;
		}

		if ((iStatus == E__OK) && (iCmdCount > 0))
		{
			if (iGetRepeatIndex () < 1)
				iCmdType = iGetCommandType (achCmdArgs[0], &iCmdIndex);

			if (iCmdType != TYPE__UNKNOWN)
			{
				vCommandDispatcher (iCmdIndex, iCmdCount, achCmdArgs);

				vResourceCleanup ();
			}
		}
	}

} /* vIbitCli () */



