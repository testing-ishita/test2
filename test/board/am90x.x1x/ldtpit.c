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

/* ldtpit.c - board-specific LDT information
 */



#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/io.h>
#include "cctboard.h"
#include <bit/board_service.h>

/* includes */
LDTPIT_INFO  localLDTPIT = {
						0x218,
						0x219,
						0x21A,
						0x21B,
						0x21C,
						IRQ
};


/*****************************************************************************
 * brdLEDinfo: returns the LED_INFO global data structure
 * RETURNS: SIO_Info* */
UINT32 brdLdtPitInfo (void *ptr)
{
	UINT8 temp2 = 0;

	vIoWriteReg(localLDTPIT.bLdtPitControl, REG_8, 0);

	/* Enable LDT PIT Interrupt */
	temp2 = ((UINT8)dIoReadReg(INTERRUPT_CTRL_REG, REG_8));
	temp2 |= LDT_INT_ENABLE;
	vIoWriteReg(INTERRUPT_CTRL_REG, REG_8, temp2);


	*((LDTPIT_INFO**)ptr) = &localLDTPIT;

	return E__OK;
}

/*****************************************************************************
 * ldtClearInt: LDT PIT Clear Interrupt
 *
 * RETURNS: none
 */

UINT32 ldtClearInt (void *ptr)
{

	UINT8 bTemp = 0;

	bTemp = dIoReadReg(localLDTPIT.bLdtPitControl, REG_8);
	bTemp &=  ~0x10;
	vIoWriteReg(localLDTPIT.bLdtPitControl, REG_8, bTemp);

	return (E__OK);
} /* ldtClearInt () */

/*****************************************************************************
 * ldtSetInt: LDT PIT Set Interrupt
 *
 * RETURNS: none
 */
UINT32 ldtSetInt (void *ptr)
{
	*((UINT8*)ptr) |= 0x80;

	return (E__OK);
}

