#ifndef __tests_h__
	#define __tests_h__

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

/* tests.h - extern definitions for all available tests
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/tests.h,v 1.18 2015-04-01 16:45:49 hchalla Exp $
 *
 * $Log: tests.h,v $
 * Revision 1.18  2015-04-01 16:45:49  hchalla
 * Added support for backplane CPCI tests.
 *
 * Revision 1.17  2015-03-10 09:56:31  hchalla
 * Added New Vme Bus error test.
 *
 * Revision 1.16  2015-02-27 11:58:49  mgostling
 * Added new VME bus tests for network VME CUTE
 *
 * Revision 1.15  2015-02-25 17:58:19  hchalla
 * Added new XM 650 site tests.
 *
 * Revision 1.14  2015-02-03 11:53:11  mgostling
 * Added enpmc tests
 *
 * Revision 1.13  2015-01-29 10:14:17  mgostling
 * Add additional tests for VP91x
 *
 * Revision 1.12  2015-01-21 18:02:23  gshirasat
 * Added AMC PCIe fabric specific tests
 *
 * Revision 1.11  2014-11-24 15:59:09  mgostling
 * Added XMRS2TestBoard1 and XMRS2TestBoard2
 *
 * Revision 1.10  2014-10-15 11:50:47  mgostling
 * Disabled reference to IpmiAlarmIntTest.
 *
 * Revision 1.9  2014-09-19 13:34:04  mgostling
 * Added IpmiAMCTest_M3
 *
 * Revision 1.8  2014-09-19 09:39:58  mgostling
 * Added tsi721SmbusTest(), bitBackupSpiFwhTest() and new ethernet tests
 * from eth2.c
 *
 * Revision 1.7  2014-03-28 15:19:38  cdobson
 * Added wPciMPTGeneralTest
 *
 * Revision 1.6  2014-03-27 11:57:30  mgostling
 * Added a new test to vefify XMC differention IO signals.
 *
 * Revision 1.5  2014-03-20 16:10:03  mgostling
 * Added enxmc GPIO loopback test
 * Added ADVR3 VPX loopback test
 *
 * Revision 1.4  2014-03-06 16:33:22  cdobson
 * Added new audio tests in audioTest2.c
 *
 * Revision 1.3  2014-01-29 13:20:22  hchalla
 * Added new tests for SRIO.
 *
 * Revision 1.2  2013-11-25 10:51:03  mgostling
 * Added new tests for TRB1x and ADCR6
 *
 * Revision 1.1  2013-09-04 07:35:27  chippisley
 * Import files into new source repository.
 *
 * Revision 1.29  2012/03/21 16:24:26  cdobson
 * Added XM RS1/20x tests.
 *
 * Revision 1.28  2012/02/17 11:30:37  hchalla
 * Added support for PP 91x and PP 81x, added new data structures for TPM module and DS3905 tests.
 *
 * Revision 1.27  2011/12/01 13:48:46  hchalla
 * Updated for VX 541, added new tests and board service functions.
 *
 * Revision 1.26  2011/11/14 17:32:45  hchalla
 * Added support for PP 531.
 *
 * Revision 1.25  2011/08/02 17:07:29  hchalla
 * Updated macros.
 *
 * Revision 1.24  2011/05/11 12:16:25  hchalla
 * Initial version of PBIT for TP 702, PBIT test changes.
 *
 * Revision 1.23  2011/03/22 13:39:02  hchalla
 * Added new tests.
 *
 * Revision 1.22  2011/02/28 11:58:25  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.21  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.20  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.19  2010/10/25 15:36:04  jthiru
 * Added function prototypes
 *
 * Revision 1.18  2010/09/15 12:18:38  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.17  2010/09/10 09:53:02  cdobson
 * Added Ethernet controler to controller test definition.
 *
 * Revision 1.16  2010/06/23 10:59:21  hmuneer
 * CUTE V1.01
 *
 * Revision 1.15  2010/03/31 16:28:35  hmuneer
 * no message
 *
 * Revision 1.14  2010/02/26 10:34:58  jthiru
 * Added SODIMM SPD test
 *
 * Revision 1.13  2010/01/19 12:04:10  hmuneer
 * vx511 v1.0
 *
 * Revision 1.12  2009/12/16 14:10:32  sdahanayaka
 * Added the VME and Watchdog Tests. Also fixed some PCI Bugs
 *
 * Revision 1.11  2009/09/29 12:28:53  swilson
 * Tidy up layout. Add Application Flash tests.
 *
 * Revision 1.10  2009/06/10 14:44:15  hmuneer
 * ck505
 *
 * Revision 1.9  2009/06/09 15:04:16  jthiru
 * Added IPMI Watchdog test case routines
 *
 * Revision 1.8  2009/06/05 14:34:22  jthiru
 * Added SII3124 and CAN test cases
 *
 * Revision 1.7  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.6  2009/05/21 08:18:10  cvsuser
 * Added FWH test
 *
 * Revision 1.5  2009/05/20 12:12:06  swilson
 * Add interrupt test.
 *
 * Revision 1.4  2009/05/18 09:25:29  hmuneer
 * no message
 *
 * Revision 1.3  2009/05/15 11:00:00  jthiru
 * Added new test case entries
 *
 * Revision 1.2  2009/02/04 07:55:46  jthiru
 * Added serial and ethernet test cases
 *
 * Revision 1.1  2008/05/16 14:36:24  swilson
 * Add test list management and test execution capabilities.
 *
 */


