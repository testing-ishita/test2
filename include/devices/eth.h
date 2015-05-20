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

/* eth.h - definitions for ethernet devices
 */

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/eth.h,v 1.4 2014-03-04 10:00:19 mgostling Exp $
*
* $Log: eth.h,v $
* Revision 1.4  2014-03-04 10:00:19  mgostling
* Added support for i350AM4 Quad Ethernet controller
*
* Revision 1.3  2014-01-29 13:22:35  hchalla
* Removed unwanted tabs and spaces.
*
* Revision 1.2  2013-11-25 11:02:01  mgostling
* Add support for i350AM2 and i210 controllers.
*
* Revision 1.1  2013-09-04 07:40:27  chippisley
* Import files into new source repository.
*
* Revision 1.13  2012/09/19 09:48:14  jthiru
* Updated Display config test for vx91x boards without P0 connector, updated PCIe port width test for vx91x
*
* Revision 1.12  2012/04/12 13:42:46  hchalla
* Added flag for ethernet to display the mac address while running t,999 test.
*
* Revision 1.11  2011/09/29 14:39:43  hchalla
* New ethernet device added for tr803.
*
* Revision 1.10  2011/08/02 17:08:33  hchalla
* New variables and macros added.
*
* Revision 1.9  2011/02/01 12:12:07  hmuneer
* C541A141
*
* Revision 1.8  2010/11/04 17:32:48  hchalla
* Added support for new controller 82577 (Added new defines for registers).
*
* Revision 1.7  2010/09/10 09:54:02  cdobson
* Added new controller type.
*
* Revision 1.6  2010/06/23 10:59:59  hmuneer
* CUTE V1.01
*
* Revision 1.5  2010/03/31 16:28:20  hmuneer
* no message
*
* Revision 1.4  2010/01/19 12:04:10  hmuneer
* vx511 v1.0
*
* Revision 1.3  2009/06/10 08:48:40  jthiru
* Added routine to display Ethernet MAC address in config test case
*
* Revision 1.2  2009/05/15 11:01:31  jthiru
* Added EEPROM Write and Ext loopback test and enhancements
*
* Revision 1.1  2009/02/02 17:12:39  jthiru
* Added Ethernet definitions
*
*
*/


/* defines */

/* Error Codes at offset of 6, one for each instance */

#define	E__TEST_NO_DEVICE		E__BIT 
#define E__RESET_TIMEOUT		E__BIT + 0x6
#define E__PHY_RESET_TIMEOUT	E__BIT + 0xC
#define E__CTRL_RESET_TIMEOUT	E__BIT + 0x12
#define E__FAIL_ROLLING_1		E__BIT + 0x18
#define E__FAIL_ROLLING_0		E__BIT + 0x1E
#define E__TEST_NO_MEM			E__BIT + 0x24
#define E__EEPROM_ACCESS		E__BIT + 0x2A
#define E__EEPROM_CHECKSUM		E__BIT + 0x30
#define E__PHYREADERR			E__BIT + 0x36
#define E__INT_LB_TIMED_OUT		E__BIT + 0x3C
#define E__INT_LB_BAD_DATA		E__BIT + 0x42
#define E__ICR_NOT_ZERO			E__BIT + 0x48
#define E__NO_MASKINT			E__BIT + 0x4E
#define E__WRONG_MASKINT		E__BIT + 0x54
#define E__PHYRESET_TIMEOUT		E__BIT + 0x5A
#define E__LOC_LB_BAD_MEDIA		E__BIT + 0x60
#define E__LOC_LB_LINK_FAIL		E__BIT + 0x66
#define E__LOC_LB_TIMED_OUT		E__BIT + 0x6C
#define E__LOC_LB_BAD_DATA		E__BIT + 0x72
#define E__EEPROM_TIMEOUT		E__BIT + 0x78
#define E__SPI_EEPROM_TIMEOUT	E__BIT + 0x7E
#define E__EEPROM_WRITE			E__BIT + 0x84
#define E__SPI_EEPROM_WRITE		E__BIT + 0x8A
#define E__EEPROM_BAD_MEDIA		E__BIT + 0x90
#define E__EEPROM_RW			E__BIT + 0x96

/* PCI register access size codes */

#define PCI_BYTE					0x01	
#define PCI_WORD					0x02
#define PCI_DWORD					0x03	

#define MAX_CONTROLLERS			2				/* how many controllers we may use */

	/* Masks for Various Registers */

#define PCI_IO_BASE_MASK			0xFFFFFFFC
#define PCI_MEM_BASE_MASK			0xFFFFFFF0
#define PCI_ROM_BASE_MASK			0xFFFFF800
#define TX_DESC_BASE			0x00400000L
#define TX_DESC_LENGTH			0x00100000L
#define TX_BUFF_BASE			0x00600000L
#define TX_BUFF_LENGTH			0x00100000L
#define TX_BUFF_SIZE			0x00000800L		/* 2-kByte TX buffers */
#define TX_NUM_DESC				(TX_DESC_LENGTH / TX_BUFF_SIZE / MAX_CONTROLLERS)

#define RX_DESC_BASE			0x00800000L
#define RX_DESC_LENGTH			0x00100000L
#define RX_BUFF_BASE			0x00A00000L
#define RX_BUFF_LENGTH			0x00100000L
#define RX_BUFF_SIZE			0x00000800L		/* 2-kByte RX buffers */
#define RX_NUM_DESC				(RX_DESC_LENGTH / RX_BUFF_SIZE / MAX_CONTROLLERS)

/* PCI Configuration Space Register Definitions */
#define VID_INTEL		0x8086

#define DID_82543GC_F	0x1001		/* with fiber interface */
#define DID_82543GC_C	0x1004		/* with copper interface */
#define DID_82544EI		0x1008		/* copper only */
#define DID_82546EB		0x1010		/* copper only */
#define DID_82546GB		0x1079		/* copper only */
#define DID_82540EM		0x100E		/* copper only */
#define DID_82541PI		0x1076		/* copper only */
#define DID_82541PIa	0x107C		/* copper only, lead-free */
#define DID_82571EB_C	0x105E		/* pci express dual gigabit copper (unprogrammed default) */
#define DID_82571EB_F	0x105F		/* pci express dual gigabit fiber */
#define DID_82571EB_S	0x1060		/* pci express dual gigabit serdes */
#define DID_82573L		0x109A		/* pci express gigabit */
#define DID_82575EB_C	0x10A7		/* pci express dual gigabit copper */
#define DID_82577	 	0x10EA		/* pci express gigabit */
#define DID_82580_C	    0x1516		/* pci express dual gigabit copper (unprogrammed default) */ /*Changed from 150E to 1516*/
#define DID_82580_CS    0x150E      /* pci express dual gigabit copper - serdes */
#define DID_82580_EB    0x1510
#define DID_82579_C     0x1502

