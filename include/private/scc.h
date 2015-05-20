#ifndef __scc_h__
	#define __scc_h__

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

/* scc.h - include file for SCC access functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/private/scc.h,v 1.1 2013-09-04 07:40:42 chippisley Exp $
 *
 * $Log: scc.h,v $
 * Revision 1.1  2013-09-04 07:40:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2008/06/20 12:16:58  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.1  2008/05/14 09:59:08  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/05/12 15:04:40  swilson
 * Add IBIT console. Debugging various library functions.
 *
 */


/* includes */

#include <stdtypes.h>

#include <devices/scc16550.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern void sysSccInit (UINT16 wBaseAddr, UINT16 wBaudDiv);

extern void sysSccTxChar (UINT16 wBaseAddr, UINT8 bData);
extern UINT8 sysSccRxChar (UINT16 wBaseAddr);

extern int sysSccPollTx (UINT16 wBaseAddr);
extern void sysSccFlushTx (UINT16 wBaseAddr);

extern int sysSccPollRx (UINT16 wBaseAddr);



/* forward declarations */


#endif

