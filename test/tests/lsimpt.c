/*++
 *
 *    Copyright (C) Concurrent Technologies plc., Colchester, Essex, UK., 1999
 *
 *    The software below is supplied on the understanding that no responsibility
 *    is assumed by Concurrent Technologies plc. for any errors contained within
 *    the software. Furthermore, the company makes no commitment to update and /
 *    or maintain the currency of the software and also reserves the right to
 *    change its specification at any time.
 *
 *    Concurrent Technologies plc, assumes no responsibility for either the use
 *    of this software or any infringements of patents and / or other rights of
 *    third parties which may result from its use.
 *
 *--
 */

/*
 *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Date         Author  Comments
 * -----------  ------  -------------------------------------------------------
 * 25-Mar-2014  CGD     Copied to CUTE2 
 *-----------------------------------------------------------------------------
 */

/* include files */

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
#include <bit/board_service.h>
#include <alloc.h>

#include <devices/mpi_type.h>
#include <devices/mpi.h>
#include <devices/mpi_ioc.h>
#include <devices/mpi_init.h>	/* Initiator Mode messages */


/* #define DEBUG */

#define vendor_LSI			0x1000

#define id_LSI53C1020		0x0020
#define id_LSI53C1021		0x0021
#define id_LSI53C1030		0x0030
#define id_LSI53C1064		0x0050
#define id_LSI53C1068E		0x0058

#define PCI_IO_BAR          0x10

#define NOT_CONFIGURED		0x99

#define TIME_LIMIT          8000

/* 16 bit errors */
#define E_OK               0
#define E_TEST_PASS        0
#define E_TEST_FAIL        1
#define	E_NO_DEVICE        2
#define E_CONTROLLER_FAIL  3
#define E_1s_ERROR         4
#define E_0s_ERROR         5
#define E_NO_EXTINT        5
#define E_TEST_NOT_FOUND   7

#define FAIL    0
#define PASS    1

#define MAX_TEST_NUMBER 2

/* Host Page Buffer is area of host memeory allocated to the IOC for its own use */
/* This is NOT used for message transactions between the host and IOC. */
#define HOST_PAGE_BUFFER        0x1000000L   /* 16MB */
#define REQUEST_FRAME_BUFFER    0x1100000L   /* 17MB - 2KBytes size */
#define REPLY_FRAME_BUFFER      0x1100800L   /* 17MB */
#define SGL_BUFFER              0x1101000L   /* 17MB + 4KB */
#define SENSE_BUFFER            0x1101800L   /* 17MB + 6KB */
#define SGL_BUFFER_SIZE         1024
#define SGL_FLAGS_LENGTH_READ   0xD1000400L  /* Simple Element, Only & last Element, Read, 1KB Size */
#define SGL_FLAGS_LENGTH_WRITE  0xD5000400L  /* Simple Element, Only & last Element, Write, 1KB Size */
#define SENSE_BUFFER_SIZE       18
#define MAX_REQUEST_FRAMES      16
#define MAX_REPLY_FRAMES        16
#define MAX_FRAME_SIZE          128         /* Bytes */
#define REQUEST_FRAME_BUFFER_SIZE MAX_REQUEST_FRAMES * MAX_FRAME_SIZE
#define REPLY_FRAME_BUFFER_SIZE MAX_REPLY_FRAMES * MAX_FRAME_SIZE
#define INVALID_ADDRESS         0xFFFFFFFFL
#define MPI_REPLY_FIFO_EMPTY    0xFFFFFFFFL
#define NO_FRAME                0xFF

#define NVSRAM_START_ADDRESS    0x22000000
#define NVSRAM_END_ADDRESS      0x22000800

/* IRQ status & Reply Timeout */
#define IRQ_NOT_PENDING         0x00
#define IRQ_PENDING             0x01
#define IRQ_INVALID             0x02
#define IRQ_TIMEOUT_TENTHS      300

#define SCSI_TEST_UNIT_READY    0x00
#define SCSI_INQUIRY            0x12
#define SCSI_READ               0x08
#define SCSI_READ10             0x28
#define SCSI_READ_CAPACITY      0x25
#define SCSI_START_STOP_UNIT    0x1B

/* Local Message Passing Interface Error codes */
/* These are add-ons to the IOC_STATUS codes   */
#define MPI_STATUS_NO_REQUEST_FRAME_OR_CREDIT   0xFF00
#define MPI_STATUS_SEND_REQUEST_FAILED          0xFF01
#define MPI_STATUS_NO_REPLY_INTERRUPT           0xFF02
#define MPI_STATUS_QUEUE_EMPTY                  0xFF03
#define MPI_STATUS_INVALID_REPLY_POINTER        0xFF04
#define MPI_STATUS_NO_CONTEXT_MATCH             0xFF05
#define MPI_STATUS_MBR_FAILED                   0xFF06
#define MPI_TEST_SEQUENCE_FAILED                0xFF07

enum FRAME_STATE { AVAILABLE, PENDING, UNAVAILABLE};

typedef struct 
{
	UINT16	wDid;
	char	*pDescription;
} DEVINFO;

typedef union 
{
    UINT8 bByte[4];
    UINT16 wWord[2];
    UINT32 dLong;
} CONVERT;

typedef struct {
    UINT32  dSystemDoorbell;
    UINT32  dWriteSequence;
    UINT32  dHostDiagnostic;
    UINT32  dTestBaseAddress;
    UINT32  dReserved1[8];
    UINT32  dHostInterruptStatus;
    UINT32  dHostInterruptMask;
    UINT32  dReserved2[2];
    UINT32  dRequestQueue;
    UINT32  dReplyQueue;
    UINT32  dHiPriRequestQueue;
} LSI_MPT_REGS;


typedef struct {
	volatile LSI_MPT_REGS *psDevRegs;
	PCI_PFA Pfa;
	UINT16 wIoBaseAddress;
	char achErrorMsg [80];
} DEVICE;

    
typedef struct
{
    UINT16  wReserved1;
    UINT8   bChainOffset;
    UINT8   bFunction;
    UINT8   bReserved2[3];
    UINT8   bMsgFlags;
    UINT32  dMsgContext;
} MPI_REQUEST_HEADER;

typedef struct {
    UINT16  wReserved1;
    UINT8   bMsgLength;
    UINT8   bFunction;
    UINT8   bReserved2[3];
    UINT8   bMsgFlags;
    UINT32  dMsgContext;
    UINT8   bReserved3[2];
    UINT16  wIOCStatus;
    UINT32  dIOCLogInfo;
    } MPI_REPLY_HEADER;

typedef struct inquiry_structure 
{
    UINT8 dev_type;
    UINT8 dev_type_mod;
    UINT8 version;
    UINT8 format;
    UINT8 add_length;
    UINT8 _reserved[2];
    UINT8 flags;
    char vendor_id[8];
    char product_id[16];
    char product_rev[4];
} MPI_INQUIRY_RESPONSE_STRUCT;

typedef struct
{
    UINT32 dLogicalBlocks;
    UINT32 dBlockLength;
} MPI_READ_CAPACITY_RESPONSE_STRUCT;
	
	

typedef struct {
    UINT16  wCreditRemaining;
    enum FRAME_STATE aeFrameState[MAX_REQUEST_FRAMES];      /* Available / Pending Reply / Unavailable */
    MPI_REQUEST_HEADER  *apsFramePointer[MAX_REQUEST_FRAMES];
    UINT32 adFramePhysicalAddress[MAX_REQUEST_FRAMES];
               } REQUEST_FRAME_STATUS;
               
typedef struct {
    enum FRAME_STATE aeFrameState[MAX_REPLY_FRAMES];      /* Available / Pending Reply / Unavailable */
    MPI_REPLY_HEADER  *apsFramePointer[MAX_REPLY_FRAMES];
    UINT32 adFramePhysicalAddress[MAX_REQUEST_FRAMES];
               } REPLY_FRAME_STATUS;          
               
/* static functions' declarations */
static UINT16 wReplyHandler(volatile LSI_MPT_REGS *psDevice);

/* external functions */

/* local variables */
static PTR48	p1, p2, p3, p4, p5;
static UINT32	dHandle1, dHandle2, dHandle3, dHandle4, dHandle5;
static MSG_IOC_FACTS_REPLY  sIocFactsReply;
static REQUEST_FRAME_STATUS sRequestFrameStatus;
static REPLY_FRAME_STATUS sReplyFrameStatus;
volatile UINT8 *pbSGLBuffer;
volatile UINT8 *pbSenseBuffer;
static UINT16   wContext; /* Context for matching Requests to Replies */
static UINT8    bIRQNumber;
static int		iVector= -1;
static UINT8    bIRQState;
static UINT8    bINTsEnabled;
static DEVICE  *psDUT;		/* device pointer used by interrupt handler */

//#pragma interrupt (vMPTIntPassHandler)
static void	vMPTIntPassHandler (void);
//#pragma interrupt (vMPTIntFailHandler)
//static void vMPTIntFailHandler (void);

/* PBSRAM Testing Function prototypes */

#ifdef INCLUDE_LSI_MPT_PBSRAM_TEST
static UINT16 wPCIMPT_Diag_CheckMode_EnabledIo (UINT16 wHostRegIoBAR);
static void vPCIMPT_Diag_Write_To_Hardware (UINT16 wAddress, UINT32 dValue);
static UINT32 dPCIMPT_Diag_Read_From_Hardware (UINT16 wAddress);
static UINT32 dPCIMPT_Diag_IoReadInternalRegister32 (UINT16 wHostRegIoBAR, UINT32 dAddress);
static void vPCIMPT_Diag_IoWriteInternalRegister32 (UINT16 wHostRegIoBAR, UINT32 dAddress, UINT32 dValue);
static void vPCIMPT_Diag_ReadDword (DEVICE *psPciDevice, UINT32 dAddress, UINT32 *pdValue);
static void vPCIMPT_Diag_WriteDword (DEVICE *psPciDevice, UINT32 dInternalAddress, UINT32 dValue);
static UINT16 wPCIMPT_PBSRAM_FixedPattern_Test (DEVICE *psPciDevice);
static UINT16 wPCIMPT_PBSRAM_WalkingZero_Test (DEVICE *psPciDevice);
static UINT16 wPCIMPT_PBSRAM_WalkingOne_Test (DEVICE *psPciDevice);
static UINT16 wPciMPT_EnableDiagnostic (UINT16 wIoBaseAddress);
static UINT16 wPciMPT_GetIoBaseAddress (const PCI_PFA Pfa);
#endif


/******************************************************************************
 *
 * vInitMemHandles: Zero all global memory handles
 *
 * RETURNS: none
 */

void vInitMemHandles (void)
{
	dHandle1 = dHandle2 = dHandle3 = dHandle4 = dHandle5 = 0;
}

/******************************************************************************
 *
 * vFreeMemHandles: Free used memory resources
 *
 * RETURNS: none
 */

void vFreeMemHandles (void)
{
	if (dHandle1)
		vFreePtr(dHandle1);

	if (dHandle2)
		vFreePtr(dHandle2);

	if (dHandle3)
		vFreePtr(dHandle3);

	if (dHandle4)
		vFreePtr(dHandle4);

	if (dHandle5)
		vFreePtr(dHandle5);
}

/******************************************************************************
 *
 * dEndianSwap: Performs Endian Swap on 32bit word
 *
 * RETURNS: none (CPU state preserved)
 */

UINT32 dEndianSwap (UINT32 dWord)
{
	union{
		UINT32 dWord;
		UINT8  bByte[4];
		} uInput, uOutput;
	
	
	     
	uInput.dWord = dWord;
	
	uOutput.bByte[0] = uInput.bByte[3];
	uOutput.bByte[1] = uInput.bByte[2];
	uOutput.bByte[2] = uInput.bByte[1];
	uOutput.bByte[3] = uInput.bByte[0];
	
	return(uOutput.dWord);
}

/******************************************************************************
 *
 * vMPTIntPassHandler: special case interrupt handler for MPT Interrupt
 *
 * RETURNS: none (CPU state preserved)
 */

static void vMPTIntPassHandler (void)
{
	if (psDUT != NULL) {
		bIRQState = IRQ_PENDING;
		psDUT->psDevRegs->dHostInterruptStatus = 0x00;
	}
}

/******************************************************************************
 *
 * vMPTIntFailHandler: special case interrupt fail handler for MPT interrupt
 *
 * RETURNS: none (CPU state preserved)
 */

//static void vMPTIntFailHandler (void)
//{
//	bIRQState = IRQ_INVALID;
//}


/******************************************************************************
 *
 * vWaitForInt: Waits for Interrupt response or timeout
 *
 * RETURNS: none
 */

