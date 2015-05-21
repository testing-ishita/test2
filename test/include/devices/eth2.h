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

/* eth2.h - definitions for ethernet devices
 */

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/eth2.h,v 1.5 2015-04-01 16:46:33 hchalla Exp $
*
* $Log: eth2.h,v $
* Revision 1.5  2015-04-01 16:46:33  hchalla
* Added support for 82599 SFP 10G tests.
*
* Revision 1.4  2014-11-04 15:43:20  mgostling
* Added more register definitions.
* Removed all unused register definitions inherited from eth.h.
*
* Revision 1.3  2014-10-03 14:52:45  chippisley
* Added further support for x540 devices
*
* Revision 1.2  2014-09-22 10:07:34  mgostling
* Work in progress
*
* Revision 1.1  2014-09-19 12:57:02  mgostling
* Initial check-in to CVS
*
*
*/

/* defines */

/* Error Codes at offset of 6, one for each instance */

#define	E__TEST_NO_DEVICE					E__BIT
#define E__RESET_TIMEOUT					E__BIT + 0x6
#define E__PHY_RESET_TIMEOUT				E__BIT + 0xC
#define E__CTRL_RESET_TIMEOUT				E__BIT + 0x12
#define E__FAIL_ROLLING_1					E__BIT + 0x18
#define E__FAIL_ROLLING_0					E__BIT + 0x1E
#define E__TEST_NO_MEM						E__BIT + 0x24
#define E__EEPROM_ACCESS					E__BIT + 0x2A
#define E__EEPROM_CHECKSUM					E__BIT + 0x30
#define E__PHYREADERR						E__BIT + 0x36
#define E__INT_LB_TIMED_OUT					E__BIT + 0x3C
#define E__INT_LB_BAD_DATA					E__BIT + 0x42
#define E__ICR_NOT_ZERO						E__BIT + 0x48
#define E__NO_MASKINT						E__BIT + 0x4E
#define E__WRONG_MASKINT					E__BIT + 0x54
#define E__PHYRESET_TIMEOUT					E__BIT + 0x5A
#define E__LOC_LB_BAD_MEDIA					E__BIT + 0x60
#define E__LOC_LB_LINK_FAIL					E__BIT + 0x66
#define E__LOC_LB_TIMED_OUT					E__BIT + 0x6C
#define E__LOC_LB_BAD_DATA					E__BIT + 0x72
#define E__EEPROM_TIMEOUT					E__BIT + 0x78
#define E__SPI_EEPROM_TIMEOUT				E__BIT + 0x7E
#define E__EEPROM_WRITE						E__BIT + 0x84
#define E__SPI_EEPROM_WRITE					E__BIT + 0x8A
#define E__EEPROM_BAD_MEDIA					E__BIT + 0x90
#define E__EEPROM_RW						E__BIT + 0x96
#define E__X540_SEMAPHORE					E__BIT  + 0x9C


/* PCI register access size codes */

#define PCI_BYTE							0x01
#define PCI_WORD							0x02
#define PCI_DWORD							0x03

#define MAX_CONTROLLERS						2				/* how many controllers we may use */

	/* Masks for Various Registers */

//#define PCI_IO_BASE_MASK					0xFFFFFFFC
//#define PCI_MEM_BASE_MASK					0xFFFFFFF0
//#define PCI_ROM_BASE_MASK					0xFFFFF800
#define TX_DESC_BASE						0x00400000L
#define TX_DESC_LENGTH						0x00100000L
#define TX_BUFF_BASE						0x00600000L
#define TX_BUFF_LENGTH						0x00100000L
#define TX_BUFF_SIZE						0x00000800L		/* 2-kByte TX buffers */
#define TX_NUM_DESC							(TX_DESC_LENGTH / TX_BUFF_SIZE / MAX_CONTROLLERS)

#define RX_DESC_BASE						0x00800000L
#define RX_DESC_LENGTH						0x00100000L
#define RX_BUFF_BASE						0x00A00000L
#define RX_BUFF_LENGTH						0x00100000L
#define RX_BUFF_SIZE						0x00000800L		/* 2-kByte RX buffers */
#define RX_NUM_DESC							(RX_DESC_LENGTH / RX_BUFF_SIZE / MAX_CONTROLLERS)

/* PCI Configuration Space Register Definitions */
#define VID_INTEL							0x8086

#define DID_i350AMx_C						0x1521			/*pci express gigabit copper */
#define DID_i350AMx_S						0x1523			/*pci express gigabit serdes */
#define DID_i210_C							0x1533			/*pci express gigabit copper */
#define DID_i210_S							0x1537			/*pci express gigabit serdes */
#define DID_x540							0x1528			/*pci express 10 gigabit copper */
#define DID_82599_SFP                 0x10FB

#define DIDVID_i350AMx_C					((DID_i350AMx_C << 16) | VID_INTEL)
#define DIDVID_i350AMx_S					((DID_i350AMx_S << 16) | VID_INTEL)
#define DIDVID_i210_C						((DID_i210_C << 16) | VID_INTEL)
#define DIDVID_i210_S						((DID_i210_S << 16) | VID_INTEL)
#define DIDVID_x540							((DID_x540 << 16) | VID_INTEL)
#define DIDVID_82599_SFP                ((DID_82599_SFP << 16) | VID_INTEL)

