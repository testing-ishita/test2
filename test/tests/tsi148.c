
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

/* tsi148.c - PCI Bus related tests
 */


/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/tsi148.c,v 1.13 2015-03-23 10:55:36 mgostling Exp $
 * $Log: tsi148.c,v $
 * Revision 1.13  2015-03-23 10:55:36  mgostling
 * Further reduction in the amount of data transfered whilst the VME bus is reserved in VMESimpleTransferTest.
 * Fixed slot-based delay calculation whilst tryingto acquire the VME bus.
 *
 * Revision 1.12  2015-03-19 16:55:38  mgostling
 * Reduced the time the VME bus is reserved by the TSI148 during each test.
 *
 * Revision 1.11  2015-03-17 16:55:22  mgostling
 * Added dAcquireTsi148VMEBus anddReleaseTsi148VMEBus functions to obtain
 * exclusive access to VME Bus on the backplane and avoid bus contention and
 * BUSERR being generated.
 *
 * Revision 1.10  2015-03-17 10:43:30  hchalla
 * Added dAcquireUniverseVMEBus, dReleaseUniverseVMEBus functions to exclusive
 * access to VME Bus on the backplane between the boards, this will avoid
 * any bus contention and BUSERR being generated.
 *
 * Revision 1.9  2015-03-12 16:04:28  mgostling
 * Fixed a problem with VMEBridgelessInterfaceTest once debugging code was disabled.
 *
 * Revision 1.8  2015-03-12 15:30:58  hchalla
 * Changes to Universe VME related tests for configuring Inbound outBound
 * Image sizes.
 *
 * Revision 1.7  2015-03-10 16:29:09  mgostling
 * Tidied up code and test header descriptions.
 *
 * Revision 1.6  2015-03-10 09:59:17  hchalla
 * Added Universe DMA tests and VME Bus Error Tests.
 *
 * Revision 1.5  2015-03-05 11:48:12  hchalla
 * Added VME Backplane tests.
 *
 * Revision 1.4  2015-02-27 14:32:07  mgostling
 * Added new test for VME signals on bridgeless boards
 * Added new VME data transfer tests for ethernet Soak Master
 *
 * Revision 1.3  2013-11-25 13:02:03  mgostling
 * Added missing CVS headers.
 *
 *
 */

/* includes */
/*****************************************************************************/
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
#include <devices/tsi148.h>
#include <devices/universe.h>
#include "config.h"
#include <bitops.h>

/* defines */
//#define DEBUG
#ifdef DEBUG
static char buffer[128];
#endif

#define INT_PIN_NUM				2  /* Found by experimentation */
#define VME_IMAGE_BASE_ADDR		0x00500000L
#define TEST_DMA_ADDR			0x00700000L
#define TEST_VME_ADDR			0xFFFF0000L

#define WRITE_VME 				0
#define READ_VME 				1

#define _1MB_DWORD (0x00100000 / sizeof(UINT32))
#define _512KB_DWORD (0x80000 / sizeof(UINT32))
#define _128KB_DWORD (0x20000 / sizeof(UINT32))
#define _4KB_DWORD  (0x1000 / sizeof(UINT32))
#define _8KB_DWORD  (0x2000 / sizeof(UINT32))


#define osWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define osWriteWord(regAddr,value)  (*(UINT16*)(regAddr) = value)
#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define osReadByte(regAddr)			(*(UINT8*)(regAddr))
#define osReadWord(regAddr)			(*(UINT16*)(regAddr))
#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))

#ifndef INCLUDE_LWIP_SMC
static PTR48 tPtr1;
#endif
static PTR48 tPtr2,tPtr3;
static volatile UINT32 mapRegSpace = 0;
static UINT32 mHandle;
static int vmeInt;



/******************************************************************************
*
* initTsi148
*
* The function performs TSI148 Initialisations
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 initTsi148( void )
{
	PCI_PFA	pfa;
	UINT32	regVal;
	UINT32	result = E__TEST_NO_DEVICE;
	
	if (iPciFindDeviceById (1, VID_TUNDRA, DID_TSI148, &pfa) == E__OK)
	{
#ifdef DEBUG
//		sprintf(buffer,"Found TSI148 bridge at %x:%x.%x\n", PCI_PFA_BUS(pfa), PCI_PFA_DEV(pfa), PCI_PFA_FUNC(pfa));
//		vConsoleWrite(buffer);
#endif

		/*Enable the bus mastering and memory/io space access*/
		regVal=PCI_READ_WORD(pfa,PCI_COMMAND);
		regVal |= 0x06;
		PCI_WRITE_WORD(pfa,PCI_COMMAND,regVal);

		/*Get the memory mapped CRG*/
		regVal = PCI_READ_DWORD(pfa,PCI_BAR0);
		regVal &= ~0xf;

#ifdef DEBUG
//		sprintf(buffer,"PCI CRG=%x\n",regVal);
//		vConsoleWrite(buffer);
#endif

#ifndef INCLUDE_LWIP_SMC
		mHandle = dGetPhysPtr(regVal,0x1000,&tPtr1,(void*)&mapRegSpace);

		if(mHandle == E__FAIL)
		{
			return E__TEST_NO_MEM;
		}
#endif

		if( (osReadDWord(mapRegSpace)) != TSI148_DEVID_VENID )
		{
#ifdef DEBUG
			sprintf(buffer,"Unable to read from mapped CRG(%x):%x\n",
				mapRegSpace,osReadDWord(mapRegSpace));
			vConsoleWrite(buffer);
#endif
			vFreePtr(mHandle);
			return E__TSI148_ACCESS;
		}
		else
		{
			/*Sanity initialisations*/
			osWriteDWord((mapRegSpace+CSRBCR),0x40000000);
			osWriteDWord((mapRegSpace+INTEN), 0x00000000);
			osWriteDWord((mapRegSpace+INTC),  0xFFFFFFFF);
			osWriteDWord((mapRegSpace+INTEO), 0x00000000);
			osWriteDWord((mapRegSpace+VICR),  0x00000000);


			/* Clear the VMEBus exception Status bit */
			regVal = TSI148_VEAT_VESCL;
			regVal = BYTE_SWAP(regVal);
			osWriteDWord((mapRegSpace+VEAT),regVal);

			return E__OK;
		}
	}

	return result;
}

/******************************************************************************
*
* dAcquireTsi148VMEBus
*
* This function reserves the VME bus for use by this boards
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 dAcquireTsi148VMEBus (UINT8 slot)
{
	UINT32 regVal = 0;
	UINT32 rt = E__OK;
	UINT32 maxRetry = 0x1000;
	UINT32 temp;

	/*
	 * Read VMCTRL and set the VS bit and wait for VSA bit to be set
	 */
	temp = osReadDWord((mapRegSpace + VMCTRL));
	regVal = BYTE_SWAP(temp);
	regVal |= VMCTRL_DWB + VMCTRL_TOFF + VMCTRL_TON;
	temp = BYTE_SWAP(regVal);
	osWriteDWord((mapRegSpace + VMCTRL), temp);

	temp = osReadDWord((mapRegSpace + VMCTRL));
	regVal = BYTE_SWAP(temp);

	 while (((regVal & VMCTRL_DHB)!= VMCTRL_DHB) && (--maxRetry > 0))
	{
		vDelay(1 * (slot + 1)); // slot is a zero-based value
		regVal |= VMCTRL_DWB + VMCTRL_TOFF + VMCTRL_TON;
		temp = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace + VMCTRL), temp);
		temp = osReadDWord((mapRegSpace + VMCTRL));
		regVal = BYTE_SWAP(temp);
	}

	if (maxRetry == 0)
	{
#ifdef DEBUG
		sysDebugWriteString("Failed to acquire VME bus\n");
		sprintf(buffer, "VMCTRL:%08X [raw:%08X]\n", regVal, temp);
		sysDebugWriteString(buffer);
#endif
		regVal &= ~(VMCTRL_DWB + VMCTRL_TOFF + VMCTRL_TON);
		temp = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace + VMCTRL), temp);
		rt = E__VME_BUS_ACQUIRE;
	}
	else
	{
		rt = E__OK;
	}

//#ifdef DEBUG
//	sprintf(buffer, "Acquire VMCTRL:%08X [raw:%08X]\n", regVal, temp);
//	sysDebugWriteString(buffer);
//#endif

	return rt;
}

/******************************************************************************
*
* dReleaseTsi148VMEBus
*
* This function releases the VME bus for use by other boards
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 dReleaseTsi148VMEBus (void)
{
	UINT32 regVal = 0;
	UINT32 rt = E__OK;
	UINT32 maxRetry = 0x80;
	UINT32 temp;

	/*
	 * Read VMCTRL and clear the VS bit to release the bus
	 */
	temp = osReadDWord((mapRegSpace + VMCTRL));
	regVal = BYTE_SWAP(temp);
	regVal &= ~(VMCTRL_DWB + VMCTRL_TOFF + VMCTRL_TON);
	temp = BYTE_SWAP(regVal);
	osWriteDWord((mapRegSpace + VMCTRL), temp);

	temp = osReadDWord((mapRegSpace + VMCTRL));
	regVal = BYTE_SWAP(temp);

	while (((regVal & VMCTRL_DWB)!= 0x0) && (maxRetry-- > 0))
	{
		vDelay(1);
		regVal &= ~(VMCTRL_DWB + VMCTRL_TOFF + VMCTRL_TON);
		temp = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace + VMCTRL), temp);
		temp = osReadDWord((mapRegSpace + VMCTRL));
		regVal = BYTE_SWAP(temp);
	}

	if (maxRetry == 0)
	{
		rt = E__VME_BUS_RELEASE;
	}
	else
	{
		rt = E__OK;
	}

//#ifdef DEBUG
//	sprintf(buffer, "Release VMCTRL:%08X [raw:%08X]\n", regVal, temp);
//	sysDebugWriteString(buffer);
//#endif

	return rt;
}

/******************************************************************************
*
* dAcquireUniverseVMEBus
*
* This function reserves the VME bus for use by this boards
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 dAcquireUniverseVMEBus (UINT8 slot)
{
	UINT32 regVal=0,rt=0,maxRetry = 0x1000;

	/*
	 * Read MAST_CTL and set the VOWN BIT and wait VOWN_ACK
	 */
	regVal = osReadDWord((mapRegSpace + MAST_CTL));
	regVal |= 0x80000;
	osWriteDWord((mapRegSpace+MAST_CTL), regVal);

	regVal = osReadDWord((mapRegSpace + MAST_CTL));

	do
	{
		vDelay(1*slot);
		regVal |= 0x80000;
		osWriteDWord((mapRegSpace+MAST_CTL), regVal);
		regVal = osReadDWord((mapRegSpace + MAST_CTL));
	}while ( ((regVal & 0x40000)!= 0x40000) && (maxRetry-- > 0));

	if (maxRetry == 0)
	{
		rt = E__TEST_VMEBUSS_ERR;
	}
	else
	{
		rt = E__OK;
	}

	return rt;
}

/******************************************************************************
*
* dReleaseUniverseVMEBus
*
* This function releases the VME bus for use by other boards
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 dReleaseUniverseVMEBus (void)
{
	UINT32 regVal=0,rt=0,maxRetry = 0x80;

	/*
	 * Read MAST_CTL and set the VOWN BIT and wait VOWN_ACK
	 */
	regVal = osReadDWord((mapRegSpace + MAST_CTL));
	regVal &= ~0x80000;
	osWriteDWord((mapRegSpace+MAST_CTL), regVal);

	regVal = osReadDWord((mapRegSpace + MAST_CTL));

	do
	{
		vDelay(1);
		regVal &= ~0x80000;
		osWriteDWord((mapRegSpace+MAST_CTL), regVal);
		regVal = osReadDWord((mapRegSpace + MAST_CTL));
	}while ( ((regVal & 0x40000)!= 0x0) && (maxRetry-- > 0));

	if (maxRetry == 0)
	{
		rt = E__TEST_VMEBUSS_ERR;
	}
	else
	{
		rt = E__OK;
	}

	return rt;
}


