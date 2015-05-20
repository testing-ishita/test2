/************************************************************************
 *                                                                      *
 *      Copyright 2014 Concurrent Technologies, all rights reserved.    *
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/enpmc.c,v 1.2 2015-03-10 12:40:52 mgostling Exp $
 * $Log: enpmc.c,v $
 * Revision 1.2  2015-03-10 12:40:52  mgostling
 * Disabled debug definition
 *
 * Revision 1.1  2015-02-03 12:07:28  mgostling
 * Added suppport for ENPMC_OO1
 *
 *
 * Initial version of EN PMC 8619 DMA support.
 *
 */

/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>
#include <devices/enpmc8619.h>
#include <private/cpu.h>

/* defines */

//#define DEBUG

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))
#define osWriteDWord(regAddr,value)	(*(UINT32*)(regAddr) = value)

#define vWriteByte(regAddr,value)	(*(UINT8*) (regAddr) = value)
#define vWriteWord(regAddr,value)	(*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)	(*(UINT32*)(regAddr) = value)

#define E__TEST_NO_MEM					E__BIT  + 0x02
#define E__NO_MEM						E__BIT  + 0x03
#define E__DMA_FATAL_ERROR				E__BIT  + 0x04
#define E__ALL_DMA_CHANNLES_IN_USE		E__BIT  + 0x05
#define E__INVALID_DMA_DESCRIPTOR		E__BIT  + 0x06
#define E__INVALID_DMA_CHANNEL			E__BIT  + 0x07
#define E__BAD_GPIO_LOOPBACK			E__BIT  + 0x08

#define DMA_WRITE	1
#define DMA_READ	0

#define TESTPAT1		0x12345678L
#define TEST_RAM_ADDR1	0x0CA00000L
#define TEST_RAM_ADDR	0x0C900000L
#define TEST_DMA_DESC	0x0C800000L

#define DMA_BUFF_SIZE	(0x100000)

#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

#define PLX_DMA_ERROR_HEADER_LOG_0 0x40
#define PLX_DMA_ERROR_HEADER_LOG_1 0x44
#define PLX_DMA_ERROR_HEADER_LOG_2 0x48
#define PLX_DMA_ERROR_HEADER_LOG_3 0x4C
#define PLX_DMA_DEV_ERROR_STAT     0x50
#define PLX_DMA_MAX_OS_RW_REQ      0x54

/* typedefs */

/* locals */

static ENPMC_P8619_DMA_CHAN_INFO dma_channels[MAX_DMA_CHANNELS];
static PTR48 tPtr3,tPtr1,tPtr2,tPtr3,tPtr4;
static UINT32 mHandle,mHandle1,mHandle2,mHandle3,mHandle4;

/* globals */

/* externals */

/* forward declarations */

static INT32 enpmc_pex8619_dma_status(ENPMC_PEX_8619_DMA_CTRL *p8619Dma, INT32 descriptor);
static INT32 enpmc_pex8619_is_valid_channel(INT32 handle);
void wait_signal(volatile INT32 *flag, INT32 bit);
static INT32 enpmc_pex8619_wait_dma( ENPMC_PEX_8619_DMA_CTRL *p8619Dma, INT32 descriptor);
UINT32 enpmc_pex8619_dma_test (void);
UINT32 enpmc_pex8619_prepare_sg_buffer (addr_t from, addr_t to, INT32 count,
										INT32 dir, ENPMC_P8619_DMA_CHAN_INFO *pDmaChanInfo);

/*****************************************************************************
 * enpmc_pex8619_init:
 *
 * Initialise resources needed for the PLX8619
 *
 * RETURNS: E__OK or error
 */
