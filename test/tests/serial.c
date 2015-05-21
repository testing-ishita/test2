
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

/* serial.c - BIT test for serial port 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/serial.c,v 1.8 2014-05-12 16:10:15 jthiru Exp $
 * $Log: serial.c,v $
 * Revision 1.8  2014-05-12 16:10:15  jthiru
 * Adding support for VX91x and fixes to existing bugs
 *
 * Revision 1.7  2014-03-19 14:47:13  cdobson
 * Fixed the occasional baud rate test failure.
 *
 * Revision 1.6  2014-03-18 14:22:34  cdobson
 * Fixed the baud rate test for the P17C9X7945 Quad UART.
 *
 * Revision 1.5  2014-03-14 11:39:59  cdobson
 * Added support for the P17C9X7945 Quad UART. Interrupt handling
 * modified to support both edge (legacy) and level (PCI) interrupts.
 *
 * Revision 1.4  2013-11-25 12:49:20  mgostling
 * Corrected a message typo.
 *
 * Revision 1.3  2013-10-08 07:13:39  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.2  2013/09/26 12:35:23  chippisley
 * Removed obsolete HAL
 *
 * Revision 1.1  2013/09/04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.27  2012/09/14 15:38:45  jthiru
 * CUTE Support for VX91X - C5043B1110, fixes to existing bugs
 *
 * Revision 1.26  2012/03/21 15:53:47  cdobson
 * Eliminate compiler warnings.
 *
 * Revision 1.25  2011/11/21 11:17:22  hmuneer
 * no message
 *
 * Revision 1.24  2011/11/14 17:34:09  hchalla
 * Updated for PP 531.
 *
 * Revision 1.23  2011/10/27 15:50:39  hmuneer
 * no message
 *
 * Revision 1.22  2011/08/26 15:51:11  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.21  2011/06/07 14:53:55  hchalla
 * Added serial irq generation support using IOAPIC0.
 *
 * Revision 1.20  2011/03/22 13:59:17  hchalla
 * Added support for VX813 RS 485/RS 422 external loopback.
 *
 * Revision 1.19  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.18  2010/10/25 15:38:11  jthiru
 * Added RS422, RS485 test cases
 *
 * Revision 1.17  2010/09/15 12:25:15  hchalla
 * Modified tests to  support TR 501 BIT/CUTE.
 *
 * Revision 1.16  2010/09/14 09:59:51  cdobson
 * Added code to wait for transmitter empty before running baud rate test.
 *
 * Revision 1.15  2010/09/10 10:07:09  cdobson
 * Added tests for Tx finished before performing tests.
 * Added small delays for harware to settle.
 * Stopped the register test from sending a break.
 *
 * Revision 1.14  2010/02/26 10:40:52  jthiru
 * Baud rate test case change
 *
 * Revision 1.13  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.12  2009/07/24 11:08:33  jthiru
 * Reverted ISR based interrupt test
 *
 * Revision 1.11  2009/06/18 09:12:00  jthiru
 * Interrupt test case - Implemented ISR based interrupt test
 *
 * Revision 1.10  2009/06/12 11:53:06  jthiru
 * Expanded baud rate limit for TPA40
 *
 * Revision 1.9  2009/06/11 10:37:44  jthiru
 * Expanded baud rate limit for 9600 for TPA40
 *
 * Revision 1.8  2009/06/09 15:01:01  jthiru
 * Modified Baud rate limits
 *
 * Revision 1.7  2009/06/03 09:34:21  cvsuser
 * Interrupt test check now changed to THRE
 *
 * Revision 1.6  2009/06/03 08:30:37  cvsuser
 * Tidy up test names, Interrupt check changed to THRE
 *
 * Revision 1.5  2009/05/29 10:36:59  cvsuser
 * Added FIFO reset to serial baud rate test
 *
 * Revision 1.4  2009/05/22 15:32:45  cvsuser
 * Added support for 0 - all port test option, changes to baud limits
 *
 * Revision 1.3  2009/05/21 08:14:40  cvsuser
 * Added support to test the COM port used by BIT
 *
 * Revision 1.2  2009/05/15 11:05:57  jthiru
 * Enhancements to serial tests
 *
 * Revision 1.1  2009/02/02 17:18:27  jthiru
 * Add Serial BIT test code
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
#include <devices/scc16550.h>
#include <bit/io.h>
#include <bit/hal.h>
#include <bit/interrupt.h>
 
#include <bit/board_service.h>
#include <bit/board_service.h>

#include <private/sys_delay.h>
#include <private/port_io.h>


/* defines */
/* Error Codes at offset of 6, one for each instance */

#define E__TEST_PORTS				E__BIT
#define E__TEST_REGACCESS			E__BIT + 0x6
#define E__TEST_INTERRUPT_TIMEOUT	E__BIT + 0xc
#define E__TEST_TX_TIMEOUT			E__BIT + 0x12
#define E__TEST_RX_TIMEOUT			E__BIT + 0x18
#define E__TEST_DATA_MISMATCH		E__BIT + 0x1E
#define E__TEST_TX_EMPTY_TIMEOUT	E__BIT + 0x24
#define E__TEST_CTS					E__BIT + 0x2A
#define	E__TEST_DSR					E__BIT + 0x30
#define	E__TEST_DCD					E__BIT + 0x36
#define	E__TEST_RI					E__BIT + 0x3c
#define E__BAUD_9600				E__BIT + 0x42
#define E__BAUD_19200				E__BIT + 0x48
#define E__BAUD_38400				E__BIT + 0x4E
#define E__BAUD_56000				E__BIT + 0x54
#define E__BAUD_115200				E__BIT + 0x5A
#define E__LB_TXRDY_TIMEOUT			E__BIT + 0x60
#define E__LB_RX_TIMEOUT			E__BIT + 0x66
#define E__TEST_DTR					E__BIT + 0x6C
#define E__NO_PORTS_TO_TEST			E__BIT + 0x72
#define E__PIN_MAP			       (E__BIT + 0x73)
#define E__UNMASK_SLOT		       (E__BIT + 0x74)
#define E__MASK_SLOT		       (E__BIT + 0x75)

#define BANK2						LCR_DLAB
#define TIMEOUT           			0x0100000
#define TIMEOUT_1S					0x1000
#define TIMED_OUT					0
#define MOD_LOOP					0x10

//#define DEBUG 
/* Do not use this flag - deprecated */

/* typedefs */

/* constants */

/* locals */
static UINT8 rts_loop = MSR_CTS;
static UINT8 dtr_loop = MSR_DSR | MSR_CD;
static SCC_PARAMS serialportarr[32];
static SCC_PARAMS rs485arr[32];
static int isrCalled;
static UINT16 wBaseAddr;

#ifdef DEBUG
	static char buffer[128];
#endif

/* globals */
SERIALBAUD_INFO *pSerialBaudInfo;
/* externals */
extern void vDelay(UINT32 dMsDelay);

/* forward declarations */
static UINT32 dInterconnectTest(UINT32);
static UINT32 dInterruptTest(UINT32);
static UINT32 dBaudRateTest(UINT32);
static UINT32 dLocalloopback(UINT32);
static UINT32 dExternalLoopback(UINT32);
static UINT32 dWaitTxEmpty (UINT16 wBaseAddr);
static void SerialInit(UINT16 wBaseAddr, UINT16 wBaudDiv);
/*static void SerialTxChar(UINT16	wBaseAddr, UINT8 bData);*/
extern void vDelay(UINT32 dMsDelay);
extern void vConsoleWrite(char*	achMessage);
static void SerialExit(UINT32);
void serialIntHandler (void);


