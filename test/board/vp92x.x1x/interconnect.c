
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

/*
 * pbit.c: Power-on BIT Test handler
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/interconnect.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: interconnect.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
 * Import files into new source repository.
 *
 * Revision 1.3  2012/08/10 03:39:17  madhukn
 * *** empty log message ***
 *
 * Revision 1.2  2012/08/07 03:26:25  madhukn
 * *** empty log message ***
 *
 * Revision 1.1  2012/06/22 11:42:00  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>

 
#include <bit/bit.h>
#include <bit/board_service.h>

#include <private/cpu.h>
#include <private/debug.h>

#include <cute/arch.h>
#include <cute/interconnect.h>

const UINT8 abInterconnectTemplate[] =
{
		0x25, 0x00,			/* 0x00: Vendor ID low byte		*/
		0x00, 0x00,			/* 0x01: Vendor ID high byte 	*/
		'V',  0x00,			/* 0x02: Board name				*/
		'P',  0x00,			/* 0x03:	-"-					*/
		'9',  0x00,			/* 0x04:	-"-					*/
		'2',  0x00,			/* 0x05:	-"-					*/
		'x',  0x00,			/* 0x06:	-"-					*/
		'/',  0x00,			/* 0x07:	-"-					*/
		'x',  0x00,			/* 0x08:	-"-					*/
		'1',  0x00,			/* 0x09:	-"-					*/
		'x',  0x00,			/* 0x0A:	-"-					*/
		0x00, 0x00,			/* 0x0B:	-"-					*/
		0x00, 0x00,			/* 0x0C: Intel Reserved			*/
		0x00, 0x00,			/* 0x0D:	-"-					*/
		0x00, 0x00,			/* 0x0E:	-"-					*/
		0x00, 0x00,			/* 0x0F:	-"-					*/
		0x01, 0x00,			/* 0x10: Hardware Test Rev No.	*/
		0x01, 0x00,			/* 0x11: Class ID F1 = Universe 01 = TSI148	*/
		0x00, 0x00,			/* 0x12: Intel Reserved			*/
		0x00, 0x00,			/* 0x13:	-"-					*/
		0x00, 0x00,			/* 0x14:	-"- 				*/
		0x03, 0x00,			/* 0x15: Reset Status 			*/
		0x00, 0x03,			/* 0x16: Program Table Index 	*/
		0x00, 0x03,			/* 0x17: NMI Enable 			*/
		0x00, 0x00,			/* 0x18: General Status 		*/
		0x00, 0x03,			/* 0x19: General Control 		*/
		0xA0, 0x01,			/* 0x1A: BIST Support level 	*/
		0x00, 0x03,			/* 0x1B: BIST Data In 			*/
		0x00, 0x01,			/* 0x1C: BIST Data Out 			*/
		0x00, 0x01,			/* 0x1D: BIST Slave Status 		*/
		0x00, 0x03,			/* 0x1E: BIST Master Status 	*/
		0x03, 0x01,			/* 0x1F: BIST Test ID 			*/

    /* Cache Memory Record */

        0x05, 0x00,         /* 0x20: Record Type            */
        0x04, 0x00,         /* 0x21: Record Length          */
        0xFF, 0x00,         /* 0x22: Cache Size (low)       */
        0x01, 0x00,         /* 0x23: Cache Size (high)      */
        0x20, 0x00,         /* 0x24: Cache Entry Size       */
        0x00, 0x01,         /* 0x25: Cache Control.         */

    /* Protection Record */

        0x0B, 0x00,         /* 0x26: Record Type            */
        0x01, 0x00,         /* 0x27: Record Length          */
        0x00, 0x01,         /* 0x28: Protection Level       */

    /* Local Memory Record */

        0x11, 0x00,         /* 0x29: Record Type            */
        0x05, 0x00,         /* 0x2A: Record Length          */
        0x00, 0x00,         /* 0x2B: Start Address, low     */
        0x00, 0x00,         /* 0x2C: Start Address, high    */
        0x00, 0x00,         /* 0x2D: End Address, low       */
        0x00, 0x00,         /* 0x2E: End Address, high      */
        0x00, 0x03,         /* 0x2F: Control Register       */

    /* Memory Record */

        0x01, 0x00,         /* 0x30: Record Type            */
        0x03, 0x00,         /* 0x31: Record Length          */
        0x00, 0x00,         /* 0x32: Size low               */
        0x00, 0x00,         /* 0x33: Size high              */
        0x20, 0x00,         /* 0x34: Status Register        */

    /* Window Record */

        0x0C, 0x00,         /* 0x35: Record Type            */
        0x04, 0x00,         /* 0x36: Record Length          */
        0x00, 0x00,         /* 0x37: Addr low               */
        0x00, 0x00,         /* 0x38: Addr high              */
        0x00, 0x00,         /* 0x39: Size low               */
        0x04, 0x00,         /* 0x3A: Size high              */

    /* Board Specific Record */

        0xFE, 0x00,         /* 0x3B: Record Type            */
        0x0A, 0x00,         /* 0x3C: Record Length          */
        0x00, 0x00,         /* 0x3D: Hardware Revision      */
        0x00, 0x00,         /* 0x3E: Firmware Revision      */
        0x00, 0x01,         /* 0x3F: On-Board Control       */
        0x00, 0x00,         /* 0x40: On-Board Status        */
        0x00, 0x00,         /* 0x41: User Configuration     */
        0x00, 0x00,         /* 0x42: */
        0x00, 0x00,         /* 0x43: */
        0x00, 0x00,         /* 0x44: */
        0x00, 0x00,         /* 0x45: */
        0x07, 0x00,         /* 0x46: SCSI Controller SCSI ID*/

    /* Firmware Communication Record */

        0x0F, 0x00,         /* 0x47: Record Type            */
        0x04, 0x00,         /* 0x48: Record Length          */
        0x00, 0x01,         /* 0x49: SCSI IF test sync ctrl */
        0x00, 0x01,         /* 0x4A:                        */
        0x00, 0x01,         /* 0x4B:                        */
        0x00, 0x01,         /* 0x4C:                        */

	/* End-of-Template Record */

		0xFF, 0x00,			/* 0x4d: Record Type			*/

}; /* abInterconnectTemplate */