UINT32 enpmc_pex8619_init
(
	ENPMC_PEX_8619_DEV_CTRL *p8619Dev,
	UINT32 dInstance,
	UINT32 dSrcPhysAddr,
	UINT32 dDstPhysAddr
)
{
	PCI_PFA pfa;
	UINT32 dBarAddress;
	UINT32 dBar;
	UINT32 ii = 0;
	UINT32 dmaDevice;
#ifdef DEBUG
	UINT32 temp = 0;
	char buffer[80];
#endif

	memset (&pfa, 0, sizeof(pfa));
	if (E__OK == iPciFindDeviceById (dInstance, 0x10B5, 0x8619, &pfa))
	{
#ifdef DEBUG
		sprintf (buffer, "Found PEX 8619: pfa : %x\n", pfa);
		vConsoleWrite (buffer);
#endif

		dBarAddress = PCI_READ_DWORD (pfa, 0x10);
		p8619Dev->bus = PCI_PFA_BUS (pfa);
		p8619Dev->device = PCI_PFA_DEV (pfa);
		p8619Dev->function = PCI_PFA_FUNC (pfa);

#ifdef DEBUG
		sprintf (buffer, "BarAddress:  0x%x\n", dBarAddress);
		vConsoleWrite (buffer);
		sprintf (buffer, "Bus = %d, Dev = %d , Func = %d\n", p8619Dev->bus, p8619Dev->device, p8619Dev->function);
		vConsoleWrite (buffer);
#endif

		// check for DMA Device
		pfa = PCI_MAKE_PFA (p8619Dev->bus, 0x0, 1);
		dmaDevice = PCI_READ_DWORD (pfa, 0x0);

#ifdef DEBUG
		sprintf (buffer, "dmaDevice = 0x%x\n", dmaDevice);
		vConsoleWrite (buffer);
#endif

		if (dmaDevice == 0x861910b5)
		{
#ifdef DEBUG
			sprintf (buffer, "DMA Device Present : pfa : 0x%x\n", pfa);
			vConsoleWrite (buffer);
#endif
		}
		else
		{
			vConsoleWrite ("Dma Device not found\n");
			return (E__BIT + E__DEVICE_NOT_FOUND);
		}

		mHandle3 = dGetPhysPtr (dBarAddress, 0x40000, &tPtr3, (void*) &(p8619Dev->csr));

#ifdef DEBUG
		sprintf (buffer, "mHandle3 = %x\n", mHandle3);
		vConsoleWrite (buffer);
#endif

		if (mHandle3 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}

#ifdef DEBUG
		sprintf (buffer, "DevCtrl(Reg 0x70) = 0x%x\n", vReadDWord (p8619Dev->csr + 0x70));
		vConsoleWrite (buffer);

		temp = vReadDWord (p8619Dev->csr + 0x70);
		vWriteDWord (p8619Dev->csr + 0x70, (temp | 0xF0000));	// Clear the Error just in case

		sprintf (buffer, "DevCtrl(Reg 0x70) = 0x%x\n", vReadDWord (p8619Dev->csr + 0x70));
		vConsoleWrite (buffer);
#endif

		// BAR address for
		dBar = dPciReadReg (pfa, 0x18, REG_32);
		dBar = (dBar & 0xffffff00);

		// Create 2MB Buffer for DMA transfer
		mHandle1 = dGetPhysPtr (dSrcPhysAddr, DMA_BUFF_SIZE, &tPtr1, (void*) &(p8619Dev->ramMem1));

#ifdef DEBUG
		sprintf (buffer, "ramMem1  : %x\n", (UINT32)(p8619Dev->ramMem1));
		vConsoleWrite (buffer);
		sprintf (buffer, "mHandle1  : %d\n", mHandle1);
		vConsoleWrite (buffer);
#endif

		if (mHandle1 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}

		//Initialize the buffer with a test pattern
		for (ii = 0; ii < DMA_BUFF_SIZE; ii += 4)
		{
			osWriteDWord ((p8619Dev->ramMem1 + ii), (UINT32)TESTPAT1);
		}

		mHandle2 = dGetPhysPtr (dDstPhysAddr, DMA_BUFF_SIZE, &tPtr2, (void*) &(p8619Dev->ramMem));
		if (mHandle2 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}

#ifdef DEBUG
		sprintf (buffer, "ramMem  : %x\n", (UINT32)(p8619Dev->ramMem));
		vConsoleWrite (buffer);
		sprintf (buffer, "mHandle2  : %d\n", mHandle2);
		vConsoleWrite (buffer);
#endif

		pfa = PCI_MAKE_PFA (p8619Dev->bus, 0x0, 1);
		dBarAddress = PCI_READ_DWORD (pfa, 0x10);

#ifdef DEBUG
		sprintf (buffer, "dBarAddress  : %x\n", dBarAddress);
		vConsoleWrite (buffer);
#endif
	}
	else
	{
		vConsoleWrite ("No ENPMC Card Inserted\n");
		return (E__BIT + E__DEVICE_NOT_FOUND);
	}

	return (E__OK);
}

/*****************************************************************************
 * enpmc_pex8619_dma_init:
 *
 * Initialise resources for the needed PLX8619 DMA interface
 *
 * RETURNS: E__OK or error
 */
