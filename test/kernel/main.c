
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

/* MAIN.C - C-code entry point
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/main.c,v 1.18 2015-03-12 15:28:40 hchalla Exp $
 *
 * $Log: main.c,v $
 * Revision 1.18  2015-03-12 15:28:40  hchalla
 * CUTE main stack size increased to 2K.
 *
 * Revision 1.17  2015-02-27 11:03:28  mgostling
 * Initialise VME bridge after PCI bus enumeration
 *
 * Revision 1.16  2015-01-21 17:54:35  gshirasat
 * Flushed the TLB before starting the scheduler.
 *
 * Revision 1.15  2014-11-11 12:19:33  mgostling
 * Add conditional around vGetUSMIPAddrs to stop it being included in BIT builds
 *
 * Revision 1.14  2014-11-10 09:37:05  mgostling
 * Add new service to verify RTC has reset properly and resolve issue if not.
 *
 * Revision 1.13  2014-10-09 09:56:15  chippisley
 * Reinstated call to rio_init_mports().
 *
 * Revision 1.12  2014-10-03 14:54:05  chippisley
 * Added INCLUDE_USM_IP_ADDRESS conditional.
 *
 * Revision 1.11  2014-09-19 09:22:56  mgostling
 * Add support for two TSI721 SRIO devices
 *
 * Revision 1.8.2.1  2014-06-04 15:47:47  hchalla
 * Added support for Dual SRIO scanning and configuring from main function.
 *
 * Revision 1.10  2014-06-13 10:30:43  mgostling
 * Resolve compiler warnings
 *
 * Revision 1.9  2014-05-12 16:10:15  jthiru
 * Adding support for VX91x and fixes to existing bugs
 *
 * Revision 1.8  2014-03-06 13:36:12  hchalla
 * SriopreInit for SRIO instance 2 is removed.
 *
 * Revision 1.7  2014-02-14 11:42:50  chippisley
 * Added startup dianostic messages.
 *
 * Revision 1.6  2014/01/29 13:26:15  hchalla
 * Added support for SRIO interface initialization for AM 94x.
 *
 * Revision 1.5  2013-11-25 11:14:14  mgostling
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings for MAKE_BIT build.
 *
 * Revision 1.4  2013-10-08 07:09:46  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.3  2013/09/26 14:37:38  chippisley
 * Fixed compiler warning for unused variable.
 *
 * Revision 1.2  2013/09/26 12:45:14  chippisley
 * Added support for network Soak Master Client.
 * Removed obsolete HAL
 *
 * Revision 1.1  2013/09/04 07:41:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.46  2012/11/02 14:08:28  chippisley
 * SMP changes, revised demo tasks (debug mode).
 *
 * Revision 1.45  2012/03/21 15:27:55  cdobson
 * Added some casts to eliminate compiler warnings.
 *
 * Revision 1.44  2012/02/17 15:21:03  hchalla
 * Changed Copyright String to 2012 from 2011.
 *
 * Revision 1.43  2012/02/17 11:32:58  hchalla
 * Added support for PP 81x and debug information from serial port.
 *
 * Revision 1.42  2011/12/02 11:18:33  hchalla
 * COM2 Debug is enabled before PBIT is called, from board service function.
 *
 * Revision 1.41  2011/11/21 11:16:47  hmuneer
 * no message
 *
 * Revision 1.40  2011/11/14 17:33:32  hchalla
 * Updated for PP 531.
 *
 * Revision 1.39  2011/06/13 12:33:14  hmuneer
 * no message
 *
 * Revision 1.38  2011/06/07 14:57:52  hchalla
 * Removed signon messages from BIOS and is moved to bitid command display.
 *
 * Revision 1.37  2011/05/16 14:31:33  hmuneer
 * Info Passing Support
 *
 * Revision 1.36  2011/04/05 13:07:43  hchalla
 * Added board service to detect the board is in Syscon or Peripheral.
 *
 * Revision 1.35  2011/03/22 13:45:52  hchalla
 * Added new board service for configuring VXS ports.
 *
 * Revision 1.34  2011/02/28 11:58:40  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.33  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.32  2010/11/05 09:43:48  hchalla
 * Added flags check for building CUTE and BIT for cpci backplane and vpx backplane.
 *
 * Revision 1.31  2010/11/04 17:38:03  hchalla
 * Added Debug statements.
 *
 * Revision 1.30  2010/10/08 12:15:16  jthiru
 * Fix -- hard coded values removed
 *
 * Revision 1.29  2010/09/15 12:21:55  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.28  2010/09/10 10:15:31  cdobson
 * Added brdPostScanInit function.
 *
 * Revision 1.27  2010/07/26 13:14:10  hmuneer
 * Source Cleanup
 *
 * Revision 1.26  2010/06/23 11:00:42  hmuneer
 * CUTE V1.01
 *
 * Revision 1.25  2010/05/05 15:48:53  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.24  2010/04/23 09:06:20  swilson
 * Initialize dynamic heap management.
 *
 * Revision 1.23  2010/01/19 12:04:22  hmuneer
 * vx511 v1.0
 *
 * Revision 1.22  2009/09/29 12:26:05  swilson
 * Zero the Test Number NVRAM before entering test handler.
 *
 * Revision 1.21  2009/06/10 14:00:36  swilson
 * Correction to comment associated with smp_dispatch().
 *
 * Revision 1.20  2009/05/18 12:36:25  swilson
 * Add code to clear CMOS error tokens used during bootstrap.
 *
 * Revision 1.19  2009/01/22 14:43:00  swilson
 * Reduce CPU speed to two decimal places.
 *
 * Revision 1.18  2008/09/29 10:13:18  swilson
 * Remove code using obsolete dBitFlags. Add static variable used to track
 *  active test handler type and function for accessing its value. Re-structure test
 *  handler selection so sign-on is grouped with IBIT code.
 *
 * Revision 1.17  2008/09/19 14:45:02  swilson
 * String output in SMP main() routine should be debug-only.
 *
 * Revision 1.16  2008/07/24 15:10:32  swilson
 * Add accidentally deleted include file.
 *
 * Revision 1.15  2008/07/24 14:44:37  swilson
 * Add board-specific H/W initialization, PCI initialization, CPU frequency display.
 *  Revise header file locations.
 *
 * Revision 1.14  2008/07/18 12:39:51  swilson
 * Add init for SMP and paged memory.
 *
 * Revision 1.13  2008/07/10 16:51:30  swilson
 * Add calibrated delay fuunctions. Re-code loader to support SMP initialization and
 *  add SMP init.
 *
 * Revision 1.12  2008/06/20 12:16:54  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.11  2008/05/29 10:09:47  swilson
 * Add local implementation of HAL memory functions. Modify loader to configure the runtime GDT and IDT.
 *
 * Revision 1.10  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 * Revision 1.9  2008/05/21 13:27:54  swilson
 * Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
 *
 * Revision 1.8  2008/05/15 15:32:40  swilson
 * Add build and version tracking. Implement & test missing c-library functions.
 *
 * Revision 1.7  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.6  2008/05/12 15:03:02  swilson
 * IBIT testing.
 *
 * Revision 1.5  2008/04/25 15:25:07  swilson
 * General organizational changes and additions.
 *
 * Revision 1.4  2008/04/24 16:05:50  swilson
 * Further updates. Added HAL and BIT library - only I/O functions present at this point.
 *
 * Revision 1.3  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 * Revision 1.2  2008/04/21 16:59:24  swilson
 * Basic framework, now with stack, initialised data and C-to-asm linkage all apparently working.
 *
 */



