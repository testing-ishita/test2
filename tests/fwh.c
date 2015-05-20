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

/* fwh.c - FWH test, which tests the FWH device
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/fwh.c,v 1.2 2013-10-08 07:13:38 chippisley Exp $
 *
 * $Log: fwh.c,v $
 * Revision 1.2  2013-10-08 07:13:38  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.7  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.6  2010/03/31 16:27:10  hmuneer
 * no message
 *
 * Revision 1.5  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.4  2009/06/09 15:00:20  jthiru
 * Added support for MCH firmware hub detection
 *
 * Revision 1.3  2009/05/22 08:00:52  cvsuser
 * Reverted vGetMemorySize parameter
 *
 * Revision 1.2  2009/05/22 07:47:06  cvsuser
 * Removed warnings
 *
 * Revision 1.1  2009/05/21 08:06:37  cvsuser
 * Initial checkin for FWH test
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <errors.h>		
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <devices/ide.h>
#include <bit/hal.h>
 
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>
#include <bit/console.h>
#include <devices/fwh.h>


//#define DEBUG
#ifdef DEBUG
		static char buffer[128];
#endif



/* typedefs */

typedef struct tagDbMem						/* Memory Data Record */
{
	char	achMem[25];						/* Memory description */

	UINT32	dBaseAddr;						/* Base address of memory */
	UINT32	dMemSize;						/* Size of memory */
	UINT8	bMemWidth;						/* Memory width (devices) */
	UINT8	bMemDepth;						/* Memory depth (devices) */

	UINT8	bRomClass;						/* general classification of device */

	UINT32	dPageSize;						/* size of page/hole for accessing memory */
	UINT8	bPageAlg;						/* Page-select algorithm */
	UINT8	bPageAlgParams;					/* Parameters for algorithm selected above */
	UINT32	adPageParams[MAX_PARAMS];		/* Parameters for algorithm selected above */
	UINT8	bDevAlg;						/* Device-select algorithm */
	UINT8	bDevAlgParams;					/* Parameters for algorithm selected above */
	UINT32	adDevParams[MAX_PARAMS];		/* Parameters for algorithm selected above */
	UINT8	bBaseAddrAlg;					/* Base Address-select algorithm */
	UINT8	bBaseAddrAlgParams;				/* Parameters for algorithm selected above */
	UINT32	adBaseAddrParams[MAX_PARAMS];	/* Parameters for algorithm selected above */

	/* This field is completed by device auto-detect */

	UINT16	wDevSectorCount;		/* sectors in a single device */

	/* This field is completed when operating on memory */

	UINT32	dVirtualAddr;
	
} DB_MEM;

typedef	enum	_CS_CTRL
{
	CS_DISABLE	= 0,
	CS_ENABLE

}	CS_CTRL;

typedef struct tagChipset
{
	const char*	achNorthBridge;
	const char*	achSouthBridge;

	void 		(*pfChipsetMemSize)(UINT32* pdMemSize);

	void 		(*pfChipsetWrite)(CS_CTRL ctrl);
	void 		(*pfChipsetHole)(CS_CTRL ctrl);
	UINT32	dBaseAddr;
	UINT8  	bMemWidth;
	UINT8	bRomClass;
}	CHIPSET;


typedef void (*DETECT_FN)(CHIPSET* psChipset);
typedef UINT32 (*DEV_DETECT_FN)(CHIPSET* psChipset);


typedef UINT32	(*PAGE_ALG_FN) (DB_MEM*, UINT32, UINT32*);
typedef void	(*DEVICE_ALG_FN) (DB_MEM*, UINT8, UINT32*);
typedef	UINT32	(*BASEADDR_ALG_FN) (DB_MEM*);

typedef struct tagSector
{
	UINT16		wCount;			// number of sectors
	UINT32		dSize;			// size of sector(s)

} SECTOR;

typedef struct tagSfDevice
{
	UINT8		bManufId;
	UINT8		bDeviceId;
	const char*	achName;
	UINT32		dSectorSize;
	UINT16		wNumSectors;

} FWH_DEVICE;


/* constants */

