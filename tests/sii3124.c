/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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

/* sii3124.c - SII3124 PCI-X SATA Controller test, which tests the disk interface 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/sii3124.c,v 1.2 2013-10-08 07:13:39 chippisley Exp $
 *
 * $Log: sii3124.c,v $
 * Revision 1.2  2013-10-08 07:13:39  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.3  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.2  2009/06/19 13:45:16  jthiru
 * Added check for port number passed by user
 *
 * Revision 1.1  2009/06/05 14:31:48  jthiru
 * Initial check in for SII3124 test module
 *
 *
 */

/* includes */

#include <stdtypes.h>
#include <errors.h>		
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <devices/sii3124.h>
#include <bit/hal.h>
 
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>

/* defines */

/* #define DEBUG */
#ifdef DEBUG
		static char buffer[128];
#endif

#define MAX_PORTS 4


/*
 * Register access macros
 */
#define vWriteHostRegSil24(reg, value)	(*(UINT32*)(psDev->pbHostReg + reg) = value)
#define dReadHostRegSil24(reg)			(*(UINT32*)(psDev->pbHostReg + reg))

#define vWritePortRegSil24(reg, value)	(*(UINT32*)(psDev->pbPortReg + reg) = value)
#define dReadPortRegSil24(reg)			(*(UINT32*)(psDev->pbPortReg + reg))

#define MAX_DRIVE_NUMBER	1
#define MAX_HEAD_NUMBER		15
#define MAX_CYLINDER_NUMBER	1023
#define MAX_SECTOR_NUMBER	255

#define IDE_IRQ			14

#define TYPE_IDE			1
#define TYPE_SATA			2

/* typedefs */
struct s_Params             /* command line parameters */
{
	UINT8   bTest;          /* test number */
	UINT8	bController;	/* controller instance */
	UINT8	bChannel;		/* which IDE channel */
	UINT8   bDrive; 	    /* sub test parameter */
	UINT16	wCylinder;		/* cylinder number for read test */
	UINT8	bHead;			/* head number for read test */
	UINT8	bSector;		/* sector number for read test */
} ;

typedef struct {
	UINT16	wCmdReg;
	UINT16	wCtrlReg;
	UINT8   bFlags; 
	UINT8	bIrq;
} PORTINFO;

typedef struct {
	UINT8 	bBus;			/* PCI bus, device and 		*/
	UINT8 	bDev;			/* function numbers of the	*/
	UINT8 	bFunc;			/* IDE controller			*/
	UINT8 	bCtrlType;		/* IDE or SATA controller	*/
	UINT16	wBMIdeRegs;		/* Bus Master IDE regs		*/
	UINT32   bNumPorts;
	UINT8  *pbHostReg;
	UINT8  *pbPortReg;
	PORTINFO sPortInfo[4];
} DEVICE;

/* constants */
struct {
	UINT16 bPort;
	UINT32 dData;
} const sSilTestData [] = {
	{PORT_FIFO_THRES,  0x00000058}, 
	{PORT_SCONTROL, 0x000000AA}, 
	{PORT_PHY_CFG,  0x0000000f},
	{0,0}
};					/* register test data		*/

/* globals */
static PTR48	p1, p2, p3, p4;
static UINT32	dHandle1, dHandle2, dHandle3, dHandle4;

/* externals */
extern void vDelay(UINT32 dMsDelay);
extern UINT32 dGetPhysPtr (UINT32 dBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);
extern void vConsoleWrite(char*	achMessage);
extern void vFreePtr(UINT32 dHandle);


/*****************************************************************************
 * Sil24WaitRegister: This function waits for the register to be writen
 *
 * RETURNS: Read register value.
 */

static UINT32 Sil24WaitRegister 
(
	DEVICE *psDev,
	volatile UINT32 dReg, 
	UINT32 dMask, 
	UINT32 dVal,
    UINT32 dInterval, 
    UINT32  dTimeOut 
)
{
    UINT32 dTmp;
    
    dTmp = dReadPortRegSil24(dReg);
    /*dTimeOut *= 10;*/
    while ((dTmp & dMask) == dVal && dTimeOut) 
    {
        vDelay(dInterval);
	    dTimeOut--;
        dTmp = dReadPortRegSil24(dReg);
    }
    return dTmp;
}


/*****************************************************************************
 * sil24ConfigPort: This function configures the port
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 sil24ConfigPort 
(
	DEVICE *psDev,
	UINT32 bPortNum
)
{

	/* configure IRQ WoC */
	if (psDev->sPortInfo[bPortNum].bFlags & SIL24_FLAG_PCIX_IRQ_WOC)
		vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_CTRL_STAT,(UINT32)PORT_CS_IRQ_WOC);
	else
		vWritePortRegSil24((bPortNum*PORT_REGS_SIZE)+ PORT_CTRL_CLR,(UINT32)PORT_CS_IRQ_WOC);

	/* zero error counters.*/
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_DECODE_ERR_THRESH,0x8000);
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_CRC_ERR_THRESH,0x8000);
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_HSHK_ERR_THRESH,0x8000);
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_DECODE_ERR_CNT,0x8000);
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_CRC_ERR_CNT,0x8000);
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_HSHK_ERR_CNT,0x8000);

	/* always use 64bit activation */
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_CTRL_CLR,(UINT32)PORT_CS_32BIT_ACTV);

	/* clear port multiplier enable and resume bits */
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_CTRL_CLR,(UINT32)(PORT_CS_PMP_EN | PORT_CS_PMP_RESUME));
	return E__OK;
}


