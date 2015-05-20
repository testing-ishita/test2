
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

/* mirrorflash.c - test mirror flash devices
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/mirrorflash.c,v 1.2 2013-10-08 07:13:38 chippisley Exp $
 *
 * $Log: mirrorflash.c,v $
 * Revision 1.2  2013-10-08 07:13:38  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.3  2010/11/04 17:53:04  hchalla
 * Fixed bugs in VP 417 Application flash erase, and reading of the flash.
 *
 * Revision 1.2  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.1  2009/09/29 12:31:15  swilson
 * Application Flash test for Mirror Flash device.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <string.h>
#include <errors.h>

#include <bit/bit.h>
#include <bit/mem.h>
#include <bit/io.h>
#include <bit/console.h>
#include <bit/delay.h>
 
#include <bit/board_service.h>

#include <devices/mirrorflash.h>

/* defines */

#define LAST_PAGE			(0xFFFFFFFF)			/* page select function will mask bits */

#define FLASH_BUFFER_SIZE	32						/* flash write buffer size in bytes */

	/* Error codes */

#define E__NO_DEVICE		(E__BIT + 0x00000001)
#define E__MEM_ALLOC1		(E__BIT + 0x00000002)
#define E__MEM_ALLOC2		(E__BIT + 0x00000003)
#define E__MEM_ALLOC3		(E__BIT + 0x00000004)
#define E__WRONG_KEY		(E__BIT + 0x00000010)
#define E__TIMEOUT_1		(E__BIT + 0x00000011)
#define E__TIMEOUT_2		(E__BIT + 0x00000012)
#define E__UNKNOWN_DEVICE	(E__BIT + 0x00000020)
#define E__ERASE_ERROR 		(E__BIT + 0x00000030)
#define E__WRITE_ERROR 		(E__BIT + 0x00000040)
#define E__VERIFY_ERROR 	(E__BIT + 0x00000050)
#define E__DATA_MISMATCH	(E__BIT + 0x00000060)
#define E__WRONG_SECTOR		(E__BIT + 0x00000070)


/* typedefs */

typedef struct tagFlashId
{
	UINT32	dManufId;
	union
	{
		UINT32	dDevId;
		UINT8	abDevId[4];
	};

} FLASH_ID;

/* constants */

/* locals */

static UINT32	dEraseKey = 0;

/* globals */

/* externals */

/* forward declarations */

static UINT32	dKeyGen (void);
static UINT32	dDetectDevice (AF_PARAMS** ppsAppFlash);
static UINT32	dEraseSector (UINT32 dPage, UINT32 dOffset, PTR48 pMem);
static void		vReadFromSector (UINT32 dPage, UINT32 dOffset, UINT32 dLength, PTR48 pMem, PTR48 pBuffer);
static UINT32	dWriteToSector (UINT32 dPage, UINT32 dOffset, UINT32 dLength, PTR48 pMem, PTR48 pBuffer);
static void		vCreateTestPattern (UINT32 dLength, PTR48 pDest);
static UINT32	dCompareBuffer (UINT32	dPage, UINT32 dLength, PTR48 pReference, PTR48 pOriginal, PTR48 pData);

static UINT32	verifyErase (UINT32	dPage, UINT32 dLength, PTR48 pData);

/*****************************************************************************
 * MirrorFlashAccessTest: test access to Mirror Flash device
 *
 * RETURNS: None
 */

TEST_INTERFACE (MirrorFlashAccessTest, "Application Flash Access Test")
{
	AF_PARAMS*	psAppFlash;
	UINT32	dStatus;
	char	achBuffer[80];


	/* Do a device detect */

	dStatus = dDetectDevice (&psAppFlash);

	if (dStatus == E__OK)
	{
		sprintf (achBuffer, "Found: %s\n", psAppFlash->achName);
		vConsoleWrite (achBuffer);
	}

	return (dStatus);

} /* MirrorFlashAccessTest () */


/***************************************************************************** 
 * MirrorFlashRwTest: non-destructive R/W test on mirror flash device
 *
 * By default, we can will use the last secor of the device. Normally this 
 * would have XSA firmware installed, but with BIT code present XSA is
 * inaccessible.
 *
 * RETURNS: None
 */

static UINT32 currentSector = 0;

