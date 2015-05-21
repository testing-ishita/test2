#ifndef __bit_h__
	#define __bit_h__

/************************************************************************
 *                                                                      *
 *      Copyright 2008 Concurrent Technologies, all rights reserved.    *
 *                                                                      *
 *      The program below is supplied by Concurrent Technologies        *
 *      on the understanding that no responsibility is assumed by       *
 *      Concurrent Technologies for any errors contained therein.       *
 *      Furthermore, Concurrent Technologies makes no commitment to     *
 *      update or keep current the program code, and reserves the       *
 *      right to change its specificatbitns at any time.                 *
 *                                                                      *
 *      Concurrent Technologies assumes no responsibility either for    *
 *      the use of this code or for any infringements of the patent     *
 *      or other rights of third parties which may result from its use  *
 *                                                                      *
 ************************************************************************/

/* bit.h - I/O port access functbitns (bit_bit.c)
 */

/* Versbitn control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/bit.h,v 1.1 2013-09-04 07:35:26 chippisley Exp $
 *
 * $Log: bit.h,v $
 * Revision 1.1  2013-09-04 07:35:26  chippisley
 * Import files into new source repository.
 *
 * Revision 1.13  2012/09/14 15:38:45  jthiru
 * CUTE Support for VX91X - C5043B1110, fixes to existing bugs
 *
 * Revision 1.12  2011/05/16 14:31:06  hmuneer
 * Info Passing Support
 *
 * Revision 1.11  2011/01/20 10:01:25  hmuneer
 * CA01A151
 *
 * Revision 1.10  2010/09/15 12:17:39  hchalla
 * Added New flags for TR 501 BIT/CUTE
 *
 * Revision 1.9  2010/07/26 13:14:10  hmuneer
 * Source Cleanup
 *
 * Revision 1.8  2010/06/23 10:59:21  hmuneer
 * CUTE V1.01
 *
 * Revision 1.7  2010/05/05 15:48:16  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.6  2009/05/21 12:48:53  swilson
 * Revise PBIT duration flags in the light of actual measurements of test times.
 *
 * Revision 1.5  2008/09/29 10:02:56  swilson
 * Revise and extend startup flags passed to PBIT/IBIT handler. Corrections to handling
 *  of PBIT flags. Add test duration to BIT structure.
 *
 * Revision 1.4  2008/07/18 12:23:56  swilson
 * Add resource clean-up function.
 *
 * Revision 1.3  2008/06/20 12:16:58  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.2  2008/06/02 10:31:32  swilson
 * Add cctExecuteBit() interface and modify IBIT to use this. Add support for complex
 *  BIT execution expressions and BIT parameters.
 *
 * Revision 1.1  2008/05/16 14:36:24  swilson
 * Add test list management and test execution capabilities.
 *
 * Revisbitn 1.1  2008/04/24 16:04:38  swilson
 * Updated includes to support HAL and BIT library.
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/hal.h>


/* defines */

	/* Startup Flags - from dStartupFlags */

#define  SF__NO_DEBUG		0x00		/* debug mode */
#define  SF__DEBUG			0x01

#define  SF__IBIT			0x00		/* test handler */
#define  SF__PBIT			0x01

#define  SF__BIT			0x00		/* test handler CUTE MODE*/
#define  SF__CUTE			0x01

#define  SF__BSP			0x00		/* CPU mode */
#define  SF__AP				0x01

#define  SF__PBIT_10S		0x00		/* PBIT test duration */
#define  SF__PBIT_30S		0x01
#define  SF__PBIT_60S		0x02
#define  SF__PBIT_TEST		0x03

#define  SF__FAIL_BIT		0x00		/* PBIT failure action */
#define  SF__FAIL_BIOS		0x01
#define  SF__FAIL_OS		0x02
#define  SF__FAIL_IBIT		0x03


/* Switch Settings*/

#define SW__USER			0x00
#define SW__TEST			0x01

#define	FWID_BASIC		0		/* minimum detail */
#define	FWID_FULL		1		/* maximum detail */

#define	FWMODE_BIT		0		/* minimum detail */
#define	FWMODE_CUTE		1		/* maximum detail */

	/* Test Handler definitions */

#define	TH__REMOTE			0x00		/* Remote Interactive or Continuous BIT */
#define	TH__PBIT			0x01		/* Power-on BIT */
#define	TH__LBIT			0x02		/* Local Interactive BIT */
#define	TH__STH				0x04		/* Power-on CUTE(STH)                   */
#define	TH__MTH				0x08		/* Local Interactive CUTE (MTH)         */

	/* Bitfields used in test list/directory */

#define BITF__EXE_PBIT_10S	0x00000001	/* 10 second PBIT selection mask */
#define BITF__EXE_PBIT_30S	0x00000003	/* 30 second PBIT selection mask */
#define BITF__EXE_PBIT_60S	0x00000007	/* 60 second PBIT selection mask */
#define BITF__EXE_PBIT_TEST	0x0000000F	/* testing - forced fail */

#define BITF__PBIT_10S		0x00000001	/* suitable for use with >= 10 second PBIT */
#define BITF__PBIT_30S		0x00000002	/* suitable for use with >= 30 second PBIT */
#define BITF__PBIT_60S		0x00000004	/* suitable for use with >= 60 second PBIT */
#define BITF__PBIT_TEST		0x00000008	/* suitable for use with 'test-mode' PBIT */

#define BITF__LBIT			0x00000100	/* suitable for use with Local IBIT */
#define BITF__RBIT			0x00000200	/* suitable for use with Remote IBIT */
#define BITF__CBIT			0x00000400	/* suitable for use with Local CBIT */
#define BITF__CUTE			0x00000800	/* suitable for use with CUTE --> This is STH -- Anonymous */
#define BITF__COOP			0x00001000	/* suitable for use with CUTE        */

#define BITF__NO_WDOG		0x00001000	/* don't run the watchdog for this test */

	/* Test interface management macros */

#define TEST_INTERFACE(fn, name)	const char ach##fn [] = name; 	\
									UINT32 d##fn (UINT32* adTestParams)

#define EXTERN_INTERFACE(fn)		extern char ach##fn [];		\
									extern UINT32 d##fn (UINT32* adTestParams);

#define TEST_NAME(fn)				ach##fn, 	\
									&d##fn

/* Enumerators for various mutexes*/
#define MAX_CUTEBIT_MUTEX			7

enum {
	MUTEX_DEBUG_STR_WRITE=0,
	MUTEX_DEBUG_CHAR_WRITE,
	MUTEX_DEBUG_FLUSH_STR,
	MUTEX_CONSOLE_PRINT,
	MUTEX_IOACCESS,
	MUTEX_MALLOC,
	MUTEX_STDIO
};


	/* in bit_exec.c */

extern UINT32 cctExecuteBit (UINT16 wTestNum, UINT32* adTestParams, 
								UINT32 (*pfServiceProvider)(SYS_SRVC, void*));

extern void vResourceCleanup (void);

	/* in bit_support.c */

extern UINT8 bStartupDebugMode (void);
extern void   bForceStartupDebugMode(void);
extern UINT8 bStartupTestHandler (void);
extern UINT8 bStartupCUTEMode (void);
extern UINT8 bStartupCpuMode (void);
extern UINT8 bStartupPbitMode (void);
extern UINT8 bStartupFailAction (void);

extern UINT8 bGetTestHandler (void);
extern UINT32 dGetPbitSelectionMask (void);
extern void vDebugWrite (char* achMessage);
extern UINT8  bGetBiosInfoPresent 	(void);
extern UINT8  bGetBaudRate 			(void);

/* forward declarations */


#endif

