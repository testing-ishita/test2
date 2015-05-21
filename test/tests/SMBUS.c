
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

/* SMBUS.c - LED Test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/SMBUS.c,v 1.6 2015-02-25 18:02:20 hchalla Exp $
 * $Log: SMBUS.c,v $
 * Revision 1.6  2015-02-25 18:02:20  hchalla
 * Removed unwanted code and debugs.
 *
 * Revision 1.5  2015-01-29 10:20:26  mgostling
 * Added Thermal Sensor Test for VP91x.
 *
 * Revision 1.4  2014-10-15 11:49:41  mgostling
 * Updated tsi721SmbusTest to verify GPIO settings match with fitted devices.
 *
 * Revision 1.3  2014-09-19 09:31:39  mgostling
 * Added tsi721SmbusTest()
 *
 * Revision 1.2  2013-11-25 12:50:13  mgostling
 * Added ClkGenSI52143Test for TRB1x
 *
 * Revision 1.1  2013-09-04 07:46:45  chippisley
 * Import files into new source repository.
 *
 * Revision 1.12  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.11  2011/03/22 14:00:50  hchalla
 * Added support for new LM95245 device test.
 *
 * Revision 1.10  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.9  2010/12/03 09:52:45  jthiru
 * Removed ck505 test from VP717 PBIT/IBIT - Affects board reset
 *
 * Revision 1.8  2010/10/25 15:38:28  jthiru
 * Cosmetic changes
 *
 * Revision 1.7  2010/09/15 12:25:15  hchalla
 * Modified tests to  support TR 501 BIT/CUTE.
 *
 * Revision 1.6  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.5  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.4  2009/06/10 14:45:11  hmuneer
 * smbus and eeprom update
 *
 * Revision 1.3  2009/06/08 09:55:58  hmuneer
 * TPA40
 *
 * Revision 1.2  2009/05/29 14:07:27  hmuneer
 * Error Codes Fixed
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

#define LM95245_MANUF_ID   0x01
#define ADT7461_MANUF_ID   0x41
#define LM95245_REV_ID     0xB3

#define THERM_DIODE_OFFSET 		0x0000003F

#define SMBUS_REG_ERROR		(E__BIT  +0x01)
#define SMBUS_INFO_ERROR	(E__BIT  +0x02)

#define CK505_SMBUS_ERROR	(E__BIT  +0x01)
#define CK505_ID_ERROR		(E__BIT  +0x02)

#define CK409_SMBUS_ERROR	(E__BIT  +0x01)
#define CK409_ID_ERROR		(E__BIT  +0x02)

#define DB800_SMBUS_ERROR	(E__BIT  +0x01)
#define DB800_ID_ERROR		(E__BIT  +0x02)

#define SI52143_SMBUS_ERROR	(E__BIT  +0x01)
#define SI52143_ID_ERROR	(E__BIT  +0x02)

#define DS3905_SMBUS_ERROR	(E__BIT  +0x01)
#define DS3905_1_ID_ERROR	(E__BIT  +0x02)
#define DS3905_2_ID_ERROR	(E__BIT  +0x03)
#define DS3905_3_ID_ERROR	(E__BIT  +0x04)
#define DS3905_DEV_ERROR    (E__BIT +0x05)
#define DS3905_BUS_ERROR    (E__BIT +0x06)

#define TSI721_INFO_ERROR	(E__BIT  +0x03)
#define TSI721_PARAM_ERROR	(E__BIT  +0x04)
#define TSI721_SMBUS_ERROR	(E__BIT  +0x05)
#define TSI721_EEPROM_ERROR	(E__BIT  +0x06)

#define	SMB_BDATA			0x08		/* Data xfr - Byte*/
#define	SMB_WDATA			0x0C		/* Data xfr - Word*/

/***************************/

 #define	SMB_BYTE			0x04		/* ? */

/****************************/

	/* Errors */

#define	E__SMB_READ_FAIL	(E__BIT  +0x01)
#define	E__SMB_READ_CLSN	(E__BIT  +0x02)
#define	E__SMB_READ_DERR	(E__BIT  +0x03)
#define	E__TIMEOUT			(E__BIT  +0x04)

#define	E__SMB_BAD_ADDR		(E__BIT  +0x05)
#define	E__SMB_BAD_TYPE		(E__BIT  +0x06)
#define	E__UNKNOWN			(E__BIT  +0x07)
#define	E__SMB_BAD_PARAM	(E__BIT  +0x08)
#define E__REMOTE_OPEN      (E__BIT  +0x09)
#define E__REMOTE_SHORT	    (E__BIT  +0x0A)
#define E__REMOTE_VCC       (E__BIT  +0x0B)
#define LM95245_SMBUS_ERROR	(E__BIT  +0x01)


/* Read only registers */

#define	MX_CMD_LT			0x00		/* Local Temp */
#define	MX_CMD_RTHB			0x01		/* Remote Temp High Byte */
#define	MX_CMD_SR			0x02		/* Status Register */
#define MX_CMD_RTLB			0x10		/* Read Remote Temp Low Byte */

/* Read registers */

