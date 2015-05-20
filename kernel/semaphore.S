
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

/* semaphore.S - atomic on-off semaphore operations
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/semaphore.S,v 1.1 2013-09-04 07:41:29 chippisley Exp $
 *
 * $Log: semaphore.S,v $
 * Revision 1.1  2013-09-04 07:41:29  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2008/07/24 14:56:48  swilson
 * System-level functions moved back here from BLIB, as they shouldn't be used there.
 *
 * Revision 1.1  2008/07/18 12:46:37  swilson
 * Multiprocessor support through lockable semaphores.
 *
 */


.text

/*****************************************************************************
 * sysSemaphoreLock: wait and acquire a semaphore
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysSemaphoreLock (SEMAPHORE* s)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dSema	semaphore storage
 */
	.globl _sysSemaphoreLock
_sysSemaphoreLock:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx	# get semaphore address

sema_wait:
	pause				#
	lock				#
	btsl	$0, %ds:(%ebx)		# wait until we set the semaphore

	nop				# NOPs for ICE break-in
	nop				#
	nop				#

	jc	sema_wait		# wait for _us_ to set the semaphore

	popl	%ebx

	leave
	ret


/*****************************************************************************
 * sysSemaphoreRelease: release a currently held semaphore
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysSemaphoreRelease (SEMAPHORE* s)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dSema	semaphore storage
 */
	.globl _sysSemaphoreRelease
_sysSemaphoreRelease:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx	# get semaphore address
	btrl	$0, %ds:(%ebx)		# release the semaphore

	popl	%ebx

	leave
	ret


/*****************************************************************************
 * sysSemaphoreTest: attempt to acquire a semaphore, don't wait if not free
 *
 * RETURNS: E__OK if aquired, E__FAIL if not
 *
 * PROTOTYPE:
 *
 *	int sysSemaphoreTest (SEMAPHORE* s)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dSema	semaphore storage
 */
	.globl _sysSemaphoreTest
_sysSemaphoreTest:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx

	movl	0x08(%ebp), %ebx	# get semaphore address
	xorl	%eax, %eax		# assume E__OK returned

	pause				#
	lock				#
	btsl	$0, %ds:(%ebx)		# try to set the semaphore
	jnc	sema_aquired		# if _we_ set semaphore, exit with E__OK

	decl	%eax			# still locked, return E_FAIL (-1)

sema_aquired:
	popl	%ebx

	leave
	ret




.data

.bss

