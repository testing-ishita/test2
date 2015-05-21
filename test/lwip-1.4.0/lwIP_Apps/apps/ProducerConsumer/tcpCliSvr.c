/*******************************************************************************
*
* Filename:	 tcpCliSvr.c
*
* Description:	TCP client/server test functions.
*
* $Revision: 1.3 $
*
* $Date: 2013-11-25 11:17:51 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/lwIP_Apps/apps/ProducerConsumer/tcpCliSvr.c,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

/* Standard includes. */
#include "stdlib.h"

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/sockets.h"
#include "lwip/mem.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

#include <private/atomic.h>

#include "config.h"
#include "../debug/dbgLog.h"

#include "../vgalib/vga.h"


#define REQUEST_MSG_SIZE		1024 //4096

#define TCP_TASK_PRIORTY 		5
#define TCP_TASK_STACK_SIZE		configMINIMAL_STACK_SIZE

#define SERVER_PORT_NUM			5001
#define SERVER_MAX_CONNECTIONS	4


#undef INCLUDE_LWIP_SVR_DEBUG
#undef INCLUDE_LWIP_CLI_DEBUG

#if defined(INCLUDE_LWIP_SVR_DEBUG) || defined(INCLUDE_LWIP_CLI_DEBUG)
#warning "***** DEBUG ON *****"
#endif

#ifdef INCLUDE_LWIP_SVR_DEBUG /* sysDebugPrintf */
#define SVRDBG(x)	do {DBLOG x;} while(0)
#else
#define SVRDBG(x)
#endif

#ifdef INCLUDE_LWIP_CLI_DEBUG /* sysDebugPrintf */
#define CLIDBG(x)	do {sysDebugPrintf x;} while(0)
#else
#define CLIDBG(x)
#endif


#ifdef INCLUDE_DEBUG_VGA
extern int startLine;
#endif

static UINT32 srvMsgCount = 0;
static UINT32 cliMsgCount = 0;
/* static BOOL verboseMode = FALSE; */
static BOOL tcpClientActive = FALSE;

#ifdef INCLUDE_DEBUG_VGA
static int currentLine = -1;
#endif

/****************************** Server routines *******************************/