#define MX_CMD_RD_C			0x03		/* Read Configuration */
#define	MX_CMD_RD_CR		0x04		/* Read Conversion Rate */
#define	MX_CMD_RD_LHS		0x05		/* Read Local High Setpoint */
#define	MX_CMD_RD_LLS		0x06		/* Read Local Low Setpoint */
#define	MX_CMD_RD_RHSHB		0x07		/* Read Remote High Setpoint High Byte*/
#define	MX_CMD_RD_RLSHB		0x08		/* Read Remote Low Setpoint High Byte*/

/* typedefs */
/* constants */
/* locals */

static SMBUS_INFO *smbusInfo;
static DS3905_INFO *ds3905Info;
static EEPROM_INFO *eepromInfo;

/* globals */

UINT16	 ManufId,RevId;

UINT32 wSMBusHRead
(
	UINT8	bAddr,		/* 7 bit address */
	UINT8	bCmd,		/* Command to send */
	UINT16*	pwData,		/* For Data read */
	UINT8	bType,		/* Type of xfr  Ie. SMB_BDATA or SMB_WDATA */
	UINT16	wTimeout	/* Timeout in 10mS blocks */
);

UINT32	wSMBusExec(
	UINT16	wTimeout								/* Timeout in 10mS blocks */
);


UINT8 readTemp(void);
static UINT32 wMxFaultCheck (void);
static UINT32 wMxReadTempLR (void);
static UINT32 checkTsi721 (SRIO_DEVICE_SMBUS_ADDRESS *srioDevice);

/*****************************************************************************
 * wReg8BitTest: this function performs a bit-wise test on the specified 
 * 8-bit register. A marching 1/0 pattern is used, a mask value specifies
 * which bits are included in the test.
 * RETURNS : E__OK or E__... error code */
