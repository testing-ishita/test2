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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx813.09x/SCR.c,v 1.2 2014-03-13 15:50:32 swilson Exp $
 * $Log: SCR.c,v $
 * Revision 1.2  2014-03-13 15:50:32  swilson
 * Fix RO and RW bitmasks in line with PIG: following a fix to the SCR test logic registers 0x212 and 0x216 were failing.
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011/03/22 13:31:21  hchalla
 * Initial Version
 *
 * Revision
 * board-specific superio support.
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>


SACReg SACRegs[] = { {"Status & Control Register 0",	      0x210, 0x00, 0xEF, 0x10 },
					 {"Status & Control Register 1",	      0x211, 0x00, 0xFF, 0x20 },
					 {"Status & Control Register 2",	      0x212, 0x0E, 0x70, 0x30 },
					 {"Status & Control Register 4",	      0x216, 0x68, 0x97, 0x40 },
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










