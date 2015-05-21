
/************************************************************************
 *                                                                      *
 *      Copyright 2010 Concurrent Technologies, all rights reserved.    *
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
 * mouse.c
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/mouse.c,v 1.3 2013-11-25 11:55:57 mgostling Exp $
 * $Log: mouse.c,v $
 * Revision 1.3  2013-11-25 11:55:57  mgostling
 * Added missing CVS headers
 *
 *
 *  Created on: 4 Jun 2010
 *      Author: engineer
 */

#include <stdtypes.h>
#include <errors.h>
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <devices/mouse.h>
#include <bit/hal.h>
 
#include <bit/io.h>
#include <bit/delay.h>
#include <bit/console.h>


//#define DEBUG

UINT32 reset_ps2mouse (void);
UINT32 write_ps2mouse(UINT8 mouse_command);
UINT32 read_ps2mouse(UINT8 *mouse_data );
UINT32 wait_to_send2 (void);
UINT32 flush_outbuffer (void);
UINT32 identify_ps2mouse (void);
UINT32 echo_ps2mouse (void);
UINT32 packet_ps2mouse( PS2MOUSE_PACKET *sDataPacket);
UINT32 move_ps2mouse(void);
UINT32 action_ps2mouse(void);



/*****************************************************************************
 * ps2MouseTest: PS2 Mouse Connectivity Test
 *
 * RETURNS: None
 */
TEST_INTERFACE (ps2MouseTest, "PS2 Mouse Connectivity Test")
{
	UINT32 rt = E__FAIL;
	int i;
#ifdef DEBUG
	char buffer[80];
#endif

	for (i = 0; (i < 3) && (rt != E__OK); i++)		/* reset mouse 3 times */
	{
		#ifdef DEBUG
			sprintf (buffer, "Reset %d\n", i + 1);
			vConsoleWrite(buffer);
		#endif

		rt = reset_ps2mouse ();
	}

	if (rt != E__OK)
		vConsoleWrite ("Did not find mouse");

	vDelay(10);

	/* Test Identify Device from PS/2 mouse */
	if (rt == E__OK)
		rt = identify_ps2mouse ();

	/* Test echo from PS/2 mouse */
	if (rt == E__OK)
		rt = echo_ps2mouse ();


	return rt;
}



/*****************************************************************************
 * ps2MouseActionTest: PS2 Mouse Action Test
 *
 * RETURNS: None
 */
TEST_INTERFACE (ps2MouseActionTest, "PS2 Mouse Action Test")
{
	UINT32 rt = E__FAIL;
	int i;

	#ifdef DEBUG
		char buffer[80];
	#endif

	for (i = 0; (i < 3) && (rt != E__OK); i++)		/* reset mouse 3 times */
	{
		#ifdef DEBUG
			sprintf (buffer, "Reset %d\n", i + 1);
			vConsoleWrite(buffer);
		#endif

		rt = reset_ps2mouse ();
	}

	if (rt != E__OK)
		vConsoleWrite ("Did not find mouse");

	vDelay(10);

	identify_ps2mouse ();

	if(rt == E__OK)
		rt = action_ps2mouse();

	return rt;
}



/*****************************************************************************
 * ps2MouseActionReport: PS2 Mouse Action Reporting
 *
 * RETURNS: None
 */
TEST_INTERFACE (ps2MouseActionReport, "PS2 Mouse Action Reporting")
{
	UINT32 rt = E__FAIL;
	int i;

	#ifdef DEBUG
		char buffer[80];
	#endif

	for (i = 0; (i < 3) && (rt != E__OK); i++)		/* reset mouse 3 times */
	{
		#ifdef DEBUG
			sprintf (buffer, "Reset %d\n", i + 1);
			vConsoleWrite(buffer);
		#endif

		rt = reset_ps2mouse ();
	}

	if (rt != E__OK)
		vConsoleWrite ("Did not find mouse");

	vDelay(10);

	identify_ps2mouse ();

	if(rt == E__OK)
		rt = move_ps2mouse();

	return rt;
}


