
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

/* keybd.c - Keyboard/ mouse test, which tests the PS2 Kbd/ mouse controller
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/keybd.c,v 1.2 2013-10-08 07:13:38 chippisley Exp $
 *
 * $Log: keybd.c,v $
 * Revision 1.2  2013-10-08 07:13:38  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.4  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.3  2009/06/03 08:30:58  cvsuser
 * Tidy up test names
 *
 * Revision 1.2  2009/05/21 08:14:53  cvsuser
 * Shortened test name length
 *
 * Revision 1.1  2009/05/15 11:11:27  jthiru
 * Initial checkin for Keyboard & Mouse Controller tests
 *
 *
 */


/* includes */

#include <stdtypes.h>
#include <errors.h>		
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <devices/keybd.h>
#include <bit/hal.h>
 
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>


/* defines */
// #define DEBUG 
#ifdef DEBUG
		static char buffer[128];
#endif


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */
extern void vDelay(UINT32 dMsDelay);
extern void vConsoleWrite(char*	achMessage);

/* forward declarations */


/**************************************************************************/
/*  wait_to_send.                                                         */
/*                                                                        */
/*    This function waits upto 25 milliseconds for the keyboard to read   */
/* and acknowledge a command or data sent to it. The function responds    */
/* with E__OK if the data was read of the status register value if not.     */
/*                                                                        */
/**************************************************************************/

static UINT32 wait_to_send
(
	void
)
{
	UINT32 dtest_status;
	UINT8 timeout;
	UINT8 status_byte;

	dtest_status = E__OK;

	timeout = 0;

	/** Wait for status bit to go high. Upto 25 mS */

	status_byte = dIoReadReg(STATUS_PORT, REG_8);

	/* Check for kb input buffer empty. */
	while ((timeout <= TIMEOUT_LIMIT) && !(status_byte & 0x02))
	{
		vDelay(1);
		timeout++;
		status_byte = dIoReadReg(STATUS_PORT, REG_8);
	}

	/* Wait for status bit to go low. */
	timeout = 0;

	status_byte = dIoReadReg(STATUS_PORT, REG_8);

	/* Check for kb input buffer empty. */
	while ((timeout <= TIMEOUT_LIMIT) && (status_byte & 0x02)) /* Kb read data */
	{
		vDelay(1);
		timeout++;
		status_byte = dIoReadReg(STATUS_PORT, REG_8);
	}

	if ((timeout >= TIMEOUT_LIMIT) || (status_byte & 0x02))
	 dtest_status = status_byte;

	return dtest_status;
}

#if 0
/*****************************************************************************
 * dKbdInit: Keyboard Interrupt init function 
 *
 * RETURNS: E__OK or error code
 */

static UINT32 dKbdInit
(
	UINT8 kb_int
)
{
	UINT32 dtest_status = E__OK;
	UINT16 timeout;
	UINT8 tmpbyte,status_byte;
	UINT32 dwait_result;  
	UINT8 keybd_present;
#if 0
	/* To check if keyboard present, write out 0xEE to the output port  */
    /* (0x60). Now read keyboard output buffer until empty. */
    /* An 0xEE should have been returned. Enable keyboard and  */
    /* read responses if any. If the 0xEE was received from the key-  */
    /* board then it has echo'ed back and so is present.          */

    dwait_result = wait_to_send();
    if (dwait_result == E__OK)
    {
      /* Write out an ECHO cmd to the keyboard. */
      vIoWriteReg(KBIN_BUFF, REG_8, KEYBD_ECHO);

      dwait_result = wait_to_send();
      if (dwait_result == E__OK)
	  {
        keybd_present = 0;

        vDelay(25);
        /* Read reply from echo command. */
        timeout = 0;
        status_byte = dIoReadReg(STATUS_PORT, REG_8);   /* Read status */

        while ((timeout <= 255) &&   /* If haven't read 255 characters and */
               (status_byte & 0x01))  /* output buffer is full. */
		{
			tmpbyte = dIoReadReg(KBOUT_BUFF, REG_8);     /* Read char out of buffer. */
			if (tmpbyte == KEYBD_ECHO) 
				keybd_present = 1;
			vDelay(1);
			status_byte = dIoReadReg (STATUS_PORT, REG_8);       /* Read status byte now. */
			timeout++;
		}
      }    
      if (!keybd_present) 
      {
      	dtest_status = E__KBD_RW;
#ifdef DEBUG
	  	vConsoleWrite("Keyboard not present.\n");
#endif
      }
    }
#endif
	if (dtest_status == E__OK)
	{
		/* If keyboard input buffer empty */
		dwait_result = wait_to_send();
		if (dwait_result == E__OK)
		{
			vIoWriteReg(CMD_PORT, REG_8, KEYBD_ENABLE);    /* Enable keyboard. */

			/* Check keyboard buffer is empty, Enable command has been read. */
			dwait_result = wait_to_send();
			if (dwait_result != E__OK)
			{
#ifdef DEBUG
				/* Key board was never ready for initialisation. May not be there. */
				sprintf(buffer,"Key board not read enable cmd. Status %x.\n",
				                                   ((dwait_result&0x0F0)>>4));
				vConsoleWrite(buffer);
#endif
				dtest_status = E__KBC_RW;
			}
		}

		/* Read enable response if there is one. */
		vDelay(25);
		timeout = 0;
		status_byte = dIoReadReg(STATUS_PORT, REG_8);   /* Read status */

		while ((timeout <= 255) &&       /* If haven't read 255 characters and */
		   (status_byte &0x01))      /* output buffer is full. */
		{
			tmpbyte = dIoReadReg(KBOUT_BUFF, REG_8);            /* Read char out of buffer. */
			vDelay(1);
			status_byte = dIoReadReg(STATUS_PORT, REG_8);       /* Read status byte now. */
			timeout++;
		}
	}

	if (dtest_status == E__OK)
	{
		/* Write command to initialise keyboard. */
		vIoWriteReg(CMD_PORT, REG_8, CMD_BYTE);

		/* Now wait for command to be read. */
		dwait_result = wait_to_send();
		if (dwait_result != E__OK)
		{
#ifdef DEBUG
			  /* Key board did not read command byte. May not be present. */
			  sprintf(buffer,"Key board not read mode cmd. Status %x.\n",
			                                ((dwait_result&0x0F0)>>4));
			  vConsoleWrite(buffer);
#endif
			  dtest_status = E__KBD_RW;
		}

		if (dtest_status == E__OK)
		{
			/* Mode command read ok. Now send mode data. */
			/* Write initialisation code to command register. */
			tmpbyte = 0x40 | kb_int;
			vIoWriteReg(KBIN_BUFF, REG_8, tmpbyte);

			/* Now wait for command byte to be read. */
			dwait_result = wait_to_send();
			if (dwait_result != E__OK)
			{
#ifdef DEBUG
				/* Key board did not read command data byte. */
				sprintf(buffer,"Key board did not read cmd data. Status %x.\n",
				                               ((dwait_result & 0x0F0) >> 4));
				vConsoleWrite(buffer);
#endif
				dtest_status = E__KBC_RW;
			}
		} /* End of mode cmd read ok. */
	}
	return dtest_status;
}
#endif

