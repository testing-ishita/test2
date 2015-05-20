
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

/* eth.c - BIT test for Ethernet
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/eth.c,v 1.9 2015-03-10 15:42:15 mgostling Exp $
 * $Log: eth.c,v $
 * Revision 1.9  2015-03-10 15:42:15  mgostling
 * Tidied up and added some extra debug messages.
 *
 * Revision 1.8  2014-11-04 15:13:50  mgostling
 * Fixed two typos found during debugging dev->dev loopback for x540 controller in eth2.c
 *
 * Revision 1.7  2014-07-29 11:36:25  mgostling
 * Corrected parameters passed to SERVICE__BRD_SKIP_ETH_IF.
 * Corrected handling of device count in dFind825xx.
 *
 * Revision 1.6  2014-03-06 13:49:04  hchalla
 * Fixed problem checking instances 2,3 for 82580.
 *
 * Revision 1.5  2014-03-04 10:00:19  mgostling
 * Added support for i350AM4 Quad Ethernet controller
 *
 * Revision 1.4  2014-02-14 11:37:29  chippisley
 * Fixed Ethernet test function used to find the given instance of a 825xx device.
 * If the found instance was indicated to be skipped, it was previously also
 * skipping the remaining device instances of a multifunction device.
 *
 * Revision 1.3  2013/11/25 11:48:00  mgostling
 * Add support for TRB1x
 *
 * Revision 1.2  2013-10-08 07:13:38  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.30  2012/09/19 09:48:14  jthiru
 * Updated Display config test for vx91x boards without P0 connector, updated PCIe port width test for vx91x
 *
 * Revision 1.29  2012/09/14 15:38:45  jthiru
 * CUTE Support for VX91X - C5043B1110, fixes to existing bugs
 *
 * Revision 1.28  2012/04/12 13:43:24  hchalla
 * Added flag for ethernet to display the mac address while running t,999 test.
 *
 * Revision 1.27  2012/03/27 08:00:26  cdobson
 * Reduce the number of compiler warnings.
 *
 * Revision 1.26  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.25  2011/12/01 13:44:27  hchalla
 * Updated for VX 541 board.
 *
 * Revision 1.24  2011/11/14 17:34:09  hchalla
 * Updated for PP 531.
 *
 * Revision 1.23  2011/09/29 14:40:49  hchalla
 * Minor changes to tr 803 before intial release.
 *
 * Revision 1.22  2011/08/26 15:50:41  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.21  2011/08/02 16:58:17  hchalla
 * Added support for new ethernet controller 82579, 82580 copper and serdes.
 *
 * Revision 1.20  2011/06/13 12:33:38  hmuneer
 * no message
 *
 * Revision 1.19  2011/05/11 12:17:17  hchalla
 * TP 702 PBIT Related changes.
 *
 * Revision 1.18  2011/04/05 13:10:15  hchalla
 * Fixed problem with external loopback for XM 510.
 *
 * Revision 1.17  2011/03/22 13:53:35  hchalla
 * Added support for new controller 82580 four channels (2 copper, 2 serdes).
 *
 * Revision 1.16  2011/02/28 11:59:20  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.15  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.14  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.13  2010/11/04 17:50:19  hchalla
 * Added support for new controller 82577.
 *
 * Revision 1.12  2010/09/15 12:25:15  hchalla
 * Modified tests to  support TR 501 BIT/CUTE.
 *
 * Revision 1.11  2010/09/10 10:04:01  cdobson
 * Added controller to controller test.
 *
 * Revision 1.10  2010/06/23 11:01:56  hmuneer
 * CUTE V1.01
 *
 * Revision 1.9  2010/03/31 16:27:10  hmuneer
 * no message
 *
 * Revision 1.8  2010/02/26 10:38:24  jthiru
 * Debug prints in #ifdef DEBUG
 *
 * Revision 1.7  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.6  2009/07/23 15:17:29  jthiru
 * Added 82575EB support to EEPROM read
 *
 * Revision 1.5  2009/06/10 08:48:23  jthiru
 * Added routine to display Ethernet MAC address in config test case
 *
 * Revision 1.4  2009/06/03 08:29:50  cvsuser
 * Tidy up test names
 *
 * Revision 1.3  2009/05/21 13:35:27  cvsuser
 * Added support to execute test without parameters
 *
 * Revision 1.2  2009/05/15 11:02:39  jthiru
 * Added Eth EEPROM Write and Ext loopback test and enhancements
 *
 * Revision 1.1  2009/02/02 17:00:06  jthiru
 * Ethernet BIT Code
 *
 *
 */


/* includes */

#include <stdtypes.h>
#include <errors.h>
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <devices/eth.h>
#include <bit/hal.h>

#include <bit/board_service.h>

#define INCLUDE_STATS
#define INCLUDE_I82577
#define INCLUDE_I82577
//#define DEBUG 1
//#undef DEBUG


/* KMRN Mode Control */
#define HV_KMRN_MODE_CTRL       PHY_REG(769, 16)
#define HV_KMRN_MDIO_SLOW       0x0400

#define e1e_flush()     dReadReg825xx(STATUS)
#define BM_PHY_REG_PAGE(offset) \
        ((UINT16)(((offset) >> PHY_PAGE_SHIFT) & 0xFFFF))


#define BM_PHY_REG_NUM(offset) \
        ((UINT16)(((offset) & MAX_PHY_REG_ADDRESS) |\
         (((offset) >> (PHY_UPPER_SHIFT - PHY_PAGE_SHIFT)) &\
                ~MAX_PHY_REG_ADDRESS)))
#ifdef DEBUG
		static char buffer[128];
#endif

/*
 * Register access macros
 */
#define vWriteReg825xx(reg, value)	(*(volatile UINT32*)(psDev->pbReg + reg) = value)
#define dReadReg825xx(reg)			(*(volatile UINT32*)(psDev->pbReg + reg))

#define vWriteFlashReg(reg, value)	(*(volatile UINT32*)(psDev->pbFlashBase + reg) = value)

#define dWriteFlashReg(reg, value)	(*(volatile UINT32*)(psDev->pbFlashBase + reg) = value)
#define wWriteFlashReg(reg, value)   (*(volatile UINT16*)(psDev->pbFlashBase + reg) = value)
#define bWriteFlashReg(reg, value)   (*(volatile UINT8*)(psDev->pbFlashBase + reg) = value)

#define dReadFlashReg(reg)			(*(volatile UINT32*)(psDev->pbFlashBase + reg))

#define wReadFlashReg(reg)			(*(volatile UINT16*)(psDev->pbFlashBase + reg))
#define bReadFlashReg(reg)			(*(volatile UINT8*)(psDev->pbFlashBase + reg))

#define PHY_PAGE_SHIFT 5
#define PHY_REG(page, reg) (((page) << PHY_PAGE_SHIFT)|((reg) & MAX_PHY_REG_ADDRESS))


/* typedefs */
/* constants */
const UINT8 test_data[] = {0, 1, 2, 3, 4, 5};

/* locals */
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
	"10 BASE TX",
	"100 BASE TX",
	"1000 BASE TX"
};



/* globals */
static PTR48	p1, p2, p3, p4, p5, p6, p7;
static UINT32	dHandle1, dHandle2, dHandle3, dHandle4, dHandle5, dHandle6, dHandle7;
static UINT8	bByteBucket;	/* used for flushing results 		*/

/* externals */
extern void vDelay(UINT32 dMsDelay);
extern UINT32 dGetPhysPtr (UINT32 dBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);
extern void vConsoleWrite(char*	achMessage);
extern void vFreePtr(UINT32 dHandle);

/* forward declarations */
UINT32 dFind825xx(DEVICE* psDev, UINT8 bInstance,UINT8 bMode);
static UINT32	dInitPciDevice (DEVICE* psDev, UINT8 bIndex);
static UINT32	dEepromrwtest(DEVICE * psDev,UINT8 bType);
static void 	vGetLanAddr (DEVICE* psDev, UINT8*	pbLanAddr);
static void		vInitNvmParamsIch9Lan (DEVICE *psDev);
static void		vSerDesLoopbackMode(DEVICE *psDev);
static void		vSerDesChannelLoopbackMode(DEVICE *psDev);
static UINT32 dFlashCycleInitIch9Lan (DEVICE *psDev);
static UINT32 dFlashCycleIch9Lan(DEVICE *psDev, UINT32 dTimeOut);
static void vReadFlashDataIch9Lan (DEVICE *psDev, UINT32 dOffset, UINT8 bSize, UINT16 *pwdata);
static void vReadFlashWordIch9Lan (DEVICE *psDev, UINT32 dOffset, UINT16 *pwdata);
static void vReadNvmIch9Lan(DEVICE *psDev, UINT16 wOffset, UINT16 wWords, UINT16 *pwData);
#ifdef DEBUG
static UINT32 dGetPhyId(DEVICE *psDev);
static UINT32 dGetAutoRdDone(DEVICE *psDev);
#endif
static void WritePhyRegister_BM(DEVICE *psDev, UINT32 Offset, UINT16 Data);
static UINT8 vGetPhyAddrForPage_BM (DEVICE *psDev,UINT32 dPage, UINT32 Offset);
static UINT16 ReadPhyRegister_BM(DEVICE *psDev, UINT32 Offset, UINT16 *pData);
static void vPhyWakeupReg_BM(DEVICE *psDev, UINT32 Offset, UINT16 *pData,UINT8 read);
void vPhyWorkAround1(DEVICE *psDev);
void vDoWorkarounds(DEVICE *psDev);
void vDoK1Workaround(DEVICE *psDev);
/* static void vInit82577(DEVICE* psDev,UINT8   bSpeed); */
static void vClear8257xStatistics(DEVICE* psDev);
static void vDisplay8257xStats(DEVICE* psDev);
static void vDisplayPhyRegs (DEVICE* psDev);

#if 0
	static void vDisplay8257xDiagRegs(DEVICE* psDev);
#endif

static void vSwitchI350ToSerdes (DEVICE *psDev);
static void vSwitchI350ToCopper (DEVICE *psDev);


static UINT32 ReadPhyRegister_LV (DEVICE *psDev,UINT32 dOffset,UINT16 *pData, UINT8 flag);
static UINT32 access_phy_debug_regs_hv(DEVICE *psDev, UINT32 dOffset, UINT16 *pData, UINT8 read);
static UINT32 get_phy_addr_for_hv_page(UINT32 page);
static UINT32 WritePhyRegister_LV (DEVICE *psDev, UINT32 dOffset, UINT16 data, UINT8 locked);
static UINT32 write_kmrn_reg(DEVICE *psDev, UINT32 offset, UINT16 data);
static UINT32 read_kmrn_reg(DEVICE *psDev, UINT32 offset, UINT16 *data);
static UINT32 phy_has_link_generic(DEVICE *psDev, UINT32 iterations,
                               UINT32 usec_interval, UINT8 *success);
void config_collision_dist(DEVICE *psDev);
static UINT32 set_mdio_slow_mode_hv(DEVICE *psDev);
UINT32 copper_link_setup_82577(DEVICE *psDev);
static void  lan_init_done_ich8lan(DEVICE *psDev);
void  phy_force_speed_duplex_setup(DEVICE *psDev, UINT16 *phy_ctrl,UINT32 dDuplex,UINT32 dSpeed);
void power_up_phy_copper(DEVICE *psDev);
UINT32 sw_lcd_config_ich8lan (DEVICE *psDev);
UINT32 oem_bits_config_ich8lan (DEVICE *psDev, UINT8 d0_state );
static UINT32 check_reset_block_ich8lan(DEVICE *psDev);
UINT32 ReadPhyRegister_BM2(DEVICE *psDev, UINT32 offset, UINT16 *data);
UINT32 WritePhyRegister_BM2(DEVICE *psDev, UINT32 offset, UINT16 data);

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


	for (wIndex = 0; wIndex < wInterval; wIndex++)
		for (wCount = 0; wCount < 100; wCount++)
			bByteBucket++;

} /* vEepromDelay () */


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

} /* vRaiseClock () */


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

} /* vLowerClock () */


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

	switch (psDev->bController)
	{
	case CTRLT_82546EB:
	case CTRLT_82540EM:
	case CTRLT_82541PI:
		if ((dReadReg825xx (EECD) & EECD_PRES) == 0)
		{
#ifdef DEBUG
			sprintf(buffer, "SROM not present: %d", dReadReg825xx (EECD));
			vConsoleWrite(buffer);
#endif
			return E__EEPROM_ACCESS;
		}
	case CTRLT_82571EB:
	case CTRLT_82573L:
	case CTRLT_82574:
	case CTRLT_82580:
	case CTRLT_82580EB:
	case CTRLT_82576EB:
	case CTRLT_I350:

		vWriteReg825xx (EECD, dReadReg825xx (EECD) | EECD_REQ);

		while (wTimeout > 0)
		{
			if ((dReadReg825xx (EECD) & EECD_GNT) == EECD_GNT)
				return E__OK;
			wTimeout--;
			vDelay (1);
		}

		if (wTimeout == 0)
		{
#ifdef DEBUG
			vConsoleWrite("SROM grant timeout");
#endif
			return E__EEPROM_ACCESS;
		}
		break;

	case CTRLT_I210:
		break;
	}

	return E__OK;

} /* wEepromInit () */


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


	dEecdRegValue = dReadReg825xx (EECD);

	if ((psDev->bController != CTRLT_82571EB) ||
		(psDev->bController == CTRLT_82573L)  ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_82580EB))
	{
		dEecdRegValue |= EECD_CS;
	}
	else
	{
		dEecdRegValue &= ~EECD_CS;
	}

	dEecdRegValue &= ~EECD_DI;

	vWriteReg825xx (EECD, dEecdRegValue);

	vRaiseClock(psDev, &dEecdRegValue);
	vLowerClock(psDev, &dEecdRegValue);

	/* release EEPROM for hardware */

	if ((psDev->bController == CTRLT_82546EB) ||
	    (psDev->bController == CTRLT_82540EM) ||
	    (psDev->bController == CTRLT_82541PI) ||
	    (psDev->bController == CTRLT_82571EB) ||
	    (psDev->bController == CTRLT_82573L)  ||
		(psDev->bController == CTRLT_82574)   ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_82580EB) ||
		(psDev->bController == CTRLT_82576EB) ||
		(psDev->bController == CTRLT_I350))
	{
		dEecdRegValue = dReadReg825xx (EECD);
		dEecdRegValue &= ~EECD_REQ;
		vWriteReg825xx (EECD, dEecdRegValue);
	}

} /* vEepromCleanup () */


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

	for (wIndex = 0; wIndex < wCount; wIndex++)
	{
		wData = wData << 1;
		vRaiseClock (psDev, &dEecdRegValue);

		dEecdRegValue = dReadReg825xx (EECD);

		dEecdRegValue &= ~(EECD_DI);

		if (dEecdRegValue & EECD_DO)
			wData |= 1;

		vLowerClock (psDev, &dEecdRegValue);
	}

	return (wData);

} /* wShiftInBits () */


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

	if ((psDev->bController == CTRLT_82571EB) ||
		(psDev->bController == CTRLT_82573L)  ||
		(psDev->bController == CTRLT_82541PI) ||
		(psDev->bController == CTRLT_82575EB) ||
		(psDev->bController == CTRLT_82576EB) ||
		(psDev->bController == CTRLT_82574)   ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_82580EB) ||
		(psDev->bController == CTRLT_I350))
	{
		dEecdRegValue |= EECD_DO;
	}

	do {
		dEecdRegValue &= ~EECD_DI;

		if (wData & dMask)
			dEecdRegValue |= EECD_DI;

		vWriteReg825xx (EECD, dEecdRegValue);

		vEepromDelay (50);

		vRaiseClock (psDev, &dEecdRegValue);
		vLowerClock (psDev, &dEecdRegValue);

		dMask = dMask >> 1;

	} while (dMask != 0);

	dEecdRegValue &= ~EECD_DI;

	vWriteReg825xx (EECD, dEecdRegValue);

} /* vShiftOutBits () */


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

} /* vStandBy () */


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

	dEecdRegValue = dReadReg825xx (EECD);

	dEecdRegValue &= ~EECD_CK;
	vWriteReg825xx (EECD, dEecdRegValue);

	dEecdRegValue |= EECD_CS;

	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

#if 0
	dEecdRegValue |= EECD_CK;
	vWriteReg82543 (EECD, dEecdRegValue);
	vEepromDelay (5);
#endif

	dEecdRegValue &= ~EECD_CS;

	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

	dEecdRegValue &= ~EECD_CK;
	vWriteReg825xx (EECD, dEecdRegValue);
	vEepromDelay (5);

} /* v8257xStandBy () */


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


	for (wIndex = 0; wIndex < EEPROM_TIMEOUT; wIndex++)
	{
		vShiftOutBits(psDev, EEPROM_RDSR_OPCODE_SPI, 8);
		bStatus = wShiftInBits (psDev, 8);
		v825xxStandBy (psDev);

		if (!(bStatus & EEPROM_STATUS_RDY_SPI))
			break;

		vEepromDelay (5);

	}

    /* ATMEL SPI write time could vary from 0-20mSec on 3.3V devices (and
     * only 0-5mSec on 5V devices)
     */
	if(wIndex == EEPROM_TIMEOUT)
	{
#ifdef DEBUG
	 sprintf(buffer, "wWaitSpiEepromCmdDone %02x\n", bStatus);
	vConsoleWrite(buffer);
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

	if (dWaitSpiEepromCommandDone(psDev) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("dWriteSpiEepromByte SPI cmd done error\n");
#endif
		return (E__SPI_EEPROM_WRITE);
	}
	return (E__OK);
}


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

	vStandBy (psDev);

	for (wIndex = 0; wIndex < EEPROM_TIMEOUT; wIndex++)
	{
		dEecdRegValue = dReadReg825xx (EECD);

		if (dEecdRegValue & EECD_DO)
			return (E__OK);

		vEepromDelay (5);
	}

/*	if (psDev->bController == CTRLT_82544EI)
		return E__OK;*/	/* 82544 and ATMEL eeprom don't pass this function */

	return (E__EEPROM_TIMEOUT);

} /* dWaitEepromCommandDone () */


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

	if ((dTestStatus = wEepromInit (psDev)) != E__OK)
		return dTestStatus;

	if (psDev->bController == CTRLT_I210)
	{
#ifdef DEBUG
		vConsoleWrite ("Write ignored - not allowed on i210\n");
#endif
		return dTestStatus;
	}
	else if ((psDev->bController == CTRLT_82571EB) ||
			 (psDev->bController == CTRLT_82573L)  ||
			 (psDev->bController == CTRLT_82541PI) ||
			 (psDev->bController == CTRLT_82575EB) ||
			 (psDev->bController == CTRLT_82576EB) ||
			 (psDev->bController == CTRLT_82574)   ||
			 (psDev->bController == CTRLT_82580)   ||
			 (psDev->bController == CTRLT_82580EB) ||
			 (psDev->bController == CTRLT_I350))
	{
		/* spi interface */
		dTestStatus = dWriteSpiEepromByte (psDev, wReg*2, ((UINT8)wData & 0xff));
		if (dTestStatus == E__OK)
			dTestStatus = dWriteSpiEepromByte (psDev, wReg*2+1, (UINT8)(wData >> 8));
	}
	else
	{
		/* microwire interface */

		vWriteReg825xx (EECD, dReadReg825xx (EECD) | EECD_CS);

		vShiftOutBits (psDev, EEPROM_EWEN_OPCODE, EEPROM_W_OP_SIZE);
		vShiftOutBits (psDev, wReg, EEPROM_W_INDEX_SIZE);

		vStandBy (psDev);

		vShiftOutBits (psDev, EEPROM_WRITE_OPCODE, EEPROM_OPCODE_SIZE);
		vShiftOutBits (psDev, wReg, EEPROM_INDEX_SIZE);
		vShiftOutBits (psDev, wData, EEPROM_DATA_SIZE);

		if (dWaitEepromCommandDone (psDev) != E__OK)
			return (E__EEPROM_WRITE);

		vShiftOutBits (psDev, EEPROM_EWDS_OPCODE, EEPROM_W_OP_SIZE);
		vShiftOutBits (psDev, wReg, EEPROM_W_INDEX_SIZE);
	}

	vEepromCleanup (psDev);
	vDelay (1);

	return (dTestStatus);

} /* dWriteEepromWord () */

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

/*
	if (psDev->bController == CTRLT_I210)
	{
*/
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
/*
	}
*/

#ifdef DEBUG
	sprintf (buffer, "iNVM CSR: %04X=%08X Reset=%d\n",
						wAddress, dCSR, bReset);
	vConsoleWrite (buffer);
#endif
	return (dCSR);

} /* wReadInvmCSR () */

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

/*
	if (psDev->bController == CTRLT_I210)
	{
*/
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
/*
	}
*/

#ifdef DEBUG
	sprintf (buffer, "iNVM PHY: %02X=%04X Reset=%d\n", wAddress, wData, bReset);
	vConsoleWrite (buffer);
#endif

	return (wData);

} /* wReadInvmPhy () */
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

/*
	if (psDev->bController == CTRLT_I210)
	{
*/
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
/*
	}
*/

#ifdef DEBUG
	sprintf (buffer, "iNVM Word: %02X=%04X Reset=%d\n", wAddress, wData, bReset);
	vConsoleWrite (buffer);
#endif

	return (wData);

} /* wReadInvmWord () */


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
	UINT16 	wTestStatus;
	UINT32	dRegData;
	UINT32	dTmp;
	UINT32	dTimeout;

	dRegData = dReadReg825xx (EECD);
	if ((psDev->bController == CTRLT_I210) && ((dRegData & EECD_FLASH_IN_USE) == 0))
	{
		wData = wReadInvmWord (psDev, wReg);
	}
	else if ((psDev->bController == CTRLT_82571EB) ||
			 (psDev->bController == CTRLT_82573L)  ||
			 (psDev->bController == CTRLT_82541PI) ||
			 (psDev->bController == CTRLT_82575EB) ||
			 (psDev->bController == CTRLT_82576EB) ||
			 (psDev->bController == CTRLT_82574)   ||
			 (psDev->bController == CTRLT_82580)   ||
			 (psDev->bController == CTRLT_82580EB) ||
			 (psDev->bController == CTRLT_I350)    ||
			 (psDev->bController == CTRLT_I210))
	{
		dTmp = (UINT32) wReg;
		dTmp = (dTmp << 2) | 1;

		vWriteReg825xx (EERD, dTmp);

		dTimeout = 5;
		do
		{
			dTmp = dReadReg825xx (EERD);
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
	else
	{
		if ((wTestStatus = wEepromInit (psDev)) != E__OK)
			return 0;

		vWriteReg825xx (EECD, dReadReg825xx (EECD) | EECD_CS);

		vShiftOutBits (psDev, EEPROM_READ_OPCODE, EEPROM_OPCODE_SIZE);
		vShiftOutBits (psDev, wReg, EEPROM_INDEX_SIZE);

		wData = wShiftInBits (psDev, EEPROM_DATA_SIZE);

		vEepromCleanup (psDev);
	}

	return (wData);

} /* wReadEepromWord () */


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
	UINT32	Data,i;
	UINT32	Command;
	UINT16	wTimeout;

	/* read internal PHY */
	if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		Command = ((RegAddress << 16)& MDIC_REG_ADD) | ((psDev->bPhyAddress << 21) & MDIC_PHY_ADD) | MDIC_OP_READ;
		vWriteReg825xx (MDIC, Command);

		vDelay (10);

	    for (i = 0; i < (640 * 3); i++)
	    {
            vDelay (50);
            Data = dReadReg825xx (MDIC);
            if (Data & MDIC_READY)
                break;
        }
        if (!(Data & MDIC_READY))
        {
#ifdef DEBUG
			vConsoleWrite ("MDI Write did not complete\n");
#endif
            return 0xFFFF;
        }
        if (Data & MDIC_ERROR)
        {
#ifdef DEBUG
            vConsoleWrite ("MDI Error\n");
#endif
            return 0xFFFF;
        }

        return (UINT16)Data;
	}
	else
	{
		Command = ((RegAddress << 16) & MDIC_REG_ADD)
					| ((psDev->bPhyAddress << 21) & MDIC_PHY_ADD)
					| MDIC_OP_READ;

		vWriteReg825xx (MDIC, Command);

		vDelay(1);
		wTimeout = 50;

		Data = dReadReg825xx (MDIC);
		while (((Data & MDIC_READY) == 0) && (wTimeout-- > 0))
			vDelay (1);

		if (wTimeout != 0 && (Data & MDIC_ERROR) == 0)
			return (UINT16) (Data & MDIC_DATA);
		else
			return 0xFFFF;
	}

} /* ReadPhyRegister () */


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
	UINT32	regVal;

	/* write internal PHY */

/*
	vWriteReg825xx (MDIC, (Data & MDIC_DATA)
							| ((RegAddress << 16) & MDIC_REG_ADD)
							| ((psDev->bPhyAddress << 21) & MDIC_PHY_ADD)
							| MDIC_OP_WRITE);
*/

	regVal = (Data & MDIC_DATA)
				| ((RegAddress << 16) & MDIC_REG_ADD)
				| ((psDev->bPhyAddress << 21) & MDIC_PHY_ADD)
				| MDIC_OP_WRITE;

	vWriteReg825xx (MDIC, regVal);

	wTimeout = 50;
	status = dReadReg825xx (MDIC);
	while (((status & MDIC_READY) == 0)	&& (wTimeout-- > 0))
	{
		status = dReadReg825xx (MDIC);
		vDelay (1);
	}
} /* WritePhyRegister() */


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
	UINT16 wRegVal1, wRegVal2, wRegVal3, wRegVal4;

#ifdef DEBUG
	sprintf (buffer, "PHY registers (address = %02Xh)\n", psDev->bPhyAddress);
	vConsoleWrite (buffer);
