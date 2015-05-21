
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

/* hal_io.c - BIT I/O port read/write code
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/hal/hal_io.c,v 1.1 2013-09-04 07:26:54 chippisley Exp $
 *
 * $Log: hal_io.c,v $
 * Revision 1.1  2013-09-04 07:26:54  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 * Revision 1.2  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/04/24 16:03:55  swilson
 * Created HAL.
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/hal.h>
#include <private/port_io.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * spIoReadByte: HAL interface to local 8-bit port read
 *
 * RETURNS: None
 */

void spIoReadByte
(
	CCT_IO_REG*	psParams
)
{
	psParams->dData = (UINT32)sysInPort8 ((UINT16)psParams->dPort);

} /* spIoReadByte () */


/*****************************************************************************
 * spIoReadWord: HAL interface to local 16-bit port read
 *
 * RETURNS: None
 */

void spIoReadWord
(
	CCT_IO_REG*	psParams
)
{
	psParams->dData = (UINT32)sysInPort16 ((UINT16)psParams->dPort);

} /* spIoReadWord () */


/*****************************************************************************
 * spIoReadDword: HAL interface to local 32-bit port read
 *
 * RETURNS: None
 */

void spIoReadDword
(
	CCT_IO_REG*	psParams
)
{
	psParams->dData = sysInPort32 ((UINT16)psParams->dPort);

} /* spIoReadDword () */


/*****************************************************************************
 * spIoWriteByte: HAL interface to local 8-bit port Write
 *
 * RETURNS: None
 */

void spIoWriteByte
(
	CCT_IO_REG*	psParams
)
{
	sysOutPort8 ((UINT16)psParams->dPort, (UINT8)psParams->dData);

} /* spIoWriteByte () */


/*****************************************************************************
 * spIoWriteWord: HAL interface to local 16-bit port Write
 *
 * RETURNS: None
 */

void spIoWriteWord
(
	CCT_IO_REG*	psParams
)
{
	sysOutPort16 ((UINT16)psParams->dPort, (UINT16)psParams->dData);

} /* spIoWriteWord () */


/*****************************************************************************
 * spIoWriteDword: HAL interface to local 32-bit port Write
 *
 * RETURNS: None
 */

void spIoWriteDword
(
	CCT_IO_REG*	psParams
)
{
	sysOutPort32 ((UINT16)psParams->dPort, psParams->dData);

} /* spIoWriteDword () */

