
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/trb1x.msd/cpu.c,v 1.1 2013-11-25 14:21:34 mgostling Exp $
 *
 * $Log: cpu.c,v $
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011-08-26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
 *
 *
 *
 */


/* includes */

#include <stdtypes.h>
#include <bit/msr.h>
#include <bit/board_service.h>
#include <stdio.h>
#include <bit/console.h>

/* defines */
//#define DEBUG

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



