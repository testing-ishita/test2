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

/* ipmi.c - BIT test for IPMI
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/ipmi.c,v 1.15 2015-01-21 17:59:54 gshirasat Exp $
 *
 * $Log: ipmi.c,v $
 * Revision 1.15  2015-01-21 17:59:54  gshirasat
 * Check for UNR thresholds only for temperature sensors as we do no usually populated for voltage sensor.
 *
 * Revision 1.14  2014-11-04 15:04:59  mgostling
 * Pass Geographic Address OEM command for IPMI controller to dAMCTests
 * and added IpmiAMCTest_M3 removing the need for a separate file for use with
 * the Cortex M3 IPMI Controller as the Geographic Address OEM command used
 * in dAmcTests requires a different value for the 8051 IPMI Controller.
 *
 * Revision 1.13  2014-10-15 12:00:20  mgostling
 * Updated dTestThresholds to make the code easier to understand.
 * dIpmiSensorThresholdTest now returns the sensor number and threshold bitmap as the error code.
 * Changed dAMCTests to obtain the hotswap ejector handle position by using the
 * standard 'get sensor reading' command instead of using a special OEM command.
 *
 * Revision 1.12  2014-10-09 14:44:52  mgostling
 * Disabled the code for the IpmiAlarmIntTest as this feature is not supported by the IPMI micro
 * and it is not a requirement of the IPMI specification.
 *
 * Revision 1.11  2014-09-19 13:06:59  mgostling
 * Added additional debugging messages
 *
 * Revision 1.10  2014-07-24 14:07:10  mgostling
 * Added function to obtain board slot id from IPMI controller.
 *
 * Revision 1.9  2014-06-13 10:31:51  mgostling
 * Resolve compiler warnings
 *
 * Revision 1.8  2014-04-29 09:01:25  mgostling
 * Fixed FPU exceptions in Sensor Threshold Test.
 * Fixed conversion errors of floating point values in Sensor Threshold Test.
 * Added additional debugging messages to Alarm Interrupt Test.
 * Report reading, threshold and sensor ID when Alarm Interrupt Test fails.
 *
 * Revision 1.7  2014-02-14 11:35:47  chippisley
 * Modified IPMI sensor threshold test to report failure only for Upper Critical
 * and Upper Non Recoverable conditions.
 *
 * Added build option for AM 945 variant to workaround IPMI firmware bug that
 * reports negative temperatures as high positive values on sensor 2. When this
 * build option is selected, sensor 2 is compared with sensor 1 and a test failure
 * is not indicated if sensor 2 is reported by the IPMI firmware as a higher value.
 *
 * Revision 1.6  2014/01/29 13:39:47  hchalla
 * Added support for getting status for set port state for AMC boards, and get ipmb address for slot id.
 *
 * Revision 1.5  2013-11-25 11:52:19  mgostling
 * Relaxed interrupt timings.
 *
 * Revision 1.4  2013-10-08 07:13:38  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.3  2013/09/27 16:02:28  chippisley
 * Added dIpmiReady() function to avoid long timeout when
 * calling dIpmiSendRequest().
 *
 * Revision 1.2  2013/09/27 12:19:46  chippisley
 * Added bGetAMCIpmbAddrs() function.
 *
 * Revision 1.1  2013/09/04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.14  2011/08/26 15:51:00  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.13  2011/08/02 17:21:43  hchalla
 * Fixed build error undeclared 	UINT8  bBoardNmi variable.
 *
 * Revision 1.12  2011/08/02 16:59:00  hchalla
 * *** empty log message ***
 *
 * Revision 1.11  2011/06/13 12:33:38  hmuneer
 * no message
 *
 * Revision 1.10  2011/05/16 14:54:41  hmuneer
 * Info Passing Support
 *
 * Revision 1.9  2011/05/16 14:32:12  hmuneer
 * Info Passing Support
 *
 * Revision 1.8  2011/03/22 13:56:22  hchalla
 * Cleanup for spaces and tabs.
 *
 * Revision 1.7  2011/02/28 11:59:20  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.6  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.5  2010/11/12 11:31:56  hchalla
 * IPMI code clean up.
 *
 * Revision 1.4  2010/11/04 17:51:11  hchalla
 * Fixed problem with floating point calculations in IPMI, by setting a double flag in FPU. Installed FPU handler when an exception occurs to clear the FPU exception flag.
 *
 * Revision 1.3  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.2  2009/06/09 14:59:22  jthiru
 * Added IPMI Watchdog test cases
 *
 * Revision 1.1  2009/05/15 11:10:34  jthiru
 * Initial checkin for IPMI tests
 *
 *
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <errors.h>
#include <error_msg.h>

#include <bit/console.h>
#include <bit/bit.h>
#include <bit/mem.h>
#include <bit/delay.h>
#include <bit/msr.h>
#include <private/cpu.h>
#include <devices/ipmi.h>

#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/board_service.h>
#include <private/cpu.h>

#include "../debug/dbgLog.h"

/* defines */

//#define DEBUG
//#define DEBUG_VERBOSE
#undef INCLUDE_IPMI_DEBUG

#if defined(DEBUG) || defined(DEBUG_VERBOSE) || defined(INCLUDE_IPMI_DEBUG)
#warning "***** DEBUG ON *****"
#endif

#ifdef INCLUDE_IPMI_DEBUG /* use DBLOG or sysDebugPrintf */
#define IPMIDBG(x)	do {DBLOG x;} while(0)
#else
#define IPMIDBG(x)
#endif

#define FPU_ADD				0
#define FPU_SUB				1
#define FPU_MUL				2
#define FPU_DIV				3

	/* fpuSetup () operations */

#define FPU_READ_CW			0
#define FPU_WRITE_CW		1
#define FPU_INIT			2

	/* Error codes */
#define E__NOT_SUPPORTED		(E__BIT + 0x00000000)

#define E__FPU_ADD_ERROR		(E__BIT + 0x00000001)
#define E__FPU_SUB_ERROR		(E__BIT + 0x00000002)
#define E__FPU_MUL_ERROR		(E__BIT + 0x00000003)
#define E__FPU_DIV_ERROR		(E__BIT + 0x00000004)
#define E__FPU_FLG_ERROR		(E__BIT + 0x00000005)

#define E__FPU_EXP_ERROR		(E__BIT + 0x00000006)
#define E__FPU_INT_ERROR		(E__BIT + 0x00000007)

#define E__FPU_FERR				(E__BIT + 0x00000100)
#define E__FPU_MF				(E__BIT + 0x00000200)

#define CPU_POS_THR_ERROR		(E__BIT  +0x04)
#define CPU_NEG_THR_ERROR		(E__BIT  +0x05)

	/* FPU exception/interrupt defines */

#define FPU_MF				16				/* CPU H/W exception 16 */
#define FPU_FERR			13				/* FERR interrupts on APIC pin 13 */

#if defined(VPX)
#define MAX_VPX_SLOTS                                          22
static UINT8 vpxBoardId[MAX_VPX_SLOTS] = {
                0xFF,           /*Invalid as there is no slot 0*/
                0x1E,           /*Slot ID 1 */
                0x1D,
                0x1C,
                0x1B,
                0x1A,
                0x19,
                0x18,
                0x17,
                0x16,
                0x15,
                0x14,
                0x13,
                0x12,
                0x11,
                0x10,
                0x0F,
                0x0E,
                0x0D,
                0x0C,
                0x0B,
                0x0A
};
#endif

static char buffer[128];

/* externals */
 extern void	fpuSetup(UINT32 operation, UINT32 *op);
 extern void fpuClearFlags(UINT32 op);

/* constants */

/* locals */

/* globals */
static UINT8 isrCalled = 0;
static  int	fpuFlag;
/* static	int	iVector; */
static double CPUTemp;
static double AmbientTemp;
static char errorbuffer[64];

#ifdef INCLUDE_AM94X_VARIANT_IPMI_WORKAROUND
/* For Aeroflex AM 945 variant only */
#warning "***** INCLUDE_AM94X_VARIANT_IPMI_WORKAROUND *****"
static UINT16 ipmiVer = 0;
static INT32 tempReading1 = 0;
static INT32 tempReading2 = 0;
#endif

/* externals */

/* forward declarations */
static int setupInt(int		iPin,int		iVector	);
static void IpmifpuHandler (void);
static UINT32 installFPUExcpHandler(int iPin, int iVector);
static UINT32 UninstallFPUExcpHandler(int iPin, int iVector);
/* static UINT32 tempsenseDTS(int *rd);  */

static int removeInt
(
	int		iPin,		/* pin number, -1 if not a pin */
	int		iVector		/* vector number if not a pin  */
);

/*****************************************************************************
 *
 * Outbyte: Write a byte into the IO port
 *
 * RETURNS: N/A
 */

static void Outbyte
(
	UINT16 wPort,
	UINT8 bData
)
{
	vIoWriteReg(wPort, REG_8, bData);
	/* __asm__("out	%%al, %%dx" : : "a" (bData), "d" (wPort));*/
}


/*****************************************************************************
 *
 * Inbyte: Read a byte into the IO port
 *
 * RETURNS: Returns the byte read
 */

static UINT8 Inbyte
(
	UINT16 wPort
)
{
	UINT32 result;
	result = dIoReadReg(wPort, REG_8);
	/*__asm__("in		%%dx, %%al"	: "=a" (result) : "d" (wPort) );*/
	return (UINT8)result;
}


/*****************************************************************************
 *
 * dIpmiProbe: Check is SMIC interface exist
 *
 * RETURNS: E__OK if it is OK, or error code
 */

static UINT32 dIpmiProbe (void)
{
	UINT32 dTestStatus = E__OK;

	if (bReadSmicStatus == 0xFF)
		dTestStatus = E__NO_IPMI;

	return dTestStatus;
}


/*****************************************************************************
 * vSmicDelay: Delay a millisecond
 *
 * RETURNS: N/A
 */

static void vSmicDelay (void)
{
    vDelay(5);
} /* vSmicDelay () */


/*****************************************************************************
 * dSmicSendRequest - Send request to SMIC
 *
 * Send request over SMIC registers
 *
 * RETURNS: E__OK if it is OK, or error code
 */

static UINT32 dSmicSendRequest
(
	const UINT8* pbMessage,
	UINT8	bLength
)
{
    UINT32 dTimeout;


	if (bLength < 2)
		return E__IPMI_COMM;

    dTimeout = TIMEOUT;

	while (((bReadSmicFlags) & FLAGS_BUSY) == FLAGS_BUSY && --dTimeout > 0)
		vSmicDelay(); /* do nothing ... */

    if (dTimeout == 0)
    	return E__IPMI_COMM;

	vWriteSmicControl (CC_SMS_WR_START);
	vWriteSmicData (*pbMessage++);
	vWriteSmicFlags (FLAGS_BUSY);
	bLength--;

	do
	{
	    dTimeout = TIMEOUT;
		while (((bReadSmicFlags) & FLAGS_TX_DATA_READY) != FLAGS_TX_DATA_READY
				&& --dTimeout > 0)
			vSmicDelay(); /* do nothing ... */
	    if (dTimeout == 0)
    		return E__IPMI_COMM;

	    dTimeout = TIMEOUT;
		while (((bReadSmicFlags) & FLAGS_BUSY) == FLAGS_BUSY && --dTimeout > 0)
			vSmicDelay(); /* do nothing ... */
	    if (dTimeout == 0)
    		return E__IPMI_COMM;

		vWriteSmicControl ((bLength > 1) ? CC_SMS_WR_NEXT : CC_SMS_WR_END);

		vWriteSmicData (*pbMessage++);
		vWriteSmicFlags (FLAGS_BUSY);
		bLength--;

        dTimeout = TIMEOUT;
		while (((bReadSmicFlags) & FLAGS_BUSY) == FLAGS_BUSY && --dTimeout > 0)
			vSmicDelay(); /* do nothing ... */
	    if (dTimeout == 0)
    		return E__IPMI_COMM;

	} while (bLength > 0);

	return (E__OK);

} /* dSmicSendRequest () */


/*****************************************************************************
 * dSmicReceiveResponse - Receive Respond from SMIC
 *
 * Received response put into buffer and save length of buffer
 *
 * RETURNS: E__OK if it is OK, or error code
 *			abBuffer[] - recived response
 *			bLength - Length of buffer
 */

static UINT32 dSmicReceiveResponse
(
	UINT8*	pbMessage,
	UINT8*	pbMessageLength
)
{
	UINT8 bReadControl;
	UINT8 bReadStatus;
	UINT8 bReadData;
    UINT32 dTimeout, dTimeout2;


    *pbMessageLength = 0;

	bReadControl = CC_SMS_RD_START;

	dTimeout2 = TIMEOUT;
	do
	{
        dTimeout = TIMEOUT;
		while (((bReadSmicFlags) & FLAGS_RX_DATA_READY) != FLAGS_RX_DATA_READY
				&& --dTimeout > 0)
			vSmicDelay(); /* do nothing ... */

		if (dTimeout == 0)
			return E__IPMI_COMM;

		dTimeout = TIMEOUT;
		while (((bReadSmicFlags) & FLAGS_BUSY) == FLAGS_BUSY && --dTimeout > 0)
			vSmicDelay(); /* do nothing ... */

		if (dTimeout == 0)
			return E__IPMI_COMM;

		vWriteSmicControl (bReadControl);
		vWriteSmicFlags (FLAGS_BUSY);

		dTimeout = TIMEOUT;
		while ( (((bReadSmicFlags) & FLAGS_BUSY) == FLAGS_BUSY) && (--dTimeout > 0))
			vSmicDelay(); /* do nothing ... */

		if (dTimeout == 0)
			return E__IPMI_COMM;

		bReadStatus = bReadSmicStatus;
		bReadData   = bReadSmicData;

		switch (bReadStatus)
		{
			case SC_SMS_RD_START :
				*pbMessage++ = bReadData;
				(*pbMessageLength)++;
				bReadControl = CC_SMS_RD_NEXT;
				break;

			case SC_SMS_RD_NEXT  :
				*pbMessage++ = bReadData;
				(*pbMessageLength)++;
				break;

			case SC_SMS_RD_END   :
				*pbMessage++ = bReadData;
				(*pbMessageLength)++;
				bReadControl = CC_SMS_RD_END;
				break;

			default :
				break;
		}

	} while ((bReadStatus != SC_SMS_RDY) && (--dTimeout2 > 0));

	if (dTimeout2 == 0)
		return E__IPMI_COMM;

	return (E__OK);

} /* dSmicReceiveResponse () */


