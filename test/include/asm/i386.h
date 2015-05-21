#ifndef __i386_h__
	#define __i386_h__

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

/* i386.h - Intel CPU Specific Assembler Defines
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/asm/i386.h,v 1.1 2013-09-04 07:28:59 chippisley Exp $
 *
 * $Log: i386.h,v $
 * Revision 1.1  2013-09-04 07:28:59  chippisley
 * Import files into new source repository.
 *
 * Revision 1.9  2011/05/16 14:30:56  hmuneer
 * Info Passing Support
 *
 * Revision 1.8  2010/09/15 12:16:58  hchalla
 * Implemented new API for locking the memory after allocation.
 *
 * Revision 1.7  2010/03/31 16:28:47  hmuneer
 * no message
 *
 * Revision 1.6  2009/06/08 16:15:35  swilson
 * Add definition for number of S/W interrupts; make IDT_SLOTS use this when
 *  setting IDT size.
 *
 * Revision 1.5  2009/05/20 09:04:48  swilson
 * Increase the number of IDT slots.
 *
 * Revision 1.4  2008/07/10 16:51:32  swilson
 * Add calibrated delay fuunctions. Re-code loader to support SMP initialization and
 *  add SMP init.
 *
 * Revision 1.3  2008/07/02 10:52:45  swilson
 * Enable >4GB memory access via paged memory mechanism (PSE-36).
 *
 * Revision 1.2  2008/05/29 10:09:50  swilson
 * Add local implementation of HAL memory functions. Modify loader to configure the runtime GDT and IDT.
 *
 * Revision 1.1  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 */


.set ICR_LOW,		0xFEE00300
.set SVR,			0xFEE000F0
.set APIC_ID,		0xFEE00020
.set LVT3,			0xFEE00370
.set APIC_ENABLED,	0x0100
.set COUNT,			0x00
.set VACANT,		0x00

	/* System Selectors */

.set NULL_SEL,	0x0000			# GDT entry: NULL selector

.set CS_SEL,	0x0008			# GDT entry: code selector
.set DS_SEL,	0x0010			# GDT entry: data selector

.set RES_SEL,		0x0080			# GDT entry: first reserved selector
.set END_RES_SEL,	0x00F8			# GDT entry: last  reserved selector
.set USER_SEL,	    0x0100			# GDT entry: first user selector
.set END_SEL,	    0x03F8			# GDT entry: last user selector


	/* GDT and IDT Definitions */

.set SW_SLOTS,	4				# number of software interrupt slots

.set GDT_BASE,	0x00000000		# base address of GDT
.set GDT_SLOTS,	128				# number of GDT slots

.set IDT_BASE,	0x00001000		# base address of IDT
.set IDT_SLOTS,	88 + SW_SLOTS	# number of IDT slots


	/* Paged Memory Definitions */

.set PDIR_BASE,	0x00002000		# base address of Page Directory (4KB aligned)
.set PDIR_ENTRIES, 1024			# number of directory entries

#endif

