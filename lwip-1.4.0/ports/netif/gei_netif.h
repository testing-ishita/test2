/*******************************************************************************
*
* Filename:	 gei_netif.h
*
* Description:	lwIP Ethernet driver for Intel Gigabit devices header file.
*
* $Revision: 1.10 $
*
* $Date: 2015-03-31 11:44:53 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/ports/netif/gei_netif.h,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#ifndef _GEIF_H
#define _GEIF_H


/*******************************************************************************
*
* General Definitions
*
*******************************************************************************/

#define	GEIF_MAX_NUM	4	/* maximum number of devices */

/* Device ID's */

#define INTEL_DEVICEID_82573L	0x109A
#define INTEL_DEVICEID_82576EB	0x10C9
#define INTEL_DEVICEID_82579LM2	0x1502

#define INTEL_DEVICEID_82580_QUAD_COPPER	0x150E
#define INTEL_DEVICEID_82580_DUAL_COPPER	0x1516
#define INTEL_DEVICEID_82580EB_SERDES		0x1510
#define INTEL_DEVICEID_i350AMx_COPPER		0x1521
#define INTEL_DEVICEID_i350AMx_SERDES		0x1523
#define INTEL_DEVICEID_i210					0x1533
#define INTEL_DEVICEID_i210_SERDES			0x1537

/* Device Types */

#define GEIF_DEVTYPE_PCI		1	/* 8254x PCI devices */
#define GEIF_DEVTYPE_ADVANCED	2	/* 82575/82576/82580 advanced devices */
#define GEIF_DEVTYPE_ICH		3	/* ICH integrated devices */

/* Link Types */

#define GEIF_LINKTYPE_COPPER	1
#define GEIF_LINKTYPE_SERDES	2

/* General Flags */

#define GEIF_FLAG_POLLING_MODE		0x0001
#define GEIF_FLAG_PROMISC_MODE		0x0002
#define GEIF_FLAG_ALLMULTI_MODE		0x0004
#define GEIF_FLAG_MULTICAST_MODE	0x0008
#define GEIF_FLAG_BROADCAST_MODE	0x0010

/* Packet Status */

#define GEIF_PKT_NONE			1
#define GEIF_PKT_OK				0
#define GEIF_PKT_INVALID		-1
#define GEIF_PKT_INVALID_LEN	-2
#define GEIF_PKT_MEM_ERROR		-3
#define GEIF_PKT_RX_ERROR		-4
#define GEIF_PKT_TX_ERROR		-5
#define GEIF_PKT_TX_TIMEOUT		-6
#define GEIF_PKT_DESC_ERROR		-7
#define GEIF_PKT_TCPIP_ERROR	-8

/* Ethernet Packet Definitions */

#define ETHER_MIN_LEN   60
#define	ETHER_ADDR_LEN	6		/* number of bytes in an ethernet (MAC) address */
#define	ETHER_TYPE_LEN	2		/* number of bytes in the type field */
#define	ETHER_CRC_LEN	4		/* number of bytes in the trailing CRC field */
#define	ETHER_MAX_LEN	1518	/* maximum packet length */
#define	ETHER_HDR_LEN	(ETHER_ADDR_LEN*2+ETHER_TYPE_LEN)
#define	ETHERMTU		(ETHER_MAX_LEN-ETHER_HDR_LEN-ETHER_CRC_LEN)

/* Miscellaneous Definitions */

#define GEIF_EEPROM_MAC_OFFSET	0x0	/* MAC address offset */

#ifndef ROUND_UP
#define ROUND_UP(x, align)	((((UINT32)(x)) + (align - 1)) & ~(align - 1)) 
#endif


/*******************************************************************************
*
* Device Register Definitions
*
*******************************************************************************/

#define GEIF_CTRL				0x0000			/* Control Register */
#define GEIF_CTRL_FD_BIT		0x00000001
#define GEIF_CTRL_PRIOR_BIT		0x00000004
#define GEIF_CTRL_ASDE_BIT		0x00000020
#define GEIF_CTRL_SLU_BIT		0x00000040
#define GEIF_CTRL_ILOS_BIT		0x00000080
#define GEIF_CTRL_SPD_10_BIT	0x00000000
#define GEIF_CTRL_SPD_100_BIT	0x00000100
#define GEIF_CTRL_SPD_1000_BIT 	0x00000200
#define GEIF_CTRL_SPEED_BITS	0x00000300
#define GEIF_CTRL_FRCSPD_BIT	0x00000800
#define GEIF_CTRL_FRCDPX_BIT	0x00001000
#define GEIF_CTRL_SWDPIN0_BIT	0x00040000
#define GEIF_CTRL_SWDPIN1_BIT	0x00080000
#define GEIF_CTRL_SWDPIN2_BIT	0x00100000
#define GEIF_CTRL_SWDPIN3_BIT	0x00200000
#define GEIF_CTRL_SWDPIO0_BIT	0x00400000
#define GEIF_CTRL_SWDPIO1_BIT	0x00800000
#define GEIF_CTRL_SWDPIO2_BIT	0x01000000
#define GEIF_CTRL_SWDPIO3_BIT	0x02000000
#define GEIF_CTRL_RST_BIT		0x04000000
#define GEIF_CTRL_RFCE_BIT		0x08000000
#define GEIF_CTRL_TFCE_BIT		0x10000000
#define GEIF_CTRL_PHY_RST_BIT	0x80000000
#define GEIF_CTRL_MDC_BIT		GEIF_CTRL_SWDPIN3_BIT
#define GEIF_CTRL_MDIO_BIT		GEIF_CTRL_SWDPIN2_BIT
#define GEIF_CTRL_MDC_DIR_BIT	GEIF_CTRL_SWDPIO3_BIT
#define GEIF_CTRL_MDIO_DIR_BIT	GEIF_CTRL_SWDPIO2_BIT
#define GEIF_CTRL_SWDPIOLO_SHIFT		22
#define GEIF_CTRL_ILOS_SHIFT 			7

