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

/* ram.c - Memory test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/ram.c,v 1.11 2015-03-12 15:24:02 hchalla Exp $
 *
 * $Log: ram.c,v $
 * Revision 1.11  2015-03-12 15:24:02  hchalla
 * Added Debug prints for more debugging info on adTestParams.
 *
 * Revision 1.10  2015-02-25 18:03:41  hchalla
 * Removed all board specific GPIO strap readings to detect RAM size and
 * moved them into board specific services.
 *
 * Revision 1.9  2014-08-04 08:47:42  mgostling
 * Rationalise service call to obtain memory size from GPIO
 *
 * Revision 1.8  2014-04-22 16:39:31  swilson
 * Fix test name AHCI not Ahci.
 *
 * Revision 1.7  2014-03-06 13:49:52  hchalla
 * Adedded cache_off and cache_on functions instead of sysCacheDisable and sysCacheEnable.
 *
 * Revision 1.6  2014-01-10 09:54:56  mgostling
 * Use board_service function to obtain correct memory size for Haswell boards.
 * Tidied up white space chars.
 *
 * Revision 1.5  2013-11-28 15:26:42  mgostling
 * Remove references to HAL in VME builds.
 *
 * Revision 1.4  2013-11-25 12:22:18  mgostling
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.3  2013-11-07 11:43:01  mgostling
 * Fixed block count processing for memory below 4GB in dmemaddrtest() and dmemdatatest()
 * Added support for Haswell
 *
 * Revision 1.2  2013-10-08 07:13:38  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.18  2012/08/30 16:02:17  hmuneer
 * 8GB support for vp717
 *
 * Revision 1.17  2011/10/27 15:50:39  hmuneer
 * no message
 *
 * Revision 1.16  2011/06/07 14:55:01  hchalla
 * Added support for testing the ram above 4GB DRAM. Checked in on behalf of Haroon Munner.
 *
 * Revision 1.15  2011/04/05 13:47:33  hchalla
 * Replaced debug statements vconsolewrite with sysdebug.
 *
 * Revision 1.14  2011/03/22 13:58:43  hchalla
 * Modified ram tests for VX 813 and ECC error reporting.
 *
 * Revision 1.13  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.12  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.11  2010/11/04 17:54:23  hchalla
 * Updated ram tests.
 *
 * Revision 1.10  2010/10/06 10:13:50  jthiru
 * PBIT coding for VP717- testcase fixes
 *
 * Revision 1.9  2010/09/15 12:25:15  hchalla
 * Modified tests to  support TR 501 BIT/CUTE.
 *
 * Revision 1.8  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.7  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.6  2009/06/11 09:08:41  jthiru
 * Changes to memory size for SCH to eork in PBIT - too early to access
 *
 * Revision 1.5  2009/06/10 16:33:44  jthiru
 * Changes to memory size for SCH
 *
 * Revision 1.4  2009/06/09 16:23:31  jthiru
 * Added MCH Memory test case routines
 *
 * Revision 1.3  2009/06/03 08:29:50  cvsuser
 * Tidy up test names
 *
 * Revision 1.2  2009/05/28 09:50:18  cvsuser
 * checkin LE3100 support in data line test
 *
 * Revision 1.1  2009/05/15 11:12:00  jthiru
 * Initial checkin for Memory tests
 *
 *
 */


/* includes */


#include <stdtypes.h>
#include <errors.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <bit/bit.h>
#include <bit/hal.h>
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>
#include <bit/board_service.h>
#include <cute/idx.h>
#include <private/cpu.h>
#include <private/debug.h>
#include <devices/pci_devices.h>
#include <devices/ram.h>
#include <bitops.h>

