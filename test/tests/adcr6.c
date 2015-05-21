
/************************************************************************
 *                                                                      *
 *      Copyright 2013 Concurrent Technologies, all rights reserved.    *
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/adcr6.c,v 1.9 2014-06-20 14:16:56 mgostling Exp $
 * $Log: adcr6.c,v $
 * Revision 1.9  2014-06-20 14:16:56  mgostling
 * Added some debugging messages.
 *
 * Revision 1.8  2014-04-29 10:25:28  mgostling
 * Corrected adTestsParams passed to Enxmc_Pex8619_Gpio_Test
 *
 * Revision 1.7  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.6  2014-03-20 16:28:26  mgostling
 * Fixed compiler warning
 *
 * Revision 1.5  2014-03-20 16:08:16  mgostling
 * Added support for a second XMC GPIO loopback test as the two XMC
 * sites have different GPIO configurations.
 *
 * Revision 1.4  2014-03-06 13:46:57  hchalla
 * Added debug #ifdef for debuggin.
 *
 * Revision 1.3  2014-01-10 09:44:52  mgostling
 * Preserve original adTestParams when calling other test interface routines.
 * Tidied code.
 *
 * Revision 1.2  2013-12-10 11:52:28  mgostling
 * Added additional tests for XMC and PMC GPIO signals passed to RTM module.
 *
 * Revision 1.1  2013-11-25 14:27:41  mgostling
 * New ADCR6 tests.
 *
 *
 */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/delay.h>
#include <private/debug.h>
#include <devices/ahci.h>

/* defines */
//#define DEBUG

#define PCIX_CAP_STRUCT			0x07
#define PCIX_STATUS				0x02

#define PERICOM_PCIX_STATUS		0x82	// bug in silicon means you cannot scan for PCIX capability

#define PCIe_CAP_STRUCT			0x10
#define PCIe_LINK_CAP			0x0C
#define PCIe_LINK_NEG			0x12

#define IDT_SW_REG_BASE			0x3E000
#define IDT_GASA_ADDR			0xFF8
#define IDT_GASA_DATA			0xFFC
#define IDT_GPIO_FUNC			IDT_SW_REG_BASE + 0x116c
#define IDT_GPIO_CFG			IDT_SW_REG_BASE + 0x1174
#define IDT_GPIO_DATA			IDT_SW_REG_BASE + 0x1178
#define IDT_TEMP_CTRL			IDT_SW_REG_BASE + 0x11D4
#define IDT_TEMP_STATUS			IDT_SW_REG_BASE + 0x11D8
#define IDT_TEMP_ALARM			IDT_SW_REG_BASE + 0x11DC

#define IDT_TEMP_SENSOR_PDOWN	0x80000000L			// control bit in IDT_TEMP_CTRL
#define IDT_TEMP_SENSOR_UPDATED	0x80000000L			// status bit in IDT_TEMP_ALARM

// error codes
#define	E__ERROR_ADCR6_BAD_PARAM		0x80000100;
#define	E__ERROR_ADCR6_NOT_FOUND		0x80000101;
#define	E__ERROR_ADCR6_BAD_LINK			0x80000102;
#define	E__ERROR_ADCR6_BAD_DETECT		0x80000103;
#define	E__ERROR_ADCR6_NO_CARD			0x80000104;
#define	E__ERROR_ADCR6_OVER_TEMP		0x80000105;
#define	E__ERROR_ADCR6_BAD_GPIO			0x80000106;

// passed parameter bitmap for required tests
#define	ADCR6_TEST_PMC1A				0x0001		// adTestParams = 1
#define	ADCR6_TEST_PMC1B				0x0002		// adTestParams = 2
#define	ADCR6_TEST_PMC2A				0x0004		// adTestParams = 3
#define	ADCR6_TEST_PMC2B				0x0008		// adTestParams = 4
#define	ADCR6_TEST_XMC1					0x0010		// adTestParams = 5
#define	ADCR6_TEST_XMC2					0x0020		// adTestParams = 6
#define	ADCR6_TEST_SATA_DISK			0x0040		// adTestParams = 7
#define	ADCR6_TEST_RTM_GPIO_PMC			0x0080		// adTestParams = 8
#define	ADCR6_TEST_RTM_GPIO_XMC_SITE1	0x0100		// adTestParams = 9
#define	ADCR6_TEST_RTM_GPIO_XMC_SITE2	0x0200		// adTestParams = 10

#define ADCR6_PMC_PRIMARY				0x01
#define ADCR6_PMC_SECONDARY				0x02

/* locals */
static char achBuffer[128];
static UINT32 adTestParamsCopy[17];

/* externals */

