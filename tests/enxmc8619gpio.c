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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/enxmc8619gpio.c,v 1.12 2014-11-14 17:18:55 mgostling Exp $
 * $Log: enxmc8619gpio.c,v $
 * Revision 1.12  2014-11-14 17:18:55  mgostling
 * Updated xmc_gpio_loopback_ADCR6 for the revised test requirements of XMC slot 1
 * Added some additional ebug messages to validate ENXMC_GPIO_LOOPBACK_INFO parameter passing.
 *
 * Revision 1.11  2014-07-29 11:32:24  mgostling
 * Use common error codes from enxmc8619.h instead of defining them locally.
 *
 * Revision 1.10  2014-07-11 08:45:29  mgostling
 * Updated XMC tests for ADCR6.
 * Loopback pin connections for XMC site 2 were not as originally specified.
 *
 * Revision 1.9  2014-06-20 14:29:05  mgostling
 * Fixed parameter passing and retrieval for SERVICE__BRD_GET_ENXMC_GPIO_INFO function.
 * Added some debugging messages.
 *
 * Revision 1.8  2014-04-29 08:43:21  mgostling
 * Added comment that VRE1x requires adTestParam2.
 * No changes to the code.
 *
 * Revision 1.7  2014-04-23 15:38:38  mgostling
 * Updated VRE1x loopback connection details
 *
 * Revision 1.6  2014-03-28 09:08:30  mgostling
 * The VRE1x requires a different test for each XMC slot as they support different GPIOs
 *
 * Revision 1.5  2014-03-27 14:10:21  mgostling
 * Fixed a problem with the VRE1x loopback test for GPIOs 34 to 39.
 *
 * Revision 1.4  2014-03-27 11:55:17  mgostling
 * Added a service to obtain board specific GPIO loopback configurations.
 * Added a new test to verify XMC differential IO signals.
 *
 * Revision 1.3  2014-03-26 14:43:30  mgostling
 * Refactored and debugged XMC GPIO loopback tests.
 *
 * Revision 1.2  2014-03-21 13:29:24  mgostling
 * Added support for the XMC GPIO test for the TRB1x and VRE1x.
 * Added support for the two different XMC GPIO tests required by the ADCR6.
 *
 * Revision 1.1  2014-03-20 15:56:50  mgostling
 * Moved all GPIO tests into a seperate file
 *
 */
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
#include <devices/enxmc8619.h>
#include <private/cpu.h>

/* defines */

//#define DEBUG

#define I2C_PCA8574_1	0x4C			// 0x26 << 1
#define I2C_PCA8574_2	0x4E			// 0x27 << 1

/* locals */

// the following tables are for GPIO signals driven by PLX8619
static char		achBuffer[80];
static PCI_PFA	enxmcPfa;			// required by ENXMC GPIO I2C routines

/* externals */

/* forward declarations */
static UINT32 pmc_gpio_loopback (void);
static UINT32 xmc_gpio_loopback_ADCR6 (UINT32 site);
static UINT32 xmc_gpio_loopback_TRB1X (void);
static UINT32 xmc_gpio_loopback_VRE1X (UINT32 site);
static UINT32 xmc_gpio0_29_test (PCI_PFA pfa, UINT32 dConfig1, UINT32 dConfig2, UINT32 dOutputs, UINT32 dInputs, const ENXMC_GPIO_SIGNAL_PAIRS *loopback_table);
static UINT32 xmc_gpio32_47_test (UINT8 i2cGpioDev);
static UINT32 xmc_gpio40_47_test (void);

static UINT8 enxmc_pex8619_gpio_read_I2C_input(UINT8 mask);
static UINT8 enxmc_pex8619_gpio_read_I2C_output(void);
static void enxmc_pex8619_gpio_write_I2C(UINT8 value, UINT8 mask);

// ======================= I2C external hardware interface ============================

/*****************************************************************************
 * enxmc_pex8619_gpio_write_I2C_hw: Write the I2C bus signals
 *
 * Uses GPIO30 & GPIO31 as bit-banging interface on I2C bus
 *
 * RETURNS: None
 */
static void enxmc_pex8619_gpio_write_I2C(UINT8 value, UINT8 mask)
{
//	Bit 0 of UINT8 value = I2C CLOCK signal
//	Bit 1 of UINT8 value = I2C DATA signal

	UINT16 wTemp;
	UINT32 dOutput;

	dOutput = PCIE_READ_DWORD(enxmcPfa, PLX_GPIO2_DIR_CTL);
	if ((mask & I2C_CLOCK_BIT) != 0)
	{
		dOutput &= PLX_GPIO2_I2C_CLOCK_INPUT_MASK;
		dOutput |= PLX_GPIO2_I2C_CLOCK_OUTPUT;
	}

	if ((mask & I2C_DATA_BIT) != 0)
	{
		dOutput &= PLX_GPIO2_I2C_DATA_INPUT_MASK;
		dOutput |= PLX_GPIO2_I2C_DATA_OUTPUT;
	}


	wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_OUTPUT_DATA);
	wTemp &= ~(PLX_GPIO2_I2C_CLOCK_LINE + PLX_GPIO2_I2C_DATA_LINE);
	wTemp |= ((UINT16) value) << 14;

	PCIE_WRITE_WORD(enxmcPfa, PLX_GPIO2_OUTPUT_DATA, wTemp);
	PCIE_WRITE_DWORD(enxmcPfa, PLX_GPIO2_DIR_CTL, dOutput);
}

/*****************************************************************************
 * enxmc_pex8619_gpio_read_I2C_input: Read the I2C bus signals
 *
 * Uses GPIO30 & GPIO31 as bit-banging interface on I2C bus
 *
 * RETURNS: I2C bus value
 */
static UINT8 enxmc_pex8619_gpio_read_I2C_input(UINT8 mask)
{
//	Bit 0 of UINT8 value = I2C CLOCK signal
//	Bit 1 of UINT8 value = I2C DATA signal
//	Bit 7 set = error
//
// this procedure cannot identify I2C bus errors so bit 7 will never be set

	UINT16 wTemp;
	UINT32 dOutput;

	dOutput = PCIE_READ_DWORD(enxmcPfa, PLX_GPIO2_DIR_CTL);
	if ((mask & I2C_CLOCK_BIT) != 0)
		dOutput &= PLX_GPIO2_I2C_CLOCK_INPUT_MASK;

	if ((mask & I2C_DATA_BIT) != 0)
		dOutput &= PLX_GPIO2_I2C_DATA_INPUT_MASK;

	PCIE_WRITE_DWORD(enxmcPfa, PLX_GPIO2_DIR_CTL, dOutput);

	wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
	wTemp >>= 14;
	wTemp &= ((UINT16) mask);

	return ((UINT8) (wTemp));
}

/*****************************************************************************
 * enxmc_pex8619_gpio_read_I2C_output: Read the I2C bus output port signals
 *
 * Uses GPIO30 & GPIO31 as bit-banging interface on I2C bus
 *
 * RETURNS: I2C bus value
 */
static UINT8 enxmc_pex8619_gpio_read_I2C_output(void)
{
//	Bit 0 of UINT8 value = I2C CLOCK signal
//	Bit 1 of UINT8 value = I2C DATA signal
//	Bit 7 set = error
//
// this procedure cannot identify I2C bus errors so bit 7 will never be set

	UINT16 wTemp;

	wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_OUTPUT_DATA);
	wTemp >>= 14;

	return ((UINT8) (wTemp));
}

// ====================+= common PMC GPIO loopback test ======================


