/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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

/* superio.c - board-specific superio information
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>

#include <bit/board_service.h>
#include <bit/io.h>


#define PCI_ERROR_BASE		(E__BIT + 0x6000) // all global errors are
											  // E__BIT + PCI_ERRORE_BASE + error code 1-fh


#define DRAMC_ERROR					0x10
#define PCIE0_ERROR					0x20
#define PCIE1_ERROR					0x30
#define IGD0_ERROR					0x40
#define IDT_ERROR					0x50
#define DMA0_ERROR					0x60
#define IDT8_ERROR					0x70
#define IDT12_ERROR					0x80
#define IDT16_ERROR					0x90
#define MEI_ERROR					0xA0
#define IDER_ERROR					0xB0
#define KT_ERROR					0xC0
#define EHCI1_ERROR					0xD0
#define EHCI2_ERROR					0xE0
#define RP1_ERROR				    0xF0
#define RP3_ERROR					0x100
#define RP5_ERROR					0x110
#define PI7C9X_ERROR				0x120
#define PI7C9X2_ERROR				0x130
#define	I825800_ERROR				0x140
#define	I825801_ERROR				0x150
#define	I825802_ERROR				0x160
#define	I825803_ERROR				0x170

#define TSI148_ERROR				0x180
#define PTP_ERROR				    0x190
#define LPC_ERROR				    0x1A0
#define SATA1_ERROR					0x1B0
#define SATA2_ERROR					0x1C0
#define SMBUS_ERROR					0x1D0
#define OPTDEV_ERROR				0x1E0
#define I82574_ERROR1       		0x1F0


pci_Regs   I825801Regs[]  = {{"INTRLINE", 0x3C, 0xff},
							 {NULL  , 0x00, 0x00}
							};



pci_Regs   I825679Regs[]   = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
						   };


 pci_Regs   I82574Regs[]  = {{NULL  , 0x00, 0x00}};
 
pci_Device PEP3Devices[]= {
							{"Intel 82574", PCI_DEVICE | PCI_EXPRESS|PCI_OPTIONAL, PCI_SINGLE_FUNC, 0x8086, 0x10d3, 0, 0, 0x00,
													I82574Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I82574_ERROR1},
							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
						  };

				
pci_Device PericomDevices[]= {{"Pericom Device ",  PCI_BRIDGE | PCI_DEVICE | PCI_OPTIONAL | CPCI_PARENT| PCI_EXPRESS | PCI_EXPRESS_END  , PCI_SINGLE_FUNC, 0x12d8, 0xE110, 0, 0, 0x00,
								  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

pci_Device PlXDevices[]= {	{"PLX Device 1", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL  , PCI_MULTI_FUNC, 0x10B5, 0x8624, 4, 0, 0x00,
							  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

							{"PLX Device 2",  PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL  , PCI_MULTI_FUNC, 0x10B5, 0x8624, 5, 0, 0x00,
								  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

							{"PLX Device 3", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL  , PCI_MULTI_FUNC, 0x10B5, 0x8624, 6, 0, 0x00,
																  I825679Regs, {0x00,0x00,0x00},NULL,(void *)PericomDevices ,NULL, I825801_ERROR, NULL},

							{"PLX Device 4", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL , PCI_MULTI_FUNC, 0x10B5, 0x8624, 8, 0, 0x00,
																  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

							{"PLX Device 5",PCI_BRIDGE  | PCI_EXPRESS | PCI_OPTIONAL  , PCI_MULTI_FUNC, 0x10B5, 0x8624, 9, 0, 0x00,
																  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},


							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						   };

pci_Device PlXBDevices[]= {	{"PLX Device 1", PCI_DEVICE | PCI_EXPRESS | PCI_OPTIONAL  , PCI_MULTI_FUNC, 0x10B5, 0x87D0, 0, 1, 0x00,
							  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

						{"PLX Device 2", PCI_DEVICE | PCI_EXPRESS | PCI_OPTIONAL  , PCI_MULTI_FUNC, 0x10B5, 0x87D0, 0, 2, 0x00,
							  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

						{"PLX Device 3", PCI_DEVICE | PCI_EXPRESS | PCI_OPTIONAL  , PCI_MULTI_FUNC, 0x10B5, 0x87D0, 0, 3, 0x00,
															  I825679Regs, {0x00,0x00,0x00},NULL,NULL,NULL, I825801_ERROR, NULL},

						{"PLX Device 4", PCI_DEVICE | PCI_EXPRESS | PCI_OPTIONAL , PCI_MULTI_FUNC, 0x10B5, 0x87D0, 0, 4, 0x00,
															  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

						 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						};



	pci_Device PlXcDevices[]= {	{"PLX  Bridge 1", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT  , PCI_MULTI_FUNC, 0x10B5, 0x8717, 1, 0, 0x00,
								  I825679Regs, {0x00,0x00,0x00}, NULL,(void *) NULL,NULL, I825801_ERROR, NULL},

								{"PLX  Bridge 2", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT  , PCI_MULTI_FUNC, 0x10B5, 0x8717, 2, 0, 0x00,
								  I825679Regs, {0x00,0x00,0x00}, NULL,NULL,NULL, I825801_ERROR, NULL},

								{"PLX  Bridge 3", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT , PCI_MULTI_FUNC, 0x10B5, 0x8717, 8, 0, 0x00,
								  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

								{"PLX  Bridge 4", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT , PCI_MULTI_FUNC, 0x10B5, 0x8717, 9, 0, 0x00,
																  I825679Regs, {0x00,0x00,0x00},NULL,NULL,NULL, I825801_ERROR, NULL},

								{"PLX  Bridge 5", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT , PCI_MULTI_FUNC, 0x10B5, 0x8717, 10,0, 0x00,
																  I825679Regs, {0x00,0x00,0x00}, NULL,NULL, NULL, I825801_ERROR, NULL},

							    {"PLX  Bridge 6", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT, PCI_MULTI_FUNC | PXMC_PMC1 /*| PXMC_XMC1 */, 0x10B5, 0x8717, 11, 0, 0x00,
								  I825679Regs, {0x00,0x00,0x00}, NULL,NULL/*(void *)I8624Devices*/, NULL, I825801_ERROR, NULL},


							    {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };


