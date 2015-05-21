
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

/* test_dir.c - Test Directory for this board
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp717.x8x/test_list.c,v 1.1 2015-04-02 11:41:18 mgostling Exp $
 *
 * $Log: test_list.c,v $
 * Revision 1.1  2015-04-02 11:41:18  mgostling
 * Initial check-in to CVS
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.13  2012/03/29 14:14:24  cdobson
 * Added tests for XM RS1/20x.
 *
 * Revision 1.12  2011/11/21 11:15:43  hmuneer
 * no message
 *
 * Revision 1.11  2011/10/27 15:48:10  hmuneer
 * no message
 *
 * Revision 1.10  2011/05/11 15:24:51  hchalla
 * Added new AD CR5 PMC carrier test.
 *
 * Revision 1.9  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.8  2011/01/20 10:01:23  hmuneer
 * CA01A151
 *
 * Revision 1.7  2010/12/03 09:52:45  jthiru
 * Removed ck505 test from VP717 PBIT/IBIT - Affects board reset
 *
 * Revision 1.6  2010/10/25 15:34:35  jthiru
 * Added new test cases, flags to selective non PBIT tests
 *
 * Revision 1.5  2010/10/08 12:58:28  jthiru
 * Removed Dual RAM address test from PBIT
 *
 * Revision 1.4  2010/10/06 10:12:14  jthiru
 * PBIT coding for VP717
 *
 * Revision 1.3  2010/09/14 09:16:36  cdobson
 * Removed TSI-148 loopback test. Doesnt work with byte swap FPGA.
 *
 * Revision 1.2  2010/09/10 09:48:02  cdobson
 * Added Ethernet controller to controller loopback test.
 *
 * Revision 1.1  2010/06/23 10:49:08  hmuneer
 * CUTE V1.01
 *
 * Revision 1.1  2010/01/19 12:03:15  hmuneer
 * vx511 v1.0
 *
 * Revision 1.1  2009/09/29 12:16:45  swilson
 * Create VP417 Board
 *
 * Revision 1.13  2009/05/29 14:08:21  hmuneer
 * Error Codes Fixed
 *
 * Revision 1.12  2009/05/21 14:06:35  swilson
 * Updated members, permission flags and timing figures to match release configuration.
 *
 * Revision 1.11  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.10  2009/05/21 08:18:56  cvsuser
 * Added FWH test
 *
 * Revision 1.9  2009/05/20 15:49:33  swilson
 * Remove 'Random Test' as it is not part of the production firmware.
 *
 * Revision 1.8  2009/05/20 12:14:39  swilson
 * CPU interrupt test.
 *
 * Revision 1.7  2009/05/19 08:09:53  cvsuser
 * no message
 *
 * Revision 1.5  2009/05/15 10:56:10  jthiru
 * Added new test case entries
 *
 * Revision 1.4  2009/02/04 07:55:07  jthiru
 * Added serial and ethernet test cases
 *
 * Revision 1.3  2008/09/29 10:16:31  swilson
 * Add test duration field. Change some permissions for testing. Improve commenting.
 *
 * Revision 1.2  2008/07/18 13:14:04  swilson
 * *** empty log message ***
 *
 * Revision 1.1  2008/05/16 14:36:19  swilson
 * Add test list management and test execution capabilities.
 *
 */


/* includes */

#include <stdtypes.h>
#include <string.h>

#include <bit/bit.h>
#include <bit/tests.h>
#include <bit/board_service.h>

#include <cute/idx.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/*
 * This is the default test directory for the VP 71x/x1x board
 */

static TEST_ITEM	asTestDirectory[] =
{
	/*	Test number,
	 *	Test duration (mS),
	 *  Test permissions: PBIT | LBIT | RBIT | CBIT,
	 * 	root name
	 */
	{
		1,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (PassingTest)
	},

	{
		2,
		20,
		BITF__PBIT_TEST | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (FailingTest)
	},


	{
		10,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT/* | BITF__CUTE*/,
		TEST_NAME (CPUExec)
	},


	{
		11,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SpeedStepTest)
	},

	{
		14,
		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (FPUTest)
	},

	{
		15,
		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (FPUExpTest)
	},


	{
		16,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MMXTest)
	},

	{
		17,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (XMMTest)
	},

	{
		18,
		600,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (InterruptTest)
	},




	{
		20,
		70,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MemDatalineTest)
	},

	{
		21,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MemAddrlineTest)
	},

	{
		22,
		70,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MemEccTest)
	},

	{
		30,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PciTest)
	},

	{
		31,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PciPW)
	},

	{
		32,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PciBist)
	},

	{
		40,
		800,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthRegAccessTest)
	},

	{
		41,
		1200,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthIntLoopBackTest)
	},

	{
		42,
		800,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthInterruptTest)
	},

	{
		43,
		2000,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthPhyRegReadTest)
	},

	{
		44,
		2000,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthEepromTest)
	},

	{
		45,
		8300,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (EthExtLoopBackTest)
	},

	{
		46,
		70,
		BITF__PBIT_60S |BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (EthEepromWriteTest)
	},

	{
		47,
		70,
						BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (EthInterfaceSelect)
	},

	{
		48,
		8300,
						BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (EthExtDevLoopbackTest)
	},

	{
		50,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PciPXMCTest)
	},

	{
		51,
		20,
						 BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (PciADCRXTest)
	},

