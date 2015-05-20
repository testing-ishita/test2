
/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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

/* interrupt.c - test CPU interrupt handling chain
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/interrupt.c,v 1.2 2013-11-25 11:51:40 mgostling Exp $
 *
 * $Log: interrupt.c,v $
 * Revision 1.2  2013-11-25 11:51:40  mgostling
 * Relaxed interrupt timings.
 *
 * Revision 1.1  2013-09-04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2011/11/14 17:34:29  hchalla
 * Updated for PP 531.
 *
 * Revision 1.7  2011/08/02 16:58:50  hchalla
 * Modified CPU interrupt test mechanism, checked in on behalf of Haroon.
 *
 * Revision 1.6  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.5  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.4  2010/03/31 16:27:10  hmuneer
 * no message
 *
 * Revision 1.3  2009/09/29 12:30:31  swilson
 * Corrections to commenting.
 *
 * Revision 1.2  2009/05/20 13:18:13  swilson
 * Function names containing 'IRQ' replaced with 'pin' for consistency with other code.
 *
 * Revision 1.1  2009/05/20 12:11:39  swilson
 * CPU interrupt test.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>

#include <bit/bit.h>
#include <bit/delay.h>
#include <bit/console.h>
#include <bit/io.h>


/* defines */

//#define DEBUG

/* Error codes */
#define E__NO_INTERRUPT		(E__BIT + 0x00000010)
#define E__SLOW_INTERRUPT	(E__BIT + 0x00000011)
#define E__FAST_INTERRUPT	(E__BIT + 0x00000012)

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */
extern unsigned long xTaskGetTickCount( void );
/* forward declarations */



/*****************************************************************************
 * InterruptTest: test CPU interrupt handling via APIC/IOAPIC
 *
 * RETURNS: E__OK or E__... error code
 */

TEST_INTERFACE (InterruptTest, "CPU Interrupt Test")
{
	UINT32	dStatus ;
#ifdef DEBUG
	char	achBuffer[80];
#endif
	unsigned long ticks1, ticks2, result;

	ticks1 = xTaskGetTickCount( );
	vDelay(100);	//wait 10ms, should have 10 ticks
	ticks2 = xTaskGetTickCount( );

	result = ticks2 - ticks1;
	if(ticks1 == ticks2)
		dStatus = E__NO_INTERRUPT;
	else if (result >= 95 && result <= 105)
		dStatus = E__OK;
	else if (result < 95)
		dStatus = E__SLOW_INTERRUPT;
	else
		dStatus = E__FAST_INTERRUPT;

#ifdef DEBUG
	sprintf (achBuffer, "Ticks1 %x, Ticks2 %x\n", ticks1, ticks2);
	vConsoleWrite (achBuffer);
	sysDebugWriteString (achBuffer);
#endif

	return dStatus;

} /* InterruptTest */

