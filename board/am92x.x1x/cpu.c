
/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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

/* cpu.c - board-specific CPU related functions
 */

/* includes */

#include <stdtypes.h>
#include <bit/msr.h>
#include <bit/board_service.h>
#include <stdio.h>
#include <bit/console.h>

//#undef DEBUG

/*****************************************************************************
 * brdGetNumCores: report expected number of CPU cores
 *
 * RETURNS: expected number of cores
 */
UINT32 brdGetNumCores (void *ptr)
{
#ifdef DEBUG
	char buffer[64];
#endif
	UINT32 msrHigh=0,msrLow=0;

	/* read the CORE_THREAD_COUNT MSR */
	vReadMsr(0x35,&msrHigh,&msrLow);

#ifdef DEBUG
	sprintf(buffer, "Cores %d, Threads %d\n", ((msrLow >> 16) & 0x0000ffff), (msrLow & 0x0000ffff) );
	vConsoleWrite(buffer);
#endif

	*((UINT8*)ptr) = ((msrLow >> 16) & 0x0000ffff);

	return E__OK;		/* single, dual-core CPU */

} /* brdGetNumCores () */


/*****************************************************************************
 * brdGetFpuExceptionMap: report exception handling supported by CPU/chipset
 *
 * RETURNS: bitmap indicating supported methods
 */
UINT32 brdGetFpuExceptionMap (void *ptr)
{
	*((UINT32*)ptr) = FPU_EXCEPTION_MF; /* only support MF, no FERR */

	return E__OK;

} /* brdGetFpuExceptionMap () */