/******************************************************************************
*
* initUniverse
*
* The function performs UniverseII Initialisation
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 initUniverse( void )
{
	PCI_PFA	pfa;
	UINT32	regVal,dTemp;
	UINT32	result = E__TEST_NO_DEVICE;

	if (iPciFindDeviceById(1, VID_TUNDRA, DID_UNIVERSE, &pfa) == E__OK)
	{
#ifdef DEBUG
		sprintf(buffer,"Found UniverseII bridge at %x:%x.%x\n", PCI_PFA_BUS(pfa), PCI_PFA_DEV(pfa), PCI_PFA_FUNC(pfa));
		vConsoleWrite(buffer);
#endif

		/*Enable the bus mastering and memory/io space access*/
		regVal=PCI_READ_WORD(pfa,PCI_COMMAND);
		regVal |= 0x06;
		PCI_WRITE_WORD(pfa,PCI_COMMAND,regVal);

		/*Get the memory mapped CRG*/
		regVal = PCI_READ_DWORD(pfa,PCI_BAR0);
		regVal &= ~0xf;

#ifdef DEBUG
		sprintf(buffer,"PCI CRG=%x\n",regVal);
		vConsoleWrite(buffer);
#endif

#ifndef INCLUDE_LWIP_SMC
		mHandle = dGetPhysPtr(regVal,0x1000,&tPtr1,(void*)&mapRegSpace);

		if(mHandle == E__FAIL)
		{
			return E__TEST_NO_MEM;
		}
#endif

		if( (osReadDWord(mapRegSpace)) != UNIV_DEVID_VENID )
		{
#ifdef DEBUG
			sprintf(buffer,"Unable to read from mapped CRG(%x):%x\n",
				mapRegSpace,osReadDWord(mapRegSpace));
			vConsoleWrite(buffer);
#endif
			vFreePtr(mHandle);
			return E__TSI148_ACCESS;
		}
		else
		{
			/* Release LOCAL sysfail */
			vWriteDWord((mapRegSpace+VCSR_CLR), LOC_SYSFAIL);

		    /* disable interrupts */
			vWriteDWord(mapRegSpace+LINT_EN,0x00000000);

			vWriteDWord((mapRegSpace+LINT_MAP0), 0x00000000);/* map to LINT0 */
			vWriteDWord(mapRegSpace+LINT_MAP2,0x00000000);
			vWriteDWord(mapRegSpace+LINT_MAP1,0x00000000);

		    /* clear any pending interrupts */
			//vWriteDWord( mapRegSpace+LINT_STAT,0x00FFFFFF);


			dTemp = vReadDWord((mapRegSpace + MISC_CTL));
			dTemp &= 0x0fefffff;
			dTemp |= 0x60000000; //was 0x31000000
		    vWriteDWord((mapRegSpace+MISC_CTL), dTemp);

		    dTemp = 0x80E00000;
		    vWriteDWord((mapRegSpace+MAST_CTL), dTemp);

			/*Read the VEAT register*/
			regVal = osReadDWord((mapRegSpace + PCI_CSR));

			// clear VME error status
			if ((regVal & S_TA) == S_TA)
			{
				regVal |= S_TA;
				osWriteDWord((mapRegSpace+PCI_CSR), regVal);
			}

			   /* Clear any VME mapping */
			osWriteDWord (mapRegSpace+VINT_MAP0,0x0000000 );
			osWriteDWord(  mapRegSpace+VINT_MAP1,0x00000000 );
			osWriteDWord( mapRegSpace+VINT_MAP2,0x0000000 );

			osWriteDWord(mapRegSpace+VINT_EN,0x00000000);

			 /* disable interrupts */
			vWriteDWord(mapRegSpace+LINT_EN,0x00000000);


			 /* clear any pending interrupts */
			regVal = osReadDWord( mapRegSpace + VINT_STAT );
			osWriteDWord(mapRegSpace + VINT_STAT, regVal);

		   regVal = osReadDWord( mapRegSpace + VINT_STAT );
		   osWriteDWord(mapRegSpace + VINT_STAT, regVal);


			regVal = osReadDWord(mapRegSpace+LSI0_CTL);
			regVal |= LSI_EN;
			osWriteDWord(mapRegSpace+LSI0_CTL,regVal);

			regVal = osReadDWord(mapRegSpace+LSI1_CTL);
			regVal |= LSI_EN;
			osWriteDWord(mapRegSpace+LSI1_CTL,regVal);

			regVal = osReadDWord(mapRegSpace+LSI2_CTL);
			regVal |= LSI_EN;
			osWriteDWord(mapRegSpace+LSI2_CTL,regVal);

			regVal = osReadDWord(mapRegSpace+LSI3_CTL);
			regVal |= LSI_EN;
			osWriteDWord(mapRegSpace+LSI3_CTL,regVal);

			return E__OK;
		}
	}

	return result;
}

/******************************************************************************
*
* deinitTsi148
*
* The function performs TSI148 de-initialisations
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static void deinitTsi148( void )
{
	UINT32 regVal;

	osWriteDWord((mapRegSpace+CSRBCR),0x40000000);
	osWriteDWord((mapRegSpace+INTEN),0x00000000);
	osWriteDWord((mapRegSpace+INTC),0xFFFFFFFF);
	osWriteDWord((mapRegSpace+INTEO),0x00000000);
	osWriteDWord((mapRegSpace+VICR),0x00000000);

	/* Clear the VMEBus exception Status bit */
	regVal = TSI148_VEAT_VESCL;
	regVal = BYTE_SWAP(regVal);
	osWriteDWord((mapRegSpace+VEAT),regVal);

	regVal = osReadDWord(mapRegSpace+ITAT7);
	regVal &= ~TSI148_IMAGE_EN;
	osWriteDWord(mapRegSpace+ITAT7,regVal);

#ifndef INCLUDE_LWIP_SMC
	vFreePtr(mHandle);
#endif
}

/******************************************************************************
*
* deinitUniverse
*
* The function performs TSI148 de-initialisations
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static void deinitUniverse( void )
{
	UINT32 regVal;

	/*Read the CSR register*/
	regVal = osReadDWord((mapRegSpace + PCI_CSR));

	// clear VME error status
	if ((regVal & S_TA) == S_TA)
	{
		regVal |= S_TA;
		osWriteDWord((mapRegSpace+PCI_CSR), regVal);
	}


	/* Release LOCAL sysfail */
	vWriteDWord((mapRegSpace+VCSR_CLR), LOC_SYSFAIL);
	vWriteDWord(mapRegSpace+LINT_MAP0,0x00000000);
	vWriteDWord(mapRegSpace+LINT_MAP1,0x00000000);
	vWriteDWord(mapRegSpace+LINT_MAP2,0x00000000);

    /* disable interrupts */
	vWriteDWord(mapRegSpace+LINT_EN,0x00000000);

	osWriteDWord(mapRegSpace+VINT_EN,0x00000000);

	   /* Clear any VME mapping */
	osWriteDWord (mapRegSpace+VINT_MAP0,0x0000000 );
	osWriteDWord(  mapRegSpace+VINT_MAP1,0x00000000 );
	osWriteDWord( mapRegSpace+VINT_MAP2,0x0000000 );

	regVal = osReadDWord( mapRegSpace + VINT_STAT );
	osWriteDWord(mapRegSpace + VINT_STAT, regVal);

	 /* clear any pending interrupts */
	regVal = osReadDWord( mapRegSpace + LINT_STAT );
	osWriteDWord(mapRegSpace + LINT_STAT, regVal);


	//vWriteDWord( mapRegSpace+LINT_STAT,0x00FFFFFF);

	regVal = osReadDWord(mapRegSpace+LSI0_CTL);
	regVal &= ~LSI_EN;
	osWriteDWord(mapRegSpace+LSI0_CTL,regVal);

	regVal = osReadDWord(mapRegSpace+LSI1_CTL);
	regVal &= ~LSI_EN;
	osWriteDWord(mapRegSpace+LSI1_CTL,regVal);

	regVal = osReadDWord(mapRegSpace+LSI2_CTL);
	regVal &= ~LSI_EN;
	osWriteDWord(mapRegSpace+LSI2_CTL,regVal);

	regVal = osReadDWord(mapRegSpace+LSI3_CTL);
	regVal &= ~LSI_EN;
	osWriteDWord(mapRegSpace+LSI3_CTL,regVal);

}

/******************************************************************************
*
* readDwordTsi148
*
* reads data from the VME bus and checks the VME bus error status 
*
* RETURNS: 0 if no bus error else error code
*
******************************************************************************/
UINT32 readDwordTsi148(UINT32 *addr, UINT32 *val, UINT8 slot)
{
	UINT32 regVal;
	UINT32 rt;
#ifdef DEBUG
	UINT32 temp;
	char buffer[80];
#endif

	if (mapRegSpace == 0)
	{
		return E__TSI148_ACCESS;
	}

	rt = E__OK;

	// acquire the bus for a block of 8 reads
	if (((((UINT32)addr) > 2) & 0xFFFFFFFC) == 0)
	{
		rt = dAcquireTsi148VMEBus(slot);
	}

	if (rt == E__OK)
	{
		/*Read the VEAT register*/
		regVal = osReadDWord((mapRegSpace + VEAT));

		// clear VME error status
		regVal |= TSI148_VEAT_VESCL_VME;
		osWriteDWord((mapRegSpace + VEAT), regVal);

		// read data from VME bus
		*val = *addr;

		// check bus status
		regVal = osReadDWord((mapRegSpace + VEAT));
		if ((regVal & TSI148_VEAT_VES_VME) != 0)
		{
#ifdef DEBUG
			sprintf(buffer, "Address:%08X \n", (UINT32)addr);
			sysDebugWriteString(buffer);
			temp = BYTE_SWAP(regVal);
			sprintf(buffer, "VEAT error:%08X [raw:%08X]\n", temp, regVal);
			sysDebugWriteString(buffer);
#endif

			regVal |= TSI148_VEAT_VESCL_VME;
			osWriteDWord((mapRegSpace + VEAT), regVal);

#ifdef DEBUG
			regVal = osReadDWord((mapRegSpace + VEAL));
			temp = BYTE_SWAP(regVal);
			sprintf(buffer, "VEAL :%08X [raw:%08X]\n", temp, regVal);
			sysDebugWriteString(buffer);
#endif
			rt = E__TEST_VMEBUSS_ERR;
		}
		else
		{
			rt = E__OK;
		}

		// release the bus after a block of 8 reads
		if (((((UINT32)addr) > 2) & 0x00000003) == 0x00000003)
		{
			dReleaseTsi148VMEBus();
		}
	}

	return rt;
}


/******************************************************************************
*
* readDwordUniverse
*
* reads data from the VME bus and checks the VME bus error status
*
* RETURNS: 0 if no bus error else error code
*
******************************************************************************/
UINT32 readDwordUniverse(UINT32 *addr, UINT32 *val)
{
	UINT32 regVal;
//	UINT32 maxRetry = 0x10;
#ifdef DEBUG
	char buffer[80];
#endif

	if( mapRegSpace == 0 )
	{
		return E__TSI148_ACCESS;
	}

	/*Read the CSR register*/
	regVal = osReadDWord((mapRegSpace + PCI_CSR));

	// clear VME error status
	if ((regVal & S_TA) == S_TA)
	{
		regVal |= S_TA;
		osWriteDWord((mapRegSpace+PCI_CSR), regVal);
	}

	// read data from VME bus
	//*val = *addr;
	*val = osReadDWord(addr);


	// check bus status
	regVal = osReadDWord((mapRegSpace + PCI_CSR));
	if ((regVal & S_TA) == S_TA)
	{
#ifdef DEBUG
		sprintf(buffer, "Sig Target Abort error:%08X \n", regVal);
		sysDebugWriteString(buffer);
		sprintf(buffer, "addr:%08X,val:%08X \n", (UINT32)addr, *val);
		sysDebugWriteString(buffer);
#endif
		regVal |= S_TA;
		osWriteDWord((mapRegSpace+PCI_CSR), regVal);
#ifdef DEBUG
		regVal = osReadDWord((mapRegSpace + PCI_CSR));
		sprintf(buffer, "Sig Target Abort Error:%08X\n", regVal);
		sysDebugWriteString(buffer);
#endif
		return E__TEST_VMEBUSS_ERR;
	}
	else
	{
		return E__OK;
	}
}

