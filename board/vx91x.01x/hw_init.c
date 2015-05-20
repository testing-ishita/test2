
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx91x.01x/hw_init.c,v 1.1 2014-05-15 09:42:03 jthiru Exp $
 *
 * $Log: hw_init.c,v $
 * Revision 1.1  2014-05-15 09:42:03  jthiru
 * Adding vx91x board sources
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.3  2011/06/07 15:04:34  hchalla
 * Initial version of PBIT for VX813.
 *
 * Revision 1.2  2011/03/22 13:25:11  hchalla
 * Fixed NMI and Speed step problems by setting SERRIRQ_CTL num clock cycles  to 6, for speed stepping magic bit in MCHBAR.
 *
 * Revision 1.1  2011/02/28 11:57:43  hmuneer
 * no message
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
#define  SPECIAL_SMI    0x70

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
#if 0
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
	bFunc = 0;
	pfa = PCI_MAKE_PFA (0, 29, bFunc);
	PCI_WRITE_WORD (pfa, SPECIAL_SMI, 0x00000000);

	pfa = PCI_MAKE_PFA (0, 26, bFunc);
	PCI_WRITE_WORD (pfa, SPECIAL_SMI, 0x00000000);


#if 0
	for (bFunc = 1; bFunc <= 4; bFunc++)
	{
		pfa = PCI_MAKE_PFA (0, 29, bFunc);
		PCI_WRITE_WORD (pfa, USB_LEGKEY, 0x2f00);

		pfa = PCI_MAKE_PFA (0, 26, bFunc);
		PCI_WRITE_WORD (pfa, USB_LEGKEY, 0x2f00);
	}
#endif

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
	PCI_WRITE_BYTE (pfa, 0x64, 0xD2);
	vFreePtr(mHandle);


	pfa  = PCI_MAKE_PFA(0,0,0);
	Addr = (PCI_READ_DWORD(pfa,0x54) | 0x00002008);
	PCI_WRITE_DWORD(pfa,0x54,Addr);

	//for speedstep to work
	Addr = (PCI_READ_DWORD(pfa,0x48)  & 0xfffffff0);
	mHandle = dGetPhysPtr(Addr,0x1000,&tPtr1,(void*)&mapRegSpace);
	osWriteDWord((mapRegSpace+0x5da8), 0x00000001);
	vFreePtr(mHandle);
#endif
#if 1
        PCI_PFA pfa;
        UINT32  dTemp;
        UINT16  wAcpiBase;
        UINT8   bFunc;
        UINT32  Addr, dRcba, mHandle1;

        PTR48 tPtr1, ptr1;
        volatile UINT32 mapRegSpace=0;
        volatile UINT32 mapRegSpace1 = 0;
        UINT32 mHandle;
        (void)ptr;

        rtcInit();

        UINT8 buffer[64];
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

        pfa = PCI_MAKE_PFA(0, 31, 0);
        dRcba = PCI_READ_DWORD(pfa, 0xF0);
        if(dRcba & 1)
          dRcba = dRcba & 0xFFFFC000;
        else
        {
            return E__FAIL;
        }
        /* RCBA registers in chipset configuration */
        /* Disable 0,22,2; 0,22,3; 0,30,0; enable 0,31,6 */
        mHandle1 = dGetPhysPtr(dRcba, 0x4000, &ptr1, (void*)&mapRegSpace1);
        osWriteDWord((mapRegSpace1 + 0x3428),
            (osReadDWord(mapRegSpace1 + 0x3428) | 0x18));
        osWriteDWord((mapRegSpace1 + 0x3418),
            (osReadDWord(mapRegSpace1 + 0x3418) & ~(1<<4)));

        /*
         * Disable the legacy keyboard SMI traps (so KBC access works)
         */
        bFunc = 0;
        pfa = PCI_MAKE_PFA (0, 29, bFunc);
        PCI_WRITE_WORD (pfa, SPECIAL_SMI, 0x00000000);

        pfa = PCI_MAKE_PFA (0, 26, bFunc);
        PCI_WRITE_WORD (pfa, SPECIAL_SMI, 0x00000000);


#if 0
        for (bFunc = 1; bFunc <= 4; bFunc++)
        {
                pfa = PCI_MAKE_PFA (0, 29, bFunc);
                PCI_WRITE_WORD (pfa, USB_LEGKEY, 0x2f00);

                pfa = PCI_MAKE_PFA (0, 26, bFunc);
                PCI_WRITE_WORD (pfa, USB_LEGKEY, 0x2f00);
        }
#endif

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
        PCI_WRITE_BYTE (pfa, 0x64, 0xD2);
        vFreePtr(mHandle);


        pfa  = PCI_MAKE_PFA(0,0,0);
        Addr = (PCI_READ_DWORD(pfa,0x54) | 0x00002008);
        PCI_WRITE_DWORD(pfa,0x54,Addr);

        //for speedstep to work
        Addr = (PCI_READ_DWORD(pfa,0x48)  & 0xfffffff0);
        mHandle = dGetPhysPtr(Addr,0x1000,&tPtr1,(void*)&mapRegSpace);
        osWriteDWord((mapRegSpace+0x5da8), 0x00000001);
        vFreePtr(mHandle);
        vFreePtr(mHandle1);
#endif
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
