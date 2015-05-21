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
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/coopRam.c,v 1.3 2014-07-24 14:05:55 mgostling Exp $
 * $Log: coopRam.c,v $
 * Revision 1.3  2014-07-24 14:05:55  mgostling
 * Added condtitionals around debug messages.
 *
 * Revision 1.2  2013-11-25 11:43:38  mgostling
 * The RAM block was changed from 4MB to 1MB.
 *
 * Revision 1.1  2013-09-04 07:46:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.10  2012/09/14 15:38:45  jthiru
 * CUTE Support for VX91X - C5043B1110, fixes to existing bugs
 *
 * Revision 1.9  2012/03/21 15:58:02  cdobson
 * Eliminate compiler warnings.
 *
 * Revision 1.8  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.7  2011/12/01 13:44:46  hchalla
 * Updated for VX 541 board.
 *
 * Revision 1.6  2011/10/27 15:50:39  hmuneer
 * no message
 *
 * Revision 1.5  2011/04/05 13:12:25  hchalla
 * Added File header and Version control header.
 *
 *
 */
#include <stdtypes.h>
#include <errors.h>
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <devices/ram.h>
#include <bit/hal.h>
 
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>
#include <private/cpu.h>
#include <private/debug.h>
#include <devices/pci_devices.h>
#include <cute/packet.h>
#include <cute/sm3.h>
#include <cute/arch.h>


//#define DEBUG
//#define DEBUG_SOAK


extern UINT32 dGetPhysPtr (UINT32 dBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);

#if defined(VPX)

char	achErrBuff[80];
static  UINT32 currentRamBlock;
static  UINT32 totalRamBlocks;

typedef struct
{
	UINT32 base;
	UINT32 len;
	UINT32 block;

}RamBlock;


extern UINT32 get_testable_ram ( RAM *p );

/*The RAM block was changed from 4MB to 1MB */
UINT32 get_RamBlock(RamBlock *block)
{
	RAM testRam;
	UINT32 nextBlock;

	get_testable_ram ( &testRam );

	if (testRam.base % 0x100000 != 0)
	{
		testRam.base += 0x100000 - (testRam.base % 0x100000);
		if (testRam.len % 0x100000 == 0)	/* don't short length twice */
			testRam.len -= 0x100000;
	}

	if (testRam.len % 0x100000 != 0)
		testRam.len -= (testRam.len % 0x100000);

	totalRamBlocks = (testRam.len - testRam.base) / 0x100000;
	if(currentRamBlock < totalRamBlocks)
		nextBlock = currentRamBlock + 1;
	else
		currentRamBlock = 0;

	block->base  = testRam.base + (currentRamBlock * 0x100000);
	block->len   = 0x100000;
	block->block = currentRamBlock;

#ifdef DEBUG_SOAK
	sprintf(achErrBuff, "Total Blocks %d, Current Block %d, Ram Base %x, Ram Length %x\n",
	totalRamBlocks, currentRamBlock, testRam.base, testRam.len);
	sysDebugWriteString (achErrBuff);
#endif


	currentRamBlock = nextBlock;

	return E__OK;
}


static UINT32 wDramReadWriteBlock( UINT32 dAddr, UINT32 len  )
{
	UINT32	dEndAddr;
	UINT32	dData;
	UINT32	dTemp;			/* holds original memory contents */
#ifdef DEBUG
	char   buffer[64];
#endif

	dEndAddr = dAddr + len;

	while (dAddr < dEndAddr)
	{
		dTemp = *(UINT32*)dAddr;		/* read original data */
		*(UINT32*)dAddr = ~dTemp;		/* write compliment */
		dData = *(UINT32*)dAddr;		/* re-read */

		if (dData != ~dTemp)			/* compare */
		{
 			#ifdef DEBUG
            	sprintf(buffer,"Failure at %x\n",dAddr);
                sysDebugWriteString (buffer);
			#endif

        	return (E__DAT_RDWR_ERR);
		}

		*(UINT32*)dAddr = dTemp;		/* write back original data */
		dData = *(UINT32*)dAddr;		/* re-read */

		if (dData != dTemp)				/* compare */
		{
			#ifdef DEBUG
            	sprintf(buffer,"Failure at %x\n",dAddr);
                sysDebugWriteString (buffer);
			#endif

        	return (E__DAT_RDWR_ERR);
		}

		dAddr += 4;
	}

#ifdef DEBUG
        sysDebugWriteString ("Co-ord Test passed\n");
#endif
	return (E__OK);

} /* wDramReadWriteBlock () */