/*******************************************************************************
*
* tcpServerWorkTask
*
* Worker task spawned by the TCP server to handle a connection.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void tcpServerWorkTask( void *pvParams )
{
	int sFd;
	
	int rdCount;
	int txCount;
	int rxCount;
	UINT32 loopCount;
	UINT32 msgRecd;
	UINT32 msgSent;
	UINT32 msgError;
	char *serverMsg;
	int status;
	int opt;
	socklen_t optlen;
	
#ifdef INCLUDE_DEBUG_VGA
	int cpuNo;
	UINT32 taskNum;
	char achBuffer[80];
#endif

	
	sFd = (int) *((int *)pvParams);
	
#ifdef INCLUDE_DEBUG_VGA
	cpuNo = sPortGetCurrentCPU();
	taskNum = uxTaskNumGet( cpuNo, NULL );
#endif
	
	status = 0;
	rxCount = 0;
	msgRecd = 0;
	msgSent = 0;
	msgError = 0;
	
	serverMsg = mem_malloc( REQUEST_MSG_SIZE );
	
#ifdef INCLUDE_DEBUG_VGA
		sprintf( achBuffer, "CPU:%d T%02u: tcpWrk started...", cpuNo, taskNum );
		vgaPutsXY( 1, (currentLine + 1), achBuffer );
		vgaClearToEol();
#endif

	/* set recv timeout (1000 ms) */
  	opt = 1000;
  	optlen = sizeof(int);
  	if ( lwip_setsockopt( sFd, SOL_SOCKET, SO_RCVTIMEO, &opt, optlen ) < 0 )
	{
		SVRDBG(("Error - setsockopt failed, errno: %d\n", errno));
	}
  	
  	opt = 0;
  	if ( lwip_getsockopt( sFd, SOL_SOCKET, SO_RCVTIMEO, &opt, &optlen ) < 0 )
	{
		SVRDBG(("Error - getsockopt failed, errno: %d\n", errno));
	}
	else
	{
		SVRDBG(("%s: opt: %d optlen: %u\n", __FUNCTION__, opt, optlen));
	}
	
	/* sysDebugPrintf("%s: Starting...\n", __FUNCTION__); */
	
	loopCount = 0;
	
	while( (status == 0) && (serverMsg != NULL) )
	{
		txCount = 0;
		rdCount = lwip_read( sFd, (char *) &serverMsg[rxCount], (REQUEST_MSG_SIZE - rxCount) );
		/* sysDebugPrintf("%s: rdCount: %d\n", __FUNCTION__, rdCount); */
		
		if ( rdCount <= 0 )
		{
/*
			SVRDBG(("Error - reading client request, errno: %d\n", errno));
			msgError++;
			status = -1;
*/
			rdCount = 0;
			
			if ( (errno > 0) && (errno != EAGAIN) )
			{
				SVRDBG(("Error - reading client request, errno: %d\n", errno));
				msgError++;
				status = -1;
			}
/*
			else
			{
				vTaskDelay( 10 );
			}
*/
		}
		else
		{
			rxCount += rdCount;
				
			if (rxCount == REQUEST_MSG_SIZE)
			{	
				rxCount = 0;
				msgRecd++;
				
				txCount = lwip_write( sFd, (char *) &serverMsg[0], REQUEST_MSG_SIZE );
				/* sysDebugPrintf("%s: txCount: %d\n", __FUNCTION__, txCount); */
	
				if ( txCount < 0 )
				{
					SVRDBG(("Error - unable to send client response, errno: %d\n", errno));
					msgError++;
					status = -1;
				}
				else if (txCount < REQUEST_MSG_SIZE)
				{
					SVRDBG(("Error - txCount %d, less than expected\n", txCount));
					msgError++;
					status = -1;
				}
				else
				{
					msgSent++;
					
					atomic32Inc( &srvMsgCount );
				}
				
				vTaskDelay( 10 );
				/* vTaskDelay( 100 ); */
			}
			else
			{
				if (rxCount > REQUEST_MSG_SIZE)
				{
					SVRDBG(("Error - message too large!!\n"));
					msgError++;
					status = -1;
				}
			}
		}
		
		loopCount++;
		
#ifdef INCLUDE_DEBUG_VGA
		sprintf( achBuffer, "CPU:%d T%02u: tcpWrk Count: %u RX: %u TX: %u ERR: %u [%04d:%04d]", 
				cpuNo, taskNum, loopCount, msgRecd, msgSent, msgError, rdCount, txCount );
		vgaPutsXY( 1, (currentLine + 1), achBuffer );
		vgaClearToEol();
#endif

	}
	
	if ( serverMsg != NULL )
	{
		mem_free( serverMsg );
	}
	else
	{
		SVRDBG(("Error - buffer allocation failed\n"));
	}
	

	SVRDBG(("tcpWrk RX: %u TX: %u ERR: %u [%04d:%04d]\n", 
				msgRecd, msgSent, msgError, rdCount, txCount));

	SVRDBG(("Client sFd: %d connection closed\n", sFd));
	
	lwip_close( sFd );
	vTaskDelete( NULL );
}


/*******************************************************************************
*
* tcpServer
*
* TCP connection server task.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void tcpServer( void *pvParams )
{
	struct sockaddr_in	serverAddr;
	struct sockaddr_in	clientAddr;
	socklen_t sockAddrSize;
	int sFd;
	int newFd;
	int i = 0;
	char workName[16];
	int cpuNo;

#ifdef INCLUDE_DEBUG_VGA
	UINT32 taskNum;
	char achBuffer[80];
	
	if ( currentLine == -1 )
	{
		currentLine = atomic32Inc( (UINT32 *) &startLine );
	}
	
	vgaClearLine( currentLine );
#endif
	
	cpuNo = sPortGetCurrentCPU();
	
#ifdef INCLUDE_DEBUG_VGA
	taskNum = uxTaskNumGet( cpuNo, NULL );
#endif

	SVRDBG(("TCP Server started, msgSize %u\n", REQUEST_MSG_SIZE));


	/* setup local address */
	sockAddrSize = sizeof( struct sockaddr_in );
	memset( (char *) &serverAddr, 0, sockAddrSize );
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons( SERVER_PORT_NUM );
	serverAddr.sin_addr.s_addr = htonl( INADDR_ANY );

	/* create a TCP socket */
	if ( (sFd = lwip_socket( AF_INET, SOCK_STREAM, 0 )) < 0 )
	{
		SVRDBG(("Error - unable to create socket\n"));
		vTaskDelete( NULL );
	}

	/* bind socket to local address */
	if ( lwip_bind( sFd, (struct sockaddr *) &serverAddr, sockAddrSize ) < 0 )
	{
		SVRDBG(("Error - unable to bind local address\n"));
		lwip_close( sFd );
		vTaskDelete( NULL );
	}

	/* create queue for client connection requests */
	if ( lwip_listen( sFd, SERVER_MAX_CONNECTIONS ) != 0 )
	{
		SVRDBG(("Error - unable to create listen queue\n"));
		lwip_close( sFd );
		vTaskDelete( NULL );
	}
	
	atomic32Set( &srvMsgCount, 0 );
	
