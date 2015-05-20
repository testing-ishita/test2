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

/* ide.c - IDE test, which tests the disk interface 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/ide.c,v 1.5 2015-04-01 16:49:16 hchalla Exp $
 *
 * $Log: ide.c,v $
 * Revision 1.5  2015-04-01 16:49:16  hchalla
 * Added support for 6 ports while detection of IDE ports.
 *
 * Revision 1.4  2015-02-25 18:02:57  hchalla
 * Added support for new PCI IDE device ID support.
 *
 * Revision 1.3  2013-11-25 11:46:46  mgostling
 * Add support for TRB1x
 *
 * Revision 1.2  2013-09-26 12:35:23  chippisley
 * Removed obsolete HAL
 *
 * Revision 1.1  2013/09/04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.13  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.12  2011/10/27 15:50:39  hmuneer
 * no message
 *
 * Revision 1.11  2011/03/22 13:56:08  hchalla
 * Modified  ide tests and parameters.
 *
 * Revision 1.10  2011/02/28 11:59:20  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.9  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.8  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.7  2010/10/06 10:12:58  jthiru
 * PBIT coding for VP717 - added support for all ports, test case fixes
 *
 * Revision 1.6  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.5  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.4  2009/06/19 13:21:49  jthiru
 * Added check for port number passed by user
 *
 * Revision 1.3  2009/06/03 08:30:58  cvsuser
 * Tidy up test names
 *
 * Revision 1.2  2009/05/21 14:28:01  cvsuser
 * Added support to execute test without parameters
 *
 * Revision 1.1  2009/05/15 11:10:09  jthiru
 * Initial checkin for SATA tests
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <errors.h>		
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <devices/ide.h>
#include <bit/hal.h>
#include <bit/board_service.h>
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>
#include <private/cpu.h>
#include <error_msg.h>


/* defines */

//#undef DEBUG
#define DEBUG


static char buffer[128];


/* constants */
const PORTINFO sPortInfo [] = {
	{0x1f0, 0x3f4, 14},
	{0x170, 0x374, 15}
};

const TESTINFO sTestData [] = {
	{HDC_PORT_NSECT,  0x55}, 
	{HDC_PORT_SECTNO, 0xAA}, 
	{HDC_PORT_LOCYL,  0x0f},
	{0,0}
};					/* register test data		*/


/* locals */

/* globals */


/* externals */
extern void vDelay(UINT32 dMsDelay);
extern void vConsoleWrite(char*	achMessage);
extern void spMemGetBuffer (CCT_MEM_PTR* psParams);
extern void spMemFreeMem (CCT_MEM_PTR* psParams);
extern void spMemFreeAll (CCT_MEM_PTR* psParams);

/* forward declarations */


/*****************************************************************************
 * dIderegtest: this function checks the registers of the controller.
 *				Write and read test patterns to the cylinder and sector registers.
 *
 * RETURNS: Test passed E_OK or error code.
 */

static UINT32 dIderegtest 
( 
	DEVICE *psDev, 
	UINT8 bport
)
{
	UINT32 dTestStatus = E__OK;
	UINT8 	bIndex = 0;
	UINT8 	bData;
	UINT16	wPort;
	UINT8 bDrive = 0;

    if(bport == 2) /* For device 1*/
    {
    	bport = 0;
    	bDrive = 1;
    }
    if(bport == 3) /* For device 1*/
    {
    	bport  = 1;
    	bDrive = 1;
    }

	wPort = psDev->sPortInfo[bport].wCmdReg;
	
	if (bDrive == 0)
		vIoWriteReg (wPort + HDC_PORT_SDH, REG_8, 0xE0);		/* master drive */
	else
		vIoWriteReg (wPort + HDC_PORT_SDH, REG_8, 0xF0);		/* slave drive */

	while (sTestData[bIndex].bPort != 0)
	{
		vIoWriteReg (wPort + sTestData[bIndex].bPort, REG_8, sTestData[bIndex].bData);
		bIndex++;
	}

	bIndex = 0;

	while (sTestData[bIndex].bPort != 0)
	{
		bData = dIoReadReg(wPort + sTestData[bIndex].bPort, REG_8);

		if (bData != sTestData[bIndex].bData)
		{
			dTestStatus = E__IDE_REGS_ERROR;
#ifdef DEBUG
			sprintf (buffer, "Reg : %x. Read : %x, Wrote %x Port: %x\n",
						(int)(wPort + sTestData[bIndex].bPort),
						(int)bData,
						(int)sTestData[bIndex].bData, bport + 1);
			vConsoleWrite (buffer);
#endif
			break;
		}

		bIndex++;
    }

    return dTestStatus;

} /* dIderegtest () */


/*****************************************************************************
 * dInitIde: this function detects a PCI IDE controller and performs basic
 * 				initialization.
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 dInitIde 
( 
	DEVICE *psDev, 
	UINT32 bInstance 
)
{
	UINT8	bTemp;
    UINT32	dFound;
    UINT16	wPciCmd;
	UINT32	dPciDevId;
	PCI_PFA pfa;

	/* find the IDE controller by class code */
  	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
    dFound = iPciFindDeviceByClass (bInstance, 0x01, 0x01, &pfa);
    if (dFound == E__DEVICE_NOT_FOUND)
	{
#ifdef DEBUG
		sprintf(buffer, "SATA Instance %d not found",bInstance);
		vConsoleWrite(buffer);
#endif
		return dFound;
	}
	psDev->bBus = PCI_PFA_BUS(pfa);
	psDev->bDev = PCI_PFA_DEV(pfa);
	psDev->bFunc = PCI_PFA_FUNC(pfa);
