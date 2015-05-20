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

/* ipmi.h - definitions for ipmi
 */

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/ipmi.h,v 1.8 2014-11-04 14:57:41 mgostling Exp $
*
* $Log: ipmi.h,v $
* Revision 1.8  2014-11-04 14:57:41  mgostling
* Added CMD_GET_GEOGRAPHIC_ADDR_M3 for Cortex M3 IPMI controller
*
* Revision 1.7  2014-10-15 12:03:41  mgostling
* Added hotswap ejectotr handle sensor ID.
* Removed unused error codes.
*
* Revision 1.6  2014-10-09 14:36:26  mgostling
* Added sensor threshold bitmaps for SDR records.
*
* Revision 1.5  2014-09-19 12:57:51  mgostling
* Added function prototypes needed for ipmi_M3.c
*
* Revision 1.4  2014-07-24 14:08:34  mgostling
* Added function prototype for bGetVpxIpmbAddrs()
*
* Revision 1.3  2014-01-29 13:23:52  hchalla
* Added new functions for AM 94x to get Set port state status and IPMB address for slot calculations.
*
* Revision 1.2  2013-09-27 12:21:09  chippisley
* Added NFC_OEM_REQUEST.
*
* Revision 1.1  2013/09/04 07:40:27  chippisley
* Import files into new source repository.
*
* Revision 1.2  2009/06/09 15:01:44  jthiru
* Added Definitions for Watchdog test case
*
* Revision 1.1  2009/05/15 11:14:29  jthiru
* Initial checkin for IPMI tests
*
*
*
*/


/* defines */

/* Addresses of SMIC registers */

#define SMIC_DATA			0x0CA9
#define SMIC_CONTROL		0x0CAA		/* Control & Status register */
#define SMIC_FLAGS			0x0CAB

#define bReadSmicFlags	(Inbyte (SMIC_FLAGS))
#define bReadSmicStatus	(Inbyte (SMIC_CONTROL))
#define bReadSmicData	(Inbyte (SMIC_DATA))

#define vWriteSmicControl(data)		Outbyte(SMIC_CONTROL, data)
#define vWriteSmicData(data)		Outbyte(SMIC_DATA, data)
#define vWriteSmicFlags(data)		Outbyte(SMIC_FLAGS, data)

#define TIMEOUT				100000	/* Timeout for Smic Delay */
#define SMS_LEN_BUFFER		70		/* Max length of SMS buffers */

#define NFC_PICMG_REQUEST           0x2C
#define NFC_PICMG_RESPONSE          0x2D

/* OEM Commands */
#define CMD_GET_GEOGRAPHIC_ADDR     0x23	// 8051
#define CMD_GET_GEOGRAPHIC_ADDR_M3  0x60	// Cortex M3
#define CMD_GET_HOTSWAP_HANDLE_POSN 0x24

/* PICMG Commands */
#define CMD_GET_PICMG_PROPERTIES    0x00
#define CMD_FRU_CONTROL             0x04
#define CMD_GET_FRU_LED_PROPERTIES  0x05
#define CMD_GET_LED_COLOUR_CAPABILITIES 0x06
#define CMD_SET_FRU_LED_STATE       0x07
#define CMD_GET_FRU_LED_STATE       0x08
/* Masks for SMIC Flags Registers Bits */

#define FLAGS_BUSY			0x01
#define FLAGS_TX_DATA_READY	0x40
#define FLAGS_RX_DATA_READY	0x80
#define FLAGS_SMI			0x10
#define FLAGS_EVT_ATN		0x08
#define FLAGS_SMS_ATN		0x04

/* SMS Transfer Stream Control Codes */

#define CC_SMS_GET_STATUS	0x40
#define CC_SMS_WR_START		0x41
#define CC_SMS_WR_NEXT		0x42
#define CC_SMS_WR_END		0x43
#define CC_SMS_RD_START		0x44
#define CC_SMS_RD_NEXT		0x45
#define CC_SMS_RD_END		0x46

	/* SMS Transfer Stream Status Codes */

#define SC_SMS_RDY			0xC0	
#define SC_SMS_WR_START		0xC1
#define SC_SMS_WR_NEXT		0xC2
#define SC_SMS_WR_END		0xC3
#define SC_SMS_RD_START		0xC4
#define SC_SMS_RD_NEXT		0xC5
#define SC_SMS_RD_END		0xC6

	/* Network Function Codes (higher 6 bits). Value without shift */

