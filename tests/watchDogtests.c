
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/watchDogtests.c,v 1.8 2015-03-17 10:44:22 hchalla Exp $
 * $Log: watchDogtests.c,v $
 * Revision 1.8  2015-03-17 10:44:22  hchalla
 * Changed watchdog timeout to 100millisecond from 10 seconds due to FPGA change.
 *
 * Revision 1.7  2015-03-10 15:49:49  mgostling
 * Fixed compiler warnings.
 *
 * Revision 1.6  2015-03-05 11:49:45  hchalla
 * Added logic to Clear IOCHK# request in 0x61 register.
 *
 * Revision 1.5  2015-02-27 17:20:20  hchalla
 * Clearing IOCHK# bit in 0x61, then disabling NMI and Uninstalling watchdog
 * timer interrupt handler.
 *
 * Revision 1.4  2015-02-25 18:05:19  hchalla
 * Added support for new VP B1x watchdog timer support which is purely based on
 * FPGA.
 *
 * Revision 1.3  2015-01-29 09:56:31  mgostling
 * Add board services to allow access to the LPC superio watchdog.
 * Fix compiler warning messages.
 *
 * Revision 1.2  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments.
 *       This tag was also designed to allow search+sort operations that output a list of functions and test names;
 *       if the tag is used in comments then we get spurious lines included in this list.
 *
 * Revision 1.1  2013-09-04 07:46:46  chippisley
 * Import files into new source repository.
 *
 * Revision 1.15  2012/09/14 15:38:46  jthiru
 * CUTE Support for VX91X - C5043B1110, fixes to existing bugs
 *
 * Revision 1.14  2011/12/01 13:45:13  hchalla
 * Updated for VX 541 board.
 *
 * Revision 1.13  2011/11/14 17:34:09  hchalla
 * Updated for PP 531.
 *
 * Revision 1.12  2011/06/07 14:53:15  hchalla
 * Added #ifdef DEBUG for debug prints.
 *
 * Revision 1.11  2011/05/11 12:17:17  hchalla
 * TP 702 PBIT Related changes.
 *
 * Revision 1.10  2011/03/22 14:07:07  hchalla
 * Implemented watchdog board services to avoid board specific register usage.
 *
 * Revision 1.9  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.8  2010/06/29 09:25:00  hchalla
 * Code Cleanup, Added headers and comments.
 *
 * Revision 1.7  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.6  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.5  2009/05/29 14:06:37  hmuneer
 * Error Codes Fixed
 *
 * Revision 1.4  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.2  2009/05/20 14:33:28  hmuneer
 * no message
 *
 * Revision 1.1  2009/05/18 09:25:30  hmuneer
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
#include <bit/interrupt.h>

#include <bit/board_service.h>



#define WD_ACTION_INT		0		/* set watchdog to generate an interrupt */
#define WD_ACTION_RST		1		/* set watchdog to reset the board */
#define WD_ACTION_INT_RST	2		/* the WD output is used to generated an interrupt via external logic */

#define WD_PAT_MASK			0x03
#define WD_PAT_1			0x02
#define WD_PAT_2			0x01
#define WD_ACTION_MASK		0x04
#define WD_ACTION_NMI		0x00
#define WD_ACTION_RST2		0x04
#define WD_STATUS			0x08
#define WD_HW_DISABLE		0x10
#define WD_SW_ENABLE		0x20
#define WD_HW_ENABLE		0x08

#define WTD_ACTION_ERROR			(E__BIT + 0x01)
#define WTD_SWITCH_ERROR			(E__BIT + 0x02)
#define WTD_NMI_ERROR				(E__BIT + 0x04)
#define WTD_RESET_ERROR				(E__BIT + 0x04)

//#define DEBUG

void EnableWatchdog(UINT8 timeoutAction );
void DisableWatchdog(void );
static UINT32 xwWdgTestWrapper (UINT8 type);


static WATCHDOG_INFO *wDogInfo;
static UINT16 wdAddress;
static int     vector ;

