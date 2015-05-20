#ifndef __pci_h__
	#define __pci_h__

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

/* pci.h - PCI device access and search functions (pci.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/pci.h,v 1.2 2014-01-29 13:19:33 hchalla Exp $
 *
 * $Log: pci.h,v $
 * Revision 1.2  2014-01-29 13:19:33  hchalla
 * Added new defines for PCIE express extended config space registers.
 *
 * Revision 1.1  2013-09-04 07:35:27  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.7  2010/09/15 12:18:38  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.6  2010/09/10 10:56:02  cdobson
 * Added dPciGetTopOfMem definition.
 *
 * Revision 1.5  2010/06/23 10:59:21  hmuneer
 * CUTE V1.01
 *
 * Revision 1.4  2008/07/24 14:49:29  swilson
 * Additional definitions required by PCI initialization code.
 *
 * Revision 1.3  2008/06/20 12:16:58  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.2  2008/05/12 14:54:58  swilson
 * Update with revised parameter ordering.
 *
 * Revision 1.1  2008/04/30 07:51:21  swilson
 * Add PCI support library.
 *
 * Revision 1.1  2008/04/24 16:04:38  swilson
 * Updated includes to support HAL and BIT library.
 *
 */


/* includes */

#include <stdtypes.h>

/* defines */

#define PCI_MAKE_PFA(b,d,f)	(((UINT16)(b) << 8) | ((UINT16)(d) << 3) | (UINT16)(f))
#define PCI_PFA_BUS(pfa)	((pfa) >> 8)
#define PCI_PFA_DEV(pfa)	(((pfa) >> 3) & 0x1F)
#define PCI_PFA_FUNC(pfa)	((pfa) & 0x7)

#define PCI_PFA_NEXT_BUS(pfa)	( ((pfa) & 0x00FF) | (((pfa) + 0x100) & 0xFF00) )
#define PCI_PFA_NEXT_DEV(pfa)	( ((pfa) & 0xFF07) | (((pfa) + 8)     & 0x00F8) )
#define PCI_PFA_NEXT_FUNC(pfa)	( ((pfa) & 0xFFF8) | (((pfa) + 1)     & 0x0007) )

	/* Size-specific macro substitutions */

#define PCI_READ_BYTE(p,r)	( (UINT8)  dPciReadReg((p),(r),REG_8)  )
#define PCI_READ_WORD(p,r)	( (UINT16) dPciReadReg((p),(r),REG_16) )
#define PCI_READ_DWORD(p,r)	( (UINT32) dPciReadReg((p),(r),REG_32) )

#define PCI_WRITE_BYTE(p,r,v)	( vPciWriteReg((p),(r),REG_8, (UINT32)(v)) )
#define PCI_WRITE_WORD(p,r,v)	( vPciWriteReg((p),(r),REG_16,(UINT32)(v)) )
#define PCI_WRITE_DWORD(p,r,v)	( vPciWriteReg((p),(r),REG_32,(UINT32)(v)) )


#define PCIE_READ_BYTE(p,r)		( (UINT8)  dPciReadRegExtd((p),(r),REG_8)  )
#define PCIE_READ_WORD(p,r)		( (UINT16) dPciReadRegExtd((p),(r),REG_16) )
#define PCIE_READ_DWORD(p,r)	( (UINT32) dPciReadRegExtd((p),(r),REG_32) )

#define PCIE_WRITE_BYTE(p,r,v)	( vPciWriteRegExtd((p),(r),REG_8, (UINT32)(v)) )
#define PCIE_WRITE_WORD(p,r,v)	( vPciWriteRegExtd((p),(r),REG_16,(UINT32)(v)) )
#define PCIE_WRITE_DWORD(p,r,v)	( vPciWriteRegExtd((p),(r),REG_32,(UINT32)(v)) )

	/* Error codes */
	
#define E__DEVICE_NOT_FOUND		(E__PCI + 0x00000001)

	/* Type-0: Devices */

