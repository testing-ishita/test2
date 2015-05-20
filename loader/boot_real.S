
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

/* BOOT_REAL.S - 1st-stage Entry Point
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/loader/boot_real.S,v 1.12 2015-04-02 11:42:54 mgostling Exp $
 *
 * $Log: boot_real.S,v $
 * Revision 1.12  2015-04-02 11:42:54  mgostling
 * Add support for using COM3 as the debug port
 *
 * Revision 1.11  2014-12-15 12:29:43  mgostling
 * Disable DEBUG setting as it interferes with COM port testing in production test.
 *
 * Revision 1.10  2014-11-10 16:24:29  mgostling
 * Updated for CUTE 2.04.01 for VRE1x, removing temporary fix added for AMC1x  CUTE V2.01.01
 *
 * Revision 1.9  2014-11-10 11:03:00  mgostling
 * Force baudrate to 115200 as AMC1x BIOS bug does not pass correct info in EDI register.
 *
 * Revision 1.8  2014-06-18 08:19:22  mgostling
 * Reset COMPort to 1.
 * Added DEBUG conditional directive around code that forces debug settings.
 *
 * Revision 1.7  2014-05-12 16:10:15  jthiru
 * Adding support for VX91x and fixes to existing bugs
 *
 * Revision 1.6  2014-03-06 13:42:38  hchalla
 * Set Debug flag to 1 for enabling AM 94x debugging in soak.
 *
 * Revision 1.5  2014-03-04 10:38:01  mgostling
 * Restoring COM1 as default port.
 *
 * Revision 1.4  2014-03-04 10:36:38  mgostling
 * Added conditional code to display EDI passed from BIOS.
 * Added conditional code to add a 5 second delay before jumping to BIT 2nd stage.
 *
 * Revision 1.3  2014-02-14 11:43:58  chippisley
 * Forced debug mode on to allow reporting of initial startup messages to aid
 * test department and diagnostics.
 *
 * Revision 1.2  2013/09/04 13:28:35  chippisley
 * Commented out debug force.
 *
 * Revision 1.1  2013/09/04 07:42:08  chippisley
 * Import files into new source repository.
 *
 * Revision 1.22  2012/11/02 14:15:21  chippisley
 * Added conditionals for pcbrd.
 *
 * Revision 1.21  2011/11/21 11:16:56  hmuneer
 * no message
 *
 * Revision 1.20  2011/11/14 17:33:46  hchalla
 * Updated for PP 531.
 *
 * Revision 1.19  2011/06/13 12:33:23  hmuneer
 * no message
 *
 * Revision 1.18  2011/06/07 14:56:12  hchalla
 * Removed the support for serial port baud rate below 9600, the minimum baud rate now is 9600.
 *
 * Revision 1.17  2011/05/16 14:31:43  hmuneer
 * Info Passing Support
 *
 * Revision 1.16  2011/03/22 13:46:57  hchalla
 * Added support for SMP for VX813.
 *
 * Revision 1.15  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.14  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.13  2010/10/08 12:15:53  jthiru
 * CMOS location 0x25 changed to 0x70
 *
 * Revision 1.12  2010/06/23 11:00:52  hmuneer
 * CUTE V1.01
 *
 * Revision 1.11  2010/05/05 15:49:10  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.10  2009/05/18 14:22:50  swilson
 * Add error-code handling in the case of checksum fauilures, or BIT transition to C-code.
 *  Fix bug in SMP init: MSB of EBP was not initialized.
 *
 * Revision 1.9  2009/05/15 11:08:41  jthiru
 * Modified checksum calculation using dword
 *
 * Revision 1.8  2009/04/07 15:27:31  swilson
 * Tidy-up comments
 *
 * Revision 1.7  2008/09/19 14:43:05  swilson
 * Update description of command-line flags. Change console output function to honour
 *  'debug' flag.
 *
 * Revision 1.6  2008/07/18 12:44:59  swilson
 * Can't use cs: segment overrides where data is modified as CPU may have write-to-executable-segment protection enabled.
 *
 * Revision 1.5  2008/07/10 16:51:30  swilson
 * Add calibrated delay fuunctions. Re-code loader to support SMP initialization and
 * add SMP init.
 *
 * Revision 1.4  2008/06/20 12:16:54  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 * C-code.
 *
 * Revision 1.3  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 * Revision 1.2  2008/04/21 16:59:24  swilson
 * Basic framework, now with stack, initialised data and C-to-asm linkage all apparently working.
 *
 */

