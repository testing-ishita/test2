/*******************************************************************************
*
* Filename:	 gei_netif.c
*
* Description:	lwIP Ethernet driver for Intel Gigabit devices.
*
* $Revision: 1.14 $
*
* $Date: 2015-03-31 11:44:53 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/ports/netif/gei_netif.c,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

#include "lwIP_Apps.h"

#include <bit/pci.h>
#include <private/cpu.h>
#include <private/sys_delay.h>
#include <private/port_io.h>
#include <private/atomic.h>

#include "config.h"

#include "gei_netif.h"
#include "../debug/dbgLog.h"

#define GEIF_POLL_MODE				/* define to switch to polling mode */
//#undef GEIF_POLL_MODE 				/* define to switch to polling mode */
#undef GEIF_STATIC_MEM_ARRAY		/* define to use static array for memory */
#define GEIF_SHARED_ISR				/* define to use shared interrupt handling */

#ifndef GEIF_SHARED_ISR
#undef GEIF_MAX_NUM
#define GEIF_MAX_NUM 1		/* restrict number allowed to one in this mode */
#endif

#undef INCLUDE_GEIF_DEBUG
#undef INCLUDE_GEIF_ERROR

#ifdef INCLUDE_DEBUG_VGA
#include "../vgalib/vga.h"
#define VGA_GEIF_NUM 	0 	/* interface number to use */
#endif


/* Debug levels */
#define GEIF_DBG_INIT	0x01
#define GEIF_DBG_TRACE	0x02
#define GEIF_DBG_RX		0x10
#define GEIF_DBG_TX		0x20
#define GEIF_DBG_DIAG	(GEIF_DBG_INIT) // (GEIF_DBG_INIT | GEIF_DBG_RX | GEIF_DBG_TX)

#ifdef INCLUDE_GEIF_DEBUG
#warning "***** DEBUG ON *****"
static UINT32 dbgLevel = (GEIF_DBG_INIT | GEIF_DBG_TRACE);
/* use DBLOG or sysDebugPrintf */
#define GEIFDBG( lvl, x )	do if ((lvl) & dbgLevel) {DBLOG x;} while(0)
#else
#define GEIFDBG( lvl, x )
#endif

#ifdef INCLUDE_GEIF_ERROR
#warning "***** ERROR MESSAGES ON *****"
/* use DBLOG or sysDebugPrintf */
#define GEIFERR( x )	do {sysDebugPrintf x;} while(0)
#else
#define GEIFERR( x )
#endif


/* Define to describe network interface name. */
#define IFNAME0 'g'
#define IFNAME1 'e'


/**
 * Struct to hold private data used to operate the interface.
 */
typedef struct
{
	struct netif *pNetif;      	/* lwIP network interface */
	
	char name[4];				/* Descriptive name: ge0 */
	UINT8 num;					/* Number of this interface */
	int cpuNo;					/* CPU number we're running on */
	
	UINT32 baseAddr;			/* Base address */
	UINT32 flashAddr;			/* Flash base address on ICH controllers */
	
	UINT16 vid;					/* Vendor ID */
	UINT16 did;					/* Device ID */
	PCI_PFA	pfa;				/* Device location (packed PCI Function Address) */
	UINT8 irqNo;				/* Device IRQ number */
	UINT8 instance;				/* Device instance */
	int	devType;				/* Device type */
	
	UINT16 phyId1;				/* PHY ID1 */
    UINT16 phyId2;				/* PHY ID2 */
	
	sys_mutex_t tx_mutex;		/* Control access to transmitter */
	sys_mutex_t op_mutex;		/* Control access ethernet output */
	sys_sem_t rx_sem;     		/* Semaphore to signal receive thread */
	
	UINT8 phyAddr;				/* PHY address */
	
	UINT8 *pRxDescMem;			/* Pointer to receive descriptor base address */
	UINT8 *pTxDescMem;			/* Pointer to transmit descriptor base address */
	
	UINT8 *pRxBufMem;			/* Pointer to receive buffer base address */
	UINT8 *pTxBufMem;			/* Pointer to transmit buffer base address */
	
	UINT32 rxDescTail;			/* Index to receive descriptor tail */
	UINT32 txDescTail;			/* Index to transmit descriptor tail */
	UINT32 txDescLastCheck;		/* Index of descriptor checked */
	
	UINT32 intStatus;			/* Interrupt status */
	long higherPriTaskWoken;	/* Higher priority task woken */
	
	int linkType;				/* Link type (copper or serdes) */
	UINT32 linkMode; 			/* Current link mode (media type) */
	UINT32 linkStatus;			/* Current link status */
	UINT32 linkSpeed;			/* Current link speed */
	
	UINT32 intCount;			/* Total interrupt count */
	UINT32 linkIntCount;		/* Link interrupt count */
	UINT32 rxIntCount;			/* Receive interrupt count */
	UINT32 txIntCount;			/* Transmit interrupt count */
	UINT32 taskSigCount;		/* Task interrupt signal count */
	UINT32 taskSigErrors;		/* Task interrupt signal errors */
	UINT32 rxSigCount;			/* Recieved signal count */
	UINT32 rxPktCount;			/* Receive packet count */
	UINT32 txPktCount;			/* Transmit packet count */
	UINT32 rxPktDropped;		/* Receive packets dropped */
	UINT32 txPktDropped;		/* Transmit packets dropped */
  
} GEIF_NET; /* GEIF_NET */

																			/* Descriptor memory, must be 128 byte aligned */
#define GEIF_RXDESC_MEM_SIZE 	( (GEIF_RXD_CNT * sizeof(GEIF_RXD)) + 128 ) /* RX descriptor size plus alignment */
#define GEIF_TXDESC_MEM_SIZE 	( (GEIF_TXD_CNT * sizeof(GEIF_TXD)) + 128 ) /* TX descriptor size plus alignment */
																			/* Data buffer memory, must be 16 byte aligned */
#define GEIF_RXBUF_MEM_SIZE  	( (GEIF_RXD_CNT * GEIF_RX_BUF_SIZE) + 16 )	/* RX bufer size plus alignment */					
#define GEIF_TXBUF_MEM_SIZE  	( (GEIF_TXD_CNT * GEIF_TX_BUF_SIZE) + 16 )	/* TX bufer size plus alignment */

#define GEIF_TOTAL_MEM_SIZE 	(GEIF_RXDESC_MEM_SIZE + GEIF_TXDESC_MEM_SIZE + GEIF_RXBUF_MEM_SIZE + GEIF_TXBUF_MEM_SIZE)

#ifdef GEIF_STATIC_MEM_ARRAY
static UINT8 geifMem[GEIF_TOTAL_MEM_SIZE * GEIF_MAX_NUM];	/* Descriptor/buffer memory */
#define GEIF_MEM_ADDR (UINT32)geifMem						/* Descriptor/buffer memory start address */
#else
#define GEIF_MEM_ADDR 	0x04100000							/* Descriptor/buffer memory start address */
															/* See boot_prot.s for memory mapping details.       */
															/* Note: the total memory size used by all instances */
															/* of this driver must not exceed 2MB or changes to  */
															/* the system memory mapping will be requied.        */      
#endif

#ifdef INCLUDE_DEBUG_VGA
extern int startLine;
#endif

#ifndef GEIF_SHARED_ISR
static GEIF_NET *pGlobalGei = NULL;  /* for use by ISR */
#endif


static UINT32 geifValidateNvmChecksum_82580(struct netif *pNetif, char *pName);
UINT32 geifValidateNvmChksumWithOffset (GEIF_NET *pGeif, UINT16 offset);

#ifdef INCLUDE_GEIF_DEBUG
/*******************************************************************************
*
* geifShowRegs
*
* Show some registers for debug.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifShowRegs( UINT32 dbgLvl, GEIF_NET *pGeif )
{
	GEIFDBG( dbgLvl, ("%s: CTRL   : 0x%08x\n", pGeif->name, GEIF_READ_REG( GEIF_CTRL )));
	GEIFDBG( dbgLvl, ("%s: STATUS : 0x%08x\n", pGeif->name, GEIF_READ_REG( GEIF_STATUS )));
	GEIFDBG( dbgLvl, ("%s: CTRLEXT: 0x%08x\n", pGeif->name, GEIF_READ_REG( GEIF_CTRLEXT )));
	GEIFDBG( dbgLvl, ("%s: RCTL   : 0x%08x\n", pGeif->name, GEIF_READ_REG( GEIF_RCTL )));
	GEIFDBG( dbgLvl, ("%s: TCTL   : 0x%08x\n", pGeif->name, GEIF_READ_REG( GEIF_TCTL )));
	GEIFDBG( dbgLvl, ("%s: ITR    : 0x%08x\n", pGeif->name, GEIF_READ_REG( GEIF_ITR )));
}
#endif


/*******************************************************************************
*
* geifDisableDevInt
* 
* This function disables the device interrupts.
*
*
* RETURNS: Interrupt setting before disable
* 
*******************************************************************************/
static UINT32 geifDisableDevInt( GEIF_NET *pGeif )
{
	volatile UINT32 intSet;
	volatile UINT32 intIcr;
	
	
	intSet = GEIF_READ_REG( GEIF_IMS );
	GEIF_WRITE_REG( GEIF_IMC, GEIF_IMC_ALL_BITS );
	
	/* Read back ICR to clean up all pending interrupts */
	intIcr = GEIF_READ_REG( GEIF_ICR );
	
	/* Just to stop compiler warnings */
	if (intIcr != 0)
	{
        intIcr = 0;
    }
	
	return intSet;
}


/*******************************************************************************
*
* geifDevReset
*
* This function resets the device.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifDevReset( GEIF_NET *pGeif )
{
	UINT32 regVal;

	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));
	
	/* Disable all interrupt */
	geifDisableDevInt( pGeif );     
	
	/* Disable/clean RX CTL */
	GEIF_WRITE_REG( GEIF_RCTL, 0 );
	
	/* Disable/clean TX CTL */
	GEIF_WRITE_REG( GEIF_TCTL, 0 );
	
	/* Reset the chip */
	GEIF_WRITE_REG( GEIF_CTRL, GEIF_CTRL_RST_BIT );
	
	/* Wait hardware ready */
	vTaskDelay( 250 );
	
	GEIF_WRITE_REG( GEIF_CTRL, (GEIF_CTRL_SLU_BIT | GEIF_CTRL_FRCSPD_BIT | GEIF_CTRL_FRCDPX_BIT) );
	
	/* Clean up GEIF_CTRL_EXT */
	regVal = 0;
	if ( pGeif->devType == GEIF_DEVTYPE_ADVANCED )
    {
    	regVal |= 0x10000000;	/* Set Driver loaded bit for FW */
	}

	GEIF_WRITE_REG( GEIF_CTRLEXT, regVal );

	/* Clean all pending interrupts */
	geifDisableDevInt( pGeif );   
}


/*******************************************************************************
*
* geifTxRxDisable
*
* This function disables the transmitter and receiver. 
*
*
* RETURN: None
* 
*******************************************************************************/
static void geifTxRxDisable( GEIF_NET *pGeif )
{
	/* Disable chip RX/TX */
	
	GEIF_CLRBIT( GEIF_RCTL, GEIF_RCTL_EN_BIT );
	GEIF_CLRBIT( GEIF_TCTL, GEIF_TCTL_EN_BIT );

}


/*******************************************************************************
*
* geifTxRxEnable
*
* This function enables the transmitter and receiver.
*
*
* RETURN: None
* 
*******************************************************************************/
static void geifTxRxEnable( GEIF_NET *pGeif )
{
	UINT32 regVal;

	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));

	/* Enable chip RX/TX */

	if (pGeif->did == INTEL_DEVICEID_82576EB)
	{
		// this must be set first, otherwise the remaining registers settings are ignored
		GEIF_SETBIT( GEIF_RCTL, GEIF_RCTL_EN_BIT );

		// GEIF_RXDCTL_ENABLE cannot be set until GEIF_RCTL_EN_BIT is set in RCTL
		GEIF_SETBIT( GEIF_RXDCTL0, GEIF_RXDCTL_ENABLE );
		regVal = GEIF_READ_REG( GEIF_RXDCTL0 );
		while ((regVal & GEIF_RXDCTL_ENABLE) == 0)
		{
			// wait for GEIF_RXDCTL_ENABLE to go high
			vDelay(1);
			regVal = GEIF_READ_REG( GEIF_RXDCTL0 );
		}

		// must to be done last as register setting is ignored until GEIF_RXDCTL_ENABLE is set
		GEIF_WRITE_REG( GEIF_RDT0, (GEIF_RXD_CNT - 1) );
	}
	else
	{
		GEIF_SETBIT( GEIF_RCTL, GEIF_RCTL_EN_BIT );
	}

	GEIF_SETBIT( GEIF_TCTL, GEIF_TCTL_EN_BIT );

#ifdef INCLUDE_GEIF_DEBUG
	geifShowRegs( GEIF_DBG_INIT, pGeif );
#endif
}


/*******************************************************************************
*
* geifInitMem
*
* Initialize memory used.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifInitMem( GEIF_NET *pGeif )
{
	UINT8 *pMem;

	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));
	
	/* Setup RX descriptor memory */
	pMem = (UINT8 *) (GEIF_MEM_ADDR + (GEIF_TOTAL_MEM_SIZE * pGeif->num));
	GEIFDBG( GEIF_DBG_INIT, ("%s: geifRxDescMem: 0x%08x-0x%08x  %u bytes\n", pGeif->name, 
			(UINT32)pMem, ((UINT32)pMem + GEIF_RXDESC_MEM_SIZE - 1), GEIF_RXDESC_MEM_SIZE ));	
	memset( pMem, 0, GEIF_RXDESC_MEM_SIZE );
	pGeif->pRxDescMem = (UINT8 *) ROUND_UP( pMem, 128 );	
	GEIFDBG( GEIF_DBG_INIT, ("%s: pRxDescMem   : 0x%08x-0x%08x  %u bytes\n", 
			pGeif->name, (UINT32)pGeif->pRxDescMem, 
			((UINT32)pGeif->pRxDescMem + (GEIF_RXD_CNT * sizeof(GEIF_RXD)) - 1),
			(GEIF_RXD_CNT * sizeof(GEIF_RXD))));
	
	/* Setup TX descriptor memory */
	pMem += GEIF_RXDESC_MEM_SIZE;
	GEIFDBG( GEIF_DBG_INIT, ("%s: geifTxDescMem: 0x%08x-0x%08x  %u bytes\n", pGeif->name, 
			(UINT32)pMem, ((UINT32)pMem + GEIF_TXDESC_MEM_SIZE - 1), GEIF_TXDESC_MEM_SIZE ));
	memset( pMem, 0, GEIF_TXDESC_MEM_SIZE );
	pGeif->pTxDescMem = (UINT8 *) ROUND_UP( pMem, 128 );
	GEIFDBG( GEIF_DBG_INIT, ("%s: pTxDescMem   : 0x%08x-0x%08x  %u bytes\n", 
			pGeif->name, (UINT32)pGeif->pTxDescMem,
			((UINT32)pGeif->pTxDescMem + (GEIF_TXD_CNT * sizeof(GEIF_TXD)) - 1), 
			(GEIF_TXD_CNT * sizeof(GEIF_TXD))));
	
	/* Setup RX data buffers */
	pMem += GEIF_TXDESC_MEM_SIZE;
	GEIFDBG( GEIF_DBG_INIT, ("%s: geifRxBufMem : 0x%08x-0x%08x  %u bytes\n", pGeif->name, 
			(UINT32)pMem, ((UINT32)pMem + GEIF_RXBUF_MEM_SIZE - 1), GEIF_RXBUF_MEM_SIZE ));
	memset( pMem, 0, GEIF_RXBUF_MEM_SIZE );
	pGeif->pRxBufMem = (UINT8 *) ROUND_UP( pMem, 16 );
	GEIFDBG( GEIF_DBG_INIT, ("%s: pRxBufMem    : 0x%08x-0x%08x  %u bytes\n", 
			pGeif->name, (UINT32)pGeif->pRxBufMem,
			((UINT32)pGeif->pRxBufMem + (GEIF_RXD_CNT * GEIF_RX_BUF_SIZE) - 1),  
			(GEIF_RXD_CNT * GEIF_RX_BUF_SIZE)));
	
	/* Setup TX data buffers */
	pMem += GEIF_RXBUF_MEM_SIZE;
	GEIFDBG( GEIF_DBG_INIT, ("%s: geifTxBufMem : 0x%08x-0x%08x  %u bytes\n", pGeif->name, 
			(UINT32)pMem, ((UINT32)pMem + GEIF_TXBUF_MEM_SIZE - 1), GEIF_TXBUF_MEM_SIZE ));
	memset( pMem, 0, GEIF_TXBUF_MEM_SIZE );
	pGeif->pTxBufMem = (UINT8 *) ROUND_UP( pMem, 16 );
	GEIFDBG( GEIF_DBG_INIT, ("%s: pTxBufMem    : 0x%08x-0x%08x  %u bytes\n", 
			pGeif->name, (UINT32)pGeif->pTxBufMem,
			((UINT32)pGeif->pTxBufMem + (GEIF_TXD_CNT * GEIF_TX_BUF_SIZE) - 1),  
			(GEIF_TXD_CNT * GEIF_TX_BUF_SIZE)));
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: Total memory used: %u bytes\n", pGeif->name, GEIF_TOTAL_MEM_SIZE));
}


