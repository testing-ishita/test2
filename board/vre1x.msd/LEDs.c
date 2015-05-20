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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/LEDs.c,v 1.1 2014-03-04 10:25:18 mgostling Exp $
 * $Log: LEDs.c,v $
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
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
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/board_service.h>

LED_INFO  localLEDs = {
						0x210,		// Reg
						0x08		// bit 3
					  };


/*****************************************************************************
 * brdLEDinfo: returns the LED_INFO global data structure
 * RETURNS: SIO_Info* */
UINT32 brdLEDinfo(void *ptr)
{
	*((LED_INFO**)ptr) = &localLEDs;

	return E__OK;
}










