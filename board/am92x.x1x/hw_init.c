
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/am92x.x1x/hw_init.c,v 1.1 2013-09-04 07:13:28 chippisley Exp $
 *
 * $Log: hw_init.c,v $
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
	(void)ptr;
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


	pfa = PCI_MAKE_PFA(0,31,0);
	PCI_WRITE_DWORD(pfa,0x80,0x3f030010);	/* I/O decode range */
	PCI_WRITE_DWORD(pfa,0x88,0x007c0601);	/* LPC I/F Generic Decode Range 2 */
	PCI_WRITE_DWORD(pfa,0x8C,0x000C0311);	/* LPC I/F Generic Decode Range 3 */
	PCI_WRITE_DWORD(pfa,0x90,0x007C0C81);	/* LPC I/F Generic Decode Range 4 */
	//PCI_WRITE_DWORD(pfa,0x90,0x000c03e9);	/* LPC I/F Generic Decode Range 4 */
	
	HWINITDBG(("I/O decode range              : 0x%x\n", PCI_READ_DWORD(pfa,0x80)));
	HWINITDBG(("LPC I/F Generic Decode Range 2: 0x%x\n", PCI_READ_DWORD(pfa,0x88)));
	HWINITDBG(("LPC I/F Generic Decode Range 3: 0x%x\n", PCI_READ_DWORD(pfa,0x8c)));
	HWINITDBG(("LPC I/F Generic Decode Range 4: 0x%x\n", PCI_READ_DWORD(pfa,0x90)));
	
	// PCI_WRITE_BYTE (pfa, 0x64, 0xD2);		/* Serial IRQ Control */
	PCI_WRITE_BYTE (pfa, 0x64, 0xD1);		/* Serial IRQ Control */
	HWINITDBG(("Serial IRQ Control            : 0x%x\n", PCI_READ_BYTE(pfa,0x64)));

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
	Addr = (PCI_READ_DWORD(pfa,0xF0)  & 0xfffffff0); /* Root Complex Base Address */
	HWINITDBG(("Root Complex Base Address     : 0x%x\n", Addr));
	mHandle = dGetPhysPtr(Addr,0x4000,&tPtr1,(void*)&mapRegSpace);
	
	// osWriteDWord((mapRegSpace+0x3418), 0x15ea1ff1); 
	osWriteDWord((mapRegSpace+0x3418), 0x1dff1ff3); /* Function Disable */
	HWINITDBG(("Function Disable              : 0x%x\n", osReadDWord((mapRegSpace+0x3418))));
	
#ifdef INCLUDE_DEBUG_VGA
	/* Disable KT and IDE-R, leave Display Bus (BDF) enabled */	
	osWriteDWord((mapRegSpace+0x3428), 0x0000001f);	/* Function Disable 2 */
#else
	/* Disable KT, IDE-R and Display Bus (BDF) */
	osWriteDWord((mapRegSpace+0x3428), 0x0000001E);	/* Function Disable 2 */
#endif
	HWINITDBG(("Function Disable 2            : 0x%x\n", osReadDWord((mapRegSpace+0x3428))));
	
	vFreePtr(mHandle);
	

	pfa  = PCI_MAKE_PFA(0,0,0);
	Addr = (PCI_READ_DWORD(pfa,0x54) | 0x00002008);
	PCI_WRITE_DWORD(pfa,0x54,Addr);

	//for speedstep to work
	Addr = (PCI_READ_DWORD(pfa,0x48)  & 0xfffffff0);
	mHandle = dGetPhysPtr(Addr,0x1000,&tPtr1,(void*)&mapRegSpace);
	osWriteDWord((mapRegSpace+0x5da8), 0x00000001);
	vFreePtr(mHandle);

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
#if 0
	int		i;
	UINT32	dPciTopOfMem;
	PCI_PFA	pfa;
	UINT16	prev11, prev12, prev21, prev22;
	UINT32 msrHigh=0,msrLow=0;

	(void)ptr;
	
	HWINITDBG(("%s:\n", __FUNCTION__));
	
	for(i = 0x200; i <= 0x20f; i++)
	{
		vReadMsr( i, &msrHigh, &msrLow );
		HWINITDBG(("MSR 0x%x: 0x%x 0x%x\n", i, msrHigh, msrLow));
		vWriteMsr( i, 0, 0 );
	}

	vWriteMsr (0x200, 0, 0x00000006);
	vWriteMsr (0x201, 0x0000000f, 0x80000800);

	vWriteMsr (0x202, 0, 0xffe00005);
	vWriteMsr (0x203, 0x0000000f, 0xffe00800);

	dPciTopOfMem = dPciGetTopOfMem(); /* get lowest PCI memory used */
	
	HWINITDBG(("dPciTopOfMem: 0x%x\n", dPciTopOfMem));

	dPciTopOfMem &= 0xf8000000;
	dPciTopOfMem = dPciTopOfMem >> 16;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	prev11 = PCI_READ_WORD (pfa, 0xb0);
	PCI_WRITE_WORD (pfa, 0xb0, (UINT16)dPciTopOfMem);
	prev12 = PCI_READ_WORD (pfa, 0xb0);
	HWINITDBG(("prev11: 0x%x prev12: 0x%x\n", prev11, prev12));

	pfa = PCI_MAKE_PFA (0xff, 0, 1);
	prev21 = PCI_READ_WORD (pfa, 0x80);
	PCI_WRITE_WORD (pfa, 0x80, (UINT16)(((dPciTopOfMem>>4) & 0xff08) | 3 ));
	prev22 = PCI_READ_WORD (pfa, 0x80);
	HWINITDBG(("prev21: 0x%x prev22: 0x%x\n", prev21, prev22));
#endif
	return E__OK;
}


