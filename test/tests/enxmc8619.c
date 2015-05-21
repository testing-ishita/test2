/************************************************************************
 *                                                                      *
 *      Copyright 2010 Concurrent Technologies, all rights reserved.    *
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/enxmc8619.c,v 1.7 2015-03-10 09:58:29 hchalla Exp $
 * $Log: enxmc8619.c,v $
 * Revision 1.7  2015-03-10 09:58:29  hchalla
 * Changed dma Instance for XMC Site 2.
 *
 * Revision 1.6  2014-12-15 12:33:32  mgostling
 * Fix bug in enxmc_pex8619_dma_test.
 * Tidied up the code.
 *
 * Revision 1.5  2014-07-29 11:32:24  mgostling
 * Use common error codes from enxmc8619.h instead of defining them locally.
 *
 * Revision 1.4  2014-03-20 15:58:22  mgostling
 * Moved all GPIO tests into a seperate file
 *
 * Revision 1.3  2013-12-10 11:59:52  mgostling
 * Added additional tests for XMC and PMC GPIO signals passed to RTM module.
 * Added low level I2C bit-banging routines for GPIO tests.
 *
 * Revision 1.2  2013-10-08 07:13:38  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2011/03/22 13:54:34  hchalla
 * Initial version of EN XMC 8619 DMA support.
 *
 */
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
#include <devices/enxmc8619.h>
#include <private/cpu.h>

/* defines */

//#define DEBUG 1

#define DMA_WRITE 1
#define DMA_READ 0

#define  TESTPAT1 0x12345678L
#define  TEST_RAM_ADDR1  0x0CA00000L
#define  TEST_RAM_ADDR   0x0C900000L
#define  TEST_DMA_DESC   0x0C800000L

#define  DMA_BUFF_SIZE (0x100000 * 4)

#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))

#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))
#define osWriteDWord(regAddr,value) (*(UINT32*)(regAddr) = value)

#define vWriteByte(regAddr,value)	(*(UINT8*) (regAddr) = value)
#define vWriteWord(regAddr,value)   (*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

/* locals */

static ENXMC_P8619_DMA_CHAN_INFO dma_channels[MAX_DMA_CHANNELS];

static PTR48 tPtr3,tPtr1,tPtr2,tPtr3,tPtr4;
static UINT32 mHandle,mHandle1,mHandle2,mHandle3,mHandle4;


/* forward declarations */

UINT32  enxmc_pex8619_pre_init (void);

UINT32 enxmc_pex8619_init (ENXMC_PEX_8619_DEV_CTRL *p8619Dev,
						   UINT32 dInstance,
						   UINT32 dSrcPhysAddr,
						   UINT32 dDstPhysAddr);

UINT32 enxmc_pex8619_dma_init (ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
							   ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo,UINT32 bDmaInst);

UINT32  enxmc_pex8619_dma_start (ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
								 ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo,
								 addr_t from,
								 /*addr_t*/ UINT32 to,
								 INT32 size,
								 INT32 dir,
								 INT8 read_write,
								 INT32 buf_xfer_flags);

UINT32 enxmc_pex8619_prepare_sg_buffer (addr_t from,
										addr_t to,
										INT32 count,
										INT32 dir,
										ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo);

UINT32 enxmc_pex8619_verify_dma_data (addr_t from,
									  addr_t to,
									  INT32 count,
									  INT32 dir,
									  ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo);

static INT32 enxmc_pex8619_dma_status(ENXMC_PEX_8619_DMA_CTRL *p8619Dma, INT32 descriptor);
static INT32 enxmc_pex8619_is_valid_channel(INT32 handle);
void enxmc_wait_signal(volatile INT32 *flag, INT32 bit);

#ifdef DMA_CONTROL_ENABLE
static INT32 enxmc_pex8619_pause_dma(ENXMC_PEX_8619_DMA_CTRL *p8619Dma, INT32 descriptor);
static INT32 enxmc_pex8619_resume_dma(ENXMC_PEX_8619_DMA_CTRL *p8619Dma, INT32 descriptor);
static INT32 enxmc_pex8619_stop_dma(ENXMC_PEX_8619_DMA_CTRL *p8619Dma, INT32 descriptor);
#endif

static INT32 enxmc_pex8619_wait_dma( ENXMC_PEX_8619_DMA_CTRL *p8619Dma, INT32 descriptor);
UINT32 enxmc_pex8619_dma_test (void);
UINT32 enxmc2_pex8619_dma_test (void);
static INT32 enxmc_pex8619_free_dma (INT32 descriptor);


