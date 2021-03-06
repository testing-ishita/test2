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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/postLED.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: postLED.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
 * Import files into new source repository.
 *
 * Revision 1.1  2012/06/22 11:42:01  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */

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