/*****************************************************************************
 * dKbdevicetest: Keyboard Interconnect testing function 
 *
 * RETURNS: E__OK or error code
 */

static UINT32 dKbdevicetest
(
	void
)
{
	UINT32 dtest_status = E__OK;
	UINT16 timeout;
	UINT8 tmpbyte,status_byte;
	UINT32 dwait_result;  
	UINT8 keybd_present;

	/* To check if keyboard present, write out 0xEE to the output port  */
    /* (0x60). Now read keyboard output buffer until empty. */
    /* An 0xEE should have been returned. Enable keyboard and  */
    /* read responses if any. If the 0xEE was received from the key-  */
    /* board then it has echo'ed back and so is present.          */

    dwait_result = wait_to_send();
    if (dwait_result == E__OK)
    {
      /* Write out an ECHO cmd to the keyboard. */
      vIoWriteReg(KBIN_BUFF, REG_8, KEYBD_ECHO);

      dwait_result = wait_to_send();
      if (dwait_result == E__OK)
	  {
        keybd_present = 0;

        vDelay(25);
        /* Read reply from echo command. */
        timeout = 0;
        status_byte = dIoReadReg(STATUS_PORT, REG_8);   /* Read status */

        while ((timeout <= 255) &&   /* If haven't read 255 characters and */
               (status_byte & 0x01))  /* output buffer is full. */
		{
			tmpbyte = dIoReadReg(KBOUT_BUFF, REG_8);     /* Read char out of buffer. */
			if (tmpbyte == KEYBD_ECHO) 
				keybd_present = 1;
			vDelay(1);
			status_byte = dIoReadReg (STATUS_PORT, REG_8);       /* Read status byte now. */
			timeout++;
		}
      }    
      if (!keybd_present) 
      {
      	dtest_status = E__KBD_RW;
#ifdef DEBUG
	  	vConsoleWrite("Keyboard not present.\n");
#endif
      }
    }
  return dtest_status;
}


/*****************************************************************************
 * dKbCInit: Initialise Keyboard Controller 
 *
 * RETURNS: E__OK or error code
 */

static UINT32 dKbCTest
(
	void
)
{
	UINT32 dtest_status;
	UINT8 status_byte;
	UINT16 timeout;
	
	dtest_status = E__OK;

	/* 
	* Enable AUX port for boards which have PS/2 mouse port 
	*/
	
	vIoWriteReg(CMD_PORT, REG_8, 0xA8);
	vDelay (10);

	/*** Flush the keyboard buffer. ***/
	timeout = 0;
	status_byte = dIoReadReg(STATUS_PORT, REG_8);   /* Read status */

	while ((timeout <= 255) &&       /* If haven't read 255 characters and */
	 (status_byte & 0x01))      /* output buffer is full. */
	{
		dIoReadReg(KBOUT_BUFF, REG_8);            /* Read char out of buffer. */
		vDelay(1);
		status_byte = dIoReadReg(STATUS_PORT, REG_8);       /* Read status byte now. */
		timeout++;
	}

	/* Check that it didn't time out. */
	if (timeout >= 255) 
	{
#ifdef DEBUG
		/* Key board was never ready for initialisation. Did not flush */
		vConsoleWrite("Key board Controller not ready. Timed out while flushing.");
#endif
		dtest_status = E__KBC_RW;
	}
	return dtest_status;
}