#ifdef DEBUG
	sprintf(buffer, "B: %d, D: %d, F: %d\n", PCI_PFA_BUS(pfa), PCI_PFA_DEV(pfa), PCI_PFA_FUNC(pfa));
	vConsoleWrite(buffer);
#endif
	psDev->bCtrlType = TYPE_IDE;
	psDev->wBMIdeRegs = 0;

	/* save the legacy I/O and IRQ values */

	for (bTemp = 0; bTemp < 2; bTemp++)
	{
		psDev->sPortInfo[bTemp].wCmdReg  = sPortInfo[bTemp].wCmdReg;
		psDev->sPortInfo[bTemp].wCtrlReg = sPortInfo[bTemp].wCtrlReg;
		psDev->sPortInfo[bTemp].bIrq     = sPortInfo[bTemp].bIrq;
	}

	dPciDevId = dPciReadReg(pfa, 0x00, PCI_DWORD);


	/* if this is a serial ATA controller, make sure the ports are enabled */
	if ( dPciDevId == 0x25a38086 || dPciDevId == 0x25b08086 || 
	 	 dPciDevId == 0x26808086 || dPciDevId == 0x27c48086 || 
	 	 dPciDevId == 0x292D8086 || dPciDevId == 0x29288086 ||
	 	 dPciDevId == 0x27df8086 || dPciDevId == 0x25a28086 || /* VX405 ICH7 */
	 	 dPciDevId == 0x3b2e8086 || dPciDevId == 0x3b2d8086 ||
	 	 dPciDevId == 0x1c018086 || dPciDevId == 0x1c098086 ||
	 	 dPciDevId == 0x1e038086 || dPciDevId == 0x1e018086 ||
	 	 dPciDevId == 0x1e098086 || dPciDevId == 0x8c018086 ||
	 	 dPciDevId == 0x8c098086 )
	{
	
		if ((dPciDevId != 0x8c018086) &&
			(dPciDevId != 0x8c098086 ))
		{
			vPciWriteReg (pfa, 0xa0, PCI_BYTE, 0x18);
		
		#ifdef DEBUG
            sprintf(buffer,"SATA Init reg %x\n",dPciReadReg(pfa,0xa4, PCI_DWORD));
            vConsoleWrite (buffer);
		#endif
		}

		bTemp = (UINT8)dPciReadReg(pfa, 0x92, PCI_BYTE);

		if (dPciDevId == 0x26808086)		/*3100 chipset*/
		{
			bTemp |= 0x0f;
			vPciWriteReg (pfa, 0x92, PCI_BYTE, bTemp);
			vDelay(1000);
			psDev->bCtrlType = TYPE_SATA;
		}
		if (dPciDevId == 0x8c018086)		/* Lynx Point IDE ports 1 to 4 */
		{
			bTemp |= 0x0f;
			vPciWriteReg (pfa, 0x92, PCI_BYTE, bTemp);
			vPciWriteReg (pfa, 0x98, PCI_DWORD, 0x205C0220L);
			vPciWriteReg (pfa, 0x9C, PCI_DWORD, 0x00000020L);
			vDelay(1000);
			psDev->bCtrlType = TYPE_SATA;
		}
		else if (dPciDevId == 0x8c098086)	/* Lynx Point IDE ports 5 & 6 */
		{
			bTemp |= 0x3;
			vPciWriteReg (pfa, 0x92, PCI_BYTE, bTemp);
			vDelay(1000);
			psDev->bCtrlType = TYPE_SATA;
		}
		else if(dPciDevId == 0x3b2e8086)
		{
			bTemp |= 0x33;
			vPciWriteReg (pfa, 0x92, PCI_BYTE, bTemp);
			vDelay(1000);
			psDev->bCtrlType = TYPE_SATA;
		}
		else
		{
			bTemp |= 0x1F;
			vPciWriteReg (pfa, 0x92, PCI_BYTE, bTemp);
			vDelay(1000);
			psDev->bCtrlType = TYPE_SATA;
		}

		/* Add here IDE controller ids */
		if((dPciDevId == 0x27df8086) || (dPciDevId == 0x25a28086) ||
		   (dPciDevId == 0x3B668086))
		{
			psDev->bCtrlType = TYPE_IDE;
			
	    	wPciCmd = (UINT16)dPciReadReg (pfa, 0x04, PCI_WORD);
    		vPciWriteReg (pfa, 0x04, PCI_WORD, wPciCmd | (1 << 1));
		}
		
		/* Use the SATA controller in native mode, so get resources */

		bTemp = (UINT8)dPciReadReg (pfa, 0x09, PCI_BYTE);
		vPciWriteReg (pfa, 0x09, PCI_BYTE, bTemp | 0x05);	

		
		for (bTemp = 0; bTemp < 2; bTemp++)
		{
			psDev->sPortInfo[bTemp].wCmdReg  = (UINT16)dPciReadReg (pfa, 0x10 + (bTemp * 8), PCI_WORD) & 0xfffc;
			psDev->sPortInfo[bTemp].wCtrlReg = (UINT16)dPciReadReg (pfa, 0x14 + (bTemp * 8), PCI_WORD) & 0xfffc;
			psDev->sPortInfo[bTemp].bIrq     = (UINT8)(dPciReadReg (pfa, 0x3c, PCI_WORD) >> 8);
		}

	}
    
 	/* make sure the device IO access and BMDMA is enabled */

    wPciCmd = (UINT16)dPciReadReg (pfa, 0x04, PCI_WORD);
    vPciWriteReg (pfa, 0x04, PCI_WORD, wPciCmd | 0x0005);

	psDev->wBMIdeRegs = (UINT16)dPciReadReg (pfa, 0x20, PCI_WORD) & 0xfffc;
 	
	/* report what was found */
