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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/tr803.39x/pci.c,v 1.1 2015-01-29 11:07:37 mgostling Exp $
 * $Log: pci.c,v $
 * Revision 1.1  2015-01-29 11:07:37  mgostling
 * Import files into new source repository.
 *
 * Revision 1.2  2013-10-31 10:50:58  mgostling
 * Removed redundant code
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.3  2011-09-29 14:38:37  hchalla
 * Minor Changes for release of TR 80x V1.01.
 *
 * Revision 1.2  2011/08/26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>
#include <bit/io.h>

#define DEBUG

#define GP_LVL          0x0C
#define GPIO_BASE	    0x48
#define LPC_BRIDGE_DEV	31

#define PCI_ERROR_BASE		(E__BIT + 0x6000) // all global errors are 
											  // E__BIT + PCI_ERRORE_BASE + error code 1-fh


#define DRAMC_ERROR					0x10
#define PCIE0_ERROR					0x20
#define PCIE1_ERROR					0x30
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

//#ifdef DEBUG
//	static char buffer[128];
//#endif


pci_Regs   IDT1Regs[]     = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
pci_Regs   DMA0Regs[]    = {{"INTRLINE", 0x3C, 0xff},
							{NULL  , 0x00, 0x00}
						   };

pci_Device PES32MDevices[]= {
								{"IDT Peripheral [PES32NT8AG2]", PCI_DEVICE |PCI_OPTIONAL| PCI_EXPRESS,
								 PCI_MULTI_FUNC , 0x111d, 0x808c, 0, 1, 0x00, IDT1Regs,
								{0x00,0x00,0x00}, NULL, NULL, NULL, IDT_ERROR, NULL},

		 	 	 	 	 	 {"DMA 0 [PES32NT8AG2]", PCI_DEVICE | PCI_EXPRESS,
							  PCI_MULTI_FUNC , 0x111d, 0x808c, 0, 2, 0x00, DMA0Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, DMA0_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};

pci_Regs   IDT4Regs[]     = {{"INTRLINE", 0x3C, 0xff},
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

pci_Regs   IDT17Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
pci_Regs   IDT18Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
pci_Regs   IDT19Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
pci_Regs   IDT20Regs[]    = {{"INTRLINE", 0x3C, 0xff},
								{NULL  , 0x00, 0x00}
							   };
pci_Regs   IDT21Regs[]    = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
								   };
pci_Regs   IDT22Regs[]    = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
								   };
pci_Regs   IDT23Regs[]    = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
								   };


