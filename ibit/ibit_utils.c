
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
 * BIT_UTILS: code supporting BIST execution and statistics commands
 */

/*  $Header: /home/cvs/cvsroot/CCT_BIT_2/ibit/ibit_utils.c,v 1.3 2013-11-25 10:40:38 mgostling Exp $
 *
 *  $Log: ibit_utils.c,v $
 *  Revision 1.3  2013-11-25 10:40:38  mgostling
 *  Removed hal specific code as hal no longer used
 *
 *  Revision 1.2  2013-09-26 12:48:05  chippisley
 *  Removed obsolete HAL.
 *
 *  Revision 1.1  2013/09/04 07:27:43  chippisley
 *  Import files into new source repository.
 *
 *  Revision 1.8  2011/01/20 10:01:25  hmuneer
 *  CA01A151
 *
 *  Revision 1.7  2010/05/05 15:46:13  hmuneer
 *  Cute rev 0.1
 *
 *  Revision 1.6  2009/07/01 09:42:45  swilson
 *  Error in test range reversal.
 *
 *  Revision 1.5  2009/06/10 17:30:02  swilson
 *  Make "test range" IBIT command ignore empt test numbers after the first, making the
 *   command more uesfull.
 *
 *  Revision 1.4  2009/05/21 12:44:19  swilson
 *  Zero the test parameters array - some tests may rely on these being zero.
 *
 *  Revision 1.3  2009/05/20 15:50:28  swilson
 *  Add code to record test number in CMOS for CPU exception reporting.
 *
 *  Revision 1.2  2009/05/18 09:25:29  hmuneer
 *  no message
 *
 *  Revision 1.1  2008/09/29 13:38:22  swilson
 *  Originally named bit_utils.c, but renamed to avoid confusion with blib files.
 *
 *  Revision 1.3  2008/09/29 09:57:58  swilson
 *  Add code to display execution time for test.
 *
 *  Revision 1.2  2008/09/17 15:51:51  swilson
 *  Remove use of global variables in IBIT directory - a hang over from the MON2 code
 *   this was based on.
 *
 *  Revision 1.1  2008/09/17 13:46:44  swilson
 *  Improvements to Local IBIT handler: add test execution counters and functions for display and zeroing them. Add continue on error function. Improve layout of results displayed when a test executes; include run and fail counts. Add support for VT100 compatible cursor-key handling within command-line editor. Move BIT execution support functions to a separate file BIT_UTIL.C. Make 'reset' command more robust.
 *
 */

/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>

#include <bit/bit.h>
 
#include <bit/hal.h>
#include <bit/io.h>
#include <bit/conslib.h>
#include <bit/board_service.h>

#include <private/sys_delay.h>

#include "ibit.h"


/* defines */

#define MAX_SLOT		256			/* maximum of 256 unique tests */
#define INVALID_SLOT	MAX_SLOT	


/* typedefs */

typedef struct tagTestStats
{
	UINT16	wTestNum;
	UINT32	dRunCount;
	UINT32	dFailCount;

} TEST_STATS;


/* constants */

/* locals */

static TEST_STATS	asTestStats[MAX_SLOT];


/* globals */

/* externals */


#if 0 /* Not used, HAL no longer present */
extern jmp_buf sErrorRecoveryJb;
extern UINT32 cctServiceProvider (SYS_SRVC tServiceId, void* psParams);
#endif

/* forward declarations */

static void		vExecGetParams (UINT32* adParams, int* pt, int endTok, char* pTokens[]);
static UINT32	dDoTest (int iSupNotFound, int tNum, UINT32* adParams);

static void 	vLogTestResult (UINT16 wTestNum, UINT32 dTestResult);
static int		iFindSlot (UINT16 wTestNum);



/***************************************************************************
 * vExecParseCmd: parse the command and execute the tests
 *
 * This function is recursive
 *
 * RETURNS: none
 */

