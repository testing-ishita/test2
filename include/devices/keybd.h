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

/* keybd.h - definitions for keybd/ mouse devices
 */
/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/keybd.h,v 1.1 2013-09-04 07:40:27 chippisley Exp $
*
* $Log: keybd.h,v $
* Revision 1.1  2013-09-04 07:40:27  chippisley
* Import files into new source repository.
*
* Revision 1.1  2009/05/15 11:14:53  jthiru
* Initial checkin for Keyboard & Mouse Controller tests
*
*
*
*/

/* defines */
#define TIMEOUT_LIMIT           25
#define KB_CHECK                1
#define NO_CHECK                0
#define CMD_PORT                0x64  /* Write only */
#define STATUS_PORT             0x64  /* Read only */
#define KBOUT_BUFF              0x60  /* Read only */
#define KBIN_BUFF               0x60  /* Write only */
#define DISABLE_AUX				0xA7	/* disable aux port on KBC */
#define ENABLE_AUX				0xA8	/* enable aux port on KBC */
#define AUXIF_TEST				0xA9	/* test aux interface */
#define ECHO_AUX				0xD3	/* next byte put to out buffer and set AUXB */
#define WRITE_AUX				0xD4	/* next byte send to aux port */
#define KEYBD_ECHO              0x0EE
#define CMD_BYTE                0x60
#define KEYBD_ENABLE            0x0AE
#define SELF_TEST               0x0AA
#define IF_TEST                 0x0AB
#define READ_OUTPUT             0x0D0
#define KBINTS                  0x01
#define NO_KBINTS               0x00

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
#define DELAY_SAMPLE			100		/* delay between mouse reading in ms */
#define TIMEOUT_MOUSE_MOVE		80		/* how many DELAY_SAMPLE delay */
#define TIMEOUT_MOUSE_ACTION	50		/* timeout for one mouse action in DELAY_SAMPLE */
#define TIMEOUT_PS2_RESET		500		/* timeout for second byte (AA) after mouse reset */
#define DELAY_BETWEEN_ACTION	250		/* delay between test mouse action */

/* PS2 mouse packet */
typedef struct			
{
	UINT8 bStatus;		/* status byte */
	UINT8 bXData;		/* X-data: motion from last packet */
	UINT8 bYData;		/* Y-data: motion from last packet */
} PS2MOUSE_PACKET;

#define E__KBC_RW				E__BIT + 0x0
#define E__KBD_RW				E__BIT + 0x1
#define E__KBC_TIMEOUT			E__BIT + 0x2
#define E__KBC_SELFTEST 		E__BIT + 0x3
#define E__KBC_NO_AUX_ENABLE	E__BIT + 0x4
#define E__KBC_ERR_FLUSH		E__BIT + 0x5
#define E__KBC_CMD_EAUX			E__BIT + 0x6
#define E__KBC_CMD_RD			E__BIT + 0x7
#define E__KBC_FALSE_AUX_ECHO	E__BIT + 0x8
#define E__KBC_NO_AUX_ECHO		E__BIT + 0x9
#define E__KBC_NO_PS2_DATA		E__BIT + 0xA
#define E__KBC_IF_RD			E__BIT + 0xB
#define E__AUX_IF_TIMEOUT		E__BIT + 0xC
#define E__AUX_DATA_ERR			E__BIT + 0xD
#define E__AUX_WR_CMD			E__BIT + 0xE
#define E__PS2_RST				E__BIT + 0xF
#define E__NO_MOUSE				E__BIT + 0x10
#define E__MOUSE_ERR			E__BIT + 0x11
#define E__MOUSE_FALSE_ID		E__BIT + 0x12
#define E__MOUSE_NOID			E__BIT + 0x13
#define E__NO_CMD_READ			E__BIT + 0x14
#define ERR_FALSE_PS2_ECHO		E__BIT + 0x15
#define ERR_NO_PS2_ECHO			E__BIT + 0x16
#define E__FALSE_PS2_ACK		E__BIT + 0x17
#define E__NO_PS2_ACK			E__BIT + 0x18
#define E__TIMEOUT_ACTION		E__BIT + 0x19
#define E__NO_PS2_PACKET		E__BIT + 0x1A