#ifdef INCLUDE_DEBUG_VGA
	sprintf( achBuffer, "CPU:%d T%02u: tcpSvr Listening on port %d", cpuNo, taskNum, SERVER_PORT_NUM );
	vgaPutsXY( 1, currentLine, achBuffer );
	vgaClearToEol();
#else
	SVRDBG(("Listening on port %d\n", SERVER_PORT_NUM));
#endif

	/* accept net connection requests and spawn tasks to process them */
	for( ;; )
	{
		if ( (newFd = lwip_accept( sFd, (struct sockaddr *) &clientAddr, &sockAddrSize )) < 0 )
		{
			SVRDBG(("Error - failed to accept connections\n"));
			lwip_close( sFd );
			vTaskDelete( NULL );
		}

		sprintf( workName, "tcpWrk%d", i++ );

		SVRDBG(("Connection request received, starting %s task\n", workName));
		
		if( xTaskCreate( cpuNo, tcpServerWorkTask, ( signed char * ) workName, 
								TCP_TASK_STACK_SIZE, ( void * ) &newFd, 
								TCP_TASK_PRIORTY, ( xTaskHandle * ) NULL ) != pdPASS )
		
		{

			SVRDBG(("Error - failed to start worker task\n"));
			lwip_close( newFd );
		}
	}
}


/*******************************************************************************
*
* startTcpServer
*
* This function creates the TCP server task.
*
*
* RETURNS: None
* 
*******************************************************************************/
void startTcpServer( void )
{
	int cpuNo;

	
	cpuNo = sPortGetCurrentCPU();
	
	xTaskCreate( cpuNo, tcpServer, 
						( signed char * ) "tcpSvr", TCP_TASK_STACK_SIZE,
						 NULL, TCP_TASK_PRIORTY, NULL );
}


/****************************** Client routines *******************************/


/*******************************************************************************
*
* fillBuffer
*
* This function fills the given buffer with data;
*
*
* RETURNS: None
* 
*******************************************************************************/
static void fillBuffer( UINT8 *b, int len )
{
	int i;

  
  	for(i = 0; i < len; i++)
	{
		b[i] = ( i + 1 );
	}
}


/*******************************************************************************
*
* calcChecksum
*
* This function calculates the checksum of the given buffer.
*
*
* RETURNS: None
* 
*******************************************************************************/
static UINT32 calcChecksum( UINT8 *buffer, UINT32 length )
{
  	UINT32 i;
  	UINT32 checksum = 0;


  	for(i = 0; i < length; i++)
	{
		checksum += buffer[i];
	}

	/* CLIDBG(("Checksum: 0x%08x Length: %u\n", checksum, i)); */

  	return checksum;
}


/*******************************************************************************
*
* tcpClient
*
* TCP client task. Attemps to connect to the TCP server and send/receive data.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void tcpClient( void *pvParams )
{
	char *clientMsg;
	char *serverIpAddr;
	struct sockaddr_in serverAddr; 
	int sockAddrSize;
	int sFd;
	int status;
	UINT32 msgRecd;
	UINT32 msgSent;
	UINT32 msgError;
	int txCount;
	int rxCount;
	int rdCount;
	UINT32 sum1;
	UINT32 sum2;
	int opt;
	socklen_t optlen;


#ifdef INCLUDE_DEBUG_VGA
	int cpuNo;
	UINT32 taskNum;
	char achBuffer[80];
	
	if ( currentLine == -1 )
	{
		currentLine = atomic32Inc( (UINT32 *) &startLine );
	}
	
	vgaClearLine( currentLine );
#endif

	
	serverIpAddr = (char *) pvParams;
	
#ifdef INCLUDE_DEBUG_VGA
	cpuNo = sPortGetCurrentCPU();
	taskNum = uxTaskNumGet( cpuNo, NULL );
#endif
	
	clientMsg = mem_malloc( REQUEST_MSG_SIZE );
	
	if ( clientMsg == NULL )
	{
		CLIDBG(("Error - buffer allocation failed\n"));
		return;
	}
	
	CLIDBG(("TCP Client started, msgSize: %u\n", REQUEST_MSG_SIZE));

#if 1

	/* setup server address */
	sockAddrSize = sizeof( struct sockaddr_in );
	memset( (char *) &serverAddr, 0, sockAddrSize );
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons( SERVER_PORT_NUM );

	if ( (serverAddr.sin_addr.s_addr = inet_addr( serverIpAddr )) == IPADDR_NONE )
	{
		CLIDBG(("Error - unknown server [%s]\n", serverIpAddr));
		vTaskDelete( NULL );
		return;
	}

	msgError = 0;
	
