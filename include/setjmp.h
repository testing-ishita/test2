#ifndef __setjmp_h__
	#define __setjmp_h__

/************************************************************************
 *                                                                      *
 *      Copyright 2008 Concurrent Technologies, all rights reserved.    *
 *                                                                      *
 *      The program below is supplied by Concurrent Technologies        *
 *      on the understanding that no responsibility is assumed by       *
 *      Concurrent Technologies for any setjmp contained therein.       *
 *      Furthermore, Concurrent Technologies makes no commitment to     *
 *      update or keep current the program code, and reserves the       *
 *      right to change its specifications at any time.                 *
 *                                                                      *
 *      Concurrent Technologies assumes no responsibility either for    *
 *      the use of this code or for any infringements of the patent     *
 *      or other rights of third parties which may result from its use  *
 *                                                                      *
 ************************************************************************/

/* setjmp.h - execution environment save/restore
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/setjmp.h,v 1.1 2013-09-04 07:28:50 chippisley Exp $
 *
 * $Log: setjmp.h,v $
 * Revision 1.1  2013-09-04 07:28:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2008/09/17 13:53:01  swilson
 * Update saved-register structure.
 *
 * Revision 1.2  2008/05/15 15:32:40  swilson
 * Add build and version tracking. Implement & test missing c-library functions.
 *
 * Revision 1.1  2008/04/25 15:22:23  swilson
 * New C-library modules. Basic structure, little content.
 *
 */


/* includes */

#include <stdtypes.h>

/* defines */

/* typedefs */

typedef struct tagJmpBuf	/* context save buffer */
{
	/* UINT32 used throughout to avoid alignment issues */

					/* DS/ES are constants */
	UINT32	fs;
	UINT32	gs;
					/* EAX used for return */
	UINT32	ebx;
	UINT32	ecx;
	UINT32	edx;

	UINT32	ebp;
	UINT32	esi;
	UINT32	edi;

	UINT32	esp;
	UINT32	eip;

	UINT32	eflags;

} jmp_buf[1];

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */

extern int setjmp (jmp_buf psJmpBuf);
extern void longjmp (jmp_buf psJmpBuf, int retValue);


#endif