/******************************************************************************
 *
 * echo_ps2mouse - test echo of PS/2 mouse
 *
 * Put PS/2 mouse into wrap (echo) mode and test answers.
 * After test send no wrap command.
 *
 * RETURNS: E_OK if it is OK, or error code
 *
 * Error codes:
 *	ERR_FLUSH
 *	ERR_NO_READ_CMD
 *	ERR_NO_READ_DATA
 *	ERR_NO_PS2_ECHO
 *	ERR_FALSE_PS2_ECHO
 */
UINT32 echo_ps2mouse (void)
{
	UINT32 wTestStatus;
	UINT16 wWaitResult;
	UINT8 bOutBuffer;

	wTestStatus = E__OK;

	if (wTestStatus == E__OK)
		wTestStatus = flush_outbuffer();

	/* Put PS/2 mouse into wrap mode */
	if (wTestStatus == E__OK)
		wTestStatus = write_ps2mouse (PS2_SET_WRAP);

	/* Test echo */
	if (wTestStatus == E__OK)
	{
		vIoWriteReg(CMD_PORT, REG_8, WRITE_AUX);
		wWaitResult = wait_to_send2();

		if (wWaitResult != E__OK)
		{
			/* Keyboard controller did not read command byte. */
			vConsoleWrite("Aux write command not read.");
			wTestStatus = ERR_NO_READ_CMD;
		}
	}

	/* Write some byte to PS/2 mouse */
	if (wTestStatus == E__OK)
	{
		vIoWriteReg(KBIN_BUFF, REG_8, PS2_SET_SCALING);
		wWaitResult = wait_to_send2();

		if (wWaitResult != E__OK)
		{
			/* PS/2 mouse did not read command byte. */
			vConsoleWrite("PS/2 not read command.");
			wTestStatus = ERR_NO_READ_DATA;
		}
	}

	/* Check echo from PS/2 mouse */

	if (wTestStatus == E__OK)
	{
		wTestStatus = read_ps2mouse (&bOutBuffer);	/* read answer from PS/2 mouse */
		if (wTestStatus == E__OK)
		{
			if (bOutBuffer != PS2_SET_SCALING)
			{
				wTestStatus = ERR_FALSE_PS2_ECHO;
			}
		}
		else
			wTestStatus = ERR_NO_PS2_ECHO;		/* nothimg received from PS/2 mouse */
	}

	/* Back mouse from wrap to previous mode */

	write_ps2mouse (PS2_RESET_WRAP);

	return wTestStatus;
}



/******************************************************************************
 *
 * identify_ps2mouse - identify PS/2 mouse
 *
 * Send to PS/2 mouse command to identify. If mouse identify is not 00h then write
 *
 * RETURNS: E_OK if it is OK, or error code
 *
 * Error codes:
 *	ERR_FLUSH
 *	ERR_NO_READ_CMD
 *	ERR_NO_READ_DATA
 *	ERR_FALSE_ID
 *	ERR_NO_ID
 *
 */
UINT32 identify_ps2mouse (void)
{
	UINT32 wTestStatus;
	UINT8 bOutBuffer;
	char buffer[80];

	wTestStatus = E__OK;

	/* Test Identify Device from PS/2 mouse */

	if (wTestStatus == E__OK)
		wTestStatus = flush_outbuffer ();

	if (wTestStatus == E__OK)
		wTestStatus = write_ps2mouse (PS2_ID);	/* send command for identify mouse */

	if (wTestStatus == E__OK)
	{
		wTestStatus = read_ps2mouse (&bOutBuffer);	/* read answer from PS/2 mouse */
		if (wTestStatus == E__OK)
		{
			if (bOutBuffer != PS2RET_MOUSEID)
			{
				sprintf(buffer, "Wrong mouse identify. ID %x", bOutBuffer);
				vConsoleWrite(buffer);
				wTestStatus = ERR_FALSE_ID;
			}
		}
		else
		{
			wTestStatus = ERR_NO_ID;
		}
	}

	return wTestStatus;
}



