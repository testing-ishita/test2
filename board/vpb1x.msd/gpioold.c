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
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/gpioold.c,v 1.1 2015-02-25 17:50:45 hchalla Exp $
 *
 * $Log: gpioold.c,v $
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.1  2012/06/22 11:42:00  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */
#include <stdtypes.h>
#include <bit/bit.h>

#include <bit/board_service.h>

GPIO_INFO  localGpio = {
								0x213,
								0x2,
								0x2
						};

/*****************************************************************************
 * brdGetTempinfo: returns the TEMPERATURE_INFO global data structure
 * RETURNS: TEMPERATURE_INFO* */
UINT32 brdGetGPIOInfo(void *ptr)
{
	*((GPIO_INFO**)ptr) = &localGpio;

	return E__OK;
}
