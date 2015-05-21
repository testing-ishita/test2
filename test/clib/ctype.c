
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


/* ctype.c - implementation of <ctype.h> 'C' library functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/ctype.c,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: ctype.c,v $
 * Revision 1.1  2013-09-04 07:17:58  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 */


/* includes */

#include <ctype.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

	/*
	 *	_U	upper
	 *	_L	lower
	 *	_D	digit
	 *	_C	cntrl
	 *	_P	punct
	 *	_S	white space (space/lf/tab)
	 *	_X	hex digit
	 *	_SP	hard space (0x20)
	 */

unsigned char _ctype[] = 
{
	_C,		_C,		_C,		_C,		_C,		_C,		_C,		_C,		/* 0x00-0x0F */
	_C,		_C|_S,	_C|_S,	_C|_S,	_C|_S,	_C|_S,	_C,		_C,		
	_C,		_C,		_C,		_C,		_C,		_C,		_C,		_C,		/* 0x10-0x1F */
	_C,		_C,		_C,		_C,		_C,		_C,		_C,		_C,		
	_S|_SP,	_P,		_P,		_P,		_P,		_P,		_P,		_P,		/* 0x20-0x2F */
	_P,		_P,		_P,		_P,		_P,		_P,		_P,		_P,		
	_D,		_D,		_D,		_D,		_D,		_D,		_D,		_D,		/* 0x30-0x3F */
	_D,		_D,		_P,		_P,		_P,		_P,		_P,		_P,		
	_P,		_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,		/* 0x40-0x4F */
	_U,		_U,		_U,		_U,		_U,		_U,		_U,		_U,		
	_U,		_U,		_U,		_U,		_U,		_U,		_U,		_U,		/* 0x50-0x5F */
	_U,		_U,		_U,		_P,		_P,		_P,		_P,		_P,		
	_P,		_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,		/* 0x60-0x6F */
	_L,		_L,		_L,		_L,		_L,		_L,		_L,		_L,		
	_L,		_L,		_L,		_L,		_L,		_L,		_L,		_L,		/* 0x70-0x7F */
	_L,		_L,		_L,		_P,		_P,		_P,		_P,		_C,		
	0,		0,		0,		0,		0,		0,		0,		0,		/* 0x80-0x8F */
	0,		0,		0,		0,		0,		0,		0,		0,	
	0,		0,		0,		0,		0,		0,		0,		0,		/* 0x90-0x9F */
	0,		0,		0,		0,		0,		0,		0,		0,
	_S|_SP,	_P,		_P,		_P,		_P,		_P,		_P,		_P,		/* 0xA0-0xAF */
	_P,		_P,		_P,		_P,		_P,		_P,		_P,		_P,
	_P,		_P,		_P,		_P,		_P,		_P,		_P,		_P,		/* 0xB0-0xBF */
	_P,		_P,		_P,		_P,		_P,		_P,		_P,		_P,
	_U,		_U,		_U,		_U,		_U,		_U,		_U,		_U,		/* 0xC0-0xCF */
	_U,		_U,		_U,		_U,		_U,		_U,		_U,		_U,
	_U,		_U,		_U,		_U,		_U,		_U,		_U,		_P,		/* 0xD0-0xDF */
	_U,		_U,		_U,		_U,		_U,		_U,		_U,		_L,
	_L,		_L,		_L,		_L,		_L,		_L,		_L,		_L,		/* 0xE0-0xEF */
	_L,		_L,		_L,		_L,		_L,		_L,		_L,		_L,
	_L,		_L,		_L,		_L,		_L,		_L,		_L,		_P,		/* 0xF0-0xFF */
	_L,		_L,		_L,		_L,		_L,		_L,		_L,		_L
};

/* externals */

/* forward declarations */


/****************************************************************************
 * tolower: convert uppercase characters to lowercase
 *
 * RETURNS: lowercase version of character
 */

int tolower 
(
	int	ch
)
{
	if (isupper (ch))
		ch += 'a' - 'A';

	return (ch);

} /* tolower () */


/****************************************************************************
 * toupper: convert lowercase characters to uppercase
 *
 * RETURNS: uppercase version of character
 */

int toupper 
(
	int	ch
)
{
	if (islower (ch))
		ch -= 'a' - 'A';

	return (ch);

} /* toupper () */

