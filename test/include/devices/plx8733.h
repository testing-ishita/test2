/************************************************************************
 *                                                                      *
 *      Copyright 2010 Concurrent Technologies, all rights reserved.    *
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

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/plx8733.h,v 1.1 2014-03-20 10:38:11 mgostling Exp $
*
* $Log: plx8733.h,v $
* Revision 1.1  2014-03-20 10:38:11  mgostling
* New VPX Loopback test for ADVR3
*
*
*/

typedef void * addr_t; 


#define PEX8733_VENDOR			0x10b5
#define PEX8733_DEVICE			0x8733
#define PEX8733_DMA_DEVICE		0x87D0
#define PEX8733_NT_VIRTUAL		0x87B0
#define PEX8733_NT_LINK			0x87A0

#define PCI_CONFIG_SPACE			256 * 1024

#define PCIE_REG_BASE_ADDR(port)			(port * 0x1000)
#define PCIE_DMA_BASE_ADDR(dmaChannel)		((dmaChannel * 0x1000) + 0x00021000L)
#define NT_VIRTUAL_BAR_REG_ADDR(bar)		(0xD4 + (bar * 4))
#define NT_LINK_BAR_REG_ADDR(bar)			(0xE8 + (bar * 4))
#define NT_BAR_TX_BASE_ADDR(bar)			(0xC3C + (bar * 4))

#define	PCIE_NT0_LINK_BASE			0x0003F000L
#define	PCIE_NT0_VIRTUAL_BASE		0x0003E000L
#define	PCIE_NT1_LINK_BASE			0x0003D000L
#define	PCIE_NT1_VIRTUAL_BASE		0x0003C000L

#define	PCIE_A_LUT0_BASE			0x00038000L
#define	PCIE_A_LUT1_BASE			0x00039000L
#define	PCIE_A_LUT2_BASE			0x0003A000L
#define	PCIE_A_LUT3_BASE			0x0003B000L

#define	PCIE_DMA_RAM_BASE			0x00020000L
#define	PCIE_DMA0_BASE				0x00021000L
#define	PCIE_DMA1_BASE				0x00022000L
#define	PCIE_DMA2_BASE				0x00023000L
#define	PCIE_DMA3_BASE				0x00024000L

#define SWITCH_PORT_0				0
#define SWITCH_PORT_NT0_LINK		0x10				// 16
#define SWITCH_PORT_NT0_VIRTUAL		0x11				// 17
#define SWITCH_PORT_DMA0			0x12				// 18
#define SWITCH_PORT_DMA1			0x13				// 19
#define SWITCH_PORT_DMA2			0x14				// 20
#define SWITCH_PORT_DMA3			0x15				// 21
#define SWITCH_PORT_DMA_RAM			0x16				// 22

// DMA registers
#define	REG_DMA_GLOBAL_CONTROL			0x1F8
#define	REG_DMA_CHANNEL_ASSOCIATION		0x1FC
#define REG_DMA_SRC_ADDRESS_LOWER		0x200
#define REG_DMA_SRC_ADDRESS_UPPER		0x204
#define REG_DMA_DEST_ADDRESS_LOWER		0x208
#define REG_DMA_DEST_ADDRESS_UPPER		0x20C

#define REG_DMA_TRANSFER_SIZE			0x210
#define 	DMA_DESC_VALID				0x80000000L

#define	REG_DMA_CONTROL_STATUS			0x238
#define		DMA_GRACEFUL_PAUSE			0x00000001L
#define 	DMA_ABORT					0x00000002L
#define 	DMA_COMP_STA_WRITE_BACK		0x00000004L
#define		DMA_START					0x00000008L
#define 	DMA_DESC_RING_STOP			0x00000010L
#define		ON_CHIP_DESCRIPTOR_MODE		0x00000020L
#define		OFF_CHIP_DESCRIPTOR_MODE	0x00000040L
#define		DMA_DESCRIPTOR_INVALID		0x00000100L
#define		DMA_GRACEFUL_PAUSE_DONE		0x00000200L
#define		DMA_ABORT_DONE				0x00000400L
#define		DMA_IMMED_PAUSE_DONE		0x00001000L
#define 	DMA_TRAN_SIZE_128B			0x00010000L
#define 	DMA_IN_PROGRESS				0x40000000L

