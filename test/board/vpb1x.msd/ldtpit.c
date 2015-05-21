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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/ldtpit.c,v 1.1 2015-02-25 17:50:45 hchalla Exp $
 *
 * $Log: ldtpit.c,v $
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.2  2012/07/23 10:45:43  madhukn
 * *** empty log message ***
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
	bTemp = ((UINT8)dIoReadReg(0x22B, REG_8));
	bTemp |= (1<<5);
	vIoWriteReg(0x22B, REG_8, bTemp);

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
         bTemp = ( bTemp |  (0x10) );
	    vIoWriteReg(0x21C, REG_8, bTemp);

	    return (E__OK);
} /* ldtClearInt () */