#endif
	for (i = 0; i < 25; i+=4)
	{
		wRegVal1 = ReadPhyRegister (psDev, i);
		wRegVal2 = ReadPhyRegister (psDev, i+1);
		wRegVal3 = ReadPhyRegister (psDev, i+2);
		wRegVal4 = ReadPhyRegister (psDev, i+3);
#ifdef	DEBUG
		sprintf (buffer, "%3d  0x%04x 0x%04x 0x%04x 0x%04x\n", i,
				wRegVal1, wRegVal2, wRegVal3, wRegVal4);
		vConsoleWrite (buffer);
#endif
	}
	if((wRegVal1 == 0xFFFF) || (wRegVal2 == 0xFFFF)
		|| (wRegVal3 == 0xFFFF) || (wRegVal4 == 0xFFFF))
		return(E__PHYREADERR);
	else
		return(E__OK);

} /* dReadPhyRegs () */


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


	/* Enable receiver then transmitter to avoid loss of frames */

	if (dFunc & EN_RX)
	{
		dRegVal = dReadReg825xx (RCTL);		/* enable RX */
		vWriteReg825xx (RCTL, dRegVal | RCTL_EN);

		vDelay (10);				/* give RX time to prefetch buffers */
	}


	if (dFunc & EN_TX)
	{
		dRegVal = dReadReg825xx (TCTL);		/* enable TX */
		vWriteReg825xx (TCTL, dRegVal | TCTL_EN);
	}

} /* vEnableRxTx () */


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


	/* Disable transmitter then receiver to avoid loss of frames */

	dRegVal = dReadReg825xx (TCTL);		/* disable TX */
	vWriteReg825xx (TCTL, dRegVal & ~TCTL_EN);

	vDelay (10);				/* give TX time to complete */

	dRegVal = dReadReg825xx (RCTL);		/* disable RX */
	vWriteReg825xx (RCTL, dRegVal & ~RCTL_EN);

} /* vDisableTxRx () */


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

	RegData  = ReadPhyRegister (psDev, PHY_CTRL);
	RegData |= PHY_CTRL_RESET;
	WritePhyRegister (psDev, PHY_CTRL, RegData);

	for (i = 50; ((RegData & PHY_CTRL_RESET) != 0) && (i > 0); i--)
	{
		RegData = ReadPhyRegister (psDev, PHY_CTRL);
		vDelay (1);
	}

	if (i == 0)
	{
#ifdef DEBUG
		vConsoleWrite("Timeout waiting for PHY to reset.");
#endif
		return E__PHY_RESET_TIMEOUT;;
	}
	return E__OK;

} /* PhyReset () */


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

	CtrlReg  =  ReadPhyRegister (psDev, PHY_CTRL);
	CtrlReg &= ~M__PHY_CTRL_SPEED;


	if (psDev->bController == CTRLT_82574)
	{
		WritePhyRegister_BM(psDev,0x1d,0x03);
		WritePhyRegister_BM(psDev,0x1e,0x00);
	}
	else if (psDev->bController == CTRLT_82577 /*|| psDev->bController == CTRLT_82579*/ /*Hari*/)
	{
		WritePhyRegister_LV(psDev,22,0xff08,0);
		WritePhyRegister_LV(psDev,PHY_REG(769,16),0x2580,0);
		WritePhyRegister_LV(psDev,PHY_REG(769,27),0xffff,0);
	}
	else
	{
		CtrlReg |= PHY_CTRL_AUTO_NEG;
	}

	switch (bSpeed)
	{
	case SPEED_AUTO:
		/* advertise all speeds */
		RegData = ReadPhyRegister (psDev, PHY_1000);
		RegData |= PHY_1000_1000TXFD;
		WritePhyRegister (psDev, PHY_1000, RegData);

		RegData = ReadPhyRegister (psDev, PHY_ADVERT);
		RegData |= (PHY_ADVERT_10TXFD | PHY_ADVERT_100TXFD);
		WritePhyRegister (psDev, PHY_ADVERT, RegData);
		break;

	case SPEED_1000 :

		RegData = ReadPhyRegister (psDev, PHY_ADVERT);
		RegData &= ~(PHY_ADVERT_10TXFD
					 | PHY_ADVERT_10TXHD
					 |PHY_ADVERT_100TXFD
					 | PHY_ADVERT_100TXHD);

		WritePhyRegister (psDev, PHY_ADVERT, RegData);

		CtrlReg |= PHY_CTRL_SPEED_1000;
		CtrlReg &= ~0x1000;
		WritePhyRegister (psDev, PHY_CTRL, CtrlReg);

		if (psDev->bController == CTRLT_82576EB  )
		{
		    WritePhyRegister(psDev,0x04,0x01E1);
	    }
		else if ((psDev->bController == CTRLT_82577) ||
				 (psDev->bController == CTRLT_82579))
		{
			WritePhyRegister(psDev,0x00,0x140);
			vDelay(10);
		}
		else if (psDev->bController == CTRLT_82574)
		{
			WritePhyRegister(psDev,0x00,0x1340);
			vDelay(10);
		}
		else if (psDev->bController == CTRLT_82580)
		{
				/*WritePhyRegister(psDev,0x00,0x0140);*/
				WritePhyRegister(psDev,0x00,0x1340);
				vDelay(50);
		}
		else if (psDev->bController == CTRLT_82575EB)
		{
				WritePhyRegister(psDev,0x00,0x0140);
				vDelay(50);
		}
		else
		{
		   WritePhyRegister(psDev,0x00,0x1340);
		   WritePhyRegister(psDev,0x04,0x01E1);
		   WritePhyRegister(psDev,0x09,0x0200);
	    }
		break;

	case SPEED_100  :
		/* advertise 100 TX only */
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
		if (psDev->bController == CTRLT_82577 ||
			psDev->bController == CTRLT_82579)
		{
		     WritePhyRegister_LV(psDev,0x00,0xA100,0);
		     vDelay(10);
		     WritePhyRegister_LV(psDev,0x00,0x2100,0);
		     vDelay(10);
		}
		else if (psDev->bController == CTRLT_82580 || psDev->bController == CTRLT_82574)
		{
			 WritePhyRegister(psDev,0x00,0xA100);
			 vDelay(10);
			 WritePhyRegister(psDev,0x00,0x2100);
			 vDelay(10);
		}
		break;

	case SPEED_10   : /* identical actions ... drop thru' ... */
	default         :
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
		if ((psDev->bController == CTRLT_82577) ||
			(psDev->bController == CTRLT_82579))
		{
			WritePhyRegister_LV(psDev,0x00,0x8100,0);
			vDelay(10);
			WritePhyRegister_LV(psDev,0x00,0x0100,0);
			vDelay(10);
		}
		else if ((psDev->bController == CTRLT_82580) ||
				 (psDev->bController == CTRLT_82574))
		{
			WritePhyRegister(psDev,0x00,0x8100);
			vDelay(10);
			WritePhyRegister(psDev,0x00,0x0100);
			vDelay(10);
		}

		break;
	}

	if (psDev->bController == CTRLT_82567)
	    vPhyWorkAround1(psDev);


	if (bSpeed != SPEED_1000)
	{
		/* restart auto neg */
		if ((psDev->bController == CTRLT_82577) &&
			(psDev->bController != CTRLT_82580) &&
			(psDev->bController != CTRLT_82579))
		{
		    /* force full duplex */
		    CtrlReg = ReadPhyRegister(psDev, PHY_CTRL);
		    CtrlReg |= PHY_CTRL_DUPLEX;
			CtrlReg |= PHY_CTRL_RESTART_AUTO;
			WritePhyRegister (psDev, PHY_CTRL, CtrlReg); /*SRW- fixed CtrlReg */
	   	}
	}

	if ((bSpeed == SPEED_1000) &&
		((psDev->bController == CTRLT_82576LM ||
			psDev->bController == CTRLT_82576EB)))
	{
		    WritePhyRegister(psDev,0x00,0x4140);
	}

} /* PhySpeed () */


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
	UINT32	dRegVal = 0;
	UINT16  CtrlReg = 0;
	UINT16 wTempReg = 0;

#ifdef DEBUG
	char buffer [80];
#endif


	if ((psDev->bController == CTRLT_I350) ||
		(psDev->bController == CTRLT_I210))
	{
		vWriteReg825xx(CTRL_EXT, 0);
	}

	if (psDev->bController == CTRLT_82567)
	{
        CtrlReg = ReadPhyRegister(psDev,PHY_CTRL);
        CtrlReg |= PHY_CTRL_LOOPBACK;
        WritePhyRegister(psDev,PHY_CTRL,CtrlReg);
        WritePhyRegister_BM(psDev,0x1F37,0x3F1C);
	}
	else if ((psDev->bController == CTRLT_82576EB) && (bLbMode == LB_INTERNAL))
	{
       CtrlReg = ReadPhyRegister(psDev,PHY_CTRL);
        CtrlReg |= PHY_CTRL_LOOPBACK;
        WritePhyRegister(psDev,PHY_CTRL,CtrlReg);
	}
	else if ((psDev->bController == CTRLT_82577) && (bLbMode == LB_INTERNAL))
	{
		CtrlReg = ReadPhyRegister(psDev,PHY_CTRL);
        CtrlReg |= PHY_CTRL_LOOPBACK;

		WritePhyRegister_BM(psDev,PHY_CTRL, 0x4140);
		dRegVal = dReadReg825xx (RCTL) & M__RCTL_LBM;
        vWriteReg825xx(RCTL, dRegVal);
	}
	else if ((psDev->bController == CTRLT_82579) && (bLbMode == LB_INTERNAL))
	{
		/* Disable PHY energy detect power down */
		ReadPhyRegister_LV(psDev, PHY_REG(0, 21), &wTempReg,0);
		WritePhyRegister_LV(psDev, PHY_REG(0, 21), wTempReg & ~(1 << 3),0);

		/* Disable full chip energy detect */
		ReadPhyRegister_LV(psDev, PHY_REG(776, 18), &wTempReg,0);
		WritePhyRegister_LV(psDev, PHY_REG(776, 18), (wTempReg | 1),0);

		CtrlReg = ReadPhyRegister(psDev,PHY_CTRL);
        CtrlReg |= PHY_CTRL_LOOPBACK;

        wTempReg = ReadPhyRegister(psDev,19);
        WritePhyRegister(psDev,19,(wTempReg | 0x8001));

		WritePhyRegister(psDev,PHY_CTRL, 0x4140);

		dRegVal = dReadReg825xx (RCTL) & M__RCTL_LBM;
        vWriteReg825xx(RCTL, dRegVal);
	}
	else if ((bLbMode == LB_INTERNAL) &&
			 ((psDev->bController == CTRLT_82580) ||
			  (psDev->bController == CTRLT_I350)  ||
			  (psDev->bController == CTRLT_I210)))
	{
		if (dSpeed == SPEED_1000)
		{
#ifdef DEBUG
			sprintf (buffer, "Testing %s 1000MB/s\n", psDev->achCtrlName);
		  	vConsoleWrite(buffer);
#endif
			WritePhyRegister(psDev,PHY_CTRL, 0x4140);
			vDelay(50);
		}
		else if (dSpeed == SPEED_100)
		{
#ifdef DEBUG
			sprintf (buffer, "Testing %s 100MB/s\n", psDev->achCtrlName);
		  	vConsoleWrite(buffer);
#endif
			WritePhyRegister(psDev,PHY_CTRL, 0x6100);
			vDelay(50);
		}
		else if (dSpeed == SPEED_10)
		{
#ifdef DEBUG
			sprintf (buffer, "Testing %s 10MB/s\n", psDev->achCtrlName);
		  	vConsoleWrite(buffer);
#endif
			WritePhyRegister(psDev,PHY_CTRL, 0x4100);
			vDelay(50);
		}
	}
	else if ((psDev->bController == CTRLT_82577) && (bLbMode == LB_EXTERNAL) && (dSpeed == SPEED_1000))
	{
		vDelay(10);
		ReadPhyRegister_LV(psDev,19,&wTempReg,0);
		vDelay(10);
		WritePhyRegister_LV(psDev,19,(wTempReg | (1<<7)),0);
		vDelay(10);
		WritePhyRegister_LV(psDev,PHY_REG(770,17),0x9004,0);
		vDelay(10);
	}
	else if ((psDev->bController == CTRLT_82579) && (bLbMode == LB_EXTERNAL) && (dSpeed == SPEED_1000))
	{
		vDelay(10);
		ReadPhyRegister_LV(psDev,19,&wTempReg,0);
		vDelay(10);
		WritePhyRegister_LV(psDev,19,(wTempReg | (1<<7)),0);
		vDelay(10);
		WritePhyRegister_LV(psDev,PHY_REG(770,17),0x9004,0);

 	   /* Disable PHY energy detect power down */
		ReadPhyRegister_LV(psDev, PHY_REG(0, 21), &wTempReg, 0);
		WritePhyRegister_LV(psDev, PHY_REG(0, 21), wTempReg & ~(1 << 3),0);

		/* Disable full chip energy detect */
		ReadPhyRegister_LV(psDev, PHY_REG(776, 18), &wTempReg,0);
		WritePhyRegister_LV(psDev, PHY_REG(776, 18), (wTempReg | 1),0);

 		vDelay(10);
	}
	else if ((psDev->bController == CTRLT_82575EB) && (bLbMode == LB_EXTERNAL))
	{
	 	if (dSpeed == SPEED_1000)
		{
			vConsoleWrite("Testing 1000MB/s\n");
			CtrlReg = ReadPhyRegister(psDev,PHY_CTRL);
			CtrlReg |= PHY_CTRL_LOOPBACK;
			WritePhyRegister(psDev,PHY_CTRL, 0x0140);
			vDelay(50);
			WritePhyRegister(psDev,0x09, 0x1b00);
			vDelay(50);
			WritePhyRegister(psDev,0x12, 0x1610);
			vDelay(50);
			WritePhyRegister_BM(psDev,0x1F37,0x3F1C);
		}
		else if (dSpeed == SPEED_100)
		{
			vConsoleWrite("Testing 100MB/s\n");
			CtrlReg = ReadPhyRegister(psDev,PHY_CTRL);
			CtrlReg |= PHY_CTRL_LOOPBACK;
			WritePhyRegister(psDev,PHY_CTRL, 0xA100);
			vDelay(50);
			WritePhyRegister(psDev,PHY_CTRL, 0x2100);
			vDelay(50);
		}
		else if (dSpeed == SPEED_10)
		{
			vConsoleWrite("Testing 10MB/s\n");
			CtrlReg = ReadPhyRegister(psDev,PHY_CTRL);
			CtrlReg |= PHY_CTRL_LOOPBACK;
			WritePhyRegister(psDev,PHY_CTRL, 0x8100);
			vDelay(50);
			WritePhyRegister(psDev,PHY_CTRL, 0x0100);
			vDelay(50);
		}
	}
	else if ((psDev->bController == CTRLT_82580) && (bLbMode == LB_EXTERNAL) && (dSpeed == SPEED_1000))
	{
		vDelay(10);
		wTempReg = ReadPhyRegister(psDev, 19);
		vDelay(100);
		WritePhyRegister(psDev, 19 ,(wTempReg | (1<<7)));
		vDelay(100);
	}
	else if ((psDev->bController == CTRLT_I350) &&(bLbMode == LB_EXTERNAL) && (dSpeed == SPEED_1000))
	{
		vDelay(10);
		wTempReg = ReadPhyRegister(psDev, 19);
		vDelay(100);
		WritePhyRegister(psDev, 19, (wTempReg | (1<<7)));		// set external cable mode
		vDelay(100);
		wTempReg = ReadPhyRegister(psDev, PHY_CTRL);
		wTempReg |= PHY_CTRL_RESTART_AUTO;
		WritePhyRegister(psDev,PHY_CTRL, wTempReg);
		vDelay(100);
	}
	else if ((psDev->bController == CTRLT_82574) && (bLbMode == LB_EXTERNAL) && (dSpeed == SPEED_1000))
	{
		ReadPhyRegister_BM2(psDev, GG82563_REG(6, 16),&wTempReg);
		vDelay(10);
		WritePhyRegister_BM2(psDev, GG82563_REG(6, 16), (wTempReg | (1 << 5)));
		vDelay(10);
		PhyReset(psDev);
		vDelay(100);
	}
	else if ((psDev->bController == CTRLT_82571EB) && (bLbMode == LB_EXTERNAL) && (dSpeed == SPEED_1000))
	{
#ifdef DEBUG
		vConsoleWrite("\n82571EB GB EXT Loopback\n");
#endif

		PhyReset(psDev);

		vWriteReg825xx(0x0000, 0x00000040); /* Set link up */
		vDelay(50);

		vWriteReg825xx( 0x00100,
					    0x00000002 |
						0x00000004 |
						0x00000008 |
						0x00000010 |
						0x00000020 |
						0x00008000);

		vWriteReg825xx(0x00018,
						0x00000010 |
						0x00000040 |
						0x00000080 |
						0x00000100 |
						0x00000400 |
						0x00000800);
		vDelay(50);

		/* This sequence tunes the PHY's DSP. */
		WritePhyRegister(psDev, PHY_CTRL, 0x0140);
		vDelay(10);

		WritePhyRegister(psDev,0x9, 0x1A00);
		WritePhyRegister(psDev,0x12, 0x0C10);
		WritePhyRegister(psDev,0x12, 0x1C10);
		WritePhyRegister(psDev,0x1F35, 0x0065);
		WritePhyRegister(psDev,0x1837, 0x3F7C);
		WritePhyRegister(psDev,0x1437, 0x3FDC);
		WritePhyRegister(psDev,0x1237, 0x3F7C);
		WritePhyRegister(psDev,0x1137, 0x3FDC);

		vDelay(50);
	}
	else if ((psDev->bController == CTRLT_82576EB) && (bLbMode == LB_INTERNAL))
	{
        CtrlReg = ReadPhyRegister(psDev,PHY_CTRL);
        CtrlReg |= PHY_CTRL_LOOPBACK;
        WritePhyRegister(psDev,PHY_CTRL,CtrlReg);
	}
	else if ((psDev->bController == CTRLT_I210) && (bLbMode == LB_EXTERNAL))
	{
#ifdef DEBUG
		sprintf (buffer, "\nvSetLoopbackMode 12 speed=%d\n", dSpeed);
		vConsoleWrite(buffer);
#endif
		wTempReg = PHY_CTRL_DUPLEX + PHY_CTRL_AUTO_NEG;
		switch (dSpeed)
		{
			case SPEED_10:
				break;

			case SPEED_100:
				wTempReg += PHY_CTRL_SPEED_100;
				break;
			case SPEED_1000:
				wTempReg += PHY_CTRL_SPEED_1000;
				break;
		}
		WritePhyRegister(psDev, PHY_CTRL, wTempReg);

		WritePhyRegister(psDev, PHY_REG(6, 18) , 0x08);		// enable Stub Test

		wTempReg |= PHY_CTRL_RESTART_AUTO;
		WritePhyRegister(psDev, PHY_CTRL, wTempReg);

		PhyReset(psDev);
		vDelay(8000);
	}
	else
	{
		dRegVal = dReadReg825xx (RCTL) & M__RCTL_LBM;
		switch (bLbMode)
		{
			case LB_INTERNAL :
			{
				if (psDev->bController == CTRLT_82544EI)
				{
					dRegVal |= RCTL_LBM_XCVR;
					wTempReg = ReadPhyRegister (psDev, PHY_CTRL);
					WritePhyRegister (psDev, PHY_CTRL, wTempReg  | PHY_CTRL_LOOPBACK);
				}
				dRegVal |= RCTL_LBM_TBI;
			}
			break;

			case LB_LOCAL    :
		/*		dRegVal |= RCTL_LBM_GMII; */
				break;

			default :
				break;
		}

		vWriteReg825xx(RCTL, dRegVal);
	}

} /* vSetLoopbackMode () */


/*******************************************************************
 * vInitRxDescr: configure the receiver descriptor Registers
 *
 * RETURNS: none
 */

static void vInitRxDescr (DEVICE *psDev, UINT16 wDescrCount)
{
	vWriteReg825xx (RDBAL, psDev->dRxDescAddr);
	vWriteReg825xx (RDBAH, 0x00000000);
	vWriteReg825xx (RDLEN, psDev->dRxDescSize);
	vWriteReg825xx (RDH, 0x00000000);
	vWriteReg825xx (RDT, wDescrCount);
} /* vInitRxDescr () */


/*******************************************************************
 * vInitTxDescr: configure the transmitter descriptor Registers
 *
 * RETURNS: none
 */

static void vInitTxDescr (DEVICE *psDev, UINT16 wDescrCount)
{
	vWriteReg825xx (TDBAL, psDev->dTxDescAddr);
	vWriteReg825xx (TDBAH, 0x00000000);
	vWriteReg825xx (TDLEN, psDev->dTxDescSize);
	vWriteReg825xx (TDH, 0x00000000);
	vWriteReg825xx (TDT, wDescrCount);
} /* vInitTxDescr () */


/*****************************************************************************
 * vGetEthMacAddr: Return station address
 *
 * RETURNS : N/A
 */

void vGetEthMacAddr
(
	UINT8  bInstance,
	UINT8* pbAddress,
	UINT8 bMode

)
{
	UINT8	bIndex;
	DEVICE	sDevice;

	if (dFind825xx (&sDevice, bInstance,bMode) == PCI_SUCCESS)
	{
		dInitPciDevice (&sDevice, 0);

									/*CTRLT_82567LM*/
		if ((sDevice.bController == CTRLT_82567) ||
			(sDevice.bController == CTRLT_82577) ||
			(sDevice.bController == CTRLT_82579) )
		{
		   vInitNvmParamsIch9Lan(&sDevice);
	    }

		vGetLanAddr (&sDevice, pbAddress);
	}
	else
		for (bIndex = 0; bIndex < 6; bIndex++)
			pbAddress [bIndex] = 0;
}


/*******************************************************************
 * vGetLanAddr: read the LAN address from the serial EEPROM
 *
 * CNDM: Changed ordering of the bytes. IA bytes 1 and 2 are the MS
 *       part of the Ethernet address, IA Bytes 3 to 6 are the LS part
 *       of the Ethernet address.
 *
 * RETURNS: none
 */

static void vGetLanAddr
(
	DEVICE*	psDev,			/* pointer to device data structure */
	UINT8*	pbLanAddr		/* just what is says */
)
{
	UINT16 wTemp;
	UINT16 awFlashEeprom [0x5F];

#ifdef DEBUG
	char buffer[0x64];
#endif

	wTemp =  wReadEepromWord (psDev, EEPROM_IA2_1);
	pbLanAddr [0] = (UINT8) wTemp;
	pbLanAddr [1] = (UINT8)(wTemp >> 8);
	wTemp =  wReadEepromWord (psDev, EEPROM_IA4_3);
	pbLanAddr [2] = (UINT8) wTemp;
	pbLanAddr [3] = (UINT8)(wTemp >> 8);
	wTemp =  wReadEepromWord (psDev, EEPROM_IA6_5);
	pbLanAddr [4] = (UINT8) wTemp;
	pbLanAddr [5] = (UINT8)(wTemp >> 8);

	/* if EEPROM is shared, increment second LAN address */

	if ( ((psDev->bController == CTRLT_82546EB) ||
		 (psDev->bController == CTRLT_82571EB) ||
		 (psDev->bController == CTRLT_82575EB) ||
		 (psDev->bController == CTRLT_82576EB) ||
		 (psDev->bController == CTRLT_82574)) && (psDev->bFunc == 1) )
	{
		pbLanAddr [5]++;
		if (pbLanAddr [5] == 0)
		{
			pbLanAddr [4]++;
			if (pbLanAddr [4] == 0)
				pbLanAddr [3]++;
		}
	}
	else if ( (psDev->bController == CTRLT_82580) ||
		 		(psDev->bController == CTRLT_82580EB) )
	{
		/* Multiport adapters share the same EEPROM, hence will have the same address */
		/* We adjust the last octet using the PCI function ID */
	
		pbLanAddr[5] = (UINT8) (pbLanAddr[5] +  (( (dReadReg825xx( STATUS )) & 0xc) >> 2));
	}
	else if (psDev->bController == CTRLT_I350)
 	{
		if  (psDev->bFunc == 1)
		{
			wTemp =  wReadEepromWord (psDev, 0x80);
			pbLanAddr [0] = (UINT8) wTemp;
			pbLanAddr [1] = (UINT8)(wTemp >> 8);
			wTemp =  wReadEepromWord (psDev, 0x81);
			pbLanAddr [2] = (UINT8) wTemp;
			pbLanAddr [3] = (UINT8)(wTemp >> 8);
			wTemp =  wReadEepromWord (psDev, 0x82);
			pbLanAddr [4] = (UINT8) wTemp;
			pbLanAddr [5] = (UINT8)(wTemp >> 8);
		}
		if  (psDev->bFunc == 2)
		{
			wTemp =  wReadEepromWord (psDev, 0xC0);
			pbLanAddr [0] = (UINT8) wTemp;
			pbLanAddr [1] = (UINT8)(wTemp >> 8);
			wTemp =  wReadEepromWord (psDev, 0xC1);
			pbLanAddr [2] = (UINT8) wTemp;
			pbLanAddr [3] = (UINT8)(wTemp >> 8);
			wTemp =  wReadEepromWord (psDev, 0xC2);
			pbLanAddr [4] = (UINT8) wTemp;
			pbLanAddr [5] = (UINT8)(wTemp >> 8);
		}
		if  (psDev->bFunc == 3)
		{
			wTemp =  wReadEepromWord (psDev, 0x100);
			pbLanAddr [0] = (UINT8) wTemp;
			pbLanAddr [1] = (UINT8)(wTemp >> 8);
			wTemp =  wReadEepromWord (psDev, 0x101);
			pbLanAddr [2] = (UINT8) wTemp;
			pbLanAddr [3] = (UINT8)(wTemp >> 8);
			wTemp =  wReadEepromWord (psDev, 0x102);
			pbLanAddr [4] = (UINT8) wTemp;
			pbLanAddr [5] = (UINT8)(wTemp >> 8);
			}
	}
/*
	else if ((psDev->bController == CTRLT_82580EB) && (psDev->bFunc == 2))
	{
		wTemp =  wReadEepromWord (psDev, 0xC0);
		pbLanAddr [0] = (UINT8) wTemp;
		pbLanAddr [1] = (UINT8)(wTemp >> 8);
		wTemp =  wReadEepromWord (psDev, 0xC1);
		pbLanAddr [2] = (UINT8) wTemp;
		pbLanAddr [3] = (UINT8)(wTemp >> 8);
		wTemp =  wReadEepromWord (psDev, 0xC2);
		pbLanAddr [4] = (UINT8) wTemp;
		pbLanAddr [5] = (UINT8)(wTemp >> 8);

	}
	else if ((psDev->bController == CTRLT_82580EB) && (psDev->bFunc == 3))
	{
		wTemp =  wReadEepromWord (psDev, 0x100);
		pbLanAddr [0] = (UINT8) wTemp;
		pbLanAddr [1] = (UINT8)(wTemp >> 8);
		wTemp =  wReadEepromWord (psDev, 0x101);
		pbLanAddr [2] = (UINT8) wTemp;
		pbLanAddr [3] = (UINT8)(wTemp >> 8);
		wTemp =  wReadEepromWord (psDev, 0x102);
		pbLanAddr [4] = (UINT8) wTemp;
		pbLanAddr [5] = (UINT8)(wTemp >> 8);
	}
*/
	else if ((psDev->bController == CTRLT_82567) ||
			 (psDev->bController == CTRLT_82577) ||
			 (psDev->bController == CTRLT_82579))
    {
	    vReadNvmIch9Lan(psDev,0x00,EEPROM_SIZE,(UINT16*)&awFlashEeprom);
	    pbLanAddr [0] = awFlashEeprom[0];
	    pbLanAddr [1] = awFlashEeprom[0] >> 8;
	    pbLanAddr [2] = awFlashEeprom[1];
	    pbLanAddr [3] = awFlashEeprom[1] >> 8;
	    pbLanAddr [4] = awFlashEeprom[2];
	    pbLanAddr [5] = awFlashEeprom[2] >> 8;
    }


#ifdef DEBUG
	sprintf (buffer, "\nLAN Address  = %02x:%02x:%02x:%02x:%02x:%02x\n", pbLanAddr [0], pbLanAddr [1], pbLanAddr [2],
			                                             pbLanAddr [3], pbLanAddr [4], pbLanAddr [5] );
	vConsoleWrite(buffer);
#endif

} /* vGetLanAddr () */


/*******************************************************************
 * vInit825xx: initialize an instance of the 8257x
 *
 * RETURNS: none
 */