/*****************************************************************************
 *
 * dIpmiSendRequest: Send request to SMIC interface and receive response
 *
 * RETURNS: E__OK if it is OK, or error code
 *			UINT8 *pbReponse	array with responsed message
 * 			pbRespLength		length of responsed messsage
 */

UINT32 dIpmiSendRequest
(
	UINT8 *pbRequest,	/* array with reqested message */
	UINT8 bReqLength,	/* length of requested message */
	UINT8 *pbResponse,	/* array with responsed message */
	UINT8 *pbRespLength	/* length of responsed messsage */
)
{
	UINT32 dTestStatus;

#ifdef DEBUG
	UINT8 i;
#endif

	*pbRespLength = 0;

	/*(void) dReadTimeStamp (&dStartTimeLow, &dStartTimeHigh);*/

	/* send request */

    dTestStatus = dSmicSendRequest (pbRequest, bReqLength);
#ifdef DEBUG
	if (dTestStatus == E__OK)
	{
		sprintf (buffer, "Request:  ");
		for (i = 0; i < bReqLength; i++)
			sprintf (buffer, "%s%02X ", buffer, pbRequest [i]);
		vConsoleWrite(buffer);
		vConsoleWrite("\n");
	}
#endif

	/* receive response */

    if (dTestStatus == E__OK)
	    dTestStatus = dSmicReceiveResponse (pbResponse, pbRespLength);
#ifdef DEBUG
	if (dTestStatus == E__OK)
	{
		sprintf (buffer, "Response: ");
		for (i = 0; i < *pbRespLength; i++)
			sprintf (buffer, "%s%02X ", buffer, pbResponse [i]);
		vConsoleWrite(buffer);
		vConsoleWrite("\n");
	}
#endif

	/* if response is short */

    if (dTestStatus == E__OK && *pbRespLength < 3)
    	dTestStatus = E__RESPONSE;

	/* if completion code is not OK, return error */

    if (dTestStatus == E__OK && pbResponse [2] != COMP_OK)
    	dTestStatus = E__COMP_CODE;

	return dTestStatus;
}


/*****************************************************************************
 *
 * dIpmiInit: Initialize IPMI interface.
 *
 * RETURNS: E__OK if it is OK, or error code
 */

UINT32 dIpmiInit (void)
{
	UINT32	dTestStatus;

	board_service (SERVICE__BRD_ENABLE_SMIC_LPC_IOWINDOW, NULL, NULL);

	/* check is SMIC interface exist */

	if ((dTestStatus = dIpmiProbe ()) != E__OK)
	{
		#ifdef DEBUG
			vConsoleWrite ("SMIC interface is not enabled");
		#endif
		return dTestStatus;
	}

	return dTestStatus;
}

/*****************************************************************************
 *
 * dReadDeviceID: Initialize IPMI interface.
 *
 * RETURNS: E__OK if it is OK, or error code
 */

UINT32 dReadDeviceID (void)
{
	UINT32 dtest_status;
	UINT8	abRequest [SMS_LEN_BUFFER];
	UINT8	abResponse [SMS_LEN_BUFFER];
	UINT8	bRespLength;

	/* Request Get Device ID */
	abRequest [0] = (NFC_APP_REQUEST << 2) | (LUN_BMC & 0x03);	/* NFC + LUN */
	abRequest [1] = CMD_GET_DEVICE_ID;	/* command */

	dtest_status = dIpmiSendRequest(&abRequest[0], 2, &abResponse[0], &bRespLength);
	if (dtest_status == E__OK)
	{
		/* index 0, 1 refers to response NFC/LUN and command */
		/* Verify manufacturer ID */
		if((abResponse[9] == 0x4A) && (abResponse[10] == 0x5F) &&(abResponse[11] == 0x00))
			dtest_status = E__OK;
		else
			dtest_status = E__DATAMISMATCH;
	}
	return dtest_status;
}


/*****************************************************************************
 *
 * dIpmiInterruptTest: Test IPMI interrupt.
 *
 * RETURNS: E__OK if it is OK, or error code
 */

static UINT32 dIpmiInterruptTest (void)
{
	UINT32	dTimeout;
	UINT32	dTestStatus = E__OK;
	UINT8	bTemp;


	/* wait for BUSY flag is cleared */
    dTimeout = TIMEOUT;
	while (((bReadSmicFlags) & FLAGS_BUSY) == FLAGS_BUSY && --dTimeout > 0)
		vSmicDelay();

	if (dTimeout == 0)
		return E__IPMI_COMM;

	/* Enable SMIC Not Busy Interrupt */

#ifdef DEBUG
  sprintf (buffer, "Before enable 0x215=%02Xh Flags=%02Xh\n",
			Inbyte(0x215), bReadSmicFlags);
  vConsoleWrite(buffer);
#endif

	Outbyte (SMIC_INT_REG, SMIC_NOT_BUSY_ENABLE); /* enable only NOT BUSY int */

#ifdef DEBUG
  sprintf (buffer, "Before interrupt 0x215=%02Xh Flags=%02Xh\n",
			Inbyte(0x215), bReadSmicFlags);
  vConsoleWrite (buffer);
#endif

	/* Cause interrupt */
	vWriteSmicControl (CC_SMS_GET_STATUS);	/* get status */
	vWriteSmicFlags (FLAGS_BUSY);			/* set Busy flag */
	vSmicDelay();							/* wait interrupt */

#ifdef DEBUG
  sprintf (buffer, "After interrupt 0x215=%02Xh Flags=%02Xh\n",
			Inbyte(0x215), bReadSmicFlags);
  vConsoleWrite(buffer);
#endif

	/* disable interrupt */

	bTemp = Inbyte (SMIC_INT_REG) & ~SMIC_NOT_BUSY_ENABLE;	/* disable interrupt */
	Outbyte (SMIC_INT_REG, bTemp);

#ifdef DEBUG
  sprintf (buffer, "After disable 0x215=%02Xh Flags=%02Xh\n",
			Inbyte(0x215), bReadSmicFlags);
  vConsoleWrite(buffer);
#endif

	/* check what was happened */
	if ((Inbyte (SMIC_INT_REG) & SMIC_NOT_BUSY_FLAG) == 0)
		dTestStatus = E__IPMI_NO_INT;		/* No interrupt. */

	/* clear BUSY interrupt flag */

	bTemp = Inbyte (SMIC_INT_REG) & ~SMIC_NOT_BUSY_FLAG;	/* clear flag */
	Outbyte (SMIC_INT_REG, bTemp);

#ifdef DEBUG
  sprintf (buffer, "After cleared flag 0x215=%02Xh Flags=%02Xh\n",
			Inbyte(0x215), bReadSmicFlags);
  vConsoleWrite(buffer);
#endif

	/* clear RX_DATA_RDY flag */

	if ((bReadSmicFlags & FLAGS_RX_DATA_READY) != 0)
	{
		vWriteSmicControl (CC_SMS_RD_END);	/* get status */
		vWriteSmicFlags (FLAGS_BUSY);		/* set Busy flag */

		dTimeout = TIMEOUT;
		while (((bReadSmicFlags) & FLAGS_BUSY) == FLAGS_BUSY && --dTimeout > 0)
		vSmicDelay();

		if (dTimeout == 0)
			return E__IPMI_COMM;

#ifdef DEBUG
  sprintf (buffer, "After clear data 0x215=%02Xh Flags=%02Xh\n",
			Inbyte(0x215), bReadSmicFlags);
  vConsoleWrite(buffer);
#endif
	}

	return dTestStatus;
}


/*****************************************************************************
 *
 * dIpmiSelfTestStatus: IPMI Self test status - Internal BIT test.
 *
 * RETURNS: E__OK if it is OK, or error code
 */

UINT32 dIpmiSelfTestStatus (void)
{
	UINT32 dtest_status;
	UINT8	abRequest [SMS_LEN_BUFFER];
	UINT8	abResponse [SMS_LEN_BUFFER];
	UINT8	bRespLength;

	/* Request Get Device ID */

	abRequest [0] = (NFC_APP_REQUEST << 2) | (LUN_BMC & 0x03);	/* NFC + LUN */
	abRequest [1] = CMD_GET_SELF_TEST;	/* command */

	dtest_status = dIpmiSendRequest(&abRequest[0], 2, &abResponse[0], &bRespLength);
	if (dtest_status == E__OK)
	{
		/* index 0, 1 refers to response NFC/LUN and command */
		/* Verify manufacturer ID */
		if(abResponse[3] == 0x55)
			dtest_status = E__OK;
		else if(abResponse[3] == 0x56)
			dtest_status = E__CMDNOTIMPL;
		else if(abResponse[3] == 0x57)
			dtest_status = E__IPMIRW;
		else if(abResponse[3] == 0x58)
			dtest_status = E__IPMIFATALERR;
		else if(abResponse[3] != 0xFF)
			dtest_status = E__IPMISPECERR;
	}
	if(dtest_status == E__CMDNOTIMPL)
	{
#ifdef DEBUG
		vConsoleWrite("Command not implemented\n");
#endif
		dtest_status = E__OK;
	}
	return dtest_status;
}


/*****************************************************************************
 *
 * dIpmiSDRformatTest: IPMI SDR entries Format verification.
 *
 * RETURNS: E__OK if it is OK, or error code
 */

UINT32 dIpmiSDRformatTest (void)
{
	UINT32 dTestStatus   = E__OK;
	UINT8  abSdrRq [SMS_LEN_BUFFER];
	UINT8  abSdrRs [SMS_LEN_BUFFER];
	UINT16 wRecordId, wNextRecordId;
	UINT8  bRsLen;

	/* loop through the SDR Repository */
	for (wRecordId = 0x0000; wRecordId != 0xFFFF; wRecordId = wNextRecordId)
	{
		/* issue IPMI command to get (entire) SDR */

		abSdrRq [0] = (NFC_STORAGE_REQUEST << 2) |(LUN_BMC & 0x03);
		abSdrRq [1] = CMD_GET_SDR_ENTRY;
		abSdrRq [2] = 0x00;
		abSdrRq [3] = 0x00;
		abSdrRq [4] = wRecordId & 0xFF;
		abSdrRq [5] = (wRecordId >> 8) & 0xFF;
		abSdrRq [6] = 0x00;
		abSdrRq [7] = 0xFF;

		if ((dTestStatus =
			dIpmiSendRequest (&abSdrRq[0], 8, &abSdrRs [0], &bRsLen))
			!= E__OK)
		break;

		/* extract next record ID from response */

		wNextRecordId = abSdrRs [3] + 256 * abSdrRs[4];

		/* Record length and version*/

		if((abSdrRs[9] == bRsLen - 10) && (abSdrRs[7] == 0x51))
			dTestStatus = E__OK;
		else
			dTestStatus = E__SDRFORMAT + wRecordId;
	} /* end for() */

	return (dTestStatus);
}


/*****************************************************************************
 * exponent: calculates 10 to the power of dPow
 *
 * RETURNS: result
 */

static double exponent (double dPow)
{
	int i, pow;
	double exp = 10;
	char buffer[80];

	pow = (int)dPow;

	if (pow > 0)
	{
		for (i = 1; i < pow; i++)
		exp *= 10.0;
	}
	else if (pow < 0)
	{
		for (i = 1; i > pow; i--)
		{
		exp /= 10.0;
		}
	}
	else /* pow == 0 */
		return 1.0;

	if (fpuFlag == 1)
	{
		sprintf (buffer, "3. FPU Exception Occured\n");
		vConsoleWrite(buffer);
		return (E__FPU_EXP_ERROR);
	}
	return exp;
}

#ifdef DEBUG_VERBOSE
/*****************************************************************************
 *
 * vDecodeFullSensorRecord: Dump SDR record.
 *
 */