/*****************************************************************************\
 *
 *  TITLE:  WatchDogIntTest
 *
 *  ABSTRACT:  Entry point for the WatchDogIntTest
 *
 * 	RETURNS: E__OK, E__FAIL
 *
\*****************************************************************************/
TEST_INTERFACE (WatchDogIntTest, "Watchdog Interrupt Test")
{
	UINT32   rt = E__OK;
#ifdef DEBUG
	char buffer[80];
#endif

	board_service(SERVICE__BRD_GET_WATCHDOG_INFO, NULL, &wDogInfo);

#ifdef DEBUG
	sprintf(buffer, "wDogInfo.bWdogType:%s\n", wDogInfo->bWdogType == SIO ? "SIO" : "CPLD/FPGA");
	vConsoleWrite (buffer);
	sprintf(buffer, "wDogInfo.wWdogIoPort:%#x\n", wDogInfo->wWdogIoPort);
	vConsoleWrite (buffer);
	sprintf(buffer, "wDogInfo.bIntPin:%#x\n", wDogInfo->bIntPin);
	vConsoleWrite (buffer);
	sprintf(buffer, "wDogInfo.bWdogEnBit:%#x\n", wDogInfo->bWdogEnBit);
	vConsoleWrite (buffer);
#endif
	board_service(SERVICE__BRD_STORE_CONFIG_CHANGES, NULL, NULL);

	board_service(SERVICE__BRD_ENABLE_WD_LPC_IOWINDOW, NULL, NULL);
	board_service(SERVICE__BRD_WATCHDOG_DISABLE, NULL, NULL);

	board_service(SERVICE__BRD_WATCHDOG_ENABLE, NULL, NULL);

	rt = xwWdgTestWrapper(WD_ACTION_INT);

	board_service(SERVICE__BRD_RESTORE_CONFIG_CHANGES, NULL, NULL);
	return rt;
}

/*****************************************************************************\
 *
 *  TITLE:  WatchDogRstTest
 *
 *  ABSTRACT:  Entry point for the WatchDogRstTest
 *
 * 	RETURNS: E__OK, E__FAIL
 *
\*****************************************************************************/
TEST_INTERFACE (WatchDogRstTest, "Watchdog Reset Test")
{
	UINT32  rt = E__OK;
#ifdef DEBUG
	char buffer[80];
#endif


	board_service(SERVICE__BRD_GET_WATCHDOG_INFO, NULL, &wDogInfo);

#ifdef DEBUG
	sprintf(buffer, "wDogInfo.wWdogIoPort:%#x\n", wDogInfo->wWdogIoPort);
	vConsoleWrite (buffer);
	sprintf(buffer, "wDogInfo.bIntPin:%#x\n", wDogInfo->bIntPin);
	vConsoleWrite (buffer);
	sprintf(buffer, "wDogInfo.bWdogEnBit:%#x\n", wDogInfo->bWdogEnBit);
	vConsoleWrite (buffer);
#endif
	board_service(SERVICE__BRD_WATCHDOG_DISABLE, NULL, NULL);
	board_service(SERVICE__BRD_WATCHDOG_ENABLE, NULL, NULL);
	rt = xwWdgTestWrapper(WD_ACTION_RST);

	return rt;
}

/*****************************************************************************\
 *
 *  TITLE:  DisableWatchdog ()
 *
 *  ABSTRACT:  Disable the watchdog by writing into the registers.
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/

void DisableWatchdog(void )
{

	if (wDogInfo->bWdogType == SIO)
	{
		vIoWriteReg(wdAddress + 0x65, REG_8, 0x00 );
		vIoWriteReg(wdAddress + 0x67, REG_8, 0x00 );
		vIoWriteReg(wdAddress + 0x68, REG_8, 0x00 );
		vIoWriteReg(wdAddress + 0x66, REG_8, 0x00 );
	}
	else
	{

		board_service(SERVICE__BRD_WATCHDOG_DISABLE, NULL, NULL);
		vIoWriteReg(0x61, REG_8,  dIoReadReg(0x61, REG_8) |(0x08));	/* clear IOCHK# */
		vIoWriteReg(0x61, REG_8,  dIoReadReg(0x61, REG_8) & ~(0x08));	/* clear IOCHK# */
		vIoWriteReg(0x70, REG_8, 0x80);	/* disable NMI */
	}
}


