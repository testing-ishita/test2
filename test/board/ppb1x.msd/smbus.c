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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/smbus.c,v 1.1 2015-03-17 10:37:55 hchalla Exp $
 *
 * $Log: smbus.c,v $
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
 * Revision 1.1  2013-09-04 07:13:51  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/board_service.h>

SMBUS_INFO  localSMBUS = {
								0,			//Bus
								31,			//Device
								3,			//Function
								0x20,		//GPIOBaseReg
								0x00,		//GPIOBase

								0x05,		//reg1
								0xff,	    //mask1
								0x06,		//reg2
								0xff,	    //mask2
								
							};

DS3905_INFO  localDS3905 = {
								0xAA,
								0xAC,
								0xAE
							};

/*****************************************************************************
 * brdEEPROMinfo: returns the EEPROM INFO global data structure
 * RETURNS: SIO_Info* */
UINT32 brdSMBUSinfo(void *ptr)
{
	PCI_PFA	 pfa;
	UINT16   temp;

	pfa = PCI_MAKE_PFA (localSMBUS.Bus, localSMBUS.Device, localSMBUS.Function);
	localSMBUS.GPIOBase = PCI_READ_DWORD (pfa, localSMBUS.GPIOBaseReg);	
	localSMBUS.GPIOBase &= ~0x00000001;
	temp = PCI_READ_WORD (pfa, 0x04);
	PCI_WRITE_WORD (pfa, 0x04, (temp | 0x01));	
	PCI_WRITE_BYTE (pfa, 0x40, 0x01);	

	*((SMBUS_INFO**)ptr) = &localSMBUS;

	return E__OK;
}



/*****************************************************************************
 * brdEEPROMinfo: returns the EEPROM INFO global data structure
 * RETURNS: SIO_Info* */
UINT32 brdDS3905info(void *ptr)
{
	/*Bit Enable NV2 Switch for DS3905*/
	vIoWriteReg(0x312,REG_8, (dIoReadReg(0x312, REG_8)|0x10));
	*((DS3905_INFO**)ptr) = &localDS3905;

	return E__OK;
}






