#ifndef __vsprintf_h__
	#define __vsprintf_h__

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
 *																		*
 *		Portions: Copyright (C) 1991, 1992  Linus Torvalds				*
 *                                                                      *
 ************************************************************************/

/* stdarg.h - variable-list argument handling
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/stdarg.h,v 1.1 2013-09-04 07:28:50 chippisley Exp $
 *
 * $Log: stdarg.h,v $
 * Revision 1.1  2013-09-04 07:28:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 */
 

/* includes */

/* defines */

/* typedefs */

typedef char*	va_list;

/* constants */

/* locals */

/* globals */

/* externals */

#define va_arg(AP, TYPE)		(AP += __va_rounded_size (TYPE), *((TYPE *) (AP - __va_rounded_size (TYPE))))

#define va_start(AP, LASTARG)	(AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))

#define va_end(ap) 				(void)0 

#define __va_rounded_size(TYPE)	(((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))


/* forward declarations */

#endif

