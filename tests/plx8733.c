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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/plx8733.c,v 1.1 2014-03-20 10:38:11 mgostling Exp $
 * $Log: plx8733.c,v $
 * Revision 1.1  2014-03-20 10:38:11  mgostling
 * New VPX Loopback test for ADVR3
 *
 *
 */
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <bit/io.h>
#include <bit/board_service.h>
#include <bit/interrupt.h>
#include <bit/delay.h>
#include <devices/plx8733.h>

/* defines */
//#define DEBUG

#define E__TEST_NO_MEM			E__BIT  + 0x02
#define E__BAD_PARAMETER		E__BIT  + 0x03
#define E__BAD_DMA_XFER1		E__BIT  + 0x04
#define E__BAD_DMA_XFER2		E__BIT  + 0x05
#define E__BAD_LINK_WIDTH		E__BIT  + 0x06
#define E__BAD_LINK_SPEED		E__BIT  + 0x07

#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define BYTE_DMA_BUFSIZE				0x4000							// buffer size in bytes
#define DWORD_DMA_BUFSIZE				(BYTE_DMA_BUFSIZE / 4)			// buffer size in dwords

#define LOCAL_BUFFER1					0x20000000L
#define LOCAL_BUFFER2					LOCAL_BUFFER1 + BYTE_DMA_BUFSIZE

/* locals */

static PTR48 tPtr1, tPtr2, tPtr3;
static UINT32 mHandle1, mHandle2, mHandle3;

const static UINT8  switchDmaChannel[5] =
{
	SWITCH_PORT_DMA0,
	SWITCH_PORT_DMA1,
	SWITCH_PORT_DMA2,
	SWITCH_PORT_DMA3,
	SWITCH_PORT_DMA_RAM
};

static RTM_PCIE_SWITCH_INFO * switchInfo;
static SWITCH_BRIDGE_CONFIG switchConfig [MAX_SWITCH_PORTS];
static UINT8	numFoundBridges;

#ifdef DEBUG
char buffer[128];
#endif

/* forward declarations */

UINT32 plx8733GetRegister (addr_t cfgBaseAddr, UINT8 portNum, UINT16 regAddr, UINT32 * regData);
UINT32 plx8733SetRegister (addr_t cfgBaseAddr, UINT8 portNum, UINT16 regAddr, UINT32 regData);
UINT32 pex8733Init (RTM_PEX_8733_DEV_CTRL *p8733Dev, UINT8 dInstance, UINT16 wVendorId, UINT16 wDeviceId);
UINT32 pex8733SavePortConfigInfo (RTM_PEX_8733_DEV_CTRL *p8733Dev);
void pex8733RestorePortRoutingInfo (void);
void pex8733ChangeNtPort (RTM_PEX_8733_DEV_CTRL *p8733Dev, RTM_PCIE_SWITCH_PORT_INFO *vpxLink);
void pex8733restoreInitialConfig (RTM_PEX_8733_DEV_CTRL *p8733Dev);
UINT32 pex8733TestLink (RTM_PEX_8733_DEV_CTRL *p8733Dev, RTM_PCIE_SWITCH_PORT_INFO *vpxLink);
void pex8733HotResetSwitches (RTM_PEX_8733_DEV_CTRL *p8733Dev);
UINT32 pex8733ConfigureNtPort (RTM_PEX_8733_DEV_CTRL *p8733Dev, RTM_PCIE_SWITCH_PORT_INFO *vpxLink);
UINT32 pex8733DmaInit(RTM_PEX_8733_DEV_CTRL *p8733Dev);
UINT32 pex8733DmaTransfer(RTM_PEX_8733_DEV_CTRL *p8733Dev, UINT32 dSource, UINT32 dDest, UINT32 dSize);


/****************************************************************
 *
 * plx8733GetRegister - read a register from the switch
 *
 */
