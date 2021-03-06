
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

/* app_flash.c - board-specific application flash information
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/io.h>

#include <bit/board_service.h>
#include <devices/mirrorflash.h>

/* defines */

#define PAGE_BASE		(0xFF000000)
#define PAGE_SIZE		(0x10000)		/* 8MB */

/* constants */

/* typedefs */

/* locals */

const AF_PARAMS	asFlashTypes[] =
{
	/* Array of devices supported on this board */

	{SPANSION_MID,  GL512_DID, GL512_SEC_SIZE, GL512_NUM_SEC, PAGE_BASE, PAGE_SIZE, "S29GL512N"},
	{           0,          0,              0,             0,         0,         0, NULL}
};



/*****************************************************************************
 * brdAppFlashGetParams: get structure array detailing supported flash
 *
 * RETURNS: pointer to AF_PARAMS structure
 */
UINT32 brdAppFlashGetParams (void *ptr)
{
	*((AF_PARAMS**)ptr) = (AF_PARAMS*)asFlashTypes;

	return E__OK;

} /* brdAppFlashGetParams () */


/*****************************************************************************
 * brdAppFlashSelectPage: select an applcation flash page
 *
 * Page selected modulo the number of selection bits. It is left to the
 * caller to validate the page number against the device and page sizes.
 *
 * RETURNS: none
 */
UINT32 brdAppFlashSelectPage(void *ptr)
{
	UINT8	bTemp;

	bTemp = ( *((UINT32*)ptr) << 1) & 0xFE;
	vIoWriteReg (0x310, REG_8, bTemp);

	bTemp = dIoReadReg (0x21D, REG_8);
	bTemp &= 0x8F;
	bTemp |= (UINT8)(( *((UINT32*)ptr) >> 7) & 0x07) << 4;
	vIoWriteReg (0x21D, REG_8, bTemp);

	return E__OK;

} /* brdAppFlashSelectPage () */


/*****************************************************************************
 * brdAppFlashEnable:
 * RETURNS: none
 */
UINT32 brdAppFlashEnable (void *ptr)
{
	(void)ptr;
	return E__OK;
} /* brdAppFlashEnable () */

