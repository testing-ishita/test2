
/************************************************************************
 *                                                                      *
 *      Copyright 2014 Concurrent Technologies, all rights reserved.    *
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

/* xmRs2Test.c - XMRS2, multiple serial port test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/xmRs2Test.c,v 1.2 2014-12-16 12:31:12 mgostling Exp $
 * $Log: xmRs2Test.c,v $
 * Revision 1.2  2014-12-16 12:31:12  mgostling
 * Fixed compiler warning when building BIT.
 *
 * Revision 1.1  2014-11-24 16:06:24  mgostling
 * XM RS2 Test functions.
 *
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

/* defines */

//#define DEBUG

#define E__MEM_ALLOC				(E__BIT | E__CUTE)
#define E__INVALID_PARAM            (E__BIT + 0x2)
#define E__TEST_RX_TIMEOUT			(E__BIT + 0x10)
#define E__TEST_DATA_MISMATCH		(E__BIT + 0x20)
#define E__TEST_RX_NOTEMPTY_TIMEOUT	(E__BIT + 0x30)
#define E__TEST_TX_EMPTY_TIMEOUT	(E__BIT + 0x40)
#define E__TEST_CTS					(E__BIT + 0x50)
#define	E__TEST_DSR					(E__BIT + 0x51)
#define	E__TEST_DCD					(E__BIT + 0x52)
#define	E__TEST_RI					(E__BIT + 0x53)
#define E__INT_ERROR				(E__BIT + 0x60)

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

#define TEST_232					1
#define TEST_422_485				2
#define TEST_RC_VARIANT				3
#define TEST_INTERNAL				4

#define EXAR_VID					0x13A8
#define XR17V358					0x0358

#define MAX_XM_RS2_PORTS			8

/* typedefs */

typedef struct {
	UINT16	wBaseAddr;
} PORTINFO;

typedef struct {
	PCI_PFA	   pfa;
	PTR48      tPtr1;
	UINT32     mDevHandle;
	UINT8     *pbDevRegs;
	UINT32     bMaxPorts;
	PORTINFO   asPortInfo[MAX_XM_RS2_PORTS];
} DEVICE;


/* locals */

static UINT8 bRtsLoop = CTS_MASK;
static UINT8 bDtrLoop = DSR_MASK | DCD_MASK;
static UINT8 bIntFlag = 0;
static UINT8 bVector;
static UINT8 bBaudRates [5] = {BAUD_9600, BAUD_19200, BAUD_38400, BAUD_57600, BAUD_115000};

#ifdef DEBUG
static UINT32 dBaud[5] = {9600, 19200, 38400, 57600, 115200};
#endif

/* globals */

/* externals */

extern UINT8 sysGetTestHandler (void);

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
	char	achText[80];
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

	pDevice->mDevHandle = dGetPhysPtr(dDeviceAddr, 0x4000, &pDevice->tPtr1, (void*) &dMapRegSpace);
	pDevice->pbDevRegs = (UINT8 *)dMapRegSpace;

	if (pDevice->mDevHandle == E__FAIL)
	{
#ifdef DEBUG
		vConsoleWrite ("Can not Allocate Mem Mapped Reg Space for XMRS2\n");
#endif
		return E__MEM_ALLOC;
	}

	/* set the maximum latency */
//	PCI_WRITE_BYTE (pDevice->pfa, PCI_LATENCY, 0x10);

	/* enable MEM, Bus Mastering and SERR */
	wTemp  = PCI_READ_WORD (pDevice->pfa, PCI_COMMAND);
	wTemp &= ~0x0147;
	wTemp |=  0x0106;
	PCI_WRITE_WORD (pDevice->pfa, PCI_COMMAND, wTemp);

	return E__OK;
}


/******************************************************************************
 * dFindXmRs2 - Find instance of a XMRS2 module.
 * This function performs a scan of the PCI bus to find an instance of
 * a XMRS2 XMC module.
 * RETURNS: E__OK if found otherwise E__DEVICE_NOT_FOUND
 */