#ifdef DEBUG
    sprintf (buffer, "Testing %s controller @ bus:%02X, dev:%02X, func:%02X\n",
					psDev->bCtrlType == TYPE_IDE ? "IDE" : "SATA", 
					psDev->bBus, psDev->bDev, psDev->bFunc);

	vConsoleWrite (buffer);

	for (bTemp = 0; bTemp < 2; bTemp++)
	{	
		sprintf (buffer, "CMD %04x CTRL %04x BM %04x IRQ %02x\n", 
			psDev->sPortInfo[bTemp].wCmdReg,
			psDev->sPortInfo[bTemp].wCtrlReg,
			psDev->wBMIdeRegs,
			psDev->sPortInfo[bTemp].bIrq);
		vConsoleWrite (buffer);
	}
#endif

  	/* enable master and secondary controller */

	bTemp= (UINT8)dPciReadReg (pfa, 0x41, PCI_BYTE);
	vPciWriteReg (pfa, 0x41, PCI_BYTE, bTemp | 0x80);
	bTemp= (UINT8)dPciReadReg (pfa, 0x43, PCI_BYTE);
	vPciWriteReg (pfa, 0x43, PCI_BYTE, bTemp | 0x80);

	return (E__OK);

} /* dInitIde () */

/*****************************************************************************
 * bnport: Report maximum number of port supported by the controller (Could
 *         become board specific?)
 *
 * RETURNS: none
 */
static UINT8 bnport
(
		DEVICE *psDev
)
{
	PCI_PFA pfa;
	UINT32	dPciDevId;

	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	dPciDevId = dPciReadReg(pfa, 0x00, PCI_DWORD);
	if ((dPciDevId == 0x3b2e8086) || 
		(dPciDevId == 0x1c018086) ||  
		(dPciDevId == 0x1e038086) ||
		(dPciDevId == 0x1e018086) || 
		(dPciDevId == 0x1e098086) ||
		(dPciDevId == 0x8c098086))
	{
		return 4;
	}
	else if((dPciDevId == 0x8c018086) || dPciDevId == 0x1C3C8086)
	{
		return 6;
	}
	else
	{
		return 2;
}
}

/*****************************************************************************
 * vClosePciDevice: restore system once test complete
 *
 * RETURNS: none
 */

static void vClosePciDevice
( 
	DEVICE *psDev 
)
{
	UINT32	wPciCmd;
//	UINT8	bTemp;
	PCI_PFA pfa;

	/* make sure the device IO, memory, BMDMA access is disabled */
	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	wPciCmd = (UINT16)dPciReadReg (pfa, 0x04, PCI_WORD);
	vPciWriteReg (pfa, 0x04, PCI_WORD, wPciCmd & 0xFFF8);
	
#if 0
	/* disable SATA controller */

	if ( psDev->bCtrlType == TYPE_SATA )
	{
		bTemp = dPciReadReg (pfa, 0x92, PCI_BYTE);
		vPciWriteReg (pfa, 0x92, PCI_BYTE, bTemp & 0xfc);
	}
#endif
} /* vClosePciDevice () */


/*****************************************************************************
 * dIdeinterrupttest: test function for interrupt testing of IDE/ SATA 
 *						hard disk/ CF
 *
 * RETURNS: Test status
 */