/*******************************************************************************
*
* geifEerdDone
*
* Polls the status bit (bit 1) of the EERD to determine when the read is done.
*
*
* RETURNS: ERR_OK or ERR_IF
* 
*******************************************************************************/
static int geifEerdDone( GEIF_NET *pGeif, int eerd )
{
    UINT32 i;
	UINT32 reg;
    int result;

	result = ERR_IF;

    for (i = 0; i < 10000; i++) 
	{
        if (eerd == GEIF_EEPROM_POLL_READ)
		{
            reg = GEIF_READ_REG( GEIF_EERD );
		}
        else
		{
            reg = GEIF_READ_REG( GEIF_EEWR );
		}

        if (reg & GEIF_EEPROM_RW_REG_DONE) 
		{
            result = ERR_OK;
            break;
        }
        
		/* wait 5us */
		sysDelayMicroseconds(5);
    }

    return result;
}


/*******************************************************************************
*
* geifReadEerd
*
* Reads a 16 bit word from the EEPROM using the EERD register.
*
*
* RETURNS: ERR_OK or ERR_IF
* 
*******************************************************************************/
static int geifReadEerd( GEIF_NET *pGeif, UINT32 offset, UINT32 words, UINT16 *data )
{
    UINT32 i;
	UINT32 eerd = 0;
    int result;


	result = ERR_OK;
	*data = 0;

    for (i = 0; i < words; i++) 
	{
        eerd = ((offset+i) << GEIF_EEPROM_RW_ADDR_SHIFT) +
                         GEIF_EEPROM_RW_REG_START;

		GEIF_WRITE_REG( GEIF_EERD, eerd );

        result = geifEerdDone( pGeif, GEIF_EEPROM_POLL_READ );

        if (result == ERR_IF) 
		{
            break;
        }

		eerd = GEIF_READ_REG( GEIF_EERD );

        data[i] = (UINT16) (eerd >> GEIF_EEPROM_RW_REG_DATA);					
    }

    return result;
}

/*****************************************************************************
 * geifReadInvmWord: read the requested word structure from iNVM
 *
 * RETURNS: 16-bit data
 */

static UINT16 geifReadInvmWord
(
	GEIF_NET *pGeif,
	UINT32	 index
)
{
	UINT32	dData;
	UINT16	wData;
	UINT16	wAddress;
	UINT8	bIndex;
	UINT8	found;

	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: index=%d\n", pGeif->name, __FUNCTION__, index));
	
	// Read iNVM until required data structure found

	found = FALSE;
	for (bIndex = 0; (!found) && (bIndex < GEIF_INVM_SIZE); ++bIndex)
	{
		dData = GEIF_READ_REG (GEIF_INVM_DATA_REG(bIndex));

		switch (dData & GEIF_INVM_STRUCT_TYPE_MASK)
		{
			case GEIF_INVM_NVM_END:
				bIndex = GEIF_INVM_SIZE;				// end of iNVM data
				break;

			case GEIF_INVM_WORD_AUTOLOAD:
				wAddress = (UINT16) ((dData & GEIF_INVM_WORD_ADDR_MASK) >> 9);
				wData = (UINT16) (dData >> 16);
				if (wAddress == index)
				{
					found = TRUE;
				}
				break;

			case GEIF_INVM_CSR_AUTOLOAD:
				++bIndex;
				break;

			case GEIF_INVM_PHY_AUTOLOAD:
			case GEIF_INVM_INVALIDATED:
			default:
				break;
		}
	}

	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: iNVM Word: %02X=%04X\n", pGeif->name, __FUNCTION__, wAddress, wData));

	return (wData);

} /* geifReadInvmWord () */


/*******************************************************************************
*
* geifEepromReadWord
* 
* Read a word from the EEPROM.
*
*
* RETURNS: value in WORD size
* 
*******************************************************************************/
static UINT16 geifEepromReadWord( GEIF_NET *pGeif, UINT32 index )
{
	UINT32	dRegData;
	UINT16	val=0;

	/* check which eeprom type is used */

	dRegData = GEIF_READ_REG( GEIF_INVM_EECD_REG );
	if (((pGeif->did == INTEL_DEVICEID_i210) || (pGeif->did == INTEL_DEVICEID_i210_SERDES)) && 
		((dRegData & GEIF_EECD_FLASH_IN_USE) == 0))
	{
		val = geifReadInvmWord (pGeif, index);
	}
	else
	{
		geifReadEerd( pGeif, index, 1, &val );
	}

	return val;
}


/*******************************************************************************
*
* geifFlashReadWord
* 
* Read a word from the ICH flash.
*
*
* RETURNS: value in WORD size
* 
*******************************************************************************/
static UINT16 geifFlashReadWord( GEIF_NET *pGeif, UINT32 index )
{
	UINT16 sts;
	UINT32 offset, data = 0;
	UINT32 i;
	UINT32 regVal;

	/* Clear any pending operation. */
	
	GEIF_FL_SETBIT_2( GEIF_FL_HFSSTS, GEIF_FL_HFSSTS_FLCDONE | GEIF_FL_HFSSTS_FLCERR );

	/* Calculate and set eeprom offset */

	offset = index;

	// ICH/PCH has two banks of NVM which it uses alternately
	// so select the correct one
	regVal = GEIF_READ_REG( GEIF_EEC );
	if ((regVal & (GEIF_EEC_NV_PRES + GEIF_EEC_NV_AUTO_RD)) == (GEIF_EEC_NV_PRES + GEIF_EEC_NV_AUTO_RD))
	{
		if (regVal & GEIF_EEC_NV_SEC1VAL)
		{
			// Sector 1 is valid so adjust the offset accordingly
			offset += 0x800;
		}
	}
	offset <<= 1; /* Convert word offset to bytes */
	offset += (GEIF_FL_READ_4( GEIF_FL_GFP ) & GEIF_FL_GFP_BASE) * GEIF_FL_SECTOR_SIZE;
	
	GEIF_FL_WRITE_4( GEIF_FL_FADDR, offset );
	
	/* Start cycle */
	
	GEIF_FL_WRITE_2( GEIF_FL_HFSCTL, GEIF_FL_SIZE_WORD | GEIF_FL_READ | GEIF_FL_HFSCTL_FLCGO );
	
	for (i = 0; i < 10000; i++)
	{
		if (GEIF_FL_READ_2( GEIF_FL_HFSSTS ) & GEIF_FL_HFSSTS_FLCDONE)
		{
			break;
		}
		
		sysDelayMilliseconds(1);
	}
	
	data = GEIF_FL_READ_4( GEIF_FL_FDATA0 );
	sts = GEIF_FL_READ_2( GEIF_FL_HFSSTS );
	
	/* Clear status */
	
	GEIF_FL_SETBIT_2( GEIF_FL_HFSSTS, GEIF_FL_HFSSTS_FLCDONE | GEIF_FL_HFSSTS_FLCERR );
	
	if (i == 100000 || sts & GEIF_FL_HFSSTS_FLCERR)
	{
		return 0;
	}
	else
	{
		return (data & 0xFFFF);
	}	
}


/*******************************************************************************
*
* geifEtherAdrGet
* 
*
* This function gets the Ethernet address from the EEPROM/ICH Flash.
*
* RETURNS: None.
* 
*******************************************************************************/
static void geifEtherAdrGet( struct netif *pNetif, GEIF_NET *pGeif )
{
	UINT32 i;
	UINT32 j = 0;
	UINT16 val;
    UINT16 nvm_offset = 0;


	for (i = 0; i < ETHER_ADDR_LEN / sizeof(UINT16); i++) 
	{
		/* get word i from EEPROM/Flash */
		
		if (pGeif->devType == GEIF_DEVTYPE_ICH)
		{
			val = geifFlashReadWord( pGeif, (UINT16)(GEIF_EEPROM_MAC_OFFSET + i ));
		}
		else
		{
			val = geifEepromReadWord( pGeif, (UINT16)(GEIF_EEPROM_MAC_OFFSET + i ));
		}
		
		pNetif->hwaddr[j++] = (UINT8) val;
		pNetif->hwaddr[j++] = (UINT8) (val >> 8);
	}
	
	/* Multiport adapters share the same EEPROM, hence will have the same address */
	/* We adjust this by XORing the PCI function ID with the last octet */
	/* This is the method use by Intel */ 
	if ((pGeif->did == INTEL_DEVICEID_82580_QUAD_COPPER) ||
		(pGeif->did == INTEL_DEVICEID_82580_DUAL_COPPER) ||
		(pGeif->did == INTEL_DEVICEID_82580EB_SERDES)) 
	{
		pNetif->hwaddr[5] = (UINT8) (pNetif->hwaddr[5] + GEIF_FID(GEIF_READ_REG( GEIF_STATUS )));
	}
	else if ((pGeif->did == INTEL_DEVICEID_i350AMx_COPPER) ||
			 (pGeif->did == INTEL_DEVICEID_i350AMx_SERDES))
	{
		// calculate correct offset into eeprom
		nvm_offset = NVM_82580_LAN_FUNC_OFFSET(pGeif->pfa & 0x03);
		val = geifEepromReadWord( pGeif, nvm_offset);
		pNetif->hwaddr [0] = (UINT8) val;
		pNetif->hwaddr [1] = (UINT8)(val >> 8);
		val = geifEepromReadWord( pGeif, nvm_offset + 0x1);
		pNetif->hwaddr [2] = (UINT8) val;
		pNetif->hwaddr [3] = (UINT8)(val >> 8);
		val = geifEepromReadWord( pGeif, nvm_offset + 0x2);
		pNetif->hwaddr [4] = (UINT8) val;
		pNetif->hwaddr [5] = (UINT8)(val >> 8);
	}
	else if ((pGeif->did != INTEL_DEVICEID_i210) && (pGeif->did != INTEL_DEVICEID_i210_SERDES))
	{
		if (GEIF_FID(GEIF_READ_REG( GEIF_STATUS )))
		{
			pNetif->hwaddr[5] ^= 1;
		}
	}

	GEIFDBG( GEIF_DBG_INIT, ("%s: Ethernet address: %02x:%02x:%02x:%02x:%02x:%02x\n", 
				pGeif->name,
				pNetif->hwaddr[0], pNetif->hwaddr[1], pNetif->hwaddr[2], 
				pNetif->hwaddr[3], pNetif->hwaddr[4], pNetif->hwaddr[5]) );
}

/**
 *  geifValidateNvmChksumWithOffset- Validate EEPROM
 *  checksum
 *  @hw: pointer to the netif structure
 *  @offset: offset in words of the checksum protected region
 *
 *  Calculates the EEPROM checksum by reading/adding each word of the EEPROM
 *  and then verifies that the sum of the EEPROM is equal to 0xBABA.
 **/
UINT32 geifValidateNvmChksumWithOffset (GEIF_NET *pGeif, UINT16 offset)
{
	INT32 ret_val = 0;
	UINT16 checksum = 0;
	UINT16 i, nvm_data;
	char buffer[80];
	UINT32 dRegData;

	GEIFDBG(GEIF_DBG_INIT, ("geifValidateNvmChksumWithOffset\n"));

/*
	sprintf(buffer,"geifValidateNvmChksumWithOffset\n");
	sysDebugWriteString(buffer);
*/

	dRegData = GEIF_READ_REG( GEIF_INVM_EECD_REG );
	if (((pGeif->did == INTEL_DEVICEID_i210) || (pGeif->did == INTEL_DEVICEID_i210_SERDES)) 
		&& ((dRegData & GEIF_EECD_FLASH_IN_USE) == 0))
	{
		return ret_val;		// no checksum in iNVM eeprom
	}

    for (i = offset; i < ((NVM_CHECKSUM_REG + offset) + 1); i++) 
    {

    	nvm_data = geifEepromReadWord (pGeif,i);
        checksum += nvm_data;
    }



    if (checksum != (UINT16) NVM_SUM) 
    {
        GEIFDBG(GEIF_DBG_INIT,("NVM Checksum Invalid\n"));
        
    	sprintf(buffer,"Invalid NVM Checksum:%04x\n",checksum);
    	sysDebugWriteString(buffer);

        ret_val = -1;
    }
    else
    {
    	GEIFDBG(GEIF_DBG_INIT,("NVM Checksum: 0x%04x\n", checksum));
/*
    	sprintf(buffer,"Checksum:%04x\n",checksum);
    	sysDebugWriteString(buffer);
*/
    }

    return ret_val;
}


/**
 *  geifValidateNvmChecksum_82580 - Validate EEPROM checksum
 *  @netif: pointer to the netif structure
 *  @pName: name of the interface
 *
 *  Calculates the EEPROM section checksum by reading/adding each word of
 *  the EEPROM and then verifies that the sum of the EEPROM is
 *  equal to 0xBABA.
 **/
static UINT32 geifValidateNvmChecksum_82580(struct netif *pNetif, char *pName)
{
    GEIF_NET *pGeif = pNetif->state;
    INT32 ret_val = 0;
/*    UINT16 eeprom_regions_count = 1; */
    UINT16 j=0, nvm_data=0;
    UINT16 nvm_offset=0;
/*    char buffer[80]; */

    GEIFDBG(GEIF_DBG_INIT,("geifValidateNvmChecksum_82580\n"));
/*
	sprintf(buffer,"geifValidateNvmChecksum_82580\n");
	sysDebugWriteString(buffer);
*/
    nvm_data = geifEepromReadWord (pGeif, NVM_COMPATIBILITY_REG_3);

/*    
	sprintf(buffer,"nvm_data compatibility:%d\n",nvm_data);
    sysDebugWriteString(buffer);
*/
   if (nvm_data & NVM_COMPATIBILITY_BIT_MASK) 
   {
			/* if checksums compatibility bit is set validate checksums
			 * for all 4 ports. */
			/* eeprom_regions_count = 4; */
/*
			sprintf(buffer,"eeprom_regions_count:%d\n",eeprom_regions_count);
			sysDebugWriteString(buffer);
*/
			if (strcmp(pName,"ge0") == 0)
			{
				j = 0;
			}
			else if (strcmp(pName,"ge1") == 0)
			{
				j =1;
			}
			else if (strcmp(pName,"ge2") == 0)
			{
				j =2;
			}
			else if (strcmp(pName,"ge3") == 0)
			{
				j =3;
			}
   }
   else
   {
	   j =0;
   }

    nvm_offset = NVM_82580_LAN_FUNC_OFFSET(j);

	// i350AMx_COPPER may not be the first function in the i350 controller
	if (pGeif->did == INTEL_DEVICEID_i350AMx_COPPER)
		nvm_offset = NVM_82580_LAN_FUNC_OFFSET(pGeif->pfa & 0x03);

    ret_val = geifValidateNvmChksumWithOffset(pGeif,nvm_offset);

     if (ret_val != 0)
            goto out;

out:
    return ret_val;
}




/*******************************************************************************
*
* geifRatClear
*
* This function clears the Receive Address Table (RAT).
*
*
* RETURNS: None
*
*******************************************************************************/
static void geifRatClear( GEIF_NET *pGeif )
{
	UINT32 i;
	
	
	/* Clean the RAT */
	for (i = 0; i < GEIF_NUM_RAR; i ++)
	{    	
		GEIF_WRITE_REG( (GEIF_RAL0 + (i * 8)), 0 );
		GEIF_WRITE_REG( (GEIF_RAH0 + (i * 8)), 0 );        
	}
}


/*******************************************************************************
*
* geifMtaClear
*
* This function clears the Multicast Table Array (MTA).
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifMtaClear( GEIF_NET *pGeif )
{
	UINT32 i;
	
	
	/* Clean up MTA */
	for (i = 0; i < GEIF_NUM_MTA; i++)
    {
	    GEIF_WRITE_REG( (GEIF_MTA0 + (4 * i)), 0 );
    }
}