#define PCI_VENDOR_ID			0x00
#define PCI_DEVICE_ID			0x02
#define PCI_COMMAND				0x04
#define  PCI_BMEN						0x0004
#define  PCI_MEMEN						0x0002
#define  PCI_IOEN						0x0001
#define PCI_STATUS				0x06
#define PCI_REVISION_ID			0x08
#define PCI_PROG_IF				0x09
#define PCI_SUB_CLASS			0x0A
#define PCI_BASE_CLASS			0x0B
#define  PCI_BASECLASS_BRIDGE			0x06
#define  PCI_SUBCLASS_PCI2PCI			0x04
#define PCI_CACHE_LINE			0x0C
#define PCI_LATENCY				0x0D
#define PCI_HEADER_TYPE			0x0E
#define PCI_BIST				0x0F
#define PCI_BAR0				0x10
#define PCI_BAR1				0x14
#define PCI_BAR2				0x18
#define PCI_BAR3				0x1C
#define PCI_BAR4				0x20
#define PCI_BAR5				0x24
#define PCI_CIS_POINTER			0x28
#define PCI_SUBSYS_VID			0x2C
#define PCI_SUBSYS_DID			0x2E
#define PCI_ROM_BAR				0x30
#define PCI_CAP_POINTER			0x34
#define PCI_INT_LINE			0x3C
#define PCI_INT_PIN				0x3D
#define PCI_MIN_GRANT			0x3E
#define PCI_MAX_LATENCY			0x3F

	/* Type-1: PCI-PCI Bridge */

#define PCI_PRI_BUS				0x18
#define PCI_SEC_BUS				0x19
#define PCI_SUB_BUS				0x1A
#define PCI_SEC_LATENCY			0x1B
#define PCI_IO_BASE				0x1C
#define PCI_IO_LIMIT			0x1D
#define PCI_SEC_STAT			0x1E
#define PCI_MEM_BASE			0x20
#define PCI_MEM_LIMIT			0x22
#define PCI_PREF_BASE			0x24
#define PCI_PREF_LIMIT			0x26
#define PCI_PREF_UBASE			0x28
#define PCI_PREF_ULIMIT			0x2C
#define PCI_IO_UBASE			0x30
#define PCI_IO_ULIMIT			0x32
#define PCI_BR_ROM_BAR			0x38
#define PCI_BRIDGE_CTRL			0x3E

	/* Type-2: CardBus Bridge */


/* PCI Express Registers */

