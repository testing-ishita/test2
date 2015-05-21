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

/*
 * SPI based EEPROM access spieepromtest.c
 * 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/spieepromtest.c,v 1.3 2013-11-25 12:53:01 mgostling Exp $
 * $Log: spieepromtest.c,v $
 * Revision 1.3  2013-11-25 12:53:01  mgostling
 * Added missing CVS headers.
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

//#define DEBUG

#define WAIT_CLK_HIGH	sysDelayMicroseconds (5)
#define WAIT_CLK_LOW	sysDelayMicroseconds (7)

#define EE_CLK 0x04
#define EE_DATA_IN 0x01
#define EE_DATA_OUT 0x02
#define EE_CS 0x08
#define EE_MUX_EE 0x10
#define EE_MUX_PLX 0xEF

#define DATA_LO 0xFD
#define DATA_HI 0x02

#define EE_DAT_SH_IN 0
#define EE_DAT_SH 1

#define CLK_LO 0xFB
#define CLK_HI 0x04

#define CS_LO 0xF7
#define CS_HI 0x08

#define EEPROM_OFF 0x800

#define SPI_RD    0x03
#define SPI_WR    0x02
#define SPI_WREN  0x06
#define SPI_RDSR  0x05

#define SPI_EEPROM 0x9000

#define SPI_EEPROM_WRITE_TIMEOUT_ERROR	(E__BIT + SPI_EEPROM + 0x01)
#define SPI_EEPROM_DATA_MISMATCH_ERROR  (E__BIT + SPI_EEPROM + 0x02)


static SPIEEPROM_INFO *pSpiEEPROMData;
extern SPIEEPROM_INFO *spiBrdEEPROMInfo (void);

/*****************************************************************************\
 *
 *  TITLE:  spiEEPROMInit ()
 *
 *  ABSTRACT:  This function initialize the spi interface and gets the offset of 
 *				the SPI status and control register.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiEEPROMInit (void)
{
	board_service(SERVICE__BRD_GET_SPI_EEPROM_INFO, NULL, &pSpiEEPROMData);

}

/*****************************************************************************\
 *
 *  TITLE:  spiCsHigh ()
 *
 *  ABSTRACT:  This function sets the chipselect signal high for reading/writing 
 *				SPI EEPROM.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiCsHigh (void)
{
	UINT32 bTemp = 0;
#ifdef DEBUG
	unsigned char achBuffer[80];
#endif

	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);

#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Before CS High:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
	sysDelayMicroseconds (1);
	bTemp |= CS_HI;
	vIoWriteReg (pSpiEEPROMData->EEPROMCntrl,REG_8,bTemp);
	sysDelayMicroseconds (1);


#ifdef DEBUG
	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);
	sprintf ( achBuffer, "\nSPI After CS High:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
}


/*****************************************************************************\
 *
 *  TITLE:  spiCsLow ()
 *
 *  ABSTRACT:  This function sets the chipselect signal Low for reading/writing 
 *				SPI EEPROM
 *
 * 	RETURNS: None
 *
\*****************************************************************************/

void spiCsLow (void)
{
    UINT32 bTemp = 0;
#ifdef DEBUG
	unsigned char achBuffer[80];
#endif



	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl,REG_8);

#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Before CS Low:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
	sysDelayMicroseconds (1);
	bTemp &= CS_LO;
	vIoWriteReg (pSpiEEPROMData->EEPROMCntrl,REG_8,bTemp);
	sysDelayMicroseconds (1);

#ifdef DEBUG
	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);
	sprintf ( achBuffer, "\nSPI After CS Low:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
}

