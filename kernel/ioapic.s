
/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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

/* idt.S - IDT manipulation primatives
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/ioapic.s,v 1.1 2013-09-04 07:41:28 chippisley Exp $
 *
 * $Log: ioapic.s,v $
 * Revision 1.1  2013-09-04 07:41:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.10  2012/11/02 14:07:39  chippisley
 * Added sysIpiTick(), sysCpuIntLock() & sysCpuIntUnLock().
 *
 * Revision 1.9  2011/03/22 13:45:17  hchalla
 * *** empty log message ***
 *
 * Revision 1.8  2010/09/13 09:32:07  cdobson
 * Added check of CPU ID before setting IPI delivery mode.
 *
 * Revision 1.7  2010/09/13 08:49:30  cdobson
 * Changed IPI delivery mode from edge to level.
 *
 * Revision 1.6  2010/06/23 11:00:42  hmuneer
 * CUTE V1.01
 *
 * Revision 1.5  2010/03/31 16:25:57  hmuneer
 * IOAPIC 1 Support
 *
 * Revision 1.4  2009/06/08 16:10:28  swilson
 * Add function for sending fixed-priority IPI to an AP. Fixe typo in
 *  function header comment.
 *
 * Revision 1.3  2009/05/29 14:09:47  hmuneer
 * no message
 *
 * Revision 1.2  2009/05/20 13:16:55  swilson
 * Add interrupt resource clean-up. All references to 'IRQ' replaced with 'pin' for consistency
 *  with other code.
 *
 * Revision 1.1  2009/05/20 08:58:49  swilson
 * APIC/IOAPIC setup and management functions.
 *
 */


.include "asm/i386.h"


.set APIC_BASE,		0xFEE00000			# Local (CPU) APIC
.set APIC_EOI,		0xFEE000B0			# EOI port
.set APIC_LINT0,	0x350				# EOI port
.set APIC_LINT1,	0x360				# EOI port

.set APIC_ICR_L,	0xFEE00300			# ICR low bytes
.set APIC_ICR_H,	0xFEE00310			# ICR high bytes

.set IOAPIC_BASE,	0xFEC00000			# I/O APIC
.set IOAPIC_INDEX,	0x00				# index register
.set IOAPIC_DATA,	0x10				# data register

.set IOAPIC_INPUTS,		24				# number of inputs
.set IOAPIC_REDIRBASE,	0x10			# start of the redirection table (64-bits per vector)
.set IOAPIC_PIN_MASK,	0x00010000		# interrupt mask bit


.set IOAPIC1_BASE,		0xFEC10000		# I/O APIC
.set IOAPIC1_INDEX,		0x00			# index register
.set IOAPIC1_DATA,		0x10			# data register

.set IOAPIC1_INPUTS,	24				# number of inputs
.set IOAPIC1_REDIRBASE,	0x10			# start of the redirection table (64-bits per vector)
.set IOAPIC1_PIN_MASK,	0x00010000		# interrupt mask bit

.set QM57_OIC,           0xFED1F1FE
.text


/*****************************************************************************
 * sysInitIoApic: initialise the I/O APIC routing
 *
 * ENTRY: none
 *
 * RETURNS: none
 *
 * MODIFIED: none
 *
 */

	.globl _sysInitIoApic
_sysInitIoApic:

	pushal

    movl    $QM57_OIC, %ebx     #enable IOAPIC
    movw    $0x300, 0(%ebx)
    movl    0(%ebx),%edx        #Read flushes the data


	/* Initialize APIC LINT0 and LINT1 */
	movl	$APIC_BASE,  %ebx		 		# load APIC address [EBX]
	movl	$0x00000700, APIC_LINT0(%ebx)
	movl	$0x00000400, APIC_LINT1(%ebx)


	/* Initialize IOAPIC(s) */
	movl	$IOAPIC_BASE, %ebx		 # load IOAPIC address [EBX]
	movl	$IOAPIC_REDIRBASE, %edx	 # redirection base offset
	movl	$0x20, %eax				 # H/W vectors start at 0x20

	/* Initialize the ISA interrupt pins */

	movl	$16, %ecx				 # 16 ISA pins

isaPinLoop:
	movl	%edx, IOAPIC_INDEX(%ebx)		# select IOAPIC register (lower)
	movl	$0x00010000, IOAPIC_DATA(%ebx)	# disabled, edge, high
	orl	%eax, IOAPIC_DATA(%ebx)				# set the vector
	inc	%edx								#

	movl	%edx, IOAPIC_INDEX(%ebx)		# select IOAPIC register (upper)
	movl	$0x00000000, IOAPIC_DATA(%ebx)	# destination = APIC #0
	inc	%edx								#

	inc	%eax								# increment vector# and repeat
	loop	isaPinLoop						#


	/* Initialize the PCI (PIRQ) interrupt pins */

	movl	$8, %ecx						# 8 PIRQ pins, following the ISA