UINT32 plx8733GetRegister
(
	addr_t				plx8733PortCfgBaseAddr,
	UINT8				portNum,
	UINT16 				regAddr,
	UINT32 *			regData
)
{
	UINT32				status;
	UINT32				cfgAddr;			// pcie memory mapped register address
	UINT8				port;				// port address conversion
	UINT32				addr;				// pcie address offset for port

	status = E__OK;
	*regData = 0xFFFFFFFFL;

	// set correct memory offset for required port
	cfgAddr = (UINT32) plx8733PortCfgBaseAddr;
	switch (portNum)
	{
		case SWITCH_PORT_NT0_VIRTUAL:
			addr = PCIE_NT0_VIRTUAL_BASE;
			break;

		case SWITCH_PORT_NT0_LINK:
			addr = PCIE_NT0_LINK_BASE;
			break;

		case SWITCH_PORT_DMA0:
		case SWITCH_PORT_DMA1:
		case SWITCH_PORT_DMA2:
		case SWITCH_PORT_DMA3:
			port = portNum - SWITCH_PORT_DMA0;
			addr = PCIE_DMA_BASE_ADDR(port);
			break;

		case SWITCH_PORT_DMA_RAM:
			addr = PCIE_DMA_RAM_BASE;
			break;

		default:
			addr = PCIE_REG_BASE_ADDR(portNum);
			break;
	}

	cfgAddr += addr;
	cfgAddr += regAddr;

	*regData = vReadDWord (cfgAddr);

	return status;
}

/****************************************************************
 *
 * plx8733SetRegister - write to switch register
 *
 */
UINT32 plx8733SetRegister
(
	addr_t				plx8733PortCfgBaseAddr,
	UINT8				portNum,
	UINT16 				regAddr,
	UINT32				regData
)
{
	UINT32				status;
	UINT32				cfgAddr;			// pcie memory mapped register address
	UINT8				port;				// port address conversion
	UINT32				addr;				// pcie address offset for port

	status = E__OK;

	// set correct memory offset for required port
	cfgAddr = (UINT32) plx8733PortCfgBaseAddr;
	switch (portNum)
	{
		case SWITCH_PORT_NT0_VIRTUAL:
			addr = PCIE_NT0_VIRTUAL_BASE;
			break;

		case SWITCH_PORT_NT0_LINK:
			addr = PCIE_NT0_LINK_BASE;
			break;

		case SWITCH_PORT_DMA0:
		case SWITCH_PORT_DMA1:
		case SWITCH_PORT_DMA2:
		case SWITCH_PORT_DMA3:
			port = portNum - SWITCH_PORT_DMA0;
			addr = PCIE_DMA_BASE_ADDR(port);
			break;

		case SWITCH_PORT_DMA_RAM:
			addr = PCIE_DMA_RAM_BASE;
			break;

		default:
			addr = PCIE_REG_BASE_ADDR(portNum);
			break;
	}

	cfgAddr += addr;
	cfgAddr += regAddr;

	vWriteDWord (cfgAddr, regData);

	return status;
}

/****************************************************************
 *
 * pex8733SavePortConfigInfo 
 *
 * save bus routing details for all switches PEX8733 switches
 * save NT device memory allocation and bridge memory window
 */
UINT32 pex8733SavePortConfigInfo(RTM_PEX_8733_DEV_CTRL *p8733Dev)
{
	UINT32	rt = E__OK;
	PCI_PFA	pfa;
	UINT8	cnt;

	// save bus routing information and memory allocation from all bridges
	for (cnt = 0; cnt < MAX_SWITCH_PORTS; ++cnt)
	{
		if (E__OK == iPciFindDeviceByClass (cnt + 1, 0x06, 0x04, &pfa))
		{
			switchConfig[cnt].bridgePfa = pfa;
			switchConfig[cnt].busInfo = PCI_READ_DWORD(pfa, PCI_PRI_BUS);
			switchConfig[cnt].memAlloc = PCI_READ_DWORD(pfa, PCI_MEM_BASE);
			switchConfig[cnt].bar0 = PCI_READ_DWORD(pfa, PCI_BAR0);
			switchConfig[cnt].bar1 = PCI_READ_DWORD(pfa, PCI_BAR1);
		}
		else
		{
			if (cnt == 0)
			{
				rt = E__DEVICE_NOT_FOUND;
			}
			break;
		}
	}

	// number of bridges found
	numFoundBridges = cnt;

	return rt;
}

/****************************************************************
 *
 * pex8733RestorePortRoutingInfo - restore bus routing details for all PEX8733 switches
 *
 */
