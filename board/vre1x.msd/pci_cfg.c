
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/pci_cfg.c,v 1.2 2014-03-28 17:25:34 cdobson Exp $
 *
 * $Log: pci_cfg.c,v $
 * Revision 1.2  2014-03-28 17:25:34  cdobson
 * Added board service to get PCI device interrupt information.
 *
 * Revision 1.1  2014-03-04 10:25:18  mgostling
 * First version for VRE1x
 *
 * Revision 1.2  2013-12-10 12:10:58  mgostling
 * Removed redundant code.
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
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
 */

/* includes */

#include <stdtypes.h>

#include <bit/pci.h>
 
#include <bit/board_service.h>
#include <private/port_io.h>
#include <private/debug.h>


/* defines */

#define EDGE_LEVEL_0		0x04D0		/* IRQ edge (0) or level (1) */
#define EDGE_LEVEL_1		0x04D1


/* typedefs */
typedef struct PciDeviceInt {
	UINT8 bBus, bDev, bFunc;
	UINT8 bIntA, bIntB, bIntC, bIntD;
} PCI_DEV_INT;

typedef struct PciBridgeInfo {
	UINT8 bBus, bDev, bFunc;
	UINT8 bInitialBus;
} PCI_BRIDGE_INFO;

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


/* Bridge table */
/* Bus, Dev, Func, InitialBus */
static PCI_BRIDGE_INFO BridgeTable[] = {
		{0, 28, 4, 1},		/* root bridge for PMC/XMC 2 */
		{1,  0, 0, 2},		/* X130 PMC Bridge */
};

/* Interruptible device tables */
/* Fixed Onboard devices */
/* Bus, Dev, Func, IntA, IntB, IntC, IntD */
/* Bus numbers in this table are updated using */
/* the bridge table above. Bus numbers should  */
/* match the InitialBus numbers above.         */ 
static PCI_DEV_INT OnBoardDevs[] = {
		{2, 5, 0, 17, 18, 19, 16}, /* PMC 2 */
		{2, 8, 0, 16, 17, 18, 19}  /* PMC 1 */
};

/* Expansion bus devices */
/* This table is not currently used. */
/* Needed for future                 */ 
//static PCI_DEV_INT ExpansionDevs[] = {
//		{0, 1, 2, 20, 21, 22, 23}, /* CPCI bus  */
//};



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
	sysDebugWriteString ("brdPciGetParams.\n");
	sPciParams.dHbMemAlignment	= 0x100000;		/* 1MB */
	sPciParams.wHbIoAlignment	= 8;			/* 8 bytes */

	sPciParams.dPciMemBase		= 0xDFFFFFFF;
	sPciParams.wPciIoBase		= 0xFFFF;

	sPciParams.dPciMemLimit		= sPciParams.dPciMemBase;
	sPciParams.wPciIoLimit		= sPciParams.wPciIoBase;

	sPciParams.bHostBridges		= 7;	// two in Haswell and five in LynxPoint

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


/****************************************************************************
 * GetSecBus: Read bridge secondary bus register
 * 
 * RETURNS: secondary bus number
 */
static UINT8 GetSecBus (PCI_PFA Pfa)
{
	return PCI_READ_BYTE(Pfa, PCI_SEC_BUS);
} /* GetSecBus () */


/****************************************************************************
 * UpdateBridgeTable: Update bus numbers in the above bridge table
 * 
 * RETURNS: none
 */
static void UpdateBridgeTable (void)
{
	int i, j;

	for (i = 0; i < LENGTH(BridgeTable); i++) {
	    // If not bus zero then lookup previous entries for matching initial bus.
	    if (BridgeTable[i].bBus != 0) {
	    	for (j = 0; j < i; j++) {
		        if (BridgeTable[i].bBus == BridgeTable[j].bInitialBus) {
		          BridgeTable[i].bBus = GetSecBus (PCI_MAKE_PFA(BridgeTable[j].bBus,
		        		  BridgeTable[j].bDev, BridgeTable[j].bFunc));
		          break;
		        }
	    	}
	    }
	}
} /* UpdateBridgeTable () */


/****************************************************************************
 * UpdatePciInterruptTable: Update bus numbers in PCI interrupt tables
 * 
 * RETURNS: none
 */
static void UpdatePciInterruptTable (void)
{
	int i, j;

	UpdateBridgeTable();

	for (i = 0; i < LENGTH(OnBoardDevs); i++) {
	    if (OnBoardDevs[i].bBus != 0) {
	    	for (j = 0; j < LENGTH (BridgeTable); j++) {
	    		if (OnBoardDevs[i].bBus == BridgeTable[j].bInitialBus) {
	    			OnBoardDevs[i].bBus = GetSecBus (PCI_MAKE_PFA(BridgeTable[j].bBus,
	    					BridgeTable[j].bDev, BridgeTable[j].bFunc));
	    			break;
	    		}
	    	}
	    }
	}

//	for (i = 0; i < LENGTH(ExpansionDevs); i++) {
//	    if (ExpansionDevs[i].bBus != 0) {
//	    	for (j = 0; j < LENGTH (BridgeTable); j++) {
//	    		if (ExpansionDevs[i].bBus == BridgeTable[j].bInitialBus) {
//	    			ExpansionDevs[i].bBus = GetSecBus (PCI_MAKE_PFA(BridgeTable[j].bBus,
//	    					BridgeTable[j].bDev, BridgeTable[j].bFunc));
//	    			break;
//	    		}
//	    	}
//	    }
//	}
} /* UpdatePciInterruptTable () */


/****************************************************************************
 * GetIrq: Get interrupt for given PCI device PFA
 * 
 * RETURNS: Interrupt number
 */
static UINT8 GetIrq (PCI_PFA Pfa, PCI_DEV_INT *DevIntInfo)
{
	/* get int pin from device and work out the interrupt */
	UINT8 IntPin;
	UINT8 Irq = 0;

	IntPin = PCI_READ_BYTE(Pfa, PCI_INT_PIN);
	if (IntPin != 0) {
		switch (IntPin) {
		case 1:
			Irq = DevIntInfo->bIntA;
			break;
		case 2:
			Irq = DevIntInfo->bIntB;
			break;
		case 3:
			Irq = DevIntInfo->bIntC;
			break;
		case 4:
			Irq = DevIntInfo->bIntD;
			break;
		}
	}

	return Irq;
}


/****************************************************************************
 * brdGetPciInterruptInfo: Get interrupt info for specific PFA
 *
 * RETURNS: none
 */

UINT32 brdGetPciInterruptInfo (void *ptr)
{
	PCI_INTERRUPT_INFO *IntInfo = *((PCI_INTERRUPT_INFO **)ptr);
	static int TableUpdated = 0;
	int i;

	/* Update interrupt tables if required */
	
	if (!TableUpdated) {
		UpdatePciInterruptTable();
		TableUpdated++;
	}

	/* scan the tables for a matching PFA */
	
	for (i = 0; i < LENGTH(OnBoardDevs); i++) {
		if (PCI_MAKE_PFA(OnBoardDevs[i].bBus,
				OnBoardDevs[i].bDev, OnBoardDevs[i].bFunc) == IntInfo->Pfa) {
			IntInfo->IrqNo = GetIrq(IntInfo->Pfa, &OnBoardDevs[i]);
			IntInfo->ApicNo = 0;
			return E__OK;
		}
	}

	return E__BIT;
}