static void vWaitForInt (void)
{
	UINT16 wTenths;
	for (wTenths = 0; (wTenths < IRQ_TIMEOUT_TENTHS)  && (bIRQState == IRQ_NOT_PENDING); wTenths++)
	{
		vDelay (100); /* Tenth of a second delay */
	}
}


/******************************************************************************
 *
 * vMPTIntHandler: 
 *
 * RETURNS: status (CPU state preserved)
 */

UINT16 wMPTIntHandler (DEVICE *psDevice)
{
	UINT16 wStatus;
	
	/* Wait for interrupt / timeout */
	vWaitForInt();

	/* Check the interrupt status */
	if(bIRQState == IRQ_PENDING)
	{
		vConsoleWrite("IRQ Pending\n");
		wStatus = wReplyHandler(psDevice->psDevRegs);
	}
	else
	{
		vConsoleWrite("No IRQ Asserted\n");
		wStatus = MPI_STATUS_NO_REPLY_INTERRUPT;
	}

	/* clear interrupt status */
	psDevice->psDevRegs->dHostInterruptStatus = 0x00;

	/* remask the reply message interrupt - clears INTA assertion
	   - otherwise remains asserted until all messages have been read */
	psDevice->psDevRegs->dHostInterruptMask |= MPI_HIS_REPLY_MESSAGE_INTERRUPT;

	/* Send End Of Interrupt to PIC so it can be asserted next time */
//	CFS_send_EOI(bIRQVector);

	return(wStatus);
}


/******************************************************************************
 *
 * wFindMPTController - Find the nth occurance of a fusion MPT scsi chip 
 *
 *
 * RETURNS: as returned from wfPciFindVendor ()
 */

UINT16 wFindMPTController
(
    PCI_PFA *pPfa,
	const char **pDescription,
    UINT8 bOccurrence
)
{
    const UINT16 awDeviceIds [] = {
		id_LSI53C1020,
		id_LSI53C1030,
		id_LSI53C1064,
		id_LSI53C1068E};

	const char *pDescr [] = {
		"53C1020 SCSI",
		"53C1030 SCSI",
		"53C1064 SAR",
		"53C1068E SAS",
		"Invalid"};

	int i;
	UINT8 bBus, bDev, bFunc, bMultiFunc, bHostBridges, bScanToBus;
	
	i = bBus = bDev = bFunc = bHostBridges = bScanToBus = 0;

	do {
		PCI_PFA pfa;
		UINT16 wVid, wDid;
		UINT8 bTemp;
		int n;
		
		pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
		
		wVid = PCI_READ_WORD(pfa, PCI_VENDOR_ID);
		if (wVid == vendor_LSI) {
			wDid = PCI_READ_WORD(pfa, PCI_DEVICE_ID);
			
			// check list of DID's
			for (n = 0; n < LENGTH (awDeviceIds); n++) {
				if (awDeviceIds[n] == wDid) {
					i++;
					*pDescription = pDescr [n];
					break;
				}
			}
		}
		
		if (i == bOccurrence) {
			*pPfa = pfa;
			return E_OK;
		}

		if (bFunc == 0)	/* check for multifunction devices */
		{
			if (wVid == 0xFFFF)	/* no device on the bus at this address */
			{
				bMultiFunc = 0;
			}
			else
			{
				UINT8 bTemp;
				pfa = PCI_MAKE_PFA (bBus, bDev, 0);
				bTemp = PCI_READ_BYTE(pfa, PCI_HEADER_TYPE);

				bMultiFunc = (bTemp & 0x80) || 					/* found a multifunction device */
							 ((bBus == 0) && (bDev == 0));		/* always set pfa 0:0:0 to multifunction device */
			}
		}

		if (wVid != 0xFFFF)	/* found something */
		{
			pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
			bTemp = PCI_READ_BYTE(pfa, PCI_BASE_CLASS);

			if (bTemp == 0x06)	/* PCI<->PCI bridge class */
			{
				pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
				bTemp = PCI_READ_BYTE(pfa, PCI_SUB_CLASS);

				if (bTemp == 0x00)
				{
					if (bHostBridges > 0)
						bScanToBus++;

					bHostBridges++;
				}
				else if (bTemp == 0x04)		/* PCI-PCI bridge*/
				{
					pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);
					bTemp = PCI_READ_BYTE(pfa, PCI_SUB_BUS);

					if (bTemp > bScanToBus)
						bScanToBus = bTemp;
				}
			}
		}

		/* Increment device/bus numbers */

		if (bMultiFunc == 0)
			(bDev)++;
		else
		{
			bFunc++;
			if (bFunc == 8)
			{
				(bDev)++;
				bFunc = 0;
				bMultiFunc = 0;
			}
		}

		if (bDev == 32)
		{
			bDev = 0;
			(bBus)++;
		}
		
	} while (bBus <= bScanToBus);


	return (E_NO_DEVICE);
}


/******************************************************************************
 *
 * vPrintDeviceDetails - Prints the PCI address
 *
 * This function simply displays the address components of the specified PCI
 * address.
 *
 * RETURNS: N/A
 */

void vPrintDeviceDetails
(
	const PCI_PFA Pfa,
	const char *pDescr
)
{
	char	achBuffer [80];

	vConsoleWrite ("\n");
	sprintf (achBuffer, "Found %s (PCI Bus: %02X, Device: %02X, Function: %02X)\n",
						 pDescr,
	                     PCI_PFA_BUS(Pfa), PCI_PFA_DEV(Pfa),PCI_PFA_FUNC(Pfa));
	vConsoleWrite (achBuffer);
}


/******************************************************************************
 *
 * psGetDevicePtr - returns a pointer to the device's operating registers
 *
 * This function reads the 32-bit memory base address from PCI, gets a
 * selector, and returns a pointer to that space. The pointer can then be
 * used for accessing the device's operating registers.
 *
 * RETURNS: LSI_MPT_REGS*
 */

volatile LSI_MPT_REGS* psGetDevicePtr (DEVICE *psDevice)
{
	PCI_PFA Pfa = psDevice->Pfa;
//	volatile LSI_MPT_REGS* pdev;

//	selector sel;
	UINT32   adev;

	adev  = PCI_READ_DWORD(Pfa, PCI_BAR1);
	adev &= PCI_MEM_BASE_MASK;
	dHandle1 = dGetPhysPtr (adev, sizeof(LSI_MPT_REGS), &p1, (void*)&psDevice->psDevRegs);
//	sel   = set_scratch_1 (adev, sizeof(LSI_MPT_REGS));
//	pdev  = (volatile LSI_MPT_REGS*) buildptr (sel, (near *) NULL);

	return psDevice->psDevRegs;
}


/******************************************************************************
 *
 * wMPT_DoorbellSendMsg -
 *
 * RETURNS: 
 */
UINT16 wMPT_DoorbellSendMsg(volatile LSI_MPT_REGS *psDevice,
		UINT8 *pRequestMsg, UINT8 bRequestSizeBytes, UINT8 *pReplyMsg)
{
	UINT8 i, bRequestSizeDwords, bReplySizeWords;
	volatile CONVERT uMsgData;
	UINT16 *pwReply;
	    
	/* Convert bytes to send into longwords to send */
	bRequestSizeDwords = bRequestSizeBytes >> 2;
    
    
	/* Clear pending doorbell interrupts from previous IOC state
	 * changes, should they exist 
	 */
	if(psDevice->dHostInterruptStatus & MPI_HIS_DOORBELL_INTERRUPT)
	{
		psDevice->dHostInterruptStatus = 0x00;
	}


	/* Issue Handshake Request to IOC */
	psDevice->dSystemDoorbell = ((MPI_FUNCTION_HANDSHAKE << MPI_DOORBELL_FUNCTION_SHIFT) | 
			((bRequestSizeDwords) << MPI_DOORBELL_ADD_DWORDS_SHIFT) );

	/* Doorbell Used bit will become set because a doorbell function has 
	 * started, Wait for the interrupt then turn it off 
	 */
	while(!(psDevice->dHostInterruptStatus & MPI_HIS_DOORBELL_INTERRUPT));

	psDevice->dHostInterruptStatus = 0x00;

	/* Wait for the IOP to finish processing - it'll clear IOP Doorbell 
	 * Status bit 
	 */
	while((psDevice->dHostInterruptStatus & MPI_HIS_IOP_DOORBELL_STATUS));
    
	for(i = 0; i < bRequestSizeDwords; i++)
    	{

        	/* Request may not be on longword boundary,
        	 * so read bytes and convert to longwords,
        	 * for writing to doorbell register. 
        	 */
        
        	/* CHECK THIS IS RIGHT WAY ROUND !!!! */
        	uMsgData.bByte[0] = *pRequestMsg++;
        	uMsgData.bByte[1] = *pRequestMsg++;
        	uMsgData.bByte[2] = *pRequestMsg++;
        	uMsgData.bByte[3] = *pRequestMsg++;
                
        	psDevice->dSystemDoorbell = uMsgData.dLong;
                
        	/* Wait for the IOP to finish processing - it'll clear IOP 
        	 * Doorbell Status bit 
        	 */
        	while((psDevice->dHostInterruptStatus & MPI_HIS_IOP_DOORBELL_STATUS));
                
	}
    
	/*
	 * Wait for Reply and read first 32bits into the reply message structure.
	 * Reads from doorbell are 16bit, so we need two, the second contains 
	 * the reply length.
	 */    

	pwReply = (UINT16 *)pReplyMsg;
    
	while(!(psDevice->dHostInterruptStatus & MPI_HIS_DOORBELL_INTERRUPT));
	*pwReply = (psDevice->dSystemDoorbell & MPI_DOORBELL_DATA_MASK);

	psDevice->dHostInterruptStatus = 0x00;
	pwReply++;
    
	while(!(psDevice->dHostInterruptStatus & MPI_HIS_DOORBELL_INTERRUPT));    
	*pwReply = (psDevice->dSystemDoorbell & MPI_DOORBELL_DATA_MASK);
    
	psDevice->dHostInterruptStatus = 0x00;
    
	/* Reply length is in the lower byte of this second word.
	 * It is expressed as the number of 32bit words.
	 * Double this for no. of 16bit reads and subtract 2 for the
	 * ones we already did.
	 */
	bReplySizeWords = (*pwReply << 1) - 2;
	pwReply++;

	for(i = 0; i < bReplySizeWords; i++)
	{
		while(!(psDevice->dHostInterruptStatus & MPI_HIS_DOORBELL_INTERRUPT));    
		*pwReply = (psDevice->dSystemDoorbell & MPI_DOORBELL_DATA_MASK);

		psDevice->dHostInterruptStatus = 0x00;
		pwReply++;
        
	}

	/* Wait for IOP to reset Doorbell Used bit, and clear interrupt */
	while(!(psDevice->dHostInterruptStatus & MPI_HIS_DOORBELL_INTERRUPT));    
	psDevice->dHostInterruptStatus = 0x00;
    
	/* If we got here the Send / Receive must have worked */
	return E_TEST_PASS;
    
}




/******************************************************************************
 *
 * GetReplyPointer - Returns the Pointer corresponding to the Physical Address
 *                   of the reply.
 *
 * RETURNS: 
 */
MPI_REPLY_HEADER *psGetReplyPointer(UINT32 dPhysicalAddress)
{
	UINT8 bFrameNo;
    
	for(bFrameNo = 0; bFrameNo < MAX_REPLY_FRAMES; bFrameNo++)
	{
		if(sReplyFrameStatus.adFramePhysicalAddress[bFrameNo] == dPhysicalAddress)
		{
			return(sReplyFrameStatus.apsFramePointer[bFrameNo]);
		}

	}
    
	/* If we got here, error occured */
	return (MPI_REPLY_HEADER *)INVALID_ADDRESS;

}

/******************************************************************************
 *
 * vReleaseMatchingRequestFrame - Releases the request frame whose message
 *	context number matches that passed to this function. The request frame
 *	is made available for re-use.
 *
 * RETURNS: Nothing
 */
void vReleaseMatchingRequestFrame(UINT32  dReplyMsgContext)
{
	UINT8 bFrameNo;
	MPI_REQUEST_HEADER *psRequestHeader;
//	char    achBuffer [80];
    
	for(bFrameNo = 0; bFrameNo < MAX_REPLY_FRAMES; bFrameNo++)
	{
		psRequestHeader = sRequestFrameStatus.apsFramePointer[bFrameNo];
        
		if(psRequestHeader->dMsgContext == dReplyMsgContext)
		{
			/* Reply frame corresponds to this request frame 
			 * Make the request frame available for re-use.
			 */
			sRequestFrameStatus.aeFrameState[bFrameNo] = AVAILABLE;
			/*vConsoleWrite("Matching Request Frame Released");*/
			return;
		}

	}
    
	/* No matching request found */
	/*vConsoleWrite("Failed to find matching request");*/
}



