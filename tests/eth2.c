
/************************************************************************
 *                                                                      *
 *      Copyright 2014 Concurrent Technologies, all rights reserved.    *
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

/* eth2.c - BIT test for Ethernet
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/eth2.c,v 1.6 2015-04-01 16:46:53 hchalla Exp $
 * $Log: eth2.c,v $
 * Revision 1.6  2015-04-01 16:46:53  hchalla
 * Added support for 82599 SFP 10G tests.
 *
 * Revision 1.5  2014-11-04 16:13:58  mgostling
 * Fixed x540 dev->dev loopback test.
 *
 * Revision 1.4  2014-10-03 14:57:13  chippisley
 * Fixed issues with Ethernet Loopback Tests and added further support for x540 devices.
 *
 * Revision 1.3  2014-09-22 10:14:00  mgostling
 * Work in progress
 *
 * Revision 1.1  2014-09-19 10:46:28  mgostling
 * Initial check-in to CVS. Work in progress.
 *
 *
 */


/* includes */

#include <stdtypes.h>
#include <errors.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <devices/eth2.h>

#include <bit/bit.h>
#include <bit/board_service.h>

#define INCLUDE_STATS

/* defines */
//#define DEBUG
//#define DEBUG_VERBOSE

#undef DEBUG
#undef DEBUG_VERBOSE	// define DEBUG_VERBOSE to display more information 

#define	E__TEST_WRONG_NUMBER_OF_PARAMS		E__BIT

#define E1E_FLUSH()							dReadReg825xx (STATUS)

/*
 * Register access macros
 */
 
#define vWriteReg825xx(reg, value)			(*(volatile UINT32*) (psDev->pbReg + reg) = value)
#define dReadReg825xx(reg)					(*(volatile UINT32*) (psDev->pbReg + reg))
		
#define vWriteFlashReg(reg, value)			(*(volatile UINT32*) (psDev->pbFlashBase + reg) = value)
		
#define dWriteFlashReg(reg, value)			(*(volatile UINT32*) (psDev->pbFlashBase + reg) = value)
#define wWriteFlashReg(reg, value)			(*(volatile UINT16*) (psDev->pbFlashBase + reg) = value)
#define bWriteFlashReg(reg, value)			(*(volatile UINT8*) (psDev->pbFlashBase + reg) = value)
		
#define dReadFlashReg(reg)					(*(volatile UINT32*) (psDev->pbFlashBase + reg))
		
#define wReadFlashReg(reg)					(*(volatile UINT16*) (psDev->pbFlashBase + reg))
#define bReadFlashReg(reg)					(*(volatile UINT8*) (psDev->pbFlashBase + reg))

#define PHY_PAGE_SHIFT						5
#define PHY_REG(page, reg)					(((page) << PHY_PAGE_SHIFT) | ((reg) & MAX_PHY_REG_ADDRESS))

#define vGetEthMacAddr vGetEthMacAddr2
#define dCheckTestData      dCheckTestData2
#define dFind825xx             dFind825xx2
#define dGetNumberOfEthInstances dGetNumberOfEthInstances2
#define test_data        test_data2

/* typedefs */

/* constants */

const UINT8 test_data[] = {0, 1, 2, 3, 4, 5};

#ifdef DEBUG
static const char *(aachMediaType []) =
{
	"Copper",
	"Fiber",
	"SerDes"
};
#endif

static const char *(aachSpeed []) =
{
	"Auto",
	"10MB BASE TX",
	"100MB BASE TX",
	"1GB BASE TX",
	"10GB BASE TX"
};

/* locals */

#ifdef DEBUG
static char buffer[128];
#endif

/* globals */

static PTR48	p1, p2, /*p3,*/ p4, p5, p6, p7;
static UINT32	dHandle1, dHandle2, dHandle3, dHandle4, dHandle5, dHandle6, dHandle7;
static UINT8	bByteBucket;	/* used for flushing results 		*/

/* externals */

extern void vDelay(UINT32 dMsDelay);
extern UINT32 dGetPhysPtr (UINT32 dBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);
extern void vConsoleWrite(char*	achMessage);
extern void vFreePtr(UINT32 dHandle);

/* forward declarations */

UINT32 dFind825xx (DEVICE* psDev, UINT8 bInstance,UINT8 bMode);
static UINT32 dInitPciDevice (DEVICE* psDev, UINT8 bIndex);
static UINT32 dEepromrwtest (DEVICE * psDev,UINT8 bType);
static UINT32 dGetLanAddr (DEVICE* psDev, UINT8* pbLanAddr);
//static void vSerDesLoopbackMode (DEVICE *psDev);
static void vSerDesChannelLoopbackMode (DEVICE *psDev);

static void vClear8257xStatistics (DEVICE* psDev);
static void vDisplay8257xStats (DEVICE* psDev);
static void vDisplayPhyRegs (DEVICE* psDev);

#if 0
	static void vDisplay8257xDiagRegs(DEVICE* psDev);
#endif

static void vSwitchI350ToSerdes (DEVICE *psDev);
static void vSwitchI350ToCopper (DEVICE *psDev);

//static UINT32 write_kmrn_reg (DEVICE *psDev, UINT32 offset, UINT16 data);
//static UINT32 read_kmrn_reg (DEVICE *psDev, UINT32 offset, UINT16 *data);
//static UINT32 phy_has_link_generic (DEVICE *psDev, UINT32 iterations, UINT32 usec_interval, UINT8 *success);
void config_collision_dist (DEVICE *psDev);
//static UINT32 set_mdio_slow_mode_hv (DEVICE *psDev);
void  phy_force_speed_duplex_setup (DEVICE *psDev, UINT16 *phy_ctrl,UINT32 dDuplex,UINT32 dSpeed);
void power_up_phy_copper (DEVICE *psDev);
static UINT32 dCtrlReset (DEVICE *psDev);

// X540 specific
static void initX540CommonRegisters(void);
static void x540ReleaseSwFwSync (DEVICE * psDev, UINT16 mask);
static UINT32 x540AcquireSwFwSync (DEVICE * psDev, UINT16 mask);
static void x540ReleaseSwFwSemaphore (DEVICE * psDev);
static UINT32 x540GetSwFwSemaphore (DEVICE * psDev);

#if 0
#define vWriteReg825xx(reg, value)			writeEthReg(psDev, reg, value)

void writeEthReg
(
	DEVICE*	psDev,
	UINT32 reg, 
	UINT32 value
)
{
	volatile UINT8 * addr;
	
	addr = psDev->pbReg;
	addr += reg;
	*(volatile UINT32 *) addr = value;
}

#define dReadReg825xx(reg)					readEthReg(psDev, reg)

UINT32 readEthReg
(
	DEVICE*	psDev,
	UINT32 reg
)
{
	volatile UINT8 * addr;
	UINT32 value;
	
	addr = psDev->pbReg;
	addr += reg;
	value = *(volatile UINT32 *) addr;
	
	return value;
}
#endif

/*****************************************************************************
 * vEepromDelay: a short delay
 *
 * The required delay is measured in uS (but the delay is actually ... ?)
 *
 * RETURNS: none
 */
static void vEepromDelay
(
	UINT16	wInterval
)
{
	UINT16	wIndex;
	UINT16	wCount;

	for (wIndex = 0; wIndex < wInterval; ++wIndex)
	{
		for (wCount = 0; wCount < 100; ++wCount)
		{
			++bByteBucket;
		}
	}
}

/*****************************************************************************
 * vRaiseClock: provide a 0->1 transition on the EEPROM clock
 *
 * RETURNS: none
 */
static void vRaiseClock
(
	DEVICE*	psDev,
	UINT32*	pdEecdRegValue
)
{
	*pdEecdRegValue = *pdEecdRegValue | EECD_CK;
	vWriteReg825xx (EECD, *pdEecdRegValue);
	vEepromDelay (50);
}


/*****************************************************************************
 * vLowerClock: provide a 1->0 transition on the EEPROM clock
 *
 * RETURNS: none
 */
static void vLowerClock
(
	DEVICE*	psDev,
	UINT32*	pdEecdRegValue
)
{
	*pdEecdRegValue = *pdEecdRegValue & ~EECD_CK;
	vWriteReg825xx (EECD, *pdEecdRegValue);
	vEepromDelay (50);
}

/*****************************************************************************
 * wEepromInit: check does EEPROM exist and enable access to EEPROM for 825xx
 *
 * RETURNS: E__OK or E__EEPROM_ACCESS
 */
static UINT32 wEepromInit
(
	DEVICE* psDev
)
{
	UINT16 wTimeout = 50;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	switch (psDev->bController)
	{
		case CTRLT_I350:

			vWriteReg825xx (EECD, dReadReg825xx (EECD) | EECD_REQ);

			while (wTimeout > 0)
			{
				if ((dReadReg825xx (EECD) & EECD_GNT) == EECD_GNT)
				{
					return E__OK;
				}
				--wTimeout;
				vDelay (1);
			}

			if (wTimeout == 0)
			{
#ifdef DEBUG
				vConsoleWrite ("SROM grant timeout");
#endif

				return E__EEPROM_ACCESS;
			}
			break;

		case CTRLT_I210:
			break;
	}

	return E__OK;
}

/*****************************************************************************
 * vEepromCleanup: place EEPROM signals in an inactive state after a command
 *
 * RETURNS: none
 */
static void vEepromCleanup
(
	DEVICE*	psDev
)
{
	UINT32 dEecdRegValue;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	dEecdRegValue = dReadReg825xx (EECD);
	dEecdRegValue &= ~(EECD_CS + EECD_DI);

	vWriteReg825xx (EECD, dEecdRegValue);

	vRaiseClock (psDev, &dEecdRegValue);
	vLowerClock (psDev, &dEecdRegValue);

	/* release EEPROM for hardware */

	if (psDev->bController == CTRLT_I350)
	{
		dEecdRegValue = dReadReg825xx (EECD);
		dEecdRegValue &= ~EECD_REQ;
		vWriteReg825xx (EECD, dEecdRegValue);
	}
}

/*****************************************************************************
 * wShiftInBits: clock in 16-bits of data from the EEPROM
 *
 * RETURNS: 16-bit data
 */

static UINT16 wShiftInBits
(
	DEVICE*	psDev,
	UINT16	wCount
)
{
	UINT32	dEecdRegValue;
	UINT16	wData;
	UINT16	wIndex;


	dEecdRegValue = dReadReg825xx (EECD);

	dEecdRegValue &= ~(EECD_DO | EECD_DI);
	wData = 0;

	for (wIndex = 0; wIndex < wCount; ++wIndex)
	{
		wData = wData << 1;
		vRaiseClock (psDev, &dEecdRegValue);

		dEecdRegValue = dReadReg825xx (EECD);
		dEecdRegValue &= ~(EECD_DI);

		if (dEecdRegValue & EECD_DO)
		{
			wData |= 1;
		}

		vLowerClock (psDev, &dEecdRegValue);
	}

	return (wData);
}

/*****************************************************************************
 * vShiftOutBits: clock out a variable length bit-stream to the EEPROM
 *
 * RETURNS: none
 */
static void vShiftOutBits
(
	DEVICE*	psDev,
	UINT16	wData,
	UINT16	wCount
)
{
	UINT32	dEecdRegValue;
	UINT32	dMask;

	dMask = 0x01L << (wCount - 1);

	dEecdRegValue = dReadReg825xx (EECD);
	dEecdRegValue &= ~(EECD_DO | EECD_DI);

	if 		(psDev->bController == CTRLT_I350)
	{
		dEecdRegValue |= EECD_DO;
	}

	do {
		dEecdRegValue &= ~EECD_DI;

		if (wData & dMask)
		{
			dEecdRegValue |= EECD_DI;
		}

		vWriteReg825xx (EECD, dEecdRegValue);

		vEepromDelay (50);

		vRaiseClock (psDev, &dEecdRegValue);
		vLowerClock (psDev, &dEecdRegValue);

		dMask = dMask >> 1;

	} while (dMask != 0);

	dEecdRegValue &= ~EECD_DI;
	vWriteReg825xx (EECD, dEecdRegValue);

}

#if 0
/*****************************************************************************
 * vStandBy: place EEPROM signals into a standby state
 *
 * RETURNS: none
 */
static void vStandBy
(
	DEVICE*	psDev
)
{
	UINT32	dEecdRegValue;

	dEecdRegValue = dReadReg825xx (EECD);

	dEecdRegValue &= ~(EECD_CS | EECD_CK);
	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

	dEecdRegValue |= EECD_CK;
	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

	dEecdRegValue |= EECD_CS;
	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

	dEecdRegValue &= ~EECD_CK;
	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

}
#endif

/*****************************************************************************
 * v825xxStandBy: place EEPROM signals into a standby state
 *
 * RETURNS: none
 */
static void v825xxStandBy
(
	DEVICE*	psDev
)
{
	UINT32	dEecdRegValue;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	dEecdRegValue = dReadReg825xx (EECD);

	dEecdRegValue &= ~EECD_CK;
	vWriteReg825xx (EECD, dEecdRegValue);

	dEecdRegValue |= EECD_CS;

	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

	dEecdRegValue &= ~EECD_CS;

	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

	dEecdRegValue &= ~EECD_CK;
	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

}

/*****************************************************************************
 * dWaitSpiEepromCommandDone: wait for EEPROM to complete an erase or write
 *
 * RETURNS: E__OK or E__TIMEOUT
 */
static UINT32 dWaitSpiEepromCommandDone
(
	DEVICE*	psDev
)
{
	UINT32	wIndex;
	UINT8	bStatus;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	for (wIndex = 0; wIndex < EEPROM_TIMEOUT; ++wIndex)
	{
		vShiftOutBits (psDev, EEPROM_RDSR_OPCODE_SPI, 8);
		bStatus = wShiftInBits (psDev, 8);
		v825xxStandBy (psDev);

		if (!(bStatus & EEPROM_STATUS_RDY_SPI))
		{
			break;
		}

		vEepromDelay (5);

	}

	/* ATMEL SPI write time could vary from 0-20mSec on 3.3V devices (and
	 * only 0-5mSec on 5V devices)
	 */
	if (wIndex == EEPROM_TIMEOUT)
	{
#ifdef DEBUG
		sprintf (buffer, "wWaitSpiEepromCmdDone %02x\n", bStatus);
		vConsoleWrite (buffer);
#endif

		return (E__SPI_EEPROM_TIMEOUT);
	}

	return E__OK;
} /* dWaitSpiEepromCommandDone () */

/*****************************************************************************
 * dWriteSpiEepromByte: write a byte to an SPI EEPROM address
 *
 * RETURNS: E__OK, E__EEPROM_ERASE or E__EPROM_WRITE
 */
static UINT32 dWriteSpiEepromByte
(
	DEVICE*	psDev,
	UINT16	wReg,
	UINT8	bData
)
{
#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* spi interface */
#ifdef DEBUG
	sprintf(buffer, "dWriteSpiEepromByte %04x=%02x\n", wReg, bData);
	vConsoleWrite(buffer);
#endif

	vWriteReg825xx (EECD, dReadReg825xx (EECD) & ~EECD_CS);
	vShiftOutBits (psDev, EEPROM_WREN_OPCODE_SPI, 8);

	v825xxStandBy (psDev);

	vShiftOutBits (psDev, EEPROM_WRITE_OPCODE_SPI, 8);
	vShiftOutBits (psDev, wReg, 16);
	vShiftOutBits (psDev, bData, 8);

	v825xxStandBy (psDev);

	if (dWaitSpiEepromCommandDone (psDev) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("dWriteSpiEepromByte SPI cmd done error\n");
#endif

		return (E__SPI_EEPROM_WRITE);
	}
	return (E__OK);
}

#if 0
/*****************************************************************************
 * dWaitEepromCommandDone: wait for EEPROM to complete an erase or write
 *
 * RETURNS: E__OK or E__TIMEOUT
 */

static UINT32 dWaitEepromCommandDone
(
	DEVICE*	psDev
)
{
	UINT32	dEecdRegValue;
	UINT32	wIndex;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	vStandBy (psDev);

	for (wIndex = 0; wIndex < EEPROM_TIMEOUT; ++wIndex)
	{
		dEecdRegValue = dReadReg825xx (EECD);

		if (dEecdRegValue & EECD_DO)
		{
			return (E__OK);
		}

		vEepromDelay (5);
	}

	return (E__EEPROM_TIMEOUT);
}
#endif

/*****************************************************************************
 * dWriteEepromWord: write a word to an EEPROM address
 *
 * RETURNS: E__OK, E__EEPROM_ERASE or E__EPROM_WRITE
 */

static UINT32 dWriteEepromWord
(
	DEVICE*	psDev,
	UINT16	wReg,
	UINT16	wData
)
{
	UINT32	dTestStatus = E__OK;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if ((dTestStatus = wEepromInit (psDev)) != E__OK)
		return dTestStatus;

	if (psDev->bController == CTRLT_I210 || psDev->bController == CTRLT_82599_SFP)
	{
#ifdef DEBUG
		vConsoleWrite ("Write ignored - not allowed on i210\n");
#endif

		return dTestStatus;
	}
	else if (psDev->bController == CTRLT_I350)
	{
		/* spi interface */
		dTestStatus = dWriteSpiEepromByte (psDev, wReg * 2, ((UINT8)wData & 0xff));
		if (dTestStatus == E__OK)
		{
			dTestStatus = dWriteSpiEepromByte (psDev, (wReg * 2) + 1, (UINT8)(wData >> 8));
		}
	}

	vEepromCleanup (psDev);
	vDelay (1);

	return (dTestStatus);

}

#if 0
/*****************************************************************************
 * wReadInvmCSR: read the requested CSR structure from iNVM
 *
 * RETURNS: 16-bit data
 */

static UINT32 dReadInvmCSR
(
	DEVICE*	psDev,
	UINT16	wReg
)
{
	UINT32	dCSR = 0xFFFFFFFFL;
	UINT32	dData;
	UINT8	bIndex;
	UINT8	bReset;
	UINT16	wAddress;
	UINT8	found;

//	if (psDev->bController == CTRLT_I210)
//	{
		// Read iNVM until required data structure found

		found = FALSE;
		for (bIndex = 0; (!found) && (bIndex < INVM_SIZE); ++bIndex)
		{
			dData = dReadReg825xx (INVM_DATA_REG(bIndex));
			bReset = (UINT8)((dData & INVM_RESET_TYPE_MASK) >> 3);
			switch (dData & INVM_STRUCT_TYPE_MASK)
			{
				case INVM_NVM_END:
					bIndex = INVM_SIZE;				// end of iNVM data
					break;

				case INVM_CSR_AUTOLOAD:
					wAddress = (UINT16) ((dData >> 16) & INVM_CSR_ADDR_MASK);
					dCSR = dReadReg825xx (INVM_DATA_REG(bIndex + 1));
					if (wAddress == wReg)
					{
						found = TRUE;
					}
					++bIndex;
					break;

				case INVM_WORD_AUTOLOAD:
				case INVM_PHY_AUTOLOAD:
				case INVM_INVALIDATED:
				default:
					break;
			}
		}
	}

#ifdef DEBUG
	sprintf (buffer, "iNVM CSR: %04X=%08X Reset=%d\n", wAddress, dCSR, bReset);
	vConsoleWrite (buffer);
#endif

	return (dCSR);

}

/*****************************************************************************
 * wReadInvmPhy: read the requested PHY structure from iNVM
 *
 * RETURNS: 32-bit data
 */
static UINT16 wReadInvmPhy
(
	DEVICE*	psDev,
	UINT16	wReg
)
{
	UINT32	dData;
	UINT16	wData;
	UINT16	wAddress;
	UINT8	bIndex;
	UINT8	bReset;
	UINT8	found;

//	if (psDev->bController == CTRLT_I210)
//	{
		// Read iNVM until required data structure found

		found = FALSE;
		for (bIndex = 0; (!found) && (bIndex < INVM_SIZE); ++bIndex)
		{
			dData = dReadReg825xx (INVM_DATA_REG(bIndex));
			bReset = (UINT8)((dData & INVM_RESET_TYPE_MASK) >> 3);
			switch (dData & INVM_STRUCT_TYPE_MASK)
			{
				case INVM_NVM_END:
					bIndex = INVM_SIZE;				// end of iNVM data
					break;

				case INVM_CSR_AUTOLOAD:
					++bIndex;
					break;

				case INVM_PHY_AUTOLOAD:
					wAddress = (UINT16) ((dData & INVM_PHY_ADDR_MASK) >> 11);
					wData = (UINT16) (dData >> 16);
					if (wAddress == wReg)
					{
						found = TRUE;
					}
					break;

				case INVM_WORD_AUTOLOAD:
				case INVM_INVALIDATED:
				default:
					break;
			}
		}
	}

#ifdef DEBUG
	sprintf (buffer, "iNVM PHY: %02X=%04X Reset=%d\n", wAddress, wData, bReset);
	vConsoleWrite (buffer);
#endif

	return (wData);

}
#endif

/*****************************************************************************
 * wReadInvmWord: read the requested word structure from iNVM
 *
 * RETURNS: 16-bit data
 */

static UINT16 wReadInvmWord
(
	DEVICE*	psDev,
	UINT16	wReg
)
{
	UINT32	dData;
	UINT16	wData;
	UINT16	wAddress;
	UINT8	bIndex;
	UINT8	found;

#ifdef DEBUG
	UINT8	bReset;
#endif

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

//	if (psDev->bController == CTRLT_I210)
//	{
		// Read iNVM until required data structure found

		found = FALSE;
		for (bIndex = 0; (!found) && (bIndex < INVM_SIZE); ++bIndex)
		{
			dData = dReadReg825xx (INVM_DATA_REG(bIndex));

#ifdef DEBUG
			bReset = (UINT8)((dData & INVM_RESET_TYPE_MASK) >> 3);
#endif

			switch (dData & INVM_STRUCT_TYPE_MASK)
			{
				case INVM_NVM_END:
					bIndex = INVM_SIZE;				// end of iNVM data
					break;

				case INVM_WORD_AUTOLOAD:
					wAddress = (UINT16) ((dData & INVM_WORD_ADDR_MASK) >> 9);
					wData = (UINT16) (dData >> 16);
					if (wAddress == wReg)
					{
						found = TRUE;
					}
					break;

				case INVM_CSR_AUTOLOAD:
					++bIndex;
					break;

				case INVM_PHY_AUTOLOAD:
				case INVM_INVALIDATED:
				default:
					break;
			}
		}
//	}

#ifdef DEBUG
	sprintf (buffer, "iNVM Word: %02X=%04X Reset=%d\n", wAddress, wData, bReset);
	vConsoleWrite (buffer);
#endif

	return (wData);
}

/*****************************************************************************
 * wReadEepromWord: read a word from the specified EEPROM address
 *
 * RETURNS: 16-bit data
 */
static UINT16 wReadEepromWord
(
	DEVICE*	psDev,
	UINT16	wReg
)
{
	UINT16	wData = 0xffff;
	UINT32	dRegData;
	UINT32	dTmp;
	UINT32	dTimeout;
	UINT32	dReg;

	if (psDev->bController == CTRLT_I210) 
	{
		dRegData = dReadReg825xx (EECD);
	}
		if ((psDev->bController == CTRLT_I210)&& ((dRegData & EECD_FLASH_IN_USE) == 0))
	{
		wData = wReadInvmWord (psDev, wReg);
	}
	else if ((psDev->bController == CTRLT_I350) ||
			 (psDev->bController == CTRLT_I210) ||
			 (psDev->bController == CTRLT_X540) ||
			 (psDev->bController == CTRLT_82599_SFP) )
	{
		dTmp = (UINT32) wReg;
		dTmp = (dTmp << 2) | 1;

		if (psDev->bController == CTRLT_X540)
		{
			dReg = EERD_X540;
		}
		else if (psDev->bController == CTRLT_82599_SFP)
		{
			dReg = EERD_X540;
		}
		else
		{
			dReg = EERD;
		}
		
		vWriteReg825xx (dReg, dTmp);

		dTimeout = 5;
		do
		{
			dTmp = dReadReg825xx (dReg);
			vDelay(1);
		} while (((--dTimeout) > 0) && ((dTmp & 2) == 0));

		if (dTimeout != 0)
		{
			wData = dTmp >> 16;
		}
#ifdef DEBUG
		else
			vConsoleWrite ("Read Failed\n");
#endif
	}

	return (wData);
}

/*****************************************************************************
 * ReadPhyRegister: read a PHY register
 *
 * RETURNS: 16-bit data
 */
static UINT16 ReadPhyRegister
(
	DEVICE *psDev,
	UINT32	RegAddress
)
{
	UINT32	Data;
	UINT32	Command;
	UINT16	wTimeout;

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		// setup and write the address cycle command
		Command = RegAddress
					| ((psDev->bPhyAddress << MSCA_DEVADD_SHIFT) & M__MSCA_DEVADD)
					| MSCA_OP_ADDR_CYCLE
					| MSCA_MDICMD;

		vWriteReg825xx (MSCA, Command);
	
		for (wTimeout = 0; wTimeout < MSCA_CMD_TIMEOUT; ++wTimeout)
		{
			vDelay (1);
			Data = dReadReg825xx (MSCA);
			if ((Data & MSCA_MDICMD) == 0)
			{
				break;
			}
		}
		
		if ((Data & MSCA_MDICMD) != 0)
		{
			return 0xFFFF;
		}

		// address cycle complete, so setup and write the read command
		Command = RegAddress
					| ((psDev->bPhyAddress << MSCA_DEVADD_SHIFT) & M__MSCA_DEVADD)
					| MSCA_OP_READ
					| MSCA_MDICMD;

		vWriteReg825xx (MSCA, Command);
		
		for (wTimeout = 0; wTimeout < MSCA_CMD_TIMEOUT; ++wTimeout)
		{
			vDelay (1);
			Data = dReadReg825xx (MSCA);
			if ((Data & MSCA_MDICMD) == 0)
			{
				break;
			}
		}
	
		if ((Data & MSCA_MDICMD) != 0)
		{
			return 0xFFFF;
		}
		
		// read the data from the MDI read/write register
		Data = dReadReg825xx (MSRWD) >> 16;
		return Data;
	}
	else
	{
		/* read internal PHY */
		Command = ((RegAddress << 16) & MDIC_REG_ADD)
					| ((psDev->bPhyAddress << 21) & MDIC_PHY_ADD)
					| MDIC_OP_READ;
	
		vWriteReg825xx (MDIC, Command);
	
		vDelay  (1);
		wTimeout = 50;
	
		Data = dReadReg825xx (MDIC);
		while (((Data & MDIC_READY) == 0) && (wTimeout-- > 0))
		{
			vDelay (1);
		}
	
		if (wTimeout != 0 && (Data & MDIC_ERROR) == 0)
		{
			return (UINT16) (Data & MDIC_DATA);
		}
		else
		{
			return 0xFFFF;
		}
	}
}

/*****************************************************************************
 * WritePhyRegister: write into a PHY register
 *
 * RETURNS: none
 */
static void WritePhyRegister
(
	DEVICE *psDev,
	UINT32	RegAddress,
	UINT16	Data
)
{
	UINT16	wTimeout;
	UINT32	status;
	UINT32	Command;
	UINT32	regVal;

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		// put the data in the MDI read/write data register
		vWriteReg825xx (MSRWD, Data);

		// setup and write the address cycle command
		Command = RegAddress
					| ((psDev->bPhyAddress << MSCA_DEVADD_SHIFT) & M__MSCA_DEVADD)
					| MSCA_OP_ADDR_CYCLE
					| MSCA_MDICMD;

		vWriteReg825xx (MSCA, Command);
	
		// wait for command to complete
		for (wTimeout = 0; wTimeout < MSCA_CMD_TIMEOUT; ++wTimeout)
		{
			vDelay (1);
			Data = dReadReg825xx (MSCA);
			if ((Data & MSCA_MDICMD) == 0)
			{
				break;
			}
		}
	
		if ((Data & MSCA_MDICMD) == 0)
		{
			// address cycle complete, so setup and write the write command
			Command = RegAddress
						| ((psDev->bPhyAddress << MSCA_DEVADD_SHIFT) & M__MSCA_DEVADD)
						| MSCA_OP_WRITE
						| MSCA_MDICMD;
	
			vWriteReg825xx (MSCA, Command);
			
			// wait for command to complete
			for (wTimeout = 0; wTimeout < MSCA_CMD_TIMEOUT; ++wTimeout)
			{
				vDelay (1);
				Data = dReadReg825xx (MSCA);
				if ((Data & MSCA_MDICMD) == 0)
				{
					break;
				}
			}
		}
	}
	else
	{
		/* write internal PHY */
	
		regVal = (Data & MDIC_DATA)
					| ((RegAddress << 16) & MDIC_REG_ADD)
					| ((psDev->bPhyAddress << 21) & MDIC_PHY_ADD)
					| MDIC_OP_WRITE;
	
		vWriteReg825xx (MDIC, regVal);
	
		wTimeout = 50;
		status = dReadReg825xx (MDIC);
		while (((status & MDIC_READY) == 0) && (wTimeout-- > 0))
		{
			status = dReadReg825xx (MDIC);
			vDelay (1);
		}
	}
}

