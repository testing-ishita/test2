
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/spieeprom.c,v 1.1 2014-03-04 10:25:18 mgostling Exp $
 *
 * $Log: spieeprom.c,v $
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

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>

SPIEEPROM_INFO localSpiEEPROM = {  0x214 };

/*****************************************************************************\
 *
 *  TITLE:  spiBrdEEPROMInfo ()
 *
 *  ABSTRACT:  This function gets the SPI Board EEPROM information, here
 *			    it is the status control register used for SPI communication with
 *				EEPROM.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
UINT32 brdSpiEEPROMInfo (void *ptr)
{
	*((SPIEEPROM_INFO**)ptr) = &localSpiEEPROM;

	return E__OK;
}
		


