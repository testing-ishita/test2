
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

/* ledtest.c - LED Test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/videoTest.c,v 1.4 2015-04-01 16:50:45 hchalla Exp $
 * $Log: videoTest.c,v $
 * Revision 1.4  2015-04-01 16:50:45  hchalla
 * Added new Intel Video graphics chip Device ID.
 *
 * Revision 1.3  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.2  2013-11-25 13:14:37  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1  2013-09-04 07:46:45  chippisley
 * Import files into new source repository.
 *
 * Revision 1.14  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.13  2011/12/01 13:45:13  hchalla
 * Updated for VX 541 board.
 *
 * Revision 1.12  2011/11/14 17:34:09  hchalla
 * Updated for PP 531.
 *
 * Revision 1.11  2011/08/02 17:01:48  hchalla
 * Added new video test for TR 803.
 *
 * Revision 1.10  2011/05/11 12:17:17  hchalla
 * TP 702 PBIT Related changes.
 *
 * Revision 1.9  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.8  2010/11/04 17:58:32  hchalla
 * Added new video controller support SM722 for VP 417.
 *
 * Revision 1.7  2010/09/15 12:25:15  hchalla
 * Modified tests to  support TR 501 BIT/CUTE.
 *
 * Revision 1.6  2010/06/29 10:31:54  hchalla
 * Code Cleanup, Added headers and comments.
 *
 * Revision 1.5  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.4  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.3  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.2  2009/06/08 09:55:58  hmuneer
 * TPA40
 *
 * Revision 1.1  2009/05/18 09:25:30  hmuneer
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
 
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <devices/pci_devices.h>
#include <private/sys_delay.h>


/* defines */
/* typedefs */
/* constants */
/* locals */
/* globals */
UINT32  dDidVid = 0;        /* device and vendor IDs */
/* externals */
/* forward declarations */

//#define DEBUG


/* index and data register pairs */

#define REG_SEQX			0x3C4		/* VGA Sequencer Index Register */
#define REG_SEQD			0x3C5		/* VGA Sequencer Data Register */
#define	REG_CRTX			0x3D4		/* VGA CRT index register */
#define	REG_CRTD			0x3D5		/* VGA CRT data register */

/* indexed registers */
#define IREG_MCR62			0x62        /* Memory Type & Timing Control */
#define IREG_MCR76			0x76        /* Memory Type & Timing Control */
#define IREG_CRT0E			0x0E		/* Cursor Location High */
#define IREG_CRT0F			0x0F		/* Cursor Location Low */
#define IREG_SCR18			0x18        /* General Graphics Command 2 */
#define IREG_SCR1C			0x1C        /* Interrupt Status */
#define IREG_SCR1F			0x1F        /* Mask & HW Interrupt Enable */
#define IREG_GPR73			0x73        /* User Defined Register 2 */

#define VID_SM722			0x126F
#define DID_VIDEOSM722		0x0720

#define VID_INTEL	        0x8086
#define DID_VIDEO	        0x2a42
#define DID_VIDEOi7		    0x0046
#define DID_ATI             0x94CB
#define VID_ATI             0x1002

#define DID_VIDEO_ATI         	((DID_ATI << 16) | VID_ATI)
#define DID_VIDEO_SANDYBRIDGE	0x0116
#define DID_VIDEO_IVYBRIDGE		0x0166
#define DID_VIDEO_HASWELL		0x0416
#define DID_VIDEO_HASWELL_CELERON		0x0406

#define DIDVID_VIDEO 	            ((DID_VIDEO << 16) | VID_INTEL)
#define DIDVID_VIDEOi7 	            ((DID_VIDEOi7 << 16) | VID_INTEL)
#define DIDVID_VIDEO_SANDYBRIDGE 	((DID_VIDEO_SANDYBRIDGE << 16) | VID_INTEL)
#define DIDVID_VIDEOSM722           ((DID_VIDEOSM722 << 16) | VID_SM722)
#define DIDVID_VIDEO_IVYBRIDGE 		((DID_VIDEO_IVYBRIDGE << 16) | VID_INTEL)
#define DIDVID_VIDEO_HASWELL 		((DID_VIDEO_HASWELL << 16) | VID_INTEL)
#define DIDVID_VIDEO_HASWELL_CELERON 		((DID_VIDEO_HASWELL_CELERON << 16) | VID_INTEL)

