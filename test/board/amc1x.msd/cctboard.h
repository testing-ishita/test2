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
#define  SRIO_RESET_1		0x01			/* 1 = assert reset */
#define  SRIO_RESET_2		0x01			/* 1 = assert reset */


/* Status & Control Register 1 */

#define STATUS_REG_1		0x0212
#define  USER_TEST_MTH_STH	0x02			/* 0 = MTH, 1 = STH */
#define  CONS_JUMPER		0x04			/* 0 = Serial, 1 = VGA */
#define  CUTE_MODE			0x08			/* 0 = BIOS, 1 = CUTE */
#define  IPMI_NMI			0x40			/* 0 = no NMI, 1 = NMI */
#define  REGISTER_LOCK		0x80			/* 0 = locked, 1 = unlocked */

/* Status & Control Register 2 */

#define STATUS_REG_2		0x0214
#define  BOARD_REV			0xF0			/* board revision: 0 = A, 1 = B, etc. */

/* GPIO Register */

#define GPIO_REG_1			0x0213
#define  GPIO_INPUT_1		0x01
#define  GPIO_INPUT_2		0x02
#define  GPIO_INPUT_3		0x04
#define  GPIO_OUTPUT		0x08

/* Interrupt Control Register */

#define INTERRUPT_CTRL_REG	0x0215
#define  LDT_INT_ENABLE		0x08


/* Long Duration Timer Count Registers */

#define LDT_0				0x0218	/* LSB */
#define LDT_1				0x0219
#define LDT_2				0x021A
#define LDT_3				0x021B

/* Long Duration Timer Status and Control Register */

#define LDT_STAT_CTL		0x021C
#define  CLK_SEL			0x0DF			/* mask for clock select bit */
#define  CLK_SEL_HFCLK4		0x020			/*	clock select = 3 */
#define  CLK_SEL_HFCLK128	0x000			/*	clock select = 1MHz */
#define  INT_FLAG			0x010			/* 1 = LDT/PIT interrupt has occured */
#define  LDT_MODE			0x0F1			/* mask for LDT/PIT mode */
#define  PIT_10K			0x00E			/*	mode = PIT 10kHz */
#define  PIT_5K				0x00C			/*	mode = PIT 5kHz */
#define  PIT_2K				0x00A			/*	mode = PIT 2kHz */
#define  PIT_1K				0x008			/*	mode = PIT 1kHz */
#define  PIT_500			0x006			/*	mode = PIT 500Hz */
#define  PIT_200			0x004			/*	mode = PIT 200Hz */
#define  PIT_100			0x002			/*	mode = PIT 100Hz */
#define  LDT				0x000			/*	mode = LDT */
#define  RUN				0x001			/* 0 = stop, 1 = run */


/* FPGA Version Register */

#define CCT_FPGA_REV	0x021D

/* Hidden hardware Status & Control Registers */

#define CCT_HIDDEN_HW_STATUS	0x021E
#define  RECOVERY_MODE			0x01		/* 0 = Normal, 1 = Run Recovery BIOS */
#define  BIOS_SAFE_DFLTS		0x02		/* 0 = Normal, 1 = Use BIOS safe defaults */
#define  SATA_NAND				0x04		/* 0 = Not fitted, 1 = fitted */
#define  SPI_BOOT_DEV			0x10		/* 0 = 1st device, 1 = 2nd device */
#define  FRONT_PANEL_GPI		0x40
#define  SRIO_HOST_SLAVE		0x80		/* 0 = host, 1 = slave (default with switch off) */

#define CCT_HIDDEN_HW_CONTROL	0x021F
#define  RESET_x540				0x01		/* 1 = Assert reset */
#define  RESET_i210				0x02		/* 1 = Assert reset to both i210s */
#define  SOL_ENABLE				0x04		/* 0 = Serial-over-LAN disabled, 1 = enabled */
#define  SOL_COM_PORT			0x10		/* 0 = Serial-over-LAN on COM1, 1 = COM2 */
#define  FORCE_ISP_MODE			0x20		/* 1 = connect COM2 to micro-controller UART1 */ 
#define  SPI_DEV_SELECT			0xC0		/* 00 = use external switch for CS0, CS1 unused */
											/* 01 = CS0 to device 1, CS1 to device 2 */
											/* 10 = CS0 to device 2, CS1 to device 1 */
											/* 11 = CSO to device selected by external switch, CS1 to the other */

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


