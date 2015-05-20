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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/enpmc8619.h,v 1.1 2015-02-03 12:07:28 mgostling Exp $
 * $Log: enpmc8619.h,v $
 * Revision 1.1  2015-02-03 12:07:28  mgostling
 * Added suppport for ENPMC_OO1
 *
 *
 * Initial version of EN PMC 8619 DMA support.
 *
*/

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>


#define E__TEST_NO_MEM									E__BIT  + 0x02
#define E__NO_MEM										E__BIT  + 0x03
#define E__DMA_FATAL_ERROR								E__BIT  + 0x04
#define E__ALL_DMA_CHANNLES_IN_USE						E__BIT  + 0x05
#define E__INVALID_DMA_DESCRIPTOR						E__BIT  + 0x06
#define E__INVALID_DMA_CHANNEL							E__BIT  + 0x07
#define E__ENPMC_VXSPCIE_LOOPBACK_DATA_ERROR			E__BIT  + 0x08

/* some useful macros */

#define array_size(x)			(sizeof(x) / sizeof(x[0]))
#define offset(m, s)			(&((typeof(s) *)0)->m)
#define setbit(x, y)			((x) = ((x) | (1 << (y))))
#define clrbit(x, y)			((x) = ((x) & ~(1 << (y))))
#define getbit(x, y)			(((x) & (1 << (y))) >> (y))
#define bitmask(a)				(1 << (a))
#define testbit(n, a)			(n & bitmask(a))
#define valid_addr(a)			(((unsigned long)a) && ~((unsigned long)a))

/*
	Every window has two ends - local & remote. The local end of a window maps to a local buffer
	while the remote end of a window is mapped to buffer on the remote end. There are maximum 4
	windows numbered 0 to 3.
*/
#define LOCAL											0
#define REMOTE											1

#define MAX_WIN_ID										7

#define WIN_ID(end, window)		((window * 2 + end) << 12)

#define MAX_BACKPLANE_DEVICES							8
#define NUM_WINDOWS										4
#define MAGIC											0xc001f00d

#define DEBUG_UART										0
#define USE_HIGHMEM										1
#define USE_BIOSMEM										0

#define MAX_DMA_CHANNELS								4

/*defines for the DMA transfer*/

#define BIT_DMA_DONE									0
#define BIT_DMA_ERROR									1
#define BIT_DMA_INVALID									2
#define BIT_DMA_ABORT									3
#define BIT_DMA_PAUSE									4
#define BIT_DMA_NOTUSED									5
#define BITMASK_DMA_ANY									~0


/*PLX specific defines*/

#define PEX8619_NUM_PORTS								16
#define PEX8619_VENDOR									0x10b5
#define PEX8619_DEVICE									0x8619
#define PEX8619_NT_CLASS								0x068000
#define PEX8619_NONNT_CLASS								0x060400
#define PEX8619_NUM_DMA_CHANNELS						4
#define PEX8619_NUM_DOORBELLS							16
#define PEX8619_NUM_MBOX								8

#define PEX_MIN_MBOX_SMART_MODE							4

#define VIRTUAL_PORT									16
#define LINK_PORT										17

/*PLX register defines*/