TEST_INTERFACE (MirrorFlashRwTest, "Application Flash R/W Test")
{
	AF_PARAMS*	psAppFlash;
	PTR48	pMem;
	PTR48	pOrigBuff;
	PTR48	pTestBuff;
	UINT32	dStatus;
	UINT32	dOffset;
	UINT32	dPage = 0;
	UINT32	dHandle1, dHandle2, dHandle3;
	UINT32	dTemp;
	UINT32  	count;
	char		achBuffer[80];

	/* Check that device is present */

	dStatus = dDetectDevice (&psAppFlash);

	if (dStatus != E__OK)
		return (dStatus);

	/* Get a pointer for access to the App. Flash */

	dHandle1 = dGetPhysPtr (psAppFlash->dPageBase, psAppFlash->dPageSize, &pMem, &dTemp);

	if (dHandle1 == E__FAIL)
	{
		vConsoleWrite ("Mem Allocation Failed\n");
		vFreePtr (dHandle1);
		return (E__MEM_ALLOC1);
	}


	/* Get two sector-sized buffers */
	dHandle2 = dGetBufferPtr (psAppFlash->dSectorSize, &pOrigBuff, &dTemp);
	if (dHandle2 == E__FAIL)
	{
		vConsoleWrite ("Buffer 1 Allocation Failed\n\n");
		vFreePtr (dHandle1);
		vFreePtr (dHandle2);
		return (E__MEM_ALLOC2);
	}


	dHandle3 = dGetBufferPtr (psAppFlash->dSectorSize, &pTestBuff, &dTemp);
	if (dHandle3 == E__FAIL)
	{
		vConsoleWrite ("Buffer 2 Allocation Failed\n\n");
		vFreePtr (dHandle1);
		vFreePtr (dHandle2);
		vFreePtr (dHandle3);
		return (E__MEM_ALLOC3);
	}


	if ((adTestParams[0] == 0) )
	{
		count = 1;

		if(currentSector > psAppFlash->dNumSectors-1 )
			currentSector = 0;

		dPage = currentSector;
		currentSector++;
	}
	else if ((adTestParams[0] == 1) )
	{
		count = 1;
		if( (adTestParams[1] >= 0) && ( adTestParams[1] < psAppFlash->dNumSectors) )
			dPage   = adTestParams[1];
		else
			dStatus = E__WRONG_SECTOR;
	}
	else if ((adTestParams[0] == 2) )
	{
		if( (adTestParams[1] == 0) && ( adTestParams[2] == 1) )
		{
			count = psAppFlash->dNumSectors;
			dPage = 0;
		}
		else
			dStatus = E__WRONG_SECTOR;
	}
	else
		dStatus = E__WRONG_SECTOR;



	/* Get parameters for this test */
	dOffset = 0;

	vConsoleWrite ("\n");
	while( (count > 0) && (dStatus == E__OK) )
	{
		sprintf (achBuffer, "\rTesting Sector %d ", dPage);
		vConsoleWrite (achBuffer);

		/* Read existing sector contents */
		vReadFromSector (dPage, dOffset, psAppFlash->dSectorSize, pMem, pOrigBuff);

		/* Erase the sector */
		if (dStatus == E__OK)
		{
			dStatus = dEraseSector (dPage*2, dOffset, pMem);
			dStatus = dEraseSector ((dPage*2)+1, dOffset, pMem); /*Hari Added */
		}

		if(dStatus == E__OK)
			dStatus = verifyErase (dPage, psAppFlash->dSectorSize, pMem);

		/* Write a test pattern */
		if (dStatus == E__OK)
		{
			vCreateTestPattern (psAppFlash->dSectorSize, pTestBuff);
			dStatus = dWriteToSector (dPage, dOffset, psAppFlash->dSectorSize, pMem, pTestBuff);
			if(dStatus != E__OK)
			{
				vConsoleWrite ("\nWrite Test Pattern Fail\n");
				return (dStatus);
			}
		}

		/* Confirm test pattern */
		if (dStatus == E__OK)
			dStatus = dCompareBuffer (dPage, psAppFlash->dSectorSize, pTestBuff, pOrigBuff, pMem);


		if (dStatus != E__OK)
			return (dStatus);
		/* Restore the original contents - attempt this even if there was a failure above */
		/* Erase the sector */
		dStatus = dEraseSector (dPage*2, dOffset, pMem);
		if (dStatus != E__OK)
				return (dStatus);
		dStatus = dEraseSector ((dPage*2)+1, dOffset, pMem); /*Hari Added */
		if(dStatus == E__OK)
			dStatus = verifyErase (dPage, psAppFlash->dSectorSize, pMem);
		else
			return (dStatus);


		if (dStatus == E__OK)
		{
			dStatus = dWriteToSector (dPage, dOffset, psAppFlash->dSectorSize, pMem, pOrigBuff);
			if(dStatus != E__OK)
			{
				vConsoleWrite ("\nWrite Original Data Fail\n");
				return (dStatus);
			}
		}
		else
		{
			return (dStatus);
		}

		if (dStatus == E__OK)
		{
			dStatus = dCompareBuffer (dPage, psAppFlash->dSectorSize, pOrigBuff, pTestBuff, pMem );
		}
		else
		{
					return (dStatus);
		}

		count--;
		dPage++;
	}
	vConsoleWrite ("\n");
	/* Clean up */
	vFreePtr (dHandle1);
	vFreePtr (dHandle2);
	vFreePtr (dHandle3);

	/* If first write/compare part of test failed, return that error code,
	 * else use overall test result.
	 */

	return (dStatus);

} /* MirrorFlashRwTest () */


