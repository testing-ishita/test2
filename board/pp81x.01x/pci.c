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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/pp81x.01x/pci.c,v 1.1 2013-09-04 07:13:50 chippisley Exp $
 *
 * $Log: pci.c,v $
 * Revision 1.1  2013-09-04 07:13:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2012/04/12 12:32:45  hchalla
 * Updated the pci device list to display all required PCI device link widths properly.
 *
 * Revision 1.3  2012/02/21 11:08:30  hchalla
 * Added satellite mode bridge for pci test.
 *
 * Revision 1.2  2012/02/20 17:37:33  hchalla
 * Change to the index (from 7 to 8) of the LPC Devices pciDevices0.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>
#include <bit/io.h>
#include "cctboard.h"


#define AHCI_MODE   0x06
#define IDE_MODE     0x01
#define RAID_MODE  0x04

#define GP_LVL          0x0C
#define GPIO_BASE	    0x48
#define LPC_BRIDGE_DEV	31

#define PCI_ERROR_BASE		(E__BIT + 0x6000) // all global errors are 
											  // E__BIT + PCI_ERRORE_BASE + error code 1-fh


	#define DRAMC_ERROR					0x10
	#define PCIE0_ERROR					0x20
	#define PCIE1_ERROR					0x30
    #define PCIEP11_ERROR					0x31
    #define PCIEP12_ERROR					0x32
	#define IGD0_ERROR					0x40
	#define IDT_ERROR					0x50
	#define DMA0_ERROR					0x60
    #define IDT4_ERROR					0x6D
	#define IDT8_ERROR					0x70
	#define IDT12_ERROR					0x85
	#define IDT16_ERROR					0x86
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
	#define EHCI1_ERROR					0xD0
	#define EHCI2_ERROR					0xE0
	#define RP1_ERROR				    0xF0
	#define RP3_ERROR					0x100
	#define RP5_ERROR					0x110
	#define PI7C9X_ERROR				0x120
	#define PI7C9X2_ERROR			0x130
   #define PI7C9X3_ERROR			0x131
	#define	I825800_ERROR			0x140
	#define	I825801_ERROR			0x150
	#define I82579_ERROR              0x160
	#define	I825803_ERROR			0x170

	#define TSI148_ERROR				0x180
	#define PTP_ERROR				    0x190
	#define LPC_ERROR				    0x1A0
	#define SATA1_ERROR					0x1B0
	#define SATA2_ERROR					0x1C0
	#define SMBUS_ERROR					0x1D0
	#define OPTDEV_ERROR				0x1E0

	pci_Regs   DMA0Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };

	pci_Regs   PCIE1Regs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };

	pci_Regs   PI7C9XRegs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};

	pci_Regs   IDTRegs[]      = {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							    };

	pci_Regs   PI7C9X3Regs[]= {{NULL  , 0x00, 0x00}
								  };
	pci_Device PCIE0BDevices[]= {
			                                                {"PI7C9X130 PMC2", PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PCI_OPTIONAL | PXMC_PARENT,
											                PCI_SINGLE_FUNC | PXMC_PMC2 , 0x12d8, 0xe130, 0, 0,  0x00, PI7C9X3Regs, {0x00,0x00,0x00}, NULL, NULL,
										                    NULL, PI7C9X3_ERROR},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};

	pci_Device PCIE0BDevicesDummy[]= {
				                                                {"Unknown", PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PCI_OPTIONAL | PXMC_PARENT,
												                PCI_SINGLE_FUNC | PXMC_PMC1 , 0x111d, 0x8061, 0, 0,  0x00, PI7C9X3Regs, {0x00,0x00,0x00}, NULL, NULL,
											                    NULL, PI7C9X3_ERROR},
									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};


	pci_Regs   IDERRegs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   KTRegs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Device MEIDevices[]  = {{"IDER [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c3c,
								  22, 2, 0x00, IDERRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, IDER_ERROR, NULL},

								{"KT [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c3d,
								  22, 3, 0x00, KTRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, KT_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };






	pci_Regs   I825801Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							    };
	pci_Regs   I825802Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							    };
	pci_Regs   I825803Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							    };
	pci_Device I82580Devices[]= {{"Intel 82580 copper", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x150E, 0, 1, 0x00,
								  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	pci_Regs   I825800Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							    };
	pci_Device RP1BDevices[]  = {{"Intel 82580 copper", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x150E, 0, 0, 0x00,
								I825800Regs, {0x00,0x00,0x00}, (void*)I82580Devices, NULL, NULL, I825800_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	pci_Device I82580FibreDevices[]= {{"Intel 82580 Fibre", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x1510, 0, 1, 0x00,
								  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	pci_Device RP5BFibreDevices[]  = {{"Intel 82580 Fibre", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x1510, 0, 0, 0x00,
								I825800Regs, {0x00,0x00,0x00}, (void*)I82580FibreDevices, NULL, NULL, I825800_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	pci_Device I82580CuDevices[]= {{"Intel 82580 Copper", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x150E, 0, 1, 0x00,
								  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},
								  {"Intel 82580 Fibre", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x1510, 0,1, 0x00,
								  I825800Regs, {0x00,0x00,0x00}, (void*)NULL, NULL, NULL, I825801_ERROR, NULL},
								  {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };


	pci_Device RP5BCuFibreDevices[]  = {{"Intel 82580 Fibre", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x1510, 0, 0, 0x00,
								I825800Regs, {0x00,0x00,0x00}, (void*)I82580CuDevices, NULL, NULL, I825800_ERROR, NULL},
								{"Intel 82580 Copper", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x150E, 0, 0, 0x00,
								 I825800Regs, {0x00,0x00,0x00}, (void*)I82580CuDevices, NULL, NULL, I825800_ERROR, NULL},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };


	pci_Regs   TSI148Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Device PI7C9XDevices[]= {{"TSI148", PCI_DEVICE, PCI_SINGLE_FUNC, 0x10e3, 0x0148, 4, 0, 0x00,
								 TSI148Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, TSI148_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
								};


	pci_Regs   PI7C9X2Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};

	pci_Regs   RP3Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Regs   RP5Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
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
	pci_Device LPCDevices_IDE[]  = {
								{"SATA 1 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c01,
								 31, 2, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR},
								{"SMBUS [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c22,
								  31, 3, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},
								{"SATA 2 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c09,
								  31, 5, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA2_ERROR},
								 /*{"Intel Thermal Sensor Device", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c24,
								  31, 6, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},*/

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };

	pci_Device LPCDevices_AHCI[]  = {
								{"SATA 1 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c03,
								 31, 2, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR},
								{"SMBUS [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c22,
								  31, 3, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},
								{"SATA 2 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c09,
								  31, 5, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA2_ERROR},
								 /*{"Intel Thermal Sensor Device", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c24,
								  31, 6, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},*/

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };

	pci_Device LPCDevices_RAID[]  = {
									{"SATA 1 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x282a,
									 31, 2, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR},
									{"SMBUS [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c22,
									  31, 3, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},
									{"SATA 2 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c09,
									  31, 5, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA2_ERROR},
									 /*{"Intel Thermal Sensor Device", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c24,
									  31, 6, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},*/

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
								   };




	pci_Device PEP11Devices[]= {
							    {"PI7C9X130 PMC1", PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PCI_OPTIONAL | PXMC_PARENT,
							    PCI_SINGLE_FUNC | PXMC_PMC1 , 0x12d8, 0xe130, 0, 0,  0x00, PI7C9X3Regs, {0x00,0x00,0x00}, NULL, NULL,
							     NULL, PI7C9X3_ERROR},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
						      };

	pci_Device PEP12Devices[]= {
								                   {"PI7C9X130 CPCI", PCI_BRIDGE | PCI_DEVICE | CPCI_PARENT, PCI_SINGLE_FUNC,
								                    0x12d8, 0xe130, 0, 0, 0x00, PI7C9XRegs, {0x00,0x00,0x00},
								                    NULL, (void*)NULL, NULL, PI7C9X_ERROR},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							      };

	pci_Device RP1MDevices[] = {
													{"PCI EXPRESS Root Port 3 [Cougar Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
									 0x8086, 0x1c14, 28, 2, 0x00, RP3Regs, {0x00,0x00,0x00}, NULL,NULL,
									 NULL, RP3_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
								   };


	pci_Regs   PCIEP1Regs[]  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
								   };
	pci_Device RP0MDevices[] = {

														{"PCI EXPRESS Port-1-1 x8  [Sandy Bridge]",
														PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/ | PXMC_PARENT,
														PCI_SINGLE_FUNC | PXMC_XMC1,
														0x8086, 0x0105, 1, 1, 0x00,
														PCIEP1Regs, {0x00,0x00,0x00}, NULL, (void*)PEP11Devices, NULL, PCIEP11_ERROR},

														{"PCI EXPRESS Port-1-2 x8  [Sandy Bridge]", PCI_BRIDGE | PCI_EXPRESS ,
														PCI_SINGLE_FUNC,
														0x8086, 0x0109, 1, 2, 0x00,
														PCIEP1Regs, {0x00,0x00,0x00}, NULL, (void*)PEP12Devices, NULL, PCIEP12_ERROR},

														{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };


	pci_Device RP0MSatelliteDevices[] = {

														{"PCI EXPRESS Port-1-1 x8  [Sandy Bridge]",
														PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PXMC_PARENT,
														PCI_SINGLE_FUNC | PXMC_XMC2,
														0x8086, 0x0105, 1, 1, 0x00,
														PCIEP1Regs, {0x00,0x00,0x00}, NULL, (void*)PEP11Devices, NULL, PCIEP11_ERROR},

														{"PCI EXPRESS Port-1-2 x8  [Sandy Bridge]", PCI_BRIDGE | PCI_EXPRESS ,
														PCI_SINGLE_FUNC,
														0x8086, 0x0109, 1, 2, 0x00,
														PCIEP1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, PCIEP12_ERROR},

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

	pci_Regs   I82579Regs[]  = {{NULL  , 0x00, 0x00}};

	pci_Device pciDevices0[] = {{"DRAM Controller [Sandy Bridge]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x0104,
								 0, 0, 0x00, DRAMCRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, DRAMC_ERROR, NULL},
	
								{"PCI EXPRESS-0 Port x8 [Sandy Bridge]", PCI_BRIDGE | PCI_EXPRESS|PXMC_PARENT ,
								  PCI_MULTI_FUNC, 0x8086, 0x0101, 1, 0, 0x00, PCIE0Regs,
								  {0x00,0x00,0x00}, RP0MDevices, (void*)PCIE0BDevices, NULL, PCIE0_ERROR, NULL},

								 {"PCI EXPRESS-1 [Sandy Bridge]", PCI_DEVICE, PCI_SINGLE_FUNC,
								  0x8086, 0x0116, 2, 0, 0x00, PCIE1Regs, {0x00,0x00,0x00}, NULL, NULL,
								  NULL, PCIE1_ERROR, NULL},

									{"OptDev [Sandy Bridge]", PCI_DEVICE | PCI_OPTIONAL , PCI_SINGLE_FUNC,
									0x8086, 0x0103, 4, 0, 0x00, NULL, {0x00,0x00,0x00}, NULL, NULL,
									NULL, OPTDEV_ERROR, NULL},

									{"Intel 82579LM [Cougar Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1502, 25, 0, 0x00, I82579Regs,
										{0x00,0x00,0x00}, NULL, NULL, NULL, I82579_ERROR },

								{"USB EHCI 2 [Cougar Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1c2d, 26, 0,
								  0x00, EHCI2Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI2_ERROR, NULL },

								{"PCI EXPRESS Root Port 1 [Cougar Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								  0x8086, 0x1c10, 28, 0, 0x00, RP1Regs, {0x00,0x00,0x00},  (void*)RP1MDevices,
								  (void*)RP1BDevices,NULL , RP1_ERROR, NULL},

								{"USB EHCI 1 [Cougar Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1c26, 29, 0,
								  0x00, EHCI1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR, NULL },

								{"LPC Interface [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c4f, 31, 0,
								  0x00, LPCRegs, {0x00,0x00,0x00}, (void*)LPCDevices_IDE, NULL, NULL, LPC_ERROR, NULL },

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
	
	if((temp & PMC1_PRESENT) == PMC1_PRESENT)
	{
		rt |= PXMC_PMC1;
	}

	if((temp & PMC2_PRESENT) == PMC2_PRESENT)
	{
		rt |= PXMC_PMC2;
	}

	if((temp & XMC1_PRESENT) == XMC1_PRESENT)
	{
		rt |= PXMC_XMC1;
	}

	if( (temp & XMC2_PRESENT) == XMC2_PRESENT)
	{
		rt |= PXMC_XMC2;
	}

	return rt;
}



/*****************************************************************************
 * brdChkPeripheral: check peripheral mode and AD CR2 PMC module's presence
 * RETURNS: status
 * */
UINT8 brdChkPeripheral(void)
{
	UINT8 temp, rt;

	temp = ((UINT8)dIoReadReg(0x214, REG_8));

	if(temp & CPCI_SYSEN)
		rt = 1;
	else
		rt = 0;

	return rt;
}

UINT8 brdChkSatellite(void)
{
		UINT8 temp, rt = 0;

		temp = ((UINT8)dIoReadReg(0x214, REG_8));

		if(temp & FORCE_SAT)
			rt = 0;
		else
			rt = 1;

		return rt;
}



	pci_DB  brdPCIDB = { 0x00, brdChkPXMC, brdChkSatellite, brdChkPeripheral, pciDevices0, PCI_ERROR_BASE};

	/*****************************************************************************
	 * brdGetPCIDB: returns the SIO global data structure
 	* RETURNS: pci_DB*
 	* */
	UINT32 brdGetPCIDB(void *ptr)
	{
		UINT8 bTemp;
		PCI_PFA pfa;

		bTemp = brdChkPXMC();

		if ( (bTemp & XMC2_PRESENT) == XMC2_PRESENT)
		{
			pciDevices0[1].BDevices=NULL;
		}


		pfa = PCI_MAKE_PFA (0, 31, 2);

		bTemp = PCI_READ_BYTE(pfa,0x0A);

		if (bTemp == AHCI_MODE )
		{
			pciDevices0[8].MDevices = (void*)LPCDevices_AHCI;
		}
		else if (bTemp == RAID_MODE)
		{
			pciDevices0[8].MDevices = (void*)LPCDevices_RAID;
		}

		bTemp = brdChkSatellite ();

		if (bTemp == 0)
		{
			pciDevices0[1].MDevices = NULL;
			pciDevices0[1].MDevices = (void*)RP0MSatelliteDevices;
		}


		*((pci_DB**)ptr) = &brdPCIDB;

		return E__OK;
	}


