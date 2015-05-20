
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/idt.S,v 1.1 2013-09-04 07:41:28 chippisley Exp $
 *
 * $Log: idt.S,v $
 * Revision 1.1  2013-09-04 07:41:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2012/11/02 14:07:07  chippisley
 * Added more exception vectors, pcbrd PIC EOI.
 *
 * Revision 1.7  2011/03/22 13:44:23  hchalla
 * *** empty log message ***
 *
 * Revision 1.6  2010/06/23 11:00:42  hmuneer
 * CUTE V1.01
 *
 * Revision 1.5  2010/03/31 16:25:57  hmuneer
 * IOAPIC 1 Support
 *
 * Revision 1.4  2009/06/08 16:12:00  swilson
 * Add support for four software interrupts. Update comments to reflect this and
 *  clarify other areas.
 *
 * Revision 1.3  2009/05/20 15:51:07  swilson
 * Correction to exception error code reporting.
 *
 * Revision 1.2  2009/05/20 13:23:30  swilson
 * Function names containing 'IRQ' replaced with 'pin' for consistency with other code.
 *  Interrupt resource clean up function added. CPU exceptions now log  error and reboot into BIOS if
 *  no user handler available.
 *
 * Revision 1.1  2009/05/20 08:57:48  swilson
 * IDT Creation and default Exception/Interrupt handlers.
 *
 */


.include "asm/i386.h"


.text

idt:	.long	sysExeption00		# CPU exceptions
	.long	sysExeption01
	.long	sysExeption02
	.long	sysExeption03
	.long	sysExeption04
	.long	sysExeption05
	.long	sysExeption06
	.long	sysExeption07
	.long	sysExeption08
	.long	sysExeption09
	.long	sysExeption0A
	.long	sysExeption0B
	.long	sysExeption0C
	.long	sysExeption0D
	.long	sysExeption0E
	.long	sysExeption0F

	.long	sysExeption10
	.long	sysExeption11
	.long	sysExeption12
	.long	sysExeption13
	.long	sysExeption14
	.long	sysExeption15
	.long	sysExeption16
	.long	sysExeption17
	.long	sysExeption18
	.long	sysExeption19
	.long	sysExeption1A
	.long	sysExeption1B
	.long	sysExeption1C
	.long	sysExeption1D
	.long	sysExeption1E
	.long	sysExeption1F

	.long	sysHwExeption20		# H/W interrupts
	.long	sysHwExeption21
	.long	sysHwExeption22
	.long	sysHwExeption23
	.long	sysHwExeption24
	.long	sysHwExeption25
	.long	sysHwExeption26
	.long	sysHwExeption27
	.long	sysHwExeption28
	.long	sysHwExeption29
	.long	sysHwExeption2A
	.long	sysHwExeption2B
	.long	sysHwExeption2C
	.long	sysHwExeption2D
	.long	sysHwExeption2E
	.long	sysHwExeption2F

	.long	sysHwExeption30
	.long	sysHwExeption31
	.long	sysHwExeption32
	.long	sysHwExeption33
	.long	sysHwExeption34
	.long	sysHwExeption35
	.long	sysHwExeption36
	.long	sysHwExeption37
	.long	sysHwExeption38
	.long	sysHwExeption39
	.long	sysHwExeption3A
	.long	sysHwExeption3B
	.long	sysHwExeption3C
	.long	sysHwExeption3D
	.long	sysHwExeption3E
	.long	sysHwExeption3F

	.long	sysSwExeption40		# S/W interrupts
	.long	sysSwExeption41
	.long	sysSwExeption42
	.long	sysSwExeption43
	.long	sysSwExeption44

	.long	sysExeption45		# IOAPIC1
	.long	sysExeption46
	.long	sysExeption47
	.long	sysExeption48
	.long	sysExeption49
	.long	sysExeption4A
	.long	sysExeption4B
	.long	sysExeption4C
	.long	sysExeption4D
	.long	sysExeption4E
	.long	sysExeption4F
	.long	sysExeption50
	.long	sysExeption51
	.long	sysExeption52
	.long	sysExeption53
	.long	sysExeption54
	.long	sysExeption55
	.long	sysExeption56
	.long	sysExeption57
	.long	sysExeption58
	.long	sysExeption59
	.long	sysExeption5A
	.long	sysExeption5B
	.long	sysExeption5C
	.long	sysExeption5D
	.long	sysExeption5E
	.long	sysExeption5F

	.long	sysExeption60
	.long	sysExeption61
	.long	sysExeption62
	.long	sysExeption63
	.long	sysExeption64
	.long	sysExeption65
	.long	sysExeption66
	.long	sysExeption67
	.long	sysExeption68
	.long	sysExeption69
	.long	sysExeption6A
	.long	sysExeption6B
	.long	sysExeption6C
	.long	sysExeption6D
	.long	sysExeption6E
	.long	sysExeption6F

	.long	sysExeption70
	.long	sysExeption71
	.long	sysExeption72
	.long	sysExeption73
	.long	sysExeption74
	.long	sysExeption75
	.long	sysExeption76
	.long	sysExeption77
	.long	sysExeption78
	.long	sysExeption79
	.long	sysExeption7A
	.long	sysExeption7B
	.long	sysExeption7C
	.long	sysExeption7D
	.long	sysExeption7E
	.long	sysExeption7F

	.long	sysExeption80
	.long	sysExeption81
	.long	sysExeption82
	.long	sysExeption83
	.long	sysExeption84
	.long	sysExeption85
	.long	sysExeption86
	.long	sysExeption87
	.long	sysExeption88
	.long	sysExeption89
	.long	sysExeption8A
	.long	sysExeption8B
	.long	sysExeption8C
	.long	sysExeption8D
	.long	sysExeption8E
	.long	sysExeption8F

    .long	sysExeption90
	.long	sysExeption91
	.long	sysExeption92
	.long	sysExeption93
	.long	sysExeption94
	.long	sysExeption95
	.long	sysExeption96
	.long	sysExeption97
	.long	sysExeption98
	.long	sysExeption99
	.long	sysExeption9A
	.long	sysExeption9B
	.long	sysExeption9C
	.long	sysExeption9D
	.long	sysExeption9E
	.long	sysExeption9F

	.long	sysExeptionA0
	.long	sysExeptionA1
	.long	sysExeptionA2
	.long	sysExeptionA3
	.long	sysExeptionA4
	.long	sysExeptionA5
	.long	sysExeptionA6
	.long	sysExeptionA7
	.long	sysExeptionA8
	.long	sysExeptionA9
	.long	sysExeptionAA
	.long	sysExeptionAB
	.long	sysExeptionAC
	.long	sysExeptionAD
	.long	sysExeptionAE
	.long	sysExeptionAF