static const char	achDeviceTitle430[] = "Intel 430HX";
static	const char	achDeviceTitle440[] = "Intel 440BX";
static	const char	achDeviceTitle440gx[] = "Intel 440GX";
static	const char	achDeviceTitle855[] = "Intel 855GME";
static	const char	achDeviceTitle7520[] = "Intel E7520";
static	const char	achDeviceTitle945gm[] = "Intel 945GM";
static	const char	achDeviceTitlele3100[] = "Intel LE3100";
static	const char	achDeviceTitlepiix[] = "Intel PIIX4E";
static	const char	achDeviceTitle6300esb[] = "Intel 6300ESB";
static	const char	achDeviceTitleich7[] = "Intel ICH7";
static  const char  achDeviceTitleich9[] = "Intel ICH9M";
static  const char  achDeviceTitlegm45[] ="Intel GM45";
static  const char  achDeviceTitlesch[] = "Intel SCH" ;

static	const char achDeviceNameI802AC[]  = "Intel 82802AC";
static	const char achDeviceNameLW080[]   = "Atmel AT49LW080";
static	const char achDeviceNameFW080[]   = "ST Micro M50FW080";
static const char   at26df161a [] = "SPI AT26DF161A through custom FWH" ; 

static	const FWH_DEVICE asDeviceTypes[] =
{
	{ INTEL_MID,  I802AC_DID, achDeviceNameI802AC, FWH8_SEC_SIZE, FWH8_NUM_SEC },
	{ ATMEL_MID,  LW080_DID,  achDeviceNameLW080,  FWH8_SEC_SIZE, FWH8_NUM_SEC },
	{ STM_MID,    FW080_DID,  achDeviceNameFW080,  FWH8_SEC_SIZE, FWH8_NUM_SEC }
};



/* locals */

/* globals */

/* externals */
extern void vConsoleWrite(char*	achMessage);

/* forward declarations */
static void v430hx_MemSize (UINT32* pdMemSize);
static void v430hx_MemHole (CS_CTRL ctrl);
static void v440bx_MemSize (UINT32* pdMemSize);
static void v440bx_MemHole (CS_CTRL ctrl);
static void v440gx_MemSize (UINT32* pdMemSize);
static void v440gx_MemHole (CS_CTRL ctrl);
static void v6300esb_WriteControl (CS_CTRL ctrl);
static void v855gme_MemSize (UINT32* pdMemSize);
static void v945gm_MemSize (UINT32* pdMemSize);
static void vE7520_MemSize (UINT32* pdMemSize);
static void vLE3100_MemSize (UINT32* pdMemSize);
static void vLE3100_WriteControl (CS_CTRL ctrl);
static void vIch7_WriteControl (CS_CTRL ctrl);
static void vPiix4e_WriteControl (CS_CTRL ctrl);
static void vGm45_MemSize( UINT32* pdMemSize );



UINT32 dPciReadReg1(UINT32 bus, UINT32 dev, UINT32 fn, UINT8 bReg, REG_SIZE size)
{
	PCI_PFA pfa;
	UINT32 dresult;

	pfa = PCI_MAKE_PFA (bus, dev, fn);
	dresult = dPciReadReg(pfa, bReg, size);
	return dresult;
}


void vPciWriteReg1(UINT32 bus, UINT32 dev, UINT32 fn, UINT8 bReg, REG_SIZE size, UINT32 dData)
{
	PCI_PFA pfa;

	pfa = PCI_MAKE_PFA (bus, dev, fn);
	vPciWriteReg(pfa, bReg, size, dData);
}


void	vWriteByte32( UINT32 dAddr, UINT8 bData )
{
	*( (UINT8 *) dAddr )	= bData;
}


UINT8	bReadByte32( UINT32 dAddr )
{
	return *( (UINT8 *) dAddr );
}