#define PCI_VID								0x00			/* Vendor ID offset				*/
#define PCI_VENDOR_ID						0x00			/*		-"-						*/
#define PCI_DID								0x02			/* Device ID offset				*/
#define PCI_DEVICE_ID						0x02			/*		-"-						*/
#define PCI_CACHE_LINE_SIZE					0x0C
#define	PCI_BASE_ADDR_1						0x10
#define	PCI_BASE_ADDR_2						0x14
#define PCI_ROMEN							0x00000001		/* Expansion ROM enable 	*/

#define X540_CAPABILITY_DEVICE_CONTROL		0xA8
#define X540_TLP_PAYLOAD_SIZE				0x20
#define X540_CAPABILITY_LINK_CONTROL		0xB0
#define X540_COMMON_CLK_CONFIG				0x40

/* Memory Mapped Registers
 */

#define I8257x_REG_SIZE						0x00020000
#define I350_REG_SIZE						0x00040000
#define X540_REG_SIZE						0x00200000

#define CTRL								0x00000L		/* General */
#define STATUS								0x00008L
#define EECD								0x00010L
#define EERD								0x00014L
#define CTRL_EXT							0x00018L
#define MDIC								0x00020L
#define TXCW								0x00178L

#define ICR									0x000C0L		/* Interrupt */
#define ICS									0x000C8L
#define IMC									0x000D8L

#define RCTL								0x00100L		/* Receive */
#define FCRTL								0x02160L
#define FCRTH								0x02168L
#define RDBAL								0x02800L
#define RDBAH								0x02804L
#define RDLEN								0x02808L
#define RDH									0x02810L
#define RDT									0x02818L
#define RDTR								0x02820L

#define RXDCTL								0x02828L		/* RX DMA */

#define TCTL								0x00400L		/* Transmit */
#define TIPG								0x00410L

#define TDBAL								0x03800L		/* Transmit */
#define TDBAH								0x03804L
#define TDLEN								0x03808L
#define TDH									0x03810L
#define TDT									0x03818L

#define TXDCTL								0x03828L		/* TX DMA */

#define RXCSUM								0x05000L		/* RX DMA */


#define RAL									0x05400L		/* Receive */
#define RAH									0x05404L

/* Statistics */
#define CRCERRS								0x04000L
#define ALGNERRC							0x04004L
#define SYMERRS								0x04008L
#define RXERRC								0x0400CL
#define MPC									0x04010L
#define SCC									0x04014L
#define ECOL								0x04018L
#define MCC									0x0401CL
#define LATECOL								0x04020L
#define COLC								0x04028L
#define DC									0x04030L
#define TNCRS								0x04034L
#define SEC									0x04038L
#define CEXTERR								0x0403CL
#define RLEC								0x04040L
#define XONRXC								0x04048L
#define XONTXC								0x0404CL
#define XOFFRXC								0x04050L
#define XOFFTXC								0x04054L
#define FCRUC								0x04058L
#define PRC64								0x0405CL
#define PRC127								0x04060L
#define PRC255								0x04064L
#define PRC511								0x04068L
#define PRC1023								0x0406CL
#define PRC1522								0x04070L
#define GPRC								0x04074L
#define BPRC								0x04078L
#define MPRC								0x0407CL
#define GPTC								0x04080L
#define GORCL								0x04088L
#define GORCH								0x0408CL
#define GOTCL								0x04090L
#define GOTCH								0x04094L
#define RNBC								0x040A0L
#define	RUC									0x040A4L
#define RFC									0x040A8L
#define ROC									0x040ACL
#define RJC									0x040B0L
#define TORL								0x040C0L
#define TORH								0x040C4L
#define TOTL								0x040C8L
#define TOTH								0x040CCL
#define TPR									0x040D0L
#define TPT									0x040D4L
#define PTC64								0x040D8L
#define PTC127								0x040DCL
#define PTC255								0x040E0L
#define PTC511								0x040E4L
#define PTC1023								0x040E8L
#define PTC1522								0x040ECL
#define MPTC								0x040F0L
#define BPTC								0x040F4L

/* x540 specific */

#define x540_TESTED_INTERRUPTS				0x7f6fffffL /* Exclude reserved bits and LSC */
#define EICR_LSC							0x00100000L
#define LINKS_UP							0x40000000L /* Link up */