/******************************************************************************
*
* writeDwordTsi148
*
* Writes data to the VME bus and checks the VME bus error status 
*
* RETURNS: 0 if no bus error else error code
*
******************************************************************************/
UINT32 writeDwordTsi148(UINT32 *addr, UINT32 val, UINT8 slot)
{
	UINT32 regVal;
	UINT32 rt;
#ifdef DEBUG
	UINT32 temp;
	char buffer[80];
#endif

	if( mapRegSpace == 0 )
	{
		return E__TSI148_ACCESS;
	}

	rt = E__OK;

	// acquire the bus for a block of 8 writes
	if (((((UINT32)addr) > 2) & 0xFFFFFFF8) == 0)
	{
		rt = dAcquireTsi148VMEBus(slot);
	}

	if (rt == E__OK)
	{
		/*Read the VEAT register*/
		regVal = osReadDWord((mapRegSpace + VEAT));

		// clear VME error status
		regVal |= TSI148_VEAT_VESCL_VME;
		osWriteDWord((mapRegSpace + VEAT), regVal);

		// write data to VME bus
		*addr = val;

		// check bus status
		regVal = osReadDWord((mapRegSpace + VEAT));
		if ((regVal & TSI148_VEAT_VES_VME) != 0)
		{
#ifdef DEBUG
			sprintf(buffer, "Address:%08X \n", (UINT32)addr);
			sysDebugWriteString(buffer);
			temp = BYTE_SWAP(regVal);
			sprintf(buffer, "VEAT error:%08X [raw:%08X]\n", temp, regVal);
			sysDebugWriteString(buffer);
#endif

			regVal |= TSI148_VEAT_VESCL_VME;
			osWriteDWord((mapRegSpace + VEAT), regVal);

#ifdef DEBUG
			regVal = osReadDWord((mapRegSpace + VEAL));
			temp = BYTE_SWAP(regVal);
			sprintf(buffer, "VEAL :%08X [raw:%08X]\n", temp, regVal);
			sysDebugWriteString(buffer);
#endif
			rt = E__TEST_VMEBUSS_ERR;
		}
		else
		{
			rt = E__OK;
		}

		// release the bus after a block of 8 writes
		if (((((UINT32)addr) > 2) & 0x00000007) == 0x00000007)
		{
			dReleaseTsi148VMEBus();
		}
	}
	return rt;
}


/******************************************************************************
*
* writeDwordUniverse
*
* Writes data to the VME bus and checks the VME bus error status
*
* RETURNS: 0 if no bus error else error code
*
******************************************************************************/
UINT32 writeDwordUniverse(UINT32 *addr, UINT32 val)
{
	UINT32 regVal;
//	UINT32 maxRetry=0x10;
#ifdef DEBUG
	char buffer[80];
#endif

	if( mapRegSpace == 0 )
	{
		return E__TSI148_ACCESS;
	}

	/*Read the CSR register*/
	regVal = osReadDWord((mapRegSpace + PCI_CSR));

	// clear VME error status
	if ((regVal & S_TA) == S_TA)
	{
		regVal |= S_TA;
		osWriteDWord((mapRegSpace+PCI_CSR), regVal);
	}

	// write data to VME bus
	osWriteDWord(addr,val);

	//*addr = val;

	// check bus status
	regVal = osReadDWord((mapRegSpace + PCI_CSR));
	if ((regVal & S_TA) == S_TA)
	{
#ifdef DEBUG
		sprintf(buffer, "SigAbort error:%08X \n", regVal);
		sysDebugWriteString(buffer);

		sprintf(buffer, "addr:%08X,val:%08X \n", (UINT32)addr, val);
		sysDebugWriteString(buffer);
#endif
		regVal |= S_TA;
		osWriteDWord((mapRegSpace+PCI_CSR), regVal);
#ifdef DEBUG
		regVal = osReadDWord((mapRegSpace + PCI_CSR));
		sprintf(buffer, "SigAbort error :%08X\n", regVal);
		sysDebugWriteString(buffer);
#endif
		return E__TEST_VMEBUSS_ERR;
	}
	else
	{
		return E__OK;
	}
}

/******************************************************************************
*
* dmaXferTsi148
*
* Perform DMA transfer 
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 dmaXferTsi148(UINT8 direction, UINT32 * srcAddress, UINT32 * dstAddress, UINT8 slot)
{
	UINT32	regVal;
	UINT32	timeout;
	UINT32	rt;
#ifdef DEBUG
	UINT32	temp;
#endif

#ifdef DEBUG
	sprintf(buffer, "%s src:%08X dst:%08X\n", direction == WRITE_VME ? "WRITE_VME" : "READ_VME", (UINT32)srcAddress, (UINT32)dstAddress);
	sysDebugWriteString(buffer);
#endif

	rt = dAcquireTsi148VMEBus(slot);
	if (rt == E__OK)
	{
		// clear VME error status
		regVal = osReadDWord((mapRegSpace + VEAT));
		regVal |= TSI148_VEAT_VESCL_VME;
		osWriteDWord((mapRegSpace+VEAT), regVal);

		// clear DMAcontrol register
		osWriteDWord((mapRegSpace + DCTL0), 0);

		// source buffer address
		regVal= BYTE_SWAP((UINT32)srcAddress);
		osWriteDWord((mapRegSpace + DSAU0), 0);
		osWriteDWord((mapRegSpace + DSAL0), regVal);

		// destination buffer address
		regVal = BYTE_SWAP((UINT32)dstAddress);
		osWriteDWord((mapRegSpace + DDAU0), 0);
		osWriteDWord((mapRegSpace + DDAL0), regVal);

		// source attributes
		regVal = 0;			// assume PCI
		if (direction == READ_VME)
		{
			regVal = DMA_SRC_VME + DMA_SRC_BLT + DMA_SRC_D32 + DMA_SRC_A64;
		}
		regVal = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace + DSAT0), regVal);

		// destination attributes
		regVal = 0;			//  assume PCI
		if (direction == WRITE_VME)
		{
			regVal = DMA_DST_VME +  DMA_DST_BLT + DMA_DST_D32 + DMA_DST_A64;
		}
		regVal = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace + DDAT0), regVal);

		// number of DWORDs to transfer
		regVal= BYTE_SWAP(0x1000);
		osWriteDWord((mapRegSpace + DCNT0), regVal);

		// Initiate DMA
		regVal = BYTE_SWAP(DMA_START);
		osWriteDWord((mapRegSpace + DCTL0) , regVal);

		// Wait for the DMA to complete
		timeout = 0x1000;
		do
		{
			vDelay(1);
			regVal = osReadDWord(mapRegSpace + DCSTA0);
			regVal = BYTE_SWAP(regVal);
			timeout--;
		} while ((timeout!=0) && (regVal & DMA_BUSY));

		if (timeout == 0 || !(regVal & DMA_DONE))
		{
			rt = E__TEST_VMEDMA_ERR;
		}

#ifdef DEBUG
//		temp = BYTE_SWAP(regVal);
//		sprintf(buffer, "DCSTA0:%08X [raw:%08X]\n", regVal, temp);
//		sysDebugWriteString(buffer);
//		regVal >>= 16;
//		sprintf(buffer, "ERR:%d ABT:%d PAU:%d DON:%d BSY:%d ERRS:%d ERTn:%d\n", 
//					(regVal & 0x1000) >> 12, (regVal & 0x800) >> 11, (regVal & 0x400) >> 10, (regVal & 0x200) >> 9,
//					(regVal & 0x100) >> 8, (regVal & 0x10) >> 4, regVal & 0x3);
//		vConsoleWrite(buffer);
//
//		/*Read the VEAT register*/
//		regVal = osReadDWord((mapRegSpace + VEAT));
//		temp = BYTE_SWAP(regVal);
//		sprintf(buffer, "VEAT:%08X [raw:%08X]\n", temp, regVal);
//		sysDebugWriteString(buffer);
//
//		/*Read the VEAU register*/
//		regVal = osReadDWord((mapRegSpace + VEAU));
//		temp = BYTE_SWAP(regVal);
//		sprintf(buffer, "VEAU:%08X [raw:%08X]\n", temp, regVal);
//		sysDebugWriteString(buffer);
//
//		/*Read the VEAL register*/
//		regVal = osReadDWord((mapRegSpace + VEAL));
//		temp = BYTE_SWAP(regVal);
//		sprintf(buffer, "VEAL:%08X [raw:%08X]\n", temp, regVal);
//		sysDebugWriteString(buffer);
//
//		// read the VEAT register
//		regVal = osReadDWord((mapRegSpace + VEAT));
//		// clear VME error status
//		regVal |= TSI148_VEAT_VESCL_VME;
//		osWriteDWord((mapRegSpace+VEAT), regVal);
//
//		regVal = osReadDWord((mapRegSpace + VEAT));
//		temp = BYTE_SWAP(regVal);
//		sprintf(buffer, "VEAT:%08X [raw:%08X]\n", temp, regVal);
//		sysDebugWriteString(buffer);
#endif
	}

	dReleaseTsi148VMEBus();

	return rt;
}

/******************************************************************************
*
* dmaXferUniverse
*
* Perform DMA transfer
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 dmaXferUniverse(UINT8 direction, UINT32 * srcAddress, UINT32 * dstAddress)
{
	UINT32	regVal;
	UINT32	timeout;
	UINT32	rt;
	char buffer[80];
#ifdef DEBUG
//	UINT32	temp;
#endif

#ifdef DEBUG
	sprintf(buffer, "%s src:%08X dst:%08X\n", direction == WRITE_VME ? "WRITE_VME" : "READ_VME", (UINT32)srcAddress, (UINT32)dstAddress);
	sysDebugWriteString(buffer);
#endif

	rt = E__OK;


	/*Read the CSR register*/
	regVal = osReadDWord((mapRegSpace + PCI_CSR));

	// clear VME error status
	if ((regVal & S_TA) == S_TA)
	{
		regVal |= S_TA;
		osWriteDWord((mapRegSpace+PCI_CSR), regVal);
	}



    // Check that DMA is idle
	if (osReadDWord(mapRegSpace + DGCS) & 0x00008000) {
		return E__VME_DMA_BUSY;
	}


	// clear DMAcontrol register
	osWriteDWord((mapRegSpace + DCTL), 0);

	// number of DWORDs to transfer
	regVal= (0x1000);
	osWriteDWord((mapRegSpace + DTBC), regVal);


	if (direction == WRITE_VME)
	{
		// source buffer address
		regVal= ((UINT32)srcAddress);
		osWriteDWord((mapRegSpace + DLA), 0);
		osWriteDWord((mapRegSpace + DLA), regVal);

		// destination buffer address
		regVal = ((UINT32)dstAddress);
		osWriteDWord((mapRegSpace + DVA), 0);
		osWriteDWord((mapRegSpace + DVA), regVal);
	}

	if (direction == READ_VME)
	{
		// source buffer address
		regVal = ((UINT32)dstAddress);
		osWriteDWord((mapRegSpace + DLA), 0);
		osWriteDWord((mapRegSpace + DLA), regVal);

		// destination buffer address
		regVal= ((UINT32)srcAddress);
		osWriteDWord((mapRegSpace + DVA), 0);
		osWriteDWord((mapRegSpace + DVA), regVal);
	}

	// source attributes
	regVal = 0;			// assue PCI
	regVal = osReadDWord((mapRegSpace + DCTL));
	if (direction == READ_VME)
	{
		regVal &= ~(0x80000000);
		regVal |= (0x820000);
		osWriteDWord((mapRegSpace + DCTL), regVal);
	}

	// destination attributes
	regVal = 0;			//  assume PCI
	if (direction == WRITE_VME)
	{
		regVal |= (0x80000000);
		regVal |= (0x820000);
		osWriteDWord((mapRegSpace + DCTL), regVal);
	}

	//Program DGCS register
	//Stop the DMA first
	osWriteDWord((mapRegSpace + DGCS), 0x40000000);
	vDelay(50);
	osWriteDWord(mapRegSpace + DGCS,0x00006F00);
	vDelay(10);

	// Initiate DMA
	//VON-VOFF
	regVal = 0;
	regVal = osReadDWord((mapRegSpace + DGCS));
	//regVal |= 0x540000;
	regVal &= ~(0xFF0000);
	regVal &= ~(0x000000ff);
	regVal |= 0x80006f00;

	osWriteDWord((mapRegSpace + DGCS), regVal);
	// Wait for the DMA to complete
	timeout = 0x2000;
	do
	{
		vDelay(1);
		regVal = osReadDWord(mapRegSpace + DGCS);
		timeout--;
	} while ((timeout!=0) && (regVal & 0x800) != 0x800);

	if (timeout == 0 || !(regVal & 0x800))
	{
		rt = E__TEST_VMEDMA_ERR;
	}

#ifdef DEBUG
	sprintf(buffer, "DGCS:%08X \n", regVal);
	vConsoleWrite(buffer);
	regVal >>= 8;
	regVal &= 0x07;
	sprintf(buffer, "P_ERR:%d V_ERR:%d L_ERR:%d\n", ((regVal & 0x1)),((regVal>>1) & 0x1),(regVal>>2 & 0x1));
	vConsoleWrite(buffer);
