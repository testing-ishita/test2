
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

/* superio.c - Super I/O related tests
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/SpeedSteptests.c,v 1.5 2014-09-19 09:34:29 mgostling Exp $
 * $Log: SpeedSteptests.c,v $
 * Revision 1.5  2014-09-19 09:34:29  mgostling
 * Check if TURBO is supported on CPU before trying to access MSR
 *
 * Revision 1.4  2013-12-10 12:12:29  mgostling
 * Updated speedstep test for TRB1x.
 *
 * Revision 1.3  2013-11-25 12:51:37  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.2  2013-10-08 07:13:39  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:45  chippisley
 * Import files into new source repository.
 *
 * Revision 1.12  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.11  2011/06/07 14:52:17  hchalla
 * Added board service function for vx813 to set the speed step ratio.
 *
 * Revision 1.10  2011/04/05 13:16:17  hchalla
 * Fixed Speed step problem with PP 712, the register offsets were incorrect for MSR's.
 *
 * Revision 1.9  2011/03/22 14:01:44  hchalla
 * Added support for speed stepping for new processor Sandy bridge.
 *
 * Revision 1.8  2011/02/28 11:59:20  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.7  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.6  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.5  2010/11/04 17:57:14  hchalla
 * Added PROCHOT detection when the processor got heated up displays temperature and returns error.
 *
 * Revision 1.4  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.3  2009/05/29 14:06:24  hmuneer
 * Error Codes Fixed
 *
 * Revision 1.2  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
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
#include <bit/msr.h>
 
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <private/cpu.h>
#include <private/debug.h>
#include <bit/board_service.h>

/* defines */
#undef DEBUG
//#define DEBUG
#define SPEEDSTEP_ERROR				(E__BIT + 0x1000)
#define SPEEDSTEP_PROCHOT_ERROR		(E__BIT + 0x1010)


UINT32 perfNehalemSpeedStp( void )
{
	UINT32 msrHigh=0,msrLow=0;
	UINT8 currentRatio;
	PCI_PFA pfa;
	UINT32 dHardwarePciId;
	UINT32 dTestStatus = 0;
	CPUID_REGS sRegs;

#ifdef DEBUG
	char	achBuffer[128];
	UINT8 maxRatio=0,minRatio=0;
#endif

	pfa = PCI_MAKE_PFA (0, 0, 0);
	dHardwarePciId = dPciReadReg(pfa, 0x00, REG_32);
	if( (dHardwarePciId != 0x00448086) &&			// 1st Gen Core CPU
		(dHardwarePciId != 0x01048086) &&			// 2nd Gen Core CPU
		(dHardwarePciId != 0x01548086) &&			// 3rd Gen Core CPU
		(dHardwarePciId != 0x0C048086) )			// 4th Gen Core CPU
	{
		return 1;
	}

	/*Check if the Intel Speed step is enabled*/
	vReadMsr(0x1a0,&msrHigh,&msrLow);

	if (!(msrLow & 0x10000))
	{
		#ifdef DEBUG
			sprintf ( achBuffer, "Enhanced Speed step is not enabled\n");
			vConsoleWrite (achBuffer);
		#endif
		return SPEEDSTEP_ERROR;
	}

	// check if TURBO is supported by this CPU
	sysCpuid (6, (void*) &sRegs);
	if ((sRegs.eax & 0x00000002) != 0)
	{
		/*Check if turbo is enabled and if yes then bail out*/
		if (!(msrHigh & 0x40))
		{
			#ifdef DEBUG
				sprintf ( achBuffer, "Turbo is enabled hence bailing out\n");
				vConsoleWrite (achBuffer);
			#endif
			return SPEEDSTEP_ERROR;
		}
	}

	/*Enable hardware all co-ordination*/
	vReadMsr(0x1aa,&msrHigh,&msrLow);
	msrLow &= ~0x1;
	vWriteMsr(0x1aa,msrHigh,msrLow);

	#ifdef DEBUG
		sprintf ( achBuffer, "Min Ratio=%x Max Ratio=%x\n",minRatio,maxRatio);
		vConsoleWrite (achBuffer);
	#endif

	vReadMsr(0x198,&msrHigh,&msrLow);
	currentRatio = ((msrLow >> 8) & 0xff);

	board_service(SERVICE__BRD_SET_INTEL_SPEED_STEP, NULL, &dTestStatus);

	//set current ratio and check
	vReadMsr(0x199,&msrHigh,&msrLow);
	msrLow &= 0xffff00ff;
	msrLow |= (currentRatio << 8) & 0xff00;
	vWriteMsr(0x199,msrHigh,msrLow);

	return dTestStatus;
}

