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

/* superio.c - board-specific superio information 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/tr803.39x/eeprom.c,v 1.1 2015-01-29 11:07:37 mgostling Exp $
 * $Log: eeprom.c,v $
 * Revision 1.1  2015-01-29 11:07:37  mgostling
 * Import files into new source repository.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011/08/26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>

EEPROM_INFO  localEEPROM = {
								0,			//Bus
								31,			//Device
								0,			//Function
								0x48,		//GPIOBaseReg
								0x00,		//GPIOBase
								0x34,		//GP_IO_SEL
								0x00004000,	//GP_IO_SEL_BIT
								0x38,		//GP_LVL
								0x02000000,	//GP_LVL_CBIT
								0x00004000,	//GP_LVL_DBIT
								0xA0		//ADDR
							};


/*****************************************************************************
 * brdEEPROMinfo: returns the EEPROM INFO global data structure
 * RETURNS: SIO_Info*
 * */
UINT32 brdEEPROMinfo(void *ptr)
{
	PCI_PFA	 pfa;

	pfa = PCI_MAKE_PFA (localEEPROM.Bus, localEEPROM.Device, localEEPROM.Function);
	localEEPROM.GPIOBase = PCI_READ_DWORD (pfa, localEEPROM.GPIOBaseReg);	
	localEEPROM.GPIOBase &= 0xFFFFFF80;

	*((EEPROM_INFO**)ptr)= &localEEPROM;

	return E__OK;
}