#endif
	/*Read the VEAT register*/
	regVal = osReadDWord((mapRegSpace + PCI_CSR));

	if ((regVal & S_TA) == S_TA)
	{
		sprintf(buffer, "SigAbort error:%08X \n",   regVal);
		vConsoleWrite(buffer);
		rt = E__TEST_VMEBUSS_ERR;
	}

	// clear VME error status
	regVal |= S_TA;
	osWriteDWord((mapRegSpace+PCI_CSR), regVal);

	//Stop the DMA
	osWriteDWord((mapRegSpace + DGCS), 0x40000000);
	vDelay(50);
	regVal = osReadDWord(mapRegSpace + DGCS);
	osWriteDWord(mapRegSpace + DGCS,regVal);
	vDelay(10);


	return rt;
}

/******************************************************************************
*
* dmaTxfer
*
* Peform DMA txfer 
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 dmaTxfer(UINT8 busErr)
{
	UINT32 pHandle,regVal,dmaAddr=0;
	UINT32 i=0,timeout;

	if (busErr)
	{
		timeout = 0x10;
	}
	else
	{
		timeout = 0x1000;
	}

	/*Write a pattern into the DMA buffer*/
	pHandle = dGetPhysPtr(TEST_DMA_ADDR,0x10000,&tPtr2,(void*)&dmaAddr);

	if (pHandle == E__FAIL)
	{
		return E__TEST_NO_MEM;
	}

	for (i = 0; i < 0x1000; i += 4 )
	{
		osWriteDWord((dmaAddr+i),TESTPAT1);
		regVal=osReadDWord(dmaAddr+i);
	}

	/*Set the DMA0 registers for a block write transaction*/
	osWriteDWord((mapRegSpace+DCTL0),0);
	osWriteDWord((mapRegSpace+DSAU0),0);

	regVal= BYTE_SWAP(TEST_DMA_ADDR);
	osWriteDWord((mapRegSpace+DSAL0),regVal);

	osWriteDWord((mapRegSpace+DDAU0),0);

	regVal = BYTE_SWAP(TEST_VME_ADDR);
	osWriteDWord((mapRegSpace+DDAL0),regVal);

	regVal = 0x00000042U;
	regVal = BYTE_SWAP(regVal);
	osWriteDWord((mapRegSpace+DSAT0),regVal);

	regVal = 0x10000042U;
	regVal = BYTE_SWAP(regVal);
	osWriteDWord((mapRegSpace+DDAT0),regVal);

	regVal= BYTE_SWAP(0x1000);
	osWriteDWord((mapRegSpace+DCNT0),regVal);

	/*Initiate DMA*/
	regVal = BYTE_SWAP(0x02800000);
	osWriteDWord((mapRegSpace+DCTL0),regVal);

	/*Wait for the DMA to complete*/
	do
	{
		vDelay(1);
		regVal = osReadDWord( mapRegSpace + DCSTA0 );
		regVal = BYTE_SWAP( regVal );
		timeout--;
	} while ((timeout!=0) && !(regVal & 0x02000000L));

	if (timeout == 0 || !(regVal & 0x02000000))
	{
		vFreePtr(pHandle);
		return E__TEST_VMEDMA_ERR;
	}

	vFreePtr(pHandle);

	return E__OK;
}


/*****************************************************************************
* performDma
* Peform DMA
* RETURNS: 0 on success else error code
******************************************************************************/
static UINT32 performDma( UINT8 uw_r )
{
	UINT32 pHandle,regVal,dmaAddr=0;
	UINT32 rt, i=0,timeout;

	timeout = 0x1000;

	/*Allocate DMA buffer*/
	pHandle = dGetPhysPtr(TEST_DMA_ADDR,0x10000,&tPtr2,(void*)&dmaAddr);

	if(pHandle == E__FAIL)
	{
		return E__TEST_NO_MEM;
	}
	if(uw_r == 1)
	{
		for(i=0; i < 0x1000; i+=4 )
		{
			osWriteDWord((dmaAddr+i),BSTESTPAT1);
			regVal=osReadDWord(dmaAddr+i);
		}
	}

	/*Set the DMA0 registers for a block transaction*/
	osWriteDWord((mapRegSpace+DCTL0),0);
	osWriteDWord((mapRegSpace+DSAU0),0);

	regVal= BYTE_SWAP(TEST_VME_ADDR);
	osWriteDWord((mapRegSpace+DSAL0),regVal);

	osWriteDWord((mapRegSpace+DDAU0),0);

	regVal = BYTE_SWAP(TEST_DMA_ADDR);
	osWriteDWord((mapRegSpace+DDAL0),regVal);

	if(uw_r == 0)
	{
		regVal = 0x10000042U;
		regVal = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace+DSAT0),regVal);
	}
	else
	{
		regVal = 0x00000042U;
		regVal = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace+DSAT0),regVal);
	}

	if(uw_r == 0)
	{
		regVal = 0x00000042U;
		regVal = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace+DDAT0),regVal);
	}
	else
	{
		regVal = 0x10000042U;
		regVal = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace+DDAT0),regVal);
	}

	regVal= BYTE_SWAP(0x1000);
	osWriteDWord((mapRegSpace+DCNT0),regVal);

	/*Initiate DMA*/
	regVal = BYTE_SWAP( 0x02800000 );
	osWriteDWord((mapRegSpace+DCTL0),regVal);

	/*Wait for the DMA to complete*/
	do {
		vDelay(1);
		regVal = osReadDWord( mapRegSpace + DCSTA0 );
		regVal = BYTE_SWAP( regVal );
		timeout--;
	}while( (timeout!=0) && !(regVal & 0x02000000L ));

	if( timeout == 0 || !(regVal & 0x02000000 ) )
	{
		vFreePtr(pHandle);
		return E__TEST_VMEDMA_ERR;
	}
	if(uw_r == 0)
	{
		for(i=0; i < 0x1000; i+=4 )
		{
			regVal=osReadDWord(dmaAddr+i);
			if (regVal == BSTESTPAT2)
			{
				rt = E__OK;
			}
			else
			{
#ifdef DEBUG
	sprintf (buffer, "Pos: %d regVal: 0x%x\n", i, regVal);
	vConsoleWrite(buffer);
#endif
			rt = E__BSWERR;
			}
		}
	}

	vFreePtr(pHandle);

	return rt;
}
/******************************************************************************
*
* tsi148SetupVmeImage
*
* Setup VME Image
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static void tsi148SetupVmeImage(void)
{
	UINT32 rPtr;
	UINT32 regVal=0;
	int offset=0;

	/*Set up VME Image 7 */
	rPtr=mapRegSpace+ITAT7;

	/*Attribute register setup*/
	regVal = 0x0000002FU;
	regVal = BYTE_SWAP(regVal);
	osWriteDWord(rPtr,regVal);

	/*Setup inbound translation offset register*/
	rPtr-=4;
	regVal = VME_IMAGE_BASE_ADDR - TEST_VME_ADDR;
	regVal = BYTE_SWAP(regVal);
	osWriteDWord(rPtr,regVal);

	rPtr-=4;
	if( VME_IMAGE_BASE_ADDR < TEST_VME_ADDR )
	{
		offset = -1;
	}
	else
	{
		offset=0;
	}

	regVal = BYTE_SWAP( offset );
	osWriteDWord(rPtr,regVal);

	/*Setup inbound translation addresses*/
	rPtr -= 4;
	regVal = BYTE_SWAP( TEST_VME_ADDR );
	osWriteDWord(rPtr,regVal);

	rPtr -= 4;
	osWriteDWord(rPtr,0);

	rPtr -= 4;
	regVal = BYTE_SWAP( TEST_VME_ADDR );
	osWriteDWord(rPtr,regVal);

	rPtr -= 4;
	osWriteDWord(rPtr,0);

	/*Now enable the VME Image*/
	rPtr=mapRegSpace+ITAT7;
	regVal  = osReadDWord(rPtr);
	regVal |= TSI148_IMAGE_EN;
	osWriteDWord(rPtr,regVal);

}

/******************************************************************************
*
* tsi148LoopbackTest
*
* Peforms VME loopback test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static int tsi148LoopbackTest( void )
{
	UINT32 i,result;
	UINT32 vHandle;
	UINT32 vmeAddr;

	if( mapRegSpace == 0 )
	{
		return E__TSI148_ACCESS;
	}

	/*Setup Vme image*/
	tsi148SetupVmeImage();

	vHandle = dGetPhysPtr(VME_IMAGE_BASE_ADDR,0x10000,&tPtr3,(void*)&vmeAddr);

	if(vHandle == E__FAIL)
	{
		return E__TEST_NO_MEM;
	}
	
	/*Perform DMA */
	if( (result=dmaTxfer(0)) != E__OK )
	{
		return result;
	}

	/*Check for the pattern written */
	for(i=0; i < 0x1000; i+=4 )
	{
		if( (osReadDWord(vmeAddr+i)) != TESTPAT1 )
		{
#ifdef DEBUG
			sprintf(buffer,"Written=%x Read=%x\n",TESTPAT1,osReadDWord(vmeAddr+i));
			vConsoleWrite(buffer);

			sprintf(buffer,"VERR:%x VEAT:%x DSTA0=%x\n",
				osReadDWord(mapRegSpace+264),osReadDWord(mapRegSpace+VEAT),
					osReadDWord( mapRegSpace + DCSTA0 ));
			vConsoleWrite(buffer);
#endif
			vFreePtr(vHandle);
			return E__TEST_VMELB_FAIL;
		}
	}
	vFreePtr(vHandle);
	return E__OK;

}

/******************************************************************************
*
* tsi148BusErrTest
*
* Peforms VME Bus error test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 tsi148BusErrTest( void )
{
	UINT32 regVal;
	
	if( mapRegSpace == 0 )
	{
		return E__TSI148_ACCESS;
	}
		
	/*Make sure the VES bit is cleared*/
	regVal = TSI148_VEAT_VESCL;
    regVal = BYTE_SWAP(regVal);
	osWriteDWord((mapRegSpace+VEAT),regVal);

	/*Perform DMA */
	dmaTxfer(1);
	
	/*Read the VEAT register*/
	regVal = osReadDWord((mapRegSpace + VEAT));
	regVal = BYTE_SWAP(regVal);

#ifdef DEBUG
	sprintf(buffer,"VEAT:%x\n",regVal);
	vConsoleWrite(buffer);
#endif

	if( regVal & 0x00080000 )
	{
		regVal = TSI148_VEAT_VESCL;
		regVal = BYTE_SWAP(regVal);
		osWriteDWord((mapRegSpace+VEAT),regVal);
		return E__OK;
	}
	else
	{
		return E__TEST_VMEBUSS_ERR;
	}

}

/******************************************************************************
*
* vmeIntHandler
*
* VME NMI Interrupt Handler
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static void vmeIntHandler (void)
{
	osWriteDWord((mapRegSpace+INTC),0xFFFFFFFF);	/* clear interrupts */
	osWriteDWord((mapRegSpace+INTEN),0x0);
	osWriteDWord((mapRegSpace+INTEO),0x0);
	vmeInt = 1;
}

/******************************************************************************
*
* tsi148NmiTest
*
* VME NMI Test
*
*
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
static UINT32 tsi148NmiTest( void )
{
	PCI_PFA	pfa;
	UINT16	wAcpiBase;
	UINT32	regVal,sIntMap;
	int		vector=INT_PIN_NUM ;
	UINT8	bPort61;
	UINT16  GPI_disable_temp;
#if 0
	UINT32  temp;
#endif
	vmeInt = 0;
	if( mapRegSpace == 0 )
	{
		return E__TSI148_ACCESS;
	}
		
	/*Make sure the VES bit is cleared*/
	regVal = TSI148_VEAT_VESCL;
    regVal = BYTE_SWAP(regVal);
	osWriteDWord((mapRegSpace+VEAT),regVal);

	/*Map VERR to INTD(NMI)*/
	sIntMap = osReadDWord((mapRegSpace+INTM2));
	regVal=sIntMap | 0x00000003U;
	osWriteDWord((mapRegSpace+INTM2),regVal);

	/*Enable VERR interrupt*/
	regVal = 0x00001000U;
	regVal = BYTE_SWAP(regVal);
	osWriteDWord((mapRegSpace+INTEN),regVal);
	osWriteDWord((mapRegSpace+INTEO),regVal);

