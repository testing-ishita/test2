
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

/* board_cfg.c - functions reporting board specific architecture
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/switch_eeprom.c,v 1.2 2014-03-20 17:16:40 mgostling Exp $
 *
 * $Log: switch_eeprom.c,v $
 * Revision 1.2  2014-03-20 17:16:40  mgostling
 * Turned off DEBUG messages
 *
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 */

/* includes */

#include <stdio.h>
#include <stdtypes.h>
#include <string.h>

#include <private/port_io.h>
#include <private/sys_delay.h>
#include <private/debug.h>

#include <bit/board_service.h>
#include <bit/delay.h>
#include <bit/mem.h>

/* defines */
//#define DEBUG

#define PCIE_SWITCH_NO_EEPROM_ERROR			(E__BIT + 0x6201)
#define PCIE_SWITCH_EEPROM_RSVD_ERROR		(E__BIT + 0x6202)
#define PCIE_SWITCH_EEPROM_TIMEOUT_ERROR	(E__BIT + 0x6203)

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

/* locals */
static volatile UINT32 baseAddress;

/* forward declarations */
UINT32 dCheckSwitchEeprom(PCI_PFA pfa, UINT8 instance);

/********************************************************************
 * PcieSwitchEepromTest
 *
 * This test will test the board specific GPIO lines.
 ********************************************************************/
TEST_INTERFACE (PcieSwitchEepromTest, "PCIe Switch EEPROM Access Test")
{
	PCI_PFA	pfa;
	UINT32	dStatus;

	dStatus	= E__OK;
	dStatus = iPciFindDeviceById (1, 0x10B5, 0x8733, &pfa);
	if (dStatus == E__OK)
	{
		dStatus = dCheckSwitchEeprom(pfa, 1);
	}
	dStatus = iPciFindDeviceById (7, 0x10B5, 0x8733, &pfa);
	if (dStatus == E__OK)
	{
		dStatus = dCheckSwitchEeprom(pfa, 7);
	}
    return dStatus;
}

UINT32 dCheckSwitchEeprom(PCI_PFA pfa, UINT8 instance)
{
	UINT32	dConfigSpace;
	UINT32	dRegVal;
	UINT32	dHandle;
	PTR48	p1;
	UINT32	dStatus;

	INT8	bTimeout;
#ifdef DEBUG
	char 	achBuffer[64];
#endif

#ifdef DEBUG
	sprintf(achBuffer,"Switch instance %d: pfa=0x%04X\n", instance, pfa);
	sysDebugWriteString(achBuffer);
#endif

	dStatus	= E__OK;
	dConfigSpace = PCIE_READ_DWORD (pfa, 0x0010);			// switch register configuration space
	dHandle = dGetPhysPtr (dConfigSpace, 0x1000, &p1, (void *)(&baseAddress));

#ifdef DEBUG
	sprintf(achBuffer,"Configuration register base address: 0x%08X\n", dConfigSpace);
	sysDebugWriteString(achBuffer);
#endif

	dRegVal = vReadDWord(baseAddress + 0x00000260L);				// switch eeprom command/status register

#ifdef DEBUG
	sprintf(achBuffer, "Extended Read (Device:Vendor) %08X\n", vReadDWord(baseAddress));
	sysDebugWriteString (achBuffer);
	sprintf(achBuffer,"Switch eeprom cmd/status register: 0x%08X\n", dRegVal);
	sysDebugWriteString(achBuffer);
#endif
	switch ((dRegVal & 0x00030000L) >> 16)
	{
		case 0:		// No eeprom found by switch
#ifdef DEBUG
			sysDebugWriteString("**FAIL**:Switch reports no eeprom present\n");
#endif
			dStatus = PCIE_SWITCH_NO_EEPROM_ERROR + ((instance - 1) << 4);
			break;

		case 2:		// error - eeprom status from switch is 'reserved'
#ifdef DEBUG
			sysDebugWriteString("**FAIL**:Switch reports 'reserved' eeprom status\n");
#endif
			dStatus = PCIE_SWITCH_EEPROM_RSVD_ERROR + ((instance - 1) << 4);
			break;

		case 1:		// eeprom found - validation signature verified (initialised)
#ifdef DEBUG
			sysDebugWriteString("Switch reports eeprom found and validation signature verified\n");
#endif
			break;

		case 3:		// eeprom found - validation signature NOT verified (un-initialised)
#ifdef DEBUG
			sysDebugWriteString("Switch reports eeprom found and validation signature NOT verified\n");
#endif
			break;
	}

	if (dStatus == E__OK)
	{
		dRegVal &= 0xFFFF0000L;
		dRegVal |= 0x00006000L;									// read four bytes from eeprom at address 0
		vWriteDWord(baseAddress + 0x00000260L, dRegVal);		// send command

		bTimeout = 10;	// about one second
		vDelay(100);

		dRegVal = vReadDWord(baseAddress + 0x00000260L);		// check for command completion
		while (((dRegVal & 0x00040000L) != 0x00000000L )&& bTimeout > 0)
		{
			vDelay (100);
			dRegVal = vReadDWord(baseAddress + 0x00000260L);	
			--bTimeout;
		}
		
		if (((dRegVal & 0x00040000L) != 0x00000000L ) || bTimeout == 0)
			dStatus = PCIE_SWITCH_EEPROM_TIMEOUT_ERROR + ((instance - 1) << 4);
		else
		{
			dRegVal = vReadDWord(baseAddress + 0x00000264L);	// read eeprom data buffer
			dStatus = E__OK;
		}
	}

#ifdef DEBUG
	sprintf(achBuffer,"Switch eeprom signature dword: 0x%08X\n", dRegVal);
	sysDebugWriteString(achBuffer);
#endif

	vFreePtr (dHandle);			// release memory allocation

	return dStatus;
}
