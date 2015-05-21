#ifndef __string_h__
	#define __string_h__

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

/* string.h - String Manipulation Library
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/string.h,v 1.2 2014-03-13 15:46:12 swilson Exp $
 *
 * $Log: string.h,v $
 * Revision 1.2  2014-03-13 15:46:12  swilson
 * Move definition of size_t from string.h to stdtypes.h, that is where various files assume it will be.
 *
 * Revision 1.1  2013-09-04 07:28:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2009/09/29 12:20:59  swilson
 * Create far-pointer memory access functions
 *
 * Revision 1.3  2008/05/21 13:27:54  swilson
 * Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
 *
 * Revision 1.2  2008/04/25 15:23:51  swilson
 * Clean-up of header.
 *
 * Revision 1.1  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 */


/* includes */

#include <stdtypes.h>


/* defines */

#ifndef NULL
#	define NULL			(0)
#endif

#ifndef MK_PTR48
#
#	define MK_PTR48(sel, ofs)	( ((UINT64)(sel) << 32) | (UINT64)(int)(ofs) )
#
#	define PTR48_SEL(p)			(UINT32)( (p) >> 32 )
#	define PTR48_OFS(p)			(UINT32)(p)
#
#endif

	/* Alternate forms */

#define strcmpi(s1,s2)      stricmp(s1,s2)
#define strncmpi(s1,s2,n)   strnicmp(s1,s2,n)


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

	/* in mem.S */

extern void*	memccpy (void* d, const void* s, int c, size_t n);
extern void*	memmove (void* d, const void* s, size_t n);

extern int		memcmp (const void* s1, const void* s2, size_t n);
extern int		memicmp (const void* s1, const void* s2, size_t n);

extern void*	memchr (const void* s, int c, size_t n);

extern void*	memset (void* d, int c, size_t n);


extern PTR48	_fmemcpy (PTR48 d, const PTR48 s, size_t n);
extern PTR48	_fmemccpy (PTR48 d, const PTR48 s, int c, size_t n);
extern PTR48	_fmemmove (PTR48 d, const PTR48 s, size_t n);

extern int		_fmemcmp (const PTR48 s1, const PTR48 s2, size_t n);
extern int		_fmemicmp (const PTR48 s1, const PTR48 s2, size_t n);

extern PTR48	_fmemchr (const PTR48 s, int c, size_t n);

extern PTR48	_fmemset (PTR48 d, int c, size_t n);

extern void		_fMemWriteByte (PTR48 d, UINT8 b);
extern void		_fMemWriteWord (PTR48 d, UINT16 w);
extern void		_fMemWriteDword (PTR48 d, UINT32 dw);

extern UINT8	_fMemReadByte (PTR48 d);
extern UINT16	_fMemReadWord (PTR48 d);
extern UINT32	_fMemReadDword (PTR48 d);


	/* in string_a.S */

extern size_t	strlen (const char* s);

extern char*	strchr (const char* s, int c);
extern char*	strrchr (const char* s, int c);

extern char*	strcat (char* d, const char* s);
extern char*	strncat (char* d, const char* s, size_t n);

extern int		strcmp (const char* s1, const char* s2);
extern int		strncmp (const char* s1, const char* s2, size_t n);

extern char*	strcpy (char* d, const char* s);
extern char*	strncpy (char* d, const char* s, size_t n);

extern int		stricmp (const char* s1, const char* s2);
extern int		strnicmp (const char* s1, const char* s2, size_t n);

extern char*	strset (char* s, int c);
extern char*	strnset (char* s, int c, size_t n);

extern char*	strlwr (char* s);
extern char*	strupr (char* s);

	/* in string.c */

extern char*	strpbrk (const char* s1, const char* s2);
extern size_t	strspn (const char* s1, const char* s2);
extern char*	strstr (const char* s1, const char* s2);

extern void*	memcpy (void* d, const void* s, size_t n);

	/* in bcopy.s */
extern void bcopy (char *s, char *d, size_t n);


/* forward declarations */


#endif