/*****************************************************************************\
 *
 *  TITLE:  EnableWatchdog ()
 *
 *  ABSTRACT:  Enable the watch-dog by writing into the appropriate registers.
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void EnableWatchdog(UINT8 timeoutAction )
{


	if (wDogInfo->bWdogType == SIO)
	{
		vIoWriteReg(0x2e, REG_8, 0x55 );
		vIoWriteReg(0x2e, REG_8, 0x07 );
		vIoWriteReg(0x2f, REG_8, 0x0a );
		vIoWriteReg(0x2e, REG_8, 0x60 );
		wdAddress = dIoReadReg(0x2f, REG_8);
		wdAddress = wdAddress << 8;

		vIoWriteReg(0x2e, REG_8, 0x61 );
		wdAddress |= dIoReadReg(0x2f, REG_8);

		vIoWriteReg(wdAddress + 0x65, REG_8, 0x80 );
		vIoWriteReg(wdAddress + 0x67, REG_8, 0x70 );
		vIoWriteReg(wdAddress + 0x68, REG_8, 0x00 );
		vIoWriteReg(wdAddress + 0x66, REG_8, 0x02 );
	}

	if(timeoutAction == 0x04)
	{

		if (wDogInfo->bWdogType == SIO)
		{
			vIoWriteReg(wdAddress + 0x66, REG_8, 0x04 );
			vIoWriteReg(wdAddress + 0x47, REG_8, 0x0c );
		}

		if (wDogInfo->bWdogType == CPLD_FPGA)
		{
			board_service(SERVICE__BRD_WATCHDOG_ENABLE_RESET, NULL, NULL);
		}
		else
		{
			board_service(SERVICE__BRD_WATCHDOG_ENABLE_RESET, NULL, NULL);
		}
#ifdef DEBUG
		vConsoleWrite ("Going for System Reset\n")  ;
#endif
	}
	else
	{
#ifdef DEBUG
		vConsoleWrite ("Going for NMI\n")  ;
#endif
		board_service(SERVICE__BRD_WATCHDOG_ENABLE_NMI, NULL, NULL);
	}
}


static UINT32 watchdogFlag;
static UINT32 wDogINT;

/*****************************************************************************\
 *
 *  TITLE:  watchdogHandler ()
 *
 *  ABSTRACT:  Watch-dog interrupt handler handles the watch-dog interrupt and sets
 *             watch-dog flag and disable the watch-dog.
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
static void watchdogHandler (void)
{


	watchdogFlag = 1;
	DisableWatchdog();
	return;
}


static void wDogNMIHandler (void)
{
	wDogINT = 1;
	DisableWatchdog();
	return;
}


/*****************************************************************************\
 *
 *  TITLE:  setupInt ()
 *
 *  ABSTRACT:  configure the interrupt vector and install the watch-dog handler
 *  			for handling the interrupt handler.
 *
 * 	RETURNS: E__OK or E__FAIL.
 *
\*****************************************************************************/
static UINT32 setupInt(void)
{
#if 0
/* This section is not commented and makes test fail in VX91X
 hence hash iffed to 0 */
	if (wDogInfo->bIntPin == 0x2)
	{
			if (sysInstallUserHandler (2, watchdogHandler) != 0)
			{
			   vConsoleWrite ("ERR: sysInstallUserHandler() Failed !!!\n")  ;
			   return E__FAIL ;
			}
	}
	else
#endif
	{
	 		   if ((vector = sysPinToVector (wDogInfo->bIntPin, SYS_IOAPIC0)) == -1)
			   {
				   vConsoleWrite ("ERR: sysPinToVector (7) Failed !!!\n") ;
				    return E__FAIL ;
			    }

			    if (sysInstallUserHandler (vector, watchdogHandler) != 0)
			    {
			       vConsoleWrite ("ERR: sysInstallUserHandler() Failed !!!\n")  ;
			       return E__FAIL ;
			    }

			    /* Now, Unmask the Pin... */
			    if (sysUnmaskPin (wDogInfo->bIntPin, SYS_IOAPIC0) == -1)
			    {
			       vConsoleWrite ("ERR: sysUnmaskPin() Failed !!!\n") ;
			       sysInstallUserHandler (vector, 0) ;
			       return E__FAIL ;
			    }
	}

	    return E__OK ;
}


