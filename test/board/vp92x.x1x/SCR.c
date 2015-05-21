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

/* superio.c - board-specific superio information 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/SCR.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: SCR.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
 * Import files into new source repository.
 *
 * Revision 1.2  2012/07/03 12:42:19  madhukn
 * updated Status and control register definitions for VP91x,
 * Added VME tests for CUTE MTH
 *
 * Revision 1.1  2012/06/22 11:41:58  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>


SACReg SACRegs[] = { {"Status & Control Register 0",	      0x210, 0x00, 0xff, 0x10 },
					 {"Status & Control Register 1",	      0x212, 0x00, 0xff, 0x20 },
					 {"Status & Control Register 2",	      0x211, 0x00, 0x24, 0x30 },
					 {"Status & Control Register 3",	      0x21d, 0x00, 0x70, 0x40 },
					 {NULL,								      0x00,  0x00, 0x00, 0x00 }
				   };


/*****************************************************************************
 * brdGetSCR: returns the Status & Control Registers global data structure
 * RETURNS: SACReg* */
UINT32 brdGetSCR(void *ptr)
{
	*((SACReg**)ptr) = SACRegs;

	return E__OK;
}










