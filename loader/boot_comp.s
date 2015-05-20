
/************************************************************************
 *                                                                      *
 *      Copyright 2013 Concurrent Technologies, all rights reserved.    *
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

/* BOOT_COMP.S - Compressed 2nd-stage Entry Point
 *
 * Uncompresses the 2nd-stage to the LOAD_ADDRESS and then jumps to the
 * entry point (prot_entry) at the START_ADDRESS. Then the prot_entry code,
 * in boot_prot.s, continues with the boot sequence.   
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/loader/boot_comp.s,v 1.1 2013-09-04 07:42:08 chippisley Exp $
 *
 * 
 *
 */

.include "asm/i386.h"

# BIT Flags (passed in EDI)
.set BITF_DIAG,		0x00000001	# 1 = diagnostic port active (3F8:9600,8,N,1)
.set BITF_MODE,		0x00000002	# 0 = Local IBIT mode, 1 = PBIT
.set BITF_AP,		0x00000004	# BSP = 0, AP = 1

# 2nd-stage addresses
.ifdef PCBRD
.set LOAD_ADDRESS,	0x200000	# Uncompress image to here
.set START_ADDRESS,	0x20000c	# Start address
.else 
.set LOAD_ADDRESS,	0x100000	# Uncompress image to here
.set START_ADDRESS,	0x10000c	# Start address
.endif

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
	.long	0		/* 2nd stage image size */


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
 * 4) Protected Mode port-80 codes are in range 0xCCB1-0xCCBF
 */

	.globl _comp_entry
_comp_entry:

	/* Sign-on via Port-80 */
	movb	$0xA2, %al					# PORT-80 (for ICE breakpoint)
	outb	%al, $0x80					#
	
	/* Skip if AP */
	testl	$BITF_AP, %edi				# only do if BSP
	jnz	skip_decomp						#
	
	movl	%edi, _dStartupFlags		# save registers we need to preseve
	movl	%ebp, _dApEntry				#
	
	/* Zero uninitialized data segment */
	movl	$__bss_start, %edi			# BSS start address in [EDI]

	movl	$__bss_stop, %ecx			# BSS size in [ECX]
	subl	%edi,%ecx					#
	shrl	$2,%ecx						# convert size to DWORDs

	xorl	%eax, %eax					# zero BSS segment
	rep									#
	stosl	
	
	/* Set-up a temporary Stack */
	lss		_stack_start, %esp			# load stack ptr
	movl	%esp, %ebp
	
	/* Uncompress the 2nd stage */
	pushl	$LOAD_ADDRESS				# push the load address
	call 	_LZ_UncompressImage
	add		%esp,4
	
	/* Restore registers we preserved */
	movl	_dStartupFlags, %edi
	movl	_dApEntry, %ebp

skip_decomp:
	
	/* Jump to relocated (uncompressed) 2nd stage entry point */
	ljmp	$0x08,$START_ADDRESS
	hlt									# should never return... but...
	

/****************************************************************************/

.data
	.align 16

	.globl _dStartupFlags
_dStartupFlags:
	.long	0

	.globl _dApEntry
_dApEntry:
	.long	0

/*
 * Stack
 */
	.align 16
	.globl _stack_start
_stack_start:

	.long 0x00080000		# BSP stack grows down from 0x00080000
	.long DS_SEL			#


/****************************************************************************/

.bss

