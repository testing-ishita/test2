#ifndef __mem_io_h__
	#define __mem_io_h__

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

/* mem_io.h - direct memory access functions (mem_io.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/mem_io.h,v 1.1 2013-09-04 07:40:42 chippisley Exp $
 *
 * $Log: mem_io.h,v $
 * Revision 1.1  2013-09-04 07:40:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2008/05/14 09:59:08  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
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

extern void sysWriteMem8 (UINT32 dAddr, UINT8 bData);
extern void sysWriteMem16 (UINT32 dAddr, UINT16 wData);
extern void sysWriteMem32 (UINT32 dAddr, UINT32 dData);

extern UINT8 sysReadMem8 (UINT32 dAddr);
extern UINT16 sysReadMem16 (UINT32 dAddr);
extern UINT32 sysReadMem32 (UINT32 dAddr);


/* forward declarations */


#endif