#ifdef DEBUG
	sprintf(buffer,"Vector=%x\n",vector);
	vConsoleWrite(buffer);
#endif

	if (sysInstallUserHandler (vector, vmeIntHandler) != 0)
	{
		return E__TES_VMENMI_ERR ;
	}

	vIoWriteReg(0x70, REG_8, 0x80);	// disable NMI
	bPort61 = dIoReadReg (0x61, REG_8) & 0x07;
	vIoWriteReg (0x61, REG_8, bPort61 | 0x08);
	vIoWriteReg (0x61, REG_8, bPort61);
	vIoWriteReg (0x70, REG_8, 0);	// enable NMI (in RTC)


	pfa = PCI_MAKE_PFA (0, 31, 0);
	wAcpiBase = 0xFFFE & PCI_READ_WORD (pfa, 0x40);
	/* Disable NMI to SMI */
	GPI_disable_temp = dIoReadReg (wAcpiBase + 0x68, REG_16);
	vIoWriteReg (wAcpiBase + 0x68, REG_16, 0x0800);

#if 0
	//code to force NMI via PCH, this is just test code
	temp = dIoReadReg (0x468, REG_16) & ~0x0200;
	sprintf(buffer,"temp=%x\n",temp);
	vConsoleWrite(buffer);
	if(temp & 0x00000100)
		vIoWriteReg (0x468, REG_16, (temp | 0x0100));
	vIoWriteReg (0x468, REG_16, (temp | 0x0100));
#endif
	/*Perform DMA */
	dmaTxfer(1);

	vDelay(10);
	
	vIoWriteReg (wAcpiBase + 0x68, REG_16, GPI_disable_temp);
	vIoWriteReg(0x70, REG_8, 0x80);	/* disable NMI */

#ifdef DEBUG
	regVal = (UINT8)dIoReadReg(0x212, REG_8);
	sprintf(buffer,"SCR 212:%x,VERR=%x\n",regVal,osReadDWord((mapRegSpace+VEAT)));
	vConsoleWrite(buffer);
#endif

	osWriteDWord((mapRegSpace+INTC),0xFFFFFFFF);
	osWriteDWord((mapRegSpace+INTM2),sIntMap);
	
	bPort61 = dIoReadReg (0x61, REG_8) & 0x07; /* Reset Interrupt Source */
	vIoWriteReg (0x61, REG_8, bPort61 | 0x08);
	vIoWriteReg (0x61, REG_8, bPort61);

	sysInstallUserHandler (vector, 0);   

	if( vmeInt == 1 )
	{
		return E__OK;
	}
	else
	{
		return E__TES_VMENMI_ERR;
	}

}

/******************************************************************************
*
* TSI148Access
*
* VME Bridge Access Test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (TSI148Access, "TSI148 Access Test")
{
	UINT32			rt = E__OK;

#ifdef INCLUDE_LWIP_SMC
	VME_DRV_INFO *vmeDrvInfo;

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;
#endif

	/*Detect TSI148*/
	rt = initTsi148();

	/*Deinit TSI148 as we do want to do anything more out here*/
	deinitTsi148();

	return rt;
}

/******************************************************************************
*
* TSI148Int
*
* VME NMI Test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (TSI148Int, "TSI148 NMI Test")
{
	UINT32  rt = E__OK;
	
#ifdef INCLUDE_LWIP_SMC
	VME_DRV_INFO *vmeDrvInfo;

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;
#endif

	rt = initTsi148();

	if( rt == E__OK )
	{
		rt = tsi148NmiTest();
	}

	deinitTsi148();

	return rt;
}

/******************************************************************************
*
* TSI148VME
*
* VME Loopback tests
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (TSI148VME, "TSI148 Loopback Test")
{
	UINT32  rt = E__OK;
	
#ifdef INCLUDE_LWIP_SMC
	VME_DRV_INFO *vmeDrvInfo;

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;
#endif

	rt = initTsi148();

	if( rt == E__OK )
	{
		rt = tsi148LoopbackTest();
	}

	deinitTsi148();
	
	return rt;
}

/******************************************************************************
*
* TSI148VMEerror
*
* VME Bus Error Test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (TSI148VMEerror, "TSI148 VME Bus Error Test")
{
	UINT32  rt = E__OK;
	
#ifdef INCLUDE_LWIP_SMC
	VME_DRV_INFO *vmeDrvInfo;

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;
#endif

	rt = initTsi148();

	if( rt == E__OK )
	{
		rt = tsi148BusErrTest();
	}

	deinitTsi148();
	
	return rt;
}


/******************************************************************************
*
* TSI148ByteSwapWrite
*
* VME Byte Swap during VME write test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (TSI148ByteSwapWrite, "TSI148 VME BS on Write Test")
{
	UINT32  rt = E__OK;
	UINT8 valreg;
	UINT32 i, vHandle;
	UINT32 vmeAddr;
	// UINT32 regVal=0;
	char tbuffer[128];

	rt = initTsi148();

	if( rt == E__OK )
	{
		/* Set Slave Byte Swap */
		valreg = dIoReadReg(0x210, REG_8);
		vIoWriteReg(0x210, REG_8, (valreg & (~0x38)));
		valreg = dIoReadReg(0x210, REG_8);
		vIoWriteReg(0x210, REG_8, (valreg | (2<<3)));
		if( mapRegSpace == 0 )
		{
			return E__TSI148_ACCESS;
		}
		/* Setup VME Image */
		tsi148SetupVmeImage();

		vHandle = dGetPhysPtr(VME_IMAGE_BASE_ADDR,0x10000,&tPtr3,(void*)&vmeAddr);

		if(vHandle == E__FAIL)
		{
			return E__TEST_NO_MEM;
		}

		/* Write Test pattern */
		for(i=0; i < 0x1000; i+=4 )
		{
			osWriteDWord((vmeAddr+i),BSTESTPAT1);
			// regVal=osReadDWord(vmeAddr+i);
		}
		sprintf(tbuffer,"Execute test TSI148 BS on Write Check Test \n"
							"on companion board within 10 sec\n");
		vConsoleWrite(tbuffer);
		vDelay(10000);
		for(i=0; i < 0x1000; i+=4 )
		{
			if ( BSTESTPAT1 == osReadDWord(vmeAddr+i))
				rt = E__OK;
			else
				rt = E__VME_WERR;
		}
	}
	deinitTsi148();
	vIoWriteReg(0x210, REG_8, (valreg & (~0x38)));
	vFreePtr(vHandle);
	return rt;
}

/******************************************************************************
*
* TSI148ByteSwapWriteChk
*
* VME Byte Swap verification during DMA read cycle test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (TSI148ByteSwapWriteChk, "TSI148 VME BS on Write Check Test")
{
	UINT32  rt = E__OK;

	rt = initTsi148();

	if( rt == E__OK )
	{
		/*Perform DMA Read */
		rt = performDma(0);
	}
	return rt;
}


/******************************************************************************
*
* TSI148ByteSwapRead
*
* VME Byte Swap during VME Read test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (TSI148ByteSwapRead, "TSI148 VME BS on Read Test")
{
	UINT32  rt = E__OK;
	UINT32 i, vHandle;
	UINT32 vmeAddr;
	// UINT32 regVal=0;
	char tbuffer[128];

	rt = initTsi148();

	if( rt == E__OK )
	{
		if( mapRegSpace == 0 )
		{
			return E__TSI148_ACCESS;
		}
		/* Setup VME Image */
		tsi148SetupVmeImage();

		vHandle = dGetPhysPtr(VME_IMAGE_BASE_ADDR,0x10000,&tPtr3,(void*)&vmeAddr);

		if(vHandle == E__FAIL)
		{
			return E__TEST_NO_MEM;
		}

		/* Write Test pattern */
		for(i=0; i < 0x1000; i+=4 )
		{
			osWriteDWord((vmeAddr+i),BSTESTPAT1);
			// regVal=osReadDWord(vmeAddr+i);
		}
		sprintf(tbuffer,"Execute test TSI148 BS on Read Check Test \n"
							"on companion board within 10 sec\n");
		vConsoleWrite(tbuffer);
		vDelay(10000);
		for(i=0; i < 0x1000; i+=4 )
		{
			if ( BSTESTPAT1 == osReadDWord(vmeAddr+i))
				rt = E__OK;
			else
				rt = E__VME_WERR;
		}
	}
	deinitTsi148();

	vFreePtr(vHandle);
	return rt;
}

/******************************************************************************
*
* TSI148ByteSwapReadChk
*
* VME Byte Swap verification during DMA read cycle test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (TSI148ByteSwapReadChk, "TSI148 VME BS on Read Check Test")
{
	UINT32  rt = E__OK;
	UINT8 valreg;

	rt = initTsi148();

	if( rt == E__OK )
	{
		/* Set Master Byte Swap */
		valreg = dIoReadReg(0x210, REG_8);
		vIoWriteReg(0x210, REG_8, (valreg & (~0x38)));
		valreg = dIoReadReg(0x210, REG_8);
		vIoWriteReg(0x210, REG_8, (valreg | (1<<3)));
		/*Perform DMA Read */
		rt = performDma(0);
	}
	vIoWriteReg(0x210, REG_8, (valreg & (~0x38)));
	if (rt != E__OK)
		rt = E__BSRERR;

	return rt;
}

/******************************************************************************
*
* vmeBridgelessIntHandler
*
* VME NMI/IRQ5 Interrupt Handler
*
******************************************************************************/
static void vmeBridgelessIntHandler (void)
{
	vmeInt = 1;
}

