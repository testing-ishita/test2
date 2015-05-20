/*******************************************************************************
*
* Filename:	 packet.c
*
* Description:	Header file for core packet functions.
*
* $Revision: 1.3 $
*
* $Date: 2013-11-25 10:57:31 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/include/cute/packet.h,v $
*
* Copyright 2008-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

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
 * packet.h
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/cute/packet.h,v 1.3 2013-11-25 10:57:31 mgostling Exp $
 * $Log: packet.h,v $
 * Revision 1.3  2013-11-25 10:57:31  mgostling
 * Added missing CVS headers.
 *
 */

#ifndef PACKET_H_
#define PACKET_H_

#define MAX_DATA			2048
#define MAX_SLAVES			32
#define MAX_SLOTS			32
#define MAX_PARAMS			32
#define BOOL				UINT8
#define IC_GRANULARITY		8192

#define Q_TX				1
#define Q_RX				2

#define FLAG_PKT_EID		0x0001
#define FLAG_ACK_PKT		0x0002
#define FLAG_ACK_NAK		0x0004
#define FLAG_F3     		0x0008

#define FLAG_PKT_PRI		0x0010
#define FLAG_SYS_PKT		0x0020
#define FLAG_PKT_2P0		0x0040
#define FLAG_HAS_DTA		0x0080

#define FLAG_WHO_AMI		0x0100
#define FLAG_CLK_SET		0x0200
#define FLAG_TST_ENT		0x0400
#define FLAG_TST_XIT		0x0800

#define FLAG_PCR_RUT		0x1000
#define FLAG_PCR_RST		0x2000
#define FLAG_F14    		0x4000
#define FLAG_F15    		0x8000

#define TIMEOUT_PERIOD		200
#define PKT_TIMEOUT_NONE	0xffffffff
#define PKT_TIMEOUT_NORM 	TIMEOUT_PERIOD

#define	SOAK_ID				0x8000

#define NO_ACK_WAIT			0
#define	ACK_WAIT			1

#define NET_SLAVE			1

#define CPCI_MASTER			0
#define CPCI_SLAVE			1

#define ID__MONARCH			0
#define ID__SLAVE			1


#define CPCI_SUBSYSID		0xccc1		/* bridge subsystem id		*/

#define VPX_MASTER			0
#define VPX_SLAVE			1
#define VPX_SUBSYSID		0xccc1		/* bridge subsystem id		*/

#define E_NOT_PENDING		0x8007
#define E_LENGTH_OVERRUN   	0x800b

#define COOP_LM_WR			0
#define COOP_LM_DA			1
#define COOP_LM_WR_ID		23
#define COOP_LM_DA_ID		25
#define ALT_COOP_LM_WR_ID	300
#define ALT_COOP_LM_DA_ID	301
#define ALT_COOP_LM_WR_STR	"Coop LM Read/Write Test"
#define ALT_COOP_LM_DA_STR	"Coop LM Dual Address Test"

#define SysCon_ID			0x1F

/*------------------------------------- */
enum PKTERRs
{
		E__READ_OVERFLOW=1,
		E__TIMEOUT,
		E__CHECKSUM,
		E__NO_RESPONSE,
		E__NO_PACKETS,
		E__NOT_FOUND,
		E__TOO_SHORT
};


