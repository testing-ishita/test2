
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
 #include <bit/io.h>
#include <bit/board_service.h>

/* serial port info for standard COM1 & COM2 ports */

SERIALBAUD_INFO  localSerialBaudInfo[]  = {
					{115200, 1,  50,  15},
					{56000,  2,  80,  45},
					{38400,  3, 115,  70},
					{19200,  6, 230, 140},
					{9600,  12, 460, 290}
				   };

/* serial port info for the Quad UART ports */

SERIALBAUD_INFO  localSerialBaudInfoQuad[]  = {
					{115200, 8,  50,  15},
					{56000, 16,  80,  45},
					{38400, 24, 115,  70},
					{19200, 48, 230, 140},
					{9600,  96, 460, 290}
				   };

UINT32 brdSerialBaudInfo (void *ptr)
{
	/* check we have valid signature and return the relevant port info */
	/* If dBaudHigh == 0xAA55 then the port number is in dBaudLow      */
	
	if (((*((SERIALBAUD_INFO**)ptr))->dBaudHigh == 0xAA55) &&
			((*((SERIALBAUD_INFO**)ptr))->dBaudLow > 2))
	{
		/* port number > 2 so return info for the Quad UART */
		*((SERIALBAUD_INFO**)ptr) = localSerialBaudInfoQuad;
	}		
	else
	{
		/* port 1 or 2 so return info for standard ports */
		*((SERIALBAUD_INFO**)ptr) = localSerialBaudInfo;
	}		

	return E__OK;
}

UINT32 vEnRS485_hd(void *ptr)
{
	(void)ptr;

	vIoWriteReg(0x31E, REG_8, 0x43); /* Half Duplex RS485, termination disabled */

	return E__OK;
}


UINT32 vEnRS485_fd(void *ptr)
{
	(void)ptr;

	vIoWriteReg(0x31E, REG_8, 0x41); /* Full Duplex RS485, termination disabled */

	return E__OK;
}


UINT32 vDisRS485(void *ptr)
{
	(void)ptr;

	vIoWriteReg(0x31E, REG_8, 0x0); /* RS232 mode */

	return E__OK;
}