static void vInit825xx
(
	DEVICE* psDev,		/* pointer to device data structure */
    UINT8   bSpeed		/* required link speed */
)
{
	UINT8	abLanAddr[6];
	UINT32	dRegVal = 0;
	PCI_PFA pfa;
	UINT32 dTarc = 0;
#ifdef DEBUG
	UINT32 dRet = 0;
    UINT32 dPhyCtrl = 0;
	UINT32 dExtCnfCtrl = 0;
	char buffer[0x64];
#endif

	/*
	 * Initialize the registers
	 */

	/* Set control register to select speed and full-duplex mode */
	dRegVal = dReadReg825xx (CTRL);
	dRegVal = 	CTRL_FD		| 	/* always full duplex */
				CTRL_FRCDPLX;	/* force duplex - we select it */

	switch (bSpeed)
	{
	case SPEED_AUTO:
		dRegVal |= CTRL_ASDE;		/* auto detect speed */
		break;
	case SPEED_1000 :
		dRegVal |= CTRL_SPEED1000;
		break;
	case SPEED_100  :
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

	/*if ((psDev->bController == CTRLT_82580))
	{
		dRegVal |= (CTRL_RFCE | CTRL_TFCE);
	}*/

	vWriteReg825xx (CTRL, dRegVal);

	if (psDev->bController == CTRLT_82576EB && psDev->bType == IF_SERDES)
    {
    	//remove SerDes LoopBack
	    vWriteReg825xx(SERDESCTL,SCTL_DISABLE_SERDES_LOOPBACK);
	   /* Unset switch control to serdes energy detect */
	   dRegVal = dReadReg825xx(CONNSW);
	   dRegVal &= ~CONNSW_ENRGSRC;
	   vWriteReg825xx(CONNSW, dRegVal);
	   vDelay(10);

	   /* Set PCS register for forced speed */
	   dRegVal = dReadReg825xx(PCS_LCTL);
	   dRegVal &= ~(PCS_LCTL_AN_ENABLE|PCS_LCTL_AN_RESTART|PCS_LCTL_AN_TIMEOUT);     /* Disable Autoneg*/

	   dRegVal |=  PCS_LCTL_FLV_LINK_UP |   /* Force link up */
	            PCS_LCTL_FSV_1000 |      /* Force 1000    */
	            PCS_LCTL_FDV_FULL |      /* SerDes Full duplex */
	            PCS_LCTL_FSD |           /* Force Speed */
	            PCS_LCTL_FORCE_LINK;     /* Force Link */

	   vWriteReg825xx(PCS_LCTL, dRegVal);
   }


	if (psDev->bController == CTRLT_82571EB && psDev->bType == IF_SERDES)
	{
	    //remove SerDes LoopBack
		vWriteReg825xx(SERDESCTL,SCTL_DISABLE_SERDES_LOOPBACK);

		/* Set TBI mode */
		dRegVal = dReadReg825xx (CTRL_EXT) & M__CTRLEXT_LINK;
		vWriteReg825xx (CTRL_EXT, dRegVal | CTRLEXT_LINK_TBI);
	 }

	if ((psDev->bController == CTRLT_82573L)  ||
		(psDev->bController == CTRLT_82575EB) ||
		(psDev->bController == CTRLT_82574)   ||
		(psDev->bController == CTRLT_82577)   ||
		(psDev->bController == CTRLT_82579)   ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_82580EB) ||
		(psDev->bController == CTRLT_I350)    ||
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
		vWriteReg825xx (CTRL_EXT, dReadReg825xx (CTRL_EXT) | CTRLEXT_SPDBYPS);



	if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		vWriteReg825xx (CTRL_EXT, dReadReg825xx (CTRL_EXT) & ~(0x100000));
		vWriteReg825xx (CTRL_EXT, dReadReg825xx (CTRL_EXT) | ~(0xFBFFFFFF));
	}

    /* Initialize the flow control registers to recommended values */
    if ((psDev->bController != CTRLT_82567) &&
		(psDev->bController != CTRLT_82577) &&
		(psDev->bController != CTRLT_82579))
    {
		vWriteReg825xx (FCAL, 0x00C28001);
		vWriteReg825xx (FCAH, 0x00000100);
		vWriteReg825xx (FCT, 0x00008808);
		vWriteReg825xx (FCTTV, 0);

		/* Initialize VLAN type to recommended value and disable VLAN */
		vWriteReg825xx (VET, 0x00008100);
    }

	/* change back to default ... CNDM */
	/* Set packet buffer allocation for 50:50 between Tx and Rx */
	pfa = PCI_MAKE_PFA(psDev->bBus, psDev->bDev, psDev->bFunc);

	dRegVal = dPciReadReg (pfa, PCI_VID, PCI_DWORD);

	if ((psDev->bController == CTRLT_82571EB) ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_82580EB))
		vWriteReg825xx (PBA, 0x00000020); 	/* set Rx to 32k, Tx is */

	if (psDev->bController == CTRLT_82576EB)
    {
		vWriteReg825xx (RXPBS, 0x00000020);
		vWriteReg825xx (TXPBS, 0x00000020); /* set Rx to 32k, Tx is */
    }

	/* auto set from this */
	if (psDev->bController == CTRLT_82573L)
		vWriteReg825xx (PBA, 0x0000000C); 	/* set Rx to 12k?, Tx is */
											/* auto set from this */

	if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82574))
		vWriteReg825xx (PBA, 0x0000000C);

	if ((psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		vWriteReg825xx (PBA, 0x0000001AL);
		/*vWriteReg825xx (0x1008,0x0000003FL);*/ /*Hari*/
	}

	if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579) )
	{
#ifdef DEBUG
		dExtCnfCtrl = dReadReg825xx(ICH9_EXTCNF_CTRL);
		dPhyCtrl = dReadReg825xx(ICH9_PHY_CTRL);

		sprintf (buffer, "PHY CTRL %x of 82567\n",
							(int)dPhyCtrl);
		vConsoleWrite (buffer);
#endif
		if ((psDev->bController == CTRLT_82577) || (psDev->bController == CTRLT_82579))
			vWriteReg825xx(ICH9_PHY_CTRL,0xb840088);
		else
			vWriteReg825xx(ICH9_PHY_CTRL,0xb84008d);

#ifdef DEBUG
		dRet = dGetAutoRdDone(psDev);
		if (dRet == 1)
		{
			vConsoleWrite("NVM Auto Read Done\n");
		}
#endif

		vWriteReg825xx(ERT,ERT_2048 | (1 << 13));

		dTarc = dReadReg825xx(TARC(0));
		dTarc |= SPEED_MODE_BIT;
		vWriteReg825xx(TARC(0), dTarc);
		vWriteReg825xx(KABGTXD, 0x00050000L);
	}
	else if ((psDev->bController == CTRLT_82577) || (psDev->bController == CTRLT_82579))
	{
		vWriteReg825xx(ERT,ERT_2048);
		vWriteReg825xx(0x02170L,0x00040402);
		vWriteReg825xx(0x02828L,0x00000000L);
		vWriteReg825xx(0x03828L,0x00000000L);
		vWriteReg825xx(0x05008L,0x000000C0L);
	}



	/* Clear any pending interrupts */

	vWriteReg825xx (IMC, I8257x_VALID_INTERRUPTS);	/* clear interrupt mask */
	(void)dReadReg825xx (ICR);				/* read status to clear active */

	/* Receive control register */

	vWriteReg825xx (RCTL, RCTL_LPE       |	/* long Packet Enable	*/
	                      RCTL_SBP       |  /* Store bad packets */
	                      RCTL_RDMTS_1_2 |  /* free buff threshold = RDLEN/2 */
						  RCTL_UPE 		 |
						  RCTL_MPE       |  /* make sure we get everything */
	                      RCTL_BSIZE_2048); /* use 2-kByte RX buffers */

	/* Disable flow control XON/XOFF frames */

	vWriteReg825xx (FCRTL, 0);
	if ((psDev->bController == CTRLT_82571EB) ||
		(psDev->bController == CTRLT_82573L)  ||
		(psDev->bController == CTRLT_82575EB) ||
		(psDev->bController == CTRLT_82576EB) ||
		(psDev->bController == CTRLT_82567)   ||
		(psDev->bController == CTRLT_82574)   ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_82580EB) )
		vWriteReg825xx (FCRTH, 0x80003000);
	else
		vWriteReg825xx (FCRTH, 0);

	/* Receive descriptor ring */

	vWriteReg825xx (RDBAL, psDev->dRxDescAddr);
	vWriteReg825xx (RDBAH, 0x00000000);
	vWriteReg825xx (RDLEN, psDev->dRxDescSize);
	vWriteReg825xx (RDH, 0x00000000);				/* ring is empty so head = tail = 0 */
	vWriteReg825xx (RDT, 0x00000000);

	/* Set zero delay for receive interrupts */

	vWriteReg825xx (RDTR, 0);

	/* Receive descriptor control */
	dRegVal = 0;
	dRegVal = (1 << S__WTHRESH);

	if ((psDev->bController == CTRLT_82575EB) ||
		(psDev->bController == CTRLT_82576EB) ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_82580EB) ||
		(psDev->bController == CTRLT_I350)    ||
		(psDev->bController == CTRLT_I210))
		dRegVal |= (1 << 25);
	else
		dRegVal |= (1 << S__GRAN);

	vWriteReg825xx (RXDCTL, dRegVal); /*** Need to select this for tuning ***/


	/* Set inter-packet gap for IEEE 802.3 standard */
	if (psDev->bController == CTRLT_82567)
    {
	    vWriteReg825xx (TIPG, 0x00702008);
    }
	else if ((psDev->bController == CTRLT_82577) ||
			 (psDev->bController == CTRLT_82579))
	{
		vWriteReg825xx (TIPG, 0x00602008);
	}
	else if ((psDev->bController == CTRLT_I350) ||
			 (psDev->bController == CTRLT_I210))
	{
		vWriteReg825xx (TIPG, 0x00601008);
	}
	else
		vWriteReg825xx (TIPG, 0x00600806);

	/* Set IFS throttle to (recommended) zero for present */

	vWriteReg825xx (AIT, 0);

	/* Set Transfer DMA control to (recommended) zero */

	vWriteReg825xx (TXDMAC, 0);

	/* Transmit descriptor ring */

	vWriteReg825xx (TDBAL, psDev->dTxDescAddr);
	vWriteReg825xx (TDBAH, 0x00000000);
	vWriteReg825xx (TDLEN, psDev->dTxDescSize);
	vWriteReg825xx (TDH, 0x00000000); /* ring is empty so head = tail = 0 */
	vWriteReg825xx (TDT, 0x00000000); /* TEST purposes only ... */

	/* Set zero delay for transmit interrupts */

	vWriteReg825xx (TIDV, 1); /* use 1 as 0 is not allowed ... CNDM */

	/* Transmit descriptor control */
	dRegVal = 0;
	dRegVal = (1 << S__WTHRESH);

	if ((psDev->bController == CTRLT_82575EB) ||
		(psDev->bController == CTRLT_82576EB) ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_82580EB) ||
		(psDev->bController == CTRLT_I350)    ||
		(psDev->bController == CTRLT_I210))
		dRegVal |= (1 << 25);
	else
		dRegVal |= (1 << S__GRAN);

	vWriteReg825xx (TXDCTL, dRegVal); /** Need to select this for tuning **/

	/* Dont' need IP or TCP/UDP checksum offload */

	vWriteReg825xx (RXCSUM, 0);


	/* Set receive address (from EEPROM) */
	dRegVal = 0;
	vGetLanAddr (psDev, abLanAddr);
	dRegVal = abLanAddr[0] | (abLanAddr[1] << 8) | (abLanAddr[2] << 16) |
			(abLanAddr[3] << 24);
	vWriteReg825xx (RAL, dRegVal);
	dRegVal = abLanAddr[4] | (abLanAddr[5] << 8);
	vWriteReg825xx (RAH, dRegVal | RAH_AV); /* and mark as valid */

	/* disable auto negotiation */
	if (psDev->bController != CTRLT_82567)
	{
		dRegVal = dReadReg825xx(TXCW);
		vWriteReg825xx (TXCW, dRegVal & (~TXCW_ANE));
	}
} /* vInit825xx () */

#if 0 /* not used */
/*******************************************************************
 * vInit82577: initialize an instance of the 8257x
 *
 * RETURNS: none
 */

static void vInit82577
(
	DEVICE* psDev,		/* pointer to device data structure */
    UINT8   bSpeed		/* required link speed */
)
{
	UINT8	abLanAddr[6];
	UINT32	dRegVal = 0;
	PCI_PFA pfa;




#ifdef DEBUG
	UINT32 dPhyCtrl = 0;
	UINT32 dExtCnfCtrl = 0;
	UINT32 dRet = 0;
	UINT32 dTarc = 0;
	char buffer[0x64];
#endif
	/*
	 * Initialize the registers
	 */

	/* Set control register to select speed and full-duplex mode */
	if ((psDev->bController == CTRLT_82574))
	{
		vWriteReg825xx (CTRL, 0x00100248L);
		vWriteReg825xx (CTRL_EXT, 0x00000000L);
		vWriteReg825xx (CTRL_EXT, 0x00580000L);
	}
	else
	{
		vWriteReg825xx (CTRL, 0x00100240L);
		vWriteReg825xx (CTRL_EXT, 0x00000000L);
		vWriteReg825xx (CTRL_EXT, 0x115A1000L);
	}

	/*if ((psDev->bController != CTRLT_82574))
	{
		vWriteReg825xx (CTRL_EXT, dReadReg825xx (CTRL_EXT) & ~(0x100000));
		vWriteReg825xx (CTRL_EXT, dReadReg825xx (CTRL_EXT) | ~(0xFBFFFFFFL));
	}*/

   /* Initialize the flow control registers to recommended values */
	if ((psDev->bController == CTRLT_82574))
   {
		vWriteReg825xx (FCAL, 0x00C28001L);
		vWriteReg825xx (FCAH, 0x00000100L);
		vWriteReg825xx (FCT, 0x00008808L);
		vWriteReg825xx (FCTTV, 0);
		vWriteReg825xx (0x00034L,0x0010C024L);

		/* Initialize VLAN type to recommended value and disable VLAN */
		vWriteReg825xx (VET, 0x00008100L);
   }
   else
	{
		/*vWriteReg825xx (FCAL, 0x08430b40L);
		vWriteReg825xx (FCAH, 0x00000008L);
		vWriteReg825xx (FCT, 0x80908000L);
		vWriteReg825xx (FCTTV, 0);*/

		/* Initialize VLAN type to recommended value and disable VLAN */
		/*vWriteReg825xx (VET, 0x0000C800L);*/
	}

	/* change back to default ... CNDM */
	/* Set packet buffer allocation for 50:50 between Tx and Rx */
	pfa = PCI_MAKE_PFA(psDev->bBus, psDev->bDev, psDev->bFunc);

	dRegVal = dPciReadReg (pfa, PCI_VID, PCI_DWORD);

	if ((psDev->bController != CTRLT_82574))
	{
		/*vWriteReg825xx (PBA, 0x001A0006L);
		vWriteReg825xx (0x01008L,0x00000020L);*/
	}
	else
	{
		vWriteReg825xx (PBA, 0x00180010L);
		vWriteReg825xx (0x01008L,0x00000020L);
	}

#if 0
	if ((psDev->bController == CTRLT_82577))
	{
		dExtCnfCtrl = dReadReg825xx(ICH9_EXTCNF_CTRL);
		dPhyCtrl = dReadReg825xx(ICH9_PHY_CTRL);
		#ifdef DEBUG
			sprintf (buffer, "PHY CTRL %x of 82567\n",
							(int)dPhyCtrl);
			vConsoleWrite (buffer);
		#endif

		vWriteReg825xx(ICH9_PHY_CTRL,0xb84008d);
		dRet = dGetAutoRdDone(psDev);

		if (dRet == 1)
		{
#ifdef DEBUG
			vConsoleWrite("NVM Auto Read Done\n");
#endif
		}

		vWriteReg825xx(ERT,ERT_2048 | (1 << 13));

		dTarc = dReadReg825xx(TARC(0));
		dTarc |= SPEED_MODE_BIT;
		vWriteReg825xx(TARC(0), dTarc);
		vWriteReg825xx(KABGTXD, 0x00050000L);
	}
#endif
	/* Clear any pending interrupts */

	vWriteReg825xx (IMC, I8257x_VALID_INTERRUPTS);	/* clear interrupt mask */
	(void)dReadReg825xx (ICR);				/* read status to clear active */

	/* Receive control register */
	{
		vWriteReg825xx (RCTL,0x6010020L);
	}

	{
		vWriteReg825xx (TCTL,0x3003f0f8L);
		vWriteReg825xx (0x02170L,0x00040402L);
	}

	if ((psDev->bController == CTRLT_82577))
	{
		/*vWriteReg825xx (0x02008L,0x00000100L);*/
	}

	/* Disable flow control XON/XOFF frames */

	vWriteReg825xx (FCRTL, 0);
	vWriteReg825xx (FCRTH, 0x80003000);

	/* Receive descriptor ring */

	vWriteReg825xx (RDBAL, psDev->dRxDescAddr);
	vWriteReg825xx (RDBAH, 0x00000000);
	vWriteReg825xx (RDLEN, psDev->dRxDescSize);
	vWriteReg825xx (RDH, 0x00000000);	/* ring is empty so head = tail = 0 */
	vWriteReg825xx (RDT, 0x00000000);

	/* Set zero delay for receive interrupts */

	vWriteReg825xx (RDTR, 0);

	/* Receive descriptor control */
	dRegVal = 0;
	dRegVal = dReadReg825xx (RXDCTL);
	dRegVal = (1 << S__WTHRESH);
	dRegVal |= (1 << S__GRAN);
	vWriteReg825xx (RXDCTL, dRegVal); /*** Need to select this for tuning ***/


	/* Set inter-packet gap for IEEE 802.3 standard */
	/*if ((psDev->bController == CTRLT_82577))*/
    {
	    vWriteReg825xx (TIPG, 0x00602008);
    }


	/* Set IFS throttle to (recommended) zero for present */

	vWriteReg825xx (AIT, 0);

	/* Set Transfer DMA control to (recommended) zero */

	vWriteReg825xx (TXDMAC, 0);

	/* Transmit descriptor ring */

	vWriteReg825xx (TDBAL, psDev->dTxDescAddr);
	vWriteReg825xx (TDBAH, 0x00000000);
	vWriteReg825xx (TDLEN, psDev->dTxDescSize);
	vWriteReg825xx (TDH, 0x00000000); /* ring is empty so head = tail = 0 */
	vWriteReg825xx (TDT, 0x00000000); /* TEST purposes only ... */

	/* Set zero delay for transmit interrupts */

	vWriteReg825xx (TIDV, 1); /* use 1 as 0 is not allowed ... CNDM */

	/* Transmit descriptor control */
	dRegVal = 0;
	dRegVal = dReadReg825xx (TXDCTL);
	dRegVal = (1 << S__WTHRESH);

	dRegVal |= (1 << S__GRAN);

	vWriteReg825xx (TXDCTL, dRegVal); /** Need to select this for tuning **/

	/* Dont' need IP or TCP/UDP checksum offload */

	vWriteReg825xx (RXCSUM, 0);


	/* Set receive address (from EEPROM) */
	dRegVal = 0;
	vGetLanAddr (psDev, abLanAddr);
	dRegVal = abLanAddr[0] | (abLanAddr[1] << 8) | (abLanAddr[2] << 16) |
			(abLanAddr[3] << 24);
	vWriteReg825xx (RAL, dRegVal);
	dRegVal = abLanAddr[4] | (abLanAddr[5] << 8);
	vWriteReg825xx (RAH, dRegVal | RAH_AV); /* and mark as valid */
} /* vInit825xx () */
#endif

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
	DEVICE *psDev,				/* pointer to device data structure		*/
	const UINT8 *pbSrcAddr,		/* source LAN address (local address)	*/
	const UINT8 *pbDestAddr,	/* destination LAN address				*/
	UINT16	wDataSize,			/* size of data part of frame			*/
	UINT16	wNumFrames			/* number of frames to create			*/
)
{
	UINT32	dTxBuffAddr;	/* Address of TX buffers */
	UINT32	dRxBuffAddr;	/* Address of RX buffers */
	UINT16	wNumBuff;		/* number of buffers required by each frame */
	UINT16	wLastBlkSize;	/* size of last block of buffer */
	UINT16	wCount;			/* counter for frames */
	UINT16	wIndex;			/* counter for blocks within frame */
	UINT16	wDesc;			/* descriptor number */
	UINT8*	pbTx;			/* pointer to TX buffer data */
	UINT8*	pbRx;			/* pointer to RX buffer data */


	/* Clear out all the descriptors
	 */

	dTxBuffAddr = psDev->dTxBuffAddr;
	dRxBuffAddr = psDev->dRxBuffAddr;

	for (wCount = 0; wCount < TX_NUM_DESC; wCount++)
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

	for (wCount = 0; wCount < RX_NUM_DESC; wCount++)
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

	if (psDev->bController == CTRLT_82541PI)
	{
		/* this device seems to drop the first frame
		 * after changing speed, so lets send 1 more
		 */

		wNumFrames++;
	}

	for (wCount = 0; wCount < wNumFrames; wCount++)
	{
		for (wIndex = 0; wIndex < wNumBuff; wIndex++)
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

		memcpy (pbTx,	  pbDestAddr, 6);		/* 1st: Destination Address	*/
		memcpy (pbTx + 6, pbSrcAddr,  6);		/* 2nd: Source Address		*/
		pbTx [12] = (UINT8) (wDataSize >> 8);	/* 3rd: Frame Length (MSB)	*/
		pbTx [13] = (UINT8)  wDataSize;			/* 4th: Frame Length (LSB)	*/

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

} /* wCreateDescriptorRings () */


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
	UINT16	awEeprom[EEPROM_SIZE];
	UINT16	wChecksum;
	UINT8	bIndex;

	if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
    {
	    /* Read the EEPROM contents to an array */
  		 vReadNvmIch9Lan(psDev,0x00,EEPROM_SIZE,(UINT16*)&awEeprom);
    }
	else
	{
		if ((psDev->bController == CTRLT_82571EB) ||
			(psDev->bController == CTRLT_82573L)  ||
			(psDev->bController == CTRLT_82575EB) ||
			(psDev->bController == CTRLT_82576EB) ||
			(psDev->bController == CTRLT_82574)   ||
			(psDev->bController == CTRLT_82580)   ||
//			(psDev->bController == CTRLT_82580EB) ||
			(psDev->bController == CTRLT_I350)    ||
			(psDev->bController == CTRLT_I210))
		{
			if ((dReadReg825xx (EECD) & EECD_PRES) == 0)
			{
				#ifdef DEBUG
					sprintf (buffer, "EEPROM not present\n");
					vConsoleWrite(buffer);
				#endif
					return E__EEPROM_ACCESS;
			}
		}

		/* Read the EEPROM contents to an array */
		for (bIndex = 0; bIndex < EEPROM_SIZE; bIndex++)
		{
/*
			if (((psDev->bController == CTRLT_82580EB) || (psDev->bController == CTRLT_I350))
				 && (psDev->bFunc == 1))
*/
			if ((psDev->bController == CTRLT_I350) && (psDev->bFunc > 0))
			{
				switch (psDev->bFunc)
				{
					case 1:
						awEeprom[bIndex] = wReadEepromWord (psDev, bIndex + 0x80);
						break;

					case 2:
						awEeprom[bIndex] = wReadEepromWord (psDev, bIndex + 0xC0);
						break;

					case 3:
						awEeprom[bIndex] = wReadEepromWord (psDev, bIndex + 0x100);
						break;
			}
			}
/*
			else if ((psDev->bController == CTRLT_82580EB) && (psDev->bFunc == 2))
			{
				awEeprom[bIndex] = wReadEepromWord (psDev, bIndex + 0xC0);
			}
			else if ((psDev->bController == CTRLT_82580EB) && (psDev->bFunc == 3))
			{
				awEeprom[bIndex] = wReadEepromWord (psDev, bIndex + 0x100);
			}
*/
			else
			{
				awEeprom[bIndex] = wReadEepromWord (psDev, bIndex);
			}
		}
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

	for (bIndex = 0; bIndex < EEPROM_SIZE; bIndex++)
		wChecksum += awEeprom[bIndex];

	if (wChecksum != EEPROM_CHECKSUM)
	{
#ifdef DEBUG
		sprintf (buffer, "EEPROM content does not sum to %04Xh, actual = %04Xh\n",
						EEPROM_CHECKSUM, wChecksum);
		vConsoleWrite(buffer);
#endif
		return (E__EEPROM_CHECKSUM);
	}

	return(E__OK);

} /* dEpromTest () */


/*******************************************************************
 * dEpromTestNVM: Test the EEPROM connectivity and checksum
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dEpromTestNVM
(
	DEVICE* psDev		/* pointer to device data structure */
)
{
	UINT8	bIndex;
	UINT32	adInvm[INVM_SIZE];

	/* clear iNVM data */
	memset(adInvm, 0, (sizeof(UINT32)) * INVM_SIZE);

	/* Read the iNVM contents into buffer */
	for (bIndex = 0; bIndex < INVM_SIZE; ++bIndex)
	{
		adInvm[bIndex] = dReadReg825xx (INVM_DATA_REG(bIndex));
	}

	if (adInvm[0] == 0)
	{
#ifdef DEBUG
		vConsoleWrite("iNVM empty\n");
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
#endif

#ifdef DEBUG
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

} /* dEpromTestNVM () */


/*******************************************************************
 * dCtrlReset: Reset the selected Ethernet controller
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dCtrlReset (DEVICE *psDev)
{
	UINT32 dTemp = 0;
	UINT16 wResetTimer = 0;
	UINT32 dResetBits = CTRL_RST;

	if ((psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
	    dResetBits |= CTRL_PHY_RESET;
	}

    /*if (psDev->bController == CTRLT_82580)
    	dResetBits |= DEV_RESET;*/

	dTemp = dReadReg825xx (CTRL);
	vWriteReg825xx (CTRL, dTemp | dResetBits);

	vDelay(20);
	wResetTimer = 200; /* allow 200ms for RESET to complete ... */

	while (((dReadReg825xx (CTRL) & CTRL_RST) == CTRL_RST) && (wResetTimer > 0))
	{
		wResetTimer--;
		vDelay (1);
	}


    if ((psDev->bController == CTRLT_82580) ||
 		(psDev->bController == CTRLT_I350)  ||
		(psDev->bController == CTRLT_I210))
    {
		wResetTimer = 200; /* allow 200ms for RESET to complete ... */
		while ((dReadReg825xx(PCIE_CTRL_GCR) & DEV_RESET_INPROGRS) == DEV_RESET_INPROGRS && (wResetTimer >0))
		{
			wResetTimer--;
			vDelay(1);
		}
    }

	/* Reset PHY */
	if(wResetTimer != 0)
	{
			/* Reset PHY */
		if ((psDev->bController != CTRLT_82577) &&
			(psDev->bController != CTRLT_82579))
		{
			dTemp = dReadReg825xx (CTRL);
			vWriteReg825xx (CTRL, dTemp | CTRL_PHY_RESET);
		}

		vDelay (50);

		vWriteReg825xx (CTRL, dTemp);

	    if ((psDev->bController == CTRLT_82576EB) ||
			(psDev->bController == CTRLT_82580))
		{
			vWriteReg825xx(CTRL,dReadReg825xx (CTRL) & (0xfffffbff));
		}

		vDelay (400);
		return E__OK;
	}
	else
		return E__CTRL_RESET_TIMEOUT;

} /* dCtrlReset () */


/*******************************************************************
 * dEepromrwtest:
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dEepromrwtest
(
	DEVICE* psDev,	/* pointer to device data structure */
	UINT8	bType	/* Interface Type: IF_COPPER or IF_FIBRE	*/
)
{
	UINT16  wOrgChecksum = 0;
	UINT32 dstatus;

	if ((bType != IF_COPPER) && (bType != IF_FIBER) && (bType != IF_SERDES))
	{
#ifdef DEBUG
		vConsoleWrite ("Invalid Media Type specified\n");
#endif
		return E__EEPROM_BAD_MEDIA;
	}

	if ((psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579) ||
		(psDev->bController == CTRLT_I210))
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
		vConsoleWrite("Error writing to EEPROM Checksum\n");
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
		vConsoleWrite(buffer);
#endif
		return (E__EEPROM_RW);
	}


	/* Force re-load of EEPROM ckecksum */
	dstatus = dWriteEepromWord (psDev, EEPROM_CSUM, wOrgChecksum);

	if (dstatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("Error writing to EEPROM Checksum\n");
#endif
		return (dstatus);
	}

	return (E__OK);

} /* dEepromrwtest () */


