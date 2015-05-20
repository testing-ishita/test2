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

/*
 * ahciTest.c
 *
 */
 
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/ahciTest.c,v 1.6 2015-03-05 11:47:42 hchalla Exp $
 *
 * $Log: ahciTest.c,v $
 * Revision 1.6  2015-03-05 11:47:42  hchalla
 * Added debug prints.
 *
 * Revision 1.5  2014-04-22 16:39:03  swilson
 * Fix test name AHCI not Ahci.  NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.4  2014-03-06 13:48:19  hchalla
 * re-checkedin due to windows formatting.
 *
 * Revision 1.3  2014-03-04 08:55:18  mgostling
 * Ensure IRQ allocated.
 * Do not use sysInterruptEnable & sysInterruptDisable functions.
 *
 * Revision 1.2  2014-01-10 09:39:27  mgostling
 * Rationalised test parameter handling.
 *
 * Revision 1.1  2013-11-25 14:27:22  mgostling
 * New AHCI tests.
 *
 * Revision 1.9  2013-10-25 14:59:32  mgostling
 * Move function prototypes of static routines out of header file
 *
 * Revision 1.8  2013-10-15 11:05:00  mgostling
 * Fix enableLynxPointAhci() when SATA already in AHCI mode.
 * Release memory handles after use.
 * Remove multiple achBuffers.
 * Reduce output when debugging disabled.
 *
 * Revision 1.7  2013-10-03 14:01:53  mgostling
 * Removed additional debugging messages
 *
 * Revision 1.6  2013-10-03 13:40:52  mgostling
 * Disabled DEBUG definition
 *
 * Revision 1.5  2013-10-03 11:45:26  mgostling
 * Fixed a typo that caused an assignment rather than the required test.
 *
 * Revision 1.4  2013-09-27 12:46:11  mgostling
 * Tidied up restoration of LynxPoint IDEoperation on exit from tests.
 *
 * Revision 1.3  2013-09-06 08:48:54  mgostling
 * Added CVS keywords
 *
 *
 *  Created on: 10 Jul 2013
 *      Author: rupadhyay
 */

/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>		/* bit/errors.h ? or BIT_ERROR macro */

#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/delay.h>
#include <bit/mem.h>
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/console.h>

#include <bit/msr.h>
#include <bit/board_service.h>
#include <devices/ram.h>
#include <private/port_io.h>
#include <devices/ahci.h>

//#define  DEBUG

static   UINT8   UserSuppliedInstance = 0;
static   UINT8   UserSuppliedPort     = 0;
volatile UINT8   ahciInt              = 0;
static 	 UINT8	 bLegacyInt			  = 0;
static 	 UINT8   bAhciAbarAlloc       = 0;
static 	 UINT8   bLynxPointIde        = 0;
static 	 UINT32	 bLegacyAbar		  = 0;

static   UINT32  PortBars[32];
static   UINT32  Abar;
static   UINT32  AbarPtr;
static   UINT32  dHandle1, dHandle2, dHandle3, dHandle4;
static   UINT32  ptrCmdList  = CMDLIST_BAR; // 0x10000000;
static   UINT32  ptrCmdTable = CMDTBL_BAR ; // 0x10001000;
static   UINT32  ptrRcvdFis  = RCVDFIS_BAR; // 0x10008000;
static   UINT32  ptrDataFis  = PHYSICAL_DATA_XFER_LOCATION; // 0x1000_A000

char    achBuffer[128]; // = {0};

// Forward declarations

static void    AhciInitParams(UINT32* adTestParams);
static int     InterruptTestAhciPfa(UINT8 Bus, UINT8 Device, UINT8 Function);
static int     BasicTestAhciPfa(UINT8 Bus, UINT8 Device, UINT8 Function);
static int     DriveAttachTestAhciPfa(UINT8 Bus, UINT8 Device, UINT8 Function);
static void    PopulatePortBars(UINT32 pi);
static int     PrintPortStatus(void);

static int     EnableAhci(void);
static int     EnableAhciWithoutInterrupt(void);
static int     DisableAhci(void);
static UINT16  wSwap (UINT16 wData);
static void    GetDriveName(char* buffer);
static void    LoadIdentifyCommand(FisRegH2d* cmd_fis);
static void    LoadReadSectorCommand(FisRegH2d* cmd_fis);
static int     SetupAhciPort(int PortNo);
static int     FreeAllocations(void);
static void    ResetGlobals(void);
static int     AllocateAhciBuffers (void);
static void    AhciIntHandler0 (void);

static int     enableLynxPointAhci (void);
static void    disableLynxPointAhci (void);

#ifdef DEBUG
static void    HexDump(UINT8* buffer);
static void    DumpSector(void);
#endif

/*****************************************************************************\
 *
 *  TITLE:  ResetGlobals
 *
 *  ABSTRACT: Initialise AHCI globals
 *
\*****************************************************************************/
static void   ResetGlobals(void)
{
	memset(PortBars, 0, sizeof(PortBars));

	Abar     = 0;
	AbarPtr  = 0;
	dHandle1 = 0;
	dHandle2 = 0;
	dHandle3 = 0;
	dHandle4 = 0;

	ahciInt = 0;

	ptrCmdList  = CMDLIST_BAR; // 0x10000000;
	ptrCmdTable = CMDTBL_BAR ; // 0x10001000;
	ptrRcvdFis  = RCVDFIS_BAR; // 0x10008000;
	ptrDataFis  = PHYSICAL_DATA_XFER_LOCATION; // 0x1000_A000
}

