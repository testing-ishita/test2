#ifndef __pci_devices_h__
	#define __pci_devices_h__

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

/* pci_devices.h - PCI Vendor and Device codes
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/pci_devices.h,v 1.1 2013-09-04 07:40:27 chippisley Exp $
 *
 * $Log: pci_devices.h,v $
 * Revision 1.1  2013-09-04 07:40:27  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2009/05/18 09:25:29  hmuneer
 * no message
 *
 * Revision 1.3  2008/09/22 16:15:27  swilson
 * Add parameter-passing to makefile so additional warnings can be passed to build and
 *  board type can be specified.
 *
 * Revision 1.2  2008/07/24 14:53:01  swilson
 * Fix incorrectly formatted comments.
 *
 * Revision 1.1  2008/06/20 12:16:58  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 */


/* includes */

#include <stdtypes.h>

/* defines */

/*--------------------------------------------------------------------------*/

#define PCI_VID_LSI				0x1000	/* LSI Logic/Symbios */

#define PCI_DID_SYM53C825		0x0003	/* Sym53c825 */
#define PCI_DID_SYM53C875		0x000F	/* Sym53c875 */
#define PCI_DID_SYM53C895		0x0010	/* Sym53c895 */
#define PCI_DID_SYM53C895A		0x0012	/* Sym53c895A */
#define PCI_DID_SYM53C10X0		0x0021	/* Sym53c10x0 */

/*--------------------------------------------------------------------------*/

#define PCI_VID_ATI				0x1002	/* ATI */

#define PCI_DID_MOBILITY_7500	0x4C57	/* Radeon Mobility 7500 */

#define ATI_MR7500      0x0601

/*--------------------------------------------------------------------------*/

#define PCI_VID_NEC				0x1033	/* NEC */

#define PCI_DID_				0x0125	/* PCIe-PCIX Bridge */

/*--------------------------------------------------------------------------*/

#define PCI_VID_PLX				0x10B5	/* PLX TECHNOLOGY */

#define PCI_DID_HB8				0x6540	/* HB8 (transparent) */
#define PCI_DID_HB8_NT			0x6542	/* HB8 (non-transparent) */

#define PCI_DID_8114			0x8114	/* 8114 PCIe-PCIX Bridge */

/*--------------------------------------------------------------------------*/

#define PCI_VID_TUNDRA			0x10E3	/* Tundra */

#define PCI_DID_UNIVERSE_II		0x0000	/* Universe/Universe-II */

/*--------------------------------------------------------------------------*/

#define PCI_VID_SERVERWORKS		0x1166	/* ServerWorks */

#define PCI_DID_CNB30LE			0x0009	/* CNB30LE North Bridge */
#define PCI_DID_CIOBX2			0x0101	/* CIOB-X2 */

#define PCI_DID_CSB5			0x0201	/* CSB5 South Bridge */
#define PCI_DID_CSB5_IDE		0x0212	/* CSB5 IDE Controller */
#define PCI_DID_CSB5_USB		0x0220	/* CSB5 USB */
#define PCI_DID_CSB5_LPC		0x0225	/* CSB5 LPC */
#define PCI_DID_CSB5_XIOAPIC	0x0230	/* CSB5 XIOAPIC */

/*--------------------------------------------------------------------------*/

#define PCI_VID_CCT				0x125F	/* CCT */

#define PCI_DID_CCPMC_232		0x2071	/* CC PMC/232 */
#define PCI_DID_CCPMC_23P		0x2084	/* CC PMC/23P */
#define PCI_DID_CCPMC_422		0x2091	/* CC PMC/422 */
#define PCI_DID_CLPMC_596		0x2042	/* CL PMC/596 */

/*--------------------------------------------------------------------------*/

#define PCI_VID_SILICON_MOTION	0x126F	/* Silicon Motion */

#define PCI_DID_SM722			0x0720	/* SM722 */
#define PCI_DID_SM731			0x0730	/* SM731 */

/*--------------------------------------------------------------------------*/

#define PCI_VID_HINT			0x3388	/* PLX (HiNT) */

