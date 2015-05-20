/*******************************************************************************
*
* Filename:	 smClient.c
*
* Description:	Soak Master Client.
*
* $Revision: 1.6 $
*
* $Date: 2014-07-24 15:01:17 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/lwIP_Apps/apps/SmClient/smClient.c,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

/* Standard includes. */
#include "stdlib.h"
#include "errors.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <private/atomic.h>
#include <cute/packet.h>

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"

#include "ssmmp.h"

#include "config.h"
#include "../debug/dbgLog.h"

#include "../vgalib/vga.h"

#define SERVER_PORT_NUM			5001
#define MAX_REQ_MSG_SIZE		4096	/* must = SSMMP_PACKET_LEN or greater */

#define SMC_TASK_PRIORTY 		6
#define SMC_TASK_STACK_SIZE		configMINIMAL_STACK_SIZE

#define SMC_CONN_TYPE_UKN	0	/* Unknowm connection */
#define SMC_CONN_TYPE_LEG	1	/* Legacy Soak Master */
#define SMC_CONN_TYPE_USM	2	/* Universal Soak Master */

#define SMC_PKT_OK		0
#define SMC_PKT_IGNORE	1
#define SMC_PKT_ERROR	2

#undef INCLUDE_LWIP_SMC_DEBUG
#undef INCLUDE_LWIP_SMC_ERRS
#define INCLUDE_LWIP_SMC_DIAG

#if defined(INCLUDE_LWIP_SMC_DEBUG) || defined(INCLUDE_LWIP_SMC_ERRS) || defined(INCLUDE_LWIP_SMC_DIAG)
#warning "***** DEBUG ON *****"
#endif

#ifdef INCLUDE_LWIP_SMC_DEBUG /* use DBLOG or sysDebugPrintf */
#define CLIDBG(x)	do {DBLOG x;} while(0)
#define CLILOGBUF(x) do {dbLogBuffer x;} while(0)
#define INCLUDE_LWIP_SMC_ERRS	/* make sure errors are on as well */
#else
#define CLIDBG(x)
#define CLILOGBUF(x)
#endif

#ifdef INCLUDE_LWIP_SMC_ERRS /* use DBLOG or sysDebugPrintf */
#define CLIERR(x)	do {DBLOG x;} while(0)
#else
#define CLIERR(x)
#endif

#ifdef INCLUDE_LWIP_SMC_DIAG /* use DBLOG or sysDebugPrintf */
#define CLIDIAG(x)	do {sysDebugPrintf x;} while(0)
#else
#define CLIDIAG(x)
#endif

#ifdef INCLUDE_DEBUG_VGA
extern int startLine;
#endif

extern void vGetUSMIPAddrs(char *pBuff);

static char smIpAddr[20];			/* Soak Master IP address */
static BOOL smClientActive = FALSE;
static BOOL smConnectActive = FALSE;

#ifdef INCLUDE_DEBUG_VGA
static int currentLine = -1;
#endif

static xQueueHandle hSmcMbox = NULL;
static xQueueHandle hSthMbox = NULL;

static UINT8 *smcBuffer = NULL;
static int smcConnType = SMC_CONN_TYPE_UKN;

static UINT32 smcBoardAddr = 0x1f;	/* default to INTERCONNECT_LOCAL_SLOT */

/* Statistic Counters */
static UINT32 smcRxCount = 0;
static UINT32 smcTxCount = 0;
static UINT32 smcErrCount = 0;
static UINT32 smcMemErrCount = 0;
static UINT32 smcConnErrCount = 0;
static UINT32 smcConnRetryCount = 0;
static UINT32 smcPostCount = 0;
static UINT32 smcPostErrCount = 0;
static UINT32 smcFetchCount = 0;
static UINT32 smcFetchErrCount = 0;
static UINT32 sthPostCount = 0;
static UINT32 sthPostErrCount = 0;
static UINT32 sthFetchCount = 0;
static UINT32 sthFetchErrCount = 0;


#ifdef INCLUDE_LWIP_SMC_DEBUG
/*******************************************************************************
*
* dbLogBuffer
*
* Print given buffer on debug console.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void dbLogBuffer( UINT32 addr, UINT8 *buf, UINT32 size )
{
    UINT32 i;
    UINT32 n;
	char textBuf[81];
	int len;

    
    for (n = 0; n < size; n+=16)
    {
		len = 0;
		sprintf( &textBuf[len], "%08x ", addr );
		len += 9;
		addr += 16;
        
        /* Print data */
		for (i = 0; i < 16; i++)
		{
			if( (n + i) < size )
			{
				sprintf( &textBuf[len], "%02x ", buf[(n + i)] );
			}
			else
			{
				sprintf( &textBuf[len], "   " );
			}
			
			len += 3;
        }
        
        /* Print ASCII */
        for (i = 0; i < 16; i++)    
		{
			if ( (n + i) < size )
			{
				sprintf( &textBuf[len], "%c", ( (buf[n + i] > 33) && (buf[n + i] < 127) ) ? buf[n + i] : '.' );
			}
			else
			{
				sprintf( &textBuf[len], "." );
			}

			len++;
		}

		sprintf( &textBuf[len], "\n" );
		
		CLIDBG(("%s", textBuf));
    }    
}
#endif

/*******************************************************************************
*
* smcCreateMbox
*
* Creates communication mailbox.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
static int smcCreateMbox( xQueueHandle *hMbox, int size )
{
	*hMbox = xQueueCreate( size, sizeof( void * ) );

	if( *hMbox != NULL )
	{
		CLIDBG(("%s: - hMbox: 0x%x\n", __FUNCTION__, *hMbox));
		return E__OK;
	}
	else
	{
		CLIERR(("Error - failed to create mailbox\n"));
		return E__FAIL;
	}
}


/*******************************************************************************
*
* smcFreeMbox
*
* Free communication mailbox.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
static int smcFreeMbox( xQueueHandle *hMbox )
{
	void *pMsg;
	
	
	if ( *hMbox != NULL )
	{
		CLIDBG(("%s: - hMbox: 0x%x\n", __FUNCTION__, *hMbox));
		
		if ( uxQueueMessagesWaiting( *hMbox ) != 0 )
		{
			/* Flush the queue */
			while( xQueueReceive( *hMbox, (void **) &pMsg, 0 ) == pdPASS )
			{
				if (pMsg != NULL)
				{
					CLIDBG(("%s: - pMsg : 0x%x\n", __FUNCTION__, pMsg));
					
					mem_free( pMsg );
				}
			}
		}
		
		vQueueDelete( *hMbox );
		*hMbox = NULL;
				
		return E__OK;
	}
	else
	{
		return E__OK;
	}	
}


/*******************************************************************************
*
* smcMboxPost
*
* Post a message in the communication mailbox.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
static int smcMboxPost( xQueueHandle *hMbox, void *pMsg, UINT32 uTimeOut )
{

	/* CLIDBG(("%s : pMsg: 0x%08x uTimeOut: %u\n", __FUNCTION__, (UINT32) pMsg, uTimeOut)); */
	
	if( xQueueSend( *hMbox, &pMsg, uTimeOut ) == pdPASS )
	{
		return E__OK;
	}
	else
	{		
		/* The queue was already full. */
		return E__FAIL;
	}

}


