/*
 * RapidIO mport driver for Tsi721 PCIExpress-to-SRIO bridge
 *
 * Copyright 2011 Integrated Device Technology, Inc.
 * Alexandre Bounine <alexandre.bounine@idt.com>
 * Chul Kim <chul.kim@idt.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
 
#include <stdio.h>
#include <string.h>
#include <alloc.h>

#include <stdtypes.h>
#include <errors.h>
#include <bitops.h>
#include <bit/delay.h>
#include <private/debug.h>
#include <private/cpu.h>
#include <private/k_alloc.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/console.h>
#include <bit/board_service.h>
#include <srio/rioerr.h>
#include <srio/rio_std.h>
#include <srio/rio_drv.h>

#include <srio/tsi721.h>
#include <devices/ipmi.h>

extern int riohdid0;
extern int riohdid1;


#define ONE_MEG   (1024*1024)

#define TSI721_PCIE_CAP_REG                   0x40
#define CC_SPS_CMD_NOT_RCVD		              0xD6
#define CC_OK                                 0x00

#undef DEBUG_PW	/* Inbound Port-Write debugging */
// #define DEBUG_SRIO

#undef SRIO_SPEED_SET	// define to enable speed set code

#define DRV_DESC        "Tsi721"

static UINT8 amcSlotId = 0;
unsigned char globalSRIOSpeed = 0;

static void tsi721_set_srio_boot_cmplbit(struct tsi721_device *priv);
static void tsi721_errata_workaround_1(struct tsi721_device *priv);
static void tsi721_errata_workaround_2(struct tsi721_device *priv);
static void tsi721_set_srio_linkrate(struct tsi721_device *priv,UINT8 bSrioBrate);
//static void tsi721_fundamental_reset(struct tsi721_device *priv);

static void tsi721_set_srio_boot_cmplbit(struct tsi721_device *priv)
{
   volatile UINT32 dData = 0;

    dData  =  vReadDWord(priv->regs+TSI721_DEVCTL);
   /* WorkAround: Set the SRIO_BOOT_CMPL bit. */
    vWriteDWord(priv->regs+TSI721_DEVCTL,dData | 4);	
}

#if 0
static void tsi721_fundamental_reset(struct tsi721_device *priv)
{
   volatile UINT32 dData = 0;

    dData  =  vReadDWord(priv->regs+TSI721_DEVCTL);
   /* Reset TSI 721 */
    vWriteDWord(priv->regs+TSI721_DEVCTL,dData | 0x01);
}
#endif


static void tsi721_errata_workaround_1(struct tsi721_device *priv)
{
   // WorkAround: PCIe MSIXTBL and MSIXPBA incorrect default values
   vWriteDWord(priv->regs+TSI721_PCIECFG_EPCTL,vReadDWord(priv->regs+TSI721_PCIECFG_EPCTL) | 0x01);
   vWriteDWord(priv->regs+TSI721_PCIECFG_MSIXTBL,0x0002C000);
   vWriteDWord(priv->regs+TSI721_PCIECFG_MSIXPBA,0x0002A000);	
   vWriteDWord(priv->regs+TSI721_PCIECFG_EPCTL,	vReadDWord(priv->regs+TSI721_PCIECFG_EPCTL) & ~(0x01));
}


static void tsi721_errata_workaround_2(struct tsi721_device *priv)
{
    vWriteDWord(priv->regs+TSI721_UNDOCUMENTED_REG_1,0x0000006F);
    vWriteDWord(priv->regs+TSI721_UNDOCUMENTED_REG_2,0x0000006F);
    vWriteDWord(priv->regs+TSI721_UNDOCUMENTED_REG_3,0x0000006F);
    vWriteDWord(priv->regs+TSI721_UNDOCUMENTED_REG_4,0x0000006F);
    vWriteDWord(priv->regs+TSI721_UNDOCUMENTED_REG_5,0x0000006F);
    vWriteDWord(priv->regs+TSI721_UNDOCUMENTED_REG_6,0x00000000);
    vWriteDWord(priv->regs+TSI721_UNDOCUMENTED_REG_7,0x00000000);
    vWriteDWord(priv->regs+TSI721_UNDOCUMENTED_REG_8,0x00000000);
}


static void tsi721_set_srio_linkrate(struct tsi721_device *priv,UINT8 bSrioBrate)
{
	volatile UINT32 dData = 0;
#ifdef DEBUG_SRIO	
	char achBuffer[80];
#endif
	
	/* Perform a device-wide hot reset */
	vWriteDWord((priv->regs+TSI721_DEVCTL),vReadDWord(priv->regs+TSI721_DEVCTL) &(0xFFF0FFFF));
	/* Perform a SRIO reset MAC core logic reset */
	vWriteDWord((priv->regs+TSI721_DEVCTL),vReadDWord(priv->regs+TSI721_DEVCTL)|(0x10000));
	//vDelay(5);
	
	dData = vReadDWord(priv->regs+RIO_SP_CTL2);
	
#ifdef DEBUG_SRIO
	sprintf (achBuffer,"bSrioBrate: 0x%x RIO_SP_CTL2: 0x%08x\n", bSrioBrate, dData);
	sysDebugWriteString(achBuffer);
#endif	

	dData &= 0xFEAAFFFF;
	dData |= (0x04000000 >> (2 * bSrioBrate));
	
	/* Set new rate */
	vWriteDWord(priv->regs+RIO_SP_CTL2,dData);

	//vDelay(5); // allow time for link retrain
	
#ifdef DEBUG_SRIO
	sprintf (achBuffer,"RIO_SP_CTL2: 0x%08x (0x%08x)\n", vReadDWord(priv->regs+RIO_SP_CTL2), dData);
	sysDebugWriteString(achBuffer);
#endif
}


/*static void tsi721_omsg_handler(struct tsi721_device *priv, int ch);
static void tsi721_imsg_handler(struct tsi721_device *priv, int ch);*/

/**
 * tsi721_lcread - read from local SREP config space
 * @mport: RapidIO master port info
 * @index: ID of RapdiIO interface
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Value to be read into
 *
 * Generates a local SREP space read. Returns %0 on
 * success or %-EINVAL on failure.
 */
static int tsi721_lcread(struct rio_mport *mport, int index, UINT32 offset,
			 int len, UINT32 *data)
{
	struct tsi721_device *priv = mport->priv;

	if (len != sizeof(UINT32))
		return -EINVAL; /* only 32-bit access is supported */

	*data = vReadDWord(priv->regs + offset);

	return 0;
}

/**
 * tsi721_lcwrite - write into local SREP config space
 * @mport: RapidIO master port info
 * @index: ID of RapdiIO interface
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Value to be written
 *
 * Generates a local write into SREP configuration space. Returns %0 on
 * success or %-EINVAL on failure.
 */
static int tsi721_lcwrite(struct rio_mport *mport, int index, UINT32 offset,
			  int len, UINT32 data)
{
	struct tsi721_device *priv = mport->priv;

	if (len != sizeof(UINT32))
		return -EINVAL; /* only 32-bit access is supported */

	vWriteDWord(priv->regs + offset,data);

	return 0;
}

/**
 * tsi721_maint_dma - Helper function to generate RapidIO maintenance
 *                    transactions using designated Tsi721 DMA channel.
 * @priv: pointer to tsi721 private data
 * @sys_size: RapdiIO transport system size
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @data: Location to be read from or write into
 * @do_wr: Operation flag (1 == MAINT_WR)
 *
 * Generates a RapidIO maintenance transaction (Read or Write).
 * Returns %0 on success and %-EINVAL or %-EFAULT on failure.
 */
