#ifndef __port_io_h__
	#define __port_io_h__

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

/* port_io.h - I/O port access functions (port_io.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/port_io.h,v 1.1 2013-09-04 07:40:42 chippisley Exp $
 *
 * $Log: port_io.h,v $
 * Revision 1.1  2013-09-04 07:40:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2008/05/14 09:59:08  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
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

extern void sysOutPort8 (UINT16 wPort, UINT8 bData);
extern void sysOutPort16 (UINT16 wPort, UINT16 wData);
extern void sysOutPort32 (UINT16 wPort, UINT32 dData);

extern UINT8 sysInPort8 (UINT16 wPort);
extern UINT16 sysInPort16 (UINT16 wPort);
extern UINT32 sysInPort32 (UINT16 wPort);

/* forward declarations */


#endif