TEST_INTERFACE ( SpeedStepTest, "Intel Enhanced Speed Step Test" )
{
	UINT32	rt = E__OK;
//#ifdef DEBUG
	char	achBuffer[128];
//#endif
	UINT32	dMsrM, dMsrL;
	UINT32	msrHigh=0,msrLow=0;
	UINT16	P0_Ratio, PN_1_Ratio, Pn_Ratio;
	UINT16	P0_VID,	PN_1_VID, Pn_VID;
#ifdef DEBUG
	float	FSB_Clk;
#endif
	UINT8	n_states, state1, state2;
	float	a, b, c, d, e;
	int		n;


	/*Speed step steps are different for Nehalem core*/
#if 0
#ifdef DEBUG
		sprintf(achBuffer,"\nSpeed Step CPU Temperature:%d\n", getCPUTemp());
		vConsoleWrite(achBuffer);
		sysDebugWriteString(achBuffer);
#endif
#endif
	rt = perfNehalemSpeedStp();
	if ( rt != 1 )
	{
		return rt;
	}
	
	vReadMsr (0x198, &dMsrM, &dMsrL);	// Perf Status	
	P0_Ratio	= (UINT16)(dMsrM >> 8)  & 0x001F;
	PN_1_Ratio	= (UINT16)(dMsrL >> 24) & 0x001F;
	P0_VID 		= (UINT16)(dMsrM)       & 0x003F;	
	vReadMsr (0x19D, &dMsrM, &dMsrL);	// Perf Status
	PN_1_VID 	= (UINT16)(dMsrL) & 0x003F;

#ifdef DEBUG
		sprintf ( achBuffer, "Max bus ratio = %d\tMin bus ratio = %d\n", P0_Ratio, PN_1_Ratio);
		vConsoleWrite (achBuffer);
		sprintf ( achBuffer, "Max VID = %d\t\tMin VID = %d\n", P0_VID, PN_1_VID);
		vConsoleWrite (achBuffer);
#endif

#ifdef DEBUG
	vReadMsr (0xCD, &dMsrM, &dMsrL);	// System Bus Clock Status
	if ((dMsrL & 0x0000000fL) == 0x00000003)
		FSB_Clk = 166.66;
	else
		FSB_Clk = 133.33;

		sprintf ( achBuffer, "System Bus Clock =%dMHz\n", (int)FSB_Clk);// %3.2f
		vConsoleWrite (achBuffer);
#endif

	if(P0_Ratio%2 == 0)
		n_states = ((P0_Ratio - PN_1_Ratio) / 2) + 1;
	else
		n_states = ((P0_Ratio - PN_1_Ratio) / 2) + 2;
		
#ifdef DEBUG
		sprintf ( achBuffer, "Operating Points =%d\n", n_states);
		vConsoleWrite (achBuffer);
		vConsoleWrite ("\nEnabling Speedstep\n");	
#endif

	vReadMsr (0x1a0, &dMsrM, &dMsrL);	// System Bus Clock Status	
	dMsrL |= 0x00010000;
	vWriteMsr (0x1a0, dMsrM, dMsrL);	// System Bus Clock Status

	//must check every thing of something is wrong return SPEEDSTEP_ERROR
	rt = E__OK;
	for (n = 0; n <=  (n_states-1) && (rt == E__OK); n++)
	{
		if(n == (n_states-1))
		{
			if(P0_Ratio%2 == 0)
				Pn_Ratio = (n * 2) + PN_1_Ratio;
			else
				Pn_Ratio = ((n * 2)-1) + PN_1_Ratio;
		}
		else
			Pn_Ratio = (n * 2) + PN_1_Ratio;	

		a = Pn_Ratio - PN_1_Ratio;
		b = P0_VID - PN_1_VID;
		c = P0_Ratio;
		d = PN_1_Ratio;
		e = PN_1_VID;

		Pn_VID = (int) /*ceil*/ ( (a * (b/(c - d))) + e);
	
		vReadMsr (0x2A, &dMsrM, &dMsrL);	// System Bus Clock Status
		state1 = (UINT8)(dMsrL >> 22);
		state1 &= 0x1f;

#ifdef DEBUG
			sprintf ( achBuffer, "\nCurrent Ratio =%d\nSwitching to: ", state1);
			vConsoleWrite( achBuffer );
			sprintf ( achBuffer,"Ratio=%3d, VID=%3d, f=%5dMHz, V=%5dmV\t 0x%04X\n",
					  Pn_Ratio, Pn_VID, (int)(Pn_Ratio * FSB_Clk), (Pn_VID * 16) + 700, (Pn_Ratio << 8) | Pn_VID);
			vConsoleWrite( achBuffer );
#endif

		vWriteMsr (0x199, 0x00, (Pn_Ratio << 8) | Pn_VID);	// change System Bus Clock Status
		vReadMsr (0x198, &dMsrM, &dMsrL);	// Perf Status
		while((dMsrL & 0x00030000) == 0x00030000)
		{
			vReadMsr (0x198, &dMsrM, &dMsrL);	// Perf Status	
			vDelay(1);
		}
		vDelay(1);

		vReadMsr (0x2A, &dMsrM, &dMsrL);	// System Bus Clock Status
		state2 = (UINT8)(dMsrL >> 22);
		state2 &= 0x1f;
		if(state2 == Pn_Ratio)
		{
#ifdef DEBUG
				sprintf ( achBuffer, "Switch complete [%d]\n\n ",state2);
				vConsoleWrite (achBuffer);
#endif
			rt = E__OK;
		}
		else
		{
			#ifdef DEBUG
				sprintf( achBuffer, "Switch Failed to [%d], current [%d]\n\n ",Pn_Ratio, state2);
				vConsoleWrite(achBuffer);
			#endif
			vReadMsr(0x19C,&msrHigh,&msrLow);
			if ((msrLow & 0x02) == 0x02)
			{
				sprintf ( achBuffer, "Therm STS=%x:%x\n",msrHigh,msrLow);
				sysDebugWriteString(achBuffer);
				return (SPEEDSTEP_PROCHOT_ERROR);
			}
			rt = SPEEDSTEP_ERROR + n;
		}
	}
	return rt;
}