/*****************************************************************************
 * sil24PortInitial: This function leaves the port in initial state
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 sil24PortInitial 
(
	DEVICE *psDev,
	UINT32 bPortNum
)
{
	UINT32 dTemp = 0; 
	
   	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE) + PORT_CTRL_STAT,(UINT32)PORT_CS_INIT);
	Sil24WaitRegister(psDev,(bPortNum*PORT_REGS_SIZE) + PORT_CTRL_STAT,
			  PORT_CS_INIT, PORT_CS_INIT, 10, 100);
	dTemp = Sil24WaitRegister(psDev,(bPortNum*PORT_REGS_SIZE) + PORT_CTRL_STAT,
				PORT_CS_RDY, 0, 10, 100);

	if ((dTemp & (UINT32)(PORT_CS_INIT | PORT_CS_RDY)) != (UINT32)PORT_CS_RDY) 
	{
	#ifdef DEBUG
		sprintf(buffer,"PORT_CTRL_STAT %#x\n",(int)dTemp);
		vConsoleWrite(buffer);
	#endif
        return E__INIT1_PORT;
	} 
	return E__OK;
}


/*****************************************************************************
 * sil24SoftReset: This function soft resets the port
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 sil24SoftReset
(
	DEVICE *psDev,
	UINT32 bPortNum
)
{
	volatile UINT16 dTemp;
	struct sil24Prb prb;
	UINT16 irq_enabled = 0;
	UINT16 irq_mask = 0;
	UINT16 irq_stat = 0;

	/* put the port into known state */
	if (sil24PortInitial(psDev,bPortNum))
	{
#ifdef DEBUG
			vConsoleWrite( "port not ready");
#endif
		return E__SOFT_RST;
	}

	memset(&prb,  0 , sizeof(struct sil24Prb));
	/*0x80 indicates to Sil3124 to send s soft reset sequence to the device.*/
	prb.wCtrl = 0x80;
	/* Register - Host to Device FIS */
	prb.bFis[0] = 0x27;

	/* temporarily plug completion and error interrupts */
	dTemp = dReadPortRegSil24((bPortNum*PORT_REGS_SIZE)+PORT_IRQ_ENABLE_SET);  
	irq_enabled = dTemp;

	/*memcpy the contents of the PRB into the RAM slot - Direct Command issuance method.*/
	memcpy((UINT32*)(psDev->pbPortReg)+(bPortNum*PORT_REGS_SIZE),(UINT32*)&prb, sizeof(struct sil24Prb));
	 
	/*Write the slot number into which the PRB contents have been written - as part of Direct Command issuance method.*/
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE)+PORT_EXEC_FIFO,0x0);

	irq_mask = (UINT16) ((PORT_IRQ_COMPLETE | PORT_IRQ_ERROR) << PORT_IRQ_RAW_SHIFT);
	irq_stat = 0;
	irq_stat = Sil24WaitRegister(psDev,(bPortNum*PORT_REGS_SIZE)+PORT_IRQ_STAT, irq_mask, 0x0, 10, 1000);

	/* clear IRQs */
	vWritePortRegSil24 ((bPortNum*PORT_REGS_SIZE)+PORT_IRQ_STAT,irq_stat);
	irq_stat >>= PORT_IRQ_RAW_SHIFT;

	/* restore IRQ enabled */
	vWritePortRegSil24 ((bPortNum*PORT_REGS_SIZE)+ PORT_IRQ_ENABLE_SET,irq_enabled);

	if ( !(irq_stat & PORT_IRQ_COMPLETE) )
	{
		/* force port into known state */
		sil24PortInitial(psDev,bPortNum);
		if (irq_stat & PORT_IRQ_ERROR)
		{
#ifdef DEBUG
			vConsoleWrite("SRST command error");
#endif
			return E__SOFT_RST;
		}
		else
		{
#ifdef DEBUG
			vConsoleWrite("timeout");
#endif
		return E__SOFT_RST;
		}
	}
	return E__OK; 
}