#define PCI_EXP_FLAGS       2   /* Capabilities register */
#define PCI_EXP_FLAGS_VERS  0x000f  /* Capability version */
#define PCI_EXP_FLAGS_TYPE  0x00f0  /* Device/Port type */
#define  PCI_EXP_TYPE_ENDPOINT  0x0 /* Express Endpoint */
#define  PCI_EXP_TYPE_LEG_END   0x1 /* Legacy Endpoint */
#define  PCI_EXP_TYPE_ROOT_PORT 0x4 /* Root Port */
#define  PCI_EXP_TYPE_UPSTREAM  0x5 /* Upstream Port */
#define  PCI_EXP_TYPE_DOWNSTREAM 0x6    /* Downstream Port */
#define  PCI_EXP_TYPE_PCI_BRIDGE 0x7    /* PCI/PCI-X Bridge */
#define  PCI_EXP_TYPE_RC_END    0x9 /* Root Complex Integrated Endpoint */
#define  PCI_EXP_TYPE_RC_EC 0xa /* Root Complex Event Collector */
#define PCI_EXP_FLAGS_SLOT  0x0100  /* Slot implemented */
#define PCI_EXP_FLAGS_IRQ   0x3e00  /* Interrupt message number */
#define PCI_EXP_DEVCAP      4   /* Device capabilities */
#define  PCI_EXP_DEVCAP_PAYLOAD 0x07    /* Max_Payload_Size */
#define  PCI_EXP_DEVCAP_PHANTOM 0x18    /* Phantom functions */
#define  PCI_EXP_DEVCAP_EXT_TAG 0x20    /* Extended tags */
#define  PCI_EXP_DEVCAP_L0S 0x1c0   /* L0s Acceptable Latency */
#define  PCI_EXP_DEVCAP_L1  0xe00   /* L1 Acceptable Latency */
#define  PCI_EXP_DEVCAP_ATN_BUT 0x1000  /* Attention Button Present */
#define  PCI_EXP_DEVCAP_ATN_IND 0x2000  /* Attention Indicator Present */
#define  PCI_EXP_DEVCAP_PWR_IND 0x4000  /* Power Indicator Present */
#define  PCI_EXP_DEVCAP_RBER    0x8000  /* Role-Based Error Reporting */
#define  PCI_EXP_DEVCAP_PWR_VAL 0x3fc0000 /* Slot Power Limit Value */
#define  PCI_EXP_DEVCAP_PWR_SCL 0xc000000 /* Slot Power Limit Scale */
#define  PCI_EXP_DEVCAP_FLR     0x10000000 /* Function Level Reset */
#define PCI_EXP_DEVCTL      8   /* Device Control */
#define  PCI_EXP_DEVCTL_CERE    0x0001  /* Correctable Error Reporting En. */
#define  PCI_EXP_DEVCTL_NFERE   0x0002  /* Non-Fatal Error Reporting Enable */
#define  PCI_EXP_DEVCTL_FERE    0x0004  /* Fatal Error Reporting Enable */
#define  PCI_EXP_DEVCTL_URRE    0x0008  /* Unsupported Request Reporting En. */
#define  PCI_EXP_DEVCTL_RELAX_EN 0x0010 /* Enable relaxed ordering */
#define  PCI_EXP_DEVCTL_PAYLOAD 0x00e0  /* Max_Payload_Size */
#define  PCI_EXP_DEVCTL_EXT_TAG 0x0100  /* Extended Tag Field Enable */
#define  PCI_EXP_DEVCTL_PHANTOM 0x0200  /* Phantom Functions Enable */
#define  PCI_EXP_DEVCTL_AUX_PME 0x0400  /* Auxiliary Power PM Enable */
#define  PCI_EXP_DEVCTL_NOSNOOP_EN 0x0800  /* Enable No Snoop */
#define  PCI_EXP_DEVCTL_READRQ  0x7000  /* Max_Read_Request_Size */
#define  PCI_EXP_DEVCTL_BCR_FLR 0x8000  /* Bridge Configuration Retry / FLR */
#define PCI_EXP_DEVSTA      10  /* Device Status */
#define  PCI_EXP_DEVSTA_CED 0x01    /* Correctable Error Detected */
#define  PCI_EXP_DEVSTA_NFED    0x02    /* Non-Fatal Error Detected */
#define  PCI_EXP_DEVSTA_FED 0x04    /* Fatal Error Detected */
#define  PCI_EXP_DEVSTA_URD 0x08    /* Unsupported Request Detected */
#define  PCI_EXP_DEVSTA_AUXPD   0x10    /* AUX Power Detected */
#define  PCI_EXP_DEVSTA_TRPND   0x20    /* Transactions Pending */
#define PCI_EXP_LNKCAP      12  /* Link Capabilities */
#define  PCI_EXP_LNKCAP_SLS 0x0000000f /* Supported Link Speeds */
#define  PCI_EXP_LNKCAP_MLW 0x000003f0 /* Maximum Link Width */
#define  PCI_EXP_LNKCAP_ASPMS   0x00000c00 /* ASPM Support */
#define  PCI_EXP_LNKCAP_L0SEL   0x00007000 /* L0s Exit Latency */
#define  PCI_EXP_LNKCAP_L1EL    0x00038000 /* L1 Exit Latency */
#define  PCI_EXP_LNKCAP_CLKPM   0x00040000 /* L1 Clock Power Management */
#define  PCI_EXP_LNKCAP_SDERC   0x00080000 /* Suprise Down Error Reporting Capable */
#define  PCI_EXP_LNKCAP_DLLLARC 0x00100000 /* Data Link Layer Link Active Reporting Capable */
#define  PCI_EXP_LNKCAP_LBNC    0x00200000 /* Link Bandwidth Notification Capability */
#define  PCI_EXP_LNKCAP_PN  0xff000000 /* Port Number */
#define PCI_EXP_LNKCTL      16  /* Link Control */
#define  PCI_EXP_LNKCTL_ASPMC   0x0003  /* ASPM Control */
#define  PCI_EXP_LNKCTL_RCB 0x0008  /* Read Completion Boundary */
#define  PCI_EXP_LNKCTL_LD  0x0010  /* Link Disable */
#define  PCI_EXP_LNKCTL_RL  0x0020  /* Retrain Link */
#define  PCI_EXP_LNKCTL_CCC 0x0040  /* Common Clock Configuration */
#define  PCI_EXP_LNKCTL_ES  0x0080  /* Extended Synch */
#define  PCI_EXP_LNKCTL_CLKREQ_EN 0x100 /* Enable clkreq */
#define  PCI_EXP_LNKCTL_HAWD    0x0200  /* Hardware Autonomous Width Disable */
#define  PCI_EXP_LNKCTL_LBMIE   0x0400  /* Link Bandwidth Management Interrupt Enable */
#define  PCI_EXP_LNKCTL_LABIE   0x0800  /* Lnk Autonomous Bandwidth Interrupt Enable */
#define PCI_EXP_LNKSTA      18  /* Link Status */
#define  PCI_EXP_LNKSTA_CLS 0x000f  /* Current Link Speed */
#define  PCI_EXP_LNKSTA_CLS_2_5GB 0x01  /* Current Link Speed 2.5GT/s */
#define  PCI_EXP_LNKSTA_CLS_5_0GB 0x02  /* Current Link Speed 5.0GT/s */
#define  PCI_EXP_LNKSTA_NLW 0x03f0  /* Nogotiated Link Width */
#define  PCI_EXP_LNKSTA_NLW_SHIFT 4 /* start of NLW mask in link status */
#define  PCI_EXP_LNKSTA_LT  0x0800  /* Link Training */
#define  PCI_EXP_LNKSTA_SLC 0x1000  /* Slot Clock Configuration */
#define  PCI_EXP_LNKSTA_DLLLA   0x2000  /* Data Link Layer Link Active */
#define  PCI_EXP_LNKSTA_LBMS    0x4000  /* Link Bandwidth Management Status */
#define  PCI_EXP_LNKSTA_LABS    0x8000  /* Link Autonomous Bandwidth Status */
#define PCI_EXP_SLTCAP      20  /* Slot Capabilities */
#define  PCI_EXP_SLTCAP_ABP 0x00000001 /* Attention Button Present */
#define  PCI_EXP_SLTCAP_PCP 0x00000002 /* Power Controller Present */
#define  PCI_EXP_SLTCAP_MRLSP   0x00000004 /* MRL Sensor Present */
#define  PCI_EXP_SLTCAP_AIP 0x00000008 /* Attention Indicator Present */
#define  PCI_EXP_SLTCAP_PIP 0x00000010 /* Power Indicator Present */
#define  PCI_EXP_SLTCAP_HPS 0x00000020 /* Hot-Plug Surprise */
#define  PCI_EXP_SLTCAP_HPC 0x00000040 /* Hot-Plug Capable */
#define  PCI_EXP_SLTCAP_SPLV    0x00007f80 /* Slot Power Limit Value */
#define  PCI_EXP_SLTCAP_SPLS    0x00018000 /* Slot Power Limit Scale */
#define  PCI_EXP_SLTCAP_EIP 0x00020000 /* Electromechanical Interlock Present */
#define  PCI_EXP_SLTCAP_NCCS    0x00040000 /* No Command Completed Support */
#define  PCI_EXP_SLTCAP_PSN 0xfff80000 /* Physical Slot Number */
#define PCI_EXP_SLTCTL      24  /* Slot Control */
#define  PCI_EXP_SLTCTL_ABPE    0x0001  /* Attention Button Pressed Enable */
#define  PCI_EXP_SLTCTL_PFDE    0x0002  /* Power Fault Detected Enable */
#define  PCI_EXP_SLTCTL_MRLSCE  0x0004  /* MRL Sensor Changed Enable */
#define  PCI_EXP_SLTCTL_PDCE    0x0008  /* Presence Detect Changed Enable */
#define  PCI_EXP_SLTCTL_CCIE    0x0010  /* Command Completed Interrupt Enable */
#define  PCI_EXP_SLTCTL_HPIE    0x0020  /* Hot-Plug Interrupt Enable */
#define  PCI_EXP_SLTCTL_AIC 0x00c0  /* Attention Indicator Control */
#define  PCI_EXP_SLTCTL_PIC 0x0300  /* Power Indicator Control */
#define  PCI_EXP_SLTCTL_PCC 0x0400  /* Power Controller Control */
#define  PCI_EXP_SLTCTL_EIC 0x0800  /* Electromechanical Interlock Control */
#define  PCI_EXP_SLTCTL_DLLSCE  0x1000  /* Data Link Layer State Changed Enable */
#define PCI_EXP_SLTSTA      26  /* Slot Status */
#define  PCI_EXP_SLTSTA_ABP 0x0001  /* Attention Button Pressed */
#define  PCI_EXP_SLTSTA_PFD 0x0002  /* Power Fault Detected */
#define  PCI_EXP_SLTSTA_MRLSC   0x0004  /* MRL Sensor Changed */
#define  PCI_EXP_SLTSTA_PDC 0x0008  /* Presence Detect Changed */
#define  PCI_EXP_SLTSTA_CC  0x0010  /* Command Completed */
#define  PCI_EXP_SLTSTA_MRLSS   0x0020  /* MRL Sensor State */
#define  PCI_EXP_SLTSTA_PDS 0x0040  /* Presence Detect State */
#define  PCI_EXP_SLTSTA_EIS 0x0080  /* Electromechanical Interlock Status */
#define  PCI_EXP_SLTSTA_DLLSC   0x0100  /* Data Link Layer State Changed */
#define PCI_EXP_RTCTL       28  /* Root Control */
#define  PCI_EXP_RTCTL_SECEE    0x01    /* System Error on Correctable Error */
#define  PCI_EXP_RTCTL_SENFEE   0x02    /* System Error on Non-Fatal Error */
#define  PCI_EXP_RTCTL_SEFEE    0x04    /* System Error on Fatal Error */
#define  PCI_EXP_RTCTL_PMEIE    0x08    /* PME Interrupt Enable */
#define  PCI_EXP_RTCTL_CRSSVE   0x10    /* CRS Software Visibility Enable */
#define PCI_EXP_RTCAP       30  /* Root Capabilities */
#define PCI_EXP_RTSTA       32  /* Root Status */
#define PCI_EXP_DEVCAP2     36  /* Device Capabilities 2 */
#define  PCI_EXP_DEVCAP2_ARI    0x20    /* Alternative Routing-ID */
#define PCI_EXP_DEVCTL2     40  /* Device Control 2 */
#define  PCI_EXP_DEVCTL2_ARI    0x20    /* Alternative Routing-ID */
#define PCI_EXP_LNKCTL2     48  /* Link Control 2 */
#define PCI_EXP_SLTCTL2     56  /* Slot Control 2 */

