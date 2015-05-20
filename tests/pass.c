
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

/* pass.c - example test function, which always passes
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/pass.c,v 1.2 2014-04-22 16:36:32 swilson Exp $
 *
 * $Log: pass.c,v $
 * Revision 1.2  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.1  2013-09-04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2010/06/29 13:31:54  hchalla
 * Code Cleanup, Added headers and comments.
 *
 * Revision 1.7  2009/05/15 12:05:10  swilson
 * Remove all the cruft. Test now does only what was intended - returns 'pass'.
 *
 * Revision 1.6  2009/05/15 11:05:25  jthiru
 * Modified to print logaddr
 *
 * Revision 1.5  2009/02/02 17:02:17  jthiru
 * Changes to dGetPhysPtr function call
 *
 * Revision 1.4  2008/07/24 15:09:30  swilson
 * Fix use of system-level console code. Add test of HAL delay.
 *
 * Revision 1.3  2008/06/20 12:16:54  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.2  2008/06/02 10:31:28  swilson
 * Add cctExecuteBit() interface and modify IBIT to use this. Add support for complex
 *  BIT execution expressions and BIT parameters.
 *
 * Revision 1.1  2008/05/16 14:35:26  swilson
 * Create trivial tests for checking test execution.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>		/* bit/errors.h ? or BIT_ERROR macro */

#include <bit/bit.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */



/*****************************************************************************\
 *
 *  TITLE:  PassingTest
 *
 *  ABSTRACT:  test function that always returns E__OK
 *
 * 	RETURNS: E__OK.
 *
\*****************************************************************************/
TEST_INTERFACE (PassingTest, "Passing Test")
{
	return (E__OK);

} /* PassingTest */