.include "asm/i386.h"

#.set DEBUG, 1
#.set FORCE_STH, 1
#.set STARTUP_DELAY, 1

# reset COMPort to 1 once debugging is complete!
.set COMPort, 1
#.set COMPort, 2
#.set COMPort, 3

# IMPORTANT:
# If the COMPort is changed to a different port,
# then "bDebugPort" at the start of  board_cfg.c must be
# changed accordingly as well

# 16550 compatible COM2 serial port
.if COMPort == 2
	.set COM_TX,	0x02F8		# transmit register
	.set COM_LCR,	0x02FB		# Line Control Register
	.set COM_DLL,	0x02F8		# Divisor Low
	.set COM_DLH,	0x02F9		# Divisor High
	.set COM_IER,	0x02F9		# Interrupr control
	.set COM_LSR,	0x02FD		# line status register
.set LSR_TX,	0x20		#  TX empty bit
.endif

# 16550 compatible COM1 serial port
.if COMPort == 1
	.set COM_TX,	0x03F8		# transmit register
	.set COM_LCR,	0x03FB		# Line Control Register
	.set COM_DLL,	0x03F8		# Divisor Low
	.set COM_DLH,	0x03F9		# Divisor High
	.set COM_IER,	0x03F9		# Interrupr control
	.set COM_LSR,	0x03FD		# line status register
	.set LSR_TX,	0x20		# TX empty bit
.endif

# 16550 compatible COM3 serial port
.if COMPort == 3
	.set COM_TX,	0x03E8		# transmit register
	.set COM_LCR,	0x03EB		# Line Control Register
	.set COM_DLL,	0x03E8		# Divisor Low
	.set COM_DLH,	0x03E9		# Divisor High
	.set COM_IER,	0x03E9		# Interrupr control
	.set COM_LSR,	0x03ED		# line status register
	.set LSR_TX,	0x20		# TX empty bit
.endif

# BIT Flags (passed in EDI)

.set BITF_DIAG,	0x00000001	# 1 = diagnostic port active (3F8:9600,8,N,1)
.set BITF_MODE,	0x00000002	# 0 = Local IBIT mode, 1 = PBIT
.set BITF_AP,	0x00000004	# BSP = 0, AP = 1


.code16
.text

/*
 * 1st Stage Signature
 */
.ifndef MAKE_CUTE
sig_start:
	.byte	'B'			# 1st Stage signature
	.byte	'I'			# first byte in the binary
	.byte	'T'			#
	.byte	'1'			#
.else
sig_start:
	.byte	'C'			# 1st Stage signature
	.byte	'U'			# first byte in the binary
	.byte	'T'			#
	.byte	'E'			#
.endif

csum_value:
	.long	0			# place-holders for: image checksum
image_size:
	.long	0			# 1st stage image size
code32_start:
.ifdef COMPRESSED
	.long	0x300000	# 2nd stage load address
.else
.ifdef PCBRD
	.long	0x200000	# 2nd stage load address
.else
	.long	0x100000	# 2nd stage load address
.endif
.endif



/*
 * Real-mode Entry point - called from the BIOS (BSP) or SMP init (AP)
 * ===========================================================================
 *
 * Things we assume when entering here:
 *
 * 1) we are running in real mode
 *
 * 2) EBX contains the physical address of the binary image (BSP only)
 *
 * 3) EDI contains flags as follows:
 *	[0]   - 1 = diagnostic port active (3F8:9600,8,N,1)
 *	[1]   - 0 = Local IBIT mode, 1 = PBIT
 *	[2]   - 0 = BSP, 1 = AP
 *	[3]   - 0 = BIT, 1 = CUTE	:only matters for combined BIT/CUTE image
 *  [5-4] - power-on BIT duration
 *  [7-6] - test failure action
 *  [8]   - 1 = ESI contains address of BIOS_INFO structure
 *  [9-11]- Serial port(s) baud rate
 *
 * 4) ESI contains the address of BIOS_INFO structure
 *
 * 5) Memory location at 0x9000 is used by SMP semaphore, it is shared by
 *    both real mode and protected mode code
 *
 * 6) Meomory location 0x9004 is used to save contents of ESI during realmode
 *    phase of BSP
 *
 * 7) Real Mode port-80 codes are in range 0xCCA1-0xCCAF
 */

	.globl	real_entry