/*------------------------------------- */
enum	PKTIDs
		{
		id_AA_PCR=1,		/* 1 PCR Address Allocation Packet */
		id_AA_SCN,			/* 2 Force PCR to rescan I2C bus */
		id_AA_SL,			/* 3 Slave Address Allocation Packet */
		id_AA_PORT,			/* 4 Soak Master Address Allocation Packet */

		id_RBL,				/* 5 Request BIST List */
		id_SBL,				/* 6 Send BIST List */
		id_BGO, 			/* 7 Start BIST */
		id_GCT, 			/* 8 Get Command Tail */
		id_SCT,				/* 9 Send Command Tail */

		id_TEMP,			/* 10 Oven Temperature */

		id_REV,				/* 11 Request Soak Master Revision Level */
		id_VER,				/* 12 State Soak Master Revision Level */

		id_GEE, 			/* 13 Get Extended Error Code */
		id_SEE, 			/* 14 Send Extended Error Code */

		id_D_RDW,			/* 15 Request Debug Window */
		id_D_WAK,			/* 16 Debug Window Opened */
		id_D_WNK,			/* 17 Debug Windows declined */
		id_D_CDW,			/* 18 Close Debug window */
		id_D_CLS,			/* 19 Clear Screen */
		id_D_XY,			/* 20 Move cursor */
		id_D_CR,			/* 21 Move cursor to the beginning of the next line */
		id_D_STR,			/* 22 Display string */
		id_D_NUM,			/* 23 Display number */
		id_D_SCL,			/* 24 Scroll an area of the screen */
		id_D_DMP,			/* 25 Dump debug window */
		id_D_LSD,			/* 26 Load Screen Dump */

		id_V_PUT,			/* 27 Store data in Soak Master memory */
		id_V_GET,			/* 28 Retrieve stored data */
		id_V_DTA,			/* 29 Requested data packet */
		id_WRT				/* 30 Write text packet */

		};


#define	MaxTail		100		/* Longest command tail	*/

#pragma pack(1)

struct	sHeader
{
	UINT16	wLength;
	UINT16	wSourceID;
  	UINT16	wDestID;
  	UINT16	wFlags;
  	UINT16	wChecksum;
	UINT32	dType;
};

struct sPkt_BGO
{
  	UINT16	wLength;
	UINT16	wSourceID;
  	UINT16	wDestID;
  	UINT16	wFlags;
  	UINT16	wChecksum;
	UINT16	wTestId;
	char	achTail[MaxTail];
} ;

typedef union
{
	struct	sHeader		hdr;
	UINT8				bData[MAX_DATA];
	UINT16				wData[MAX_DATA/2];
	struct	sPkt_BGO	sBgo;
} uPacket;

typedef struct
{
	BOOL	bFitted;
	UINT16	wAddr;
	UINT32	dBar;
} sSlave;

typedef struct
{
	UINT32	dBar;
	UINT32	dBarM;
} sSlaveMemBar;

	typedef struct
	{
		UINT32 testNum;	// 23 or 25
		UINT32 coopNum;	// 300 or 301
		UINT32 offset;
		UINT32 length;
		UINT32 status;
		UINT32 retry;
	}sm3vme_COOP;

	typedef struct
	{
		BOOL		bFitted;
		UINT16		wAddr;
		UINT32		dBar;
		UINT8   	msgFlag;
		UINT8   	errMsg[64];
		UINT8   	pramFlag;
		UINT8   	params[128];
		sm3vme_COOP	coop[2];
	} sm3vmeSlave;

	typedef struct
	{
		UINT8   	msgFlag;
		UINT8   	errMsg[64];
		UINT8   	pramFlag;
		UINT8   	params[128];
		sm3vme_COOP	syscon_coop[2];
	} sm3vmesyscon;

	typedef struct
	{
		UINT32 length;
		UINT32 count;
		UINT32 status;
		UINT32 type[MAX_PARAMS];
		UINT32 data[MAX_PARAMS];
	}saved_params;

	typedef struct
	{
		UINT8  parcel_type;
		UINT16 length;
		UINT16 status;
		UINT8  data[256];
	} struc_get_params_res;

#pragma pack()



/* function prototypes */

UINT16   wPacketSend (UINT8 bSlot, uPacket *psPkt, UINT8 bAckWait, UINT32 dTimeout);
void     vSendAck (UINT8 bSlot, UINT16 wSourceID, UINT16 wDestID, BOOL bNak);
UINT16   wPacketSend (UINT8 bSlot, uPacket *psPkt, UINT8 bAckWait, UINT32 dTimeout);
UINT16   wPacketReceive(UINT8 bSlot, uPacket *psPkt, BOOL bAckSend);
void     vFreeAllPackets ( void );
UINT16   wPacketAddQueue (UINT8 bQueue, uPacket* psPkt);
uPacket* psPacketGetQueue (UINT8 bQueue);
UINT16   wPacketSendQueue (UINT8 bQueue, UINT8 bSlot);

void   configureCoopWindows (sm3vmeSlave *asSlots, sm3vme_COOP *syscon_coop);

#endif /* PACKET_H_ */
