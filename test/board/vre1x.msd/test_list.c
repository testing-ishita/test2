
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/test_list.c,v 1.8 2014-10-15 13:28:55 mgostling Exp $
 *
 * $Log: test_list.c,v $
 * Revision 1.8  2014-10-15 13:28:55  mgostling
 * Disabled the entry for the IpmiAlarmIntTest as it is not supported by the IPMI micro
 * and it is not a requirement of the IPMI specification.
 *
 * Revision 1.7  2014-04-23 15:27:25  mgostling
 * Added missing BITF__CUTE flags for certain tests.
 *
 * Revision 1.6  2014-03-28 17:26:20  cdobson
 * Added wPciMPTGeneralTest
 *
 * Revision 1.5  2014-03-27 11:56:00  mgostling
 * Added a new test to vefify XMC differention IO signals.
 *
 * Revision 1.4  2014-03-26 14:45:11  mgostling
 * Removed tests 51 to 53 form list.
 *
 * Revision 1.3  2014-03-20 16:05:24  mgostling
 * Added enxmc GPIO loopback test
 * Added ADVR3 VPX loopback test
 *
 * Revision 1.2  2014-03-06 16:38:24  cdobson
 * Changed audio test names to new tests in audioTest2.c
 *
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.2  2013-09-25 13:36:43  mgostling
 * Disable AHCI interrupt test (87) and USB Device connectid test (91).
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.4  2011-11-14 17:18:29  hchalla
 * Added new test for tr803 idt reading temperature test and other updates.
 *
 * Revision 1.3  2011/09/29 14:38:37  hchalla
 * Minor Changes for release of TR 80x V1.01.
 *
 * Revision 1.2  2011/08/26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
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

		20,
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
		BITF__PBIT_30S |  BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthIntLoopBackTest)
	},

	{
		42,

		800,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
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
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthExtLoopBackTest)
	},

	{
		46,

		70,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthEepromWriteTest)
	},

	{
		48,

		8300,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthExtDevLoopbackTest)
	},
	
	{
		50,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PciPXMCTest)
	},

	{
		60,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (VPXBridge)
	},

	{
		61,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PcieVPXTest)
	},

	{
		62,
		
		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (PcieSwitchEepromTest)
	},

#ifdef CUTE
    {
        63,

        500,
                          BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
        TEST_NAME (PciePex8733Loopback)
    },
#endif

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
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SerialLocalloopback)
	},

	{
		73,

		600,
						 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SerialExternalLoopback)
	},

	{
		74,

		60,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
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
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IdeRegaccessTest)
	},

	{
		81,

		2100,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
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

#if 0
	{
		87,

		2800,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (AhciInterruptTest)
	},
#endif

	{
		90,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (UsbRegTest)
	},
	
#if 0
	{
		91,

		20,
		                 BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (UsbDeviceCheck)
	},
#endif

	{
		92,

		400,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
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

		20,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (LEDTest)
	},
	
	{
		102,

		20,
				         BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (brdGPIOTest)
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

	{
		130,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SMBUSTest)
	},
	
	{
		131,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (CKSI52143Test)
	},
	
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

	{
		170,

		400,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SpiMRAMTest)
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

		60,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (IpmiThresholdTest)
	},

#if 0
//
// This function is not implemented in any of the current IPMI micros
// There is no requirement for the IPMI controller to generate interrupts
// for the conditions that this test was trying to validate!
//
// This code can be removed at a later date if this feature remains unimplemented
//
	{
		192,

		60,
		                 BITF__LBIT | BITF__RBIT | BITF__CUTE,
		TEST_NAME (IpmiAlarmIntTest)
	},
#endif

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

#ifdef CUTE
	{
		215,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE |BITF__COOP,
		TEST_NAME (MstrRemoteRamReadWrite)
	},
#endif

	{
		220,

		500,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (VideoAccess)
	},

    {
        224,

        500,
                         BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
        TEST_NAME (TPMTest)
    },
	
#ifdef CUTE
    {
        230,

        500,
        BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
        TEST_NAME (HdaAudioCodecTest)
    },

    {
        231,

        500,
        BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
        TEST_NAME (HdaAudioJackTest)
    },

    {
        232,

        500,
        BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
        TEST_NAME (HdaAudioSoundTest)
    },

    {
        248,

        500,
        BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
        TEST_NAME (Enxmc_Pex8619_Diff_IO_Test)
    },

    {
        249,

        500,
        BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
        TEST_NAME (Enxmc_Pex8619_Gpio_Test)
    },

    {
        250,

        500,
        BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
        TEST_NAME (enxmc_pex8619_dma_test)
    },
    {
        260,

        500,
        BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT,
        TEST_NAME (wPciMPTGeneralTest)
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