/*****************************************************************************
 * MirrorFlashErase: Erase entire Mirror Flash device
 *
 * RETURNS: None
 */

TEST_INTERFACE (MirrorFlashErase, "Application Flash Erase Utility")
{
	AF_PARAMS*	psAppFlash;
	PTR48	pMem;
	UINT32	dStatus;
	UINT32	dSector;
	UINT32	dOffset;
	UINT32	dHandle;
	UINT32	dTemp;
	UINT32	dPage;
	// UINT32	nSectsPerPage;
	char	achBuffer[80];


	/* No params - supply a 'random' erase key */

	if ((adTestParams[0] == 0) || (dEraseKey == 0))
	{
		vConsoleWrite ("** Warning **\n\n");
		vConsoleWrite ("This test erases the entire Application Flash. If you wish to continue, please\n");
		vConsoleWrite ("re-run the test with the following parameter: ");

		dEraseKey = dKeyGen () & 0xFF;	/* only use LSB */

		sprintf (achBuffer, "%d\n\n", dEraseKey);
		vConsoleWrite (achBuffer);

		return (E__OK);
	}

	/* Check the supplied key */

	if (adTestParams[1] != dEraseKey)
	{
		sprintf (achBuffer, "supplied %02X, expected %02X\n\n", adTestParams[1], dEraseKey);
		vConsoleWrite (achBuffer);

		vConsoleWrite ("You supplied the wrong erase key, please re-run the test to obtain a new key.\n");
		dEraseKey = 0;

		return (E__WRONG_KEY);
	}

	dEraseKey = 0;			/* Zero the key: it is use-once to prevent accidents */

	/* Check that device is present */

	dStatus = dDetectDevice (&psAppFlash);

	if (dStatus != E__OK)
		return (dStatus);

	/* Get a pointer for access to the App. Flash */

	dHandle = dGetPhysPtr (psAppFlash->dPageBase, psAppFlash->dPageSize, &pMem, &dTemp);

	if (dHandle == E__FAIL)
		return (E__MEM_ALLOC1);

	/*
	 * Erase the entire application flash
	 */

    // 	nSectsPerPage = 1; psAppFlash->dPageSize / psAppFlash->dSectorSize;
	vConsoleWrite ("\n");
	for (dSector = 0; (dStatus == E__OK) && (dSector < psAppFlash->dNumSectors); dSector++)
	{
		sprintf (achBuffer, "\rErasing Sector %d of %d", dSector, psAppFlash->dNumSectors);
		vConsoleWrite (achBuffer);

		/* Calculate page and offset where this sector is located */
	
		dPage = dSector *2;// nSectsPerPage;
		dOffset  = 0;//(dSector % nSectsPerPage) * psAppFlash->dSectorSize;

		dStatus = dEraseSector (dPage, dOffset, pMem);
		dStatus = dEraseSector (dPage+1, dOffset, pMem); /*Hari*/

		if(dStatus== E__OK)
			dStatus = verifyErase (dPage/2, psAppFlash->dSectorSize, pMem);
	}
	vConsoleWrite ("\n");

	/* Clean up */

	vFreePtr (dHandle);

	return (dStatus);

} /* MirrorFlashErase () */


