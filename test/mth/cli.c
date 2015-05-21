
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


/* includes */

#include <stdio.h>
#include <errors.h>

#include <bit/bit.h>
#include <bit/conslib.h>
#include <private/cpu.h>

#include "mth.h"

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************\
 *
 *  TITLE:  vMTHCli ()
 *
 *  ABSTRACT: Entry point for the MTH command line interface,
 *   		  It takes the command input from the user, process the command,
 *   		  dispatch the command for the execution.
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/

void vMTHCli (void)
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
	vMTHInitCmdHistory ();

	dExitCode = dMTHInitTestCounters ();

	if (dExitCode != E__OK)
	{
		sprintf  (achBuffer, "dInitTestCounters() generated runtime error 0x%08X", dExitCode);
		puts (achBuffer);
		sysHalt ();
	}

	/* Process commands - forever */

	while (1)
	{
		iStatus = iMTHGetUserCommand (achUserCmd);

		if (iStatus == E__OK)
		{
			vProcessUserCmd (achUserCmd, &iCmdCount, achUserArgs);

			iStatus = iMTHProcessCommandLine (iCmdCount, achUserArgs,
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
				iCmdType = iMTHGetCommandType (achCmdArgs[0], &iCmdIndex);

			if (iCmdType != TYPE__UNKNOWN)
			{
				vMTHCommandDispatcher (iCmdIndex, iCmdCount, achCmdArgs);

				vResourceCleanup ();
			}
		}
	}

} /* vMTHCli () */