static int tsi721_maint_dma(struct tsi721_device *priv, UINT32 sys_size,
			UINT16 destid, UINT8 hopcount, UINT32 offset, int len,
			UINT32 *data, int do_wr)
{
	volatile void  *regs = priv->regs + TSI721_DMAC_BASE(priv->mdma.ch_id);
	struct tsi721_dma_desc *bd_ptr;
	UINT32 rd_count=0, swr_ptr=0, ch_stat=0;
	int i=0, err = 0;
	UINT32 op = do_wr ? MAINT_WR : MAINT_RD;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	UINT32 dummy=0;
#pragma GCC diagnostic pop

#ifdef DEBUG	
	dummy = vReadDWord(regs);
	sysDebugFlush();

	if (op == MAINT_WR)
	{
	    sysDebugWriteString("MAINT_WR\n");
	}
	else
	{
		sysDebugWriteString("MAINT_RD\n");
	}

	sprintf (achBuffer,"DMA BASE:%x\n",regs);
	sysDebugWriteString(achBuffer);
#endif	
	if (offset > (RIO_MAINT_SPACE_SZ - len) || (len != sizeof(UINT32)))
		return -EINVAL;

	bd_ptr = priv->mdma.bd_base;

	rd_count = vReadDWord(regs + TSI721_DMAC_DRDCNT);

	/* Initialize DMA descriptor */
	bd_ptr[0].type_id = cpu_to_le32((DTYPE2 << 29) | (op << 19) | destid);
	bd_ptr[0].bcount = cpu_to_le32((sys_size << 26) | 0x04);
	bd_ptr[0].raddr_lo = cpu_to_le32((hopcount << 24) | offset);
	bd_ptr[0].raddr_hi = 0;
	
	if (do_wr)
	{
		bd_ptr[0].data[0] = cpu_to_be32p(*data);			
	}
	else
		bd_ptr[0].data[0] = 0xffffffff;

	mb(); 
	
	/* Start DMA operation */
	vWriteDWord(regs + TSI721_DMAC_DWRCNT,(rd_count + 2));
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	dummy = vReadDWord(regs + TSI721_DMAC_DWRCNT);
#pragma GCC diagnostic pop

	i = 0;

	/* Wait until DMA transfer is finished */
	while ((ch_stat = vReadDWord(regs + TSI721_DMAC_STS))
							& TSI721_DMAC_STS_RUN) 
   {
		vDelay(1);
		if (++i >= 5000000) 
		{
#ifdef DEBUG_SRIO
			sprintf(achBuffer,"tsi721_maint_dma() : DMA[%d] read timeout ch_status=%x\n",
				 priv->mdma.ch_id, ch_stat);
			sysDebugWriteString(achBuffer);	
#endif			
			if (!do_wr)
				*data = 0xffffffff;
			err = -EIO;
			goto err_out;
		}
	}

     
	if (ch_stat & TSI721_DMAC_STS_ABORT) 
	{
		/* If DMA operation aborted due to error,
		 * reinitialize DMA channel
		 */
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"%s : DMA ABORT ch_stat=%x\n",__func__, ch_stat);
		sysDebugWriteString(achBuffer);
		sprintf(achBuffer,"ERR: OP=%d : destid=%x hc=%x off=%x\n",	do_wr ? MAINT_WR : MAINT_RD, destid, hopcount, offset);
		sysDebugWriteString(achBuffer);
#endif		
		vWriteDWord(regs + TSI721_DMAC_CTL,TSI721_DMAC_CTL_INIT);
		vWriteDWord(regs + TSI721_DMAC_INT,TSI721_DMAC_INT_ALL);
		vDelay(10);
		vWriteDWord(regs + TSI721_DMAC_DWRCNT,0);
		vDelay(1);
		if (!do_wr)
			*data = 0xffffffff;
		err = -EIO;
		goto err_out;
	}
#if 0	
	else
	{
		sprintf(achBuffer,"SUCCESS:OP=%d : destid=%x hc=%x off=%x\n",	do_wr ? MAINT_WR : MAINT_RD, destid, hopcount, offset);
		sysDebugWriteString(achBuffer);
	}
#endif

	if (!do_wr)
		*data = be32_to_cpu(bd_ptr[0].data[0]);

	/*
	 * Update descriptor status FIFO RD pointer.
	 * NOTE: Skipping check and clear FIFO entries because we are waiting
	 * for transfer to be completed.
	 */
	swr_ptr = vReadDWord(regs + TSI721_DMAC_DSWP);
	vWriteDWord(regs + TSI721_DMAC_DSRP,swr_ptr);
err_out:

	return err;
}

/**
 * tsi721_cread_dma - Generate a RapidIO maintenance read transaction
 *                    using Tsi721 BDMA engine.
 * @mport: RapidIO master port control structure
 * @index: ID of RapdiIO interface
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @val: Location to be read into
 *
 * Generates a RapidIO maintenance read transaction.
 * Returns %0 on success and %-EINVAL or %-EFAULT on failure.
 */
static int tsi721_cread_dma(struct rio_mport *mport, int index, UINT16 destid,
			UINT8 hopcount, UINT32 offset, int len, UINT32 *data)
{
	UINT32 retry = 2;
	int ret_val;
	struct tsi721_device *priv = mport->priv;
	
#if 0
	return tsi721_maint_dma(priv, mport->sys_size, destid, hopcount,
				offset, len, data, 0);
#endif
	 ret_val = tsi721_maint_dma(priv, mport->sys_size, destid, hopcount,
					offset, len, data, 0);

	 while((ret_val != 0) && (retry>0))
	 {
		 ret_val = tsi721_maint_dma(priv, mport->sys_size, destid, hopcount,
		 					offset, len, data, 0);
		 retry--;
	 }

	 return ret_val;
}

/**
 * tsi721_cwrite_dma - Generate a RapidIO maintenance write transaction
 *                     using Tsi721 BDMA engine
 * @mport: RapidIO master port control structure
 * @index: ID of RapdiIO interface
 * @destid: Destination ID of transaction
 * @hopcount: Number of hops to target device
 * @offset: Offset into configuration space
 * @len: Length (in bytes) of the maintenance transaction
 * @val: Value to be written
 *
 * Generates a RapidIO maintenance write transaction.
 * Returns %0 on success and %-EINVAL or %-EFAULT on failure.
 */
static int tsi721_cwrite_dma(struct rio_mport *mport, int index, UINT16 destid,
			 UINT8 hopcount, UINT32 offset, int len, UINT32 data)
{
	struct tsi721_device *priv = mport->priv;
	UINT32 temp = data,retry=2;
	int ret_val=-1;

#if 0
	return tsi721_maint_dma(priv, mport->sys_size, destid, hopcount,
				offset, len, &temp, 1);
#endif
	ret_val = tsi721_maint_dma(priv, mport->sys_size, destid, hopcount,
					offset, len, &temp, 1);
	 while((ret_val != 0) && (retry>0))
	{
		 ret_val = tsi721_maint_dma(priv, mport->sys_size, destid, hopcount,
		 					offset, len, &temp, 1);
		 retry--;
	}

	 return ret_val;
}

/**
 * tsi721_pw_enable - enable/disable port-write interface init
 * @mport: Master port implementing the port write unit
 * @enable:    1=enable; 0=disable port-write message handling
 */
#if 0 //Date: 03/03/2014
static int tsi721_pw_enable(struct rio_mport *mport, int enable)
{
	struct tsi721_device *priv = mport->priv;
	UINT32 rval;

	rval = vReadDWord(priv->regs + TSI721_RIO_EM_INT_ENABLE);

	if (enable)
		rval |= TSI721_RIO_EM_INT_ENABLE_PW_RX;
	else
		rval &= ~TSI721_RIO_EM_INT_ENABLE_PW_RX;

	/* Clear pending PW interrupts */
	vWriteDWord(priv->regs + TSI721_RIO_PW_RX_STAT,TSI721_RIO_PW_RX_STAT_PW_DISC | TSI721_RIO_PW_RX_STAT_PW_VAL);
	/* Update enable bits */
	vWriteDWord(priv->regs + TSI721_RIO_EM_INT_ENABLE,rval);

	return 0;
}


static void tsi721_interrupts_init(struct tsi721_device *priv)
{
	UINT32 intr;
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	UINT32 dummy;
#pragma GCC diagnostic pop
	
	/* Enable IDB interrupts */
	vWriteDWord(priv->regs + TSI721_SR_CHINT(IDB_QUEUE),TSI721_SR_CHINT_ALL);
	vWriteDWord(priv->regs + TSI721_SR_CHINTE(IDB_QUEUE),TSI721_SR_CHINT_IDBQRCV);

	/* Enable SRIO MAC interrupts */
	vWriteDWord(priv->regs + TSI721_RIO_EM_DEV_INT_EN,TSI721_RIO_EM_DEV_INT_EN_INT);

	/* Enable interrupts from channels in use */
#ifdef CONFIG_RAPIDIO_DMA_ENGINE
	intr = TSI721_INT_SR2PC_CHAN(IDB_QUEUE) |
		(TSI721_INT_BDMA_CHAN_M &
		 ~TSI721_INT_BDMA_CHAN(TSI721_DMACH_MAINT));
#else
	intr = TSI721_INT_SR2PC_CHAN(IDB_QUEUE);
#endif
	vWriteDWord(priv->regs + TSI721_DEV_CHAN_INTE,intr);

	if (priv->flags & TSI721_USING_MSIX)
		intr = TSI721_DEV_INT_SRIO;
	else
		intr = TSI721_DEV_INT_SR2PC_CH | TSI721_DEV_INT_SRIO |
			TSI721_DEV_INT_SMSG_CH | TSI721_DEV_INT_BDMA_CH;

	vWriteDWord(priv->regs + TSI721_DEV_INTE,intr);
	dummy = vReadDWord(priv->regs + TSI721_DEV_INTE);
}
#endif

#ifdef CONFIG_PCI_MSI
/**
 * tsi721_omsg_msix - MSI-X interrupt handler for outbound messaging
 * @irq: Linux interrupt number
 * @ptr: Pointer to interrupt-specific data (mport structure)
 *
 * Handles outbound messaging interrupts signaled using MSI-X.
 */