UINT32 enxmc_pex8619_init
(
	ENXMC_PEX_8619_DEV_CTRL *p8619Dev,
	UINT32 dInstance,
	UINT32 dSrcPhysAddr,
	UINT32 dDstPhysAddr
)
{
	PCI_PFA pfa;
	UINT32 dBarAddress;
	UINT32 dBar;
	UINT32 ii = 0;
#ifdef DEBUG
	char buffer[80];
#endif

	memset (&pfa, 0, sizeof(pfa));
	if (E__OK == iPciFindDeviceById (dInstance, 0x10B5, 0x8619, &pfa))
	{

		dBarAddress = PCI_READ_DWORD (pfa, 0x10);
	 	p8619Dev->bus = PCI_PFA_BUS(pfa);
 		p8619Dev->device = PCI_PFA_DEV(pfa);
 		p8619Dev->function = PCI_PFA_FUNC(pfa);

		mHandle3  = dGetPhysPtr (dBarAddress, 0x40000, &tPtr3, (void*)(&p8619Dev->csr));
		if ( mHandle3 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}

		// Setup BAR 2 NT Port Registers for DMA Buffer
		vWriteDWord (p8619Dev->csr + 0x11000 + 0xE8, 0xFF800000);

#ifdef DEBUG
		sprintf (buffer, "0xE8 : %x\n", vReadDWord (p8619Dev->csr + 0x11000 + 0xE8));
		vConsoleWrite (buffer);
#endif
		vWriteDWord (p8619Dev->csr + 0x11000 + 0xC3C, 0x500000);

		// Enter Requester ID Translation Lookup Table
		vWriteDWord (p8619Dev->csr + 0x10000 + 0xD94, 0x80000000);
		vWriteDWord (p8619Dev->csr + 0x11000 + 0xDB4, 0x00010001);

		//To enable the bridge from the NT Mode
		vWriteDWord (p8619Dev->csr + 0x1DC, (vReadDWord (p8619Dev->csr + 0x1DC) | 0x20000000));

		dBar = dPciReadReg (pfa, 0x18, REG_32);
		dBar = (dBar & 0xffffff00);

		// Create 2MB Buffer for DMA transfer
		mHandle1 = dGetPhysPtr (dSrcPhysAddr, DMA_BUFF_SIZE, &tPtr1, (void*)(&p8619Dev->ramMem1));

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
			osWriteDWord ((p8619Dev->ramMem1+ii), (UINT32)TESTPAT1);
		}

		mHandle2  = dGetPhysPtr (dDstPhysAddr, DMA_BUFF_SIZE, &tPtr2, (void*)(&p8619Dev->ramMem));
		if (mHandle2 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}

		pfa = PCI_MAKE_PFA(p8619Dev->bus, 0x0, 1);
		dBarAddress = PCI_READ_DWORD (pfa, 0x10);

	#ifdef DEBUG
		sprintf (buffer, "dBarAddress  : %x\n", dBarAddress);
		vConsoleWrite (buffer);
	#endif

#if 0
		mapRegSpace =  (UINT32*)sysMemGetPhysPtrLocked ((UINT64)dBarAddress, 0x100000);

		vWriteDWord (mapRegSpace + 0xDB4, 0x02010201);

		sprintf (buffer, " LUT Entry : %x\n", vReadDWord (mapRegSpace + 0xDB4));
		vConsoleWrite (buffer);
#endif

	}
	else
	{
		return (E__DEVICE_NOT_FOUND);
	}

	return (E__OK);
}

UINT32 enxmc_pex8619_idt_init
(
	ENXMC_PEX_8619_DEV_CTRL *p8619Dev,
	UINT32 dInstance
)
{
	PCI_PFA pfa;
#ifdef DEBUG
	char buffer[80];
#endif
	UINT32 dBarAddress, dDidVid;


	memset (&pfa, 0, sizeof(pfa));
	if (E__OK == iPciFindDeviceById (dInstance, 0x10B5, 0x8619, &pfa))
	{
	 	p8619Dev->bus = PCI_PFA_BUS(pfa);
 		p8619Dev->device = PCI_PFA_DEV(pfa);
 		p8619Dev->function = PCI_PFA_FUNC(pfa);

 		p8619Dev->bus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);
 		pfa = PCI_MAKE_PFA(p8619Dev->bus, p8619Dev->device, p8619Dev->function);
#ifdef DEBUG
		sprintf (buffer, "p8619Dev->bus :%d p8619Dev->device: %d p8619Dev->function:%d\n",
				 p8619Dev->bus, p8619Dev->device, p8619Dev->function);
		vConsoleWrite (buffer);
#endif
		dDidVid = PCI_READ_DWORD (pfa, 0x00);

		if (dDidVid == 0xffffffff)
		{
			return (E__DEVICE_NOT_FOUND);
		}

		PCI_WRITE_WORD (pfa, 0x04, 0x0007);
 		dBarAddress = PCI_READ_DWORD (pfa, 0x14);

#ifdef DEBUG
	sprintf (buffer, "dBarAddress  : %x\n", dBarAddress);
	vConsoleWrite (buffer);
#endif
		mHandle3  = dGetPhysPtr (dBarAddress, DMA_BUFF_SIZE, &tPtr3, (void*)(&p8619Dev->pciMem));
		if (mHandle3 == E__FAIL)
		{
			return (E__TEST_NO_MEM);
		}
	}
	else
	{
		return (E__DEVICE_NOT_FOUND);
	}

	return (E__OK);
}

