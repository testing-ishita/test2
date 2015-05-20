#ifndef __io_h__
	#define __io_h__

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

/* io.h - I/O port access functions (bit_io.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/io.h,v 1.1 2013-09-04 07:35:26 chippisley Exp $
 *
 * $Log: io.h,v $
 * Revision 1.1  2013-09-04 07:35:26  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2008/04/24 16:04:38  swilson
 * Updated includes to support HAL and BIT library.
 *
 */


/* includes */

#include <stdtypes.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT32 dIoReadReg (UINT16 wPort, REG_SIZE size);
extern void vIoWriteReg (UINT16 wPort, REG_SIZE size, UINT32 dData);
extern UINT32 dIoMaskReadReg (UINT16 wPort, REG_SIZE size, UINT32 dAndMask);
extern UINT32 dIoRmwReg (UINT16 wPort, REG_SIZE size, UINT32 dAndMask, UINT32 dOrMask);

/* forward declarations */


#endif

