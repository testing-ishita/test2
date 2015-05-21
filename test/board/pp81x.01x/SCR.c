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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/pp81x.01x/SCR.c,v 1.1 2013-09-04 07:13:50 chippisley Exp $
 *
 * $Log: SCR.c,v $
 * Revision 1.1  2013-09-04 07:13:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>


SACReg SACRegs[] = {{"Status & Control Register 0",          0x210, 0x00, 0x07, 0x10 },
		 	 	 	 	 	 	 	{"Status & Control Register 1",          0x211, 0x01, 0x93, 0x20 },
		 	 	 	 	 	 	 	{"Status & Control Register 2",          0x212, 0x08, 0x38, 0x30 },
		 	 	 	 	 	 	 	{"CPCI Status Register",                    0x214, 0x00, 0x00, 0x40 },
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