void vExecParseCmd
(
	int		loops,
	int		startTok,
	int		endTok,
	char*	pTokens[]
)
{
	UINT32	adParams[17];
	UINT32	dStatus;
	int		repeat;
	int		i;
	int		t;
	int		t1;
	int		t2;
	int		cnt;
	int		nest;
	int		step;
	int		supress;


	/* Zero the test params for safety */

	for (i = 0; i < 17; i++)
		adParams[i] = 0;


	/* Adjust loop count and step if 'endless' selected */

	step = 1;

	if (loops == 0)
	{
		step = 0;
		loops = 1;
	}

	/* Execute a test series based on loop count */

	dStatus = E__OK;

	for (repeat = 0; (dStatus == E__OK) && (repeat < loops); repeat += step)
	{
		for (t = startTok;  (dStatus == E__OK) && (t <= endTok); t++)
		{
			/* Process a group - { } */

			if (strcmp (pTokens[t], "{") == 0)
			{
				/* Find matching '}' */

				nest = 1;

				for (i = t+1; i <= endTok; i++)
				{
					if (strcmp (pTokens[i], "{") == 0)
						nest++;

					else if (strcmp (pTokens[i], "}") == 0)
					{
						if (nest == 1)		/* exit if same nesting level */
							break;
						else
							nest--;
					}
				}

				/* See if loop count supplied and recursively process the
				 * {...} contents
				 */

				if (strcmp (pTokens[i+1], ":") == 0)
				{
					cnt = atoi (pTokens[i+2]);

					vExecParseCmd (cnt, t+1, i-1, pTokens);
					t = i + 2;
				}

				else
				{
					vExecParseCmd (1, t+1, i-1, pTokens);
					t = i;
				}
			}

			/* Process a single test or test range */

			else
			{
				/* Assume a single test */

				t1 = atoi (pTokens[t]);
				t2 = t1;

				/* Process range */

				if (strcmp (pTokens[t+1], "-") == 0)
				{
					t += 2;
					t2 = atoi (pTokens[t]);
				}

				/* Check range is assending, if not swap them */

				if (t2 < t1)
				{
					i  = t1;
					t1 = t2;
					t2 = i;
				}

				/* Execute a series of tests */

				supress = 0;				/* don't supress "not found" for first test */

				vExecGetParams (adParams, &t, endTok, pTokens);

				for (i = t1; (dStatus == E__OK) && (i <= t2); i++)
				{
					dStatus = dDoTest (supress, i, adParams);

					/* When executing a range, execute all tests in between */

					if ((dStatus == E__TEST_NOT_FOUND) && (i > t1))
						dStatus = E__OK;

					/* Force a PASS if error action is CONTINUE and the test 
					 * failed. However runtime errors will still halt testing.
					 */

					if ((dStatus & E__BIT) && (iGetBeaFlag () == BEA_CONTINUE))
						dStatus = E__OK;

					supress = 1;			/* supress "not found" for further tests */
				}
			}

		} /* token loop */

	} /* repeat loop */

} /* vExecParseCmd () */


/***************************************************************************
 * vExecGetParams: dedicated test parameter extraction
 *
 * RETURNS: none
 */

static void vExecGetParams
(
	UINT32*	adParams,
	int*	pt,
	int		endTok,
	char*	pTokens[]
)
{
	int		p = 0;
	int		t = *pt;

	/* Set parameter count to zero */

	adParams[p++] = 0;

	/* Scan tokens for test parameters */

	if (strcmp (pTokens[t+1], "(") == 0)
	{
		t += 2;

		while ((t <= endTok) && (strcmp (pTokens[t], ")") != 0))
		{
			adParams[p++] = atoi (pTokens[t++]);
			adParams[0]++;
		}

		*pt = t;
	}

} /* vExecGetParams () */


/***************************************************************************
 * dDoTest: call the test function
 *
 * RETURNS: none
 */

