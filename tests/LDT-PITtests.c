
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/LDT-PITtests.c,v 1.2 2013-11-25 11:54:06 mgostling Exp $
 * $Log: LDT-PITtests.c,v $
 * Revision 1.2  2013-11-25 11:54:06  mgostling
 * Updated for TRB1x
 *
 * Revision 1.1  2013-09-04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.12  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.11  2011/12/01 13:44:58  hchalla
 * Updated for VX 541 board.
 *
 * Revision 1.10  2011/11/14 17:34:09  hchalla
 * Updated for PP 531.
 *
 * Revision 1.9  2011/10/27 15:50:39  hmuneer
 * no message
 *
 * Revision 1.8  2011/08/26 15:50:41  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.7  2011/08/02 17:00:35  hchalla
 * Added support for IO APIC interrupt mode.
 *
 * Revision 1.6  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.5  2010/11/12 11:32:11  hchalla
 * LDT PIT code clean up.
 *
 * Revision 1.4  2010/11/04 17:52:13  hchalla
 * Updated LDT PIT tests to get the LDT IO registers from the board specific information.
 *
 * Revision 1.3  2010/03/31 16:27:10  hmuneer
 * no message
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
#include <bit/interrupt.h>
 
#include <bit/board_service.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>

/* defines */
//#define DEBUG

#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)
#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))

#define E__FAILED_STD_LDT			((E__BIT) + 0x0010U)
#define E__FAILED_STD_PIT_100Hz		((E__BIT) + 0x0011U)
#define E__FAILED_STD_PIT_200Hz		((E__BIT) + 0x0012U)
#define E__FAILED_STD_PIT_500Hz		((E__BIT) + 0x0013U)
#define E__FAILED_STD_PIT_1000Hz	((E__BIT) + 0x0014U)
#define E__FAILED_STD_PIT_2000Hz	((E__BIT) + 0x0015U)
#define E__FAILED_STD_PIT_5000Hz	((E__BIT) + 0x0016U)
#define E__FAILED_STD_PIT_10000Hz	((E__BIT) + 0x0017U)
#define E__FAILED_ENH_LDT_EARLY     ((E__BIT) + 0x0018U)
#define E__PIN_MAP		            ((E__BIT) + 0x0073U)
#define E__UNMASK_SLOT	            ((E__BIT) + 0x0074U)
#define E__MASK_SLOT	            ((E__BIT) + 0x0075U)

#define MODE_INTERRUPT_ENABLE		(0x80U)

#define MODE_MASK					(0x0EU)
#define MODE_PIT_10000Hz			(0x0EU)
#define MODE_PIT_5000Hz				(0x0CU)
#define MODE_PIT_2000Hz				(0x0AU)
#define MODE_PIT_1000Hz				(0x08U)
#define MODE_PIT_500Hz				(0x06U)
#define MODE_PIT_200Hz				(0x04U)
#define MODE_PIT_100Hz				(0x02U)
#define MODE_LDT					(0x00U)

#define MODE_RUN_MASK				(0x01U)
#define MODE_RUN_GO					(0x01U)
#define MODE_RUN_STOP				(0x00U)


#define IRQ_TEST_PASS 0x00
#define IRQ_TEST_FAIL 0x01
#define IRQ_TEST_NONE 0x01

static UINT8 bIrqStatus = IRQ_TEST_NONE;        /* NONE, PASS or FAIL */


static void vLDTIrqPassHandler (void);


#define LDT_OVERFLOWED				( (((UINT8)dIoReadReg(CONTROL_STATUS, REG_8)) & 0x10) == 0x10)
#define LDT_OVERFLOW_RESET			vIoWriteReg (CONTROL_STATUS, REG_8, \
									((UINT8)dIoReadReg(CONTROL_STATUS, REG_8)) & ~0x10)

static UINT8 ldtOverFlow(void);
static void ldtOverflowReset(void);
LDTPIT_INFO* ldtPit;

	const struct
	{
		UINT32 dInitialValue;
		UINT16 wMilliSeconds;
		UINT32 wErrorCode;
		UINT8  bControl;
	}

	asModes [] = /* definition continues ... */
	{
		{0xFFFF0000U, 100, E__FAILED_STD_LDT,         MODE_LDT},
		{         0U,  12, E__FAILED_STD_PIT_100Hz,   MODE_PIT_100Hz},
		{         0U,   6, E__FAILED_STD_PIT_200Hz,   MODE_PIT_200Hz},
		{         0U,   3, E__FAILED_STD_PIT_500Hz,   MODE_PIT_500Hz},
		{         0U,   2, E__FAILED_STD_PIT_1000Hz,  MODE_PIT_1000Hz},
		{         0U,   1, E__FAILED_STD_PIT_2000Hz,  MODE_PIT_2000Hz},
		{         0U,   1, E__FAILED_STD_PIT_5000Hz,  MODE_PIT_5000Hz},
		{         0U,   1, E__FAILED_STD_PIT_10000Hz, MODE_PIT_10000Hz}
	};