#define PCI_DID_HB6				0x0020	/* HB6 (transparent) */
#define PCI_DID_HB6_NT			0x0021	/* HB6 (non-transparent) */
#define PCI_DID_HB2				0x0026	/* HB2 */

/*--------------------------------------------------------------------------*/

#define PCI_VID_S3				0x5333	/* S3 */

/*--------------------------------------------------------------------------*/

#define PCI_VID_INTEL			0x8086	/* Intel */

#define PCI_DID_82543_FI		0x1001	/* 82543 Ethernet (Fi) */
#define PCI_DID_82543_CU		0x1004	/* 82543 Ethernet (Cu) */
#define PCI_DID_82544			0x1008	/* 82544 Ethernet */
#define PCI_DID_82540			0x100E	/* 82540 Ethernet */
#define PCI_DID_82546EB			0x1010	/* 82546EB Ethernet */
#define PCI_DID_82571EB_CU		0x105E	/* 82571EB Ethernet (Cu) */
#define PCI_DID_82571EB_FI		0x105F	/* 82571EB Ethernet (Fi) */
#define PCI_DID_82571EB_SD		0x1060	/* 82571EB Ethernet (Sd) */
#define PCI_DID_82546GB			0x1079	/* 82546GB Ethernet */
#define PCI_DID_82541PI			0x107C	/* 82541PI Ethernet */
#define PCI_DID_82573L			0x109A	/* 82573L Ethernet */
#define PCI_DID_82559ER			0x1209	/* 82559ER Ethernet */
#define PCI_DID_82551			0x1229	/* 82551 Ethernet */

#define PCI_DID_ICH				0x244E	/* ICH PCI Bridge */

#define PCI_DID_6300ESB_LPC		0x25A1	/* 6300ESB LPC I/F */
#define PCI_DID_6300ESB_IDE		0x25A2	/* 6300ESB IDE (PATA) */
#define PCI_DID_6300ESB_SATA	0x25A3	/* 6300ESB SATA (IDE) */
#define PCI_DID_6300ESB_SMBUS	0x25A4	/* 6300ESB SMBus */
#define PCI_DID_6300ESB_AC97_A	0x25A6	/* 6300ESB AC97 Audio */
#define PCI_DID_6300ESB_AC97_M	0x25A7	/* 6300ESB AC97 Modem */
#define PCI_DID_6300ESB_UHCI1	0x25A9	/* 6300ESB USB 1.1a */
#define PCI_DID_6300ESB_UHCI2	0x25AA	/* 6300ESB USB 1.1b */
#define PCI_DID_6300ESB_WD		0x25AB	/* 6300ESB Watchdog */
#define PCI_DID_6300ESB_APIC	0x25AC	/* 6300ESB APIC */
#define PCI_DID_6300ESB_EHCI	0x25AD	/* 6300ESB USB 2.0 */
#define PCI_DID_6300ESB_PCIX	0x25AE	/* 6300ESB PCI-X Bridge */
#define PCI_DID_6300ESB_RAID	0x25B0	/* 6300ESB SATA (RAID) */

#define PCI_DID_945GM_MCH		0x27A0	/* 945GM MCH */
#define PCI_DID_945GM_IGD1		0x27A2	/* 945GM IGD */
#define PCI_DID_945GM_IGD2		0x27A6	/* 945GM IGD */
#define PCI_DID_945GM_MCH_A3	0x27AC	/* 945GM MCH (A3) */

