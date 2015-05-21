
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

/* random.c - example test function that returns a random pass or fail
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/random.c,v 1.2 2014-04-22 16:36:32 swilson Exp $
 *
 * $Log: random.c,v $
 * Revision 1.2  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.1  2013-09-04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2010/06/29 13:52:07  hchalla
 * Code Cleanup, Added headers and comments.
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
#include <stdio.h>
#include <errors.h>

#include <bit/bit.h>
#include <bit/io.h>
#include <bit/console.h>

/* defines */

#define E__TEST_FAIL	E__BIT + 0x0001


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */

/*****************************************************************************\
 *
 *  TITLE:  RandomTest
 *
 *  ABSTRACT:  test function that randomly returns E__OK or E__FAIL
 *
 * 	RETURNS: E__OK or E__FAIL.
 *
\*****************************************************************************/
TEST_INTERFACE (RandomTest, "Random Test")
{
	UINT8	bSeconds;
	char	achBuffer[80];


	vIoWriteReg (0x0070, REG_8, 0x00);
	bSeconds = (UINT8)dIoReadReg (0x0071, REG_8);

	sprintf (achBuffer, "Seconds count = %02d\n", bSeconds);
	vConsoleWrite (achBuffer);

	if (bSeconds & 0x01)	/* fail if odd */
		return (E__TEST_FAIL);
	else
		return (E__OK);

} /* RandomTest */