static UINT32 dFindXmRs2 (PCI_PFA *pPfa, UINT16 *wDevId, UINT8 bInstance)
{
	UINT8	bBus, bDev, bMaxBus;
	UINT16	wClass;
	UINT32	wDid;
	UINT16	wVid;
	UINT8	bCnt;
#ifdef DEBUG
	char    achText[100];
#endif

	bMaxBus = 1;
	bCnt = 0;		// number of boards found

	// scan all busses, devices and assume function 0
	for (bBus = 0; bBus <= bMaxBus; bBus++)
	{
		for (bDev = 0; bDev < 32; bDev++)
		{
			wVid = PCI_READ_WORD (PCI_MAKE_PFA(bBus, bDev, 0), PCI_VENDOR_ID);
			if (wVid != 0xffff)
			{
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

			if (wVid == EXAR_VID)
			{
				wDid = PCI_READ_WORD (PCI_MAKE_PFA(bBus, bDev, 0), PCI_DEVICE_ID);
				if (wDid == XR17V358)
				{
					++bCnt;		// found one!
					if (bCnt == bInstance)
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
static UINT32 dCheckDevicePresent (DEVICE *pDevice, UINT8 bInstance)
{
	UINT32	dStatus;
	UINT16 wDid;
	UINT8	bPort;
#ifdef DEBUG
	char   achText[60];
#endif

	dStatus = dFindXmRs2 (&pDevice->pfa, &wDid, bInstance);
	if (dStatus == E__OK)
	{
		vInitPciDevice (pDevice);

#ifdef DEBUG
		sprintf (achText ,"Found XMRS2 at Bus:%d Dev:%d Func:%d\n",
				PCI_PFA_BUS(pDevice->pfa), PCI_PFA_DEV(pDevice->pfa), PCI_PFA_FUNC(pDevice->pfa));
		vConsoleWrite(achText);
#endif

		// Initialise the serial port base address information

		for (bPort = 0; bPort < pDevice->bMaxPorts; ++bPort)
		{
			pDevice->asPortInfo[bPort].wBaseAddr = 0x0000 + (0x400 * bPort);
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
	UINT16 wBaseAddr;

	wBaseAddr = pDevice->asPortInfo[bPort].wBaseAddr;

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
 * dPortModemSignalTest: function for testing the external loop-back
 * 						 of the modem signals of the serial port
 *
 * RETURNS: Test Status
 */

static UINT32 dPortModemSignalTest
(
	DEVICE *pDevice,
	UINT8 bPort
)
{
	UINT32	dTestStatus = E__OK;
	UINT8	bRegVal;
	UINT16	wBaseAddr;
#ifdef CUTE
	char	achText[100];
	char	*psTestText = "Loopback test";
#endif

	wBaseAddr = pDevice->asPortInfo[bPort].wBaseAddr;

	bRegVal = bReadReg8(pDevice, wBaseAddr + MCR) | RTSC_MASK | MOD_LOOP;
	vDelay(1);
	vWriteReg8 (pDevice, wBaseAddr + MCR, bRegVal);

	vDelay(10);
	if (bRtsLoop == CTS_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & CTS_MASK) == 0x00)
		{
			dTestStatus = E__TEST_CTS;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d CTS MASK 1 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bRtsLoop == DSR_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & DSR_MASK) == 0x00)
		{
			dTestStatus = E__TEST_DSR;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d DSR MASK 1 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bRtsLoop == DCD_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & DCD_MASK) == 0x00)
		{
			dTestStatus = E__TEST_DCD;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d DCD MASK 1 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bRtsLoop == RI_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & RI_MASK) == 0x00)
		{
			dTestStatus = E__TEST_RI;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d RI MASK 1 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}

	bRegVal = bReadReg8(pDevice, wBaseAddr + MCR) & (~RTSC_MASK);
	vDelay(1);
	vWriteReg8 (pDevice, wBaseAddr + MCR, bRegVal);
	vDelay(10);
	if (bRtsLoop == CTS_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & CTS_MASK) == CTS_MASK)
		{
			dTestStatus = E__TEST_CTS;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d CTS MASK 2 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bRtsLoop == DSR_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & DSR_MASK) == DSR_MASK)
		{
			dTestStatus = E__TEST_DSR;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d DSR MASK 2 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bRtsLoop == DCD_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & DCD_MASK) == DCD_MASK)
		{
			dTestStatus = E__TEST_DCD;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d DCD MASK 2 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bRtsLoop == RI_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & RI_MASK) == RI_MASK)
		{
			dTestStatus = E__TEST_RI;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d RI MASK 2 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}

	bRegVal = bReadReg8(pDevice, wBaseAddr + MCR) | DTRC_MASK;
	vDelay(1);
	vWriteReg8 (pDevice, wBaseAddr + MCR, bRegVal);
	vDelay(10);
	if (bDtrLoop == CTS_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & CTS_MASK) == 0x00)
		{
			dTestStatus = E__TEST_CTS;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d CTS MASK 3 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bDtrLoop == DSR_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & DSR_MASK) == 0x00)
		{
			dTestStatus = E__TEST_DSR;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d DSR MASK 3 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bDtrLoop == DCD_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & DCD_MASK) == 0x00)
		{
			dTestStatus = E__TEST_DCD;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d DCD MASK 3 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bDtrLoop == RI_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & RI_MASK) == 0x00)
		{
			dTestStatus = E__TEST_RI;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d RI MASK 3 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}

	bRegVal = bReadReg8(pDevice, wBaseAddr + MCR) & ~DTRC_MASK;
	vDelay(1);
	vWriteReg8 (pDevice, wBaseAddr + MCR, bRegVal);
	vDelay(10);
	if (bDtrLoop == CTS_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & CTS_MASK) == CTS_MASK)
		{
			dTestStatus = E__TEST_CTS;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d CTS MASK 4 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bDtrLoop == DSR_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & DSR_MASK) == DSR_MASK)
		{
			dTestStatus = E__TEST_DSR;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d DSR MASK 4 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bDtrLoop == DCD_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & DCD_MASK) == DCD_MASK)
		{
			dTestStatus = E__TEST_DCD;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d DCD MASK 4 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}
	if (bDtrLoop == RI_MASK)
	{
		if ((bReadReg8(pDevice, wBaseAddr + MSR) & RI_MASK) == RI_MASK)
		{
			dTestStatus = E__TEST_RI;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d RI MASK 4 \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}
	}

	return dTestStatus;

