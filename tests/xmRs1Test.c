
/************************************************************************
 *                                                                      *
 *      Copyright 2012 Concurrent Technologies, all rights reserved.    *
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

/* xmRs1Test.c - XM RS1, multiple serial port test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/xmRs1Test.c,v 1.2 2013-11-28 15:31:33 mgostling Exp $
 * $Log: xmRs1Test.c,v $
 * Revision 1.2  2013-11-28 15:31:33  mgostling
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013-09-04 07:46:46  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2012/04/26 15:47:35  cdobson
 * Added code for getting parameters from CPCI soakmaster when
 * testing VME boards.
 *
 * Revision 1.3  2012/04/17 10:24:51  cdobson
 * Changed external loopback such that it only tests the first 3 serial channels.
 * An additional parameter is used to allow testing of all the channels.
 *
 * Revision 1.2  2012/03/27 08:50:09  cdobson
 * Added instance and port numbers to returned error code.
 *
 * Revision 1.1  2012/03/21 15:36:56  cdobson
 * First checkin of the XM RS1/20x test.
 *
 */


/* includes */
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>	
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>
#include <bit/interrupt.h>

#include <devices/scc16550.h>

#if defined(VME)
#include <cute/idx.h>
#endif

/* defines */
//#define DEBUG
#define E__MEM_ALLOC				(E__BIT | E__CUTE)
#define E__TEST_PORTS				(E__BIT + 0x2)
#define E__INVALID_PARAM            (E__BIT + 0x4)
#define E__TEST_REGACCESS			(E__BIT + 0x6)
#define E__TEST_INTERRUPT_TIMEOUT	(E__BIT + 0xc)
#define E__TEST_TX_TIMEOUT			(E__BIT + 0x12)
#define E__TEST_RX_TIMEOUT			(E__BIT + 0x18)
#define E__TEST_DATA_MISMATCH		(E__BIT + 0x1E)
#define E__TEST_RX_NOTEMPTY_TIMEOUT	(E__BIT + 0x24)
#define E__TEST_TX_EMPTY_TIMEOUT	(E__BIT + 0x25)
#define E__TEST_CTS					(E__BIT + 0x2A)
#define	E__TEST_DSR					(E__BIT + 0x30)
#define	E__TEST_DCD					(E__BIT + 0x36)
#define	E__TEST_RI					(E__BIT + 0x3c)
#define E__BAUD_9600				(E__BIT + 0x42)
#define E__BAUD_19200				(E__BIT + 0x48)
#define E__BAUD_38400				(E__BIT + 0x4E)
#define E__BAUD_56000				(E__BIT + 0x54)
#define E__BAUD_115200				(E__BIT + 0x5A)
#define E__LB_TXRDY_TIMEOUT			(E__BIT + 0x60)
#define E__LB_RX_TIMEOUT			(E__BIT + 0x66)
#define E__TEST_DTR					(E__BIT + 0x6C)
#define E__NO_PORTS_TO_TEST			(E__BIT + 0x72)
#define E__PIN_MAP					(E__BIT + 0x73)
#define E__UNMASK_SLOT				(E__BIT + 0x74)
#define E__MASK_SLOT				(E__BIT + 0x75)
#define E__INT_ERROR				(E__BIT + 0x76)

#define BANK2						LCR_DLAB
#define TIMEOUT						0x0100000
#define TIMEOUT_1S					0x1000
#define TIMED_OUT					0
#define	DCD_MASK					0x80
#define	RI_MASK						0x40
#define	DSR_MASK					0x20
#define	CTS_MASK					0x10
#define	RTSC_MASK					0x02
#define	DTRC_MASK					0x01
#define MOD_LOOP					0x10

#define OXFORD_SEMI_VID				0x1415
#define OXPCI945_DID_4PORT			0xC208
#define OXPCI945_DID_8PORT			0xC308

#define MAX_XM_RS1_PORTS			8

/* typedefs */
typedef struct {
	UINT16	wBaseAddr;
} PORTINFO;