/******************************************************************************
 *
 * flush_outbuffer - flush output buffer from keyboard controller
 *
 * Read out buffer while there are something
 *
 * RETURNS: E_OK if it is OK, or error code
 *
 * Error codes:
 * 	ERR_FLUSH
 *
 */
UINT32 flush_outbuffer (void)
{
	UINT32 wTestStatus;
	UINT16 timeout;
	UINT8  bStatusByte;
	//UINT8  tmpbyte;

	wTestStatus = E__OK;

	timeout = 0;
	bStatusByte = dIoReadReg(STATUS_PORT, REG_8);

	while ((timeout <= 255) &&       /* If haven't read 255 characters and */
			(bStatusByte &0x01))      /* output buffer is full. */
	{
		dIoReadReg(KBOUT_BUFF, REG_8);    /* Read char out of buffer. */
		bStatusByte = dIoReadReg(STATUS_PORT, REG_8);       /* Read status byte now. */
		timeout++;
	}

	/* Check that it didn't time out. */

	if (timeout >= 255)
	{
		/* Keyboard was never ready for initialisation. Did not flush */
#ifdef DEBUG
		vConsoleWrite ("Mouse controller not ready. Timed out while flushing.\n");
#endif
		wTestStatus = ERR_FLUSH;
	}

	return wTestStatus;
}



/******************************************************************************
 *
 * wait_to_send - wait to send byte to keyboard controler
 *
 * This function waits upto 25 milliseconds for the keyboard to read
 * and acknowledge a command or data sent to it. The function responds
 * with E_OK if the data was read of the status register value if not.
 *
 * RETURNS: E_OK if it is OK, or staus port of KBC
 *
 */
UINT32 wait_to_send2 (void)
{
	UINT32 wTestStatus;
	UINT8  bTimeout;
	UINT8  bStatusByte;

	wTestStatus = E__OK;

	bTimeout = 0;

	/** Wait for INPB status bit to go high. Upto 25 mS */

	bStatusByte = dIoReadReg(STATUS_PORT, REG_8);

	while ((bTimeout <= TIMEOUT_LIMIT) && !(bStatusByte & 0x02))
	{
		vDelay(1);
		bTimeout++;
		bStatusByte = dIoReadReg(STATUS_PORT, REG_8);
	}

	/* Wait for INPB status bit to go low. */

	bTimeout = 0;

	bStatusByte = dIoReadReg(STATUS_PORT, REG_8);

	/* Check for kb input buffer empty. */
	while ((bTimeout <= TIMEOUT_LIMIT) && (bStatusByte & 0x02)) /* Kb read data */
	{
		vDelay(1);
		bTimeout++;
		bStatusByte = dIoReadReg(STATUS_PORT, REG_8);
	}

	if ((bTimeout >= TIMEOUT_LIMIT) || (bStatusByte&0x02))
		wTestStatus = bStatusByte;

	return wTestStatus;
}



/******************************************************************************
 *
 * read_ps2mouse - read one byte data from PS/2 mouse
 *
 * Read one byte data from PS/2 mouse over output buffer of keyboard controler
 *
 * RETURNS: E_OK if it is OK, or error code.
 *          mouse_data - data from PS/2 mouse
 *
 * Error codes:
 * 	ERR_NO_PS2_DATA
 *
 */
UINT32 read_ps2mouse( UINT8 *mouse_data )  /* where to put data from PS/2 mouse */
{
	UINT32 wTestStatus;
	UINT16 timeout;
	UINT8  bStatusByte;
#ifdef DEBUG
    UINT8 bOutBuffer;
	char buffer[80];
#endif

	wTestStatus = E__OK;
	timeout = 0;
	bStatusByte = 0;	/* inbyte(STATUS_PORT); */
	while ((timeout < TIMEOUT_LIMIT) && ((bStatusByte & 0x01) == 0))
	{
		vDelay(1);
		bStatusByte = dIoReadReg(STATUS_PORT, REG_8);
		if ((bStatusByte & 0x21) == 0x01)
		{
			/* if received byte is not from AUX port, delete from buffer */
			bStatusByte = 0;
			dIoReadReg(KBOUT_BUFF, REG_8);
		}
		timeout++;
	}

	if (timeout >= TIMEOUT_LIMIT)
	{
		/* PS/2 did not respond to request. */
#ifdef DEBUG
		bOutBuffer = dIoReadReg(KBOUT_BUFF, REG_8);      /**** for monitor data ****/
		sprintf(buffer, "TIMEOUT waiting for reading PS/2 mouse. Status %x, Data %x\n", bStatusByte, bOutBuffer);
		vConsoleWrite(buffer);
#endif
		wTestStatus = ERR_NO_PS2_DATA;
	}
	else
	{
		/* Read result from keyboard controller. */
		*mouse_data = dIoReadReg(KBOUT_BUFF, REG_8);
	}

	return wTestStatus;
}


