
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

/* stdlib.c - miscellaneous support routines
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/stdlib.c,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: stdlib.c,v $
 * Revision 1.1  2013-09-04 07:17:58  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2008/05/15 15:32:40  swilson
 * Add build and version tracking. Implement & test missing c-library functions.
 *
 * Revision 1.2  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/04/25 15:22:23  swilson
 * New C-library modules. Basic structure, little content.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdlib.h>
#include <ctype.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * atoi: converts a string to an 'int' type
 *
 * Leading whitespace will be ignored. 
 * Terminates conversion on first non-digit.
 *
 * RETURNS: converted value, 0 on error (invalid string)
 */

int atoi
(
	char*	s
)
{
	return (int)atol (s);

} /* atoi () */


/*****************************************************************************
 * atol: converts a string to a 'long' type
 *
 * Leading whitespace will be ignored.
 * Terminates conversion on first non-digit.
 *
 * RETURNS: converted value, 0 on error (invalid string)
 */

long atol
(
	char*	s
)
{
	long	lValue = 0;
	long	lMult  = 1;		/* default is positive numbers*/


	/* Strip any leading white-space */

	while ( (*s != NULL) && ((*s == ' ') || (*s == '\t')) )
		s++;

	/* Optional sign character */

	if ((*s == '+') || (*s == '-'))
	{
		lMult = 1 - (*s - '+');		/* gives 1 or -1 */
		s++;
	}

	/* Convert digits */

	while (isdigit (*s))
	{
		lValue = (lValue * 10) + (*s - '0');
		s++;
	}

	lValue *= lMult;

	return (lValue);

} /* atol () */

