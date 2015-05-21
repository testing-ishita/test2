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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/amc1x.msd/pci.c,v 1.2 2014-10-15 11:47:36 mgostling Exp $
 *
 * $Log: pci.c,v $
 * Revision 1.2  2014-10-15 11:47:36  mgostling
 * Added missing entry for SATA controller in AHCI mode.
 *
 * Revision 1.1  2014-09-19 10:41:42  mgostling
 * Initial check in to CVS
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


#define DRAMC_ERROR					0x10
#define PCIE0_ERROR					0x20
#define PCIE1_ERROR					0x21
#define PCIE2_ERROR					0x22
#define HECI_ERROR					0x30
#define EHCI_2_ERROR				0x40
#define RP_0_ERROR					0x50
#define RP_3_ERROR					0x51
#define RP_4_ERROR					0x52
#define EHCI_1_ERROR				0x60
#define LPC_ERROR					0x70
#define SATA1_ERROR					0x71
#define SMBUS_ERROR					0x72
#define I_X540_1_ERROR				0x60
#define I_X540_2_ERROR				0x61
#define SRIO_1_ERROR				0x70
#define SRIO_2_ERROR				0x80
#define I_210_1_ERROR				0x90
#define I_210_2_ERROR				0xA0
#define IDT_0_ERROR					0xB0
#define IDT_2_ERROR					0xB1
#define IDT_6_ERROR					0xB2
#define IDT_8_ERROR					0xB3
#define IDT_16_ERROR				0xB4
#define TEXAS_ERROR					0xC0
#define SILICON_ERROR				0xD0
/*
#define IDT_16_ERROR					0x86
#define IDT17_ERROR					0x87
#define IDT18_ERROR					0x88
#define IDT19_ERROR					0x89
#define IDT20_ERROR					0x90
#define IDT21_ERROR					0x91
#define IDT22_ERROR					0x92
#define IDT23_ERROR					0x93
#define MEI_ERROR					0xA0
#define IDER_ERROR					0xB0
#define KT_ERROR					0xC0
#define XHCI_ERROR					0xD0
#define USBSERI_ERROR				0xF0
#define USBCTRL_ERROR				0x100
#define EHCI1_ERROR					0x100
#define RP7_ERROR					0x140
#define PI7C9X_ERROR				0x150
#define PI7C9X2_ERROR				0x151
#define PI7C9X1_ERROR				0x152
#define PI7C9X3_ERROR				0x153
#define PI7C9X4_ERROR				0x154
#define PI7C9X5_ERROR				0x155
#define PI7C9X6_ERROR				0x156
#define PI7C9X7_ERROR				0x157
#define I82579_ERROR				0x160
#define I825800_ERROR				0x161
#define	I825801_ERROR				0x170
#define	I825802_ERROR				0x180
#define I82574_ERROR1				0x190
#define TSI148_ERROR				0x1a0
#define PTP_ERROR					0x1b0
#define OPTDEV_ERROR				0x200
#define HDAUDIO_ERROR				0x210
#define PCIBr_ERROR					0x220
*/

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

/*							{"<NAME>", <PCI TYPE>, <FUNC TYPE>,
								<VENDOR ID>, <DEVICE ID>,<DEV NUM>, <FUNC NUM>, <FLAGS>,
								<Registers>, {0x00,0x00,0x00}, <MDevices>, <BDevices>,
								 <PXMC_PFA>, <ERROR_BASE>, NULL},
*/