/*******************************************************************************
*
* geifVlanTableClear
*
* This function clear all entries in the VLAN filter table.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifVlanTableClear( GEIF_NET *pGeif )
{
	UINT32 i;
	
	
	GEIF_WRITE_REG( GEIF_VET, 0x0 ); 
	
	for (i = 0; i < GEIF_NUM_VLAN; i++)
	{
		GEIF_WRITE_REG( (GEIF_VFTA0 + (4 * i)), 0 );
	}
}


/*******************************************************************************
*
* geifRxAdrSet
*
* This function sets an address in the Receive Address Table.
*
*
* RETURNS: None
*
*******************************************************************************/
static void geifRxAdrSet( GEIF_NET *pGeif, UINT8 adr[], int index )
{
	UINT32 ral = 0;
	UINT32 rah = 0;
	
	
	if ( index < GEIF_NUM_RAR )
	{
		/* Split the address into RAH/RAL */
		ral = ((UINT32)adr[0] | ((UINT32)adr[1] << 8) | ((UINT32)adr[2] << 16) | ((UINT32)adr[3] << 24) );
		rah = ((UINT32)adr[4] | ((UINT32)adr[5] << 8) | (GEIF_RAH_VALID_BIT));
		
		GEIF_WRITE_REG( (GEIF_RAL0 + (index * 8)), ral );
		GEIF_WRITE_REG( (GEIF_RAH0 + (index * 8)), rah );
		
		GEIFDBG( GEIF_DBG_INIT, ("%s: %s: RAT[%d]: 0x%08X 0x%08X\n", 
				pGeif->name, __FUNCTION__, index,
				GEIF_READ_REG( (GEIF_RAH0 + (index * 8))), 
				GEIF_READ_REG( (GEIF_RAL0 + (index * 8))) ));
	}
	else
	{
		GEIFERR(("%s: %s: Error - invalid RAT index\n", pGeif->name, __FUNCTION__));
	}
}


/*******************************************************************************
*
* geifHwAddrInit
*
* This function initializes the hardware addresses. 
* 
* There are two types of receive filter: the 16-entry Receive Address Table 
* which can be used to perform perfect filtering and the Multicast Table 
* Array with 4096-bit hash filters. In addition to these, there is also a
* VLAN filter table.
* 
* Our MAC address is put in the RTA, the MTA is not used and VLAN's are not 
* supported. 	 
*
*
* RETURNS: None
*
*******************************************************************************/
static void geifHwAddrInit( struct netif *pNetif, GEIF_NET *pGeif )
{
	/* Clear the Receive Address Table */
	geifRatClear( pGeif );
	
	/* Clear the Multicast Table Array */
	geifMtaClear( pGeif );
	
	/* Clear the VLAN filters */
	geifVlanTableClear( pGeif );

	/* We use the first entry in the RAT for our station address (MAC) */
	geifRxAdrSet( pGeif, pNetif->hwaddr, 0 );
}


/*******************************************************************************
*
* geifRxDesUpdate
*
* This function cleans up receive descriptors.
* Then it updates the tail pointer to make it available to the hardware.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifRxDesUpdate( GEIF_NET *pGeif )
{
    UINT32 tempTail;
    UINT8 *pRxDesc;
   

    /* Get the RX tail descriptor */
    tempTail = pGeif->rxDescTail;
    pRxDesc = GEIF_GET_RXD_ADDR( tempTail );

    /* Clean up status field */
    GEIF_WRITE_DESC_BYTE( pRxDesc,RXD_STATUS_OFFSET, 0 );

    /* Update the tail pointer */
    GEIF_GET_RXD_TAIL_UPDATE( pGeif->rxDescTail, 1 );
    
	/* Release this descriptor */
    GEIF_WRITE_REG( GEIF_RDT0, tempTail );
}


#ifdef INCLUDE_GEIF_DEBUG
/*******************************************************************************
*
* geifShowRxDesc
*
* Show receive descriptor.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifShowRxDesc( UINT32 dbgLvl, GEIF_RXD *pRxDesc )
{
	GEIFDBG( dbgLvl, ("%*saddrhi: 0x%08x addrlo: 0x%08x\n",
				5, "", pRxDesc->addrhi, pRxDesc->addrlo ));
	GEIFDBG( dbgLvl, ("%*slen   : 0x%04x     csum  : 0x%04x\n",
				5, "", pRxDesc->len, pRxDesc->csum ));
	GEIFDBG( dbgLvl, ("%*ssts   : 0x%02x       err   : 0x%02x      special: 0x%04x\n",
				5, "", pRxDesc->sts, pRxDesc->err, pRxDesc->special ));
}
#endif


/*******************************************************************************
*
* geifRxSetup
* 
* This function sets up the receive descriptors and configures the receiver.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifRxSetup( GEIF_NET *pGeif )
{
	UINT16 i;
	UINT8 *pRxDesc;
	UINT8 *pBuf;
	UINT32 rxcsum;
	
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));
	
	pGeif->rxDescTail = 0;
	
	for (i = 0; i < GEIF_RXD_CNT; i++)
	{
		pRxDesc = GEIF_GET_RXD_ADDR( i );
		pBuf = &pGeif->pRxBufMem[i * GEIF_RX_BUF_SIZE];
		
		GEIF_WRITE_DESC_LONG( pRxDesc, RXD_BUFADRLOW_OFFSET, (UINT32)pBuf );

		GEIF_WRITE_DESC_LONG( pRxDesc, RXD_BUFADRHIGH_OFFSET, 0 );      

		/* Set STATUS field */
		GEIF_WRITE_DESC_BYTE( pRxDesc,RXD_STATUS_OFFSET, 0 );

#ifdef INCLUDE_GEIF_DEBUG		
		/* Sample some descriptors */
		if ( (i == 0) || (i == 63) || (i == 127) )
		{
			GEIFDBG( GEIF_DBG_INIT, ("%s: pRxDesc[%03u]: 0x%08x  data: 0x%08x\n", 
						pGeif->name, i, (UINT32)pRxDesc, (UINT32)pBuf));
			geifShowRxDesc( GEIF_DBG_INIT, (GEIF_RXD *) pRxDesc );
		}
#endif
	}

	/* Set receive interrupt delay & flush descriptors */
	if (( pGeif->did == INTEL_DEVICEID_82579LM2 ) ||
		( pGeif->did == INTEL_DEVICEID_82573L ))
	{
		GEIF_WRITE_REG( GEIF_RDTR, (GEIF_RDTR_RXINT_DELAY | GEIF_RDTR_FPD_BIT) ); 
	}

	/* Set descriptor base register */
	GEIF_WRITE_REG( GEIF_RDBAL0, (UINT32) pGeif->pRxDescMem );
	GEIF_WRITE_REG( GEIF_RDBAH0, 0 );

	/* Set descriptor length register */
	GEIF_WRITE_REG( GEIF_RDLEN0, (RXD_SIZE * GEIF_RXD_CNT) );

	if ( pGeif->devType == GEIF_DEVTYPE_ADVANCED )
	{
		/* Set receive buffer size and descriptor type. */
		/* Receive buffer size value is in 1 KB resolution, if receive buffer */
		/* size field is equal 0x0, then RCTL determines the packet buffer size. */
		/* Clearing the descriptor type field sets the type to legacy */

		GEIF_CLRBIT( GEIF_SRRCTL0, (GEIF_SRRCTL_DESCTYPE | GEIF_SRRCTL_BSIZEPKT) );
		GEIFDBG( GEIF_DBG_INIT, ("%s: SRRCTL0: 0x%08x\n", pGeif->name, GEIF_READ_REG( GEIF_SRRCTL0 )));

		/* Enable receive queue */
		GEIF_SETBIT( GEIF_RXDCTL0, GEIF_RXDCTL_ENABLE );
	}

	/* Set descriptor head/tail registers */
	GEIF_WRITE_REG( GEIF_RDH0, 0 );
	GEIF_WRITE_REG( GEIF_RDT0, (GEIF_RXD_CNT - 1) ); 

	/* Set receive control register */
	GEIF_WRITE_REG( GEIF_RCTL, (GEIF_RCTL_BSIZE_2048 | GEIF_RCTL_SECRC_BIT) );

	/* Set receive checksum control */
	rxcsum = (GEIF_RXCSUM_TUOFL_BIT |GEIF_RXCSUM_IPOFL_BIT | GEIF_RXCSUM_PKT_CSUM_OFF | GEIF_RXCSUM_IPV6OFL_BIT);
	GEIF_WRITE_REG( GEIF_RXCSUM, rxcsum );
}


#ifdef INCLUDE_GEIF_DEBUG
/*******************************************************************************
*
* geifShowTxDesc
*
* Show transmit descriptor.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifShowTxDesc( UINT32 dbgLvl, GEIF_NET *pGeif, UINT32 descNum, GEIF_TXD *pTxDesc )
{
	GEIFDBG( dbgLvl, ("%s: pTxDesc[%03u]: 0x%08x\n", pGeif->name, descNum, (UINT32)pTxDesc));
					
	GEIFDBG( dbgLvl, ("%*saddrhi: 0x%08x addrlo : 0x%08x\n", 
				5, "", pTxDesc->addrhi, pTxDesc->addrlo ));
	GEIFDBG( dbgLvl, ("%*scmd   : 0x%08x sts    : 0x%02x\n", 
				5, "", pTxDesc->cmd, pTxDesc->sts ));
	GEIFDBG( dbgLvl, ("%*spopts : 0x%02x       special: 0x%04x\n", 
				5, "", pTxDesc->popts, pTxDesc->special ));
}
#endif


/*******************************************************************************
*
* geifTxSetup
* 
* This function sets up the transmitter descriptors and configures the transmitter.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifTxSetup( GEIF_NET *pGeif )
{
	UINT16 i;
	UINT8 *pTxDesc;
	UINT8 *pBuf;
	UINT32 regVal;
    UINT32 count;
	
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));
	
	pGeif->txDescTail = 0;
	pGeif->txDescLastCheck = GEIF_TXD_CNT - 1;
	
	for (i = 0; i < GEIF_TXD_CNT; i++)
	{
		pTxDesc = GEIF_GET_TXD_ADDR( i );
		pBuf = &pGeif->pTxBufMem[i * GEIF_TX_BUF_SIZE];
		
		GEIF_WRITE_DESC_LONG( pTxDesc, TXD_BUFADRLOW_OFFSET, (UINT32)pBuf );

		GEIF_WRITE_DESC_LONG( pTxDesc, TXD_BUFADRHIGH_OFFSET, 0 );

		/* Set the command field */
		GEIF_WRITE_DESC_BYTE( pTxDesc, TXD_CMD_OFFSET, 
								(TXD_CMD_EOP | TXD_CMD_IFCS | 
								TXD_CMD_IDE | TXD_CMD_RS) );

		/* Set DD bits in status field */
		GEIF_WRITE_DESC_BYTE( pTxDesc, TXD_STATUS_OFFSET, TXD_STAT_DD );

#ifdef INCLUDE_GEIF_DEBUG        
		/* Sample some descriptors */
		if ( (i == 0) || (i == 63) || (i == 127) )
		{
			geifShowTxDesc( GEIF_DBG_INIT, pGeif, i, (GEIF_TXD *) pTxDesc );
		}
#endif
	}

	/* Set descriptor base register */
	GEIF_WRITE_REG( GEIF_TDBAL0, (UINT32) pGeif->pTxDescMem );
	GEIF_WRITE_REG( GEIF_TDBAH0, 0 );    

	/* Set the length field */
	GEIF_WRITE_REG( GEIF_TDLEN0, (TXD_SIZE * GEIF_TXD_CNT) );

	/* Initialize tail and head registers */
	GEIF_WRITE_REG( GEIF_TDH0, 0 );
	GEIF_WRITE_REG( GEIF_TDT0, 0 );

	/* Set the interrupt delay for Transmit */
	if (pGeif->did == INTEL_DEVICEID_82579LM2)
	{
		GEIF_WRITE_REG( GEIF_TIDV, GEIF_TIDV_DELAY_5 );

		/* Set transmitter descriptor control register */
		regVal = GEIF_READ_REG( GEIF_TXDCTL0 ); 
		count = (GEIF_TXD_CNT / 8) - 1;
		regVal = ((regVal & 0x1ffffff) | (count << 25)); 
		GEIF_WRITE_REG( GEIF_TXDCTL0, regVal );
	}

	/* Set the TIPG register, for media type */
	regVal = GEIF_READ_REG( GEIF_STATUS ); 
	if (regVal & GEIF_STATUS_TBIMODE_BIT)
	{
		/* Fiber/Serdes device */
		regVal = GEIF_TIPG_IPGT_FIBER | (GEIF_TIPG_IPGR1 << GEIF_TIPG_IPGR1_SHIFT) 
		    	| (GEIF_TIPG_IPGR2 << GEIF_TIPG_IPGR2_SHIFT);
		    
		GEIFDBG( GEIF_DBG_INIT, ("%s: TBI mode\n", pGeif->name));
	}
	else
	{
		/* Copper device */
		regVal = GEIF_TIPG_IPGT_COPPER | (GEIF_TIPG_IPGR1 << GEIF_TIPG_IPGR1_SHIFT) 
				| (GEIF_TIPG_IPGR2 << GEIF_TIPG_IPGR2_SHIFT);
	}

	GEIF_WRITE_REG( GEIF_TIPG, regVal );
	GEIFDBG( GEIF_DBG_INIT, ("%s: TIPG: 0x%08x\n", pGeif->name, GEIF_READ_REG( GEIF_TIPG )));
	
	/* Set transmitter control register */
	regVal = GEIF_READ_REG( GEIF_TCTL );
	regVal |= (GEIF_TCTL_PSP_BIT | (GEIF_TCTL_THRESHOLD << GEIF_TCTL_CT_SHIFT));
	GEIF_WRITE_REG( GEIF_TCTL, regVal );

	/* Enable transmitter queue */
	if ( pGeif->devType == GEIF_DEVTYPE_ADVANCED )
	{
		GEIF_SETBIT( GEIF_TXDCTL0, GEIF_TXDCTL_ENABLE );
		regVal = GEIF_READ_REG( GEIF_TXDCTL0 );
	}
}


/*******************************************************************************
*
* geifConfigure
*
* This function configures the interface for promisc, multicast
* and broadcast modes.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifConfigure( GEIF_NET *pGeif, UINT32 flags )
{
	UINT32 oldVal;
	UINT32 newVal;
	
	
	/* Read RX control register */
	
	oldVal = GEIF_READ_REG( GEIF_RCTL );
	
	newVal = oldVal;
	
	/* Prepare new value for RX control register */
	
	if (flags & GEIF_FLAG_PROMISC_MODE)
	{
		newVal |= (GEIF_RCTL_UPE_BIT | GEIF_RCTL_MPE_BIT | GEIF_RCTL_BAM_BIT);
	}
	else
	{
		newVal &= ~(GEIF_RCTL_UPE_BIT | GEIF_RCTL_MPE_BIT);
	}
	
	if (flags & GEIF_FLAG_BROADCAST_MODE)
	{
		newVal |= GEIF_RCTL_BAM_BIT;
	}
	else if (!(flags & GEIF_FLAG_PROMISC_MODE))
	{
		newVal &= ~GEIF_RCTL_BAM_BIT;
	}
	
	if (flags & GEIF_FLAG_ALLMULTI_MODE)
	{
		newVal |= GEIF_RCTL_MPE_BIT;
	}
	else if (!(flags & GEIF_FLAG_PROMISC_MODE))
	{
		newVal &= ~GEIF_RCTL_MPE_BIT;
	}
	
	/* Set up RX control register if needed */
	
	if (newVal != oldVal)
	{
		GEIF_WRITE_REG( GEIF_RCTL, newVal );
	}
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: done\n", pGeif->name, __FUNCTION__));
}


/*******************************************************************************
*
* geifPhyRead
* 
* This function reads the MII registers in the PHY. 
*
* RETURN: Value read or 0xffff on error.
* 
*******************************************************************************/
static UINT16 geifPhyRead( GEIF_NET *pGeif, UINT8 phyAddr, UINT8 phyReg )
{
	int count = 6;
	volatile UINT32 mdicRegVal;
	
	
	mdicRegVal = (GEIF_MDIC_RD_BIT | (UINT32) phyAddr << GEIF_MDIC_PHY_SHIFT | 
	          		(UINT32) phyReg << GEIF_MDIC_REG_SHIFT);
	
	GEIF_WRITE_REG( GEIF_MDIC, mdicRegVal );
	
	sysDelayMicroseconds( 32 ); 	/* wait 32 microseconds */
	
	while (count--)		/* wait max 96 microseconds */
	{
		mdicRegVal = GEIF_READ_REG( GEIF_MDIC );
		
		if (mdicRegVal & GEIF_MDIC_RDY_BIT)
		{
		    break;
		}
		
		sysDelayMicroseconds( 22 );
	}
	
	if ((mdicRegVal & (GEIF_MDIC_RDY_BIT | GEIF_MDIC_ERR_BIT)) != GEIF_MDIC_RDY_BIT)
	{
		/* Failed */
		return 0xffff;
	}
	else
	{
		return (UINT16) mdicRegVal;
	}		
}


