
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

//cleanup
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/mem.h>
#include <bit/hal.h>
 
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>
#include <bit/board_service.h>
#include <private/debug.h>
#include <private/port_io.h>
#include <devices/scc16550.h>
#include <comm/serial.h>
#include <cute/packet.h>



static UINT16 wPort;
static void  vSetBaud(UINT16 wBaudDiv);
static void  vSetDTR (UINT8 bVal);
static void  vSetRTS (UINT8 bVal);
static UINT8 bGetCTS (void);
static UINT8 bGetDCD (void);

UINT32 InitSerialComm( void )
{
	UINT16 temp;
	char  buff[32];

	if(board_service(SERVICE__BRD_GET_SOAK_PORT, NULL, &temp) == E__OK)
	{
	wPort = temp;

		sprintf(buff, "InitSerialComm() port %x\n", wPort);
		sysDebugWriteString (buff);

	vSetBaud (BAUD_9600);
	vSetDTR (0);
	vSetRTS (0);

	return E__OK;
	}
	else
	{
		sysDebugWriteString ("InitSerialComm() FAIL\n");
		return E__FAIL;
	}

} /* InitialiseUniverse () */


UINT32 wSerialMessageGet( UINT8 *pbMessage,	UINT16 wMax, UINT16 *wLength )
{
	UINT32	wStatus = E__OK;
	UINT16	wTimeout=TIMEOUT_PERIOD;
	UINT8	bTmp;
	UINT8	bDcd, bCount=0;

	/* Wait for the OK */
	while (!(bDcd = bGetDCD()) && (bCount < 5))
	{
		if (bDcd)
			bCount++;
		else
			bCount = 0;

		vDelay (1);

		if (wTimeout-- == 0)
		{
			return(E__TIMEOUT);
		}
	}

	vSetDTR (TRUE);

	wTimeout = TIMEOUT_PERIOD;

	while (bGetDCD() == TRUE)
	{
		/* wait for character available */
		if ( (sysInPort8(wPort + LSR) & 1) !=0)
		{
			/* read and store the character */
			bTmp = sysInPort8 (wPort);
			pbMessage [(*wLength)++] = bTmp ;

			if ((*wLength)>wMax)
			{
				vSetDTR(FALSE);
				return(E__READ_OVERFLOW);
			}
			wTimeout = TIMEOUT_PERIOD;
		}
		else
		{
			if (wTimeout-- == 0)
			{
				vSetDTR (FALSE);
				return E__TIMEOUT;
			}
			vDelay (1);
		}
	}

	vSetDTR (FALSE);

	/* get any remaining characters from the FIFO */
	while ((sysInPort8(wPort + LSR) & 1) !=0)
	{
		/* read and store the character */
		bTmp = sysInPort8 (wPort);
		pbMessage [(*wLength)++] = bTmp ;
		if ((*wLength)>wMax)
		{
			vSetDTR(FALSE);
			return(E__READ_OVERFLOW);
		}
	}

	return wStatus;

} /* InitialiseUniverse () */



UINT32 wSerialMessagePut( const UINT8 *pbMessage, UINT16 wLength, UINT32 dTimeout )
{
	UINT32	StartH;
	UINT16	Timeout=TIMEOUT_PERIOD;

	/* We want to Xmt */
	vSetRTS(TRUE);

	/* Wait for the OK */
	StartH = 0;
	while (!bGetCTS())
	{
		vDelay (1);
		if (StartH++ > dTimeout)
		{
			vSetRTS(FALSE);
			return(E__TIMEOUT);
		}
	}

	/* Xmt Data */
	while (wLength--)
	{
		sysOutPort8 (wPort, *pbMessage++);
		while(! (sysInPort8(wPort + LSR) &0x40) );
	}

	/* Wait for the buffer to drain */
	while ( (sysInPort8(wPort + LSR) &0x40) == 0);

	/* We've finished */
	vSetRTS(FALSE);

	/* wait for receiver to finish message */
	while (bGetCTS())
	{
		if (--Timeout == 0)
			break;
		vDelay (1);
	}

	return(E__OK);
}



UINT32 Serial_detect_message ()
{
	if (bGetDCD() == TRUE)
			return E__OK;
	else
		return E_NOT_PENDING;
}



UINT8 serialSoakMasterReady (void)
{
	return bGetCTS();
}


void serialSetPacketWaiting (void)
{
	vSetRTS(TRUE);
}



/************************************************************************/
/*  vSetBaud                                                           */
/*                                                                      */
/************************************************************************/
static void vSetBaud (UINT16 wBaudDiv)
{
	UINT8	bTmp;

	/* Program baud rate divisor */
	bTmp = sysInPort8(wPort + LCR);
	sysOutPort8 (wPort + LCR, bTmp | LCR_DLAB);	/* DLAB = 1 to access divisor*/
	sysOutPort8 (wPort + DLL, (UINT8)(wBaudDiv & 0x00FF));
	sysOutPort8 (wPort + DLH, (UINT8)(wBaudDiv >> 8));
	sysOutPort8 (wPort + LCR, bTmp); // Clear DLAB bit

	/* Select 8N1 format */
	sysOutPort8 (wPort + LCR, LCR_DATA8 | LCR_NONE | LCR_STOP1);

	/* Disable interrupts */
	sysOutPort8 (wPort + IER, 0);

	/* Enable and reset FIFOs */
	sysOutPort8 (wPort + FCR, FCR_FEN | FCR_CRF | FCR_CTF);


}



/************************************************************************/
/*  vSetDTR                                                           	*/
/*                                                                      */
/************************************************************************/
static void vSetDTR (UINT8 bVal)
{
	UINT8	bTmp;
	bTmp = sysInPort8 (wPort + MCR);
	if (bVal)
		bTmp |= 0x01;
	else
		bTmp &= 0xFE;
	sysOutPort8 (wPort + MCR, bTmp);
}



/************************************************************************/
/*  vSetRTS                                                           	*/
/*                                                                      */
/************************************************************************/
static void vSetRTS (UINT8 bVal)
{
	UINT8	bTmp;
	bTmp = sysInPort8 (wPort + MCR);
	if (bVal)
		bTmp |= 0x02;
	else
		bTmp &= 0xFD;
	sysOutPort8 (wPort + MCR, bTmp);
}



/************************************************************************/
/*  bGetDCD                                                           	*/
/*                                                                      */
/************************************************************************/
static UINT8 bGetDCD (void)
{
	UINT8	bTmp;

	bTmp = sysInPort8 (wPort + MSR) & 0x80;
	if (bTmp > 0)
		return TRUE;
	else
		return FALSE;
}

/************************************************************************/
/*  bGetCTS                                                           	*/
/*                                                                      */
/************************************************************************/
static UINT8 bGetCTS (void)
{
	UINT8	bTmp;

	bTmp = sysInPort8 (wPort + MSR) & 0x10;
	if (bTmp > 0)
		return TRUE;
	else
		return FALSE;
}



