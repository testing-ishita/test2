/************************************************************************
 *                                                                      *
 *      Copyright 1999 Concurrent Technologies, all rights reserved.    *
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

/*  $Header: /home/cvs/cvsroot/CCT_BIT_2/tools.cct/source/bit_load/BIGREAL.H,v 1.1 2013-09-04 07:57:07 chippisley Exp $
 *
 *  $Log: BIGREAL.H,v $
 *  Revision 1.1  2013-09-04 07:57:07  chippisley
 *  Import files into new source repository.
 *
 *  Revision 1.1  2008/04/18 13:41:07  swilson
 *  Place under CVS control.
 *
 *  Revision 0.1  1999/11/12 11:17:18  stevew
 *  First RCS Checkin 0.01-03
 *
 */


#ifndef __bigreal_h
	#define __bigreal_h

  /* Define these types if they've not been defined elsewhere */

#ifndef UINT32
	typedef unsigned long	UINT32;
#endif

#ifndef UINT16
	typedef unsigned int	UINT16;
#endif

#ifndef UINT8
	typedef unsigned char	UINT8;
#endif


extern "C" far UINT8	bGoBigReal (void);

extern "C" far UINT32	dReadDword32 (UINT32 dAddr);
extern "C" far UINT16	wReadWord32 (UINT32 dAddr);
extern "C" far UINT8	bReadByte32 (UINT32 dAddr);

extern "C" far void		vWriteDword32 (UINT32 dAddr, UINT32 dData);
extern "C" far void		vWriteWord32 (UINT32 dAddr, UINT16 wData);
extern "C" far void		vWriteByte32 (UINT32 dAddr, UINT8 bData);

extern "C" far void		vWritePattern32 (UINT32 dAddr, UINT32 dCount);

extern "C" far void		vCopyDword32 (UINT32 dSource, UINT32 dDest, UINT32 dSize);
extern "C" far void		vSetDword32 (UINT32 dSource, UINT32 dData, UINT32 dSize);

extern "C" far UINT32	dInDword (UINT16 wPort);
extern "C" far void		vOutDword (UINT16 wPort, UINT32 dData);
extern "C" far UINT16	wInWord (UINT16 wPort);
extern "C" far void		vOutWord (UINT16 wPort, UINT16 wData);

extern "C" far void		vRmwSet32 (UINT32 dAddr, UINT32 dMask);
extern "C" far void		vRmwClear32 (UINT32 dAddr, UINT32 dMask);

extern "C" far void		vEnableCache (void);
extern "C" far void		vDisableCache (void);

extern "C" void	far vReadMsr (UINT32 dIndex, UINT32 far* dMsb,
											 UINT32 far* dLsb);
extern "C" void	far vWriteMsr (UINT32 dIndex, UINT32 dMsb, UINT32 dLsb);

extern "C" far UINT32	dCacheState (void);

extern "C" far vInitMemW (void);
extern "C" far vInitMemR (void);

extern "C" far vMemWrite (UINT32 dStart, UINT32 dLength);
extern "C" far vMemRead (UINT32 dStart, UINT32 dLength);


#endif

