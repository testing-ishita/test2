
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

/* BOOT_PROT.S - 2nd-stage Entry Point
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/loader/boot_prot.S,v 1.4 2014-05-12 16:10:15 jthiru Exp $
 *
 * $Log: boot_prot.S,v $
 * Revision 1.4  2014-05-12 16:10:15  jthiru
 * Adding support for VX91x and fixes to existing bugs
 *
 * Revision 1.3  2014-03-06 13:41:49  hchalla
 * Added more reserved MMU memory space from (0x05000000 - 0x06FFFFFF).
 *
 * Revision 1.2  2014-03-04 09:03:43  mgostling
 * Store EDI in _dStartupFlags before testing $BITF_AP
 *
 * Revision 1.1  2013-09-04 07:42:08  chippisley
 * Import files into new source repository.
 *
 * Revision 1.28  2012/11/02 14:15:01  chippisley
 * Added _lapicInit and for pcbrd added call to sysInitPic().
 *
 * Revision 1.27  2011/06/13 12:33:23  hmuneer
 * no message
 *
 * Revision 1.26  2011/05/16 14:31:43  hmuneer
 * Info Passing Support
 *
 * Revision 1.25  2011/03/22 13:46:57  hchalla
 * Added support for SMP for VX813.
 *
 * Revision 1.24  2011/02/28 11:58:52  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.23  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.22  2010/11/10 11:39:45  hchalla
 * Fixed Build error.
 *
 * Revision 1.21  2010/11/10 10:16:04  hchalla
 * Added NMI unmask code to enable NMI's.
 *
 * Revision 1.20  2010/09/13 09:34:34  cdobson
 * Updated SMP initialisation to support for Nehalem CPU.
 * This needs IPI delivery mode set to Level.
 *
 * Revision 1.19  2010/06/23 11:00:52  hmuneer
 * CUTE V1.01
 *
 * Revision 1.18  2010/05/05 15:49:10  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.17  2010/04/23 09:05:55  swilson
 * Add reserved, direct-mapped pages for dynamic heaps.
 *
 * Revision 1.16  2009/06/10 13:59:59  swilson
 * Only call smp_dispatch() once, before entering idle loop. Add Port-80 output inside
 *  idle loop for debug etc. Update comments to reflect new operation.
 *
 * Revision 1.15  2009/06/08 16:07:27  swilson
 * Add code/data for recording APIC IDs for all cores. For AP cores, do a soft enable
 *  via the SVR. Leave interrupts enabled on the APs before halting
 *
 * Revision 1.14  2009/05/20 09:04:04  swilson
 * Add calls to create an IDT and initialise the IOAPIC.
 *
 * Revision 1.13  2009/04/07 15:27:31  swilson
 * Tidy-up comments
 *
 * Revision 1.12  2008/09/29 10:07:10  swilson
 * Rename dBitFlags to dStartupFlags to better indicate their function.
 *
 * Revision 1.11  2008/09/19 14:43:25  swilson
 * Update description of command-line flags.
 *
 * Revision 1.10  2008/07/18 12:43:21  swilson
 * Rename stage-2 signature to prevent confusion.
 *
 * Revision 1.9  2008/07/11 14:44:05  swilson
 * Add call to SMP dispatch code.
 *
 * Revision 1.8  2008/07/10 16:51:30  swilson
 * Add calibrated delay fuunctions. Re-code loader to support SMP initialization and
 *  add SMP init.
 *
 * Revision 1.7  2008/07/02 10:52:46  swilson
 * Enable >4GB memory access via paged memory mechanism (PSE-36).
 *
 * Revision 1.6  2008/06/20 12:16:54  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.5  2008/05/29 10:09:47  swilson
 * Add local implementation of HAL memory functions. Modify loader to configure the runtime GDT and IDT.
 *
 * Revision 1.4  2008/04/30 07:51:19  swilson
 * Add PCI support library.
 *
 * Revision 1.3  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 * Revision 1.2  2008/04/21 16:59:24  swilson
 * Basic framework, now with stack, initialised data and C-to-asm linkage all apparently working.
 *
 */

.include "asm/i386.h"

.set APIC_BASE,		0xFEE00000		# Local (CPU) APIC
.set APIC_EOI,		0xFEE000B0		# EOI port
.set APIC_LINT0,	0x350			# EOI port
.set APIC_LINT1,	0x360			# EOI por

# BIT Flags (passed in EDI)

.set BITF_DIAG,		0x00000001		# 1 = diagnostic port active (3F8:9600,8,N,1)
.set BITF_MODE,		0x00000002		# 0 = Local IBIT mode, 1 = PBIT
.set BITF_AP,		0x00000004		# BSP = 0, AP = 1

