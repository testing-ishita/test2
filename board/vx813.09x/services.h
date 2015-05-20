/*
 * services.h
 *
 *  Created on: 27 Oct 2010
 *      Author: engineer
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
extern UINT32 brdGetWdogInfo        (void *ptr);
extern UINT32 brdWatchdogEnableNmi  (void *ptr);
extern UINT32 brdWatchdogEnableReset(void *ptr);
extern UINT32 brdWatchdogCheckSwitch(void *ptr);
extern UINT32 brdGetGPIOInfo        (void *ptr);
extern UINT32 brdConfigureVxsNTPorts(void *ptr);
extern UINT32 brdGetVxsTPorts		(void *ptr);
extern UINT32 brdGetRS485PortMap    (void *ptr);
extern UINT32 brdGetSokePort 		(void *ptr);
extern UINT32 brdGetBiosInfo		(void *ptr);
extern UINT32 brdGetExtendedInfo    (void *ptr);
extern UINT32 brdGetTempGradeString (void* ptr);
extern UINT32 brdGetForceDebugOn    (void* ptr);
extern UINT32 brdGetForceDebugOff   (void* ptr);
extern UINT32 brdGetSensorThresholds(void* ptr);
extern UINT32 brdGetSthTestList 	(void* ptr);
extern UINT32 brdProcessSthTestList (void* ptr);
extern UINT32 brdGetBIOSVersion     (void* ptr);
extern UINT32 brdGetNameString      (void* ptr);
extern UINT32 brdSetSpeedStep       (void* ptr);
extern UINT32 brdGetXmcGpioInfo		(void *ptr);
extern UINT32 brdGetPciInterruptInfo(void *ptr);


#endif /* SERVICES_H_ */