/*****************************************************************************
 * dWaitTxEmpty: Wait until transmit buffer is empty or timed out
 *
 * RETURNS: E__OK or E__... error code
 */

UINT32 dWaitTxEmpty (UINT16 wBaseAddr)
{
	int i = 100;  /* 100 mSec delay */

	while((((UINT8)(dIoReadReg(wBaseAddr + LSR, REG_8)) & LSR_THRE) == 0x0) && (i > 0) ) {
		vDelay(1);
		i--;
	}

	if(i == 0)
	{
		return E__TEST_TX_TIMEOUT;
	}

	return E__OK;
}


/*****************************************************************************
 * vFlushRxBuffer: Read Rx data port until no more data
 *
 * RETURNS: Nothing
 */

void vFlushRxBuffer (UINT16 wBaseAddr)
{
	int k = 0;

	while(((UINT8)(dIoReadReg(wBaseAddr + LSR, REG_8)) & LSR_DR) && (k++ < 255))
	{
		dIoReadReg(wBaseAddr + RX, REG_8);
	}
}


/*****************************************************************************
 * SerialInit: initialize a serial port for 8 data, no parity, 1 stop
 *
 * RETURNS: None
 */

void SerialInit 
(
	UINT16	wBaseAddr,		/* port base address */
	UINT16	wBaudDiv		/* baud rate divisor */
)
{
	/* Program baud rate divisor */

	vIoWriteReg (wBaseAddr + LCR, REG_8, LCR_DLAB);					/* DLAB = 1 */
	vIoWriteReg (wBaseAddr + DLL, REG_8, (UINT8)(wBaudDiv & 0x00FF));
	vIoWriteReg (wBaseAddr + DLH, REG_8, (UINT8)(wBaudDiv >> 8));
	vDelay (30);

	/* Select 8N1 format */

	vIoWriteReg (wBaseAddr + LCR, REG_8, LCR_DATA8 | LCR_NONE | LCR_STOP1);

	/* Disable interrupts */

	vIoWriteReg (wBaseAddr + IER, REG_8, 0);

	/* Enable and reset FIFOs */
	/* The P17C9X7945 seems to need the FIFOs to be reset twice,
	 * otherwise the LSR_DR bit is always set
	 */

	vIoWriteReg (wBaseAddr + FCR, REG_8, FCR_FEN | FCR_CRF | FCR_CTF);
	vDelay (50);
	vIoWriteReg (wBaseAddr + FCR, REG_8, FCR_FEN | FCR_CRF | FCR_CTF);
	vDelay (50);

} /* SerialInit () */



/*****************************************************************************
 * drs485Loopback: function for testing the RS485 ports
 *
 * RETURNS: Test Status
 */

UINT32 drs485Loopback
(
	UINT32 dnop
)
{
	UINT32	dTestStatus;	/* error reporting */
	UINT16	wTxMcrAddress;	/* address of sender's modem control reg. */
	UINT16	wRxMcrAddress;	/* address of receiver's modem control reg. */
	UINT16	wTxLsrAddress;	/* address of sender's line status reg. */
	UINT16	wRxLsrAddress;	/* address of receiver's line status reg. */
	UINT16	wTxDataAddress;	/* address of sender's data register */
	UINT16	wRxDataAddress;	/* address of receiver's data register */
	UINT16	wPattern;		/* transmitted data */
	UINT8	bChannel;		/* channel index */
	UINT8	bTxData;		/* transmitting channel data */
	UINT8	bRxData;		/* received data */
	UINT8	bLsrData;		/* line status register data */
	UINT8   bMaster;
	UINT32  dTimeout;
	dTestStatus = E__OK;

#ifdef DEBUG
				vConsoleWrite ("Entering RS485 Loopback\n");
#endif
#ifdef DEBUG
				wRxMcrAddress = (rs485arr[0].wBaseAddr) + MCR;
				sprintf(buffer, "MCR: 0x%x Value: 0x%x\n", rs485arr[0].wBaseAddr, dIoReadReg (wRxMcrAddress, REG_8));
				vConsoleWrite (buffer);
				wRxMcrAddress = (rs485arr[1].wBaseAddr) + MCR;
				sprintf(buffer, "MCR: 0x%x Value: 0x%x\n", rs485arr[1].wBaseAddr, dIoReadReg (wRxMcrAddress, REG_8));
				vConsoleWrite (buffer);
#endif
	for (bMaster = 0; bMaster < dnop; bMaster++)
	{
		/* Set all channels to slave first (i.e. transmitter not driving) */
		
		for (bChannel = 0; bChannel < dnop; bChannel++)
		{
			SerialInit(rs485arr[bChannel].wBaseAddr, BAUD_9600);
			wRxMcrAddress = (rs485arr[bChannel].wBaseAddr) + MCR;
			bRxData = (UINT8)dIoReadReg (wRxMcrAddress, REG_8);
			vIoWriteReg (wRxMcrAddress, REG_8, (UINT32)(bRxData & ~MCR_DTR));
			
			/* Disable Receiver's Transmission */
			
			if (bChannel != bMaster)
			{
				vIoWriteReg(wRxMcrAddress, REG_8, 0);
				vIoWriteReg(wRxMcrAddress, REG_8, ((UINT8)dIoReadReg (wRxMcrAddress, REG_8) | MCR_RTS));
			}
		}
#ifdef DEBUG
				sprintf(buffer, "Set all channels to slave MCR: 0x%x\n", dIoReadReg (wRxMcrAddress, REG_8));
				vConsoleWrite (buffer);
#endif
		/* Enable transmitter for the master channel (DTR = 1) */

		wTxMcrAddress = (rs485arr[bMaster].wBaseAddr) + MCR;
		vIoWriteReg(wTxMcrAddress, REG_8, 0);
		bTxData = (UINT8)dIoReadReg (wTxMcrAddress, REG_8);
		vIoWriteReg (wTxMcrAddress, REG_8, (UINT32)(bTxData | MCR_DTR));
#ifdef DEBUG
				sprintf(buffer, "Enable transmitter for master MCR: 0x%x\n", dIoReadReg (wTxMcrAddress, REG_8));
				vConsoleWrite (buffer);
#endif
		/* Flush all the receivers */

		for (bChannel = 0; bChannel < dnop; bChannel++)
		{
			vFlushRxBuffer (rs485arr[bChannel].wBaseAddr);
		}

#ifdef DEBUG
				vConsoleWrite ("Flush all receivers\n");
#endif
		/* Write a 256-byte pattern to the master, check reception on all
		 * channels; master and slaves
		 */

		for (wPattern = 0; wPattern <= 0x00FF; wPattern++)
		{
			bTxData = (UINT8)wPattern;

			/* Master: check for TX empty, then send a character */

			wTxDataAddress = (rs485arr[bMaster].wBaseAddr) + THR;
			wTxLsrAddress  = (rs485arr[bMaster].wBaseAddr) + LSR;

			dTimeout = 0;
			bLsrData = 0;

			while ((dTimeout < TIMEOUT_LIMIT) && ((bLsrData & 0x40) == 0))
			{
				bLsrData = (UINT8)dIoReadReg (wTxLsrAddress, REG_8);
				dTimeout++;
			}

			if (dTimeout >= TIMEOUT_LIMIT)
			{
#ifdef DEBUG
				sprintf (buffer, "Timeout waiting for Tx Ready, COM%d, Tx char %02X",
					   bMaster, (UINT8)wPattern);
				vConsoleWrite (buffer);
#endif

				dTestStatus = E__LB_TXRDY_TIMEOUT;
				break;
			}

			vIoWriteReg (wTxDataAddress, REG_8, (UINT32)bTxData);	/* write the character */


			/* Master and Slaves: check the received character */

			for (bChannel = 0; bChannel < dnop; bChannel++)
			{
				wRxLsrAddress  = (rs485arr[bChannel].wBaseAddr) + LSR;
				wRxDataAddress = (rs485arr[bChannel].wBaseAddr) + RBR;

				dTimeout = 0;
				bLsrData = 0;

				while ((dTimeout < TIMEOUT_LIMIT) && ((bLsrData & LSR_DR) == 0))
				{
					bLsrData = (UINT8)dIoReadReg (wRxLsrAddress, REG_8);
					dTimeout++;
				}

				if (dTimeout >= TIMEOUT_LIMIT)
				{
#ifdef DEBUG
					sprintf (buffer, "Rx Timeout on COM%d (Tx on COM%d, char %02X)",
							bChannel, bMaster, (UINT8)wPattern);
					vConsoleWrite (buffer);
#endif
					dTestStatus = E__LB_RX_TIMEOUT;
					break;
				}

				/* Get Rx character and test against actual Tx character */

				bRxData = (UINT8)dIoReadReg (wRxDataAddress, REG_8);

				if (bRxData != bTxData)
				{
#ifdef DEBUG
						sprintf (buffer, "Mismatch: Sent %02X on COM%d, Received %02X on COM%d",
								bTxData, bMaster, bRxData, bChannel);
					vConsoleWrite (buffer);
#endif
					dTestStatus = E__TEST_DATA_MISMATCH;
					break;
				}
			}

			if (dTestStatus != E__OK)	/* break out of loop on failure */
				break;

		} /* for (wPattern) */


		/* Disable transmitter for the master channel */

		wTxMcrAddress = (rs485arr[bMaster].wBaseAddr) + MCR;
		bTxData = (UINT8)dIoReadReg (wTxMcrAddress, REG_8);
		vIoWriteReg (wTxMcrAddress, REG_8, bTxData & ~MCR_DTR);
	}
	return (dTestStatus);

}