void pex8733RestorePortRoutingInfo(void)
{
	UINT8	cnt;

	for (cnt = 0; cnt < numFoundBridges; ++cnt)
	{
		// enable memory & IO interfaces. Restore bus routing, BAR Allocations and downstream Memory window
		PCI_WRITE_DWORD(switchConfig[cnt].bridgePfa, PCI_PRI_BUS, switchConfig[cnt].busInfo);
		PCI_WRITE_DWORD(switchConfig[cnt].bridgePfa, PCI_MEM_BASE, switchConfig[cnt].memAlloc);
		PCI_WRITE_DWORD(switchConfig[cnt].bridgePfa, PCI_BAR0, switchConfig[cnt].bar0);
		PCI_WRITE_DWORD(switchConfig[cnt].bridgePfa, PCI_BAR1, switchConfig[cnt].bar1);
		PCI_WRITE_WORD(switchConfig[cnt].bridgePfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);
	}
}

/****************************************************************
 *
 * pex8733HotResetSwitches - send hot reset to host switch and RTM switch
 *
 */
void pex8733HotResetSwitches (RTM_PEX_8733_DEV_CTRL *p8733Dev)
{
	UINT16	wTemp;
	UINT32	dReg;

	// send Hot Reset to all downstream ports of RTM Switch
	wTemp = PCI_READ_WORD (p8733Dev->rtmPfa, PCI_BRIDGE_CTRL);
	wTemp |= 0x40;

	PCI_WRITE_WORD (p8733Dev->rtmPfa,PCI_BRIDGE_CTRL, wTemp);
	vDelay(1);
	wTemp &= ~0x40;
	PCI_WRITE_WORD (p8733Dev->rtmPfa,PCI_BRIDGE_CTRL, wTemp);
	vDelay(100);

#ifdef DEBUG
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_STATION_0_1_LANE_STATUS, &dReg);
	sprintf(buffer,"RTM Lane Status 0x330                                         : %08X\n", dReg);
	vConsoleWrite(buffer);
#endif

	// do not reset the RTM switch if it is connected to the host switch
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VIRTUAL_SWITCH_DEBUG, &dReg);
	dReg |= RESET_PROPOGATION_DISABLE;
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VIRTUAL_SWITCH_DEBUG, dReg);

	// send Hot Reset to all downstream ports of host Switch
	wTemp = PCI_READ_WORD (p8733Dev->hostPfa, PCI_BRIDGE_CTRL);
	wTemp |= 0x40;
	PCI_WRITE_WORD (p8733Dev->hostPfa,PCI_BRIDGE_CTRL, wTemp);
	vDelay(1);
	wTemp &= ~0x40;
	PCI_WRITE_WORD (p8733Dev->hostPfa,PCI_BRIDGE_CTRL, wTemp);
	vDelay(100);

	// enable reset of RTM switch again
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VIRTUAL_SWITCH_DEBUG, &dReg);
	dReg &= ~RESET_PROPOGATION_DISABLE;
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VIRTUAL_SWITCH_DEBUG, dReg);

	// send Hot Reset to NT Link device
	wTemp = PCI_READ_WORD (p8733Dev->hostPortPfa, PCI_BRIDGE_CTRL);
	wTemp |= 0x40;
	PCI_WRITE_WORD (p8733Dev->hostPortPfa,PCI_BRIDGE_CTRL, wTemp);
	vDelay(1);
	wTemp &= ~0x40;
	PCI_WRITE_WORD (p8733Dev->hostPortPfa,PCI_BRIDGE_CTRL, wTemp);
	vDelay(100);
}

/****************************************************************
 *
 * pex8733restoreInitialConfig - restore switch to original configuration
 *
 */
void pex8733restoreInitialConfig (RTM_PEX_8733_DEV_CTRL *p8733Dev)
{
	UINT32	dReg;

	// restore RTM port 1 to NT mode
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VS0_UPSTREAM, &dReg);
	dReg &= VS0_UPSTREAM_NTPORT_MASK;
	dReg |= VS0_UPSTREAM_NT_ENABLE + (1 << 8);
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VS0_UPSTREAM, dReg);

	// send hot reset to switches now that RTM NT port has changed
	pex8733HotResetSwitches(p8733Dev);

	// restore the PCI bridge bus and memory allocations
	pex8733RestorePortRoutingInfo();

	// restore bridge memory allocation
	PCI_WRITE_DWORD (p8733Dev->initRtmTrPortPfa, PCI_MEM_BASE, p8733Dev->initBridgeMemAlloc);
	PCI_WRITE_DWORD (p8733Dev->initRtmTrPortPfa, PCI_BAR0, p8733Dev->initBridgeBar0);
	PCI_WRITE_DWORD (p8733Dev->initRtmTrPortPfa, PCI_BAR1, p8733Dev->initBridgeBar1);
	PCI_WRITE_WORD (p8733Dev->initRtmTrPortPfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);

	// restore NT BAR memory allocation
	PCI_WRITE_DWORD (p8733Dev->initRtmNtPortPfa, PCI_BAR0, p8733Dev->initNtBar0);
	PCI_WRITE_DWORD (p8733Dev->initRtmNtPortPfa, PCI_BAR2, p8733Dev->initNtBar2);
	PCI_WRITE_WORD (p8733Dev->initRtmNtPortPfa, PCI_COMMAND, 0);
}