UINT32 enpmc_pex8619_dma_init
(
	ENPMC_PEX_8619_DMA_CTRL *p8619Dma,
	ENPMC_P8619_DMA_CHAN_INFO *pDmaChanInfo,
	UINT32 bDmaInst
)
{
	PCI_PFA pfa;
	UINT32 dBarAddress;
	UINT32 dTemp;
	UINT32 i = 0;
	UINT32 dVidDid = 0;
#ifdef DEBUG
	UINT32 temp = 0;
	char buffer[80];
#endif

	memset (&pfa, 0, sizeof(pfa));
	if (E__OK == iPciFindDeviceById (bDmaInst, 0x10B5, 0x8619, &pfa))
	{
#ifdef DEBUG
		vConsoleWrite ("PLX 8619 DMA Init\n");
#endif

		dBarAddress = PCI_READ_DWORD (pfa, 0x10);
	 	p8619Dma->bus = PCI_PFA_BUS (pfa);
		p8619Dma->device = PCI_PFA_DEV (pfa);
		p8619Dma->function = PCI_PFA_FUNC (pfa);


#ifdef DEBUG
		sprintf (buffer, "BarAddress:  0x%x\n", dBarAddress);
		vConsoleWrite (buffer);

		sprintf (buffer, "Bus = %d, Dev = %d , Func = %d\n", p8619Dma->bus, p8619Dma->device, p8619Dma->function);
		vConsoleWrite (buffer);
#endif


		dTemp = PCI_READ_DWORD (pfa, 0x04);

#ifdef DEBUG
		sprintf (buffer, "DMA Space PCI Command Status Reg(Before) :  0x%x\n", dTemp);
		vConsoleWrite (buffer);
#endif

 		PCI_WRITE_DWORD (pfa, 0x04, (dTemp | 0x00000007));

#ifdef DEBUG
 		dTemp = PCI_READ_DWORD (pfa, 0x04);
		sprintf (buffer, "DMA Space PCI Command Status Reg(After):  0x%x\n", dTemp);
		vConsoleWrite (buffer);
#endif

		mHandle4 = dGetPhysPtr (dBarAddress, 0x40000, &tPtr4, (void*) &(p8619Dma->dmacsr));

		if (mHandle4 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}

#ifdef DEBUG
		sprintf (buffer, "mHandle4 = %x\n", mHandle4);
		vConsoleWrite (buffer);

		sprintf (buffer, "DevCtrl(Reg 0x70) = 0x%x\n", vReadDWord (p8619Dma->dmacsr + 0x70));
		vConsoleWrite (buffer);

		temp = vReadDWord (p8619Dma->dmacsr + 0x70);
		vWriteDWord (p8619Dma->dmacsr + 0x70, (temp | 0xF0000)); // Clear the Error just in case

		sprintf (buffer, "DevCtrl(Reg 0x70) = 0x%x\n", vReadDWord (p8619Dma->dmacsr + 0x70));
		vConsoleWrite (buffer);
#endif

		dVidDid = vReadDWord (p8619Dma->dmacsr);

#ifdef DEBUG
		sprintf (buffer, "DMA Config Space Reg Addr = 0x%x and dVidDid  = 0x%x\n", (UINT32) p8619Dma->dmacsr, dVidDid);
		vConsoleWrite (buffer);
#endif


		if (dVidDid == 0x861910b5)
		{
#ifdef DEBUG
			vConsoleWrite ("PLX 8619 DMA Device Found\n");
#endif
		}
		else
		{
#ifdef DEBUG
			vConsoleWrite ("PLX 8619 DMA Device NOT Found\n");
#endif
			vFreePtr (mHandle4);
			return (E__BIT + E__DEVICE_NOT_FOUND);
		}

		/* Initialize all the dma channels to be not in use */
		i = 0;
		while (i < array_size(dma_channels))
		{
			dma_channels[i].inuse = 0;
			++i;
		}
	}
	else
	{
		return (E__BIT + E__DEVICE_NOT_FOUND);
	}

	return (E__OK);
}

/*****************************************************************************
 * enpmc_pex8619_dma_start:
 *
 * Initialise the DMA controller and start the transfer
 *
 * RETURNS: E__OK or error
 */