/* forward declarations */
UINT32 findAdcr6 (PCI_PFA *pfa);
UINT32 checkPmc (UINT8 pmcBus, UINT8 pmcId, UINT16 idtGpio, UINT8 testBitmap, UINT8 pciType);
UINT32 checkXmc (UINT8 idtBus, UINT8 xmcId, UINT16 idtGpio);
UINT32 checkPericomBridge (UINT8 idtBus, UINT16 idtGpio, UINT8 testBitmap);
UINT32 checkMarvellSata (UINT8 idtBus, UINT8 testBitmap, UINT32 *adTestParams);
UINT16 getPcieLinkStatus (PCI_PFA pfa);
UINT32 getPcieLinkCapability (PCI_PFA pfa);
UINT16 getPcixStatus (PCI_PFA pfa);
UINT8 findPciCapability (PCI_PFA pfa, UINT8 start, UINT8 type);
UINT16 getIdtGpioStatus (PCI_PFA pfa);

// AHCI CUTE tests function prototypes
extern UINT32 dAhciRegisterTest (UINT32* adTestParams);
extern UINT32 dAhciDriveAttachTest (UINT32* adTestParams);
extern UINT32 dAhciInterruptTest (UINT32* adTestParams);

// ENXMC CUTE test function prototype
extern UINT32 dEnxmc_Pex8619_Gpio_Test (UINT32* adTestParams);

/*****************************************************************************
 * adcr6Test: Test the ADCR6 interfaces
 *
 * RETURNS: None 
 */
TEST_INTERFACE (Adcr6Test, "ADCR6 Test")
{
	UINT32	rt;
	PCI_PFA adcr6RootPfa;
	UINT16	idtGpioData;
	UINT8	adcr6Bus;
	UINT16	testBitmap;
	int		cnt;

	// take a local copy of the adTestParams array passed to this Test Case
	// This test invokes other Test Cases functions 
	// and modifies adTestParams to pass the required parameters for those tests
	for (cnt = 0; cnt < LENGTH(adTestParams); ++cnt)
	{
		adTestParamsCopy[cnt] = adTestParams[cnt];
	}
#ifdef DEBUG
	sprintf(achBuffer, "adTestParams[0] = %d\n", adTestParams[0]);
	vConsoleWrite(achBuffer);
#endif

	testBitmap = 0;
	for (cnt = 1; cnt <= adTestParams[0]; ++cnt)
	{
#ifdef DEBUG
		sprintf(achBuffer, "adTestParams[%d] = %d\n", cnt, adTestParams[cnt]);
		vConsoleWrite(achBuffer);
#endif

		if ((adTestParams[cnt] < 1) || (adTestParams[cnt] > 10))		// test numbers 1 to 10)
		{
			vConsoleWrite("Invalid test parameter\n");
			return E__ERROR_ADCR6_BAD_PARAM;
		}
		testBitmap |= (1 << (adTestParams[cnt] - 1));
		adTestParams[cnt] = 0;
	}

#ifdef DEBUG
	sprintf (achBuffer, "testBitmap = %04X\n", testBitmap);
	vConsoleWrite(achBuffer);
#endif

	rt = findAdcr6(&adcr6RootPfa);

	if (rt == E__OK)			// we found it
	{
		// ADCR6 IDT switch configuration:
		//	Port0  -> host x8
		//	port4  -> XMC2 x8
		//	Port8  -> Pericom PCI-X Bridge x4 for PMC1 & PMC2
		//	Port12 -> Marvell Sata x2
		//	Port16 -> XMC1 x8

		// check downstream port connections

		// internal bus of the IDT switch on the ADCR6
		adcr6Bus = PCI_PFA_BUS(adcr6RootPfa) + 1;

		// get GPIO indications of XMC/PMC cards
		idtGpioData = getIdtGpioStatus(adcr6RootPfa);

		rt = checkPericomBridge(adcr6Bus, idtGpioData, testBitmap);		// PMC slots

		if ((rt == E__OK) && ((testBitmap & ADCR6_TEST_XMC1) != 0))
			rt = checkXmc(adcr6Bus, 1, idtGpioData);					// XMC 1

		if ((rt == E__OK) && ((testBitmap & ADCR6_TEST_XMC2) != 0))
			rt = checkXmc(adcr6Bus, 2, idtGpioData);					// XMC 2

		if (rt == E__OK)
			rt = checkMarvellSata(adcr6Bus, testBitmap, adTestParams);	// SATA

		if ((rt == E__OK) && ((testBitmap & ADCR6_TEST_RTM_GPIO_PMC) != 0))
		{
			adTestParams[0] = 1;
			adTestParams[1] = 1;										// RTM PMC GPIO
			adTestParams[2] = 0;										// not used
			rt = dEnxmc_Pex8619_Gpio_Test(adTestParams);
		}

		if ((rt == E__OK) && ((testBitmap & ADCR6_TEST_RTM_GPIO_XMC_SITE1) != 0))
		{
			adTestParams[0] = 2;
			adTestParams[1] = 2;										// RTM XMC GPIO
			adTestParams[2] = 1;										// SITE 1
			rt = dEnxmc_Pex8619_Gpio_Test(adTestParams);
		}

		if ((rt == E__OK) && ((testBitmap & ADCR6_TEST_RTM_GPIO_XMC_SITE2) != 0))
		{
			adTestParams[0] = 2;
			adTestParams[1] = 2;										// RTM XMC GPIO
			adTestParams[2] = 2;										// SITE 2
			rt = dEnxmc_Pex8619_Gpio_Test(adTestParams);
		}
	}

	// restore original adTestParams for this function
	// from local copy in case this function is invoked again
	for (cnt = 0; cnt < LENGTH(adTestParams); ++cnt)
	{
		adTestParams[cnt] = adTestParamsCopy[cnt];
	}

	return rt;
}

