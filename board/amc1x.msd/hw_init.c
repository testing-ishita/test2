
/************************************************************************
 *                                                                      *
 *      Copyright 2014 Concurrent Technologies, all rights reserved.    *
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/amc1x.msd/hw_init.c,v 1.2 2014-10-03 14:47:30 chippisley Exp $
 *
 * $Log: hw_init.c,v $
 * Revision 1.2  2014-10-03 14:47:30  chippisley
 * Disabled 8 Series/C220 Series Chipset Family PCI Express Root Port #5 which was causing PCI scan lockup.
 *
 * Revision 1.1  2014-09-19 10:41:42  mgostling
 * Initial check in to CVS
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

#ifdef INCLUDE_HWINIT_DEBUG /* sysDebugPrintf or DBLOG*/
#define HWINITDBG( x )	do {sysDebugPrintf x;} while(0)
#else
#define HWINITDBG( x )
#endif

#define MCH_BRIDGE_DEV	0
#define  MCH_BAR		0x48
#define  GRAPHICS_CTRL	0x50
#define  DEV_ENABLE		0x54
#define  TOLUD			0xBC

#define LPC_BRIDGE_DEV	31
#define  ACPI_BASE		0x40		/* ACPI I/O base address config. register */
#define  SMI_EN			0x30		/* SMI Enable */
#define  ALT_GP_SMI_EN	0x38		/* Alternate GPI SMI Enable */
#define  SIRQ_CNTL		0x64		/* SIRQ_CNTL serial IRQ control */
#define  LPC_DEC_ENABLE	0x80	/* LPC fixed IO decodes */
#define  GEN1_DEC		0x84		/* LPC I/F Generic decode range 2 */
#define  GEN2_DEC		0x88		/* LPC I/F Generic decode range 2 */
#define  GEN3_DEC		0x8C		/* LPC I/F Generic decode range 2 */
#define  GEN4_DEC		0x90		/* LPC I/F Generic decode range 4 */
#define  USB_LEGKEY		0x94		/* Legacy Keyboard and Mouse Control register */
#define  PCH_RCBA		0xF0		/* Root Complex Base Address */
#define  PCH_RPFN			0x0404	/* PCH root port function number and hide */
#define  PCH_FUNC_DISABLE	0x3418	/* PCH device disable control */
#define  PCH_FUNC_DISABLE_2	0x3428	/* PCH device disable control 2 */