/*****************************************************************************
 * dKeyGen: generate a pseudo-random key value
 *
 * RETURNS: the key
 */

static UINT32	dKeyGen (void)
{
	int		s;
	int		m;
	int		h;

	/* Get current time */

	vIoWriteReg (0x70, REG_8, 0x00);
	s = (UINT8)dIoReadReg (0x71, REG_8);

	vIoWriteReg (0x70, REG_8, 0x02);
	m = (UINT8)dIoReadReg (0x71, REG_8);

	vIoWriteReg (0x70, REG_8, 0x04);
	h = (UINT8)dIoReadReg (0x71, REG_8);

	/* Create a time-stamp to be used as a key */

	return (h * m * s) + ((h - s) * m) + ((h - m) * s) + s;

} /* dKeyGen () */


/*****************************************************************************
 * dDetectDevice: detect a supported device and get its parameters
 *
 * RETURNS: E__OK or E__... error code
 */

static UINT32 dDetectDevice
(
	AF_PARAMS**	ppsAppFlash
)
{
	AF_PARAMS*	psAppFlash;
	FLASH_ID	sFlashId;
	PTR48	pMem;
	UINT32	dHandle;
	UINT32	dTemp;
	char	achBuffer[80];
	int		i;
	UINT32 page;
	*ppsAppFlash = NULL;

	/* Get a pointer to the supported types */

	board_service(SERVICE__BRD_GET_APPFLASH_PARAMS, NULL, &psAppFlash);

	if ((psAppFlash == NULL) || (psAppFlash->dSectorSize == 0))
		return (E__NO_DEVICE);

	/* Get a pointer for access to the App. Flash */

	dHandle = dGetPhysPtr (psAppFlash->dPageBase, psAppFlash->dPageSize, &pMem, &dTemp);

	if (dHandle == E__FAIL)
		return (E__MEM_ALLOC1);

	/* Put Flash in 'read identifier' mode */

	page = 0;/* Select the first page */
	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);

	_fMemWriteByte (pMem + 0xAAA, 0xAA);
	_fMemWriteByte (pMem + 0x555, 0x55);
	_fMemWriteByte (pMem + 0xAAA, F_IDENT);

	/* Get manufacturer ID */

	sFlashId.dManufId = _fMemReadByte (pMem + 0);

	/* Get device ID */

	sFlashId.dDevId = 0;

	sFlashId.abDevId[0] = _fMemReadByte (pMem + 0x1E);
	sFlashId.abDevId[1] = _fMemReadByte (pMem + 0x1C);
	sFlashId.abDevId[2] = _fMemReadByte (pMem + 2);

	/* Reset Flash to read mode */

	_fMemWriteByte (pMem, F_RESET);
	
	vFreePtr (dHandle);

	/* Test to see if device is recognised */

	i = 0;

	while (psAppFlash[i].dSectorSize != 0)
	{
		if ( (psAppFlash[i].dManuf == sFlashId.dManufId) &&
			 (psAppFlash[i].dDevice == sFlashId.dDevId) )
		{
			*ppsAppFlash = psAppFlash;
			return (E__OK);
		}

		i++;
	}

	/* Failed to find the device */

	sprintf (achBuffer, "Device identify failed: MID = %02X, DID = %06X\n",
				sFlashId.dManufId, sFlashId.dDevId);
	vConsoleWrite (achBuffer);

	return (E__UNKNOWN_DEVICE);

} /* dDetectDevice () */


/*****************************************************************************
 * dEraseSector: erase a single device sector
 *
 * RETURNS: E__OK or E__... error code
 */