/*******************************************************************************
*
* geifPhyWrite
* 
* This function writes to the MII registers in the PHY. 
*
* RETURN: ERR_OK or ERR_IF
* 
*******************************************************************************/
static int geifPhyWrite( GEIF_NET *pGeif, UINT8 phyAddr, UINT8 phyReg, UINT16 dataVal )
{
	int count = 6;
	volatile UINT32 mdicRegVal;
	
	
	if ((phyAddr == MII_CTRL_REG) &&
            (dataVal & (MII_CR_RESET | MII_CR_POWER_DOWN)))  
    {
    	/* 
		 * On the 82580, the internal PHY should not be reset using 
		 * PHYREG 0 bit 15 (PCTRL.Reset), since in this case the internal PHY 
		 * configuration process is bypassed and then there is no guarantee 
		 * the PHY will operate correctly.
		 */		 
		if ( (MII_OUI(pGeif->phyId1, pGeif->phyId2) == GEIF_OUI_INTEL) ||
				(MII_OUI(pGeif->phyId1, pGeif->phyId2) == 0x005500))
	    {
	    	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: reset ignored for this device\n", 
					pGeif->name, __FUNCTION__));
			return ERR_OK;
	    }
	}
	
	mdicRegVal = (GEIF_MDIC_WR_BIT | (UINT32) phyAddr << GEIF_MDIC_PHY_SHIFT | 
	          		(UINT32) phyReg << GEIF_MDIC_REG_SHIFT);
	
	GEIF_WRITE_REG( GEIF_MDIC, (mdicRegVal | dataVal) );
	
    sysDelayMicroseconds( 32 ); 	/* wait 32 microseconds */
	
	while (count--)		/* wait max 96 microseconds */
	{
		mdicRegVal = GEIF_READ_REG( GEIF_MDIC );
		
		if (mdicRegVal & GEIF_MDIC_RDY_BIT)
		{
		    break;
		}
		
		sysDelayMicroseconds( 22 );
	}
	
	if ((mdicRegVal & (GEIF_MDIC_RDY_BIT | GEIF_MDIC_ERR_BIT)) != GEIF_MDIC_RDY_BIT)
	{
		return ERR_IF;
	}
	else
	{
		return ERR_OK;
	}	
}


/*******************************************************************************
*
* geifPhyReset
* 
* This function resets the PHY.
*
* RETURN: ERR_OK or ERR_IF
* 
*******************************************************************************/
static int geifPhyReset( GEIF_NET *pGeif )
{
	UINT16 miiSts;
	UINT16 miiCtl;
	int count;
	
	
	/* Get status register so we can determine extended capabilities */
	miiSts = geifPhyRead( pGeif, pGeif->phyAddr, MII_STAT_REG );
	
	/* Power down PHY */
	geifPhyWrite( pGeif, pGeif->phyAddr, MII_CTRL_REG, MII_CR_POWER_DOWN );
	geifPhyWrite( pGeif, pGeif->phyAddr, MII_CTRL_REG, 0 );
	
	/* Set reset bit and then wait for it to clear. */
	geifPhyWrite( pGeif, pGeif->phyAddr, MII_CTRL_REG, MII_CR_RESET );
	
	for (count = 0; count < 100; count++ )
	{
		miiCtl = geifPhyRead( pGeif, pGeif->phyAddr, MII_CTRL_REG );
		
		if ( !(miiCtl & MII_CR_RESET) )
		{
			break;
		}
		
		sysDelayMilliseconds( 10 );
	}
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: miiCtl: 0x%04x count: %d\n", 
			pGeif->name, __FUNCTION__, miiCtl, count));
	
	if (count < 100)
	{
		/* If extended capabilities then this is a gigabit PHY */
		if (miiSts & MII_SR_EXT_STS)
		{
			/* Advertise gigabit modes */
			geifPhyWrite( pGeif, pGeif->phyAddr, MII_MASSLA_CTRL_REG, 
						(MII_MASSLA_CTRL_1000T_FD|MII_MASSLA_CTRL_1000T_HD) );
		}
	
		return ERR_OK;
	}
	else
	{
		return ERR_IF;
	}
}


#ifdef INCLUDE_GEIF_DEBUG
/*******************************************************************************
*
* geifLinkModeShow
*
* Shows the current link mode.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifLinkModeShow( GEIF_NET *pGeif, UINT32 linkMode )
{
	if ((IFM_SUBTYPE(linkMode) == IFM_1000_T) && ((linkMode & IFM_GMASK) == IFM_FDX))
    {
        GEIFDBG( GEIF_DBG_DIAG, ("%s: linkMode: 1000FD (0x%08x)\n", pGeif->name, linkMode));	        
    }	        
    else if ((IFM_SUBTYPE(linkMode) == IFM_1000_T) &&  ((linkMode & IFM_GMASK) == IFM_HDX))
    {    
    	GEIFDBG( GEIF_DBG_DIAG, ("%s: linkMode: 1000HD (0x%08x)\n", pGeif->name, linkMode));		
	}			
    else if ((IFM_SUBTYPE(linkMode) == IFM_100_TX) && ((linkMode & IFM_GMASK) == IFM_FDX))
	{
		GEIFDBG( GEIF_DBG_DIAG, ("%s: linkMode: 100FD (0x%08x)\n", pGeif->name, linkMode));	        
    }	        
    else if ((IFM_SUBTYPE(linkMode) == IFM_100_TX) &&  ((linkMode & IFM_GMASK) == IFM_HDX))
    {
		GEIFDBG( GEIF_DBG_DIAG, ("%s: linkMode: 100HD (0x%08x)\n", pGeif->name, linkMode));	        
    }			   
    else if ((IFM_SUBTYPE(linkMode) == IFM_10_T) && ((linkMode & IFM_GMASK) == IFM_FDX))
	{
		GEIFDBG( GEIF_DBG_DIAG, ("%s: linkMode: 10FD (0x%08x)\n", pGeif->name, linkMode));	        
    }	        
    else if ((IFM_SUBTYPE(linkMode) == IFM_10_T) &&  ((linkMode & IFM_GMASK) == IFM_HDX))
	{
		GEIFDBG( GEIF_DBG_DIAG, ("%s: linkMode: 10HD (0x%08x)\n", pGeif->name, linkMode));      
    }
    else
	{
        GEIFDBG( GEIF_DBG_DIAG, ("%s: linkMode: NONE (0x%08x)\n", pGeif->name, linkMode));       
    }
}
#endif


/*******************************************************************************
*
* geifSerdesModeGet
* 
* This function gets the SERDES (link) status.
*
* RETURN: ERR_OK or ERR_IF
* 
*******************************************************************************/
static int geifSerdesModeGet( GEIF_NET *pGeif, UINT32 *linkMode, UINT32 *linkStatus )
{
	UINT32 pcslsts;
	
	
	*linkMode = IFM_ETHER;
	*linkStatus = IFM_AVALID;
	
	
	pcslsts = GEIF_READ_REG( GEIF_PCSLSTS );
	
	GEIFDBG( GEIF_DBG_DIAG, ("%s: %s: PCSLSTS: 0x%08x\n", pGeif->name, __FUNCTION__, pcslsts));
	
	/* No link */
	if ( !(pcslsts & GEIF_PCSLSTS_LINK_UP))
    {
	    *linkMode |= IFM_NONE;
	    return ERR_IF;
    }
    
    /* Set link as active */
    *linkStatus |= IFM_ACTIVE;
	
	/* Determine the link settings */
	if (pcslsts & GEIF_PCSLSTS_1000T_FD )
    {
        *linkMode |= IFM_1000_T|IFM_FDX;	        
    }	        
    else if (pcslsts & GEIF_PCSLSTS_1000T_HD )
    {    
		*linkMode |= IFM_1000_T|IFM_HDX;		
	}			
    else if (pcslsts & GEIF_PCSLSTS_100T_FD )
	{
        *linkMode |= IFM_100_TX|IFM_FDX;	        
    }	        
    else if (pcslsts & GEIF_PCSLSTS_100T_HD )
    {
        *linkMode |= IFM_100_TX|IFM_HDX;	        
    }			   
    else if (pcslsts & GEIF_PCSLSTS_10T_FD )
	{
        *linkMode |= IFM_10_T|IFM_FDX;        
    }	        
    else if (pcslsts & GEIF_PCSLSTS_10T_HD )
	{
        *linkMode |= IFM_10_T|IFM_HDX;    
    }
    else
	{
        *linkMode |= IFM_NONE;      
    }
    
#ifdef INCLUDE_GEIF_DEBUG
	geifLinkModeShow( pGeif, *linkMode );
#endif
		
	return ERR_OK;
}


/*******************************************************************************
*
* geifPhyModeGet
* 
* This function gets the PHY (link) status.
*
* RETURN: ERR_OK or ERR_IF
* 
*******************************************************************************/
static int geifPhyModeGet( GEIF_NET *pGeif, UINT32 *linkMode, UINT32 *linkStatus )
{
	UINT16 miiSts;
	UINT16 miiCtl;
	UINT16 miiAnar;
	UINT16 miiLpar;
	UINT16 gmiiAnar = 0;
	UINT16 gmiiLpar = 0;
	UINT16 anlpar;
	
	
	*linkMode = IFM_ETHER;
	*linkStatus = IFM_AVALID;
	
	/* Read MII status register once to unlatch link status bit */
	
	miiSts = geifPhyRead( pGeif, pGeif->phyAddr, MII_STAT_REG );
	
	/* Read again to know its current value */
	
	miiSts = geifPhyRead( pGeif, pGeif->phyAddr, MII_STAT_REG );
	
	GEIFDBG( GEIF_DBG_DIAG, ("%s: %s: miiSts: 0x%04x\n", pGeif->name, __FUNCTION__, miiSts));
	
	/* No link */
	if ( (miiSts == 0xffff) || !(miiSts & MII_SR_LINK_STATUS))
    {
	    *linkMode |= IFM_NONE;
	    return ERR_IF;
    }
	
	/* Set link as active */
	*linkStatus |= IFM_ACTIVE;
	
	/* Read the control, ability and link */
	/* partner advertisement registers    */
	miiCtl = geifPhyRead( pGeif, pGeif->phyAddr, MII_CTRL_REG );
	miiAnar = geifPhyRead( pGeif, pGeif->phyAddr, MII_AN_ADS_REG );
	miiLpar = geifPhyRead( pGeif, pGeif->phyAddr, MII_AN_PRTN_REG );
	
	GEIFDBG( GEIF_DBG_DIAG, ("%s: miiCtl: 0x%04x miiAnar: 0x%04x miiLpar: 0x%04x\n", 
			pGeif->name, miiCtl, miiAnar, miiLpar));
	
	if (miiSts & MII_SR_EXT_STS)
    {
	    gmiiAnar = geifPhyRead( pGeif, pGeif->phyAddr, MII_MASSLA_CTRL_REG );
	    gmiiLpar = geifPhyRead( pGeif, pGeif->phyAddr, MII_MASSLA_STAT_REG );
    }
	
	/* Determine the link settings.
	 * If autoneg is on, get the link settings from the advertisement and 
	 * link partner ability registers.
	 * If autoneg is off, use the settings in the control register.
	 */
	if (miiCtl & MII_CR_AUTO_EN)
    {
	    anlpar = miiAnar & miiLpar;
	    if (gmiiAnar & MII_MASSLA_CTRL_1000T_FD && gmiiLpar & MII_MASSLA_STAT_LP1000T_FD)
	    {
	        *linkMode |= IFM_1000_T|IFM_FDX;	        
	    }	        
	    else if (gmiiAnar & MII_MASSLA_CTRL_1000T_HD && gmiiLpar & MII_MASSLA_STAT_LP1000T_HD)
	    {    
			*linkMode |= IFM_1000_T|IFM_HDX;		
		}			
	    else if (anlpar & MII_ANAR_100TX_FD)
		{
	        *linkMode |= IFM_100_TX|IFM_FDX;	        
	    }	        
	    else if (anlpar & MII_ANAR_100TX_HD)
	    {
	        *linkMode |= IFM_100_TX|IFM_HDX;	        
	    }			   
	    else if (anlpar & MII_ANAR_10TX_FD)
		{
	        *linkMode |= IFM_10_T|IFM_FDX;        
	    }	        
	    else if (anlpar & MII_ANAR_10TX_HD)
		{
	        *linkMode |= IFM_10_T|IFM_HDX;    
	    }
	    else
		{
	        *linkMode |= IFM_NONE;      
	    }
	}
	else
    {
	    if (miiCtl & MII_CR_FDX)
		{
	        *linkMode |= IFM_FDX;
	    }
		else
		{
	        *linkMode |= IFM_HDX;
	    }	        
	        				        
	    if ((miiCtl & (MII_CR_100|MII_CR_1000)) == (MII_CR_100|MII_CR_1000))
		{
	        *linkMode |= IFM_1000_T;
	    }
		else if (miiCtl & MII_CR_100)
		{
	        *linkMode |= IFM_100_TX;
	    }
		else
		{
	        *linkMode |= IFM_10_T;
	    }
	}
	
#ifdef INCLUDE_GEIF_DEBUG
	geifLinkModeShow( pGeif, *linkMode );
#endif
	
	return ERR_OK;
}


/*******************************************************************************
*
* geifCheckPhy
* 
* This function reads the PHY and checks it's status. 
*
* RETURN: ERR_OK or ERR_IF
* 
*******************************************************************************/
static int geifCheckPhy( GEIF_NET *pGeif )
{
	UINT16 dataVal;
	int result;


	result = ERR_IF;

	dataVal = geifPhyRead( pGeif, pGeif->phyAddr, MII_STAT_REG );

	if ((dataVal != 0) && (dataVal != 0xffff))
	{
		result = ERR_OK;
	}

	pGeif->phyId1 = geifPhyRead( pGeif, pGeif->phyAddr, MII_PHY_ID1_REG );
	pGeif->phyId2 = geifPhyRead( pGeif, pGeif->phyAddr, MII_PHY_ID2_REG );

	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: phyAddr: %u dataVal: 0x%04x phyId1: 0x%04x phyId2: 0x%04x\n", 
				pGeif->name, __FUNCTION__, pGeif->phyAddr, 
				dataVal, pGeif->phyId1, pGeif->phyId2));

	return result;
}


/*******************************************************************************
*
* geifPchOEMbitsConfig
* 
* This function sets the OEM bits for PCH/PCH2 adapters.
*
* The four available bits are collectively called OEM bits. We configures the 
* Gbe Disable and LPLU bits. The OEM Write Enable bit and SW Config bit
* are used to determine whether HW should have configured LPLU and Gbe Disable.
* 
* Note: After a warm reset, to the BIOS not via fast reboot(), Gbe may be 
*       disabled hence we need to re-enable it here.
*       
* RETURN: None.
* 
*******************************************************************************/
static void geifPchOEMbitsConfig( GEIF_NET *pGeif )
{
	UINT32 regVal;
	UINT16 result;
	UINT16 oemBits;
	int status;
    

	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));

	status = ERR_OK;
	
	if ( pGeif->did != INTEL_DEVICEID_82579LM2 )
	{
		/* Check Write Enable bit */
		regVal = GEIF_READ_REG( GEIF_EXTCNF_CTRL );
		
		GEIFDBG( GEIF_DBG_INIT, ("%s: EXTCNF_CTRL: 0x%08x\n", pGeif->name, regVal));

		if ((regVal & GEIF_EXTCNF_OEM_WR_ENABLE))
		{
			status = ERR_IF;
		}
	}

	if (status == ERR_OK)
	{
		/* Check if SW should configure else this will be done by the HW */
    	regVal = GEIF_READ_REG( GEIF_FEXTNVM );
    	
    	GEIFDBG( GEIF_DBG_INIT, ("%s: FEXTNVM: 0x%08x\n", pGeif->name, regVal));
					
		if (!(regVal & GEIF_FEXTNVM_SW_CONFIG_ICH))
		{
			status = ERR_IF;
		}
	}
	
	if (status == ERR_OK)
	{
    	/* Read control register to see what bit should be set */
    	regVal = GEIF_READ_REG( GEIF_PHY_CTRL );
    	
		/* Set page 0 and we access these register via phy addres 1 */
		geifPhyWrite( pGeif, 1, MII_PAGE_SELECT, (0 << MII_PAGE_SELECT_SHIFT) );
		
		/* Read the current OEM bits */
		oemBits = geifPhyRead( pGeif, 1, MII_PHY_PM_REG );
		result = oemBits;
		
		/* Gbe may be disabled so we will re-enable it here */ 
		result &= ~(MII_PHY_PM_OEM_GBE_DIS | MII_PHY_PM_OEM_LPLU);
		
		if (regVal & GEIF_PHY_CTRL_GBE_DISABLE)
		{
			result |= MII_PHY_PM_OEM_GBE_DIS;
		}

		if (regVal & GEIF_PHY_CTRL_D0A_LPLU)
		{
			result |= MII_PHY_PM_OEM_LPLU;
		}
		
		/* Restart Auto-negotiation */
		if (result != oemBits)
		{
			/* So new settings are read */
			result |= MII_PHY_PM_OEM_RESTART_AN;
			geifPhyWrite( pGeif, 1, MII_PHY_PM_REG, result );
		}
		else
		{
			geifPhyWrite( pGeif, pGeif->phyAddr, MII_CTRL_REG, (MII_CR_AUTO_EN | MII_CR_RESTART) );
		}
		
		GEIFDBG( GEIF_DBG_INIT, ("%s: PHY_CTRL: 0x%08x oemBits: 0x%04x result: 0x%04x\n", 
					pGeif->name, regVal, oemBits, result));
	}
}