/*****************************************************************************
 * dLoopbackTest: This function verifies the data path of the specified UART.
 * Both internal and external loopback paths are supported. For the internal
 * loopback path data is transferred within channel. For the external path data
 * is transferred between adjacent channels.
 *
 * RETURNS : E__OK or E__... error code
 */

UINT32 dLoopbackTest
(
	UINT8	bTxChan,	/* transmitting SCC channel*/
	UINT8	bRxChan		/* receiving SCC channel */
)
{
	UINT32	dTimeout;		/* timeout counter while polling */
	UINT32	dTestStatus;	/* error reporting */
	UINT16	wTxLsrAddress;	/* absolute address of sender's line status reg. */
	UINT16	wRxLsrAddress;	/* absolute address of receiver's line status reg. */
	UINT16	wTxDataAddress;	/* absolute address of sender's data register */
	UINT16	wRxDataAddress;	/* absolute address of receiver's data register */
	UINT8	bTxData;		/* transmitted data */
	UINT8	bRxData;		/* received data */
	UINT8	bLsrData;		/* line status register data */
	UINT8 wRxMcrAddress, wTxMcrAddress;
	dTestStatus = E__OK;


#ifdef DEBUG
			sprintf (buffer, "rs485arr[bTxChan].wBaseAddr:%#x, rs485arr[bRxChan].wBaseAddr:%#x\n",
					rs485arr[bTxChan].wBaseAddr, rs485arr[bRxChan].wBaseAddr);
			vConsoleWrite (buffer);
#endif

	wTxLsrAddress = rs485arr[bTxChan].wBaseAddr + LSR;
	wRxLsrAddress = rs485arr[bRxChan].wBaseAddr + LSR;

	wTxDataAddress = rs485arr[bTxChan].wBaseAddr + THR;
	wRxDataAddress = rs485arr[bRxChan].wBaseAddr + RBR;

	wTxMcrAddress = (rs485arr[bTxChan].wBaseAddr) + MCR;
	bRxData = (UINT8)dIoReadReg (wTxMcrAddress, REG_8);
	vIoWriteReg (wTxMcrAddress, REG_8, (UINT32)(bRxData & ~MCR_DTR));

	wRxMcrAddress = (rs485arr[bRxChan].wBaseAddr) + MCR;
	bRxData = (UINT8)dIoReadReg (wRxMcrAddress, REG_8);
	vIoWriteReg (wRxMcrAddress, REG_8, (UINT32)(bRxData & ~MCR_DTR));

	vIoWriteReg((rs485arr[bRxChan].wBaseAddr + MCR), REG_8, 00);
	vIoWriteReg((rs485arr[bRxChan].wBaseAddr + MCR), REG_8,
	(dIoReadReg(rs485arr[bRxChan].wBaseAddr + MCR, REG_8)| MCR_RTS));

	vIoWriteReg((rs485arr[bTxChan].wBaseAddr + MCR), REG_8, 00);
	vIoWriteReg((rs485arr[bTxChan].wBaseAddr + MCR), REG_8,
	(dIoReadReg(rs485arr[bTxChan].wBaseAddr + MCR, REG_8)| MCR_DTR));

   	/* Flush any pending Rx characters. */
   	
	vFlushRxBuffer(rs485arr[bTxChan].wBaseAddr);
	vFlushRxBuffer(rs485arr[bRxChan].wBaseAddr);

	/* For all characters from 0 to 0xFE transmit and check receive. */

	for (bTxData = 0; bTxData < 0xFF; bTxData++)
	{
		/* Wait for Tx empty. */

		dTimeout = 0;
		bLsrData = 0;

		while ((dTimeout < TIMEOUT_LIMIT) && ((bLsrData & LSR_DHRE) == 0))
		{
			bLsrData = (UINT8)dIoReadReg (wTxLsrAddress, REG_8);
			dTimeout++;
		}

		if (dTimeout >= TIMEOUT_LIMIT)
		{
#ifdef DEBUG
			sprintf (buffer, "Timeout waiting for Tx Ready, COM%d, Tx char %02X",
                   bTxChan, bTxData);
			vConsoleWrite (buffer);
#endif
			dTestStatus = E__LB_TXRDY_TIMEOUT;
			break;
		}


		/* Write out test character and wait for Rx received character */

		vIoWriteReg  (wTxDataAddress, REG_8, bTxData);

		dTimeout = 0;
		bLsrData = 0;

		while ((dTimeout < TIMEOUT_LIMIT) && ((bLsrData & LSR_DR) == 0))
		{
			bLsrData = (UINT8)dIoReadReg (wRxLsrAddress, REG_8);
			dTimeout++;
		}

		if (dTimeout >= TIMEOUT_LIMIT)
		{
#ifdef DEBUG
			sprintf (buffer, "Rx Timeout on COM%d (Tx on COM%d, char %02X)",
					bRxChan, bTxChan, bTxData);
			vConsoleWrite (buffer);
#endif
			dTestStatus = E__LB_RX_TIMEOUT;
			break;
		}


		/* Get Rx character and test against actual Tx character */

		bRxData = (UINT8)dIoReadReg (wRxDataAddress, REG_8);

		if (bRxData != bTxData)
		{
#ifdef DEBUG
			sprintf (buffer, "Mismatch: Sent %02X on COM%d, Received %02X on COM%d",
						bTxData, bTxChan, bRxData, bRxChan);
			vConsoleWrite (buffer);
#endif
			dTestStatus = E__TEST_DATA_MISMATCH;
			break;
		}

	} /* for () */
#ifdef DEBUG
			sprintf (buffer, "Loopback test complete: 0x%x\n", dTestStatus);
			vConsoleWrite (buffer);
#endif
	return (dTestStatus);

} /* wLoopbackTest () */


