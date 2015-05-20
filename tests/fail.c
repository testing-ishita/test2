
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

/* fail.c - example test function, which always fails
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/fail.c,v 1.1 2013-09-04 07:46:42 chippisley Exp $
 *
 * $Log: fail.c,v $
 * Revision 1.1  2013-09-04 07:46:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 * Revision 1.1  2008/05/16 14:35:26  swilson
 * Create trivial tests for checking test execution.
 *
 */


/* includes */

#include <stdtypes.h>
#include <errors.h>

#include <bit/bit.h>

/* defines */

#define E__TEST_FAIL	E__BIT + 0


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */



/*****************************************************************************
 * FailingTest: test function that always returns an error code
 *
 * RETURNS: None
 */

TEST_INTERFACE (FailingTest, "Failing Test")
{
	return (E__TEST_FAIL);

} /* FailingTest */