#define EICR								0x00800L
#define EICS								0x00808L
#define EIMC								0x00888L
#define QPRC(_i)							(0x01030 + ((_i) * 0x40))	/* 16 of these */
#define QBRC_L(_i)							(0x01034 + ((_i) * 0x40))	/* 16 of these */
#define QBRC_H(_i)							(0x01038 + ((_i) * 0x40))	/* 16 of these */
#define QPRDC(_i)							(0x01430 + ((_i) * 0x40))	/* 16 of these */
#define RQSMR(_i)							(0x02300 + ((_i) * 4))		/* 32 of these */
#define RXDSTATCTRL							0x02F40L
#define RXDGPC								0x02F50L
#define RXDGBCL								0x02F54L
#define RXDGBCH								0x02F58L
#define RXDDPC								0x02F5CL
#define RXDDBCL								0x02F60L
#define RXDDBCH								0x02F64L
#define RXLPBKPC							0x02F68L
#define RXLPBKBCL							0x02F6CL
#define RXLPBKBCH							0x02F70L
#define RXDLPBKPC							0x02F74L
#define RXDLPBKBCL							0x02F78L
#define RXDLPBKBCH							0x02F7CL
#define RXCTRL								0x03000L
#define PXON2OFFCNT(_i)						(0x03240 + ((_i) * 4))		/* 8 of these */
#define PXONTXC(_i)							(0x03F00 + ((_i) * 4))		/* 8 of these */
#define PXOFFTXC(_i)						(0x03F20 + ((_i) * 4))		/* 8 of these */
#define LXONTXC								0x03F60L
#define LXOFFTXC							0x03F68L
#define RXMPC(_i)							(0x03FA0 + ((_i) * 4))		/* 8 of these */
#define ILLERRC								0x04004L
#define ERRBC								0x04008L
#define MLFC								0x04034L
#define MRFC								0x04038L
#define RLEC								0x04040L
#define PXONRXCNT(_i)						(0x04140 + ((_i) * 4))		/* 8 of these */
#define PXOFFRXCNT(_i)						(0x04160 + ((_i) * 4))		/* 8 of these */
#define LXONRXCNT							0x041A4L
#define LXOFFRXCNT							0x041A8L
#define RXNFGPC								0x041B0L
#define RXNFGBCL							0x041B4L
#define RXNFGBCH							0x041B8L
#define HLREG0								0x04240L
#define HLREG1								0x04244L
#define PAP									0x04248L
#define MSCA								0x0425CL
#define MSRWD								0x04260L
#define MAXFRS								0x04268L
#define TREG								0x0426CL
#define MFLCN                               0x04294L
#define AUTOC                         0x042A0L
#define LINKS								0x042A4L
#define MMNGC								0x042D0L
#define MPVC								0x04318L
#define MACC								0x04330L
#define RTTDQSEL							0x04904L
#define RTTBCNRC							0x04984L
#define DMATXCTL							0x04A80L
#define FCTRL_X540							0x05080L
#define MCSTCTRL							0x05090L
#define QPTC(_i)							(0x06030 + ((_i) * 0x40))	/* 16 of these */
#define TQSM(_i)							(0x08600 + ((_i) * 0x4))	/* 32 of these */
#define QBTC_L(_i)							(0x08700 + ((_i) * 0x8))	/* 16 of these */
#define QBTC_H(_i)							(0x08704 + ((_i) * 0x8))	/* 16 of these */
#define SSVPC								0x08780L
#define TXDGPC								0x087A0L
#define TXDGBCL								0x087A4L
#define TXDGBCH								0x087A8L
#define EECD_X540							0x10010L
#define EERD_X540							0x10014L
#define SWSM								0x10140L
#define SW_FW_SYNC							0x10160L
#define GCR_EXT								0x11050L

#define RXPBSIZE(_i)						(0x03C00 + ((_i) * 4))		/* 8 of these 0x03C00 - 0x03C1C */
#define RXPBSIZE_SHIFT						10

/* Receive DMA registers */

#define RDBAL_X540(_i)						(((_i) < 64) ? (0x01000 + ((_i) * 0x40)) : (0x0D000 + (((_i) - 64) * 0x40)))
#define RDBAH_X540(_i)						(((_i) < 64) ? (0x01004 + ((_i) * 0x40)) : (0x0D004 + (((_i) - 64) * 0x40)))
#define RDLEN_X540(_i)						(((_i) < 64) ? (0x01008 + ((_i) * 0x40)) : (0x0D008 + (((_i) - 64) * 0x40)))
#define RDH_X540(_i)						(((_i) < 64) ? (0x01010 + ((_i) * 0x40)) : (0x0D010 + (((_i) - 64) * 0x40)))
#define RDT_X540(_i)						(((_i) < 64) ? (0x01018 + ((_i) * 0x40)) : (0x0D018 + (((_i) - 64) * 0x40)))
#define RXDCTL_X540(_i)						(((_i) < 64) ? (0x01028 + ((_i) * 0x40)) : (0x0D028 + (((_i) - 64) * 0x40)))
#define RAL_X540(_i)						(((_i) <= 15) ? (0x05400 + ((_i) * 8)) : (0x0A200 + ((_i) * 8)))
#define RAH_X540(_i)						(((_i) <= 15) ? (0x05404 + ((_i) * 8)) : (0x0A204 + ((_i) * 8)))
#define MTA_X540(_i)						(0x05200 + ((_i) * 4))
#define DCA_RXCTRL(_i)						(((_i) <= 15) ? (0x02200 + ((_i) * 4)) : (((_i) < 64) ? (0x0100C + ((_i) * 0x40)) : (0x0D00C + (((_i) - 64) * 0x40))))

/* Transmit DMA registers */

#define TDBAL_X540(_i)						(0x06000 + ((_i) * 0x40)) /* 32 of these (0-31)*/
#define TDBAH_X540(_i)						(0x06004 + ((_i) * 0x40))
#define TDLEN_X540(_i)						(0x06008 + ((_i) * 0x40))
#define DCA_TXCTRL(_i)						(0x0600C + ((_i) * 0x40))
#define TDH_X540(_i)						(0x06010 + ((_i) * 0x40))
#define TDT_X540(_i)						(0x06018 + ((_i) * 0x40))
#define TXDCTL_X540(_i)						(0x06028 + ((_i) * 0x40))
#define TDWBAL_X540(_i)						(0x06038 + ((_i) * 0x40))
#define TDWBAH_X540(_i)						(0x0603C + ((_i) * 0x40))

/* flow control registers */

