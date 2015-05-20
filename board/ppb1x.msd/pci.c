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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/pci.c,v 1.3 2015-04-02 16:21:09 hchalla Exp $
 *
 * $Log: pci.c,v $
 * Revision 1.3  2015-04-02 16:21:09  hchalla
 * Updated pci devices for single XMC/PMC 2 site.
 *
 * Revision 1.2  2015-04-01 16:44:29  hchalla
 * Initial release of CUTE firmware 02.02.01.
 *
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
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
#include "../ppb1x.msd/cctboard.h"


/* defines */
//#define DEBUG

#define vReadByte(addr)			(*(UINT8*)(addr))
#define vReadWord(addr)			(*(UINT16*)(addr))
#define vReadDWord(addr)		(*(UINT32*)(addr))

#define vWriteByte(addr,value)	(*(UINT8*)(addr) = value)
#define vWriteWord(addr,value)	(*(UINT16*)(addr) = value)
#define vWriteDWord(addr,value)	(*(UINT32*)(addr) = value)

#define GP_LVL          0x0C
#define GPIO_BASE	    0x48
#define LPC_BRIDGE_DEV	31

#define AHCI_MODE   0x06
#define IDE_MODE    0x01
#define RAID_MODE   0x04

// error codes
#define PCI_ERROR_BASE		(E__BIT + 0x6000) // all global errors are
											  // E__BIT + PCI_ERROR_BASE + error code 1-fh



	#define DRAMC_ERROR					0x10
	#define PCIE0_ERROR					0x20
	#define PCIE1_ERROR					0x30
	#define IGD0_ERROR					0x40
	#define AUDIO_ERROR					0x50
	#define THERM_ERROR					0x60
	#define PLX0_ERROR					0x70
	#define PLX1_ERROR					0x71
	#define PLX2_ERROR					0x72
	#define PLX3_ERROR					0x73
	#define PLX4_ERROR					0x74
	#define PLX5_ERROR					0x75
	#define PLX6_ERROR					0x76
	#define PLX7_ERROR					0x77
	#define PLX8_ERROR					0x78
	#define PLX9_ERROR					0x79
	#define PLX10_ERROR					0x7A
	#define PLX11_ERROR					0x7B
	#define PLXNT_ERROR					0x7C
	#define DMA0_ERROR					0x80
	#define DMA1_ERROR					0x81
	#define DMA2_ERROR					0x82
	#define DMA3_ERROR					0x83
	#define DMA4_ERROR					0x84
	#define DMA5_ERROR					0x85
	#define DMA6_ERROR					0x86
	#define DMA7_ERROR					0x87
	#define XHCI_ERROR					0xA0
	#define CHAP_ERROR					0xB0
	#define CHAP1_ERROR					0xB1
	#define MEI_ERROR					0xC0
	#define IDER_ERROR					0xD0
	#define KT_ERROR					0xE0
	#define EHCI1_ERROR					0xF0
	#define EHCI2_ERROR					0x100
	#define RP1_ERROR				    0x110
	#define RP2_ERROR					0x111
	#define RP3_ERROR					0x112
	#define RP4_ERROR					0x113
	#define RP5_ERROR					0x114
	#define RP6_ERROR					0x115
	#define RP7_ERROR					0x116
	#define RP8_ERROR					0x117
	#define HDA_ERROR					0x120
	#define PI7C9X_ERROR				0x130
	#define QU_ERROR					0x140
	#define	I3500_ERROR					0x150
	#define	I3501_ERROR					0x151
	#define	I3502_ERROR					0x152
	#define	I3503_ERROR					0x153
   #define	I3504_ERROR					    0x154
	#define	I825990_ERROR					0x160
	#define	I825991_ERROR					0x161
	#define	PMC_ERROR					0x170
	#define XMC_ERROR					0x180
	#define PI7C9X3_ERROR			0x190
 	 #define ENXMC_ERROR			0x191

