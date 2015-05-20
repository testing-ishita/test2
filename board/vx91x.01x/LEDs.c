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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx91x.01x/LEDs.c,v 1.1 2014-05-15 09:42:03 jthiru Exp $
 * $Log: LEDs.c,v $
 * Revision 1.1  2014-05-15 09:42:03  jthiru
 * Adding vx91x board sources
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011/03/22 13:27:11  hchalla
 * Added Register offset for USER LED.
 *
 * Revision
 * board-specific superio support.
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/board_service.h>

LED_INFO  localLEDs = {
						0x216,		// Reg
						0x40		// bit 2							
					  };


/*****************************************************************************
 * brdLEDinfo: returns the LED_INFO global data structure
 * RETURNS: SIO_Info* */
UINT32 brdLEDinfo(void *ptr)
{
	UINT8 temp2;
	temp2 = ((UINT8)dIoReadReg(localLEDs.port, REG_8));
	temp2 |= 80;
	vIoWriteReg(localLEDs.port, REG_8, temp2);

	*((LED_INFO**)ptr) = &localLEDs;

	return E__OK;
}