/*****************************************************************************
 * sil24InitPort: This function initializes the port
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 sil24InitPort 
(
	DEVICE *psDev,
	UINT32 bPortNum
)
{
	UINT16 dTemp; 

	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE)+PORT_IRQ_ENABLE_SET,(UINT32)0x03);
	
	vWritePortRegSil24((bPortNum*PORT_REGS_SIZE)+PORT_CTRL_STAT,(UINT32)PORT_CS_INIT);
	Sil24WaitRegister(psDev,(bPortNum*PORT_REGS_SIZE)+PORT_CTRL_STAT,
			  PORT_CS_INIT, PORT_CS_INIT, 10, 100);
	dTemp = Sil24WaitRegister(psDev,(bPortNum*PORT_REGS_SIZE)+PORT_CTRL_STAT,
				PORT_CS_RDY, 0, 10, 100);

	if ((dTemp & (PORT_CS_INIT | PORT_CS_RDY)) != PORT_CS_RDY) 
	{
        return E__INIT_PORT;
	}
	else
	{ 
		dTemp = dReadPortRegSil24((bPortNum*PORT_REGS_SIZE)+PORT_SSTATUS);  
		if ((dTemp & 0x03) == 0x03)
		{
           if(sil24SoftReset(psDev,bPortNum) == -1)
           {
              return E__INIT_PORT;
           }
           else
           {
              return E__OK;
           }
        }
        else
        {
          return E__INIT_PORT;
        }
	}
	return 0;
}


/*****************************************************************************
 * dSil24Init: This function initializes and configures port and PHY
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 dSil24Init 
(
	DEVICE *psDev
)
{
	UINT32 dTmp;
	UINT32 i = 0;
 
	
    vWriteHostRegSil24(HOST_CTRL, 0x0);
	/* init ports */
	for (i = 0; i < psDev->bNumPorts; i++) 
	{	
		/* Initial PHY setting */
		vWritePortRegSil24 ((i * PORT_REGS_SIZE) + (PORT_PHY_CFG), 0x20C);
		/* Clear port RST */
		dTmp = dReadPortRegSil24((i * PORT_REGS_SIZE) + PORT_CTRL_STAT);
				
		if (dTmp & PORT_CS_PORT_RST) 
		{
			vWritePortRegSil24 ((i*PORT_REGS_SIZE)+(PORT_CTRL_CLR),(UINT32)PORT_CS_PORT_RST);
			dTmp = Sil24WaitRegister (psDev,(i*PORT_REGS_SIZE)+ PORT_CTRL_STAT,
						PORT_CS_PORT_RST,
						PORT_CS_PORT_RST, 10, 100);
			if (dTmp & PORT_CS_PORT_RST)
				return E__PORT_RESET;
			#ifdef DEBUG
				vConsoleWrite("failed to clear port RST\n");
			#endif
		}
        /* configure port */ 
        sil24InitPort(psDev,i);
   	    sil24ConfigPort(psDev,i);
	} 
	return E__OK;
} 


/*****************************************************************************
 * dTestInit: this function detects a PCI IDE controller and performs basic
 * initialization.
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 dTestInit 
( 
	DEVICE *psDev, 
	UINT8 bInstance 
)
{
	UINT8	bTemp;
	UINT32	dTemp;
	UINT16	wTemp;
    UINT32	dFound;
	// UINT32	dPciDevId;
	PCI_PFA pfa;

   pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
   /* find the IDE controller by class code */
	dFound = iPciFindDeviceByClass (bInstance, 0x01, 0x80, &pfa);
    
    if (dFound == E__DEVICE_NOT_FOUND)
	{
		return dFound;
	}
	
	psDev->bBus = PCI_PFA_BUS(pfa);
	psDev->bDev = PCI_PFA_DEV(pfa);
	psDev->bFunc = PCI_PFA_FUNC(pfa);
#ifdef DEBUG
	sprintf(buffer, "B: %d, D: %d, F: %d\n", PCI_PFA_BUS(pfa), PCI_PFA_DEV(pfa), PCI_PFA_FUNC(pfa));
	vConsoleWrite(buffer);
#endif

	psDev->bCtrlType = TYPE_SATA;
	psDev->wBMIdeRegs = 0;
	

	// dPciDevId = dPciReadReg (pfa, 0x00, REG_32);

  /* if this is a serial ATA controller, make sure the ports are enabled */

  /* report what was found */
#ifdef DEBUG
    sprintf (buffer, "Testing %s controller @ bus:%02X, dev:%02X, func:%02X\n",
					psDev->bCtrlType == TYPE_IDE ? "IDE" : "SATA", 
					psDev->bBus, psDev->bDev, psDev->bFunc);
	vConsoleWrite(buffer);
