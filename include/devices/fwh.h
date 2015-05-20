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
 *                                                                      *
 ************************************************************************/

/* fwh.h - definitions for fwh devices
 */

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/fwh.h,v 1.1 2013-09-04 07:40:27 chippisley Exp $
*
* $Log: fwh.h,v $
* Revision 1.1  2013-09-04 07:40:27  chippisley
* Import files into new source repository.
*
* Revision 1.1  2009/05/21 08:17:24  cvsuser
* Initial checkin for FWH test
*
*
*/

#define E__CHIPSET_NF		E__BIT
#define E__ROM_DETECT		E__BIT + 0x1


#define TOLM			0xC4
#define	SIZE_64KB		0x00010000L

#define I430HX_DEV		0

#define	VID_430HX		0x8086
#define DID_430HX		0x1250

#define DRB7_430			0x67
#define	 SIZE_4MB		0x00400000L

#define DRAMC			0x57
#define	 HEN_512KB_430		0x40
#define	 HEN_15MB_430		0x80
#define	 HEN_MASK_430		0x3F

#define I440BX_DEV		0

#define	VID_440BX		0x8086
#define DID_440BX_AGP	0x7190
#define DID_440BX		0x7192

#define DRB7_440bx			0x67
#define	 SIZE_8MB		0x00800000L

#define FDHC_430bx			0x68
#define	 HEN_512KB_430bx	0x40
#define	 HEN_15MB_430bx		0x80
#define	 HEN_MASK_430bx		0x3F

#define I440GX_DEV		0

#define	VID_440GX		0x8086
#define DID_440GX_AGP	0x71A0
#define DID_440GX		0x71A2

#define DRB7_440			0x67
#define	 SIZE_8MB		0x00800000L

#define FDHC_440gx			0x68
#define	 HEN_512KB_440gx	0x40
#define	 HEN_15MB_440gx		0x80
#define	 HEN_MASK_440gx		0xC0

#define I855GME_DEV		0
#define I855GME_MEM_FN	1			//	Memory Controller function

#define	VID_855GME		0x8086
#define DID_855GME		0x3580

#define DRB3			0x43
#define	SIZE_32MB		0x02000000L

#define E7520_DEV		0

#define	VID_E7520		0x8086
#define DID_E7520		0x3590

#define TOLM_7520		0xC4
#define	SIZE_64KB		0x00010000L

#define I945GM_DEV		0

#define	VID_945GM		0x8086
#define DID_945GM		0x27A0
#define DID_945GM_2		0x27AC

#define TOLUD_945			0x9C		//	Top Of Low Usable DRAM
#define	SIZE_16MB		0x01000000L

#define LE3100_DEV0		0		// MCH
#define LE3100_DEV31		31		// LPC bridge


#define	VID_LE3100		0x8086
#define DID_LE3100		0x35B0

#define TOLM_3100			0xC4


#define PIIX4E_DEV		7

#define	VID_PIIX4E		0x8086
#define DID_PIIX4E		0x7110

#define	XBCS			0x4E
#define	 XTD_BIOS_EN	0x80
#define	 LOW_BIOS_EN	0x40
#define	 BIOS_WE		0x04

#define i6300ESB_DEV		31			// 6300ESB is device 28-31, with various functions

#define	VID_6300ESB		0x8086
#define DID_6300ESB		0x25A1

#define ICH7_DEV		31			// ICH7 is device 28-31, with various functions

#define	VID_ICH7		0x8086
#define DID_ICH7		0x27BD
#define DID_ICH7R		0x27B8

#define ICH9M_DEV	31

#define ICH9_VID	0x8086

#define	ICH9M_1		0x2911
#define ICH9M_2		0x2917
#define ICH9M_3		0x2919

#define GM45_DEV	0

#define GM45_VID	0x8086
#define GM45_DID1	0x2A40
#define GM45_DID2	0x2A42

#define TOLUD_gm45		0xB0

#define	SIZE_1MB		0x0100000L


#define  INTEL_VID 0x8086
#define  SCH_DEV   31
#define  SCH_DEVID 0x8119

#define PCI_VID					0x00	/* Vendor ID offset				*/
#define PCI_VENDOR_ID			0x00	/*		-"-						*/
#define PCI_DID					0x02	/* Device ID offset				*/
#define PCI_DEVICE_ID			0x02	/*		-"-						*/

#define MAX_BOARDS		100					/* board types allowed */
#define MAX_MEMS		(MAX_BOARDS * 3)	/* average of three per board */
#define	MAX_PARAMS		4					/* params for page algorithm */
#define MAX_DEV_PER_MEM	16					/* devices in any memory array */
#define MAX_MEM_WIDTH	8					/* max number devices in width */

#define DESELECT_DEVICE		(UINT8)(0xFF)


#define MPC_DATA_PORT		0x0CBC
#define MPC_ADDRL_PORT		0x0CB0
#define MPC_ADDRH_PORT		0x0CB4

#define LOCAL_SLOT			31

#define CLASS_040		1		/* 29F0x0 and 28SF040 */
#define CLASS_STRATA_B	10		/* Intel StrataFlash (byte) */
#define CLASS_STRATA_W	11		/* Intel StrataFlash (word) */
#define CLASS_FWH		12		/* Firmware Hub */
#define CLASS_MFLASH	13		/* Mirror Flash */
#define CLASS_SST_FWH	14		/* SST Firmware Hub */
#define CLASS_SPI		15		/* SPI */
#define CLASS_PSUDO_FWH 16		/* Psudo FWH */
#define CLASS_FWH_SPI   17
#define CLASS_SRAM		20		/* battery-backed SRAM */

#define MANUF_ID_ADDR		0x00000000L
#define DEVICE_ID_ADDR		0x00000001L

#define FWH_CLR_STATUS		0x50
#define FWH_RESET			0xFF

#define I802AC_DID			0xAC			// Intel 1MB part
#define LW080_DID			0xE1			// Atmel
#define FW080_DID			0x2D			// STM

#define ATMEL_MID			0x1F			// Atmel manuf ID
#define INTEL_MID			0x89			// Intel manuf ID
#define STM_MID				0x20			// ST Micro manuf ID

	/* Firmware Hub Devices */

#define FWH8_SEC_SIZE		0x10000L		// 64-kByte sectors
#define FWH8_NUM_SEC		16				//

#define FWH_READ_ID			0x90

