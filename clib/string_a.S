
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

/* STRING.S - Optimized implementation of <string.h> functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/string_a.S,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: string_a.S,v $
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


.include "asm/i386.h"

.text


/*****************************************************************************
 * strlen: get length of 's'
 *
 * RETURNS: length of 's'
 *
 * PROTOTYPE:
 *
 *	size_t	strlen (const char* s)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] source pointer
 */
	.globl _strlen
_strlen:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi			#
	pushl	%ecx			#

	cld				# string ops increment

	movl	8(%ebp), %edi		# load source pointer [ES:EDI]

	xorb	%al, %al		# load [AL] with NULL for compare

	xorl	%ecx, %ecx		# start count at -1
	dec	%ecx			#

	repne				# read char and check for NULL
	scasb				#

	notl	%ecx			# get length-1 for return
	decl	%ecx			#
	movl	%ecx, %eax		#

	popl	%ecx			#
	popl	%edi			#

	leave
	ret


/*****************************************************************************
 * strchr: scans 's' for the first occurence of character 'c'
 *
 * RETURNS: pointer to first occurrence of 'c' in 's'
 *	    if 'c' not in 's', returns NULL
 *
 * PROTOTYPE:
 *
 *	char* strchr (const char* s, int c)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s : source pointer
 * 	[EBP+0C] c : search character
 */
	.globl _strchr
_strchr:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#

	cld				# string ops increment

	movl	0x08(%ebp), %esi	# load source pointer [DS:ESI]
	movb	0x0C(%ebp), %ah		# load search character
	
strchr_loop:
	lodsb				# read and check char 
	cmp	%al, %ah		#
	je	strchr_found		# jump if match

	orb	%al, %al		# continue search until NULL found
	jnz	strchr_loop		# 

	xor	%eax, %eax		# return NULL if no match
	jmp	strchr_done		#

strchr_found:
	decl	%esi			# back-up pointer to before char
	movl	%esi, %eax		# put in [EAX] for return

strchr_done:
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strrchr: scans 's' for the last occurence of character 'c'
 *
 * RETURNS: pointer to last occurrence of 'c' in 's'
 *	    if 'c' not in 's', returns NULL
 *
 * PROTOTYPE:
 *
 *	char* strrchr (const char* s, int c)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s : source pointer
 * 	[EBP+0C] c : search character
 */
	.globl _strrchr
_strrchr:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%ebx			#

	cld				# string ops increment

	movl	0x08(%ebp), %esi	# load source pointer [DS:ESI]
	movb	0x0C(%ebp), %ah		# load search character
	
	xor	%ebx, %ebx		# return NULL if no match

strrchr_loop:
	lodsb				# read and check char 
	cmp	%al, %ah		#
	jne	strrchr_next		# jump if no match

	movl	%esi, %ebx		# record pointer to current match
	dec	%ebx			# back-up pointer to before char

strrchr_next:
	orb	%al, %al		# stop when NULL found, [EBX] contains last
	jnz	strrchr_loop		#
	
	movl	%ebx, %eax		# put pointer in [EAX] for return

	popl	%ebx			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strcat: appends 's' to 'd'
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	char* strcat (char* d, const char* s)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : source pointer
 * 	[EBP+0C] s : destination pointer
 */
	.globl _strcat
_strcat:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%edi			#
	pushl	%ecx			#

	cld				# string ops increment

	movl	0x08(%ebp), %edi	# load destination pointer [ES:EDI]
	movl	0x0C(%ebp), %esi	# load source pointer [DS:ESI]

	/* Find end of destination string */

	xorl	%ecx, %ecx		# start count at -1
	dec	%ecx			#

	xor	%al, %al		# search for char = NULL
	repne				#
	scasb				#

	decl	%edi			# back-up pointer to before NULL

	/* Append source onto destination */

strcat_copy:
	lodsb				# copy char [AL] 
	stosb				#

	orb	%al, %al		# check if NULL reached
	jnz	strcat_copy		# loop if not

	movl	0x08(%ebp), %eax	# return 'd' pointer

	popl	%ecx			#
	popl	%edi			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strncat: appends maximum of 'n' characters from 's' onto 'd'
 *
 * Stops at NULL, else terminates with NULL after 'n' chars from 's'.
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	char* strncat (char* d, const char* s, size_t n);
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s : source pointer
 * 	[EBP+0C] d : destination pointer
 * 	[EBP+10] n : maximum number of characters
 */
	.globl _strncat
