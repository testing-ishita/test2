/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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

/* sm3.h -
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/cute/sm3.h,v 1.2 2013-11-25 10:58:04 mgostling Exp $
 * $Log: sm3.h,v $
 * Revision 1.2  2013-11-25 10:58:04  mgostling
 * Added missing CVS headers.
 *
 *
 */

#ifndef SM3_H_
#define SM3_H_

void   vCommInit (void);
UINT32 wMessageGet(	UINT8  bSlot, UINT8 *pbMessage, UINT16 wMax );
UINT32 wMessagePut(	UINT8  bSlot, const UINT8 *pbMessage, UINT16 wLength,
		            UINT32 dTimeout );
UINT16 IDX_detect_message (UINT8 bSlot);
UINT8  bSoakMasterReady (void);
void   vSetPacketWaiting (void);
UINT16 	wPerformAutoId (void);
UINT32 reScanBackPlane(void);

void vParseCommandTail (UINT8* pchTail);
UINT32 vSaveCommandTail( UINT8 bSlot, UINT8* pchTail );
UINT32 vProcessSavedParams( UINT8 bSlot, saved_params *params );
void  IDX_p3_bist_exit (UINT16 wErrCode, char* pchTxt);

UINT32 bAddrToMem (UINT32 dBar);
UINT32 bMemToSlot (UINT32 dBar);

void   processSlavePackets( UINT8 bSlot, uPacket *psPkt );
void   processSlaveRxPackets( UINT8 bSlot);
UINT32 pParamReq( UINT8 bSlot, UINT8* pbData, UINT16 wMaxSize, UINT16 size );

#endif /* SM3_H_ */
