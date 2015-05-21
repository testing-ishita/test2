
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

/* far_mem.S - PTR48 interface to mem.S functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/far_mem.S,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: far_mem.S,v $
 * Revision 1.1  2013-09-04 07:17:58  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2009/09/29 12:20:16  swilson
 * Create far-pointer memory access functions
 *
 * Revision 1.1  2008/05/21 13:27:54  swilson
 * Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
 *
 */


.text


/*****************************************************************************
 * memchr: search 'n' bytes of memory at 's' for character 'c'
 *
 * RETURNS: pointer to first occurance of 'c', else NULL (sel:ofs = 0:0)
 *
 * PROTOTYPE:
 *
 *	void*	_fmemchr (const PTR48* s, int c, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s : source pointer - offset
 * 	[EBP+0C]		    - selector
 * 	[EBP+10] c : compare byte
 * 	[EBP+14] n : maximum number of bytes to search
 */
	.globl __fmemchr
__fmemchr:

	pushl	%ebp
	movl	%esp, %ebp

	subl	$0x20, %esp		# make space for CALL args & locals

	movl	%ebx, -0x04(%ebp)	# save [EBX]

	movw	%es, %bx		# save [ES]
	movl	%ebx, -0x08(%ebp)	#


	movl	0x0C(%ebp), %eax	# load [ES] = 's' selector
	movw	%ax, %es		#

	# Put other arguments on stack for call

	movl	0x08(%ebp), %eax	# s
	movl	%eax, 0x00(%esp)	#

	movl	0x10(%ebp), %eax	# c
	movl	%eax, 0x04(%esp)	#

	movl	0x14(%ebp), %eax	# n
	movl	%eax, 0x08(%esp)	#

	call	_memchr			# call the standard library function


	movl	-0x08(%ebp), %ebx	# restore [ES]
	movw	%bx, %es		#

	movl	-0x04(%ebp), %ebx	# restore [EBX]

	leave
	ret


/*****************************************************************************
 * _fmemcmp: compare 'n' bytes of memory at 's1' to memory at 's2'
 *
 * RETURNS:  0 if 's1' and 's2' match,
 *	   < 0 if 's1' < 's2'
 *	   > 0 if 's1' > 's2'
 *
 * PROTOTYPE:
 *
 *	int _fmemcmp (const PTR48* s1, const PTR48* s2, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s1 : source pointer #1 - offset
 * 	[EBP+0C]			- selector
 * 	[EBP+10] s2 : source pointer #2 - offset
 * 	[EBP+14]			- selector
 * 	[EBP+18] n  : number of bytes to compare
 */
	.globl __fmemcmp
__fmemcmp:

	pushl	%ebp
	movl	%esp, %ebp

	subl	$0x20, %esp		# make space for CALL args & locals

	movl	%ebx, -0x04(%ebp)	# save [EBX]

	movw	%ds, %bx		# save [DS] and [ES]
	movl	%ebx, -0x08(%ebp)	#
	movw	%es, %bx		#
	movl	%ebx, -0x0C(%ebp)	#

	movl	0x0C(%ebp), %eax	# load [DS] = 's1' selector
	movw	%ax, %ds		#

	movl	0x14(%ebp), %eax	# load [ES] = 's2' selector
	movw	%ax, %es		#

	# Put other arguments on stack for call

	movl	0x08(%ebp), %eax	# s1
	movl	%eax, 0x00(%esp)	#

	movl	0x10(%ebp), %eax	# s2
	movl	%eax, 0x04(%esp)	#

	movl	0x18(%ebp), %eax	# n
	movl	%eax, 0x08(%esp)	#

	call	_memcmp			# call the standard library function

	movl	-0x0C(%ebp), %ebx	# restore [DS] and [ES]
	movw	%bx, %es		#
	movl	-0x08(%ebp), %ebx	#
	movw	%bx, %ds		#

	movl	-0x04(%ebp), %ebx	# restore [EBX]

	leave
	ret


/*****************************************************************************
 * _fmemicmp: compare 'n' bytes of memory at 's1' to memory at 's2'
 *
 * Bytes are treated as unsigned chars and testing is case insensitive
 *
 * RETURNS:  0 if 's1' and 's2' match,
 *	   < 0 if 's1' < 's2'
 *	   > 0 if 's1' > 's2'
 *
 * PROTOTYPE:
 *
 *	int _fmemcmp (const PTR48* s1, const PTR48* s2, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s1 : source pointer #1 - offset
 * 	[EBP+0C]			- selector
 * 	[EBP+10] s2 : source pointer #2 - offset
 * 	[EBP+14]			- selector
 * 	[EBP+18] n  : number of bytes to compare
 */
	.globl __fmemicmp