static UINT32 dIdeinterrupttest 
(
	DEVICE *psDev, 
	UINT8 bInstance, 
	UINT8 bport
)
{
	UINT32 dtest_status;
	UINT16 wPort, wbmio, wCount;
	PCI_PFA pfa;
	UINT32 dtimeout = 200;
	UINT8 bdisk = 0xE0;

    if(bport == 2) /* For device 1*/
    {
    	bport = 0;
    	bdisk = 0xF0;
    }
    if(bport == 3) /* For device 1*/
    {
    	bport = 1;
    	bdisk = 0xF0;
    }
	wPort = (psDev->sPortInfo[bport].wCmdReg);
	pfa   = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	wbmio = (UINT16)dPciReadReg (pfa, 0x20 , PCI_WORD) & 0xfffc;

#ifdef DEBUG
	sprintf(buffer, "Before Issuing Command Port Addr: 0x%x, Port Status: 0x%x Ctrl: 0x%x\n",
					wPort, dIoReadReg(wPort + HDC_PORT_STATUS, REG_8), dPciReadReg (pfa, 0x04, PCI_WORD));
	vConsoleWrite(buffer);
#endif

	vIoWriteReg(wPort + HDC_PORT_SDH, REG_8, bdisk);
	/* Issue Identify command */
	vIoWriteReg(wPort + HDC_PORT_CMD, REG_8, HDC_CMD_IDENT);


#ifdef DEBUG
	sprintf(buffer, "After Issuing command Sts: 0x%x, BMIDEsts: 0x%x\n", 
				dIoReadReg(wPort + HDC_PORT_STATUS, REG_8), dIoReadReg(wbmio + 2 + (bport * 8), REG_8));
	vConsoleWrite(buffer);
#endif

	do{
		vDelay(10);
		dtimeout--;
		if(dIoReadReg(wbmio + 2 + (bport * 8), REG_8) & (1 << 2))
			break;
	}while(dtimeout > 0);

#ifdef DEBUG
	sprintf(buffer, "After Issuing command Sts: 0x%x, BMIDEsts: 0x%x\n", 
				dIoReadReg(wPort + HDC_PORT_STATUS, REG_8), dIoReadReg(wbmio + 2 + (bport * 8), REG_8));
	vConsoleWrite(buffer);
#endif

	if((dtimeout == 0) || (dtimeout < 0))
	{
		dtest_status = E__IDE_NO_INTERRUPT;

		/* Read awaiting 512 byte ID data if any! */
		for (wCount = 0; wCount < 256; wCount++)
			dIoReadReg(wPort + HDC_PORT_DATA, REG_16);
	}
	else
	{
		/* Read awaiting 512 byte ID data */
		for (wCount = 0; wCount < 256; wCount++)
			dIoReadReg(wPort + HDC_PORT_DATA, REG_16);

		/* Clear Interrupt */
		vIoWriteReg(wbmio + 2 + (bport * 8), REG_8, 1 << 2);
		
		dtest_status = E__OK;
	}
	return dtest_status;
}


/*****************************************************************************
 * dIdereadsect: test function for reading a sector from IDE/ SATA 
 *						hard disk/ CF
 *
 * RETURNS: Test status
 */

static UINT32 dIdereadsect 
(
	DEVICE *psDev, 
	UINT8 bInstance, 
	UINT8 bport
)
{
	UINT32 dtest_status;
	UINT16 wPort, wbmio;
	PCI_PFA pfa;
	UINT32 dtimeout = 200;
	UINT8 bdisk = 0xE0;
    UINT8 tReg;
    

#if 0 /* Not used, HAL no longer present */
	CCT_MEM_PTR bPtr,buf;
	
    /*Get a block of memory*/
    memset(&bPtr, 0, sizeof(CCT_MEM_PTR));
    memset(&buf,  0, sizeof(CCT_MEM_PTR));

    bPtr.dBlockSize = 0x1000;
    spMemGetBuffer(&bPtr);
    if( bPtr.pBlock == NULL )
    {
    	return E__BIT_IDE_NO_MEM;
    }

    buf.dBlockSize = 0x1000;
    spMemGetBuffer(&buf);
    if( buf.pBlock == NULL )
    {
    	return E__BIT_IDE_NO_MEM;
    }
#else
	PTR48	bPtr, buf;
    UINT32	bPtrAddr, bufAddr;
    UINT32	dHandle1, dHandle2;
	
	dHandle1 = dGetBufferPtr( 0x1000, &bPtr, &bPtrAddr );
	if (dHandle1 == E__FAIL)
	{
		vFreePtr (dHandle1);
		return E__BIT_IDE_NO_MEM;
	}
	
	dHandle2 = dGetBufferPtr( 0x1000, &buf, &bufAddr );
	if (dHandle2 == E__FAIL)
	{
		vFreePtr (dHandle1);
		vFreePtr (dHandle2);
		return E__BIT_IDE_NO_MEM;
	}
	
	memset( (void *)bPtrAddr, 0, 0x1000 );
    memset( (void *)bufAddr, 0, 0x1000 );
    
#endif

    if(bport == 2)
    {
    	bport = 0;
        bdisk = 0xF0;
    }
    if(bport == 3)
    {
    	bport = 1;
        bdisk = 0xF0;
    }

	wPort = (psDev->sPortInfo[bport].wCmdReg);
	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	wbmio = (UINT16)dPciReadReg (pfa, 0x20 , PCI_WORD) & 0xfffc;

	vIoWriteReg(wPort + HDC_PORT_LOCYL, REG_8, 0x0); /* Cylinder number 0 */
	vIoWriteReg(wPort + HDC_PORT_HICYL, REG_8, 0x0); /* Cylinder number 0 */
	vIoWriteReg(wPort + HDC_PORT_SECTNO, REG_8, 0x0);/* Sector number 0   */
	vIoWriteReg(wPort + HDC_PORT_NSECT, REG_8, 0x1); /* Number of Sectors */
	vIoWriteReg(wPort + HDC_PORT_SDH, REG_8, bdisk); /* E0 Head Number    */

#if 0 /* Not used, HAL no longer present */
	/* Set up PRD Table with one entry pad length with EOT */
	*(UINT32*)((UINT32)bPtr.qPhysicalAddress) = (UINT32)buf.qPhysicalAddress;
	*(UINT32*)((UINT32)bPtr.qPhysicalAddress + 4) = 0x200 | (1<<31);
	/* Reset Signature memory location */
	*(UINT16*)((UINT32)buf.qPhysicalAddress + 0x1FE) = 0;	
#else
	/* Set up PRD Table with one entry pad length with EOT */
	*(UINT32*)(bPtrAddr) = bufAddr;
	*(UINT32*)(bPtrAddr + 4) = 0x200 | (1<<31);
	/* Reset Signature memory location */
	*(UINT16*)(bufAddr + 0x1FE) = 0;		
#endif	

    /*Clear off the status registers*/
    tReg = dIoReadReg(wbmio + 2 + (bport * 8), REG_8);
    tReg |= 0x86;
    vIoWriteReg(wbmio + 2 + (bport * 8), REG_8,tReg);
       

#ifdef DEBUG
	sprintf(buffer, "Before Issuing command Sts: 0x%x, BMIDEsts: 0x%x BMIDECtl: 0x%x "
			"timeout: 0x%x Err.: 0x%x PRD: 0x%x Signature: 0x%x\n", 
			dIoReadReg(wPort + HDC_PORT_STATUS, REG_8), 
			dIoReadReg(wbmio + 2 + (bport * 8), REG_8),
			dIoReadReg(wbmio + (bport * 8), REG_8),
			dtimeout,
			dIoReadReg(wPort + HDC_PORT_ERROR, REG_8),
			dIoReadReg(wbmio + 4 + (bport * 8), REG_32),
#if 0 /* Not used, HAL no longer present */
			*(UINT16*)((UINT32)buf.qPhysicalAddress + 0x1FE));
#else
			*(UINT16*)(bufAddr + 0x1FE));
