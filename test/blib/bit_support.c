
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

/* BIT_SUPPORT.C - BIT Mode-Specific Support (abstracting sys level code)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/bit_support.c,v 1.1 2013-09-04 07:11:17 chippisley Exp $
 *
 * $Log: bit_support.c,v $
 * Revision 1.1  2013-09-04 07:11:17  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2011/05/16 14:29:19  hmuneer
 * Info Passing Support
 *
 * Revision 1.3  2010/05/05 15:42:55  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.2  2009/05/21 12:49:58  swilson
 * Revise PBIT duration flags in the light of actual measurements of test times.
 *
 * Revision 1.1  2008/09/29 10:19:34  swilson
 * Various general functions supporting BIT operations. Routines for getting startup
 *  flags and operating modes. General-purpose debug function, safe for use in test code.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>

#include <bit/bit.h>

#include <private/debug.h>
#include <private/semaphore.h>


/* defines */

	/* Bitfields used by dStartupFlags
	 *
	 *	[0]   - 1 = diagnostic port active (3F8:9600,8,N,1)
	 *	[1]   - 0 = Local IBIT mode, 1 = PBIT
	 *	[2]   - 0 = BSP, 1 = AP
	 *	[3]   - 0 = BIT, 1 = CUTE	:only matters for combined BIT/CUTE image
	 *  [5-4] - power-on BIT duration
	 *  [7-6] - test failure action
	 *  [8]   - 1 = ESI contains address of BIOS_INFO structure
	 *  [9-11]- Serial port(s) baud rate
	 */

#define MASK__DEBUG_MODE			0x00000001	/* 1 = enable debug channel */
#define SHIFT__DEBUG_MODE			0
#define MASK__TEST_HANDLER			0x00000002	/* 0 = IBIT, 1 = PBIT */
#define SHIFT__TEST_HANDLER			1
#define MASK__CPU_CLASS				0x00000004	/* 0 = BSP, 1 = AP */
#define SHIFT__CPU_CLASS			2
#define MASK__CUTE_MODE				0x00000008	/* 0 = BIT,  1 = CUTE */
#define SHIFT__CUTE_MODE			3
#define MASK__PBIT_DURATION			0x00000030	/* 0 = 10S, 1 = 30S, 2 = 60S, 3 = TEST */
#define SHIFT__PBIT_DURATION		4
#define MASK__FAIL_ACTION			0x000000C0	/* 0 = BIT, 1 = BIOS, 2 = OS, 3 = IBIT */
#define SHIFT__FAIL_ACTION			6

#define MASK__BIOS_INFO				0x00000100	/* 0 = NO BIOS INFO, 1 = BIOS INFO location in ESI */
#define SHIFT__BIOS_INFO			8

#define MASK__BAUD_INFO				0x00000E00
#define SHIFT__BAUD_INFO			9

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT32	dStartupFlags;

extern UINT8 sysGetTestHandler (void);


/* forward declarations */



/*****************************************************************************
 * bStartupDebugMode: get the dStartupFlags debug mode
 *
 * RETURNS: SF__NO_DEBUG (0) or SF__DEBUG
 */

UINT8 bStartupDebugMode (void)
{
	return (UINT8)((dStartupFlags & MASK__DEBUG_MODE) >> SHIFT__DEBUG_MODE);

} /* bStartupDebugMode () */


/*****************************************************************************
 * bForceStartupDebugMode: sets the dStartupFlags debug mode
 *
 * RETURNS: -
 */

void bForceStartupDebugMode (void)
{
	dStartupFlags |= MASK__DEBUG_MODE;

} /* bForceStartupDebugMode () */


/*****************************************************************************
 * bStartupTestHandler: get the dStartupFlags test handler mode
 *
 * RETURNS: TH__LBIT (0) or TH__PBIT
 */

UINT8 bStartupTestHandler (void)
{
	return (UINT8)((dStartupFlags & MASK__TEST_HANDLER) >> SHIFT__TEST_HANDLER);

} /* bStartupTestHandler () */


/*****************************************************************************
 * bStartupCUTEMode: get the dStartupFlags CUTE mode
 *
 * RETURNS: SF__BIT (0) or SF__CUTE
 */

UINT8 bStartupCUTEMode (void)
{
	return (UINT8)((dStartupFlags & MASK__CUTE_MODE) >> SHIFT__CUTE_MODE);

} /* bStartupCUTEMode () */


/*****************************************************************************
 * bStartupCpuMode: get the dStartupFlags CPU operating mode
 *
 * RETURNS: SF__CPU_BSP (0) or SF__CPU_AP
 */

