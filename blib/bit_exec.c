
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

/* bit_exec.c - BIT execution interface
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/bit_exec.c,v 1.4 2015-03-13 10:33:41 mgostling Exp $
 *
 * $Log: bit_exec.c,v $
 * Revision 1.4  2015-03-13 10:33:41  mgostling
 * Fixed compiler warning
 *
 * Revision 1.3  2015-03-12 15:24:02  hchalla
 * Added Debug prints for more debugging info on adTestParams.
 *
 * Revision 1.2  2013-09-26 12:58:11  chippisley
 * Removed obsolete HAL.
 *
 * Revision 1.1  2013/09/04 07:11:17  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/01/11 17:24:36  hchalla
 * New board service function added to the code base and new board VX813.
 *
 * Revision 1.4  2010/06/23 10:47:28  hmuneer
 * CUTE V1.01
 *
 * Revision 1.3  2009/05/20 13:14:49  swilson
 * Add interrupt resource clean-up.
 *
 * Revision 1.2  2008/09/29 10:18:00  swilson
 * Add code to verify OS test handler permissions. Improve commenting.
 *
 * Revision 1.1  2008/07/18 12:19:31  swilson
 * Moved here from kernel, as more appropriate location. Added resource cleanup function.
 *
 * Revision 1.1  2008/06/02 10:31:28  swilson
 * Add cctExecuteBit() interface and modify IBIT to use this. Add support for complex
 *  BIT execution expressions and BIT parameters.
 *
 */


/* includes */

#include <stdtypes.h>
#include <errors.h>

 
#include <bit/hal.h>
#include <bit/bit.h>
#include <bit/interrupt.h>
#include <bit/board_service.h>
#include <private/cpu.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */



/*****************************************************************************
 * cctExecuteBit: common entry point for BIST execution
 *
 * RETURNS: E__OK or E__... error code
 */

UINT32 cctExecuteBit 
(
	UINT16	wTestNum,								/* test number */
	UINT32* adTestParams,							/* test-specific parameters */
	UINT32	(*pfServiceProvider)(SYS_SRVC, void*)	/* ptr to Service Provider */
)
{
	TEST_ITEM*	psTestList;
	UINT32	dErrorCode;
	int		iIndex;
#ifdef DEBUG
	char achBuffer[80];
#endif

#if 0 /* Not used, HAL no longer present */

	/* Register the service provider */

	if (pfServiceProvider != NULL)
		cctRegisterServiceProvider (pfServiceProvider);
	else
		return (E__NO_SERVICE_PROVIDER);

#else
	if (pfServiceProvider != NULL)
		return (E__NO_SERVICE_PROVIDER);
#endif

	/* Get the test directory */

	board_service(SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

	if (psTestList == NULL)
		return (E__NO_TEST_DIRECTORY);

	/* Check that the test exists */

	for (iIndex = 0; psTestList[iIndex].wTestNum != 0; iIndex++)
	{
		if (psTestList[iIndex].wTestNum == wTestNum)
			break;
	} 

	if (psTestList[iIndex].wTestNum == 0)
		return (E__TEST_NOT_FOUND);


	/* Check that current test handler has permission to execute this test. 
	 *
	 * Only main() can set the test handler to anything other than TH__REMOTE,
	 * so just make sure OS cannot execute PBIT or IBIT exclusive tests. We
	 * have to assume that RBIT and CBIT flags have been honoured by the OS
	 * application.
	 */

	if (bGetTestHandler () == TH__REMOTE)
	{
		if (((BITF__RBIT | BITF__CBIT) & psTestList[iIndex].dFlags) == 0)
			return (E__TEST_PERMISSIONS);
	}

	/*
	 * Execute test
	 */
#ifdef DEBUG
	sprintf(achBuffer,"cctExecuteBit: adTestParams[0]: %x,adTestParams[1]:\n ",adTestParams[0],adTestParams[1]);
	sysDebugWriteString(achBuffer);
#endif

	dErrorCode = (psTestList[iIndex].pTestFn)(adTestParams);

	/* Ensure any system resources used by the test are released */

	vResourceCleanup ();

	return (dErrorCode);

} /* cctExecuteBit () */


/***************************************************************************
 * vResourceCleanup: release all finite system resources
 *
 * This is a common 
 *
 * RETURNS: none
 */

void vResourceCleanup (void)
{
	/* Free GDT and Page-Table slots */

	sysMmuFreeAll ();
	sysGdtDeleteAllSlots ();

} /* vResourceCleanup () */
