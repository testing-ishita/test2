
/************************************************************************
 *                                                                      *
 *      Copyright 2010 Concurrent Technologies, all rights reserved.    *
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

 /* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/enpmcgpio.c,v 1.2 2015-03-10 12:40:52 mgostling Exp $
 * $Log: enpmcgpio.c,v $
 * Revision 1.2  2015-03-10 12:40:52  mgostling
 * Disabled debug definition
 *
 * Revision 1.1  2015-02-03 12:07:28  mgostling
 * Added suppport for ENPMC_OO1
 *
 *
 * Initial version of EN PMC 8619 GPIO test support.
 *
 */

/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>
#include <bit/bitbangI2c.h>
#include <bit/board_service.h>
#include <devices/enpmc8619.h>
#include <private/cpu.h>

/* defines */

//#define DEBUG

#define E__BAD_GPIO_LOOPBACK			E__BIT  + 0x08
#define E__BAD_DIFF_IO_LOOPBACK_DEV0	E__BIT  + 0x09
#define E__BAD_DIFF_IO_LOOPBACK_DEV2	E__BIT  + 0x0A
#define E__NO_ENPMC_SITE_TEST_FUNCTION	E__BIT  + 0x0B
#define	E__NO_ENPMC_SITE_ERR			E__BIT	+ 0x0C
#define	E__NO_ENPMC_GPIO_DATA_BASE    	E__BIT  + 0x0D
#define SWITCH_8619_NEXT_INSTANCES 5
#define vReadByte(regAddr)			(*(UINT8*)(regAddr))#define vReadWord(regAddr)			(*(UINT16*)(regAddr))#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))
#define vWriteByte(regAddr,value)	(*(UINT8*) (regAddr) = value)#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

/* typedefs */

/* locals */

static char	achInfoBuffer[64];
static ENPMC_GPIO_SIGNAL_PAIRS pmcGpioSignals[] ={
	{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}, {10, 11},
	{12, 13}, {14, 15}, {16, 17}, {18, 19}, {20, 21},
	{22, 23}, {24, 25}, {26, 27}, {28, 29}, {30, 31},
	{0xFF, 0xFF}											// end of list
};

// PMC GPIO loopback settings for 64 GPI/O's
static ENPMC_GPIO_LOOPBACK_INFO pmcLoopbackInfo ={
	1,											// plx8619Instance
	&pmcGpioSignals[0],							// gpio table
	0x22222222L,								// PLX PEX819 GPIO_CONFIG1 register: GPIO 0,2,4,6,8,10,12,14 outputs, 1,3,5,7,9,11,13,15 inputs
	0x22222222L,								// PLX PEX819 GPIO_CONFIG2 register: GPIO 16,18,20,22,24,26,28,30 outputs, 17,19,21,23,25,27,29,31 inputs
	0x55555555L,								// bitmap of GPIOs configured as outputs
	0xAAAAAAAAL,								// bitmap of GPIOs configured as inputs
};

/* globals */

/* externals */

/* forward declarations */

/*****************************************************************************
 * enpmc_gpio0_31_test:
 *
 *	Test the PLX PEX8619 GPIO0 to GPIO31 interfaces routed through the PMC
 *	connectors to the RTM module
 *
 * RETURNS: E__OK or error
 */