/*****************************************************************************\
 *
 *  TITLE:  AhciInterruptTest
 *
 *  ABSTRACT:
 *
 * 	RETURNS: E__OK.
 *
\*****************************************************************************/
TEST_INTERFACE (AhciInterruptTest, "AHCI Interrupt Test")
{

	PCI_PFA   pfa;
	UINT32	  dFound;
	int       InstanceBus;
	int       InstanceDev;
	int       InstanceFunc;
	int       Status = 0;

	ahciInt = 0;
	AhciInitParams(adTestParams);

	Status = enableLynxPointAhci();
	if (Status != E__OK)
		return Status;

#ifdef DEBUG
	sprintf(achBuffer, "UserSuppliedInstance: %x adTestParams[0]:%x, adTestParams[1]:%x, adTestParams[2]:%x\n", UserSuppliedInstance,
						adTestParams[0],adTestParams[1],adTestParams[2]);
	vConsoleWrite(achBuffer);
#endif
//	dFound = iPciFindDeviceById(UserSuppliedInstance, MARVELL_VID, MARVELL_AHCI_DEVID, &pfa);

	// SATA controller
	dFound = iPciFindDeviceByClass(UserSuppliedInstance, 0x01, 0x06, &pfa);

	if (dFound == E__DEVICE_NOT_FOUND) return E__PFA_DOESNT_EXIST;

	ahciInt = 0;

	InstanceBus  = PCI_PFA_BUS(pfa);
	InstanceDev  = PCI_PFA_DEV(pfa);
	InstanceFunc = PCI_PFA_FUNC(pfa);

    #ifdef DEBUG
	sprintf(achBuffer, "Instance found at: %x/%x/%x\n", InstanceBus, InstanceDev, InstanceFunc);
	vConsoleWrite(achBuffer);
    #endif

	Status = InterruptTestAhciPfa(InstanceBus, InstanceDev, InstanceFunc);

	disableLynxPointAhci();
	
	//vDelay(1000);

	if(ahciInt == 1)
	{
	   sprintf(achBuffer, "AhciInterrupt received.\n");
	   vConsoleWrite(achBuffer);
	}
	else
	{
	   sprintf(achBuffer, "Error: No AhciInterrupts received.\n");
	   vConsoleWrite(achBuffer);
	}

	return Status;
}

/*****************************************************************************\
 *
 *  TITLE:  AhciDriveAttachTest
 *
 *  ABSTRACT:
 *
 * 	RETURNS: E__OK.
 *
\*****************************************************************************/
TEST_INTERFACE (AhciDriveAttachTest, "AHCI Drive Presence Test")
{

	PCI_PFA   pfa;
	UINT32	  dFound;
	int       InstanceBus;
	int       InstanceDev;
	int       InstanceFunc;
	UINT32    Status = 0;

	AhciInitParams(adTestParams);

	Status = enableLynxPointAhci();
	if (Status != E__OK)
		return Status;

#ifdef DEBUG
	sprintf(achBuffer, "UserSuppliedInstance: %x adTestParams[0]:%x, adTestParams[1]:%x, adTestParams[2]:%x\n", UserSuppliedInstance,
						adTestParams[0],adTestParams[1],adTestParams[2]);
	vConsoleWrite(achBuffer);
#endif

	// SATA controller
	dFound = iPciFindDeviceByClass(UserSuppliedInstance, 0x01, 0x06, &pfa);

	if (dFound == E__DEVICE_NOT_FOUND) return E__PFA_DOESNT_EXIST;

	ahciInt = 0;

	InstanceBus  = PCI_PFA_BUS(pfa);
	InstanceDev  = PCI_PFA_DEV(pfa);
	InstanceFunc = PCI_PFA_FUNC(pfa);

    #ifdef DEBUG
	sprintf(achBuffer, "Instance found at: %x/%x/%x\n", InstanceBus, InstanceDev, InstanceFunc);
	vConsoleWrite(achBuffer);
    #endif

	Status = DriveAttachTestAhciPfa(InstanceBus, InstanceDev, InstanceFunc);

	disableLynxPointAhci();

	return Status;
}


/*****************************************************************************\
 *
 *  TITLE:  AhciRegisterTest
 *
 *  ABSTRACT:
 *
 * 	RETURNS: E__OK.
 *
\*****************************************************************************/
TEST_INTERFACE (AhciRegisterTest, "AHCI Register Access Test")
{

	PCI_PFA   pfa;
	UINT32	  dFound;
	int       InstanceBus;
	int       InstanceDev;
	int       InstanceFunc;
	int       Status = 0;

	AhciInitParams(adTestParams);

	Status = enableLynxPointAhci();
	if (Status != E__OK)
		return Status;

#ifdef DEBUG
	sprintf(achBuffer, "UserSuppliedInstance: %x adTestParams[0]:%x, adTestParams[1]:%x, adTestParams[2]:%x\n", UserSuppliedInstance,
						adTestParams[0],adTestParams[1],adTestParams[2]);
	vConsoleWrite(achBuffer);
#endif
	// SATA controller
	dFound = iPciFindDeviceByClass(UserSuppliedInstance, 0x01, 0x06, &pfa);

	if (dFound == E__DEVICE_NOT_FOUND) 
		return E__PFA_DOESNT_EXIST;

	ahciInt = 0;

	InstanceBus  = PCI_PFA_BUS(pfa);
	InstanceDev  = PCI_PFA_DEV(pfa);
	InstanceFunc = PCI_PFA_FUNC(pfa);

    #ifdef DEBUG
	sprintf(achBuffer, "Instance found at: %x/%x/%x\n", InstanceBus, InstanceDev, InstanceFunc);
	vConsoleWrite(achBuffer);
    #endif

	Status = BasicTestAhciPfa(InstanceBus, InstanceDev, InstanceFunc);

	disableLynxPointAhci();

	return Status;
}

/*****************************************************************************\
 *
 *  TITLE:  AhciInitParams
 *
 *  ABSTRACT:
 *           Initialise parameters for required AHCI controller and port
 *
 * 	RETURNS: None.
 *
\*****************************************************************************/
static void AhciInitParams(UINT32* adTestParams)
{
	// default to first port on first AHCI controller
	UserSuppliedInstance = 0;
	UserSuppliedPort = 0;

	// check for user input
	if (adTestParams[0] > 0)
	{
		UserSuppliedInstance = adTestParams[1];
		if (adTestParams[0] == 2)
			UserSuppliedPort = adTestParams[2];
	}
	
	// If no parameters are entered, select the first AHCI controller instance
	// and test the first port on it.
	// FIX IT
	if (UserSuppliedInstance == 0)
		UserSuppliedInstance = 1;

	if (UserSuppliedPort == 0)
		UserSuppliedPort = 1;
}

static void AhciIntHandler0 (void)
{
   ahciInt = 1;
   *((UINT32*)(AbarPtr + PortBars[UserSuppliedPort - 1] + 0x10))  |= BIT0; // Write BIT-0 to clear the interrupt.
   *((UINT32*)(AbarPtr + 0x08)) |= 1 << (UserSuppliedPort - 1);
}


