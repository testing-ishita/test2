#ifndef __mirrorflash_h__
	#define __mirrorflash_h__

/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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

/* mirrorflash.h - definitions for Spansion Mirror Flash
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/mirrorflash.h,v 1.1 2013-09-04 07:40:27 chippisley Exp $
 *
 * $Log: mirrorflash.h,v $
 * Revision 1.1  2013-09-04 07:40:27  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2009/09/29 12:31:15  swilson
 * Application Flash test for Mirror Flash device.
 *
 */


/* includes */

/* defines */

#define SPANSION_MID		0x01			/* Spansion manuf ID */

#define GL512_SEC_SIZE		0x20000L		/* 64MB = 512 * 128-kByte sectors */
#define GL512_NUM_SEC		512
#define GL512_DID			0x7E2301L		/* part ID (offsets 02:1C:1E) */

	/* Device Commands */

#define F_RESET				0xF0
#define F_IDENT				0x90
#define F_WRITE				0xA0
#define F_ERASE_SETUP	 	0x80
#define F_ERASE				0x30
#define F_CONFIRM			0x29
#define F_STATUS_RESET		0x50
#define F_WRITE_BUFFER		0x25

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


#endif