real_entry:

	jmp	start_of_code

ap_flag:
	.long	0						# zero if BSP, non-zero for all APs

/*
 * Body of real-mode code
 */

	.align 16

.ifdef MAKE_BIT_CUTE
cute_sig:
	.byte	'C'						# cute signature for combined bit/cute
	.byte	'U'						# 32nd byte in the binary
	.byte	'T'
	.byte	'E'
.endif


start_of_code:

	movw	$0xCCA1, %ax			# CUTE START
	outw	%ax, $0x80				# PORT-80 (for ICE breakpoint)

	cli
	cld

	//only wait for semaphore if not BSP
	movw	%cs, %ax				# set DS = ES = CS
	movw	%ax, %ds				#
	movw	%ax, %es				#
	orl		$0, %es:ap_flag			# if not BSP jump to sema_wait
	jnz		sema_wait				#

	//save ESI to 0x9004
	movw	$0, %ax					# set DS = 0
	movw	%ax, %ds				#
	movl	$0x9004, %eax			# copy of ESI 0x9004
	movl	%esi, (%eax)			# save esi to 0x9004

	//clear 0x9000 to 0
	movw	$0, %ax					# set DS = 0
	movw	%ax, %ds				#
	movl	$0x9000, %eax			# MP semaphore at physical 0x9000
	movl	$0, (%eax)				# clear it to 0
	jnz		init_Start				# then jump to init

/* Acquire the Initialization Semaphore before proceeding */
sema_wait:
	movw	$0, %ax					# set DS = 0
	movw	%ax, %ds				#
	movl	$0x9000, %eax			# MP semaphore at physical 0x9000
	pause
	lock
	btsl	$0, (%eax) 				# wait until we set the semaphore
	nop								# NOPs for ICE break-in
	nop								#
	nop								#
	jc		sema_wait				# wait for _us_ to set the semaphore

init_Start:
	movw	%cs, %ax				# set DS = ES = CS
	movw	%ax, %ds				#
	movw	%ax, %es				#

.if DEBUG == 1
									# force debug port settings
	#andl	$0xfffff1ff, %edi		# should be removed from release builds
	orl     $0x00000801, %edi		# set debug mode on & baud 115200
	#orl     $0x00000800, %edi		# set baud 115200
	#orl     $0x00000001, %edi		# set debug mode on
.endif


.if FORCE_STH == 1
	orl     $0x00000002, %edi		# for forcing STH mode
									# should be removed from release builds
.endif

	/* Clear Protection and Paging bits */
	movl	%cr0, %eax				#
	andl	$0x7FFFFFFE, %eax		#
	movl	%eax, %cr0				#

	movw	$0xCCA2, %ax			# CUTE START
	outw	%ax, $0x80				# PORT-80 (for ICE breakpoint)

	/*
	 * Init COM1 if BSP
	*/
	orl		$0, %es:ap_flag			#
	jnz		APMSG					#

	call 	serInit					# Init COM1 if debugging enabled
	
.if DEBUG == 1
									# For code debugging only
	lea	BSP_init_msg, %bx			# BSP Init message if debugging enabled
	call	serOutString			# send to COM1
.endif
	
	jmp  chk1


APMSG:
.if DEBUG == 1
									# For code debugging only
	lea	AP_init_msg, %bx			# AP Init message if debugging enabled
	call	serOutString			# send to COM1
.endif

	/*
	 * Quick checksum of 1st-stage
	 */
