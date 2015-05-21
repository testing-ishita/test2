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

#include <stdtypes.h>
#include <bit/bit.h>

#include <bit/board_service.h>

GPIO_INFO  localGpio = {
								0x214,
								0x4,
								0x2
						};

/*****************************************************************************
 * brdGetGPIOInfo: returns the GPIO_INFO global data structure
 * RETURNS: GPIO_INFO*
 */
UINT32 brdGetGPIOInfo(void *ptr)
{
	*((GPIO_INFO**)ptr) = &localGpio;

	return E__OK;
}
