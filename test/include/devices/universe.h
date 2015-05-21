/************************************************************************
 *                                                                      *
 *      Copyright 2015 Concurrent Technologies, all rights reserved.    *
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

/*
 * universe.h
 */
 
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/universe.h,v 1.2 2015-03-05 11:45:16 hchalla Exp $
 * $Log: universe.h,v $
 * Revision 1.2  2015-03-05 11:45:16  hchalla
 * Added more register definitions for universe chip II.
 *
 * Revision 1.1  2015-02-25 17:52:00  hchalla
 * Initial checkin for universe II chip header file.
 *
 *
 */

#ifndef INCLUDE_DEVICES_UNIVERSE_H_
#define INCLUDE_DEVICES_UNIVERSE_H_

#define VID_TUNDRA		0x10E3
#define DID_UNIVERSE	0x0000
#define UNIV_DEVID_VENID	0x000010E3

/* Register addresses */
/* ------------------ */

#define CR_CSR_OFS	0x7F000	/* Base offset for CR/CSR accesses = 508kBytes */

#define PCI_CSR		0x4		/* PCI Configuration Space Control & Status */
#define S_TA	0x08000000	/* target abort */

#define LSI0_CTL	0x100	/* PCI Slave Image 0 - Control				*/
#define LSI0_BS		0x104	/*		    - Base address register			*/
#define LSI0_BD		0x108	/*		    - Bound address register		*/
#define LSI0_TO		0x10C	/*		    - Translation offset			*/

#define LSI1_CTL	0x114	/* PCI Slave Image 1						*/
#define LSI1_BS		0x118	/*											*/
#define LSI1_BD		0x11C	/*											*/
#define LSI1_TO		0x120	/*											*/

#define LSI2_CTL	0x128	/* PCI Slave Image 2						*/
#define LSI2_BS		0x12C	/*											*/
#define LSI2_BD		0x130	/*											*/
#define LSI2_TO		0x134	/*											*/

#define LSI3_CTL	0x13C	/* PCI Slave Image 3						*/
#define LSI3_BS		0x140	/*											*/
#define LSI3_BD		0x144	/*											*/
#define LSI3_TO		0x148	/*											*/

#define SCYC_CTL	0x170	/* Special Cycle Control Register			*/
#define SCYC_ADDR	0x174	/* Special Cycle PCI bus Address Register	*/
#define SCYC_EN		0x178	/* Special Cycle Swap/Compare Data			*/
#define SCYC_CMP	0x17C	/* Special Cycle Compare Data Register		*/
#define SCYC_SWP	0x180	/* Special Cycle Swap Data Register			*/

#define LMISC		0x184	/* PCI Miscellaneous Register				*/
#define SLSI		0x188	/* Special PCI Slave Image					*/
#define CMDERR		0x18C	/* PCI Command Error Log Register			*/
#define LAERR		0x190	/* PCI Address Error Log					*/

#define DCTL		0x200	/* DMA Transfer Control Register			*/
#define DTBC		0x204	/* DMA Transfer Byte Count Register			*/
#define DLA			0x208	/* DMA PCI bus Address Register				*/
#define DVA			0x210	/* DMA VMEbus Address Register				*/
#define DCPP		0x218	/* DMA Command Packet Pointer				*/
#define DGCS		0x220	/* DMA General Control and Status Register	*/
#define D_LLUE		0x224	/* DMA Linked List Update Enable register	*/

#define LINT_EN		0x300	/* PCI Interrupt Enable						*/
#define LINT_STAT	0x304	/* PCI Interrupt Status						*/
#define LINT_MAP0	0x308	/* PCI Interrupt Map0						*/
#define LINT_MAP1	0x30C	/* PCI Interrupt Map1						*/

#define VINT_EN		0x310	/* VMEbus Interrupt Enable					*/
#define VINT_STAT	0x314	/* VMEbus Interrupt Status					*/
#define VINT_MAP0	0x318	/* VMEbus Interrrupt Map0					*/
#define VINT_MAP1	0x31C	/* VMEbus Interrrupt Map1					*/
#define VINT_MAP2	0x344 /* PCI Interrupt Map1						*/
#define LINT_MAP2	0x340 /* PCI Interrupt Map1						*/