/******************************************************************************
 *
 * write_ps2mouse - write command to PS/2 mouse
 *
 * First write command to keyboard controler for aux write and after
 * write command for PS/2 mouse to input buffer and check ACK answer from PS/2 mouse.
 *
 * RETURNS: E_OK if it is OK, or error code
 *
 * Error codes:
 *	ERR_NO_READ_CMD
 *	ERR_NO_READ_DATA
 *	ERR_FALSE_PS2_ACK
 *	ERR_NO_PS2_ACK
 *
 */
UINT32 write_ps2mouse
(
	UINT8 mouse_command   /* command for PS/2 mouse */
)
{
	UINT32 wTestStatus;
	UINT16 wWaitResult;
	UINT8 bOutBuffer;
#ifdef DEBUG
	char buffer[80];
#endif

	wTestStatus = E__OK;

	/* Write PS/2 mouse command to keyboard controler */

	vIoWriteReg(CMD_PORT, REG_8, WRITE_AUX);
	wWaitResult = wait_to_send2();

	if (wWaitResult != E__OK)
	{
		/* Keyboard controler did not read command byte. */
#ifdef DEBUG
		vConsoleWrite("Aux write command not read.\n");
#endif
		wTestStatus = ERR_NO_READ_CMD;
	}

	/* Write command to PS/2 mouse */

	if (wTestStatus == E__OK)
	{
		vIoWriteReg(KBIN_BUFF, REG_8, mouse_command);
		wWaitResult = wait_to_send2();

		if (wWaitResult != E__OK)
		{
			/* PS/2 mouse did not read command byte. */
#ifdef DEBUG
			vConsoleWrite("PS/2 not read command.\n");
#endif
			wTestStatus = ERR_NO_READ_DATA;
		}
	}

	/* Wait to receive ACK from PS/2 mouse */

	if (wTestStatus == E__OK)
	{
		if (read_ps2mouse (&bOutBuffer) == E__OK)
		{
			if (bOutBuffer != PS2RET_ACK)
			{
#ifdef DEBUG
				sprintf (buffer, "Received byte from PS/2 mouse is not ACK. Received %x\n", bOutBuffer);
				vConsoleWrite (buffer);
#endif
				wTestStatus = ERR_FALSE_PS2_ACK;
			}
		}
		else
		{
#ifdef DEBUG
			vConsoleWrite ("Not received ACK from PS/2 mouse.\n");
#endif
			wTestStatus = ERR_NO_PS2_ACK;
		}
	}

	return wTestStatus;
}





/******************************************************************************
 *
 * reset_ps2mouse - reset PS/2 mouse
 *
 * Reset mouse and check answers after reset.
 *
 * RETURNS: E_OK if it is OK, or error code
 *
 * Error codes:
 *	ERR_FLUSH
 *	ERR_NO_READ_CMD
 *	ERR_NO_READ_DATA
 *	ERR_NO_MOUSE
 *	ERR_MOUSE
 *
 */
