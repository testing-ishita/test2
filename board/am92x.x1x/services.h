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

/*
 * services.h
 *
 *  Created on: 11 April 2011
 *      Author: Muthiahr
 */

#ifndef SERVICES_H_
#define SERVICES_H_

typedef struct tagServiceType
{
	UINT32	iServiceID;
	UINT32	(*pfnCmdHandler)();

} SERVICE_TYPE;

/* board_cfg.c */

extern UINT32 brdGetConsolePort 	(void *ptr);
extern UINT32 brdGetDebugPort  		(void *ptr);
extern UINT32 brdGetSccPortMap 		(void *ptr);
extern UINT32 brdGetSccParams  		(void *ptr);
extern UINT32 brdGetMaxDramBelow4GB (void *ptr);
extern UINT32 brdFlashLed 			(void *ptr);
extern UINT32 brdReset 				(void *ptr);
extern UINT32 brdCheckUserMthSwitch (void *ptr);
extern UINT32 brdSelectEthInterface (void *ptr);
extern UINT32 brdGetNumCores 		(void *ptr);
extern UINT32 brdGetFpuExceptionMap (void *ptr);
extern UINT32 brdHwInit 			(void *ptr);
extern UINT32 brdPostScanInit		(void *ptr);
extern UINT32 brdCuteInfo			(void *ptr);
extern UINT32 brdNvWrite 			(void *ptr);
extern UINT32 brdNvRead 			(void *ptr);
extern UINT32 brdPciGetParams 		(void *ptr);
extern UINT32 brdPciCfgHbStart 		(void *ptr);
extern UINT32 brdPciCfgHbEnd 		(void *ptr);
extern UINT32 brdPciRoutePirqToIrq 	(void *ptr);
extern UINT32 brdDisplaySignon 		(void *ptr);
extern UINT32 brdGetVersionInfo 	(void *ptr);
extern UINT32 brdGetTestDirectory 	(void *ptr);
extern UINT32 brdPostON				(void *ptr);
extern UINT32 brdPostOFF			(void *ptr);
extern UINT32 brdAppFlashGetParams 	(void *ptr);
extern UINT32 brdAppFlashSelectPage (void *ptr);
extern UINT32 brdAppFlashEnable 	(void *ptr);
extern UINT32 brdGetSuperIO			(void *ptr);
extern UINT32 brdGetSuperIONum 		(void *ptr);
extern UINT32 brdGetPCIDB			(void *ptr);
extern UINT32 brdGetSCR				(void *ptr);
extern UINT32 brdLdtPitInfo 		(void *ptr);
extern UINT32 brdEEPROMinfo			(void *ptr);
extern UINT32 brdSMBUSinfo			(void *ptr);
extern UINT32 brdGetTempinfo		(void *ptr);
extern UINT32 brdLEDinfo			(void *ptr);
extern UINT32 brdSpiEEPROMInfo 		(void *ptr);
extern UINT32 brdSerialBaudInfo     (void *ptr);
extern UINT32 brdInitIcr 			(void *ptr);
extern UINT32 brdGetCoopTestTable   (void *ptr);
extern UINT32 vEnRS485_fd			(void *ptr);
extern UINT32 vDisRS485				(void *ptr);
extern UINT32 vEnRS485_hd			(void *ptr);
extern UINT32 brdCheckUniverseJumper(void *ptr);
extern UINT32 brdGetSokePort        (void *ptr);
extern UINT32 brdGetWdogInfo        (void *ptr);
extern UINT32 brdWatchdogEnableNmi  (void *ptr);
extern UINT32 brdWatchdogEnableReset(void *ptr);
extern UINT32 brdWatchdogCheckSwitch(void *ptr);
extern UINT32 brdGetGPIOInfo        (void *ptr);
extern UINT32 brdGetIPMINMI         (void *ptr);
extern UINT32 brdSkipEthInterface   (void *ptr);
extern UINT32 brdMemfromGPIO	    (void *ptr);
extern UINT32 brdConfigCpciBackplane(void *ptr);
extern UINT32 brdDeConfigCpciBackplane(void *ptr);
extern UINT32 brdGetCpciBrInstance (void *ptr);
extern UINT32 brdCheckCpciIsSyscon (void *ptr);
extern UINT32 brdclearIPMINMI 	   (void *ptr);
extern UINT32 brdSkipEthTest 	   (void *ptr);
extern UINT32 brdSkipIOResourceAllocation 	   (void *ptr);
extern UINT32 ldtClearInt                  (void *ptr);
extern UINT32 brdSetSpeedStep           (void *ptr);
extern UINT32 brdStoreConfigChanges(void* ptr);
extern UINT32 brdRestoreConfigChanges(void* ptr);
extern UINT32 brdEnableSmicLpcIoWindow(void* ptr);
extern UINT32 brdGetBiosInfo		(void *ptr);
extern UINT32 brdGetExtendedInfo    (void *ptr);
extern UINT32 brdGetSlotId( void *ptr );

#endif /* SERVICES_H_ */