typedef struct {
	UINT32	   dInstance;
	PCI_PFA	   pfa;
	PTR48      tPtr1;
	PTR48	   tPtr2;
	UINT32     mDevHandle;
	UINT8     *pbDevRegs;
	UINT32     bMaxPorts;
	PORTINFO   asPortInfo[MAX_XM_RS1_PORTS];
} DEVICE;

#if defined(VME)
typedef  struct tagVmeTestParams
{
	UINT32 dCount;
    UINT32 dFlag;
} VME_TEST_PARAMS;
#endif

/* constants */
#if defined(VME)
const UINT8 abLoopbackTypes[] =
{
	2,
	IDX_UINT32_TYPE,
	IDX_UINT32_TYPE
};
const UINT8 abInterruptTypes[] =
{
	1,
	IDX_UINT32_TYPE
};
#endif

/* locals */
static UINT8 bRtsLoop = CTS_MASK;
static UINT8 bDtrLoop = DSR_MASK | DCD_MASK;
static UINT8 bIntFlag = 0;
static UINT8 bVector[8];
/* globals */
/* externals */
#if  defined(VME)
extern UINT8 sysGetTestHandler (void);
#endif
/* forward declarations */


/******************************************************************************
 * bReadReg8 - Read memory mapped register
 * This function reads a byte from the memory mapped registers.
 * RETURNS: data byte
 */
static UINT8 bReadReg8 (DEVICE *pDevice, UINT16 wOffset)
{
	return *((UINT8 *)(pDevice->pbDevRegs + wOffset));
}


/******************************************************************************
 * vWriteReg8 - Write memory mapped register
 * This function write a byte to the memory mapped registers.
 * RETURNS: N/A
 */
static void vWriteReg8 (DEVICE *pDevice, UINT16 wOffset, UINT8 bValue)
{
	*((UINT8 *)(pDevice->pbDevRegs + wOffset)) = bValue;
}


/******************************************************************************
 * vInitPciDevice - initializes the PCI configuration registers
 * This function initialises the PCI device registers.
 * RETURNS: N/A
 */
static UINT32 vInitPciDevice(DEVICE *pDevice)
{
	UINT32	dMapRegSpace=0;
	UINT32	dDeviceAddr;
	UINT16	wTemp;
#ifdef DEBUG
	char	achText[100];
#endif

	/* disable memory and I/O accesses */
	wTemp  = PCI_READ_WORD (pDevice->pfa, PCI_COMMAND);
	wTemp &= ~(PCI_MEMEN | PCI_IOEN);
	PCI_WRITE_WORD (pDevice->pfa, PCI_COMMAND, wTemp);

	dDeviceAddr  = PCI_READ_DWORD (pDevice->pfa, PCI_BAR0);
	dDeviceAddr &= PCI_MEM_BASE_MASK;

	#ifdef DEBUG
		sprintf(achText,"Base Address %x\n",dDeviceAddr);
		vConsoleWrite(achText);
	#endif

	pDevice->mDevHandle = dGetPhysPtr(dDeviceAddr,128,&pDevice->tPtr1,(void*)&dMapRegSpace);
	pDevice->pbDevRegs = (UINT8 *)dMapRegSpace;

	if (pDevice->mDevHandle == E__FAIL)
	{
		#ifdef DEBUG
			sprintf(achText, "Can not Allocate Mem Mapped Reg Space for XM RS1 instance %d\n",
					pDevice->dInstance );
			vConsoleWrite(achText);
		#endif
		return E__MEM_ALLOC;
	}

	/* set the maximum latency */
	PCI_WRITE_BYTE (pDevice->pfa, PCI_LATENCY, 0x10);

	/* enable I/O, MEM, Bus Mastering and SERR */
	wTemp  = PCI_READ_WORD (pDevice->pfa, PCI_COMMAND);
	wTemp &= ~0x0147;
	wTemp |=  0x0107;
	PCI_WRITE_WORD (pDevice->pfa, PCI_COMMAND, wTemp);

	return E__OK;
}