UINT32 enxmc_pex8619_pre_init (void)
{
	UINT8	bTemp;
	PCI_PFA pfa;
	UINT32 dBarAddress;

	static PTR48 tPtr1;
	static volatile UINT32 mapRegSpace = 0;
#ifdef DEBUG
	char    buffer[64];
#endif


	/*PLX Switch  bus allocation */
#if 0
	pfa = PCI_MAKE_PFA(0, 0x1, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x2);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x5);

	PCI_WRITE_WORD (pfa, 0x20, 0xE000);
	PCI_WRITE_WORD (pfa, 0x22, 0xE450);

	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);

	pfa = PCI_MAKE_PFA(2, 0x0, 0);
    PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x2);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x3);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x4);
	PCI_WRITE_WORD (pfa, 0x20, 0xE000);
	PCI_WRITE_WORD (pfa, 0x22, 0xE440);
	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);

	pfa = PCI_MAKE_PFA(3, 0x0, 0);
	PCI_WRITE_DWORD (pfa, 0x10, 0xE4400000);
	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);
#endif

	pfa = PCI_MAKE_PFA(0, 0x6, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x2);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x6);
	PCI_WRITE_WORD (pfa, 0x20, 0xF000);
	PCI_WRITE_WORD (pfa, 0x22, 0xF450);
	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);

	pfa = PCI_MAKE_PFA(2, 0x0, 0);
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0x2);
	PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x3);
	PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x4);
	PCI_WRITE_WORD (pfa, 0x20, 0xF000);
	PCI_WRITE_WORD (pfa, 0x22, 0xF440);
	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);
	pfa = PCI_MAKE_PFA(3, 0x0, 0);
	PCI_WRITE_DWORD (pfa, 0x10, 0xF4400000);
	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);

	dBarAddress = PCI_READ_DWORD (pfa, 0x10);

#ifdef DEBUG
	sprintf (buffer, "dBarAddress  : %x\n", dBarAddress);
	vConsoleWrite (buffer);
#endif

	mHandle = dGetPhysPtr (dBarAddress, 0x10000, &tPtr1,  (void*)&(mapRegSpace));

	if (mHandle == E__FAIL)
	{
		return (E__TEST_NO_MEM);
	}

	// Setup BAR 2 NT Port Registers for DMA Buffer Transfer
	vWriteDWord (mapRegSpace + 0x11000 + 0xE8, 0xFFE00000);
#ifdef DEBUG
	sprintf (buffer, "0xE8 : %x\n", vReadDWord(mapRegSpace + 0x11000 + 0xE8));
	vConsoleWrite (buffer);
#endif
	vWriteDWord (mapRegSpace + 0x11000 + 0xC3C, 0x00);
#ifdef DEBUG
	sprintf(buffer, "0xC3C : %x\n",vReadDWord (mapRegSpace + 0x11000 + 0xC3C));
      vConsoleWrite(buffer);
#endif
	// Enter Requester ID Translation Lookup Table
	vWriteDWord (mapRegSpace + 0x10000 + 0xD94, 0x80000000);
	vWriteDWord (mapRegSpace + 0x11000 + 0xDB4 ,0x00010001);

	vWriteDWord (mapRegSpace + 0x11000 + 0xDC4, 0x02030203);

#ifdef DEBUG
	sprintf (buffer, " LUT Entry : %x\n",vReadDWord (mapRegSpace + 0x11000 + 0xDC4));
	vConsoleWrite (buffer);
	sprintf (buffer, " Control Register 0x1DC : %x\n", vReadDWord (mapRegSpace + 0x1DC));
	vConsoleWrite (buffer);
#endif

	vWriteDWord (mapRegSpace + 0x1DC, (vReadDWord (mapRegSpace + 0x1DC) | 0x20000000));
	vWriteDWord (mapRegSpace + 0x1000 + 0x1DC, (vReadDWord (mapRegSpace + 0x1000 + 0x1DC) | 0x20000000));
	vWriteDWord (mapRegSpace + 0x10000 + 0x1DC, (vReadDWord (mapRegSpace + 0x10000 + 0x1DC) | 0x20000000));
	vWriteDWord (mapRegSpace + 0x11000 + 0x1DC, (vReadDWord (mapRegSpace + 0x11000 + 0x1DC) | 0x20000000));
#ifdef DEBUG
	sprintf (buffer, " Control Register 0x1DC : %x\n", vReadDWord (mapRegSpace + 0x1DC));
	vConsoleWrite (buffer);
#endif

	vDelay (5000);
	vFreePtr (mHandle);
	return (E__OK);
}

UINT32 enxmc_pex8619_dma_init
(
	ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo,
	UINT32 bDmaInst
)
{
	PCI_PFA pfa;
	UINT32 dBarAddress;
	UINT32 dTemp;
	UINT32 i = 0, dVidDid = 0;

	memset (&pfa, 0, sizeof(pfa));
	if (E__OK == iPciFindDeviceById (bDmaInst, 0x10B5, 0x8619, &pfa))
	{
		dBarAddress = PCI_READ_DWORD (pfa, 0x10);
	 	p8619Dma->bus = PCI_PFA_BUS(pfa);
 		p8619Dma->device = PCI_PFA_DEV(pfa);
 		p8619Dma->function = PCI_PFA_FUNC(pfa);


 		dTemp = PCI_READ_DWORD (pfa, 0x04);
 		PCI_WRITE_DWORD (pfa, 0x04,(dTemp | 0x00000007));

 		mHandle4 = dGetPhysPtr (dBarAddress, 0x40000, &tPtr4, (void*)&(p8619Dma->dmacsr));

 		if(mHandle4 == E__FAIL)
 		{
 			return (E__TEST_NO_MEM);
 		}

		dVidDid = vReadDWord (p8619Dma->dmacsr);

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
			return (E__DEVICE_NOT_FOUND);
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
		return (E__DEVICE_NOT_FOUND);
	}

	return (E__OK);
}

