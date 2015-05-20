/******************************************************************************
*
* Filename:     spiFwh.c
*
* Description:  This file contains the test for SPI based firmware hubs
*
*
* Copyright 2000-2009 Concurrent Technologies.
*
******************************************************************************/
/* spiFwh.c : SPI firmware Hub related tests.*/

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/spiFwh.c,v 1.5 2015-04-01 16:50:18 hchalla Exp $
 * $Log: spiFwh.c,v $
 * Revision 1.5  2015-04-01 16:50:18  hchalla
 * Added unlock and locking of CCT Hidden IO registers.
 *
 * Revision 1.4  2014-09-19 09:35:27  mgostling
 * Added bitBackupSpiFwhTest()
 *
 * Revision 1.3  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments.
 * This tag was also designed to allow search+sort operations that output a list of functions and test names; 
 * if the tag is used in comments then we get spurious lines included in this list.
 *
 * Revision 1.2  2013-11-25 12:54:09  mgostling
 * Added support for NUMONYX_N25Q064A on TRB1x.
 *
 * Revision 1.1  2013-09-04 07:46:45  chippisley
 * Import files into new source repository.
 *
 */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/bit.h>
 
#include <bit/console.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/board_service.h>

//#define DEBUG

#define SST_VENDORID            0x00BF
#define ATMEL_VENDORID          0x001F
#define NUMONYX_VENDORID        0x0020
#define SST25VF032A_DEVICEID    0x4A25
#define SST25VF032B_DEVICEID    0x4B25
#define SST25VF064C_DEVICEID    0x4B25
#define AT25DF641_DEVICEID    	0x0048
#define NUMONYX_N25Q064A		0x17BA


#define SPI_FLASH_WRITE_TIMEOUT 100

#define INTEL_LPC               0x1F
#define ICH9M_SPIBAR            0x3800
#define RCBA_REG_OFFSET         0xF0
#define RCBA_LEN                0x4000
#define SPI_RCBA_MASK           0xFFFFC000

#define INTEL_ICH9M             0x2917
#define INTEL_IBEX_PEAK			0x3B07
#define INTEL_COUGAR_POINT      0x1C4F
#define INTEL_PANTHER_POINT     0x1E55
#define INTEL_LYNX_POINT     	0x8C4F

#define FADDR            0x08
#define FDATA0           0x10
#define SSFSTS           0x90
#define SSFCTL           0x91
#define OPTYPE           0x96
#define OPMENU0          0x98
#define OPMENU1          0x9c

enum{
    WRITE_BYTE=0,
    READ_BYTE,
    DEVICE_ID,
    ENABLE_WRITE,
    BLOCK_ERASE,
    ENABLE_WSR,
    WRITE_SR,
    READ_SR
};

typedef struct _spiDevInfo
{
        UINT16 vendorId;        /*Vendor ID */
        UINT16 deviceId;        /*Device ID */
        UINT16 opType;          /*Describe the type of opcode */
        UINT32 opCodeMenu0;     /*Supported Opcode menu 0*/
        UINT32 opCodeMenu1;     /*Supported Opcode menu 1*/
}SPIDEVINFO;

static SPIDEVINFO spiDevices [] = {
	{ 
		SST_VENDORID,
		SST25VF032A_DEVICEID,
		0x174B,
		0x069f0302,
		0x05015020,
	},
	{
		SST_VENDORID,
		SST25VF032B_DEVICEID,
		0x174B,
		0x069f0302,
		0x05015020,
	},
	{
		ATMEL_VENDORID,
		AT25DF641_DEVICEID,
		0x174B,
		0x069f0302,
		0x05015020,
	},
	{
		NUMONYX_VENDORID,
		NUMONYX_N25Q064A,
		0x174B,
		0x069f0302,
		0x05015020,
	},
	
	{ 0,0,0,0,0 }
};

#define osWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define osWriteWord(regAddr,value)  (*(UINT16*)(regAddr) = value)
#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define osReadByte(regAddr)			(*(UINT8*)(regAddr))
#define osReadWord(regAddr)			(*(UINT16*)(regAddr))
#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))

#define E__CHIPSET_NF           E__BIT + 0x1
#define E__ROM_DETECT           E__BIT + 0x2
#define E__TEST_NO_MEM			E__BIT + 0x3