#define	E__TEST_NO_DEVICE		E__BIT 
#define E__VIDEO_ACCESS			E__BIT  + 0x01
#define E__VIDEO_MEM			E__BIT  + 0x02

#ifdef DEBUG
	static char	 buffer[128];
#endif

/*****************************************************************************\
 *
 *  TITLE:  findvideo ()
 *
 *  ABSTRACT: This function will find the video device using pci vendor id and
 *            device id scan.
 *
 * 	RETURNS: Video device id.
 *
\*****************************************************************************/
static PCI_PFA findvideo(UINT32 *pdDidVid)
{
	PCI_PFA	pfa;

	UINT8	bTemp;
	UINT8	bBus = 0;
	UINT8	bDev = 0;
	UINT8	bFunc = 0;
	UINT8	bMultiFunc   = 0;	/* non-zero indicates multi-function device */
	UINT8	bHostBridges = 0;
	UINT8	bScanToBus   = 0;	
	
	do 
	{
		pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
		*pdDidVid = PCI_READ_DWORD(pfa, PCI_VENDOR_ID);

		if ((*pdDidVid == DIDVID_VIDEO))
		{		
			#ifdef DEBUG
				sprintf(buffer, "Video found at Bus:%x, Dev:%x, Func:%x\n", bBus, bDev,  bFunc);
				vConsoleWrite(buffer); 
			#endif
				
			return pfa;		
		}
		else if ((*pdDidVid == DIDVID_VIDEOi7))
		{
			#ifdef DEBUG
				sprintf(buffer, "Video i7 found at Bus:%x, Dev:%x, Func:%x\n", bBus, bDev,  bFunc);
				vConsoleWrite(buffer);
			#endif

			return pfa;
		}
		else if ((*pdDidVid == DIDVID_VIDEO_SANDYBRIDGE))
		{
			#ifdef DEBUG
				sprintf(buffer, "Sandy Bridge Video found at Bus:%x, Dev:%x, Func:%x\n", bBus, bDev,  bFunc);
				vConsoleWrite(buffer);
			#endif

			return pfa;
		}
		else if ((*pdDidVid == DIDVID_VIDEO_IVYBRIDGE))
		{
			#ifdef DEBUG
				sprintf(buffer, "Ivy Bridge Video found at Bus:%x, Dev:%x, Func:%x\n", bBus, bDev,  bFunc);
				vConsoleWrite(buffer);
			#endif

			return pfa;
		}
		else if ((*pdDidVid == DIDVID_VIDEO_HASWELL))
		{
			#ifdef DEBUG
				sprintf(buffer, "Haswell Video found at Bus:%x, Dev:%x, Func:%x\n", bBus, bDev,  bFunc);
				vConsoleWrite(buffer);
			#endif

			return pfa;
		}
		else if ((*pdDidVid == DIDVID_VIDEO_HASWELL_CELERON))
		{
			#ifdef DEBUG
				sprintf(buffer, "Haswell Video found at Bus:%x, Dev:%x, Func:%x\n", bBus, bDev,  bFunc);
				vConsoleWrite(buffer);
			#endif

			return pfa;
		}
		else if ((*pdDidVid == DIDVID_VIDEOSM722))
		{
			#ifdef DEBUG
				sprintf(buffer, "SM 722 found at Bus:%x, Dev:%x, Func:%x\n", bBus, bDev,  bFunc);
				vConsoleWrite(buffer);
			#endif

			return pfa;
		}
		else if ((*pdDidVid == DID_VIDEO_ATI))
		{
		   #ifdef DEBUG
					sprintf(buffer, "ATI Video Device E2400 found at Bus:%x, Dev:%x, Func:%x\n", bBus, bDev,  bFunc);
					vConsoleWrite(buffer);
		   #endif

			return pfa;
		}
		else	/* look for PCI-PCI bridges */
		{
			if (bFunc == 0)
			{
				pfa = PCI_MAKE_PFA (bBus, bDev, 0);
				bTemp = (UINT8)PCI_READ_BYTE (pfa, PCI_HEADER_TYPE);
				bMultiFunc = (bTemp & 0x80) || ((bBus == 0) && (bDev == 0));
			}
			pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
			bTemp = (UINT8)PCI_READ_BYTE (pfa, PCI_BASE_CLASS );

			if (bTemp == 0x06)	/* PCI<->PCI bridge class */
			{
				pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
				bTemp = (UINT8)PCI_READ_BYTE (pfa, PCI_SUB_CLASS);

				if (bTemp == 0x00)
				{
					if (bHostBridges > 0)
						bScanToBus++;

					bHostBridges++;
				}
				else if (bTemp == 0x04)		/* PCI-PCI */
				{
					pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
					bTemp = (UINT8)PCI_READ_BYTE (pfa, PCI_SUB_BUS);
					if (bTemp > bScanToBus)
						bScanToBus = bTemp;
				}
			}
		}

		/* Increment device/bus numbers */

		if (bMultiFunc != 0)
		{
			bFunc++;
			if (bFunc == 8)
			{
				(bDev)++;
				bFunc = 0;
				bMultiFunc = 0;
			}
		}
		else
			(bDev)++;

		if (bDev == 32)
		{
			bDev = 0;
			(bBus)++;
		}

	}while (bBus <= bScanToBus);

	return NULL;

}

