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
 * $Header:
 * $Log: 
 * Revision
 * board-specific superio support.
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
								0x04,		//GP_IO_SEL
								0x00000002,	//GP_IO_SEL_BIT
								0x0c,		//GP_LVL
								0x00000080,	//GP_LVL_CBIT
								0x00000002,	//GP_LVL_DBIT
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
	localEEPROM.GPIOBase &= ~0x00000001;

	*((EEPROM_INFO**)ptr)= &localEEPROM;

	return E__OK;
}