#endif
  /* make sure the device IO access is enabled */
    wTemp = (UINT16)dPciReadReg (pfa, PCI_COMMAND, REG_16);
	vPciWriteReg (pfa, PCI_COMMAND, REG_16, wTemp | PCI_MEMEN | PCI_BMEN);
						
	/* Set Cache Line Size register (nominal value is 32)
     */										
	vPciWriteReg (pfa, PCI_CACHE_LINE_SIZE, REG_8, 32);

	/* Create a pointer to the memory-mapped registers */
	dTemp  = dPciReadReg (pfa, 0x10, REG_32);
	dTemp &= PCI_MEM_BASE_MASK;
	dHandle1 = dGetPhysPtr (dTemp, 0x00001000, &p1, (void*)&psDev->pbHostReg);
	if(dHandle1 == E__FAIL)
	{
	#ifdef DEBUG
		vConsoleWrite("Unable to allocate PCI ROM \n"); 
	#endif
		return(E__TEST_NO_MEM);
	}

	/* Create a pointer to the memory-mapped registers */
	dTemp  = dPciReadReg (pfa, 0x18, REG_32);
	dTemp &= PCI_MEM_BASE_MASK;
	dHandle2 = dGetPhysPtr (dTemp, 0x00008000, &p2, (void*)&psDev->pbPortReg);
	if(dHandle2 == E__FAIL)
	{
	#ifdef DEBUG
		vConsoleWrite("Unable to allocate PCI ROM \n"); 
	#endif
		return(E__TEST_NO_MEM);
	}
	
  	/* enable master and secondary controller */
    bTemp= (UINT8)dPciReadReg (pfa, 0x41, REG_8);
    vPciWriteReg (pfa, 0x41, REG_8, bTemp | 0x80);
    bTemp= (UINT8)dPciReadReg (pfa, 0x43, REG_8); 
    vPciWriteReg (pfa, 0x43, REG_8, bTemp | 0x80);
    psDev->wBMIdeRegs = (UINT16)dPciReadReg (pfa, 0x20, REG_16) & 0xfffc;

    psDev->bNumPorts = MAX_PORTS;
    dTemp = dSil24Init (psDev);
	if(dTemp != E__OK)
		return dTemp;
	else
		return (E__OK);

} /* dTestInit () */


/*****************************************************************************
 * dTestRegs: test function for testing the registers of IDE/ SATA controller
 *
 *   Write and read test patterns to the cylinder and sector registers.
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 dTestRegs ( DEVICE *psDev, UINT8 bDrive )
{
	UINT32 dTestStatus = E__OK;
	UINT8 	bIndex = 0;
	UINT32 	dData;


	while (sSilTestData[bIndex].bPort != 0)
	{
		vWritePortRegSil24 ((bDrive*PORT_REGS_SIZE)+ sSilTestData[bIndex].bPort,(UINT32)sSilTestData[bIndex].dData);
		bIndex++;
	}
	
	bIndex = 0;

	while (sSilTestData[bIndex].bPort != 0)
	{
		dData = dReadPortRegSil24 ((bDrive*PORT_REGS_SIZE)+sSilTestData[bIndex].bPort);
        
		if (dData != sSilTestData[bIndex].dData)
		{
			dTestStatus = E__REGS_ERROR;
#ifdef DEBUG
			sprintf (buffer, "Reg : %x. Read : %x, Wrote %x\n",
						(int)(sSilTestData[bIndex].bPort),
						(int)dData,
						(int)sSilTestData[bIndex].dData);
			vConsoleWrite(buffer);
#endif
			break;
		}
		bIndex++;
    }
    return dTestStatus;
} /* dTestRegs () */


/*****************************************************************************
 * dIdentifyDrive: test function for interrupt generation verification by sending
 *					a drive ID command to the disk
 * 
 *
 * RETURNS: Test passed E_OK or error code.
 */
 