#define FCRTV								0x032A0
#define FCCFG								0x03D00
#define FCTTV(_i)							(0x03200 + ((_i) * 4))		/* 4 of these (0-3) */
#define FCRTL_X540(_i)						(0x03220 + ((_i) * 8))		/* 8 of these (0-7) */
#define FCRTH_X540(_i)						(0x03260 + ((_i) * 8))		/* 8 of these (0-7) */
#define TXDCTL_X540(_i)						(0x06028 + ((_i) * 0x40))
#define UTA_X540(_i)						(0x0F400 + ((_i) * 4))


///*
// * Bit-fields for individual registers
// */

/* CTRL */

#define CTRL_FD								0x00000001L
#define CTRL_LRST							0x00000008L
#define CTRL_ASDE							0x00000020L
#define CTRL_SLU							0x00000040L
#define CTRL_ILOS							0x00000080L
#define M__CTRL_SPEED						0xFFFFFCFFL
#define CTRL_SPEED10						0x00000000L
#define CTRL_SPEED100						0x00000100L
#define CTRL_SPEED1000						0x00000200L
#define CTRL_FRCSPD							0x00000800L
#define CTRL_FRCDPLX						0x00001000L
#define CTRL_RST							0x04000000L
#define CTRL_RFCE							0x08000000L
#define CTRL_TFCE							0x10000000L

/* CTRL Bit Masks */
#define IXGBE_CTRL_GIO_DIS  0x00000004 /* Global IO Master Disable bit */
#define IXGBE_CTRL_LNK_RST  0x00000008 /* Link Reset. Resets everything. */
#define IXGBE_CTRL_RST      0x04000000 /* Reset (SW) */
#define IXGBE_CTRL_RST_MASK (IXGBE_CTRL_LNK_RST | IXGBE_CTRL_RST)

#define DEV_RESET_INPROGRS					0x80000000L
#define PCIE_CTRL_GCR						0x5B00
#define PCIE_CTRL_GCR_X540					0x11000

/* STATUS */

#define	STATUS_LU							0x00000002L
#define	STATUS_LU_X540						0x00000008L

/* EECD */

#define EECD_CK								0x00000001L
#define EECD_CS								0x00000002L
#define EECD_DI								0x00000004L
#define	EECD_DO								0x00000008L
#define	EECD_REQ							0x00000040L
#define	EECD_FLASH_IN_USE					0x00000040L		/* i210 only */
#define	EECD_GNT							0x00000080L
#define	EECD_PRES							0x00000100L
//#define	EECD_SIZE							0x00000200L

/* CTRL_EXT */

#define CTRLEXT_ASDCHK						0x00001000L
#define CTRLEXT_SPDBYPS						0x00008000L
#define CTRLEXT_NS_DIS						0x00010000L
#define M__CTRLEXT_LINK						0xFF3FFFFFL
#define CTRLEXT_LINK_COPPER					0x00000000L
#define CTRLEXT_LINK_TBI					0x00C00000L

/* MDIC */

#define MDIC_DATA							0x0000FFFFL
#define MDIC_REG_ADD						0x001F0000L
#define MDIC_PHY_ADD						0x03E00000L
#define MDIC_OP_WRITE						0x04000000L
#define MDIC_OP_READ						0x08000000L
#define MDIC_READY							0x10000000L
#define MDIC_ERROR							0x40000000L

/* TXCW */

#define TXCW_ANE							0x80000000L


/* ICR */

#define I8257x_VALID_INTERRUPTS				0x000000DBL
#define I350_RESERVED_INTERRUPTS			0x0823822AL

#define ICR_TXQE							0x00000002L
#define ICR_LSC								0x00000004L
#define ICR_RXCFG							0x00000400L


/* RCTL */

#define RCTL_EN								0x00000002L
#define RCTL_SBP							0x00000004L
#define RCTL_UPE							0x00000008L
#define RCTL_MPE							0x00000010L
#define RCTL_LPE							0x00000020L
#define RCTL_RDMTS_1_2						0x00000000L
#define RCTL_BSIZE_2048						0x00000000L

/* TCTL */

#define TCTL_EN								0x00000002L

/* RAH */

#define RAH_AV								0x80000000L

/* EECD_X540 */

#define	EECD_SIZE_X540						0x00007800L
#define	EECD_SIZE_SHIFT_X540				11
#define	EECD_WORD_SIZE_SHIFT_X540			6

/* RXCTRL */

#define RXCTRL_EN							0x00000001L

/* DMATXCTL */

#define DMATXCTL_EN							0x00000001L

/* MSCA */

#define MSCA_MDIADD_MASK					0x0000FFFFL
#define M__MSCA_DEVADD						0x001F0000L
#define MSCA_DEVADD_SHIFT					16
#define M__MSCA_PORTADD						0x03E00000L
#define MSCA_PORTADD_SHIFT					21
#define M__MSCA_OPCODE						0x0C000000L
#define MSCA_OPCODE_SHIFT					26
#define MSCA_OP_ADDR_CYCLE					0x00000000L		// opcode 00
#define MSCA_OP_WRITE						0x04000000L		// opcode 01
#define MSCA_OP_READ_AUTOINC				0x08000000L		// opcode 10
#define MSCA_OP_READ						0x0C000000L		// opcode 11
#define MSCA_MDICMD							0x40000000L
#define MSCA_MDICMD_SHIFT					30

#define MSCA_CMD_TIMEOUT					100

/* MSRWD */

#define MSRWD_WRITEDATA_MASK				0x0000FFFFL
#define MSRWD_READDATA_MASK					0xFFFF0000L


/* MACC */

#define MACC_FLU							0x00000001L