#define PLX_PORT_REGS_SIZE								0x1000
#define PLX_DEV_CMD_STA									0x004
#define PLX_DEV_CTL_STA									0x070
#define PLX_DEV_LINK_CAP								0x74
#define PLX_DEV_LINK_STA								0x7A		// 16 bit register
#define PLX_VIRT_BAR2_SETUP								0x0d4
#define PLX_LINK_BAR2_SETUP								0x0e8
#define PLX_DEV_EEP_STA_CTL								0x0260
#define PLX_DEV_EEP_BUF									0x0264
#define PLX_GPIO1_DIR_CTL								0x062C
#define PLX_GPIO2_DIR_CTL								0x0630
#define PLX_GPIO_DEBOUNCE_CTL							0x0638
#define PLX_GPIO1_INPUT_DATA							0x063C		// 16 bit register
#define PLX_GPIO2_INPUT_DATA							0x0640		// 16 bit register
#define PLX_GPIO1_OUTPUT_DATA							0x0644		// 16 bit register
#define PLX_GPIO2_OUTPUT_DATA							0x0648		// 16 bit register
#define PLX_VIRT_IRQ_MASK_CLR							0x0c58
#define PLX_DEV_MEM_BAR2_ADDR_TRAN_LOW					0x0c3c
#define PLX_VIRT_IRQ_SET								0x0c4c
#define PLX_VIRT_IRQ_CLR								0x0c50
#define PLX_LINK_IRQ_SET								0x0c5c
#define PLX_LINK_IRQ_CLR								0x0c60
#define PLX_LINK_IRQ_MASK_CLR							0x0c68
#define PLX_NTPORT_SCRATCH0								0x0c6c
#define PLX_VIRT_REQID_LUT								0x0d94
#define PLX_LINK_REQID_LUT								0x0db4
#define PLX_HEADER_LOG1									0x0fd4

#define PLX_DMA_GLOBAL_CTL								0x01fc
#define PLX_DMA_CHAN0_SRC_ADDR							0x0200
#define PLX_DMA_CHAN_SIZE								0x0100
#define PLX_DMA_SRC_ADDR_LOW_OFF						0x00
#define PLX_DMA_SRC_ADDR_UPP_OFF						0x04
#define PLX_DMA_DST_ADDR_LOW_OFF						0x08
#define PLX_DMA_DST_ADDR_UPP_OFF						0x0c
#define PLX_DMA_CHAN_TRAN_SIZE_OFF						0x10
#define PLX_DMA_CHAN_DESC_RING_ADDR_LOW_OFF				0x14
#define PLX_DMA_CHAN_DESC_RING_ADDR_UPP_OFF				0x18
#define PLX_DMA_CHAN_NXT_DESC_RING_ADDR_LOW_OFF			0x1c
#define PLX_DMA_CHAN_DESC_RING_SIZE_OFF					0x20
#define PLX_DMA_CHAN_LAST_DESC_TRAN_SIZE_OFF			0x28
#define PLX_DMA_CHAN_CTL_STA_OFF						0x38
#define PLX_DMA_CHAN_INTR_CTL_STA_OFF					0x3c

#define PLX_DMA_CHAN_GRACE_PAUSE_CTL_BIT				0
#define PLX_DMA_CHAN_ABORT_CTL_BIT						1
#define PLX_DMA_CHAN_COMP_STA_WRITE_BACK_CTL_BIT		2
#define PLX_DMA_CHAN_START_CTL_BIT						3
#define PLX_DMA_CHAN_DESC_MODE_SEL_CTL_BIT				4
#define PLX_DMA_CHAN_RING_OP_MODE_CTL_BIT				5
#define PLX_DMA_CHAN_RESET_CTL_BIT						6
#define PLX_DMA_CHAN_TRAN_SIZE_CTL_BIT					16

#define PLX_DMA_CHAN_ERR_INTR_STA_BIT					16
#define PLX_DMA_CHAN_INV_DESC_INTR_STA_BIT				17
#define PLX_DMA_CHAN_DESC_DONE_INTR_STA_BIT				18
#define PLX_DMA_CHAN_ABORT_DONE_INTR_STA_BIT			19
#define PLX_DMA_CHAN_GRACE_PAUSE_DONE_INTR_STA_BIT		20
#define PLX_DMA_CHAN_IMM_PAUSE_DONE_INTR_STA_BIT		21
#define PLX_DMA_CHAN_DONE_INTR_ENA_BIT					30
#define PLX_DMA_CHAN_VALID_BIT							31
#define PLX_DMA_CHAN_MODE_BITS							0x00000003