_strncat:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%edi			#
	pushl	%ecx			#

	cld				# string ops increment

	movl	0x08(%ebp), %edi	# load destination pointer [ES:EDI]
	movl	0x0C(%ebp), %esi	# load source pointer [DS:ESI]

	/* Find end of destination string */

	xorl	%ecx, %ecx		# start count at -1
	dec	%ecx			#

	xor	%al, %al		# search for char = NULL
	repne				#
	scasb				#

	decl	%edi			# back-up pointer to before NULL

	/* Append maximum of 'n' chars from source onto destination */

	movl	0x10(%ebp), %ecx	# load maximum char count

strncat_copy:
	decl	%ecx			# decrement count (catches 'n' = 0)
	js	strncat_done		# done if [ECX] now -ve

	lodsb				# copy char [AL] 
	stosb				#

	orb	%al, %al		# check if NULL reached in source
	jnz	strncat_copy		# repeat upto maximum

	jmp	strncat_exit		# don't append another NULL

strncat_done:
	xorb	%al, %al		# NULL terminate after 'n' chars
	stosb				#

strncat_exit:
	movl	0x08(%ebp), %eax	# return 'd' pointer

	popl	%ecx			#
	popl	%edi			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strcmp: compare 's1' to 's2'
 *
 * Comparison stop at first NULL
 *
 * RETURNS:  0 if 's1' and 's2' match,
 *	   < 0 if 's1' < 's2'
 *	   > 0 if 's1' > 's2'
 *
 * PROTOTYPE:
 *
 *	int strcmp (const char* s1, const char* s2)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s1 : source pointer #1
 * 	[EBP+0C] s2 : source pointer #2
 */
	.globl _strcmp
_strcmp:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%edi			#

	cld				# string ops increment

	movl	0x08(%ebp), %esi	# load 's1' pointer [DS:ESI]
	movl	0x0C(%ebp), %edi	# load 's2' pointer [ES:EDI]

strcmp_loop:
	lodsb				# read from 's1'
	movb	%es:(%edi), %ah		# read from 's2'
	incl	%edi			#

	orw	%ax, %ax		# both strings at NULL?
	jz	strcmp_done		# yes, exit

	subb	%ah, %al		# compare chars (s1 - s2)
	jz	strcmp_loop		# loop if equal