UINT32 wReg8BitSMBUSTest( UINT16	dPortAddress, UINT8	bBitMask)
{
	UINT8   Backup;
	UINT8	bFixed;			/* fixed bit pattern (of those outside the mask) */
	UINT8	bPattern;		/* current test bit-pattern */
	UINT8	bBitNum;		/* register bit number */
	UINT8	bTemp;			/* like it says! */

	#ifdef DEBUG 
		char	achBuffer[80];	/* text formatting buffer */
	#endif

	Backup = ((UINT8)dIoReadReg(dPortAddress, REG_8));
	bFixed = ((UINT8)dIoReadReg(dPortAddress, REG_8)) & ~bBitMask;

	for (bBitNum = 0; bBitNum < 8; bBitNum++)
	{
		bPattern = ((1 << bBitNum) & bBitMask);		/* Marching '1' */
		vIoWriteReg(dPortAddress, REG_8, bPattern | bFixed);
		bTemp = ((UINT8)dIoReadReg(dPortAddress, REG_8)) & bBitMask;

		if (bTemp != bPattern)
		{	
			vIoWriteReg(dPortAddress, REG_8, Backup);				
			
			#ifdef DEBUG
				sprintf (achBuffer, "Data error (1)[Addr:0x%x]: wrote %02Xh, read %02Xh",
						 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
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
				sprintf (achBuffer, "Data error (0)[Addr:0x%x]: wrote %02Xh, read %02Xh",
					 	 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}
	}
	
	vIoWriteReg(dPortAddress, REG_8, Backup);
	return (E__OK);

} /* wReg8BitTest () */


/*****************************************************************************
 * SMBUSTest: Test the EEPROM
 *
 * RETURNS: None
 */
TEST_INTERFACE (SMBUSTest, "SMBUS Device Test")
{
	UINT32   rt = E__OK;
		
	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);
	if(smbusInfo == NULL)
		return SMBUS_INFO_ERROR;

	if(wReg8BitSMBUSTest( smbusInfo->GPIOBase + smbusInfo->reg1, smbusInfo->mask1) != E__OK)
		rt = SMBUS_REG_ERROR;
	else if(wReg8BitSMBUSTest( smbusInfo->GPIOBase + smbusInfo->reg2, smbusInfo->mask2) != E__OK) 
		rt = SMBUS_REG_ERROR;

	return (rt);
}

/*****************************************************************************
 * checkTsi721: Check a particular TSI721 SRIO bridge and its associated eeprom
 *
 * RETURNS: E___OK or error code
 */
static UINT32 checkTsi721 (SRIO_DEVICE_SMBUS_ADDRESS *srioDevice)
{
	UINT32	rt = E__OK;
	UINT32	timeOut;
	volatile UINT8	status;

	// check the TSI721
	vIoWriteReg (smbusInfo->GPIOBase + 0x00, REG_8, 0x7E);						// clear status flags
	vIoWriteReg (smbusInfo->GPIOBase + 0x04, REG_8, srioDevice->srioAddr | 1);	// set device address
	vIoWriteReg (smbusInfo->GPIOBase + 0x03, REG_8, 0x20);						// set device command: read LSB of status register
	vIoWriteReg (smbusInfo->GPIOBase + 0x02, REG_8, 0x48);						// start SMBUS Byte Data protocol

	status = dIoReadReg (smbusInfo->GPIOBase + 0x00, REG_8);
	vDelay (1);
	timeOut = 1000;
	while ((status & 0x01) != 0x00 && (timeOut > 0))					// wait for operation to complete or timeout
	{
		vDelay (1);
		status = dIoReadReg (smbusInfo->GPIOBase + 0x00, REG_8);
		--timeOut;
	}

	if ((status & 0x1C) != 0)
	{
#ifdef DEBUG
		sprintf ( achBuffer, "\nReg %d, status 0x%x, timeOut %d \t", 3, status, timeOut);
		vConsoleWrite (achBuffer);
#endif
		rt = TSI721_SMBUS_ERROR;
	}

	// check the TSI721 associated eeprom
	if (rt == E__OK)
	{
		vIoWriteReg (smbusInfo->GPIOBase + 0x00, REG_8, 0x7E);							// clear status flags
		vIoWriteReg (smbusInfo->GPIOBase + 0x04, REG_8, srioDevice->eepromAddr | 1);	// set device address
		vIoWriteReg (smbusInfo->GPIOBase + 0x03, REG_8, 0x00);
		vIoWriteReg (smbusInfo->GPIOBase + 0x02, REG_8, 0x40);							// start SMBUS Quick protocol

		status = dIoReadReg (smbusInfo->GPIOBase + 0x00, REG_8);
		vDelay (1);
		timeOut = 1000;
		while((status & 0x01) != 0x00 && (timeOut > 0))				// wait for operation to complete or timeout
		{
			vDelay (1);
			status = dIoReadReg (smbusInfo->GPIOBase + 0x00, REG_8);
			--timeOut;
		}

		if ((status & 0x1C) != 0)
		{
#ifdef DEBUG
			sprintf ( achBuffer, "\nReg %d, status 0x%x, timeOut %d \t", 3, status, timeOut);
			vConsoleWrite (achBuffer);
#endif
			rt = TSI721_EEPROM_ERROR;
		}
	}
	return (rt);
}


/*****************************************************************************
 * tsi721SmbusTest: Check the TSI721 SRIO bridges and their associated eeproms
 *
 * RETURNS: E___OK or error code
 */
TEST_INTERFACE (tsi721SmbusTest, "TSI721 SMBus Interface Test")
{
	UINT32							rt = E__OK;
	UINT8							index;
	UINT8							devCnt;
	SRIO_DEVICE_SMBUS_INFO *		srio;
	SRIO_DEVICE_SMBUS_ADDRESS *		srioDevice;

#ifdef DEBUG
	char	 achBuffer[128];
#endif

	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);
	board_service(SERVICE__BRD_GET_SRIO_INFO, NULL, &srio);

	if (smbusInfo == NULL)
		return SMBUS_INFO_ERROR;

	if (srio == NULL)
		return TSI721_INFO_ERROR;

	devCnt = srio->numDev;

	if (adTestParams[0] == 0)					// test all devices on the board
	{
		srioDevice = srio->srioDeviceList;
		for (index = 0; index < devCnt; ++index)
		{
			rt = checkTsi721 (&srioDevice[index]);
			if (rt != E__OK)
			{
				// straps report device as fitted?
				if ((srio->devFitted & (1 << index)) != 0)
				{
					// device failed to respond
					rt |= ((index + 1) << 8);		// set device specific error code
					break;
				}
				else
				{
					// device did not respond and it is not reported as fitted
					rt = E__OK;
				}
			}
			else
			{
				// straps report device as not fitted?
				if ((srio->devFitted & (1 << index)) == 0)
				{
					// device responded
					rt |= ((index + 1) << 8);		// set device specific error code
					break;
				}
				else
				{
					// device responded and it is reported as fitted
					rt = E__OK;
				}
			}
		}
	}
	else
	{
		if (adTestParams[1] > devCnt)			// 1 based counter
		{
			return TSI721_PARAM_ERROR;
		}
		else
		{
			srioDevice = srio->srioDeviceList;	 // 0 based index
			rt = checkTsi721 (&srioDevice[adTestParams[1] - 1]);
			if (rt != E__OK)
			{
				rt |= (adTestParams[1] << 8);	// set device specific error code
			}
		}
	}

	return rt;
}


/*****************************************************************************
 * ClkGenSI52143Test: Test the SI52143 Clock
 *
 * RETURNS: None
 */
TEST_INTERFACE (CKSI52143Test, "SI52143 Clock Gen Test")
{
	UINT32   rt = E__OK, tm;
	UINT8	 temp;

#ifdef DEBUG
	char	 achBuffer[128];
#endif

	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);

	if(smbusInfo == NULL)
		return SMBUS_INFO_ERROR;

	vIoWriteReg( smbusInfo->GPIOBase + 0x00, REG_8, 0x1E );		// clear status flags
	vIoWriteReg( smbusInfo->GPIOBase + 0x04, REG_8, 0xD7 );		// set device address
	
	/*
	 * Add 0x80 to register number as command byte to read specific register
	 * Not specified in the data sheet for the device!!!
	 */
	
	vIoWriteReg( smbusInfo->GPIOBase + 0x03, REG_8, 0x83 );		// set device command: read control register 3
	vIoWriteReg( smbusInfo->GPIOBase + 0x02, REG_8, 0x48 );		// start SMBUS Byte Data protocol
	temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);
	vDelay(1);
	tm = 1000;
	while((temp&0x01)==0x01 && (tm>0))							// wait for operation to complete or timeout
	{
		vDelay(1);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);	
		tm--;
	}
	
	#ifdef DEBUG
		sprintf ( achBuffer, "\nReg %d, Status 0x%x, tm %d \t", 3, temp, tm);	
		vConsoleWrite (achBuffer);	
	#endif

	if(tm == 0)
	{
		rt = SI52143_SMBUS_ERROR;
	}
	else
	{
		vDelay(100);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x05, REG_8);	// read response from device
		#ifdef DEBUG
			sprintf ( achBuffer, "Data 0x%x\t", temp);	
			vConsoleWrite (achBuffer);	
		#endif	
		if((temp & 0x0F) != 0x08)			 					// should report vendor ID of 0x08
			rt = SI52143_ID_ERROR;
	}
	
	return (rt);
}