#endif			
	vConsoleWrite(buffer);
#endif

#if 0 /* Not used, HAL no longer present */
	/* Program the address in BMIDE Desc. table */
	vIoWriteReg (wbmio + 4 + (bport * 8), REG_32, (UINT32)bPtr.qPhysicalAddress);
#else
	/* Program the address in BMIDE Desc. table */
	vIoWriteReg (wbmio + 4 + (bport * 8), REG_32, bPtrAddr);
#endif

	/* Reset BMIDE DMA engine is direction clear and not active */	
	vIoWriteReg (wbmio + (bport * 8), REG_8, 
			dIoReadReg (wbmio + (bport * 8), REG_8) & (~((1 << 3) | (1))));

	/* Set BMIDE Direction of BMIDE Engine*/
	vIoWriteReg (wbmio + (bport * 8), REG_8, (1 << 3));

	/* Issue DMA Read Ext command */	
	vIoWriteReg (wPort + HDC_PORT_CMD, REG_8, 0xC8);

	/* Start the BMIDE DMA Engine */
	vIoWriteReg (wbmio + (bport * 8), REG_8, dIoReadReg (wbmio + (bport * 8), REG_8) | 1);

	do{
	vDelay(10);
	dtimeout--;

            /*Wait for the interrupt to occur*/
            if((dIoReadReg(wbmio + 2 + (bport * 8), REG_8) & (1 << 2)) &&
                    !(dIoReadReg(wPort + HDC_PORT_STATUS, REG_8) & 0x80))
		break;
	}while(dtimeout > 0);

	if((dtimeout == 0) || (dtimeout < 0))
	{
		vDelay(1);
		vIoWriteReg (wbmio + (bport * 8), REG_8, (dIoReadReg(wbmio, REG_8)) & (~(1 | (1 << 3))));
		vDelay(1);
		/* Read IDE Status Register */
		dIoReadReg(wPort + HDC_PORT_STATUS, REG_8);
	
#ifdef DEBUG
                sprintf(buffer,"No Interrupt\n");
                vConsoleWrite(buffer);

		sprintf(buffer, "After Issuing command Sts: 0x%x, BMIDEsts: 0x%x BMIDECtl: 0x%x " 
				"timeout: 0x%x Err.: 0x%x PRD: 0x%x Signature: 0x%x NI\n",
				dIoReadReg(wPort + HDC_PORT_STATUS, REG_8), 
				dIoReadReg(wbmio + 2 + (bport * 8), REG_8),
				dIoReadReg(wbmio + (bport * 8), REG_8),
				dtimeout,
				dIoReadReg(wPort + HDC_PORT_ERROR, REG_8),
				dIoReadReg(wbmio + 4 + (bport * 8), REG_32),
#if 0 /* Not used, HAL no longer present */
			*(UINT16*)((UINT32)buf.qPhysicalAddress + 0x1FE));
#else
			*(UINT16*)(bufAddr + 0x1FE));
#endif					
		vConsoleWrite(buffer);
#endif
		dtest_status = E__IDE_NO_INTERRUPT;
	}
	else
	{

		/* Poll for BMIDE ACTIVE bit to clear the BMIDE START bit */
		/*do{
		vDelay(10);
		dtimeout--;
		if(!(dIoReadReg(wbmio + 2 + (bport * 8), REG_8) & 1))
			break;
		}while(dtimeout > 0);*/

		/* Clear the BMIDE DMA Start Bit */
		vDelay(1);
		vIoWriteReg (wbmio + (bport * 8), REG_8, (dIoReadReg(wbmio, REG_8)) & (~(1 | (1 << 3))));
		vDelay(1);
		/* Read IDE Status Register */
		dIoReadReg(wPort + HDC_PORT_STATUS, REG_8);

		/* Clear Interrupt */
		vIoWriteReg(wbmio + 2 + (bport * 8), REG_8, dIoReadReg(wbmio + 2 + (bport * 8), REG_8));

#ifdef DEBUG
		sprintf(buffer, "After Issuing command Sts: 0x%x, BMIDEsts: 0x%x BMIDECtl: 0x%x "
				"timeout: 0x%x Err.: 0x%x PRD: 0x%x Signature: 0x%x\n",
				dIoReadReg(wPort + HDC_PORT_STATUS, REG_8), 
				dIoReadReg(wbmio + 2 + (bport * 8), REG_8),
				dIoReadReg(wbmio + (bport * 8), REG_8),
				dtimeout,
				dIoReadReg(wPort + HDC_PORT_ERROR, REG_8),
				dIoReadReg(wbmio + 4 + (bport * 8), REG_32),
#if 0 /* Not used, HAL no longer present */
			*(UINT16*)((UINT32)buf.qPhysicalAddress + 0x1FE));
#else
			*(UINT16*)(bufAddr + 0x1FE));
