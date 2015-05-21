
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

/* gdt.S - GDT manipulation primatives
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/gdt.S,v 1.1 2013-09-04 07:41:28 chippisley Exp $
 *
 * $Log: gdt.S,v $
 * Revision 1.1  2013-09-04 07:41:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2010/11/04 17:35:58  hchalla
 * Alignment of the code.
 *
 * Revision 1.4  2010/09/15 12:21:55  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.3  2009/05/20 08:55:58  swilson
 * Correction to 'slot clear' function, as was multiplying to get number of DWORDs and
 *  overwriting the IDT!
 *
 * Revision 1.2  2008/05/29 10:09:47  swilson
 * Add local implementation of HAL memory functions. Modify loader to configure the runtime GDT and IDT.
 *
 * Revision 1.1  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 */


.include "asm/i386.h"


.text

/*****************************************************************************
 * sysGdtWriteSlot: configure a user GDT slot
 *
 * RETURNS: 0 if slot is valid, -1 if not
 *
 * PROTOTYPE:
 *
 *	int sysGdtWriteSlot (UINT16 dSlot, UINT64 qDescriptor)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] dSlot - GDT selector
 * 	[EBP+0C] qDescriptor - encoded descriptor
 */
	.globl _sysGdtWriteSlot
_sysGdtWriteSlot:

	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%ecx

	# Check this is a VALID User Slot
	movl	0x08(%ebp), %ecx		# get selector
	cmpw	$RES_SEL, %cx			# must be >= RES_SEL
	jb		GdtWrite_Fail			#

	cmpw	$END_SEL, %cx			# and <= END_SEL
	ja		GdtWrite_Fail			#

	# Get encoded GDT slot data into [EDX:EAX]
	movl	0x0C(%ebp), %eax		# low bytes
	movl	0x10(%ebp), %edx		# high bytes

	# Write the slot data
	movl	$GDT_BASE, %ebx			# get GDT address
	addl	%ecx, %ebx				# offset to slot

	movl	%eax, (%ebx)			# write out slot data
	movl	%edx, 0x04(%ebx)		#

	xorl	%eax, %eax				# return  0 to indicate success
	jmp		GdtWrite_Done			#

GdtWrite_Fail:
	xorl	%eax, %eax				# return -1 for invalid slot number
	decl	%eax					#

GdtWrite_Done:
	popl	%ecx
	popl	%ebx

	leave
	ret


/*****************************************************************************
 * sysGdtDeleteAllSlots: delete all user GDT slots 
 *
 * RETURNS: selector for slot, NULL on error
 *
 * PROTOTYPE:
 *
 *	void sysGdtDeleteAllSlots (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysGdtDeleteAllSlots
_sysGdtDeleteAllSlots:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi
	pushl	%ecx

	movl	$USER_SEL, %edi			# start with first user slot

	movl	$END_SEL, %ecx			# difference = number of bytes
	subl	%edi, %ecx				#
	shrl	$2, %ecx				# /4 = number of DWORDs

	addl	$GDT_BASE, %edi			# get address of first slot

	xorl	%eax, %eax				# set all user slots to zero
	rep								#
	stosl							#

	popl	%ecx
	popl	%edi

	leave
	ret


/*****************************************************************************
 * sysGdtFindFreeSlot: locate a free slot in 
 *
 * RETURNS: selector for slot, NULL on error
 *
 * PROTOTYPE:
 *
 *	UINT16 sysGdtFindFreeSlot (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysGdtFindFreeSlot
_sysGdtFindFreeSlot:

	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%ecx

	movl	$USER_SEL, %ebx			# start with first user slot
	movl	$END_SEL, %ecx			# get number of slots to search
	subl	%ebx, %ecx				#
	addl	$GDT_BASE, %ebx			# get address of first slot
	xorl	%eax, %eax				# [EAX] = 0

GdtFind_Loop:
	cmpl	%eax, (%ebx)			# check for LSB slot data = 0
	jne		GdtFind_Next			# no, go try next
	cmpl	%eax, 0x04(%ebx)		# check for MSB slot data = 0
	je		GdtFind_Found			# yes, found a slot

GdtFind_Next:
	addl	$8, %ebx				# move to next slot
	loop	GdtFind_Loop			# repeat until all slots searched
	xorl	%eax, %eax				# return NULL if slot not found
	jmp		GdtFind_Done			#

GdtFind_Found:
	subl	$GDT_BASE, %ebx			# remove GDT start offset
	movzwl	%bx, %eax				# put selector in [EAX] for return

GdtFind_Done:
	popl	%ecx
	popl	%ebx

	leave
	ret


/*****************************************************************************
 * _sysGdtFindFreeSlotRes: locate a free slot in
 *
 * RETURNS: selector for slot, NULL on error
 *
 * PROTOTYPE:
 *
 *	UINT16 _sysGdtFindFreeSlotRes (void)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 */
	.globl _sysGdtFindFreeSlotRes
_sysGdtFindFreeSlotRes:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx
	pushl	%ecx

	movl	$RES_SEL, %ebx			# start with first user slot
	movl	$END_RES_SEL, %ecx		# get number of slots to search
	subl	%ebx, %ecx				#

	addl	$GDT_BASE, %ebx			# get address of first slot

	xorl	%eax, %eax				# [EAX] = 0
GdtFind_Loop_Res:
	cmpl	%eax, (%ebx)			# check for LSB slot data = 0
	jne		GdtFind_Next_Res		# no, go try next

	cmpl	%eax, 0x04(%ebx)		# check for MSB slot data = 0
	je		GdtFind_Found_Res		# yes, found a slot

GdtFind_Next_Res:
	addl	$8, %ebx				# move to next slot
	loop	GdtFind_Loop_Res		# repeat until all slots searched

	xorl	%eax, %eax				# return NULL if slot not found
	jmp		GdtFind_Done_Res		#


GdtFind_Found_Res:
	subl	$GDT_BASE, %ebx			# remove GDT start offset
	movzwl	%bx, %eax				# put selector in [EAX] for return

GdtFind_Done_Res:
	popl	%ecx
	popl	%ebx

	leave
	ret

.data

.bss