/*****************************************************************************
 * xmc_gpio0_29_test:
 *
 * Test the PLX PEX8619 GPIO0 to GPIO29 interfaces routed through the XMC
 * connectors to the RTM module
 *
 * RETURNS: E__OK or error
 */
UINT32 xmc_gpio0_29_test (PCI_PFA pfa, UINT32 dConfig1, UINT32 dConfig2, UINT32 dOutputs, UINT32 dInputs, const ENXMC_GPIO_SIGNAL_PAIRS *loopback_table)
{
	UINT32	dRet = E__OK;
	UINT32	dMask;
	UINT32	dTemp;
	UINT16	wTemp;
	UINT16	wExpected;
	int		cnt;

#ifdef DEBUG
	vConsoleWrite("xmc_gpio0_29_test\n");
	sprintf (achBuffer," inputs=%08X, outputs=%08X\nconfig1=%08X, config2=%08X\n",dInputs, dOutputs, dConfig1, dConfig2);
	vConsoleWrite(achBuffer);
#endif

	dMask = dOutputs | dInputs;

#ifdef DEBUG
	vConsoleWrite("...set all bits high\n");
#endif

	// configure GPIOs in PLX PEX8619
	PCIE_WRITE_DWORD(pfa, PLX_GPIO1_DIR_CTL, dConfig1);
	PCIE_WRITE_DWORD(pfa, PLX_GPIO2_DIR_CTL, dConfig2);

	// set all bits high
	PCIE_WRITE_WORD(pfa, PLX_GPIO1_OUTPUT_DATA, (UINT16)(dOutputs & 0xFFFF));
	PCIE_WRITE_WORD(pfa, PLX_GPIO2_OUTPUT_DATA, (UINT16)(dOutputs >> 16));

	dTemp = (UINT32) PCIE_READ_WORD(pfa, PLX_GPIO1_INPUT_DATA);
	dTemp |= ((UINT32) PCIE_READ_WORD(pfa, PLX_GPIO2_INPUT_DATA)) << 16;
	dTemp &= dMask;
	if (dTemp != dInputs)
	{
		dRet = E__BAD_GPIO_LOOPBACK;
		sprintf (achBuffer, "XMC GPIO0 to GPIO29 loopback error:\nwrote 0x%08X, expected 0x%08X, read 0x%08X\n", dOutputs, dInputs, dTemp);
		vConsoleWrite(achBuffer);
	}

	// set all bits low
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...set all bits low\n");
#endif

		PCIE_WRITE_WORD(pfa, PLX_GPIO1_OUTPUT_DATA, 0);
		PCIE_WRITE_WORD(pfa, PLX_GPIO2_OUTPUT_DATA, 0);

		dTemp = (UINT32) PCIE_READ_WORD(pfa, PLX_GPIO1_INPUT_DATA);
		dTemp |= ((UINT32) PCIE_READ_WORD(pfa, PLX_GPIO2_INPUT_DATA)) << 16;
		dTemp &= dMask;
		if (dTemp != 0)
		{
			dRet = E__BAD_GPIO_LOOPBACK;
			sprintf (achBuffer, "XMC GPIO0 to GPIO29 loopback error:\nwrote 0, expected 0, read 0x%08X\n", dTemp);
			vConsoleWrite(achBuffer);
		}
	}

	// moving 1:
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...moving 1\n");
#endif

		for (cnt = 0; loopback_table[cnt].out != 0xFF; ++cnt)
		{
			if (loopback_table[cnt].out < 16)
			{
				wTemp = 0;
				wExpected = 0;
				setbit(wTemp, loopback_table[cnt].out);
				PCIE_WRITE_WORD(pfa, PLX_GPIO1_OUTPUT_DATA, wTemp);
				wTemp = PCIE_READ_WORD(pfa, PLX_GPIO1_INPUT_DATA);
				wTemp &= (UINT16)(dMask);
				setbit(wExpected, loopback_table[cnt].in);
				if (wTemp != wExpected)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achBuffer, "XMC GPIO0 to GPIO15 loopback error:\nwrote 0x%04X, expected 0x%04X, read 0x%04X\n",
									bitmask(loopback_table[cnt].out),
									wExpected,
									wTemp);
					vConsoleWrite(achBuffer);
					break;
				}
			}
			else
			{
				wTemp = 0;
				wExpected = 0;
				setbit(wTemp, (loopback_table[cnt].out - 16));
				PCIE_WRITE_WORD(pfa, PLX_GPIO2_OUTPUT_DATA, wTemp);
				wTemp = PCIE_READ_WORD(pfa, PLX_GPIO2_INPUT_DATA);
				wTemp &= (UINT16)(dMask >> 16);
				setbit(wExpected, (loopback_table[cnt].in - 16));
				if (wTemp != wExpected)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achBuffer, "XMC GPIO16 to GPIO29 loopback error:\nwrote 0x%04X, expected 0x%04X, read 0x%04X\n",
									bitmask(loopback_table[cnt].out - 16),
									wExpected,
									wTemp);
					vConsoleWrite(achBuffer);
					break;
				}
			}
		}
	}

	// moving 0:
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...moving 0\n");
#endif

		for (cnt = 0; loopback_table[cnt].out != 0xFF; ++cnt)
		{
			if (loopback_table[cnt].out < 16)
			{
				wTemp = 0xFFFF;
				wExpected = ((UINT16) dInputs);
				clrbit(wTemp, loopback_table[cnt].out);
				PCIE_WRITE_WORD(pfa, PLX_GPIO1_OUTPUT_DATA, wTemp);
				wTemp = PCIE_READ_WORD(pfa, PLX_GPIO1_INPUT_DATA);
				wTemp &= (UINT16)(dMask);
				clrbit(wExpected, loopback_table[cnt].in);
				wExpected &= (UINT16)(dMask);
				if (wTemp != (UINT16) wExpected)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achBuffer, "XMC GPIO0 to GPIO15 loopback error:\nwrote 0x%04X, expected 0x%04X, read 0x%04X\n",
									0xFFFF & ~(bitmask(loopback_table[cnt].out)),
									wExpected,
									wTemp);
					vConsoleWrite(achBuffer);
					break;
				}
			}
			else
			{
				wTemp = 0xFFFF;
				wExpected = (UINT16)(dInputs >> 16);
				clrbit(wTemp, (loopback_table[cnt].out - 16));
				PCIE_WRITE_WORD(pfa, PLX_GPIO2_OUTPUT_DATA, wTemp);
				wTemp = PCIE_READ_WORD(pfa, PLX_GPIO2_INPUT_DATA);
				wTemp &= (UINT16)(dMask >> 16);
				clrbit(wExpected, loopback_table[cnt].in - 16);
				if (wTemp != wExpected)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achBuffer, "XMC GPIO16 to GPIO29 loopback error:\nwrote 0x%04X, expected 0x%04X, read 0x%04X\n",
									0xFFFF & ~(bitmask(loopback_table[cnt].out - 16)),
									wExpected,
									wTemp);
					vConsoleWrite(achBuffer);
					break;
				}
			}
		}
	}

	return dRet;
}