/*
 * Entry point - called from boot_real.S
 * ===========================================================================
 *
 * Things we assume when entering here:
 *
 * 1) We are running in protected mode
 *
 * 2) IDT, GDT and SMP initialisation is complete
 *
 * 2) bCpuCount contains the number of logical CPU cores
 *
 * 3) dStartupFlags contains flags as follows:
 *	[0]   - 1 = diagnostic port active (3F8:9600,8,N,1)
 *	[1]   - 0 = Local IBIT mode, 1 = PBIT
 *	[2]   - 0 = BSP, 1 = AP
 *	[3]   - 0 = BIT, 1 = CUTE	:only matters for combined BIT/CUTE image
 *  [5-4] - power-on BIT duration
 *  [7-6] - test failure action
 *  [8]   - 1 = dBiosInfo_addr contains address of BIOS_INFO structure
 *  [9-11]- Serial port(s) baud rate
 *
 * 3) dBiosInfo_addr address of BIOS_INFO structure
 *
 * 6) Port-80 codes are in range 0xCCC1-0xCCCF
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>

#include <private/k_alloc.h>
#include <private/cpu.h>
#include <private/debug.h>
#include <private/scc.h>
#include <private/sys_delay.h>
#include <private/atomic.h>

#include <bit/bit.h>
#include <bit/hal.h>
#include <bit/board_service.h>

#include <cute/arch.h>

/*Includes for FreeRtos*/
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "crhook.h"
#include "death.h"
#include "semphr.h"

#include "config.h"
#include "../debug/dbgLog.h"
#include "lwip/netif.h"


/* defines */
#define MAX_TSI721_DEVS  3

#if defined(INCLUDE_MAIN_DEBUG) || defined(INCLUDE_DEBUG_PORT_INIT) || defined(INCLUDE_DEMO_TASKS) || defined(INCLUDE_DEBUG_VGA)
#warning "***** DEBUG ON *****"
#endif

#ifdef INCLUDE_DEBUG_VGA
#include "../vgalib/vga.h"
int startLine = 0;
#endif

#ifdef INCLUDE_DEMO_TASKS

#define mainDEMO_TASK_PRIORITY      ( tskIDLE_PRIORITY + 5 )
#define mainDEMO_STACK_SIZE         configMINIMAL_STACK_SIZE

#endif

#define mainCUTEBIT_TASK_PRIORITY 	( tskIDLE_PRIORITY + 5 )
#define mainCUTEBIT_TASK_STACKSIZE	( ( unsigned short ) 1024 * 2)  



