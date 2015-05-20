
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


/* xmmx.S - mmx & xmm read/write & test functions
 */

/* Version control:
 *
 * Revision 1.0  2009/04/02 hmuneer
 * CReated.
 *
 */



.text



.globl _xmmWrite 
_xmmWrite:
	pushl	%ebp
	movl	%esp, %ebp
	pushal

	movl    %cr4, %eax
	or      $0x200, %eax
	movl    %eax, %cr4	

	movl    0x08(%ebp), %eax	# reg number
	movl	0x0C(%ebp), %edi	# pointer to buffer


	cmp     $0x00, %eax
	jz		_xmm0RegWrite
	cmp     $0x01, %eax
	jz		_xmm1RegWrite
	cmp     $0x02, %eax
	jz		_xmm2RegWrite
	cmp     $0x03, %eax
	jz		_xmm3RegWrite
	cmp     $0x04, %eax
	jz		_xmm4RegWrite
	cmp     $0x05, %eax
	jz		_xmm5RegWrite
	cmp     $0x06, %eax
	jz		_xmm6RegWrite
	cmp     $0x07, %eax
	jz		_xmm7RegWrite

_xmm0RegWrite:
	movdqu	0x00(%edi), %xmm0	# load mm0
	jmp		_xmmWriteEnd

_xmm1RegWrite:
	movdqu	0x00(%edi), %xmm1	# load mm1
	jmp		_xmmWriteEnd

_xmm2RegWrite:
	movdqu	0x00(%edi), %xmm2	# load mm2
	jmp		_xmmWriteEnd

_xmm3RegWrite:
	movdqu	0x00(%edi), %xmm3	# load mm3
	jmp		_xmmWriteEnd

_xmm4RegWrite:
	movdqu	0x00(%edi), %xmm4	# load mm4
	jmp		_xmmWriteEnd

_xmm5RegWrite:
	movdqu	0x00(%edi), %xmm5	# load mm5
	jmp		_xmmWriteEnd

_xmm6RegWrite:
	movdqu	0x00(%edi), %xmm6	# load mm6
	jmp		_xmmWriteEnd

_xmm7RegWrite:
	movdqu	0x00(%edi), %xmm7	# load mm7
	jmp		_xmmWriteEnd


_xmmWriteEnd:
	popal
	leave
	ret 



	.globl _xmmRead 
_xmmRead:
	pushl	%ebp
	movl	%esp, %ebp
	pushal

	movl    %cr4, %eax
	or      $0x200, %eax
	movl    %eax, %cr4	

	movl    0x08(%ebp), %eax	# reg number
	movl	0x0C(%ebp), %edi	# pointer to buffer

	cmp     $0x00, %eax
	jz		_xmm0RegRead
	cmp     $0x01, %eax
	jz		_xmm1RegRead
	cmp     $0x02, %eax
	jz		_xmm2RegRead
	cmp     $0x03, %eax
	jz		_xmm3RegRead
	cmp     $0x04, %eax
	jz		_xmm4RegRead
	cmp     $0x05, %eax
	jz		_xmm5RegRead
	cmp     $0x06, %eax
	jz		_xmm6RegRead
	cmp     $0x07, %eax
	jz		_xmm7RegRead

_xmm0RegRead:
	movdqu	%xmm0, 0x00(%edi)	# load mm0
	jmp		_xmmReadEnd

_xmm1RegRead:
	movdqu	%xmm1, 0x00(%edi)	# load mm1
	jmp		_xmmReadEnd

_xmm2RegRead:
	movdqu	%xmm2, 0x00(%edi)	# load mm2
	jmp		_xmmReadEnd

_xmm3RegRead:
	movdqu	%xmm3, 0x00(%edi)	# load mm3
	jmp		_xmmReadEnd

_xmm4RegRead:
	movdqu	%xmm4, 0x00(%edi)	# load mm4
	jmp		_xmmReadEnd

_xmm5RegRead:
	movdqu	%xmm5, 0x00(%edi)	# load mm5
	jmp		_xmmReadEnd

_xmm6RegRead:
	movdqu	%xmm6, 0x00(%edi)	# load mm6
	jmp		_xmmReadEnd

_xmm7RegRead:
	movdqu	%xmm7, 0x00(%edi)	# load mm7
	jmp		_xmmReadEnd


_xmmReadEnd:
	popal
	leave
	ret 



	.globl _xmmTest 
_xmmTest:

	pushl	%ebp
	movl	%esp, %ebp
	pushal
	
	movl    0x08(%ebp), %eax	# operation type
	movl	0x0C(%ebp), %edi	# pointer to result		

	cmp     $0x00, %eax
	jz		_xmmAdd
	cmp     $0x01, %eax
	jz		_xmmSub
	cmp     $0x02, %eax
	jz		_xmmMul
	
_xmmAdd:
	movl	0x10(%ebp), %esi	# pointer to result	
	movdqu	0x00(%esi), %xmm0	# load mmo
	movl	0x14(%ebp), %esi	# pointer to result
	movdqu	0x00(%esi), %xmm1	# load mm1
	paddb	%xmm0, %xmm1		# byte add 
	movdqu	%xmm1, 0x00(%edi)	# copy dMsb
	jmp		_xmmResult

_xmmSub:
	movl	0x10(%ebp), %esi	# pointer to result	
	movdqu	0x00(%esi), %xmm2	# load mmo
	movl	0x14(%ebp), %esi	# pointer to result
	movdqu	0x00(%esi), %xmm3   # load mm1
	psubb	%xmm2, %xmm3			# byte add 
	movdqu	%xmm3, 0x00(%edi)	# copy dMsb
	jmp		_xmmResult