/******************************************************************************
 *
 * vDisplayScsiIoErrorResponse - Displays Port Facts Response
 *
 * RETURNS: 
 */

void vDisplayScsiIoErrorResponse(MSG_SCSI_IO_REPLY *psReply)
{
	char    achBuffer [80];

	vConsoleWrite("Scsi IO Error Response:");

	sprintf (achBuffer, "Target = %02X , Bus = %02X\n",
		psReply->TargetID, psReply->Bus);
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "CDB Length = %02X , Sense Buffer Length = %02X\n",
		psReply->CDBLength, psReply->SenseBufferLength);
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "SCSI Status = %02X , SCSI State = %02X\n",
		psReply->SCSIStatus, psReply->SCSIState);
	vConsoleWrite (achBuffer);
	
	sprintf (achBuffer, "Transfer Count = %08X , Sense Count = %08X\n",
		psReply->TransferCount, psReply->SenseCount);
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "Response Info = %08X , Task Tag = %04X\n",
		psReply->TransferCount, psReply->SenseCount);
	vConsoleWrite (achBuffer);
}



/******************************************************************************
 *
 * vDisplayScsiTaskMgmtResponse - Displays Scsi Task Management Response
 *
 * RETURNS: 
 */

void vDisplayScsiTaskMgmtResponse(MSG_SCSI_TASK_MGMT_REPLY *psReply)
{
	char    achBuffer [80];

	vConsoleWrite("Scsi Task Mgmt Response:");

	sprintf (achBuffer, "Target = %02X , Bus = %02X\n",
		psReply->TargetID, psReply->Bus);
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "Response Code = %02X , Task type = %02X\n",
		psReply->ResponseCode, psReply->TaskType);
	vConsoleWrite (achBuffer);

	sprintf (achBuffer, "Message Flags = %02X , Termination Count = %02X\n",
		psReply->MsgFlags, psReply->TerminationCount);
	vConsoleWrite (achBuffer);
	
}

/******************************************************************************
 *
 * vDisplayScsiIOInquiryResponse
 *
 * RETURNS: 
 */

void vDisplayScsiIOInquiryResponse(MSG_SCSI_IO_REQUEST *psScsiIoRequest)
{
	const char line_a [] = "Bus %02d  Target %03d : %s\n";
	const char line_b [] = "Bus %02d  Target %03d : %s, %s\n";
	const char line_c [] = "Bus %02d  Target %03d : %s, %s, %s\n";

	char    achBuffer [80];
	char	achVendor   [9];
	char	achProduct [17];
	char	achRevision [5];
	const char *line;
		

	MPI_INQUIRY_RESPONSE_STRUCT *psInquiryResponse;
	
	/* Overlay Inquiry Response Structure on Scatter Gather List Response Buffer */
	psInquiryResponse = (MPI_INQUIRY_RESPONSE_STRUCT *)pbSGLBuffer;
	
	if (psInquiryResponse->add_length >= 12)
	{
		strncpy (achVendor, psInquiryResponse->vendor_id, 8);
		achVendor [8] = 0;
		line = line_a;

		if (psInquiryResponse->add_length >= 28)
		{
			strncpy (achProduct, psInquiryResponse->product_id, 16);
			achProduct [16] = 0;
			line = line_b;
		}

		if (psInquiryResponse->add_length >= 32)
		{
			strncpy (achRevision, psInquiryResponse->product_rev, 4);
			achRevision [4] = 0;
			line = line_c;
		}

		sprintf (achBuffer, line, psScsiIoRequest->Bus, psScsiIoRequest->TargetID , achVendor, achProduct, achRevision);
		vConsoleWrite (achBuffer);
	}

}


/******************************************************************************
 *
 * vDisplayScsiIOReadCapacityResponse - Reads the structure returned in the 
 *		Read Capacity CDB response and displays the drive capacity
 *		in MegaBytes or GigaBytes.
 *
 * NOTE:	The LBAs and Block Length fields are returned in Big Endian
 *		format, and thus have to be converted to little endian for
 *		calculation.
 *
 * RETURNS: 
 */

void vDisplayScsiIOReadCapacityResponse(void)
{
	UINT32	dCapacity;
	char    achBuffer [80];
	
	MPI_READ_CAPACITY_RESPONSE_STRUCT *psReadCapacityResponse;

	/* Overlay Read Capacity Response Structure on Scatter Gather List Response Buffer */
	psReadCapacityResponse = (MPI_READ_CAPACITY_RESPONSE_STRUCT *)pbSGLBuffer;

	/* Get no of Megablocks */
	dCapacity = dEndianSwap(psReadCapacityResponse->dLogicalBlocks) >> 20;

	/* Multiply by Block size to get Megabytes */
	dCapacity = dCapacity * dEndianSwap(psReadCapacityResponse->dBlockLength);

	
	/* If Capacity < 1000 express in MBytes, otherwise Gigabytes */
	if (dCapacity < 1000)
	{
		sprintf (achBuffer, "        Capacity %d MegaBytes\n", dCapacity);
		vConsoleWrite (achBuffer);
	}
	else
	{
		dCapacity = dCapacity / 1000;
		sprintf (achBuffer, "        Capacity %d GigaBytes\n", dCapacity);
		vConsoleWrite (achBuffer);
	}
	
}


/******************************************************************************
 *
 * wCheckScsiIOReadResponseForMBR - Checks the Read response for the 0x55AA found
 *				at the end of the Master Boot Record.
 *
 * NOTE:	Makes the assumption that it was the Master Boot Record that
 *		has been read. Might be worth checking the original request to
 *		confirm this.....
 *
 * RETURNS: 
 */
UINT16 wCheckScsiIOReadResponseForMBR(void)
{
	/* Test Byte 0x01FE */
	if( (pbSGLBuffer[510] == 0x55) && (pbSGLBuffer[511] == 0xAA) )
	{
		vConsoleWrite("        Master Boot Record - PASS\n");
		return(MPI_IOCSTATUS_SUCCESS);
	}
	else
	{
		vConsoleWrite("        Master Boot Record - FAIL\n");
		return(MPI_STATUS_MBR_FAILED);
	}
	
}



/******************************************************************************
 *
 * ProcessReply -  Dummy Function which could be used to process standard address
 *		replies by farming them out to the appropriate handlers. Was used
 *		during debug.
 *
 * RETURNS: 
 */
UINT16 wProcessReply(MPI_REPLY_HEADER *psReplyHeader)
{
#if 0	
	char    achBuffer [80];

	 /*Use this for debugging future devices .....*/
	switch(psReplyHeader->bFunction)
	{
		case MPI_FUNCTION_IOC_FACTS:
			vDisplayIOCFactsResponse((MSG_IOC_FACTS_REPLY *)psReplyHeader);
			break;

		case MPI_FUNCTION_PORT_FACTS:
			vDisplayPortFactsResponse((MSG_PORT_FACTS_REPLY *)psReplyHeader);
			break;

		case MPI_FUNCTION_PORT_ENABLE:
			vDisplayPortEnableResponse((MSG_PORT_ENABLE_REPLY *)psReplyHeader);
			break;

		case MPI_FUNCTION_SCSI_IO_REQUEST:
			vDisplayScsiIoErrorResponse((MSG_SCSI_IO_REPLY *)psReplyHeader);
			break;

		case MPI_FUNCTION_SCSI_TASK_MGMT:
			vDisplayScsiTaskMgmtResponse((MSG_SCSI_TASK_MGMT_REPLY *)psReplyHeader);
			break;
			
		default:
			sprintf (achBuffer, "Reply Function = %02X  Length = %02X\n",
				psReplyHeader->bFunction, psReplyHeader->bMsgLength);
				
			vConsoleWrite (achBuffer);
			break;
	
	} 
#endif	
	return (psReplyHeader->wIOCStatus);
  
}


/******************************************************************************
 *
 * vProcessContextReply - Used to handle context replies (Context matches message
 *			context in the request). Response data, if any, is in the 
 *			SGL buffer specified in the request.
 *
 * RETURNS: 
 */
UINT16 wProcessContextReply(UINT32 dContext)
{
	char    achBuffer [80];
	UINT8   bFrameNo;
	UINT16  wStatus;
	MPI_REQUEST_HEADER  *psRequestHeader;
	MSG_SCSI_IO_REQUEST *psScsiIoRequest;
		
	/* Search for matching request frame */

	for(bFrameNo = 0; bFrameNo < MAX_REQUEST_FRAMES; bFrameNo++)
	{
		psRequestHeader = sRequestFrameStatus.apsFramePointer[bFrameNo];
        
		if(psRequestHeader->dMsgContext == dContext)
		{

			/* Reply frame corresponds to this request frame */
			switch(psRequestHeader->bFunction)
			{
				
				case MPI_FUNCTION_SCSI_IO_REQUEST:
					
					/* Overlay Scsi IO Request Header on Request Message */
					psScsiIoRequest = (MSG_SCSI_IO_REQUEST *)psRequestHeader;
					
					/* Handle according to Command Discriptor Block's
					    Command Byte... */
					switch(psScsiIoRequest->CDB[0])
					{
						case SCSI_INQUIRY:
							vDisplayScsiIOInquiryResponse(psScsiIoRequest);
							wStatus = MPI_IOCSTATUS_SUCCESS;
							break;

						case SCSI_TEST_UNIT_READY:
							/* Context Reply indicates Unit is Ready */
							wStatus = MPI_IOCSTATUS_SUCCESS;
							break;

						case SCSI_READ_CAPACITY:
							/* Context Reply indicates Capacity Returned */
							vDisplayScsiIOReadCapacityResponse();
							wStatus = MPI_IOCSTATUS_SUCCESS;
							break;

						case SCSI_READ:
							/* Context Reply indicates Read Returned */
							wStatus = wCheckScsiIOReadResponseForMBR();
							break;



						default:
							sprintf (achBuffer, "CDB Command = %02X\n",
                						psScsiIoRequest->CDB[0]);
                				
                					vConsoleWrite (achBuffer);
							
							wStatus = MPI_IOCSTATUS_SUCCESS;
							break;
							
					}
					
					/* End of SCSI IO REQUEST Context reply handling */
					break;		
				

                		default:
                			sprintf (achBuffer, "Matching Request Function = %02X\n",
                				psRequestHeader->bFunction);
                				
                			vConsoleWrite (achBuffer);
                			wStatus = MPI_IOCSTATUS_SUCCESS;
					break;
                				
				
			}

			/* Handled Context Reply */
			return(wStatus);
			
		}

	}
    
	/* No matching request found */
	vConsoleWrite("Failed to find matching request\n");
	
	return(MPI_STATUS_NO_CONTEXT_MATCH);
  
}



/******************************************************************************
 *
 * vDisplayScsiIoData - Here for debug - used to display SGL buffer used by IOC
 *			to return responses along with Context response.
 *
 * RETURNS: 
 */
/*
void vDisplayScsiIoData(void)
{
	char    achBuffer [80];
	UINT8	ascii;
	UINT16 i,j;
	
	vConsoleWrite("SGL Data: ");

	for (i = 0; i < 512; i += 16)
	{
		
		sprintf (achBuffer, "%04X: %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X   ",
			 i, pbSGLBuffer[i], pbSGLBuffer[i+1], pbSGLBuffer[i+2], pbSGLBuffer[i+3],
	 		    pbSGLBuffer[i+4], pbSGLBuffer[i+5], pbSGLBuffer[i+6], pbSGLBuffer[i+7],
	 		    pbSGLBuffer[i+8], pbSGLBuffer[i+9], pbSGLBuffer[i+10], pbSGLBuffer[i+11],
    	 		    pbSGLBuffer[i+12], pbSGLBuffer[i+13], pbSGLBuffer[i+14], pbSGLBuffer[i+15]);


		for (j = 0; j<16; j++)
		{
			ascii = pbSGLBuffer[i+j];

			if (ascii < 0x20 || ascii > 0x7F || ascii == 0x3C || ascii == 0x5B)
			{
				ascii = 0x20;
			}

			strncat(achBuffer, (char *)&ascii, 1);

		}
		
		vConsoleWrite(achBuffer);
		
	}	
	
}  */


/******************************************************************************
 *
 * vDisplaySenseBuffer - For Debug - displays sense data buffer used by IOC to
 *			report error status.
 *
 * RETURNS: 
 */
/*
void vDisplaySenseBuffer(void)
{
	char    achBuffer [80];
	UINT16 i,j;
	
	vConsoleWrite("SGL Data: ");

	for (i = 0; i < 18; i++)
	{
		sprintf (achBuffer, "Byte %02X: %02X", i, pbSenseBuffer[i]);
		
		vConsoleWrite(achBuffer);
	}	
	
}  */