/*******************************************************************
 * dLocalLoopback:
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dLocalLoopback
(
	DEVICE* psDev,	/* pointer to device data structure */
	UINT8	bType,	/* Interface Type: IF_COPPER or IF_FIBRE	*/
	UINT32	dSpeed	/* speed for test */
)
{
	UINT32	dTemp;
	UINT32	i;
	UINT16	wCompleted;
	UINT16	wDescrCount;
	UINT16	wTimer;
	UINT32	dStatus;
	UINT16	wResetTimer,reg_data;
	UINT8	abEthernetAddress [6];
#ifdef DEBUG
	char buffer[0x64];
#endif

	UINT8 link;


	/* Check the Media Type: As Fibre Optic interfaces operate at 1000Mb/s
	 * then loopback can not be selected as the 82543GC performs echo
	 * cancellation and the message is effectively lost. Fail with an
	 * "Invalid Media" error for fibre optic interfaces.
	 */

	if ((psDev->bController == CTRLT_82580EB) && (bType == IF_SERDES))
	{
			return (E__OK);
	}

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

	if ((psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		lan_init_done_ich8lan(psDev);
		power_up_phy_copper(psDev);
	}


	/* Get this device's ethernet address */
	vGetLanAddr (psDev, abEthernetAddress);
	vClear8257xStatistics(psDev);


	if (psDev->bController == CTRLT_82577)
	{
		write_kmrn_reg(psDev, E1000_KMRNCTRLSTA_TIMEOUTS, 0xFFFF);
		read_kmrn_reg(psDev,E1000_KMRNCTRLSTA_INBAND_PARAM,&reg_data);
		reg_data |= 0x3F;
		write_kmrn_reg(psDev,E1000_KMRNCTRLSTA_INBAND_PARAM, reg_data);
		copper_link_setup_82577(psDev);
		vDoWorkarounds(psDev);
		e1e_flush();
	}
	else if (psDev->bController == CTRLT_82579)
	{
		copper_link_setup_82577(psDev);
		e1e_flush();
	}

	if(psDev->bController == CTRLT_82567)
	{
        PhySpeed (psDev, SPEED_100);
        PhyReset(psDev);
    }
    else
    {
        PhySpeed (psDev, dSpeed);
        e1e_flush();
    }



	/* Initialize the device ...	*/
	#ifdef DEBUG
		vConsoleWrite ("Initializing the device ...\n");
	#endif
	if(psDev->bController == CTRLT_82567)
	{
	   vInit825xx (psDev, SPEED_100);
	}
	else
	{
	   vInit825xx (psDev, dSpeed);
	   e1e_flush();
	}


	/* Create the descriptor rings ...	*/
	#ifdef DEBUG
		vConsoleWrite ("Creating descriptor rings ...\n");
	#endif
	wDescrCount = wCreateDescriptorRings (psDev, abEthernetAddress,
											abEthernetAddress, 0x0C00, 1);
	vInitTxDescr (psDev, 0);
	vInitRxDescr (psDev, 0);

	/* Enter full duplex mode	*/
	#ifdef DEBUG
		vConsoleWrite ("Entering full duplex mode ...\n");
	#endif


	if (psDev->bController != CTRLT_82567)
		vSetLoopbackMode (psDev, bType, LB_EXTERNAL, dSpeed);

	vDelay (10);

	if ((psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		phy_has_link_generic(psDev, COPPER_LINK_UP_LIMIT, 10, &link);

		if (link)
		{
#ifdef DEBUG
			vConsoleWrite ("Link is UP\n");
#endif
			config_collision_dist(psDev);
		}
		else
		{
#ifdef DEBUG
			vConsoleWrite ("Link is Down\n");
#endif
		}
	}

	#ifdef DEBUG
		vConsoleWrite ("Attempting to establish the link ...\n");
	#endif
	if ((psDev->bController != CTRLT_82567) &&
		(psDev->bController != CTRLT_82580) &&
		(psDev->bController != CTRLT_I350)	&&
		(psDev->bController != CTRLT_I210))
	{
		dTemp = dReadReg825xx (CTRL);
		vWriteReg825xx (CTRL, dTemp | CTRL_LRST);
		vDelay (10);
	}

	dTemp = dReadReg825xx (CTRL);
	if ((psDev->bController != CTRLT_82567) &&
		(psDev->bController != CTRLT_82580) &&
		(psDev->bController != CTRLT_I350)	&&
		(psDev->bController != CTRLT_I210))
	{
		vWriteReg825xx (CTRL, (dTemp & (~CTRL_LRST)) | CTRL_SLU);
		vDelay (10);
	}

	dTemp = dReadReg825xx (CTRL);
 	if(psDev->bController == CTRLT_82567)
	{
		vWriteReg825xx (CTRL, dTemp | CTRL_SLU);
		vDelay (10);
	}

 	if ((psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
 	{
 		dTemp = dReadReg825xx (CTRL);
 		vWriteReg825xx (CTRL, dTemp | CTRL_FD);
 		vDelay(10);
 		vWriteReg825xx (CTRL, (dTemp & (~CTRL_LRST)) | CTRL_SLU | CTRL_FRCSPD | CTRL_FRCDPLX);
 		vDelay (100);
 	}
 	else if ((psDev->bController == CTRLT_82580) ||
			 (psDev->bController == CTRLT_I350))
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
		vConsoleWrite(buffer);
#endif
		dTemp = dReadReg825xx (CTRL);
		dTemp &= M__CTRL_SPEED;
		switch (dSpeed)
		{
			case SPEED_10:
				break;

			case SPEED_100:
				dTemp += CTRL_SPEED100;
				break;
			case SPEED_1000:
				dTemp += CTRL_SPEED1000;
				break;
		}

 		vWriteReg825xx (CTRL, (dTemp | CTRL_FRCSPD | CTRL_FRCDPLX | CTRL_FD | CTRL_SLU));
 		vDelay (100);
	}

	#ifdef DEBUG
		vConsoleWrite ("Checking to see if the link could be established ...\n");
	#endif
	if ((psDev->bController == CTRLT_82580) ||
		(psDev->bController == CTRLT_I350)  ||
		(psDev->bController == CTRLT_I210))
	{
		vDelay(19*100);
		wResetTimer = 20000;
	}
	else
		wResetTimer = 10000;

	while (((dReadReg825xx (STATUS) & STATUS_LU) == 0) && (wResetTimer > 0))
	{
		vDelay (1);
		wResetTimer--;
	}

	if (wResetTimer == 0)
	{
		#ifdef DEBUG
            sprintf (buffer,"Unable to establish link (CTRL=%x,STATUS=%x)\n",
                      dReadReg825xx (CTRL), dReadReg825xx (STATUS));
            vConsoleWrite(buffer);
            vDisplayPhyRegs(psDev);

		#endif
		return (E__LOC_LB_LINK_FAIL);
	}

#ifdef DEBUG
        sprintf (buffer,"CTRL=%x, STATUS=%x, wResetTimer=%d\n",
					dReadReg825xx (CTRL),
					dReadReg825xx (STATUS),
					wResetTimer);

        vConsoleWrite(buffer);
#endif

	/* ... and send ...	*/
	#ifdef DEBUG
		vConsoleWrite("Sending\n");
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
		wTimer--;
	} while (!wCompleted && (wTimer > 0));

#ifdef DEBUG
	sprintf (buffer, "wCompleted:%d wTimer:%d\n",wCompleted,wTimer);
	vConsoleWrite(buffer);
#endif

	vDisableTxRx (psDev);
	vDelay (10);

	if (wTimer == 0)
	{
#ifdef DEBUG
		sprintf (buffer, "%s: Status 0x%08x,\n"
					"TxD0 status 0x%02x, TxD1 status 0x%02x,\n"
					"RxD0 status 0x%02x, RxD1 status 0x%02x",
					"ERROR - Ethernet Data Transmission Error Timed Out",
					dReadReg825xx (STATUS),
					(int) psDev->psTxDesc[0].bStatus,
					(int) psDev->psTxDesc[1].bStatus,
					(int) psDev->psRxDesc[0].bStatus,
					(int) psDev->psRxDesc[1].bStatus);

		vConsoleWrite(buffer);

		sprintf (buffer, " RCTL %08X, CTRL %08X,"
					"PCTRL %04X  TCTL %08X\n",
					dReadReg825xx (RCTL),
					dReadReg825xx (CTRL),
					ReadPhyRegister (psDev, PHY_CTRL),
					dReadReg825xx (TCTL));

		vConsoleWrite(buffer);
#endif
		 vDisplay8257xStats(psDev);
		 vDisplayPhyRegs(psDev);
		return (E__LOC_LB_TIMED_OUT);
	}

	for (i = 0; i < LOOP_BACK_SIZE; i++)
	{
		if (psDev->pbTxBuff [i] != psDev->pbRxBuff [i])
		{
			#ifdef DEBUG
				sprintf (buffer,
						"ERROR - Ethernet Data Transmission Error,"
						" expected %02Xh, got %02Xh\n",
						psDev->pbRxBuff [i],
						psDev->pbTxBuff [i]);

				vConsoleWrite(buffer);
			#endif
			return (E__LOC_LB_BAD_DATA);
		}
	}

	return (E__OK);
}/* wLocalLoopback () */


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
	DEVICE* psDev,	/* pointer to device data structure			*/
	UINT8	bType	/* interface mode: IF_COPPER or IF_FIBRE	*/

)
{
	UINT32	dTemp= 0;
	UINT32	i = 0;
	UINT16	wCompleted = 0;
	UINT16	wDescrCount = 0;
	UINT16	wTimer = 0;
	UINT32	dStatus = 0;
	UINT8	abEthernetAddress [6];
	UINT16  wPhyReg = 0;
	UINT32 kmrnctrlsta = 0;
#ifdef DEBUG
    UINT32 dPhyId = 0;
	char achBuff[0x64];
#endif

	if ( (psDev->bController == CTRLT_82580EB) && (psDev->bType == IF_SERDES))
	{
		return (E__OK);
	}
	if (psDev->bType == IF_SERDES)
	{
		if ((psDev->bController != CTRLT_82576EB) &&
			(psDev->bController != CTRLT_I350))
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
			vConsoleWrite("ERROR - Timed out whilst resetting the controller\n");
		#endif
		return E__CTRL_RESET_TIMEOUT;
	}

	/* Get this device's ethernet address */
	vGetLanAddr (psDev, abEthernetAddress);

								/*CTRLT_82567LM*/
	if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
#ifdef DEBUG
		   dPhyId = dGetPhyId(psDev);
		   sprintf (achBuff, "PHY ID %x of 82567",
							(int)dPhyId);
		   vConsoleWrite (achBuff);
#endif
	}

	if (psDev->bController == CTRLT_82567)
    {
		/* Set Default MAC Interface speed to 100MB/s */
        ReadPhyRegister_BM(psDev,PHY_REG(2, 21), &wPhyReg);
        wPhyReg &= ~0x0007;
        wPhyReg |= 0x0006;
        WritePhyRegister_BM(psDev,PHY_REG(2, 21), wPhyReg);

        /* Assert SW reset for above settings to take effect */
        PhyReset (psDev);
        vDelay(1);

        /* Force Full Duplex */
        ReadPhyRegister_BM(psDev,PHY_REG(769, 16), &wPhyReg);
        WritePhyRegister_BM(psDev,PHY_REG(769, 16), wPhyReg | 0x000C);

        /* Set Link Up (in force link) */
        ReadPhyRegister_BM(psDev,PHY_REG(776, 16), &wPhyReg);
        WritePhyRegister_BM(psDev,PHY_REG(776, 16), wPhyReg | 0x0040);

        /* Force Link */
        ReadPhyRegister_BM(psDev,PHY_REG(769, 16), &wPhyReg);
        WritePhyRegister_BM(psDev,PHY_REG(769, 16), wPhyReg | 0x0040);

        /* Set Early Link Enable */
        ReadPhyRegister_BM(psDev,PHY_REG(769, 20), &wPhyReg);
        WritePhyRegister_BM(psDev,PHY_REG(769, 20), wPhyReg | 0x0400);

        PhySpeed (psDev, SPEED_1000);
	}

	if(psDev->bController == CTRLT_82577 /*|| psDev->bController == CTRLT_82579*/)
	{
			/* Wake up PHY */
			/*WritePhyRegister(psDev, 0, (ReadPhyRegister(psDev, 0) & (~0x800)));*/
			/* Read Kumeran register */
			kmrnctrlsta = (((7<<16) & 0x1F0000) | 0x200000);
			vWriteReg825xx(0x34, kmrnctrlsta);
			vDelay(2);
			kmrnctrlsta = dReadReg825xx(0x34);

			/* Configure kumeran register */
			kmrnctrlsta &= ~0x0002;
			/* Write Kumeran Register */
			kmrnctrlsta = (((7<<16) & 0x1F0000) | kmrnctrlsta);
			vWriteReg825xx(0x34, kmrnctrlsta);
			vDelay(2);
			/* Configure */
			vDelay(20);
			vWriteReg825xx (CTRL, ((dReadReg825xx(CTRL) & (~(0x200 | 0x100))) | 0x800));
			vWriteReg825xx (CTRL_EXT, (dReadReg825xx(CTRL_EXT) | 0x8000));
			vDelay(40);
	 }
	else if (psDev->bController == CTRLT_82579)
	{
		copper_link_setup_82577(psDev);
		e1e_flush();
		vDelay(10);
	}

    if ((psDev->bController == CTRLT_82576EB) && (psDev->bType != IF_SERDES ))
    {
#ifdef DEBUG
        vConsoleWrite("Setting phy speed to 1G\n");
#endif
        PhySpeed (psDev, SPEED_1000);
        vDelay (250);
#ifdef DEBUG
        sprintf (buffer, "PHY Control register set to %x\n",ReadPhyRegister (psDev, 0));
        vConsoleWrite(buffer);
#endif
    }

	if (psDev->bController != CTRLT_82576EB )
	{
		PhySpeed (psDev, SPEED_1000);
		vDelay (10);
		if ((psDev->bController != CTRLT_82576LM) &&
			(psDev->bController != CTRLT_82577)   &&
			(psDev->bController != CTRLT_82579))
		{
			dStatus = PhyReset (psDev);
		}

		if (dStatus != E__OK)
		{
			#ifdef DEBUG
			vConsoleWrite("ERROR - Timed out whilst resetting the PHY\n");
			#endif
			return E__PHYRESET_TIMEOUT;
		}
	}


								/*CTRLT_82567LM*/
	if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		vInit825xx (psDev, SPEED_1000);
	}
	else
	{
		vInit825xx (psDev, SPEED_1000); /* 82544EI works only at 1000Mb/sec */
	}

	/* Initialize the device and then create the descriptor rings ...	*/
	wDescrCount = wCreateDescriptorRings (psDev, abEthernetAddress,
											abEthernetAddress, LOOP_BACK_SIZE, 1);

	vInitTxDescr (psDev, 0);
	vInitRxDescr (psDev, 0);

	/* enter loopback mode 	 */
	vSetLoopbackMode (psDev, bType, LB_INTERNAL, SPEED_1000);
	vDelay (10);

	/* Enter full duplex mode
	*/
	dTemp = dReadReg825xx (CTRL);
	vWriteReg825xx (CTRL, dTemp | CTRL_FD);
	vDelay(1);

#ifdef DEBUG
    sprintf (buffer, "PHY Control register after setting LB=%x\n", ReadPhyRegister (psDev, 0));
    vConsoleWrite(buffer);
#endif

	if ((psDev->bController == CTRLT_82576EB) && (psDev->bType == IF_SERDES))
	{
	    vSerDesLoopbackMode(psDev);
    }
    else if (psDev->bController == CTRLT_82576EB )
	{

            #ifdef DEBUG
			vConsoleWrite("Setting 82576 Copper mode parameters\n");
	    #endif
	        vWriteReg825xx (CTRL, dReadReg825xx (CTRL) | CTRL_SLU );
                vDelay (100);
	}
    else if (psDev->bController == CTRLT_82579)
    {
    	/* Reset the LINK (1 -> 0) ... causing autonegotiation ... */
    	dTemp = dReadReg825xx (CTRL);
    	vWriteReg825xx (CTRL, dTemp | CTRL_LRST);
    	vDelay (10);
    	dTemp = dReadReg825xx (CTRL);
    	vWriteReg825xx (CTRL, (dTemp & (~CTRL_LRST)) | CTRL_SLU | CTRL_FRCSPD | CTRL_FRCDPLX);
    	vDelay (100);
    }
	else
	{
		/* Reset the LINK (1 -> 0) ... causing autonegotiation ... */
		dTemp = dReadReg825xx (CTRL);
		vWriteReg825xx (CTRL, dTemp | CTRL_LRST);
		vDelay (10);
		dTemp = dReadReg825xx (CTRL);
		vWriteReg825xx (CTRL, (dTemp & (~CTRL_LRST)) | CTRL_SLU | CTRL_FRCSPD | CTRL_FRCDPLX);
		vDelay (100);
		if (!((psDev->bController == CTRLT_I350) ||
				(psDev->bController == CTRLT_I210)))
		{
			vWriteReg825xx (CTRL, dReadReg825xx (CTRL) | CTRL_ILOS);
		}
	}

	if ((psDev->bController == CTRLT_82580) ||
		(psDev->bController == CTRLT_I350)  ||
		(psDev->bController == CTRLT_I210))
	{
		wTimer = 1000;
	}
	else if(psDev->bController == CTRLT_82579)
		wTimer = 800;
	else
	    wTimer = 400;


	while (((dReadReg825xx (STATUS) & STATUS_LU) == 0) && (wTimer > 0))
	{
		vDelay (1);
		wTimer--;
	}

	if (wTimer == 0)
	{
#ifdef DEBUG
		vConsoleWrite("\nLink UP Timed Out\n");
#endif
		return (E__INT_LB_TIMED_OUT);
	}
	else
	{
#ifdef DEBUG
		vConsoleWrite("\nSending...\n");
#endif
	}

	/* ... and send ...	 */

	vEnableRxTx (psDev, EN_RXTX);
	vInitRxDescr (psDev, wDescrCount);
	vDelay(40);
	vInitTxDescr (psDev, wDescrCount);

	if ((psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		wTimer = 3000;
	}
	else
		wTimer = 1000;

	do
	{
		vDelay (1);
		wCompleted = ((psDev->psRxDesc [0].bStatus & RDESC_STATUS_DD) != 0) &&
		             ((psDev->psRxDesc [1].bStatus & RDESC_STATUS_DD) != 0);
		wTimer--;
	} while (!wCompleted && (wTimer > 0));

	vDisableTxRx (psDev);
	vDelay (10);

	if (wTimer == 0)
	{
		#ifdef DEBUG
		sprintf (achBuff, "%s: Status 0x%08x,\n"
				"TxD0 status 0x%02x, TxD1 status 0x%02x,\n"
				"RxD0 status 0x%02x, RxD1 status 0x%02x",
		        "ERROR - Ethernet Data Transmission Error Timed Out",
				dReadReg825xx (STATUS),
				(int) psDev->psTxDesc[0].bStatus,
				(int) psDev->psTxDesc[1].bStatus,
				(int) psDev->psRxDesc[0].bStatus,
				(int) psDev->psRxDesc[1].bStatus);
		vConsoleWrite(achBuff);

		sprintf (achBuff, "\nRCTL %08X, CTRL %08X,"
				" PSTATUS %04X PCTRL %04X  TCTL %08X\n",
				dReadReg825xx (RCTL), dReadReg825xx (CTRL),
				ReadPhyRegister (psDev, 0x01),
				ReadPhyRegister (psDev, PHY_CTRL), dReadReg825xx (TCTL));
		vConsoleWrite(achBuff);
		#endif
		return (E__INT_LB_TIMED_OUT);
	}

	for (i = 0; i < LOOP_BACK_SIZE; i++)
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
			return (E__INT_LB_BAD_DATA);
		}
	}

	return (E__OK);

} /* dInternalLoopback () */



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
	int i;

	wBufsPerFrame= (wDataSize + ETH_OH + (TX_BUFF_SIZE - 1)) / TX_BUFF_SIZE;
	pbRxBuff = psRxDev->pbRxBuff;

	/* start testing from the 1st received frame */
	pbTxBuff = psTxDev->pbTxBuff;

	pbTxBuff += ((UINT16)pbRxBuff[14] * TX_BUFF_SIZE * wBufsPerFrame);

	for (wFrameCount = 0; wFrameCount < wNumFrames; wFrameCount++)
	{

		for (i = 0; i < wDataSize; i++)
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
#ifdef INCLUDE_I82599
	if(psDev->bController != CTRLT_82599)
	{
#endif
		dTail = dReadReg825xx(TDT);
		wRet = (UINT16)dTail;
		dTail += dDescrCount;
		vWriteReg825xx(TDT, dTail);
#ifdef INCLUDE_I82599
	}
	else
	{
		dTail = dReadReg825xx(I8259x_TDT);
		wRet = (UINT16)dTail;
		dTail += wDescrCount;
		vWriteReg825xx(I8259x_TDT, dTail);
	}
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

	while (wDescrCount > 0)
	{
		if ((psDev->psTxDesc [wCurrentDescr++].bStatus & TDESC_STATUS_DD) != 0)
		{
			wDescrCount--;
		}
		else
			break;

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
	UINT16	wDescrCount	/* number of descriptors to TX */
)
{
	while (wDescrCount > 0)
	{
		if ((psDev->psRxDesc [wCurrentDescr++].bStatus & RDESC_STATUS_DD) != 0)
		{
			wDescrCount--;
		}
		else
			break;

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

	while ((psDescr->bStatus & TDESC_STATUS_DD) != 0)
	{
		psDescr++;
		wCount++;
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
	DEVICE* psTxDev,	/* pointer to transmit device data structure */
	DEVICE* psRxDev,	/* pointer to receive device data structure */
	UINT8	bType,		/* Interface Type: IF_COPPER or IF_FIBRE	*/
	UINT16	wFrameCount,/* number of frames to send for testing */
	UINT8   bTestSpeed  /* speed for test. 0 = all, 1 = 10MB, 2 = 100MB 3 = 1GB */
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
	UINT16  reg_data;
	UINT16	wFramesReceived;
	UINT8	abTxEthAddress [6];
	UINT8	abRxEthAddress [6];
	UINT8*	pEthAddr;
	DEVICE* psDev;
	int		iDev;
	char	achBuffer[80];
	UINT16	wRetry;
	UINT8  link = 0;
#ifdef INCLUDE_I82599
	UINT16 wlist_offset, wsfp_id, wdata_value, wdata_offset;
#endif
#ifdef DEBUG
	vConsoleWrite ("\n\nPMC loopback\n");
#endif

	/* Check the Media Type: As Fibre Optic interfaces operate at 1000Mb/s
	 * then loopback can not be selected as the 82543GC performs echo
	 * cancellation and the message is effectively lost. Fail with an
	 * "Invalid Media" error for fibre optic interfaces.
	 */


	bMinSpeed = SPEED_10;

	if ((bType == IF_FIBER) || (bType == IF_SERDES))
	{
		vConsoleWrite ("SERDES interface\n");
		bMinSpeed = SPEED_1000;
	}

	bMaxSpeed = psTxDev->bMaxSpeed;
	if (psRxDev->bMaxSpeed < bMaxSpeed)
		bMaxSpeed = psRxDev->bMaxSpeed;

	if (bTestSpeed > 0)
	{
		bMinSpeed = bTestSpeed;
		bMaxSpeed = bTestSpeed;
	}

	for (bSpeed = bMinSpeed; bSpeed <= bMaxSpeed; bSpeed++)
	{

		/* Reset the Ethernet Controllers
		*/

		for (iDev = 0; iDev < 2; iDev++)
		{
			if (iDev == 0)
				psDev = psTxDev;
			else
				psDev = psRxDev;

			if ((psTxDev->bController == psRxDev->bController) &&
				(psDev->bController == CTRLT_I350) &&
				(psDev->bType == IF_SERDES))
			{
#ifdef DEBUG
				vConsoleWrite ("Controller reset not executed as this\n");
				vConsoleWrite ("will set i350 back to IF_COPPER\n");
#endif
				break;		// reset not required
			}

#ifdef INCLUDE_I82599
			if (psDev->bController != CTRLT_82599)
			{
#endif
				dStatus = dCtrlReset (psDev);
				vClear8257xStatistics(psDev);

				if (dStatus != E__OK)
				{
					sprintf (psTxDev->achErrorMsg,
							"ERROR - Timed out whilst resetting controller %d",
							iDev);
					return E__RESET_TIMEOUT;
				}

				if ((psDev->bController == CTRLT_82577) /*|| (psDev->bController == CTRLT_82579)*/)
				{
					lan_init_done_ich8lan(psDev);
					vDelay(10);
					power_up_phy_copper(psDev);
					vDelay(10);
					write_kmrn_reg(psDev, E1000_KMRNCTRLSTA_TIMEOUTS, 0xFFFF);
					read_kmrn_reg(psDev,E1000_KMRNCTRLSTA_INBAND_PARAM,&reg_data);
					reg_data |= 0x3F;
					write_kmrn_reg(psDev,E1000_KMRNCTRLSTA_INBAND_PARAM, reg_data);
					copper_link_setup_82577(psDev);
					vDoWorkarounds(psDev);
					vDelay(10);
					e1e_flush();
				}
				else if (psDev->bController == CTRLT_82579)
				{
					copper_link_setup_82577(psDev);
					vDelay(10);
					e1e_flush();
				}

#ifdef DEBUG
				vConsoleWrite ("Controller reset complete\n");
#endif
#ifdef INCLUDE_I82599
			}
#endif
		} /* End of For loop */

		/* Get the device's ethernet addresses
		*/

		for (iDev = 0; iDev < 2; iDev++)
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

#ifdef INCLUDE_I82577
									/*CTRLT_82567LM*/
		    if ((psDev->bController == CTRLT_82567) ||
				(psDev->bController == CTRLT_82577) ||
				(psDev->bController == CTRLT_82579))
		    {
		       vInitNvmParamsIch9Lan(psDev);
	        }
#endif
		    vGetLanAddr (psDev, pEthAddr);
		}


		/* Initialize the device and then create the descriptor rings ...
		*/
#ifdef DEBUG
		vConsoleWrite ("Initializing devices and creating descriptor rings ...\n");
#endif
		for (iDev = 0; iDev < 2; iDev++)
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
				PhySpeed (psDev, bSpeed);
				e1e_flush();

				if ((bSpeed == SPEED_1000) &&
					((psDev->bController == CTRLT_82576LM) ||
					 (psDev->bController == CTRLT_82576EB)))

		       	{
			        PhyReset(psDev);
			        vDelay (10);
		        }
		    }

#ifdef INCLUDE_I82599
			if (psDev->bController != CTRLT_82599)
			{
#endif
				vInit825xx (psDev, bSpeed);
			 	e1e_flush();

#ifdef INCLUDE_I82599
			}
#endif

#ifdef INCLUDE_I82599
			else
			{
				wStatus = w8259xReset(psDev);
				if(wStatus == E__OK)
				{
					wStatus = w8259xStart(psDev);
				}
				/* SFP reset and set linkup - Autoneg */
				vWriteReg825xx(I8259x_AUTOC,
				    (dReadReg825xx(I8259x_AUTOC) | I8259x_AUTOC_AN_RESTART));
			}
#endif

			if (psDev->bType == IF_SERDES)
			{
			#ifdef INCLUDE_I82599
				if (psDev->bController != CTRLT_82599)
				{
			#endif	/* make sure TBI mode is enabled */
					dTemp = dReadReg825xx (CTRL_EXT) & M__CTRLEXT_LINK;
					vWriteReg825xx (CTRL_EXT, dTemp | CTRLEXT_LINK_TBI);
			#ifdef INCLUDE_I82599
				}
			#endif
			}

			/* Init RxDev and TxDev */
			if (iDev == 0)
			{
				wDescrUsed = wCreateDescriptorRings (psDev, abTxEthAddress,
									abRxEthAddress, 0x0C00, wFrameCount + 10);
				wDescrPerFrame = wDescrUsed / (wFrameCount + 10);
				vInitTxDescr (psDev, 0);
			}
			else
			{
				wDescrUsed = wCreateDescriptorRings (psDev, abTxEthAddress,
									abRxEthAddress, 0x0C00, wFrameCount);
				vInitRxDescr (psDev, 0);
			}

		    if (psDev->bController == CTRLT_82580)
			    vSetLoopbackMode (psDev, bType, LB_EXTERNAL, bSpeed);
		    else
				vSetLoopbackMode (psDev, bType, LB_NONE, bSpeed);


#ifdef INCLUDE_I82599
			if (psDev->bController != CTRLT_82599)
			{
#endif
				/* Enter full duplex mode
				*/
#ifdef DEBUG
				vConsoleWrite ("Entering full duplex mode ...\n");
#endif
				dTemp = dReadReg825xx (CTRL);
				vWriteReg825xx (CTRL, dTemp | CTRL_FD);
				vDelay (1);
#ifdef INCLUDE_I82599
			}
#endif
		}

#ifdef INCLUDE_I82599
	if (psDev->bController != CTRLT_82599)
	{
#endif
		/* Reset the LINK (1 -> 0) ... causing autonegotiation ...
		*/
#ifdef DEBUG
		vConsoleWrite ("Resetting the LINK ...\n");
#endif

		for (iDev = 0; iDev < 2; iDev++)
		{
			if (iDev == 0)
				psDev = psTxDev;
			else
				psDev = psRxDev;

#ifdef DEBUG
			vConsoleWrite ("Attempting to establish the link ...\n");
#endif

			if ((psDev->bController == CTRLT_82580EB) ||
				((psDev->bController == CTRLT_I350) && (psDev->bType == IF_SERDES)))
			{
				vSerDesChannelLoopbackMode (psDev);
			}
			else
			{
				dTemp = dReadReg825xx (CTRL);
				vWriteReg825xx (CTRL, dTemp | CTRL_LRST);
				vDelay (10);

				dTemp = dReadReg825xx (CTRL);
				vWriteReg825xx (CTRL, (dTemp & (~CTRL_LRST)) | CTRL_SLU);

				if ((psDev->bType == IF_SERDES) & (psDev->bController != CTRLT_82571EB) )
				{
					dTemp = dReadReg825xx (CTRL);
					vWriteReg825xx (CTRL, dTemp | CTRL_ILOS);
				}
				else if ((psDev->bType == IF_SERDES) & (psDev->bController == CTRLT_82571EB) )
				{
					dTemp = dReadReg825xx (CTRL);
					vWriteReg825xx (CTRL, dTemp & ~CTRL_ILOS);
				}

				if ((psDev->bController == CTRLT_82577) ||
					(psDev->bController == CTRLT_82579))
				{
					dTemp = dReadReg825xx (CTRL);
					vWriteReg825xx (CTRL, dTemp | CTRL_FD);
					vDelay(10);
					dTemp = dReadReg825xx (CTRL);
					vWriteReg825xx (CTRL, (dTemp & (~CTRL_LRST)) | CTRL_SLU | CTRL_FRCSPD | CTRL_FRCDPLX);
					vDelay (100);
				}
			}

			vDelay (10);
		}

		sprintf (achBuffer, "Waiting for %s link\n", aachSpeed[bSpeed]);
		vConsoleWrite (achBuffer);

		for (iDev = 0; iDev < 2; iDev++)
		{
			if (iDev == 0)
				psDev = psTxDev;
			else
				psDev = psRxDev;

			if ((psDev->bController == CTRLT_82580EB) ||
				((psDev->bController == CTRLT_I350) && (psDev->bType == IF_SERDES)))
			{

				wResetTimer = 4000;
				while (((dReadReg825xx (PCS_LSTAT) & PCS_LINK_OK) == 0) && (wResetTimer > 0))
				{
					vDelay (10);
					wResetTimer--;
				}

  			    if (wResetTimer == 0)
				{
#ifdef DEBUG
					sprintf (psTxDev->achErrorMsg,
								"ERROR - Controller %d unable to establish the link",
								iDev);
					sprintf (achBuffer,
								"ERROR - Controller %d unable to establish the link",
								iDev);
					vConsoleWrite (achBuffer);
#endif
					return (E__LOC_LB_LINK_FAIL);
				}
			}
			else
			{
				if (psDev->bController == CTRLT_82580)
				{
					wResetTimer = 10000;
				}
				else
				{
					wResetTimer = 5000;
				}

				e1e_flush();
				while (((dReadReg825xx (STATUS) & STATUS_LU) == 0) && (wResetTimer > 0))
				{
					vDelay (10);
					wResetTimer--;
				}

				if (wResetTimer == 0)
				{
#ifdef DEBUG
					sprintf (psTxDev->achErrorMsg,
								"ERROR - Controller %d unable to establish the link",
								iDev);
					sprintf (achBuffer,
								"ERROR - Controller %d unable to establish the link",
								iDev);
					vConsoleWrite (achBuffer);
#endif
					return (E__LOC_LB_LINK_FAIL);
				}
			}

			if ((psDev->bController == CTRLT_82577) ||
				(psDev->bController == CTRLT_82579))
			{
				phy_has_link_generic(psDev,COPPER_LINK_UP_LIMIT,1000,&link);
				if (link)
				{
#ifdef DEBUG
					vConsoleWrite ("Phy Link UP\n");
#endif
				}
				else
				{
#ifdef DEBUG
					vConsoleWrite ("Phy Link Down\n");
#endif
				}
			}
			else if( psDev->bType != IF_SERDES)
			{
				if (psDev->bController != CTRLT_82580EB)
				{
					wResetTimer = 1000;
					while (((ReadPhyRegister (psDev,0x01) & 0x0004) == 0) && (wResetTimer > 0))
					{
						vDelay (1);
						wResetTimer--;
					}

					if (wResetTimer == 0)
					{
						sprintf (psTxDev->achErrorMsg,
								"ERROR 2 - Controller %d unable to establish the link",
								iDev);
#ifdef DEBUG
						sprintf (achBuffer,
								"ERROR 2 - Controller %d unable to establish the phy link",	iDev);
						vConsoleWrite (achBuffer);
#endif
						return (E__LOC_LB_LINK_FAIL);
					}
				}
			}
		}

		vDelay (2000);	/* wait for hub */

		/* ... and send ...
		*/

		vEnableRxTx (psRxDev, EN_RX);
		vInitRxDescr (psDev, wDescrUsed);
		vDelay (200);	/* wait for hub */
		vEnableRxTx (psTxDev, EN_TX);
#ifdef INCLUDE_I82599
	}
	else
	{
		vDelay (2000);

		/* Enable RxDev */
		vInitRxDescr (psRxDev, wDescrUsed);
		vDelay (200);
	}
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
			 	break;
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
		sprintf (psTxDev->achErrorMsg, "\nTransmitted %d descriptors, received %d",
				wCountDescr ((TDESC*)psTxDev->psTxDesc), wCountDescr ((TDESC*)psRxDev->psRxDesc));
		sprintf (achBuffer, "Transmitted %d descriptors, received %d",
				wCountDescr ((TDESC*)psTxDev->psTxDesc), wCountDescr ((TDESC*)psRxDev->psRxDesc));
		vConsoleWrite (achBuffer);
		if (psTxDev->bType ==IF_COPPER)
		{

			vConsoleWrite ("\nTransmit Phy:\n");
			vDisplayPhyRegs(psTxDev);
			vConsoleWrite ("\nRecvr Phy:\n");
			vDisplayPhyRegs(psRxDev);
		}
#endif
		return E__LOC_LB_TIMED_OUT;
	}

	/* must have Tx'd and Rx'd something so do some more */
	vConsoleWrite ("Sending\n");
	wSendDescr(psTxDev, (wFrameCount-1) * wDescrPerFrame);		/* send the rest */
	vDelay (2000); /*1000*/

	/* check transmitted desriptors */
	psDev = psTxDev;
	wTxDescrCount = 0;
	while ((psDev->psTxDesc [psDev->wTxCurrentDescr + wTxDescrCount].bStatus & TDESC_STATUS_DD) != 0)
	{
		wTxDescrCount++;
	}

	/* check received descriptors */
	psDev = psRxDev;
	wFramesReceived = 0;
	wRxDescrCount = 0;
	wCompleted = 0;
	while ((psDev->psRxDesc [wRxDescrCount].bStatus & RDESC_STATUS_DD) != 0)
	{
		wRxDescrCount++;
		if ((psDev->psRxDesc [wRxDescrCount].bStatus & RDESC_STATUS_EOP) != 0)
		{
			wFramesReceived++;

			/* Allow 1 frame to be lost */
			if (wFramesReceived >= (wFrameCount - 1))
				wCompleted = 1;
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
		sprintf (achBuffer, "ERROR - Ethernet Data Receive Error."
							" Timed Out, transmitted %d received %d descriptors\n"
							"Rx status[0]=%02x  Rx status[1]=%02x\n"
							"Tx status[0]=%02x  Tx status[1]=%02x\n",
							(int)wTxDescrCount, (int)wRxDescrCount,
							(int)psRxDev->psRxDesc [0].bStatus,
							(int)psRxDev->psRxDesc [1].bStatus,
							(int)psTxDev->psTxDesc [0].bStatus,
							(int)psTxDev->psTxDesc [1].bStatus);
		vConsoleWrite (achBuffer);
#endif
		return (E__LOC_LB_TIMED_OUT);
	}

#ifdef INCLUDE_I82577
	if (psTxDev->bController == CTRLT_82574 || psRxDev->bController == CTRLT_82574 ||
	    psTxDev->bController == CTRLT_82576LM || psRxDev->bController == CTRLT_82576LM) /*TODO: Hari Integ Bios */
	{

	}
	else
	{
#endif
		dStatus = dCheckTestData (psTxDev, psRxDev,
	  			      LOOP_BACK_SIZE, wFramesReceived);
	    if (dStatus != E__OK)
		   return dStatus;
#ifdef INCLUDE_I82577
    }
#endif

	}
	for (iDev = 0; iDev < 2; iDev++)
	{
		if (iDev == 0)
		{
			psDev = psTxDev;
		}
		else
		{
			psDev = psRxDev;
		}
#ifdef INCLUDE_I82599
		if (psDev->bController == CTRLT_82599)
		{
			dStatus = d8259xReset(psDev);
		}
#endif
	}

#ifdef DEBUG
	vConsoleWrite("Done PMC Loopback\n");
#endif
	return (E__OK);

} /* wPMCLoopback () */


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
	DEVICE* psDev		/* pointer to device data structure */
)
{
	UINT32	dCount;
	UINT32	dIcr;
	UINT32	dMask;
	UINT32	dRegVal;

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
		vConsoleWrite(buffer);
#endif
		return (E__ICR_NOT_ZERO);
	}

	/* Install interrupt handler and enable interrupts for this device */

	/* TODO new_interrupt_table	(psDev->bVector, (void far *) PCI_pass_handler,
										 (void far *) PCI_fail_handler, TRUE);*/

	/* Test each interrupt source: first check internal hardware with source
	 * masked, then check external interrupt generation.
	 */

	for (dCount = 0; dCount < 11; dCount++)
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
			continue;

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
			vConsoleWrite(buffer);