/*******************************************************************************
*
* smcMboxFetch
*
* Fetch a message in the communication mailbox.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
static int smcMboxFetch( xQueueHandle *hMbox, void **ppMsg, UINT32 uTimeOut )
{

	if( xQueueReceive( *hMbox, &(*ppMsg), uTimeOut ) == pdPASS )
	{
		/* CLIDBG(("%s: pMsg: 0x%08x uTimeOut: %u\n", __FUNCTION__, (UINT32) *ppMsg, uTimeOut)); */
		
		return E__OK;
	}
	else 
	{
		/* Timed out. */
		*ppMsg = NULL;
		return E__TIMEOUT;
	}
}


/*******************************************************************************
*
* smcConnect
*
* Connect to Soak Master. 
* 
*
*
* RETURNS: connected sFd or error (< 0)
* 
*******************************************************************************/
static int smcConnect( char *serverIpAddr )
{
	int sFd;
	int status;
	struct sockaddr_in serverAddr; 
	int sockAddrSize;
	UINT32 conError;
	int opt;
	socklen_t optlen;

#ifdef INCLUDE_DEBUG_VGA	
	int cpuNo;
	UINT32 taskNum;
	cpuNo = sPortGetCurrentCPU();
	taskNum = uxTaskNumGet( cpuNo, NULL );
#endif
	
	
	/* Setup server address */
	sockAddrSize = sizeof( struct sockaddr_in );
	memset( (char *) &serverAddr, 0, sockAddrSize );
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons( SERVER_PORT_NUM );
	
	serverAddr.sin_addr.s_addr = inet_addr( serverIpAddr );
	
	/* CLIDIAG(("smcConnect: server: %s (0x%x)\n", serverIpAddr, serverAddr.sin_addr.s_addr)); */

	if ( (serverAddr.sin_addr.s_addr == IPADDR_NONE) ||
		 (serverAddr.sin_addr.s_addr == 0))
	{
		CLIERR(("Error - invalid server address [%s]\n", serverIpAddr));
		sFd = -1;
		vGetUSMIPAddrs( serverIpAddr );
		atomic32Inc( &smcConnRetryCount );
	}
	else
	{
		CLIDIAG(("Connecting to server: %s\n", serverIpAddr));
		
#ifdef INCLUDE_DEBUG_VGA
		vgaPrinfXY( 1, currentLine, "CPU:%d T%02u: smc    [%s] attempting connect...", 
				cpuNo, taskNum, serverIpAddr );
		vgaClearToEol();
#endif
		conError = 0;
		
		do
		{
			/* Create a TCP socket */
			sFd = lwip_socket( AF_INET, SOCK_STREAM, 0 );
			
			if ( sFd < 0 )
			{
				CLIERR(("Error - unable to create socket, errno: %d\n", errno));
				status = 0;
			}
			else
			{
				status = lwip_connect( sFd, (struct sockaddr *) &serverAddr, sockAddrSize );
				
				if ( status < 0 )
				{
					if (errno == ECONNRESET)
					{
						/* Resets can happen repeatedly, so wait a while before retrying */
						vTaskDelay( 1000 );
					}
					
					conError++;
					atomic32Inc( &smcConnRetryCount );
				
#ifdef INCLUDE_DEBUG_VGA
					vgaPrinfXY( 1, currentLine, "CPU:%d T%02u: smc    [%s] connect: %u failed, errno: %d", 
							cpuNo, taskNum, serverIpAddr, conError, errno );
					vgaClearToEol();
#endif
					lwip_close( sFd );
					sFd = -1;
				}
				else
				{
					/* Set recv timeout (100 ms) */
				  	opt = 100;
				  	optlen = sizeof(int);
				  	if ( lwip_setsockopt( sFd, SOL_SOCKET, SO_RCVTIMEO, &opt, optlen ) < 0 )
					{
						CLIERR(("Error - setsockopt failed, errno: %d\n", errno));
					}
				  	
				  	opt = 0;
				  	if ( lwip_getsockopt( sFd, SOL_SOCKET, SO_RCVTIMEO, &opt, &optlen ) < 0 )
					{
						CLIERR(("Error - getsockopt failed, errno: %d\n", errno));
					}
					else
					{
						CLIDBG(("%s: opt: %d optlen: %u\n", __FUNCTION__, opt, optlen));
					}
					
					smConnectActive = TRUE;
				}
				
				/* CLIDBG(("lwip_connect: status: %d\n", status)); */
			}
		}
		while ( (smClientActive == TRUE) && (status < 0) );
	}
	
	return sFd;
}


/*******************************************************************************
*
* smcSendPkt
*
* Sends given packet data to Soak Master. 
* 
*
*
* RETURNS: number of bytes transmitted.
* 
*******************************************************************************/
static int smcSendPkt( int sFd, UINT8 *pTxBuf, int txLength )
{
	int txCount;
	
	
	txCount = lwip_write( sFd, (char *) pTxBuf, txLength );
	
	CLIDBG(("%s: - txCount: %d [0x%X]\n", __FUNCTION__, txCount, txCount));
	
	if ( txCount < 0 )
	{
		CLIERR(("%s: Error - sending SM response, errno: %d txCount: %d\n", 
				__FUNCTION__, errno, txCount));
				
		atomic32Inc( &smcErrCount );
		
		if ( (errno == EBADF) || (errno == ECONNABORTED) || 
			(errno == ECONNRESET) || (errno == ENOTCONN) )
		{
			/* Connection lost */
			smConnectActive = FALSE;
			atomic32Inc( &smcConnErrCount );
			
			CLIERR(("%s: Error - connection lost\n", __FUNCTION__));
		}
	}
	else if (txCount < txLength)
	{
		CLIERR(("%s: Error - txCount %d, less than expected\n", __FUNCTION__, txCount));
		atomic32Inc( &smcErrCount );
	}
	else
	{
		atomic32Inc( &smcTxCount );
	}
	
	return txCount;
}


/*******************************************************************************
*
* smcRecvPkt
*
* Attempt to receive packet data from the Soak Master. 
* 
*
*
* RETURNS: number of bytes received.
* 
*******************************************************************************/
static int smcRecvPkt( int sFd, UINT8 *pRxBuf, int rxLength )
{
	int rdCount;
	
	
	/* Try to read a message from the Soak Master */
	rdCount = lwip_read( sFd, (char *) pRxBuf, rxLength );

	if ( rdCount <= 0 )
	{	
		if ( (errno > 0) && (errno != EAGAIN) )
		{	
			CLIERR(("%s: Error - reading SM message, errno: %d rdCount: %d\n", 
					__FUNCTION__, errno, rdCount));
			
			if ( (errno == EBADF) || (errno == ECONNABORTED) || 
				(errno == ECONNRESET) || (errno == ENOTCONN) )
			{
				/* Connection lost */
				smConnectActive = FALSE;
				atomic32Inc( &smcConnErrCount );
				
				CLIERR(("%s: Error - connection lost\n", __FUNCTION__));
			}
			else
			{
				atomic32Inc( &smcErrCount );
			}
			
			/* Indicate there was an error */
			rdCount = -1;
		}
		else
		{
			/* Make sure we clear the read count as there's no valid data */
			/* and we don't want to indicate an error for EAGAIN */
			rdCount = 0;
		}
	}
	else
	{
		CLIDBG(("%s: pRxBuf: 0x%x, rxLength: %d, rdCount: %d\n", 
				__FUNCTION__, (UINT32) pRxBuf, rxLength, rdCount));
	}
	
	return rdCount;
}