#define GEIF_STATUS					0x0008		/* Status Register */
#define GEIF_STATUS_FD_BIT			0x1
#define GEIF_STATUS_LU_BIT			0x2
#define GEIF_STATUS_FID				0xc 		/* Function ID */
#define GEIF_STATUS_TBIMODE_BIT		0x20
#define GEIF_STATUS_SPEED_100_BIT	0x40
#define GEIF_STATUS_SPEED_1000_BIT	0x80

#define GEIF_FID(x)	(((x) & GEIF_STATUS_FID) >> 2)

#define GEIF_EEC				0x0010			/* Flash Control Register */
#define GEIF_EEC_NV_PRES		0x00000100		/* NVM present */
#define GEIF_EEC_NV_AUTO_RD		0x00000200		/* NVM auto read done */
#define GEIF_EEC_NV_SEC1VAL		0x00400000		/* NVM bank select */

#define GEIF_CTRLEXT			0x0018			/* Extended Control Register */
#define GEIF_CTRLEXT_SDP7_DATA 	0x00000080 		/* Value of SW Definable Pin 7 */
#define GEIF_CTRLEXT_LM_SERDES	0x00C00000		/* Link Mode */

#define GEIF_MDIC				0x0020			/* MDI Control Register */
#define GEIF_MDIC_WR_BIT		0x4000000
#define GEIF_MDIC_RD_BIT		0x8000000
#define GEIF_MDIC_RDY_BIT		0x10000000
#define GEIF_MDIC_ERR_BIT		0x40000000
#define GEIF_MDIC_REG_SHIFT		16
#define GEIF_MDIC_PHY_SHIFT		21

#define GEIF_FEXTNVM4 		0x0024  			/* Future Extended NVM 4 */
#define GEIF_FEXTNVM4_DISABLE_SERDES_LOOPBACK 0x0400

#define GEIF_FEXTNVM  		0x0028  			/* Future Extended NVM Register */
#define GEIF_FEXTNVM_SW_CONFIG_ICH	(1 << 27) 	/* NVM SW Config bit */

#define GEIF_CONNSW  		0x0034  			/* Copper/Fiber switch control */
#define GEIF_CONNSW_ENRGSRC 0x4

#define GEIF_VET			0x0038				/* VLAN Ethertype Register */

#define GEIF_ICR			0x00C0				/* Interrupt Cause Read Register */
#define GEIF_ICR_TXDW		0x00000001 			/* TX descriptor writeback */
#define GEIF_ICR_TXQE		0x00000002 			/* TX queue empty */
#define GEIF_ICR_LSC		0x00000004 			/* Link status change */
#define GEIF_ICR_RXSEQ		0x00000008 			/* RX sequence error */
#define GEIF_ICR_RXDMT0		0x00000010 			/* RX desc minimum threshold */
#define GEIF_ICR_RXO		0x00000040 			/* RX overrun */
#define GEIF_ICR_RXT0		0x00000080 			/* RX timer expired */
#define GEIF_ICR_MDAC		0x00000200 			/* MDI access complete */
#define GEIF_ICR_RXCFG		0x00000400 			/* Received /C/ ordered sets */
#define GEIF_ICR_GPI_SDP1	0x00000800 			/* general purpose int, 82544 */
#define GEIF_ICR_PHYINT		0x00001000 			/* PHY interrupt pin asserted */
#define GEIF_ICR_SPI_SDP6	0x00002000 			/* general purpose int, pin 6[2] */
#define GEIF_ICR_SPI_SDP7	0x00004000 			/* general purpose int, pin 7[3] */
#define GEIF_ICR_TXD_LOW	0x00008000 			/* TX desc low threshold (!82544) */
#define GEIF_ICR_SRPD		0x00010000 			/* RX small packet */
#define GEIF_ICR_OMED		0x00100000 			/* Other link detect */
#define GEIF_RXINTRS		(GEIF_ICR_RXO|GEIF_ICR_RXT0)
#define GEIF_TXINTRS		GEIF_ICR_TXDW
#define GEIF_LINKINTRS		GEIF_ICR_LSC
#define GEIF_INTRS			(GEIF_RXINTRS|GEIF_TXINTRS|GEIF_LINKINTRS)

#define GEIF_ITR			0x00C4				/* Interrupt throttling */

#define GEIF_IMS			0x00D0				/* Interrupt Mask Set Register */
#define GEIF_IMS_TXDW_BIT	0x01
#define GEIF_IMS_TXQE_BIT	0x02
#define GEIF_IMS_LSC_BIT	0x04
#define GEIF_IMS_RXSEQ_BIT	0x08
#define GEIF_IMS_RXDMT0_BIT	0x10
#define GEIF_IMS_RXO_BIT	0x40
#define GEIF_IMS_RXTO_BIT	0x80
#define GEIF_IMS_MDAC_BIT	0x200
#define GEIF_IMS_RXCFG_BIT	0x400
#define GEIF_IMS_TXDLOW_BIT	0x8000