/******************************************************************************
 * dFindXmRs1 - Find instance of a XM RS1 module.
 * This function performs a scan of the PCI bus to find an instance of
 * a XM RS1 XMC module.
 * RETURNS: E__OK if found otherwise E__DEVICE_NOT_FOUND
 */
static UINT32 dFindXmRs1 (UINT32 dInstance, PCI_PFA *pPfa, UINT16 *wDevId)
{
	UINT8	bBus, bDev, bMaxBus;
	UINT32	dFound = 0;
#ifdef DEBUG
	char    achText[100];
#endif
	
	bMaxBus = 1;

	// scan all busses, devices and assume function 0
	for (bBus = 0; bBus <= bMaxBus; bBus++)
	{
		for (bDev = 0; bDev < 32; bDev++)
		{
			UINT16 wVid;

			wVid = PCI_READ_WORD (PCI_MAKE_PFA(bBus, bDev, 0), PCI_VENDOR_ID);
			if (wVid != 0xffff)
			{
				UINT16 wClass;
				
				wClass = PCI_READ_WORD (PCI_MAKE_PFA(bBus, bDev, 0), PCI_SUB_CLASS);
				if (wClass == 0x604)
				{
					UINT8 bSubBus;
					bSubBus = PCI_READ_BYTE (PCI_MAKE_PFA(bBus, bDev, 0), PCI_SUB_BUS);
					if (bSubBus > bMaxBus)
					{
						bMaxBus = bSubBus;
						#ifdef DEBUG
							sprintf (achText,"Max bus: %d\n", bMaxBus);
							vConsoleWrite(achText);
						#endif
					}
				}
			}
			if (wVid == OXFORD_SEMI_VID)
			{
				UINT32 wDid;
				
				wDid = PCI_READ_WORD (PCI_MAKE_PFA(bBus, bDev, 0), PCI_DEVICE_ID);
				if ((wDid == OXPCI945_DID_4PORT) || (wDid == OXPCI945_DID_8PORT))
				{
					if (++dFound == dInstance)
					{
						*pPfa = PCI_MAKE_PFA(bBus, bDev, 0);
						*wDevId = wDid;
						return E__OK;						
					}
				}
			}
		}
	}
	return E__DEVICE_NOT_FOUND;
}

/******************************************************************************
 * dCheckDevicePresent - Find device to test
 * This function initialises finds and initialises the PCI device and data
 * structures for the other tests. 
 * RETURNS: E__OK if device present
 */
static UINT32 dCheckDevicePresent (DEVICE *pDevice)
{
	UINT8  done = 0;
	UINT32 dStatus = E__OK;
	UINT16 wDid;
	char   achText[60];
#ifdef CUTE
	char   *psTestText = "Check device";
#endif

	while( (done != 1) && (dStatus == E__OK) )
	{
		if ((dStatus = dFindXmRs1(pDevice->dInstance, &pDevice->pfa, &wDid)) == E__OK)
		{
			UINT8   bPort;

			vInitPciDevice (pDevice);

			#ifdef DEBUG
				sprintf(achText ,"XM RS1/20x func %d Found\n", pDevice->dInstance);
				vConsoleWrite(achText);
			#endif
			
			pDevice->bMaxPorts = *(pDevice->pbDevRegs + 0x004);
			
			sprintf(achText ,"Found %d channel XM RS1 at Bus:%d Dev:%d Func:%d\n", pDevice->bMaxPorts-1,
					PCI_PFA_BUS(pDevice->pfa), PCI_PFA_DEV(pDevice->pfa), PCI_PFA_FUNC(pDevice->pfa));
			vConsoleWrite(achText);

			// Check number of channels

			if ((wDid == OXPCI945_DID_4PORT) && pDevice->bMaxPorts != 4)
			{
				dStatus = E__TEST_REGACCESS;
				#ifdef CUTE
					sprintf(achText ,"%s: Found a 4 channel controller but device reports %d channels\n", psTestText, pDevice->bMaxPorts);
					vConsoleWrite(achText);
				#endif
				return dStatus;
			}

			if ((wDid == OXPCI945_DID_8PORT) && pDevice->bMaxPorts != 8)
			{
				dStatus = E__TEST_REGACCESS;
				#ifdef CUTE
					sprintf(achText ,"%s: Found a 8 channel controller but device reports %d channels\n", psTestText, pDevice->bMaxPorts);
					vConsoleWrite(achText);
				#endif
				return dStatus;
			}

			// Initialise the serial port base address information

			for (bPort = 0; bPort < pDevice->bMaxPorts; bPort++)
			{
				if (bPort > sizeof(pDevice->asPortInfo))
					break;
					
				pDevice->asPortInfo[bPort].wBaseAddr = 0x1000 + (0x200 * bPort);
			}
			
			// for the XM RS1 the last port is not available so
			// decrement the max number of ports
			
			pDevice->bMaxPorts--;
				
			done++;
		}
	}

	return dStatus;
}