#define STATID		0x320	/* Interrupt Status/ID Out					*/

#define V1_STATID	0x324	/* VIRQ1 Status/ID							*/
#define V2_STATID	0x324	/* VIRQ2 Status/ID							*/
#define V3_STATID	0x324	/* VIRQ3 Status/ID							*/
#define V4_STATID	0x324	/* VIRQ4 Status/ID							*/
#define V5_STATID	0x324	/* VIRQ5 Status/ID							*/
#define V6_STATID	0x324	/* VIRQ6 Status/ID							*/
#define V7_STATID	0x324	/* VIRQ7 Status/ID							*/

#define MAST_CTL	0x400	/* Master Control							*/
#define MISC_CTL	0x404	/* Miscellaneous Control					*/

	#define	SYSCON		0x00020000		/* System controller			*/
	#define V64AUTO		0x00010000		/* VME64 Auto-ID				*/

#define MISC_STAT	0x408	/* Miscellaneous Status						*/
#define USER_AM		0x40C	/* User AM Codes Register					*/

#define VSI0_CTL	0x0F00	/* VMEbus Slave Image 0 - Control			*/
#define VSI0_BS		0x0F04	/*		       - Base Address Register		*/
#define VSI0_BD		0x0F08	/*		       - Bound Address Register		*/
#define VSI0_TO		0x0F0C	/*		       - Translation Offset			*/

#define VSI1_CTL	0x0F14	/* VMEbus Slave Image 1						*/
#define VSI1_BS		0x0F18	/*											*/
#define VSI1_BD		0x0F1C	/*											*/
#define VSI1_TO		0x0F20	/*											*/

#define VSI2_CTL	0x0F28	/* VMEbus Slave Image 2						*/
#define VSI2_BS		0x0F2C	/*											*/
#define VSI2_BD		0x0F30	/*											*/
#define VSI2_TO		0x0F34	/*											*/

#define VSI3_CTL	0x0F3C	/* VMEbus Slave Image 3						*/
#define VSI3_BS		0x0F40	/*											*/
#define VSI3_BD		0x0F44	/*											*/
#define VSI3_TO		0x0F48	/*											*/

#define VRAI_CTL	0x0F70	/* VMEbus Register Access Image Control Reg	*/
#define VRAI_BS		0x0F74	/* VMEbus Register Access Image Control Reg */

#define VCSR_CTL	0x0F80	/* VMEbus CSR Control Register				*/
#define VCSR_TO		0x0F84	/* VMEbus CSR Translation Offset			*/
#define V_AMERR		0x0F88	/* VMEbus AM Code Error Log					*/
#define VAERR		0x0F8C	/* VMEbus Address Error Log					*/

#define VCSR_CLR	0x0FF4	/* VMEbus CSR Bit Clear Register			*/
#define VCSR_SET	0x0FF8	/* VMEbus CSR Bit Set Register				*/
/*#define VCSR_BS		0x0FFC*/	/* VMEbus CSR Base Address Register			*/
#define LOC_SYSFAIL		0x40000000		/* sysfail bit in VCSR_CLR reg */
#define GLB_SYSFAIL		0x00004000		/* sysfail bit in LINT_STAT reg */

/* Register Bitfields */
/* ------------------ */

#define LSI_EN		0x80000000	/*  Local slave image					*/
#define LSI_PWEN	0x040000000
#define LSI_VDW8	0x000000000
#define LSI_VDW16	0x000400000
#define LSI_VDW32	0x000800000
#define LSI_VDW64	0x000C00000
#define LSI_VAS16	0x000000000
#define LSI_VAS24	0x000020000
#define LSI_VAS32	0x000030000
#define LSI_PGM		0x000008000
#define LSI_SUPER	0x000004000
#define LSI_VCT		0x000000100
#define LSI_LASm	0x000000000
#define LSI_LASi	0x000000001
#define LSI_LASc	0x000000002

#define VSI_EN		0x80000000	/*  Local slave image					*/

#define UNIV_SCONS	0x00020000	/* system controller status				*/

#endif /* INCLUDE_DEVICES_UNIVERSE_H_ */
