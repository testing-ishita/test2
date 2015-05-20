
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/hw_init.c,v 1.1 2014-03-04 10:25:18 mgostling Exp $
 *
 * $Log: hw_init.c,v $
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.2  2013-12-10 12:01:17  mgostling
 * Removed unused variable.
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2  2011-08-26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
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
#define  SIRQ_CNTL		0x64		/* SIRQ_CNTL serial IRQ control */
#define  GEN2_DEC		0x88		/* LPC I/F Generic decode range 2 */
#define  GEN4_DEC		0x90		/* LPC I/F Generic decode range 4 */
#define  USB_LEGKEY		0x94		/* Legacy Keyboard and Mouse Control register */

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
		sysOutPort8 (0x71, 0x13);

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
	UINT16	wAcpiBase = 0;
	(void)	ptr;
	UINT32	Addr;

	PTR48 tPtr1;
	volatile UINT32 mapRegSpace=0;
	UINT32 mHandle;


	rtcInit();

	/*
	 * Disable SMI sources
	 */

	/* If BIOS assigned an address to ACPI registers, disable SMI sources */

	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wAcpiBase = 0xFFFE & PCI_READ_WORD (pfa, ACPI_BASE);

	if (wAcpiBase != 0)
	{
		/* Disable all possible SMI events */

		dTemp = sysInPort32 (wAcpiBase + SMI_EN);
		sysOutPort32 (wAcpiBase + SMI_EN, dTemp & ~0x020668FF);

		/* Disable GPI generated SMI */
		sysOutPort16 (wAcpiBase + ALT_GP_SMI_EN, 0x0000);
	}


	pfa = PCI_MAKE_PFA(0,LPC_BRIDGE_DEV,0);
	PCI_WRITE_DWORD(pfa,GEN4_DEC,0x000c03e9);					// address decode 4
	Addr = (PCI_READ_DWORD(pfa,0xF0)  & 0xfffffff0);			// root complex base address
	mHandle = dGetPhysPtr(Addr,0x4000,&tPtr1,(void*)&mapRegSpace);

	dTemp = osReadDWord(mapRegSpace+0x3418);					// function disable register
	dTemp |= 0x00040000;										// turn off unused PCIe Bridge 2
	osWriteDWord((mapRegSpace+0x3418), dTemp);

	dTemp = osReadDWord(mapRegSpace+0x3428);					// function disable 2 register
	dTemp |= 0x0000001E;										// turn off all MEI interfaces
	osWriteDWord((mapRegSpace+0x3428), dTemp);

	PCI_WRITE_BYTE (pfa, SIRQ_CNTL, 0xD2);
	vFreePtr(mHandle);

	/*
	 * Disable the legacy keyboard SMI traps (so KBC access works)
	 */

	pfa = PCI_MAKE_PFA(0,LPC_BRIDGE_DEV,0);
	PCI_WRITE_WORD(pfa,USB_LEGKEY,0x8f00);

    /*
     *  Initialize the TPM Module
     */
    sysOutPort8 (0x4e, 0x55);

    sysOutPort8 (0x4e, 0x60); /* Set IO BASE address to 0x3b0 */
    sysOutPort8 (0x4f, 0x03);

    sysOutPort8 (0x4e, 0x61);
    sysOutPort8 (0x4f, 0xb0);


    sysOutPort8 (0x4e, 0x70); /*Set IRQ to 6 */
    sysOutPort8 (0x4f, 0x06);

    sysOutPort8 (0x4e, 0x38); /*Set Sync Error Enable */
    sysOutPort8 (0x4f, 0x01);

    sysOutPort8 (0x4e, 0x30); /*Activate the TPM Device */
    sysOutPort8 (0x4f, 0x01);

    sysOutPort8 (0x4e, 0xaa);
	
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
#ifdef INCLUDE_HWINIT_DEBUG
	UINT16	prev11, prev12, prev21, prev22;
#endif
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
	
#ifdef INCLUDE_HWINIT_DEBUG
	prev11 = PCI_READ_WORD (pfa, 0xb0);
#endif

	PCI_WRITE_WORD (pfa, 0xb0, (UINT16)dPciTopOfMem);

#ifdef INCLUDE_HWINIT_DEBUG
	prev12 = PCI_READ_WORD (pfa, 0xb0);
#endif
	
	HWINITDBG(("prev11: 0x%x prev12: 0x%x\n", prev11, prev12));

	pfa = PCI_MAKE_PFA (0xff, 0, 1);
	
#ifdef INCLUDE_HWINIT_DEBUG
	prev21 = PCI_READ_WORD (pfa, 0x80);
#endif

	PCI_WRITE_WORD (pfa, 0x80, (UINT16)(((dPciTopOfMem>>4) & 0xff08) | 3 ));
	
#ifdef INCLUDE_HWINIT_DEBUG
	prev22 = PCI_READ_WORD (pfa, 0x80);
#endif

	HWINITDBG(("prev21: 0x%x prev22: 0x%x\n", prev21, prev22));

#endif  // #if 0

	return E__OK;
}


