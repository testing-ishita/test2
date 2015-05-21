
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

.include "asm/i386.h"


# MSR Registers

.set FSB_CLOCK_STS,	0x00CD		# FSB_FREQ in bits[2:0]
.set IA32_PERF_STS,	0x0198		#


.text





/*****************************************************************************
 * boardCalibrateDelay: CPU-specific delay setup
 *
 * Input:
 *	None
 *
 * Output:
 *	_dTicksPer_us loaded
 *
 * Compute the TSC operating frequency and thereby, the count for 1uS
 */
	.globl _boardCalibrateDelay
_boardCalibrateDelay:

	/*Disbale Turbo*/
	movl	$0x000001a0, %ecx			# EFI_MSR_IA32_MISC_ENABLES
	rdmsr								#
	orl		$0x00000040, %edx			#
	wrmsr

	/* Determine ratios */
	movl	$0x000000CE, %ecx	# EFI_PLATFORM_INFORMATION
	rdmsr						#
	shrl	$8, 		 %eax	#
	andl	$0x000000ff, %eax	#
	shrl	$8, 		 %edx	#
	andl	$0x000000ff, %edx	#

	movl  $100, %ebx
	mull   %ebx

	ret





/****************************************************************************/

.data

/****************************************************************************/

.bss