#endif
			return (E__NO_MASKINT);
		}

		if ((dIcr & (~(ICR_RXCFG | ICR_LSC))) != dMask)	/* check all except link status change and rxcfg*/
		{
#ifdef DEBUG
			/*vRestoreInterruptState (psDev);*/
			sprintf (buffer, "(bit-%d) Wrong interrupt generated"
			                             " (ICR=%08X)", dCount, dIcr);
			vConsoleWrite(buffer);
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
	vConsoleWrite("Interrupt OK\n");
#endif
	return (E__OK);

} /* dInterruptTest () */


/*******************************************************************
 * dRegisterTest: Test the register access by rolling ones and zeros
 * pattern writing and read back
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dRegisterTest
(
	DEVICE* psDev		/* pointer to device data structure */
)
{
	UINT32	dRdbahOld;
	UINT32	dTestPattern;
	UINT32	dTestRegister;
	UINT32	i;
	UINT32	dTestStatus;

	dTestStatus = E__OK;

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
	dRdbahOld = dReadReg825xx (RDBAH);


	/* Perform a rolling '1' test on the RDBAH register
	*/

	dTestPattern = 0x00000001;
	for (i = 0; (i < 32) && (dTestStatus == E__OK); i++)
	{
		vWriteReg825xx (RDBAH, dTestPattern);
		dTestRegister = dReadReg825xx (RDBAH);
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

	/* Perform a rolling '0' test on the RDBAH register
	*/

	dTestPattern = 0x00000001;
	for (i = 0; (i < 32) && (dTestStatus == E__OK); i++)
	{
		vWriteReg825xx (RDBAH, ~dTestPattern);
		dTestRegister = dReadReg825xx (RDBAH);
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

	vWriteReg825xx (RDBAH, dRdbahOld);

	return dTestStatus;

} /* dRegisterTest () */


/*******************************************************************
 * dInitPciDevice: Initialise the Ethernet device, allocate Memory
 * BARs, Tx/ RX descriptors, buffers
 *
if (psDev->bController != CTRLT_82576EB)
	  {
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dInitPciDevice
(
	DEVICE* psDev,		/* pointer to device data structure */
	UINT8	bIndex		/* controller index */
)
{
	PCI_PFA pfa;
	UINT32  dTemp;
	UINT32	dDidVid;
	UINT16	wTemp;

	psDev->bIndex = bIndex;

	/* determine type of ethernet controller */
	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	dDidVid = dPciReadReg (pfa, PCI_VID, PCI_DWORD);

	psDev->bMaxSpeed = SPEED_1000;

	switch (dDidVid)
	{
	case DIDVID_FIBER :
	case DIDVID_COPPER :
		psDev->bController = CTRLT_82543GC;
		psDev->bPhyAddress = 1;		/* external PHY on new GB1 cards */
		strcpy (psDev->achCtrlName, "82543GC");
		break;
	case DIDVID_82544EI :
		psDev->bController = CTRLT_82544EI;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82544EI");
		break;
	case DIDVID_82546EB :
		psDev->bController = CTRLT_82546EB;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82546EB");
		break;
	case DIDVID_82546GB :
		psDev->bController = CTRLT_82546EB; /* use the same as EB */
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82546GB");
		break;
	case DIDVID_82540EM :
		psDev->bController = CTRLT_82540EM;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82540EM");
		break;
	case DIDVID_82541PI :
	case DIDVID_82541PIa :
		psDev->bController = CTRLT_82541PI;
		psDev->bPhyAddress = 1;     /* internal PHY */
		strcpy (psDev->achCtrlName, "82541PI");
		break;
	case DIDVID_82571EB_F :
	case DIDVID_82571EB_S :
	case DIDVID_82571EB_C:
		psDev->bController = CTRLT_82571EB;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82571EB");
		break;
	case DIDVID_82580_C:
	case DIDVID_82580_CS:
		psDev->bController = CTRLT_82580;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82580");
		break;
	case DIDVID_82580_EB:
		psDev->bController = CTRLT_82580EB;
		psDev->bPhyAddress = 1;		/* internal PHY */
		psDev->bType = IF_SERDES;
		strcpy (psDev->achCtrlName, "82580SerDes");
		break;
	case DIDVID_82573L:
		psDev->bController = CTRLT_82573L;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82573L");
		break;
	case DIDVID_82575EB_C:
		psDev->bController = CTRLT_82575EB;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82575EB");
		break;
	case DIDVID_82576EB:
        case DIDVID_82576EB_C:
		psDev->bController = CTRLT_82576EB;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82576");
		break;
	case DIDVID_82574:
		psDev->bController = CTRLT_82574;
		psDev->bPhyAddress = 1;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82574");
		break;
	case DIDVID_82567:
		psDev->bController = CTRLT_82567;
		psDev->bPhyAddress = 2;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82567");
		break;
    case DIDVID_82577:
		psDev->bController = CTRLT_82577;
		psDev->bPhyAddress = 2;		/* internal PHY */
		strcpy (psDev->achCtrlName, "82577");
		break;
    case DIDVID_82579_C:
    	psDev->bController = CTRLT_82579;
    	psDev->bPhyAddress = 2;		/* internal PHY */
    	strcpy (psDev->achCtrlName, "82579");
    	break;
    case DIDVID_i350AMx_C:
    case DIDVID_i350AMx_S:
    	psDev->bController = CTRLT_I350;
    	psDev->bPhyAddress = 0;		/* internal PHY */
    	strcpy (psDev->achCtrlName, "i350");
    	break;
    case DIDVID_i210:
    	psDev->bController = CTRLT_I210;
    	psDev->bPhyAddress = 0;		/* internal PHY */
    	strcpy (psDev->achCtrlName, "i210");
    	break;
	default:
		psDev->bController = CTRLT_UNKNOWN;
		strcpy (psDev->achCtrlName, "Unknown");
		break;
	}

#ifdef DEBUG
	 sprintf(buffer, "\nController Name: %s type:%d \n", psDev->achCtrlName, psDev->bType);
	 vConsoleWrite(buffer);
#endif

	/* Enable memory access and bus-mastership, leave EPROM to individual
	 * sub-BISTs
     */

	wTemp = (UINT16)dPciReadReg (pfa, PCI_COMMAND, PCI_WORD);
	vPciWriteReg (pfa, PCI_COMMAND, PCI_WORD, (wTemp | PCI_MEMEN | PCI_BMEN) & ~(1<<4));

#ifdef DEBUG
	sprintf(buffer, "PCI_COMMAND: 0x%x \n", dPciReadReg (pfa, PCI_COMMAND, PCI_WORD));
	vConsoleWrite(buffer);
#endif

	/* Set Cache Line Size register (nominal value is 32)
     */

	vPciWriteReg (pfa, PCI_CACHE_LINE_SIZE, PCI_BYTE, 32);

	/* Create a pointer to the memory-mapped registers */

	dTemp  = dPciReadReg (pfa, PCI_BASE_ADDR_1, PCI_DWORD);
	dTemp &= PCI_MEM_BASE_MASK;

#ifdef DEBUG
	sprintf(buffer, "Mem BAR 0: 0x%x \n", dTemp);
  	vConsoleWrite(buffer);
#endif

	if (psDev->bController == CTRLT_I350)
		dHandle1 = dGetPhysPtr (dTemp, I350_REG_SIZE, &p1, (void*)&psDev->pbReg);
	else
		dHandle1 = dGetPhysPtr (dTemp, I8257x_REG_SIZE, &p1, (void*)&psDev->pbReg);

	if(dHandle1 == E__FAIL)
	{
		#ifdef DEBUG
			vConsoleWrite("Unable to allocate BAR Mem \n");
		#endif
		return(E__TEST_NO_MEM);
	}

#ifdef DEBUG
	sprintf(buffer, "Mem BAR 0 val: 0x%x 0x%x\n", (UINT32)(psDev->pbReg), *(psDev->pbReg));
  	vConsoleWrite(buffer);
#endif

  	/* Create a pointer to the Flash EPROM */
	dTemp  = dPciReadReg (pfa, PCI_ROM_BASE, PCI_DWORD);
	dTemp &= PCI_ROM_BASE_MASK;
	dHandle2 = 0;
	if (dTemp != 0)
	{
		dHandle2 = dGetPhysPtr (dTemp, 0x00080000, &p2, (void*)&psDev->pbRomRead);
		if(dHandle2 == E__FAIL)
		{
#ifdef DEBUG
			vConsoleWrite("Unable to allocate PCI ROM \n");
#endif
			return(E__TEST_NO_MEM);
		}
	}

	dHandle3 = 0;
	if ((psDev->bController == CTRLT_82567) || (psDev->bController == CTRLT_82577) || (psDev->bController == CTRLT_82579))
	{
		/* Create a pointer to the Flash EPROM (for write accesses) */
		dTemp  = dPciReadReg (pfa, PCI_BASE_ADDR_2, PCI_DWORD);
		dTemp &= PCI_MEM_BASE_MASK;
		dHandle3 = dGetPhysPtr (dTemp, 0x1000, &p3, (void*)&psDev->pbFlashBase);
		if(dHandle3 == E__FAIL)
		{
			#ifdef DEBUG
				vConsoleWrite("Unable to allocate MEM BAR2 82567\n");
			#endif
			return(E__TEST_NO_MEM);
		}
#ifdef DEBUG
		sprintf(buffer, "Mem BAR 1 (Flash): [0x14] = 0x%x \n", dTemp);
		vConsoleWrite(buffer);
#endif

	}
	else
	{
		dTemp  = dPciReadReg (pfa, PCI_BASE_ADDR_2, PCI_DWORD);
		dTemp &= PCI_ROM_BASE_MASK;
		if (dTemp != 0)
		{
			dHandle3 = dGetPhysPtr (dTemp, 0x00080000, &p3, (void*)&psDev->pbRomWrite);
			if(dHandle3 == E__FAIL)
			{
#ifdef DEBUG
				vConsoleWrite("Unable to allocate MEM BAR2 \n");
#endif
				return(E__TEST_NO_MEM);
			}
		}
	}


	psDev->bVector = (UINT8) dPciReadReg (pfa, PCI_INT_LINE, PCI_BYTE);

	/* -TODO: psDev->bVector = irq_to_vector (psDev->bVector); */
	/* Get pointers for the device's data structures          */

	psDev->dTxDescSize = TX_NUM_DESC * 16; /* 8 = quantity, 16 = size */
	psDev->dTxDescAddr = TX_DESC_BASE + (bIndex * (TX_DESC_LENGTH / MAX_CONTROLLERS));
	dHandle4 = dGetPhysPtr (psDev->dTxDescAddr, TX_DESC_LENGTH, &p4, (void*)&psDev->psTxDesc);
	if(dHandle4 == E__FAIL)
	{
	#ifdef DEBUG
			vConsoleWrite("Unable to allocate TX Desc\n");
	#endif
			return(E__TEST_NO_MEM);
	}

	psDev->dTxBuffAddr = TX_BUFF_BASE + (bIndex * (TX_BUFF_LENGTH / MAX_CONTROLLERS));
	dHandle5 = dGetPhysPtr (psDev->dTxBuffAddr, TX_BUFF_LENGTH, &p5, (void*)&psDev->pbTxBuff);
	if(dHandle5 == E__FAIL)
	{
	#ifdef DEBUG
		vConsoleWrite("Unable to allocate TX Buf \n");
	#endif
		return(E__TEST_NO_MEM);
	}
	memset((void*)psDev->pbTxBuff, 0xCC, TX_BUFF_LENGTH);
	psDev->dRxDescSize = RX_NUM_DESC * 16; /* 8 = quantity, 16 = size */
	psDev->dRxDescAddr = RX_DESC_BASE + (bIndex * (RX_DESC_LENGTH / MAX_CONTROLLERS));
	dHandle6 = dGetPhysPtr (psDev->dRxDescAddr, RX_DESC_LENGTH, &p6, (void*)&psDev->psRxDesc);
	if(dHandle6 == E__FAIL)
	{
	#ifdef DEBUG
		vConsoleWrite("Unable to allocate Rx Desc \n");
	#endif
		return(E__TEST_NO_MEM);
	}

	psDev->dRxBuffAddr = RX_BUFF_BASE + (bIndex * (RX_BUFF_LENGTH / MAX_CONTROLLERS));
	dHandle7 = dGetPhysPtr (psDev->dRxBuffAddr, RX_BUFF_LENGTH, &p7, (void*)&psDev->pbRxBuff);
	if(dHandle7 == E__FAIL)
	{
	#ifdef DEBUG
		vConsoleWrite("Unable to allocate Rx Buff \n");
	#endif
		return(E__TEST_NO_MEM);
	}
	memset((void*)psDev->pbRxBuff, 0xFF, RX_BUFF_LENGTH);

	return(E__OK);

} /* dInitPciDevice () */


/*******************************************************************
 * dFind825xx: Find the occurance of Ethernet device instances
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

UINT32 dFind825xx(DEVICE* psDev, UINT8 bInstance,UINT8 bMode)
{
	PCI_PFA	pfa;
    UINT32  dDidVid;        /* device and vendor IDs */
	UINT8	bCount;			/* running count of devices */
	UINT8	bTemp;
	UINT8	bBus = 0;
	UINT8	bDev = 0;
	UINT8	bFunc = 0;
	UINT8	bMultiFunc = 0;	/* non-zero indicates multi-function device */
	UINT8	bHostBridges = 0;
	UINT8	bScanToBus = 0;

	SkipEthIf_PARAMS   params;

#ifdef DEBUG
	char buffer[80];
#endif

	memset(psDev, 0, sizeof(DEVICE));	/* clear all structure */
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
		if(board_service(SERVICE__BRD_SKIP_ETH_BUS, NULL, NULL) == E__OK)
		{
		  #ifdef DEBUG
			vConsoleWrite("Bus Skipped\n");
		  #endif

       //if ((bBus == 4) && (bDev >=0) && (bFunc >=0))
			 if (bBus == 4)
			 {
				 bBus = 5;
				 bDev = 0;
				 bFunc = 0;
			 }
		}

		pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
		dDidVid = dPciReadReg(pfa, PCI_VID, PCI_DWORD);

		if ((dDidVid == DIDVID_82571EB_F) || (dDidVid == DIDVID_82571EB_C) ||
		    (dDidVid == DIDVID_82573L)    || (dDidVid == DIDVID_82571EB_S) ||
			(dDidVid == DIDVID_82575EB_C) || (dDidVid == DIDVID_COPPER)    ||
			(dDidVid == DIDVID_FIBER)     || (dDidVid == DIDVID_82544EI)   ||
			(dDidVid == DIDVID_82546EB)   || (dDidVid == DIDVID_82540EM)   ||
			(dDidVid == DIDVID_82546GB)   || (dDidVid == DIDVID_82541PI)   ||
			(dDidVid == DIDVID_82541PIa)  || (dDidVid == DIDVID_82567 )    ||
			(dDidVid == DIDVID_82574)     || (dDidVid == DIDVID_82576EB )  ||
            (dDidVid == DIDVID_82576EB_C) || (dDidVid == DIDVID_82577)     ||
            (dDidVid == DIDVID_82580_C)   || (dDidVid == DIDVID_82579_C)   ||
            (dDidVid == DIDVID_82580_EB)  || (dDidVid == DIDVID_82580_CS)  ||
            (dDidVid == DIDVID_i350AMx_C) || (dDidVid == DIDVID_i350AMx_S) ||
			(dDidVid == DIDVID_i210))
		{

#ifdef DEBUG
			sprintf(buffer, "Instance %d found at address: %d:%d:%d, %#X:%#X\n",
							bCount + 1, bBus, bDev, bFunc, dDidVid & 0xFFFF, dDidVid >> 16);
			vConsoleWrite(buffer);
#endif

			params.dDidVid   = dDidVid;
			params.bInstance = bCount + 1;		// service requires one based counter
			params.bMode = bMode;
			params.pfa = pfa;

			if(board_service(SERVICE__BRD_SKIP_ETH_IF, NULL, &params) == E__OK)
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
				bCount++;
				if (bCount == bInstance)	/* we found the card of interest */
				{
#ifdef DEBUG
					sprintf(buffer, "Found Ethernet device %d\n", bCount);
					vConsoleWrite(buffer);
#endif
					if (dDidVid == DIDVID_FIBER) /* correct assumption, if wrong */
						psDev->bType = IF_FIBER;

					if ((dDidVid == DIDVID_82571EB_S) ||
						(dDidVid == DIDVID_82576EB)   ||
						(dDidVid == DIDVID_82580_EB)  ||
						(dDidVid == DIDVID_i350AMx_S))
					{
						psDev->bType = IF_SERDES;
					}

					if ((dDidVid == DIDVID_82571EB_S) ||
						(dDidVid == DIDVID_82571EB_F) ||
						(dDidVid == DIDVID_82571EB_C) ||
						(dDidVid == DIDVID_82575EB_C) ||
						(dDidVid == DIDVID_82546EB)	  ||
						(dDidVid == DIDVID_82546GB)	  ||
						(dDidVid == DIDVID_82576EB)   ||
						(dDidVid == DIDVID_82576EB_C) ||
						(dDidVid == DIDVID_82580_CS)  ||
						(dDidVid == DIDVID_i350AMx_C) ||
						(dDidVid == DIDVID_i350AMx_S))
					{
						psDev->bMultiFunc = 1;
					}

					if ((dDidVid == DIDVID_82580_C) ||
						(dDidVid == DIDVID_82580_EB))
					{
						psDev->bMultiFunc = 3;
					}


/*					if ((bFunc == 2) || (bFunc == 3))
					{
						if (((dDidVid != DIDVID_i350AMx_C) &&
							(dDidVid != DIDVID_i350AMx_S)))
						{
							return (E__TEST_NO_DEVICE);
						}
#ifdef DEBUG
//						sprintf(buffer, "Found 82580 SerDes\n");
//						vConsoleWrite(buffer);
#endif
					}
*/
					psDev->bBus = bBus;
					psDev->bDev = bDev;
					psDev->bFunc = bFunc;
					
#ifdef DEBUG
					sprintf(buffer, "Success: Device Found\n");
					vConsoleWrite(buffer);
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
			bTemp = (UINT8)dPciReadReg (pfa,
			                              PCI_BASE_CLASS, PCI_BYTE);

			if (bTemp == 0x06)	/* PCI<->PCI bridge class */
			{
				pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
				bTemp = (UINT8)dPciReadReg (pfa, PCI_SUB_CLASS, PCI_BYTE);

				if (bTemp == 0x00)
				{
					if (bHostBridges > 0)
						bScanToBus++;

					bHostBridges++;
				}
				else if (bTemp == 0x04)		/* PCI-PCI bridge*/
				{
					pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
					bTemp = (UINT8)dPciReadReg (pfa, PCI_SUB_BUS, PCI_BYTE);

					if (bTemp > bScanToBus)
						bScanToBus = bTemp;
				}
			}
		}

		/* Increment device/bus numbers */

		if (bMultiFunc == 0)
			(bDev)++;
		else
		{
			bFunc++;
			if (bFunc == 8)
			{
				(bDev)++;
				bFunc = 0;
				bMultiFunc = 0;
			}
		}

		if (bDev == 32)
		{
			bDev = 0;
			(bBus)++;
		}

	} while (bBus <= bScanToBus);
	
#ifdef DEBUG
		sprintf(buffer, "Exit: bBus:%d bDev:%d bFunc:%d bScanToBus: %u\n",bBus,bDev,bFunc, bScanToBus);
		vConsoleWrite(buffer);
#endif

	return (E__TEST_NO_DEVICE);

} /* dFind825xx () */


