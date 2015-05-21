
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

/* hw_init.c - board-specific hardware initialization
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/am90x.x1x/hw_init.c,v 1.1 2015-01-21 17:51:23 gshirasat Exp $
 *
 * $Log: hw_init.c,v $
 * Revision 1.1  2015-01-21 17:51:23  gshirasat
 * Initial import of AM 90x USM port
 *
 * Revision 1.1  2013-09-04 07:13:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/delay.h>
#include <bit/mem.h>
#include <bit/io.h>
 
#include <bit/msr.h>

#include <private/port_io.h>

#include "config.h"
#include "../debug/dbgLog.h"

/* defines */
#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

#define PLX_WIDTH_REG				0x78

#undef INCLUDE_HWINIT_DEBUG

#if defined(INCLUDE_HWINIT_DEBUG) || defined(INCLUDE_DEBUG_VGA)
#warning "***** DEBUG ON *****"
#endif

#ifdef INCLUDE_HWINIT_DEBUG /* sysDebugPrintf */
#define HWINITDBG( x )	do {DBLOG x;} while(0)
#else
#define HWINITDBG( x )
#endif

#define LPC_BRIDGE_DEV	31
#define  ACPI_BASE		0x40		/* ACPI I/O base address config. register */
#define  SMI_EN			0x30		/* SMI Enable */
#define  ALT_GP_SMI_EN	0x38		/* Alternate GPI SMI Enable */

#define UHCI_0_DEV		29
#define  USB_LEGKEY		0xC0		/* Legacy Keyboard and Mouse Control register */
#define  SPECIAL_SMI    0x70

#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)
#define osReadDWord(regAddr)		 (*(UINT32*)(regAddr))

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */
extern void sysDebugPrintf( char *format, ... );

/* forward declarations */

static UINT8 bcdToDecimalInit(UINT8 hex)
{
	UINT8  temp, temp2;

	temp  = hex & 0x0f;
	if(temp > 0x09)
		return 0xff;

	temp2 = (hex >> 4) & 0x0f;
	if(temp2 > 0x09)
		return 0xff;

	temp += (temp2  * 10);

	return temp;
}


static void rtcInit(void)
{
	UINT8 temp;

	sysOutPort8(0x70, 0x0A);
	sysOutPort8 (0x71, 0x26);

	sysOutPort8 (0x70, 0x0D);
	sysOutPort8 (0x71, 0x00);

	sysOutPort8 (0x70, 0x0B);
	sysOutPort8 (0x71, 0x82);


	vDelay(100);

	//seconds
	sysOutPort8 (0x70, 0x00);
	temp = bcdToDecimalInit(sysInPort8(0x71));
	if(temp > 59)
		sysOutPort8 (0x71, 0x00);

	//seconds alarm
	sysOutPort8 (0x70, 0x01);
	sysOutPort8 (0x71, 0x00);

	//minutes
	sysOutPort8 (0x70, 0x02);
	temp = bcdToDecimalInit(sysInPort8(0x71));
	if(temp > 59)
		sysOutPort8 (0x71, 0x00);

	//minutes alarm
	sysOutPort8 (0x70, 0x03);
	sysOutPort8 (0x71, 0x00);

	//hours
	sysOutPort8 (0x70, 0x04);
	temp = bcdToDecimalInit(sysInPort8(0x71));
	if(temp > 23)
		sysOutPort8 (0x71, 0x00);

	//Hours alarm
	sysOutPort8 (0x70, 0x05);
	sysOutPort8 (0x71, 0x00);

	//Day of week
	sysOutPort8 (0x70, 0x06);
	sysOutPort8 (0x71, 0x00);

	//day of month
	sysOutPort8 (0x70, 0x07);
	temp = bcdToDecimalInit(sysInPort8(0x71));
	if(temp > 31)
		sysOutPort8 (0x71, 0x01);

	//month
	sysOutPort8 (0x70, 0x08);
	temp = bcdToDecimalInit(sysInPort8(0x71));
	if(temp > 12)
		sysOutPort8 (0x71, 0x01);

	//year
	sysOutPort8 (0x70, 0x09);
	temp = bcdToDecimalInit(sysInPort8(0x71));
	if(temp > 99)
		sysOutPort8 (0x71, 0x09);

	//century
	sysOutPort8 (0x70, 0x32);
	temp = bcdToDecimalInit(sysInPort8(0x71));
	if(temp != 20)
		sysOutPort8 (0x71, 0x20);


	sysOutPort8 (0x70, 0x0B);
	sysOutPort8 (0x71, 0x02);
}



/*****************************************************************************
 * brdHwInit: do custom hardware initialization
 *
 * RETURNS: none
 */

