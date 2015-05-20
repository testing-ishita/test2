/************************************************************************
 *                                                                      *
 *      Copyright 2015 Concurrent Technologies, all rights reserved.    *
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
 * cpciTests.c
 *
 *  Created on: 23 Mar 2015
 *   Author: hchalla
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/cpciTests.c,v 1.1 2015-04-01 16:39:22 hchalla Exp $
 * $Log: cpciTests.c,v $
 * Revision 1.1  2015-04-01 16:39:22  hchalla
 * Initial Version of CPCI backplane tests.
 *
 */

#include <errors.h>
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>

#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>
#include <bit/board_service.h>
#include <private/debug.h>
#include "config.h"
#include <bitops.h>

#define	E__TEST_NO_DEVICE		E__BIT
#define E__PI7X130_ACCESS		E__BIT  + 0x01
#define E__TEST_NO_MEM			E__BIT  + 0x02
#define E__TEST_NO_INT			E__BIT  + 0x03
#define E__TEST_CPCIDMA_ERR		E__BIT  + 0x04
#define E__TEST_CPCILB_FAIL		E__BIT  + 0x05
#define E__TEST_CPCIBUSS_ERR		E__BIT  + 0x06
#define E__TES_CPCINMI_ERR		E__BIT  + 0x07
#define E__CPCI_WERR				E__BIT  + 0x08
#define E__BSWERR				E__BIT  + 0x09
#define E__BSRERR				E__BIT  + 0x0A
#define E__CPCI_SIMPLE_RXFER		E__BIT  + 0x10
#define E__CPCI_SIMPLE_WXFER	E__BIT  + 0x20
#define E__CPCI_DMA_XFER			E__BIT  + 0x30
#define E_CPCI_DMA_BUSY         E__BIT  + 0x31
#define E_CPCI_NO_DEVICE         E__BIT  + 0x32

#define _4KB_DWORD  (0x1000 / sizeof(UINT32))

#define PCI_CSR		0x4		/* PCI Configuration Space Control & Status */
#define S_TA	0x08000000	/* target abort */

#define osWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define osWriteWord(regAddr,value)  (*(UINT16*)(regAddr) = value)
#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define osReadByte(regAddr)			(*(UINT8*)(regAddr))
#define osReadWord(regAddr)			(*(UINT16*)(regAddr))
#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))

#define DEBUG_SOAK
#define DEBUG

static PTR48 tPtr2,tPtr3;

PCI_PFA	pfa;


/******************************************************************************
*
* dInitPericomPI7X130
*
* The function performs PI7X130 Initialisations
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 dInitPericomPI7X130( void )
{
	UINT32	regVal;
	UINT32	result = E__TEST_NO_DEVICE;

	UINT8 bBus=0,bDevice=0,bOccurance=0;
	UINT16 wVenId =0;
	UINT32 dTemp=0,dLocalBar=0,dBar=0;
	PCI_PFA pfa;

#ifdef DEBUG_SOAK
	char buffer[80];
#endif

	board_service(SERVICE__BRD_GET_CPCI_INSTANCE,NULL,&bOccurance);

	if (iPciFindDeviceById ( bOccurance ,0x12D8,0xE130,&pfa) == E__OK)
	{
#ifdef DEBUG
		sprintf(buffer,"Found Pericom bridge at %x:%x.%x\n", PCI_PFA_BUS(pfa), PCI_PFA_DEV(pfa), PCI_PFA_FUNC(pfa));
		vConsoleWrite(buffer);
#endif

		/*Enable the bus mastering and memory/io space access*/
		regVal=PCI_READ_WORD(pfa,PCI_COMMAND);
		regVal |= 0x07;
		PCI_WRITE_WORD(pfa,PCI_COMMAND,regVal);

		/*Read the CPCI status register*/
		regVal = PCI_READ_DWORD(pfa, PCI_CSR);

		// clear  error status
		if ((regVal & S_TA) == S_TA)
		{
			regVal |= S_TA;
			PCI_WRITE_DWORD(pfa,PCI_CSR, regVal);
		}
		return E__OK;
	}

	return result;
}


