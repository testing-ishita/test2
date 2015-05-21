/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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

/* ide.h - definitions for ide devices
 */
/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/tempsensor.h,v 1.3 2015-03-10 15:45:47 mgostling Exp $
*
* $Log: tempsensor.h,v $
* Revision 1.3  2015-03-10 15:45:47  mgostling
* Updated TIMEOUT definition.
* Tidied up the code.
*
* Revision 1.2  2015-02-25 17:59:41  hchalla
* Added new error codes for gettting threshold temperature sensors.
*
* Revision 1.1  2013-09-04 07:40:27  chippisley
* Import files into new source repository.
*
* Revision 1.1  2009/05/15 11:15:43  jthiru
* Initial checkin for Temperature Sensor tests
*
*
*
*/

#define		TIMEOUT		0x1000			// 100

#define		MID_REG						0xFE
#define		DID_REG						0xFF
#define		BRD_TEMP					0x0
#define		CPU_TEMPU					0x1
#define		TEMP_CONF					0x3
#define		CPU_TEMPL					0x10
#define		ADT_EXTENDED_TEMP			0x04
#define		ADT_EXTENDED_TEMP_OFFSET	64

#define		E__SMB_NOT_AVAIL			E__BIT + 0x0
#define		E__SMBC_BUSY				E__BIT + 0x1
#define		E__SMBRD_TIMEOUT			E__BIT + 0x2
#define		E__ACCESS_DATA				E__BIT + 0x3
#define		E__TEMPTHRESH				E__BIT + 0x4
#define		E_SMBRW_ERROR				E__BIT + 0x5
#define		E__CPUTEMPTHRESH			E__BIT + 0x6
#define		E__BRDTEMP1THRESH			E__BIT + 0x7
#define		E__BRDTEMP2THRESH			E__BIT + 0x8

#define		TEMP_FROM_REG(val) (val > 127 ? val-256 : val)

/* typedefs */

typedef struct {
	UINT8	bBus;			/* PCI bus, device and		*/
	UINT8	bDev;			/* function numbers of the	*/
	UINT8	bFunc;			/* USB controller			*/
	UINT16	wSMBIoRegs; 	/* USB IO regs				*/
	UINT8	data;
} DEVICE;