/*****************************************************************************\
 *
 *  TITLE:  spiClkHigh ()
 *
 *  ABSTRACT:  This function sets the Clock signal High for reading/writing 
 *			    SPI EEPROM.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiClkHigh (void)
{
	UINT32 bTemp = 0;
#ifdef DEBUG
	unsigned char achBuffer[80];
#endif


	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl,REG_8);
	sysDelayMicroseconds (1);
#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Before CLK High:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
	bTemp |= CLK_HI;
	vIoWriteReg (pSpiEEPROMData->EEPROMCntrl,REG_8,bTemp);
	sysDelayMicroseconds (1);
#ifdef DEBUG
	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);
	sprintf ( achBuffer, "\nSPI After CLK High:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
}

/*****************************************************************************\
 *
 *  TITLE:  spiClkLow ()
 *
 *  ABSTRACT:  This function sets the Clock signal Low for reading/writing 
 *			   SPI EEPROM.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiClkLow (void)
{
	
	UINT32 bTemp = 0;
#ifdef DEBUG
	unsigned char achBuffer[80];
#endif


	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl,REG_8);
#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Before CLK Low:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
	sysDelayMicroseconds (1);
	bTemp &= CLK_LO;
	vIoWriteReg (pSpiEEPROMData->EEPROMCntrl,REG_8,bTemp);
	sysDelayMicroseconds (1);
#ifdef DEBUG
	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);
	sprintf ( achBuffer, "\nSPI After CLK Low:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
}


/*****************************************************************************\
 *
 *  TITLE:  spiDataHigh ()
 *
 *  ABSTRACT: This function sets the Data signal High for reading/writing 
 *			   SPI EEPROM..
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiDataHigh (void)
{
	UINT32 bTemp = 0;
#ifdef DEBUG
	unsigned char achBuffer[80];
#endif



	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl,REG_8);
#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Data High:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
	sysDelayMicroseconds (1);
	bTemp |= DATA_HI;
	vIoWriteReg (pSpiEEPROMData->EEPROMCntrl,REG_8,bTemp);
	sysDelayMicroseconds (1);

#ifdef DEBUG
	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);
	sprintf ( achBuffer, "\nSPI After Data High:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
}

/*****************************************************************************\
 *
 *  TITLE:  spiDataLow ()
 *
 *  ABSTRACT:  This function sets the Data signal Low for reading/writing 
 *			   SPI EEPROM.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiDataLow (void)
{
	UINT32 bTemp = 0;
#ifdef DEBUG
	unsigned char achBuffer[80];
#endif

	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl,REG_8);
	sysDelayMicroseconds (1);
#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Data Low:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
	bTemp &= DATA_LO;
	vIoWriteReg (pSpiEEPROMData->EEPROMCntrl,REG_8,bTemp);
	sysDelayMicroseconds (2);
#ifdef DEBUG
	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);
	sprintf ( achBuffer, "\nSPI After Data Low:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
}


/*****************************************************************************\
 *
 *  TITLE:  spiEESwitch ()
 *
 *  ABSTRACT:  This function selects Muxing of EEPROM between PLX and BANK 2 SPI 
 *				EEPROM.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiEESwitch (void)
{
	UINT32 bTemp = 0;
#ifdef DEBUG
	unsigned char achBuffer[80];
#endif

	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl,REG_8);
	sysDelayMicroseconds (1);
#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Before EE Switch:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
	bTemp |= EE_MUX_EE;
	vIoWriteReg (pSpiEEPROMData->EEPROMCntrl,REG_8,bTemp);
	sysDelayMicroseconds (1);
#ifdef DEBUG
	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);
	sprintf ( achBuffer, "\nSPI After EE Switch:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
}

/*****************************************************************************\
 *
 *  TITLE:  spiPLXSwitch ()
 *
 *  ABSTRACT:  This function selects Muxing of EEPROM between PLX and BANK 2 SPI 
 *				EEPROM.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiPLXSwitch (void)
{

	UINT32 bTemp = 0;
#ifdef DEBUG
	unsigned char achBuffer[80];
#endif

	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl,REG_8);
	sysDelayMicroseconds (1);
#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Before PLX Switch:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
	bTemp &= EE_MUX_PLX;
	vIoWriteReg (pSpiEEPROMData->EEPROMCntrl,REG_8,bTemp);
	sysDelayMicroseconds (1);
#ifdef DEBUG
	bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl, REG_8);
	sprintf ( achBuffer, "\nSPI After PLX Switch:%#x \n",bTemp);
	vConsoleWrite (achBuffer);
#endif
}



/*****************************************************************************\
 *
 *  TITLE:  spiByteSend ()
 *
 *  ABSTRACT:  This function sends a Byte to SPI EEPROM
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiByteSend (UINT8 bByte_to_Send)
{
	int index = 0;  // bit position counter 0x80 -> 0x40 -> ... -> 0x01

	spiClkLow ();

	for (index = 7; index >= 0; index--)
	{

		if (bByte_to_Send & (0x1 << index))
		{
			spiDataHigh ();
		}
		else
		{
			spiDataLow ();
		}
		spiClkHigh ();
		spiClkLow ();
		spiDataLow ();
	}
}

/*****************************************************************************\
 *
 *  TITLE:  spiByteGet ()
 *
 *  ABSTRACT:  This function gets a byte from the SPI Interface.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
UINT8 spiByteGet (void)
{
	int index = 0;  // bit position counter 0x80 -> 0x40 -> ... -> 0x01
	unsigned char X = 0;      // value being read from HW to return to caller
	UINT32 bTemp = 0;

	X = 0;

	spiClkLow ();
	for(index = 7; index >= 0; index--)
	{

		spiClkHigh ();
		WAIT_CLK_HIGH;
		bTemp = dIoReadReg (pSpiEEPROMData->EEPROMCntrl,REG_8);
		sysDelayMicroseconds (1);
		bTemp = bTemp & EE_DATA_IN;

		{
			X |= (bTemp << index);
		}

		spiClkLow ();
	}

	return X;
}



/*****************************************************************************\
 *
 *  TITLE:  spiReadByte ()
 *
 *  ABSTRACT:  This function Reads a byte from the SPI EEPROM at a given offset.
 *
 * 	RETURNS: Data from SPI EEPROM (bData).
 *
\*****************************************************************************/
UINT8 spiReadByte (UINT16 wOffset)
{ 

	UINT8 bByteOffset = 0;
	unsigned char bData = 0;
	
	spiCsHigh ();
	spiEESwitch ();
	spiCsHigh ();


	spiCsLow ();
	spiByteSend(SPI_RD);
	wOffset = wOffset + EEPROM_OFF;
	bByteOffset = ((wOffset >> 8) & 0xFF);
	spiByteSend (bByteOffset);
    bByteOffset = ((wOffset));
	spiByteSend (bByteOffset);

    bData = spiByteGet ();
	spiCsHigh ();
	spiPLXSwitch ();

	return (bData);

}

