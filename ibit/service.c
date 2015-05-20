
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

/*  $Header: /home/cvs/cvsroot/CCT_BIT_2/ibit/service.c,v 1.1 2013-09-04 07:27:43 chippisley Exp $
 *
 *  $Log: service.c,v $
 *  Revision 1.1  2013-09-04 07:27:43  chippisley
 *  Import files into new source repository.
 *
 *  Revision 1.15  2011/01/20 10:01:25  hmuneer
 *  CA01A151
 *
 *  Revision 1.14  2010/05/05 15:46:13  hmuneer
 *  Cute rev 0.1
 *
 *  Revision 1.13  2009/09/29 12:22:55  swilson
 *  Add command for throwing a GPF (for testing the exception handling mechanism).
 *
 *  Revision 1.12  2009/05/21 12:45:03  swilson
 *  Decode test flags into individual bits for clarity.
 *
 *  Revision 1.11  2009/05/20 14:41:46  swilson
 *  Add pagenation to TM and SUM command output.
 *
 *  Revision 1.10  2009/05/20 09:02:51  swilson
 *  Correction to Test Menu display.
 *
 *  Revision 1.9  2008/09/29 10:22:59  swilson
 *  Add test duration to summary display.
 *
 *  Revision 1.8  2008/09/17 15:51:51  swilson
 *  Remove use of global variables in IBIT directory - a hang over from the MON2 code
 *   this was based on.
 *
 *  Revision 1.7  2008/09/17 13:46:44  swilson
 *  Improvements to Local IBIT handler: add test execution counters and functions for display and zeroing them. Add continue on error function. Improve layout of results displayed when a test executes; include run and fail counts. Add support for VT100 compatible cursor-key handling within command-line editor. Move BIT execution support functions to a separate file BIT_UTIL.C. Make 'reset' command more robust.
 *
 *  Revision 1.6  2008/07/24 14:34:23  swilson
 *  Fix looped test execution - should use BIT mechanism, rather than MON2. Clean up comments to remove // style
 *
 *  Revision 1.5  2008/07/18 12:23:23  swilson
 *  Fix static function declarations.
 *
 *  Revision 1.4  2008/06/20 12:16:54  swilson
 *  Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *   C-code.
 *
 *  Revision 1.3  2008/06/02 10:31:28  swilson
 *  Add cctExecuteBit() interface and modify IBIT to use this. Add support for complex
 *   BIT execution expressions and BIT parameters.
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
#include <stdlib.h>
#include <string.h>
#include <errors.h>

#include <private/port_io.h>
#include <private/mem_io.h>
#include <private/cpu.h>

 
#include <bit/pci.h>
#include <bit/conslib.h>
#include <bit/board_service.h>

#include "ibit.h"




/***************************************************************************
 * vExecuteTest: execute a compound test sequence
 *
 * RETURNS: none
 */

void vExecuteTest
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	char	achCommand[128] = "";
	char*	pTokens[40];
	int		i;
	int		numTok;


	(void)iCmdType;

	for (i = 0; i < 40; i++)
		pTokens[i] = NULL;

	cputs ("Run BIT: ");

	/* Post-process: skip the first argument - the 'T' command */

	for (i = 1; i < iCmdCount; i++)
		vExecPostProc (achCommand, achUserArgs[i]);

	puts (achCommand);

	numTok = iExecTokenize (achCommand, &pTokens[0]);

	/* Execute the tests - this is recursive, so call with loop count = 1 */

	vExecParseCmd (1, 0, numTok-1, pTokens);

} /* vExecuteTest () */


/***************************************************************************
 * vTestSummary: display results for one or more tests
 *
 * RETURNS: none
 */

void vTestSummary
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
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
			vGetTestStats (psTestList[iIndex].wTestNum, &dRunCount, &dFailCount);

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

} /* vTestSummary () */
