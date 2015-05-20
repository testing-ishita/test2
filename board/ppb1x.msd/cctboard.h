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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/cctboard.h,v 1.2 2015-04-01 16:44:29 hchalla Exp $
 *
 * $Log: cctboard.h,v $
 * Revision 1.2  2015-04-01 16:44:29  hchalla
 * Initial release of CUTE firmware 02.02.01.
 *
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
 * Revision 1.1  2013-09-04 07:13:49  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 *
 */
 

//* includes */

/* defines */

/* ------------------------------------------------------------------------ */
/* Custom ISA Registers                                                     */
/* ------------------------------------------------------------------------ */

/* Status & Control Register 0 */

/* ---------------------------------------------------------------------------
 * Custom ISA Registers
 * -------------------------------------------------------------------------*/

/* Status & Control Register 0 */

#define	STATUS_REG_0	0x210
#define	 HW_REV			0xE0	/* hardware revision, 000b = Rev-A	*/
#define	 HW_REV_SHIFT	5		/*	-"-	*/
#define	 CONS_PORT		0x08	/* 0 = COM1 for serial console, 1 = COM2	*/
#define	 CONS_JUMPER	0x04	/* 0 = COM port, 1 = VGA	*/
#define	 USER_JUMPER	0x02	/* 0 = MTH, 1 = Soak (if MODE_JUMPER = CPSA) */
#define	 MODE_JUMPER	0x01	/* 0 = BIOS, 1 = CPSA	*/


/* Status & Control Register 1 */

#define	STATUS_REG_1	0x211
#define  PMC_25			0x00
#define  PMC_50			0x01
#define  PMC_33			0x04
#define  PMC_66			0x05
#define  PMCX_50		0x02
#define  PMCX_66		0x06
#define  PMCX_100		0x03
#define  PMCX_133		0x07
#define	 PMC2_FREQ		0x1C
#define  PMC1_FREQ		0xE0
#define	 USER_LED		0x01	/* 1 = drive LED on */

/* Status & Control Register 2 */

#define	STATUS_REG_2	0x212
#define	 XMC2_PRESENT	0x40
#define	 XMC1_PRESENT	0x80
#define	 FP_NMI			0x20
#define	 IPMI_NMI		0x10
#define	 DEG_FAL_MASK	0x08
#define	 PMC_EREADY		0x04	/* 1 = PMC module ready */
#define	 PMC2_PRESENT	0x01	/* 1 = present */
#define	 PMC1_PRESENT	0x02	/* 1 = present */


/* General Purpose I/O Register*/
#define GP_IO_REG		0x213
#define FAN_SENSOR     0x40
#define RTM_RESET_IN  0x80
#define  USB_PWR_EN	0x04	/*		-"- */
#define  GPI_1			0x20	/* 0 = GPIO is input, 1 = GPIO is output */
#define  GPI_0			0x10	/*		-"- */
#define  GPO_1			0x02	/* value read/driven via GPIO */
#define  GPO_0			0x01	/* 		-"- */
#define NUM_GPIO_PINS	2

/* CPCI Status Register */
#define	CPCI_STATUS		0x214
#define	 CPCI_FAL		0x40	/* 1 = asserted */
#define	 CPCI_DEG		0x20	/* 1 = asserted */
#define	 CPCI_VIO		0x10	/* 1 = VIO present and valid */
#define	 CPCI_FREQ		0x08	/* 0 = 33MHz, 1 = 66MHz */
#define	 FORCE_SAT		0x04	/* 1 = forced satellite mode */
#define	 CPCI_PRESENT	0x02	/* 1 = CPCI bus present */
#define	 CPCI_SYSEN		0x01	/* 1 = System Controller */

/* Interrupt Control Register */

#define	INT_CONTROL_REG	0x215

#define	 UC_MODE		0x80	/* 0 = IPMI, 1 = ISP */
#define	 GPI_INT_FLAG	0x40	/* 0 = clear event, 1 = interrupt occured */
#define	 SMS_ATN_INT_FLAG 0x20	/*		-"- */
#define	 SMIC_INT_FLAG	0x10	/*		-"- */
#define	 GPI_INT_ENA	0x04	/* 0 = disable, 1 = enable interrupt */
#define	 SMS_ATN_INT_ENA 0x02	/*		-"- */
#define	 SMIC_INT_ENA	0x01	/*		-"- */

/* Interrupt Configuration Register */