#define PCI_DID_ICH7M_LPC		0x27BD	/* ICH7-M (DH) LPC */
#define PCI_DID_ICH7M_SATA		0x27C4	/* ICH7-M SATA */
#define PCI_DID_ICH7M_RAID		0x27C6	/* ICH7-M RAID */
#define PCI_DID_ICH7M_UHCI1		0x27C8	/* ICH7-M UHCI #1 */
#define PCI_DID_ICH7M_UHCI2		0x27C9	/* ICH7-M UHCI #2 */
#define PCI_DID_ICH7M_UHCI3		0x27CA	/* ICH7-M UHCI #3 */
#define PCI_DID_ICH7M_UHCI4		0x27CB	/* ICH7-M UHCI #4 */
#define PCI_DID_ICH7M_EHCI		0x27CC	/* ICH7-M EHCI */
#define PCI_DID_ICH7M_PCIE1		0x27D0	/* ICH7-M PCIe #1 */
#define PCI_DID_ICH7M_PCIE2		0x27D2	/* ICH7-M PCIe #2 */
#define PCI_DID_ICH7M_PCIE3		0x27D4	/* ICH7-M PCIe #3 */
#define PCI_DID_ICH7M_PCIE4		0x27D6	/* ICH7-M PCIe #4 */
#define PCI_DID_ICH7M_HD_AUDIO	0x27D8	/* ICH7-M HD-Audio */
#define PCI_DID_ICH7M_SMBUS		0x27DA	/* ICH7-M SMBUS */
#define PCI_DID_ICH7M_PATA		0x27DF	/* ICH7-M PATA */
#define PCI_DID_ICH7M_PCIE5		0x27E0	/* ICH7-M PCIe #5 */
#define PCI_DID_ICH7M_PCIE6		0x27E2	/* ICH7-M PCIe #6 */
#define PCI_DID_ICH7M_PCI		0x2448	/* ICH7-M PCI Bridge */

#define PCI_DID_855GME_HUB		0x3580	/* 855GME Host Hub I/F */
#define PCI_DID_855GME_IGD		0x3582	/* 855GME Graphics Dev */
#define PCI_DID_855GME_MC		0x3584	/* 855GME Memory Ctrl */
#define PCI_DID_855GME_CONFIG	0x3585	/* 855GME Config Proc */

#define PCI_DID_E7520_MCH		0x3590	/* E7520 MCH */
#define PCI_DID_E7520_MCH_RAS	0x3591	/* E7520 MCH (RAS) */
#define PCI_DID_E7520_DMA		0x3594	/* E7520 DMA Engine */
#define PCI_DID_E7520_PCIE_A0	0x3595	/* E7520 PCIe A0 */
#define PCI_DID_E7520_PCIE_A1	0x3596	/* E7520 PCIe A1 */
#define PCI_DID_E7520_PCIE_B0	0x3597	/* E7520 PCIe B0 */
#define PCI_DID_E7520_PCIE_B1	0x3598	/* E7520 PCIe B1 */
#define PCI_DID_E7520_PCIE_C0	0x3599	/* E7520 PCIe C0 */
#define PCI_DID_E7520_PCIE_C1	0x359A	/* E7520 PCIe C1 */

#define PCI_DID_LE3100_MCH		0x35B0	/* LE3100 MCH */
#define PCI_DID_LE3100_DRC		0x35B1	/* LE3100 DRC */
#define PCI_DID_LE3100_EDMA		0x35B5	/* LE3100 EDMA */
#define PCI_DID_LE3100_PCIE_A0	0x35B6	/* LE3100 PCIe A */
#define PCI_DID_LE3100_PCIE_A1	0x35B7	/* LE3100 PCIe A1 */
#define PCI_DID_LE3100_ECR		0x35C8	/* LE3100 ECR */
#define PCI_DID_LE3100_PCIE_B0	0x2690	/* LE3100 PCIe B */
#define PCI_DID_LE3100_PCIE_B1	0x2692	/* LE3100 PCIe B1 */
#define PCI_DID_LE3100_PCIE_B2	0x2694	/* LE3100 PCIe B2 */
#define PCI_DID_LE3100_PCIE_B3	0x2696	/* LE3100 PCIe B3 */
#define PCI_DID_LE3100_UHCI1	0x2688	/* LE3100 UHCI #1 */
#define PCI_DID_LE3100_UHCI2	0x2689	/* LE3100 UHCI #2 */
#define PCI_DID_LE3100_EHCI		0x268C	/* LE3100 EHCI */
#define PCI_DID_LE3100_LPC		0x2670	/* LE3100 LPC */
#define PCI_DID_LE3100_SATA		0x2680	/* LE3100 SATA */
#define PCI_DID_LE3100_SMBUS	0x269B	/* LE3100 SMBUS */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */

#endif

