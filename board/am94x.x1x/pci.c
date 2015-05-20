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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/am94x.x1x/pci.c,v 1.2 2014-03-06 13:29:05 hchalla Exp $
 *
 * $Log: pci.c,v $
 * Revision 1.2  2014-03-06 13:29:05  hchalla
 * All files re-checked in due to formatting issue with windows, now converted to
 * unix format for cvs checkin.
 *
 * Revision 1.1  2014-01-29 14:06:58  hchalla
 * Initial checkin for AM 94x SRIO testing.
 *
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

		#define PI7C9X1_ERROR				0x152
		
		#define PI7C9X3_ERROR				0x153
		
		#define PI7C9X4_ERROR				0x154
		
		#define PI7C9X5_ERROR				0x155
		
		#define PI7C9X6_ERROR				0x156
		
		#define PI7C9X7_ERROR				0x157

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

	pci_Regs   IDTRegs[]      = {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							    };

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
	pci_Regs I825801Regs[]  = {
								{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							};

	pci_Device	MDevices[]= {
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	pci_Device	BDevices[]= {
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	
	pci_Device MDevices_2_0_0[]= {{"Intel 82580 serdes Backplane2", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC, 0x8086, 0x1510, 0, 1, 0x00,
								  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},

								  {"Intel 82580 Copper Backplane2", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC, 0x8086, 0x150E, 0, 2, 0x00,
								  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},

								  {"Intel 82580 Copper Backplane2", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC, 0x8086, 0x150E, 0, 3, 0x00,
								  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	/*3:0:0*/
	
	pci_Device	BDevices_0_1_2[]= {{"Intel 82580 serdes Backplane1", PCI_DEVICE , PCI_MULTI_FUNC, 0x8086, 0x1510, 0, 0, 0x00,
								  I825801Regs, {0x00,0x00,0x00},(void *)MDevices_2_0_0, NULL,  NULL, I825801_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };
	
	pci_Device	BDevices_8_0_0[]= {
								{"Tsi148 [Tempe]", PCI_DEVICE, PCI_SINGLE_FUNC, 
									0x10E3, 0x0148, 4, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, TSI148_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	/*5:0:0*/
	pci_Device	BDevices_4_1_0[]= {

									{"PI7C9X130 ",   PCI_BRIDGE | PCI_DEVICE | PCI_EXPRESS|CPCI_PARENT |PCI_OPTIONAL, PCI_SINGLE_FUNC,
								     0x12d8, 0xe130, 0, 0,  0x00, IntrRegs, {0x00,0x00,0x00}, NULL,NULL,
								     NULL, PI7C9X_ERROR,},

									{"PERICOM ", PCI_BRIDGE|PCI_DEVICE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
										0x12D8, 0xE110, 0, 0, 0x00, 
										IntrRegs, {0x00,0x00,0x00}, NULL, NULL,NULL,
										 PI7C9X_ERROR, NULL},
									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};

	/*4:1:0*/
	pci_Device	BDevices_3_0_0[]= {
									{"IDT Switch ", PCI_BRIDGE|PCI_DEVICE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_MULTI_FUNC, 
										0x111D, 0x808F, 1, 0, 0x00, 
										IntrRegs, {0x00,0x00,0x00}, NULL, BDevices_4_1_0,NULL, PI7C9X3_ERROR, NULL},
										
									{"IDT Switch ", PCI_BRIDGE|PCI_DEVICE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
										0x111D, 0x808F, 2, 0, 0x00,IntrRegs, {0x00,0x00,0x00}, NULL, NULL,NULL, PI7C9X4_ERROR, NULL},
									{"IDT Switch ", PCI_BRIDGE|PCI_DEVICE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
										0x111D, 0x808F, 6, 0, 0x00, 
										IntrRegs, {0x00,0x00,0x00}, NULL, NULL,NULL,
										 PI7C9X7_ERROR, NULL},
									{"IDT Switch ", PCI_BRIDGE|PCI_DEVICE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
										0x111D, 0x808F, 8, 0, 0x00, 
										IntrRegs, {0x00,0x00,0x00}, NULL, NULL,
										NULL, PI7C9X5_ERROR, NULL},
									{"IDT Switch ", PCI_BRIDGE|PCI_DEVICE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
										0x111D, 0x808F, 0x10, 0, 0x00, 
										IntrRegs, {0x00,0x00,0x00}, NULL, NULL,
										NULL, PI7C9X6_ERROR, NULL},
										
									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};
	
	pci_Device	BDevices_0_1C_0[]= {
								{"IDT Switch ", PCI_BRIDGE|PCI_DEVICE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_MULTI_FUNC, 
									0x111D, 0x808F, 0, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, BDevices_3_0_0,
									NULL, PI7C9X_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};

	pci_Device	MDevices_A_0_0[]= {
								{"82580 Gigabit Network Connection 2", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x150E, 0, 1, 0x00, 
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
	


	pci_Device	MDevices_0_1C_0[]= {
		
							{"Panther Point PCI Express Root Port 2 ", PCI_BRIDGE|PCI_EXPRESS|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E14, 0x1C, 2, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, RP5_ERROR, NULL},
							
							{"Panther Point PCI Express Root Port 4 ", PCI_BRIDGE|PCI_EXPRESS|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E18, 0x1C, 4, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, /*BDevices_0_1C_4,*/NULL, 
									NULL, RP5_ERROR, NULL},
																																																																																																					
							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};


	
	pci_Device PCIE0BDevices[]= {{"PCIe to SRIO Bridge 0", PCI_DEVICE | PCI_EXPRESS  | PCI_OPTIONAL | PCI_EXPRESS_END,
									  PCI_MULTI_FUNC, 0x111d, 0x80ab, 0, 0, 0x00, IDTRegs, {0x00,0x00,0x00},
									  NULL, NULL, NULL, IDT_ERROR, NULL},
	
									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};
	/*1:0:0*/								
	pci_Device PCIE0B1Devices[]= {{"PCIe to SRIO Bridge 1", PCI_DEVICE | PCI_EXPRESS | PCI_OPTIONAL  | PCI_EXPRESS_END,
									  PCI_MULTI_FUNC, 0x111d, 0x80ab, 0, 0, 0x00, IDTRegs, {0x00,0x00,0x00},
									  NULL, NULL, NULL, IDT_ERROR, NULL},
	
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
								{"Panther Point 2 port SATA IDE Controller", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 
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
	
	pci_Device	MDevices_0_1_0[]= {							
							{"Ivy Bridge PCI Express Root Port 2n Function", PCI_BRIDGE|PCI_EXPRESS|PCI_OPTIONAL|PCI_EXPRESS_END, PCI_SINGLE_FUNC, 
									0x8086, 0x0155, 1, 1, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, (void*)PCIE0BDevices, 
									NULL, OPTDEV_ERROR, NULL},
								{"Ivy Bridge PCI Express Root Port 3n Function", PCI_BRIDGE|PCI_EXPRESS, PCI_SINGLE_FUNC, 
									0x8086, 0x0159, 1, 2, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, BDevices_0_1_2, 
									NULL, OPTDEV_ERROR, NULL},		
								
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
	
	pci_Device	pciDevices0[]= {
								{"Ivy Bridge DRAM Controller", PCI_DEVICE, PCI_SINGLE_FUNC, 
									0x8086, 0x0154, 0, 0, 0x00, 
									DRAMCRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, DRAMC_ERROR, NULL},
								{"Ivy Bridge PCI Express Root Port", PCI_BRIDGE|PCI_EXPRESS, PCI_MULTI_FUNC, 
									0x8086, 0x0151, 1, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, MDevices_0_1_0, (void*)PCIE0B1Devices, 
									NULL, PCIE0_ERROR, NULL},
								/*{"Ivy Bridge Graphics Controller", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x0166, 2, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, IGD0_ERROR, NULL},*/
									
								{" Optional Device", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x0153, 4, 0, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								{"USB xHCI Host Controller[Optional]", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E31, 0x14, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, OPTDEV_ERROR, NULL},
								
								/*{"82579LM Gigabit Network Connection", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1502, 0x19, 0, 0x00, 
									NULL, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, I82579_ERROR, NULL},*/
									
								{"Panther Point USB Enhanced Host Controller #2", PCI_DEVICE|PCI_OPTIONAL, PCI_SINGLE_FUNC, 
									0x8086, 0x1E2D, 0x1A, 0, 0x00, 
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

								{"Panther Point USB Enhanced Host Controller #2", PCI_BRIDGE|PCI_EXPRESS|PCI_DEVICE|PCI_OPTIONAL|PCI_EXPRESS_END, PCI_SINGLE_FUNC, 
									0x8086, 0x2448, 0x1E, 0, 0x00, 
									IntrRegs, {0x00,0x00,0x00}, NULL, NULL, 
									NULL, EHCI1_ERROR, NULL},
									
								{"Panther Point LPC Controller", PCI_DEVICE, PCI_MULTI_FUNC, 
									0x8086, 0x1E55, 0x1F, 0, 0x00, 
									PlaneRegs, {0x00,0x00,0x00}, MDevices_0_1F_0, NULL, 
									NULL, LPC_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};	

	


	


	pci_DB  brdPCIDB = { 0x00, NULL, NULL, NULL, pciDevices0, PCI_ERROR_BASE};

	/*****************************************************************************
	 * brdGetPCIDB: returns the SIO global data structure
 	* RETURNS: pci_DB*
 	* */
	UINT32 brdGetPCIDB(void *ptr)
	{
		*((pci_DB**)ptr) = &brdPCIDB;

		return E__OK;
	}