/*****************************************************************************
 * kb_selftest: test function to execute Keyboard's self test
 *							
 *
 * RETURNS: E__OK or error code
 */

static UINT32 dKbselftest
(
	void
)
{
	UINT32 dtest_status;
	UINT8 timeout;
	UINT32 dwait_result;

	dtest_status = E__OK;

	/* Write command to tell keyboard controller to perform self test. */
	vIoWriteReg(CMD_PORT, REG_8, SELF_TEST);

	/* Now wait for command to be read. */
	dwait_result = wait_to_send();
	if (dwait_result != E__OK)
	{
#ifdef DEBUG
		/* Key board did not read command byte. */
		sprintf(buffer,"Self test command not read. Status %x.\n",
		                             ((dwait_result & 0x0F0) >> 4));
		vConsoleWrite(buffer);
#endif
		dtest_status = E__KBC_RW;
	}

	if (dtest_status == E__OK)
	{
		/* Now wait to read response to self test. */
		timeout = 0;

		while ((timeout < TIMEOUT_LIMIT) &&
		       ((dIoReadReg(STATUS_PORT, REG_8) & 0x01) == 0))
		{
			vDelay(1);
			timeout++;
		}
		if (timeout >= TIMEOUT_LIMIT)
		{
#ifdef DEBUG
		  	/* Key board did not respond to selftest. */
			vConsoleWrite("TIMEOUT waiting for self test result.\n");
#endif
			dtest_status = E__KBC_TIMEOUT;
		}
		else
		{
		  	/* Read result from keyboard controller. */
			dwait_result = dIoReadReg(KBOUT_BUFF, REG_8);
		  	if (dwait_result != 0x55)
		  	{
#ifdef DEBUG
		    	/* Key board returned error result to selftest. */
				sprintf (buffer, "Self test failed - returned %02X\n", dwait_result);
		    	vConsoleWrite(buffer);
#endif
		    	dtest_status = E__KBC_SELFTEST;
		  	}
		} /* End of Not read result timeout. */
	} /* End of read self test command. */
#if 1
	if (dtest_status == E__OK)
	{
		dwait_result = wait_to_send();
		if (dwait_result != E__OK)
		{
#ifdef DEBUG
		  /* Key board did not read command byte. */
		  vConsoleWrite("Keyboard not ready after Self test\n");
#endif
		  dtest_status = E__KBC_RW;
		}
	}
#endif
	return dtest_status;
}


/*****************************************************************************
 * dKbCInttest: test function to test Keyboard's interrupt
 *							
 *
 * RETURNS: E__OK or error code
 */

static UINT32 dKbCInttest
(
	void
)
{
	UINT32 dwait_result;
	UINT32 dtest_status;
#if 0
	UINT8 i;

	dKbdInit(1);
	
    vIoWriteReg(CMD_PORT, REG_8, READ_OUTPUT);

    /* Now wait for command to be read. */
    dwait_result = wait_to_send();
    if (dwait_result != E__OK)
    {
#ifdef DEBUG
      /* Key board did not read command byte. */
      sprintf(buffer,"Read output port command not read. Status %x.\n",
                                 ((dwait_result&0x0F0)>>4));
      vConsoleWrite(buffer);
#endif
    }
	

	for (i = 0; i < 25; i++)
	{
#ifdef DEBUG

		sprintf(buffer, "Status Port: 0x%x Data: 0x%x\n", dIoReadReg(STATUS_PORT, REG_8), 
				dIoReadReg(KBOUT_BUFF, REG_8));
		vConsoleWrite(buffer);
#endif
	}

	dtest_status = E__KBD_NOT_IMPL;
#endif
	/* Write command to initialise keyboard. */
	vIoWriteReg(CMD_PORT, REG_8, CMD_BYTE);

	/* Now wait for command to be read. */
	dwait_result = wait_to_send();
	if (dwait_result != E__OK)
	{
#ifdef DEBUG
		  /* Key board did not read command byte. May not be present. */
		  sprintf(buffer,"Key board not read mode cmd. Status %x.\n",
										((dwait_result&0x0F0)>>4));
		  vConsoleWrite(buffer);
#endif
		  dwait_result = E__KBD_RW;
	}

	dtest_status = dwait_result;
	
	return dtest_status;
}


/******************************************************************************
 *
 * flush_outbuffer - flush output buffer from keyboard controller
 * 
 * Read out buffer while there are something
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */

static UINT32 flush_outbuffer 
(
	void
)
{
	UINT32 dTestStatus;
	UINT16 timeout;
	UINT8 bStatusByte;

	dTestStatus = E__OK;
	
	timeout = 0;
	bStatusByte = dIoReadReg(STATUS_PORT, REG_8);   /* Read status */

	while ((timeout <= 255) &&       /* If haven't read 255 characters and */
			(bStatusByte &0x01))      /* output buffer is full. */
	{
		dIoReadReg(KBOUT_BUFF, REG_8);            /* Read char out of buffer. */
		bStatusByte = dIoReadReg(STATUS_PORT, REG_8);       /* Read status byte now. */
		timeout++;
	}

	/* Check that it didn't time out. */

	if (timeout >= 255) 
	{
		/* Keyboard was never ready for initialisation. Did not flush */
#ifdef DEBUG
		vConsoleWrite ("Keyboard controller not ready. Timed out while flushing.\n");
#endif
		dTestStatus = E__KBC_ERR_FLUSH;
	}

	return dTestStatus;
}


