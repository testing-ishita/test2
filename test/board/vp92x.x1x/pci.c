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

/* pci.c - board-specific superio information
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/pci.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: pci.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
 * Import files into new source repository.
 *
 * Revision 1.3  2012/08/08 12:28:48  madhukn
 * *** empty log message ***
 *
 * Revision 1.2  2012/08/07 03:27:40  madhukn
 * *** empty log message ***
 *
 * Revision 1.1  2012/06/22 11:42:00  madhukn
 * Initial release
 *
 * Revision 1.2  2012/02/20 14:49:28  hchalla
 * Added new macros for CPCI pericom bridge, and Added more checks for XMC1 and XMC2 fitted for PCI related tests.
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>
#include <bit/io.h>
#include "cctboard.h"

#define GP_LVL          0x0C
#define GPIO_BASE	    0x48
#define LPC_BRIDGE_DEV	31

#define AHCI_MODE   0x06
#define IDE_MODE     0x01
#define RAID_MODE  0x04

#define PCI_ERROR_BASE		(E__BIT + 0x6000) // all global errors are 
											  // E__BIT + PCI_ERRORE_BASE + error code 1-fh


	#define DRAMC_ERROR				0x10
	#define PCIE0_ERROR				0x20
	#define PCIE1_ERROR				0x30
    #define PCIEP11_ERROR			0x31
    #define PCIEP12_ERROR			0x32
	#define IGD0_ERROR					0x40
	#define IDT_ERROR					0x50
	#define DMA0_ERROR				0x60
    #define IDT4_ERROR					0x6D
	#define IDT8_ERROR					0x70
	#define IDT12_ERROR				0x85
	#define IDT16_ERROR				0x86
	#define IDT17_ERROR				0x87
	#define IDT18_ERROR				0x88
	#define IDT19_ERROR				0x89
    #define IDT20_ERROR				0x90
	#define IDT21_ERROR				0x91
	#define IDT22_ERROR				0x92
	#define IDT23_ERROR				0x93
	#define MEI_ERROR					0xA0
	#define IDER_ERROR					0xB0
	#define KT_ERROR					    0xC0
	#define XHCI_ERROR			  	   0xD0
	#define HECI_ERROR				   0xE0
	#define USBSERI_ERROR		   0xF0
    #define USBCTRL_ERROR		   0x100
    #define EHCI1_ERROR		       0x100

	#define RP1_ERROR				    0x110
	#define RP3_ERROR					0x120
	#define RP5_ERROR					0x130
	#define RP7_ERROR					0x140
	
	#define PI7C9X_ERROR				0x150
	#define PI7C9X2_ERROR			0x151


   #define I82579_ERROR               0x160
   #define I825800_ERROR             0x161
	#define	I825801_ERROR			0x170
	#define	I825802_ERROR			0x180
	#define I82574_ERROR1       		0x190
	#define TSI148_ERROR				0x1a0
	#define PTP_ERROR				    0x1b0
	#define LPC_ERROR				    0x1c0
	#define SATA1_ERROR				0x1d0
	#define SATA2_ERROR				0x1e0
	#define SMBUS_ERROR				0x1f0
	#define OPTDEV_ERROR			0x200
  #define HDAUDIO_ERROR            0x210
  #define PCIBr_ERROR					0x220



	pci_Regs IntrRegs[]	= {
							{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
						  };
  
	pci_Regs PlaneRegs[]= {
							{NULL  , 0x00, 0x00}
						  };

	pci_Regs DRAMCRegs[]= {
							{NULL  , 0x00, 0x00}
						  };


	pci_Device	MDevices[]= {
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	pci_Device	BDevices[]= {
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};



	pci_Device	BDevices_8_0_0[]= {
								{"Tsi148 [Tempe]", PCI_DEVICE, PCI_SINGLE_FUNC, 
									0x10E3, 0x0148, 4, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, TSI148_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	
	pci_Device	BDevices_0_1C_0[]= {
								{"PCI Express to PCI-XPI7C9X130 PCI-X Bridge ", PCI_BRIDGE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC|PXMC_PMC1|PXMC_PMC2|PXMC_PARENT, 
									0x12D8, 0xE130, 0, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, BDevices_8_0_0, 
									NULL, PI7C9X_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};

	pci_Device	MDevices_A_0_0[]= {
								{"82580 Gigabit Network connection 2", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x150E, 0, 1, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, I825802_ERROR, NULL},

								{"82580 Gigabit Network connection 3", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x150E, 0, 2, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, I825802_ERROR, NULL},

								{"82580 Gigabit Network connection 4", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x150E, 0, 3, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, I825802_ERROR, NULL},
									
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	
	pci_Device	BDevices_0_1C_4[]= {
								{"82580 Gigabit Network Connection", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x150E, 0, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, MDevices_A_0_0, NULL, 
									NULL, I825801_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	
	pci_Device PCIBDevices[] = {{"PCI Bridge", PCI_BRIDGE | PCI_OPTIONAL | PCI_CARRIER , PCI_SINGLE_FUNC ,
							0x8086, 0xb152, 14, 0, 0, PlaneRegs, {0,0,0}, NULL, NULL, NULL, PCIBr_ERROR, NULL},
							
							{"PCI Bridge", PCI_BRIDGE | PCI_OPTIONAL | PCI_CARRIER , PCI_SINGLE_FUNC ,
							0x1011, 0x0024, 14, 0, 0, PlaneRegs, {0,0,0}, NULL, NULL, NULL, PCIBr_ERROR, NULL},

							{"PCI Bridge", PCI_BRIDGE | PCI_OPTIONAL | PCI_CARRIER , PCI_SINGLE_FUNC ,
							 0x12d8, 0x8152, 14, 0, 0, PlaneRegs, {0,0,0}, NULL, NULL, NULL, PCIBr_ERROR, NULL},

							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						   };

	pci_Device	BDevices_0_1C_6[]= {
								{"PI7C9X110 PCI Express to PCI bridge", PCI_BRIDGE|PCI_EXPRESS, PCI_SINGLE_FUNC, 
									0x12D8, 0xE110, 0, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, (void*)PCIBDevices, 
									NULL, PI7C9X2_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};

	pci_Device	MDevices_0_1C_0[]= {
							{"Panther Point PCI Express Root Port 5 ", PCI_BRIDGE|PCI_EXPRESS, PCI_SINGLE_FUNC, 
									0x8086, 0x1E18, 0x1C, 4, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, BDevices_0_1C_4, 
									NULL, RP5_ERROR, NULL},
							{"Panther Point PCI Express Root Port 7", PCI_BRIDGE|PCI_EXPRESS|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E1C, 0x1C, 6, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, BDevices_0_1C_6, 
									NULL, RP7_ERROR, NULL},
							{"Panther Point PCI Express Root Port 8", PCI_BRIDGE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E1E, 0x1C, 7, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};

	pci_Device	MDevices_0_1F_0[]= {
								{"Panther Point 4 port SATA IDE Controller", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x1E01, 0x1F, 2, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, SATA1_ERROR, NULL},
								{"Panther Point SMBus Controller", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x1E22, 0x1F, 3, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, SMBUS_ERROR, NULL},
								{"Unknown optional device", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 
									0x8086, 0x1E23, 0x1F, 4, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{"Panther Point 2 port SATA IDE Controller", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x1E09, 0x1F, 5, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, SATA2_ERROR, NULL},
								{"Panther Point Thermal Management Controller", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 
									0x8086, 0x1E24, 0x1F, 6, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	

	
	/*							{"<NAME>", <PCI TYPE>, <FUNC TYPE>, 
								<VENDOR ID>, <DEVICE ID>,<DEV NUM>, <FUNC NUM>, <FLAGS>, 
								<Registers>, {0x00,0x00,0x00}, <MDevices>, <BDevices>, 
								 <PXMC_PFA>, <ERROR_BASE>, NULL},*/
	pci_Device	MDevices_0_16_0[]= {
							{"Panther Point HECI Controller #2", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 
									0x8086, 0x1E3B, 0x16, 1, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
							{"Panther Point IDE-r Controller", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 
									0x8086, 0x1E3C, 0x16, 2, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
							{"Panther Point KT Controller", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 
									0x8086, 0x1E3D, 0x16, 3, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	pci_Device	MDevices_0_1_0[]= {		{"Ivy Bridge PCI Express Root Port 2n Function", PCI_BRIDGE|PCI_EXPRESS|PCI_OPTIONAL| PXMC_PARENT, PCI_MULTI_FUNC|PXMC_XMC1/*|PXMC_XMC2 |*/|PXMC_OPTIONAL, 
									0x8086, 0x0155, 1, 1, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	
	pci_Device	pciDevices0[]= {
								{"Ivy Bridge DRAM Controller", PCI_DEVICE, PCI_SINGLE_FUNC, 
									0x8086, 0x0154, 0, 0, 0x00, 
									DRAMCRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, DRAMC_ERROR, NULL},
								{"Ivy Bridge PCI Express Root Port-1", PCI_BRIDGE|PCI_EXPRESS| PXMC_PARENT, PCI_MULTI_FUNC|PXMC_XMC2 /*|PXMC_XMC1|*/|PXMC_OPTIONAL, 
									0x8086, 0x0151, 1, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, MDevices_0_1_0, NULL, 
									NULL, PCIE0_ERROR, NULL},
									
								{"Ivy Bridge Graphics Controller", PCI_DEVICE, PCI_SINGLE_FUNC, 
									0x8086, 0x0166, 2, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, IGD0_ERROR, NULL},
								{"Unknown Optional Device", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x0153, 4, 0, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{"USB xHCI Host Controller[Optional]", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E31, 0x14, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{"Panther Point HECI Controller #1", PCI_DEVICE, PCI_SINGLE_FUNC, 
									0x8086, 0x1E3A, 0x16, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, MDevices_0_16_0, NULL, 
									NULL, HECI_ERROR, NULL},
								/*{"82579LM Gigabit Network Connection", PCI_DEVICE, PCI_SINGLE_FUNC, 
									0x8086, 0x1502, 0x19, 0, 0x00, 
									NULL, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, I82579_ERROR, NULL},*/
								{"Panther Point USB Enhanced Host Controller #2", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E2D, 0x1A, 0, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{"Panther Point High Definition Audio Controller", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E20, 0x1B, 0, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{"Panther Point PCI Express Root Port 1", PCI_BRIDGE|PCI_EXPRESS, PCI_MULTI_FUNC, 
									0x8086, 0x1E10, 0x1C, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, MDevices_0_1C_0, BDevices_0_1C_0, 
									NULL, RP1_ERROR, NULL},
								{"Panther Point USB Enhanced Host Controller #1", PCI_DEVICE, PCI_SINGLE_FUNC, 
									0x8086, 0x1E26, 0x1D, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, EHCI1_ERROR, NULL},
								{"Panther Point LPC Controller", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x1E55, 0x1F, 0, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, MDevices_0_1F_0, NULL, 
									NULL, LPC_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};	

	


	/*****************************************************************************
	 * brdChkPXMC: check PMC/XMC module's presence
	 * RETURNS: status
	 * */
/*****************************************************************************
 * brdChkPXMC: check PMC/XMC module's presence
 * RETURNS: status
 * */
UINT8 brdChkPXMC(void)
{
	UINT8 temp, rt = 0;

	temp = ((UINT8)dIoReadReg(0x212, REG_8));
	
	if(temp & 0x01)
		rt |= PXMC_PMC1;

	if(temp & 0x02)
		rt |= PXMC_PMC2;

	temp = ((UINT8)dIoReadReg(0x21D, REG_8));

	if(temp & 0x01)
		rt |= PXMC_XMC1;

	if(temp & 0x04)
		rt |= PXMC_XMC2;

	return rt;
}




/*****************************************************************************
 * brdChkPeripheral: check peripheral mode and AD CR5 PMC module's presence
 * RETURNS: status
 * */
UINT8 brdChkPeripheral(void)
{
	UINT32 reg32;
	UINT16 reg;
	UINT8 rt;
	PCI_PFA	pfa;
	UINT16  gpioBase=0x500;
	//Read GPIO 48 to check the presence of AD CR5 PMC module

	pfa = PCI_MAKE_PFA (0, 31, 0);
	gpioBase = dPciReadReg(pfa, 0x48, REG_32);		
	gpioBase &= 0xFF80;
	//GPIO48: 0 -> Module present  1 -> Module Absent
	//Read GPIO pins 48 for Second Slot board interface
	reg32 = dIoReadReg(gpioBase+0x38,REG_32);//GP_LVL[63:32]
	reg = (reg32>>16)&1;
	if((reg&1) == 0)
	{
		rt = 1;
	}
	else
	{
		rt = 0;
	}
	return rt;
}


/*UINT8 brdChkSatellite(void)
{
		UINT8 temp, rt = 0;

		temp = ((UINT8)dIoReadReg(0x214, REG_8));

		if(temp & FORCE_SAT)
			rt = 0;
		else
			rt = 1;

		return rt;
}*/



	pci_DB  brdPCIDB = { 0x00, brdChkPXMC, NULL, brdChkPeripheral, pciDevices0, PCI_ERROR_BASE};

	/*****************************************************************************
	 * brdGetPCIDB: returns the SIO global data structure
 	* RETURNS: pci_DB*
 	* */
	UINT32 brdGetPCIDB(void *ptr)
	{
		*((pci_DB**)ptr) = &brdPCIDB;

		return E__OK;
	}