static irqreturn_t tsi721_omsg_msix(int irq, void *ptr)
{
	struct tsi721_device *priv = ((struct rio_mport *)ptr)->priv;
	int mbox;

	mbox = (irq - priv->msix[TSI721_VECT_OMB0_DONE].vector) % RIO_MAX_MBOX;
	tsi721_omsg_handler(priv, mbox);
	return IRQ_HANDLED;
}

/**
 * tsi721_imsg_msix - MSI-X interrupt handler for inbound messaging
 * @irq: Linux interrupt number
 * @ptr: Pointer to interrupt-specific data (mport structure)
 *
 * Handles inbound messaging interrupts signaled using MSI-X.
 */
static irqreturn_t tsi721_imsg_msix(int irq, void *ptr)
{
	struct tsi721_device *priv = ((struct rio_mport *)ptr)->priv;
	int mbox;

	mbox = (irq - priv->msix[TSI721_VECT_IMB0_RCV].vector) % RIO_MAX_MBOX;
	tsi721_imsg_handler(priv, mbox + 4);
	return IRQ_HANDLED;
}

/**
 * tsi721_srio_msix - Tsi721 MSI-X SRIO MAC interrupt handler
 * @irq: Linux interrupt number
 * @ptr: Pointer to interrupt-specific data (mport structure)
 *
 * Handles Tsi721 interrupts from SRIO MAC.
 */
static irqreturn_t tsi721_srio_msix(int irq, void *ptr)
{
	struct tsi721_device *priv = ((struct rio_mport *)ptr)->priv;
	UINT32 srio_int;

	/* Service SRIO MAC interrupts */
	srio_int = vReadDWord(priv->regs + TSI721_RIO_EM_INT_STAT);
	if (srio_int & TSI721_RIO_EM_INT_STAT_PW_RX)
		tsi721_pw_handler((struct rio_mport *)ptr);

	return IRQ_HANDLED;
}

/**
 * tsi721_sr2pc_ch_msix - Tsi721 MSI-X SR2PC Channel interrupt handler
 * @irq: Linux interrupt number
 * @ptr: Pointer to interrupt-specific data (mport structure)
 *
 * Handles Tsi721 interrupts from SR2PC Channel.
 * NOTE: At this moment services only one SR2PC channel associated with inbound
 * doorbells.
 */
static irqreturn_t tsi721_sr2pc_ch_msix(int irq, void *ptr)
{
	struct tsi721_device *priv = ((struct rio_mport *)ptr)->priv;
	UINT32 sr_ch_int;

	/* Service Inbound DB interrupt from SR2PC channel */
	sr_ch_int = vReadDWord(priv->regs + TSI721_SR_CHINT(IDB_QUEUE));
	if (sr_ch_int & TSI721_SR_CHINT_IDBQRCV)
		tsi721_dbell_handler((struct rio_mport *)ptr);

	/* Clear interrupts */
	vWriteDWord(priv->regs + TSI721_SR_CHINT(IDB_QUEUE),sr_ch_int);
	/* Read back to ensure that interrupt was cleared */
	sr_ch_int = vReadDWord(priv->regs + TSI721_SR_CHINT(IDB_QUEUE));

	return IRQ_HANDLED;
}

/**
 * tsi721_request_msix - register interrupt service for MSI-X mode.
 * @mport: RapidIO master port structure
 *
 * Registers MSI-X interrupt service routines for interrupts that are active
 * immediately after mport initialization. Messaging interrupt service routines
 * should be registered during corresponding open requests.
 */
static int tsi721_request_msix(struct rio_mport *mport)
{
	struct tsi721_device *priv = mport->priv;
	int err = 0;

	err = request_irq(priv->msix[TSI721_VECT_IDB].vector,
			tsi721_sr2pc_ch_msix, 0,
			priv->msix[TSI721_VECT_IDB].irq_name, (void *)mport);
	if (err)
		goto out;

	err = request_irq(priv->msix[TSI721_VECT_PWRX].vector,
			tsi721_srio_msix, 0,
			priv->msix[TSI721_VECT_PWRX].irq_name, (void *)mport);
	if (err)
		free_irq(
			priv->msix[TSI721_VECT_IDB].vector,
			(void *)mport);
out:
	return err;
}

/**
 * tsi721_free_msix - release interrupt service .
 * @mport: RapidIO master port structure
 *
 * Free MSI-X interrupt service routines for interrupts that are active
 * immediately after mport initialization.
 */
static void tsi721_free_msix(struct rio_mport *mport)
{
	struct tsi721_device *priv = mport->priv;

	free_irq(priv->msix[TSI721_VECT_IDB].vector,(void *)mport);
	free_irq(priv->msix[TSI721_VECT_PWRX].vector,(void *)mport);

	if (priv->flags & TSI721_USING_MSIX)
		pci_disable_msix(priv->pdev);
}

/**
 * tsi721_enable_msix - Attempts to enable MSI-X support for Tsi721.
 * @priv: pointer to tsi721 private data
 *
 * Configures MSI-X support for Tsi721. Supports only an exact number
 * of requested vectors.
 */
static int tsi721_enable_msix(struct tsi721_device *priv)
{
	struct msix_entry entries[TSI721_VECT_MAX];
	int err;
	int i;
	char achBuffer[80];

	entries[TSI721_VECT_IDB].entry = TSI721_MSIX_SR2PC_IDBQ_RCV(IDB_QUEUE);
	entries[TSI721_VECT_PWRX].entry = TSI721_MSIX_SRIO_MAC_INT;

	/*
	 * Initialize MSI-X entries for Messaging Engine:
	 * this driver supports four RIO mailboxes (inbound and outbound)
	 * NOTE: Inbound message MBOX 0...4 use IB channels 4...7. Therefore
	 * offset +4 is added to IB MBOX number.
	 */
	for (i = 0; i < RIO_MAX_MBOX; i++) {
		entries[TSI721_VECT_IMB0_RCV + i].entry =
					TSI721_MSIX_IMSG_DQ_RCV(i + 4);
		entries[TSI721_VECT_IMB0_INT + i].entry =
					TSI721_MSIX_IMSG_INT(i + 4);
		entries[TSI721_VECT_OMB0_DONE + i].entry =
					TSI721_MSIX_OMSG_DONE(i);
		entries[TSI721_VECT_OMB0_INT + i].entry =
					TSI721_MSIX_OMSG_INT(i);
	}

#ifdef CONFIG_RAPIDIO_DMA_ENGINE
	/*
	 * Initialize MSI-X entries for Block DMA Engine:
	 * this driver supports XXX DMA channels
	 * (one is reserved for SRIO maintenance transactions)
	 */
	for (i = 0; i < TSI721_DMA_CHNUM; i++) {
		entries[TSI721_VECT_DMA0_DONE + i].entry =
					TSI721_MSIX_DMACH_DONE(i);
		entries[TSI721_VECT_DMA0_INT + i].entry =
					TSI721_MSIX_DMACH_INT(i);
	}
#endif /* CONFIG_RAPIDIO_DMA_ENGINE */

	err = pci_enable_msix(priv->pdev, entries, ARRAY_SIZE(entries));
	if (err) {
		if (err > 0)
			dev_info(&priv->pdev->dev,
				 "Only %d MSI-X vectors available, "
				 "not using MSI-X\n", err);
		else
		{
#ifdef DEBUG_SRIO
			sprintf(achBuffer,"Failed to enable MSI-X (err=%d)\n", err);
			sysDebugWriteString(achBuffer);
#endif			
		}
		return err;
	}

	/*
	 * Copy MSI-X vector information into tsi721 private structure
	 */
	priv->msix[TSI721_VECT_IDB].vector = entries[TSI721_VECT_IDB].vector;
	snprintf(priv->msix[TSI721_VECT_IDB].irq_name, IRQ_DEVICE_NAME_MAX,
		 DRV_NAME "-idb@pci:%s", pci_name(priv->pdev));
	priv->msix[TSI721_VECT_PWRX].vector = entries[TSI721_VECT_PWRX].vector;
	snprintf(priv->msix[TSI721_VECT_PWRX].irq_name, IRQ_DEVICE_NAME_MAX,
		 DRV_NAME "-pwrx@pci:%s", pci_name(priv->pdev));

	for (i = 0; i < RIO_MAX_MBOX; i++) {
		priv->msix[TSI721_VECT_IMB0_RCV + i].vector =
				entries[TSI721_VECT_IMB0_RCV + i].vector;
		snprintf(priv->msix[TSI721_VECT_IMB0_RCV + i].irq_name,
			 IRQ_DEVICE_NAME_MAX, DRV_NAME "-imbr%d@pci:%s",
			 i, pci_name(priv->pdev));

		priv->msix[TSI721_VECT_IMB0_INT + i].vector =
				entries[TSI721_VECT_IMB0_INT + i].vector;
		snprintf(priv->msix[TSI721_VECT_IMB0_INT + i].irq_name,
			 IRQ_DEVICE_NAME_MAX, DRV_NAME "-imbi%d@pci:%s",
			 i, pci_name(priv->pdev));

		priv->msix[TSI721_VECT_OMB0_DONE + i].vector =
				entries[TSI721_VECT_OMB0_DONE + i].vector;
		snprintf(priv->msix[TSI721_VECT_OMB0_DONE + i].irq_name,
			 IRQ_DEVICE_NAME_MAX, DRV_NAME "-ombd%d@pci:%s",
			 i, pci_name(priv->pdev));

		priv->msix[TSI721_VECT_OMB0_INT + i].vector =
				entries[TSI721_VECT_OMB0_INT + i].vector;
		snprintf(priv->msix[TSI721_VECT_OMB0_INT + i].irq_name,
			 IRQ_DEVICE_NAME_MAX, DRV_NAME "-ombi%d@pci:%s",
			 i, pci_name(priv->pdev));
	}

#ifdef CONFIG_RAPIDIO_DMA_ENGINE
	for (i = 0; i < TSI721_DMA_CHNUM; i++) {
		priv->msix[TSI721_VECT_DMA0_DONE + i].vector =
				entries[TSI721_VECT_DMA0_DONE + i].vector;
		snprintf(priv->msix[TSI721_VECT_DMA0_DONE + i].irq_name,
			 IRQ_DEVICE_NAME_MAX, DRV_NAME "-dmad%d@pci:%s",
			 i, pci_name(priv->pdev));

		priv->msix[TSI721_VECT_DMA0_INT + i].vector =
				entries[TSI721_VECT_DMA0_INT + i].vector;
		snprintf(priv->msix[TSI721_VECT_DMA0_INT + i].irq_name,
			 IRQ_DEVICE_NAME_MAX, DRV_NAME "-dmai%d@pci:%s",
			 i, pci_name(priv->pdev));
	}
#endif /* CONFIG_RAPIDIO_DMA_ENGINE */

	return 0;
}
#endif /* CONFIG_PCI_MSI */

