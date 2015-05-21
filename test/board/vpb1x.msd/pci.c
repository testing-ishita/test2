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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/pci.c,v 1.3 2015-03-10 09:54:37 hchalla Exp $
 * $Log: pci.c,v $
 * Revision 1.3  2015-03-10 09:54:37  hchalla
 * Added support for EN XMC test card, XMC Site testing.
 *
 * Revision 1.2  2015-02-27 17:19:17  hchalla
 * Added support for XMC site to test XM 510 boards.
 *
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * */

/* includes */

#include <stdio.h>
#include <stdtypes.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/board_service.h>
#include <bit/io.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include "cctboard.h"

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
	#define	I2500_ERROR					0x160
	#define	I2501_ERROR					0x161
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

/* locals */

	pci_Regs   PI7C9X3Regs[]= {{NULL  , 0x00, 0x00}
								  };
	pci_Device PCIE0BDevicesDummy[]= {
				                                                {"Unknown", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PCI_OPTIONAL | PXMC_PARENT,
												                PCI_SINGLE_FUNC | PXMC_PMC1 , 0x111d, 0x8061, 0, 0,  0x00, PI7C9X3Regs, {0x00,0x00,0x00}, NULL, NULL,
											                    NULL, PI7C9X3_ERROR},
									{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
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
#if 0
	pci_Device PLX0MDevices[] = {{"PEX8733-0 DMA 0", PCI_DEVICE | PCI_EXPRESS,
								 PCI_MULTI_FUNC , 0x10B5, 0x87D0, 0, 1, 0x00, DMA0Regs,
								 {0x00,0x00,0x00}, NULL, NULL, NULL, DMA0_ERROR, NULL},

			 	 	 	 	 	{"PEX8733-0 DMA 1", PCI_DEVICE | PCI_EXPRESS,
								 PCI_MULTI_FUNC , 0x10B5, 0x87D0, 0, 2, 0x00, DMA1Regs,
								 {0x00,0x00,0x00}, NULL, NULL, NULL, DMA1_ERROR, NULL},

			 	 	 	 	 	{"PEX8733-0 DMA 2", PCI_DEVICE | PCI_EXPRESS,
								 PCI_MULTI_FUNC , 0x10B5, 0x87D0, 0, 3, 0x00, DMA2Regs,
								 {0x00,0x00,0x00}, NULL, NULL, NULL, DMA2_ERROR, NULL},

			 	 	 	 	 	{"PEX8733-0 DMA 3", PCI_DEVICE | PCI_EXPRESS,
								 PCI_MULTI_FUNC , 0x10B5, 0x87D0, 0, 4, 0x00, DMA3Regs,
								 {0x00,0x00,0x00}, NULL, NULL, NULL, DMA3_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};
#endif
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
								  {0x00,0x00,0x00}, NULL, NULL, NULL, PCIE1_ERROR, NULL},

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
	pci_Device RP1BDevices[]  = {{"Intel i210 copper", PCI_DEVICE | PCI_EXPRESS,
								  PCI_SINGLE_FUNC, 0x8086, 0x1533, 0, 0, 0, I2500Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, I2500_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
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


	pci_Device RP4BDevices[]  = {{"Intel i350 Copper", PCI_DEVICE | PCI_EXPRESS,
								 PCI_MULTI_FUNC, 0x8086, 0x1521, 0, 0, 0, I3500Regs,
								 {0x00,0x00,0x00}, (void*)i350MDevices, NULL, NULL, I3500_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};



	pci_Device RP2BDevices[]  = {{"Pericom PCI-X Bridge", PCI_BRIDGE,
								 PCI_SINGLE_FUNC, 0x12D8, 0xE130, 0, 0, 0, PCIXRegs,
								 {0x00,0x00,0x00}, NULL, (void*)SCSIDevices, NULL, PI7C9X_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};


	/*pci_Device RP6BDevices[]  = {{"Intel i350 Copper", PCI_DEVICE | PCI_EXPRESS,
								 PCI_MULTI_FUNC, 0x8086, 0x1521, 0, 0, 0, I3500Regs,
								 {0x00,0x00,0x00}, (void*)i350MDevices, NULL, NULL, I3500_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};*/

	pci_Regs   I2501Regs[]	  = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
								};
	pci_Device RP7BDevices[]  = {{"Intel i210 copper", PCI_DEVICE | PCI_EXPRESS,
								  PCI_SINGLE_FUNC, 0x8086, 0x1533, 0, 0, 0, I2501Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, I2501_ERROR, NULL},

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
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
			  					 {0x00,0x00,0x00}, NULL,  (void*)RP2BDevices, NULL, RP1_ERROR, NULL},

								{"PCI EXPRESS Root Port 4 [Lynx Point]", PCI_BRIDGE | PCI_EXPRESS,
								 PCI_MULTI_FUNC, 0x8086, 0x8c18, 28, 4, 0, RP4Regs,
			  					 {0x00,0x00,0x00}, NULL, (void*)RP4BDevices, NULL, RP4_ERROR, NULL},

								/*{"PCI EXPRESS Root Port 6 [Lynx Point]", PCI_BRIDGE | PCI_EXPRESS,
			  					  PCI_MULTI_FUNC, 0x8086, 0x8c1c, 28, 6, 0, RP6Regs,
								  {0x00,0x00,0x00}, NULL, (void*)RP6BDevices, NULL, RP6_ERROR, NULL},

								{"PCI EXPRESS Root Port 7 [Lynx Point]", PCI_BRIDGE | PCI_EXPRESS,
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
								{"PCI Express to PCI-XPI7C9X130 PCI-X Bridge ", PCI_BRIDGE|PCI_EXPRESS|PCI_EXPRESS_END|PCI_OPTIONAL, PCI_SINGLE_FUNC|PXMC_PMC1|PXMC_PMC2|PXMC_PARENT,
									0x12D8, 0xE130, 0, 0, 0x00,
									IntrRegs, {0x00,0x00,0x00}, NULL, BDevices_8_0_0,
									NULL, PI7C9X_ERROR, NULL},
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

								{"Internal Graphics Device [Haswell]", PCI_DEVICE,
								 PCI_SINGLE_FUNC, 0x8086, 0x0416, 2, 0, 0, IGD0Regs,
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
								 {0x00,0x00,0x00}, (void*)RP1MDevices, (void*)RP0BDevices, NULL, RP1_ERROR, NULL},

								{"USB EHCI 1 [Lynx Point]", PCI_DEVICE,
								 PCI_SINGLE_FUNC, 0x8086, 0x8c26, 29, 0, 0, EHCI1Regs,
								 {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR, NULL },

								{"LPC Interface [Lynx Point]", PCI_DEVICE,
								 PCI_MULTI_FUNC, 0x8086, 0x8c4f, 31, 0, 0, LPCRegs,
								 {0x00,0x00,0x00}, (void*)LPCDevices, NULL, NULL, LPC_ERROR, NULL },

								{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};	



#define TRXMC_ERROR_BASE		(E__BIT + 0x7000) // all global errors are
										          // E__BIT + PCI_ERRORE_BASE + error code 1-fh

#define TRXMC_PEX8619r_ERROR	0x10
#define TRXMC_PEX8619M_ERROR	0x20
#define TRXMC_PEX86191_ERROR	0x30
#define TRXMC_PEX86192_ERROR	0x40
#define TRXMC_PEX86193_ERROR	0x50
#define TRXMC_PI7C9X_ERROR		0x60

pci_Regs   trXmcPI7C9XRegs[] = {{"INTRLINE", 0x3C, 0xff},
							    {NULL  , 0x00, 0x00}
							   };

pci_Device trXmcPEX86192D[] = {{"PI7C9X130", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PXMC_PARENT,
								PCI_SINGLE_FUNC | PXMC_PMC1, 0x12d8, 0xe130, 0, 0, 0, trXmcPI7C9XRegs,
								{0x00,0x00,0x00}, NULL, NULL, NULL, TRXMC_PI7C9X_ERROR, NULL},

							   {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
						      };


pci_Regs   trXmcPEX86191Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								   {NULL  , 0x00, 0x00}
						          };
pci_Regs   trXmcPEX86192Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								   {NULL  , 0x00, 0x00}
						          };
pci_Regs   trXmcPEX86195Regs[]  = {{"INTRLINE", 0x3C, 0xff},
								   {NULL  , 0x00, 0x00}
						          };

pci_Options   trXmcPEX86193Option[] = {{OPTION_MAX_PCIE_WIDTH, 0x00, 4},
								       {NULL  , 0x00, 0x00}
						   		       };

pci_Device trXmcPEX8619Devices[]={ {"PEX8619 D1 [TRXMC]", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT,
									PCI_SINGLE_FUNC | PXMC_XMC1, 0x10b5, 0x8619, 1, 0, 0, trXmcPEX86191Regs,
									{0x00,0x00,0x00}, NULL, NULL, NULL, TRXMC_PEX86191_ERROR, NULL},

				                   {"PEX8619 D2 [TRXMC]", PCI_BRIDGE | PCI_EXPRESS,
								    PCI_SINGLE_FUNC, 0x10b5, 0x8619, 2, 0, 0, trXmcPEX86192Regs,
									{0x00,0x00,0x00}, NULL, (void*)trXmcPEX86192D, NULL, TRXMC_PEX86192_ERROR, NULL},

								   {"PEX8619 D3 [TRXMC]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
								    PCI_SINGLE_FUNC, 0x10b5, 0x8619, 3, 0, 0, trXmcPEX86195Regs,
									{0x00,0x00,0x00}, NULL, NULL, NULL, TRXMC_PEX86193_ERROR, trXmcPEX86193Option},

								   {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							      };



pci_Regs   trXmcPEX8619MRegs[]   = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
						           };
pci_Device trXmcPEX8619rMDevice[]={{"PEX8619 M1 [TRXMC]", PCI_DEVICE | PCI_EXPRESS,
									PCI_MULTI_FUNC, 0x10b5, 0x8619, 0, 1, 0, trXmcPEX8619MRegs,
									{0x00,0x00,0x00}, NULL, NULL, NULL, TRXMC_PEX8619M_ERROR, NULL},

							       {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
						          };


pci_Regs   trXmcPEX8619rRegs[]= {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
						        };
pci_Device trXmcDevices[] = {{"PEX8619 Root [TRXMC]", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END,
							  PCI_MULTI_FUNC, 0x10b5, 0x8619, 1, 0, 0, trXmcPEX8619rRegs,
							  {0x00,0x00,0x00}, (void*)trXmcPEX8619rMDevice, (void*)trXmcPEX8619Devices, NULL, TRXMC_PEX8619r_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
						    };



pci_DB  trXMCPCIDB = { 0x00, NULL, NULL, NULL, trXmcDevices, TRXMC_ERROR_BASE};

/* forward declarations */
UINT32 checkVpxSwitch(void *ptr, PCI_PFA pfa);

/*****************************************************************************
 * brdGetTrXmcPCIDB: returns the TRXMC pci data structure
 * RETURNS: E__OK
 * */
UINT32 brdGetTrXmcPCIDB(void *ptr)
{
	*((pci_DB**)ptr) = &trXMCPCIDB;

	return E__OK;
}



/*****************************************************************************
 * brdChkPXMC: check PMC/XMC module's presence
 * RETURNS: status
 * */
UINT8 brdChkPXMC(void)
{
	UINT8 temp, rt = 0;

	temp = ((UINT8)dIoReadReg(0x212, REG_8));

	if(temp & PMC1_PRESENT)
		rt |= PXMC_PMC1;

	if(temp & PMC2_PRESENT)
		rt |= PXMC_PMC2;


	temp = ((UINT8)dIoReadReg(0x21D, REG_8));

	if(temp & 0x01)
		rt |= PXMC_XMC1;

	if(temp & 0x04)
		rt |= PXMC_XMC2;


	return rt;
}


UINT8 brdChkSatellite(void)
{
	return 1;// satellite mode not supported on VPX
}


UINT8 brdChkPeripheral(void)
{
	return 1;
}



pci_DB  brdPCIDB = { 0x00, brdChkPXMC, NULL, brdChkPeripheral, pciDevices0, PCI_ERROR_BASE};



/*****************************************************************************
 * brdGetPCIDB: returns the PCI data structure
* RETURNS: pci_DB*
* */
UINT32 brdGetPCIDB(void *ptr)
{

	UINT8 bTemp;
	PCI_PFA	    pfa;
	UINT8       SecondaryBus;
	UINT16	    VID, DID;

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


	*((pci_DB**)ptr) = &brdPCIDB;

	return E__OK;
}

#if 0
/*****************************************************************************
 * brdCheckPcieSwConfig: checks the correct config for TRXMC test
 * RETURNS: E__OK
 * */

UINT32 brdCheckPcieSwConfig(void *ptr)
{
	UINT8  per;
	PCI_PFA pfa;
	UINT32  rt;
	UINT8   secBus;

	#ifdef DEBUG
		char   achBuffer[128];
	#endif

	rt = E__OK;

	per = ((UINT8)dIoReadReg(STATUS_REG_0, REG_8));
	((PCIE_SW_CONFIG_INFO*)ptr)->tye = (per & SYS_CON) >> 7;		// syscon or peripheral

	#ifdef DEBUG
		sprintf (achBuffer, "per = 0x%02X\n", (per & SYS_CON) >> 7);
		vConsoleWrite (achBuffer);
	#endif

	((PCIE_SW_CONFIG_INFO*)ptr)->count = 0;

	pfa = PCI_MAKE_PFA(0, 1, 0);								// PCIe root port for VPX switch
	secBus = PCI_READ_BYTE(pfa, PCI_SEC_BUS);
	pfa = PCI_MAKE_PFA(secBus, 0, 0);
	rt = checkVpxSwitch(ptr, pfa);
	if (rt == E__OK)
	{
		pfa = PCI_MAKE_PFA(0, 1, 1);							// PCIe root port for VPX switch
		secBus = PCI_READ_BYTE(pfa, PCI_SEC_BUS);
		pfa = PCI_MAKE_PFA(secBus, 0, 0);
		rt = checkVpxSwitch(ptr, pfa);
	}
	return rt;
}

/*****************************************************************************
 * checkVpxSwitch: checks the correct config for TRXMC test
 * RETURNS: E__OK
 * */
UINT32 checkVpxSwitch(void *ptr, PCI_PFA pfa)
{
	UINT32  rt;
	UINT32  dMem;
	UINT32  dReg;
	UINT32  mHandle;
	PTR48   tPtr;
	volatile UINT32 memMap;

	#ifdef DEBUG
		char   achBuffer[128];
	#endif

	rt = E__OK;

	dMem = PCIE_READ_DWORD (pfa, 0x10);							// Device Specific configuration memory window
	mHandle = dGetPhysPtr(dMem,0x40000,&tPtr,(void*)&memMap);	// 256K memory window

	// check switch is not in NT mode
	dReg = vReadDWord (memMap + 0x360L);						// switch NT configuration

	#ifdef DEBUG
		sprintf (achBuffer, "SW Reg 0x360 = 0x%08X\n", dReg);
		vConsoleWrite (achBuffer);
	#endif

	if ((dReg & 0x2000L) != 0)
		rt = PCI_ERROR_BASE + PLXNT_ERROR;

	dReg = vReadDWord (memMap + 0x300L);						// switch configuration

	#ifdef DEBUG
		sprintf (achBuffer, "SW Reg 0x300 = 0x%08X\n", dReg);
		vConsoleWrite (achBuffer);
	#endif

	switch (dReg & 0x3FL)
	{
		case 0x12:						// PORT_CFG_LINK_1X8_1X8 + 1X8_1X8
			((PCIE_SW_CONFIG_INFO*)ptr)->count += 3;
			break;

		case 0x1A:						// PORT_CFG_LINK_1X8_1X8 + _1X8_2X4
			((PCIE_SW_CONFIG_INFO*)ptr)->count += 4;
			break;

		case 0x22:						// PORT_CFG_LINK_1X8_1X8 + _2X4_2X4
			((PCIE_SW_CONFIG_INFO*)ptr)->count += 5;
			break;

		default:
			((PCIE_SW_CONFIG_INFO*)ptr)->count = 0;
			rt = PCI_ERROR_BASE + PLX0_ERROR;
			break;
	}

	#ifdef DEBUG
		sprintf (achBuffer, "brInfo.type = %d, brInfo.count = %d\n",
					((PCIE_SW_CONFIG_INFO*)ptr)->tye, ((PCIE_SW_CONFIG_INFO*)ptr)->count);
		vConsoleWrite (achBuffer);
	#endif

	vFreePtr (mHandle); 		// release memory

	return rt;
}


/*****************************************************************************
 * brdGetTrXmcParentBus: returns the parent bus of the TRXMC
 * RETURNS: E__OK
 * */
UINT32 brdGetTrXmcParentBus(void *ptr)
{
	pci_Device*	pDev;
	PCI_PFA pfa;
	UINT8 secBus;
	UINT8 subBus;
#ifdef DEBUG
	char achBuffer[80];
#endif

//	pDev = ((pci_Device*) &brdPCIDB.Devices0[1]);			// pciDevices0

#ifdef DEBUG
//	sprintf (achBuffer, "%s\n", pDev->Name);
//	vConsoleWrite (achBuffer);
#endif

	pDev = ((pci_Device*)brdPCIDB.Devices0[1].BDevices);	// PCIE0BDevices

#ifdef DEBUG
	sprintf (achBuffer, "%s\n", pDev->Name);
	vConsoleWrite (achBuffer);
#endif

	pDev = (pci_Device*)pDev->BDevices;						// PLXrS1Devices

#ifdef DEBUG
	sprintf (achBuffer, "%s\n", pDev->Name);
	vConsoleWrite (achBuffer);
#endif

	pfa = pDev->Vars.pfa;
	subBus = PCI_READ_BYTE(pfa, PCI_SUB_BUS);
	secBus = pDev->Vars.sBUS;
#ifdef DEBUG
	sprintf (achBuffer, "secBus = %#X\nsubBus = %#X\n", secBus, subBus);
	vConsoleWrite (achBuffer);
#endif

	if (secBus == subBus)
	{
		++pDev;
		secBus = pDev->Vars.sBUS;
	}

#ifdef DEBUG
	sprintf (achBuffer, "PEX8619 bus = %#X\n", secBus);
	vConsoleWrite (achBuffer);
#endif

	*((UINT8*)ptr) = secBus;

	return E__OK;
}
#endif
