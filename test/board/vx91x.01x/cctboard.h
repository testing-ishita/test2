#ifndef __cctboard_h__
	#define __cctboard_h__

/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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
 *																		*
 ************************************************************************/

/* cctboard.h - board-specific register definitions, etc.
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx91x.01x/cctboard.h,v 1.1 2014-05-15 09:42:03 jthiru Exp $
 *
 * $Log: cctboard.h,v $
 * Revision 1.1  2014-05-15 09:42:03  jthiru
 * Adding vx91x board sources
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011/03/22 13:18:04  hchalla
 * Updated cctboard file.
 *
 * Revision 1.1  2011/02/28 11:57:43  hmuneer
 * no message
 *
 *
 */
 

/* includes */

/* defines */

/* ------------------------------------------------------------------------ */
/* Custom ISA Registers                                                     */
/* ------------------------------------------------------------------------ */

/* Status & Control Register 0 */

#define STATUS_REG_0		0x0210
#define  COM_PORT			0x08	/* 1 = Watchdog enabled */
#if 0
#define  CONS_JUMPER		0x080	/* 0 = VGA, 1 = COM1 */
#define  CMOS_RESTORE		0x040	/* 0 = factory CMOS defaults, 1 = user CMOS defaults */
#define  FAST_BSW			0x020	/* 1 = enable Fast byte swapping */
#define  SLAVE_BSW			0x010	/* 1 = enable byte swapping for Slave */
#define  MASTER_BSW			0x008	/* 1 = enable byte swapping for Master */
#define  BOARD_REV			0x007	/* board revision: 0 = A, 1 = B, etc. */
#endif

/* Status & Control Register 1 */
#if 0
#define STATUS_REG_1		0x0212
#define  FP_NMI				0x080	/* 1 = front panel switch caused NMI */
#define  LINT1_NMI			0x040	/* 1 = LINT1 caused NMI */
#define  BERR_FLAG			0x020	/* 1 = VME bus error has occured */
#define  BERR_ENA			0x010	/* 1 = VME bus error interrupt enabled */
#define  EXP_PMC_2			0x008	/* 1 = PCI compliant module in carrier PMC #2 */
#define  EXP_PMC_1			0x004	/* 1 = PCI compliant module in carrier PMC #1 */
#define  PMC_2				0x002	/* 1 = PCI compliant module in baseboard PMC #2 */
#define  PMC_1				0x001	/* 1 = PCI compliant module in baseboard PMC #1 */
#endif

/* Status & Control Register 2 */
#if 0
#define STATUS_REG_2		0x0211
#define  MODE_JUMPER		0x080	/* 0 = VSA Mode, 1 = BIOS Mode */
#define  USER_JUMPER		0x040	/* 0 = switch off, 1 = switch on */
#define  SYSRST_ENA			0x020	/* 1 = system reset enabled */
#define  CCT_UNLOCK			0x010	/* 0 = use COM3, 1 = use COM1 for Serial Console */
#define  COM_PORT			0x008	/* 1 = Watchdog enabled */
#define  WDOG_RESET_ENA		0x004	/* 1 = Watchdog can reset board */
#define  PMC_EREADY			0x002	/* 1 = Proc. PMC ready for enumeration */
#define  PCIX_SPEED			0x001	/* 0 = PCI-X Bus 33MHZ, 1 = 66MHz */
#endif

/* Status & Control Register 3 */
#if 0
#define STATUS_REG_3		0x021D
#define  FLASH_STATUS		0x080	/* 0 = Device Ready, 1 = Device Busy */
#define  FLASH_A25			0x040	/* Flash Memory A25 */
#define  FLASH_A24			0x020	/* Flash Memory A24 */
#define  FLASH_A23			0x010	/* Flash Memory A23 */
#define  SEL_MASK			0x070	/* device/page select bit mask */
#define  XMC2_MBIST			0x008	/* 1 = XMC2 passed BIST */
#define  XMC2_PRESENT		0x004	/* 1 = XMC2 fitted */
#define  XMC1_MBIST			0x002	/* 1 = XMC1 passed BIST */
#define  XMC1_PRESENT		0x001	/* 1 = XMC1 fitted */
#endif

/* Long Duration Timer Count Registers */
#if 0
#define LDT_0				0x0218	/* LSB */
#define LDT_1				0x0219
#define LDT_2				0x021A
#define LDT_3				0x021B
#endif
/* Long Duration Timer Status and Control Register */