/*****************************************************************************\
 *
 *  TITLE:  wReg8BitTest ()
 *
 *  ABSTRACT: This function will test the 8-bit register test by writing fixed
 *  		   pattern to known register locations.
 *
 * 	RETURNS: E__OK, E__FAIL.
 *
\*****************************************************************************/

static UINT32 wReg8BitTest(PCI_PFA pfa, UINT16	dPortAddress, UINT8	bBitMask)
{
	UINT8   Backup;
	UINT8	bFixed;			/* fixed bit pattern (of those outside the mask) */
	UINT8	bPattern;		/* current test bit-pattern */
	UINT8	bBitNum;		/* register bit number */
	UINT8	bTemp;			/* like it says! */

	#ifdef DEBUG
		char	achBuffer[80];	/* text formatting buffer */
	#endif

	Backup = PCI_READ_BYTE(pfa, dPortAddress);
	bFixed = PCI_READ_BYTE(pfa, dPortAddress) & ~bBitMask;

	for (bBitNum = 0; bBitNum < 8; bBitNum++)
	{
		bPattern = ((1 << bBitNum) & bBitMask);		/* Marching '1' */
		PCI_WRITE_BYTE(pfa, dPortAddress, bPattern | bFixed);
		bTemp = PCI_READ_BYTE(pfa, dPortAddress) & bBitMask;

		if (bTemp != bPattern)
		{	
			PCI_WRITE_BYTE(pfa, dPortAddress, Backup);				
			
			#ifdef DEBUG
				sprintf (achBuffer, "\nData error (1)[Addr:0x%x]: wrote %02Xh, read %02Xh",
						 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}

		bPattern = (~(1 << bBitNum) & bBitMask);	/* Marching '0' */
		PCI_WRITE_BYTE(pfa, dPortAddress, bPattern | bFixed);
		bTemp =  PCI_READ_BYTE(pfa, dPortAddress) & bBitMask;

		if (bTemp != bPattern)
		{		
			PCI_WRITE_BYTE(pfa, dPortAddress, Backup);
			
			#ifdef DEBUG
				sprintf (achBuffer, "\nData error (0)[Addr:0x%x]: wrote %02Xh, read %02Xh",
					 	 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}
	}
	
	PCI_WRITE_BYTE(pfa, dPortAddress, Backup);
	return (E__OK);

} /* wReg8BitTest () */

/*****************************************************************************\
 *
 *  TITLE:  VideoAccess
 *
 *  ABSTRACT: Entry point to the test VideoAccess of the Intel Video Device.
 *
 * 	RETURNS: E__OK, E__FAIL, E__ error code.
 *
\*****************************************************************************/
TEST_INTERFACE (VideoAccess, "Video Interconnect Test")
{
	UINT32   rt = E__OK;
	PCI_PFA	 pfa;
	UINT32   dTemp;
	void     *ptr;
	PTR48	 p1;
	UINT32	 dHandle1;
	char     buffer[128];
	UINT8  reg;
	UINT32 dDidVid = 0;

	pfa = findvideo(&dDidVid);

	if (pfa != NULL)
	{		
		if (wReg8BitTest(pfa, 0x3c, 0xff) == E__FAIL)
			rt = E__VIDEO_ACCESS;

		if(rt == E__OK)
		{

			if (dDidVid == DIDVID_VIDEOSM722)
			{
				dTemp = PCI_READ_DWORD(pfa, 0x10) & 0xfffffff0;
				dTemp += 0x00200000;

				/* switch to linear addressing mode */
				vIoWriteReg (REG_SEQX, REG_8, IREG_SCR18);
				reg = dIoReadReg (REG_SEQD,REG_8);
				vIoWriteReg (REG_SEQD,REG_8, reg | 0x01);
			}
			else if ((dDidVid == DIDVID_VIDEOi7)		   ||
					 (dDidVid == DIDVID_VIDEO_HASWELL)	   ||
					 (dDidVid == DIDVID_VIDEO_HASWELL_CELERON)	   ||
					 (dDidVid == DIDVID_VIDEO_SANDYBRIDGE) ||
					 (dDidVid == DIDVID_VIDEO_IVYBRIDGE))
			{
				rt = E__OK;
				dTemp = PCI_READ_DWORD(pfa, 0x18) & 0xfffffff0;
				return (rt);
			}
			else if (dDidVid == DID_VIDEO_ATI)
			{
				rt = E__OK;
				dTemp = PCI_READ_DWORD(pfa, 0x10) & 0xfffffff0;
				//return (rt);
			}
			else
			{
				dTemp = PCI_READ_DWORD(pfa, 0x18) & 0xfffffff0;
			}

			#ifdef DEBUG
				sprintf(buffer, "Mem BAR 0: 0x%x \n", dTemp);
				vConsoleWrite(buffer); 
			#endif

			if(dTemp != NULL)
			{
				dHandle1 = dGetPhysPtr (dTemp, 0x10000, &p1, &ptr);
				if(dHandle1 == E__FAIL)
				{
					#ifdef DEBUG
						vConsoleWrite("Unable to allocate BAR Mem \n"); 
					#endif
					rt = E__VIDEO_MEM;
				}
			}
			else
				rt = E__VIDEO_MEM;
		}


		if(rt == E__OK)
		{

			*((UINT32*)ptr+4096) = 0xabcdabcd;
			dTemp = *((UINT32*)ptr+4096);
			if( dTemp == 0xabcdabcd)
			{
				*(((UINT32*)ptr)+4097) = 0x12341234;
				dTemp = *(((UINT32*)ptr)+4097);
				if( dTemp != 0x12341234)
				{
					sprintf(buffer,"Written 0x12341234, read 0x%x ",dTemp);
					vConsoleWrite(buffer);
					rt = E__VIDEO_MEM;
				}
			}
			else
			{
				sprintf(buffer,"Written 0xabcdabcd, read 0x%x ",dTemp);
									vConsoleWrite(buffer);
				rt = E__VIDEO_MEM;
			}
		}
	}
	else
		rt = E__TEST_NO_DEVICE;

	/* restore nonlinear addressing */
	if (dDidVid == DIDVID_VIDEOSM722)
	{
		vIoWriteReg (REG_SEQX, REG_8, IREG_SCR18);
		reg = dIoReadReg (REG_SEQD,REG_8);
		vIoWriteReg (REG_SEQD,REG_8, reg & ~0x01);
	}

	vFreePtr(dHandle1);
	return (rt);
}