UINT32 wCoopRamReadWrite(UINT32* adTestParams)
{
	UINT32	wTestStatus = E__OK;
	char	achBuffer[80];					/* text formatting buffer 	*/
	RamBlock block;
	UINT32   bar;
	UINT32 	mMapSpace;

#ifdef DEBUG_SOAK
	sysDebugWriteString ("wCoopRamReadWrite()\n");
#endif
	wTestStatus = E__OK;


#ifdef DEBUG_SOAK
              sprintf(achErrBuff, "adTestParams[0]: %d, adTestParams[1]: %d, adTestParams[2]: %d,  \\
                  adTestParams[3]: %d, adTestParams[4]: %d, adTestParams[5]: %d \n",
                  adTestParams[0], adTestParams[1], adTestParams[2], adTestParams[3],
                  adTestParams[4], adTestParams[5]);
              sysDebugWriteString (achErrBuff);
#endif

	if (adTestParams[1] == 99)
	{
		if (adTestParams[2] == 1)
		{
			achBuffer[0] = '\0';
			if ((*(UINT8*)adTestParams[3] == 'S') || (*(UINT8*)adTestParams[3] =='s'))
			{
#ifdef DEBUG_SOAK
					sysDebugWriteString ("phase=1 Slave\n");
#endif

					get_RamBlock(&block);

					bar = dSetMemWindow (block.base, block.len);

#ifdef DEBUG_SOAK
					sprintf(achErrBuff, "Current Block %d, Base %x, Length %x, bar %x\n",
							block.block, block.base, block.len, bar);
					sysDebugWriteString (achErrBuff);
#endif

					sprintf (achBuffer, "%xH,%xH", bar, block.len);

			}
			else
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString("phase=1 Master\n");
#endif
			}
#ifdef DEBUG_SOAK
			sysDebugWriteString(achBuffer);
#endif
			IDX_p3_bist_exit (1, achBuffer);
			return E__OK;
		}
		else if (adTestParams[2] == 2)
		{
			achErrBuff[0] = '\0';

			if ((*(UINT8*)adTestParams[3] == 'M') || (*(UINT8*)adTestParams[3] =='m'))
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString("phase=2 Master\n");



				sprintf(achErrBuff, "Base %x, Length %x\n", adTestParams[4], adTestParams[5]);
				sysDebugWriteString (achErrBuff);
#endif

				mMapSpace = bAddrToMem (adTestParams[4]);

				if(mMapSpace != NULL )
				{
#ifdef DEBUG_SOAK
					sprintf(achErrBuff, "mMapSpace %x\n", mMapSpace);
					sysDebugWriteString (achErrBuff);
#endif
					wTestStatus = wDramReadWriteBlock( mMapSpace, adTestParams[5] );
				}
				else
				{
#ifdef DEBUG_SOAK
					sysDebugWriteString ("bAddrToMem() Fail \n");
#endif
					wTestStatus = E__FAIL;
				}

				if(wTestStatus == E__OK)
				{
#ifdef DEBUG_SOAK
					sprintf(achErrBuff, "Slot %d Passed", bMemToSlot (adTestParams[4]));
#endif
				}
				else
				{
#ifdef DEBUG_SOAK
					sprintf(achErrBuff, "Slot %d Failed", bMemToSlot (adTestParams[4]));
#endif
				}
			}
			else
			{
#ifdef DEBUG_SOAK
				sysDebugWriteString("phase=2 Slave\n");
#endif
			}
			
			IDX_p3_bist_exit (wTestStatus, achErrBuff);
		}
		else
			IDX_p3_bist_exit (0x300, "Invalid phase");
	}
	else
		IDX_p3_bist_exit (0x300, "Invalid parameter");

	return E__OK;

} /* wCoopRamReadWrite() */

#endif






#if defined(CPCI)