chk1:
	orl	$0, %es:ap_flag				# only do checksum if BSP
	jnz	checksum1_ok				#

	movl	$0x01000001, %edx		# preload error code - E__1ST_STAGE_CSUM
	call	postBitError			# in case of  checksum fail

	movl	%cs:(image_size), %ecx	# get 1st-stage image size
	subl	$8, %ecx				# don't include signature and checksum

	lea		image_size, %si			# [DS:SI] points to start of checksummed data

	xorl	%edx, %edx				# zero checksum [EDX] and upper [EAX]
	xorl	%eax, %eax				#

checksum1_loop:
	lodsb							# sum bytes in [EDX]
	add		%eax, %edx				#
	loop	checksum1_loop			#

	cmpl	%edx, %cs:(csum_value)	# verify we got the correct checksum
	je		checksum1_ok			#

	/* Checksum is bad; signal error via Port-80, then halt */

	movb	$0xE0, %al				# send error to PORT-80
	outb	%al, $0x80				#

	lea		checksum1_msg, %bx		# ptr to message
	call	serOutString			# send to COM1

	hlt								# HALT: not safe to continue

checksum1_ok:
	/*
	 * Prepare for protected mode switch
	 */

	orl		$0, %es:ap_flag			# only mask PICs if BSP
	jnz		skip_mask_all			#

	lea		checksum1_OK, %bx		# Checksum1 OK message if debugging enabled
	call	serOutString			# send to COM1

.if DEBUG == 1
									# display parameters passed in by BIOS
	pushl	%edi					# save registers
	pushw	%ax
	pushw	%cx
	lea		parameter_value+7, %bx	# BSP Init message if debugging enabled
	movw	$8, %cx					# number of hex digits

bNextDigit:
	movw	%di, %ax				# get next digit
	andb	$0x0F, %al				# isolate digit
	cmpb	$9, %al					# check range
	jbe		bDecimal
	subb	$10, %al				# adjust for hex digit
	addb	$0x41, %al				# ascii character for hex number
	jmp		bHexDigit

bDecimal:
	addb	$0x30, %al				# ascii character for decimal number

bHexDigit:
	movb	%al, %cs:(%bx)			# save in buffer

	decw	%bx						# point to next entry in buffer
	decb	%cl						# check number of digits processed
	shrl	$4, %edi				# adjust EDI for next digit
	orb		%cl, %cl				# finished?
	jnz		bNextDigit

	popw	%cx						# restore registers
	popw	%ax
	popl	%edi

	lea		parameter_msg, %bx		# display EDI value
	call	serOutString			# send to COM port
.endif


	xorl	%ebp, %ebp				# save real-mode start address in EBP
	pushw	%cs						#
	popw	%bp						#
	movzwl	%bx, %ebx				#
	shll	$4, %ebp				#

	/* Mask all interrupts */

	movb	$0xFF, %al				# slave PIC
	outb	%al, $0xA1				#
	movb	$0xFB, %al				# master PIC, leave cascade (IRQ2)
	outb	%al, $0x21				#
									#
	movb	$0x80, %al				# mask NMI via CMOS index register bit-8
	outb	%al, $0x70				#

skip_mask_all:

	/*  Ensure cache is enabled */

	movl	%cr0, %eax				#
	andl	$0x9FFFFFFF, %eax		# clear CD and NW bits
	movl	%eax, %cr0				#

	/* Reset coprocessor */

	xorw	%ax, %ax				# clear IRQ13 and assert #IGNNE
	outb	%al, $0xf0				#
	call	delay					#

	outb	%al, $0xf1				# reset FPU
	call	delay					#


	/* 
	 * If debug console is enabled, output a BSP sign-on message
	 */

	orl	$0, %es:ap_flag				# only display if BSP
	jnz	skip_signon					#

	lea	bsp_signon_msg, %bx			# ptr to message
	call	serOutString			# send to COM1

skip_signon:


	/*
	 * Switch to protected mode
	 */

	orl	$0, %es:ap_flag				# only enable A20 if BSP
	jnz	skip_a20					#

	/* Patch 'jmpi' instruction with the 2nd-stage loader entry point */

	movl	%cs:code32_start, %eax	# get build location of 2nd-stage
	addl	$12, %eax				# skip the header part
	movl	%eax, %cs:code32		# update 'jmpi' code below

	/* Enable A20 */

	inb	$0x92, %al					# Port-92
	orb	$0x02, %al					# set 	#A20 gate
	outb	%al, $0x92				#

