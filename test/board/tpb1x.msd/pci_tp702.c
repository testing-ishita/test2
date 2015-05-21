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
 * $Header: /indsoftdata/cvs/software/CUTE_1/CUTE_1/TRD/board/tp702.38x/pci.c,v 1.1 2014/04/10 09:18:19 arul Exp $
 *
 * $Log: pci.c,v $
 * Revision 1.1  2014/04/10 09:18:19  arul
 * Initial version
 *
 * Revision 1.1  2014/01/29 14:14:26  kamalr
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.3  2011/04/05 13:52:50  hchalla
 * Corrected tabs and spaces.
 *
 * Revision 1.2  2011/01/20 10:01:17  hmuneer
 * CA01A151
 *
 * Revision 1.1  2010/11/04 18:07:52  hchalla
 * Newly added support for tp 702 board.
 *
 */

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
	#define I82574_ERROR				0x150
	#define PCIBr_ERROR					0x170
	#define PCIBr0_ERROR				0x180
	#define PCIBr1_ERROR				0x190
	#define PI7C9X3_ERROR				0x200


	pci_Regs   MEI1Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Regs   PTIDERRegs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Device MEIDevices[]  = {{"MEI1 [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b66,
								  22, 2, 0x00, MEI1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, MEI1_ERROR},

								{"PT IDER [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b67,
								  22, 3, 0x00, PTIDERRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, PTIDER_ERROR},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
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
								  31, 2, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR},

								{"SMBUS [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b30,
								  31, 3, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},

								{"SATA 2 [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b2d,
								  31, 5, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA2_ERROR},



								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };



	pci_Regs   PI7C9X2Regs[]= {{NULL  , 0x00, 0x00}
								  };
	pci_Device PCIPLXDevices[]= {{"PI7C9X130", PCI_BRIDGE | PCI_DEVICE | CPCI_PARENT, PCI_SINGLE_FUNC, 0x12d8,
								   0xe130, 0, 0,  0x00, PI7C9X2Regs, {0x00,0x00,0x00}, NULL, NULL,NULL, PI7C9X2_ERROR},
								   {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
								  };

	pci_Regs   PCIBrRegs[]   = {{NULL      , 0x00, 0x00}
								   };
	pci_Device PCIBDevices[] = {{"PCI Bridge-0", PCI_BRIDGE , PCI_SINGLE_FUNC ,0x10b5, 0x8619, 1, 0, 0, PCIBrRegs, {0,0,0}, NULL, (void*)PCIPLXDevices, NULL, PCIBr_ERROR},
								{"PCI Bridge-1", PCI_BRIDGE, PCI_SINGLE_FUNC ,0x10b5, 0x8619, 3, 0, 0, PCIBrRegs, {0,0,0}, NULL, NULL, NULL, PCIBr_ERROR},
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };


	pci_Regs   I82574Regs[]  = {{NULL  , 0x00, 0x00}
							   };
	pci_Device RP4Devices[]= {{"Intel 82574-1", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x10D3, 0, 0, 0x00,
								I82574Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I82574_ERROR},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };



	pci_Regs   RP2Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Regs   RP4Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Device RP0MDevices[] = {
								{"PCI EXPRESS Root Port 2 [Ibex Peak]", PCI_BRIDGE | PCI_EXPRESS |PCI_OPTIONAL, PCI_MULTI_FUNC,
						  		  0x8086, 0x3b46, 28, 2, 0x00, RP2Regs, {0x00,0x00,0x00}, NULL, NULL,
						  		  NULL, RP4_ERROR},
								{"PCI EXPRESS Root Port 4 [Ibex Peak]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
			  					  0x8086, 0x3b4a, 28, 4, 0x00, RP4Regs, {0x00,0x00,0x00}, NULL, (void*)RP4Devices,
			  					  NULL, RP4_ERROR},


								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };



	pci_Regs   TSI148Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								   };
	pci_Device PI7C9XDevices[]= {{"TSI148", PCI_DEVICE, PCI_SINGLE_FUNC, 0x10e3, 0x0148, 4, 0, 0x00,
								 TSI148Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, TSI148_ERROR},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
								};



	pci_Regs   PI7C9XRegs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Device RP0BDevices[] = {{"PI7C9X130", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END, PCI_SINGLE_FUNC,
								 0x12d8, 0xe130, 0, 0, 0x00, PI7C9XRegs, {0x00,0x00,0x00},
								 NULL, (void*)PI7C9XDevices, NULL, PI7C9X_ERROR},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							   };


	pci_Regs   DRAMCRegs[]   = {{NULL  , 0x00, 0x00}
							   };
	pci_Regs   PCIEGRegs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };

	pci_Device PCIMDevices[]= {{"PCI Device PLX Tech", PCI_DEVICE | PCI_EXPRESS, PCI_MULTI_FUNC ,
				                      0x10b5, 0x8619, 0, 1, 0x00, PCIEGRegs, {0x00,0x00,0x00},
				                      NULL,NULL, NULL, PCIBr_ERROR},
									 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
								      };

	pci_Device PCIEGDevices[]= {{"PCI Bridge PLX Tech", PCI_BRIDGE | PCI_EXPRESS |PCI_EXPRESS_END, PCI_MULTI_FUNC ,
			                      0x10b5, 0x8619, 0, 0, 0x00, PCIEGRegs, {0x00,0x00,0x00}, (void*)PCIMDevices,
			                      (void*)PCIBDevices, NULL, PCIBr_ERROR},
								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
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

	pci_Regs   Eth82577Regs[]   = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00} };

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
								 0, 0, 0x00, DRAMCRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, DRAMC_ERROR},
	
								{"PCI EXPRESS-G Port x8 [Arrandale]", PCI_BRIDGE | PCI_EXPRESS,
								  PCI_SINGLE_FUNC, 0x8086, 0x0045, 1, 0, 0x00, PCIEGRegs,
								  {0x00,0x00,0x00}, NULL,(void *)PCIEGDevices, NULL, PCIEG_ERROR},

								{"Internal Graphics Device [Arrandale]", PCI_DEVICE, PCI_SINGLE_FUNC,
								  0x8086, 0x0046, 2, 0, 0x00, IGD0Regs, {0x00,0x00,0x00}, NULL, NULL,
								  NULL, IGD0_ERROR},	

								{"PCI EXPRESS Port-1 x8  [Arrandale]", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT |
								  PCI_DEV_PRESENT, PCI_SINGLE_FUNC | PXMC_XMC2, 0x8086, 0x0047, 6, 0, 0x00,
								  PCIEP1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, PCIEP1_ERROR},

								{"MEI [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b64, 22, 0, 0x00,
								  MEIRegs, {0x00,0x00,0x00}, (void*)MEIDevices, NULL, NULL, MEI_ERROR },

								 {"82577 Ethernet Controller", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x10ea, 25, 0,
								 0x00, Eth82577Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR },

								{"USB EHCI 1 [Ibex Peak]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x3b3c, 26, 0,
								  0x00, EHCI1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR },

								{"PCI EXPRESS Root Port 0 [Ibex Peak]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								  0x8086, 0x3b42, 28, 0, 0x00, RP0Regs, {0x00,0x00,0x00}, (void*)RP0MDevices,
								  NULL, NULL, RP0_ERROR},

								{"USB EHCI 2 [Ibex Peak]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x3b34, 29, 0,
								  0x00, EHCI2Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI2_ERROR },

								{"PCI-to-PCI Bridge [Ibex Peak]", PCI_BRIDGE, PCI_SINGLE_FUNC, 0x8086, 0x2448,
								  30, 0, 0x00, PTPRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, PTP_ERROR },

								{"LPC Interface [Ibex Peak]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x3b07, 31, 0,
								  0x00, LPCRegs, {0x00,0x00,0x00}, (void*)LPCDevices, NULL, NULL, LPC_ERROR },

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0},
							};	


	/*****************************************************************************
	 * brdChkPXMC: check PMC/XMC module's presence
	 * RETURNS: status
	 * */
	UINT8 brdChkPXMC(void)
	{
		UINT8 temp, rt = 0;

		temp = ((UINT8)dIoReadReg(0x312, REG_8));

		if(temp & 0x40)
			rt |= PXMC_XMC1;

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

		if(temp & 0x01)
			rt = 1;
		else
		    rt = 0;

				return rt;
	}	


	UINT8 brdChkSatellite(void)
	{
		UINT8 temp, rt = 0;

		temp = ((UINT8)dIoReadReg(0x214, REG_8));

		if(temp & 0x04)
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
	*((pci_DB**)ptr) = &brdPCIDB;

	return E__OK;
}