/* Extended Capabilities (PCI-X 2.0 and Express) */
#define PCI_EXT_CAP_ID(header)      (header & 0x0000ffff)
#define PCI_EXT_CAP_VER(header)     ((header >> 16) & 0xf)
#define PCI_EXT_CAP_NEXT(header)    ((header >> 20) & 0xffc)

#define PCI_EXT_CAP_ID_ERR  1
#define PCI_EXT_CAP_ID_VC   2
#define PCI_EXT_CAP_ID_DSN  3
#define PCI_EXT_CAP_ID_PWR  4
#define PCI_EXT_CAP_ID_ARI  14
#define PCI_EXT_CAP_ID_ATS  15
#define PCI_EXT_CAP_ID_SRIOV    16

/* Advanced Error Reporting */
#define PCI_ERR_UNCOR_STATUS    4   /* Uncorrectable Error Status */
#define  PCI_ERR_UNC_TRAIN  0x00000001  /* Training */
#define  PCI_ERR_UNC_DLP    0x00000010  /* Data Link Protocol */
#define  PCI_ERR_UNC_POISON_TLP 0x00001000  /* Poisoned TLP */
#define  PCI_ERR_UNC_FCP    0x00002000  /* Flow Control Protocol */
#define  PCI_ERR_UNC_COMP_TIME  0x00004000  /* Completion Timeout */
#define  PCI_ERR_UNC_COMP_ABORT 0x00008000  /* Completer Abort */
#define  PCI_ERR_UNC_UNX_COMP   0x00010000  /* Unexpected Completion */
#define  PCI_ERR_UNC_RX_OVER    0x00020000  /* Receiver Overflow */
#define  PCI_ERR_UNC_MALF_TLP   0x00040000  /* Malformed TLP */
#define  PCI_ERR_UNC_ECRC   0x00080000  /* ECRC Error Status */
#define  PCI_ERR_UNC_UNSUP  0x00100000  /* Unsupported Request */
#define PCI_ERR_UNCOR_MASK  8   /* Uncorrectable Error Mask */
    /* Same bits as above */