/*******************************************************************
 * dReadPhyRegs: Accesses the contents of all PHY registers
 *
 * RETURNS: UINT32
 */
static UINT32 dReadPhyRegs
(
	DEVICE* psDev
)
{
	int i;
	int mmd;
	UINT16 wRegVal1, wRegVal2, wRegVal3, wRegVal4;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

#ifdef DEBUG
	sprintf (buffer, "PHY registers (address = %02Xh)\n", psDev->bPhyAddress);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		mmd = MDIO_MMD_PMAPMD;		/* 1: Physical Medium Attachment/Physical Medium Dependent */
		for (i = 0; i < 12; i += 4)
		{
			wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i));
			wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 1));
			wRegVal3 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 2));
			wRegVal4 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 3));

#ifdef	DEBUG
			sprintf (buffer, "%02X:%04X  0x%04X 0x%04X 0x%04X 0x%04X\n", 
					mmd, i, wRegVal1, wRegVal2, wRegVal3, wRegVal4);
			vConsoleWrite (buffer);
#endif
		}
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0E));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0F));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x0E, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif
		
		wRegVal3 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x81));
		wRegVal4 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x82));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x81, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xE400));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xE400, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xE800));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xE800, wRegVal1);
		vConsoleWrite (buffer);
#endif

		
//		mmd = MDIO_MMD_WIS;			/* 2: WAN Interface Sublayer */

		mmd = MDIO_MMD_PCS;			/* 3: Physical Coding Sublayer */
		for (i = 0; i < 8; i += 4)
		{
			wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i));
			wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 1));
			wRegVal3 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 2));
			wRegVal4 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 3));

#ifdef	DEBUG
			sprintf (buffer, "%02X:%04X  0x%04X 0x%04X 0x%04X 0x%04X\n", 
					mmd, i, wRegVal1, wRegVal2, wRegVal3, wRegVal4);
			vConsoleWrite (buffer);
#endif
		}

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x08));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0x08, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0E));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0F));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x0E, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x20));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x21));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x20, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif
		


//		mmd = MDIO_MMD_PHYXS;		/* 4: PHY Extender Sublayer */
//		mmd = MDIO_MMD_DTEXS;		/* 5: DTE Extender Sublayer */
//		mmd = MDIO_MMD_TC;			/* 6: Transmission Convergence */

		mmd = MDIO_MMD_AN;			/* 7: Auto-Negotiation */
		for (i = 0; i < 8; i += 4)
		{
			wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i));
			wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 1));
			wRegVal3 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 2));
			wRegVal4 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 3));

#ifdef	DEBUG
			sprintf (buffer, "%02X:%04X  0x%04X 0x%04X 0x%04X 0x%04X\n", 
					mmd, i, wRegVal1, wRegVal2, wRegVal3, wRegVal4);
			vConsoleWrite (buffer);
#endif
		}

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x08));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0x08, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0E));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0F));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x0E, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x10));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0x10, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x13));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0x13, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x16));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0x16, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x19));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0x19, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x20));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x21));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x20, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xC800));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xC800, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xC810));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xC810, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xCC00));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xCC00, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		mmd = MDIO_MMD_C22EXT;		/* 29: Clause 22 extension */
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x02));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x03));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x02, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x05));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x06));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x05, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x08));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0x08, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0E));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0F));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x0E, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif

		mmd = MDIO_MMD_VEND1;		/* 30: Vendor specific 1 */
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x02));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x03));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x02, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x05));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x06));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x05, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x08));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0x08, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0E));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0F));

#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x0E, wRegVal1, wRegVal2);
		vConsoleWrite (buffer);
#endif

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xC800));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xC800, wRegVal1);
		vConsoleWrite (buffer);
#endif
		
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xCC00));
		
#ifdef	DEBUG
		sprintf (buffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xC800, wRegVal1);
		vConsoleWrite (buffer);
#endif

//		mmd = MDIO_MMD_VEND2;		/* 31: Vendor specific 2 */
	}
	else
	{
		for (i = 0; i < 25; i += 4)
		{
			wRegVal1 = ReadPhyRegister (psDev, i);
			wRegVal2 = ReadPhyRegister (psDev, i + 1);
			wRegVal3 = ReadPhyRegister (psDev, i + 2);
			wRegVal4 = ReadPhyRegister (psDev, i + 3);

#ifdef	DEBUG
			sprintf (buffer, "%3d  0x%04x 0x%04x 0x%04x 0x%04x\n", i,
					wRegVal1, wRegVal2, wRegVal3, wRegVal4);
			vConsoleWrite (buffer);
#endif
		}
	}
	
	if ((wRegVal1 == 0xFFFF) ||
		(wRegVal2 == 0xFFFF) ||
		(wRegVal3 == 0xFFFF) ||
		(wRegVal4 == 0xFFFF))
	{
		return (E__PHYREADERR);
	}
	else
	{
		return (E__OK);
	}
}


/*******************************************************************
 * vEnableRxTx: enable receiver then transmiter
 *
 * RETURNS: none
 */

static void vEnableRxTx
(
	DEVICE*	psDev,		/* pointer to device data structure */
	int		dFunc		/* what to enable */
)
{
	UINT32	dRegVal;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* Enable receiver then transmitter to avoid loss of frames */

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		if (dFunc & EN_RX)
		{
#ifdef DEBUG
			vWriteReg825xx (MAXFRS, 0x20000000L);
			dRegVal = dReadReg825xx (HLREG0);				/* enable jumbo frames */
			vWriteReg825xx (HLREG0, dRegVal | 0x04L);
#endif
			dRegVal = dReadReg825xx (RXCTRL);				/* enable RX */
			vWriteReg825xx (RXCTRL, dRegVal | RXCTRL_EN);
	
			vDelay (10);									/* give RX time to prefetch buffers */
		}
		
		if (dFunc & EN_TX)
		{
			dRegVal = dReadReg825xx (DMATXCTL);				/* enable TX */
			vWriteReg825xx (DMATXCTL, dRegVal | DMATXCTL_EN);
		}
	}
	else
	{
		if (dFunc & EN_RX)
		{
			dRegVal = dReadReg825xx (RCTL);					/* enable RX */
			vWriteReg825xx (RCTL, dRegVal | RCTL_EN);
	
			vDelay (10);									/* give RX time to prefetch buffers */
		}
	
		if (dFunc & EN_TX)
		{
			dRegVal = dReadReg825xx (TCTL);					/* enable TX */
			vWriteReg825xx (TCTL, dRegVal | TCTL_EN);
		}
	}

}

/*******************************************************************
 * vDisableTxRx: disable transmitter then receiver
 *
 * RETURNS: none
 */
static void vDisableTxRx
(
	DEVICE*	psDev		/* pointer to device data structure */
)
{
	UINT32	dRegVal;


#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* Disable transmitter then receiver to avoid loss of frames */

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		dRegVal = dReadReg825xx (RXCTRL);				/* disable RX */
		vWriteReg825xx (RXCTRL, dRegVal & ~RXCTRL_EN);

		vDelay (10);									/* give TX time to complete */

		dRegVal = dReadReg825xx (DMATXCTL);				/* disable TX */
		vWriteReg825xx (DMATXCTL, dRegVal & ~DMATXCTL_EN);
	}
	else
	{
		dRegVal = dReadReg825xx (TCTL);					/* disable TX */
		vWriteReg825xx (TCTL, dRegVal & ~TCTL_EN);
	
		vDelay (10);									/* give TX time to complete */
	
		dRegVal = dReadReg825xx (RCTL);					/* disable RX */
		vWriteReg825xx (RCTL, dRegVal & ~RCTL_EN);
	}

}

/*******************************************************************
 * PhyReset: Reset PHY through MAC register
 *
 * RETURNS: E__OK or E__... error code
 */
static int PhyReset
(
	DEVICE*	psDev
)
{
	UINT16	RegData;
	UINT16	i;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		return E__OK;
	}
	
	RegData  = ReadPhyRegister (psDev, PHY_CTRL);
	RegData |= PHY_CTRL_RESET;
	WritePhyRegister (psDev, PHY_CTRL, RegData);

	for (i = 50; ((RegData & PHY_CTRL_RESET) != 0) && (i > 0); --i)
	{
		RegData = ReadPhyRegister (psDev, PHY_CTRL);
		vDelay (1);
	}

	if (i == 0)
	{
#ifdef DEBUG
		vConsoleWrite ("Timeout waiting for PHY to reset.");
#endif

		return E__PHY_RESET_TIMEOUT;;
	}
	return E__OK;
}

/*******************************************************************
 * PhySpeed: Set PHY speed
 *
 * RETURNS: none
 */
static void PhySpeed
(
	DEVICE*	psDev,
	UINT8	bSpeed
)
{
	UINT16	RegData = 0;
	UINT16	CtrlReg = 0;
	UINT32	dReg;

#ifdef DEBUG
	sprintf (buffer, "%s speed: %u\n", __FUNCTION__, bSpeed);
	vConsoleWrite (buffer);
#endif

	switch (bSpeed)
	{
		case SPEED_AUTO:
#ifdef DEBUG
			vConsoleWrite ("Setting: AUTO\n");
#endif
			/* advertise all speeds */
			if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
			{
				// 10GB
				dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_10GB_CTRL);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData |= PHY_AN_10GB_CTRL_ADV10G;
				WritePhyRegister(psDev, dReg, RegData);
				// 1GB
				dReg = X540_PHY_REG(MDIO_MMD_VEND1, PHY_MII_AUTONEG_VENDOR_PROV1);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData |= PHY_MII_1GBASE_T_ADVERTISE;
				WritePhyRegister(psDev, dReg, RegData);
				// 100MB
				dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_ADVERTISE);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData &= ~PHY_AN_ABILITY_100HALF;
				RegData |= PHY_AN_ABILITY_100FULL;
				WritePhyRegister(psDev, dReg, RegData);
			}
			else
			{
				CtrlReg = ReadPhyRegister (psDev, PHY_CTRL);
				CtrlReg &= ~M__PHY_CTRL_SPEED;
				CtrlReg |= PHY_CTRL_AUTO_NEG;
				RegData = ReadPhyRegister (psDev, PHY_1000);
				RegData |= PHY_1000_1000TXFD;
				WritePhyRegister (psDev, PHY_1000, RegData);
		
				RegData = ReadPhyRegister (psDev, PHY_ADVERT);
				RegData |= (PHY_ADVERT_10TXFD | PHY_ADVERT_100TXFD);
				WritePhyRegister (psDev, PHY_ADVERT, RegData);
			}
			break;
	
		case SPEED_10G:
#ifdef DEBUG
			vConsoleWrite ("Setting: 10Gb\n");
#endif
			/* advertise 10GB only */
//			if (psDev->bController == CTRLT_X540)
//			{
				// 10GB
				dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_10GB_CTRL);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData |= PHY_AN_10GB_CTRL_ADV10G;
				WritePhyRegister(psDev, dReg, RegData);
				// 1GB
				dReg = X540_PHY_REG(MDIO_MMD_VEND1, PHY_MII_AUTONEG_VENDOR_PROV1);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData &= ~PHY_MII_1GBASE_T_ADVERTISE;
				WritePhyRegister(psDev, dReg, RegData);
				// 100MB
				dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_ADVERTISE);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData &= ~(PHY_AN_ABILITY_100HALF | PHY_AN_ABILITY_100FULL);
				WritePhyRegister(psDev, dReg, RegData);
//			}
			break;
	
		case SPEED_1G:
#ifdef DEBUG
			vConsoleWrite ("Setting: 1Gb\n");
#endif
			/* advertise 1GB only */
			if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
			{
				// 10GB
				dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_10GB_CTRL);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData &= ~PHY_AN_10GB_CTRL_ADV10G;
				WritePhyRegister(psDev, dReg, RegData);
				// 1GB
				dReg = X540_PHY_REG(MDIO_MMD_VEND1, PHY_MII_AUTONEG_VENDOR_PROV1);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData |= PHY_MII_1GBASE_T_ADVERTISE;
				WritePhyRegister(psDev, dReg, RegData);
				// 100MB
				dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_ADVERTISE);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData &= ~(PHY_AN_ABILITY_100HALF | PHY_AN_ABILITY_100FULL);
				WritePhyRegister(psDev, dReg, RegData);
			}
			else
			{
				CtrlReg = ReadPhyRegister (psDev, PHY_CTRL);
				CtrlReg &= ~M__PHY_CTRL_SPEED;
				CtrlReg |= PHY_CTRL_AUTO_NEG;
				RegData = ReadPhyRegister (psDev, PHY_ADVERT);
				RegData &= ~(PHY_ADVERT_10TXFD
							| PHY_ADVERT_10TXHD
							|PHY_ADVERT_100TXFD
							| PHY_ADVERT_100TXHD);
		
				WritePhyRegister (psDev, PHY_ADVERT, RegData);
		
				CtrlReg |= PHY_CTRL_SPEED_1000;
				CtrlReg &= ~0x1000;
				WritePhyRegister (psDev, PHY_CTRL, CtrlReg);
		
				WritePhyRegister (psDev,0x00,0x1340);
				WritePhyRegister (psDev,0x04,0x01E1);
				WritePhyRegister (psDev,0x09,0x0200);
			}
			break;
	
		case SPEED_100M:
#ifdef DEBUG
			vConsoleWrite ("Setting: 100Mb\n");
#endif
			/* advertise 100 TX only */
			if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
			{
				// 10GB
				dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_10GB_CTRL);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData &= ~PHY_AN_10GB_CTRL_ADV10G;
				WritePhyRegister(psDev, dReg, RegData);
				// 1GB
				dReg = X540_PHY_REG(MDIO_MMD_VEND1, PHY_MII_AUTONEG_VENDOR_PROV1);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData &= ~PHY_MII_1GBASE_T_ADVERTISE;
				WritePhyRegister(psDev, dReg, RegData);
				// 100MB
				dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_ADVERTISE);
				RegData = ReadPhyRegister(psDev, dReg);
				RegData &= ~PHY_AN_ABILITY_100HALF;
				RegData |= PHY_AN_ABILITY_100FULL;
				WritePhyRegister(psDev, dReg, RegData);
			}
			else
			{
				CtrlReg = ReadPhyRegister (psDev, PHY_CTRL);
				CtrlReg &= ~M__PHY_CTRL_SPEED;
				CtrlReg |= PHY_CTRL_AUTO_NEG;
				RegData = ReadPhyRegister (psDev, PHY_1000);
				RegData &= ~(PHY_1000_1000TXFD | PHY_1000_1000TXHD);
				WritePhyRegister (psDev, PHY_1000, RegData);
		
				RegData = ReadPhyRegister (psDev, PHY_ADVERT);
				RegData &= ~(PHY_ADVERT_10TXFD
							| PHY_ADVERT_10TXHD
							|PHY_ADVERT_100TXHD);
		
				RegData |= (PHY_ADVERT_100TXFD | PHY_ADVERT_100TXHD);
				WritePhyRegister (psDev, PHY_ADVERT, RegData);
		
				CtrlReg |= PHY_CTRL_SPEED_100;
				WritePhyRegister (psDev, PHY_CTRL, CtrlReg);
			}
			break;
	
		default:
			if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
			{
				break;
			}
			/* i350 & i210 identical actions as if set to 10MB ... so drop thru' */
			
		case SPEED_10M:
#ifdef DEBUG
			vConsoleWrite ("Setting: 10Mb\n");
#endif
			CtrlReg = ReadPhyRegister (psDev, PHY_CTRL);
			CtrlReg &= ~M__PHY_CTRL_SPEED;
			CtrlReg |= PHY_CTRL_AUTO_NEG;
			/* advertise 10 only */
			RegData = ReadPhyRegister (psDev, PHY_1000);
			RegData &= ~(PHY_1000_1000TXFD | PHY_1000_1000TXHD);
			WritePhyRegister (psDev, PHY_1000, RegData);
	
			RegData = ReadPhyRegister (psDev, PHY_ADVERT);
			RegData &= ~(PHY_ADVERT_100TXFD
						| PHY_ADVERT_100TXHD
						| PHY_ADVERT_10TXHD);
	
			RegData |= (PHY_ADVERT_10TXFD | PHY_ADVERT_10TXHD);
			WritePhyRegister (psDev, PHY_ADVERT, RegData);
			CtrlReg |= PHY_CTRL_SPEED_10;
			WritePhyRegister (psDev, PHY_CTRL, CtrlReg); //
			break;
	}

	if ((bSpeed != SPEED_1G) && (psDev->bController != CTRLT_X540) &&(psDev->bController != CTRLT_82599_SFP))
	{
		/* restart auto neg */
		/* force full duplex */
		CtrlReg = ReadPhyRegister (psDev, PHY_CTRL);
		CtrlReg |= PHY_CTRL_DUPLEX;
		CtrlReg |= PHY_CTRL_RESTART_AUTO;
		WritePhyRegister (psDev, PHY_CTRL, CtrlReg); /*SRW- fixed CtrlReg */
	}
	else if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{	
#ifdef DEBUG_VERBOSE
		sprintf (buffer, "MMD 07:0010  0x%04X\n", ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_ADVERTISE)));
		vConsoleWrite (buffer);
		sprintf (buffer, "MMD 07:0020  0x%04X\n", ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_10GB_CTRL)));
		vConsoleWrite (buffer);
		sprintf (buffer, "MMD 30:C400  0x%04X\n", ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_VEND1, PHY_MII_AUTONEG_VENDOR_PROV1)));
		vConsoleWrite (buffer);
#endif

		/* restart auto neg */
		dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL);
		RegData = ReadPhyRegister(psDev, dReg);
		RegData |= PHY_AN_CTRL_RESTART;
		WritePhyRegister(psDev, dReg, RegData);
	}
}

/*******************************************************************
 * vSetLoopbackMode: configure the requested loopback mode
 *
 * RETURNS: none
 */
static void vSetLoopbackMode
(
	DEVICE*	psDev,		/* pointer to device data structure			*/
	UINT8	bType,		/* Interface Type: IF_COPPER or IF_FIBRE	*/
	UINT8	bLbMode,	/* loopback mode							*/
	UINT32	dSpeed		/* loopback speed							*/
)
{
	UINT32	dRegVal;
	UINT16	wTempReg = 0;
	
#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if ((psDev->bController == CTRLT_I350) ||
		(psDev->bController == CTRLT_I210))
	{
		vWriteReg825xx(CTRL_EXT, 0);
	}
	
#ifdef DEBUG
		sprintf (buffer, "loopback mode=%u  speed=%u\n", bLbMode, dSpeed);
		vConsoleWrite (buffer);
#endif

	if (bLbMode == LB_INTERNAL)
	{
		if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
		{
			// speed must have already been set to 10GB
			
			if (psDev->bController != CTRLT_82599_SFP)
			{
				// disable auto negotiation
				wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL));
				wTempReg &= ~PHY_AN_AUTO_NEGOTIATE;
				WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL), wTempReg);
			}

			// set in loopback mode
			dRegVal = dReadReg825xx(HLREG0);
			dRegVal |= HLREG0_LPBK;
			vWriteReg825xx(HLREG0, dRegVal);
			
			dRegVal = dReadReg825xx(FCTRL_X540);
			dRegVal |= FCTRL_SBP | FCTRL_MPE | FCTRL_BAM;
			vWriteReg825xx(FCTRL_X540, dRegVal);

#ifdef DEBUG			
			sprintf (buffer, "HLREG0: 0x%08x FCTRL: 0x%08x\n", dReadReg825xx(HLREG0), dReadReg825xx(FCTRL_X540));
			vConsoleWrite (buffer);
#endif
			if (psDev->bController == CTRLT_82599_SFP)
			{
				// force link up
				dRegVal = dReadReg825xx(AUTOC);
				dRegVal &= ~AUTOC_LMS_MASK;
				dRegVal |= AUTOC_FLU | AUTOC_LMS_10G_LINK_NO_AN
								| AUTOC_AN_RESTART;
				vWriteReg825xx(AUTOC, dRegVal);
			}
			else
			{
				// force link up
				dRegVal = dReadReg825xx(MACC);
				dRegVal |= MACC_FLU;
				vWriteReg825xx(MACC, dRegVal);
			}

			E1E_FLUSH();
			
			if (psDev->bController != CTRLT_82599_SFP)
				vDelay(40);
			else
				vDelay(20);
		}
		else
		{
			// the i350 and i210 do not support 10GB
			if (dSpeed == SPEED_1G)
			{
#ifdef DEBUG
				sprintf (buffer, "Testing %s 1GB/s\n", psDev->achCtrlName);
				vConsoleWrite (buffer);
#endif

				WritePhyRegister (psDev,PHY_CTRL, 0x4140);
				vDelay(50);
			}
			else if (dSpeed == SPEED_100M)
			{
#ifdef DEBUG
				sprintf (buffer, "Testing %s 100MB/s\n", psDev->achCtrlName);
				vConsoleWrite (buffer);
#endif

				WritePhyRegister (psDev,PHY_CTRL, 0x6100);
				vDelay(50);
			}
			else if (dSpeed == SPEED_10M)
			{
#ifdef DEBUG
				sprintf (buffer, "Testing %s 10MB/s\n", psDev->achCtrlName);
				vConsoleWrite (buffer);
#endif

				WritePhyRegister (psDev,PHY_CTRL, 0x4100);
				vDelay(50);
			}
		}
	}
	else
	{
			// bLbMode == LB_EXTERNAL
		if ((psDev->bController == CTRLT_I350) && (dSpeed == SPEED_1G))
		{
			vDelay(10);
			wTempReg = ReadPhyRegister (psDev, 19);
			vDelay(100);
			WritePhyRegister (psDev, 19, (wTempReg | (1<<7)));		// set external cable mode
			vDelay(100);
			wTempReg = ReadPhyRegister (psDev, PHY_CTRL);
			wTempReg |= PHY_CTRL_RESTART_AUTO;
			WritePhyRegister (psDev,PHY_CTRL, wTempReg);
			vDelay(100);
		}
		// bLbMode == LB_EXTERNAL
		if ((psDev->bController == CTRLT_I350) && (dSpeed == SPEED_10M))
		{
			wTempReg = 0x8100;
			WritePhyRegister (psDev,PHY_CTRL, wTempReg);
			vDelay(10);
			wTempReg = 0x0100;
			WritePhyRegister (psDev,PHY_CTRL, wTempReg);
			vDelay(100);
		}
		// bLbMode == LB_EXTERNAL
		if ((psDev->bController == CTRLT_I350) && (dSpeed == SPEED_100M))
		{
			wTempReg = 0xA100;
			WritePhyRegister (psDev,PHY_CTRL, wTempReg);
			vDelay(10);
			wTempReg = 0x2100;
			WritePhyRegister (psDev,PHY_CTRL, wTempReg);
			vDelay(100);
		}
		else if (psDev->bController == CTRLT_I210)
		{
			wTempReg = PHY_CTRL_DUPLEX + PHY_CTRL_AUTO_NEG;
			switch (dSpeed)
			{
				case SPEED_10M:
					break;

				case SPEED_100M:
					wTempReg += PHY_CTRL_SPEED_100;
					break;

				case SPEED_1G:
					wTempReg += PHY_CTRL_SPEED_1000;
					break;
			}

			WritePhyRegister (psDev, PHY_CTRL, wTempReg);
			WritePhyRegister (psDev, PHY_REG(6, 18) , 0x08);		// enable Stub Test

			wTempReg |= PHY_CTRL_RESTART_AUTO;
			WritePhyRegister (psDev, PHY_CTRL, wTempReg);

			PhyReset (psDev);
			vDelay(8000);
		}
		else if ((psDev->bController == CTRLT_X540) && (bLbMode == LB_EXTERNAL) )
		{
			// put in external loopback mode
			wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1));
			wTempReg |= PHY_PMA_EXT_PHY_LOOPBACK;
			WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1), wTempReg);
		}
		else if ((psDev->bController == CTRLT_82599_SFP) && (bLbMode == LB_EXTERNAL) )
		{
			// put in external loopback mode
			wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1));
			wTempReg |= PHY_PMA_EXT_PHY_LOOPBACK;
			WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1), wTempReg);
		}
		else if ((psDev->bController == CTRLT_X540) && (bLbMode == LB_NONE) )
		{
			// take out of external loopback mode
			wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1));
			wTempReg &= ~PHY_PMA_EXT_PHY_LOOPBACK;
			WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1), wTempReg);

			// take out of internal loopback mode
			dRegVal = dReadReg825xx(HLREG0);
			dRegVal &= ~HLREG0_LPBK;
			vWriteReg825xx(HLREG0, dRegVal);
		
			// disable 'force link up'
			dRegVal = dReadReg825xx(MACC);
			dRegVal &= ~MACC_FLU;
			vWriteReg825xx(MACC, dRegVal);
			
			E1E_FLUSH();
			
			// enable auto negotiation
			wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL));
			wTempReg |= PHY_AN_AUTO_NEGOTIATE;
			WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL), wTempReg);
		}
		else if ((psDev->bController == CTRLT_82599_SFP) && (bLbMode == LB_NONE) )
		{
			// take out of external loopback mode
			//wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1));
			//wTempReg &= ~PHY_PMA_EXT_PHY_LOOPBACK;
		//	WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1), wTempReg);

			// take out of internal loopback mode
			dRegVal = dReadReg825xx(HLREG0);
			dRegVal &= ~HLREG0_LPBK;
			vWriteReg825xx(HLREG0, dRegVal);

			// disable 'force link up'
			dRegVal = dReadReg825xx(MACC);
			dRegVal &= ~MACC_FLU;
			vWriteReg825xx(MACC, dRegVal);

			E1E_FLUSH();

			// enable auto negotiation
			//wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL));
			//wTempReg |= PHY_AN_AUTO_NEGOTIATE;
			//WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL), wTempReg);
		}
		else if ((psDev->bController == CTRLT_X540) && (bLbMode == LB_CHANNEL) )
		{
			// take out of external loopback mode
			wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1));
			wTempReg &= ~PHY_PMA_EXT_PHY_LOOPBACK;
			WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1), wTempReg);

			// take out of internal loopback mode
			dRegVal = dReadReg825xx(HLREG0);
			dRegVal &= ~HLREG0_LPBK;
			dRegVal |= 0x08012FFF;
			vWriteReg825xx(HLREG0, dRegVal);

			dRegVal = dReadReg825xx(HLREG1);
			dRegVal = dReadReg825xx(HLREG1);

			dRegVal = 0;
			// enable 'force link up'
			dRegVal = dReadReg825xx(MACC);
			dRegVal = 0x3D00;
			vWriteReg825xx(MACC, dRegVal);

			dRegVal = dReadReg825xx(MFLCN);
			dRegVal |= 0x0A;
			vWriteReg825xx(MFLCN, dRegVal);

			E1E_FLUSH();

			// enable auto negotiation
			wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL));
			wTempReg |= PHY_AN_AUTO_NEGOTIATE;
			WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL), wTempReg);
		}
		else if ((psDev->bController == CTRLT_82599_SFP) && (bLbMode == LB_CHANNEL) )
		{
			// take out of external loopback mode
			//wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1));
			//wTempReg &= ~PHY_PMA_EXT_PHY_LOOPBACK;
			//WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, PHY_PMA_RX_RSVD_VENDOR_PROV1), wTempReg);

			// take out of internal loopback mode
			dRegVal = dReadReg825xx(HLREG0);
			dRegVal &= ~HLREG0_LPBK;
			dRegVal |= 0x08012FFF;
			vWriteReg825xx(HLREG0, dRegVal);

			dRegVal = dReadReg825xx(HLREG1);
			dRegVal = dReadReg825xx(HLREG1);

			dRegVal = 0;
			// enable 'force link up'
			dRegVal = dReadReg825xx(MACC);
			dRegVal = 0x3D00;
			vWriteReg825xx(MACC, dRegVal);

			dRegVal = dReadReg825xx(MFLCN);
			dRegVal |= 0x0A;
			vWriteReg825xx(MFLCN, dRegVal);

			E1E_FLUSH();

			// enable auto negotiation
			//wTempReg = ReadPhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL));
			//wTempReg |= PHY_AN_AUTO_NEGOTIATE;
			//WritePhyRegister(psDev, X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL), wTempReg);
		}
	}
}

/*******************************************************************
 * vInitRxDescr: configure the receiver descriptor Registers
 *
 * RETURNS: none
 */
