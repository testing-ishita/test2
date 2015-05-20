/************************************************************************
 *                                                                      *
 *      Copyright 2012 Concurrent Technologies, all rights reserved.    *
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

/* atomic.s - atomic operations
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/atomic.s,v 1.1 2013-09-04 07:41:28 chippisley Exp $
 *
 * $Log: atomic.s,v $
 * Revision 1.1  2013-09-04 07:41:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/11/02 13:41:04  chippisley
 * Initial import.
 *
 *
 */


.include "asm/i386.h"

.text

/*******************************************************************************
 * atomic32Get: atomically get content at target address
 *
 * RETURNS: value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Get( UINT32 *address )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 ******************************************************************************/
.globl _atomic32Get
_atomic32Get:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl  0x8(%ebp),%edx  	/* get address */
	movl (%edx), %eax		/* read value, simply with ordinary load */
	
	leave
	ret


/*******************************************************************************
 * atomic32Set: atomically set content at target address to value
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Set( UINT32 *address, UINT32 value )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 *	[EBP+0C] value to set
 ******************************************************************************/
.globl _atomic32Set
_atomic32Set:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl  0x8(%ebp),%edx	/* get address */
	movl  0xc(%ebp),%eax  	/* get value to set */
               				/* xchg locks bus */
	xchg  %eax,(%edx)   	/* swap 'clear' value with bus lock */

	leave
	ret


/*******************************************************************************
 * atomic32Add: atomically add value to content at target address
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Add( UINT32 *address, UINT32 value )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 *	[EBP+0C] value to add
 ******************************************************************************/
.globl _atomic32Add
_atomic32Add:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl  0x8(%ebp),%edx  	/* get address */
	movl  0xc(%ebp),%eax  	/* get value to add */

	lock			  		/* lock the bus during the next instruction */
	xadd  %eax,(%edx)   	/* exclusively add value */
	
	leave
	ret


/*******************************************************************************
 * atomic32Sub: atomically subtract value from content at target address
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Sub( UINT32 *address, UINT32 value )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 *	[EBP+0C] value to subtract
 ******************************************************************************/
.globl _atomic32Sub
_atomic32Sub:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl  0x8(%ebp),%edx  /* get address */
	movl  0xc(%ebp),%eax  /* get value to subtract */
	
	negl  %eax				/* negate %eax */
	lock					/* lock the bus during the next instruction */
	xadd  %eax,(%edx)		/* exclusively add value */
	
	leave
	ret
	

/*******************************************************************************
 * atomic32Inc: atomically increment content at target address
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Inc( UINT32 *address )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 ******************************************************************************/
.globl _atomic32Inc
_atomic32Inc:
	pushl	%ebp
	movl	%esp, %ebp
		
	movl  0x8(%ebp),%edx	/* get address */
	movl  $0x1,%eax			/* increment by 1 */

	lock                	/* lock the bus during the next instruction */
	xadd  %eax,(%edx)		/* exclusively add value */
	
	leave
	ret
	

/*******************************************************************************
 * atomic32Dec: atomically decremant content at target address
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Dec( UINT32 *address )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 ******************************************************************************/
.globl _atomic32Dec
_atomic32Dec:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl  0x8(%ebp),%edx  	/* get address */
	movl  $0x1,%eax     	/* decrement by 1 */
	negl  %eax		  		/* negate %eax */

	lock					/* lock the bus during the next instruction */
	xadd  %eax,(%edx)		/* exclusively add value */

	leave
	ret


/*******************************************************************************
 * atomic32And: atomically AND content at target address with value
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32And( UINT32 *address, UINT32 value )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 *	[EBP+0C] value to XOR
 ******************************************************************************/
.globl _atomic32And
_atomic32And:
	pushl	%ebp
	movl	%esp, %ebp

	movl  0x8(%ebp),%edx  	/* get address */
	movl  (%edx),%eax   	/* get old value */