/******************************************************************************
 *
 * vClearSGLBuffer - Zeros out SGL Buffer
 *		    Useful for ensuring no remanents of previous disk sector
 *		    read remain in the buffer.
 *
 * RETURNS: 
 */
void vClearSGLBuffer(void)
{
	UINT16	i;
	
	for (i = 0; i < SGL_BUFFER_SIZE; i++)
	{
		pbSGLBuffer[i] = 0x00;
	}

}

/******************************************************************************
 *
 * vReplyHandler -
 *
 * RETURNS: 
 */
UINT16 wReplyHandler(volatile LSI_MPT_REGS *psDevice)
{

	UINT32  dReplyMFD; /* Reply Message Frame Descriptor */
	MPI_REPLY_HEADER *psReplyHeader;
	UINT16 wStatus;
    
    
    
	/* Read from Reply Queue */
	dReplyMFD = psDevice->dReplyQueue;
    
	if(dReplyMFD == MPI_REPLY_FIFO_EMPTY)
	{
		wStatus = MPI_STATUS_QUEUE_EMPTY;
	}
	else if(dReplyMFD & MPI_ADDRESS_REPLY_A_BIT)
	{
		/* It is an Address Descriptor */

		/* Address Reply - Shift left 1 bit to get Physical Address */
		dReplyMFD = dReplyMFD << 1;
            
		/* Get Pointer to Physical Address */
		psReplyHeader = psGetReplyPointer(dReplyMFD);
            
		if (psReplyHeader == (MPI_REPLY_HEADER *)INVALID_ADDRESS)
		{
			vConsoleWrite("Invalid Reply Pointer\n");
			wStatus = MPI_STATUS_INVALID_REPLY_POINTER;
		}
		else
		{
                
			wStatus = wProcessReply(psReplyHeader);
                
			/* Free Up Request Packet corresponding to this reply */
			vReleaseMatchingRequestFrame(psReplyHeader->dMsgContext);

			/* Clear the used Reply Frame */
			memset(psReplyHeader, 0, MAX_FRAME_SIZE);
        
			/* Return Frame Address to IOC Reply FIFO */
			psDevice->dReplyQueue = dReplyMFD;
		}
            
	}
	else
	{
		/* Context Reply */
		wStatus = wProcessContextReply(dReplyMFD);
			
		/* Free Up Request Packet corresponding to this reply */
		vReleaseMatchingRequestFrame(dReplyMFD);
	}
	
	return(wStatus);
        
}



/******************************************************************************
 *
 * vInitSGL - initializes the Scatter Gather List & Sense Buffers
 *
 * RETURNS: N/A
 *
 */

void vInitSGLSenseBuffer(void)
{
//	selector sel;

	/* Create Simple Scatter Gather List (SGL) */
//	sel = set_scratch_1 (SGL_BUFFER, SGL_BUFFER_SIZE);
//	pbSGLBuffer = (far UINT8 *) buildptr (sel, (near *) NULL);
	dHandle2 = dGetPhysPtr (SGL_BUFFER, SGL_BUFFER_SIZE, &p2, (void*)&pbSGLBuffer);

	/* Create Sense Buffer */
//	sel = set_scratch_1 (SENSE_BUFFER, SENSE_BUFFER_SIZE);
//	pbSenseBuffer = (far UINT8 *) buildptr (sel, (near *) NULL);
	dHandle3 = dGetPhysPtr (SENSE_BUFFER, SENSE_BUFFER_SIZE, &p3, (void*)&pbSenseBuffer);

	
}



/******************************************************************************
 *
 * vInitMsgQueues - initializes the Request and Reply message frames.
 *
 * Clears the frames and dispatches the Reply frame addresses to the IOCs Reply
 * Free FIFO. The corresponding reply frame status is set to pending, as they
 * are waiting for the IOC to fill them.
 * Request frame status is set to available so the host driver knows it can
 * write to them.
 *
 * RETURNS: N/A
 *
 */

void vInitMsgQueues(volatile LSI_MPT_REGS *psDevice)
{
	UINT8   bFrameNo;
	UINT32  dFrameOffset;
	UINT8   *psRequestFramePtr;
	UINT8   *psReplyFramePtr;
//	selector sel;

	/* Create Request Frames 
	 * Start with selector to the Request Frame Buffer
	 * 1st frame is at the start of the buffer - i.e. At offset 0
	 */
//	sel   = set_scratch_1 (REQUEST_FRAME_BUFFER, REQUEST_FRAME_BUFFER_SIZE);
	dHandle4 = dGetPhysPtr (REQUEST_FRAME_BUFFER, REQUEST_FRAME_BUFFER_SIZE, &p4, (void*)&psRequestFramePtr);
	dFrameOffset = 0;

	for(bFrameNo = 0; bFrameNo < MAX_REQUEST_FRAMES; bFrameNo++)
	{
		UINT8 *Ptr;

		/* Create Pointer to this frame in the buffer */
//		psRequestFramePtr = (MPI_REQUEST_HEADER *) buildptr (sel,
//					(void near *)dFrameOffset);
		Ptr = psRequestFramePtr + dFrameOffset;

		/* Clear Frame */
		memset(Ptr, 0, MAX_FRAME_SIZE);

		/* Fill in Local Request Frame Data */
		sRequestFrameStatus.aeFrameState[bFrameNo] = AVAILABLE;
		sRequestFrameStatus.apsFramePointer[bFrameNo] = (MPI_REQUEST_HEADER *)Ptr;
		sRequestFrameStatus.adFramePhysicalAddress[bFrameNo] = REQUEST_FRAME_BUFFER + dFrameOffset;
                
		/* Increment Frame Ptr for next frame */
		dFrameOffset += MAX_FRAME_SIZE;

	}

	/* Reset the Request Frame Credit Counter (Downcounter) */
	sRequestFrameStatus.wCreditRemaining = sIocFactsReply.GlobalCredits;

	/* Create Reply Frames 
	 * Start with selector to the Reply Frame Buffer
	 * 1st frame is at the start of the buffer - i.e. At offset 0
	 */
//	sel   = set_scratch_1 (REPLY_FRAME_BUFFER, REPLY_FRAME_BUFFER_SIZE);
	dHandle5 = dGetPhysPtr (REPLY_FRAME_BUFFER, REPLY_FRAME_BUFFER_SIZE, &p5, (void*)&psReplyFramePtr);
	dFrameOffset = 0;

	for(bFrameNo = 0; bFrameNo < MAX_REPLY_FRAMES; bFrameNo++)
	{
		UINT8 *Ptr;

		/* Create Pointer to this frame in the buffer */
//		psReplyFramePtr = (MPI_REPLY_HEADER *) buildptr (sel, (void near *)dFrameOffset);
		Ptr = (((UINT8*)psReplyFramePtr) + dFrameOffset);
                
		/* Clear Frame */
		memset(Ptr, 0, MAX_FRAME_SIZE);

		/* Fill in Local Reply Frame Data:- PENDING (With IOC) */
		sReplyFrameStatus.aeFrameState[bFrameNo] = PENDING;
		sReplyFrameStatus.apsFramePointer[bFrameNo] = (MPI_REPLY_HEADER *)Ptr;
		sReplyFrameStatus.adFramePhysicalAddress[bFrameNo] = REPLY_FRAME_BUFFER + dFrameOffset;
                        
		/* Send Frame Address to IOC Reply FIFO */
		psDevice->dReplyQueue = sReplyFrameStatus.adFramePhysicalAddress[bFrameNo];

		/* Increment Frame Ptr for next frame */
		dFrameOffset += MAX_FRAME_SIZE;

	}
    
}



/******************************************************************************
 *
 * psGetRequestFrame - Gets Frame No of available Request Frame
 *
 * Only issues a request frame if host driver has available frame AND in credit
 * with IOC.
 *
 * RETURNS: Frame No or 0xFFFFFFFF (INVALID_ADDRESS)
 *
 */
MPI_REQUEST_HEADER *psGetRequestFrame(volatile LSI_MPT_REGS *psDevice)
{
	UINT8   bFrameNo;
	MPI_REQUEST_HEADER *psRequestHeader;
        
	/* Check there is capacity in Request FIFO - i.e we are in credit */
	if(sRequestFrameStatus.wCreditRemaining)
	{
		/* Check we have an available Request frame */
		for(bFrameNo = 0; bFrameNo < MAX_REPLY_FRAMES; bFrameNo++)
		{
			if(sRequestFrameStatus.aeFrameState[bFrameNo] == AVAILABLE)
			{
				psRequestHeader = sRequestFrameStatus.apsFramePointer[bFrameNo];
                
				/* Clean the Request Frame so caller doesn't have to */
				memset(psRequestHeader, 0, MAX_FRAME_SIZE);
                
				return (psRequestHeader);
			}
		}
        
	}

	/* If we got here, either there is no available frame or no credit with IOC */
	return((MPI_REQUEST_HEADER *)INVALID_ADDRESS);
    
}


/******************************************************************************
 *
 * bSendRequestFrame - Sends a Request Frame to the IOC
 *
 * Fills in the Context Field so that the reply can be associated with the
 * request.
 *
 * RETURNS: BOOLEAN
 *
 */
UINT8 bSendRequestFrame(volatile LSI_MPT_REGS *psDevice, 
			MPI_REQUEST_HEADER *psFrameHeader)
{
	UINT8 bFrameNo;
	UINT8 bStatus;
	
	if(sRequestFrameStatus.wCreditRemaining)
	{
		
		/* Find Frame Number corresponding to psFrameHeader */
		for(bFrameNo = 0; bFrameNo < MAX_REPLY_FRAMES; bFrameNo++)
		{
			if(sRequestFrameStatus.apsFramePointer[bFrameNo] == psFrameHeader)
			{
				/* Increment Context Number for new message.
				 * Enables response to associated with this request.
				 */

				psFrameHeader->dMsgContext = (UINT32)wContext;

				/* Mark Frame as Pending with IOC */
				sRequestFrameStatus.aeFrameState[bFrameNo] = PENDING;

				/* Send Physical Address of Request Frame to IOC */
				psDevice->dRequestQueue = sRequestFrameStatus.adFramePhysicalAddress[bFrameNo];
                
				/* Successful - Quit loop */
				bStatus = PASS;
				break;
        
			}
		}

	}
	else
	{
		vConsoleWrite ("ERROR - No Credit\n");
		bStatus = FAIL;
	}

	return bStatus;
}



/******************************************************************************
 *
 * wMPISendRequest - Sends Message Passing Interface Request. All Requests to
 *			the IOC come through here (except doorbell requests).
 *
 *			Finds an available request frame, copies the request
 *			into it, enables interrupt for the response, and calls
 *			the response handler 
 *
 * RETURNS: 
 */
UINT16 wMPISendRequest(DEVICE *psDevice, MPI_REQUEST_HEADER *psRequest, UINT8 bRequestLength)
{
	
	MPI_REQUEST_HEADER *psMPIRequest;
	UINT16 wStatus;
	
#ifdef DEBUG
	vConsoleWrite ("wMPISendRequest\n");
#endif
	/* First find an available Request Frame */

	psMPIRequest = (MPI_REQUEST_HEADER *)psGetRequestFrame(psDevice->psDevRegs);

	if(psMPIRequest == (MPI_REQUEST_HEADER  *)INVALID_ADDRESS)
	{
		vConsoleWrite ("No Request Frame or Credit\n");
		return(MPI_STATUS_NO_REQUEST_FRAME_OR_CREDIT);
	}
	

	/* Create MPI Request - Copy from request */
	memcpy(psMPIRequest, psRequest, bRequestLength);
	
	/* Clear IRQ flag */
	bIRQState = IRQ_NOT_PENDING;

	/* unmask the reply message interrupt */
	psDevice->psDevRegs->dHostInterruptMask &= ~MPI_HIS_REPLY_MESSAGE_INTERRUPT;
	
	/* Send the Message */
	if(bSendRequestFrame(psDevice->psDevRegs, psMPIRequest) == FAIL)
	{
		vConsoleWrite ("Send Frame Failed\n");
		wStatus = MPI_STATUS_SEND_REQUEST_FAILED;
	}
	else
	{
		wStatus = wMPTIntHandler(psDevice);
	}
	
	return (wStatus);
	
}




/******************************************************************************
 *
 * vInitController - initializes the SCSI PCI configuration registers
 *
 * This function initialises the SCSI PCI.
 *
 * RETURNS: N/A
 *
 */