pci_Device PES32rS1Devices[]= {{"IDT port 4 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT ,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 4, 0, 0x00, IDT4Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT4_ERROR, NULL},

							 {"IDT port 8 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 8, 0, 0x00, IDT8Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

							/* {"IDT port 12 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808C, 12, 0, 0x00, IDT12Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},*/

							 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL| CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 16, 0, 0x00, IDT16Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

							  {"IDT port 20 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 20, 0, 0x00, IDT16Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};

pci_Device PES32rS2Devices[]= {{"IDT port 4 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT ,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 4, 0, 0x00, IDT4Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT4_ERROR, NULL},

							 {"IDT port 8 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 8, 0, 0x00, IDT8Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

							 {"IDT port 12 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808C, 12, 0, 0x00, IDT12Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},

							 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 16, 0, 0x00, IDT16Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

							  {"IDT port 17 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							 PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 17, 0, 0x00, IDT17Regs,
							 {0x00,0x00,0x00}, NULL, NULL, NULL, IDT17_ERROR, NULL},

							 {"IDT port 18 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 18, 0, 0x00, IDT18Regs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, IDT18_ERROR, NULL},

							 {"IDT port 19 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 19, 0, 0x00, IDT19Regs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, IDT19_ERROR, NULL},

							  {"IDT port 20 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 20, 0, 0x00, IDT20Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};


pci_Device PES32rS3Devices[]= {{"IDT port 4 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT ,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 4, 0, 0x00, IDT4Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT4_ERROR, NULL},

							 {"IDT port 8 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 8, 0, 0x00, IDT8Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

							 {"IDT port 12 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808C, 12, 0, 0x00, IDT12Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},

							 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 16, 0, 0x00, IDT16Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

							  {"IDT port 17 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							 PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 17, 0, 0x00, IDT17Regs,
							 {0x00,0x00,0x00}, NULL, NULL, NULL, IDT17_ERROR, NULL},

							 {"IDT port 18 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 18, 0, 0x00, IDT18Regs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, IDT18_ERROR, NULL},

							 {"IDT port 19 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 19, 0, 0x00, IDT19Regs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, IDT19_ERROR, NULL},

							  {"IDT port 20 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 20, 0, 0x00, IDT20Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							  {"IDT port 21 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 21, 0, 0x00, IDT21Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							  {"IDT port 22 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 22, 0, 0x00, IDT22Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							  {"IDT port 23 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 23, 0, 0x00, IDT23Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};

pci_Device PES32rS0Devices[]= {{"IDT port 4 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT ,
								  PCI_SINGLE_FUNC , 0x111d, 0x808C, 4, 0, 0x00, IDT4Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT4_ERROR, NULL},

								 {"IDT port 8 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
								  PCI_SINGLE_FUNC , 0x111d, 0x808C, 8, 0, 0x00, IDT8Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

								 {"IDT port 12 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
								  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808C, 12, 0, 0x00, IDT12Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},

								 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
								  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 16, 0, 0x00, IDT16Regs,
								  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

								 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
								};



pci_Device PES32BDevices0[]= {{"IDT port 4 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT ,
									  PCI_SINGLE_FUNC , 0x111d, 0x808C, 4, 0, 0x00, IDT4Regs,
									  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT4_ERROR, NULL},

									 {"IDT port 8 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
									  PCI_SINGLE_FUNC , 0x111d, 0x808C, 8, 0, 0x00, IDT8Regs,
									  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

									 {"IDT port 12 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
									  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808C, 12, 0, 0x00, IDT12Regs,
									  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},

									 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
									  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 16, 0, 0x00, IDT16Regs,
									  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

									 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
									};
pci_Device PES32BDevices1[]= {{"IDT port 4 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT ,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 4, 0, 0x00, IDT4Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT4_ERROR, NULL},

							 {"IDT port 8 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 8, 0, 0x00, IDT8Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

							 /*{"IDT port 12 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808C, 12, 0, 0x00, IDT12Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},*/ /*Hari*/

							 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | PCI_OPTIONAL| CPCI_PARENT,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 16, 0, 0x00, IDT16Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

							  {"IDT port 20 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 20, 0, 0x00, IDT16Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};

pci_Device PES32BDevices2[]= {{"IDT port 4 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT ,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 4, 0, 0x00, IDT4Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT4_ERROR, NULL},

							 {"IDT port 8 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 8, 0, 0x00, IDT8Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

							 {"IDT port 12 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808C, 12, 0, 0x00, IDT12Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},

							 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 16, 0, 0x00, IDT16Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

							  {"IDT port 17 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							 PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 17, 0, 0x00, IDT17Regs,
							 {0x00,0x00,0x00}, NULL, NULL, NULL, IDT17_ERROR, NULL},

							 {"IDT port 18 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 18, 0, 0x00, IDT18Regs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, IDT18_ERROR, NULL},

							 {"IDT port 19 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 19, 0, 0x00, IDT19Regs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, IDT19_ERROR, NULL},

							  {"IDT port 20 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 20, 0, 0x00, IDT20Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0, NULL},
							};
pci_Device PES32BDevices3[]= {{"IDT port 4 (B) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT ,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 4, 0, 0x00, IDT4Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT4_ERROR, NULL},

							 {"IDT port 8 (A) [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC , 0x111d, 0x808C, 8, 0, 0x00, IDT8Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT8_ERROR, NULL},

							 {"IDT port 12 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1, 0x111d, 0x808C, 12, 0, 0x00, IDT12Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT12_ERROR, NULL},

							 {"IDT port 16 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							  PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 16, 0, 0x00, IDT16Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT16_ERROR, NULL},

							  {"IDT port 17 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							 PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 17, 0, 0x00, IDT17Regs,
							 {0x00,0x00,0x00}, NULL, NULL, NULL, IDT17_ERROR, NULL},

							 {"IDT port 18 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 18, 0, 0x00, IDT18Regs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, IDT18_ERROR, NULL},

							 {"IDT port 19 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 19, 0, 0x00, IDT19Regs,
							{0x00,0x00,0x00}, NULL, NULL, NULL, IDT19_ERROR, NULL},

							  {"IDT port 20 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 20, 0, 0x00, IDT20Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							  {"IDT port 21 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 21, 0, 0x00, IDT21Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							  {"IDT port 22 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 22, 0, 0x00, IDT22Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

							  {"IDT port 23 [PES32NT8AG2]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT,
							   PCI_SINGLE_FUNC | PXMC_XMC1 | PXMC_OPTIONAL, 0x111d, 0x808C, 23, 0, 0x00, IDT23Regs,
							  {0x00,0x00,0x00}, NULL, NULL, NULL, IDT20_ERROR, NULL},

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
							  PCI_MULTI_FUNC, 0x111d, 0x808C, 0, 0, 0x00, IDTRegs, {0x00,0x00,0x00},
							  (void*)PES32MDevices, (void*)PES32rS1Devices, NULL, IDT_ERROR, NULL},
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
pci_Device I82580Devices[]= {{"Intel 82580 copper", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x1516, 0, 1, 0x00,
							  I825801Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, I825801_ERROR, NULL},
							{NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						   };

pci_Regs   I825800Regs[]  = {{"INTRLINE", 0x3C, 0xff},
							 {NULL  , 0x00, 0x00}
						    };
pci_Device RP5BDevices[]  = {{"Intel 82580 copper", PCI_DEVICE|PCI_OPTIONAL, PCI_MULTI_FUNC, 0x8086, 0x1516, 0, 0, 0x00,
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
pci_Device RP1MDevices[] = {{"PCI EXPRESS Root Port 5 [Cougar Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_SINGLE_FUNC,
		  					  0x8086, 0x1c18, 28, 4, 0x00, RP5Regs, {0x00,0x00,0x00}, NULL, (void*)/*RP5BCuFibreDevices*/RP5BDevices,
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
pci_Device LPCDevices[]  = {
							{"SATA 1 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c01,
							 31, 2, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA1_ERROR},
							{"SMBUS [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c22,
							  31, 3, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},
							 {"UnKnown Device", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c23,
							  31, 4, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},
							{"SATA 2 [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c09,
							  31, 5, 0x00, SATA1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, SATA2_ERROR},
							 {"Intel Thermal Sensor Device", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c24,
							  31, 6, 0x00, SMBUSRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, SMBUS_ERROR},

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
							  {0x00,0x00,0x00}, NULL, (void*)PCIE0BDevices, NULL, PCIE0_ERROR, NULL},

							{"Internal Graphics Device [Sandy Bridge]", PCI_DEVICE, PCI_SINGLE_FUNC,
							  0x8086, 0x0116, 2, 0, 0x00, IGD0Regs, {0x00,0x00,0x00}, NULL, NULL,
							  NULL, IGD0_ERROR, NULL},

							 {"Internal Graphics Device [Sandy Bridge]", PCI_DEVICE, PCI_SINGLE_FUNC,
							  0x8086, 0x0103, 4, 0, 0x00, IGD0Regs, {0x00,0x00,0x00}, NULL, NULL,
							  NULL, IGD0_ERROR, NULL},

							{"Ethernet [Cougar Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1502, 25, 0, 0x00,
							  MEIRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, MEI_ERROR, NULL },

							{"USB EHCI 2 [Cougar Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1c2d, 26, 0,
							  0x00, EHCI2Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI2_ERROR, NULL },

							{"PCI EXPRESS Root Port 1 [Cougar Point]", PCI_BRIDGE | PCI_EXPRESS, PCI_MULTI_FUNC,
							  0x8086, 0x1c10, 28, 0, 0x00, RP1Regs, {0x00,0x00,0x00}, (void*)RP1MDevices,
							  NULL, NULL, RP1_ERROR, NULL},

							{"USB EHCI 1 [Cougar Point]", PCI_DEVICE, PCI_SINGLE_FUNC, 0x8086, 0x1c26, 29, 0,
							  0x00, EHCI1Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, EHCI1_ERROR, NULL },

							{"LPC Interface [Cougar Point]", PCI_DEVICE, PCI_MULTI_FUNC, 0x8086, 0x1c4f, 31, 0,
							  0x00, LPCRegs, {0x00,0x00,0x00}, (void*)LPCDevices, NULL, NULL, LPC_ERROR, NULL },

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
pci_Device trXmcPEX86192D[] = {{"PI7C9X130", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END | PXMC_PARENT, PCI_SINGLE_FUNC | PXMC_PMC1,
							    0x12d8, 0xe130, 0, 0, 0x00, trXmcPI7C9XRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, TRXMC_PI7C9X_ERROR},

							   {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00},
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

pci_Device trXmcPEX8619Devices[]={ {"PEX8619 D1 [TRXMC]", PCI_BRIDGE | PCI_EXPRESS | PXMC_PARENT, PCI_SINGLE_FUNC | PXMC_XMC1, 0x10b5,
								    0x8619, 1, 0, 0x00, trXmcPEX86191Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, TRXMC_PEX86191_ERROR, NULL},

				                   {"PEX8619 D2 [TRXMC]", PCI_BRIDGE | PCI_EXPRESS, PCI_SINGLE_FUNC, 0x10b5, 0x8619,
				                    2, 0, 0x00, trXmcPEX86192Regs, {0x00,0x00,0x00}, NULL, (void*)trXmcPEX86192D, NULL,
				                    TRXMC_PEX86192_ERROR, NULL},

								   {"PEX8619 D3 [TRXMC]", PCI_BRIDGE | PCI_EXPRESS | CPCI_PARENT, PCI_SINGLE_FUNC, 0x10b5, 0x8619,
				                    3, 0, 0x00, trXmcPEX86195Regs, {0x00,0x00,0x00}, NULL, NULL, NULL, TRXMC_PEX86193_ERROR, trXmcPEX86193Option},

								   {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
							      };



pci_Regs   trXmcPEX8619MRegs[]   = {{"INTRLINE", 0x3C, 0xff},
									{NULL  , 0x00, 0x00}
						           };
pci_Device trXmcPEX8619rMDevice[]={{"PEX8619 M1 [TRXMC]", PCI_DEVICE | PCI_EXPRESS, PCI_MULTI_FUNC, 0x10b5, 0x8619,
				                    0, 1, 0x00, trXmcPEX8619MRegs, {0x00,0x00,0x00}, NULL, NULL, NULL, TRXMC_PEX8619M_ERROR, NULL},

							       {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						          };


pci_Regs   trXmcPEX8619rRegs[]= {{"INTRLINE", 0x3C, 0xff},
								 {NULL  , 0x00, 0x00}
						        };
pci_Device trXmcDevices[] = {{"PEX8619 Root [TRXMC]", PCI_BRIDGE | PCI_EXPRESS | PCI_EXPRESS_END, PCI_MULTI_FUNC,
							  0x10b5, 0x8619, 0, 0, 0x00, trXmcPEX8619rRegs, {0x00,0x00,0x00}, (void*)trXmcPEX8619rMDevice,
							  (void*)trXmcPEX8619Devices, NULL, TRXMC_PEX8619r_ERROR, NULL},

							 {NULL, 0, 0, 0, 0, 0, 0, 0, NULL, {0,0,0}, NULL, NULL, NULL, 0x00, NULL},
						    };

pci_DB  trXMCPCIDB = { 0x00, NULL, NULL, NULL, trXmcDevices, TRXMC_ERROR_BASE};


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

	temp = ((UINT8)dIoReadReg(0x211, REG_8));
	
	if(temp & 0x20)		//the board does not report XMC presence
		rt |= PXMC_XMC1;

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



pci_DB  brdPCIDB = { 0x00, brdChkPXMC, brdChkSatellite, brdChkPeripheral, pciDevices0, PCI_ERROR_BASE};



/*****************************************************************************
 * brdGetPCIDB: returns the SIO global data structure
* RETURNS: pci_DB*
* */
UINT32 brdGetPCIDB(void *ptr)
{
/*
	UINT8 temp, per;
	UINT16 wTemp = 0;
	UINT16 wGpioBase = 0;
	PCI_PFA	pfa;
	UINT8 bVal = 0;

	per = ((UINT8)dIoReadReg(0x31E, REG_8));
	temp = (((UINT8)dIoReadReg(0x31E, REG_8)) >> 4) & 0x03;

	if( (per & 0x04) != 0x04)
	{
		switch(temp)
		{

			case 0x00:
				((pci_Device*)(pciDevices0[1].BDevices))[0].BDevices = (void*)PES32BDevices0;
				break;

			case 0x01:
				((pci_Device*)(pciDevices0[1].BDevices))[0].BDevices = (void*)PES32BDevices1;
				break;

			case 0x02:
				((pci_Device*)(pciDevices0[1].BDevices))[0].BDevices = (void*)PES32BDevices2;
				break;

			case 0x03:
				((pci_Device*)(pciDevices0[1].BDevices))[0].BDevices = (void*)PES32BDevices3;
				break;
		}
	}
	else
	{
		switch(temp)
		{
			case 0x00:
				((pci_Device*)(pciDevices0[1].BDevices))[0].BDevices = (void*)PES32rS0Devices;
				break;

			case 0x01:
				((pci_Device*)(pciDevices0[1].BDevices))[0].BDevices = (void*)PES32rS1Devices;
				break;

			case 0x02:
				((pci_Device*)(pciDevices0[1].BDevices))[0].BDevices = (void*)PES32rS2Devices;
				break;

			case 0x03:
				((pci_Device*)(pciDevices0[1].BDevices))[0].BDevices = (void*)PES32rS3Devices;
				break;
		}
	}
    pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
	wTemp = dIoReadReg (wGpioBase+ GP_LVL, REG_16);
	if ( (wTemp & 0x8000) == 0x8000 )
	{
		((pci_Device*)(pciDevices0[6].MDevices))[0].BDevices = (void*)RP5BCuFibreDevices;
	}
	else
	{
		((pci_Device*)(pciDevices0[6].MDevices))[0].BDevices = (void*)RP5BDevices;
	}
*/
	*((pci_DB**)ptr) = &brdPCIDB;

	return E__OK;
}

/*****************************************************************************
 * brdCheckPcieSwConfig: checks the correct config for TRXMC test
 * RETURNS: E__OK
 * */

UINT32 brdCheckPcieSwConfig(void *ptr)
{
/*
	UINT8 temp, per;

	per = ((UINT8)dIoReadReg(0x31E, REG_8));
	temp = (((UINT8)dIoReadReg(0x31E, REG_8)) >> 4) & 0x03;

	if( (per & 0x04) != 0x04)
	{
		((PCIE_SW_CONFIG_INFO*)ptr)->tye = 1;
		switch(temp)
		{
			case 0x00:
				//1 - x4 device, 1 - x4 bridge
				((PCIE_SW_CONFIG_INFO*)ptr)->count = 1;
				break;

			case 0x01:
				//1 - x4 device, 4 - x1 bridges
				((PCIE_SW_CONFIG_INFO*)ptr)->count = 4;
				break;

			case 0x02:
				//1 - x4 device, 1 - x4 bridge
				((PCIE_SW_CONFIG_INFO*)ptr)->count = 1;
				break;

			case 0x03:
				//1 - x8 device
				((PCIE_SW_CONFIG_INFO*)ptr)->count = 0;
				break;
		}
	}
	else
	{
	((PCIE_SW_CONFIG_INFO*)ptr)->tye = 0;
		switch(temp)
		{
			case 0x00:
				//8 - x1 bridges
				((PCIE_SW_CONFIG_INFO*)ptr)->count = 8;
				break;

			case 0x01:
				//1 - x4 & 4 - x1 bridges
				((PCIE_SW_CONFIG_INFO*)ptr)->count = 5;
				break;

			case 0x02:
				//2 - x4 bridges
				((PCIE_SW_CONFIG_INFO*)ptr)->count = 2;
				break;

			case 0x03:
				//1 - x8 bridges
				((PCIE_SW_CONFIG_INFO*)ptr)->count = 1;
				break;
		}
	}
*/

	// read PCIe configuration register from switch

	return E__OK;
}


/*****************************************************************************
 * brdGetTrXmcParentBus: returns the parent bus of the TRXMC
 * RETURNS: E__OK
 * */
UINT32 brdGetTrXmcParentBus(void *ptr)
{
	*((UINT8*)ptr) = ((pci_Device*)((pci_Device*)brdPCIDB.Devices0[1].BDevices)[0].BDevices)[*((UINT8*)ptr)].Vars.sBUS;

	return E__OK;
}