/**
 * tsi721_init_pc2sr_mapping - initializes outbound (PCIe->SRIO)
 * translation regions.
 * @priv: pointer to tsi721 private data
 *
 * Disables SREP translation regions.
 */
static void tsi721_init_pc2sr_mapping(struct tsi721_device *priv)
{
	int i;

	/* Disable all PC2SR translation windows */
	for (i = 0; i < TSI721_OBWIN_NUM; i++)
		vWriteDWord(priv->regs + TSI721_OBWINLB(i),0);
}

/**
 * tsi721_rio_map_inb_mem -- Mapping inbound memory region.
 * @mport: RapidIO master port
 * @lstart: Local memory space start address.
 * @rstart: RapidIO space start address.
 * @size: The mapping region size.
 * @flags: Flags for mapping. 0 for using default flags.
 *
 * Return: 0 -- Success.
 *
 * This function will create the inbound mapping
 * from rstart to lstart.
 */
static int tsi721_rio_map_inb_mem(struct rio_mport *mport, dma_addr_t lstart,
		UINT64 rstart, UINT32 size, UINT32 flags)
{
	struct tsi721_device *priv = mport->priv;
	int i;
	UINT32 regval;
#ifdef DEBUG_SRIO
	UINT32 dSize = 0;
	char achBuffer[80];
#endif

#ifdef DEBUG_SRIO
	dSize = size;
	sprintf (achBuffer,"lstart:%08x rstart:%x%x\n",lstart, (UINT32)(rstart>>32), (UINT32)(rstart));
	sysDebugWriteString(achBuffer);

	sprintf (achBuffer,"size:%x\n",dSize);
	sysDebugWriteString(achBuffer);
#endif	
	
	if (!is_power_of_2(size) || size < 0x1000 ||
	    ((UINT64)lstart & (size - 1)) || (rstart & (size - 1)))
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString("tsi721_rio_map_inb_mem:Invalid Input Values\n");
#endif		
		return -EINVAL;
	}

	/* Search for free inbound translation window */
	for (i = 0; i < TSI721_IBWIN_NUM; i++) 
	{
		regval = vReadDWord(priv->regs + TSI721_IBWIN_LB(i));
		if (!(regval & TSI721_IBWIN_LB_WEN))
			break;
	}

	if (i >= TSI721_IBWIN_NUM) 
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"Unable to find free inbound window\n");
		sysDebugWriteString(achBuffer);
#endif		
		return -EBUSY;
	}

	vWriteDWord(priv->regs + TSI721_IBWIN_SZ(i),TSI721_IBWIN_SIZE(size) << 8);
	vWriteDWord(priv->regs + TSI721_IBWIN_TUA(i),((UINT64)lstart >> 32));
	vWriteDWord(priv->regs + TSI721_IBWIN_TLA(i),((UINT64)lstart & TSI721_IBWIN_TLA_ADD));
		  

	vWriteDWord(priv->regs + TSI721_IBWIN_UB(i),rstart >> 32);
	vWriteDWord(priv->regs + TSI721_IBWIN_LB(i),(rstart & TSI721_IBWIN_LB_BA) | TSI721_IBWIN_LB_WEN);
	
	
#ifdef DEBUG_SRIO
	sprintf(achBuffer,"Configured IBWIN%d mapping (RIO_0x%x%x -> ",i, (UINT32)(rstart>>32), (UINT32)(rstart));
	sysDebugWriteString(achBuffer);	
	sprintf (achBuffer,"PCIe_0x%08x)\n",(UINT32)lstart);
	sysDebugWriteString(achBuffer);	
#endif	
	return 0;
}

/**
 * fsl_rio_unmap_inb_mem -- Unmapping inbound memory region.
 * @mport: RapidIO master port
 * @lstart: Local memory space start address.
 */
static void tsi721_rio_unmap_inb_mem(struct rio_mport *mport,
				dma_addr_t lstart)
{
	struct tsi721_device *priv = mport->priv;
	int i;
	UINT64 addr;
	UINT32 regval;

	/* Search for matching active inbound translation window */
	for (i = 0; i < TSI721_IBWIN_NUM; i++) 
	{
		regval = vReadDWord(priv->regs + TSI721_IBWIN_LB(i));
		if (regval & TSI721_IBWIN_LB_WEN) {
			regval = vReadDWord(priv->regs + TSI721_IBWIN_TUA(i));
			addr = (UINT64)regval << 32;
			regval = vReadDWord(priv->regs + TSI721_IBWIN_TLA(i));
			addr |= regval & TSI721_IBWIN_TLA_ADD;

			if (addr == (UINT64)lstart) {
				vWriteDWord(priv->regs + TSI721_IBWIN_LB(i),0);
				break;
			}
		}
	}
}

/*
 * tsi721_get_outb_pref_asinfo - Get Outbound prefetchable address space
 */
static int tsi721_get_outb_pref_asinfo( struct rio_mport *mport,UINT64 *addr, UINT64 *size )
{
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif	
	struct tsi721_device *priv = mport->priv;

	*addr = (UINT64)priv->pfmem_base;
	*size = (UINT64)priv->pf_size;

#ifdef DEBUG_SRIO
	sprintf (achBuffer,"*addr:0x%x%x\n", (UINT32)(*addr>>32), (UINT32)(*addr));	
	sysDebugWriteString(achBuffer);
	
	sprintf(achBuffer,"*size:%x%x\n", (UINT32)(*size>>32), (UINT32)(*size));
	sysDebugWriteString(achBuffer);
	
	sprintf (achBuffer,"priv->pfmem_base:%lx priv->pf_size:0x%x%x\n",
			priv->pfmem_base, (UINT32)(priv->pf_size>>32), (UINT32)(priv->pf_size));
	sysDebugWriteString(achBuffer);
#endif	
	return 0;
}

/*
 * tsi721_get_outb_npref_asinfo - Get Outbound non-prefetechable address space
 */
static int tsi721_get_outb_npref_asinfo( struct rio_mport *mport,UINT64 *addr, UINT64 *size )
{
	struct tsi721_device *priv = mport->priv;

	*addr = (UINT64)priv->npfmem_base;
	*size = (UINT64)priv->npf_size;

	return 0;
}

/*
 * tsi721_map_outb - Map outbound window
 */