#define DID_82567	0x10F5		/* vx511*/
#define DID_82574	0x10D3		/* vx511*/
//[original the one below is just for testing]#define DID_82576	0x10E7		/* vx511*/
#define DID_82576EB		0x10E7		/* vx511*/
#define DID_82576EB_C	0x10C9		/* vx511*/

#define DID_i350AMx_C	0x1521		/* trb1x: pci express dual gigabit copper */
#define DID_i350AMx_S	0x1523		/* trb1x: pci express dual gigabit serdes */
#define DID_i210		0x1533		/* trb1x: pci express megabit copper */



#define DIDVID_FIBER	 ((DID_82543GC_F << 16) | VID_INTEL)
#define DIDVID_COPPER	 ((DID_82543GC_C << 16) | VID_INTEL)
#define DIDVID_82544EI	 ((DID_82544EI << 16)   | VID_INTEL)
#define DIDVID_82546EB	 ((DID_82546EB << 16)   | VID_INTEL)
#define DIDVID_82546GB	 ((DID_82546GB << 16)   | VID_INTEL)
#define DIDVID_82540EM	 ((DID_82540EM << 16)   | VID_INTEL)
#define DIDVID_82541PI   ((DID_82541PI << 16)   | VID_INTEL)
#define DIDVID_82541PIa  ((DID_82541PIa << 16)  | VID_INTEL)
#define DIDVID_82571EB_F ((DID_82571EB_F << 16) | VID_INTEL)
#define DIDVID_82571EB_C ((DID_82571EB_C << 16) | VID_INTEL)
#define DIDVID_82571EB_S ((DID_82571EB_S << 16) | VID_INTEL)
#define DIDVID_82573L	 ((DID_82573L << 16)    | VID_INTEL)
#define DIDVID_82575EB_C ((DID_82575EB_C << 16) | VID_INTEL)
#define DIDVID_82577   	 ((DID_82577 << 16)     | VID_INTEL)
#define DIDVID_82580_C   ((DID_82580_C << 16) | VID_INTEL)
#define DIDVID_82580_CS  ((DID_82580_CS << 16) | VID_INTEL)
#define DIDVID_82579_C   ((DID_82579_C << 16) | VID_INTEL)
#define DIDVID_82580_EB  ((DID_82580_EB << 16) | VID_INTEL)

#define DIDVID_82567     ((DID_82567 << 16) | VID_INTEL)
#define DIDVID_82574     ((DID_82574 << 16) | VID_INTEL)
#define DIDVID_82576EB     ((DID_82576EB << 16) | VID_INTEL)
#define DIDVID_82576EB_C     ((DID_82576EB_C << 16) | VID_INTEL)

#define DIDVID_i350AMx_C	((DID_i350AMx_C << 16) | VID_INTEL)
#define DIDVID_i350AMx_S	((DID_i350AMx_S << 16) | VID_INTEL)
#define DIDVID_i210			((DID_i210 << 16) | VID_INTEL)


#define PCI_VID					0x00	/* Vendor ID offset				*/
#define PCI_VENDOR_ID			0x00	/*		-"-						*/
#define PCI_DID					0x02	/* Device ID offset				*/
#define PCI_DEVICE_ID			0x02	/*		-"-						*/
#define PCI_CACHE_LINE_SIZE		0x0C
#define	PCI_BASE_ADDR_1			0x10
#define	PCI_BASE_ADDR_2			0x14
#define PCI_ROMEN				0x00000001	/* Expansion ROM enable 	*/

/* i8257x Memory Mapped Registers
 */

#define I8257x_REG_SIZE	0x00020000
#define I350_REG_SIZE	0x00040000

#define CTRL		0x00000L	/* General */
#define STATUS		0x00008L
#define EECD		0x00010L
#define EERD		0x00014L	/* 82571 */
#define CTRL_EXT	0x00018L
#define MDIC		0x00020L	/* only 82544 */
#define FCAL		0x00028L
#define FCAH		0x0002CL
#define FCT			0x00030L
#define VET			0x00038L
#define FCTTV		0x00170L
#define TXCW		0x00178L
#define RXCW		0x00180L

#define PBA			0x01000L	/* DMA */
#define EEMNGCTL	0x01010L

#define ICR			0x000C0L	/* Interrupt */
#define ICS			0x000C8L
#define IMS			0x000D0L
#define IMC			0x000D8L

#define RCTL		0x00100L	/* Receive */
#define FCRTL		0x02160L
#define FCRTH		0x02168L
#define RDBAL		0x02800L
#define RDBAH		0x02804L
#define RDLEN		0x02808L
#define RDH			0x02810L
#define RDT			0x02818L
#define RDTR		0x02820L

#define RXDCTL		0x02828L	/* RX DMA */

#define TCTL		0x00400L	/* Transmit */
#define TIPG		0x00410L
#define TBT			0x00448L
#define AIT			0x00458L

#define TXDMAC		0x03000L	/* TX DMA */

#define PBMPN		0x03400L	/* FIFO buffer page number */
								/* Only in 82575		   */

#define TDBAL		0x03800L	/* Transmit */
#define TDBAH		0x03804L
#define TDLEN		0x03808L
#define TDH			0x03810L
#define TDT			0x03818L
#define TIDV		0x03820L

#define TXDCTL		0x03828L	/* TX DMA */

#define RXCSUM		0x05000L	/* RX DMA */

#define MTA			0x05200L	/* Receive */
#define RAL			0x05400L
#define RAH			0x05404L
#define VFTA		0x05600L

