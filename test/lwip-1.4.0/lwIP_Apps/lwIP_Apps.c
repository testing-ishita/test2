/*******************************************************************************
*
* Filename:	 lwIP_Apps.c
*
* Description:	lwIP Applications source file, modified for CCT CUTE.
*
* $Revision: 1.6 $
*
* $Date: 2015-01-29 10:35:56 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/lwIP_Apps/lwIP_Apps.c,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

/*
    FreeRTOS V7.0.2 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <private/debug.h>

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/tcpip.h"
#include "lwip/stats.h"
#include "lwip/mem.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif

#include "./utils/CommandInterpreter.h"

#include "lwIP_Apps.h"

#include "config.h"
#include "../debug/dbgLog.h"

#undef LWIP_APPS_DEBUG

#ifdef LWIP_APPS_DEBUG /* sysDebugPrintf */
#define APPSDBG(x)	do {DBLOG x;} while(0) 
#else
#define APPSDBG(x)
#endif

#ifdef INCLUDE_LWIP_COMMAND_SERVER
#warning "***** LWIP COMMAND SERVER INCLUDED *****"
#endif

#if defined(LWIP_DEBUG) || defined(LWIP_APPS_DEBUG)
#warning "***** DEBUG ON *****"
#endif

#ifdef LWIP_INCLUDE_ASSERT
#warning "***** LWIP ASSERTIONS ON *****"
#endif

#if !(LWIP_DHCP)
#warning "***** STATIC IP CONFIGURATION *****"
#endif


/* Dimensions the cTxBuffer array - which is itself used to hold replies from 
command line commands.  cTxBuffer is a shared buffer, so protected by the 
xTxBufferMutex mutex. */
#define lwipappsTX_BUFFER_SIZE	1024

/* The maximum time to block waiting to obtain the xTxBufferMutex to become
available. */
#define lwipappsMAX_TIME_TO_WAIT_FOR_TX_BUFFER_MS	( 100 / portTICK_RATE_MS )

static struct netif xNetIf[LWIP_APPS_MAX_DEVICES];

/* lwip statistics */
extern struct stats_ lwip_stats;

extern void startTcpServer( void );
extern void stopTcpClient( void );

/*-----------------------------------------------------------*/

/*
 * The function that implements the lwIP based sockets command interpreter
 * server.
 */
extern void vBasicSocketsCommandInterpreterTask( void *pvParameters );

/*
 * Network interface initialization function.
 */
extern err_t geifInit( struct netif *netif );

/*
 * GE network interface statistics functions.
 */
extern size_t geifGetStats( char *pName, signed char *pcWriteBuffer, size_t xWriteBufferLen );

/*
 * SM Client statistics functions.
 */
extern size_t smcGetStats( signed char *pcWriteBuffer, size_t xWriteBufferLen );


/*-----------------------------------------------------------*/
#if 0 /* Not used */
/* Semaphore used to guard the Tx buffer. */
static xSemaphoreHandle xTxBufferMutex = NULL;

/* The Tx buffer itself.  This is used to hold the text generated by the 
execution of command line commands, and (hopefully) the execution of 
server side include callbacks.  It is a shared buffer so protected by the
xTxBufferMutex mutex.  pcLwipAppsBlockingGetTxBuffer() and 
vLwipAppsReleaseTxBuffer() are provided to obtain and release the 
xTxBufferMutex respectively.  pcLwipAppsBlockingGetTxBuffer() must be used with
caution as it has the potential to block. */
static signed char cTxBuffer[ lwipappsTX_BUFFER_SIZE ];
#endif

#ifdef INCLUDE_LWIP_COMMAND_SERVER

/*-----------------------------------------------------------*/

static portBASE_TYPE prvRunTimeStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	// configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of task stats. */
	vTaskList( pcWriteBuffer );

	/* There is no more data to return after this single string, so return pdFALSE. */
	return pdFALSE;
}

/* Structure that defines the "stats" command line command. */
static const xCommandLineInput xRunTimeStats =
{
	(signed char *) "tasklist",
	(signed char *) "tasklist : FreeRTOS task list\r\n",
	prvRunTimeStatsCommand,
};


/*-----------------------------------------------------------*/
static void getLwipProtoStats( signed char *pcWriteBuffer, struct stats_proto *proto, 
							char *name )
{
	size_t len;
	
	
	pcWriteBuffer[0] = 0;
	sprintf( (char *) pcWriteBuffer, "\r\n%s Statistics\r\n", name );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "xmit    : %u\r\n", proto->xmit);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "recv    : %u\r\n", proto->recv);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "fw      : %u\r\n", proto->fw);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "drop    : %u\r\n", proto->drop);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "chkerr  : %u\r\n", proto->chkerr);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "lenerr  : %u\r\n", proto->lenerr);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "memerr  : %u\r\n", proto->memerr);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "rterr   : %u\r\n", proto->rterr);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "proterr : %u\r\n", proto->proterr);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "opterr  : %u\r\n", proto->opterr);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "err     : %u\r\n", proto->err);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "cachehit: %u\r\n", proto->cachehit);
}

