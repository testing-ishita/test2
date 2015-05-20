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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/trb1x.msd/cctboard.h,v 1.1 2013-11-25 14:21:34 mgostling Exp $
 *
 * $Log: cctboard.h,v $
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011-08-26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
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
//#define  COM_PORT			0x08	/* 1 = Watchdog enabled */
#if 0
#define  BOARD_REV			0x0E0	/* board revision: 0 = A, 1 = B, etc. */
#define  NVMR0_STATUS		0x010	/* 1 = devices write protected */
#define  CONS_JUMPER		0x008	/* 0 = VGA, 1 = COM1 */
#define  COM_PORT			0x004	/* 0 = use COM1, 1 = use COM2 for Serial Console */
#define  FW_MODE			0x002	/* 0 = CUTE, 1 = BIOS */
#define  CUTE_MODE			0x001	/* 0 = MTH, 1 = STH (in BIOS mode 1 = USER switch) */
#endif

/* Status & Control Register 1 */
#define STATUS_REG_1		0x0211
#if 0
#define  SATA_NAND_FITTED	0x080	/* 0 = SATA_NAND fitted, 1 = not fitted */
#define  XMC_FITTED			0x040	/* 0 = fitted, 1 = not fitted */
#define  TPM_FITTED			0x020	/* 0 = fitted, 1 = not fitted */
#define  XMC_BIST			0x010	/* 0 = XMC BIST running, 1 = complete */
#define  HIDDEN_ACCESS		0x008	/* 0 = no access, 1 = hidden register access */
#define  USER_LED			0x001	/* 0 = USER LED off, 1 = USER LED on */
#endif

/* Status & Control Register 2 */
#define STATUS_REG_2		0x0212
#if 0
#define  VPX_RESET_ENA		0x080	/* 0 = SYSRESET disabled, 1 = SYSRESET enabled */
#define  USER_JUMPER		0x040	/* 0 = switch off, 1 = switch on */
#define  IPMI_NMI			0x020	/* 1 = IPMI is cause of NMI */
#define  VPX_REF_CLOCK		0x010	/* 0 = clock is input from VPX, 1 = clock is output to VPX */
#define  SATA_NAND_RESET	0x008	/* 0 = reset asserted */
#define  PEX8717_RESET		0x004	/* 0 = controlled by PCH, 1 = PEX8717 is in reset */
#define  VPX_CLOCK			0x002	/* 0 = spread spectrum, 1 = fixed clock */
#define  MLVDS_SPEED		0x001	/* 0 = 25MHZ, 1 = 100MHz */
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
#define  LDT_INT_ENA		0x080	/* 1 = LDT interrupt enable SERIRQ INT5 */
#define  CLK_SEL			0x06F	/* mask for clock select bit */
#define  CLK_SEL_31KHZ		0x020	/*  clock select = 31.25KHz */
#define  CLK_SEL_1MHZ		0x000	/*  clock select = 1MHz */
#define  INT_FLAG			0x010	/* 1 = LDT/PIT interrupt has occurred */
#define  LDT_MODE			0x061	/* mask for LDT/PIT mode */
#define  PIT_10K			0x00E	/*	mode = PIT 10kHz */
#define  PIT_5K				0x00C	/*	mode = PIT 5kHz */
#define  PIT_2K				0x00A	/*	mode = PIT 2kHz */
#define  PIT_1K				0x008	/*	mode = PIT 1kHz */
#define  PIT_500			0x006	/*	mode = PIT 500Hz */
#define  PIT_200			0x004	/*	mode = PIT 200Hz */
#define  PIT_100			0x002	/*	mode = PIT 100Hz */
#define  LDT				0x000	/*	mode = LDT */
#define  RUN				0x001	/* 0 = stop, 1 = run */

/* CCT Security Register DRAM  */
#if 0
#define CCT_SECURITY_DRAM	0x0240
#define  PLX_PROTEXT		0x080	/* 0 = write protect disabled */
#define  FRU_PROTECT		0x040	/* 0 = write protect disabled */
#define  I350_PROTECT		0x020	/* 0 = write protect disabled */
#define  MRAM_PROTECT		0x010	/* 0 = write protect disabled */
#define  BIOS_PROTECT		0x008	/* 0 = write protect disabled */
#define  CCT_SEC_BIT2		0x004	/* CCT Security Bit 2 */
#define  CCT_SEC_BIT1		0x002	/* CCT Security Bit 1 */
#define  DDR_RESET			0x001	/* 0 = DDR_RST# driven high, 1 = driven low */
#endif

/* CCT Security Register 0  */
#if 0
#define CCT_SECURITY0		0x0240
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

/* CCT Security Register 1  */
#if 0
#define CCT_SECURITY1		0x0241
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

/* Hardware Status Register  */
#if 0
#define STATUS_REG_3		0x0310
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

/* VME Address Capture Data/Control Register */

//#define VME_ADDR_CAPT		0x0213
//#define  CAPT_STATUS		0x080	/* 1 = capture in progress (R/O) */
//#define  SEQ_DATA			0x00F	/* sequential data nyble (R/O) */

//#define  CAPT_ABORT			0x004	/* 1 = abort current capture (W/O) */
//#define  READ_RST			0x002	/* 1 = reset read sequence (W/O) */
//#define  CAPT_ENA			0x001	/* 1 = capture enable (W/O) */


/* VME Slot ID Register */

//#define VME_SLOT_ID			0x031E
//#define  GAP				0x020	/* VME64x slot number parity bit */
//#define  VID				0x01F	/* VME64x slot number */


/* GPIO Status & Control Register */

//#define GPIO_CONTROL		0x031C
//#define  GPIO2_DIR			0x040	/* 0 = pin is input, 1 = output */
//#define  GPIO1_DIR			0x020	/* 0 = pin is input, 1 = output */
//#define  GPIO0_DIR			0x010	/* 0 = pin is input, 1 = output */
//#define  GPIO2_DATA			0x004	/* wr = set pin state, rd = current state */
//#define  GPIO1_DATA			0x002	/* wr = set pin state, rd = current state */
//#define  GPIO0_DATA			0x001	/* wr = set pin state, rd = current state */


/* Serial I/O Configuration Register */

#define SER_VPX_CFG			0x031E
#define  GPIO1_MODE			0x080	/* 0 = GPIO1, 1 = mask reset enabled */
#define  VPX_MASK			0x030	/*RO VPX Fabric Select */
#define  VPX_SEL2			0x020	/*    00 = 1x8, 01 = 2x4 */
#define  VPX_SEL1			0x010	/*    10 = 4x2, 11 = 4x2 */
#define  GDISC_STATUS		0x008	/*RO VPX GPIO */
#define  SYSCON_STATUS		0x004	/*RO  1 = System controller */
#define  COM2_MASK			0x003	/* COM2 function mask */
#define  COM2_DUPLEX		0x002	/* 0 = RS485 Full duplex, 1 = half duplex */
#define  COM2_MODE			0x001	/* 0 = RS232, 1 = RS485 */

/* Fake ACPI Power Control Register */

//#define CCT_PM1a_CNT_BLK_l 	0x021E

//#define CCT_PM1a_CNT_BLK_h 	0x021F
//#define  PM1_SLP_EN			0x20	/* 1 = sleep enable */
//#define  PM1_SLP_TYPx		0x1C


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



/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


#endif


