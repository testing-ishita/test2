
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp717.x8x/hw_init.c,v 1.1 2015-04-02 11:41:18 mgostling Exp $
 *
 * $Log: hw_init.c,v $
 * Revision 1.1  2015-04-02 11:41:18  mgostling
 * Initial check-in to CVS
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.5  2011/10/27 15:48:10  hmuneer
 * no message
 *
 * Revision 1.4  2011/01/20 10:01:23  hmuneer
 * CA01A151
 *
 * Revision 1.3  2010/09/10 10:57:17  cdobson
 * Added code for setting TOLUD register.
 *
 * Revision 1.2  2010/09/10 09:50:10  cdobson
 * Added brdPostScanInit to set memory cacheing.
 *
 * Revision 1.1  2010/06/23 10:49:08  hmuneer
 * CUTE V1.01
 *
 * Revision 1.1  2010/01/19 12:03:15  hmuneer
 * vx511 v1.0
 *
 * Revision 1.1  2009/09/29 12:16:45  swilson
 * Create VP417 Board
 *
 * Revision 1.3  2009/05/19 08:09:53  cvsuser
 * no message
 *
 * Revision 1.2  2008/09/19 14:53:34  swilson
 * Add code to disable legacy keyboard and mouse support by UHCI USB controller; this
 *  ensures we can directly access the keyboard controller hardware.
 *
 * Revision 1.1  2008/07/24 14:54:47  swilson
 * Board-specific hardware initialization.
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
#include <bit/board_service.h>

#include <private/port_io.h>


/* defines */

#define LPC_BRIDGE_DEV	31
#define  ACPI_BASE		0x40		/* ACPI I/O base address config. register */
#define  SMI_EN			0x30		/* SMI Enable */
#define  ALT_GP_SMI_EN	0x38		/* Alternate GPI SMI Enable */

#define UHCI_0_DEV		29
#define  USB_LEGKEY		0xC0		/* Legacy Keyboard and Mouse Control register */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */



/*****************************************************************************
 * brdHwInit: do custom hardware initialization
 *
 * RETURNS: none
 */

#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)
#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))

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



UINT32 brdHwInit (void *ptr)
{
	PCI_PFA	pfa;
	UINT32	dTemp;
	UINT16	wAcpiBase;
	UINT8	bFunc;
	UINT32	Addr;

	PTR48 tPtr1;
	volatile UINT32 mapRegSpace=0;
	UINT32 mHandle;
	(void)ptr;

	rtcInit();


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


	pfa = PCI_MAKE_PFA(0,31,0);
	PCI_WRITE_DWORD(pfa,0x90,0x000c03e9);
	Addr = (PCI_READ_DWORD(pfa,0xF0)  & 0xfffffff0);
	mHandle = dGetPhysPtr(Addr,0x4000,&tPtr1,(void*)&mapRegSpace);
	osWriteDWord((mapRegSpace+0x3418), 0x15ea1ff1);
	osWriteDWord((mapRegSpace+0x3428), 0x00000005);

	pfa  = PCI_MAKE_PFA(0,0,0);
	Addr = (PCI_READ_DWORD(pfa,0x54) | 0x00002008);
	PCI_WRITE_DWORD(pfa,0x54,Addr);

	/*Make sure to disable DTS and C state support to avoid any SMIs*/
	asm volatile( 	"movl $0x19B,%%ecx\n\t"			\
					"rdmsr \n\t"					\
					"andl $0xff000000,%%eax \n\t"	\
					"wrmsr \n\t"					\

					"movl $0xE2,%%ecx\n\t"			\
					"rdmsr\n\t"						\
					"andl $0xfc000000,%%eax \n\t"	\
					"wrmsr \n\t"

					"movl $0x1FC,%%ecx\n\t"			\
					"rdmsr\n\t"						\
					"andl $0xfffffffC,%%eax \n\t"	\
					"wrmsr \n\t"					\
					: /*No outputs*/				\
					: /*No inputs*/					\
					: "%eax","%ebx","%ecx"			\
				);


	//enable super I/O Access
	vIoWriteReg(0x2e, REG_8, 0x55);

	//Select watchdog
	vIoWriteReg(0x2e, REG_8, 0x07);
	vIoWriteReg(0x2f, REG_8, 0x0A);

	//Set I/O address for PMC_BLOCK
	vIoWriteReg(0x2e, REG_8, 0x60);
	vIoWriteReg(0x2f, REG_8, 0x0C);

	//Select COM3
	vIoWriteReg(0x2e, REG_8, 0x07);
	vIoWriteReg(0x2f, REG_8, 0x0b);

	//Set interrupt to int 11
	vIoWriteReg(0x2e, REG_8, 0x70);
	vIoWriteReg(0x2f, REG_8, 0x0b);

	//disable super I/O Access
	vIoWriteReg(0x2e, REG_8, 0xAA);

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
	int		i;
	UINT32	dPciTopOfMem;
	PCI_PFA	pfa;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	UINT16	prev11, prev12, prev21, prev22;
#pragma GCC diagnostic pop

	(void)ptr;

	for(i = 0x200; i <= 0x20f; i++)
		vWriteMsr (i, 0, 0);

	vWriteMsr (0x200, 0, 0x00000006);
	vWriteMsr (0x201, 0x0000000f, 0x80000800);

	vWriteMsr (0x202, 0, 0xffe00005);
	vWriteMsr (0x203, 0x0000000f, 0xffe00800);

	dPciTopOfMem = dPciGetTopOfMem(); /* get lowest PCI memory used */

	dPciTopOfMem &= 0xf8000000;
	dPciTopOfMem = dPciTopOfMem >> 16;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	prev11 = PCI_READ_WORD (pfa, 0xb0);
	PCI_WRITE_WORD (pfa, 0xb0, (UINT16)dPciTopOfMem);
	prev12 = PCI_READ_WORD (pfa, 0xb0);

	pfa = PCI_MAKE_PFA (0xff, 0, 1);
	prev21 = PCI_READ_WORD (pfa, 0x80);
	PCI_WRITE_WORD (pfa, 0x80, (UINT16)(((dPciTopOfMem>>4) & 0xff08) | 3 ));
	prev22 = PCI_READ_WORD (pfa, 0x80);

	return E__OK;
}