/*-----------------------------------------------------------*/
static portBASE_TYPE prvTcpStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	// configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of statistics. */
	getLwipProtoStats( pcWriteBuffer, &lwip_stats.tcp, "TCP" );

	/* There is no more data to return after this single string, so return pdFALSE. */
	return pdFALSE;
}

/* Structure that defines the "tcpstats" command line command. */
static const xCommandLineInput xTcpStats =
{
	(signed char *) "tcpstats",
	(signed char *) "tcpstats : TCP statistics\r\n",
	prvTcpStatsCommand,
};


/*-----------------------------------------------------------*/
static portBASE_TYPE prvIpStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	// configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of statistics. */
	getLwipProtoStats( pcWriteBuffer, &lwip_stats.ip, "IP" );

	/* There is no more data to return after this single string, so return pdFALSE. */
	return pdFALSE;
}

/* Structure that defines the "ipstats" command line command. */
static const xCommandLineInput xIpStats =
{
	(signed char *) "ipstats",
	(signed char *) "ipstats  : IP statistics\r\n",
	prvIpStatsCommand,
};


/*-----------------------------------------------------------*/
static portBASE_TYPE prvEthArpStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	// configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of statistics. */
	getLwipProtoStats( pcWriteBuffer, &lwip_stats.etharp, "Ethernet ARP" );

	/* There is no more data to return after this single string, so return pdFALSE. */
	return pdFALSE;
}

/* Structure that defines the "etharpstats" command line command. */
static const xCommandLineInput xEthArpStats =
{
	(signed char *) "arpstats",
	(signed char *) "arpstats : Ethernet ARP statistics\r\n",
	prvEthArpStatsCommand,
};


/*-----------------------------------------------------------*/
static portBASE_TYPE prvEthLinkStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	// configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	/* Generate a table of statistics. */
	getLwipProtoStats( pcWriteBuffer, &lwip_stats.link, "Ethernet Link" );

	/* There is no more data to return after this single string, so return 
	pdFALSE. */
	return pdFALSE;
}

/* Structure that defines the "ethlinkstats" command line command. */
static const xCommandLineInput xEthLinkStats =
{
	(signed char *) "linkstats",
	(signed char *) "linkstats: Ethernet link statistics\r\n",
	prvEthLinkStatsCommand,
};


/*-----------------------------------------------------------*/
portBASE_TYPE netStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	static int ifNo = 0;
	char ifname[4];
	
	
	// configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;
	pcWriteBuffer[0] = 0;
	
	if (ifNo > 4)
	{
		ifNo = 0;
	}
	
	sprintf( ifname, "ge%d", ifNo );
	
	if ( geifGetStats( ifname, pcWriteBuffer, xWriteBufferLen ) )
	{
		ifNo++;
		return pdTRUE;
	}
	else
	{
		ifNo = 0;
		return pdFALSE;
	}
}


/* Structure that defines the "netstats" command line command. */
static const xCommandLineInput xNetStats =
{
	(signed char *) "netstats",
	(signed char *) "netstats : Network statistics\r\n",
	netStatsCommand,
};


/*-----------------------------------------------------------*/
static size_t getLwipMemRegionStats( signed char *pcWriteBuffer, struct stats_mem *mem, char *name )
{
	size_t len;
	
	sprintf( (char *) pcWriteBuffer, "\r\nlwIP %s Memory\r\n", name );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "available: %u\r\n", mem->avail);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "in use   : %u\r\n", mem->used);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "maximum  : %u\r\n", mem->max);
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "errors   : %u\r\n", mem->err);
	len = strlen( (char *) pcWriteBuffer );
	
	return len;
}

static size_t getLwipSystemStats( signed char *pcWriteBuffer, struct stats_sys *sys )
{
	size_t len;
	
	sprintf( (char *) pcWriteBuffer, "\r\nlwIP System Info\r\n" );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "sem.used  : %u\r\n", sys->sem.used );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "sem.max   : %u\r\n", sys->sem.max );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "sem.err   : %u\r\n", sys->sem.err );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "mutex.used: %u\r\n", sys->mutex.used );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "mutex.max : %u\r\n", sys->mutex.max );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "mutex.err : %u\r\n", sys->mutex.err );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "mbox.used : %u\r\n", sys->mbox.used );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "mbox.max  : %u\r\n", sys->mbox.max );
	len = strlen( (char *) pcWriteBuffer );
	sprintf( (char *) &pcWriteBuffer[len], "mbox.err  : %u\r\n", sys->mbox.err );
	len = strlen( (char *) pcWriteBuffer );
	
  	return len;
}