/******************************************************************************
*
* readDwordPericom
*
* reads data from the CPCI bus and checks the CPCI bus error status
*
* RETURNS: 0 if no bus error else error code
*
******************************************************************************/
UINT32 readDwordPericom (UINT32 *addr, UINT32 *val)
{
	UINT32 regVal;
#ifdef DEBUG
	char buffer[80];
#endif

	/*Read the CPCI status register*/
	regVal = PCI_READ_DWORD(pfa, PCI_CSR);

	// clear  error status
	if ((regVal & S_TA) == S_TA)
	{
		regVal |= S_TA;
		PCI_WRITE_DWORD(pfa,PCI_CSR, regVal);
	}

	// read data from CPCI bus
	//*val = *addr;
	*val = osReadDWord(addr);


	// check bus status
	regVal = PCI_READ_DWORD(pfa, PCI_CSR);
	if ((regVal & S_TA) == S_TA)
	{
#ifdef DEBUG
		sprintf(buffer, "Sig Target Abort error:%08X \n",  regVal);
		sysDebugWriteString(buffer);
		sprintf(buffer, "addr:%08X,val:%08X \n",   addr,*val);
		sysDebugWriteString(buffer);
#endif
		regVal |= S_TA;
		PCI_WRITE_DWORD(pfa,PCI_CSR, regVal);
#ifdef DEBUG
		regVal = PCI_READ_DWORD(pfa, PCI_CSR);
		sprintf(buffer, "Sig Target Abort Error:%08X\n", regVal);
		sysDebugWriteString(buffer);
#endif
		return E__TEST_CPCIBUSS_ERR;
	}
	else
	{
		return E__OK;
	}
}

/******************************************************************************
*
* writeDwordPericom
*
* Writes data to the CPCI bus and checks the CPCI bus error status
*
* RETURNS: 0 if no bus error else error code
*
******************************************************************************/
UINT32 writeDwordPericom (UINT32 *addr, UINT32 val)
{
	UINT32 regVal;
	UINT32 maxRetry=0x10;
#ifdef DEBUG
	UINT32 temp;
	char buffer[80];
#endif


	/*Read the CPCI status register*/
	regVal = PCI_READ_DWORD(pfa, PCI_CSR);

	// clear  error status
	if ((regVal & S_TA) == S_TA)
	{
		regVal |= S_TA;
		PCI_WRITE_DWORD(pfa,PCI_CSR, regVal);
	}


	// write data to CPCI bus
	osWriteDWord(addr,val);

	// check bus status
	regVal = PCI_READ_DWORD(pfa, PCI_CSR);
	if ((regVal & S_TA) == S_TA)
	{
#ifdef DEBUG
		sprintf(buffer, "SigAbort error:%08X \n",   regVal);
		sysDebugWriteString(buffer);

		sprintf(buffer, "addr:%08X,val:%08X \n",   addr,val);
		sysDebugWriteString(buffer);
#endif
		regVal |= S_TA;
		PCI_WRITE_DWORD(pfa,PCI_CSR, regVal);
#ifdef DEBUG
		regVal = PCI_READ_DWORD(pfa, PCI_CSR);
		sprintf(buffer, "SigAbort error :%08X\n", regVal);
		sysDebugWriteString(buffer);
#endif
		return E__TEST_CPCIBUSS_ERR;
	}
	else
	{
		return E__OK;
	}
}