/*****************************************************************************\
 *
 *  TITLE:  waitDevReady ()
 *
 *  ABSTRACT:  This function reads the SPI Status in a loop till the SPI device
 *				gets ready after finished writing the data.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
UINT32 waitDevReady (void)
{
	UINT8 bData = 0;
	UINT32 timeout = 10;


	do {
			spiCsLow ();
			spiByteSend (SPI_RDSR);
			bData = spiByteGet ();
			spiCsHigh ();
			sysDelayMicroseconds (1000);
	} while ( (bData & 0x01) && (timeout--));

	if (timeout == 0)
	{
		spiPLXSwitch();
		return SPI_EEPROM_WRITE_TIMEOUT_ERROR;
	}
	else
	{
		 spiPLXSwitch();
		return E__OK;
	}
}

/*****************************************************************************\
 *
 *  TITLE:  spiWriteByte ()
 *
 *  ABSTRACT:  This function writes a byte to a SPI EEPROM with a given data and
 *				offset.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
UINT32 spiWriteByte (UINT8 bData, UINT16 wOffset)
{
	UINT8 bByteOffset = 0;
	UINT32 time = 1;
#ifdef DEBUG
	char	 achBuffer[128];
#endif

	spiCsHigh ();
	spiEESwitch ();
	spiCsHigh ();
	spiCsLow ();

	spiByteSend (SPI_WREN);
	spiCsHigh ();
	spiCsLow ();
	spiByteSend (SPI_WR);

	wOffset = wOffset + EEPROM_OFF;
	bByteOffset =  (wOffset >> 8) & 0xFF;
	spiByteSend (bByteOffset);
    bByteOffset =  (wOffset);
	spiByteSend (bByteOffset);

	spiByteSend (bData);
	spiCsHigh ();
	sysDelayMicroseconds( time );

	if (waitDevReady () != E__OK)
	{
#ifdef DEBUG
			sprintf ( achBuffer, "\nTime Out While Writing Data\n");
			vConsoleWrite (achBuffer);
#endif
			return (SPI_EEPROM_WRITE_TIMEOUT_ERROR);
	}

	return (E__OK);

}



/*****************************************************************************\
 *
 *  TITLE:  SpiEEPROMTest ()
 *
 *  ABSTRACT:  This function tests the SPI EEPROM by writing and reading a pattern 
 *			   to memory offseet. 
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
TEST_INTERFACE (SpiEEPROMTest, "SPI EEPROM Test")
{
	UINT32   rt = E__OK, err;
#ifdef DEBUG
	char	 achBuffer[128];
#endif
	UINT16	 i = 0;
	UINT8	 temp = 0, temp2 = 0;
	UINT8	 pattern = 0;

	/*
	 *  Initialise the SPI EEPROM
	 */
	spiEEPROMInit ();

#ifdef DEBUG
	sprintf ( achBuffer, "\nSPI Control:%#x \n",pSpiEEPROMData->EEPROMCntrl);
	vConsoleWrite (achBuffer);
#endif


	i       = 0x00;
	pattern = 0x01;
	err     = E__OK;

	while( (i < 0x10) && (err == E__OK) )
	{

		temp = spiReadByte(i);

#ifdef DEBUG
				sprintf ( achBuffer, "\ntemp %#x, Idx %#x\n",temp, i);
				vConsoleWrite (achBuffer);
#endif

		if (spiWriteByte(pattern,i) != E__OK)
		{
			err = SPI_EEPROM_WRITE_TIMEOUT_ERROR;
			rt = SPI_EEPROM_WRITE_TIMEOUT_ERROR;
		}

		vDelay (10);
		temp2 = spiReadByte(i);

#ifdef DEBUG
		sprintf ( achBuffer, "\ntemp %d, Idx %d, Data 0x%x, Expected 0x%x\n",temp, i, temp2, pattern);
		vConsoleWrite (achBuffer);
#endif

		if(temp2 != pattern)
		{
			rt = SPI_EEPROM_DATA_MISMATCH_ERROR;
			err = SPI_EEPROM_DATA_MISMATCH_ERROR;
		}
		else
		{
			 rt = E__OK;
		}

		pattern = pattern << 1;
		if(pattern == 0)
			pattern = 0x01;
				
		if (spiWriteByte(temp,i) != E__OK)
		{
			rt = SPI_EEPROM_WRITE_TIMEOUT_ERROR;
			err = SPI_EEPROM_WRITE_TIMEOUT_ERROR;
		}
		vDelay (10);
		i++;
	}
	return (rt);
}




