
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

/* RTCTest.c - LED Test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/RTCTests.c,v 1.5 2015-04-02 08:34:01 hchalla Exp $
 * $Log: RTCTests.c,v $
 * Revision 1.5  2015-04-02 08:34:01  hchalla
 * Added delay for reading DATE and MONTH from RTC registers as sometimes
 * this gives a value of zero on some boards.
 *
 * Revision 1.4  2015-04-01 16:48:18  hchalla
 * Added more debug info to debug the tests.
 *
 * Revision 1.3  2015-03-05 11:47:11  hchalla
 * Disabling of NMI in 0x61 ioport register.
 *
 * Revision 1.2  2014-11-10 11:06:17  mgostling
 * Fixed RTC test for month register verification.
 * Do not allow a month value of zero.
 *
 * Revision 1.1  2013-09-04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.16  2011/12/01 13:45:55  hchalla
 * Updated for VX 541 board added IRQ 8 support for RTC interrupt test.
 *
 * Revision 1.15  2011/11/14 17:34:09  hchalla
 * Updated for PP 531.
 *
 * Revision 1.14  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.13  2010/11/12 11:32:55  hchalla
 * RTC BCD to decimal conversion fixes.
 *
 * Revision 1.12  2010/11/04 17:54:52  hchalla
 * Decreased the delay to read the uip bit in the RTC register.
 *
 * Revision 1.11  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.10  2010/03/16 11:47:01  jthiru
 * Day of month test changed
 *
 * Revision 1.9  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.8  2009/07/24 14:06:04  hmuneer
 * V1.02
 *
 * Revision 1.7  2009/06/12 12:35:35  swilson
 * Fixed test names - had TEST, should be Test
 *
 * Revision 1.6  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.5  2009/06/08 09:55:58  hmuneer
 * TPA40
 *
 * Revision 1.4  2009/06/03 13:39:39  hmuneer
 * Loop Bug Fixed
 *
 * Revision 1.3  2009/06/03 09:06:52  hmuneer
 * UIP checks
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
#include <bit/interrupt.h>


/* defines */
//#define DEBUG

#define RTC_SECONDS_ERROR		(E__BIT + 0x01)
#define RTC_MINUTES_ERROR		(E__BIT + 0x02)
#define RTC_HOURS_ERROR			(E__BIT + 0x03)
#define RTC_DAY_ERROR			(E__BIT + 0x04)
#define RTC_MONTH_ERROR			(E__BIT + 0x05)
#define RTC_YEAR_ERROR			(E__BIT + 0x06)
#define RTC_CENTUARY_ERROR		(E__BIT + 0x07)
#define RTC_SECONDS_INCREMENT	(E__BIT + 0x10)

#define RTC_INT_ERROR			(E__BIT + 0x17)

#define osWriteDWord(regAddr,value)		(*(UINT32*)(regAddr) = value)
#define osReadDWord(regAddr)			(*(UINT32*)(regAddr))

/* typedefs */

/* constants */

/* locals */

static int vector;
static int RTCIntFlag = 0;

/* globals */

/* externals */

extern void postErrorMsg(char* buffer);

/* forward declarations */


UINT8 bcdToDecimal(UINT8 hex)
{
	UINT8  temp, temp2;

	temp  = hex & 0x0f;
	if(temp > 0x09)
		return 0xff;

	temp2 = (hex >> 4) & 0x0f;
	if(temp2 > 0x09)
		return 0xff;

	temp += (temp2  * 10);

	return temp;
}

/*****************************************************************************
 * uip:
 *
 * RETURNS: None
 */

UINT32 uip(UINT32 err)
{
	UINT8  uip, tm;

	tm = 255;
	vIoWriteReg(0x70, REG_8, 0x0a);
	uip = ((UINT8)dIoReadReg(0x71, REG_8));
	while( ((uip & 0x80) == 0x80)  && (tm > 0))
	{
		vDelay(1); //was 5
		vIoWriteReg(0x70, REG_8, 0x0a);
		uip = ((UINT8)dIoReadReg(0x71, REG_8));
		tm--;
	}

	if(tm >0)
		return E__OK;
	else
		return err;
}