/*****************************************************************************
 * CK505Test: Test the EEPROM
 *
 * RETURNS: None
 */
TEST_INTERFACE (CK505Test, "CK505 Device Test")
{
	UINT32   rt = E__OK, tm;
	UINT16	  temp = 0;

	#ifdef DEBUG
		char	 achBuffer[128];
	#endif


	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);

	if(smbusInfo == NULL)
		return SMBUS_INFO_ERROR;

	board_service(SERVICE__BRD_GET_EEPROM_INFO, NULL, &eepromInfo);
	temp = dIoReadReg(eepromInfo->GPIOBase + 0x0C, REG_16);

	if ( (temp & 0x100) == 0x100 )
	{
	vIoWriteReg( smbusInfo->GPIOBase + 0x00, REG_8, 0x1E );
	vIoWriteReg( smbusInfo->GPIOBase + 0x04, REG_8, 0xD3 );
	vIoWriteReg( smbusInfo->GPIOBase + 0x03, REG_8, 0x87 );
	vIoWriteReg( smbusInfo->GPIOBase + 0x02, REG_8, 0x48 );
	temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);
	vDelay(1);
	tm = 1000;
	while((temp&0x01)==0x01 && (tm>0))
	{
		vDelay(1);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);	
		tm--;
	}
	
	#ifdef DEBUG
		sprintf ( achBuffer, "\n GPIO Base: 0x%x Reg %d, Status 0x%x, tm %d \t",
				smbusInfo->GPIOBase, 7, temp, tm);
		vConsoleWrite (achBuffer);	
	#endif

	if(tm == 0)
	{
		rt = CK409_SMBUS_ERROR;
	}
	else
	{
		vDelay(100);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x05, REG_8);
		#ifdef DEBUG
			sprintf ( achBuffer, "Data 0x%x\t", temp);	
			vConsoleWrite (achBuffer);	
		#endif	

		if(temp != 0x11)
			rt = CK409_ID_ERROR;
	}
		return (rt);
	}
	else
	{		
		vConsoleWrite ("CK 505 Device is not Applicable for this Board\n");
		return (E__OK);
	}
}



/*****************************************************************************
 * CK409Test: Test the EEPROM
 *
 * RETURNS: None
 */
TEST_INTERFACE (CK409Test, "CK409 Device Test")
{
	UINT32   rt = E__OK, tm;
	UINT8	 temp;

#ifdef DEBUG
	char	 achBuffer[128];
#endif

	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);

	if(smbusInfo == NULL)
		return SMBUS_INFO_ERROR;	
		
	vIoWriteReg( smbusInfo->GPIOBase + 0x00, REG_8, 0x1E );
	vIoWriteReg( smbusInfo->GPIOBase + 0x04, REG_8, 0xD3 );
	vIoWriteReg( smbusInfo->GPIOBase + 0x03, REG_8, 0x87 );
	vIoWriteReg( smbusInfo->GPIOBase + 0x02, REG_8, 0x48 );
	temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);
	vDelay(1);
	tm = 1000;
	while((temp&0x01)==0x01 && (tm>0))
	{
		vDelay(1);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);	
		tm--;
	}
	
	#ifdef DEBUG
		sprintf ( achBuffer, "\nReg %d, Status 0x%x, tm %d \t", 7, temp, tm);	
		vConsoleWrite (achBuffer);	
	#endif

	if(tm == 0)
	{
		rt = CK409_SMBUS_ERROR;
	}
	else
	{
		vDelay(100);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x05, REG_8);
		#ifdef DEBUG
			sprintf ( achBuffer, "Data 0x%x\t", temp);	
			vConsoleWrite (achBuffer);	
		#endif	

		if(temp != 0x31)
			rt = CK409_ID_ERROR;
	}
	
	return (rt);
}



/*****************************************************************************
 * DB800Test: Test the EEPROM
 *
 * RETURNS: None
 */
