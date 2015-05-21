
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

/* nvram.c - board-specific non-volatile storage management
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/am92x.x1x/nvram.c,v 1.1 2013-09-04 07:13:28 chippisley Exp $
 *
 * $Log: nvram.c,v $
 * Revision 1.1  2013-09-04 07:13:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */

/* includes */

#include <stdtypes.h>
 
#include <bit/board_service.h>
#include <private/port_io.h>


/* defines */

#define CMOS0_INDEX		0x70
#define CMOS0_DATA		0x71

#define CMOS1_INDEX		0x72
#define CMOS1_DATA		0x73

#define CMOS_NMI_OFF	0x80

	/* Fixed CMOS nodes */

#define CMOS_XSA			    0x70		/* was 35 NV0 */
#define  M_XSA_USER				0x01
#define  S_XSA_USER				0
#define  M_XSA_BOARDMODE		0x06
#define  S_XSA_BOARDMODE		1
#define  M_XSA_BOOTMODE			0x18
#define  S_XSA_BOOTMODE			3
#define  M_XSA_BITRESULTVALID	0x40
#define  S_XSA_BITRESULTVALID	6
#define  M_XSA_BRDTYPEVALID		0x80
#define  S_XSA_BRDTYPEVALID		7

#define CMOS_BOARDTYPE		0x36

#define CMOS_TESTNUMBER_L	0xE0		/* NV1 */
#define CMOS_TESTNUMBER_H	0xE1

#define CMOS_ERRORCODE_LL	0xE2
#define CMOS_ERRORCODE_LH	0xE3
#define CMOS_ERRORCODE_HL	0xE4
#define CMOS_ERRORCODE_HH	0xE5


/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */

static void vWriteCmos (UINT8 bRegister, UINT8 bMask, UINT8 bShift, UINT8 bData);
static UINT8 bReadCmos (UINT8 bRegister, UINT8 bMask, UINT8 bShift);



/*****************************************************************************
 * brdNvWrite: write data to NVRAM storage
 *
 * This board exclusively uses CMOS storage, via hard-coded nodes.
 *
 * RETURNS: none
 */
//void brdNvWrite( UINT32	dNvToken, UINT32 dData )
UINT32 brdNvWrite( void *ptr )
{
	switch (((NV_RW_Data*)ptr)->dNvToken)
	{
		case NV__TEST_NUMBER :

			vWriteCmos (CMOS_TESTNUMBER_L, 0xFF, 0, (UINT8) ((NV_RW_Data*)ptr)->dData);
			vWriteCmos (CMOS_TESTNUMBER_H, 0xFF, 0, (UINT8)(((NV_RW_Data*)ptr)->dData >> 8));
			break;

		case NV__ERROR_CODE :

			vWriteCmos (CMOS_ERRORCODE_LL, 0xFF, 0, (UINT8) ((NV_RW_Data*)ptr)->dData       );
			vWriteCmos (CMOS_ERRORCODE_LH, 0xFF, 0, (UINT8)(((NV_RW_Data*)ptr)->dData  >> 8 ));
			vWriteCmos (CMOS_ERRORCODE_HL, 0xFF, 0, (UINT8)(((NV_RW_Data*)ptr)->dData  >> 16));
			vWriteCmos (CMOS_ERRORCODE_HH, 0xFF, 0, (UINT8)(((NV_RW_Data*)ptr)->dData  >> 24));
			break;

		case NV__BIOS_BOOT :

			vWriteCmos (CMOS_XSA, M_XSA_USER, S_XSA_USER, (UINT8)((NV_RW_Data*)ptr)->dData );
			break;

		case NV__RESULT_VALID :

			vWriteCmos (CMOS_XSA, M_XSA_BITRESULTVALID, S_XSA_BITRESULTVALID, 
						 (UINT8)((NV_RW_Data*)ptr)->dData  );
			break;

		default:
			break;
	};

	return E__OK;

} /* brdNvWrite () */


