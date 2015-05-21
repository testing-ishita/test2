
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

/* bit_io.c - BIT I/O port access code
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/pci.c,v 1.6 2014-03-06 13:27:40 hchalla Exp $
 *
 * $Log: pci.c,v $
 * Revision 1.6  2014-03-06 13:27:40  hchalla
 * Fixed bug in PCI Extended writes Dword write were written as a word write and changed
 * bus, device , function numbers from pfa to use macros provided.
 *
 * Revision 1.5  2014-01-29 13:16:41  hchalla
 * Modified PCIE access logic, previously bus numbers were not bit shifted correctly.
 *
 * Revision 1.4  2013-12-10 12:10:14  mgostling
 * Tidied up code . No functional changes.
 *
 * Revision 1.3  2013-11-11 15:16:00  mgostling
 * Updated PCIE register functions.
 * Revision 1.2  2013/09/26 12:58:11  chippisley
 * Removed obsolete HAL.
 *
 * Revision 1.1  2013/09/04 07:11:17  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2010/05/05 15:42:55  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.4  2009/12/16 14:10:32  sdahanayaka
 * Added the VME and Watchdog Tests. Also fixed some PCI Bugs
 *
 * Revision 1.3  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.2  2008/05/12 14:49:35  swilson
 * Corrections to parameter passing order - use an order compatible with code used in
 *  other projects.
 *
 * Revision 1.1  2008/04/30 07:51:19  swilson
 * Add PCI support library.
 *
 */


/* includes */

#include <stdtypes.h>
#include <string.h>

#include <bit/hal.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <private/port_io.h>
#include <private/debug.h>

/* defines */

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define MCH_PCI_EXPRESS_BASE_ADDRESS 0xE0000000

#define MmPciAddress(Bus, Device, Function, Register) ((UINT32)MCH_PCI_EXPRESS_BASE_ADDRESS + (UINT32)(Bus << 20) + (UINT16)(Device << 15) + (UINT16)(Function << 12) + (UINT32)(Register))

#define PCI_ADDR		0x0CF8
#define PCI_DATA		0x0CFC

#define PCI_CFG_ENABLE	0x80000000

/* typedefs */

/* constants */

/* locals */

static UINT8 bPciHighestBus = 255;

static PTR48 tPtr;
static UINT32 mHandle;
static volatile UINT32 memMap;

static UINT32 mWrHandle;
static PTR48 tWrPtr;
static volatile UINT32 memWrMap;

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * vPciSetHighestBus: specify the highest bus to scan
 *
 * RETURNS: None
 */

void vPciSetHighestBus
(
	UINT8	bBus
)
{
	bPciHighestBus = bBus;

} /* vPciSetHighestBus () */


/*****************************************************************************
 * dPciReadReg: read a PCI configuration register
 *
 * RETURNS: configuration data, as a UINT32
 */

UINT32 dPciReadReg
(
	PCI_PFA		pfa,		/* PCI function address */
	UINT8		bReg,		/* register number */
	REG_SIZE	size		/* register size */
)
{
	UINT32 dData;

	/* Enable config access to device and register */

	sysOutPort32 (PCI_ADDR, PCI_CFG_ENABLE | 
							((UINT32)pfa << 8) | 
							(UINT32)(bReg & 0xFC));

	/* Read the register */

	switch (size)
	{
		case REG_8 :	/* 8-bit register */

			dData = (UINT32)sysInPort8( PCI_DATA + (bReg & 0x03) );
			break;

		case REG_16 :	/* 16-bit register */

			dData = (UINT32)sysInPort16( PCI_DATA + (bReg & 0x02) );
			break;

		default :		/* 32-bit register */

			dData = sysInPort32( PCI_DATA );
			break;
	}

	/* Disable config access before exit */

	sysOutPort32 (PCI_ADDR, 0);
	
	return dData;

} /* dPciReadReg () */


/*****************************************************************************
 * vPciWriteReg: write a PCI configuration register
 *
 * RETURNS: None
 */