pirqPinLoop:
	movl	%edx, IOAPIC_INDEX(%ebx)		# select IOAPIC register (lower)
	movl	$0x0001A000, IOAPIC_DATA(%ebx)	# disabled, level, low
	orl	%eax, IOAPIC_DATA(%ebx)				# set the vector
	inc	%edx								#

	movl	%edx, IOAPIC_INDEX(%ebx)		# select IOAPIC register (upper)
	movl	$0x00000000, IOAPIC_DATA(%ebx)	# destination = APIC #0
	inc	%edx								#

	inc	%eax								# increment vector# and repeat
	loop	pirqPinLoop						#



	movl	$IOAPIC1_BASE, %ebx		 		# load IOAPIC1 address [EBX]
	movl	$IOAPIC1_REDIRBASE, %edx 		# redirection base offset
	movl	$0x44, %eax				 		# H/W vectors start at 0x20

	/* Initialize the ISA interrupt pins */

	movl	$24, %ecx						# 24 PINS

ioapic1PinLoop:
	movl	%edx, IOAPIC1_INDEX(%ebx)		# select IOAPIC register (lower)
	movl	$0x0001a000, IOAPIC1_DATA(%ebx)	# disabled, Level, Low
	orl	%eax, IOAPIC1_DATA(%ebx)			# set the vector
	inc	%edx								#

	movl	%edx, IOAPIC1_INDEX(%ebx)		# select IOAPIC register (upper)
	movl	$0x00000000, IOAPIC1_DATA(%ebx)	# destination = APIC #0
	inc	%edx								#

	inc	%eax								# increment vector# and repeat
	loop	ioapic1PinLoop					#

	popal
	ret


/*****************************************************************************
 * sysApicSendEoi: send an EOI for a given vector
 *
 * ENTRY: none
 *
 * RETURNS: none
 *
 * MODIFIED: none
 *
 */
	.globl _sysApicSendEoi
_sysApicSendEoi:

	pushl	%ebx

	movl	$APIC_EOI, %ebx		# issue EOI to APIC
	movl	$0, (%ebx)			#

	popl	%ebx
	ret


  /*
   * C-callable functions
   */

/*****************************************************************************
 * _sysMaskPin: mask an IOAPIC interrupt request
 *
 * RETURNS: 0 if 'pin' is valid, -1 if not, -2 if IOAPIC is invalid
 *
 * PROTOTYPE:
 *
 *	int sysMaskPin (UINT8 bPin)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] bPin - interrupt request pin
 * 	[EBP+0c] IOAPIC - IOAPIC 0 or 1
 */
	.globl _sysMaskPin
_sysMaskPin:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%ecx


	# Check this is a VALID pin
	movl	0x08(%ebp), %ecx		# get pin number in ECX
	movzbl	%cl, %ecx				#

	cmpl	$IOAPIC_INPUTS, %ecx	# < highest pin supported?
	jae	MaskPin_Fail				# no, then error

    # Check this is a VALID IOAPIC [0 or 1]
	movl	0x0c(%ebp), %eax      	# get ioapic number in EAX
	movzbl	%al, %eax			  	#
	cmp     $0x00, %eax
	jz		MASK_IOAPIC_0
	cmp     $0x01, %eax
	jz		MASK_IOAPIC_1
	jmp		MASK_IOAPIC_FAIL

	# Mask the requested slot number
MASK_IOAPIC_0:
	shll	$1, %ecx					# convert pin to a register offset
	addl	$IOAPIC_REDIRBASE, %ecx		# add to redirection base offset
	
	movl	$IOAPIC_BASE, %ebx			# select desired redirection register
	movl	%ecx, IOAPIC_INDEX(%ebx)	#

	movl	IOAPIC_DATA(%ebx), %eax		# set the mask bit to disable Pin
	orl	$IOAPIC_PIN_MASK, %eax			#
	movl	%eax, IOAPIC_DATA(%ebx)		#

	xorl	%eax, %eax					# return 0 to indicate success
	jmp	MaskPin_Done					#