static void vInitRxDescr (DEVICE *psDev, UINT16 wDescrCount)
{
#ifdef DEBUG
	sprintf (buffer, "%s: wDescrCount: %u\n", __FUNCTION__, wDescrCount);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP )
	{
		vWriteReg825xx (RDBAL_X540(0), psDev->dRxDescAddr);
		vWriteReg825xx (RDBAH_X540(0), 0x00000000);
		vWriteReg825xx (RDLEN_X540(0), psDev->dRxDescSize);
		vWriteReg825xx (RDH_X540(0), 0x00000000);
		vWriteReg825xx (RDT_X540(0), wDescrCount);
#ifdef DEBUG
		sprintf (buffer, "Rx Descriptor address = 0x%08X\n", psDev->dRxDescAddr);
		vConsoleWrite (buffer);
		sprintf (buffer, "Rx Descriptor size = 0x%08X\n", psDev->dRxDescSize);
		vConsoleWrite (buffer);
#endif
	}
	else
	{
		vWriteReg825xx (RDBAL, psDev->dRxDescAddr);
		vWriteReg825xx (RDBAH, 0x00000000);
		vWriteReg825xx (RDLEN, psDev->dRxDescSize);
		vWriteReg825xx (RDH, 0x00000000);
		vWriteReg825xx (RDT, wDescrCount);
	}
}

/*******************************************************************
 * vInitTxDescr: configure the transmitter descriptor Registers
 *
 * RETURNS: none
 */
static void vInitTxDescr (DEVICE *psDev, UINT16 wDescrCount)
{
#ifdef DEBUG
	sprintf (buffer, "%s: wDescrCount: %u\n", __FUNCTION__, wDescrCount);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		vWriteReg825xx (TDBAL_X540(0), psDev->dTxDescAddr);
		vWriteReg825xx (TDBAH_X540(0), 0x00000000);
		vWriteReg825xx (TDLEN_X540(0), psDev->dTxDescSize);
		vWriteReg825xx (TDH_X540(0), 0x00000000);
		vWriteReg825xx (TDT_X540(0), wDescrCount);
#ifdef DEBUG
		sprintf (buffer, "Tx Descriptor address = 0x%08X\n", psDev->dTxDescAddr);
		vConsoleWrite (buffer);
		sprintf (buffer, "Tx Descriptor size = 0x%08X\n", psDev->dTxDescSize);
		vConsoleWrite (buffer);
#endif
	}
	else
	{
		vWriteReg825xx (TDBAL, psDev->dTxDescAddr);
		vWriteReg825xx (TDBAH, 0x00000000);
		vWriteReg825xx (TDLEN, psDev->dTxDescSize);
		vWriteReg825xx (TDH, 0x00000000);
		vWriteReg825xx (TDT, wDescrCount);
	}
}

/*****************************************************************************
 * vGetEthMacAddr: Return station address
 *
 * RETURNS : N/A
 */
void vGetEthMacAddr
(
	UINT8	bInstance,
	UINT8*	pbAddress,
	UINT8	bMode
)
{
	UINT8	bIndex;
	DEVICE	sDevice;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if (dFind825xx (&sDevice, bInstance, bMode) == PCI_SUCCESS)
	{
		dInitPciDevice (&sDevice, 0);
		dGetLanAddr (&sDevice, pbAddress);
	}
	else
	{
		for (bIndex = 0; bIndex < 6; ++bIndex)
		{
			pbAddress [bIndex] = 0;
		}
	}
}

/*******************************************************************
 * dGetLanAddr: read the LAN address from the serial EEPROM
 *
 * CNDM: Changed ordering of the bytes. IA bytes 1 and 2 are the MS
 *       part of the Ethernet address, IA Bytes 3 to 6 are the LS part
 *       of the Ethernet address.
 *
 * RETURNS: none
 */
static UINT32 dGetLanAddr
(
	DEVICE*	psDev,			/* pointer to device data structure */
	UINT8*	pbLanAddr		/* just what is says */
)
{
	UINT16	wTemp;
	UINT16	wAddr;
	UINT16	wPointer;
	UINT16	wLength;
	UINT16	wX540EepromSize;
	UINT16	wEepromWordCnt;
	UINT32	rt;
	UINT32	dReg;
	UINT32	dEecdVal;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	// initialise LAN address buffer
	for (rt = 0; rt < 6; ++rt)
	{
		pbLanAddr [rt] = 0xFF;
	}
	
	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		dReg = EECD_X540;
	}
	else
	{
		dReg = EECD;
	}
	
	dEecdVal = dReadReg825xx (dReg);

#ifdef DEBUG
	sprintf (buffer, "EECD = 0x%08X\n", dEecdVal);
	vConsoleWrite (buffer);
#endif

	if ((dEecdVal & EECD_PRES) == 0)
	{
#ifdef DEBUG
		sprintf (buffer, "EEPROM not present\n");
		vConsoleWrite (buffer);
#endif

		return E__EEPROM_ACCESS;
	}

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		rt = x540AcquireSwFwSync(psDev, SW_FW_SYNC_SW_NVM);
		if (rt != E__OK)
		{
#ifdef DEBUG
			sprintf (buffer, "\nLAN Address  = %02x:%02x:%02x:%02x:%02x:%02x\n",
						pbLanAddr [0], pbLanAddr [1], pbLanAddr [2],
						pbLanAddr [3], pbLanAddr [4], pbLanAddr [5]);
			vConsoleWrite (buffer);
#endif

			return rt;
		}

		wX540EepromSize = (UINT16) ((dEecdVal & EECD_SIZE_X540) >> EECD_SIZE_SHIFT_X540);
		wEepromWordCnt = 1 << (wX540EepromSize + EECD_WORD_SIZE_SHIFT_X540);
		
#ifdef DEBUG
		sprintf (buffer, "wX540EepromSize = 0x%04X, wEepromWordCnt = 0x%04X\n", wX540EepromSize, wEepromWordCnt);
		vConsoleWrite (buffer);
#endif
		wAddr = X540_EEP_LAN0 + psDev->bFunc;
		
#ifdef DEBUG
		sprintf (buffer, "Pointer address = 0x%04X\n", wAddr);
		vConsoleWrite (buffer);
#endif
		
		// get pointer to required LAN structure
		wPointer = wReadEepromWord (psDev, wAddr);
		
#ifdef DEBUG
		sprintf (buffer, "Section pointer = 0x%04X\n", wPointer);
		vConsoleWrite (buffer);
#endif
		if ((wPointer != 0xFFFF) || (wPointer != 0) || (wPointer < wEepromWordCnt))
		{
			// check size of LAN structure
			wLength = wReadEepromWord (psDev, wPointer);
			
#ifdef DEBUG
			sprintf (buffer, "Section length = 0x%04X\n", wLength);
			vConsoleWrite (buffer);
#endif
			if ((wLength != 0xFFFF) || (wLength != 0) || (wLength < wEepromWordCnt))
			{
				// get required LAN information
				wTemp =  wReadEepromWord (psDev, wPointer + 1);
				pbLanAddr [0] = (UINT8) wTemp;
				pbLanAddr [1] = (UINT8)(wTemp >> 8);
				wTemp =  wReadEepromWord (psDev, wPointer + 2);
				pbLanAddr [2] = (UINT8) wTemp;
				pbLanAddr [3] = (UINT8)(wTemp >> 8);
				wTemp =  wReadEepromWord (psDev, wPointer + 3);
				pbLanAddr [4] = (UINT8) wTemp;
				pbLanAddr [5] = (UINT8)(wTemp >> 8);
			}
		}

		x540ReleaseSwFwSync(psDev, SW_FW_SYNC_SW_NVM);
	}
	else		// i210 or 1350
	{
		wTemp =  wReadEepromWord (psDev, EEPROM_IA2_1);
		pbLanAddr [0] = (UINT8) wTemp;
		pbLanAddr [1] = (UINT8)(wTemp >> 8);
		wTemp =  wReadEepromWord (psDev, EEPROM_IA4_3);
		pbLanAddr [2] = (UINT8) wTemp;
		pbLanAddr [3] = (UINT8)(wTemp >> 8);
		wTemp =  wReadEepromWord (psDev, EEPROM_IA6_5);
		pbLanAddr [4] = (UINT8) wTemp;
		pbLanAddr [5] = (UINT8)(wTemp >> 8);

		/* i250 has shared EEPROM , so find correct LAN address */
	
		if ((psDev->bController == CTRLT_I350) && (psDev->bFunc > 0))
		{
			wAddr = (psDev->bFunc * 0x40) + 0x40;
			wTemp =  wReadEepromWord (psDev, wAddr);
			pbLanAddr [0] = (UINT8) wTemp;
			pbLanAddr [1] = (UINT8)(wTemp >> 8);
			wTemp =  wReadEepromWord (psDev, wAddr + 1);
			pbLanAddr [2] = (UINT8) wTemp;
			pbLanAddr [3] = (UINT8)(wTemp >> 8);
			wTemp =  wReadEepromWord (psDev, wAddr + 2);
			pbLanAddr [4] = (UINT8) wTemp;
			pbLanAddr [5] = (UINT8)(wTemp >> 8);
		}
	}
	
#ifdef DEBUG
	sprintf (buffer, "\nLAN Address  = %02x:%02x:%02x:%02x:%02x:%02x\n",
				pbLanAddr [0], pbLanAddr [1], pbLanAddr [2],
				pbLanAddr [3], pbLanAddr [4], pbLanAddr [5]);
	vConsoleWrite (buffer);
#endif

	return E__OK;
}

/*******************************************************************
 * vInitx540: initialize an instance of the x540
 *
 * RETURNS: none
 */
static void vInitx540 
(
	DEVICE*	psDev,		/* pointer to device data structure */
	UINT8	bSpeed		/* required link speed */
)
{
//	UINT32	status;
	UINT32	dReg;
	UINT32	dRegVal;
	UINT32	hlreg0Val;
	UINT16	wPhyVal;
	UINT8	bCnt;
#ifdef DEBUG
	UINT8	bLanAddr[6];		/* just what is says */
#endif

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	// disable the receive unit
	vWriteReg825xx (RXCTRL, 0);
	
	// set interrupt clear mask to disable interrupts
	vWriteReg825xx (EIMC, 0x7FFFFFFFL);
	
	// clear pending interrupts and flush any writes
	dRegVal = dReadReg825xx (EICR);
	
	// disable the transmit unit by disabling each queue
	for (bCnt = 0; bCnt < X540_MAX_TX_QUEUES; ++bCnt)
	{
		vWriteReg825xx (TXDCTL_X540(bCnt), TXDCTL_SWFLSH);
	}
	
	// disable the receive unit by stopping the queues
	for (bCnt = 0; bCnt < X540_MAX_RX_QUEUES; ++bCnt)
	{
		dRegVal = dReadReg825xx (RXDCTL_X540(bCnt));
		dRegVal &= ~RXDCTL_ENABLE;
		dRegVal |= RXDCTL_SWFLSH;
		vWriteReg825xx (RXDCTL_X540(bCnt), dRegVal);
	}
	
	E1E_FLUSH();
	
	// flush pending TX transactions
	hlreg0Val = dReadReg825xx (HLREG0);
	vWriteReg825xx (HLREG0, hlreg0Val | HLREG0_LPBK);				// stop any TX if link comes up
	
	dRegVal = dReadReg825xx (GCR_EXT);
	vWriteReg825xx (GCR_EXT, dRegVal | GCR_EXT_BUFFERS_CLEAR);		// clear PCIE transaction layer

	E1E_FLUSH();
	vDelay(1);

	// restore previous register values
	vWriteReg825xx (GCR_EXT, dRegVal);
	vWriteReg825xx (HLREG0, hlreg0Val);

	// sometimes two resets are needed!
	dCtrlReset (psDev);
	vDelay(100);
	dCtrlReset (psDev);
	
	vWriteReg825xx (RXPBSIZE(0), 384 << RXPBSIZE_SHIFT);	// default to 384KB
	
#ifdef DEBUG
{
		UINT32 rar_high;
		UINT32 rar_low;
		UINT16 i;
	
		rar_high = dReadReg825xx (RAH_X540(0));
		rar_low = dReadReg825xx (RAL_X540(0));
	
		for (i = 0; i < 4; i++)
		{
			bLanAddr[i] = (UINT8)(rar_low >> (i*8));
		}
	
		for (i = 0; i < 2; i++)
		{
			bLanAddr[i+4] = (UINT8)(rar_high >> (i*8));
		}
	
		sprintf (buffer, "\nLAN Address  = %02x:%02x:%02x:%02x:%02x:%02x\n",
					bLanAddr [0], bLanAddr [1], bLanAddr [2],
					bLanAddr [3], bLanAddr [4], bLanAddr [5]);
		vConsoleWrite (buffer);
}
#endif

//	for (bCnt = 0; bCnt < X540_MAX_RAR_ENTRIES; ++bCnt)
//	{
//		vWriteReg825xx (RAL_X540(bCnt), 0);					// clear out MAC addresses
//		vWriteReg825xx (RAH_X540(bCnt), 0);
//	}

	// clear out the MTA (Multicast Table Array))
	vWriteReg825xx (MCSTCTRL, 0);


	for (bCnt = 0; bCnt < X540_MAX_RAR_ENTRIES; ++bCnt)
	{
		vWriteReg825xx (MTA_X540(bCnt), 0);					// clear out Multicast Table Array
	}

	for (bCnt = 0; bCnt < 128; ++bCnt)
	{
		vWriteReg825xx (UTA_X540(bCnt), 0);					// clear out Unicast table array
	}
	
//	vWriteReg825xx (FCTRL, RCTL_LPE         |	/* long Packet Enable	*/
//						RCTL_SBP       		|	/* Store bad packets */
//						RCTL_RDMTS_1_2 		|	/* free buff threshold = RDLEN/2 */
//						RCTL_UPE       		|
//						RCTL_MPE       		|	/* make sure we get everything */
//						RCTL_BSIZE_2048);		/* use 2-kByte RX buffers */

	vWriteReg825xx (FCTRL_X540, FCTRL_SBP |			/* Store bad packets */
								FCTRL_UPE |
								FCTRL_MPE |
                                FCTRL_BAM);			/* make sure we get everything */
								
//	vWriteReg825xx (MCSTCTRL, 0);
	
	/* Disable flow control XON/XOFF frames */
	for (bCnt = 0; bCnt < 8; ++bCnt)
	{
		vWriteReg825xx (FCRTL_X540(bCnt), 0);
		vWriteReg825xx (FCRTH_X540(bCnt), 0);
	}

	/* configure receive descriptor ring */
	for (bCnt = 0; bCnt < X540_MAX_RX_QUEUES; ++bCnt)
	{
		vWriteReg825xx (RDBAL_X540(bCnt), psDev->dRxDescAddr);
		vWriteReg825xx (RDBAH_X540(bCnt), 0x00000000);
		vWriteReg825xx (RDLEN_X540(bCnt), psDev->dRxDescSize);
		vWriteReg825xx (RDH_X540(bCnt), 0x00000000);				/* ring is empty so head = tail = 0 */
		vWriteReg825xx (RDT_X540(bCnt), 0x00000000);
		vWriteReg825xx (RXDCTL_X540(bCnt), RXDCTL_ENABLE);
	}

	for (bCnt = 0; bCnt < 32; ++bCnt)
	{
		vWriteReg825xx (TDBAL_X540(bCnt), psDev->dTxDescAddr);
		vWriteReg825xx (TDBAH_X540(bCnt), 0x00000000);
		vWriteReg825xx (TDLEN_X540(bCnt), psDev->dTxDescSize);
		vWriteReg825xx (TDH_X540(bCnt), 0x00000000);				/* ring is empty so head = tail = 0 */
		vWriteReg825xx (TDT_X540(bCnt), 0x00000000);				/* TEST purposes only ... */
		vWriteReg825xx (TXDCTL_X540(bCnt), TXDCTL_ENABLE | (1 << TXDCTL_WTHRESH_SHIFT));
	}
	
	/* Don't need IP or TCP/UDP checksum offload */
	vWriteReg825xx (RXCSUM, 0);
	
//	// disable auto negotiation
//	dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_CTRL);
//	wPhyVal = ReadPhyRegister(psDev, dReg);
//	wPhyVal &= ~PHY_AN_AUTO_NEGOTIATE;
//	WritePhyRegister(psDev, dReg, wPhyVal);
	
	// set No Snoop Disable
	dRegVal = dReadReg825xx (CTRL_EXT);
	dRegVal |= CTRLEXT_NS_DIS;
	vWriteReg825xx (CTRL_EXT, dRegVal);
	
	// disable flow control
	dReg = X540_PHY_REG(MDIO_MMD_AN, PHY_AN_ADVERTISE);
	wPhyVal = ReadPhyRegister(psDev, dReg);
	wPhyVal &= ~(PHY_AN_SYM_PAUSE_FULL | PHY_AN_ASM_PAUSE_FULL);
	WritePhyRegister(psDev, dReg, wPhyVal);
	
	for (bCnt = 0; bCnt < 4; ++bCnt)
	{
		vWriteReg825xx (FCTTV(bCnt), 0);
	}
	
	for (bCnt = 0; bCnt < 8; ++bCnt)
	{
		vWriteReg825xx (FCRTL_X540(bCnt), 0);
		vWriteReg825xx (FCRTH_X540(bCnt), 0);
	}
	
	vWriteReg825xx (FCRTV, 0);
	vWriteReg825xx (FCCFG, 0);

	// clear rate limiters
	for (bCnt = 0; bCnt < X540_MAX_TX_QUEUES; ++bCnt)
	{
		vWriteReg825xx (RTTDQSEL, bCnt);
		vWriteReg825xx (RTTBCNRC, 0);
	}
	E1E_FLUSH();
	
	// disable relaxed ordering
	for (bCnt = 0; bCnt < X540_MAX_TX_QUEUES; ++bCnt)
	{
		dRegVal = dReadReg825xx (DCA_TXCTRL(bCnt));
		dRegVal &= ~DCA_TXCTRL_DESC_WRO_EN;
		vWriteReg825xx (DCA_TXCTRL(bCnt), dRegVal);
	}

	for (bCnt = 0; bCnt < X540_MAX_RX_QUEUES; ++bCnt)
	{
		dRegVal = dReadReg825xx (DCA_RXCTRL(bCnt));
		dRegVal &= ~(DCA_RXCTRL_DATA_WRO_EN | DCA_RXCTRL_HEAD_WRO_EN);
		vWriteReg825xx (DCA_RXCTRL(bCnt), dRegVal);
	}
}

/*******************************************************************
 * vInit825xx: initialize an instance of the 8257x
 *
 * RETURNS: none
 */
static void vInit825xx
(
	DEVICE*	psDev,		/* pointer to device data structure */
	UINT8	bSpeed		/* required link speed */
)
{
	UINT8	abLanAddr[6];
	UINT32	dRegVal;
	PCI_PFA	pfa;
//	UINT32	dTarc = 0;
#ifdef DEBUG
//	UINT32	dRet = 0;
//	UINT32	dPhyCtrl = 0;
//	UINT32	dExtCnfCtrl = 0;
//	char	buffer[0x64];
#endif

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if ((psDev->bController == CTRLT_X540) || (psDev->bController == CTRLT_82599_SFP))
	{
		vInitx540 (psDev, bSpeed);
	}
	else
	{
		/*
		* Initialize the registers
		*/
		/* Set control register to select speed and full-duplex mode */
		dRegVal = dReadReg825xx (CTRL);
		dRegVal = CTRL_FD |			/* always full duplex */
					CTRL_FRCDPLX;	/* force duplex - we select it */
		
		switch (bSpeed)
		{
			case SPEED_AUTO:
				dRegVal |= CTRL_ASDE;		/* auto detect speed */
				break;
		
//			case SPEED_10G:
//				dRegVal |= CTRL_SPEED10000;
//				break;
		
			case SPEED_1G:
				dRegVal |= CTRL_SPEED1000;
				break;
		
			case SPEED_100M:
				dRegVal |= CTRL_SPEED100;
				break;
		
			default :
				dRegVal |= CTRL_SPEED10;
				break;
		}
		
		if (bSpeed != SPEED_AUTO)
		{
			dRegVal |= CTRL_FRCSPD;		/* force speed - we select it */
		}
		
		vWriteReg825xx (CTRL, dRegVal);
		
		if ((psDev->bController == CTRLT_I350) ||
			(psDev->bController == CTRLT_I210))
		{
			vWriteReg825xx (CTRL_EXT, 0x00000000);
		}
		
		/* Extended control - bypass auto-speed selection */
		if (bSpeed == SPEED_AUTO)
		{
			dRegVal = dReadReg825xx (CTRL_EXT);
			dRegVal |= CTRLEXT_ASDCHK;
			dRegVal &= ~CTRLEXT_SPDBYPS;
			vWriteReg825xx (CTRL_EXT, dRegVal);
		}
		else
		{
			vWriteReg825xx (CTRL_EXT, dReadReg825xx (CTRL_EXT) | CTRLEXT_SPDBYPS);
		}

	
		pfa = PCI_MAKE_PFA(psDev->bBus, psDev->bDev, psDev->bFunc);
		dRegVal = dPciReadReg (pfa, PCI_VID, PCI_DWORD);
	
		/* Clear any pending interrupts */
	
		vWriteReg825xx (IMC, I8257x_VALID_INTERRUPTS);	/* clear interrupt mask */
		(void)dReadReg825xx (ICR);				/* read status to clear active */
	
		/* Receive control register */
	
		vWriteReg825xx (RCTL, RCTL_LPE         |	/* long Packet Enable	*/
								RCTL_SBP       |	/* Store bad packets */
								RCTL_RDMTS_1_2 |	/* free buff threshold = RDLEN/2 */
								RCTL_UPE       |
								RCTL_MPE       |	/* make sure we get everything */
								RCTL_BSIZE_2048);	/* use 2-kByte RX buffers */
	
		/* Disable flow control XON/XOFF frames */
	
		vWriteReg825xx (FCRTL, 0);
		vWriteReg825xx (FCRTH, 0);
	
		/* Receive descriptor ring */
	
		vWriteReg825xx (RDBAL, psDev->dRxDescAddr);
		vWriteReg825xx (RDBAH, 0x00000000);
		vWriteReg825xx (RDLEN, psDev->dRxDescSize);
		vWriteReg825xx (RDH, 0x00000000);				/* ring is empty so head = tail = 0 */
		vWriteReg825xx (RDT, 0x00000000);
	
		/* Set zero delay for receive interrupts */
	
//		vWriteReg825xx (RDTR, 0);
	
		/* Receive descriptor control */
		dRegVal = 0;
		dRegVal = (1 << S__WTHRESH);
	
//		if ((psDev->bController == CTRLT_I350 ||
//			(psDev->bController == CTRLT_I210))
//		{
			dRegVal |= (1 << 25);
//		}
//		else
//			dRegVal |= (1 << S__GRAN);
	
		vWriteReg825xx (RXDCTL, dRegVal); /*** Need to select this for tuning ***/
	
	
		/* Set inter-packet gap for IEEE 802.3 standard */
//		if ((psDev->bController == CTRLT_I350) ||
//			(psDev->bController == CTRLT_I210))
//		{
			vWriteReg825xx (TIPG, 0x00601008);
//		}
//		else
//			vWriteReg825xx (TIPG, 0x00600806);
	
		/* Set IFS throttle to (recommended) zero for present */
	
//		vWriteReg825xx (AIT, 0);
	
		/* Set Transfer DMA control to (recommended) zero */
	
//		vWriteReg825xx (TXDMAC, 0);
	
		/* Transmit descriptor ring */
	
		vWriteReg825xx (TDBAL, psDev->dTxDescAddr);
		vWriteReg825xx (TDBAH, 0x00000000);
		vWriteReg825xx (TDLEN, psDev->dTxDescSize);
		vWriteReg825xx (TDH, 0x00000000); /* ring is empty so head = tail = 0 */
		vWriteReg825xx (TDT, 0x00000000); /* TEST purposes only ... */
	
		/* Set zero delay for transmit interrupts */
	
//		vWriteReg825xx (TIDV, 1); /* use 1 as 0 is not allowed ... CNDM */
	
		/* Transmit descriptor control */
		dRegVal = 0;
		dRegVal = (1 << S__WTHRESH);
	
//		if ((psDev->bController == CTRLT_I350) ||
//			(psDev->bController == CTRLT_I210))
//		{
			dRegVal |= (1 << 25);
//		}
//		else
//			dRegVal |= (1 << S__GRAN);
	
		vWriteReg825xx (TXDCTL, dRegVal); /** Need to select this for tuning **/
	
		/* Dont' need IP or TCP/UDP checksum offload */
	
		vWriteReg825xx (RXCSUM, 0);
	
		/* Set receive address (from EEPROM) */
		dRegVal = 0;
		dGetLanAddr (psDev, abLanAddr);
		dRegVal = abLanAddr[0] |
					(abLanAddr[1] << 8) |
					(abLanAddr[2] << 16) |
					(abLanAddr[3] << 24);
	
		vWriteReg825xx (RAL, dRegVal);
		dRegVal = abLanAddr[4] | (abLanAddr[5] << 8);
		vWriteReg825xx (RAH, dRegVal | RAH_AV); /* and mark as valid */
	
		/* disable auto negotiation */
		dRegVal = dReadReg825xx (TXCW);
		vWriteReg825xx (TXCW, dRegVal & (~TXCW_ANE));
	}
}

/*******************************************************************
 * wCreateDescriptorRings: prepare data structures in memory
 *
 * RX descriptors reference buffers that are fixed as 2-kBytes in size;
 * TX buffers are limited only by the maximum size (16288 bytes), but we
 * will use 2-kBytes for those too.
 *
 * RETURNS: Number of desriptors used
 */
static UINT16 wCreateDescriptorRings
(
	DEVICE	*psDev,				/* pointer to device data structure		*/
	const	UINT8 *pbSrcAddr,	/* source LAN address (local address)	*/
	const	UINT8 *pbDestAddr,	/* destination LAN address				*/
	UINT16	wDataSize,			/* size of data part of frame			*/
	UINT16	wNumFrames			/* number of frames to create			*/
)
{
	UINT32	dTxBuffAddr;		/* Address of TX buffers */
	UINT32	dRxBuffAddr;		/* Address of RX buffers */
	UINT16	wNumBuff;			/* number of buffers required by each frame */
	UINT16	wLastBlkSize;		/* size of last block of buffer */
	UINT16	wCount;				/* counter for frames */
	UINT16	wIndex;				/* counter for blocks within frame */
	UINT16	wDesc;				/* descriptor number */
	UINT8*	pbTx;				/* pointer to TX buffer data */
	UINT8*	pbRx;				/* pointer to RX buffer data */

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* Clear out all the descriptors */

	dTxBuffAddr = psDev->dTxBuffAddr;
	dRxBuffAddr = psDev->dRxBuffAddr;

	for (wCount = 0; wCount < TX_NUM_DESC; ++wCount)
	{
		/* TX descriptors */

		psDev->psTxDesc [wCount].dAddrL    = dTxBuffAddr;
		psDev->psTxDesc [wCount].dAddrH    = 0L;
		psDev->psTxDesc [wCount].wLength   = 0;
		psDev->psTxDesc [wCount].bCsOffset = 0;
		psDev->psTxDesc [wCount].bCommand  = TDESC_COMMAND_RS;
		psDev->psTxDesc [wCount].bStatus   = 0;
		psDev->psTxDesc [wCount].bCsStart  = 0;
		psDev->psTxDesc [wCount].wSpecial  = 0;
		dTxBuffAddr += TX_BUFF_SIZE;
	}

	for (wCount = 0; wCount < RX_NUM_DESC; ++wCount)
	{
		/* RX descriptors */

		psDev->psRxDesc [wCount].dAddrL   = dRxBuffAddr;
		psDev->psRxDesc [wCount].dAddrH   = 0L;
		psDev->psRxDesc [wCount].wLength  = 0;
		psDev->psRxDesc [wCount].bStatus  = 0;
		psDev->psRxDesc [wCount].bErrors  = 0;
		psDev->psRxDesc [wCount].wSpecial = 0;
		dRxBuffAddr += RX_BUFF_SIZE;
	}

	/* Find number of descriptors required to cover each buffer and size of
	 * last block in buffer.
	 */

	wNumBuff = (wDataSize + ETH_OH + (TX_BUFF_SIZE - 1)) / TX_BUFF_SIZE;
	wLastBlkSize = wDataSize + ETH_OH - (TX_BUFF_SIZE * (wNumBuff - 1));

	/*
	 * Prepare the buffer descriptors, process each frame in turn
	 */

	dTxBuffAddr = psDev->dTxBuffAddr;
	dRxBuffAddr = psDev->dRxBuffAddr;

	for (wCount = 0; wCount < wNumFrames; ++wCount)
	{
		for (wIndex = 0; wIndex < wNumBuff; ++wIndex)
		{
			wDesc = (wCount * wNumBuff) + wIndex;

			/* TX descriptors */

			psDev->psTxDesc[wDesc].dAddrL	= dTxBuffAddr;
			psDev->psTxDesc[wDesc].dAddrH	= 0L;
			psDev->psTxDesc[wDesc].wLength	= TX_BUFF_SIZE;
			psDev->psTxDesc[wDesc].bCsOffset= 0;
			psDev->psTxDesc[wDesc].bCommand	= TDESC_COMMAND_RS;
			psDev->psTxDesc[wDesc].bStatus	= 0;
			psDev->psTxDesc[wDesc].bCsStart	= 0;
			psDev->psTxDesc[wDesc].wSpecial	= 0;
			dTxBuffAddr += TX_BUFF_SIZE;
		}

		/* Adjust last TX descriptors command for each buffer */

		psDev->psTxDesc [wDesc].bCommand = TDESC_COMMAND_RS |
		                                   TDESC_COMMAND_IFCS |
		                                   TDESC_COMMAND_EOP;

		psDev->psTxDesc [wDesc].wLength  = wLastBlkSize;
	}

	/*
	 * Initialize each frame in turn
	 */

	pbTx = psDev->pbTxBuff;
	pbRx = psDev->pbRxBuff;

	for (wCount = 0; wCount < wNumFrames; wCount++)
	{
		/* TX buffers */

		memcpy (pbTx, pbDestAddr, 6);			/* 1st: Destination Address	*/
		memcpy (pbTx + 6, pbSrcAddr,  6);		/* 2nd: Source Address		*/
		pbTx [12] = (UINT8) (wDataSize >> 8);	/* 3rd: Frame Length (MSB)	*/
		pbTx [13] = (UINT8) wDataSize;			/* 4th: Frame Length (LSB)	*/

		pbTx [14] = (UINT8) (wCount & 0xff);	/* frame count for debug 	*/
		pbTx [15] = (UINT8) (wCount >> 8);

		memcpy (pbTx + ETH_TOP_OH + 2, test_data, 64);
		memcpy (pbTx + ETH_TOP_OH + 66, pbTx + ETH_TOP_OH, wDataSize - 66);

		pbTx += (TX_BUFF_SIZE * wNumBuff);

		/* RX buffers */

		memset (pbRx, 0, wDataSize + ETH_OH);

		pbRx += (RX_BUFF_SIZE * wNumBuff);
	}

	wDesc = wNumFrames * wNumBuff;
	return wDesc;
}