/*****************************************************************************\
 *
 *  TITLE:  removeInt ()
 *
 *  ABSTRACT:  de-configure the interrupt vector and un-install the watch-dog handler
 *  			for handling the interrupt handler.
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
static void removeInt(void)
{
	watchdogFlag = 0;
#if 0
/* This section is not commented and makes test fail in VX91X
 hence hash iffed to 0 */
	if (wDogInfo->bIntPin == 0x2)
	{
			sysInstallUserHandler (2, 0);
	}
	else
#endif
	{
		    sysMaskPin (wDogInfo->bIntPin, SYS_IOAPIC0) ;
			sysInstallUserHandler (vector, 0);
	}
}


/*****************************************************************************\
 *
 *  TITLE:  xwWdgTestWrapper ()
 *
 *  ABSTRACT:  Performs both WatchDog Tests.
 *
 * 	RETURNS: WTD_SWITCH_ERROR,WTD_NMI_ERROR,WTD_RESET_ERROR or E__OK
 *
\*****************************************************************************/
static UINT32 xwWdgTestWrapper (UINT8 type)
{
	UINT32	rt = E__OK, i;
	UINT32	dServRet = E__FAIL;
	UINT8	bPort61;
	PCI_PFA	pfa;
	UINT16	wAcpiBase;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	UINT16	GPI_disable_temp;
#pragma GCC diagnostic pop
#ifdef DEBUG
	char buffer[64];
	UINT8 bTemp = 0;
#endif


	watchdogFlag = 0;

	if((type != WD_ACTION_RST) && (type != WD_ACTION_INT))
		return (WTD_ACTION_ERROR);



	board_service(SERVICE__BRD_WATCHDOG_CHECK_SWITCH, &dServRet, NULL);
	if(dServRet == E__FAIL)
	{
		vConsoleWrite ("The Watchdog Switch is set to Disabled\n");
		return (WTD_SWITCH_ERROR);
	}

	if(type == WD_ACTION_INT)
	{

		EnableWatchdog( 0x00 );


		if (wDogInfo->bWdogType == CPLD_FPGA)
		{
				wDogINT = 0;

				if (sysInstallUserHandler (wDogInfo->bIntPin, wDogNMIHandler) != 0)
				{
					return E__FAIL;
				}


				vIoWriteReg(0x70, REG_8, 0x80);	/* disable NMI */
				bPort61 = dIoReadReg (0x61, REG_8) & 0x07;
				vIoWriteReg (0x61, REG_8, bPort61 | 0x08);
				vIoWriteReg (0x61, REG_8, bPort61);
				vIoWriteReg (0x70, REG_8, 0);	/* enable NMI (in RTC) */


				pfa = PCI_MAKE_PFA (0, 31, 0);
				wAcpiBase = 0xFFFE & PCI_READ_WORD (pfa, 0x40);

				/* Disable NMI to SMI */
				GPI_disable_temp = dIoReadReg (wAcpiBase + 0x68, REG_16);
				vIoWriteReg (wAcpiBase + 0x68, REG_16, 0x0800);

	#ifdef DEBUG
			vConsoleWrite ("WatchDog running for 10 Seconds\n");
	#endif
			//for(i =0; i< 100; i++)
			for(i =0; i< 2; i++)
			{
				vDelay(100);
				board_service(SERVICE__BRD_WATCHDOG_PAT, NULL, NULL);
			}
		}
		else
		{
			if(setupInt() != E__OK)
			{
				vConsoleWrite ("INT Setup failed\n");
				return (WTD_NMI_ERROR);
			}

#ifdef DEBUG
		vConsoleWrite ("WatchDog running for 2 Seconds\n");
#endif
					for(i =0; i< 20; i++)
					{
						vDelay(100);
						board_service(SERVICE__BRD_WATCHDOG_PAT, NULL, NULL);
					}
		}


#ifdef DEBUG
		bTemp = dIoReadReg(wDogInfo->wWdogIoPort, REG_8);
		sprintf(buffer,"Watchdog IO Port : %#x\n",bTemp);
		vConsoleWrite(buffer);
		vConsoleWrite ("INT in 1 Second\n");
#endif

		if (wDogInfo->bWdogType == CPLD_FPGA)
		{
				for(i =0; i< 10; i++)
				{
					vDelay(100);
					if (wDogINT)
						break;
				}
#ifdef DEBUG
				sprintf(buffer,"Watchdog INT : %#x\n",wDogINT);
				vConsoleWrite(buffer);
#endif
				if(!wDogINT)
				{
					vConsoleWrite ("WatchDog INT Test Failed\n");
					rt = (WTD_NMI_ERROR);
				}

				vIoWriteReg(0x61, REG_8,  dIoReadReg(0x61, REG_8) |(0x08));	/* clear IOCHK# */
				vIoWriteReg(0x61, REG_8,  dIoReadReg(0x61, REG_8) & ~(0x08));	/* clear IOCHK# */
				vIoWriteReg(0x70, REG_8, 0x80);	/* disable NMI */

				if (sysInstallUserHandler (wDogInfo->bIntPin, 0) != 0)
				{
								 return E__FAIL;
				}
		}
		else
		{
			for(i =0; i< 24; i++)
			{
				vDelay(100);
				if (watchdogFlag)
					break;
			}

			if(watchdogFlag != 1)
			{
				vConsoleWrite ("WatchDog INT Test Failed\n");
				rt = (WTD_NMI_ERROR);
			}
			removeInt();
		}


		DisableWatchdog();
	}
	else
	{
		EnableWatchdog( 0x04 );
		if (wDogInfo->bWdogType == CPLD_FPGA)
		{
#ifdef DEBUG
			vConsoleWrite ("WatchDog Reset in 10 Seconds\n");
#endif
			for(i =0; i< 150; i++)
			{
				vDelay(100);
			}
		}
		else
		{
			vConsoleWrite ("WatchDog running for 3 Seconds\n");
			for(i =0; i< 30; i++)
			{
				vDelay(100);
				board_service(SERVICE__BRD_WATCHDOG_PAT, NULL, NULL);
			}
			for(i =0; i< 30 ; i++) /* 30 */
			{
						vDelay(100);
			}
		}

		if (wDogInfo->bWdogType == CPLD_FPGA)
		{

		}
		else
		{
			vConsoleWrite ("WatchDog Reset Test Failed\n");
			DisableWatchdog();
			board_service(SERVICE__BRD_WATCHDOG_DISABLE, NULL, NULL);
		}

		rt = (WTD_RESET_ERROR);
	}

	// taken from INDIA CUTE archive
	if (wDogInfo->bWdogType == SIO)
	{
		vIoWriteReg(0x2e, REG_8, 0x55 );
		vIoWriteReg(0x2e, REG_8, 0x07 );
		vIoWriteReg(0x2f, REG_8, 0x0a );
		vIoWriteReg(0x2e, REG_8, 0x60 );
		vIoWriteReg(0x2f, REG_8, 0xc);
	}

	return rt;
}

