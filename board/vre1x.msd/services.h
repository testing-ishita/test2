/************************************************************************
 *                                                                      *
 *      Copyright 2010 Concurrent Technologies, all rights reserved.    *
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

/* services.h - function prototypes for board service functions
 */

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/services.h,v 1.7 2014-07-29 11:39:29 mgostling Exp $
*
* $Log: services.h,v $
* Revision 1.7  2014-07-29 11:39:29  mgostling
* Implement SERVICE__BRD_SKIP_ETH_IF so that the network interface
* to the Universal Soak Master is not modified during ethernet device testing.
*
* Revision 1.6  2014-07-24 14:29:12  mgostling
* Added brdGetSlotId() protoype declaration.
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
extern UINT32 brdGetSokePort 			(void *ptr);
extern UINT32 brdSelectEthInterface		(void *ptr);
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
#ifdef CUTE
extern UINT32 brdInitIcr 				(void *ptr);
#endif
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
extern UINT32 brdGetBiosInfo			(void *ptr);
extern UINT32 brdGetExtendedInfo    	(void *ptr);
extern UINT32 brdGetBIOSVersion     	(void* ptr);
extern UINT32 brdGetNameString      	(void* ptr);
extern UINT32 brdGetTempGradeString 	(void* ptr);
extern UINT32 brdLdtPitIntEnable		(void *ptr);
extern UINT32 brdDecodeTpmIo			(void *ptr);
extern UINT32 brdDecodeSmicIo			(void *ptr);
extern UINT32 brdEnableHDA				(void *ptr);
extern UINT32 brdGetDramSize			(void *ptr);
extern UINT32 brdGetHdaList				(void *ptr);
extern UINT32 brdGetRtmPex8733			(void *ptr);
extern UINT32 brdGetXmcGpioInfo			(void *ptr);
extern UINT32 brdGetPciInterruptInfo	(void *ptr);
extern UINT32 brdGetSlotId				(void *ptr);
extern UINT32 brdSkipEthInterface		(void *ptr);

#endif /* SERVICES_H_ */