/*****************************************************************************\
 *
 *  TITLE:  spiProbeDevice ()
 *
 *  ABSTRACT:  The function performs the SPI FWH test by reading the flash ID and
* 			   checking with the supported device list
 *
 * 	RETURNS: 0 on success else error code
 *
\*****************************************************************************/
static UINT32 spiProbeDevice (void)
{
	UINT32		regVal;
	UINT32		result = E__ROM_DETECT;
	UINT32		i = 0;
	UINT32		mHandle;
	UINT32		timeout;
	volatile	UINT32 rcba;
	volatile	UINT32 rcbaBaseAddr;
	PCI_PFA		pfa;
	PTR48		tPtr;

#ifdef DEBUG
	char	achBuffer[80];
#endif

	/*Detect the chipset*/
	pfa = PCI_MAKE_PFA (0, INTEL_LPC, 0);
	regVal = PCI_READ_DWORD (pfa, RCBA_REG_OFFSET);

	if (regVal == 0xFFFFFFFF)
	{
		return E__CHIPSET_NF;
	}

	regVal &= SPI_RCBA_MASK;

	mHandle = dGetPhysPtr (regVal, RCBA_LEN, &tPtr, (void*)&rcba);

	if (result == E__FAIL)
	{
		return E__TEST_NO_MEM;
	}

	rcbaBaseAddr = (UINT32)(rcba + ICH9M_SPIBAR);
	
	/*Lets now probe for the supported devices*/	
	
	while (spiDevices[i].vendorId != 0)
	{
		/*Do some cleanup */
		osWriteByte ((rcbaBaseAddr + SSFSTS), 0x0f);	 		

		/*Set the opcode menu and type*/
		osWriteWord ((rcbaBaseAddr + OPTYPE), spiDevices[i].opType);
		osWriteDWord ((rcbaBaseAddr + OPMENU0), spiDevices[i].opCodeMenu0);
		osWriteDWord ((rcbaBaseAddr + OPMENU1), spiDevices[i].opCodeMenu1);

		/*Make sure there is no command in progress*/
		regVal = osReadByte (rcbaBaseAddr + SSFSTS);
		
		if( regVal & 0x1 )
		{
#ifdef DEBUG
			vConsoleWrite ("\ncommand in progress!\n");
#endif
			vFreePtr (mHandle);
			return E__ROM_DETECT;
		}

		/*Clear any unwanted bits*/
		if ((osReadByte (rcbaBaseAddr + SSFSTS)) & 0xc)
		{
			osWriteByte ((rcbaBaseAddr + SSFSTS), 0xc);
		}

		osWriteDWord ((rcbaBaseAddr + FADDR), 0);
		osWriteDWord ((rcbaBaseAddr + FDATA0), 0);

		/*Configure and set the control register*/
		regVal = (DEVICE_ID & 0x7) << 4;
		regVal |= 2 << 8;  /* DBC    */
		regVal |= 0x4000;  /* DS = 1 */
		regVal |= 0x2;

		osWriteDWord ((rcbaBaseAddr + SSFSTS), (regVal << 8));
		
		/*Wait for the command to complete execution*/
		timeout = SPI_FLASH_WRITE_TIMEOUT;

		while (timeout--)
		{
			regVal = osReadByte (rcbaBaseAddr + SSFSTS);

			if (regVal & 0x4)
			{
				break;
			}

			if (regVal & 0x8)
			{
#ifdef DEBUG
				vConsoleWrite ("\nregVal & 0x8\n");
#endif
				osWriteByte ((rcbaBaseAddr + SSFSTS), 0xc);
				vFreePtr (mHandle);
				return E__ROM_DETECT;
			}
	
			vDelay (10); //Was 5 Hari:06/01/2011
		}

		/*Clear out the status bits*/
		osWriteByte ((rcbaBaseAddr+SSFSTS), 0xc);
			
		if (timeout == 0)
		{
#ifdef DEBUG
			vConsoleWrite ("\ntimeout!\n");
#endif
			vFreePtr (mHandle);
			return E__ROM_DETECT;
		}

		regVal = (osReadDWord (rcbaBaseAddr + FDATA0));
		if (((spiDevices[i].deviceId << 8) | (spiDevices[i].vendorId)) == regVal)
		{
#ifdef DEBUG
			sprintf (achBuffer, "\nOK, ID = %x\n", regVal);
			vConsoleWrite (achBuffer);
#endif
			result = E__OK;
			break;
		}
		else
		{
#ifdef DEBUG
			sprintf (achBuffer, "\nWrong ID, ID = %x\n", regVal);
			vConsoleWrite (achBuffer);
#endif
			result = E__ROM_DETECT;
		}

		i++;
	}

	vFreePtr (mHandle);
	return result;
}  /*spiProbeDevice() */