/****************************************************************
 *
 * pex8733Init - Obtain information about and initialise root port of RTM switch
 *
 */
UINT32 pex8733Init (RTM_PEX_8733_DEV_CTRL *p8733Dev, UINT8 dInstance, UINT16 wVendorId, UINT16 wDeviceId)
{
	PCI_PFA pfa;
	UINT32 dCsrAddress;
	UINT32 dReg;

	if (E__OK == iPciFindDeviceById (dInstance, wVendorId, wDeviceId ,&pfa))
	{
		p8733Dev->rtmPfa = pfa;
		p8733Dev->rtmDmaPfa = pfa + 1;			// function 1 of upstream port of switch

		PCI_WRITE_WORD (pfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);
		dCsrAddress = PCI_READ_DWORD (pfa, PCI_BAR0);

#ifdef DEBUG
		sprintf(buffer,"RTM switch CsrAddress                                         : %x\n", dCsrAddress);
		vConsoleWrite(buffer);
#endif
		mHandle2  = dGetPhysPtr(dCsrAddress, PCI_CONFIG_SPACE, &tPtr2, (void*)(&p8733Dev->rtmMappedCsr));
		if( mHandle2 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}

		// unlock access to the configuration registers - register not defined in data book!
		plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_UNLOCK_CONFIG, &dReg);
		dReg |= UNLOCK_CONFIG_LOCKBIT;
		plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_UNLOCK_CONFIG, dReg);

		// add entries into RID LUT for RTM switch DMA controller and CPU
		dReg = 0x00010001 + (((UINT32) PCI_PFA_BUS(pfa)) << 24);
		plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_NT0_VIRTUAL, REG_NT0_VIRTUAL_RID_LUT, dReg);

		// switch eeprom configures port 1 of RTM switch as NT mode with BAR2 set to 1MB
		p8733Dev->initRtmTrPortPfa = PCI_MAKE_PFA(PCI_READ_BYTE(p8733Dev->rtmPfa, PCI_SEC_BUS), 1, 0);
		p8733Dev->initRtmNtPortPfa = PCI_MAKE_PFA(PCI_READ_BYTE(p8733Dev->initRtmTrPortPfa, PCI_SEC_BUS), 0, 0);

		// save initial bridge memory allocation 
		p8733Dev->initBridgeMemAlloc = PCI_READ_DWORD(p8733Dev->initRtmTrPortPfa, PCI_MEM_BASE);
		p8733Dev->initBridgeBar0 = PCI_READ_DWORD(p8733Dev->initRtmTrPortPfa, PCI_BAR0);
		p8733Dev->initBridgeBar1 = PCI_READ_DWORD(p8733Dev->initRtmTrPortPfa, PCI_BAR1);

		// save NT BAR0 and BAR2 memory allocation
		p8733Dev->initNtBar0 = PCI_READ_DWORD(p8733Dev->initRtmNtPortPfa, PCI_BAR0);
		p8733Dev->initNtBar2 = PCI_READ_DWORD(p8733Dev->initRtmNtPortPfa, PCI_BAR2);

		// the NT Virtual device doesn't get enabled by default
		PCI_WRITE_WORD (p8733Dev->initRtmNtPortPfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);

		mHandle3  = dGetPhysPtr(p8733Dev->initNtBar2, BYTE_DMA_BUFSIZE, &tPtr3, (void*)(&p8733Dev->rtmMappedNtBar2));
		if( mHandle3 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}
	}
	else
	{
		return (E__DEVICE_NOT_FOUND);
	}

	return (E__OK);
}

/****************************************************************
 *
 * pex8733ChangeNtPort - set a port into NT mode
 *
 */