static int tsi721_map_outb( struct rio_mport *mport, UINT8 win, UINT16 destid, UINT64 lstart,
		UINT64 rstart, UINT64 size, UINT32 flags )
{
    struct tsi721_device *priv = mport->priv;
    UINT32           regval;
#ifdef DEBUG_SRIO
    char achBuffer[80];
    sprintf (achBuffer,"tsi721_rio_map_outb_mem create win %d PCI Memory Start Addr 0x%x%x",
    		win, (UINT32)(lstart>>32), (UINT32)(lstart));
    sysDebugWriteString(achBuffer);		    
    sprintf (achBuffer,"size:0x%x%x\n", (UINT32)(size>>32), (UINT32)(size));
	sysDebugWriteString(achBuffer);		
#endif

    if (!is_power_of_2(size) || size < 0x1000)
    {
#ifdef DEBUG_SRIO
        sysDebugWriteString ("tsi721_rio_map_outb_mem size error \n");
#endif	
        return -EINVAL;
    }

    if ((UINT64)lstart & (size - 1))
    {
#ifdef DEBUG_SRIO
        sysDebugWriteString ("tsi721_rio_map_outb_mem ERROR WIN addr must by multiple of WIN size \n");
#endif	
        return -EINVAL;
    }

    if( win >= TSI721_OBWIN_NUM )
    {
#ifdef DEBUG_SRIO
    	 sysDebugWriteString ("tsi721_rio_map_outb_mem ERROR , max windows supported are 8\n");
#endif	 
    	        return -EINVAL;
    }

    regval = vReadDWord (priv->regs + TSI721_OBWINLB(win));

    if (regval & TSI721_OBWINLB_WEN)
    {
#ifdef DEBUG_SRIO
        sprintf (achBuffer,"tsi721_rio_map_outb_mem window %d in use! \n", win);
        sysDebugWriteString(achBuffer);
#endif	
        return -EINVAL;
    }

    vWriteDWord (priv->regs + TSI721_OBWINSZ(win),(UINT32)(TSI721_OBWIN_SIZE(size) << 8));
    vWriteDWord (priv->regs + TSI721_OBWINUB(win),(UINT32)(lstart >> 32));
    vWriteDWord (priv->regs + TSI721_OBWINLB(win),(UINT32)((lstart & TSI721_OBWINLB_BA) | TSI721_OBWINLB_WEN));

#ifdef DEBUG_SRIO
    sprintf (achBuffer,"WINDOW %d  LB:  %08x\r\n", win, vReadDWord (priv->regs + TSI721_OBWINLB(win)));
    sysDebugWriteString(achBuffer);
    sprintf (achBuffer,"WINDOW %d  UB:  %08x\r\n", win, vReadDWord (priv->regs + TSI721_OBWINUB(win)));
    sysDebugWriteString(achBuffer);
    sprintf (achBuffer,"WINDOW %d  SZ:  %08x\r\n", win, vReadDWord (priv->regs + TSI721_OBWINSZ(win)));
    sysDebugWriteString(achBuffer);
#endif    

    return 0;
}

/*
 * tsi721_config_outb_zone - Map outbound zone
 */
static int tsi721_config_outb_zone( struct rio_mport *mport, UINT8 win, UINT8 zone, UINT16 destid,
		UINT64 rstart , UINT32 flags )
{
    struct tsi721_device *priv = mport->priv;
    UINT32           lut0;
    UINT32           lut1;
    UINT32           lut2;
    UINT32           regval;

#ifdef DEBUG_SRIO
    char achBuffer[80];

    sprintf (achBuffer,"tsi721_rio_map_outb_zone set win %d zone %d RIO addr %x:0x%x%x\n",
    		win, zone, destid, (UINT32)(rstart>>32), (UINT32)(rstart));
    sysDebugWriteString(achBuffer);		
#endif    

    /* wait for zone select to be available */
    regval = vReadDWord (priv->regs + TSI721_ZONE_SEL);
    while ( regval & TSI721_ZONE_SEL_GO )
    {
        regval = vReadDWord (priv->regs + TSI721_ZONE_SEL);
    }

    if( flags & RIO_NWRITE_SWRITE )
    {
    	lut0 = TSI721_WRTYPE_NWRITE;
    }
    else if ( flags & RIO_MAINTENANCE_WRITE )
    {
    	lut0 = TSI721_WRTYPE_MTWRITE;
    }
    else if ( flags & RIO_NWRITE_RESPONSE )
    {
    	lut0 = TSI721_WRTYPE_NWRITE_R;
    }
    else
    {
    	lut0 = TSI721_WRTYPE_NWRITE;
    }

    lut0 |= (UINT32) ((rstart & 0xFFFFF000) |
    		((flags & RIO_MAINTENANCE_READ) ? TSI721_RDTYPE_MTREAD:TSI721_RDTYPE_NREAD)  << 8);
    lut1 =  (UINT32) (rstart >> 32);
    lut2 =  destid;

    vWriteDWord (priv->regs + TSI721_LUT_DATA0,lut0);
    vWriteDWord (priv->regs + TSI721_LUT_DATA1,lut1);
    vWriteDWord (priv->regs + TSI721_LUT_DATA2,lut2);

    regval = TSI721_ZONE_SEL_GO | (win << 3) | zone;
    vWriteDWord (priv->regs + TSI721_ZONE_SEL,regval);

#ifdef DEBUG_SRIO
    sprintf (achBuffer,"WINDOW %d  Zone %d DATA0:  %08x\r\n", win, zone,vReadDWord (priv->regs + TSI721_LUT_DATA0));
    sysDebugWriteString(achBuffer);
    sprintf (achBuffer,"WINDOW %d  Zone %d DATA1:  %08x\r\n", win, zone,vReadDWord (priv->regs + TSI721_LUT_DATA1));
    sysDebugWriteString(achBuffer);
    sprintf (achBuffer,"WINDOW %d  Zone %d DATA2:  %08x\r\n", win, zone,vReadDWord (priv->regs + TSI721_LUT_DATA2));
    sysDebugWriteString(achBuffer);
#endif
    return 0;
}

/*
 * tsi721_unmap_outb - Un-map outbound window
 */
void tsi721_unmap_outb(struct rio_mport *mport, UINT8 win )
{
    struct tsi721_device *priv = mport->priv;
    UINT32 regval;
    UINT8 i;

    if( win >= TSI721_OBWIN_NUM )
    {
#ifdef DEBUG_SRIO
         sysDebugWriteString ("tsi721_rio_map_outb_mem ERROR , max windows supported are 8\n");
#endif	 
                return;
     }

    regval = vReadDWord (priv->regs + TSI721_OBWINLB(win));

    if (!(regval & TSI721_OBWINLB_WEN))
    {
#ifdef DEBUG_SRIO
        sysDebugWriteString ("tsi721_rio_unmap_outb_mem window NOT in use! \n");
#endif	
        return;
    }

    for( i=0; i < 8; i++ )
        tsi721_config_outb_zone(mport,win,i,0,0,0);

    vWriteDWord (priv->regs + TSI721_OBWINLB(win),0);
    regval = vReadDWord (priv->regs + TSI721_OBWINLB(win));

}


/**
 * tsi721_init_sr2pc_mapping - initializes inbound (SRIO->PCIe)
 * translation regions.
 * @priv: pointer to tsi721 private data
 *
 * Disables inbound windows.
 */
static void tsi721_init_sr2pc_mapping(struct tsi721_device *priv)
{
	int i;

	/* Disable all SR2PC inbound windows */
	for (i = 0; i < TSI721_IBWIN_NUM; i++)
		vWriteDWord(priv->regs + TSI721_IBWIN_LB(i),0);
}

/**
 * tsi721_bdma_maint_init - Initialize maintenance request BDMA channel.
 * @priv: pointer to tsi721 private data
 *
 * Initialize BDMA channel allocated for RapidIO maintenance read/write
 * request generation
 * Returns %0 on success or %-ENOMEM on failure.
 */