__fmemicmp:

	pushl	%ebp
	movl	%esp, %ebp

	subl	$0x20, %esp		# make space for CALL args & locals

	movl	%ebx, -0x04(%ebp)	# save [EBX]

	movw	%ds, %bx		# save [DS] and [ES]
	movl	%ebx, -0x08(%ebp)	#
	movw	%es, %bx		#
	movl	%ebx, -0x0C(%ebp)	#

	movl	0x0C(%ebp), %eax	# load [DS] = 's1' selector
	movw	%ax, %ds		#

	movl	0x14(%ebp), %eax	# load [ES] = 's2' selector
	movw	%ax, %es		#

	# Put other arguments on stack for call

	movl	0x08(%ebp), %eax	# s1
	movl	%eax, 0x00(%esp)	#

	movl	0x10(%ebp), %eax	# s2
	movl	%eax, 0x04(%esp)	#

	movl	0x18(%ebp), %eax	# n
	movl	%eax, 0x08(%esp)	#

	call	_memicmp		# call the standard library function

	movl	-0x0C(%ebp), %ebx	# restore [DS] and [ES]
	movw	%bx, %es		#
	movl	-0x08(%ebp), %ebx	#
	movw	%bx, %ds		#

	movl	-0x04(%ebp), %ebx	# restore [EBX]

	leave
	ret


/*****************************************************************************
 * _fmemcpy: copy 'n' bytes of memory from 's' to 'd'
 *
 * 's' and 'd' must not overlap within a 32-bit word 
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	PTR48*	_fmemcpy (PTR48* d, const PTR48* s, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 * 	[EBP+10] s : source pointer - offset
 * 	[EBP+14]		    - selector
 * 	[EBP+18] n : number of bytes to copy
 */
	.globl __fmemcpy
__fmemcpy:

	pushl	%ebp
	movl	%esp, %ebp

	subl	$0x20, %esp		# make space for CALL args & locals

	movl	%ebx, -0x04(%ebp)	# save [EBX]

	movw	%ds, %bx		# save [DS] and [ES]
	movl	%ebx, -0x08(%ebp)	#
	movw	%es, %bx		#
	movl	%ebx, -0x0C(%ebp)	#

	movl	0x0C(%ebp), %eax	# load [DS] = 'd' selector
	movw	%ax, %es		#

	movl	0x14(%ebp), %eax	# load [ES] = 's' selector
	movw	%ax, %ds		#

	# Put other arguments on stack for call

	movl	0x08(%ebp), %eax	# d
	movl	%eax, 0x00(%esp)	#

	movl	0x10(%ebp), %eax	# s
	movl	%eax, 0x04(%esp)	#

	movl	0x18(%ebp), %eax	# n
	movl	%eax, 0x08(%esp)	#

	call	_memcpy			# call the standard library function

	movl	-0x0C(%ebp), %ebx	# restore [DS] and [ES]
	movw	%bx, %es		#
	movl	-0x08(%ebp), %ebx	#
	movw	%bx, %ds		#

	movl	-0x04(%ebp), %ebx	# restore [EBX]

	leave
	ret


/*****************************************************************************
 * _fmemccpy: copy bytes from 's' to 'd', stop if 'c' copied OR 'n' bytes copied
 *
 * RETURNS:  pointer to char after last copied, else NULL (sel:ofs = 0:0)
 *
 * PROTOTYPE:
 *
 *	PTR48*	_fmemcpy (PTR48* d, const PTR48* s, int c, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 * 	[EBP+10] s : source pointer - offset
 * 	[EBP+14]		    - selector
 * 	[EBP+18] c : compare byte
 * 	[EBP+1C] n : maximum number of bytes to copy
 */
	.globl __fmemccpy
