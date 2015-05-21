
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

/* HEADER.S - Linkage for CBIT and RIBIT
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/loader/header.S,v 1.2 2013-11-11 15:15:11 mgostling Exp $
 *
 * $Log: header.S,v $
 * Revision 1.2  2013-11-11 15:15:11  mgostling
 * Updated _s2_signature variable name.
 *
 * Revision 1.1  2013-09-04 07:42:08  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2009/04/07 14:34:58  swilson
 * *** empty log message ***
 *
 * Revision 1.2  2008/07/24 14:42:14  swilson
 * Add missing parameter comment.
 *
 * Revision 1.1  2008/07/18 12:45:15  swilson
 * OS Discovery header.
 *
 */


.text

/*
 * Signature for OS discovery
 */

	.align	4

osSignature:
	.byte	'C'			/* unique signature */
	.byte	'C'
	.byte	'T'
	.byte	'_'
	.byte	'B'
	.byte	'I'
	.byte	'T'
	.byte	'+'

	.long	_s2_signature		/* 2nd Stage start address */
	.long	_cctExecuteBit		/* test execution entry point */
	.long	_pTestDirectory		/* test list */


/****************************************************************************/

.data


/****************************************************************************/

.bss