idt_end:



/*****************************************************************************
 * sysInstallIdt: install our IDT
 *
 * ENTRY: none
 *
 * RETURNS: none
 *
 * MODIFIED: none
 *
 */

	.globl _sysInstallIdt
_sysInstallIdt:
	pushal

	lea	idt, %esi			# GDT template in [ESI]
	movl	$IDT_BASE, %edi			# base address of GDT in [EDI]

	movl	$(idt_end - idt), %ecx		# GDT template length in DWORDs
	shrl	$2, %ecx			#

idtLoop:
	lodsl					# get a handler address
	movl	%eax, %ebx			# save in [EBX]

	stosw					# write Offset[15:0] to IDT

	movw	%cs, %ax			# write CS to IDT
	stosw					#

	movw	$0x8E00, %ax			# write "Interrupt Gate" type to IDT
	stosw					#

	movl	%ebx, %eax			# write offset[31:16] to IDT
	shrl	$16, %eax			#
	stosw					#
	
	loop	idtLoop				# repeat for all handlers

	popal
	ret


/*****************************************************************************
 * _sysInstallUserHandler: install a user routine as an interrupt service
 *
 * RETURNS: 0 if slot is valid, -1 if not
 *
 * PROTOTYPE:
 *
 *	int sysInstallUserHandler (UINT8 bVector,  void (*pf)())
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] vVector - IDT vector number
 * 	[EBP+0C] dAddress - physical handler address
 */
	.globl _sysInstallUserHandler