static portBASE_TYPE prvLwipMemStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	size_t len;
  	char * memp_names[MEMP_MAX] = {
#define LWIP_MEMPOOL(name,num,size,desc) (desc),
#include "lwip/memp_std.h"
  	};
  
	static int poolNo = 0;
	
	// configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;

	pcWriteBuffer[0] = 0;
	
	/* Get a table of statistics. */
	if ( poolNo < MEMP_MAX ) 
	{
		getLwipMemRegionStats( pcWriteBuffer, &lwip_stats.memp[poolNo], memp_names[poolNo] );
		poolNo++;
		
		/* More data to return */
		return pdTRUE;
	}
	else
	{
		len = getLwipMemRegionStats( pcWriteBuffer, &lwip_stats.mem, "HEAP" );
		getLwipSystemStats( &pcWriteBuffer[len], &lwip_stats.sys );
		
		/* No more data to return */
		poolNo = 0;
		return pdFALSE;
	}
}

/* Structure that defines the "lwipmemstats" command line command. */
static const xCommandLineInput xLwipMemStats =
{
	(signed char *) "lwipstats",
	(signed char *) "lwipstats: lwIP memory/system statistics\r\n",
	prvLwipMemStatsCommand,
};

/*-----------------------------------------------------------*/
#ifdef INCLUDE_LWIP_SMC
static portBASE_TYPE smcStatsCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	// configASSERT( pcWriteBuffer );

	/* This function assumes the buffer length is adequate. */
	( void ) xWriteBufferLen;
	
	smcGetStats( pcWriteBuffer, xWriteBufferLen );
	
	/* No more data to return */
	return pdFALSE;
}


/* Structure that defines the "smcstats" command line command. */
static const xCommandLineInput xSmcStats =
{
	(signed char *) "smcstats",
	(signed char *) "smcstats : SM client statistics\r\n",
	smcStatsCommand,
};
#endif /* INCLUDE_LWIP_SMC */

#endif /* INCLUDE_LWIP_COMMAND_SERVER */

/*-----------------------------------------------------------*/

#ifdef INCLUDE_DBGLOG
static portBASE_TYPE dbgLogPrintCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	portBASE_TYPE result;
	static UINT32 logEntry = 0;
	
	
	pcWriteBuffer[0] = 0;
	
	if (dbgLogGetEntry( (char * ) pcWriteBuffer, xWriteBufferLen, logEntry ) == DBLOG_OK)
	{	
		logEntry++;
		
		if ( logEntry < MAX_DBG_LOG_ENTRIES)
		{
			result = pdTRUE;
		}
		else
		{
			result = FALSE;
		}
	}
	else
	{
		/* No more data to return */
		result = FALSE;
	}
	
	if (result == FALSE)
	{
		logEntry = dbgLogGetNextEntry();
		
		sprintf( (char *) pcWriteBuffer, "Next entry: %u\n", logEntry);
		
		logEntry = 0;
	}
	
	return result;
}

/* Structure that defines the "dblp" command line command. */
static const xCommandLineInput xDbgLogPrint =
{
	(signed char *) "dblp",
	(signed char *) "dblp     : print debug log\r\n",
	dbgLogPrintCommand,
};

static portBASE_TYPE dbgLogClearCommand( signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	
	dbgLogClear();	
	sprintf( (char *) pcWriteBuffer, "Debug log cleared\n");
		
	return pdFALSE;
}

/* Structure that defines the "dblc" command line command. */
static const xCommandLineInput xDbgLogClear =
{
	(signed char *) "dblc",
	(signed char *) "dblc     : clear debug log\r\n",
	dbgLogClearCommand,
};
#endif

/*-----------------------------------------------------------*/


