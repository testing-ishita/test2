#ifndef __debug_h__
	#define __debug_h__

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

/* debug.h - include file for debug functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/debug.h,v 1.1 2013-09-04 07:40:42 chippisley Exp $
 *
 * $Log: debug.h,v $
 * Revision 1.1  2013-09-04 07:40:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/03/22 13:40:56  hchalla
 * Added new functions.
 *
 * Revision 1.4  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.3  2008/09/19 10:14:32  swilson
 * Add missing checkin log markup.
 *
 */


/* includes */

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern void sysDebugOpen (void);
extern void sysDebugClose (void);
extern int  sysIsDebugopen (void);
extern void sysDebugPrintf( char *format, ... );
extern void sysDebugWriteString (char* buffer);
extern void sysAPDebugWriteString (char* buffer);
extern void sysDebugWriteChar (char c);
extern void sysAPDebugWriteChar (char c);
extern void sysDebugFlush (void);

/* forward declarations */


#endif

