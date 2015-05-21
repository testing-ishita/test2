
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/interconnect.c,v 1.3 2014-08-04 08:45:31 mgostling Exp $
 *
 * $Log: interconnect.c,v $
 * Revision 1.3  2014-08-04 08:45:31  mgostling
 * Remove conditional around service call.
 *
 * Revision 1.2  2014-03-18 14:20:20  cdobson
 * Corrected the board name.
 *
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.3  2013-10-17 10:21:56  mgostling
 * Corrected board name string
 *
 * Revision 1.2  2013-09-13 10:23:34  jthiru
 * Updates for SOAK TESTING
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.2.2.2  2012-11-13 17:11:40  hchalla
 * Increased delay for the backplane detection of the peripheral boards based on VPX Slot ID.
 *
 * Revision 1.2.2.1  2012-11-05 15:55:03  hchalla
 * Added support to test 5 tr 80x in a 3U VPX daisy chained backplane.
 *
 * Revision 1.2  2011-08-26 15:48:57  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.1  2011/08/02 17:09:57  hchalla
 * Initial version of sources for TR 803 board.
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
#include <cute/packet.h>

#define VPX_MAX_DELAY  0x4000

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

//extern sPeripheralSlave asPeripheralSlot[MAX_SLAVES];

const UINT8 abInterconnectTemplate[] =
{
		0x25, 0x00,			/* 0x00: Vendor ID low byte		*/
		0x00, 0x00,			/* 0x01: Vendor ID high byte 	*/
		'V',  0x00,			/* 0x02: Board name				*/
		'R',  0x00,			/* 0x03:	-"-					*/
		'E',  0x00,			/* 0x04:	-"-					*/
		'1',  0x00,			/* 0x05:	-"-					*/
		'x',  0x00,			/* 0x06:	-"-					*/
		'/',  0x00,			/* 0x07:	-"-					*/
		'm',  0x00,			/* 0x08:	-"-					*/
		's',  0x00,			/* 0x09:	-"-					*/
		'd',  0x00,			/* 0x0A:	-"-					*/
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
        0x00, 0x01,			/* 0x4d: Intercon Synchronisation for Master			*/
        0x00, 0x01,			/* 0x4e: Intercon Synchronisation for Slave			*/
		0xFF, 0x00,			/* 0x4f: Record Type			*/

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
	UINT16		wCntr;              /* general word-wide counter */
	UINT8   	bTemp;
	UINT16		wRegNum;            /* index to register within template */
	UINT32		dMemSize;           /* memory size */
	UINT8		bMemSizeLow;        /* low and high dp ram */
	UINT8		bMemSizeHi;         /*        -"-          */
	UINT8		bMemLow;            /* low and high memory size components  */
	UINT8		bMemHi;             /*                  -"-                 */
	brd_info 	info;
#ifdef DEBUG_SOAK
	UINT8 bVPXSlotID=0;
	UINT8 bSysCon;
	UINT8 achBuffer[80];
#endif

	(void)ptr;

	/* Fill the whole template area with the end-of-template type code */
	bTemp = 0xff;
	for (wCntr = 0; wCntr < 0x1000; wCntr++)
	{
		vWriteIcByte( INTERCONNECT_LOCAL_SLOT, wCntr, bTemp);
	}

	/* Copy the template */
	for (wCntr = 0; wCntr < IC_TEMPLATE_SIZE; wCntr++)
	{
		wRegNum = wCntr*2;
		bTemp = abInterconnectTemplate[wRegNum];
		vWriteIcByte( INTERCONNECT_LOCAL_SLOT, wRegNum, bTemp);
		bTemp = abInterconnectTemplate[wRegNum+1];
		vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wRegNum+1), bTemp);
	}

	board_service(SERVICE__BRD_CUTE_INFO, NULL, &info);

	dMemSize    = info.memory;
	bMemSizeHi  = dMemSize >> 8;
	bMemSizeLow = dMemSize & 0xFF;
	dMemSize--;
	bMemHi      = dMemSize >> 8;
	bMemLow     = dMemSize & 255;

	wCntr = 0x40;      /* Point to first record after header. */
	bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	while (bTemp != LOCAL_MEMORY_RECORD)
	{
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2));
		wCntr += (bTemp * 2) + 4;
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	}
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(END_ADDRESS_LOW * 2)),  bMemLow);
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(END_ADDRESS_HIGH * 2)), bMemHi);

	/* Find the memory record and insert the memory end address.  */
	bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	while (bTemp != MEMORY_RECORD)
	{
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2));
		wCntr += (bTemp * 2) + 4;
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	}
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(MEM_SIZE_LOW * 2)), bMemSizeLow);
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(MEM_SIZE_HIGH * 2)), bMemSizeHi);

	bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	while (bTemp != BOARD_SPECIFIC_RECORD)
	{
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+2));
		wCntr += (bTemp * 2) + 4;
		bTemp = bReadIcByte( INTERCONNECT_LOCAL_SLOT, wCntr);
	}
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(HARDWARE_REVISION * 2)), (info.HardwareRev));
	vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (wCntr+(FIRMWARE_REVISION * 2)), (info.firmwareVersion));

#if 0
	bSysCon = dIoReadReg(0x31E, REG_8);
	if ((bSysCon & 0x04) != 0x04)
	{

		bVPXSlotID = bReadVPXSlotID();
#ifdef DEBUG_SOAK
		sprintf (achBuffer,"VPX SlotID:%d\n",bVPXSlotID);
		sysDebugWriteString(achBuffer);
#endif

		if ( (bVPXSlotID == 3) || (bVPXSlotID == 4))
		{

		}
		else
		{
			vDelay(VPX_MAX_DELAY - (bVPXSlotID*0x1000));
		}

		vGetSlavePeripheralBoards();

		if(vReadByte(asPeripheralSlot[1].dBar) == 0x25)
		{
#ifdef DEBUG_SOAK
			sysDebugWriteString("Found Slave Peripheral 1");
#endif
			for (wCntr = 0; wCntr < 0x20; wCntr++)
			{
				wRegNum = wCntr*2;
				bTemp = vReadByte(asPeripheralSlot[1].dBar+wRegNum);
				vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (0x200+wRegNum), bTemp);
				bTemp = vReadByte(asPeripheralSlot[1].dBar+(wRegNum+1));
				vWriteIcByte( INTERCONNECT_LOCAL_SLOT, ((0x201+wRegNum)), bTemp);
			}

	  	    if(vReadByte(asPeripheralSlot[1].dBar+0x200) == 0x25)
			{
#ifdef DEBUG_SOAK
	  			sysDebugWriteString("Found Slave Peripheral 2");
#endif
	  			wRegNum = wCntr*2;
	  			bTemp = vReadByte(asPeripheralSlot[1].dBar+wRegNum);
	  			vWriteIcByte( INTERCONNECT_LOCAL_SLOT, (0x300+wRegNum), bTemp);
	  			bTemp = vReadByte(asPeripheralSlot[1].dBar+(wRegNum+1));
	  			vWriteIcByte( INTERCONNECT_LOCAL_SLOT, ((0x301+wRegNum)), bTemp);
			}
		}
	}
	else
	{

		bVPXSlotID = bReadVPXSlotID();
#ifdef DEBUG_SOAK
		sprintf (achBuffer,"VPX SlotID:%d\n",bVPXSlotID);
		sysDebugWriteString(achBuffer);
#endif
		vDelay(VPX_MAX_DELAY);

	}
#endif
	return E__OK;
}