UINT32 xmc_gpio32_47_test (UINT8 i2cGpioDev)
{
/*****************************************************************************
 * xmc_gpio32_47_test:
 *
 * Test the XMC GPIO 32 to GPIO47 interfaces routed through the XMC
 * connectors to the RTM module
 *
 * GPIOs are linked to next adjacent GPIO so GPIO32 is linked to GPIO33 etc
 * Odd numbered GPIOs must always be driven HIGH to maintain read inputs properly
 *
 * 32		33		i2c_1->0 <-> i2c_1->1
 * 34		35		i2c_1->2 <-> i2c_1->3
 * 36		37		i2c_1->4 <-> i2c_1->5
 * 38		39		i2c_1->6 <-> i2c_1->7
 *
 * 40		41		i2c_2->0 <-> i2c_2->4
 * 42		43		i2c_2->1 <-> i2c_2->5
 * 44		45		i2c_2->2 <-> i2c_2->6
 * 46		47		i2c_2->3 <-> i2c_2->7
 *
 * RETURNS: E__OK or error
 */

	UINT32 dRet;
	UINT8	bOutVal;
	UINT8	bInVal;
	int		cnt;

#ifdef DEBUG
	vConsoleWrite("xmc_gpio32_47_test");
	if (i2cGpioDev == I2C_PCA8574_1)
	{
		vConsoleWrite(" - testing gpio32 to 39\n");
	}
	else
	{
		vConsoleWrite(" - testing gpio40 to 47\n");
	}

#endif

#ifdef DEBUG
	vConsoleWrite("...set all bits high\n");
#endif

	// set all bits high:
	dRet = bitbangI2cWriteData (i2cGpioDev, 0xFF);
	if (dRet == E__OK)
	{
		dRet = bitbangI2cReadData (i2cGpioDev, &bInVal);
		if (dRet == E__OK)
		{
			if ((bInVal & 0xAA) != 0xAA)
			{
				dRet = E__BAD_GPIO_LOOPBACK;
				if (i2cGpioDev == I2C_PCA8574_1)
				{
					sprintf (achBuffer, "XMC GPIO32 to GPIO39 loopback error: wrote 0x55, read 0x%02X\n", bInVal & 0xAA);
				}
				else
				{
					sprintf (achBuffer, "XMC GPIO40 to GPIO47 loopback error: wrote 0x55, read 0x%02X\n", bInVal & 0xAA);
				}
				vConsoleWrite(achBuffer);
			}
		}
	}

	// set all bits low
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...set all bits low\n");
#endif

		dRet = bitbangI2cWriteData (i2cGpioDev, 0xAA);
		if (dRet == E__OK)
		{
			dRet = bitbangI2cReadData (i2cGpioDev, &bInVal);
			if (dRet == E__OK)
			{
				if ((bInVal & 0xAA) != 00)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					if (i2cGpioDev == I2C_PCA8574_1)
					{
						sprintf (achBuffer, "XMC GPIO32 to GPIO39 loopback error: wrote 0x00, read 0x%02X\n", bInVal & 0xAA);
					}
					else
					{
						sprintf (achBuffer, "XMC GPIO40 to GPIO47 loopback error: wrote 0x00, read 0x%02X\n", bInVal & 0xAA);
					}
					vConsoleWrite(achBuffer);
				}
			}
		}
	}

	// moving 1
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...moving 1\n");
#endif

		for (cnt = 0, bOutVal = 0xAB; cnt < 4; ++cnt)
		{
			dRet = bitbangI2cWriteData (i2cGpioDev, bOutVal);
			if (dRet == E__OK)
			{
				dRet = bitbangI2cReadData (i2cGpioDev, &bInVal);
				if (dRet == E__OK)
				{
					if ((bInVal & 0xAA) != ((bOutVal & 0x55) << 1))
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						if (i2cGpioDev == I2C_PCA8574_1)
						{
							sprintf (achBuffer, "XMC GPIO32 to GPIO39 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal & 0x55, bInVal & 0xAA);
						}
						else
						{
							sprintf (achBuffer, "XMC GPIO40 to GPIO47 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal & 0x55, bInVal & 0xAA);
						}
						vConsoleWrite(achBuffer);
						break;
					}
					bOutVal <<= 2;
					bOutVal |= 0xAA;
				}
			}
		}
	}

	// moving 0
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...moving 0\n");
#endif

		for (cnt = 0, bOutVal = 0xFE; cnt < 4; ++cnt)
		{
			dRet = bitbangI2cWriteData (i2cGpioDev, bOutVal);
			if (dRet == E__OK)
			{
				dRet = bitbangI2cReadData (i2cGpioDev, &bInVal);
				if (dRet == E__OK)
				{
					if ((bInVal & 0xAA) != ((bOutVal & 0x55) << 1))
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						if (i2cGpioDev == I2C_PCA8574_1)
						{
							sprintf (achBuffer, "XMC GPIO32 to GPIO39 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal & 0x55, bInVal & 0xAA);
						}
						else
						{
							sprintf (achBuffer, "XMC GPIO40 to GPIO47 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal & 0x55, bInVal & 0xAA);
						}
						vConsoleWrite(achBuffer);
						break;
					}
					bOutVal <<= 2;
					bOutVal |= 0xAB;
				}
			}
		}
	}

	return dRet;
}

/*****************************************************************************
 * xmc_gpio40_47_test:
 *
 * Test the XMC GPIO 40 to GPIO47 interfaces routed through the XMC
 * connectors to the RTM module
 *
 * uses GPIO 40 to 43 looped back to GPIO 44 to 47 respectively
 *
 * GPIO 44 to 47 must always be driven HIGH to maintain read inputs properly
 *
 * 40		44		i2c_2->0 <-> i2c_2->4
 * 41		45		i2c_2->1 <-> i2c_2->5
 * 42		46		i2c_2->2 <-> i2c_2->6
 * 43		47		i2c_2->3 <-> i2c_2->7
 *
 * RETURNS: E__OK or error
 */
UINT32 xmc_gpio40_47_test (void)
{
	UINT32	dRet = E__OK;
	UINT8	bOutVal;
	UINT8	bInVal;
	int		cnt;

#ifdef DEBUG
	vConsoleWrite("xmc_gpio40_47_test\n");
#endif

#ifdef DEBUG
	vConsoleWrite("...set all bits high\n");
#endif

	// set all bits high:
	dRet = bitbangI2cWriteData (I2C_PCA8574_2, 0xFF);			// GPIO40 to GPIO43
	if (dRet == E__OK)
	{
		dRet = bitbangI2cReadData (I2C_PCA8574_2, &bInVal);		// GPIO44 to GPIO47
		if (dRet == E__OK)
		{
			if ((bInVal & 0xF0) != 0xF0)
			{
				dRet = E__BAD_GPIO_LOOPBACK;
				sprintf (achBuffer, "XMC GPIO40 to GPIO47 loopback error: wrote 0xF, read 0x%X\n", (bInVal & 0xF0) >> 4);
				vConsoleWrite(achBuffer);
			}
		}
	}

	// set all bits low
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...set all bits low\n");
#endif

		dRet = bitbangI2cWriteData (I2C_PCA8574_2, 0xF0);
		if (dRet == E__OK)
		{
			dRet = bitbangI2cReadData (I2C_PCA8574_2, &bInVal);
			if (dRet == E__OK)
			{
				if ((bInVal & 0xF0) != 00)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achBuffer, "XMC GPIO40 to GPIO47 loopback error: wrote 0x0, read 0x%X\n", (bInVal & 0xF0) >> 4);
					vConsoleWrite(achBuffer);
				}
			}
		}
	}

	// moving 1
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...moving 1\n");
#endif

		for (cnt = 0, bOutVal = 0xF1; cnt < 4; ++cnt)
		{
			dRet = bitbangI2cWriteData (I2C_PCA8574_2, bOutVal);
			if (dRet == E__OK)
			{
				dRet = bitbangI2cReadData (I2C_PCA8574_2, &bInVal);
				if (dRet == E__OK)
				{
					if (((bInVal & 0xF0) >> 4) != (bOutVal & 0x0F))
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "XMC GPIO40 to GPIO47 loopback error: wrote 0x%X, read 0x%X\n", bOutVal & 0x0F, (bInVal & 0xF0) >> 4);
						vConsoleWrite(achBuffer);
						break;
					}
					bOutVal <<= 1;
					bOutVal |= 0xF0;
				}
			}
		}
	}

	// moving 0
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...moving 0\n");
#endif

		for (cnt = 0, bOutVal = 0xFE; cnt < 4; ++cnt)
		{
			dRet = bitbangI2cWriteData (I2C_PCA8574_2, bOutVal);
			if (dRet == E__OK)
			{
				dRet = bitbangI2cReadData (I2C_PCA8574_2, &bInVal);
				if (dRet == E__OK)
				{
					if (((bInVal & 0xF0) >> 4) != (bOutVal & 0x0F))
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "XMC GPIO40 to GPIO47 loopback error: wrote 0x%X, read 0x%X\n", bOutVal & 0x0F, (bInVal & 0xF0) >> 4);
						vConsoleWrite(achBuffer);
						break;
					}
					bOutVal <<= 1;
					bOutVal |= 0xF1;
				}
			}
		}
	}

	return dRet;
}