skip_a20:

	/* Load GDT and IDT */

	orl	$0, %es:ap_flag				# only enable A20 if BSP
	jnz	skip_IDT_GDT				#

	lidt	%cs:idt_48				# load IDT with 0,0

	xorl	%eax, %eax				# get linear address of GDT
	movw	%cs, %ax				#
	shll	$4, %eax				#
	addl	$gdt, %eax				#
	movl	%eax, (gdt_48+2)		# save for use with LGDT

	lgdt	%cs:gdt_48				# load GDT

skip_IDT_GDT:
	/* Set PE bit */

	movl	%cr0, %eax				#
	orb	$1, %al						# set protection enable (PE) bit
	movl	%eax, %cr0				#

	jmp	flush_queue					# flush the prefetch queue
flush_queue:


	/*
	 * Checksum the 2nd-stage image before passing control
	 */

	orl	$0, %es:ap_flag				# only do checksum if BSP
	jnz	checksum2_ok				#

	movl	$0x01000002, %edx		# preload error code - E__2ND_STAGE_CSUM
	call	postBitError			# in case of  checksum fail

	movl	%cs:code32_start, %esi	# get build location of 2nd-stage in [ESI]
	movw	$DS_SEL, %ax			# load DS with 4GB data selector
	movw	%ax, %ds

	.byte 0x67						# 32-bit address prefix
	lodsl							# skip over signature

	.byte 0x67						# 32-bit address prefix
	lodsl							# put expected checksum in [EBX]
	movl	%eax, %ebx				#

	movl	(%esi), %ecx			# get length in [ECX] ... don't increment [ESI], yet
	subl	$8, %ecx				# don't include signature and checksum

	xorl	%edx, %edx				# zero checksum [EDX] and upper [EAX]
	xorl	%eax, %eax				#

checksum2_loop:
	.byte 0x67						# 32-bit address prefix
	lodsb							# sum bytes in [EDX]
	addl	%eax, %edx				#
	.byte 0x67
	loop	checksum2_loop			#

	cmpl	%edx, %ebx				# verify we got the correct checksum
	je	checksum2_ok				#

	/* Checksum is bad; send a message to the serial port, then halt */

	lea	checksum2_msg, %bx			# ptr to message
	call	serOutString			# send to COM1

	movb	$0xE1, %al				# send error to PORT-80
	outb	%al, $0x80				#

	hlt								# HALT: not safe to continue

checksum2_ok:

	/*
	 * BSP only: replace the signature bytes at the head of this code 
	 * with NOPs, as the AP start address must be 4KB aligned.
	 */

	orl	$0, %es:ap_flag				# only clear signature if BSP
	jnz	set_AP_flag					#

	lea		checksum2_OK, %bx		# Checksum2 OK message if debugging enabled
	call	serOutString			# send to COM1

.if STARTUP_DELAY == 1
	lea		Delay5Sec_msg, %bx		# Checksum2 OK message if debugging enabled
	call	serOutString			# send to COM1

	pushw	%ax
	pushw	%cx
	movw	$5000, %ax				#5 second delay
	movw	%ax, %cx
millisecond:
	movw	%cx, %ax				#save number of milliseconds
	movw	$67, %cx				#66.6 recurring * 15us for 1 mil
togglewait:
	pushw	%ax
	in		$0x61, %al				#get current setting
	andb	$0x10, %al
	movb	%al, %ah				#save it
tryAgain:
	out		%al, $0x0ED				#I/O delay cycles
	in		$0x61, %al				#get it again
	andb	$0x10, %al
	cmpb	%ah, %al				#check refresh - 15us
	jz		tryAgain				#wait for refresh to change
	popw	%ax
	loop 	togglewait				#continue for this millisecond
	movw	%ax, %cx				#recover number of milliseconds
	loop	millisecond
	popw	%cx
	popw	%ax