void pex8733ChangeNtPort (RTM_PEX_8733_DEV_CTRL *p8733Dev, RTM_PCIE_SWITCH_PORT_INFO *vpxLink)
{
	UINT32	dReg;
	UINT8	bPort;

	bPort = vpxLink->rtmPort;

	// put RTM port into NT mode
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VS0_UPSTREAM, &dReg);
	dReg &= VS0_UPSTREAM_NTPORT_MASK;
	dReg |= (((UINT32)bPort) << 8) | VS0_UPSTREAM_NT_ENABLE;
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VS0_UPSTREAM, dReg);

#ifdef DEBUG
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_0, REG_VS0_UPSTREAM, &dReg);
	sprintf(buffer,"VS0 Upstream 0x360                                            : %08X\n", dReg);
	vConsoleWrite(buffer);
#endif
	// get the PFAs of the switch port connected to the NT-Link port and of the NT-Link port itself
	p8733Dev->rtmTrPortPfa = PCI_MAKE_PFA(PCI_READ_BYTE(p8733Dev->rtmPfa, PCI_SEC_BUS), vpxLink->rtmPort, 0);
	p8733Dev->rtmNtPortPfa = PCI_MAKE_PFA(PCI_READ_BYTE(p8733Dev->rtmTrPortPfa, PCI_SEC_BUS), 0, 0);

}

/****************************************************************
 *
 * pex8733ConfigureNtPort - Configure ports for NT operation
 *
 */
UINT32 pex8733ConfigureNtPort(RTM_PEX_8733_DEV_CTRL *p8733Dev, RTM_PCIE_SWITCH_PORT_INFO *vpxLink)
{
	UINT32	rt = E__OK;
	UINT32	dReg;
	UINT32	dCap;

	// stop original bridge from decoding resources
	PCI_WRITE_WORD (p8733Dev->initRtmTrPortPfa, PCI_COMMAND, 0);
	
	// set new bridge memory allocation
	PCI_WRITE_DWORD (p8733Dev->rtmTrPortPfa, PCI_MEM_BASE, p8733Dev->initBridgeMemAlloc);
	PCI_WRITE_DWORD (p8733Dev->rtmTrPortPfa, PCI_BAR0, p8733Dev->initBridgeBar0);
	PCI_WRITE_DWORD (p8733Dev->rtmTrPortPfa, PCI_BAR1, p8733Dev->initBridgeBar1);
	PCI_WRITE_WORD (p8733Dev->rtmTrPortPfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);

	// set new Virtual NT device memory allocation
	PCI_WRITE_DWORD (p8733Dev->rtmNtPortPfa, PCI_BAR0, p8733Dev->initNtBar0);
	PCI_WRITE_DWORD (p8733Dev->rtmNtPortPfa, PCI_BAR2, p8733Dev->initNtBar2);
	PCI_WRITE_WORD (p8733Dev->rtmNtPortPfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);

	// obtain PFAs for the NT Link device and the host downstream port connected to the NT Link device
	p8733Dev->hostPortPfa = PCI_MAKE_PFA(PCI_READ_BYTE(p8733Dev->hostPfa, PCI_SEC_BUS), vpxLink->linkPort, 0);
	PCI_WRITE_WORD (p8733Dev->hostPortPfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);

	p8733Dev->hostNtLinkPfa = PCI_MAKE_PFA(PCI_READ_BYTE(p8733Dev->hostPortPfa, PCI_SEC_BUS), 0, 0);
	PCI_WRITE_WORD (p8733Dev->hostNtLinkPfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);

#ifdef DEBUG
	sprintf(buffer,"Upstream port of switch for NT Link [b:d:f]                   : %02X:%02X:%02X\n",
			 PCI_PFA_BUS(p8733Dev->hostPfa), PCI_PFA_DEV(p8733Dev->hostPfa), PCI_PFA_FUNC(p8733Dev->hostPfa));
	vConsoleWrite(buffer);
	sprintf(buffer,"Downstream port of switch for NT Link [b:d:f]                 : %02X:%02X:%02X\n",
			 PCI_PFA_BUS(p8733Dev->hostPortPfa), PCI_PFA_DEV(p8733Dev->hostPortPfa), PCI_PFA_FUNC(p8733Dev->hostPortPfa));
	vConsoleWrite(buffer);
	sprintf(buffer,"NT Link device of RTM switch [b:d:f]                          : %02X:%02X:%02X\n",
			 PCI_PFA_BUS(p8733Dev->hostNtLinkPfa), PCI_PFA_DEV(p8733Dev->hostNtLinkPfa), PCI_PFA_FUNC(p8733Dev->hostNtLinkPfa));
	vConsoleWrite(buffer);
	sprintf(buffer,"new RTM bridge [b:d:f]                                        : %02X:%02X:%02X\n",
			 PCI_PFA_BUS(p8733Dev->rtmTrPortPfa), PCI_PFA_DEV(p8733Dev->rtmTrPortPfa), PCI_PFA_FUNC(p8733Dev->rtmTrPortPfa));
	vConsoleWrite(buffer);
	dReg = PCI_READ_DWORD(p8733Dev->rtmTrPortPfa, PCI_MEM_BASE);
	sprintf(buffer,"new bridge memory allocation                                  : 0x%X \n", dReg);
	vConsoleWrite(buffer);
	dReg = PCI_READ_DWORD (p8733Dev->rtmNtPortPfa, 0);			// vendor & device IDs
	sprintf(buffer,"new virtual NT Port Id                                        : 0x%X \n", dReg);
	vConsoleWrite(buffer);
	sprintf(buffer,"new NT Port [b:d:f]                                           : %02X:%02X:%02X\n",
			 PCI_PFA_BUS(p8733Dev->rtmNtPortPfa), PCI_PFA_DEV(p8733Dev->rtmNtPortPfa), PCI_PFA_FUNC(p8733Dev->rtmNtPortPfa));
	vConsoleWrite(buffer);
	dReg = PCI_READ_DWORD(p8733Dev->rtmNtPortPfa, PCI_BAR2);
	sprintf(buffer,"new NT Port BAR2                                              : 0x%X \n", dReg);
	vConsoleWrite(buffer);
#endif

	dCap = PCI_READ_DWORD (p8733Dev->rtmNtPortPfa,REG_LINK_CAPABILITY);
	dReg = PCI_READ_DWORD (p8733Dev->rtmNtPortPfa,REG_LINK_STATUS);
	
#ifdef DEBUG
	sprintf(buffer,"NT Link speed capability                                      : %08X\n", dCap);
	vConsoleWrite(buffer);
	sprintf(buffer,"NT Link speed status                                          : %08X\n", dReg);
	vConsoleWrite(buffer);
#endif

	// check for valid link speed and link widths
	if ((dCap & LINK_MAX_WIDTH_MASK)  != ((dReg & LINK_CURRENT_WIDTH_MASK) >> 16))
		rt = E__BAD_LINK_WIDTH + (vpxLink->rtmPort << 8);
	else if ((dCap & LINK_MAX_SPEED_MASK)  != ((dReg & LINK_CURRENT_SPEED_MASK) >> 16))
		rt = E__BAD_LINK_SPEED + (vpxLink->rtmPort << 8);

	return rt;
}


