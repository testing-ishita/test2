
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

/* mem.S - optimized implementation of <string.h> 'C' library functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/mem.S,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: mem.S,v $
 * Revision 1.1  2013-09-04 07:17:58  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2008/05/21 13:27:54  swilson
 * Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
 *
 * Revision 1.4  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.3  2008/05/12 15:58:34  swilson
 * Various bug fixes following testing.
 *
 * Revision 1.2  2008/04/25 15:24:34  swilson
 * Completed string.h implementation.
 *
 * Revision 1.1  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 */


.text


/*****************************************************************************
 * memchr: search 'n' bytes of memory at 's' for character 'c'
 *
 * RETURNS: pointer to first occurance of 'c', else NULL
 *
 *          [DX] also set to [ES], or NULL on error
 *
 * PROTOTYPE:
 *
 *	void*	memchr (const void* s, int c, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s : source pointer
 * 	[EBP+0C] c : compare byte
 * 	[EBP+10] n : maximum number of bytes to search
 */
	.globl _memchr
_memchr:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi				#
	pushl	%ecx				#

	cld					# string ops increment

	movl	0x08(%ebp), %edi		# load source pointer
	movl	0x0C(%ebp), %eax		# get search character
	movl	0x10(%ebp), %ecx		# load count

	jcxz	memchr_fail			# if nothing to search, fail

memchr_scan:
	repne					# search for char
	scasb					#
	jne	memchr_fail			# fail if not found

memchr_pass:
	movw	%es, %dx			# put [ES] selector in [EDX]
	movzwl	%dx, %edx			# for return

	decl	%edi				# adjust pointer to found char
	movl	%edi, %eax			#
	jmp	memchr_exit			#

memchr_fail:
	xorl	%edx, %edx			# on fail return sel:ofs = 0:0
	xorl	%eax, %eax			#

memchr_exit:
	popl	%ecx				#
	popl	%edi				#

	leave
	ret


/*****************************************************************************
 * memcmp: compare 'n' bytes of memory at 's1' to memory at 's2'
 *
 * RETURNS:  0 if 's1' and 's2' match,
 *	   < 0 if 's1' < 's2'
 *	   > 0 if 's1' > 's2'
 *
 * PROTOTYPE:
 *
 *	int memcmp (const void* s1, const void* s2, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s1 : source pointer #1
 * 	[EBP+0C] s2 : source pointer #2
 * 	[EBP+10] n  : number of bytes to compare
 */
	.globl _memcmp
_memcmp:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi				#
	pushl	%edi				#
	pushl	%ecx				#

	cld					# string ops increment

	movl	0x08(%ebp), %esi		# load source1 pointer
	movl	0x0C(%ebp), %edi		# load source2 pointer
	movl	0x10(%ebp), %ecx		# load count

	xorb	%al, %al			# if count = zero, I suppose they match
	jcxz	memcmp_done			#

memcmp_compare:
	lodsb		 			# get char from s1
	subb	%es:(%edi), %al			# compare to s2
	jne	memcmp_done			# if no match, exit

	incl	%edi				# increment s2 counter and loop
	loop	memcmp_compare			#

memcmp_done:
	movsbl	%al, %eax			# convert result to (int)

	popl	%ecx				#
	popl	%edi				#
	popl	%esi				#

	leave
	ret


/*****************************************************************************
 * memicmp: compare 'n' bytes of memory at 's1' to memory at 's2'
 *
 * Bytes are treated as unsigned chars and testing is case insensitive
 *
 * RETURNS:  0 if 's1' and 's2' match,
 *	   < 0 if 's1' < 's2'
 *	   > 0 if 's1' > 's2'
 *
 * PROTOTYPE:
 *
 *	int memcmp (const void* s1, const void* s2, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s1 : source pointer #1
 * 	[EBP+0C] s2 : source pointer #2
 * 	[EBP+10] n  : number of bytes to compare
 */
	.globl _memicmp
_memicmp:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi				#
	pushl	%edi				#
	pushl	%ecx				#

	cld					# string ops increment

	movl	0x08(%ebp), %esi		# load source1 pointer
	movl	0x0C(%ebp), %edi		# load source2 pointer
	movl	0x10(%ebp), %ecx		# load count

	xorb	%al, %al			# if count = zero, I suppose they match
	jcxz	memicmp_done			#

memicmp_compare:
	lodsb					# get char from s1

	cmpb	$'Z', %al			# check 'Z' (split favours > 'Z')
	jg	memicmp_done_s1			#
	cmpb	$'A', %al			#
	jl	memicmp_done_s1			#
	orb	$0x20, %al			# make lowercase for compare
memicmp_done_s1:

	movb	%es:(%edi), %ah			# get char from s2

	cmpb	$'Z', %ah			# check 'Z' (split favours > 'Z')
	jg	memicmp_done_s2			#
	cmpb	$'A', %ah			#
	jl	memicmp_done_s2			#
	orb	$0x20, %ah			# make lowercase for compare
memicmp_done_s2:

	subb	%ah, %al			# do the compare
	jne	memicmp_done			# if no match, exit

	incl	%edi				# increment s2 counter and loop
	loop	memicmp_compare			#

memicmp_done:
	movsbl	%al, %eax			# convert result to (int)

	popl	%ecx				#
	popl	%edi				#
	popl	%esi				#

	leave
	ret


/*****************************************************************************
 * memcpy: copy 'n' bytes of memory from 's' to 'd'
 *
 * 's' and 'd' must not overlap within a 32-bit word 
 *
 * RETURNS: 'd'
 *
 *          [DX] also set to [ES]
 *
 * PROTOTYPE:
 *
 *	void*	memcpy (void* d, const void* s, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : source pointer
 * 	[EBP+0C] s : destination pointer
 * 	[EBP+10] n : number of bytes to copy
 */
 
