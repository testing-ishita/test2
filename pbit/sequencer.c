
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
 * pbit.c: Power-on BIT Test handler
 */

/*  $Header: /home/cvs/cvsroot/CCT_BIT_2/pbit/sequencer.c,v 1.2 2013-09-26 12:39:48 chippisley Exp $
 *
 *  $Log: sequencer.c,v $
 *  Revision 1.2  2013-09-26 12:39:48  chippisley
 *  Removed obsolete HAL
 *
 *  Revision 1.1  2013/09/04 07:44:51  chippisley
 *  Import files into new source repository.
 *
 *  Revision 1.9  2011/01/20 10:01:26  hmuneer
 *  CA01A151
 *
 *  Revision 1.8  2009/05/21 12:51:43  swilson
 *  Add test parameters array and zero it - some tests may rely on elements other than the first being zero.
 *
 *  Revision 1.7  2009/05/18 09:25:30  hmuneer
 *  no message
 *
 *  Revision 1.6  2009/01/22 14:49:32  swilson
 *  Updates to handling of NVRAM storage for PBIT results.
 *
 *  Revision 1.5  2008/09/29 13:27:05  swilson
 *  Clean up makefile include dependencies.
 *
 *  Revision 1.4  2008/09/29 10:06:01  swilson
 *  Use function calls, not direct access to dBitFlasgs. Use vDebugWrite() not 'sys' function.
 *
 *  Revision 1.3  2008/09/19 14:41:27  swilson
 *  Update PBIT main loop so that testing works correctly and supports test duration and
 *   failure action flags. Added debug output for progress monitoring.
 *
 *  Revision 1.2  2008/07/18 12:40:30  swilson
 *  Add call to clean-up resources after running a test.
 *
 *  Revision 1.1  2008/06/20 12:16:54  swilson
 *  Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *   C-code.
 *
 */

/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>

 
#include <bit/bit.h>
#include <bit/board_service.h>

#include <private/cpu.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

#if 0 /* Not used, HAL no longer present */
extern UINT32 cctServiceProvider (SYS_SRVC tServiceId, void* psParams);
#endif

/* forward declarations */



/***************************************************************************
 * vPbitSequencer: execute PBIT
 *
 *	[0],   1 = diagnostic port active (3F8:9600,8,N,1)
 *	[1],   0 = Local IBIT mode, 1 = PBIT
 *	[2],   0 = BSP, 1 = AP
 *  [5-4], - power-on BIT duration
 *  [7-6], - test failure action
 *
 * RETURNS: none
 */

void vPbitSequencer (void)
{
	TEST_ITEM*	psTestList;
	UINT32	dErrorCode;
	UINT32	dPbitMask;
	UINT32	dFailureAction;
	UINT32	adTestParams[17];
	int		iIndex;
	char	achBuffer[80];
	NV_RW_Data	nvdata;

	
	dErrorCode = E__OK;

	/* Test params cannot used in PBIT mode */

	for (iIndex = 0; iIndex < 17; iIndex++)
		adTestParams[iIndex] = 0;

	vDebugWrite ("PBIT Started...\n\n");

	/* Put BIT flags into variables */

	dPbitMask = dGetPbitSelectionMask ();
	dFailureAction = bStartupFailAction ();

	/* Get the test directory */
	board_service(SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

	if (psTestList == NULL)
		dErrorCode = E__NO_TEST_DIRECTORY;


	/* Loop through test list, executing all tests that are compatible with
	 * this power-on test level
	 */

	for (iIndex = 0; (dErrorCode == E__OK) && (psTestList[iIndex].wTestNum != 0); iIndex++)
	{
		/* Check the PBIT duration flag */

		if ((dPbitMask & psTestList[iIndex].dFlags) != 0)
		{
			sprintf (achBuffer, "Starting test %-3d: ", psTestList[iIndex].wTestNum);
			vDebugWrite (achBuffer);

			/* Record the test number and a runtime error code in NVRAM in
			 * case the board hangs
			 */

			nvdata.dNvToken = NV__TEST_NUMBER;
			nvdata.dData    = psTestList[iIndex].wTestNum;
			board_service(SERVICE__BRD_NV_WRITE, NULL, &nvdata);

			nvdata.dNvToken = NV__ERROR_CODE;
			nvdata.dData    = E__BOARD_HANG;
			board_service(SERVICE__BRD_NV_WRITE, NULL, &nvdata);

			board_service(SERVICE__BRD_POST_ON, NULL, NULL);

			/* Start the watchdog timer */
			board_service(SERVICE__BRD_WATCHDOG_ENABLE, NULL, NULL);

			/* Execute the test */

#if 0 /* Not used, HAL no longer present */
			dErrorCode = cctExecuteBit (psTestList[iIndex].wTestNum, 
										adTestParams, cctServiceProvider);
#else
			dErrorCode = cctExecuteBit (psTestList[iIndex].wTestNum, 
										adTestParams, NULL);
#endif
			vResourceCleanup ();

			/* Stop the watchdog timer */

			board_service(SERVICE__BRD_WATCHDOG_DISABLE, NULL, NULL);

			board_service(SERVICE__BRD_POST_OFF, NULL, NULL);

			/* Update the result code */

			nvdata.dNvToken = NV__ERROR_CODE;
			nvdata.dData    = dErrorCode;
			board_service(SERVICE__BRD_NV_WRITE, NULL, &nvdata);

			if (dErrorCode != E__OK)
			{
				sprintf (achBuffer, "FAIL - Error code 0x%08X\n", dErrorCode);
				vDebugWrite (achBuffer);
			}
			else
			{
				vDebugWrite ("PASS\n");
			}
		}
	}

	/* If a test failed, check if we should halt here and flash an LED */

	if ((dErrorCode != E__OK) && (dFailureAction == SF__FAIL_BIT))
	{
		vDebugWrite ("\nPBIT Terminated - Halting\n");
		board_service(SERVICE__BRD_FLASH_LED, NULL, NULL);
	}

	/* Done! So boot back into BIOS mode */

	vDebugWrite ("\nPBIT Complete - returning to BIOS\n");

	nvdata.dNvToken = NV__BIOS_BOOT;
	nvdata.dData    = 1;
	board_service(SERVICE__BRD_NV_WRITE, NULL, &nvdata);

	board_service(SERVICE__BRD_RESET, NULL, NULL);

	vDebugWrite ("\nBoard Reset Failed\n");
	sysHalt ();

} /* vPbitSequencer () */