UINT32 enxmc_pex8619_dma_start
(
	ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo,
	addr_t from,
	/*addr_t*/ UINT32 to,
	INT32 size,
	INT32 dir,
	INT8 read_write,
	INT32 buf_xfer_flags
)
{
	INT32 ret = 0;
	INT32 i = 0;
	addr_t dmaReg;
#ifdef DEBUG
	char buffer[0x80];
#endif
	UINT32 dDmaCtl = 0, dNumChannels = 0;
	ENXMC_P8619_DMA_CHAN_INFO *chan = NULL;

	/* Configure the number of channels */
	dDmaCtl  = vReadDWord(p8619Dma->dmacsr+PLX_DMA_GLOBAL_CTL);

#ifdef DEBUG
	sprintf (buffer, "dDmaCtl : %#x\n", dDmaCtl);
	vConsoleWrite (buffer);
#endif

	dNumChannels = dDmaCtl & (PLX_DMA_CHAN_MODE_BITS);

	vWriteDWord (p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL, dDmaCtl);
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
	vConsoleWrite ("enxmc_pex8619_dma_start 1\n");
	sprintf (buffer, "dNumChannels : %#x\n", dNumChannels);
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
			pDmaChanInfo->aborted = 0;
			pDmaChanInfo->descriptor_base = 0;
			pDmaChanInfo->num_descriptors = 0;
			pDmaChanInfo->direction = 1;
			pDmaChanInfo->status = 0;
			chan->paused = 0;
			chan->done = 0;
			break;
		}
		++i;
	}

	if (i > 4)
	{
#ifdef DEBUG
		vConsoleWrite ("No Channels Available for USE\n");
#endif
		return (E__ALL_DMA_CHANNLES_IN_USE);
	}

	i = 0;
	/* disable on-chip mode */
	i = vReadDWord (p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL);
	if (i & bitmask(2))
	{
		ret += vWriteDWord( p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL, i & ~(bitmask(2)));
	}

#ifdef DEBUG
	vConsoleWrite ("enxmc_pex8619_dma_start 1\n");
	sprintf (buffer, "p8619Dma->dmacsr : %#x\n", (UINT32)p8619Dma->dmacsr);
	vConsoleWrite (buffer);
	sprintf (buffer, "PLX_DMA_GLOBAL_CTL : %#x\n", vReadDWord (p8619Dma->dmacsr + PLX_DMA_GLOBAL_CTL));
	vConsoleWrite (buffer);