/****************************************************************
 *
 * pex8733DmaInit - Initialise DMA controller
 *
 */

UINT32 pex8733DmaInit(RTM_PEX_8733_DEV_CTRL *p8733Dev)
{
	UINT32 dReg;

	// sanity check!
	dReg = PCI_READ_DWORD(p8733Dev->rtmDmaPfa, PCI_VENDOR_ID);
	if (dReg != 0x87D010B5)
	{
		return (E__DEVICE_NOT_FOUND);
	}

	// enable bus master, memory and I/O access
	dReg = PCI_READ_DWORD (p8733Dev->rtmDmaPfa, PCI_COMMAND);
	PCI_WRITE_DWORD (p8733Dev->rtmDmaPfa, PCI_COMMAND, (dReg | PCI_BMEN | PCI_MEMEN | PCI_IOEN));

	// configure a single DMA channel only
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_GLOBAL_CONTROL, 1L);

	return E__OK;
}

/****************************************************************
 *
 * pex8733DmaTransfer - transfer a block of memory using DMA engine in RTM switch
 *
 */

UINT32 pex8733DmaTransfer(RTM_PEX_8733_DEV_CTRL *p8733Dev, UINT32 dSource, UINT32 dDest, UINT32 dSize)
{
	int		cnt;
	UINT32	dReg;
	UINT32	rt = E__OK;

	// DMA transfer in block mode
#ifdef DEBUG
	sprintf(buffer,"DMA 0x%04X bytes from 0x%08X to 0x%08X\n", dSize, dSource, dDest);
	vConsoleWrite(buffer);
#endif
	
	// set the source register
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_SRC_ADDRESS_LOWER, dSource);
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_SRC_ADDRESS_UPPER, 0);

	// set the destination register
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_DEST_ADDRESS_LOWER, dDest);
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_DEST_ADDRESS_UPPER, 0);

	// set the size register
	dReg = dSize | DMA_DESC_VALID;
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_TRANSFER_SIZE, dReg);

	// 'gracefully pause' the  DMA controller 
	dReg = DMA_GRACEFUL_PAUSE | DMA_START | DMA_DESC_RING_STOP | DMA_GRACEFUL_PAUSE_DONE | DMA_ABORT_DONE;
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_CONTROL_STATUS, dReg);
	vDelay(150);