.endif

	movl	%ebp, %ebx				#
	movl	$0x90909090, %eax		# EAX = 4 * NOP

	movl	%eax, 0x00(%ebx)		# replace signature with NOPs
	movl	%eax, 0x04(%ebx)		#
	movl	%eax, 0x08(%ebx)		#
	movl	%eax, 0x0C(%ebx)		#

	incl	%es:ap_flag				# increment AP flag, prevents BSP code re-running

	jmp	call_2nd_stage				#


	/* AP only: set the AP flag in EDI so 2nd stage knows CPU type */

set_AP_flag:
	or	$BITF_AP, %edi				# signal to 2nd-stage that this is an AP

	movl	$0x01000003, %edx		# preload error code - E__2ND_STAGE_LAUNCH
	call	postBitError			# in case of failure

	/*
	 * Pass control to the 2nd-stage - far jump loads code selector
	 * EDI is unmodified
	 * EBP contains the address of this routine
	 */

call_2nd_stage:
	.byte 0x66						# 32-bit instruction prefix
	.byte 0xea						# jmpi-opcode
code32:	.long	0x12345678			# offset of 2nd-stage entry (patched above)
	.word	CS_SEL					# code selector

	hlt								# just in-case jump fails


/*****************************************************************************
 * postBitError: write a 32-bit BIT error code to CMOS: NV2 60h onwards
 *
 * Input:
 *	EDX: error code
 *
 * Output:
 *	None
 */

.ifdef PCBRD
/* Don't change CMOS on Virtual PC Configuration */
postBitError:

	ret
	
.else
postBitError:
	pushw	%ax

	movb	$0x60, %ah				# CMOS register # in [AH]

ecLoop:
	movb	%ah, %al				# set CMOS register
	outb	%al, $0x72				#
	movb	%dl, %al				# set value
	outb	%al, $0x73				#

	rorl	$8, %edx				# get next byte

	inc	%ah							# repeat for all bytes
	cmp	$0x63, %ah					#
	jbe	ecLoop						#

	popw	%ax
	ret
.endif


/*****************************************************************************
 * serInit: initializes COM1
 *
 * Input:
 *	none
 *
 * Output:
 *	None
 */

serInit:

	pushw	%ax
	pushw	%dx
	pushl   %ebx


	/* Only write to port if debugging enabled */
	testw	$1, %di					# check 'debug' bit in passed params
	jz	    init_done				# skip if not set

	movw	$COM_LCR, %dx			# Program baud rate divisor
	inb		%dx, %al				# DLAB = 1 to access divisor
	orb 	$0x80, %al				#
	outb	%al, %dx				#

	movl    %edi, %ebx
	shrl   	$9, %ebx
	andl	$0x00000007, %ebx

	cmp     $0x00, %ebx
	jz		baud_9600

	cmp     $0x01, %ebx
	jz		baud_19200

	cmp     $0x02, %ebx
	jz		baud_38400

	cmp     $0x03, %ebx
	jz		baud_57600

	cmp     $0x04, %ebx
	jz		baud_115200

	jmp     baud_9600				# default to 9600 baud


baud_9600:
	movw	$COM_DLL, %dx			# Write Divisor Low
	movb 	$0x0C, %al				#
	outb	%al, %dx				#
	jmp     baud_done


baud_19200:
	movw	$COM_DLL, %dx			# Write Divisor Low
	movb 	$0x06, %al				#
	outb	%al, %dx				#
	jmp     baud_done


baud_38400:
	movw	$COM_DLL, %dx			# Write Divisor Low
	movb 	$0x03, %al				#
	outb	%al, %dx				#
	jmp baud_done


baud_57600:
	movw	$COM_DLL, %dx			# Write Divisor Low
	movb 	$0x02, %al				#
	outb	%al, %dx				#
	jmp baud_done


baud_115200:
	movw	$COM_DLL, %dx			# Write Divisor Low
	movb 	$0x01, %al				#
	outb	%al, %dx				#