static UINT32 dEraseSector
(
	UINT32	dPage,		/* page where sector located */
	UINT32	dOffset,	/* offset within page to sector */
	PTR48	pMem		/* pointer to App. Flash window */
)
{
	UINT16	wTimeout = 1000;		/* 100ms total timeout */
	UINT8	bTemp;
	char	achBuffer[80];
	UINT32 page;

	/* Set-up the block erase */
	page = 0;/* Select the first page */
	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);

	_fMemWriteByte (pMem, F_STATUS_RESET);		/* Reset the status register */

	_fMemWriteByte (pMem + 0xAAA, 0xAA);
	_fMemWriteByte (pMem + 0x555, 0x55);
	_fMemWriteByte (pMem + 0xAAA, F_ERASE_SETUP);

	_fMemWriteByte (pMem + 0xAAA, 0xAA);
	_fMemWriteByte (pMem + 0x555, 0x55);

	/* Erase the selected block */

	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &dPage);

	_fMemWriteByte (pMem + dOffset, F_ERASE ) ;


	/* Wait for the program/erase cycle to start */

	bTemp = _fMemReadByte (pMem + dOffset);

	while ( ((bTemp & 0x08) == 0) && (wTimeout > 0) )
	{
		vDelay (1);
		wTimeout--;

		bTemp = _fMemReadByte (pMem + dOffset);
	}

	if (wTimeout == 0)
	{
		vConsoleWrite ("Erase Cycle Start Failed\n");
		return (E__TIMEOUT_1);
	}

	/* Wait for the program/erase cycle to complete */

	while ( ((bTemp & 0x80) == 0) && (wTimeout > 0) )
	{
		vDelay (1);
		wTimeout--;

		bTemp = _fMemReadByte (pMem + dOffset);
	}

	_fMemWriteByte (pMem + dOffset, F_RESET);

	if (wTimeout == 0)
	{
		sprintf (achBuffer, "Erase Sector %d Failed\n", dPage);
		vConsoleWrite (achBuffer);
		return (E__TIMEOUT_2);
	}

	return (E__OK);

} /* dEraseSector () */


/*****************************************************************************
 * vReadFromSector: 
 *
 * RETURNS: E__OK or E__... error code
 */

static void vReadFromSector 
(
	UINT32	dPage, 
	UINT32	dOffset, 
	UINT32	dLength,
	PTR48	pMem, 
	PTR48	pBuffer
)
{
	UINT32  tpage;

	tpage = dPage*2;
	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &tpage);
	_fmemcpy (pBuffer, pMem + dOffset, 0x10000);

	tpage++;
	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &tpage);
	_fmemcpy (pBuffer+0x10000, pMem + dOffset, 0x10000);
	
} /* vReadFromSector () */


/*****************************************************************************
 * dWriteToSector: 
 *
 * RETURNS: E__OK or E__... error code
 */

static UINT32 dWriteToSector
(
	UINT32	dPage, 
	UINT32	dOffset, 
	UINT32	dLength,
	PTR48	pMem,
	PTR48	pBuffer
)
{
	UINT32	dIndex, tindex, tpage,dRetVal = E__OK;
	UINT16	bBlockSize;
	UINT8	bTemp;
	UINT8	bData;
	int		i;
	UINT32 page;

	/* Reset the status register */

	page = 0;/* Select the first page */
	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);
	_fMemWriteByte (pMem, F_STATUS_RESET);

	/* Write the buffer to flash */
	dIndex = 0;
	while (dIndex < dLength)
	{
		/* Set number of bytes written in this block */

		//if ((dLength - dIndex) > FLASH_BUFFER_SIZE)
			bBlockSize = FLASH_BUFFER_SIZE;
		//else
		//	bBlockSize = (UINT8)(dLength - dIndex);
		
		/* Set up write command */

		page = 0;/* Select the first page */
		board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);

		_fMemWriteByte (pMem + 0xAAA, 0xAA);
		_fMemWriteByte (pMem + 0x555, 0x55);

		if(dIndex >= 0x10000)
		{
			tindex = dIndex-0x10000;
			tpage  = (dPage*2)+1;
			board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &tpage);
		}
		else
		{
			tindex = dIndex;
			tpage  = dPage*2;
			board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &tpage);
		}


		_fMemWriteByte (pMem + tindex, F_WRITE_BUFFER);		/* Write to Buffer command */
		_fMemWriteByte (pMem + tindex, bBlockSize - 1);		/* word count */
		
		/* Write bytes to flash buffer */
		
		for (i = 0; i < bBlockSize; i++)
		{
			bTemp = _fMemReadByte (pBuffer + dIndex + i);
			_fMemWriteByte (pMem + tindex+i, bTemp);
		}

		tindex += (i-1);
		dIndex += (i - 1);		/* set index to last byte of block */

		_fMemWriteByte (pMem + tindex, F_CONFIRM);
		
		/* Check for error from status */

		bData = _fMemReadByte (pBuffer + tindex);

		do 
		{
			bTemp = _fMemReadByte (pMem + tindex);
			if ((bTemp & 0x20) != 0)	/* bit 5 = timeout */
			{
				/* On timeout check data one more time */
				bTemp = _fMemReadByte (pMem + tindex); /*Hari */
				bData = _fMemReadByte (pBuffer + tindex); /*Hari*/
				if ((bTemp & 0x80) != (bData & 0x80))
				{
					return (E__TIMEOUT_1);
				}
			}
		} while ((bTemp & 0x80) != (bData & 0x80));

		_fMemWriteByte (pMem + tindex, F_RESET);			/* clear status */

		dIndex++;		/* move to next block */
	}

	/* Reset back to Read Mode */

	page = 0;/* Select the first page */
	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);
	_fMemWriteByte (pMem, F_RESET);

	return (dRetVal);

} /* dWriteToSector () */