/*****************************************************************************
 * findAdcr6: Find the Root port of the IDT switch on the ADCR6
 * RETURNS: OK or error
 */
UINT32 findAdcr6 (PCI_PFA *adcr6Pfa)
{
	UINT32	rt;
	UINT32	dTemp;
	UINT16	vid;
	UINT16	did;
	UINT16	linkStatus;
	PCI_PFA pfa;
	UINT8	secondaryBus;
	UINT8	subordinateBus;
	UINT8	adcr6Found;
	UINT8	temperature;

	rt = E__OK;

	// check if the ADCR6 is connected to XMC Slot1 of host VX819

#ifdef DEBUG
	pfa = PCI_MAKE_PFA (0,1,0);				// first PCIe root port in CPU
	vid = PCI_READ_WORD(pfa, 0);
	did = PCI_READ_WORD(pfa, 2);
	sprintf(achBuffer, "pfa = 0x%04X, Vid =  0x%04X, Did = 0x%04X\n", pfa, vid, did);
	vConsoleWrite(achBuffer);

	pfa = PCI_MAKE_PFA (1,0,0);				// root port of IDT Switch fitted to VX819
	vid = PCI_READ_WORD(pfa, 0);
	did = PCI_READ_WORD(pfa, 2);
	sprintf(achBuffer, "pfa = 0x%04X, Vid =  0x%04X, Did = 0x%04X\n", pfa, vid, did);
	vConsoleWrite(achBuffer);
#endif

	pfa = PCI_MAKE_PFA (2,16,0);			// IDT switch port 16 is connected to XMC slot 1
	secondaryBus = PCI_READ_BYTE(pfa, PCI_SEC_BUS);
	subordinateBus = PCI_READ_BYTE(pfa, PCI_SUB_BUS);

	adcr6Found = FALSE;
	if (secondaryBus != subordinateBus)
	{
		// the IDT switch on ADCR6 requires at least 6 buses

		pfa = PCI_MAKE_PFA(secondaryBus, 0, 0);	// Root port of IDT Switch fitted to ADCR6
		vid = PCI_READ_WORD(pfa, 0);
		did = PCI_READ_WORD(pfa, 2);

#ifdef DEBUG
		sprintf(achBuffer, "Host(XMC1)->ADCR6 switch pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", pfa, vid, did);
		vConsoleWrite(achBuffer);
#endif

		if ((vid == 0x111D) && (did == 0x808F))	// found the IDT switch on the ADCR6
		{
			*adcr6Pfa = pfa;
			adcr6Found = TRUE;
		}
	}

	if (!adcr6Found)
	{
		// check if the ADCR6 is connected to XMC Slot2 of host VX819
		pfa = PCI_MAKE_PFA (0,1,1);				// second PCIe root port of CPU is connected to XMC slot 2
		secondaryBus = PCI_READ_BYTE(pfa, PCI_SEC_BUS);
		subordinateBus = PCI_READ_BYTE(pfa, PCI_SUB_BUS);

		pfa = PCI_MAKE_PFA(secondaryBus, 0, 0);	// Root port of IDT Switch fitted to ADCR6
		vid = PCI_READ_WORD(pfa, 0);
		did = PCI_READ_WORD(pfa, 2);

#ifdef DEBUG
		sprintf(achBuffer, "Host(XMC2)->ADCR6 switch pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", pfa, vid, did);
		vConsoleWrite(achBuffer);
#endif

		if ((vid == 0x111D) && (did == 0x808F))	// found the IDT switch on the ADCR6
		{
			*adcr6Pfa = pfa;
			adcr6Found = TRUE;
		}
	}

	if (!adcr6Found)
	{
		vConsoleWrite("ADCR6 not found\n");
		rt = E__ERROR_ADCR6_NOT_FOUND;
	}
	else
	{
		// check root port has negotiated x8 width and GEN2
		// by reading PCIE Capability Link Status register
		linkStatus = getPcieLinkStatus(pfa);

#ifdef DEBUG
		sprintf(achBuffer, "Root Port (Host CPU) Port Width = %d, clock = Gen%d\n", (linkStatus & 0x03F0) >> 4, linkStatus & 0x000F);
		vConsoleWrite(achBuffer);
#endif

		if ((linkStatus & 0x03FF) != 0x0082)	// link should always be x8 and GEN2
		{
			rt = E__ERROR_ADCR6_BAD_LINK;
			vConsoleWrite("ADCR6 Root Port (Host CPU) not configured correctly\n");
			sprintf(achBuffer, "Port Width = %d, clock = Gen%d\n", (linkStatus & 0x03F0) >> 4, linkStatus & 0x000F);
			vConsoleWrite(achBuffer);
		}

		// read the switch temperature sensor
		if (rt == E__OK)
		{
			PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_TEMP_CTRL);
			dTemp = PCIE_READ_DWORD(pfa, IDT_GASA_DATA);

#ifdef DEBUG
			sprintf(achBuffer, "IDT_TEMP_CTRL 0x%08X\n", dTemp);
			vConsoleWrite(achBuffer);
#endif

			// enable the temperature sensor
			dTemp &= ~IDT_TEMP_SENSOR_PDOWN;
			PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_TEMP_CTRL);
			PCIE_WRITE_DWORD(pfa, IDT_GASA_DATA, dTemp);

			// allow time for temperature sensor to update
			dTemp = 0;
			while ((dTemp & IDT_TEMP_SENSOR_UPDATED) == 0)
			{
				vDelay(20);
				PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_TEMP_ALARM);
				dTemp = PCIE_READ_DWORD(pfa, IDT_GASA_DATA);

#ifdef DEBUG
				sprintf(achBuffer, "IDT_TEMP_ALARM 0x%08X\n", dTemp);
				vConsoleWrite(achBuffer);
#endif
			}

			PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_TEMP_STATUS);
			temperature = PCIE_READ_BYTE(pfa, IDT_GASA_DATA);

			sprintf(achBuffer, "ADCR6 Switch temperature = %d.%d deg C\n", temperature >> 1, ((temperature & 0x01) != 0) ? 5 : 0);
			vConsoleWrite(achBuffer);

			if ((((temperature >> 1) == 125) && ((temperature & 0x01) == 1))		// 125.5
				|| ((temperature >> 1) > 125))
			{
				rt = E__ERROR_ADCR6_OVER_TEMP;
			}
		}
	}

	return rt;
}