_sysInstallUserHandler:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%ecx

	# Check this is a VALID  vector

	movl	0x08(%ebp), %ecx		# get vector number in ECX
	movzbl	%cl, %ecx			#

	cmpl	$IDT_SLOTS, %ecx		# < highest vector supported?
	jae	IdtWrite_Fail			# no, then error

	# Get handler address

	movl	0x0C(%ebp), %eax		# low bytes

	# Write the slot data

	shll	$2, %ecx			# convert vector# to DWORD offset

	lea	userExceptionHandler, %ebx	# get user handler address
	addl	%ecx, %ebx			# offset to slot

	movl	%eax, (%ebx)			# write out slot data

	xorl	%eax, %eax			# return 0 to indicate success
	jmp	IdtWrite_Done			#


IdtWrite_Fail:
	xorl	%eax, %eax			# return -1 for invalid slot number
	decl	%eax				#

IdtWrite_Done:
	popl	%ecx
	popl	%ebx

	leave
	ret


/*****************************************************************************
 * _sysDeleteUserHandlers: clean-up user handlers after tets
 *
 * RETURNS: none
 *
 * PROTOTYPE:
 *
 *	void sysDeleteUserHandlers (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysDeleteUserHandlers
_sysDeleteUserHandlers:

	pushl	%ebp
	movl	%esp, %ebp

	pushal

	# Write the slot data

	lea	userExceptionHandler, %edi	# get user handler table address
	movl	$IDT_SLOTS, %ecx		# number of slots to clear

	xorl	%eax, %eax			# clear slots 
	rep					#
	stosl					#

	popal

	leave
	ret


/*****************************************************************************
 * sysExeptionXX: handlers for exceptions 0x00 - 0x1F
 *
 * ENTRY: none
 *
 * RETURNS: does not return from here
 *
 * MODIFIED: EAX
 *
 */

sysExeption00:
	pushal
	movb	$(0x00), %al
	jmp	_sysGenericCpu

sysExeption01:
	pushal
	movb	$(0x01), %al
	jmp	_sysGenericCpu

sysExeption02:
	pushal
	movb	$(0x02), %al
	jmp	_sysGenericCpu

sysExeption03:
	pushal
	movb	$(0x03), %al
	jmp	_sysGenericCpu

sysExeption04:
	pushal
	movb	$(0x04), %al
	jmp	_sysGenericCpu

sysExeption05:
	pushal
	movb	$(0x05), %al
	jmp	_sysGenericCpu

sysExeption06:
	pushal
	movb	$(0x06), %al
	jmp	_sysGenericCpu

sysExeption07:
	pushal
	movb	$(0x07), %al
	jmp	_sysGenericCpu

sysExeption08:
	pushal
	movb	$(0x08), %al
	jmp	_sysGenericCpu

sysExeption09:
	pushal
	movb	$(0x09), %al
	jmp	_sysGenericCpu

sysExeption0A:
	pushal
	movb	$(0x0A), %al
	jmp	_sysGenericCpu

sysExeption0B:
	pushal
	movb	$(0x0B), %al
	jmp	_sysGenericCpu

sysExeption0C:
	pushal
	movb	$(0x0C), %al
	jmp	_sysGenericCpu

sysExeption0D:
	pushal
	movb	$(0x0D), %al
	jmp	_sysGenericCpu

sysExeption0E:
	pushal
	movb	$(0x0E), %al
	jmp	_sysGenericCpu

sysExeption0F:
	pushal
	movb	$(0x0F), %al
	jmp	_sysGenericCpu


sysExeption10:
	pushal
	movb	$(0x10), %al
	jmp	_sysGenericCpu

sysExeption11:
	pushal
	movb	$(0x11), %al
	jmp	_sysGenericCpu

sysExeption12:
	pushal
	movb	$(0x12), %al
	jmp	_sysGenericCpu

sysExeption13:
	pushal
	movb	$(0x13), %al
	jmp	_sysGenericCpu

sysExeption14:
	pushal
	movb	$(0x14), %al
	jmp	_sysGenericCpu

sysExeption15:
	pushal
	movb	$(0x15), %al
	jmp	_sysGenericCpu

sysExeption16:
	pushal
	movb	$(0x16), %al
	jmp	_sysGenericCpu

sysExeption17:
	pushal
	movb	$(0x17), %al
	jmp	_sysGenericCpu

