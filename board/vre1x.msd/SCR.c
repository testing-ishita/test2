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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/SCR.c,v 1.2 2014-04-23 15:30:03 mgostling Exp $
 * $Log: SCR.c,v $
 * Revision 1.2  2014-04-23 15:30:03  mgostling
 * Exclide bit 4 of register 0x213 from SCR Test.
 *
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.2  2013-09-18 09:28:53  jthiru
 * fixed SCR test in SOAK TRB12
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2.2.1  2012-11-05 15:55:03  hchalla
 * Added support to test 5 tr 80x in a 3U VPX daisy chained backplane.
 *
 * Revision 1.2  2011-08-26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/io.h>
#include <bit/board_service.h>
//													 	  Port    WR    RO   Error
// reg 0x212 bit1 is RW, bit4 is R0
// reg 0x213 bit4 is RW but resets both PLX8733 switches wiping out their PCI enumeration
SACReg SACRegsSyscon[] = {{"Status & Control Register 0", 0x210, 0x08, 0xF7, 0x10 },
						  {"Status & Control Register 1", 0x211, 0x8C, 0x33, 0x20 },
						  {"Status & Control Register 2", 0x212, 0x00, 0xC7, 0x30 },
						  {"Status & Control Register 3", 0x213, 0x02, 0x89, 0x40 },
						  {"Interrupt Control Register ", 0x215, 0x87, 0x00, 0x50 },
						  {NULL,						  0x00,  0x00, 0x00, 0x00 }
						 };

/*****************************************************************************
 * brdGetSCR: returns the Status & Control Registers global data structure
 * RETURNS: SACReg* */
UINT32 brdGetSCR(void *ptr)
{
	*((SACReg**)ptr) = SACRegsSyscon;
	return E__OK;
}