#define	INT_CONFIG_REG	0x21D
#define	 IPMI_STATUS	0x80	/* 1 = IPMI interrupt asserted, Wr.0 to clear */
#define  GPIO1_TRANS	0x40	/* 1 = GPIO transition trggered interrupt */
#define  GPIO0_TRANS	0x20	/*		-"- */
/*#define	 PIT_STATUS		0x40*/	/* 1 = PIT interrup asserted, Wr.0 to clear */
/*#define	 PIT_INT		0x20*/	/* 1 = PIT interrupt enabled */
#define	 M66EN_STATUS	0x10	/* 1 = M66EN asserted (h->l transition) */
#define	 M66EN_ENABLE	0x08	/* 1 = enable M66EN interrupt */
#define	 ENUM_STATUS	0x04	/* 1 = ENUM asserted */
#define	 ENUM_ENABLE	0x02	/* 1 = enable ENUM interrupt */
#define	 ENUM_ROUTE		0x01	/* 0 = NMI, 1 = combined interrupt */

/* SMIC Data Register */

#define	SMIC_DATA_REG	0CA9h

/* SMIC Control/Status Register */

#define	SMIC_CTL_STAT	0xCAA
#define	 CS7			0x80	/* see IPMI specification for definitions */
#define	 CS6			0x40	/*					-"- */
#define	 CS5			0x20	/*					-"- */
#define	 CS4			0x10	/*					-"- */
#define	 CS3			0x08	/*					-"- */
#define	 CS2			0x04	/*					-"- */
#define	 CS1			0x02	/*					-"- */
#define	 CS0			0x01	/*					-"- */

/* SMIC Flags Register */

#define	SMIC_FLAGS		0xCAB
#define	 RX_DATA_RDY	0x80	/* see IPMI specification for definitions */
#define	 TX_DATA_RDY	0x40	/*					-"- */
#define	 SMI_ACT		0x10	/*					-"- */
#define	 EVT_ATN		0x08	/*					-"- */
#define	 SMS_ATN		0x04	/*					-"- */
#define	 BUSY			0x01	/*					-"- */


/* ----------------------------------------------------------------------------*/
/* Board specific memory locations in FWH*/
/* --------------------------------------------------------------------------*/

#define	FWH_BASE		0xFFC0FFFF	/* Start of FWH */

/* Firmware Hub General Purpose Inputs - pseudo memory */

#define	FWH_GPI			0xFFBC0100
#define	 BIOS2			0x10	/* 0 = Extd Temp, 1 = Normal */
#define	 BIOS1			0x08	/* Reserved BIOS Option 1 */
#define	 BIOS0			0x04	/* Reserved BIOS Option 0 */
#define	 RECOVERY_MODE	0x02	/* 0 = Run Recovery BIOS, 1 = Normal */
#define	 CMOS_RESTORE	0x01	/* 0 = User Settings, 1 = Factory Settings */

#define HW_STATUS_REG0  0x310
#define J4_SATA4_SELECT   0x01
#define J5_SATA5_SELECT   0x02
#define CF_PRESENT            0x04
#define ONE_PMC_SITE        0x08
#define RTM_TYPE                0x10
#define SPI_DEV_SELECT     0x20
//#define AMT_DISABLE           0x40

#define HW_STATUS_REG1                       0x311
#define ACPI_DISABLE                                0x01
#define BOARD_SLEEP_PWR                     0x02
#define CPCI_PME_WAKE_MASK                0x04
#define CPCI_PBRST_WAKE_MASK            0x08
#define FRONT_PB_WAKE_MASK               0x10
#define RTM_RST_WAKE_MASK                 0x20
#define IPMI_WAKE_MASK                          0x40
#define PCH_GPIO_I2C_MUX_SEL              0x80

#define HW_STATUS_REG2    0x312
#define XMC2_RESET                                 0x01
#define XMC1_RESET                                 0x02
#define PMC2_RESET                                 0x04
#define PMC1_RESET                                 0x08
#define NVSTRAP_I2C_BUS_ENABLE        0x10
#define XMC_I2C_BUS_ENABLE                0x20
#define SPI_CTRL_BIT0                              0x40
#define SPI_CTRL_BIT1                              0x80

#define HW_STATUS_REG3    0x31D
#define AMT_DISABLE                                 0x01
#define FACTORY_DEFAULTS                      0x02
#define SERIAL_OVER_LAN                         0x04
#define RESERVED                                      0x08
#define PMC2_M66EN                                 0x10
#define PMC1_M66EN                                 0x20


#define HW_STATUS_REG4    0x31E
#define XMC2_NVMRO                                0x01
#define XMC1_NVMRO                                0x02

#define FPGA_REV_REG          0x31F


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


#endif