sysExeption18:
	pushal
	movb	$(0x18), %al
	jmp	_sysGenericCpu

sysExeption19:
	pushal
	movb	$(0x19), %al
	jmp	_sysGenericCpu

sysExeption1A:
	pushal
	movb	$(0x1A), %al
	jmp	_sysGenericCpu

sysExeption1B:
	pushal
	movb	$(0x1B), %al
	jmp	_sysGenericCpu

sysExeption1C:
	pushal
	movb	$(0x1C), %al
	jmp	_sysGenericCpu

sysExeption1D:
	pushal
	movb	$(0x1D), %al
	jmp	_sysGenericCpu

sysExeption1E:
	pushal
	movb	$(0x1E), %al
	jmp	_sysGenericCpu

sysExeption1F:
	pushal
	movb	$(0x1F), %al
	jmp	_sysGenericCpu


/*****************************************************************************
 * sysHwExeptionXX: handlers for exceptions 0x20 - 0x3F
 *
 * ENTRY: none
 *
 * RETURNS: does not return from here
 *
 * MODIFIED: %eax
 *
 */

sysHwExeption20:
	pushal
	movb	$(0x20), %al
	jmp	_sysGeneric

sysHwExeption21:
	pushal
	movb	$(0x21), %al
	jmp	_sysGeneric

sysHwExeption22:
	pushal
	movb	$(0x22), %al
	jmp	_sysGeneric

sysHwExeption23:
	pushal
	movb	$(0x23), %al
	jmp	_sysGeneric

sysHwExeption24:
	pushal
	movb	$(0x24), %al
	jmp	_sysGeneric

sysHwExeption25:
	pushal
	movb	$(0x25), %al
	jmp	_sysGeneric

sysHwExeption26:
	pushal
	movb	$(0x26), %al
	jmp	_sysGeneric

sysHwExeption27:
	pushal
	movb	$(0x27), %al
	jmp	_sysGeneric

sysHwExeption28:
	pushal
	movb	$(0x28), %al
	jmp	_sysGeneric

sysHwExeption29:
	pushal
	movb	$(0x29), %al
	jmp	_sysGeneric

sysHwExeption2A:
	pushal
	movb	$(0x2A), %al
	jmp	_sysGeneric

sysHwExeption2B:
	pushal
	movb	$(0x2B), %al
	jmp	_sysGeneric

sysHwExeption2C:
	pushal
	movb	$(0x2C), %al
	jmp	_sysGeneric

sysHwExeption2D:
	pushal
	movb	$(0x2D), %al
	jmp	_sysGeneric

sysHwExeption2E:
	pushal
	movb	$(0x2E), %al
	jmp	_sysGeneric

sysHwExeption2F:
	pushal
	movb	$(0x2F), %al
	jmp	_sysGeneric


sysHwExeption30:
	pushal
	movb	$(0x30), %al
	jmp	_sysGeneric

sysHwExeption31:
	pushal
	movb	$(0x31), %al
	jmp	_sysGeneric

sysHwExeption32:
	pushal
	movb	$(0x32), %al
	jmp	_sysGeneric

sysHwExeption33:
	pushal
	movb	$(0x33), %al
	jmp	_sysGeneric

sysHwExeption34:
	pushal
	movb	$(0x34), %al
	jmp	_sysGeneric

sysHwExeption35:
	pushal
	movb	$(0x35), %al
	jmp	_sysGeneric

sysHwExeption36:
	pushal
	movb	$(0x36), %al
	jmp	_sysGeneric

sysHwExeption37:
	pushal
	movb	$(0x37), %al
	jmp	_sysGeneric

sysHwExeption38:
	pushal
	movb	$(0x38), %al
	jmp	_sysGeneric

sysHwExeption39:
	pushal
	movb	$(0x39), %al
	jmp	_sysGeneric

sysHwExeption3A:
	pushal
	movb	$(0x3A), %al
	jmp	_sysGeneric

sysHwExeption3B:
	pushal
	movb	$(0x3B), %al
	jmp	_sysGeneric

sysHwExeption3C:
	pushal
	movb	$(0x3C), %al
	jmp	_sysGeneric

sysHwExeption3D:
	pushal
	movb	$(0x3D), %al
	jmp	_sysGeneric