UINT32  enpmc_pex8619_dma_start
(
	ENPMC_PEX_8619_DMA_CTRL *p8619Dma,
	ENPMC_P8619_DMA_CHAN_INFO *pDmaChanInfo,
	addr_t from,
	addr_t to,
	INT32 size,
	INT32 dir,
	INT8 read_write,
	INT32 buf_xfer_flags,
	UINT32 *descr
)
{
	INT32 ret = E__OK;
	INT32 i = 0;
	UINT32 dDmaCtl = 0;
	UINT32 dNumChannels = 0;
	ENPMC_P8619_DMA_CHAN_INFO *chan = NULL;
	volatile addr_t LocalDmacsr;			/* pointer to config space register */
#ifdef DEBUG
	char buffer[0x80];
#endif

#ifdef DEBUG
	sprintf (buffer, "p8619Dma->dmacsr : 0x%#x\n", (UINT32) p8619Dma->dmacsr);
	vConsoleWrite (buffer);
#endif

	/* Configure the number of channels */
	dDmaCtl  = vReadDWord (p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL);

#ifdef DEBUG
	sprintf (buffer, "dDmaCtl 1 : %#x\n",dDmaCtl);
	vConsoleWrite (buffer);
#endif

	dNumChannels = dDmaCtl & (PLX_DMA_CHAN_MODE_BITS);
	vWriteDWord(p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL, dDmaCtl);
	switch (dNumChannels)
	{
		case 0:
			dNumChannels = 4;
			break;
		case 1:
			dNumChannels = 1;
			break;
		case 2:
			dNumChannels = 2;
			break;
		default:
			dNumChannels = 0;
	}

#ifdef DEBUG
	vConsoleWrite ("enpmc_pex8619_dma_start 1\n");
	sprintf(buffer, "dNumChannels : %#x\n", dNumChannels);
	vConsoleWrite (buffer);
#endif

	/*Initialize the DMA channels*/
	while (i < dNumChannels)
	{
		if (!dma_channels[i].inuse)
		{
			pDmaChanInfo = &dma_channels[i];
			pDmaChanInfo->channel = i;
			pDmaChanInfo->inuse = 1;
			pDmaChanInfo->magic = MAGIC;
			pDmaChanInfo->aborted = chan->paused = chan->done = 0;
			pDmaChanInfo->descriptor_base = 0;
			pDmaChanInfo->num_descriptors = 0;
			pDmaChanInfo->direction = 1;
			pDmaChanInfo->status = 0;
			break;
		}
		++i;
	}

#ifdef DEBUG
	sprintf (buffer, "i = %d\n", i);
	vConsoleWrite (buffer);
#endif

	if (i > 4)
	{
#ifdef DEBUG
		vConsoleWrite ("No Channels Available for USE\n");
#endif
		return (E__ALL_DMA_CHANNLES_IN_USE);
	}

	i = 0;


	// Disable on-chip mode, set to DMA Off-Chip Descriptor Mode
	i = vReadDWord (p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL);
	clrbit (i, 2);
	vWriteDWord (p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL, i);


#ifdef DEBUG
	vConsoleWrite ("enpmc_pex8619_dma_start 1\n");
	sprintf (buffer, "p8619Dma->dmacsr : %#x\n", (UINT32) p8619Dma->dmacsr);
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_GLOBAL_CTL : %#x\n", vReadDWord (p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL));
	vConsoleWrite (buffer);
#endif
	pDmaChanInfo->direction = read_write;

	if (buf_xfer_flags)
	{
#ifdef DEBUG
		vConsoleWrite ("\n\nDMA Block Mode\n\n");
#endif

		LocalDmacsr = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE);


		/* Populate the DMA Channel for the Source Address Lower Offset */
		vWriteDWord (LocalDmacsr + PLX_DMA_SRC_ADDR_LOW_OFF, (UINT32)from);

#ifdef DEBUG
		sprintf (buffer, "ChannelAddress = %#x\n", (UINT32) LocalDmacsr);
		vConsoleWrite (buffer);

		sprintf (buffer, "PLX_DMA_SRC_ADDR_LOW_OFF : %#x\n", vReadDWord (LocalDmacsr + PLX_DMA_SRC_ADDR_LOW_OFF));
		vConsoleWrite (buffer);
#endif

		/* Populate the DMA Channel for the Source Address Upper Offset */
		vWriteDWord (LocalDmacsr + PLX_DMA_SRC_ADDR_UPP_OFF, 0);

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_SRC_ADDR_UPP_OFF : %#x\n", vReadDWord (LocalDmacsr + PLX_DMA_SRC_ADDR_UPP_OFF));
		vConsoleWrite (buffer);
#endif

		/* Populate the DMA Channel for the Destination Address Lower Offset */
		vWriteDWord(LocalDmacsr + PLX_DMA_DST_ADDR_LOW_OFF, (UINT32)to);		/*0x8000000*/

#ifdef DEBUG
		sprintf (buffer, "from : %#x\n", (UINT32) from);
		vConsoleWrite (buffer);
		sprintf (buffer, "to : %#x\n", (UINT32) to);
		vConsoleWrite (buffer);
#endif

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_DST_ADDR_LOW_OFF : %#x\n", vReadDWord (LocalDmacsr + PLX_DMA_DST_ADDR_LOW_OFF));
		vConsoleWrite (buffer);
#endif

		/* Populate the DMA Channel for the Destination Address Upper Offset with zero */
		vWriteDWord (LocalDmacsr + PLX_DMA_DST_ADDR_UPP_OFF, 0);

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_DST_ADDR_UPP_OFF : %#x\n", vReadDWord (LocalDmacsr + PLX_DMA_DST_ADDR_UPP_OFF));
		vConsoleWrite (buffer);
#endif

		i = 0;
		//i = size | (1<<PLX_DMA_CHAN_VALID_BIT) | (1<<PLX_DMA_CHAN_DONE_INTR_ENA_BIT);

		i =  size |
			(1 << PLX_DMA_CHAN_VALID_BIT) | // Valid bit
		//	(1<<29) |  // Keep source address constant
		//	(1<<28) |  // Keep destination address constant
			(1 << PLX_DMA_CHAN_DONE_INTR_ENA_BIT);

			{
			UINT32 temp = vReadDWord (LocalDmacsr + PLX_DMA_CHAN_TRAN_SIZE_OFF);
			temp = temp & 0x08000000;
			i = i | temp;
		}

		/* Populate the current descriptor transfer size  */
		vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_TRAN_SIZE_OFF, i);



#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_CHAN_LAST_DESC_TRAN_SIZE_OFF : %#x\n", vReadDWord (LocalDmacsr + PLX_DMA_CHAN_TRAN_SIZE_OFF));
		vConsoleWrite (buffer);
#endif

		i = 0;
		i = vReadDWord (LocalDmacsr + PLX_DMA_CHAN_CTL_STA_OFF);

#ifdef DEBUG
			sprintf (buffer, "PLX_DMA_CHAN_CTL_STA_OFF : %#x\n", i);
			vConsoleWrite (buffer);
#endif

#if 1
		clrbit (i, PLX_DMA_CHAN_DESC_MODE_SEL_CTL_BIT);			// Set the DMA Block Mode

		clrbit (i, PLX_DMA_CHAN_COMP_STA_WRITE_BACK_CTL_BIT);	// Make sure descriptor write-back ([2]) is disabled
		//setbit (i, PLX_DMA_CHAN_COMP_STA_WRITE_BACK_CTL_BIT);	// Make sure descriptor write-back ([2]) is disabled


		clrbit (i, 31);		// Clear the DMA Channel DMA Header Loggin Valid Bit
		clrbit (i, 8);		// Clear the DMA Channel Descrptor Invalid Status
		clrbit (i, 9);		// Clear the DMA Channel Graceful Pause Done Status
		clrbit (i, 10);		// Clear the DMA Channel Abort Done Status
		clrbit (i, 11);		// Clear the DMA Channel Factory Test Only
		clrbit (i, 12);		// Clear the DMA Immediate Pause Done Status
		setbit (i, 16);		// Set the DMA Max paylod Size to 128 bytes
		clrbit (i, 17);		// Set the DMA Max paylod Size to 128 bytes
		clrbit (i, 18);		// Set the DMA Max paylod Size to 128 bytes

		setbit (i, PLX_DMA_CHAN_START_CTL_BIT);		// Start DMA Transfer
#endif

#ifdef DEBUG
		sprintf (buffer, "Value to be set for PLX_DMA_CHAN_CTL_STA_OFF is : %#x\n", i);
		vConsoleWrite (buffer);