TEST_INTERFACE (RTCTest, "RTC Basic Test")
{
	UINT32 rt = E__FAIL;
	UINT8  temp1, temp2;

#ifdef DEBUG
	char   achBuffer[128];
#endif
	char buffer[60];



	//check if seconds are ticking over
	//------------------------------------------

	rt = uip(RTC_SECONDS_INCREMENT);
	if(rt == E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0);
		temp1 = ((UINT8)dIoReadReg(0x71, REG_8));
		vDelay(1500);
		rt = uip(RTC_SECONDS_INCREMENT);
		if(rt == E__OK)
		{
			vIoWriteReg(0x70, REG_8, 0);
			temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
			if(temp1 != temp2)
			{
				temp2 = bcdToDecimal(temp2);
				#ifdef DEBUG
					sprintf ( achBuffer, "\nS :%d ", temp2);
					vConsoleWrite (achBuffer);
				#endif

				if(temp2 <= 59)
					rt = E__OK;
				else
				{
					sprintf ( buffer, "\nSeconds Register Value :%d ", temp2);
					postErrorMsg(buffer);
					rt = RTC_SECONDS_ERROR;
				}
			}
			else
				rt = RTC_SECONDS_INCREMENT;
		}
	}

	//check Minutes
	//-------------------------------------------
#ifndef DEBUG
	if(rt == E__OK)
#endif
		rt = uip(RTC_MINUTES_ERROR);

	if(rt == E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x02);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp2 = bcdToDecimal(temp2);

#ifdef DEBUG
		sprintf ( achBuffer, "\nM :%d ", temp2);
		vConsoleWrite (achBuffer);
#endif

		if(temp2 <= 59)
			rt = E__OK;
		else
		{
			rt = RTC_MINUTES_ERROR;
			sprintf ( buffer, "\nMinutes Register Value :%d ", temp2);
			postErrorMsg(buffer);
		}
	}


	//check hours
	//-------------------------------------------
#ifndef DEBUG
	if(rt == E__OK)
#endif
		rt = uip(RTC_HOURS_ERROR);

	if(rt == E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x0b);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		if(temp2 & 0x02)
		{
			vIoWriteReg(0x70, REG_8, 0x04);
			temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
			temp2 = bcdToDecimal(temp2);

#ifdef DEBUG
			sprintf ( achBuffer, "\nH :%d ", temp2);
			vConsoleWrite (achBuffer);
#endif

			if(temp2 <= 23)
				rt = E__OK;
			else
			{
				rt = RTC_HOURS_ERROR;
				sprintf ( buffer, "\nHours Register Value :%d ", temp2);
				postErrorMsg(buffer);
			}
		}
		else
		{
			vIoWriteReg(0x70, REG_8, 0x04);
			temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
			temp2 = bcdToDecimal(temp2);

#ifdef DEBUG
			sprintf ( achBuffer, "\nH :%d ", temp2);
			vConsoleWrite (achBuffer);
#endif

			if(temp2 <= 12)
				rt = E__OK;
			else
			{
				rt = RTC_HOURS_ERROR;
				sprintf ( buffer, "\nHours Register Value :%d ", temp2);
				postErrorMsg(buffer);
			}
		}
	}


	//check Days of month
	//-------------------------------------------
#ifndef DEBUG
	if(rt == E__OK)
#endif
		rt = uip(RTC_DAY_ERROR);

	if(rt == E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x07);
		vDelay(2);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		vDelay(2);
		temp2 = bcdToDecimal(temp2);

#ifdef DEBUG
		sprintf ( achBuffer, "\nDm:%d ", temp2);
		vConsoleWrite (achBuffer);
		sysDebugWriteString(achBuffer);
#endif

		if((temp2 != 0) && (temp2 <= 31))
			rt = E__OK;
		else
		{
			rt = RTC_DAY_ERROR;
			sprintf ( buffer, "\nDay Register Value :%d ", temp2);
			postErrorMsg(buffer);
			sysDebugWriteString(buffer);
		}