/* typedefs */
#ifdef INCLUDE_DEMO_TASKS

typedef struct
{
	UINT8 taskNum;
	UINT8 taskName[16];
}TASKPARAM;

#endif


/* constants */

/* locals */

static UINT32	dActiveTestHandler = TH__REMOTE;	/* this is static for a reason - don't change it */
static int		iCpuCores = 1;


/* globals */
xSemaphoreHandle globalMutexPool[MAX_CUTEBIT_MUTEX]; /*Pool of global mutexes which will be used
														across the code*/

/* externals */

extern UINT8	bCpuCount;
extern UINT8 	bCpuConfigured;
extern UINT32	dStartupFlags;
extern UINT32	dBiosInfo_addr;

#if 0 /* Not used, HAL no longer present */
extern UINT32 cctServiceProvider (SYS_SRVC tServiceId, void* psParams);
#endif

extern void vPbitSequencer (void);
extern void vIbitCli (void);

extern void dbgLogInit();
extern void dbgInstallExceptionHandlers( void );

extern void vMTHCli (void);
extern void vSTHRouter (void);

extern void sysPciInitialize (void);
extern void pciIntInit( void );

extern void networkInit( void );
extern void startSmClient( char *serverIpAddr );
extern size_t xPortGetFreeHeapSize( void );

#if 0 /* Not required and causes build issue with GCC 4.7.3 */
/* Make following functions visible to boot_prot */
extern void c_entry (void) asm ("_c_entry");
extern void smp_dispatch (void) asm ("_smp_dispatch");
extern void bsp_ident (void) asm ("_bsp_ident");
extern void debugInit (void) asm ("_debugInit");
extern void secondStageSignon (void) asm ("_secondStageSignon");
extern void bsp_wait (void) asm ("_bsp_wait");
#endif

#ifdef INCLUDE_MAIN_DEBUG	
extern void pciListDevs( int verbose );
#endif

#if defined(INCLUDE_DEMO_TASKS)

#undef FREERTOS_SEMA_TEST	/* define/undef semaphore test */

extern UINT32 idleCount[];


static TASKPARAM tParam[configMAX_CPU_NUM + 1];

#define RAND_TSIZE		32	/* must be a power of two */
#define RAND_TMSK		(RAND_TSIZE - 1)
#define RAND_MULT(x)	(x) * 1664525L + 1013904223L
#define RAND_MAX		0x3fffffff

static char randInit = 0;
static unsigned long randSeed = 1;
static unsigned long randIdx = 0;
static unsigned long randVal[RAND_TSIZE];
static xSemaphoreHandle randMutex;


/*****************************************************************************
 * rand: compute a random value
 *
 * RETURNS: random value
 */
static int rand( void )
{	
	int j;
	
	
	xSemaphoreTake( randMutex, portMAX_DELAY );
	
	if (randInit == 0)
	{
		for (j = 0; j < 8; ++j)
		{
			randSeed = RAND_MULT(randSeed);
		}
		
		for (j = 0; j < RAND_TSIZE; ++j)
		{
			randVal[j] = (randSeed = RAND_MULT(randSeed));
		}
		
		randIdx = randVal[RAND_TSIZE - 1];
		randInit = 1;
	}
	
	randSeed = RAND_MULT(randSeed);
	j = randIdx & RAND_TMSK;
	randIdx = randVal[j];
	randVal[j] = randSeed;
	
	xSemaphoreGive( randMutex );

	return (randIdx & RAND_MAX);
}


/*****************************************************************************
 * vTaskCodePeriodic: Demo Periodic Task
 *
 * RETURNS: None
 */
void vTaskCodePeriodic( void * pvParameters )
{
	UINT32 count = 0;
	UINT32 taskNum;
	portSHORT cpuNo;
	portTickType delayTicks;
	char *pName;
#ifdef INCLUDE_DEBUG_VGA
	int currentLine;
	char achBuffer[80];
#endif


	pName = ((TASKPARAM*)(pvParameters))->taskName;
	
#ifdef INCLUDE_DEBUG_VGA	
	currentLine = atomic32Inc( (UINT32 *) &startLine );
	vgaClearLine( currentLine );
#endif
	
	cpuNo = sPortGetCurrentCPU();
	taskNum = uxTaskNumGet( cpuNo, NULL );
	
#ifdef INCLUDE_MAIN_DEBUG
	DBLOG( "CPU:%d Starting T%02u: %s\n", cpuNo, taskNum, pName );
#endif
	
	for(;;)
	{
		atomic32Inc( (UINT32 *) &count );
		
		// random delay, 1 - 1024 ticks
		delayTicks = (rand() & 0x03ff) + 1;

#ifdef INCLUDE_DEBUG_VGA
		sprintf( achBuffer, "CPU:%d T%02u: %s  Count: %u Delay ticks: %04lu", 
				cpuNo, taskNum, pName, count, delayTicks );
		vgaPutsXY( 1, currentLine, achBuffer );
#endif

		vTaskDelay( delayTicks );	
	}
}