#endif

		vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_CTL_STA_OFF, i);

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_CHAN_CTL_STA_OFF : %#x\n", vReadDWord (LocalDmacsr + PLX_DMA_CHAN_CTL_STA_OFF));
		vConsoleWrite (buffer);
#endif
	}
	else
	{
#ifdef DEBUG
		vConsoleWrite ("\n\nDMA Ring Discriptor Mode - Off Chip\n\n");
#endif

#if 0
		pDmaChanInfo->num_descriptors = enpmc_pex8619_prepare_sg_buffer (from, (addr_t) to, size, dir, pDmaChanInfo);
		if(pDmaChanInfo->num_descriptors < 1)
		{
			vConsoleWrite ("sg buffer failed");
			ret = E__NO_MEM;
			return (ret);
		}
#endif

		ret = enpmc_pex8619_prepare_sg_buffer (from, (addr_t) to, size, dir, pDmaChanInfo);
		if (ret != E__OK)
		{
			vConsoleWrite ("sg buffer failed");
			return ret;
		}


		LocalDmacsr = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE);

		/* clear dma channel addr registers as we will be using ring */
		vWriteDWord (LocalDmacsr + PLX_DMA_SRC_ADDR_LOW_OFF  , 0);
		vWriteDWord (LocalDmacsr + PLX_DMA_SRC_ADDR_UPP_OFF  , 0);
		vWriteDWord (LocalDmacsr + PLX_DMA_DST_ADDR_LOW_OFF  , 0);
		vWriteDWord (LocalDmacsr + PLX_DMA_DST_ADDR_UPP_OFF  , 0);
		vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_TRAN_SIZE_OFF, 0);

		/* write the descriptor ring address */
		 vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_DESC_RING_ADDR_LOW_OFF, (UINT32)(pDmaChanInfo->descriptor_base));
		/* higher 16 bits as 0 */
		 vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_DESC_RING_ADDR_UPP_OFF, 0);

		/* next descriptor as self */
		vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_NXT_DESC_RING_ADDR_LOW_OFF, (UINT32)(pDmaChanInfo->descriptor_base));

		/* Set the descriptor ring size */
		vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_DESC_RING_SIZE_OFF, (UINT32)(pDmaChanInfo->num_descriptors));

		/* current descriptor transfer size as 0 */
		vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_LAST_DESC_TRAN_SIZE_OFF, 0);

		i = 0;
		i = vReadDWord (LocalDmacsr + PLX_DMA_CHAN_INTR_CTL_STA_OFF);

		/* disable invalid-desc intr */
		clrbit (i, 1);

		vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_INTR_CTL_STA_OFF, i);

		/* Get the DMA Control / Status Register Value */
		i = 0;
		i = vReadDWord (LocalDmacsr + PLX_DMA_CHAN_CTL_STA_OFF);

		clrbit (i, 31);		// Clear the DMA Channel DMA Header Loggin Valid Bit
		clrbit (i, 8);		// Clear the DMA Channel Descrptor Invalid Status
		clrbit (i, 9);		// Clear the DMA Channel Graceful Pause Done Status
		clrbit (i, 10);		// Clear the DMA Channel Abort Done Status
		clrbit (i, 11);		// Clear the DMA Channel Factory Test Only
		clrbit (i, 12);		// Clear the DMA Immediate Pause Done Status

		/* disable completion writeback */
		clrbit (i, PLX_DMA_CHAN_COMP_STA_WRITE_BACK_CTL_BIT);

		setbit (i, PLX_DMA_CHAN_DESC_MODE_SEL_CTL_BIT); //Enable DMA Ring Descriptor Mode
		setbit (i, PLX_DMA_CHAN_RING_OP_MODE_CTL_BIT); // Enable Descriptor Ring Stops after the Descriptor's current round finishes.

		setbit (i, PLX_DMA_CHAN_START_CTL_BIT); // Start DMA Transfer

#ifdef DEBUG
		sprintf (buffer, "Value to be set for PLX_DMA_CHAN_CTL_STA_OFF is : %#x\n", i);
		vConsoleWrite (buffer);
#endif

		vWriteDWord (LocalDmacsr + PLX_DMA_CHAN_CTL_STA_OFF, i);

		#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_CHAN_CTL_STA_OFF : %#x\n", vReadDWord (LocalDmacsr + PLX_DMA_CHAN_CTL_STA_OFF));
		vConsoleWrite (buffer);
#endif
	}

#ifdef DEBUG
	vConsoleWrite ("dma started...");
#endif

	/* return the descriptor to the user */
	//ret = (((INT32)pDmaChanInfo) ^ MAGIC);

	 /* return the descriptor to the user */
	 *descr = (((INT32)pDmaChanInfo) ^ MAGIC);

#ifdef DEBUG
	sprintf (buffer, "MAGIC = %#x, pDmaChanInfo = %#x , ret = %#x\n", MAGIC, (UINT32) pDmaChanInfo, *descr);
	vConsoleWrite (buffer);

	sprintf (buffer, "Return from enpmc_pex8619_dma_start \n");
	vConsoleWrite (buffer);
#endif

	return ret;
}


/*****************************************************************************
 * enpmc_pex8619_prepare_sg_buffer:
 *
 * RETURNS: E__OK or error
 */
