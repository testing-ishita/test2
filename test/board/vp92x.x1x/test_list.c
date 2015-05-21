
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/test_list.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: test_list.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
 * Import files into new source repository.
 *
 * Revision 1.9  2013/02/04 11:04:26  madhukn
 * CUTE 1.03
 *
 * Revision 1.8  2012/10/10 06:47:24  madhukn
 * Changes for VP91x PBIT V1.01-01
 *
 * Revision 1.7  2012/08/24 10:55:20  madhukn
 * *** empty log message ***
 *
 * Revision 1.6  2012/08/24 10:15:09  madhukn
 * BIT_VER Tool modified to generate new format [XorV][Version].[Revision]-[subversion]
 *
 * De-emphasis bit set for B:0 D:1 F:1 for XMC2 detection
 *
 * Test added to Board Temperature through Super IO
 *
 * Revision 1.5  2012/08/09 14:33:06  madhukn
 * *** empty log message ***
 *
 * Revision 1.4  2012/08/07 15:16:23  madhukn
 * *** empty log message ***
 *
 * Revision 1.3  2012/08/07 03:32:59  madhukn
 * *** empty log message ***
 *
 * Revision 1.2  2012/07/03 12:42:19  madhukn
 * updated Status and control register definitions for VP91x,
 * Added VME tests for CUTE MTH
 *
 * Revision 1.1  2012/06/22 11:42:01  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/bit.h>
#include <bit/tests.h>
#include <bit/board_service.h>

#include <cute/idx.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/*
 * This is the default test directory for the VP 91x/01x board
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

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (PassingTest)
	},

	{
		2,

		20,
		 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (FailingTest)
	},


	{
		10,

		100,
		 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (CPUExec)
	},

	{
		11,

		110,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SpeedStepTest)
	},

	{
		12,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (FPUTest)
	},

	{
		13,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (FPUExpTest)
	},

	{
		14,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MMXTest)
	},

	{
		15,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (XMMTest)
	},

	{
		16,

		90,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (InterruptTest)
	},

	{
		20,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MemDatalineTest)
	},

	{
		21,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MemAddrlineTest)
	},


	{
		22,
		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MemEccTest)
	},

#ifdef CUTE
	{
		23,
		70,
		BITF__CUTE,
		TEST_NAME (DramReadWrite)
	},

	{
		25,
		70,
		BITF__CUTE,
		TEST_NAME (DramDualAddress)
	},
#endif


 	{
		30,
		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PciTest)
	},

	{
		31,

		20,
						BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PciPW)
	},

	{
		32,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PciBist)
	},

	{
		40,

		950,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthRegAccessTest)
	},

	{
		41,

		2780,
		BITF__PBIT_30S |  BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthIntLoopBackTest)
	},

	{
		42,

		770,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthInterruptTest)
	},

	{
		43,

		1010,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthPhyRegReadTest)
	},

	{
		44,

		260,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthEepromTest)
	},

	{
		45,

		8300,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT ,
		TEST_NAME (EthExtLoopBackTest)
	},

	{
		46,

		70,
						BITF__LBIT | BITF__RBIT | BITF__CBIT,
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
						BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (PciPXMCTest)
	},

	


	{
		60,
		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE ,
		TEST_NAME (TSI148Access)
	},

	{
		61,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT ,
		TEST_NAME (TSI148Int)
	},

	{
		63,
		10,
		BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (TSI148VMEerror)
	},

	{
		64,
		20,
						BITF__LBIT | BITF__RBIT,
		TEST_NAME (TSI148ByteSwapWrite)
	},

	{
		65,
		20,
						BITF__LBIT | BITF__RBIT,
		TEST_NAME (TSI148ByteSwapWriteChk)
	},

	{
		66,
		20,
						BITF__LBIT | BITF__RBIT,
		TEST_NAME (TSI148ByteSwapRead)
	},

	{
		67,
		20,
						BITF__LBIT | BITF__RBIT,
		TEST_NAME (TSI148ByteSwapReadChk)
	},


	{
		70,

		170,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SerialRegAccessTest)
	},

	{
		71,

		1520,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SerialInterruptTest)
	},

	{
		72,

		760,
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

		1220,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (SerialBaudRateTest)
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

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (UsbRegTest)
	},

	{
		91,

		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (UsbDeviceCheck)
	},

	{
		92,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (UsbOCCheck)
	},

	{
		100,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SCRTest)
	},

	{
		101,

		20,
		BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (LEDTest)
	},
	{
		102,
	
		20,
		BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (brdGPIOTest)
	},

	{
		110,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (bitSpiFwhTest)
	},

	{
		120,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (KbCInterconnect)
	},

	{
		121,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (KbCInterrupt)
	},

	{
		122,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (KbCSelfTest)
	},

	{
		130,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SMBUSTest)
	},

	{
		140,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (NVRAMTest)
	},

	{
		150,

		1490,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (RTCTest)
	},

	{
		151,

		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
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

#ifdef CUTE
	{
		163,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (PMCSCSIGeneralTest)
	},

	{
		164,
		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (PMCSCSIIFTest)
	},
#endif

	{
		170,
		260,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (EEPROMTest)
	},

	{
		180,

		20,
		 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IpmiSmicInterfaceTest)
	},
		
	{
		190,
		3600,
						BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (TempTest)
	},

	
	{
		191,
		3600,
		BITF__LBIT | BITF__RBIT,
		TEST_NAME (BoardTemperatureSense)
	},

	{
		200,
		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (SuperIo1Test)
	},

	{
		201,
		10,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (SuperIo1DeviceTest)
	},
	
	{
		202,
		3600,
						BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (WatchDogIntTest)
	},
	
	{
		210,

		130,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (LdtBasicTest)
	},

	{
		211,

		130,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (LdtIntTest)
	},
	
	{
		224,

		1030,
		BITF__PBIT_10S| BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (TPMTest)
	},

	/*{
		230,
		10,
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
	},*/

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

/* globals */

UINT32* pTestDirectory = (UINT32*)asTestDirectory;		/* required global for OS interface */

/* externals */

/* forward declarations */



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