static int BasicTestAhciPfa(UINT8 Bus, UINT8 Device, UINT8 Function)
{
	PCI_PFA	    pfa;
	UINT16	    VID, DID;
	UINT8       ControllerMode = 0;
	UINT32      dHandle;
	PTR48       p1;
	UINT32      temp    = 0;
#ifdef DEBUG
	UINT32      ports_implemented = 0;
#endif

	ResetGlobals();
	memset(PortBars, 0, sizeof(PortBars));
	Abar    = 0;
	AbarPtr = 0;

	pfa = PCI_MAKE_PFA (Bus, Device, Function);
	VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
	DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

	ControllerMode = PCI_READ_BYTE (pfa, 0x0A);

	// Check to see if a valid PCI device exists at the PFA?
	if ( (VID == 0xFFFF) || (VID == 0x0000) ||
		 (DID == 0xFFFF) || (DID == 0x0000)
	   )
	{
		return E__PFA_DOESNT_EXIST;
	}
	
    #ifdef DEBUG
	vConsoleWrite ("Valid PFA exists.\n");
    #endif

	// Check to see if we are in AHCI mode?
	if (ControllerMode != 0x06)
	{
		vConsoleWrite ("SataCtrlr NOT in AHCI mode.\n");
		return E__NOT_IN_AHCI_MODE;
	}


    #ifdef DEBUG
	vConsoleWrite ("SataCtrlr in AHCI mode.\n");
    #endif

	Abar = PCI_READ_DWORD(pfa, 0x24);

    #ifdef DEBUG
	sprintf(achBuffer, "Abar = 0x%x\n", Abar);
	vConsoleWrite (achBuffer);
    #endif

	/*
	 * Panther Point EDS 14.1.16
	 * Note:
	 * The ABAR register must be set to a value of 0001_0000h or greater.
	 */
	if (Abar < 0x100000)
	{
		vConsoleWrite ("ABAR is not correctly assigned.\n");
		return E__GARBAGE_ABAR;
	}

	temp  = PCI_READ_DWORD(pfa, 0x04);
	temp |= 6;   // Enable memory and bus_master
	PCI_WRITE_DWORD(pfa, 0x04, temp);

	dHandle = dGetPhysPtr (Abar, 0x800, &p1, &AbarPtr);
	temp    = osReadDWord((AbarPtr + AHCI_HBA_CAP));

    #ifdef DEBUG
	sprintf(achBuffer, "Total ports = %d and CmdSlots = %d\n", (temp & 0x1F) + 1, ((temp >> 8) & 0x1F) + 1);
	vConsoleWrite (achBuffer);
    #endif

    #ifdef DEBUG
	ports_implemented = osReadDWord((AbarPtr + AHCI_PORTS_IMPL));
	sprintf(achBuffer, "ports_implemented = 0x%x\n", ports_implemented);
	vConsoleWrite (achBuffer);
    #endif

	if (!EnableAhciWithoutInterrupt())
	{
		vConsoleWrite ("Error setting AHCI_EN bit in controller.\n");
		return E__ERROR_ENABLING_AHCI;
	}
#ifdef DEBUG   
	else
	{
		vConsoleWrite ("AHCI mode entered.\n");
	}
#endif

	DisableAhci();
	vFreePtr(dHandle);

	return E__OK;
}