sysHwExeption3E:
	pushal
	movb	$(0x3E), %al
	jmp	_sysGeneric

sysHwExeption3F:
	pushal
	movb	$(0x3F), %al
	jmp	_sysGeneric


/*****************************************************************************
 * sysSwExeptionXX: handlers for exceptions 0x40 - N
 *
 * ENTRY: none
 *
 * RETURNS: does not return from here
 *
 * MODIFIED: %eax
 *
 */

sysSwExeption40:				/* Used by SMP BIT */
	pushal
	movb	$(0x40), %al
	jmp	_sysGeneric

sysSwExeption41:				/* RTOS timer tick */
	pushal
	movb	$(0x41), %al
	jmp	_sysGeneric

sysSwExeption42:				/* RTOS task yield */
	pushal
	movb	$(0x42), %al
	jmp	_sysGeneric

sysSwExeption43:				/* RTOS start AP sheduling */
	pushal
	movb	$(0x43), %al
	jmp	_sysGeneric

sysSwExeption44:				/* RTOS AP tick */
	pushal
	movb	$(0x44), %al
	jmp	_sysGeneric



/***********************/
/*		IOAPIC1		   */

sysExeption45:
	pushal
	movb	$(0x45), %al
	jmp	_sysGenericCpu

sysExeption46:
	pushal
	movb	$(0x46), %al
	jmp	_sysGenericCpu

sysExeption47:
	pushal
	movb	$(0x47), %al
	jmp	_sysGenericCpu

sysExeption48:
	pushal
	movb	$(0x48), %al
	jmp	_sysGenericCpu

sysExeption49:
	pushal
	movb	$(0x49), %al
	jmp	_sysGenericCpu

sysExeption4A:
	pushal
	movb	$(0x4A), %al
	jmp	_sysGenericCpu

sysExeption4B:
	pushal
	movb	$(0x4B), %al
	jmp	_sysGenericCpu

sysExeption4C:
	pushal
	movb	$(0x4C), %al
	jmp	_sysGenericCpu

sysExeption4D:
	pushal
	movb	$(0x4D), %al
	jmp	_sysGenericCpu

sysExeption4E:
	pushal
	movb	$(0x4E), %al
	jmp	_sysGenericCpu

sysExeption4F:
	pushal
	movb	$(0x4F), %al
	jmp	_sysGenericCpu

sysExeption50:
	pushal
	movb	$(0x50), %al
	jmp	_sysGenericCpu

sysExeption51:
	pushal
	movb	$(0x51), %al
	jmp	_sysGenericCpu

sysExeption52:
	pushal
	movb	$(0x52), %al
	jmp	_sysGenericCpu

sysExeption53:
	pushal
	movb	$(0x53), %al
	jmp	_sysGenericCpu


sysExeption54:
	pushal
	movb	$(0x54), %al
	jmp	_sysGenericCpu

sysExeption55:
	pushal
	movb	$(0x55), %al
	jmp	_sysGenericCpu

sysExeption56:
	pushal
	movb	$(0x56), %al
	jmp	_sysGenericCpu

sysExeption57:
	pushal
	movb	$(0x57), %al
	jmp	_sysGenericCpu

sysExeption58:
	pushal
	movb	$(0x58), %al
	jmp	_sysGenericCpu

sysExeption59:
	pushal
	movb	$(0x59), %al
	jmp	_sysGenericCpu

sysExeption5A:
	pushal
	movb	$(0x5A), %al
	jmp	_sysGenericCpu

sysExeption5B:
	pushal
	movb	$(0x5B), %al
	jmp	_sysGenericCpu

sysExeption5C:
	pushal
	movb	$(0x5C), %al
	jmp	_sysGenericCpu

sysExeption5D:
	pushal
	movb	$(0x5D), %al
	jmp	_sysGenericCpu

sysExeption5E:
	pushal
	movb	$(0x5E), %al
	jmp	_sysGenericCpu

sysExeption5F:
	pushal
	movb	$(0x5F), %al
	jmp	_sysGenericCpu

sysExeption60:
	pushal
	movb	$(0x60), %al
	jmp	_sysGenericCpu

sysExeption61:
	pushal
	movb	$(0x61), %al
	jmp	_sysGenericCpu