/* includes */

#include <stdtypes.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

EXTERN_INTERFACE (PassingTest)
EXTERN_INTERFACE (FailingTest)
EXTERN_INTERFACE (RandomTest)
EXTERN_INTERFACE (SerialRegAccessTest)
EXTERN_INTERFACE (SerialInterruptTest)
EXTERN_INTERFACE (SerialBaudRateTest)
EXTERN_INTERFACE (SerialLocalloopback)
EXTERN_INTERFACE (SerialExternalLoopback)
EXTERN_INTERFACE (SerialRS422Loopback)
EXTERN_INTERFACE (SerialRS485Loopback)
EXTERN_INTERFACE (SerialRS485ExtLoopback)
EXTERN_INTERFACE (EthRegAccessTest)
EXTERN_INTERFACE (EthEepromTest)
EXTERN_INTERFACE (EthPhyRegReadTest)
EXTERN_INTERFACE (EthIntLoopBackTest)
EXTERN_INTERFACE (EthInterruptTest)
EXTERN_INTERFACE (EthExtDevLoopbackTest)
EXTERN_INTERFACE (IpmiSmicInterfaceTest)
EXTERN_INTERFACE (IpmiIntTest)
EXTERN_INTERFACE (IpmiSelfTest)
EXTERN_INTERFACE (IpmiSDRFormatTest)
EXTERN_INTERFACE (IpmiThresholdTest)
//EXTERN_INTERFACE (IpmiAlarmIntTest) // nonsense test! Not implemented by our IPMI micro and not required by the spec!
EXTERN_INTERFACE (EthExtLoopBackTest)
EXTERN_INTERFACE (EthEepromWriteTest)
EXTERN_INTERFACE (IdeRegaccessTest)
EXTERN_INTERFACE (IdeIntTest)
EXTERN_INTERFACE (IdeRdSectTest)
EXTERN_INTERFACE (IdeFlashDriveTest)
EXTERN_INTERFACE (AhciRegisterTest)
EXTERN_INTERFACE (AhciDriveAttachTest)
EXTERN_INTERFACE (AhciInterruptTest)
EXTERN_INTERFACE (KbCInterconnect)
EXTERN_INTERFACE (KbCSelfTest)
EXTERN_INTERFACE (KbdInterconnect)
EXTERN_INTERFACE (KbCInterrupt)
EXTERN_INTERFACE (KbCAuxTest)
EXTERN_INTERFACE (MouseInterconnect)
EXTERN_INTERFACE (MouseActionTest)
EXTERN_INTERFACE (MemDatalineTest)
EXTERN_INTERFACE (MemAddrlineTest)
EXTERN_INTERFACE (MemSPDEEPROMTest)
EXTERN_INTERFACE (UsbRegTest)
EXTERN_INTERFACE (UsbDeviceCheck)
EXTERN_INTERFACE (UsbOCCheck)
EXTERN_INTERFACE (ThermAccessTest)
EXTERN_INTERFACE (ThermThreshTest)
EXTERN_INTERFACE (MemEccTest)
EXTERN_INTERFACE (SuperIo1Test)
EXTERN_INTERFACE (SuperIo2Test)
EXTERN_INTERFACE (SuperIo1DeviceTest)
EXTERN_INTERFACE (SuperIo2DeviceTest)
EXTERN_INTERFACE (PciTest)
EXTERN_INTERFACE (PciPXMCTest)
EXTERN_INTERFACE (PciPW)
EXTERN_INTERFACE (PciBist)
EXTERN_INTERFACE (CPciBridge)
EXTERN_INTERFACE (PciCPCITest)
EXTERN_INTERFACE (VPXBridge)
EXTERN_INTERFACE (PcieVPXTest)
EXTERN_INTERFACE (PcieSwitchEepromTest)
EXTERN_INTERFACE (PciADCR2Test)
EXTERN_INTERFACE (PciADCRXTest)
EXTERN_INTERFACE (PciADCR5Test)
EXTERN_INTERFACE (LEDTest)
EXTERN_INTERFACE (NVRAMTest)
EXTERN_INTERFACE (EEPROMTest)
EXTERN_INTERFACE (Site1Test)
EXTERN_INTERFACE (Site2Test)
EXTERN_INTERFACE (SpiEEPROMTest)
EXTERN_INTERFACE (AudioCodecTest)
EXTERN_INTERFACE (AudioJackTest)
EXTERN_INTERFACE (AudioSoundTest)
EXTERN_INTERFACE (VideoAccess)
EXTERN_INTERFACE (VideoRAM)
EXTERN_INTERFACE (VideoInterrupt)
EXTERN_INTERFACE (RTCTest)
EXTERN_INTERFACE (RTCIntTest)
EXTERN_INTERFACE (PITTest)
EXTERN_INTERFACE (SCRTest)
EXTERN_INTERFACE (WatchDogIntTest)
EXTERN_INTERFACE (WatchDogRstTest)
EXTERN_INTERFACE (LdtBasicTest)
EXTERN_INTERFACE (LdtIntTest)
EXTERN_INTERFACE (SpeedStepTest)
EXTERN_INTERFACE (MMXTest)
EXTERN_INTERFACE (XMMTest)
EXTERN_INTERFACE (FPUTest)
EXTERN_INTERFACE (FPUExpTest)
EXTERN_INTERFACE (SMBUSTest)
EXTERN_INTERFACE (CK409Test)
EXTERN_INTERFACE (TempTest)
EXTERN_INTERFACE (CK505Test)
EXTERN_INTERFACE (DB800Test)
EXTERN_INTERFACE (DB400Test)
EXTERN_INTERFACE (DS3905Test)
EXTERN_INTERFACE (CKSI52143Test)
EXTERN_INTERFACE (tsi721SmbusTest)
EXTERN_INTERFACE (InterruptTest)
EXTERN_INTERFACE (CPUExec)
EXTERN_INTERFACE (Fwhdevid)
EXTERN_INTERFACE (sii3124RegaccessTest)	
EXTERN_INTERFACE (sii3124InterruptTest)
EXTERN_INTERFACE (sii3124RdFirstSectTest)
EXTERN_INTERFACE (CanCtrlAccessTest)
EXTERN_INTERFACE (CanCtrllbTest)
EXTERN_INTERFACE (IpmiWDPreTest)
EXTERN_INTERFACE (IpmiWDResetTest)
EXTERN_INTERFACE (MirrorFlashAccessTest)
EXTERN_INTERFACE (MirrorFlashRwTest)
EXTERN_INTERFACE (MirrorFlashErase)
EXTERN_INTERFACE (vmeDeviceAccessTest)
EXTERN_INTERFACE (universeIntTest)
EXTERN_INTERFACE (wDogIntTest)
EXTERN_INTERFACE (wDogResetTest)
EXTERN_INTERFACE (TSI148Access)
EXTERN_INTERFACE (TSI148Int)
EXTERN_INTERFACE (TSI148VME)
EXTERN_INTERFACE (TSI148VMEerror)
EXTERN_INTERFACE (TSI148ByteSwapWrite)
EXTERN_INTERFACE (TSI148ByteSwapWriteChk)
EXTERN_INTERFACE (TSI148ByteSwapRead)
EXTERN_INTERFACE (TSI148ByteSwapReadChk)
EXTERN_INTERFACE (bitSpiFwhTest)
EXTERN_INTERFACE (bitBackupSpiFwhTest)
EXTERN_INTERFACE (SodimmSpdTest)
EXTERN_INTERFACE (ThermSenseTest)
EXTERN_INTERFACE (BoardTemperatureSense)
EXTERN_INTERFACE (amcPcieDataTxfer)
EXTERN_INTERFACE (amcPcieLinkTest)
#ifdef CUTE
EXTERN_INTERFACE (CctTestInfo)
#endif
EXTERN_INTERFACE (DramReadWrite)
EXTERN_INTERFACE (DramDualAddress)
EXTERN_INTERFACE (fpNMITest)
EXTERN_INTERFACE (ps2MouseTest)
EXTERN_INTERFACE (ps2MouseActionTest)
EXTERN_INTERFACE (ps2MouseActionReport)
EXTERN_INTERFACE (PMCSCSIGeneralTest)
EXTERN_INTERFACE (PMCSCSIIFTest)
EXTERN_INTERFACE (EthInterfaceSelect)
EXTERN_INTERFACE (Enxmc_Pex8619_Diff_IO_Test)
EXTERN_INTERFACE (Enxmc_Pex8619_Gpio_Test)
EXTERN_INTERFACE (enxmc_pex8619_dma_test)
EXTERN_INTERFACE (enxmc2_pex8619_dma_test)
EXTERN_INTERFACE (enxmc_vxs_pex8619_loopback)
EXTERN_INTERFACE (GPIOTest)
EXTERN_INTERFACE (URTMGPIOTest)
EXTERN_INTERFACE (LM95245Test)
EXTERN_INTERFACE (XMRS1Test)
EXTERN_INTERFACE (XMRS1IntTest)
EXTERN_INTERFACE (XMRS2TestBoard1)
EXTERN_INTERFACE (XMRS2TestBoard2)
EXTERN_INTERFACE (IpmiAMCTest)
EXTERN_INTERFACE (IpmiAMCTest_M3)
EXTERN_INTERFACE (enpmc_pex8619_dma_test)
EXTERN_INTERFACE (enpmc_gpio_test)
#ifdef CUTE
EXTERN_INTERFACE (MstrRemoteRamReadWrite)
#endif
EXTERN_INTERFACE (VpxCarrierTest)
EXTERN_INTERFACE (trXmcPciPW)
EXTERN_INTERFACE (trXmcPXMCTest)
EXTERN_INTERFACE (VxsPcieConnectivity)
EXTERN_INTERFACE (VxsPcieLoopBackA)
EXTERN_INTERFACE (VxsPcieLoopBackB)
EXTERN_INTERFACE (brdGPIOTest)