UINT32 brdHwInit (void *ptr)
{
	PCI_PFA	pfa;
	UINT32	dTemp;
	UINT16	wAcpiBase,wTemp = 0;
	UINT8	bFunc;
	UINT32	Addr;
	PTR48 tPtr1;
	volatile UINT32 mapRegSpace=0;
	UINT32 mHandle;


	HWINITDBG(("%s:\n", __FUNCTION__));

	rtcInit();
	
	
	/* Enable the Root PCIE  port B:D:F = 0:1:0 */
	pfa = PCI_MAKE_PFA (0, 0, 0);
	wTemp = PCI_READ_WORD (pfa, 0x54);
	wTemp |= 0x001B;
	PCI_WRITE_WORD (pfa, 0x54,wTemp);

	/*
	 * Disable SMI sources
	 */

	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wAcpiBase = 0xFFFE & PCI_READ_WORD (pfa, ACPI_BASE);

	/* If BIOS assigned an address to ACPI registers, disable SMI sources */

	if (wAcpiBase != 0)
	{
		/* Disable all possible SMI events */

		dTemp = sysInPort32 (wAcpiBase + SMI_EN);
		sysOutPort32 (wAcpiBase + SMI_EN, dTemp & ~0x080668FF);

		/* Disable GPI generated SMI */
		sysOutPort16 (wAcpiBase + ALT_GP_SMI_EN, 0x0000);
	}


	/*
	 * Disable the legacy keyboard SMI traps (so KBC access works)
	 */
	 
	bFunc = 0;
	pfa = PCI_MAKE_PFA (0, 29, bFunc);
	PCI_WRITE_WORD (pfa, SPECIAL_SMI, 0x00000000);

	pfa = PCI_MAKE_PFA (0, 26, bFunc);
	PCI_WRITE_WORD (pfa, SPECIAL_SMI, 0x00000000);

	for (bFunc = 1; bFunc <= 4; bFunc++)
	{
		pfa = PCI_MAKE_PFA (0, 29, bFunc);
		PCI_WRITE_WORD (pfa, USB_LEGKEY, 0x2f00);

		pfa = PCI_MAKE_PFA (0, 26, bFunc);
		PCI_WRITE_WORD (pfa, USB_LEGKEY, 0x2f00);
	}

	/* Disable NMI*/
	dTemp = sysInPort8(0x70);
	dTemp |= 0x80;
	sysOutPort8(0x70,dTemp);

	pfa = PCI_MAKE_PFA(0,29,0);
	PCI_WRITE_DWORD(pfa,0x6c,0);

	pfa = PCI_MAKE_PFA(0,26,0);
	PCI_WRITE_DWORD(pfa,0x6c,0);
	

	/* Enable/Disable LPC functions */
	pfa = PCI_MAKE_PFA(0,31,0);
	PCI_WRITE_DWORD(pfa,0x90,0x000c03e9);
	Addr = (PCI_READ_DWORD(pfa,0xF0)  & 0xfffffff0);
	mHandle = dGetPhysPtr(Addr,0x4000,&tPtr1,(void*)&mapRegSpace);
	osWriteDWord((mapRegSpace+0x3418), 0x15ea1ff1);
	/* Disable KT and IDE-R */
	osWriteDWord((mapRegSpace+0x3428), 0x0000001E);
	PCI_WRITE_BYTE (pfa, 0x64, 0xD1);
	
	vFreePtr(mHandle);
	

	pfa  = PCI_MAKE_PFA(0,0,0);
	Addr = (PCI_READ_DWORD(pfa,0x54) | 0x00002008);
	PCI_WRITE_DWORD(pfa,0x54,Addr);

	//for speedstep to work
	Addr = (PCI_READ_DWORD(pfa,0x48)  & 0xfffffff0);
	mHandle = dGetPhysPtr(Addr,0x1000,&tPtr1,(void*)&mapRegSpace);
	osWriteDWord((mapRegSpace+0x5da8), 0x00000001);
	vFreePtr(mHandle);

	/* Perform PLX init*/
	pfa = PCI_MAKE_PFA (0, 1, 0);
	//PCI- PCI Bridge
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
    PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x1);
    PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0xe); //A

	PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);
    PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);

	// PLX Switch - upstream
	pfa = PCI_MAKE_PFA (1,0,0);
    PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x1);
    PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x2);
    PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0xE);//A

	PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);
    PCI_WRITE_BYTE (pfa, PCI_COMMAND, 0x07);

	pfa = PCI_MAKE_PFA(2,1,0);

 	PCI_WRITE_BYTE(pfa,PLX_WIDTH_REG,0x10);
	vDelay(100);
	PCI_WRITE_BYTE(pfa,PLX_WIDTH_REG,0x20);
	vDelay(100);

	return E__OK;

} /* brdHwInit () */

/*****************************************************************************
 * brdPostScanInit: adjust mem limits, and anything else that is required after
 * 					the PCI scan etc,  otherwise do nothing
 *
 * RETURNS: none
 */
UINT32 brdPostScanInit(void *ptr)
{
	return E__OK;
}


