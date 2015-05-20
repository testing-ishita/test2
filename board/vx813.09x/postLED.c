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

/* postLED.c - board-specific superio information 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx813.09x/postLED.c,v 1.1 2013-11-28 15:13:54 mgostling Exp $
 * $Log: postLED.c,v $
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011/03/22 13:31:03  hchalla
 * *** empty log message ***
 *
 * Revision
 * board-specific superio support.
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>
#include <bit/io.h>



/* Turn ON the POST LED to indicate a running test*/
UINT32 brdPostON(void *ptr)
{
	UINT8 temp2;
	(void)ptr;

	temp2 = ((UINT8)dIoReadReg(0x61, REG_8));
	temp2 |= 0x03;
	vIoWriteReg(0x61, REG_8, temp2);

	return E__OK;
}


/* Turn OFF the POST LED*/
UINT32 brdPostOFF( void *ptr)
{
	UINT8 temp2;
	(void)ptr;

	temp2 = ((UINT8)dIoReadReg(0x61, REG_8));		
	temp2 &= ~0x03;
	vIoWriteReg(0x61, REG_8, temp2);

	return E__OK;
}