UINT32 enpmc_pex8619_prepare_sg_buffer
(
	addr_t from,
	addr_t to,
	INT32 count,
	INT32 dir,
	ENPMC_P8619_DMA_CHAN_INFO *pDmaChanInfo
)
{
	INT32 num_desc;
	INT32 remaining;
	INT32 i;
	INT32 ret = E__OK;
	INT32 len;
	UINT32 srcaddr;
	UINT32 dstaddr;
	volatile UINT32 *base_from = NULL;
	volatile UINT32 dBase;
	UINT32 dTempAddrs = 0;
#ifdef DEBUG
	char buffer[0x80];
#endif

	remaining = count;
	num_desc = 0;

	while (remaining)
	{
		if (remaining < 4096)
		{
			remaining = 0;
		}
		else
		{
			remaining -= 4096;
		}
		++num_desc;
	}
	//ret = num_desc;
	pDmaChanInfo->num_descriptors = num_desc;
	remaining = count;

#ifdef DEBUG
	sprintf (buffer, "Count  : %d\n", count);
	vConsoleWrite (buffer);
	sprintf (buffer, "num_desc : %d\n", num_desc);
	vConsoleWrite (buffer);
#endif

	/*Write a pattern into the DMA buffer*/
	mHandle = dGetPhysPtr (TEST_DMA_DESC, (num_desc * 16 * 16 ), &tPtr3, (void*)&dBase);
	if( mHandle == E__FAIL)
	{
		return (E__TEST_NO_MEM);
	}

	dTempAddrs = dBase;
	dTempAddrs = (dTempAddrs) & (0xFFFFFFC0);
	dBase = dTempAddrs;
	base_from = (UINT32*)dBase;

#ifdef DEBUG
	sprintf (buffer, "base_from : %#x\n", (UINT32) base_from);
	vConsoleWrite (buffer);
#endif

	pDmaChanInfo->descriptor_base = (UINT32*)TEST_DMA_DESC;
	srcaddr = (UINT32)from;
	dstaddr = (UINT32)to;

	for (i = 0; i < num_desc; ++i)
	{
		len = (remaining > 4096) ? 4096 : remaining;
		base_from[0] = len;
		remaining -= len;
		setbit (base_from[0], PLX_DMA_CHAN_VALID_BIT);
		clrbit (base_from[0], 27);
		/* enable dma done interrupt for the last descriptor */
		if (!remaining)
		{
#ifdef DEBUG
			vConsoleWrite ("enabling done bit for the last desc\n");
#endif
			setbit (base_from[0], PLX_DMA_CHAN_DONE_INTR_ENA_BIT);
		}

		base_from[1] = 0;
		base_from[2] = dstaddr;
		base_from[3] = srcaddr;
		base_from += 4;
		srcaddr += len;
		dstaddr += len;
	}

	return ret;
}

/*****************************************************************************
 * enpmc_pex8619_verify_dma_data:
 *
 * RETURNS: E__OK or error
 */
UINT32 enpmc_pex8619_verify_dma_data
(
	addr_t from,
	addr_t to,
	INT32 count,
	INT32 dir,
	ENPMC_P8619_DMA_CHAN_INFO *pDmaChanInfo
)
{
	// UINT32 srcaddr;
	UINT32 dstaddr;
	UINT32 ii;
	UINT32 dReturn = E__OK;
	UINT32 regVal = 0;
#ifdef DEBUG
	char buffer[80];
#endif

	// srcaddr = (UINT32)from;
	dstaddr = (UINT32)to;

	for (ii = 0 ; ii < count; ii += 4)
	{
		regVal = osReadDWord (dstaddr + ii);
		if (regVal == TESTPAT1)
		{
			dReturn = E__OK;
		}
		else
		{
#ifdef DEBUG
			sprintf (buffer, "Pos: %d regVal: 0x%x\n", ii, regVal);
			vConsoleWrite (buffer);
#endif
			dReturn = E__FAIL;
			return (dReturn);
		}
	}
	return (dReturn);
}


/*****************************************************************************
 * enpmc_pex8619_verify_dma_data:
 *
 * RETURNS: E__OK or error
 */