#define CRCERRS		0x04000L	/* Statistics */
#define ALGNERRC	0x04004L
#define SYMERRS		0x04008L
#define RXERRC		0x0400CL
#define MPC			0x04010L
#define SCC			0x04014L
#define ECOL		0x04018L
#define MCC			0x0401CL
#define LATECOL		0x04020L
#define COLC		0x04028L
#define DC			0x04030L
#define TNCRS		0x04034L
#define SEC			0x04038L
#define CEXTERR		0x0403CL
#define RLEC		0x04040L
#define XONRXC		0x04048L
#define XONTXC		0x0404CL
#define XOFFRXC		0x04050L
#define XOFFTXC		0x04054L
#define FCRUC		0x04058L
#define PRC64		0x0405CL
#define PRC127		0x04060L
#define PRC255		0x04064L
#define PRC511		0x04068L
#define PRC1023		0x0406CL
#define PRC1522		0x04070L
#define GPRC		0x04074L
#define BPRC		0x04078L
#define MPRC		0x0407CL
#define GPTC		0x04080L
#define GORCL		0x04088L
#define GORCH		0x0408CL
#define GOTCL		0x04090L
#define GOTCH		0x04094L
#define RNBC		0x040A0L
#define	RUC			0x040A4L
#define RFC			0x040A8L
#define ROC			0x040ACL
#define RJC			0x040B0L
#define TORL		0x040C0L
#define TORH		0x040C4L
#define TOTL		0x040C8L
#define TOTH		0x040CCL
#define TPR			0x040D0L
#define TPT			0x040D4L
#define PTC64		0x040D8L
#define PTC127		0x040DCL
#define PTC255		0x040E0L
#define PTC511		0x040E4L
#define PTC1023		0x040E8L
#define PTC1522		0x040ECL
#define MPTC		0x040F0L
#define BPTC		0x040F4L
#define TSCTC		0x040F8L	/* only 82544 */
#define TSCTFC		0x040FCL	/* only 82544 */

#define RDFH		0x02410L	/* Diagnostic */
#define RDFT		0x02418L
#define RDFHS		0x02420L
#define RDFTS		0x02428L
#define RDFPC		0x02430L
#define TDFH		0x03410L
#define TDFT		0x03418L
#define TDFHS		0x03420L
#define TDFTS		0x03428L
#define TDFPC		0x03430L
#define TSPMT		0x03830L	/* only 82544 */
#define PBM			0x10000L
#define PBM_SIZE	0x10000L
#define ERT         0x02008L    /* only 82567 */
#define MDICNFG		0x00E04L	/* only on i350 */


/*
 * Bit-fields for individual registers
 */

/* CTRL */

#define CTRL_FD				0x00000001L
#define CTRL_BEM			0x00000002L
#define CTRL_PRIOR			0x00000004L		/* only 82543GC */
#define CTRL_LRST			0x00000008L
#define DEV_RESET			0x20000000L
#define CTRL_ASDE			0x00000020L
#define CTRL_SLU			0x00000040L
#define CTRL_ILOS			0x00000080L
#define M__CTRL_SPEED		0xFFFFFCFFL
#define CTRL_SPEED10		0x00000000L
#define CTRL_SPEED100		0x00000100L
#define CTRL_SPEED1000		0x00000200L
#define CTRL_FRCSPD			0x00000800L
#define CTRL_FRCDPLX		0x00001000L
#define CTRL_SPD_SEL  		0x00000300L  /* Speed Select Mask */
#define M__CTRL_SWDPINSLO	0xFFC3FFFFL
#define CTRL_SWDPINSLO0		0x00040000L
#define CTRL_SWDPINSLO1		0x00080000L
#define CTRL_SWDPINSLO2		0x00100000L
#define CTRL_SWDPINSLO3		0x00200000L
#define M__CTRL_SWDPIOLO	0xFC3FFFFFL
#define CTRL_SWDPIOLO0		0x00400000L
#define CTRL_SWDPIOLO1		0x00800000L
#define CTRL_SWDPIOLO2		0x01000000L
#define CTRL_SWDPIOLO3		0x02000000L
#define CTRL_RST			0x04000000L
#define CTRL_RFCE			0x08000000L
#define CTRL_TFCE			0x10000000L
#define CTRL_VME			0x40000000L
#define CTRL_PHY_RESET		0x80000000L

#define DEV_RESET_INPROGRS  0x80000000L
#define PCIE_CTRL_GCR       0x5B00

/* STATUS */

#define	STATUS_FD			0x00000001L
#define	STATUS_LU			0x00000002L
#define	STATUS_TCKOK		0x00000004L
#define	STATUS_RBCOK		0x00000008L
#define	STATUS_TXOFF		0x00000010L
#define	STATUS_TBIMODE		0x00000020L
#define M__STATUS_SPEED		0xFFFFFF3FL
#define	STATUS_SPEED10		0x00000000L
#define	STATUS_SPEED100		0x00000040L
#define	STATUS_SPEED1000	0x00000080L
#define M__STATUS_ASDV		0xFFFFFCFFL
#define	STATUS_ASDV10		0x00000000L
#define	STATUS_ASDV100		0x00000100L
#define	STATUS_ASDV1000		0x00000200L
#define	STATUS_MTXCKOK		0x00000400L
#define	STATUS_PCI66		0x00000800L
#define	STATUS_BUS64		0x00001000L

/* EECD */

#define EECD_CK				0x00000001L
#define EECD_CS				0x00000002L
#define EECD_DI				0x00000004L
#define	EECD_DO				0x00000008L
#define	M__EECD_FWE			0xFFFFFFCFL
#define	S__EECD_FWE			4
#define	EECD_FWE_DIS		0x00000010L
#define	EECD_FWE_EN			0x00000020L
#define	EECD_REQ			0x00000040L
#define	EECD_FLASH_IN_USE	0x00000040L			/* i210 only */
#define	EECD_GNT			0x00000080L
#define	EECD_PRES			0x00000100L
#define	EECD_SIZE			0x00000200L

/* CTRL_EXT */

#define M__CTRLEXT_GPIEN		0xFFFFFFF0L
#define CTRLEXT_GPI_EN0			0x00000001L
#define CTRLEXT_GPI_EN1			0x00000002L
#define CTRLEXT_GPI_EN2			0x00000004L
#define CTRLEXT_GPI_EN3			0x00000008L
#define M__CTRLEXT_SWDPINSHI	0xFFFFFF0FL
#define CTRLEXT_SWDPINSHI0		0x00000010L
#define CTRLEXT_SWDPINSHI1		0x00000020L
#define CTRLEXT_SWDPINSHI2		0x00000040L
#define CTRLEXT_SWDPINSHI3		0x00000080L
#define M__CTRLEXT_SWDPIOHI		0xFFFFF0FFL
#define CTRLEXT_SWDPIOHI0		0x00000100L
#define CTRLEXT_SWDPIOHI1		0x00000200L
#define CTRLEXT_SWDPIOHI2		0x00000400L
#define CTRLEXT_SWDPIOHI3		0x00000800L
#define CTRLEXT_ASDCHK			0x00001000L
#define CTRLEXT_EERST			0x00002000L
#define CTRLEXT_IPS				0x00004000L
#define CTRLEXT_SPDBYPS			0x00008000L
#define M__CTRLEXT_LINK			0xFF3FFFFFL
#define CTRLEXT_LINK_COPPER		0x00000000L
#define CTRLEXT_LINK_TBI		0x00C00000L