UINT8 bStartupCpuMode (void)
{
	return (UINT8)((dStartupFlags & MASK__CPU_CLASS) >> SHIFT__CPU_CLASS);

} /* bStartupCpuMode () */


/*****************************************************************************
 * bStartupPbitMode: get the dStartupFlags PBIT test duration
 *
 * RETURNS: SF__PBIT_10S (0), SF__PBIT_60S, SF__PBIT_5M	or SF__PBIT_10M
 */

UINT8 bStartupPbitMode (void)
{
	return (UINT8)((dStartupFlags & MASK__PBIT_DURATION) >> SHIFT__PBIT_DURATION);

} /* bStartupPbitMode () */


/*****************************************************************************
 * bStartupFailAction: get the dStartupFlags PBIT fail action
 *
 * RETURNS: SF__FAIL_BIT (0), SF__FAIL_BIOS, SF__FAIL_OS or SF__FAIL_IBIT
 */

UINT8 bStartupFailAction (void)
{
	return (UINT8)((dStartupFlags & MASK__FAIL_ACTION) >> SHIFT__FAIL_ACTION);

} /* bStartupFailAction () */


/*****************************************************************************
 * bGetTestHandler: get the current test handler mode
 *
 * This allows us to enforce restrictions on what code can be executed by
 * local and remote test handlers.
 *
 * RETURNS: TH__REMOTE (0), TH__PBIT or TH__LBIT
 */

UINT8 bGetTestHandler (void)
{
	return sysGetTestHandler ();

} /* bGetTestHandler () */


/*****************************************************************************
 * bGetBiosInfoPresent: checks if BIOS_INFO structure flag is set
 *
 * RETURNS: TH__NO_BIOSINFO (0), TH__BIOSINFO
 */

UINT8 bGetBiosInfoPresent (void)
{
	return (UINT8)((dStartupFlags & MASK__BIOS_INFO) >> SHIFT__BIOS_INFO);

} /* bGetBiosInfoPresent () */


/*****************************************************************************
 * bGetBaudRate: Get the BIOS baud rate
 *
 * RETURNS: baud rate
 */

UINT8 bGetBaudRate (void)
{
	return (UINT8)((dStartupFlags & MASK__BAUD_INFO) >> SHIFT__BAUD_INFO);

} /* bGetBaudRate () */


/*****************************************************************************
 * dGetPbitSelectionMask: get the PBIT testing duration
 *
 * The Test List permission field for PBIT mode is a single bit indicating the 
 * minimum test duration required to run the test.
 *
 * This function converts the BIOS selected test duration into a mask that is
 * compatible with the permissions bitmap. A bitwise AND can then determine
 * whether a test is compatible with the current selection.
 *
 * RETURNS: the BIOS selection in BITF__ format
 */

UINT32 dGetPbitSelectionMask (void)
{
	UINT32	dPbitDuration;

	switch (bStartupPbitMode ())
	{
		case SF__PBIT_TEST :
			dPbitDuration = BITF__EXE_PBIT_TEST;
			break;

		case SF__PBIT_60S :
			dPbitDuration = BITF__EXE_PBIT_60S;
			break;

		case SF__PBIT_30S :
			dPbitDuration = BITF__EXE_PBIT_30S;
			break;

		default :
			dPbitDuration = BITF__EXE_PBIT_10S;
	};

	return (dPbitDuration);

} /* dGetPbitSelectionMask () */


/*****************************************************************************
 * vDebugWrite: write a NULL terminated string to the debug device
 *
 * The debug console can only be used in PBIT or IBIT mode, as it makes use
 * of 'sys' functions. This routine allows Test code to include debug output,
 * but in a compatible manner, by first validating the operating mode.
 *
 * NOTE: this function does not automatically append a newline character.
 *
 * RETURNS: none
 */

static SEMAPHORE semaDebugWrite = 0;

void vDebugWrite
(
	char*	achMessage
)
{
	UINT8	bTestHandler;


	bTestHandler = bGetTestHandler ();

	if ((bTestHandler == TH__PBIT) || (bTestHandler == TH__LBIT))
	{
		sysSemaphoreLock (&semaDebugWrite);

		sysDebugOpen ();					/* do an open as may be first call */
		sysDebugWriteString (achMessage);
		sysDebugFlush ();					/* don't close: next write will be quicker */

		sysSemaphoreRelease (&semaDebugWrite);
	}

} /* vDebugWrite () */