/*****************************************************************************
 * vTaskCodeBackground: Demo Background Periodic Task
 *
 * RETURNS: None
	 */
void vTaskCodeBackground( void * pvParameters )
{
	UINT32 count = 0;
	portSHORT cpuNo;
	UINT32 taskNum;
	char *pName;

#ifdef INCLUDE_DEBUG_VGA
	int currentLine;
	char achBuffer[80];
#endif	


	pName = ((TASKPARAM*)(pvParameters))->taskName;

#ifdef INCLUDE_DEBUG_VGA	
	currentLine = atomic32Inc( (UINT32 *) &startLine );
	vgaClearLine( currentLine );
#endif

	cpuNo = sPortGetCurrentCPU();
	taskNum = uxTaskNumGet( cpuNo, NULL );
	
#ifdef INCLUDE_MAIN_DEBUG
	DBLOG( "CPU:%d Starting T%02u: %s\n", cpuNo, taskNum, pName );
#endif
	
	for(;;)
	{
		atomic32Inc( (UINT32 *) &count );
		
#ifdef INCLUDE_DEBUG_VGA
		sprintf( achBuffer, "CPU:%d T%02u: %s  Count: %u Idle count: %u", 
					cpuNo, taskNum, pName, count, idleCount[cpuNo] );
		vgaPutsXY( 1, currentLine, achBuffer );
#endif
		
		vTaskDelay( 1000 );
	
#ifdef FREERTOS_SEMA_TEST
		if ( count == 10 )
		{
			/* Grab the debug message mutex to block other tasks */
			/* User - check serial console output has stoppped */
			xSemaphoreTake(globalMutexPool[MUTEX_DEBUG_STR_WRITE],portMAX_DELAY);
		}
		
		if ( count == 20 )
		{
			/* Release the debug message */
			/* User - check serial console output has resumed */
			xSemaphoreGive(globalMutexPool[MUTEX_DEBUG_STR_WRITE]);
			count = 0;
		}
#endif
	}
}

#endif /* INCLUDE_DEMO_TASKS */


#ifdef INCLUDE_STH_DEV_MODE
#warning "***** STH Development Mode  *****"

/*****************************************************************************
 * sthTask: Slave Test Handler task. A separate STH task is created when the 
 *          MTH is also running. This is done for development purposes only.
 *
 * RETURNS: None
 */
static void sthTask( void* param )
{
	vSTHRouter();
}


/*****************************************************************************
 * startSth: Create and start the SM client and STH task. 
 * This is done for development purposes only.
 *
 * RETURNS: None
 */
void startSth( void )
{
	static char buff[20];
	xTaskHandle xHandle;
	
#ifndef INCLUDE_USM_IP_ADDRESS	
	struct netif *pNetif;
#endif	

	vTaskDelay(2000);
	
#ifdef INCLUDE_USM_IP_ADDRESS
	strcpy(buff, USM_IP_ADDRESS);
#else

	pNetif = netif_find( "ge0" );

	if (pNetif != NULL)
	{
		sprintf (buff,"%u.%u.%u.%u",ip4_addr1_16(&pNetif->gw.addr), ip4_addr2_16(&pNetif->gw.addr),
					ip4_addr3_16(&pNetif->gw.addr), ip4_addr4_16(&pNetif->gw.addr));
	}
	else
	{
		/* Try ge1 interface on other MCH */
		pNetif = netif_find( "ge1" );
		if (pNetif != NULL)
		{
			sprintf (buff,"%u.%u.%u.%u",ip4_addr1_16(&pNetif->gw.addr), ip4_addr2_16(&pNetif->gw.addr),
							ip4_addr3_16(&pNetif->gw.addr), ip4_addr4_16(&pNetif->gw.addr));
		}
		else
		{
			strcpy(buff, "0.0.0.0");
		}
	}
#endif

#ifdef INCLUDE_MAIN_DEBUG
	DBLOG( "Soak Master IP: %s\n", buff );
#endif

#ifdef INCLUDE_LWIP_SMC	
	startSmClient( buff );
#endif
	
	vTaskDelay(1000);
	
	xTaskCreate( 0, sthTask, "sth", mainCUTEBIT_TASK_STACKSIZE,
			NULL, mainCUTEBIT_TASK_PRIORITY, &xHandle );
}
#endif /* INCLUDE_STH_DEV_MODE */


