
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx91x.01x/cpu.c,v 1.1 2014-05-15 09:42:03 jthiru Exp $
 *
 * $Log: cpu.c,v $
 * Revision 1.1  2014-05-15 09:42:03  jthiru
 * Adding vx91x board sources
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011/03/22 13:19:12  hchalla
 * Added code for getting number of CPU cores by reading MSR.
 *
 * Revision 1.1  2011/02/28 11:57:43  hmuneer
 * no message
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
	UINT32 msrHigh=0,msrLow=0;
#ifdef DEBUG
	char buffer[64];
#endif

	/* read the CORE_THREAD_COUNT MSR */
	vReadMsr(0x35,&msrHigh,&msrLow);

#ifdef DEBUG
	sprintf(buffer, "Coers %d, Threads %d\n", ((msrLow >> 16) & 0x0000ffff), (msrLow & 0x0000ffff) );
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