/******************************************************************************
*
* dTestPeripheralBoard
*
* This will do a CPCI bus transfers from CPCI peripheral board on to Syscon.
*
* RETURNS: 0 if no bus error else error code
*
******************************************************************************/
UINT32 dTestPeripheralBoard(CPCI_DRV_INFO *pCpciDrvInfo)
{
	UINT32 dMaxSize=0;
	UINT32	*address1;
	UINT32	value;
	UINT32	cnt;
	UINT32 pHandle, pciAddr=0;
	UINT32 rt = E__OK;
	//CPCI_DRV_INFO *cpciDrvInfo;

	board_service(SERVICE__BRD_GET_CPCI_UPSTREAM_IMAGE, NULL, &pCpciDrvInfo);

	pHandle = dGetPhysPtr(pCpciDrvInfo->upStreamAddress,0x10000,&tPtr2,(void*)&pciAddr);

	if (pHandle == E__FAIL)
	{
		return E__TEST_NO_MEM;
	}

	dMaxSize = _4KB_DWORD;
	address1 = pciAddr;

	if (rt == E__OK)
	{
		for (cnt = 0; cnt < dMaxSize; ++cnt)
		{

			rt = writeDwordPericom(&address1[cnt], 0);
			if (rt != E__OK)
			{
				break;
			}
		}

		if (rt == E__OK)
		{
			for (cnt = 0; cnt < dMaxSize; ++cnt)
			{

				rt = readDwordPericom(&address1[cnt], &value);
				if (rt != E__OK)
				{
					break;
				}
				if (value != 0)
				{
					rt = E__CPCI_SIMPLE_RXFER + (0x100 * pCpciDrvInfo->Currnetslot)+ 1;
					break;
				}
			}
		}
	}

	if (rt == E__OK)
	{
		for (cnt = 0; cnt < dMaxSize; ++cnt)
		{
			rt = writeDwordPericom(&address1[cnt], 0x55555555);
			if (rt != E__OK)
			{
				break;
			}
		}
		if (rt == E__OK)
		{
			for (cnt = 0; cnt < dMaxSize; ++cnt)
			{
				rt = readDwordPericom(&address1[cnt], &value);
				if (rt != E__OK)
				{
					break;
				}
				if (value != 0x55555555)
				{
					rt = E__CPCI_SIMPLE_RXFER +(0x100 * pCpciDrvInfo->Currnetslot) + 2;
					break;
				}
			}
		}
	}

	if (rt == E__OK)
	{
		for (cnt = 0; cnt < dMaxSize; ++cnt)
		{

			rt = writeDwordPericom(&address1[cnt], 0xAAAAAAAA);
			if (rt != E__OK)
			{
				break;
			}
		}

		if (rt == E__OK)
		{
			for (cnt = 0; cnt < dMaxSize; ++cnt)
			{

				rt = readDwordPericom(&address1[cnt], &value);

				if (rt != E__OK)
				{
					break;
				}
				if (value != 0xAAAAAAAA)
				{
					rt = E__CPCI_SIMPLE_RXFER + (0x100 * pCpciDrvInfo->Currnetslot)+ 3;
					break;
				}
			}
		}
	}

	if (pHandle)
	{
		vFreePtr(pHandle);
	}

	return rt;
}