# BSP and AP stack size

.set STACK_SIZE,	0x1000		# stacks are 4KB in size


.text

/*
 * 2nd Stage Signature
 */

	.globl	_s2_signature
_s2_signature:
	.byte	'B'		/* 2nd Stage signature */
	.byte	'I'
	.byte	'T'
	.byte	'2'


	.long	0		/* place-holders for:	image checksum */
	.long	0		/* 			2nd stage image size */


/*
 * Entry point - called from boot_real.S
 * ===========================================================================
 *
 * Things we assume when entering here:
 *
 * 1) We are running in protected mode
 *
 * 2) EDI contains flags as follows:
 *	[0]   - 1 = diagnostic port active (3F8:9600,8,N,1)
 *	[1]   - 0 = Local IBIT mode, 1 = PBIT
 *	[2]   - 0 = BSP, 1 = AP
 *	[3]   - 0 = BIT, 1 = CUTE	:only matters for combined BIT/CUTE image
 *  [5-4] - power-on BIT duration
 *  [7-6] - test failure action
 *  [8]   - 1 = ESI contains address of BIOS_INFO structure
 *  [9-11]- Serial port(s) baud rate
 *
 * 3) EBP contains the address of the AP real-mode entry point
 *
 * 4) Memory location at 0x9000 is used by SMP semaphore, it is shared by
 *    both real mode and protected mode code
 *
 * 5) Original ESI content (address of BIOS_INFO) is retrieved from  Memory
 *    location 0x9004
 *
 * 6) Protected Mode port-80 codes are in range 0xCCB1-0xCCBF
 */

	.globl _prot_entry
_prot_entry:

	/* Sign-on via Port-80 */
	movb	$0xA2, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	/* Ensure selectors are suitably initialized 
	 * We assume [DS] and [ES] are 4GB linear, others are NULL */
	movw	$DS_SEL, %ax				# [DS] = [ES] = 4GB data selector
	movw	%ax, %ds					#
	movw	%ax, %es					#

	xorw	%ax, %ax					# [FS] = [GS] = NULL selector
	movw	%ax, %fs					#
	movw	%ax, %gs					#
	
#	movl	%edi, _dStartupFlags		# save to 'C' visible globals

	/* BSP only: save Bit Flags, the BSP will set the AP bit later */
	testl	$BITF_AP, %edi				# only do if BSP
	jnz	skip_mem_init					#
	movl	%edi, _dStartupFlags		# save to 'C' visible globals

	movl	%ebp, _dApEntry				#
	movl    $0x9004, %eax
	movl    (%eax), %ebx
	movl    %ebx, _dBiosInfo_addr       #

	/* Zero uninitialized data segment */
	movl	$__bss_start, %edi			# BSS start address in [EDI]

	movl	$__bss_stop, %ecx			# BSS size in [ECX]
	subl	%edi,%ecx					#
	shrl	$2,%ecx						# convert size to DWORDs

	xorl	%eax, %eax					# zero BSS segment
	rep									#
	stosl								#

	/* Zero system tables in low memory */

	movl	$0, %edi					# start at offset zero
	movl	$0x20000, %ecx				# 512KB as DWORDs

	xorl	%eax, %eax					# zero system tables
	rep									#
	stosl								#


skip_mem_init:

	/* Skip if AP */
	testl	$BITF_AP, _dStartupFlags	# only do if BSP
	jnz	skip_gdt_idt_init				#

	/* Install the final GDT and IDT */
	/* Copy the GDT template to final location */
	
	lea	gdt, %esi						# GDT template in [ESI]
	movl	$GDT_BASE, %edi				# base address of GDT in [EDI]

	movl	$(gdt_end - gdt), %ecx		# GDT template length in DWORDs
	shrl	$2, %ecx					#

	rep									# copy GDT data
	movsl								#

	/* Create the IDT */
	call	_sysInstallIdt				# creates an IDT for us
	
.ifdef PCBRD
	call	_sysInitPic					# initialize PIC
.else
	call	_sysInitIoApic				# program the IOAPIC
.endif


