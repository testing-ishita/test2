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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx91x.01x/ldtpit.c,v 1.1 2014-05-15 09:42:03 jthiru Exp $
 * $Log: ldtpit.c,v $
 * Revision 1.1  2014-05-15 09:42:03  jthiru
 * Adding vx91x board sources
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.4  2011/11/21 11:15:59  hmuneer
 * no message
 *
 * Revision 1.3  2011/10/27 15:48:23  hmuneer
 * no message
 *
 * Revision 1.2  2011/03/22 13:26:36  hchalla
 * None
 *
 * Revision 1.1  2011/02/28 11:57:43  hmuneer
 * no message
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
						POLL
};


/*****************************************************************************
 * brdLEDinfo: returns the LED_INFO global data structure
 * RETURNS: SIO_Info* */
UINT32 brdLdtPitInfo (void *ptr)
{
	UINT8 temp2 = 0;
	/* Enable LDT PIT Interrupt */
	temp2 = ((UINT8)dIoReadReg(0x21c, REG_8));
	temp2 &= ~0x80;
	vIoWriteReg(0x21c, REG_8, temp2);

	*((LDTPIT_INFO**)ptr) = &localLDTPIT;

	return E__OK;
}


