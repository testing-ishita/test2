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

/* sii3124.h - definitions for sii3124 devices
 */

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/sii3124.h,v 1.1 2013-09-04 07:40:27 chippisley Exp $
*
* $Log: sii3124.h,v $
* Revision 1.1  2013-09-04 07:40:27  chippisley
* Import files into new source repository.
*
* Revision 1.2  2009/06/19 13:44:52  jthiru
* Added check for port number passed by user
*
* Revision 1.1  2009/06/05 14:32:58  jthiru
* Initial check in for SII3124 test module
*
*
*
*/

#define BUFFER_ADDRESS			0x30000	/* address of sector buffer		*/
#define ATACMD_ADDRESS			0x40000	/* address of sector buffer		*/

/* SII3124 Error Codes at offset of 6 for each instance */
#define E__TEST_NO_DEVICE		E__BIT
#define E__REGS_ERROR			E__BIT + 0x6
#define E__DIAG_ERROR			E__BIT + 0xC
#define E__IRQ_ERROR			E__BIT + 0x12
#define E__READ_ERROR			E__BIT + 0x18
#define E__INIT_PORT            E__BIT + 0x1E
#define E__SOFT_RST             E__BIT + 0x24
#define E__INIT1_PORT           E__BIT + 0x2A
#define E__PORT_RESET			E__BIT + 0x30
#define E__TEST_NO_MEM			E__BIT + 0x31
#define E__IDE_NO_SIGN			E__BIT + 0x37
#define E__TEST_NO_PORT			E__BIT + 0x3D


#define PCI_MEM_BASE_MASK		0xFFFFFFF0
#define PCI_CACHE_LINE_SIZE		0x0C

#define HDC_CMD_RECAL	  		0x10	/* recalibrate command 			*/
#define HDC_CMD_READ	  		0x20	/* read command 				*/
#define HDC_CMD_WRITE	  		0x30	/* write command 				*/
#define HDC_CMD_VERIFY	  		0x40	/* verify command 				*/
#define HDC_CMD_FORMAT	  		0x50	/* format command 				*/
#define HDC_CMD_INIT	  		0x60	/* initialize command 			*/
#define HDC_CMD_SEEK	  		0x70	/* seek command 				*/
#define HDC_CMD_DIAG	  		0x90	/* perform diagnostic command 	*/
#define HDC_CMD_READ_LONG 		0x22	/* read long (ECC) command 		*/
#define HDC_CMD_WRITE_LONG 		0x32	/* write long (ECC) command 	*/
#define HDC_CMD_SET_PARAM 		0x91	/* set parameters command 		*/
#define HDC_CMD_IDENT			0xEC	/* Identify drive command 		*/

/*
 * Port request block (PRB) 32 bytes
 */
struct sil24Prb {
	UINT16	wCtrl;
	UINT16	wProt;
	UINT32	dRxCnt;
	UINT8	bFis[24];
};

/*
 * Scatter gather entry (SGE) 16 bytes
 */
struct sil24Sge {
	UINT32	dLowAddr;
	UINT32	dHighAddr;
	UINT32	dCnt;
	UINT32	dFlags;
};

/*
 * Port multiplier
 */
struct sil24PortMultiplier {
	UINT32	dDiag;
	UINT32	dSactive;
};

enum {
	SIL24_HOST_BAR		= 0,
	SIL24_PORT_BAR		= 2,

	/* sil24 fetches in chunks of 64bytes.  The first block
	 * contains the PRB and two SGEs.  From the second block, it's
	 * consisted of four SGEs and called SGT.  Calculate the
	 * number of SGTs that fit into one page.
	 */
	/*SIL24_PRB_SZ		= sizeof(struct sil24Prb)
				  + 2 * sizeof(struct sil24Sge),
	SIL24_MAX_SGT		= (PAGE_SIZE - SIL24_PRB_SZ)
				  / (4 * sizeof(struct sil24Sge)),*/

	/* This will give us one unused SGEs for ATA.  This extra SGE
	 * will be used to store CDB for ATAPI devices.
	 */
	/*SIL24_MAX_SGE		= 4 * SIL24_MAX_SGT + 1,*/