/*******************************************************************
 * dGetNumberOfEthInstances: Helper function to obtain number of
 * Ethernet ports present on boards, esp when board has dynamic
 * port configurations - PO fitted or not
 *
 * RETURNS: Number of ethernet ports available on board
 */

UINT32 dGetNumberOfEthInstances
(
    void
)
{
  DEVICE sDevice;
  UINT16 wPciStatus = PCI_SUCCESS;
  UINT8 bInstance = 1;

  while(wPciStatus == PCI_SUCCESS) {
      wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);
      bInstance = bInstance + 1;
  }
#ifdef DEBUG
                        sprintf(buffer, "Number of Instances: %d\n", (bInstance - 2));
                        vConsoleWrite(buffer);
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

	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	wTemp = (UINT16)dPciReadReg (pfa, PCI_COMMAND, PCI_WORD);

	vPciWriteReg (pfa, PCI_COMMAND,
					PCI_WORD, wTemp & ~(PCI_IOEN | PCI_MEMEN | PCI_BMEN));

	/* clear up all used mem regions */
	vFreePtr(dHandle1);

	if (dHandle2 != 0)
		vFreePtr(dHandle2);

	if (dHandle3 != 0)
		vFreePtr(dHandle3);

	vFreePtr(dHandle4);
	vFreePtr(dHandle5);
	vFreePtr(dHandle6);
	vFreePtr(dHandle7);

} /* vClosePciDevice () */


/*****************************************************************************
 * EthRegAccessTest: test function for Ethernet Register access
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthRegAccessTest, "Ethernet Register Access Test")
{
	UINT16 wPciStatus;
	UINT32 dtest_status;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	DEVICE sDevice;

	if(adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			bInstance++;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		bInstance++;
	}
	do {
#ifdef DEBUG
			sprintf(buffer, "\n\nInstance to find: %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
		wPciStatus =
			dFind825xx (&sDevice, bInstance, ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
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
			#ifdef DEBUG
      			vConsoleWrite("Need to soft Exit\n");
			#endif
			bLoop = 0; /* soft exit if cannot find controller in loop mode */
		}

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
		#ifdef DEBUG
  			vConsoleWrite("adTestParams[1] == 0\n");
		#endif
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
		#ifdef DEBUG
  			vConsoleWrite("i < adTestParams[0]\n");
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

	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}
	return (dtest_status);

} /* EthRegAccessTest */

/*****************************************************************************
 * EthEepromTest: test function for Ethernet EEPROM connectivity and checksum
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthEepromTest, "Ethernet EEPROM Test")
{
	UINT16 wPciStatus;
	UINT32 dRegData;
	UINT32 dtest_status;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	static UINT8 cont = 0;
	DEVICE sDevice;

	if(adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			bInstance++;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		bInstance++;
	}
	do {
#ifdef DEBUG
			sprintf(buffer, "Instance to find: %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
		wPciStatus = dFind825xx (&sDevice, bInstance, ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
			if(!(bInstance & 0xFE))
				cont++;

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
				if ((sDevice.bController == CTRLT_82567) ||
					(sDevice.bController == CTRLT_82577) ||
					(sDevice.bController == CTRLT_82579))
				{
			        vInitNvmParamsIch9Lan(&sDevice);
				}

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

		else if ( (bLoop == 0) ||
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
			bLoop = 0;

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}

	return (dtest_status);

} /* EthEepromTest */

/*****************************************************************************
 * EthPhyRegReadTest: test function for Ethernet PHY access
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthPhyRegReadTest, "Ethernet PHY Test")
{
	UINT16  wPciStatus;
	UINT32  dtest_status;
	UINT8   bInstance = 0, bLoop = 0, i = 1;
	DEVICE  sDevice;
	UINT16  wPhyReg;

	if(adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			bInstance++;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		bInstance++;
	}
	do {
#ifdef DEBUG
			sprintf(buffer, "\n\nInstance to find: %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
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

                    /* Initialize the device and then create the descriptor rings ...	*/
                    vInit825xx (&sDevice, SPEED_1000); /* 82544EI works only at 1000Mb/sec */

					if (sDevice.bController == CTRLT_82567)
					{
						vInitNvmParamsIch9Lan(&sDevice);

						/* Set Default MAC Interface speed to 100MB/s */
						ReadPhyRegister_BM(&sDevice,PHY_REG(2, 21), &wPhyReg);
						wPhyReg &= ~0x0007;
						wPhyReg |= 0x0006;
						WritePhyRegister_BM(&sDevice,PHY_REG(2, 21), wPhyReg);

						/* Assert SW reset for above settings to take effect */
						PhyReset (&sDevice);
						vDelay(1);

						PhySpeed (&sDevice, SPEED_1000);
					}
					else if (sDevice.bController == CTRLT_82577 || sDevice.bController == CTRLT_82579 )
					{
						vInitNvmParamsIch9Lan(&sDevice);

											/* Set Default MAC Interface speed to 100MB/s */
						ReadPhyRegister_LV(&sDevice,PHY_REG(2, 21), &wPhyReg,0);
						wPhyReg &= ~0x0007;
						wPhyReg |= 0x0006;
						WritePhyRegister_LV(&sDevice,PHY_REG(2, 21), wPhyReg,0);

						/* Assert SW reset for above settings to take effect */
						PhyReset (&sDevice);
						vDelay(1);

						PhySpeed (&sDevice, SPEED_1000);
					}

					dtest_status = dReadPhyRegs (&sDevice);
					#ifdef DEBUG
						sprintf(buffer, "Test status: 0x%x\n", dtest_status);
						vConsoleWrite(buffer);
					#endif
				}

			}
		}

		else if ( (bLoop == 0) ||
				( (bLoop == 1) && (bInstance == 1)) ||
				( (bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
			sprintf(buffer, "Unable to locate instance %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
			return (E__TEST_NO_DEVICE);
		}

		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */
	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}
	return (dtest_status);

} /* EthPhyRegReadTest */


/*****************************************************************************
 * EthIntLoopBackTest: test function for internal loopback data transfer
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthIntLoopBackTest, "Ethernet Internal Loopback Test")
{
	UINT16 wPciStatus;
	UINT32 dtest_status;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	DEVICE sDevice;

	if(adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			bInstance++;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		bInstance++;
	}

	do
	{
#ifdef DEBUG
		sprintf(buffer, "Instance to find: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);
#ifdef DEBUG
        sprintf(buffer, "Device Instance: %d type: %d\n", bInstance, sDevice.bType);
        vConsoleWrite(buffer);
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
				sprintf(buffer, "Failed to initialise the device: %d\n", bInstance);
				vConsoleWrite(buffer);
#endif
				vClosePciDevice(&sDevice);
				return(dtest_status);
			}
			else if ((sDevice.bType == IF_SERDES) || (sDevice.bType == IF_FIBER ))
			{
				dtest_status = E__OK;
				vClosePciDevice(&sDevice);
				//#ifdef DEBUG
					vConsoleWrite("Int Loopback not supported in  SERDES/FIBER mode\n");
				//#endif
			}
			else
			{
				if ((sDevice.bController == CTRLT_82567) ||
					(sDevice.bController == CTRLT_82577) ||
					(sDevice.bController == CTRLT_82579))
				{
					 vInitNvmParamsIch9Lan(&sDevice);
				}

				dtest_status = dInternalLoopback (&sDevice, sDevice.bType);
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
      		vConsoleWrite("Unable to locate required instance");
#endif
			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in instance: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}

	return (dtest_status);

} /* EthIntLoopBackTest */


/*****************************************************************************
 * EthInterruptTest: test function for interrupt
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthInterruptTest, "Ethernet Interrupt Test")
{
	UINT16 wPciStatus;
	UINT32 dtest_status;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	static UINT8 cont = 0;
	DEVICE sDevice;

	if(adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			bInstance++;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		bInstance++;
	}
	do {
#ifdef DEBUG
			sprintf(buffer, "\n\nInstance to find: %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1 );
			if(!(bInstance & 0xFE))
				cont++;
			if (dtest_status != E__OK)
			{
#ifdef DEBUG
					sprintf(buffer, "Failes to initialise the device: %d\n", bInstance);
					vConsoleWrite(buffer);
#endif
				vClosePciDevice(&sDevice);
				return(dtest_status);
			}
			else
			{
				dtest_status = dInterruptTest (&sDevice);
#ifdef DEBUG
				sprintf(buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
#endif
			}
		}

		else if ( (bLoop == 0) ||
				( (bLoop == 1) && (bInstance == 1)) ||
				( (bLoop == 1) && (adTestParams[1] != 0)))
		{
#ifdef DEBUG
			sprintf(buffer, "Unable to locate instance %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
			return (E__TEST_NO_DEVICE);
		}

		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}

	return (dtest_status);

} /* EthInterruptTest */


/*****************************************************************************
 * EthExtLoopBackTest: test function for external loopback data transfer
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthExtLoopBackTest, "Ethernet External Loopback Test")
{
	UINT16 wPciStatus;
	UINT32 dtest_status;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	static UINT8 cont = 0;
	DEVICE sDevice;

	if(adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			bInstance++;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		bInstance++;
	}
	do
	{
#ifdef DEBUG
		sprintf(buffer, "Instance to find: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
		wPciStatus = dFind825xx (&sDevice, bInstance,ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
#ifdef DEBUG
			sprintf(buffer, "%s found at b:%d %d f:%d\n", 
					sDevice.achCtrlName, sDevice.bBus, sDevice.bDev, sDevice.bFunc);
			vConsoleWrite(buffer);
#endif
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
			if(!(bInstance & 0xFE))
				cont++;

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
				if ((sDevice.bController == CTRLT_82567) ||
					(sDevice.bController == CTRLT_82577) ||
					(sDevice.bController == CTRLT_82579))
				{
					 vInitNvmParamsIch9Lan(&sDevice);
				}

				if ((sDevice.bController == CTRLT_82577) ||
					(sDevice.bController == CTRLT_82579))
				{
					dtest_status = dLocalLoopback (&sDevice, sDevice.bType, SPEED_1000);
				}
				else if (sDevice.bController == CTRLT_82571EB)
				{
					dtest_status = dLocalLoopback (&sDevice, sDevice.bType, SPEED_10);
					if(dtest_status == E__OK)
						dtest_status = dLocalLoopback (&sDevice, sDevice.bType, SPEED_100);
				}
				else
				{
					dtest_status = dLocalLoopback (&sDevice, sDevice.bType, SPEED_10);
					vDelay(1);

					if(dtest_status == E__OK)
						dtest_status = dLocalLoopback (&sDevice, sDevice.bType,SPEED_100);

					vDelay(1);

					/* 1GB external loopback does not work on i210 */
					if (sDevice.bController != CTRLT_I210)
					{
						if (dtest_status == E__OK)
							dtest_status = dLocalLoopback (&sDevice, sDevice.bType, SPEED_1000);
					}
				}
#ifdef DEBUG
				sprintf(buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
#endif
			}
		}

		else if ( (bLoop == 0) ||
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
			bLoop = 0;


		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}
	return (dtest_status);

} /* EthExtLoopBackTest */


/*****************************************************************************
 * EthExtDevLoopbackTest: test function for external loopback data transfer
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthExtDevLoopbackTest, "Ethernet Dev->Dev Loopback Test")
{
	UINT16	wPciStatus;
	UINT32	dtest_status;
	UINT8	bTxInstance = 1, bRxInstance = 2;
	DEVICE	sTxDevice;
	DEVICE	sRxDevice;
	char	achBuffer[80];
	UINT8	bTxInstanceType = 0;			// for i350 SERDES+Copper loopback
	UINT8	bRxInstanceType = 0;			// for i350 SERDES+Copper loopback

	if(adTestParams[0] >= 1)
	{
		bTxInstance = adTestParams[1];
	}

	if(adTestParams[0] >= 2)
	{
		bRxInstance = adTestParams[2];
	}

	sprintf(achBuffer, "Testing Tx Instance %d, Rx Instance %d\n", bTxInstance, bRxInstance);
	vConsoleWrite(achBuffer);

#ifdef DEBUG
	sprintf(buffer, "Tx Instance to find: %d\n", bTxInstance);
	vConsoleWrite(buffer);
#endif
	wPciStatus = dFind825xx (&sTxDevice, bTxInstance, ETH_MAC_NO_DISP_MODE);

	if (wPciStatus == PCI_SUCCESS)
	{
		dtest_status = dInitPciDevice (&sTxDevice, bTxInstance);
		if (dtest_status != E__OK)
		{
#ifdef DEBUG
				sprintf(buffer, "Failed to initialise the Tx device: %d\n", bTxInstance);
				vConsoleWrite(buffer);
#endif
			vClosePciDevice(&sTxDevice);
			return(dtest_status);
		}
		if ((sTxDevice.bController == CTRLT_82567) ||
			(sTxDevice.bController == CTRLT_82577) ||
			(sTxDevice.bController == CTRLT_82579))
		{
			 vInitNvmParamsIch9Lan(&sTxDevice);
		}
	}
	else
	{
		#ifdef DEBUG
			sprintf(buffer, "Tx Instance not found: %d\n", bTxInstance);
			vConsoleWrite(buffer);
		#endif
		return E__TEST_NO_DEVICE + bTxInstance;
	}

#ifdef DEBUG
	sprintf(buffer, "Rx Instance to find: %d\n", bRxInstance);
	vConsoleWrite(buffer);
#endif
	wPciStatus = dFind825xx (&sRxDevice, bRxInstance,ETH_MAC_NO_DISP_MODE);

	if (wPciStatus == PCI_SUCCESS)
	{
		dtest_status = dInitPciDevice (&sRxDevice, bRxInstance);
		if (dtest_status != E__OK)
		{
#ifdef DEBUG
				sprintf(buffer, "Failed to initialise the Rx device: %d\n", bRxInstance);
				vConsoleWrite(buffer);
#endif
			vClosePciDevice(&sTxDevice);
			vClosePciDevice(&sRxDevice);
			return(dtest_status);
		}
		if ((sRxDevice.bController == CTRLT_82567) ||
			(sRxDevice.bController == CTRLT_82577) ||
			(sRxDevice.bController == CTRLT_82579))
		{
			 vInitNvmParamsIch9Lan(&sRxDevice);
		}
	}
	else
	{
		#ifdef DEBUG
			sprintf(buffer, "Rx Instance not found: %d\n", bRxInstance);
			vConsoleWrite(buffer);
		#endif
		vClosePciDevice(&sTxDevice);
		return E__TEST_NO_DEVICE + bRxInstance;
	}


	if (
	  /*(sTxDevice.bController == CTRLT_82577)   ||
		(sRxDevice.bController == CTRLT_82577)   ||
		(sTxDevice.bController == CTRLT_82574)   ||
		(sRxDevice.bController == CTRLT_82574)   || */
		(sTxDevice.bController == CTRLT_82580EB) ||
		(sRxDevice.bController == CTRLT_82580EB) ||
		(sTxDevice.bController == CTRLT_82580)   ||
		(sRxDevice.bController == CTRLT_82580)   ||
		(sTxDevice.bController == CTRLT_I350)    ||
		(sRxDevice.bController == CTRLT_I350)    ||
		(sTxDevice.bController == CTRLT_I210)    ||
		(sRxDevice.bController == CTRLT_I210))
	{
#ifdef DEBUG
		sprintf(buffer, "TxType = %d, RxType = %d\n", sTxDevice.bController, sRxDevice.bController);
		vConsoleWrite(buffer);
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
					vSwitchI350ToSerdes(&sRxDevice);
				}
				else
				{
					// TxDevice is configured as IF_COPPER
					vSwitchI350ToSerdes(&sTxDevice);
				}
			}
		}

		if ((sTxDevice.bType == IF_SERDES) && (sRxDevice.bType == IF_SERDES))
		{
#ifdef DEBUG
			vConsoleWrite("Serdes->Serdes loop-back\n");
#endif
			dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_1000);

			// check original interface types
			if (bTxInstanceType != bRxInstanceType)
			{
				// one device is SERDES, the other is COPPER
				// so restore interfaces to original eeprom configuration
				if (bTxInstanceType == IF_SERDES)
				{
					// RxDevice was configured as IF_COPPER
					vSwitchI350ToCopper(&sRxDevice);
				}
				else
				{
					// TxDevice was configured as IF_COPPER
					vSwitchI350ToCopper(&sTxDevice);
				}
			}
		}
		else if ((sTxDevice.bType == IF_COPPER) && (sRxDevice.bType == IF_COPPER))
		{
#ifdef DEBUG
			vConsoleWrite("Copper->Copper loop-back\n");
#endif
		    dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_1000);
		}
		else
		{
			vConsoleWrite("Device->Device Test only possible on SAME MEDIA\n");
			sprintf(achBuffer, "sTxDevice.bType:%#x  sRxDevice.bType%#x\n", sTxDevice.bType,sRxDevice.bType);
			vConsoleWrite(achBuffer);
			return (E__OK);
		}

	}
	else if ((sTxDevice.bController == CTRLT_82571EB) ||
			 (sRxDevice.bController == CTRLT_82571EB) ||
			 (sTxDevice.bController == CTRLT_82576EB) ||
			 (sRxDevice.bController == CTRLT_82576EB) ||
			 (sTxDevice.bController == CTRLT_82576LM) ||
			 (sRxDevice.bController == CTRLT_82576LM))
	{
		dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_AUTO);
	}
	else if ((sTxDevice.bController == CTRLT_82577) ||
			 (sRxDevice.bController == CTRLT_82577) ||
			 (sTxDevice.bController == CTRLT_82579) ||
			 (sRxDevice.bController == CTRLT_82579) ||
			 (sTxDevice.bController == CTRLT_82574) ||
			 (sRxDevice.bController == CTRLT_82574))
	{
		dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_10);
		if (dtest_status == E__OK)
			dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_100);
		if (dtest_status == E__OK)
			dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_1000);
	}
	else
	{
		dtest_status = dPMCLoopback (&sTxDevice, &sRxDevice, sTxDevice.bType, 10, SPEED_100);
	}

#ifdef DEBUG
	sprintf(buffer, "Test status: 0x%x\n", dtest_status);
	vConsoleWrite(buffer);
#endif


	vClosePciDevice(&sTxDevice);
	vClosePciDevice(&sRxDevice);
	return (dtest_status);

} /* EthExtDevLoopbackTest */


/*****************************************************************************
 * EthEepromWriteTest: test function for non destructive read and write of
 *						Ethernet EEPROM
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (EthEepromWriteTest, "Ethernet EEPROM Read Write Test")
{
	UINT16 wPciStatus;
	UINT32 dtest_status;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	static UINT8 cont = 0;
	DEVICE sDevice;

	if(adTestParams[0])
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;
			bInstance++;
		}
		else
		{
			bInstance = adTestParams[i];
		}
	}
	else
	{
		bLoop = 1;
		bInstance++;
	}
	do {
#ifdef DEBUG
			sprintf(buffer, "Instance to find: %d\n", bInstance);
			vConsoleWrite(buffer);
#endif
		wPciStatus = dFind825xx (&sDevice, bInstance, ETH_MAC_NO_DISP_MODE);

		if (wPciStatus == PCI_SUCCESS)
		{
			dtest_status = dInitPciDevice (&sDevice, bInstance - 1);
			if(!(bInstance & 0xFE))
				cont++;
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
				if (sDevice.bController == CTRLT_82567)
				{
			        vConsoleWrite("Intel 82567 not supported\n");
					vClosePciDevice(&sDevice);
					dtest_status = E__OK;
				}
				else
				{
					dtest_status = dEepromrwtest(&sDevice, sDevice.bType);
#ifdef DEBUG
					sprintf(buffer, "Test status: 0x%x\n", dtest_status);
					vConsoleWrite(buffer);
#endif
				}
			}
		}

		else if ( (bLoop == 0) ||
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
			bLoop = 0;

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));		/* test all requested instances */

	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}
	return (dtest_status);

} /* EthExtLoopBackTest */





static void vSerDesLoopbackMode(DEVICE *psDev)
{
   UINT32 dReg = 0;

   dReg = dReadReg825xx (CTRL);
   vWriteReg825xx (CTRL, dReg | CTRL_LRST);
   vDelay (10);

   vWriteReg825xx (CTRL_EXT, dReadReg825xx (CTRL_EXT) | (0xC00000));
   vDelay(10);
   vWriteReg825xx(SERDESCTL,ENABLE_SERDES_LOOPBACK);


   vDelay(20);
   dReg = dReadReg825xx(CTRL);
   dReg &= ~(CTRL_RFCE |
             CTRL_TFCE |
             CTRL_RST);

   dReg |= (CTRL_SLU | CTRL_FD | CTRL_ILOS);

   vWriteReg825xx(CTRL, dReg);
   vDelay(10);
   /* Unset switch control to serdes energy detect */
   dReg = dReadReg825xx(CONNSW);
   dReg &= ~CONNSW_ENRGSRC;
   vWriteReg825xx(CONNSW, dReg);
   vDelay(10);
   /* Set PCS register for forced speed */
   dReg = dReadReg825xx(PCS_LCTL);
   dReg &= ~PCS_LCTL_AN_ENABLE;     /* Disable Autoneg*/

   dReg |=  PCS_LCTL_FLV_LINK_UP |   /* Force link up */
            PCS_LCTL_FSV_1000 |      /* Force 1000    */
            PCS_LCTL_FDV_FULL |      /* SerDes Full duplex */
            PCS_LCTL_FSD |           /* Force Speed */
            PCS_LCTL_FORCE_LINK;     /* Force Link */

   vWriteReg825xx(PCS_LCTL, dReg);
}

static void vSerDesChannelLoopbackMode(DEVICE *psDev)
{
    UINT32 dReg = 0,dLinkUp = 0;

	if (psDev->bController != CTRLT_I350)
	   vWriteReg825xx(SERDESCTL, SCTL_DISABLE_SERDES_LOOPBACK);

    dReg = dReadReg825xx(PCS_CFG);
    dReg |= 0x00000008;
    vWriteReg825xx(PCS_CFG, dReg);
    vDelay(10);

    dReg = dReadReg825xx(CTRL);
    dReg &= ~(CTRL_ILOS /*|CTRL_RFCE|CTRL_TFCE*/);
    dReg |= (CTRL_FD | CTRL_SLU);
    vWriteReg825xx(CTRL, dReg);

    vDelay(10);
    dReg = dReadReg825xx(CTRL_EXT);
    dReg &= 0xFF3FFFFFL;
    dReg |= 0x00C00000L;
    vWriteReg825xx(CTRL_EXT, dReg);

    /* Set CONNSW.ENRGSRC to disable */
    dReg = dReadReg825xx(CONNSW);
    dReg &= ~(CONNSW_ENRGSRC);
    vWriteReg825xx(CONNSW, dReg);
    vDelay(100);

    dReg = dReadReg825xx(PCS_LSTAT);
    if (dReg & PCS_LSTS_SYNK_OK)
    {
	    dLinkUp = TRUE;
	    vConsoleWrite("PCS: Link is UP\n");
    }
    else
    {
	    vConsoleWrite("PCS: Link is Down\n");
	    dLinkUp = FALSE;

	    /* Force link up in the PCS register */
	    dReg = dReadReg825xx(PCS_LCTL);
	    dReg &= ~(PCS_LCTL_AN_ENABLE|PCS_LCTL_AN_RESTART|PCS_LCTL_AN_TIMEOUT); /* Disable ANEG */
	    dReg |= PCS_LCTL_FLV_LINK_UP | /* Force link up */
	    PCS_LCTL_FSV_1000 | /* Force 1000 */
	    PCS_LCTL_FDV_FULL | /* SerDes Full duplex */
	    PCS_LCTL_FSD | /* Force Speed */
	    PCS_LCTL_FORCE_LINK; /* Force Link */
	    vWriteReg825xx(PCS_LCTL, dReg);
	    vDelay(20);
    }

    /* Set ILOS if link is not up */
    if ((dLinkUp == FALSE) && (psDev->bController != CTRLT_I350))
    {
	    /* Set bit 7 (Invert Loss) and set link up in bit 6. */
	    dReg = dReadReg825xx(CTRL);
	    dReg |= (CTRL_ILOS);
	    vWriteReg825xx(CTRL, dReg);
    }
    vDelay(20);

}




static void vInitNvmParamsIch9Lan (DEVICE *psDev)
{
    UINT32 dGfpReg;
    UINT32 dSectorBaseAddr;
    UINT32 dSectorEndAddr;
#ifdef DEBUG
    UINT32 dReg = 0;
    char  achBuff [80];
#endif

    /* Can't read flash registers if the register set isn't mapped. */
    if (!psDev->pbFlashBase)
	{
            vConsoleWrite("ERROR: Flash registers not mapped\n");
            return;
    }

#ifdef DEBUG
	dReg = dReadReg825xx(0x0010);
    sprintf (achBuff, "Flash control Reg: %08x\n",(UINT32)dReg);
    vConsoleWrite(achBuff);
#endif

    dGfpReg = dReadFlashReg(ICH_FLASH_GFPREG);

#ifdef DEBUG
    sprintf (achBuff, "dGfpReg: %08x\n",(UINT32)dGfpReg);
    vConsoleWrite(achBuff);
#endif

#ifdef DEBUG
    dReg = dReadFlashReg(0x50);
    sprintf (achBuff, "dAccessPerm: %08x\n",(int)dReg);
    vConsoleWrite(achBuff);
    dReg = dReadFlashReg(0x60);
    sprintf (achBuff, "dGbeLimit: %08x\n",(int)dReg);
    vConsoleWrite(achBuff);
#endif

    dSectorBaseAddr = dGfpReg & FLASH_GFPREG_BASE_MASK;
#ifdef DEBUG
    sprintf (achBuff, "dSectorBaseAddr: %08x\n",(int)dSectorBaseAddr);
    vConsoleWrite(achBuff);
#endif
    dSectorEndAddr = ((dGfpReg >> 16) & FLASH_GFPREG_BASE_MASK) + 1;
#ifdef DEBUG
    sprintf (achBuff, "dSectorEndAddr: %08x\n",(int)dSectorEndAddr);
    vConsoleWrite(achBuff);
#endif
    /* flash_base_addr is byte-aligned */
    psDev->dGbeFlashBase = dSectorBaseAddr << FLASH_SECTOR_ADDR_SHIFT;

#ifdef DEBUG
    sprintf (achBuff, "psDev->dGbeFlashBase: %08x\n",(int)psDev->dGbeFlashBase);
    vConsoleWrite(achBuff);
#endif
    psDev->dFlashBankSize = (dSectorEndAddr - dSectorBaseAddr)
                                << FLASH_SECTOR_ADDR_SHIFT;
    psDev->dFlashBankSize /= 2;
    psDev->dFlashBankSize /= sizeof(UINT16);

#ifdef DEBUG
    sprintf (achBuff, "psDev->dFlashBankSize: %08x\n",(int)psDev->dFlashBankSize);
    vConsoleWrite(achBuff);
#endif
    psDev->wFlashWordSize = ICH9_SHADOW_RAM_WORDS;
}