/*******************************************************************************
*
* smcSendErrorResp
*
* Send error response to Soak Master. 
* 
*
*
* RETURNS: None.
* 
*******************************************************************************/
static void smcSendErrorResp( int sFd, UINT8 *pBuffer, char opcode, UINT32 errCode )
{
	int packetSize;
	SSMMPACKET *pPktUsm;
	char respOpcode;
	BOOL doSend;
	
	
	if (smcConnType == SMC_CONN_TYPE_USM)
	{
		doSend = TRUE;
		
		switch( opcode )
		{
			case SSMMP_RSINFO: 	packetSize = sizeof(SSMMPRSLINFOACK);
								respOpcode = SSMMP_RSINFO_ACK;
								break;
								
			case SSMMP_AADDR: 	packetSize = sizeof(SSMMPAADDRACK);
								respOpcode = SSMMP_AADDR_ACK;
								break;
								
			case SSMMP_RTLIST: 	packetSize = sizeof(SSMMPRTLISTACK);
								respOpcode = SSMMP_RTLIST_ACK;
								break;
								
			case SSMMP_ETEST:	packetSize = sizeof(SSMMPETESTACK);
								respOpcode = SSMMP_ETEST_ACK;
								break;
								
			case SSMMP_RSTATUS:	packetSize = sizeof(SSMMPRSTATUSACK);
								respOpcode = SSMMP_RSTATUS_ACK;
								break;
			
			default: 			doSend = FALSE;
								break;
		}
		
		if (doSend == TRUE)
		{
			pPktUsm = (SSMMPACKET *) pBuffer;
			memset( (char *) pPktUsm, 0, packetSize );
			
			strcpy( pPktUsm->header.packetId, SSMMP_PACKET_ID );
			pPktUsm->header.opcode = respOpcode;
			pPktUsm->header.packetSize = (UINT32) packetSize;
			pPktUsm->header.result = errCode;
			
			smcSendPkt( sFd, pBuffer, packetSize );
		}
	}
}


/*******************************************************************************
*
* lsmReqInfo
*
* Creates a request information packet in legacy SM format. 
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static uPacket *lsmReqInfo( void )
{
	UINT16 i;
	uPacket *pPktLeg;
	
	
	pPktLeg = (uPacket *) mem_malloc( 14 );
						
	if (pPktLeg != NULL)
	{		
		pPktLeg->hdr.wLength = 14;
		pPktLeg->hdr.wSourceID = SOAK_ID; /* 0x8000 */
		pPktLeg->hdr.wDestID = smcBoardAddr;
		pPktLeg->hdr.wFlags = (FLAG_WHO_AMI | FLAG_HAS_DTA | FLAG_SYS_PKT | FLAG_PKT_PRI);
		pPktLeg->hdr.dType = (UINT32) id_AA_SL;	/* 3 Slave Address Allocation Packet */
		
		/* Add the checksum */
	    pPktLeg->hdr.wChecksum = 0;
	    for(i = 0; i < 8; i++)
	    {
			pPktLeg->hdr.wChecksum += pPktLeg->bData[i];
	    }
	    for(i = 10; i < pPktLeg->hdr.wLength; i++)
	    {
	    	pPktLeg->hdr.wChecksum += pPktLeg->bData[i];
	    }
	}
	else
	{
		CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
		atomic32Inc( &smcMemErrCount );
	}
	
	return pPktLeg;
}


/*******************************************************************************
*
* lsmReqAssignAddr
*
* Creates an assign address packet in legacy SM format. 
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static uPacket *lsmReqAssignAddr( SSMMPAADDR *pPktAddr )
{
	UINT16 i;
	uPacket *pPktLeg;
	
	
	pPktLeg = (uPacket *) mem_malloc( sizeof(uPacket) );
						
	if (pPktLeg != NULL)
	{		
		pPktLeg->hdr.wLength = 14;
		pPktLeg->hdr.wSourceID = SOAK_ID; /* 0x8000 */
		pPktLeg->hdr.wDestID = smcBoardAddr;
		pPktLeg->hdr.wFlags = (FLAG_WHO_AMI | FLAG_HAS_DTA | FLAG_SYS_PKT | FLAG_PKT_PRI);
		pPktLeg->hdr.dType = (UINT32) id_AA_PORT;	/* 4 Soak Master Address Allocation Packet */
		
		pPktLeg->wData[7] = (UINT16)(pPktAddr->noOfSlaveBoards & 0xffff);
		pPktLeg->hdr.wLength += 2;
		
		for (i = 0; i < pPktLeg->wData[7]; i++)
		{
			pPktLeg->wData[8 + i] = (UINT16) (pPktAddr->slaveBoardAddress[i] & 0xffff);
			pPktLeg->hdr.wLength += 2;			
		}
		
		/* Add the checksum */
	    pPktLeg->hdr.wChecksum = 0;
	    for(i = 0; i < 8; i++)
	    {
			pPktLeg->hdr.wChecksum += pPktLeg->bData[i];
	    }
	    for(i = 10; i < pPktLeg->hdr.wLength; i++)
	    {
	    	pPktLeg->hdr.wChecksum += pPktLeg->bData[i];
	    }
	}
	else
	{
		CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
		atomic32Inc( &smcMemErrCount );
	}
	
	return pPktLeg;
}


/*******************************************************************************
*
* lsmReqTestList
*
* Creates a request test list packet in legacy SM format. 
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static uPacket *lsmReqTestList( SSMMPRTLIST *pPktRtl )
{
	UINT16 i;
	uPacket *pPktLeg;
	
	
	pPktLeg = (uPacket *) mem_malloc( 14 );
						
	if (pPktLeg != NULL)
	{		
		pPktLeg->hdr.wLength = 14;
		pPktLeg->hdr.wSourceID = SOAK_ID; /* 0x8000 */
		pPktLeg->hdr.wDestID = pPktRtl->boardAddress;
		pPktLeg->hdr.wFlags = (FLAG_HAS_DTA | FLAG_SYS_PKT);
		pPktLeg->hdr.dType = id_RBL;	/* 5 Request BIST List */
		
		/* Add the checksum */
	    pPktLeg->hdr.wChecksum = 0;
	    for(i = 0; i < 8; i++)
	    {
			pPktLeg->hdr.wChecksum += pPktLeg->bData[i];
	    }
	    for(i = 10; i < pPktLeg->hdr.wLength; i++)
	    {
	    	pPktLeg->hdr.wChecksum += pPktLeg->bData[i];
	    }
	}
	else
	{
		CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
		atomic32Inc( &smcMemErrCount );
	}
	
	return pPktLeg;
}