/*****************************************************************************
 * SerialInit: initialise a serial port for 8 data, no parity, 1 stop
 *
 * RETURNS: None
 */

static void SerialInit 
(
	DEVICE	*pDevice,		/* device pointer    */
	UINT8	bPort,			/* port number       */
	UINT16	wBaudDiv		/* baud rate divisor */
)
{
	UINT16 wBaseAddr = pDevice->asPortInfo[bPort].wBaseAddr;
	
	/* Program baud rate divisor */

	vWriteReg8 (pDevice, wBaseAddr + LCR, LCR_DLAB);		/* DLAB = 1 */
	vWriteReg8 (pDevice, wBaseAddr + DLL, (UINT8)(wBaudDiv & 0x00FF));
	vWriteReg8 (pDevice, wBaseAddr + DLH, (UINT8)(wBaudDiv >> 8));
	vDelay (30);

	/* Select 8N1 format */

	vWriteReg8 (pDevice, wBaseAddr + LCR, LCR_DATA8 | LCR_NONE | LCR_STOP1);

	/* Disable interrupts */

	vWriteReg8 (pDevice, wBaseAddr + IER, 0);

	/* Enable and reset FIFOs */

	vWriteReg8 (pDevice, wBaseAddr + FCR, FCR_FEN | FCR_CRF | FCR_CTF);
	vDelay (30);

} /* SerialInit () */


/*****************************************************************************
 * dPortLoopbackTest: function for testing the external loop-back
 * 			   and modem access of the serial port
 *
 * RETURNS: Test Status
 */

