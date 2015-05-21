/************************************************************************
 *                                                                      *
 *      Copyright 2013 Concurrent Technologies, all rights reserved.    *
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

/* bcopy.s - optimized buffer copy function
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/bcopy.s,v 1.1 2013-09-04 07:17:57 chippisley Exp $
 *
 * $Log: bcopy.s,v $
 * Revision 1.1  2013-09-04 07:17:57  chippisley
 * Import files into new source repository.
 *
 *
 */


.text


/*******************************************************************************
*
* bcopy - copy one buffer to another
*
* This function copies n bytes from source to destination.  
* The copy is optimized to copy 4 bytes at a time if possible.
* Overlapping buffers are handled. 
*
* RETURNS: none
* 
* PROTOTYPE:
* void bcopy ( char *s, char *d, size_t n )
*     char *s;   - pointer to source buffer
*     char *d;   - pointer to destination buffer
*     size_t n;  - number of bytes to copy
*     
* STACK:
*
*	[EBP+00] old [EBP]
*	[EBP+04] Return Address
* 	[EBP+08] source buffer pointer
* 	[EBP+0C] destination buffer pointer
* 	[EBP+10] number of bytes to copy
*/

	.globl _bcopy
_bcopy:
        pushl   %ebp
        movl    %esp,%ebp

        pushl   %esi
        pushl   %edi
        pushf

        movl    0x08(%ebp),%esi			# Get source
        movl    0x0C(%ebp),%edi			# Get destination
        movl    0x10(%ebp),%edx			# Get n bytes

        testl   %edx,%edx				# If n bytes = 0
        je      bCopyDone				# then exit 

        /* Check for overlap, we will copy backwards if the destination     */
        /* is within the source i.e. destination - source > 0 and < n bytes */

        movl    %edi,%eax				# Get the destination
        subl    %esi,%eax				# Destination - source
        jbe     bCopyFwd				# If destination is below source or
										# destination = source copy forward

        cmpl    %edx,%eax				# Compare to n bytes
        jb      bCopyBwd				# If < n bytes copy backwards

bCopyFwd:								# Copy forwards
        cld

        cmpl    $10,%edx				# If length is less than 10 
        jb      bCopyFwd2				# It's better to do a byte copy

        movl    %edi,%eax				# If destination and source are not both odd 
        xorl    %esi,%eax				# or both even
        btl     $0,%eax
        jc      bCopyFwd2				# Do a byte copy rather than a long copy

        movl    %esi,%eax				# Copy the first 3 bytes if the buffers are
        andl    $3,%eax					# odd-aligned
        je      bCopyFwd1				# Skip if long aligned and start long copy

        negl    %eax					# Complement the remainder
        andl    $3,%eax					# Copy the first 3 bytes
        movl    %eax,%ecx				# %ecx has count
        rep
        movsb							# Copy the bytes
        subl    %eax,%edx				# Decrement count by %eax

bCopyFwd1:								# Copy longs forwards

        movl    %edx,%ecx				# We're copying 4 bytes at a time,
        shrl    $2,%ecx					# so divide count by 4 (count /= 4)
        rep
        movsl							# Copy the longs
        andl    $3,%edx					# Remainder in %edx

bCopyFwd2:								# Copy bytes forwards

        movl    %edx,%ecx				# Setup %ecx as the loop counter
        testl   %ecx,%ecx				# Test if there's anything left to copy
        je      bCopyDone				# No then exit
        rep
        movsb							# Copy the bytes
        jmp     bCopyDone

bCopyBwd:								# Copy backwards

        addl    %edx,%esi
        addl    %edx,%edi

        std

        cmpl    $10,%edx				# If length is less than 10
        jb      bCopyBwd2				# It's better to do a byte copy

        movl    %edi,%eax				# If destination and source are not both odd 
        xorl    %esi,%eax				# or both even
        btl     $0,%eax
        jc      bCopyBwd2				# Do a byte copy rather than a long copy

        movl    %esi,%eax				# Copy the first 3 bytes if the buffers are
        andl    $3,%eax					# odd-aligned
        je      bCopyBwd1				# Skip if long aligned and start long copy

        movl    %eax,%ecx				# Copy the first 3 bytes, %ecx has count
        decl    %esi
        decl    %edi
        rep
        movsb							# Copy the bytes
        incl    %esi
        incl    %edi
        subl    %eax,%edx				# Decrement count by %eax

bCopyBwd1:								# Copy longs backwards

        movl    %edx,%ecx				# We're copying 4 bytes at a time,
        shrl    $2,%ecx					# so divide count by 4 (count /= 4)
        addl    $-4,%esi
        addl    $-4,%edi
        rep
        movsl							# Copy the longs
        addl    $4,%esi
        addl    $4,%edi
        andl    $3,%edx					# Remainder in %edx

bCopyBwd2:								# Copy bytes backwards

        movl    %edx,%ecx				# Setup %ecx as the loop counter
        testl   %ecx,%ecx				# Test if there's anything left to copy
        je      bCopyDone				# No then exit
        decl    %esi
        decl    %edi
        rep
        movsb							# Copy the bytes

bCopyDone:

        popf
        popl    %edi
        popl    %esi

        leave
        ret


.data

.bss