static int tsi721_bdma_maint_init(struct tsi721_device *priv)
{
	volatile struct tsi721_dma_desc *bd_ptr;
	volatile UINT64		*sts_ptr;
	dma_addr_t	bd_phys, sts_phys;
	int		sts_size;
	int		bd_num = 2;
	volatile void *regs;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif	
	UINT32 /*dDmaHandle,*/pTsi721DmaBuff,/*dStatusHandle,*/pTsi721StatusBuff;
	/*PTR48	pDma,pDmaStatus;*/
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	UINT32 dummy;
#pragma GCC diagnostic pop

#ifdef DEBUG_SRIO
	sprintf(achBuffer,"Init Block DMA Engine for Maintenance requests, CH%d\n",
		TSI721_DMACH_MAINT);
	sysDebugWriteString(achBuffer);	
#endif
	/*
	 * Initialize DMA channel for maintenance requests
	 */
	priv->mdma.ch_id = TSI721_DMACH_MAINT;
	regs = priv->regs + TSI721_DMAC_BASE(TSI721_DMACH_MAINT);
	
	/* Allocate space for DMA descriptors */
	pTsi721DmaBuff = sysMemGetPhysPtrLocked((UINT64)TSI721_BUF_DESC_BASE,(UINT32)(bd_num * sizeof(struct tsi721_dma_desc)));
	if(pTsi721DmaBuff == NULL)
	{
#ifdef DEBUG_SRIO
			sysDebugWriteString("Unable to allocate DMA Desc Buf \n"); 
#endif			
			return(-ENOMEM);
	}

#ifdef DEBUG_SRIO
	dummy = vReadDWord(priv->regs+RIO_SR_RSP_TO);
	sprintf(achBuffer,"RIO_SR_RSP_TO %#x\n",
			dummy);
		sysDebugWriteString(achBuffer);
#endif

	bd_ptr = (struct tsi721_dma_desc*)pTsi721DmaBuff;
	bd_phys = TSI721_BUF_DESC_BASE;
	
	if (!bd_ptr)
		return -ENOMEM;

	priv->mdma.bd_num = bd_num;
	priv->mdma.bd_phys = bd_phys;
	priv->mdma.bd_base = (struct tsi721_dma_desc*)bd_ptr;

#ifdef DEBUG_SRIO
	sprintf(achBuffer,"DMA descriptors @%x (phys = %x)\n",
		(unsigned int)bd_ptr, (unsigned int)bd_phys);
	sysDebugWriteString(achBuffer);		
#endif	

	/* Allocate space for descriptor status FIFO */
	sts_size = (bd_num >= TSI721_DMA_MINSTSSZ) ?
					bd_num : TSI721_DMA_MINSTSSZ;
					
#ifdef DEBUG_SRIO
	sprintf(achBuffer,"1. bd_num @ %d sts_size:%d\n",
		bd_num, (unsigned int)sts_size);
	sysDebugWriteString(achBuffer);		
#endif					
	
	sts_size = roundup_pow_of_two(sts_size);
	
#ifdef DEBUG_SRIO
	sprintf(achBuffer,"2. bd_num @ %d sts_size:%d\n",
		bd_num, (unsigned int)sts_size);
	sysDebugWriteString(achBuffer);		
#endif	
	sysDebugFlush();
	pTsi721StatusBuff = sysMemGetPhysPtrLocked((UINT64)TSI721_DMA_STATUS_BASE,(UINT32)(sts_size * sizeof(struct tsi721_dma_sts)));
	
	if (pTsi721StatusBuff == NULL)
	{
	 	sysDebugWriteString("Unable to allocate DMA Status Buff \n"); 
		return(-ENOMEM);
	}
	
	sts_ptr = (UINT64*)pTsi721StatusBuff;
	sts_phys = TSI721_DMA_STATUS_BASE;
	
	sysDebugFlush();

#ifdef DEBUG_SRIO
	sprintf(achBuffer,"desc status FIFO @ %x sts_size = %d phys=%x\n",
		(unsigned int)sts_ptr,(unsigned int)sts_size, (unsigned int)sts_phys);		
	sysDebugWriteString(achBuffer);		
#endif	
	
	if( sts_ptr )
	{
		memset((UINT64*)sts_ptr, 0 ,(sts_size * sizeof(struct tsi721_dma_sts)));
	}

	if (!sts_ptr) 
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString("sts_ptr not valid freeing dStatusHandle\n");
#endif		
		/* Free space allocated for DMA descriptors */
		priv->mdma.bd_base = NULL;
		return -ENOMEM;
	}

	priv->mdma.sts_phys = sts_phys;
	priv->mdma.sts_base = (UINT64*)sts_ptr;
	priv->mdma.sts_size = sts_size;

	/* Initialize DMA descriptors ring */
	bd_ptr[bd_num - 1].type_id = cpu_to_le32(DTYPE3 << 29);
	bd_ptr[bd_num - 1].next_lo = cpu_to_le32((UINT64)bd_phys &
						 TSI721_DMAC_DPTRL_MASK);
	bd_ptr[bd_num - 1].next_hi = cpu_to_le32((UINT64)bd_phys >> 32);

	/* Setup DMA descriptor pointers */
	vWriteDWord(regs + TSI721_DMAC_DPTRH,((UINT64)bd_phys >> 32));
	vWriteDWord(regs + TSI721_DMAC_DPTRL,((UINT64)bd_phys & TSI721_DMAC_DPTRL_MASK));

	/* Setup descriptor status FIFO */
	vWriteDWord(regs + TSI721_DMAC_DSBH,((UINT64)sts_phys >> 32));
	vWriteDWord(regs + TSI721_DMAC_DSBL,((UINT64)sts_phys & TSI721_DMAC_DSBL_MASK));
	vWriteDWord(regs + TSI721_DMAC_DSSZ,TSI721_DMAC_DSSZ_SIZE(sts_size));

	/* Clear interrupt bits */
	vWriteDWord(regs + TSI721_DMAC_INT,TSI721_DMAC_INT_ALL);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	dummy = vReadDWord(regs + TSI721_DMAC_INT);
#pragma GCC diagnostic pop

	/* Toggle DMA channel initialization */
	vWriteDWord(regs + TSI721_DMAC_CTL,TSI721_DMAC_CTL_INIT);
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
	dummy = vReadDWord(regs + TSI721_DMAC_CTL);
#pragma GCC diagnostic pop

	vDelay(10);
	return 0;
}

static int tsi721_bdma_maint_free(struct tsi721_device *priv)
{
	UINT32 ch_stat;
	struct tsi721_bdma_maint *mdma = &priv->mdma;
	volatile void *regs = priv->regs + TSI721_DMAC_BASE(mdma->ch_id);

	if (mdma->bd_base == NULL)
		return 0;

	/* Check if DMA channel still running */
	ch_stat = vReadDWord(regs + TSI721_DMAC_STS);
	if (ch_stat & TSI721_DMAC_STS_RUN)
		return -EFAULT;

	/* Put DMA channel into init state */
	vWriteDWord(regs + TSI721_DMAC_CTL,TSI721_DMAC_CTL_INIT);

	/* Free space allocated for DMA descriptors */
	mdma->bd_base = NULL;
	mdma->sts_base = NULL;
	return 0;
}

/**
 * tsi721_messages_init - Initialization of Messaging Engine
 * @priv: pointer to tsi721 private data
 *
 * Configures Tsi721 messaging engine.
 */
static int tsi721_messages_init(struct tsi721_device *priv)
{
	int	ch;

	vWriteDWord(priv->regs + TSI721_SMSG_ECC_LOG,0);
	vWriteDWord(priv->regs + TSI721_RETRY_GEN_CNT,0);
	vWriteDWord(priv->regs + TSI721_RETRY_RX_CNT,0);

	/* Set SRIO Message Request/Response Timeout */
	vWriteDWord(priv->regs + TSI721_RQRPTO,TSI721_RQRPTO_VAL);

	/* Initialize Inbound Messaging Engine Registers */
	for (ch = 0; ch < TSI721_IMSG_CHNUM; ch++) {
		/* Clear interrupt bits */
		vWriteDWord(priv->regs + TSI721_IBDMAC_INT(ch),TSI721_IBDMAC_INT_MASK);
		/* Clear Status */
		vWriteDWord(priv->regs + TSI721_IBDMAC_STS(ch),0);

		vWriteDWord(priv->regs + TSI721_SMSG_ECC_COR_LOG(ch),TSI721_SMSG_ECC_COR_LOG_MASK);
		vWriteDWord(priv->regs + TSI721_SMSG_ECC_NCOR(ch),TSI721_SMSG_ECC_NCOR_MASK);
	}

	return 0;
}

/**
 * tsi721_disable_ints - disables all device interrupts
 * @priv: pointer to tsi721 private data
 */
static void tsi721_disable_ints(struct tsi721_device *priv)
{
	int ch;

	/* Disable all device level interrupts */
	vWriteDWord(priv->regs + TSI721_DEV_INTE,0);

	/* Disable all Device Channel interrupts */
	vWriteDWord(priv->regs + TSI721_DEV_CHAN_INTE,0);

	/* Disable all Inbound Msg Channel interrupts */
	for (ch = 0; ch < TSI721_IMSG_CHNUM; ch++)
		vWriteDWord(priv->regs + TSI721_IBDMAC_INTE(ch),0);

	/* Disable all Outbound Msg Channel interrupts */
	for (ch = 0; ch < TSI721_OMSG_CHNUM; ch++)
		vWriteDWord(priv->regs + TSI721_OBDMAC_INTE(ch),0);

	/* Disable all general messaging interrupts */
	vWriteDWord(priv->regs + TSI721_SMSG_INTE,0);

	/* Disable all BDMA Channel interrupts */
	for (ch = 0; ch < TSI721_DMA_MAXCH; ch++)
		vWriteDWord(priv->regs + TSI721_DMAC_BASE(ch) + TSI721_DMAC_INTE,0);

	/* Disable all general BDMA interrupts */
	vWriteDWord(priv->regs + TSI721_BDMA_INTE,0);

	/* Disable all SRIO Channel interrupts */
	for (ch = 0; ch < TSI721_SRIO_MAXCH; ch++)
		vWriteDWord(priv->regs + TSI721_SR_CHINTE(ch),0);

	/* Disable all general SR2PC interrupts */
	vWriteDWord(priv->regs + TSI721_SR2PC_GEN_INTE,0);

	/* Disable all PC2SR interrupts */
	vWriteDWord(priv->regs + TSI721_PC2SR_INTE,0);

	/* Disable all I2C interrupts */
	vWriteDWord(priv->regs + TSI721_I2C_INT_ENABLE,0);

	/* Disable SRIO MAC interrupts */
	vWriteDWord(priv->regs + TSI721_RIO_EM_INT_ENABLE,0);
	vWriteDWord(priv->regs + TSI721_RIO_EM_DEV_INT_EN,0);
}