sysExeption62:
	pushal
	movb	$(0x62), %al
	jmp	_sysGenericCpu

sysExeption63:
	pushal
	movb	$(0x63), %al
	jmp	_sysGenericCpu


sysExeption64:
	pushal
	movb	$(0x64), %al
	jmp	_sysGenericCpu

sysExeption65:
	pushal
	movb	$(0x65), %al
	jmp	_sysGenericCpu

sysExeption66:
	pushal
	movb	$(0x66), %al
	jmp	_sysGenericCpu

sysExeption67:
	pushal
	movb	$(0x67), %al
	jmp	_sysGenericCpu

sysExeption68:
	pushal
	movb	$(0x68), %al
	jmp	_sysGenericCpu

sysExeption69:
	pushal
	movb	$(0x69), %al
	jmp	_sysGenericCpu

sysExeption6A:
	pushal
	movb	$(0x6A), %al
	jmp	_sysGenericCpu

sysExeption6B:
	pushal
	movb	$(0x6B), %al
	jmp	_sysGenericCpu

sysExeption6C:
	pushal
	movb	$(0x6C), %al
	jmp	_sysGenericCpu

sysExeption6D:
	pushal
	movb	$(0x6D), %al
	jmp	_sysGenericCpu

sysExeption6E:
	pushal
	movb	$(0x6E), %al
	jmp	_sysGenericCpu

sysExeption6F:
	pushal
	movb	$(0x6F), %al
	jmp	_sysGenericCpu

sysExeption70:
	pushal
	movb	$(0x70), %al
	jmp	_sysGenericCpu

sysExeption71:
	pushal
	movb	$(0x71), %al
	jmp	_sysGenericCpu

sysExeption72:
	pushal
	movb	$(0x72), %al
	jmp	_sysGenericCpu

sysExeption73:
	pushal
	movb	$(0x73), %al
	jmp	_sysGenericCpu


sysExeption74:
	pushal
	movb	$(0x74), %al
	jmp	_sysGenericCpu

sysExeption75:
	pushal
	movb	$(0x75), %al
	jmp	_sysGenericCpu

sysExeption76:
	pushal
	movb	$(0x76), %al
	jmp	_sysGenericCpu

sysExeption77:
	pushal
	movb	$(0x77), %al
	jmp	_sysGenericCpu

sysExeption78:
	pushal
	movb	$(0x78), %al
	jmp	_sysGenericCpu

sysExeption79:
	pushal
	movb	$(0x79), %al
	jmp	_sysGenericCpu

sysExeption7A:
	pushal
	movb	$(0x7A), %al
	jmp	_sysGenericCpu

sysExeption7B:
	pushal
	movb	$(0x7B), %al
	jmp	_sysGenericCpu

sysExeption7C:
	pushal
	movb	$(0x7C), %al
	jmp	_sysGenericCpu

sysExeption7D:
	pushal
	movb	$(0x7D), %al
	jmp	_sysGenericCpu

sysExeption7E:
	pushal
	movb	$(0x7E), %al
	jmp	_sysGenericCpu

sysExeption7F:
	pushal
	movb	$(0x7F), %al
	jmp	_sysGenericCpu

sysExeption80:
	pushal
	movb	$(0x80), %al
	jmp	_sysGenericCpu

sysExeption81:
	pushal
	movb	$(0x81), %al
	jmp	_sysGenericCpu

sysExeption82:
	pushal
	movb	$(0x82), %al
	jmp	_sysGenericCpu

sysExeption83:
	pushal
	movb	$(0x83), %al
	jmp	_sysGenericCpu


sysExeption84:
	pushal
	movb	$(0x84), %al
	jmp	_sysGenericCpu

sysExeption85:
	pushal
	movb	$(0x85), %al
	jmp	_sysGenericCpu

sysExeption86:
	pushal
	movb	$(0x86), %al
	jmp	_sysGenericCpu

sysExeption87:
	pushal
	movb	$(0x87), %al
	jmp	_sysGenericCpu

sysExeption88:
	pushal
	movb	$(0x88), %al
	jmp	_sysGenericCpu

sysExeption89:
	pushal
	movb	$(0x89), %al
	jmp	_sysGenericCpu