#define GEIF_IMC			0x00D8				/* Interrupt Mask Clear Register */
#define GEIF_IMC_ALL_BITS	0xffffffff
#define GEIF_IMC_TXDW_BIT	0x01
#define GEIF_IMC_TXQE_BIT	0x02
#define GEIF_IMC_LSC_BIT	0x04
#define GEIF_IMC_RXSEQ_BIT	0x08
#define GEIF_IMC_RXDMT0_BIT	0x10
#define GEIF_IMC_RXO_BIT	0x40
#define GEIF_IMC_RXTO_BIT	0x80
#define GEIF_IMC_MDAC_BIT	0x200
#define GEIF_IMC_RXCFG_BIT	0x400
#define GEIF_IMC_TXDLOW_BIT	0x8000

#define GEIF_RCTL				0x100			/* Receive Control Register */
#define GEIF_RCTL_MO_SHIFT		12
#define GEIF_RCTL_BSIZE_2048	0
#define GEIF_RCTL_BSIZE_4096	0x00030000
#define GEIF_RCTL_BSIZE_8192	0x00020000
#define GEIF_RCTL_BSIZE_16384	0x00010000
#define GEIF_RCTL_EN_BIT		0x00000002
#define GEIF_RCTL_SBP_BIT		0x00000004
#define GEIF_RCTL_UPE_BIT		0x00000008
#define GEIF_RCTL_MPE_BIT		0x00000010
#define GEIF_RCTL_LPE_BIT		0x00000020
#define GEIF_RCTL_BAM_BIT		0x00008000
#define GEIF_RCTL_BSEX_BIT		0x02000000
#define GEIF_RCTL_SECRC_BIT		0x04000000

#define GEIF_FCTTV				0x170   		/* Flow Control TX Timer Value Register */
#define GEIF_TXCW				0x178			/* TX Configuration Word Register */
#define GEIF_RXCW				0x180			/* RX Configuration Word Register */

#define GEIF_TCTL				0x400			/* TX control Register */
#define GEIF_TCTL_EN_BIT		0x2
#define GEIF_TCTL_PSP_BIT		0x8
#define GEIF_TCTL_PBE_BIT		0x800000
#define GEIF_TCTL_COLD_BIT		0x3ff000
#define GEIF_TCTL_COLD_SHIFT	12
#define GEIF_TCTL_CT_SHIFT		4
#define GEIF_TCTL_THRESHOLD 	16L

#define GEIF_EXTCNF_CTRL			0x0f00  	/* Extended config control */
#define GEIF_EXTCNF_OEM_WR_ENABLE	0x00000008

#define GEIF_PHY_CTRL 				0x0f10  	/* PHY Control Register */
#define GEIF_PHY_CTRL_GBE_DISABLE	0x00000040
#define GEIF_PHY_CTRL_D0A_LPLU		0x00000002

#define GEIF_TIPG				0x410			/* Transmit IPG Register */
#define GEIF_TIPG_IPGR1_SHIFT	10
#define GEIF_TIPG_IPGR2_SHIFT	20
#define GEIF_TIPG_IPGT_FIBER	9L
#define GEIF_TIPG_IPGT_COPPER	8L
#define GEIF_TIPG_IPGR1			8L
#define GEIF_TIPG_IPGR2			6L

#define GEIF_PBA				0x1000  		/* Packet Buffer Allocation Register */
#define GEIF_FCRTL				0x2160  		/* Flow Control RX Threshold Low Register */
#define GEIF_FCRTH				0x2168  		/* Flow Control RX Threshold High Register */
#define GEIF_RDBAL0				0x2800			/* RX Descriptor Base, Low Register */
#define GEIF_RDBAH0				0x2804			/* RX Descriptor Base, High Register */
#define GEIF_RDLEN0				0x2808			/* RX Descriptor Table Length Register */

#define GEIF_SRRCTL0			0x280C			/* RX Split/Replication Control Register */
#define GEIF_SRRCTL_BSIZEPKT	0x0000007F 		/* Packet size in 1K chunks */
#define GEIF_SRRCTL_BSIZEHDR	0x00000F00 		/* Header size in 64 byte chunks */
#define GEIF_SRRCTL_RDMTS       0x01F00000 		/* RX descriptor minimum threshold size */
#define GEIF_SRRCTL_DESCTYPE	0x0E000000 		/* RX descriptor type */
#define GEIF_SRRCTL_DROP_ENB	0x80000000 		/* Drop when out of descs */

#define GEIF_RDH0				0x2810			/* RX Descriptor Head (consumer idx) Register */
#define GEIF_RDT0				0x2818			/* RX Descriptor Tail (producer idx) Register */
#define GEIF_RDTR				0x2820  		/* Rx Delay Timer Ring Register */
#define GEIF_RDTR_RXINT_DELAY   0
#define GEIF_RDTR_FPD_BIT		0x80000000	

#define GEIF_RXDCTL0			0x2828			/* RX Descriptor Control Register */
#define GEIF_RXDCTL_PTHRESH		0x0000003F 		/* Prefetch threshold  */
#define GEIF_RXDCTL_HTHRESH		0x00003F00 		/* Host threshold */
#define GEIF_RXDCTL_WTHRESH		0x003F0000 		/* Writeback threshold */
#define GEIF_RXDCTL_GRAN		0x01000000 		/* Granularity */
#define GEIF_RXDCTL_ENABLE   	0x02000000 		/* Enable queue (82575+) */
#define GEIF_RXDCTL_SWFLUSH		0x04000000 		/* RX software flush */

#define GEIF_RADV				0x282C  		/* Absolute RX Delay Timer Register */
                               
#define GEIF_TDBAL0				0x3800			/* TX Descriptor Base, Low Register */
#define GEIF_TDBAH0				0x3804			/* TX Descriptor Base, High Register */
#define GEIF_TDLEN0				0x3808			/* TX Descriptor Bable Length Register */
#define GEIF_TDH0				0x3810			/* TX Descriptor Head (consumer idx) Register */
#define GEIF_TDT0				0x3818			/* TX Descriptor Tail (producer idx) Register */