void vInitController
(
    DEVICE *psDevice
)
{
	UINT16  wTemp;
	PCI_PFA Pfa = psDevice->Pfa;

	/* disable memory and I/O accesses */

	wTemp  = PCI_READ_WORD(Pfa, PCI_COMMAND);
	wTemp &= ~(PCI_MEMEN | PCI_IOEN);
	PCI_WRITE_WORD(Pfa, PCI_COMMAND, wTemp);


	/*
	 * Determine the size of the expansion ROM, set the maximum latency
	 * timer and initialize the command register.
	 */

	PCI_WRITE_BYTE(Pfa, PCI_LATENCY, 0x10);

	/* enable I/O, MEM, Bus Mastering and SERR */

	wTemp  = PCI_READ_WORD(Pfa, PCI_COMMAND);
	wTemp &= ~0x0147;
	wTemp |=  0x0107;
	PCI_WRITE_WORD(Pfa, PCI_COMMAND, wTemp);

}



/******************************************************************************
 *
 * wRegisterTest - Test access to device registers 
 *
 * RETURNS: 
 */
UINT16 wRegisterTest ( DEVICE *psDevice)
{
	/* Since there are no useful memory mapped device 
	 * registers to test, we will have to use the 
	 * I/O base register (BAR1)
	 */

	PCI_PFA Pfa = psDevice->Pfa;
	UINT16	wSavedCommand;
	UINT16	wStatus = E_OK;
	UINT32	dSavedBar;
	UINT32	dValue;
	UINT32	dRegValue;
	UINT8 	i;


	vConsoleWrite("Register Test....\n");
	
	wSavedCommand  = PCI_READ_WORD(Pfa, PCI_COMMAND);
		
	/* get the I/O base reg and disable the device */
	dSavedBar = PCI_READ_DWORD(Pfa, PCI_IO_BAR);

	for (i = 8; (i < 32) && (wStatus == E_OK); i++)

	{
		dValue = 1L << i;
		dValue |= 1L; /* Bit 0 always reads back as 1 */

		PCI_WRITE_DWORD(Pfa, PCI_IO_BAR, dValue);
		dRegValue = PCI_READ_DWORD(Pfa, PCI_IO_BAR);

		if (dRegValue != dValue)
		{
			sprintf (psDevice->achErrorMsg, "Walking \"1\" Error bit %d Wrote %08X, Read %08X\n", i, dValue, dRegValue);
			vConsoleWrite(psDevice->achErrorMsg);
			wStatus = E_1s_ERROR;
		}
	}

	for (i = 8; (i < 32) && (wStatus == E_OK); i++)
	{
		dValue = ~(1L << i);
		dValue &= 0xFFFFFF01L;

		PCI_WRITE_DWORD(Pfa, PCI_IO_BAR, dValue);
		dRegValue = PCI_READ_DWORD(Pfa, PCI_IO_BAR);

		if (dRegValue != dValue)
		{
			sprintf (psDevice->achErrorMsg, "Walking \"1\" Error bit %d Wrote %08X, Read %08X\n", i, dValue, dRegValue);
			vConsoleWrite(psDevice->achErrorMsg);
			wStatus = E_0s_ERROR;
		}
	}

	/* restore the registers */
	PCI_WRITE_DWORD(Pfa, PCI_IO_BAR, dSavedBar);
	
	PCI_WRITE_WORD(Pfa, PCI_COMMAND, wSavedCommand);

	if (wStatus == E_OK)
	{
		vConsoleWrite ("     PASS\n");
	}
	
	return wStatus;
}




/******************************************************************************
 *
 * wIOCStateTest -
 *
 * RETURNS: 
 */
UINT16 wIOCStateTest(volatile LSI_MPT_REGS *psDevice)
{
	UINT16	wStatus = E_OK;
	
	/* IOC state reported in upper nibble of System Doorbell Register */
	switch(psDevice->dSystemDoorbell & MPI_IOC_STATE_MASK)
	{
		case MPI_IOC_STATE_RESET:
			vConsoleWrite("IOC STATE:- RESET\n");
			wStatus = E_TEST_FAIL;
			break;
            
		case MPI_IOC_STATE_READY:
			/* Normal before inititialisation don't display anything */
			 vConsoleWrite("IOC STATE:- READY\n");
			break;
            
		case MPI_IOC_STATE_OPERATIONAL:
			/* Normal after initialisation don't display anything */
			 vConsoleWrite("IOC STATE:- OPERATIONAL\n");
			break;
            
		case MPI_IOC_STATE_FAULT:
	    		vConsoleWrite("IOC STATE:- FAULT\n");
			wStatus = E_TEST_FAIL;
	    		break;
            
		default:
			vConsoleWrite("IOC STATE:- UNKNOWN - SPOOKY!\n");
			wStatus = E_TEST_FAIL;
			break;
            
	}
    
	return wStatus;
}



/******************************************************************************
 *
 * wIOCFactsDoorbellRequest - Issues an IOC Facts Request via the Doorbell
 *
 * RETURNS: 
 */
UINT16 wIOCFactsDoorbellRequest(volatile LSI_MPT_REGS *psDevice)
{
	MSG_IOC_FACTS sIocFactsRequest;
	UINT16 wStatus;

	/* Make sure doorbell is not in use already */
	if(psDevice->dSystemDoorbell & MPI_DOORBELL_USED)
	{
		vConsoleWrite("Doorbell in use\n");
		return E_TEST_FAIL;
	}
    
    
	/* Create IOC_Facts Request */
	memset(&sIocFactsRequest, 0, sizeof(MSG_IOC_FACTS));
	sIocFactsRequest.Function = MPI_FUNCTION_IOC_FACTS;
    
	/* Clear IOC_Reply Space */
	memset(&sIocFactsReply, 0, sizeof(MSG_IOC_FACTS_REPLY));
    
	/* Send message via Doorbell Register */
	wStatus = wMPT_DoorbellSendMsg(psDevice, (UINT8 *)&sIocFactsRequest, 
					sizeof(MSG_IOC_FACTS), (UINT8 *)&sIocFactsReply);
    
	return wStatus;
    
}




/******************************************************************************
 *
 * wIOCFactsRequest - Sends IOCFacts request via message queue. Should get
 *                    response indicated by interrupt.
 *
 * RETURNS: 
 */
UINT16 wIOCFactsRequest(DEVICE *psDevice)
{
	MSG_IOC_FACTS sIocFactsRequest;
	
	/* Zero all fields of request  */
	memset(&sIocFactsRequest, 0, sizeof(MSG_IOC_FACTS));

	/* Create IOC_Facts Request */
	sIocFactsRequest.Function = MPI_FUNCTION_IOC_FACTS;
	
	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sIocFactsRequest, sizeof(MSG_IOC_FACTS)));
}


/******************************************************************************
 *
 * wPortFactsRequest - Sends Port Facts request via message queue. Should get
 *                    response indicated by interrupt.
 *
 * RETURNS: 
 */
UINT16 wPortFactsRequest(DEVICE *psDevice)
{
	MSG_PORT_FACTS sPortFactsRequest;

	/* Zero all fields of request  */
	memset(&sPortFactsRequest, 0, sizeof(MSG_PORT_FACTS));
	
	/* Create Port_Facts Request */
	sPortFactsRequest.Function = MPI_FUNCTION_PORT_FACTS;
	sPortFactsRequest.PortNumber = 0;

	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sPortFactsRequest, sizeof(MSG_PORT_FACTS)));

}


/******************************************************************************
 *
 * wPortEnableRequest - Sends Port Enable request via message queue. Should get
 *                    response indicated by interrupt.
 *
 * RETURNS: 
 */
UINT16 wPortEnableRequest(DEVICE *psDevice)
{
	MSG_PORT_ENABLE sPortEnableRequest;

	/* Zero all fields of request  */
	memset(&sPortEnableRequest, 0, sizeof(MSG_PORT_ENABLE));

	/* Create Port_Enable Request */
	sPortEnableRequest.Function = MPI_FUNCTION_PORT_ENABLE;
	sPortEnableRequest.PortNumber = 0;

	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sPortEnableRequest, sizeof(MSG_PORT_ENABLE)));
}







/******************************************************************************
 *
 * wScsiInquiry - Constructs & Sends SCSI Inquiry
 *
 * RETURNS: 
 */
UINT16 wScsiInquiry(DEVICE *psDevice, UINT8 bBus, UINT8 bTarget, UINT8 bLun)
{
	MSG_SCSI_IO_REQUEST sScsiIoRequest;
	
	/* Zero all fields of request */
	memset (&sScsiIoRequest, 0, sizeof(MSG_SCSI_IO_REQUEST));

	sScsiIoRequest.Function = MPI_FUNCTION_SCSI_IO_REQUEST;
	sScsiIoRequest.TargetID = bTarget;
	sScsiIoRequest.Bus = bBus;
	sScsiIoRequest.CDBLength = 6;
	sScsiIoRequest.SenseBufferLength = SENSE_BUFFER_SIZE;
	sScsiIoRequest.LUN[1] = bLun;
	sScsiIoRequest.Control = MPI_SCSIIO_CONTROL_READ | MPI_SCSIIO_CONTROL_SIMPLEQ;
	sScsiIoRequest.CDB[0] = SCSI_INQUIRY;
	sScsiIoRequest.CDB[4] = 36;
	sScsiIoRequest.DataLength = 36;
	sScsiIoRequest.SenseBufferLowAddr = SENSE_BUFFER;
	sScsiIoRequest.SGL.u.Simple.FlagsLength = SGL_FLAGS_LENGTH_READ;
	sScsiIoRequest.SGL.u.Simple.u.Address32 = SGL_BUFFER;

	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sScsiIoRequest, sizeof(MSG_SCSI_IO_REQUEST)));
	
}

/******************************************************************************
 *
 * wReadMBR - Constructs & Sends SCSI IO Read request of the first Logical Block
 *		on the disk. This should contain the Master Boot Record, which 
 *		we'll check when we get the response.
 *
 * RETURNS: 
 */
UINT16 wReadMBR(DEVICE *psDevice, UINT8 bBus, UINT8 bTarget, UINT8 bLun)
{
	MSG_SCSI_IO_REQUEST sScsiIoRequest;
	
	/* Zero all fields of request */
	memset (&sScsiIoRequest, 0, sizeof(MSG_SCSI_IO_REQUEST));

	sScsiIoRequest.Function = MPI_FUNCTION_SCSI_IO_REQUEST;
	sScsiIoRequest.TargetID = bTarget;
	sScsiIoRequest.Bus = bBus;
	sScsiIoRequest.CDBLength = 6;
	sScsiIoRequest.SenseBufferLength = SENSE_BUFFER_SIZE;
	sScsiIoRequest.LUN[1] = bLun;
	sScsiIoRequest.Control = MPI_SCSIIO_CONTROL_READ | MPI_SCSIIO_CONTROL_SIMPLEQ;
	sScsiIoRequest.CDB[0] = SCSI_READ;
	sScsiIoRequest.CDB[1] = 0;
	sScsiIoRequest.CDB[2] = 0;
	sScsiIoRequest.CDB[3] = 0;
	sScsiIoRequest.CDB[4] = 1;
	sScsiIoRequest.DataLength = 0x0200L;
	sScsiIoRequest.SenseBufferLowAddr = SENSE_BUFFER;
	sScsiIoRequest.SGL.u.Simple.FlagsLength = SGL_FLAGS_LENGTH_READ;
	sScsiIoRequest.SGL.u.Simple.u.Address32 = SGL_BUFFER;

	/* Clear SGL Buffer vefore sending the request, so no remanents of previous read are interpretted */
	vClearSGLBuffer();
	
	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sScsiIoRequest, sizeof(MSG_SCSI_IO_REQUEST)));
	
}



/******************************************************************************
 *
 * wReadCapacity - Constructs & Sends SCSI IO Read Capacity Request. The response
 *		will contain the number of Logical Blocks, and the Block size,
 *		and will be used to calculate the drive capacity.
 *
 * RETURNS: 
 */
UINT16 wReadCapacity(DEVICE *psDevice, UINT8 bBus, UINT8 bTarget, UINT8 bLun)
{
	MSG_SCSI_IO_REQUEST sScsiIoRequest;
	
	/* Zero all fields of request */
	memset (&sScsiIoRequest, 0, sizeof(MSG_SCSI_IO_REQUEST));

	sScsiIoRequest.Function = MPI_FUNCTION_SCSI_IO_REQUEST;
	sScsiIoRequest.TargetID = bTarget;
	sScsiIoRequest.Bus = bBus;
	sScsiIoRequest.CDBLength = 10;
	sScsiIoRequest.SenseBufferLength = SENSE_BUFFER_SIZE;
	sScsiIoRequest.LUN[1] = bLun;
	sScsiIoRequest.Control = MPI_SCSIIO_CONTROL_READ | MPI_SCSIIO_CONTROL_SIMPLEQ;
	sScsiIoRequest.CDB[0] = SCSI_READ_CAPACITY;
	sScsiIoRequest.DataLength = 8;
	sScsiIoRequest.SenseBufferLowAddr = SENSE_BUFFER;
	sScsiIoRequest.SGL.u.Simple.FlagsLength = SGL_FLAGS_LENGTH_READ;
	sScsiIoRequest.SGL.u.Simple.u.Address32 = SGL_BUFFER;

	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sScsiIoRequest, sizeof(MSG_SCSI_IO_REQUEST)));
	
}