	/*
	 * Global controller registers (128 bytes @ BAR0)
	 */
		/* 32 bit regs */
	HOST_SLOT_STAT		= 0x00, /* 32 bit slot stat * 4 */
	HOST_CTRL		= 0x40,
	HOST_IRQ_STAT		= 0x44,
	HOST_PHY_CFG		= 0x48,
	HOST_BIST_CTRL		= 0x50,
	HOST_BIST_PTRN		= 0x54,
	HOST_BIST_STAT		= 0x58,
	HOST_MEM_BIST_STAT	= 0x5c,
	HOST_FLASH_CMD		= 0x70,
		/* 8 bit regs */
	HOST_FLASH_DATA		= 0x74,
	HOST_TRANSITION_DETECT	= 0x75,
	HOST_GPIO_CTRL		= 0x76,
	HOST_I2C_ADDR		= 0x78, /* 32 bit */
	HOST_I2C_DATA		= 0x7c,
	HOST_I2C_XFER_CNT	= 0x7e,
	HOST_I2C_CTRL		= 0x7f,

	/* HOST_SLOT_STAT bits */
	HOST_SSTAT_ATTN		= (1 << 31),

	/* HOST_CTRL bits */
	HOST_CTRL_M66EN		= (1 << 16), /* M66EN PCI bus signal */
	HOST_CTRL_TRDY		= (1 << 17), /* latched PCI TRDY */
	HOST_CTRL_STOP		= (1 << 18), /* latched PCI STOP */
	HOST_CTRL_DEVSEL	= (1 << 19), /* latched PCI DEVSEL */
	HOST_CTRL_REQ64		= (1 << 20), /* latched PCI REQ64 */
	HOST_CTRL_GLOBAL_RST	= (1 << 31), /* global reset */

	/*
	 * Port registers
	 * (8192 bytes @ +0x0000, +0x2000, +0x4000 and +0x6000 @ BAR2)
	 */
	PORT_REGS_SIZE		= 0x2000,

	PORT_LRAM		= 0x0000, /* 31 LRAM slots and PMP regs */
	PORT_LRAM_SLOT_SZ	= 0x0080, /* 32 bytes PRB + 2 SGE, ACT... */

	PORT_PMP		= 0x0f80, /* 8 bytes PMP * 16 (128 bytes) */
	PORT_PMP_STATUS		= 0x0000, /* port device status offset */
	PORT_PMP_QACTIVE	= 0x0004, /* port device QActive offset */
	PORT_PMP_SIZE		= 0x0008, /* 8 bytes per PMP */

		/* 32 bit regs */
	PORT_CTRL_STAT		= 0x1000, /* write: ctrl-set, read: stat */
	PORT_CTRL_CLR		= 0x1004, /* write: ctrl-clear */
	PORT_IRQ_STAT		= 0x1008, /* high: status, low: interrupt */
	PORT_IRQ_ENABLE_SET	= 0x1010, /* write: enable-set */
	PORT_IRQ_ENABLE_CLR	= 0x1014, /* write: enable-clear */
	PORT_ACTIVATE_UPPER_ADDR= 0x101c,
	PORT_EXEC_FIFO		= 0x1020, /* command execution fifo */
	PORT_CMD_ERR		= 0x1024, /* command error number */
	PORT_FIS_CFG		= 0x1028,
	PORT_FIFO_THRES		= 0x102c,
		/* 16 bit regs */
	PORT_DECODE_ERR_CNT	= 0x1040,
	PORT_DECODE_ERR_THRESH	= 0x1042,
	PORT_CRC_ERR_CNT	= 0x1044,
	PORT_CRC_ERR_THRESH	= 0x1046,
	PORT_HSHK_ERR_CNT	= 0x1048,
	PORT_HSHK_ERR_THRESH	= 0x104a,
		/* 32 bit regs */
	PORT_PHY_CFG		= 0x1050,
	PORT_SLOT_STAT		= 0x1800,
	PORT_CMD_ACTIVATE	= 0x1c00, /* 64 bit cmd activate * 31 (248 bytes) */
	PORT_CONTEXT		= 0x1e04,
	PORT_EXEC_DIAG		= 0x1e00, /* 32bit exec diag * 16 (64 bytes, 0-10 used on 3124) */
	PORT_PSD_DIAG		= 0x1e40, /* 32bit psd diag * 16 (64 bytes, 0-8 used on 3124) */
	PORT_SCONTROL		= 0x1f00,
	PORT_SSTATUS		= 0x1f04,
	PORT_SERROR		= 0x1f08,
	PORT_SACTIVE		= 0x1f0c,