/******************************************************************************
*
* VMEBridgelessInterfaceTest
*
* Test VME signals on bridgless boards with TF622 test fixture
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (VMEBridgelessInterfaceTest, "VME Bridgeless Interface Test")
{
	int		iVector;
	UINT32	rt;
	UINT16	delay;
	UINT8	regVal;
	char	achBuffer[80];
	UINT8	slot;
	UINT16	statusReg;

	#define VME_BUS_REG		0x22D

	rt = E__OK;

	board_service(SERVICE__BRD_VME_BRIDGELESS_SLOTID, NULL, &slot);
	sprintf (achBuffer, "Geographic slot address: %d\n", slot);
	vConsoleWrite(achBuffer);

	// check if a different address is used for the VME Bridgeless Status Register
	rt = board_service(SERVICE__BRD_GET_VME_BRIDGELESS_REG, NULL, &statusReg);
	if (rt != E__OK)
	{
		// use default address
		statusReg = VME_BUS_REG;
	}

	// initialise VME status and control register 10
	vIoWriteReg (statusReg, REG_8, 0x12);		// clear SFAIL_ST and ACFAIL_ST
	regVal = dIoReadReg(statusReg, REG_8);
	if (regVal != 0)
	{
		vConsoleWrite("\nUnable to initialize VME port\n");
		vConsoleWrite("Ensure both switches on TF622 are released and no LEDs are lit.\n");
		sprintf(achBuffer, "Port 0x%X = %02X, [should be 0x00]\n", statusReg, regVal);
		vConsoleWrite(achBuffer);
		sprintf(achBuffer, "SFAIL_O:%d   SFAIL_I:%d   SFAIL_EN:%d SFAIL_ST:%d\nACFAIL_EN:%d ACFAIL_TY:%d ACFAIL_ST:%d\nTSI148_PR:%d\n", 
					((regVal & 0x80) >> 7), ((regVal & 0x40) >> 6), ((regVal & 0x20) >> 5), ((regVal & 0x10) >> 4),
					((regVal & 0x08) >> 3), ((regVal & 0x04) >> 2), ((regVal & 0x02) >> 1), ((regVal & 0x01)));
		vConsoleWrite(achBuffer);
		return E__BIT;
	}

	// flush the console input before we start
	while (kbhit() == 1)
	{
		getchar();
	}

	vConsoleWrite("\nTesting SYSFAIL input...\n");
	vConsoleWrite("Push SY switch and then press any key to continue...\n");
	delay = 300;		// 15 seconds
	while (kbhit() != 1)
	{
		vDelay(50);
		--delay;
		if (delay == 0)
		{
			vConsoleWrite("Timeout waiting for key press\n");
			rt = E__BIT + 1;		// operator error
			break;
		}
	}
	while (kbhit() == 1)
	{
		getchar();				// flush the console input
	}

#ifdef DEBUG
//	regVal = dIoReadReg(statusReg, REG_8);
//	sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//	vConsoleWrite(achBuffer);
#endif

	if (rt == E__OK)
	{
		regVal = dIoReadReg(statusReg, REG_8) & 0x40;
		if (regVal != 0)
		{
			vConsoleWrite("Release SY switch...\n");
			delay = 300;		// 15 seconds
			while (delay > 0)
			{
				regVal = dIoReadReg(statusReg, REG_8) & 0x40;
				if (regVal == 0)
				{
					break;
				}
				vDelay(50);
				--delay;
			}
			if (delay == 0)
			{
				vConsoleWrite("SYSFAIL input: FAIL - input not cleared\n");
				rt = E__BIT + 3;		// SYSFAIL input not cleared
			}
		}
		else
		{
			vConsoleWrite("SYSFAIL input: FAIL - input not seen\n");
			rt = E__BIT + 2;		// SYSFAIL input not seen
		}
	}

#ifdef DEBUG
//	regVal = dIoReadReg (statusReg, REG_8);
//	sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//	vConsoleWrite(achBuffer);
#endif

	if (rt == E__OK)
	{
		vConsoleWrite("SYSFAIL input: PASS\n");
		vDelay(750);

		while (kbhit() == 1)
		{
			getchar();				// flush the console input
		}
		vConsoleWrite("\nTesting SYSFAIL output...\n");
		vConsoleWrite("Press any key when SY LED is lit...\n");
		delay = 300;		// 5 seconds
		vIoWriteReg (statusReg, REG_8, 0x80);
		while (kbhit() != 1)
		{
			vDelay(50);
			--delay;
			if (delay == 0)
			{
				vConsoleWrite("Timeout waiting for key press\n");
				vConsoleWrite("SYSFAIL output: FAIL - output not seen\n");
				rt = E__BIT + 4;		// SYSFAIL output not seen
				break;
			}
		}
		while (kbhit() == 1)
		{
			getchar();				// flush the console input
		}
	}

#ifdef DEBUG
//	regVal = dIoReadReg (statusReg, REG_8);
//	sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//	vConsoleWrite(achBuffer);
#endif

	vIoWriteReg (statusReg, REG_8, 0);

#ifdef DEBUG
//	regVal = dIoReadReg (statusReg, REG_8);
//	sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//	vConsoleWrite(achBuffer);
#endif

	if (rt == E__OK)
	{
		vConsoleWrite("SYSFAIL output: PASS\n");
		vConsoleWrite("\nTesting SYSFAIL interrupt...\n");
		iVector = sysPinToVector(5, SYS_IOAPIC0);
		if (iVector == -1)
		{
			vConsoleWrite("Unable to configure interrupt\n");
			return E__BIT + 5;
		}
		sysInstallUserHandler(iVector, vmeBridgelessIntHandler);
		if (sysUnmaskPin(5, SYS_IOAPIC0) == -1)
		{
			sysInstallUserHandler(iVector, 0);
			vConsoleWrite("Unable to unmask interrupt\n");
			return E__BIT + 6;
		}
		vmeInt = 0;
		sysEnableInterrupts();
		vIoWriteReg (statusReg, REG_8, 0x20);			// enable SYSFAIL interrupt
		vDelay(50);
		vIoWriteReg (statusReg, REG_8, 0xA0);			// trigger interrupt
		vDelay(200);

#ifdef DEBUG
//		sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//		vConsoleWrite(achBuffer);
#endif

		if (sysMaskPin(5, SYS_IOAPIC0) == -1)
		{
			sysInstallUserHandler(iVector, 0);
			vConsoleWrite("Unable to mask interrupt\n");
			return E__BIT + 7;
		}

		regVal = dIoReadReg (statusReg, REG_8);
		if ((regVal & 0x10) == 0)
		{
			vConsoleWrite("SYSFAIL interrupt: FAIL, SFAIL_ST = 0 \n");
			rt = E__BIT + 8;
		}
		vIoWriteReg (statusReg, REG_8, 0x10);		// disable SFAIL_O, SFAIL_EN and clear SFAIL_ST

#ifdef DEBUG
//		regVal = dIoReadReg (statusReg, REG_8);
//		sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//		vConsoleWrite(achBuffer);
#endif
	}

	if (rt == E__OK)
	{
		if (vmeInt == 0)
		{
			vConsoleWrite("SYSFAIL  interrupt: FAIL, no interrupt detected\n");
			rt = E__BIT + 9;
		}
	}

	if (rt == E__OK)
	{
		vConsoleWrite("SYSFAIL interrupt: PASS\n");
		vConsoleWrite("\nTesting ACFAIL interrupt...\n");
		vmeInt = 0;
		vIoWriteReg (statusReg, REG_8, 0x0E);		// enable ACFAIL on IRQ5, ACFAIL_EN and clear ACFAIL_ST

#ifdef DEBUG
//		regVal = dIoReadReg (statusReg, REG_8);
//		sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//		vConsoleWrite(achBuffer);
#endif

		if (sysUnmaskPin(5, SYS_IOAPIC0) == -1)
		{
			sysInstallUserHandler(iVector, 0);
			vConsoleWrite("Unable to unmask interrupt\n");
			return E__BIT + 6;
		}

		vConsoleWrite("Push AC switch and then press any key to continue...\n");
		delay = 300;		// 15 seconds
		while (kbhit() != 1)
		{
			vDelay(50);
			--delay;
			if (delay == 0)
			{
				vConsoleWrite("Timeout waiting for key press\n");
				rt = E__BIT + 1;		// operator error
				break;
			}
		}
		while (kbhit() == 1)
		{
			getchar();				// flush the console input
		}
		vIoWriteReg (statusReg, REG_8, 0x04);		// disable ACFAIL_EN

#ifdef DEBUG
//		regVal = dIoReadReg (statusReg, REG_8);
//		sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//		vConsoleWrite(achBuffer);
#endif

		if (sysMaskPin(5, SYS_IOAPIC0) == -1)
		{
			sysInstallUserHandler(iVector, 0);
			vConsoleWrite("Unable to mask interrupt\n");
			return E__BIT + 7;
		}
		sysInstallUserHandler(iVector, 0);

		vConsoleWrite("Release AC switch and then press any key to continue...\n");
		delay = 300;		// 15 seconds
		while (kbhit() != 1)
		{
			vDelay(50);
			--delay;
			if (delay == 0)
			{
				vConsoleWrite("Timeout waiting for key press\n");
				rt = E__BIT + 1;		// operator error
				break;
			}
		}
		while (kbhit() == 1)
		{
			getchar();				// flush the console input
		}
	}

	if (rt == E__OK)
	{
		regVal = dIoReadReg (statusReg, REG_8);
		if ((regVal & 0x02) == 0)
		{
			vConsoleWrite("ACFAIL interrupt: FAIL, ACFAIL_ST = 0 \n");
			rt = E__BIT + 10;
		}
		vIoWriteReg (statusReg, REG_8, 0x02);		// enable ACFAIL NMI, disable ACFAIL_EN and clear ACFAIL_ST

#ifdef DEBUG
//		regVal = dIoReadReg (statusReg, REG_8);
//		sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//		vConsoleWrite(achBuffer);
#endif
	}

	if (rt == E__OK)
	{
		if (vmeInt == 0)
		{
			vConsoleWrite("ACFAIL interrupt: FAIL, no interrupt detected\n");
			rt = E__BIT + 11;
		}
	}

	if (rt == E__OK)
	{
		vConsoleWrite("ACFAIL interrupt: PASS\n");
		vConsoleWrite("\nTesting ACFAIL NMI...\n");
		vmeInt = 0;
		vIoWriteReg (statusReg, REG_8, 0x0A);		// enable ACFAIL NMI, ACFAIL_EN and clear ACFAIL_ST

#ifdef DEBUG
//		regVal = dIoReadReg (statusReg, REG_8);
//		sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//		vConsoleWrite(achBuffer);
#endif

		if (sysInstallUserHandler (2, vmeBridgelessIntHandler) != 0)			//NMI
		{
			vConsoleWrite("Unable to configure NMI\n");
			return E__BIT + 5;
		}

		vIoWriteReg(0x70, REG_8, 0x80);				// disable NMI
		regVal = dIoReadReg (0x61, REG_8) & 0x07;
		vIoWriteReg (0x61, REG_8, regVal | 0x08);
		vIoWriteReg (0x61, REG_8, regVal);
		vIoWriteReg (0x70, REG_8, 0);				// enable NMI

		vConsoleWrite("Push AC switch and then press any key to continue...\n");
		delay = 300;		// 15 seconds
		while (kbhit() != 1)
		{
			vDelay(50);
			--delay;
			if (delay == 0)
			{
				vConsoleWrite("Timeout waiting for key press\n");
				rt = E__BIT + 1;		// operator error
				break;
			}
		}
		while (kbhit() == 1)
		{
			getchar();				// flush the console input
		}
		vIoWriteReg (statusReg, REG_8, 0x00);		// disable ACFAIL_EN
		vIoWriteReg(0x70, REG_8, 0x80);			// disable NMI
		sysInstallUserHandler (2, 0);

#ifdef DEBUG
//		regVal = dIoReadReg (statusReg, REG_8);
//		sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//		vConsoleWrite(achBuffer);
#endif
	}

	if (rt == E__OK)
	{
		vConsoleWrite("Release AC switch and then press any key to continue...\n");
		delay = 300;		// 15 seconds
		while (kbhit() != 1)
		{
			vDelay(50);
			--delay;
			if (delay == 0)
			{
				vConsoleWrite("Timeout waiting for key press\n");
				rt = E__BIT + 1;		// operator error
				break;
			}
		}
		while (kbhit() == 1)
		{
			getchar();				// flush the console input
		}
	}

	if (rt == E__OK)
	{
		regVal = dIoReadReg (statusReg, REG_8);
		if ((regVal & 0x02) == 0)
		{
			vConsoleWrite("ACFAIL NMI: FAIL, ACFAIL_ST = 0 \n");
			rt = E__BIT + 12;
		}
		vIoWriteReg (statusReg, REG_8, 0x02);		// enable ACFAIL NMI, disable ACFAIL_EN and clear ACFAIL_ST

#ifdef DEBUG
//		regVal = dIoReadReg (statusReg, REG_8);
//		sprintf(achBuffer, "Port 0x%X = 0x%02X, [should be 0x00]\n", statusReg, regVal);
//		vConsoleWrite(achBuffer);
#endif
	}

	if (rt == E__OK)
	{
		if (vmeInt == 0)
		{
			vConsoleWrite("ACFAIL NMI: FAIL, no NMI detected\n");
			rt = E__BIT + 13;
		}
		else
		{
			vConsoleWrite("ACFAIL NMI: PASS\n");
		}
	}

	return rt;
}

/******************************************************************************
*
* VMESimpleTransferTest
*
* VME Bridge backplane simple transfer test
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (VMESimpleTransferTest, "VME Backplane Data Transfer Test")
{
	VME_DRV_INFO *vmeDrvInfo;
	UINT32	*address1;
	UINT32	*address2;
	UINT32	value;
	UINT32	cnt;
	UINT32	rt = E__OK;
	UINT8	slot;
	UINT8	bridgePresent;
	UINT32 dMaxSize=0;
#ifdef DEBUG
	char achBuffer[80];
#endif

	board_service(SERVICE__BRD_VME_BRIDGELESS_MODE, NULL, &bridgePresent);
	if (bridgePresent == 0)
	{
		// bridge not present
		return E__TEST_NO_DEVICE;
	}

#ifdef DEBUG
	sprintf (achBuffer, "Geographic slot address: %d\n", vmeDrvInfo->slot);
	sysDebugWriteString(achBuffer);
#endif

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;

	slot = vmeDrvInfo->slot + 1;			// adjacent board
	if (adTestParams[0] > 0)
	{
		if (adTestParams[0] != 1)
		{
			rt = E__FAIL;
		}
		else
		{
			// one based, remote slot number passed as a parameter
			// we require zero-based slot number for address offset calculation
			slot = adTestParams[1] - 1;
		}
	}

#ifdef DEBUG
	sprintf(achBuffer,"VME OutboundAddr1:(%08X)\n", (UINT32)vmeDrvInfo->outboundAddr1);
	sysDebugWriteString(achBuffer);

	sprintf(achBuffer,"VME OutboundAddr2:(%08X)\n", (UINT32)vmeDrvInfo->outboundAddr2);
	sysDebugWriteString(achBuffer);

	sprintf(achBuffer,"VME Slot to test:(%08X)\n", slot);
	sysDebugWriteString(achBuffer);
#endif

	if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
	{
		address1 = vmeDrvInfo->outboundAddr1 + (slot * _1MB_DWORD);		// 1MB per slot
		address2 = vmeDrvInfo->outboundAddr2 + (slot * _1MB_DWORD);

		rt = initTsi148();
	}
	else
	{
		address1 = vmeDrvInfo->outboundAddr1 + (slot * _1MB_DWORD);		// 8KB per slot
		address2 = vmeDrvInfo->outboundAddr2 + (slot * _1MB_DWORD);

		rt = initUniverse();
		rt = dAcquireUniverseVMEBus( vmeDrvInfo->slot);
	}

	if (rt == E__OK)
	{
		dMaxSize = _128KB_DWORD;
		for (cnt = 0; cnt < dMaxSize; ++cnt)
		{
		//if (cnt%256 == 0)
		//	{
		//		sprintf(buffer, "W1 ");
		//		vConsoleWrite(buffer);
		//	}
			if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
			{
				rt = writeDwordTsi148(&address1[cnt], 0x55555555, vmeDrvInfo->slot);
			}
			else
			{
				rt = writeDwordUniverse(&address1[cnt], 0x55555555);
			}
			if (rt != E__OK)
			{
				break;
			}
			if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
			{
				rt = writeDwordTsi148(&address2[cnt], 0x12345678, vmeDrvInfo->slot);
			}
			else
			{
				rt = writeDwordUniverse(&address2[cnt], 0x12345678);
			}
			if (rt != E__OK)
			{
				break;
			}
		}
		if (rt == E__OK)
		{
			for (cnt = 0; cnt < dMaxSize; ++cnt)
			{
			//	if (cnt%256 == 0)
			//	{
			//		sprintf(buffer, "R1 ");
			//		vConsoleWrite(buffer);
			//	}

				if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
				{
					rt = readDwordTsi148(&address1[cnt], &value, vmeDrvInfo->slot);
				}
				else
				{
					rt = readDwordUniverse(&address1[cnt], &value);
				}
				if (rt != E__OK)
				{
					break;
				}
				if (value != 0x55555555)
				{
#ifdef DEBUG
					sprintf(achBuffer,"Image 0 addr %08X: Wrote 0x55555555, read 0x%08X\n", (UINT32)&address1[cnt], value);
					sysDebugWriteString(achBuffer);
#endif
					rt = E__VME_SIMPLE_XFER + 1;
					break;
				}

				if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
				{
					rt = readDwordTsi148(&address2[cnt], &value, vmeDrvInfo->slot);
				}
				else
				{
					rt = readDwordUniverse(&address2[cnt], &value);
				}
				if (rt != E__OK)
				{
					break;
				}
				if (value != 0x12345678)
				{
#ifdef DEBUG
					sprintf(achBuffer,"Image 1 addr %08X: Wrote 0x12345678, read 0x%08X\n", (UINT32)&address2[cnt], value);
					sysDebugWriteString(achBuffer);
#endif
					rt = E__VME_SIMPLE_XFER + 2;
					break;
				}
			}
		}
	}

	if (rt == E__OK)
	{
		for (cnt = 0; cnt < dMaxSize; ++cnt)
		{
		//	if (cnt%256 == 0)
		//	{
		//		sprintf(buffer, "W2 ");
		//		vConsoleWrite(buffer);
		//	}

			if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
			{
				rt = writeDwordTsi148(&address1[cnt], 0xAAAAAAAA, vmeDrvInfo->slot);
			}
			else
     		{
				rt = writeDwordUniverse(&address1[cnt], 0xAAAAAAAA);
			}
			if (rt != E__OK)
			{
				break;
			}
		}
		if (rt == E__OK)
		{
			for (cnt = 0; cnt < dMaxSize; ++cnt)
			{
			//	if (cnt%256 == 0)
			//	{
			//		sprintf(buffer, "R2 ");
			//		vConsoleWrite(buffer);
			//	}

				if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
				{
					rt = readDwordTsi148(&address1[cnt], &value, vmeDrvInfo->slot);
				}
				else
				{
					rt = readDwordUniverse(&address1[cnt], &value);
				}
				if (rt != E__OK)
				{
					break;
				}
				if (value != 0xAAAAAAAA)
				{
#ifdef DEBUG
					sprintf(achBuffer,"Image 0 addr %08X: Wrote 0xAAAAAAAA, read 0x%08X\n", (UINT32)&address1[cnt], value);
					sysDebugWriteString(achBuffer);
#endif
					rt = E__VME_SIMPLE_XFER + 3;
					break;
				}

				if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
				{
					rt = readDwordTsi148(&address2[cnt], &value, vmeDrvInfo->slot);
				}
				else
				{
					rt = readDwordUniverse(&address2[cnt], &value);
				}
				if (rt != E__OK)
				{
					break;
				}
				if (value != 0x12345678)
				{
#ifdef DEBUG
					sprintf(achBuffer,"Image 1 addr %08X: Wrote 0x12345678, read 0x%08X\n", (UINT32)&address2[cnt], value);

					sysDebugWriteString(achBuffer);
#endif
					rt = E__VME_SIMPLE_XFER + 4;
					break;
				}
			}
		}
	}

	if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
	{
		// ensure the bus is released if there is an error
		dReleaseTsi148VMEBus();
		deinitTsi148();
	}
	else
	{
		dReleaseUniverseVMEBus();
		deinitUniverse();
	}

	return rt;
}

/******************************************************************************
*
* VMEDMATransferTest
*
* TSI148 Bridge backplane simple transfer testt
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (VMEDMATransferTest, "VME Backplane DMA Transfer Test")
{
	VME_DRV_INFO *vmeDrvInfo;
	UINT32 pHandle;
	UINT32	*dmaBuffer;
	UINT32	*vmeAddress1;
	UINT32	*vmeAddress2;
	UINT32	cnt;
	UINT32	rt = E__OK;
	UINT8	slot;
	UINT8	bridgePresent;
#ifdef DEBUG
	char achBuffer[80];
#endif

	board_service(SERVICE__BRD_VME_BRIDGELESS_MODE, NULL, &bridgePresent);
	if (bridgePresent == 0)
	{
		// bridge not present
		return E__TEST_NO_DEVICE;
	}

#ifdef DEBUG
	sprintf (achBuffer, "Geographic slot address: %d\n", vmeDrvInfo->slot);
	sysDebugWriteString(achBuffer);
#endif

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;

	slot = vmeDrvInfo->slot + 1;			// adjacent board
	if (adTestParams[0] > 0)
	{
		if (adTestParams[0] != 1)
		{
			rt = E__FAIL;
		}
		else
		{
			// one based, remote slot number passed as a parameter
			// we require zero-based slot number for address offset calculation
			slot = adTestParams[1] - 1;
		}
	}

	if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
	{
		// set VME addresses for remote board images
		vmeAddress1 = ((UINT32 *)0x66000000) + (slot * _1MB_DWORD);
		vmeAddress2 = ((UINT32 *)0x99000000) + (slot * _1MB_DWORD);

		rt = dAcquireTsi148VMEBus( vmeDrvInfo->slot);
	}
	else
	{
		// set VME addresses for remote board images
		vmeAddress1 = ((UINT32 *)0x66000000) + (slot * _1MB_DWORD);
		vmeAddress2 = ((UINT32 *)0x99000000) + (slot * _1MB_DWORD);

		rt = dAcquireUniverseVMEBus( vmeDrvInfo->slot);
	}

	if (rt == E__OK)
	{
		// map DMA buffer
		pHandle = dGetPhysPtr(TEST_DMA_ADDR, 0x10000, &tPtr2, (void *)&dmaBuffer);

		if (pHandle == E__FAIL)
		{
			rt = E__TEST_NO_MEM;
		}

		if (rt == E__OK)
		{
			if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
			{
				rt = initTsi148();
				if (rt == E__OK)
				{
					// fill image2 with 0x12345678
					for (cnt = 0; cnt < 0x400; ++cnt)
					{
						dmaBuffer[cnt] = 0x12345678;
					}
					rt = dmaXferTsi148(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress2, vmeDrvInfo->slot);
				}
				if (rt == E__OK)
				{
					// fill image1 with 0x55555555
					memset(dmaBuffer, 0x55, 0x10000);
					rt = dmaXferTsi148(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress1, vmeDrvInfo->slot);
				}
				if (rt == E__OK)
				{
					rt = dmaXferTsi148(READ_VME, vmeAddress2, (UINT32 *) TEST_DMA_ADDR, vmeDrvInfo->slot);
					if (rt == E__OK)
					{
						for (cnt = 0; cnt < 0x400; ++cnt)
						{
							// check that local buffer loaded from image2 is still 0x12345678
							if (dmaBuffer[cnt] != 0x12345678)
							{
								rt = E__VME_DMA_XFER + 1;
								break;
							}
						}
					}
				}
				if (rt == E__OK)
				{
					rt = dmaXferTsi148(READ_VME, vmeAddress1, (UINT32 *) TEST_DMA_ADDR, vmeDrvInfo->slot);
					if (rt == E__OK)
					{
						for (cnt = 0; cnt < 0x400; ++cnt)
						{
							// check that local buffer loaded from image1 is still 0x55555555
							if (dmaBuffer[cnt] != 0x55555555)
							{
								rt = E__VME_DMA_XFER + 2;
								break;
							}
						}
					}
				}
				if (rt == E__OK)
				{
					// fill image1 with 0xAAAAAAAA
					memset(dmaBuffer, 0xAA, 0x10000);
					rt = dmaXferTsi148(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress1, vmeDrvInfo->slot);
				}
				if (rt == E__OK)
				{
					rt = dmaXferTsi148(READ_VME, vmeAddress2, (UINT32 *) TEST_DMA_ADDR, vmeDrvInfo->slot);
					if (rt == E__OK)
					{
						for (cnt = 0; cnt < 0x400; ++cnt)
						{
							// check that local buffer loaded from image2 is still 0x12345678
							if (dmaBuffer[cnt] != 0x12345678)
							{
								rt = E__VME_DMA_XFER + 3;
								break;
							}
						}
					}
				}
				if (rt == E__OK)
				{
					rt = dmaXferTsi148(READ_VME, vmeAddress1, (UINT32 *) TEST_DMA_ADDR, vmeDrvInfo->slot);
					if (rt == E__OK)
					{
						for (cnt = 0; cnt < 0x400; ++cnt)
						{
							// check that local buffer loaded from image1 is still 0xAAAAAAAA
							if (dmaBuffer[cnt] != 0xAAAAAAAA)
							{
								rt = E__VME_DMA_XFER + 4;
								break;
							}
						}
					}
				}
			}
			else
			{
				rt = initUniverse();
				if (rt == E__OK)
				{
					// fill image2 with 0xFFFFFFFF
					memset(dmaBuffer, 0xFF, 0x10000);
					rt = dmaXferUniverse(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress2);
				}
				if (rt == E__OK)
				{
					// fill image1 with 0x00000000
					memset(dmaBuffer, 0, 0x10000);
					rt = dmaXferUniverse(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress1);
				}
				if (rt == E__OK)
				{
					rt = dmaXferUniverse(READ_VME, vmeAddress2, (UINT32 *) TEST_DMA_ADDR);
					if (rt == E__OK)
					{
						for (cnt = 0; cnt < 0x400; ++cnt)
						{
							// check that local buffer loaded from image2 is still 0xFFFFFFFF
							if (dmaBuffer[cnt] != 0xFFFFFFFF)
							{
								rt = E__VME_DMA_XFER + 1;
								break;
							}
						}
					}
				}
				if (rt == E__OK)
				{
					// fill image1 with 0x55555555
					memset(dmaBuffer, 0x55, 0x10000);
					rt = dmaXferUniverse(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress1);
				}
				if (rt == E__OK)
				{
					rt = dmaXferUniverse(READ_VME, vmeAddress2, (UINT32 *) TEST_DMA_ADDR);
					if (rt == E__OK)
					{
						for (cnt = 0; cnt < 0x400; ++cnt)
						{
							// check that local buffer loaded from image2 is still 0xFFFFFFFF
							if (dmaBuffer[cnt] != 0xFFFFFFFF)
							{
								rt = E__VME_DMA_XFER + 2;
								break;
							}
						}
					}
				}
				if (rt == E__OK)
				{
					// fill image1 with 0xAAAAAAAA
					memset(dmaBuffer, 0xAA, 0x10000);
					rt = dmaXferUniverse(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress1);
				}
				if (rt == E__OK)
				{
					rt = dmaXferUniverse(READ_VME, vmeAddress2, (UINT32 *) TEST_DMA_ADDR);
					if (rt == E__OK)
					{
						for (cnt = 0; cnt < 0x400; ++cnt)
						{
							// check that local buffer loaded from image2 is still 0xFFFFFFFF
							if (dmaBuffer[cnt] != 0xFFFFFFFF)
							{
								rt = E__VME_DMA_XFER + 3;
								break;
							}
						}
					}
				}
				if (rt == E__OK)
				{
					// fill image2 with 0x00000000
					memset(dmaBuffer, 0, 0x10000);
					rt = dmaXferUniverse(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress2);
				}
				if (rt == E__OK)
				{
					// fill image1 with 0xFFFFFFFF
					memset(dmaBuffer, 0xFF, 0x10000);
					rt = dmaXferUniverse(WRITE_VME, (UINT32 *) TEST_DMA_ADDR, vmeAddress1);
				}
				if (rt == E__OK)
				{
					rt = dmaXferUniverse(READ_VME, vmeAddress2, (UINT32 *) TEST_DMA_ADDR);
					if (rt == E__OK)
					{
						for (cnt = 0; cnt < 0x400; ++cnt)
						{
							// check that local buffer loaded from image2 is still 0x00000000
							if (dmaBuffer[cnt] != 0x0)
							{
								rt = E__VME_DMA_XFER + 4;
								break;
							}
						}
					}
				}
			}
		}
	}

	if (pHandle != E__FAIL)
	{
		vFreePtr(pHandle);
	}

	if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
	{
		// ensure the bus is released if there is an error
		dReleaseTsi148VMEBus();
		deinitTsi148();
	}
	else
	{
		dReleaseUniverseVMEBus();
		deinitUniverse();
	}

	return rt;
}

/******************************************************************************
*
* VMEBusErrorTest
*
* Generate a VME bus error
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (VMEBusErrorTest, "VME Bus Error Test")
{
	VME_DRV_INFO *vmeDrvInfo;
	UINT32	*address1;
	UINT32	value;
	UINT32	cnt;
	UINT32	rt = E__OK;
	UINT8	bridgePresent;
#ifdef DEBUG
	char achBuffer[80];
#endif
	board_service(SERVICE__BRD_VME_BRIDGELESS_MODE, NULL, &bridgePresent);
	if (bridgePresent == 0)
	{
		// bridge not present
		return E__TEST_NO_DEVICE;
	}

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;

#ifdef DEBUG
	sprintf(achBuffer,"VME OutboundAddr1:(%08X)\n",	(UINT32)vmeDrvInfo->outboundAddr1);
	sysDebugWriteString(achBuffer);
#endif

	// only 10 slots in the back plane so no board will ever respond to this address for slot 11
	address1 = vmeDrvInfo->outboundAddr1 + (11 * _1MB_DWORD);		// 1MB per slot

	if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
	{
		rt = initTsi148();
	}
	else
	{
		rt = initUniverse();
	}

	cnt = 1;
	if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
	{
		rt = readDwordTsi148(&address1[cnt], &value, vmeDrvInfo->slot);
	}
	else
	{
		rt = readDwordUniverse(&address1[cnt], &value);
	}

	if (rt == E__TEST_VMEBUSS_ERR)
	{
		rt = E__OK;
	}
	else
	{
		rt = E__TEST_VMEBUSS_ERR;
	}

	if (vmeDrvInfo->deviceId == TSI148_DEVID_VENID)
	{
		deinitTsi148();
	}
	else
	{
		deinitUniverse();
	}

	return rt;
}


/******************************************************************************
*
* Tsi148VmeConfig
*
* TSI148 VME Configuration Utility
*
*	Sets up an In-Bound image mapping local ram to VME mem space
*
* RETURNS: 0 on success else error code
*
******************************************************************************/