#ifdef DEBUG
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_CONTROL_STATUS, &dReg);
	sprintf(buffer,"DMA Control/Status (GracefulP)                                : %08X\n", dReg);
	vConsoleWrite(buffer);
#endif

	// now start the DMA transfer
	dReg = DMA_START | DMA_TRAN_SIZE_128B | DMA_GRACEFUL_PAUSE_DONE | DMA_ABORT_DONE;
	plx8733SetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_CONTROL_STATUS, dReg);
	vDelay(150);

	// wait for DMA transfer to complete
	cnt = 5;
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_CONTROL_STATUS, &dReg);
	while (((dReg & DMA_IN_PROGRESS) != 0) && (cnt > 0))
	{
		plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_CONTROL_STATUS, &dReg);
		vDelay(150);
		--cnt;
	}

#ifdef DEBUG
	plx8733GetRegister(p8733Dev->rtmMappedCsr, SWITCH_PORT_DMA0, REG_DMA_CONTROL_STATUS, &dReg);
	sprintf(buffer,"DMA Control/Status (DMA started)                              : %08X\n", dReg);
	vConsoleWrite(buffer);
#endif

	return rt;
}

/****************************************************************
 *
 * pex8733TestLink - Test a single VPX loopback link
 *
 */
UINT32 pex8733TestLink (RTM_PEX_8733_DEV_CTRL *p8733Dev, RTM_PCIE_SWITCH_PORT_INFO *vpxLink)
{
	UINT32	rt = E__OK;
	UINT32 * dSrc;
	UINT32 * dDest;
	UINT16	wCnt;

	// find the upstream port of the switch connected to the NT Link device
	rt = iPciFindDeviceById (vpxLink->hostInstance, vpxLink->hostVid, vpxLink->hostDid, &p8733Dev->hostPfa);
	if (rt ==E__OK)
	{
#ifdef DEBUG
		sprintf(buffer,"host switch [b:d:f]                                           : %02X:%02X:%02X\n",
				 PCI_PFA_BUS(p8733Dev->hostPfa), PCI_PFA_DEV(p8733Dev->hostPfa), PCI_PFA_FUNC(p8733Dev->hostPfa));
		vConsoleWrite(buffer);
#endif

		// enable required NT port
		pex8733ChangeNtPort (p8733Dev, vpxLink);

		// send hot reset to switches now connected with NT port
		pex8733HotResetSwitches(p8733Dev);

		// restore the PCI bridge bus allocation 
		pex8733RestorePortRoutingInfo();

		// configure NT Virtual port under test using information obtained when the board booted into CUTE
		rt = pex8733ConfigureNtPort(p8733Dev, vpxLink);

		if (rt == E__OK)
		{
			// flush both 'local' buffers for DMA transfer
			memset(p8733Dev->rtmMappedBuffer, 0, BYTE_DMA_BUFSIZE * 2);
        
			// fill local buffer 1 with value for first DMA transfer
			memset(p8733Dev->rtmMappedBuffer, vpxLink->rtmPort, BYTE_DMA_BUFSIZE);

#ifdef DEBUG
			dSrc = (UINT32 *) p8733Dev->rtmMappedBuffer;
			sprintf(buffer,"DMA test pattern                                              : %08X\n", *dSrc);
			vConsoleWrite(buffer);
#endif
			rt = pex8733DmaInit(p8733Dev);
    
			if (rt == E__OK)
			{
				// run DMA transfer 1 - copy from local buffer 1 to memory at Virtual NT BAR2
				// use physical memory addresses and not mapped memory addresses
				rt = pex8733DmaTransfer(p8733Dev, LOCAL_BUFFER1, p8733Dev->initNtBar2, BYTE_DMA_BUFSIZE);
     
				if (rt == E__OK)
				{
					// get mapped address of NT BAR2 memory window and 'local' buffer
					dSrc = (UINT32 *) p8733Dev->rtmMappedBuffer;
					dDest = (UINT32 *) p8733Dev->rtmMappedNtBar2;
		
					// check DMA transfer was successful
					for (wCnt = 0; wCnt < DWORD_DMA_BUFSIZE; ++wCnt, ++dSrc, ++dDest)
					{
						if (*dDest != *dSrc)
						{
							rt = E__BAD_DMA_XFER1 + (vpxLink->rtmPort << 8);
							break;
						}
					}
				}
			}
    
			if (rt == E__OK)
			{
				// run DMA transfer 2 - copy from memory from Virtual NT BAR2 to local buffer 2
				// use physical memory addresses and not mapped memory addresses
				rt = pex8733DmaTransfer(p8733Dev, p8733Dev->initNtBar2, LOCAL_BUFFER2, BYTE_DMA_BUFSIZE);
    
				if (rt == E__OK)
				{
					// get mapped address of NT BAR2 memory window and 'local' buffer
					dSrc = (UINT32 *) p8733Dev->rtmMappedBuffer;
					dDest = (UINT32 *) (p8733Dev->rtmMappedBuffer + DWORD_DMA_BUFSIZE);
					
					// check DMA transfer was successful
					for (wCnt = 0; wCnt < DWORD_DMA_BUFSIZE; ++wCnt, ++dSrc, ++dDest)
					{
						if (*dDest != *dSrc)
						{
							rt = E__BAD_DMA_XFER2 + (vpxLink->rtmPort << 8);
							break;
						}
					}
				}
			}
		}
	}

	return rt;
}