MASK_IOAPIC_1:
	shll	$1, %ecx					# convert pin to a register offset
	addl	$IOAPIC1_REDIRBASE, %ecx	# add to redirection base offset

	movl	$IOAPIC1_BASE, %ebx			# select desired redirection register
	movl	%ecx, IOAPIC1_INDEX(%ebx)	#

	movl	IOAPIC1_DATA(%ebx), %eax	# set the mask bit to disable Pin
	orl		$IOAPIC1_PIN_MASK, %eax		#
	movl	%eax, IOAPIC1_DATA(%ebx)	#

	xorl	%eax, %eax					# return 0 to indicate success
	jmp		MaskPin_Done				#

MaskPin_Fail:
	xorl	%eax, %eax					# return -1 for invalid slot number
	decl	%eax						#
	jmp		MaskPin_Done				#

MASK_IOAPIC_FAIL:
	xorl	%eax, %eax					# return -2 for invalid IOAPIC number
	decl	%eax						#
	decl	%eax						#

MaskPin_Done:
	popl	%ecx
	popl	%ebx

	leave
	ret


/*****************************************************************************
 * _sysUnmaskPin: mask an IOAPIC interrupt request
 *
 * RETURNS: 0 if 'pin' is valid, -1 if not, -2 if IOAPIC is invalid
 *
 * PROTOTYPE:
 *
 *	int sysUnmaskPin (UINT8 bPin)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] bPin - interrupt request pin
 * 	[EBP+0c] IOAPIC - IOAPIC 0 or 1
 */
	.globl _sysUnmaskPin
_sysUnmaskPin:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%ecx

	# Check this is a VALID pin

	movl	0x08(%ebp), %ecx		# get pin number in ECX
	movzbl	%cl, %ecx			#

	cmpl	$IOAPIC_INPUTS, %ecx		# < highest pin supported?
	jae	UnmaskPin_Fail			# no, then error

	# Check this is a VALID IOAPIC [0 or 1]
	movl	0x0c(%ebp), %eax      # get ioapic number in EAX
	movzbl	%al, %eax			  #
	cmp     $0x00, %eax
	jz		UNMASK_IOAPIC_0
	cmp     $0x01, %eax
	jz		UNMASK_IOAPIC_1
	jmp		UNMASK_IOAPIC_FAIL


	# Mask the requested slot number
UNMASK_IOAPIC_0:
	shll	$1, %ecx			# convert pin to a register offset
	addl	$IOAPIC_REDIRBASE, %ecx		# add to redirection base offset
	
	movl	$IOAPIC_BASE, %ebx		# select desired redirection register
	movl	%ecx, IOAPIC_INDEX(%ebx)	#

	movl	IOAPIC_DATA(%ebx), %eax		# set the mask bit to disable Pin
	andl	$~IOAPIC_PIN_MASK, %eax		#
	movl	%eax, IOAPIC_DATA(%ebx)		#

	xorl	%eax, %eax			# return 0 to indicate success
	jmp	UnmaskPin_Done			#

UNMASK_IOAPIC_1:
	shll	$1, %ecx			# convert pin to a register offset
	addl	$IOAPIC1_REDIRBASE, %ecx		# add to redirection base offset

	movl	$IOAPIC1_BASE, %ebx		# select desired redirection register
	movl	%ecx, IOAPIC1_INDEX(%ebx)	#

	movl	IOAPIC1_DATA(%ebx), %eax		# set the mask bit to disable Pin
	andl	$~IOAPIC1_PIN_MASK, %eax		#
	movl	%eax, IOAPIC1_DATA(%ebx)		#

	xorl	%eax, %eax			# return 0 to indicate success
	jmp		UnmaskPin_Done			#


UnmaskPin_Fail:
	xorl	%eax, %eax			# return -1 for invalid slot number
	decl	%eax				#
	jmp		UnmaskPin_Done		#

UNMASK_IOAPIC_FAIL:
	xorl	%eax, %eax			# return -2 for invalid IOAPIC number
	decl	%eax				#
	decl	%eax				#

UnmaskPin_Done:
	popl	%ecx
	popl	%ebx

	leave
	ret