#define EHCI_DEV_1		29
#define EHCI_DEV_2		26
#define  SPECIAL_SMI			0x70
#define  EHCI_EXTENDED_LEGACY	0x6c

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
	UINT32	Addr;
	PTR48 tPtr1;
	volatile UINT32 mapRegSpace = 0;
	UINT32 mHandle;

	HWINITDBG(("%s:\n", __FUNCTION__));
	rtcInit();

	/* Enable the Root PCIE  port B:D:F = 0:1:0 */

	pfa = PCI_MAKE_PFA (0, MCH_BRIDGE_DEV, 0);
	wTemp = PCI_READ_WORD (pfa, DEV_ENABLE);
	wTemp = 0x000F;
	PCI_WRITE_WORD (pfa, DEV_ENABLE, wTemp);			// disable graphics, HDA, CHAP and thermal

	wTemp = 0x0003;
	PCI_WRITE_WORD (pfa, GRAPHICS_CTRL, wTemp);		// disable VGA resource allocation

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
		sysOutPort32 (wAcpiBase + SMI_EN, dTemp & ~0x0C4668FF);

		/* Disable GPI generated SMI */
		sysOutPort16 (wAcpiBase + ALT_GP_SMI_EN, 0x0000);
	}

	PCI_WRITE_DWORD(pfa, LPC_DEC_ENABLE, 0x3f030010);	/* I/O decode ranges (COM1 + COM2) */
	PCI_WRITE_DWORD(pfa, GEN2_DEC, 0x000C0211);			/* LPC I/F Generic Decode Range 2 */
	PCI_WRITE_DWORD(pfa, GEN3_DEC, 0x000C0311);			/* LPC I/F Generic Decode Range 3 */
	PCI_WRITE_DWORD(pfa, GEN4_DEC, 0x00FC0CA1);			/* LPC I/F Generic Decode Range 4 */

	HWINITDBG(("I/O decode range              : 0x%x\n", PCI_READ_DWORD (pfa, 0x80)));
	HWINITDBG(("LPC I/F Generic Decode Range 2: 0x%x\n", PCI_READ_DWORD (pfa, 0x88)));
	HWINITDBG(("LPC I/F Generic Decode Range 3: 0x%x\n", PCI_READ_DWORD (pfa, 0x8c)));
	HWINITDBG(("LPC I/F Generic Decode Range 4: 0x%x\n", PCI_READ_DWORD (pfa, 0x90)));

	PCI_WRITE_BYTE (pfa, SIRQ_CNTL, 0xD2);				/* Serial IRQ Control */
	HWINITDBG(("Serial IRQ Control            : 0x%x\n", PCI_READ_BYTE (pfa, 0x64)));

	/*
	 * Disable the SMI sources
	 */

	pfa = PCI_MAKE_PFA (0, EHCI_DEV_1, 0);
	PCI_WRITE_WORD (pfa, SPECIAL_SMI, 0x00000000);
	PCI_WRITE_DWORD(pfa, EHCI_EXTENDED_LEGACY, 0);

	pfa = PCI_MAKE_PFA (0, EHCI_DEV_2, 0);
	PCI_WRITE_WORD (pfa, SPECIAL_SMI, 0x00000000);
	PCI_WRITE_DWORD(pfa, EHCI_EXTENDED_LEGACY, 0);

	/*
	 * Disable the legacy keyboard SMI traps (so KBC access works)
	 */

	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	PCI_WRITE_WORD (pfa, USB_LEGKEY, 0x8f00);

	/* Disable NMI*/
	dTemp = sysInPort8 (0x70);
	dTemp |= 0x80;
	sysOutPort8 (0x70, dTemp);

	/* Enable/Disable LPC functions */
	Addr = (PCI_READ_DWORD (pfa, PCH_RCBA) & 0xfffffff0); /* Root Complex Base Address */
	HWINITDBG (("Root Complex Base Address     : 0x%x\n", Addr));
	mHandle = dGetPhysPtr (Addr, 0x4000, &tPtr1, (void* )&mapRegSpace);
	
	HWINITDBG (("Function Disable              : 0x%x\n", osReadDWord ((mapRegSpace + PCH_FUNC_DISABLE))));
	HWINITDBG (("Function Disable 2            : 0x%x\n", osReadDWord ((mapRegSpace + PCH_FUNC_DISABLE_2))));

	/* Function Disable */
	//dTemp = 0x09E30011;										// turn off unused PCIe Bridges, USB3, HDA and thermal
	dTemp = 0x09F30011;										// turn off unused PCIe Bridges, USB3, HDA and thermal
	osWriteDWord ((mapRegSpace + PCH_FUNC_DISABLE), dTemp);

	dTemp = 0x76543012;											// renumber PCIe Bridges
	osWriteDWord ((mapRegSpace + PCH_RPFN), dTemp);

	HWINITDBG (("Function Disable              : 0x%x\n", osReadDWord ((mapRegSpace + PCH_FUNC_DISABLE))));

	/* Disable KT, IDE-R and Display Bus (BDF) */
	osWriteDWord ((mapRegSpace + PCH_FUNC_DISABLE_2), 0x0000001C);	/* Function Disable 2 */
	HWINITDBG (("Function Disable 2            : 0x%x\n", osReadDWord ((mapRegSpace + PCH_FUNC_DISABLE_2))));

	vFreePtr (mHandle);


	//for speedstep to work
	pfa  = PCI_MAKE_PFA(0, MCH_BRIDGE_DEV, 0);
	Addr = (PCI_READ_DWORD (pfa, MCH_BAR) & 0xfffffff0);
	mHandle = dGetPhysPtr (Addr, 0x1000, &tPtr1, (void*) &mapRegSpace);
	osWriteDWord ((mapRegSpace + 0x5da8), 0x00000001);
	vFreePtr (mHandle);

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
#if 1
	int		i;
	UINT32	dPciTopOfMem;
	UINT32	msrHigh=0,msrLow=0;
	PCI_PFA	pfa;
#ifdef INCLUDE_HWINIT_DEBUG
	UINT16	prev11, prev12, prev21, prev22;
#endif

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

//	dPciTopOfMem &= 0xf8000000;

	pfa = PCI_MAKE_PFA (0, 0, 0);

#ifdef INCLUDE_HWINIT_DEBUG
	prev11 = PCI_READ_DWORD (pfa, TOLUD);	
#endif

	PCI_WRITE_WORD (pfa, TOLUD, dPciTopOfMem);

#ifdef INCLUDE_HWINIT_DEBUG
	prev12 = PCI_READ_DWORD (pfa, TOLUD);
#endif

	HWINITDBG(("prev11: 0x%x prev12: 0x%x\n", prev11, prev12));
/*
	pfa = PCI_MAKE_PFA (0xff, 0, 1);

#ifdef INCLUDE_HWINIT_DEBUG
	prev21 = PCI_READ_WORD (pfa, 0x80);
#endif

	PCI_WRITE_WORD (pfa, 0x80, (UINT16)(((dPciTopOfMem>>4) & 0xff08) | 3 ));

#ifdef INCLUDE_HWINIT_DEBUG
	prev22 = PCI_READ_WORD (pfa, 0x80);
#endif

	HWINITDBG(("prev21: 0x%x prev22: 0x%x\n", prev21, prev22));
*/
#endif

	return E__OK;
}