#ifdef INCLUDE_NETWORK
void vGetUSMIPAddrs(char *pBuff)
{
#ifndef INCLUDE_USM_IP_ADDRESS
	struct netif *pNetif;
	static char buff[20];
	static char zero_ip[] = "0.0.0.0";
#endif

	vTaskDelay(1000);
	
#ifdef INCLUDE_USM_IP_ADDRESS
#warning "***** SM Static IP address  *****"
	strcpy(pBuff, USM_IP_ADDRESS);
#else

	pNetif = netif_find( "ge0" );

	if (pNetif != NULL)
	{
		sprintf (buff,"%u.%u.%u.%u",ip4_addr1_16(&pNetif->gw.addr), ip4_addr2_16(&pNetif->gw.addr),
					ip4_addr3_16(&pNetif->gw.addr), ip4_addr4_16(&pNetif->gw.addr));
		strcpy(pBuff,buff);

		if ( strcmp(buff,zero_ip) == 0 )
		{
			vTaskDelay(1000);
		   /* Try ge1 interface on other MCH */
			pNetif = netif_find( "ge1" );
			if (pNetif != NULL)
			{
			    sprintf (buff,"%u.%u.%u.%u",ip4_addr1_16(&pNetif->gw.addr), ip4_addr2_16(&pNetif->gw.addr),
										ip4_addr3_16(&pNetif->gw.addr), ip4_addr4_16(&pNetif->gw.addr));
				strcpy(pBuff,buff);
			}
			else
			{
				strcpy(buff, "0.0.0.0");
				strcpy(pBuff,buff);
			}
		}
	}
	else
	{
		strcpy(buff, "0.0.0.0");
		strcpy(pBuff,buff);
	}
#endif

#ifdef INCLUDE_MAIN_DEBUG
	DBLOG( "Soak Master IP: %s\n", buff );
#endif

}
#endif

/*****************************************************************************
 * startCuteBit: Task to start CUTE/BIT
 *
 * RETURNS: None
	 */
static void startCuteBit( void* param )
{
	//char    vBuffer[32];
	DISPLAY_SIGNON_INFO info;
	char	achBuffer[80];
#ifdef INCLUDE_LWIP_SMC
	char  achSrverIpAddrs[20];
#endif
#ifdef MAKE_CUTE
	Temp_String_Info temp_info;
	char    vBuffer[32];
#endif

#ifdef  USE_BOARD_SWITCHES
	UINT8   umswitch;
#endif
	/* Enter (IBIT or PBIT)/(MTH or STH) as selected by BIT flags or switches */

#if defined(MAKE_BIT) || defined (MAKE_BIT_CUTE)

	#ifdef MAKE_BIT_CUTE
		if(bStartupCUTEMode() == SF__BIT)
		{
	#endif
			/* BIT Mode Select between IBIT or PBIT*/
	#ifdef  USE_BOARD_SWITCHES
			board_service(SERVICE__BRD_USR_MTH_SWITCH, NULL, &umswitch);
			if(umswitch == SW__TEST )
	#endif

	#ifdef USE_STARTUP_FLAGS
			if (bStartupTestHandler() == SF__IBIT)
	#endif
			{
				dActiveTestHandler = TH__LBIT;

				sysDebugWriteString ("TH__IBIT\n");

				/* Display the sign-on message and version info */

				puts ("\n");
				puts (" O-O O                              ");
				puts (" |  /        C O N C U R R E N T    ");
				puts (" O O O                              ");
				puts ("  /  |     T E C H N O L O G I E S  ");
				puts (" O O-O                              ");
				puts ("                                    ");
				puts ("    B u i l t - i n   T e s t       ");
				puts ("                                    ");
				puts ("   (c) Copyright 2008-2015          ");
				puts ("                                    ");

				info.iLevel = FWID_BASIC;
				info.mode   = FWMODE_BIT;
				board_service(SERVICE__BRD_DISPLAY_SIGNON, NULL, &info);

				sprintf (achBuffer, "%i CPU cores @ %i.%02iGHz",
									(int)bCpuCount,
									sysGetCpuFrequency () / 1000,
									(sysGetCpuFrequency () % 1000) / 10);
				puts (achBuffer);
				sysDebugFlush ();

				/* Enter the test handler CLI */
				vIbitCli ();
			}
			else	/* PBIT */
			{
				dActiveTestHandler = TH__PBIT;

				sprintf (achBuffer, "%i CPU cores @ %i.%02iGHz\n",
									(int)bCpuCount,
									sysGetCpuFrequency () / 1000,
									(sysGetCpuFrequency () % 1000) / 10);
				sysDebugWriteString (achBuffer);
				vPbitSequencer ();
			}
	#ifdef MAKE_BIT_CUTE
		}
	#endif

#endif


#if defined(MAKE_CUTE) || defined (MAKE_BIT_CUTE)

	#ifdef MAKE_BIT_CUTE
		if(bStartupCUTEMode() == SF__CUTE)
		{
	#endif
			/* Allow early messages and debug output to be printed before displaying sign-on */
			vTaskDelay( 1000 );

			/* CUTE Mode Select between MTH or STH */
	#ifdef  USE_BOARD_SWITCHES
#warning "*****  USE_BOARD_SWITCHES SET *****"
			board_service(SERVICE__BRD_USR_MTH_SWITCH, NULL, &umswitch);
			sprintf (achBuffer, "umswitch: 0x%x\n", umswitch);
			sysDebugWriteString (achBuffer);

			if(umswitch == SW__USER )
	#endif

	#ifdef USE_STARTUP_FLAGS
#warning "*****  USE_STARTUP_FLAGS SET *****"
			if (bStartupTestHandler() == SF__IBIT)
	#endif
			{
#ifdef INCLUDE_STH_DEV_MODE		
				startSth();
				vTaskDelay(1000);
#endif
				dActiveTestHandler = TH__MTH;
				sysDebugWriteString ("TH__MTH\n");
				/* Display the sign-on message and version info */

				puts ("\n");
				puts (" O-O O                              ");
				puts (" |  /        C O N C U R R E N T    ");
				puts (" O O O                              ");
				puts ("  /  |     T E C H N O L O G I E S  ");
				puts (" O O-O                              ");
				puts ("                                    ");
				puts (" M a s t e r  T e s t  H a n d l e r");
				puts ("                                    ");
				puts ("(c) Copyright 2008-2015             ");
				puts ("                                    ");

				info.iLevel = FWID_BASIC;
				info.mode   = FWMODE_CUTE;
				board_service(SERVICE__BRD_DISPLAY_SIGNON, NULL, &info);

				sprintf (achBuffer, "%i CPU cores @ %i.%02iGHz",
						(int)bCpuCount,
						sysGetCpuFrequency () / 1000,
						(sysGetCpuFrequency () % 1000) / 10);
				puts (achBuffer);
				sysDebugFlush ();

				temp_info.size   = 32;
				temp_info.buffer = (UINT8 *)vBuffer;
				if(board_service(SERVICE__BRD_GET_BOARD_NAME, NULL, &temp_info) == E__OK)
				{
					sprintf (achBuffer, "NAME: %s", temp_info.buffer);
					puts (achBuffer);
				}

				temp_info.size   = 32;
				temp_info.buffer = (UINT8 *)vBuffer;
				if(board_service(SERVICE__BRD_GET_TEMP_GRADE_STRING, NULL, &temp_info) == E__OK)
				{
					sprintf (achBuffer, "TYPE: %s", temp_info.buffer);
					puts (achBuffer);
				}
				sysDebugFlush ();

				if(board_service(SERVICE__BRD_GET_BIOS_VERSION, NULL, vBuffer) == E__OK)
				{
					sprintf (achBuffer, "BIOS: %s", vBuffer);
					puts (achBuffer);
				}
				puts ("\n");
				sysDebugFlush ();

				/* Enter the test handler CLI */
				vMTHCli ();
			}
			else	/* STH */
			{
				dActiveTestHandler = TH__STH;
				sysDebugWriteString ("TH__STH\n");
				board_service(SERVICE__BRD_PROCESS_STH_ORIDE_LIST, NULL, NULL);
				
#ifdef INCLUDE_LWIP_SMC
				vTaskDelay(2000);
				
				vGetUSMIPAddrs(achSrverIpAddrs);
				
				sprintf (achBuffer, "Soak Master IP: %s\n", achSrverIpAddrs);
				sysDebugWriteString (achBuffer);
				sysDebugFlush ();

				startSmClient( achSrverIpAddrs );
	
				vTaskDelay(1000);
#endif
				vSTHRouter();

			}

	#ifdef MAKE_BIT_CUTE
		}
	#endif

#endif


	/* Stop us exiting to the loader should the above return */
	while (1);
}