void vPciWriteReg
(
	PCI_PFA		pfa,		/* PCI function address */
	UINT8		bReg,		/* register number */
	REG_SIZE	size,		/* register size */
	UINT32		dData		/* data value to write */
)
{
	/* Enable config access to device and register */

	sysOutPort32 (PCI_ADDR, PCI_CFG_ENABLE | 
							((UINT32)pfa << 8) | 
							(UINT32)(bReg & 0xFC));

	/* Write the register */

	switch (size)
	{
		case REG_8 :	/* 8-bit register */

			sysOutPort8 (PCI_DATA + (bReg & 0x03), (UINT8)dData);
			break;

		case REG_16 :	/* 16-bit register */

			sysOutPort16 (PCI_DATA + (bReg & 0x02), (UINT16)dData);
			break;

		default :		/* 32-bit register */

			sysOutPort32 (PCI_DATA, dData);
			break;
	}

	/* Disable config access before exit */

	sysOutPort32 (PCI_ADDR, 0);
	

} /* vPciWriteReg () */


/*****************************************************************************
 * dPciReadRegExtd: read a PCI Express extended configuration register
 *
 * RETURNS: configuration data, as a UINT32
 */

UINT32 dPciReadRegExtd
(
	PCI_PFA		pfa,		/* PCI Function Address */
	UINT16		wReg,		/* valid range 0x0 - 0x0FFF */
	REG_SIZE	size		/* register size */
)
{
	UINT16 Bus;
	UINT16 Device;
	UINT16 Function;
	UINT32 dOffset;
	UINT32 dData;

	//Bus      = ((pfa & 0xFF00)>>8);
	//Device   = (pfa & 0x00FC);
	//Function = (pfa & 0x0003);

	Bus         = PCI_PFA_BUS(pfa);
	Device    = PCI_PFA_DEV(pfa);
	Function = PCI_PFA_FUNC(pfa);


	dOffset = MmPciAddress( Bus, Device, Function, wReg );
	mHandle = dGetPhysPtr( dOffset, 0x10000, &tPtr, (void*)&memMap );

	if(mHandle == E__FAIL)
	{
		sysDebugWriteString( "No handle to read mapped window!\n" );
	}

	switch ( size )
	{
		case REG_8 :
			dData = vReadByte( memMap );
			break;
		case REG_16 :
			dData = vReadWord( memMap );
			break;
		case REG_32 :
			dData = vReadDWord( memMap );
			break;
			
		default: 
			dData = 0;
			break;
	}

	if (mHandle)
		vFreePtr(mHandle);

	return dData;

}/* dPciReadRegExtd () */


/*****************************************************************************
 * vPciWriteRegExtd: write a PCI Express extended configuration register
 *
 * RETURNS: None
 */

void vPciWriteRegExtd
(
	PCI_PFA		pfa,		/* PCI Function Address */
	UINT16		wReg,		/* valid range 0x0 - 0x0FFF */
	REG_SIZE	size,		/* register size */
	UINT32		dData
)
{
	UINT16 Bus;
	UINT16 Device;
	UINT16 Function;
	UINT32 dOffset;

	//Bus      = ((pfa & 0xFF00) >> 8);
	//Device   = (pfa & 0x00FC);
	//Function = (pfa & 0x0003);

	Bus         = PCI_PFA_BUS(pfa);
	Device    = PCI_PFA_DEV(pfa);
	Function = PCI_PFA_FUNC(pfa);

	dOffset = MmPciAddress( Bus, Device, Function, wReg );

	mWrHandle = dGetPhysPtr( dOffset, 0x10000, &tWrPtr, (void*)&memWrMap );

	if(mWrHandle == E__FAIL)
	{
		sysDebugWriteString("No handle to read mapped window!\n");
	}

	switch ( size )
	{
		case REG_8 :
			vWriteByte( memWrMap, dData );
			break;
		case REG_16 :
			vWriteWord( memWrMap, dData);
			break;
		case REG_32 :
			vWriteDWord( memWrMap, dData);
			break;
			
		default: break;
	}

	if(mWrHandle)
		vFreePtr(mWrHandle);

} /* vPciWriteRegExtd () */


/*****************************************************************************
 * iPciFindDeviceById: find the Nth PCI device with given Vendor/Device IDs
 *
 * RETURNS: E__OK or E__DEVICE_NOT_FOUND
 */