/*****************************************************************************
 * getIdtGpioStatus: Ensure IDT GPIO configured as inputs and read GPIO status
 * RETURNS: GPIO status
 */
UINT16 getIdtGpioStatus (PCI_PFA pfa)
{
	UINT32	idtGpioFunc;
	UINT32	idtGpioCfg;
	UINT32	idtGpioData;

// read IDT switch GPIO registers

	PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_GPIO_FUNC);
	idtGpioFunc = PCIE_READ_DWORD(pfa, IDT_GASA_DATA);
	PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_GPIO_CFG);
	idtGpioCfg = PCIE_READ_DWORD(pfa, IDT_GASA_DATA);

#ifdef DEBUG
	sprintf(achBuffer, "IDT_GPIO_FUNC = 0x%08X\n", idtGpioFunc);
	vConsoleWrite(achBuffer);
	sprintf(achBuffer, "IDT_GPIO_CFG = 0x%08X\n", idtGpioCfg);
	vConsoleWrite(achBuffer);
#endif

	if ((idtGpioCfg & 0x000000F0L) != 0)
	{
		PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_GPIO_CFG);
		PCIE_WRITE_DWORD(pfa, IDT_GASA_DATA, ~0x000000F0L);
		vDelay(1);
	}

	if ((idtGpioFunc & 0x000000F0L) != 0)
	{
		PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_GPIO_FUNC);
		PCIE_WRITE_DWORD(pfa, IDT_GASA_DATA, ~0x000000F0L);
		vDelay(1);
	}

	PCIE_WRITE_DWORD(pfa, IDT_GASA_ADDR, IDT_GPIO_DATA);
	idtGpioData = PCIE_READ_DWORD(pfa, IDT_GASA_DATA);


#ifdef DEBUG
	sprintf(achBuffer, "Raw  IDT_GPIO_DATA = 0x%08X\n", idtGpioData);
	vConsoleWrite(achBuffer);
	sprintf(achBuffer, "Used IDT_GPIO_DATA = 0x%08X\n", ~idtGpioData);
	vConsoleWrite(achBuffer);
#endif

	// GPIO signals are active low, so invert them to make things easier

	return (UINT16) ((~idtGpioData) & 0x1FF);
}

/*****************************************************************************
 * checkXmc: Checks an XMC slot connected to the ADCR6
 * RETURNS: OK or error
 */
