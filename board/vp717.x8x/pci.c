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

/* superio.c - board-specific superio information 
 */

/* Version control:
 *
 * $Header:
 * $Log: 
 * Revision
 * board-specific superio support.
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>
#include <bit/io.h>


#define PCI_ERROR_BASE		(E__BIT + 0x6000) // all global errors are 
											  // E__BIT + PCI_ERRORE_BASE + error code 1-fh


	#define DRAMC_ERROR					0x10
	#define PCIEG_ERROR					0x20
	#define IGD0_ERROR					0x30
	#define PCIEP1_ERROR				0x40
	#define MEI_ERROR					0x50
		#define MEI1_ERROR				0x60
		#define PTIDER_ERROR			0x70
	#define EHCI1_ERROR					0x80
	#define EHCI2_ERROR				    0x90
	#define PTP_ERROR				    0xA0
	#define LPC_ERROR				    0xB0
		#define SATA1_ERROR				0xC0
		#define SATA2_ERROR				0xD0
		#define SMBUS_ERROR				0xE0
	#define RP0_ERROR				    0xF0
		#define RP2_ERROR				0x100
		#define RP4_ERROR				0x110

	#define PI7C9X_ERROR				0x120
	#define TSI148_ERROR				0x130
	#define PI7C9X2_ERROR				0x140
	#define I825760_ERROR				0x150
	#define I825761_ERROR				0x160
	#define PCIBr_ERROR					0x170
	#define PI7C9X3_ERROR				0x180


	pci_Regs   MEI1Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   PTIDERRegs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Device MEIDevices[]  = {{"MEI1 [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b66,
								  22, 2, 0x00, MEI1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, MEI1_ERROR, NULL},

								{"PT IDER [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b67,
								  22, 3, 0x00, PTIDERRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, PTIDER_ERROR, NULL},

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
	pci_Device LPCDevices[]  = {{"SATA 1 [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b2e,
								  31, 2, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR, NULL},

								{"SMBUS [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b30,
								  31, 3, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR, NULL},

								{"SATA 2 [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b2d,
								  31, 5, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA2_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };



	pci_Regs   PCIBrRegs[]   = {{NULL      , 0x00, 0x00}
								   };
	pci_Device PCIBDevices[] = {{"PCI Bridge", PCI_BRIDGE | PCI_OPTIONAL | PCI_CARRIER , PCI_SINGLE_FUNC ,
								0x8086, 0xb152, 14, 0, 0, PCIBrRegs, {0,0,0}, NULL, NULL, NULL, PCIBr_ERROR, NULL},
								
								{"PCI Bridge", PCI_BRIDGE | PCI_OPTIONAL | PCI_CARRIER , PCI_SINGLE_FUNC ,
								0x1011, 0x0024, 14, 0, 0, PCIBrRegs, {0,0,0}, NULL, NULL, NULL, PCIBr_ERROR, NULL},

								{"PCI Bridge", PCI_BRIDGE | PCI_OPTIONAL | PCI_CARRIER , PCI_SINGLE_FUNC ,
								 0x12d8, 0x8152, 14, 0, 0, PCIBrRegs, {0,0,0}, NULL, NULL, NULL, PCIBr_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };



	pci_Regs   PI7C9X2Regs[]= {{NULL  , 0x00, 0x00}
							  };
	pci_Device RP2Devices[]= {{"PI7C9X130", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END, PCI_SINGLE_FUNC, 0x12d8,
							   0xe110, 0, 0,  0x00, PI7C9X2Regs, {0x00,0x00,0x00}, NULL, (void*)PCIBDevices,
							   NULL, PI7C9X2_ERROR, NULL},

							   {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							  };


	pci_Regs   I825761Regs[]  = {{NULL  , 0x00, 0x00}
							   };
	pci_Device I82576Devices[]= {{"Intel 82576", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x10c9, 0, 1, 0x00,
								 I825761Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825761_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	pci_Regs   I825760Regs[]  = {{NULL  , 0x00, 0x00}
							   };
	pci_Device RP4Devices[]= {{"Intel 82576", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x10c9, 0, 0, 0x00,
								I825760Regs, {0x00,0x00,0x00}, (void*)I82576Devices, NULL, NULL, I825760_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };


	pci_Regs   RP2Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Regs   RP4Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Device RP0MDevices[] = {{"PCI EXPRESS Root Port 2 [Ibex Peak]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								 0x8086, 0x3b46, 28, 2, 0x00, RP2Regs, {0x00,0x00,0x00}, NULL, (void*)RP2Devices,
								 NULL, RP2_ERROR, NULL},

								{"PCI EXPRESS Root Port 4 [Ibex Peak]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
			  					  0x8086, 0x3b4a, 28, 4, 0x00, RP4Regs, {0x00,0x00,0x00}, NULL, (void*)RP4Devices,
			  					  NULL, RP4_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };



	pci_Regs   TSI148Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								   };
	pci_Device PI7C9XDevices[]= {{"TSI148", PCI_DEVICE, PCI_SINGLE_FUNC, 0x10e3, 0x0148, 4, 0, 0x00,
								 TSI148Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, TSI148_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
								};



	pci_Regs   PI7C9XRegs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Device RP0BDevices[] = {{"PI7C9X130", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL, PCI_SINGLE_FUNC,
								 0x12d8, 0xe130, 0, 0, 0x00, PI7C9XRegs, {0x00,0x00,0x00},
								 NULL, (void*)PI7C9XDevices, NULL, PI7C9X_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };


	pci_Regs   PI7C9X3Regs[]= {{NULL  , 0x00, 0x00}
								  };
	pci_Device PEP1Devices[]= {{"PI7C9X130", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL | PXMC_PARENT,
								PCI_SINGLE_FUNC | PXMC_PMC1 | PXMC_PMC2 | PXMC_OPTIONAL, 0x12d8, 0xe130, 0, 0,  0x00, PI7C9X3Regs, {0x00,0x00,0x00}, NULL, (void*)PCIBDevices,
							    NULL, PI7C9X3_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						      };




	pci_Regs   DRAMCRegs[]   = {{NULL  , 0x00, 0x00}
							   };
	pci_Regs   PCIEGRegs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   IGD0Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   PCIEP1Regs[]  = {{"INTRLINE", 0x3C, 0xff},
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
	pci_Regs   PTPRegs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   LPCRegs[]     = {{NULL  , 0x00, 0x00}
							   };
	pci_Regs   RP0Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Device pciDevices0[] = {{"DRAM Controller [Arrandale]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x0044,
								 0, 0, 0x00, DRAMCRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, DRAMC_ERROR, NULL},
	
								{"PCI EXPRESS-G Port x8 [Arrandale]", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT,
								  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x8086, 0x0045, 1, 0, 0x00, PCIEGRegs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, PCIEG_ERROR, NULL},

								{"Internal Graphics Device [Arrandale]", PCI_DEVICE, PCI_SINGLE_FUNC,
								  0x8086, 0x0046, 2, 0, 0x00, IGD0Regs, {0x00,0x00,0x00}, NULL, NULL,
								  NULL, IGD0_ERROR, NULL},

								{"PCI EXPRESS Port-1 x8  [Arrandale]", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT |
								  PCI_DEV_PRESENT, PCI_SINGLE_FUNC | PXMC_XMC2 | PXMC_OPTIONAL, 0x8086, 0x0047, 6, 0, 0x00,
								  PCIEP1Regs, {0x00,0x00,0x00}, NULL, (void*)PEP1Devices, NULL, PCIEP1_ERROR, NULL},

								

								{"MEI [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b64, 22, 0, 0x00,
								  MEIRegs, {0x00,0x00,0x00}, (void*)MEIDevices, NULL, NULL, MEI_ERROR , NULL},

								{"USB EHCI 1 [Ibex Peak]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x3b3c, 26, 0,
								  0x00, EHCI1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR , NULL},

								{"PCI EXPRESS Root Port 0 [Ibex Peak]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								  0x8086, 0x3b42, 28, 0, 0x00, RP0Regs, {0x00,0x00,0x00}, (void*)RP0MDevices,
								  (void*)RP0BDevices, NULL, RP0_ERROR, NULL},

								{"USB EHCI 2 [Ibex Peak]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x3b34, 29, 0,
								  0x00, EHCI2Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI2_ERROR , NULL},

								{"PCI-to-PCI Bridge [Ibex Peak]", PCI_BRIDGE, PCI_SINGLE_FUNC, 0x8086, 0x2448,
								  30, 0, 0x00, PTPRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, PTP_ERROR, NULL },

								{"LPC Interface [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b07, 31, 0,
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
	 * brdChkPeripheral: check peripheral mode and AD CR2 PMC module's presence
	 * RETURNS: status
	 * */
	UINT8 brdChkPeripheral(void)
	{
		UINT8 temp, rt;

		temp = ((UINT8)dIoReadReg(0x212, REG_8));
	
		if( (temp & 0x04) || (temp & 0x08) )
			rt = 1;
		else
			rt = 0;
	
		return rt;
	}	


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