#define GEIF_TIDV				0x3820			/* TX Interrupt Delay Value Register */
#define GEIF_TIDV_DELAY_5        5

#define GEIF_TXDCTL0			0x3828			/* TX descriptor control Register */
#define GEIF_TXDCTL_PTHRESH		0x0000003F 		/* Prefetch threshold  */
#define GEIF_TXDCTL_HTHRESH		0x00003F00 		/* Host threshold */
#define GEIF_TXDCTL_WTHRESH		0x003F0000 		/* Writeback threshold */
#define GEIF_TXDCTL_COUNT_DESC	0x00400000 		/* Count outstanding descs */
#define GEIF_TXDCTL_GRAN		0x01000000 		/* Granularity */
#define GEIF_TXDCTL_LWTHRESH	0xFE000000 		/* Low descriptor thresh */
#define GEIF_TXDCTL_ENABLE      0x02000000 		/* Enable this TX queue */
#define GEIF_TXDCTL_SWFLUSH     0x04000000 		/* Trigger TXD writeback */
#define GEIF_TXDCTL_PRIORITY    0x08000000 		/* High priority Q */

#define GEIF_PCSLCTL 				0x4208 		/* PCS Link Control */
#define GEIF_PCSLCTL_AN_ENABLE  	0x10000
#define GEIF_PCSLCTL_AN_RESTART		0x20000
#define GEIF_PCSLCTL_AN_TIMEOUT_EN	0x40000
#define GEIF_PCSLCTL_FLV_LINK_UP	1
#define GEIF_PCSLCTL_FSD			0x10
#define GEIF_PCSLCTL_FORCE_LINK		0x20
#define GEIF_PCSLCTL_FSV_1000		4
#define GEIF_PCSLCTL_FSV_10		    0
#define GEIF_PCSLCTL_FDV_FULL		8
#define GEIF_PCSLCTL_FSD			0x10
#define GEIF_PCSLCTL_FORCE_LINK		0x20
#define GEIF_PCSLCTL_FORCE_FCTRL	0x80

#define GEIF_PCSLSTS 				0x420c 			/* PCS Link Status */
#define GEIF_PCSLSTS_LINK_UP		0x1
#define GEIF_PCSLSTS_10T_HD			0x1
#define GEIF_PCSLSTS_10T_FD			0x9
#define GEIF_PCSLSTS_100T_HD		0x3
#define GEIF_PCSLSTS_100T_FD		0xb
#define GEIF_PCSLSTS_1000T_HD		0x5
#define GEIF_PCSLSTS_1000T_FD		0xd
#define GEIF_PCSLSTS_AN_COMPLETE	0x10000

#define GEIF_RXCSUM				0x5000			/* RX Checksum Control Register */
#define GEIF_RXCSUM_IPOFL_BIT	0x0100
#define GEIF_RXCSUM_TUOFL_BIT	0x0200
#define GEIF_RXCSUM_IPV6OFL_BIT	0x0400
#define GEIF_RXCSUM_PKT_CSUM_OFF	(14 + 40)	/* The offset at which the packet checksum starts.
										 		 * Specified this way to handle and IPv6 header immediately
 										 		 * following an RFC 894 Ethernet header. */

#define GEIF_MTA0				0x5200			/* Multicast Table Array Start Register */
#define GEIF_RAL0				0x5400			/* RX Address Low Register */
#define GEIF_RAH0				0x5404			/* RX Address High Register */
#define GEIF_RAH_VALID_BIT		0x80000000		/* RX Address valid */
#define GEIF_VFTA0				0x5600			/* VLAN filter table Register */

#define GEIF_NUM_MTA			128
#define GEIF_NUM_RAR			16                             
#define GEIF_NUM_VLAN			128
#define GEIF_MAX_NUM_MULTI		(GEIF_NUM_RAR + GEIF_NUM_MTA - 1)


/* Register access macro's */

#define GEIF_READ_REG( regOff ) \
		(*(UINT32*)(pGeif->baseAddr + regOff))

#define GEIF_WRITE_REG( regOff, data ) \
		(*(UINT32*)(pGeif->baseAddr + regOff) = data)
		
#define GEIF_SETBIT(regOff, val) \
        GEIF_WRITE_REG(regOff, GEIF_READ_REG(regOff) | (val))

#define GEIF_CLRBIT(offset, val) \
        GEIF_WRITE_REG(offset, GEIF_READ_REG(offset) & ~(val))


/*******************************************************************************
*
* Descriptor Definitions
*
*******************************************************************************/

/* Descriptor types */
#define GEIF_DESCTYPE_LEGACY	0x00000000 /* Legacy */
#define GEIF_DESCTYPE_ADV_ONE	0x02000000 /* Advanced, one buffer */
#define GEIF_DESCTYPE_ADV_SP	0x04000000 /* Advanced, header splitting */
#define GEIF_DESCTYPE_ADV_REP	0x06000000 /* Advanced, header replication */
#define GEIF_DESCTYPE_ADV_LRREP	0x08000000 /* Adv, hdr repl, large pkt only */
#define GEIF_DESCTYPE_ADV_SPALW	0x0A000000 /* Adv, hdr split always */

/* The PRO/1000 hardware requires at least 128 descriptors per DMA ring */

#define GEIF_RXD_CNT 128
#define GEIF_TXD_CNT 128

/* Size of the receiver buffers. */
/* This size gives us the benefit that any frame fits into one buffer.  */
/* Note: The value MUST be dividable by 16! */