/* MDIC */

#define MDIC_DATA				0x0000FFFFL
#define MDIC_REG_ADD			0x001F0000L
#define MDIC_PHY_ADD			0x03E00000L
#define MDIC_OP_WRITE			0x04000000L
#define MDIC_OP_READ			0x08000000L
#define MDIC_READY				0x10000000L
#define MDIC_IE					0x20000000L
#define MDIC_ERROR				0x40000000L

/* TXCW */

#define M__TXCW_TXCONFIGWORD	0xFFFF0000L
#define TXCW_TXCONFIG			0x40000000L
#define TXCW_ANE				0x80000000L

/* RXCW */

#define M__RXCW_RXCONFIGWORD	0xFFFF0000L
#define RXCW_RXCONFIGNOCARRIER	0x04000000L
#define RXCW_RXCONFIGINVALID	0x08000000L
#define RXCW_RXCONFIGCHANGE		0x10000000L
#define RXCW_RXCONFIG			0x20000000L
#define RXCW_RXSYNCHRONISE		0x40000000L
#define RXCW_ANC				0x80000000L

/* ICR */

#define I8257x_VALID_INTERRUPTS  0x000000DB
#define I350_RESERVED_INTERRUPTS 0x0823822AL

#define ICR_TXDW			0x00000001L
#define ICR_TXQE			0x00000002L
#define ICR_LSC				0x00000004L
#define ICR_RXSEQ			0x00000008L
#define ICR_RXDMT0			0x00000010L
#define ICR_RXO				0x00000040L
#define ICR_RXT0			0x00000080L
#define ICR_RXCFG			0x00000400L
#define M__ICR_GPI			0xFFFF87FFL

/* IMS */

#define IMS_TXDW			0x00000001L
#define IMS_TXQE			0x00000002L
#define IMS_LSC				0x00000004L
#define IMS_RXSEQ			0x00000008L
#define IMS_RXDMT0			0x00000010L
#define IMS_RXO				0x00000040L
#define IMS_RXT0			0x00000080L
#define IMS_RXCFG			0x00000100L
#define M__IMS_GPI			0xFFFF87FFL

/* IMC */

#define IMC_TXDW			0x00000001L
#define IMC_TXQE			0x00000002L
#define IMC_LSC				0x00000004L
#define IMC_RXSEQ			0x00000008L
#define IMC_RXDMT0			0x00000010L
#define IMC_RXO				0x00000040L
#define IMC_RXT0			0x00000080L
#define IMC_RXCFG			0x00000100L
#define M__IMC_GPI			0xFFFF87FFL

/* RCTL */

#define RCTL_RST			0x00000001L
#define RCTL_EN				0x00000002L
#define RCTL_SBP			0x00000004L
#define RCTL_UPE			0x00000008L
#define RCTL_MPE			0x00000010L
#define RCTL_LPE			0x00000020L
#define M__RCTL_LBM			0xFFFFFF3FL
#define RCTL_LBM_NONE		0x00000000L
#define RCTL_LBM_TBI		0x00000040L
#define RCTL_LBM_GMII		0x00000080L
#define RCTL_LBM_XCVR		0x000000C0L
#define M__RCTL_RDMTS		0xFFFFFCFFL
#define RCTL_RDMTS_1_2		0x00000000L
#define RCTL_RDMTS_1_4		0x00000100L
#define RCTL_RDMTS_1_8		0x00000200L
#define M__RCTL_MO			0xFFFFCFFFL
#define RCTL_MO_47_36		0x00000000L
#define RCTL_MO_46_35		0x00001000L
#define RCTL_MO_45_34		0x00002000L
#define RCTL_MO_43_32		0x00003000L
#define RCTL_BAM			0x00008000L
#define M__RCTL_BSIZE		0xFFFCFFFFL
#define RCTL_BSIZE_2048		0x00000000L
#define RCTL_BSIZE_1024		0x00010000L
#define RCTL_BSIZE_512		0x00020000L
#define RCTL_BSIZE_256		0x00030000L
#define RCTL_VFE			0x00040000L
#define RCTL_CFIEN			0x00080000L
#define RCTL_CFI			0x00100000L
#define RCTL_DPF			0x00400000L
#define RCTL_PMCF			0x00800000L
#define RCTL_BSEX			0x02000000L
#define RCTL_SECRC			0x04000000L

/* TCTL */

#define TCTL_RST			0x00000001L
#define TCTL_EN				0x00000002L
#define TCTL_PSP			0x00000008L
#define M__TCTL_CT			0xFFFFF00FL
#define S__TCTL_CT			4
#define M__TCTL_COLD		0xFFC00FFFL
#define S__TCTL_COLD		12
#define TCTL_SWXOFF			0x00400000L
#define TCTL_PBE			0x00800000L
#define TCTL_RTLC			0x01000000L
#define TCTL_NRTU			0x02000000L

/* RAH */

#define RAH_AV				0x80000000L

/* Receive descriptor status */

#define RDESC_STATUS_DD		0x01
#define RDESC_STATUS_EOP	0x02
#define RDESC_STATUS_IXSM	0x04
#define RDESC_STATUS_VP		0x08
#define RDESC_STATUS_TCPCS	0x20
#define RDESC_STATUS_IPCS	0x40
#define RDESC_STATUS_PIF	0x80

/* Receive descriptor errors */

#define RDESC_ERRORS_CE		0x01
#define RDESC_ERRORS_SE		0x02
#define RDESC_ERRORS_SEQ	0x04
#define RDESC_ERRORS_CXE	0x10
#define RDESC_ERRORS_TCPE	0x20
#define RDESC_ERRORS_IPE	0x40
#define RDESC_ERRORS_RXE	0x80

/* Transmit descriptor command */

#define TDESC_COMMAND_EOP	0x01
#define TDESC_COMMAND_IFCS	0x02
#define TDESC_COMMAND_IC	0x04
#define TDESC_COMMAND_RS	0x08
#define TDESC_COMMAND_RPS	0x10
#define TDESC_COMMAND_DEXT	0x20
#define TDESC_COMMAND_VLE	0x40
#define TDESC_COMMAND_IDE	0x80

/* Transmit descriptor status */

#define TDESC_STATUS_DD		0x01
#define TDESC_STATUS_EC		0x02
#define TDESC_STATUS_LC		0x04
#define TDESC_STATUS_TU		0x08

/* EEPROM Contents */

