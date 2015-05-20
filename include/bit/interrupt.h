#ifndef __interrupt_h__
	#define __interrupt_h__

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

/* interrupt.h - interrupt primitives
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/interrupt.h,v 1.1 2013-09-04 07:35:26 chippisley Exp $
 *
 * $Log: interrupt.h,v $
 * Revision 1.1  2013-09-04 07:35:26  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2012/11/02 14:05:32  chippisley
 * Added sysCpuIntLock(), sysCpuIntUnLock() & SW vector descriptions.
 *
 * Revision 1.4  2010/03/31 16:28:35  hmuneer
 * no message
 *
 * Revision 1.3  2009/06/08 16:17:45  swilson
 * Add support for four software inter-processor interrupts.
 *
 * Revision 1.2  2009/05/20 13:19:03  swilson
 * Function names containing 'IRQ' replaced with 'pin' for consistency with other code.
 *  Interrupt resource clean up functions added.
 *
 * Revision 1.1  2009/05/20 12:12:17  swilson
 * CPU interrupt test.
 *
 */


/* includes */

#include <stdtypes.h>

/* defines */
#define SYS_IOAPIC0			0x00
#define SYS_IOAPIC1			0x01

#define SW_VECTOR_1			0x40	/* Used by SMP BIT */
#define SW_VECTOR_2			0x41	/* RTOS timer tick */
#define SW_VECTOR_3			0x42	/* RTOS task yield */
#define SW_VECTOR_4			0x43	/* RTOS start AP sheduling */
#define SW_VECTOR_5			0x44	/* RTOS AP tick */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

	/* idt.S */

extern 	int sysInstallUserHandler (UINT8 bVector, void (*pf)());
extern	void sysDeleteUserHandlers (void);

	/* ioapic.S */

extern	int sysUnmaskPin (UINT8 bPin, UINT8 IOAPIC);
extern	int sysMaskPin (UINT8 bPin, UINT8 IOAPIC);
extern	void sysMaskAll (void);

extern	void sysDisableInterrupts (void);
extern	void sysEnableInterrupts (void);

extern	UINT32 sysCpuIntLock( void );
extern	void sysCpuIntUnlock( UINT32 key );

extern	int sysPinToVector (UINT8 bPin, UINT8 IOAPIC);

extern	int sysSendIpi (UINT8 bApicId, UINT8 bVector);

/* forward declarations */


#endif