char	achErrBuff[80];
static  UINT32 currentRamBlock;
static  UINT32 totalRamBlocks;

UINT32 	mMapSpace = 0 ;
static UINT32	 dHandle = 0;
static PTR48	 p1 = NULL;

typedef struct
{
	UINT32 base;
	UINT32 len;
	UINT32 block;

}RamBlock;



extern UINT32 get_testable_ram ( RAM *p );

UINT32 get_RamBlock(RamBlock *block)
{
	RAM testRam;
	UINT32 nextBlock;

	get_testable_ram ( &testRam );

	if (testRam.base % 0x4000000 != 0)
	{
		testRam.base += 0x4000000 - (testRam.base % 0x4000000);
		if (testRam.len % 0x4000000 == 0)	/* don't short length twice */
			testRam.len -= 0x4000000;
	}

	if (testRam.len % 0x4000000 != 0)
		testRam.len -= (testRam.len % 0x4000000);

	totalRamBlocks = (testRam.len - testRam.base) / 0x4000000;
	if(currentRamBlock < totalRamBlocks)
		nextBlock = currentRamBlock + 1;
	else
		currentRamBlock = 0;

	block->base  = testRam.base + (currentRamBlock * 0x4000000);
	block->len   = 0x4000000;
	block->block = currentRamBlock;

#ifdef DEBUG_SOAK
	sprintf(achErrBuff, "Total Blocks %d, Current Block %d, Ram Base %x, Ram Length %x\n",
	totalRamBlocks, currentRamBlock, testRam.base, testRam.len);
	sysDebugWriteString (achErrBuff);
#endif


	currentRamBlock = nextBlock;

	return E__OK;
}


static UINT32 wDramReadWriteBlock( UINT32 dAddr, UINT32 len  )
{
	UINT32	dEndAddr;
	UINT32	dData;
	UINT32	dTemp;			/* holds original memory contents */
#ifdef DEBUG
	char   buffer[64];
#endif

	dEndAddr = dAddr + len;

	while (dAddr < dEndAddr)
	{
		dTemp = *(UINT32*)dAddr;		/* read original data */
		*(UINT32*)dAddr = ~dTemp;		/* write compliment */
		dData = *(UINT32*)dAddr;		/* re-read */

		if (dData != ~dTemp)			/* compare */
		{
 			#ifdef DEBUG
            	sprintf(buffer,"Failure at %x\n",dAddr);
                sysDebugWriteString (buffer);
			#endif

        	return (E__DAT_RDWR_ERR);
		}

		*(UINT32*)dAddr = dTemp;		/* write back original data */
		dData = *(UINT32*)dAddr;		/* re-read */

		if (dData != dTemp)				/* compare */
		{
			#ifdef DEBUG
            	sprintf(buffer,"Failure at %x\n",dAddr);
                sysDebugWriteString (buffer);
			#endif

        	return (E__DAT_RDWR_ERR);
		}

		dAddr += 4;
	}

#ifdef DEBUG
        sysDebugWriteString ("Co-ord Test passed\n");
#endif
	return (E__OK);

} /* wDramReadWriteBlock () */