/*****************************************************************************
 * drs422Loopback: function for testing the RS422 ports
 *
 * RETURNS: Test Status
 */

UINT32 drs422Loopback
(
	UINT32 dnop
)
{
	UINT32 dTestStatus = E__OK;
	UINT8 tx, rx;

	for (tx = 0; tx < dnop; tx++)
	{
		for (rx = 0; rx < dnop; rx++)
		{
			if(tx != rx)
			{
				SerialInit(rs485arr[tx].wBaseAddr, BAUD_9600);
				SerialInit(rs485arr[rx].wBaseAddr, BAUD_9600);
				if (dTestStatus == E__OK)
					dTestStatus = dLoopbackTest (tx, rx);
			}
		}
	}

	return dTestStatus;
}


/*****************************************************************************
 * drs485ExtLoopback: function for testing the RS 485 RS422 ports
 *
 * RETURNS: Test Status
 */

UINT32 drs485ExtLoopback
(
	UINT32 dnop
)
{
	UINT32 dTestStatus = E__OK;
	UINT8 port;

		for (port = 0; port < dnop; port++)
		{
			SerialInit(rs485arr[port].wBaseAddr, BAUD_9600);
			if (dTestStatus == E__OK)
				dTestStatus = dLoopbackTest (port, port);
		}
	return dTestStatus;
}


/*****************************************************************************
 * dExternalLoopback: function for testing the external loopback 
 * 					   and modem access of the serial port
 *
 * RETURNS: Test Status
 */

UINT32 dExternalLoopback
(
	UINT32 dnop		/* Number of Ports */
)
{
	UINT16 wBaudDiv = BAUD_9600;
	UINT32 dtest_status, j, i;
	UINT8 rx, tx;
	UINT8 regval;

	for(j = 0; j < dnop; j++)
	{
		SerialInit(serialportarr[j].wBaseAddr, wBaudDiv);

		/* Flush Rx */

		vFlushRxBuffer (serialportarr[j].wBaseAddr);
		
		for(tx = 0; tx < 0xFF; tx++)
		{
			i = 0;

			/* Wait for Tx Empty */

			if (dWaitTxEmpty(serialportarr[j].wBaseAddr) != E__OK)
			{
				dtest_status = E__TEST_TX_EMPTY_TIMEOUT;
#ifdef DEBUG
   				vConsoleWrite("dExternalLoopback: TX TIMEOUT \n");
#endif
				break;
			}

			/* Write the test character */

			vIoWriteReg (serialportarr[j].wBaseAddr + TX, REG_8, tx);

			/* Wait for Rx Data available */

			i = 0;
			while((((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + LSR, REG_8)) & LSR_DR) == 0x0) && (i < TIMEOUT) )
				i++;

			if(i >= TIMEOUT)
			{	
				dtest_status = E__TEST_RX_TIMEOUT;	
#ifdef DEBUG
				vConsoleWrite("dExternalLoopback: RX TIMEOUT \n"); 
#endif
				break;
			}
			rx = (UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + RX, REG_8));
			if(tx != rx)
			{
				dtest_status = E__TEST_DATA_MISMATCH;	
#ifdef DEBUG
				vConsoleWrite("dExternalLoopback: Data mismatch\n");
#endif
				break;
			}
			else
			{
				dtest_status = E__OK;
			}	
		}

		/* Modem Signal Test */
		
		if(dtest_status == E__OK)
		{
			regval = ((dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8)) | MSR_DDSR | MOD_LOOP);
			vDelay(1);
			vIoWriteReg (serialportarr[j].wBaseAddr + MCR, REG_8, regval);
			/*for(i = 0; i < 10000; i++){}			*/
			vDelay(10);
			if(rts_loop == MSR_CTS)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_CTS) == 0x00)
				{
					dtest_status = E__TEST_CTS;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: CTS MASK 1 \n");
#endif
					break;
				}
			if(rts_loop == MSR_DSR)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_DSR) == 0x00)
				{
					dtest_status = E__TEST_DSR;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: DSR MASK 1 \n");
#endif
					break;
				}
			if(rts_loop == MSR_CD)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_CD) == 0x00)
				{
					dtest_status = E__TEST_DCD;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: DCD MASK 1 \n");
#endif
					break;
				}
			if(rts_loop == MSR_RI)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_RI) == 0x00)
				{
					dtest_status = E__TEST_RI;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: RI MASK 1 \n");
#endif
					break;
				}

			regval = ((dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8)) & (~MSR_DDSR));
			vDelay(1);
			vIoWriteReg (serialportarr[j].wBaseAddr + MCR, REG_8, regval);
			vDelay(10);
			if(rts_loop == MSR_CTS)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_CTS) == MSR_CTS)
				{
					dtest_status = E__TEST_CTS;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: CTS MASK 2 \n");
#endif
					break;
				}
			if(rts_loop == MSR_DSR)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_DSR) == MSR_DSR)
				{
					dtest_status = E__TEST_DSR;
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: DSR MASK 2 \n");
#endif
					break;
				}
			if(rts_loop == MSR_CD)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_CD) == MSR_CD)
				{
					dtest_status = E__TEST_DCD;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: DCD MASK 2 \n");
#endif
					break;
				}
			if(rts_loop == MSR_RI)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_RI) == MSR_RI)
				{
					dtest_status = E__TEST_RI;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: RI MASK 2 \n");
#endif
					break;
				}
			regval = ((dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8)) | MSR_DCTS);
			vDelay(1);
			vIoWriteReg (serialportarr[j].wBaseAddr + MCR, REG_8, regval);
			vDelay(10);			
			if(dtr_loop == MSR_CTS)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_CTS) == 0x00)
				{
					dtest_status = E__TEST_CTS;
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: CTS MASK 3 \n");
#endif
					break;
				}
			if(dtr_loop == MSR_DSR)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_DSR) == 0x00)
				{
					dtest_status = E__TEST_DSR;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: DSR MASK 3 \n");
#endif
					break;
				}
			if(dtr_loop == MSR_CD)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_CD) == 0x00)
				{
					dtest_status = E__TEST_DCD;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: DCD MASK 3 \n");
#endif
					break;
				}
			if(dtr_loop == MSR_RI)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_RI) == 0x00)
				{
					dtest_status = E__TEST_RI;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: RI MASK 3 \n");
#endif
					break;
				}
			regval = ((dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8)) & ~MSR_DCTS);
			vDelay(1);
			vIoWriteReg (serialportarr[j].wBaseAddr + MCR, REG_8, regval);
			vDelay(10);			
			if(dtr_loop == MSR_CTS)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_CTS) == MSR_CTS)
				{
					dtest_status = E__TEST_CTS;
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: CTS MASK 4 \n");
#endif
					break;
				}
			if(dtr_loop == MSR_DSR)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_DSR) == MSR_DSR)
				{
					dtest_status = E__TEST_DSR;
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: DSR MASK 4 \n");
#endif
					break;
				}
			if(dtr_loop == MSR_CD)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_CD) == MSR_CD)
				{
					dtest_status = E__TEST_DCD;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: DCD MASK 4 \n");
#endif
					break;
				}
			if(dtr_loop == MSR_RI)
				if( ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MSR, REG_8)) & MSR_RI) == MSR_RI)
				{
					dtest_status = E__TEST_RI;	
#ifdef DEBUG
					vConsoleWrite("dExternalLoopback: RI MASK 4 \n");
#endif
					break;
				}
		}
		
		
		if(dtest_status == E__OK)
		{
#ifdef DEBUG
			vConsoleWrite("dExternalLoopback: PASS\n"); 
#endif
		}
		else
		{
#ifdef DEBUG
			vConsoleWrite("dExternalLoopback: FAIL\n"); 
#endif
		break;
		}
	}

	if(dtest_status != E__OK)
		return(dtest_status + serialportarr[j].bPort - 1);
	else
		return dtest_status;
} /* dExternalLoopback */