UINT32 checkXmc (UINT8 idtBus, UINT8 xmcId, UINT16 idtGpio)
{
	UINT32	rt;
	UINT8	idtPort;			// port address of the XMC slot
	UINT8	secondaryBus;
	UINT8	pciSeen;
	UINT16	gpioMask;
	UINT16	vid;
	UINT16	did;
	UINT16	linkStatus;
	UINT16	linkCapability;
	PCI_PFA	pfa;

	rt = E__OK;

	// default to XMC slot1
	idtPort = 16;
	gpioMask = 0x10;

	if (xmcId == 2)				// is slot 2?
	{
		idtPort >>= 2;			// idtPort = 4;
		gpioMask <<= 2;			// idtGpio = 0x40
	}

	pfa = PCI_MAKE_PFA(idtBus, idtPort, 0);
	secondaryBus = PCI_READ_BYTE(pfa, PCI_SEC_BUS);	// bus of controller in XMC slot of ADCR6

#ifdef DEBUG
	vid = PCI_READ_WORD(pfa, 0);
	did = PCI_READ_WORD(pfa, 2);
	sprintf(achBuffer, "ADCR6 XMC%d switch pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", xmcId, pfa, vid, did);
	vConsoleWrite(achBuffer);
#endif

	pfa = PCI_MAKE_PFA(secondaryBus, 0, 0);			// PFA of controller connected to IDT switch
	vid = PCI_READ_WORD(pfa, 0);
	did = PCI_READ_WORD(pfa, 2);

#ifdef DEBUG
	sprintf(achBuffer, "ADCR6 XMC%d controller pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", xmcId, pfa, vid, did);
	vConsoleWrite(achBuffer);
#endif
	pciSeen = TRUE;
	if ((vid == 0xFFFF) || (did == 0xFFFF))
		pciSeen = FALSE;

	if (pciSeen && ((idtGpio & gpioMask) != 0))
	{
		sprintf(achBuffer, "Controller detected on PCI bus and by GPIO in XMC%d \n", xmcId);
		vConsoleWrite(achBuffer);

		linkStatus = getPcieLinkStatus(pfa) & 0x3FF;;
		linkCapability = ((UINT16) getPcieLinkCapability(pfa)) & 0x3FF;

#ifdef DEBUG
		sprintf(achBuffer, "ADCR6 XMC%d Controller Port Width = %d, clock = Gen%d\n", xmcId, (linkStatus & 0x03F0) >> 4, linkStatus & 0x000F);
		vConsoleWrite(achBuffer);
#endif
		if (linkStatus != linkCapability)
		{
			rt = E__ERROR_ADCR6_BAD_LINK;
			sprintf(achBuffer, "ADCR6 XMC%d Controller not configured correctly\n", xmcId);
			vConsoleWrite(achBuffer);
			sprintf(achBuffer, "Port Width = %d, clock = Gen%d\n", (linkStatus & 0x03F0) >> 4, linkStatus & 0x000F);
			vConsoleWrite(achBuffer);
		}
	}

	if ((rt == E__OK) && (!pciSeen && ((idtGpio & gpioMask) == 0)))
	{
		rt = E__ERROR_ADCR6_NO_CARD;
		sprintf(achBuffer, "No Controller detected on PCI bus or by GPIO in ADCR6 XMC%d\n", xmcId);
		vConsoleWrite(achBuffer);
	}

	// check that both detections match

	if ((rt == E__OK) &&
		((pciSeen && ((idtGpio & gpioMask) == 0)) ||
		(!pciSeen && ((idtGpio & gpioMask) != 0))))
	{
		rt = E__ERROR_ADCR6_BAD_DETECT;
		if (pciSeen)
		{
			sprintf(achBuffer, "Controller detected on PCI bus but not by GPIO in ADCR6 XMC%d\n", xmcId);
			vConsoleWrite(achBuffer);
		}
		else
		{
			sprintf(achBuffer, "Controller detected by GPIO but not on PCI bus in ADCR6 XMC%d\n", xmcId);
			vConsoleWrite(achBuffer);
		}
	}

	return rt;
}

/*****************************************************************************
 * checkPericomBridge: Checks a PMC slot connected to the ADCR6
 * RETURNS: OK or error
 */
