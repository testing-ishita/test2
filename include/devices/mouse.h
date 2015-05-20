
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
 * mouse.h
 *
 *  Created on: 4 Jun 2010
 *      Author: engineer
 */

#ifndef MOUSE_H_
#define MOUSE_H_

#define TRUE					1
#define FALSE					0

#define TIMEOUT_LIMIT			25

#define DELAY_SAMPLE			100		/* delay between mouse reading in ms */
#define TIMEOUT_MOUSE_MOVE		80		/* how many DELAY_SAMPLE delay */
#define TIMEOUT_MOUSE_ACTION	50		/* timeout for one mouse action in DELAY_SAMPLE */
#define TIMEOUT_PS2_RESET		500		/* timeout for second byte (AA) after mouse reset */
#define DELAY_BETWEEN_ACTION	250		/* delay between test mouse action */

/* Keyboard controllers ports */

#define CMD_PORT				0x64	/* Write only */
#define STATUS_PORT				0x64	/* Read only */
#define KBOUT_BUFF				0x60	/* Read only */
#define KBIN_BUFF				0x60	/* Write only */

#define DISABLE_AUX				0xA7	/* disable aux port on KBC */
#define ENABLE_AUX				0xA8	/* enable aux port on KBC */
#define AUXIF_TEST				0xA9	/* test aux interface */
#define ECHO_AUX				0xD3	/* next byte put to out buffer and set AUXB */
#define WRITE_AUX				0xD4	/* next byte send to aux port */

/* Definitions of PS/2 mouse commands. In comment are return codes from mouse */

#define PS2_RESET_SCALE			0xE6	/* resets X-Y scaling factor to 1:1. Return: 0xFA */
#define PS2_SET_SCALING			0xE7	/* sets X-Y scaling factor to 2:1. Return: 0xFA */
#define PS2_SET_RESOLUTION		0xE8	/* sets mouse resolution. Return: 0xFA */
#define PS2_STATUS				0xE9	/* read PS/2 mouse status. Return: 0xFA, xx, xx, xx */
#define PS2_SET_STREAM			0xEA	/* sets stream mode. Return: 0xFA */
#define PS2_READ_DATA			0xEB	/* transfer data packet from mouse to controller. Return: 0xFA, xx, xx, xx */
#define PS2_RESET_WRAP			0xEC	/* resets wrap mode. Return: 0xFA */
#define PS2_SET_WRAP			0xEE	/* sets wrap mode. Return: 0xFA */
#define PS2_SET_REMOTE			0xF0	/* sets remote mode. Return: 0xFA */
#define PS2_ID					0xF2	/* identify unit. Return: 0xFA, 0x00 */
#define PS2_ENABLE				0xF4	/* enables data stransfers in stream mode. Return: 0xFA */
#define PS2_DISABLE				0xF5	/* disables data stransfers in stream mode. Return: 0xFA */
#define PS2_SET_STANDARD		0xF6	/* initializes mouse with standard values. Return: 0xFA */
#define PS2_RESEND				0xFE	/* retransmit the last data packet. Return: xx, (xx, xx) */
#define PS2_RESET				0xFF	/* executes internal mouse test.
										   Return: 0xFA, after 500ms 0xAA and 0x00 (not need to read)
										   If mouse returns 0xFC instead 0xAA, than it is self-test error
										 */

/* Definitions of PS/2 return codes */

#define PS2RET_MOUSEID			0x00	/* mouse ID */
#define PS2RET_ACK				0xFA	/* acknowledge */
#define PS2RET_ERROR			0xFC	/* mouse error */
#define PS2RET_RESEND			0xFE	/* resend request */
#define PS2RET_RESET			0xAA	/* Second byte 500ms after ACK after PS/2 mouse RESET */

/* Definitions positions in PS2 mouse packet */

#define PACKET_STATUS			0		/* status byte (buttons...) */
#define PACKET_X				1		/* X-data: motion from last packet */
#define PACKET_Y				2		/* Y-data: motion from last packet */

#define PACKET_LEN				3		/* length of packet */

/* Definitions of bits in status byte of mouse packet */

#define STATUS_LEFT			0x01	/* left button */
#define STATUS_RIGHT		0x02	/* right button */
#define STATUS_MIDDLE		0x04	/* middle button (if exist) */
#define STATUS_BUTTONS		STATUS_LEFT + STATUS_RIGHT + STATUS_MIDDLE	/* all buttons */
#define STATUS_XNG			0x10	/* X-value negative */
#define STATUS_YNG			0x20	/* Y-value negative */

/* Errors codes */

#define ERR_GENERAL			E__BIT			/* General error */
#define ERR_FLUSH			E__BIT + 0x01	/* Timeout trying to flush kbc buffer */
#define ERR_NO_AUX_ENABLE	E__BIT + 0x02	/* KBC did not read auxiliary enable command */
#define ERR_NO_READ_CMD		E__BIT + 0x03	/* KBC did not read mode command */
#define ERR_NO_READ_DATA	E__BIT + 0x04	/* KBC did not read mode command data */
#define ERR_NO_AUX_ECHO		E__BIT + 0x05	/* No echo from auxiliary port */
#define ERR_FALSE_AUX_ECHO	E__BIT + 0x06	/* Wrong echo from auxiliary port */
#define ERR_NO_MOUSE		E__BIT + 0x07	/* PS/2 mouse did not find */
#define ERR_MOUSE			E__BIT + 0x08	/* PS/2 mouse report error after reset */
#define ERR_NO_ID			E__BIT + 0x09	/* No identify from PS/2 mouse */
#define ERR_FALSE_ID		E__BIT + 0x0A	/* Wrong identify from PS/2 mouse */
#define ERR_NO_PS2_ECHO		E__BIT + 0x0B	/* No echo from PS/2 mouse */
#define ERR_FALSE_PS2_ECHO	E__BIT + 0x0C	/* Wrong echo from PS/2 mouse */
#define ERR_NO_PS2_ACK		E__BIT + 0x0D	/* No received ACK from PS/2 mouse */
#define ERR_FALSE_PS2_ACK	E__BIT + 0x0E	/* Received byte from PS/2 mouse is not ACK */
#define ERR_NO_PS2_DATA		E__BIT + 0x0F	/* No data byte from PS/2 mouse */
#define ERR_NO_PS2_PACKET	E__BIT + 0x10	/* No data packet from PS/2 mouse */
#define ERR_TIMEOUT_ACTION	E__BIT + 0x11	/* Timeout waiting for test mouse buttons and moving */
#define ERR_IF_NO_READ		E__BIT + 0x12	/* Auxiliary interface test command not read */
#define ERR_IF_TIMEOUT		E__BIT + 0x13	/* Timeout waiting for auxiliary interface test result */
#define ERR_IF_BASE			E__BIT + 0x13	/* Add with error code from KBC for AUX IF test */


/* PS2 mouse packet */

typedef struct
{
	UINT8 bStatus;		/* status byte */
	UINT8 bXData;		/* X-data: motion from last packet */
	UINT8 bYData;		/* Y-data: motion from last packet */
} PS2MOUSE_PACKET;



#endif /* MOUSE_H_ */
