
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

/* ledtest.c - LED Test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/ledtest.c,v 1.1 2013-09-04 07:46:43 chippisley Exp $
 * $Log: ledtest.c,v $
 * Revision 1.1  2013-09-04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.3  2009/06/12 10:58:08  hmuneer
 * LED test added
 *
 * Revision 1.2  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.1  2009/05/18 09:16:39  hmuneer
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
 
#include <bit/board_service.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>


/* defines */
//#define DEBUG

/* typedefs */
/* constants */
/* locals */
/* globals */
/* externals */
/* forward declarations */


/*****************************************************************************
 * LEDTest: Test the user LED
 *
 * RETURNS: None
 */
TEST_INTERFACE (LEDTest, "LED Test")
{
	UINT8 temp2, i;
	LED_INFO  *localLEDs;

	board_service(SERVICE__BRD_GET_LED_INFO, NULL, &localLEDs);

	for(i = 0; i < 5; i++)
	{
		temp2 = ((UINT8)dIoReadReg(localLEDs->port, REG_8));
		temp2 |= localLEDs->bit;
		vIoWriteReg(localLEDs->port, REG_8, temp2);
		vDelay (150);		
		temp2 &= ~localLEDs->bit;			
		vIoWriteReg(localLEDs->port, REG_8, temp2);		
		vDelay (150);
	}

	/*
	for(i = 0; i < 5; i++)
	{
		//vConsoleWrite ("\n1 ");
		temp2 = ((UINT8)dIoReadReg(0x211, REG_8));
		temp2 |= 0x01;
		vIoWriteReg(0x211, REG_8, temp2);
		//vConsoleWrite (" 2 ");
		vDelay (500);		
		temp2 &= ~0x01;			
		vIoWriteReg(0x211, REG_8, temp2);
		//vConsoleWrite (" 3");
		vDelay (500);
	}
	*/


	return (E__OK);
}




