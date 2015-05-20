
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

/* smp.c - smp processor tests.
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/smp.c,v 1.2 2014-04-22 16:36:32 swilson Exp $
 *
 * $Log: smp.c,v $
 * Revision 1.2  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.1  2013-09-04 07:46:45  chippisley
 * Import files into new source repository.
 *
 * Revision 1.6  2011/03/22 14:02:45  hchalla
 * Added support for new processor SMP.
 *
 * Revision 1.5  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.4  2010/09/14 10:02:21  cdobson
 * Changed application processor code to set result flag instead of calling
 * vConsoleWrite. vConsoleWrite calls FreeRTOS code which causes the
 * FreeRTOS to be run on both the boot and application processors.
 *
 * Revision 1.3  2010/06/29 14:24:09  hchalla
 * Code Cleanup, Added headers and comments.
 *
 * Revision 1.2  2009/06/10 13:58:10  swilson
 * Fix error codes - should be hex. Fix incorect error message formatting/text. Update comments.
 *
 * Revision 1.1  2009/06/08 16:03:59  swilson
 * Improved version of the 'Core Execution' test that
 *  actually executes code on each core when the test is run.
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
#include <errors.h>
#include <bit/board_service.h>
#include <bit/bit.h>
#include <bit/msr.h>

#include <bit/console.h>
#include <bit/delay.h>
#include <bit/interrupt.h>

/* defines */

#define E__CPU_COUNT	(E__BIT + 0x00000100)		/* incorect number of cores during SMP init */
#define E__SW_INIT		(E__BIT + 0x00000101)		/* failure installing S/W handler */
#define E__SEND_IPI		(E__BIT + 0x00000110)		/* failure sending IPI to AP (core 1-15) */
#define E__AP_TASK		(E__BIT + 0x00000120)		/* AP task not called (core 1-15) */
#define E__AP_BOUNCE	(E__BIT + 0x00000130)		/* AP task called too many times (core 1-15) */

/* typedefs */

/* constants */

/* locals */

static	int		iApFlag;
static	int		iApStatus;

/* globals */

/* externals */

extern UINT8	bCpuCount;
extern UINT8	abApicId[];

/* forward declarations */

extern void vApTask (void);


/*****************************************************************************\
 *
 *  TITLE:  CPUExec
 *
 *  ABSTRACT:  Check detected number of cores and execute code on each AP core.
 *
 * 	RETURNS: E__OK, E__ error code.
 *
\*****************************************************************************/
TEST_INTERFACE (CPUExec, "CPU Core Execution")
{
	UINT8	bExpectedCores;
	UINT8	bCore;
	int		iTimeout;
	int		iStatus;
	char	achBuffer[80];


	/* Check number of detected cores against expected value for board */
	board_service(SERVICE__BRD_GET_NUM_CORES, NULL, &bExpectedCores);

	if (bCpuCount != bExpectedCores)
	{
		sprintf (achBuffer, "Wrong number of CPU cores: got %d, expected %d\n", 
				(int)bCpuCount, (int)bExpectedCores);
		vConsoleWrite (achBuffer);

		return (E__CPU_COUNT);
	}

	sprintf (achBuffer, "Board contains %d cores.\n", (int)bCpuCount);
	vConsoleWrite (achBuffer);


	/* Chain our AP task onto a Software Interrupt */

	iStatus = sysInstallUserHandler (SW_VECTOR_1, vApTask);

	if (iStatus != 0)
	{
		vConsoleWrite ("Error installing software interrupt handler\n");
		return (E__SW_INIT);
	}


	/* For each AP in turn, execute the AP task */

	for (bCore = 1; bCore < bCpuCount; bCore++)
	{
		iApFlag   = 0;
		iApStatus = -1;

		sprintf (achBuffer, "AP #%d: ", (int)bCore);
		vConsoleWrite (achBuffer);

		/* Send a fixed IPI to the AP's APIC */

		iStatus = sysSendIpi (abApicId[bCore], SW_VECTOR_1);

		if (iStatus != 0)
		{
			vConsoleWrite ("Error sending IPI to core\n");
			return (E__SEND_IPI + bCore);
		}
	
		/* Wait for the AP task to be called */

		iTimeout = 0;

		while ((iApFlag == 0) && (iTimeout < 100))
		{
			vDelay (100);
			iTimeout++;
		}

		if (iApFlag == 0)
		{
			vConsoleWrite ("Timeout - AP task not called\n");
			return (E__AP_TASK + bCore);
		}

		/* Check for AP task called once only */

		if (iApFlag > 1)
		{
			sprintf (achBuffer, "AP task called %d times (should be once)\n", iApFlag);
			vConsoleWrite (achBuffer);

			return (E__AP_BOUNCE + bCore);
		}

		if (iApStatus == 0)
			vConsoleWrite ("OK\n");
	}

	return (E__OK);

} /* PassingTest */



/*****************************************************************************\
 *
 *  TITLE:  vApTask ()
 *
 *  ABSTRACT:  Trivial AP task.
 *
 * 	RETURNS: NONE.
 *
\*****************************************************************************/
void vApTask (void)
{

#if 0 //just testing code for speedstep
	UINT32 msrHigh=0,msrLow=0;
	UINT8 maxRatio=0,minRatio=0;

		//#ifdef DEBUG
	    char	achBuffer[128];
	    //#endif


	    /*Check if the Intel Speed step is enabled*/
	    vReadMsr(0x1a0,&msrHigh,&msrLow);

	    if( !(msrLow & 0x10000) )
	    {
			sprintf ( achBuffer, "Enhanced Speed step is not enabled\n");
			vConsoleWrite (achBuffer);
	     }
	    else
	    {
	    	sprintf ( achBuffer, "Enhanced Speed step is enabled\n");
	    	vConsoleWrite (achBuffer);
	    }

	    /*Check if turbo is enabled and if yes then bail out*/
	    if(!(msrHigh & 0x40))
	    {
			sprintf ( achBuffer, "Turbo is enabled \n");
			vConsoleWrite (achBuffer);
	    }
	    else
	    {
	    	sprintf ( achBuffer, "Turbo not enabled \n");
	    	vConsoleWrite (achBuffer);
	    }

	    /*Enable hardware all co-ordination*/
	     vReadMsr(0x1aa,&msrHigh,&msrLow);
	     sprintf ( achBuffer, "hardware all co-ordination %x \n", msrLow);
	     vConsoleWrite (achBuffer);
	     msrLow &= ~0x1;
	     msrLow |= 0x400000;
	     vWriteMsr(0x1aa,msrHigh,msrLow);

	    /*Get the min & max ratio supported*/
	    vReadMsr(0xce,&msrHigh,&msrLow);

	    maxRatio = (msrLow>>8) & 0xff;
	    minRatio = (msrHigh>>8) & 0xff;

		sprintf ( achBuffer, "Min Ratio=%x Max Ratio=%x\n",minRatio,maxRatio);
		vConsoleWrite (achBuffer);

		vReadMsr(0x199,&msrHigh,&msrLow);
		sprintf ( achBuffer, "Perf CTL=%x:%x\n",msrHigh,msrLow);
		vConsoleWrite (achBuffer);

		vReadMsr(0x198,&msrHigh,&msrLow);
		sprintf ( achBuffer, "Perf STS=%x:%x\n",msrHigh,msrLow);
		vConsoleWrite (achBuffer);
#endif

	iApFlag++;
	iApStatus = 0;

} /* vApTask () */