static UINT32 enpmc_gpio0_31_test
(
	UINT32 dBaseAddr,
	UINT32 dConfig1,
	UINT32 dConfig2,
	const ENPMC_GPIO_SIGNAL_PAIRS  *loopback_table
)
{
	UINT32	dRet = E__OK;
	UINT16	wTemp;
	UINT16	wExpected;
	UINT16	wCount;
	UINT16 	wZeroIn = 0;
	UINT16 	wZeroOut = 0;
	// configure GPIOs in PLX PEX8619
	vWriteDWord (dBaseAddr + PLX_GPIO1_DIR_CTL, dConfig1);
	vWriteDWord (dBaseAddr + PLX_GPIO2_DIR_CTL, dConfig2);
	//Preparing the mask table based on I/O table for moving '0' (zero) test	for (wCount = 0; loopback_table[wCount].out != 0xFF; ++wCount)
	{		wZeroOut = (wZeroOut | (1 << loopback_table[wCount].out));
		wZeroIn = (wZeroIn | (1 << loopback_table[wCount].in));
	}
	// moving 1's:
	for (wCount = 0; loopback_table[wCount].out != 0xFF; ++wCount)
	{
		if (loopback_table[wCount].out < 16)
		{			wTemp = 0;
			wExpected = 0;
			setbit (wTemp, loopback_table[wCount].out);
			vWriteWord (dBaseAddr + PLX_GPIO1_OUTPUT_DATA, wTemp);
			wTemp = vReadWord (dBaseAddr + PLX_GPIO1_INPUT_DATA);
			setbit (wExpected, loopback_table[wCount].in);
			if (wTemp != wExpected)
			{
				dRet = E__BAD_GPIO_LOOPBACK;
				sprintf (achInfoBuffer, "ENPMC GPIO0 to GPIO15 loopback error while wrting ones:\nwrote 0x%04X, expected 0x%04X, read 0x%04X\n",
						bitmask(loopback_table[wCount].out), wExpected, wTemp);
				vConsoleWrite (achInfoBuffer);
				break;
			 }
		}
		else		{
			wTemp = 0;
			wExpected = 0;
			setbit (wTemp, (loopback_table[wCount].out - 16));
			vWriteWord (dBaseAddr + PLX_GPIO2_OUTPUT_DATA, wTemp);
			wTemp = vReadWord (dBaseAddr + PLX_GPIO2_INPUT_DATA);
			setbit (wExpected, (loopback_table[wCount].in - 16));
			if (wTemp != wExpected)
			{
				dRet = E__BAD_GPIO_LOOPBACK;
				sprintf (achInfoBuffer, "ENPMC GPIO16 to GPIO31 loopback error while writing ones:\nwrote 0x%04X, expected 0x%04X, read 0x%04X\n",
						 bitmask(loopback_table[wCount].out - 16), wExpected, wTemp);
				vConsoleWrite (achInfoBuffer);
				break;
			}
		}
	}
	//moving 0's
	if (dRet == E__OK)
	{
		for (wCount = 0; loopback_table[wCount].out != 0xFF; ++wCount)
		{
			if (loopback_table[wCount].out < 16)
			{
				wTemp = 0xFFFF;
				wExpected = 0xFFFF;
				clrbit (wTemp, loopback_table[wCount].out);
				wTemp = wTemp & wZeroOut;
				vWriteWord (dBaseAddr + PLX_GPIO1_OUTPUT_DATA, wTemp);
				wTemp = vReadWord (dBaseAddr + PLX_GPIO1_INPUT_DATA);
				clrbit (wExpected, loopback_table[wCount].in);
				wExpected = wExpected & wZeroIn;
				if (wTemp != (UINT16) wExpected)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achInfoBuffer, "ENPMC GPIO0 to GPIO15 loopback error while writing zeros:\nwrote 0x%04X, expected 0x%04X, read 0x%04X\n",
									0xFFFF & ~(bitmask(loopback_table[wCount].out)), wExpected,wTemp);
					vConsoleWrite (achInfoBuffer);
					break;
				}
			}
			else
			{
				wTemp = 0xFFFF;
				wExpected = 0xFFFF;
				clrbit (wTemp, (loopback_table[wCount].out - 16));
				wTemp = wTemp & wZeroOut;
				vWriteWord (dBaseAddr + PLX_GPIO2_OUTPUT_DATA, wTemp);
				wTemp = vReadWord(dBaseAddr + PLX_GPIO2_INPUT_DATA);
				clrbit (wExpected, loopback_table[wCount].in - 16);
				wExpected = wExpected & wZeroIn;
				if (wTemp != wExpected)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achInfoBuffer, "ENPMC GPIO16 to GPIO31 loopback error while writing zeros:\nwrote 0x%04X, expected 0x%04X, read 0x%04X\n",
									0xFFFF & ~(bitmask(loopback_table[wCount].out - 16)), wExpected, wTemp);
					vConsoleWrite (achInfoBuffer);
					break;
				}
			}
		}
	}
	return dRet;
}