pci_Regs   DMA0Regs[]    = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
						   };

pci_Regs   IDT8Regs[]     = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
						   };
pci_Regs   IDT12Regs[]    = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
						   };
pci_Regs   IDT16Regs[]    = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
						   };


pci_Regs   IDTRegs[]      = {{"INTRLINE", 0x3C, 0xff},
							 {NULL  , 0x00, 0x00}
							};

pci_Regs   PCIE1Regs[]   = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
						   };

pci_Regs   I825802Regs[]  = {{"INTRLINE", 0x3C, 0xff},
							 {NULL  , 0x00, 0x00}
							};
pci_Regs   I825803Regs[]  = {{"INTRLINE", 0x3C, 0xff},
							 {NULL  , 0x00, 0x00}
							};

pci_Device I82580BDevices[]= {{"Intel 82580 controller", PCI_DEVICE  , PCI_MULTI_FUNC, 0x8086, 0x1510, 0, 1, 0x00,
							  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						   };


pci_Device I82580Devices[]= {{"Intel 82580 controller", PCI_DEVICE  , PCI_MULTI_FUNC, 0x8086, 0x1510, 0, 0, 0x00,
							  I825801Regs, {0x00,0x00,0x00},(void *)I82580BDevices, NULL,  NULL, I825801_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						   };


pci_Regs   RP3Regs[]     = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
							};
pci_Regs   RP5Regs[]     = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
							};

