
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx813.09x/test_list.c,v 1.4 2014-11-24 16:07:39 mgostling Exp $
 *
 * $Log: test_list.c,v $
 * Revision 1.4  2014-11-24 16:07:39  mgostling
 * Added XMRS2TestBoard1 and XMRS2TestBoard2
 *
 * Revision 1.3  2014-04-22 16:40:53  swilson
 * Fix PBIT permissions for two test cases.
 *
 * Revision 1.2  2014-01-10 09:48:53  mgostling
 * Added flag for SOAK testing to adcr6Test and changed test number to 230.
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.2  2013-10-15 11:10:57  mgostling
 * Add AHCI and ADCR6 tests
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.5  2011/11/21 11:15:59  hmuneer
 * no message
 *
 * Revision 1.4  2011/06/07 15:04:34  hchalla
 * Initial version of PBIT for VX813.
 *
 * Revision 1.3  2011/05/16 14:30:16  hmuneer
 * Info Passing Support
 *
 * Revision 1.2  2011/03/22 13:32:49  hchalla
 * Initial Version.
 *
 * Revision 1.1  2011/02/28 11:57:43  hmuneer
 * no message
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
 * This is the default test directory for the TP 40x/35x board
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
		BITF__PBIT_TEST | BITF__LBIT | BITF__RBIT | BITF__CBIT,
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
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
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

#ifdef CUTE
    {
		23,
		70,
						BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (DramReadWrite)
	},

    {
		25,
		70,
						BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (DramDualAddress)
	},
#endif

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

#ifdef CUTE
	{
		51,
		20,
				         BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (VxsPcieConnectivity)
	},

	{
		52,
		20,
				         BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (VxsPcieLoopBackA)
	},

	{
		53,
		20,
				         BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (VxsPcieLoopBackB)
	},
#endif

	{
		60,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (TSI148Access)
	},

	{
		61,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (TSI148Int)
	},

	{
		62,

		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (TSI148VME)
	},

	{
		63,
		20,
		BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (TSI148VMEerror)
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
		TEST_NAME (SerialRS485ExtLoopback)
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
		85,
		2800,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (AhciRegisterTest)
	},

	{
		86,
		2800,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (AhciDriveAttachTest)
	},

	{
		87,
		2800,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (AhciInterruptTest)
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
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SCRTest)
	},

	{
		101,
		1600,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (LEDTest)
	},

	{
		102,
		1600,
						BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (GPIOTest)
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
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (NVRAMTest)
	},

	{
		150,
		1100,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (RTCTest)
	},

	{
		151,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (RTCIntTest)
	},

#ifdef CUTE
	{
		163,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (PMCSCSIGeneralTest)
	},
#endif

	{
		170,
		400,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EEPROMTest)
	},

	{
		180,
		3600,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
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
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (ThermSenseTest)
	},

	{
		200,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SuperIo1Test)
	},

	{
		201,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SuperIo1DeviceTest)
	},

	{
		210,
		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (LdtBasicTest)
	},

	{
		211,
		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (LdtIntTest)
	},

	{
		220,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SMBUSTest)
	},

	{
		221,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (CK505Test)
	},

	{
		222,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (LM95245Test)
	},

#ifdef CUTE
	{
		230,
		2000,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (Adcr6Test)
	},

	{
		231,
		2000,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (XMRS2TestBoard1)
	},

	{
		232,
		2000,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (XMRS2TestBoard2)
	},
#endif

	{
		240,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (fpNMITest)
	},

#ifdef CUTE
	{
		250,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (enxmc_pex8619_dma_test)
	},

	{
		251,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (enxmc_vxs_pex8619_loopback)
	},
#endif

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
/* globals */

/* required global for OS interface */
UINT32* pTestDirectory = (UINT32*)asTestDirectory;


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

