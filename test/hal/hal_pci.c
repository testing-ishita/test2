
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

/* bit_io.c - BIT PCI(express) configuration space access code
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/hal/hal_pci.c,v 1.2 2013-11-25 11:49:16 mgostling Exp $
 *
 * $Log: hal_pci.c,v $
 * Revision 1.2  2013-11-25 11:49:16  mgostling
 * Corrected PCIExpress memory mapped register access.
 *
 * Revision 1.1  2013-09-04 07:26:55  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/01/20 10:01:25  hmuneer
 * CA01A151
 *
 * Revision 1.4  2010/09/15 12:16:23  hchalla
 * Implemented new API for locking the memory after allocation.
 *
 * Revision 1.3  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 * Revision 1.2  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/04/30 07:51:19  swilson
 * Add PCI support library.
 *
 */


/* includes */

#include <stdio.h>
#include <stdtypes.h>

#include <bit/console.h>
#include <bit/hal.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <private/port_io.h>
#include <private/debug.h>

/* defines */
//#define DEBUG

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define MCH_PCI_EXPRESS_BASE_ADDRESS 0xE0000000L

#define MmPciAddress(pfa) (MCH_PCI_EXPRESS_BASE_ADDRESS + (((UINT32) pfa) << 12))

#define PCI_ADDR		0x0CF8
#define PCI_DATA		0x0CFC

#define PCI_CFG_ENABLE	0x80000000


/* typedefs */

/* constants */

/* locals */
static UINT32  mHandle;
static PTR48   tPtr;
static volatile UINT32 	memMap;

static UINT32  mWrHandle;
static PTR48   tWrPtr;
static volatile UINT32 	memWrMap;

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * spPciReadReg: HAL interface to PCI configuration register read
 *
 * RETURNS: None
 */

void spPciReadReg
(
	CCT_PCI_REG*	psParams
)
{
	/* Enable config access to device and register */

	sysOutPort32 (PCI_ADDR, PCI_CFG_ENABLE | 
							((UINT32)psParams->pfa << 8) | 
							(UINT32)(psParams->bRegister & 0xFC));

	/* Read the register */

	switch (psParams->size)
	{
		case REG_8 :	/* 8-bit register */

			psParams->dData = (UINT32)sysInPort8 (PCI_DATA + 
												(psParams->bRegister & 0x03));
			break;

		case REG_16 :	/* 16-bit register */

			psParams->dData = (UINT32)sysInPort16 (PCI_DATA + 
												(psParams->bRegister & 0x02));
			break;

		default :		/* 32-bit register */

			psParams->dData = sysInPort32 (PCI_DATA);
			break;
	}

	/* Disable config access before exit */

	sysOutPort32 (PCI_ADDR, 0);

} /* spPciReadReg () */


/*****************************************************************************
 * spPciWriteReg: HAL interface to PCI configuration register write
 *
 * RETURNS: None
 */

void spPciWriteReg
(
	CCT_PCI_REG*	psParams
)
{
	/* Enable config access to device and register */

	sysOutPort32 (PCI_ADDR, PCI_CFG_ENABLE | 
							((UINT32)psParams->pfa << 8) | 
							(UINT32)(psParams->bRegister & 0xFC));

	/* Write the register */

	switch (psParams->size)
	{
		case REG_8 :	/* 8-bit register */

			sysOutPort8 (PCI_DATA + (psParams->bRegister & 0x03), 
							(UINT8)psParams->dData);
			break;

		case REG_16 :	/* 16-bit register */

			sysOutPort16 (PCI_DATA + (psParams->bRegister & 0x02), 
							(UINT16)psParams->dData);
			break;

		default :		/* 32-bit register */

			sysOutPort32 (PCI_DATA, psParams->dData);
			break;
	}

	/* Disable config access before exit */

	sysOutPort32 (PCI_ADDR, 0);

} /* spPciWriteReg () */


/*****************************************************************************
 * spPciExprReadReg: HAL interface to PCI Express extended config read
 *
 * RETURNS: None
 */

void spPciExprReadReg
(
	CCT_PCIE_REG*	psParams
)
{
	UINT32 dOffset;

	dOffset = MmPciAddress (psParams->pfa);
	mHandle = dGetPhysPtr(dOffset,0x1000,&tPtr,(void*)&memMap);		// 4K memory window

	if(mHandle == E__FAIL)
	{
		sysDebugWriteString("No handle to read mapped window!\n");
	}

	switch (psParams->size)
	{
		case REG_8 :
			psParams->dData = (UINT32) vReadByte (memMap + (UINT32) psParams->wRegister);
			break;
		case REG_16 :
			psParams->dData = (UINT32) vReadWord (memMap + (UINT32) psParams->wRegister);
			break;
		case REG_32 :
			psParams->dData = vReadDWord (memMap + (UINT32) psParams->wRegister);
			break;
	}

	vFreePtr (mHandle); 		// release memory
} /* spPciExprReadReg () */


/*****************************************************************************
 * spPciExprWriteReg: HAL interface to PCI Express extended config write
 *
 * RETURNS: None
 */

void spPciExprWriteReg
(
	CCT_PCIE_REG*	psParams
)
{
	UINT32 dOffset;

	dOffset = MmPciAddress (psParams->pfa);
	mWrHandle = dGetPhysPtr(dOffset,0x1000,&tWrPtr,(void*)&memWrMap);		// 4K memory window

	if(mWrHandle == E__FAIL)
	{
		sysDebugWriteString("No handle to write mapped window!\n");
	}

	switch (psParams->size)
	{
		case REG_8 :
			vWriteByte (memWrMap + (UINT32) psParams->wRegister, psParams->dData);
			break;
		case REG_16 :
			vWriteWord (memWrMap + (UINT32) psParams->wRegister, psParams->dData);
			break;
		case REG_32 :
			vWriteDWord (memWrMap + (UINT32) psParams->wRegister, psParams->dData);
			break;
	}

	vFreePtr (mWrHandle); 		// release memory
} /* spPciExprWriteReg () */



