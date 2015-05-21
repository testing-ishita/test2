
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

/* msr.S - msr read/write functions
 */

/* Version control:
 *
 * Revision 1.0  2009/04/02 hmuneer
 * CReated.
 *
 */


.text


	.globl _vReadMsr 
_vReadMsr:

	pushl	%ebp
	movl	%esp, %ebp

	pushal

	movl	0x08(%ebp), %ecx	# dIndex number
	movl	0x0C(%ebp), %edi	# pointer to dMsb
	movl	0x10(%ebp), %esi	# pointer to dLsb

	.byte	0x0F, 0x32			# 

	movl	%edx, 0x00(%edi)	# copy dMsb
	movl	%eax, 0x00(%esi)	# copy dLsb	

	popal

	leave
	ret 



	.globl _vWriteMsr 
_vWriteMsr:

	pushl	%ebp
	movl	%esp, %ebp

	pushal

	movl	0x08(%ebp), %ecx	# ECX is MSR register number
	movl	0x0C(%ebp), %edx	# EDX contains MSBs
	movl	0x10(%ebp), %eax	# EAX contains LSBs
	.byte	0x0F, 0x30			# 	

	popal

	leave
	ret 



.data

.bss