/*****************************************************************************
 * dLocalloopback: function for testing the local loopback functionality 
 *				   of the serial port
 *
 * RETURNS: Test Status
 */

UINT32 dLocalloopback
(
	UINT32 dnop		/* Number of Ports */
)
{
	UINT16 wBaudDiv = BAUD_9600;
	UINT32 dtest_status, j, i;
	UINT8 rx, tx;

	for(j = 0; j < dnop; j++)
	{
#ifdef DEBUG
		//CGD
		sprintf(buffer, "Testing port 0x%x\n", serialportarr[j].wBaseAddr);
		vConsoleWrite(buffer);
#endif
		/* make sure any output has finished */
		
		i = TIMEOUT_1S;
		
		/* Wait for Tx Empty */
		
		if (dWaitTxEmpty(serialportarr[j].wBaseAddr) != E__OK)
		{
			dtest_status = E__TEST_TX_TIMEOUT;
			break;
		}

		SerialInit(serialportarr[j].wBaseAddr, wBaudDiv);
		vIoWriteReg (serialportarr[j].wBaseAddr + MCR, REG_8, (dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8)) | MCR_LOOP );
		
		/* Flush Rx */

		vFlushRxBuffer (serialportarr[j].wBaseAddr);
		
		for(tx = 0; tx < 0xFF; tx++)
		{
			i = 0;

			/* Wait for Tx Empty */

			while((((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + LSR, REG_8)) & LSR_DHRE) == 0x0) && (i < TIMEOUT) )
				i++;
			if(i >= TIMEOUT)
			{	
				dtest_status = E__TEST_TX_TIMEOUT;
				break;
			}
			
			/* Write the test character */
			
			vIoWriteReg (serialportarr[j].wBaseAddr + TX, REG_8, tx);
			
			/* Wait for Rx Data available */
			
			i = 0;
			while((((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + LSR, REG_8)) & LSR_DR) == 0x0) && (i < TIMEOUT) )
				i++;

//			#ifdef DEBUG
//				sprintf(buffer, "port LSR: 0x%x\n", ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8))));
//				vConsoleWrite(buffer);
//			#endif
			
			if(i >= TIMEOUT)
			{	
				dtest_status = E__TEST_RX_TIMEOUT;
				break;
			}
			
			rx = (UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + RX, REG_8));
			if(tx != rx)
			{
				dtest_status = E__TEST_DATA_MISMATCH;
				break;
			}
			else
			{
				dtest_status = E__OK;
			}	
		}
		vIoWriteReg (serialportarr[j].wBaseAddr + MCR, REG_8, (dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8)) & 0xE7);
		
		if(dtest_status == E__OK)
		{
#ifdef DEBUG
			vConsoleWrite("dLocalloopback: PASS\n"); 
#endif
		}
		else
		{
#ifdef DEBUG
			vConsoleWrite("dLocalloopback: FAIL\n"); 
#endif
			break;
		}
		
	}
	
	if(dtest_status != E__OK)
		return(dtest_status + serialportarr[j].bPort - 1);
	else
		return dtest_status;
} /* dLocalloopback */


/*****************************************************************************
 * dBaudRateTest: function for testing baud rate of the serial port
 *
 * RETURNS: Test Status
 */

UINT32 dBaudRateTest
(
	UINT32 dnop /* Number of Ports */
)
{
	UINT32 i, j, k, dtest_status = E__OK;	
	UINT16 wBaudDiv;
	UINT32 baud, time = 1;
	UINT8 tx = 0xFF;
	SERIALBAUD_INFO SerialInfo;

	/* board_service(SERVICE__BRD_GET_SERIAL_BAUD_INFO, NULL, &pSerialBaudInfo); */

	for(j = 0; j < dnop; j++)
	{
	
		/* request info for the current port */
		
		SerialInfo.dBaudHigh = 0xAA55;
		SerialInfo.dBaudLow  = (UINT32)serialportarr[j].bPort;
		pSerialBaudInfo = &SerialInfo;

		board_service(SERVICE__BRD_GET_SERIAL_BAUD_INFO, NULL, &pSerialBaudInfo);

		/* make sure any output has finished */
		
		i = TIMEOUT_1S;
		
		/* Wait for Tx Empty */
		
		if (dWaitTxEmpty (serialportarr[j].wBaseAddr) != E__OK)
		{
			dtest_status = E__TEST_TX_TIMEOUT;
			break;
		}

		/*for(wBaudDiv = 12; wBaudDiv > 0; wBaudDiv--)*/
		/*for(wBaudDiv = 1; wBaudDiv < 6; wBaudDiv++) */
		for (k = 0; k < 5; k++)
		{
			UINT32 dBaudRate;
			
			wBaudDiv  = pSerialBaudInfo[k].wBaudDiv;
			dBaudRate = pSerialBaudInfo[k].dBaudRate;
			
			/*SerialInit(serialportarr[j].wBaseAddr, wBaudDiv);*/
			/* Program baud rate divisor */

			vIoWriteReg (serialportarr[j].wBaseAddr + LCR, REG_8, LCR_DLAB);					/* DLAB = 1 */
			vIoWriteReg (serialportarr[j].wBaseAddr + DLL, REG_8, (UINT8)(wBaudDiv & 0x00FF));
			vIoWriteReg (serialportarr[j].wBaseAddr + DLH, REG_8, (UINT8)(wBaudDiv >> 8));
			vDelay(10);

			/* Select 8N1 format */
			
			vIoWriteReg (serialportarr[j].wBaseAddr + LCR, REG_8, LCR_DATA8 | LCR_NONE | LCR_STOP1);

			/* Disable interrupts */
			
			vIoWriteReg (serialportarr[j].wBaseAddr + IER, REG_8, 0);

			/* The P17C9X7945 seems to need the FIFOs to be reset twice,
			 * otherwise the LSR_DR bit is always set
			 */

			do {
				vIoWriteReg (serialportarr[j].wBaseAddr + FCR, REG_8, FCR_FEN | FCR_CRF | FCR_CTF);
				vDelay (50);
				vIoWriteReg (serialportarr[j].wBaseAddr + FCR, REG_8, FCR_FEN | FCR_CRF | FCR_CTF);
				vDelay (50);
			} while ((dIoReadReg(serialportarr[j].wBaseAddr + LSR, REG_8) & LSR_DR) != 0x0);
			
			/* Flush Rx */
			
			vFlushRxBuffer (serialportarr[j].wBaseAddr);
			
			/* Set Loopback */
			
			vIoWriteReg (serialportarr[j].wBaseAddr + MCR, REG_8, (dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8)) | MCR_LOOP );
			
			/* Wait for Tx Empty */
			
			if (dWaitTxEmpty (serialportarr[j].wBaseAddr) != E__OK)
			{	
				dtest_status = E__TEST_TX_TIMEOUT;
				break;
			}
			
			/* Flush Rx */
			
			vFlushRxBuffer (serialportarr[j].wBaseAddr);
			
			vDelay(100);
			
			/* Transmit a byte of data */
			
			vIoWriteReg (serialportarr[j].wBaseAddr + TX, REG_8, tx);
			
			/* Poll till data arrives*/
			
			baud = 0;
			while((((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + LSR, REG_8)) & LSR_DR) == 0x0) && (i < TIMEOUT))
			{
				sysDelayMicroseconds( time );
				baud++;
			}
			
			/* 3.25 / 10 to be set in predivider*/

#ifdef DEBUG
			sprintf(buffer, "div:%d baud %d\n", wBaudDiv, baud);
			vConsoleWrite(buffer);
#endif

			if((baud < pSerialBaudInfo[k].dBaudHigh) && (baud > pSerialBaudInfo[k].dBaudLow))
				dtest_status = E__OK;
			else
			{				
				switch (dBaudRate)
				{
				case 9600:
					dtest_status = E__BAUD_9600;
					break;
				case 19200:
					dtest_status = E__BAUD_19200;
					break;

				case 38400:
					dtest_status = E__BAUD_38400;
					break;

				case 56000:
					dtest_status = E__BAUD_56000;
					break;

				case 115200:
					dtest_status = E__BAUD_115200;
					break;
				}
			
				vIoWriteReg(0x80, REG_16, baud);
				break;
			}

		}
		if(dtest_status != E__OK)
			break;
	}
	
	if(dtest_status != E__OK)
		return(dtest_status + serialportarr[j].bPort - 1);
	else
		return dtest_status;
	
} /* dBaudRateTest */