/******************************************************************************
 *
 * aux_enable - enable aux port
 *
 * Send command to keyboard controler to enable aux port.
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */
 
static UINT32 aux_enable 
(
	void
)
{
	UINT32 dTestStatus;
	UINT32 dWaitResult;

	dTestStatus = E__OK;

	dTestStatus = flush_outbuffer();

	/* Enable auxiliary device */

	if (dTestStatus == E__OK)
	{
		vIoWriteReg (CMD_PORT, REG_8, ENABLE_AUX);
		dWaitResult = wait_to_send();

		if (dWaitResult != E__OK)
		{
			/* Keyboard controller did not read command byte. */
#ifdef DEBUG
			sprintf (buffer, "Enable auxiliary device command not read. Status %x.\n",
					 dWaitResult);
			vConsoleWrite(buffer);
#endif
			dTestStatus = E__KBC_NO_AUX_ENABLE;
		}
	}

	return dTestStatus;
}


/******************************************************************************
 *
 * read_ps2mouse - read one byte data from PS/2 mouse
 *
 * Read one byte data from PS/2 mouse over output buffer of keyboard controler
 *
 * RETURNS: E__OK if it is OK, or error code.
 *          mouse_data - data from PS/2 mouse
 *
 *
 */

static UINT32 read_ps2mouse
(
	UINT8 *mouse_data   /* where to put data from PS/2 mouse */
)
{
	UINT32 dTestStatus;
	UINT16 timeout;
	UINT8 bStatusByte; 
#ifdef DEBUG
    UINT8 bOutBuffer;
#endif

	dTestStatus = E__OK;
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
		bOutBuffer = dIoReadReg(KBOUT_BUFF, REG_8); 	 /**** for monitor data ****/
		sprintf(buffer, "TIMEOUT waiting for reading PS/2 mouse. Status %x, Data %x, timeout %d\n", bStatusByte, bOutBuffer, timeout);
		vConsoleWrite(buffer);
#endif
		dTestStatus = E__KBC_NO_PS2_DATA;
	}
	else
	{
		/* Read result from keyboard controller. */
		*mouse_data = dIoReadReg(KBOUT_BUFF, REG_8);
	}

	return dTestStatus;  
}


/******************************************************************************
 *
 * aux_echo - send echo to aux port
 *
 * Send echo to aux port via D3h command for KBC and check answer
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */

static UINT32 aux_echo 
(
	void
)
{
	UINT32 dTestStatus;
	UINT32 dWaitResult;
#ifdef DEBUG
	UINT8 bStatusByte;
#endif
	UINT8 bOutBuffer;

	/* Flush output buffer */
	dTestStatus = flush_outbuffer();
	
	/* Send echo to aux port */

	if (dTestStatus == E__OK)
	{
		vIoWriteReg (CMD_PORT, REG_8, ECHO_AUX);	
		dWaitResult = wait_to_send();
		if (dWaitResult != E__OK)
		{
			/* Keyboard controller did not read command byte. */
#ifdef DEBUG
			sprintf (buffer, "Echo auxiliary port command not written. Status %x.\n",
					 dWaitResult);
			vConsoleWrite (buffer);
#endif
			dTestStatus = E__KBC_CMD_EAUX;
		}
		else
		{
			vIoWriteReg (KBIN_BUFF, REG_8, PS2_SET_SCALING);	/* send something */
			dWaitResult = wait_to_send();
			if (dWaitResult != E__OK)
			{
				/* Keyboard controller did not read byte. */
#ifdef DEBUG
				sprintf (buffer, "Not written to auxiliary port. Status %x.\n", dWaitResult);
				vConsoleWrite (buffer);
#endif
				dTestStatus = E__KBC_CMD_RD;
			}
			else
			{
				dWaitResult = read_ps2mouse (&bOutBuffer);
				if (dWaitResult == E__OK)
				{
					if (bOutBuffer != PS2_SET_SCALING)
					{
#ifdef DEBUG
						sprintf (buffer, "Wrong echo from auxiliary port. Sent %x, Received %x.\n", 
									PS2_SET_SCALING, bOutBuffer);
						vConsoleWrite (buffer);
						bStatusByte = dIoReadReg(STATUS_PORT, REG_8);       /* Read status byte now. */
						sprintf (buffer, "Status %x\n", bStatusByte);
						vConsoleWrite (buffer);
#endif
						dTestStatus = E__KBC_FALSE_AUX_ECHO;
					}
				}
				else
				{
#ifdef DEBUG
					sprintf (buffer, "Not received echo from auxiliary port. Status %x.\n", dWaitResult);
					vConsoleWrite (buffer);
#endif
					dTestStatus = E__KBC_NO_AUX_ECHO;
				}
			}
		}
	}
	
	return dTestStatus;
}


