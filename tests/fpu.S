
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


/* xmmx.S - mmx & xmm read/write & test functions
 */

/* Version control:
 *
 * Revision 1.0  2009/04/02 hmuneer
 * CReated.
 *
 */



.text


	.globl _fpuExpTest 
_fpuExpTest:

	pushl	%ebp
	movl	%esp, %ebp
	pushal

	WBINVD

	movl    0x08(%ebp), %eax	# operation type
	movl	0x0C(%ebp), %edi	# pointer to result	
	
	movl	0x10(%ebp), %esi	# pointer to a
	fld	0x00(%esi)		# load mmo
	movl	0x14(%ebp), %esi	# pointer to b
	fld	0x00(%esi)		# load mm1

	fdivp				# 
	fwait				# allow exception to occur

	popal
	leave
	ret 



	.globl _fpuTest 
_fpuTest:

	pushl	%ebp
	movl	%esp, %ebp
	pushal


	movl    0x08(%ebp), %eax	# operation type
	movl	0x0C(%ebp), %edi	# pointer to result	

	cmp     $0x00, %eax
	jz		_fpuAdd
	cmp     $0x01, %eax
	jz		_fpuSub
	cmp     $0x02, %eax
	jz		_fpuMul
	cmp     $0x03, %eax
	jz		_fpuDiv


_fpuAdd:
	movl	0x10(%ebp), %esi	# pointer to a
	fld		0x00(%esi)			# load mmo
	movl	0x14(%ebp), %esi	# pointer to b
	fld		0x00(%esi)			# load mm1
	faddp
	fstp	0x00(%edi)
	jmp		_fpuResult	


_fpuSub:	
	movl	0x10(%ebp), %esi	# pointer to a
	fld		0x00(%esi)			# load mmo
	movl	0x14(%ebp), %esi	# pointer to b
	fld		0x00(%esi)			# load mm1
	fsubp
	fstp	0x00(%edi)
	jmp		_fpuResult


_fpuMul:
	movl	0x10(%ebp), %esi	# pointer to a
	fld		0x00(%esi)			# load mmo
	movl	0x14(%ebp), %esi	# pointer to b
	fld		0x00(%esi)			# load mm1
	fmulp
	fstp	0x00(%edi)
	jmp		_fpuResult


_fpuDiv:
	movl	0x10(%ebp), %esi	# pointer to a
	fld		0x00(%esi)			# load mmo
	movl	0x14(%ebp), %esi	# pointer to b
	fld		0x00(%esi)			# load mm1
	fdivp
	fstp	0x00(%edi)
	jmp		_fpuResult


_fpuResult:
	popal
	leave
	ret 




	.globl _fpuSetup
_fpuSetup:

	pushl	%ebp
	movl	%esp, %ebp
	pushal

	movl    0x08(%ebp), %eax	# operation type
	movl	0x0C(%ebp), %edi	# pointer to result	
	cmp     $0x00, %eax
	jz		_readCW
	cmp     $0x01, %eax
	jz		_writeCW
	cmp     $0x02, %eax
	jz		_writeINIT

_readCW:	
	fnstcw	0x00(%edi)
	jmp     fpuSetupEnd

_writeCW:
	fldcw	0x00(%edi)
	jmp     fpuSetupEnd

_writeINIT:
	FNINIT
	jmp     fpuSetupEnd

fpuSetupEnd:
	popal
	leave
	ret 




	.globl _fpuGetFlags
_fpuGetFlags:

	pushl	%ebp
	movl	%esp, %ebp
	pushal
	
	movl    0x08(%ebp), %edi	# pointer to result	
	
	FNSTSW  %ax				# store FPU status word in AX
	mov  	%ax, 0x00(%edi)	# Store AX in the variable passed
	FNCLEX					# Clear all FPU exceptions
		
	popal
	leave
	ret 



	.globl _fpuClearFlags
_fpuClearFlags:

	pushal

	FNCLEX					# Clear all FPU exceptions
		
	popal
	leave
	ret 



	.globl _cpuSetNe
_cpuSetNe:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %eax	# get operation type

	cmpb	$0x00, %al		# 'set' or 'clear'?
	jz	clrNE			#


setNE:	movl	%cr0, %eax		#
	movl	%eax, %ebx		# save current NE state in [EBX]
	orb	$0x20, %al		# set NE bit
	movl	%eax, %cr0		#

	jmp     cpuSetNe_Done

clrNE:	movl	%cr0, %eax		#
	movl	%eax, %ebx		# save current NE state in [EBX]
	andb	$~0x20, %al		# clear NE bit
	movl	%eax, %cr0		#

cpuSetNe_Done:
	andl	$0x00000010, %ebx	# isolate the NE bit
	shrl	$5, %ebx		#
	movl	%ebx, %eax		# put in [EAX] for return

	popl	%ebx

	leave
	ret 


.data

.bss