__fmemccpy:

	pushl	%ebp
	movl	%esp, %ebp

	subl	$0x20, %esp		# make space for CALL args & locals

	movl	%ebx, -0x04(%ebp)	# save [EBX]

	movw	%ds, %bx		# save [DS] and [ES]
	movl	%ebx, -0x08(%ebp)	#
	movw	%es, %bx		#
	movl	%ebx, -0x0C(%ebp)	#

	movl	0x0C(%ebp), %eax	# load [DS] = 'd' selector
	movw	%ax, %es		#

	movl	0x14(%ebp), %eax	# load [ES] = 's' selector
	movw	%ax, %ds		#

	# Put other arguments on stack for call

	movl	0x08(%ebp), %eax	# d
	movl	%eax, 0x00(%esp)	#

	movl	0x10(%ebp), %eax	# s
	movl	%eax, 0x04(%esp)	#

	movl	0x18(%ebp), %eax	# c
	movl	%eax, 0x08(%esp)	#

	movl	0x1C(%ebp), %eax	# n
	movl	%eax, 0x0C(%esp)	#

	call	_memccpy		# call the standard library function

	movl	-0x0C(%ebp), %ebx	# restore [DS] and [ES]
	movw	%bx, %es		#
	movl	-0x08(%ebp), %ebx	#
	movw	%bx, %ds		#

	movl	-0x04(%ebp), %ebx	# restore [EBX]

	leave
	ret


/*****************************************************************************
 * _fmemmove: copy 'n' bytes of memory from 's' to 'd'
 *
 * 's' and 'd' overlaps are handled correctly, but not as fast as _fmemcpy()
 *
 * RETURNS:  'd'
 *
 * PROTOTYPE:
 *
 *	PTR48* _fmemmove (PTR48* d, const PTR48* s, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : source pointer - offset
 * 	[EBP+0C]		    - selector
 * 	[EBP+10] s : destination pointer - offset
 * 	[EBP+14]			 - selector
 * 	[EBP+18] n : number of bytes to copy
 */
	.globl __fmemmove
__fmemmove:

	pushl	%ebp
	movl	%esp, %ebp

	subl	$0x20, %esp		# make space for CALL args & locals

	movl	%ebx, -0x04(%ebp)	# save [EBX]

	movw	%ds, %bx		# save [DS] and [ES]
	movl	%ebx, -0x08(%ebp)	#
	movw	%es, %bx		#
	movl	%ebx, -0x0C(%ebp)	#

	movl	0x0C(%ebp), %eax	# load [DS] = 'd' selector
	movw	%ax, %es		#

	movl	0x14(%ebp), %eax	# load [ES] = 's' selector
	movw	%ax, %ds		#

	# Put other arguments on stack for call

	movl	0x08(%ebp), %eax	# d
	movl	%eax, 0x00(%esp)	#

	movl	0x10(%ebp), %eax	# s
	movl	%eax, 0x04(%esp)	#

	movl	0x18(%ebp), %eax	# n
	movl	%eax, 0x08(%esp)	#

	call	_memmove		# call the standard library function

	movl	-0x0C(%ebp), %ebx	# restore [DS] and [ES]
	movw	%bx, %es		#
	movl	-0x08(%ebp), %ebx	#
	movw	%bx, %ds		#

	movl	-0x04(%ebp), %ebx	# restore [EBX]

	leave
	ret


/*****************************************************************************
 * _fmemset: set 'n' bytes of memory at 'd' to character 'c'
 *
 * RETURNS: 'd'
 *
 * PROTOTYPE:
 *
 *	PTR48* _fmemset (PTR48* d, int c, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 * 	[EBP+10] c : fill character
 * 	[EBP+14] n : number of bytes to fill
 */
	.globl __fmemset
__fmemset:

	pushl	%ebp
	movl	%esp, %ebp

	subl	$0x20, %esp		# make space for CALL args & locals

	movl	%ebx, -0x04(%ebp)	# save [EBX]

	movw	%es, %bx		# save [ES]
	movl	%ebx, -0x08(%ebp)	#

	movl	0x0C(%ebp), %eax	# load [ES] = 'd' selector
	movw	%ax, %es		#

	# Put other arguments on stack for call

	movl	0x08(%ebp), %eax	# d
	movl	%eax, 0x00(%esp)	#

	movl	0x10(%ebp), %eax	# c
	movl	%eax, 0x04(%esp)	#

	movl	0x14(%ebp), %eax	# n
	movl	%eax, 0x08(%esp)	#

	call	_memset			# call the standard library function

	movl	-0x08(%ebp), %ebx	# restore [ES]
	movw	%bx, %es		#

	movl	-0x04(%ebp), %ebx	# restore [EBX]

	leave
	ret


