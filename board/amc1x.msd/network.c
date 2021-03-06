/*******************************************************************************
*
* Filename:	 network.c
*
* Description:	Network initialization functions.
*
* $Revision: 1.4 $
*
* $Date: 2014-11-04 15:48:07 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/board/amc1x.msd/network.c,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/


#include <stdtypes.h>
#include <private/debug.h>
#include <bit/board_service.h>
#include <services.h>

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
	
	if (bGetTestHandler() == TH__MTH)
	{
		// No network, so don't initialise any interfaces
		return;
	}
	geifGetMemInfo( &addr, &size );
	NETDBG(("geif mem:  Pool: 0x%08x-0x%08x  size: %u bytes\n", addr, (addr + size - 1), size));
	
	/* Setup the network interface configuration     */
	/* Note: if DHCP is configured, it will override */
	/* the IP address settings made here              */		
			
	memset( &ethDevParms, 0, sizeof(ETH_DEV_PARAMS) );

#if 0
	if(board_service(SERVICE__BRD_CHECK_DUAL_SRIO, NULL, NULL) == E__OK)
	{
		ethDevParms.num = 2;					/* Number of devices to start */
	}
	else
	{
		ethDevParms.num = 1;					/* Number of devices to start */
	}
#endif

	// board can have either or both of the two TSI721s fitted
	// so for networking assume that both are fitted
	// both i210 controllers are always fitted
	ethDevParms.num = 2;					/* Number of devices to start */

	ethDevParms.devices[0].vid = 0x8086;
	ethDevParms.devices[0].did = 0x1537;	/* i210 SERDES */
	ethDevParms.devices[0].irqNo = 18;
	ethDevParms.devices[0].instance = 1;
	IP4_ADDR((&ethDevParms.devices[0].ipAddr), 192,67,84,248);
	IP4_ADDR((&ethDevParms.devices[0].netMask), 255,255,255,0);
	IP4_ADDR((&ethDevParms.devices[0].gateway), 192,67,84,21);
	
	ethDevParms.devices[1].vid = 0x8086;
	ethDevParms.devices[1].did = 0x1537;	/* i210 SERDES */
	ethDevParms.devices[1].irqNo = 19;
	ethDevParms.devices[1].instance = 2;
	IP4_ADDR((&ethDevParms.devices[1].ipAddr), 192,67,84,249);
	IP4_ADDR((&ethDevParms.devices[1].netMask), 255,255,255,0);
	IP4_ADDR((&ethDevParms.devices[1].gateway), 192,67,84,21);
	
	ethDevParms.devices[2].vid = 0x8086;
	ethDevParms.devices[2].did = 0x1528;	/* x540 Copper */
	ethDevParms.devices[2].irqNo = 16;
	ethDevParms.devices[2].instance = 1;
	IP4_ADDR((&ethDevParms.devices[2].ipAddr), 192,67,84,250);
	IP4_ADDR((&ethDevParms.devices[2].netMask), 255,255,255,0);
	IP4_ADDR((&ethDevParms.devices[2].gateway), 192,67,84,21);

	ethDevParms.devices[3].vid = 0x8086;
	ethDevParms.devices[3].did = 0x1528;	/* x540 Copper */
	ethDevParms.devices[3].irqNo = 17;
	ethDevParms.devices[3].instance = 2;
	IP4_ADDR((&ethDevParms.devices[3].ipAddr), 192,67,84,251);
	IP4_ADDR((&ethDevParms.devices[3].netMask), 255,255,255,0);
	IP4_ADDR((&ethDevParms.devices[3].gateway), 192,67,84,21);

	/* Creates the TCP/IP thread and start the interfaces */
	tcpip_init( 1, lwIPAppsInit, (void *) &ethDevParms );
#endif
}