#endif

	pDmaChanInfo->direction = read_write;

	// calculate register base address for this channel
	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE);

	if (buf_xfer_flags)
	{
		/* clear dma channel addr registers as we will be using ring */
		vWriteDWord (dmaReg + PLX_DMA_SRC_ADDR_LOW_OFF, (UINT32)from);

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_SRC_ADDR_LOW_OFF : %#x\n", vReadDWord (dmaReg + PLX_DMA_SRC_ADDR_LOW_OFF));
		vConsoleWrite (buffer);
#endif
		vWriteDWord (dmaReg + PLX_DMA_SRC_ADDR_UPP_OFF, 0);

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_SRC_ADDR_UPP_OFF : %#x\n", vReadDWord (dmaReg + PLX_DMA_SRC_ADDR_UPP_OFF));
		vConsoleWrite (buffer);
#endif

		vWriteDWord (dmaReg + PLX_DMA_DST_ADDR_LOW_OFF, (UINT32)to); /*0x8000000*/

#ifdef DEBUG
		sprintf (buffer, "from : %#x\n", (UINT32)from);
		vConsoleWrite (buffer);
		sprintf (buffer, "to : %#x\n", to);
		vConsoleWrite (buffer);
#endif

#ifdef DEBUG
		sprintf(buffer, "PLX_DMA_DST_ADDR_LOW_OFF : %#x\n", vReadDWord (dmaReg + PLX_DMA_DST_ADDR_LOW_OFF));
		vConsoleWrite(buffer);
#endif

		vWriteDWord(dmaReg + PLX_DMA_DST_ADDR_UPP_OFF, 0);

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_DST_ADDR_UPP_OFF : %#x\n", vReadDWord (dmaReg + PLX_DMA_DST_ADDR_UPP_OFF));
		vConsoleWrite (buffer);
#endif

		i = 0;
		i = size | (1 << PLX_DMA_CHAN_VALID_BIT) | (1 << PLX_DMA_CHAN_DONE_INTR_ENA_BIT);
		/* current descriptor transfer size as 0 */

		vWriteDWord (dmaReg + PLX_DMA_CHAN_TRAN_SIZE_OFF, i);

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_CHAN_LAST_DESC_TRAN_SIZE_OFF : %#x\n", vReadDWord (dmaReg + PLX_DMA_CHAN_TRAN_SIZE_OFF));
		vConsoleWrite (buffer);
#endif

		i = 0;
		i = vReadDWord (dmaReg + PLX_DMA_CHAN_INTR_CTL_STA_OFF);

		setbit (i, PLX_DMA_CHAN_GRACE_PAUSE_CTL_BIT);

		/* disable invalid-desc intr */
		clrbit (i, PLX_DMA_CHAN_ABORT_CTL_BIT);

		/* enable done, graceful pause & immediate pause interrupt */
		setbit (i, PLX_DMA_CHAN_START_CTL_BIT);
		setbit (i, PLX_DMA_CHAN_DESC_MODE_SEL_CTL_BIT);
		setbit (i, PLX_DMA_CHAN_RING_OP_MODE_CTL_BIT);

		vWriteDWord (dmaReg + PLX_DMA_CHAN_INTR_CTL_STA_OFF, i);

#ifdef DEBUG
		sprintf (buffer, "PLX_DMA_CHAN_INTR_CTL_STA_OFF : %#x\n", vReadDWord (dmaReg + PLX_DMA_CHAN_INTR_CTL_STA_OFF));
		 vConsoleWrite (buffer);
#endif


		i = 0;
		/* clear dma channel status/control bits except txfer size & poll period  	*/

		i = vReadDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF);

		/* enable completion writeback */
		//change
		setbit (i, PLX_DMA_CHAN_COMP_STA_WRITE_BACK_CTL_BIT);
			/* set ring mode */
		clrbit (i, PLX_DMA_CHAN_DESC_MODE_SEL_CTL_BIT);
		/* set start dma bit */
		setbit (i, PLX_DMA_CHAN_START_CTL_BIT);
		/* stop ring when done */
		clrbit (i, PLX_DMA_CHAN_RING_OP_MODE_CTL_BIT);
		/* set max txfr size */
		setbit (i, PLX_DMA_CHAN_TRAN_SIZE_CTL_BIT);
	}
	else
	{
		pDmaChanInfo->num_descriptors =
			enxmc_pex8619_prepare_sg_buffer (from, (addr_t) to, size, dir, pDmaChanInfo);

		if (pDmaChanInfo->num_descriptors < 1)
		{
			vConsoleWrite ("sg buffer failed");
			ret = E__NO_MEM;
			return (ret);
		}

		/* clear dma channel addr registers as we will be using ring */
		vWriteDWord (dmaReg + PLX_DMA_SRC_ADDR_LOW_OFF, 0);
		vWriteDWord (dmaReg + PLX_DMA_SRC_ADDR_UPP_OFF, 0);
		vWriteDWord (dmaReg + PLX_DMA_DST_ADDR_LOW_OFF, 0);
		vWriteDWord (dmaReg + PLX_DMA_DST_ADDR_UPP_OFF, 0);

		/* clear the active interrupts for this channel */
		vWriteDWord (p8619Dma->dmacsr + 0x0200 + (pDmaChanInfo->channel * 0x0100) + 0x38, 0x80000000);

		/* write the ring address */
		 vWriteDWord (dmaReg + PLX_DMA_CHAN_DESC_RING_ADDR_LOW_OFF, (UINT32)pDmaChanInfo->descriptor_base);

		 /* higher 16 bits as 0 */
		vWriteDWord (dmaReg + PLX_DMA_CHAN_DESC_RING_ADDR_UPP_OFF, 0);

		/* next descriptor as self */
		vWriteDWord (dmaReg + PLX_DMA_CHAN_NXT_DESC_RING_ADDR_LOW_OFF, (UINT32)pDmaChanInfo->descriptor_base);

		/* ring size */
		vWriteDWord (dmaReg + PLX_DMA_CHAN_DESC_RING_SIZE_OFF, (UINT32)pDmaChanInfo->num_descriptors);

		/* current descriptor transfer size as 0 */
		vWriteDWord (dmaReg + PLX_DMA_CHAN_LAST_DESC_TRAN_SIZE_OFF, 0);

		i = 0;
		i = vReadDWord (dmaReg + PLX_DMA_CHAN_INTR_CTL_STA_OFF);

		setbit (i, PLX_DMA_CHAN_GRACE_PAUSE_CTL_BIT);

		/* disable invalid-desc intr */
		clrbit (i, PLX_DMA_CHAN_ABORT_CTL_BIT);

		/* enable done, graceful pause & immediate pause interrupt */
		setbit (i, PLX_DMA_CHAN_START_CTL_BIT);
		setbit (i, PLX_DMA_CHAN_DESC_MODE_SEL_CTL_BIT);
		setbit (i, PLX_DMA_CHAN_RING_OP_MODE_CTL_BIT);

		vWriteDWord (dmaReg + PLX_DMA_CHAN_INTR_CTL_STA_OFF, i);

		i = 0;
		/* clear dma channel status/control bits except txfer size & poll period  	*/
		i = vReadDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF);

		/* enable completion writeback */
		setbit (i, PLX_DMA_CHAN_COMP_STA_WRITE_BACK_CTL_BIT);
		/* set ring mode */
		setbit (i, PLX_DMA_CHAN_DESC_MODE_SEL_CTL_BIT);
		/* set start dma bit */
		setbit (i, PLX_DMA_CHAN_START_CTL_BIT);
		/* stop ring when done */
		setbit (i, PLX_DMA_CHAN_RING_OP_MODE_CTL_BIT);
		/* set max txfr size */
		setbit (i, PLX_DMA_CHAN_TRAN_SIZE_CTL_BIT);
	}