static UINT32 dPortLoopbackTest (DEVICE *pDevice, UINT8 bPort)
{
	UINT16	wBaudDiv = BAUD_9600;
	UINT32	dTestStatus = E__OK, i;
	UINT8	rx, tx;
	UINT8	bRegVal;
	UINT16	wBaseAddr;
	char	achText[100];
	char	*psTestText = "Loopback test";
	
	sprintf(achText ,"%s: Testing CH %d of %d\n", psTestText, bPort, pDevice->bMaxPorts);
	vConsoleWrite(achText);
		
	SerialInit(pDevice, bPort, wBaudDiv);
	
	wBaseAddr = pDevice->asPortInfo[bPort].wBaseAddr;

	/* Flush Rx */
	i = 0;
	while (((bReadReg8(pDevice, wBaseAddr + LSR) & LSR_DR) != 0) && (i < TIMEOUT))
		rx = bReadReg8(pDevice, wBaseAddr + RX);
		
	if (i >= TIMEOUT)
	{
		dTestStatus = E__TEST_RX_NOTEMPTY_TIMEOUT;
		#ifdef CUTE  
			sprintf(achText ,"%s: CH %d RX NOT EMPTY TIMEOUT \n", psTestText, bPort);
			vConsoleWrite(achText);
		#endif
		goto dtest_error;
	}
		
	// Write/read characters
	for (tx = 0; tx < 0xFF; tx++)
	{
		i = 0;
		/* Wait for Tx Empty */
		while (((bReadReg8(pDevice, wBaseAddr + LSR) & LSR_DHRE) == 0x0) && (i < TIMEOUT))
			i++;
		if (i >= TIMEOUT)
		{	
			dTestStatus = E__TEST_TX_EMPTY_TIMEOUT;
			#ifdef CUTE  
				sprintf(achText ,"%s: CH %d TX TIMEOUT \n", psTestText, bPort);
				vConsoleWrite(achText);
			#endif
			break;
		}

		/* Write the test character */
		vWriteReg8 (pDevice, wBaseAddr + TX, tx);

		/* Wait for Rx Data available */
		i = 0;
		while (((bReadReg8(pDevice, wBaseAddr + LSR) & LSR_DR) == 0x0) && (i < TIMEOUT))
			i++;

		if (i >= TIMEOUT)
		{	
			dTestStatus = E__TEST_RX_TIMEOUT;	
			#ifdef CUTE  
				sprintf(achText ,"%s: CH %d RX TIMEOUT \n", psTestText, bPort);
				vConsoleWrite(achText);
			#endif
			break;
		}

		/* read and check character */
		rx = bReadReg8(pDevice, wBaseAddr + RX);
		if (tx != rx)
		{
			dTestStatus = E__TEST_DATA_MISMATCH;	
			#ifdef CUTE  
				sprintf(achText ,"%s: CH %d DATA MISMATCH TX 0x%02x RX 0x%02x\n", psTestText, bPort, tx, rx);
				vConsoleWrite(achText);
			#endif
			break;
		}
		else
		{
			dTestStatus = E__OK;
		}	
	}

	/* Modem Signal Test */
	if (dTestStatus == E__OK)
	{
		bRegVal = bReadReg8(pDevice, wBaseAddr + MCR) | RTSC_MASK | MOD_LOOP;
		vDelay(1);
		vWriteReg8 (pDevice, wBaseAddr + MCR, bRegVal);
		/*for(i = 0; i < 10000; i++){}			*/
		vDelay(10);
		if (bRtsLoop == CTS_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & CTS_MASK) == 0x00)
			{
				dTestStatus = E__TEST_CTS;	
			#ifdef CUTE  
				sprintf(achText ,"%s: CH %d CTS MASK 1 \n", psTestText, bPort);
				vConsoleWrite(achText);
			#endif
				goto dtest_error;
			}
		if (bRtsLoop == DSR_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & DSR_MASK) == 0x00)
			{
				dTestStatus = E__TEST_DSR;	
			#ifdef CUTE  
				sprintf(achText ,"%s: CH %d DSR MASK 1 \n", psTestText, bPort);
				vConsoleWrite(achText);
			#endif
				goto dtest_error;
			}
		if (bRtsLoop == DCD_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & DCD_MASK) == 0x00)
			{
				dTestStatus = E__TEST_DCD;	
			#ifdef CUTE  
				sprintf(achText ,"%s: CH %d DCD MASK 1 \n", psTestText, bPort);
				vConsoleWrite(achText);
			#endif
				goto dtest_error;
			}
		if (bRtsLoop == RI_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & RI_MASK) == 0x00)
			{
				dTestStatus = E__TEST_RI;	
			#ifdef CUTE  
				sprintf(achText ,"%s: CH %d RI MASK 1 \n", psTestText, bPort);
				vConsoleWrite(achText);
			#endif
				goto dtest_error;
			}

		bRegVal = bReadReg8(pDevice, wBaseAddr + MCR) & (~RTSC_MASK);
		vDelay(1);
		vWriteReg8 (pDevice, wBaseAddr + MCR, bRegVal);
		vDelay(10);
		if (bRtsLoop == CTS_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & CTS_MASK) == CTS_MASK)
			{
				dTestStatus = E__TEST_CTS;	
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d CTS MASK 2 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bRtsLoop == DSR_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & DSR_MASK) == DSR_MASK)
			{
				dTestStatus = E__TEST_DSR;
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d DSR MASK 2 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bRtsLoop == DCD_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & DCD_MASK) == DCD_MASK)
			{
				dTestStatus = E__TEST_DCD;	
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d DCD MASK 2 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bRtsLoop == RI_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & RI_MASK) == RI_MASK)
			{
				dTestStatus = E__TEST_RI;	
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d RI MASK 2 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}

		bRegVal = bReadReg8(pDevice, wBaseAddr + MCR) | DTRC_MASK;
		vDelay(1);
		vWriteReg8 (pDevice, wBaseAddr + MCR, bRegVal);
		vDelay(10);			
		if (bDtrLoop == CTS_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & CTS_MASK) == 0x00)
			{
				dTestStatus = E__TEST_CTS;
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d CTS MASK 3 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bDtrLoop == DSR_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & DSR_MASK) == 0x00)
			{
				dTestStatus = E__TEST_DSR;	
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d DSR MASK 3 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bDtrLoop == DCD_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & DCD_MASK) == 0x00)
			{
				dTestStatus = E__TEST_DCD;	
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d DCD MASK 3 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bDtrLoop == RI_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & RI_MASK) == 0x00)
			{
				dTestStatus = E__TEST_RI;	
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d RI MASK 3 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		bRegVal = bReadReg8(pDevice, wBaseAddr + MCR) & ~DTRC_MASK;
		vDelay(1);
		vWriteReg8 (pDevice, wBaseAddr + MCR, bRegVal);
		vDelay(10);			
		if (bDtrLoop == CTS_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & CTS_MASK) == CTS_MASK)
			{
				dTestStatus = E__TEST_CTS;
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d CTS MASK 4 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bDtrLoop == DSR_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & DSR_MASK) == DSR_MASK)
			{
				dTestStatus = E__TEST_DSR;
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d DSR MASK 4 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bDtrLoop == DCD_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & DCD_MASK) == DCD_MASK)
			{
				dTestStatus = E__TEST_DCD;	
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d DCD MASK 4 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
		if (bDtrLoop == RI_MASK)
			if ((bReadReg8(pDevice, wBaseAddr + MSR) & RI_MASK) == RI_MASK)
			{
				dTestStatus = E__TEST_RI;	
				#ifdef CUTE  
					sprintf(achText ,"%s: CH %d RI MASK 4 \n", psTestText, bPort);
					vConsoleWrite(achText);
				#endif
				goto dtest_error;
			}
	}
		
		
	#ifdef DEBUG
		sprintf(achText ,"%s: CH %d PASS\n", psTestText, bPort);
		vConsoleWrite(achText);
	#endif

	return dTestStatus;

