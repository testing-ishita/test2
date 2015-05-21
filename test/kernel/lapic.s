/*******************************************************************************
*
* Filename: 	lapic.s
* 
* Description:	Local APIC functions.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 07:41:28 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/kernel/lapic.s,v $
*
* Copyright 2012 Concurrent Technologies, Plc.
* 
*
*******************************************************************************/

.include "asm/i386.h"


.set LAPIC_APICID,				0xFEE00020
.set LAPIC_VERSION,				0xFEE00030
.set LAPIC_TASK_PRIORITY,		0xFEE00080
.set LAPIC_EOI,					0xFEE000B0
.set LAPIC_LOGICAL_DST,			0xFEE000D0
.set LAPIC_DEST_FORMAT,			0xFEE000E0
.set LAPIC_SPURIOUS,			0xFEE000F0
.set LAPIC_INTR_CMD_LOW,		0xFEE00300
.set LAPIC_INTR_CMD_HIGH,		0xFEE00310
.set LAPICLVT_TIMER,			0xFEE00320
.set LAPICLVT_PERF_CNTR,		0xFEE00340
.set LAPICLVT_LINT0,			0xFEE00350
.set LAPICLVT_LINT1,			0xFEE00360
.set LAPICLVT_ERROR,			0xFEE00370
.set LAPIC_TIMER_INIT_CNT,		0xFEE00380
.set LAPIC_TIMER_CURRENT_CNT,	0xFEE00390
.set LAPIC_TIMER_DIVIDER,		0xFEE003E0

.set LTIMER_DIVIDE_BY_ONE,		0x0000000B
.set LTIMER_DIVIDE_BY_16,		0x00000003
.set LTIMER_DIVIDE_BY_128,		0x0000000A
.set LTIMER_PERIODIC,			0x00020000
.set APIC_DISABLE,				0x10000
.set APIC_SW_ENABLE,			0x100
.set APIC_CPUFOCUS,				0x200
.set APIC_NMI,					(4<<8)
.text


/*******************************************************************************
*
* apicInit
*
* Performs early initialization of the Local APIC.
* Further configuration, timer tick rate etc, is done later in 
* xPortStartScheduler().
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
	.globl _lapicInit
_lapicInit:

	pushl	%ebp
	movl	%esp, %ebp
	
    pushl	%eax
    pushl	%ebx
    pushl	%ecx
    
    #movb	$0xF8, %al					# PORT-80 (for ICE breakpoint)
	#outb	%al, $0x80					#

	movl   	$0x00010000,%eax			# mask (disable) timer interrupt
	movl	$LAPICLVT_TIMER, %ebx
	movl	%eax,(%ebx)
	
	movl	$0x00000000,%eax			# clear counter
	movl	$LAPIC_TIMER_INIT_CNT, %ebx
	movl	%eax,(%ebx)
	
	movl	$0xFFFFFFFF,%eax			# select flat model
	movl	$LAPIC_DEST_FORMAT, %ebx
	movl	%eax,(%ebx)
	
	#movl	$APIC_NMI,%eax				# generate NMI on performance counter overflow!
	#movl	$LAPICLVT_PERF_CNTR, %ebx	
	#movl	%eax,(%ebx)
	
	movl	$0x00000000,%eax			# set task priority
	movl	$LAPIC_TASK_PRIORITY, %ebx
	movl	%eax,(%ebx)
	
	movl	$0x1b,%ecx					# enable the APIC (global enable)
	rdmsr
	orl		$0x00000800,%eax
	wrmsr
	
	movl   $0x00000100,%eax				# map spurious interrupt to dummy isr #15B
	movl	$LAPIC_SPURIOUS, %ebx
	movl	%eax,(%ebx)
	
	popl 	%ecx
	popl 	%ebx
	popl 	%eax
	
	leave
	ret


/*******************************************************************************
*
* lapicGetId
*
* Reads and returns the Local APIC ID of the current CPU.
* 
* 
*
* RETURNS: the Local APIC ID.
*
*******************************************************************************/
	.globl _lapicGetId
_lapicGetId:

	pushl	%ebp
	movl	%esp, %ebp

    pushl   %ebx

 	movl	$LAPIC_APICID, %ebx
	movl	(%ebx), %eax
	shrl	$24, %eax					# align to bit-0

    popl	%ebx

	leave
	ret

.bss
