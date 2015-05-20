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
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/temperature.c,v 1.1 2014-03-04 10:25:18 mgostling Exp $
 *
 * $Log: temperature.c,v $
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1  2013-09-04 07:13:51  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 */

/*
 * temperature.c
 *
 *  Created on: 18 Mar 2010
 *      Author: engineer
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>

TEMPERATURE_INFO  localTemperature = {
										0x30,
										0x01,
										0xb3
									 };

/*****************************************************************************
 * brdGetTempinfo: returns the TEMPERATURE_INFO global data structure
 * RETURNS: TEMPERATURE_INFO* */
UINT32 brdGetTempinfo(void *ptr)
{
	*((TEMPERATURE_INFO**)ptr) = &localTemperature;

	return E__OK;
}