/*******************************************************************************
*
* geifSerdesLinkInit
*
* Initialize and setup the SERDES link.
* 
*
* RETURNS: None.
*
*******************************************************************************/ 
static void geifSerdesLinkInit( GEIF_NET *pGeif )
{
	UINT32 reg;
	UINT32 ctrl;

#ifdef GEIF_POLL_MODE	
	UINT32 pcslsts;
	int count;
#endif	
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));

	/*
	 * On the 82575, SERDES loopback mode persists until it is
	 * explicitly turned off or a power cycle is performed.  A read to
	 * the register does not indicate its status.  Therefore, we ensure
	 * loopback mode is disabled during initialization.
	 */
	/* GEIF_WRITE_REG( GEIF_FEXTNVM4, GEIF_FEXTNVM4_DISABLE_SERDES_LOOPBACK ); */	/* Disable loopback */
	
	/* Force link up, set 1gb */
	reg = GEIF_READ_REG( GEIF_CTRL );

	if (pGeif->did == INTEL_DEVICEID_i210_SERDES)
	{
		reg |= GEIF_CTRL_SLU_BIT |
				//GEIF_CTRL_SPD_10_BIT |
				GEIF_CTRL_SPD_1000_BIT|
		       GEIF_CTRL_FRCSPD_BIT;
	}
	else
	{
		reg |= GEIF_CTRL_SLU_BIT |
	       GEIF_CTRL_SPD_1000_BIT |
	       GEIF_CTRL_FRCSPD_BIT;
	}
	/* Set both sw defined pins on 82575/82576 */
/*
	if (pGeif->did == INTEL_DEVICEID_82576_SERDES)
	{
		reg |= GEIF_CTRL_SWDPIO0_BIT | GEIF_CTRL_SWDPIO1_BIT;
	}
*/
	
	GEIF_WRITE_REG( GEIF_CTRL, reg );
	
	/* Set link mode and power on */
	reg = GEIF_READ_REG( GEIF_CTRLEXT );
	reg |= GEIF_CTRLEXT_LM_SERDES;
	reg &= ~GEIF_CTRLEXT_SDP7_DATA;
	GEIF_WRITE_REG( GEIF_CTRLEXT, reg );

	/* Set switch control to SERDES energy detect */
	reg = GEIF_READ_REG( GEIF_CONNSW );
	reg |= ~GEIF_CONNSW_ENRGSRC;
	GEIF_WRITE_REG( GEIF_CONNSW, reg );

	/*
	 * SERDES mode allows for forcing speed or autonegotiating speed at 1gb.
	 * Autoneg should be the default used by most drivers. This is the
	 * mode that will be compatible with older link partners and switches.
	 */
	reg = GEIF_READ_REG( GEIF_PCSLCTL );
	reg &= ~(GEIF_PCSLCTL_AN_ENABLE | GEIF_PCSLCTL_FLV_LINK_UP |
		GEIF_PCSLCTL_FSD | GEIF_PCSLCTL_FORCE_LINK);

#if 1
	if (pGeif->did == INTEL_DEVICEID_i210_SERDES)
	{
		/* Set PCS register for forced speed */
		reg |= GEIF_PCSLCTL_FLV_LINK_UP |   /* Force link up */
				//GEIF_PCSLCTL_FSV_10 |      /* Force 10   */
				GEIF_PCSLCTL_FSV_1000 |      /* Force 1000  */
		       GEIF_PCSLCTL_FDV_FULL |      /* SERDES Full duplex */
		       GEIF_PCSLCTL_FSD |           /* Force Speed */
		       GEIF_PCSLCTL_FORCE_LINK;     /* Force Link */

		GEIFDBG( GEIF_DBG_INIT, ("%s: Configuring Forced Link, PCSLCTL: 0x%08x\n",
				pGeif->name, reg));
	}
	else
	{
		/* Set PCS register for autoneg */
		reg |= GEIF_PCSLCTL_FSV_1000 |      /* Force 1000 */
		       GEIF_PCSLCTL_FDV_FULL |      /* SERDES Full duplex */
		       GEIF_PCSLCTL_AN_ENABLE |     /* Enable Autoneg */
		       GEIF_PCSLCTL_AN_RESTART;     /* Restart Autoneg */
		       
		       /* GEIF_PCSLCTL_AN_TIMEOUT_EN | */ /* Enable timeout */
		       
		GEIFDBG( GEIF_DBG_INIT, ("%s: Configuring Autoneg, PCSLCTL: 0x%08x\n", 
				pGeif->name, reg));
	}
#else
#warning "***** SERDES forced link speed *****"
		/* Set PCS register for forced speed */
		reg |= GEIF_PCSLCTL_FLV_LINK_UP |   /* Force link up */
		       GEIF_PCSLCTL_FSV_1000 |      /* Force 1000    */
		       GEIF_PCSLCTL_FDV_FULL |      /* SERDES Full duplex */
		       GEIF_PCSLCTL_FSD |           /* Force Speed */
		       GEIF_PCSLCTL_FORCE_LINK;     /* Force Link */
		
		GEIFDBG( GEIF_DBG_INIT, ("%s: Configuring Forced Link, PCSLCTL: 0x%08x\n", 
				pGeif->name, reg));			
#endif

	/* Force flow control */
	reg |= GEIF_PCSLCTL_FORCE_FCTRL;
	
	ctrl = GEIF_READ_REG( GEIF_CTRL );
	ctrl |= (GEIF_CTRL_RFCE_BIT | GEIF_CTRL_TFCE_BIT);
	GEIF_WRITE_REG( GEIF_CTRL, ctrl );
	
	/* Configure and start link */
	GEIF_WRITE_REG( GEIF_PCSLCTL, reg );
				
#ifdef GEIF_POLL_MODE

	/* Wait for link to come up or timeout */
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: waiting for link...\n", pGeif->name, __FUNCTION__));
	
	if (reg & GEIF_PCSLCTL_AN_ENABLE)
	{
		/* Wait for autoneg to complete or timeout */
		count = 0;
		
		do
		{
			vTaskDelay( 1000 );
	    	pcslsts = GEIF_READ_REG( GEIF_PCSLSTS );
	    	count++;
		}
		while (!(pcslsts & GEIF_PCSLSTS_AN_COMPLETE) && (count < 10));
	}
	    
    count = 0;
    
    /* Get the link status */
    while ((count < 20) && 
		(geifSerdesModeGet( pGeif, &pGeif->linkMode, &pGeif->linkStatus ) == ERR_IF))
    {
    	vTaskDelay( 500 );
    	count++;
	}
#endif	

	GEIFDBG( GEIF_DBG_INIT, ("%s: PCSLCTL: 0x%08x PCSLSTS: 0x%08x\n", 
					pGeif->name, GEIF_READ_REG( GEIF_PCSLCTL ), 
					GEIF_READ_REG( GEIF_PCSLSTS )));
}


/*******************************************************************************
*
* geifPhyLinkInit
*
* Initialize and setup the PHY link.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifPhyLinkInit( GEIF_NET *pGeif )
{
	UINT32 ctrl;

#ifdef GEIF_POLL_MODE
	UINT16 miiSts;
	int count;
#endif	
	
	
	/* Initialize the link */
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));
	
	if ( pGeif->did == INTEL_DEVICEID_82573L )
	{
		/* We will establish a link through PHY, and then manually */
	    /* configure the device based on PHY's negotiation results */
		
		ctrl = GEIF_READ_REG( GEIF_CTRL );
		
		ctrl |= (GEIF_CTRL_FRCSPD_BIT | GEIF_CTRL_FRCDPX_BIT | GEIF_CTRL_SLU_BIT);
	    
		/* Set DMA configuration */                            
	    ctrl &= ~GEIF_CTRL_PRIOR_BIT;
	    
	    /* Reset the PHY */
	    GEIF_WRITE_REG( GEIF_CTRL, (ctrl | GEIF_CTRL_PHY_RST_BIT) );
	
	    /* Wait for the PHY to settle down */
	    vTaskDelay( 100 );
	    GEIF_WRITE_REG( GEIF_CTRL, ctrl );
	    vTaskDelay( 1000 );
	}
	else
	{
		if ((pGeif->did != INTEL_DEVICEID_82580_DUAL_COPPER) &&
			(pGeif->did != INTEL_DEVICEID_82580_QUAD_COPPER) &&
			(pGeif->did != INTEL_DEVICEID_i350AMx_COPPER)    &&
			(pGeif->did != INTEL_DEVICEID_i210) &&
			(pGeif->did != INTEL_DEVICEID_i210_SERDES))
		{
			geifPhyReset( pGeif );
		}
		
		/* Start auto negotiation */
		
		if (pGeif->devType == GEIF_DEVTYPE_ICH)
		{
			geifPchOEMbitsConfig( pGeif );
		}
		else
		{
			geifPhyWrite( pGeif, pGeif->phyAddr, MII_CTRL_REG, (MII_CR_AUTO_EN | MII_CR_RESTART) );
		}

#ifdef GEIF_POLL_MODE
		/* Wait for link to come up or timeout */

		GEIFDBG( GEIF_DBG_INIT, ("%s: %s: waiting for link...\n", pGeif->name, __FUNCTION__));
				
		count = 0;

		do
		{
			vTaskDelay( 1000 );
	    	miiSts = geifPhyRead( pGeif, pGeif->phyAddr, MII_STAT_REG );
	    	count++;
		}
		while (!(miiSts & MII_SR_AUTO_NEG) && (count < 10));
#endif
	}

#ifdef GEIF_POLL_MODE
	    
    count = 0;
    
    /* Get the link status */
    while ((count < 20) && 
		(geifPhyModeGet( pGeif, &pGeif->linkMode, &pGeif->linkStatus ) == ERR_IF))
    {
    	vTaskDelay( 500 );
    	count++;
	}
#endif		
}


#if LWIP_NETIF_STATUS_CALLBACK
/*******************************************************************************
*
* geifStatusCallback
*
* Link status change callback function.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifStatusCallback( struct netif *pNetif )
{
#if defined(INCLUDE_GEIF_DEBUG) || defined(INCLUDE_DEBUG_VGA)
	GEIF_NET *pGeif = pNetif->state;
	
#ifdef INCLUDE_DEBUG_VGA	
	char achBuffer[80];
#endif
	
	if (netif_is_up(pNetif)) 
	{
		GEIFDBG( GEIF_DBG_DIAG, ("%s: is configured IP: %s\n", 
			pGeif->name, ip_ntoa(&pNetif->ip_addr)));
	} 
	else 
	{
		GEIFDBG( GEIF_DBG_DIAG, ("%s: is unconfigured\n", pGeif->name));
	}

#ifdef INCLUDE_DEBUG_VGA	
	if (pGeif->num == VGA_GEIF_NUM)
	{
		/* Note: using ip4_addr1_16() etc here as ip_ntoa() is not reentrant */
		sprintf( achBuffer, "IP: %u.%u.%u.%u",
					ip4_addr1_16(&pNetif->ip_addr.addr), ip4_addr2_16(&pNetif->ip_addr.addr),
					ip4_addr3_16(&pNetif->ip_addr.addr), ip4_addr4_16(&pNetif->ip_addr.addr));
		vgaPutsXY( 49,23, achBuffer );
		vgaClearToEol();
	}
#endif

#endif /* defined(INCLUDE_GEIF_DEBUG) || defined(INCLUDE_DEBUG_VGA) */

}
#endif /* LWIP_NETIF_STATUS_CALLBACK */


/*******************************************************************************
*
* geifLinkUpdate
*
* Update link status.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifLinkUpdate( struct netif *pNetif )
{	
	GEIF_NET *pGeif;
	UINT32 status;
	
#ifdef INCLUDE_DEBUG_VGA
	char achBuffer[80];
#endif

	pGeif = pNetif->state;
	
	GEIF_CLRBIT( GEIF_CTRL, GEIF_CTRL_SPEED_BITS );
	
	/* Set speed to match physical interface (PHY/SERDES) */
	
	GEIF_CLRBIT( GEIF_RCTL, GEIF_RCTL_SBP_BIT );
	
	switch (IFM_SUBTYPE(pGeif->linkMode))
	{
		case IFM_1000_T:
		case IFM_1000_SX:
			pGeif->linkSpeed = 1000;
			GEIF_SETBIT( GEIF_CTRL, GEIF_CTRL_SPD_1000_BIT );
			break;
		
		case IFM_100_TX:
			pGeif->linkSpeed = 100;
			GEIF_SETBIT( GEIF_CTRL, GEIF_CTRL_SPD_100_BIT );
			break;
		
		case IFM_10_T:
			pGeif->linkSpeed = 10;
			
			GEIF_SETBIT( GEIF_CTRL, GEIF_CTRL_SPD_10_BIT );
			break;
		
		default:
			pGeif->linkSpeed = 0;	
			break;
	}
	
	/* Set duplex to match physical interface (PHY/SERDES) */
	
	if ((pGeif->linkMode & IFM_GMASK) == IFM_FDX)
	{
	    GEIF_SETBIT( GEIF_CTRL, GEIF_CTRL_FD_BIT );
	}
	else
	{
	    GEIF_CLRBIT( GEIF_CTRL, GEIF_CTRL_FD_BIT );
	}
	
	/* Announce link up/down. */
	
	status = GEIF_READ_REG( GEIF_STATUS );
	
	GEIFDBG( GEIF_DBG_DIAG, ("%s: %s: linkStatus: 0x%08x status: 0x%08x\n", 
			pGeif->name, __FUNCTION__, pGeif->linkStatus, status));
	
	if ( pGeif->linkStatus & IFM_ACTIVE )
	{
		status |= GEIF_STATUS_LU_BIT;
		
		if (netif_is_link_up( pNetif ) == 0)
		{
			netif_set_link_up( pNetif );
		}
	}
	else
	{
		status &= ~GEIF_STATUS_LU_BIT;
		
		netif_set_link_down( pNetif );
	}
	
	GEIFDBG( GEIF_DBG_DIAG, ("%s: %s: Link is %s Speed: %u Flags: 0x%02x\n", 
			pGeif->name, __FUNCTION__, 
			((status & GEIF_STATUS_LU_BIT)?"up":"down"),
			pGeif->linkSpeed, pNetif->flags));
			
#ifdef INCLUDE_DEBUG_VGA
		if (pGeif->num == VGA_GEIF_NUM)
		{
			sprintf( achBuffer, "%s: Link is %s Speed: %u Flags: 0x%02x", 
					pGeif->name, ((status & GEIF_STATUS_LU_BIT)?"up":"down"),
					pGeif->linkSpeed, pNetif->flags );
			vgaPutsXY( 0, 24, achBuffer );
			vgaClearToEol();
		}
#endif
}


