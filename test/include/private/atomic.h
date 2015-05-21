#ifndef __atomic_h__
#define __atomic_h__

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

/* atomic.h - atomic operations (atomic.s)
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/atomic.h,v 1.1 2013-09-04 07:40:41 chippisley Exp $
 *
 * $Log: atomic.h,v $
 * Revision 1.1  2013-09-04 07:40:41  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/11/02 13:42:35  chippisley
 * Initial import.
 *
 *
 */

/* includes */
#include <stdtypes.h>


/* externals */
extern UINT32 atomic32Get( UINT32 *address );
extern UINT32 atomic32Set( UINT32 *address, UINT32 value );
extern UINT32 atomic32Add( UINT32 *address, UINT32 value );
extern UINT32 atomic32Sub( UINT32 *address, UINT32 value );
extern UINT32 atomic32Inc( UINT32 *address );
extern UINT32 atomic32Dec( UINT32 *address );
extern UINT32 atomic32And( UINT32 *address, UINT32 value );
extern UINT32 atomic32Nand( UINT32 *address, UINT32 value );
extern UINT32 atomic32Or( UINT32 *address, UINT32 value );
extern UINT32 atomic32Xor( UINT32 *address, UINT32 value );
extern UINT32 atomic32Cas( UINT32 *address, UINT32 oldValue, UINT32 newValue );
 
#endif /* __atomic_h__ */