#endif					
		vConsoleWrite(buffer);
#endif

#if 0 /* Not used, HAL no longer present */
		if((*(UINT16*)((UINT32)buf.qPhysicalAddress + 0x1FE)) == 0xAA55)
#else
		if((*(UINT16*)(bufAddr + 0x1FE)) == 0xAA55)
#endif		
			dtest_status = E__OK;
		else
			dtest_status = E__IDE_NO_SIGN;
	}

#if 0 /* Not used, HAL no longer present */	
        spMemFreeAll(&bPtr);/* Fix spMemFreeMem */
        /*spMemFreeAll(&buf);*/
#else
	vFreePtr (dHandle1);
	vFreePtr (dHandle2);
	/* We must now free all the kernel heap memory allocated via dGetBufferPtr(), */
	/* as there is no complement function for doing this for individual allocations!! */
	sysMmuFreeAll();
	sysGdtDeleteAllSlots();
#endif

	return dtest_status;
}


/*****************************************************************************
 * IdeRegaccessTest: test function for non destructive read and write of
 *						IDE Registers to test the presence of hard disk/ CF
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IdeRegaccessTest, "IDE Disk Connectivity Test")
{
	UINT32 dideStatus;
	UINT32 dtest_status = E__TEST_NO_DEVICE;
	UINT8 bInstance = 0, bLoop = 0, bport = 0;
	UINT8 bList = 0, bCount = 0, k;
	DEVICE sDevice;	


	if(adTestParams[0] >= 2)
	{
		if(adTestParams[0] % 2 != 0)
		{
			sprintf(buffer, "Error[%x] Wrong Parameters", E__BIT_IDE_PARAMS);
			postErrorMsg(buffer);
			return(E__BIT_IDE_PARAMS);
		}

		bCount = adTestParams[0] / 2;
		bLoop  = 1;
		bList  = 1;
	}
	else	
	{
		bLoop = 1;
		bInstance++;
		bport++;
	}


	k = 0;
	do
	{
		if(bList)
		{
			if(k >= bCount)
			{
				bLoop  = 0;
				bList  = 0;
				continue;
			}
			bInstance = adTestParams[1 + (k*2)];
			bport     = adTestParams[2 + (k*2)];
			k++;
		}

		#ifdef DEBUG
			sprintf(buffer, "Instance to find: %d: Port: %d\n", bInstance,
					adTestParams[2]);
			vConsoleWrite(buffer);
		#endif

		dideStatus = dInitIde (&sDevice, bInstance); /* bInstance - Controller instance */
		
		if (dideStatus == E__OK)
		{
			if(bport > bnport(&sDevice))
			{
				#ifdef DEBUG  
      				vConsoleWrite("Unable to locate instance of IDE port\n"); 
				#endif
      			sprintf(buffer, "Error[%x] No Port, Inst:%d, Port:%d", E__TEST_NO_PORT, bInstance, bport);
      			vConsoleWrite(buffer);
      			postErrorMsg(buffer);
				return (E__TEST_NO_PORT);
			}

			dtest_status = dIderegtest(&sDevice, bport - 1); /* bport - Port number */

			#ifdef DEBUG
				sprintf(buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
			#endif

			if( (dtest_status == E__OK) && (bList != 1))
			{
				if((adTestParams[1] == 0) && (adTestParams[2] != 0))
				{
					bInstance++;
					bLoop = 1;
				}
				if((adTestParams[2] == 0) && (adTestParams[1] != 0))
				{
					if(bport < bnport(&sDevice))
					{
						bport++;
						bLoop = 1;
					}
					else
						bLoop = 0;
				}
				if((adTestParams[1] == 0) && (adTestParams[2] == 0))
				{
					if(bport < bnport(&sDevice))
					{
						bport++;
						bLoop = 1;
					}
					else
					{
						bport = 1;
						bInstance++;
						bLoop = 1;
					}
				}		
			}	
		}
		else if (bLoop == 0 || (bList == 1))
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of IDE\n"); 
			#endif
      		sprintf(buffer, "Error[%x] No Inst, Inst:%d, Port:%d", E__TEST_NO_DEVICE, bInstance, bport);
      		postErrorMsg(buffer);
			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

	} while ((bLoop == 1) && (dtest_status == E__OK));		/* test all requested instances */


	vClosePciDevice(&sDevice);
	if((dtest_status != E__OK) && (dtest_status != E__TEST_NO_DEVICE))
	{
		dtest_status += bInstance - 1;
		#ifdef DEBUG
			sprintf(buffer, "Error[%x] Inst:%d, Port:%d", dtest_status, bInstance, bport);
			vConsoleWrite(buffer);
			vConsoleWrite("\n");
		#endif
		postErrorMsg(buffer);
	}
	return (dtest_status);

} /* IdeRegaccessTest */


