
/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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
		  BITF__LBIT | BITF__RBIT | BITF__CBIT ,
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
		42,
		800,
		BITF__PBIT_30S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (EthInterruptTest)
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
		2000,
		BITF__PBIT_60S |BITF__LBIT | BITF__RBIT | BITF__CBIT,
		TEST_NAME (EthExtDevLoopbackTest)
	},
	
		
	/*{
			50,
			2000,
			BITF__PBIT_60S |BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
			TEST_NAME (SRIOChannel1Test)
	},

	{
			51,
			2000,
			BITF__PBIT_60S |BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
			TEST_NAME (SRIOChannel2Test)
	},*/
	

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

   {
		130,

		20,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT | BITF__CUTE,
		TEST_NAME (SMBUSTest)
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

/*
	{
		170,
		400,
		BITF__PBIT_10S | BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
		TEST_NAME (EEPROMTest)
	},
*/

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
		BITF__CUTE,
		TEST_NAME (IpmiAMCTest)
	},
#ifdef CUTE
	/*{
		208,
		20,
		BITF__CUTE |BITF__COOP,
		TEST_NAME (MstrRemoteRamReadWrite)
	},*/
#endif

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
	#if 1
	{
			220,
			2000,
			BITF__PBIT_60S |BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
			TEST_NAME (CCT_SrioRegisterAccessTest)
	},

	{
			221,
			2000,
			BITF__PBIT_60S |BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
			TEST_NAME (CCT_SrioRegisterLinkSpeedTest)
	},	
	{
			222,
			2000,
			BITF__PBIT_60S |BITF__LBIT | BITF__RBIT | BITF__CBIT| BITF__CUTE,
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
#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))
	
	
#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr)  = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

UINT32 brdGetTestDirectory (void *ptr)
{
	UINT8	bTemp;
	UINT32 mHandle;
	UINT32 pointer,dTemp,Base_Address;
	PCI_PFA	pfa;	
	PTR48  tPtr;
	
	
	pfa = PCI_MAKE_PFA (1,0,0);
	if(dPciReadReg (pfa, 0x00, REG_32) == 0x80AB111D)
	{
		/*Enabling Memory and I/O Access*/
		bTemp = PCI_READ_BYTE (pfa, 0x04);
		bTemp |= 0x07;
		
		
		PCI_WRITE_BYTE (pfa, 0x04, bTemp);
		Base_Address = dPciReadReg (pfa, 0x10, REG_32);

		mHandle  = dGetPhysPtr (Base_Address, 0x1000, &tPtr, (void*)(&pointer));
				

		if(mHandle !=NULL)
		{
			dTemp= vReadDWord(pointer + 0x48004);
			dTemp = dTemp | (1 << 2);
			vWriteDWord ((pointer + 0x48004), dTemp);
			dTemp= vReadDWord (pointer + 0x48004);
		}


	}

	
	pfa = PCI_MAKE_PFA (2,0,0);
	if(dPciReadReg (pfa, 0x00, REG_32) == 0x80AB111D)
	{
		/*Enabling Memory and I/O Access*/
		bTemp = PCI_READ_BYTE (pfa, 0x04);
		bTemp |= 0x07;
		
		
		PCI_WRITE_BYTE (pfa, 0x04, bTemp);
		Base_Address = dPciReadReg (pfa, 0x10, REG_32);

		mHandle  = dGetPhysPtr (Base_Address, 0x1000, &tPtr, (void*)(&pointer));
				

		if(mHandle !=NULL)
		{
			dTemp= vReadDWord (pointer + 0x48004);
			dTemp = dTemp | (1 << 2);
			vWriteDWord ((pointer + 0x48004), dTemp);
			dTemp= vReadDWord (pointer + 0x48004);
		}


	}

	*((TEST_ITEM**)ptr) = asTestDirectory;
	return E__OK;

} /* brdGetTestDirectory () */