UINT32 reset_ps2mouse (void)
{
	UINT32 wTestStatus;
	UINT32 wWaitResult;
	UINT16 wTimeout;
	UINT8 bOutBuffer;
#ifdef DEBUG
	char buffer[80];
#endif

	wTestStatus = flush_outbuffer();	/* flush output buffer */

	/* Send write aux command to keyboard controler */

	if (wTestStatus == E__OK)
	{
		vIoWriteReg(CMD_PORT, REG_8, WRITE_AUX);
		wWaitResult = wait_to_send2();

		if (wWaitResult != E__OK)
		{
			/* Keyboard controler did not read command byte. */
			wTestStatus = ERR_NO_READ_CMD;
		}
	}

	/* Write reset command to PS/2 mouse */

	if (wTestStatus == E__OK)
	{
		vIoWriteReg(KBIN_BUFF, REG_8, PS2_RESET);
		wWaitResult = wait_to_send2();

		if (wWaitResult != E__OK)
		{
			/* PS/2 mouse did not read command byte. */
			wTestStatus = ERR_NO_READ_DATA;
		}
	}

	/* Wait to receive ACK from PS/2 mouse */

	if (wTestStatus == E__OK)
	{
		wTestStatus = read_ps2mouse (&bOutBuffer);
		while (wTestStatus == E__OK && bOutBuffer != PS2RET_ACK)
		{
			wTestStatus = read_ps2mouse (&bOutBuffer);
		}

		if (wTestStatus != E__OK)
			wTestStatus = ERR_NO_MOUSE;
	}

	/* Wait to receive second byte (AAh) */

	if (wTestStatus == E__OK)
	{
		wTimeout = 0;
		wTestStatus = ERR_GENERAL;
		while (wTimeout < TIMEOUT_PS2_RESET && wTestStatus != E__OK)
		{
			vDelay (1);
			wTestStatus = read_ps2mouse (&bOutBuffer);
			wTimeout++;
		}

		if (wTimeout >= TIMEOUT_PS2_RESET)
		{
#ifdef DEBUG
			vConsoleWrite("No second byte after reset.\n");
#endif
			wTestStatus = ERR_NO_MOUSE;
		}
		else
		{
			bOutBuffer = dIoReadReg(KBOUT_BUFF, REG_8);
#ifdef DEBUG
			sprintf (buffer, "Timeout=%d", wTimeout);
			vConsoleWrite (buffer);
#endif
		}

		if (wTestStatus == E__OK)
		{
			if (bOutBuffer == PS2RET_ERROR)
				wTestStatus = ERR_MOUSE;
			else if (bOutBuffer != PS2RET_RESET)
			{
#ifdef DEBUG
				sprintf(buffer, "Wrong second byte after reset. Received %x\n", bOutBuffer);
				vConsoleWrite(buffer);
#endif
				wTestStatus = ERR_NO_MOUSE;
			}
		}
	}

	return wTestStatus;
}




/******************************************************************************
 *
 * packet_ps2mouse - receives data packet from PS/2 mouse
 *
 * Receives 3 bytes with data (buttons status, X and Y offset)
 *
 * RETURNS: E_OK if it is OK, or error code
 *			sDataPacket - data packet from PS/2 mouse
 *
 * Error codes:
 *	ERR_NO_PS2_PACKET
 *
 */
UINT32 packet_ps2mouse
(
	PS2MOUSE_PACKET *sDataPacket	/* where to put data packet from PS/2 mouse */
)
{
	UINT32 wTestStatus;
	UINT8  bOutBuffer;
	UINT16 i;		/* for loop counter */

	wTestStatus = E__OK;

	/* Read data packet from PS/2 mouse */

	for (i = 0; i < PACKET_LEN; i++)
	{
		if (wTestStatus == E__OK)
		{
			wTestStatus = read_ps2mouse(&bOutBuffer);
			if (wTestStatus == E__OK)
			{
				switch (i)
				{
					case PACKET_STATUS: sDataPacket->bStatus = bOutBuffer; break;
					case PACKET_X: 		sDataPacket->bXData = bOutBuffer; break;
					case PACKET_Y: 		sDataPacket->bYData = bOutBuffer; break;
				}
			}
		}
		else
		  break;
	}

	if (wTestStatus != E__OK)
	{
		#ifdef DEBUG
			vConsoleWrite("PS/2 data packet not received");
		#endif
		wTestStatus = ERR_NO_PS2_PACKET;
	}

	return wTestStatus;
}