/*****************************************************************************
 * _sysMaskAll: mask all IOAPIC interrupt requests
 *
 * RETURNS: 0 if 'pin' is valid, -1 if not
 *
 * PROTOTYPE:
 *
 *	void sysMaskAll (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysMaskAll
_sysMaskAll:

	pushl	%ebp
	movl	%esp, %ebp

	pushal

	# Mask the requested slot number

	movl	$IOAPIC_INPUTS, %ecx		# number of pins
	movl	$IOAPIC_REDIRBASE, %edx		# redirection base offset
	
maskAll_Loop:
	movl	$IOAPIC_BASE, %ebx		# select desired redirection register
	movl	%edx, IOAPIC_INDEX(%ebx)	#

	movl	IOAPIC_DATA(%ebx), %eax		# set the mask bit to disable Pin
	orl	$IOAPIC_PIN_MASK, %eax		#
	movl	%eax, IOAPIC_DATA(%ebx)		#

	loop	maskAll_Loop


# Mask the requested slot number

	movl	$IOAPIC1_INPUTS, %ecx		# number of pins
	movl	$IOAPIC1_REDIRBASE, %edx	# redirection base offset

maskAll1_Loop:
	movl	$IOAPIC1_BASE, %ebx			# select desired redirection register
	movl	%edx, IOAPIC1_INDEX(%ebx)	#

	movl	IOAPIC1_DATA(%ebx), %eax	# set the mask bit to disable Pin
	orl		$IOAPIC1_PIN_MASK, %eax		#
	movl	%eax, IOAPIC1_DATA(%ebx)	#

	loop	maskAll1_Loop

	popal

	leave
	ret


/*****************************************************************************
 * _sysEnableInterrupts: enable interrupts at CPU
 *
 * RETURNS: none
 *
 * PROTOTYPE:
 *
 *	void sysEnableInterrupts (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysEnableInterrupts
_sysEnableInterrupts:

	sti
	ret


/*****************************************************************************
 * _sysDisableInterrupts: disable interrupts at CPU
 *
 * RETURNS: none
 *
 * PROTOTYPE:
 *
 *	void sysDisableInterrupts (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysDisableInterrupts
_sysDisableInterrupts:

	cli
	ret


/*****************************************************************************
 * _sysCpuIntLock: lock out interrupts on the local CPU
 *
 * RETURNS: current EFLAGS and this 'key' should be passed to sysCpuIntUnlock() 
 *          to re-enable interrupts. 
 *
 * PROTOTYPE:
 *
 *	UINT32 sysCpuIntLock( void )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysCpuIntLock
_sysCpuIntLock:
	pushl	%ebp
	movl	%esp, %ebp
	pushf				/* push EFLAGS on stack */
	cli					/* disable interrupts */
	popl 	%eax		/* get EFLAGS in EAX and return */
	leave
	ret


/*****************************************************************************
 * _sysCpuIntUnlock: unlock interrupts on the local CPU.
 *                   Uses the lock-out key obtained from a preceding call to 
 *                   sysCpuIntLock(). 
 *
 * RETURNS: none. 
 *
 * PROTOTYPE:
 *
 *	void sysCpuIntUnlock( UINT32 key )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 *	[EBP+08] lock-out key 
 */
	.globl _sysCpuIntUnlock
_sysCpuIntUnlock:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl	0x08(%ebp),%eax		/* get key in EAX */
	testl	$0x00000200,%eax	/* is IF bit set in key? */
	jz	CpuIntUnlock			/* no, skip next instruction */
	sti							/* enable interrupts */

CpuIntUnlock:
	leave
	ret


/*****************************************************************************
 * _sysPinToVector: disable interrupts at CPU
 *
 * RETURNS: 0 if 'pin' is valid, -1 if not, -2 if IOAPIC is invalid
 *
 * PROTOTYPE:
 *
 *	int sysPinToVector (UINT8 bPin)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] bPin - interrupt request pin
 * 	[EBP+0c] IOAPIC - IOAPIC 0 or 1
 */
	.globl _sysPinToVector
_sysPinToVector:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%ecx

	movl	0x08(%ebp), %eax		# get pin number in [EAX]
	movzbl	%cl, %ecx			#

	cmpl	$IOAPIC_INPUTS, %eax		# < highest pin supported?
	jae	PinToVector_Fail		# no, then error

	# Check this is a VALID IOAPIC [0 or 1]
	movl	0x0c(%ebp), %ebx        # get ioapic number in EAX
	movzbl	%bl, %ebx			    #
	cmp     $0x00, %ebx
	jz		PinToVector_IOAPIC_0
	cmp     $0x01, %ebx
	jz		PinToVector_IOAPIC_1
	jmp		PinToVector_IOAPIC_FAIL

PinToVector_IOAPIC_0:
	addl	$0x20, %eax			# hardware vectors start at 0x20
	jmp	PinToVector_Done		#

PinToVector_IOAPIC_1:
	addl	$0x44, %eax			# hardware vectors start at 0x44
	jmp	PinToVector_Done		#

PinToVector_Fail:
	xorl	%eax, %eax			# return -1 for invalid slot number
	decl	%eax				#
	jmp		PinToVector_Done	#