/**
 * tsi721_setup_mport - Setup Tsi721 as RapidIO subsystem master port
 * @priv: pointer to tsi721 private data
 *
 * Configures Tsi721 as RapidIO master port.
 */
static int  tsi721_setup_mport(struct tsi721_device *priv)
{
	
	// int err = 0;
	struct rio_ops *ops;
	unsigned int size,size_mport;	
	struct rio_mport *mport;

#ifdef DEBUG_SRIO
    sysDebugWriteString ("In Function: tsi721_setup_mport()\n\n");
#endif    
	size = sizeof(struct rio_ops);
		
	ops = (struct rio_ops*) k_malloc(size);
	
	memset(ops,0,size);

	if (!ops) 
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString("Unable to allocate memory for rio_ops\n");
#endif		
		return -ENOMEM;
	}

	ops->lcread = tsi721_lcread;
	ops->lcwrite = tsi721_lcwrite;
	ops->cread = tsi721_cread_dma;
	ops->cwrite = tsi721_cwrite_dma;	
	ops->map_inb = tsi721_rio_map_inb_mem;
	ops->unmap_inb = tsi721_rio_unmap_inb_mem;
	ops->get_outb_pref_asinfo = tsi721_get_outb_pref_asinfo;
	ops->get_outb_npref_asinfo = tsi721_get_outb_npref_asinfo;
	ops->map_outb = tsi721_map_outb;
	ops->unmap_outb = tsi721_unmap_outb;
	ops->config_outb_zone = tsi721_config_outb_zone;


	size_mport = sizeof(struct rio_mport);
	
	//mport = kzalloc(sizeof(struct rio_mport), GFP_KERNEL);
	 mport = (struct rio_mport*)k_malloc(size_mport);
	 
	 memset(mport,0,size_mport);
	
	if (!mport) 
	{	
		k_free(ops);		
#ifdef DEBUG_SRIO
		sysDebugWriteString("Unable to allocate memory for mport\n");
#endif		
		return -ENOMEM;
	}

	mport->ops = ops;
	mport->index = 0;
	mport->sys_size = 0; /* small system */
	mport->phy_type = RIO_PHY_SERIAL;
	mport->priv = (void *)priv;
	mport->phys_efptr = 0x100;
	mport->amc_slot_id = amcSlotId; /*Hari: Newly added Slot ID*/
	priv->mport = mport;


	sprintf(mport->name,DRV_DESC);

	/* Hook up interrupt handler */

#ifdef CONFIG_PCI_MSI
	if (!tsi721_enable_msix(priv))
		priv->flags |= TSI721_USING_MSIX;
	else if (!pci_enable_msi(pdev))
		priv->flags |= TSI721_USING_MSI;
	else
		dev_info(&pdev->dev,
			 "MSI/MSI-X is not available. Using legacy INTx.\n");
#endif /* CONFIG_PCI_MSI */

#if 0 //Date: 03/03/2014
	if (!err) {
		tsi721_interrupts_init(priv);
		ops->pwenable = tsi721_pw_enable;
	} else {
		/*sysDebugWriteString("Unable to get assigned PCI IRQ "
			"vector %02X err=0x%x\n", pdev->irq, err);*/
		goto err_exit;
	}
#endif
#ifdef CONFIG_RAPIDIO_DMA_ENGINE
	tsi721_register_dma(priv);
#endif
	/* Enable SRIO link */
	vWriteDWord(priv->regs + TSI721_DEVCTL,vReadDWord(priv->regs + TSI721_DEVCTL) |
		  TSI721_DEVCTL_SRBOOT_CMPL);

	rio_register_mport(mport);

	if (mport->host_deviceid >= 0)
		vWriteDWord(priv->regs + (0x100 + RIO_PORT_GEN_CTL_CSR),RIO_PORT_GEN_HOST | RIO_PORT_GEN_MASTER |
			  RIO_PORT_GEN_DISCOVERED);
			  
#ifdef DEBUG_SRIO
	sysDebugWriteString ("Out Function: tsi721_setup_mport()\n\n");		  
#endif	
	return 0;

#if 0 //Date: 03/03/2014
err_exit:
	k_free(priv);
	k_free(mport);
	return err;
#endif
}


int tsi721_init(UINT32 dInstance)
{
	struct tsi721_device *priv;
	int cap;
	int err;
	UINT32 regval;
	UINT32 size_tsi721dev;	
	UINT32 mBar0Handle;
	PCI_PFA pfa;
	UINT16 wTemp;
	UINT64 dBar0Address,dBar2Address,dBar4Address;
	UINT32  dBar2Size=0,dBar4Size=0;
	char achBuffer[80];
	UINT8 amcIpmbAddrs = 0;
	UINT8 amcSPSStatus = 0,bSrioBrate=0;
	UINT32 retryCount;
	UINT32 masterMode;
	UINT8 umswitch = 0;
	
#ifdef DEBUG_SRIO
	UINT32 dIdVid =0;
#endif
	

	size_tsi721dev = (sizeof(struct tsi721_device));
	priv = (struct tsi721_device*)k_malloc(size_tsi721dev);	
	memset(priv,0,size_tsi721dev);
	memset (&pfa, 0, sizeof(pfa));
	
	if (priv == NULL) 
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString("Failed to allocate memory for device\n");
#endif		
		err = -ENOMEM;
		goto err_exit;
	}

	amcIpmbAddrs = bGetAMCIpmbAddrs();
#ifdef DEBUG_SRIO
	sprintf(achBuffer,"AMC IPMB ADDRS:%x\n",amcIpmbAddrs);
	sysDebugWriteString(achBuffer);	
#endif	
	amcSlotId = ((amcIpmbAddrs - 0x72)/2)+1;
	
#ifdef DEBUG_SRIO
	sprintf(achBuffer,"AMC SLOT ID:%x\n",amcSlotId);
	sysDebugWriteString(achBuffer);	
#endif
	
	amcSPSStatus = bGetSPSStatus(&bSrioBrate);	
//#ifdef DEBUG_SRIO
	sprintf(achBuffer,"amcSPSStatus:%x\n",amcSPSStatus);
	vConsoleWrite(achBuffer);
//#endif
	vDelay(500);

	board_service(SERVICE__BRD_USR_MTH_SWITCH, NULL, &umswitch);

	retryCount = 0;	 

	if (umswitch == 1)
	{
		retryCount = 600; //Changed from 60 to 600 to support CCT MCH is polling based IPMB communication too slow.
	}
	else
	{
		retryCount = 60;
	}

	while (amcSPSStatus != CC_OK)
	{
		vDelay(500);
		amcSPSStatus = bGetSPSStatus(&bSrioBrate);
/*
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"amcSPSStatus:%x retries: %u\n",amcSPSStatus, retryCount);
		sysDebugWriteString(achBuffer);	
#endif	
*/		

		if (amcSPSStatus != CC_OK)
		{
			retryCount--;

			if (retryCount <= 0)
			{
				sprintf(achBuffer,"Get SPS status timeout!, using defaults\n");
				sysDebugWriteString(achBuffer);	
				
				/* Use default */
				bSrioBrate = 0x2; /* 3.125 GBaud */
				//bSrioBrate = 0x4; /* 5.0 GBaud */
				break;
/*				
				k_free( priv );
				return (E__FAIL);
*/
			}
		}
	}

	sprintf(achBuffer,"amcSPSStatus:%x retries: %u\n",amcSPSStatus, retryCount);
	vConsoleWrite(achBuffer);
	sprintf(achBuffer,"bSrioBrate:%x\n",bSrioBrate + 1);
	vConsoleWrite(achBuffer);
	vConsoleWrite("Delay for MCH to establish connection speed...\n");
	// initial bSrioBrate value returned by MCH may be wrong 
	// as the MCH may not have had time to establish the connection
	if (amcSPSStatus == CC_OK)
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"bSrioBrate:%x\n",bSrioBrate + 1);
		sysDebugWriteString(achBuffer);
		sysDebugWriteString("Delay for MCH to establish connection speed...\n");
#endif

		vDelay(1000);
		amcSPSStatus = bGetSPSStatus(&bSrioBrate);
		
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"bSrioBrate:%x\n",bSrioBrate + 1);
		sysDebugWriteString(achBuffer);