TEST_INTERFACE (Tsi148VmeConfig, "TSI148 VME Configuration Utility")
{
	UINT32  rt     = E__OK;
	UINT64	ddOffset;
	UINT8	bSlave = 3;				/* In-Bound image no			*/
	UINT32	dBase  = 0;				/* VME Base address				*/
	UINT32	dSize  = 0x04000000;	/* Size							*/
	UINT32	dAddr  = 0x40000000;	/* RAM Address					*/
	UINT32	dCtrl  = 0x80000025;	/* Control register value		*/
	UINT8	close  = 0;
	char	buffer[64];
	TSI148Regs *pUR;

#ifdef INCLUDE_LWIP_SMC
	VME_DRV_INFO *vmeDrvInfo;

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;
#endif

	rt = initTsi148();

	if( rt == E__OK )
	{
		pUR = (TSI148Regs*) mapRegSpace;

		if(adTestParams[0] == 1) /* 1 parameter passed by user */
		{
			if (adTestParams[1] > 7)
			{
				vConsoleWrite("Valid In-Bound Image Number is 0-7\n");
				return E__FAIL;
			}

			bSlave = adTestParams[1];
			dBase  = 0;
			dSize  = 0;
			dAddr  = 0;
			dCtrl  = 0;
			close  = 1;
			sprintf(buffer,"\nClosing In-Bound Image %x\n", bSlave);
			vConsoleWrite(buffer);
		}
		else if(adTestParams[0] == 5) /* 5 params passed by user */
		{
			if (adTestParams[1] > 7)
			{
				vConsoleWrite("Valid In-Bound Image Number is 0-7\n");
				return E__FAIL;
			}

			bSlave = adTestParams[1];
			dBase  = adTestParams[2];
			dSize  = adTestParams[3];
			dAddr  = adTestParams[4];
			dCtrl  = adTestParams[5];

			vConsoleWrite("To close the Image just pass In-Bound Image No\n");
		}
		else if(adTestParams[0] > 0)
		{
			vConsoleWrite("Wrong number of parameters, enter parameters in following order\n");
			vConsoleWrite("(In-Bound Image No, VME Base Address, Size, RAM Address, Control register)\n");
			vConsoleWrite("To close an Image just pass In-Bound Image No\n");
			return E__FAIL;
		}
		else
		{
			vConsoleWrite("Using Default Configuration, to change enter parameters in following order\n");
			vConsoleWrite("(In-Bound Image No, VME Base Address, Size, RAM Address, Control register) \n");
			vConsoleWrite("To close an Image just pass In-Bound Image No\n");
		}

		if(close == 0)
		{
			sprintf(buffer,"\nIn-Bound Image No \t%x\n", bSlave);
			vConsoleWrite(buffer);
			sprintf(buffer,"VME Base Address  \t0x%x\n", dBase);
			vConsoleWrite(buffer);
			sprintf(buffer,"Size              \t0x%x\n", dSize);
			vConsoleWrite(buffer);
			sprintf(buffer,"RAM Address       \t0x%x\n", dAddr);
			vConsoleWrite(buffer);
			sprintf(buffer,"Control register  \t0x%x\n", dCtrl);
			vConsoleWrite(buffer);
		}

		pUR->sitRegs[bSlave].sITAT  = 0;
		pUR->sitRegs[bSlave].sITSAL = BYTE_SWAP(dBase);
		pUR->sitRegs[bSlave].sITSAU = 0;
		pUR->sitRegs[bSlave].sITEAL = BYTE_SWAP((dBase + dSize - 0x10000));
		pUR->sitRegs[bSlave].sITEAU = 0;
		ddOffset  = dAddr;
		ddOffset -= dBase;
		pUR->sitRegs[bSlave].sITOFL = BYTE_SWAP(ddOffset);
		pUR->sitRegs[bSlave].sITOFU = BYTE_SWAP((ddOffset >> 32));
		pUR->sitRegs[bSlave].sITAT  = BYTE_SWAP(dCtrl);

		deinitTsi148();
	}
	else
	{
		vConsoleWrite("\nTSI148 Initialisation Failed\n");
		return E__FAIL;
	}

	return rt;
}