/*****************************************************************************
 * c_entry: this is our main() function, called from boot_prot.S
 *
 * RETURNS: None
 */
__attribute__((regparm(0))) void c_entry (void)
{
	xTaskHandle xHandle;
	UINT8   	i=0,bFlag = 0;
    UINT16		port, baud = 0;
    NV_RW_Data	nvdata;
#ifdef INCLUDE_DEBUG_VGA
    char achBuffer[80];
#endif
    

#if 0 /* Not used, HAL no longer present */

	/*
	 * Register the local service provider now, as code will be using BIT
	 * services for PCI accesses.
	 */

	cctRegisterServiceProvider (cctServiceProvider);
#endif

	/*
	 * Key sub-system initialisation
	 */

	sysMmuInitialize ();				/* Initialise the local page table management */
	sysInitMalloc (K_HEAP, U_HEAP);		/* Initialise the heap management */

#ifdef INCLUDE_EXCEPTION_DEBUG
	dbgInstallExceptionHandlers();
#endif

#ifdef INCLUDE_DBGLOG	
	dbgLogInit();
#endif

	/*Initialise the mutexes so that we can start using the public functions*/
	for(i=0; i < MAX_CUTEBIT_MUTEX; i++)
	{
		globalMutexPool[i]=xSemaphoreCreateMutex();

		if( globalMutexPool[i] == NULL )
		{
			/*We should not get here, die...*/
			while(1);
		}
	}

	/*
	 * Search and parse BIOS_INFO and  EXTENDED_INFO structures
	 */
	board_service(SERVICE__BRD_GET_BIOS_INFO,     NULL, &dBiosInfo_addr);
	board_service(SERVICE__BRD_GET_EXTENDED_INFO, NULL, NULL);


	/*
	 * Open/close the debug channel and sign-on - we can't use vDebugWrite() this
	 * early, so make direct calls to sysDebug...
	 */
	if(bStartupDebugMode())
	{
		sysDebugWriteString ("\n[Debugging Enabled]\n");
		
		if(sysIsDebugopen())
		{
			if(board_service(SERVICE__BRD_GET_FORCE_DEBUG_OFF, NULL, NULL) == E__OK)
			{	
				sysDebugWriteString ("[Debugging Forced Off]\n");
				sysDebugClose();
			}
		}
	}
	else if( board_service(SERVICE__BRD_GET_FORCE_DEBUG_ON, NULL, NULL) == E__OK )
	{
		sysDebugWriteString ("[Debugging Forced On]\n");
		bForceStartupDebugMode();
		sysDebugOpen();
	}
	
	sysDebugFlush ();
	
#ifdef INCLUDE_DEBUG_VGA
	vgaDisplayInit();
	vgaSetCursor( CURSOR_OFF );
#endif

	sysDebugWriteString ("Starting HW Initialization\n");
	board_service(SERVICE__BRD_HARDWARE_INIT, NULL, NULL); /* board-specific H/W initialisation */

	if(board_service(SERVICE__BRD_GET_CONSOLE_PORT, NULL, &port) == E__OK)
	{
		baud = bGetBaudRate();
		//baud = 0; //hard coded by Rajan. Need to remove after debug
		sysSccInit (port, baud);
		
#ifdef INCLUDE_DEBUG_VGA
		sprintf( achBuffer, "Console open, port: 0x%x baud: %u\n", port, baud );
		vgaPutsXY( 0, startLine++, achBuffer );
		
#endif
	}

	if(board_service(SERVICE__BRD_GET_DEBUG_PORT, NULL, &port) == E__OK)
	{

//#ifdef commented by Rajan. Need to uncomment after debug
//#ifdef INCLUDE_DEBUG_PORT_INIT
			baud = bGetBaudRate();
            baud = 0; //hard coded by Rajan. Need to remove after debug
			sysSccInit (port, baud);
//#endif

#ifdef INCLUDE_DEBUG_VGA			
			sprintf( achBuffer, "Debug %s,   port: 0x%x baud: %u\n", 
						(sysIsDebugopen()? "open":"closed"), port, baud );
			vgaPutsXY( 0, startLine++, achBuffer );
#endif		
	}


#ifdef INCLUDE_MAIN_DEBUG
	{	
		UINT32 temp = 0;
		
		board_service(SERVICE__BRD_GET_MAX_DRAM_BELOW_4GB, NULL, &temp);
		DBLOG( "Max DRAM below 4GB: 0x%08x\n", temp );
	}
#endif

#if	defined(MAKE_CUTE) || defined (MAKE_BIT_CUTE)
	#if defined(VPX)
		vpxPreInit();
	#endif

	#if defined(CPCI)
		board_service(SERVICE__BRD_CHECK_CPCI_IS_SYSCON,NULL,&bFlag);
		cpciPreInit(bFlag);
	#endif

	board_service(SERVICE__BRD_CONFIGURE_VXS_NT_PORTS, NULL, NULL);
#endif

#if defined (SRIO)

	sysDebugWriteString ("Initializing: SRIO\n");
	
	for (i = 1 ; i < MAX_TSI721_DEVS; i++)
	{
	   srioPreInit (i);
	}

#endif

#ifndef CPCI
	if (!bFlag)
		vDelay(2000);// delay added, so pmc160 will show up!
#endif

	sysDebugWriteString ("Initializing: PCI\n");
	sysPciInitialize ();		/* (re)scan PCI and allocate resources */

	sysDebugWriteString ("Rajan--> Step1\n");


#if	defined(MAKE_CUTE) || defined (MAKE_BIT_CUTE)
	#if defined (VME) && defined (INCLUDE_LWIP_SMC)
		// pci bus enumeration must have been run
		InitialiseVMEDevice();
	#endif
#endif

	sysDebugWriteString ("Rajan--> Step2\n");


#ifdef INCLUDE_DEBUG_VGA
	vgaPutsXY( 0, startLine, "FreeRTOS starting....\n\n" );
	startLine += 2;
#endif

	board_service(SERVICE__BRD_POST_SCAN_INIT, NULL, NULL); // post scan H/W initialisations
	board_service(SERVICE__BRD_ENABLE_SMIC_LPC_IOWINDOW, NULL, NULL); /* board-specific smic Initialization */
	/*
	 * Having got this far, clear the load error that we've been holding in CMOS
	 */
	nvdata.dNvToken = NV__TEST_NUMBER;
	nvdata.dData    = 0;
	board_service(SERVICE__BRD_NV_WRITE, NULL, &nvdata);

	nvdata.dNvToken = NV__ERROR_CODE;
	nvdata.dData    = E__BOARD_HANG;
	board_service(SERVICE__BRD_NV_WRITE, NULL, &nvdata);

	
#ifdef INCLUDE_MAIN_DEBUG
	DBLOG( "CUTE/BIT Kernel heap: 0x%08x-0x%08x\n", (UINT32)K_HEAP, ((UINT32)K_HEAP + HEAP_SIZE - 1) );
	DBLOG( "CUTE/BIT User heap  : 0x%08x-0x%08x\n", (UINT32)U_HEAP, ((UINT32)U_HEAP + HEAP_SIZE - 1) );
	
	xPortGetFreeHeapSize();
#endif

	
	/* Initialise task data */
	vTaskDataInit( bCpuCount );
	
	/* Initialise PCI shared interrupt handling */
	pciIntInit();

#ifdef INCLUDE_MAIN_DEBUG	
	pciListDevs( 0 );
#endif
	
	/* Create and start the network */
	sysDebugWriteString ("Initializing: Network\n");
	networkInit();

#if defined(SRIO)
	sysDebugWriteString ("Initializing: TSI721\n");
	
	for (i = 1 ; i < MAX_TSI721_DEVS; i++)
	{
		tsi721_init(i);
	}

	rio_init_mports();
	
#endif

	// some boards may not reset the RTC cleanly
	board_service(SERVICE__BRD_INIT_RTC, NULL, NULL);

#ifdef INCLUDE_DEMO_TASKS
	
	randMutex = xSemaphoreCreateMutex();
	if (randMutex == NULL)
	{
		sysDebugWriteString ("Error - Failed to create rand mutex\n");
	}
	else
	{
		for (i = 0; i < bCpuConfigured; i++)
		{
			tParam[i].taskNum = i + 1;
			sprintf( tParam[i].taskName, "Demo%u", tParam[i].taskNum );
			xTaskCreate( i, vTaskCodePeriodic, tParam[i].taskName, mainDEMO_STACK_SIZE, 
						&tParam[i],	mainDEMO_TASK_PRIORITY, &xHandle );
		}
		
		tParam[i].taskNum = i + 1;
		sprintf( tParam[i].taskName, "BKGRD" );	
		xTaskCreate( 0, vTaskCodeBackground, tParam[i].taskName, mainDEMO_STACK_SIZE, 
						&tParam[i],	mainDEMO_TASK_PRIORITY+1, &xHandle );
	}						
#endif

	sysDebugWriteString ("Creating CUTE task\n");
	xTaskCreate( 0, startCuteBit, "CuteBitTask", mainCUTEBIT_TASK_STACKSIZE,
			NULL, mainCUTEBIT_TASK_PRIORITY, &xHandle );

#ifdef INCLUDE_MAIN_DEBUG
	sysDebugWriteString ("\n[Starting FreeRtos Scheduler]\n");
	sysDebugFlush ();
#endif


	sysInvalidateTLB();

	/* Start the FreeRTOS Scheduler, does not return */
	vTaskStartScheduler();

} /* c_entry () */



