
/************************************************************************
 *                                                                      *
 *      Copyright 2014 Concurrent Technologies, all rights reserved.    *
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/amc1x.msd/test_list.c,v 1.5 2014-11-04 15:23:58 mgostling Exp $
 * $Log: test_list.c,v $
 * Revision 1.5  2014-11-04 15:23:58  mgostling
 * Added BITF_CUTE to PciPw test permissions.
 * Changed BITF_CUTE to BITF_CBIT in IpmiAMCTest_M3 test permissions.
 *
 * Revision 1.4  2014-10-09 09:48:32  chippisley
 * Removed redundant code that accesses the Tsi721 from the brdGetTestDirectory() function.
 *
 * Revision 1.3  2014-10-03 15:18:31  chippisley
 * Corrected bus/dev numbers in brdGetTestDirectory() code which accesses the Tsi721.
 *
 * Revision 1.2  2014-09-19 12:56:05  mgostling
 * Replaced IpmiAMCTest() with IpmiAMCTest_M3()
 *
 * Revision 1.1  2014-09-19 10:41:42  mgostling
 * Initial check in to CVS
 *
 *
 */

/* includes */

#include <stdtypes.h>

#include <bit/bit.h>
#include <bit/mem.h>
#include <bit/tests.h>
#include <bit/board_service.h>

#include <cute/idx.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/*
 * This is the default test directory for the AMC 310/08x board
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
		12,
		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (FPUTest)
	},

	{
		13,
		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (FPUExpTest)
	},

	{
		14,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (MMXTest)
	},

	{
		15,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (XMMTest)
	},

	{
		16,
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
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
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
		TEST_NAME (EthRegAccessTest2)
	},

	{
		41,
		800,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthIntLoopBackTest2)
	},

	{
		42,
		800,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthInterruptTest2)
	},

	{
		43,
		800,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthPhyRegReadTest2)
	},

	{
		44,
		2000,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthEepromTest2)
	},

	{
		45,
		8300,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (EthExtLoopBackTest2)
	},

	{
		46,
		70,
		BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (EthEepromWriteTest2)
	},

	{
		47,
		2000,
		BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (EthExtDevLoopbackTest2)
	},

/*
	{
			50,
			2000,
			BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
			TEST_NAME (SRIOChannel1Test)
	},

	{
			51,
			2000,
			BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
			TEST_NAME (SRIOChannel2Test)
	},
*/

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

#ifdef CUTE
	{
		83,

		2800,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IdeFlashDriveTest)
	},
#endif

	{
		85,
		2800,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (AhciRegisterTest)
	},

	{
		86,
		2800,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (AhciDriveAttachTest)
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
		102,
		1600,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (brdGPIOTest)
	},

	{
		110,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (bitSpiFwhTest)
	},

	{
		111,
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (bitBackupSpiFwhTest)
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

	{
		130,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SMBUSTest)
	},

	{
		132,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (tsi721SmbusTest)
	},

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
		170,
		400,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (EEPROMTest)
	},

	{
		180,
		3600,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IpmiSmicInterfaceTest)
	},

	{
		181,
		3600,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IpmiIntTest)
	},

	{
		182,
		3600,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IpmiSelfTest)
	},

	{
		185,
		3600,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IpmiWDPreTest)
	},

	{
		186,
		3600,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (IpmiWDResetTest)
	},

    {
		190,
		20,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IpmiSDRFormatTest)
	},

	{
		191,
		20,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IpmiThresholdTest)
	},

	{
		192,
		2000,
		BITF__CBIT,
		TEST_NAME (IpmiAMCTest_M3)
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

#if 1
	{
		220,
		2000,
		BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (CCT_SrioRegisterAccessTest)
	},

	{
		221,
		2000,
		BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (CCT_SrioRegisterLinkSpeedTest)
	},
	{
		222,
		2000,
		BITF__PBIT_60S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (CCT_SrioExternalLoopBackTest)
	},
	
	{
		223,
		2000,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (CCT_SrioScanBackPlaneDevs)
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