skip_gdt_idt_init:
	movb	$0xA4, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	/* Load the GDTR */
	lgdt	gdt_48						# install GDT

	/* Load the IDTR */
	lidt	idt_48						# install IDT


	/* Skip if AP */
	testl	$BITF_AP, _dStartupFlags	# only do if BSP
	jnz	skip_ptbl_init					#


	/* Set-up and Enable Paging - PSE-36 Mechanism with 4MB pages */
	/* Prepare the page directory: 1024 pages */
	movl	$PDIR_BASE, %edi			# base address (above GDT)
	movl	$PDIR_ENTRIES, %ecx			# number of entries
	xorl	%eax, %eax					#
	rep									# zero 32-bit Page Directory entry
	stosl								#

	/* Identity map the first 16MB of DRAM (4 entries) */
	movl	$PDIR_BASE, %edi			# base address (above GDT)

	movl	$0x00000183, %eax			# 0x00000000 - 0x003FFFFF
	movl	%eax,0x000(%edi)			# set G, PS, R/W, P bits

	movl	$0x00400183, %eax			# 0x00400000 - 0x007FFFFF
	movl	%eax,0x004(%edi)			#

	movl	$0x00800183, %eax			# 0x00800000 - 0x00BFFFFF
	movl	%eax,0x008(%edi)			#

	movl	$0x00C00183, %eax			# 0x00C00000 - 0x00FFFFFF
	movl	%eax,0x00C(%edi)			#

	/* Identity map the next 32MB of DRAM  as Malloc Heaps (8 entries) */
	movb	$0xA5, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	movl	$0x01000183, %eax			# 0x01000000 - 0x013FFFFF
	movl	%eax,0x010(%edi)			#

	movl	$0x01400183, %eax			# 0x01400000 - 0x017FFFFF
	movl	%eax,0x014(%edi)			#

	movl	$0x01800183, %eax			# 0x01800000 - 0x01BFFFFF
	movl	%eax,0x018(%edi)			#

	movl	$0x01C00183, %eax			# 0x01C00000 - 0x01FFFFFF
	movl	%eax,0x01C(%edi)			#

	movl	$0x02000183, %eax			# 0x02000000 - 0x023FFFFF
	movl	%eax,0x020(%edi)			#

	movl	$0x02400183, %eax			# 0x02400000 - 0x027FFFFF
	movl	%eax,0x024(%edi)			#

	movl	$0x02800183, %eax			# 0x02800000 - 0x02BFFFFF
	movl	%eax,0x028(%edi)			#

	movl	$0x02C00183, %eax			# 0x02C00000 - 0x02FFFFFF
	movl	%eax,0x02C(%edi)			#

	/* Identity map the next 32MB of DRAM (8 entries) */
	movl	$0x03000183, %eax			# 0x03000000 - 0x033FFFFF
	movl	%eax,0x030(%edi)			#

	movl	$0x03400183, %eax			# 0x03400000 - 0x037FFFFF
	movl	%eax,0x034(%edi)			#

	movl	$0x03800183, %eax			# 0x03800000 - 0x03BFFFFF
	movl	%eax,0x038(%edi)			#

	movl	$0x03C00183, %eax			# 0x03C00000 - 0x03FFFFFF
	movl	%eax,0x03C(%edi)			#

	movl	$0x04000183, %eax			# 0x04000000 - 0x043FFFFF	Usage:
	movl	%eax,0x040(%edi)			# 0x04000000 - 0x0400FFFF	FreeRTOS Kernel Heap
										# 0x04100000 - 0x042FFFFF	GEIF Driver Descriptor/Buffer Pool
										# 0x04300000 - 0x043FFFFF	lwIP Heap	
										
	movl	$0x04400183, %eax			# 0x04400000 - 0x047FFFFF	lwIP Memory Pool
	movl	%eax,0x044(%edi)			#

	movl	$0x04800183, %eax			# 0x04800000 - 0x04BFFFFF
	movl	%eax,0x048(%edi)			#

	movl	$0x04C00183, %eax			# 0x04C00000 - 0x04FFFFFF	Debug log
	movl	%eax,0x04C(%edi)			#
	#Newly added by Hari
    movl	$0x05000183, %eax			# 0x05000000 - 0x053FFFFF
	movl	%eax,0x050(%edi)			#

	movl	$0x05400183, %eax			# 0x05400000 - 0x057FFFFF
	movl	%eax,0x054(%edi)			#

	movl	$0x05800183, %eax			# 0x05800000 - 0x05BFFFFF
	movl	%eax,0x058(%edi)			#

	movl	$0x05C00183, %eax			# 0x05C00000 - 0x05FFFFFF
	movl	%eax,0x05C(%edi)			#

	movl	$0x06000183, %eax			# 0x06000000 - 0x063FFFFF
	movl	%eax,0x060(%edi)			#

    movl	$0x06400183, %eax			# 0x06400000 - 0x067FFFFF
	movl	%eax,0x064(%edi)			#

	movl	$0x06800183, %eax			# 0x06800000 - 0x06BFFFFF
	movl	%eax,0x068(%edi)			#

	movl	$0x06C00183, %eax			# 0x06C00000 - 0x06FFFFFF
	movl	%eax,0x06C(%edi)			#

	/* Identity map APIC etc. */

	movb	$0xA6, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	movl	$0xFEC00183, %eax			# 0xFEC00000 - 0xFEFFFFFF
	movl	%eax,0xFEC(%edi)			#

	movl	$0xFF000183, %eax			# 0xFF000000 - 0xFF3FFFFF
	movl	%eax,0xFF0(%edi)			#

	movl	$0xFF400183, %eax			# 0xFF400000 - 0xFF7FFFFF
	movl	%eax,0xFF4(%edi)			#

	movl	$0xFF800183, %eax			# 0xFF800000 - 0xFFBFFFFF
	movl	%eax,0xFF8(%edi)			#

	movl	$0xFFC00183, %eax			# 0xFFC00000 - 0xFFFFFFFF
	movl	%eax,0xFFC(%edi)			#