#define NFC_CHASSIS_REQUEST		0x00
#define NFC_CHASSIS_RESPONSE	0x01
#define NFC_SENSOR_EVENT		0x04
#define NFC_APP_REQUEST			0x06
#define NFC_APP_RESPONSE		0x07
#define NFC_STORAGE_REQUEST		0x0A
#define NFC_STORAGE_RESPONSE	0x0B
#define NFC_OEM_REQUEST			0x34	/* OEM Network Function */

#define LUN					0x00	/* (lower 2 bits) */

	/* Status & Control Register 3 (for SMIC interrupts) */

#define SMIC_INT_REG		0x215	/* Address of register */
#define SMIC_NOT_BUSY_ENABLE 0x01	/* SMIC Not Busy Interrupt Enable bit */
#define SMS_ATN_ENABLE		0x02	/* SMS_ATN Interrupt Enable bit */
#define SMIC_GPI_ENABLE		0x04	/* GPI Interrupt Enable bit */
#define SMIC_NOT_BUSY_FLAG	0x10	/* SMIC Not Busy Interrupt Enable flag */
#define SMS_ATN_FLAG		0x20	/* SMS_ATN Interrupt Enable flag */
#define SMIC_GPI_FLAG		0x40	/* GPI Interrupt Enable flag */

/* Commands */

#define CMD_WATCHDOG_RESET	0x22	/* Reset Watchdog Timer */
#define CMD_WATCHDOG_SET	0x24	/* Set Watchdog Timer */
#define CMD_WATCHDOG_GET	0x25	/* Get Watchdog Timer */
#define CMD_GET_DEVICE_ID	0x01	/* Get device ID */
#define CMD_GET_SEL_INFO	0x40	/* Get SEL Info */
#define CMD_GET_SEL_ALLOC	0x41	/* Get SEL Allocation Info */
#define CMD_RESERVE_SEL		0x42	/* Reserve SEL */
#define CMD_GET_SEL_ENTRY	0x43	/* Get SEL Entry */
#define CMD_ADD_SEL_ENTRY	0x44	/* Add SEL ENTRY */
#define CMD_PART_ADD_SEL	0x45	/* Partial Add SEL Entry */
#define CMD_DEL_SEL_ENTRY	0x46	/* Delete SEL Entry */
#define CMD_CLEAR_SEL		0x47	/* Clear SEL */
#define CMD_GET_SEL_TIME	0x48	/* Get SEL Time */
#define CMD_SET_SEL_TIME	0x49	/* Set SEL Time */
#define CMD_GET_SELF_TEST   0x04
#define CMD_GET_SDR_ENTRY       0x23    /* Get SDR entry */
#define CMD_GET_SENS_RD         0x2D    /* Get sensor reading */
#define CMD_GET_SENS_THRESH     0x27    /* Get sensor threshold */

#define LUN_BMC					0x00	/* (lower 2 bits) */

#define MAX_LEN_BUFFER		40

#define INT_DELAY			500	/* add delay for interrupt */
#define NMI_VECTOR			0x02	/* NMI interrupt vector */

#define WD_NOT_LOG			0x80	/* don't log bit */
#define WD_TIMER_START		0x40	/* timer start bit */

#define WD_TIMER_MASK		0x07	/* mask for timer use bits */
#define WD_TIMER_FRB2		0x01	/* timer use BIOS FRB2 */
#define WD_TIMER_POST		0x02	/* timer use BIOS/POST */
#define WD_TIMER_OSLOAD		0x03	/* timer use OS Load */
#define WD_TIMER_SMS		0x04	/* timer use SMS/OS */
#define WD_TIMER_OEM		0x05	/* timer use OEM */

#define WD_PRE_TM_MASK		0x70	/* mask for pre-timeout interrupt */
#define WD_PRE_TM_NONE		0x00	/* pre-timeout interrupt: none */
#define WD_PRE_TM_SMI		0x10	/* pre-timeout interrupt: SMI */
#define WD_PRE_TM_NMI		0x20	/* pre-timeout interrupt: NMI */
#define WD_PRE_TM_MESSAGE	0x30	/* pre-timeout interrupt: Message */

#define WD_ACTION_MASK		0x07	/* mask for timeout action */
#define WD_ACTION_NONE		0x00	/* no timeout action */
#define WD_ACTION_HARD		0x01	/* timeout action: hard reset */
#define WD_ACTION_PDOWN		0x02	/* timeout action: power down */
#define WD_ACTION_PCYCLE	0x03	/* timeout action: power cycle */

