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

/* spimram.c - board-specific SPI RAM Chip GPIO information  used for GPIO bit-bang
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/trb1x.msd/spimram.c,v 1.1 2013-11-25 14:21:34 mgostling Exp $
 * $Log: spimram.c,v $
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.1  2011-08-02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
 *
 */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>

#include <bit/board_service.h>

typedef enum
{
	SPI=0,
	I2C
} eSpiProtocol;

typedef enum
{
	GPIO=0,
	CPLD,
	FPGA
} eSpiInterface;


SPI_MRAM_INFO  localMRAM = {
								0,		//Bus
								31,		//Device
								0,		//Function
								71,     //SpiCs
								70,     //SpiClk
								69,     //MISO
								68,     //MOSI
								0x00,	//ADDR-I2C
								0x48,	//GPIOBaseReg
								GPIO,
								SPI,	//Interface
								0x00,	//GPIOBase
								REG_32
							};


/*****************************************************************************
 * brdEEPROMinfo: returns the EEPROM INFO global data structure
 * RETURNS: SIO_Info*
 * */
UINT32 brdMRAMinfo(void *ptr)
{
	PCI_PFA	 pfa;

	pfa = PCI_MAKE_PFA (localMRAM.Bus, localMRAM.Device, localMRAM.Function);
	localMRAM.GPIOBase = PCI_READ_DWORD (pfa, localMRAM.GPIOBaseReg);
	localMRAM.GPIOBase &= 0xFFFFFF80;

	*((SPI_MRAM_INFO**)ptr)= &localMRAM;

	return E__OK;
}
