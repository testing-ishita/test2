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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/cctboard.h,v 1.2 2014-03-18 14:19:51 cdobson Exp $
 *
 * $Log: cctboard.h,v $
 * Revision 1.2  2014-03-18 14:19:51  cdobson
 * Reinstate some status register bit definitions.
 *
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
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
#define  SYS_CON			0x080	/* 0 = SysCON, 1 = Peripheral */
#define  FLASH_BANK			0x040	/* 0 = Dev 1, 1 = Dev 2 */
#define  COM2_ROUTING		0x020	/* 0 = VPX P5, 1 = front panel (SW4-2) */
#define  NVMR0_STATUS		0x010	/* 1 = devices write protected */
#define  USER_LED			0x008	/* 0 = USER LED off, 1 = USER LED on */
#define  BOARD_REV			0x007	/* board revision: 0 = A, 1 = B, etc. */

/* Status & Control Register 1 */
#define STATUS_REG_1		0x0211
#define  USER_LED_MODE		0x080	/* 0 = USER LED, 1 = Processor Hot */
#define  RFU1				0x040
#define  CONS_PORT			0x020	/* 0 = COM1, 1 = COM2 (SW2-1) */
#define  PMC1_HIGH_SP		0x010	/* 0 = x25MHz, 1 = x33MHz */
#define  PMC_SPEED1			0x008	/* 00 = PCI 25/33Mhz,  01 = PCI 50/66Mhz */
#define  PMC_SPEED0			0x004	/* 10 = PCIX 50/60Mhz, 11 = PCIX 100Mhz */
#define  VPX_RESET_ENA		0x002	/* 0 = SYSRESET enabled, 1 = SYSRESET disabled (SW1-4) */
#define  PMC_M66EN			0x001	/* 0 = 25/33Mhz, 1 = 50/66Mhz */


/* Status & Control Register 2 */
#define STATUS_REG_2		0x0212
#define  XMC2_PRESENT		0x080	/* 1 = XMC2 fitted */
#define  XMC1_PRESENT		0x040	/* 1 = XMC1 fitted */
#define  RFU2				0x020
#define  MICRO_NMI			0x010	/* 1 = NMI has occured */
#define  RFU3				0x008
#define  INIT_DONE			0x004	/* 0 = INIT not complete, 1 = INIT complete */
#define  PMC2_PRESENT		0x002	/* 1 = PMC2 fitted */
#define  PMC1_PRESENT		0x001	/* 1 = PMC1 fitted */


/* Status & Control Register 3 */
#define STATUS_REG_3		0x0213
#if 0
#define  RFU4				0x080
#define  RFU5				0x040
#define  RFU6				0x020
#define  RFU7				0x010
#define  RFU8				0x008
#define  VPX_GDISC_INT		0x004	/* 1 = GDISC interrupt present */
#define  VPX_GDISC_INT_EN	0x002	/* 1 = GDISC interrupt enabled */
#define  VPX_GDISC_STAT		0x001	/* GDISC signal status */
#endif

/* Status & Control Register 4 */
#define STATUS_REG_4		0x0214
#if 0
#define  RSVD1				0x080
#define  RFU9				0x040
#define  IPMI_NMI			0x020	/* 1 = IPMI is cause of NMI */
#define  RFU10				0x010
#define  RFU11				0x008
#define  VPX_RESET			0x004	/* 1 = assert reset */
#define  RFU12				0x002
#define  VPX_REF_CLK		0x001	/* 0 = 25MHz, 1 = 100MHz (SW5-1))*/
#endif

/* Interrupt Control Register */
#define INT_CTL				0x0215
#if 0
#define  ISP_MODE			0x080	/* 0 = COM2 in normal mode, 1 = COM2 in ISP mode */
#define  MICRO_GP_INT_STAT	0x040	/* 1 = interrupt request (INT5) */
#define  SMS_ATN_INT		0x020	/* 1 = SMS interrupt has occured */
#define  SMIC_NOT_BUSY		0x010	/* 1 = SMIC not busy event has occured */
#define  RFU13				0x008
#define  MICRO_GP_INT_EN	0x004	/* 1 = interrupt enabled */
#define  SMS_ATN_INT_EN		0x002	/* 1 = SMS interrupt enabled */
#define  SMIC_NOT_BUSY_EN	0x001	/* 1 = SMIC not busy interrupt enabled */
#endif

/* CPU Temperature sensor value */
#define CPU_TEMP_SENSOR		0x0216

/* Ambient Temperature sensor value */
#define AMB_TEMP_SENSOR		0x0217

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

/* Interrupt Configuration 0 register */

#define LDT_STAT_CTL		0x021C
#if 0
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
#endif

/* CCT Security Register 0  */
#if 0
#define CCT_SECURITY0		0x0240
#endif

/* CCT Security Register 1  */
#if 0
#define CCT_SECURITY1		0x0241
#endif


/* GPI Status Register  */
//#if 0
#define GPI_STATUS			0x0310	/*  Bits 0-7: current signal state */
//#endif

/* GPI Interrupt Control Register  */
//#if 0
#define GPI_INT_CTL			0x0311	/* Bits 0-7: 0 = Interrupt on rising edge, 1 = falling edge */
//#endif

/* GPI Interrupt Status Register  */
//#if 0
#define GPI_INT_STATUS		0x0312	/* Bits 0-7: 1 = transition occurred and interrupt generated */
//#endif

/* GPIO Control and Status Register  */
//#if 0
#define GPIO_CTL_STATUS		0x0313	/*  Bits 0-7: read inputs & set outputs */
//#endif

/* GPIO Direction Control Register  */
//#if 0
#define GPIO_DIR_CTL		0x0314	/* Bits 0-7: 0 = Input, 1 = Output */
//#endif

/* GPIO Interrupt Control Register  */
//#if 0
#define GPIO_INT_CTL		0x0315	/* Bits 0-7: 0 = Interrupt on rising edge, 1 = falling edge */
//#endif

/* GPIO Interrupt Status Register  */
//#if 0
#define GPIO_INT_STATUS		0x0316	/* Bits 0-7: 1 = transition occurred and interrupt generated */
//#endif

/* GPI Interrupt Mask Register  */
//#if 0
#define GPI_INT_MASK		0x0317	/* Bits 0-7: 0 = Interrupt  masked, 1 = Interrupt enabled */
//#endif

/* GPIO Interrupt Mask Register  */
//#if 0
#define GPIO_INT_MASK		0x0318	/* Bits 0-7: 0 = Interrupt  masked, 1 = Interrupt enabled */
//#endif

/* Serial I/O Configuration Register */
#if 0
#define SERIAL_CFG		0x31D
#define  RFU14				0x080
#define  COM1_EN			0x040	/* 1 COM1 enabled */
#define  RFU15				0x020
#define  RFU16				0x010
#define  COM1_MODE			0x008	/* 0 = RS232, 1 = RS485/RS422 */
#define  COM1_DUPLEX		0x004	/* 1 = half duplex */
#define  COM1_CTL			0x002	/* 0 = DTR, 1 = RTS */
#define  COM1_SLEW			0x001	/* 0 = normal, 1 = Fast */
#endif
/* FPGA Version Register  */
//#if 0
#define FPGA_REV			0x031F	/* FPGA version */
//#endif


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