/*****************************************************************************\
 *
 *  TITLE:  bitSpiFwhTest
 *
 *  ABSTRACT:  The function performs the SPI FWH test by reading the flash ID and
 *             checking with the supported device list.
 *
 * 	RETURNS: 0 on success else error code
 *
\*****************************************************************************/
TEST_INTERFACE (bitSpiFwhTest, "SPI ROM Device ID test")
{
	UINT32	result;
	UINT16	regVal;
	PCI_PFA	pfa;

#ifdef DEBUG
	UINT8 achBuffer[64];
#endif

	/*Detect the chipset*/
	pfa = PCI_MAKE_PFA (0, INTEL_LPC, 0);
	regVal = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

	if (regVal == 0xFFFF)
	{
		return E__CHIPSET_NF;
	}

	switch (regVal)
	{
		case INTEL_ICH9M:
		{
#ifdef DEBUG
			sprintf (achBuffer, "INTEL_ICH9M\n");
			vConsoleWrite (achBuffer);
#endif
			result = spiProbeDevice();
			break;
		}

		case INTEL_IBEX_PEAK:
		case INTEL_COUGAR_POINT:
		{
#ifdef DEBUG
			if (regVal == INTEL_IBEX_PEAK)
			{
				sprintf (achBuffer, "INTEL_IBEX_PEAK\n");
			}
			else
			{
				sprintf (achBuffer, "INTEL_COUGAR_POINT\n");
			}
			vConsoleWrite (achBuffer);
#endif
			result = spiProbeDevice();
			break;
		}

		case INTEL_PANTHER_POINT:
		{
#ifdef DEBUG
			sprintf (achBuffer, "INTEL_PANTHER_POINT\n");
			vConsoleWrite (achBuffer);
#endif
			result = spiProbeDevice();
			break;
		}

		case INTEL_LYNX_POINT:
		{
#ifdef DEBUG
			sprintf (achBuffer, "INTEL_LYNX_POINT\n");
			vConsoleWrite (achBuffer);
#endif
			result = spiProbeDevice();
			break;
		}

		default:
		{
			result = E__CHIPSET_NF;
			break;
		}
	}

	return result;
} /* bitSpiFwhTest () */


/*****************************************************************************\
 *
 *  TITLE:  bitSpiFwhTest
 *
 *  ABSTRACT:  The function performs the SPI FWH test by reading the flash ID and
 *             checking with the supported device list.
 *
 * 	RETURNS: 0 on success else error code
 *
\*****************************************************************************/
TEST_INTERFACE (bitBackupSpiFwhTest, "Backup SPI ROM Device ID test")
{
	UINT32	result;
	UINT16	regVal;
	PCI_PFA	pfa;
	UINT8	backupDev;

#ifdef DEBUG
	UINT8 achBuffer[64];
#endif

	board_service(SERVICE__BRD_UNLOCK_CCT_IOREGS, NULL,NULL);
	/*Detect the chipset*/
	pfa = PCI_MAKE_PFA (0, INTEL_LPC, 0);
	regVal = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

	if (regVal == 0xFFFF)
	{
		return E__CHIPSET_NF;
	}

	backupDev = 1;				//select backup SPI FWH
	board_service(SERVICE__BRD_SELECT_SPI_FWH, &result, (void *) &backupDev);

	if (result == E__OK)
	{
		switch (regVal)
		{
			case INTEL_ICH9M:
			{
#ifdef DEBUG
				sprintf (achBuffer, "INTEL_ICH9M\n");
				vConsoleWrite (achBuffer);
#endif
				result = spiProbeDevice();
				break;
			}

			case INTEL_IBEX_PEAK:
			case INTEL_COUGAR_POINT:
			{
#ifdef DEBUG
				if (regVal == INTEL_IBEX_PEAK)
				{
					sprintf (achBuffer, "INTEL_IBEX_PEAK\n");
				}
				else
				{
					sprintf (achBuffer, "INTEL_COUGAR_POINT\n");
				}
				vConsoleWrite (achBuffer);
#endif
				result = spiProbeDevice();
				break;
			}

			case INTEL_PANTHER_POINT:
			{
#ifdef DEBUG
				sprintf (achBuffer, "INTEL_PANTHER_POINT\n");
				vConsoleWrite (achBuffer);
#endif
				result = spiProbeDevice();
				break;
			}

			case INTEL_LYNX_POINT:
			{
#ifdef DEBUG
				sprintf (achBuffer, "INTEL_LYNX_POINT\n");
				vConsoleWrite (achBuffer);
#endif
				result = spiProbeDevice();
				break;
			}

			default:
			{
				result = E__CHIPSET_NF;
				break;
			}
		}

		backupDev = 0;				// restore default setting
		board_service(SERVICE__BRD_SELECT_SPI_FWH, &result, (void *) &backupDev);
	}

	board_service(SERVICE__BRD_LOCK_CCT_IOREGS, NULL,NULL);

	return result;
} /* bitBackupSpiFwhTest () */
