
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

/* port_io.S - I/O Port Read and Write primitives
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/port_io.S,v 1.1 2013-09-04 07:41:29 chippisley Exp $
 *
 * $Log: port_io.S,v $
 * Revision 1.1  2013-09-04 07:41:29  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.2  2008/05/12 14:51:24  swilson
 * *** empty log message ***
 *
 * Revision 1.1  2008/04/24 16:05:50  swilson
 * Further updates. Added HAL and BIT library - only I/O functions present at this point.
 *
 * Revision 1.2  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 * Revision 1.1  2008/04/21 17:03:09  swilson
 * *** empty log message ***
 *
 */


.text

/*****************************************************************************
 * sysOutPort8: write 8-bit value to I/O port
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysOutPort8 (UINT16 wPort, UINT8 bData)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] wPort	16-bit port number
 *	[EBP+0C] bData	8-bit data
 */
	.globl _sysOutPort8
_sysOutPort8:

	pushl	%ebp
	movl	%esp, %ebp

	movw	0x08(%ebp), %dx
	movb	0x0C(%ebp), %al
	out	%al, %dx

	leave
	ret


/*****************************************************************************
 * sysOutPort16: write 16-bit value to I/O port
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysOutPort16 (UINT16 wPort, UINT16 wData)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] wPort	16-bit port number
 *	[EBP+0C] wData	16-bit data
 */
	.globl _sysOutPort16
_sysOutPort16:

	pushl	%ebp
	movl	%esp, %ebp

	movw	0x08(%ebp), %dx
	movw	0x0C(%ebp), %ax
	out	%ax, %dx

	leave
	ret


/*****************************************************************************
 * sysOutPort32: write 32-bit value to I/O port
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysOutPort32 (UINT16 wPort, UINT32 dData)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] wPort	16-bit port number
 *	[EBP+0C] dData	32-bit data
 */
	.globl _sysOutPort32
_sysOutPort32:

	pushl	%ebp
	movl	%esp, %ebp

	movw	0x08(%ebp), %dx
	movl	0x0C(%ebp), %eax
	out	%eax, %dx

	leave
	ret


/*****************************************************************************
 * sysInPort8: read 8-bit value from I/O port
 *
 * RETURNS: 8-bit port contents
 *
 * PROTOTYPE:
 *
 *	UINT8 sysInPort8 (UINT16 wPort)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] wPort	16-bit port number
 */
	.globl _sysInPort8
_sysInPort8:

	pushl	%ebp
	movl	%esp, %ebp

	movw	0x08(%ebp), %dx
	in	%dx, %al

	leave
	ret


/*****************************************************************************
 * sysInPort16: read 16-bit value from I/O port
 *
 * RETURNS: 16-bit port contents
 *
 * PROTOTYPE:
 *
 *	UINT16 sysInPort16 (UINT16 wPort)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] wPort	16-bit port number
 */
	.globl _sysInPort16
_sysInPort16:

	pushl	%ebp
	movl	%esp, %ebp

	movw	0x08(%ebp), %dx
	in	%dx, %ax

	leave
	ret


/*****************************************************************************
 * sysInPort32: read 32-bit value from I/O port
 *
 * RETURNS: 32-bit port contents
 *
 * PROTOTYPE:
 *
 *	UINT32 sysInPort32 (UINT16 wPort)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] wPort	16-bit port number
 */
	.globl _sysInPort32
_sysInPort32:

	pushl	%ebp
	movl	%esp, %ebp

	movw	0x08(%ebp), %dx
	in	%dx, %eax

	leave
	ret


.data

.bss

