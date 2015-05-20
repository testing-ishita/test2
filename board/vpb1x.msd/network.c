/*******************************************************************************
*
* Filename:	 network.c
*
* Description:	Network initialization functions.
*
* $Revision: 1.1 $
*
* $Date: 2015-02-25 17:50:45 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/network.c,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/


#include <stdtypes.h>
#include <private/debug.h>
#include <private/port_io.h>
#include <bit/pci.h>
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
	PCI_PFA	pfa;
	UINT16	wGPIOBase;
	UINT32  gtemp;
	u32_t addr;
	u32_t size;

	

		if (bGetTestHandler() == TH__MTH)
		{
			// No network, so don't initialise any interfaces
			return;
		}

		// Read GPIO 35 (RC Variant) - to determine which interface to use
		// 0 = Not RC (i210), 1 = RC (i350)

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
	
	ethDevParms.num = 1;					/* Number of devices to start */


	// check GPIO35 for RC variant
	if ((gtemp & 0x00000010) == 0x00)
	{
		ethDevParms.devices[0].vid = 0x8086;
		ethDevParms.devices[0].did = 0x1521;	// I350 Copper
		ethDevParms.devices[0].irqNo = 16;
		ethDevParms.devices[0].instance = 1;
		IP4_ADDR((&ethDevParms.devices[0].ipAddr), 192,67,84,250);
		IP4_ADDR((&ethDevParms.devices[0].netMask), 255,255,255,0);
		IP4_ADDR((&ethDevParms.devices[0].gateway), 192,67,84,21);
	}
	else
	{
		ethDevParms.devices[0].vid = 0x8086;
		ethDevParms.devices[0].did = 0x1521;	// I350 Copper
		ethDevParms.devices[0].irqNo = 17;
		ethDevParms.devices[0].instance = 2;
		IP4_ADDR((&ethDevParms.devices[0].ipAddr), 192,67,84,250);
		IP4_ADDR((&ethDevParms.devices[0].netMask), 255,255,255,0);
		IP4_ADDR((&ethDevParms.devices[0].gateway), 192,67,84,21);
	}


	/* Creates the TCP/IP thread and start the interfaces */
	tcpip_init( 1, lwIPAppsInit, (void *) &ethDevParms );
#endif

}