#ifdef DEBUG
	vConsoleWrite("dma started...");
#endif

	vWriteDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF, i);

#ifdef DEBUG
	sprintf (buffer, "PLX_DMA_CHAN_CTL_STA_OFF : %#x\n",vReadDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF));
	vConsoleWrite (buffer);
#endif

	/* return the descriptor to the user */
	ret = (((INT32)pDmaChanInfo) ^ MAGIC);

	return (ret);
}


UINT32 enxmc_pex8619_prepare_sg_buffer
(
	addr_t from,
	addr_t to,
	INT32 count,
	INT32 dir,
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo
)
{
	INT32 num_desc, remaining, i, ret = E__OK, len;
	UINT32 srcaddr, dstaddr;
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
		if(remaining < 4096)
		{
			remaining = 0;
		}
		else
		{
			remaining -= 4096;
		}
		++num_desc;
	}
	ret = num_desc;
	remaining = count;

#ifdef DEBUG
	sprintf (buffer, "num_desc : %d\n", num_desc);
    vConsoleWrite (buffer);
#endif

	/*Write a pattern into the DMA buffer*/
	mHandle = dGetPhysPtr (TEST_DMA_DESC, (num_desc * 16 * 16 ), &tPtr3, (void*)&dBase);

	if (mHandle == E__FAIL)
	{
		return (E__TEST_NO_MEM);
	}

	dTempAddrs = dBase;
	dTempAddrs = (dTempAddrs) & (0xFFFFFFC0);
	dBase = dTempAddrs;
	base_from = (UINT32*)dBase;

#ifdef DEBUG
	sprintf (buffer, "base_from : %#x\n", (UINT32)base_from);
    vConsoleWrite(buffer);
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
		srcaddr+=len;
		dstaddr+=len;
	}

	return (ret);
}