#define EEPROM_IA2_1		0x00
#define EEPROM_IA4_3		0x01
#define EEPROM_IA6_5		0x02
#define EEPROM_COMPAT		0x03
#define EEPROM_PBA1_2		0x08
#define EEPROM_PBA3_4		0x09
#define EEPROM_ICW1			0x0A
#define EEPROM_SUBID		0x0B
#define EEPROM_SUBVID		0x0C
#define EEPROM_DID			0x0D
#define EEPROM_VID			0x0E
#define EEPROM_ICW2			0x0F
#define EEPROM_OEM			0x10
#define EEPROM_CPOWER		0x12	/* 82546 */
#define EEPROM_ICW3B		0x14	/* 82546 */
#define EEPROM_SDPC			0x20	/* 82544 */
#define EEPROM_CCTRL		0x21	/* 82544 */
#define EEPROM_DPOWER		0x22	/* 82544 */
#define EEPROM_ICW3A		0x24	/* 82546 */
#define EEPROM_LEDCTL		0x2F	/* 82546 */
#define EEPROM_PXE0			0x30	/* 82544 */
#define EEPROM_PXE1			0x31	/* 82544 */
#define EEPROM_PXE2			0x32	/* 82544 */
#define EEPROM_CSUM			0x3F

#define EEPROM_SIZE			(EEPROM_CSUM + 1)

/* INVM data structure ids and types */
#define INVM_DATA_REG(_n)      	(0x12120 + 4*(_n))
#define	INVM_SIZE					64
#define INVM_ULT_BYTES_SIZE         8
#define INVM_RECORD_SIZE_IN_BYTES   4
#define INVM_VER_FIELD_ONE      	0x1FF8
#define INVM_VER_FIELD_TWO      	0x7FE000
#define INVM_IMGTYPE_FIELD      	0x1F800000
#define INVM_MAJOR_MASK   			0x3F0
#define INVM_MINOR_MASK   			0xF
#define INVM_MAJOR_SHIFT  			4

#define INVM_STRUCT_TYPE_MASK	0x0007
#define	INVM_NVM_END			0x0000
#define	INVM_WORD_AUTOLOAD		0x0001
#define	INVM_CSR_AUTOLOAD		0x0002
#define	INVM_PHY_AUTOLOAD		0x0003
#define	INVM_INVALIDATED		0x0007

#define INVM_RESET_TYPE_MASK	0x0018	
#define INVM_LOAD_POWER_UP		0x0000	/* only reset on power-up */
#define INVM_LOAD_PCIE_RESET	0x0008	/* and on power-up reset */
#define INVM_LOAD_SW_RESET		0x0018	/* and on pcie reset and power-up reset */

#define INVM_WORD_ADDR_MASK		0xFE00
#define INVM_CSR_ADDR_MASK		0x7FFF
#define INVM_PHY_ADDR_MASK		0xF800

/* SPI EEPROM Status Register */
#define EEPROM_STATUS_RDY_SPI	0x01
#define EEPROM_STATUS_WEN_SPI	0x02
#define EEPROM_STATUS_BP0_SPI	0x04
#define EEPROM_STATUS_BP1_SPI	0x08
#define EEPROM_STATUS_WPEN_SPI	0x80


/* EEPROM Commands - SPI */
#define EEPROM_READ_OPCODE_SPI		0x03  /* EEPROM read opcode */
#define EEPROM_WRITE_OPCODE_SPI		0x02  /* EEPROM write opcode */
#define EEPROM_A8_OPCODE_SPI		0x08  /* opcode bit-3 = address bit-8 */
#define EEPROM_WREN_OPCODE_SPI		0x06  /* EEPROM set Write Enable latch */
#define EEPROM_WRDI_OPCODE_SPI		0x04  /* EEPROM reset Write Enable latch */
#define EEPROM_RDSR_OPCODE_SPI		0x05  /* EEPROM read Status register */
#define EEPROM_WRSR_OPCODE_SPI		0x01  /* EEPROM write Status register */
#define EEPROM_ERASE4K_OPCODE_SPI	0x20  /* EEPROM ERASE 4KB */
#define EEPROM_ERASE64K_OPCODE_SPI	0xD8  /* EEPROM ERASE 64KB */
#define EEPROM_ERASE256_OPCODE_SPI	0xDB  /* EEPROM ERASE 256B */


/* EEPROM Initialization Control 1 */

#define ICW1_SIG			0x4000
#define ICW1_MB1			0x2000
#define ICW1_PRESET			(ICW1_SIG | ICW1_MB1)

#define ICW1_IPS			0x1000
#define ICW1_FRCSPD			0x0800
#define ICW1_FD				0x0400
#define ICW1_LRST			0x0200
#define M__ICW1_SWDPIO		0xFE1F
#define S__ICW1_SWDPIO		5
#define ICW1_ILOS			0x0010
#define M__ICW1_PM			0xFFF3
#define ICW1_LOAD_SUBSYS	0x0002
#define ICW1_LOAD_VIDDID	0x0001

/* EEPROM Initialization Control 2 */

#define M__ICW2_PAUSE		0xCFFF
#define S__ICW2_PAUSE		12
#define ICW2_ANE			0x0000
#define M__ICW2_FLASHSIZE	0xF9FF
#define S__ICW2_FLASHSIZE   9
#define ICW2_FLASHSIZE_512K	0x0000
#define ICW2_FLASHSIZE_1M	0x0200
#define ICW2_FLASHSIZE_2M	0x0400
#define ICW2_FLASHSIZE_4M	0x0600
#define ICW2_FLASHDIS		0x0100
#define M__ICW2_SWDPIO		0xFF0F
#define S__ICW2_SWDPIO		0

/* EEPROM Checksum value */

#define EEPROM_CHECKSUM		(0xBABA)

#define EEPROM_W_OP_SIZE		(5)		/* for write enable/disable */
#define EEPROM_W_INDEX_SIZE		(4)
#define EEPROM_OPCODE_SIZE		(3)		/* for read/write */
#define EEPROM_INDEX_SIZE		(6)
#define EEPROM_DATA_SIZE		(16)	/* 16-bit EEPROM data */

#define EEPROM_READ_OPCODE		(0x6)
#define EEPROM_WRITE_OPCODE		(0x5)
#define EEPROM_ERASE_OPCODE		(0x7)
#define EEPROM_EWEN_OPCODE		(0x13)
#define EEPROM_EWDS_OPCODE		(0x10)

#define EEPROM_TIMEOUT			(2000)	/* count * 5uS, timeout for command */


#define _		0x00000000
#define M___		0xFFFFFFFF

/* PHY registers */