/* SW_FW_SYNC */

#define SWSM_SMBI							0x00000001

#define SW_FW_SYNC_SW_NVM					0x00000001
#define SW_FW_SYNC_SW_PHY0					0x00000002
#define SW_FW_SYNC_SW_PHY1					0x00000004
#define SW_FW_SYNC_SW_MAC					0x00000008
#define SW_FW_SYNC_HW_NVM					0x00000010
#define SW_FW_SYNC_FW_NVM					0x00000020
#define SW_FW_SYNC_FW_PHY0					0x00000040
#define SW_FW_SYNC_FW_PHY1					0x00000080
#define SW_FW_SYNC_FW_MAC					0x00000100
#define SW_FW_SYNC_NVM_UPDATE				0x00000200
#define SW_FW_SYNC_SW_MNG					0x00000400
#define SW_FW_SYNC_SEMAPHORE				0x80000000

/* RXDCTL_X540 */
/* Receive Config masks */
#define RXDCTL_ENABLE						0x02000000		/* Enable specific Rx Queue */
#define RXDCTL_SWFLSH						0x04000000		/* Rx Desc. write-back flushing */
#define RXDCTL_RLPMLMASK					0x00003FFF		/* Only supported on the X540 */
#define RXDCTL_RLPML_EN						0x00008000		
#define RXDCTL_VME							0x40000000		/* VLAN mode enable */

/* DCA_RXCTRL */

#define DCA_RXCTRL_DATA_WRO_EN				0x00002000		// Rx data Write Relax Order Enable
#define DCA_RXCTRL_HEAD_WRO_EN				0x00008000		// Rx Split Header Relax Order Enable

/* TXDCTL_X540 */
/* Transmit Config masks */
#define TXDCTL_ENABLE						0x02000000		/* Enable specific Tx Queue */
#define TXDCTL_SWFLSH						0x04000000		/* Tx Desc. write-back flushing */
#define TXDCTL_WTHRESH_SHIFT				16 				/* shift to WTHRESH bits */
/* Enable short packet padding to 64 bytes */
#define ITX_PAD_ENABLE						0x00000400
#define IJUMBO_FRAME_ENABLE					0x00000004		/* Allow jumbo frames */
/* This allows for 16K packets + 4k for vlan */
#define MAX_FRAME_SZ						0x40040000

/* DCA_TXCTRL */

#define DCA_TXCTRL_DESC_WRO_EN				0x00000800		// Relax Order Enable of Tx Descriptor


/* FCTRL */

#define FCTRL_SBP							0x00000002L
#define FCTRL_MPE							0x00000100L
#define FCTRL_UPE							0x00000200L
#define FCTRL_BAM							0x00000400L

/* HLREG0 */

#define HLREG0_TXCRCEN						0x00000001		/* bit  0 */
#define HLREG0_RXCRCSTRP					0x00000002		/* bit  1 */
#define HLREG0_JUMBOEN						0x00000004		/* bit  2 */
#define HLREG0_TXPADEN						0x00000400		/* bit 10 */
#define HLREG0_TXPAUSEEN					0x00001000		/* bit 12 */
#define HLREG0_RXPAUSEEN					0x00004000		/* bit 14 */
#define HLREG0_LPBK							0x00008000		/* bit 15 */
#define HLREG0_MDCSPD						0x00010000		/* bit 16 */
#define HLREG0_CONTMDC						0x00020000		/* bit 17 */
#define HLREG0_CTRLFLTR						0x00040000		/* bit 18 */
#define HLREG0_PREPEND						0x00F00000		/* bits 20-23 */
#define HLREG0_PRIPAUSEEN					0x01000000		/* bit 24 */
#define HLREG0_RXPAUSERECDA					0x06000000		/* bits 25-26 */
#define HLREG0_RXLNGTHERREN					0x08000000		/* bit 27 */
#define HLREG0_RXPADSTRIPEN					0x10000000		/* bit 28 */

/*
 * AUTOC
 */
#define AUTOC_LMS_SHIFT                        13
#define AUTOC_LMS_MASK                        (0x7 << AUTOC_LMS_SHIFT)
#define AUTOC_FLU                                    0x00000001
#define AUTOC_LMS_10G_LINK_NO_AN    (0x1 <<  AUTOC_LMS_SHIFT)
#define AUTOC_AN_RESTART                     0x00001000

/* PCI Express Control */
#define GCR_CMPL_TMOUT_MASK					0x0000F000
#define GCR_CMPL_TMOUT_10ms					0x00001000
#define GCR_CMPL_TMOUT_RESEND				0x00010000
#define GCR_CAP_VER2						0x00040000
				
#define GCR_EXT_MSIX_EN						0x80000000
#define GCR_EXT_BUFFERS_CLEAR				0x40000000
#define GCR_EXT_VT_MODE_16					0x00000001
#define GCR_EXT_VT_MODE_32					0x00000002
#define GCR_EXT_VT_MODE_64					0x00000003
#define GCR_EXT_SRIOV						(GCR_EXT_MSIX_EN | GCR_EXT_VT_MODE_64)

#define X540_MAX_TX_QUEUES					128
#define X540_MAX_RX_QUEUES					128
#define X540_MAX_RAR_ENTRIES				128
#define X540_MC_TBL_SIZE					128

/* Receive descriptor status */

