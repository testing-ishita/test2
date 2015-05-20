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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx91x.01x/pci.c,v 1.1 2014-05-15 09:42:03 jthiru Exp $
 * $Log: pci.c,v $
 * Revision 1.1  2014-05-15 09:42:03  jthiru
 * Adding vx91x board sources
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011/03/22 13:30:36  hchalla
 * Initial version of PCI devices list checked in on behalf of Haroon Muneer.
 *
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

#if 0
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



	pci_Regs   DMA0Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Device PES32MDevices[]= {{"DMA 0 [PES32NT8AG2]", PCI_DEVICE | PCI_EXPRESS,
								  PCI_MULTI_FUNC , 0x111d, 0x808f, 0, 2, 0x00, DMA0Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, DMA0_ERROR, NULL},

								{"NT port [PES32NT8AG2]", PCI_DEVICE | PCI_EXPRESS | PCI_OPTIONAL,
								  PCI_MULTI_FUNC , 0x111d, 0x808f, 0, 1, 0x00, DMA0Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, DMA0_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
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
	pci_Device PES32BDevices[]= {{"IDT port 8 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT  | PCI_OPTIONAL,
								  PCI_SINGLE_FUNC , 0x111d, 0x808f, 8, 0, 0x00, IDT8Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

								 {"IDT port 12 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT | PCI_OPTIONAL,
								  PCI_SINGLE_FUNC , 0x111d, 0x808f, 12, 0, 0x00, IDT12Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},

								 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT,
								  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808f, 16, 0, 0x00, IDT16Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

								 {"IDT port 20 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT | PCI_OPTIONAL,
								  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808f, 20, 0, 0x00, IDT16Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};








	pci_Regs   PCIE1Regs[]   = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
	pci_Device PCIE0MDevices[]= {{"PCI EXPRESS-1 Port x4/x8 [Sandy Bridge]", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT,
								  PCI_MULTI_FUNC | PXMC_XMC2, 0x8086, 0x0105, 1, 1, 0x00, PCIE1Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, PCIE1_ERROR, NULL},

								{"PCI EXPRESS-1 Port x4 [Sandy Bridge]", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | PXMC_PARENT,
								  PCI_MULTI_FUNC | PXMC_XMC2 | PXMC_OPTIONAL, 0x8086, 0x0109, 1, 2, 0x00, PCIE1Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, PCIE1_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};

	pci_Regs   IDTRegs[]      = {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							    };
	pci_Device PCIE0BDevices[]= {{"IDT Port 0 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END,
								  PCI_MULTI_FUNC, 0x111d, 0x808f, 0, 0, 0x00, IDTRegs, {0x00,0x00,0x00},
								  (void*)PES32MDevices, (void*)PES32BDevices, NULL, IDT_ERROR, NULL},

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
	pci_Device I82580Devices[]= {{"Intel 82580 copper", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x150e, 0, 1, 0x00,
								  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},

								 {"Intel 82580 Backplane", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1510, 0, 2, 0x00,
								  I825802Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825802_ERROR, NULL},

								 {"Intel 82580 Backplane", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1510, 0, 3, 0x00,
								  I825803Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825803_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	pci_Regs   I825800Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
							    };
	pci_Device RP3Devices[]  = {{"Intel 82580 copper", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x150e, 0, 0, 0x00,
								I825800Regs, {0x00,0x00,0x00}, (void*)I82580Devices, NULL, NULL, I825800_ERROR, NULL},

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
	pci_Device RP5BDevices[] = {{"PI7C9X130", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PXMC_PARENT,
								 PCI_SINGLE_FUNC | PXMC_PMC1 | PXMC_PMC2, 0x12d8, 0xe130, 0, 0, 0x00,
								 PI7C9X2Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, PI7C9X2_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	pci_Regs   RP3Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Regs   RP5Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Device RP1MDevices[] = {{"PCI EXPRESS Root Port 3 [Cougar Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								 0x8086, 0x1c14, 28, 2, 0x00, RP3Regs, {0x00,0x00,0x00}, NULL, (void*)RP3Devices,
								 NULL, RP3_ERROR, NULL},

								{"PCI EXPRESS Root Port 5 [Cougar Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
			  					  0x8086, 0x1c18, 28, 4, 0x00, RP5Regs, {0x00,0x00,0x00}, NULL, (void*)RP5BDevices,
			  					  NULL, RP5_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							   };

	pci_Regs   PI7C9XRegs[]  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Device RP1BDevices[] = {{"PI7C9X130", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END, PCI_SINGLE_FUNC,
								 0x12d8, 0xe130, 0, 0, 0x00, PI7C9XRegs, {0x00,0x00,0x00},
								 NULL, (void*)PI7C9XDevices, NULL, PI7C9X_ERROR, NULL},

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
	pci_Device LPCDevices[]  = {{"SATA 1 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c01,
								  31, 2, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR},

								{"SMBUS [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c22,
								  31, 3, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},

								{"SATA 2 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c09,
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
	pci_Device pciDevices0[] = {{"DRAM Controller [Sandy Bridge]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x0104,
								 0, 0, 0x00, DRAMCRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, DRAMC_ERROR, NULL},
	
								{"PCI EXPRESS-0 Port x8 [Sandy Bridge]", PCI_BRIDGE | PCI_EXPRESS ,
								  PCI_MULTI_FUNC, 0x8086, 0x0101, 1, 0, 0x00, PCIE0Regs,
								  {0x00,0x00,0x00}, (void*)PCIE0MDevices, (void*)PCIE0BDevices, NULL, PCIE0_ERROR, NULL},

								{"Internal Graphics Device [Sandy Bridge]", PCI_DEVICE, PCI_SINGLE_FUNC,
								  0x8086, 0x0116, 2, 0, 0x00, IGD0Regs, {0x00,0x00,0x00}, NULL, NULL,
								  NULL, IGD0_ERROR, NULL},

								{"OptDev [Sandy Bridge]", PCI_DEVICE | PCI_OPTIONAL , PCI_SINGLE_FUNC,
								  0x8086, 0x0103, 4, 0, 0x00, NULL, {0x00,0x00,0x00}, NULL, NULL,
								  NULL, OPTDEV_ERROR, NULL},


								{"MEI [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c3a, 22, 0, 0x00,
								  MEIRegs, {0x00,0x00,0x00}, (void*)MEIDevices, NULL, NULL, MEI_ERROR, NULL },

								{"USB EHCI 2 [Cougar Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1c2d, 26, 0,
								  0x00, EHCI2Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI2_ERROR, NULL },

								{"PCI EXPRESS Root Port 1 [Cougar Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
								  0x8086, 0x1c10, 28, 0, 0x00, RP1Regs, {0x00,0x00,0x00}, (void*)RP1MDevices,
								  (void*)RP1BDevices, NULL, RP1_ERROR, NULL},

								{"USB EHCI 1 [Cougar Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1c26, 29, 0,
								  0x00, EHCI1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR, NULL },

								{"PCI-to-PCI Bridge [Cougar Point]", PCI_BRIDGE, PCI_SINGLE_FUNC, 0x8086, 0x2448,
								  30, 0, 0x00, PTPRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, PTP_ERROR, NULL },

								{"LPC Interface [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c4f, 31, 0,
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

		temp = ((UINT8)dIoReadReg(0x211, REG_8));
		
		if(temp & 0x01)
			rt |= PXMC_PMC1;

		if(temp & 0x02)
			rt |= PXMC_PMC2;


		if(temp & 0x08)
			rt |= PXMC_XMC1;
	
		if(temp & 0x10)
			rt |= PXMC_XMC2;

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



	/*****************************************************************************
	 * brdGetVxsTPorts: returns the VXS transparent ports
	 * RETURNS: E__OK
	 * */
	UINT32 brdGetVxsTPorts(void *ptr)
	{
		pci_Device* node;

		node = (pci_Device*)brdPCIDB.Devices0[1].BDevices;
		node = (pci_Device*)(node)[0].BDevices;

		((VXS_T_PORTS*)ptr)->portA = &node[1];
		((VXS_T_PORTS*)ptr)->portB = &node[0];

		return E__OK;
	}

#endif
#if 1
#define DRAMC_ERROR                                     0x10
#define PCIE0_ERROR                                     0x20
#define PCIE1_ERROR                                     0x30
#define IGD0_ERROR                                      0x40
#define IDT_ERROR                                       0x50
#define DMA0_ERROR                                      0x60
#define IDT8_ERROR                                      0x70
#define IDT12_ERROR                                     0x80
#define IDT16_ERROR                                     0x90
#define MEI_ERROR                                       0xA0
#define IDER_ERROR                                      0xB0
#define KT_ERROR                                        0xC0
#define EHCI1_ERROR                                     0xD0
#define EHCI2_ERROR                                     0xE0
#define RP1_ERROR                                   0xF0
#define RP3_ERROR                                       0x100
#define RP5_ERROR                                       0x110
#define PI7C9X_ERROR                            0x120
#define PI7C9X2_ERROR                           0x130
#define I825800_ERROR                           0x140
#define I825801_ERROR                           0x150
#define I825802_ERROR                           0x160
#define I825803_ERROR                           0x170
#define TSI148_ERROR                            0x180
#define PTP_ERROR                                   0x190
#define LPC_ERROR                                   0x1A0
#define SATA1_ERROR                                     0x1B0
#define SATA2_ERROR                                     0x1C0
#define SMBUS_ERROR                                     0x1D0
#define OPTDEV_ERROR                            0x1E0
#define HDAUDIO_ERROR                           0x1F0
#define THERM_ERROR                             0x200
#define ADDNL_ERROR                              0x210

pci_Regs   DMA0Regs[]    = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                   };
pci_Device PES32MDevices[]= {
                              {"DMA 0 [PES32NT8AG2]",
                               PCI_DEVICE | PCI_EXPRESS,
                               PCI_MULTI_FUNC ,
                               0x111d,
                               0x808f,
                               0,
                               2,
                               0x00,
                               DMA0Regs,
                               {0x00,0x00,0x00},
                               NULL,
                               NULL,
                               NULL,
                               DMA0_ERROR},

                              {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
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
pci_Device PES32BDevices[]= {
                              {"IDT port 8 (B) [PES32NT8AG2]",
                               PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
                               PCI_SINGLE_FUNC,
                               0x111d,
                               0x808f,
                               0x8,
                               0,
                               0x00,
                               IDT8Regs,
                               {0x00,0x00,0x00},
                               NULL,
                               NULL,
                               NULL,
                               IDT8_ERROR},

                               {"IDT port 12 (A) [PES32NT8AG2]",
                                PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
                                PCI_SINGLE_FUNC,
                                0x111d,
                                0x808f,
                                0xc,
                                0,
                                0x00,
                                IDT12Regs,
                                {0x00,0x00,0x00},
                                NULL,
                                NULL,
                                NULL,
                                IDT12_ERROR},

                               {"IDT port 16 [PES32NT8AG2]",
                                PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT,
                                PCI_SINGLE_FUNC | PXMC_XMC1,
                                0x111d,
                                0x808f,
                                0x10,
                                0,
                                0x00,
                                IDT16Regs,
                                {0x00,0x00,0x00},
                                NULL,
                                NULL,
                                NULL,
                                IDT16_ERROR},

                               {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
                             };


pci_Regs   PCIE1Regs[]   = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                   };

pci_Device PCIE0MDevices[]= {
                              {"PCI EXPRESS Root Port [Ivy Bridge]",
                              PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | PXMC_PARENT,
                              PCI_MULTI_FUNC | PXMC_XMC2 /*| PXMC_OPTIONAL*/,
                              0x8086,
                              0x0155,
                              1,
                              1,
                              0x00,
                              PCIE1Regs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              PCIE1_ERROR},

                              {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},

                           };

pci_Regs   IDTRegs[]      = {
                              {"INTRLINE", 0x3C, 0xff},
                              {NULL  , 0x00, 0x00}
                            };

pci_Device PCIE0BDevices[]= {
                              {"IDT Port 0 [PES32NT8AG2]",
                              PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END,
                              PCI_MULTI_FUNC,
                              0x111d,
                              0x808f,
                              0,
                              0,
                              0x00,
                              IDTRegs,
                              {0x00,0x00,0x00},
                              (void*)PES32MDevices,
                              (void*)PES32BDevices,
                              NULL,
                              IDT_ERROR},

                              {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
                           };







pci_Regs   IDERRegs[]  = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                   };
pci_Regs   KTRegs[]    = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
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
pci_Device I82580Devices[]= {
                             {"Intel 82580 copper",
                              PCI_DEVICE | PCI_EXPRESS_END,
                              PCI_MULTI_FUNC,
                              0x8086,
                              0x150e,
                              0,
                              1,
                              0x00,
                              I825801Regs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              I825801_ERROR},

                             {"Intel 82580 Backplane",
                              PCI_DEVICE | PCI_EXPRESS_END,
                              PCI_MULTI_FUNC,
                              0x8086,
                              0x1510,
                              0,
                              2,
                              0x00,
                              I825802Regs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              I825802_ERROR},

                             {"Intel 82580 Backplane",
                              PCI_DEVICE | PCI_EXPRESS_END,
                              PCI_MULTI_FUNC,
                              0x8086,
                              0x1510,
                              0,
                              3,
                              0x00,
                              I825803Regs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              I825803_ERROR},

                             {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
                            };

pci_Regs   I825800Regs[]  = {{"INTRLINE", 0x3C, 0xff},
                                                         {NULL  , 0x00, 0x00}
                                                    };
pci_Device RP3Devices[]  = {
                            {"Intel 82580 copper",
                             PCI_DEVICE | PCI_EXPRESS_END,
                             PCI_MULTI_FUNC,
                             0x8086,
                             0x150e,
                             0,
                             0,
                             0x00,
                             I825800Regs,
                             {0x00,0x00,0x00},
                             (void*)I82580Devices,
                             NULL,
                             NULL,
                             I825800_ERROR},

                            {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
                           };



pci_Regs   TSI148Regs[]  = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                   };
pci_Device PI7C9XDevices[]= {
                             {"TSI148",
                              PCI_DEVICE,
                              PCI_SINGLE_FUNC,
                              0x10e3,
                              0x0148,
                              4,
                              0,
                              0x00,
                              TSI148Regs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              TSI148_ERROR},

                             {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
                            };


pci_Regs   PI7C9X2Regs[]  = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                        };
pci_Device RP5BDevices[] = {
                            {"PI7C9X130",
                             PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PXMC_PARENT,
                             PCI_SINGLE_FUNC | PXMC_PMC1 | PXMC_PMC2,
                             0x12d8,
                             0xe130,
                             0,
                             0,
                             0x00,
                             PI7C9X2Regs,
                             {0x00,0x00,0x00},
                             NULL,
                             NULL,
                             NULL,
                             PI7C9X2_ERROR},

                            {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
                           };

pci_Regs   RP3Regs[]     = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                        };
pci_Regs   RP5Regs[]     = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                        };
pci_Device RP1MDevices[] = {
                            {"PCI EXPRESS Root Port 3 [Panther Point]",
                             PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/,
                             PCI_MULTI_FUNC,
                             0x8086,
                             0x1e14,
                             0x1C,
                             2,
                             0x00,
                             RP3Regs,
                             {0x00,0x00,0x00},
                             NULL,
                             (void*)RP3Devices,
                             NULL,
                             RP3_ERROR},

                            {"PCI EXPRESS Root Port 5 [Panther Point]",
                             PCI_BRIDGE | PCI_EXPRESS /*| PCI_EXPRESS_END*/,
                             PCI_MULTI_FUNC,
                             0x8086,
                             0x1e18,
                             0x1C,
                             4,
                             0x00,
                             RP5Regs,
                             {0x00,0x00,0x00},
                             NULL,
                             (void*)RP5BDevices,
                             NULL,
                             RP5_ERROR},

                            {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
                           };

pci_Regs   PI7C9XRegs[]  = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                        };

pci_Device RP1BDevices[] = {
                            {"PI7C9X130",
                             PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END,
                             PCI_SINGLE_FUNC,
                             0x12d8,
                             0xe130,
                             0,
                             0,
                             0x00,
                             PI7C9XRegs,
                             {0x00,0x00,0x00},
                             NULL,
                             (void*)PI7C9XDevices,
                             NULL,
                             PI7C9X_ERROR},

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
pci_Regs   ThermRegs[]   = {{"INTRLINE", 0x3C, 0xff},
                                                        {NULL  , 0x00, 0x00}
                                                   };

pci_Device LPCDevices[]  = {
                            {"SATA Controller 1 [Panther Point]",
                             PCI_DEVICE | PCI_OPTIONAL,
                             PCI_MULTI_FUNC,
                             0x8086,
                             0x1e01,
                             0x1f,
                             2,
                             0x00,
                             SATA1Regs,
                             {0x00,0x00,0x00},
                             NULL,
                             NULL,
                             NULL,
                             SATA1_ERROR},
                             {"SATA Controller 1 [Panther Point]",
                              PCI_DEVICE | PCI_OPTIONAL,
                              PCI_MULTI_FUNC,
                              0x8086,
                              0x1e03,
                              0x1f,
                              2,
                              0x00,
                              SATA1Regs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              SATA1_ERROR},
                            {"SMBUS Controller [Panther Point]",
                             PCI_DEVICE,
                             PCI_MULTI_FUNC,
                             0x8086,
                             0x1e22,
                             0x1f,
                             3,
                             0x00,
                             SMBUSRegs,
                             {0x00,0x00,0x00},
                             NULL,
                             NULL,
                             NULL,
                             SMBUS_ERROR},

                            {"SATA Controller 2 [Panther Point]",
                             PCI_DEVICE,
                             PCI_MULTI_FUNC,
                             0x8086,
                             0x1e09,
                             0x1f,
                             5,
                             0x00,
                             SATA1Regs,
                             {0x00,0x00,0x00},
                             NULL,
                             NULL,
                             NULL,
                             SATA2_ERROR},

                             {"Thermal Mgmt Controller [Panther Point]",
                              PCI_DEVICE | PCI_OPTIONAL,
                              PCI_MULTI_FUNC,
                              0x8086,
                              0x1e09,
                              0x1f,
                              6,
                              0x00,
                              ThermRegs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              THERM_ERROR},

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
pci_Regs   AddlnRegs[]    = {{"INTRLINE", 0x3C, 0xff},
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
pci_Regs   HDAudioRegs[]   = {{"INTRLINE", 0x3C, 0xff},
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


pci_Device AddnlMDevices[]= {
                              {"MEI Contoller 1 [Panther Point]",
                               PCI_DEVICE | PCI_OPTIONAL,
                               PCI_MULTI_FUNC,
                               0x8086,
                               0x1e3c,
                               0x16,
                               2,
                               0x00,
                               MEIRegs,
                               {0x00,0x00,0x00},
                               NULL,
                               NULL,
                               NULL,
                               MEI_ERROR},

                             {"MEI Contoller 1 [Panther Point]",
                              PCI_DEVICE | PCI_OPTIONAL,
                              PCI_MULTI_FUNC,
                              0x8086,
                              0x1e3d,
                              0x16,
                              3,
                              0x00,
                              MEIRegs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              MEI_ERROR},

                              {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
                           };

/* Array of Bus 0 Device structs */
pci_Device pciDevices0[] = {
                            {"DRAM Controller [Ivy Bridge]", /* Name String */
                             PCI_DEVICE,                     /* Device type */
                             PCI_SINGLE_FUNC,                /* Function type */
                             0x8086,                         /* Vendor ID */
                             0x0154,                         /* Device ID */
                             0,                              /* Device Number */
                             0,                              /* Function Number */
                             0x00,                           /* Flags found n tested set it always 0 ??*/
                             DRAMCRegs,                      /* Register info for intr line */
                             {0x00,0x00,0x00},               /* Dynamic parameters - ?? */
                             NULL,                           /* MF Devices inline to this node `V` */
                             NULL,                           /* Board devices under this node `>`*/
                             NULL,                           /* First PMC/XMC node ?? */
                             DRAMC_ERROR},                   /* Error Code */

                            {"PCI EXPRESS-0 Port x8 [Ivy Bridge]",
                             PCI_BRIDGE | PCI_EXPRESS ,
                             PCI_SINGLE_FUNC,
                             0x8086,
                             0x0151,
                             1,
                             0,
                             0x00,
                             PCIE0Regs,
                             {0x00,0x00,0x00},
                             (void*)PCIE0MDevices,
                             (void*)PCIE0BDevices,
                             NULL,
                             PCIE0_ERROR},

                            {"Internal Graphics Device [Ivy Bridge]",
                             PCI_DEVICE,
                             PCI_SINGLE_FUNC,
                             0x8086,
                             0x0166,
                             2,
                             0,
                             0x00,
                             IGD0Regs,
                             {0x00,0x00,0x00},
                             NULL,
                             NULL,
                             NULL,
                             IGD0_ERROR},

                             {"Addnl. Device [Ivy Bridge]",
                              PCI_DEVICE,
                              PCI_SINGLE_FUNC,
                              0x8086,
                              0x0153,
                              4,
                              0,
                              0x00,
                              AddlnRegs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              ADDNL_ERROR},

                             {"Addnl. Device [Ivy Bridge]",
                              PCI_DEVICE,
                              PCI_SINGLE_FUNC,
                              0x8086,
                              0x1E31,
                              0x14,
                              0,
                              0x00,
                              AddlnRegs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              ADDNL_ERROR},

                            {"MEI Contoller 1 [Panther Point]",
                             PCI_DEVICE,
                             PCI_SINGLE_FUNC,
                             0x8086,
                             0x1e3a,
                             0x16,
                             0,
                             0x00,
                             MEIRegs,
                             {0x00,0x00,0x00},
                             (void*)AddnlMDevices,
                             NULL,
                             NULL,
                             MEI_ERROR},

                            {"USB EHCI Controller 2 [Panther Point]",
                             PCI_DEVICE,
                             PCI_SINGLE_FUNC,
                             0x8086,
                             0x1e2d,
                             0x1A,
                             0,
                             0x00,
                             EHCI2Regs,
                             {0x00,0x00,0x00},
                             NULL,
                             NULL,
                             NULL,
                             EHCI2_ERROR},

                            {"HD Audio Controller [Panther Point]",    /* Name String */
                             PCI_DEVICE | PCI_OPTIONAL,                                /* Device type */
                             PCI_SINGLE_FUNC,                /* Function type */
                             0x8086,                         /* Vendor ID */
                             0x01e20,                        /* Device ID */
                             0x1B,                           /* Device Number */
                             0,                              /* Function Number */
                             0x00,                           /* Flags found n tested set it always 0 ??*/
                             HDAudioRegs,                    /* Register info for intr line */
                             {0x00,0x00,0x00},               /* Dynamic parameters - ?? */
                             NULL,                           /* MF Devices inline to this node `V` */
                             NULL,                           /* Board devices under this node `>`*/
                             NULL,                           /* First PMC/XMC node ?? */
                             HDAUDIO_ERROR},                 /* Error Code */

                            {"PCI EXPRESS Root Port 1 [Panther Point]",
                             PCI_BRIDGE | PCI_EXPRESS,
                             PCI_MULTI_FUNC,
                             0x8086,
                             0x1e10,
                             0x1C,
                             0,
                             0x00,
                             RP1Regs,
                             {0x00,0x00,0x00},
                             (void*)RP1MDevices,
                             (void*)RP1BDevices,
                             NULL,
                             RP1_ERROR},

                            {"USB EHCI Controller [Panther Point]",
                             PCI_DEVICE,
                             PCI_SINGLE_FUNC,
                             0x8086,
                             0x1e26,
                             0x1d,
                             0,
                             0x00,
                             EHCI1Regs,
                             {0x00,0x00,0x00},
                             NULL,
                             NULL,
                             NULL,
                             EHCI1_ERROR},

                             {"Addnl. Device [Panther Point]",
                              PCI_BRIDGE | PCI_OPTIONAL,
                              PCI_SINGLE_FUNC,
                              0x8086,
                              0x2448,
                              0x1e,
                              0,
                              0x00,
                              MEIRegs,
                              {0x00,0x00,0x00},
                              NULL,
                              NULL,
                              NULL,
                              ADDNL_ERROR},

                            {"LPC Interface [Panther Point]",
                             PCI_DEVICE,
                             PCI_MULTI_FUNC,
                             0x8086,
                             0x1e55,
                             0x1F,
                             0,
                             0x00,
                             LPCRegs,
                             {0x00,0x00,0x00},
                             (void*)LPCDevices,
                             NULL,
                             NULL,
                             LPC_ERROR},

                            {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
                          };


/*****************************************************************************
 * brdChkPXMC: check PMC/XMC module's presence
 * RETURNS: status
 * */
UINT8 brdChkPXMC(void)
{
        UINT8 temp, rt = 0;

        temp = ((UINT8)dIoReadReg(0x211, REG_8));

        if(temp & 0x01)
                rt |= PXMC_PMC1;

        if(temp & 0x02)
                rt |= PXMC_PMC2;


        if(temp & 0x08)
                rt |= PXMC_XMC1;

        if(temp & 0x10)
                rt |= PXMC_XMC2;

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



/*****************************************************************************
 * brdGetVxsTPorts: returns the VXS transparent ports
 * RETURNS: E__OK
 * */
UINT32 brdGetVxsTPorts(void *ptr)
{
        pci_Device* node;

        node = (pci_Device*)brdPCIDB.Devices0[1].BDevices;
        node = (pci_Device*)(node)[0].BDevices;

        ((VXS_T_PORTS*)ptr)->portA = &node[1];
        ((VXS_T_PORTS*)ptr)->portB = &node[0];

        return E__OK;
}

#endif