#endif
	}

	bSrioBrate += 1;

	globalSRIOSpeed = bSrioBrate;

	if (E__OK == iPciFindDeviceById (dInstance,0x111d,0x80ab,&pfa))
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nTSI721 instance %u found @B:%d D:%d F:%d\n",
					dInstance, PCI_PFA_BUS(pfa), PCI_PFA_DEV(pfa),PCI_PFA_FUNC(pfa));
		sysDebugWriteString(achBuffer);	
#endif		
	}
	else
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nTSI721 instance %u not found\n", dInstance);
		sysDebugWriteString(achBuffer);                
#endif
		return (E__FAIL);
	}

	masterMode = board_service(SERVICE__BRD_CHECK_SRIO_MASTER, NULL, NULL);
	
	/* If reading SRIO mode (master or slave) from status register is not supported */
	if (masterMode == SERVICE_NOT_SUPPORTED)
	{
		/* use old method based on amcSlotId */
		/* note: slot 10 is the default used in test for the master board */
		if ( amcSlotId == 10 )
		{
			masterMode = E__OK;
		}
	}
	
	if (masterMode == E__OK)
	{
		if (dInstance == 1)
		{
#ifdef DEBUG_SRIO
			sysDebugWriteString ("MASTER 0\n");
#endif
			riohdid0=0;
		}
		
		if (dInstance == 2)
		{		
#ifdef DEBUG_SRIO
			sysDebugWriteString ("MASTER 1\n");
#endif
			riohdid1=4;
		}
	}
	else
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString ("SLAVE\n");
#endif
	}

	wTemp = (UINT16)dPciReadReg (pfa, PCI_COMMAND, REG_16);
	vPciWriteReg (pfa, PCI_COMMAND, REG_16, (wTemp | 0x7));
	
#ifdef DEBUG_SRIO
	dIdVid = PCI_READ_DWORD (pfa, 0x00);
	sprintf (achBuffer,"DID:%#x VID:%#x\n",((dIdVid)&0xFFFF),((dIdVid>>16)&0xFFFF));
	sysDebugWriteString(achBuffer);

	sprintf(achBuffer,"BARSETUP0: 0x%08x BAR0: 0x%08x\n",
			PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP0), PCIE_READ_DWORD(pfa,0x10));
	sysDebugWriteString(achBuffer);
	 
	sprintf(achBuffer,"BARSETUP2: 0x%08x BAR2: 0x%08x\n",
			PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP2), PCIE_READ_DWORD(pfa,0x18));
	sysDebugWriteString(achBuffer);
	
	sprintf(achBuffer,"BARSETUP4: 0x%08x BAR4: 0x%08x\n",
			PCIE_READ_DWORD(pfa,TSI721_PCIECFG_BARSETUP4), PCIE_READ_DWORD(pfa,0x20));
	sysDebugWriteString(achBuffer);
#endif	
	dBar0Address = PCI_READ_DWORD (pfa, 0x10);			
	mBar0Handle = sysMemGetPhysPtrLocked((UINT64)dBar0Address,(0x80000));
	
	if( mBar0Handle == NULL)
	{
			return (-ENOMEM);
	}
	
	priv->regs = (UINT32*)mBar0Handle;
	
	
	if (!priv->regs) 
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString("Unable to map device registers space, aborting\n");
#endif		
		err = -ENOMEM;
		goto err_free_res; 
	}
	else
	{
#ifdef DEBUG_SRIO
		sprintf (achBuffer,"priv->regs:%x\n",(UINT32)priv->regs);
		sysDebugWriteString(achBuffer);
#endif		
	}

#ifdef DEBUG_SRIO
		sprintf (achBuffer,"Before Errata Work Arounds\n");
		sysDebugWriteString(achBuffer);
#endif
	/*
	 * tsi 721 errata  work arounds
	 */
	tsi721_errata_workaround_1(priv);
	tsi721_errata_workaround_2(priv);
#ifdef DEBUG_SRIO
		sprintf (achBuffer,"After Errata Work Arounds\n");
		sysDebugWriteString(achBuffer);
#endif

	/* 
	 *  Set Baud Rate 
	 */ 
	tsi721_set_srio_linkrate(priv,bSrioBrate);
	
#ifdef DEBUG_SRIO
	sprintf (achBuffer,"After set srio link rate\n");
	sysDebugWriteString(achBuffer);
#endif
	 tsi721_set_srio_boot_cmplbit(priv);

#ifdef DEBUG_SRIO
	sprintf (achBuffer,"After set srio boot cmplbit Errata Work Arounds\n");
	sysDebugWriteString(achBuffer);
#endif
	
#if 0
	if (masterMode == E__OK)
	{
		vDelay(14000);
	}
	else
	{
		//vDelay(4000); //Hari 27/10/2014
		vDelay(1000*amcSlotId);
	}
#endif

#ifdef DEBUG_SRIO
	sprintf (achBuffer,"Before dBar2Address\n");
	sysDebugWriteString(achBuffer);
#endif
	dBar2Address = PCI_READ_DWORD (pfa, 0x18);	
	dBar2Address &= 0xFFFFFFF0;
	
	//dBar2Size = (16 * ONE_MEG);
	dBar2Size = (8 * ONE_MEG);

#ifdef DEBUG_SRIO
	sprintf (achBuffer,"Before dBar2Size\n");
	sysDebugWriteString(achBuffer);
#endif

	priv->pfmem_base = dBar2Address; 
	if (!priv->pfmem_base) 
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString("Outbound prefetchable memory not available\n");
#endif
	}
	else
	{
#ifdef DEBUG_SRIO
		sprintf (achBuffer,"priv->pfmem_base:%x\n",(UINT32)priv->pfmem_base);
		sysDebugWriteString(achBuffer);
#endif
	}

	priv->pf_size = dBar2Size; /*Hari: need to find pci_resource_len */

	dBar4Address = PCI_READ_DWORD (pfa, 0x20);	
	dBar4Address &= 0xFFFFFFF0;
	dBar4Size = (4 * ONE_MEG);

	priv->npfmem_base = dBar4Address;
	priv->npf_size = dBar4Size; /*Hari: need to find pci_resource_len */
	
	if (!priv->npfmem_base) 
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString("Outbound non prefetchable memory not available\n");
#endif
	}
	else
	{
#ifdef DEBUG_SRIO
		sprintf (achBuffer,"priv->npfmem_base:%x\n",(UINT32)priv->npfmem_base);
		sysDebugWriteString(achBuffer);
#endif
	}


#if 0
	if (masterMode == E__OK)
	{
		vDelay(2000); 	 
	}
	else
	{
		vDelay(2000/amcSlotId); 	 
	}
#endif

	cap = TSI721_PCIE_CAP_REG;
	
	/* Clear "no snoop" and "relaxed ordering" bits, use maximum MRRS. */
	regval = PCI_READ_DWORD(pfa,(cap+PCI_EXP_DEVCTL));
	regval &= ~(PCI_EXP_DEVCTL_READRQ | PCI_EXP_DEVCTL_RELAX_EN | PCI_EXP_DEVCTL_NOSNOOP_EN);
	regval |= 0x5 << MAX_READ_REQUEST_SZ_SHIFT;
	PCI_WRITE_DWORD(pfa,cap + PCI_EXP_DEVCTL,regval);
	
	/* Adjust PCIe completion timeout. */
	regval = PCI_READ_DWORD(pfa,(cap+PCI_EXP_DEVCTL2));
	regval &= ~(0x0f);
	PCI_WRITE_DWORD(pfa,cap + PCI_EXP_DEVCTL2,regval|0x2);
	
	/*
	* FIXUP: correct offsets of MSI-X tables in the MSI-X Capability Block
	*/
	
	PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_EPCTL,(UINT32)0x1);
	PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_MSIXTBL,(UINT32)TSI721_MSIXTBL_OFFSET);
	PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_MSIXPBA,(UINT32)TSI721_MSIXPBA_OFFSET);
	PCIE_WRITE_DWORD(pfa,TSI721_PCIECFG_EPCTL,(UINT32)0);
	
	
	/* End of FIXUP */
	tsi721_disable_ints(priv);
	
	tsi721_init_pc2sr_mapping(priv);
	tsi721_init_sr2pc_mapping(priv);

	if (tsi721_bdma_maint_init(priv)) 
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString("BDMA initialization failed, aborting\n");
#endif
		err = -ENOMEM;
		goto err_unmap_bars;
	}

	err = tsi721_messages_init(priv);
	if (err)
		goto err_free_consistent;

	if (masterMode == E__OK)
	{
  	   vDelay(2000);
	}

	err = tsi721_setup_mport(priv);
	if (err)
		goto err_free_consistent;

	return 0;

err_free_consistent:

	tsi721_bdma_maint_free(priv);
err_unmap_bars:
	if (priv->regs)
		vFreePtr(mBar0Handle);	
err_free_res:
	/*vFreePtr(dHandle);*/
err_exit:
	return err;
}
