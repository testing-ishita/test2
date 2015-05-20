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

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/ldtpit.c,v 1.1 2015-03-17 10:37:55 hchalla Exp $
 *
 * $Log: ldtpit.c,v $
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
 * Revision 1.1  2013-09-04 07:13:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/io.h>
 
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
	UINT8 bTemp = 0;

	/* Enable LDT PIT IRQ*/
	bTemp = ((UINT8)dIoReadReg(0x21c, REG_8));
	bTemp &= ~0x80;
	vIoWriteReg(0x21c, REG_8, bTemp);
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

		 bTemp = dIoReadReg(0x21C, REG_8);
         bTemp = ( bTemp | (0x10) );
	    vIoWriteReg(0x21C, REG_8, bTemp);

	    return (E__OK);
} /* ldtClearInt () */