/*****************************************************************************
 * pmc_gpio_loopback:
 *
 * Test the PLX PEX8619 GPIO interfaces routed through the PMC
 * connectors to the RTM module
 *
 * GPIO0 to GPIO15 are looped back to GPIO32 to GPIO47
 *
 * RETURNS: E__OK or error
 */
UINT32 pmc_gpio_loopback (void)
{
/*	GPIOs of PLX PEX8619 looped back through I2C I/O Expander on EnXMC/002
 *
 * PMC connector
 *
 * PLX		EnXMC	I2C I/O
 * GPIO		GPIO	Expander
 * 00		32		i2c_1->0
 * 01		33		i2c_1->1
 * 02		34		i2c_1->2
 * 03		35		i2c_1->3
 * 04		36		i2c_1->4
 * 05		37		i2c_1->5
 * 06		38		i2c_1->6
 * 07		39		i2c_1->7
 *
 * 08		40		i2c_2->0
 * 09		41		i2c_2->1
 * 10		42		i2c_2->2
 * 11		43		i2c_2->3
 * 12		44		i2c_2->4
 * 13		45		i2c_2->5
 * 14		46		i2c_2->6
 * 15		47		i2c_2->7
 *
 */

	UINT32	dRet = E__OK;
	UINT16	wTemp;
	UINT8	bOutVal;
	int		cnt;

	ENXMC_GPIO_LOOPBACK_INFO * loopbackInfo;

#ifdef DEBUG
	vConsoleWrite("pmc_gpio_loopback\n");
#endif

	// Get switch port info for the VRE1x and RTM
	board_service(SERVICE__BRD_GET_ENXMC_GPIO_INFO, &dRet, &loopbackInfo);

#ifdef DEBUG
	sprintf (achBuffer, "loopbackInfo->plx8619Instance = %d\n", loopbackInfo->plx8619Instance);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dConfig1 = 0x%08X\n", loopbackInfo->dConfig1);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dConfig2 = 0x%08X\n", loopbackInfo->dConfig2);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dOutputs = 0x%08X\n", loopbackInfo->dOutputs);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dInputs = 0x%08X\n", loopbackInfo->dInputs);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->gpioTable = 0x%08X\n", (UINT32)loopbackInfo->gpioTable);
	vConsoleWrite(achBuffer);
#endif

	// upstream port is port 1 of PLX8619
	// so step over port 0 and DMA function to get to port1 on PCI Bus
	dRet = iPciFindDeviceById (loopbackInfo->plx8619Instance, 0x10B5, 0x8619, &enxmcPfa);

	// configure GPIO0 to GPIO15 as inputs
	PCIE_WRITE_DWORD(enxmcPfa, PLX_GPIO1_DIR_CTL, 0);		// GPIO0 to GPIO15 all inputs

#ifdef DEBUG
	vConsoleWrite("...set all bits high\n");
#endif

	// set all bits high
	dRet = bitbangI2cWriteData (I2C_PCA8574_1, 0xFF);		// GPIO32 to GPIO39
	if (dRet == E__OK)
	{
		dRet = bitbangI2cWriteData (I2C_PCA8574_2, 0xFF);	// GPIO40 to GPIO47
		if (dRet == E__OK)
		{
			wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO1_INPUT_DATA);
			if (wTemp != 0xFFFF)
			{
				dRet = E__BAD_GPIO_LOOPBACK;
				sprintf (achBuffer, "PMC GPIO loopback error: wrote 0xFFFF, read 0x%04X\n", wTemp);
				vConsoleWrite(achBuffer);
			}
		}
	}

	// set all bits low
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...set all bits low\n");
#endif

		dRet = bitbangI2cWriteData (I2C_PCA8574_1, 0);		// GPIO32 to GPIO39
		if (dRet == E__OK)
		{
			dRet = bitbangI2cWriteData (I2C_PCA8574_2, 0);	// GPIO40 to GPIO47
			if (dRet == E__OK)
			{
				wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO1_INPUT_DATA);
				if (wTemp != 0)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achBuffer, "PMC GPIO loopback error: wrote 0x0000, read 0x%04X\n", wTemp);
					vConsoleWrite(achBuffer);
				}
			}
		}
	}

	// moving 1
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...moving 1\n");
#endif

		for (cnt = 0, bOutVal = 1; cnt < 16; ++cnt)
		{
			// drive outputs
			if (cnt < 8)
			{
				dRet = bitbangI2cWriteData (I2C_PCA8574_1, bOutVal);		// GPIO32 to GPIO39
				if (dRet == E__OK)
					dRet = bitbangI2cWriteData (I2C_PCA8574_2, 0);			// GPIO40 to GPIO47
			}
			else
			{
				if (cnt == 8)
					bOutVal = 1;

				dRet = bitbangI2cWriteData (I2C_PCA8574_1, 0);				// GPIO32 to GPIO39
				if (dRet == E__OK)
					dRet = bitbangI2cWriteData (I2C_PCA8574_2, bOutVal);	// GPIO40 to GPIO47
			}
			// read inputs
			if (dRet == E__OK)
			{
				wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO1_INPUT_DATA);
				if (cnt < 8)
				{
					if (wTemp != ((UINT16) bOutVal))
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "PMC GPIO loopback error: wrote 0x00%02X, read 0x%04X\n",
											bOutVal, wTemp);
					}
				}
				else
				{
					if (wTemp != (((UINT16) bOutVal) << 8))
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "PMC GPIO loopback error: wrote 0x%02X00, read 0x%04X\n",
											bOutVal, wTemp);
					}
				}
				if (dRet != E__OK)
				{
					vConsoleWrite(achBuffer);
					break;
				}
				bOutVal <<= 1;
			}
		}
	}

	// moving 0
	if (dRet == E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("...moving 0\n");
#endif

		for (cnt = 0, bOutVal = 0xFE; cnt < 16; ++cnt)
		{
			// drive outputs
			if (cnt < 8)
			{
				dRet = bitbangI2cWriteData (I2C_PCA8574_1, bOutVal);		// GPIO32 to GPIO39
				if (dRet == E__OK)
					dRet = bitbangI2cWriteData (I2C_PCA8574_2, 0xFF);		// GPIO40 to GPIO47
			}
			else
			{
				if (cnt == 8)
					bOutVal = 0xFE;

				dRet = bitbangI2cWriteData (I2C_PCA8574_1, 0xFF);			// GPIO32 to GPIO39
				if (dRet == E__OK)
					dRet = bitbangI2cWriteData (I2C_PCA8574_2, bOutVal);	// GPIO40 to GPIO47

			}
			// read inputs
			if (dRet == E__OK)
			{
				wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO1_INPUT_DATA);
				if (cnt < 8)
				{
					if (wTemp != (0xFF00 + (UINT16) bOutVal))
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "PMC GPIO loopback error: wrote 0x%x04, read 0x%04X\n",
											(0xFF00 + (UINT16) bOutVal), wTemp);
					}
				}
				else
				{
					if (wTemp != (((UINT16) bOutVal) << 8) + 0x00FF)
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "PMC GPIO loopback error: wrote 0x%0x04, read 0x%04X\n",
											((((UINT16) bOutVal) << 8) + 0x00FF), wTemp);
					}
				}
				if (dRet != E__OK)
				{
					vConsoleWrite(achBuffer);
					break;
				}
				bOutVal <<= 1;
				bOutVal |= 1;
			}
		}
	}

	return dRet;
}

