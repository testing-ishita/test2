
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

/* delay.S - calibrated delay functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/sys_delay.S,v 1.1 2013-09-04 07:41:29 chippisley Exp $
 *
 * $Log: sys_delay.S,v $
 * Revision 1.1  2013-09-04 07:41:29  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2010/06/23 11:00:42  hmuneer
 * CUTE V1.01
 *
 * Revision 1.2  2008/09/26 13:14:11  swilson
 * Add function for reading the TSC.
 *
 * Revision 1.1  2008/07/24 14:56:48  swilson
 * System-level functions moved back here from BLIB, as they shouldn't be used there.
 *
 * Revision 1.1  2008/07/18 12:46:17  swilson
 * Calibrated delay function using CPU TSC.
 *
 * Revision 1.1  2008/07/10 16:51:30  swilson
 * Add calibrated delay fuunctions. Re-code loader to support SMP initialization and
 *  add SMP init.
 *
 */

.include "asm/i386.h"


.text

/*****************************************************************************
 * sysGetCpuFrequency: get CPU frequency
 *
 * RETURNS: CPU frequency in MHz
 *
 * PROTOTYPE:
 *
 *	UINT32 sysGetCpuFrequency (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysGetCpuFrequency
_sysGetCpuFrequency:

	pushl	%ebp
	movl	%esp, %ebp

	movl	_dTicksPer_us, %eax	# ticks-per-us is the same as frequency in MHz

	leave
	ret


/*****************************************************************************
 * sysDelayMilliseconds: delay for a number of milliseconds
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysDelayMilliseconds (UINT32 dMsDelay)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dMsDelay delay period measured in mS
 */
	.globl _sysDelayMilliseconds
_sysDelayMilliseconds:

	pushl	%ebp
	movl	%esp, %ebp

	pushal

	movl	0x08(%ebp), %eax	# delay interval
	call	_msDelay			# do delay

	popal

	leave
	ret


/*****************************************************************************
 * sysDelayMicroseconds: delay for a number of microseconds
 *
 * RETURNS: None
 *
 * PROTOTYPE:
 *
 *	void sysDelayMicroseconds (UINT32 dUsDelay)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dUsDelay delay period measured in uS
 */
	.globl _sysDelayMicroseconds
_sysDelayMicroseconds:

	pushl	%ebp
	movl	%esp, %ebp

	pushal

	movl	0x08(%ebp), %eax	# delay interval
	call	_usDelay			# do delay

	popal

	leave
	ret


/*****************************************************************************
 * sysGetTimestamp: read the timestamp counter
 *
 * RETURNS: timestamp counter value
 *
 * PROTOTYPE:
 *
 *	UINT64 sysGetTimestamp (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysGetTimestamp
_sysGetTimestamp:

	pushl	%ebp
	movl	%esp, %ebp

	rdtsc				# get current TSC [EDX:EAX]

	leave
	ret


/****************************************************************************
 * Low level functions - called directly from Loader module
 ****************************************************************************/


/*****************************************************************************
 * calibrateDelay: CPU-specific delay setup
 *
 * Input:
 *	None
 *
 * Output:
 *	_dTicksPer_us loaded
 *
 * Compute the TSC operating frequency and thereby, the count for 1uS
 */
	.globl _calibrateDelay
_calibrateDelay:
	
	pushal

	call	_boardCalibrateDelay	# calibrate processor specific delay

	movl	%eax, _dTicksPer_us	# save MHz as number of clock ticks per uS

	popal
	ret



/*****************************************************************************
 * msDelay: delay for 'n' mS
 *
 * Input:
 *	EAX = number of milliseconds
 *
 * Output:
 *	None
 *
 * Called by sysDelayMilliseconds ()
 */
	.globl _msDelay
_msDelay:
	pushl	%esi
	pushl	%edx

	movl	%eax, %esi		# put delay (mS) in ESI
	movl	$1000, %eax		#
	mull	%esi			# convert mS to uS [EDX:EAX] only [EAX] used

	popl	%edx
	popl	%esi

	jmp	_usDelay


/*****************************************************************************
 * usDelay: delay for 'n' uS
 *
 * Input:
 *	EAX = number of microseconds
 *
 * Output:
 *	None
 *
 * Called by sysDelayMicroseconds ()
 */
	.globl _usDelay
_usDelay:
	pushal

	movl	%eax, %esi		# put delay (uS) in ESI

	rdtsc				# get current TSC [EDX:EAX], put in ECX:EBX
	movl	%eax, %ebx		#
	movl	%edx, %ecx		#

	movl	%edx, %edi		# save MSB for compare

	movl	_dTicksPer_us, %eax	# get delay*ticks_per_us in [EDX:EAX]
	mull	%esi			#

	addl	%eax, %ebx		# compute termination count [EDI:ESI]
	adcl	%edx, %ecx		#


	cmpl	%edi, %ecx		# check if MSB will wrap before expiring
	jb	usDelay_wrap		#

usDelay_loop:
	rdtsc				# get current TSC [EDX:EAX]

	subl	%ebx, %eax		# compute difference
	sbbl	%ecx, %edx		#
	jb	usDelay_loop		# if not beyond end-count, loop

	jmp	usDelay_exit		#

usDelay_wrap:
	rdtsc				# get current TSC [EDX:EAX]

	cmpl	%edx, %edi		# wait for current MSB < start MSB
	jb	usDelay_wrap		#

	subl	%eax, %ebx		# compute difference
	sbbl	%edx, %ecx		#
	ja	usDelay_wrap		# if not beyond end count, loop

usDelay_exit:
	popal
	ret


/****************************************************************************/

.data

_dTicksPer_us:
	.long	0			#


/****************************************************************************/

.bss