sysExeption8A:
	pushal
	movb	$(0x8A), %al
	jmp	_sysGenericCpu

sysExeption8B:
	pushal
	movb	$(0x8B), %al
	jmp	_sysGenericCpu

sysExeption8C:
	pushal
	movb	$(0x8C), %al
	jmp	_sysGenericCpu

sysExeption8D:
	pushal
	movb	$(0x8D), %al
	jmp	_sysGenericCpu

sysExeption8E:
	pushal
	movb	$(0x8E), %al
	jmp	_sysGenericCpu

sysExeption8F:
	pushal
	movb	$(0x8F), %al
	jmp	_sysGenericCpu

sysExeption90:
	pushal
	movb	$(0x90), %al
	jmp	_sysGenericCpu

sysExeption91:
	pushal
	movb	$(0x91), %al
	jmp	_sysGenericCpu

sysExeption92:
	pushal
	movb	$(0x92), %al
	jmp	_sysGenericCpu

sysExeption93:
	pushal
	movb	$(0x93), %al
	jmp	_sysGenericCpu


sysExeption94:
	pushal
	movb	$(0x94), %al
	jmp	_sysGenericCpu

sysExeption95:
	pushal
	movb	$(0x95), %al
	jmp	_sysGenericCpu

sysExeption96:
	pushal
	movb	$(0x96), %al
	jmp	_sysGenericCpu

sysExeption97:
	pushal
	movb	$(0x97), %al
	jmp	_sysGenericCpu

sysExeption98:
	pushal
	movb	$(0x98), %al
	jmp	_sysGenericCpu

sysExeption99:
	pushal
	movb	$(0x99), %al
	jmp	_sysGenericCpu

sysExeption9A:
	pushal
	movb	$(0x9A), %al
	jmp	_sysGenericCpu

sysExeption9B:
	pushal
	movb	$(0x9B), %al
	jmp	_sysGenericCpu

sysExeption9C:
	pushal
	movb	$(0x9C), %al
	jmp	_sysGenericCpu

sysExeption9D:
	pushal
	movb	$(0x9D), %al
	jmp	_sysGenericCpu

sysExeption9E:
	pushal
	movb	$(0x9E), %al
	jmp	_sysGenericCpu

sysExeption9F:
	pushal
	movb	$(0x9F), %al
	jmp	_sysGenericCpu

sysExeptionA0:
	pushal
	movb	$(0xA0), %al
	jmp	_sysGenericCpu

sysExeptionA1:
	pushal
	movb	$(0xA1), %al
	jmp	_sysGenericCpu

sysExeptionA2:
	pushal
	movb	$(0xA2), %al
	jmp	_sysGenericCpu

sysExeptionA3:
	pushal
	movb	$(0xA3), %al
	jmp	_sysGenericCpu


sysExeptionA4:
	pushal
	movb	$(0xA4), %al
	jmp	_sysGenericCpu

sysExeptionA5:
	pushal
	movb	$(0xA5), %al
	jmp	_sysGenericCpu

sysExeptionA6:
	pushal
	movb	$(0xA6), %al
	jmp	_sysGenericCpu

sysExeptionA7:
	pushal
	movb	$(0xA7), %al
	jmp	_sysGenericCpu

sysExeptionA8:
	pushal
	movb	$(0xA8), %al
	jmp	_sysGenericCpu

sysExeptionA9:
	pushal
	movb	$(0xA9), %al
	jmp	_sysGenericCpu

sysExeptionAA:
	pushal
	movb	$(0xAA), %al
	jmp	_sysGenericCpu

sysExeptionAB:
	pushal
	movb	$(0xAB), %al
	jmp	_sysGenericCpu

sysExeptionAC:
	pushal
	movb	$(0xAC), %al
	jmp	_sysGenericCpu

sysExeptionAD:
	pushal
	movb	$(0xAD), %al
	jmp	_sysGenericCpu

sysExeptionAE:
	pushal
	movb	$(0xAE), %al
	jmp	_sysGenericCpu

sysExeptionAF:
	pushal
	movb	$(0xAF), %al
	jmp	_sysGenericCpu