// =============== board specific XMC GPIO loopback routines =================

/*****************************************************************************
 * xmc_gpio_loopback_ADCR6:
 *
 *	Test the PLX PEX8619 GPIO interfaces routed through the XMC
 *	connectors to the RTM module
 *
 * RETURNS: E__OK or error
 */
UINT32 xmc_gpio_loopback_ADCR6 (UINT32 site)
{
/*	GPIOs of PLX PEX8619 looped back through I2C I/O Expander on EnXMC/002
 *
 * XMC connectors on ADCR6
 *
 * EnXMC	EnXMC	I2C I/O
 * GPIO		GPIO	Expander
 * 00		02									SITE 2 only
 * 01		03									SITE 2 only
 * 04		06									SITE 2 only
 * 05		07									SITE 2 only
 * 08		10									SITE 2 only
 * 09		11									SITE 2 only
 * 12		14									SITE 2 only
 * 13		15									SITE 2 only
 * 16		18									SITE 2 only
 * 17		19									SITE 2 only
 * 20		22									SITE 2 only
 * 21		23									SITE 2 only
 * 24		26									SITE 2 only
 * 25		27									SITE 2 only

 * 30		31		Reserved for I2C bit-banging

 * 32		28		i2c_1->0 <-> EnXMC GPIO 28	SITE 2 only
 * 33		29		i2c_1->1 <-> EnXMC GPIO 29	SITE 2 only
 * 34		36		i2c_1->2 <-> i2c_1->4		SITE 2 only
 * 35		37		i2c_1->3 <-> i2c_1->5		SITE 2 only
 * 38		39		i2c_1->6 <-> i2c_1->7		SITE 2 only

 * GPIO0 to GPIO29 not used on XMC Site 1

 * 30		31		Reserved for I2C bit-banging

 * GPIO33 to GPIO39 not used on XMC SITE 1

 * 40		41		i2c_2->0 <-> i2c_2->4		SITE 1 & 2
 * 42		43		i2c_2->1 <-> i2c_2->5		SITE 1 & 2
 * 44		45		i2c_2->2 <-> i2c_2->6		SITE 1 & 2
 * 46		47		i2c_2->3 <-> i2c_2->7		SITE 1 & 2
 */

	UINT32	dRet;
	UINT8	bOutVal;
	UINT8	bInVal;
	UINT16	wTemp;
	int		cnt;

	ENXMC_GPIO_LOOPBACK_INFO siteParam;					// use this structure to pass XMC site to service
	ENXMC_GPIO_LOOPBACK_INFO * loopbackInfo;

#ifdef DEBUG
	vConsoleWrite("xmc_gpio_loopback_ADCR6\n");
	sprintf (achBuffer, "XMC site = %d\n", site);
	vConsoleWrite(achBuffer);
#endif

	// site 1 : tests XMC GPIO: 22 to 29 and 32 to 47
	// site 2 : tests XMC GPIO: 0 to 29 and 32 to 47

	// Get GPIO configuration info for the VX81x and ADCR6

	//pass XMC site to service using loopbackInfo->plx8619Instance
	siteParam.plx8619Instance = (UINT8) site;
	loopbackInfo = &siteParam;
	board_service(SERVICE__BRD_GET_ENXMC_GPIO_INFO, &dRet, &loopbackInfo);

#ifdef DEBUG
	sprintf (achBuffer, "loopbackInfo->plx8619Instance = %d\n", loopbackInfo->plx8619Instance);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dConfig1 = 0x%08X\n", loopbackInfo->dConfig1);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dConfig2 = 0x%08X\n", loopbackInfo->dConfig2);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dOutputs = 0x%08X\n", loopbackInfo->dOutputs);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dInputs = 0x%08X\n", loopbackInfo->dInputs);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->gpioTable = 0x%08X\n", (UINT32)loopbackInfo->gpioTable);
	vConsoleWrite(achBuffer);
#endif
	// upstream port is port 1 of PLX8619
	// so step over port 0 and DMA function to get to port1 on PCI Bus
	dRet = iPciFindDeviceById (loopbackInfo->plx8619Instance, 0x10B5, 0x8619, &enxmcPfa);

	// Site 2 only
	if (site == 2)
	{
		if (dRet == E__OK)
		{
			// site 2 tests GPIO0 to 27
			dRet = xmc_gpio0_29_test (enxmcPfa,
										loopbackInfo->dConfig1,
										loopbackInfo->dConfig2,
										loopbackInfo->dOutputs,
										loopbackInfo->dInputs,
										loopbackInfo->gpioTable);
		}
		// test XMC GPIO 28, 29 and 32 to 39
		// GPIO 28 and 29 are configured as inputs in the PLX8619
		// GPIO 32 to 39 are driven by the first I2C I/O Expander
		// GPIO 36, 37 and  39 must always be driven HIGH to maintain read inputs properly

		// set all bits high:
		if (dRet == E__OK)
		{
		
#ifdef DEBUG
			vConsoleWrite("...set all bits high - XMC GPIO 28, 29 and 32 to 39\n");
#endif

			dRet = bitbangI2cWriteData (I2C_PCA8574_1, 0xFF);			// GPIO32 to GPIO39
			if (dRet == E__OK)
			{
				dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);		// GPIO32 to GPIO39
				if (dRet == E__OK)
				{
					if ((bInVal & 0xB0) != 0xB0)
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x4C, read 0x%02X\n", (bInVal & 0xB0) >> 4);
						vConsoleWrite(achBuffer);
					}

					wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
					if ((wTemp & 0x3000) != 0x3000)
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "XMC GPIO28, 29, 32, 33 loopback error: wrote 0x3, read 0x%02X\n", (wTemp & 0x3000) >> 12);
						vConsoleWrite(achBuffer);
					}
				}

#ifdef DEBUG
				sprintf (achBuffer, "XMC GPIO28, 29, 32, and 33 loopback: wrote 0x03, read 0x%02X\n", (wTemp & 0x3000) >> 12);
				vConsoleWrite(achBuffer);
				sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback: wrote 0xFC, read 0x%02X(0x%02X)\n", bInVal, bInVal & 0xB0);
				vConsoleWrite(achBuffer);
#endif

			}
		}

#ifdef DEBUG
		vConsoleWrite("...set all bits low\n");