#ifdef INCLUDE_GEIF_DEBUG
/*******************************************************************************
*
* geifDbgBuffer
*
* Write given buffer as debug output.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifDbgBuffer( UINT32 dbgLvl, UINT32 addr, UINT8 *buf, UINT32 size )
{
    UINT32 i;
    UINT32 n;
	char textBuf[81];
	int len;

    
    for (n = 0; n < size; n+=16)
    {
		len = 0;
		sprintf( &textBuf[len], "%08x ", addr );
		len += 9;
		addr += 16;
        
		for (i = 0; i < 16; i++)
		{
			if( (n + i) < size )
			{
				sprintf( &textBuf[len], "%02x ", buf[(n + i)] );
			}
			else
			{
				sprintf( &textBuf[len], "   " );
			}
			
			len += 3;
        }

		sprintf( &textBuf[len], "\n" );
		
		GEIFDBG( dbgLvl, ("%s", textBuf));
    }    
}
#endif


/*******************************************************************************
*
* geifEncapPkt
*
* This function encapsulate an outbound packet in transmit descriptors 
* ready for a packet transmit operation.
*
*
* RETURNS: Pointer to descriptor (last) if successful else NULL.
* 
*******************************************************************************/
static UINT8 *geifEncapPkt( GEIF_NET *pGeif, struct pbuf *pPkt )
{
	struct pbuf *q;
	UINT32 txdBufAddr;
	UINT16 len;
	UINT8 *pDesc;
	UINT32 descCount;
	UINT32 pbufCount;
	UINT32 tempTail;

	
	pDesc = NULL;   
	descCount = 0;
	pbufCount = 0;
	len = 0;
	
	
	tempTail = pGeif->txDescTail;
	
	for( q = pPkt; q != NULL; q = q->next ) 
	{
		GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: pbuf[%u] len: %u\n", 
				pGeif->name, __FUNCTION__, pbufCount, q->len));

		/* If this is the first time round the loop or there's not enough room */
		/* left in the current descriptor buffer get the next descriptor       */
		if ( (descCount == 0) || (len + q->len > GEIF_TX_BUF_SIZE) )
		{
			if (pDesc != NULL)
			{
				/* We've finished with this descriptor, prepare it */
				/* for transmit before getting the next one */ 
			
				/* Set up the length field */
				GEIF_WRITE_DESC_WORD( pDesc, TXD_LENGTH_OFFSET, len );
				
				/* Set up the command field */
				GEIF_WRITE_DESC_BYTE( pDesc, TXD_CMD_OFFSET,
				                            (TXD_CMD_IDE | TXD_CMD_RS) );
				                            
				/* Zero the status field in Tx Desc */
				GEIF_WRITE_DESC_BYTE( pDesc, TXD_STATUS_OFFSET, 0 );
				
#ifdef INCLUDE_GEIF_DEBUG		
				geifShowTxDesc( GEIF_DBG_TRACE, pGeif, tempTail, (GEIF_TXD *) pDesc );
#endif
				tempTail = (tempTail + 1) % (GEIF_TXD_CNT);
			}
			
			/* Get the next TX descriptor */
			pDesc = GEIF_GET_TXD_ADDR( tempTail );
		
			if (!(GEIF_READ_DESC_BYTE( pDesc, TXD_STATUS_OFFSET ) & TXD_STAT_DD))
			{
				GEIFERR(("%s: %s: Error - no TX descriptor available\n", 
						pGeif->name, __FUNCTION__));
				
			    return NULL;  
			}
			
			descCount++;
		
			/* Get descriptors buffer address */
			GEIF_READ_DESC_LONG( pDesc, TXD_BUFADRLOW_OFFSET, txdBufAddr );
		}

		/* Copy the data one pbuf at a time. The size of the */
		/* data in each pbuf is kept in the ->len variable.  */
		if( q == pPkt ) 
		{
			bcopy( &( ( char * ) q->payload )[ ETH_PAD_SIZE ], (char *)txdBufAddr, (q->len - ETH_PAD_SIZE) );
			txdBufAddr += q->len - ETH_PAD_SIZE;
		} 
		else 
		{
			bcopy( (char *) q->payload, (char *)txdBufAddr, q->len );
			txdBufAddr += q->len;
		}
/*		
#ifdef INCLUDE_GEIF_DEBUG
		geifDbgBuffer( GEIF_DBG_TRACE, 0, (UINT8 *) q->payload, q->len );
#endif
*/
		len += q->len;
		
		if ( q->next == NULL )
		{
			/* This is the last packet in the chain, so we're done */
			
			if (len < ETHER_MIN_LEN)
			{
			    len = ETHER_MIN_LEN;
			}
			
			/* Set up the length field */
			GEIF_WRITE_DESC_WORD( pDesc, TXD_LENGTH_OFFSET, len );
			
			/* Set up the command field */
			GEIF_WRITE_DESC_BYTE( pDesc, TXD_CMD_OFFSET, 
			                           (TXD_CMD_EOP | TXD_CMD_IFCS | 
			                            TXD_CMD_IDE | TXD_CMD_RS) );
			                            
			/* Zero the status field in Tx Desc */
			GEIF_WRITE_DESC_BYTE( pDesc, TXD_STATUS_OFFSET, 0 );
			
#ifdef INCLUDE_GEIF_DEBUG		
			geifShowTxDesc( GEIF_DBG_TRACE, pGeif, tempTail, (GEIF_TXD *) pDesc );
#endif
		}
		
		pbufCount++;
	}
	
	/* Update the tail pointer */
	GEIF_GET_TXD_TAIL_UPDATE( pGeif->txDescTail, descCount );
	
	GEIFDBG( GEIF_DBG_TX, ("%s: %s: pbufCount: %u descCount: %u txDescTail: %u CPU:%d\n", 
		pGeif->name, __FUNCTION__, pbufCount, descCount, pGeif->txDescTail, sPortGetCurrentCPU()));
		
	return pDesc;
}


#ifdef GEIF_POLL_MODE
/*******************************************************************************
*
* geifPollSend
*
* This function sends a packet using polling mode.
*
*
* RETURNS: GEIF_PKT_OK if successful else error code.
* 
*******************************************************************************/
static int geifPollSend( GEIF_NET *pGeif, struct pbuf *pPkt )
{
	UINT32 count;
	UINT8 *pDesc;
	int result;


	// GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: tot_len: %u\n", pGeif->name, __FUNCTION__, pPkt->tot_len));
	
	/* Check if the transmitter idle */ 
	if (GEIF_READ_REG( GEIF_TDH0 ) != GEIF_READ_REG( GEIF_TDT0 ))
	{
		GEIFERR(("%s: %s: Error - TX Active\n", pGeif->name, __FUNCTION__));
		return GEIF_PKT_TX_ERROR;
	}    

	/* Encapsulate the packet */
	pDesc = geifEncapPkt( pGeif, pPkt );
	
	if (pDesc != NULL)
	{
		/* Issue the transmit command */
		GEIF_WRITE_REG( GEIF_TDT0, pGeif->txDescTail );
	
		result = GEIF_PKT_NONE;
		
		/* Wait for packet to be sent or 2 second timeout */
		for ( count = 0; count < 20000; count++ )
		{
			if ((UINT8)GEIF_READ_DESC_BYTE( pDesc, TXD_STATUS_OFFSET) & TXD_STAT_DD )
		    {
				result = GEIF_PKT_OK;
				break;
			}
			else
			{
				sysDelayMicroseconds(100); // 100 us
			}
		}
		
		/* Check if we timed out */
		if ((result != GEIF_PKT_OK) && (count >= 20000))
		{
			GEIFERR(("%s: %s: Error - TX timeout, TXD_STAT: 0x%x\n", 
					pGeif->name, __FUNCTION__, GEIF_READ_DESC_BYTE( pDesc, TXD_STATUS_OFFSET )));
					
			result = GEIF_PKT_TX_TIMEOUT;
		}
		
		/* Wait for transmitter idle or 2 second timeout */
		for ( count = 0; count < 20000; count++ )
		{ 
			if (GEIF_READ_REG( GEIF_TDH0 ) == GEIF_READ_REG( GEIF_TDT0 ))
			{
				break;
			}
			else
			{
				sysDelayMicroseconds(100); // 100 us
			}
		}
		
	}
	else
	{
		result = GEIF_PKT_DESC_ERROR;
	}
	
	// GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: result: %d\n", pGeif->name, __FUNCTION__, result));
	
	return result;
}
#else
/*******************************************************************************
*
* geifSend
*
* This function sends a packet.
*
*
* RETURNS: GEIF_PKT_OK if successful else error code.
* 
*******************************************************************************/
static int geifSend( GEIF_NET *pGeif, struct pbuf *pPkt )
{
	UINT8 *pDesc;
	int result;
	

	GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: len: %u\n", pGeif->name, __FUNCTION__, pPkt->tot_len));

    /* Encapsulate the packet */
	pDesc = geifEncapPkt( pGeif, pPkt );
	
	if (pDesc != NULL)
	{
		/* Issue the transmit command */
		GEIF_WRITE_REG( GEIF_TDT0, pGeif->txDescTail );
		
		result = GEIF_PKT_OK;
	}
	else
	{
		result = GEIF_PKT_DESC_ERROR;
	}
	
	return result;
}
#endif /* GEIF_POLL_MODE */


/*******************************************************************************
*
* geifLowLevelOutput
*
* This function outputs the packet.
* 
* The packet is contained in the pbuf that is passed to the function. This pbuf
* might be chained.
*
*
* RETURNS: ERR_OK if the packet could be sent.
* 
* Note: Returning ERR_MEM here can lead to strange results. You might consider 
*       waiting for space to become available since the stack doesn't retry to 
*       send a packet dropped because of memory failure (except for the TCP timers).
* 
*******************************************************************************/
static err_t geifLowLevelOutput( struct netif *pNetif, struct pbuf *pPkt )
{
	GEIF_NET *pGeif;	
	struct eth_hdr *pxHeader;
	int status;
	err_t result;


	pGeif = pNetif->state;
	status = GEIF_PKT_TX_ERROR;
	

	if (!(pGeif->linkStatus & IFM_ACTIVE)) /* Check link is up */
	{
		GEIFERR(("%s: %s: Error - link down, pPkt: 0x%x len: %u\n", 
				pGeif->name, __FUNCTION__, (UINT32)pPkt, pPkt->tot_len));
				
        result = ERR_CONN;
    }
	else if( pPkt->tot_len > ETHER_MAX_LEN ) /* Check packet size */
	{
		/* Packet too big */
		GEIFERR(("%s: %s: Error - packet too big, pPkt: 0x%x len: %u\n", 
				pGeif->name, __FUNCTION__, (UINT32)pPkt, pPkt->tot_len));
				
		result = ERR_BUF;
	}
	else /* OK to send packet */
	{
		GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: len: %u\n", pGeif->name, __FUNCTION__, pPkt->tot_len));

		/* Lock access while we send */
		sys_mutex_lock( &pGeif->tx_mutex );

		/* Send the packet */
#ifdef GEIF_POLL_MODE
		status = geifPollSend( pGeif, pPkt );
#else
		status = geifSend( pGeif, pPkt );
#endif
		
		if( status != GEIF_PKT_OK ) 
		{
			result = ERR_BUF;
		}
		else
		{
			LINK_STATS_INC( link.xmit );
			atomic32Inc( &pGeif->txPktCount );
			
			snmp_add_ifoutoctets( pNetif, (pPkt->tot_len - ETH_PAD_SIZE) );
			pxHeader = ( struct eth_hdr * )pPkt->payload;

			if( ( pxHeader->dest.addr[ 0 ] & 1 ) != 0 ) 
			{
				/* broadcast or multicast packet*/
				snmp_inc_ifoutnucastpkts( pNetif );
			} 
			else 
			{
				/* unicast packet */
				snmp_inc_ifoutucastpkts( pNetif );
			}
			
			result = ERR_OK;
		}
		
		/* We'er done, unlock access */
		sys_mutex_unlock( &pGeif->tx_mutex );
		
		GEIFDBG( GEIF_DBG_TX, ("%s: %s: pPkt: 0x%x len: %u status: %d\n", 
				pGeif->name, __FUNCTION__, (UINT32)pPkt, pPkt->tot_len, status ));
	}
	
	/* Packet send failed, log error */
	if (result != ERR_OK)
	{
		LINK_STATS_INC( link.lenerr );
		LINK_STATS_INC( link.drop );
		atomic32Inc( &pGeif->txPktDropped );
		snmp_inc_ifoutdiscards( pNetif );
	}
	
	return result;	
}


/*******************************************************************************
*
* geifOutput
*
* This function is called by the TCP/IP stack when an IP packet should be
* sent. It uses the ethernet ARP module provided by lwIP to resolve the
* destination MAC address. The ARP module will later call our low level
* output function.
*
*
* RETURNS: ERR_OK if successful or an error code.
* 
*******************************************************************************/ 
static err_t geifOutput( struct netif *pNetif, struct pbuf *pPkt, ip_addr_t *ipaddr )
{
	GEIF_NET *pGeif = pNetif->state;
	err_t result;
	
	
	if (!(pGeif->linkStatus & IFM_ACTIVE)) /* Check link is up */
	{
		GEIFDBG( GEIF_DBG_TX, ("%s: %s: Error - link down, pPkt: 0x%x len: %u\n", 
				pGeif->name, __FUNCTION__, (UINT32)pPkt, pPkt->tot_len));
		
		LINK_STATS_INC( link.lenerr );
		LINK_STATS_INC( link.drop );
		atomic32Inc( &pGeif->txPktDropped );
		snmp_inc_ifoutdiscards( pNetif );
				
        result = ERR_CONN;
    }
	else
	{
		GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: pPkt: 0x%x len: %u\n", 
				pGeif->name, __FUNCTION__, (UINT32)pPkt, pPkt->tot_len));
	
		/* Make sure only one thread is in this function. */
	    sys_mutex_lock( &pGeif->op_mutex );
	
		result = etharp_output( pNetif, pPkt, ipaddr);
		
		sys_mutex_unlock( &pGeif->op_mutex );
		
		if (result != ERR_OK)
		{
			GEIFERR(("%s: %s: Error - etharp_output failed, error: %d\n", 
				pGeif->name, __FUNCTION__, result));
		}
	}
	
	return result;
}


/*******************************************************************************
*
* geifLowLevelInput
*
* This function inputs a packet. If a new received packet is available, 
* it is copied into an allocated pbuf chain and returned to the caller.
*
*
* RETURNS: the pbuf filled with the received packet (including MAC header) or
*          NULL on error.
* 
*******************************************************************************/ 
static struct pbuf *geifLowLevelInput( GEIF_NET *pGeif, int *pStatus )
{
	UINT8  *pRxDesc;
    UINT16 len;
    UINT8  rxStatus;
    UINT8  rxdErr;
    UINT32 bufAddr;
    UINT32 pbufCount;
    struct pbuf *pPkt = NULL, *q;

	
	pRxDesc = GEIF_GET_RXD_ADDR( pGeif->rxDescTail );
	
	/* Check RX descriptor status */
	
	rxStatus = GEIF_READ_DESC_BYTE( pRxDesc, RXD_STATUS_OFFSET );
	
	if (!(rxStatus & RXD_STAT_DD) || !(rxStatus & RXD_STAT_EOP))
	{   
		/* No packet ready */
		*pStatus = GEIF_PKT_NONE;
		return NULL;
	}
	
	rxdErr = GEIF_READ_DESC_BYTE( pRxDesc, RXD_ERROR_OFFSET ); 
	
	if (rxdErr & (UINT8)(~(RXD_ERROR_IPE | RXD_ERROR_TCPE)))
	{
		GEIFERR(("%s: %s: Error - packet error 0x%x\n", 
					pGeif->name, __FUNCTION__, rxdErr));
		geifRxDesUpdate( pGeif );
		*pStatus = GEIF_PKT_RX_ERROR;
		return NULL;
	}
	
	/* Get the packet length */
	/* Max. packet length: 1514 bytes */
	GEIF_READ_DESC_WORD( pRxDesc, RXD_LENGTH_OFFSET, len );
	
	len &= ~0xc000;
	
	GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: pRxDesc: 0x%x len: %u\n", 
				pGeif->name, __FUNCTION__, (UINT32) pRxDesc, len));

	if (len > (ETHERMTU + ETHER_HDR_LEN) || 
		len < ETHER_MIN_LEN)
	{        
		GEIFERR(("%s: %s: Error - invalid packet length: %u\n", 
				pGeif->name, __FUNCTION__, len));
		geifRxDesUpdate( pGeif );
		*pStatus = GEIF_PKT_INVALID_LEN;
		return NULL;
	}
	
	/* Get the descriptors buffer and copy packet to callers pbuf */
	GEIF_READ_DESC_LONG( pRxDesc, RXD_BUFADRLOW_OFFSET, bufAddr );
	
	/* We allocate a pbuf chain of pbufs from the pool. */
#if ETH_PAD_SIZE
	len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif	

	pPkt = pbuf_alloc( PBUF_RAW, len, PBUF_POOL );

	if( pPkt != NULL )
	{
#if ETH_PAD_SIZE
		pbuf_header( pPkt, -ETH_PAD_SIZE ); /* drop the padding word */
#endif
		/* We iterate over the pbuf chain until we    */
		/* have read the entire packet into the pbuf. */
		
		pbufCount = 0;
			
		for( q = pPkt; q != NULL; q = q->next ) 
		{
			/* Read enough bytes to fill this pbuf in the chain. The
			* available data in the pbuf is given by the q->len
			* variable.
			* This does not necessarily have to be a memory copy, you can also preallocate
			* pbufs for a DMA-enabled MAC and after receiving truncate it to the
			* actually received size. In this case, ensure the usTotalLength member of the
			* pbuf is the sum of the chained pbuf len members.
			*/
			bcopy( (char *) bufAddr, (char *) q->payload, q->len );
			bufAddr += q->len;
			
			GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: pbuf[%u] len: %u\n", 
						pGeif->name, __FUNCTION__, pbufCount, q->len));
						