static UINT32 wStandard_LDT_PIT_Testing (void)
{	
	UINT32 i;
	UINT32 wTestStatus = E__OK;
#ifdef DEBUG
	char   achBuffer[32];
#endif

	i = 0;
	while ((i < 7) && (wTestStatus == E__OK))
	{
		vIoWriteReg(ldtPit->bLdtPitControl, REG_8, MODE_RUN_STOP);
		vIoWriteReg(ldtPit->bTimerByte0,   REG_8, asModes [i].dInitialValue);
		vIoWriteReg(ldtPit->bTimerByte1,   REG_8, asModes [i].dInitialValue >> 8);
		vIoWriteReg(ldtPit->bTimerByte2,   REG_8, asModes [i].dInitialValue >> 16);
		vIoWriteReg(ldtPit->bTimerByte3,   REG_8, asModes [i].dInitialValue >> 24);
		vIoWriteReg(ldtPit->bLdtPitControl, REG_8, asModes [i].bControl | MODE_RUN_GO);

		vDelay (asModes [i].wMilliSeconds+1);

		if (ldtOverFlow() == 0x10)
		{
			ldtOverflowReset();
		}
		else
		{
			wTestStatus = asModes [i].wErrorCode;
		}

		vIoWriteReg(ldtPit->bLdtPitControl, REG_8, MODE_RUN_STOP);
		i++;
	}

	return wTestStatus;
}

/********************************************************************
 * vLDTClearInt
 *
 * Clear the LDT hardware interrupt enable and status bits
 ********************************************************************/
static void vLDTClearInt (void)
{
	board_service(SERVICE__BRD_LDT_CLEAR_INTR, NULL, NULL);
}

/********************************************************************
 * vIrqPassHandler
 *
 * Interrupt handler for correct IRQ.
 * Set PASS flag and clears hardware interrupt
 ********************************************************************/
static void vLDTIrqPassHandler (void)
{
	vLDTClearInt();
	ldtOverflowReset(); /*Hari*/
    bIrqStatus = IRQ_TEST_PASS;
    vIoWriteReg(ldtPit->bLdtPitControl, REG_8, MODE_RUN_STOP);
    return;
}