#define GEIF_RX_BUF_SIZE  2048

/* Size of the transmit buffers. If you set this value to small all frames */
/* greater than this size will be dropped. */
/* Note: The value MUST be dividable by 16! */

#define GEIF_TX_BUF_SIZE  1520

/* RX descriptor structure */

#define RXD_BUFADRLOW_OFFSET         0   /* buf mem low offset */
#define RXD_BUFADRHIGH_OFFSET        4   /* buf mem high offset */
#define RXD_LENGTH_OFFSET            8   /* length offset */
#define RXD_CHKSUM_OFFSET            10  /* cksum offset */
#define RXD_STATUS_OFFSET            12  /* status offset */
#define RXD_ERROR_OFFSET             13  /* error offset */
#define RXD_SPECIAL_OFFSET           14  /* special offset */
#define RXD_SIZE                     16  /* descriptor size */

/* RX Descriptor Error Field */

#define RXD_ERROR_CE				0x01 /* CRC or Align error */
#define RXD_ERROR_SE				0x02 /* Symbol error */
#define RXD_ERROR_SEQ				0x04 /* Sequence error */
#define RXD_ERROR_RSV				0x08 /* Reserved */
#define RXD_ERROR_CXE				0x10 /* Carrier ext error */
#define RXD_ERROR_TCPE				0x20 /* TCP/UDP CKSUM error */
#define RXD_ERROR_IPE				0x40 /* IP CKSUM error */
#define RXD_ERROR_RXE				0x80 /* RX data error */

/* RX Descriptor Status Field */

#define RXD_STAT_DD					0x01 /* Descriptor done */
#define RXD_STAT_EOP				0x02 /* End of packet */
#define RXD_STAT_IXSM				0x04 /* Ignore checksum results */
#define RXD_STAT_VP					0x08 /* Frame is VLAN tagged */
#define RXD_STAT_RSV				0x10 /* Reserved */
#define RXD_STAT_TCPCS 				0x20 /* TCP/UDP checksum calculated */
#define RXD_STAT_IPCS				0x40 /* IP checksum calculated */
#define RXD_STAT_PIF				0x80 /* passed exact filter */
	
/* Legacy Receive Rescriptor */	
typedef struct
{
	volatile UINT32	addrlo;
	volatile UINT32	addrhi;
	volatile UINT16	len;
	volatile UINT16	csum;
	volatile UINT8	sts;
	volatile UINT8	err;
	volatile UINT16	special;
} GEIF_RXD;


/* TX Descriptor Command Fields */

#define TXD_CMD_EOP					0x01	/* End of packet */
#define TXD_CMD_IFCS				0x02	/* Insert FCS */
#define TXD_CMD_TSE					0x04	/* TCP segmentation enable */
#define TXD_CMD_RS					0x08	/* Report status */
#define TXD_CMD_RPS					0x10	/* Report packet sent */
#define TXD_CMD_DEXT				0x20	/* Extension */
#define TXD_CMD_VLE					0x40	/* VLAN enable */
#define TXD_CMD_IDE					0x80	/* Interrupt delay enable */

/* TX Descriptor Status Fields */

#define TXD_STAT_DD					0x01	/* Descriptor done */
#define TXD_STAT_EC					0x02	/* Excess collisions */
#define TXD_STAT_LC					0x04	/* Late collision */
#define TXD_STAT_TU					0x08	/* Transmit underrun */

/* TX descriptors structure */

#define TXD_BUFADRLOW_OFFSET         0   /* buf mem low offset */
#define TXD_BUFADRHIGH_OFFSET        4   /* buf mem high offset */
#define TXD_LENGTH_OFFSET            8   /* buf length offset */
#define TXD_CSO_OFFSET               10  /* checksum offset */
#define TXD_CMD_OFFSET               11  /* command offset */
#define TXD_STATUS_OFFSET            12  /* status offset */
#define TXD_CSS_OFFSET               13  /* cksum start */
#define TXD_SPECIAL_OFFSET           14  /* special field */
#define TXD_SIZE                     16  /* descriptor size */

/* Legacy Transmit Descriptor */
typedef struct
{
    volatile UINT32	addrlo;
    volatile UINT32	addrhi;
    volatile UINT32	cmd;
    volatile UINT8	sts;
    volatile UINT8	popts;
    volatile UINT16	special;
} GEIF_TXD;


/* Descriptor access macro's */

#define GEIF_WRITE_DESC_WORD(pDesc, offset, value)      \
        (*(UINT16 *)((UINT32)pDesc + offset) = (UINT16)(value & 0xffff))

#define GEIF_WRITE_DESC_LONG(pDesc, offset, value)      \
        (*(UINT32 *)((UINT32)pDesc + offset) = (UINT32)value)

#define GEIF_READ_DESC_WORD(pDesc, offset, result)              \
        result = ((UINT16)(*(UINT16 *)((UINT32)pDesc + offset)) & 0xffff)

#define GEIF_READ_DESC_LONG(pDesc, offset, result)              \
        result = ((UINT32)( *(UINT32 *)((UINT32)pDesc + offset)))

#define GEIF_WRITE_DESC_BYTE(pDesc, offset, value)      \
        (*(UINT8 *)((UINT32)pDesc + offset) = (UINT8) (value & 0xff))

#define GEIF_READ_DESC_BYTE(pDesc, offset)              \
        ((UINT8)( *(UINT8 *)((UINT32)pDesc + offset)) & 0xff)

#define GEIF_GET_RXD_ADDR(offset)          \
        (pGeif->pRxDescMem + ((offset) * RXD_SIZE))

