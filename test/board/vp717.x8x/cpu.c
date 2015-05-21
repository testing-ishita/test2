
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp717.x8x/cpu.c,v 1.1 2015-04-02 11:41:18 mgostling Exp $
 *
 * $Log: cpu.c,v $
 * Revision 1.1  2015-04-02 11:41:18  mgostling
 * Initial check-in to CVS
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.3  2011/01/20 10:01:23  hmuneer
 * CA01A151
 *
 * Revision 1.2  2010/09/13 08:42:12  cdobson
 * Changed number of cores from 2 to 4.
 *
 * Revision 1.1  2010/06/23 10:49:08  hmuneer
 * CUTE V1.01
 *
 * Revision 1.1  2010/01/19 12:03:15  hmuneer
 * vx511 v1.0
 *
 * Revision 1.1  2009/09/29 12:16:45  swilson
 * Create VP417 Board
 *
 * Revision 1.3  2009/06/09 16:14:14  swilson
 * Opening part of comment got missed during cut-and-paste operation!
 *
 * Revision 1.2  2009/06/09 16:07:03  swilson
 * Added new function for getting FPU exception handling capabilities for
 *  board.
 *
 * Revision 1.1  2009/06/08 16:02:45  swilson
 * Board-specific CPU related functions.
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/board_service.h>



/*****************************************************************************
 * brdGetNumCores: report expected number of CPU cores
 *
 * RETURNS: expected number of cores
 */
UINT32 brdGetNumCores (void *ptr)
{
	*((UINT8*)ptr) = 4;

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