static UINT32 wInt_LDT_PIT_Testing (void)
{	
	UINT32 i, tm,j;
	UINT32 wTestStatus = E__OK;
	UINT8  temp1;
	int		iVector= -1;
	UINT8 irqNum= 0;
	UINT8  bTemp;

	#ifdef DEBUG
		char   achBuffer[32];
	#endif
	

	i = 0;
	while ((i < 7) && (wTestStatus == E__OK))
	{


		 bIrqStatus = IRQ_TEST_NONE;

		if (ldtPit->bIrqMode == 1)
		{
			irqNum = 5;
			/* Find vector used by system for chosen IOAPIC pin */
			iVector = sysPinToVector (irqNum, SYS_IOAPIC0);

			if (iVector == -1)
				return (E__PIN_MAP);

			sysInstallUserHandler (iVector, vLDTIrqPassHandler);

			if (sysUnmaskPin (irqNum, SYS_IOAPIC0) == -1)
					return (E__UNMASK_SLOT);

			sysEnableInterrupts();
		}

		vIoWriteReg(ldtPit->bLdtPitControl, REG_8, MODE_RUN_STOP);

		vIoWriteReg(ldtPit->bTimerByte0,   REG_8, asModes [i].dInitialValue);
		vIoWriteReg(ldtPit->bTimerByte1,   REG_8, asModes [i].dInitialValue >> 8);
		vIoWriteReg(ldtPit->bTimerByte2,   REG_8, asModes [i].dInitialValue >> 16);
		vIoWriteReg(ldtPit->bTimerByte3,   REG_8, asModes [i].dInitialValue >> 24);

		bTemp = MODE_RUN_STOP;
		if (ldtPit->bIrqMode == 1)
		{
			// update interrupt enable flag in bTemp for LdtPitControl register
			board_service(SERVICE__BRD_SET_LDTPIT_INTR, NULL, &bTemp);
#ifdef DEBUG
			sprintf ( achBuffer, "Interrupt Enable = %#02X\n", bTemp);
			vConsoleWrite (achBuffer);
#endif
		}
		
		bTemp |= (asModes [i].bControl | MODE_RUN_GO);
		vIoWriteReg(ldtPit->bLdtPitControl, REG_8, bTemp);

#ifdef DEBUG
		sprintf ( achBuffer, "LDT/PIT control = %#02X\n", bTemp);
		vConsoleWrite (achBuffer);
#endif
		if (ldtPit->bIrqMode == 1)
		{

			for (j = 0; j < ((asModes [i].wMilliSeconds+1)); j++)
			{
				vDelay(1);
				if ((i == 0) && (j == 0))
				{
					/* LDT test mode */
					/* Check if we have had an interrupt */
					if (bIrqStatus == IRQ_TEST_PASS)
					{
								/* we got an interrupt, but too early */
							wTestStatus = E__FAILED_ENH_LDT_EARLY;
							break;
					}
				}
			}

#ifdef DEBUG
			sprintf ( achBuffer, "After delay of %dms\n", asModes [i].wMilliSeconds);
			vConsoleWrite (achBuffer);
			sprintf ( achBuffer, "LDT Control = %#02X\n", dIoReadReg(ldtPit->bLdtPitControl, REG_8));
			vConsoleWrite (achBuffer);
			sprintf ( achBuffer, "LDT Overflow = %#02X\n", ldtOverFlow());
			vConsoleWrite (achBuffer);
#endif
			if (ldtOverFlow() == 0x10)
			{
				ldtOverflowReset();
			}
			else
			{
				wTestStatus = asModes [i].wErrorCode;
			}

			if (bIrqStatus == IRQ_TEST_PASS)
			{

#ifdef DEBUG
				sprintf ( achBuffer, "\nPASS: Got Interrupt IRQ%d\n", irqNum);
				vConsoleWrite (achBuffer);
#endif
		    	wTestStatus = E__OK;
			}
			else if (bIrqStatus == IRQ_TEST_NONE)
		    {

#ifdef DEBUG
				vConsoleWrite ("In IRQ Mode\n");
				vConsoleWrite ("No interrupt\n");
#endif

  			    /* Mask and disable interrupt and uninstall our interrupt handler */
				if (sysMaskPin (irqNum, SYS_IOAPIC0) == -1)
						return (E__MASK_SLOT);
				sysInstallUserHandler (iVector, 0);
				wTestStatus = asModes [i].wErrorCode;
			}

			if (sysMaskPin (irqNum, SYS_IOAPIC0) == -1)
				 return (E__MASK_SLOT);

			sysInstallUserHandler (iVector, 0);
			vIoWriteReg(ldtPit->bLdtPitControl, REG_8, MODE_RUN_STOP);
		}
		else
		{

			tm = 150;

			do
			{
				//read interrupt
				vIoWriteReg(0x20, REG_8, 0x0a);
				temp1 = ((UINT8)dIoReadReg(0x20, REG_8));
				tm--;
				vDelay(1);
			}while( ((temp1 & 0x20) != 0x20) && (tm > 0));

			vDelay(asModes [i].wMilliSeconds); /*Hari*/

			if((temp1 & 0x20) == 0x20)
			{
				#ifdef DEBUG
					sprintf ( achBuffer, "\nInterrupt wait %d\n", tm);					vConsoleWrite (achBuffer);
				#endif

				if (ldtOverFlow() == 0x10)
					ldtOverflowReset();
				else
					wTestStatus = asModes [i].wErrorCode;
			}
			else
			{
				#ifdef DEBUG
					vConsoleWrite ("No interrupt\n");
					vConsoleWrite ("In POLL Mode\n");
				#endif
				wTestStatus = asModes [i].wErrorCode;
				ldtOverflowReset();
			}
		}
	 i++;
  }

	return wTestStatus;
}





TEST_INTERFACE ( LdtBasicTest, "LDT/PIT Basic Test" )
{
	UINT32   rt = E__OK;

#ifdef DEBUG
	char achBuffer[80];
#endif

	board_service(SERVICE__BRD_GET_LDPIT_INFO, NULL, &ldtPit);

	if (ldtPit != NULL)
	{
#ifdef DEBUG
		sprintf ( achBuffer, "\nldtPit->bLdtPitControl %x\n",ldtPit->bLdtPitControl);
		vConsoleWrite (achBuffer);
		sprintf ( achBuffer, "\nldtPit->bTimerByte0 %x\n",ldtPit->bTimerByte0);
		vConsoleWrite (achBuffer);
		sprintf ( achBuffer, "\nldtPit->bTimerByte1 %x\n",ldtPit->bTimerByte1);
		vConsoleWrite (achBuffer);
		sprintf ( achBuffer, "\nldtPit->bTimerByte2 %x\n",ldtPit->bTimerByte2);
		vConsoleWrite (achBuffer);
		sprintf ( achBuffer, "\nldtPit->bTimerByte3 %x\n",ldtPit->bTimerByte3);
		vConsoleWrite (achBuffer);
#endif
		rt = wStandard_LDT_PIT_Testing ();
	}

	return rt;
}


TEST_INTERFACE ( LdtIntTest, "LDT/PIT Interrupt Test" )
{
	UINT32   rt = E__OK;

	board_service(SERVICE__BRD_GET_LDPIT_INFO, NULL, &ldtPit);

	if (ldtPit != NULL)
		rt = wInt_LDT_PIT_Testing ();

	return rt;
}



static UINT8 ldtOverFlow(void)
{
	return (((UINT8)dIoReadReg(ldtPit->bLdtPitControl, REG_8)) & 0x10);
}

static void ldtOverflowReset(void)
{
	vIoWriteReg (ldtPit->bLdtPitControl, REG_8,((UINT8)dIoReadReg(ldtPit->bLdtPitControl, REG_8)) & ~0x10);
}