static int InterruptTestAhciPfa(UINT8 Bus, UINT8 Device, UINT8 Function)
{
	PCI_PFA	    pfa;
	UINT16	    VID, DID;
	UINT8       ControllerMode = 0;
	UINT32      dHandle;
	PTR48       p1;
	UINT32      temp    = 0;
	UINT32      ports_implemented = 0;
	UINT32      TestAhciPfaStatus = 0;
	UINT8 		bTmp;
	int		  	iVector = -1;
	UINT8	  	bIrq = 16;				 // Hard-coded for PP91X, as no alternate mechanism exists.
	
	static CmdHeader    ch_0;
	static CmdTable     cmd_table;
    static PrdEntry     prd_entry;
	
	memset((void *)&ch_0,      0 , sizeof(ch_0));
	memset((void *)&cmd_table, 0 , sizeof(cmd_table));
	memset((void *)&prd_entry, 0 , sizeof(prd_entry));

    ResetGlobals();
	memset(PortBars, 0, sizeof(PortBars));
	Abar    = 0;
	AbarPtr = 0;

	pfa = PCI_MAKE_PFA (Bus, Device, Function);
	VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
	DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

	ControllerMode = PCI_READ_BYTE (pfa, 0x0A);

	// Check to see if a valid PCI device exists at the PFA?
	if ( (VID == 0xFFFF) || (VID == 0x0000) ||
		 (DID == 0xFFFF) || (DID == 0x0000)
	   )
	{
		return E__PFA_DOESNT_EXIST;
	}

    #ifdef DEBUG
	vConsoleWrite ("Valid PFA exists.\n");
    #endif

	// Check to see if we are in AHCI mode?
	if (ControllerMode != 0x06)
	{
		vConsoleWrite ("SataCtrlr NOT in AHCI mode.\n");
		return E__NOT_IN_AHCI_MODE;
	}


    #ifdef DEBUG
	vConsoleWrite ("SataCtrlr in AHCI mode.\n");
    #endif
	
	Abar = PCI_READ_DWORD(pfa, 0x24);

    #ifdef DEBUG
	sprintf(achBuffer, "Abar = 0x%x\n", Abar);
	vConsoleWrite (achBuffer);
    #endif

	/*
	 * Panther Point EDS 14.1.16
	 * Note:
	 * The ABAR register must be set to a value of 0001_0000h or greater.
	 */
	if (Abar < 0x100000)
	{
		vConsoleWrite ("ABAR is not correctly assigned.\n");
		return E__GARBAGE_ABAR;
	}

	temp  = PCI_READ_DWORD(pfa, 0x04);
	temp |= 6;   // Enable memory and bus_master
	PCI_WRITE_DWORD(pfa, 0x04, temp);

	dHandle = dGetPhysPtr (Abar, 0x800, &p1, &AbarPtr);
	temp    = osReadDWord((AbarPtr + AHCI_HBA_CAP));

    #ifdef DEBUG
	sprintf(achBuffer, "Total ports = %d and CmdSlots = %d\n", (temp & 0x1F) + 1, ((temp >> 8) & 0x1F) + 1);
	vConsoleWrite (achBuffer);
    #endif
	
	ports_implemented = osReadDWord((AbarPtr + AHCI_PORTS_IMPL));

    #ifdef DEBUG
	sprintf(achBuffer, "ports_implemented = 0x%x\n", ports_implemented);
	vConsoleWrite (achBuffer);
    #endif

	if ((VID == 0x8086) && (DID == 0x8c03))
	{
		bIrq = 19;

		bTmp = PCI_READ_BYTE(pfa, 0x3c);			// is interrupt configured in chipset
		if (bTmp == 0)
		{
			PCI_WRITE_BYTE(pfa, 0x3c, bIrq);		// configure interrupt
			bTmp = bIrq;
		}
		if (bIrq != bTmp)							// make sure we use the right interrupt
			bIrq = bTmp;
		
	}
	iVector = sysPinToVector (bIrq, SYS_IOAPIC0); 
	sysInstallUserHandler (iVector, AhciIntHandler0);
	sysUnmaskPin (bIrq, SYS_IOAPIC0);

	PopulatePortBars(ports_implemented);

	if(!PrintPortStatus())
	{
		vConsoleWrite ("No attached drives detected.\n");
		return E__NO_DRIVES_ATTACHED;
	}

	if (!EnableAhci())
	{
		vConsoleWrite ("Error setting AHCI_EN bit in controller.\n");
		return E__ERROR_ENABLING_AHCI;
	}

	AllocateAhciBuffers();

    prd_entry.dba = PHYSICAL_DATA_XFER_LOCATION;			// 0x1000A000
    prd_entry.dbc = 511;

	memcpy(&cmd_table.prd_entry0, &prd_entry, sizeof(PrdEntry));
	memcpy((void*)ptrCmdTable, &cmd_table, sizeof(cmd_table));

	ch_0.options = 0x405;
	ch_0.prdtl   = 1;
	ch_0.ctba0   = CMDTBL_BAR;								// 0x10001000
	memcpy((void*)ptrCmdList, &ch_0, sizeof(ch_0));

#ifdef DEBUG
	sprintf  (achBuffer, "prd_entry = %#08X\n", (unsigned int) &prd_entry); 
	vConsoleWrite (achBuffer);
	sprintf  (achBuffer, "cmd_table = %#08X\n", (unsigned int) &cmd_table); 
	vConsoleWrite (achBuffer);
	sprintf  (achBuffer, "ptrCmdTable = %#08X\n", (unsigned int) &ptrCmdTable); 
	vConsoleWrite (achBuffer);
	sprintf  (achBuffer, "ch_0 = %#08X\n", (unsigned int) &ch_0); 
	vConsoleWrite (achBuffer);
	sprintf  (achBuffer, "ptrCmdList = %#08X\n", (unsigned int) &ptrCmdList); 
	vConsoleWrite (achBuffer);
#endif

	if (PortBars[UserSuppliedPort - 1])
		TestAhciPfaStatus = SetupAhciPort(UserSuppliedPort - 1);


	sysMaskPin(bIrq, SYS_IOAPIC0);

	FreeAllocations();
	DisableAhci();
	vFreePtr(dHandle);
	
	return TestAhciPfaStatus;
}


static int DriveAttachTestAhciPfa(UINT8 Bus, UINT8 Device, UINT8 Function)
{
	PCI_PFA	    pfa;
	UINT16	    VID, DID;
	UINT8       ControllerMode = 0;
	UINT32      dHandle;
	PTR48       p1;
	UINT32      temp    = 0;
	volatile UINT32 ports_implemented = 0;
//	UINT32 		dReg;
	UINT32		rt;


    ResetGlobals();
	memset(PortBars, 0, sizeof(PortBars));
	Abar    = 0;
	AbarPtr = 0;

	pfa = PCI_MAKE_PFA (Bus, Device, Function);
	VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
	DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

	ControllerMode = PCI_READ_BYTE (pfa, 0x0A);

	// Check to see if a valid PCI device exists at the PFA?
	if ( (VID == 0xFFFF) || (VID == 0x0000) ||
		 (DID == 0xFFFF) || (DID == 0x0000)
	   )
	{
		return E__PFA_DOESNT_EXIST;
	}

    #ifdef DEBUG
	vConsoleWrite ("Valid PFA exists.\n");
    #endif

	// Check to see if we are in AHCI mode?
	if (ControllerMode != 0x06)
	{
		vConsoleWrite ("SataCtrlr NOT in AHCI mode.\n");
		return E__NOT_IN_AHCI_MODE;
	}


    #ifdef DEBUG
	vConsoleWrite ("SataCtrlr in AHCI mode.\n");
    #endif

	Abar = PCI_READ_DWORD(pfa, 0x24);

    #ifdef DEBUG
	sprintf(achBuffer, "Abar = 0x%x\n", Abar);
	vConsoleWrite (achBuffer);
    #endif

	/*
	 * Panther Point EDS 14.1.16
	 * Note:
	 * The ABAR register must be set to a value of 0001_0000h or greater.
	 */
	if (Abar < 0x100000)
	{
		vConsoleWrite ("ABAR is not correctly assigned.\n");
		return E__GARBAGE_ABAR;
	}

	temp  = PCI_READ_DWORD(pfa, 0x04);
	temp |= 6;   // Enable memory and bus_master
	PCI_WRITE_DWORD(pfa, 0x04, temp);

	dHandle = dGetPhysPtr (Abar, 0x800, &p1, &AbarPtr);
	temp    = osReadDWord((AbarPtr + AHCI_HBA_CAP));

    #ifdef DEBUG
	sprintf(achBuffer, "Total ports = %d and CmdSlots = %d\n", (temp & 0x1F) + 1, ((temp >> 8) & 0x1F) + 1);
	vConsoleWrite (achBuffer);
    #endif
	
	ports_implemented = osReadDWord((AbarPtr + AHCI_PORTS_IMPL));

    #ifdef DEBUG
	sprintf(achBuffer, "ports_implemented = 0x%x\n", ports_implemented);
	vConsoleWrite (achBuffer);
    #endif

	PopulatePortBars(ports_implemented);

	if(!PrintPortStatus())
	{
		vConsoleWrite ("No attached drives detected.\n");
		rt = E__NO_DRIVES_ATTACHED;
	}
	else
		rt = E__OK;

    DisableAhci();
	vFreePtr(dHandle);

	return rt;
}