#define WD_EXP_FLAG_OEM		0x20	/* timer use expiration flag for OEM */
#define WD_EXP_FLAG_SMS		0x10	/* timer use expiration flag for SMS */
#define WD_EXP_FLAG_OSLOAD	0x08	/* timer use expiration flag for OS Load */
#define WD_EXP_FLAG_POST	0x04	/* timer use expiration flag for BIOS/POST */
#define WD_EXP_FLAG_FRB2	0x02	/* timer use expiration flag for BIOS FRB2 */


#define	E__NO_IPMI			E__BIT + 0x0	/* IPMI interface is not found */
#define E__IPMI_COMM		E__BIT + 0x1	/* error in IPMI communication */
#define E__COMP_CODE		E__BIT + 0x2	/* wrong completion code */
#define E__RESPONSE			E__BIT + 0x3	/* wrong response (less bytes) */
#define E__SLAVE			E__BIT + 0x5	/* board is in satelite mode */ 
#define E__DATAMISMATCH 	E__BIT + 0x6
#define E__IPMI_NO_INT		E__BIT + 0x7
#define E__CMDNOTIMPL		E__BIT + 0x8
#define	E__IPMIRW			E__BIT + 0x9
#define	E__IPMIFATALERR		E__BIT + 0x10
#define	E__IPMISPECERR		E__BIT + 0x11
#define E__SDRFORMAT		E__BIT + 0x12	/* Next 20 error codes for SDR record number*/
#define	E__NO_SENSOR		E__BIT + 0x33	
#define E__VERIFY			E__BIT + 0x76
#define E__NO_INT			E__BIT + 0x77
#define E__TIMEOUT			E__BIT + 0x78

// assertion and deassertion threshold bitmaps

#define LOWER_NON_CRITICAL_GOING_LOW		0x0001
#define LOWER_NON_CRITICAL_GOING_HIGH		0x0002
#define LOWER_CRITICAL_GOING_LOW			0x0004
#define LOWER_CRITICAL_GOING_HIGH			0x0008
#define LOWER_NON_RECOVERABLE_GOING_LOW		0x0010
#define LOWER_NON_RECOVERABLE_GOING_HIGH	0x0020
#define UPPER_NON_CRITICAL_GOING_LOW		0x0040
#define UPPER_NON_CRITICAL_GOING_HIGH		0x0080
#define UPPER_CRITICAL_GOING_LOW			0x0100
#define UPPER_CRITICAL_GOING_HIGH			0x0200
#define UPPER_NON_RECOVERABLE_GOING_LOW		0x0400
#define UPPER_NON_RECOVERABLE_GOING_HIGH	0x0800
#define NON_CRITICAL_COMPARISON				0x1000
#define CRITICAL_COMPARISON					0x2000
#define NON_RECOVERABLE_COMPARISON			0x4000

// settable and readable threshold bitmaps

#define LOWER_NON_CRITICAL_READABLE			0x0001
#define LOWER_CRITICAL_READABLE				0x0002
#define LOWER_NON_RECOVERABLE_READABLE		0x0004
#define UPPER_NON_CRITICAL_READABLE			0x0008
#define UPPER_CRITICAL_READABLE				0x0010
#define UPPER_NON_RECOVERABLE_READABLE		0x0020
#define LOWER_NON_CRITICAL_SETTABLE			0x0100
#define LOWER_CRITICAL_SETTABLE				0x0200
#define LOWER_NON_RECOVERABLE_SETTABLE		0x0400
#define UPPER_NON_CRITICAL_SETTABLE			0x0800
#define UPPER_CRITICAL_SETTABLE				0x1000
#define UPPER_NON_RECOVERABLE_SETTABLE		0x2000

// AMC Hot Swap ejector handle sensor ID

#define AMC_MODULE_HOT_SWAP_ID					0xF2

/* Completion Codes */

#define COMP_OK				0
#define COMP_REQ_NOT_FOUND	0xCB	/* Requested info not found */

UINT8 bGetVpxIpmbAddrs (void);
UINT8 bGetAMCIpmbAddrs (void);
UINT8 bGetSPSStatus (UINT8 *bSrioSpeed);

// for ipmi_M3.c

UINT32 dIpmiSendRequest
(
	UINT8 *pbRequest,	/* array with requested message */
	UINT8 bReqLength,	/* length of requested message */
	UINT8 *pbResponse,	/* array with response message */
	UINT8 *pbRespLength	/* length of response message */
);
UINT32 dIpmiInit (void);
void vIpmiDeinit (void);