#define RDESC_STATUS_DD						0x01
#define RDESC_STATUS_EOP					0x02
//#define RDESC_STATUS_IXSM					0x04
//#define RDESC_STATUS_VP						0x08
//#define RDESC_STATUS_TCPCS					0x20
//#define RDESC_STATUS_IPCS					0x40
//#define RDESC_STATUS_PIF					0x80

///* Receive descriptor errors */

//#define RDESC_ERRORS_CE		0x01
//#define RDESC_ERRORS_SE						0x02
//#define RDESC_ERRORS_SEQ					0x04
//#define RDESC_ERRORS_CXE					0x10
//#define RDESC_ERRORS_TCPE					0x20
//#define RDESC_ERRORS_IPE					0x40
//#define RDESC_ERRORS_RXE					0x80

/* Transmit descriptor command */

#define TDESC_COMMAND_EOP					0x01
#define TDESC_COMMAND_IFCS					0x02
//#define TDESC_COMMAND_IC					0x04
#define TDESC_COMMAND_RS					0x08
//#define TDESC_COMMAND_RPS					0x10
//#define TDESC_COMMAND_DEXT					0x20
//#define TDESC_COMMAND_VLE					0x40
//#define TDESC_COMMAND_IDE					0x80

///* Transmit descriptor status */

#define TDESC_STATUS_DD						0x01
//#define TDESC_STATUS_EC						0x02
//#define TDESC_STATUS_LC						0x04
//#define TDESC_STATUS_TU						0x08

/* EEPROM Contents */

#define EEPROM_IA2_1						0x00
#define EEPROM_IA4_3						0x01
#define EEPROM_IA6_5						0x02
#define EEPROM_CSUM							0x3F

#define EEPROM_SIZE							(EEPROM_CSUM + 1)

// X540 NVM Map (sections offsets in 'EEPROM')

#define	X540_EEP_ANALOG						0x03
#define	X540_EEP_PHY						0x04
#define	X540_EEP_OPT_ROM					0x05
#define	X540_EEP_LAN0						0x09
#define	X540_EEP_LAN1						0x0A
#define	X540_EEP_MAC0						0x0B
#define	X540_EEP_MAC1						0x0C
#define	X540_EEP_FW							0x0F

/* INVM data structure ids and types */

#define INVM_DATA_REG(_n)					(0x12120 + 4*(_n))
#define INVM_SIZE							64
#define INVM_ULT_BYTES_SIZE					8
#define INVM_RECORD_SIZE_IN_BYTES			4
#define INVM_VER_FIELD_ONE					0x1FF8
#define INVM_VER_FIELD_TWO					0x7FE000
#define INVM_IMGTYPE_FIELD					0x1F800000
#define INVM_MAJOR_MASK						0x3F0
#define INVM_MINOR_MASK						0xF
#define INVM_MAJOR_SHIFT					4

#define INVM_STRUCT_TYPE_MASK				0x0007
#define	INVM_NVM_END						0x0000
#define	INVM_WORD_AUTOLOAD					0x0001
#define	INVM_CSR_AUTOLOAD					0x0002
#define	INVM_PHY_AUTOLOAD					0x0003
#define	INVM_INVALIDATED					0x0007

#define INVM_RESET_TYPE_MASK				0x0018
#define INVM_LOAD_POWER_UP					0x0000			/* only reset on power-up */
#define INVM_LOAD_PCIE_RESET				0x0008			/* and on power-up reset */
#define INVM_LOAD_SW_RESET					0x0018			/* and on pcie reset and power-up reset */

#define INVM_WORD_ADDR_MASK					0xFE00
#define INVM_CSR_ADDR_MASK					0x7FFF
#define INVM_PHY_ADDR_MASK					0xF800

/* SPI EEPROM Status Register */
#define EEPROM_STATUS_RDY_SPI				0x01
#define EEPROM_STATUS_WEN_SPI				0x02
#define EEPROM_STATUS_BP0_SPI				0x04
#define EEPROM_STATUS_BP1_SPI				0x08
#define EEPROM_STATUS_WPEN_SPI				0x80

/* EEPROM Commands - SPI */
#define EEPROM_READ_OPCODE_SPI				0x03			/* EEPROM read opcode */
#define EEPROM_WRITE_OPCODE_SPI				0x02			/* EEPROM write opcode */
#define EEPROM_A8_OPCODE_SPI				0x08			/* opcode bit-3 = address bit-8 */
#define EEPROM_WREN_OPCODE_SPI				0x06			/* EEPROM set Write Enable latch */
#define EEPROM_WRDI_OPCODE_SPI				0x04			/* EEPROM reset Write Enable latch */
#define EEPROM_RDSR_OPCODE_SPI				0x05			/* EEPROM read Status register */
#define EEPROM_WRSR_OPCODE_SPI				0x01			/* EEPROM write Status register */
#define EEPROM_ERASE4K_OPCODE_SPI			0x20			/* EEPROM ERASE 4KB */
#define EEPROM_ERASE64K_OPCODE_SPI			0xD8			/* EEPROM ERASE 64KB */
#define EEPROM_ERASE256_OPCODE_SPI			0xDB			/* EEPROM ERASE 256B */

///* EEPROM Checksum value */

#define EEPROM_CHECKSUM						(0xBABA)

//#define EEPROM_W_OP_SIZE					(5)				/* for write enable/disable */
//#define EEPROM_W_INDEX_SIZE					(4)
//#define EEPROM_OPCODE_SIZE					(3)				/* for read/write */
//#define EEPROM_INDEX_SIZE					(6)
//#define EEPROM_DATA_SIZE					(16)			/* 16-bit EEPROM data */

