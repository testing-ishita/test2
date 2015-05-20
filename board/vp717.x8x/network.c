/*******************************************************************************
*
* Filename:	 network.c
*
* Description:	Network initialization functions.
*
* $Revision: 1.1 $
*
* $Date: 2015-04-02 11:41:18 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/board/vp717.x8x/network.c,v $
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
#include <bit/bit.h>
/* lwIP includes */
#include "lwip/ip_addr.h"
#include "lwIP_Apps.h"
#include "../debug/dbgLog.h"
#endif


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
	PCI_PFA	pfa;
	UINT16	wGPIOBase;
	UINT32  gtemp;

	// Read GPIO 48 (RC Variant) - to determine which interface to use
	// 0 = Not RC , 1 = RC

	pfa = PCI_MAKE_PFA (0, 31, 0);
	wGPIOBase = 0xFFFE & PCI_READ_WORD (pfa, 0x48);
	gtemp = sysInPort32 (wGPIOBase + 0x48);

	NETDBG(("lwIP Initializing...\n"));
	
	geifGetMemInfo( &addr, &size );
	NETDBG(("geif mem:  Pool: 0x%08x-0x%08x  size: %u bytes\n", addr, (addr + size - 1), size));
	
	/* Setup the network interface configuration     */
	/* Note: if DHCP is configured, it will override */
	/*       the IP address settings made here       */
			
	memset( &ethDevParms, 0, sizeof(ETH_DEV_PARAMS) );

	// enable the required controller
	if ((gtemp & 0x00010000) == 0x00)
	{
		ethDevParms.devices[0].instance = 2;
		ethDevParms.devices[0].irqNo = 17;
	}
	else
	{
		ethDevParms.devices[0].instance = 1;
		ethDevParms.devices[0].irqNo = 16;
	}

	ethDevParms.num = 1;					/* Number of devices to start */

	ethDevParms.devices[0].vid = 0x8086;
	ethDevParms.devices[0].did = 0x10C9;	// 82576EB Copper
	IP4_ADDR((&ethDevParms.devices[0].ipAddr), 192,168,0,248); // 192,67,84,250);
	IP4_ADDR((&ethDevParms.devices[0].netMask), 255,255,255,0);
	IP4_ADDR((&ethDevParms.devices[0].gateway), 192,168,0,1); // 192,67,84,21);
/*
	ethDevParms.devices[1].vid = 0x8086;
	ethDevParms.devices[1].did = 0x10C9;	// 82576EB Copper
	ethDevParms.devices[1].irqNo = 16;
	ethDevParms.devices[1].instance = 1;
	IP4_ADDR((&ethDevParms.devices[1].ipAddr), 192,67,84,250);
	IP4_ADDR((&ethDevParms.devices[1].netMask), 255,255,255,0);
	IP4_ADDR((&ethDevParms.devices[1].gateway), 192,67,84,21);
*/

	/* Creates the TCP/IP thread and start the interfaces */
	tcpip_init( 1, lwIPAppsInit, (void *) &ethDevParms );
#endif

}