TEST_INTERFACE (DB800Test, "DB800 Device Test")
{
	UINT32   rt = E__OK, tm;
	UINT8	 temp;


#ifdef DEBUG
	char	 achBuffer[128];
#endif

	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);
	if(smbusInfo == NULL)
		return SMBUS_INFO_ERROR;
		
	vIoWriteReg( smbusInfo->GPIOBase + 0x00, REG_8, 0x1E );
	vIoWriteReg( smbusInfo->GPIOBase + 0x04, REG_8, 0xDD );
	vIoWriteReg( smbusInfo->GPIOBase + 0x03, REG_8, 62   );
	vIoWriteReg( smbusInfo->GPIOBase + 0x02, REG_8, 0x48 );
	temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);
	vDelay(1);
	tm = 1000;
	while((temp&0x01)==0x01 && (tm>0))
	{
		vDelay(1);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);	
		tm--;
	}

	#ifdef DEBUG
		sprintf ( achBuffer, "\nReg %d, Status 0x%x, tm %d \t", 62, temp, tm);	
		vConsoleWrite (achBuffer);	
	#endif

	if(tm == 0)
	{
		rt = DB800_SMBUS_ERROR;
	}
	else
	{
		vDelay(100);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x05, REG_8);
		#ifdef DEBUG
			sprintf ( achBuffer, "Data 0x%x\t", temp);	
			vConsoleWrite (achBuffer);	
		#endif	

		if(temp != 0x05)
			rt = DB800_ID_ERROR;
	}

	return (rt);
}




TEST_INTERFACE (DB400Test, "DB400 Device Test")
{
	UINT32   rt = E__OK, tm;
	UINT8	 temp;


#ifdef DEBUG
	char	 achBuffer[128];
#endif

	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);
	if(smbusInfo == NULL)
		return SMBUS_INFO_ERROR;
		
	vIoWriteReg( smbusInfo->GPIOBase + 0x00, REG_8, 0x1E );
	vIoWriteReg( smbusInfo->GPIOBase + 0x04, REG_8, 0xDD );
	vIoWriteReg( smbusInfo->GPIOBase + 0x03, REG_8, 62 );
	vIoWriteReg( smbusInfo->GPIOBase + 0x02, REG_8, 0x48 );
	temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);
	vDelay(1);
	tm = 1000;
	while((temp&0x01)==0x01 && (tm>0))
	{
		vDelay(1);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x00, REG_8);	
		tm--;
	}

	#ifdef DEBUG
		sprintf ( achBuffer, "\nReg %d, Status 0x%x, tm %d \t", 62, temp, tm);	
		vConsoleWrite (achBuffer);	
	#endif

	if(tm == 0)
	{
		rt = DB800_SMBUS_ERROR;
	}
	else
	{
		vDelay(100);
		temp = dIoReadReg(smbusInfo->GPIOBase + 0x05, REG_8);
		#ifdef DEBUG
			sprintf ( achBuffer, "Data 0x%x\t", temp);	
			vConsoleWrite (achBuffer);	
		#endif	

		if(temp != 0x05)
			rt = DB800_ID_ERROR;
	}

	return (rt);
}



#define HOST_STATUS				0x00
#define	HOST_CONTROL			0X02
#define	HOST_COMMAND			0X03
#define TRANSMIT_SLAVE_ADDRESS	0X04
#define	HOST_DATA_0				0X05
#define	HOST_DATA_1				0X06

UINT32 writeReg(UINT8 addr,UINT8 reg,UINT8  data)
{
	UINT32   rt = E__OK, tm;
	UINT8	 temp;

	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);
	if (smbusInfo == NULL)
		return SMBUS_INFO_ERROR;

	vIoWriteReg( smbusInfo->GPIOBase + HOST_STATUS, REG_8, 0x1E );
	vIoWriteReg( smbusInfo->GPIOBase + TRANSMIT_SLAVE_ADDRESS, REG_8, addr );
	vIoWriteReg( smbusInfo->GPIOBase + HOST_COMMAND, REG_8, reg );
	vIoWriteReg( smbusInfo->GPIOBase + HOST_DATA_0, REG_8, data );
	vIoWriteReg( smbusInfo->GPIOBase + HOST_CONTROL, REG_8, 0x48 );
	temp = dIoReadReg( smbusInfo->GPIOBase + HOST_STATUS, REG_8 );
	
	vDelay(1);
	tm = 1000;
	while (((temp & 0x01) == 0x01) && (tm > 0))
	{
		vDelay(1);		
		temp = dIoReadReg(smbusInfo->GPIOBase + HOST_STATUS, REG_8);
		tm--;
	}

	if(tm == 0)
	{
		rt = DS3905_SMBUS_ERROR;
	}
	else
	{
		rt = E__OK;
		
	}

	return (rt);
}


UINT32 readReg(UINT8 addr, UINT8 reg, UINT8* data)
{
	UINT32   rt = E__OK, tm;
	UINT8	 temp;

	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);
	if(smbusInfo == NULL)
		return SMBUS_INFO_ERROR;

	vIoWriteReg( smbusInfo->GPIOBase + HOST_STATUS, REG_8, 0xFF );
	vIoWriteReg( smbusInfo->GPIOBase + HOST_CONTROL, REG_8, 0x00 );
	vIoWriteReg( smbusInfo->GPIOBase + TRANSMIT_SLAVE_ADDRESS, REG_8, (addr|1) );
	vIoWriteReg( smbusInfo->GPIOBase + HOST_COMMAND, REG_8, reg );
	vIoWriteReg( smbusInfo->GPIOBase + HOST_CONTROL, REG_8, 0x48 );
	vDelay(1);
	temp = dIoReadReg( smbusInfo->GPIOBase + HOST_STATUS, REG_8 );
	tm = 1000;
	while((temp&0x01)==0x01 && (tm>0))
	{
		vDelay(1);
		temp = dIoReadReg(smbusInfo->GPIOBase + HOST_STATUS, REG_8);
		tm--;
	}

	if(tm == 0)
	{
		rt = DS3905_SMBUS_ERROR;
	}
	else if( (temp&0x04) == 0x04)
	{
		rt = DS3905_DEV_ERROR;
	}
	else if( (temp&0x08) == 0x08)
	{
			rt = DS3905_BUS_ERROR;
	}
	else
	{
		vDelay(100);
		*data = dIoReadReg(smbusInfo->GPIOBase + HOST_DATA_0, REG_8);
	}

	return (rt);
}