/*****************************************************************************
 * dInterruptTest: function for testing the interrupt generation of the 
 * 					  serial port
 *
 * RETURNS: Test Status
 */

UINT32 dInterruptTest
(
	UINT32 dnop		/* Number of Ports */
)
{
	//UINT16 wBaudDiv = BAUD_115000;
        UINT16 wBaudDiv = BAUD_9600;
	UINT32 dtest_status, j, i;
	int		iVector= -1;
  
#ifdef DEBUG
	char achBuffer[80];
#endif
	
	for(j = 0; j < dnop; j++)
	{
		/* make sure any output has finished */

		if (dWaitTxEmpty(serialportarr[j].wBaseAddr) == E__TEST_TX_TIMEOUT)
		{
			dtest_status = E__TEST_TX_TIMEOUT;
			vConsoleWrite("FAIL: TX TIMEOUT\n");
			break;
		}

		SerialInit(serialportarr[j].wBaseAddr, wBaudDiv);

		sysEnableInterrupts();

		/* Find vector used by system for chosen IOAPIC pin */
		
		iVector = sysPinToVector (serialportarr[j].bIrq, SYS_IOAPIC0);

		if (iVector == -1)
			return (E__PIN_MAP);

		/* clear the interrupt counter and initialise the port address for the */ 
		/* interrupt handler */

		isrCalled = 0;
		wBaseAddr = serialportarr[j].wBaseAddr;

		/* install the interrupt handler and unmask the interrupt */

		sysInstallUserHandler (iVector, serialIntHandler);
		if (sysUnmaskPin (serialportarr[j].bIrq, SYS_IOAPIC0) == -1) {
			sysInstallUserHandler (iVector, 0);
			return (E__UNMASK_SLOT);
		}

		/* enable the uart transmit buffer empty interrupt */

		vIoWriteReg (serialportarr[j].wBaseAddr + IER, REG_8, (dIoReadReg(serialportarr[j].wBaseAddr + IER, REG_8)) | IER_THRE | 0x03);
		vIoWriteReg (serialportarr[j].wBaseAddr + MCR, REG_8, (dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8)) | 0x08);

		i = 0;
                /* Wait for Tx Empty */
                while((((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + LSR, REG_8)) & LSR_DHRE) == 0x0) && (i < TIMEOUT) )
                i++;
                if(i >= TIMEOUT)
                {
                        dtest_status = E__TEST_TX_TIMEOUT;
                    break;
                }

                /* Write the test character */
                vIoWriteReg (serialportarr[j].wBaseAddr + TX, REG_8, 0x48);
                /* Wait for Rx Data available */
                i = 0;
                /* Wait for Tx Empty */
                while((((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + LSR, REG_8)) & LSR_THRE) == 0x0) && (i < TIMEOUT) )
                i++;
                if(i >= TIMEOUT)
                {
                        dtest_status = E__TEST_TX_TIMEOUT;
                    break;
                }

                vIoWriteReg (serialportarr[j].wBaseAddr + TX, REG_8, 0x48);

		/* we are sending 2 characters at 115200 baud. This should take 174 microseconds */

		vDelay (10);			/* wait 10mS (plenty of time for the interrupts) */

		/* Mask and disable interrupt and unnstall our interrupt handler */

		if (sysMaskPin (serialportarr[j].bIrq, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);


		sysInstallUserHandler (iVector, 0);

#ifdef DEBUG
		sprintf(buffer, "port LCR: 0x%x\n", ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + LCR, REG_8))));
		vConsoleWrite(buffer);
		sprintf(buffer, "port IER: 0x%x\n", ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + IER, REG_8))));
		vConsoleWrite(buffer);
		sprintf(buffer, "port MCR: 0x%x\n", ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + MCR, REG_8))));
		vConsoleWrite(buffer);
		sprintf(buffer, "port LSR: 0x%x\n", ((UINT8)(dIoReadReg(serialportarr[j].wBaseAddr + LSR, REG_8))));
		vConsoleWrite(buffer);
#endif
		
		/* Check we saw at least one interrupt (we should have 3) */
		
		if (isrCalled == 0)
		{
#ifdef DEBUG
			sprintf (achBuffer, "No interrupt received on pin:%d on %d IOAPIC\n", serialportarr[j].bIrq, SYS_IOAPIC0);
			vConsoleWrite(achBuffer);
#endif
			dtest_status = E__TEST_INTERRUPT_TIMEOUT;
		}
		else
		{
#ifdef DEBUG
			sprintf (achBuffer, "%d interrupts received on pin :%d on %d IOAPIC\n",isrCalled, serialportarr[j].bIrq, SYS_IOAPIC0);
			vConsoleWrite(achBuffer);
#endif
			dtest_status = E__OK;
		}

		/* read the interrupt cause */
		
		dIoReadReg(serialportarr[j].wBaseAddr + IIR, REG_8);

		/* Flush Rx in case there is a loopback connector (max 256 characters) */
		
		vFlushRxBuffer (serialportarr[j].wBaseAddr);
		
		/* Make sure interrupts are disabled */

		vIoWriteReg (serialportarr[j].wBaseAddr + IER, REG_8, (dIoReadReg(serialportarr[j].wBaseAddr + IER, REG_8)) & ~IER_THRE);
		
		/* clear the interrupt counter and port address */

		isrCalled = 0;
		wBaseAddr = 0;

		if(dtest_status != E__OK)
			break;
	}
    if(dtest_status != E__OK)
		return(dtest_status + serialportarr[j].bPort - 1);
	else
		return dtest_status;
} /* dInterruptTest */


/*****************************************************************************
 * dInterconnectTest: function for testing the access to 
 * 					  registers of the serial port
 *
 * RETURNS: Test Status
 */