static UINT32 dIdentifyDrive 
(
	DEVICE *psDev, 
	UINT8 bDrive
)
{
	UINT32 	dTestStatus = E__OK;
	struct sil24AtaBlock *pAtaCmd;
	char *reason;
    UINT32 dIrqEnabled = 0;
    UINT32 dIrqMask = 0;
    UINT32 dIrqStat = 0;
    UINT32 dCmdLow = 0;
    UINT32 dCmdHigh = 0;
	UINT16	wCount;			/* word transfer count 		*/
	UINT8   bCmd;
	UINT8   bHead;
	UINT32   dCyl;
	UINT16  wSect;
	UINT16  wFeat;
	
	dIrqEnabled = dReadPortRegSil24 ((bDrive*PORT_REGS_SIZE) + PORT_IRQ_ENABLE_SET);
	
    /* put the port into known state */
	dTestStatus = sil24PortInitial(psDev,bDrive);
    if (dTestStatus)
    { 
	    reason = "port not ready\n";
#ifdef DEBUG
       vConsoleWrite(reason);
#endif
  	   return dTestStatus;
    }
    	
	dHandle3 = dGetPhysPtr (ATACMD_ADDRESS, 0x00000800, &p3, (void*)&pAtaCmd);
	if(dHandle3 == E__FAIL)
	{
	#ifdef DEBUG
		vConsoleWrite("Unable to allocate PCI ROM \n"); 
	#endif
		return(E__TEST_NO_MEM);
	}
	
    bCmd = HDC_CMD_IDENT;
    bHead = 0;
    dCyl = 0;
    wSect = 0;
    wFeat = 0;
    wCount = 1;
    
    pAtaCmd->prb.bFis[0] = 0x27; /*Register - Host to Device FIS.*/
    pAtaCmd->prb.bFis[1] = 0x80;
    pAtaCmd->prb.bFis[2] = bCmd;  
    pAtaCmd->prb.bFis[3] = wFeat;
    pAtaCmd->prb.bFis[4] = wSect;
    pAtaCmd->prb.bFis[5] = dCyl;
    pAtaCmd->prb.bFis[6] = (dCyl >> 8);
    pAtaCmd->prb.bFis[7] = bHead;
    pAtaCmd->prb.bFis[8] = (wSect >> 8);
    pAtaCmd->prb.bFis[9] = (dCyl >> 16);
    pAtaCmd->prb.bFis[10] = (dCyl >> 24);
    pAtaCmd->prb.bFis[11] = (wFeat >> 8);
    pAtaCmd->prb.bFis[12] = wCount;
    pAtaCmd->prb.bFis[13] = (wCount >> 8);
    pAtaCmd->prb.bFis[14] = 0;
    pAtaCmd->prb.bFis[15] = 0;
    pAtaCmd->prb.bFis[16] = 0;
    pAtaCmd->prb.bFis[17] = 0;
    pAtaCmd->prb.bFis[18] = 0;
    pAtaCmd->prb.bFis[19] = 0;
    pAtaCmd->prb.bFis[20] = 0;
    pAtaCmd->prb.bFis[21] = 0;
    pAtaCmd->prb.bFis[22] = 0;
    pAtaCmd->prb.bFis[23] = 0;
    pAtaCmd->sge[0].dLowAddr = (UINT32)BUFFER_ADDRESS;
    pAtaCmd->sge[0].dHighAddr =  (UINT32) 0x00000000;
    pAtaCmd->sge[0].dCnt = 512;
    pAtaCmd->sge[0].dFlags = 0x80000000;
    
    dCmdLow = (UINT32)ATACMD_ADDRESS;
    dCmdHigh = (UINT32)0x00000000;
    
    vWritePortRegSil24 ((bDrive*PORT_REGS_SIZE)+ PORT_CMD_ACTIVATE,dCmdLow);
    vWritePortRegSil24 ((bDrive*PORT_REGS_SIZE)+ PORT_CMD_ACTIVATE+0x4,dCmdHigh);

    dIrqMask = (UINT32)((PORT_IRQ_COMPLETE | PORT_IRQ_ERROR) << PORT_IRQ_RAW_SHIFT);
    /*x = (uint32_t *) (hba->cfg.MemBase[0] + PORT_IRQ_STAT);*/
    dIrqStat = 0;
    dIrqStat = Sil24WaitRegister(psDev,(bDrive*PORT_REGS_SIZE)+PORT_IRQ_STAT, dIrqMask, 0x0, 10, 1000);
    
    vWritePortRegSil24((bDrive*PORT_REGS_SIZE)+PORT_IRQ_STAT,dIrqMask);
    
    dIrqStat >>= PORT_IRQ_RAW_SHIFT;

    /* restore IRQ enabled */
    vWritePortRegSil24((bDrive*PORT_REGS_SIZE)+PORT_IRQ_ENABLE_SET,dIrqEnabled);
    
#ifdef DEBUG    
    sprintf(buffer, "dIrqMask %#x\n",(int)dIrqMask);
	vConsoleWrite(buffer);
    sprintf(buffer, "dIrqStat %#x\n",(int)dIrqStat);
	vConsoleWrite(buffer);
    sprintf(buffer, "dIrqEnabled %#x\n",(int)dIrqEnabled);
	vConsoleWrite(buffer);
#endif	
    
    if ( !(dIrqStat & PORT_IRQ_COMPLETE) )
    {
      if (dIrqStat & PORT_IRQ_ERROR)
      {
         reason = "SRST command error\n";
#ifdef DEBUG
		 vConsoleWrite(reason);
#endif
         dTestStatus = E__IRQ_ERROR;
         return dTestStatus;
      }
      else
      {
         reason = "timeout";
         dTestStatus = E__IRQ_ERROR;
         vConsoleWrite(reason);
         return dTestStatus;
      }
    }
	return dTestStatus;

} /* dIdentifyDrive () */


/*****************************************************************************
 * wReadSector: Test function for reading a sector from IDE/ SATA 
 * 
 *
 * RETURNS: Test passed E_OK or error code.
 */
 