static void vReadNvmIch9Lan(DEVICE *psDev, UINT16 wOffset, UINT16 wWords, UINT16 *pwData)
{
	UINT32 dActOffset= 0;
	UINT16 i, dWord;
#ifdef DEBUG
	char  achBuff [80];
#endif

	if ((wOffset >= psDev->wFlashWordSize) || (wWords > psDev->wFlashWordSize - wOffset) || (wWords == 0))
	{

		return;
	}

	dActOffset = (dReadReg825xx(EECD) & EECD_SEC1VAL)? psDev->dFlashBankSize:0;

#ifdef DEBUG
	sprintf (achBuff, "dActOffset: %08x\n", dActOffset);
	vConsoleWrite(achBuff);
#endif

	dActOffset += wOffset;

	for (i = 0; i < wWords; i++)
	 {
		vReadFlashWordIch9Lan(psDev,dActOffset + i,&dWord);
		pwData[i] = dWord;
	}
}



static void vReadFlashWordIch9Lan (DEVICE *psDev, UINT32 dOffset, UINT16 *pwdata)
{
     dOffset <<= 1;
     vReadFlashDataIch9Lan(psDev, dOffset, 2, pwdata);

}



 static void vReadFlashDataIch9Lan (DEVICE *psDev, UINT32 dOffset, UINT8 bSize, UINT16 *pwdata)
 {
      UINT16 wHsfStsReg;
      UINT16 wHsflCtlReg;
      UINT32 dFlashLinearAddr;
      UINT32 dFlashData = 0;
      UINT32 dRetVal = -1;
      UINT8 bCount = 0;
	  UINT16 wTemp;

      if (bSize < 1  || bSize > 2 || dOffset > ICH_FLASH_LINEAR_ADDR_MASK)
      {
	         vConsoleWrite("Error in vReadFlashDataIch9Lan");
              return;
      }

      dFlashLinearAddr = (ICH_FLASH_LINEAR_ADDR_MASK & dOffset) + psDev->dGbeFlashBase;

      do
	  {
           vDelay(1);
           /* Steps */
           dRetVal = dFlashCycleInitIch9Lan(psDev);
           if (dRetVal != 0)
                   break;

           wHsflCtlReg = (UINT16)wReadFlashReg(ICH_FLASH_HSFCTL);
           /* 0b/1b corresponds to 1 or 2 byte size, respectively. */
           wHsflCtlReg &= ~HSFC_FDBC;
           wHsflCtlReg |= ((bSize - 1)<<8);
           wHsflCtlReg &= ~HSFC_FCYCLE;

           wHsflCtlReg |= (0 << 1);
           wHsflCtlReg |= (0 << 2);

           wWriteFlashReg(ICH_FLASH_HSFCTL, wHsflCtlReg);
           dWriteFlashReg(ICH_FLASH_FADDR, dFlashLinearAddr);

           dRetVal = dFlashCycleIch9Lan(psDev,
                                  ICH_FLASH_READ_COMMAND_TIMEOUT);

            if (dRetVal == 0)
			{
                dFlashData = (UINT32)dReadFlashReg(ICH_FLASH_FDATA0);

				if (bSize == 1)
				{
                     *pwdata = (UINT8)(dFlashData & 0x000000FF);
				}
				else if (bSize == 2)
				{
                     *pwdata = (UINT16)(dFlashData & 0x0000FFFF);
                }
                break;
             }
			 else
			 {
				 wTemp = (UINT16)wReadFlashReg(ICH_FLASH_HSFSTS);
				 wHsfStsReg = wTemp;
	 			 if (wHsfStsReg & HSFS_FCERR)
				 {
		  			 continue;
   				 }
				 else if (!(wHsfStsReg & HSFS_FDONE))
				 {
	   				 vConsoleWrite("Timeout error - flash cycle " "did not complete.");
                                break;
				 }
			 }
	  } while (bCount++ < ICH_FLASH_CYCLE_REPEAT_COUNT);
	  return;
 }



static UINT32 dFlashCycleIch9Lan(DEVICE *psDev, UINT32 dTimeOut)
{
        UINT16 wHsfStsReg;
        UINT16 wHsflCtl;
        UINT32 i = 0;

        /* Start a cycle by writing 1 in Flash Cycle Go in Hw Flash Control */
        wHsflCtl = (UINT16)wReadFlashReg(ICH_FLASH_HSFCTL);
		wHsflCtl &= ~HSFC_FGO;
		wHsflCtl |= HSFC_FGO;

		wWriteFlashReg(ICH_FLASH_HSFCTL, wHsflCtl);

        /* wait till FDONE bit is set to 1 */
        do {
				wHsfStsReg = (UINT16)wReadFlashReg(ICH_FLASH_HSFSTS);

                if (wHsfStsReg & HSFS_FDONE)
                        break;
                vDelay(1);
        } while (i++ < dTimeOut);

        if ( ((wHsfStsReg & HSFS_FDONE) == 1) && ((wHsfStsReg & HSFS_FCERR) == 0))
        {
				return 0;
        }

	return -1;
}




static UINT32 dFlashCycleInitIch9Lan (DEVICE *psDev)
{
	 UINT16 wHsfSts;
	 UINT32 dRetVal = -1;
   	 UINT32 i = 0;


  	 wHsfSts = (UINT16)wReadFlashReg (ICH_FLASH_HSFSTS);

 	 /* Check if the flash descriptor is valid */
 	 if (!(wHsfSts & HSFS_FDV))
	 {
		 vConsoleWrite("Flash descriptor invalid.  "
				 "SW Sequencing must be used.\n");
		 return -1;
	 }

  	 /* Clear FCERR and DAEL in hw status by writing 1 */
  	 wHsfSts |= (HSFS_FCERR |HSFS_AEL);
  	 wWriteFlashReg(ICH_FLASH_HSFSTS, wHsfSts);


  	 while ((wHsfSts & HSFS_FCERR))
  	 {
			wHsfSts |= (HSFS_FCERR |HSFS_AEL);
  		  	wWriteFlashReg(ICH_FLASH_HSFSTS, wHsfSts);
  		    wHsfSts = (UINT16)wReadFlashReg (ICH_FLASH_HSFSTS);
  	 }

  	 wHsfSts = (UINT16)wReadFlashReg (ICH_FLASH_HSFSTS);

	 if ((wHsfSts & HSFS_SCIP) == 0)
	 {
 		  wHsfSts  |= HSFS_FDONE;
		  wWriteFlashReg(ICH_FLASH_HSFSTS, wHsfSts);
		  dRetVal = 0;
	 }
	 else
	 {
		 for (i = 0; i < ICH_FLASH_READ_COMMAND_TIMEOUT; i++)
		 {
			 wHsfSts = (UINT16)wReadFlashReg(ICH_FLASH_HSFSTS);

			 if ((wHsfSts & HSFS_SCIP) == 0)
			 {
	   			 dRetVal = 0;
				 break;
  			 }
			 vDelay(1);
		 }
		 if (dRetVal == 0)
		 {
             wHsfSts |= HSFS_FDONE;
			 wWriteFlashReg(ICH_FLASH_HSFSTS, wHsfSts);
		 }
		 else
		 {
			 vConsoleWrite("Flash controller busy, cannot get access");
		 }
	 }
	 return dRetVal;
}


#ifdef DEBUG
static UINT32 dGetPhyId(DEVICE *psDev)
{
     UINT16 wPhyId;
//     UINT32 dRevision;
     UINT32 dPhyId;
     //char achBuff[80];

     wPhyId = ReadPhyRegister(psDev, PHY_ID1);
     dPhyId = (UINT32)(wPhyId << 16);
     vDelay (2);
     //sprintf (achBuff, "PHY ID1 %x of 82567", (int)wPhyId);
     //vConsoleWrite (achBuff);

     wPhyId = ReadPhyRegister(psDev, PHY_ID2);
     dPhyId   |= (UINT32)(wPhyId & PHY_REVISION_MASK);
     vDelay (2);
    // sprintf (achBuff, "PHY ID2 %x of 82567", (int)wPhyId);
   //  vConsoleWrite (achBuff);

//     dRevision = (UINT32)(wPhyId & ~PHY_REVISION_MASK);

     return ((UINT32)dPhyId);
}
#endif




static UINT16 ReadPhyRegister_BM(DEVICE *psDev, UINT32 Offset, UINT16 *pData)
{
    UINT16  dRetVal;
    UINT32 dPageSelect = 0;
    UINT32 dPage = Offset >> IGP_PAGE_SHIFT;
    UINT32 dPageShift = 0;

    /* Page 800 works differently than the rest so it has its own func */
    if (dPage == BM_WUC_PAGE)
    {
	    vConsoleWrite ("In Wake Up Phy\n");
        vPhyWakeupReg_BM(psDev, Offset, pData, 1);
        dRetVal = *pData;
        return dRetVal;
    }

    psDev->bPhyAddress = vGetPhyAddrForPage_BM(psDev,dPage, Offset);

    if (Offset > MAX_PHY_MULTI_PAGE_REG)
    {
         /* Page select is register 31 for phy address 1 and 22 for
          * phy address 2 and 3. Page select is shifted only for phy address 1.
          */

          if (psDev->bPhyAddress == 1)
          {
              dPageShift  = IGP_PAGE_SHIFT;
              dPageSelect = IGP01E1000_PHY_PAGE_SELECT;
          }
          else
          {
              dPageShift  = 0;
              dPageSelect = BM_PHY_PAGE_SELECT;
          }
          /* Page is shifted left, PHY expects (page x 32) */
          WritePhyRegister (psDev, dPageSelect, (dPage << dPageShift));
     }
     dRetVal = ReadPhyRegister (psDev, MAX_PHY_REG_ADDRESS & Offset);
     *pData = dRetVal;

     if ((psDev->bController == CTRLT_82567) || (psDev->bController == CTRLT_82577) || (psDev->bController == CTRLT_82579))
       psDev->bPhyAddress = 2;
     else
       psDev->bPhyAddress = 1;

	 return dRetVal;
}


static UINT8 vGetPhyAddrForPage_BM (DEVICE *psDev,UINT32 dPage, UINT32 Offset)
{
	 UINT8 bPhyAddr = 2;

	 if ( (psDev->bController == CTRLT_82567) || (psDev->bController == CTRLT_82577) || (psDev->bController == CTRLT_82579))
	 {
	    if ((dPage >= 768) || (dPage == 0 && dPage == 25) || (dPage == 31))
           bPhyAddr = 1;
	 }
	 else
	 {
		 bPhyAddr = 1;
	 }

     return bPhyAddr;
}


static void WritePhyRegister_BM(DEVICE *psDev, UINT32 Offset, UINT16 Data)
{
    UINT32 dPageSelect = 0;
    UINT32 dPage = Offset >> IGP_PAGE_SHIFT;
    UINT32 dPageShift = 0;

    /* Page 800 works differently than the rest so it has its own func */
    if (dPage == BM_WUC_PAGE)
    {
	    vConsoleWrite ("In Wake Up Phy\n");
        vPhyWakeupReg_BM(psDev, Offset,&Data,0);
    }

    psDev->bPhyAddress = vGetPhyAddrForPage_BM(psDev,dPage, Offset);

    if (Offset > MAX_PHY_MULTI_PAGE_REG)
    {
         /* Page select is register 31 for phy address 1 and 22 for
          * phy address 2 and 3. Page select is shifted only for
          * phy address 1. */

          if (psDev->bPhyAddress == 1)
          {
              dPageShift  = IGP_PAGE_SHIFT;
              dPageSelect = IGP01E1000_PHY_PAGE_SELECT;
          }
          else
          {
              dPageShift  = 0;
              dPageSelect = BM_PHY_PAGE_SELECT;
          }

          /* Page is shifted left, PHY expects (page x 32) */
          WritePhyRegister (psDev, dPageSelect,(dPage << dPageShift));
     }

     WritePhyRegister (psDev, MAX_PHY_REG_ADDRESS & Offset,Data);

      if (psDev->bController == CTRLT_82567 || (psDev->bController == CTRLT_82577) || (psDev->bController == CTRLT_82579))
       psDev->bPhyAddress = 2;
     else
       psDev->bPhyAddress = 1;
}



static void vPhyWakeupReg_BM(DEVICE *psDev, UINT32 Offset, UINT16 *pData, UINT8 read)
{
    UINT16  reg = BM_PHY_REG_NUM(Offset);
    UINT16 wPhyReg = 0;

    /* Gig must be disabled for MDIO accesses to page 800 */
    if ((psDev->bController == CTRLT_82577 || psDev->bController == CTRLT_82579)
        && (!((dReadReg825xx(ICH9_PHY_CTRL)) & PHY_CTRL_GBE_DISABLE)))
	{
        vConsoleWrite("Attempting to access page 800 while gig enabled.\n");
	}
	
    /* All operations in this function are phy address 1 */
    psDev->bPhyAddress =1;

    /* Set page 769 */
    WritePhyRegister(psDev, IGP01E1000_PHY_PAGE_SELECT,
                             (BM_WUC_ENABLE_PAGE << IGP_PAGE_SHIFT));

    wPhyReg = ReadPhyRegister (psDev, BM_WUC_ENABLE_REG );


    /* First clear bit 4 to avoid a power state change */
    wPhyReg &= ~(BM_WUC_HOST_WU_BIT);
    WritePhyRegister(psDev, BM_WUC_ENABLE_REG, wPhyReg);

    /* Write bit 2 = 1, and clear bit 4 to 769_17 */
    WritePhyRegister(psDev, BM_WUC_ENABLE_REG,
    		wPhyReg | BM_WUC_ENABLE_BIT);


    /* Select page 800 */
     WritePhyRegister(psDev,IGP01E1000_PHY_PAGE_SELECT, (BM_WUC_PAGE << IGP_PAGE_SHIFT));
      /* Write the page 800 offset value using opcode 0x11 */
     WritePhyRegister(psDev, BM_WUC_ADDRESS_OPCODE, reg);

    if (read) {
            /* Read the page 800 value using opcode 0x12 */
            *pData = ReadPhyRegister (psDev, BM_WUC_DATA_OPCODE);
    } else {
            /* Write the page 800 value using opcode 0x12 */
    	WritePhyRegister(psDev, BM_WUC_DATA_OPCODE,*pData);
    }

    /*
     * Restore 769_17.2 to its original value
     * Set page 769
     */
    WritePhyRegister(psDev, IGP01E1000_PHY_PAGE_SELECT,
                             (BM_WUC_ENABLE_PAGE << IGP_PAGE_SHIFT));

    /* Clear 769_17.2 */
   WritePhyRegister(psDev, BM_WUC_ENABLE_REG, wPhyReg);

 if (psDev->bController == CTRLT_82567 || (psDev->bController == CTRLT_82577) || (psDev->bController == CTRLT_82579))
   psDev->bPhyAddress = 2;
 else
   psDev->bPhyAddress = 1;


}



UINT32 ReadPhyRegister_BM2(DEVICE *psDev, UINT32 offset, UINT16 *data)
{
        UINT32 ret_val = E__OK;
        UINT16 page = (UINT16)(offset >> IGP_PAGE_SHIFT);


       /* Page 800 works differently than the rest so it has its own func */
       if (page == BM_WUC_PAGE)
     {
	    vConsoleWrite ("In Wake Up Phy\n");
               vPhyWakeupReg_BM(psDev, offset,data,0);
     }
       psDev->bPhyAddress = 1;

        if (offset > MAX_PHY_MULTI_PAGE_REG) {

                /* Page is shifted left, PHY expects (page x 32) */
                WritePhyRegister(psDev, BM_PHY_PAGE_SELECT,page);
        }

        *data = ReadPhyRegister(psDev, MAX_PHY_REG_ADDRESS & offset);

        return ret_val;
}


UINT32 WritePhyRegister_BM2(DEVICE *psDev, UINT32 offset, UINT16 data)
{
       UINT32 ret_val = E__OK;
       UINT16 page = (UINT16)(offset >> IGP_PAGE_SHIFT);

      /* Page 800 works differently than the rest so it has its own func */
       if (page == BM_WUC_PAGE)
     {
	    vConsoleWrite ("In Wake Up Phy\n");
               vPhyWakeupReg_BM(psDev, offset,&data,0);
     }

     psDev->bPhyAddress = 1;

        if (offset > MAX_PHY_MULTI_PAGE_REG) {
                /* Page is shifted left, PHY expects (page x 32) */
                WritePhyRegister(psDev, BM_PHY_PAGE_SELECT,
                                                   page);
        }

        WritePhyRegister(psDev, MAX_PHY_REG_ADDRESS & offset,data);

        return ret_val;
}

UINT32 copper_link_setup_82577(DEVICE *psDev)
{
	UINT32 ret_val=E__OK;
	UINT16 phy_data;

	/* Enable CRS on TX. This must be set for half-duplex operation. */
	ReadPhyRegister_LV(psDev, I82577_CFG_REG, &phy_data,0);
	phy_data |= I82577_CFG_ASSERT_CRS_ON_TX;

	/* Enable downshift */
	phy_data |= I82577_CFG_ENABLE_DOWNSHIFT;

	WritePhyRegister_LV(psDev, I82577_CFG_REG, phy_data,0);
	return ret_val;
}


static UINT32 ReadPhyRegister_LV (DEVICE *psDev,UINT32 dOffset,UINT16 *pData, UINT8 flag)
{

    UINT32 ret_val;
    UINT16 page = BM_PHY_REG_PAGE(dOffset);
    UINT16 reg = BM_PHY_REG_NUM(dOffset);

   /* Page 800 works differently than the rest so it has its own func */
    if (page == BM_WUC_PAGE)
    {
        vPhyWakeupReg_BM(psDev, dOffset,pData, 1);
		return E__OK;
    }

    if (page > 0 && page < HV_INTC_FC_PAGE_START)
	{
        ret_val = access_phy_debug_regs_hv(psDev, dOffset, pData, 1);
        return ret_val;
    }

     psDev->bPhyAddress = get_phy_addr_for_hv_page(page);

    if (page == HV_INTC_FC_PAGE_START)
            page = 0;

   if (reg > MAX_PHY_MULTI_PAGE_REG)
   {
        UINT32 phy_addr = psDev->bPhyAddress;

        psDev->bPhyAddress = 1;

        /* Page is shifted left, PHY expects (page x 32) */
        WritePhyRegister (psDev,IGP01E1000_PHY_PAGE_SELECT,
                              (page << IGP_PAGE_SHIFT));
        psDev->bPhyAddress = phy_addr;
    }



    *pData = ReadPhyRegister(psDev, (MAX_PHY_REG_ADDRESS & reg));

    if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		psDev->bPhyAddress = 2;
	}

    return E__OK;
}

static UINT32  access_phy_debug_regs_hv(DEVICE *psDev, UINT32 dOffset,
                                          UINT16 *pData, UINT8 read)
{
    UINT32 addr_reg = 0;
    UINT32 data_reg = 0;

    /* This takes care of the difference with desktop vs mobile phy */
    addr_reg = (psDev->bController == CTRLT_82577 || psDev->bController == CTRLT_82579) ?
               I82577_ADDR_REG:I82578_ADDR_REG;
    data_reg = addr_reg + 1;

    /* All operations in this function are phy address 2 */
    psDev->bPhyAddress = 2;

    /* masking with 0x3F to remove the page from offset */
     WritePhyRegister (psDev, addr_reg, (UINT16)dOffset & 0x3F);


    /* Read or write the data value next */
    if (read)
            *pData= ReadPhyRegister(psDev, data_reg);
    else
            WritePhyRegister (psDev, data_reg, *pData);

    return E__OK;
}

static UINT32  get_phy_addr_for_hv_page(UINT32 page)
{
    UINT32 phy_addr = 2;

    if (page >= HV_INTC_FC_PAGE_START)
            phy_addr = 1;

    return phy_addr;
}

static UINT32 WritePhyRegister_LV (DEVICE *psDev, UINT32 dOffset, UINT16 data, UINT8 locked)
{

    UINT32 ret_val;
    UINT16 page = BM_PHY_REG_PAGE(dOffset);
    UINT16 reg = BM_PHY_REG_NUM(dOffset);


    /* Page 800 works differently than the rest so it has its own func */
    if (page == BM_WUC_PAGE) {
            vPhyWakeupReg_BM(psDev, dOffset,&data, 0);
            return E__OK;
    }

    if (page > 0 && page < HV_INTC_FC_PAGE_START)
	{
        ret_val = access_phy_debug_regs_hv(psDev, dOffset, &data, 0);
        return ret_val;
    }

    psDev->bPhyAddress = get_phy_addr_for_hv_page(page);

    if (page == HV_INTC_FC_PAGE_START)
            page = 0;



    if (reg > MAX_PHY_MULTI_PAGE_REG) {
            UINT32 phy_addr = psDev->bPhyAddress;

            psDev->bPhyAddress = 1;
            /* Page is shifted left, PHY expects (page x 32) */
            WritePhyRegister (psDev,IGP01E1000_PHY_PAGE_SELECT,(page << IGP_PAGE_SHIFT));
            psDev->bPhyAddress = phy_addr;
    }


    WritePhyRegister (psDev, (MAX_PHY_REG_ADDRESS & reg),data);
    if (psDev->bController == CTRLT_82567 || (psDev->bController == CTRLT_82577) || (psDev->bController == CTRLT_82579))
               psDev->bPhyAddress = 2;

    return E__OK;
}

#ifdef DEBUG
static UINT32 dGetAutoRdDone(DEVICE *psDev)
{
   UINT32 i = 0;
   UINT32 dRetVal = 1;

   if (psDev->bController == CTRLT_82567)
   {
	   while (i < AUTO_READ_DONE_TIMEOUT)
	   {
		  if (dReadReg825xx(EECD) & EECD_AUTO_RD)
	          break;
	      vDelay(1);
	      i++;
	   }

	   if (i == AUTO_READ_DONE_TIMEOUT)
	   {
	     vConsoleWrite("Auto read by HW from NVM has not completed.\n");
	     dRetVal = -1;
	     return dRetVal;
	    }

	    return dRetVal;
   }
   return dRetVal;
}
#endif

void vPhyWorkAround1(DEVICE *psDev)
{
	UINT32 dPhyCtrl = 0;

	dPhyCtrl = dReadReg825xx(ICH9_PHY_CTRL);

    dPhyCtrl &= 0xFFFFFFC0;
    dPhyCtrl |= 0x8D;
    vWriteReg825xx(ICH9_PHY_CTRL, dPhyCtrl);
    dPhyCtrl = dReadReg825xx(ICH9_PHY_CTRL);
}





/*****************************************************************************
 * EthInterfaceSelect: Ethernet Front/Rear Interface Selection
 *
 * RETURNS: Test status
 */
#define	E__TEST_WRONG_NUMBER_OF_PARAMS		E__BIT

TEST_INTERFACE (EthInterfaceSelect, "Ethernet IF Selection")
{
	UINT32 rt;
	SelEthIf_PARAMS params;

	if ((adTestParams[0] != 2) )
		return E__TEST_WRONG_NUMBER_OF_PARAMS;

	params.cnt   = adTestParams[1];
	params.Iface = adTestParams[2];
	board_service(SERVICE__BRD_SELECT_ETH_IF, &rt, &params);

	return rt;

}


void vDoWorkarounds(DEVICE *psDev)
{
	UINT16 phy_data=0,reg=0;

	if(psDev->bController == CTRLT_82577 /*|| psDev->bController == CTRLT_82579*/)
	{
		set_mdio_slow_mode_hv(psDev);
		vDelay(10);
        WritePhyRegister_LV(psDev,PHY_REG(769, 25), 0x4431,0);
        WritePhyRegister_LV(psDev,PHY_REG(770, 16), 0xa204,0);
        psDev->bPhyAddress = 1;
        WritePhyRegister(psDev,IGP01E1000_PHY_PAGE_SELECT,0);
        psDev->bPhyAddress = 2;
        vDoK1Workaround(psDev);
        ReadPhyRegister_LV(psDev,PHY_REG(BM_PORT_CTRL_PAGE, 17),&phy_data,1);
        WritePhyRegister_LV(psDev,PHY_REG(BM_PORT_CTRL_PAGE, 17),phy_data & 0x00FF,0);
        ReadPhyRegister_LV(psDev, BM_WUC, &reg,0);
       /* WritePhyRegister_LV(psDev,PHY_REG(800,0),0x0067,0);
        WritePhyRegister_LV(psDev,PHY_REG(800,2),0x00ff,0);*/
        sw_lcd_config_ich8lan(psDev);
        oem_bits_config_ich8lan(psDev,1);
	}
}

void vDoK1Workaround(DEVICE *psDev)
{
	UINT16 k1_enable = 0;
	UINT16 kmrn_reg = 0;
	UINT16  status_reg=0;
	UINT32 ctrl_ext=0,ctrl_reg=0,reg=0;
	UINT8 link = 0;


	if(psDev->bController == CTRLT_82577 /*|| psDev->bController == CTRLT_82579*/ )
	{
		/* Check if link is present */
		phy_has_link_generic(psDev,COPPER_LINK_UP_LIMIT,10,&link);
		if (link)
		{

			ReadPhyRegister_LV(psDev, HV_M_STATUS,&status_reg,0);

			status_reg &= HV_M_STATUS_LINK_UP |
			              HV_M_STATUS_AUTONEG_COMPLETE |
				          HV_M_STATUS_SPEED_MASK;

			if (status_reg == (HV_M_STATUS_LINK_UP |
				               HV_M_STATUS_AUTONEG_COMPLETE |
				               HV_M_STATUS_SPEED_1000))
			{
				k1_enable = 0;
			}

			WritePhyRegister_LV(psDev, PHY_REG(770, 19),0x0100,0);
		}
		else
		{
			/* PHY 1Gig linkdown Work around */
			WritePhyRegister_LV(psDev, PHY_REG(770, 19), 0x4100,0);
		}

		/* Read Kumeran register */
		read_kmrn_reg(psDev,E1000_KMRNCTRLSTA_K1_CONFIG,&kmrn_reg);

		if (k1_enable)
			kmrn_reg |= E1000_KMRNCTRLSTA_K1_ENABLE;
		else
			kmrn_reg &= ~E1000_KMRNCTRLSTA_K1_ENABLE;

		write_kmrn_reg(psDev,E1000_KMRNCTRLSTA_K1_CONFIG,kmrn_reg);
		vDelay(20);

		ctrl_ext = dReadReg825xx(CTRL_EXT);
		ctrl_reg = dReadReg825xx(CTRL);
		reg = ctrl_reg & ~(CTRL_SPEED1000 | CTRL_SPEED100);
		reg |= CTRL_FRCSPD;
		vWriteReg825xx(CTRL, reg);
        vWriteReg825xx(CTRL_EXT, ctrl_ext | CTRL_EXT_SPD_BYPS);
        vDelay(20);
        vWriteReg825xx(CTRL, ctrl_reg);
        vWriteReg825xx(CTRL_EXT, ctrl_ext);
        vDelay(20);
	}
}

