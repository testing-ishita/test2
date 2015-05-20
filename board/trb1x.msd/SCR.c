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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/trb1x.msd/SCR.c,v 1.1 2013-11-25 14:21:34 mgostling Exp $
 * $Log: SCR.c,v $
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
// reg 0x212 bit1 and bit4 are RO
SACReg SACRegsPeriph[] = {{"Status & Control Register 0", 0x210, 0x00, 0xFF, 0x10 },
						  {"Status & Control Register 1", 0x211, 0x01, 0xFC, 0x20 },
						  //{"Status & Control Register 2", 0x212, 0x0D, 0x92, 0x30 },
						  {NULL,						  0x00,  0x00, 0x00, 0x00 }
				    	 };

//													 	  Port    WR    RO   Error
// reg 0x212 bit1 is RW, bit4 is R0
SACReg SACRegsSyscon[] = {{"Status & Control Register 0", 0x210, 0x00, 0xFF, 0x10 },
						  {"Status & Control Register 1", 0x211, 0x01, 0xFC, 0x20 },
						  //{"Status & Control Register 2", 0x212, 0x0F, 0x90, 0x30 },
						  {NULL,						  0x00,  0x00, 0x00, 0x00 }
						 };

//													 		   Port    WR    RO   Error
// reg 0x212 bit1 and bit4 are RW
SACReg SACRegsIndependant[] = {{"Status & Control Register 0", 0x210, 0x00, 0xFF, 0x10 },
							   {"Status & Control Register 1", 0x211, 0x01, 0xFC, 0x20 },
							   //{"Status & Control Register 2", 0x212, 0x1F, 0x80, 0x30 },
							   {NULL,						   0x00,  0x00, 0x00, 0x00 }
							  };


/*****************************************************************************
 * brdGetSCR: returns the Status & Control Registers global data structure
 * RETURNS: SACReg* */
UINT32 brdGetSCR(void *ptr)
{
	UINT8	reg212;

	reg212 = ((UINT8)dIoReadReg(0x0212, REG_8));
	
	if ((reg212 & 0x40) != 0)
		*((SACReg**)ptr) = SACRegsIndependant;		// bit1 and bit4 are RW
	else if ((reg212 & 0x10) != 0)
		*((SACReg**)ptr) = SACRegsSyscon;		// bit1 is RW, bit4 is RO
	else
		*((SACReg**)ptr) = SACRegsPeriph;		// bit1 and bit4 are RO
	

	return E__OK;
}