/******************************************************************************
 *
 * wStartUnit - Constructs & Sends Start Unit Request
 *
 * RETURNS: 
 */
UINT16 wStartUnit(DEVICE *psDevice, UINT8 bBus, UINT8 bTarget, UINT8 bLun)
{
	MSG_SCSI_IO_REQUEST sScsiIoRequest;
	
	/* Zero all fields of request */
	memset (&sScsiIoRequest, 0, sizeof(MSG_SCSI_IO_REQUEST));

	sScsiIoRequest.Function = MPI_FUNCTION_SCSI_IO_REQUEST;
	sScsiIoRequest.TargetID = bTarget;
	sScsiIoRequest.Bus = bBus;
	sScsiIoRequest.CDBLength = 6;
	sScsiIoRequest.SenseBufferLength = SENSE_BUFFER_SIZE;
	sScsiIoRequest.LUN[1] = bLun;
	sScsiIoRequest.Control = MPI_SCSIIO_CONTROL_NODATATRANSFER | MPI_SCSIIO_CONTROL_SIMPLEQ;
	sScsiIoRequest.CDB[0] = SCSI_START_STOP_UNIT;
	sScsiIoRequest.CDB[4] = 0x01; /* Start */
	sScsiIoRequest.DataLength = 0;
	sScsiIoRequest.SenseBufferLowAddr = SENSE_BUFFER;
	sScsiIoRequest.SGL.u.Simple.FlagsLength = SGL_FLAGS_LENGTH_READ;
	sScsiIoRequest.SGL.u.Simple.u.Address32 = SGL_BUFFER;

	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sScsiIoRequest, sizeof(MSG_SCSI_IO_REQUEST)));
	
}


/******************************************************************************
 *
 * wTestUnitReady - Tests for unit readiness.
 *
 * RETURNS: 
 */
UINT16 wTestUnitReady(DEVICE *psDevice, UINT8 bBus, UINT8 bTarget, UINT8 bLun)
{
	MSG_SCSI_IO_REQUEST sScsiIoRequest;
	
	/* Zero all fields of request */
	memset (&sScsiIoRequest, 0, sizeof(MSG_SCSI_IO_REQUEST));

	sScsiIoRequest.Function = MPI_FUNCTION_SCSI_IO_REQUEST;
	sScsiIoRequest.TargetID = bTarget;
	sScsiIoRequest.Bus = bBus;
	sScsiIoRequest.CDBLength = 6;
	sScsiIoRequest.SenseBufferLength = SENSE_BUFFER_SIZE;
	sScsiIoRequest.LUN[1] = bLun;
	sScsiIoRequest.Control = MPI_SCSIIO_CONTROL_NODATATRANSFER | MPI_SCSIIO_CONTROL_SIMPLEQ;
	sScsiIoRequest.CDB[0] = SCSI_TEST_UNIT_READY;
	sScsiIoRequest.DataLength = 0;
	sScsiIoRequest.SenseBufferLowAddr = SENSE_BUFFER;
	sScsiIoRequest.SGL.u.Simple.FlagsLength = SGL_FLAGS_LENGTH_READ;
	sScsiIoRequest.SGL.u.Simple.u.Address32 = SGL_BUFFER;

	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sScsiIoRequest, sizeof(MSG_SCSI_IO_REQUEST)));
}


/******************************************************************************
 *
 * wTargetReset - Issues Bus Reset - will also reset all targets
 *
 * RETURNS: 
 */
UINT16 wBusReset(DEVICE *psDevice, UINT8 bBus, UINT8 bTarget, UINT8 bLun)
{
	MSG_SCSI_TASK_MGMT sScsiTaskMgmtRequest;
	
	/* Zero all fields of request */
	memset (&sScsiTaskMgmtRequest, 0, sizeof(MSG_SCSI_TASK_MGMT));

	sScsiTaskMgmtRequest.Function = MPI_FUNCTION_SCSI_TASK_MGMT;
	sScsiTaskMgmtRequest.TargetID = bTarget;
	sScsiTaskMgmtRequest.Bus = bBus;
	sScsiTaskMgmtRequest.TaskType = MPI_SCSITASKMGMT_TASKTYPE_TARGET_RESET;
	sScsiTaskMgmtRequest.LUN[1] = bLun;
	sScsiTaskMgmtRequest.TaskMsgContext = 0x00;

	return (wMPISendRequest(psDevice, (MPI_REQUEST_HEADER *)&sScsiTaskMgmtRequest, sizeof(MSG_SCSI_TASK_MGMT)));
}



/******************************************************************************
 *
 * wScanForDevicesTest - Scans for devices on the port
 *
 * RETURNS: 
 */

UINT16 wScanForDevicesTest(DEVICE *psDevice)
{
	UINT8 bMaxBusses, bBus;
	UINT8 bMaxDevices, bDevice;
	UINT8 /*bMaxLuns, bLun,*/ bDevicesFound;
	UINT16 wScanStatus, wStatus;
		
	bMaxBusses = sIocFactsReply.MaxBuses;
	bMaxDevices = sIocFactsReply.MaxDevices;
	bDevicesFound = 0;
	
	vConsoleWrite ("Scanning ...\n");

	/* Expect the worst */
	wScanStatus = MPI_TEST_SEQUENCE_FAILED;

	for(bBus = 0; bBus < bMaxBusses; bBus++)
	{
		for(bDevice = 0; bDevice < bMaxDevices; bDevice++)
		{
			wStatus = wScsiInquiry(psDevice, bBus, bDevice, 0);
			if(wStatus == MPI_IOCSTATUS_SUCCESS)
			{
				/* Do a Test Unit Ready to flush any pending
				   Sense Data prior to requesting drive capacity */
				wStatus = wTestUnitReady(psDevice, bBus, bDevice, 0);
				
				/* May need to do more processing with response to above */
				
				/* New device found - reset overall scan status */
				wScanStatus = MPI_TEST_SEQUENCE_FAILED;
				
				bDevicesFound++;
			}

			if(wStatus == MPI_IOCSTATUS_SUCCESS)
			{
				/* Read Capacity */
				wStatus = wReadCapacity(psDevice, bBus, bDevice, 0);
				
				if(wStatus != MPI_IOCSTATUS_SUCCESS)
				{
					vConsoleWrite("        Read Drive Capacity Failure\n");
				}
			}

			if(wStatus == MPI_IOCSTATUS_SUCCESS)
			{
				/* Read Master Boot Record */
				wStatus = wReadMBR(psDevice, bBus, bDevice, 0);

				if(wStatus == MPI_IOCSTATUS_SUCCESS)
				{
					/* Inquiry, Capacity & ReadMBR successfull. */
					wScanStatus = MPI_IOCSTATUS_SUCCESS;
					 
				}			

			}
			
			if(wStatus == MPI_STATUS_NO_REPLY_INTERRUPT)
			{
				/* We could be here a long time - quit */
				wScanStatus = MPI_TEST_SEQUENCE_FAILED;
				break;
			}
			

		}
		
		
	}

	if(bDevicesFound == 0)
	{
		vConsoleWrite("No Devices Found\n");
	}
	
	return (wScanStatus);
}


/******************************************************************************
 *
 * wIOCInitTest -
 *
 * RETURNS: 
 */
UINT16 wIOCInitTest(volatile LSI_MPT_REGS *psDevice)
{
	MSG_IOC_INIT sIOCInitRequest;
	MSG_IOC_INIT_REPLY sIOCInitReply;
//	volatile UINT8 *pbHostPageBuffer;
//	selector selDevice;
	UINT16 wStatus;

	/* Create Host Page Buffer of size requested in IOC Facts Reply */
//	selDevice = set_scratch_1 (HOST_PAGE_BUFFER,
//			sIocFactsReply.HostPageBufferSGE.FlagsLength & 0x00FFFFFF);
//	pbHostPageBuffer = (far UINT8 *) buildptr (selDevice, (near *) NULL);
//	dHandle5 = dGetPhysPtr (HOST_PAGE_BUFFER, sIocFactsReply.HostPageBufferSGE.FlagsLength & 0x00FFFFFF, &p5, (void*)&pbHostPageBuffer);
    
	/* Clear and fill in Init Request Message */
	memset(&sIOCInitRequest, 0, sizeof(MSG_IOC_INIT));

	sIOCInitRequest.WhoInit = MPI_WHOINIT_HOST_DRIVER;
	sIOCInitRequest.Function = MPI_FUNCTION_IOC_INIT;
	sIOCInitRequest.Flags = 0x00; /* Disables FIFO Host Signaling */
	sIOCInitRequest.MaxDevices = sIocFactsReply.MaxDevices;
	sIOCInitRequest.MaxBuses = 0x01; /* MPT_MAX_BUS;*/   /* 0x01 */
	sIOCInitRequest.MsgVersion = MPI_VERSION;
	sIOCInitRequest.ReplyFrameSize = 0x50; /* MPT_REPLY_FRAME_SIZE;*/ /* 0x50 */
/*    sIOCInitRequest.HostPageBufferSGE.FlagsLength = (0xD1 << MPI_SGE_FLAGS_SHIFT) | (sIocFactsReply.HostPageBufferSGE.FlagsLength  & 0x00FFFFFF);*/
	sIOCInitRequest.HostPageBufferSGE.FlagsLength = (0xD1 << MPI_SGE_FLAGS_SHIFT) | (0x50);
	sIOCInitRequest.HostPageBufferSGE.u.Address32 = HOST_PAGE_BUFFER;

    
	wStatus = wMPT_DoorbellSendMsg(psDevice, (UINT8 *)&sIOCInitRequest,
			sizeof(MSG_IOC_INIT), (UINT8 *)&sIOCInitReply);
    
	return wStatus;
        
}


void vStartINTHandler (DEVICE *psDevice)
{
	PCI_INTERRUPT_INFO IntInfo;
	PCI_INTERRUPT_INFO *psIntInfo;
#ifdef DEBUG
	char achBuffer[80];
#endif

	/* Get IRQ Vector for this PCI Device */
	IntInfo.Pfa   = psDevice->Pfa;
	IntInfo.IrqNo = 0;
	psIntInfo     = &IntInfo;
	board_service (SERVICE__BRD_GET_PCI_INT_INFO, NULL, (void *)&psIntInfo);
	bIRQNumber    = IntInfo.IrqNo;
	psDUT         = psDevice;
#ifdef DEBUG
	sprintf (achBuffer,"Using IRQ %d\n", bIRQNumber);
	vConsoleWrite(achBuffer);
#endif

//	bIRQNumber = PCI_READ_BYTE(Pfa, PCI_INT_LINE);
	/* Find vector used by system for chosen IOAPIC pin */
	iVector = sysPinToVector (bIRQNumber, SYS_IOAPIC0);

	/* Install interrupt handler and enable interrupts for this device */

	sysInstallUserHandler (iVector, vMPTIntPassHandler);
	sysUnmaskPin (bIRQNumber, SYS_IOAPIC0);

	/* Enable Interrupts */

	sysEnableInterrupts();
	bINTsEnabled = 1;	

}

void vStopINTHandler (void)
{
//       	disable();
//       	CFS_mask_slot (bIRQVector);
	sysInstallUserHandler (iVector, 0);
	bINTsEnabled = 0;
	psDUT        = NULL;
}



/******************************************************************************
 *
 * wMPIStartUp - Confirm that Fusion MPT device is working, and in a ready
 *		 state. Then issues a sequence of commands to setup the device,
 *		 enable ports and make it ready for accepting SCSI commands.
 
 * RETURNS: E_OK or E_...
 */