#ifdef DEBUG
		sprintf ( achBuffer, "\nReturn value :0x%08X ", temp2);
		vConsoleWrite (achBuffer);
#endif
	}



	//check Months
	//-------------------------------------------
#ifndef DEBUG
	if(rt == E__OK)
#endif
		rt = uip(RTC_MONTH_ERROR);

	if(rt == E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x08);
		vDelay(2);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		vDelay(2);
		temp2 = bcdToDecimal(temp2);

#ifdef DEBUG
		sprintf ( achBuffer, "\nMn:%d ", temp2);
		vConsoleWrite (achBuffer);
		sysDebugWriteString(achBuffer);
#endif

		if((temp2 != 0) && (temp2 <= 12))
			rt = E__OK;
		else
		{
			rt = RTC_MONTH_ERROR;
			sprintf ( buffer, "\nMonth Register Value :%d ", temp2);
			postErrorMsg(buffer);
		}
	}


	//check Year
	//-------------------------------------------
#ifndef DEBUG
	if(rt == E__OK)
#endif
		rt = uip(RTC_YEAR_ERROR);

	if(rt == E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x09);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp2 = bcdToDecimal(temp2);

#ifdef DEBUG
		sprintf ( achBuffer, "\nY :%d ", temp2);
		vConsoleWrite (achBuffer);
		sysDebugWriteString(achBuffer);
#endif

		if(temp2 <= 99)
			rt = E__OK;
		else
		{
			rt = RTC_YEAR_ERROR;
			sprintf ( buffer, "\nYear Register Value :%d ", temp2);
			postErrorMsg(buffer);
		}
	}

	//check Century
	//-------------------------------------------
#ifndef DEBUG
	if(rt == E__OK)
#endif
		rt = uip(RTC_CENTUARY_ERROR);

	if(rt == E__OK)
	{
		vIoWriteReg(0x70, REG_8, 0x32);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp2 = bcdToDecimal(temp2);

#ifdef DEBUG
		sprintf ( achBuffer, "\nC :%d ", temp2);
		vConsoleWrite (achBuffer);
		sysDebugWriteString(achBuffer);
#endif

		if(temp2 == 20)
			rt = E__OK;
		else
		{
			rt = RTC_CENTUARY_ERROR;
			sprintf ( buffer, "\nCentury Register Value :%d ", temp2);
			postErrorMsg(buffer);
		}
	}

	vIoWriteReg(0x70, REG_8, 0x80); // Hari: 02/03/2015
	return (rt);
}




TEST_INTERFACE (RTCIntTest, "RTC Interrupt Test")
{
	UINT8	temp2, state = 0;

#ifdef DEBUG
	char	achBuffer[128];
#endif

	//disable interrupt
	vIoWriteReg(0x70, REG_8, 0x0b);
	temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
	temp2 &= ~0x40;
	vIoWriteReg(0x71, REG_8, temp2);

	//clear interrupt
	vIoWriteReg(0xa0, REG_8, 0x4a);

	//enable interrupt
	vIoWriteReg(0x70, REG_8, 0x0b);
	temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
	temp2 |= 0x40;
	vIoWriteReg(0x71, REG_8, temp2);

	vDelay(5);//wait for seconds register to increment

	//read interrupt
	vIoWriteReg(0xa0, REG_8, 0x0a);
	temp2 = ((UINT8)dIoReadReg(0xa0, REG_8));

	#ifdef DEBUG
		sprintf ( achBuffer, "\n0x%x\n", temp2);
		vConsoleWrite (achBuffer);
#endif

	if(temp2 & 0x01)
		state = 1;
	else
		state = 0;


	if(state == 1)
	{
		//disable interrupt
		vIoWriteReg(0x70, REG_8, 0x0b);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp2 &= ~0x40;
		vIoWriteReg(0x71, REG_8, temp2);

		//clear interrupt
		vIoWriteReg(0xa0, REG_8, 0x4a);
		vIoWriteReg(0xa0, REG_8, 0x0a);

		vDelay(5);

		//read interrupt
		vIoWriteReg(0xa0, REG_8, 0x0a);
		temp2 = ((UINT8)dIoReadReg(0xa0, REG_8));

#ifdef DEBUG
			sprintf ( achBuffer, "\n0x%x\n", temp2);
			vConsoleWrite (achBuffer);
#endif

		if((temp2 & 0x01) == 0x00)
			state = 2;
		else
			state = 0;
	}


	if(state == 2)
	{
		//clear interrupt
		vIoWriteReg(0xa0, REG_8, 0x4a);

		//enable interrupt
		vIoWriteReg(0x70, REG_8, 0x0b);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp2 |= 0x40;
		vIoWriteReg(0x71, REG_8, temp2);

		vDelay(5);

		//read interrupt
		vIoWriteReg(0xa0, REG_8, 0x0a);
		temp2 = ((UINT8)dIoReadReg(0xa0, REG_8));

#ifdef DEBUG
			sprintf ( achBuffer, "\n0x%x\n", temp2);
			vConsoleWrite (achBuffer);
#endif
		if(temp2 & 0x01)
			state = 3;
		else
			state = 0;
	}

	//disable interrupt
	vIoWriteReg(0x70, REG_8, 0x0b);
	temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
	temp2 &= ~0x40;
	vIoWriteReg(0x71, REG_8, temp2);

	vIoWriteReg(0x70, REG_8, 0x80); // Hari: 02/03/2015
	if(state == 3)
		return E__OK;
	else
		return RTC_INT_ERROR;
}