const UINT16 IC_TEMPLATE_SIZE = sizeof (abInterconnectTemplate)/2;



/*
void dumpTemplate()
{
	UINT16	wCntr, wRegNum;
	UINT8   bTemp;
	char	achBuffer[80];

	sysDebugWriteString ("\n\nInterconnect template\n\n");
	for (wCntr = 0; wCntr < IC_TEMPLATE_SIZE; wCntr++)
	{
		wRegNum = wCntr*2;

		bIcReadByte( INTERCONNECT_LOCAL_SLOT, wRegNum, &bTemp);
		sprintf(achBuffer,"%x\t%x\t", wCntr, bTemp);
		sysDebugWriteString (achBuffer);

		bIcReadByte( INTERCONNECT_LOCAL_SLOT, (wRegNum+1), &bTemp);
		sprintf(achBuffer,"%x\n", bTemp);
		sysDebugWriteString (achBuffer);
	}
}*/




/*****************************************************************************
 * InitIcr:
 *
 * This function initializes the board's local interconnect template. The
 * structure above is copied to local memory, then hardware dependent registers
 * are configured according to the settings in the board status registers.
 *
 * RETURNS: none
 */
	UINT32 brdInitIcr (void *ptr)
	{
		UINT16		wCntr;				/* general word-wide counter */
		UINT8		bTemp;
		UINT16		wRegNum;			/* index to register within template */
		UINT32		dMemSize;			/* memory size */
		UINT8		bMemSizeLow;		/* low and high dp ram */
		UINT8		bMemSizeHi; 		/*		  -"-		   */
		UINT8		bMemLow;			/* low and high memory size components	*/
		UINT8		bMemHi; 			/*					-"- 				*/
		brd_info	info;
		PCI_PFA 		pfa;
	
		(void)ptr;
	
		/* Fill the whole template area with the end-of-template type code */
		bTemp = 0xff;
		for (wCntr = 0; wCntr < 0x1000; wCntr++)
		{
			bIcWriteByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		}
	
		/* Copy the template */
		for (wCntr = 0; wCntr < IC_TEMPLATE_SIZE; wCntr++)
		{
			wRegNum = wCntr*2;
			bTemp = abInterconnectTemplate[wRegNum];
			bIcWriteByte( INTERCONNECT_LOCAL_SLOT, wRegNum, &bTemp);
			bTemp = abInterconnectTemplate[wRegNum+1];
			bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wRegNum+1), &bTemp);
		}
	
		board_service(SERVICE__BRD_CUTE_INFO, NULL, &info);
	
		dMemSize	= info.memory;
		bMemSizeHi	= dMemSize >> 8;
		bMemSizeLow = dMemSize & 0xFF;
		dMemSize--;
		bMemHi		= dMemSize >> 8;
		bMemLow 	= dMemSize & 255;
	
		wCntr = 0x40;	   /* Point to first record after header. */
		bIcReadByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		while (bTemp != LOCAL_MEMORY_RECORD)
		{
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2), &bTemp);
			wCntr += (bTemp * 2) + 4;
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		}
		bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(END_ADDRESS_LOW * 2)),  &bMemLow);
		bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(END_ADDRESS_HIGH * 2)), &bMemHi );
	
		/* Find the memory record and insert the memory end address.  */
		bIcReadByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		while (bTemp != MEMORY_RECORD)
		{
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2), &bTemp);
			wCntr += (bTemp * 2) + 4;
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		}
		bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wCntr + (MEM_SIZE_LOW  * 2)), &bMemSizeLow);
		bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wCntr + (MEM_SIZE_HIGH * 2)), &bMemSizeHi );
	
			/* Find the Window record and insert the window address  */
		pfa = PCI_MAKE_PFA (0,28,0);
		bTemp = PCI_READ_BYTE(pfa, PCI_SEC_BUS);
		pfa = PCI_MAKE_PFA ( bTemp,0,0);
		bTemp = PCI_READ_BYTE(pfa, PCI_SEC_BUS);
		pfa = PCI_MAKE_PFA (bTemp,4,0);
		dMemSize = PCI_READ_DWORD(pfa, 0x10);
		dMemSize -= 0x4100000;
		dMemSize = dMemSize >> 16;
		bMemSizeLow = dMemSize & 0xff;
		bMemSizeHi = dMemSize >> 8;
	
		bIcReadByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		while (bTemp != WINDOW_RECORD)
		{
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2), &bTemp);
			wCntr += (bTemp * 2) + 4;
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		}
		bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(WINDOW_LOW  * 2)), &bMemSizeLow);
		bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(WINDOW_HIGH * 2)), &bMemSizeHi);
	
		bIcReadByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		while (bTemp != BOARD_SPECIFIC_RECORD)
		{
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2), &bTemp);
			wCntr += (bTemp * 2) + 4;
			bIcReadByte( INTERCONNECT_LOCAL_SLOT, wCntr, &bTemp);
		}
		bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(HARDWARE_REVISION * 2)), &(info.HardwareRev));
		bIcWriteByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(FIRMWARE_REVISION * 2)), &(info.firmwareVersion));
	
		return E__OK;
	}