skip_ptbl_init:

	movb	$0xA7, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	/* Point CR3 (PDBA) at the Page Directory, caching enabled */
	movl	$PDIR_BASE, %eax			# set PDBA, PCD & PWT = 0
	movl	%eax, %cr3					#

	/* Select PSE-36 with 4MB pages and enable paging */
	movl	%cr4, %eax					#
	btsl	$4, %eax					# enable Page Size Extensions CR4.PSE
	btrl	$5, %eax					# disable Page Address Extension CR4.PAE
	movl	%eax, %cr4					#

	movl	%cr0, %eax					#
	btsl	$31, %eax					# enable Paging CR0.PG
	movl	%eax, %cr0					#

	/* Set-up Stack */
	lss		_stack_start, %esp			# load stack ptr

	lea		_stack_start, %ebx			# drop stack start by STACK_SIZE (4KB) for each core
	subl	$STACK_SIZE, (%ebx)			#


	movb	$0xA9, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	/* Count the cores */
	incb	_bCpuCount					# register this core


SMP_INIT:
	/* Enable SMP - detect available cores and put into halting loop */
	testl	$BITF_AP, _dStartupFlags	# only do SMP init if BSP
	jnz	skip_smp_init					#

	movl	$0x9000, %eax				#
	pause
	lock
	btr		$0, (%eax)					# clear semaphore so APs can run

	movl	_dStartupFlags, %edi		# set the AP flag
	orl		$BITF_AP, %edi				#
	movl	%edi, _dStartupFlags 		#

	movb	$0xAA, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	/* Initialize delay mechanism */
	call	_calibrateDelay				# compute delay factor for this CPU

	call 	_debugInit					# init serial debug port
	call    _secondStageSignon

	movb	$0xAB, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	/* Initialise the local APIC */

	movl	$SVR, %esi					# enable local APIC
	movl	(%esi), %eax				#
	orl		$APIC_ENABLED, %eax			#
	movl	%eax, (%esi)				#

	movl	$ICR_LOW, %esi				# INIT IPI
	movl	$0x000C4500, %eax			#
	movl	%eax, (%esi)				#

	movl	$200, %eax					# delay 10 mS
	call	_msDelay					#
	
	/* Check CPU type */
	movl 	$1, %eax
	cpuid
	andl	$0xffff0, %eax
	cmpl	$0x20650, %eax
	je		CPU_nehalem
	cmpl	$0x206A0, %eax
	je		CPU_nehalem


	shrl	$12, %ebp					# get bits[19:12] of the start address in EBP[7:0]
	movl	$ICR_LOW, %esi				# SIPI
	movl	$0x000C4600, %eax			#
	orl	%ebp, %eax						# set vector for AP init code
	movl	%eax, (%esi)				#

	movl	$200, %eax					# delay 200 mS
	call	_msDelay					#

	movl	$ICR_LOW, %esi				# SIPI
	movl	$0x000C4600, %eax			#
	orl	%ebp, %eax						# set vector for AP init code
	movl	%eax, (%esi)				#

	movl	$200, %eax					# delay 200 mS
	call	_msDelay					#
	jmp		cont_bsp_init