static void PopulatePortBars(UINT32 pi)
{
	int  counter  = 0;
	int  BitIndex = 1;
	char ports_implemented_list[128];

	memset(ports_implemented_list, 0, sizeof(ports_implemented_list));

	counter = UserSuppliedPort - 1;
	BitIndex = (1 << counter);

	if (pi & BitIndex)
	{
		sprintf(achBuffer,  "%d, ", counter + 1);
		strcat(ports_implemented_list, achBuffer);
		PortBars[counter] = (0x100 + counter * 0x80);
	}

#ifdef DEBUG
	strcat(ports_implemented_list, "\n");
	vConsoleWrite(ports_implemented_list);
#endif
}

static int PrintPortStatus(void)
{
	int    counter;
	int    drive_count = 0;
	UINT32 temp1 = 0;
	UINT32 temp2 = 0;
#ifdef DEBUG
	UINT32 temp3 = 0;
#endif

	for (counter = 0; counter < 32; counter++)
	{
		if (PortBars[counter])
		{
			temp1 = (PortBars[counter]); // Offset from Abar
			temp2 = osReadDWord((AbarPtr + temp1 + AHCI_PxSSTS));

            #ifdef DEBUG
			sprintf(achBuffer, "PxSSTS = 0x%x\n", temp2);
			vConsoleWrite (achBuffer);
			temp3 = osReadDWord((AbarPtr + temp1 + AHCI_PxSERR));
 			sprintf(achBuffer, "PxSERR = 0x%x\n", temp3);
			vConsoleWrite (achBuffer);
            #endif
			
			// Check if both the below conditions are met?
			// Dev presence detected and phys established + Interface in active state
			if ( ((temp2 & 0xF) == 0x3) /* && (((temp2 >> 8) & 0xF) == 0x1)*/ )
			{
				sprintf(achBuffer, "Drive detected on port %d\n", counter + 1);
				vConsoleWrite(achBuffer);

				drive_count++;
			}
			else
			{
				PortBars[counter] = 0;
			}
		}
	}

	// No drives detected
	return drive_count;
}

static int  EnableAhci(void)
{
   UINT32 temp = 0;


   temp = *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL));


   if (temp & BIT31)
   {
#ifdef DEBUG   
	  sprintf(achBuffer, "AHCI_EN bit already set.\n");
	  vConsoleWrite(achBuffer);
#endif   

	   // SET interrupt enable bit (BIT-1) in GHC
	   temp |= BIT1;
	   *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL)) = temp;

      return 1;   // AHCI already enabled.
   }

   // If its not, then set it.
   temp |= BIT31;
   *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL)) = temp;

   vDelay(100);

   temp = *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL));
   if (temp & BIT31)
   {
	   // SET interrupt enable bit (BIT-1) in GHC
	   temp |= BIT1;
	   *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL)) = temp;

       return 1;
   }
   else
   {
	  sprintf(achBuffer, "Error setting AHCI_EN bit.\n");
	  vConsoleWrite(achBuffer);

      return 0; // Error setting AHCI_EN bit.
   }

}

static int  EnableAhciWithoutInterrupt(void)
{
   UINT32 temp = 0;


   temp = *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL));


   if (temp & BIT31)
   {
#ifdef DEBUG   
	  sprintf(achBuffer, "AHCI_EN bit already set.\n");
	  vConsoleWrite(achBuffer);
#endif
      return 1;   // AHCI already enabled.
   }

   // If its not, then set it.
   temp |= BIT31;
   *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL)) = temp;

   vDelay(100);

   temp = *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL));
   if (temp & BIT31)
   {
       return 1;
   }
   else
   {
	  sprintf(achBuffer, "Error setting AHCI_EN bit.\n");
	  vConsoleWrite(achBuffer);

      return 0; // Error setting AHCI_EN bit.
   }

}


static int  DisableAhci(void)
{
   UINT32 temp = 0;


   temp = *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL));


   if (!(temp & BIT31))
   {
 #ifdef DEBUG
	  sprintf(achBuffer, "AHCI_EN bit already cleared.\n");
	  vConsoleWrite(achBuffer);
#endif

      return 1;   // AHCI already enabled.
   }

   // If its not clear, then clear it.
   temp &= ~BIT31;
   *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL)) = temp;

   vDelay(100);

   temp = *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL));
   if (!(temp & BIT31))
   {
       return 1;
   }
   else
   {
	  sprintf(achBuffer, "Error clearing AHCI_EN bit.\n");
	  vConsoleWrite(achBuffer);

      return 0;
   }

}

static void LoadIdentifyCommand(FisRegH2d* cmd_fis)
{
    memset(cmd_fis, 0, sizeof(FisRegH2d));

	cmd_fis->fis_type     = 0x27;
	cmd_fis->c            = 1;
	cmd_fis->command      = 0xEC;
	cmd_fis->device       = 0xA0;

    return;
}

static void LoadReadSectorCommand(FisRegH2d* cmd_fis)
{
    memset(cmd_fis, 0, sizeof(FisRegH2d));

	cmd_fis->fis_type     = 0x27;
	cmd_fis->c            = 1;
	cmd_fis->command      = 0xC8;
	cmd_fis->device       = 0xA0;
	cmd_fis->sector_count = 1;
	cmd_fis->lba0         = 1;

    return;
}