UINT32 enxmc_pex8619_verify_dma_data
(
	addr_t from,
	addr_t to,
	INT32 count,
	INT32 dir,
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo
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

	for (ii = 0 ; ii < count ; ii += 4)
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


static INT32 enxmc_pex8619_dma_status
(
	ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
	INT32 descriptor
)
{
	INT32 dRet;
	UINT32 dmaint = 0;
	UINT32 tm = 0;
	addr_t dmaReg;

#ifdef DEBUG
	char buffer[0x80];
#endif

	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;

	pDmaChanInfo = (ENXMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);

	/*If the chan is valid then, return the channel status*/
	if (enxmc_pex8619_is_valid_channel ((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("Invalid dma descriptor");
#endif
	}
	tm = 1024 * 10000;

	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE);

	while ( !(getbit (dmaint, PLX_DMA_CHAN_DESC_DONE_INTR_STA_BIT)) && tm-- )
	{
		dmaint = vReadDWord (dmaReg + PLX_DMA_CHAN_INTR_CTL_STA_OFF);

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

static INT32 enxmc_pex8619_is_valid_channel(INT32 handle)
{
	INT32 i = 0;

	/*check the validity of the dma channel - handle*/
	while (i < array_size(dma_channels))
	{
		if ((addr_t)&dma_channels[i] == (addr_t)handle)
		{
#ifdef DEBUG
				vConsoleWrite("Valid channel\n");
#endif
				return(E__OK);
		}
		++i;
	}
#ifdef DEBUG
	vConsoleWrite("Invalid channel\n");
#endif
	return(E__INVALID_DMA_CHANNEL);
}

#ifdef DMA_CONTROL_ENABLE
static INT32 enxmc_pex8619_pause_dma
(
	ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
	INT32 descriptor
)
{
	addr_t dmaReg;
	INT32 dRet, i = 0;
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;

	pDmaChanInfo = (ENXMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);
	if (enxmc_pex8619_is_valid_channel ((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("Invalid dma descriptor");
#endif
		return (E__INVALID_DMA_DESCRIPTOR);
	}

	pDmaChanInfo->paused = 1;

// ???	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE);
	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + pDmaChanInfo->channel;

	i = vReadDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF);

	i |= bitmask(0);

	vWriteDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF, (UINT32)i);

	/* wait for dma to pause */
#ifdef DEBUG
	vConsoleWrite ("Waiting for dma to pause");
#endif
	enxmc_wait_signal (&pDmaChanInfo->done, BITMASK_DMA_ANY);

 	return (dRet);
}

static INT32  enxmc_pex8619_resume_dma
(
	ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
	INT32 descriptor
)
{
	addr_t dmaReg;
	INT32 dRet, i = 0;
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;

	pDmaChanInfo = ( ENXMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);
	if (enxmc_pex8619_is_valid_channel ((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("Invalid dma descriptor");
#endif
		return (E__INVALID_DMA_DESCRIPTOR);
	}

// ???	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE);
	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + pDmaChanInfo->channel;

	i = vReadDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF);
	i &= ~(bitmask(0));
	/*Resume the DMA*/
	vWriteDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF, (UINT32)i);

	return dRet;
}

static INT32 enxmc_pex8619_stop_dma
(
	ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
	INT32 descriptor
)
{
	addr_t dmaReg;
	INT32 dRet, i = 0;
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;

	pDmaChanInfo = (ENXMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);

	if(enxmc_pex8619_is_valid_channel((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("Invalid dma descriptor\n");
#endif
		return (E__INVALID_DMA_DESCRIPTOR);
	}

	pDmaChanInfo->aborted = 1;

// ???	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE);
	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + pDmaChanInfo->channel;

	i = vReadDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF);
	i |= bitmask(1);
	vWriteDWord (dmaReg + PLX_DMA_CHAN_CTL_STA_OFF, (UINT32)i);

	/* wait for dma to abort */
#ifdef DEBUG
	vConsoleWrite ("Waiting for dma to abort\n");
#endif
	enxmc_wait_signal (&pDmaChanInfo->done, BITMASK_DMA_ANY);

	return (dRet);
}

#endif

void enxmc_wait_signal(volatile INT32 *flag, INT32 bit)
{

	UINT32 dTimeout = 1024*10000;

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



static INT32 enxmc_pex8619_free_dma (INT32 descriptor)
{
	INT32 dRet;
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;

	pDmaChanInfo = (ENXMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);

	if (enxmc_pex8619_is_valid_channel ((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite ("Invalid dma descriptor\n");
#endif
		return (E__INVALID_DMA_DESCRIPTOR);
	}
	dRet = E__OK;

	/*Free the DMA channel*/
	pDmaChanInfo->inuse = 0;

	return (dRet);
}

static INT32 enxmc_pex8619_wait_dma
(
	ENXMC_PEX_8619_DMA_CTRL *p8619Dma,
	INT32 descriptor
)
{
	addr_t dmaReg;
	INT32 ret = E__OK;
	UINT32 i = 0;
	ENXMC_P8619_DMA_CHAN_INFO *pDmaChanInfo;

	pDmaChanInfo = (ENXMC_P8619_DMA_CHAN_INFO *)(descriptor ^ MAGIC);

	if (enxmc_pex8619_is_valid_channel((INT32)pDmaChanInfo) != E__OK)
	{
#ifdef DEBUG
		vConsoleWrite("Invalid dma descriptor\n");
#endif
		ret = E__INVALID_DMA_DESCRIPTOR;
		return (ret);
	}

	/* see if dma is active */
	ret = enxmc_pex8619_dma_status (p8619Dma, descriptor);
	if (ret & BITMASK_DMA_ANY)
	{
#ifdef DEBUG
		vConsoleWrite("dma already over\n");
#endif

// ???	dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + (pDmaChanInfo->channel * PLX_DMA_CHAN_SIZE);
		dmaReg = p8619Dma->dmacsr + PLX_DMA_CHAN0_SRC_ADDR + pDmaChanInfo->channel;

		/* Clear the DMA Interrupt */
		i = vReadDWord (dmaReg + PLX_DMA_CHAN_INTR_CTL_STA_OFF);

		vWriteDWord (dmaReg + PLX_DMA_CHAN_INTR_CTL_STA_OFF, (UINT32)i);
		return (E__OK);
	}

	/* dma is active...wait for it */
	enxmc_wait_signal (&pDmaChanInfo->done, BITMASK_DMA_ANY);
	/*ret = pDmaChanInfo->status;*/
	return (E__OK);
}

TEST_INTERFACE (enxmc_pex8619_dma_test, "ENXMC DMA Test 1")
{
	UINT32 dRet;
	UINT32 descriptor;
	ENXMC_PEX_8619_DEV_CTRL s8619Dev;
	ENXMC_PEX_8619_DMA_CTRL s8619Dma;
	ENXMC_P8619_DMA_CHAN_INFO sDmaChanInfo;
	UINT32 bDmaInst,dInstance;
	UINT32 dDmaErrorStatus = 0;
	PCI_PFA pfa;

	memset (&s8619Dev, 0, sizeof(s8619Dev));
	memset (&s8619Dma, 0, sizeof(s8619Dma));
	memset (&sDmaChanInfo, 0, sizeof(sDmaChanInfo));

	bDmaInst = 2;
	dInstance = 1;

	dRet = enxmc_pex8619_init (&s8619Dev, dInstance, TEST_RAM_ADDR1, TEST_RAM_ADDR);

	if (dRet == E__OK)
	{
		dRet = enxmc_pex8619_dma_init (&s8619Dma, &sDmaChanInfo, bDmaInst);
	}

	descriptor = enxmc_pex8619_dma_start (&s8619Dma,
											&sDmaChanInfo,
											(addr_t)TEST_RAM_ADDR1,
											(UINT32)TEST_RAM_ADDR,
											DMA_BUFF_SIZE,
											0,
											DMA_WRITE,
											1);

	if (dRet == E__OK)
	{
		dRet = enxmc_pex8619_wait_dma (&s8619Dma,descriptor);
	}

	if (dRet == E__OK)
	{
		dRet = enxmc_pex8619_verify_dma_data (s8619Dev.ramMem1,
											  s8619Dev.ramMem,
											  DMA_BUFF_SIZE,
											  0,
											  &sDmaChanInfo);
	}

	pfa = PCI_MAKE_PFA(s8619Dma.bus, s8619Dma.device, s8619Dma.function);

	dDmaErrorStatus = PCI_READ_DWORD (pfa, 0x70);
	if ( (dDmaErrorStatus & 0x40000) == 0x40000)
	{
		return (E__DMA_FATAL_ERROR);
	}

	enxmc_pex8619_free_dma (descriptor);

#ifdef DMA_CONTROL_ENABLE
    enxmc_pex8619_pause_dma (&s8619Dma,descriptor);
    enxmc_pex8619_resume_dma (&s8619Dma,descriptor);
    enxmc_pex8619_stop_dma (&s8619Dma,descriptor);
#endif

    vFreePtr (mHandle4);
    vFreePtr (mHandle);
    vFreePtr (mHandle2);
    vFreePtr (mHandle1);
    vFreePtr (mHandle3);

	return (dRet);
}

TEST_INTERFACE (enxmc2_pex8619_dma_test, "ENXMC DMA Test 2")
{
	UINT32 dRet;
	UINT32 descriptor;
	ENXMC_PEX_8619_DEV_CTRL s8619Dev;
	ENXMC_PEX_8619_DMA_CTRL s8619Dma;
	ENXMC_P8619_DMA_CHAN_INFO sDmaChanInfo;
	UINT32 dDmaErrorStatus = 0;
	UINT32 bDmaInst=0,dInstance = 0;
	PCI_PFA pfa;



	memset (&s8619Dev, 0, sizeof(s8619Dev));
	memset (&s8619Dma, 0, sizeof(s8619Dma));
	memset (&sDmaChanInfo, 0, sizeof(sDmaChanInfo));

	//dInstance = 4;
	//bDmaInst = 5;

	dInstance = 7;
	bDmaInst = 8;

	dRet = enxmc_pex8619_init (&s8619Dev, dInstance, TEST_RAM_ADDR1, TEST_RAM_ADDR);
	if (dRet == E__OK)
	{
		dRet = enxmc_pex8619_dma_init (&s8619Dma, &sDmaChanInfo, bDmaInst);
	}

	if (dRet == E__OK)
	{
		descriptor = enxmc_pex8619_dma_start (&s8619Dma,
												&sDmaChanInfo,
												(addr_t)TEST_RAM_ADDR1,
												(UINT32)TEST_RAM_ADDR,
												DMA_BUFF_SIZE,
												0,
												DMA_WRITE,
												1);
	}
	if (dRet == E__OK)
	{
		dRet = enxmc_pex8619_wait_dma (&s8619Dma, descriptor);
	}

	if ((dRet ==E__OK))
	{
    	dRet = enxmc_pex8619_verify_dma_data (s8619Dev.ramMem1,
											  s8619Dev.ramMem ,
											  DMA_BUFF_SIZE,
											  0,
											  &sDmaChanInfo);
	}

	pfa = PCI_MAKE_PFA(s8619Dma.bus,s8619Dma.device,s8619Dma.function);

	dDmaErrorStatus = PCI_READ_DWORD (pfa, 0x70);

	if ((dDmaErrorStatus & 0x40000) == 0x40000)
	{
	    return (E__DMA_FATAL_ERROR);
	}

    enxmc_pex8619_free_dma (descriptor);

#ifdef DMA_CONTROL_ENABLE
    enxmc_pex8619_pause_dma (&s8619Dma,descriptor);
    enxmc_pex8619_resume_dma (&s8619Dma,descriptor);
    enxmc_pex8619_stop_dma (&s8619Dma,descriptor);
#endif
	vFreePtr (mHandle4);
	vFreePtr (mHandle);
	vFreePtr (mHandle2);
	vFreePtr (mHandle1);
	vFreePtr (mHandle3);


	return (dRet);
}



TEST_INTERFACE (enxmc_vxs_pex8619_loopback, "ENXMC VXS PCIe Loopback")
{
	UINT32 dRet = E__OK,ii = 0,temp = 0;
	ENXMC_PEX_8619_DEV_CTRL s8619Dev;
	UINT32 dInstance;


	memset (&s8619Dev, 0, sizeof(s8619Dev));

	dInstance = 3;

	dRet = enxmc_pex8619_idt_init (&s8619Dev,dInstance);

	if (dRet == E__OK)
	{
		 sysCacheDisable();
		 for (ii = 0 ; ii < DMA_BUFF_SIZE; ii += 4)
		 {
			 if (ii % (0x40000) == 0)
			 {
				 vConsoleWrite(".");
			 }

			 temp = ((ii << 24) | (ii << 16) | (ii << 8) | ii);
			 osWriteDWord ((s8619Dev.pciMem+ii), (UINT32)temp);
			  if (temp != osReadDWord(s8619Dev.pciMem+ii) )
			  {
				 dRet = (E__ENXMC_VXSPCIE_LOOPBACK_DATA_ERROR);
				 break;
			  }
		 }
		 vConsoleWrite ("\n");
		 sysCacheEnable();
	}

	vFreePtr(mHandle3);

	return (dRet);
}