/*******************************************************************************
*
* lsmExeTest
*
* Creates an execute test packet in legacy SM format. 
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static uPacket *lsmExeTest( SSMMPETEST *pEtest )
{
	int len;
	UINT16 i;
	uPacket *pPktLeg;
	
	
	pPktLeg = (uPacket *) mem_malloc( sizeof(struct sPkt_BGO) );
						
	if (pPktLeg != NULL)
	{		
		pPktLeg->sBgo.wSourceID = SOAK_ID; /* 0x8000 */
		pPktLeg->sBgo.wDestID 	= pEtest->boardAddress;
		pPktLeg->sBgo.wFlags 	= (FLAG_TST_ENT | FLAG_HAS_DTA | FLAG_SYS_PKT);
		pPktLeg->sBgo.wTestId 	= (UINT16) (pEtest->testNumber & 0xffff);
		
		pPktLeg->sBgo.achTail[0] = '\0';
		
		len = strlen( pEtest->testTail );
		
		if ( (len > 0) && (len < MaxTail) )
		{
			strcpy( pPktLeg->sBgo.achTail, pEtest->testTail );
		}
		
	  	pPktLeg->sBgo.wLength = 12 + strlen( pPktLeg->sBgo.achTail ) + 1;
		
		/* Add the checksum */
	    pPktLeg->sBgo.wChecksum = 0;
	    for(i = 0; i < 8; i++)
	    {
	    	pPktLeg->sBgo.wChecksum += pPktLeg->bData[i];
	    }
	    for(i = 10; i < pPktLeg->hdr.wLength; i++)
	    {
	    	pPktLeg->sBgo.wChecksum += pPktLeg->bData[i];
	    }
	}
	else
	{
		CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
		atomic32Inc( &smcMemErrCount );
	}
	
	return pPktLeg;
}


/*******************************************************************************
*
* smcTranslateRxPkt
*
* Translate and copy the packet in given buffer. Slave test handler expects
* the packet in legacy format. If a packet is received from the Universal
* Soak Master it is converted to the legacy format.  
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static void *smcTranslateRxPkt( int sFd, UINT8 *pBuffer, int pktLen, char *pOpcode )
{
	uPacket *pPktLeg;
	SSMMPACKET *pPktUsm;
	
	
	if (smcConnType == SMC_CONN_TYPE_LEG)
	{
		*pOpcode = 0xff;
				
		pPktLeg = (uPacket *) mem_malloc( pktLen );
						
		if (pPktLeg != NULL)
		{		
			bcopy( (char *) pBuffer, (char *) pPktLeg, pktLen );
		}
		else
		{
			CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
			atomic32Inc( &smcMemErrCount );
		}
	}
	else if (smcConnType == SMC_CONN_TYPE_USM)
	{
		pPktUsm = (SSMMPACKET *) pBuffer;
		*pOpcode = pPktUsm->header.opcode;
			
		CLIDBG(("smcTranslateRxPkt: header.opcode: %X\n", pPktUsm->header.opcode));
		
		switch( pPktUsm->header.opcode )
		{
			case SSMMP_RSINFO: 	pPktLeg = lsmReqInfo();
								break;
								
			case SSMMP_AADDR: 	pPktLeg = lsmReqAssignAddr( &pPktUsm->aaddr );
								break;
								
			case SSMMP_RTLIST: 	pPktLeg = lsmReqTestList( &pPktUsm->rtList );
								break;
								
			case SSMMP_ETEST:	pPktLeg = lsmExeTest( &pPktUsm->exeTest );
								break;
								
			case SSMMP_RSTATUS:	pPktLeg = NULL;	/* Not supported yet */
								break;
			
			default: 			pPktLeg = NULL;
								*pOpcode = 0xff;
								break;
		}
		
		if (pPktLeg == NULL)
		{
			smcSendErrorResp( sFd, pBuffer, *pOpcode, USM_PKT_ERROR );
		}
	}
	else
	{
		pPktLeg = NULL;
	}
	
	return (void *) pPktLeg;
}


/*******************************************************************************
*
* usmSlaveInfoAck
*
* Creates a slave information acknowledge packet in Universal SM format. 
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static UINT8 *usmSlaveInfoAck( uPacket *pPktLeg, int *pTxLen )
{
	int len;
	UINT32 i;
	UINT32 idx;
	SSMMPRSLINFOACK *pPktUsm;
	int packetSize;
	
	
	packetSize = (int) sizeof( SSMMPRSLINFOACK );
	pPktUsm = (SSMMPRSLINFOACK *) mem_malloc( packetSize );
						
	if (pPktUsm != NULL)
	{	
		memset( (char *) pPktUsm, 0, packetSize );
		
		strcpy( pPktUsm->header.packetId, SSMMP_PACKET_ID );
		pPktUsm->header.opcode = SSMMP_RSINFO_ACK;
		pPktUsm->header.packetSize = (UINT32) packetSize;
		pPktUsm->header.result = 0;
		
		if ( (pPktLeg->bData[14] > 0) && (pPktLeg->bData[14] < SSMMP_MAX_SLAVE_BOARDS) )
		{
			pPktUsm->noOfSlaveBoards = pPktLeg->bData[14];
			
			if (pPktLeg->hdr.wSourceID == 0xffff)
			{
				pPktUsm->boardAddrDetectMechanism = USM_BOARDADDR_SMALLOC_MECH;
			}
			else
			{
				pPktUsm->boardAddrDetectMechanism = USM_BOARDADDR_SLOTID_MECH;
			}
			
			i = 0;
			idx = 15;
			
			/* Currently we only support 1 slave, as we're the only board */
			/* TODO: need to set values for slaves, other than first, properly */
			while ( (i < pPktUsm->noOfSlaveBoards) && (idx < pPktLeg->hdr.wLength))
			{	
				if (pPktLeg->hdr.wSourceID == 0xffff)
				{
					/* Set to our current address */
					pPktUsm->slaveBoard[i].boardAddress = (smcBoardAddr & USM_BOARDADDR_SLOTID_MASK);
				}
				else
				{
					/* Set to the detected address */
					pPktUsm->slaveBoard[i].boardAddress = (pPktLeg->hdr.wSourceID & USM_BOARDADDR_SLOTID_MASK);
				}
				
				pPktUsm->slaveBoard[i].chassisId = 0; // currently not used
				
				len = strlen( (char *) &pPktLeg->bData[idx] );
				
				if ( (len > 0) && (len < SSMMP_BOARDNAME_SIZE) )
				{
					strcpy( pPktUsm->slaveBoard[i].boardName, (char *) &pPktLeg->bData[idx] );
					idx += len + 1;
					
					len = strlen( (char *) &pPktLeg->bData[idx] );
					
					if ( (len > 0) && (len < SSMMP_BOARD_SERIAL_NUM) )
					{
						strcpy( pPktUsm->slaveBoard[i].serNumber, (char *) &pPktLeg->bData[idx] );
						idx += len + 1;
					}
				}

				i++;
			}
		}
		
		CLIDBG(("%s: noOfSlaveBoards: %u\n", __FUNCTION__, pPktUsm->noOfSlaveBoards));
		
		*pTxLen = packetSize;
	}
	else
	{
		CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
		atomic32Inc( &smcMemErrCount );
		*pTxLen = 0;
	}
	
	return (UINT8 *) pPktUsm;
}


