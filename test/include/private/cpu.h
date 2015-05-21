#ifndef __cpu_h__
	#define __cpu_h__

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

/* cpu.h - CPU primitives (cpu.c)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/cpu.h,v 1.1 2013-09-04 07:40:41 chippisley Exp $
 *
 * $Log: cpu.h,v $
 * Revision 1.1  2013-09-04 07:40:41  chippisley
 * Import files into new source repository.
 *
 * Revision 1.7  2010/09/15 12:20:24  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.6  2010/06/23 11:00:13  hmuneer
 * CUTE V1.01
 *
 * Revision 1.5  2009/09/29 12:29:28  swilson
 * Add function for throwing a GPF
 *
 * Revision 1.4  2008/09/17 13:48:48  swilson
 * Add CPU halt function.
 *
 * Revision 1.3  2008/07/18 12:25:27  swilson
 * Revisions to MMU prototypes.
 *
 * Revision 1.2  2008/05/29 10:09:50  swilson
 * Add local implementation of HAL memory functions. Modify loader to configure the runtime GDT and IDT.
 *
 * Revision 1.1  2008/05/14 09:59:08  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/05/12 15:04:40  swilson
 * Add IBIT console. Debugging various library functions.
 *
 */


/* includes */

#include <stdtypes.h>

/* defines */

	/* System Selectors */

#define NULL_SEL		0x0000

#define CS_SEL			0x0008
#define DS_SEL			0x0010

#define USER_SEL		0x0080
#define END_SEL			0x03F8

	/* Descriptor fields */

#define DESC_G			0x8000		/* 4kB granularity */
#define DESC_B			0x4000		/* big */
#define DESC_AVL		0x1000		/* available */

#define DESC_P			0x0080		/* present */
#define DESC_DPL0		0x0000		/* descriptor privilege */
#define DESC_DPL1		0x0020		/*			-"-			*/
#define DESC_DPL2		0x0040		/*			-"-			*/
#define DESC_DPL3		0x0060		/*			-"-			*/

#define DESC_E			0x0004		/* data: expand-up */
#define DESC_W			0x0002		/* data: writeable */
#define DESC_C			0x0004		/* code: conforming */
#define DESC_R			0x0002		/* code: read only */
#define DESC_A			0x0001		/* accessed */


#define GDT_TYPE_RO		0xC090		/* read-only data */
#define GDT_TYPE_RW		0xC092		/* read-write data */

#define GDT_TYPE_XO		0xC098		/* execute only code */
#define GDT_TYPE_XR		0xC09A		/* read-execute code */

/* typedefs */

typedef struct tagCpuRegs
{
	UINT32	eax;	
	UINT32	ebx;
	UINT32	edx;
	UINT32	ecx;

} CPUID_REGS;


/* constants */

/* locals */

/* globals */

/* externals */

	/* cpu.S */

extern void sysCpuid (UINT32 dFunc, CPUID_REGS* psRegs);

extern void sysCacheEnable (void);
extern void sysCacheDisable (void);
extern void sysInvalidateTLB (void);
extern void _sysThrowGpf (void);

extern void sysHalt (void);

	/* gdt.S */

extern int sysGdtWriteSlot (UINT16 dSlot, UINT64 qDescriptor);
extern void sysGdtDeleteAllSlots (void);
extern UINT16 sysGdtFindFreeSlot (void);
extern UINT16 sysGdtFindFreeSlotRes (void);

	/* mmu.c */

extern UINT64 sysCreateDescriptor (UINT32 dBase, UINT32 dLength, UINT16 wAttribs);

extern void sysMmuInitialize (void);
extern void sysMmuFreeAll (void);
extern UINT32 sysMmuMapBlock (UINT16 wHandle, UINT64 qBase, UINT32 dLength);
extern void sysMmuUnmapBlock (UINT16 wHandle);
extern void sysMmuUnmapAll (void);

extern UINT32 sysMallocBlock (UINT32 dBufferSize);

extern UINT32 sysMemGetPhysPtrLocked(UINT64	qPhysicalAddress, UINT32 dBlockSize);

/* forward declarations */


#endif