static void RTCIntHandler (void)
{
	RTCIntFlag = 1;
	return;
}

TEST_INTERFACE (RTCIrqIntTest, "RTC IRQ Interrupt Test")
{
	UINT8  temp2;

	//disable interrupt
	vIoWriteReg(0x70, REG_8, 0x0b);
	temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
	temp2 &= ~0x40;
	vIoWriteReg(0x71, REG_8, temp2);

	//clear interrupt
	/*vIoWriteReg(0xa0, REG_8, 0x4a);*/
	RTCIntFlag = 0;

	if ((vector = sysPinToVector (8, SYS_IOAPIC0)) == -1)
	{
		vConsoleWrite ("ERR: sysPinToVector (8) Failed !!!\n") ;
			return E__FAIL ;
	}

	if (sysInstallUserHandler (vector, RTCIntHandler) != 0)
	{
		vConsoleWrite ("ERR: sysInstallUserHandler() Failed !!!\n")  ;
		return E__FAIL ;
	}

		/* Now, Unmask the Pin... */
	if (sysUnmaskPin (8, SYS_IOAPIC0) == -1)
	{
		vConsoleWrite ("ERR: sysUnmaskPin() Failed !!!\n") ;
		sysInstallUserHandler (vector, 0) ;
		return E__FAIL ;
	}

	//enable interrupt
	vIoWriteReg(0x70, REG_8, 0x0b);
	temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
	temp2 |= 0x40;
	vIoWriteReg(0x71, REG_8, temp2);

	vDelay(5);//wait for seconds register to increment

	if(RTCIntFlag != 1)
	{
		vConsoleWrite ("1. RTC INT Test Failed\n");
		return (RTC_INT_ERROR);
	}

	//disable interrupt
	vIoWriteReg(0x70, REG_8, 0x0b);
	temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
	temp2 &= ~0x40;
	vIoWriteReg(0x71, REG_8, temp2);


	if(RTCIntFlag == 1)
	{

		RTCIntFlag = 0;

		//enable interrupt
		vIoWriteReg(0x70, REG_8, 0x0b);
		temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
		temp2 |= 0x40;
		vIoWriteReg(0x71, REG_8, temp2);

		vDelay(5);

		 if(RTCIntFlag != 1)
		{
			vConsoleWrite ("2. RTC INT Test Failed\n");
			return (RTC_INT_ERROR);
		}
	}

	//disable interrupt
	vIoWriteReg(0x70, REG_8, 0x0b);
	temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
	temp2 &= ~0x40;
	vIoWriteReg(0x71, REG_8, temp2);

	sysMaskPin (8, SYS_IOAPIC0) ;
	sysInstallUserHandler (vector, 0);
	return (E__OK);

}