#define PCI_ERR_UNCOR_SEVER 12  /* Uncorrectable Error Severity */
    /* Same bits as above */
#define PCI_ERR_COR_STATUS  16  /* Correctable Error Status */
#define  PCI_ERR_COR_RCVR   0x00000001  /* Receiver Error Status */
#define  PCI_ERR_COR_BAD_TLP    0x00000040  /* Bad TLP Status */
#define  PCI_ERR_COR_BAD_DLLP   0x00000080  /* Bad DLLP Status */
#define  PCI_ERR_COR_REP_ROLL   0x00000100  /* REPLAY_NUM Rollover */
#define  PCI_ERR_COR_REP_TIMER  0x00001000  /* Replay Timer Timeout */
#define PCI_ERR_COR_MASK    20  /* Correctable Error Mask */
    /* Same bits as above */
#define PCI_ERR_CAP     24  /* Advanced Error Capabilities */
#define  PCI_ERR_CAP_FEP(x) ((x) & 31)  /* First Error Pointer */
#define  PCI_ERR_CAP_ECRC_GENC  0x00000020  /* ECRC Generation Capable */
#define  PCI_ERR_CAP_ECRC_GENE  0x00000040  /* ECRC Generation Enable */
#define  PCI_ERR_CAP_ECRC_CHKC  0x00000080  /* ECRC Check Capable */
#define  PCI_ERR_CAP_ECRC_CHKE  0x00000100  /* ECRC Check Enable */
#define PCI_ERR_HEADER_LOG  28  /* Header Log Register (16 bytes) */
#define PCI_ERR_ROOT_COMMAND    44  /* Root Error Command */
/* Correctable Err Reporting Enable */
#define PCI_ERR_ROOT_CMD_COR_EN     0x00000001
/* Non-fatal Err Reporting Enable */
#define PCI_ERR_ROOT_CMD_NONFATAL_EN    0x00000002
/* Fatal Err Reporting Enable */
#define PCI_ERR_ROOT_CMD_FATAL_EN   0x00000004
#define PCI_ERR_ROOT_STATUS 48
#define PCI_ERR_ROOT_COR_RCV        0x00000001  /* ERR_COR Received */
/* Multi ERR_COR Received */
#define PCI_ERR_ROOT_MULTI_COR_RCV  0x00000002
/* ERR_FATAL/NONFATAL Recevied */
#define PCI_ERR_ROOT_UNCOR_RCV      0x00000004
/* Multi ERR_FATAL/NONFATAL Recevied */
#define PCI_ERR_ROOT_MULTI_UNCOR_RCV    0x00000008
#define PCI_ERR_ROOT_FIRST_FATAL    0x00000010  /* First Fatal */
#define PCI_ERR_ROOT_NONFATAL_RCV   0x00000020  /* Non-Fatal Received */
#define PCI_ERR_ROOT_FATAL_RCV      0x00000040  /* Fatal Received */
#define PCI_ERR_ROOT_COR_SRC    52
#define PCI_ERR_ROOT_SRC    54