/*******************************************************************
 * dEpromTest: Test the EEPROM connectivity and checksum
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dEpromTest
(
	DEVICE* psDev		/* pointer to device data structure */
)
{
//	UINT16	awEeprom[EEPROM_SIZE];
	static UINT16	awEeprom[0x1000];
	UINT8	bIndex;
	UINT16	wChecksum;
	UINT16	wX540EepromSize;
	UINT16	wEepromWordCnt;
	UINT16	wPointer;
	UINT16	wLength;
	UINT16	wVal;
	UINT32	dReg;
	UINT32	dEecdVal;
	UINT32	rt;
	int		i, j;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		rt = x540AcquireSwFwSync(psDev, SW_FW_SYNC_SW_NVM);
		if (rt != E__OK)
		{
			return rt;
		}
		dReg = EECD_X540;
	}
	else //	if ((psDev->bController == CTRLT_I350) || (psDev->bController == CTRLT_I210))
	{
		dReg = EECD;
	}
	
	dEecdVal = dReadReg825xx (dReg);
	
#ifdef DEBUG
		sprintf (buffer, "EECD = 0x%08X\n", dEecdVal);
		vConsoleWrite (buffer);
#endif

	if ((dEecdVal & EECD_PRES) == 0)
	{
#ifdef DEBUG
		sprintf (buffer, "EEPROM not present\n");
		vConsoleWrite (buffer);
#endif

		return E__EEPROM_ACCESS;
	}

	/* Read the EEPROM contents to an array */
	for (bIndex = 0; bIndex < EEPROM_SIZE; ++bIndex)
	{
		if ((psDev->bController == CTRLT_I350) && (psDev->bFunc > 0))
		{
			awEeprom[bIndex] = wReadEepromWord (psDev, bIndex + ((psDev->bFunc * 0x40) + 0x40));
		}
		else
		{
			awEeprom[bIndex] = wReadEepromWord (psDev, bIndex);
		}
	}

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		x540ReleaseSwFwSync(psDev, SW_FW_SYNC_SW_NVM);
	}
	
#ifdef DEBUG
	/* Display the contents */

	vConsoleWrite ("EEPROM Contents\n");
	for (bIndex = 0; bIndex < EEPROM_SIZE; bIndex += 4)
	{
		sprintf (buffer, "%02X:%04X  %02X:%04X  %02X:%04X  %02X:%04X\n",
					bIndex,		awEeprom[bIndex],
					bIndex + 1, awEeprom[bIndex + 1],
					bIndex + 2, awEeprom[bIndex + 2],
					bIndex + 3, awEeprom[bIndex + 3]);
		vConsoleWrite (buffer);
	}
#endif

	/* Compute the Checksum */
	wChecksum = 0;

	for (bIndex = 0; bIndex < EEPROM_SIZE; ++bIndex)
	{
		wChecksum += awEeprom[bIndex];
	}

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		wX540EepromSize = (UINT16) ((dEecdVal & EECD_SIZE_X540) >> EECD_SIZE_SHIFT_X540);
		wEepromWordCnt = 1 << (wX540EepromSize + EECD_WORD_SIZE_SHIFT_X540);
		
#ifdef DEBUG
		sprintf (buffer, "wX540EepromSize = 0x%04X, wEepromWordCnt = 0x%04X\n", wX540EepromSize, wEepromWordCnt);
		vConsoleWrite (buffer);
#endif

		/*
		* Include all data from pointers 0x3, 0x6-0xE. This excludes the
		* FW, PHY module, and PCIe Expansion/Option ROM pointers.
		*/
		for (i = X540_EEP_ANALOG; i < X540_EEP_FW; ++i)
		{
			/* ignore PHY and Option Rom sections */
			if (psDev->bController != CTRLT_82599_SFP)
			{
				if (i == X540_EEP_PHY || i == X540_EEP_OPT_ROM)
					continue;
			}

			wPointer = wReadEepromWord (psDev, i);
			
#ifdef DEBUG
			sprintf (buffer, "Section pointer 0x%X= 0x%04X\n", i, wPointer);
			vConsoleWrite (buffer);
#endif

			/* Skip pointer section if the pointer is invalid. */
			if ((wPointer == 0xFFFF) || (wPointer == 0) || (wPointer >= wEepromWordCnt))
				continue;
				
			wLength = wReadEepromWord (psDev, wPointer);

#ifdef DEBUG
			sprintf (buffer, "Section length = 0x%04X\n", wLength);
			vConsoleWrite (buffer);
#endif

			/* Skip pointer section if length is invalid. */
			if ((wLength == 0xFFFF) || (wLength == 0) || ((wPointer + wLength) >= wEepromWordCnt))
				continue;
				
			for (j = wPointer + 1; j <= wPointer + wLength; ++j)
			{
				wVal = wReadEepromWord (psDev, j);
				wChecksum += wVal;
			}
		}
	}

	if (wChecksum != EEPROM_CHECKSUM)
	{
#ifdef DEBUG
		sprintf (buffer, "EEPROM content does not sum to %04Xh, actual = %04Xh\n",
						EEPROM_CHECKSUM, wChecksum);
		vConsoleWrite (buffer);
#endif

		return (E__EEPROM_CHECKSUM);
	}

	return(E__OK);
}

/*******************************************************************
 * dEpromTestNVM: Test the EEPROM connectivity and checksum
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dEpromTestNVM
(
	DEVICE*	psDev		/* pointer to device data structure */
)
{
	UINT8	bIndex;
	UINT32	adInvm[INVM_SIZE];

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* clear iNVM data */
	memset (adInvm, 0, (sizeof(UINT32)) * INVM_SIZE);

	/* Read the iNVM contents into buffer */
	for (bIndex = 0; bIndex < INVM_SIZE; ++bIndex)
	{
		adInvm[bIndex] = dReadReg825xx (INVM_DATA_REG (bIndex));
	}

	if (adInvm[0] == 0)
	{
#ifdef DEBUG
		vConsoleWrite ("iNVM empty\n");
#endif

		return E__EEPROM_ACCESS;
	}

#ifdef DEBUG
	/* Display the raw contents */
	vConsoleWrite ("iNVM Contents\n");
	for (bIndex = 0; bIndex < INVM_SIZE; bIndex += 4)
	{
		sprintf (buffer, "%02X:%08X  %02X:%08X  %02X:%08X  %02X:%08X\n",
					bIndex,		adInvm[bIndex],
					bIndex + 1, adInvm[bIndex + 1],
					bIndex + 2, adInvm[bIndex + 2],
					bIndex + 3, adInvm[bIndex + 3]);
		vConsoleWrite (buffer);
	}

	vConsoleWrite ("Decoded iNVM Contents\n");
	for (bIndex = 0; bIndex < INVM_SIZE; ++bIndex)
	{
		switch (adInvm[bIndex] & INVM_STRUCT_TYPE_MASK)
		{
			case INVM_NVM_END:
				bIndex = INVM_SIZE;				// end of iNVM data
				break;

			case INVM_WORD_AUTOLOAD:
				sprintf (buffer, "Word: %02X=%04X Reset=%d\n",
							(adInvm[bIndex] & INVM_WORD_ADDR_MASK) >> 9,
							adInvm[bIndex] >> 16,
							(adInvm[bIndex] & INVM_RESET_TYPE_MASK) >> 3);

				vConsoleWrite (buffer);
				break;

			case INVM_CSR_AUTOLOAD:
				sprintf (buffer, "CSR: %04X=%08X Reset=%d\n",
							(adInvm[bIndex] >> 16) & INVM_CSR_ADDR_MASK,
							adInvm[bIndex + 1],
							(adInvm[bIndex] & INVM_RESET_TYPE_MASK) >> 3);

				vConsoleWrite (buffer);
				++bIndex;
				break;

			case INVM_PHY_AUTOLOAD:
				sprintf (buffer, "PHY: %02X=%04X Reset=%d\n",
							(adInvm[bIndex] & INVM_PHY_ADDR_MASK) >> 11,
							adInvm[bIndex] >> 16,
							(adInvm[bIndex] & INVM_RESET_TYPE_MASK) >> 3);

				vConsoleWrite (buffer);
				break;

			case INVM_INVALIDATED:
			default:
				break;
		}
	}
#endif

	return(E__OK);
}

/* Clear Transmit pending.
 *
 *
 */
void dClearTxPending(DEVICE* psDev)
{
    UINT32 gcr_ext=0, hlreg0=0;

    /*
     * Set loopback enable to prevent any transmits from being sent
     * should the link come up.  This assumes that the RXCTRL.RXEN bit
     * has already been cleared.
     */

    hlreg0 = dReadReg825xx(HLREG0);

    vWriteReg825xx(HLREG0, hlreg0 | HLREG0_LPBK);

    /* initiate cleaning flow for buffers in the PCIe transaction layer */
    gcr_ext=  dReadReg825xx(GCR_EXT);
    vWriteReg825xx(GCR_EXT,gcr_ext|GCR_EXT_BUFFERS_CLEAR);

    /* Flush all writes and allow 20usec for all transactions to clear */
    E1E_FLUSH();
    vDelay (200);

    /* restore previous register values */
    vWriteReg825xx(GCR_EXT,gcr_ext);
    vWriteReg825xx(HLREG0, hlreg0);
}

/*******************************************************************
 * dCtrlReset: Reset the selected Ethernet controller
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dCtrlReset (DEVICE *psDev)
{
	UINT32	dTemp = 0;
	UINT16	wResetTimer = 0;
	UINT32	dResetBits = CTRL_RST;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if ((psDev->bController == CTRLT_X540) || psDev->bController == CTRLT_82599_SFP)
		dClearTxPending(psDev);

	dTemp = dReadReg825xx (CTRL);
	vWriteReg825xx (CTRL, dTemp | dResetBits);
	E1E_FLUSH();
	
	vDelay(20);
	wResetTimer = 200; /* allow 200ms for RESET to complete ... */

	if ((psDev->bController == CTRLT_X540) || psDev->bController == CTRLT_82599_SFP)
	{
		while (((dReadReg825xx (CTRL) & IXGBE_CTRL_RST_MASK) == IXGBE_CTRL_RST_MASK) && (wResetTimer > 0))
		{
			--wResetTimer;
			vDelay (1);
		}
	}
	else
	{
		while (((dReadReg825xx (CTRL) & CTRL_RST) == CTRL_RST) && (wResetTimer > 0))
		{
			--wResetTimer;
			vDelay (1);
		}
	}


	if ((psDev->bController == CTRLT_I350)  ||
		(psDev->bController == CTRLT_I210))
	{
		wResetTimer = 200; /* allow 200ms for RESET to complete ... */
		while ((dReadReg825xx (PCIE_CTRL_GCR) & DEV_RESET_INPROGRS) == DEV_RESET_INPROGRS && (wResetTimer >0))
		{
			--wResetTimer;
			vDelay(1);
		}
	}

	if ((psDev->bController == CTRLT_X540) || (psDev->bController == CTRLT_82599_SFP))
	{
		if(wResetTimer != 0)
			return E__OK;
		else
			return E__CTRL_RESET_TIMEOUT;
	}
	else
	{
		/* Reset PHY */
		if(wResetTimer != 0)
		{
			/* Reset PHY */
			vDelay (50);
			vWriteReg825xx (CTRL, dTemp);
			vDelay (400);
			return E__OK;
		}
		else
		{
			return E__CTRL_RESET_TIMEOUT;
		}
	}
}

/*******************************************************************
 * dEepromrwtest:
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dEepromrwtest
(
	DEVICE*	psDev,	/* pointer to device data structure */
	UINT8	bType	/* Interface Type: IF_COPPER or IF_FIBRE	*/
)
{
	UINT16	wOrgChecksum = 0;
	UINT32	dstatus;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if ((bType != IF_COPPER) && (bType != IF_FIBER) && (bType != IF_SERDES))
	{
#ifdef DEBUG
		vConsoleWrite ("Invalid Media Type specified\n");
#endif

		return E__EEPROM_BAD_MEDIA;
	}

	if ( psDev->bController == CTRLT_I210 ||  (psDev->bController == CTRLT_82599_SFP) )
	{
		return (E__OK);
	}

	psDev->bType = bType;

	/* Get device details for this Bus/Device */

	dInitPciDevice (psDev, 0);

	/* Give details of what we are about to do */
#ifdef DEBUG
	sprintf (buffer, "EEPROM Read and Write %s [%s] on bus:%d, device:%d, function:%d\n",
						 psDev->achCtrlName,
						 aachMediaType [bType],
						 psDev->bBus, psDev->bDev, psDev->bFunc);
	vConsoleWrite (buffer);
#endif

	wOrgChecksum = wReadEepromWord (psDev, EEPROM_CSUM);

	/* Write new checksum to EEPROM */
	dstatus = dWriteEepromWord (psDev, EEPROM_CSUM, 0xABCD);

	if (dstatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("Error writing to EEPROM Checksum\n");
#endif

		return (dstatus);
	}

	/* Read back and verify */

	if(wReadEepromWord (psDev, EEPROM_CSUM) != 0xABCD)
	{
#ifdef DEBUG
		sprintf (buffer, "Error verifying EEPROM Checksum "
				"read %04Xh, expected 0xABCD\n",
				wReadEepromWord (psDev, EEPROM_CSUM));
		vConsoleWrite (buffer);
#endif

		return (E__EEPROM_RW);
	}

	/* Force re-load of EEPROM ckecksum */
	dstatus = dWriteEepromWord (psDev, EEPROM_CSUM, wOrgChecksum);

	if (dstatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("Error writing to EEPROM Checksum\n");
#endif

		return (dstatus);
	}

	return (E__OK);
}

/*******************************************************************
 * dLocalLoopback:
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dLocalLoopback
(
	DEVICE*	psDev,	/* pointer to device data structure */
	UINT8	bType,	/* Interface Type: IF_COPPER or IF_FIBRE	*/
	UINT32	dSpeed	/* speed for test */
)
{
	UINT32	dTemp;
	UINT32	dMask;
	UINT32	i;
	UINT16	wCompleted;
	UINT16	wDescrCount;
	UINT16	wTimer;
	UINT32	dStatus;
#ifdef DEBUG
	UINT16	reg_data;
#endif
	UINT8	abEthernetAddress [6];
//	UINT8 link;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* Check the Media Type: As Fibre Optic interfaces operate at 1000Mb/s
	 * then loopback can not be selected as the 82543GC performs echo
	 * cancellation and the message is effectively lost. Fail with an
	 * "Invalid Media" error for fibre optic interfaces.
	 */

	if ((bType == IF_FIBER) || (bType == IF_SERDES))
	{
#ifdef DEBUG
		vConsoleWrite ("ERROR - Bad Media selected for local loopback test\n");
#endif

		return (E__LOC_LB_BAD_MEDIA);
	}


	/* Reset the Ethernet Controller	*/
	dStatus = dCtrlReset (psDev);

	if (dStatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("ERROR - Timed out whilst resetting the controller\n");
#endif

		return E__CTRL_RESET_TIMEOUT;
	}

	/* Get this device's ethernet address */
	dGetLanAddr (psDev, abEthernetAddress);
	vClear8257xStatistics (psDev);

	PhySpeed (psDev, dSpeed);
	E1E_FLUSH();
	vDelay (10);

	if (psDev->bController != CTRLT_I350)
		dStatus = PhyReset (psDev);

	if (dStatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("ERROR - Timed out whilst resetting the PHY\n");
#endif

		return E__PHYRESET_TIMEOUT;
	}

	/* Initialize the device ...	*/
#ifdef DEBUG
	vConsoleWrite ("Initializing the device ...\n");
#endif

	vInit825xx (psDev, dSpeed);
	E1E_FLUSH();
	
	/* Create the descriptor rings ...	*/
#ifdef DEBUG
	vConsoleWrite ("Creating descriptor rings ...\n");
#endif

	wDescrCount = wCreateDescriptorRings (psDev, abEthernetAddress,
											abEthernetAddress, LOOP_BACK_SIZE, 1);
	vInitRxDescr (psDev, 0);
	vDelay(40);
	vInitTxDescr (psDev, 0);
	
#ifdef DEBUG
	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		sprintf (buffer, "LINKS: 0x%08X\n", dReadReg825xx (LINKS));
		vConsoleWrite (buffer);
	}
#endif

	/* Set mode	*/
#ifdef DEBUG
	vConsoleWrite ("Entering external loopback mode ...\n");
#endif

	vSetLoopbackMode (psDev, bType, LB_EXTERNAL, dSpeed);
	vDelay (10);

#ifdef DEBUG
	vConsoleWrite ("Attempting to establish the link ...\n");
#endif


	if (psDev->bController == CTRLT_I350)
	{
		/* Reset the LINK (1 -> 0) ... causing auto-negotiation ... */
		dTemp = dReadReg825xx (CTRL);
		vWriteReg825xx (CTRL, (dTemp | CTRL_SLU | CTRL_FRCSPD | CTRL_FRCDPLX | CTRL_FD));
		vDelay (100);
	}
	else if (psDev->bController == CTRLT_I210)
	{
#ifdef DEBUG
		reg_data = ReadPhyRegister(psDev, PHY_STATUS);
		sprintf (buffer, "PHY_STATUS %#04X\n", reg_data);
		vConsoleWrite (buffer);
#endif

		dTemp = dReadReg825xx (CTRL);
		dTemp &= M__CTRL_SPEED;
		switch (dSpeed)
		{
			case SPEED_10M:
				break;

			case SPEED_100M:
				dTemp += CTRL_SPEED100;
				break;
			case SPEED_1G:
				dTemp += CTRL_SPEED1000;
				break;
		}

 		vWriteReg825xx (CTRL, (dTemp | CTRL_FRCSPD | CTRL_FRCDPLX | CTRL_FD | CTRL_SLU));
 		vDelay (100);
	}

#ifdef DEBUG
	vConsoleWrite ("Checking to see if the link could be established ...\n");
#endif

	if ((psDev->bController == CTRLT_X540) || (psDev->bController == CTRLT_82599_SFP))
	{
		/* Check link up using the link status register, as using */
		/* the general status register is only useful in IOV mode */
		dMask = LINKS_UP;
		
		wTimer = 20000;

		while (((dReadReg825xx (LINKS) & dMask) == 0) && (wTimer > 0))
		{
			vDelay (1);
			--wTimer;
		}
		
#ifdef DEBUG
		sprintf (buffer, "LINKS: 0x%08X\n", dReadReg825xx (LINKS));
		vConsoleWrite (buffer);
#endif		
	}
	else
	{
		if ((psDev->bController == CTRLT_I350) ||
			(psDev->bController == CTRLT_I210))
		{
			vDelay(19*100);
			wTimer = 20000;
		}
		else
			wTimer = 10000;
	
		dMask = STATUS_LU;
		
		while (((dReadReg825xx (STATUS) & dMask) == 0) && (wTimer > 0))
		{
			vDelay (1);
			--wTimer;
		}
#ifdef DEBUG
		sprintf (buffer, "LINKS: 0x%08X\n", dReadReg825xx (LINKS));
		vConsoleWrite (buffer);
#endif
	}
#ifdef DEBUG
	sprintf (buffer,"wTimer:%08X\n",wTimer);
	vConsoleWrite (buffer);
#endif

	if (wTimer == 0)
	{
#ifdef DEBUG
		sprintf (buffer,"Unable to establish link (CTRL=%x,STATUS=%x)\n",
				dReadReg825xx (CTRL), dReadReg825xx (STATUS));
		vConsoleWrite (buffer);
		vDisplayPhyRegs (psDev);
#endif
		if (psDev->bController != CTRLT_I350)
			vSetLoopbackMode (psDev, bType, LB_NONE, dSpeed);

		return (E__LOC_LB_LINK_FAIL);
	}

#ifdef DEBUG
	sprintf (buffer,"CTRL=%x, STATUS=%x, wTimer=%d\n",
				dReadReg825xx (CTRL),
				dReadReg825xx (STATUS),
				wTimer);

	vConsoleWrite (buffer);
#endif

	/* ... and send ...	*/
#ifdef DEBUG_VERBOSE
	vClear8257xStatistics (psDev);
#endif

#ifdef DEBUG
	vConsoleWrite ("\nSending\n");
#endif
	vEnableRxTx (psDev, EN_RXTX);
	vInitRxDescr (psDev, wDescrCount);
	vDelay(40);
	vInitTxDescr (psDev, wDescrCount);

	wTimer = 1000;

	do
	{
		vDelay (1);
		wCompleted = ((psDev->psRxDesc [0].bStatus & RDESC_STATUS_DD) != 0) &&
					 ((psDev->psRxDesc [1].bStatus & RDESC_STATUS_DD) != 0);
		--wTimer;
	} while (!wCompleted && (wTimer > 0));

#ifdef DEBUG
	sprintf (buffer, "wCompleted:%d wTimer:%d\n",wCompleted,wTimer);
	vConsoleWrite (buffer);
#endif

	vDisableTxRx (psDev);
	vDelay (10);
	
#ifdef DEBUG_VERBOSE
	sprintf (buffer, 
				"TxD0 status 0x%02x, TxD1 status 0x%02x,\n"
				"TxD0 length 0x%04x, TxD1 length 0x%04x\n",
				(int) psDev->psTxDesc[0].bStatus,
				(int) psDev->psTxDesc[1].bStatus,
				(int) psDev->psTxDesc[0].wLength,
				(int) psDev->psTxDesc[1].wLength);
	vConsoleWrite (buffer);
		
	sprintf (buffer,
				"RxD0 status 0x%02x, RxD1 status 0x%02x\n"
				"RxD0 errors 0x%02x, RxD1 errors 0x%02x\n"
				"RxD0 length 0x%04x, RxD1 length 0x%04x\n",
				(int) psDev->psRxDesc[0].bStatus,
				(int) psDev->psRxDesc[1].bStatus,
				(int) psDev->psRxDesc[0].bErrors,
				(int) psDev->psRxDesc[1].bErrors,
				(int) psDev->psRxDesc[0].wLength,
				(int) psDev->psRxDesc[1].wLength);
	vConsoleWrite (buffer);
#endif

	if (wTimer == 0)
	{
#ifdef DEBUG
		vConsoleWrite ("ERROR - Ethernet Data Transmission Error Timed Out\n");
#endif

		vDisplay8257xStats (psDev);
		vDisplayPhyRegs (psDev);

		vSetLoopbackMode (psDev, bType, LB_NONE, dSpeed);
		return (E__LOC_LB_TIMED_OUT);
	}

	for (i = 0; i < LOOP_BACK_SIZE; ++i)
	{
		if (psDev->pbTxBuff [i] != psDev->pbRxBuff [i])
		{
#ifdef DEBUG
			sprintf (buffer,
					"ERROR - Ethernet Data Transmission Error,"
					" expected %02Xh, got %02Xh\n",
					psDev->pbRxBuff [i],
					psDev->pbTxBuff [i]);

			vConsoleWrite (buffer);
#endif

			vSetLoopbackMode (psDev, bType, LB_NONE, dSpeed);
			return (E__LOC_LB_BAD_DATA);
		}
		
#ifdef DEBUG_VERBOSE
		// Show part of buffers		
		if ( i < 32 )
		{
			sprintf (buffer, "pbRxBuff[%02u]: 0x%02x pbTxBuff [%02u]: 0x%02x\n",
			         i, psDev->pbRxBuff [i], i, psDev->pbTxBuff [i]);
			vConsoleWrite(buffer);
		}
#endif
	}

#ifdef DEBUG_VERBOSE
	vDisplay8257xStats(psDev);
#endif
	vSetLoopbackMode (psDev, bType, LB_NONE, dSpeed);
	return (E__OK);
}

/*******************************************************************
 * dInternalLoopback:
 *
 * WARNING: Internal loopback does not function in 10Mb/s mode as there
 *          are bugs in the 82543GC silicon.
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dInternalLoopback
(
	DEVICE*	psDev,	/* pointer to device data structure			*/
	UINT8	bType	/* interface mode: IF_COPPER or IF_FIBRE	*/
)
{
	UINT32	dMask;
	UINT32	dTemp = 0;
	UINT32	i = 0;
	UINT16	wCompleted = 0;
	UINT16	wDescrCount = 0;
	UINT16	wTimer = 0;
	UINT32	dStatus = 0;
	UINT8	abEthernetAddress [6];
	UINT8	bSpeed;
//	UINT16  wPhyReg = 0;
//	UINT32 kmrnctrlsta = 0;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if (psDev->bType == IF_SERDES)
	{
		if (psDev->bController != CTRLT_I350)
		{
			/* switch to copper and use internal phy */
			dTemp = dReadReg825xx (CTRL_EXT) & M__CTRLEXT_LINK;
			vWriteReg825xx (CTRL_EXT, dTemp | CTRLEXT_LINK_COPPER);
		}
	}

	/* Reset the Ethernet Controller */
	dStatus = dCtrlReset (psDev);

	if (dStatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("ERROR - Timed out whilst resetting the controller\n");
#endif

		return E__CTRL_RESET_TIMEOUT;
	}

	/* Get this device's ethernet address */
	dGetLanAddr (psDev, abEthernetAddress);

	bSpeed = SPEED_1G;
	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		bSpeed = SPEED_10G;					// can only do local loopback at 10G if no link
	}

	if (psDev->bController != CTRLT_82599_SFP)
	{
		PhySpeed (psDev, bSpeed);
		vDelay (10);
		dStatus = PhyReset (psDev);
	}

	if (dStatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("ERROR - Timed out whilst resetting the PHY\n");
#endif

		return E__PHYRESET_TIMEOUT;
	}

	vInit825xx (psDev, SPEED_1G); /* speed ignored for x540 */

	/* Initialize the device and then create the descriptor rings ...	*/
	wDescrCount = wCreateDescriptorRings (psDev, abEthernetAddress,
											abEthernetAddress, LOOP_BACK_SIZE, 1);

	vInitRxDescr (psDev, 0);
	vDelay(40);
	vInitTxDescr (psDev, 0);
	
#ifdef DEBUG
	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		sprintf (buffer, "LINKS: 0x%08X\n", dReadReg825xx (LINKS));
		vConsoleWrite (buffer);
	}