static int SetupAhciPort(int PortNo)
{
    UINT32 PxFB    = 0;
    UINT32 PxFB_U  = 0;
    UINT32 PxCLB   = 0;
    UINT32 PxCLB_U = 0;
    UINT32 PxCMD   = 0; // Command register
    UINT32 PxCI    = 0; // Command Issue
    UINT32 PxIE    = 0; // Interrupt enable
//    UINT32 PxSCTL  = 0; // Port control
    UINT32 PxSERR  = 0; // Port error
	UINT32 dTemp;

    char   DriveIdentifyBuffer[256] = {0};

    // Populate PxFB register
    PxIE    = PortBars[PortNo] + AHCI_PxIE       ;
    PxFB    = PortBars[PortNo] + AHCI_PxFB       ;
    PxFB_U  = PortBars[PortNo] + AHCI_PxFB_U     ;
    PxCLB   = PortBars[PortNo] + AHCI_PxCLB      ;
    PxCLB_U = PortBars[PortNo] + AHCI_PxCLB_U    ;
//	PxSCTL  = PortBars[PortNo] + AHCI_PxSCTL     ;
	PxSERR  = PortBars[PortNo] + AHCI_PxSERR     ;

    *((UINT32*)(AbarPtr + PxSERR)) = 0x07FF0F03L; // reset error flags
	
//    if ((*((UINT32*)(AbarPtr + PxSCTL)) & 0x0000000FL) == 0)
//		*((UINT32*)(AbarPtr + PxSCTL)) |= BIT0;	  // reset port and renegotiate connection
	
    *((UINT32*)(AbarPtr + PxIE))  |= BIT0;
//    *((UINT32*)(AbarPtr + PxIE))  = 0x7DC000FF;		// enable every possible interrupt!

    *((UINT32*)(AbarPtr + PxFB))  = RCVDFIS_BAR;	// 0x10008000

    if (*((UINT32*)(AbarPtr + PxFB_U)))
	    *((UINT32*)(AbarPtr + PxFB_U)) = 0; // Make sure higher dword is 0.

    *((UINT32*)(AbarPtr + PxCLB)) = CMDLIST_BAR;	// 0x10000000
    if (*((UINT32*)(AbarPtr + PxCLB_U)))
	    *((UINT32*)(AbarPtr + PxCLB_U)) = 0; // Make sure higher dword is 0.


    LoadIdentifyCommand((FisRegH2d*)ptrCmdTable);

    // Set FIS Recv Enable (FRE, offset 0x18, BIT4)
    PxCMD = PortBars[PortNo] + AHCI_PxCMD;
    *((UINT32*)(AbarPtr + PxCMD)) |= BIT4;

    vDelay (100);

    // Set the Start Bit (i.e. BIT0)
    *((UINT32*)(AbarPtr + PxCMD)) |= BIT0;

    vDelay (100);

    //Issue command slot-0
    PxCI = PortBars[PortNo] + AHCI_PxCI ;
    *((UINT32*)(AbarPtr + PxCI)) |= BIT0;

    vDelay (100);

//#if 0
#ifdef DEBUG
    sprintf(achBuffer, "Global Ctrl  = %08x\n", *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Int. Enable  = %08x\n", *((UINT32*)(AbarPtr + PxIE)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "FIS Base     = %08x\n", *((UINT32*)(AbarPtr + PxFB)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "CmdList Base = %08x\n", *((UINT32*)(AbarPtr + PxCLB)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Command      = %08x\n", *((UINT32*)(AbarPtr + PxCMD)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "CmdIssue     = %08x\n", *((UINT32*)(AbarPtr + PxCI)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Int. Status  = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x10)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "TaskFileData = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x20)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Signature    = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x24)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Sata Status  = %08x\n", *((UINT32*)(AbarPtr + PxCLB + AHCI_PxSSTS)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Sata Control = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x2C)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Sata Error   = %08x\n", *((UINT32*)(AbarPtr + PxCLB + AHCI_PxSERR)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Sata Active  = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x34)));
    vConsoleWrite(achBuffer);
#endif
//#endif

    // Make sure the command has completed by checking CmdIssue (CI) register.
	dTemp = *((UINT32*)(AbarPtr + PxCI));
    if (dTemp & BIT0)
    {
#ifdef DEBUG
		sprintf(achBuffer, "CmdIssue     = %08x\n", dTemp);
		vConsoleWrite(achBuffer);
#endif
		sprintf(achBuffer, "Fatal Error1 (Port %02x): Command did not complete.\n", PortNo);
		vConsoleWrite(achBuffer);

		return E__IDENTIFY_CMD_STUCK;
    }

    // Have we received any PIO FIS?
    #ifdef DEBUG
    sprintf(achBuffer, "PIO FIS dword = %08x\n", *((UINT32*)(ptrRcvdFis  + 0x20)));
    vConsoleWrite(achBuffer);
    #endif

    if ( (UINT8) *((UINT32*)(ptrRcvdFis  + 0x20)) != 0x5F)
    {
	    sprintf(achBuffer, "PIO FIS dword = %08x\n", *((UINT32*)(ptrRcvdFis  + 0x20)));
	    vConsoleWrite(achBuffer);
	    sprintf(achBuffer, "FatalError: No PIO Setup FIS received. Aborting\n");
	    vConsoleWrite(achBuffer);

	    return E__NO_PIO_FIS_RCVD;
    }

    // Did we get any errors in PIO Setup FIS?
    if ( (*((UINT32*)(ptrRcvdFis  + 0x20)) >> 24) != 0x00)
    {
	   	sprintf(achBuffer, "PIO FIS dword = %08x\n", *((UINT32*)(ptrRcvdFis  + 0x20)));
	   	vConsoleWrite(achBuffer);
		sprintf(achBuffer, "FatalError: Error set in PIO Setup FIS. Aborting\n");
		vConsoleWrite(achBuffer);
		return E__ERROR_IN_PIO_FIS;
    }



    vDelay (100);
    memcpy (DriveIdentifyBuffer, (void*)ptrDataFis, 256);
    GetDriveName(DriveIdentifyBuffer + 54);


    // Load new CFIS and issue the command.
    LoadReadSectorCommand((FisRegH2d*)ptrCmdTable);

    //Issue command slot-0
    *((UINT32*)(AbarPtr + PxCI)) |= BIT0;

	// allow longer for disk to seek to the required place 
    vDelay (500);
	
#if 0
#ifdef DEBUG
    sprintf(achBuffer, "Global Ctrl  = %08x\n", *((UINT32*)(AbarPtr + AHCI_GLOBAL_HBA_CTRL)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Int. Enable  = %08x\n", *((UINT32*)(AbarPtr + PxIE)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "FIS Base     = %08x\n", *((UINT32*)(AbarPtr + PxFB)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "CmdList Base = %08x\n", *((UINT32*)(AbarPtr + PxCLB)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Command      = %08x\n", *((UINT32*)(AbarPtr + PxCMD)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "CmdIssue     = %08x\n", *((UINT32*)(AbarPtr + PxCI)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Int. Status  = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x10)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "TaskFileData = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x20)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Signature    = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x24)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Sata Status  = %08x\n", *((UINT32*)(AbarPtr + PxCLB + AHCI_PxSSTS)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Sata Control = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x2C)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Sata Error   = %08x\n", *((UINT32*)(AbarPtr + PxCLB + AHCI_PxSERR)));
    vConsoleWrite(achBuffer);
    sprintf(achBuffer, "Sata Active  = %08x\n", *((UINT32*)(AbarPtr + PxCLB + 0x34)));
    vConsoleWrite(achBuffer);
#endif
#endif

    // Make sure the command has completed by checking CmdIssue (CI) register.
	dTemp = *((UINT32*)(AbarPtr + PxCI));
    if (dTemp & BIT0)
    {
#ifdef DEBUG
		sprintf(achBuffer, "CmdIssue     = %08x\n", dTemp);
		vConsoleWrite(achBuffer);
#endif
		sprintf(achBuffer, "Fatal Error2 (Port %02x): Command did not complete.\n", PortNo);
	    vConsoleWrite(achBuffer);

        return E__READ_SECTOR_CMD_STUCK;
    }

#ifdef DEBUG
    DumpSector();
#endif

    return (E__OK);
}


#ifdef DEBUG
static void DumpSector(void)
{
    HexDump((UINT8*)ptrDataFis);
    HexDump((UINT8*)ptrDataFis + 128);
    HexDump((UINT8*)ptrDataFis + 256);
    HexDump((UINT8*)ptrDataFis + 384);
}

static void HexDump(UINT8* buffer)
{
   int    row, col;

   for (row = 0; row < 8; row++)
   {
      for (col = 0; col < 16; col++)
      {
         if (col == 8)
         {
      		sprintf(achBuffer, "- ");
      		vConsoleWrite(achBuffer);
         }

         sprintf(achBuffer, "%02X  ", buffer[row * 16 + col]);
         vConsoleWrite(achBuffer);
      }

      sprintf(achBuffer, "\n");
      vConsoleWrite(achBuffer);
   }

   return;
}
#endif

static void GetDriveName(char* buffer)
{
   int  counter        =  0 ;
   char drive_name[32] = {0};

   for (counter = 0; counter < 16; counter++)
      *((UINT16*)buffer + counter) = wSwap(*((UINT16*)buffer + counter));

   memcpy(drive_name, buffer, 32);
   drive_name[31] = 0;

   sprintf(achBuffer, "Drive Model: %s\n", drive_name);
   vConsoleWrite(achBuffer);
}

static UINT16 wSwap (UINT16 wData)
{
	return ((wData >> 8) | ((wData & 0xff) << 8));
}


static int FreeAllocations(void)
{
	vFreePtr(dHandle1);
	vFreePtr(dHandle2);
	vFreePtr(dHandle3);
	vFreePtr(dHandle4);

	return 1;
}

static int AllocateAhciBuffers (void)
{

   int    counter = 0;
   PTR48  p1;

   dHandle1  = dGetPhysPtr (ptrCmdList,  _8K_, &p1, &ptrCmdList);
   dHandle2  = dGetPhysPtr (ptrCmdTable, _8K_, &p1, &ptrCmdTable);
   dHandle3  = dGetPhysPtr (ptrRcvdFis , _8K_, &p1, &ptrRcvdFis);
   dHandle4  = dGetPhysPtr (ptrDataFis,  _8K_, &p1, &ptrDataFis);

	if( (dHandle1 == E__FAIL) || (dHandle2 == E__FAIL) ||
		(dHandle3 == E__FAIL) || (dHandle4 == E__FAIL))
	{
	#ifdef DEBUG
		vConsoleWrite("Unable to allocate AHCI buffers.\n");
	#endif
		return(-1);
	}

   for (counter = 0; counter < _8K_; counter++)
   {
      *((UINT32*)(ptrCmdList  + counter)) = 0x00;
      *((UINT32*)(ptrCmdTable + counter)) = 0x00;
	  *((UINT32*)(ptrRcvdFis  + counter)) = 0x00;
	  *((UINT32*)(ptrDataFis  + counter)) = 0x00;
   }

   return 1;
}


static int enableLynxPointAhci(void)
{
	int 	rt;
	UINT16	VID, DID;
	UINT16	wTemp;
	UINT32	dTemp;
	UINT32	AbarBase;
	UINT32	Abar;
	UINT32	dRcbaBase;
	UINT32  pRcba;
	UINT32	dHandle;
	PTR48   p1;
	PCI_PFA lpcPfa;
	PCI_PFA ahciPfa;
	int 	cnt;
	
	//FIX IT for non Lynxpoint
	rt = E__OK;
	
	lpcPfa = PCI_MAKE_PFA (0, 0x1F, 0);					// LPC controller
	VID = PCI_READ_WORD (lpcPfa, PCI_VENDOR_ID);
	DID = PCI_READ_WORD (lpcPfa, PCI_DEVICE_ID);
	if ((VID == 0x8086) && (DID == 0x8C4F))				// LynxPoint LPC
	{
		ahciPfa = PCI_MAKE_PFA (0, 0x1F, 2);			// AHCI/IDE controller 1
		DID = PCI_READ_WORD (ahciPfa, PCI_DEVICE_ID);
		
		if (DID == 0x8C01)								// configured as IDE controller, so reconfigure to AHCI
		{
			bLynxPointIde = 1;							// indicate that the controller was in IDE mode when we started
			
#ifdef DEBUG
			vConsoleWrite("Switching Lynx Point IDE to AHCI\n");
#endif	
			// disable all io/memory resources allocated to 2nd SATA controller
#if 0
			ide2Pfa = PCI_MAKE_PFA (0, 0x1F, 5);		// 2nd SATA controller
			PCI_WRITE_DWORD(ide2Pfa, 0x10, 0);			// disable resource
			PCI_WRITE_DWORD(ide2Pfa, 0x14, 0);			// disable resource
			PCI_WRITE_DWORD(ide2Pfa, 0x18, 0);			// disable resource
			PCI_WRITE_DWORD(ide2Pfa, 0x1c, 0);			// disable resource
			PCI_WRITE_DWORD(ide2Pfa, 0x20, 0);			// disable resource
			PCI_WRITE_DWORD(ide2Pfa, 0x24, 0);			// disable resource
			PCI_WRITE_WORD(ide2Pfa,  0x4, 0);			// disable resource decode
#endif
			dRcbaBase = PCI_READ_DWORD (lpcPfa, 0xF0) & 0xFFFFFFFEL;
			dHandle = dGetPhysPtr (dRcbaBase,  0x4000, &p1, &pRcba);
    
			dTemp = *((UINT32*)(pRcba + 0x3418));		// function control register
			dTemp |= 0x2000000L;
			*((UINT32*)(pRcba + 0x3418)) = dTemp;		// disable 2nd SATA controller (D31:F5)
		
			// set up internal PCH interrupt routing
			// BIOS routes INTB# interrupts via PIRQD# to legacy PIC INT_0A
		
			bLegacyInt = PCI_READ_BYTE(lpcPfa, 0x63);
			PCI_WRITE_BYTE(lpcPfa, 0x63, 0x80);			// route interrupt to IOAPIC
    
			wTemp = PCI_READ_WORD(ahciPfa, 0x90);
			wTemp &= ~0x00E0;
			wTemp |= 0x60;						// set AHCI mode, 6 ports - FIX IT
			PCI_WRITE_WORD(ahciPfa, 0x90, wTemp);
		
			vFreePtr(dHandle);

			// re-read DID 
			DID = PCI_READ_WORD (ahciPfa, PCI_DEVICE_ID);
			if (DID == 0x8C03)								// SATA now configured as AHCI controller
			{
				PCI_WRITE_WORD(ahciPfa, 0x4, 7);			// enable memory access to AHCI registers
				dTemp = PCI_READ_DWORD(ahciPfa, 0x24);		// check if BAR already allocated
				if ((dTemp & 0xFFFF0000L) == 0)
				{
					if (dTemp != 0)
						bLegacyAbar = dTemp;				// preserve legacy ABAR value (I/O address)
#ifdef DEBUG
					vConsoleWrite("Allocating memory for ABAR\n");
#endif
					Abar = 0xBC000000L;
					PCI_WRITE_DWORD(ahciPfa, 0x24, Abar);	// allocate BAR memory address
					bAhciAbarAlloc = 1;
				}
			}
		}
		
		// implement AHCI ports
		if (DID == 0x8C03)								// SATA configured as AHCI controller
		{
			PCI_WRITE_WORD(ahciPfa, 0x4, 7);			// enable memory access to AHCI registers
			Abar = PCI_READ_DWORD(ahciPfa, 0x24);		// get ABAR
			dHandle = dGetPhysPtr (Abar, 0x800, &p1, &AbarBase);
			
			dTemp = *((UINT32*)(AbarBase + AHCI_PORTS_IMPL));
			if (dTemp == 0)
			{
#ifdef DEBUG
				vConsoleWrite("Implementing ports\n");
#endif
				*((UINT32*)(AbarBase + AHCI_PORTS_IMPL)) = 0x0000003FL;
				vDelay(1);
				dTemp = *((UINT32*)(AbarBase + AHCI_PORTS_IMPL));
				vDelay(1);
				dTemp = *((UINT32*)(AbarBase + AHCI_PORTS_IMPL));
				vDelay(1);
			}
#ifdef DEBUG
			vConsoleWrite("Resetting Controller\n");
#endif
			dTemp = *((UINT32*)(AbarBase + AHCI_GLOBAL_HBA_CTRL));	// AHCI control register
			dTemp |= BIT0;
			*((UINT32*)(AbarBase + AHCI_GLOBAL_HBA_CTRL)) = dTemp;	// reset AHCI controller
			
			cnt = 15;
			while (cnt > 0)
			{
				vDelay(100);
				dTemp = *((UINT32*)(AbarBase + AHCI_GLOBAL_HBA_CTRL));	// poll AHCI control register
				if ((dTemp & BIT0) == 0)
					break;
					
				--cnt;
			}

#ifdef DEBUG
			sprintf(achBuffer, "Ports Impl. = %08x\n", *((UINT32*)(AbarBase + AHCI_PORTS_IMPL)));
			vConsoleWrite(achBuffer);
			sprintf(achBuffer, "Global Ctrl = %08x\n", *((UINT32*)(AbarBase + AHCI_GLOBAL_HBA_CTRL)));
			vConsoleWrite(achBuffer);
#endif
			if (cnt == 0)
				rt = E__ERROR_LYNX_POINT_AHCI;
    
			vFreePtr (dHandle);
		}
	}
	
	return rt;
}

static void disableLynxPointAhci (void)
{
	UINT16	VID, DID;
	UINT32	dTemp;
	UINT32	dRcbaBase;
	UINT32  pRcba;
	UINT32	dHandle;
	PTR48   p1;
	PCI_PFA lpcPfa;
	PCI_PFA ahciPfa;
	
	lpcPfa = PCI_MAKE_PFA (0, 0x1F, 0);					// LPC controller
	VID = PCI_READ_WORD (lpcPfa, PCI_VENDOR_ID);
	DID = PCI_READ_WORD (lpcPfa, PCI_DEVICE_ID);
	
	if ((VID == 0x8086) && (DID == 0x8C4F))				// LynxPoint LPC
	{
		ahciPfa = PCI_MAKE_PFA (0, 0x1F, 2);			// AHCI/IDE controller 1
		DID = PCI_READ_WORD (ahciPfa, PCI_DEVICE_ID);
		if (DID == 0x8C03)
		{
			if (bAhciAbarAlloc != 0)
			{
				PCI_WRITE_DWORD(ahciPfa, 0x24, bLegacyAbar);	// restore legacy ABAR value
				bAhciAbarAlloc = 0;
			}
			
			if (bLynxPointIde != 0)
			{
				PCI_WRITE_BYTE(ahciPfa, 0x90, 0x00);		// restore IDE mode
				dTemp = (UINT32) PCI_READ_BYTE(ahciPfa, 0x90);
				bLynxPointIde = 0;
				
				dRcbaBase = PCI_READ_DWORD (lpcPfa, 0xF0);
				dHandle = dGetPhysPtr (dRcbaBase,  0x4000, &p1, &pRcba);
        
				dTemp = *((UINT32*)(pRcba + 0x3418));		// function control register
				dTemp &= ~0x2000000L;
				*((UINT32*)(pRcba + 0x3418)) = dTemp;		// enable 2nd SATA controller (D31:F5)
				
				vFreePtr (dHandle);
			}
			
			if (bLegacyInt != 0)
			{
				PCI_WRITE_BYTE(lpcPfa, 0x63, bLegacyInt);	// restore Legacy interrupt
				bLegacyInt = 0;
			}
		}
	}
}