/* Virtual Channel */
#define PCI_VC_PORT_REG1    4
#define PCI_VC_PORT_REG2    8
#define PCI_VC_PORT_CTRL    12
#define PCI_VC_PORT_STATUS  14
#define PCI_VC_RES_CAP      16
#define PCI_VC_RES_CTRL     20
#define PCI_VC_RES_STATUS   26

/* Power Budgeting */
#define PCI_PWR_DSR     4   /* Data Select Register */
#define PCI_PWR_DATA        8   /* Data Register */
#define  PCI_PWR_DATA_BASE(x)   ((x) & 0xff)        /* Base Power */
#define  PCI_PWR_DATA_SCALE(x)  (((x) >> 8) & 3)    /* Data Scale */
#define  PCI_PWR_DATA_PM_SUB(x) (((x) >> 10) & 7)   /* PM Sub State */
#define  PCI_PWR_DATA_PM_STATE(x) (((x) >> 13) & 3) /* PM State */
#define  PCI_PWR_DATA_TYPE(x)   (((x) >> 15) & 7)   /* Type */
#define  PCI_PWR_DATA_RAIL(x)   (((x) >> 18) & 7)   /* Power Rail */
#define PCI_PWR_CAP     12  /* Capability */
#define  PCI_PWR_CAP_BUDGET(x)  ((x) & 1)   /* Included in system budget */

/*
 * Hypertransport sub capability types
 *
 * Unfortunately there are both 3 bit and 5 bit capability types defined
 * in the HT spec, catering for that is a little messy. You probably don't
 * want to use these directly, just use pci_find_ht_capability() and it
 * will do the right thing for you.
 */
#define HT_3BIT_CAP_MASK    0xE0
#define HT_CAPTYPE_SLAVE    0x00    /* Slave/Primary link configuration */
#define HT_CAPTYPE_HOST     0x20    /* Host/Secondary link configuration */