#endif
	

	/* enter loopback mode */
	vSetLoopbackMode (psDev, bType, LB_INTERNAL, SPEED_1G);		// for the x540, speed is already set to 10GB so this parameter is ignored)
	vDelay (10);

	if ((psDev->bController != CTRLT_X540) &&  (psDev->bController != CTRLT_82599_SFP) )
	{
		/* Enter full duplex mode */
		dTemp = dReadReg825xx (CTRL);
		vWriteReg825xx (CTRL, dTemp | CTRL_FD);
		vDelay (1);

#ifdef DEBUG
		sprintf (buffer, "PHY Control register after setting LB=%x\n", ReadPhyRegister (psDev, 0));
		vConsoleWrite (buffer);
#endif

		/* Reset the LINK (1 -> 0) ... causing autonegotiation ... */
		dTemp = dReadReg825xx (CTRL);
		vWriteReg825xx (CTRL, dTemp | CTRL_LRST);
		vDelay (10);
		dTemp = dReadReg825xx (CTRL);
		vWriteReg825xx (CTRL, (dTemp & (~CTRL_LRST)) | CTRL_SLU | CTRL_FRCSPD | CTRL_FRCDPLX);
		vDelay (100);
	}


	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		/* Check link up using the link status register, as using */
		/* the general status register is only useful in IOV mode */
		dMask = LINKS_UP;
		
		wTimer = 1000;
		while (((dReadReg825xx (LINKS) & dMask) == 0) && (wTimer > 0))
		{
			vDelay (1);
			--wTimer;
		}
		
#ifdef DEBUG
		sprintf (buffer, "LINKS: 0x%08X\n", dReadReg825xx (LINKS));
		vConsoleWrite (buffer);
#endif		
	}
	else
	{
		dMask = STATUS_LU;
		
		wTimer = 1000;
		while (((dReadReg825xx (STATUS) & dMask) == 0) && (wTimer > 0))
		{
			vDelay (1);
			--wTimer;
		}
	}
	
/*
#ifdef DEBUG
	dReadPhyRegs(psDev);
#endif
*/

	if (wTimer == 0)
	{
#ifdef DEBUG
		sprintf (buffer, "STATUS register after setting LB = 0x%08X\n", dReadReg825xx (STATUS));
		vConsoleWrite (buffer);

		vConsoleWrite ("\nLink UP Timed Out\n");
#endif
		vSetLoopbackMode (psDev, bType, LB_NONE, SPEED_1G);		// restore normal operations
		return (E__INT_LB_TIMED_OUT);
	}
	else
	{
#ifdef DEBUG_VERBOSE
		vClear8257xStatistics (psDev);
#endif
#ifdef DEBUG
		vConsoleWrite ("\nSending...\n");
#endif
	}

	/* ... and send ...	 */

	vEnableRxTx (psDev, EN_RXTX);
	vInitRxDescr (psDev, wDescrCount);
	vDelay(40);
	vInitTxDescr (psDev, wDescrCount);

	wTimer = 1000;

	do
	{
		vDelay (1);
		wCompleted = ((psDev->psRxDesc [0].bStatus & RDESC_STATUS_DD) != 0) &&
					 ((psDev->psRxDesc [1].bStatus & RDESC_STATUS_DD) != 0);

		--wTimer;
	} while (!wCompleted && (wTimer > 0));

	vDisableTxRx (psDev);
	vDelay (10);
	
#ifdef DEBUG_VERBOSE
	sprintf (buffer, 
				"TxD0 status 0x%02x, TxD1 status 0x%02x,\n"
				"TxD0 length 0x%04x, TxD1 length 0x%04x\n",
				(int) psDev->psTxDesc[0].bStatus,
				(int) psDev->psTxDesc[1].bStatus,
				(int) psDev->psTxDesc[0].wLength,
				(int) psDev->psTxDesc[1].wLength);
	vConsoleWrite (buffer);
		
	sprintf (buffer,
				"RxD0 status 0x%02x, RxD1 status 0x%02x\n"
				"RxD0 errors 0x%02x, RxD1 errors 0x%02x\n"
				"RxD0 length 0x%04x, RxD1 length 0x%04x\n",
				(int) psDev->psRxDesc[0].bStatus,
				(int) psDev->psRxDesc[1].bStatus,
				(int) psDev->psRxDesc[0].bErrors,
				(int) psDev->psRxDesc[1].bErrors,
				(int) psDev->psRxDesc[0].wLength,
				(int) psDev->psRxDesc[1].wLength);
	vConsoleWrite (buffer);
#endif

	if (wTimer == 0)
	{
#ifdef DEBUG
		vConsoleWrite ("ERROR - Ethernet Data Transmission Error Timed Out\n");
#endif
		vSetLoopbackMode (psDev, bType, LB_NONE, SPEED_1G);		// restore normal operations
		return (E__INT_LB_TIMED_OUT);
	}

	for (i = 0; i < LOOP_BACK_SIZE; ++i)
	{
		if (psDev->pbTxBuff [i] != psDev->pbRxBuff [i])
		{
#ifdef DEBUG
			sprintf (buffer,
			        "ERROR - Ethernet Data Transmission Error,"
			        " expected %02Xh, got %02Xh\n",
			         psDev->pbRxBuff [i], psDev->pbTxBuff [i]);
			vConsoleWrite(buffer);
#endif

			vSetLoopbackMode (psDev, bType, LB_NONE, SPEED_1G);		// restore normal operations
			return (E__INT_LB_BAD_DATA);
		}

#ifdef DEBUG_VERBOSE
		// Show part of buffers		
		if ( i < 32 )
		{
			sprintf (buffer, "pbRxBuff[%02u]: 0x%02x pbTxBuff [%02u]: 0x%02x\n",
			         i, psDev->pbRxBuff [i], i, psDev->pbTxBuff [i]);
			vConsoleWrite(buffer);
		}
#endif
	}

#ifdef DEBUG_VERBOSE
	vDisplay8257xStats(psDev);
#endif

	vSetLoopbackMode (psDev, bType, LB_NONE, SPEED_1G);		// restore normal operations
	return (E__OK);
}

/*******************************************************************
 * dCheckTestData
 *
 * RETURNS: E_OK if transmitted and received data match otherwise
 *			E_LOC_LB_BAD_DATA
 */
UINT32 dCheckTestData (DEVICE *psTxDev, DEVICE *psRxDev, UINT16 wDataSize,
				UINT16 wNumFrames)
{
	UINT8	*pbRxBuff, *pbTxBuff;
	UINT16	wFrameCount;
	UINT16	wBufsPerFrame;
	int		i;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	wBufsPerFrame= (wDataSize + ETH_OH + (TX_BUFF_SIZE - 1)) / TX_BUFF_SIZE;
	pbRxBuff = psRxDev->pbRxBuff;

	/* start testing from the 1st received frame */
	pbTxBuff = psTxDev->pbTxBuff;

	pbTxBuff += ((UINT16)pbRxBuff[14] * TX_BUFF_SIZE * wBufsPerFrame);

	for (wFrameCount = 0; wFrameCount < wNumFrames; ++wFrameCount)
	{

		for (i = 0; i < wDataSize; ++i)
		{
			if (pbTxBuff [i] != pbRxBuff [i])
			{
				sprintf (psTxDev->achErrorMsg,
						"ERROR - Ethernet Data Transmission Error,"
						" frame %d, expected %02Xh, got %02Xh at offset %xh\n",
						wFrameCount, pbTxBuff [i], pbRxBuff [i], i);

				return (E__LOC_LB_BAD_DATA);
			}
		}

		pbTxBuff += (wBufsPerFrame * TX_BUFF_SIZE);
		pbRxBuff += (wBufsPerFrame * RX_BUFF_SIZE);
	}
	return E__OK;
}

/*******************************************************************
 * wSendDescr
 *
 * RETURNS: descriptor index before adding new descriptors
 */
static UINT16 wSendDescr
(
	DEVICE*	psDev,
	UINT16	dDescrCount	/* number of descriptors to TX */
)
{
	UINT32	dTail;
	UINT16	wRet;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		dTail = dReadReg825xx (TDT_X540(0));
		wRet = (UINT16) dTail;
		dTail += dDescrCount;
		vWriteReg825xx (TDT_X540(0), dTail);
	}
	else
	{
		dTail = dReadReg825xx (TDT);
		wRet = (UINT16) dTail;
		dTail += dDescrCount;
		vWriteReg825xx (TDT, dTail);
	}

#ifdef DEBUG
		sprintf (buffer, "Tx Descriptor address = 0x%08X\n", psDev->dTxDescAddr);
		vConsoleWrite (buffer);
		sprintf (buffer, "Tx Descriptor size = 0x%08X\n", psDev->dTxDescSize);
		vConsoleWrite (buffer);
		sprintf (buffer, "TDT = 0x%08X\n", wRet);
		vConsoleWrite (buffer);
		sprintf (buffer, "dDescrCount = %d\n", dDescrCount);
		vConsoleWrite (buffer);
		sprintf (buffer, "New TDT = 0x%08X\n", dTail);
		vConsoleWrite (buffer);
#endif

	return wRet;
}

/*******************************************************************
 * bTestTxDescr
 *
 * RETURNS: 0 if all descriptors are done
 */
static UINT8 bTestTxDescr
(
	DEVICE*	psDev,
	UINT16	wDescrCount	/* number of descriptors to TX */
)
{
	UINT16	wCurrentDescr = psDev->wTxCurrentDescr;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	while (wDescrCount > 0)
	{
#ifdef DEBUG
		sprintf (buffer, "psDev->psTxDesc [%d].bStatus %04X\n", wCurrentDescr, psDev->psTxDesc [wCurrentDescr].bStatus);
		vConsoleWrite (buffer);
#endif

		if ((psDev->psTxDesc [wCurrentDescr++].bStatus & TDESC_STATUS_DD) != 0)
		{
			--wDescrCount;
		}
		else
		{
			break;
		}
	}

	return wDescrCount == 0 ? 0 : 1;
}

/*******************************************************************
 * bTestRxDescr
 *
 * RETURNS: 0 if all descriptors are done
 */
static UINT8 bTestRxDescr
(
	DEVICE*	psDev,
	UINT16	wCurrentDescr,
	UINT16	wDescrCount	/* number of descriptors to RX */
)
{
#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	while (wDescrCount > 0)
	{
#ifdef DEBUG
		sprintf (buffer, "psDev->psRxDesc [%d].bStatus %04X\n", wCurrentDescr, psDev->psRxDesc [wCurrentDescr].bStatus);
		vConsoleWrite (buffer);
		sprintf (buffer, "psDev->psRxDesc [%d].bErrors %04X\n", wCurrentDescr, psDev->psRxDesc [wCurrentDescr].bErrors);
		vConsoleWrite (buffer);
#endif

		if ((psDev->psRxDesc [wCurrentDescr++].bStatus & RDESC_STATUS_DD) != 0)
		{
			--wDescrCount;
		}
		else
		{
			break;
		}
	}

	return wDescrCount == 0 ? 0 : 1;
}

#ifdef DEBUG
/*******************************************************************
 * wCountDescr
 *
 * RETURNS: 0 if all descriptors are done
 */

static UINT16 wCountDescr
(
	TDESC	*psDescr
)
{
	UINT16	wCount = 0;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	while ((psDescr->bStatus & TDESC_STATUS_DD) != 0)
	{
		++psDescr;
		++wCount;
	}

	return wCount;
}
#endif

/*******************************************************************
 * wPMCLoopback:
 *
 * RETURNS: E__OK or an E__... BIST error code
 */
static UINT32 dPMCLoopback
(
	DEVICE*	psTxDev,	/* pointer to transmit device data structure */
	DEVICE*	psRxDev,	/* pointer to receive device data structure */
	UINT8	bType,		/* Interface Type: IF_COPPER or IF_FIBRE	*/
	UINT16	wFrameCount,/* number of frames to send for testing */
	UINT8	bTestSpeed  /* speed for test. 0 = all, 1 = 10MB, 2 = 100MB 3 = 1GB, 4 = 10GB*/
)
{
	UINT32	dTemp;
	UINT8	bSpeed;
	UINT8	bMaxSpeed;
	UINT8	bMinSpeed;
	UINT16	wCompleted;
	UINT16	wRxDescrCount;
	UINT16	wTxDescrCount;
	UINT16	wDescrUsed;
	UINT16	wDescrPerFrame;
	UINT32	dStatus;
	UINT16	wResetTimer;
	UINT16	wTemp;
//	UINT16  reg_data;
	UINT16	wFramesReceived;
	UINT8	abTxEthAddress [6];
	UINT8	abRxEthAddress [6];
	UINT8*	pEthAddr;
	DEVICE*	psDev;
	int		iDev;
	char	achBuffer[80];
	UINT16	wRetry;
//	UINT8	link = 0;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* Check the Media Type: As Fibre Optic interfaces operate at 1000Mb/s
	 * then loopback can not be selected as the 82543GC performs echo
	 * cancellation and the message is effectively lost. Fail with an
	 * "Invalid Media" error for fibre optic interfaces.
	 */

	if ((psTxDev->bController == CTRLT_X540))
	{
		bMinSpeed = SPEED_100M;
		bMaxSpeed = SPEED_10G;
	}
	else if 	(psTxDev->bController == CTRLT_82599_SFP)
	{
		bMinSpeed = SPEED_10G;
	   bMaxSpeed = SPEED_10G;
	}
	else
	{
		bMinSpeed = SPEED_10M;
		bMaxSpeed = SPEED_1G;
	}

	if (/*(bType == IF_FIBER) ||*/ (bType == IF_SERDES))
	{
		vConsoleWrite ("SERDES interface\n");
		bMinSpeed = SPEED_1G;
	}

	bMaxSpeed = psTxDev->bMaxSpeed;
	if (psRxDev->bMaxSpeed < bMaxSpeed)
	{
		bMaxSpeed = psRxDev->bMaxSpeed;
	}

	if (bTestSpeed != 0)
	{
		bMinSpeed = bTestSpeed;
		bMaxSpeed = bTestSpeed;
	}

	for (bSpeed = bMinSpeed; bSpeed <= bMaxSpeed; ++bSpeed)
	{
		/* Reset the Ethernet Controllers */

		for (iDev = 0; iDev < 2; ++iDev)
		{
			if (iDev == 0)
			{
				psDev = psTxDev;
			}
			else
			{
				psDev = psRxDev;
			}

			if ((psTxDev->bController == psRxDev->bController) &&
				(psDev->bController == CTRLT_I350) &&
				(psDev->bType == IF_SERDES))
			{
#ifdef DEBUG
				vConsoleWrite ("Controller reset not executed as this\n");
				vConsoleWrite ("may set i350 back to IF_COPPER\n");
#endif
				break;		// reset not required
			}

			dStatus = dReadReg825xx(HLREG1);
			dStatus = dCtrlReset (psDev);
			vClear8257xStatistics (psDev);

			if (dStatus != E__OK)
			{
				sprintf (psTxDev->achErrorMsg,
						"ERROR - Timed out whilst resetting controller %d",
						iDev);
				return E__RESET_TIMEOUT;
			}
#ifdef DEBUG
			vConsoleWrite ("Controller reset complete\n");
#endif
		} /* End of For loop */

		/* Get the device's ethernet addresses */
		/* Initialize the devices and create the descriptor rings ... */
#ifdef DEBUG
		vConsoleWrite ("Initializing devices and creating descriptor rings ...\n");
#endif

		for (iDev = 0; iDev < 2; ++iDev)
		{
			if (iDev == 0)
			{
				psDev = psTxDev;
				pEthAddr = abTxEthAddress;
			}
			else
			{
				psDev = psRxDev;
				pEthAddr = abRxEthAddress;
			}

			dGetLanAddr (psDev, pEthAddr);
		}

		for (iDev = 0; iDev < 2; ++iDev)
		{
			if (iDev == 0)
			{
				psDev = psTxDev;
			}
			else
			{
				psDev = psRxDev;
			}


			if (psDev->bType != IF_SERDES)
			{
#ifdef DEBUG
				vConsoleWrite ("Set Phy Speed ...\n");
#endif
				if (psDev->bController != CTRLT_82599_SFP)
				{
					PhySpeed (psDev, bSpeed);
					E1E_FLUSH();
				}
			}

			vInit825xx (psDev, bSpeed); /* speed ignored for x540 */
		 	E1E_FLUSH();

			if (psDev->bType == IF_SERDES)
			{
				/* make sure TBI mode is enabled */
				dTemp = dReadReg825xx (CTRL_EXT) & M__CTRLEXT_LINK;
				vWriteReg825xx (CTRL_EXT, dTemp | CTRLEXT_LINK_TBI);
			}

			/* Init RxDev and TxDev */
			if (iDev == 0) // TxDev
			{
				wDescrUsed = wCreateDescriptorRings (psDev,
														abTxEthAddress,
														abRxEthAddress,
														LOOP_BACK_SIZE,
														wFrameCount + 10);

				wDescrPerFrame = wDescrUsed / (wFrameCount + 10);
			
#ifdef DEBUG
				sprintf (buffer, "Tx Descriptors used = %d\n", wDescrUsed);
				vConsoleWrite (buffer);
				sprintf (buffer, "Tx Descriptors per frame = %d\n", wDescrPerFrame);
				vConsoleWrite (buffer);
#endif

				vInitTxDescr (psDev, 0);
			}
			else // RxDev
			{
				wDescrUsed = wCreateDescriptorRings (psDev,
														abTxEthAddress,
														abRxEthAddress,
														LOOP_BACK_SIZE,
														wFrameCount);
			
#ifdef DEBUG
				sprintf (buffer, "Rx Descriptors used = %d\n", wDescrUsed);
				vConsoleWrite (buffer);
#endif

				vInitRxDescr (psDev, 0);
			}

			if ((psTxDev->bController != CTRLT_X540) && (psTxDev->bController != CTRLT_82599_SFP))
			{
				/* Enter full duplex mode */
#ifdef DEBUG
				vConsoleWrite ("Entering full duplex mode ...\n");
#endif
				dTemp = dReadReg825xx (CTRL);
				vWriteReg825xx (CTRL, dTemp | CTRL_FD);
				vDelay (1);
			}
		}

		for (iDev = 0; iDev < 2; ++iDev)
		{
			if (iDev == 0)
			{
				psDev = psTxDev;
			}
			else
			{
				psDev = psRxDev;
			}
#ifdef DEBUG
			vConsoleWrite ("Attempting to establish the link ...\n");
#endif

			if ((psDev->bController == CTRLT_I350) && (psDev->bType == IF_SERDES))
			{
				vSerDesChannelLoopbackMode (psDev);
			}
			else
			{

				vSetLoopbackMode (psDev, psDev->bType, LB_CHANNEL, bSpeed);  //Hari 29/10/2014

				vDelay (100);
				if ((psTxDev->bController != CTRLT_X540) && (psTxDev->bController != CTRLT_82599_SFP))
				{
					dTemp = dReadReg825xx (CTRL);
					vWriteReg825xx (CTRL, dTemp | CTRL_LRST);
					vDelay (100);
	
					dTemp = dReadReg825xx (CTRL);
					dTemp = 0;
					vWriteReg825xx (CTRL, dTemp);
					vDelay (100);

					vWriteReg825xx (CTRL, dTemp | CTRL_SLU);
				}

				if (psDev->bType == IF_SERDES)
				{
					dTemp = dReadReg825xx (CTRL);
					vWriteReg825xx (CTRL, dTemp | CTRL_ILOS);
				}
			}
	
			vDelay (10);
		}

		sprintf (achBuffer, "Waiting for %s link\n", aachSpeed[bSpeed]);
		vConsoleWrite (achBuffer);

		for (iDev = 0; iDev < 2; ++iDev)
		{
			if (iDev == 0)
			{
				psDev = psTxDev;
			}
			else
			{
				psDev = psRxDev;
			}
		
			if ((psDev->bController == CTRLT_I350) && (psDev->bType == IF_SERDES))
			{
				wResetTimer = 4000;
				while (((dReadReg825xx (PCS_LSTAT) & PCS_LINK_OK) == 0) && (wResetTimer > 0))
				{
					vDelay (10);
					--wResetTimer;
				}
			}
			else if (psDev->bController == CTRLT_X540 || (psDev->bController == CTRLT_82599_SFP))
			{
				/* Check link up using the link status register, as using */
				/* the general status register is only useful in IOV mode */
				wResetTimer = 20000;

				dTemp = dReadReg825xx (LINKS);
				while (((dTemp & LINKS_UP) == 0) && (wResetTimer > 0))
				{
					vDelay (1);
					--wResetTimer;
					dTemp = dReadReg825xx (LINKS);
				}
				
#ifdef DEBUG
				sprintf (buffer, "wResetTimer = %d\n", wResetTimer);
				vConsoleWrite (buffer);
				sprintf (buffer, "Link speed = %s\n", aachSpeed[((dTemp >> 28) & 0x03) + 1]);
				vConsoleWrite (buffer);
#endif
			}
			else
			{
				wResetTimer = 5000;
				E1E_FLUSH();
			
				while (((dReadReg825xx (STATUS) & STATUS_LU) == 0) && (wResetTimer > 0))
				{
					vDelay (10);
					--wResetTimer;
				}
			}

			if (wResetTimer == 0)
			{
#ifdef DEBUG
				sprintf (psTxDev->achErrorMsg,
							"ERROR - Controller %d unable to establish the link",
							iDev);
				sprintf (buffer,
							"ERROR - Controller %d unable to establish the link",
							iDev);
				vConsoleWrite (buffer);
#endif
				return (E__LOC_LB_LINK_FAIL);
			}

			if ((psDev->bType != IF_SERDES) && (psDev->bController != CTRLT_X540) &&(psDev->bController != CTRLT_82599_SFP))
			{
				wResetTimer = 1000;
				while (((ReadPhyRegister (psDev,0x01) & 0x0004) == 0) && (wResetTimer > 0))
				{
					vDelay (1);
					--wResetTimer;
				}
			
				if (wResetTimer == 0)
				{
					sprintf (psTxDev->achErrorMsg,
							"ERROR 2 - Controller %d unable to establish the link",
							iDev);
#ifdef DEBUG
					sprintf (buffer,
							"ERROR 2 - Controller %d unable to establish the phy link",	iDev);
					vConsoleWrite (buffer);
#endif  
					return (E__LOC_LB_LINK_FAIL);
				}
			}
		}

		vDelay (2000);	/* wait for hub */

		/* ... and send ... */

		vInitRxDescr (psRxDev, wDescrUsed);
		vEnableRxTx (psRxDev, EN_RX);
		vDelay (200);	/* wait for hub */
		vEnableRxTx (psTxDev, EN_TX);

#ifdef DEBUG
		psDev = psRxDev;
		dTemp = dReadReg825xx(HLREG0);
		dTemp |= 0x08012FFF;				// enable Rx error length reporting
		vWriteReg825xx(HLREG0, dTemp);
		
		/*
		 * Hari :added
		 */
		psDev = psTxDev;
		dTemp = dReadReg825xx(HLREG0);
		dTemp |= 0x08012FFF;				// enable Tx error length reporting
		vWriteReg825xx(HLREG0, dTemp);

#ifdef DEBUG_VERBOSE
		for (wRetry = 0; wRetry < 0x10; ++wRetry)
		{
			wTemp = ReadPhyRegister (psDev, X540_PHY_REG(MDIO_MMD_PMAPMD, wRetry));
			sprintf (buffer, "PhyReg 1:%02X = 0x%04X\n", wRetry, wTemp);
			vConsoleWrite (buffer);
		}
		
		sprintf (buffer, "HLREG0 = 0x%08X\n", dReadReg825xx(HLREG0));
		vConsoleWrite (buffer);
		sprintf (buffer, "HLREG1 = 0x%08X\n", dReadReg825xx(HLREG1));
		vConsoleWrite (buffer);
		sprintf (buffer, "PAP    = 0x%08X\n", dReadReg825xx(PAP));
		vConsoleWrite (buffer);
		sprintf (buffer, "MSCA   = 0x%08X\n", dReadReg825xx(MSCA));
		vConsoleWrite (buffer);
		sprintf (buffer, "MSRWD  = 0x%08X\n", dReadReg825xx(MSRWD));
		vConsoleWrite (buffer);
		sprintf (buffer, "TREG   = 0x%08X\n", dReadReg825xx(TREG));
		vConsoleWrite (buffer);
		sprintf (buffer, "LINKS  = 0x%08X\n", dReadReg825xx(LINKS));
		vConsoleWrite (buffer);
		sprintf (buffer, "MAXFRS = 0x%08X\n", dReadReg825xx(MAXFRS));
		vConsoleWrite (buffer);
		sprintf (buffer, "MMNGC  = 0x%08X\n", dReadReg825xx(MMNGC));
		vConsoleWrite (buffer);
		sprintf (buffer, "MPVC   = 0x%08X\n", dReadReg825xx(MPVC));
		vConsoleWrite (buffer);
		sprintf (buffer, "MACC   = 0x%08X\n", dReadReg825xx(MACC));
		vConsoleWrite (buffer);
#endif
#endif

		wRetry = 10;
		do
		{
#ifdef DEBUG
			vConsoleWrite ("Probe\t");
#endif
	
			vInitTxDescr (psTxDev, 0);
			wTemp = wSendDescr(psTxDev, 1 * wDescrPerFrame);			/* send one frame */
			psTxDev->wTxCurrentDescr = wTemp;
			vDelay (2000);
			if (bTestTxDescr (psTxDev, 1 * wDescrPerFrame) == 0)
			{
				if (bTestRxDescr (psRxDev, 0, 1 * wDescrPerFrame) == 0)
				{
					break;
				}
			}
		} while (--wRetry > 0);

#ifdef DEBUG
		vConsoleWrite ("Probe\t");
#endif

		if (wRetry == 0)
		{
			/* complete fail */
			vDisableTxRx (psTxDev);
			vDisableTxRx (psRxDev);

#ifdef DEBUG
			sprintf (psTxDev->achErrorMsg, 
					"\nTransmitted 20 descriptors, received %d",
					wCountDescr ((TDESC*)psRxDev->psRxDesc));
			sprintf (buffer, 
					"Transmitted 20 descriptors, received %d",
					wCountDescr ((TDESC*)psRxDev->psRxDesc));
			vConsoleWrite (buffer);
#ifdef DEBUG_VERBOSE
			vConsoleWrite ("\nTx Channel Statistics\n");
			vDisplay8257xStats(psTxDev);
			vDisplayPhyRegs (psTxDev);
			psDev = psTxDev;
			sprintf (buffer, "\nHLREG0 = 0x%08X\n", dReadReg825xx(HLREG0));
			vConsoleWrite (buffer);
			sprintf (buffer, "HLREG1 = 0x%08X\n", dReadReg825xx(HLREG1));
			vConsoleWrite (buffer);
			sprintf (buffer, "PAP    = 0x%08X\n", dReadReg825xx(PAP));
			vConsoleWrite (buffer);
			sprintf (buffer, "MSCA   = 0x%08X\n", dReadReg825xx(MSCA));
			vConsoleWrite (buffer);
			sprintf (buffer, "MSRWD  = 0x%08X\n", dReadReg825xx(MSRWD));
			vConsoleWrite (buffer);
			sprintf (buffer, "TREG   = 0x%08X\n", dReadReg825xx(TREG));
			vConsoleWrite (buffer);
			sprintf (buffer, "LINKS  = 0x%08X\n", dReadReg825xx(LINKS));
			vConsoleWrite (buffer);
			sprintf (buffer, "MAXFRS = 0x%08X\n", dReadReg825xx(MAXFRS));
			vConsoleWrite (buffer);
			sprintf (buffer, "MMNGC  = 0x%08X\n", dReadReg825xx(MMNGC));
			vConsoleWrite (buffer);
			sprintf (buffer, "MPVC   = 0x%08X\n", dReadReg825xx(MPVC));
			vConsoleWrite (buffer);
			sprintf (buffer, "MACC   = 0x%08X\n", dReadReg825xx(MACC));
			vConsoleWrite (buffer);
			
			vConsoleWrite ("\nRx Channel Statistics\n");
			vDisplay8257xStats(psRxDev);
			psDev = psRxDev;
			vDisplayPhyRegs(psRxDev);
			
			sprintf (buffer, "\nHLREG0 = 0x%08X\n", dReadReg825xx(HLREG0));
			vConsoleWrite (buffer);
			sprintf (buffer, "HLREG1 = 0x%08X\n", dReadReg825xx(HLREG1));
			vConsoleWrite (buffer);
			sprintf (buffer, "PAP    = 0x%08X\n", dReadReg825xx(PAP));
			vConsoleWrite (buffer);
			sprintf (buffer, "MSCA   = 0x%08X\n", dReadReg825xx(MSCA));
			vConsoleWrite (buffer);
			sprintf (buffer, "MSRWD  = 0x%08X\n", dReadReg825xx(MSRWD));
			vConsoleWrite (buffer);
			sprintf (buffer, "TREG   = 0x%08X\n", dReadReg825xx(TREG));
			vConsoleWrite (buffer);
			sprintf (buffer, "LINKS  = 0x%08X\n", dReadReg825xx(LINKS));
			vConsoleWrite (buffer);
			sprintf (buffer, "MAXFRS = 0x%08X\n", dReadReg825xx(MAXFRS));
			vConsoleWrite (buffer);
			sprintf (buffer, "MMNGC  = 0x%08X\n", dReadReg825xx(MMNGC));
			vConsoleWrite (buffer);
			sprintf (buffer, "MPVC   = 0x%08X\n", dReadReg825xx(MPVC));
			vConsoleWrite (buffer);
			sprintf (buffer, "MACC   = 0x%08X\n", dReadReg825xx(MACC));
			vConsoleWrite (buffer);
#endif
#endif

			return E__LOC_LB_TIMED_OUT;
		}

		/* must have Tx'd and Rx'd something so do some more */
		vConsoleWrite ("Sending\n");
		wSendDescr (psTxDev, (wFrameCount-1) * wDescrPerFrame);		/* send the rest */
		vDelay (2000); /*1000*/
	
		/* check transmitted descriptors */
		psDev = psTxDev;
		wTxDescrCount = 0;
		while ((psDev->psTxDesc [psDev->wTxCurrentDescr + wTxDescrCount].bStatus & TDESC_STATUS_DD) != 0)
		{
			++wTxDescrCount;
		}

		/* check received descriptors */
		psDev = psRxDev;
		wFramesReceived = 0;
		wRxDescrCount = 0;
		wCompleted = 0;
		while ((psDev->psRxDesc [wRxDescrCount].bStatus & RDESC_STATUS_DD) != 0)
		{
			++wRxDescrCount;
			if ((psDev->psRxDesc [wRxDescrCount].bStatus & RDESC_STATUS_EOP) != 0)
			{
				++wFramesReceived;
	
				/* Allow 1 frame to be lost */
				if (wFramesReceived >= (wFrameCount - 1))
				{
					wCompleted = 1;
				}
			}
		}

		vDisableTxRx (psTxDev);
		vDisableTxRx (psRxDev);
		vConsoleWrite ("Done\n");
	
		vDelay (10);

		if (wCompleted == 0)
		{
#ifdef DEBUG
			sprintf (psTxDev->achErrorMsg, "ERROR - Ethernet Data Receive Error."
								" Timed Out, transmitted %d received %d descriptors\n"
								"Rx status[0]=%02x  Rx status[1]=%02x\n"
								"Tx status[0]=%02x  Tx status[1]=%02x\n",
								(int)wTxDescrCount, (int)wRxDescrCount,
								(int)psRxDev->psRxDesc [0].bStatus,
								(int)psRxDev->psRxDesc [1].bStatus,
								(int)psTxDev->psTxDesc [0].bStatus,
								(int)psTxDev->psTxDesc [1].bStatus);
			sprintf (buffer, "ERROR - Ethernet Data Receive Error."
								" Timed Out, transmitted %d received %d descriptors\n"
								"Rx status[0]=%02x  Rx status[1]=%02x\n"
								"Tx status[0]=%02x  Tx status[1]=%02x\n",
								(int)wTxDescrCount, (int)wRxDescrCount,
								(int)psRxDev->psRxDesc [0].bStatus,
								(int)psRxDev->psRxDesc [1].bStatus,
								(int)psTxDev->psTxDesc [0].bStatus,
								(int)psTxDev->psTxDesc [1].bStatus);
			vConsoleWrite (buffer);
			
			vDisplay8257xStats(psTxDev);
			vDisplay8257xStats(psRxDev);
#endif

			return (E__LOC_LB_TIMED_OUT);
		}
	
		dStatus = dCheckTestData (psTxDev, psRxDev, LOOP_BACK_SIZE, wFramesReceived);
		if (dStatus != E__OK)
		{
			return dStatus;
		}

		for (iDev = 0; iDev < 2; ++iDev)
		{
			if (iDev == 0)
			{
				psDev = psTxDev;
			}
			else
			{
				psDev = psRxDev;
			}

		}
	}
#ifdef DEBUG
	vConsoleWrite("Done PMC Loopback\n");
#endif

	return (E__OK);
}