/*****************************************************************************
 * IdeIntTest: test function for interrupt of IDE/ SATA hard disk/ CF
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IdeIntTest, "IDE Interrupt Test")
{
	UINT32 dideStatus;
	UINT32 dtest_status = E__TEST_NO_DEVICE;
	UINT8 bInstance = 0, bLoop = 0, bport = 0;
	UINT8 bList = 0, bCount = 0, k;
	DEVICE sDevice;	

	if(adTestParams[0] >= 2)
	{
		if(adTestParams[0] % 2 != 0)
		{
			sprintf(buffer, "Error[%x] Wrong Parameters", E__BIT_IDE_PARAMS);
			postErrorMsg(buffer);
			return(E__BIT_IDE_PARAMS);
		}

		bCount = adTestParams[0] / 2;
		bLoop  = 1;
		bList  = 1;
	}
	else	
	{
		bLoop = 1;
		bInstance++;
		bport++;
	}

	k = 0;
	do
	{
		if(bList)
		{
			if(k >= bCount)
			{
				bLoop  = 0;
				bList  = 0;
				continue;
			}
			bInstance = adTestParams[1 + (k*2)];
			bport     = adTestParams[2 + (k*2)];
			k++;
		}

		#ifdef DEBUG
			sprintf(buffer, "bCount %d, bLoop %d, bList %d\n", bCount, bLoop, bList);
			vConsoleWrite(buffer);
			sprintf(buffer, "Instance to find: %d: Port: %d\n", bInstance,
					adTestParams[2]);
			vConsoleWrite(buffer); 
		#endif

		dideStatus = dInitIde (&sDevice, bInstance); /* bInstance - Controller instance */
		
		if (dideStatus == E__OK)
		{
			if(bport > bnport(&sDevice))
			{
				#ifdef DEBUG  
      				vConsoleWrite("Unable to locate instance of IDE port\n"); 
				#endif
				sprintf(buffer, "Error[%x] No Port, Inst:%d, Port:%d", E__TEST_NO_PORT, bInstance, bport);
      			postErrorMsg(buffer);
				return (E__TEST_NO_PORT);
			}

			dtest_status = dIdeinterrupttest(&sDevice, bInstance, bport - 1); /* bport - Port number */

			#ifdef DEBUG
				sprintf(buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
			#endif

			if((dtest_status == E__OK) && (bList != 1))
			{
				if((adTestParams[1] == 0) && (adTestParams[2] != 0))
				{
					bInstance++;
					bLoop = 1;
				}
				if((adTestParams[2] == 0) && (adTestParams[1] != 0))
				{
					if(bport < bnport(&sDevice))
					{
						bport++;
						bLoop = 1;
					}
					else
						bLoop = 0;
				}
				if((adTestParams[1] == 0) && (adTestParams[2] == 0))
				{
					if(bport < bnport(&sDevice))
					{
						bport++;
						bLoop = 1;
					}
					else
					{
						bport = 1;
						bInstance++;
						bLoop = 1;
					}
				}		
			}	
		}
		else if ( (bLoop == 0) || (bList == 1))
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of IDE\n"); 
			#endif
			sprintf(buffer, "Error[%x] No Inst, Inst:%d, Port:%d", E__TEST_NO_DEVICE, bInstance, bport);
      		postErrorMsg(buffer);
			return (E__TEST_NO_DEVICE);
		}
		else	/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

	} while ((bLoop == 1) && (dtest_status == E__OK));		/* test all requested instances */

	vClosePciDevice(&sDevice);
	if((dtest_status != E__OK) && (dtest_status != E__TEST_NO_DEVICE))
	{
		dtest_status += bInstance - 1;
		#ifdef DEBUG
			sprintf(buffer, "Error[%x] Inst:%d, Port:%d", dtest_status, bInstance, bport);
			vConsoleWrite(buffer);
			vConsoleWrite("\n");
		#endif
		postErrorMsg(buffer);
	}
	return (dtest_status);

} /* IdeIntTest */