static UINT32 dReadSector 
(
	DEVICE *psDev, 
	UINT8 bDrive
)
{
	UINT32 	dTestStatus = E__OK;
	UINT16	wCount;			/* word transfer count 		*/
	UINT8	bCmd;			/* ide command register		*/
	UINT32 dIrqEnabled = 0;
    UINT32 dIrqMask = 0;
    UINT32 dIrqStat = 0;
    UINT32 dCmdLow = 0;
    UINT32 dCmdHigh = 0;
	UINT8   bHead;
	UINT32   dCyl;
	UINT16  wSect;
	UINT16  wFeat;
	struct sil24AtaBlock *pAtaCmd;
    UINT32 uiSlotNum;
    UINT32 dTemp;
	
	dHandle4 = dGetPhysPtr (ATACMD_ADDRESS, 0x00000800, &p4, (void*)&pAtaCmd);
	if(dHandle4 == E__FAIL)
	{
#ifdef DEBUG
		vConsoleWrite("Unable to allocate PCI ROM \n"); 
#endif
		return(E__TEST_NO_MEM);
	}
	
	/* Reset Signature memory location */
	*(UINT16*)(BUFFER_ADDRESS + 0x1FE) = 0;

	bHead = 0xE0;
    bCmd = HDC_CMD_READ;
    dCyl = (UINT32)0;
    wSect = (UINT16)0;
    wFeat = 0;
    wCount = 1;
    
    pAtaCmd->prb.bFis[0] = 0x27; /*Register - Host to Device FIS.*/
    pAtaCmd->prb.bFis[1] = 0x80;
    pAtaCmd->prb.bFis[2] = bCmd;  
    pAtaCmd->prb.bFis[3] = wFeat;
    pAtaCmd->prb.bFis[4] = wSect;
    pAtaCmd->prb.bFis[5] = dCyl;
    pAtaCmd->prb.bFis[6] = (dCyl >> 8);
    pAtaCmd->prb.bFis[7] = bHead;
    pAtaCmd->prb.bFis[8] = (wSect >> 8);
    pAtaCmd->prb.bFis[9] = (dCyl >> 16);
    pAtaCmd->prb.bFis[10] = (dCyl >> 24);
    pAtaCmd->prb.bFis[11] = (wFeat >> 8);
    pAtaCmd->prb.bFis[12] = wCount;
    pAtaCmd->prb.bFis[13] = (wCount >> 8);
    pAtaCmd->prb.bFis[14] = 0;
    pAtaCmd->prb.bFis[15] = 0;
    pAtaCmd->prb.bFis[16] = 0;
    pAtaCmd->prb.bFis[17] = 0;
    pAtaCmd->prb.bFis[18] = 0;
    pAtaCmd->prb.bFis[19] = 0;
    pAtaCmd->prb.bFis[20] = 0;
    pAtaCmd->prb.bFis[21] = 0;
    pAtaCmd->prb.bFis[22] = 0;
    pAtaCmd->prb.bFis[23] = 0;
    pAtaCmd->sge[0].dLowAddr = (UINT32)BUFFER_ADDRESS;
    pAtaCmd->sge[0].dHighAddr =  (UINT32) 0x00000000;
    pAtaCmd->sge[0].dCnt = 512;
    pAtaCmd->sge[0].dFlags = 0x80000000;
    
    
    uiSlotNum = 0;/*Start with the command slot zero.*/
    dTemp = dReadPortRegSil24(PORT_SLOT_STAT);
    while((dTemp & (1<<uiSlotNum)) != 0)
    {
       uiSlotNum++;
       dTemp = dReadPortRegSil24(PORT_SLOT_STAT);
       if (uiSlotNum == 31) /*only 31 slots are available - 0 to 31.*/
       {
          break;
       }
    }

    if(uiSlotNum == 31) /*only 31 slots are available - 0 to 31.*/
    {
#ifdef DEBUG
        sprintf(buffer,"SLOT_STSTUS=%x slotnum=%d\n", dTemp, uiSlotNum);
        vConsoleWrite(buffer);
        vConsoleWrite("Command Slot Not Available\n");
#endif
        dTestStatus = E__READ_ERROR;
        return dTestStatus;
    }
     
    dCmdLow =(UINT32)ATACMD_ADDRESS;
    dCmdHigh = (UINT32)0x00000000;

    vWritePortRegSil24 ((bDrive*PORT_REGS_SIZE)+ PORT_CMD_ACTIVATE + uiSlotNum*8,dCmdLow);
    vWritePortRegSil24 ((bDrive*PORT_REGS_SIZE)+ PORT_CMD_ACTIVATE+ uiSlotNum*8+0x4,dCmdHigh);

    dIrqMask = (UINT32)((PORT_IRQ_COMPLETE | PORT_IRQ_ERROR) << PORT_IRQ_RAW_SHIFT);
    dIrqStat = 0;
    dIrqStat = Sil24WaitRegister(psDev,(bDrive*PORT_REGS_SIZE)+PORT_IRQ_STAT, dIrqMask, 0x0, 10, 1000);
    
    vWritePortRegSil24((bDrive*PORT_REGS_SIZE)+PORT_IRQ_STAT,dIrqMask);
    
    dIrqStat >>= PORT_IRQ_RAW_SHIFT;

    /* restore IRQ enabled */
    vWritePortRegSil24((bDrive*PORT_REGS_SIZE)+PORT_IRQ_ENABLE_SET,dIrqEnabled);
    
#ifdef DEBUG    
    sprintf(buffer,"dIrqMask %#x\n",(int)dIrqMask);
	vConsoleWrite(buffer);
    sprintf(buffer,"dIrqStat %#x\n",(int)dIrqStat);
	vConsoleWrite(buffer);
    sprintf(buffer,"dIrqEnabled %#x\n",(int)dIrqEnabled);
	vConsoleWrite(buffer);
#endif	
    
    if ( !(dIrqStat & PORT_IRQ_COMPLETE) )
    {
      if (dIrqStat & PORT_IRQ_ERROR)
      {
#ifdef DEBUG
		 vConsoleWrite("SRST command error\n");
#endif
         dTestStatus = E__READ_ERROR; 
         return dTestStatus;
      }
      else
      {
#ifdef DEBUG
		 vConsoleWrite("timeout\n");
#endif
         dTestStatus = E__READ_ERROR;
         return dTestStatus;
      }
    }
	if((*(UINT16*)(BUFFER_ADDRESS + 0x1FE)) == 0xAA55)
		dTestStatus = E__OK;
	else
		dTestStatus = E__IDE_NO_SIGN;

	return dTestStatus;
}


