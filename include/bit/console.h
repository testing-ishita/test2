#ifndef __console_h__
	#define __console_h__

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

/* mem.h - direct memory access functions (mem.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/console.h,v 1.1 2013-09-04 07:35:26 chippisley Exp $
 *
 * $Log: console.h,v $
 * Revision 1.1  2013-09-04 07:35:26  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/06/20 12:16:58  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.1  2008/05/22 16:04:57  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 */


/* includes */

#include <stdtypes.h>
#include <string.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern void vConsoleWrite (char* achMessage);


/* forward declarations */


#endif

