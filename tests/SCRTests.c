
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

/* SCRTests.c - Status and Control registers Tests.
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/SCRTests.c,v 1.3 2015-04-01 16:48:46 hchalla Exp $
 * $Log: SCRTests.c,v $
 * Revision 1.3  2015-04-01 16:48:46  hchalla
 * Added more debug info for debugging in this test.
 *
 * Revision 1.2  2013-11-25 12:25:33  mgostling
 * Fixed wRegReadOnly test.
 *
 * Revision 1.1  2013-09-04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.7  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.6  2010/06/29 14:07:32  hchalla
 * Code Cleanup, Added headers and comments.
 *
 * Revision 1.5  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.4  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.3  2009/06/10 14:45:11  hmuneer
 * smbus and eeprom update
 *
 * Revision 1.2  2009/06/08 09:55:58  hmuneer
 * TPA40
 *
 * Revision 1.1  2009/05/18 09:16:38  hmuneer
 * no message
 *
 * Revision 1.0  Feb 9 2009
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
 
#include <bit/board_service.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>


/* defines */
//#define DEBUG
#define SCR_RW_ERROR		(E__BIT  +10000)
#define SCR_RO_ERROR		(E__BIT  +20000)

/* typedefs */
/* constants */
/* locals */
/* globals */
/* externals */
/* forward declarations */


/*****************************************************************************\
 *
 *  TITLE:  wRegReadOnlyTest ()
 *
 *  ABSTRACT:  This function tests the reading of status and control registers.
 *
 * 	RETURNS: E__OK, E__FAIL.
 *
\*****************************************************************************/
static UINT32 wRegReadOnlyTest( UINT16	dPortAddress, UINT8	bBitMask)
{
	UINT8   Backup;
	volatile UINT8	bFixed;
	UINT8	bInvert;

	#ifdef DEBUG
		char	achBuffer[80];	/* text formatting buffer   */
	#endif

	Backup   = ((UINT8)dIoReadReg(dPortAddress, REG_8));
	bFixed   = Backup & bBitMask;			// get current RO bits

	bInvert = ((~Backup) & bBitMask);		// invert RO bits
	bInvert |= (Backup & (~bBitMask));		// keep RW bits unchanged


	vDelay(4);
#ifdef DEBUG
	sprintf (achBuffer, "Data: Backup %02Xh, bBitMask %02Xh\n", Backup, bBitMask);
	vConsoleWrite (achBuffer);
	sysDebugWriteString(achBuffer);
	sprintf (achBuffer, "Data: bInvert %02Xh, bFixed (before) %02Xh\n", bInvert, bFixed);
	vConsoleWrite (achBuffer);
	sysDebugWriteString(achBuffer);
#endif
	
	vIoWriteReg(dPortAddress, REG_8, bInvert);
	
	bFixed = ((UINT8)dIoReadReg(dPortAddress, REG_8));
	vDelay(4);
#ifdef DEBUG
	sprintf (achBuffer, "Data: bFixed (after) %02Xh\n", bFixed);
	vConsoleWrite (achBuffer);
#endif

	if(Backup == bFixed )
		return E__OK;
	else
	{
		#ifdef DEBUG
			sprintf (achBuffer, "Data error: Backup %02Xh, bBitMask %02Xh\n", Backup, bBitMask);
			vConsoleWrite (achBuffer);
			sysDebugWriteString(achBuffer);
			sprintf (achBuffer, "Data error: bInvert %02Xh, bFixed %02Xh", bInvert, bFixed);
			vConsoleWrite (achBuffer);
			sysDebugWriteString(achBuffer);
		#endif
		return E__FAIL;
	}
}