UINT16 wMPIStartUp(DEVICE *psPciDevice)
{
	UINT16 wTestStatus;

	
	vInitController(psPciDevice);

#if 1		
	/*Hari : Reset the SAS Controller */
	psPciDevice->psDevRegs->dHostDiagnostic |= 0x00000004;
	
	vDelay (1000); /*Allow Controller to reset */
	
	psPciDevice->psDevRegs->dHostDiagnostic &= ~(0x00000004);
	
	vDelay (1000);
#endif
	vIoWriteReg (psPciDevice->wIoBaseAddress + MPI_WRITE_SEQUENCE_OFFSET, REG_8, 0xAA);
  	vDelay (2000);
  	
	wTestStatus = wIOCStateTest (psPciDevice->psDevRegs);
		
	if(wTestStatus == E_OK)
	{
		wTestStatus = wIOCFactsDoorbellRequest (psPciDevice->psDevRegs);
	}
        
	if(wTestStatus == E_OK)
    {
        	wTestStatus = wIOCInitTest (psPciDevice->psDevRegs);
    }
        
        if(wTestStatus == E_OK)
	{
		/* Initialise Request & Reply Buffers */
		vInitMsgQueues (psPciDevice->psDevRegs);

		/* Initialise Scatter Gather List Buffer, and Sense Buffer */
		vInitSGLSenseBuffer();

		/* Start Interrupt Handler */
		vStartINTHandler (psPciDevice);

	}
        		
	if(wTestStatus == E_OK)
	{
		wTestStatus = wPortFactsRequest (psPciDevice);

		if(wTestStatus == MPI_IOCSTATUS_SUCCESS)
		{
			wTestStatus = wPortEnableRequest (psPciDevice);
			
			if(wTestStatus != MPI_IOCSTATUS_SUCCESS)
			{
				vConsoleWrite("NO Port Enable Response...\n");
			}
			
		}				
	}
	
	return (wTestStatus);
	
}



/******************************************************************************
 *
 * wScsiTest - Performs the selected Scsi Test
 * RETURNS: Bist Error Codes E_OK, E_TEST_FAIL, E_TEST_NOT_FOUND
 */
UINT16 wScsiTest(DEVICE *psPciDevice, UINT8 bTestChoice)
{
	UINT16	wIOCStatus, wBistStatus;
	UINT16	wReturnStatus = E_TEST_FAIL;

	/* Flush any replies in the reply fifo and handle them,
	   otherwise Context replies might arrive out of synch	
	   with the SGL buffer containing response data */
	   
	
	do
	{
		wIOCStatus =  wReplyHandler(psPciDevice->psDevRegs);
		
	} while (wIOCStatus != MPI_STATUS_QUEUE_EMPTY);
	
	if (bTestChoice == 0)
	{
		/* Test Controller Only - start with register access */
		wBistStatus = wRegisterTest (psPciDevice);
		
		if(wBistStatus == E_OK)
		{
			/* Test Message Passing Interface */
			vConsoleWrite("Testing Message Passing Interface & IRQs...\n");
			
			wIOCStatus = wIOCFactsRequest (psPciDevice);
			
			if(wIOCStatus == MPI_IOCSTATUS_SUCCESS)
			{
				vConsoleWrite("     PASS\n");
				wReturnStatus = E_OK;
			}
			else
			{
				vConsoleWrite("     FAIL\n");
			}
		}
		else
		{
			vConsoleWrite(psPciDevice->achErrorMsg);
		}
	}
			

	else if (bTestChoice == 1)
	{
		wIOCStatus = wScanForDevicesTest(psPciDevice);

		if(wIOCStatus == MPI_IOCSTATUS_SUCCESS)
		{
			wReturnStatus = E_OK;
		}
	}

	else
	{
		wReturnStatus = E_TEST_NOT_FOUND;
	}

	return(wReturnStatus);
	
} 




/* public BIST functions */
 
/******************************************************************************
 *
 * wPciMPTGeneralTest - Suite of general SCSI controller Tests
 * 
 * Take 2 parameters.
 *  1st Parameter is test type :-
 *    0 	- Test Discovered Controllers
 *    1 	- Scan for Devices
 *    2 	- Test Expected Controllers
 *      
 *  2nd Parameter is 
 *    0 	- All instances
 *    1,2...- Instance number to test  
 *
 * RETURNS: E_OK or E_...
 */

TEST_INTERFACE (wPciMPTGeneralTest, "MPT SCSI/SATA Controller Tests")
{
	struct	{
		UINT8 bTestChoice;
		UINT8 bInstance;
		PCI_PFA   pfa;
		} sParameters;

	DEVICE	sPciDevice;
	const char	*pDescription;
//	UINT16		wMddxStatus;
	UINT16		wTestStatus;
	UINT32		dReturnStatus = E_OK;
	char achBuffer [80];
	UINT8 bLoop;
    UINT8 bExpectedInstances, fCheckInstances;

    vInitMemHandles();

	sParameters.bTestChoice = 0;	/* default: select Test 0  */
	sParameters.bInstance   = 0;	/* default: Test All MPT Fusion SCSI controllers */

	bExpectedInstances = 0;			/* default: Don't expect specific no of controllers */
    fCheckInstances = FALSE;

	if(adTestParams[1])
	{
		sParameters.bTestChoice = adTestParams[2];
		
		if (adTestParams[1] == 2)
			sParameters.bInstance = adTestParams[3];
			
	}

	if(sParameters.bTestChoice == 2)
	{
		/* Actual Test is Controller Test with Expected Instance comparisom at end */
		sParameters.bTestChoice = 0;
			
		/* Second parameter is expected instances */
		bExpectedInstances = sParameters.bInstance;
		fCheckInstances = TRUE;
			
		/* Test all discovered instances */
		sParameters.bInstance   = 0;
			
	}

	/* Interrupt handlers disabled at start */
	bINTsEnabled = 0;
	wContext     = 0;
	psDUT        = NULL;
	
	/* If 'test all' selected, set the loop flag and set instance to first */
	bLoop = 0;

	if (sParameters.bInstance == 0)
	{
		bLoop = 1;				
		sParameters.bInstance++;
	}

	/* Now we know which controller(s) to test and which tests to perform.
	 *
	 * For each selected controller instance, find the controller then call to
	 * the general test routine.
	 */

	do
	{
		
       	/* check PCI bus for LSI10x0 SCSI controllers */
       	if (wFindMPTController (&sPciDevice.Pfa, &pDescription,
       				sParameters.bInstance) == E_OK)
       	{
       		vPrintDeviceDetails(sPciDevice.Pfa, pDescription);
                
       		/* Get Pointer to Register Structure in Memory Space */
       		sPciDevice.psDevRegs = psGetDevicePtr(&sPciDevice);
			
			/* Get Message Passing Interface Running, Device Enabled,
			   Ports Enabled, Ready to Handler SCSI Commands */
			wTestStatus = wMPIStartUp(&sPciDevice);
			
			if(wTestStatus == MPI_IOCSTATUS_SUCCESS)
			{
				vConsoleWrite ("Calling wScsiTest\n");
				wTestStatus = wScsiTest(&sPciDevice, sParameters.bTestChoice);
				
			}
			else
			{
				vConsoleWrite("MPI StartUp Failure\n");
                dReturnStatus = E__BIT + wTestStatus;
			}
            
            sParameters.bInstance++;
        
       	}
		else if (bLoop == 0 || (bLoop == 1 && sParameters.bInstance == 1))
		{
			sprintf (achBuffer, "Unable to locate instance %d of MPT Fusion SCSI Device\n",
						sParameters.bInstance);
			vConsoleWrite (achBuffer);

			dReturnStatus = E__BIT + E_NO_DEVICE;
            
            /* Quit */
            bLoop = 0;

		}
       	else
       	{
       		/* Testing Multiple instances, just gone beyond last discovered instance */
            /* Decrement Instance number to last one Found */
            sParameters.bInstance--;

            /* Don't look any further */
            bLoop = 0;
            
            /* Were we expecting a specific number of instances? */
            if(fCheckInstances)
            {
                if(bExpectedInstances != sParameters.bInstance)
                {
                    sprintf (achBuffer, "FAIL Expected %d instances, Found %d instances\n",
						bExpectedInstances, sParameters.bInstance);
					vConsoleWrite (achBuffer);
                    
                    dReturnStatus = E__BIT + E_TEST_FAIL;
                }
                
            }
       	}
            

       	if(bINTsEnabled == 1)
       	{
       		/* Disable previously enabled interrupt handler */
       		vStopINTHandler();
       	}		
        
		

	} while (bLoop == 1 && dReturnStatus == E_OK);
	
	vFreeMemHandles();

	return(dReturnStatus);
	
}

#ifdef INCLUDE_LSI_MPT_PBSRAM_TEST
TEST_INTERFACE(wPciMPT_PBSRAM_Test, "LSI MPT PBSRAM Test")
{
	struct	{
		UINT8 bTestChoice;
		UINT8 bInstance;
		} sParameters;

	DEVICE	sPciDevice;
	const char	*pDescription;
	UINT16		wTestStatus;
	UINT32		dReturnStatus = E_OK;
	char achBuffer [80];
	UINT8 bLoop;
    UINT8 bExpectedInstances, fCheckInstances;

	sParameters.bTestChoice = 0;	/* default: select Test 0  */
	sParameters.bInstance   = 0;	/* default: Test All MPT Fusion SCSI controllers */

	bExpectedInstances = 0;			/* default: Don't expect specific no of controllers */
    fCheckInstances = FALSE;

	if(adTestParams[1])
	{
		sParameters.bTestChoice = adTestParams[2];

		if (adTestParams[1] == 2)
			sParameters.bInstance = adTestParams[3];

	}
	
	/* If 'test all' selected, set the loop flag and set instance to first */
	bLoop = 0;

	if (sParameters.bInstance == 0)
	{
		bLoop = 1;				
		sParameters.bInstance++;
	}
	
		/* Interrupt handlers disabled at start */
	bINTsEnabled = 0;
	wContext = 0;
	
	
	do
	{
		
       	/* check PCI bus for LSI10x0 SCSI controllers */
       	if (wFindMPTController (&sPciDevice.Pfa, &pDescription,
       				sParameters.bInstance) == E_OK)
       	{
       		vPrintDeviceDetails(sPciDevice.Pfa, pDescription);
       		/* Get Pointer to Register Structure in Memory Space */
       		
           /* Get Message Passing Interface Running, Device Enabled,
			   Ports Enabled, Ready to Handler SCSI Commands */			
			vInitController(&sPciDevice);

#if 0						
			wTestStatus = wMPIStartUp(&sPciDevice);
			
			if(wTestStatus != MPI_IOCSTATUS_SUCCESS)
			{
				
			    vConsoleWrite("MPI StartUp Failure\n");
                dReturnStatus = E_TEST_FAIL;
                return (dReturnStatus);
			}
#endif			
																			
			sPciDevice.wIoBaseAddress = wPciMPT_GetIoBaseAddress(sPciDevice.Pfa);
		 	
		 	wTestStatus = wPciMPT_EnableDiagnostic (sPciDevice.wIoBaseAddress);
		 	
		 	if (wTestStatus != E_OK)
		 	{			 
		 		dReturnStatus = E__BIT + wTestStatus;
			 	return (dReturnStatus);
		 	}
		 	vDelay(1);
		 	wTestStatus = wPCIMPT_PBSRAM_WalkingOne_Test(&sPciDevice);
		 	
		 	if (wTestStatus != E_OK)
		 	{			 	
		 		dReturnStatus = E__BIT + wTestStatus;
		        return (dReturnStatus);
		 	}
		 	vDelay(1);
		 	wTestStatus = wPCIMPT_PBSRAM_WalkingZero_Test(&sPciDevice);
		 	
		 	if (wTestStatus != E_OK)
		 	{			 	
		 		dReturnStatus = E__BIT + wTestStatus;
		        return (dReturnStatus);
		 	}
		 	vDelay(1);
		 	wTestStatus = wPCIMPT_PBSRAM_FixedPattern_Test(&sPciDevice);
		 	
		 	if (wTestStatus != E_OK)
		 	{			 	 			 
		 		dReturnStatus = E__BIT + wTestStatus;
		        return (dReturnStatus);
		 	}
		 	          
			sParameters.bInstance++;
       	}
		else if (bLoop == 0 || (bLoop == 1 && sParameters.bInstance == 1))
		{
			sprintf (achBuffer, "Unable to locate instance %d of MPT Fusion SCSI Device\n",
						sParameters.bInstance);
			vConsoleWrite (achBuffer);

			dReturnStatus = E__BIT + E_NO_DEVICE;
            
            /* Quit */
            bLoop = 0;

		}
       	else
       	{
       		/* Testing Multiple instances, just gone beyond last discovered instance */
            /* Decrement Instance number to last one Found */
            sParameters.bInstance--;

            /* Don't look any further */
            bLoop = 0;
            
            /* Were we expecting a specific number of instances? */
            if(fCheckInstances)
            {
                if(bExpectedInstances != sParameters.bInstance)
                {
                    sprintf (achBuffer, "FAIL Expected %d instances, Found %d instances\n",
						bExpectedInstances, sParameters.bInstance);
					vConsoleWrite (achBuffer);
                    
                    dReturnStatus = E__BIT + E_TEST_FAIL;
                }
                
            }
       	}       	  	
        
  	} while (bLoop == 1 && dReturnStatus == E_OK);
  	
  	return (dReturnStatus);
  		
}

