/****************************************************************************
 *                                                                                                                *
 *      Copyright 2008 Concurrent Technologies, all rights reserved.        *
 *                                                                                                                 *
 *      The program below is supplied by Concurrent Technologies           *
 *      on the understanding that no responsibility is assumed by             *
 *      Concurrent Technologies for any errors contained therein.               *
 *      Furthermore, Concurrent Technologies makes no commitment to     *
 *      update or keep current the program code, and reserves the              *
 *      right to change its specifications at any time.                                     *
 *                                                                                                                    *
 *      Concurrent Technologies assumes no responsibility either for          *
 *      the use of this code or for any infringements of the patent                  *
 *      or other rights of third parties which may result from its use               *
 *                                                                                                                     *
 ******************************************************************************/

/*
 * services.h
 *
 *  Created on: 27 Oct 2010
 *      Author: engineer
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/services.h,v 1.1 2015-02-25 17:50:45 hchalla Exp $
 *
 * $Log: services.h,v $
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.4  2012/10/10 06:47:24  madhukn
 * Changes for VP91x PBIT V1.01-01
 *
 * Revision 1.3  2012/08/07 15:16:23  madhukn
 * *** empty log message ***
 *
 * Revision 1.2  2012/08/07 03:28:09  madhukn
 * *** empty log message ***
 *
 * Revision 1.1  2012/06/22 11:42:01  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */

#ifndef SERVICES_H_
#define SERVICES_H_

typedef struct tagServiceType
{
	UINT32	iServiceID;
	UINT32	(*pfnCmdHandler)();

} SERVICE_TYPE;

/* board_cfg.c */

extern UINT32 brdGetConsolePort 		(void *ptr);
extern UINT32 brdGetDebugPort  			(void *ptr);
extern UINT32 brdGetSccPortMap 			(void *ptr);
extern UINT32 brdGetSccParams  			(void *ptr);
extern UINT32 brdGetMaxDramBelow4GB 	(void *ptr);
extern UINT32 brdFlashLed 				(void *ptr);
extern UINT32 brdReset 					(void *ptr);
extern UINT32 brdCheckUserMthSwitch 	(void *ptr);
extern UINT32 brdCheckETHPresence 		(void *ptr);

extern UINT32 brdGetSokePort 			(void *ptr);
extern UINT32 brdSkipEthInterface		(void *ptr);
extern UINT32 brdGetNumCores 			(void *ptr);
extern UINT32 brdGetFpuExceptionMap 	(void *ptr);
extern UINT32 brdHwInit 				(void *ptr);
extern UINT32 brdCuteInfo				(void *ptr);
extern UINT32 brdNvWrite 				(void *ptr);
extern UINT32 brdNvRead 				(void *ptr);
extern UINT32 brdPciGetParams 			(void *ptr);
extern UINT32 brdPciCfgHbStart 			(void *ptr);
extern UINT32 brdPciCfgHbEnd 			(void *ptr);
extern UINT32 brdPciRoutePirqToIrq 		(void *ptr);
extern UINT32 brdDisplaySignon 			(void *ptr);
extern UINT32 brdGetVersionInfo 		(void *ptr);
extern UINT32 brdGetTestDirectory 		(void *ptr);
extern UINT32 brdAppFlashGetParams 		(void *ptr);
extern UINT32 brdAppFlashSelectPage	 	(void *ptr);
extern UINT32 brdAppFlashEnable 		(void *ptr);
extern UINT32 brdGetSuperIO				(void *ptr);
extern UINT32 brdGetSuperIONum 			(void *ptr);
extern UINT32 brdGetPCIDB				(void *ptr);
extern UINT32 brdGetSCR					(void *ptr);
extern UINT32 brdLdtPitInfo 			(void *ptr);
extern UINT32 brdEEPROMinfo				(void *ptr);
extern UINT32 brdSMBUSinfo				(void *ptr);
extern UINT32 brdGetTempinfo			(void *ptr);
extern UINT32 brdLEDinfo				(void *ptr);
extern UINT32 brdSpiEEPROMInfo 			(void *ptr);
extern UINT32 brdSerialBaudInfo     	(void *ptr);
extern UINT32 brdInitIcr 				(void *ptr);
extern UINT32 brdGetCoopTestTable   	(void *ptr);
extern UINT32 brdGetFlashDriveParams	(void *ptr);
extern UINT32 brdGetTrXmcPCIDB			(void *ptr);
extern UINT32 brdCheckPcieSwConfig		(void *ptr);
extern UINT32 brdGetTrXmcParentBus		(void *ptr);
extern UINT32 brdGetIPMINMI             (void *ptr);
extern UINT32 brdClearIPMINMI           (void *ptr);
extern UINT32 brdMRAMinfo               (void *ptr);
extern UINT32 brdPostScanInit           (void *ptr);
extern UINT32 brdGetRS485PortMap        (void* ptr);
extern UINT32 brdSetSpeedStep           (void *ptr);
extern UINT32 vEnRS485_hd               (void *ptr);
extern UINT32 vEnRS485_fd               (void *ptr);
extern UINT32 vDisRS485                 (void *ptr);
extern UINT32 brdConfigCpciBackplane	(void *ptr);
extern UINT32 brdDeConfigCpciBackplane	(void *ptr);
extern UINT32 brdGetCpciBrInstance 		(void *ptr);
extern UINT32 brdCheckCpciIsSyscon 		(void *ptr);
extern UINT32 brdGetWdogInfo        	(void *ptr);
extern UINT32 brdWatchdogEnableNmi  	(void *ptr);
extern UINT32 brdWatchdogEnableReset	(void *ptr);
extern UINT32 brdWatchdogCheckSwitch	(void *ptr);
extern UINT32 brdGetBiosInfo			(void *ptr);
extern UINT32 brdGetExtendedInfo    	(void *ptr);
extern UINT32 brdGetBIOSVersion     	(void* ptr);
extern UINT32 brdGetNameString      	(void* ptr);
extern UINT32 ldtClearInt				(void *ptr);
extern UINT32 brdDS3905info         	(void *ptr);
extern UINT32 brdUnLockCCTIoRegs 		(void *ptr);
extern UINT32 brdLockCCTIoRegs    		(void *ptr);
extern UINT32 brdGetTempGradeString 	(void* ptr);
extern UINT32 brdStoreConfigChanges		(void* ptr);
extern UINT32 brdIsTPMFitted			(void *ptr);
extern UINT32 brdRestoreConfigChanges	(void* ptr);
extern UINT32 brdEnableWdLpcIoWindow	(void* ptr);
extern UINT32 brdCheckVMEBridgePresent	(void* ptr);
extern UINT32 brdGetVMEBridgelessSlot	(void* ptr);
extern UINT32 brdGetSlotId				(void* ptr);
extern UINT32 brdGetSerialNumber		(void* ptr);
extern UINT32 brdSelectSpiFwh				(void *ptr);
extern UINT32 brdWatchdogDisable (void *ptr);
extern UINT32 brdWatchdogEnable (void *ptr);
extern UINT32 brdGetVmeDeviceImages( void* ptr );
extern UINT32 brdGetTempTresholds(void *ptr);
extern UINT32 brdGetDramSize (void* ptr);
#endif /* SERVICES_H_ */
