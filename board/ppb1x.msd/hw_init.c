
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/hw_init.c,v 1.2 2015-04-01 16:44:29 hchalla Exp $
 *
 * $Log: hw_init.c,v $
 * Revision 1.2  2015-04-01 16:44:29  hchalla
 * Initial release of CUTE firmware 02.02.01.
 *
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
 * Revision 1.2  2013-10-08 07:17:49  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:13:49  chippisley
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
#include <bit/pci.h>
#include <bit/delay.h>
#include <bit/mem.h>
#include <bit/io.h>
 
#include <bit/msr.h>

#include <private/port_io.h>

#include "../debug/dbgLog.h"
#include "../ppb1x.msd/config.h"

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
#define  TOLUD			0xBC


#define UHCI_0_DEV		29
#define  USB_LEGKEY		0xC0		/* Legacy Keyboard and Mouse Control register */
#define  SIRQ_CNTL		0x64		/* SIRQ_CNTL serial IRQ control */
#define  GEN2_DEC		0x88		/* LPC I/F Generic decode range 2 */
#define  GEN4_DEC		0x90		/* LPC I/F Generic decode range 4 */

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
	UINT16	wAcpiBase, wTemp = 0;
	UINT8	bFunc;
	(void) ptr;
	UINT32	Addr;

	PTR48 tPtr1;
	volatile UINT32 mapRegSpace=0;
	UINT32 mHandle;
	UINT16 GPIOAddr=0,wVal=0;


	rtcInit();

	/*
	 * Disable SMI sources
	 */
     /* Enable the Root PCIE  port B:D:F = 0:1:0 */
	pfa = PCI_MAKE_PFA (0, 0, 0);
	wTemp = PCI_READ_WORD (pfa, 0x54);
	wTemp |= 0x001B;
	PCI_WRITE_WORD (pfa, 0x54,wTemp);


	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wAcpiBase = 0xFFFE & PCI_READ_WORD (pfa, ACPI_BASE);

	/* If BIOS assigned an address to ACPI registers, disable SMI sources */

	if (wAcpiBase != 0)
	{
		/* Disable all possible SMI events */

		dTemp = sysInPort32 (wAcpiBase + SMI_EN);
		sysOutPort32 (wAcpiBase + SMI_EN, dTemp & ~0x020668FF);

		/* Disable GPI generated SMI */
		sysOutPort16 (wAcpiBase + ALT_GP_SMI_EN, 0x0000);
	}


	pfa = PCI_MAKE_PFA(0,31,0);
	PCI_WRITE_DWORD(pfa,0x80,0x3f030110);
	PCI_WRITE_DWORD(pfa,0x88,0x00FC03b1);
	PCI_WRITE_DWORD(pfa,0x8C,0x000C0311);
	PCI_WRITE_DWORD(pfa,0x90,0x007C0C81);

	PCI_WRITE_DWORD(pfa,0x98,0xff000001);
	PCI_WRITE_WORD(pfa,0xD8,0xFFCE);

	PCI_WRITE_BYTE (pfa, 0x64, 0xD2);

	/*GPIO 2 Function to use Native Function This is due to interrupt routing for PMC1 site*/
	GPIOAddr = PCI_READ_WORD(pfa,0x48);
	GPIOAddr = GPIOAddr & 0xFFFE;
	wVal = sysInPort16(GPIOAddr+0);
	wVal = (wVal & 0xFFFB);
	sysOutPort16(GPIOAddr+0,wVal);

	// set GPIO16 & 17 as outputs for I2c bit-banging
	dTemp = sysInPort32(GPIOAddr+4);
	dTemp &= 0xFFFCFFFF;
	sysOutPort32(GPIOAddr+4,dTemp);



	/*
	 * Disable the legacy keyboard SMI traps (so KBC access works)
	 */

	for (bFunc = 0; bFunc <= 3; bFunc++)
	{
		pfa = PCI_MAKE_PFA (0, UHCI_0_DEV, bFunc);
		PCI_WRITE_WORD (pfa, USB_LEGKEY, 0x2f00);
	}

	pfa = PCI_MAKE_PFA(0,26,0);
	PCI_WRITE_WORD(pfa,USB_LEGKEY,0x2f00);

	pfa = PCI_MAKE_PFA(0,26,1);
	PCI_WRITE_WORD(pfa,USB_LEGKEY,0x2f00);

	pfa = PCI_MAKE_PFA(0,26,2);
	PCI_WRITE_WORD(pfa,USB_LEGKEY,0x2f00);


	pfa = PCI_MAKE_PFA(0,3,0);
	PCI_WRITE_DWORD(pfa,0x4C,0x3D);

	pfa = PCI_MAKE_PFA(0,LPC_BRIDGE_DEV,0);
	PCI_WRITE_DWORD(pfa,GEN4_DEC,0x007c0c81);					// address decode 4
	Addr = (PCI_READ_DWORD(pfa,0xF0)  & 0xfffffff0);			// root complex base address
	mHandle = dGetPhysPtr(Addr,0x4000,&tPtr1,(void*)&mapRegSpace);

	dTemp = osReadDWord(mapRegSpace+0x3418);					// function disable register
	dTemp = 0x03AA0011;										// turn off unused PCIe Bridge 2
	osWriteDWord((mapRegSpace+0x3418), dTemp);

	dTemp = osReadDWord(mapRegSpace+0x3428);					// function disable 2 register
	dTemp |= 0x0000001D;										// turn off all MEI interfaces
	osWriteDWord((mapRegSpace+0x3428), dTemp);

	PCI_WRITE_BYTE (pfa, SIRQ_CNTL, 0xD2);
	vFreePtr(mHandle);

	pfa  = PCI_MAKE_PFA(0,0,0);
	Addr = (PCI_READ_DWORD(pfa,0x54) | 0x00002008);
	PCI_WRITE_DWORD(pfa,0x54,Addr);

	//for speedstep to work
	Addr = (PCI_READ_DWORD(pfa,0x48)  & 0xfffffff0);
	mHandle = dGetPhysPtr(Addr,0x1000,&tPtr1,(void*)&mapRegSpace);
	osWriteDWord((mapRegSpace+0x5da8), 0x00000001);
	vFreePtr(mHandle);


} /* brdHwInit () */

/*****************************************************************************
 * brdPostScanInit: adjust mem limits, and anything else that is required after
 * 					the PCI scan etc,  otherwise do nothing
 *
 * RETURNS: none
 */
UINT32 brdPostScanInit(void *ptr)
{
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

	return E__OK;
}