	/* PORT_CTRL_STAT bits */
	PORT_CS_PORT_RST	= (1 << 0), /* port reset */
	PORT_CS_DEV_RST		= (1 << 1), /* device reset */
	PORT_CS_INIT		= (1 << 2), /* port initialize */
	PORT_CS_IRQ_WOC		= (1 << 3), /* interrupt write one to clear */
	PORT_CS_CDB16		= (1 << 5), /* 0=12b cdb, 1=16b cdb */
	PORT_CS_PMP_RESUME	= (1 << 6), /* PMP resume */
	PORT_CS_32BIT_ACTV	= (1 << 10), /* 32-bit activation */
	PORT_CS_PMP_EN		= (1 << 13), /* port multiplier enable */
	PORT_CS_RDY		   =  (1 << 31), /* port ready to accept commands */

	/* PORT_IRQ_STAT/ENABLE_SET/CLR */
	/* bits[11:0] are masked */
	PORT_IRQ_COMPLETE	= (1 << 0), /* command(s) completed */
	PORT_IRQ_ERROR		= (1 << 1), /* command execution error */
	PORT_IRQ_PORTRDY_CHG	= (1 << 2), /* port ready change */
	PORT_IRQ_PWR_CHG	= (1 << 3), /* power management change */
	PORT_IRQ_PHYRDY_CHG	= (1 << 4), /* PHY ready change */
	PORT_IRQ_COMWAKE	= (1 << 5), /* COMWAKE received */
	PORT_IRQ_UNK_FIS	= (1 << 6), /* unknown FIS received */
	PORT_IRQ_DEV_XCHG	= (1 << 7), /* device exchanged */
	PORT_IRQ_8B10B		= (1 << 8), /* 8b/10b decode error threshold */
	PORT_IRQ_CRC		= (1 << 9), /* CRC error threshold */
	PORT_IRQ_HANDSHAKE	= (1 << 10), /* handshake error threshold */
	PORT_IRQ_SDB_NOTIFY	= (1 << 11), /* SDB notify received */

	DEF_PORT_IRQ		= PORT_IRQ_COMPLETE | PORT_IRQ_ERROR |
				  PORT_IRQ_PHYRDY_CHG | PORT_IRQ_DEV_XCHG |
				  PORT_IRQ_UNK_FIS | PORT_IRQ_SDB_NOTIFY,

	/* bits[27:16] are unmasked (raw) */
	PORT_IRQ_RAW_SHIFT	= 16,
	PORT_IRQ_MASKED_MASK	= 0x7ff,
	PORT_IRQ_RAW_MASK	= (0x7ff << PORT_IRQ_RAW_SHIFT),

	/* ENABLE_SET/CLR specific, intr steering - 2 bit field */
	PORT_IRQ_STEER_SHIFT	= 30,
	PORT_IRQ_STEER_MASK	= (3 << PORT_IRQ_STEER_SHIFT),

	/* PORT_CMD_ERR constants */
	PORT_CERR_DEV		= 1, /* Error bit in D2H Register FIS */
	PORT_CERR_SDB		= 2, /* Error bit in SDB FIS */
	PORT_CERR_DATA		= 3, /* Error in data FIS not detected by dev */
	PORT_CERR_SEND		= 4, /* Initial cmd FIS transmission failure */
	PORT_CERR_INCONSISTENT	= 5, /* Protocol mismatch */
	PORT_CERR_DIRECTION	= 6, /* Data direction mismatch */
	PORT_CERR_UNDERRUN	= 7, /* Ran out of SGEs while writing */
	PORT_CERR_OVERRUN	= 8, /* Ran out of SGEs while reading */
	PORT_CERR_PKT_PROT	= 11, /* DIR invalid in 1st PIO setup of ATAPI */
	PORT_CERR_SGT_BOUNDARY	= 16, /* PLD ecode 00 - SGT not on qword boundary */
	PORT_CERR_SGT_TGTABRT	= 17, /* PLD ecode 01 - target abort */
	PORT_CERR_SGT_MSTABRT	= 18, /* PLD ecode 10 - master abort */
	PORT_CERR_SGT_PCIPERR	= 19, /* PLD ecode 11 - PCI parity err while fetching SGT */
	PORT_CERR_CMD_BOUNDARY	= 24, /* ctrl[15:13] 001 - PRB not on qword boundary */
	PORT_CERR_CMD_TGTABRT	= 25, /* ctrl[15:13] 010 - target abort */
	PORT_CERR_CMD_MSTABRT	= 26, /* ctrl[15:13] 100 - master abort */
	PORT_CERR_CMD_PCIPERR	= 27, /* ctrl[15:13] 110 - PCI parity err while fetching PRB */
	PORT_CERR_XFR_UNDEF	= 32, /* PSD ecode 00 - undefined */
	PORT_CERR_XFR_TGTABRT	= 33, /* PSD ecode 01 - target abort */
	PORT_CERR_XFR_MSTABRT	= 34, /* PSD ecode 10 - master abort */
	PORT_CERR_XFR_PCIPERR	= 35, /* PSD ecode 11 - PCI prity err during transfer */
	PORT_CERR_SENDSERVICE	= 36, /* FIS received while sending service */