/******************************************************************************
 *
 * move_ps2mouse - move PS/2 mouse
 *
 * Check PS/2 mouse when it is moved and pressed buttons.
 *
 * RETURNS: E_OK if it is OK, or error code
 *
 * Error codes:
 *	ERR_NO_READ_CMD
 *	ERR_NO_READ_DATA
 *	ERR_FALSE_PS2_ACK
 *	ERR_NO_PS2_ACK
 * 	ERR_NO_PS2_DATA
 *
 */
UINT32 move_ps2mouse(void)
{
	UINT32 wTestStatus;
	UINT8  bPrevStatus;
	UINT16 i;
	PS2MOUSE_PACKET sMousePacket;
	char buffer[128];

	wTestStatus = E__OK;

	/* Init PS/2 mouse to standard mode */
	wTestStatus = write_ps2mouse (PS2_SET_STANDARD);

	/* Set PS/2 mouse to remote mode */
	if (wTestStatus == E__OK)
		wTestStatus = write_ps2mouse (PS2_SET_REMOTE);

	/* Test PS/2 mouse moving */
	vConsoleWrite("Move and press mouse next 10s");

	if (wTestStatus == E__OK)
		wTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */

	if (wTestStatus == E__OK)
		wTestStatus = packet_ps2mouse (&sMousePacket);	/* read for first time */

	if (wTestStatus == E__OK)
	{
 		bPrevStatus = 0;	/* for first time */

		for (i = 0; i < TIMEOUT_MOUSE_MOVE; i++)
		{
			vDelay(DELAY_SAMPLE);	/* wait for next sample */

			if (wTestStatus == E__OK)
				wTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */

			if (wTestStatus == E__OK)
				wTestStatus = packet_ps2mouse (&sMousePacket);	/* read for first time */

			/* write mouse changing */

			if (wTestStatus == E__OK)
			{
				/* check if mouse was pressed or realised button ore moved */
				if ( ((bPrevStatus & (STATUS_BUTTONS)) ^ (sMousePacket.bStatus & (STATUS_BUTTONS))) ||
					 (sMousePacket.bXData != 0) || (sMousePacket.bYData != 0) )
				{
					sprintf (buffer, "BUTTONS: ");
					if (sMousePacket.bStatus & STATUS_LEFT)
						sprintf (buffer, "%s LEFT ", buffer);	/* LEFT pressed */
					else
						sprintf (buffer, "%s      ", buffer);
					if (sMousePacket.bStatus & STATUS_RIGHT)
						sprintf (buffer, "%s RIGHT", buffer);	/* RIGHT pressed */
					else
						sprintf (buffer, "%s      ", buffer);
					if (sMousePacket.bStatus & STATUS_MIDDLE)
						sprintf (buffer, "%s MIDDLE", buffer);	/* MIDDLE pressed */
					else
						sprintf (buffer, "%s       ", buffer);

					sprintf (buffer, "%s   MOVE: ", buffer);
					if (sMousePacket.bXData != 0)
					{
						if (sMousePacket.bStatus & STATUS_XNG)
							sprintf (buffer, "%s LEFT", buffer);	/* move LEFT */
						else
							sprintf (buffer, "%s RIGHT", buffer);	/* move RIGHT */
					}
					if (sMousePacket.bYData != 0)
					{
						if (sMousePacket.bStatus & STATUS_YNG)
							sprintf (buffer, "%s DOWN", buffer);	/* move DOWN */
						else
							sprintf (buffer, "%s UP", buffer);		/* move UP */
					}
					sprintf (buffer, "%s\n", buffer);
					vConsoleWrite (buffer);
				}
		 		bPrevStatus = sMousePacket.bStatus;
			}
		}
	}

	return wTestStatus;
}


/******************************************************************************
 *
 * action_ps2mouse - test actions of PS/2 mouse
 *
 * Check PS/2 mouse actions separately (left and right buttons, and move in each directions),
 *
 * RETURNS: E_OK if it is OK, or error code
 *
 * Error codes:
 *	ERR_TIMEOUT_ACTION
 *	ERR_NO_READ_CMD
 *	ERR_NO_READ_DATA
 *	ERR_FALSE_PS2_ACK
 *	ERR_NO_PS2_ACK
 * 	ERR_NO_PS2_DATA
 *
 */

