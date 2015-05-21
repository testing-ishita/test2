#ifndef __cctboard_h__
	#define __cctboard_h__

/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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

/* includes */

/* defines */

/* ------------------------------------------------------------------------ */
/* Custom ISA Registers                                                     */
/* ------------------------------------------------------------------------ */

/* Status & Control Register 0 */

#define STATUS_REG_0		0x0210
#define  COM_PORT			0x08	/* 1 = Watchdog enabled */


/* Status & Control Register 1 */

#define STATUS_REG_1		0x0212
#define  CUTE_MODE			(0x1 << 3)	/* 0 = BIOS, 1 = CUTE */
#define  CONS_JUMPER		(0x1 << 2)	/* 0 = Serial, 1 = VGA */
#define  USER_TEST_MTH_STH	(0x1 << 1)    /* 0 = MTH, 1 = STH */
#define  IPMI_NMI			(0x1 << 6)


/* Status & Control Register 2 */

#define STATUS_REG_2		0x0214
#define  BOARD_REV			0x0030	/* board revision: 0 = A, 1 = B, etc. */

#define INTERRUPT_CTRL_REG	0x0215
#define  LDT_INT_ENABLE		0x08


/* Status & Control Register 3 */

/* Long Duration Timer Count Registers */

#define LDT_0				0x0218	/* LSB */
#define LDT_1				0x0219
#define LDT_2				0x021A
#define LDT_3				0x021B

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


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


#endif