#define PLX_LINK_LUT_ENTRY_ENABLE_BIT					0
#define PLX_VIRT_LUT_ENTRY_ENABLE_BIT					31
#define PLX_LINK_MAX_LUT_ENTRY							31
#define PLX_VIRT_MAX_LUT_ENTRY							7

#define PLX_DMA_CHAN_CTL_STA_MASK						0x3f

#define PLX_DEV_EEP_PRESENT_STATUS_BITS					0x03
#define PLX_DEV_EEP_CMD_STATUS_BIT						18
#define PLX_DEV_EEP_CMD_WRITE_ENABLE_BIT				25
#define PLX_DEV_EEP_BUF_WIDTH							4

// GPIO I2C configuration
#define PLX_GPIO2_I2C_CLOCK_OUTPUT						0x20000000
#define PLX_GPIO2_I2C_CLOCK_INPUT_MASK					0xCFFFFFFF
#define PLX_GPIO2_I2C_DATA_OUTPUT						0x80000000
#define PLX_GPIO2_I2C_DATA_INPUT_MASK					0x3FFFFFFF
#define PLX_GPIO2_I2C_CLOCK_LINE						0x4000			// GPIO30
#define PLX_GPIO2_I2C_DATA_LINE							0x8000			// GPIO31

#define SHIFT_LOWER_WORD								16
#define SHIFT_UPPER_WORD								16
#define LOWEST_2BITS_ALIGN_CHECK						0x03
#define LOWER_12BITS_MASK								0x00000fff
#define LOWER_3BITS_MASK								0x0000fff8
#define LOWER_4BITS_MASK								0xfffffff0
#define UPPER_WORD_MASK									0x0000ffff
#define LOWER_WORD_MASK									0xffff0000

typedef void * addr_t; 

typedef struct pex8619
{
	void *dev;						/* os-specific pointer to device structure */
	UINT32 port;					/*port number*/
	UINT8 bus;						/*bus number*/
	UINT8 device:5;					/*device id*/
	UINT8 function:3;				/*function number*/
	UINT32 irq;						/*Interrupt number*/
	volatile addr_t csr;			/*pointer to config space register*/
	volatile addr_t pciMem;
	volatile addr_t ramMem;
	volatile addr_t ramMem1;
	int active;
 }ENPMC_PEX_8619_DEV_CTRL;

typedef struct pex8619dma
{
	void *dev;						/* os-specific pointer to device structure */
	UINT32 port;					/*port number*/
	UINT8 bus;						/*bus number*/
	UINT8 device:5;					/*device id*/
	UINT8 function:3;				/*function number*/
	UINT32 irq;						/*Interrupt number*/
	volatile addr_t dmacsr;			/*pointer to config space register*/
	int active;
} ENPMC_PEX_8619_DMA_CTRL;

typedef struct {
	INT32 magic;
	UINT32 channel;					/*DMA channel*/
	INT32 inuse;					/*DMA channel  - inuse or free*/
	INT32 status;					/*DMA channel status*/
	INT32 num_descriptors;			/*NUmber of desc for the DMA transfer*/
	INT32 aborted, paused, done;	/*DMA state - abort, pause or done*/
	INT32 direction;				/*direction - READ or WRITE*/
	addr_t descriptor_base;
} ENPMC_P8619_DMA_CHAN_INFO;

typedef struct _pmc_pex8619_gpio_signal_pairs_t
{
	UINT8 out;						// output GPIO drives input GPIO
	UINT8 in;
} ENPMC_GPIO_SIGNAL_PAIRS;

typedef struct _pmc_pex8619_gpio_loopback_info_t
{
	UINT8	plx8619Instance;
	const ENPMC_GPIO_SIGNAL_PAIRS * gpioTable;
	UINT32	dConfig1;
	UINT32	dConfig2;
	UINT32	dOutputs;
	UINT32	dInputs;
} ENPMC_GPIO_LOOPBACK_INFO;