dtest_error:
	dTestStatus += (bPort * 0x100);
	#ifdef CUTE  
		sprintf(achText ,"%s: CH %d FAIL %08x\n", psTestText, bPort, dTestStatus);
		vConsoleWrite(achText);
	#endif
	return (dTestStatus);
}



/******************************************************************************
 * vIntHandler - Interrupt handler
 *
 * RETURNS: none
 */
static void vIntHandler (void)
{
	bIntFlag = 1;
	return;
}


/******************************************************************************
 * vInitInterrupts - Initialise PIRQ interrupts
 *
 * RETURNS: none
 */
static void vInitInterrupts()
{
	int i;

	for (i = 0; i < 8; i++)
	{
		bVector[i] = sysPinToVector (16 + i, SYS_IOAPIC0);
		sysInstallUserHandler (bVector[i], vIntHandler);
		sysUnmaskPin (16 + i, SYS_IOAPIC0);
	}
}

/******************************************************************************
 * vDeInitInterrupts - Restore PIRQ interrupts
 *
 * RETURNS: none
 */
static void vDeInitInterrupts()
{
	int i;

	for (i = 0; i < 8; i++)
	{
		sysMaskPin (16 + i, SYS_IOAPIC0) ;
		sysInstallUserHandler (bVector[i], 0) ;
	}
}


/******************************************************************************
 * dPortInterruptTest - Test UART interrupt
 *
 * RETURNS: E__OK if test pass
 */