/*12:0:0*/
pci_Device	BDevices_12_0_0[]= {
								{"Silicon VGA Controller ",   PCI_DEVICE | PCI_OPTIONAL,  PCI_MULTI_FUNC /*PCI_SINGLE_FUNC*/,
									0x126F, 0x0720, 0, 0, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, SILICON_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
							};
/*11:0:0*/
pci_Device	BDevices_11_0_0[]= {
								{"Texas PCIe-PCI Bridge ", PCI_BRIDGE | PCI_DEVICE | CPCI_PARENT | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x104C, 0x8231, 0, 0,  0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, BDevices_12_0_0,
									NULL, TEXAS_ERROR,},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
							};

/*7:0:0*/
pci_Device	BDevices_7_0_0[]= {
								{"IDT Switch ", PCI_BRIDGE | PCI_DEVICE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL, PCI_MULTI_FUNC,
									0x111D, 0x808F, 2, 0, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, IDT_2_ERROR, NULL},

								{"IDT Switch ", PCI_BRIDGE | PCI_DEVICE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x111D, 0x808F, 6, 0, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, IDT_6_ERROR, NULL},

								{"IDT Switch ", PCI_BRIDGE | PCI_DEVICE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x111D, 0x808F, 8, 0, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, IDT_8_ERROR, NULL},

								{"IDT Switch ", PCI_BRIDGE | PCI_DEVICE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x111D, 0x808F, 0x10, 0, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, BDevices_11_0_0,
									NULL, IDT_16_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
							};
/*6:0:0*/
pci_Device	BDevices_6_0_0[]= {
							{"IDT Switch Port 0", PCI_BRIDGE | PCI_DEVICE | PCI_EXPRESS| PCI_EXPRESS_END | PCI_OPTIONAL, PCI_SINGLE_FUNC,
								0x111D, 0x808F, 0, 0, 0x00,
								IntrRegs, {0x00, 0x00, 0x00}, NULL, BDevices_7_0_0,
								NULL, IDT_0_ERROR, NULL},

							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
						};

/*5:0:0*/
pci_Device	BDevices_5_0_0[]= {
							{"i210 SerDes Backplane Connection 2", PCI_DEVICE, PCI_SINGLE_FUNC,
								0x8086, 0x1537, 0, 0, 0x00,
								IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
								NULL, I_210_2_ERROR, NULL},
							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
						};

/*4:0:0*/
pci_Device	BDevices_4_0_0[]= {
							{"i210 SerDes Backplane Connection 1", PCI_DEVICE, PCI_SINGLE_FUNC,
								0x8086, 0x1537, 0, 0, 0x00,
								IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
								NULL, I_210_1_ERROR, NULL},
							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
						};

/*3:0:0*/
pci_Device	BDevices_3_0_0[]= {
								{"PCIe to SRIO Bridge 2", PCI_DEVICE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x111d, 0x80ab, 0, 0, 0x00,
									IDTRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, SRIO_2_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0x00, NULL},
							};

/*2:0:0*/
pci_Device BDevices_2_0_0[]= {
								{"PCIe to SRIO Bridge 1", PCI_DEVICE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x111d, 0x80ab, 0, 0, 0x00,
									IDTRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, SRIO_1_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0x00, NULL},
							};

/*1:0:x*/
pci_Device MDevices_1_0_0[]= {
								{"x540 10 Gigabit Network Connection 2", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x8086, 0x1528, 0, 1, 0x00,
									I825801Regs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, I_X540_2_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
							};

/*1:0:0*/
pci_Device BDevices_1_0_0[]= {
								{"x540 10 Gigabit Network Connection 1", PCI_DEVICE | PCI_OPTIONAL, PCI_MULTI_FUNC,
									0x8086, 0x1528, 0, 0, 0x00,
									I825801Regs, {0x00, 0x00, 0x00}, MDevices_1_0_0, NULL,
									NULL, I_X540_1_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
							};

/*0:1F:x*/
pci_Device	MDevices_0_1F_0[]= {
								{"Lynx Point SATA/IDE Controller 1", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x8086, 0x8C01, 0x1F, 2, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, SATA1_ERROR, NULL},

								{"Lynx Point SATA/IDE Controller 1", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x8086, 0x8C03, 0x1F, 2, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, SATA1_ERROR, NULL},

								{"Lynx Point SMBus Controller", PCI_DEVICE, PCI_SINGLE_FUNC,
									0x8086, 0x8C22, 0x1F, 3, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, SMBUS_ERROR, NULL},

								{"Lynx Point SATA/IDE Controller 2", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x8086, 0x8C09, 0x1F, 5, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
									NULL, SMBUS_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
							};

/*0:1C:x*/
pci_Device	MDevices_0_1C_0[]= {

								{"Lynx Point PCIe Root Port 3 ", PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x8086, 0x8C16, 0x1C, 3, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, BDevices_5_0_0,
									NULL, RP_3_ERROR, NULL},

								{"Lynx Point PCIe Root Port 4 ", PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x8086, 0x8C18, 0x1C, 4, 0x00,
									IntrRegs, {0x00, 0x00, 0x00}, NULL, BDevices_6_0_0,
									NULL, RP_4_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
							};

/*0:1:x*/
pci_Device	MDevices_0_1_0[]= {
								{"Haswell PCIe Root Port Function 1", PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x8086, 0x0C05, 1, 1, 0x00,
									PlaneRegs, {0x00, 0x00, 0x00}, NULL, BDevices_2_0_0,
									NULL, PCIE1_ERROR, NULL},

								{"Haswell PCIe Root Port Function 2", PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PCI_OPTIONAL, PCI_SINGLE_FUNC,
									0x8086, 0x0C09, 1, 2, 0x00,
									PlaneRegs, {0x00, 0x00, 0x00}, NULL, BDevices_3_0_0,
									NULL, PCIE2_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
							};

/*0:0:0*/
pci_Device	pciDevices0[]= {
							{"Haswell DRAM Controller", PCI_DEVICE, PCI_SINGLE_FUNC,
								0x8086, 0x0C04, 0, 0, 0x00,
								DRAMCRegs, {0x00, 0x00, 0x00}, NULL, NULL,
								NULL, DRAMC_ERROR, NULL},

							{"Haswell PCIe Root Port Function 0", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								0x8086, 0x0C01, 1, 0, 0x00,
								IntrRegs, {0x00, 0x00, 0x00}, MDevices_0_1_0, BDevices_1_0_0,
								NULL, PCIE0_ERROR, NULL},

							{"Lynx Point HECI Controller", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC,
								0x8086, 0x8C3A, 0x16, 0, 0x00,
								PlaneRegs, {0x00, 0x00, 0x00}, NULL, NULL,
								NULL, HECI_ERROR, NULL},

							{"Lynx Point USB Enhanced Host Controller #2", PCI_DEVICE | PCI_OPTIONAL, PCI_SINGLE_FUNC,
								0x8086, 0x8C2D, 0x1A, 0, 0x00,
								PlaneRegs, {0x00, 0x00, 0x00}, NULL, NULL,
								NULL, EHCI_2_ERROR, NULL},

							{"Lynx Point PCIe Root Port 0", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								0x8086, 0x8C14, 0x1C, 0, 0x00,
								IntrRegs, {0x00, 0x00, 0x00}, MDevices_0_1C_0, BDevices_4_0_0,
								NULL, RP_0_ERROR, NULL},

							{"Lynx Point USB Enhanced Host Controller #1", PCI_DEVICE, PCI_SINGLE_FUNC,
								0x8086, 0x8C26, 0x1D, 0, 0x00,
								IntrRegs, {0x00, 0x00, 0x00}, NULL, NULL,
								NULL, EHCI_1_ERROR, NULL},

							{"Lynx Point LPC Controller", PCI_DEVICE, PCI_MULTI_FUNC,
								0x8086, 0x8C4F, 0x1F, 0, 0x00,
								PlaneRegs, {0x00, 0x00, 0x00}, MDevices_0_1F_0, NULL,
								NULL, LPC_ERROR, NULL},

							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0, 0, 0}, NULL, NULL, NULL, 0, NULL},
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