strcmp_done:
	movsbl	%al, %eax		# put comparison result in [EAX]

	popl	%edi			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strncmp: compare 's1' to 's2', maximum of 'n' chars
 *
 * Comparison stop at first NULL or after a maximum of 'n' chars
 *
 * RETURNS:  0 if 's1' and 's2' match,
 *	   < 0 if 's1' < 's2'
 *	   > 0 if 's1' > 's2'
 *
 * PROTOTYPE:
 *
 *	int strncmp (const char* s1, const char* s2, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s1 : source pointer #1
 * 	[EBP+0C] s2 : source pointer #2
 * 	[EBP+10] n : maximum number of characters
 */
	.globl _strncmp
_strncmp:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%edi			#
	pushl	%ecx			#

	cld				# string ops increment

	movl	0x08(%ebp), %esi	# load 's1' pointer [DS:ESI]
	movl	0x0C(%ebp), %edi	# load 's2' pointer [ES:EDI]
	movl	0x10(%ebp), %ecx	# load maximum char count

	xorb	%al, %al		# if 'n' = 0, strings are equal!

strncmp_loop:
	decl	%ecx			# decrement count (catches 'n' = 0)
	js	strncmp_done		# done if [ECX] now -ve (strings are equal)

	lodsb				# read from 's1'
	movb	%es:(%edi), %ah		# read from 's2'
	incl	%edi			#

	orw	%ax, %ax		# both strings at NULL?
	jz	strncmp_done		# yes, exit

	subb	%ah, %al		# compare chars (s1 - s2)
	jz	strncmp_loop		# loop if equal

strncmp_done:
	movsbl	%al, %eax		# put comparison result in [EAX]

	popl	%ecx			#
	popl	%edi			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strcpy: copy 's' to 'd'
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	char* strcpy (char* d, const char* s)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer
 * 	[EBP+0C] s : source pointer
 */
	.globl _strcpy
_strcpy:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%edi			#

	cld				# string ops increment

	movl	0x08(%ebp), %edi	# load 'd' pointer [ES:EDI]
	movl	0x0C(%ebp), %esi	# load 's' pointer [DS:ESI]

strcpy_copy:
	lodsb				# copy byte
	stosb				#

	orb	%al, %al		# check for NULL
	jnz	strcpy_copy		# loop if not

	movl	0x08(%ebp), %eax	# return 'd' pointer

	popl	%edi			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strncpy: copy a maximum of 'n' chars from 's' to 'd'
 *
 * 'd' might not be NULL terminated. Pad 'd' with NULL when len (s) < n
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	char* strncpy (char* d, const char* s, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s : source pointer
 * 	[EBP+0C] d : destination pointer
 * 	[EBP+10] n : maximum number of characters
 */
	.globl _strncpy
_strncpy:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%edi			#
	pushl	%ecx			#

	cld				# string ops increment

	movl	0x08(%ebp), %edi	# load 'd' pointer [ES:EDI]
	movl	0x0C(%ebp), %esi	# load 's' pointer [DS:ESI]
	movl	0x10(%ebp), %ecx	# load maximum char count

strncpy_copy:
	decl	%ecx			# decrement count (catches 'n' = 0)
	js	strncpy_done		# done if [ECX] now -ve

	lodsb				# copy byte
	stosb				#

	orb	%al, %al		# check for NULL
	jnz	strncpy_copy		# loop if not


	xorb	%al, %al		# NULL char for padding

strncpy_pad:
	decl	%ecx			# done if [ECX] now -ve
	js	strncpy_done		#

	stosb				# pad with NULL to 'n' characters
	jmp	strncpy_pad		#


strncpy_done:
	movl	0x08(%ebp), %eax	# return 'd' pointer

	popl	%ecx			#
	popl	%edi			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * stricmp: case-insensitive compare of 's1' to 's2'
 *
 * Comparison stop at first NULL
 *
 * RETURNS:  0 if 's1' and 's2' match,
 *	   < 0 if 's1' < 's2'
 *	   > 0 if 's1' > 's2'
 *
 * PROTOTYPE:
 *
 *	int strcmp (const char* s1, const char* s2)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s1 : source pointer #1
 * 	[EBP+0C] s2 : source pointer #2
 */
	.globl _stricmp
_stricmp:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%edi			#

	cld				# string ops increment

	movl	0x08(%ebp), %esi	# load 's1' pointer [DS:ESI]
	movl	0x0C(%ebp), %edi	# load 's2' pointer [ES:EDI]

stricmp_loop:
	lodsb				# read from 's1'
	movb	%es:(%edi), %ah		# read from 's2'
	incl	%edi			#

	orw	%ax, %ax		# both strings at NULL?
	jz	stricmp_done		# yes, exit

stricmp_s1:
	cmpb	$'Z', %al		# check 's1' for uppercase
	ja	stricmp_s2		#
	cmpb	$'A', %al		#
	jb	stricmp_s2		#

	orb	$0x20, %al		# make lowercase for compare

stricmp_s2:
	cmpb	$'Z', %ah		# check 's2' for uppercase
	ja	stricmp_test		#
	cmpb	$'A', %ah		#
	jb	stricmp_test		#

	orb	$0x20, %ah		# make lowercase for compare

stricmp_test:
	subb	%ah, %al		# compare chars (s1 - s2)
	jz	stricmp_loop		# loop if equal

stricmp_done:
	movsbl	%al, %eax		# put comparison result in [EAX]

	popl	%edi			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strnicmp: case insensitive compare of 's1' to 's2', maximum of 'n' chars
 *
 * Comparison stop at first NULL or after a maximum of 'n' chars
 *
 * RETURNS:  0 if 's1' and 's2' match,
 *	   < 0 if 's1' < 's2'
 *	   > 0 if 's1' > 's2'
 *
 * PROTOTYPE:
 *
 *	int strncmp (const char* s1, const char* s2, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] s1 : source pointer #1
 * 	[EBP+0C] s2 : source pointer #2
 * 	[EBP+10] n : maximum number of characters
 */
	.globl _strnicmp
_strnicmp:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%esi			#
	pushl	%edi			#
	pushl	%ecx			#

	cld				# string ops increment

	movl	0x08(%ebp), %esi	# load 's1' pointer [DS:ESI]
	movl	0x0C(%ebp), %edi	# load 's2' pointer [ES:EDI]
	movl	0x10(%ebp), %ecx	# load maximum char count

	xorb	%al, %al		# if 'n' = 0, strings are equal!

strnicmp_loop:
	decl	%ecx			# decrement count (catches 'n' = 0)
	js	strnicmp_done		# done if [ECX] now -ve

	lodsb				# read from 's1'
	movb	%es:(%edi), %ah		# read from 's2'
	incl	%edi			#

	orw	%ax, %ax		# both strings at NULL?
	jz	strnicmp_done		# yes, exit

strnicmp_s1:
	cmpb	$'Z', %al		# check 's1' for uppercase
	ja	strnicmp_s2		#
	cmpb	$'A', %al		#
	jb	strnicmp_s2		#

	orb	$0x20, %al		# make lowercase for compare

strnicmp_s2:
	cmpb	$'Z', %ah		# check 's2' for uppercase
	ja	strnicmp_test		#
	cmpb	$'A', %ah		#
	jb	strnicmp_test		#

	orb	$0x20, %ah		# make lowercase for compare

strnicmp_test:
	subb	%ah, %al		# compare chars (s1 - s2)
	jz	strnicmp_loop		# loop if equal

strnicmp_done:
	movsbl	%al, %eax		# put comparison result in [EAX]

	popl	%ecx			#
	popl	%edi			#
	popl	%esi			#

	leave
	ret


/*****************************************************************************
 * strset: set all characters in 'd' to 'c'
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	char* strset (char* d, int c)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer
 * 	[EBP+0C] c : character used to set string
 */
	.globl _strset
_strset:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi			#

	cld				# string ops increment

	movl	0x08(%ebp), %edi	# load 'd' pointer [ES:EDI]
	movb	0x0C(%ebp), %al		# fill character [AL]

strset_copy:
	andb	$0xFF, (%edi)		# exit if existing char is NULL
	jz	strset_done		#

	stosb				# write fill char to string
	jmp	strset_copy		#

strset_done:
	movl	0x08(%ebp), %eax	# return 'd' pointer

	popl	%edi			#

	leave
	ret


/*****************************************************************************
 * strnset:  set up to 'n' characters in 'd' to 'c'
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	char* strnset (char* d, int c, size_t n)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer
 * 	[EBP+0C] c : character used to set string
 * 	[EBP+10] n : maximum number of characters
 */
	.globl _strnset
_strnset:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi			#
	pushl	%ecx			#

	cld				# string ops increment

	movl	0x08(%ebp), %edi	# load 'd' pointer [ES:EDI]
	movb	0x0C(%ebp), %al		# fill character [AL]
	movl	0x10(%ebp), %ecx	# load maximum char count

strnset_copy:
	decl	%ecx			# decrement count (catches 'n' = 0)
	js	strnset_done		# done if [ECX] now -ve

	andb	$0xFF, (%edi)		# exit if existing char is NULL
	jz	strset_done		#

	stosb				# write fill char to string
	jmp	strnset_copy		#

strnset_done:
	movl	0x08(%ebp), %eax	# return 'd' pointer

	popl	%ecx			#
	popl	%edi			#

	leave
	ret


/*****************************************************************************
 * strlwr: set all characters in 'd' to lowercase
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	char* strlwr (char* d)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer
 */
	.globl _strlwr
_strlwr:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi			#

	cld				# string ops increment

	movl	0x08(%ebp), %edi	# load 'd' pointer [ES:EDI]

strlwr_copy:
	movb	(%edi), %al		# read char, check for NULL
	or	%al, %al		#
	jz	strlwr_done		#

	cmpb	$'Z', %al		# check for uppercase
	ja	strlwr_next		#
	cmpb	$'A', %al		#
	jb	strlwr_next		#

	orb	$0x20, %al		# make lowercase

strlwr_next:
	stosb				# write char back to string
	jmp	strlwr_copy		#

strlwr_done:
	movl	0x08(%ebp), %eax	# return 'd' pointer

	popl	%edi			#

	leave
	ret


/*****************************************************************************
 * strupr: set all characters in 'd' to uppercase
 *
 * RETURNS: pointer to 'd'
 *
 * PROTOTYPE:
 *
 *	char* strupr (char* d)
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] d : destination pointer
 */
	.globl _strupr
_strupr:

	pushl	%ebp
	movl	%esp, %ebp

	pushl	%edi			#

	cld				# string ops increment

	movl	0x08(%ebp), %edi	# load 'd' pointer [ES:EDI]

strupr_copy:
	movb	(%edi), %al		# read char, check for NULL
	or	%al, %al		#
	jz	strupr_done		#

	cmpb	$'a', %al		# check for lowercase
	jb	strupr_next		#
	cmpb	$'z', %al		#
	ja	strupr_next		#

	andb	$0xDF, %al		# make uppercase

strupr_next:
	stosb				# write char back to string
	jmp	strupr_copy		#

strupr_done:
	movl	0x08(%ebp), %eax	# return 'd' pointer

	popl	%edi			#

	leave
	ret


.data

.bss