#define HT_5BIT_CAP_MASK    0xF8
#define HT_CAPTYPE_IRQ      0x80    /* IRQ Configuration */
#define HT_CAPTYPE_REMAPPING_40 0xA0    /* 40 bit address remapping */
#define HT_CAPTYPE_REMAPPING_64 0xA2    /* 64 bit address remapping */
#define HT_CAPTYPE_UNITID_CLUMP 0x90    /* Unit ID clumping */
#define HT_CAPTYPE_EXTCONF  0x98    /* Extended Configuration Space Access */
#define HT_CAPTYPE_MSI_MAPPING  0xA8    /* MSI Mapping Capability */
#define  HT_MSI_FLAGS       0x02        /* Offset to flags */
#define  HT_MSI_FLAGS_ENABLE    0x1     /* Mapping enable */
#define  HT_MSI_FLAGS_FIXED 0x2     /* Fixed mapping only */
#define  HT_MSI_FIXED_ADDR  0x00000000FEE00000ULL   /* Fixed addr */
#define  HT_MSI_ADDR_LO     0x04        /* Offset to low addr bits */
#define  HT_MSI_ADDR_LO_MASK    0xFFF00000  /* Low address bit mask */
#define  HT_MSI_ADDR_HI     0x08        /* Offset to high addr bits */
#define HT_CAPTYPE_DIRECT_ROUTE 0xB0    /* Direct routing configuration */
#define HT_CAPTYPE_VCSET    0xB8    /* Virtual Channel configuration */
#define HT_CAPTYPE_ERROR_RETRY  0xC0    /* Retry on error configuration */
#define HT_CAPTYPE_GEN3     0xD0    /* Generation 3 hypertransport configuration */
#define HT_CAPTYPE_PM       0xE0    /* Hypertransport powermanagement configuration */

/* Alternative Routing-ID Interpretation */
#define PCI_ARI_CAP     0x04    /* ARI Capability Register */
#define  PCI_ARI_CAP_MFVC   0x0001  /* MFVC Function Groups Capability */
#define  PCI_ARI_CAP_ACS    0x0002  /* ACS Function Groups Capability */
#define  PCI_ARI_CAP_NFN(x) (((x) >> 8) & 0xff) /* Next Function Number */
#define PCI_ARI_CTRL        0x06    /* ARI Control Register */
#define  PCI_ARI_CTRL_MFVC  0x0001  /* MFVC Function Groups Enable */
#define  PCI_ARI_CTRL_ACS   0x0002  /* ACS Function Groups Enable */
#define  PCI_ARI_CTRL_FG(x) (((x) >> 4) & 7) /* Function Group */

/* Address Translation Service */
#define PCI_ATS_CAP     0x04    /* ATS Capability Register */
#define  PCI_ATS_CAP_QDEP(x)    ((x) & 0x1f)    /* Invalidate Queue Depth */
#define  PCI_ATS_MAX_QDEP   32  /* Max Invalidate Queue Depth */
#define PCI_ATS_CTRL        0x06    /* ATS Control Register */
#define  PCI_ATS_CTRL_ENABLE    0x8000  /* ATS Enable */
#define  PCI_ATS_CTRL_STU(x)    ((x) & 0x1f)    /* Smallest Translation Unit */
#define  PCI_ATS_MIN_STU    12  /* shift of minimum STU block */