#define PHY_CTRL					0x00
#define PHY_CTRL_RESET				0x8000U
#define PHY_CTRL_LOOPBACK			0x4000U
#define M__PHY_CTRL_SPEED			0x2040U /* uses bits 13 (LSB) and 6 (MSB) */
#define PHY_CTRL_SPEED_1000			0x0040U
#define PHY_CTRL_SPEED_100			0x2000U
#define PHY_CTRL_SPEED_10			0x0000U
#define PHY_CTRL_AUTO_NEG			0x1000U
#define PHY_CTRL_POWER_DOWN			0x0800U
#define PHY_CTRL_ISOLATE			0x0400U
#define PHY_CTRL_RESTART_AUTO		0x0200U
#define PHY_CTRL_DUPLEX				0x0100U
#define PHY_CTRL_COLLISION_TEST		0x0080U

#define PHY_STATUS				0x01
#define PHY_PID					0x02	/* PHY Identifier */

#define PHY_ADVERT				0x04
#define PHY_ADVERT_10TXHD		0x0020
#define PHY_ADVERT_10TXFD		0x0040
#define PHY_ADVERT_100TXHD		0x0080
#define PHY_ADVERT_100TXFD		0x0100

#define PHY_1000				0x09
#define PHY_1000_1000TXHD		0x0100
#define PHY_1000_1000TXFD		0x0200

#define ALL_HALF_DUPLEX   (PHY_ADVERT_10TXHD |  PHY_ADVERT_100TXHD)
/* 1000/H is not supported, nor spec-compliant. */
#define ALL_SPEED_DUPLEX  (PHY_ADVERT_10TXHD |   PHY_ADVERT_10TXFD | \
                                PHY_ADVERT_100TXHD |  PHY_ADVERT_100TXFD | \
                                                     PHY_1000_1000TXFD)
#define ALL_NOT_GIG       (PHY_ADVERT_10TXHD |   PHY_ADVERT_10TXFD | \
                                PHY_ADVERT_100TXHD |  PHY_ADVERT_100TXFD)
#define ALL_100_SPEED    (PHY_ADVERT_100TXHD |  PHY_ADVERT_100TXFD)
#define ALL_10_SPEED      (PHY_ADVERT_10TXHD |   PHY_ADVERT_10TXFD)
#define ALL_FULL_DUPLEX   (PHY_ADVERT_10TXFD |  PHY_ADVERT_100TXFD | \
                                                     PHY_1000_1000TXFD)




#define	PCI_ROM_BASE			0x30	/* Expansion ROM base address	*/

#define IF_COPPER				0		/* media interface type */
#define IF_FIBER				1
#define IF_SERDES				2

#define CTRLT_UNKNOWN			0
#define CTRLT_82543GC			1			/* Controller type 82543GC (copper+fibre) */
#define CTRLT_82544EI			2			/* Controller type 82544EI (copper) */
#define CTRLT_82546EB			3			/* Controller type 82546EB (copper) */
#define CTRLT_82540EM			4			/* Controller type 82540EM */
#define CTRLT_82541PI			5			/* Controller type 82541PI */
#define CTRLT_82571EB			6			/* Controller type 82571EB */
#define CTRLT_82573L			7			/* Controller type 82573L */
#define CTRLT_82575EB			8
#define CTRLT_82567				9
#define CTRLT_82574				10
#define CTRLT_82576EB			11
#define CTRLT_82567LM			12
#define CTRLT_82576LM			13			/* Controller type 82576EB */
#define CTRLT_82577		        14			/* Controller type 82577 */
#define CTRLT_82574L            15
#define CTRLT_82599             16
#define CTRLT_82580             17
#define CTRLT_82580EB           18
#define CTRLT_82579             19
#define CTRLT_I350              20
#define CTRLT_I210              21

#define LINE_BUFF_SIZE          80

#ifdef INCLUDE_I82599
#define CTRLT_82599				12
#endif

#define PCI_SCANNING			0
#define PCI_SUCCESS				1
#define DEVICE_NOT_FOUND		0xFFFF

#define SPEED_AUTO				0
#define SPEED_10				1		/* interface speed Mbits/Second */
#define SPEED_100				2
#define SPEED_1000				3


#define LB_NONE					0		/* signal loopback modes */
#define LB_INTERNAL				1
#define LB_LOCAL				2
#define LB_EXTERNAL				3

#define MAX_PHY_REG_ADDRESS		31

#define S__WTHRESH				16
#define S__GRAN					24

#define GG82563_PAGE_SHIFT        5
#define GG82563_REG(page, reg)    \
        (((page) << GG82563_PAGE_SHIFT) | ((reg) & MAX_PHY_REG_ADDRESS))

	/* Ignore CRC32 in message size computation, assume chip automatically
	 * adds the CRC. CNDM 
	 */
#define PHY_REG_MASK                      0x1F
#define PHY_PAGE_SHIFT 5
#define PHY_REG(page, reg) (((page) << PHY_PAGE_SHIFT)|((reg) & MAX_PHY_REG_ADDRESS))

#define ETH_TOP_OH				14		/* src addr + dest addr + length */
#define ETH_TAIL_OH				4		/* crc32 */
#define ETH_OH					(ETH_TOP_OH /*+ ETH_TAIL_OH*/)
	
#define TEST_DATA_SIZE			64
	
#define LOOP_BACK_SIZE			(3072UL)
#define TRAFFIC_PACKETS			32
#define LOOPBACK_PACKETS		10
	
#define MAX_CONTROLLERS			2				/* how many controllers we may use */

#define EN_TX					1		/* enable transmitter */
#define EN_RX					2		/* enable receiver */
#define EN_RXTX					(EN_TX | EN_RX)

/* 82577 Mobile Phy Status Register */
#define HV_M_STATUS                       26
#define HV_M_STATUS_AUTONEG_COMPLETE      0x1000
#define HV_M_STATUS_SPEED_MASK            0x0300
#define HV_M_STATUS_SPEED_1000            0x0200
#define HV_M_STATUS_LINK_UP               0x0040
#define CTRL_EXT_SPD_BYPS            0x00008000 /* Speed Select Bypass */



/* Strapping Option Register - RO */
#define STRAP                     0x0000C
#define E1000_STRAP_SMBUS_ADDRESS_MASK  0x00FE0000
#define E1000_STRAP_SMBUS_ADDRESS_SHIFT 17

#define BM_PORT_CTRL_PAGE                 769
#define FEXTNVM  0x00028L  /* Future Extended NVM - RW */
#define LEDCTL   0x00E00L  /* LED Control - RW */
#define E1000_FEXTNVM_SW_CONFIG        1
#define E1000_FEXTNVM_SW_CONFIG_ICH8M (1 << 27) /* Bit redefined for ICH8M */

