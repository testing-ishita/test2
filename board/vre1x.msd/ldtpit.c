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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/ldtpit.c,v 1.1 2014-03-04 10:25:18 mgostling Exp $
 * $Log: ldtpit.c,v $
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.5.2.1  2012-11-05 15:55:03  hchalla
 * Added support to test 5 tr 80x in a 3U VPX daisy chained backplane.
 *
 * Revision 1.5  2011-11-21 08:49:26  hchalla
 * LDT PIT Enabling the corrected interrupt bit.
 *
 * Revision 1.4  2011/11/14 17:18:29  hchalla
 * Added new test for tr803 idt reading temperature test and other updates.
 *
 * Revision 1.3  2011/10/27 15:47:34  hmuneer
 * no message
 *
 * Revision 1.2  2011/08/26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
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
 * brdLdtPitInfo: enables the LDT PIT interrupt and returns LDTPIT_INFO
 * RETURNS: E__OK */
UINT32 brdLdtPitInfo (void *ptr)
{
	UINT8 bTemp;

	/* Enable LDT PIT IRQ*/
	bTemp = dIoReadReg(localLDTPIT.bLdtPitControl, REG_8);
	bTemp |= 0x80;
	vIoWriteReg(localLDTPIT.bLdtPitControl, REG_8, bTemp);

	*((LDTPIT_INFO**)ptr) = &localLDTPIT;

	return E__OK;
}


/*****************************************************************************
 * brdLdtPitIntEnable: Enable LDT PIT Interrupt
 *
 * RETURNS: E__OK
 */

UINT32 brdLdtPitIntEnable(void *ptr)
{
	// set LDT interrupt enable bit
	*((UINT8*)ptr) |= 0x80;

	return E__OK;
}