	/* bits of PRB control field */
	PRB_CTRL_PROTOCOL	= (1 << 0), /* override def. ATA protocol */
	PRB_CTRL_PACKET_READ	= (1 << 4), /* PACKET cmd read */
	PRB_CTRL_PACKET_WRITE	= (1 << 5), /* PACKET cmd write */
	PRB_CTRL_NIEN		= (1 << 6), /* Mask completion irq */
	PRB_CTRL_SRST		= (1 << 7), /* Soft reset request (ign BSY?) */

	/* PRB protocol field */
	PRB_PROT_PACKET		= (1 << 0),
	PRB_PROT_TCQ		= (1 << 1),
	PRB_PROT_NCQ		= (1 << 2),
	PRB_PROT_READ		= (1 << 3),
	PRB_PROT_WRITE		= (1 << 4),
	PRB_PROT_TRANSPARENT	= (1 << 5),

	/*
	 * Other constants
	 */
	SGE_TRM			= (1 << 31), /* Last SGE in chain */
	SGE_LNK			= (1 << 30), /* linked list
						Points to SGT, not SGE */
	SGE_DRD			= (1 << 29), /* discard data read (/dev/null)
						data address ignored */

	SIL24_MAX_CMDS		= 31,

	/* board id */
	BID_SIL3124		= 0,
	BID_SIL3132		= 1,
	BID_SIL3131		= 2,

	/* host flags */
	SIL24_FLAG_PCIX_IRQ_WOC	= (1 << 24), /* IRQ loss errata on PCI-X */
	
	IRQ_STAT_4PORTS		= 0xf
};

struct sil24AtaBlock {
	struct sil24Prb prb;
	struct sil24Sge sge[256];
};

struct sil24AtapiBlock {
	struct sil24Prb prb;
	UINT8 bCdb[16];
	struct sil24Sge sge[256];
};

union sil24CmdBlock {
	struct sil24AtaBlock ata;
	struct sil24AtapiBlock atapi;
};


/* structure for identify drive data */
struct sIdentify 
{
	UINT16 	wConfig,			/* 0  */
			NoPhyCyls,			/* 1  */
			Resvd1,				/* 2  */
			NoHeads,			/* 3  */
			NoUnfBPT,			/* 4  */
			NoUnfBPS,			/* 5  */
			NoPhySPT,			/* 6  */
			Resvd2[3];			/* 7-9 */
	char	SerNo[20];			/* 10-19 */
	UINT16	BufType, 			/* 20 */
			BufSize,			/* 21 */
			NoECCBytes;			/* 22 */
	char	FirmId[8],			/* 23-26 */
			ModelNo[40];		/* 27-46 */
	UINT16	SectPerIrqM,		/* 47 */
			Mode32,				/* 48 */
			ModeDmaLba,			/* 49 */
			Resvd3,				/* 50 */
			ModePio,			/* 51 */
			ModeDma,			/* 52 */
			Resvd4,				/* 53 */
			NoLogCyls, 			/* 54 */
			NoLogHeads, 		/* 55 */
			NoLogSPT;			/* 56 */
	UINT32	NoLogBPS;			/* 57 */
	UINT16	SectPerIrq;			/* 59 */
	UINT32	NoLbaSectors;		/* 60 */
	UINT16	SingleDma,			/* 62 */
			MultiDma,			/* 63 */
			ModeAdvPio,			/* 64 */
			ModeDmaCycle,		/* 65 */
			Resvd5[22],			/* 66-87 */
			ModeUdma,			/* 88 */
			Resvd7a[4],			/* 89-92 */
			HwReset,			/* 93 */
			Resvd7b[34],		/* 94-127 */
/*			Resvd7[39],			 89-127 */
			Man[32],			/* 128-159 */
			Resvd6[96];			/* 160-255 */
};
			

