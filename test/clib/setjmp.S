
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

/* setjmp.S - execution environment save/restore
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/setjmp.S,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: setjmp.S,v $
 * Revision 1.1  2013-09-04 07:17:58  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/09/16 08:11:21  swilson
 * Revise saved registers and re-write to make more efficient.
 *
 * Revision 1.1  2008/05/15 15:32:40  swilson
 * Add build and version tracking. Implement & test missing c-library functions.
 *
 */

.include "asm/i386.h"


.text

/* Context save stucture has the following format:
 *
 *	typedef struct tagJmpBuf
 *	{
 *		UINT32	fs;	# 0x00
 *		UINT32	gs;	# 0x04
 *
 *		UINT32	ebx;	# 0x08
 *		UINT32	ecx;	# 0x0C
 *		UINT32	edx;	# 0x10
 *
 *		UINT32	ebp;	# 0x14
 *		UINT32	esi;	# 0x18
 *		UINT32	edi;	# 0x1C
 *
 *		UINT32	esp;	# 0x20
 *		UINT32	eip;	# 0x24
 *
 *		UINT32	eflags;	# 0x28
 *	
 *	} jmp_buf[1];
 *
 */

.set _SJ_FS,			0x00
.set _SJ_GS,			0x04

.set _SJ_EBX,			0x08
.set _SJ_ECX,			0x0C
.set _SJ_EDX,			0x10

.set _SJ_EBP,			0x14
.set _SJ_ESI,			0x18
.set _SJ_EDI,			0x1C

.set _SJ_ESP,			0x20
.set _SJ_EIP,			0x24

.set _SJ_EFLAGS,		0x28



/*****************************************************************************
 * setjmp: save key registers in structure via supplied pointer
 *
 * RETURNS: 0 (to indicate function was called directly)
 *
 * PROTOTYPE:
 *
 *	int setjmp (jmp_buf psJmpBuf)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] psJmpBuf : pointer to context save buffer
 */
	.globl _setjmp
_setjmp:

	pushl	%ebp
	movl	%esp, %ebp

	movl	%ebx, %eax			# save [EBX]
	movl	0x08(%ebp), %ebx		# load structure pointer


	# Save the context

	movl	%eax, _SJ_EBX(%ebx)		# save [EBX]
	movl	%ecx, _SJ_ECX(%ebx)		# save [ECX]
	movl	%edx, _SJ_EDX(%ebx)		# save [EDX]

	pushf					# put entry flags in [EAX]
	popl	%eax				#
	movl	%eax, _SJ_EFLAGS(%ebx)		# save

	xorl	%eax, %eax			# clear upper [EAX]

	movw	%fs, %ax			# get [FS]
	movl	%eax, _SJ_FS(%ebx)		# save

	movw	%gs, %ax			# get [GS]
	movl	%eax, _SJ_GS(%ebx)		# save

	movl	%esi, %eax			# get [ESI]
	movl	%eax, _SJ_ESI(%ebx)		# save

	movl	%edi, %eax			# get [EDI]
	movl	%eax, _SJ_EDI(%ebx)		# save

	movl	0x00(%ebp), %eax		# load value of [EBP] on stack
	movl	%eax, _SJ_EBP(%ebx)		# save

	movl	0x04(%ebp), %eax		# load value of [EIP] on stack
	movl	%eax, _SJ_EIP(%ebx)		# save

	movl	%ebp, %eax			# get [EBP=ESP]
	movl	%eax, _SJ_ESP(%ebx)		# save


	# Return zero to indicate direct calling

	xorl	%eax, %eax			# return 0

	leave
	ret


/*****************************************************************************
 * longjmp: compare 'n' bytes of memory at 's1' to memory at 's2'
 *
 * RETURNS: jumps to address saved via setjmp(), with return value
 *
 * PROTOTYPE:
 *
 *	void longjmp (jmp_buf psJmpBuf, int retValue)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] psJmpBuf : pointer to context save buffer
 * 	[EBP+0C] retValue : value for setjmp() to return
 */
	.globl _longjmp
_longjmp:

	pushl	%ebp
	movl	%esp, %ebp


	# Load GCC expected values for DS and ES

	movw	$DS_SEL, %ax			# set [DS/ES]
	movw	%ax, %ds			#
	movw	%ax, %es			#

	# Load passed params

	movl	0x08(%ebp), %ebx		# load structure pointer
	movl	0x0C(%ebp), %eax		# load return value

	# Validate return value

	orl	%eax, %eax			# check for illegal retValue = 0
	jnz	longjmp_restore			#

	incl	%eax				# substitute 1 for 0


	# Restore the context

longjmp_restore:

	# Reconstruct stack-frame from original setjmp()

	movl	_SJ_ESP(%ebx), %ecx		# [EBP=ESP]
	movl	%ecx, %esp			#
	movl	%ecx, %ebp			#

	movl	_SJ_EBP(%ebx), %ecx		# [EBP] value on stack
	movl	%ecx, 0x00(%ebp)		#

	movl	_SJ_EIP(%ebx), %ecx		# [EIP] return address on stack
	movl	%ecx, 0x04(%ebp)		#

	# Load segment and index registers

	movl	_SJ_FS(%ebx), %ecx 		# [FS]
	movw	%cx, %fs			#

	movl	_SJ_GS(%ebx), %ecx		# [GS]
	movw	%cx, %gs			#

	movl	_SJ_ESI(%ebx), %ecx		# [ESI]
	movl	%ecx, %esi			#

	movl	_SJ_EDI(%ebx), %ecx		# [EDI]
	movl	%ecx, %edi			#

	# Set flags to exit value

	movl	_SJ_EFLAGS(%ebx), %ecx		# put entry flags on stack
	pushl	%ecx				#
	popf					# load 

	# Finally, load [E?X] registers

	movl	_SJ_EDX(%ebx), %edx		# [EDX]
	movl	_SJ_ECX(%ebx), %ecx		# [ECX]
	movl	_SJ_EBX(%ebx), %ebx		# [EBX]


	# Return to address after original setjmp ()

	leave
	ret


.data

.bss