#ifdef CUTE
EXTERN_INTERFACE (Adcr6Test)
EXTERN_INTERFACE (PciePex8733Loopback)
#endif

/* Utility functions
 * */
EXTERN_INTERFACE (Tsi148VmeConfig)
EXTERN_INTERFACE (Tsi148PciConfig)
EXTERN_INTERFACE (DispConfig)
EXTERN_INTERFACE (SpiMRAMTest)
EXTERN_INTERFACE(idtTempSensorTest)
EXTERN_INTERFACE(RTCIrqIntTest)
EXTERN_INTERFACE(TPMTest)

/* SRIO Tests */
EXTERN_INTERFACE(CCT_SrioRegisterAccessTest)
EXTERN_INTERFACE(CCT_SrioRegisterLinkSpeedTest)
EXTERN_INTERFACE(CCT_SrioExternalLoopBackTest)
EXTERN_INTERFACE(CCT_SrioScanBackPlaneDevs)

/* New audio tests */
EXTERN_INTERFACE (HdaAudioCodecTest)
EXTERN_INTERFACE (HdaAudioJackTest)
EXTERN_INTERFACE (HdaAudioSoundTest)

/* MPT SCSI/SAS */
EXTERN_INTERFACE (wPciMPTGeneralTest)

/* New Ethernet Tests */
EXTERN_INTERFACE (EthRegAccessTest2)
EXTERN_INTERFACE (EthEepromTest2)
EXTERN_INTERFACE (EthPhyRegReadTest2)
EXTERN_INTERFACE (EthIntLoopBackTest2)
EXTERN_INTERFACE (EthInterruptTest2)
EXTERN_INTERFACE (EthExtDevLoopbackTest2)
EXTERN_INTERFACE (EthExtLoopBackTest2)
EXTERN_INTERFACE (EthEepromWriteTest2)

/* VME Tests */
EXTERN_INTERFACE(VMESimpleTransferTest)
EXTERN_INTERFACE(VMEDMATransferTest)
EXTERN_INTERFACE(VMEBridgelessInterfaceTest)
EXTERN_INTERFACE(VMEBusErrorTest)
/* CPCI Tests */
EXTERN_INTERFACE(CPCISimpleTransferTest)

/* forward declarations */


#endif