#ifdef INCLUDE_DEBUG_VGA
	sprintf( achBuffer, "CPU:%d T%02u: tcpCli [%s] attempting connect...", 
			cpuNo, taskNum, serverIpAddr );
	vgaPutsXY( 1, currentLine, achBuffer );
	vgaClearToEol();
#endif
	
	do
	{
		/* create a TCP socket */
		if ( (sFd = lwip_socket( AF_INET, SOCK_STREAM, 0 )) < 0 )
		{
			CLIDBG(("Error - unable to create socket, errno: %d\n", errno));
			vTaskDelete( NULL );
			return;
		}
	
		status = lwip_connect( sFd, (struct sockaddr *) &serverAddr, sockAddrSize );
		
		if ( status < 0 )
		{
			msgError++;
			
#ifdef INCLUDE_DEBUG_VGA
			sprintf( achBuffer, "CPU:%d T%02u: tcpCli [%s] connect: %u failed, errno: %d", 
					cpuNo, taskNum, serverIpAddr, msgError, errno );
			vgaPutsXY( 1, currentLine, achBuffer );
			vgaClearToEol();
#endif

			lwip_close( sFd );
			
			/* vTaskDelay( 100 ); */
		}
	}
	while ( (tcpClientActive == TRUE) && (status != 0) );
	
#else
	/* create a TCP socket */
	if ( (sFd = lwip_socket( AF_INET, SOCK_STREAM, 0 )) < 0 )
	{
		CLIDBG(("Error - unable to create socket, errno: %d\n", errno));
		vTaskDelete( NULL );
		return;
	}

	/* setup server address */
	sockAddrSize = sizeof( struct sockaddr_in );
	memset( (char *) &serverAddr, 0, sockAddrSize );
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons( SERVER_PORT_NUM );

	if ( (serverAddr.sin_addr.s_addr = inet_addr( serverIpAddr )) == IPADDR_NONE )
	{
		CLIDBG(("Error - unknown server [%s]\n", serverIpAddr));
		lwip_close( sFd );
		vTaskDelete( NULL );
		return;
	}

	/* connect to server */
	if ( lwip_connect( sFd, (struct sockaddr *) &serverAddr, sockAddrSize ) < 0 )
	{
		CLIDBG(("Error - unable to connect to server [%s], errno: %d\n", serverIpAddr, errno));
		lwip_close( sFd );
		vTaskDelete( NULL );
		return;
	}
#endif

	/* set recv timeout (1000 ms) */
  	opt = 1000;
  	optlen = sizeof(int);
  	if ( lwip_setsockopt( sFd, SOL_SOCKET, SO_RCVTIMEO, &opt, optlen ) < 0 )
	{
		CLIDBG(("Error - setsockopt failed, errno: %d\n", errno));
	}
  	
  	opt = 0;
  	if ( lwip_getsockopt( sFd, SOL_SOCKET, SO_RCVTIMEO, &opt, &optlen ) < 0 )
	{
		CLIDBG(("Error - getsockopt failed, errno: %d\n", errno));
	}
	else
	{
		CLIDBG(("%s: opt: %d optlen: %u\n", __FUNCTION__, opt, optlen));
	}
	
	txCount = 0;
	rxCount = 0;
	status = 0;
	msgRecd = 0;
	msgSent = 0;
	msgError = 0;
	atomic32Set( &cliMsgCount, 0 );
	