//#define EEPROM_READ_OPCODE					(0x6)
//#define EEPROM_WRITE_OPCODE					(0x5)
//#define EEPROM_ERASE_OPCODE					(0x7)
//#define EEPROM_EWEN_OPCODE					(0x13)
//#define EEPROM_EWDS_OPCODE					(0x10)

#define EEPROM_TIMEOUT						(2000)			/* count * 5uS, timeout for command */


/* PHY registers i350 & i210*/

#define PHY_CTRL							0x00
#define PHY_CTRL_RESET						0x8000U
#define M__PHY_CTRL_SPEED					0x2040U			/* uses bits 13 (LSB) and 6 (MSB) */
#define PHY_CTRL_SPEED_1000					0x0040U
#define PHY_CTRL_SPEED_100					0x2000U
#define PHY_CTRL_SPEED_10					0x0000U
#define PHY_CTRL_AUTO_NEG					0x1000U
#define PHY_CTRL_RESTART_AUTO				0x0200U
#define PHY_CTRL_DUPLEX						0x0100U

#define PHY_STATUS							0x01

#define PHY_ADVERT							0x04
#define PHY_ADVERT_10TXHD					0x0020
#define PHY_ADVERT_10TXFD					0x0040
#define PHY_ADVERT_100TXHD					0x0080
#define PHY_ADVERT_100TXFD					0x0100

#define PHY_1000							0x09
#define PHY_1000_1000TXHD					0x0100
#define PHY_1000_1000TXFD					0x0200

/* x540 MDIO Manageable Devices (MMDs). */
#define MDIO_MMD_PMAPMD						1		/* Physical Medium Attachment/Physical Medium Dependent */
#define MDIO_MMD_WIS						2		/* WAN Interface Sublayer */
#define MDIO_MMD_PCS						3		/* Physical Coding Sublayer */
#define MDIO_MMD_PHYXS						4		/* PHY Extender Sublayer */
#define MDIO_MMD_DTEXS						5		/* DTE Extender Sublayer */
#define MDIO_MMD_TC							6		/* Transmission Convergence */
#define MDIO_MMD_AN							7		/* Auto-Negotiation */
#define MDIO_MMD_C22EXT						29		/* Clause 22 extension */
#define MDIO_MMD_VEND1						30		/* Vendor specific 1 */
#define MDIO_MMD_VEND2						31		/* Vendor specific 2 */


/* x540 PHY registers*/

#define X540_PHY_REG(mmd, reg)				((reg) | ((mmd) << MSCA_DEVADD_SHIFT))
#define M__PHY_CTRL_SPEED					0x2040U			/* uses bits 13 (LSB) and 6 (MSB) */
#define PHY_CTRL_SPEED_10GB					0x2040U
#define PHY_CTRL_SPEED_1GB					0x0040U
#define PHY_CTRL_SPEED_100MB				0x2000U
#define PHY_CTRL_LOW_POWER					0x0800U
#define PHY_CTRL_SPEED_SEL_10GB				0x0004U
#define PHY_CTRL_LOOPBACK					0x0001U

//#define PHY_STATUS							0x01			// MMD 1:1
//#define PHY_PID								0x02			/* PHY Identifier */

//#define PHY_ADVERT							0x04			// MMD 1:4
#define PHY_ADVERT_10GB						0x0001U
#define PHY_ADVERT_1GB						0x0010U
#define PHY_ADVERT_100MB					0x0020U
#define PHY_ADVERT_10MB						0x0040U

#define PHY_DEV_STAT1						0x05			// MMD 1:5

#define PHY_DEV_STAT2						0x06			// MMD 1:5

#define PHY_STATUS2							0x08			// MMD 1:5
#define PHY_STATUS2_DEV_PRESENT				0x8000U

#define PHY_PMA_RX_RSVD_VENDOR_PROV1		0xE400U			// MMD 1:0xE400
#define PHY_PMA_EXT_PHY_LOOPBACK			0x8000U

#define PHY_AN_CTRL							0x00			// MMD 7:0
#define PHY_AN_CTRL_RESTART					0x0200U
#define PHY_AN_AUTO_NEGOTIATE				0x1000U

#define PHY_AN_ADVERTISE					0x10			// MMD 7:0x10
#define M__PHY_AN_ABILITY					0x1FE0U
#define PHY_AN_ABILITY_100HALF				0x0080U
#define PHY_AN_ABILITY_100FULL				0x0100U
#define PHY_AN_ABILITY_100FULL				0x0100U
#define PHY_AN_SYM_PAUSE_FULL				0x0400U
#define PHY_AN_ASM_PAUSE_FULL				0x0800U

#define PHY_AN_10GB_CTRL					0x20			// MMD 7:0x20
#define PHY_AN_10GB_CTRL_ADV10G				0x1000U

#define PHY_MII_AUTONEG_VENDOR_PROV1		0xC400			// MMD 30:0xC400
#define PHY_MII_1GBASE_T_ADVERTISE			0x8000U

#define	PCI_ROM_BASE						0x30			/* Expansion ROM base address	*/

#define IF_COPPER							0				/* media interface type */
#define IF_FIBER							1
#define IF_SERDES							2

#define CTRLT_UNKNOWN						0
#define CTRLT_I350							20				/* continuation from controllers in eth.h */
#define CTRLT_I210							21
#define CTRLT_X540							22
#define CTRLT_82599_SFP                23

#define LINE_BUFF_SIZE						80

