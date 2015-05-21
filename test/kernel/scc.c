
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

/* scc.c - common, low level serial I/O functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/scc.c,v 1.1 2013-09-04 07:41:29 chippisley Exp $
 *
 * $Log: scc.c,v $
 * Revision 1.1  2013-09-04 07:41:29  chippisley
 * Import files into new source repository.
 *
 * Revision 1.9  2011/11/21 11:16:47  hmuneer
 * no message
 *
 * Revision 1.8  2011/06/07 14:58:06  hchalla
 * Removed the support for serial port baud rate below 9600, the minimum baud rate now is 9600.
 *
 * Revision 1.7  2011/05/16 14:31:33  hmuneer
 * Info Passing Support
 *
 * Revision 1.6  2011/03/22 13:46:15  hchalla
 * *** empty log message ***
 *
 * Revision 1.5  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.4  2008/09/29 13:27:05  swilson
 * Clean up makefile include dependencies.
 *
 * Revision 1.3  2008/06/20 12:16:54  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.2  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/05/12 14:56:27  swilson
 * General access to SCC devices.
 *
 */


/* includes */

#include <stdtypes.h>

#include <private/port_io.h>
#include <private/scc.h>

 

#include <devices/scc16550.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */



/*****************************************************************************
 * sysSccInit: initialise an SCC channel for 8 data, no parity, 1 stop
 *
 * RETURNS: None
 */

void sysSccInit 
(
	UINT16	wBaseAddr,		/* port base address */
	UINT16	wBaudDiv		/* baud rate divisor */
)
{
	/* Program baud rate divisor */

	sysOutPort8 (wBaseAddr + LCR, LCR_DLAB);					/* DLAB = 1 */
	switch(wBaudDiv)
	{
		case 0:
			sysOutPort8 (wBaseAddr + DLL, (UINT8)(BAUD_9600 & 0x00FF));
			break;

		case 1:
			sysOutPort8 (wBaseAddr + DLL, (UINT8)(BAUD_19200 & 0x00FF));
			break;

		case 2:
			sysOutPort8 (wBaseAddr + DLL, (UINT8)(BAUD_38400 & 0x00FF));
			break;

		case 3:
			sysOutPort8 (wBaseAddr + DLL, (UINT8)(BAUD_57600 & 0x00FF));
			break;

		case 4:
			sysOutPort8 (wBaseAddr + DLL, (UINT8)(BAUD_115000 & 0x00FF));
			break;

		default:
			sysOutPort8 (wBaseAddr + DLL, (UINT8)(BAUD_9600 & 0x00FF));
			break;

	}
	sysOutPort8 (wBaseAddr + DLH, (UINT8)(0));

	/* Select 8N1 format */

	sysOutPort8 (wBaseAddr + LCR, LCR_DATA8 | LCR_NONE | LCR_STOP1);

	/* Disable interrupts */

	sysOutPort8 (wBaseAddr + IER, 0);

	/* Enable and reset FIFOs */

	sysOutPort8 (wBaseAddr + FCR, FCR_FEN | FCR_CRF | FCR_CTF);

} /* sysSccInit () */


/*****************************************************************************
 * sysSccTxChar: transmit a character via a serial channel, wait for TX empty
 *
 * RETURNS: None
 */

void sysSccTxChar
(
	UINT16	wBaseAddr,		/* port base address */
	UINT8	bData			/* character to send */
)
{
	/* Wait for TX Buffer to go empty */
	int tmout = 1000;

	while ( (sysSccPollTx (wBaseAddr) == 1) && (tmout > 0) )
		tmout--;

	/* Write the character */

	sysOutPort8 (wBaseAddr + TX, bData);

} /* sysSccTxChar () */


/*****************************************************************************
 * sysSccRxChar: receive a character from a serial channel, wait for RX full
 *
 * RETURNS: None
 */

UINT8 sysSccRxChar
(
	UINT16	wBaseAddr		/* port base address */
)
{
	/* Wait for char in RX Buffer */

	while (sysSccPollRx (wBaseAddr) == 0)
		;

	/* Read the character */

	return sysInPort8 (wBaseAddr + RX);

} /* sysSccRxChar () */


/*****************************************************************************
 * sysSccPollTx: test serial channel Transmit Buffer Empty status
 *
 * RETURNS: 0 if empty, 1 if full
 */

int sysSccPollTx
(
	UINT16	wBaseAddr		/* port base address */
)
{
	if (sysInPort8 (wBaseAddr + LSR) & LSR_THRE)
		return (0);

	return (1);

} /* sysSccPollTx () */


/*****************************************************************************
 * sysSccFlushTx: wait for TX to complete activity
 *
 * RETURNS: none
 */

void sysSccFlushTx
(
	UINT16	wBaseAddr		/* port base address */
)
{
	UINT8	bTemp;


	do {
		bTemp = sysInPort8 (wBaseAddr + LSR);

	} while ((bTemp & LSR_DHRE) == 0);

} /* sysSccFlushTx () */


/*****************************************************************************
 * sysSccPollRx: test serial channel Receive Buffer Full status
 *
 * RETURNS: 0 if empty, 1 if full
 */

int sysSccPollRx
(
	UINT16	wBaseAddr		/* port base address */
)
{
	if (sysInPort8 (wBaseAddr + LSR) & LSR_DR)
		return (1);

	return (0);

} /* sysSccPollRx () */



