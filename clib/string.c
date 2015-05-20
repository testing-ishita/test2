
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

/* STRING.C - the 'more complex' string functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/string.c,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: string.c,v $
 * Revision 1.1  2013-09-04 07:17:58  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 */


/* includes */

#include <string.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * strpbrk: 
 *
 * RETURNS: None
 */

char* strpbrk
(
	const char* s1,
	const char* s2
)
{
	char*	s = (char*)s1;

	return (s);


} /* strpbrk () */


/*****************************************************************************
 * strspn: 
 *
 * RETURNS: None
 */

size_t strspn
(
	const char* s1,
	const char* s2
)
{
	return (0);

} /* strspn () */


/*****************************************************************************
 * strstr: 
 *
 * RETURNS: None
 */

char* strstr
(
	const char* s1,
	const char* s2
)
{
	char*	s = (char*)s1;

	return (s);

} /* strstr () */


/*****************************************************************************
 * memcpy: 
 * 
 * Replaces version in mem.s which has issues with source/destination 
 * orientation and overlaps.   
 *
 * RETURNS: destination
 */
 
void* memcpy
(
    void *       d,   /* pointer to destination */
    const void * s,   /* pointer to  source */
    size_t       n    /* size of memory to copy */
)
{
	bcopy ((char *) s, (char *) d, (size_t) n);
	return (d);
}