/* Extended Configuration Control and Size */
#define E1000_EXTCNF_CTRL_MDIO_SW_OWNERSHIP         0x00000020
#define E1000_EXTCNF_CTRL_LCD_WRITE_ENABLE             0x00000001
#define E1000_EXTCNF_CTRL_OEM_WRITE_ENABLE           0x00000008
#define E1000_EXTCNF_CTRL_SWFLAG                               0x00000020
#define E1000_EXTCNF_CTRL_GATE_PHY_CFG                   0x00000080
#define E1000_EXTCNF_SIZE_EXT_PCIE_LENGTH_MASK     0x00FF0000
#define E1000_EXTCNF_SIZE_EXT_PCIE_LENGTH_SHIFT          16
#define E1000_EXTCNF_CTRL_EXT_CNF_POINTER_MASK   0x0FFF0000
#define E1000_EXTCNF_CTRL_EXT_CNF_POINTER_SHIFT          16
#define EXTCNF_CTRL  0x00F00  /* Extended Configuration Control */
#define EXTCNF_SIZE  0x00F08  /* Extended Configuration Size */
#define HV_LED_CONFIG           PHY_REG(768, 30) /* LED Configuration */


/* SMBus Address Phy Register */
#define HV_SMB_ADDR            PHY_REG(768, 26)
#define HV_SMB_ADDR_PEC_EN     0x0200
#define HV_SMB_ADDR_VALID      0x0080


typedef struct tagRdesc
{
	UINT32	dAddrL;		/* 64-bit buffer address */
	UINT32	dAddrH;
	UINT16	wLength;
	UINT16	wPacketCsum;
	UINT8	bStatus;
	UINT8	bErrors;
	UINT16	wSpecial;
} RDESC;

typedef struct tagTdesc		/* Legacy Mode */
{
	UINT32	dAddrL;		/* 64-bit buffer address */
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
	UINT8	bBus;			/* PCI address of device */
	UINT8	bDev;
	UINT8	bFunc;
	UINT8	bType;			/* interface type (IF_COPPER, IF_FIBER) */	
	UINT8	bMultiFunc;		/* non-zero indicates multi-function device */
	
	UINT8	bVector;

	UINT8	bController;	/* controller type (82543, 82544...) */
	char	achCtrlName [80];	/* controller name */

	UINT8	bPhyAddress;	
	UINT8	bMaxSpeed;		/* maximum speed on external connector (def=1000) */
	UINT8	bIndex;			/* controller number */

	/* Use 8-bit pointers as this simplifies register indexing. These can
	 * then be cast to whatever type is required.
	 */

	volatile UINT8*	pbReg;			/* internal registers */
	volatile UINT8*	pbRomRead;		/* BIOS ROM, read accesses */
	volatile UINT8*	pbRomWrite;		/* BIOS ROM, write accesses */
	volatile UINT8*	pbFlashBase;     /* ICH9 NVM Flash Base Address */
	volatile UINT32	dGbeFlashBase;  /* Gige Flash Base Address from Flashbase */

	UINT32	dTxDescAddr;	/* base address of TX descriptor ring */
	UINT32	dTxDescSize;	/* length of TX descriptor ring */
	volatile TDESC*	psTxDesc;		/* pointer to TX descriptor ring */

	UINT32	dTxBuffAddr;	/* base address of TX buffers */
	UINT8*	pbTxBuff;		/* pointer to TX buffers */

	UINT32	dRxDescAddr;	/* base address of RX descriptor ring */
	UINT32	dRxDescSize;	/* length of RX descriptor ring */
	volatile RDESC*	psRxDesc;		/* pointer to RX descriptor ring */

	UINT32	dRxBuffAddr;	/* base address of RX buffers */
	UINT8*	pbRxBuff;		/* pointer to RX buffers */
	
	/* TX & RX descriptor indexes */
	
	UINT16	wTxCurrentDescr;	/* start of most recent transmit chain */
	UINT32  	dFlashBankSize;     /* ICH9 Flash Bank Size */
	UINT16  	wFlashWordSize;     /* ICH9 Flash Word Size */

	char	achErrorMsg[200];	/* used for composite messages */
}DEVICE;

void vGetEthMacAddr (UINT8  bInstance, UINT8* pbAddress,UINT8 bMode);
UINT32 dGetNumberOfEthInstances(void);

#define SERDESCTL	0x00024L
#define RXPBS       0x02404L
#define TXPBS       0x03404L

/* SerDes 82576 Ethernet controller */
#define ENABLE_SERDES_LOOPBACK       	0x0410
#define SCTL_DISABLE_SERDES_LOOPBACK 	0x0400
#define CONNSW                     		0x00034L          /* Copper/Fiber switch control - RW */
#define MANC                       		0x05820L /* Management control */
#define CONNSW_ENRGSRC             		0x4
#define PCS_LCTL_FLV_LINK_UP      		1
#define PCS_LCTL_FSV_100          		2
#define PCS_LCTL_FSV_1000          		4
#define PCS_LCTL_FDV_FULL          		8
#define PCS_LCTL_FSD               		0x10
#define PCS_LCTL_FORCE_LINK        		0x20
#define PCS_LCTL_FORCE_FCTRL      		0x80
#define PCS_LCTL_AN_ENABLE         		0x10000
#define PCS_LCTL_AN_RESTART        		0x20000
#define PCS_LCTL_AN_TIMEOUT        		0x40000
#define PCS_LCTL                   		0x04208L  /* PCS Link Control - RW */
#define PCS_CFG                    		0x04200L
#define PCS_STATUS                 		0x0420CL
#define PCS_LSTAT                       0x0420CL
#define PCS_LSTS_SYNK_OK                0x10
#define PCS_LINK_OK                     0x01


/* ICH9 NVM  Flash Registers */
#define ICH_FLASH_GFPREG                0x0000
#define ICH_FLASH_HSFSTS                0x0004
#define ICH_FLASH_HSFCTL                0x0006
#define ICH_FLASH_FADDR                 0x0008
#define ICH_FLASH_FDATA0                0x0010
#define ICH_FLASH_PR0                   0x0074


/* ICH9 Flash Hardware Sequencing Registers */
/*HWS Flash Status Register masks */
#define HSFS_FLOCKDN  0x8000
#define HSFS_FDV      0x4000
#define HSFS_FDOPSS   0x2000
#define HSFS_SCIP     0x0020
#define HSFS_BERASE   0x0018
#define HSFS_AEL      0x0004
#define HSFS_FCERR    0x0002
#define HSFS_FDONE    0x0001


