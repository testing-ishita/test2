
/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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


/* includes */

#include <stdtypes.h>

#include <bit/pci.h>

#include <bit/board_service.h>
#include <private/port_io.h>


/* defines */

#define EDGE_LEVEL_0		0x04D0		/* IRQ edge (0) or level (1) */
#define EDGE_LEVEL_1		0x04D1

//#define DEBUG
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

	/* Only one host bridge: 0 is the 945GM */
	*(((PCI_HUB_END_PARAMS*)ptr)->pdMemAddr) -=  (sPciParams.dHbMemAlignment - 1);
	*(((PCI_HUB_END_PARAMS*)ptr)->pdMemAddr) &= ~(sPciParams.dHbMemAlignment - 1);

	*(((PCI_HUB_END_PARAMS*)ptr)->pwIoAddr)  -=  (sPciParams.wHbIoAlignment - 1);
	*(((PCI_HUB_END_PARAMS*)ptr)->pwIoAddr)  &= ~(sPciParams.wHbIoAlignment - 1);

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
static UINT8 idtInstances;

UINT32 brdSkipIOResourceAllocation (void *ptr) //PCI_PFA pfa,UINT8 *skipStatus)
{
	UINT32 id;
	UINT8 i,onboard = 0;
	PCI_PFA pfa = ((PCI_CFG_SKIP_IO_ALLOC_PARAMS*) ptr)->pfa;

	((PCI_CFG_SKIP_IO_ALLOC_PARAMS*) ptr)->skipStatus = 0;

#ifdef DEBUG
	char buffer[100];
#endif

	UINT16 pciDevId[19] = {0x0104,0x0101,0x0105,0x0103,0x1502,0x1C2D,0x1C10,0x1C18,
						 0x1C26,0x2448,0x1C4F,0x1C01,0x1C22,0x1C09,0x808F,0xE110,
						 0x1510,0x1E09,0x1E01};

	id = PCI_READ_DWORD (pfa,0x0);

#ifdef DEBUG
	sprintf(buffer,"PFA : 0x%x Device ID : 0x%x\n",pfa,id);
	sysDebugWriteString(buffer);
#endif


	if(id == 0x808f111d){
		idtInstances = idtInstances + 1;
#ifdef  DEBUG
		sprintf(buffer,"IDT Instances : %d\n",idtInstances);
		sysDebugWriteString(buffer);
#endif
	}

	if((idtInstances > 3) && (id == 0x808f111d)){
		((PCI_CFG_SKIP_IO_ALLOC_PARAMS*) ptr)->skipStatus = 1;
#ifdef DEBUG
		 sprintf(buffer,"Skip Status changed caz found more IDT devices than onboard\n");
		 sysDebugWriteString(buffer);
#endif
		return E__OK;

	}

	for(i=0;i<19;i++)
	{
		if(((id >> 16) & 0xFFFF) == pciDevId[i]){
			onboard = 1;
			break;
		}
	}
#ifdef DEBUG
	if(onboard == 1){
		sprintf(buffer,"Onboard device found at index : %d\n",i);
		sysDebugWriteString(buffer);
	}
#endif

	if((idtInstances > 1) && (onboard == 0)){
		((PCI_CFG_SKIP_IO_ALLOC_PARAMS*) ptr)->skipStatus = 1	;

#ifdef DEBUG
		 sprintf(buffer,"Skip Status changed caz found some bp device : 0x%x\n",id);
		 sysDebugWriteString(buffer);
#endif
	}

	return E__OK;

}