/*****************************************************************************
 * sysGenericCpu: generic handler for exceptions 0x00 - 0x1F
 *
 * ENTRY: %eax = IDT slot number
 *
 * RETURNS: none
 *
 * MODIFIED: none
 *
 */
	.globl _sysGenericCpu
_sysGenericCpu:

	pushw	%ds
	pushw	%es
	pushw	%fs
	pushw	%gs

	xchg	%al, %ah

	/* Record exception error in CMOS: 
	 *
	 * E__EXCEPTION = 0x01001000 + vector number
	 */

	movb	$0x65, %al			# cmosErrorCodeHH
	outb	%al, $0x72
	movb	$0x01, %al
	outb	%al, $0x73

	movb	$0x64, %al			# cmosErrorCodeHL
	outb	%al, $0x72
	movb	$0x00, %al
	outb	%al, $0x73

	movb	$0x63, %al			# cmosErrorCodeLH
	outb	%al, $0x72
	movb	$0x10, %al
	outb	%al, $0x73

	movb	$0x62, %al			# cmosErrorCodeLL
	outb	%al, $0x72
	xchg	%al, %ah
	outb	%al, $0x73

	/* See if a user handler exists */

	movw	$DS_SEL, %bx
	movw	%bx, %ds

	movzbl	%al, %eax			# get DWORD slot number from vector
	pushl   %eax				# save vector
	shll	$2, %eax			#

	lea	userExceptionHandler, %ebx	# get offset into User Exception Handler table
	addl	%eax, %ebx			#

	movl	%ds:(%ebx), %eax		# get the user handler address

	orl	%eax, %eax			# NULL = no user handler
	jnz	callUserHandler			#

	/* No User Handler available, so set 'Boot to BIOS' and reset board */

	movb	$0x35, %al			# set Boot-to-BIOS flag
	outb	%al, $0x70			#
	inb	$0x71, %al			#
	orb	$0x01, %al			#
	outb	%al, $0x71			#

	movb	$0x06, %al			# issue a PCI reset
	movw	$0x0CF9, %dx			#
	outb	%al, %dx			#


	/* Call user handler, which must handle the exception, then return */

callUserHandler:

	call	*%eax				# call handler
	addl    $4, %esp            # clean up pushed argument (vector) */
	
	call	_sysApicSendEoi			# issue EOI to APIC

	popw	%gs
	popw	%fs
	popw	%es
	popw	%ds

	popal
	iret




/*****************************************************************************
 * sysGeneric: generic handler for hardware exceptions 0x20 - 0x3F
 *                             and software exceptions 0x40 - N
 *
 * ENTRY: %al = IDT vector number
 *
 * RETURNS: none
 *
 * MODIFIED: none
 *
 */
	.globl _sysGeneric
_sysGeneric:
	pushl	%ds
	pushl	%es
	pushl	%fs
	pushl	%gs

	movw	$DS_SEL, %bx
	movw	%bx, %ds

	movzbl	%al, %eax			# get DWORD slot number from vector
	shll	$2, %eax			#

	lea	userExceptionHandler, %ebx	# get offset into User Exception Handler table
	addl	%eax, %ebx			#

	/* Call user handler, if any */

	movl	%ds:(%ebx), %eax		# get the user handler address

	orl	%eax, %eax			# NULL = no user handler
	jz	noHwHandler			#

	call	*%eax				# call handler

noHwHandler:

.ifdef PCBRD
	pushl 	%edx
	movl	$0x20, %edx
	movl	$0x20, %eax
	outb	%al, %dx		# issue EOI to PIC
	popl 	%edx
.else
	call	_sysApicSendEoi	# issue EOI to APIC
.endif	

	popl	%gs
	popl	%fs
	popl	%es
	popl	%ds

	popal
	iret



.data

	.align 16

userExceptionHandler:
	.long	0		# 0x00 - 0x0F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x10 - 0x1F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x20 - 0x2F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x30 - 0x3F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x40 - 0x4F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x50 - 0x5F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x60 - 0x6F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x70 - 0x7F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x80 - 0x8F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0x90 - 0x9F
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0xA0 - 0xAF
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0xB0 - 0xBF
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0xC0 - 0xCF
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0xD0 - 0xDF
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0xE0 - 0xEF
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

	.long	0		# 0xF0 - 0xFF
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0




.bss