atomic32And_retry:

	movl  0xc(%ebp),%ecx  	/* get value to AND */
	andl  %eax, %ecx
	lock			  		/* lock the bus during the next instruction */
	cmpxchg  %ecx,(%edx)	/* exchange */
	jnz atomic32And_retry	/* retry if not successful */

	leave
	ret	


/*******************************************************************************
 * atomic32Nand: atomically NAND content at target address with value
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Nand( UINT32 *address, UINT32 value )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 *	[EBP+0C] value to XOR
 ******************************************************************************/
.globl _atomic32Nand
_atomic32Nand:
	pushl	%ebp
	movl	%esp, %ebp

	movl  0x8(%ebp),%edx  	/* get address */
	movl  (%edx),%eax   	/* get old value */

atomic32Nand_retry:

	movl  0xc(%ebp),%ecx  	/* get value to NAND */
	andl  %eax, %ecx
    not   %ecx
	lock			  		/* lock the bus during the next instruction */
	cmpxchg  %ecx,(%edx)	/* exchange */
	jnz atomic32Nand_retry	/* retry if not successful */
	
	leave
	ret	
		

/*******************************************************************************
 * atomic32Or: atomically OR content at target address with value
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Or( UINT32 *address, UINT32 value )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 *	[EBP+0C] value to OR
 ******************************************************************************/
.globl _atomic32Or
_atomic32Or:
	pushl	%ebp
	movl	%esp, %ebp
	
	movl  0x8(%ebp),%edx  	/* get address */
	movl  (%edx),%eax   	/* get old value */

atomicOr_retry:
	movl  0xc(%ebp),%ecx  	/* get value to OR */
	orl   %eax, %ecx
	lock			  		/* lock the bus during the next instruction */
	cmpxchg  %ecx,(%edx)	/* exchange */
	jnz atomicOr_retry		/* retry if not successful */
	
	leave
	ret


/*******************************************************************************
 * atomic32Xor: atomically XOR content at target address with value
 *
 * RETURNS: Old value at target address
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Xor( UINT32 *address, UINT32 value )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 *	[EBP+0C] value to XOR
 ******************************************************************************/
.globl _atomic32Xor
_atomic32Xor:
	pushl	%ebp
	movl	%esp, %ebp

	movl  0x8(%ebp),%edx  	/* get address */
	movl  (%edx),%eax   	/* get old value */

atomic32Xor_retry:

	movl  0xc(%ebp),%ecx  	/* get value to XOR */
	xorl  %eax, %ecx
	lock			  		/* lock the bus during the next instruction */
	cmpxchg  %ecx,(%edx)	/* exchange */
	jnz atomic32Xor_retry	/* retry if not successful */
	
	leave
	ret


/*******************************************************************************
 * atomic32Cas: atomically performs a compare and swap. It tests memory location 
 *              <*address> contains <oldValue> and if it does, sets the value of 
 *              <*address> to <newValue>. 
 *
 * RETURNS: 1 if the swap is executed else 0.
 *
 * PROTOTYPE:
 *
 *	UINT32 atomic32Cas( UINT32 *address, UINT32 oldValue, UINT32 newValue )
 *
 * STACK:
 *
 *	[EBP+00] old [EBP]
 *	[EBP+04] Return Address
 * 	[EBP+08] target address
 *	[EBP+0C] old value
 *	[EBP+10] new value
 ******************************************************************************/
.globl _atomic32Cas
_atomic32Cas:
	pushl	%ebp
	movl	%esp, %ebp

	movl  0x8(%ebp),%edx	/* get address */
	movl  0xc(%ebp),%eax 	/* get old_value to cmp */
	movl  0x10(%ebp),%ecx	/* get new_value to set */
	
	lock					/* lock the Bus during the next instruction */
	cmpxchg %ecx,(%edx)		/* if (%eax == (%edx)         */
							/*   {ZF = 1; (%edx) = %ecx;} */
							/* else                       */
							/*   {ZF = 0; %eax = (%edx);} */
	jne atomic32Cas1
	movl $1, %eax			/* set return to 1 */
	leave
	ret

atomic32Cas1:
	movl $0, %eax			/* set return to 0 */
	leave
	ret


.bss