/*****************************************************************************
 * vCreateTestPattern: 
 *
 * RETURNS: E__OK or E__... error code
 */

static void vCreateTestPattern 
(
	UINT32	dLength,
	PTR48	pDest
)
{
	UINT32	dIndex;
	UINT32	dTemp;
	UINT32	dBitRev;
	int		i;

	UINT32	ad[] = { 0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
					 0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF };

	for (dIndex = 0; dIndex < dLength; dIndex += 4)
	{
		dTemp = dIndex;
		dBitRev = 0;

		for (i = 0; i < 8; i++)
		{
			dBitRev |= ad[dTemp & 0xF];
			dTemp >>= 4;
			dBitRev <<= 4;
		}

		_fMemWriteDword (pDest + dIndex, dIndex | dBitRev);
	}

} /* vCreateTestPattern () */


/*****************************************************************************
 * dCompareBuffer: 
 *
 * RETURNS: E__OK or E__... error code
 */

static UINT32 dCompareBuffer
(
	UINT32	dPage,
	UINT32	dLength,
	PTR48	pReference,
	PTR48   pOriginal,
	PTR48	pData
)
{
	UINT32		dIndex;
	UINT32		dData1;
	UINT32		dData2;
	UINT32      dOrig;
	char		achBuffer[80];
	UINT32		page;

	/* Set-up the block erase */
	page = 0;/* Select the first page */
	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);

	_fMemWriteByte (pData, F_STATUS_RESET);		/* Reset the status register */

	/* DWORD compare of buffer contents */

	for (dIndex = 0; dIndex < dLength; dIndex += 4)
	{
		dData1 = _fMemReadDword (pReference + dIndex);
		dOrig  = _fMemReadDword (pOriginal  + dIndex);

		if(dIndex >= 0x10000)
		{
			page = ((dPage*2)+1);
			board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);
			dData2 = _fMemReadDword (pData + (dIndex-0x10000));
		}
		else
		{
			page = (dPage*2);
			board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);
			dData2 = _fMemReadDword (pData + dIndex);
		}

		if (dData1 != dData2)
		{
			sprintf (achBuffer, "Data error at offset 0x%X: expected 0x%08X, read 0x%08X, Org 0x%08X\n",
					 dIndex, dData1, dData2, dOrig);
			vConsoleWrite (achBuffer);

			return (E__DATA_MISMATCH);
		}
	}
	return (E__OK);

} /* dCompareBuffer () */


static UINT32	verifyErase (UINT32	dPage, UINT32 dLength, PTR48 pData)
{
	UINT32		dIndex;
	UINT32		dData2;
	char		achBuffer[80];
	UINT32      page;

	/* Set-up the block erase */
	page = 0;/* Select the first page */
	board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);

	_fMemWriteByte (pData, F_RESET);		/* Reset the status register */

	/* DWORD compare of buffer contents */

	for (dIndex = 0; dIndex < dLength; dIndex += 4)
	{

		if(dIndex >=0x10000)
		{
			page = ((dPage*2)+1);
			board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);
			dData2 = _fMemReadDword (pData + (dIndex-0x10000));
		}
		else
		{
			page = (dPage*2);
			board_service(SERVICE__BRD_SELECT_APPFLASH_PAGE, NULL, &page);
			dData2 = _fMemReadDword (pData + dIndex);
		}

		if(dData2 != 0xffffffff)
		{
			sprintf (achBuffer, "**Data Error** after Erase at offset 0x%X: 0x%08X\n", dIndex, dData2);
			vConsoleWrite (achBuffer);

			return (E__ERASE_ERROR);
		}

	}

	return (E__OK);

}
