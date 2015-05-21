
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

/* cpu.S - CPU primitives
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/cpu.S,v 1.1 2013-09-04 07:41:28 chippisley Exp $
 *
 * $Log: cpu.S,v $
 * Revision 1.1  2013-09-04 07:41:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2010/06/23 11:00:42  hmuneer
 * CUTE V1.01
 *
 * Revision 1.4  2009/09/29 12:25:00  swilson
 * Add command for throwing a GPF (for testing the exception handling mechanism).
 *
 * Revision 1.3  2008/09/17 13:49:12  swilson
 * Add CPU halt function.
 *
 * Revision 1.2  2008/05/21 13:27:54  swilson
 * Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
 *
 * Revision 1.1  2008/05/12 15:04:40  swilson
 * Add IBIT console. Debugging various library functions.
 *
 */


.text

/*****************************************************************************
 * sysCpuid: execute a CPUID function
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysCpuid (UINT32 dFunc, CPUID_REGS* psRegs)
 *
 *	struct {
 *		UINT32	eax;	
 *		UINT32	ebx;
 *		UINT32	edx;
 *		UINT32	ecx;
 *
 *	} CPUID_REGS;
 *
 *	Note: the function number is not checked for validity.
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dFunc	CPUID funcion
 *	[EBP+0C] psRegs	pointer to register structure
 */
	.globl _sysCpuid
_sysCpuid:

	pushl	%ebp
	movl	%esp, %ebp

	pushal

	movl	0x08(%ebp), %eax	# function number
	movl	0x0C(%ebp), %edi	# pointer to structure

	.byte	0x0F, 0xA2		# CPUID op-code

	movl	%eax, 0x00(%edi)	# copy result registers to structure
	movl	%ebx, 0x04(%edi)	#
	movl	%edx, 0x08(%edi)	#
	movl	%ecx, 0x0C(%edi)	#

	popal

	leave
	ret


/*****************************************************************************
 * _sysInvalidateTLB: Invalidate the whole TLB
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void _sysInvalidateTLB (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysInvalidateTLB
_sysInvalidateTLB:

	pushl	%ebp
	movl	%esp, %ebp

	movl	%cr3, %eax		# Invalidate the whole TLB
	movl	%eax, %cr3		#

	leave
	ret





/*****************************************************************************
 * sysCacheEnable: enable the CPU cache
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysCacheEnable (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysCacheEnable
_sysCacheEnable:

	pushl	%ebp
	movl	%esp, %ebp

	movl	%cr0, %eax		# enable cache
	andl	$~0x60000000, %eax	#
	movl	%eax, %cr0		#

	leave
	ret


/*****************************************************************************
 * sysCacheDisable: disable the CPU cache
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysCacheDisable (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysCacheDisable
_sysCacheDisable:

	pushl	%ebp
	movl	%esp, %ebp

	.byte	0x0F, 0x09		# WBINVD op-code

	movl	%cr0, %eax		# disable cache
	orl	$0x60000000, %eax	#
	movl	%eax, %cr0		#

	leave
	ret


/*****************************************************************************
 * sysHalt: halt the CPU
 *
 * RETURNS: DOES NOT RETURN
 *
 * PROTOTYPE:
 *
 *	void sysHalt (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysHalt
_sysHalt:

	cli				# interrupts off
	hlt				# halt
	jmp	_sysHalt			# don't return if woken from HLT


/*****************************************************************************
 * _sysThrowGpf: throw a general protection fault to test IBIT fault handling
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void _sysThrowGpf (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl __sysThrowGpf
__sysThrowGpf:

	pushl	%ebp
	movl	%esp, %ebp

	xorl	%eax, %eax				# load DS with NULL selector
	movw	%ax, %ds				#

	movl	%ds:(%eax), %eax		# reference memory with NULL selector

	leave
	ret


.data

.bss

