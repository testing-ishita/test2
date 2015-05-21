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

/* superio.c - board-specific superio information
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>

#include <bit/board_service.h>


SACReg SACRegs[] = {									 /* PORT WRMask ROMask Error */
					{"Status & Control Register 1",			0x212, 0x00, 0xBF, 0x30 },
					{"GPIO Register",						0x213, 0x08, 0xf7, 0x40 },
					{"Status & Control Register 2",			0x214, 0x00, 0xff, 0x50 },
					{"Interrupt Control Register ",			0x215, 0x0F, 0x80, 0x60 },

					{NULL,									0x00,  0x00, 0x00, 0x00 }
				   };


/*****************************************************************************
 * brdGetSCR: returns the Status & Control Registers global data structure
 * RETURNS: SACReg* */
UINT32 brdGetSCR(void *ptr)
{
	*((SACReg**)ptr) = SACRegs;

	return E__OK;
}