/* Called from the TCP/IP thread. */
void lwIPAppsInit( void *pvArgument )
{
	ETH_DEV_PARAMS *pParams;
	ETH_DEV *pEthDev;
	int i;

	
	/* Use state to pass arguments onto the driver */
	pParams = (ETH_DEV_PARAMS *) pvArgument;
	
	if ((pParams != NULL) && (pParams->num > 0))
	{	
		for ( i = 0; i < pParams->num; i++ )
		{
			pEthDev = &pParams->devices[i];
			
#if LWIP_DHCP
			/* We're using DHCP so clear our addresses */
    		IP4_ADDR(&pEthDev->ipAddr, 0,0,0,0);
    		IP4_ADDR(&pEthDev->netMask, 0,0,0,0);
    		IP4_ADDR(&pEthDev->gateway, 0,0,0,0);
#endif

			/* Start the network interface */
			APPSDBG(("Starting network interface %d: 0x%04x:0x%04x:%u IP: %s\n", 
							i, pEthDev->vid, pEthDev->did, pEthDev->instance, 
							ip_ntoa(&pEthDev->ipAddr)));
							
			netif_add( &xNetIf[i], &pEthDev->ipAddr, &pEthDev->netMask, &pEthDev->gateway, 
						(void *) pEthDev, geifInit, tcpip_input );
							
#if LWIP_DHCP
			/* Start DHCP negotiation for this network interface */
		    dhcp_start( &xNetIf[i] );
#else
			/* We're using a static configuration for this network interface */
			netif_set_up( &xNetIf[i] );
#endif
		}
		
		/* Set first network interface as the default */			
		netif_set_default( &xNetIf[0] );

#if 0
		/* Create the mutex used to ensure mutual */
		/* exclusive access to the Tx buffer. */
		xTxBufferMutex = xSemaphoreCreateMutex();
		/* configASSERT( xTxBufferMutex ); */

		if (xTxBufferMutex == NULL)
		{
			APPSDBG(("%s: Error - failed to create mutex\n", __FUNCTION__));
		}
#endif

#ifdef INCLUDE_LWIP_COMMAND_SERVER
		{	
			int cpuNo;
			
			cpuNo = sPortGetCurrentCPU();
			
			/* Create the FreeRTOS defined basic command server. */
			/* This demonstrates use of the lwIP sockets API. */
			
			/* Register the commands, this must be done before using */
			/* the xCmdIntProcessCommand() function */
			xCmdIntRegisterCommand( &xRunTimeStats );
			xCmdIntRegisterCommand( &xTcpStats );
			xCmdIntRegisterCommand( &xIpStats );
			xCmdIntRegisterCommand( &xEthArpStats );
			xCmdIntRegisterCommand( &xEthLinkStats );
			xCmdIntRegisterCommand( &xNetStats );
			xCmdIntRegisterCommand( &xLwipMemStats );
#ifdef INCLUDE_LWIP_SMC
			xCmdIntRegisterCommand( &xSmcStats );
#endif

#ifdef INCLUDE_DBGLOG
			xCmdIntRegisterCommand( &xDbgLogPrint );
			xCmdIntRegisterCommand( &xDbgLogClear );
#endif
			
			/* Create the command server */
			xTaskCreate( cpuNo, vBasicSocketsCommandInterpreterTask, 
						( signed char * ) "cmdSvr", configMINIMAL_STACK_SIZE, NULL, 5, NULL );
						 
			
			/* Create the TCP server. */
			/* startTcpServer(); */
		}
#endif /* INCLUDE_LWIP_COMMAND_SERVER */	 

	}
	else
	{
		APPSDBG(("%s: Error - Ethernet device not configured\n", __FUNCTION__));
	}
}


/*-----------------------------------------------------------*/


/* Called from the smcClient interface to obtain IP address allocated by DHCP server */ 
unsigned int lwIPAppsIpAddr( void )
{
	return ((unsigned int)xNetIf[0].ip_addr.addr);
}


/*-----------------------------------------------------------*/


/* Called from the smcClient interface to obtain device MAC address */ 
unsigned char * lwIPAppsMacAddr( void )
{
	return ((unsigned char *) &xNetIf[0].hwaddr[0]);
}


/*-----------------------------------------------------------*/

#if 0 /* Not used */

signed char *pcLwipAppsBlockingGetTxBuffer( void )
{
signed char *pcReturn;

	/* Attempt to obtain the semaphore that guards the Tx buffer. */
	if( xSemaphoreTakeRecursive( xTxBufferMutex, lwipappsMAX_TIME_TO_WAIT_FOR_TX_BUFFER_MS ) == pdFAIL )
	{
		/* The semaphore could not be obtained before timing out. */
		pcReturn = NULL;
	}
	else
	{
		/* The semaphore was obtained successfully.  Return a pointer to the
		Tx buffer. */
		pcReturn = cTxBuffer;
	}

	return pcReturn;
}
/*-----------------------------------------------------------*/

void vLwipAppsReleaseTxBuffer( void )
{
	/* Finished with the Tx buffer.  Return the mutex. */
	xSemaphoreGiveRecursive( xTxBufferMutex );
}
#endif