_xmmMul:
	movl	0x10(%ebp), %esi	# pointer to result	
	movdqu	0x00(%esi), %xmm4	# load mmo
	movl	0x14(%ebp), %esi	# pointer to result
	movdqu	0x00(%esi), %xmm5   # load mm1
	pmullw	%xmm4, %xmm5			# byte add 
	movdqu	%xmm5, 0x00(%edi)	# copy dMsb
	jmp		_xmmResult


_xmmResult:
	popal
	leave
	ret 





	.globl _mmxWrite 
_mmxWrite:
	pushl	%ebp
	movl	%esp, %ebp
	pushal

	movl    0x08(%ebp), %eax	# reg number
	movl	0x0C(%ebp), %edi	# pointer to buffer

	cmp     $0x00, %eax
	jz		_mm0RegWrite
	cmp     $0x01, %eax
	jz		_mm1RegWrite
	cmp     $0x02, %eax
	jz		_mm2RegWrite
	cmp     $0x03, %eax
	jz		_mm3RegWrite
	cmp     $0x04, %eax
	jz		_mm4RegWrite
	cmp     $0x05, %eax
	jz		_mm5RegWrite
	cmp     $0x06, %eax
	jz		_mm6RegWrite
	cmp     $0x07, %eax
	jz		_mm7RegWrite

_mm0RegWrite:
	movq	0x00(%edi), %mm0	# load mm0
	jmp		_mmxWriteEnd

_mm1RegWrite:
	movq	0x00(%edi), %mm1	# load mm1
	jmp		_mmxWriteEnd

_mm2RegWrite:
	movq	0x00(%edi), %mm2	# load mm2
	jmp		_mmxWriteEnd

_mm3RegWrite:
	movq	0x00(%edi), %mm3	# load mm3
	jmp		_mmxWriteEnd

_mm4RegWrite:
	movq	0x00(%edi), %mm4	# load mm4
	jmp		_mmxWriteEnd

_mm5RegWrite:
	movq	0x00(%edi), %mm5	# load mm5
	jmp		_mmxWriteEnd

_mm6RegWrite:
	movq	0x00(%edi), %mm6	# load mm6
	jmp		_mmxWriteEnd

_mm7RegWrite:
	movq	0x00(%edi), %mm7	# load mm7
	jmp		_mmxWriteEnd


_mmxWriteEnd:
	popal
	leave
	ret 



	.globl _mmxRead 
_mmxRead:
	pushl	%ebp
	movl	%esp, %ebp
	pushal

	movl    0x08(%ebp), %eax	# reg number
	movl	0x0C(%ebp), %edi	# pointer to buffer

	cmp     $0x00, %eax
	jz		_mm0RegRead
	cmp     $0x01, %eax
	jz		_mm1RegRead
	cmp     $0x02, %eax
	jz		_mm2RegRead
	cmp     $0x03, %eax
	jz		_mm3RegRead
	cmp     $0x04, %eax
	jz		_mm4RegRead
	cmp     $0x05, %eax
	jz		_mm5RegRead
	cmp     $0x06, %eax
	jz		_mm6RegRead
	cmp     $0x07, %eax
	jz		_mm7RegRead

_mm0RegRead:
	movq	%mm0, 0x00(%edi)	# load mm0
	jmp		_mmxReadEnd

_mm1RegRead:
	movq	%mm1, 0x00(%edi)	# load mm1
	jmp		_mmxReadEnd

_mm2RegRead:
	movq	%mm2, 0x00(%edi)	# load mm2
	jmp		_mmxReadEnd

_mm3RegRead:
	movq	%mm3, 0x00(%edi)	# load mm3
	jmp		_mmxReadEnd

_mm4RegRead:
	movq	%mm4, 0x00(%edi)	# load mm4
	jmp		_mmxReadEnd

_mm5RegRead:
	movq	%mm5, 0x00(%edi)	# load mm5
	jmp		_mmxReadEnd

_mm6RegRead:
	movq	%mm6, 0x00(%edi)	# load mm6
	jmp		_mmxReadEnd

_mm7RegRead:
	movq	%mm7, 0x00(%edi)	# load mm7
	jmp		_mmxReadEnd


_mmxReadEnd:
	popal
	leave
	ret 




	.globl _mmxTest 
_mmxTest:


	pushl	%ebp
	movl	%esp, %ebp
	pushal
	
	movl    0x08(%ebp), %eax	# operation type
	movl	0x0C(%ebp), %edi	# pointer to result	

	cmp     $0x00, %eax
	jz		_mmxAdd
	cmp     $0x01, %eax
	jz		_mmxSub
	cmp     $0x02, %eax
	jz		_mmxMul
	
_mmxAdd:
	movq	0x10(%ebp), %mm0	# load mmo
	movq	0x18(%ebp), %mm1	# load mm1
	paddb	%mm0, %mm1			# byte add 
	movq	%mm1, 0x00(%edi)	# copy dMsb
	jmp		_mmxResult

_mmxSub:
	movq	0x10(%ebp), %mm2	# load mmo
	movq	0x18(%ebp), %mm3	# load mm1
	psubb	%mm2, %mm3			# byte add 
	movq	%mm3, 0x00(%edi)	# copy dMsb
	jmp		_mmxResult


_mmxMul:
	movq	0x10(%ebp), %mm2	# load mmo
	movq	0x18(%ebp), %mm3	# load mm1
	pmullw	%mm2, %mm3			# byte add 
	movq	%mm3, 0x00(%edi)	# copy dMsb
	jmp		_mmxResult


_mmxResult:
	popal
	leave
	ret 




.data

.bss