/*****************************************************************************
 * sysGetTestHandler: provide secure access to the BIT operating mode
 *
 * In RBIT and CBIT modes we will access the default dActiveTestHandler value,
 * so can't distinguish between them (in any case, we can't trust what the OS
 * tells us about its test handler mode).
 *
 * RETURNS: 0, TH__PBIT or TH__L_IBIT
 */

UINT8 sysGetTestHandler (void)
{
	return (dActiveTestHandler);

} /* sysGetTestHandler () */

/*FreeRtos Specific Functions*/
/*-----------------------------------------------------------*/

void vMainQueueSendPassed( void )
{

} /* vMainQueueSendPassed() */


void vApplicationIdleHook( void )
{
        /* The co-routines are executed in the idle task using the idle task
        hook. */
        vCoRoutineSchedule();
} /* vApplicationIdleHook() */



/*****************************************************************************
 * Interface functions, called from boot_prot.S
 */

__attribute__((regparm(0))) void secondStageSignon (void)
{
#ifdef MAKE_BIT
	sysAPDebugWriteString ("Concurrent Technologies BIT: 2nd-stage\n");
#endif

#ifdef MAKE_CUTE
	sysAPDebugWriteString ("Concurrent Technologies CUTE: 2nd-stage\n");
#endif

#ifdef MAKE_BIT_CUTE
	sysAPDebugWriteString ("Concurrent Technologies BIT/CUTE: 2nd-stage\n");
#endif

}

__attribute__((regparm(0))) void debugInit (void)
{
	board_service(SERVICE__BRD_SERIAL_HARDWARE_INIT, NULL, NULL); /* board-specific H/W initialisation */
	sysDebugOpen ();
}

__attribute__((regparm(0))) void smp_dispatch (void)
{
	char	achBuffer[80];

	sprintf (achBuffer, "AP core %i idling...\n", iCpuCores++);
	sysAPDebugWriteString (achBuffer);

} /* smp_dispatch () */


__attribute__((regparm(0))) void bsp_ident (void)
{
	sysAPDebugWriteString ("BSP core Initialised\n");

} /* smp_dispatch () */


__attribute__((regparm(0))) void bsp_wait (void)
{
	sysAPDebugWriteString ("BSP core waiting\n");

} /* smp_dispatch () */