void vDecodeFullSensorRecord(UINT8 pSdr[])
{
	UINT8 i;

	vConsoleWrite("\nFull Sensor Record Decode:\n");
	sprintf (buffer, "Record ID = 0x%04X\n", pSdr[0] + (pSdr[1] * 256));
	vConsoleWrite(buffer);
	sprintf (buffer, "SDR Version = 0x%02X\n", pSdr[2]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Record Type = %d\n", pSdr[3]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Record Length = %d\n", pSdr[4]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Owner ID = 0x%02X\n", pSdr[5]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Owner LUN = 0x%02X\n", pSdr[6]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Number = %d\n", pSdr[7]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Entity ID = 0x%02X\n", pSdr[8]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Entity Instance = 0x%02X\n", pSdr[9]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Initialization = 0x%02X\n", pSdr[10]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Capabilities = 0x%02X\n", pSdr[11]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Type = 0x%02X\n", pSdr[12]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Event/Reading Type = 0x%02X\n", pSdr[13]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Assertion Event/Lower Threshold Reading Mask = 0x%04X\n", (pSdr[14] * 256) + pSdr[15]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Deassertion Event/Upper Threshold Reading Mask = 0x%04X\n", (pSdr[16] * 256) + pSdr[17]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Discrete Reading/Settable/Readable Threshold Mask = 0x%04X\n", (pSdr[18] * 256) + pSdr[19]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Units 1 = 0x%02X\n", pSdr[20]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Units 2 = 0x%02X\n", pSdr[21]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Units 3 = 0x%02X\n", pSdr[22]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Linearization = 0x%02X\n", pSdr[23]);
	vConsoleWrite(buffer);
	sprintf (buffer, "M = 0x%02X\n", pSdr[24]);
	vConsoleWrite(buffer);
	sprintf (buffer, "M Tolerance = 0x%02X\n", pSdr[25]);
	vConsoleWrite(buffer);
	sprintf (buffer, "B = 0x%02X\n", pSdr[26]);
	vConsoleWrite(buffer);
	sprintf (buffer, "B Accuracy = 0x%02X\n", pSdr[27]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Accuracy, Accuracy exp = 0x%02X\n", pSdr[28]);
	vConsoleWrite(buffer);
	sprintf (buffer, "R exp, B exp = 0x%02X\n", pSdr[29]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Analog characteristic flags = 0x%02X\n", pSdr[30]);
	vConsoleWrite(buffer);
	
	if (pSdr[21] == 1)		// temperature
	{
		sprintf (buffer, "Nominal Reading = %d\n", (INT8) pSdr[31]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Nominal Maximum =  %d\n", (INT8) pSdr[32]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Nominal Minimum = %d\n", (INT8) pSdr[33]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Sensor Maximum Reading = %d\n", (INT8) pSdr[34]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Sensor Minimum Reading = %d\n", (INT8) pSdr[35]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Upper non-recoverable Threshold = %d\n", (INT8) pSdr[36]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Upper critical Threshold = %d\n", (INT8) pSdr[37]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Upper non-critical Threshold = %d\n", (INT8) pSdr[38]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Lower non-recoverable Threshold = %d\n", (INT8) pSdr[39]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Lower critical Threshold = %d\n", (INT8) pSdr[40]);
		vConsoleWrite(buffer);
		sprintf (buffer, "Lower non-critical Threshold = %d\n", (INT8) pSdr[41]);
		vConsoleWrite(buffer);
	}
	else						// voltage
	{
		sprintf (buffer, "Nominal Reading = %d\n", (((UINT16) pSdr[31]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Nominal Maximum =  %d\n", (((UINT16) pSdr[32]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Nominal Minimum = %d\n", (((UINT16) pSdr[33]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Sensor Maximum Reading = %d\n", (((UINT16) pSdr[34]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Sensor Minimum Reading = %d\n", (((UINT16) pSdr[35]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Upper non-recoverable Threshold = %d\n", (((UINT16) pSdr[36]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Upper critical Threshold = %d\n", (((UINT16) pSdr[37]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Upper non-critical Threshold = %d\n", (((UINT16) pSdr[38]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Lower non-recoverable Threshold = %d\n", (((UINT16) pSdr[39]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Lower critical Threshold = %d\n", (((UINT16) pSdr[40]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
		sprintf (buffer, "Lower non-critical Threshold = %d\n", (((UINT16) pSdr[41]) * pSdr[24]) / 10);
		vConsoleWrite(buffer);
	}
	sprintf (buffer, "Positive-going Threshold Hysteresis = %d\n", pSdr[42]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Negative-going Threshold Hysteresis = %d\n", pSdr[43]);
	vConsoleWrite(buffer);
	// pSdr[44] & pSdr[45] reserved
	sprintf (buffer, "OEM = 0x%02X\n", pSdr[46]);
	vConsoleWrite(buffer);
	sprintf (buffer, "ID String Type/Length Code = 0x%02X\n", pSdr[47]);
	vConsoleWrite(buffer);

	switch (pSdr[47] & 0xC0) 
	{
		case 0x00:
			sprintf (buffer, "ID String Type = unicode\n");
			break;
	
		case 0x40:
			sprintf (buffer, "ID String Type = BCD Plus\n");
			break;
	
		case 0x80:
			sprintf (buffer, "ID String Type = 6 Bit ascii\n");
			break;
	
		case 0xC0:
			sprintf (buffer, "ID String Type = 8 Bit ascii\n");
			break;
	}
	vConsoleWrite(buffer);

	if (((pSdr[47] & 0x1F) != 0) && ((pSdr[47] & 0x1F) != 0x1F))
	{
		memset(buffer, 0x00, 80 );
		for (i = 0; i < (pSdr[47] & 0x1F); ++i)
		{
			buffer[i] = pSdr[48 + i];
		}
		vConsoleWrite(buffer);
		vConsoleWrite("\n");
	}
	vConsoleWrite("\n");
}

/*****************************************************************************
 *
 * vDecodeCompactSensorRecord: Dump SDR record.
 *
 */
void vDecodeCompactSensorRecord(UINT8 pSdr[])
{
	UINT8 i;

	vConsoleWrite("\nFull Sensor Record Decode:\n");
	sprintf (buffer, "Record ID = 0x%04X\n", pSdr[0] + (pSdr[1] * 256));
	vConsoleWrite(buffer);
	sprintf (buffer, "SDR Version = 0x%02X\n", pSdr[2]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Record Type = %d\n", pSdr[3]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Record Length = %d\n", pSdr[4]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Owner ID = 0x%02X\n", pSdr[5]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Owner LUN = 0x%02X\n", pSdr[6]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Number = %d\n", pSdr[7]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Entity ID = 0x%02X\n", pSdr[8]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Entity Instance = 0x%02X\n", pSdr[9]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Initialization = 0x%02X\n", pSdr[10]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Capabilities = 0x%02X\n", pSdr[11]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Type = 0x%02X\n", pSdr[12]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Event/Reading Type = 0x%02X\n", pSdr[13]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Assertion Event/Lower Threshold Reading Mask = 0x%04X\n", (pSdr[14] * 256) + pSdr[15]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Deassertion Event/Upper Threshold Reading Mask = 0x%04X\n", (pSdr[16] * 256) + pSdr[17]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Discrete Reading/Settable/Readable Threshold Mask = 0x%04X\n", (pSdr[18] * 256) + pSdr[19]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Units 1 = 0x%02X\n", pSdr[20]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Units 2 = 0x%02X\n", pSdr[21]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Units 3 = 0x%02X\n", pSdr[22]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Record Sharing [Byte 1] = 0x%02X\n", pSdr[23]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Record Sharing [Byte 2]= 0x%02X\n", pSdr[24]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Positive-going Threshold Hysteresis = 0x%02X\n", pSdr[25]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Negative-going Threshold Hysteresis = 0x%02X\n", pSdr[26]);
	vConsoleWrite(buffer);
	// pSdr offset 27 to 29 reserved
	sprintf (buffer, "OEM = 0x%02X\n", pSdr[30]);
	vConsoleWrite(buffer);
	sprintf (buffer, "ID String Type/Length Code = 0x%02X\n", pSdr[31]);
	vConsoleWrite(buffer);

	switch (pSdr[31] & 0xC0) 
	{
		case 0x00:
			sprintf (buffer, "ID String Type = unicode\n");
			break;
	
		case 0x40:
			sprintf (buffer, "ID String Type = BCD Plus\n");
			break;
	
		case 0x80:
			sprintf (buffer, "ID String Type = 6 Bit ascii\n");
			break;
	
		case 0xC0:
			sprintf (buffer, "ID String Type = 8 Bit ascii\n");
			break;
	}
	vConsoleWrite(buffer);

	if (((pSdr[31] & 0x1F) != 0) && ((pSdr[31] & 0x1F) != 0x1F))
	{
		memset(buffer, 0x00, 80 );
		for (i = 0; i < (pSdr[31] & 0x1F); ++i)
		{
			buffer[i] = pSdr[32 + i];
		}
		vConsoleWrite(buffer);
		vConsoleWrite("\n");
	}
	vConsoleWrite("\n");
}

/*****************************************************************************
 *
 * vDecodeSensorReading: Dump Sensor Reading.
 *
 */
void vDecodeSensorReading(UINT8 pSr[], UINT8 units, UINT8 scaling)
{
	vConsoleWrite("\nSensor Reading Decode:\n");
	sprintf (buffer, "Completion Code = 0x%02X\n", pSr[0]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Reading [raw analog]= 0x%02X\n", pSr[1]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Sensor Status = 0x%02X\n", pSr[2]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Present Threshold Status/Current State Mask = 0x%02X\n", pSr[3]);
	vConsoleWrite(buffer);
	sprintf (buffer, "Optional/Reserved = 0x%02X\n", pSr[4]);
	vConsoleWrite(buffer);

	if (units == 4)			// voltage
	{
		sprintf (buffer, "\nCurrent Reading = %d\n\n", (((UINT16) pSr[1]) * scaling) / 10);
		vConsoleWrite(buffer);
	}
	else 					// temperature or not an analog sensor
	{
		sprintf (buffer, "\nCurrent Reading = %d\n\n", (INT8) pSr[1]);
		vConsoleWrite(buffer);
	}
}
#endif


/*****************************************************************************
 * fRawToUnits: converts a raw value from the sensor into a real
 *              reading in valid units.
 *
 * RETURNS: result
 */

double fRawToUnits (UINT8 raw, UINT8 units, UINT16 M, UINT16 B,
		    UINT8 B_Exp, UINT8 R_Exp)
{
	double fM, fB, fK1, fK2;
	double fRaw;
	char buffer[80];

	/* check the caller doesn't want non-analog units */

	units = units >> 6;
	if (units == 0x03)
		return 0.0;

	if (units == 0x01)
	{
		/* 1's complement units */
		fRaw = (raw < 128) ? (double)raw : (double)(raw - 255);
	}
	else if (units == 0x02)
	{
		/* 2's complement units */
		fRaw = (raw < 128) ? (double)raw : (double)(raw - 256);
	}
	else
	{
		/* unsigned units */
		fRaw = (double)raw;
	}

	/* convert M & B values to 10-bit 2's complement */

	fM = (M < 512) ? M : (double)M - 1024.0;
	fB = (B < 512) ? B : (double)B - 1024.0;

	/* convert R exp & B exp values to 4-bit 2's complement */

	fK1 = (B_Exp < 8) ? B_Exp : (double)B_Exp - 16;
	fK2 = (R_Exp < 8) ? R_Exp : (double)R_Exp - 16;

	/* convert complemented raw with linearisation */
	if (fpuFlag == 1)
	{
		sprintf (buffer, "fRawToUnits: FPU Exception Occurred\n");
		vConsoleWrite(buffer);
		return (E__FPU_EXP_ERROR);
	}

	return ((fM * fRaw) + (fB * exponent (fK1))) * exponent (fK2);
}

#if 0
//
// This function is not implemented in any of the current IPMI micros
// There is no requirement for the IPMI controller to generate interrupts
// for the conditions that this test was trying to validate!
//
// This code can be removed at a later date if this feature remains unimplemented
//

/*****************************************************************************
 * dCheckThresholdEvent: check if a threshold event has occurred
 *
 * RETURNS: threshold event status
 */

UINT32 dCheckThresholdEvent
(
	UINT8	bSensNum,
	UINT16	wThreshMask,
	UINT16	wM,
	UINT16	wB,
	UINT8	bK1,
	UINT8	bK2,
	UINT8 *	abThreshold,
	double	fReading,
	UINT8	deAssert,
	UINT8	bSensUnits1,
	UINT8	bSensUnits2
)
{
	UINT16	wThreshField;
	UINT32	dStatus;
	UINT32	dThreshStatus;
	double	fThresh;
	char *	strPtr;
	char	cUnits;
	UINT8	idx;

	dStatus = E__OK;
	cUnits = bSensUnits2 == 1 ? 'C' : bSensUnits2 == 4 ? 'V' : ' ';

#ifdef DEBUG
	sprintf (buffer, "Valid thresholds to check = 0x%04X\n", wThreshMask);
	vConsoleWrite(buffer);
#endif

	for (idx = 0; idx < 12; idx++)
	{
		dThreshStatus = E__OK;
		
		/* test this threshold is masked */

		wThreshField = 0x0001 << idx;

		if (wThreshMask & wThreshField)
		{

			/* convert raw threshold to real value */

			fThresh = fRawToUnits (abThreshold[idx >> 1], bSensUnits1, wM, wB, bK1, bK2);

#ifdef DEBUG
			sprintf (buffer, "ThresholdIndex = %d\n", idx);
			vConsoleWrite(buffer);
			sprintf (buffer, "ThresholdMask = %04X\n", wThreshField);
			vConsoleWrite(buffer);
			
			switch (wThreshField)
			{
				case 0x0001:
					strPtr = "Lower non-critical going low";
					break;
					
				case 0x0002:
					strPtr = "Lower non-critical going high";
					break;
					
				case 0x0004:
					strPtr = "Lower critical going low";
					break;
					
				case 0x0008:
					strPtr = "Lower critical going high";
					break;
					
				case 0x0010:
					strPtr = "Lower non-recoverable going low";
					break;
					
				case 0x0020:
					strPtr = "Lower non-recoverable going high";
					break;
					
				case 0x0040:
					strPtr = "Upper non-critical going low";
					break;
					
				case 0x0080:
					strPtr = "Upper non-critical going high";
					break;
					
				case 0x0100:
					strPtr = "Upper critical going low";
					break;
					
				case 0x0200:
					strPtr = "Upper critical going high";
					break;
					
				case 0x0400:
					strPtr = "Upper non-recoverable going low";
					break;
					
				case 0x0800:
					strPtr = "Upper non-recoverable going high";
					break;
			}

			sprintf (buffer, "%s %s Threshold = %d.%02d%c\n", 
						strPtr,
						deAssert == 0 ? "assertion" : "deassertion",
						(short int) fThresh,
						(short int) (((fThresh + 0.001) - (short int)fThresh) * 100.0),
						cUnits);
			vConsoleWrite(buffer);

			sprintf (buffer, "Reading = %d.%02d%c\n",
						(short int) fReading,
						(short int) (((fReading + 0.001) - (short int)fReading) * 100.0),
						cUnits);
			vConsoleWrite(buffer);
#endif

			/* determine high or low threshold & compare against reading */
			switch (wThreshField)
			{
				case 0x0001:
				case 0x0002:
					strPtr = "Lower non-critical";
					break;
					
				case 0x0004:
				case 0x0008:
					strPtr = "Lower critical";
					break;
					
				case 0x0010:
				case 0x0020:
					strPtr = "Lower non-recoverable";
					break;
					
				case 0x0040:
				case 0x0080:
					strPtr = "Upper non-critical";
					break;
					
				case 0x0100:
				case 0x0200:
					strPtr = "Upper critical";
					break;
					
				case 0x0400:
				case 0x0800:
					strPtr = "Upper non-recoverable";
					break;
			}

			if ((fReading > fThresh) && (deAssert == 0))
			{
				switch (wThreshField)
				{
					case 0x0002:			// reading exceeds upper threshold
					case 0x0008:
					case 0x0020:
					case 0x0080:
					case 0x0200:
					case 0x0800:
						if (Inbyte(0x215) & (1 << 6))
						{
							dThreshStatus = E__OK;
						}
						else
						{
							dThreshStatus = E__BIT | (UINT32)(bSensNum << 20) | (UINT32)(deAssert << 16) | (UINT32) wThreshField;
#ifdef DEBUG
							sprintf (buffer,
										"! More than %s threshold %d.%02d%s [%02Xh] 0x215: %02Xh\n",
										strPtr,
										(short int)fThresh,
										(short int)(((fThresh + 0.001) - ((short int) fThresh)) * 100.0),
										bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
										abThreshold[idx >> 1],
										Inbyte(0x215));
							vConsoleWrite(buffer);
#endif
						}
						break;
						
					default:
					
						break;
				}
			}
			else if ((fReading < fThresh) && (deAssert == 0))
			{
				switch (wThreshField)
				{
					case 0x0001:			// reading exceeds lower threshold
					case 0x0004:
					case 0x0010:
					case 0x0040:
					case 0x0100:
					case 0x0400:
						if (Inbyte(0x215) & (1 << 6))
						{
							dThreshStatus = E__OK;
						}
						else
						{
							dThreshStatus = E__BIT | (UINT32)(bSensNum << 20) | (UINT32)(deAssert << 16) | (UINT32) wThreshField;
#ifdef DEBUG
							sprintf (buffer,
										"! Less than  %s threshold %d.%02d%s [%02Xh] 0x215: %02Xh\n",
										strPtr,
										(short int)fThresh,
										(short int)(((fThresh + 0.001) - ((short int) fThresh)) * 100.0),
										bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
										abThreshold[idx >> 1],
										Inbyte(0x215));
							vConsoleWrite(buffer);
#endif
						}
						break;
						
					default:
					
						break;
				}
			}
			else if ((fReading > fThresh) && (deAssert == 1))
			{
				switch (wThreshField)
				{
					case 0x0002:			// reading within upper threshold
					case 0x0008:
					case 0x0020:
					case 0x0080:
					case 0x0200:
					case 0x0800:
						if (Inbyte(0x215) & (1 << 6))
						{
							dThreshStatus = E__OK;
						}
						else
						{
							dThreshStatus = E__BIT | (UINT32)(bSensNum << 20) | (UINT32)(deAssert << 16) | (UINT32) wThreshField;
#ifdef DEBUG
							sprintf (buffer,
										"! Within %s threshold %d.%02d%s [%02Xh] 0x215: %02Xh\n",
										strPtr,
										(short int)fThresh,
										(short int)(((fThresh + 0.001) - ((short int) fThresh)) * 100.0),
										bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
										abThreshold[idx >> 1],
										Inbyte(0x215));
							vConsoleWrite(buffer);
#endif
						}
						break;
						
					default:
					
						break;
				}
			}
			else if ((fReading < fThresh) && (deAssert == 1))
			{
				switch (wThreshField)
				{
					case 0x0001:			// reading within lower threshold
					case 0x0004:
					case 0x0010:
					case 0x0040:
					case 0x0100:
					case 0x0400:
						if (Inbyte(0x215) & (1 << 6))
						{
							dThreshStatus = E__OK;
						}
						else
						{
							dThreshStatus = E__BIT | (UINT32)(bSensNum << 20) | (UINT32)(deAssert << 16) | (UINT32) wThreshField;
#ifdef DEBUG
							sprintf (buffer,
										"! Within %s threshold %d.%02d%s [%02Xh] 0x215: %02Xh\n",
										strPtr,
										(short int)fThresh,
										(short int)(((fThresh + 0.001) - ((short int) fThresh)) * 100.0),
										bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
										abThreshold[idx >> 1],
										Inbyte(0x215));
							vConsoleWrite(buffer);
#endif
						}
						break;
						
					default:
					
						break;
				}
			}


#ifdef DEBUG
			sprintf (buffer, "ThresholdStatus = %02X\n", dThreshStatus);
			vConsoleWrite(buffer);
#endif
			if (dThreshStatus != E__OK)
			{
				sprintf (buffer, "Sensor Number [%d]: Reading = %3d.%02d%c, %s threshold = %d.%02d%c\n",
							bSensNum,
							(short int) fReading,
							(short int) (((fReading + 0.001) - ((short int) fReading)) * 100.0),
							cUnits,
							strPtr,
							(short int) fThresh,
							(short int) (((fThresh + 0.001) - ((short int) fThresh)) * 100.0),
							cUnits);
				vConsoleWrite(buffer);
				dStatus |= dThreshStatus;
			}
		}
	}	/* end for() */
	
	return dStatus;
}
#endif

#if 0
//
// This function is not implemented in any of the current IPMI micros
// There is no requirement for the IPMI controller to generate interrupts
// for the conditions that this test was trying to validate!
//
// This code can be removed at a later date if this feature remains unimplemented
//

/*****************************************************************************
 * dTestAlarm:  tests interrupt generation when sensor reading exceeds the
 *				thresholds described in the corresponding SDR.
 *				pSdr points to the first byte
 *              of an SDR record in memory.
 *
 * RETURNS: E__OK or an E__... error code
 */

UINT32 dTestAlarm (UINT8 bReading, UINT8 pSdr[], UINT16 wRecordId)
{
	UINT32	dThreshStatus;
	UINT16	wM, wB;
	UINT8	bK1, bK2;
	UINT8	bSensNum;
	UINT8	bSensUnits1;
	UINT8	bSensUnits2;
	UINT16	wThreshAssertMask;
	UINT16	wThreshDeAssertMask;
	UINT16	wThreshReadableMask;
	UINT16	wThresholdMask;
	UINT8	abThreshold[6];
	volatile double fReading; // fNominal;
	UINT8	cnt;

	dThreshStatus = E__OK;

	/* take what we need from the SDR */

	bSensNum    = pSdr[7];
	bSensUnits1 = pSdr[20];
	bSensUnits2 = pSdr[21];
	wThreshAssertMask = ((pSdr[14] * 256) + pSdr[15]) & 0x7FF;
	wThreshDeAssertMask = ((pSdr[16] * 256) + pSdr[17]) & 0x7FF;
	wThreshReadableMask = ((pSdr[18] * 256) + pSdr[19]) & 0x7FF;
	
	for (cnt = 0; cnt < 6; ++cnt)
	{
		wThresholdMask = 0x0001 << cnt;
		if ((wThreshReadableMask & wThresholdMask) == 0)
		{
			// only check readings against valid thresholds, so remove those that are invalid
			switch (wThresholdMask)
			{
				case LOWER_NON_CRITICAL_READABLE:
					wThreshAssertMask &= ~(LOWER_NON_CRITICAL_GOING_LOW + LOWER_NON_CRITICAL_GOING_HIGH);
					wThreshDeAssertMask &= ~(LOWER_NON_CRITICAL_GOING_LOW + LOWER_NON_CRITICAL_GOING_HIGH);
					break;

				case LOWER_CRITICAL_READABLE:
					wThreshAssertMask &= ~(LOWER_CRITICAL_GOING_LOW + LOWER_CRITICAL_GOING_HIGH);
					wThreshDeAssertMask &= ~(LOWER_CRITICAL_GOING_LOW + LOWER_CRITICAL_GOING_HIGH);
					break;

				case LOWER_NON_RECOVERABLE_READABLE:
					wThreshAssertMask &= ~(LOWER_NON_RECOVERABLE_GOING_LOW + LOWER_NON_RECOVERABLE_GOING_HIGH);
					wThreshDeAssertMask &= ~(LOWER_NON_RECOVERABLE_GOING_LOW + LOWER_NON_RECOVERABLE_GOING_HIGH);
					break;

				case UPPER_NON_CRITICAL_READABLE:
					wThreshAssertMask &= ~(UPPER_NON_CRITICAL_GOING_LOW + UPPER_NON_CRITICAL_GOING_HIGH);
					wThreshDeAssertMask &= ~(UPPER_NON_CRITICAL_GOING_LOW + UPPER_NON_CRITICAL_GOING_HIGH);
					break;

				case UPPER_CRITICAL_READABLE:
					wThreshAssertMask &= ~(UPPER_CRITICAL_GOING_LOW + UPPER_CRITICAL_GOING_HIGH);
					wThreshDeAssertMask &= ~(UPPER_CRITICAL_GOING_LOW + UPPER_CRITICAL_GOING_HIGH);
					break;

				case UPPER_NON_RECOVERABLE_READABLE:
					wThreshAssertMask &= ~(UPPER_NON_RECOVERABLE_GOING_LOW + UPPER_NON_RECOVERABLE_GOING_HIGH);
					wThreshDeAssertMask &= ~(UPPER_NON_RECOVERABLE_GOING_LOW + UPPER_NON_RECOVERABLE_GOING_HIGH);
					break;
			}
		}
	}

	wM          = pSdr[24] + (pSdr[25] >> 6) * 256;
	wB          = pSdr[26] + (pSdr[27] >> 6) * 256;
	bK1         = pSdr[29] & 0x0F;
	bK2         = pSdr[29] >> 4;

	/* (note thresholds stored in reverse order from SDR) */

	abThreshold [0] = pSdr [41]; /* lower non-critical  */
	abThreshold [1] = pSdr [40]; /* lower critical  */
	abThreshold [2] = pSdr [39]; /* lower non-recv  */
	abThreshold [3] = pSdr [38]; /* upper non-critical  */
	abThreshold [4] = pSdr [37]; /* upper critical  */
	abThreshold [5] = pSdr [36]; /* upper non-recv  */

	/* convert raw readings to real values */

	// fNominal = fRawToUnits (bNominal, bSensUnits1, wM, wB, bK1, bK2);
	fReading = fRawToUnits (bReading, bSensUnits1, wM, wB, bK1, bK2);

#ifdef DEBUG
	sprintf (buffer, "ThresholdAssertionMask = %04X\n", wThreshAssertMask);
	vConsoleWrite(buffer);
	sprintf (buffer, "ThresholdDeAssertionMask = %04X\n", wThreshDeAssertMask);
	vConsoleWrite(buffer);
	sprintf (buffer, "ThresholdReadableMask = %04X\n", wThreshReadableMask);
	vConsoleWrite(buffer);
	sprintf (buffer, "Reg 0x215 = %02X\n", Inbyte(0x215));
	vConsoleWrite(buffer);
#endif

	if (wThreshAssertMask != 0)		// something to check?
	{
#ifdef DEBUG
		sprintf (buffer, "%s %s : SensorNumber = %d\n", __FUNCTION__, "AssertThresholdEventCheck", bSensNum);
		vConsoleWrite(buffer);
#endif
	// check for threshold assertion events
		dThreshStatus = dCheckThresholdEvent(bSensNum, wThreshAssertMask, wM, wB, bK1, bK2, &abThreshold[0], fReading, 0, bSensUnits1, bSensUnits2);
	}
	if ((dThreshStatus == E__OK) && (wThreshDeAssertMask != 0))		// everything OK and something to check?
	{
#ifdef DEBUG
		sprintf (buffer, "%s %s : SensorNumber = %d\n", __FUNCTION__, "DeAssertThresholdEventCheck", bSensNum);
		vConsoleWrite(buffer);
#endif
		// check for threshold deassertion events
		dThreshStatus = dCheckThresholdEvent(bSensNum, wThreshDeAssertMask, wM, wB, bK1, bK2, &abThreshold[0], fReading, 1, bSensUnits1, bSensUnits2);
	}

	return dThreshStatus;
}
#endif

/*****************************************************************************
 * dTestThresholds: tests a sensor reading against the thresholds described
 *                  in the corresponding SDR. pSdr points to the first byte
 *                  of an SDR record in memory.
 *
 * RETURNS: E__OK or an E__... error code
 */
UINT32 dTestThresholds
	(
		UINT8 bReading,
		UINT8 pSdr[],
		UINT8 flag,
		UINT8 *pSensNum,
		INT32 *pReading
	)
{
	UINT32 dThreshStatus;
	UINT16 wM, wB;
	UINT8  bK1, bK2;
	UINT8  bSensNum, bSensUnits1, bSensUnits2;
	UINT8  bNominal;
	// UINT8 bSensInit;
	UINT16 wThreshMask, wThreshField, fThreshField;
	UINT8  abThreshold [6];
	UINT8  idx, i;
	double fReading = 0.0, fNominal= 0.0, fThresh = 0.0;
	char buffer[80];
	UINT32 readingWholePart = 0;
	UINT32 readingFractionalPart = 0;
	UINT32 readingNegative = 0;
	UINT32 thresholdWholePart = 0;
	UINT32 thresholdFractionalPart = 0
	/*UINT32 thresholdNegative = 0*/;
	// volatile int dontoptimize = 0;

	/* take what we need from the SDR */

	bSensNum    = pSdr[7];
	// bSensInit   = pSdr[10];
	bSensUnits1 = pSdr[20];
	bSensUnits2 = pSdr[21];
	bNominal    = pSdr[31];
	wThreshMask = pSdr[18] * 256 + pSdr[19];			// threshold readable mask

	wM          = pSdr[24] + (pSdr[25] >> 6) * 256;
	wB          = pSdr[26] + (pSdr[27] >> 6) * 256;
	bK1         = pSdr[29] & 0x0F;
	bK2         = pSdr[29] >> 4;

	/* note: thresholds stored in reserve order from SDR    */
	/* to match the bitmap of the 'threshold readable mask' */

	abThreshold [0] = pSdr [41]; /* lower non-critical    */
	abThreshold [1] = pSdr [40]; /* lower critical        */
	abThreshold [2] = pSdr [39]; /* lower non-recoverable */
	abThreshold [3] = pSdr [38]; /* upper non-critical    */
	abThreshold [4] = pSdr [37]; /* upper critical        */
	abThreshold [5] = pSdr [36]; /* upper non-recoverable */

#ifdef DEBUG_VERBOSE
	vConsoleWrite ("Sensor\t Reading\t LC\t    UC\t\tName\n\n");
#endif

	/* convert raw readings to real values */

	fNominal = fRawToUnits (bNominal, bSensUnits1, wM, wB, bK1, bK2);
	if (fNominal == E__FPU_EXP_ERROR)
	{
		return (E__FPU_EXP_ERROR);
	}

	fReading = fRawToUnits (bReading, bSensUnits1, wM, wB, bK1, bK2);
	if (fReading == E__FPU_EXP_ERROR)
	{
		return (E__FPU_EXP_ERROR);
	}

	if(bSensNum == 0)// readingNegative = 0, thresholdNegative = 0, UC
	{
		AmbientTemp = fReading;
	}

	// for all boards except tp702
	if(bSensNum == 1)
		CPUTemp = fReading;

	//for tp702 only
	//if(bSensNum == 1) // readingNegative = 1, thresholdNegative = 1 lc
	//{
	//	tempsenseDTS(&cpuIntTemp);
	//	 CPUTemp = cpuIntTemp;
	// 	fReading = CPUTemp;
	//}

	IPMIDBG( ("dTestThresholds: Num: 0x%02x Units: 0x%02x Reading: 0x%02x Nominal: 0x%02x\n",
					bSensNum, bSensUnits1, bReading, bNominal) );
	/* IPMIDBG( ("dTestThresholds: wM: 0x%04x wB: 0x%04x bK1: 0x%02x bK2: 0x%02x\n",
					wM, wB, bK1, bK2) ); */

	sprintf (buffer,"%2X\t %3d.%02d%s\t ", bSensNum, (short int)fReading,
				(short int)(((fReading + 0.001) - (short int)fReading) * 100.0),
				bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");

	vConsoleWrite(buffer);

	*pSensNum = bSensNum;
	*pReading = (INT32)fReading;
	readingWholePart = (UINT32)fReading;
	readingFractionalPart = (UINT16)(((fReading + 0.001) - (short int)fReading) * 100.0);
	readingNegative = 0;
	if (readingWholePart & 0x80000000)
	{
		readingWholePart = ~(readingWholePart - 1);
		readingNegative = 1;
	}

	fThreshField = 0;
	dThreshStatus = E__OK;

	/* loop through thresholds */
	for (idx = 0; idx < 6; idx++)
	{
		/* check if this threshold is readable */

		wThreshField = 0x0001 << idx;
		if (wThreshMask & wThreshField)
		{
			/* convert raw threshold to real value */
			fThresh = fRawToUnits(abThreshold[idx], bSensUnits1,
			  						wM, wB, bK1, bK2);

			thresholdWholePart = (UINT32)fThresh;
			thresholdFractionalPart = (UINT16)(((fThresh + 0.001) - (short int)fThresh) * 100.0);
			/* thresholdNegative = 0; */
			if (thresholdWholePart & 0x80000000)
			{
				thresholdWholePart = ~(thresholdWholePart - 1);
				/* thresholdNegative = 1; */
			}

			// lower critical threshold
			if (wThreshField == 0x0002)
			{
				sprintf (buffer,"%3d.%02d%s    ", (short int) fThresh,
					(short int)(((fThresh + 0.001) - (short int) fThresh) * 100.0),
					bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");
				vConsoleWrite(buffer);
			}

			// upper critical threshold
			if (wThreshField == 0x0010)
			{
				sprintf (buffer,"%3d.%02d%s\t", (short int) fThresh,
						(short int)(((fThresh + 0.001) - (short int)fThresh) * 100.0),
				bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");
				vConsoleWrite(buffer);
			}

			/* determine high or low threshold & compare against reading */
			if (flag == 1) // 1 means no error has occurred earlier so check threshold
			{
				/* Test should not fail if UNC or LNC, LC, LNR crossed */
				/* so we now only check UC and UNR */

				// upper critical  or upper non-recoverable threshold
				// We check the upper non-recoverable threshold only for temperature sensors
				// as it is not always populated for voltage sensor
				if ((wThreshField == 0x0010) || (wThreshField == 0x0020 && bSensUnits2==1))
				{
					// UC thresholds will never be negative no point comparing if reading is negative
					if (readingNegative == 0 )
					{
						// check 'whole' part of number
						if (readingWholePart > thresholdWholePart)
						{
							fThreshField  = wThreshField;
							dThreshStatus = (wThreshField << 8) | bSensNum;
							break;
						}
						// check 'fractional' part of number
						else if ((readingWholePart == thresholdWholePart) && (readingFractionalPart > thresholdFractionalPart))
						{
							fThreshField  = wThreshField;
							dThreshStatus = (wThreshField << 8) | bSensNum;
							break;
						}
					}
				}
#if 0
				// upper non-critical threshold
				else if (wThreshField < 0x0008)
				{
					if ((readingNegative == 0) && (thresholdNegative == 0))
					{
						// both thresholds are positive
						if (readingWholePart < thresholdWholePart)
						{
							fThreshField  = wThreshField;
							dThreshStatus = (wThreshField << 8) | bSensNum;
							break;
						}
						else if ((readingWholePart == thresholdWholePart) && (readingFractionalPart < thresholdFractionalPart))
						{
							fThreshField  = wThreshField;
							dThreshStatus = (wThreshField << 8) | bSensNum;
							break;
						}
					}
					else if ((readingNegative ==0) && (thresholdNegative == 1))
					{
						//no need to compare, threshold is negative but reading is positive!
						if (dontoptimize == 0)
						dontoptimize = 1;
					}
					else if ((readingNegative ==1) && (thresholdNegative == 0))
					{
						//reading is negative while threshold is positive! fail!
						fThreshField  = wThreshField;
						dThreshStatus = (wThreshField << 8) | bSensNum;
						break;
					}
					else
					{
						//both thresholds are negative
						if (readingWholePart > thresholdWholePart)
						{
							fThreshField  = wThreshField;
							dThreshStatus = (wThreshField << 8) | bSensNum;
							break;
						}
						else if ((readingWholePart == thresholdWholePart) && (readingFractionalPart > thresholdFractionalPart))
						{
							fThreshField  = wThreshField;
							dThreshStatus = (wThreshField << 8) | bSensNum;
							break;
						}
					}
				}
#endif
			}
		}
	}  /* end for() */


	// get sensor name
	if (((pSdr[47] & 0x1f) > 0) && ((pSdr[47] & 0x1f) < 0x1f))
	{
		memset(buffer, 0x00, 80 );
		for (i = 0; i < (pSdr[47] & 0x1f); i++)
		{
			buffer[i] = pSdr[48 + i];
		}
		vConsoleWrite (buffer);
	}
	vConsoleWrite ("\n");


	if (dThreshStatus != E__OK)
	{
		IPMIDBG (("dTestThresholds: fThreshField: 0x%04x fReading: %d.%02d%s fThresh %d.%02d%s\n", fThreshField,
					(short int)fReading, (short int)(((fReading + 0.001) - (short int)fReading) * 100.0),
					bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
					(short int)fThresh, (short int)(((fThresh + 0.001) - (short int)fThresh) * 100.0),
					bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : ""));

		switch (fThreshField)
		{

#if 0
			case 0x0001:	/* Lower Non-Critical */
				sprintf (errorbuffer, "\"%s\"=%3d.%02d%s < LNC=%3d.%02d%s",
							buffer, (short int)fReading,
							(short int)(((fReading + 0.001) - (short int)fReading) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
							(short int)fThresh, (short int)(((fThresh + 0.001) - (short int)fThresh) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");
				vConsoleWrite (errorbuffer);
				/* Test should not fail if LNC crossed */
				dThreshStatus = E__OK;
				break;
#endif

			case 0x0002:	/* Lower Critical */
				sprintf (errorbuffer, "\"%s\"=%3d.%02d%s < LC=%3d.%02d%s",
							buffer, (short int)fReading,
							(short int)(((fReading + 0.001) - (short int)fReading) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
							(short int)fThresh, (short int)(((fThresh + 0.001) - (short int)fThresh) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");
				vConsoleWrite (errorbuffer);
				/* Test should not fail if LC crossed */
				dThreshStatus = E__OK;
				break;

#if 0
			case 0x0004:	/* Lower Non-Recoverable */
				sprintf (errorbuffer, "\"%s\"=%3d.%02d%s < LNR=%3d.%02d%s",
							buffer, (short int)fReading,
							(short int)(((fReading + 0.001) - (short int)fReading) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
							(short int)fThresh, (short int)(((fThresh + 0.001) - (short int)fThresh) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");
				vConsoleWrite (errorbuffer);
				/* Test should not fail if LNR crossed */
				dThreshStatus = E__OK;
				break;

			case 0x0008:	/* Upper Non-Critical */
				sprintf (errorbuffer, "\"%s\"=%3d.%02d%s > UNC=%3d.%02d%s",
							buffer, (short int)fReading,
							(short int)(((fReading + 0.001) - (short int)fReading) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
							(short int)fThresh, (short int)(((fThresh + 0.001) - (short int)fThresh) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");
				vConsoleWrite (errorbuffer);
				/* Test should not fail if UNC crossed */
				dThreshStatus = E__OK;
				break;
#endif

			case 0x0010:	/* Upper Critical */
				sprintf (errorbuffer, "\"%s\"=%3d.%02d%s > UC=%3d.%02d%s",
							buffer, (short int)fReading,
							(short int)(((fReading + 0.001) - (short int)fReading) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
							(short int)fThresh, (short int)(((fThresh + 0.001) - (short int)fThresh) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");
				vConsoleWrite (errorbuffer);
				break;

			case 0x0020: 	/* Upper Non-Recoverable */
				sprintf (errorbuffer, "\"%s\"=%3d.%02d%s > UNR=%3d.%02d%s",
							buffer, (short int)fReading,
							(short int)(((fReading + 0.001) - (short int)fReading) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "",
							(short int)fThresh, (short int)(((fThresh + 0.001) - (short int)fThresh) * 100.0),
							bSensUnits2 == 1 ? "C" : bSensUnits2 == 4 ? "V" : "");
				vConsoleWrite (errorbuffer);
				break;

			default:
				vConsoleWrite("   \tSDR Data Corruption!\n\n");
				break;
		}
		vConsoleWrite ("\n\n");
	}


	if (fpuFlag == 1)
	{
		sprintf (buffer, "2. FPU Exception Occured\n");
		vConsoleWrite(buffer);
	}
	return dThreshStatus;
}


/*****************************************************************************
 * dIpmiSensorThresholdTest: Obtains an SDR record and corresponding sensor
 *								reading and tests against threshold values
 *
 * RETURNS: E__OK or an E__... error code
 */

UINT32 dIpmiSensorThresholdTest (void)
{
	UINT32 dTestStatus   = E__OK;
	UINT32 dThreshStatus = E__OK;
	UINT8  abSdrRq [SMS_LEN_BUFFER];
	UINT8  abSdrRs [SMS_LEN_BUFFER];
	UINT8  abReadRq [SMS_LEN_BUFFER];
	UINT8  abReadRs [SMS_LEN_BUFFER];
	UINT16 wRecordId, wNextRecordId;
	UINT8  bRsLen;
	INT32 tempReading;
	UINT8 bSensNum;

	CPUTemp     = 0.0;
	AmbientTemp = 0.0;

#ifdef INCLUDE_AM94X_VARIANT_IPMI_WORKAROUND
	if (ipmiVer == 0)
	{
		/* Request Get Device ID */
		abReadRq [0] = (NFC_APP_REQUEST << 2) | (LUN_BMC & 0x03);	/* NFC + LUN */
		abReadRq [1] = CMD_GET_DEVICE_ID;	/* command */

		memset(abReadRs, 0, SMS_LEN_BUFFER );

		if (dIpmiSendRequest (&abReadRq[0], 2, &abReadRs[0], &bRsLen) == E__OK)
		{
			ipmiVer = (UINT16)((UINT16)(abReadRs[5] & 0x7f) << 8) + abReadRs[6];
			IPMIDBG (("IPMI firmware version: %d.%02d (0x%04x)\n", (abReadRs[5] & 0x7f), abReadRs[6], ipmiVer));
		}
		else
		{
			IPMIDBG( ("Error - failed to IPMI firmware version\n") );
		}
	}
#endif

	/* loop through the SDR Repository */

	vConsoleWrite ("\nSensor\t Reading\t LC\t    UC\t\tName\n\n");

	for (wRecordId = 0x0000; wRecordId != 0xFFFF; wRecordId = wNextRecordId)
	{
		bSensNum = 0;
		tempReading = 0;

		/* issue IPMI command to get (entire) SDR */

		abSdrRq [0] = (NFC_STORAGE_REQUEST << 2) |(LUN_BMC & 0x03);
		abSdrRq [1] = CMD_GET_SDR_ENTRY;
		abSdrRq [2] = 0x00;
		abSdrRq [3] = 0x00;
		abSdrRq [4] = wRecordId & 0xFF;
		abSdrRq [5] = (wRecordId >> 8) & 0xFF;
		abSdrRq [6] = 0x00;
		abSdrRq [7] = 0xFF;

		dTestStatus = dIpmiSendRequest (&abSdrRq[0], 8, &abSdrRs [0], &bRsLen);
		if (dTestStatus != E__OK)
			break;

		/* extract next record ID from response */

		wNextRecordId = abSdrRs [3] + 256 * abSdrRs[4];

		/* is this a Full Sensor Record SDR? */

		if (abSdrRs[3+5] == 0x01)
		{
#ifdef DEBUG_VERBOSE
			vDecodeFullSensorRecord(&abSdrRs[5]);
#endif
			/* attempt to read the sensor for this SDR */

			abReadRq [0] = (NFC_SENSOR_EVENT << 2) | (LUN_BMC & 0x03);
			abReadRq [1] = CMD_GET_SENS_RD;
			abReadRq [2] = abSdrRs[7+5];	/* sensor # from SDR */

			dTestStatus = dIpmiSendRequest (&abReadRq[0], 3, &abReadRs [0], &bRsLen);
			
#ifdef DEBUG_VERBOSE
			vDecodeSensorReading(&abReadRs[2], abSdrRs[5+21], abSdrRs[5+24]);
#endif

			if (dTestStatus != E__OK)
			{
				if (dTestStatus == E__COMP_CODE)
				{
					dTestStatus = E__NO_SENSOR;/* sensor not found */
				}
				break;
			}

			if (dThreshStatus == E__OK)
			{
				dThreshStatus = dTestThresholds (abReadRs[3], &abSdrRs[5], 1, &bSensNum, &tempReading);

#ifdef INCLUDE_AM94X_VARIANT_IPMI_WORKAROUND
				if (bSensNum == 0x1)
				{
					tempReading1 = tempReading;
				}

				if (bSensNum == 0x2)
				{
					tempReading2 = tempReading;

					/* Workaround for AM 94x IPMI firmware bug */
					/* where negative temperatures on sensor2 are reported */
					/* incorrectly as high positive temperatures so here */
					/* we force a test pass if sensor2 is higher than sensor1 */
					/* i.e. if the board temperature is higher than the CPU temperature */
					if ( (dThreshStatus > 0) && (dThreshStatus < 6) )
					{
						IPMIDBG (("dThreshStatus: 0x%x Sensor1: %d Sensor2: %d\n", dThreshStatus, tempReading1, tempReading2));

						/* Only for these IPMI firmware versions */
						if (((ipmiVer == 0x0204) || (ipmiVer == 0x0205)) && (tempReading2 > tempReading1))
						{
							IPMIDBG (("Setting dThreshStatus: from 0x%x to 0x%x\n", dThreshStatus, E__OK));
							dThreshStatus = E__OK;
						}
					}
				}
#endif
			}
			else
			{
				/* don't save error if already seen an error */
				dTestThresholds (abReadRs[3], &abSdrRs[5], 0, &bSensNum, &tempReading);// don't check thresholds
			}
		}

	} /* end for() */

	vConsoleWrite ("\n");

	/* adjust error code for any threshold error */

	if (dTestStatus == E__OK && dThreshStatus != E__OK)
	{
		dTestStatus = E__BIT | dThreshStatus;

		IPMIDBG (("dIpmiSensorThresholdTest: dTestStatus: 0x%x dThreshStatus: 0x%x\n", dTestStatus, dThreshStatus));

		sprintf (buffer, "CPU=%3d.%02dC Amb=%3d.%02dC, %s",
					(short int)CPUTemp, (short int)(((CPUTemp + 0.001) - (short int)CPUTemp) * 100.0),
					(short int)AmbientTemp, (short int)(((AmbientTemp + 0.001) - (short int)AmbientTemp) * 100.0),
					errorbuffer);
		postErrorMsg (buffer);
	}
	else if (dTestStatus != E__OK)
	{
		IPMIDBG (("dIpmiSensorThresholdTest: dTestStatus: 0x%x dThreshStatus: 0x%x\n", dTestStatus, dThreshStatus));

		sprintf (buffer, "[%x] CPU=%3d.%02dC Amb=%3d.%02dC", dTestStatus,
					(short int)CPUTemp, (short int)(((CPUTemp + 0.001) - (short int)CPUTemp) * 100.0),
					(short int)AmbientTemp, (short int)(((AmbientTemp + 0.001) - (short int)AmbientTemp) * 100.0));
		postErrorMsg (buffer);
	}

	return (dTestStatus);
}

#if 0
//
// This function is not implemented in any of the current IPMI micros
// There is no requirement for the IPMI controller to generate interrupts
// for the conditions that this test was trying to validate!
//
// This code can be removed at a later date if this feature remains unimplemented
//

/*****************************************************************************
 * dIpmiSensorAlarmTest: Obtains an SDR record and corresponding sensor
 *						 reading and tests against interrupt generation
 *
 * RETURNS: E__OK or an E__... error code
 */

UINT32 dIpmiSensorAlarmTest (void)
{
	UINT32 dTestStatus   = E__OK;
	UINT32 dThreshStatus = E__OK;
	UINT8  abSdrRq [SMS_LEN_BUFFER];
	UINT8  abSdrRs [SMS_LEN_BUFFER];
	UINT8  abReadRq [SMS_LEN_BUFFER];
	UINT8  abReadRs [SMS_LEN_BUFFER];
	UINT16 wRecordId, wNextRecordId;
	UINT8  bRsLen;

	/* loop through the SDR Repository */

	for (wRecordId = 0x0000; wRecordId != 0xFFFF; wRecordId = wNextRecordId)
	{

		/* issue IPMI command to get (entire) SDR */

		abSdrRq [0] = (NFC_STORAGE_REQUEST << 2) | (LUN_BMC & 0x03);
		abSdrRq [1] = CMD_GET_SDR_ENTRY;
		abSdrRq [2] = 0x00;
		abSdrRq [3] = 0x00;
		abSdrRq [4] = wRecordId & 0xFF;
		abSdrRq [5] = (wRecordId >> 8) & 0xFF;
		abSdrRq [6] = 0x00;
		abSdrRq [7] = 0xFF;

		dTestStatus = dIpmiSendRequest (&abSdrRq[0], 8, &abSdrRs [0], &bRsLen);
		if (dTestStatus != E__OK)
			break;

		/* extract next record ID from response */

		wNextRecordId = abSdrRs [3] + 256 * abSdrRs[4];

#ifdef DEBUG
//		sprintf (buffer, "RecordId = %04X\n", wRecordId);
//		vConsoleWrite(buffer);
//		sprintf (buffer, "NextRecordId = %04X\n", wNextRecordId);
//		vConsoleWrite(buffer);
//		sprintf (buffer, "RecordLength = %d\n", bRsLen);
//		vConsoleWrite(buffer);
//		sprintf (buffer, "Full Sensor Record SDR = %02X\n", abSdrRs[3+5]);
//		vConsoleWrite(buffer);
#endif

		/* is this a Full Sensor Record SDR? */

		if (abSdrRs[3+5] == 0x01)
		{

			/* attempt to read the sensor for this SDR */
#ifdef DEBUG_VERBOSE
			vDecodeFullSensorRecord(&abSdrRs[5]);
#endif

			abReadRq [0] = (NFC_SENSOR_EVENT << 2) | (LUN_BMC & 0x03);
			abReadRq [1] = CMD_GET_SENS_RD;
			abReadRq [2] = abSdrRs[7+5];	/* sensor # from SDR */
			dTestStatus = dIpmiSendRequest (&abReadRq[0], 3, &abReadRs [0], &bRsLen);

#ifdef DEBUG_VERBOSE
			vDecodeSensorReading(&abReadRs[2], abSdrRs[5+21], abSdrRs[5+24]);
#endif

			if (dTestStatus != E__OK)
			{
				if (dTestStatus == E__COMP_CODE)
				{
					dTestStatus = E__NO_SENSOR;/* sensor not found */
				}
				break;
			}

			if (dThreshStatus == E__OK)
				dThreshStatus = dTestAlarm (abReadRs[3], &abSdrRs[5], wRecordId);
			else
				/* don't save error if already seen an error */
				dTestAlarm (abReadRs[3], &abSdrRs[5], wRecordId);
		}

	} /* end for() */

	/* adjust error code for any threshold error */

	if ((dTestStatus == E__OK) && (dThreshStatus != E__OK))
	{
		dTestStatus = dThreshStatus;
#ifdef DEBUG
		sprintf (buffer, "TestStatus = %X\n", dTestStatus);
		vConsoleWrite(buffer);
#endif
	}

	return (dTestStatus);
}
#endif

/*****************************************************************************
 *
 * dGetWatchdogTimer: Obtains the Watchdog timer value
 *
 * RETURNS: Test status E__OK or error
 */

static UINT32 dGetWatchdogTimer
(
	UINT8 *pbResponse 	/* buffer with message which will be received */
)
{
	UINT32 dTestStatus;
	UINT8 bLength;
	UINT8 abRequest[MAX_LEN_BUFFER];

	/* Send Request for Get Watchdog Timer */
	abRequest [0] = (NFC_APP_REQUEST << 2) | (LUN_BMC & 0x03);	/* NFC + LUN */
	abRequest [1] = CMD_WATCHDOG_GET;
	dTestStatus = dIpmiSendRequest (&abRequest[0], 2, pbResponse, &bLength);

	if (dTestStatus == E__OK && bLength < 11)
		dTestStatus = E__RESPONSE;		/* error if short response */

	return dTestStatus;
}


/*****************************************************************************
 *
 * vClearNmiStatus: Clears IPMI NMI flags in control register
 *
 * RETURNS: None
 */

static void vClearNmiStatus (void)
{
	/* clear NMI flags from IPMI and FP */

	vIoWriteReg(0x211, REG_8, 0xDf);
	vIoWriteReg(0x31d, REG_8,(dIoReadReg(0x31d, REG_8))& 0xFC);
}


/*****************************************************************************
 *
 * myHandler: Watchdog interrupt handler
 *
 * RETURNS: None
 */

static void myHandler (void)
{
	isrCalled = 1;
} /* myHandler () */


/*****************************************************************************
 *
 * vIpmiDeinit: Deinitialize IPMI interface.
 *
 * RETURNS: None
 */

void vIpmiDeinit (void)
{
	/*
	 * Disable PIT timer-1 (ISA memory refresh)
	 * Done using the approved method of seting counter in 16-bit update mode,
	 * but writing only the 8 LS bits.
	 */

	vIoWriteReg (0x43, REG_8, 0x76);	/* counter #1, write LSB/MSB, mode = 2 (rate) */
	vIoWriteReg (0x41, REG_8, 0xFF);	/* divisor = write 8 LS bits only */
}


/*****************************************************************************
 * dIpmiWatchdogPreTest: test function for Watchdog pre reset
 *
 * RETURNS: Test status
 */

UINT32 dIpmiWatchdogPreTest
(
	void
)
{
	UINT32	dTestStatus = E__OK;
	UINT8	abRequest[MAX_LEN_BUFFER];
	UINT8	abResponse[MAX_LEN_BUFFER];
	UINT8	bLength;
	UINT8	bPort61;
	UINT8	bTimerUse;
	UINT8	bTimerActions;
	UINT16	wCountdown;		/* Watchdog Timeout in ms */
	UINT8	bPreTimeout;	/* Pre-timeout in sec */
	UINT8	bAction;		/* Action: 0 - none, 1 - hard reset, 2 - power down */
	UINT8	bBoardNmi;
#ifdef DEBUG
	char	achBuffer[80];
#endif

#ifdef DEBUG
	sprintf (achBuffer, "%s\n", __FUNCTION__);
	vConsoleWrite (achBuffer);
#endif

	/* Default parameters */
	wCountdown = 20;	/* 2 sec timeout */
	bPreTimeout = 1;	/* 1 sec pre-timeout */
	bAction = 0;		/* no action */

	/* initialize IPMI interface */

	 dTestStatus = dIpmiInit ();

	/* Send Request for Set Watchdog Timer */
	if (dTestStatus == E__OK)
	{
		/* select OEM timer and don't log */
		bTimerUse = WD_TIMER_OEM | WD_NOT_LOG;

		/* select action and pre-timeout interrupt */
		bTimerActions = (bAction & WD_ACTION_MASK) | WD_PRE_TM_NMI;

		abRequest [0] = (NFC_APP_REQUEST << 2) | (LUN_BMC & 0x03);	/* NFC + LUN */
		abRequest [1] = CMD_WATCHDOG_SET;							/* command */
		abRequest [2] = bTimerUse;									/* timer use */
		abRequest [3] = bTimerActions;								/* timeout actions */
		abRequest [4] = bPreTimeout;								/* pre-timeout */
		abRequest [5] = WD_EXP_FLAG_OEM;							/* clear OEM flag */
		abRequest [6] = wCountdown & 0x00FF;						/* timeout lsbyte */
		abRequest [7] = wCountdown >> 8;							/* timeout msbyte */

		dTestStatus = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bLength);
	}

	/* Verify response */

	if (dTestStatus == E__OK)
		dTestStatus = dGetWatchdogTimer (&abResponse[0]);

	if (dTestStatus == E__OK)
	{
		if ((abResponse[3] != bTimerUse) ||
			(abResponse[4] != bTimerActions) ||
			(abResponse[5] != bPreTimeout) ||
			((abResponse[7] + 256 * abResponse[8]) != wCountdown))
		{
			dTestStatus = E__VERIFY;
		}
	}


	/* Start Watchdog Timer */

	if (dTestStatus == E__OK)
	{
		/* send Reset Watchdog Timer command */

		abRequest [0] = (NFC_APP_REQUEST << 2) | (LUN_BMC & 0x03);	/* NFC + LUN */
		abRequest [1] = CMD_WATCHDOG_RESET;							/* command */

		dTestStatus = dIpmiSendRequest (&abRequest[0], 2, &abResponse[0], &bLength);
	}

	/* Check pre-timeout NMI interrupt */

	if (dTestStatus == E__OK)
	{
		vDelay (1);		/* to ensure CFS_delay() code is pre-fetched */

		vIoWriteReg(0x70, REG_8, 0x80);	/* disable NMI */
		vClearNmiStatus ();

#ifdef DEBUG
		vConsoleWrite ("clear NMI Status\n");
#endif
		board_service(SERVICE__BRD_CLEAR_IPMI_NMI, NULL, NULL);

		/* reset NMI */

#ifdef DEBUG
		bPort61 = dIoReadReg (0x61, REG_8);
		sprintf (achBuffer, "Line %d : port 0x61 = %02X\n", __LINE__, bPort61);
		vConsoleWrite (achBuffer);
		bPort61 &= 0x07;								// bits 7-4 must always be written as 0
#else
		bPort61 = dIoReadReg (0x61, REG_8) & 0x07;		// bits 7-4 must always be written as 0
#endif

		vIoWriteReg (0x61, REG_8, bPort61 | 0x08);
		vIoWriteReg (0x61, REG_8, bPort61);

#ifdef DEBUG
		sprintf (achBuffer, "Line %d : port 0x61 = %02X\n", __LINE__, dIoReadReg (0x61, REG_8));
		vConsoleWrite (achBuffer);
#endif

		sysInstallUserHandler (NMI_VECTOR, myHandler);

		vIoWriteReg (0x70, REG_8, 0);	/* enable NMI (in RTC) */

		/* wait for NMI interrupt */
		vDelay (wCountdown * 100 - bPreTimeout * 1000 + INT_DELAY + (INT_DELAY * 2));

		if (dTestStatus == E__OK)
			dTestStatus = dGetWatchdogTimer (&abResponse[0]);


		/* reset and disable NMI interrupt */

		vIoWriteReg (0x70, REG_8, 0x80);
		vIoWriteReg (0x61, REG_8, bPort61 | 0x08);
		vIoWriteReg (0x61, REG_8, bPort61);

		vClearNmiStatus ();

		board_service(SERVICE__BRD_GET_IPMI_NMI, NULL, &bBoardNmi);

#ifdef DEBUG
		sprintf (achBuffer, "port 0x212 = %02X\n", dIoReadReg (0x212, REG_8));
		vConsoleWrite (achBuffer);
#endif

		board_service(SERVICE__BRD_CLEAR_IPMI_NMI, NULL, NULL);

		/* check interrupt */
#ifdef DEBUG
		sprintf (achBuffer, "bBoardNmi = %d\nisrCalled = %d\n", bBoardNmi, isrCalled);
		vConsoleWrite (achBuffer);
#endif

		if((isrCalled == 0) /*&& (bBoardNmi == 0)*/)
		{
			vConsoleWrite ("NO Interrupt\n") ;
			dTestStatus = E__NO_INT;		/* No interrupt. */
		}

		isrCalled = 0;
	}

	/* Check watchdog timeout flag */
	vDelay (bPreTimeout * 1000);	/* wait for watchdog timeout */

	if (dTestStatus == E__OK)
	{
		dTestStatus = dGetWatchdogTimer (&abResponse[0]);

		if (dTestStatus == E__OK)
		{
			if ((abResponse[6] & WD_EXP_FLAG_OEM) == 0)
			{
				dTestStatus = E__TIMEOUT;
			}
		}
	}

	vIpmiDeinit ();		/* Deinitialize IPMI interface */
	sysInstallUserHandler (NMI_VECTOR, 0);

	return dTestStatus;
}


/*****************************************************************************
 * dIpmiWatchdogResetTest: test function for Watchdog pre reset
 *
 * RETURNS: Test status
 */

UINT32 dIpmiWatchdogResetTest( void )
{
	UINT32	dTestStatus = E__OK;
	UINT8	abRequest[MAX_LEN_BUFFER];
	UINT8	abResponse[MAX_LEN_BUFFER];
	UINT8	bLength;
	UINT8	bPort61;
	UINT8	bTimerUse;
	UINT8	bTimerActions;
	UINT16	wCountdown;		/* Watchdog Timeout in ms */
	UINT8	bPreTimeout;	/* Pre-timeout in sec */
	UINT8	bAction;		/* Action: 0 - none, 1 - hard reset, 2 - power down */
#ifdef DEBUG
	char	achBuffer[80];
#endif

#ifdef DEBUG
	sprintf (achBuffer, "%s\n", __FUNCTION__);
	vConsoleWrite (achBuffer);
#endif

	/* Default parameters */
	wCountdown = 20;	/* 2 sec timeout */
	bPreTimeout = 1;	/* 1 sec pre-timeout */
	bAction = 1;		/* hard reset */

	/* initialize IPMI interface */

	dTestStatus = dIpmiInit ();

	/* Send Request for Set Watchdog Timer */
	if (dTestStatus == E__OK)
	{
		/* select OEM timer and don't log */
		bTimerUse = WD_TIMER_OEM | WD_NOT_LOG;

		/* select action and pre-timeout interrupt */
		bTimerActions = (bAction & WD_ACTION_MASK) | WD_PRE_TM_NMI;

		abRequest [0] = (NFC_APP_REQUEST << 2) | (LUN_BMC & 0x03);	/* NFC + LUN */
		abRequest [1] = CMD_WATCHDOG_SET;							/* command */
		abRequest [2] = bTimerUse;									/* timer use */
		abRequest [3] = bTimerActions;								/* timeout actions */
		abRequest [4] = bPreTimeout;								/* pre-timeout */
		abRequest [5] = WD_EXP_FLAG_OEM;							/* clear OEM flag */
		abRequest [6] = wCountdown & 0x00FF;						/* timeout lsbyte */
		abRequest [7] = wCountdown >> 8;							/* timeout msbyte */

		dTestStatus = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bLength);
	}

	/* Verify response */

	if (dTestStatus == E__OK)
		dTestStatus = dGetWatchdogTimer (&abResponse[0]);

	if (dTestStatus == E__OK)
	{
		if ((abResponse[3] != bTimerUse) ||
			(abResponse[4] != bTimerActions) ||
			(abResponse[5] != bPreTimeout) ||
			((abResponse[7] + 256 * abResponse[8]) != wCountdown))
		{
			dTestStatus = E__VERIFY;
		}
	}

	/* Start Watchdog Timer */

	if (dTestStatus == E__OK)
	{
		/* send Reset Watchdog Timer command */

		abRequest [0] = (NFC_APP_REQUEST << 2) | (LUN_BMC & 0x03);	/* NFC + LUN */
		abRequest [1] = CMD_WATCHDOG_RESET;							/* command */

		dTestStatus   = dIpmiSendRequest (&abRequest[0], 2, &abResponse[0], &bLength);
	}

	/* Check pre-timeout NMI interrupt */

	if (dTestStatus == E__OK)
	{
		vDelay (1);		/* to ensure CFS_delay() code is pre-fetched */

		vIoWriteReg(0x70, REG_8, 0x80);	/* disable NMI */
		vClearNmiStatus ();

#ifdef DEBUG
		vConsoleWrite ("clear NMI Status\n");
#endif

		board_service(SERVICE__BRD_CLEAR_IPMI_NMI, NULL, NULL);

		/* reset NMI */

#ifdef DEBUG
		bPort61 = dIoReadReg (0x61, REG_8);
		sprintf (achBuffer, "Line %d : port 0x61 = %02X\n", __LINE__, bPort61);
		vConsoleWrite (achBuffer);
		bPort61 &= 0x07;								// bits 7-4 must always be written as 0
#else
		bPort61 = dIoReadReg (0x61, REG_8) & 0x07;		// bits 7-4 must always be written as 0
#endif

		vIoWriteReg (0x61, REG_8, bPort61 | 0x08);
		vIoWriteReg (0x61, REG_8, bPort61);

		/*new_interrupt_table (NMI_VECTOR, NIL, NIL, TRUE);*/
		sysInstallUserHandler (NMI_VECTOR, myHandler);

		vIoWriteReg (0x70, REG_8, 0);	/* enable NMI (in RTC) */

		/* wait for NMI interrupt */
		vDelay (wCountdown * 100 - bPreTimeout * 1000 + INT_DELAY);

		if (dTestStatus == E__OK)
			dTestStatus = dGetWatchdogTimer (&abResponse[0]);

		/* reset and disable NMI interrupt */

#ifdef DEBUG
		sprintf (achBuffer, "Line %d : port 0x61 = %02X\n", __LINE__, dIoReadReg (0x61, REG_8));
		vConsoleWrite (achBuffer);
#endif

		vIoWriteReg (0x70, REG_8, 0x80);
		vIoWriteReg (0x61, REG_8, bPort61 | 0x08);
		vIoWriteReg (0x61, REG_8, bPort61);

		vClearNmiStatus ();

		/* check interrupt */
		if(isrCalled == 0)
			dTestStatus = E__NO_INT;		/* No interrupt. */

		isrCalled = 0;
	}

	/* Check watchdog timeout flag */
	vDelay (bPreTimeout * 1000);	/* wait for watchdog timeout */

	if (dTestStatus == E__OK)
	{
		dTestStatus = dGetWatchdogTimer (&abResponse[0]);

		if (dTestStatus == E__OK)
		{
			if ((abResponse[6] & WD_EXP_FLAG_OEM) == 0)
			{
				dTestStatus = E__TIMEOUT;
			}
		}
	}

	vIpmiDeinit ();		/* Deinitialize IPMI interface */

	return dTestStatus;
}


/*****************************************************************************
 * dIpmiReady: check if SMIC is ready
 *
 * RETURNS: E__OK or E__TIMEOUT or E__NO_IPMI
 */
UINT32 dIpmiReady( UINT32 dTimeout )
{
	UINT32 dStatus;


	dStatus = dIpmiInit();

	if (dStatus == E__OK)
	{
		while (((bReadSmicFlags) & FLAGS_BUSY) == FLAGS_BUSY && --dTimeout > 0)
		{
			vSmicDelay(); /* do nothing ... */
		}

		if (dTimeout == 0)
    	{
    		dStatus = E__TIMEOUT;
    	}
	}

	return dStatus;
}

#if defined(VPX)
/*****************************************************************************
 * bGetVpxIpmbAddrs : gets the AMC board address
 *
 * RETURNS: The board address or 0
 */

UINT8 bGetVpxIpmbAddrs  (void)
{
	UINT8	abRequest [SMS_LEN_BUFFER];
	UINT8	abResponse [SMS_LEN_BUFFER];
	UINT8	bRespLength;
	UINT8	bVpxIpmbAddrs = 0;
	UINT8	i = 0;


	/* Request Get Device ID */
	abRequest [0] = (NFC_OEM_REQUEST << 2) | (LUN_BMC & 0x03); /* NFC + LUN */
	abRequest [1] = 0x60;    /* command - NFC_OEM_GETSLOTID_CMD - Hardcoding Grrrrrrr !!!!! */
	if (dIpmiReady(1000*1000) == E__OK)
	{
		dIpmiSendRequest(&abRequest[0], 2, &abResponse[0], &bRespLength); /* Take care of the error conditions !! */

		for( i=0; i < MAX_VPX_SLOTS; i++ )
		{
			if( vpxBoardId[i] == abResponse[3])
			{
				bVpxIpmbAddrs = i;
				break;
			}
		}

	}
	return (bVpxIpmbAddrs);
}
#endif

/*****************************************************************************
 * bGetAMCIpmbAddrs: gets the AMC board address
 *
 * RETURNS: The board address or 0
 */
#if 0
UINT8 bGetAMCIpmbAddrs (void)
{
    UINT32 dtest_status;
    UINT8   abRequest [SMS_LEN_BUFFER];
    UINT8   abResponse [SMS_LEN_BUFFER];
    UINT8   bRespLength;
    UINT8 bAmcIpmbAddrs=0;



	if (dIpmiReady(1000*1000) == E__OK)
	{
		/* Request Get Device ID */
		abRequest [0] = (NFC_OEM_REQUEST << 2) | (LUN_BMC & 0x03); /* NFC + LUN */
		abRequest [1] = 0x01;    /* command */
		dtest_status = dIpmiSendRequest(&abRequest[0], 2, &abResponse[0], &bRespLength);
		if (dtest_status == E__OK)
		{
			/* index 0, 1 refers to response NFC/LUN and command */
			/* Verify manufacturer ID */

			bAmcIpmbAddrs = abResponse[3];
		}
		// vIpmiDeinit ();
	}
	return (bAmcIpmbAddrs);
}
#endif

/*****************************************************************************
 * IpmiWDResetTest: test function for Watchdog pre reset
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IpmiWDResetTest, "IPMI Watchdog Reset Test")
{
	UINT32 dtest_status;

	dtest_status = dIpmiWatchdogResetTest();

	return (dtest_status);
}


/*****************************************************************************
 * IpmiWDPreTest: test function for Watchdog pre reset
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IpmiWDPreTest, "IPMI Watchdog Pre Test")
{
	UINT32 dtest_status;

	dtest_status = dIpmiWatchdogPreTest();

	return (dtest_status);
}

#if 0
//
// This function is not implemented in any of the current IPMI micros
// There is no requirement for the IPMI controller to generate interrupts
// for the conditions that this test was trying to validate!
//
// This code can be removed at a later date if this feature remains unimplemented
//

/*****************************************************************************
 * IpmiAlarmIntTest: test function for interrupt generating capability
 *						when voltage and temperature critical thresholds
 *						are crossed
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IpmiAlarmIntTest, "IPMI Alarm Interrupt Test")
{
	UINT32 dtest_status;


	/* Initialize IPMI interface */

	dtest_status = dIpmiInit ();
	if (dtest_status == E__OK)
	{
		dtest_status = dIpmiSensorAlarmTest();
	}

	return (dtest_status);
}
#endif

/*****************************************************************************
 * IpmiThresholdTest: test function to verify if any sensor is crossing
 *						the threshold limits
 *
 * RETURNS: Test status
 */
extern int cpuSetNe (int iNewState);

TEST_INTERFACE (IpmiThresholdTest, "IPMI Sensor Threshold Test")
{
	UINT32 dtest_status;
	UINT32	dExceptnMap;
	UINT32	rt = E__OK;
	int		oldNe;
	UINT32 op1 = 0;

	/* Initialize IPMI interface */
	dtest_status = dIpmiInit ();
	if (dtest_status == E__OK)
	{
		fpuFlag = 0;
		board_service(SERVICE__BRD_GET_FPU_EXCEPTION_MAP, NULL, &dExceptnMap);

		if ( (dExceptnMap & (FPU_EXCEPTION_MF | FPU_EXCEPTION_FERR)) == 0 )
			rt = E__NOT_SUPPORTED;
		if (dExceptnMap & FPU_EXCEPTION_MF)
		{
			fpuFlag = 0;

			oldNe = cpuSetNe (1);		/* select internal (MF) reporting */

			installFPUExcpHandler(-1, FPU_MF);

			if (rt != E__OK)
				rt |= E__FPU_MF;
		}
		op1 = 0x27F;
		fpuSetup(2, &op1);				// reset FPU - op1 not used
		fpuSetup(1, &op1);
		dtest_status = dIpmiSensorThresholdTest();
		if (dExceptnMap & FPU_EXCEPTION_MF)
		{
			UninstallFPUExcpHandler(FPU_FERR, 0);
			cpuSetNe (oldNe);
		}
	}

	return (dtest_status);
}


/*****************************************************************************
 * IpmiSDRFormatTest: test function to verify the SDR contents
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IpmiSDRFormatTest, "IPMI SDR Format Test")
{
	UINT32 dtest_status;

	/* Initialize IPMI interface */

	dtest_status = dIpmiInit ();
	if (dtest_status == E__OK)
	{
		dtest_status = dIpmiSDRformatTest();
	}

	return (dtest_status);
}


/*****************************************************************************
 * IpmiSelfTest: test function to check the status of the IPMI internal
 *					Built in Test result
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IpmiSelfTest, "IPMI Self Test Status")
{
	UINT32 dtest_status;

	/* Initialize IPMI interface */

	dtest_status = dIpmiInit ();
	if (dtest_status == E__OK)
	{
		dtest_status = dIpmiSelfTestStatus();
	}

	return (dtest_status);
}

/*****************************************************************************
 * IpmiIntTest: test function to check the interrupt generation
 *					capabilities of the IPMI
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IpmiIntTest, "IPMI Interrupt Test")
{
	UINT32 dtest_status;

	/* Initialize IPMI interface */

	dtest_status = dIpmiInit ();
	if (dtest_status == E__OK)
	{
		dtest_status = dIpmiInterruptTest();
	}

	return (dtest_status);
}


/*****************************************************************************
 * IpmiSmicInterfaceTest: test function to check correct operation
 *							of the SMIC interface
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IpmiSmicInterfaceTest, "IPMI SMIC Interface Test")
{
	UINT32 dtest_status;

	/* Initialize IPMI interface */

	dtest_status = dIpmiInit ();
	if (dtest_status == E__OK)
	{
		dtest_status = dReadDeviceID();
	}

	return (dtest_status);
}


static UINT32 installFPUExcpHandler(int iPin, int iVector)
{
	UINT32	rt = E__OK;
	UINT32	op;

	/* Set FPU control word to generate exceptions */

	op = 0x0340;			/* enable all FPU exceptions */
	fpuSetup (1, &op);

	vIoWriteReg (0xf0, REG_8, 0 );

	if (setupInt (iPin, iVector) == E__OK)
	{
		rt = E__OK;
	}
	else
		rt = E__FPU_INT_ERROR;

	return rt;	/* Reset FPU */
}

/*****************************************************************************
 * removeInt: install interrupt handler and enable interrupt
 *
 * RETURNS: E__OK or E__FPU_INT_ERROR
 */

static int setupInt
(
	int		iPin,		/* pin number, -1 if not a pin */
	int		iVector		/* vector number if not a pin */
)
{
	if (iPin != -1)
	{
		iVector = sysPinToVector (iPin, SYS_IOAPIC0);

		if (iVector == -1)
			return (E__FPU_INT_ERROR);
	}

	sysInstallUserHandler (iVector, IpmifpuHandler);

	if (iPin != -1)
	{
		if (sysUnmaskPin (iPin, SYS_IOAPIC0) == -1)
			return (E__FPU_INT_ERROR);
	}

	return E__OK;
} /* setupInt () */

/*****************************************************************************
 * fpuHandler: exception handler for FPU test
 *
 * RETURNS: E__OK or E__FPU_INT_ERROR
 */

static void IpmifpuHandler (void)
{
	fpuFlag = 1;
	fpuClearFlags(0);
} /* fpuHandler () */

static UINT32 UninstallFPUExcpHandler(int iPin, int iVector)
{
	UINT32	rt;
	UINT32	op1;
	fpuSetup (2, &op1);
	vIoWriteReg (0xf0, REG_8, 0 );

	/* Uninstall the handler */
	if (removeInt (iPin, iVector) != E__OK)
		rt = E__FPU_INT_ERROR;
	else
		rt = E__OK;

	return rt;
}

/*****************************************************************************
 * removeInt: mask interrupt and uninstall handler
 *
 * RETURNS: E__OK or E__FPU_INT_ERROR
 */

static int removeInt
(
	int		iPin,		/* pin number, -1 if not a pin */
	int		iVector		/* vector number if not a pin */
)
{
	if (iPin != -1)
	{
		iVector = sysPinToVector (iPin, SYS_IOAPIC0);

		if (iVector == -1)
			return (E__FPU_INT_ERROR);

		if (sysMaskPin (iPin, SYS_IOAPIC0) == -1)
			return (E__FPU_INT_ERROR);

	}

	sysInstallUserHandler (iVector, 0);		/* uninstall handler */

	return E__OK;
} /* removeInt () */


#if 0
static UINT32 tempsenseDTS(int *rd)
{
	UINT32 msw, lsw, prochot;
  int	readout;
#ifdef DEBUG
	char buffer[128];
#endif


	//read thermal status
	vReadMsr (0x19c, &msw, &lsw);
	readout = (lsw >> 16) & 0x000000ff;
#ifdef DEBUG
	sprintf(buffer, "thermal status: MSW:0x%x, LSW:0x%x, Raw Temp:0x%x\n",
		    msw, lsw, readout);
	vConsoleWrite(buffer);
#endif

	//read temperature Target
	vReadMsr (0x1a2, &msw, &lsw);
	prochot = (lsw >> 16) & 0x000000ff;
#ifdef DEBUG
	sprintf(buffer, "temperature Target: MSW:0x%x, LSW:0x%x, Prochot:0x%x\n",
					msw, lsw, prochot);
	vConsoleWrite(buffer);
#endif

	readout = prochot-readout;

	//sprintf(buffer, "\nCPU Temperature:%dC\n", readout);
	//vConsoleWrite(buffer);

	*rd = readout;

	if( readout >= 103)
	{
		//vConsoleWrite("POS ERROR\n");
		//sprintf(buffer, "0x%x CPU +Ve Threshold [98C] Exceeded! [%dC] ", CPU_POS_THR_ERROR, readout);
		//postErrorMsg(buffer);
		return CPU_POS_THR_ERROR;
	}
	else if( readout < -25)
	{
		//vConsoleWrite("NEG ERROR\n");
		//sprintf(buffer, "0x%x CPU -Ve Threshold [-20C] Exceeded! [%dC] ", CPU_NEG_THR_ERROR, readout);
		//postErrorMsg(buffer);
		return CPU_NEG_THR_ERROR;
	}
	else
	{	//vConsoleWrite("OK\n");
		return E__OK;
	}

}
#endif

/*****************************************************************************
 * dAMCTests: Test the hotswap and watchdog LEDs.
 *            Report geographic address and ejection handle position.
 *
 * RETURNS: E__OK or IPMI interface error
 */
UINT32 dAMCTests
(
	UINT8	geographicAddrCmd
)
{
	UINT32	dtest_status;
	UINT16	wRecordId;
	UINT16	wNextRecordId;
	UINT8	abRequest [SMS_LEN_BUFFER];
	UINT8	abResponse [SMS_LEN_BUFFER];
	UINT8	bRespLength;
	char	achBuffer[80];
	char	aGA0[10], aGA1[10], aGA2[10];

	/* Issue IPMI command to get Geographic Address */
	abRequest [0] = (NFC_OEM_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest [1] = geographicAddrCmd;			/* micro controller specific command */

	dtest_status = dIpmiSendRequest (&abRequest[0], 2, &abResponse[0], &bRespLength);
	if (dtest_status != E__OK)
		return dtest_status;

	if ((abResponse[3] & 0x09) == 0x09)
		sprintf( aGA0,"High");
	else if ((abResponse[3] & 0x09) == 0x00)
		sprintf ( aGA0,"Low");
	else
		sprintf ( aGA0,"Floating");

	if ((abResponse[3] & 0x12) == 0x12)
		sprintf (aGA1,"High");
	else if ((abResponse[3] & 0x12) == 0x00)
		sprintf ( aGA1,"Low");
	else
		sprintf ( aGA1,"Floating");

	if ((abResponse[3] & 0x24) == 0x24)
		sprintf (aGA2,"High");
	else if ((abResponse[3] & 0x24) == 0x00)
		sprintf (aGA2,"Low");
	else
		sprintf (aGA2,"Floating");

	sprintf (achBuffer, "Geographic Address: GA0 = %s, GA1 = %s, GA2 = %s\n", aGA0, aGA1, aGA2);
	vConsoleWrite (achBuffer);

	/* Flash the Hotswap LED */

	vConsoleWrite( "Testing Hotswap (HS) LED ...\n");

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x00; /* LED ID = Hot Swap */
	abRequest[5] = 0xFF; /* LED State = On */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	vDelay (500);

	abRequest[0] = ( NFC_PICMG_REQUEST << 2 ) | ( LUN_BMC & 0x03 );
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x00; /* LED ID = Hot Swap */
	abRequest[5] = 0x00; /* LED State = Off */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	vDelay (300);

	abRequest[0] = ( NFC_PICMG_REQUEST << 2 ) | ( LUN_BMC & 0x03 );
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x00; /* LED ID = Hot Swap */
	abRequest[5] = 0xFF; /* LED State = On */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	vDelay (500);

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x00; /* LED ID = Hot Swap */
	abRequest[5] = 0x00; /* LED State = Off */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	vDelay (300);

	abRequest[0] = ( NFC_PICMG_REQUEST << 2 ) | ( LUN_BMC & 0x03 );
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x00; /* LED ID = Hot Swap */
	abRequest[5] = 0xFF; /* LED State = On */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	vDelay (500);

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x00; /* LED ID = Hot Swap */
	abRequest[5] = 0x00; /* LED State = Off */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	/* Flash the Watchdog LED */

	vConsoleWrite( "Testing Watchdog (WD) LED ...\n");

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x01; /* LED ID = Watchdog */
	abRequest[5] = 0xFF; /* LED State = On */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	vDelay (500);

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x01; /* LED ID = Watchdog */
	abRequest[5] = 0x00; /* LED State = Off */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	vDelay (300);

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x01; /* LED ID = Watchdog */
	abRequest[5] = 0xFF; /* LED State = On */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	vDelay (500);

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x01; /* LED ID = Watchdog */
	abRequest[5] = 0x00; /* LED State = Off */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK )
		return dtest_status;

	vDelay (300);

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x01; /* LED ID = Watchdog */
	abRequest[5] = 0xFF; /* LED State = On */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK )
		return dtest_status;

	vDelay (500);

	abRequest[0] = (NFC_PICMG_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_SET_FRU_LED_STATE;
	abRequest[2] = 0x00; /* Not Used */
	abRequest[3] = 0x00; /* Not Used */
	abRequest[4] = 0x01; /* LED ID = Watchdog */
	abRequest[5] = 0x00; /* LED State = Off */
	abRequest[6] = 0x00; /* Test Duration = N/A */
	abRequest[7] = 0x0F; /* LED Colour = Use Default Colour */

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength)) != E__OK )
		return dtest_status;

	vDelay (300);

	/* Issue IPMI command to get Hotswap Handle state */

	vConsoleWrite ("Reading Hotswap Handle State ...\n");

#if 0
	// do not use an OEM command to get the hotswap handle position
	// use the normal 'get sensor reading command' instead

	abRequest[0] = (NFC_OEM_REQUEST << 2) | (LUN_BMC & 0x03);
	abRequest[1] = CMD_GET_HOTSWAP_HANDLE_POSN;

	if ((dtest_status = dIpmiSendRequest (&abRequest[0], 2, &abResponse[0], &bRespLength)) != E__OK)
		return dtest_status;

	sprintf (achBuffer, "Hotswap Handle: %s", ( abResponse[3] == 0 ) ? "Open (Pulled Out)\n" : "Closed (Pushed In)\n");
	vConsoleWrite (achBuffer);
#endif

	for (wRecordId = 0x0000; wRecordId != 0xFFFF; wRecordId = wNextRecordId)
	{
		/* issue IPMI command to get SDR */

		abRequest [0] = (NFC_STORAGE_REQUEST << 2) |(LUN_BMC & 0x03);
		abRequest [1] = CMD_GET_SDR_ENTRY;
		abRequest [2] = 0x00;
		abRequest [3] = 0x00;
		abRequest [4] = wRecordId & 0xFF;
		abRequest [5] = (wRecordId >> 8) & 0xFF;
		abRequest [6] = 0x00;
		abRequest [7] = 0xFF;

		dtest_status = dIpmiSendRequest (&abRequest[0], 8, &abResponse[0], &bRespLength);
		if (dtest_status != E__OK)
		{
			break;
		}

		/* extract next record ID from response */

		wNextRecordId = abResponse [3] + (256 * abResponse[4]);

		/* is this a Compact Sensor Record SDR? */

		if (abResponse[3+5] == 0x02)
		{
#ifdef DEBUG_VERBOSE
			vDecodeCompactSensorRecord(&abResponse[5]);
#endif

			// find the ejector handle sensor
			if (abResponse[12+5] == AMC_MODULE_HOT_SWAP_ID)
			{
				// have found the sensor we were looking for
		
				/* attempt to read the sensor for this SDR */
		
				abRequest [0] = (NFC_SENSOR_EVENT << 2) | (LUN_BMC & 0x03);
				abRequest [1] = CMD_GET_SENS_RD;
				abRequest [2] = abResponse[7+5];	/* sensor # from SDR */

				dtest_status = dIpmiSendRequest (&abRequest[0], 3, &abResponse[0], &bRespLength);

#ifdef DEBUG_VERBOSE
				vDecodeSensorReading(&abResponse[2], abResponse[5+21], abResponse[5+24]);
#endif

				sprintf (achBuffer, "Hotswap Handle: %s", (abResponse[2+1] == 1) ? "Open (Pulled Out)\n" : "Closed (Pushed In)\n");
				vConsoleWrite (achBuffer);
				break;
			}
		}
	}

	vConsoleWrite ("Test Complete\n");

	/* De-Initialise IPMI Interface */

	vIpmiDeinit ();		/* Deinitialize IPMI interface */

	return dtest_status;
}

/*****************************************************************************
 * IpmiAMCTest: test function to check AMC specific features
 *				   of the SMIC interface for the 8051 micro
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (IpmiAMCTest, "IPMI AMC Tests")
{
	UINT32 dtest_status;

	/* Initialize IPMI interface */

	dtest_status = dIpmiInit ();
	if (dtest_status == E__OK)
	{
		dtest_status = dAMCTests(CMD_GET_GEOGRAPHIC_ADDR);
	}

	return (dtest_status);
}

/*****************************************************************************
 * IpmiAMCTest_M3: test function to check AMC specific features
 *				   of the SMIC interface for the Cortex M3 micro
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (IpmiAMCTest_M3, "IPMI AMC Tests")
{
	UINT32 dtest_status;

	/* Initialize IPMI interface */

	dtest_status = dIpmiInit ();
	if (dtest_status == E__OK)
	{
		dtest_status = dAMCTests(CMD_GET_GEOGRAPHIC_ADDR_M3);
	}

	return (dtest_status);
}


/*****************************************************************************
 * bGetAMCIpmbAddrs: gets the AMC board address
 *
 * RETURNS: The board address
 */
UINT8 bGetAMCIpmbAddrs (void)
{
	/*UINT32 dtest_status;*/
	UINT8   abRequest [SMS_LEN_BUFFER];
	UINT8   abResponse [SMS_LEN_BUFFER];
	UINT8   bRespLength;
	UINT8   bAmcIpmbAddrs= 0;

	/* Request Get Device ID */
	abRequest [0] = (NFC_OEM_REQUEST << 2) | (LUN_BMC & 0x03); /* NFC + LUN */
	abRequest [1] = 0x01;    /* command */

	if (dIpmiReady(1000*1000) == E__OK)
	{
		/*dtest_status =*/ dIpmiSendRequest(&abRequest[0], 2, &abResponse[0], &bRespLength);
		//if (dtest_status == E__OK)
		{
			/* index 0, 1 refers to response NFC/LUN and command */
			/* Verify manufacturer ID */
			bAmcIpmbAddrs = abResponse[3];
		}
	}

	return (bAmcIpmbAddrs);
}



UINT8 bGetSPSStatus (UINT8 *bSrioSpeed)
{
	/*UINT32 dtest_status = 0x100;*/
	UINT8   abRequest [SMS_LEN_BUFFER];
	UINT8   abResponse [SMS_LEN_BUFFER];
	UINT8   bRespLength=0;
	UINT8   bSpsStatus=0xF4;
#if 0
	/* Request Get Device ID */
	abRequest [0] = (NFC_OEM_REQUEST << 2) | (LUN_BMC & 0x03); /* NFC + LUN */
	abRequest [1] = 0xE0;    /* command */

	dtest_status = dIpmiSendRequest(&abRequest[0], 2, &abResponse[0], &bRespLength);
	while (dtest_status != E__OK)
	{
		vDelay(10);
		dtest_status = dIpmiSendRequest(&abRequest[0], 2, &abResponse[0], &bRespLength);
		/* index 0, 1 refers to response NFC/LUN and command */
		/* Verify manufacturer ID */

		bSpsStatus = abResponse[2];
		*bSrioSpeed = abResponse[3];
	}
#endif

	/* Request Get Device ID */
	abRequest [0] = (NFC_OEM_REQUEST << 2) | (LUN_BMC & 0x03); /* NFC + LUN */
	abRequest [1] = 0xE0;    /* command */
	if (dIpmiReady(1000*1000) == E__OK)
	{
		/*dtest_status =*/ dIpmiSendRequest(&abRequest[0], 2, &abResponse[0], &bRespLength);
		/* index 0, 1 refers to response NFC/LUN and command */
		/* Verify manufacturer ID */

		bSpsStatus = abResponse[2];
		*bSrioSpeed = abResponse[3];
	}

	return (bSpsStatus);
}