static INT32 enpmc_pex8619_dma_status
(
	ENPMC_PEX_8619_DMA_CTRL *p8619Dma,
	INT32 descriptor
)
{
	INT32 dRet;
	UINT32 dmaint = 0;
	UINT32 tm = 0;

#ifdef DEBUG
	char buffer[0x80];
#endif

	ENPMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;

	pDmaChanInfo = (ENPMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);

	/*If the chan is valid then, return the channel status*/
	if (enpmc_pex8619_is_valid_channel ((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("Invalid dma descriptor");
#endif
	}
	tm = 1024*10000;

	while ( !(getbit (dmaint, PLX_DMA_CHAN_DESC_DONE_INTR_STA_BIT)) && tm-- )
	{
		dmaint = vReadDWord (p8619Dma->dmacsr+PLX_DMA_CHAN0_SRC_ADDR
							 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
							 + PLX_DMA_CHAN_INTR_CTL_STA_OFF);

		if (getbit (dmaint, PLX_DMA_CHAN_DESC_DONE_INTR_STA_BIT))
		{
			setbit (pDmaChanInfo->status, BIT_DMA_DONE);
		}
	}

	pDmaChanInfo->done = pDmaChanInfo->status;

#ifdef DEBUG
	sprintf (buffer, "done: 0x%x\n", pDmaChanInfo->done);
	vConsoleWrite (buffer);
	sprintf (buffer, "dmaint: 0x%x\n", dmaint);
	vConsoleWrite (buffer);
#endif

	if (tm!=0)
	{
		dRet = pDmaChanInfo->status;

	}
	else
		dRet = E__FAIL;

	return (dRet);
}

/*****************************************************************************
 * enpmc_pex8619_is_valid_channel:
 *
 * RETURNS: E__OK or error
 */
static INT32 enpmc_pex8619_is_valid_channel(INT32 handle)
{
	INT32 i = 0;

	/*check the validity of the dma channel - handle*/
	while(i < array_size(dma_channels))
	{
		if ((addr_t)&dma_channels[i] == (addr_t)handle)
		{
#ifdef DEBUG
			vConsoleWrite("Valid channel\n");
#endif
			return (E__OK);
		}
		++i;
	}

#ifdef DEBUG
	vConsoleWrite ("Invalid channel\n");
#endif

	return(E__INVALID_DMA_CHANNEL);
}


/*****************************************************************************
 * wait_signal:
 *
 */
void wait_signal(volatile INT32 *flag, INT32 bit)
{

	UINT32 dTimeout = 1024 * 10000;

#ifdef DEBUG
	char buffer[0x80];
#endif

	/* waits till the any of the interested bits are set */
	while (dTimeout--)
	{
		if (*flag & bit)
		{
			/* got our signal...clear the bits and return */
			*flag &= ~bit;
			break;
		}
	}

#ifdef DEBUG
	sprintf (buffer, "dTimeout: %d \n", dTimeout);
	vConsoleWrite (buffer);
#endif
}



/*****************************************************************************
 * enpmc_pex8619_is_valid_channel:
 *
 * RETURNS: E__OK or error
 */
static INT32 enpmc_pex8619_free_dma (INT32 descriptor)
{
	ENPMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;

	pDmaChanInfo = (ENPMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);

	if (enpmc_pex8619_is_valid_channel ((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("Invalid dma descriptor\n");
#endif
		return (E__INVALID_DMA_DESCRIPTOR);
	}

	/*Free the DMA channel*/
	pDmaChanInfo->inuse = 0;

	return (E__OK);
}


/*****************************************************************************
 * enpmc_pex8619_is_valid_channel:
 *
 * RETURNS: E__OK or error
 */
static INT32 enpmc_pex8619_wait_dma
(
	ENPMC_PEX_8619_DMA_CTRL *p8619Dma,
	INT32 descriptor
)
{
	INT32 ret = E__OK;
	UINT32 i = 0;
	ENPMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;
#ifdef DEBUG
	char buffer[0x80];
#endif

#ifdef DEBUG
	vConsoleWrite("Entering into enpmc_pex8619_wait_dma\n");
#endif

	pDmaChanInfo = ( ENPMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);
	if (enpmc_pex8619_is_valid_channel((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("Invalid dma descriptor\n");
#endif
		ret = E__INVALID_DMA_DESCRIPTOR;
		return (ret);
	}


	/* see if dma is active */
	ret = enpmc_pex8619_dma_status (p8619Dma,descriptor);
	if (ret & BITMASK_DMA_ANY)
	{
#ifdef DEBUG
		vConsoleWrite ("dma already over\n");
#endif
		/* Clear the DMA Interrupt */
		i = vReadDWord (p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + pDmaChanInfo->channel + PLX_DMA_CHAN_INTR_CTL_STA_OFF);

		vWriteDWord (p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + pDmaChanInfo->channel + PLX_DMA_CHAN_INTR_CTL_STA_OFF, (UINT32)i);
		return (E__OK);
	}

#ifdef DEBUG
	 vConsoleWrite ("dma is active wait for it....\n");
#endif

	/* dma is active...wait for it */
	wait_signal (&pDmaChanInfo->done, BITMASK_DMA_ANY);
	/*ret = pDmaChanInfo->status;*/

#ifdef DEBUG
	vConsoleWrite ("got it\n");
#endif

#ifdef DEBUG
	vConsoleWrite ("Exiting from enpmc_pex8619_wait_dma\n");
	vConsoleWrite ("Dumping the Error Registers\n");

	sprintf (buffer, "PLX_DMA_SRC_ADDR_LOW_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_SRC_ADDR_LOW_OFF)));
	vConsoleWrite (buffer);


	sprintf (buffer, "PLX_DMA_SRC_ADDR_UPP_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_SRC_ADDR_UPP_OFF)));
	vConsoleWrite(buffer);


	sprintf (buffer, "PLX_DMA_DST_ADDR_LOW_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 +(pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_DST_ADDR_LOW_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_DST_ADDR_UPP_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_DST_ADDR_UPP_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_TRAN_SIZE_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_CHAN_TRAN_SIZE_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_DESC_RING_ADDR_LOW_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_CHAN_DESC_RING_ADDR_LOW_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_DESC_RING_ADDR_UPP_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_CHAN_DESC_RING_ADDR_UPP_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_NXT_DESC_RING_ADDR_LOW_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_CHAN_NXT_DESC_RING_ADDR_LOW_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_DESC_RING_SIZE_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_CHAN_DESC_RING_SIZE_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_DESC_LAST_ADDRESS_LOWER : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + 0x24)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_LAST_DESC_TRAN_SIZE_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_CHAN_LAST_DESC_TRAN_SIZE_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_CTL_STA_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_CHAN_CTL_STA_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_CHAN_INTR_CTL_STA_OFF : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_CHAN_INTR_CTL_STA_OFF)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_ERROR_HEADER_LOG_0 : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_ERROR_HEADER_LOG_0)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_ERROR_HEADER_LOG_1 : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_ERROR_HEADER_LOG_1)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_ERROR_HEADER_LOG_2 : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_ERROR_HEADER_LOG_2)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_DEV_ERROR_STAT : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_DEV_ERROR_STAT)));
	vConsoleWrite (buffer);

	sprintf (buffer, "PLX_DMA_MAX_OS_RW_REQ : %#x\n",
						vReadDWord ((p8619Dma->dmacsr
									 + PLX_DMA_CHAN0_SRC_ADDR
									 + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE)
									 + PLX_DMA_MAX_OS_RW_REQ)));
	vConsoleWrite (buffer);
#endif

	return (E__OK);
}


/*****************************************************************************
 * enpmc_pex8619_dma_test:
 *
 * RETURNS: E__OK or error
 */
TEST_INTERFACE (enpmc_pex8619_dma_test, "ENPMC DMA Test")
{
	UINT32 dRet;
	UINT32 descriptor;
	ENPMC_PEX_8619_DEV_CTRL s8619Dev;
	ENPMC_PEX_8619_DMA_CTRL s8619Dma;
	ENPMC_P8619_DMA_CHAN_INFO sDmaChanInfo;
	UINT32 bDmaInst,dInstance;
	UINT32 dDmaErrorStatus = 0;
	PCI_PFA pfa;
#ifdef DEBUG
		static char buffer[128];
#endif


	memset (&s8619Dev, 0, sizeof(s8619Dev));
	memset (&s8619Dma, 0, sizeof(s8619Dma));
	memset (&sDmaChanInfo, 0, sizeof(sDmaChanInfo));

	bDmaInst = 2;
	dInstance = 1;


#ifdef DEBUG
	sprintf (buffer, "Inside enpmc dma test\n");
	vConsoleWrite (buffer);
#endif

	dRet = enpmc_pex8619_init (&s8619Dev, dInstance, TEST_RAM_ADDR1, TEST_RAM_ADDR);

	if (dRet == (E__BIT + E__DEVICE_NOT_FOUND))
	{
		return dRet;
	}

	if (dRet == E__TEST_NO_MEM)
	{
		if (mHandle3 == E__FAIL)
		{
			return dRet;
		}

		if (mHandle1 == E__FAIL)
		{
	  	 	vFreePtr(mHandle3);
			return dRet;
		}

		if (mHandle2 == E__FAIL)
		{
	  	 	vFreePtr (mHandle3);
			vFreePtr (mHandle1);
			return dRet;
		}
	}

#ifdef DEBUG
	sprintf (buffer, "After Calling pex8619 Init\n");
	vConsoleWrite (buffer);
#endif

	if (dRet == E__OK)
		dRet = enpmc_pex8619_dma_init (&s8619Dma, &sDmaChanInfo,bDmaInst);

	if ((dRet == (E__BIT + E__DEVICE_NOT_FOUND)) || (dRet == E__TEST_NO_MEM))
	{
 	 	vFreePtr (mHandle3);
		vFreePtr (mHandle1);
		vFreePtr (mHandle2);
		return dRet;
	}

#ifdef DEBUG
	sprintf (buffer, "After Calling pex8619 dma Init\n");
	vConsoleWrite (buffer);
#endif

	dRet = enpmc_pex8619_dma_start (&s8619Dma,
									&sDmaChanInfo,
									(addr_t)TEST_RAM_ADDR1,
									(addr_t)TEST_RAM_ADDR,
									DMA_BUFF_SIZE,
									0, DMA_WRITE, 1, &descriptor );

	if (dRet != E__OK)
	{
		vFreePtr (mHandle3);
		vFreePtr (mHandle1);
		vFreePtr (mHandle2);
		vFreePtr (mHandle4);
		return dRet;
	}

#ifdef DEBUG
	sprintf (buffer, "After Calling pex8619 dma Start\n");
	vConsoleWrite (buffer);
#endif

	if (dRet == E__OK)
	{
		dRet = enpmc_pex8619_wait_dma (&s8619Dma, descriptor);
	}

	if (dRet != E__OK)
	{
		vFreePtr (mHandle3);
		vFreePtr (mHandle1);
		vFreePtr (mHandle2);
		vFreePtr (mHandle4);
		vFreePtr (mHandle);
		return dRet;
	}

	if (dRet == E__OK)
	{
		dRet = enpmc_pex8619_verify_dma_data (s8619Dev.ramMem1,
											  s8619Dev.ramMem,
											  DMA_BUFF_SIZE , 0, &sDmaChanInfo);
	}

	if (dRet != E__OK)
	{
		vFreePtr (mHandle3);
		vFreePtr (mHandle1);
		vFreePtr (mHandle2);
		vFreePtr (mHandle4);
		vFreePtr (mHandle);
		return dRet;
	}

	pfa = PCI_MAKE_PFA (s8619Dma.bus, s8619Dma.device, s8619Dma.function);

	dDmaErrorStatus = PCI_READ_DWORD (pfa, 0x70);
	if ((dDmaErrorStatus & 0x40000) == 0x40000)
	{
		dRet = E__DMA_FATAL_ERROR;
	}

	enpmc_pex8619_free_dma (descriptor);

	vFreePtr (mHandle3);
	vFreePtr (mHandle1);
	vFreePtr (mHandle2);
	vFreePtr (mHandle4);
	vFreePtr (mHandle);

	return dRet;
}

