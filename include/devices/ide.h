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

/* ide.h - definitions for ide devices
 */

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/ide.h,v 1.1 2013-09-04 07:40:27 chippisley Exp $
*
* $Log: ide.h,v $
* Revision 1.1  2013-09-04 07:40:27  chippisley
* Import files into new source repository.
*
* Revision 1.3  2010/06/23 10:59:59  hmuneer
* CUTE V1.01
*
* Revision 1.2  2009/06/19 13:22:13  jthiru
* Added check for port number passed by user
*
* Revision 1.1  2009/05/15 11:13:56  jthiru
* Initial checkin for SATA tests
*
*
*
*/


/* defines */
#define TYPE_IDE			1
#define TYPE_SATA			2

#define HDC_PORT				0		/* hard disk controller port 	*/
#define HDC_PORT_DATA			0		/* data RW 						*/
#define HDC_PORT_ERROR			1		/* error register RO 			*/
#define HDC_PORT_PRECOMP		1		/* write precompensation reg WO */
#define HDC_PORT_NSECT			2		/* number of sectors RW			*/
#define HDC_PORT_SECTNO 		3		/* starting sector number RW 	*/
#define HDC_PORT_LOCYL			4		/* low 8-bits of cylinder RW 	*/
#define HDC_PORT_HICYL			5		/* high 2-bits of cylinder RW 	*/
#define HDC_PORT_SDH			6		/* size/drive/head RW 			*/
#define HDC_PORT_STATUS 		7		/* status register RO 			*/
#define HDC_PORT_CMD			7		/* command register WO 			*/


/* Offset from 0x3f4, 0x374 */
#define HDC_REG         		6		/* hard disk register port 		*/
#define HDC_ALT_STATUS			6		/* alternate status register 	*/

#define HDC_CMD_RECAL	  		0x10	/* recalibrate command 			*/
#define HDC_CMD_READ	  		0x20	/* read command 				*/
#define HDC_CMD_WRITE	  		0x30	/* write command 				*/
#define HDC_CMD_VERIFY	  		0x40	/* verify command 				*/
#define HDC_CMD_FORMAT	  		0x50	/* format command 				*/
#define HDC_CMD_INIT	  		0x60	/* initialize command 			*/
#define HDC_CMD_SEEK	  		0x70	/* seek command 				*/
#define HDC_CMD_DIAG	  		0x90	/* perform diagnostic command 	*/
#define HDC_CMD_READ_LONG 		0x22	/* read long (ECC) command 		*/
#define HDC_CMD_WRITE_LONG 		0x32	/* write long (ECC) command 	*/
#define HDC_CMD_SET_PARAM 		0x91	/* set parameters command 		*/
#define HDC_CMD_IDENT			0xEC	/* Identify drive command 		*/

/* PCI register access size codes */

#define PCI_BYTE					0x01	
#define PCI_WORD					0x02
#define PCI_DWORD					0x03	

/* IDE Error Codes at offset of 6 for each instance */
#define	E__TEST_NO_DEVICE		E__BIT
#define E__IDE_REGS_ERROR		E__BIT + 0x6
#define E__BIT_IDE_PARAMS		E__BIT + 0xC
#define E__BIT_IDE_NO_MEM		E__BIT + 0xD
#define E__IDE_NO_INTERRUPT		E__BIT + 0x13
#define E__IDE_NO_SIGN			E__BIT + 0x19
#define E__IDE_INT_ERROR		E__BIT + 0x1F
#define E__TEST_NO_PORT			E__BIT + 0x25


/* typedefs */
typedef struct {
	UINT16	wCmdReg;
	UINT16	wCtrlReg;
	char	bIrq;
} PORTINFO;

typedef struct {
	UINT16 bPort;
	UINT8 bData;
} TESTINFO;

typedef struct {
	UINT8 	bBus;			/* PCI bus, device and 		*/
	UINT8 	bDev;			/* function numbers of the	*/
	UINT8 	bFunc;			/* IDE controller			*/
	UINT8 	bCtrlType;		/* IDE or SATA controller	*/
	UINT16	wBMIdeRegs;		/* Bus Master IDE regs		*/
	PORTINFO sPortInfo[2];
} DEVICE;