PinToVector_IOAPIC_FAIL:
	xorl	%eax, %eax			# return -2 for invalid IOAPIC number
	decl	%eax				#
	decl	%eax				#

PinToVector_Done:
	popl	%ecx
	popl	%ebx

	leave
	ret


/*****************************************************************************
 * _sysSendIpi: send a directed IPI vector to a specified CPU core.
 *
 * RETURNS: 0 if successful
 *	    -1 if timeout sending IPI
 *	    -2 if invalid vector specified
 *
 * PROTOTYPE:
 *
 *	int sysSendIpi (UINT8 bApicId, UINT8 bVector)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 *	[EBP+08] bApicId  - the physical APIC number for targetted CPU core
 *	[EBP+0C] bVector  - vector number to issue (normally a SW interrupt)
 */
	.globl _sysSendIpi
_sysSendIpi:

	pushl	%ebp
	movl	%esp, %ebp

	/* Get core/APIC number */

	movl	0x08(%ebp), %eax		# get APIC ID in [EAX]
	shll	$24, %eax			# align for ICR[63:56]
	movl	%eax, APIC_ICR_H		# write to ICR(high)

	/* Get vector and validate */

	movl	0x0C(%ebp), %eax		# get vector in [EAX]

	cmpb	$IDT_SLOTS, %al			# check vector is valid
	jae	SendIpi_Param			# fail if not

	/* Send a fixed IPI to the specified core */

	movzbl	%al, %eax			# zero upper bits
	
	/* check CPU type */
	
	pushl	%eax
	movl	$1, %eax
	cpuid
	andl	$0xffff0, %eax
	cmpl	$0x20650, %eax
	popl	%eax
	je		SendIpi_nehalem
	
	movb	$0x40, %ah			# set edge mode (level must be 1)
	jmp		SendIpi_Cont

SendIpi_nehalem:
	movb	$0xC0, %ah			# set level mode
	
SendIpi_Cont:
	movl	%eax, APIC_ICR_L		# write to ICR(low) to issue IPI

	/* Now wait until IPI accepted by core, or timeout */

	movl	$200, %ecx			# wait up to 200mS for core to accept

SendIpi_Loop:
	movl	APIC_ICR_L, %eax		# read ICR(low)
	testw	$0x1000, %ax			# test delivery status
	jz	SendIpi_Done			# exit once idle

	movl	$1, %eax			# delay 1 mS
	call	_msDelay				#

	loop	SendIpi_Loop			# repeat until timeout


SendIpi_Fail:
	xorl	%eax, %eax			# return -1 for timeout
	decl	%eax				#
	jmp	SendIpi_Exit			#

SendIpi_Param:
	xorl	%eax, %eax			# return -2 for param error
	decl	%eax				#
	decl	%eax				#
	jmp	SendIpi_Exit			#

SendIpi_Done:
	xorl	%eax, %eax			# return 0 if Ok

SendIpi_Exit:
	leave
	ret


/*****************************************************************************
 * _sysIpiTick: send a directed IPI vector to a specified CPU core.
 *              Only to be called from prvPreemptiveTick(), must not delay or
 *              block. 
 *
 * RETURNS: 0
 *	    
 *
 * PROTOTYPE:
 *
 *	int sysIpiTick (UINT8 bApicId, UINT8 bVector)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 *	[EBP+08] bApicId  - the physical APIC number for targetted CPU core
 *	[EBP+0C] bVector  - vector number to issue (normally a SW interrupt)
 */
	.globl _sysIpiTick
_sysIpiTick:

	pushl	%ebp
	movl	%esp, %ebp

	/* Get core/APIC number */

	movl	0x08(%ebp), %eax	# get APIC ID in [EAX]
	shll	$24, %eax			# align for ICR[63:56]
	movl	%eax, APIC_ICR_H	# write to ICR(high)

	/* Get vector */

	movl	0x0C(%ebp), %eax	# get vector in [EAX]

	/* Send a fixed IPI to the specified core */

	movzbl	%al, %eax			# zero upper bits
	
	/* check CPU type */
	
	pushl	%eax
	movl	$1, %eax
	cpuid
	andl	$0xffff0, %eax
	cmpl	$0x20650, %eax
	popl	%eax
	je		IpiTick_nehalem
	
	movb	$0x40, %ah			# set edge mode (level must be 1)
	jmp		IpiTick_Cont

IpiTick_nehalem:
	movb	$0xC0, %ah			# set level mode
	
IpiTick_Cont:
	movl	%eax, APIC_ICR_L	# write to ICR(low) to issue IPI

	xorl	%eax, %eax			# return 0

	leave
	ret

.bss