/*****************************************************************************
 * _fMemWriteByte: write a byte to memory
 *
 * RETURNS: none
 *
 * PROTOTYPE:
 *
 *	void _fMemWriteByte (PTR48* d, UINT8 b)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 * 	[EBP+10] b : byte data
 */
	.globl __fMemWriteByte
__fMemWriteByte:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx			#
	pushw	%es			#

	# Get arguments off stack

	movl	0x0C(%ebp), %eax	# load [ES] = 'd' selector
	movw	%ax, %es		#

	movl	0x08(%ebp), %ebx	# d
	movl	0x10(%ebp), %eax	# b

	movb	%al, %es:(%ebx)		# write the data

	popw	%es			#
	popl	%ebx			#

	leave
	ret


/*****************************************************************************
 * _fMemWriteByte: write a word to memory
 *
 * RETURNS: none
 *
 * PROTOTYPE:
 *
 *	void _fMemWriteWord (PTR48* d, UINT16 w)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 * 	[EBP+10] w : word data
 */
	.globl __fMemWriteWord
__fMemWriteWord:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx			#
	pushw	%es			#

	# Get arguments off stack

	movl	0x0C(%ebp), %eax	# load [ES] = 'd' selector
	movw	%ax, %es		#

	movl	0x08(%ebp), %ebx	# d
	movl	0x10(%ebp), %eax	# w

	movw	%ax, %es:(%ebx)		# write the data

	popw	%es			#
	popl	%ebx			#

	leave
	ret


/*****************************************************************************
 * _fMemWriteByte: write a dword to memory
 *
 * RETURNS: none
 *
 * PROTOTYPE:
 *
 *	void _fMemWriteDword (PTR48* d, UINT32 dw)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 * 	[EBP+10] dw : byte data
 */
	.globl __fMemWriteDword
__fMemWriteDword:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx			#
	pushw	%es			#

	# Get arguments off stack

	movl	0x0C(%ebp), %eax	# load [ES] = 'd' selector
	movw	%ax, %es		#

	movl	0x08(%ebp), %ebx	# d
	movl	0x10(%ebp), %eax	# dw

	movl	%eax, %es:(%ebx)	# write the data

	popw	%es			#
	popl	%ebx			#

	leave
	ret


/*****************************************************************************
 * _fMemReadByte: read a byte from memory
 *
 * RETURNS: BYTE data
 *
 * PROTOTYPE:
 *
 *	UINT8 _fMemReadByte (PTR48* d)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 */
	.globl __fMemReadByte
__fMemReadByte:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx			#
	pushw	%es			#


	movl	0x0C(%ebp), %eax	# load [ES] = 'd' selector
	movw	%ax, %es		#

	movl	0x08(%ebp), %ebx	# d

	movb	%es:(%ebx), %al		# read the data


	popw	%es			#
	popl	%ebx			#

	leave
	ret


/*****************************************************************************
 * _fMemReadWord: read a word from memory
 *
 * RETURNS: WORD data
 *
 * PROTOTYPE:
 *
 *	UINT16 _fMemReadWord (PTR48* d)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 */
	.globl __fMemReadWord
__fMemReadWord:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx			#
	pushw	%es			#


	movl	0x0C(%ebp), %eax	# load [ES] = 'd' selector
	movw	%ax, %es		#

	movl	0x08(%ebp), %ebx	# d

	movw	%es:(%ebx), %ax		# read the data


	popw	%es			#
	popl	%ebx			#

	leave
	ret


/*****************************************************************************
 * _fMemReadByte: read a dword from memory
 *
 * RETURNS: DWORD data
 *
 * PROTOTYPE:
 *
 *	UINT32 _fMemReadDword (PTR48* d)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer - offset
 * 	[EBP+0C]			 - selector
 */
	.globl __fMemReadDword
__fMemReadDword:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%ebx			#
	pushw	%es			#


	movl	0x0C(%ebp), %eax	# load [ES] = 'd' selector
	movw	%ax, %es		#

	movl	0x08(%ebp), %ebx	# d

	movl	%es:(%ebx), %eax	# read the data


	popw	%es			#
	popl	%ebx			#

	leave
	ret


.data

.bss