/******************************************************************************
 *
 * test_auxbus - test PS/2 bus
 *
 * Send command to keyboard controler to test auxiliary bus.
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */
 
static UINT16 test_auxbus 
(
	void
)
{
	UINT32 dTestStatus;
	UINT32 dWaitResult;
	UINT16 timeout;

	dTestStatus = flush_outbuffer();

	/* Test auxaliary bus */

	if (dTestStatus == E__OK)
	{
		vIoWriteReg (CMD_PORT, REG_8, AUXIF_TEST);
		dWaitResult = wait_to_send();
  
		if (dWaitResult != E__OK)
		{
			/* Key board did not read command byte. */
#ifdef DEBUG
			sprintf(buffer, "Test auxaliary bus command not written. Status %x.\n", dWaitResult);
			vConsoleWrite(buffer);
#endif
			dTestStatus = E__KBC_IF_RD;
		}
	}
  
	if (dTestStatus == E__OK)
	{
		/* Now wait to read response to interface test. */
		timeout = 0;

		dWaitResult = dIoReadReg(STATUS_PORT, REG_8);

		while ((timeout < TIMEOUT_LIMIT) &&
				((dWaitResult & 0x01) == 0))
		{
			vDelay(1);
			dWaitResult = dIoReadReg (STATUS_PORT, REG_8);
			timeout++;
		}

		if (timeout >= TIMEOUT_LIMIT)
		{
			/* Keyboard controller did not respond to interface test. */
#ifdef DEBUG
			vConsoleWrite ("TIMEOUT waiting for aux interface test.\n");
#endif
			dTestStatus = E__AUX_IF_TIMEOUT;
		}
		else
		{
			/* Read result from keyboard controller. */
			dWaitResult = dIoReadReg (KBOUT_BUFF, REG_8);
			if (dWaitResult != 0)
			{
				/* Error on AUX bus */
#ifdef DEBUG
				sprintf (buffer, "AUX interface test fail. Result : %X\n", dWaitResult);
				vConsoleWrite(buffer);
#endif
				dTestStatus = E__AUX_DATA_ERR;
			}
		}
	}

	return dTestStatus;
}


/******************************************************************************
 *
 * reset_ps2mouse - reset PS/2 mouse
 *
 * Reset mouse and check answers after reset.
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */

static UINT32 reset_ps2mouse 
(
	void
)
{
	UINT32 dTestStatus;
	UINT32 dWaitResult;
	UINT16 wTimeout;
	UINT8 bOutBuffer;

	dTestStatus = flush_outbuffer();	/* flush output buffer */

	/* Send write aux command to keyboard controller */

	if (dTestStatus == E__OK)
	{
		vIoWriteReg(CMD_PORT, REG_8, WRITE_AUX);
		dWaitResult = wait_to_send();

		if (dWaitResult != E__OK)
		{
			/* Keyboard controler did not read command byte. */
			dTestStatus = E__AUX_WR_CMD;
		}
	}

	/* Write reset command to PS/2 mouse */

	if (dTestStatus == E__OK)
	{
		vIoWriteReg(KBIN_BUFF, REG_8, PS2_RESET);
		dWaitResult = wait_to_send();

		if (dWaitResult != E__OK)
		{
			/* PS/2 mouse did not read command byte. */
			dTestStatus = E__PS2_RST;
		}
	}

	/* Wait to receive ACK from PS/2 mouse */

	if (dTestStatus == E__OK)
	{
		dTestStatus = read_ps2mouse (&bOutBuffer);
		while (dTestStatus == E__OK && bOutBuffer != PS2RET_ACK)
		{
			dTestStatus = read_ps2mouse (&bOutBuffer);
		}
		
		if (dTestStatus != E__OK)
			dTestStatus = E__NO_MOUSE;
	}

	/* Wait to receive second byte (AAh) */
	
	if (dTestStatus == E__OK) 
	{
		wTimeout = 0;
		dTestStatus = 1;
		while (wTimeout < TIMEOUT_PS2_RESET && dTestStatus != E__OK)
		{
			vDelay (1);
			dTestStatus = read_ps2mouse (&bOutBuffer);
			wTimeout++;
		}
		
		if (wTimeout >= TIMEOUT_PS2_RESET)
		{
#ifdef DEBUG
			vConsoleWrite("No second byte after reset.\n");
#endif
			dTestStatus = E__NO_MOUSE;
		}
		else
		{
			bOutBuffer = dIoReadReg(KBOUT_BUFF, REG_8);
#ifdef DEBUG
			sprintf (buffer, "Timeout=%d\n", wTimeout);
			vConsoleWrite (buffer);
#endif
		}

		if (dTestStatus == E__OK) 
		{
			if (bOutBuffer == PS2RET_ERROR)
				dTestStatus = E__MOUSE_ERR;
			else if (bOutBuffer != PS2RET_RESET)
			{
#ifdef DEBUG
				sprintf(buffer, "Wrong second byte after reset. Received %x\n", bOutBuffer);
				vConsoleWrite(buffer);
#endif
				dTestStatus = E__NO_MOUSE;
			}
		}
	}

	return dTestStatus;
}