/*****************************************************************************\
 *
 *  TITLE:  wReg8BitTest ()
 *
 *  ABSTRACT:  This function tests the read and write of status and
 *             control registers.
 *
 * 	RETURNS: E__OK, E__FAIL.
 *
\*****************************************************************************/
static UINT32 wReg8BitTest( UINT16	dPortAddress, UINT8	bBitMask)
{
	UINT8   Backup;
	UINT8	bFixed;			/* fixed bit pattern (of those outside the mask) */
	UINT8	bPattern;		/* current test bit-pattern */
	UINT8	bBitNum;		/* register bit number      */
	UINT8	bTemp;			/* like it says!            */

	#ifdef DEBUG
		char	achBuffer[80];	/* text formatting buffer   */
	#endif

	Backup = ((UINT8)dIoReadReg(dPortAddress, REG_8));
	bFixed = ((UINT8)dIoReadReg(dPortAddress, REG_8)) & ~bBitMask;

	for (bBitNum = 0; bBitNum < 8; bBitNum++)
	{
		bPattern = ((1 << bBitNum) & bBitMask);		/* Marching '1' */
		vIoWriteReg(dPortAddress, REG_8, bPattern | bFixed);
		bTemp = ((UINT8)dIoReadReg(dPortAddress, REG_8)) & bBitMask;
		vDelay(4);
		if (bTemp != bPattern)
		{	
			vIoWriteReg(dPortAddress, REG_8, Backup);				
			
			#ifdef DEBUG
				sprintf (achBuffer, "Data error (1)[Addr:0x%x]: wrote %02Xh, read %02Xh\n",
						 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
				sysDebugWriteString(achBuffer);
			#endif
			return E__FAIL;
		}

		bPattern = (~(1 << bBitNum) & bBitMask);	/* Marching '0' */
		vIoWriteReg(dPortAddress, REG_8, bPattern | bFixed);
		bTemp =  ((UINT8)dIoReadReg(dPortAddress, REG_8)) & bBitMask;

		if (bTemp != bPattern)
		{		
			vIoWriteReg(dPortAddress, REG_8, Backup);
			
			#ifdef DEBUG
				sprintf (achBuffer, "Data error (0)[Addr:0x%x]: wrote %02Xh, read %02Xh\n",
					 	 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
				sysDebugWriteString(achBuffer);
			#endif
			return E__FAIL;
		}
	}
	
	vIoWriteReg(dPortAddress , REG_8, Backup);
	return (E__OK);

} /* wReg8BitTest () */

/*****************************************************************************\
 *
 *  TITLE:  SCRTest ()
 *
 *  ABSTRACT:  Entry point to the SCRTest and executes the read write tests on
 *             Status and control registers.
 *
 * 	RETURNS: E__OK, E__FAIL.
 *
\*****************************************************************************/
TEST_INTERFACE (SCRTest, "Status & Control Register Test")
{
	UINT32 rt = E__OK, count = 0;
	SACReg* regs;

	#ifdef DEBUG
		char	achBuffer[80];	/* text formatting buffer */
	#endif

	board_service(SERVICE__BRD_GET_SCR_INFO, NULL, &regs);

	if(regs !=  NULL)
	{
		while(regs[count].Port != 0x00 && (rt == E__OK))
		{
			rt = wReg8BitTest( regs[count].Port, regs[count].WRMask);
			vDelay(4);
			#ifdef DEBUG
				sprintf (achBuffer, "\nTesting[WR] \"%s\"[0x%x], Result 0x%x \n", regs[count].Name, regs[count].Port, rt);
				vConsoleWrite (achBuffer);
				sysDebugWriteString(achBuffer);
			#endif

			if(rt == E__OK)
			{
				rt = wRegReadOnlyTest( regs[count].Port, regs[count].ROMask);
				vDelay(4);
				#ifdef DEBUG
					sprintf (achBuffer, "\nTesting[RD] \"%s\"[0x%x], Result 0x%x \n", regs[count].Name, regs[count].Port, rt);
					vConsoleWrite (achBuffer);
					sysDebugWriteString(achBuffer);
				#endif

				if(rt != E__OK)
					rt = regs[count].Port + SCR_RO_ERROR;
			}
			else
				rt = regs[count].Port + SCR_RW_ERROR;

			count++;
		}
	}

	return rt;
}