UINT32 checkPericomBridge (UINT8 idtBus, UINT16 idtGpio, UINT8 testBitmap)
{
	UINT32	rt;
	PCI_PFA	pfa;
	UINT16	linkStatus;
#ifdef DEBUG
	UINT16	vid;
	UINT16	did;
#endif
	UINT8	pciType;
	UINT8	pericomBus;
	UINT8	pmcBus;

	// port 8 of IDT switch is connected to PERICOM bridge
	pfa = PCI_MAKE_PFA(idtBus, 8, 0);

	// check port has negotiated x4 width and GEN1
	// by reading PCIE Capability Link Status register
	linkStatus = getPcieLinkStatus(pfa);

#ifdef DEBUG
	vid = PCI_READ_WORD(pfa, 0);
	did = PCI_READ_WORD(pfa, 2);
	sprintf(achBuffer, "ADCR6->Pericom PCI-X bridge pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", pfa, vid, did);
	vConsoleWrite(achBuffer);
	sprintf(achBuffer, "Pericom PCI-X Bridge Port Width = %d, clock = Gen%d\n", (linkStatus & 0x03F0) >> 4, linkStatus & 0x000F);
	vConsoleWrite(achBuffer);
#endif

	if ((linkStatus & 0x3FF) != 0x0041)
	{
		rt = E__ERROR_ADCR6_BAD_LINK;	// failed to negotiate correct link
		vConsoleWrite("ADCR6 Port 8 (Pericom PCI-X Bridge) not configured correctly\n");
		sprintf(achBuffer, "Port Width = %d, clock = Gen%d\n", (linkStatus & 0x03F0) >> 4, linkStatus & 0x000F);
		vConsoleWrite(achBuffer);
	}
	else
	{
		// bus allocated to PCI/PCIX controllers beyond bridge
		pericomBus = PCI_READ_BYTE(pfa, PCI_SEC_BUS);

		pfa = PCI_MAKE_PFA(pericomBus, 0, 0);

#ifdef DEBUG
		vid = PCI_READ_WORD(pfa, 0);
		did = PCI_READ_WORD(pfa, 2);
		sprintf(achBuffer, "Pericom bridge pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", pfa, vid, did);
		vConsoleWrite(achBuffer);
#endif

		pciType = (UINT8) ((PCI_READ_WORD(pfa, PERICOM_PCIX_STATUS) >> 6) & 0x07);

		pmcBus = PCI_READ_BYTE(pfa, PCI_SEC_BUS);

#ifdef DEBUG
		sprintf(achBuffer, "ADCR6 PMC bus = %d\n", pmcBus);
		vConsoleWrite(achBuffer);
#endif

		rt = checkPmc(pmcBus, 1, idtGpio, testBitmap, pciType);			// PMC 1

		if (rt == E__OK)
			rt = checkPmc(pmcBus, 2, idtGpio, testBitmap, pciType);		// PMC 2
	}

	return rt;
}

/*****************************************************************************
 * checkPmc: Checks a PMC slot connected to the ADCR6
 * RETURNS: OK or error
 */