#endif
		// set all bits low
		if (dRet == E__OK)
		{
			dRet = bitbangI2cWriteData (I2C_PCA8574_1, 0xB0);
			if (dRet == E__OK)
			{
				dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);
				if (dRet == E__OK)
				{
					if ((bInVal & 0xB0) != 00)
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x00, read 0x%02X\n", (bInVal & 0xB0) >> 4);
						vConsoleWrite(achBuffer);
					}
				}

				wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
				if ((wTemp & 0x3000) != 0x0000)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achBuffer, "XMC GPIO28, 29, 32, 33 loopback error: wrote 0x00, read 0x%02X\n", (wTemp & 0x3000) >> 12);
					vConsoleWrite(achBuffer);
				}

#ifdef DEBUG
				sprintf (achBuffer, "XMC GPIO28, 29, 32, and 33 loopback: wrote 0x00, read 0x%02X\n", (wTemp & 0x3000) >> 12);
				vConsoleWrite(achBuffer);
				sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback: wrote 0x00, read 0x%02X(0x%02X)\n", bInVal, bInVal & 0xB0);
				vConsoleWrite(achBuffer);
#endif

			}
		}

#ifdef DEBUG
		vConsoleWrite("...moving 1\n");
#endif

		// moving 1
		if (dRet == E__OK)
		{
			for (cnt = 0, bOutVal = 0xB1; cnt < 5; ++cnt)
			{
				dRet = bitbangI2cWriteData (I2C_PCA8574_1, bOutVal);
				if (dRet == E__OK)
				{

					switch (cnt)
					{
						case 0:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO32 to GPIO28 loopback: wrote 0x01, read 0x%02X\n", (wTemp & 0x3000) >> 12);
							vConsoleWrite(achBuffer);
#endif

							if ((wTemp & 0x3000) != 0x1000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO28 and GPIO32 loopback error: wrote 0x1, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							bOutVal = 0xB2;
							break;

						case 1:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO33 to GPIO29 loopback: wrote 0x02, read 0x%02X\n", (wTemp & 0x3000) >> 12);
							vConsoleWrite(achBuffer);
#endif

							if ((wTemp & 0x3000) != 0x2000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO29 and GPIO33 loopback error: wrote 0x2, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							bOutVal = 0xB4;
							break;

						case 2:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback: wrote 0x%02X, read 0x%02X(0x%02X)\n", bOutVal, bInVal, bInVal & 0xB0);
							vConsoleWrite(achBuffer);
#endif

							if (dRet == E__OK)
							{
								if (bInVal != 0x14)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							bOutVal = 0xB8;
							break;

						case 3:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback: wrote 0x%02X, read 0x%02X(0x%02X)\n", bOutVal, bInVal, bInVal & 0xB0);
							vConsoleWrite(achBuffer);
#endif

							if (dRet == E__OK)
							{
								if (bInVal != 0x28)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							bOutVal = 0xF0;
							break;

						case 4:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback: wrote 0x%02X, read 0x%02X(0x%02X)\n", bOutVal, bInVal, bInVal & 0xB0);
							vConsoleWrite(achBuffer);
#endif

							if (dRet == E__OK)
							{
								if (bInVal != 0xC0)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							break;
					}
				}
			}
		}

#ifdef DEBUG
		vConsoleWrite("...moving 0\n");
#endif
		// moving 0
		if (dRet == E__OK)
		{
			for (cnt = 0, bOutVal = 0xFE; cnt < 5; ++cnt)
			{
				dRet = bitbangI2cWriteData (I2C_PCA8574_1, bOutVal);
				if (dRet == E__OK)
				{

					switch (cnt)
					{
						case 0:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO32 to GPIO28 loopback: wrote 0x02, read 0x%02X\n", (wTemp & 0x3000) >> 12);
							vConsoleWrite(achBuffer);
#endif

							if ((wTemp & 0x3000) != 0x2000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO28 and GPIO32 loopback error: wrote 0x2, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							bOutVal = 0xFD;
							break;

						case 1:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO33 to GPIO29 loopback: wrote 0x01, read 0x%02X\n", (wTemp & 0x3000) >> 12);
							vConsoleWrite(achBuffer);
#endif

							if ((wTemp & 0x3000) != 0x1000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO29 and GPIO33 loopback error: wrote 0x1, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							bOutVal = 0xFB;
							break;

						case 2:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO33 to GPIO29 loopback: wrote 0x03, read 0x%02X\n", (wTemp & 0x3000) >> 12);
							vConsoleWrite(achBuffer);
#endif

							if ((wTemp & 0x3000) != 0x3000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO28, 29, 32 and 33 loopback error: wrote 0x3, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							
#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback: wrote 0x%02X, read 0x%02X(0x%02X)\n", bOutVal, bInVal, bInVal & 0xB0);
							vConsoleWrite(achBuffer);
#endif

							if (dRet == E__OK)
							{
								if ((bInVal & 0xFC) != 0xE8)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							bOutVal = 0xF7;
							break;

						case 3:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback: wrote 0x%02X, read 0x%02X(0x%02X)\n", bOutVal, bInVal, bInVal & 0xB0);
							vConsoleWrite(achBuffer);
#endif

							if (dRet == E__OK)
							{
								if ((bInVal & 0xFC) != 0xD4)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							bOutVal = 0xBF;
							break;

						case 4:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);

#ifdef DEBUG
							sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback: wrote 0x%02X, read 0x%02X(0x%02X)\n", bOutVal, bInVal, bInVal & 0xB0);
							vConsoleWrite(achBuffer);
#endif

							if (dRet == E__OK)
							{
								if ((bInVal & 0xFC) != 0x3C)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%02X, read 0x%02X\n", bOutVal, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							break;
					}
				}
			}
		}
	}

	// both sites
	// test XMC GPIO 40 to 47
	// GPIOs are linked to next adjacent GPIO so GPIO40 is linked to GPIO41 etc
	if (dRet == E__OK)
	{
		dRet = xmc_gpio32_47_test(I2C_PCA8574_2);			// second I2C I/O Expander
	}

	return dRet;
}

/*****************************************************************************
 * xmc_gpio_loopback_TRB1x:
 *
 *	Test the PLX PEX8619 GPIO interfaces routed through the XMC
 *	connectors to the RTM module
 *
 * RETURNS: E__OK or error
 */
UINT32 xmc_gpio_loopback_TRB1X (void)
{
/*	GPIOs of PLX PEX8619 looped back through I2C I/O Expander on EnXMC/002
 *
 * XMC connectors on TRB1x
 *
 * EnXMC	EnXMC	I2C I/O
 * GPIO		GPIO	Expander
 * 00		02
 * 01		03
 * 04		06
 * 05		07
 * 08		10
 * 09		11
 * 12		14
 * 13		15
 * 16		18
 * 17		19
 * 20		22
 * 21		23

 * 30		31		Reserved for I2C bit-banging

 * 40		44		i2c_2->0 <-> i2c_2->4
 * 41		45		i2c_2->1 <-> i2c_2->5
 * 42		46		i2c_2->2 <-> i2c_2->6
 * 43		47		i2c_2->3 <-> i2c_2->7
 */

	UINT32	dRet = E__OK;

	ENXMC_GPIO_LOOPBACK_INFO * loopbackInfo;

#ifdef DEBUG
	vConsoleWrite("xmc_gpio_loopback_TRB1x\n");
#endif

	// Get switch port info for the VRE1x and RTM
	board_service(SERVICE__BRD_GET_ENXMC_GPIO_INFO, &dRet, &loopbackInfo);

#ifdef DEBUG
	sprintf (achBuffer, "loopbackInfo->plx8619Instance = %d\n", loopbackInfo->plx8619Instance);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dConfig1 = 0x%08X\n", loopbackInfo->dConfig1);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dConfig2 = 0x%08X\n", loopbackInfo->dConfig2);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dOutputs = 0x%08X\n", loopbackInfo->dOutputs);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dInputs = 0x%08X\n", loopbackInfo->dInputs);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->gpioTable = 0x%08X\n", (UINT32)loopbackInfo->gpioTable);
	vConsoleWrite(achBuffer);
#endif
	// upstream port is port 1 of PLX8619
	// so step over port 0 and DMA function to get to port1 on PCI Bus
	dRet = iPciFindDeviceById (loopbackInfo->plx8619Instance, 0x10B5, 0x8619, &enxmcPfa);

	// test XMC GPIO: 0 to 23
	if (dRet == E__OK)
	{
		dRet = xmc_gpio0_29_test (enxmcPfa,
									loopbackInfo->dConfig1,
									loopbackInfo->dConfig2,
									loopbackInfo->dOutputs,
									loopbackInfo->dInputs,
									loopbackInfo->gpioTable);
	}

	// test XMC GPIO 40 to 47
	if (dRet == E__OK)
	{
		dRet = xmc_gpio40_47_test();	// second I2C I/O Expander
	}

	return dRet;
}

/*****************************************************************************
 * xmc_gpio_loopback_VRE1X:
 *
 *	Test the PLX PEX8619 GPIO interfaces routed through the XMC
 *	connectors to the RTM module
 *
 * RETURNS: E__OK or error
 */
UINT32 xmc_gpio_loopback_VRE1X (UINT32 site)
{
/*	GPIOs of PLX PEX8619 looped back through I2C I/O Expander on EnXMC/002
 *
 * XMC connectors on VRE1x
 *
 * EnXMC	EnXMC	I2C I/O
 * GPIO		GPIO	Expander
 * 00		02											SITE 1 only
 * 01		03											SITE 1 only
 * 04		06											SITE 1 only
 * 05		07											SITE 1 only
 * 08		10											SITE 1 only
 * 09		11											SITE 1 only
 * 12		14											SITE 1 only
 * 13		15											SITE 1 only
 * 16		18											SITE 1 only
 * 17		19											SITE 1 only
 * 20		22											SITE 1 only
 * 21		23											SITE 1 only
 * 24		26											SITE 1 only
 * 25		27											SITE 1 only

 * 30		31		Reserved for I2C bit-banging

 * 32		28		i2c_1->0 <-> EnXMC GPIO 28			SITE 1 only
 * 33		29		i2c_1->1 <-> EnXMC GPIO 29			SITE 1 only
 * 34		36,38	i2c_1->2 <-> i2c_1->4, i2c_1->6		SITE 1 only
 * 35		37,39	i2c_1->3 <-> i2c_1->5, i2c_1->7		SITE 1 only

 * 40		44		i2c_2->0 <-> i2c_2->4				SITE 1 & 2
 * 41		45		i2c_2->1 <-> i2c_2->5				SITE 1 & 2
 * 42		46		i2c_2->2 <-> i2c_2->6				SITE 1 & 2
 * 43		47		i2c_2->3 <-> i2c_2->7				SITE 1 & 2
 */

	UINT32	dRet = E__OK;
	UINT16	wTemp;
	UINT8	bOutVal;
	UINT8	bInVal;
	int		cnt;

	ENXMC_GPIO_LOOPBACK_INFO * loopbackInfo;

#ifdef DEBUG
	vConsoleWrite("xmc_gpio_loopback_VRE1x\n");
#endif

	// site 1 : tests XMC GPIO: 0 to 29 and 32 to 47
	// site 2 : tests XMC GPIO: 40 to 47 only

	// Get switch port info for the VRE1x and RTM
	board_service(SERVICE__BRD_GET_ENXMC_GPIO_INFO, &dRet, &loopbackInfo);

#ifdef DEBUG
	sprintf (achBuffer, "loopbackInfo->plx8619Instance = %d\n", loopbackInfo->plx8619Instance);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dConfig1 = 0x%08X\n", loopbackInfo->dConfig1);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dConfig2 = 0x%08X\n", loopbackInfo->dConfig2);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dOutputs = 0x%08X\n", loopbackInfo->dOutputs);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->dInputs = 0x%08X\n", loopbackInfo->dInputs);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer, "loopbackInfo->gpioTable = 0x%08X\n", (UINT32)loopbackInfo->gpioTable);
	vConsoleWrite(achBuffer);
#endif
	// upstream port is port 1 of PLX8619
	// so step over port 0 and DMA function to get to port1 on PCI Bus
	dRet = iPciFindDeviceById (loopbackInfo->plx8619Instance, 0x10B5, 0x8619, &enxmcPfa);

	if (site == 1)
	{
		// test XMC GPIO: 0 to 27
		dRet = xmc_gpio0_29_test (enxmcPfa,
									loopbackInfo->dConfig1,
									loopbackInfo->dConfig2,
									loopbackInfo->dOutputs,
									loopbackInfo->dInputs,
									loopbackInfo->gpioTable);

		// test XMC GPIO 28, 29 and 32 to 39
		// GPIO 28 and 29 are configured as inputs in the PLX8619
		// GPIO 32 to 39 are driven by the first I2C I/O Expander
		// GPIO 36 to 39 must always be driven HIGH to maintain read inputs properly

#ifdef DEBUG
		vConsoleWrite("...set all bits high - XMC GPIO 28, 29 and 32 to 39\n");
#endif

		// set all bits high:
		if (dRet == E__OK)
		{
			dRet = bitbangI2cWriteData (I2C_PCA8574_1, 0xFF);			// GPIO32 to GPIO39
			if (dRet == E__OK)
			{
				dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);		// GPIO32 to GPIO39
				if (dRet == E__OK)
				{
					if ((bInVal & 0xF0) != 0xF0)
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0xC, read 0x%X\n", (bInVal & 0xF0) >> 4);
						vConsoleWrite(achBuffer);
					}

					wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
					if ((wTemp & 0x3000) != 0x3000)
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "XMC GPIO28, 29, 32, 33 loopback error: wrote 0x3, read 0x%X\n", (wTemp & 0x3000) >> 12);
						vConsoleWrite(achBuffer);
					}
				}
			}
		}

#ifdef DEBUG
		vConsoleWrite("...set all bits low\n");
#endif
		// set all bits low
		if (dRet == E__OK)
		{
			dRet = bitbangI2cWriteData (I2C_PCA8574_1, 0xF0);
			if (dRet == E__OK)
			{
				dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);
				if (dRet == E__OK)
				{
					if ((bInVal & 0xF0) != 00)
					{
						dRet = E__BAD_GPIO_LOOPBACK;
						sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x0, read 0x%X\n", (bInVal & 0xF0) >> 4);
						vConsoleWrite(achBuffer);
					}
				}

				wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
				if ((wTemp & 0x3000) != 0x0000)
				{
					dRet = E__BAD_GPIO_LOOPBACK;
					sprintf (achBuffer, "XMC GPIO28, 29, 32, 33 loopback error: wrote 0x0, read 0x%X\n", (wTemp & 0x3000) >> 12);
					vConsoleWrite(achBuffer);
				}
			}
		}