static UINT32 dDoTest
(
	int 	iSupNotFound,
	int		wTestNum,
	UINT32*	adTestParams
)
{
	char	achBuffer[80];
	TEST_ITEM*	psTestList;
	UINT64	qStartTicks;
	UINT64	qEndTicks;
	UINT32	dErrorCode;
	UINT32	dRunCount;
	UINT32	dFailCount;
	int		iIndex;
	NV_RW_Data	nvdata;

	/* Get the test directory */

	board_service(SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

	if (psTestList == NULL)
	{
		puts ("Test Directory not found.");
		return (E__NO_TEST_DIRECTORY);
	}

	/* Find the test */

	for (iIndex = 0; psTestList[iIndex].wTestNum != 0; iIndex++)
	{
		if (psTestList[iIndex].wTestNum == wTestNum)
			break;
	} 

	if (psTestList[iIndex].wTestNum == 0)
	{
		if (iSupNotFound == 0)
			puts ("Test not found.");

		return (E__TEST_NOT_FOUND);
	}

#if 0 /* Not used, HAL no longer present */
	/* Install local error recovery point, so failures are counted */

	dErrorCode = setjmp (sErrorRecoveryJb);

	if (dErrorCode == E__OK)
	{
#endif	
		/* Execute test */

		sprintf (achBuffer, "T %3d: %-40s",
						psTestList[iIndex].wTestNum,
						psTestList[iIndex].achTestName);
		puts (achBuffer);

		/* record test number in case of exceptions */
		nvdata.dNvToken = NV__TEST_NUMBER;
		nvdata.dData    = wTestNum;
		board_service(SERVICE__BRD_NV_WRITE, NULL, &nvdata);

		board_service(SERVICE__BRD_POST_ON, NULL, NULL);

		qStartTicks = sysGetTimestamp ();

#if 0 /* Not used, HAL no longer present */
		dErrorCode = cctExecuteBit (wTestNum, adTestParams, cctServiceProvider);
#else
		dErrorCode = cctExecuteBit (wTestNum, adTestParams, NULL);
#endif

		qEndTicks = sysGetTimestamp ();

		board_service(SERVICE__BRD_POST_OFF, NULL, NULL);

		/* Display PASS/FAIL message */

		if (getx () == 0)
			cputs ("                                                ");

		if (dErrorCode == E__OK)
			cputs ("PASS");

		else
		{
			sprintf (achBuffer, "FAIL - 0x%08X", dErrorCode);
			puts (achBuffer);
		}
#if 0 /* Not used, HAL no longer present */
	}
	else	/* Handle/report runtime errors */
	{
		sprintf (achBuffer, "RUNTIME ERROR - 0x%08X", dErrorCode);
		puts (achBuffer);
	}
#endif

	/* Update the Pass/Fail stats for this test */

	vLogTestResult (wTestNum, dErrorCode);
	vGetTestStats (wTestNum, &dRunCount, &dFailCount);

	if (getx () == 0)
		cputs ("                                                    ");

	sprintf (achBuffer, "  RUNS: %-6u FAILS: %u", dRunCount, dFailCount);
	puts (achBuffer);

	/* Display execution time (debug only) */

	vShowExecutionTime (qStartTicks, qEndTicks);

	/* Check for loop abort */

	if (iCheckForEsc () == 1)
	{
		puts ("Stopped testing because 'Esc' pressed.");
		dErrorCode = E__ABORT_TEST;
	}

	return (dErrorCode);

} /* dDoTest () */


/***************************************************************************
 * dInitTestCounters: initialise the Test Stats. structure
 *
 * Scan through the test list and associate each test with a counter set.
 *
 * RETURNS: E__OK or an E__... error code
 */

UINT32 dInitTestCounters (void)
{
	TEST_ITEM*	psTestList;
	int		iIndex;
	int		iSlot;


	/* Zero all entries */

	for (iSlot = 0; iSlot < MAX_SLOT; iSlot++)
		asTestStats[iSlot].wTestNum   = INVALID_SLOT;

	vZeroTestStats ();

	/* Get the test directory */
	board_service(SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

	if (psTestList == NULL)
		return (E__NO_TEST_DIRECTORY);

	/* Scan through test list assigning a counter to each test */

	iSlot = 0;

	for (iIndex = 0; psTestList[iIndex].wTestNum != 0; iIndex++)
	{
		if (iSlot < MAX_SLOT)
			asTestStats[iSlot++].wTestNum = psTestList[iIndex].wTestNum;
		else
			return (E__TOO_MANY_TESTS);
	} 

	return (E__OK);

} /* dInitTestCounters () */


/***************************************************************************
 * vLogTestResult: log a test Pass or Fail
 *
 * RETURNS: none
 */

static void vLogTestResult
(
	UINT16	wTestNum,
	UINT32	dTestResult
)
{
	int		iSlot;


	iSlot = iFindSlot (wTestNum);

	if (iSlot != INVALID_SLOT)
	{
		asTestStats[iSlot].dRunCount++;

		if (dTestResult != E__OK)
			asTestStats[iSlot].dFailCount++;
	}

} /* vLogTestResult () */


/***************************************************************************
 * vGetTestStats: report Pass/Fail stats for a given test
 *
 * RETURNS: none
 */

void vGetTestStats
(
	UINT16	wTestNum,
	UINT32*	pdRunCount,
	UINT32*	pdFailCount
)
{
	int 	iSlot;


	*pdRunCount  = 0;
	*pdFailCount = 0;

	iSlot = iFindSlot (wTestNum);

	if (iSlot != INVALID_SLOT)
	{
		*pdRunCount  = asTestStats[iSlot].dRunCount;
		*pdFailCount = asTestStats[iSlot].dFailCount;
	}

} /* vGetTestStats () */


/***************************************************************************
 * vZeroTestStats: reset Run/Fail stats for all tests
 *
 * RETURNS: none
 */

void vZeroTestStats (void)
{
	int 	iSlot;


	for (iSlot = 0; iSlot < MAX_SLOT; iSlot++)
	{
		asTestStats[iSlot].dRunCount  = 0;
		asTestStats[iSlot].dFailCount = 0;
	}

} /* vZeroTestStats () */


/***************************************************************************
 * iFindSlot: find Pass/Fail counters associated with a given slot
 *
 * RETURNS: none
 */

static int iFindSlot
(
	UINT16	wTestNum
)
{
	int		iSlot;


	for (iSlot = 0; iSlot < MAX_SLOT; iSlot++)
	{
		if (asTestStats[iSlot].wTestNum == wTestNum)
			return (iSlot);
	}

	return (INVALID_SLOT);

} /* iFindSlot () */