UINT32 checkPmc (UINT8 pmcBus, UINT8 pmcId, UINT16 idtGpio, UINT8 testBitmap, UINT8 pciType)
{
	UINT32	rt;
	UINT8	pmcDev;				// device address of a controller on a PMC card
	UINT8	pciSeen;
	UINT16	gpioMask;
	UINT16	bitMap;
	UINT16	vid;
	UINT16	did;
	PCI_PFA	pfa;

	rt = E__OK;
	bitMap = testBitmap;

#ifdef DEBUG
		sprintf(achBuffer, "ADCR6 PMC%d\n", pmcId);
		vConsoleWrite(achBuffer);
#endif

	// default to PMC slot1
	pmcDev = 4;
	gpioMask = 0x20;			// PMC bit in idtGpio

	if (pmcId == 2)				// is slot 2?
	{
		pmcDev = 6;
		gpioMask <<= 2;			// PMC bit in idtGpio = 0x80
		bitMap >>= 2;
	}

#ifdef DEBUG
		sprintf(achBuffer, "pmcDev == %d, idtGpio = 0x%04X, gpioMask = 0x%04x, bitMap = 0x%04X\n",
					pmcDev, idtGpio, gpioMask, bitMap);
		vConsoleWrite(achBuffer);
#endif

	if ((bitMap & ADCR6_PMC_PRIMARY) != 0)			// test primary PCI address
	{
		pfa = PCI_MAKE_PFA(pmcBus, pmcDev, 0);			// PFA of controller connected to Pericom switch
		vid = PCI_READ_WORD(pfa, 0);
		did = PCI_READ_WORD(pfa, 2);

#ifdef DEBUG
		sprintf(achBuffer, "ADCR6 PMC%d primary controller pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", pmcId, pfa, vid, did);
		vConsoleWrite(achBuffer);
#endif

		pciSeen = TRUE;
		if ((vid == 0xFFFF) || (did == 0xFFFF))
			pciSeen = FALSE;

		if (pciSeen && ((idtGpio & gpioMask) != 0))
		{
			sprintf(achBuffer, "%s Primary Controller detected on PCI bus and by GPIO in ADCR6 PMC%d \n", (pciType > 0 ? "PCIX" : "PCI"), pmcId);
			vConsoleWrite(achBuffer);

		}

		if (!pciSeen && ((idtGpio & gpioMask) == 0))
		{
			rt = E__ERROR_ADCR6_NO_CARD;
			sprintf(achBuffer, "No Primary Controller detected on PCI bus or by GPIO in ADCR6 PMC%d\n", pmcId);
			vConsoleWrite(achBuffer);
		}

		// check that both detections match

		if ((rt == E__OK) &&
			((pciSeen && ((idtGpio & gpioMask) == 0)) ||
			(!pciSeen && ((idtGpio & gpioMask) != 0))))
		{
			rt = E__ERROR_ADCR6_BAD_DETECT;
			if (pciSeen)
			{
				sprintf(achBuffer, "%s Primary Controller detected on PCI bus but not by GPIO in ADCR6 PMC%d\n", (pciType > 0 ? "PCIX" : "PCI"), pmcId);
				vConsoleWrite(achBuffer);
			}
			else
			{
				sprintf(achBuffer, "Primary Controller detected by GPIO but not on PCI bus in ADCR6 PMC%d\n", pmcId);
				vConsoleWrite(achBuffer);
			}
		}
	}
	if ((rt == E__OK) && ((bitMap & ADCR6_PMC_SECONDARY) != 0))		// test secondary PCI address
	{
		pfa = PCI_MAKE_PFA(pmcBus, pmcDev + 1, 0);						// PFA of controller connected to Pericom switch
		vid = PCI_READ_WORD(pfa, 0);
		did = PCI_READ_WORD(pfa, 2);

#ifdef DEBUG
		sprintf(achBuffer, "ADCR6 PMC%d secondary controller pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", pmcId, pfa, vid, did);
		vConsoleWrite(achBuffer);
#endif

		pciSeen = TRUE;
		if ((vid == 0xFFFF) || (did == 0xFFFF))
			pciSeen = FALSE;

		if (pciSeen && ((idtGpio & gpioMask) != 0))
		{
			sprintf(achBuffer, "%s Secondary Controller detected on PCI bus and by GPIO in ADCR6 PMC%d \n", (pciType > 0 ? "PCIX" : "PCI"), pmcId);
			vConsoleWrite(achBuffer);
		}

		if (!pciSeen && ((idtGpio & gpioMask) == 0))
		{
			rt = E__ERROR_ADCR6_NO_CARD;
			sprintf(achBuffer, "No Secondary Controller detected on PCI bus or by GPIO in ADCR6 PMC%d\n", pmcId);
			vConsoleWrite(achBuffer);
		}

		// check that both detections match

		if ((rt == E__OK) &&
			((pciSeen && ((idtGpio & gpioMask) == 0)) ||
			(!pciSeen && ((idtGpio & gpioMask) != 0))))
		{
			rt = E__ERROR_ADCR6_BAD_DETECT;
				if (pciSeen)
				{
					sprintf(achBuffer, "%s Secondary Controller detected on PCI bus but not by GPIO in ADCR6 PMC%d\n", (pciType > 0 ? "PCIX" : "PCI"), pmcId);
					vConsoleWrite(achBuffer);
				}
				else
				{
					sprintf(achBuffer, "Secondary Controller detected by GPIO but not on PCI bus in ADCR6 PMC%d\n", pmcId);
					vConsoleWrite(achBuffer);
				}
		}
	}

	return rt;
}

/*****************************************************************************
 * checkMarvellSata: Find the Root port of the IDT switch on the ADCR6
 * RETURNS: OK or error
 */