/******************************************************************************
 *
 * write_ps2mouse - write command to PS/2 mouse
 *
 * First write command to keyboard controler for aux write and after
 * write command for PS/2 mouse to input buffer and check ACK answer from PS/2 mouse.
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */
 
static UINT16 write_ps2mouse 
(
	UINT8 mouse_command   /* command for PS/2 mouse */
)
{
	UINT32 dTestStatus;
	UINT16 wWaitResult;
	UINT8 bOutBuffer;

	dTestStatus = E__OK;

	/* Write PS/2 mouse command to keyboard controler */

	vIoWriteReg(CMD_PORT, REG_8, WRITE_AUX);
	wWaitResult = wait_to_send();

	if (wWaitResult != E__OK)
	{
		/* Keyboard controler did not read command byte. */
#ifdef DEBUG
		vConsoleWrite("Aux write command not read.\n");
#endif
		dTestStatus = E__KBC_CMD_EAUX;
	}

	/* Write command to PS/2 mouse */

	if (dTestStatus == E__OK)
	{
		vIoWriteReg(KBIN_BUFF, REG_8, mouse_command);
		wWaitResult = wait_to_send();

		if (wWaitResult != E__OK)
		{
			/* PS/2 mouse did not read command byte. */
#ifdef DEBUG
			vConsoleWrite("PS/2 not read command.\n");
#endif
			dTestStatus = E__NO_CMD_READ;
		}
	}

	/* Wait to receive ACK from PS/2 mouse */

	if (dTestStatus == E__OK)
	{
		if (read_ps2mouse (&bOutBuffer) == E__OK)
		{
			if (bOutBuffer != PS2RET_ACK)
			{
#ifdef DEBUG
				sprintf (buffer, "Received byte from PS/2 mouse is not ACK. Received %x\n", bOutBuffer);
				vConsoleWrite (buffer);
#endif
				dTestStatus = E__FALSE_PS2_ACK;
			}
		}
		else
		{
#ifdef DEBUG
			vConsoleWrite ("Not received ACK from PS/2 mouse.\n");
#endif
			dTestStatus = E__NO_PS2_ACK;
		}
	}

	return dTestStatus;
}


/******************************************************************************
 *
 * identify_ps2mouse - identify PS/2 mouse
 *
 * Send to PS/2 mouse command to identify. If mouse identify is not 00h then error
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */
 
static UINT16 identify_ps2mouse 
(
	void
)
{
	UINT32 dTestStatus;
	UINT8 bOutBuffer;

	dTestStatus = E__OK;
  
	/* Test Identify Device from PS/2 mouse */
	
	if (dTestStatus == E__OK)
		dTestStatus = flush_outbuffer ();
		
	if (dTestStatus == E__OK)
		dTestStatus = write_ps2mouse (PS2_ID);	/* send command for identify mouse */

	if (dTestStatus == E__OK) 
	{
		dTestStatus = read_ps2mouse (&bOutBuffer);	/* read answer from PS/2 mouse */
		if (dTestStatus == E__OK) 
		{
			if (bOutBuffer != PS2RET_MOUSEID)
			{
#ifdef DEBUG
				sprintf(buffer, "Wrong mouse identify. ID %x\n", bOutBuffer);
				vConsoleWrite(buffer);
#endif
				dTestStatus = E__MOUSE_FALSE_ID;
			}
		}
		else
		{
			dTestStatus = E__MOUSE_NOID;
		}
	}
  
	return dTestStatus;
}


/******************************************************************************
 *
 * echo_ps2mouse - test echo of PS/2 mouse
 *
 * Put PS/2 mouse into wrap (echo) mode and test answers.
 * After test send no wrap command.
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */
 
static UINT16 echo_ps2mouse 
(
	void
)
{
	UINT32 dTestStatus;
	UINT16 wWaitResult;
	UINT8 bOutBuffer;

	dTestStatus = E__OK;
  
	if (dTestStatus == E__OK)
		dTestStatus = flush_outbuffer ();
		
	/* Put PS/2 mouse into wrap mode */

	if (dTestStatus == E__OK)
		dTestStatus = write_ps2mouse (PS2_SET_WRAP);

	/* Test echo */

	if (dTestStatus == E__OK)
	{
		vIoWriteReg(CMD_PORT, REG_8, WRITE_AUX);
		wWaitResult = wait_to_send();

		if (wWaitResult != E__OK)
		{
			/* Keyboard controller did not read command byte. */
#ifdef DEBUG
			vConsoleWrite("Aux write command not read.\n");
#endif
			dTestStatus = E__KBC_CMD_EAUX;
		}
	}

	/* Write some byte to PS/2 mouse */

	if (dTestStatus == E__OK)
	{
		vIoWriteReg (KBIN_BUFF, REG_8, PS2_SET_SCALING);	/* Send some byte */
		wWaitResult = wait_to_send();

		if (wWaitResult != E__OK)
		{
			/* PS/2 mouse did not read command byte. */
			vConsoleWrite("PS/2 not read command.\n");
			dTestStatus = E__NO_CMD_READ;
		}
	}
	
	/* Check echo from PS/2 mouse */

	if (dTestStatus == E__OK)
	{
		dTestStatus = read_ps2mouse (&bOutBuffer);	/* read answer from PS/2 mouse */
		if (dTestStatus == E__OK) 
		{
			if (bOutBuffer != PS2_SET_SCALING)
			{
				dTestStatus = ERR_FALSE_PS2_ECHO;
			}
		}
		else
			dTestStatus = ERR_NO_PS2_ECHO;		/* nothing received from PS/2 mouse */
	}

	/* Back mouse from wrap to previous mode */
	
	write_ps2mouse (PS2_RESET_WRAP);
	
	return dTestStatus;
}