#if defined(MAKE_CUTE)
	{
		52,
		5000,
						 BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (XMRS1Test)
	},

	{
		53,
		5000,
						 BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (XMRS1IntTest)
	},
#endif

	{
		60,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (TSI148Access)
	},

	{
		63,
		20,
						 BITF__LBIT,
		TEST_NAME (VMEBusErrorTest)
	},

	{
		67,
		2000,
						 BITF__LBIT,
		TEST_NAME (VMEBridgelessInterfaceTest)
	},

	{
		68,
		2000,
						BITF__LBIT | BITF__CUTE,
		TEST_NAME (VMESimpleTransferTest)
	},

	{
		69,
		20,
						BITF__LBIT | BITF__CUTE,
		TEST_NAME (VMEDMATransferTest)
	},

	{
		70,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SerialRegAccessTest)
	},

	{
		71,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SerialInterruptTest)
	},

	{
		72,
		600,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (SerialLocalloopback)
	},

	{
		73,
		600,
						 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (SerialExternalLoopback)
	},

	{
		74,
		60,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (SerialBaudRateTest)
	},

	{
		75,
		60,
						BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (SerialRS422Loopback)
	},

	{
		76,
		60,
						BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (SerialRS485Loopback)
	},

	{
		80,
		2100,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (IdeRegaccessTest)
	},

	{
		81,
		2100,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (IdeIntTest)
	},

	{
		82,
		2800,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (IdeRdSectTest)
	},

	{
		90,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (UsbRegTest)
	},

	{
		91,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT ,
		TEST_NAME (UsbDeviceCheck)
	},

	{
		92,
		400,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (UsbOCCheck)
	},

	{
		100,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (SCRTest)
	},

	{
		110,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (bitSpiFwhTest)
	},

	{
		120,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (KbCInterconnect)
	},

	{
		121,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (KbCInterrupt)
	},

	{
		122,
		30,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (KbCSelfTest)
	},

#ifdef CUTE
    {
		127,
		30,
		BITF__CUTE,
		TEST_NAME (CctTestInfo)
	},
#endif

	{
		140,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (NVRAMTest)
	},

	{
		150,
		1100,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (RTCTest)
	},

	{
		151,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (RTCIntTest)
	},


	{
		160,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (ps2MouseTest)
	},

	{
		161,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (ps2MouseActionTest)
	},

	{
		162,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (ps2MouseActionReport)
	},

#if defined(MAKE_CUTE) || defined(MAKE_BIT_CUTE)
	{
		163,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (enpmc_pex8619_dma_test)
	},

	{
		164,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (enpmc_gpio_test)
	},
#endif

	{
		170,
		400,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (EEPROMTest)
	},

	{
		180,
		3600,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (WatchDogIntTest)
	},

	{
		181,
		3600,
						BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (WatchDogRstTest)
	},	

	{
		190,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (ThermSenseTest)
	},

	{
		200,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (SuperIo1Test)
	},

	{
		201,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (SuperIo1DeviceTest)
	},


	{
		210,
		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (LdtBasicTest)
	},

	{
		211,
		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (LdtIntTest)
	},


	{
		220,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (SMBUSTest)
	},

	{
		221,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (CK505Test)
	},

	{
		230,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MirrorFlashAccessTest)
	},

	{
		231,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (MirrorFlashRwTest)
	},

	{
		232,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (MirrorFlashErase)
	},

	{
		240,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (fpNMITest)
	},


	/*
	 * 900-999 test numbers are not really tests, they represent
	 * board/architecture/general utility functions only visible in MTH
	 */
#ifdef CUTE
	{
		900,
		2000,
						 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (Tsi148VmeConfig)
	},

	{
		901,
		2000,
						 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (Tsi148PciConfig)
	},