UINT32 sw_lcd_config_ich8lan (DEVICE *psDev)
{

	UINT32 i=0, data=0, cnf_size=0, cnf_base_addr=0, sw_cfg_mask=0;
	UINT16 word_addr, reg_data, reg_addr, phy_page = 0;


	sw_cfg_mask = E1000_FEXTNVM_SW_CONFIG_ICH8M;

	data = dReadReg825xx (FEXTNVM);

	if (!(data & sw_cfg_mask))
	{
		vConsoleWrite("Inside data & sw_cfg_mask\n");
        return E__OK;
	}

        /*
         * Make sure HW does not configure LCD from PHY
         * extended configuration before SW configuration
         */
        data = dReadReg825xx(ICH9_EXTCNF_CTRL);

        if (data & E1000_EXTCNF_CTRL_LCD_WRITE_ENABLE)
            return E__OK;

        cnf_size = dReadReg825xx(EXTCNF_SIZE);
        cnf_size &= E1000_EXTCNF_SIZE_EXT_PCIE_LENGTH_MASK;
        cnf_size >>= E1000_EXTCNF_SIZE_EXT_PCIE_LENGTH_SHIFT;
        if (!cnf_size)
        {
        vConsoleWrite("Inside !cn_size\n");
            return E__OK;
        }

        cnf_base_addr = data & E1000_EXTCNF_CTRL_EXT_CNF_POINTER_MASK;
        cnf_base_addr >>= E1000_EXTCNF_CTRL_EXT_CNF_POINTER_SHIFT;

        if (!(data & E1000_EXTCNF_CTRL_OEM_WRITE_ENABLE) && (psDev->bController == CTRLT_82577 || psDev->bController == CTRLT_82579) )
        {
            /*
             * HW configures the SMBus address and LEDs when the
             * OEM and LCD Write Enable bits are set in the NVM.
             * When both NVM bits are cleared, SW will configure
             * them instead.
             */
            vConsoleWrite("Inside STRAP and SMBUS config\n");
            data = dReadReg825xx(STRAP);
            data &= E1000_STRAP_SMBUS_ADDRESS_MASK;
            reg_data = data >> E1000_STRAP_SMBUS_ADDRESS_SHIFT;
            reg_data |= HV_SMB_ADDR_PEC_EN | HV_SMB_ADDR_VALID;
            WritePhyRegister_LV(psDev, HV_SMB_ADDR,reg_data,0);
            data = dReadReg825xx(LEDCTL);
            WritePhyRegister_LV(psDev, HV_LED_CONFIG,(UINT16)data,0);
        }

        /* cnf_base_addr is in DWORD */
        word_addr = (UINT16)(cnf_base_addr << 1);
        for (i = 0; i < cnf_size; i++)
        {
        	vReadNvmIch9Lan(psDev, (word_addr + i * 2), 1,&reg_data);
        	vReadNvmIch9Lan(psDev, (word_addr + i * 2 + 1),1, &reg_addr);

            /* Save off the PHY page for future writes. */
            if (reg_addr == IGP01E1000_PHY_PAGE_SELECT)
			{
                    phy_page = reg_data;
                    continue;
            }

            reg_addr &= PHY_REG_MASK;
            reg_addr |= phy_page;

            WritePhyRegister_LV(psDev, (UINT32)reg_addr,reg_data,0);
        }

        return E__OK;
}

UINT32 oem_bits_config_ich8lan (DEVICE *psDev, UINT8 d0_state )
{

      UINT32 mac_reg;
      UINT16 oem_reg;
      UINT32 ret_val = E__OK;

      mac_reg = dReadReg825xx(FEXTNVM);
      if (!(mac_reg & E1000_FEXTNVM_SW_CONFIG_ICH8M))
      {
        	vConsoleWrite("(mac_reg & E1000_FEXTNVM_SW_CONFIG_ICH8M)\n");
            return (ret_val);
      }

        mac_reg = dReadReg825xx(PHY_CTRL);

        ReadPhyRegister_LV(psDev, HV_OEM_BITS, &oem_reg,0);

        oem_reg &= ~(HV_OEM_BITS_GBE_DIS | HV_OEM_BITS_LPLU);

        if (d0_state)
        {
            if (mac_reg & E1000_PHY_CTRL_GBE_DISABLE)
                oem_reg |= HV_OEM_BITS_GBE_DIS;

            if (mac_reg & E1000_PHY_CTRL_D0A_LPLU)
                oem_reg |= HV_OEM_BITS_LPLU;
        }
        else
        {
            if (mac_reg & E1000_PHY_CTRL_NOND0A_GBE_DISABLE)
                oem_reg |= HV_OEM_BITS_GBE_DIS;

            if (mac_reg & E1000_PHY_CTRL_NOND0A_LPLU)
                oem_reg |= HV_OEM_BITS_LPLU;
        }

       /* Restart auto-neg to activate the bits */
        if (!check_reset_block_ich8lan(psDev))
            oem_reg |= HV_OEM_BITS_RESTART_AN;

        WritePhyRegister_LV(psDev, HV_OEM_BITS, oem_reg,0);

        return ret_val;

}

static UINT32 check_reset_block_ich8lan(DEVICE *psDev)
{
    UINT32 fwsm;

    fwsm = dReadReg825xx(FWSM);
    return (fwsm & E1000_ICH_FWSM_RSPCIPHY) ? E__OK
                                            : E1000_BLK_PHY_RESET;
}


static UINT32 write_kmrn_reg(DEVICE *psDev, UINT32 offset, UINT16 data)
{
    UINT32 kmrnctrlsta;

    kmrnctrlsta = ((offset << E1000_KMRNCTRLSTA_OFFSET_SHIFT) &
                   E1000_KMRNCTRLSTA_OFFSET) | data;
    vWriteReg825xx(KMRNCTRLSTA, kmrnctrlsta);

    vDelay(2);

	return E__OK;
}


static UINT32 read_kmrn_reg(DEVICE *psDev, UINT32 offset, UINT16 *data)
{
    UINT32 kmrnctrlsta;

    kmrnctrlsta = ((offset << E1000_KMRNCTRLSTA_OFFSET_SHIFT) &
                   E1000_KMRNCTRLSTA_OFFSET) | E1000_KMRNCTRLSTA_REN;
    vWriteReg825xx(KMRNCTRLSTA, kmrnctrlsta);

    vDelay(2);

    kmrnctrlsta = dReadReg825xx(KMRNCTRLSTA);
    *data = (UINT16)kmrnctrlsta;

    return E__OK;
}


static UINT32 phy_has_link_generic(DEVICE *psDev, UINT32 iterations,
                               UINT32 usec_interval, UINT8 *success)
{
    UINT32 ret_val = E__OK;
    UINT16 i, phy_status;

    for (i = 0; i < iterations; i++)
	{
        /*
         * Some PHYs require the PHY_STATUS register to be read
         * twice due to the link bit being sticky.  No harm doing
         * it across the board.
         */
		ReadPhyRegister_LV(psDev, PHY_STATUS,&phy_status,0);

        /*
         * If the first read fails, another entity may have
         * ownership of the resources, wait and try again to
         * see if they have relinquished the resources yet.
         */
        vDelay(usec_interval);
        ReadPhyRegister_LV(psDev, PHY_STATUS,&phy_status,0);

        if (phy_status & MII_SR_LINK_STATUS)
            break;

        if (usec_interval >= 1000)
            vDelay(usec_interval/1000);
        else
            vDelay(usec_interval);
    }

    *success = (i < iterations) ? 1 : 0;

    if (*success)
    	vConsoleWrite ("Link UP\n");
    else
    	vConsoleWrite ("Link Down\n");

    return ret_val;
}

void config_collision_dist (DEVICE *psDev)
{
    UINT32 tctl;

    tctl = dReadReg825xx(TCTL);

    tctl &= ~E1000_TCTL_COLD;
    tctl |= E1000_COLLISION_DISTANCE << E1000_COLD_SHIFT;

    vWriteReg825xx(TCTL, tctl);
    e1e_flush();
}

static UINT32 set_mdio_slow_mode_hv(DEVICE *psDev)
{
    UINT16 data;

    ReadPhyRegister_LV(psDev, HV_KMRN_MODE_CTRL,&data,0);

    data |= HV_KMRN_MDIO_SLOW;

    WritePhyRegister_LV(psDev, HV_KMRN_MODE_CTRL, data,0);

    return E__OK;
}

static void  lan_init_done_ich8lan(DEVICE *psDev)
{
    UINT32 data, loop = E1000_ICH8_LAN_INIT_TIMEOUT,status = 0;

    /* Wait for basic configuration completes before proceeding */
    do {
            data = dReadReg825xx(STATUS);
            data &= E1000_STATUS_LAN_INIT_DONE;
            vDelay(100);
    } while ((!data) && --loop);

    /*
     * If basic configuration is incomplete before the above loop
     * count reaches 0, loading the configuration from NVM will
     * leave the PHY in a bad state possibly resulting in no link.
     */
    if (loop == 0)
        vConsoleWrite("LAN_INIT_DONE not set, increase timeout\n");

    /* Clear the Init Done bit for the next init event */
    data = dReadReg825xx(STATUS);
    data &= ~E1000_STATUS_LAN_INIT_DONE;
    vWriteReg825xx(STATUS, data);

    vDelay(100);
    /* Clear PHY Reset Asserted bit */
    status = dReadReg825xx(STATUS);
    if (status & E1000_STATUS_PHYRA)
    	vWriteReg825xx(STATUS, status & ~E1000_STATUS_PHYRA);
    else
    	vConsoleWrite("PHY Reset Asserted not set - needs delay\n");
}


void  phy_force_speed_duplex_setup(DEVICE *psDev, UINT16 *phy_ctrl,UINT32 dDuplex,UINT32 dSpeed)
{

    UINT32 ctrl = 0;
    /* Force speed/duplex on the mac */
    ctrl = dReadReg825xx(CTRL);
    ctrl |= (CTRL_FRCSPD | CTRL_FRCDPLX);
    ctrl &= ~CTRL_SPD_SEL;

    /* Disable Auto Speed Detection */
    ctrl &= ~CTRL_ASDE;

    /* Disable autoneg on the phy */
    *phy_ctrl &= ~PHY_CTRL_AUTO_NEG;

    /* Forcing Full or Half Duplex? */
    if (dDuplex == ALL_HALF_DUPLEX)
    {
        ctrl &= ~CTRL_FD;
        *phy_ctrl &= ~PHY_CTRL_DUPLEX;
        vConsoleWrite("Half Duplex\n");
    }
	else
    {
        ctrl |= CTRL_FD;
        *phy_ctrl |= PHY_CTRL_DUPLEX;
        vConsoleWrite("Full Duplex\n");
    }

    /* Forcing 10mb or 100mb? */
    if (dSpeed == ALL_100_SPEED)
    {
        ctrl |= CTRL_SPEED100;
        *phy_ctrl |= PHY_CTRL_SPEED_100;
        *phy_ctrl &= ~(PHY_CTRL_SPEED_1000 | PHY_CTRL_SPEED_10);
        vConsoleWrite("Forcing 100mb\n");
    }
    else if (dSpeed == ALL_10_SPEED)
    {
        ctrl &= ~(CTRL_SPEED1000 | CTRL_SPEED10);
        *phy_ctrl |= PHY_CTRL_SPEED_10;
        *phy_ctrl &= ~(PHY_CTRL_SPEED_1000 | PHY_CTRL_SPEED_100);
        vConsoleWrite("Forcing 10mb\n");
    }
    else
    {
		ctrl &= ~(CTRL_SPEED10 | CTRL_SPEED100);
		ctrl |= CTRL_SPEED100;
        *phy_ctrl |= PHY_CTRL_SPEED_1000;
        *phy_ctrl &= ~(PHY_CTRL_SPEED_10 | PHY_CTRL_SPEED_100);
         vConsoleWrite("Forcing 1000mb\n");
    }

    config_collision_dist(psDev);
    vWriteReg825xx(CTRL, ctrl);
}

void power_up_phy_copper(DEVICE *psDev)
{
    UINT16 mii_reg = 0;

    /* The PHY will retain its settings across a power down/up cycle */
    mii_reg =  ReadPhyRegister(psDev, PHY_CTRL);
    mii_reg &= ~PHY_CTRL_POWER_DOWN;
    WritePhyRegister(psDev, PHY_CTRL, mii_reg);
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
	if (psDev->bController != CTRLT_82599)
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
} /* vClear8257xStatistics () */


/*******************************************************************
 * vDisplay8257xStats: formatted display of statistical counters
 *
 * RETURNS: none
 */

static void vDisplay8257xStats
(
	DEVICE* psDev		/* pointer to device data structure */
)
{

	char	achBuffer[LINE_BUFF_SIZE];


	/* Display the generalised error stats */

	vConsoleWrite ("Error statistics\n");
	if (psDev->bController != CTRLT_82599)
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
	}
#ifdef INCLUDE_I82599
	else
	{
		sprintf (achBuffer, "CRC errors ......... %08X  Illegal Byte Err ... %08X\n",
			dReadReg825xx (I8259x_CRCERRS), dReadReg825xx (I8259x_ILLERRC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "Error Byte Cnt ..... %08X  MAC Short Pkt Dsc .. %08X\n",
			dReadReg825xx (I8259x_ERRBC), dReadReg825xx (I8259x_MSPDC));
		vConsoleWrite (achBuffer);

		sprintf (achBuffer, "Rx length Err ...... %08X \n",
			dReadReg825xx (I8259x_RLEC));
		vConsoleWrite (achBuffer);

		sprintf (achBuffer, "XON RX count ....... %08X  XOFF RX count ...... %08X\n",
				dReadReg825xx (I8259x_LXONRXC), dReadReg825xx (I8259x_LXOFFRXC));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "XON TX count ....... %08X  XOFF TX count ...... %08X\n",
				dReadReg825xx (I8259x_LXONTXC), dReadReg825xx (I8259x_LXOFFTXC));
		vConsoleWrite (achBuffer);
	}
#endif

	/* Display the TX and RX stats */

	vConsoleWrite ("Transmit and Receive statistics\n");
	if (psDev->bController != CTRLT_82599)
	{
		sprintf (achBuffer, "Total packets TX ... %08X  Total octets TX .... %08X:%08X\n",
				dReadReg825xx (TPT), dReadReg825xx (TOTH), dReadReg825xx (TOTL));
		vConsoleWrite (achBuffer);
	}
#ifdef INCLUDE_I82599
	else
	{
		sprintf (achBuffer, "Total packets TX ... %08X\n",
				dReadReg825xx (TPT));
		vConsoleWrite (achBuffer);
	}
#endif
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
	if (psDev->bController != CTRLT_82599)
	{
		sprintf (achBuffer, "RX no buffers ...... %08X  RX undersize ....... %08X\n",
				dReadReg825xx (RNBC), dReadReg825xx (RUC));
		vConsoleWrite (achBuffer);
	}
#ifdef INCLUDE_I82599
	else
	{
		sprintf (achBuffer, "RX undersize ....... %08X\n",
				dReadReg825xx (RUC));
		vConsoleWrite (achBuffer);
	}
#endif
	sprintf (achBuffer, "RX fragment ........ %08X  RX oversize ........ %08X\n",
			dReadReg825xx (RFC), dReadReg825xx (ROC));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "RX jabber .......... %08X                           \n",
			dReadReg825xx (RJC));
	vConsoleWrite (achBuffer);

} /* vDisplay8257xStats () */


/*******************************************************************
 * vDisplay8257xDiagRegs: display the internal diagnostic registers
 *
 * RETURNS: none
 */
#if 0
static void vDisplay8257xDiagRegs
(
	DEVICE* psDev		/* pointer to device data structure */
)
{
	char	achBuffer[LINE_BUFF_SIZE];
	UINT32 dOffset = 0x400;

	sprintf (achBuffer, "Command reg ........ %08X  Status Reg ......... %08X\n",
			dReadReg825xx (CTRL), dReadReg825xx (STATUS));
	vConsoleWrite (achBuffer);
	if (psDev->bController != CTRLT_82599)
	{
		sprintf (achBuffer, "RX FIFO head ....... %08X  RX FIFO tail ....... %08X\n",
				dReadReg825xx (RDFH + dOffset), dReadReg825xx (RDFT + dOffset));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "RX FIFO head saved . %08X  RX FIFO tail saved . %08X\n",
				dReadReg825xx (RDFHS + dOffset), dReadReg825xx (RDFTS + dOffset));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "RX FIFO packet count %08X                           \n",
				dReadReg825xx (RDFPC + dOffset));
		vConsoleWrite (achBuffer);


		sprintf (achBuffer, "TX FIFO head ....... %08X  TX FIFO tail ....... %08X\n",
				dReadReg825xx (TDFH + dOffset), dReadReg825xx (TDFT + dOffset));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "TX FIFO head saved . %08X  RX FIFO tail saved . %08X\n",
				dReadReg825xx (TDFHS + dOffset), dReadReg825xx (TDFTS + dOffset));
		vConsoleWrite (achBuffer);
		sprintf (achBuffer, "TX FIFO packet count %08X                           \n",
				dReadReg825xx (TDFPC + dOffset));
		vConsoleWrite (achBuffer);
	}
} /* vDisplay8257xDiagRegs () */
#endif

/*******************************************************************
 * vShow8257xStatus: formatted display of status register contents
 *
 * RETURNS: none
 */
#if 0
static void vShow8257xStatus
(
	DEVICE* psDev		/* pointer to device data structure */
)
{
	UINT32	dRegVal;
	UINT16 wLinkStatus, wLinkWidth;
	PCI_PFA pfa;


	if (psDev->bController != CTRLT_82599)
	{
		dRegVal = dReadReg825xx (STATUS);
		if ((dRegVal & STATUS_FD) != 0)		/* Full duplex */
			vConsoleWrite ("Full duplex\n");
		else
			vConsoleWrite ("Half duplex\n");

		if ((dRegVal & STATUS_LU) != 0)		/* Link status */
			vConsoleWrite ("Link up\n");
		else
			vConsoleWrite ("Link not up\n");
	}
#ifdef INCLUDE_I82599
	else
	{
		dRegVal = dReadReg825xx (I8259x_LINKS);
		if ((dRegVal & 0x40000000) != 0)		/* Link status */
			vConsoleWrite ("Link up\n");
		else
			vConsoleWrite ("Link not up\n");
	}
#endif


	if (psDev->bController != CTRLT_82599)
	{
		if ((dRegVal & ~M__STATUS_SPEED) == STATUS_SPEED1000)	/* Link speed */
			vConsoleWrite ("Link: 1-Gbit\n");
		else if ((dRegVal & ~M__STATUS_SPEED) == STATUS_SPEED100)
			vConsoleWrite ("Link: 100-Mbit\n");
		else if ((dRegVal & ~M__STATUS_SPEED) == STATUS_SPEED10)
			vConsoleWrite ("Link: 10-Mbit\n");
		else
			vConsoleWrite ("Link: invalid\n");

		if ((dRegVal & ~M__STATUS_SPEED) == STATUS_ASDV1000) /* Auto-detect speed */
			vConsoleWrite ("Auto-detect: 1-Gbit\n");
		else if ((dRegVal & ~M__STATUS_ASDV) == STATUS_ASDV100)
			vConsoleWrite ("Auto-detect: 100-Mbit\n");
		else if ((dRegVal & ~M__STATUS_ASDV) == STATUS_ASDV10)
			vConsoleWrite ("Auto-detect: 10-Mbit\n");
		else
			vConsoleWrite ("Auto-detect: invalid\n");
	}
#ifdef INCLUDE_I82599
	else
	{
		dRegVal = dReadReg825xx (STATUS);
		if ((dRegVal & 0x80000) != 0)
			vConsoleWrite ("LAN can issue Master requests\n");
		else
			vConsoleWrite ("LAN doesnot issue Master requests\n");
	}
#endif
	if ((psDev->bController == CTRLT_82575EB) ||
		(psDev->bController == CTRLT_82576EB) ||
		(psDev->bController == CTRLT_82576LM) ||
		(psDev->bController == CTRLT_82599)   ||
		(psDev->bController == CTRLT_82580)   ||
		(psDev->bController == CTRLT_I350)    ||
		(psDev->bController == CTRLT_I210))
	{
		/*wLinkStatus = (UINT16)pci_read_reg (psDev->bBus, psDev->bDev, psDev->bFunc,
							0xb2, PCI_WORD);*/
		pfa = PCI_MAKE_PFA(psDev->bBus,psDev->bDev,psDev->bFunc);
	    wLinkStatus = PCI_READ_WORD (pfa, 0xb2);
	}
	else
	{
		pfa = PCI_MAKE_PFA(psDev->bBus,psDev->bDev,psDev->bFunc);
		wLinkStatus = PCI_READ_WORD (pfa, 0xf2);
	}
    wLinkWidth = (wLinkStatus & 0x1f0) >> 4;
    switch (wLinkWidth)
    {
	case 0x1:
		vConsoleWrite ("PCIExpress x1\n");
		break;
	case 0x2:
		vConsoleWrite ("PCIExpress x2\n");
		break;
	case 0x4:
		vConsoleWrite ("PCIExpress x4\n");
		break;
	case 0x8:
		vConsoleWrite ("PCIExpress x8\n");
		break;
	default:
		vConsoleWrite ("PCIExpress width error\n");
		break;
	}

} /* vShow8257xStatus () */

#endif


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
	int i;
	UINT16 wRegVal1, wRegVal2, wRegVal3, wRegVal4;
	char achBuffer[80];

	sprintf (achBuffer, "\n PHY registers (address = %02Xh)\n", psDev->bPhyAddress);
	vConsoleWrite (achBuffer);

	if ((psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
		/* Wake up PHY */
		WritePhyRegister(psDev, 0, (ReadPhyRegister(psDev, 0) & (~0x800)));
		vDelay(2);
	}
								/*CTRLT_82567LM*/
	if ((psDev->bController == CTRLT_82567) ||
		(psDev->bController == CTRLT_82577) ||
		(psDev->bController == CTRLT_82579))
	{
	  for (i = 0; i < 31; i+=4)
	  {
		  wRegVal1 = ReadPhyRegister (psDev, i);
		  wRegVal2 = ReadPhyRegister (psDev, i+1);
		  wRegVal3 = ReadPhyRegister (psDev, i+2);
		  wRegVal4 = ReadPhyRegister (psDev, i+3);
		sprintf (achBuffer, "%3d  0x%04x 0x%04x 0x%04x 0x%04x\n", i,
				wRegVal1, wRegVal2, wRegVal3, wRegVal4);
		vConsoleWrite (achBuffer);
	  }

	}
	else
	{
	  for (i = 0; i < 25; i+=4)
	  {
		wRegVal1 = ReadPhyRegister (psDev, i);
		wRegVal2 = ReadPhyRegister (psDev, i+1);
		wRegVal3 = ReadPhyRegister (psDev, i+2);
		wRegVal4 = ReadPhyRegister (psDev, i+3);
		sprintf (achBuffer, "%3d  0x%04x 0x%04x 0x%04x 0x%04x\n", i,
				wRegVal1, wRegVal2, wRegVal3, wRegVal4);
		vConsoleWrite (achBuffer);
	  }
	}
}
#endif

static void vSwitchI350ToSerdes (DEVICE *psDev)
{
	UINT32 dRegVal;

#ifdef DEBUG
	vConsoleWrite ("Switching COPPER interface to SERDES for SERDES->SERDES loopback test\n");
#endif

	// common

	// disable RCTL.RXEN
	dRegVal = dReadReg825xx (RCTL);
	dRegVal &= ~RCTL_EN;
	vWriteReg825xx(RCTL, dRegVal);

	// disable TCTL.TXEN
	dRegVal = dReadReg825xx (TCTL);
	dRegVal &= ~TCTL_EN;
	vWriteReg825xx(TCTL, dRegVal);

	// set mode CTRL_EXT.LINK_MODE = 11b
	dRegVal = dReadReg825xx (CTRL_EXT);
	dRegVal &= M__CTRLEXT_LINK;
	dRegVal |= CTRLEXT_LINK_TBI;
	vWriteReg825xx(CTRL_EXT, dRegVal);

	// using auto-negotiation
#ifdef DEBUG
	vConsoleWrite ("Using auto-negotiation\n");
#endif

	// set PCS_LCTL.AN_ENABLE
	dRegVal = dReadReg825xx (PCS_LCTL);
	dRegVal |= PCS_LCTL_AN_ENABLE;
	vWriteReg825xx(PCS_LCTL, dRegVal);

	// clear CTRL.FRCSPD
	// clear CTRL.FRCDPLX
	// clear CTRL.ILOS
	// set CTRL.SLU
	dRegVal = dReadReg825xx (CTRL);
	dRegVal &= ~(CTRL_FRCSPD + CTRL_FRCDPLX);
//	dRegVal &= ~(CTRL_FRCSPD + CTRL_FRCDPLX + CTRL_ILOS);
	dRegVal |= CTRL_SLU;
	vWriteReg825xx(CTRL, dRegVal);

	// clear PCS_LCTL.FSD
	// clear PCS_LCTL.force flow control
	// set speed PCS_LCTL.FSV = 10b
	// set PCS_LCTL.AN_TIMEOUT_EN
	dRegVal = dReadReg825xx (PCS_LCTL);
	dRegVal &= ~(PCS_LCTL_FSD + PCS_LCTL_FSV_100 + PCS_LCTL_FORCE_FCTRL);
	dRegVal |= PCS_LCTL_FSV_1000 + PCS_LCTL_AN_TIMEOUT;
	vWriteReg825xx(PCS_LCTL, dRegVal);

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
	vWriteReg825xx(CTRL, dRegVal);

	// clear PCS_LCTL.AN_ENABLE
	// set PCS_LCTL.FSD
	// set PCS_LCTL.force flow control
	// set speed PCS_LCTL.FSV = 10b
	// set PCS_LCTL.FDV
	// clear PCS_LCTL.AN_TIMEOUT_EN
	dRegVal = dReadReg825xx (PCS_LCTL);
	dRegVal &= ~(PCS_LCTL_AN_ENABLE + PCS_LCTL_FSV_100 + PCS_LCTL_AN_TIMEOUT);
	dRegVal |= PCS_LCTL_FSD + PCS_LCTL_FDV_FULL + PCS_LCTL_FSV_1000 + PCS_LCTL_FORCE_FCTRL;
	vWriteReg825xx(PCS_LCTL, dRegVal);
#endif

	// clear CONNSW.ENRGSRC
	dRegVal = dReadReg825xx (CONNSW);
	dRegVal &= ~CONNSW_ENRGSRC;
	vWriteReg825xx(CONNSW, dRegVal);

	// update device structure to allow test to run
	psDev->bType = IF_SERDES;
}

static void vSwitchI350ToCopper (DEVICE *psDev)
{
	UINT32 dRegVal;

#ifdef DEBUG
	vConsoleWrite ("\nRestoring COPPER interface to SERDES after SERDES->SERDES loopback test\n");
#endif
	// disable RCTL.RXEN
	dRegVal = dReadReg825xx (RCTL);
	dRegVal &= ~RCTL_EN;
	vWriteReg825xx(RCTL, dRegVal);

	// disable TCTL.TXEN
	dRegVal = dReadReg825xx (TCTL);
	dRegVal &= ~TCTL_EN;
	vWriteReg825xx(TCTL, dRegVal);

	// set mode CTRL_EXT.LINK_MODE = 00b
	dRegVal = dReadReg825xx (CTRL_EXT);
	dRegVal &= M__CTRLEXT_LINK;
	dRegVal |= CTRLEXT_LINK_COPPER;
	vWriteReg825xx(CTRL_EXT, dRegVal);

	// set CTRL.SLU
	dRegVal = dReadReg825xx (CTRL);
	dRegVal &= ~(CTRL_FRCSPD + CTRL_FRCDPLX);
	dRegVal |= CTRL_SLU;

	// reset PHY
	PhyReset(psDev);

	// restore device structure to original setting
	psDev->bType = IF_COPPER;
}