baud_done:
	movw	$COM_DLH, %dx			# Write Divisor High
	movb 	$0, %al					#
	outb	%al, %dx				#

	movw	$COM_LCR, %dx			# Program baud rate divisor
	inb		%dx, %al				# clear DLAB bit
	andb 	$0x7f, %al				#
	outb	%al, %dx				#
	movb 	$3, %al					# Select 8N1 format
	outb	%al, %dx				#

	movw	$COM_IER, %dx			# Disable interrupts
	movb 	$0, %al					#
	outb	%al, %dx				#

	lea		com_init_msg, %bx		# ptr to message
	call	serOutString			# send to COM1

init_done:
	popl    %ebx
	popw	%dx
	popw	%ax
	ret


/*****************************************************************************
 * serOutString: Output string CS:BX to COM1
 *
 * Input:
 *	BX: pointer to NULL terminated string
 *
 * Output:
 *	None
 */

serOutString:

	pushw	%dx
	pushw	%ax

	/* Only write to port if debugging enabled */

	testw	$1, %di					# check 'debug' bit in passed params
	jz	tx_done						# skip if not set

	/* Write string to port */

tx_loop:
	movw	 $COM_LSR, %dx			# wait for TX register empty
	inb	%dx, %al					#
	testb	$LSR_TX, %al			# 1 = empty
	jz	tx_loop						#

	movb	%cs:(%bx), %al			# read char from string
	orb	%al, %al					# is it NULL?
	jz	tx_done						# yes, we're done

	incw	%bx						# increment string pointer

	movw	$COM_TX, %dx			# output the character
	outb	%al, %dx				#

	jmp	tx_loop						# repeat

tx_done:
	popw	%ax
	popw	%dx
	ret


/*****************************************************************************
 * delay: dummy port operation to provide a delay
 *
 * Input:
 *	None
 *
 * Output:
 *	None
 */

delay:
	outb	%al, $0x80				# dummy I/O operation
	ret								#


/*
 * Preliminary GDT and IDT
 */
	.align 16
gdt:
	.word	0						# NULL descriptor
	.word	0						#
	.word	0						#
	.word	0						#

	.word	0xFFFF					# 4Gb - (0x100000*0x1000 = 4Gb)
	.word	0						# base address = 0
	.word	0x9A00					# code read/exec
	.word	0x00CF					# granularity = 4096, 386
									#  (+5th nibble of limit)

	.word	0xFFFF					# 4Gb - (0x100000*0x1000 = 4Gb)
	.word	0						# base address = 0
	.word	0x9200					# data read/write
	.word	0x00CF					# granularity = 4096, 386
									#  (+5th nibble of limit)
gdt_end:

	.align	4

	.word	0						# alignment byte
idt_48:
	.word	0						# idt limit = 0
	.word	0, 0					# idt base = 0L


	.word	0						# alignment byte
gdt_48:
	.word	gdt_end - gdt - 1		# gdt limit
	.word	0, 0					# gdt base (filled in later)


/*
 * Strings used by 1st-stage loader, when debugging enabled
 */

.ifdef MAKE_BIT_CUTE
bsp_signon_msg:
	.ascii	"Concurrent Technologies BIT/CUTE: 1st-stage"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL
.endif

.ifdef MAKE_CUTE
bsp_signon_msg:
	.ascii	"Concurrent Technologies CUTE: 1st-stage"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL
.endif

.ifdef MAKE_BIT
bsp_signon_msg:
	.ascii	"Concurrent Technologies BIT: 1st-stage"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL
.endif

com_init_msg:
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.ascii	"COM INIT"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL

BSP_init_msg:
	.ascii	"BSP START"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL

AP_init_msg:
	.ascii	"AP START"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL

checksum1_OK:
	.ascii	"checksum 1 OK"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL

checksum2_OK:
	.ascii	"checksum 2 OK"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL

checksum1_msg:
	.ascii	"1st-stage checksum is invalid"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL

checksum2_msg:
	.ascii	"2nd-stage checksum is invalid"
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL

.if STARTUP_DELAY == 1
Delay5Sec_msg:
	.ascii	"5 second delay ..."
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL
.endif

.if DEBUG == 1
parameter_msg:
	.ascii	"EDI: "
parameter_value:
	.ascii	"00000000"				# value updated in the code
	.byte	0x0D, 0x0A				# <cr>,<lf>
	.byte	0						# NULL
.endif



