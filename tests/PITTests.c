
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

/* PITTest.c - PIT Tests.
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/PITTests.c,v 1.2 2014-04-22 16:36:32 swilson Exp $
 * $Log: PITTests.c,v $
 * Revision 1.2  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.1  2013-09-04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.4  2010/06/29 13:35:49  hchalla
 * Code Cleanup, Added headers and comments.
 *
 * Revision 1.3  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.2  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.1  2009/05/18 09:16:38  hmuneer
 * no message
 *
 * Revision 1.0  Feb 9 2009
 *
 *
 */


/* includes */
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>	
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>


/* defines */
//#define DEBUG

#define PIT_COUNTER0_ERROR		(E__BIT + 0x01)
#define PIT_COUNTER1_ERROR		(E__BIT + 0x02)
#define PIT_COUNTER2_ERROR		(E__BIT + 0x03)

/* typedefs */
/* constants */
/* locals */
/* globals */
/* externals */
/* forward declarations */


/*****************************************************************************\
 *
 *  TITLE:  PITTest
 *
 *  ABSTRACT:  Entry point for PITTest, which tests the PIT counters by
 *  		   reading writing the PIT registers.
 *
 * 	RETURNS: E__OK or E__ error code.
 *
\*****************************************************************************/
TEST_INTERFACE (PITTest, "PIT Basic Test")
{
	UINT32 rt = E__OK, count = 0;
	UINT8  temp2, done = 0;
	UINT16 val;

	#ifdef DEBUG
		char   achBuffer[128];
	#endif

	
	//check counter 0
	//------------------------------------------
	vIoWriteReg(0x43, REG_8, 0x38);
	vIoWriteReg(0x40, REG_8, 0xfe);
	vIoWriteReg(0x40, REG_8, 0xff);
	count = 0;
	done = 0;
	while(done == 0)
	{
		vIoWriteReg(0x43, REG_8, 0x00);
		temp2 = ((UINT8)dIoReadReg(0x40, REG_8));
		val   = ((UINT8)dIoReadReg(0x40, REG_8));
		val   = val << 8;
		val  |= temp2;
		if(val == 0x0000)
			done = 1;
		count++;

	}
	if(count > 0)
		rt = E__OK;
	else
		rt  = PIT_COUNTER0_ERROR;
		

	//check counter 1
	//------------------------------------------
	if(rt == E__OK)
	{
		vIoWriteReg(0x43, REG_8, 0x78);
		vIoWriteReg(0x41, REG_8, 0xfe);
		vIoWriteReg(0x41, REG_8, 0xff);
		count = 0;
		done = 0;
		while(done == 0)
		{
			vIoWriteReg(0x43, REG_8, 0x00);
			temp2 = ((UINT8)dIoReadReg(0x41, REG_8));
			val   = ((UINT8)dIoReadReg(0x41, REG_8));
			val   = val << 8;
			val  |= temp2;
			if(val == 0x0000)
				done = 1;
			count++;

		}
		if(count > 0)
			rt = E__OK;
		else
			rt  = PIT_COUNTER1_ERROR;
	}

	//check counter 2
	//------------------------------------------
	if(rt == E__OK)
	{
		vIoWriteReg(0x43, REG_8, 0xb8);
		vIoWriteReg(0x42, REG_8, 0xfe);
		vIoWriteReg(0x42, REG_8, 0xff);
		count = 0;
		done = 0;
		while(done == 0)
		{
			vIoWriteReg(0x43, REG_8, 0x00);
			temp2 = ((UINT8)dIoReadReg(0x42, REG_8));
			val   = ((UINT8)dIoReadReg(0x42, REG_8));
			val   = val << 8;
			val  |= temp2;
			if(val == 0x0000)
				done = 1;
			count++;

		}
		if(count > 0)
			rt = E__OK;
		else
			rt  = PIT_COUNTER2_ERROR;
	}

	return (rt);
}