pci_Device PCIE0MDevices[]= {{"PCI EXPRESS-1 Port x4/x8 [IVY Bridge]", PCI_BRIDGE | PCI_EXPRESS  | PCI_OPTIONAL |PXMC_PARENT , PCI_MULTI_FUNC | PXMC_XMC1 ,
							 0x8086, 0x0155, 1, 1, 0x00, PCIE1Regs,
							 {0x00,0x00,0x00},(void*)NULL, NULL, NULL, PCIE1_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};

pci_Device PlXMDevices[]={{"PLX Bridge ", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END  , PCI_MULTI_FUNC, 0x10B5, 0x8717, 0, 0, 0x00,
							  I825679Regs, {0x00,0x00,0x00}, (void*)PlXBDevices, (void*)PlXcDevices, I825801_ERROR, NULL},
						  {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
						};


pci_Device RP1MDevices[] = {
						{"PCI EXPRESS Root Port 4 [Panther Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
						  0x8086, 0x1E18, 28, 4, 0x00, RP5Regs, {0x00,0x00,0x00}, NULL, (void*)PEP3Devices,
						   RP5_ERROR, NULL},


						{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
					   };


	pci_Regs   SATA1Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   SATA2Regs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   SMBUSRegs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Device LPCDevices[]  = {{"SATA 1 [Panther Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1E01,
								  31, 2, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR},

								{"SMBUS [Panther Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1e22,
								  31, 3, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},

								{"SATA 2 [Panther Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1e09,
								  31, 5, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA2_ERROR},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };


	pci_Regs   DRAMCRegs[]   = {{NULL  , 0x00, 0x00}
							   };
	pci_Regs   PCIE0Regs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   IGD0Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   MEIRegs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };

	pci_Regs   EthRegs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };

	pci_Regs   EHCI1Regs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   EHCI2Regs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   RP1Regs[]     = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
								};
	pci_Regs   PTPRegs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   LPCRegs[]     = {{NULL  , 0x00, 0x00}
							   };
	pci_Device pciDevices0[] = {{"DRAM Controller [IVY Bridge]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x0154,
								 0, 0, 0x00, DRAMCRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, DRAMC_ERROR, NULL},

								{"PCI EXPRESS-0 Port x8 [IVY Bridge]", PCI_BRIDGE | PCI_EXPRESS ,
								 PCI_MULTI_FUNC, 0x8086, 0x0151, 1, 0, 0x00, PCIE0Regs,
								  {0x00,0x00,0x00}, (void*)PCIE0MDevices, (void*)PlXMDevices, NULL, PCIE0_ERROR, NULL},

								{"Internal Graphics Device [IVY Bridge]", PCI_DEVICE | PCI_OPTIONAL , PCI_SINGLE_FUNC,
								  0x8086, 0x0166, 2, 0, 0x00, IGD0Regs, {0x00,0x00,0x00}, NULL, NULL,
								  NULL, IGD0_ERROR, NULL},

								{"OptDev [IVY Bridge]", PCI_DEVICE | PCI_OPTIONAL , PCI_SINGLE_FUNC,
								  0x8086, 0x0153, 4, 0, 0x00, NULL, {0x00,0x00,0x00}, NULL, NULL,
								  NULL, OPTDEV_ERROR, NULL},
								  
								  {"USB EHCI 0 [Panther Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1E31, 20, 0,
								  0x00, EHCI2Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI2_ERROR, NULL },


								{"Internal Ethernet Controller [Panther Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1502, 25, 0, 0x00,
								  NULL, {0x00,0x00,0x00}, (void*)NULL, NULL, NULL, MEI_ERROR, NULL },


								{"USB EHCI 2 [Panther Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1E2d, 26, 0,
								  0x00, EHCI2Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI2_ERROR, NULL },

								{"PCI EXPRESS Root Port 1 [Panther Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								  0x8086, 0x1E10, 28, 0, 0x00, RP1Regs, {0x00,0x00,0x00}, (void*)RP1MDevices,
								  (void*)I82580Devices, NULL, RP1_ERROR, NULL},

								{"USB EHCI 1 [Panther Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1E26, 29, 0,
								  0x00, EHCI1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR, NULL },

								{"PCI-to-PCI Bridge [Panther Point]", PCI_BRIDGE, PCI_SINGLE_FUNC, 0x8086, 0x2448,
								  30, 0, 0x00, PTPRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, PTP_ERROR, NULL },


								{"LPC Interface [Panther Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1e55, 31, 0,
								  0x00, LPCRegs, {0x00,0x00,0x00}, (void*)LPCDevices, NULL, NULL, LPC_ERROR, NULL },

								
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};


/*****************************************************************************
 * brdChkPXMC: check PMC/XMC module's presence
 * RETURNS: status
 * */
UINT8 brdChkPXMC(void)
{

	UINT8 temp, rt = 0;
	
	temp = ((UINT8)dIoReadReg(0x214, REG_8));
	
	if(temp & (1<<6)) 	 	
		rt |= PXMC_XMC1;

	return rt;
}

pci_DB  brdPCIDB = { 0x00, brdChkPXMC, NULL, NULL, pciDevices0, PCI_ERROR_BASE};


/*****************************************************************************
 * brdGetPCIDB: returns the SIO global data structure
* RETURNS: pci_DB*
* */
UINT32 brdGetPCIDB(void *ptr)
{
	*((pci_DB**)ptr) = &brdPCIDB;

	return E__OK;
}