CPU_nehalem:
	shrl	$12, %ebp					# get bits[19:12] of the start address in EBP[7:0]
	movl	$ICR_LOW, %esi				# SIPI
	movl	$0x000CC600, %eax			#
	orl		%ebp, %eax					# set vector for AP init code
	movl	%eax, (%esi)				#
	movl	$200, %eax					# delay 200 mS
	call	_msDelay					#

	shrl	$12, %ebp					# get bits[19:12] of the start address in EBP[7:0]
	movl	$ICR_LOW, %esi				# SIPI
	movl	$0x000CC600, %eax			#
	orl		%ebp, %eax					# set vector for AP init code
	movl	%eax, (%esi)				#
	movl	$400, %eax					# delay 200 mS
	call	_msDelay					#


cont_bsp_init:
	call    _bsp_ident					# signon for BSP

	/* Record APIC ID in an array */
	leal	_abApicId, %esi				# load pointer to BSP APIC ID
	movl	$APIC_ID, %eax				# read APIC ID
	movb	(%eax), %al					#
	movb	%al, (%esi)					# save
	
	movb	$0xAe, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#

	/* Initialize APIC LINT0 and LINT1 */
	movl	$APIC_BASE,  %ebx		 	# load APIC address [EBX]
	movl	$0x00000700, APIC_LINT0(%ebx)
	movl	$0x00000400, APIC_LINT1(%ebx)
	
	call _lapicInit              		# initialize Local APIC for BSP

	/* BSP only: Call to our C-code entry point */
	call	_c_entry					# our 'main()'
	hlt									# should never return... but...

skip_smp_init:

	/* AP only: enable APIC in SVR - APs have this disabled */
	movl	$SVR, %esi					# Spurious Vector register
	movl	(%esi), %eax				#
	orl		$0x100, %eax				# enable APIC
	movl	%eax, (%esi)				#


	movl 	$LVT3, %esi
	movl 	(%esi), %eax
	andl	$0xFFFFFF00, %eax 			# Clear out previous vector.
	orl		$0x0000001f, %eax
	movl    %eax, (%esi)

	/* Record APIC IDs in an array */

	movb	_bCpuCount, %al				# get zero-based CPU number
	decb	%al							#
	movzbl	%al, %eax					#

	leal	_abApicId, %esi				# load abApicId[] pointer for this CPU
	addl	%eax, %esi					#
	
	movl	$APIC_ID, %eax				# read APIC ID
	movl	(%eax), %eax				#
	shrl	$24, %eax					# align to bit-0
	movb	%al, (%esi)					# save
	
	/* AP only: Clear Initialization Semaphore and enter halting loop */
	call	_smp_dispatch				# report AP details & count cores


	movl	$0x9000, %eax				#
	pause								#
	lock								#
	btr		$0, (%eax)					# clear semaphore so APs can run


apEndless:

	sti									# halt the AP, interrupts enabled
	hlt									#

	jmp	apEndless						# AP remains in a halting loop, servicing interrupts



/****************************************************************************/

.data
	.align 16

	.globl _dStartupFlags
_dStartupFlags:
	.long	0

	.globl _dApEntry
_dApEntry:
	.long	0

	.globl _dBiosInfo_addr
_dBiosInfo_addr:
	.long	0

	.globl _bCpuCount
_bCpuCount:
	.byte	0

	.globl _abApicId
_abApicId:
	.byte	255			# support 16-cores 
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255
	.byte	255


	.globl _dSmpInitSemaphore
_dSmpInitSemaphore:
	.long	0


/*
 * Stack
 */
	.align 16
	.globl _stack_start
_stack_start:

	.long 0x00080000		# BSP stack grows down from 0x00080000
	.long DS_SEL			#


/*
 * GDT template
 */
	.align 16
gdt:
	.word	0				# NULL descriptor
	.word	0				#
	.word	0				#
	.word	0				#

	.word	0xFFFF			# 4Gb - (0x100000*0x1000 = 4Gb)
	.word	0				# base address = 0
	.word	0x9A00			# code read/exec
	.word	0x00CF			# granularity = 4096, 386
							#  (+5th nibble of limit)

	.word	0xFFFF			# 4Gb - (0x100000*0x1000 = 4Gb)
	.word	0				# base address = 0
	.word	0x9200			# data read/write
	.word	0x00CF			# granularity = 4096, 386
gdt_end:


/*
 * GDTR and IDTR loading placeholders
 */
	.align	4

	.word	0			# alignment bytes
idt_48:
	.word	(IDT_SLOTS * 8) - 1	# IDT limit
	.word	IDT_BASE		# IDT base


	.word	0			# alignment bytes
gdt_48:
	.word	(GDT_SLOTS * 8) - 1	# GDT limit
	.word	GDT_BASE		# GDT base


/****************************************************************************/

.bss