TEST_INTERFACE (DS3905Test, "DS3905 Connectivity Test")
{
	UINT32   rt = E__OK;
	UINT8	 temp;

	char	 achBuffer[128];

	board_service(SERVICE__BRD_UNLOCK_CCT_IOREGS, NULL,NULL);
	board_service(SERVICE__BRD_GET_DS3905_INFO, NULL, &ds3905Info);
#ifdef DEBUG
	sprintf (achBuffer,"ds3905->DS3905_Reg1:%#x\n",ds3905Info->DS3905_Reg1);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer,"ds3905->DS3905_Reg2:%#x\n",ds3905Info->DS3905_Reg2);
	vConsoleWrite(achBuffer);
	sprintf (achBuffer,"ds3905->DS3905_Reg3:%#x\n",ds3905Info->DS3905_Reg3);
	vConsoleWrite(achBuffer);
#endif
	if (ds3905Info->DS3905_Reg1 != 0xff)
	{

		rt = readReg(ds3905Info->DS3905_Reg1, 0xF8, &temp);
		if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
			rt = E__OK;
		else
			rt = DS3905_1_ID_ERROR;

			sprintf ( achBuffer, "\nNVSW 1, SW1 : %s", (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")) );
			vConsoleWrite (achBuffer);


		if(rt == E__OK)
		{
			rt = readReg(ds3905Info->DS3905_Reg1, 0xF9, &temp);
			if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
				rt = E__OK;
			else
				rt = DS3905_1_ID_ERROR;
			sprintf ( achBuffer, "\nNVSW 1, SW2 : %s",  (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")));
			vConsoleWrite (achBuffer);
		}

		if(rt == E__OK)
		{
			rt = readReg(ds3905Info->DS3905_Reg1, 0xFA, &temp);
			if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
				rt = E__OK;
			else
				rt = DS3905_1_ID_ERROR;
			sprintf ( achBuffer, "\nNVSW 1, SW3 : %s\n",  (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")));
			vConsoleWrite (achBuffer);
		}
    }


	if (ds3905Info->DS3905_Reg2 != 0xff)
	{
		    if(rt == E__OK)
		    {
				rt = readReg(ds3905Info->DS3905_Reg2, 0xF8, &temp);
				if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
					rt = E__OK;
				else
					rt = DS3905_2_ID_ERROR;
				sprintf ( achBuffer, "\nNVSW 2, SW1 : %s",  (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")));
				vConsoleWrite (achBuffer);
		    }

			if(rt == E__OK)
			{
				rt = readReg(ds3905Info->DS3905_Reg2, 0xF9, &temp);
				if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
					rt = E__OK;
				else
					rt = DS3905_2_ID_ERROR;
				sprintf ( achBuffer, "\nNVSW 2, SW2 : %s",  (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")));
				vConsoleWrite (achBuffer);
			}


			if(rt == E__OK)
			{
				rt = readReg(ds3905Info->DS3905_Reg2, 0xFA, &temp);
				if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
					rt = E__OK;
				else
					rt = DS3905_2_ID_ERROR;
				sprintf ( achBuffer, "\nNVSW 2, SW3 : %s\n",  (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")));
				vConsoleWrite (achBuffer);
			}
	}

	if (ds3905Info->DS3905_Reg3 != 0xff)
	{
			   if(rt == E__OK)
			   {
				   rt = readReg(ds3905Info->DS3905_Reg3, 0xF8, &temp);
					if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
						rt = E__OK;
					else
						rt = DS3905_3_ID_ERROR;

						sprintf ( achBuffer, "\nNVSW 3, SW1 : %s", (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")) );
						vConsoleWrite (achBuffer);
			   }


				       if(rt == E__OK)
						{
							rt = readReg(ds3905Info->DS3905_Reg3, 0xF9, &temp);
							if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
								rt = E__OK;
							else
								rt = DS3905_3_ID_ERROR;
							sprintf ( achBuffer, "\nNVSW 3, SW2 : %s",  (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")));
							vConsoleWrite (achBuffer);
						}

						if(rt == E__OK)
						{
							rt = readReg(ds3905Info->DS3905_Reg3, 0xFA, &temp);
							if( (rt == E__OK) && ((rt == 0x00) || (rt == 0x80)) )
								rt = E__OK;
							else
								rt = DS3905_3_ID_ERROR;
							sprintf ( achBuffer, "\nNVSW 3, SW3 : %s\n",  (temp==0x00?"OFF":(temp>=0x7F?"ON":"ERROR")));
							vConsoleWrite (achBuffer);
						}
			}

	  board_service(SERVICE__BRD_LOCK_CCT_IOREGS, NULL,NULL);

    return (rt);
}

UINT32 wSMBusHRead
(
	UINT8	bAddr,		/* 7 bit address */
	UINT8	bCmd,		/* Command to send */
	UINT16*	pwData,		/* For Data read */
	UINT8	bType,		/* Type of xfr  Ie. SMB_BDATA or SMB_WDATA */
	UINT16	wTimeout	/* Timeout in 10mS blocks */
)
{
UINT32	wStatus;
UINT8	bTemp;


	/*** set up the registers ***/
	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);

	bTemp = dIoReadReg (smbusInfo->GPIOBase +  HOST_STATUS,REG_8);			/* clear status bits */
	vIoWriteReg (smbusInfo->GPIOBase + HOST_STATUS,REG_8, bTemp | 0x1E);

	vIoWriteReg (smbusInfo->GPIOBase + HOST_CONTROL,REG_8, bType);			/* Set mode */
	vIoWriteReg (smbusInfo->GPIOBase + TRANSMIT_SLAVE_ADDRESS,REG_8, (bAddr | 1));	/* Read from device */
	vIoWriteReg (smbusInfo->GPIOBase + HOST_COMMAND,REG_8, bCmd);			/* Command to send */

	wStatus = wSMBusExec(wTimeout);						/* Execute command */
	if (wStatus == E__OK)
	{
		if (bType == SMB_WDATA)							/* Word of data */
		{
			bTemp = dIoReadReg (smbusInfo->GPIOBase + HOST_DATA_1,REG_8);	/* Read MSB */
			*pwData = (UINT16) bTemp << 8;				/* Move to MSB */
			bTemp = dIoReadReg (smbusInfo->GPIOBase + HOST_DATA_0,REG_8);	/* Read LSB */
			*pwData += bTemp;
		}
		else if (bType == SMB_BDATA)					/* Byte of data */
		{
			*pwData = (UINT16) dIoReadReg (smbusInfo->GPIOBase + HOST_DATA_0,REG_8);	/* Read LSB */
		}
		else											/* Fall over! */
		{
			wStatus = E__SMB_BAD_TYPE;
		}
	}
	return (wStatus);

} /* SMBusHRead() */

UINT32	wSMBusExec
(
	UINT16	wTimeout								/* Timeout in 10mS blocks */
)
{
	UINT8 bTemp;

	board_service(SERVICE__BRD_GET_SMBUS_INFO, NULL, &smbusInfo);

	bTemp = dIoReadReg (smbusInfo->GPIOBase + HOST_CONTROL,REG_8);		/* Start process */
	vIoWriteReg (smbusInfo->GPIOBase + HOST_CONTROL,REG_8, bTemp | 0x40);

	/*** Wait for a termination event:  Success, Fail, Timeout ***/
	do {
			vDelay (10);								/* 10ms delay */
			wTimeout--;
			bTemp = dIoReadReg (smbusInfo->GPIOBase + HOST_STATUS,REG_8);	/* Get status */

	   } while ( (wTimeout > 0) && ((bTemp & 0x1E)==0) );	/* While !Timeout and !Finished */

		/*** Deal with the outcome ***/
		if (wTimeout == 0)								/* It timed out */
		{
			bTemp = dIoReadReg (smbusInfo->GPIOBase + HOST_CONTROL,REG_8);	/* Cause process to 'FAIL' */
			vIoWriteReg (smbusInfo->GPIOBase + HOST_CONTROL,REG_8, (bTemp | 0x02));
			return (E__TIMEOUT);
		}
		else											/* It terminated */
		{
			bTemp = dIoReadReg (smbusInfo->GPIOBase + HOST_STATUS,REG_8) & 0x1F;	/* Get termination condition */
			switch (bTemp)
			{
				case 0x02:	return (E__OK);
				case 0x04:	return (E__SMB_READ_DERR);
				case 0x08:	return (E__SMB_READ_CLSN);
				case 0x10:	return (E__SMB_READ_FAIL);
			}
			return (E__UNKNOWN);							/* Catch all */
		}

		return (E__OK);

}

/*+***************************************************************************
 * wMxFaultCheck - Check fault conditions on LM86
 *
 * RETURNS : E_OK or E__... error code
 *
 * E__REMOTE_OPEN
 * E__REMOTE_SHORT	- May be confused with "remote is at 0^C"
 * E__REMOTE_VCC	- May be confused with "remote & local at 127^C"
 *
 */

static UINT32 wMxFaultCheck (void)
{
  UINT16 wTemp = 0;
  UINT32 wErr = 0;

#if 0
   wErr = wSMBusHRead (0x30, MX_CMD_SR, &wTemp, SMB_BDATA, 1000);

   if (wErr != E__OK) return (wErr);

  if (wTemp & 0x04)
	return (E__REMOTE_OPEN);
#endif


  /* check external temperature 1 reads a sensible value */
  wErr = wSMBusHRead (0x30, MX_CMD_RTHB, &wTemp, SMB_BDATA, 1000);

  if (wErr != E__OK) return (wErr);

  if (wTemp==0)
	return (E__REMOTE_SHORT);
  else if (wTemp==127)
  {
	  wErr = wSMBusHRead (0x30, MX_CMD_LT, &wTemp, SMB_BDATA, 1000);

	  if (wErr != E__OK) return (wErr);
	  if (wTemp==127)
		return(E__REMOTE_VCC);
  }

  return (E__OK);
}

TEST_INTERFACE (TempTest, "Thermal Sensor Test")
{
	UINT8 temp;
	char	 achBuffer[128];	
	int i=0;
	for(i=0;i<2;i++)
	{
		temp = readTemp();
	}
	sprintf (achBuffer, "\nCPU Temparature =%d\n", temp);
	vConsoleWrite (achBuffer);

	return (E__OK);
}



TEST_INTERFACE (LM95245Test, "LM95245 Device Test")
{
	UINT32   rt = E__OK;



#ifdef DEBUG
	char	 achBuffer[128];
#endif


	rt = wSMBusHRead (0x30, 0xFE, &ManufId, SMB_BDATA, 1000);
	if (LM95245_MANUF_ID == ManufId)
	{
		rt = E__OK;
	}

#ifdef DEBUG
			sprintf ( achBuffer, "Data from LM 95245 0x%x\n", ManufId);
			vConsoleWrite (achBuffer);
#endif

	rt = wSMBusHRead (0x30, 0xFF, &RevId, SMB_BDATA, 1000);
	if (RevId == LM95245_REV_ID )
	{
		rt = E__OK;
	}


#ifdef DEBUG
			sprintf ( achBuffer, "Data from LM 95245 0x%x\n", RevId);
			vConsoleWrite (achBuffer);
#endif

    rt = wMxFaultCheck();
	switch (rt)
	 {
	     case E__OK:
	    	 vConsoleWrite("Diagnostics passed.\n");
	    	 rt = E__OK;
		 break;
		 case E__REMOTE_OPEN:
			 vConsoleWrite("Warning: CPU diode is open-circuit\n");
			 rt = E__OK;
			 break;
		 case E__REMOTE_SHORT:
			 vConsoleWrite("Warning: CPU diode may be short circuited\n");
			 rt = E__OK;
			 break;
		case E__REMOTE_VCC:
			vConsoleWrite("Warning: CPU diode may be connected to Vcc\n");
			rt = E__OK;
			  break;
		default:
			  return (rt);
	 }

	  vDelay (100);
	  rt = wMxReadTempLR();

	 return (rt);
}

/*+***************************************************************************
 * wMxReadTempLR - Temperature readout for Local & Remote sensors
 *
 * RETURNS : E_OK or E__... error code
 *
 */

static UINT32 wMxReadTempLR (void)
{

  UINT16  wLocal;
  UINT32 wErr;
  char   achBuffer[80];  /* text formatting buffer */


  wErr = wSMBusHRead (0x30, MX_CMD_LT, &wLocal, SMB_BDATA, 1000);

  if (ManufId == ADT7461_MANUF_ID)
  {
	  wLocal -= 64;
  }

  if (wErr != E__OK) return (wErr);
  if (wLocal & 0x80)
	{
	  wLocal = (UINT8)(~wLocal) +1;			/* 2's complement */
	  sprintf (achBuffer, "Ambient temperature: -%d^C\n", wLocal);
	  vConsoleWrite (achBuffer);
	}
  else
	{
	  sprintf (achBuffer, "Ambient temperature: %d^C\n", wLocal);
	  vConsoleWrite (achBuffer);
	}

   return (E__OK);

} /* wMxReadTempLR() */

UINT8 readTemp(void)
{
	UINT16 temp2 = 0;
	UINT8 temp3 = 0;
#ifdef DEBUG
	char achBuffer[128];	
#endif
	
	
	
	//Initialize the PECI
	//Bank 0
	writeReg(0x50, 0, 0);
	writeReg(0x50, 0x23, 3);

	readReg(0x51, 0x23, (UINT8*)&temp2);
#ifdef DEBUG
	sprintf(achBuffer,"Value at 0x23 intially =%d\n",temp2);
	vConsoleWrite (achBuffer); 
#endif
	//Bank 1
	writeReg(0x50, 0, 1);
	writeReg(0x50, 1, 0x81);
	writeReg(0x50, 2, 0);
	writeReg(0x50, 3,0x33);
	writeReg(0x50, 4, 0);

	readReg(0x51, 0x17, (UINT8*)&temp2);
	temp2  = (temp2 << 8) & 0xff00;
	readReg(0x51, 0x18, &temp3);
	temp2 |= temp3;
#ifdef DEBUG
	sprintf(achBuffer,"temp 16 bit =%d\n",temp2);
	vConsoleWrite (achBuffer);
#endif

	temp2  = ((temp2 >> 6) & 0x3FF) | 0xFC00;
#ifdef DEBUG
	sprintf(achBuffer,"temp2 before masking =%d\n",temp2);
	vConsoleWrite (achBuffer);
#endif
	temp2  = ~(temp2 - 1);
	temp2  = 105 - temp2;//IvyBridge Junction temperature is 105
#ifdef DEBUG
	sprintf(achBuffer,"temp2 =%d\n",temp2);
	vConsoleWrite (achBuffer);
#endif
	return ((UINT8)temp2);
	
	
}