/*HWS Flash Control Register */
#define HSFC_FDBC   0x0300
#define HSFC_FCYCLE 0x0006
#define HSFC_FGO    0x0001

#define FLASH_GFPREG_BASE_MASK  0x1FFF
#define FLASH_SECTOR_ADDR_SHIFT     12
#define ICH_FLASH_LINEAR_ADDR_MASK 0x00FFFFFF
#define ICH9_SHADOW_RAM_WORDS            2048
#define ICH_CYCLE_READ                     0
#define ICH_CYCLE_WRITE                    2
#define ICH_FLASH_READ_COMMAND_TIMEOUT     10000000
#define ICH_FLASH_CYCLE_REPEAT_COUNT       10


#define ICH9_PHY_SPEC_CTRL         0x10
#define ICH9_PSCR_ENABLE_DOWNSHIFT 0x0800
#define EECD_SEC1VAL   0x00400000 /* Sector One Valid */

#define PHY_ID1               0x02 /* Phy Id Reg (word 1) */
#define PHY_ID2               0x03 /* Phy Id Reg (word 2) */
#define PHY_REVISION_MASK     0xFFFFFFF0

/* PHY Page Registers for 82567LM */
#define BM_WUC_PAGE                     800
#define IGP_PAGE_SHIFT                  5
#define MAX_PHY_MULTI_PAGE_REG          0xF
#define IGP01E1000_PHY_PAGE_SELECT      0x1F /* Page Select */
#define BM_PHY_PAGE_SELECT              22   /* Page Select for BM */
/*#define MAX_PHY_REG_ADDRESS             0x1F*/  /* 5 bit address bus (0-0x1F) */
#define ICH9_EXTCNF_CTRL            0x00F00L
#define ICH9_PHY_CTRL               0x00F10L
#define PHY_CTRL_D0A_LPLU           0x00000002
#define PHY_CTRL_SPD_EN             0x00000001
#define PHY_CTRL_NOND0A_LPLU        0x00000004
#define PHY_CTRL_NOND0A_GBE_DISABLE 0x00000008
#define PHY_CTRL_GBE_DISABLE        0x00000040


/* BM/HV Specific Registers */
#define BM_PORT_CTRL_PAGE                 769
#define BM_PCIE_PAGE                      770
#define BM_WUC_ADDRESS_OPCODE             0x11
#define BM_WUC_DATA_OPCODE                0x12
#define BM_WUC_ENABLE_PAGE                BM_PORT_CTRL_PAGE
#define BM_WUC_ENABLE_REG                 17
#define BM_WUC_ENABLE_BIT                 (1 << 2)
#define BM_WUC_HOST_WU_BIT                (1 << 4)
#define BM_WUC          				PHY_REG(BM_WUC_PAGE, 1)

/* OEM Bits Phy Register */
#define HV_OEM_BITS            PHY_REG(768, 25)
#define HV_OEM_BITS_LPLU       0x0004 /* Low Power Link Up */
#define HV_OEM_BITS_GBE_DIS    0x0040 /* Gigabit Disable */
#define HV_OEM_BITS_RESTART_AN 0x0400 /* Restart Auto-negotiation */
#define E1000_PHY_CTRL_SPD_EN             0x00000001
#define E1000_PHY_CTRL_D0A_LPLU           0x00000002
#define E1000_PHY_CTRL_NOND0A_LPLU        0x00000004
#define E1000_PHY_CTRL_NOND0A_GBE_DISABLE 0x00000008
#define E1000_PHY_CTRL_GBE_DISABLE        0x00000040

#define FWSM      0x05B54 /* FW Semaphore */
#define E1000_ICH_FWSM_RSPCIPHY          0x00000040 /* Reset PHY on PCI Reset */
#define E1000_ICH_FWSM_DISSW             0x10000000 /* FW Disables SW Writes */
#define E1000_BLK_PHY_RESET   12

#define PHY_UPPER_SHIFT                   21
#define PHY_PAGE_SHIFT 					   5

#define HV_INTC_FC_PAGE_START             768
#define I82578_ADDR_REG                   29
#define I82577_ADDR_REG                   16
#define I82577_CFG_REG                    22
#define I82577_CFG_ASSERT_CRS_ON_TX       (1 << 15)
#define I82577_CFG_ENABLE_DOWNSHIFT       (3 << 10) /* auto downshift 100/10 */
#define I82577_CTRL_REG                   23

#define E1000_KMRNCTRLSTA_OFFSET          0x001F0000
#define E1000_KMRNCTRLSTA_OFFSET_SHIFT    16
#define E1000_KMRNCTRLSTA_REN             0x00200000
#define E1000_KMRNCTRLSTA_CTRL_OFFSET     0x1    /* Kumeran Control */
#define E1000_KMRNCTRLSTA_DIAG_OFFSET     0x3    /* Kumeran Diagnostic */
#define E1000_KMRNCTRLSTA_TIMEOUTS        0x4    /* Kumeran Timeouts */
#define E1000_KMRNCTRLSTA_INBAND_PARAM    0x9    /* Kumeran InBand Parameters */
#define E1000_KMRNCTRLSTA_DIAG_NELPBK     0x1000 /* Nearend Loopback mode */
#define E1000_KMRNCTRLSTA_K1_CONFIG        0x7
#define E1000_KMRNCTRLSTA_K1_ENABLE        0x0002
#define E1000_KMRNCTRLSTA_HD_CTRL         0x10   /* Kumeran HD Control */
#define KMRNCTRLSTA 					  0x00034

#define E1000_TCTL_COLD   0x003ff000    /* collision distance */
/* Collision related configuration parameters */
#define E1000_COLLISION_THRESHOLD       15
#define E1000_CT_SHIFT                  4
#define E1000_COLLISION_DISTANCE        63
#define E1000_COLD_SHIFT                12

#define MII_SR_LINK_STATUS       0x0004 /* Link Status 1 = link */
#define COPPER_LINK_UP_LIMIT              10
#define E1000_STATUS_LAN_INIT_DONE 0x00000200  /* Lan Init Completion by NVM */
#define E1000_ICH8_LAN_INIT_TIMEOUT      1500

#define E1000_STATUS_PHYRA      0x00000400      /* PHY Reset Asserted */




#define ERT_2048                0x100

#define TARC(_n)   (0x03840L + (_n << 8))
#define SPEED_MODE_BIT (1 << 21)
#define KABGTXD     0x03004L    /* only in ICH9 */


#define AUTO_READ_DONE_TIMEOUT      10     /*ICH9*/
#define EECD_AUTO_RD          0x00000200L  /* NVM Auto Read done */

#define TRUE   1
#define FALSE  0
