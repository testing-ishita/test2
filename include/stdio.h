#ifndef __stdio_h__
	#define __stdio_h__

/************************************************************************
 *                                                                      *
 *      Copyright 2008 Concurrent Technologies, all rights reserved.    *
 *                                                                      *
 *      The program below is supplied by Concurrent Technologies        *
 *      on the understanding that no responsibility is assumed by       *
 *      Concurrent Technologies for any stdio contained therein.       *
 *      Furthermore, Concurrent Technologies makes no commitment to     *
 *      update or keep current the program code, and reserves the       *
 *      right to change its specifications at any time.                 *
 *                                                                      *
 *      Concurrent Technologies assumes no responsibility either for    *
 *      the use of this code or for any infringements of the patent     *
 *      or other rights of third parties which may result from its use  *
 *                                                                      *
 ************************************************************************/

/* stdio.h - console I/O routines
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/stdio.h,v 1.1 2013-09-04 07:28:50 chippisley Exp $
 *
 * $Log: stdio.h,v $
 * Revision 1.1  2013-09-04 07:28:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2008/09/17 13:51:06  swilson
 * Add function to retrieve cursor's 'X' coordinate.
 *
 * Revision 1.2  2008/05/12 14:53:55  swilson
 * Add usefull, non-standard functions.
 *
 * Revision 1.1  2008/04/25 15:22:23  swilson
 * New C-library modules. Basic structure, little content.
 *
 */


/* includes */

#include <stdtypes.h>


/* defines */

#define EOF			(-1)	/* end-of-file (error) */


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern int sprintf (char* achBuffer, const char* achFormat, ...);

extern int getx (void);

extern int putchar (int c);
extern int getchar (void);

extern int puts (char* s);
extern int cputs (char* s);
extern char* gets (char* s);

extern int kbhit (void);

/* forward declarations */


#endif