/*****************************************************************************
 * brdNvRead: read data from NVRAM storage
 *
 * This board exclusively uses CMOS storage, via hard-coded nodes.
 *
 * RETURNS: data, masked to appropriate width
 */

UINT32 brdNvRead( void *ptr )
{
	switch (((NV_RW_Data*)ptr)->dNvToken)
	{
		case NV__TEST_NUMBER :

			((NV_RW_Data*)ptr)->dData = bReadCmos (CMOS_TESTNUMBER_H, 0xFF, 0);
			((NV_RW_Data*)ptr)->dData = (((NV_RW_Data*)ptr)->dData << 8) | bReadCmos (CMOS_TESTNUMBER_L, 0xFF, 0);
			break;

		case NV__ERROR_CODE :

			((NV_RW_Data*)ptr)->dData = bReadCmos (CMOS_ERRORCODE_HH, 0xFF, 0);
			((NV_RW_Data*)ptr)->dData = (((NV_RW_Data*)ptr)->dData << 8) | bReadCmos (CMOS_ERRORCODE_HL, 0xFF, 0);
			((NV_RW_Data*)ptr)->dData = (((NV_RW_Data*)ptr)->dData << 8) | bReadCmos (CMOS_ERRORCODE_LH, 0xFF, 0);
			((NV_RW_Data*)ptr)->dData = (((NV_RW_Data*)ptr)->dData << 8) | bReadCmos (CMOS_ERRORCODE_LL, 0xFF, 0);
			break;

		case NV__BIOS_BOOT :

			((NV_RW_Data*)ptr)->dData = bReadCmos (CMOS_XSA, M_XSA_USER, S_XSA_USER);
			break;

		case NV__RESULT_VALID :

			((NV_RW_Data*)ptr)->dData = bReadCmos (CMOS_XSA, M_XSA_BITRESULTVALID, S_XSA_BITRESULTVALID);
			break;

		default:
			break;
	};

	return E__OK;

} /* brdNvRead () */


/*****************************************************************************
 * vWriteCmos: write data to CMOS
 *
 * It is assumed that registers 0-127 are in Bank-0, the remainder in Bank-1
 *
 * RETURNS: None
 */

static void vWriteCmos
(
	UINT8	bRegister,
	UINT8	bMask,
	UINT8	bShift,
	UINT8	bData
)
{
	UINT8	bTemp;

	if (bRegister < 128)
	{
			sysOutPort8 (CMOS0_INDEX, bRegister | CMOS_NMI_OFF);
			bTemp = sysInPort8 (CMOS0_DATA);
			bTemp = (bTemp & ~bMask) | (bData << bShift);
			sysOutPort8 (CMOS0_DATA, bTemp);
	}

	else
	{
			bRegister -= 128;

			sysOutPort8 (CMOS1_INDEX, bRegister | CMOS_NMI_OFF);
			bTemp = sysInPort8 (CMOS1_DATA);
			bTemp = (bTemp & ~bMask) | (bData << bShift);
			sysOutPort8 (CMOS1_DATA, bTemp);
	}

} /* vWriteCmos () */


/*****************************************************************************
 * bReadCmos: write data to CMOS
 *
 * It is assumed that registers 0-127 are in Bank-0, the remainder in Bank-1
 *
 * RETURNS: masked, byte value
 */

static UINT8 bReadCmos
(
	UINT8	bRegister,
	UINT8	bMask,
	UINT8	bShift
)
{
	UINT8	bTemp;

	if (bRegister < 128)
	{
			sysOutPort8 (CMOS0_INDEX, bRegister | CMOS_NMI_OFF);
			bTemp = sysInPort8 (CMOS0_DATA);
	}

	else
	{
			bRegister -= 128;

			sysOutPort8 (CMOS1_INDEX, bRegister | CMOS_NMI_OFF);
			bTemp = sysInPort8 (CMOS1_DATA);
	}

	return ((bTemp & bMask) >> bShift);

} /* bReadCmos () */