UINT32 checkMarvellSata (UINT8 idtBus, UINT8 testBitmap, UINT32 *adTestParams)
{
	UINT32	rt;
	PCI_PFA	pfa;
	UINT8	sataBus;
	UINT16	linkStatus;
#ifdef DEBUG
	UINT16	vid;
	UINT16	did;
#endif

	rt = E__OK;

	// port 12 of IDT switch is connected to Marvell SATA
	pfa = PCI_MAKE_PFA(idtBus, 12, 0);

#ifdef DEBUG
	vid = PCI_READ_WORD(pfa, 0);
	did = PCI_READ_WORD(pfa, 2);
	sprintf(achBuffer, "ADCR6->Marvell SATA pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", pfa, vid, did);
	vConsoleWrite(achBuffer);
#endif

	// check port has negotiated x4 width and GEN1
	// by reading PCIE Capability Link Status register
	linkStatus = getPcieLinkStatus(pfa);

#ifdef DEBUG
	sprintf(achBuffer, "Marvell SATA Port Width = %d, clock = Gen%d\n", (linkStatus & 0x03F0) >> 4, linkStatus & 0x000F);
	vConsoleWrite(achBuffer);
#endif

	if ((linkStatus & 0x3FF) != 0x0022)
	{
		vConsoleWrite("ADCR6 Port 12 (Marvell SATA) not configured correctly\n");
		sprintf(achBuffer, "Port Width = %d, clock = Gen%d\n", (linkStatus & 0x03F0) >> 4, linkStatus & 0x000F);
		vConsoleWrite(achBuffer);
		rt = E__ERROR_ADCR6_BAD_LINK;	// failed to negotiate correct link
	}
	else
	{
		// bus allocated to SATA controller beyond bridge
		sataBus = PCI_READ_BYTE(pfa, PCI_SEC_BUS);

		// find required instance of SATA controller for AHCI tests
		pfa = 0;
		adTestParams[1] = 0;		// first instance
		while (PCI_PFA_BUS(pfa) != sataBus)
		{
			rt = iPciFindDeviceByClass(adTestParams[1] + 1, 0x01, 0x06, &pfa);
			if (rt == E__DEVICE_NOT_FOUND)
				return E__PFA_DOESNT_EXIST;
			else
				++adTestParams[1];	// next instance
		}

#ifdef DEBUG
		vid = PCI_READ_WORD(pfa, 0);
		did = PCI_READ_WORD(pfa, 2);
		sprintf(achBuffer, "Marvell SATA pfa = 0x%04X, Vid = 0x%04X, Did = 0x%04X\n", pfa, vid, did);
		vConsoleWrite(achBuffer);
		sprintf(achBuffer, "Marvell SATA required instance = %d\n", adTestParams[1]);
		vConsoleWrite(achBuffer);
#endif

		// adTestParams[1] is required instance of SATA controller
		adTestParams[2] = 1;		// port 1
		adTestParams[0] = 2;		// 2 parameters passed
		rt = dAhciRegisterTest(adTestParams);

		// 'drive attached' tested as part of interrupt test
		if ((rt == E__OK) && ((testBitmap & ADCR6_TEST_SATA_DISK) != 0))
			rt = dAhciInterruptTest(adTestParams);
	}

	return rt;
}

/*****************************************************************************
 * getPcieLinkStatus: Find the PCIe Negotiated Link Status
 * RETURNS: PCIe Negotiated Link Status
 */
UINT16 getPcieLinkStatus (PCI_PFA pfa)
{
	UINT16	linkStat;
	UINT8	capStart;

	linkStat = 0;

	capStart = findPciCapability(pfa, PCI_CAP_POINTER, PCIe_CAP_STRUCT);

	if (capStart != 0)
		linkStat = PCI_READ_WORD(pfa, capStart + PCIe_LINK_NEG);		// negotiated link setting

#ifdef DEBUG
	sprintf(achBuffer, "Link Status = 0x%04X\n", linkStat);
	vConsoleWrite(achBuffer);
#endif

	return linkStat;
}

/*****************************************************************************
 * getPcieLinkCapability: Find the PCIe Link Capability
 * RETURNS: PCIe Link Capability
 */
UINT32 getPcieLinkCapability (PCI_PFA pfa)
{
	UINT32	capability;
	UINT8	capStart;

	capability = 0;

	capStart = findPciCapability(pfa, PCI_CAP_POINTER, PCIe_CAP_STRUCT);

	if (capStart != 0)
		capability = PCI_READ_DWORD(pfa, capStart + PCIe_LINK_CAP);		// link capabilities

#ifdef DEBUG
	sprintf(achBuffer, "Link Capability = 0x%08X\n", capability);
	vConsoleWrite(achBuffer);
#endif

	return capability;
}

/*****************************************************************************
 * getPcixCapability: Find the PCIX Capability status
 * RETURNS: PCIX Capability Status
 */
UINT16 getPcixStatus (PCI_PFA pfa)
{
	UINT16	status;
	UINT8	capStart;

	status = 0;

	capStart = findPciCapability(pfa, PCI_CAP_POINTER, PCIX_CAP_STRUCT);

	if (capStart != 0)
		status = PCI_READ_WORD(pfa, capStart + PCIX_STATUS);		// PCIX status

#ifdef DEBUG
	sprintf(achBuffer, "PCIX Status = 0x%04X\n", status);
	vConsoleWrite(achBuffer);
#endif

	return status;
}

/*****************************************************************************
 * findPciCapability: find a particular capability structure
 * RETURNS: start of capability structure
 */
UINT8 findPciCapability (PCI_PFA pfa, UINT8 start, UINT8 type)
{
	UINT8	capStart;
	UINT16	capId;

	capStart = PCI_READ_WORD(pfa, start);
	while (capStart != 0)
	{
		capId = PCI_READ_WORD(pfa, capStart);
#ifdef DEBUG
//		sprintf(achBuffer, "CapStart = 0x%02X\n", capStart);
//		vConsoleWrite(achBuffer);
//		sprintf(achBuffer, "CapId = 0x%02X\n", capId & 0x00FF);
//		vConsoleWrite(achBuffer);
//		sprintf(achBuffer, "NextCap = 0x%02X\n", capId >> 8);
//		vConsoleWrite(achBuffer);
#endif
		if ((capId & 0x00FF) == type)		// found required capability?
			break;

		capStart = capId >> 8;		// next capability
	}

	return capStart;
}