/******************************************************************************
 *  
 * Replaced with memcpy() function in string.c using bcopy() due to issues 
 * with source/destination orientation and overlaps.  
	.globl _memcpy
_memcpy:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi				#
	pushl	%edi				#
	pushl	%ecx				#

	cld					# string ops increment

	movl	0x08(%ebp), %edi		# load destination pointer
	movl	0x0C(%ebp), %esi		# load source pointer
	movl	0x10(%ebp), %ecx		# load count

	shrl	$2, %ecx			# convert byte count to dwords
	jz	memcpy_byte			# if no dword part, go to byte part

	rep					# transfer the dword part
	movsl					#

memcpy_byte:
	movl	0x10(%ebp), %ecx		# load count
	andb	$3, %cl				# mask for odd bytes
	jz	memcpy_done			# if no byte part, done

	rep					# transfer the byte part
	movsb					#

memcpy_done:
	movl	0x0C(%ebp), %eax		# return 'd'

	movw	%es, %dx			# put [ES] selector in [EDX]
	movzwl	%dx, %edx			# for return

	popl	%ecx				#
	popl	%edi				#
	popl	%esi				#

	leave
	ret
*****************************************************************************/

/*****************************************************************************
 * memccpy: copy bytes from 's' to 'd', stop if 'c' copied OR 'n' bytes copied
 *
 * RETURNS:  pointer to byte after 'c' if present, else NULL
 *
 *          [DX] also set to [ES], or NULL on 'c' not found
 *
 * PROTOTYPE:
 *
 *	void*	memccpy (void* d, const void* s, int c, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer
 * 	[EBP+0C] s : source pointer
 * 	[EBP+10] c : compare byte
 * 	[EBP+14] n : maximum number of bytes to copy
 */
	.globl _memccpy
_memccpy:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi				#
	pushl	%edi				#
	pushl	%ebx				#
	pushl	%ecx				#

	cld					# string ops increment

	movl	0x08(%ebp), %edi		# load destination pointer
	movl	0x0C(%ebp), %esi		# load source pointer
	movl	0x10(%ebp), %ebx		# load termination character
	movl	0x14(%ebp), %ecx		# load count

	jcxz	memccpy_found			# if nothing to copy, done

memccpy_copy:
	lodsb					# read char from 's'
	stosb					# write to 'd'

	cmp	%bl, %al			# test for 'c'
	je	memccpy_found			# exit if copied

	loop	memccpy_copy			# repeat until 'n' bytes copied

memccpy_fail:
	xorl	%edx, %edx			# return NULL ptr sel:ofs = 0:0
	xorl	%eax, %eax			#

	jmp	memccpy_exit			#

memccpy_found:
	movl	%edi, %eax			# return ptr to byte after 'c'

	movw	%es, %dx			# put [ES] selector in [EDX]
	movzwl	%dx, %edx			# for return

memccpy_exit:
	popl	%ecx				#
	popl	%ebx				#
	popl	%edi				#
	popl	%esi				#

	leave
	ret


/*****************************************************************************
 * memmove: copy 'n' bytes of memory from 's' to 'd'
 *
 * 's' and 'd' overlaps are handled correctly, but not as fast as memcpy()
 *
 * RETURNS:  'd'
 *
 *          [DX] also set to [ES]
 *
 * PROTOTYPE:
 *
 *	void* memmove (void* d, const void* s, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : source pointer
 * 	[EBP+0C] s : destination pointer
 * 	[EBP+10] n : number of bytes to copy
 */
	.globl _memmove
_memmove:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi				#
	pushl	%edi				#
	pushl	%ecx				#

	cld					# string ops increment

	movl	0x08(%ebp), %edi		# load destination pointer
	movl	0x0C(%ebp), %esi		# load source pointer
	movl	0x10(%ebp), %ecx		# load count

	jcxz	memmove_done			# if nothing to do, done

	rep					# transfer as bytes
	movsb					#

memmove_done:
	movl	0x0C(%ebp), %eax		# return 'd'

	movw	%es, %dx			# put [ES] selector in [EDX]
	movzwl	%dx, %edx			# for return

	popl	%ecx				#
	popl	%edi				#
	popl	%esi				#

	leave
	ret


/*****************************************************************************
 * memset: set 'n' bytes of memory at 'd' to character 'c'
 *
 * RETURNS: 'd'
 *
 *          [DX] also set to [ES]
 *
 * PROTOTYPE:
 *
 *	void* memset (void* d, int c, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer
 * 	[EBP+0C] c : fill character
 * 	[EBP+10] n : number of bytes to fill
*/
	.globl _memset
_memset:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi				#
	pushl	%ecx				#

	cld					# string ops increment

	movl	0x08(%ebp), %edi		# load destination pointer
	movl	0x0C(%ebp), %eax		# get fill character
	movl	0x10(%ebp), %ecx		# load count

	shrl	$2, %ecx			# convert byte count to dwords
	jz	memset_bytes			# if no dword part, go to byte part

	movb	%al, %ah			# get fill character in all bytes of dword
	movw	%ax, %dx			#
	shll	$16, %eax			#
	movw	%dx, %ax			#

	rep					# write the dword part
	stosl					#

memset_bytes:
	movl	0x10(%ebp), %ecx		# load count
	andb	$3, %cl				# mask for odd bytes
	jz	memset_done			# if no byte part, done

	rep					# write the byte part
	stosb					#

memset_done:
	movl	0x08(%ebp), %eax		# return 'd'

	movw	%es, %dx			# put [ES] selector in [EDX]
	movzwl	%dx, %edx			# for return

	pop	%ecx				#
	pop	%edi				#

	leave
	ret


.data

.bss

