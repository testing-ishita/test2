#ifndef __stdtypes_h__
	#define __stdtypes_h__

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

/* stdtypes.h - standard type definitions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/stdtypes.h,v 1.3 2014-03-13 15:46:12 swilson Exp $
 *
 * $Log: stdtypes.h,v $
 * Revision 1.3  2014-03-13 15:46:12  swilson
 * Move definition of size_t from string.h to stdtypes.h, that is where various files assume it will be.
 *
 * Revision 1.2  2014-01-29 13:17:27  hchalla
 * Added datatype variable dma_addr_t.
 *
 * Revision 1.1  2013-09-04 07:28:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2011/03/22 13:41:39  hchalla
 * Added new data types.
 *
 * Revision 1.7  2010/09/15 12:20:50  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.6  2008/05/21 13:27:54  swilson
 * Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
 *
 * Revision 1.5  2008/05/12 14:54:28  swilson
 * Add usefull LENGTH macro.
 *
 * Revision 1.4  2008/04/30 07:51:19  swilson
 * Add PCI support library.
 *
 * Revision 1.3  2008/04/25 15:23:51  swilson
 * Clean-up of header.
 *
 * Revision 1.2  2008/04/24 16:04:38  swilson
 * Updated includes to support HAL and BIT library.
 *
 * Revision 1.1  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 */


/* includes */

/* defines */

#ifndef NULL
#	define NULL	(0)
#endif

#define LENGTH(x)	(sizeof(x) / sizeof((x)[0]))

#ifndef _SIZE_T
	#define _SIZE_T

typedef unsigned int	size_t;

#endif

/* typedefs */

typedef unsigned char		UINT8;		/* 8-bit unsigned */
typedef char		         INT8;		/* 8-bit unsigned */
typedef unsigned short int	UINT16;		/* 16-bit unsigned */
typedef int		             INT32;		/* 32-bit unsigned */
typedef unsigned int		UINT32;		/* 32-bit unsigned */
typedef unsigned long long	UINT64;		/* 64-bit unsigned */

typedef unsigned long long	PTR48;		/* 16:32 format pointer (selector:offset) */

typedef UINT32 dma_addr_t;

typedef enum tagRegSize		/* register size */
{
	REG_8	= 1,	/* 8-bit register */
	REG_16, 		/* 16-bit register */
	REG_32			/* 32-bit register */
} REG_SIZE;

#define TRUE  1
#define FALSE 0

typedef void (*VOIDFUNCPTR) ();      /* pointer to func returning void */
typedef int (*FUNCPTR) ();      	/* pointer to func returning integer */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


#endif