/*****************************************************************************
 * IdeRdSectTest: test function to read first sector of the IDE/ SATA hard disk/ CF
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (IdeRdSectTest, "IDE Read First Sector Test")
{
	UINT32 dideStatus;
	UINT32 dtest_status = E__TEST_NO_DEVICE;
	UINT8 bInstance = 0, bLoop = 0, bport = 0;
	UINT8 bList = 0, bCount = 0, k;
	DEVICE sDevice;	

	if(adTestParams[0] >= 2)
	{
		if(adTestParams[0] % 2 != 0)
		{
			sprintf(buffer, "Error[%x] Wrong Parameters", E__BIT_IDE_PARAMS);
			postErrorMsg(buffer);
			return(E__BIT_IDE_PARAMS);
		}

		bCount = adTestParams[0] / 2;
		bLoop  = 1;
		bList  = 1;
	}
	else	
	{
		bLoop = 1;
		bInstance++;
		bport++;
	}

	k = 0;
	do
	{
		if(bList)
		{
			if(k >= bCount)
			{
				bLoop  = 0;
				bList  = 0;
				continue;
			}
			bInstance = adTestParams[1 + (k*2)];
			bport     = adTestParams[2 + (k*2)];
			k++;
		}

		#ifdef DEBUG
			sprintf(buffer, "bCount %d, bLoop %d, bList %d\n", bCount, bLoop, bList);
			vConsoleWrite(buffer);
			sprintf(buffer, "Instance to find: %d: Port: %d\n", bInstance,
					adTestParams[2]);
			vConsoleWrite(buffer); 
		#endif

		dideStatus = dInitIde (&sDevice, bInstance); /* bInstance - Contoller instance */
		
		if (dideStatus == E__OK)
		{
			if(bport > bnport(&sDevice))
			{
				#ifdef DEBUG  
      				vConsoleWrite("Unable to locate instance of IDE port\n"); 
				#endif
				sprintf(buffer, "Error[%x] No Port, Inst:%d, Port:%d", E__TEST_NO_PORT, bInstance, bport);
      			postErrorMsg(buffer);
				return (E__TEST_NO_PORT);
			}

            sysCacheDisable();
			dtest_status = dIdereadsect(&sDevice, bInstance, bport - 1); /* bport - Port number */
            sysCacheEnable();

			#ifdef DEBUG
				sprintf(buffer, "Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
			#endif

			if(dtest_status == E__OK && (bList != 1))
			{
				if((adTestParams[1] == 0) && (adTestParams[2] != 0))
				{
					bInstance++;
					bLoop = 1;
				}
				if((adTestParams[2] == 0) && (adTestParams[1] != 0))
				{
					if(bport < bnport(&sDevice))
					{
						bport++;
						bLoop = 1;
					}
					else
						bLoop = 0;
				}
				if((adTestParams[1] == 0) && (adTestParams[2] == 0))
				{
					if(bport < bnport(&sDevice)) /* Next port */
					{
						bport++;
						bLoop = 1;
					}
					else /* Next controller from port 1 */
					{
						bport = 1;
						bInstance++;
						bLoop = 1;
					}
				}		
			}	
		}
		else if( (bLoop == 0) || (bList == 1) )
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of IDE\n"); 
			#endif
			sprintf(buffer, "Error[%x] No Inst, Inst:%d, Port:%d", E__TEST_NO_DEVICE, bInstance, bport);
      		postErrorMsg(buffer);
			return (E__TEST_NO_DEVICE);
		}
		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

	} while ((bLoop == 1) && (dtest_status == E__OK));		/* test all requested instances */

	vClosePciDevice(&sDevice);
	if((dtest_status != E__OK) && (dtest_status != E__TEST_NO_DEVICE))
	{
		dtest_status += bInstance - 1;
		#ifdef DEBUG
			sprintf(buffer, "Error[%x] Inst:%d, Port:%d", dtest_status, bInstance, bport);
			vConsoleWrite(buffer);
			vConsoleWrite("\n");
		#endif
		postErrorMsg(buffer);
	}
	return (dtest_status);

} /* IdeRdSectTest */





/*****************************************************************************
 * IdeRdSectTest: test function to read first sector of the IDE/ SATA hard disk/ CF
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IdeFlashDriveTest, "IDE Flash Drive Test")
{
	UINT32 dideStatus;
	UINT32 dtest_status = E__TEST_NO_DEVICE;
	DEVICE sDevice;
	FLASH_DRV_PARAMS  params;

	if(board_service(SERVICE__BRD_GET_FLASH_DRV_PARAMS, NULL, &params) != E__OK)
	{
		#ifdef DEBUG
			sprintf(buffer, "No Flash Drive Parameters present\n");
			vConsoleWrite(buffer);
		#endif
		return E__TEST_NO_DEVICE;
	}

	#ifdef DEBUG
		sprintf(buffer, "Instance to find: %d: Port: %d\n", params.bInstance,
				params.bport);
		vConsoleWrite(buffer);
	#endif
	dideStatus = dInitIde (&sDevice, params.bInstance); /* bInstance - Controller instance */

	if (dideStatus == E__OK)
	{
		if (params.bport > bnport(&sDevice))
		{
			#ifdef DEBUG
      			vConsoleWrite("Unable to locate instance of IDE port\n");
			#endif
			return (E__TEST_NO_PORT);
		}

		dtest_status = dIderegtest(&sDevice, params.bport - 1); /* bport - Port number */
		#ifdef DEBUG
			sprintf(buffer, "IDE Reg Test status: 0x%x\n", dtest_status);
			vConsoleWrite(buffer);
		#endif

		if(dtest_status == E__OK)
		{
			dtest_status = dIdeinterrupttest(&sDevice, params.bInstance, params.bport - 1); /* bport - Port number */
			#ifdef DEBUG
				sprintf(buffer, "IDE Int Test status: 0x%x\n", dtest_status);
				vConsoleWrite(buffer);
			#endif
		}
	}

	vClosePciDevice(&sDevice);
	if((dtest_status != E__OK) && (dtest_status != E__TEST_NO_DEVICE))
	{
		dtest_status += params.bInstance - 1;
		#ifdef DEBUG
			sprintf(buffer, "Error in inst.: %d\n", params.bInstance);
			vConsoleWrite(buffer);
		#endif
	}

	return (dtest_status);

} /* IdeRdSectTest */