/******************************************************************************
 *
 * test_ps2mouse - test PS/2 mouse
 *
 * Reset mouse and send command indentify unit to PS/2 mouse and check result.
 * Check echo (wrap mode) from PS/2 mouse.
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */
 
static UINT16 test_ps2mouse 
(
	void
)
{
	UINT32 dTestStatus;
	UINT16 i;

	dTestStatus = !E__OK;
  
	for (i = 0; (i < 3) && (dTestStatus != E__OK); i++)	/* try resetting mouse max trials 3 times */
	{
#ifdef DEBUG
		sprintf (buffer, "Reset %d\n", i + 1);
		vConsoleWrite (buffer);
#endif
		dTestStatus = reset_ps2mouse ();
	}

	if (dTestStatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("Did not find mouse\n");
#endif
	}
		
	vDelay (10);

	/* Test Identify Device from PS/2 mouse */
	
	if (dTestStatus == E__OK)
	{
		dTestStatus = identify_ps2mouse ();
#ifdef DEBUG
			vConsoleWrite ("Identified mouse\n");
#endif
	}

	/* Test echo from PS/2 mouse */
	
	if (dTestStatus == E__OK)
		dTestStatus = echo_ps2mouse ();
	
	return dTestStatus;
}


/*****************************************************************************
 * dMousetest: test function to test PS/2 Mouse
 *							
 *
 * RETURNS: E__OK or error code
 */

static UINT32 dKCAuxtest
(
	void
)
{
	UINT32 dtest_status;

	/* Initialise and open AUX device on keyb controler */

	dtest_status = aux_enable();			/* Enable aux port */

	/* Test aux port for PS/2 mouse */

	if (dtest_status == E__OK)
		dtest_status = aux_echo();		/* Send echo to aux port */
	
	if (dtest_status == E__OK)
		dtest_status = test_auxbus();	/* Test aux bus */

	return dtest_status;
}
	

/*****************************************************************************
 * dMousetest: test function to test PS/2 Mouse
 *							
 *
 * RETURNS: E__OK or error code
 */

static UINT32 dMousetest
(
	void
)
{
	UINT32 dtest_status;

	/* Test PS/2 mouse */
	dtest_status = test_ps2mouse();

	return dtest_status;
}


/******************************************************************************
 *
 * packet_ps2mouse - receives data packet from PS/2 mouse
 * 
 * Receives 3 bytes with data (buttons status, X and Y offset)
 *
 * RETURNS: E__OK if it is OK, or error code
 *			sDataPacket - data packet from PS/2 mouse
 *
 */