/*******************************************************************************
*
* usmSlaveAddrAck
*
* Creates a slave assign address acknowledge packet in Universal SM format. 
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static UINT8 *usmSlaveAddrAck( uPacket *pPktLeg, int *pTxLen )
{
	SSMMPAADDRACK *pPktUsm;
	int packetSize;
	
	
	packetSize = (int) sizeof( SSMMPAADDRACK );
	pPktUsm = (SSMMPAADDRACK *) mem_malloc( packetSize );
						
	if (pPktUsm != NULL)
	{	
		memset( (char *) pPktUsm, 0, packetSize );
		
		strcpy( pPktUsm->header.packetId, SSMMP_PACKET_ID );
		pPktUsm->header.opcode = SSMMP_AADDR_ACK;
		pPktUsm->header.packetSize = (UINT32) packetSize;
		pPktUsm->header.result = 0;
		
		/* Update our address */
		smcBoardAddr = pPktLeg->wData[8];
		
		/* Currently we only support 1 slave, as we're the only board */
		/* TODO: need to set values for slaves, other than first */
		
		CLIDBG(("%s: noOfSlaveBoards: %u boardAddress: 0x%04x\n", 
				__FUNCTION__, pPktLeg->wData[7], smcBoardAddr));
		
		*pTxLen = packetSize;
	}
	else
	{
		CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
		atomic32Inc( &smcMemErrCount );
		*pTxLen = 0;
	}
	
	return (UINT8 *) pPktUsm;
}


/*******************************************************************************
*
* usmTestListAck
*
* Creates a test list packet acknowledge in Universal SM format. 
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static UINT8 *usmTestListAck( uPacket *pPktLeg, int *pTxLen )
{
	int	count;
	int idx;
	int maxSize;
	UINT32 noOfTests;
	SSMMPRTLISTACK *pPktUsm;
	int packetSize;
	
	
	packetSize = (int) sizeof( SSMMPRTLISTACK );
	pPktUsm = (SSMMPRTLISTACK *) mem_malloc( packetSize );
						
	if (pPktUsm != NULL)
	{	
		memset( (char *) pPktUsm, 0, packetSize );
				
		strcpy( pPktUsm->header.packetId, SSMMP_PACKET_ID );
		pPktUsm->header.opcode = SSMMP_RTLIST_ACK;
		pPktUsm->header.packetSize = (UINT32) packetSize;
		pPktUsm->header.result = 0;
		
		pPktUsm->boardAddress = pPktLeg->hdr.wSourceID;
				
		/* Build the test list */

		idx = sizeof (struct sHeader) + 10; /* Tests list begins after header and board name */
		
		maxSize = sizeof( uPacket );
		
		pPktUsm->noOfTests = 0;
		noOfTests = 0;
		
		while( (pPktLeg->bData[idx]!= 0) && (idx < maxSize) )
		{	
			if (noOfTests < SSMMP_MAX_TESTS_PER_BOARD)
			{
				/* Copy test details */
				
				pPktUsm->testList[noOfTests].testNumber = pPktLeg->bData[idx] +(pPktLeg->bData[idx+1] * 256);  
				idx += 2;
				pPktUsm->testList[noOfTests].testVersion = 0;  // not used
				idx += 2;
				count = 0;
				
				while (pPktLeg->bData[idx]!= 0 && (count < SSMMP_TESTTITLE_LEN) )
				{
					/* Copy test description characters */
					pPktUsm->testList[noOfTests].testTitle[count] = pPktLeg->bData[idx];
					count++;
					idx++;
				}
				
				/* NULL terminate */
				pPktUsm->testList[noOfTests].testTitle[count] = '\0';
				idx++;
				
				pPktUsm->noOfTests++;
			}
			else
			{
				/* This shouldn't happen due to the size contrains, */
				/* but if the number of tests exceeds the max, just count them */
				idx += 4;
				count = 0;
				
				while (pPktLeg->bData[idx]!= 0 && (count < SSMMP_TESTTITLE_LEN) )
				{
					idx++;
				}
				
				idx++;
			}
			
			noOfTests++;
		}
		
		CLIDBG(("%s: %u tests found\n", __FUNCTION__, noOfTests));
		
		if (noOfTests > SSMMP_MAX_TESTS_PER_BOARD)
		{
			pPktUsm->header.result = USM_NSUPP_NUM_OF_TEST;
		}
		
		*pTxLen = packetSize;
	}
	else
	{
		CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
		atomic32Inc( &smcMemErrCount );
		*pTxLen = 0;
	}
	
	return (UINT8 *) pPktUsm;
}


/*******************************************************************************
*
* usmExeTestAck
*
* Creates a test result acknowledge packet in Universal SM format. 
* 
*
*
* RETURNS: a pointer to the packet or NULL.
* 
*******************************************************************************/
static UINT8 *usmExeTestAck( uPacket *pPktLeg, int *pTxLen )
{
	SSMMPETESTACK *pPktUsm;
	int packetSize;
	int descLen;
	

	if (pPktLeg->hdr.wLength < 14)
	{
		CLIERR(("%s: Error - invalid packet length: %u\n", 
				__FUNCTION__, pPktLeg->hdr.wLength));
		
		pPktUsm = NULL;	
		*pTxLen = 0;
	}
	else
	{
		packetSize = (int) sizeof( SSMMPETESTACK );
		pPktUsm = (SSMMPETESTACK *) mem_malloc( packetSize );
							
		if (pPktUsm != NULL)
		{
			memset( (char *) pPktUsm, 0, packetSize );
					
			strcpy( pPktUsm->header.packetId, SSMMP_PACKET_ID );
			pPktUsm->header.opcode = SSMMP_ETEST_ACK;
			pPktUsm->header.packetSize = (UINT32) packetSize;
			pPktUsm->header.result = 0;
			
			pPktUsm->boardAddress = pPktLeg->hdr.wSourceID;
			
			pPktUsm->testNumber = pPktLeg->wData[5];
			pPktUsm->testErrorCode = pPktLeg->wData[6];
			
			CLIDBG(("%s: testNumber: %u testErrorCode: 0x%08x\n", __FUNCTION__, 
						pPktUsm->testNumber, pPktUsm->testErrorCode));
			
			if (pPktLeg->hdr.wLength > 14)
			{
				descLen = strlen( (char *) &pPktLeg->bData[14] );
				
				if ( (descLen > 0) && (descLen < SSMMP_MAX_ERR_DESC) )
				{
					strcpy ( (char *) pPktUsm->errorDesc, (char *) &pPktLeg->bData[14] );
				}		
			}
			
			*pTxLen = packetSize;			
		}
		else
		{
			CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
			atomic32Inc( &smcMemErrCount );
			*pTxLen = 0;
		}
	}
	
	return (UINT8 *) pPktUsm;
}