/*******************************************************************
 * dInterruptTest: test all interrupt sources
 *
 * The interrupt force bit is used for each potential source. For
 * each, the reception of an interrupt signal at the CPU is checked
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dInterruptTest
(
	DEVICE*	psDev		/* pointer to device data structure */
)
{
	UINT32	dCount;
	UINT32	dIcr;
	UINT32	dMask;
	UINT32	dRegVal;
	
#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* Disable all the interrupts and clear the interrupt cause register */

	dRegVal = I8257x_VALID_INTERRUPTS | ICR_RXCFG | ICR_LSC;	/* also clear link change and rxcfg */
	if (psDev->bController == CTRLT_I350)
	{
		dRegVal &= ~ICR_TXQE;				// this bit must be set to zero on i350
	}
	vWriteReg825xx (IMC, dRegVal );

	(void)vDelay (100);

	dIcr = dReadReg825xx (ICR);	/* read to clear */

	/* Check that all bits of ICR were cleared */

	dIcr = dReadReg825xx (ICR);

	if ((dIcr & (~ICR_RXCFG)) != 0) /* RXCFG could occur because of the device reset */
	{
#ifdef DEBUG
		sprintf (buffer, "ICR is not zero after disable/clear (0x%08x)", dIcr);
		vConsoleWrite (buffer);
#endif

		return (E__ICR_NOT_ZERO);
	}

	/* Install interrupt handler and enable interrupts for this device */

	/* TODO new_interrupt_table	(psDev->bVector, (void far *) PCI_pass_handler,
										 (void far *) PCI_fail_handler, TRUE);*/

	/* Test each interrupt source: first check internal hardware with source
	 * masked, then check external interrupt generation.
	 */

	for (dCount = 0; dCount < 11; ++dCount)
	{
		dMask = 0x0001 << dCount;

		/* Disable (mask out) all interrupts */

		(void) vDelay (10);
		dRegVal = I8257x_VALID_INTERRUPTS;
		if (psDev->bController == CTRLT_I350)
		{
			dRegVal &= ~I350_RESERVED_INTERRUPTS;	// these bits must be set to zero on i350
		}

		vWriteReg825xx (IMC, dRegVal);
		(void) vDelay (10);

		if ((dMask & I8257x_VALID_INTERRUPTS) == 0)
		{
			continue;
		}

		/* Test interrupt sources with external interrupt disabled
		 */

		dIcr = dReadReg825xx (ICR);		/* clear ICR */

		/* Cause an interrupt by writing 1 to corresponding bit of ICS */

		if (psDev->bController == CTRLT_I350)
		{
			dMask &= ~I350_RESERVED_INTERRUPTS;		// these bits must be set to zero on i350
		}

		vWriteReg825xx (ICS, dMask);
		(void) vDelay (10);

		/* Read the ICR, check that only expected interrupt was generated */

		dIcr = dReadReg825xx (ICR);

		if ((dIcr & dMask) != dMask)	/* check expected only */
		{
#ifdef DEBUG
			/*vRestoreInterruptState (psDev);*/
			sprintf (buffer, "(bit-%d) Interrupt not generated"
			                             " (ICR=%08X)", dCount, dIcr);
			vConsoleWrite (buffer);
#endif

			return (E__NO_MASKINT);
		}

		if ((dIcr & (~(ICR_RXCFG | ICR_LSC))) != dMask)	/* check all except link status change and rxcfg*/
		{
#ifdef DEBUG
			/*vRestoreInterruptState (psDev);*/
			sprintf (buffer, "(bit-%d) Wrong interrupt generated"
			                             " (ICR=%08X)", dCount, dIcr);
			vConsoleWrite (buffer);
#endif
			return (E__WRONG_MASKINT);
		}
	}

	/* Disable all interrupts */

	dRegVal = I8257x_VALID_INTERRUPTS;
	if (psDev->bController == CTRLT_I350)
	{
		dRegVal &= ~I350_RESERVED_INTERRUPTS;		// these bits must be set to zero on i350
	}
	vWriteReg825xx (IMC, dRegVal);

	/*vRestoreInterruptState (psDev);*/
#ifdef DEBUG
	vConsoleWrite ("Interrupt OK\n");
#endif

	return (E__OK);
}


/*******************************************************************
 * dInterruptTestx540: test all interrupt sources of the x540
 *
 * The interrupt force bit is used for each potential source. For
 * each, the reception of an interrupt signal at the CPU is checked
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dInterruptTestx540
(
	DEVICE*	psDev		/* pointer to device data structure */
)
{
	UINT32	dCount;
	UINT32	dIcr;
	UINT32	dMask;
	UINT32	dRegVal;
	UINT32 result;
	
#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* Disable all the interrupts and clear the interrupt cause register */

	vWriteReg825xx (EIMC, 0x7FFFFFFFL);

	vDelay (100);

	dIcr = dReadReg825xx (EICR);	/* read to clear */

	/* Check that all bits of ICR were cleared */

	dIcr = dReadReg825xx (EICR);

	if (dIcr != 0)
	{
#ifdef DEBUG
		sprintf (buffer, "ICR is not zero after disable/clear (0x%08x)", dIcr);
		vConsoleWrite (buffer);
#endif

		return (E__ICR_NOT_ZERO);
	}

	/* Install interrupt handler and enable interrupts for this device */

	/* TODO new_interrupt_table	(psDev->bVector, (void far *) PCI_pass_handler,
										 (void far *) PCI_fail_handler, TRUE);*/

	vDelay (10);
	
	/* Enable interrupts */
	dRegVal = x540_TESTED_INTERRUPTS;
	vWriteReg825xx (EIMC, dRegVal);
#ifdef DEBUG_VERBOSE
	sprintf (buffer, "EIMC: 0x%08x\n", dRegVal);
	vConsoleWrite (buffer);
#endif

	/* Test each interrupt source: first check internal hardware with source
	 * masked, then check external interrupt generation.
	 */

	for (dCount = 0; dCount < 32; ++dCount)
	{
		vDelay (10);
		
		dMask = 0x0001 << dCount;

		if ((dMask & x540_TESTED_INTERRUPTS) == 0)
		{
			continue;
		}

		/* Test interrupt sources with external interrupt disabled
		 */

		dIcr = dReadReg825xx (EICR);		/* clear ICR */

		/* Cause an interrupt by writing 1 to corresponding bit of ICS */

		vWriteReg825xx (EICS, dMask);
		vDelay (10);

		/* Read the ICR, check that only expected interrupt was generated */

		dIcr = dReadReg825xx (EICR);
	
#ifdef DEBUG_VERBOSE
		sprintf (buffer, "EICR: 0x%08x bit-%d\n", dIcr, dCount);
		vConsoleWrite (buffer);
#endif

		if ((dIcr & dMask) != dMask)	/* check expected only */
		{
#ifdef DEBUG
			sprintf (buffer, "(bit-%d) Interrupt not generated (EICR=%08X)\n",
			            dCount, dIcr);
			vConsoleWrite (buffer);
#endif

			result = (E__NO_MASKINT);
			break;
		}
		else if (dIcr != dMask)	/* check no others */
		{
#ifdef DEBUG
			sprintf (buffer, "(bit-%d) Wrong interrupt generated (EICR=%08X)\n",
			            dCount, dIcr);
			vConsoleWrite (buffer);
#endif
			result = (E__WRONG_MASKINT);
			break;
		}
		else
		{
			result = (E__OK);
		}
	}

	/* Disable all interrupts */
	vWriteReg825xx (EIMC, 0x7FFFFFFFL);

#ifdef DEBUG
	if (result == E__OK)
	{
		vConsoleWrite ("Interrupt OK\n");
	}
#endif

	return result;
}

/*******************************************************************
 * dRegisterTest: Test the register access by rolling ones and zeros
 * pattern writing and read back
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dRegisterTest
(
	DEVICE*	psDev		/* pointer to device data structure */
)
{
	UINT32	dRdbahOld;
	UINT32	dTestPattern;
	UINT32	dTestRegister;
	UINT32	i;
	UINT32	dTestStatus;
	UINT16	rdbahRegister;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	dTestStatus = dCtrlReset (psDev);
	if (dTestStatus != E__OK)
	{
#ifdef DEBUG
			sprintf(buffer,
			"ERROR - Timed out whilst resetting the controller: %d\n", psDev->bIndex);
			vConsoleWrite(buffer);
#endif

		dTestStatus = E__CTRL_RESET_TIMEOUT;
		return dTestStatus;
	}
	
	if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		rdbahRegister = RDBAH_X540(0);				// X540 has 128 RDBAH registers so use RDBAH[0]
	}
	else if ((psDev->bController == CTRLT_I350) ||
			(psDev->bController == CTRLT_I210))
	{
		rdbahRegister = RDBAH;						// I210 and I350 have a single RDBAH register
	}
	
	dRdbahOld = dReadReg825xx (rdbahRegister);


	/* Perform a rolling '1' test on the RDBAH register */

	dTestPattern = 0x00000001;
	for (i = 0; (i < 32) && (dTestStatus == E__OK); ++i)
	{
		vWriteReg825xx (rdbahRegister, dTestPattern);
		dTestRegister = dReadReg825xx (rdbahRegister);
		if (dTestRegister != dTestPattern)
		{
#ifdef DEBUG
			sprintf (buffer,
			        "ERROR - Failed rolling 1 test: put %08Xh, got %08Xh",
			         dTestPattern, dTestRegister);
			vConsoleWrite(buffer);
#endif

			dTestStatus = E__FAIL_ROLLING_1;
		}
		dTestPattern <<= 1; /* next bit ... */
	}

	/* Perform a rolling '0' test on the RDBAH register */

	dTestPattern = 0x00000001;
	for (i = 0; (i < 32) && (dTestStatus == E__OK); ++i)
	{
		vWriteReg825xx (rdbahRegister, ~dTestPattern);
		dTestRegister = dReadReg825xx (rdbahRegister);
		if (dTestRegister != ~dTestPattern)
		{
#ifdef DEBUG
			sprintf (buffer,
			        "ERROR - Failed rolling 0 test: put %08Xh, got %08Xh",
			         ~dTestPattern, dTestRegister);
			vConsoleWrite(buffer);
#endif

			dTestStatus = E__FAIL_ROLLING_0;
		}
		dTestPattern <<= 1; /* next bit ... */
	}

	vWriteReg825xx (rdbahRegister, dRdbahOld);

	return dTestStatus;
}

/*******************************************************************
 * dInitPciDevice: Initialise the Ethernet device, allocate Memory
 * BARs, Tx/ RX descriptors, buffers
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
static UINT32 dInitPciDevice
(
	DEVICE*	psDev,		/* pointer to device data structure */
	UINT8	bIndex		/* controller index */
)
{
	PCI_PFA	pfa;
	UINT32	dTemp;
	UINT32	dDidVid;
	UINT16	wTemp;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	psDev->bIndex = bIndex;

	/* determine type of ethernet controller */
	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	dDidVid = dPciReadReg (pfa, PCI_VID, PCI_DWORD);

	switch (dDidVid)
	{
		case DIDVID_i350AMx_C:
		case DIDVID_i350AMx_S:
			psDev->bController = CTRLT_I350;
			psDev->bPhyAddress = 0;		/* internal PHY */
			strcpy (psDev->achCtrlName, "i350");
			psDev->bMaxSpeed = SPEED_1G;
			break;

		case DIDVID_i210_C:
		case DIDVID_i210_S:
			psDev->bController = CTRLT_I210;
			psDev->bPhyAddress = 0;		/* internal PHY */
			strcpy (psDev->achCtrlName, "i210");
			psDev->bMaxSpeed = SPEED_1G;
			break;

		case DIDVID_x540:
			psDev->bController = CTRLT_X540;
			psDev->bPhyAddress = 0;		/* internal PHY */
			strcpy (psDev->achCtrlName, "x540");
			psDev->bMaxSpeed = SPEED_10G;
			break;

		case DIDVID_82599_SFP:
			psDev->bController = CTRLT_82599_SFP;
			psDev->bPhyAddress = 0;		/* internal PHY */
			strcpy (psDev->achCtrlName, "82599_SFP");
			psDev->bMaxSpeed = SPEED_10G;
			break;

		default:
			psDev->bController = CTRLT_UNKNOWN;
			strcpy (psDev->achCtrlName, "Unknown");
			break;
	}

#ifdef DEBUG
	 sprintf (buffer, "\nController Name: %s type:%d \n", psDev->achCtrlName, psDev->bType);
	 vConsoleWrite (buffer);
#endif

	/* Enable memory access and bus-mastership, leave EEPROM to individual sub-BISTs */

	wTemp = (UINT16) dPciReadReg (pfa, PCI_COMMAND, PCI_WORD);
	vPciWriteReg (pfa, PCI_COMMAND, PCI_WORD, (wTemp | PCI_MEMEN | PCI_BMEN) & ~(1 << 4));

#ifdef DEBUG
	sprintf (buffer, "PCI_COMMAND: 0x%x \n", dPciReadReg (pfa, PCI_COMMAND, PCI_WORD));
	vConsoleWrite (buffer);
#endif

	/* Set Cache Line Size register (nominal value is 32) */

	vPciWriteReg (pfa, PCI_CACHE_LINE_SIZE, PCI_BYTE, 32);

	/* Create a pointer to the memory-mapped registers */

	dTemp = dPciReadReg (pfa, PCI_BASE_ADDR_1, PCI_DWORD);
	dTemp &= PCI_MEM_BASE_MASK;

#ifdef DEBUG
	sprintf (buffer, "Mem BAR 0: 0x%x \n", dTemp);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController == CTRLT_I350)
	{
		dHandle1 = dGetPhysPtr (dTemp, I350_REG_SIZE, &p1, (void*) &psDev->pbReg);
	}
	else if (psDev->bController == CTRLT_X540 || psDev->bController == CTRLT_82599_SFP)
	{
		dHandle1 = dGetPhysPtr (dTemp, X540_REG_SIZE, &p1, (void*) &psDev->pbReg);
	}
	else
	{
		dHandle1 = dGetPhysPtr (dTemp, I8257x_REG_SIZE, &p1, (void*) &psDev->pbReg);
	}

	if (dHandle1 == E__FAIL)
	{
#ifdef DEBUG
		vConsoleWrite ("Unable to allocate BAR Mem \n");
#endif

		return (E__TEST_NO_MEM);
	}

#ifdef DEBUG
	sprintf (buffer, "Mem BAR 0 val: 0x%x 0x%x\n", (UINT32)(psDev->pbReg), *(psDev->pbReg));
	vConsoleWrite (buffer);
#endif

	/* Create a pointer to the Flash EPROM */
	dTemp = dPciReadReg (pfa, PCI_ROM_BASE, PCI_DWORD);
	dTemp &= PCI_ROM_BASE_MASK;
	dHandle2 = 0;
	if (dTemp != 0)
	{
		dHandle2 = dGetPhysPtr (dTemp, 0x00080000, &p2, (void*) &psDev->pbRomRead);
		if (dHandle2 == E__FAIL)
		{
#ifdef DEBUG
			vConsoleWrite ("Unable to allocate PCI ROM \n");
#endif

			return (E__TEST_NO_MEM);
		}
	}

/*
	dHandle3 = 0;
	dTemp = dPciReadReg (pfa, PCI_BASE_ADDR_2, PCI_DWORD);
	dTemp &= PCI_ROM_BASE_MASK;
	if (dTemp != 0)
	{
		dHandle3 = dGetPhysPtr (dTemp, 0x00080000, &p3, (void*) &psDev->pbRomWrite);
		if (dHandle3 == E__FAIL)
		{
#ifdef DEBUG
			vConsoleWrite ("Unable to allocate MEM BAR2 \n");
#endif

			return (E__TEST_NO_MEM);
		}
	}
*/

	psDev->bVector = (UINT8) dPciReadReg (pfa, PCI_INT_LINE, PCI_BYTE);

	/* -TODO: psDev->bVector = irq_to_vector (psDev->bVector); */
	/* Get pointers for the device's data structures          */

	psDev->dTxDescSize = TX_NUM_DESC * 16; /* 8 = quantity, 16 = size */
	psDev->dTxDescAddr = TX_DESC_BASE + (bIndex * (TX_DESC_LENGTH / MAX_CONTROLLERS));
	dHandle4 = dGetPhysPtr (psDev->dTxDescAddr, TX_DESC_LENGTH, &p4, (void*) &psDev->psTxDesc);
	if (dHandle4 == E__FAIL)
	{
#ifdef DEBUG
		vConsoleWrite("Unable to allocate TX Desc\n");
#endif

		return (E__TEST_NO_MEM);
	}

	psDev->dTxBuffAddr = TX_BUFF_BASE + (bIndex * (TX_BUFF_LENGTH / MAX_CONTROLLERS));
	dHandle5 = dGetPhysPtr (psDev->dTxBuffAddr, TX_BUFF_LENGTH, &p5, (void*) &psDev->pbTxBuff);
	if (dHandle5 == E__FAIL)
	{
#ifdef DEBUG
		vConsoleWrite ("Unable to allocate TX Buf \n");
#endif

		return (E__TEST_NO_MEM);
	}

	memset ((void* )psDev->pbTxBuff, 0xCC, TX_BUFF_LENGTH);
	psDev->dRxDescSize = RX_NUM_DESC * 16; /* 8 = quantity, 16 = size */
	psDev->dRxDescAddr = RX_DESC_BASE + (bIndex * (RX_DESC_LENGTH / MAX_CONTROLLERS));
	dHandle6 = dGetPhysPtr (psDev->dRxDescAddr, RX_DESC_LENGTH, &p6, (void*) &psDev->psRxDesc);
	if (dHandle6 == E__FAIL)
	{
#ifdef DEBUG
		vConsoleWrite ("Unable to allocate Rx Desc \n");
#endif
		return (E__TEST_NO_MEM);
	}

	psDev->dRxBuffAddr = RX_BUFF_BASE + (bIndex * (RX_BUFF_LENGTH / MAX_CONTROLLERS));
	dHandle7 = dGetPhysPtr (psDev->dRxBuffAddr, RX_BUFF_LENGTH, &p7, (void*) &psDev->pbRxBuff);
	if (dHandle7 == E__FAIL)
	{
#ifdef DEBUG
		vConsoleWrite ("Unable to allocate Rx Buff \n");
#endif

		return (E__TEST_NO_MEM);
	}
	memset ((void*) psDev->pbRxBuff, 0xFF, RX_BUFF_LENGTH);

	return (E__OK);
}

/*******************************************************************
 * initX540CommonRegisters: Initialise function 0 regsitser sof x540 that
 *                    are shared across all functions
 *
 * RETURNS: none
 */
static void initX540CommonRegisters(void)
{

	UINT32	dDidVid;
	UINT16	wTemp;
	PCI_PFA	pfa;
	UINT16	bus;
	UINT8	func;

	// device will never be on the primary bus, so start with bus 1
	for (bus = 1; bus < 0x100; ++bus)
	{
		for (func = 0; func < 32; ++func)
		{
			pfa = PCI_MAKE_PFA(bus, func, 0);
			dDidVid = dPciReadReg (pfa, PCI_VID, PCI_DWORD);
			if (dDidVid == DIDVID_x540)
			{
				/* Enable memory access and bus-mastership */
				wTemp = (UINT16) dPciReadReg (pfa, PCI_COMMAND, PCI_WORD);
				vPciWriteReg (pfa, PCI_COMMAND, PCI_WORD, (wTemp | PCI_MEMEN | PCI_BMEN) & ~(1 << 4));
	
				// the following register bits are set when the BIOS runs to completion
				// this register bit is only in function 0 but applies to both functions
				wTemp = dPciReadReg (pfa, X540_CAPABILITY_DEVICE_CONTROL, PCI_WORD);
				vPciWriteReg (pfa, X540_CAPABILITY_DEVICE_CONTROL, PCI_WORD, (wTemp | X540_TLP_PAYLOAD_SIZE));

				/* Disable memory access and bus-mastership for the time being */
				wTemp = (UINT16) dPciReadReg (pfa, PCI_COMMAND, PCI_WORD);
				vPciWriteReg (pfa, PCI_COMMAND, PCI_WORD, wTemp & ~(PCI_IOEN | PCI_MEMEN | PCI_BMEN));

				//found an x540 so stop looking on this bus
				func = 32;
			}
		}
	}
	
}

/*******************************************************************
 * dFind825xx: Find the occurance of Ethernet device instances
 *
 * RETURNS: E__OK or an E__... BIT error code
 */
UINT32 dFind825xx(DEVICE* psDev, UINT8 bInstance,UINT8 bMode)
{
	PCI_PFA	pfa;
    UINT32	dDidVid;			/* device and vendor IDs */
	UINT8	bCount;				/* running count of devices */
	UINT8	bTemp;
	UINT8	bBus = 0;
	UINT8	bDev = 0;
	UINT8	bFunc = 0;
	UINT8	bMultiFunc = 0;		/* non-zero indicates multi-function device */
	UINT8	bHostBridges = 0;
	UINT8	bScanToBus = 0;

	SkipEthIf_PARAMS	params;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	memset (psDev, 0, sizeof (DEVICE));	/* clear all structure */
	psDev->bType = IF_COPPER;			/* assume card will have a copper interface */
	bCount = 0;

#ifdef DEBUG
	sprintf(buffer, "Instance wanted: %d \n", bInstance);
	vConsoleWrite(buffer);
#endif

	do 	// while (bBus <= bScanToBus)
	{
#ifdef DEBUG
//		sprintf(buffer, "bBus:%d bDev:%d bFunc:%d bScanToBus: %u\n",bBus,bDev,bFunc, bScanToBus);
//		vConsoleWrite(buffer);
#endif

		pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
		dDidVid = dPciReadReg (pfa, PCI_VID, PCI_DWORD);

		if ((dDidVid == DIDVID_i350AMx_C) ||
			(dDidVid == DIDVID_i350AMx_S) ||
			(dDidVid == DIDVID_i210_C)    ||
			(dDidVid == DIDVID_i210_S)    ||
			(dDidVid == DIDVID_x540) ||
			(dDidVid == DIDVID_82599_SFP) )
		{

#ifdef DEBUG
			sprintf (buffer, "Instance %d found at address: %d:%d:%d, %#X:%#X\n",
							bCount + 1, bBus, bDev, bFunc, dDidVid & 0xFFFF, dDidVid >> 16);
			vConsoleWrite (buffer);
#endif

			params.dDidVid = dDidVid;
			params.bInstance = bCount + 1;		// service requires one based counter
			params.bMode = bMode;
			params.pfa = pfa;

			if (board_service (SERVICE__BRD_SKIP_ETH_IF, NULL, &params) == E__OK)
			{
#ifdef DEBUG
				vConsoleWrite("Device Skipped\n");
#endif
/*
				bCount--;

				(bDev)++;
				if (bDev == 32)
				{
					bDev = 0;
					(bBus)++;
				}
				continue;
*/
			}
			else
			{
				++bCount;
				if (bCount == bInstance)	/* we found the card of interest */
				{
#ifdef DEBUG
					sprintf (buffer, "Found Ethernet device %d\n", bCount);
					vConsoleWrite (buffer);
#endif

					if ((dDidVid == DIDVID_i350AMx_S) ||
						(dDidVid == DIDVID_i210_S))
					{
						psDev->bType = IF_SERDES;
					}

					if ((dDidVid == DIDVID_82599_SFP))
					{
						psDev->bType = IF_FIBER;
					}

					if ((dDidVid == DIDVID_i350AMx_C) ||
						(dDidVid == DIDVID_i350AMx_S) ||
						(dDidVid == DIDVID_x540) ||
						(dDidVid == DIDVID_82599_SFP))
					{
						psDev->bMultiFunc = 1;
					}

					psDev->bBus = bBus;
					psDev->bDev = bDev;
					psDev->bFunc = bFunc;

#ifdef DEBUG
					sprintf (buffer, "Success: Device Found\n");
					vConsoleWrite (buffer);
#endif

					return (PCI_SUCCESS);
				}
			}
		}

		if (bFunc == 0)	/* check for multifunction devices */
		{
			if (dDidVid == 0xFFFFFFFF)	/* no device on the bus at this address */
			{
				bMultiFunc = 0;
			}
			else
			{
				pfa = PCI_MAKE_PFA (bBus, bDev, 0);
				bTemp = (UINT8)dPciReadReg (pfa, PCI_HEADER_TYPE, PCI_BYTE);

				bMultiFunc = (bTemp & 0x80) || 					/* found a multifunction device */
							 ((bBus == 0) && (bDev == 0));		/* always set pfa 0:0:0 to multifunction device */
			}
		}

		if (dDidVid != 0xFFFFFFFF)	/* found something */
		{
			pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
			bTemp = (UINT8) dPciReadReg (pfa, PCI_BASE_CLASS, PCI_BYTE);

			if (bTemp == 0x06)	/* PCI<->PCI bridge class */
			{
				pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
				bTemp = (UINT8) dPciReadReg (pfa, PCI_SUB_CLASS, PCI_BYTE);

				if (bTemp == 0x00)
				{
					if (bHostBridges > 0)
					{
						++bScanToBus;
					}

					++bHostBridges;
				}
				else if (bTemp == 0x04)		/* PCI-PCI bridge*/
				{
					pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
					bTemp = (UINT8) dPciReadReg (pfa, PCI_SUB_BUS, PCI_BYTE);

					if (bTemp > bScanToBus)
					{
						bScanToBus = bTemp;
					}
				}
			}
		}

		/* Increment device/bus numbers */

		if (bMultiFunc == 0)
			++bDev;
		else
		{
			++bFunc;
			if (bFunc == 8)
			{
				++bDev;
				bFunc = 0;
				bMultiFunc = 0;
			}
		}

		if (bDev == 32)
		{
			bDev = 0;
			++bBus;
		}

	} while (bBus <= bScanToBus);

#ifdef DEBUG
		sprintf (buffer, "Exit: bBus:%d bDev:%d bFunc:%d bScanToBus: %u\n",bBus,bDev,bFunc, bScanToBus);
		vConsoleWrite (buffer);
#endif

	return (E__TEST_NO_DEVICE);
}

