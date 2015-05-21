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

/*
 * arch.h
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/cute/arch.h,v 1.5 2015-02-27 09:00:39 mgostling Exp $
 * $Log: arch.h,v $
 * Revision 1.5  2015-02-27 09:00:39  mgostling
 * Remove parameter from InitialiseVMEDevice
 *
 * Revision 1.4  2015-02-25 17:58:45  hchalla
 * Added support for new VME ethernet cute.
 *
 * Revision 1.3  2014-08-04 15:04:38  mgostling
 * Include function prototypes for SRIO architecture initialisation.
 *
 * Revision 1.2  2013-11-25 10:52:32  mgostling
 * Added missing CVS headers and support for VPX boards.
 *
 */

#ifndef ARCH_H_
#define ARCH_H_


#define INTERCONNECT_LOCAL_SLOT     0x1F

typedef struct vme_Bars
{
	volatile UINT32 LocalDev;
	volatile UINT32 Intercon;
	volatile UINT32 CrCsr;
}VMEBARS;

extern UINT32 InitialiseVMEDevice (void);
extern UINT8  IdentifyAgent (void);
extern UINT8  PerformVmeAutoId (void);
extern UINT32 bIcWriteByte (UINT8 bSlot, UINT16 offset, UINT8* data);
extern UINT32 bIcReadByte (UINT8 bSlot, UINT16 offset, UINT8* data);

UINT8  bReadIcByte (UINT8 bSlot, UINT8 bReg);
void   vWriteIcByte (UINT8 bSlot, UINT8 bReg, UINT8 bData);
UINT8  bReadIcReg (UINT8 bSlot, UINT8 bReg);
void   vWriteIcReg (UINT8 bSlot, UINT8 bReg, UINT8 bData);


UINT8  bIsPeripheral (void);
void   vVpxInit (void);
void   vDisableMemWindow (void);
UINT32 dSetMemWindow (UINT32 dAddress, UINT32 dLen);

void vCpciInit (void);
void cpciPreInit (UINT8);

void vpxPreInit (void);

void srioPreInit (UINT8 bFlag);
void rio_init_mports (void);
void tsi721_init (UINT32 dInstance);

void   GetVmeBars (VMEBARS *vme_Bars);
#ifndef VPX
UINT32 bSlotToBar (UINT8 slot);
UINT8  bAddrToSlot(UINT16 wAddr);
#endif

#endif