static UINT32 packet_ps2mouse
(
	PS2MOUSE_PACKET *sDataPacket	/* where to put data packet from PS/2 mouse */
)
{
	UINT32 dTestStatus;
	UINT8 bOutBuffer;
	UINT16 i;		/* for loop counter */

	dTestStatus = E__OK;
  
	/* Read data packet from PS/2 mouse */

	for (i = 0; i < PACKET_LEN; i++)
	{
		if (dTestStatus == E__OK) 
		{
			dTestStatus = read_ps2mouse(&bOutBuffer);
			if (dTestStatus == E__OK) 
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
	
	if (dTestStatus != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("PS/2 data packet not received\n");
#endif
		dTestStatus = E__NO_PS2_PACKET;
	}
  
	return dTestStatus;
}


/******************************************************************************
 *
 * dMouseActtest - test actions of PS/2 mouse
 * 
 * Check PS/2 mouse actions separately (left and right buttons, and move in each directions),
 *
 * RETURNS: E__OK if it is OK, or error code
 *
 */
 
static UINT32 dMouseActtest
(
	void
)
{
	UINT32 dTestStatus;
	UINT16 timeout;
	UINT8 bDone;
	PS2MOUSE_PACKET sMousePacket;

	dTestStatus = E__OK;
	
	/* Init PS/2 mouse to standard mode */

	dTestStatus = write_ps2mouse (PS2_SET_STANDARD);
	
	/* Set PS/2 mouse to remote mode */

	if (dTestStatus == E__OK)
		dTestStatus = write_ps2mouse (PS2_SET_REMOTE);

	/* Test PS/2 LEFT button */

	if (dTestStatus == E__OK)
	{
		vConsoleWrite ("Press LEFT button\n");
		bDone = 0;
		timeout = 0;
		
		while (timeout < TIMEOUT_MOUSE_ACTION  &&  dTestStatus == E__OK  &&  !bDone)
		{
			if (dTestStatus == E__OK) 
				dTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (dTestStatus == E__OK) 
				dTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bStatus & STATUS_LEFT)
				bDone = 1;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			dTestStatus = E__TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 RIGHT button */

	if (dTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("Press RIGHT button\n");
		bDone = 0;
		timeout = 0;
		
		while (timeout < TIMEOUT_MOUSE_ACTION  &&  dTestStatus == E__OK  &&  !bDone)
		{
			if (dTestStatus == E__OK) 
				dTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (dTestStatus == E__OK) 
				dTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bStatus & STATUS_RIGHT)
				bDone = 1;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			dTestStatus = E__TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 move UP */

	if (dTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("Move UP\n");
		bDone = 0;
		timeout = 0;
		
		while (timeout < TIMEOUT_MOUSE_ACTION  &&  dTestStatus == E__OK  &&  !bDone)
		{
			if (dTestStatus == E__OK) 
				dTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (dTestStatus == E__OK) 
				dTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bYData != 0  &&  !(sMousePacket.bStatus & STATUS_YNG))
				bDone = 1;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			dTestStatus = E__TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 move DOWN */

	if (dTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("Move DOWN\n");
		bDone = 0;
		timeout = 0;
		
		while (timeout < TIMEOUT_MOUSE_ACTION  &&  dTestStatus == E__OK  &&  !bDone)
		{
			if (dTestStatus == E__OK) 
				dTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (dTestStatus == E__OK) 
				dTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bYData != 0  &&  (sMousePacket.bStatus & STATUS_YNG))
				bDone = 1;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			dTestStatus = E__TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 move LEFT */

	if (dTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("Move LEFT\n");
		bDone = 0;
		timeout = 0;
		
		while (timeout < TIMEOUT_MOUSE_ACTION  &&  dTestStatus == E__OK  &&  !bDone)
		{
			if (dTestStatus == E__OK) 
				dTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (dTestStatus == E__OK) 
				dTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bXData != 0  &&  (sMousePacket.bStatus & STATUS_XNG))
				bDone = 1;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			dTestStatus = E__TIMEOUT_ACTION;
		}
	}

	/* Test PS/2 move RIGHT */

	if (dTestStatus == E__OK)
	{
		vDelay (DELAY_BETWEEN_ACTION);
		vConsoleWrite ("Move RIGHT\n");
		bDone = 0;
		timeout = 0;
		
		while (timeout < TIMEOUT_MOUSE_ACTION  &&  dTestStatus == E__OK  &&  !bDone)
		{
			if (dTestStatus == E__OK) 
				dTestStatus = write_ps2mouse (PS2_READ_DATA);	/* init sending packet from mouse */
			if (dTestStatus == E__OK) 
				dTestStatus = packet_ps2mouse (&sMousePacket);	/* read mouse packet */

			vDelay (DELAY_SAMPLE);	/* wait for next sample */
			timeout++;
			if (sMousePacket.bXData != 0  &&  !(sMousePacket.bStatus & STATUS_XNG))
				bDone = 1;
		}

		if (timeout >= TIMEOUT_MOUSE_ACTION)
		{
			dTestStatus = E__TIMEOUT_ACTION;
		}
	}

	return dTestStatus;
}


/*****************************************************************************
 * KbCInterconnect: test function to check Keyboard Controller 
 *							interconnect
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (KbCInterconnect, "Keyboard Controller Access Test")
{
	UINT32 dtest_status;

	
	dtest_status = dKbCTest ();
	
	return (dtest_status);
} /* KbCInterconnect */


/*****************************************************************************
 * KbCSelfTest: test function for Keyboard Controller's 
 *							self test
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (KbCSelfTest, "Keyboard Controller Self Test")
{
	UINT32 dtest_status;

	
	dtest_status = dKbselftest();
	
	return (dtest_status);
} /* KbCSelfTest */


/*****************************************************************************
 * KbCInterrupt: test function for Keyboard Controller interrupt 
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (KbCInterrupt, "Keyboard Controller Int. Test")
{
	UINT32 dtest_status;

	
	dtest_status = dKbCInttest();
	
	return (dtest_status);
} /* KbCInterrupt */


/*****************************************************************************
 * KbdInterconnect: test function for Keyboard's interconnect 
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (KbdInterconnect, "Keyboard Device Interconnect Test")
{
	UINT32 dtest_status;

	
	dtest_status = dKbdevicetest();
	
	return (dtest_status);
} /* KbdInterconnect */


/*****************************************************************************
 * KbCAuxTest: test function for Keyboard's interconnect 
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (KbCAuxTest, "Keyboard Controllers Aux. Test")
{
	UINT32 dtest_status;

	
	dtest_status = dKCAuxtest();
	
	return (dtest_status);
} /* KbCAuxTest */


/*****************************************************************************
 * MouseInterconnect: test function for PS/2 Mouse interconnect 
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (MouseInterconnect, "PS/2 Mouse Interconnect Test")
{
	UINT32 dtest_status;

	
	dtest_status = dMousetest();
	
	return (dtest_status);
} /* MouseInterconnect */


/*****************************************************************************
 * MouseActionTest: test function for PS/2 Mouse interconnect 
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (MouseActionTest, "PS/2 Mouse Action Test")
{
	UINT32 dtest_status;

	
	dtest_status = dMouseActtest();
	
	return (dtest_status);
} /* MouseActionTest */