#define GEIF_GET_TXD_ADDR(offset)          \
        (pGeif->pTxDescMem + ((offset) * TXD_SIZE))

#define GEIF_GET_TXD_TAIL_UPDATE(tmp, num)     \
        (tmp) = (pGeif->txDescTail + (num)) % (GEIF_TXD_CNT) 

#define GEIF_GET_RXD_TAIL_UPDATE(tmp, num)        \
        (tmp) = (pGeif->rxDescTail + (num)) % (GEIF_RXD_CNT)	


/*******************************************************************************
*
* EEPROM Access, via the EERD register, Definitions
*
*******************************************************************************/

#define GEIF_EERD					0x14	/* EEPROM Read - RW */
#define GEIF_EEWR					0x102C	/* EEPROM Write Register - RW */
#define GEIF_EEPROM_RW_REG_DATA		16		/* Offset to data in EEPROM read/write registers */
#define GEIF_EEPROM_RW_REG_DONE		2		/* Offset to READ/WRITE done bit */
#define GEIF_EEPROM_RW_REG_START	1		/* First bit for telling part to start operation */
#define GEIF_EEPROM_RW_ADDR_SHIFT	2		/* Shift to the address bits */
#define GEIF_EEPROM_POLL_WRITE		1		/* Flag for polling for write complete */
#define GEIF_EEPROM_POLL_READ		0		/* Flag for polling for read complete */


/*******************************************************************************
*
* INVM data structure ids and types
*
*******************************************************************************/
#define GEIF_INVM_EECD_REG				0x12010
#define GEIF_EECD_FLASH_IN_USE			0x00000040L			/* i210 only */


#define GEIF_INVM_DATA_REG(_n)			(0x12120 + 4*(_n))
#define GEIF_INVM_SIZE					64
#define GEIF_INVM_ULT_BYTES_SIZE		8
#define GEIF_INVM_RECORD_SIZE_IN_BYTES	4
#define GEIF_INVM_VER_FIELD_ONE			0x1FF8
#define GEIF_INVM_VER_FIELD_TWO			0x7FE000
#define GEIF_INVM_IMGTYPE_FIELD			0x1F800000
#define GEIF_INVM_MAJOR_MASK			0x3F0
#define GEIF_INVM_MINOR_MASK			0xF
#define GEIF_INVM_MAJOR_SHIFT			4

#define GEIF_INVM_STRUCT_TYPE_MASK	0x0007
#define GEIF_INVM_NVM_END			0x0000
#define GEIF_INVM_WORD_AUTOLOAD		0x0001
#define GEIF_INVM_CSR_AUTOLOAD		0x0002
#define GEIF_INVM_PHY_AUTOLOAD		0x0003
#define GEIF_INVM_INVALIDATED		0x0007

#define GEIF_INVM_RESET_TYPE_MASK	0x0018	
#define GEIF_INVM_LOAD_POWER_UP		0x0000	/* only reset on power-up */
#define GEIF_INVM_LOAD_PCIE_RESET	0x0008	/* and on power-up reset */
#define GEIF_INVM_LOAD_SW_RESET		0x0018	/* and on pcie reset and power-up reset */

#define GEIF_INVM_WORD_ADDR_MASK	0xFE00
#define GEIF_INVM_CSR_ADDR_MASK		0x7FFF
#define GEIF_INVM_PHY_ADDR_MASK		0xF800

/*******************************************************************************
*
* ICH Flash Access Definitions
*
*******************************************************************************/

/*
 * Flash registers.
 * 
 */

#define GEIF_FL_GFP		0x0000
#define GEIF_FL_HFSSTS	0x0004
#define GEIF_FL_HFSCTL	0x0006
#define GEIF_FL_FADDR	0x0008
#define GEIF_FL_FDATA0	0x0010

/* Flash parameters, base page number and sector count */

#define GEIF_FL_GFP_BASE		0x00001FFF
#define GEIF_FL_GFP_SIZE		0x1FFF0000

/* Hardware sequencing flash status */

#define GEIF_FL_HFSSTS_FLCDONE	0x0001 /* cycle done */
#define GEIF_FL_HFSSTS_FLCERR	0x0002 /* cycle error */
#define GEIF_FL_HFSSTS_DAEL		0x0004 /* Direct access error log */
#define GEIF_FL_HFSSTS_ERSIZE	0x0018 /* Sector erase size */
#define GEIF_FL_HFSSTS_FCLBUSY	0x0020 /* Cycle in progress */
#define GEIF_FL_HFSSTS_DVALID	0x4000 /* Flash descriptor valid */
#define GEIF_FL_HFSSTS_LOCK		0x8000 /* Config lockdown */

/* Hardware sequencing flash control */

#define GEIF_FL_HFSCTL_FLCGO	0x0001 /* Initiate cycle */
#define GEIF_FL_HFSCTL_FLCYCLE	0x0006 /* Flash cycle */
#define GEIF_FL_HFSCTL_FLDCOUNT	0x0300 /* Flash data byte count */

#define GEIF_FL_READ		0x0000
#define GEIF_FL_WRITE		0x0004
#define GEIF_FL_ERASE		0x0006

#define GEIF_FL_SIZE_BYTE	0x0000
#define GEIF_FL_SIZE_WORD	0x0100
#define GEIF_FL_SIZE_LWORD	0x0200

#define GEIF_FL_SECTOR_SIZE	4096

/*
 * Macros for accessing the flash device registers.
 * These registers are accessed via a separate Ethernet controlled BAR. 
 */

#define GEIF_FL_READ_4( offset) \
	(*(UINT32*)(pGeif->flashAddr + offset))

