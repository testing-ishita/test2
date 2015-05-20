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


/* ram.h - definitions for sdram
 */


/* Version control:

*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/ram.h,v 1.2 2013-11-07 11:40:22 mgostling Exp $
*
* $Log: ram.h,v $
* Revision 1.2  2013-11-07 11:40:22  mgostling
* Added support for Haswell
*
* Revision 1.1  2013-09-04 07:40:27  chippisley
* Import files into new source repository.
*
* Revision 1.9  2012/08/30 16:02:01  hmuneer
* 8GB support for vp717
*
* Revision 1.8  2011/10/27 15:49:47  hmuneer
* no message
*
* Revision 1.7  2011/06/08 09:38:43  hchalla
* Changed the error codes for ECC Error.
*
* Revision 1.6  2011/06/07 15:01:58  hchalla
*  Added support for 64-bit memory allocation to test the ram memory aove 4GB. Checked in on behalf of Haroon Muneer.
*
* Revision 1.5  2011/03/22 13:40:30  hchalla
* Updated new error codes.
*
* Revision 1.4  2011/02/01 12:12:07  hmuneer
* C541A141
*
* Revision 1.3  2011/01/20 10:01:26  hmuneer
* CA01A151
*
* Revision 1.2  2010/06/23 10:59:59  hmuneer
* CUTE V1.01
*
* Revision 1.1  2009/05/15 11:15:28  jthiru
* Initial checkin for Memory tests
*
*/



/* defines */
#define MAX_ROWS			32
#define RAM_BASE				0x8000000
								//leave first 128MB alone

#define SODIMM_ADDR0		0x0
#define SODIMM_ADDR1		0x1
#define SODIMM_ADDR2		0x2
#define SODIMM_ADDR3		0x3


#define SMBHSTSTS			0x00
#define SMBSLVSTS			0x01
#define SMBHSTCNT			0x02
#define SMBHSTCMD			0x03
#define SMBHSTADD			0x04
#define SMBHSTDAT0			0x05

#define SMB_READ			0x08
#define SMB_START			0x40


#define E__DATA_BUS_ERR		E__BIT
#define E__RAM_NO_MEM		E__BIT + 0x1
#define E__ADDR_SHORT		E__BIT + 0x2	
#define E__ADDR_STUCK_HIGH	E__BIT + 0x3
#define E__ECC_INDUCED_FAIL	E__BIT + 0x4
#define E__SPD_RD			E__BIT + 0x5
#define E__NO_SPD			E__BIT + 0x9
#define E__ACCESS_SPD		E__BIT + 0xD
#define E__SPD_CHKSUM		E__BIT + 0x11
#define E__INVALID_PARAM    E__BIT + 0x12
#define E__DAT_RDWR_ERR     E__BIT + 0x13
#define E__ECC_ERR          E__BIT + 0x20
#define E__ECC_CHAN0        E__ECC_ERR + 0x01
#define E__ECC_CHAN1        E__ECC_ERR + 0x02

/* typedefs */

#define RAM_GS45			0x01
#define RAM_SANDY_BRIDGE	0x02
#define RAM_GENERIC			0x04
#define RAM_HASWELL			0x08

#define HASWELL_ID              0x0C048086
#define HASWELL_BASE_MEM        0x10000000
#define HASWELL_TOLUD_REG       0xBC
#define HASWELL_TOUUD_REG_LO    0xA8
#define HASWELL_TOUUD_REG_HI    0xAC
#define HASWELL_TOM_REG_HI		0xA4

typedef struct 
{
	UINT64 total;	// total physical RAM
	UINT32 base;	// 32 bit base
	UINT32 len;		// 32 bit length  base to below 4GB
	UINT64 len64;	// memory above 4GB
	UINT32 type;	// board type, used in case boards have different RAM Arrangement

} RAM;

typedef struct 
{
	UINT32	dwd [MAX_ROWS];		/* offset of dword-sized block to test */
	UINT8	row [MAX_ROWS];		/* number of row which will be tested */
	UINT8	nrows;				/* total number of rows in the test */

} RAM_ROWS;







