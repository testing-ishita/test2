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

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/io.h>

#include <bit/board_service.h>

LED_INFO  localLEDs = {
						0x212,		// Reg
						0x10		// bit 4
					  };


/*****************************************************************************
 * brdLEDinfo: returns the LED_INFO global data structure
 * RETURNS: SIO_Info* */
UINT32 brdLEDinfo(void *ptr)
{

	*((LED_INFO**)ptr) = &localLEDs;

	return E__OK;
}