#ifdef INCLUDE_GEIF_DEBUG								
			geifDbgBuffer( GEIF_DBG_TRACE, 0, (UINT8 *) q->payload, 16 );
#endif
			pbufCount++;
		}
		
#if ETH_PAD_SIZE
		pbuf_header( p, ETH_PAD_SIZE ); /* reclaim the padding word */
#endif
		*pStatus = GEIF_PKT_OK;			
	}
	else
	{
		/* Insufficient memory to handle packet */
		GEIFERR(("%s: %s: Error - packet length: %u, too big for pbuf_alloc()\n",
				 pGeif->name, __FUNCTION__, len));
		*pStatus = GEIF_PKT_MEM_ERROR;
		LINK_STATS_INC(link.memerr);		
	} 
	
	GEIFDBG( GEIF_DBG_RX, ("%s: %s: pPkt: 0x%x tot_len: %u pbufCount: %u CPU:%d\n", 
			pGeif->name, __FUNCTION__, (UINT32)pPkt, pPkt->tot_len, pbufCount, sPortGetCurrentCPU()));
	
	/* Release descriptor */
	geifRxDesUpdate( pGeif );
	
	return pPkt;
}


/*******************************************************************************
*
* geifInput
*
* This function should be called when a packet is ready to be read
* from the interface. It uses the function geifLowLevelInput() that
* should handle the actual reception of bytes from the network
* interface. Then the type of the received packet is determined and
* the appropriate input function is called.
*
*
* RETURNS: GEIF_PKT_OK if packet received and processed or an error code.
* 
*******************************************************************************/ 
static int geifInput( struct netif *pNetif )
{
	GEIF_NET *pGeif = pNetif->state;
	struct eth_hdr *ethhdr;
	struct pbuf *pPkt;
	int status = GEIF_PKT_NONE;
	err_t err;
	
	
#ifndef GEIF_POLL_MODE
	GEIFDBG( GEIF_DBG_TRACE, ("\n%s: %s: start\n", pGeif->name, __FUNCTION__));
#endif
	
	/* Get the next packet. */
	pPkt = geifLowLevelInput( pGeif, &status );
	
	/* If packet could be read then process, else silently ignore */
	if ( pPkt != NULL )
	{
		LINK_STATS_INC( link.recv );
		atomic32Inc( &pGeif->rxPktCount );
		
		/* Points to packet payload, which starts with an Ethernet header */
		ethhdr = pPkt->payload;

		switch (htons(ethhdr->type)) 
		{
			/* ARP or IP packet? */
			case ETHTYPE_ARP:
					GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: ARP received\n", pGeif->name, __FUNCTION__));
			case ETHTYPE_IP:
#if PPPOE_SUPPORT
			/* PPPoE packet? */
			case ETHTYPE_PPPOEDISC:
			case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
		
				GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: sending to tcpip thread\n", pGeif->name, __FUNCTION__));

				/* Send tcpip_thread the full packet to process */
				err = pNetif->input( pPkt, pNetif );
				
				if ( err != ERR_OK )
				{ 
					GEIFERR(("%s: %s: Error - IP input failed, error: %d\n", 
							pGeif->name, __FUNCTION__, err));
							
					/* Packet is dropped, possible future enhancement would */
					/* be to resend to tcpip_thread when queue is not full  */
					pbuf_free( pPkt );
					pPkt = NULL;
					atomic32Inc( &pGeif->rxPktDropped );
					status = GEIF_PKT_TCPIP_ERROR;
				}
				else
				{
					status = GEIF_PKT_OK;
				}
				
				break;
		
			default:
				GEIFERR(("%s: %s: Error - invalid Ethernet header type: 0x%x\n", 
							pGeif->name, __FUNCTION__, htons(ethhdr->type)));
				pbuf_free( pPkt );
				pPkt = NULL;
				LINK_STATS_INC(link.drop);
				atomic32Inc( &pGeif->rxPktDropped );
				status = GEIF_PKT_INVALID;
				break;
		}
	}
	else
	{
		if (status != GEIF_PKT_NONE)
		{
			LINK_STATS_INC(link.drop);
			atomic32Inc( &pGeif->rxPktDropped );
		}	
	}
	
	GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: done status: %d\n", pGeif->name, __FUNCTION__, status));
	
	return status;
}


#ifndef GEIF_POLL_MODE
/*******************************************************************************
*
* geifTxDesClean
*
* Verify transmission done by checking descriptor status.
*
*
* RETURNS: None.
* 
*******************************************************************************/ 
static void geifTxDesClean( GEIF_NET *pGeif )
{
	UINT32 curCons;
	UINT32 index;
	UINT8 sts;
	UINT8 *pDesc;
	
	
	curCons = GEIF_READ_REG( GEIF_TDH0 );
	
	GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: TX interrupt received curCons: %u\n", 
				pGeif->name, __FUNCTION__, curCons));
	
	index = (pGeif->txDescLastCheck + 1) % GEIF_TXD_CNT;

	while(index != curCons)
	{
		pDesc = GEIF_GET_TXD_ADDR( index );
		
		sts = GEIF_READ_DESC_BYTE( pDesc, TXD_STATUS_OFFSET );
		
		GEIFDBG( GEIF_DBG_TRACE, ("%s: %s: pTxDesc[%03u]: 0x%x  sts: 0x%02x\n", 
					pGeif->name, __FUNCTION__, index, pDesc, sts));
		
		if (!(sts & TXD_STAT_DD))
		{
			GEIFERR(("%s: %s: Error - TX descriptor still in use\n", 
						pGeif->name, __FUNCTION__));
		    break;  
		}
		
		pGeif->txDescLastCheck = index;
		index = (pGeif->txDescLastCheck + 1) % GEIF_TXD_CNT;
	}
	
}
#endif


/*******************************************************************************
*
* geifIntTask
*
* Packet reception task.
*
*
* RETURNS: None.
* 
*******************************************************************************/  
static void geifIntTask( void *pvParameters )
{
	struct netif *pNetif;
	UINT32 intStatus;
	UINT32 pktCount;
	UINT32 loopCount;
	UINT32 rxdCheckCount;
	GEIF_NET *pGeif;

#if defined (INCLUDE_DEBUG_VGA) || defined (INCLUDE_GEIF_DEBUG)
	portSHORT cpuNo;
	UINT32 taskNum;
#endif

#ifdef INCLUDE_DEBUG_VGA
	char achBuffer[80];
	int currentLine;
	
	currentLine = atomic32Inc( (UINT32 *) &startLine ); 
#endif

	pNetif = (struct netif *) pvParameters;
	pGeif = pNetif->state; 

#if defined (INCLUDE_DEBUG_VGA) || defined (INCLUDE_GEIF_DEBUG)
	cpuNo = sPortGetCurrentCPU();
	taskNum = uxTaskNumGet( cpuNo, NULL );
#endif

	pktCount = 0;
	loopCount = 0;
	intStatus = 0;

	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: CPU:%d T%02u: starting, pNetif: 0x%x\n", 
			pGeif->name, __FUNCTION__, cpuNo, taskNum, (UINT32)pNetif));

#ifdef INCLUDE_DEBUG_VGA
	// if (pGeif->num == VGA_GEIF_NUM)
	{
		vgaClearLine( currentLine );
	}
#endif

	for( ;; )
	{
#ifdef INCLUDE_DEBUG_VGA
		// if (pGeif->num == VGA_GEIF_NUM)
		{
			/* Read again to check CPU number doesn't get corrupted! */ 
			cpuNo = sPortGetCurrentCPU();
			
#ifdef GEIF_POLL_MODE
			if ( (loopCount == 200) || (loopCount % 200 == 0) )
			{
				sprintf( achBuffer, "CPU:%d T%02u: %s    Count: %u Polling PKT: %u", 
							cpuNo, taskNum, pGeif->name, (loopCount/200), pktCount );
			}
#else
			sprintf( achBuffer, "CPU:%d T%02u: %s    Count: %u RX: %u PKT: %u TX: %u", 
					cpuNo, taskNum, pGeif->name, loopCount,
					atomic32Get( &pGeif->rxIntCount ), 
					pktCount, atomic32Get( &pGeif->txIntCount ) );
#endif
			vgaPutsXY( 1, currentLine, achBuffer );
		}
#endif

#ifdef GEIF_POLL_MODE
		/* Get the next packet. */
		if ( geifInput( pNetif ) == GEIF_PKT_OK )
		{
			pktCount++;
		}
		else
		{
			/* Give other tasks a chance to run. */
			vTaskDelay( 5 );
		}
#else
		/* Wait for signal from ISR */
		// sys_sem_wait( &pGeif->rx_sem );
		if ( sys_arch_sem_wait( &pGeif->rx_sem, 1000 ) != SYS_ARCH_TIMEOUT )
		{
			atomic32Inc( &pGeif->rxSigCount );

			/* Get and clear interrupt status */
			intStatus = atomic32Set( &pGeif->intStatus, 0 );
			
			if (intStatus != 0)
			{
				/* Unlock our interrupts */
				GEIF_WRITE_REG( GEIF_IMS, GEIF_INTRS );
				
				/* Process receive interrupts */
				if (intStatus & GEIF_RXINTRS)
				{
					rxdCheckCount = 32; /* GEIF_RXD_CNT */
					
					/* Get the next packet(s), upto rxdCheckCount */ 
					while ( (geifInput( pNetif ) == GEIF_PKT_OK) &&
							(rxdCheckCount > 0) ) 
					{
						pktCount++;
						rxdCheckCount--;
						
						if ( pktCount % 32 == 0 )
						{
							vTaskDelay( 5 );
						}
					}
				}
				
				if (intStatus & GEIF_TXINTRS)
				{
					geifTxDesClean( pGeif );
				}
				
				/* Process link change interrupts */
				if (intStatus & GEIF_LINKINTRS)
				{
					if ( pGeif->linkType == GEIF_LINKTYPE_SERDES )
					{
						geifSerdesModeGet( pGeif, &pGeif->linkMode, &pGeif->linkStatus );
					}
					else
					{
						geifPhyModeGet( pGeif, &pGeif->linkMode, &pGeif->linkStatus );
					}
					
					geifLinkUpdate( pNetif );
				}
			}
		}
#endif

		loopCount++;
	}
}


#ifndef GEIF_POLL_MODE
/*******************************************************************************
*
* geifIsr
*
* This is the Interrupt Service Routine.
*
*
* RETURNS: None.
* 
*******************************************************************************/ 
#ifdef GEIF_SHARED_ISR	
static void geifIsr( GEIF_NET *pGeif )
{
#else
static void geifIsr( void )
{
	GEIF_NET *pGeif = pGlobalGei;
#endif
	UINT32 intIcr;
	portSHORT cpuNo;
	UINT32 maskflag = 0;
	
	
	if (GEIF_READ_REG( GEIF_IMS ) & GEIF_INTRS )
    {
    	/* Mask our interrupts while we process any existing */
		GEIF_WRITE_REG( GEIF_IMC, GEIF_IMC_ALL_BITS );
		maskflag = 1;
    }
   
    /* Read and clear interrupts */
	intIcr = GEIF_READ_REG( GEIF_ICR );
    
	/*
     * Make sure there's really an interrupt event pending for us.
     * Since we're a PCI device, we may be sharing the interrupt line.
     */
	intIcr &= GEIF_INTRS;
	     
    if (intIcr != 0)
	{
		atomic32Inc( &pGeif->intCount );
		
		/* Debug - toggle User LED */
		// sysOutPort8( 0x211, sysInPort8( 0x211 ) ^ 0x1 );
			
		/* Process receive interrupts */
		if (intIcr & GEIF_RXINTRS)
		{
			atomic32Inc( &pGeif->rxIntCount );
	    }
	    
	    /* Process transmit interrupts */
	    if (intIcr & GEIF_TXINTRS)
	    {
			atomic32Inc( &pGeif->txIntCount );
		}
		
		/* Process link change interrupts */
		if (intIcr & GEIF_LINKINTRS)
	    {
			atomic32Inc( &pGeif->linkIntCount );
		}
		
		/* Save interrupt status for geifIntTask */
		atomic32Or( &pGeif->intStatus, intIcr );
	
		pGeif->higherPriTaskWoken = pdFALSE;
		
		/* Unblock geifIntTask by releasing the semaphore. */
		/* Note: we use a direct call to the FreeRTOS xSemaphoreGiveFromISR() */
		/*       the sys_arch API cannot be used here. */
    	if (xSemaphoreGiveFromISR( pGeif->rx_sem, &pGeif->higherPriTaskWoken, &cpuNo ))
    	{
    		atomic32Inc( &pGeif->taskSigCount );
		}
		else
		{
			atomic32Inc( &pGeif->taskSigErrors );
		}
 	
    	if( pGeif->higherPriTaskWoken )
    	{
    		/* If a task of higher priority than the interrupted task */
	    	/* was unblocked by the ISR then ensure context switch.   */
    		portYIELD( cpuNo );
    	}
	}
	else
	{ 
	    if(maskflag)
	    {
	    	/* Unlock our interrupts */
			GEIF_WRITE_REG( GEIF_IMS, GEIF_INTRS );
	    }
	}
}
#endif /* GEIF_POLL_MODE */


/*******************************************************************************
*
* geifGetMacAddr
*
* This function gets the given devices MAC address and stores it in the callers
* buffer. Assumes the buffer is large enough 
*
*
* RETURNS: ERR_OK or E__DEVICE_NOT_FOUND
* 
*******************************************************************************/ 
int geifGetMacAddr( char *pName, UINT8 *pMacAddr )
{
	int i;
	struct netif *pNetif;
	int result;
	
	
	pNetif = netif_find( pName );
	
	if (pNetif != NULL)
	{
		for (i = 0; i < 6; i++)
		{
			pMacAddr[i] = pNetif->hwaddr[i];
		}
		
		result = ERR_OK;
	}
	else
	{
		for (i = 0; i < 6; i++)
		{
			pMacAddr[i] = 0;
		}
		
		result = E__DEVICE_NOT_FOUND;
	}
	
	return result;
}


/*******************************************************************************
*
* geifGetStats
*
* This function gets statistic informatiom in a formated string.
*
*
* RETURNS: length of string in buffer.
* 
*******************************************************************************/ 
size_t geifGetStats( char *pName, signed char *pcWriteBuffer, size_t xWriteBufferLen )
{
	size_t len;
	struct netif *pNetif;
	GEIF_NET *pGeif;
	
	
	pNetif = netif_find( pName );
	len = 0;
	
	/* Check interface found and the buffer is sized for our needs */
	/* We allow 60 characters per line: 60 * 13 = 780              */
	if ( (pNetif != NULL) && (pcWriteBuffer != NULL) && (xWriteBufferLen >= 780) )
	{
		pGeif = pNetif->state;
		
		pcWriteBuffer[0] = 0;
		
		sprintf( (char *) pcWriteBuffer, "\r\nDevice      : %s (%03u:%02u:%02u) on CPU%d\r\n",
						pName,
						PCI_PFA_BUS(pGeif->pfa), PCI_PFA_DEV(pGeif->pfa), 
						PCI_PFA_FUNC(pGeif->pfa), pGeif->cpuNo );
		len = strlen( (char *) pcWriteBuffer );
		
		
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "MAC address : %02X:%02X:%02X:%02X:%02X:%02X\r\n", 
					pNetif->hwaddr[0], pNetif->hwaddr[1], pNetif->hwaddr[2], 
					pNetif->hwaddr[3], pNetif->hwaddr[4], pNetif->hwaddr[5] );
		len = strlen( (char *) pcWriteBuffer );
		
		/* Note: using ip4_addr1_16() etc here as ip_ntoa() is not reentrant */
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "IP address  : %u.%u.%u.%u  mask: %u.%u.%u.%u gw: %u.%u.%u.%u\r\n",
					ip4_addr1_16(&pNetif->ip_addr.addr), ip4_addr2_16(&pNetif->ip_addr.addr),
					ip4_addr3_16(&pNetif->ip_addr.addr), ip4_addr4_16(&pNetif->ip_addr.addr),
					ip4_addr1_16(&pNetif->netmask.addr), ip4_addr2_16(&pNetif->netmask.addr),
					ip4_addr3_16(&pNetif->netmask.addr), ip4_addr4_16(&pNetif->netmask.addr),
					ip4_addr1_16(&pNetif->gw.addr), ip4_addr2_16(&pNetif->gw.addr),
					ip4_addr3_16(&pNetif->gw.addr), ip4_addr4_16(&pNetif->gw.addr) );
		
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "Link status : %s\r\n", 
				((pGeif->linkStatus & IFM_ACTIVE)?"Up":"Down") );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "Link speed  : %u\r\n", pGeif->linkSpeed );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "Link changes: %u\r\n", atomic32Get( &pGeif->linkIntCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "RX interrupt: %u\r\n", atomic32Get( &pGeif->rxIntCount ) );
		len = strlen( (char *) pcWriteBuffer );

		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "TX interrupt: %u\r\n", atomic32Get( &pGeif->txIntCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "Interrupts  : %u\r\n", atomic32Get( &pGeif->intCount ) );
		len = strlen( (char *) pcWriteBuffer );
		