#define GEIF_FL_WRITE_4( offset, data ) \
	(*(UINT32*)(pGeif->flashAddr + offset) = data)

#define GEIF_FL_READ_2( offset ) \
    (*(UINT16*)(pGeif->flashAddr + offset))

#define GEIF_FL_WRITE_2( offset, data ) \
	(*(UINT16*)(pGeif->flashAddr + offset) = data)

#define GEIF_FL_SETBIT_4( offset, val )          \
        GEIF_FL_WRITE_4( offset, GEIF_FL_READ_4( offset ) | (val))

#define GEIF_FL_CLRBIT_4( offset, val )          \
        GEIF_FL_WRITE_4( offset, GEIF_FL_READ_4( offset ) & ~(val))


#define GEIF_FL_SETBIT_2( offset, val )          \
        GEIF_FL_WRITE_2( offset, GEIF_FL_READ_2( offset ) | (val))

#define GEIF_FL_CLRBIT_2( offset, val )          \
        GEIF_FL_WRITE_2( offset, GEIF_FL_READ_2( offset ) & ~(val))


/*******************************************************************************
*
* PHY/MII Definitions
*
*******************************************************************************/

#define GEIF_OUI_INTEL		0x00AA00
#define MII_OUI(id1, id2)	(((id1) << 6) | ((id2) >> 10))
#define MII_MODEL(id2)      (((id2) & IDR2_MODEL) >> 4)
#define MII_REV(id2)        ((id2) & IDR2_REV)


/* MII definitions */

#define MII_CTRL_REG			0x0		/* Control Register */
#define MII_STAT_REG			0x1		/* Status Register */
#define MII_PHY_ID1_REG			0x2		/* PHY Identifier 1 Register */
#define MII_PHY_ID2_REG			0x3		/* PHY Identifier 2 Register */
#define MII_AN_ADS_REG			0x4		/* Auto-Negotiation 	  */
										/* Advertisement Register */
#define MII_AN_PRTN_REG			0x5		/* Auto-Negotiation 	    */
										/* Partner Ability Register */
#define MII_AN_EXP_REG			0x6		/* Auto-Negotiation   */
										/* Expansion Register */
#define MII_AN_NEXT_REG			0x7		/* Auto-Negotiation 	       */
										/* Next-page Transmit Register */
#define MII_AN_PRTN_NEXT_REG	0x8 	/* Link Partner Received Next Page Register */ 
#define MII_MASSLA_CTRL_REG		0x9 	/* MATER-SLAVE Control Register */
#define MII_MASSLA_STAT_REG		0xa 	/* MATER-SLAVE Status Tegister */
#define MII_EXT_STAT_REG		0xf 	/* Extented Status Register */
#define MII_PHY_PM_REG			0x19	/* PHY Power Management Register */
#define MII_PAGE_SELECT			0x1F 	/* Page Select Register */

/* MII control register bit  */

#define MII_CR_1000				0x0040	/* 1 = 1000mb when
						   				MII_CR_100 is also 1 */
#define MII_CR_COLL_TEST		0x0080	/* collision test */
#define MII_CR_FDX				0x0100	/* FDX =1, half duplex =0 */
#define MII_CR_RESTART			0x0200	/* restart auto negotiation */
#define MII_CR_ISOLATE			0x0400	/* isolate PHY from MII */
#define MII_CR_POWER_DOWN		0x0800	/* power down */
#define MII_CR_AUTO_EN			0x1000	/* auto-negotiation enable */
#define MII_CR_100				0x2000	/* 0 = 10mb, 1 = 100mb */
#define MII_CR_LOOPBACK			0x4000	/* 0 = normal, 1 = loopback */
#define MII_CR_RESET			0x8000	/* 0 = normal, 1 = PHY reset */
#define MII_CR_NORM_EN			0x0000	/* just enable the PHY */
#define MII_CR_DEF_0_MASK       0xca7f  /* they must return zero */
#define MII_CR_RES_MASK       	0x003f  /* reserved bits,return zero */

/* MII Status register bit definitions */

#define MII_SR_LINK_STATUS		0x0004  /* link Status -- 1 = link */
#define MII_SR_AUTO_SEL			0x0008  /* auto speed select capable */
#define MII_SR_REMOTE_FAULT     0x0010  /* Remote fault detect */
#define MII_SR_AUTO_NEG         0x0020  /* auto negotiation complete */
#define MII_SR_EXT_STS			0x0100	/* extended sts in reg 15 */
#define MII_SR_T2_HALF_DPX		0x0200	/* 100baseT2 HD capable */
#define MII_SR_T2_FULL_DPX		0x0400	/* 100baseT2 FD capable */
#define MII_SR_10T_HALF_DPX     0x0800  /* 10BaseT HD capable */
#define MII_SR_10T_FULL_DPX     0x1000  /* 10BaseT FD capable */
#define MII_SR_TX_HALF_DPX      0x2000  /* TX HD capable */
#define MII_SR_TX_FULL_DPX      0x4000  /* TX FD capable */
#define MII_SR_T4               0x8000  /* T4 capable */
#define MII_SR_ABIL_MASK        0xff80  /* abilities mask */
#define MII_SR_EXT_CAP          0x0001  /* extended capabilities */
#define MII_SR_SPEED_SEL_MASK 	0xf800  /* Mask to extract just speed
                                         * capabilities  from status
                                         * register. */    
                                                  
/* MII Master-Slave Control register bit definition */

