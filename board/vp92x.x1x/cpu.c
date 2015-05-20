
/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/cpu.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: cpu.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
 * Import files into new source repository.
 *
 * Revision 1.1  2012/06/22 11:41:59  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <bit/msr.h>
#include <bit/board_service.h>
#include <stdio.h>
#include <bit/console.h>

//#define DEBUG
/*****************************************************************************
 * brdGetNumCores: report expected number of CPU cores
 *
 * RETURNS: expected number of cores
 */
UINT32 brdGetNumCores (void *ptr)
{
	UINT32 msrHigh = 0, msrLow = 0;
#ifdef DEBUG
	char buffer[64];
#endif

	/* read the CORE_THREAD_COUNT MSR */
	vReadMsr(0x35, &msrHigh, &msrLow);

#ifdef DEBUG
	sprintf(buffer, "Cores %d, Threads %d\n", ((msrLow >> 16) & 0x0000ffff), (msrLow & 0x0000ffff) );
	vConsoleWrite(buffer);
#endif

	*((UINT8*)ptr) = (msrLow & 0x0000ffff);

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



