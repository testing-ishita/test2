
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/serialbaud.c,v 1.1 2015-02-25 17:50:45 hchalla Exp $
 *
 * $Log: serialbaud.c,v $
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.1  2012/06/22 11:42:01  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */
#include <stdtypes.h>
#include <bit/bit.h>
 #include <bit/io.h>
#include <bit/board_service.h>

SERIALBAUD_INFO  localSerialBaudInfo[]  = {
					{115200, 1, 50, 10},
					{56000, 2, 80, 50},
					{38400, 3, 115, 70},
					{19200, 6, 230, 150},
					{9600, 12, 460,295}
				   };


UINT32 brdSerialBaudInfo (void *ptr)
{
	*((SERIALBAUD_INFO**)ptr) = localSerialBaudInfo;

	return E__OK;
}

UINT32 vEnRS485_hd(void *ptr)
{
	(void)ptr;

	vIoWriteReg(0x31E, REG_8, 0x0); /* Interface disabled */
	vIoWriteReg(0x31E, REG_8, 0x03); /* Half Duplex RS485 */

	return E__OK;
}


UINT32 vEnRS485_fd(void *ptr)
{
	(void)ptr;

	vIoWriteReg(0x31E, REG_8, 0x0); /* Interface disabled */
	vIoWriteReg(0x31E, REG_8, 0x01); /* Full Duplex RS485 */

	return E__OK;
}


UINT32 vDisRS485(void *ptr)
{
	(void)ptr;

	vIoWriteReg(0x31E, REG_8, 0x0); /* Interface disabled */
	vIoWriteReg(0x31E, REG_8, 0x10); /* RS232 mode */

	return E__OK;
}

