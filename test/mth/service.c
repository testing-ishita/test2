
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
 * SERVICE: interpretation of major commands
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/mth/service.c,v 1.1 2013-09-04 07:44:08 chippisley Exp $
 * $Log: service.c,v $
 * Revision 1.1  2013-09-04 07:44:08  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.2  2010/06/24 16:09:49  hchalla
 * Code Cleanup, Added headers and comments.
 *
 */




/* includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errors.h>

#include <private/port_io.h>
#include <private/mem_io.h>
#include <private/cpu.h>
#include <bit/conslib.h>
 
#include <bit/pci.h>
#include <bit/board_service.h>

#include "mth.h"




/* typedefs */

/* constants */



/*****************************************************************************\
 *
 *  TITLE:  vMTHExecuteTest ()
 *
 *  ABSTRACT:  execute a compound test sequence
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHExecuteTest
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[],
	int (*iMTHExtractArgs)()
)
{
	char	achCommand[128] = "";
	char*	pTokens[40];
	int		i;
	int		numTok;


	(void)iCmdType;

	for (i = 0; i < 40; i++)
		pTokens[i] = NULL;

	cputs ("Run Test: ");

	/* Post-process: skip the first argument - the 'T' command */

	for (i = 1; i < iCmdCount; i++)
		vExecPostProc (achCommand, achUserArgs[i]);

	puts (achCommand);

	numTok = iExecTokenize (achCommand, &pTokens[0]);

	/* Execute the tests - this is recursive, so call with loop count = 1 */

	vMTHExecParseCmd (1, 0, numTok-1, pTokens);

} /* vMTHExecuteTest () */

/*****************************************************************************\
 *
 *  TITLE:  vMTHTestSummary ()
 *
 *  ABSTRACT:  Display results for one or more tests
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHTestSummary
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[],
	int (*iMTHExtractArgs)()
)
{
	TEST_ITEM*	psTestList;
	UINT32	dRunCount;
	UINT32	dFailCount;
	UINT16	wTestNum;
	int		iIndex;
	int		iFound;
	int		iLineCount;
	char	achBuffer[80];


	(void)iCmdType;

	puts ("Test Summary:\n");

	if (iCmdCount > 1)
		wTestNum = atoi (achUserArgs[1]);
	else
		wTestNum = 0;


	/* Get the test directory */
	board_service(SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

	if (psTestList == NULL)
	{
		puts ("Test Directory not found.");
		return;
	}

	/* Loop through tests and display summary for chosen one or all */

	iFound = 0;
	iLineCount = 0;

	for (iIndex = 0; psTestList[iIndex].wTestNum != 0; iIndex++)
	{
		if ((wTestNum == 0) || (psTestList[iIndex].wTestNum == wTestNum))
		{
			vMTHGetTestStats (psTestList[iIndex].wTestNum, &dRunCount, &dFailCount);

			if ((wTestNum != 0) || (dRunCount > 0))
			{
				sprintf (achBuffer, "T %3d: %-40s  RUNS: %-6u FAILS: %u",
						psTestList[iIndex].wTestNum,
						psTestList[iIndex].achTestName,
						dRunCount, dFailCount);
				puts (achBuffer);


				/* Pause every 20 full lines, provided there is more to display */

				iLineCount++;		/* count lines */

				if ((iLineCount >= 20) && (psTestList[iIndex + 1].wTestNum != 0))
				{
					puts ("** Press any key to continue **");

					while (iGetExtdKeyPress () == 0);
					iLineCount = 0;
				}
			}

			iFound++;
		}
	}

	if (iFound == 0)
		puts ("Test not found");

} /* vMTHTestSummary () */