UINT32 dInterconnectTest
(
	UINT32 dnop		/* Number of Ports */
)
{
	UINT32 dtest_status, dPort_Scan = 0;
	UINT16 wBaudDiv = BAUD_9600;

	/* Do not use index 0 */

	for(dPort_Scan = 0; dPort_Scan < dnop; dPort_Scan++)
	{
		/* make sure any output has finished */

		if (dWaitTxEmpty(serialportarr[dPort_Scan].wBaseAddr) == E__TEST_TX_TIMEOUT)
		{
			dtest_status = E__TEST_TX_TIMEOUT;
			break;
		}

		/* Program baud rate divisor */
		
		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + LCR, REG_8, BANK2);				/* DLAB = 1 */
		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + DLL, REG_8, (UINT8)(wBaudDiv & 0x00FF));
		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + DLH, REG_8, (UINT8)(wBaudDiv >> 8));
		vDelay(30);
		if(((UINT8)dIoReadReg(serialportarr[dPort_Scan].wBaseAddr + DLL, REG_8) == (UINT8)(wBaudDiv & 0x00FF)) \
			&& ((UINT8)(dIoReadReg(serialportarr[dPort_Scan].wBaseAddr + DLH, REG_8)) == (UINT8)(wBaudDiv >> 8)))
		{
#ifdef DEBUG
			vConsoleWrite("dInterconnectTest: PASS \n");
#endif
			dtest_status = E__OK;
		}
		else
		{
#ifdef DEBUG
			vConsoleWrite("dInterconnectTest: FAIL \n");
#endif
			dtest_status = E__TEST_REGACCESS;
		}
		if(dtest_status != E__OK)
			break;
	}
	
	if(dtest_status != E__OK)
		return(dtest_status + serialportarr[dPort_Scan].bPort - 1);
	else
		return dtest_status;
} /* dInterconnectTest */


/*****************************************************************************
 * dGetNops: function for returning the number of ports to test and
 *			 generate Serial port array
 *
 * RETURNS: Number of ports for test or invalid ports (E__TEST_PORTS)
 */

UINT32 dGetNops
(
	UINT32* adTestParams /* parameters passed by user */
)
{
	UINT32 dSerialPortMap, dUserPortMap = 0, dPortScan;
	UINT32 dTemp, dnop = 0;
    UINT32 		rt;

	/* BIT Map*/
	
    board_service(SERVICE__BRD_GET_SCC_PORT_MAP, &rt, &dSerialPortMap);

#ifdef DEBUG
	sprintf(buffer, "dSerialPortMap: %d\n", dSerialPortMap);
	vConsoleWrite (buffer);
#endif

	if(adTestParams[1])
	{
		for(dTemp = 1; dTemp < (adTestParams[0] + 1); dTemp++)
		{
			dUserPortMap |= (UINT32)1 << adTestParams[dTemp];
		}
		
		for(dPortScan = 0; dPortScan < 32; dPortScan++)
		{
			if(((dSerialPortMap >> dPortScan) & 0x01) == 0) 
				if(((dUserPortMap >> dPortScan) & 0x01) == 1)
					return(E__TEST_PORTS);	
				
			if(((dSerialPortMap & dUserPortMap) >> dPortScan) & 0x1)
			{
				serialportarr[dnop].bPort = dPortScan;
				if(board_service(SERVICE__BRD_GET_SCC_PARAMS, &rt, &serialportarr[dnop]) == E__OK)
				{
#ifdef DEBUG
					sprintf(buffer, "Base Addr of port %d: 0x%x\n", dPortScan, serialportarr[dnop].wBaseAddr);
					vConsoleWrite (buffer);
#endif
					dnop++;
				}
			}
		}
	}
	else
	{
		for(dPortScan = 0; dPortScan < 32; dPortScan++)
		{
			if((dSerialPortMap >> dPortScan) & 0x1)
			{
				serialportarr[dnop].bPort = dPortScan;
				if(board_service(SERVICE__BRD_GET_SCC_PARAMS, &rt, &serialportarr[dnop]) == E__OK)
				{
#ifdef DEBUG
					sprintf(buffer, "Base Addr of port %d: 0x%x\n", dPortScan, serialportarr[dnop].wBaseAddr);
					vConsoleWrite (buffer);
#endif
					dnop++;
				}
			}
		}
	}

#ifdef DEBUG
	sprintf(buffer, "Number of ports found: %d\n", dnop);
	vConsoleWrite (buffer);
#endif

	return(dnop);
}/* dGetNops */


/*****************************************************************************
 * dGetNopsrs485: function for returning the number of ports to test and
 *			 generate RS485 port array
 *
 * RETURNS: Number of ports for test
 */

UINT32 dGetNopsrs485()
{
	UINT32 drs485PortMap, dPortScan, dnop = 0;
	UINT16 port;
	static SCC_PARAMS temp;
	UINT32 rt;

	board_service(SERVICE__BRD_GET_SCC_PORT_MAP, &rt, &drs485PortMap);

	for(dPortScan = 0; dPortScan < 32; dPortScan++)
	{
		if((drs485PortMap >> dPortScan) & 0x1)
		{
			temp.bPort = dPortScan;
			if(board_service(SERVICE__BRD_GET_SCC_PARAMS, &rt, &temp) == E__OK)
			{
				board_service(SERVICE__BRD_GET_CONSOLE_PORT, &rt, &port);
				if((temp.wBaseAddr) != port)
				{
					rs485arr[dnop].bPort = dPortScan;
					if(board_service(SERVICE__BRD_GET_SCC_PARAMS, &rt, &rs485arr[dnop]) == E__OK)
					{
#ifdef DEBUG
						sprintf(buffer, "Base Addr of port %d: 0x%x\n", dnop, rs485arr[dnop].wBaseAddr);
						vConsoleWrite (buffer);
#endif
						dnop++;
					}
				}
			}
		}
	}
	return(dnop);
}

/*****************************************************************************
 * dGetNopsrs485: function for returning the number of ports to test and
 *			 generate RS485 port array
 *
 * RETURNS: Number of ports for test
 */

UINT32 dGetNumPortRS485()
{
	UINT32 drs485PortMap, dPortScan, dnop = 0;
//	UINT16 port;
//	static SCC_PARAMS temp;
	UINT32 rt;

	board_service(SERVICE__BRD_GET_RS485_PORT_MAP, &rt, &drs485PortMap);

	for(dPortScan = 0; dPortScan < 32; dPortScan++)
	{
		if((drs485PortMap >> dPortScan) & 0x1)
		{
			rs485arr[dnop].bPort = dPortScan;
			if(board_service(SERVICE__BRD_GET_SCC_PARAMS, &rt, &rs485arr[dnop]) == E__OK)
			{
#ifdef DEBUG
				sprintf(buffer, "Base Addr of port %d: 0x%x\n", dnop, rs485arr[dnop].wBaseAddr);
				vConsoleWrite (buffer);
#endif
				dnop++;
			}
		}
	}
	return(dnop);
}

/*****************************************************************************
 * SerialExit: Exit by setting the serial port for 8 data, no parity, 1 stop
 *
 * RETURNS: None
 */

void SerialExit 
(
	UINT32 dnop		/* Number of Ports */
)
{
	UINT16 wBaudDiv = BAUD_115000;
	UINT32 dPort_Scan = 0;

	for(dPort_Scan = 0; dPort_Scan < dnop; dPort_Scan++)
	{
		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + MCR, REG_8, (dIoReadReg(serialportarr[dPort_Scan].wBaseAddr + MCR, REG_8)) & ~MCR_LOOP & ~0x8);
		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + LCR, REG_8, LCR_DLAB);					/* DLAB = 1 */
		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + DLL, REG_8, (UINT8)(wBaudDiv & 0x00FF));
		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + DLH, REG_8, (UINT8)(wBaudDiv >> 8));
		vDelay (30);

		/* Select 8N1 format */

		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + LCR, REG_8, LCR_DATA8 | LCR_NONE | LCR_STOP1);

		/* Disable interrupts */

		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + IER, REG_8, 0);

		/* Enable and reset FIFOs */
		/* The P17C9X7945 seems to need the FIFOs to be reset twice,
		 * otherwise the LSR_DR bit is always set
		 */

		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + FCR, REG_8, FCR_FEN | FCR_CRF | FCR_CTF);
		vDelay (30);
		vIoWriteReg (serialportarr[dPort_Scan].wBaseAddr + FCR, REG_8, FCR_FEN | FCR_CRF | FCR_CTF);
		vDelay (30);
	}
	
}/* SerialExit */



