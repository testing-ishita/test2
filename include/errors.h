#ifndef __errors_h__
	#define __errors_h__

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

/* errors.h - System-wide Error Codes
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/errors.h,v 1.1 2013-09-04 07:28:50 chippisley Exp $
 *
 * $Log: errors.h,v $
 * Revision 1.1  2013-09-04 07:28:50  chippisley
 * Import files into new source repository.
 *
 * Revision 1.12  2010/05/05 15:48:38  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.11  2009/05/20 09:01:38  swilson
 * Add error code base for CPU exceptions.
 *
 * Revision 1.10  2009/05/18 12:35:20  swilson
 * Add codes for use during bootstrap.
 *
 * Revision 1.9  2009/01/22 14:50:27  swilson
 * New error condition
 *
 * Revision 1.8  2008/09/29 10:03:45  swilson
 * New error code. Correction to PCI error root.
 *
 * Revision 1.7  2008/09/17 13:50:00  swilson
 * New runtime error code.
 *
 * Revision 1.6  2008/06/02 10:31:28  swilson
 * Add cctExecuteBit() interface and modify IBIT to use this. Add support for complex
 *  BIT execution expressions and BIT parameters.
 *
 * Revision 1.5  2008/05/12 14:53:17  swilson
 * Add general failure code.
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

#define E__OK			(0)
#define E__FAIL			(0xFFFFFFFF)	/* -1 */

#define E__RUNTIME		(0x01000000)	/* root for RUNTIME error codes */
#define E__HAL			(0x02000000)	/* root for HAL error codes */
#define E__PCI			(0x03000000)	/* root for PCI error codes */
#define E__CUTE			(0x04000000)	/* root for CUTE error codes */
#define E__BIT			(0x80000000)	/* root for BIT error codes */

	/* Runtime Errors */

#define E__1ST_STAGE_CSUM			(E__RUNTIME + 0x00000001)
#define E__2ND_STAGE_CSUM			(E__RUNTIME + 0x00000002)
#define E__2ND_STAGE_LAUNCH			(E__RUNTIME + 0x00000003)

#define E__NO_SERVICE_PROVIDER		(E__RUNTIME + 0x00000010)
#define E__NO_TEST_DIRECTORY		(E__RUNTIME + 0x00000011)
#define E__TEST_NOT_FOUND			(E__RUNTIME + 0x00000012)
#define E__TOO_MANY_TESTS			(E__RUNTIME + 0x00000013)
#define E__TEST_PERMISSIONS			(E__RUNTIME + 0x00000014)

#define E__ABORT_TEST				(E__RUNTIME + 0x00000020)

#define E__BOARD_HANG				(E__RUNTIME + 0x00000030)

#define E__EXCEPTION				(E__RUNTIME + 0x00001000)		/* + exception number 0x00 - 0x1F */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


#endif

