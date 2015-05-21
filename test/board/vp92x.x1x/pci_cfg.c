
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

/* board_cfg.c - functions reporting board specific architecture
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/pci_cfg.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: pci_cfg.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
 * Import files into new source repository.
 *
 * Revision 1.2  2012/08/24 10:15:09  madhukn
 * BIT_VER Tool modified to generate new format [XorV][Version].[Revision]-[subversion]
 *
 * De-emphasis bit set for B:0 D:1 F:1 for XMC2 detection
 *
 * Test added to Board Temperature through Super IO
 *
 * Revision 1.1  2012/06/22 11:42:00  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */


/* includes */

#include <stdtypes.h>

#include <bit/pci.h>
#include <bit/io.h>
#include <bit/board_service.h>
#include <private/port_io.h>
#include <private/debug.h>


/* defines */

#define EDGE_LEVEL_0		0x04D0		/* IRQ edge (0) or level (1) */
#define EDGE_LEVEL_1		0x04D1


/* typedefs */

/* constants */

/* locals */

static PCI_PARAMS	sPciParams;

	/*
	 * Board-specific routing of PIRQ interrupts to IRQs
	 */

static const UINT8	abIrqMap[] =
{
	8,		/* number or PIRQs to route */

	10,		/* PIRQA */
	9,		/* PIRQB */
	10,		/* PIRQC */
	9,		/* PIRQD */

	10,		/* PIRQE */
	9,		/* PIRQF */
	10,		/* PIRQG */
	9		/* PIRQH */
};


/* globals */

/* externals */

/* forward declarations */



/*****************************************************************************
 * brdPciGetParams: report board-specific PCI data
 *
 * RETURNS: pointer to PCI data
 */

UINT32 brdPciGetParams (void *ptr)
{
	PCI_PFA pfa;
	UINT8 reg;
	UINT8 ResetFlags;
	//Check XMC Site 2
	reg = ((UINT8)dIoReadReg(0x21D, REG_8));
	reg = 0;
	if ((reg & 0x04) != 0) {
		// we have a XMC fitted so check link status of D01:F01
		pfa = PCI_MAKE_PFA(0,1,1);	
		if ((PCI_READ_BYTE(pfa, 0xB2) & 0xF0)== 0)
		{			
			// if we are already at x1 then do nothing
			if ((PCIE_READ_BYTE(pfa, 0x224)) != 0x01)
			{
				// no link and not x1 so set for a x1 and reset
				PCIE_WRITE_BYTE(pfa, 0x224, 1);
				ResetFlags = 6;
				sysOutPort8 (0x0CF9, ResetFlags);
			}
		}
	}

	sysDebugWriteString ("brdPciGetParams.\n");
	sPciParams.dHbMemAlignment	= 0x100000;		/* 1MB */
	sPciParams.wHbIoAlignment	= 8;			/* 8 bytes */

	sPciParams.dPciMemBase		= 0xDFFFFFFF;
	sPciParams.wPciIoBase		= 0xFFFF;

	sPciParams.dPciMemLimit		= sPciParams.dPciMemBase;
	sPciParams.wPciIoLimit		= sPciParams.wPciIoBase;

	sPciParams.bHostBridges		= 1;

	sPciParams.bCsLatency		= 32;
	sPciParams.bCsCacheLine		= 16;

	*((PCI_PARAMS**)ptr)        = &sPciParams;
	sysDebugWriteString ("brdPciGetParams end.\n");
	return E__OK;

} /* brdPciGetParams () */


/****************************************************************************
 * brdPciCfgHbStart: board-specific host bridge configuration (start of bus scan)
 * 
 * RETURNS: none
 */
UINT32 brdPciCfgHbStart(void *ptr)
{
	/* Only one host bridge: 0 is the 945GM */
	*(((PCI_HUB_START_PARAMS*)ptr)->pdMemAddr) &= ~(sPciParams.dHbMemAlignment - 1);
	*(((PCI_HUB_START_PARAMS*)ptr)->pwIoAddr)  &= ~(sPciParams.wHbIoAlignment - 1);

	return E__OK;

} /* brdPciCfgHbStart () */


/****************************************************************************
 * brdPciCfgHbEnd: board-specific host bridge configuration (end of bus scan)
 * 
 * RETURNS: none
 */
UINT32 brdPciCfgHbEnd( void *ptr )
{
#if 0
	PCI_PFA	pfa;
	UINT32	dMemAddr;
	UINT32	dMemSize;
	UINT8	bTemp;
#endif

	/* Only one host bridge: 0 is the 945GM */
	*(((PCI_HUB_END_PARAMS*)ptr)->pdMemAddr) -=  (sPciParams.dHbMemAlignment - 1);
	*(((PCI_HUB_END_PARAMS*)ptr)->pdMemAddr) &= ~(sPciParams.dHbMemAlignment - 1);

	*(((PCI_HUB_END_PARAMS*)ptr)->pwIoAddr)  -=  (sPciParams.wHbIoAlignment - 1);
	*(((PCI_HUB_END_PARAMS*)ptr)->pwIoAddr)  &= ~(sPciParams.wHbIoAlignment - 1);

#if 0
	pfa = PCI_MAKE_PFA (0, 0, 0);		/* 945GM, DRC */

	/* Need to set the TOLM register otherwise we only have 2G bytes of RAM */

	/* Get total mem fitted */
	dMemSize = bTemp;
	dMemSize *= 0x400;
				
	if ((bTemp == 0x0c) || (bTemp == 0x0d) || (bTemp == 0x0f))
		dMemSize *= 2;          /* dual channel mode */


	dMemAddr &= 0xf8000000;	/* 128 MB alignment */
	dMemAddr = dMemAddr >> 16;

	if (dMemSize > dMemAddr)
		dMemSize = dMemAddr;
#endif

	return E__OK;

} /* brdPciCfgHbEnd () */


/****************************************************************************
 * brdPciRoutePirqToIrq: connect PIRQs to IRQs according to the board's table
 * 
 * RETURNS: none
 */

UINT32 brdPciRoutePirqToIrq (void *ptr)
{
	PCI_PFA	pfa;
	UINT8	irq;
	UINT8	pirq;
	UINT8	bTemp;

	(void)ptr;

	pfa = PCI_MAKE_PFA (0, 31, 0);		/* ICH7, LPC bridge */

	/* For each PIRQ used by the board, route to the specified IRQ */

	for (pirq = 0; pirq < abIrqMap[0]; pirq++)
	{
		/* Connect the PIRQ to an IRQ */
	
		irq = abIrqMap[pirq + 1];

		if (pirq < 4)				/* PIRQA-D */
			PCI_WRITE_BYTE (pfa, 0x60 + pirq, irq);

		else if ( (pirq >= 4) && (pirq < 8) )			/* PIRQE-H */
			PCI_WRITE_BYTE (pfa, 0x68 + pirq-4, irq);

		/* NOTE: PXIRQs go via APIC1 to IRQ9 */

		/* Set the IRQ to level sensitive operation */
		
		if ((irq >= 1) && (irq <= 7))
		{
			bTemp = sysInPort8 (EDGE_LEVEL_0);
			sysOutPort8 (EDGE_LEVEL_0, bTemp | (1 << irq));
		}

		else if ((irq >= 8) && (irq <= 15))
		{
			irq -= 8;
			bTemp = sysInPort8 (EDGE_LEVEL_1);
			sysOutPort8 (EDGE_LEVEL_1, bTemp | (1 << irq));
		}
	}

	return E__OK;

} /* brdPciRoutePirqToIrq () */