static UINT32 dPortInterruptTest (DEVICE *pDevice, UINT8 bPort)
{
	UINT16	wBaudDiv = BAUD_9600;
	UINT32	dStatus = E__OK;
	UINT16	wBaseAddr;
	char	achText[100];
	char	*psTestText = "Interrupt Test";

	sprintf(achText ,"%s: Testing CH %d of %d\n", psTestText, bPort, pDevice->bMaxPorts);
	vConsoleWrite(achText);

	wBaseAddr = pDevice->asPortInfo[bPort].wBaseAddr;

	SerialInit (pDevice, bPort, wBaudDiv);

	// Enable UART loop-back

	vWriteReg8 (pDevice, wBaseAddr + MCR, MCR_LOOP);

	// Clear the interrupt flag

	bIntFlag = 0;

	//enable interrupt

	vInitInterrupts ();
	vWriteReg8 (pDevice, wBaseAddr + IER, IER_RDA);
	vWriteReg8 (pDevice, wBaseAddr + MCR, MCR_LOOP | MCR_AUX2);
	
	// Write character, this will be received and generate a rx interrupt
	
	vWriteReg8 (pDevice, wBaseAddr + TX, 0x55);
	
	vDelay(10);//wait interrupt

	//read interrupt
	
	if (bIntFlag == 0)
	{
		// No interrupt
		#ifdef CUTE
			sprintf(achText ,"%s: CH %d Interrupt NOT detected\n", psTestText, bPort);
			vConsoleWrite(achText);
		#endif
		dStatus = E__INT_ERROR;
	}
	else
	{
		UINT8 bReg;
		// read interrupt status
		bReg = bReadReg8 (pDevice, wBaseAddr + IIR);
		
		if ((bReg & (IIR_IP | IIR_RDA)) != IIR_RDA)
		{
			// we have an interrupt, but not what we expected
			#ifdef CUTE  
				sprintf(achText ,"%s: CH %d Interrupt detected but status not set (IIR = 0x%02x)\n", psTestText, bPort, bReg);
				vConsoleWrite(achText);
			#endif			
			dStatus = E__INT_ERROR;		
		}
		
	}

	//disable interrupt

	bReadReg8 (pDevice, wBaseAddr + RX);		// Read character
	vWriteReg8 (pDevice, wBaseAddr + MCR, 0);	// disable loop-back and interrupt
	vWriteReg8 (pDevice, wBaseAddr + IER, 0);
	
	//restore interrupts

	vDeInitInterrupts();
	
	return dStatus;
}

/******************************************************************************
 * This test takes one parameter :-
 * 	1 - The number of XM RS1 modules to test. Each module found will be tested
 * 		until all the modules have been tested or an error occurs.
 *
 */
TEST_INTERFACE (XMRS1Test, "XM RS1 Loopback Test")
{
	UINT32	dTestStatus = E__OK;
	UINT32	dInstance, dCount;
	UINT8	bPort;
	DEVICE	sDevice;

#if  defined(VME)
	VME_TEST_PARAMS sVmeTestParams;
	EXPECTED_TYPES*	psExpectedTypes;
//	UINT16  wIdxStatus;
	
    if(sysGetTestHandler() == TH__STH)
    {
		adTestParams[0] = 0;

    	psExpectedTypes = (EXPECTED_TYPES*)&abLoopbackTypes;
		memset((void*)&sVmeTestParams,0,sizeof(VME_TEST_PARAMS));

    	/*wIdxStatus =*/ wIDX_GetParams (psExpectedTypes, (UINT8*)&sVmeTestParams);

//    	if( wIdxStatus != E__OK )
//        {
//            return E__INVALID_PARAM;
//        }

    	if (sVmeTestParams.dCount != 0)
    	{
    		adTestParams[0] = 1;
    		adTestParams[1] = sVmeTestParams.dCount;
			adTestParams[2] = 0;
    		if (sVmeTestParams.dFlag != 0)
    		{
    			adTestParams[0] = 2;
    			adTestParams[2] = sVmeTestParams.dFlag;
    		}
    	}
    }
#endif

	// check test parameters
	if (adTestParams[0])
	{
		dCount = adTestParams[1];

		// Loop through testing instances
		for (dInstance = 1; (dInstance <= dCount) && (dTestStatus == E__OK); dInstance++)
		{
			//check if XMC card present
			//------------------------------------------
	
			sDevice.dInstance = dInstance;
			dTestStatus = dCheckDevicePresent(&sDevice);
			if (dTestStatus == E__OK)
			{
				UINT8 bMaxPorts;
				
				// See how many ports to test. By default limit testing to
				// the first 3 ports (front panel ports).
				bMaxPorts = 3;
				
				// if there is an additional parameter then test all ports
				if (adTestParams[0] > 1)
					bMaxPorts = sDevice.bMaxPorts;
				
				// Lets do some loop-back tests on the ports
				for (bPort = 0; bPort < bMaxPorts; bPort++)
				{
					dTestStatus = dPortLoopbackTest (&sDevice, bPort);
					if (dTestStatus != E__OK)
					{
 						dTestStatus |= ((dInstance << 16) | (bPort << 8));
 						break;
					}
				}
			}
			else
			{
				if (dTestStatus == E__DEVICE_NOT_FOUND)
				{
					char achText[60];
					sprintf (achText, "XM RS1 instance %d not found\n", dInstance);
					vConsoleWrite(achText);
				}
			}
		}
	}
	else
	{
		vConsoleWrite ("No XM RS1 count specified\n");
		dTestStatus = E__DEVICE_NOT_FOUND; 
	}

	return (dTestStatus);
}