/****************************************************************
 *
 * vpx_pex8733_loopback - Test all VPX ports
 *
 */
TEST_INTERFACE (PciePex8733Loopback, "PCIe VPX Interface Loopback Test")
{
	RTM_PEX_8733_DEV_CTRL s8733Dev;
	UINT8	cnt;
    int		bInstance;
	UINT32	rt = E__OK;

	// Get switch port info for the VRE1x and RTM
	board_service(SERVICE__BRD_GET_RTM_PCIE_SWITCH_INFO, &rt, &switchInfo);

    if(adTestParams[0] != 0)
	{
        bInstance = adTestParams[1];
		if ((bInstance < 1) || (bInstance > switchInfo->rtmPorts))
		{
			vConsoleWrite("Invalid test parameter\n");
			return E__BAD_PARAMETER;
		}
	}
	else
		bInstance = 0;


	memset (&s8733Dev, 0, sizeof(s8733Dev));

	// 'local buffer' for DMA transfers
	mHandle1  = dGetPhysPtr(LOCAL_BUFFER1, BYTE_DMA_BUFSIZE * 2, &tPtr1, (void*)(&s8733Dev.rtmMappedBuffer));
	if( mHandle1 == E__FAIL)
	{
		return (E__TEST_NO_MEM);
	}

	// enable access to configuration registers of RTM switch
	// work out PFAs of all relevant devices on the PCI bus
	rt = pex8733Init(&s8733Dev, switchInfo->rtmSwitchInstance, switchInfo->rtmHostVid, switchInfo->rtmHostDid);

	// save initial port routing information as it gets wiped out by Hot Reset
	// obtain initial memory allocations for NT port for re-use by other ports
	if (rt == E__OK)
		rt = pex8733SavePortConfigInfo(&s8733Dev);

	if (rt == E__OK) 
	{
		// test the links by performing DMA transfers
		if (bInstance != 0)
		{
			// test a single link
			rt = pex8733TestLink(&s8733Dev, &switchInfo->rtmPortInfo[bInstance - 1]);
		}
		else
		{
			// 7 ports to test - 8th port is in downstream port chain
			for (cnt = 0; cnt < switchInfo->rtmPorts; ++cnt)
			{
				rt = pex8733TestLink(&s8733Dev, &switchInfo->rtmPortInfo[cnt]);
				if (rt != E__OK)
					break;
			}
		}
	}

	pex8733restoreInitialConfig (&s8733Dev);

	if (mHandle1 != E__FAIL)
		vFreePtr(mHandle1);

	if (mHandle2 != E__FAIL)
		vFreePtr(mHandle2);

	if (mHandle3 != E__FAIL)
		vFreePtr(mHandle3);

	return (rt);
}