/******************************************************************************
*
* dTestSysconBoard
*
* This will do a CPCI bus transfers from CPCI system controller on to all
*  visible CPCI peripheral boards on the backplane.
*
* RETURNS: 0 if no bus error else error code
*
******************************************************************************/
UINT32 dTestSysconBoard(CPCI_DRV_INFO *pCpciDrvInfo)
{
	UINT32 dMaxSize=0;
	UINT32	*address1;
	UINT32	value;
	UINT32	cnt;
	UINT32 pHandle, pciAddr=0;
	UINT32 rt = E__OK;
	UINT8 bIndex=0;
	UINT8 bNoFitCnt = 0;

	dMaxSize = _4KB_DWORD;

	for (bIndex =0; bIndex < MAX_BOARDS; bIndex++)
	{
		if (pCpciDrvInfo->slaveBoardPresent[bIndex] == 1)
		{
			pHandle = dGetPhysPtr(pCpciDrvInfo->downStreamAddress[bIndex],0x10000,&tPtr3,(void*)&pciAddr);

			if (pHandle == E__FAIL)
			{
				return E__TEST_NO_MEM;
			}

			dMaxSize = _4KB_DWORD;
			address1 = pciAddr;

			if (rt == E__OK)
			{
				for (cnt = 0; cnt < dMaxSize; ++cnt)
				{

					rt = writeDwordPericom(&address1[cnt], 0);
					if (rt != E__OK)
					{
						break;
					}
				}

				if (rt == E__OK)
				{
					for (cnt = 0; cnt < dMaxSize; ++cnt)
					{

						rt = readDwordPericom(&address1[cnt], &value);
						if (rt != E__OK)
						{
							break;
						}
						if (value != 0)
						{
							rt = E__CPCI_SIMPLE_RXFER + (0x100 * pCpciDrvInfo->slaveBoardSlotId[bIndex]) + 1;
							break;
						}
					}
				}
			}

			if (rt == E__OK)
			{
				for (cnt = 0; cnt < dMaxSize; ++cnt)
				{
					rt = writeDwordPericom(&address1[cnt], 0x55555555);
					if (rt != E__OK)
					{
						break;
					}
				}
				if (rt == E__OK)
				{
					for (cnt = 0; cnt < dMaxSize; ++cnt)
					{
						rt = readDwordPericom(&address1[cnt], &value);
						if (rt != E__OK)
						{
							break;
						}
						if (value != 0x55555555)
						{
							rt = E__CPCI_SIMPLE_RXFER + (0x100 * pCpciDrvInfo->slaveBoardSlotId[bIndex]) + 2;
							break;
						}
					}
				}
			}

			if (rt == E__OK)
			{
				for (cnt = 0; cnt < dMaxSize; ++cnt)
				{

					rt = writeDwordPericom(&address1[cnt], 0xAAAAAAAA);
					if (rt != E__OK)
					{
						break;
					}
				}

				if (rt == E__OK)
				{
					for (cnt = 0; cnt < dMaxSize; ++cnt)
					{

						rt = readDwordPericom(&address1[cnt], &value);

						if (rt != E__OK)
						{
							break;
						}
						if (value != 0xAAAAAAAA)
						{
							rt = E__CPCI_SIMPLE_RXFER + (0x100 * pCpciDrvInfo->slaveBoardSlotId[bIndex]) + 3;
							break;
						}
					}
				}
			}

			if (pHandle)
			{
				vFreePtr(pHandle);
			}

		} /*if (pCpciDrvInfo->slaveBoardPresent[bIndex] == 1)*/
		else
		{
			bNoFitCnt++;
		}
	} /*for (bIndex =0; bIndex < MAX_BOARDS; bIndex++)*/

	if (bNoFitCnt == MAX_BOARDS)
		rt = E_CPCI_NO_DEVICE;

	return rt;
}
/******************************************************************************
*
* CPCISimpleTransferTest
*
* VME Bridge backplane simple transfer test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (CPCISimpleTransferTest, "CPCI Backplane Data Transfer Test")
{
	CPCI_DRV_INFO *cpciDrvInfo;
	UINT32	rt = E__OK;
	UINT8	slot;
	UINT8	bridgePresent;

	UINT8 bSysCon;
#ifdef DEBUG
	char achBuffer[80];
#endif

	board_service(SERVICE__BRD_GET_CPCI_BRIDGE_IMAGES, NULL, &cpciDrvInfo);

	rt = dInitPericomPI7X130();

	board_service(SERVICE__BRD_CHECK_CPCI_IS_SYSCON,NULL,&bSysCon);

	if (bSysCon == 1)
	{
		/*
		 *  Do Peripheral board tests for upstream window
		 */
		rt =  dTestPeripheralBoard(cpciDrvInfo);
	}
	else
	{
		/*
		 * Do Syscon Board Tests for all the peripheral boards with the image windows.
		 */
		board_service(SERVICE__BRD_GET_CPCI_SLAVE_IMAGES,NULL,&cpciDrvInfo);
		rt = dTestSysconBoard(cpciDrvInfo);
	}

	return rt;
}

