
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

/* bit_io.c - BIT I/O port access code
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/io.c,v 1.2 2013-09-26 12:58:11 chippisley Exp $
 *
 * $Log: io.c,v $
 * Revision 1.2  2013-09-26 12:58:11  chippisley
 * Removed obsolete HAL.
 *
 * Revision 1.1  2013/09/04 07:11:17  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/01/11 17:24:36  hchalla
 * New board service function added to the code base and new board VX813.
 *
 * Revision 1.4  2010/06/23 10:47:28  hmuneer
 * CUTE V1.01
 *
 * Revision 1.3  2009/05/15 10:50:14  jthiru
 * Modified READ macros to WRITE macros in vIoWriteReg
 *
 * Revision 1.2  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/04/24 16:02:46  swilson
 * Created BIT Library
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/hal.h>
#include <bit/io.h>
#include <private/port_io.h>
#include "FreeRTOS.h"
#include "semphr.h"
/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */
extern xSemaphoreHandle globalMutexPool[MAX_CUTEBIT_MUTEX];

/* forward declarations */


/*****************************************************************************
 * dIoReadReg: read an I/O register
 *
 * RETURNS: port data, as a UINT32
 */

UINT32 dIoReadReg
(
	UINT16		wPort, 		/* port number */
	REG_SIZE	size		/* register size */
)
{
	UINT32 dData;

	xSemaphoreTake(globalMutexPool[MUTEX_IOACCESS],portMAX_DELAY);
	switch (size)
	{
		case REG_8 :
			dData = (UINT32) sysInPort8( wPort );
			break;

		case REG_16 :
			dData = (UINT32) sysInPort16( wPort );
			break;

		case REG_32 :
			dData = sysInPort32( wPort );
			break;
			
		default: 
			dData = 0;
			break;
	}
	xSemaphoreGive(globalMutexPool[MUTEX_IOACCESS]);

	return dData;

} /* dIoReadReg () */


/*****************************************************************************
 * vIoWriteReg: write an I/O register
 *
 * RETURNS: None
 */

void vIoWriteReg
(
	UINT16		wPort, 		/* port number */
	REG_SIZE	size,		/* register size */
	UINT32		dData		/* data value to write */
)
{
	xSemaphoreTake(globalMutexPool[MUTEX_IOACCESS],portMAX_DELAY);
	switch (size)
	{
		case REG_8 :
			sysOutPort8( wPort, (UINT8) dData );
			break;

		case REG_16 :
			sysOutPort16( wPort, (UINT16) dData );
			break;

		case REG_32 :
			sysOutPort32( wPort, dData );
			break;
			
		default: 
			break;
	}
	xSemaphoreGive(globalMutexPool[MUTEX_IOACCESS]);

} /* vIoWriteReg () */


/*****************************************************************************
 * dIoMaskReadReg: read an I/O register, mask data before returning
 *
 * RETURNS: masked port data, as a UINT32
 */

UINT32 dIoMaskReadReg
(
	UINT16		wPort, 		/* port number */
	REG_SIZE	size,		/* register size */
	UINT32		dAndMask	/* AND mask applied to data before return */
)
{
	UINT32	dTemp;


	dTemp = dIoReadReg (wPort, size);

	return (dTemp & dAndMask);

} /* dIoMaskReadReg () */


/*****************************************************************************
 * dIoRmwReg: read an I/O register, modify contents and write back
 *
 * RETURNS: updated port data, as a UINT32
 */

UINT32 dIoRmwReg
(
	UINT16		wPort,
	REG_SIZE	size,		/* register size */
	UINT32		dAndMask,	/* AND mask applied to read data */
	UINT32		dOrMask		/* OR mask applied before writing data */
)
{
	UINT32	dTemp;


	dTemp = dIoReadReg (wPort, size);
	dTemp &= dAndMask;
	dTemp |= dOrMask;
	vIoWriteReg (wPort, size, dTemp);

	return (dTemp);

} /* dIoRmwReg () */

