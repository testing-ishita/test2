
/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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