/*******************************************************************
 * dGetNumberOfEthInstances: Helper function to obtain number of
 * Ethernet ports present on boards, esp when board has dynamic
 * port configurations - PO fitted or not
 *
 * RETURNS: Number of ethernet ports available on board
 */
UINT32 dGetNumberOfEthInstances (void)
{
	DEVICE	sDevice;
	UINT16	wPciStatus = PCI_SUCCESS;
	UINT8	bInstance = 1;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	while (wPciStatus == PCI_SUCCESS)
	{
		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);
		++bInstance;
	}
#ifdef DEBUG
	sprintf (buffer, "Number of Instances: %d\n", (bInstance - 2));
	vConsoleWrite (buffer);
#endif

	return (bInstance - 2);
}

/*******************************************************************
 * vClosePciDevice: disable all access to the device
 *
 * RETURNS: none
 */
static void vClosePciDevice
(
	DEVICE* psDev
)
{
	UINT16	wTemp;
	PCI_PFA pfa;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	wTemp = (UINT16) dPciReadReg (pfa, PCI_COMMAND, PCI_WORD);

	vPciWriteReg (pfa, PCI_COMMAND,
					PCI_WORD, wTemp & ~(PCI_IOEN | PCI_MEMEN | PCI_BMEN));

	/* clear up all used mem regions */
	vFreePtr(dHandle1);

	if (dHandle2 != 0)
		vFreePtr (dHandle2);

	if (dHandle3 != 0)
		vFreePtr (dHandle3);

	vFreePtr (dHandle4);
	vFreePtr (dHandle5);
	vFreePtr (dHandle6);
	vFreePtr (dHandle7);

}

/*****************************************************************************
 * EthRegAccessTest: test function for Ethernet Register access
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (EthRegAccessTest2, "Ethernet Register Access Test")
{
	UINT16	wPciStatus;
	UINT32	dtest_status;
	UINT8	bInstance = 0, bLoop = 0, i = 1;
	DEVICE	sDevice;

	if (adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			++bInstance;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		++bInstance;
	}

	// initialise any x540 controllers in case we will be testing with them
	initX540CommonRegisters();

	do {
#ifdef DEBUG
		sprintf( buffer, "\nInstance to find: %d\n", bInstance);
		vConsoleWrite( buffer);
#endif
		wPciStatus = dFind825xx (&sDevice, bInstance, ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
			if (dtest_status != E__OK)
			{
#ifdef DEBUG
				sprintf (buffer, "Failed to initialise the device: %d\n", bInstance);
				vConsoleWrite (buffer);
#endif
				vClosePciDevice (&sDevice);
				return (dtest_status);
			}
			else
			{
				dtest_status = dRegisterTest (&sDevice);
#ifdef DEBUG
				sprintf(buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
#endif
			}
		}
		else if ((bLoop == 0) ||
				((bLoop == 1) && (bInstance == 1)) ||
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
			sprintf(buffer, "Unable to locate instance %d\n", bInstance);
			vConsoleWrite(buffer);
#endif

			return (E__TEST_NO_DEVICE);
		}
		else
		{
			bLoop = 0; /* soft exit if cannot find controller in loop mode */
#ifdef DEBUG
			vConsoleWrite("Need to soft Exit\n");
#endif
		}

		if (dtest_status == E__OK)
		{
			if (adTestParams[1] == 0)
			{
				++bInstance;
#ifdef DEBUG
				vConsoleWrite("adTestParams[1] == 0\n");
#endif
			}
			else
			{
				if (i < adTestParams[0])
				{
					++i;
					bInstance = adTestParams[i];
					bLoop = 1;
#ifdef DEBUG
					vConsoleWrite ("i < adTestParams[0]\n");
#endif
				}
				else
				{
					bLoop = 0;
#ifdef DEBUG
					vConsoleWrite("bLoop == 0\n");
	#endif
				}
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));/* test all requested instances */

	vClosePciDevice (&sDevice);
	if (dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf (buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif
	}

	return (dtest_status);
}

/*****************************************************************************
 * EthEepromTest: test function for Ethernet EEPROM connectivity and checksum
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (EthEepromTest2, "Ethernet EEPROM Test")
{
	UINT16	wPciStatus;
	UINT32	dRegData;
	UINT32	dtest_status;
	UINT8	bInstance = 0, bLoop = 0, i = 1;
	static	UINT8 cont = 0;
	DEVICE	sDevice;

	if(adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			++bInstance;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		++bInstance;
	}

	// initialise any x540 controllers in case we will be testing with them
	initX540CommonRegisters();

	do {
#ifdef DEBUG
		sprintf (buffer, "\nInstance to find: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif

		wPciStatus = dFind825xx (&sDevice, bInstance, ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
			if (!(bInstance & 0xFE))
			{
				++cont;
			}

			if (dtest_status != E__OK)
			{
#ifdef DEBUG
				sprintf (buffer, "Failed to initialise the device: %d\n", bInstance);
				vConsoleWrite (buffer);
#endif

				vClosePciDevice (&sDevice);
				return (dtest_status);
			}
			else
			{
				if (sDevice.bController == CTRLT_I210)
				{
					dRegData = (*(volatile UINT32 *) (sDevice.pbReg + EECD)); // dReadReg825xx(EECD);
					if ((dRegData & EECD_PRES) == EECD_PRES)
					{
						if ((dRegData & EECD_FLASH_IN_USE) == 0)
						{
							dtest_status = dEpromTestNVM (&sDevice);		// using iNVM
						}
						else
						{
							dtest_status = dEpromTest (&sDevice);			// using Flash Eeprom
						}
					}
				}
				else
				{
					dtest_status = dEpromTest (&sDevice);
				}
#ifdef DEBUG
				sprintf(buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
#endif
			}
		}
		else if ((bLoop == 0) ||
				((bLoop == 1) && (bInstance == 1)) ||
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
			sprintf (buffer, "Unable to locate instance %d\n", bInstance);
			vConsoleWrite (buffer);
#endif

			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
		{
			bLoop = 0;
		}

		if (dtest_status == E__OK)
		{
			if (adTestParams[1] == 0)
			{
				++bInstance;
			}
			else
			{
				if (i < adTestParams[0])
				{
					++i;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
				{
					bLoop = 0;
				}
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice (&sDevice);
	if (dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf (buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif
	}

	return (dtest_status);
}

/*****************************************************************************
 * EthPhyRegReadTest: test function for Ethernet PHY access
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (EthPhyRegReadTest2, "Ethernet PHY Test")
{
	UINT16	wPciStatus;
	UINT32	dtest_status;
	UINT8	bInstance = 0, bLoop = 0, i = 1;
	DEVICE	sDevice;
//	UINT16	wPhyReg;

	if (adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			++bInstance;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		++bInstance;
	}

	// initialise any x540 controllers in case we will be testing with them
	initX540CommonRegisters();

	do {
#ifdef DEBUG
		sprintf (buffer, "\nInstance to find: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif

		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
			if (dtest_status != E__OK)
			{
#ifdef DEBUG
				sprintf (buffer, "Failed to initialise the device: %d\n", bInstance);
				vConsoleWrite (buffer);
#endif

				vClosePciDevice (&sDevice);
				return (dtest_status);
			}
			else
			{
				if( (sDevice.bType == IF_SERDES) || (sDevice.bType == IF_FIBER ) )
				{
					dtest_status = E__OK;
					vClosePciDevice(&sDevice);
					//#ifdef DEBUG
						vConsoleWrite("Phy not supported in  SERDES/FIBER mode\n");
					//#endif
				}
				else
				{
					dCtrlReset(&sDevice);

					/* Initialize the device */
					vInit825xx (&sDevice, SPEED_1G); /* speed ignored for x540 */

					dtest_status = dReadPhyRegs (&sDevice);
					#ifdef DEBUG
						sprintf(buffer, "Test status: 0x%x\n", dtest_status);
						vConsoleWrite(buffer);
					#endif
				}

			}
		}

		else if ((bLoop == 0) ||
				((bLoop == 1) && (bInstance == 1)) ||
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
			sprintf(buffer, "Unable to locate instance %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
		{
			bLoop = 0;
		}

		if (dtest_status == E__OK)
		{
			if (adTestParams[1] == 0)
			{
				++bInstance;
			}
			else
			{
				if (i < adTestParams[0])
				{
					++i;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice (&sDevice);
	if (dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf (buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif
	}

	return (dtest_status);
}


/*****************************************************************************
 * EthIntLoopBackTest: test function for internal loopback data transfer
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (EthIntLoopBackTest2, "Ethernet Internal Loopback Test")
{
	UINT16	wPciStatus;
	UINT32	dtest_status;
	UINT8	bInstance = 0, bLoop = 0, i = 1;
	DEVICE	sDevice;

	if (adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			++bInstance;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		++bInstance;
	}

	// initialise any x540 controllers in case we will be testing with them
	initX540CommonRegisters();

	do {
#ifdef DEBUG
		sprintf (buffer, "\nInstance to find: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif

		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);
		
#ifdef DEBUG
        sprintf (buffer, "Device Instance: %d type: %d\n", bInstance, sDevice.bType);
        vConsoleWrite (buffer);
#endif

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
#ifdef DEBUG
			sprintf(buffer, "Device Instance: %d type: %d\n", bInstance, sDevice.bType);
			vConsoleWrite(buffer);
#endif

			if (dtest_status != E__OK)
			{
#ifdef DEBUG
				sprintf (buffer, "Failed to initialise the device: %d\n", bInstance);
				vConsoleWrite (buffer);
#endif

				vClosePciDevice (&sDevice);
				return (dtest_status);
			}
			else if (((sDevice.bType == IF_SERDES) || (sDevice.bType == IF_FIBER )) && (sDevice.bController != CTRLT_82599_SFP) )
			{
				dtest_status = E__OK;
				vClosePciDevice (&sDevice);
				vConsoleWrite ("Int Loopback not supported in  SERDES/FIBER mode\n");
			}
			else
			{
				dtest_status = dInternalLoopback (&sDevice, sDevice.bType);
#ifdef DEBUG
				sprintf (buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite (buffer);
#endif
			}
		}
		else if ((bLoop == 0) ||
				((bLoop == 1) && (bInstance == 1)) ||
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
      		vConsoleWrite("Unable to locate required instance");
#endif
			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
		{
			bLoop = 0;
		}
		
		if (dtest_status == E__OK)
		{
			if (adTestParams[1] == 0)
			{
				++bInstance;
			}
			else
			{
				if (i < adTestParams[0])
				{
					++i;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
				{
					bLoop = 0;
				}
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice (&sDevice);
	if (dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf (buffer, "Error in instance: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif
	}

	return (dtest_status);
}

/*****************************************************************************
 * EthInterruptTest: test function for interrupt
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (EthInterruptTest2, "Ethernet Interrupt Test")
{
	UINT16	wPciStatus;
	UINT32	dtest_status;
	UINT8	bInstance = 0, bLoop = 0, i = 1;
	static	UINT8 cont = 0;
	DEVICE	sDevice;

	if (adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			++bInstance;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		++bInstance;
	}

	// initialise any x540 controllers in case we will be testing with them
	initX540CommonRegisters();

	do {
#ifdef DEBUG
		sprintf (buffer, "\nInstance to find: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif

		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1 );
			if (!(bInstance & 0xFE))
			{
				cont++;
			}
			
			if (dtest_status != E__OK)
			{
#ifdef DEBUG
				sprintf(buffer, "Failes to initialise the device: %d\n", bInstance);
				vConsoleWrite(buffer);
#endif

				vClosePciDevice (&sDevice);
				return (dtest_status);
			}
			else
			{
				if (sDevice.bController == CTRLT_X540 || sDevice.bController  == CTRLT_82599_SFP)
				{
					dtest_status = dInterruptTestx540 (&sDevice);
				}
				else
				{
					dtest_status = dInterruptTest (&sDevice);
				}
#ifdef DEBUG
				sprintf (buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite (buffer);
#endif
			}
		}
		else if ((bLoop == 0) ||
				((bLoop == 1) && (bInstance == 1)) ||
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
			sprintf(buffer, "Unable to locate instance %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
		{
			bLoop = 0;
		}

		if (dtest_status == E__OK)
		{
			if (adTestParams[1] == 0)
			{
				++bInstance;
			}
			else
			{
				if (i < adTestParams[0])
				{
					++i;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
				{
					bLoop = 0;
				}
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	if (dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf (buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif
	}

	return (dtest_status);
}

/*****************************************************************************
 * EthExtLoopBackTest: test function for external loopback data transfer
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (EthExtLoopBackTest2, "Ethernet External Loopback Test")
{
	UINT16	wPciStatus;
	UINT32	dtest_status;
	UINT8	bInstance = 0, bLoop = 0, i = 1;
	static	UINT8 cont = 0;
	DEVICE	sDevice;

	if (adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			++bInstance;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		++bInstance;
	}
	
	// initialise any x540 controllers in case we will be testing with them
	initX540CommonRegisters();

	do {
#ifdef DEBUG
		sprintf (buffer, "\nInstance to find: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif

		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
#ifdef DEBUG
			sprintf (buffer, "%s found at b:%d %d f:%d\n",
					sDevice.achCtrlName, sDevice.bBus, sDevice.bDev, sDevice.bFunc);
			vConsoleWrite (buffer);
#endif
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
			if (!(bInstance & 0xFE))
			{
				++cont;
			}

			if (dtest_status != E__OK)
			{
#ifdef DEBUG
				sprintf(buffer, "Failed to initialise the device: %d\n", bInstance);
				vConsoleWrite(buffer);
#endif

				vClosePciDevice(&sDevice);
				return(dtest_status);
			}
			else
			{
				/* x540 does not support 10M external loopback */
				
				if (sDevice.bController != CTRLT_X540 && (sDevice.bController != CTRLT_82599_SFP))
				{
					dtest_status = dLocalLoopback (&sDevice, sDevice.bType, SPEED_10M);

					if (dtest_status == E__OK)
					{
						dtest_status = dLocalLoopback (&sDevice, sDevice.bType,SPEED_100M);
					}
				
					/* 1GB external loopback does not work on i210 */
					if (sDevice.bController != CTRLT_I210)
					{
						if (dtest_status == E__OK)
						{
							dtest_status = dLocalLoopback (&sDevice, sDevice.bType, SPEED_1G);
						}
					}
				}
				
				/* x540 10G external loopback only working */
				/* TODO: find out why 100M and 1G speeds selection fails */
				if (sDevice.bController == CTRLT_X540 || (sDevice.bController  == CTRLT_82599_SFP))
				{
					if (dtest_status == E__OK)
					{
						dtest_status = dLocalLoopback (&sDevice, sDevice.bType, SPEED_10G);
					}
				}

#ifdef DEBUG
				sprintf(buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
#endif
			}
		}
		else if ((bLoop == 0) ||
				((bLoop == 1) && (bInstance == 1)) ||
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
			sprintf(buffer, "Unable to locate instance %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
		{
			bLoop = 0;
		}

		if (dtest_status == E__OK)
		{
			if (adTestParams[1] == 0)
			{
				++bInstance;
			}
			else
			{
				if (i < adTestParams[0])
				{
					++i;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
				{
					bLoop = 0;
				}
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice (&sDevice);
	if (dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf (buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif
	}

	return (dtest_status);
} 

/*****************************************************************************
 * EthExtDevLoopbackTest: test function for external loopback data transfer
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (EthExtDevLoopbackTest2, "Ethernet Dev->Dev Loopback Test")
{
	UINT16	wPciStatus;
	UINT32	dtest_status;
	UINT8	bTxInstance = 1, bRxInstance = 2;
	DEVICE	sTxDevice;
	DEVICE	sRxDevice;
	char	achBuffer[80];
	UINT8	bTxInstanceType = 0;			// for i350 SERDES+Copper loopback
	UINT8	bRxInstanceType = 0;			// for i350 SERDES+Copper loopback

	if (adTestParams[0] >= 1)
	{
		bTxInstance = adTestParams[1];
	}

	if (adTestParams[0] >= 2)
	{
		bRxInstance = adTestParams[2];
	}

	sprintf (achBuffer, "Testing Tx Instance %d, Rx Instance %d\n", bTxInstance, bRxInstance);
	vConsoleWrite (achBuffer);

	// initialise any x540 controllers in case we will be testing with them
	initX540CommonRegisters();

#ifdef DEBUG
	sprintf (buffer, "Tx Instance to find: %d\n", bTxInstance);
	vConsoleWrite (buffer);
#endif

	wPciStatus = dFind825xx (&sTxDevice, bTxInstance, ETH_MAC_NO_DISP_MODE);

	if (wPciStatus == PCI_SUCCESS)
	{
		dtest_status = dInitPciDevice (&sTxDevice, bTxInstance);
		if (dtest_status != E__OK)
		{
#ifdef DEBUG
			sprintf (buffer, "Failed to initialise the Tx device: %d\n", bTxInstance);
			vConsoleWrite (buffer);
#endif
			vClosePciDevice (&sTxDevice);
			return (dtest_status);
		}
	}
	else
	{
#ifdef DEBUG
		sprintf (buffer, "Tx Instance not found: %d\n", bTxInstance);
		vConsoleWrite (buffer);
#endif

		return E__TEST_NO_DEVICE + bTxInstance;
	}

#ifdef DEBUG
	sprintf (buffer, "Rx Instance to find: %d\n", bRxInstance);
	vConsoleWrite (buffer);
#endif

	wPciStatus = dFind825xx (&sRxDevice, bRxInstance, ETH_MAC_NO_DISP_MODE);

	if (wPciStatus == PCI_SUCCESS)
	{
		dtest_status = dInitPciDevice (&sRxDevice, bRxInstance);
		if (dtest_status != E__OK)
		{
#ifdef DEBUG
			sprintf(buffer, "Failed to initialise the Rx device: %d\n", bRxInstance);
			vConsoleWrite(buffer);
#endif

			vClosePciDevice (&sTxDevice);
			vClosePciDevice (&sRxDevice);
			return (dtest_status);
		}
	}
	else
	{
#ifdef DEBUG
		sprintf (buffer, "Rx Instance not found: %d\n", bRxInstance);
		vConsoleWrite (buffer);
#endif
		vClosePciDevice (&sTxDevice);
		return (E__TEST_NO_DEVICE + bRxInstance);
	}

#ifdef DEBUG
	sprintf (buffer, "TxType = %d, RxType = %d\n", sTxDevice.bController, sRxDevice.bController);
	vConsoleWrite (buffer);
#endif

	// testing both channels on same device
	if ((sTxDevice.bController == CTRLT_I350) &&
		(sRxDevice.bController == CTRLT_I350))
	{
		// save interface type configured by the eeprom
		bTxInstanceType = sTxDevice.bType;
		bRxInstanceType = sRxDevice.bType;
    
		if (bTxInstanceType != bRxInstanceType)
		{
			// one device is SERDES, the other is COPPER
			// so switch them both to SERDES for this test
			if (sTxDevice.bType == IF_SERDES)
			{
				// RxDevice is configured as IF_COPPER
				vSwitchI350ToSerdes (&sRxDevice);
			}
			else
			{
				// TxDevice is configured as IF_COPPER
				vSwitchI350ToSerdes (&sTxDevice);
			}
		}
	}

	if ((sTxDevice.bType == IF_SERDES) && (sRxDevice.bType == IF_SERDES))
	{
#ifdef DEBUG
		vConsoleWrite("Serdes->Serdes loop-back\n");
#endif
		dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_1G);

		// check original interface types
		if (bTxInstanceType != bRxInstanceType)
		{
			// one device is SERDES, the other is COPPER
			// so restore interfaces to original eeprom configuration
			if (bTxInstanceType == IF_SERDES)
			{
				// RxDevice was configured as IF_COPPER
				vSwitchI350ToCopper (&sRxDevice);
			}
			else
			{
				// TxDevice was configured as IF_COPPER
				vSwitchI350ToCopper (&sTxDevice);
			}
		}
	}
	else if ((sTxDevice.bType == IF_COPPER) && (sRxDevice.bType == IF_COPPER))
	{
#ifdef DEBUG
		vConsoleWrite ("Copper->Copper loop-back\n");
#endif
	if ((sTxDevice.bController == CTRLT_X540) &&
		(sRxDevice.bController == CTRLT_X540))
		{
			dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_10G);
		}
	else	if ((sTxDevice.bController == CTRLT_82599_SFP) &&
			(sRxDevice.bController == CTRLT_82599_SFP))
			{
				dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_10G);
			}
		else
		{
			dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_1G);
		}
	}
	else if ((sTxDevice.bType == IF_FIBER) && (sRxDevice.bType == IF_FIBER))
	{
		if ((sTxDevice.bController == CTRLT_82599_SFP) &&
			(sRxDevice.bController == CTRLT_82599_SFP))
			{
				dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_10G);
			}
	}
	else
	{
		vConsoleWrite ("Device->Device Test only possible on SAME MEDIA\n");
		sprintf (achBuffer, "sTxDevice.bType:%#x  sRxDevice.bType%#x\n", sTxDevice.bType,sRxDevice.bType);
		vConsoleWrite (achBuffer);
		return (E__OK);
	}

#ifdef DEBUG
	sprintf (buffer, "Test status: 0x%x\n", dtest_status);
	vConsoleWrite (buffer);
#endif

	vClosePciDevice (&sTxDevice);
	vClosePciDevice (&sRxDevice);
	return (dtest_status);
}

/*****************************************************************************
 * EthEepromWriteTest: test function for non destructive read and write of
 *						Ethernet EEPROM
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (EthEepromWriteTest2, "Ethernet EEPROM Read Write Test")
{
	UINT16	wPciStatus;
	UINT32	dtest_status;
	UINT8	bInstance = 0, bLoop = 0, i = 1;
	static	UINT8 cont = 0;
	DEVICE	sDevice;

	if (adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			++bInstance;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		++bInstance;
	}
	
	// initialise any x540 controllers in case we will be testing with them
	initX540CommonRegisters();

	do {
#ifdef DEBUG
			sprintf (buffer, "\nInstance to find: %d\n", bInstance);
			vConsoleWrite (buffer);
#endif

		wPciStatus = dFind825xx (&sDevice, bInstance, ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
			if (!(bInstance & 0xFE))
			{
				++cont;
			}
			
			if (dtest_status != E__OK)
			{
#ifdef DEBUG
				sprintf (buffer, "Failed to initialise the device: %d\n", bInstance);
				vConsoleWrite (buffer);
#endif

				vClosePciDevice (&sDevice);
				return (dtest_status);
			}
			else
			{
				dtest_status = dEepromrwtest (&sDevice, sDevice.bType);
#ifdef DEBUG
				sprintf (buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite (buffer);
#endif
			}
		}
		else if ((bLoop == 0) ||
				((bLoop == 1) && (bInstance == 1)) ||
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
			sprintf(buffer, "Unable to locate instance %d\n", bInstance);
			vConsoleWrite(buffer);
#endif

			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
		{
			bLoop = 0;
		}

		if (dtest_status == E__OK)
		{
			if (adTestParams[1] == 0)
			{
				++bInstance;
			}
			else
			{
				if (i < adTestParams[0])
				{
					++i;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
				{
					bLoop = 0;
				}
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));		/* test all requested instances */

	vClosePciDevice (&sDevice);
	if (dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf (buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite (buffer);
#endif
	}
	
	return (dtest_status);
}

#if 0
static void	vSerDesLoopbackMode
(
	DEVICE *psDev
)
{
	UINT32	dReg = 0;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	dReg = dReadReg825xx (CTRL);
	vWriteReg825xx (CTRL, dReg | CTRL_LRST);
	vDelay (10);

	vWriteReg825xx (CTRL_EXT, dReadReg825xx (CTRL_EXT) | (0xC00000));
	vDelay (10);
	vWriteReg825xx (SERDESCTL, ENABLE_SERDES_LOOPBACK);

	vDelay (20);
	dReg = dReadReg825xx(CTRL);
	dReg &= ~(CTRL_RFCE |
			CTRL_TFCE |
			CTRL_RST);

	dReg |= (CTRL_SLU | CTRL_FD | CTRL_ILOS);

	vWriteReg825xx (CTRL, dReg);
	vDelay (10);
	/* Unset switch control to serdes energy detect */
	dReg = dReadReg825xx (CONNSW);
	dReg &= ~CONNSW_ENRGSRC;
	vWriteReg825xx (CONNSW, dReg);
	vDelay (10);

	/* Set PCS register for forced speed */
	dReg = dReadReg825xx (PCS_LCTL);
	dReg &= ~PCS_LCTL_AN_ENABLE;		/* Disable Autoneg*/

	dReg |= PCS_LCTL_FLV_LINK_UP |		/* Force link up */
			PCS_LCTL_FSV_1000    |		/* Force 1000    */
			PCS_LCTL_FDV_FULL    |		/* SerDes Full duplex */
			PCS_LCTL_FSD         |		/* Force Speed */
			PCS_LCTL_FORCE_LINK;		/* Force Link */

	vWriteReg825xx(PCS_LCTL, dReg);
}
#endif

static void vSerDesChannelLoopbackMode
(
	DEVICE	*psDev
)
{
	UINT32	dReg = 0,dLinkUp = 0;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController != CTRLT_I350)
	{
		vWriteReg825xx(SERDESCTL, SCTL_DISABLE_SERDES_LOOPBACK);
	}

	dReg = dReadReg825xx (PCS_CFG);
	dReg |= 0x00000008;
	vWriteReg825xx (PCS_CFG, dReg);
	vDelay (10);

	dReg = dReadReg825xx (CTRL);
	dReg &= ~(CTRL_ILOS /*|CTRL_RFCE|CTRL_TFCE*/);
	dReg |= (CTRL_FD | CTRL_SLU);
	vWriteReg825xx (CTRL, dReg);
	vDelay (10);
	
	dReg = dReadReg825xx (CTRL_EXT);
	dReg &= 0xFF3FFFFFL;
	dReg |= 0x00C00000L;
	vWriteReg825xx (CTRL_EXT, dReg);

	/* Set CONNSW.ENRGSRC to disable */
	dReg = dReadReg825xx (CONNSW);
	dReg &= ~(CONNSW_ENRGSRC);
	vWriteReg825xx (CONNSW, dReg);
	vDelay(100);

	dReg = dReadReg825xx (PCS_LSTAT);
	if (dReg & PCS_LSTS_SYNK_OK)
	{
		dLinkUp = TRUE;
		vConsoleWrite ("PCS: Link is UP\n");
	}
	else
	{
		vConsoleWrite ("PCS: Link is Down\n");
		dLinkUp = FALSE;

		/* Force link up in the PCS register */
		dReg = dReadReg825xx (PCS_LCTL);
		
		dReg &= ~(PCS_LCTL_AN_ENABLE|PCS_LCTL_AN_RESTART|PCS_LCTL_AN_TIMEOUT); /* Disable ANEG */
		dReg |= PCS_LCTL_FLV_LINK_UP |		/* Force link up */
				PCS_LCTL_FSV_1000    |		/* Force 1000 */
				PCS_LCTL_FDV_FULL    |		/* SerDes Full duplex */
				PCS_LCTL_FSD         |		/* Force Speed */
				PCS_LCTL_FORCE_LINK;		/* Force Link */
				
		vWriteReg825xx (PCS_LCTL, dReg);
		vDelay (20);
	}

	/* Set ILOS if link is not up */
	if ((dLinkUp == FALSE) && (psDev->bController != CTRLT_I350))
	{
		/* Set bit 7 (Invert Loss) and set link up in bit 6. */
		dReg = dReadReg825xx (CTRL);
		dReg |= (CTRL_ILOS);
		vWriteReg825xx (CTRL, dReg);
	}
	vDelay (20);
}


/*****************************************************************************
 * EthInterfaceSelect: Ethernet Front/Rear Interface Selection
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthInterfaceSelect2, "Ethernet IF Selection")
{
	UINT32	rt;
	SelEthIf_PARAMS params;

	if ((adTestParams[0] != 2))
	{
		return E__TEST_WRONG_NUMBER_OF_PARAMS;
	}
	
	params.cnt   = adTestParams[1];
	params.Iface = adTestParams[2];
	board_service(SERVICE__BRD_SELECT_ETH_IF, &rt, &params);

	return rt;

}

#ifdef INCLUDE_STATS
/*******************************************************************
 * vClear8257xStatistics: reset the statistical counters to zero
 *
 * RETURNS: none
 */
static void vClear8257xStatistics
(
	DEVICE* psDev		/* pointer to device data structure */
)
{
	UINT8		cnt;
	UINT32		dTemp;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	if (psDev->bController == CTRLT_X540 || (psDev->bController == CTRLT_82599_SFP))
	{
		
		vWriteReg825xx (SSVPC, 0);
		vWriteReg825xx (TXDGPC, 0);
		vWriteReg825xx (TXDGBCL, 0);
		vWriteReg825xx (TXDGBCH, 0);

		for (cnt = 0; cnt < 32; ++cnt)
		{
			vWriteReg825xx (PXON2OFFCNT(cnt), 0);
		}

		for (cnt = 0; cnt < 16; ++cnt)
		{
			vWriteReg825xx (QPRC(cnt), 0);
			vWriteReg825xx (QPRDC(cnt), 0);
			vWriteReg825xx (QBRC_L(cnt), 0);
			vWriteReg825xx (QBRC_H(cnt), 0);
			vWriteReg825xx (QPTC(cnt), 0);
			vWriteReg825xx (QBTC_L(cnt), 0);
			vWriteReg825xx (QBTC_H(cnt), 0);
		}

		for (cnt = 0; cnt < 32; ++cnt)
		{
			vWriteReg825xx (TQSM(cnt), 0);
		}
		
		dTemp = RXDGPC;				// 0x02F50L
		while (dTemp < 0x02F80L)
		{
			vWriteReg825xx (dTemp, 0);
			dTemp += 4;
		}

		dTemp = PXONTXC(0);			// 0x03F00L
		while (dTemp < 0x041F0L)
		{
			vWriteReg825xx (dTemp, 0);
			dTemp += 4;
		}
		
	}
	else
	{
		vWriteReg825xx (CRCERRS, 0);
		vWriteReg825xx (ALGNERRC, 0);
		vWriteReg825xx (SYMERRS, 0);
		vWriteReg825xx (RXERRC, 0);
		vWriteReg825xx (MPC, 0);
		vWriteReg825xx (SCC, 0);
		vWriteReg825xx (ECOL, 0);
		vWriteReg825xx (MCC, 0);
		vWriteReg825xx (LATECOL, 0);
		vWriteReg825xx (COLC, 0);
		vWriteReg825xx (DC, 0);
		vWriteReg825xx (TNCRS, 0);
		vWriteReg825xx (SEC, 0);
		vWriteReg825xx (CEXTERR, 0);
		vWriteReg825xx (RLEC, 0);
		vWriteReg825xx (XONRXC, 0);
		vWriteReg825xx (XONTXC, 0);
		vWriteReg825xx (XOFFRXC, 0);
		vWriteReg825xx (XOFFTXC, 0);
		vWriteReg825xx (FCRUC, 0);
		vWriteReg825xx (PRC64, 0);
		vWriteReg825xx (PRC127, 0);
		vWriteReg825xx (PRC255, 0);
		vWriteReg825xx (PRC511, 0);
		vWriteReg825xx (PRC1023, 0);
		vWriteReg825xx (PRC1522, 0);
		vWriteReg825xx (GPTC, 0);
		vWriteReg825xx (GORCL, 0);
		vWriteReg825xx (GORCH, 0);
		vWriteReg825xx (GOTCL, 0);
		vWriteReg825xx (GOTCH, 0);
		vWriteReg825xx (RNBC, 0);
		vWriteReg825xx (RUC, 0);
		vWriteReg825xx (RFC, 0);
		vWriteReg825xx (ROC, 0);
		vWriteReg825xx (RJC, 0);
		vWriteReg825xx (TORL, 0);
		vWriteReg825xx (TORH, 0);
		vWriteReg825xx (TOTL, 0);
		vWriteReg825xx (TOTH, 0);
		vWriteReg825xx (TPR, 0);
		vWriteReg825xx (TPT, 0);
		vWriteReg825xx (PTC64, 0);
		vWriteReg825xx (PTC127, 0);
		vWriteReg825xx (PTC255, 0);
		vWriteReg825xx (PTC511, 0);
		vWriteReg825xx (PTC1023, 0);
		vWriteReg825xx (PTC1522, 0);
		vWriteReg825xx (MPTC, 0);
		vWriteReg825xx (BPTC, 0);
	}
}

/*******************************************************************
 * vDisplay8257xStats: formatted display of statistical counters
 *
 * RETURNS: none
 */

static void vDisplay8257xStats
(
	DEVICE*	psDev		/* pointer to device data structure */
)
{
	char	achBuffer[LINE_BUFF_SIZE];

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	/* Display the generalised error stats */

	vConsoleWrite ("Error statistics\n");
	
	if (psDev->bController == CTRLT_X540 || (psDev->bController == CTRLT_82599_SFP))
	{
		sprintf (achBuffer, "CRC count .......... %08X  Illegal byte count . %08X\n",
				dReadReg825xx (CRCERRS), dReadReg825xx (ILLERRC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Error byte count ... %08X\n",
				dReadReg825xx (ERRBC));
		vConsoleWrite (achBuffer);

		sprintf (achBuffer, "Rx missed pkts 0 ... %08X  Rx missed pkts 1 ... %08X\n",
				dReadReg825xx (RXMPC(0)), dReadReg825xx (RXMPC(0)));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Rx missed pkts 2 ... %08X  Rx missed pkts 3 ... %08X\n",
				dReadReg825xx (RXMPC(2)), dReadReg825xx (RXMPC(3)));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Rx missed pkts 4 ... %08X  Rx missed pkts 5 ... %08X\n",
				dReadReg825xx (RXMPC(4)), dReadReg825xx (RXMPC(5)));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Rx missed pkts 6 ... %08X  Rx missed pkts 7 ... %08X\n",
				dReadReg825xx (RXMPC(6)), dReadReg825xx (RXMPC(7)));
		vConsoleWrite (achBuffer);

		sprintf (achBuffer, "Mac local faults ... %08X  MAC remote faults .. %08X\n",
				dReadReg825xx (MLFC), dReadReg825xx (MRFC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Rx length errors ... %08X\n",
				dReadReg825xx (RLEC));
		vConsoleWrite (achBuffer);

		sprintf (achBuffer, "Link XON Tx count .. %08X  Link XON Rx count .. %08X\n",
				dReadReg825xx (LXONTXC), dReadReg825xx (LXONRXCNT));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Link XOFF Tx count . %08X  Link XOFF Rx count . %08X\n",
				dReadReg825xx (LXOFFTXC), dReadReg825xx (LXOFFRXCNT));
		vConsoleWrite (achBuffer);

		/* Display the TX and RX stats */
	
		vConsoleWrite ("Transmit and Receive statistics\n");
		
		sprintf (achBuffer, "Total packets TX ... %08X\n",
				dReadReg825xx (TPT));
		vConsoleWrite (achBuffer);
		
		sprintf (achBuffer, "Good packets TX .... %08X  Good octets TX ..... %08X\n",
				dReadReg825xx (GPTC), dReadReg825xx (GOTCL));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Broadcast packets .. %08X  Multicast packets .. %08X\n",
				dReadReg825xx (BPTC), dReadReg825xx (MPTC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "64 byte packets .... %08X  65-127 byte packets  %08X\n",
				dReadReg825xx (PTC64), dReadReg825xx (PTC127));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "128-255 byte packets %08X  256-511 byte packets %08X\n",
				dReadReg825xx (PTC255), dReadReg825xx (PTC511));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "512-1023 byte packet %08X  1024-Max byte packet %08X\n",
				dReadReg825xx (PTC1023), dReadReg825xx (PTC1522));
		vConsoleWrite (achBuffer);
	

		sprintf (achBuffer, "Total packets RX ... %08X  Total octets RX .... %08X:%08X\n",
				dReadReg825xx (TPR), dReadReg825xx (TORH), dReadReg825xx (TORL));
		vConsoleWrite (achBuffer);
	
		sprintf (achBuffer, "Good packets RX .... %08X  Good octets RX ..... %08X\n",
				dReadReg825xx (GPRC), dReadReg825xx (GORCL));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Broadcast packets .. %08X  Multicast packets .. %08X\n",
				dReadReg825xx (BPRC), dReadReg825xx (MPRC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "64 byte packets .... %08X  65-127 byte packets  %08X\n",
				dReadReg825xx (PRC64), dReadReg825xx (PRC127));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "128-255 byte packets %08X  256-511 byte packets %08X\n",
				dReadReg825xx (PRC255), dReadReg825xx (PRC511));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "512-1023 byte packet %08X  1024-Max byte packet %08X\n",
				dReadReg825xx (PRC1023), dReadReg825xx (PRC1522));
		vConsoleWrite (achBuffer);

		sprintf (achBuffer, "Good nonfilt pkts RX %08X  Good nonfilt octets  %08X\n",
				dReadReg825xx (RXNFGPC), dReadReg825xx (RXNFGBCL));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "DMA Good packets RX  %08X  DMA Good octets RX . %08X\n",
				dReadReg825xx (RXDDPC), dReadReg825xx (RXDDBCL));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "RX undersize ....... %08X\n",
				dReadReg825xx (RUC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "RX fragment ........ %08X  RX oversize ........ %08X\n",
				dReadReg825xx (RFC), dReadReg825xx (ROC));
		vConsoleWrite (achBuffer);
	}
	else
	{
		sprintf (achBuffer, "CRC errors ......... %08X  Alignment errors ... %08X\n",
				dReadReg825xx (CRCERRS), dReadReg825xx (ALGNERRC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Symbol errors ...... %08X  RX errors .......... %08X\n",
				dReadReg825xx (SYMERRS), dReadReg825xx (RXERRC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Missed packets ..... %08X                           \n",
				dReadReg825xx (MPC));
		vConsoleWrite (achBuffer);
	
		sprintf (achBuffer, "Single collision ... %08X  Excessive collisions %08X\n",
				dReadReg825xx (SCC), dReadReg825xx (ECOL));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Multiple collisions  %08X  Late collisions .... %08X\n",
				dReadReg825xx (MCC), dReadReg825xx (LATECOL));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Collision count .... %08X                           \n",
				dReadReg825xx (COLC));
		vConsoleWrite (achBuffer);
	
		sprintf (achBuffer, "Defer count ........ %08X  Transmit with no CRS %08X\n",
				dReadReg825xx (DC), dReadReg825xx (TNCRS));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Sequence errors .... %08X  Carrier extension .. %08X\n",
				dReadReg825xx (SEC), dReadReg825xx (CEXTERR));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "RX length errors ... %08X                           \n",
				dReadReg825xx (RLEC));
		vConsoleWrite (achBuffer);
	
		sprintf (achBuffer, "XON RX count ....... %08X  XOFF RX count ...... %08X\n",
				dReadReg825xx (XONRXC), dReadReg825xx (XOFFRXC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "XON TX count ....... %08X  XOFF TX count ...... %08X\n",
				dReadReg825xx (XONTXC), dReadReg825xx (XOFFTXC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "FC unsupported count %08X                           \n",
				dReadReg825xx (FCRUC));
		vConsoleWrite (achBuffer);
	
		/* Display the TX and RX stats */
	
		vConsoleWrite ("Transmit and Receive statistics\n");
		sprintf (achBuffer, "Total packets TX ... %08X  Total octets TX .... %08X:%08X\n",
				dReadReg825xx (TPT), dReadReg825xx (TOTH), dReadReg825xx (TOTL));
		vConsoleWrite (achBuffer);
		
		sprintf (achBuffer, "Good packets TX .... %08X  Good octets TX ..... %08X\n",
				dReadReg825xx (GPTC), dReadReg825xx (GOTCL));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Broadcast packets .. %08X  Multicast packets .. %08X\n",
				dReadReg825xx (BPTC), dReadReg825xx (MPTC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "64 byte packets .... %08X  65-127 byte packets  %08X\n",
				dReadReg825xx (PTC64), dReadReg825xx (PTC127));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "128-255 byte packets %08X  256-511 byte packets %08X\n",
				dReadReg825xx (PTC255), dReadReg825xx (PTC511));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "512-1023 byte packet %08X  1024-Max byte packet %08X\n",
				dReadReg825xx (PTC1023), dReadReg825xx (PTC1522));
		vConsoleWrite (achBuffer);
	
		sprintf (achBuffer, "Total packets RX ... %08X  Total octets RX .... %08X:%08X\n",
				dReadReg825xx (TPR), dReadReg825xx (TORH), dReadReg825xx (TORL));
		vConsoleWrite (achBuffer);
	
		sprintf (achBuffer, "Good packets RX .... %08X  Good octets RX ..... %08X\n",
				dReadReg825xx (GPRC), dReadReg825xx (GORCL));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Broadcast packets .. %08X  Multicast packets .. %08X\n",
				dReadReg825xx (BPRC), dReadReg825xx (MPRC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "64 byte packets .... %08X  65-127 byte packets  %08X\n",
				dReadReg825xx (PRC64), dReadReg825xx (PRC127));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "128-255 byte packets %08X  256-511 byte packets %08X\n",
				dReadReg825xx (PRC255), dReadReg825xx (PRC511));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "512-1023 byte packet %08X  1024-Max byte packet %08X\n",
				dReadReg825xx (PRC1023), dReadReg825xx (PRC1522));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "RX no buffers ...... %08X  RX undersize ....... %08X\n",
				dReadReg825xx (RNBC), dReadReg825xx (RUC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "RX fragment ........ %08X  RX oversize ........ %08X\n",
				dReadReg825xx (RFC), dReadReg825xx (ROC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "RX jabber .......... %08X                           \n",
				dReadReg825xx (RJC));
		vConsoleWrite (achBuffer);
	}
}