/*******************************************************************************
*
* smcTranslateTxPkt
*
* Translate and copy the legacy packet to a transmit buffer. 
* If the connection type is legacy then the original packet is used.  
* 
*
*
* RETURNS: a pointer to the buffer to transmit or NULL.
* 
*******************************************************************************/
static UINT8 *smcTranslateTxPkt( uPacket *pPktLeg, int *pTxLen, int *pStatus )
{
	UINT8 *pTxBuf;
	
	
	if (smcConnType == SMC_CONN_TYPE_LEG)
	{
		pTxBuf = (UINT8 *) pPktLeg;
	
		*pTxLen = pPktLeg->hdr.wLength;
		
		*pStatus = SMC_PKT_OK;			
	}
	else if (smcConnType == SMC_CONN_TYPE_USM)
	{
		if ( (pPktLeg->hdr.wLength < 14) &&
			 (pPktLeg->hdr.wFlags & FLAG_ACK_PKT) )
		{
			CLIDBG(("%s: ignoring ACK packet\n", __FUNCTION__));
			pTxBuf = NULL;
			*pTxLen = 0;
			*pStatus = SMC_PKT_IGNORE;
		}
		else
		{
			if ( pPktLeg->hdr.wFlags & FLAG_TST_XIT )
			{	
				pTxBuf = usmExeTestAck( pPktLeg, pTxLen );
			}
			else
			{
				CLIDBG(("smcTranslateTxPkt: hdr.dType: %X\n", pPktLeg->hdr.dType));

				switch( pPktLeg->hdr.dType )
				{
					case id_SBL: 	pTxBuf = usmTestListAck( pPktLeg, pTxLen );
									break;
									
					case id_AA_SL: 	pTxBuf = usmSlaveInfoAck( pPktLeg, pTxLen );
									break;
									
					case id_AA_PORT: pTxBuf = usmSlaveAddrAck( pPktLeg, pTxLen );
									break;
					
					default: 		pTxBuf = NULL;
									CLIERR(("%s: Error - invalid packet ID: %u\n", 
											__FUNCTION__, pPktLeg->hdr.dType)); 
									break;
				}
			}
			
			if (pTxBuf != NULL)
			{
				*pStatus = SMC_PKT_OK;
			}
			else
			{
				*pStatus = SMC_PKT_ERROR;
			}
		}
		
		mem_free( pPktLeg );
	}
	else
	{
		CLIERR(("%s: Error - SM connection invalid \n", __FUNCTION__));
		pTxBuf = NULL;
		*pTxLen = 0;
		*pStatus = SMC_PKT_ERROR;
		
		mem_free( pPktLeg );
	}
	
	return pTxBuf;
}


/*******************************************************************************
*
* smcGetConnType
*
* Determines the connection type based on the packet in the given buffer. 
* 
*
*
* RETURNS: the connection type.
* 
*******************************************************************************/
static int smcGetConnType( UINT8 *pBuffer, int *pLength )
{
	uPacket *pPktLeg;
	SSMMPACKET *pPktUsm;
	char *p;
	int type;


	/* First check if this is from the USM */
	
	pPktUsm = (SSMMPACKET *) pBuffer;
	p = (char *) pPktUsm->header.packetId;				
	
	if ( (p[0] == 'S') && (p[1] == 'H') && (p[2] == 'A') && (p[3] == 'A') && 
		 (p[4] == 'N') && (p[5] == 'T') && (p[6] == 'I') && (p[7] == '\0') )
	{
		/* This is a Universal SM Packet */
		
		type = SMC_CONN_TYPE_USM;
		
		*pLength = pPktUsm->header.packetSize;
	}
	else
	{	 
		pPktLeg = (uPacket *) pBuffer;
		
		/* Check length is valid */
		if ( (pPktLeg->hdr.wLength == 0) || (pPktLeg->hdr.wLength > sizeof(uPacket)) )
		{
			/* This packet is unknown */
			
			type = SMC_CONN_TYPE_UKN;
			
			*pLength = 0;					
		}
		else
		{
			/* Assume this is a legacy SM Packet */
			
			type = SMC_CONN_TYPE_LEG;
			
			*pLength = pPktLeg->hdr.wLength;
		}
	}
	
	CLIDBG(("%s: type: %d length: %d\n", __FUNCTION__, type, *pLength));
					
	return type;
}


/*******************************************************************************
*
* smcResetStats
*
* Reset statistic counters. 
* 
*
*
* RETURNS: None
* 
*******************************************************************************/
static void smcResetStats( void )
{
	atomic32Set( &smcRxCount, 0 );
	atomic32Set( &smcTxCount, 0 );
	atomic32Set( &smcErrCount, 0 );
	atomic32Set( &smcMemErrCount, 0 );
	atomic32Set( &smcConnErrCount, 0 );
	atomic32Set( &smcConnRetryCount, 0 );
	
	atomic32Set( &smcPostCount, 0 );
	atomic32Set( &smcPostErrCount, 0 );
	atomic32Set( &smcFetchCount, 0 );
	atomic32Set( &smcFetchErrCount, 0 );
	
	atomic32Set( &sthPostCount, 0 );
	atomic32Set( &sthPostErrCount, 0 );
	atomic32Set( &sthFetchCount, 0 );
	atomic32Set( &sthFetchErrCount, 0 );
}