/*****************************************************************************
 * findMemory
 *
 * RETURNS: None
 */
static UINT32 findMemory(PCI_PFA pfa, UINT32 * dMemPtr){
	UINT32	dHandle;
	UINT32	regVal;
	PTR48	tPtr1;
#ifdef DEBUG	char  achDbgBuffer[64];#endif

	// Enable the bus mastering and memory/io space access	regVal = PCI_READ_WORD (pfa,PCI_COMMAND);
	regVal |= 0x06;
	PCI_WRITE_WORD (pfa,PCI_COMMAND,regVal);

	// Get the memory mapped CRG	regVal = PCI_READ_DWORD (pfa, PCI_BAR0);
	regVal &= ~0xf;

#ifdef DEBUG	sprintf (achDbgBuffer, "PCI CRG for ENPMC=%x\n", regVal);	vConsoleWrite (achDbgBuffer);#endif

	dHandle = dGetPhysPtr (regVal, 0x1000, &tPtr1, (void*) dMemPtr);
#ifdef DEBUG	sprintf (achDbgBuffer, "mapped ENPMCRegSpace : 0x%X\n", (UINT32) *dMemPtr);	vConsoleWrite (achDbgBuffer);#endif

	if (dHandle == E__FAIL)	{		vConsoleWrite ("\nNo memory allocated for ENPMC reg space \n");		return E__TEST_NO_MEM;	}

	return dHandle;}

/*****************************************************************************
 * Enpmc_pex8619_gpio: Test the PLX PEX8619 GPIO interfaces routed through the XMC and
 *					   PMC connectors to the RTM module
 *
 * RETURNS: None
 */
TEST_INTERFACE (enpmc_gpio_test, "ENPMC PMC GPIO Test")
{
	UINT32	dRet = E__OK;
	UINT32	dPmcBaseAddress = 0x00;
	UINT32	dInstance;
	UINT32 	mHandle;
	UINT8 	bPmcSite;
	UINT8	bEnpmcVal;
	PCI_PFA pfa;
	pci_DB*	DB;
	static UINT8 (*vChkPXMC)(void);
	ENPMC_GPIO_LOOPBACK_INFO * loopbackInfo;

	loopbackInfo = &pmcLoopbackInfo;
	board_service (SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	vChkPXMC = DB->vChkPXMC;
	if (vChkPXMC != NULL)	{
		bEnpmcVal = (*vChkPXMC)();
	}	else	{
		vConsoleWrite ("No vChkPXMC() function in database to check ENPMC sites\n");
		return E__NO_ENPMC_SITE_TEST_FUNCTION;
	}

	if ((bEnpmcVal & PXMC_PMC1) == PXMC_PMC1)	{		bPmcSite = 1;
	}
	else if ((bEnpmcVal & PXMC_PMC2) == PXMC_PMC2)	{		bPmcSite = 2;
	}	else	{		vConsoleWrite ("No ENPMC Card Inserted\n");
		return E__NO_ENPMC_SITE_ERR;	}

	dInstance = (UINT32) loopbackInfo->plx8619Instance;
	if (iPciFindDeviceById (dInstance, 0x10B5,0x8619,&pfa) != E__OK)	{		return E__FAIL;
	}

	mHandle = findMemory (pfa, &dPmcBaseAddress);
	if (mHandle != E__FAIL)	{
		dRet = enpmc_gpio0_31_test (dPmcBaseAddress, loopbackInfo->dConfig1, loopbackInfo->dConfig2, loopbackInfo->gpioTable);
		vFreePtr (mHandle);
		if (dRet != E__OK)		{			sprintf (achInfoBuffer, "PMC GPIO test failed for ENPMC site = 0x%X \n", bPmcSite);			vConsoleWrite (achInfoBuffer);
			return (dRet);		}	}	else	{		return E__TEST_NO_MEM;	}

	return dRet;
}