#define MII_MASSLA_CTRL_1000T_HD    0x100
#define MII_MASSLA_CTRL_1000T_FD    0x200
#define MII_MASSLA_CTRL_PORT_TYPE   0x400
#define MII_MASSLA_CTRL_CONFIG_VAL  0x800
#define MII_MASSLA_CTRL_CONFIG_EN   0x1000

/* MII Master-Slave Status register bit definition */

#define MII_MASSLA_STAT_LP1000T_HD  0x400
#define MII_MASSLA_STAT_LP1000T_FD  0x800
#define MII_MASSLA_STAT_REMOTE_RCV  0x1000
#define MII_MASSLA_STAT_LOCAL_RCV   0x2000
#define MII_MASSLA_STAT_CONF_RES    0x4000
#define MII_MASSLA_STAT_CONF_FAULT  0x8000

/* MII Extented Status register bit definition */

#define MII_EXT_STAT_1000T_HD       0x1000
#define MII_EXT_STAT_1000T_FD       0x2000
#define MII_EXT_STAT_1000X_HD       0x4000
#define MII_EXT_STAT_1000X_FD       0x8000

/* MII AN advertisement Register bit definition */

#define MII_ANAR_10TX_HD        0x0020
#define MII_ANAR_10TX_FD        0x0040
#define MII_ANAR_100TX_HD       0x0080
#define MII_ANAR_100TX_FD       0x0100
#define MII_ANAR_100T_4         0x0200
#define MII_ANAR_PAUSE          0x0400
#define MII_ANAR_ASM_PAUSE      0x0800
#define MII_ANAR_REMORT_FAULT   0x2000
#define MII_ANAR_NEXT_PAGE      0x8000
#define MII_ANAR_PAUSE_MASK     0x0c00

/* MII Power Management Register bit definition */

#define MII_PHY_PM_OEM_GBE_DIS		0x0040 		/* Gigabit Disable */
#define MII_PHY_PM_OEM_LPLU			0x0004 		/* Low Power Link Up */
#define MII_PHY_PM_OEM_RESTART_AN	0x0400 		/* Restart Auto-negotiation */

/* MII Page Select Register bit definition */

#define MII_PAGE_SELECT_SHIFT	5


 /* Media types */
 
#define	IFM_AUTO	0		/* Autoselect best media */
#define	IFM_MANUAL	1		/* Jumper/dipswitch selects media */
#define	IFM_NONE	2		/* Deselect all media */

/* Media mode */
#define	IFM_ETHER	0x00000020
#define	IFM_10_T	3		/* 10BaseT - RJ45 */
#define	IFM_10_2	4		/* 10Base2 - Thinnet */
#define	IFM_10_5	5		/* 10Base5 - AUI */
#define	IFM_100_TX	6		/* 100BaseTX - RJ45 */
#define	IFM_100_FX	7		/* 100BaseFX - Fiber */
#define	IFM_100_T4	8		/* 100BaseT4 - 4 pair cat 3 */
#define	IFM_100_VG	9		/* 100VG-AnyLAN */
#define	IFM_100_T2	10		/* 100BaseT2 */
#define	IFM_1000_SX	11		/* 1000BaseSX - multi-mode fiber */
#define	IFM_10_STP	12		/* 10BaseT over shielded TP */
#define	IFM_10_FL	13		/* 10BaseFL - Fiber */
#define	IFM_1000_LX	14		/* 1000baseLX - single-mode fiber */
#define	IFM_1000_CX	15		/* 1000baseCX - 150ohm STP */
#define	IFM_1000_T	16		/* 1000baseT - 4 pair cat 5 */

/* Media status bits */

#define	IFM_AVALID	0x00000001	/* Active bit valid */
#define	IFM_ACTIVE	0x00000002	/* Interface attached to working net */

/* Media options */

#define	IFM_FDX		0x00100000	/* Force full duplex */
#define	IFM_HDX		0x00200000	/* Force half duplex */
#define	IFM_FLAG0	0x01000000	/* Driver defined flag */
#define	IFM_FLAG1	0x02000000	/* Driver defined flag */
#define	IFM_FLAG2	0x04000000	/* Driver defined flag */
#define	IFM_LOOP	0x08000000	/* Put hardware in loopback */

/* Media Masks */

#define	IFM_NMASK	0x000000e0	/* Network type */
#define	IFM_TMASK	0x0000001f	/* Media sub-type */
#define	IFM_IMASK	0xf0000000	/* Instance */
#define	IFM_ISHIFT	28			/* Instance shift */
#define	IFM_OMASK	0x0000ff00	/* Type specific options */
#define	IFM_MMASK	0x00070000	/* Mode */
#define	IFM_MSHIFT	16			/* Mode shift */
#define	IFM_GMASK	0x0ff00000	/* Global options */

/* Macros to extract various bits of information from the media word */

#define	IFM_TYPE(x)         ((x) & IFM_NMASK)
#define	IFM_SUBTYPE(x)      ((x) & IFM_TMASK)
#define	IFM_TYPE_OPTIONS(x) ((x) & IFM_OMASK)
#define	IFM_INST(x)         (((x) & IFM_IMASK) >> IFM_ISHIFT)
#define	IFM_OPTIONS(x)		((x) & (IFM_OMASK|IFM_GMASK))
#define	IFM_MODE(x)	    	((x) & IFM_MMASK)


#define NVM_82580_LAN_FUNC_OFFSET(a)    ((a) ? (0x40 + (0x40 * (a))) : 0)
#define NVM_CHECKSUM_REG                0x003F
#define NVM_SUM                         0xBABA
#define NVM_COMPATIBILITY_REG_3         0x0003
#define NVM_COMPATIBILITY_BIT_MASK      0x8000
#endif /* _GEIF_H */