/*****************************************************************************
 * SerialRS485Loopback: test function for RS485 Loopback
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (SerialRS485ExtLoopback, "Serial RS485/422 Ext LBack")
{
	UINT32 ports_selected, dtest_status;

	ports_selected = dGetNumPortRS485();

	if(ports_selected != E__TEST_PORTS)
	{
		if(ports_selected > 0)
		{
			board_service(SERVICE__BRD_ENABLE_RS485_FD, NULL, NULL);
			dtest_status = drs485ExtLoopback(ports_selected);
			board_service(SERVICE__BRD_DISABLE_RS485, NULL, NULL);
			SerialExit(ports_selected);
			/*if (dtest_status == E__OK)
			{
				board_service(SERVICE__BRD_ENABLE_RS485_HD, NULL, NULL);
				dtest_status = drs485ExtLoopback(ports_selected);
				board_service(SERVICE__BRD_DISABLE_RS485, NULL, NULL);
				SerialExit(ports_selected);
			}*/
		}
		else
			return E__NO_PORTS_TO_TEST;
	}
	else
		return(ports_selected);

	return(dtest_status);

} /* SerialRS485Loopback */


/*****************************************************************************
 * SerialRS485Loopback: test function for RS485 Loopback
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (SerialRS485Loopback, "Serial RS485 Loopback Test")
{
	UINT32 ports_selected, dtest_status;

	ports_selected = dGetNopsrs485();

	if(ports_selected != E__TEST_PORTS)
	{
		if(ports_selected > 0)
		{
			board_service(SERVICE__BRD_ENABLE_RS485_HD, NULL, NULL);
			dtest_status = drs485Loopback(ports_selected);
			board_service(SERVICE__BRD_DISABLE_RS485, NULL, NULL);
			SerialExit(ports_selected);
			if (dtest_status == E__OK)
			{
				board_service(SERVICE__BRD_ENABLE_RS485_FD, NULL, NULL);
				dtest_status = drs485Loopback(ports_selected);
				board_service(SERVICE__BRD_DISABLE_RS485, NULL, NULL);
				SerialExit(ports_selected);
			}
		}
		else
			return E__NO_PORTS_TO_TEST;
	}
	else
		return(ports_selected);

	return(dtest_status);

} /* SerialRS485Loopback */

/*****************************************************************************
 * SerialRS422Loopback: test function for RS422 Loopback
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (SerialRS422Loopback, "Serial RS422 Loopback Test")
{
	UINT32 ports_selected, dtest_status;

	ports_selected = dGetNopsrs485();

	if(ports_selected != E__TEST_PORTS)
	{
		if(ports_selected > 0)
		{
			board_service(SERVICE__BRD_ENABLE_RS485_FD, NULL, NULL);
			dtest_status = drs422Loopback(ports_selected);
			board_service(SERVICE__BRD_DISABLE_RS485, NULL, NULL);
			SerialExit(ports_selected);
		}
		else
			return E__NO_PORTS_TO_TEST;
	}
	else
		return(ports_selected);

	return(dtest_status);

} /* SerialRS422Loopback */

/*****************************************************************************
 * SerialExternalLoopback: test function for External loopback
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (SerialExternalLoopback, "Serial External Loopback Test")
{
	UINT32 ports_selected, dtest_status;
	
	ports_selected = dGetNops(adTestParams);

	if(ports_selected != E__TEST_PORTS)
	{
		if(ports_selected > 0)
		{
			dtest_status = dExternalLoopback(ports_selected);
			SerialExit(ports_selected);
		}
		else
			return E__NO_PORTS_TO_TEST;
	}
	else
		return(ports_selected);

	return(dtest_status);

} /* SerialExternalLoopback */


/*****************************************************************************
 * SerialLocalloopback: test function for Local loopback 
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (SerialLocalloopback, "Serial Local Loopback Test")
{
	UINT32 ports_selected, dtest_status;
	
	ports_selected = dGetNops(adTestParams);
	
	if(ports_selected != E__TEST_PORTS)
	{
		if(ports_selected > 0)
		{
		dtest_status = dLocalloopback(ports_selected);
			SerialExit(ports_selected);
		}
		else
			return E__NO_PORTS_TO_TEST;
	}
	else
		return(ports_selected);

	return(dtest_status);

} /* SerialLocalloopback */


/*****************************************************************************
 * SerialBaudRateTest: test function for Baud rate test
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (SerialBaudRateTest, "Serial Baud Rate Test")
{
	UINT32 ports_selected, dtest_status;
	
	ports_selected = dGetNops(adTestParams);

	if(ports_selected != E__TEST_PORTS)
	{
		if(ports_selected > 0)
		{
		dtest_status = dBaudRateTest(ports_selected);
			SerialExit(ports_selected);
		}
		else
			return E__NO_PORTS_TO_TEST;
	}
	else
		return(ports_selected);

	return(dtest_status);

} /* SerialBaudRateTest */


/*****************************************************************************
 * SerialInterruptTest: test function for Serial port interrupt
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (SerialInterruptTest, "Serial Interrupt Test")
{
	UINT32 ports_selected, dtest_status;
	
	ports_selected = dGetNops(adTestParams);

	if(ports_selected != E__TEST_PORTS)
	{
		if(ports_selected > 0)
		{
			dtest_status = dInterruptTest(ports_selected);
			SerialExit(ports_selected);
		}
		else
			return E__NO_PORTS_TO_TEST;
	}
	else
		return(ports_selected);

	return(dtest_status);

} /* SerialInterruptTest */


/*****************************************************************************
 * SerialRegAccessTest: test function for Serial port Register access
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (SerialRegAccessTest, "Serial Register Access Test")
{
	UINT32 ports_selected, dtest_status;
	
	ports_selected = dGetNops(adTestParams);

	if(ports_selected != E__TEST_PORTS)
	{
		if(ports_selected > 0)
		{
			dtest_status = dInterconnectTest(ports_selected);
			SerialExit(ports_selected);
		}
		else
			return E__NO_PORTS_TO_TEST;
	}
	else
		return(ports_selected);

	return(dtest_status);

} /* SerialRegAccessTest */


/*****************************************************************************
 * SerialIntHandler: Serial interrupt handling function
 *
 * RETURNS: None
 */

void serialIntHandler (void)
{
	/* see if this one is for our serial port */
	
	if ((sysInPort8(wBaseAddr + IIR) & IIR_IP) == 0) {

		/* this is for us */
		
		switch (isrCalled) {
		case 0:
		
			/* Write 1st character */
			
			sysOutPort8 (wBaseAddr + TX, 0x48);
			break;
		case 1:
		
			/* Write 2nd character */
			
			sysOutPort8 (wBaseAddr + TX, 0x47);
			break;
		default:
		
			/* all done, disable the serial port interrupts */
			
			sysOutPort8 (wBaseAddr + IER, (sysInPort8(wBaseAddr + IER)) & ~IER_THRE);
		}
		isrCalled++;
	}
} /* myHandler () */