int iPciFindDeviceById
(
	UINT32		dInstance,	/* Nth instance to locate */
	UINT16		wVid,		/* Vendor ID to locate */
	UINT16		wDid,		/* Device ID to locate */
	PCI_PFA*	pfa			/* PCI Function Address */
)
{
	PCI_PFA		thisPfa;	/* PFA during search */
	UINT32		dCount;		/* count of found devices of type */
	UINT16		wThisVid;
	UINT16		wThisDid;
	int			iMultiFunc;	/* multi-function flag */
	
	
	thisPfa = PCI_MAKE_PFA (0,0,0);
	dCount = 0;
	
	while (PCI_PFA_BUS (thisPfa) < bPciHighestBus)
	{
		/* Reset the multi-function flag for function 0 */
	
		if (PCI_PFA_FUNC (thisPfa) == 0)
			iMultiFunc = 0;

		/* Read the Vendor ID to see if this function is present */
			
		wThisVid = (UINT16)dPciReadReg (thisPfa, PCI_VENDOR_ID, REG_16);
		
		if (wThisVid != 0xFFFF)
		{
			/* Compare the device and vendor IDs for this function. If there is 
			 * a match, increment the instance count. If this is the one we are
			 * looking for, return its PFA
			 */	
		 		 		 
			wThisDid = (UINT16)dPciReadReg (thisPfa, PCI_DEVICE_ID, REG_16);
		
			if ((wVid == wThisVid) && (wDid == wThisDid))
			{
				dCount++;
			
				if (dCount == dInstance)
				{
                                        memcpy (pfa, &thisPfa, sizeof (PCI_PFA)) ;
					return (E__OK);				
				}
			}
		
			/* For function zero only, check the header type to see if this is a
			 * multifunction device
			 */		 
		
			if (PCI_PFA_FUNC (thisPfa) == 0)
			{
				if ((UINT8)dPciReadReg (thisPfa, PCI_HEADER_TYPE, REG_8) & 0x80)
					iMultiFunc = 1;
			}
		}
		
		/* For multifunction devices, increment the function, otherwise
		 * increment the device (8 functions)
		 */		 
		
		if (iMultiFunc == 1)
			thisPfa++;

		else
			thisPfa += 8;
	}	
	
	return (E__DEVICE_NOT_FOUND);
	
} /* iPciFindDeviceById () */


/*****************************************************************************
 * iPciFindDeviceByClass: find the Nth PCI device with given Class Code
 *
 * RETURNS: E__OK or E__DEVICE_NOT_FOUND
 */

int iPciFindDeviceByClass
(
	UINT32		dInstance, 	/* Nth instance to locate */
	UINT8		bBaseClass,	/* base class code */
	UINT8		bSubClass,	/* sub class code */
	PCI_PFA*	pfa			/* PCI Function Address */
)
{
	PCI_PFA		thisPfa;	/* PFA during search */
	UINT32		dCount;		/* count of found devices of type */
	UINT16		wThisVid;
	UINT8		bThisBaseClass;	
	UINT8		bThisSubClass;	
	int			iMultiFunc;	/* multi-function flag */
	
	
	thisPfa = PCI_MAKE_PFA (0,0,0);
	dCount = 0;
		
	while (PCI_PFA_BUS (thisPfa) < bPciHighestBus)
	{
		/* Reset the multi-function flag for function 0 */
	
		if (PCI_PFA_FUNC (thisPfa) == 0)
			iMultiFunc = 0;
			
		/* Read the Vendor ID to see if this function is present */
			
		wThisVid = (UINT16)dPciReadReg (thisPfa, PCI_VENDOR_ID, REG_16);
		
		if (wThisVid != 0xFFFF)
		{
			/* Read the class IDs for this function. If there is a match, 
			 * increment the instance count. If this is the one we are looking
			 * for, return its PFA
			 */
			bThisBaseClass = (UINT8)dPciReadReg (thisPfa, PCI_BASE_CLASS, REG_8);
			bThisSubClass = (UINT8)dPciReadReg (thisPfa, PCI_SUB_CLASS, REG_8);
		
			if ((bBaseClass == bThisBaseClass) && (bSubClass == bThisSubClass))
			{
				dCount++;
			
				if (dCount == dInstance)
				{
					*pfa = thisPfa;
					return (E__OK);				
				}
			}
		
			/* For function zero only, check the header type to see if this is a
			 * multifunction device
			 */		 
		
			if (PCI_PFA_FUNC (thisPfa) == 0)
			{
				if ((UINT8)dPciReadReg (thisPfa, PCI_HEADER_TYPE, REG_8) & 0x80)
					iMultiFunc = 1;
			}
		}
		
		/* For multifunction devices, increment the function, otherwise
		 * increment the device (8 functions)
		 */		 
		
		if (iMultiFunc == 1)
			thisPfa++;

		else
			thisPfa += 8;
	}	
	
	return (E__DEVICE_NOT_FOUND);
	
} /* iPciFindDeviceByClass () */