#endif

	/*
	 * 999 is the last test number, its not really a test, its a utility
	 * that shows the board's current configuration
	 */

	{
		999,
		2000,
						 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (DispConfig)
	},

	
	{ 0, 0, NULL, NULL }	/* last entry */
};

#ifdef CUTE
static const COOP_ENTRY	asCoopTbl[] =
{
	{	23,	CP_READ_WRITE,	0},
	{	25,	CP_DUAL_ADDR,	0},
	{	0,	0,				0}
};
#endif

// test number lists terminated with 0
// must be in numerical order
UINT16 excludedBridgeTests[] = {60, 63, 68, 69, 0};
UINT16 excludedBridgelessTests[] = {67, 0};

/* globals */

UINT32* pTestDirectory = (UINT32*)asTestDirectory;		/* required global for OS interface */

/* externals */

/* forward declarations */

/*****************************************************************************
 * updateTestList: disable tests not needed for this board
 *
 */
static void updateTestList (UINT16 * excludeList) 
{
	UINT16	testIndex;
	UINT8	excludeIndex;

	excludeIndex = 0;
	for (testIndex = 0; asTestDirectory[testIndex].wTestNum != 0; ++testIndex)
	{
		// is this test to be excluded?
		if (asTestDirectory[testIndex].wTestNum == excludeList[excludeIndex])
		{
			// clear all test handler flags for this test
			asTestDirectory[testIndex].dFlags = 0;

			// next entry in excluded test list
			++excludeIndex;

			// end of excluded test list?
			if (excludeList[excludeIndex] == 0)
			{
				break;
			}
		}
	}
}

/*****************************************************************************
 * brdGetTestDirectory: get a pointer to this board's test directory
 *
 * Note: we do this via a function to allow for dynamic test directories based
 *		 on board-specific parameters.
 *
 * RETURNS: pointer to the directory
 */
UINT32 brdGetTestDirectory (void *ptr)
{
	UINT8	bridgePresent;
	
	// update board specific test lists for each type of board
	
	board_service(SERVICE__BRD_VME_BRIDGELESS_MODE, NULL, &bridgePresent);
	if (bridgePresent == 0)
	{
		// bridgeless board, so disable bridge tests
		updateTestList (excludedBridgeTests);
	}
	else
	{
		// bridged board, so disable bridgeless tests
		updateTestList (excludedBridgelessTests);
	}
	
	*((TEST_ITEM**)ptr) = asTestDirectory;

	return E__OK;

} /* brdGetTestDirectory () */

 #ifdef CUTE
/*****************************************************************************
 * vGetCoopTestTable: point to the cooperating test table
 *
 * RETURNS: none, address returned via pointer
 */
UINT32 brdGetCoopTestTable(void *ptr)
{

	*((COOP_ENTRY**)ptr) = (void*)asCoopTbl;

	return E__OK;

} /* vGetCoopTestTable () */
#endif



/*********************************************************************
 * brdProcessSthTestList: Board service to modify test list base on
 * 						  extended info if present
 * ptr	            	: NULL
 * RETURNS          	: E__OK
 */
UINT32 brdProcessSthTestList (void* ptr)
{

	UINT32 tests[256];
	STH_Override_List list;
	UINT32 count, i;

	#ifdef DEBUG
		UINT8  buffer[64];
	#endif

	memset(tests, 0x00, (256*4));
	list.size   = 256;
	list.buffer = (UINT8*)tests;

	#ifdef DEBUG
		sysDebugWriteString("\nbrdProcessSthTestList()");
	#endif

	if(board_service(SERVICE__BRD_GET_STH_ORIDE_TEST_LIST, NULL, &list) == E__OK)
	{
		count = 0;
		while(asTestDirectory[count].wTestNum != 0)
		{
			asTestDirectory[count].dFlags &= ~BITF__CUTE;
			count++;
		}

		count = 0;
		while(tests[count] != 0)
		{
			#ifdef DEBUG
				sprintf(buffer, "\nTests %d ",tests[count] );
				sysDebugWriteString(buffer);
#endif

			i = 0;
			while(asTestDirectory[i].wTestNum != 0)
			{
				if(asTestDirectory[i].wTestNum == tests[count] )
				{
					#ifdef DEBUG
						sysDebugWriteString("\tFOUND");
					#endif

					asTestDirectory[count].dFlags |= BITF__CUTE;
					break;
				}
				i++;
			}
			count++;
		}

		#ifdef DEBUG
			sysDebugWriteString("\n");
		#endif
	}

	return E__OK;

} /* brdProcessSthTestList () */