//	#define PTP_ERROR				    0x190
	#define LPC_ERROR				    0x1A0
	#define SATA1_ERROR					0x1B0
	#define SATA2_ERROR					0x1C0
	#define SMBUS_ERROR					0x1D0
	#define TSDDEV_ERROR				0x1E0
  #define UNIVERSE_II_ERROR				0x1F0






	pci_Regs   PI7C9X3Regs[]= {{NULL  , 0x00, 0x00}
								  };

	pci_Device PEP11Devices[]= {
							    {"PI7C9X130 PMC2", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL | PXMC_PARENT,
							    PCI_SINGLE_FUNC | PXMC_PMC1 , 0x12d8, 0xe130, 0, 0,  0x00, PI7C9X3Regs, {0x00,0x00,0x00}, NULL, NULL,
							     NULL, PI7C9X3_ERROR},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
						      };

	pci_Device PEP12Devices[]= {
							    {"PI7C9X130 PMC1", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL | PXMC_PARENT,
							    PCI_SINGLE_FUNC | PXMC_PMC1 , 0x12d8, 0xe130, 0, 0,  0x00, PI7C9X3Regs, {0x00,0x00,0x00}, NULL, NULL,
							     NULL, PI7C9X3_ERROR},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
						      };

	pci_Device PEP13Devices[]= {
								                   {"PI7C9X130 CPCI", PCI_BRIDGE | PCI_DEVICE | CPCI_PARENT, PCI_SINGLE_FUNC,
								                    0x12d8, 0xe130, 0, 0, 0x00, PI7C9X3Regs, {0x00,0x00,0x00},
								                    NULL, (void*)NULL, NULL, PI7C9X_ERROR},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
							      };

	/* locals */

	  pci_Regs   I82599Regs[]  = {{"INTRLINE", 0x3C, 0xff},
	                                 {NULL  , 0x00, 0x00}
	                                };
	  pci_Regs   I825991Regs[]  = {{"INTRLINE", 0x3C, 0xff},
	                                 {NULL  , 0x00, 0x00}
	                                };
	    pci_Device I82599Devices[]= {{"Intel 82599 SFP", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x10FB, 0, 1, 0x00,
	                                  I825991Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825991_ERROR, NULL},
	                                {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
	                               };
	    pci_Device RP1BDevices[]  = {{"Intel 82599 SFP", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x10FB, 0, 0, 0x00,
	                                I82599Regs, {0x00,0x00,0x00}, (void*)I82599Devices, NULL, NULL, I825990_ERROR, NULL},

	                                {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
	    	};




		pci_Regs   PLXRegs[]= {{NULL  , 0x00, 0x00}
									  };

		pci_Device PLX0MDevices[] = {{"PLX8619-0 DMA 0", PCI_DEVICE | PCI_EXPRESS,
				PCI_SINGLE_FUNC , 0x10B5, 0x8619, 0, 1, 0x00, PLXRegs,
						     {0x00,0x00,0x00}, NULL, NULL, NULL, DMA0_ERROR, NULL},
	#if 0
					 	     {"PLX 8619-1", PCI_DEVICE | PCI_EXPRESS,
					 	    		PCI_SINGLE_FUNC , 0x10B5, 0x8619, 0, 0, 0x00, PLXRegs,
						     {0x00,0x00,0x00}, NULL, NULL, NULL, DMA1_ERROR, NULL},

		                                     {"PLX8619-2", PCI_DEVICE | PCI_EXPRESS,
						       PCI_SINGLE_FUNC , 0x10B5, 0x8619, 2, 0, 0x00, PLXRegs,
						       {0x00,0x00,0x00}, NULL, NULL, NULL, DMA2_ERROR, NULL},

		                                       {"PLX8619-3", PCI_DEVICE | PCI_EXPRESS,
							PCI_SINGLE_FUNC , 0x10B5, 0x8619, 3, 0, 0x00, PLXRegs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, DMA3_ERROR, NULL},

		    					{"PLX8619-4", PCI_DEVICE | PCI_EXPRESS,
							PCI_SINGLE_FUNC , 0x10B5, 0x8619, 0, 0, 0x00, PLXRegs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, DMA3_ERROR, NULL},
	#endif
							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
						     };

		pci_Device PLX1MDevices[] = {{"PLX8619-0 DMA 0", PCI_DEVICE | PCI_EXPRESS,
							PCI_SINGLE_FUNC , 0x10B5, 0x8619, 0, 1, 0x00, PLXRegs,
							     {0x00,0x00,0x00}, NULL, NULL, NULL, DMA0_ERROR, NULL},
	#if 0
						 	     {"PLX 8619-1", PCI_DEVICE | PCI_EXPRESS,
						              PCI_SINGLE_FUNC , 0x10B5, 0x8619, 0, 0, 0x00, PLXRegs,
							     {0x00,0x00,0x00}, NULL, NULL, NULL, DMA1_ERROR, NULL},

			                                     {"PLX8619-2", PCI_DEVICE | PCI_EXPRESS,
							       PCI_SINGLE_FUNC , 0x10B5, 0x8619, 2, 0, 0x00, PLXRegs,
							       {0x00,0x00,0x00}, NULL, NULL, NULL, DMA2_ERROR, NULL},

			                                       {"PLX8619-3", PCI_DEVICE | PCI_EXPRESS,
								PCI_SINGLE_FUNC , 0x10B5, 0x8619, 3, 0, 0x00, PLXRegs,
								{0x00,0x00,0x00}, NULL, NULL, NULL, DMA3_ERROR, NULL},

			    					{"PLX8619-4", PCI_DEVICE | PCI_EXPRESS,
								PCI_SINGLE_FUNC , 0x10B5, 0x8619, 0, 0, 0x00, PLXRegs,
								{0x00,0x00,0x00}, NULL, NULL, NULL, DMA3_ERROR, NULL},
	#endif
								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							     };




		pci_Regs IntrRegs[]	= {
								{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							  };


		pci_Regs   DMA0Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   DMA1Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   DMA2Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   DMA3Regs[]     = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};


		pci_Regs   PLX2Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
										{NULL  , 0x00, 0x00}
									};
		pci_Regs   PLX3Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   PLX4Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   PLX5Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   PLX6Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Device PLXrS1Devices[]= {{"PEX8733-0 Switch port 1", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT,
									 PCI_MULTI_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x10B5, 0x8733, 1, 0, 0x00, PLX2Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL,  PLX2_ERROR, NULL},

									{"PEX8733-0 Switch port 8", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT,
									 PCI_SINGLE_FUNC, 0x10B5, 0x8733, 8, 0, 0x00, PLX3Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX3_ERROR, NULL},

									{"PEX8733-0 Switch port 9", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT,
									 PCI_SINGLE_FUNC, 0x10B5, 0x8733, 9, 0, 0x00, PLX4Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX4_ERROR, NULL},

									{"PEX8733-0 Switch port 10", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT,
									 PCI_SINGLE_FUNC, 0x10B5, 0x8733, 10, 0, 0x00, PLX5Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX5_ERROR, NULL},

									{"PEX8733-0 Switch port 11", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT,
									 PCI_SINGLE_FUNC, 0x10B5, 0x8733, 11, 0, 0x00, PLX6Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX6_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};

		pci_Regs   DMA4Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   DMA5Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   DMA6Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   DMA7Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
	#if 0
		pci_Device PLX1MDevices[] = {
									{"PEX8733-1 DMA 0", PCI_DEVICE | PCI_EXPRESS,
									 PCI_MULTI_FUNC , 0x10B5, 0x87D0, 0, 1, 0x00, DMA4Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, DMA4_ERROR, NULL},

				 	 	 	 	 	{"PEX8733-1 DMA 1", PCI_DEVICE | PCI_EXPRESS,
									 PCI_MULTI_FUNC , 0x10B5, 0x87D0, 0, 2, 0x00, DMA5Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, DMA5_ERROR, NULL},

				 	 	 	 	 	{"PEX8733-1 DMA 2", PCI_DEVICE | PCI_EXPRESS,
									 PCI_MULTI_FUNC , 0x10B5, 0x87D0, 0, 3, 0x00, DMA6Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, DMA6_ERROR, NULL},

				 	 	 	 	 	{"PEX8733-1 DMA 3", PCI_DEVICE | PCI_EXPRESS,
									 PCI_MULTI_FUNC , 0x10B5, 0x87D0, 0, 4, 0x00, DMA7Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, DMA7_ERROR, NULL},

									 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};
	#endif
		pci_Regs   PLX7Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   PLX8Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   PLX9Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   PLX10Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   PLX11Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Device PLXrS2Devices[] ={{"PEX8733-1 Switch port 1", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT,
									 PCI_MULTI_FUNC | PXMC_XMC2 | PXMC_OPTIONAL, 0x10B5, 0x8733, 1, 0, 0x00, PLX7Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX7_ERROR, NULL},

									{"PEX8733-1 Switch port 8", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT,
									 PCI_SINGLE_FUNC, 0x10B5, 0x8733, 8, 0, 0x00, PLX8Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX8_ERROR, NULL},

									{"PEX8733-1 Switch port 9", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT,
									 PCI_SINGLE_FUNC, 0x10B5, 0x8733, 9, 0, 0x00, PLX9Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX9_ERROR, NULL},

									{"PEX8733-1 Switch port 10", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT,
									 PCI_SINGLE_FUNC, 0x10B5, 0x8733, 10, 0, 0x00, PLX10Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX10_ERROR, NULL},

									{"PEX8733-1 Switch port 11", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL | CPCI_PARENT,
									 PCI_SINGLE_FUNC, 0x10B5, 0x8733, 11, 0, 0x00, PLX11Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PLX11_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};

		pci_Regs   PLX1Regs[]	= {{"INTRLINE", 0x3C, 0xff},
									 {NULL  , 0x00, 0x00}
									};
		pci_Device PCIE1BDevices[]= {{"PEX8733-1 Switch Port 0", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END,
									  PCI_MULTI_FUNC, 0x10B5, 0x8733, 0, 0, 0x00, PLX1Regs,
									  {0x00,0x00,0x00}, (void*)PLX1MDevices, (void*)PLXrS2Devices, NULL, PLX1_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};

		pci_Regs   PCIE1Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Device PCIE0MDevices[]= {{"PCI EXPRESS-1 Port x8 [Haswell]", PCI_BRIDGE | PCI_EXPRESS,
									  PCI_MULTI_FUNC, 0x8086, 0x0C05, 1, 1, 0x00, PCIE1Regs,
									  {0x00,0x00,0x00}, NULL, RP1BDevices, NULL, PCIE1_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};

		pci_Regs   PLX0Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									 {NULL  , 0x00, 0x00}
									};
		pci_Device PCIE0BDevices[]= {{"PEX8733-0 Switch Port 0", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END,
									  PCI_MULTI_FUNC, 0x10B5, 0x8733, 0, 0, 0x00, PLX0Regs,
									  {0x00,0x00,0x00}, (void*)PLX0MDevices, (void*)PLXrS1Devices, NULL, PLX0_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};


		pci_Regs   PMC1Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									 {NULL  , 0x00, 0x00}
									};
		pci_Regs   PMC2Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									 {NULL  , 0x00, 0x00}
									};
		pci_Device SCSIMDevices[] = {{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0021, 5, 1, 0, PMC2Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PMC_ERROR, NULL},

									{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0021, 8, 1, 0, PMC2Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PMC_ERROR, NULL},

									{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0030, 5, 1, 0, PMC2Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PMC_ERROR, NULL},

									{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0030, 8, 1, 0, PMC2Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PMC_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};

		pci_Device SCSIDevices[] =  {{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0021, 5, 0, 0, PMC1Regs,
									 {0x00,0x00,0x00}, (void*)SCSIMDevices, NULL, NULL, PMC_ERROR, NULL},

									{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0021, 8, 0, 0, PMC1Regs,
									 {0x00,0x00,0x00},  (void*)SCSIMDevices, NULL, NULL, PMC_ERROR, NULL},

									{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0030, 5, 0, 0, PMC1Regs,
									 {0x00,0x00,0x00},  (void*)SCSIMDevices, NULL, NULL, PMC_ERROR, NULL},

									{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0030, 8, 0, 0, PMC1Regs,
									 {0x00,0x00,0x00},  (void*)SCSIMDevices, NULL, NULL, PMC_ERROR, NULL},

									{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0050, 5, 0, 0, PMC1Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PMC_ERROR, NULL},

									{"PMC SCSI", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x1000, 0x0050, 8, 0, 0, PMC1Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, PMC_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};

		pci_Regs   I3501Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									 {NULL  , 0x00, 0x00}
									};
		pci_Regs   I3502Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									 {NULL  , 0x00, 0x00}
									};
		pci_Regs   I3503Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									 {NULL  , 0x00, 0x00}
									};
		pci_Regs   I3504Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
										 {NULL  , 0x00, 0x00}
										};

		pci_Regs   I2500Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Regs   PCIXRegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};


		pci_Regs   I3500Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Device i350MDevices[] = {{"Intel i350 Copper", PCI_DEVICE | PCI_EXPRESS,
										 PCI_MULTI_FUNC, 0x8086, 0x1521, 0, 1, 0, I3501Regs,
										 {0x00,0x00,0x00}, NULL, NULL, NULL, I3501_ERROR, NULL},

										{"Intel i350 Copper", PCI_DEVICE | PCI_EXPRESS,
										 PCI_MULTI_FUNC, 0x8086, 0x1521, 0, 2, 0, I3502Regs,
										 {0x00,0x00,0x00}, NULL, NULL, NULL, I3502_ERROR, NULL},

										{"Intel i350 Copper", PCI_DEVICE | PCI_EXPRESS,
										 PCI_MULTI_FUNC, 0x8086, 0x1521, 0, 3, 0, I3503Regs,
										 {0x00,0x00,0x00}, NULL, NULL, NULL, I3503_ERROR, NULL},

										{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
										};






		pci_Device RP2BDevices[]  = {{"Pericom PCI-X Bridge", PCI_BRIDGE,
									 PCI_SINGLE_FUNC, 0x12D8, 0xE130, 0, 0, 0, PCIXRegs,
									 {0x00,0x00,0x00}, NULL, (void*)SCSIDevices, NULL, PI7C9X_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};


		pci_Device RP6BDevices[]  = {{"Intel i350 Copper", PCI_DEVICE | PCI_EXPRESS,
									 PCI_MULTI_FUNC, 0x8086, 0x1521, 0, 0, 0, I3500Regs,
									 {0x00,0x00,0x00}, (void*)i350MDevices, NULL, NULL, I3500_ERROR, NULL},

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};

		pci_Regs   I2501Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Regs   RP1Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   RP4Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   RP6Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   RP7Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Device RP1MDevices[] = {{"PCI EXPRESS Root Port 2 [Lynx Point]", PCI_BRIDGE | PCI_EXPRESS,
									 PCI_MULTI_FUNC, 0x8086, 0x8c14, 28, 2, 0, RP1Regs,
				  					 {0x00,0x00,0x00}, NULL,  (void*)PEP12Devices, NULL, RP1_ERROR, NULL},

									{"PCI EXPRESS Root Port 4 [Lynx Point]", PCI_BRIDGE | PCI_EXPRESS,
									 PCI_MULTI_FUNC, 0x8086, 0x8c18, 28, 4, 0, RP4Regs,
				  					 {0x00,0x00,0x00}, NULL, (void*)PEP13Devices, NULL, RP4_ERROR, NULL},

									{"PCI EXPRESS Root Port 6 [Lynx Point]", PCI_BRIDGE | PCI_EXPRESS,
				  					  PCI_MULTI_FUNC, 0x8086, 0x8c1c, 28, 6, 0, RP6Regs,
									  {0x00,0x00,0x00}, NULL, (void*)RP6BDevices, NULL, RP6_ERROR, NULL},

									/*{"PCI EXPRESS Root Port 7 [Lynx Point]", PCI_BRIDGE | PCI_EXPRESS,
									 PCI_MULTI_FUNC, 0x8086, 0x8c16, 28, 7, 0, RP7Regs,
				  					 {0x00,0x00,0x00}, NULL, (void*)RP7BDevices, NULL, RP7_ERROR, NULL},*/

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};

		pci_Regs   QURegs[]		  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Device	BDevices_8_0_0[]= {
									{"Universe II VME Chip", PCI_DEVICE, PCI_SINGLE_FUNC,
										0x10E3, 0x0000, 4, 0, 0x00,
										IntrRegs, {0x00,0x00,0x00}, NULL, NULL,
										NULL, UNIVERSE_II_ERROR, NULL},
									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};

		pci_Device	RP0BDevices []= {
									{"PCI Express to PCI-XPI7C9X130 PCI-X Bridge ", PCI_BRIDGE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC|PXMC_PMC1|PXMC_PARENT,
										0x12D8, 0xE130, 0, 0, 0x00,
										IntrRegs, {0x00,0x00,0x00}, NULL, NULL,
										NULL, PI7C9X_ERROR, NULL},
									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};



		pci_Device PCIE0BDevicesDummy[]= {
					                                                {"Unknown", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL | PXMC_PARENT,
													                PCI_SINGLE_FUNC | PXMC_PMC1 , 0x111d, 0x8061, 0, 0,  0x00, PI7C9X3Regs, {0x00,0x00,0x00}, NULL, NULL,
												                    NULL, PI7C9X3_ERROR},
										{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
										};


		pci_Regs   ENXMCRegs[]= {{NULL  , 0x00, 0x00}
									  };
		pci_Device PCIE0BDevicesENXMC1[]= {
					                                                {"EN-XMC_1", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL | PXMC_PARENT,
													                PCI_MULTI_FUNC | PXMC_PMC1 , 0x10B5, 0x8619, 0, 0,  0x00, ENXMCRegs, {0x00,0x00,0x00}, PLX0MDevices, NULL,
												                    NULL, ENXMC_ERROR},
										{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
										};

		pci_Device PCIE0BDevicesENXMC2[]= {
					                                                {"EN-XMC_2", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL | PXMC_PARENT,
													                PCI_MULTI_FUNC | PXMC_PMC2 , 0x10B5, 0x8619, 0, 0,  0x00, ENXMCRegs, {0x00,0x00,0x00}, PLX1MDevices, NULL,
												                    NULL, ENXMC_ERROR},
										{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
										};


		pci_Regs   SATA1Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   SATA2Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   SMBUSRegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Regs   CHAPRegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Regs   TSDRegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Device LPCDevices[]   = {{"SATA 1 [Lynx Point]", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x8086, 0x8c01, 31, 2, 0, SATA1Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR, NULL},

									{"SATA 1 [Lynx Point]", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x8086, 0x8c03, 31, 2, 0, SATA1Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR, NULL},

									{"SMBUS [Lynx Point]", PCI_DEVICE,
									 PCI_MULTI_FUNC, 0x8086, 0x8c22, 31, 3, 0, SMBUSRegs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR, NULL},

									 {"SATA 2 [Lynx Point]", PCI_DEVICE | PCI_OPTIONAL,
									  PCI_MULTI_FUNC, 0x8086, 0x8c09, 31, 5, 0, SATA2Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, SATA2_ERROR, NULL},

									/*{"CHAP Device [Lynx Point]", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x8086, 0x8c23, 31, 4, 0, CHAPRegs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, CHAP_ERROR, NULL},


									{"Intel Thermal Sensor Device", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_MULTI_FUNC, 0x8086, 0x8c24, 31, 6, 0, TSDRegs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, TSDDEV_ERROR, NULL},*/

									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								   };

		pci_Regs   DRAMCRegs[]	  = {{NULL  , 0x00, 0x00}
									};
		pci_Regs   PCIE0Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   IGD0Regs[]    = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   AUDRegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   THERMRegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   CHAP1Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Regs   HECIRegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};

		pci_Regs   HDARegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   EHCI1Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   EHCI2Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   xHCIRegs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   RP0Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
									};
		pci_Regs   LPCRegs[]	  = {{NULL  , 0x00, 0x00}
									};
		pci_Device pciDevices0[]  = {{"DRAM Controller [Haswell]", PCI_DEVICE,
									 PCI_SINGLE_FUNC, 0x8086, 0x0C04, 0, 0, 0, DRAMCRegs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, DRAMC_ERROR, NULL},

									{"PCI EXPRESS-0 Port x8 [Haswell]", PCI_BRIDGE | PCI_EXPRESS,
									  PCI_MULTI_FUNC, 0x8086, 0x0C01, 1, 0, 0, PCIE0Regs,
									  {0x00,0x00,0x00}, (void*)PCIE0MDevices,NULL, NULL, PCIE0_ERROR, NULL},

									{"Internal Graphics Device [Haswell]", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_SINGLE_FUNC, 0x8086, 0x0416, 2, 0, 0, IGD0Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, IGD0_ERROR, NULL},

										{"Internal Graphics Device [Haswell]", PCI_DEVICE | PCI_OPTIONAL,
										 PCI_SINGLE_FUNC, 0x8086, 0x0406, 2, 0, 0, IGD0Regs,
										{0x00,0x00,0x00}, NULL, NULL, NULL, IGD0_ERROR, NULL},

									{"Audio Device [Haswell]", PCI_DEVICE | PCI_OPTIONAL,
									  PCI_SINGLE_FUNC, 0x8086, 0x0C0C, 3, 0, 0, AUDRegs,
									  {0x00,0x00,0x00}, NULL, NULL, NULL, AUDIO_ERROR, NULL},

										{"Thermal Device [Haswell]", PCI_DEVICE | PCI_OPTIONAL,
									  PCI_SINGLE_FUNC, 0x8086, 0x0C03, 4, 0, 0, THERMRegs,
									  {0x00,0x00,0x00}, NULL, NULL, NULL, THERM_ERROR, NULL},

									  /*{"CHAP Device [Haswell]", PCI_DEVICE | PCI_OPTIONAL,
									  PCI_SINGLE_FUNC, 0x8086, 0x0C07, 7, 0, 0, CHAP1Regs,
									  {0x00,0x00,0x00}, NULL, NULL, NULL, CHAP1_ERROR, NULL},
									 */
									{"USB xHCI [Lynx Point]", PCI_DEVICE,
									 PCI_SINGLE_FUNC, 0x8086, 0x8c31, 20, 0, 0, xHCIRegs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, XHCI_ERROR, NULL },

									{"HECI Controller [Lynx Point]", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_SINGLE_FUNC,  0x8086, 0x8c3a, 22, 0,  0, HECIRegs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, MEI_ERROR, NULL },

									{"USB EHCI 2 [Lynx Point]", PCI_DEVICE,
									 PCI_SINGLE_FUNC, 0x8086, 0x8c2d, 26, 0, 0, EHCI2Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI2_ERROR, NULL },

									{"High Definition Audio [Lynx Point]", PCI_DEVICE | PCI_OPTIONAL,
									 PCI_SINGLE_FUNC, 0x8086, 0x8c20, 27, 0, 0, HDARegs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, HDA_ERROR, NULL },

									{"PCI EXPRESS Root Port 0 [Lynx Point]", PCI_BRIDGE | PCI_EXPRESS,
									 PCI_MULTI_FUNC, 0x8086, 0x8c10, 28, 0, 0, RP0Regs,
									 {0x00,0x00,0x00}, (void*)RP1MDevices, (void*)PEP11Devices, NULL, RP1_ERROR, NULL},

									{"USB EHCI 1 [Lynx Point]", PCI_DEVICE,
									 PCI_SINGLE_FUNC, 0x8086, 0x8c26, 29, 0, 0, EHCI1Regs,
									 {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR, NULL },

									{"LPC Interface [Lynx Point]", PCI_DEVICE,
									 PCI_MULTI_FUNC, 0x8086, 0x8c4f, 31, 0, 0, LPCRegs,
									 {0x00,0x00,0x00}, (void*)LPCDevices, NULL, NULL, LPC_ERROR, NULL },

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
		UINT16 wGpioBase = 0;
		UINT8 bVal   = 0;
		UINT32 dTemp = 0;
		UINT8       SecondaryBus;
		UINT16	    VID, DID;
#ifdef DEBUG
		char achBuffer[80];
#endif

		pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
		wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

		// GPIO_27
		dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

#ifdef DEBUG
		sprintf (achBuffer, "\nGPIO_27:%08X",dTemp);
		vConsoleWrite (achBuffer);
#endif
		if ((dTemp & 0x8000000) == 0x8000000)
		{

			 ((pci_Device*)(pciDevices0[1].MDevices))[0].BDevices = NULL;

			bTemp = brdChkPXMC();

			if ( (bTemp & PXMC_XMC2) == PXMC_XMC2)
			{
				// Do we have site-2 populated?
				pfa = PCI_MAKE_PFA (0, 1, 0);

				SecondaryBus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);

				pfa = PCI_MAKE_PFA (SecondaryBus, 0, 0);
				VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
				DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

				if (VID == 0x10B5 && DID == 0x8619)
				{
					pciDevices0[1].BDevices= PCIE0BDevicesENXMC2;
				}
				else if (VID == 0x111d && DID == 0x8061)
					pciDevices0[1].BDevices= PCIE0BDevicesDummy;
			}


			if ( (bTemp & PXMC_XMC1) == PXMC_XMC1)
			{
				// Do we have site-1 populated?
				pfa = PCI_MAKE_PFA (0, 1, 1);

				SecondaryBus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);

				pfa = PCI_MAKE_PFA (SecondaryBus, 0, 0);
				VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
				DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

				if (VID == 0x10B5 && DID == 0x8619)
				{
					 ((pci_Device*)(pciDevices0[1].MDevices))[0].BDevices = PCIE0BDevicesENXMC1;
				}
				else if (VID == 0x111d && DID == 0x8061)
					 ((pci_Device*)(pciDevices0[1].MDevices))[0].BDevices = PCIE0BDevicesDummy;
			}
		}
		else
		{
			bTemp = brdChkPXMC();
			if ( (bTemp & PXMC_XMC2) == PXMC_XMC2)
			{
				// Do we have site-2 populated?
				pfa = PCI_MAKE_PFA (0, 1, 0);

				SecondaryBus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);

				pfa = PCI_MAKE_PFA (SecondaryBus, 0, 0);
				VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
				DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

				if (VID == 0x10B5 && DID == 0x8619)
				{
					pciDevices0[1].BDevices= PCIE0BDevicesENXMC2;
				}
				else if (VID == 0x111d && DID == 0x8061)
					pciDevices0[1].BDevices= PCIE0BDevicesDummy;
			}
		}

		*((pci_DB**)ptr) = &brdPCIDB;

		return E__OK;
	}