UINT32 action_ps2mouse(void)
{
	UINT32 wTestStatus;
	UINT16 timeout;
	UINT8 bDone;
	PS2MOUSE_PACKET sMousePacket;

	wTestStatus = E__OK;

	/* Init PS/2 mouse to standard mode */

	wTestStatus = write_ps2mouse (PS2_SET_STANDARD);

	/* Set PS/2 mouse to remote mode */

	if (wTestStatus == E__OK)
		wTestStatus = write_ps2mouse (PS2_SET_REMOTE);

	/* Test PS/2 LEFT button */

	if (wTestStatus == E__OK)
	{
		vConsoleWrite ("\nPress LEFT button");
		bDone = FALSE;
		timeout = 0;

		while (timeout < TIMEOUT_MOUSE_ACTION  &&  wTestStatus == E__OK  &&  !bDone)
		{
			if (wTestStatus == E__OK)
				wTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (wTestStatus == E__OK)
				wTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bStatus & STATUS_LEFT)
				bDone = TRUE;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			vConsoleWrite ("  Timeout");
			wTestStatus = ERR_TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 RIGHT button */

	if (wTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("\nPress RIGHT button");
		bDone = FALSE;
		timeout = 0;

		while (timeout < TIMEOUT_MOUSE_ACTION  &&  wTestStatus == E__OK  &&  !bDone)
		{
			if (wTestStatus == E__OK)
				wTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (wTestStatus == E__OK)
				wTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bStatus & STATUS_RIGHT)
				bDone = TRUE;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			vConsoleWrite ("  Timeout");
			wTestStatus = ERR_TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 move UP */

	if (wTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("\nMove UP");
		bDone = FALSE;
		timeout = 0;

		while (timeout < TIMEOUT_MOUSE_ACTION  &&  wTestStatus == E__OK  &&  !bDone)
		{
			if (wTestStatus == E__OK)
				wTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (wTestStatus == E__OK)
				wTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bYData != 0  &&  !(sMousePacket.bStatus & STATUS_YNG))
				bDone = TRUE;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			vConsoleWrite ("  Timeout");
			wTestStatus = ERR_TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 move DOWN */

	if (wTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("\nMove DOWN");
		bDone = FALSE;
		timeout = 0;

		while (timeout < TIMEOUT_MOUSE_ACTION  &&  wTestStatus == E__OK  &&  !bDone)
		{
			if (wTestStatus == E__OK)
				wTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (wTestStatus == E__OK)
				wTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bYData != 0  &&  (sMousePacket.bStatus & STATUS_YNG))
				bDone = TRUE;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			vConsoleWrite ("  Timeout");
			wTestStatus = ERR_TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 move LEFT */

	if (wTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("\nMove LEFT");
		bDone = FALSE;
		timeout = 0;

		while (timeout < TIMEOUT_MOUSE_ACTION  &&  wTestStatus == E__OK  &&  !bDone)
		{
			if (wTestStatus == E__OK)
				wTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (wTestStatus == E__OK)
				wTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bXData != 0  &&  (sMousePacket.bStatus & STATUS_XNG))
				bDone = TRUE;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			vConsoleWrite ("  Timeout");
			wTestStatus = ERR_TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 move RIGHT */

	if (wTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("\nMove RIGHT");
		bDone = FALSE;
		timeout = 0;

		while (timeout < TIMEOUT_MOUSE_ACTION  &&  wTestStatus == E__OK  &&  !bDone)
		{
			if (wTestStatus == E__OK)
				wTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (wTestStatus == E__OK)
				wTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bXData != 0  &&  !(sMousePacket.bStatus & STATUS_XNG))
				bDone = TRUE;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			vConsoleWrite ("  Timeout");
			wTestStatus = ERR_TIMEOUT_ACTION;
		}
	}

	vConsoleWrite ("\n");
	return wTestStatus;
}