/*******************************************************************************
*
* smClient
*
* Soak Master client task. 
* Attemps to connect to the Soak Master server and send/receive data.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void smClient( void *pvParams )
{
	int sFd;
	void *pPkt;
	int status;
	char opcode;
	UINT32 loopCount;
	UINT8 *pTxBuf;
	int txLength;
	int rxCount;
	int rdCount;
	int rxLength;
	int rxMaxLen;
#if defined (INCLUDE_DEBUG_VGA) || defined (INCLUDE_LWIP_SMC_DEBUG)
	int cpuNo;
#endif

#ifdef INCLUDE_DEBUG_VGA
	UINT32 taskNum;
	
	if ( currentLine == -1 )
	{
		currentLine = atomic32Inc( (UINT32 *) &startLine );
	}
	
	vgaClearLine( currentLine );
	cpuNo = sPortGetCurrentCPU();
	taskNum = uxTaskNumGet( cpuNo, NULL );
#endif
#ifdef INCLUDE_LWIP_SMC_DEBUG
	cpuNo = sPortGetCurrentCPU();
#endif
	
	CLIDBG(("%s: started on CPU:%d, smcBuffer: 0x%08x size: %u\n", 
			__FUNCTION__, cpuNo, (UINT32) smcBuffer, MAX_REQ_MSG_SIZE));

	sFd = -1;
	loopCount = 0;
	rxCount = 0;
	rxLength = 0;
	
	smcResetStats();
	
#ifdef INCLUDE_DEBUG_VGA
	vgaPrinfXY( 1, currentLine, "CPU:%d T%02u: smc    Count: %u TX: %u RX: %u ERR: %u", 
			cpuNo, taskNum, loopCount, atomic32Get( &smcTxCount ), 
			atomic32Get( &smcRxCount ), atomic32Get( &smcErrCount ) );
	vgaClearToEol();
#endif
	
	rxMaxLen = MAX_REQ_MSG_SIZE;
		
	while ( smClientActive == TRUE )
	{
		if (smConnectActive == FALSE)
		{
			rxCount = 0;
			rxLength = 0;
			rxMaxLen = MAX_REQ_MSG_SIZE;
						
			/* Close existing connection before attempting a re-connect */
			if (sFd >= 0)
			{
				lwip_close( sFd );	
			}
			
			/* Try to connect to the Soak Master */
			sFd = smcConnect( smIpAddr );
		}
		else
		{
			/* We have an open connection to the Soak Master */
			
			/* Receive and process packets */
			
			rdCount = smcRecvPkt( sFd, &smcBuffer[rxCount], (rxMaxLen - rxCount) );
			
			if ( rdCount <= 0 )
			{
				if ( rdCount < 0 )
				{
					/* There was an error, re-start at the begining again */
					rxCount = 0;
					rxMaxLen = MAX_REQ_MSG_SIZE;
				}
			}
			else
			{
				if (rxCount == 0)
				{
					/* Start of packet, get type and length */
					
					smcConnType = smcGetConnType( smcBuffer, &rxLength );
					
					/* Check length is in range */
					if ( (rxLength == 0) || (rxLength > rxMaxLen) )
					{
						CLIERR(("%s: Error - out of range, rxLength: %d rdCount: %d\n", 
								__FUNCTION__, rxLength, rdCount));
								
						atomic32Inc( &smcErrCount );
						rxCount = 0;
						rxLength = 0;
						rxMaxLen = MAX_REQ_MSG_SIZE;
					}
					else
					{
						rxCount += rdCount;
						rxMaxLen = rxLength;
					}
				}
				else
				{
					rxCount += rdCount;
				}
							
				CLIDBG(("%s: rdCount: %d rxCount: %d rxLength: %d rxMaxLen: %d\n", 
						__FUNCTION__, rdCount, rxCount, rxLength, rxMaxLen));
					
				if ( (rxLength > 0 ) && (rxCount == rxLength) )
				{	
					atomic32Inc( &smcRxCount );
					
					pPkt = smcTranslateRxPkt( sFd, smcBuffer, rxLength, &opcode );
					
					if ( pPkt != NULL )
					{		
						/* CLIDBG(("%s: recd: pPkt: 0x%08x length: %u\n", 
								__FUNCTION__, (UINT32) pPkt, rxLength));
						CLILOGBUF((0, (UINT8 *) pPkt, rxLength)); */
								
						if ( smcMboxPost( &hSthMbox, pPkt, 100 ) == E__OK )
						{
							atomic32Inc( &smcPostCount );
						}
						else
						{
							CLIERR(("%s: Error - post to Mbox failed\n", __FUNCTION__));
							
							mem_free( pPkt );
							
							atomic32Inc( &smcErrCount );
							
							atomic32Inc( &smcPostErrCount );
							
							smcSendErrorResp( sFd, smcBuffer, opcode, USM_STH_RESP_ERROR );
						}
					}
					else
					{
						CLIERR(("%s: Error - smcTranslateRxPkt failed\n", __FUNCTION__));
						atomic32Inc( &smcErrCount );
					}
					
					rxCount = 0;
					rxLength = 0;
					rxMaxLen = MAX_REQ_MSG_SIZE;
				}
				else
				{
					if (rxCount > rxMaxLen)
					{
						CLIERR(("Error - message length: %d too large!!\n", rxCount));
						atomic32Inc( &smcErrCount );
						
						rxCount = 0;
						rxMaxLen = MAX_REQ_MSG_SIZE;
					}
				}
			}
			
			/* Check for any message/response from the STH */
			
			txLength = 0;
			pPkt = NULL;
							
			if ( (smcMboxFetch( &hSmcMbox, (void **) &pPkt, 0 ) == E__OK) && (pPkt != NULL) )
			{	
				atomic32Inc( &smcFetchCount );
				
				pTxBuf = smcTranslateTxPkt( (uPacket *) pPkt, &txLength, &status );
				
				if ( pTxBuf != NULL )
				{
					if ( (status == SMC_PKT_OK) && (txLength > 0) )
					{
						/* CLIDBG(("%s: send: pPkt: 0x%08x length: %u\n", 
								__FUNCTION__, (UINT32) pTxBuf, txLength));
						CLILOGBUF((0, (UINT8 *) pTxBuf, txLength)); */
				
						/* Got a message/response from STH, send to the Soak Master */
					
						smcSendPkt( sFd, pTxBuf, txLength );	
					}
					
					mem_free( pTxBuf );
				}
				else
				{	
					if (status == SMC_PKT_ERROR)
					{
						atomic32Inc( &smcFetchErrCount );
						
						CLIERR(("%s: Error - smcTranslateTxPkt failed\n", __FUNCTION__));
						atomic32Inc( &smcErrCount );
					}
				}
			}
			
			loopCount++;
			
#ifdef INCLUDE_DEBUG_VGA
			vgaPrinfXY( 1, currentLine, "CPU:%d T%02u: smc    Count: %u TX: %u RX: %u ERR: %u", 
				cpuNo, taskNum, loopCount, atomic32Get( &smcTxCount ), 
				atomic32Get( &smcRxCount ), atomic32Get( &smcErrCount ) );
			vgaClearToEol();
#endif
		}
	}
	
	smClientActive = FALSE;
	smConnectActive = FALSE;
	
	if (sFd >= 0)
	{
		lwip_close( sFd );	
	}
	
	CLIDBG(("%s finished\n", __FUNCTION__));
	
	smcFreeMbox( &hSmcMbox );
	smcFreeMbox( &hSthMbox );
	
	if (smcBuffer == NULL)
	{
		mem_free( smcBuffer );
		smcBuffer = NULL;
	}
	
	vTaskDelete( NULL );
}