/*****************************************************************************
 * Sii3124RegaccessTest: Test function for non destructive read and write of
 *						sii3124 Registers to test the presence of hard disk/ CF
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (sii3124RegaccessTest, "SII3124 Disk Connectivity Test")
{
	UINT32 dideStatus;
	UINT32 dtest_status = E__TEST_NO_DEVICE;
	UINT8 bInstance = 0, bLoop = 0, i = 1, bport = 0;
	DEVICE sDevice;	

	if(adTestParams[0] == 2)
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;				
			bInstance++;	
		}
		else
		{
			bInstance = adTestParams[i];
		}
		if(adTestParams[2] == 0)
		{
			bLoop = 1;
			bport++;
		}
		else
		{
			bport = adTestParams[i + 1];
		}
	}
	else	
	{
		bLoop = 1;
		bInstance++;
		bport++;
	}
	/*return(E__BIT_IDE_PARAMS);*/

	do {
#ifdef DEBUG
			sprintf(buffer, "Instance to find: %d: Port: %d\n", bInstance,
					adTestParams[2]);
			vConsoleWrite(buffer); 
#endif
		dideStatus = dTestInit (&sDevice, bInstance); /* bInstance - Contoller instance */
		
		if (dideStatus == E__OK)
		{
			if(bport > 4)
			{
				#ifdef DEBUG  
      				vConsoleWrite("Unable to locate instance of SII3124 port\n"); 
				#endif
				return (E__TEST_NO_PORT);
			}
			dtest_status = dTestRegs (&sDevice, bport - 1); /* bport - Port number */
#ifdef DEBUG
			sprintf(buffer, "Test status: 0x%x\n", dtest_status); 
			vConsoleWrite(buffer); 
#endif				
			if(dtest_status == E__OK)
			{
				if((adTestParams[1] == 0) && (adTestParams[2] != 0))
				{
					bInstance++;
					bLoop = 1;
				}
				if((adTestParams[2] == 0) && (adTestParams[1] != 0))
				{
					if(bport < 4)
					{
						bport++;
						bLoop = 1;
					}
					else
						bLoop = 0;
				}
				if((adTestParams[1] == 0) && (adTestParams[2] == 0))
				{
					if(bport < 4)
					{
						bport++;
						bLoop = 1;
					}
					else
					{
						bport = 1;
						bInstance++;
						bLoop = 1;
					}
				}		
			}	
		}
		else if (bLoop == 0)
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of SII3124\n"); 
			#endif
			return (E__TEST_NO_DEVICE);
		}

		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

	} while ((bLoop == 1) && (dtest_status == E__OK));		/* test all requested instances */

	/*vClosePciDevice(&sDevice);*/
	vFreePtr(dHandle1);
	vFreePtr(dHandle2);
	vFreePtr(dHandle3);
	vFreePtr(dHandle4);
	if((dtest_status != E__OK) && (dtest_status != E__TEST_NO_DEVICE))
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}
	return (dtest_status);

} /* sii3124RegaccessTest */