/*******************************************************************
 * vDisplayPhyRegs: displays the contents PHY registers
 *
 * RETURNS: None
 */

static void vDisplayPhyRegs
(
	DEVICE* psDev
)
{
	int	i;
	int mmd;
	UINT16	wRegVal1, wRegVal2, wRegVal3, wRegVal4;
	char	achBuffer[80];

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

	sprintf (achBuffer, "\nPHY registers (address = %02Xh)\n", psDev->bPhyAddress);
	vConsoleWrite (achBuffer);
	
	if (psDev->bController == CTRLT_X540 || (psDev->bController == CTRLT_82599_SFP))
	{
		vConsoleWrite ("01: Physical Medium Attachment/Physical Medium Dependent\n");
		mmd = MDIO_MMD_PMAPMD;		/* 1: Physical Medium Attachment/Physical Medium Dependent */
		for (i = 0; i < 12; i += 4)
		{
			wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i));
			wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 1));
			wRegVal3 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 2));
			wRegVal4 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 3));

			sprintf (achBuffer, "%02X:%04X  0x%04x 0x%04x 0x%04x 0x%04x\n", 
					mmd, i, wRegVal1, wRegVal2, wRegVal3, wRegVal4);
			vConsoleWrite (achBuffer);
		}
			
		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0E));
		wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x0F));
		sprintf (achBuffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x0E, wRegVal1, wRegVal2);
		vConsoleWrite (achBuffer);

		wRegVal3 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x81));
		wRegVal4 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0x82));
		sprintf (achBuffer, "%02X:%04X  0x%04X 0x%04X\n", 
				mmd, 0x81, wRegVal1, wRegVal2);
		vConsoleWrite (achBuffer);


		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xE400));
		sprintf (achBuffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xE400, wRegVal1);
		vConsoleWrite (achBuffer);

		wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, 0xE800));		
		sprintf (achBuffer, "%02X:%04X  0x%04X\n", 
				mmd, 0xE800, wRegVal1);
		vConsoleWrite (achBuffer);
		
		vConsoleWrite ("07: Auto-Negotiation\n");
		mmd = MDIO_MMD_AN;			/* 7: Auto-Negotiation */
		for (i = 0; i < 8; i += 4)
		{
			wRegVal1 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i));
			wRegVal2 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 1));
			wRegVal3 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 2));
			wRegVal4 = ReadPhyRegister (psDev, X540_PHY_REG(mmd, i + 3));

			sprintf (achBuffer, "%02X:%04X  0x%04X 0x%04X 0x%04X 0x%04X\n", 
					mmd, i, wRegVal1, wRegVal2, wRegVal3, wRegVal4);
			vConsoleWrite (achBuffer);
		}

	}
	else
	{
		for (i = 0; i < 25; i += 4)
		{
			wRegVal1 = ReadPhyRegister (psDev, i);
			wRegVal2 = ReadPhyRegister (psDev, i + 1);
			wRegVal3 = ReadPhyRegister (psDev, i + 2);
			wRegVal4 = ReadPhyRegister (psDev, i + 3);
			sprintf (achBuffer, "%3d  0x%04x 0x%04x 0x%04x 0x%04x\n", i,
					wRegVal1, wRegVal2, wRegVal3, wRegVal4);
					
			vConsoleWrite (achBuffer);
		}
	}
}
#endif

/*******************************************************************
 * vSwitchI350ToSerdes: change the i350 functional mode to SerDes operation
 *
 * RETURNS: None
 */

static void vSwitchI350ToSerdes
(
	DEVICE	*psDev
)	
{	
	UINT32	dRegVal;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

#ifdef DEBUG
	vConsoleWrite ("Switching COPPER interface to SERDES for SERDES->SERDES loopback test\n");
#endif

	// common

	// disable RCTL.RXEN
	dRegVal = dReadReg825xx (RCTL);
	dRegVal &= ~RCTL_EN;
	vWriteReg825xx (RCTL, dRegVal);

	// disable TCTL.TXEN
	dRegVal = dReadReg825xx (TCTL);
	dRegVal &= ~TCTL_EN;
	vWriteReg825xx (TCTL, dRegVal);

	// set mode CTRL_EXT.LINK_MODE = 11b
	dRegVal = dReadReg825xx (CTRL_EXT);
	dRegVal &= M__CTRLEXT_LINK;
	dRegVal |= CTRLEXT_LINK_TBI;
	vWriteReg825xx (CTRL_EXT, dRegVal);

	// using auto-negotiation
#ifdef DEBUG
	vConsoleWrite ("Using auto-negotiation\n");
#endif

	// set PCS_LCTL.AN_ENABLE
	dRegVal = dReadReg825xx (PCS_LCTL);
	dRegVal |= PCS_LCTL_AN_ENABLE;
	vWriteReg825xx (PCS_LCTL, dRegVal);

	// clear CTRL.FRCSPD
	// clear CTRL.FRCDPLX
	// clear CTRL.ILOS
	// set CTRL.SLU
	dRegVal = dReadReg825xx (CTRL);
	dRegVal &= ~(CTRL_FRCSPD + CTRL_FRCDPLX);
//	dRegVal &= ~(CTRL_FRCSPD + CTRL_FRCDPLX + CTRL_ILOS);
	dRegVal |= CTRL_SLU;
	vWriteReg825xx (CTRL, dRegVal);

	// clear PCS_LCTL.FSD
	// clear PCS_LCTL.force flow control
	// set speed PCS_LCTL.FSV = 10b
	// set PCS_LCTL.AN_TIMEOUT_EN
	dRegVal = dReadReg825xx (PCS_LCTL);
	dRegVal &= ~(PCS_LCTL_FSD + PCS_LCTL_FSV_100 + PCS_LCTL_FORCE_FCTRL);
	dRegVal |= PCS_LCTL_FSV_1000 + PCS_LCTL_AN_TIMEOUT;
	vWriteReg825xx (PCS_LCTL, dRegVal);

#if 0
	// NOT using auto-negotiation
#ifdef DEBUG
	vConsoleWrite ("Not using auto-negotiation\n");
#endif

	// set CTRL.FD
	// set CTRL.SLU
	// clear CTRL.RFCE
	// clear CTRL.TFCE
	// set CTRL.SPEED = 10b
	dRegVal = dReadReg825xx (CTRL);
	dRegVal &= ~(CTRL_RFCE + CTRL_TFCE + CTRL_ILOS);
	dRegVal &= M__CTRL_SPEED;
	dRegVal |= CTRL_SLU + CTRL_FD + CTRL_SPEED1000;
	vWriteReg825xx (CTRL, dRegVal);

	// clear PCS_LCTL.AN_ENABLE
	// set PCS_LCTL.FSD
	// set PCS_LCTL.force flow control
	// set speed PCS_LCTL.FSV = 10b
	// set PCS_LCTL.FDV
	// clear PCS_LCTL.AN_TIMEOUT_EN
	dRegVal = dReadReg825xx (PCS_LCTL);
	dRegVal &= ~(PCS_LCTL_AN_ENABLE + PCS_LCTL_FSV_100 + PCS_LCTL_AN_TIMEOUT);
	dRegVal |= PCS_LCTL_FSD + PCS_LCTL_FDV_FULL + PCS_LCTL_FSV_1000 + PCS_LCTL_FORCE_FCTRL;
	vWriteReg825xx (PCS_LCTL, dRegVal);
#endif

	// clear CONNSW.ENRGSRC
	dRegVal = dReadReg825xx (CONNSW);
	dRegVal &= ~CONNSW_ENRGSRC;
	vWriteReg825xx (CONNSW, dRegVal);

	// update device structure to allow test to run
	psDev->bType = IF_SERDES;
}

/*******************************************************************
 * vSwitchI350ToSerdes: change the i350 functional mode to Copper operation
 *
 * RETURNS: None
 */

static void vSwitchI350ToCopper
(
	DEVICE	*psDev
)	
{	
	UINT32	dRegVal;

#ifdef DEBUG
	sprintf (buffer, "%s\n", __FUNCTION__);
	vConsoleWrite (buffer);
#endif

#ifdef DEBUG
	vConsoleWrite ("\nRestoring COPPER interface to SERDES after SERDES->SERDES loopback test\n");
#endif
	// disable RCTL.RXEN
	dRegVal = dReadReg825xx (RCTL);
	dRegVal &= ~RCTL_EN;
	vWriteReg825xx (RCTL, dRegVal);

	// disable TCTL.TXEN
	dRegVal = dReadReg825xx (TCTL);
	dRegVal &= ~TCTL_EN;
	vWriteReg825xx (TCTL, dRegVal);

	// set mode CTRL_EXT.LINK_MODE = 00b
	dRegVal = dReadReg825xx (CTRL_EXT);
	dRegVal &= M__CTRLEXT_LINK;
	dRegVal |= CTRLEXT_LINK_COPPER;
	vWriteReg825xx (CTRL_EXT, dRegVal);

	// set CTRL.SLU
	dRegVal = dReadReg825xx (CTRL);
	dRegVal &= ~(CTRL_FRCSPD + CTRL_FRCDPLX);
	dRegVal |= CTRL_SLU;

	// reset PHY
	PhyReset (psDev);

	// restore device structure to original setting
	psDev->bType = IF_COPPER;
}

/*******************************************************************
 * x540GetSwFwSemaphore: acquire the x540 shared resource semaphores
 *
 * RETURNS: None
 */

static UINT32 x540GetSwFwSemaphore
(
	DEVICE * psDev
)
{
	UINT32 semaphore;
	UINT16 timeout;
	UINT16 i; 
	
	timeout = 300;

	/* Get SMBI software semaphore between device drivers first */
	for (i = 0; i  < timeout; ++i)
	{
		semaphore = dReadReg825xx(SWSM);
		if (!(semaphore & SWSM_SMBI))
		{
			break;
		}
		vDelay(10);
	}
	
	if (i == timeout)
	{
#ifdef DEBUG
		vConsoleWrite ("Unable to obtain SWSM_SMBI sempahore\n");
#endif
		return E__X540_SEMAPHORE;
	}
    
	/* Now get the semaphore between SW/FW through the SW_FW_SYNC_SEMPAHORE bit */
	for (i = 0; i  < timeout; ++i)
	{
		semaphore = dReadReg825xx(SW_FW_SYNC);
		if (!(semaphore & SW_FW_SYNC_SEMAPHORE))
		{
			return E__OK;
		}
		vDelay(10);
	}
	
#ifdef DEBUG
	vConsoleWrite ("Unable to obtain SW_FW_SYNC sempahore\n");
#endif

	return E__X540_SEMAPHORE;
}

/*******************************************************************
 * x540ReleaseSwFwSemaphore: release the x540 shared resource semaphores
 *
 * RETURNS: None
 */

static void x540ReleaseSwFwSemaphore
(
	DEVICE * psDev
)
{
	UINT32 semaphore;
	
	/* Release both semaphores by writing 0 to the bits REGSMP and SMBI */
	semaphore = dReadReg825xx(SW_FW_SYNC);
	semaphore &= ~SW_FW_SYNC_SEMAPHORE;
	vWriteReg825xx(SW_FW_SYNC, semaphore);
	
	semaphore = dReadReg825xx(SWSM);
	semaphore &= ~SWSM_SMBI;
	vWriteReg825xx(SWSM, semaphore);
}

/*******************************************************************
 * x540AcquireSwFwSync: get control of the SW_FW_SYNC register
 *
 * RETURNS: None
 */

static UINT32 x540AcquireSwFwSync
(
	DEVICE *	psDev,
	UINT16		mask
)
{
	int		timeout;
	int		i;
	UINT32	rt;
	UINT32	swFwSync;
	UINT32	swMask = mask;
	UINT32	fwMask = SW_FW_SYNC_FW_NVM;
	UINT32	hwMask = SW_FW_SYNC_HW_NVM;
	
	timeout = 10;
	for (i = 0; i < timeout; ++i)
	{
		/*
		 * SW NVM semaphore bit is used for access to all
		 * SW_FW_SYNC bits (not just NVM)
		 */
		rt = x540GetSwFwSemaphore(psDev);
		if (rt != E__OK)
		{
			return rt;
		}
		
		swFwSync = dReadReg825xx(SW_FW_SYNC);
		if (!(swFwSync & (swMask | fwMask | hwMask)))
		{
			swFwSync |= swMask;
			vWriteReg825xx(SW_FW_SYNC, swFwSync);
			x540ReleaseSwFwSemaphore(psDev);
			break;
		}
		else
		{
				/*
				 * Firmware currently using resource (fwmask),
				 * hardware currently using resource (hwmask),
				 * or other software thread currently using
				 * resource (swmask)
				 */
			x540ReleaseSwFwSemaphore(psDev);
			vDelay(5);
		}
	}
    
	/*
	 * If the resource is not released by the FW/HW the SW can assume that
	 * the FW/HW malfunctions. In that case the SW should sets the
	 * SW bit(s) of the requested resource(s) while ignoring the
	 * corresponding FW/HW bits in the SW_FW_SYNC register.
	 */
	if (i >= timeout)
	{
		swFwSync = dReadReg825xx(SW_FW_SYNC);
		if (swFwSync & (fwMask | hwMask))
		{
			rt = x540GetSwFwSemaphore(psDev);
			if (rt != E__OK)
			{
				return rt;
			}
			
			swFwSync |= swMask;
			vWriteReg825xx(SW_FW_SYNC, swFwSync);
			x540ReleaseSwFwSemaphore(psDev);
		}
	}
	
	vDelay(10);
	return E__OK;    
}

/*******************************************************************
 * x540ReleaseSwFwSync: release the SW_FW_SYNC register
 *
 * RETURNS: None
 */

static void x540ReleaseSwFwSync
(
	DEVICE *	psDev,
	UINT16		mask
)
{
	UINT32  swFwSync;
	
	x540GetSwFwSemaphore(psDev);
	swFwSync = dReadReg825xx(SW_FW_SYNC);
	swFwSync &= ~mask;
	vWriteReg825xx(SW_FW_SYNC, swFwSync);
	x540ReleaseSwFwSemaphore(psDev);
	vDelay(5);
}