UINT32 wCoopRamReadWrite(UINT32* adTestParams)
{
	UINT32	wTestStatus = E__OK;
	char	achBuffer[80];					/* text formatting buffer 	*/
	RamBlock block;
	UINT32   bar = 0;
	UINT32   dBarParam = 0;
	UINT32   dLen = 0;


	sysDebugWriteString ("wCoopRamReadWrite()\n");
	wTestStatus = E__OK;

	if (adTestParams[1] == 99)
	{
		if (adTestParams[2] == 1)
		{
			achBuffer[0] = '\0';
			if ((*(UINT8*)adTestParams[3] == 'S') || (*(UINT8*)adTestParams[3] =='s'))
			{
					sysDebugWriteString ("phase=1 Slave\n");

					get_RamBlock(&block);

					bar = dSetMemWindow (block.base, block.len);

					sprintf(achErrBuff, "Current Block %d, Base %x, Length %x, bar %x\n",
							block.block, block.base, block.len, bar);
					sysDebugWriteString (achErrBuff);

					sprintf (achBuffer, "%xH,%xH", bar, block.len);
			}
			else
			{
				sysDebugWriteString("phase=1 Master\n");
			}
			sysDebugWriteString(achBuffer);
			IDX_p3_bist_exit (1, achBuffer);
			return E__OK;
		}
		else if (adTestParams[2] == 2)
		{
			achErrBuff[0] = '\0';

			if ((*(UINT8*)adTestParams[3] == 'M') || (*(UINT8*)adTestParams[3] =='m'))
			{
				sysDebugWriteString("phase=2 Master\n");

				sprintf(achErrBuff, "Base %x, Length %x\n", adTestParams[4], adTestParams[5]);
				sysDebugWriteString (achErrBuff);

				dBarParam = adTestParams[4];
				dLen      = adTestParams[5];

				dHandle = dGetPhysPtr(dBarParam,dLen,&p1,(void*)&mMapSpace);
				if((dHandle != E__FAIL) && (mMapSpace != NULL ))
				{
					sprintf(achErrBuff, "mMapSpace %x\n", mMapSpace);
					sysDebugWriteString (achErrBuff);
					wTestStatus = wDramReadWriteBlock( mMapSpace, dLen);

					vFreePtr (dHandle);
				}
				else
				{
					sysDebugWriteString ("dGetPhysPtr() Fail \n");
					wTestStatus = E__FAIL;
				}

				if(wTestStatus == E__OK)
					sprintf(achErrBuff, "Slot %d Passed", bMemToSlot (dBarParam));
				else
					sprintf(achErrBuff, "Slot %d Failed", bMemToSlot (dBarParam));

			}
			else
			{
				sysDebugWriteString("phase=2 Slave\n");
			}

			IDX_p3_bist_exit (wTestStatus, achErrBuff);
		}
		else
			IDX_p3_bist_exit (0x300, "Invalid phase");
	}
	else
		IDX_p3_bist_exit (0x300, "Invalid parameter");

	return E__OK;

} /* wCoopRamReadWrite() */

#endif





/*****************************************************************************
 * SPD EEPROM checksum test: test function for SPD EEPROM Contents
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (MstrRemoteRamReadWrite, "Master Remote RAM Read Write Test")
{

#ifdef DEBUG
	sysDebugWriteString ("MstrRemoteRamReadWrite()\n");
#endif

	if ( (  bIsPeripheral()  && ((*(UINT8*)adTestParams[3] == 'S') || (*(UINT8*)adTestParams[3] =='s'))) ||
	     ((!bIsPeripheral()) && ((*(UINT8*)adTestParams[3] == 'M') || (*(UINT8*)adTestParams[3] =='m')))
	   )
	{
		if( (adTestParams[2] == 1) && (adTestParams[0] == 3) )
		{
#ifdef DEBUG
			sysDebugWriteString ("Testing\n");
#endif
			wCoopRamReadWrite(adTestParams);
		}
		else if (adTestParams[2] == 2)
		{
			if ( ((adTestParams[0] == 4) && ((*(UINT8*)adTestParams[3] == 'S') || (*(UINT8*)adTestParams[3] =='s'))) ||
	     		 ((adTestParams[0] == 5) && ((*(UINT8*)adTestParams[3] == 'M') || (*(UINT8*)adTestParams[3] =='m')))
	   		   )
	   		{
#ifdef DEBUG
				sysDebugWriteString ("Testing\n");
#endif
				wCoopRamReadWrite(adTestParams);
	   		}
	   		else
			{
#ifdef DEBUG
				sysDebugWriteString ("wrong number of parameters\n");
#endif
				IDX_p3_bist_exit (0x300, "Invalid parameter");
			}
		}
		else
		{
#ifdef DEBUG
			sysDebugWriteString ("wrong number of parameters\n");
#endif
			IDX_p3_bist_exit (0x300, "Invalid parameter");
		}
	}
	else
	{
		if( adTestParams[2] == 1 )
		{
#ifdef DEBUG
			sysDebugWriteString ("Else 1\n");
#endif
			IDX_p3_bist_exit (1, "");
		}
		else
		{
#ifdef DEBUG
			sysDebugWriteString ("Else 2\n");
#endif
			IDX_p3_bist_exit (E__OK, "");
		}
	}

	return E__OK;
}