/*****************************************************************************
 * sii3124InterruptTest: Test function for interrupt generation of SII1324 
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (sii3124InterruptTest, "SII3124 Interrupt Test")
{
	UINT32 dideStatus;
	UINT32 dtest_status = E__TEST_NO_DEVICE;
	UINT8 bInstance = 0, bLoop = 0, i = 1, bport = 0;
	DEVICE sDevice;	

	if(adTestParams[0] == 2)
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;				
			bInstance++;	
		}
		else
		{
			bInstance = adTestParams[i];
		}
		if(adTestParams[2] == 0)
		{
			bLoop = 1;
			bport++;
		}
		else
		{
			bport = adTestParams[i + 1];
		}
	}
	else	
	{
		bLoop = 1;
		bInstance++;
		bport++;
	}
	/*return(E__BIT_IDE_PARAMS);*/

	do {
#ifdef DEBUG
			sprintf(buffer, "Instance to find: %d: Port: %d\n", bInstance,
					adTestParams[2]);
			vConsoleWrite(buffer); 
#endif
		dideStatus = dTestInit (&sDevice, bInstance); /* bInstance - Contoller instance */
		
		if (dideStatus == E__OK)
		{
			if(bport > 4)
			{
				#ifdef DEBUG  
      				vConsoleWrite("Unable to locate instance of SII3124 port\n"); 
				#endif
				return (E__TEST_NO_PORT);
			}
			dtest_status = dIdentifyDrive(&sDevice, bport - 1); /* bport - Port number */
#ifdef DEBUG
			sprintf(buffer, "Test status: 0x%x\n", dtest_status); 
			vConsoleWrite(buffer); 
#endif				
			if(dtest_status == E__OK)
			{
				if((adTestParams[1] == 0) && (adTestParams[2] != 0))
				{
					bInstance++;
					bLoop = 1;
				}
				if((adTestParams[2] == 0) && (adTestParams[1] != 0))
				{
					if(bport < 2)
					{
						bport++;
						bLoop = 1;
					}
					else
						bLoop = 0;
				}
				if((adTestParams[1] == 0) && (adTestParams[2] == 0))
				{
					if(bport < 2)
					{
						bport++;
						bLoop = 1;
					}
					else
					{
						bport = 1;
						bInstance++;
						bLoop = 1;
					}
				}		
			}	
		}
		else if (bLoop == 0)
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of SII3124\n"); 
			#endif
			return (E__TEST_NO_DEVICE);
		}

		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

	} while ((bLoop == 1) && (dtest_status == E__OK));		/* test all requested instances */

	/*vClosePciDevice(&sDevice);*/
	vFreePtr(dHandle1);
	vFreePtr(dHandle2);
	vFreePtr(dHandle3);
	vFreePtr(dHandle4);
	if((dtest_status != E__OK) && (dtest_status != E__TEST_NO_DEVICE))
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}
	return (dtest_status);

} /* sii3124InterruptTest */


/*****************************************************************************
 * sii3124RdFirstSectTest: Test function to read first sector 
 * 							of the disk and verify
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (sii3124RdFirstSectTest, "SII3124 Read First Sector Test")
{
	UINT32 dideStatus;
	UINT32 dtest_status = E__TEST_NO_DEVICE;
	UINT8 bInstance = 0, bLoop = 0, i = 1, bport = 0;
	DEVICE sDevice;	

	if(adTestParams[0] == 2)
	{
		if (adTestParams[1] == 0)
		{
			bLoop = 1;				
			bInstance++;	
		}
		else
		{
			bInstance = adTestParams[i];
		}
		if(adTestParams[2] == 0)
		{
			bLoop = 1;
			bport++;
		}
		else
		{
			bport = adTestParams[i + 1];
		}
	}
	else	
	{
		bLoop = 1;
		bInstance++;
		bport++;
	}
	/*return(E__BIT_IDE_PARAMS);*/

	do {
#ifdef DEBUG
			sprintf(buffer, "Instance to find: %d: Port: %d\n", bInstance,
					adTestParams[2]);
			vConsoleWrite(buffer); 
#endif
		dideStatus = dTestInit (&sDevice, bInstance); /* bInstance - Contoller instance */
		
		if (dideStatus == E__OK)
		{
			if(bport > 4)
			{
				#ifdef DEBUG  
      				vConsoleWrite("Unable to locate instance of SII3124 port\n"); 
				#endif
				return (E__TEST_NO_PORT);
			}
			dtest_status = dReadSector(&sDevice, bport - 1); /* bport - Port number */
#ifdef DEBUG
			sprintf(buffer, "Test status: 0x%x\n", dtest_status); 
			vConsoleWrite(buffer); 
#endif				
			if(dtest_status == E__OK)
			{
				if((adTestParams[1] == 0) && (adTestParams[2] != 0))
				{
					bInstance++;
					bLoop = 1;
				}
				if((adTestParams[2] == 0) && (adTestParams[1] != 0))
				{
					if(bport < 2)
					{
						bport++;
						bLoop = 1;
					}
					else
						bLoop = 0;
				}
				if((adTestParams[1] == 0) && (adTestParams[2] == 0))
				{
					if(bport < 2)
					{
						bport++;
						bLoop = 1;
					}
					else
					{
						bport = 1;
						bInstance++;
						bLoop = 1;
					}
				}		
			}	
		}
		else if (bLoop == 0)
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of SII3124\n"); 
			#endif
			return (E__TEST_NO_DEVICE);
		}

		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

	} while ((bLoop == 1) && (dtest_status == E__OK));		/* test all requested instances */

	/*vClosePciDevice(&sDevice);*/
	vFreePtr(dHandle1);
	vFreePtr(dHandle2);
	vFreePtr(dHandle3);
	vFreePtr(dHandle4);
	if((dtest_status != E__OK) && (dtest_status != E__TEST_NO_DEVICE))
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}
	return (dtest_status);

} /* sii3124RdFirstSectTest */