#define LDT_STAT_CTL		0x021C
#define  CLK_SEL			0x0DF	/* mask for clock select bit */
#define  CLK_SEL_HFCLK4		0x020	/*	clock select = HFCLK / 4 */
#define  CLK_SEL_HFCLK128	0x000	/*	clock select = HFCLK / 128 */
#define  INT_FLAG			0x010	/* 1 = LDT/PIT interrupt has occured */
#define  LDT_MODE			0x0F1	/* mask for LDT/PIT mode */
#define  PIT_10K			0x00E	/*	mode = PIT 10kHz */
#define  PIT_5K				0x00D	/*	mode = PIT 5kHz */
#define  PIT_2K				0x00C	/*	mode = PIT 2kHz */
#define  PIT_1K				0x00B	/*	mode = PIT 1kHz */
#define  PIT_500			0x00A	/*	mode = PIT 500Hz */
#define  PIT_200			0x009	/*	mode = PIT 200Hz */
#define  PIT_100			0x008	/*	mode = PIT 100Hz */
#define  LDT				0x000	/*	mode = LDT */
#define  RUN				0x001	/* 0 = stop, 1 = run */


/* VME Address Capture Data/Control Register */

#define VME_ADDR_CAPT		0x0213
#define  CAPT_STATUS		0x080	/* 1 = capture in progress (R/O) */
#define  SEQ_DATA			0x00F	/* sequential data nyble (R/O) */

#define  CAPT_ABORT			0x004	/* 1 = abort current capture (W/O) */
#define  READ_RST			0x002	/* 1 = reset read sequence (W/O) */
#define  CAPT_ENA			0x001	/* 1 = capture enable (W/O) */


/* VME Slot ID Register */

#define VME_SLOT_ID			0x031E
#define  GAP				0x020	/* VME64x slot number parity bit */
#define  VID				0x01F	/* VME64x slot number */


/* GPIO Status & Control Register */

#define GPIO_CONTROL		0x031C
#define  GPIO2_DIR			0x040	/* 0 = pin is input, 1 = output */
#define  GPIO1_DIR			0x020	/* 0 = pin is input, 1 = output */
#define  GPIO0_DIR			0x010	/* 0 = pin is input, 1 = output */
#define  GPIO2_DATA			0x004	/* wr = set pin state, rd = current state */
#define  GPIO1_DATA			0x002	/* wr = set pin state, rd = current state */
#define  GPIO0_DATA			0x001	/* wr = set pin state, rd = current state */


/* Serial I/O Configuration Register */

#define SERIAL_CONFIG		0x031D
#define  CH1_MODE			0x080	/* 0 = RS232, 1 = RS485 */
#define  CH1_DUPLEX			0x040	/* 0 = full duplex, 1 = half duplex */
#define  CH1_DIR_CTL		0x020	/* 0 = use RTS, 1 = use DTR */
#define  CH1_SLEW			0x010	/* 0 = fast slew, 1 = slow slew */
#define  CH0_MODE			0x008	/* 0 = RS232, 1 = RS485 */
#define  CH0_DUPLEX			0x004	/* 0 = full duplex, 1 = half duplex */
#define  CH0_DIR_CTL		0x002	/* 0 = use RTS, 1 = use DTR */
#define  CH0_SLEW			0x001	/* 0 = fast slew, 1 = slow slew */


/* Fake ACPI Power Control Register */

#define CCT_PM1a_CNT_BLK_l 	0x021E

#define CCT_PM1a_CNT_BLK_h 	0x021F
#define  PM1_SLP_EN			0x20	/* 1 = sleep enable */
#define  PM1_SLP_TYPx		0x1C


/* ------------------------------------------------------------------------ */
/* Board specific memory locations in FW                                    */
/* ------------------------------------------------------------------------ */

/* Mirror Flash is paged into an 8MB window */

#define PAGE_SIZE			0x000800000	/* eight, 8MB pages = 64MB device */
#define FLASH_BASE			0x0FF000000	/* base address of device page */


#define FWH_BASE			0x0FFF00000	/* Start of FW */

#define BIT_IMAGE_ADDR		0xFWH_BASE	/* placed at bottom of FW */

/* Firmware Hub General Purpose Inputs - pseudo memory */

#define FWH_GPI				0x0FFBC0100
#define  BIOS2				0x010		/* 0 = Extd Temp, 1 = Normal */
#define  BIOS1				0x008		/* Reserved BIOS Option 1 */
#define  BIOS0				0x004		/* Reserved BIOS Option 0 */
#define  BIOS_				0x002		/* Reserved */
#define  RECOVERY_MODE		0x001		/* 0 = Run Recovery BIOS, 1 = Normal */


/* ------------------------------------------------------------------------ */
/* Board type definitions                                                   */
/* ------------------------------------------------------------------------ */

#define VP_415_BRD			(VP415_03X | (VMEBRD << 5))
#define VP_417_BRD			(VP417_03X | (VMEBRD << 5))


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


#endif


