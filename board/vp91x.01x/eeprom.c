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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp91x.01x/eeprom.c,v 1.1 2015-01-29 11:08:24 mgostling Exp $
 *
 * $Log: eeprom.c,v $
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.3  2012/08/08 05:41:55  madhukn
 * *** empty log message ***
 *
 * Revision 1.2  2012/08/07 03:25:17  madhukn
 * *** empty log message ***
 *
 * Revision 1.1  2012/06/22 11:41:59  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */
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
								0x04,		//GP_IO_SEL
								0x00010000,	//GP_IO_SEL_BIT
								0x0C,		//GP_LVL
								0x00020000,	//GP_LVL_CBIT
								0x00010000,	//GP_LVL_DBIT
								0xA0,		//ADDR
								16 /*64Kbit 24LC64*/
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










