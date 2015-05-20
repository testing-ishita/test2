
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

/* mem_io.s - direct memory Read and Write primitives
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/mem_io.S,v 1.1 2013-09-04 07:41:28 chippisley Exp $
 *
 * $Log: mem_io.S,v $
 * Revision 1.1  2013-09-04 07:41:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/05/21 13:27:54  swilson
 * Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
 *
 * Revision 1.1  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/05/12 14:52:32  swilson
 * Local implementation of direct memory access.
 *
 */


.text

/*****************************************************************************
 * sysWriteMem8: write 8-bit value to physical address
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysWriteMem8 (UINT32 wAddr, UINT8 bData)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dAddr	32-bit physical address
 *	[EBP+0C] bData	8-bit data
 */
	.globl _sysWriteMem8
_sysWriteMem8:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx
	movb	0x0C(%ebp), %al
	movb	%al, (%ebx)

	popl	%ebx

	leave
	ret


/*****************************************************************************
 * sysWriteMem16: write 16-bit value to physical address
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysWriteMem16 (UINT32 wAddr, UINT16 wData)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dAddr	32-bit physical address
 *	[EBP+0C] wData	16-bit data
 */
	.globl _sysWriteMem16
_sysWriteMem16:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx
	movw	0x0C(%ebp), %ax
	movw	%ax, (%ebx)

	popl	%ebx

	leave
	ret


/*****************************************************************************
 * sysWriteMem32: write 32-bit value to physical address
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysWriteMem32 (UINT32 wAddr, UINT32 dData)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dAddr	32-bit physical address
 *	[EBP+0C] dData	32-bit data
 */
	.globl _sysWriteMem32
_sysWriteMem32:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx
	movl	0x0C(%ebp), %eax
	movl	%eax, (%ebx)

	popl	%ebx

	leave
	ret


/*****************************************************************************
 * sysReadMem8: read 8-bit value from physical address
 *
 * RETURNS: 8-bit memory contents
 *
 * PROTOTYPE:
 *
 *	UINT8 sysReadMem8 (UINT32 wAddr)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dAddr	32-bit physical address
 */
	.globl _sysReadMem8
_sysReadMem8:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx
	movb	(%ebx), %al

	popl	%ebx

	leave
	ret


/*****************************************************************************
 * sysReadMem16: read 16-bit value from physical address
 *
 * RETURNS: 16-bit memory contents
 *
 * PROTOTYPE:
 *
 *	UINT16 sysReadMem16 (UINT32 wAddr)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dAddr	32-bit physical address
 */
	.globl _sysReadMem16
_sysReadMem16:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx
	movw	(%ebx), %ax

	popl	%ebx

	leave
	ret


/*****************************************************************************
 * sysReadMem32: read 32-bit value from physical address
 *
 * RETURNS: 32-bit memory contents
 *
 * PROTOTYPE:
 *
 *	UINT32 sysReadMem32 (UINT32 wAddr)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dAddr	32-bit physical address
 */
	.globl _sysReadMem32
_sysReadMem32:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx
	movl	(%ebx), %eax

	popl	%ebx

	leave
	ret


.data

.bss

