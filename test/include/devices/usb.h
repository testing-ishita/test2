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
/* usb.h - definitions for USB
 */
/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/usb.h,v 1.1 2013-09-04 07:40:28 chippisley Exp $
*
* $Log: usb.h,v $
* Revision 1.1  2013-09-04 07:40:28  chippisley
* Import files into new source repository.
*
* Revision 1.1  2009/05/15 11:16:22  jthiru
* Initial checkin for USB tests
*
*
*/

/* defines */
#define QUEUE_HEAD					0x13000
#define Q_COMMANDS					0x14000
#define E__USB_NOT_FOUND			E__BIT 

/* At an offset of 4 */
#define E__USB_REG_ACCESS			E__BIT + 0x1	
#define E__USB_NO_MEM				E__BIT + 0x5
#define E__USB_EHCI_RST_TIMEOUT		E__BIT + 0x9
#define E__USB_EHCI_RUN_TIMEOUT		E__BIT + 0xC
#define E__USB_OVERCURRENT			E__BIT + 0x10

/* typedefs */
typedef struct {
	UINT8 	bBus;			/* PCI bus, device and 		*/
	UINT8 	bDev;			/* function numbers of the	*/
	UINT8 	bFunc;			/* USB controller			*/
	UINT8 	bCtrlType;		/* UHCI or EHCI controller	*/
	UINT16	wUSBIoRegs;		/* USB IO regs				*/
	UINT32	wUSBMemRegs;	/* USB Memory BAR regs		*/	
	UINT32* caplogaddr;		/* Mapped Capability address*/	
	UINT32* oplogaddr;
} DEVICE;