#ifdef DEBUG
		vConsoleWrite("...moving 1\n");
#endif

		// moving 1
		if (dRet == E__OK)
		{
			for (cnt = 0, bOutVal = 0xF1; cnt < 4; ++cnt)
			{
				dRet = bitbangI2cWriteData (I2C_PCA8574_1, bOutVal);
				if (dRet == E__OK)
				{

					switch (cnt)
					{
						case 0:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
							if ((wTemp & 0x3000) != 0x1000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO28 and GPIO32 loopback error: wrote 0x1, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							break;

						case 1:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
							if ((wTemp & 0x3000) != 0x2000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO29 and GPIO33 loopback error: wrote 0x2, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							break;

						case 2:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);
							if (dRet == E__OK)
							{
								bInVal >>= 4;
								if (bInVal != 0x05)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%X, read 0x%X\n", bOutVal & 0x0F, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							break;

						case 3:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);
							if (dRet == E__OK)
							{
								bInVal >>= 4;
								if (bInVal != 0x0A)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%X, read 0x%X\n", bOutVal & 0x0F, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							break;
					}
					bOutVal <<= 1;
					bOutVal |= 0xF0;
				}
			}
		}

#ifdef DEBUG
		vConsoleWrite("...moving 0\n");
#endif
		// moving 0
		if (dRet == E__OK)
		{
			for (cnt = 0, bOutVal = 0xFE; cnt < 4; ++cnt)
			{
				dRet = bitbangI2cWriteData (I2C_PCA8574_1, bOutVal);
				if (dRet == E__OK)
				{

					switch (cnt)
					{
						case 0:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
							if ((wTemp & 0x3000) != 0x2000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO28 and GPIO32 loopback error: wrote 0x2, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							break;

						case 1:
							wTemp = PCIE_READ_WORD(enxmcPfa, PLX_GPIO2_INPUT_DATA);
							if ((wTemp & 0x3000) != 0x1000)
							{
								dRet = E__BAD_GPIO_LOOPBACK;
								sprintf (achBuffer, "XMC GPIO29 and GPIO33 loopback error: wrote 0x1, read 0x%X\n", (wTemp & 0x3000) >> 12);
								vConsoleWrite(achBuffer);
							}
							break;

						case 2:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);
							if (dRet == E__OK)
							{
								bInVal >>= 4;
								if (bInVal != 0x0A)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%X, read 0x%X\n", bOutVal & 0x0F, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							break;

						case 3:
							dRet = bitbangI2cReadData (I2C_PCA8574_1, &bInVal);
							if (dRet == E__OK)
							{
								bInVal >>= 4;
								if (bInVal != 0x05)
								{
									dRet = E__BAD_GPIO_LOOPBACK;
									sprintf (achBuffer, "XMC GPIO34 to GPIO39 loopback error: wrote 0x%X, read 0x%X\n", bOutVal & 0x0F, bInVal);
									vConsoleWrite(achBuffer);
								}
							}
							break;
					}
					bOutVal <<= 1;
					bOutVal |= 0xF1;
				}
			}
		}
	}

	// for sites 1 and 2
	// test XMC GPIO 40 to 47
	if (dRet == E__OK)
	{
		dRet = xmc_gpio40_47_test();	// second I2C I/O Expander
	}

	return dRet;
}

/*****************************************************************************
 * Enxmc_pex8619_gpio: Test the PLX PEX8619 GPIO interfaces routed through the XMC and
 *					   PMC connectors to the RTM module
 *
 * RETURNS: None
 */
TEST_INTERFACE (Enxmc_Pex8619_Gpio_Test, "ENXMC PMC/XMC GPIO Test")
{
	UINT32	dRet = E__OK;

	//	PMC loopback test requires TF596
	//	XMC loopback tests require TF582

#ifdef DEBUG
	vConsoleWrite("Enxmc_Pex8619_Gpio_Test\n");
	sprintf(achBuffer, "adTestParams[0,1,2] = [%d,%d,%d]\n", adTestParams[0], adTestParams[1], adTestParams[2]);
	vConsoleWrite(achBuffer);
#endif

	// adTestparams[1] == 1 : PMC loopback test
	// adTestparams[1] == 2 : XMC loopback test ADCR6 : also requires adTestParams[2]
	// adTestparams[1] == 3 : XMC loopback test TRB1x
	// adTestparams[1] == 4 : XMC loopback test VRE1x : also requires adTestParams[2]

	if ((adTestParams[0] == 0) || (adTestParams[0] > 2))
	{
		dRet = E__FAIL;
	}
	else
	{
		if ((adTestParams[1] < 1) || (adTestParams[1] > 4))
			dRet = E__FAIL;
	}

	if (dRet == E__OK)
	{
		bitbangI2cInitHwIface (&enxmc_pex8619_gpio_read_I2C_input,
								&enxmc_pex8619_gpio_read_I2C_output,
								&enxmc_pex8619_gpio_write_I2C);

		switch (adTestParams[1])
		{
			case 1:		// PMC GPIO
				dRet = pmc_gpio_loopback ();
				break;

			case 2:		// ADCR6 XMC GPIO - requires adTestParam[2]
				if (adTestParams[0] != 2)
					dRet = E__FAIL;
				else if ((adTestParams[2] == 1) || (adTestParams[2] == 2))
					dRet = xmc_gpio_loopback_ADCR6 (adTestParams[2]);
				else
					dRet = E__FAIL;

				break;

			case 3:		// TRB1x XMC GPIO
				dRet = xmc_gpio_loopback_TRB1X ();
				break;

			case 4:		// VRE1x XMC GPIO - requires adTestParam[2]
				if (adTestParams[0] != 2)
					dRet = E__FAIL;
				else if ((adTestParams[2] == 1) || (adTestParams[2] == 2))
					dRet = xmc_gpio_loopback_VRE1X (adTestParams[2]);
				else
					dRet = E__FAIL;

				break;
		}
	}
	else
	{
		dRet = E__DEVICE_NOT_FOUND;
	}

	return (dRet);
}


/*****************************************************************************
 * Enxmc_Pex8619_Diff_IO_Test: Check the XMC Differential IO signals
 *
 * RETURNS: None
 */
TEST_INTERFACE (Enxmc_Pex8619_Diff_IO_Test, "ENXMC XMC Differential IO Test")
{
	UINT32	dRet = E__OK;
	PCI_PFA	upstreamPfa;
	PCI_PFA	downstreamPfa;
	UINT16	linkStatus;

#ifdef DEBUG
	vConsoleWrite("Enxmc_Pex8619_Diff_IO_Test\n");
#endif

	// find PEX8619 upstream port
	dRet = iPciFindDeviceById (1, 0x10B5, 0x8619, &upstreamPfa);
	if (dRet == E__OK)
	{
		// downstream ports 0 and 2 are the two x4 links that are looped back by the TF582 test fixture

		// get first downstream port
		downstreamPfa = PCI_MAKE_PFA(PCI_READ_BYTE(upstreamPfa, PCI_SEC_BUS), 0, 0);
		linkStatus = PCI_READ_WORD(downstreamPfa, PLX_DEV_LINK_STA);
		if ((linkStatus & 0x00F0) != 0x0040)
		{
			dRet = E__BAD_DIFF_IO_LOOPBACK_DEV0;
		}
		if (dRet == E__OK)
		{
			downstreamPfa = PCI_MAKE_PFA(PCI_READ_BYTE(upstreamPfa, PCI_SEC_BUS), 2, 0);
			linkStatus = PCI_READ_WORD(downstreamPfa, PLX_DEV_LINK_STA);
			if ((linkStatus & 0x00F0) != 0x0040)
			{
				dRet = E__BAD_DIFF_IO_LOOPBACK_DEV2;
			}
		}
	}
	else
	{
		dRet = E__DEVICE_NOT_FOUND;
	}

	return (dRet);
}