static UINT16  wPciMPT_GetIoBaseAddress (const PCI_PFA Pfa)
{
	UINT16 wIoBaseAddress;
#ifdef DEBUG	
	vConsoleWrite ("Getting IO Base Address\n");
#endif	
	wIoBaseAddress = PCI_READ_WORD(Pfa, 0x10);
	wIoBaseAddress = wIoBaseAddress & 0xFFFE;
	
	return (wIoBaseAddress);
	
}

static UINT16 wPciMPT_EnableDiagnostic (UINT16 wIoBaseAddress)
{
	UINT16 wReturnStatus;
	UINT8 bDiagOffsetVal;
	char achBuffer [80];

#ifdef DEBUG		
	vConsoleWrite ("Enabling Diagnostic\n");
	
	sprintf (achBuffer,"wIoBaseAddress:%#x ",wIoBaseAddress);
	vConsoleWrite (achBuffer);
#endif	
	
	
	vIoWriteReg (wIoBaseAddress + MPI_WRITE_SEQUENCE_OFFSET, REG_8, 0xFF);
	
	vIoWriteReg (wIoBaseAddress + MPI_WRITE_SEQUENCE_OFFSET, REG_8, MPI_WRSEQ_1ST_KEY_VALUE);
	vIoWriteReg (wIoBaseAddress + MPI_WRITE_SEQUENCE_OFFSET, REG_8, MPI_WRSEQ_2ND_KEY_VALUE);
	vIoWriteReg (wIoBaseAddress + MPI_WRITE_SEQUENCE_OFFSET, REG_8, MPI_WRSEQ_3RD_KEY_VALUE);
	vIoWriteReg (wIoBaseAddress + MPI_WRITE_SEQUENCE_OFFSET, REG_8, MPI_WRSEQ_4TH_KEY_VALUE);
	vIoWriteReg (wIoBaseAddress + MPI_WRITE_SEQUENCE_OFFSET, REG_8, MPI_WRSEQ_5TH_KEY_VALUE);
	vIoWriteReg (wIoBaseAddress + MPI_DIAGNOSTIC_OFFSET, REG_8, 0xB0);
		
	bDiagOffsetVal = dIoReadReg (wIoBaseAddress + MPI_DIAGNOSTIC_OFFSET, REG_8);

#ifdef DEBUG		
	sprintf (achBuffer,"bDiagOffsetVal:%#x ",bDiagOffsetVal);
	vConsoleWrite (achBuffer);
#endif	
	
	if (bDiagOffsetVal & MPI_DIAG_DRWE)
	{
#ifdef DEBUG				
		vConsoleWrite ("Diagnostic Enable PASS\n");
#endif		
		wReturnStatus = E_OK;
	}
	else
	{
		vConsoleWrite ("Diagnostic Enable FAIL\n");
		wReturnStatus = E_TEST_FAIL;
	}
	
	return (wReturnStatus);
}

static UINT16 wPCIMPT_PBSRAM_WalkingOne_Test (DEVICE *psPciDevice)
{
   
   UINT32 dNvsRamAddr = 0;
   UINT32 dDataMask = 0x00000001;
   UINT32  dIndex = 0;
   UINT32 dValue = 0;
   char achBuffer [80];

   vConsoleWrite ("Running Walking Ones Test\n");
   
   dNvsRamAddr = 0;
   dIndex = 0;    
   while (dNvsRamAddr != NVSRAM_END_ADDRESS)
   {
        if (dDataMask == 0x00000000)
        {
           dDataMask = 0x00000001;
        }
          
        vPCIMPT_Diag_WriteDword (psPciDevice, NVSRAM_START_ADDRESS + dIndex, dDataMask);
        dDataMask = dDataMask << 1;         
        dIndex += 4;  
        dNvsRamAddr = NVSRAM_START_ADDRESS + dIndex;  
             
   }
  
   vDelay (1);
   dNvsRamAddr = 0;
   dIndex = 0;
   dDataMask = 0x00000001;
  
   while (dNvsRamAddr != NVSRAM_END_ADDRESS)
   {
	   if (dDataMask == 0x00000000)
       {
           dDataMask = 0x00000001;
       }
       vPCIMPT_Diag_ReadDword (psPciDevice, NVSRAM_START_ADDRESS + dIndex, &dValue);
       if (dValue != dDataMask)
       {
	       sprintf (achBuffer, "MEMORY MISCOMPARE @ 0x%08x, expected: 0x%08x but got 0x%08x\n", (NVSRAM_START_ADDRESS + dIndex), dDataMask, dValue);
	       vConsoleWrite (achBuffer);
	       return E_TEST_FAIL;
       }
	   dDataMask = dDataMask << 1;
	   dIndex += 4;          
	   dNvsRamAddr = NVSRAM_START_ADDRESS + dIndex;   
       
   }
   
   return (E_OK);
}

static UINT16 wPCIMPT_PBSRAM_WalkingZero_Test (DEVICE *psPciDevice)
{
	
   UINT32 dNvsRamAddr = 0;
   UINT32 dDataMask = 0xFFFFFFFE;
   UINT32  dIndex = 0;
   UINT32 dValue = 0;
   char achBuffer [80];
   
   vConsoleWrite ("Running Walking Zeros Test\n");
   
   dNvsRamAddr = 0;
   dIndex = 0;
   while (dNvsRamAddr != NVSRAM_END_ADDRESS)
   {
        if (dDataMask == 0x00000000)
        {
           dDataMask = 0xFFFFFFFE;
        }
          
        vPCIMPT_Diag_WriteDword (psPciDevice, NVSRAM_START_ADDRESS + dIndex, dDataMask);
        dDataMask = dDataMask << 1;
        dIndex += 4; 
        dNvsRamAddr = NVSRAM_START_ADDRESS + dIndex;        
   }
  
   vDelay (1);
   dNvsRamAddr = 0;
   dIndex = 0;
   dDataMask = 0xFFFFFFFE;
  
   while (dNvsRamAddr != NVSRAM_END_ADDRESS)
   {
	   if (dDataMask == 0x00000000)
       {
           dDataMask = 0xFFFFFFFE;
       }
       vPCIMPT_Diag_ReadDword (psPciDevice, NVSRAM_START_ADDRESS + dIndex, &dValue);
       if (dValue != dDataMask)
       {
	       sprintf (achBuffer, "MEMORY MISCOMPARE @ 0x%08x, expected: 0x%08x but got 0x%08x\n", (NVSRAM_START_ADDRESS + dIndex), dDataMask, dValue);
	       vConsoleWrite (achBuffer);
	       return E_TEST_FAIL;
       }
	   dDataMask = dDataMask << 1;
	   dIndex += 4;   
	   dNvsRamAddr = NVSRAM_START_ADDRESS + dIndex;                
   }
	
   return E_OK;
}


static UINT16 wPCIMPT_PBSRAM_FixedPattern_Test (DEVICE *psPciDevice)
{
	const UINT32 bPat_55AA = 0x55AA55AA;
	const UINT32 bPat_AA55 = 0xAA55AA55;
	UINT32 dIndex = 0;
	UINT32 dValue = 0;
	UINT16 wTestStatus;
	char achBuffer [80];
	
    vConsoleWrite ("Running Fixed Pattern Test\n");
    
    
	for ( dIndex = NVSRAM_START_ADDRESS ; dIndex < NVSRAM_END_ADDRESS; dIndex+=4)
	{
		vPCIMPT_Diag_WriteDword (psPciDevice, dIndex,bPat_55AA);
	}	
	
	vDelay (1);
	for ( dIndex = NVSRAM_START_ADDRESS ; dIndex < NVSRAM_END_ADDRESS; dIndex+=4)
	{
		vPCIMPT_Diag_ReadDword (psPciDevice, dIndex,&dValue);
		if (dValue != bPat_55AA)
		{
		    sprintf (achBuffer, "MEMORY MISCOMPARE @ 0x%08x, expected: 0x%08x but got 0x%08x\n", (dIndex), bPat_55AA, dValue);
	        vConsoleWrite (achBuffer);
		    wTestStatus = E_TEST_FAIL;
		    return  (wTestStatus);
        }        
	}		
	
	for ( dIndex = NVSRAM_START_ADDRESS ; dIndex < NVSRAM_END_ADDRESS; dIndex+=4)
	{
		vPCIMPT_Diag_WriteDword (psPciDevice, dIndex,bPat_AA55);
	}	
	
	vDelay (1);
	for ( dIndex = NVSRAM_START_ADDRESS ; dIndex < NVSRAM_END_ADDRESS; dIndex+=4)
	{
		vPCIMPT_Diag_ReadDword (psPciDevice, dIndex,&dValue);
		if (dValue != bPat_AA55)
		{
			sprintf (achBuffer, "MEMORY MISCOMPARE @ 0x%08x, expected: 0x%08x but got 0x%08x\n", (dIndex), bPat_AA55, dValue);
	        vConsoleWrite (achBuffer);
		    wTestStatus = E_TEST_FAIL;
		    return  (wTestStatus);
        }        
	}
	
	wTestStatus = E_OK;
	
	return (wTestStatus);
}

/*
 * Common Routines to Accessing the Memory of PBSRAM testing functions
 */
 
 static void vPCIMPT_Diag_WriteDword (DEVICE *psPciDevice, UINT32 dInternalAddress, UINT32 dValue)
 {	  
	  vPCIMPT_Diag_IoWriteInternalRegister32 (psPciDevice->wIoBaseAddress, dInternalAddress, dValue);
 }
 
 
 static void vPCIMPT_Diag_ReadDword (DEVICE *psPciDevice, UINT32 dAddress, UINT32 *pdValue)
 {
	 
	  *pdValue =  dPCIMPT_Diag_IoReadInternalRegister32 (psPciDevice->wIoBaseAddress, dAddress);
       
       return;
 }
 
 
 static void vPCIMPT_Diag_IoWriteInternalRegister32 (UINT16 wHostRegIoBAR, UINT32 dAddress, UINT32 dValue)
 {
	 
	   if ( FALSE == wPCIMPT_Diag_CheckMode_EnabledIo(wHostRegIoBAR) )
	   {
		     wPciMPT_EnableDiagnostic (wHostRegIoBAR);
	   }
	   	 
	   /* Write Address to the Address Reg Offset */
	   vPCIMPT_Diag_Write_To_Hardware (wHostRegIoBAR + MPI_DIAG_RW_ADDRESS_OFFSET, (dAddress & 0xfffffffc));
	   /* Write Data to the Data Reg offset */
	   vPCIMPT_Diag_Write_To_Hardware (wHostRegIoBAR + MPI_DIAG_RW_DATA_OFFSET, dValue);
 }
 
 static UINT32 dPCIMPT_Diag_IoReadInternalRegister32 (UINT16 wHostRegIoBAR, UINT32 dAddress)
 {
	 UINT16 dValue = 0;
	 
	 if ( FALSE == wPCIMPT_Diag_CheckMode_EnabledIo(wHostRegIoBAR) )
	 {
	       wPciMPT_EnableDiagnostic (wHostRegIoBAR);
	 }
	 	
	 /* Write The internal address */
     vPCIMPT_Diag_Write_To_Hardware (wHostRegIoBAR + MPI_DIAG_RW_ADDRESS_OFFSET, (dAddress & 0xfffffffc));
     /* Read the dword from the memory */
     dValue = dPCIMPT_Diag_Read_From_Hardware (wHostRegIoBAR + MPI_DIAG_RW_DATA_OFFSET);
     
     return (dValue);
 }
 
 static UINT16 wPCIMPT_Diag_CheckMode_EnabledIo (UINT16 wHostRegIoBAR)
 {
	 UINT32 dValue = 0;
	 
	 dValue = dPCIMPT_Diag_Read_From_Hardware ( wHostRegIoBAR + MPI_DIAGNOSTIC_OFFSET);
	 
	 if ( dValue && (MPI_DIAG_MEM_ENABLE|MPI_DIAG_RW_ENABLE) )
	 {
		 return TRUE;
	 }
	 else
	 {
		 return FALSE;
	 }
	 
 }
 
 static void vPCIMPT_Diag_Write_To_Hardware (UINT16 wAddress, UINT32 dValue)
 {
	 vIoWriteReg (wAddress, REG_32, dValue);
 }
 
 
 static UINT32 dPCIMPT_Diag_Read_From_Hardware (UINT16 wAddress)
 {
	 volatile UINT32 dValue = 0;
	 
	 dValue = dIoReadReg (wAddress, REG_32);
	 
	 return (dValue);	 	 
 }
 
#endif