#define GPIO_BASE		0x48
#define LPC_BRIDGE_DEV	31
#define GP_LVL			0x0C
#define GP_LVL2			0x38

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define vWriteByte(regAddr,value)	(*(UINT8*) (regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

//#define DEBUG 1
#ifdef DEBUG

static char buffer[128];

#endif

#pragma pack(1)
typedef  struct tagRamTestParams
{
	UINT32 block0_start;
    UINT32 block0_len;
    UINT32 block0_psb_off;
    UINT8 block0_tgt_slot;
    UINT32 block1_start;
    UINT32 block1_len;
    UINT32 block1_psb_off;
    UINT8 block1_tgt_slot;
    UINT8 mem_width;

} RAM_TEST_PARAMS;
#pragma pack()

/* constants */
const UINT8 abRamTestTypes[] =
{
	9,
	IDX_UINT32_TYPE,
	IDX_UINT32_TYPE,
    IDX_UINT32_TYPE,
    IDX_UINT8_TYPE,
    IDX_UINT32_TYPE,
    IDX_UINT32_TYPE,
    IDX_UINT32_TYPE,
    IDX_UINT8_TYPE,
    IDX_UINT8_TYPE
};

/* locals */

/* globals */
static UINT16	wIoBase;
static RAM		ram;
static RAM_ROWS	rows;


/* externals */
extern void   vDelay(UINT32 dMsDelay);
extern UINT32 dGetPhysPtr (UINT32 dBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);
extern void   vConsoleWrite(char*	achMessage);
extern void   vFreePtr(UINT32 dHandle);

/* forward declarations */
#if 0 /* Not used, HAL no longer present */
extern void  spMemGetPhysPtr(CCT_MEM_PTR* psParams);
extern void  spMemFreeMem(CCT_MEM_PTR*	  psParams);
#endif

/*******************************************************************************
 * get_testable_ram - defines test area in memory
 *
 * PARAMS: (RAM*)p - pointer to structure to be filled
 *
 * RETURN: E__OK
 *******************************************************************************
 */

UINT32 get_testable_ram
(
	RAM *p
)
{
	PCI_PFA pfa;
	UINT16 wGpioBase = 0;
	UINT8 bVal   = 0;
	UINT64 dMem  = 0;
	UINT32 dTemp = 0;
	UINT32 mHandle, addr;
	PTR48 tPtr1;
	UINT16	mem;

    volatile UINT32 mapRegSpace=0;

	p->base  = RAM_BASE;
	p->total = 0;
	p->len64 = 0;



	pfa = PCI_MAKE_PFA (0, 0, 0);
	if(dPciReadReg(pfa, 0x00, REG_32) == 0x2a408086 ||
       dPciReadReg(pfa, 0x00, REG_32) == 0x00448086 )
	{
		#ifdef DEBUG
		sysDebugWriteString("GS45/Arrandale\n");
		#endif
		p->type = RAM_GS45;

		p->len = ((dPciReadReg (pfa, 0xB0, REG_16) >> 4)* 0x100000) - RAM_BASE;

		p->len64 = ((dPciReadReg (pfa, 0xA2, REG_16) >> 4)* 0x100000);

		addr = dPciReadReg (pfa, 0x48, REG_32);
		addr &= 0xfffffff0;

		mHandle = dGetPhysPtr(addr, 0x1000, &tPtr1, (void*)&mapRegSpace);

		if(mHandle == E__FAIL)
		{
			return E__FAIL;
		}

		mem =  (((*((UINT16*)(mapRegSpace+0x206))) & 0x03ff)*64)/1024;
		mem +=  (((*((UINT16*)(mapRegSpace+0x606))) & 0x03ff)*64)/1024;

		if(mem == 8)
			p->len64 += 0x100000000LL;

		p->total = p->len64 + p->len;

		#ifdef DEBUG
			sprintf (buffer, " Total mem %dGB\n", mem);
			vConsoleWrite(buffer);

			sprintf (buffer, " p->len=%08Xh\n", p->len);
			vConsoleWrite(buffer);

			sprintf (buffer, " p->len64=%08X%08Xh\n", (UINT32)(p->len64>>32), (UINT32)p->len64);
			vConsoleWrite(buffer);

			sprintf (buffer, " p->total=%08X%08Xh\n", (UINT32)(p->total>>32), (UINT32)p->total);
			vConsoleWrite(buffer);
		#endif
	}
	else if (dPciReadReg(pfa, 0x00, 0x03) == 0x01048086)
	{
		p->type = RAM_SANDY_BRIDGE;

		pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
		wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
		dTemp = dIoReadReg (wGpioBase+ GP_LVL2, REG_32);

		if ( ((dTemp >> 17) & (0x01)) == 0x01)
		{
			bVal = (0x00) | (0x1);
		}
		else if ( ((dTemp >> 18) & (0x01)) == 0x01)
		{
		    bVal = (bVal) | (0x2);
		}
		else if ( ((dTemp >> 20) & (0x01)) == 0x01)
		{
		     bVal = (bVal)| (0x4);
		}

		if ( ((dTemp >> 17) & (0x01)) == 0x00)
		{
			bVal = (bVal) & (0xFE);
		}
		else if ( ((dTemp >> 18) & (0x01)) == 0x00)
		{
			bVal = (bVal) & (0xFD);
		}
		else if ( ((dTemp >> 20) & (0x01)) == 0x00)
		{
			bVal = (bVal) & (0xFC);
		}

		switch (bVal)
		{
			case 0:
				p->base  = RAM_BASE;
				p->total = ((UINT64)4 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);

				pfa      = PCI_MAKE_PFA (0, 0, 0);
				dMem     = PCI_READ_DWORD (pfa, 0xbc);
				p->len   = (UINT32) (dMem - RAM_BASE);

				dTemp    = PCI_READ_DWORD (pfa, 0xac);
				dMem     = (dTemp << 12) & 0x0000f000;
				dTemp    = PCI_READ_DWORD (pfa, 0xa8);
				dMem    |= (dTemp >> 20) & 0x00000fff;
				p->len64 = dMem * 0x100000;
				break;

			case 1:
				p->base  = RAM_BASE;
				p->total = ((UINT64)8 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);

				pfa      = PCI_MAKE_PFA (0, 0, 0);
				dMem     = PCI_READ_DWORD (pfa, 0xbc);
				p->len   = (UINT32) (dMem - RAM_BASE);

				dTemp    = PCI_READ_DWORD (pfa, 0xac);
				dMem     = (dTemp << 12) & 0x0000f000;
				dTemp    = PCI_READ_DWORD (pfa, 0xa8);
				dMem    |= (dTemp >> 20) & 0x00000fff;
				p->len64 = dMem * 0x100000;
				break;

			case 2:
				p->base  = RAM_BASE;
				p->total = ((UINT64)16 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);

				pfa      = PCI_MAKE_PFA (0, 0, 0);
				dMem     = PCI_READ_DWORD (pfa, 0xbc);
				p->len   = (UINT32) (dMem - RAM_BASE);

				dTemp    = PCI_READ_DWORD (pfa, 0xac);
				dMem     = (dTemp << 12) & 0x0000f000;
				dTemp    = PCI_READ_DWORD (pfa, 0xa8);
				dMem    |= (dTemp >> 20) & 0x00000fff;
				p->len64 = dMem * 0x100000;
				break;

			case 4:
				p->base  = RAM_BASE;
				p->total = ((UINT64)2 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);

				pfa      = PCI_MAKE_PFA (0, 0, 0);
				dMem     = PCI_READ_DWORD (pfa, 0xbc);
				p->len   = (UINT32) (dMem - RAM_BASE);

				p->len64 = 0;
				break;
		}
	}
    else if(dPciReadReg(pfa, 0x00, REG_32) == HASWELL_ID)
    {
		p->type = RAM_HASWELL;

		// obtain ram size in GB from IO strap settings
		// output parameter in bVal is GPIO strap setting
		// returned parameter in bVal is memory size
		board_service (SERVICE__BRD_GET_DRAM_SIZE_FROM_GPIO, NULL, (void *)&bVal);
		p->total = ((UINT64)bVal * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);

        pfa      = PCI_MAKE_PFA (0, 0, 0);
        dMem     = PCI_READ_DWORD (pfa, HASWELL_TOLUD_REG);

        p->len   = (UINT32) ((dMem & 0xFFFFFFFEL) - p->base);
        dTemp    = PCI_READ_DWORD (pfa, HASWELL_TOUUD_REG_HI);

        dMem     = (dTemp << 12) & 0x0000f000;
        dTemp    = PCI_READ_DWORD (pfa, HASWELL_TOUUD_REG_LO);

        dMem    |= (dTemp >> 20) & 0x00000fff;

        p->len64 = dMem * 0x100000;
    }
	else
	{
		p->type = RAM_GENERIC;
		vPciWriteReg(pfa, 0xD0, REG_32, 0xD0020400);

		#ifdef DEBUG
			sprintf(buffer, "MEM SIZE: 0x%x\n", dPciReadReg(pfa, 0xD4, REG_32));
			vConsoleWrite(buffer);
		#endif
		if(dPciReadReg(pfa, 0x00, 0x03) == 0x35B08086)
			p->len = (dPciReadReg (pfa, 0x67, REG_8) * 0x4000000) - RAM_BASE;
		else if(dPciReadReg(pfa, 0x00, 0x03) == 0x81008086)
			p->len = 0x40000000 - RAM_BASE; /* change this to algorithm based */
		else
			p->len = (dPciReadReg (pfa, 0x60, REG_8) * 0x8000000) - RAM_BASE;
	}

	/* align base address and length to dword */
	if (p->base % 4 != 0)
	{
		p->base += 4 - (p->base % 4);
		if (p->len % 4 == 0)	/* don't short length twice */
			p->len -= 4;
	}

	if (p->len % 4 != 0)
		p->len -= (p->len % 4);

#ifdef DEBUG
		/* tell user of altered parameters */
		sprintf (buffer, "Start Address Below 4GB=%08Xh, Length Below 4GB=%08Xh\n"\
						 "End of Total RAM =%04X%08Xh, Total RAM=%04X%08Xh\n",
						 p->base, p->len,
						 ((UINT32)(p->len64>>32)), ((UINT32)p->len64),
						 ((UINT32)(p->total>>32)), ((UINT32)p->total)
				);
		vConsoleWrite(buffer);
#endif

	return (E__OK);
}


/*******************************************************************************
 * get_test_ram_rows - get rows that contain testable RAM
 *
 * METHOD: sets up global rows structure
 *
 * PARAMS: (RAM_ROWS*)p - pointer to empty test rows structure
 *
 */
static void get_test_ram_rows
(
	RAM_ROWS* p
)
{
	UINT32 id, lower, upper, row, i;
	PCI_PFA 	pfa;

	/* set i as the index into the test rows structure */

	i = 0;
	pfa = PCI_MAKE_PFA (0, 0, 0);

	/* check VID & DID for a recognised North Bridge */

	id = dPciReadReg (pfa, 0, REG_32);

	if ((id == (PCI_VID_INTEL | (PCI_DID_E7520_MCH << 16))))
	{
		/* Intel E7520 has 4 rows */
		lower = 0;

		for (row = 0; row < 4; row++)
		{
			/* calculate upper row address (note 64MB granularity)	*/
			/* if in single channel mode							*/

			upper = dPciReadReg (pfa, 0x60 + row, REG_8);
			upper *= 0x8000000;

			/* add to test if lower boundary lies within RAM ... */
			if ((lower >= ram.base) &&	(lower < (ram.base + ram.len)))
			{
				p->dwd [i] = (lower - ram.base) / 4;
				p->row [i] = row;
				i++;
			}

			/* ... or upper boundary lies within RAM */
			else if ((lower < ram.base) && (upper > ram.base))
			{
				p->dwd [i] = 0; /* base */
				p->row [i] = row;
				i++;
			}

			/* the upper boundary is the lower boundary for next iteration */
			lower = upper;
		}
	}
	else if ((id == (PCI_VID_INTEL | (PCI_DID_LE3100_MCH << 16))))
	{
		/* Intel LE3100 has 4 rows */
		lower = 0;

		for (row = 0; row < 4; row++)
		{
			/* calculate upper row address (note 64MB granularity) 	*/
			/* if in single channel mode 							*/

			upper = dPciReadReg (pfa, 0x60 + row, REG_8);
			upper *= 0x4000000;

			/* add to test if lower boundary lies within RAM ... */
			if ((lower >= ram.base) &&  (lower < (ram.base + ram.len)))
			{
				p->dwd [i] = (lower - ram.base) / 4;
				p->row [i] = row;
				i++;
			}

			/* ... or upper boundary lies within RAM */
			else if ((lower < ram.base) && (upper > ram.base))
			{
				p->dwd [i] = 0; /* base */
				p->row [i] = row;
				i++;
			}

			/* the upper boundary is the lower boundary for next iteration */
			lower = upper;
		}
	}
	else if ((id == (PCI_VID_INTEL | (0x8100 << 16))))
	{
		/* Intel SCH has 4 rows */
		lower = 0;

		for (row = 0; row < 1; row++)
		{
			upper = 0x20000000;

			/*upper = (dPciReadReg(pfa, 0xD4, REG_32) & 0xFFFF0000);*/
			/* add to test if lower boundary lies within RAM ... */

			if ((lower >= ram.base) &&  (lower < (ram.base + ram.len)))
			{
				p->dwd [i] = (lower - ram.base) / 4;
				p->row [i] = row;
				i++;
			}

			/* ... or upper boundary lies within RAM */
			else if ((lower < ram.base) && (upper > ram.base))
			{
				p->dwd [i] = 0; /* base */
				p->row [i] = row;
				i++;
			}

			/* the upper boundary is the lower boundary for next iteration */
			lower = upper;
		}
	}
	else if ((id == (PCI_VID_INTEL | (0x2A40 << 16))) ||
             (id == (PCI_VID_INTEL | (0x0044 << 16))) ||
             (id == (PCI_VID_INTEL | (0x0104 << 16))) ||
             (id == (PCI_VID_INTEL | (0x0C04 << 16))) )
	{
		/* Intel SCH has 4 rows */
		lower = 0;

		for (row = 0; row < 4; row++)
		{
			upper = 0x20000000;

			/*upper = (dPciReadReg(pfa, 0xD4, REG_32) & 0xFFFF0000);*/
			/* add to test if lower boundary lies within RAM ... */
			if ((lower >= ram.base) &&  (lower < (ram.base + ram.len)))
			{
				p->dwd [i] = (lower - ram.base) / 4;
				p->row [i] = row;
				i++;
			}

			/* ... or upper boundary lies within RAM */
			else if ((lower < ram.base) && (upper > ram.base))
			{
				p->dwd [i] = 0; /* base */
				p->row [i] = row;
				i++;
			}

			/* the upper boundary is the lower boundary for next iteration */
			lower = upper;
		}
	}

	p->nrows = i;
}




/*******************************************************************************
 * test_data_bus - tests each data pin can be set and cleared independently of
 *                 the others.
 *
 * METHOD: performs a marching 1 test across the data bus. tests each memory
 *         chip (row) within the test range.
 *
 * PARAMS: (UINT32*)p - pointer to RAM
 *
 * RETURN: E__OK otherwise Error Code
 *
 */
static UINT32 test_data_bus (UINT32* p)
{
	UINT32 val, mask, i;

	/* loop through each bit on the dword-sized data bus */
	for (mask = 1; mask != 0; mask <<= 1)
	{
		val = mask;

		/* test setting one bit (mask) and all-but-one bit (~mask) */
		do
		{
			/* write the test value to each memory chip under test */
			for (i = 0; i < rows.nrows; i++)
			{
				p[rows.dwd[i]] = val;
			}

			/* wait before reading else might just read bus pins, not chips */

			if (rows.nrows <= 1)
				vDelay (1);

			/* verify each row */
			for (i = 0; i < rows.nrows; i++)
			{
				if (p[rows.dwd[i]] != val)
				{
					#ifdef DEBUG
						sprintf (buffer, "Data bus error @ %08X expected %08X got %08X",
								ram.base + 4 * rows.dwd[i], val, p[rows.dwd[i]]);
						vConsoleWrite (buffer);
					#endif
					return E__DATA_BUS_ERR;
				}
			}

			/* invert the test data for the next pass */
			val = ~val;

		} while (val != mask);
	}

	/* write zero to location and verify */
	p[0] = 0;
	if (p[0] != 0)
		return (E__DATA_BUS_ERR);

	return (E__OK);
}




/*******************************************************************************
 * test_addr_bus - tests address bus
 *
 * METHOD: tests address bits do not stick high by writing a pattern at offset 0
 *         and checking it does not appear elsewhere, and tests address bits do
 *         not stick low by writing a pattern at each offset and checking that
 *         no other offset's contents have been changed. the test deals with a
 *         subset of offsets: those at power-of-2 addresses.
 *
 * PARAMS: (UINT32*)p - pointer to RAM
 *
 * RETURN: E__OK or error code
 *******************************************************************************
 */

static UINT32 test_addr_bus (volatile UINT32* p)
{
	UINT32 i, j, mask;

	/* set top limit of testable RAM */
	mask = (ram.len - 1) >> 2;
	sysInvalidateTLB();

	/* write test pattern at power-of-2 addresses */
	for (i = sizeof (UINT32); (i & mask) != 0; i <<= 1)
	{
		p[i] = 0xAAAAAAAA;
	}

	/* test for address bits stuck high */
	p[0] = 0x55555555;

	for (i = sizeof (UINT32); (i & mask) != 0; i <<= 1)
	{
		if (p[i] != 0xAAAAAAAA)
		{
			#ifdef DEBUG
				sprintf (buffer, "Data mismatch @ %08Xh expected %08Xh got %08Xh,stuck high\n",
						ram.base + i, 0xAAAAAAAA, p[i]);
				sysDebugWriteString (buffer);
			#endif
			return (E__ADDR_STUCK_HIGH);
		}
	}

	p[0] = 0xAAAAAAAA;

	/* test for address bits stuck low or shorted */
	for (i = sizeof (UINT32); (i & mask) != 0; i <<= 1)
	{
		p[i] = 0x55555555;
		for (j = sizeof (UINT32); (j & mask) != 0; j <<= 1)
		{
			if ((p[j] != 0xAAAAAAAA) && (i != j))
			{
				#ifdef DEBUG
					sprintf (buffer, "Data mismatch @ %08Xh expected %08Xh got %08Xh\n",
							ram.base + j, 0xAAAAAAAA, p[j]);
					sysDebugWriteString (buffer);
				#endif
				return (E__ADDR_SHORT);
			}
		}

		p[i] = 0xAAAAAAAA;
	}

	for (i = sizeof (UINT32); (i & mask) != 0; i <<= 1)
	{
		p[i] = 0x0;
	}

	p[0] = 0x0;
	return (E__OK);
}



/*******************************************************************************
 * dmemaddrtest - top level RAM address bus test function
 *
 * METHOD: sets up globals describing the RAM to be tested and which memory rows
 *         that RAM exists on, then calls address bus test.
 *
 * RETURN: UINT32 error code
 *******************************************************************************
*/

static UINT32 dmemaddrtest(	void )
{
UINT32	dtest_status = E__OK;

	UINT32* p;
	PTR48   p1;
	UINT32 dhandle;
	UINT32 base, length;
	UINT32 count, i;
	UINT64 base64;
	UINT32 blockSize;
	UINT32 size;

	/* determine which block of memory to test */

	dtest_status = get_testable_ram (&ram);

	/*
	 * ***************** TEST RAM BELOW 4GB FIRST *********************
	 */
	base = ram.base;

	if(ram.len > 0x40000000)
		size  = 0x40000000 - ram.base;
	else
		size = ram.len;

	length = size;
	blockSize = 0x40000000;			//1GB
	count = (size + (blockSize - 1)) / blockSize;
	if (size > blockSize)
		length = blockSize;

#ifdef DEBUG
	sprintf (buffer, "\n\nCount %d\n", count);
	vConsoleWrite(buffer);
#endif
	for (i = 0; ((i < count) && (dtest_status == E__OK)); i++ )
	{
		dhandle = dGetPhysPtr (base, length, &p1, &p);

		if (dhandle != E__FAIL)
		{
			#ifdef DEBUG
			/* tell user of Log Addr */
				sprintf (buffer, "\nstart address=%08Xh, Length=%08Xh\n",
						 base, length);
				vConsoleWrite(buffer);
				sprintf (buffer, "pointer address=%08Xh, data=%08Xh\n",
						 (UINT32)p, *p);
				vConsoleWrite(buffer);
			#endif
		}
		else
		{
			#ifdef DEBUG
				vConsoleWrite("Error Allocating memory\n");
			#endif

			dtest_status = E__RAM_NO_MEM;
		}

		if(dtest_status == E__OK)
		{
			/* get the numbers of the rows to test */
			get_test_ram_rows (&rows);

			#ifdef DEBUG
				sprintf (buffer, "Obtained RAM rows %x %x %x\n",
						 rows.dwd[0], rows.row[0], rows.nrows);
				vConsoleWrite (buffer);
			#endif

			/* data bus test */
			if(rows.nrows)
			{
				dtest_status = test_addr_bus (p) ;
				#ifdef DEBUG
					vConsoleWrite ("Tested Address Bus\n");
				#endif
			}
			else
			{
				#ifdef DEBUG
					vConsoleWrite("Error Allocating memory\n");
				#endif
				dtest_status = E__RAM_NO_MEM;
			}

			vFreePtr(dhandle);

			if(dtest_status == E__OK)
			{
				base += blockSize;
				if (base + blockSize > size)
				{
					length = size - base;
				}
				else if (base + blockSize < size)
				{
					length = blockSize;
				}
				#ifdef DEBUG
				sprintf (buffer, "new length %x\n", length);
				vConsoleWrite(buffer);
				#endif
			}
		}
	}


	// If there is an error or there is no RAM above 4GB the return
	if( (dtest_status != E__OK) || (ram.len64 == 0) )
		return(dtest_status);


	/*
	 * ***************** TEST RAM ABOVE 4GB *********************
	 */
	base64 = 0x100000000LL;
	if(ram.type == RAM_GS45)
	{
		count = ram.len64 / 0x40000000LL;
		if(count == 0)
			length  = ram.len64 % 0x40000000LL;
		else
			length  = 0x40000000;

		if(ram.len64 % 0x40000000LL)
			count++;
	}
	else
	{
		if(ram.len64 > (0x100000000LL + 0x40000000LL))
	{
		vConsoleWrite("1\n");
		length  = 0x40000000;
	}
	else
	{
		vConsoleWrite("2\n");
			length = (UINT32)(ram.len64-0x100000000LL);
	}

		count = (ram.len64 - 0x100000000LL) / 0x40000000LL;
		if((ram.len64-0x100000000LL) % 0x40000000LL)
		count++;
	}

#ifdef DEBUG
	sprintf (buffer, "\n\nCount 64 %d\n", count);
	vConsoleWrite(buffer);
#endif
	for (i = 0; ((i < count) && (dtest_status == E__OK)); i++ )
	{
		dhandle = dGetPhysPtr64(base64, length, &p1, &p);
		if (dhandle != E__FAIL)
		{
			#ifdef DEBUG
			/* tell user of Log Addr */
				sprintf (buffer, "\nstart address=%04X%08Xh, Length=%08Xh\n",
						 (UINT32)(base64>>32), (UINT32)(base64), length);
				vConsoleWrite(buffer);
				sprintf (buffer, "pointer address=%08Xh, data=%08Xh\n",
						 (UINT32)p, *p);
				vConsoleWrite(buffer);
			#endif
		}
		else
		{
			#ifdef DEBUG
				vConsoleWrite("Error Allocating memory\n");
			#endif

			dtest_status = E__RAM_NO_MEM;
		}

		if(dtest_status == E__OK)
		{
			/* get the numbers of the rows to test */
			get_test_ram_rows (&rows);

			#ifdef DEBUG
				sprintf (buffer, "Obtained RAM rows %x %x %x\n",
						 rows.dwd[0], rows.row[0], rows.nrows);
				vConsoleWrite (buffer);
			#endif

			/* data bus test */
			if(rows.nrows)
			{
				dtest_status = test_addr_bus (p) ;
				#ifdef DEBUG
					vConsoleWrite ("Tested Address Bus\n");
				#endif
			}
			else
			{
				#ifdef DEBUG
					vConsoleWrite("Error Allocating memory\n");
				#endif
				dtest_status = E__RAM_NO_MEM;
			}

			vFreePtr(dhandle);

			if(dtest_status == E__OK)
			{
				base64  = base64 + (UINT64)length;
				length  = 0x40000000;

				if(ram.type == RAM_GS45)
				{
					if( ((UINT64)length + base64) > (ram.len64 + 0x100000000LL) )
					{
						length = (UINT32)(ram.len64 % 0x40000000LL);
						#ifdef DEBUG
							sprintf (buffer, "new length %x\n", length);
							vConsoleWrite(buffer);
						#endif
					}
				}
				else
				{
				if( ((UINT64)length + base64) >  ram.len64 )
				{
					length = (UINT32)(ram.len64 - base64);
					#ifdef DEBUG
					sprintf (buffer, "new length %x\n", length);
					vConsoleWrite(buffer);
					#endif
				}
			}
		}
	}
	}

	return(dtest_status);

}





/*******************************************************************************
 * dmemdatatest - top level RAM data bus test function
 *
 * METHOD: sets up globals describing the RAM to be tested and which memory rows
 *         that RAM exists on, then calls data bus test.
 *
 * RETURN: UINT32 error code
 *******************************************************************************
*/
static UINT32 dmemdatatest(void)
{
	UINT32	dtest_status = E__OK;
	UINT32* p;
	PTR48   p1;
	UINT32 dhandle;
	UINT32 base, length;
	UINT32 count, i;
	UINT64 base64;
	UINT32 blockSize;
	UINT32 size;

	/* determine which block of memory to test */

	dtest_status = get_testable_ram (&ram);

	/*
	 * ***************** TEST RAM BELOW 4GB FIRST *********************
	 */
	base = ram.base;

	if(ram.len > 0x40000000)
		size  = 0x40000000 - ram.base;
	else
		size = ram.len;

	length = size;
	blockSize = 0x40000000;			//1GB
	count = (size + (blockSize - 1)) / blockSize;
	if (size > blockSize)
		length = blockSize;

#ifdef DEBUG
	sprintf (buffer, "\n\nCount %d\n", count);
	vConsoleWrite(buffer);
#endif
	for (i = 0; ((i < count) && (dtest_status == E__OK)); i++ )
	{
		dhandle = dGetPhysPtr (base, length, &p1, &p);
		if (dhandle != E__FAIL)
		{
			#ifdef DEBUG
			/* tell user of Log Addr */
				sprintf (buffer, "\nstart address=%08Xh, Length=%08Xh\n",
						 base, length);
				vConsoleWrite(buffer);
				sprintf (buffer, "pointer address=%08Xh, data=%08Xh\n",
						 (UINT32)p, *p);
				vConsoleWrite(buffer);
			#endif
		}
		else
		{
			#ifdef DEBUG
				vConsoleWrite("Error Allocating memory\n");
			#endif

			dtest_status = E__RAM_NO_MEM;
		}

		if(dtest_status == E__OK)
		{
			/* get the numbers of the rows to test */
			get_test_ram_rows (&rows);

			#ifdef DEBUG
				sprintf (buffer, "Obtained RAM rows %x %x %x\n",
						 rows.dwd[0], rows.row[0], rows.nrows);
				vConsoleWrite (buffer);
			#endif

			/* data bus test */
			if(rows.nrows)
			{
				dtest_status = test_data_bus (p) ;
				#ifdef DEBUG
					vConsoleWrite ("Tested Data Bus\n");
				#endif
			}
			else
			{
				#ifdef DEBUG
					vConsoleWrite("Error Allocating memory\n");
				#endif
				dtest_status = E__RAM_NO_MEM;
			}

			vFreePtr(dhandle);

			if(dtest_status == E__OK)
			{
				base += blockSize;
				if (base + blockSize > size)
				{
					length = size - base;
				}
				else if (base + blockSize < size)
				{
					length = blockSize;
				}
				#ifdef DEBUG
				sprintf (buffer, "new length %x\n", length);
				vConsoleWrite(buffer);
				#endif
			}
		}
	}


	// If there is an error or there is no RAM above 4GB the return
	if( (dtest_status != E__OK) || (ram.len64 == 0) )
		return(dtest_status);

	/*
	 * ***************** TEST RAM ABOVE 4GB *********************
	 */
	base64 = 0x100000000LL;

	if(ram.type == RAM_GS45)
	{
		count = ram.len64 / 0x40000000LL;
		if(count == 0)
			length  = ram.len64 % 0x40000000LL;
		else
			length  = 0x40000000;

		if(ram.len64 % 0x40000000LL)
			count++;
	}
	else
	{
		if(ram.len64 > (0x100000000LL + 0x40000000LL))
		{
			vConsoleWrite("1\n");
			length  = 0x40000000;
		}
		else
		{
			vConsoleWrite("2\n");
			length = (UINT32)(ram.len64-0x100000000LL);
		}

		count = (ram.len64 - 0x100000000LL) / 0x40000000LL;
		if((ram.len64-0x100000000LL) % 0x40000000LL)
		count++;
	}

	#ifdef DEBUG
		sprintf (buffer, "\n\nCount 64 %d\n", count);
		vConsoleWrite(buffer);
	#endif

	for (i = 0; ((i < count) && (dtest_status == E__OK)); i++ )
	{
		dhandle = dGetPhysPtr64(base64, length, &p1, &p);
		if (dhandle != E__FAIL)
		{
			#ifdef DEBUG
			/* tell user of Log Addr */
				sprintf (buffer, "\nstart address=%04X%08Xh, Length=%08Xh\n",
						 (UINT32)(base64>>32), (UINT32)(base64), length);
				vConsoleWrite(buffer);
				sprintf (buffer, "pointer address=%08Xh, data=%08Xh\n",
						 (UINT32)p, *p);
				vConsoleWrite(buffer);
			#endif
		}
		else
		{
			#ifdef DEBUG
				vConsoleWrite("Error Allocating memory\n");
			#endif

			dtest_status = E__RAM_NO_MEM;
		}

		if(dtest_status == E__OK)
		{
			/* get the numbers of the rows to test */
			get_test_ram_rows (&rows);

			#ifdef DEBUG
				sprintf (buffer, "Obtained RAM rows %x %x %x\n",
						 rows.dwd[0], rows.row[0], rows.nrows);
				vConsoleWrite (buffer);
			#endif

			/* data bus test */
			if(rows.nrows)
			{
				dtest_status = test_data_bus (p) ;
				#ifdef DEBUG
					vConsoleWrite ("Tested Data Bus\n");
				#endif
			}
			else
			{
				#ifdef DEBUG
					vConsoleWrite("Error Allocating memory\n");
				#endif
				dtest_status = E__RAM_NO_MEM;
			}

			vFreePtr(dhandle);

			if(dtest_status == E__OK)
			{
				base64  = base64 + (UINT64)length;
				length  = 0x40000000;

				if(ram.type == RAM_GS45)
				{
					if( ((UINT64)length + base64) > (ram.len64 + 0x100000000LL) )
					{
						length = (UINT32)(ram.len64 % 0x40000000LL);
						#ifdef DEBUG
							sprintf (buffer, "new length %x\n", length);
							vConsoleWrite(buffer);
						#endif
					}
				}
				else
				{
				if( ((UINT64)length + base64) >  ram.len64 )
				{
					length = (UINT32)(ram.len64 - base64);
					#ifdef DEBUG
					sprintf (buffer, "new length %x\n", length);
					vConsoleWrite(buffer);
					#endif
					}
				}
			}
		}
	}

	return(dtest_status);
}





/*******************************************************************************
 * vSmbInit - Obtains the IO address for the SMBus if available if not assigns
 *				to a fixed location
 *
 * RETURN: E__OK or error code
 *******************************************************************************
*/

static void vSmbInit (void)
{
	UINT8	bTemp;
	PCI_PFA pfa;
	pfa = PCI_MAKE_PFA (0, 31, 3);

	wIoBase = (UINT16)dPciReadReg(pfa, 0x20, REG_16);
	wIoBase &= 0xFFFE;
#ifdef DEBUG
	sprintf(buffer, "wIoBase: 0x%x\n", wIoBase);
	vConsoleWrite(buffer);
#endif

	if (wIoBase == 0)
	{
		vPciWriteReg(pfa, 0x20, REG_16, 0x4000);
		wIoBase = 0x4000;
#ifdef DEBUG
		sprintf(buffer, "No BIOS supplied SMB base 0x%x\n", dPciReadReg(pfa, 0x20, REG_16));
		vConsoleWrite (buffer);
#endif
	}

	bTemp = (UINT8)dPciReadReg(pfa, 0x40, REG_8);	/* enable I/F */
	vPciWriteReg (pfa, 0x40, REG_8, bTemp | 0x01);

} /* vSmbInit () */





static UINT8 smbReadReg( UINT8 bReg )
{
	return ((UINT8)dIoReadReg(wIoBase + bReg, REG_8));

} /* smbReadReg () */



static void smbWriteReg
(
	UINT8	bReg,
	UINT8	bData
)
{
	vIoWriteReg (wIoBase + bReg, REG_8, bData);

} /* smbWriteReg () */



/*******************************************************************************
 * dSmbReadByte - Reads the specified byte in SM device, SODIMM
 *
 *
 * RETURN: E__OK or error code
 *******************************************************************************
*/

static UINT32 dSmbReadByte(	UINT8 bDimm, UINT8 bByte, UINT8* pbData)
{
	UINT32	dStatus;
	UINT16	wTimeout;
	UINT8	bAddr;
	UINT8	bTemp;

	bAddr = (bDimm << 1) | 0xA0;

	smbWriteReg (SMBHSTCMD, bByte);
	smbWriteReg (SMBHSTADD, bAddr | 0x01);

	bTemp = smbReadReg (SMBHSTCNT);
	smbWriteReg (SMBHSTCNT, (bTemp & 0xF3) | SMB_READ);

	bTemp = smbReadReg (SMBHSTSTS);
	smbWriteReg (SMBHSTSTS, bTemp | 0x1F);		/* clear all status bits */

	bTemp = smbReadReg (SMBSLVSTS);
	smbWriteReg (SMBSLVSTS, bTemp | 0x3C);		/* clear all status bits */

	bTemp = smbReadReg (SMBHSTCNT);
	smbWriteReg (SMBHSTCNT, bTemp | SMB_START);

	wTimeout = 10;

	do
	{
		bTemp = smbReadReg (SMBHSTSTS);
		bTemp &= 0x1F;

		wTimeout--;
		vDelay (1);
	} while ((bTemp != 0x02) && ((bTemp & 0x1C) == 0) && (wTimeout > 0));

	if (bTemp == 0x02)
	{
		smbWriteReg (SMBHSTSTS, bTemp | 0x02);	/* clear all status bit */
		*pbData = smbReadReg (SMBHSTDAT0);
		dStatus = E__OK;
	}
	else
		dStatus = E__SPD_RD;

	return (dStatus);

} /* dSmbReadByte () */



/*******************************************************************************
 * dspdeepromtest - top level SPD data test function
 *
 * METHOD: Calculates the checksum of the SPD data 0 to 62 bytes
 *         and compares with the checksum value at byte 63.
 *
 * RETURN: E__OK or error code
 *******************************************************************************
*/
static UINT32 dspdeepromtest( UINT32 sodimm_addr )
{
	UINT32 i, checksum = 0;
	UINT8 d[64] = {0};

	for (i = 0; i < 63; i++)
	{
		if (dSmbReadByte(sodimm_addr, i, &d[i]) == E__OK)
		{
			checksum += d[i];
#ifdef DEBUG
			sprintf(buffer, "Data Read D[%d]: %d\n", i, d[i]);
			vConsoleWrite(buffer);
#endif
		}
		else
			return (E__NO_SPD);
	}

	if (dSmbReadByte(sodimm_addr, i, &d[63]) == E__OK)
	{
		if(d[63] == (checksum % 256))
		{
			return (E__OK);
		}
		else
		{
#ifdef DEBUG
		sprintf(buffer, "Checksum: %d %d \n", (checksum % 256), d[63]);
		vConsoleWrite(buffer);
#endif
			return (E__SPD_CHKSUM);
		}
	}
	else
		return (E__ACCESS_SPD);
}


/*****************************************************************************
 * Memory Data line test: test function for DRAM data line interconnect
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (MemDatalineTest, "Memory Data Line Connectivity")
{
	UINT32 dtest_status;
	//sysDebugWriteString("Before Cache Disable\n");
	//sysCacheDisable();
	cache_off();

	dtest_status = dmemdatatest();

	//sysDebugWriteString("After dmemdatatest()\n");
	//sysCacheEnable();
	cache_on();
	return (dtest_status);
} /* MemDatalineTest */


/*****************************************************************************
 * Memory Address line test: test function for DRAM Address line interconnect
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (MemAddrlineTest, "Memory Address Line Connectivity")
{
	UINT32 dtest_status;
	//sysCacheDisable();
	cache_off();
	dtest_status = dmemaddrtest();
	//sysCacheEnable();
	cache_on();
	return (dtest_status);
} /* MemAddrlineTest */


/*****************************************************************************
 * SPD EEPROM checksum test: test function for SPD EEPROM Contents
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (MemSPDEEPROMTest, "SPD EEPROM Checksum")
{
	UINT32 dtest_status;

	vSmbInit();

	dtest_status = dspdeepromtest(SODIMM_ADDR2);
	if(dtest_status == E__OK)
		dtest_status = dspdeepromtest(SODIMM_ADDR3);
	else
		return(dtest_status + SODIMM_ADDR2);
	if(dtest_status == E__OK)
		return (dtest_status);
	else
		return (dtest_status + SODIMM_ADDR3);
} /* MemSPDEEPROMTest */


/*****************************************************************************
 * Memory ECC Test: test function for Memory controllers ECC
 *
 * RETURNS: Test status
 */
TEST_INTERFACE (MemEccTest, "Memory Controller ECC Test")
{
	UINT32 dtest_status,temp32,dTemp;
	UINT64 dPciexBar,dMchBar;
	PCI_PFA pfa;
	UINT32  p;
	PTR48 p1;
	UINT32 dhandle;
	UINT32 dEccErrLog0C0 =0,dEccErrLog1C0=0,dEccErrLog0C1=0,dEccErrLog1C1=0;

	/* Flip ECC bits and run test */
	pfa = PCI_MAKE_PFA (0, 0, 0);

	temp32 = dPciReadReg(pfa, 0x00, REG_32);
	if ((temp32 == 0x01048086) ||
		(temp32 == HASWELL_ID))
	{

		dTemp = dPciReadReg(pfa,0x60,REG_32);
		dTemp &= 0xF0000000;
		dPciexBar = dPciReadReg(pfa,0x64,REG_32);
		dPciexBar &= 0x0000000F;
		dPciexBar = (dPciexBar << 31) | (dTemp);
		dTemp = dPciReadReg(pfa,0x60,REG_32);
		dTemp = ((dTemp>>1) & (0x00000003));

		if (dTemp == 0x00)
		{
			dPciexBar  = dPciexBar & 0xFF000000;
		}
		else if (dTemp == 0x01)
		{
			dPciexBar  = dPciexBar & 0xFE000000;
		}
		else if (dTemp == 0x02)
		{
			dPciexBar  = dPciexBar & 0xFC000000;
		}

#ifdef DEBUG
			sprintf(buffer, "PCI Ex Bar 0x%x\n",(UINT32)dPciexBar);
			vConsoleWrite(buffer);
#endif

		dTemp = dPciReadReg(pfa,0x48,REG_32);
		dTemp &=  0xFFFFF800;

		dMchBar = dPciReadReg(pfa,0x4C,REG_32);
		dMchBar &= 0x00000007;

		dMchBar = (dMchBar << 31) | (dTemp);

#ifdef DEBUG
			sprintf(buffer, "MCH Bar 0x%x\n",(UINT32)dMchBar);
			vConsoleWrite(buffer);
#endif
			dhandle = dGetPhysPtr (dMchBar, 0x100000, &p1, (void*)&p);

			if (dhandle != E__FAIL)

			{
#ifdef DEBUG
				/* tell user of Log Addr */
				sprintf (buffer, "User specified log start address=%08Xh, data=%08Xh\n",
							(UINT32)(p), vReadDWord(p));
				sysDebugWriteString (buffer);
#endif
			}
			else
			{
#ifdef DEBUG
				sysDebugWriteString("Error Allocating memory\n");
#endif
				return (E__RAM_NO_MEM);
			}


			dEccErrLog0C0 = vReadDWord(p+0x40C8);
#ifdef DEBUG
			sprintf(buffer, "dEccErrLog0C0 0x%x\n",dEccErrLog0C0);
			vConsoleWrite(buffer);
#endif

			dEccErrLog1C0 = vReadDWord(p+0x40CC);

#ifdef DEBUG
				sprintf(buffer, "dEccErrLog1C0 0x%x\n",dEccErrLog1C0);
				vConsoleWrite(buffer);
#endif
			if (dEccErrLog0C0)
			{
#ifdef DEBUG
				sprintf(buffer, "Err Column  0x%x Error Row 0x%x\n",((dEccErrLog1C0>>15) & (0x0000FFFF)),((dEccErrLog1C0) & (0x0000FFFF)));
				vConsoleWrite(buffer);
#endif
				return (E__ECC_CHAN0);
			}

			/* Prevent compiler warning */
			dEccErrLog1C0 = 0;
			dEccErrLog0C0 = dEccErrLog1C0;


			dEccErrLog0C1 = vReadDWord(p+0x44C8);
#ifdef DEBUG
			sprintf(buffer, "dEccErrLog0C1 0x%x\n",dEccErrLog0C1);
			vConsoleWrite(buffer);
#endif
    		dEccErrLog1C1 = vReadDWord(p+0x44CC);
#ifdef DEBUG
			sprintf(buffer, "dEccErrLog1C1 0x%x\n",dEccErrLog1C1);
			vConsoleWrite(buffer);
#endif

			if (dEccErrLog0C1)
			{
				#ifdef DEBUG
					sprintf(buffer, "Err Column  0x%x Error Row 0x%x\n",((dEccErrLog1C1>>15) & (0x0000FFFF)),((dEccErrLog1C1) & (0x0000FFFF)));
					vConsoleWrite(buffer);
				#endif
				return (E__ECC_CHAN1);
			}

			/* Prevent compiler warning */
			dEccErrLog1C1 = 0;
			dEccErrLog0C1 = dEccErrLog1C1;
			dtest_status = E__OK;
	}
	else
	{
		#ifdef DEBUG
			sprintf(buffer, "ECC Diag Reg 0x%x\n", dPciReadReg(pfa, 0x84, REG_32));
			vConsoleWrite(buffer);
		#endif

		vPciWriteReg(pfa, 0x84, REG_32, (dPciReadReg(pfa, 0x84, REG_32)) | (1 << 20));

		#ifdef DEBUG
			sprintf(buffer, "ECC Diag Reg 0x%x\n", dPciReadReg(pfa, 0x84, REG_32));
			vConsoleWrite(buffer);
		#endif

		//sysCacheDisable();
			cache_off();
		dtest_status = dmemdatatest();
		//sysCacheEnable();
		cache_on();

		#ifdef DEBUG
			sprintf(buffer, "ECC Diag Reg 0x%x\n", dPciReadReg(pfa, 0x84, REG_32));
			vConsoleWrite(buffer);
		#endif

			vPciWriteReg(pfa, 0x84, REG_32, (dPciReadReg(pfa, 0x84, REG_32)) & ~(1 << 20));

		#ifdef DEBUG
			sprintf(buffer, "ECC Diag Reg 0x%x\n", dPciReadReg(pfa, 0x84, REG_32));
			vConsoleWrite(buffer);
		#endif
	}

	if(dtest_status != E__OK)
		dtest_status = E__ECC_INDUCED_FAIL;

	return (dtest_status);
} /* MemEccTest */

/*****************************************************************************
 * wDramReadWriteBlock: tight-code testing of read-write block
 *
 * RETURNS: E_OK
 */
#if  defined(VME) && (defined(MAKE_CUTE) || defined(MAKE_BIT_CUTE))
static UINT32 wDramReadWriteBlock
(
	RAM_TEST_PARAMS*	psParams
)
{
	UINT32	dAddr;
	UINT32	dEndAddr;
	UINT32	dData;
	UINT32	dTemp;			/* holds original memory contents */

#if 0 /* Not used, HAL no longer present */
	CCT_MEM_PTR mPtr;

        memset((void*)&mPtr,0,sizeof(CCT_MEM_PTR));
        mPtr.qPhysicalAddress = psParams->block0_start;
        mPtr.dBlockSize = psParams->block0_len;

        spMemGetPhysPtr(&mPtr);

        if( mPtr.pBlock == NULL )
		{
#ifdef DEBUG
            sprintf(buffer,"Unable to create page table mapping\n");
            sysDebugWriteString (buffer);
#endif
            return E__DAT_RDWR_ERR;
        }

        dAddr = mPtr.dlogaddr;
#else
	UINT32	dHandle;
	PTR48	p1;

		dHandle = dGetPhysPtr (psParams->block0_start, psParams->block0_len, &p1, &dAddr);

		if (dHandle == E__FAIL)
		{
#ifdef DEBUG
            sprintf(buffer,"Unable to create page table mapping\n");
            sysDebugWriteString (buffer);
#endif
            return E__DAT_RDWR_ERR;
		}
#endif

#ifdef DEBUG
        sprintf(buffer,"Running read/write test at %x-%x\n",psParams->block0_start,dAddr);
        sysDebugWriteString (buffer);
#endif

	dEndAddr = dAddr + psParams->block0_len;

	while (dAddr < dEndAddr)
	{
		dTemp = *(UINT32*)dAddr;		/* read original data */
		*(UINT32*)dAddr = ~dTemp;		/* write compliment */
		dData = *(UINT32*)dAddr;		/* re-read */

		if (dData != ~dTemp)			/* compare */
		{
#ifdef DEBUG
                        sprintf(buffer,"Failure 1 at %x, data %x, temp %x\n", dAddr, dData, dTemp);
                        sysDebugWriteString (buffer);
#endif

#if 0 /* Not used, HAL no longer present */
                        spMemFreeMem(&mPtr);
#else
						vFreePtr(dHandle);
#endif

        		return (E__DAT_RDWR_ERR);
		}

		*(UINT32*)dAddr = dTemp;		/* write back original data */
		dData = *(UINT32*)dAddr;		/* re-read */

		if (dData != dTemp)				/* compare */
		{
#ifdef DEBUG
                        sprintf(buffer,"Failure 2 at %x, data %x, temp %x\n", dAddr, dData, dTemp);
                        sysDebugWriteString (buffer);
#endif

#if 0 /* Not used, HAL no longer present */
                        spMemFreeMem(&mPtr);
#else
						vFreePtr(dHandle);
#endif

        		return (E__DAT_RDWR_ERR);
		}

		dAddr += 4;
	}

#if 0 /* Not used, HAL no longer present */
        spMemFreeMem(&mPtr);
#else
		vFreePtr(dHandle);
#endif

#ifdef DEBUG
        sysDebugWriteString ("Co-ord Test passed\n");
#endif
	return (E__OK);

} /* wDramReadWriteBlock () */
#endif

/*****************************************************************************
 * wGetRamTestParams: retrieve RAM test parameters: start and length
 *
 * RETURNS: E_OK or E__PARAM_ERROR
 */
#if  defined(VME) && (defined(MAKE_CUTE) || defined(MAKE_BIT_CUTE))
static UINT32 wGetRamTestParams
(
	RAM_TEST_PARAMS*	psParams
)
{

	EXPECTED_TYPES*	psExpectedTypes;
	UINT16	wIdxStatus;
//        UINT32 status;

    /* status =*/ get_testable_ram (&ram);

	/* Load the test defaults */
	psParams->block0_start = ram.base;
	psParams->block0_len   = (ram.len - ram.base);
	psParams->block1_start = 0;
	psParams->block1_len   = 0;
    psParams->block0_psb_off= 0;
	psParams->block0_tgt_slot = 0x1f;
	psParams->block1_psb_off= 0;
	psParams->block1_tgt_slot = 0;
	psParams->mem_width= 3;					/* default */

	/* See if user has specified changes */
#ifdef DEBUG
        sprintf(buffer, "Def RAM base=%x Size=%x\n",psParams->block0_start,psParams->block0_len);
        sysDebugWriteString(buffer);
#endif
	psExpectedTypes = (EXPECTED_TYPES*)&abRamTestTypes;

	wIdxStatus = wIDX_GetParams (psExpectedTypes, (UINT8*)psParams);

        if( wIdxStatus != E__OK )
        {
            return E__INVALID_PARAM;
        }

	/* Ensure params are UINT32 aligned */

	psParams->block0_start = (psParams->block0_start + 3) & 0xFFFFFFFC;
	psParams->block0_len   = (psParams->block0_len + 3) & 0xFFFFFFFC;

	psParams->block1_start = (psParams->block1_start + 3) & 0xFFFFFFFC;
	psParams->block1_len   = (psParams->block1_len + 3) & 0xFFFFFFFC;

	/* Sanity check the supplied parameters */

	if (psParams->block0_len == 0)	/* dLength2 can be zero */
	{
#ifdef DEBUG
                sysDebugWriteString ("Invalid parameters\n");
#endif
		return (E__INVALID_PARAM);
	}

	return (E__OK);
} /* wGetRamTestParams () */
#endif

/*****************************************************************************
 * SPD EEPROM checksum test: test function for SPD EEPROM Contents
 *
 * RETURNS: Test status
 */
#if  defined(VME) && (defined(MAKE_CUTE) || defined(MAKE_BIT_CUTE))
TEST_INTERFACE (DramReadWrite, "Local Memory Read/Write Test")
{

    RAM_TEST_PARAMS	sParams;
	UINT32	wTestStatus;
#ifdef DEBUG
        sysDebugWriteString ("Executing memory read/write test");
#endif
        memset(&sParams,0,sizeof(RAM_TEST_PARAMS));

	/* Load the test defaults */
        get_testable_ram (&ram);

        sParams.block0_start = ram.base;
        sParams.block0_len = 0x1000000;

#ifdef DEBUG
        sprintf(buffer, "\nadTestParams[0]=%x adTestParams[1]=%x\n",adTestParams[0],
        		adTestParams[1]);
           sysDebugWriteString (buffer);
#endif

        if(adTestParams[0])
        {
            sParams.block0_start = adTestParams[1];
            sParams.block0_len = adTestParams[2];

            if( sParams.block0_len > (ram.len - ram.base) )
            {
                return E__INVALID_PARAM;
            } else if ( (sParams.block0_start + sParams.block0_len) >
                    (ram.base + ( ram.len -ram.base)) )
            {
                return E__INVALID_PARAM;
            } else if ( sParams.block0_start >=
                ( ram.base + (ram.len - ram.base)))
            {
                return E__INVALID_PARAM;
            }else
            {
				#ifdef DEBUG
					sysDebugWriteString( "Parameters obtained from user" );
					sprintf( buffer, "Start Address=%x Length=%x\n", sParams.block0_start,
                             sParams.block0_len );
				#endif
            }
        }
        else
		{
            /*Check if the soak master has any parameters for us*/
            wTestStatus = wGetRamTestParams (&sParams);

            if (wTestStatus == E__OK)
            {
#ifdef DEBUG
                    sysDebugWriteString ("Obtained test parameters from Soak master\n");
#endif
                    if( sParams.block0_len > 0x1000000 )
                    {
                    	sParams.block0_len = 0x1000000;
                    }

                    if( sParams.block1_len != 0 )
                    {
                    	sParams.block1_start=0;
                       	sParams.block1_len = 0;
                    }
            }
            else

            {
#ifdef DEBUG
                    sysDebugWriteString ("Using default test parameters\n");
#endif
                    sParams.block0_start = ram.base;
                    sParams.block0_len = 0x1000000;
                    sParams.block1_start = 0;
                    sParams.block1_len = 0;
            }
        }

 #ifdef DEBUG
        sprintf(buffer, "Obtained parameter base1=%x Size1=%x\n",sParams.block0_start,
                                    sParams.block0_len);
        sysDebugWriteString (buffer);

        sprintf(buffer, "Obtained parameter base2=%x Size2=%x\n",sParams.block1_start,
                                    sParams.block1_len);
        sysDebugWriteString (buffer);
#endif
        /* Start Testing memory */
	//sysCacheDisable();
	cache_off();

	wTestStatus = wDramReadWriteBlock (&sParams);

	//sysCacheEnable();
	cache_on();

	/* Report errors */

	if (wTestStatus != E__OK)
	{
#ifdef DEBUG
            sysDebugWriteString ("Error running co-op read/write test");
#endif
	}

	return (wTestStatus);
}
#endif


/*****************************************************************************
 * wDramDualAddressBlock: tight-code testing of dual-address block
 *
 * RETURNS: E_OK
 */
#if  defined(VME) && (defined(MAKE_CUTE) || defined(MAKE_BIT_CUTE))
static UINT32 wDramDualAddressBlock
(
	RAM_TEST_PARAMS*	psParams
)
{

		UINT32	dAddr1;
		UINT32	dEndAddr1;
		UINT32	dAddr2;
		UINT32	dEndAddr2;
		UINT32	dData;
#if 0 /* Not used, HAL no longer present */
		CCT_MEM_PTR mPtr1,mPtr2;

		memset((void*)&mPtr1,0,sizeof(CCT_MEM_PTR));
		memset((void*)&mPtr2,0,sizeof(CCT_MEM_PTR));

        mPtr1.qPhysicalAddress = psParams->block0_start;
        mPtr1.dBlockSize = psParams->block0_len;

        spMemGetPhysPtr(&mPtr1);

        if( mPtr1.pBlock == NULL )
        {
#ifdef DEBUG
            sprintf(buffer,"Unable to create page table mapping\n");
            sysDebugWriteString (buffer);
#endif
            return E__DAT_RDWR_ERR;
        }

        dAddr1 = mPtr1.dlogaddr;
        dEndAddr1 = dAddr1 + psParams->block0_len;

        if( psParams->block1_len != 0 )
        {
            mPtr2.qPhysicalAddress = psParams->block1_start;
            mPtr2.dBlockSize = psParams->block1_len;

            spMemGetPhysPtr(&mPtr2);

            if( mPtr2.pBlock == NULL )
            {
#ifdef DEBUG
                sprintf(buffer,"Unable to create page table mapping\n");
                sysDebugWriteString (buffer);
#endif
                return E__DAT_RDWR_ERR;
            }

            dAddr2 = mPtr2.dlogaddr;
            dEndAddr2 = dAddr2 + psParams->block1_len;
        }
#else
		UINT32	dHandle1, dHandle2;
		PTR48	p1, p2;

		dHandle1 = dGetPhysPtr (psParams->block0_start, psParams->block0_len, &p1, &dAddr1);

		if (dHandle1 == E__FAIL)
		{
 #ifdef DEBUG
            sprintf(buffer,"Unable to create page table mapping\n");
            sysDebugWriteString (buffer);
#endif
            return E__DAT_RDWR_ERR;
        }

        dEndAddr1 = dAddr1 + psParams->block0_len;

        if (psParams->block1_len != 0)
        {
			dHandle2 = dGetPhysPtr (psParams->block1_start, psParams->block1_len, &p2, &dAddr2);

			if (dHandle2 == E__FAIL)
			{
 #ifdef DEBUG
				sprintf(buffer,"Unable to create page table mapping\n");
				sysDebugWriteString (buffer);
#endif
				return E__DAT_RDWR_ERR;
			}
            dEndAddr2 = dAddr2 + psParams->block1_len;
        }

#endif

        dData = 0;		/* to prevent compiler warning */

        while (dAddr1 < dEndAddr1)
        {
        	*(UINT32*)dAddr1 = (dAddr1 << 2);
        	dData = *(UINT32*)dAddr1;

        	if (dData != (dAddr1 << 2))
        	{
#ifdef DEBUG
                sprintf(buffer,"Wrote %x at %x - Read %x at %x\n",
                		(dAddr1<<2),dAddr1,dData,dAddr1);
                sysDebugWriteString (buffer);
#endif

#if 0 /* Not used, HAL no longer present */
                spMemFreeMem(&mPtr1);
				if (psParams->block1_len != 0)
				{
					spMemFreeMem(&mPtr2);
				}
#else
				vFreePtr(dHandle1);
				if (psParams->block1_len != 0)
				{
					vFreePtr(dHandle2);
				}
#endif
        		return (E__DAT_RDWR_ERR);
        	}
        	dAddr1 += 4;
        }

        if( psParams->block1_len != 0 )
        {
            while (dAddr2 < dEndAddr2)
            {
            	*(UINT32*)dAddr2 = (dAddr2 << 2);
            	dData = *(UINT32*)dAddr2;

            	if (dData != (dAddr2 << 2))
            	{
#ifdef DEBUG
            		sprintf(buffer,"Wrote %x at %x - Read %x at %x\n",
                		(dAddr2<<2),dAddr2,dData,dAddr2);
            		sysDebugWriteString (buffer);
#endif
#if 0 /* Not used, HAL no longer present */
                    spMemFreeMem(&mPtr1);
                    spMemFreeMem(&mPtr2);
#else
					vFreePtr(dHandle1);
					vFreePtr(dHandle2);
#endif
            		return (E__DAT_RDWR_ERR);
            	}
            	dAddr2 += 4;
            }
        }


#if 0 /* Not used, HAL no longer present */
        spMemFreeMem(&mPtr1);
#else
		vFreePtr(dHandle1);
#endif

        if (psParams->block1_len != 0)
        {
#if 0 /* Not used, HAL no longer present */
            spMemFreeMem(&mPtr2);
#else
			vFreePtr(dHandle2);
#endif
        }

	return (E__OK);

} /* wDramDualAddressBlock () */
#endif



/*****************************************************************************
 * SPD EEPROM checksum test: test function for SPD EEPROM Contents
 *
 * RETURNS: Test status
 */
#if  defined(VME) && (defined(MAKE_CUTE) || defined(MAKE_BIT_CUTE))
TEST_INTERFACE (DramDualAddress, "Local Memory Dual Address Test")
{
        RAM_TEST_PARAMS	sParams;
	UINT32	wTestStatus;
#ifdef DEBUG
        sysDebugWriteString ("Executing Local Memory dual address test");
#endif
        memset(&sParams,0,sizeof(RAM_TEST_PARAMS));

	/* Load the test defaults */
        get_testable_ram (&ram);

        sParams.block0_start = ram.base;
        sParams.block0_len = 0x1000000;

        sParams.block1_start = ram.base + 0x4000000;
        sParams.block1_len = 0x1000000;

#ifdef DEBUG
        sprintf(buffer, "adTestParams[0]=%x adTestParams[1]=%x\n",adTestParams[0],
        		adTestParams[1]);
        sysDebugWriteString (buffer);
#endif
        if(adTestParams[0])
        {
            sParams.block0_start = adTestParams[1];
            sParams.block0_len = adTestParams[2];

            sParams.block1_start = adTestParams[3];
            sParams.block1_len = adTestParams[4];

            if( (sParams.block0_len > (ram.len - ram.base)) ||
                   (sParams.block1_len > (ram.len - ram.base)) )
            {
                return E__INVALID_PARAM;
            } else if ( ((sParams.block0_start + sParams.block0_len) >
                    (ram.base + ( ram.len -ram.base))) ||
                    ((sParams.block1_start + sParams.block1_len) >
                    (ram.base + ( ram.len -ram.base))) )
            {
                return E__INVALID_PARAM;
            } else if ( (sParams.block0_start >=
                ( ram.base + (ram.len - ram.base))) ||
                    (sParams.block1_start >=
                ( ram.base + (ram.len - ram.base))) )
            {
                return E__INVALID_PARAM;
            }
        }
        else
        {
#if defined(MAKE_BIT_CUTE) || defined(MAKE_CUTE)
            /*Check if the soak master has any parameters for us*/
            wTestStatus = wGetRamTestParams (&sParams);

            if (wTestStatus == E__OK)
            {
				#ifdef DEBUG
                    sysDebugWriteString ("Obtained test parameters from Soak master\n");
				#endif

                if( sParams.block0_len > 0x1000000 )
                {
                	sParams.block0_len = 0x1000000;
                }

                    if( sParams.block1_len > 0x1000000 )
                    {
                        sParams.block1_len = 0x1000000;
                    }

                    if( sParams.block1_start != 0 && sParams.block1_start < (ram.base+ram.len)
                    		 && sParams.block1_start < sParams.block0_start)
                    {
                    	sParams.block1_start = sParams.block0_start + sParams.block0_len;
                    }
            }
            else
#endif
            {
				#ifdef DEBUG
                    sysDebugWriteString ("Using default test parameters\n");
				#endif
                    sParams.block0_start = ram.base;
                    sParams.block0_len = 0x1000000;

                    sParams.block1_start = ram.base + 0x4000000;
                    sParams.block1_len = 0x1000000;
            }
        }

	#ifdef DEBUG
        sprintf(buffer, "Obtained parameter base1=%x Size1=%x\n",sParams.block0_start,
                sParams.block0_len);
        sysDebugWriteString (buffer);

        sprintf(buffer, "Obtained parameter base2=%x Size2=%x\n",sParams.block1_start,
                sParams.block1_len);
        sysDebugWriteString (buffer);
	#endif

	/* Start Testing memory */

	//sysCacheDisable();
        cache_off();

	wTestStatus = wDramDualAddressBlock (&sParams);

	//sysCacheEnable();
	cache_on();

	/* Report errors */
	if (wTestStatus != E__OK)
	{
#ifdef DEBUG
                sysDebugWriteString ("Error running dual address test\n");
#endif
	}
	else
	{
#ifdef DEBUG
                sysDebugWriteString ("Passed dual address test\n");
#endif
	}
	return (wTestStatus);
}
#endif
