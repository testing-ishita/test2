/*******************************************************************************
*
* Filename:	 network.c
*
* Description:	Network initialization functions.
*
* $Revision: 1.2 $
*
* $Date: 2015-04-01 16:44:29 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/network.c,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/


#include <stdtypes.h>
#include <private/debug.h>
#include <bit/bit.h>

#if defined(CUTE)
#include "config.h"

#ifdef INCLUDE_NETWORK
/* lwIP includes */
#include "lwip/ip_addr.h"
#include "lwIP_Apps.h"
#endif

#include "../debug/dbgLog.h"

#undef NET_INIT_DEBUG		/* define to include debug messages */

#ifdef NET_INIT_DEBUG
#warning "***** DEBUG ON *****"
						/* sysDebugPrintf */
#define NETDBG(x)	do {DBLOG x;} while(0)
#else
#define NETDBG(x)
#endif

#ifdef INCLUDE_NETWORK
extern void geifGetMemInfo( u32_t *pAddr, u32_t *pSize );
extern void tcpip_init( int cpuNo, void (*tcpip_init_done_fn), void *arg);
#endif

#endif /* CUTE */



/*******************************************************************************
*
* networkInit
*
* Initializes the network interface configuration and creates the TCP/IP thread.
*
*
* RETURNS: None
* 
*******************************************************************************/
void networkInit( void )
{
#if defined(CUTE) && defined (INCLUDE_NETWORK)
	static ETH_DEV_PARAMS ethDevParms;
	
	u32_t addr;
	u32_t size;
	
	NETDBG(("lwIP Initializing...\n"));
	
	geifGetMemInfo( &addr, &size );
	NETDBG(("geif mem:  Pool: 0x%08x-0x%08x  size: %u bytes\n", addr, (addr + size - 1), size));
	
	/* Setup the network interface configuration     */
	/* Note: if DHCP is configured, it will override */
	/* the IP address settings made here              */		
			
	memset( &ethDevParms, 0, sizeof(ETH_DEV_PARAMS) );
	
	ethDevParms.num = 1;					/* Number of devices to start */
			
	ethDevParms.devices[0].vid = 0x8086;
	ethDevParms.devices[0].did = 0x1521;	/* 82579LM */
	ethDevParms.devices[0].irqNo = 18;
	ethDevParms.devices[0].instance = 1;
	IP4_ADDR((&ethDevParms.devices[0].ipAddr), 192,67,84,250);
	IP4_ADDR((&ethDevParms.devices[0].netMask), 255,255,255,0);
	IP4_ADDR((&ethDevParms.devices[0].gateway), 192,67,84,21);
	

	/* Creates the TCP/IP thread and start the interfaces */
	tcpip_init( 1, lwIPAppsInit, (void *) &ethDevParms );
#endif
}