dtest_error:
	dTestStatus |= ((((UINT32) bPort) + 1) << 8);

#ifdef CUTE
	sprintf(achText ,"%s: Port %d FAIL %08x\n", psTestText, bPort + 1, dTestStatus);
	vConsoleWrite(achText);
#endif

	return (dTestStatus);
}	// dPortModemSignalTest()

/*****************************************************************************
 * dPortLoopbackTest: function for testing the external loop-back
 * 			   and modem access of the serial port
 *
 * RETURNS: Test Status
 */

static UINT32 dPortLoopbackTest (DEVICE *pDevice, UINT8 bTestMode, UINT8 bPort)
{
	UINT32	dTestStatus = E__OK, i;
	UINT8	rx, tx;
	UINT16	wBaseAddr;
	UINT8	bSpeedIndex;
	char	achText[100];
#ifdef CUTE
	char	*psTestText = "Loopback test";
#endif

	sprintf(achText ,"Testing Port %d\n", bPort + 1);
	vConsoleWrite(achText);

	wBaseAddr = pDevice->asPortInfo[bPort].wBaseAddr;

	for (bSpeedIndex = 0; bSpeedIndex < sizeof(bBaudRates); ++bSpeedIndex)
	{
		// select baud rate for this test
		SerialInit (pDevice, bPort, bBaudRates[bSpeedIndex]);

#ifdef DEBUG
		sprintf(achText ,"%s: Port %d - baud rate = %ld \n", psTestText, bPort + 1, (long) dBaud[bSpeedIndex]);
		vConsoleWrite(achText);
#endif
		if (bTestMode == TEST_INTERNAL)
		{
			// enable internal loopback
			vWriteReg8 (pDevice, wBaseAddr + MCR, MCR_LOOP);
		}

		/* Flush Rx */
		i = 0;
		while (((bReadReg8(pDevice, wBaseAddr + LSR) & LSR_DR) != 0) && (i < TIMEOUT))
		{
			rx = bReadReg8(pDevice, wBaseAddr + RX);
		}

		if (i >= TIMEOUT)
		{
			dTestStatus = E__TEST_RX_NOTEMPTY_TIMEOUT;
#ifdef CUTE
			sprintf(achText ,"%s: Port %d RX NOT EMPTY TIMEOUT \n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
			goto dtest_error;
		}

		// Write/read characters
		for (tx = 0; tx < 0xFF; ++tx)
		{
			i = 0;
			/* Wait for Tx Empty */
			while (((bReadReg8(pDevice, wBaseAddr + LSR) & LSR_DHRE) == 0x0) && (i < TIMEOUT))
			{
				++i;
			}

			if (i >= TIMEOUT)
			{
				dTestStatus = E__TEST_TX_EMPTY_TIMEOUT;
#ifdef CUTE
				sprintf(achText ,"%s: Port %d TX TIMEOUT \n", psTestText, bPort + 1);
				vConsoleWrite(achText);
#endif
				goto dtest_error;
			}

			/* Write the test character */
			vWriteReg8 (pDevice, wBaseAddr + TX, tx);

			/* Wait for Rx Data available */
			i = 0;
			while (((bReadReg8(pDevice, wBaseAddr + LSR) & LSR_DR) == 0x0) && (i < TIMEOUT))
			{
				++i;
			}

			if (i >= TIMEOUT)
			{
				dTestStatus = E__TEST_RX_TIMEOUT;
#ifdef CUTE
				sprintf (achText ,"%s: Port %d RX TIMEOUT \n", psTestText, bPort + 1);
				vConsoleWrite(achText);
#endif
				goto dtest_error;
			}

			/* read and check character */
			rx = bReadReg8(pDevice, wBaseAddr + RX);
			if (tx != rx)
			{
				dTestStatus = E__TEST_DATA_MISMATCH;
#ifdef CUTE
				sprintf(achText ,"%s: Port %d DATA MISMATCH TX 0x%02x RX 0x%02x\n", psTestText, bPort + 1, tx, rx);
				vConsoleWrite(achText);
#endif
				goto dtest_error;
			}
			else
			{
				dTestStatus = E__OK;
			}
		}

		/* Modem Signal Test */
		if ((dTestStatus == E__OK) &&
			((bTestMode == TEST_232) || (bTestMode == TEST_RC_VARIANT) ||
				((bTestMode == TEST_422_485) && (bPort >= 3))))
		{
			dTestStatus = dPortModemSignalTest(pDevice, bPort);
		}

		if (bTestMode == TEST_INTERNAL)
		{
			// disable internal loopback
			vWriteReg8 (pDevice, wBaseAddr + MCR, 0);
		}

		if (dTestStatus == E__OK)
		{
#ifdef DEBUG
			sprintf(achText ,"%s: Port %d PASS\n", psTestText, bPort + 1);
			vConsoleWrite(achText);
#endif
		}
	}

	return dTestStatus;

dtest_error:
	dTestStatus |= (((bPort + 1) << 8) + bSpeedIndex);
#ifdef CUTE
	sprintf(achText ,"%s: Port %d FAIL %08x\n", psTestText, bPort + 1, dTestStatus);
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
 * dPortInterruptTest - Test UART interrupt
 *
 * RETURNS: E__OK if test pass
 */
static UINT32 dPortInterruptTest (DEVICE *pDevice, UINT8 bPort)
{
	UINT16	wBaudDiv = BAUD_9600;
	UINT32	dStatus = E__OK;
	UINT16	wBaseAddr;
	UINT8 bReg;
#ifdef CUTE
	char	achText[100];
	char	*psTestText = "Interrupt Test";
#endif

	wBaseAddr = pDevice->asPortInfo[bPort].wBaseAddr;

	SerialInit (pDevice, bPort, wBaudDiv);

	// Enable UART loop-back
	vWriteReg8 (pDevice, wBaseAddr + MCR, MCR_LOOP);

	// Clear the interrupt flag
	bIntFlag = 0;

	//enable interrupt
	bVector = sysPinToVector (16, SYS_IOAPIC0);			// connected to INTA
	sysInstallUserHandler (bVector, vIntHandler);
	sysUnmaskPin (16, SYS_IOAPIC0);
	sysEnableInterrupts();

	vWriteReg8 (pDevice, wBaseAddr + IER, IER_RDA);

	// Write character, this will be received and generate a rx interrupt
	vWriteReg8 (pDevice, wBaseAddr + TX, 0x55);

	vDelay(10);//wait interrupt

	//read interrupt
	if (bIntFlag == 0)
	{
		// No interrupt
#ifdef CUTE
		sprintf(achText ,"%s: Port %d Interrupt NOT detected\n", psTestText, bPort);
		vConsoleWrite(achText);
#endif
		dStatus = E__INT_ERROR;
	}
	else
	{
		// read interrupt status
		bReg = bReadReg8 (pDevice, wBaseAddr + IIR);

		if ((bReg & (IIR_IP | IIR_RDA)) != IIR_RDA)
		{
			// we have an interrupt, but not what we expected
#ifdef CUTE
			sprintf(achText ,"%s: Port %d Interrupt detected but status not set (IIR = 0x%02x)\n", psTestText, bPort, bReg);
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
	sysMaskPin (16, SYS_IOAPIC0) ;
	sysInstallUserHandler (bVector, 0) ;

	return dStatus;
}

/******************************************************************************
 * XMRS2FunctionalTest
 * 1) Test the TX and RX external loopback at different baud rates.
 * 2) Test the modem signals loopback.
 * 3) Confirm interrupt processing.
 */
UINT32 dXMRS2FunctionalTest(UINT8 bTestMode, UINT8 bMaxPorts, UINT8 bBoardInstance)
{
	UINT32	dTestStatus;
	UINT8	bPort;
	UINT8	bFirstPort;
	DEVICE	sDevice;

	if (bTestMode == TEST_RC_VARIANT)
	{
		// number of ports to test: 4 or 5 starting at port 3
		bFirstPort = 3;
		sDevice.bMaxPorts = bMaxPorts + bFirstPort;
	}
	else
	{
		// number of ports to test: 3, 7 or 8 starting at port 0
		bFirstPort = 0;
		sDevice.bMaxPorts = bMaxPorts;
	}

	//check if XMC card present
	//------------------------------------------
	dTestStatus = dCheckDevicePresent(&sDevice, bBoardInstance);
	if (dTestStatus == E__OK)
	{
		for (bPort = bFirstPort; bPort < sDevice.bMaxPorts; ++bPort)
		{
			dTestStatus = dPortLoopbackTest (&sDevice, bTestMode, bPort);
			if (dTestStatus != E__OK)
			{
				dTestStatus |= ((((UINT32) bPort) + 1) << 8);
				break;
			}

			if (dTestStatus == E__OK)
			{
				dTestStatus = dPortInterruptTest (&sDevice, bPort);
				if (dTestStatus != E__OK)
				{
					dTestStatus |= ((((UINT32) bPort) + 1) << 8);
					break;
				}
			}
		}
	}
	else
	{
		if (dTestStatus == E__DEVICE_NOT_FOUND)
		{
			vConsoleWrite("XMRS2 not found\n");
		}
	}

	return (dTestStatus);
}

/******************************************************************************
 * This test takes two parameters :-
 *	1 - The operational mode of the XMRS2. Used to determine which loopback tests to apply to each channel:
 *		RS232 (front & rear external)
 *		RS422/485 (front and rear external - no modem signals at front)
 *		RC Variant (rear external only)
 *		Internal - does not check any external signals
 *	2 - The number of XMRS2 channels to test. Each channel found will be tested
 *		until all the channels have been tested or an error occurs.
 */
TEST_INTERFACE (XMRS2TestBoard1, "XMRS2 Test (Board 1)")
{
	UINT32	dTestStatus;
#ifdef DEBUG
	char	achBuff[80];
#endif

	if(sysGetTestHandler() == TH__STH)
	{
		adTestParams[0] = 2;
		adTestParams[1] = TEST_INTERNAL;	// no external signals tested
		adTestParams[2] = 8;				// all ports
	}

#ifdef DEBUG
	sprintf (achBuff, "TestMode = %d, port count = %d\n", adTestParams[1], adTestParams[2]);
	vConsoleWrite (achBuff);
#endif

	// check test parameters
	if ((adTestParams[1] == TEST_232)		 ||
		(adTestParams[1] == TEST_422_485)	 ||
		(adTestParams[1] == TEST_RC_VARIANT) ||
		(adTestParams[1] == TEST_INTERNAL))
	{
		// valid test mode
		if ((adTestParams[2] == 3) ||	// front ports (not RC variants)
			(adTestParams[2] == 4) ||	// only rear PMC ports on RC variant
			(adTestParams[2] == 5) ||	// only rear XMC ports on RC variant
			(adTestParams[2] == 7) ||	// front and rear ports on PMC variants (not RC variants)
			(adTestParams[2] == 8))		// front and rear ports on XMC variants (not RC variants)
		{
			// valid port count
			dTestStatus = E__OK;
		}
		else
		{
			// indicate failure
			adTestParams[0] = 0;
		}
	}
	else
	{
		// indicate failure
		adTestParams[0] = 0;
	}


	if (dTestStatus == E__OK)
	{
		if ((adTestParams[1] == TEST_RC_VARIANT) && (adTestParams[2] == 3))
		{
			// channel count of three is for testing the front ports only
			// RC variant does not access the front ports!!!
			// force an error
			adTestParams[0] = 0;
		}
	}

	if (adTestParams[0] == 2)
	{
		// run the tests on board 1
		dTestStatus = dXMRS2FunctionalTest (adTestParams[1], adTestParams[2], 1);
	}
	else
	{
		vConsoleWrite ("Invalid test parameters\n");
		dTestStatus = E__INVALID_PARAM;
	}

	return (dTestStatus);
}

/******************************************************************************
 * This test takes two parameters :-
 * This test takes two parameters :-
 *	1 - The operational mode of the XMRS2. Used to determine which loopback tests to apply to each channel:
 *		RS232 (front & rear external)
 *		RS422/485 (front and rear external - no modem signals at front)
 *		RC Variant (rear external only)
 *		Internal - does not check any external signals
 *	2 - The number of XMRS2 channels to test. Each channel found will be tested
 *		until all the channels have been tested or an error occurs.
 */
TEST_INTERFACE (XMRS2TestBoard2, "XMRS2 Test (Board 2)")
{
	UINT32	dTestStatus;
#ifdef DEBUG
	char	achBuff[80];
#endif

	if(sysGetTestHandler() == TH__STH)
	{
		adTestParams[0] = 2;
		adTestParams[1] = TEST_INTERNAL;	// no external signals tested
		adTestParams[2] = 8;				// all ports
	}

#ifdef DEBUG
	sprintf (achBuff, "TestMode = %d, PortCount = %d\n", adTestParams[1], adTestParams[2]);
	vConsoleWrite (achBuff);
#endif

	// check test parameters
	if ((adTestParams[1] == TEST_232)		 ||
		(adTestParams[1] == TEST_422_485)	 ||
		(adTestParams[1] == TEST_RC_VARIANT) ||
		(adTestParams[1] == TEST_INTERNAL))
	{
		// valid test mode
		if ((adTestParams[2] == 3) ||	// front ports (not RC variants)
			(adTestParams[2] == 4) ||	// only rear PMC ports on RC variant
			(adTestParams[2] == 5) ||	// only rear XMC ports on RC variant
			(adTestParams[2] == 7) ||	// front and rear ports on PMC variants (not RC variants)
			(adTestParams[2] == 8))		// front and rear ports on XMC variants (not RC variants)
		{
			// valid port count
			dTestStatus = E__OK;
		}
		else
		{
			// indicate failure
			adTestParams[0] = 0;
		}
	}
	else
	{
		// indicate failure
		adTestParams[0] = 0;
	}


	if (dTestStatus == E__OK)
	{
		if ((adTestParams[1] == TEST_RC_VARIANT) && (adTestParams[2] == 3))
		{
			// channel count of three is for testing the front ports only
			// RC variant does not access the front ports!!!
			// force an error
			adTestParams[0] = 0;
		}
	}

	if (adTestParams[0] == 2)
	{
		// run the tests on board 2
		dTestStatus = dXMRS2FunctionalTest (adTestParams[1], adTestParams[2], 2);
	}
	else
	{
		vConsoleWrite ("Invalid test parameters\n");
		dTestStatus = E__INVALID_PARAM;
	}

	return (dTestStatus);
}