#ifdef INCLUDE_GEIF_DEBUG
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "Task signal : %u  errors: %u\r\n", 
					atomic32Get( &pGeif->taskSigCount ), atomic32Get( &pGeif->taskSigErrors ) );
		len = strlen( (char *) pcWriteBuffer );
		
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "RX signal   : %u\r\n", atomic32Get( &pGeif->rxSigCount ) );
		len = strlen( (char *) pcWriteBuffer );
#endif
		
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "RX packets  : %u  dropped: %u\r\n", 
					atomic32Get( &pGeif->rxPktCount ), atomic32Get( &pGeif->rxPktDropped ) );
		len = strlen( (char *) pcWriteBuffer );
		sprintf( (char *) &pcWriteBuffer[len], ( char * ) "TX packets  : %u  dropped: %u\r\n", 
					atomic32Get( &pGeif->txPktCount ), atomic32Get( &pGeif->txPktDropped ) );
		len = strlen( (char *) pcWriteBuffer );
		
		// GEIFDBG( GEIF_DBG_DIAG, ("%s: %s: string len: %u buffer size: %u\n", 
		//		pGeif->name, __FUNCTION__, len, xWriteBufferLen));
	}
	
	return len;
}


/*******************************************************************************
*
* geifLocateDevice
*
* Find the device instance, get the base addresses and initialize the 
* device type.
*
*
* RETURNS: ERR_OK or E__DEVICE_NOT_FOUND
* 
*******************************************************************************/
static int geifLocateDevice ( GEIF_NET *pGeif )
{
	UINT32 baseAddr;
	UINT32 flashAddr;
	UINT16 reg;
	int result;
	
	
	result = iPciFindDeviceById( pGeif->instance, pGeif->vid, pGeif->did, &pGeif->pfa );
	
	if( result == ERR_OK)
	{
		/* Enable memory access and bus master */
		reg = PCI_READ_WORD( pGeif->pfa, PCI_COMMAND );
		PCI_WRITE_WORD( pGeif->pfa, PCI_COMMAND, (reg | PCI_MEMEN | PCI_BMEN) & ~(1<<4));
		reg = PCI_READ_WORD( pGeif->pfa, PCI_COMMAND );
		GEIFDBG( GEIF_DBG_INIT, ("%s: PCI_COMMAND: 0x%04x\n", pGeif->name, reg));
	
		/* Read register base addres */
		baseAddr = PCI_READ_DWORD( pGeif->pfa, PCI_BAR0 );
		baseAddr &= PCI_MEM_BASE_MASK;
		
		/* Configure for specific device */
		if ( pGeif->did == INTEL_DEVICEID_82579LM2 )
		{
			pGeif->devType = GEIF_DEVTYPE_ICH;
			
			pGeif->baseAddr = sysMemGetPhysPtrLocked((UINT64) baseAddr, 0x20000 );
			
			flashAddr = PCI_READ_DWORD( pGeif->pfa, PCI_BAR1 );
			flashAddr &= PCI_MEM_BASE_MASK;
			
			pGeif->flashAddr = sysMemGetPhysPtrLocked((UINT64) flashAddr, 0x1000 );
			
			pGeif->phyAddr = 2;
			
			pGeif->linkType = GEIF_LINKTYPE_COPPER;
		}
		else
		{
			if ( (pGeif->did == INTEL_DEVICEID_82576EB)           ||
				 (pGeif->did == INTEL_DEVICEID_82580_QUAD_COPPER) ||
				 (pGeif->did == INTEL_DEVICEID_82580_DUAL_COPPER) ||
				 (pGeif->did == INTEL_DEVICEID_82580EB_SERDES)    ||
				 (pGeif->did == INTEL_DEVICEID_i350AMx_COPPER)    ||
				 (pGeif->did == INTEL_DEVICEID_i350AMx_SERDES)    ||
				 (pGeif->did == INTEL_DEVICEID_i210)              ||
				 (pGeif->did == INTEL_DEVICEID_i210_SERDES)
				)
			{
				pGeif->devType = GEIF_DEVTYPE_ADVANCED;
			}
			else
			{
				pGeif->devType = GEIF_DEVTYPE_PCI;
			}
			
			pGeif->baseAddr = sysMemGetPhysPtrLocked((UINT64) baseAddr, 0x80000 );
		
			pGeif->flashAddr = 0;
			pGeif->phyAddr = 1;
			
			if ((pGeif->did == INTEL_DEVICEID_82580EB_SERDES) ||
				(pGeif->did == INTEL_DEVICEID_i350AMx_SERDES) ||
				(pGeif->did == INTEL_DEVICEID_i210_SERDES))
			{
				pGeif->linkType = GEIF_LINKTYPE_SERDES;
			}
			else
			{
				pGeif->linkType = GEIF_LINKTYPE_COPPER;
			}
		}
	}
	
	return result;
}


/*******************************************************************************
*
* geifLowLevelInit
*
* Initialize interface and hardware.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void geifLowLevelInit( struct netif *pNetif )
{
	GEIF_NET *pGeif = pNetif->state;
	
#ifdef INCLUDE_DEBUG_VGA
	char achBuffer[80];
#endif
	
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s:\n", pGeif->name, __FUNCTION__));
	
	/* set MAC hardware address length */
	pNetif->hwaddr_len = ETHARP_HWADDR_LEN;
	
	/* Clear MAC hardware address */
	pNetif->hwaddr[0] = 0;
	pNetif->hwaddr[1] = 0;
	pNetif->hwaddr[2] = 0;
	pNetif->hwaddr[3] = 0;
	pNetif->hwaddr[4] = 0;
	pNetif->hwaddr[5] = 0;
	
	/* Set maximum transfer unit */
	pNetif->mtu = ETHERMTU;
	
	/* Find and initialize device. */
	
	if( geifLocateDevice( pGeif ) == ERR_OK)
	{
		GEIFDBG( GEIF_DBG_INIT,("%s: Type: %d base: 0x%08x irq: %u flash base: 0x%08x\n",
				pGeif->name, pGeif->devType, pGeif->baseAddr, pGeif->irqNo, pGeif->flashAddr));

		/* Set device to known state */ 
		geifDevReset( pGeif );
	
		/* Disable TX/RX */ 
		geifTxRxDisable( pGeif );
		
		/* Initialize local memory for descriptors and buffers */
		geifInitMem( pGeif );
			
		/* Get MAC address */
		geifEtherAdrGet( pNetif, pGeif );

		geifValidateNvmChecksum_82580(pNetif,pGeif->name);
		
		/* Initialize hardware addresses */
		geifHwAddrInit( pNetif, pGeif );
		
		/* Setup TX/RX */
		geifRxSetup( pGeif );
		geifTxSetup( pGeif );
	
		/* Configure interface */
		geifConfigure( pGeif, GEIF_FLAG_BROADCAST_MODE );
			
		/* Initialize link */
#ifdef INCLUDE_DEBUG_VGA
		if ( pGeif->num == VGA_GEIF_NUM )
		{
			sprintf( achBuffer, "%s: Device: %03u:%02u:%02u IRQ: %u", pGeif->name,
						PCI_PFA_BUS(pGeif->pfa), PCI_PFA_DEV(pGeif->pfa), 
						PCI_PFA_FUNC(pGeif->pfa), pGeif->irqNo );		
			vgaPutsXY( 0,23, achBuffer );
			
			sprintf( achBuffer, "MAC: %02X%02X%02X%02X%02X%02X",
						pNetif->hwaddr[0], pNetif->hwaddr[1], pNetif->hwaddr[2],
						pNetif->hwaddr[3], pNetif->hwaddr[4], pNetif->hwaddr[5]);
			vgaPutsXY( 31,23, achBuffer );
			
			sprintf( achBuffer, "IP: %u.%u.%u.%u",
						ip4_addr1_16(&pNetif->ip_addr.addr), ip4_addr2_16(&pNetif->ip_addr.addr),
						ip4_addr3_16(&pNetif->ip_addr.addr), ip4_addr4_16(&pNetif->ip_addr.addr));
			vgaPutsXY( 49,23, achBuffer );
			
			vgaPutsXY( 0, 24, "Waiting for link..." );
		}
#endif

#if LWIP_NETIF_STATUS_CALLBACK
  		netif_set_status_callback( pNetif, geifStatusCallback ); 
#endif /* LWIP_NETIF_STATUS_CALLBACK */

		/* Set device capabilities, */
		pNetif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

		/* Initailize the link */
		if ( pGeif->linkType == GEIF_LINKTYPE_SERDES )
		{
			geifSerdesLinkInit( pGeif );
			geifSerdesModeGet( pGeif, &pGeif->linkMode, &pGeif->linkStatus );
		}
		else
		{
			geifCheckPhy( pGeif );
			geifPhyLinkInit( pGeif );
		}
		
		geifLinkUpdate( pNetif );	
		
		/* Enable TX/RX */		
		geifTxRxEnable( pGeif );
	}
	else
	{
		GEIFERR(("%s: Error - Ethernet device not found\n", pGeif->name));
		
#ifdef INCLUDE_DEBUG_VGA
		if (pGeif->num == VGA_GEIF_NUM)
		{
			vgaPutsXY( 0,23, "Ethernet device not found" );
		}
#endif
	}  
}


/*******************************************************************************
*
* geifGetMemInfo
*
* This function gets the address and size of the memory used by the driver.
*
*
* RETURNS: None.
* 
*******************************************************************************/ 
void geifGetMemInfo( u32_t *pAddr, u32_t *pSize )
{
	if ((pAddr != NULL) && (pSize != NULL))
	{
		*pAddr = GEIF_MEM_ADDR;
		*pSize = GEIF_TOTAL_MEM_SIZE;
	}		
}


/*******************************************************************************
*
* geifInit
*
* This function should be passed as a parameter to netif_add() and is
* called at the system start up to setup the network interface. 
* It calls the low level init function to do the actual setup of the hardware.
*
*
* RETURNS: ERR_OK if successful or an error code.
* 
*******************************************************************************/ 
err_t geifInit( struct netif *pNetif )
{
	GEIF_NET *pGeif;
	ETH_DEV *pEthDev;
	xTaskHandle xHandle;
	char taskName[8];

#ifndef GEIF_SHARED_ISR		
	int vector;
#endif

	
	LWIP_ASSERT("pNetif != NULL", (pNetif != NULL));
	
	if (pNetif->num >= GEIF_MAX_NUM)
	{
		GEIFERR(("%s: Error - invalid interface number: %u\n", 
					__FUNCTION__, pNetif->num));
		return ERR_VAL;
	}
	
	pGeif = mem_malloc( sizeof(GEIF_NET) );
	
	if (pGeif == NULL) 
	{
		GEIFERR(("%s: Error - out of memory\n", __FUNCTION__));
		return ERR_MEM;
	}
	else
	{
		memset( pGeif, 0, sizeof(GEIF_NET) );
	}
	
	pEthDev = (ETH_DEV *) pNetif->state;
	
	if (pEthDev != NULL)
	{
		pGeif->num = pNetif->num;
		sprintf( pGeif->name, "%c%c%u", IFNAME0, IFNAME1, pGeif->num );
	
		pGeif->vid =  pEthDev->vid;
		pGeif->did =  pEthDev->did;
		pGeif->instance = pEthDev->instance;
		pGeif->irqNo = pEthDev->irqNo;
	}
	else
	{
		GEIFERR(("%s: Error - no device given\n", __FUNCTION__));
		mem_free( pGeif );
		return ERR_VAL;
	}
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: pGeif: 0x%x vid: 0x%04x did: 0x%04x instance: %u IRQ: %u\n", 
				pGeif->name, __FUNCTION__, (UINT32) pGeif, pGeif->vid, pGeif->did, 
				pGeif->instance, pGeif->irqNo));

#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	pNetif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */
	
	/*
	* Initialize the snmp variables and counters inside the struct netif.
	* The last argument should be replaced with your link speed, in units
	* of bits per second.
	*/
	NETIF_INIT_SNMP(pNetif, snmp_ifType_ethernet_csmacd, 1000000);
	
	pNetif->state = pGeif;
	pNetif->name[0] = IFNAME0;
	pNetif->name[1] = IFNAME1;
	
	if( sys_mutex_new( &pGeif->tx_mutex ) != ERR_OK )
    {
    	GEIFERR(("%s: %s: Error - failed to create TX mutex\n", 
				pGeif->name, __FUNCTION__));
		
		mem_free( pGeif );
		
		return ERR_VAL;
    }
    
    if( sys_mutex_new( &pGeif->op_mutex ) != ERR_OK )
    {
    	GEIFERR(("%s: %s: Error - failed to OP mutex\n", 
				pGeif->name, __FUNCTION__));
		
		sys_mutex_free( &pGeif->tx_mutex );
		mem_free( pGeif );
		
		return ERR_VAL;
    }
    
    pGeif->rx_sem = xSemaphoreCreateCounting( 100, 0 );
    if (pGeif->rx_sem == NULL)
    {
    	GEIFERR(("%s: %s: Error - failed to create RX semaphore\n", 
				pGeif->name, __FUNCTION__));
        
        sys_mutex_free( &pGeif->op_mutex );
        sys_mutex_free( &pGeif->tx_mutex );
		mem_free( pGeif );
		
		return ERR_VAL;
    }
	else
	{
		/* Set output functions */
		pNetif->output = geifOutput;
		pNetif->linkoutput = geifLowLevelOutput;
	
		/* Initialize the hardware */
		geifLowLevelInit( pNetif );

#ifndef GEIF_POLL_MODE
		
#ifdef GEIF_SHARED_ISR		
		/* Install interrupt handler */
		if ( pciIntConnect( pGeif->irqNo, (VOIDFUNCPTR) geifIsr, (UINT32) pGeif ) )
		{
			/* Enable device interrupts */
			GEIF_WRITE_REG( GEIF_IMS, GEIF_INTRS );
			
			// pciIntListShow( pGeif->irqNo );
		}
		else
		{
			GEIFERR(("%s: %s: Error - failed to install ISR\n", 
					pGeif->name, __FUNCTION__));
			
			sys_mutex_free( &pGeif->tx_mutex );
			sys_sem_free(&pGeif->rx_sem);
			mem_free( pGeif );
		
			return ERR_VAL;
		}
#else
		pGlobalGei = pGeif;
		
		/* Install interrupt handler */
		vector = sysPinToVector( pGeif->irqNo, SYS_IOAPIC0 );
		
		if (vector < 0)
		{
			GEIFERR(("%s: %s: Error - invalid vector (%d)\n", 
					pGeif->name, __FUNCTION__, vector));
		}
		else
		{
			if (sysInstallUserHandler( vector, geifIsr ) == 0)
			{
				if (sysUnmaskPin( pGeif->irqNo, SYS_IOAPIC0 ) == 0)
				{
					/* Enable interrupts */
					GEIF_WRITE_REG( GEIF_IMS, GEIF_INTRS );
					
					GEIFDBG( GEIF_DBG_INIT, ("%s: %s: IRQ%u enabled, using Vector: 0x%x\n", 
								pGeif->name, __FUNCTION__, pGeif->irqNo, vector));
				}
				else
				{
					GEIFERR(("%s: %s: Error - failed to enable IRQ%u\n", 
								pGeif->name, __FUNCTION__, pGeif->irqNo));
					
					sysInstallUserHandler( vector, 0 );
				}
			}
			else
			{
				GEIFERR(("%s: %s: Error - failed to install ISR\n", 
					pGeif->name, __FUNCTION__));
			}
		}
#endif

#endif

		sprintf( taskName, "ge%dInt", pGeif->num );
		pGeif->cpuNo = sPortGetCurrentCPU();
		
		xTaskCreate( pGeif->cpuNo, geifIntTask, taskName, configMINIMAL_STACK_SIZE, 
						pNetif, (configMAX_PRIORITIES - 2), &xHandle );
	}
	
	GEIFDBG( GEIF_DBG_INIT, ("%s: %s: completed\n", pGeif->name, __FUNCTION__));
	
	return ERR_OK;
}