/* Single Root I/O Virtualization */
#define PCI_SRIOV_CAP       0x04    /* SR-IOV Capabilities */
#define  PCI_SRIOV_CAP_VFM  0x01    /* VF Migration Capable */
#define  PCI_SRIOV_CAP_INTR(x)  ((x) >> 21) /* Interrupt Message Number */
#define PCI_SRIOV_CTRL      0x08    /* SR-IOV Control */
#define  PCI_SRIOV_CTRL_VFE 0x01    /* VF Enable */
#define  PCI_SRIOV_CTRL_VFM 0x02    /* VF Migration Enable */
#define  PCI_SRIOV_CTRL_INTR    0x04    /* VF Migration Interrupt Enable */
#define  PCI_SRIOV_CTRL_MSE 0x08    /* VF Memory Space Enable */
#define  PCI_SRIOV_CTRL_ARI 0x10    /* ARI Capable Hierarchy */
#define PCI_SRIOV_STATUS    0x0a    /* SR-IOV Status */
#define  PCI_SRIOV_STATUS_VFM   0x01    /* VF Migration Status */
#define PCI_SRIOV_INITIAL_VF    0x0c    /* Initial VFs */
#define PCI_SRIOV_TOTAL_VF  0x0e    /* Total VFs */
#define PCI_SRIOV_NUM_VF    0x10    /* Number of VFs */
#define PCI_SRIOV_FUNC_LINK 0x12    /* Function Dependency Link */
#define PCI_SRIOV_VF_OFFSET 0x14    /* First VF Offset */
#define PCI_SRIOV_VF_STRIDE 0x16    /* Following VF Stride */
#define PCI_SRIOV_VF_DID    0x1a    /* VF Device ID */
#define PCI_SRIOV_SUP_PGSIZE    0x1c    /* Supported Page Sizes */
#define PCI_SRIOV_SYS_PGSIZE    0x20    /* System Page Size */
#define PCI_SRIOV_BAR       0x24    /* VF BAR0 */
#define  PCI_SRIOV_NUM_BARS 6   /* Number of VF BARs */
#define PCI_SRIOV_VFM       0x3c    /* VF Migration State Array Offset*/
#define  PCI_SRIOV_VFM_BIR(x)   ((x) & 7)   /* State BIR */
#define  PCI_SRIOV_VFM_OFFSET(x) ((x) & ~7) /* State Offset */
#define  PCI_SRIOV_VFM_UA   0x0 /* Inactive.Unavailable */
#define  PCI_SRIOV_VFM_MI   0x1 /* Dormant.MigrateIn */
#define  PCI_SRIOV_VFM_MO   0x2 /* Active.MigrateOut */
#define  PCI_SRIOV_VFM_AV   0x3 /* Active.Available */

/* Masks for Various Registers */

#define PCI_IO_BASE_MASK			0xFFFFFFFC
#define PCI_MEM_BASE_MASK			0xFFFFFFF0
#define PCI_ROM_BASE_MASK			0xFFFFF800
#define PCI_FLASH_BASE_MASK         0xFFFFF000



/* typedefs */

typedef UINT16 		PCI_PFA;	/* packed PCI Function Address */

typedef enum tagPciType
{
	PCI_HOST	= 0,
	PCI_BRIDGE,
	PCI_DEVICE,
	PCI_SLOT

} PCI_TYPE;

typedef struct tagPciNodeInfo
{
	PCI_TYPE	type;

	UINT8		bBus;
	UINT8		bDev;
	UINT8		bFunc;

	UINT8		bApicId;

	UINT32		dApicMap;
	UINT32		dPirqMap;

} PCI_NODE_INFO;


/* constants */

/* locals */

/* globals */

/* externals */

extern void vPciSetHighestBus (UINT8 bBus);

extern UINT32 dPciReadReg (PCI_PFA pfa, UINT8 bReg, REG_SIZE size);

extern void vPciWriteReg (PCI_PFA pfa, UINT8 bReg, REG_SIZE size, 
						UINT32 dData);

extern UINT32 dPciExprReadReg (PCI_PFA pfa, UINT16 wReg, REG_SIZE size);

extern void vPciExprWriteReg (PCI_PFA pfa, UINT16 wReg, REG_SIZE size, 
						UINT32 dData);

extern int iPciFindDeviceById (UINT32 dInstance, UINT16 wVid, UINT16 wDid, 
						PCI_PFA* pfa);

extern int iPciFindDeviceByClass (UINT32 dInstance, UINT8 bBaseClass, 
						UINT8 bSubClass, PCI_PFA* pfa);

extern UINT32 dPciGetTopOfMem (void);
extern void vPciWriteRegExtd
(
	PCI_PFA		pfa,		/* PCI Function Address */
	UINT16		wReg,		/* valid range 0x0 - 0x0FFF */
	REG_SIZE	size,		/* register size */
	UINT32		dData
);

UINT32 dPciReadRegExtd
(
	PCI_PFA		pfa,		/* PCI Function Address */
	UINT16		wReg,		/* valid range 0x0 - 0x0FFF */
	REG_SIZE	size		/* register size */
);


/* in pciInt.c */
extern void pciIntInit( void );
int pciIntConnect( UINT8 irqNo, VOIDFUNCPTR routine, UINT32 parameter );

/* forward declarations */


#endif