#ifdef INCLUDE_DEBUG_VGA
	sprintf( achBuffer, "CPU:%d T%02u: tcpCli [%s] TX: %u RX: %u ERR: %u [%04d:%04d]", 
			cpuNo, taskNum, serverIpAddr, msgSent, msgRecd, msgError, txCount, rxCount );
	vgaPutsXY( 1, currentLine, achBuffer );
	vgaClearToEol();
#endif
	
	while ( (tcpClientActive == TRUE) && (status == 0) )
	{
		fillBuffer( (UINT8 *) &clientMsg[0], REQUEST_MSG_SIZE );
		sum1 = calcChecksum( (UINT8 *) &clientMsg[0], REQUEST_MSG_SIZE );

		/* send request to server */
		txCount = lwip_write( sFd, (char *) &clientMsg[0], REQUEST_MSG_SIZE );
		if ( txCount < 0 )
		{
			CLIDBG(("Error - unable to send request, errno: %d\n", errno));
			msgError++;
			status = -1;
		}
		else if (txCount < REQUEST_MSG_SIZE)
		{
			CLIDBG(("Error - txCount %d, less than expected\n", txCount));
			msgError++;
			status = -1;
		}
		else
		{
			msgSent++;
			rxCount = 0;
						
			memset( (void *) &clientMsg[0], 0, REQUEST_MSG_SIZE);

			while ( (status == 0) && (rxCount != REQUEST_MSG_SIZE) )
			{
				if (rxCount < REQUEST_MSG_SIZE)
				{
					rdCount = lwip_read( sFd, (char *) &clientMsg[rxCount], (REQUEST_MSG_SIZE - rxCount) );
					if ( rdCount <= 0 )
					{
/*
						CLIDBG(("Error - reading response from server [%s], errno: %d\n", serverIpAddr, errno));
						msgError++;
						status = -1;
*/
						rdCount = 0;
			
						if ( (errno > 0) && (errno != EAGAIN) )
						{
							CLIDBG(("Error - reading response from server [%s], errno: %d\n", serverIpAddr, errno));
							msgError++;
							status = -1;
						}
					}
					else
					{
						rxCount += rdCount;
					}
				}
				else
				{
					CLIDBG(("Error - message too large!!\n"));
					msgError++;
					status = -1;
				}
			}

			if (status == 0)
			{
				sum2 = calcChecksum( (UINT8 *) &clientMsg[0], rxCount );

				if ( sum1 != sum2 )
				{
					msgError++;
				}
				else
				{
					msgRecd++;
					
					atomic32Inc( &cliMsgCount );
				}
			}
			
			/* if (verboseMode) */
			{
#ifdef INCLUDE_DEBUG_VGA
				sprintf( achBuffer, "CPU:%d T%02u: tcpCli [%s] TX: %u RX: %u ERR: %u [%04d:%04d]", 
						cpuNo, taskNum, serverIpAddr, msgSent, msgRecd, msgError, txCount, rxCount );
				vgaPutsXY( 1, currentLine, achBuffer );
				vgaClearToEol();
#endif
			}
		}

		vTaskDelay( 10 );
	}
	
	CLIDBG(("\ntcpCli [%s] TX: %u RX: %u ERR: %u [%04d:%04d]\n", 
				serverIpAddr, msgSent, msgRecd, msgError, txCount, rxCount));

	if ( clientMsg != NULL )
	{
		mem_free( clientMsg );
	}
	
	lwip_close( sFd );
	tcpClientActive = FALSE;
	vTaskDelete( NULL );
}


/*******************************************************************************
*
* startTcpClient
*
* This function creates the TCP client task.
*
* Usage: startTcpClient( "193.67.84.253" );
*
* RETURNS: None
* 
*******************************************************************************/
void startTcpClient( void )
{
	int cpuNo;
	char *serverIpAddr = "193.67.84.253";
	
	if (tcpClientActive == FALSE)
	{
		cpuNo = sPortGetCurrentCPU();
		
		tcpClientActive = TRUE;
		
		if( xTaskCreate( cpuNo, tcpClient, 
					( signed char * ) "tcpCli", TCP_TASK_STACK_SIZE,
					 (void *) serverIpAddr, TCP_TASK_PRIORTY, NULL ) != pdPASS )	
		{
	
			CLIDBG(("Error - failed to start TCP client task\n"));
		}
	}
}


/*******************************************************************************
*
* stopTcpClient
*
* This function stops the TCP client task.
*
*
* RETURNS: None
* 
*******************************************************************************/
void stopTcpClient( void )
{
	tcpClientActive = FALSE;
}