/******************************************************************************
*
* Tsi148VmeConfig
*
* TSI148 PCI Configuration Utility
*
*	Sets up an Out-Bound image mapping PCI mem space to VME mem space
*
* RETURNS: 0 on success else error code
*
******************************************************************************/
TEST_INTERFACE (Tsi148PciConfig, "TSI148 PCI Configuration Utility")
{
	UINT32  rt     = E__OK;
	UINT64	ddOffset;
	UINT8   close  = 0;
	UINT8	bSlave = 3;				/* Slave image no				*/
	UINT32	dBase  = 0;				/* PCI Base address				*/
	UINT32	dSize  = 0x04000000;	/* Size							*/
	UINT32	dAddr  = 0;				/* VME Base Address				*/
	UINT32	dCtrl  = 0x80000042;	/* Control register value		*/
	char 	buffer[64];
	TSI148Regs *pUR;

#ifdef INCLUDE_LWIP_SMC
	VME_DRV_INFO *vmeDrvInfo;

	board_service(SERVICE__BRD_GET_VME_BRIDGE_IMAGES, NULL, &vmeDrvInfo);
	mapRegSpace = vmeDrvInfo->mapRegSpace;
#endif

	rt = initTsi148();

	if( rt == E__OK )
	{
		pUR   = (TSI148Regs*) mapRegSpace;
		dBase = (pUR->sPCI_BSL & 0xffffff00) - (65 * 0x100000);

		if(adTestParams[0] == 1) /* 1 parameter passed by user */
		{
			if (adTestParams[1] > 7)
			{
				vConsoleWrite("Valid Out-Bound Image Number is 0-7\n");
				return E__FAIL;
			}

			bSlave = adTestParams[1];
			dBase  = 0;
			dSize  = 0;
			dAddr  = 0;
			dCtrl  = 0;
			close  = 1;

			sprintf(buffer,"\nClosing Out-Bound Image %x\n", bSlave);
			vConsoleWrite(buffer);
		}
		else if(adTestParams[0] == 5) /* 5 params passed by user */
		{
			if (adTestParams[1] > 7)
			{
				vConsoleWrite("Valid Out-Bound Image Number is 0-7\n");
				return E__FAIL;
			}

			bSlave = adTestParams[1];
			dBase  = adTestParams[2];
			dSize  = adTestParams[3];
			dAddr  = adTestParams[4];
			dCtrl  = adTestParams[5];

			vConsoleWrite("To close the Image just pass Out-Bound Image No\n");
		}
		else if(adTestParams[0] > 0)
		{
			vConsoleWrite("Wrong number of parameters, enter parameters in following order\n");
			vConsoleWrite("(Out-Bound Image No, PCI Base Address, Size, VME Base Address, Control register)\n");
			vConsoleWrite("To close an Image just pass Out-Bound Image No\n");
			return E__FAIL;
		}
		else
		{
			vConsoleWrite("Using Default Configuration, to change enter parameters in following order\n");
			vConsoleWrite("(Out-Bound Image No, PCI Base Address, Size, VME Base Address, Control register)\n");
			vConsoleWrite("To close an Image just pass Out-Bound Image No\n");
		}

		if(close == 0)
		{
			sprintf(buffer,"\nOut-Bound Image No \t%x\n", bSlave);
			vConsoleWrite(buffer);
			sprintf(buffer,"PCI Base Address   \t0x%x\n", dBase);
			vConsoleWrite(buffer);
			sprintf(buffer,"Size               \t0x%x\n", dSize);
			vConsoleWrite(buffer);
			sprintf(buffer,"VME Base Address   \t0x%x\n", dAddr);
			vConsoleWrite(buffer);
			sprintf(buffer,"Control register   \t0x%x\n", dCtrl);
			vConsoleWrite(buffer);
		}

		pUR->sotRegs[bSlave].sOTAT  = 0;
		pUR->sotRegs[bSlave].sOTSAL = BYTE_SWAP(dBase);
		pUR->sotRegs[bSlave].sOTSAU = 0;
		pUR->sotRegs[bSlave].sOTEAL = BYTE_SWAP((dBase + dSize - 0x10000));
		pUR->sotRegs[bSlave].sOTEAU = 0;
		ddOffset  = dAddr;
		ddOffset -= dBase;
		pUR->sotRegs[bSlave].sOTOFL = BYTE_SWAP(ddOffset);
		pUR->sotRegs[bSlave].sOTOFU = BYTE_SWAP((ddOffset >> 32));
		pUR->sotRegs[bSlave].sOTAT  = BYTE_SWAP(dCtrl);

		deinitTsi148();
	}
	else
	{
		vConsoleWrite("\nTSI148 Initialisation Failed\n");
		return E__FAIL;
	}

	return rt;
}