// SWITCH registers

// Lane Sustus (lane up or down)
#define REG_STATION_0_1_LANE_STATUS		0x330

// this register is not defined in the data book!
#define	REG_UNLOCK_CONFIG				0x350
#define		UNLOCK_CONFIG_LOCKBIT		0x00000200L

// PORT configuration
// VS0 Upstream register
#define REG_VS0_UPSTREAM				0x360
#define 	VS0_UPSTREAM_NTPORT_MASK	0xFFFF00FFL
#define 	VS0_UPSTREAM_NT_DISABLE		0x00001A00L
#define 	VS0_UPSTREAM_NT_ENABLE		0x00002000L

#define REG_VIRTUAL_SWITCH_DEBUG		0xA30
#define		RESET_PROPOGATION_DISABLE	0x00000010L

// NT registers
#define REG_LINK_CAPABILITY				0x74
#define		LINK_MAX_SPEED_MASK			0x0000000FL
#define		LINK_MAX_WIDTH_MASK			0x000003F0L
#define REG_LINK_STATUS					0x78
#define		LINK_CURRENT_SPEED_MASK		0x000F0000L
#define		LINK_CURRENT_WIDTH_MASK		0x03F00000L

#define REG_NT0_VIRTUAL_RID_LUT			0xD94
#define REG_INGRESS_CONTROL				0xF60
#define		INGRESS_CONTROL_NO_NT_ROM	0x00008000L

#define PCI_BAR_ALIGN_1MB				0xFFF00000L

#define MAX_SWITCH_PORTS				40

typedef struct pex8733
{
	PCI_PFA rtmPfa;						/* PFA of root port of RTM PEX8733 */
	PCI_PFA rtmDmaPfa;					/* PFA of DMA channel 0 of RTM PEX8733 */
	PCI_PFA hostPfa;					/* PFA of upstream port of switch connected to NT Link port of RTM PEX8733 */
	PCI_PFA hostPortPfa;				/* PFA of downstream port of switch connected to NT Link port of RTM PEX8733 */
	PCI_PFA hostNtLinkPfa;				/* PFA of NT Link port of RTM PEX8733 */
	PCI_PFA rtmTrPortPfa;				/* PFA of TR port connected to virtual NT device in RTM switch */
	PCI_PFA rtmNtPortPfa;				/* PFA of virtual NT device seen by RTM switch */
	PCI_PFA initRtmTrPortPfa;			/* initial PFA of TR port connected to virtual NT device in RTM switch */
	PCI_PFA initRtmNtPortPfa;			/* initial PFA of virtual NT device in RTM switch */
	UINT32	initBridgeMemAlloc;			/* initial memory allocated through bridge for downstream virtual NT device */
	UINT32	initBridgeBar0;				/* initial BAR0 of bridge for downstream virtual NT device */
	UINT32	initBridgeBar1;				/* initial BAR1 of bridge for downstream virtual NT device */
	UINT32	initNtBar0;					/* initial value of BAR 0 of Virtual NT device configured during PCI initialisation */
	UINT32	initNtBar2;					/* initial value of BAR 2 of Virtual NT device configured during PCI initialisation */
	addr_t rtmMappedCsr;				/* pointer to config space for RTM PEX8733 switch */
	addr_t rtmMappedNtBar2;				/* pointer to Virtual NT device memory window */
	addr_t rtmMappedBuffer;				/* pointer to buffer memory */
 } RTM_PEX_8733_DEV_CTRL;
 
typedef struct pex8733bridgeConfig
{
	PCI_PFA	bridgePfa;					/* PFA of bridge*/
	UINT32	busInfo;					/* bus routing information */
	UINT32	memAlloc;					/* memory allocated downstream */
	UINT32	bar0;						/* memory allocated to bridge */
	UINT32	bar1;						/* memory allocated to bridge */
 } SWITCH_BRIDGE_CONFIG;