/******************************************************************************
 * This test takes one parameter :-
 * 	1 - The number of XM RS1 modules to test. Each module found will be tested
 * 		until all the modules have been tested or an error occurs.
 *
 */
TEST_INTERFACE (XMRS1IntTest, "XM RS1 Interrupt Test")
{
	UINT32	dTestStatus = E__OK;
	UINT32	dInstance, dCount;
	UINT8	bPort;
	DEVICE	sDevice;

#if  defined(VME)
	VME_TEST_PARAMS sVmeTestParams;
	EXPECTED_TYPES*	psExpectedTypes;
//	UINT16  wIdxStatus;

    if(sysGetTestHandler() == TH__STH)
    {
		adTestParams[0] = 0;

		psExpectedTypes = (EXPECTED_TYPES*)&abInterruptTypes;
		memset((void*)&sVmeTestParams,0,sizeof(VME_TEST_PARAMS));

    	/*wIdxStatus =*/ wIDX_GetParams (psExpectedTypes, (UINT8*)&sVmeTestParams);

//        if( wIdxStatus != E__OK )
//        {
//            return E__INVALID_PARAM;
//        }

        if (sVmeTestParams.dCount != 0)
        {
        	adTestParams[0] = 1;
        	adTestParams[1] = sVmeTestParams.dCount;
        }
    }
#endif

	// check test parameters
	if (adTestParams[0])
	{
		dCount = adTestParams[1];

		// Loop through testing instances
		for (dInstance = 1; (dInstance <= dCount) && (dTestStatus == E__OK); dInstance++)
		{

			//check if XMC card present
			//------------------------------------------
	
			sDevice.dInstance = dInstance;
			dTestStatus = dCheckDevicePresent(&sDevice);
			if (dTestStatus == E__OK)
			{
				// Lets do some loop-back tests on the ports
				for (bPort = 0; bPort < sDevice.bMaxPorts; bPort++)
				{
					dTestStatus = dPortInterruptTest (&sDevice, bPort);
					if (dTestStatus != E__OK)
					{
 						dTestStatus |= ((dInstance << 16) | (bPort << 8));
						break;
					}
				}
			}
			else
			{
				if (dTestStatus == E__DEVICE_NOT_FOUND)
				{
					char achText[60];
					sprintf (achText, "XM RS1 instance %d not found\n", dInstance);
					vConsoleWrite(achText);
				}
			}
		}
	}
	else
	{
		vConsoleWrite ("No XM RS1 count specified\n");
		dTestStatus = E__DEVICE_NOT_FOUND; 
	}

	return (dTestStatus);
}