//#define PCI_SCANNING						0
#define PCI_SUCCESS							1
//#define DEVICE_NOT_FOUND					0xFFFF

#define SPEED_AUTO							0
#define SPEED_10M							1				/* interface speed Mbits/Second */
#define SPEED_100M							2
#define SPEED_1G							3
#define SPEED_10G							4

#define LB_NONE								0				/* signal loopback modes */
#define LB_INTERNAL							1
#define LB_LOCAL							2
#define LB_EXTERNAL							3
#define LB_CHANNEL                          4

#define MAX_PHY_REG_ADDRESS					31

#define S__WTHRESH							16

#define ETH_TOP_OH							14				/* src addr + dest addr + length */
#define ETH_TAIL_OH							4				/* crc32 */
#define ETH_OH								(ETH_TOP_OH /*+ ETH_TAIL_OH*/)

#define LOOP_BACK_SIZE						(3072UL)

#define EN_TX								1				/* enable transmitter */
#define EN_RX								2				/* enable receiver */
#define EN_RXTX								(EN_TX | EN_RX)

#define SERDESCTL							0x00024L

/* SerDes Ethernet controller */
#define ENABLE_SERDES_LOOPBACK				0x0410
#define SCTL_DISABLE_SERDES_LOOPBACK		0x0400

#define CONNSW								0x00034L		/* Copper/Fiber switch control - RW */
#define CONNSW_ENRGSRC						0x4
#define PCS_LCTL_FLV_LINK_UP				1
#define PCS_LCTL_FSV_100					2
#define PCS_LCTL_FSV_1000					4
#define PCS_LCTL_FDV_FULL					8
#define PCS_LCTL_FSD						0x10
#define PCS_LCTL_FORCE_LINK 				0x20
#define PCS_LCTL_FORCE_FCTRL				0x80
#define PCS_LCTL_AN_ENABLE					0x10000
#define PCS_LCTL_AN_RESTART					0x20000
#define PCS_LCTL_AN_TIMEOUT					0x40000
#define PCS_LCTL							0x04208L		/* PCS Link Control - RW */
#define PCS_CFG								0x04200L
#define PCS_LSTAT							0x0420CL
#define PCS_LSTS_SYNK_OK					0x10
#define PCS_LINK_OK							0x01


/* typedefs */

typedef struct tagRdesc
{
	UINT32	dAddrL;					/* 64-bit buffer address */
	UINT32	dAddrH;
	UINT16	wLength;
	UINT16	wPacketCsum;
	UINT8	bStatus;
	UINT8	bErrors;
	UINT16	wSpecial;
} RDESC;

typedef struct tagTdesc				/* Legacy Mode */
{
	UINT32	dAddrL;					/* 64-bit buffer address */
	UINT32	dAddrH;
	UINT16	wLength;
	UINT8	bCsOffset;
	UINT8	bCommand;
	UINT8	bStatus;
	UINT8	bCsStart;
	UINT16	wSpecial;
} TDESC;

typedef struct tagDevice
{
	UINT8	bBus;					/* PCI address of device */
	UINT8	bDev;
	UINT8	bFunc;
	UINT8	bType;					/* interface type (IF_COPPER, IF_FIBER) */
	UINT8	bMultiFunc;				/* non-zero indicates multi-function device */

	UINT8	bVector;

	UINT8	bController;			/* controller type (82543, 82544...) */
	char	achCtrlName [80];		/* controller name */

	UINT8	bPhyAddress;
	UINT8	bMaxSpeed;				/* maximum speed on external connector (def=1000) */
	UINT8	bIndex;					/* controller number */

	/* Use 8-bit pointers as this simplifies register indexing. These can
	 * then be cast to whatever type is required.
	 */

	volatile UINT8*	pbReg;			/* internal registers */
	volatile UINT8*	pbRomRead;		/* BIOS ROM, read accesses */
	volatile UINT8*	pbRomWrite;		/* BIOS ROM, write accesses */
	volatile UINT8*	pbFlashBase;	/* ICH9 NVM Flash Base Address */
	volatile UINT32	dGbeFlashBase;	/* Gige Flash Base Address from Flashbase */

	UINT32	dTxDescAddr;			/* base address of TX descriptor ring */
	UINT32	dTxDescSize;			/* length of TX descriptor ring */
	volatile TDESC*	psTxDesc;		/* pointer to TX descriptor ring */

	UINT32	dTxBuffAddr;			/* base address of TX buffers */
	UINT8*	pbTxBuff;				/* pointer to TX buffers */

	UINT32	dRxDescAddr;			/* base address of RX descriptor ring */
	UINT32	dRxDescSize;			/* length of RX descriptor ring */
	volatile RDESC*	psRxDesc;		/* pointer to RX descriptor ring */

	UINT32	dRxBuffAddr;			/* base address of RX buffers */
	UINT8*	pbRxBuff;				/* pointer to RX buffers */

	/* TX & RX descriptor indexes */

	UINT16	wTxCurrentDescr;		/* start of most recent transmit chain */
	UINT32  	dFlashBankSize;		/* ICH9 Flash Bank Size */
	UINT16  	wFlashWordSize;		/* ICH9 Flash Word Size */

	char	achErrorMsg[200];		/* used for composite messages */
}DEVICE;

/* externals */

void vGetEthMacAddr (UINT8  bInstance, UINT8* pbAddress,UINT8 bMode);
UINT32 dGetNumberOfEthInstances(void);