/*******************************************************************************
*
* smcReady
*
* Detect if Soak Master communication is ready.
*
*
* RETURNS: TRUE or FALSE.
* 
*******************************************************************************/
int smcReady( void )
{
	if ( (smClientActive == TRUE) && (smConnectActive == TRUE) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*******************************************************************************
*
* smcDetectMsg
*
* Detect if message ready in communication mailbox.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
int smcDetectMsg( void )
{
	int result;

	
	if ( (smClientActive == TRUE) && (hSthMbox != NULL) )
	{
		if ( uxQueueMessagesWaiting( hSthMbox ) != 0 )
		{
			result = E__OK;
		}
		else
		{
			result = E_NOT_PENDING;
		}
	}
	else
	{
		result = E_NOT_PENDING;
	}
	
	return result;
}


/*******************************************************************************
*
* smcSetMsgWaiting
*
* Indicate message is ready.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void smcSetMsgWaiting( void )
{
/*	Not currently used */

}


/*******************************************************************************
*
* smcPostMsg
*
* Post a message to the Soak Master Client.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
int smcPostMsg( void *pMsg, UINT32 uTimeOut )
{
	uPacket *pPkt;
	void *pBuf;
	int result;


	if ( (smClientActive == TRUE) && (hSmcMbox != NULL) && (pMsg != NULL) )
	{	
		pPkt = (uPacket *) pMsg;
		
		pBuf = mem_malloc( pPkt->hdr.wLength );
		
		if (pBuf != NULL)
		{
			CLIDBG(("%s: pMsg: 0x%08x -> 0x%08x length: %u timeOut: %u\n", 
					__FUNCTION__, (UINT32) pMsg, (UINT32) pBuf, 
					pPkt->hdr.wLength, uTimeOut));
				
			CLIDBG(("%s: wSourceID: 0x%04x wDestID: 0x%04x wFlags: 0x%04x\n", 
					__FUNCTION__, pPkt->hdr.wSourceID, pPkt->hdr.wDestID, 
					pPkt->hdr.wFlags));	
				
			CLILOGBUF((0, (UINT8 *) pPkt, pPkt->hdr.wLength)); 
	
			bcopy((char *) pMsg, (char *) pBuf, pPkt->hdr.wLength ); 
		
			result = smcMboxPost( &hSmcMbox, pBuf, uTimeOut );
			
			if (result == E__OK)
			{
				atomic32Inc( &sthPostCount );
			}
			else
			{
				CLIERR(("%s: Error - post to Mbox failed\n", __FUNCTION__));
				
				mem_free( pBuf );
				
				atomic32Inc( &sthPostErrCount );
			}
		}
		else
		{
			CLIERR(("%s: Error - mem_malloc failed\n", __FUNCTION__));
			atomic32Inc( &smcMemErrCount );
			result = E__FAIL;
		}
	}
	else
	{
		result = E__FAIL;
	}
	
	return result;

}


/*******************************************************************************
*
* smcFetchMsg
*
* Fetch a message from the Soak Master Client.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
int smcFetchMsg( void *pMsg, UINT32 uTimeOut )
{
	uPacket *pPkt;
	int result;
	
	
	if ( (smClientActive == TRUE) && (hSthMbox != NULL) && (pMsg != NULL) )
	{
		result = smcMboxFetch( &hSthMbox, (void **) &pPkt, uTimeOut );
		
		if (result == E__OK)
		{
			if (pPkt != NULL)
			{
				atomic32Inc( &sthFetchCount );
				
				CLIDBG(("%s: 0x%08x -> pMsg: 0x%08x length: %u timeOut: %u\n", 
						__FUNCTION__, (UINT32) pPkt, (UINT32) pMsg, 
						pPkt->hdr.wLength, uTimeOut));
					
				CLIDBG(("%s: wSourceID: 0x%04x wDestID: 0x%04x wFlags: 0x%04x\n", 
						__FUNCTION__, pPkt->hdr.wSourceID, pPkt->hdr.wDestID, 
						pPkt->hdr.wFlags));
					
				CLILOGBUF((0, (UINT8 *) pPkt, pPkt->hdr.wLength)); 
				
				bcopy( (char *) pPkt, (char *) pMsg, pPkt->hdr.wLength );
				
				mem_free( pPkt );
			}
			else
			{
				CLIERR(("%s: Error - pPkt == NULL\n", __FUNCTION__));
				
				atomic32Inc( &sthFetchErrCount );
			
				result = E__FAIL;
			}
		}
	}
	else
	{
		vTaskDelay( uTimeOut );
		
		result = E__FAIL;
	}
	
	return result;
}


/*******************************************************************************
*
* smcGetStats
*
* This function gets statistic informatiom in a formated string.
*
*
* RETURNS: E__OK or error code.
* 
*******************************************************************************/
size_t smcGetStats( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	size_t len;
	
	len = 0;
	
	
	/* Check buffer is sized for our needs */
	/* We allow 35 characters per line: 38 * 19 = 722 */
	if ( (pcWriteBuffer != NULL) && (xWriteBufferLen >= 722) )
	{
		pcWriteBuffer[0] = 0;
	
		sprintf( (char *) pcWriteBuffer, "\r\nTask status%*s: %s\r\n",
						9, "", (smClientActive ? "Active":"Stopped") );
		len = strlen( (char *) pcWriteBuffer );
		                               
		sprintf( (char *) &pcWriteBuffer[len], "Board address%*s: 0x%x\r\n", 7, "", smcBoardAddr);
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], "Connection status%*s: %s\r\n", 
					3, "", (smConnectActive ? "Connected":"Disconnected"));
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], "Connection type%*s: ", 5, "" );
		len = strlen( (char *) pcWriteBuffer );
		
		if ( smcConnType == SMC_CONN_TYPE_LEG )
		{
			sprintf( (char *) &pcWriteBuffer[len], "Legacy SM\r\n");
			len = strlen( (char *) pcWriteBuffer );
		}
		else if ( smcConnType == SMC_CONN_TYPE_USM )
		{
			sprintf( (char *) &pcWriteBuffer[len], "Universal SM\r\n");
			len = strlen( (char *) pcWriteBuffer );
		}
		else
		{
			sprintf( (char *) &pcWriteBuffer[len], "Unknown\r\n");
			len = strlen( (char *) pcWriteBuffer );
		}
		
		sprintf( (char *) &pcWriteBuffer[len], "SM address%*s: %s\r\n", 10, "", smIpAddr);
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], "Connection retries%*s: %u\r\n", 2, "", atomic32Get( &smcConnRetryCount) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], "Connection errors%*s: %u\r\n", 3, "", atomic32Get( &smcConnErrCount) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], "Communication errors: %u\r\n", atomic32Get( &smcErrCount ) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], "Memory errors:%*s: %u\r\n", 6, "", atomic32Get( &smcMemErrCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], "SM RX messages%*s: %u\r\n", 6, "", atomic32Get( &smcRxCount ) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], "SM TX messages%*s: %u\r\n", 6, "", atomic32Get( &smcTxCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], "SMC post count%*s: %u\r\n", 6, "", atomic32Get( &smcPostCount ) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], "SMC post errors%*s: %u\r\n", 5, "", atomic32Get( &smcPostErrCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], "STH fetch count%*s: %u\r\n", 5, "", atomic32Get( &sthFetchCount ) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], "STH fetch errors%*s: %u\r\n", 4, "", atomic32Get( &sthFetchErrCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], "STH post count%*s: %u\r\n", 6, "", atomic32Get( &sthPostCount ) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], "STH post errors%*s: %u\r\n", 5, "", atomic32Get( &sthPostErrCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], "SMC fetch count%*s: %u\r\n", 5, "", atomic32Get( &smcFetchCount ) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], "SMC fetch errors%*s: %u\r\n", 4, "", atomic32Get( &smcFetchErrCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
		/* sysDebugPrintf("%s: buffer length: %u (max: %u)\n", __FUNCTION__, len, xWriteBufferLen); */
	}
	else
	{
		CLIERR(("%s: Error - buffer: 0x%x invalid, length: %u)\n", 
			__FUNCTION__, (UINT32) pcWriteBuffer, xWriteBufferLen));
	}
	
	return len;
}


/*******************************************************************************
*
* startSmClient
*
* This function creates the Soak Master client task.
*
* Usage: startSmClient( "193.67.84.253" );
*
* RETURNS: None
* 
*******************************************************************************/
void startSmClient( char *serverIpAddr )
{
	int cpuNo;
	
	
	if (smClientActive == FALSE)
	{
		smcBuffer = mem_malloc( MAX_REQ_MSG_SIZE );
		
		if (smcBuffer != NULL)
		{	
			memset( smcBuffer, 0, MAX_REQ_MSG_SIZE );
		}
		else
		{
			CLIERR(("Error - smcBuffer allocation failed\n"));
		}
		
		if ( smcBuffer != NULL )
		{
			/* Save a copy of the Soak Master IP address */
			strcpy( smIpAddr, serverIpAddr );
		
			smcCreateMbox( &hSmcMbox, 32 );
			smcCreateMbox( &hSthMbox, 32 );
		
			cpuNo = 1; // sPortGetCurrentCPU();
			
			smClientActive = TRUE;
			
			if( xTaskCreate( cpuNo, smClient, 
						( signed char * ) "smc", SMC_TASK_STACK_SIZE,
						 NULL, SMC_TASK_PRIORTY, NULL ) != pdPASS )	
			{
				CLIERR(("Error - failed to start smc task\n"));
			}
		}
	}
}


/*******************************************************************************
*
* stopSmClient
*
* This function stops the Soak Master client task.
*
*
* RETURNS: None
* 
*******************************************************************************/
void stopSmClient( void )
{
	smClientActive = FALSE;
	
	vTaskDelay( 2000 );
	
	smcFreeMbox( &hSmcMbox );
	smcFreeMbox( &hSthMbox );
	
	if (smcBuffer == NULL)
	{
		mem_free( smcBuffer );
		smcBuffer = NULL;
	}
}
