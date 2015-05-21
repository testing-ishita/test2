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

extern UINT32 brdGetConsolePort 		(void *ptr);
extern UINT32 brdGetDebugPort  			(void *ptr);
extern UINT32 brdGetSccPortMap 			(void *ptr);
extern UINT32 brdGetSccParams  			(void *ptr);
extern UINT32 brdGetMaxDramBelow4GB 	(void *ptr);
extern UINT32 brdFlashLed 				(void *ptr);
extern UINT32 brdReset 					(void *ptr);
extern UINT32 brdCheckUserMthSwitch 	(void *ptr);
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
extern UINT32 brdGetFlashDriveParams	(void *ptr);
extern UINT32 brdGetTrXmcPCIDB			(void *ptr);
extern UINT32 brdCheckPcieSwConfig		(void *ptr);
extern UINT32 brdGetTrXmcParentBus		(void *ptr);
extern UINT32 brdGetIPMINMI             (void *ptr);
extern UINT32 brdClearIPMINMI           (void *ptr);
extern UINT32 brdMRAMinfo               (void *ptr);
extern UINT32 brdConfigureVPXNTPorts    (void *ptr);
extern UINT32 brdGetVpxBrInstance       (void *ptr);
extern UINT32 brdPostScanInit           (void *ptr);
extern UINT32 brdGetRS485PortMap        (void* ptr);
extern UINT32 brdSetSpeedStep           (void *ptr);
extern UINT32 vEnRS485_hd               (void *ptr);
extern UINT32 vEnRS485_fd               (void *ptr);
extern UINT32 vDisRS485                 (void *ptr);
extern UINT32 brdLdtPitIntEnable(void *ptr);




#endif /* SERVICES_H_ */
