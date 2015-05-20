#ifndef __scc16550_h__
	#define __scc16550_h__

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

/* SCC16550.H - definitions for 16550 compatible UART
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/scc16550.h,v 1.1 2013-09-04 07:40:27 chippisley Exp $
 *
 * $Log: scc16550.h,v $
 * Revision 1.1  2013-09-04 07:40:27  chippisley
 * Import files into new source repository.
 *
 * Revision 1.7  2011/10/27 15:49:47  hmuneer
 * no message
 *
 * Revision 1.6  2011/06/07 15:02:30  hchalla
 * Support of minimum baud rate 9600
 *
 * Revision 1.5  2011/05/16 14:31:19  hmuneer
 * Info Passing Support
 *
 * Revision 1.4  2010/10/25 15:36:30  jthiru
 * Added defenitions for serial tests
 *
 * Revision 1.3  2009/05/18 09:25:29  hmuneer
 * no message
 *
 * Revision 1.2  2008/05/12 14:55:31  swilson
 * Add usefull baud-rate defines.
 *
 * Revision 1.1  2008/04/21 17:01:00  swilson
 * *** empty log message ***
 *
 */


/* includes */

/* defines */

#define COM1_BASE		(0x03F8)// currently set to com2

	/* With DLAB = 0 */

#define TX				0		/* Transmit Holding Buffer (w/o) */
#define RX				0		/* Receive Buffer (r/o) */
#define THR				0
#define RBR 			0

#define IER				1		/* Interrupt Enable Register (1=enable) */
#define		IER_RDA		0x01	/*	receive data available */
#define		IER_THRE	0x02	/*	transmit holding regiser empty */
#define		IER_RLS		0x04	/*	receive line status */
#define		IER_MS		0x08	/*	modem status */

#define IIR				2		/* Interrupt Identification Register (r/o) */
#define		IIR_IP		0x01	/*	0 = interrupt pending */
#define		IIR_INT		0x06	/*	mask for interrupt bits */
#define		IIR_MS		0x00	/*	modem status */
#define		IIR_THRE	0x02	/*	transmit holding regiser empty */
#define		IIR_RDA		0x04	/*	receive data available */
#define		IIR_RLS		0x06	/*	receive line status */
#define		IIR_FEN		0xC0	/*	FIFOs enabled */

#define FCR				2		/* FIFO Control Register (w/o) */
#define		FCR_FEN		0x01	/*	1 = enable FIFOs */
#define		FCR_CRF		0x02	/*	1 = clear receive FIFO */
#define		FCR_CTF		0x04	/*	1 = clear transmit FIFO */
#define		FCR_DMAMODE	0x08	/* */
#define		FCR_TL1		0x00	/*	FIFO interrupt trigger level = 1 */
#define		FCR_TL4		0x40	/*	FIFO interrupt trigger level = 4 */
#define		FCR_TL8		0x80	/*	FIFO interrupt trigger level = 8 */
#define		FCR_TL14	0xC0	/*	FIFO interrupt trigger level = 14 */

#define LCR				3		/* Line Control Register */
#define		LCR_DATA5	0x00	/*	5-bit data */
#define		LCR_DATA6	0x01	/*	6-bit data */
#define		LCR_DATA7	0x02	/*	7-bit data */
#define		LCR_DATA8	0x03	/*	8-bit data */
#define		LCR_STOP1	0x00	/*	1 stop bit */
#define		LCR_STOP2	0x04	/*	2 stop bits */
#define		LCR_NONE	0x00	/*	no parity */
#define		LCR_ODD		0x08	/*	odd parity */
#define		LCR_EVEN	0x18	/*	even parity */
#define		LCR_HIGH	0x28	/*	high parity */
#define		LCR_LOW		0x38	/*	low parity */
#define		LCR_BREAK	0x40	/*	1 = send a break */
#define		LCR_DLAB	0x80	/*	1 = enable divisors */

#define MCR				4		/* Modem Control Register */
#define		MCR_DTR		0x01	/*	1 = set DTR active */
#define		MCR_RTS		0x02	/*	1 = set RTS active */
#define		MCR_AUX1	0x04	/*	set aux output 1 state */
#define		MCR_AUX2	0x08	/*	set aux output 2 state */
#define		MCR_LOOP	0x10	/*	1 = enable loopback mode */

#define LSR				5		/* Line Status Register (r/o) */
#define		LSR_DR		0x01	/*	1 = data ready */
#define		LSR_OE		0x02	/*	1 = overflow error */
#define		LSR_PE		0x04	/*	1 = parity error */
#define		LSR_FE		0x08	/*	1 = framing error */
#define		LSR_BREAK	0x10	/*	1 = break received */
#define		LSR_THRE	0x20	/*	1 = transmit holding register empty */
#define		LSR_DHRE	0x40	/*	1 = data holding regiter empty */
#define		LSR_FIFO	0x80	/*	1 = FIFO error */

#define MSR				6		/* Modem Status Register (r/o) */
#define		MSR_DCTS	0x01	/*	1 = delta CTS */
#define		MSR_DDSR	0x02	/*	1 = delta DSR */
#define		MSR_TERI	0x04	/*	1 = trailing edge RI */
#define		MSR_DDCD	0x08	/*	1 = delta DCD */
#define		MSR_CTS		0x10	/*	state of CTS */
#define		MSR_DSR		0x20	/*	state of DSR */
#define		MSR_RI		0x40	/*	state of RI */
#define		MSR_CD		0x80	/*	state of CD */

#define SCRATCH			7		/* Scratch Register */

	/* With DLAB = 1 */

#define DLL				0		/* Divisor Latch Low */
#define DLH				1		/* Divisor Latch High */

#define BAUD_9600		12		/* Divisor values */
#define BAUD_19200		6
#define BAUD_38400		3
#define BAUD_57600		2
#define BAUD_115000		1

#define TIMEOUT_LIMIT		0x00010000

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


#endif