void v440bx_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check North Bridge */

	wVid = (UINT16)dPciReadReg1 (0, I440BX_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, I440BX_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try 440BX: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ( (wVid != VID_440BX) ||
			((wDid != DID_440BX) && (wDid != DID_440BX_AGP)) )
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achNorthBridge	= achDeviceTitle430;
	psChipset->pfChipsetMemSize = v440bx_MemSize;
	psChipset->pfChipsetHole	= v440bx_MemHole;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* w440bx_Detect () */


static void v440bx_MemSize
(
	UINT32*	pdMemSize
)
{
	UINT8	bRegVal;


	bRegVal = (UINT8)dPciReadReg1 (0, I440BX_DEV, 0, DRB7_440bx, PCI_BYTE);

	*pdMemSize = (UINT32)bRegVal * SIZE_8MB;

} /* v440bx_MemSize () */


static void v440bx_MemHole
(
	CS_CTRL ctrl
)
{
	UINT8	bTemp;


	if (ctrl == CS_ENABLE)
	{
		/* Enable hole at 15MB-16MB */

		bTemp = (UINT8)dPciReadReg1 (0, I440BX_DEV, 0, FDHC_430bx, PCI_BYTE);
		vPciWriteReg1 (0, I440BX_DEV, 0, FDHC_430bx, PCI_BYTE,
						(bTemp & HEN_MASK_430bx) | HEN_15MB_430bx);
	}

	else	/* Disable hole */
	{
		bTemp = (UINT8)dPciReadReg1 (0, I440BX_DEV, 0, FDHC_430bx, PCI_BYTE);
		vPciWriteReg1 (0, I440BX_DEV, 0, FDHC_430bx, PCI_BYTE,
						bTemp & HEN_MASK_430bx);
	}

} /* v440bx_MemHole () */


void v430hx_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check North Bridge */

	wVid = (UINT16)dPciReadReg1 (0, I430HX_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, I430HX_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try 430HX: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ((wVid != VID_430HX) || (wDid != DID_430HX))
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achNorthBridge	= achDeviceTitle440;
	psChipset->pfChipsetMemSize = v430hx_MemSize;
	psChipset->pfChipsetHole	= v430hx_MemHole;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* w430hx_Detect () */


static void v430hx_MemSize
(
	UINT32*	pdMemSize
)
{
	UINT8	bRegVal;


	bRegVal = (UINT8)dPciReadReg1 (0, I430HX_DEV, 0, DRB7_440bx, PCI_BYTE);

	*pdMemSize = (UINT32)bRegVal * SIZE_4MB;

} /* v430hx_MemSize () */


static void v430hx_MemHole
(
	CS_CTRL ctrl
)
{
	UINT8	bTemp;


	if (ctrl == CS_ENABLE)
	{
		/* Enable hole at 15MB-16MB */

		bTemp = (UINT8)dPciReadReg1 (0, I430HX_DEV, 0, DRAMC, PCI_BYTE);
		vPciWriteReg1 (0, I430HX_DEV, 0, DRAMC, PCI_BYTE,
						(bTemp & HEN_MASK_430bx) | HEN_15MB_430bx);
	}

	else	/* Disable hole */
	{
		bTemp = (UINT8)dPciReadReg1 (0, I430HX_DEV, 0, DRAMC, PCI_BYTE);
		vPciWriteReg1 (0, I430HX_DEV, 0, DRAMC, PCI_BYTE,
						bTemp & HEN_MASK_430bx);
	}

} /* v430hx_MemHole () */


void v440gx_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check North Bridge */

	wVid = (UINT16)dPciReadReg1 (0, I440GX_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, I440GX_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try 440GX: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ( (wVid != VID_440GX) ||
			((wDid != DID_440GX) && (wDid != DID_440GX_AGP)) )
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achNorthBridge = achDeviceTitle440gx;
	psChipset->pfChipsetMemSize = v440gx_MemSize;
	psChipset->pfChipsetHole	= v440gx_MemHole;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* v440gx_Detect () */


static void v440gx_MemSize
(
	UINT32*	pdMemSize
)
{
	UINT8	bRegVal;


	bRegVal = (UINT8)dPciReadReg1 (0, I440GX_DEV, 0, DRB7_440bx, PCI_BYTE);

	*pdMemSize = (UINT32)bRegVal * SIZE_8MB;

} /* v440gx_MemSize () */


static void v440gx_MemHole
(
	CS_CTRL ctrl
)
{
	UINT8	bTemp;


	if (ctrl == CS_ENABLE)
	{
		/* Enable hole at 15MB-16MB */

		bTemp = (UINT8)dPciReadReg1 (0, I440GX_DEV, 0, FDHC_430bx, PCI_BYTE);
		vPciWriteReg1 (0, I440GX_DEV, 0, FDHC_430bx, PCI_BYTE,
						bTemp | HEN_15MB_430bx);
	}

	else	/* Disable hole */
	{
		bTemp = (UINT8)dPciReadReg1 (0, I440GX_DEV, 0, FDHC_430bx, PCI_BYTE);
		vPciWriteReg1 (0, I440GX_DEV, 0, FDHC_430bx, PCI_BYTE,
						bTemp & ~HEN_MASK_430bx);
	}

} /* v440gx_MemHole () */


void v855gme_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check North Bridge */

	wVid = (UINT16)dPciReadReg1 (0, I855GME_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, I855GME_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try 855GME: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ((wVid != VID_855GME) || (wDid != DID_855GME))
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achNorthBridge = achDeviceTitle855;
	psChipset->pfChipsetMemSize = v855gme_MemSize;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* v855gme_Detect () */


static void v855gme_MemSize
(
	UINT32*	pdMemSize
)
{
	UINT8	bRegVal;


	bRegVal = (UINT8)dPciReadReg1 (0, I855GME_DEV, I855GME_MEM_FN, DRB3, PCI_BYTE);

	*pdMemSize = (UINT32)bRegVal * SIZE_32MB;

} /* v855gme_MemSize () */


void vE7520_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check North Bridge */

	wVid = (UINT16)dPciReadReg1 (0, E7520_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, E7520_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try E7520: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ((wVid != VID_E7520) || (wDid != DID_E7520))
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achNorthBridge = achDeviceTitle7520;
	psChipset->pfChipsetMemSize = vE7520_MemSize;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* vE7520_Detect () */


static void vE7520_MemSize
(
	UINT32*	pdMemSize
)
{
	UINT16	wRegVal;

	wRegVal = (UINT16)dPciReadReg1 (0, E7520_DEV, 0, TOLM, PCI_WORD);

	*pdMemSize = (UINT32)(wRegVal & 0xF800) * SIZE_64KB;

} /* vE7520_MemSize () */



void v945gm_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check North Bridge */

	wVid = (UINT16)dPciReadReg1 (0, I945GM_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, I945GM_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try 945GM: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ((wVid != VID_945GM) || ((wDid != DID_945GM) && (wDid != DID_945GM_2)))
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achNorthBridge = achDeviceTitle945gm;
	psChipset->pfChipsetMemSize = v945gm_MemSize;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* v945gm_Detect () */


static void v945gm_MemSize
(
	UINT32*	pdMemSize
)
{
	UINT8	bRegVal;


	bRegVal = (UINT8)dPciReadReg1 (0, I945GM_DEV, 0, TOLUD_945, PCI_BYTE);

	*pdMemSize = (UINT32)(bRegVal & 0xF8) * SIZE_16MB;

} /* v945gm_MemSize () */


void vLE3100_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check North Bridge */

	wVid = (UINT16)dPciReadReg1 (0, LE3100_DEV0, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, LE3100_DEV0, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try LE3100: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ((wVid != VID_LE3100) || (wDid != DID_LE3100))
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achNorthBridge = achDeviceTitlele3100;	// must have NB and SB defined
	psChipset->achSouthBridge = achDeviceTitlele3100;	// else will get an error

	psChipset->pfChipsetMemSize = vLE3100_MemSize;

	psChipset->pfChipsetWrite = vLE3100_WriteControl;

	psChipset->bMemWidth = 1;

	psChipset->dBaseAddr = 0xFFF00000;

	psChipset->bRomClass = CLASS_FWH;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* vLE3100_Detect () */


static void vLE3100_MemSize
(
	UINT32*	pdMemSize
)
{
	UINT16	wRegVal;

	wRegVal = (UINT16)dPciReadReg1 (0, LE3100_DEV0, 0, TOLM, PCI_WORD);

	*pdMemSize = (UINT32)(wRegVal & 0xF800) * SIZE_64KB;

} /* vLE3100_MemSize () */


static void vLE3100_WriteControl
(
	CS_CTRL ctrl
)
{
	UINT8	bTemp;


	if (ctrl == CS_ENABLE)
	{
		/* Enable writes to the BIOS ROM */

		bTemp = (UINT8)dPciReadReg1 (0, LE3100_DEV31, 0, 0xDC, PCI_BYTE);
		vPciWriteReg1 (0, LE3100_DEV31, 0, 0xDC, PCI_BYTE, bTemp | 0x01);
	}

	else	/* Disable writes to the BIOS ROM */
	{
	}

} /* vLE3100_WriteControl () */


void vPiix4e_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check South Bridge */

	wVid = (UINT16)dPciReadReg1 (0, PIIX4E_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, PIIX4E_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try PIIX4E: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ((wVid != VID_PIIX4E) || (wDid != DID_PIIX4E))
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achSouthBridge = achDeviceTitlepiix;
	psChipset->pfChipsetWrite = vPiix4e_WriteControl;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* vPiix4e_Detect () */


static void vPiix4e_WriteControl
(
	CS_CTRL ctrl
)
{
	UINT8	bTemp;


	if (ctrl == CS_ENABLE)
	{
		/* Enable writes to the BIOS ROM via PIIX4E register */

		bTemp = (UINT8)dPciReadReg1 (0, PIIX4E_DEV, 0, XBCS, PCI_BYTE);
		vPciWriteReg1 (0, PIIX4E_DEV, 0, XBCS, PCI_BYTE,
						bTemp | (XTD_BIOS_EN | LOW_BIOS_EN | BIOS_WE));
	}

	else	/* Disable writes to the BIOS ROM via PIIX4E register */
	{
		bTemp = (UINT8)dPciReadReg1 (0, PIIX4E_DEV, 0, XBCS, PCI_BYTE);
		vPciWriteReg1 (0, PIIX4E_DEV, 0, XBCS, PCI_BYTE,
						bTemp & ~(XTD_BIOS_EN | BIOS_WE));
	}

} /* vPiix4e_WriteControl () */


void v6300esb_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check South Bridge */

	wVid = (UINT16)dPciReadReg1 (0, i6300ESB_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, i6300ESB_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try 6300ESB: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ((wVid != VID_6300ESB) || (wDid != DID_6300ESB))
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achSouthBridge = achDeviceTitle6300esb;
	psChipset->pfChipsetWrite = v6300esb_WriteControl;
	psChipset->bMemWidth = 1;

	psChipset->dBaseAddr = 0xFFF00000;

	psChipset->bRomClass = CLASS_FWH;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* v6300esb_Detect () */


static void v6300esb_WriteControl
(
	CS_CTRL ctrl
)
{
	UINT16	wTemp;


	if (ctrl == CS_ENABLE)
	{
		/* Enable writes to the BIOS ROM */

		wTemp = (UINT8)dPciReadReg1 (0, i6300ESB_DEV, 0, 0x4E, PCI_WORD);
		vPciWriteReg1 (0, i6300ESB_DEV, 0, 0x4E, PCI_WORD, wTemp | 0x0001);
	}

	else	/* Disable writes to the BIOS ROM */
	{
	}

} /* v6300esb_WriteControl () */


void vIch7_Detect
(
	CHIPSET*	psChipset
)
{
	UINT16	wVid;
	UINT16	wDid;


	/* Check South Bridge */

	wVid = (UINT16)dPciReadReg1 (0, ICH7_DEV, 0, PCI_VID, PCI_WORD);
	wDid = (UINT16)dPciReadReg1 (0, ICH7_DEV, 0, PCI_DID, PCI_WORD);

#ifdef DEBUG
	sprintf (buffer, ". Try ICH7M: VID=0x%04X, DID=0x%04X ... ", wVid, wDid);
	vConsoleWrite(buffer);
#endif

	if ( (wVid != VID_ICH7) || ((wDid != DID_ICH7) && (wDid != DID_ICH7R)) )
	{
#ifdef DEBUG
		vConsoleWrite ("fail\n");
#endif
		return;
	}

	/* Detected: so install structure members */

	psChipset->achSouthBridge = achDeviceTitleich7;
	psChipset->pfChipsetWrite = vIch7_WriteControl;

#ifdef DEBUG
	vConsoleWrite ("pass\n");
#endif

} /* vIch7_Detect () */


static void vIch7_WriteControl
(
	CS_CTRL ctrl
)
{
	UINT8	bTemp;


	if (ctrl == CS_ENABLE)
	{
		/* Enable writes to the BIOS ROM */

		bTemp = (UINT8)dPciReadReg1 (0, ICH7_DEV, 0, 0xDC, PCI_BYTE);
		vPciWriteReg1 (0, ICH7_DEV, 0, 0xDC, PCI_BYTE, bTemp | 0x0001);
	}

	else	/* Disable writes to the BIOS ROM */
	{
	}

} /* vIch7_WriteControl () */


static void vIch9_WriteControl
(
	CS_CTRL ctrl
)
{
	UINT8 wTemp;

	if (ctrl == CS_ENABLE)
	{
		/* Enable writes to the BIOS ROM */
		wTemp = (UINT8)dPciReadReg1 (0, ICH9M_DEV, 0, 0xDC, PCI_BYTE);
		vPciWriteReg1 (0, ICH9M_DEV, 0, 0xDC, PCI_BYTE, wTemp | 0x01);
		
		vPciWriteReg1 (0, ICH9M_DEV, 0, 0xD8, PCI_WORD, 0xFFCF);
	}

	else	/* Disable writes to the BIOS ROM */
	{
	}

}

void vIch9_Detect( CHIPSET* psChipset )
{
	// UINT16 wVid;
	UINT16 wDid;

	// wVid = (UINT16)dPciReadReg1 (0, ICH9M_DEV, 0, PCI_VID, PCI_WORD);
    wDid = (UINT16)dPciReadReg1 (0, ICH9M_DEV, 0, PCI_DID, PCI_WORD);

	switch(wDid)
	{
		case ICH9M_1:
		case ICH9M_2:
		case ICH9M_3:
			break;
		default:
#ifdef DEBUG
			vConsoleWrite("Unable to detect ICH9 South Bridge\n");
#endif
			return;
 	}

	psChipset->achSouthBridge = achDeviceTitleich9;
    psChipset->pfChipsetWrite = vIch9_WriteControl;
}



void vGm45_Detect( CHIPSET* psChipset )
{
	// UINT16  wVid;
    UINT16  wDid;


        /* Check North Bridge */

    // wVid = (UINT16)dPciReadReg1 (0, GM45_DEV, 0, PCI_VID, PCI_WORD);
    wDid = (UINT16)dPciReadReg1 (0, GM45_DEV, 0, PCI_DID, PCI_WORD);

	switch(wDid)
	{
		case GM45_DID1:
		case GM45_DID2:
			break;
		default:
#ifdef DEBUG
		vConsoleWrite("North bridge not of GM45 type\n");
#endif	
			break;
	}

	psChipset->achNorthBridge = achDeviceTitlegm45;
	psChipset->pfChipsetMemSize = vGm45_MemSize;

}

static void vGm45_MemSize( UINT32* pdMemSize )
{
	UINT16	bRegVal;


	bRegVal = (UINT16)dPciReadReg1 (0, GM45_DEV, 0, TOLUD_gm45, PCI_WORD);

	*pdMemSize = (UINT32)((bRegVal & 0xFFF8) >> 4) * SIZE_1MB;

}


static void schmemSize (UINT32* pdMemSize)
{
#if 0
	UINT16	bRegVal;


	bRegVal = (UINT16)dPciReadReg1 (0, GM45_DEV, 0, TOLUD, PCI_WORD);

	*pdMemSize = (UINT32)((bRegVal & 0xFFF8) >> 4) * SIZE_1MB;
#endif

    *pdMemSize = 0x200000L ;

} /* End of function memSize() */


static void schwriteControl (CS_CTRL ctrl)
{
    if (ctrl == CS_ENABLE)
    {
       vPciWriteReg1 (0, SCH_DEV, 0, 0xD8, PCI_WORD, 0x0001);
    }
    else	/* Disable writes to the BIOS ROM */
    {
    }

} /* End of function writeControl() */


void vSch_Detect (CHIPSET* psChipset)
{
    UINT16 wVid ;
    UINT16 wDid ;

    wVid = (UINT16)dPciReadReg1 (0, SCH_DEV, 0, PCI_VID, PCI_WORD) ;
    wDid = (UINT16)dPciReadReg1 (0, SCH_DEV, 0, PCI_DID, PCI_WORD) ;

    if ((wVid != INTEL_VID) || (wDid != SCH_DEVID))
       return ;

    psChipset->achNorthBridge   = achDeviceTitlesch ;
    psChipset->achSouthBridge   = achDeviceTitlesch ;
    psChipset->pfChipsetMemSize = schmemSize ;
	psChipset->pfChipsetWrite = schwriteControl;
	psChipset->bMemWidth = 1;
	psChipset->dBaseAddr = 0xFFE00000;
	psChipset->bRomClass = CLASS_FWH_SPI;

} /* End of function vSch_Detect() */







/**************************************************************************************************************************/



/*****************************************************************************
 * vFwh_ResetDevice: reset device to read mode
 *
 * Returns: None
 */

static void vFwh_ResetDevice
(
	UINT32	dBaseAddr,
	UINT8	bStepSize
)
{
	vWriteByte32 (dBaseAddr, FWH_CLR_STATUS);	/* clear status */
	vWriteByte32 (dBaseAddr, FWH_RESET);

} /* vFwh_ResetDevice () */


/*****************************************************************************
 * dFWH_SPI_DetectDevice: perform a device identify
 *
 * Returns: E__OK or error code
 */

UINT32	dFWH_SPI_DetectDevice 
(
	CHIPSET*	psChipset
)
{ 
    // register UINT32	dBaseAddr; 
	// register UINT8	bStepSize;
	
    /* Check the memory's device class, if not compatible we exit now */ 
    if (psChipset->bRomClass != CLASS_FWH_SPI) 
    	return (E__ROM_DETECT); 
 
	// bStepSize = psChipset->bMemWidth;	 // byte wide devices
	// dBaseAddr = psChipset->dBaseAddr;

	 
    /* Device ID detection is not supported */ 

#ifdef DEBUG
	sprintf (buffer, "Found: %s\n", at26df161a);
	vConsoleWrite(buffer);
#endif 

	return (E__OK); 
 
} /* dFWH_SPI_DetectDevice () */ 


/*****************************************************************************
 * dFwh_DetectDevice: perform a device identify
 *
 * Returns: E__OK or error code
 */

UINT32	dFwh_DetectDevice
(
	CHIPSET*	psChipset
)
{
	register UINT32	dBaseAddr;
	register UINT8	bStepSize;
	UINT8	bManufId;
	UINT8	bDeviceId;
	int		iIndex;

#ifdef DEBUG
	vConsoleWrite ("dFwh_DetectDevice\n");
#endif
	/* Check the memory's device class, if not compatible we exit now */

	if (psChipset->bRomClass != CLASS_FWH)
		return (E__ROM_DETECT);

	/* Memory device class matches ours */

#ifdef DEBUG
	vConsoleWrite (". Try Firmware Hub class\n");
#endif
	
	/* Map Physical to Virtual Memory */
	
	bStepSize = psChipset->bMemWidth;	// byte wide devices
	dBaseAddr = psChipset->dBaseAddr;

	/* Do a device ID */

	vFwh_ResetDevice (dBaseAddr, bStepSize);

	vWriteByte32 (dBaseAddr, FWH_READ_ID);

	bManufId  = bReadByte32 (dBaseAddr + (MANUF_ID_ADDR * bStepSize));
	bDeviceId = bReadByte32 (dBaseAddr + (DEVICE_ID_ADDR * bStepSize));

	vFwh_ResetDevice (dBaseAddr, bStepSize);

	
#ifdef DEBUG
	sprintf (buffer, ". Auto ID: M-ID 0x%02X D-ID 0x%02X\n", bManufId, bDeviceId);
	vConsoleWrite(buffer);
#endif

	/* Loop through all supported device types and find this one */

	for (iIndex = 0; iIndex < LENGTH (asDeviceTypes); iIndex++)
	{
		if( ( bManufId  == asDeviceTypes[iIndex].bManufId	)
		&&	( bDeviceId == asDeviceTypes[iIndex].bDeviceId	)	)
		{
#ifdef DEBUG
			sprintf (buffer, "Found: %s\n", asDeviceTypes[iIndex].achName);
			vConsoleWrite(buffer);
#endif
		}
	}

	return (E__OK);

} /* dFwh_DetectDevice () */


static	const DETECT_FN	apfDetectFns[] =
{
	/* Intel chipsets */

	v430hx_Detect,
	v440bx_Detect,
	v440gx_Detect,
	v855gme_Detect,
	vE7520_Detect,
	v945gm_Detect,
	vLE3100_Detect,
	vPiix4e_Detect,
	v6300esb_Detect,
	vIch7_Detect,
	vIch9_Detect,
	vGm45_Detect,
    vSch_Detect,
};

static	const DEV_DETECT_FN	apfDevDetectFns[] =
{
	/*dSpi_DetectDevice,
	dF040_DetectDevice,
	d28SF040_DetectDevice,
	d29SF040_DetectDevice,
	dStrataB_DetectDevice,
	dStrataW_DetectDevice,*/
	dFwh_DetectDevice/*,
	dFwh2_DetectDevice,
	dMFlashB_DetectDevice,
    psudoFWH_DetectDevice*/,
    dFWH_SPI_DetectDevice   	/* This has to be always the last function */
                                /* because there is no detection mechanism */
                                /* for the device.                         */
};


UINT32 dDetectChipset
(
	CHIPSET*	psChipset
)
{
	UINT32	dStatus = E__OK;
	int		iIndex;


	/* Initialize the structure members */

	psChipset->achNorthBridge	= NULL;
	psChipset->achSouthBridge	= NULL;
	psChipset->pfChipsetMemSize	= NULL;
	psChipset->pfChipsetWrite	= NULL;
	psChipset->pfChipsetHole	= NULL;


	/* Call each of the chipset detection functions */

	for (iIndex = 0; iIndex < LENGTH (apfDetectFns); iIndex++)
		(apfDetectFns[iIndex])(psChipset);


	/* Verify that all required structure members are initialized */

	if ((psChipset->achNorthBridge == NULL) ||
		(psChipset->achSouthBridge == NULL) ||
		(psChipset->pfChipsetMemSize == NULL))
		dStatus = E__CHIPSET_NF;

	return (dStatus);

} /* dDetectChipset () */


/*******************************************************************************
 * vGetMemorySize - Reads the size of memory in system
 *
 *
 * RETURN: E__OK or error code
 *******************************************************************************
*/
void vGetMemorySize
(
	CHIPSET* psChipset,
	UINT32*	pdMemSize
)
{
	(psChipset->pfChipsetMemSize)(pdMemSize);

} /* vGetMemorySize () */


/*******************************************************************************
 * vWriteEnableChipset - Enable chipset
 *
 *
 * RETURN: E__OK or error code
 *******************************************************************************
*/
void vWriteEnableChipset
(
	CHIPSET*	psChipset
)
{
	if (psChipset->pfChipsetWrite != NULL)
		(psChipset->pfChipsetWrite)(CS_ENABLE);

} /* vWriteEnableChipset () */

#if 0
/****************************************************************************
 * bIcReadReg: read the specified interconnect register
 *
 * RETURNS: data byte
 */

static UINT8 bIcReadReg
(
	UINT8	bSlot,
	UINT16	wRegister
)
{
	UINT16	wIcAddr;


	wIcAddr = ((UINT16)bSlot << 11) | (wRegister << 2);

	vIoWriteReg( MPC_ADDRL_PORT, REG_8, (UINT8)   wIcAddr        );
	vIoWriteReg( MPC_ADDRH_PORT, REG_8, (UINT8) ( wIcAddr >> 8 ) );

	return dIoReadReg( MPC_DATA_PORT, REG_8 );

} /* bIcRead () */


/****************************************************************************
 * vIcWriteReg: write a byte to the specified interconnect register
 *
 * RETURNS: none
 */

static void vIcWriteReg
(
	UINT8	bSlot,
	UINT16	wRegister,
	UINT8	bData
)
{
	UINT16	wIcAddr;


	wIcAddr = ((UINT16)bSlot << 11) | (wRegister << 2);

	vIoWriteReg( MPC_ADDRL_PORT, REG_8, (UINT8) wIcAddr );
	vIoWriteReg( MPC_ADDRH_PORT, REG_8, (UINT8) ( wIcAddr >> 8 ) );

	vIoWriteReg( MPC_DATA_PORT, REG_8, bData );

} /* vIcWrite () */
#endif

/*******************************************************************************
 * dAutoDetectDevice - Detect the FWH
 *
 *
 * RETURN: E__OK or error code
 *******************************************************************************
*/
UINT32 dAutoDetectDevice
(
	CHIPSET*	psChipset
)
{
	UINT32	dStatus;
	int		iIndex;

	/* Select first device for the detection */

	/*vSelectDevice (psThisMem, 0);*/

	/* Call each of the device detection functions (for first device) until
	 * we get a match
	 */

	for (iIndex = 0; iIndex < LENGTH (apfDevDetectFns); iIndex++)
	{
		dStatus = (apfDevDetectFns[iIndex])(psChipset);

		if (dStatus == E__OK)
		{
			return (E__OK);
		}
		else if( dStatus != E__ROM_DETECT )
		{
			return dStatus;
		}
	}

	return (E__ROM_DETECT);

} /* dAutoDetectDevice () */



/*****************************************************************************
 * Fwhdevid: test function for Firmware Hub Device Identification test
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (Fwhdevid, "Firmware Hub Device ID Test")
{

	UINT32 dtest_status = E__OK;
	UINT32	pdMemSize;
	CHIPSET	sChipset;
	
	dtest_status = dDetectChipset (&sChipset); /* bInstance - Contoller instance */

	if (dtest_status == E__OK)
	{
		vGetMemorySize (&sChipset, &pdMemSize);
#ifdef DEBUG
		sprintf(buffer, "Mem Size: 0x%x\n", pdMemSize);
		vConsoleWrite(buffer); 
#endif		
		vWriteEnableChipset (&sChipset);
		
		dtest_status = dAutoDetectDevice (&sChipset);

	}
	return dtest_status;
}


