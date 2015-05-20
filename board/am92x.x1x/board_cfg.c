
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

/* includes */

#include <stdtypes.h>
#include <string.h>
#include <stdio.h>
#include <private/port_io.h>
#include <private/sys_delay.h>
#include <private/debug.h>
#include <bit/bit.h>
#include <bit/io.h>
#include <bit/pci.h>
#include <bit/delay.h>
#include <bit/board_service.h>
#include "cctboard.h"
#include <bit/mem.h>
#include <bit/msr.h>
#include <devices/eth.h>

/* defines */

//#define DEBUG

#define PORT_61		0x61		/* NMI Status & Control register */
#define SPKR_EN		0x02
#define SPKR_GATE	0x01

#define COM1		0			/* array index for numbered COM ports */
#define COM2		1
#define COM3		2

#define PCI_PRI_BUS_NUM				0x18
#define PCI_SEC_BUS_NUM				0x19
#define PCI_SUB_BUS_NUM				0x1A

#define VID_INTEL		0x8086
#define DID_82580_EB    0x1510
#define DIDVID_82580_EB  ((DID_82580_EB << 16) | VID_INTEL)

#define SPEEDSTEP_ERROR				(E__BIT + 0x1000)
#define SPEEDSTEP_PROCHOT_ERROR     (E__BIT + 0x1010)
#define PORT_NTMTBLADDR		0x004D0
#define PORT_NTMTBLSTS		0x004D4
#define PORT_NTMTBLDATA		0x004D8
#define PORT_NTREQIDCAP		0x004DC

#define PORT0_NT_REG_BASE	0x01000
#define PORT8_NT_REG_BASE	0x11000
#define PORT12_NT_REG_BASE	0x19000
#define PORT16_NT_REG_BASE	0x21000

#define PORT_NTCTL			0x00400
#define  USER_JUMPER		0x002	/*0 = MTH 1 = SOAK */

//#define DEBUG
/* typedefs */

/* constants */

const UINT8			bDebugPort = COM1;	/* COM1 is debug console */

const SCC_PARAMS	asSccParams[] =
{
	{ 1,	"Ch 0",	0x03F8, 4 },	/* COM1 Port A in SIO 	  */
	{ 2,	"Ch 1", 0x02F8, 3 },	/* COM2 Port B     -"-    */
	{ 0,	"",		0,		0 }
};

/* locals */
typedef struct{
	UINT16 regOffset;
	UINT32 dwValue;
}PCI_REG_DATA;
typedef struct {
	UINT8 bBus,bDevice,bFunc;
	UINT8 nNumRegs;
	PCI_REG_DATA pciRegData[8];
}PCI_REG_CONFIG;

PCI_REG_CONFIG pciStoredConfig;

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * brdGetConsolePort: report the stdin/stdout console device
 * ptr	  : pointer to a UINT16 for base address of port
 * RETURNS: E__OK
 */

UINT32 brdGetConsolePort (void* ptr)
{
	
	*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;
	#if 0
	/* Serial console can be COM1 or COM3 */

	if (bConsPort == 1)
		*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;
	else
		*((UINT16*)ptr) = asSccParams[COM2].wBaseAddr;
	#endif
	return E__OK;

} /* brdGetConsolePort () */


/*****************************************************************************
 * brdGetDebugPort: report the debug console device
 * ptr	  : pointer to a UINT16 for base address of port
 * RETURNS: E__OK
 */
UINT32 brdGetDebugPort (void* ptr)
{
	*((UINT16*)ptr) = asSccParams[bDebugPort].wBaseAddr;

	return E__OK;

} /* brdGetDebugPort () */

/*****************************************************************************
 * brdGetSccPortMap: provide a map of ports available for testing
 * ptr	  : pointer to a UINT32 bitmap - when bit set indicates port available,
 *          bit-0 = COM1
 * RETURNS: E__OK
 */
UINT32 brdGetSccPortMap (void* ptr)
{
	UINT32	dPortMap = 0;
	int		i = 0;
	UINT16  consp;

	//get console port base address
	brdGetConsolePort (&consp);

/*
 * - check soak/MTH mode, if not peripheral and in soak then soak port can't be tested,
 *   if peripheral mode then soak port can be tested, if MTH then console port can't be tested
 * - Check if in IBIT, if in IBIT then console port can't be tested
 * - check debug mode, if debug mode enabled then debug port can't be tested
 * */

	/* Build a port map */
	while (asSccParams[i].bPort != 0)
	{
		//skip console port in MTH and IBIT mode
		if( asSccParams[i].wBaseAddr == consp)
		{
			if( (bGetTestHandler() == TH__MTH) || (bGetTestHandler() == TH__LBIT ) )
			{
				i++;
				continue;
			}
		}

		//skip debug port, COM1 for vx813, if debug channel open
		if(asSccParams[i].bPort == 1)
			if(sysIsDebugopen() == 1)
			{
				i++;
				continue;
			}

		dPortMap |= (UINT32)1 << asSccParams[i].bPort;
		i++;
	}

	*((UINT32*)ptr) = dPortMap;

	return E__OK;
} /* brdGetSccPortMap() */

/*****************************************************************************
 * brdGetSokePort: report the stdin/stdout console device
 *
 * RETURNS: base address of port
 */

UINT32 brdGetSokePort (void *ptr)
{
	*((UINT16*)ptr) = asSccParams[COM1].wBaseAddr;

	return E__OK;

} /* brdGetConsolePort () */


/*****************************************************************************
 * brdGetSccParams: report parameters for specified SCC port
 *
 *SCC_PARAMS*
 *UINT8		bPort
 * RETURNS: pointer to port data
 */
UINT32 brdGetSccParams(void* ptr )
{
	int		i = 0;

	/* Find data for port */
	while (asSccParams[i].bPort != 0)
	{
		if (asSccParams[i].bPort == ((SCC_PARAMS*)ptr)->bPort  )
		{
			memcpy(ptr, &asSccParams[i], sizeof(SCC_PARAMS) );
			return E__OK;
		}

		i++;
	}
	return E__FAIL;

} /* brdGetSccParams () */



/*****************************************************************************
 * brdGetMaxDramBelow4GB: get size of DRAM below 4GB
 *
 * RETURNS: 1 + highest addressable DRAM, below 4GB
 */
UINT32 brdGetMaxDramBelow4GB (void* ptr)
{
	*((UINT32*)ptr) = 0x40000000; /* fixed at 'safe' 1GB */

	return E__OK;

} /* brdGetMaxDramBelow4GB () */



/*****************************************************************************
 * brdFlashLed: flash a suitable board specific LED - forever
 *
 * RETURNS: none
 */
UINT32 brdFlashLed (void* ptr)
{
	UINT8	bTemp;

	(void)ptr;

	/* Flash LED at 2Hz */

	bTemp = sysInPort8 (PORT_61);
	bTemp = (bTemp & ~SPKR_EN) | SPKR_GATE;			/* Gate enable, Spkr disable */

	while (1)
	{
		sysOutPort8 (PORT_61, bTemp | SPKR_EN);		/* 300mS on */
		sysDelayMilliseconds (300);

		sysOutPort8 (PORT_61, bTemp);				/* 200mS off */
		sysDelayMilliseconds (200);
	}

	return E__OK;

} /* brdFlashLed () */



/*****************************************************************************
 * brdReset:
 *
 * RETURNS:
 */
UINT32 brdReset (void *ptr)
{
	(void)ptr;

	/* First try the PCI reset port */
	sysDelayMilliseconds(200);
	sysOutPort8 (0x0CF9, 0x6);
	//sysOutPort8 (0x92, ((sysInPort8(0x92)) | 0x1));

	/* If that fails, try keyboard controller */
	sysOutPort8 (0x0064, 0xFE);

	return E__OK; // should never return

} /* brdReset () */




/*****************************************************************************
 * brdCheckUserMthSwitch:
 *
 * RETURNS: switch Flag
 * */
UINT32 brdCheckUserMthSwitch (void *ptr)
{
	UINT8	umswitch;

	umswitch = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8));

	if ((umswitch & USER_TEST_MTH_STH) == USER_TEST_MTH_STH)
		*((UINT8*)ptr) =  1; /*STH*/
	else
		*((UINT8*)ptr) =  0; /*MTH*/

	return E__OK;

} /* brdCheckUserMthSwitch () */


UINT32 brdGetIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8));
    if (reg & IPMI_NMI)
		*((UINT8*)ptr) =1;
	else
		*((UINT8*)ptr) =0;

	return E__OK;
}

UINT32 brdclearIPMINMI(void *ptr)
{
	UINT8 reg;

	reg = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8)) & (~IPMI_NMI);
    vIoWriteReg(STATUS_REG_1, REG_8, reg );

	return E__OK;
}


/*****************************************************************************
 * brdSkipEthInterface: selects front or rear Ethernet interface
 *
 * RETURNS: success or error code
 */

UINT32 brdSkipEthInterface (void *ptr)
{

		if((((SkipEthIf_PARAMS*)ptr)->dDidVid   == DIDVID_82580_EB) &&
		   (((SkipEthIf_PARAMS*)ptr)->bInstance > 3) )
			return E__OK;
		else
			return 0x01;

} /* brdSelectEthInterface () */

#define  GPIO_BASE	 0x48
#define LPC_BRIDGE_DEV	31
#define GP_LVL2 0x38

UINT32 brdMemfromGPIO (void *ptr)
{
	PCI_PFA pfa;
	UINT16 wGpioBase = 0;	
	UINT32 dTemp = 0;


	pfa = PCI_MAKE_PFA (0, 0, 0);

	if (dPciReadReg(pfa, 0x00, 0x03) == 0x01048086)
	{
		pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
		wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
		dTemp = (dIoReadReg (wGpioBase+ GP_LVL2, REG_32) >> 6) & 0x3;
		*((UINT32*)ptr) = dTemp;

		return E__OK;

	}
	else
	{

		return E__FAIL;
	}

}

UINT32 brdConfigCpciBackplane (void *ptr)
{
	PCI_PFA	pfa;

		pfa = PCI_MAKE_PFA (0, 28, 0);
	//PCI- PCI Bridge
	PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
    PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0x03);
    PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0x04); //A

		
	pfa = PCI_MAKE_PFA(0x3, 0x0, 0);
    *((PCI_PFA*)ptr) = pfa;
	
	
	
    return (E__OK);
    
    }
    
    
    UINT32 brdDeConfigCpciBackplane (void *ptr)
    {
    	(void)ptr;
    	PCI_PFA	pfa;      
    
		pfa = PCI_MAKE_PFA (3, 0, 0);
		PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
		PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
		PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);

		pfa = PCI_MAKE_PFA (0, 28, 0);
		PCI_WRITE_BYTE (pfa, PCI_PRI_BUS_NUM, 0);
		PCI_WRITE_BYTE (pfa, PCI_SEC_BUS_NUM, 0);
		PCI_WRITE_BYTE (pfa, PCI_SUB_BUS_NUM, 0);
        return (E__OK);
	
	}


UINT32 brdGetCpciBrInstance (void *ptr)
{
	UINT32 dInstance = 0;
	(void)ptr;

	dInstance = 1;
	*((UINT32*)ptr) = dInstance;

	return (E__OK);
}

UINT32 brdCheckCpciIsSyscon (void *ptr)
{

	*((UINT8*)ptr) = 1;
	 return (E__OK);


}


UINT32 brdSkipEthTest (void *ptr)
{
/*
		if(((SkipEthTest_PARAMS*)ptr)->dDidVid   != DIDVID_82579LM)
			return E__OK;

		if((((SkipEthTest_PARAMS*)ptr)->dDidVid   == DIDVID_82579LM) &&
		   (((SkipEthTest_PARAMS*)ptr)->bTestid   == ETHEXTLOOPBACKTEST
		   ) )
			return E__OK;
		else
			return 0x01;
*/
	return E__OK;
}

UINT32 brdSetSpeedStep(void *ptr)
{
	UINT32 msrHigh=0,msrLow=0,timeout;
	UINT8 maxRatio=0,minRatio=0,i=0;
#ifdef DEBUG
	char achBuffer[80];
#endif	

    /*Get the min & max ratio supported*/
    vReadMsr(0xce,&msrHigh,&msrLow);

	if(((msrHigh>>2)&3)>0)
	{
#ifdef DEBUG
		 sprintf ( achBuffer, "Configurable TDP-SKU\n");
		 vConsoleWrite (achBuffer);
#endif

		vReadMsr(0x648,&msrHigh,&msrLow);
		maxRatio = msrLow&0xff;
		vReadMsr(0xce,&msrHigh,&msrLow);
		minRatio = (msrHigh>>8) & 0xff;
	}
	else
	{
#ifdef DEBUG
		 sprintf ( achBuffer, "Nominal TDP-SKU\n");
		 vConsoleWrite (achBuffer);
#endif

		maxRatio = (msrLow>>8) & 0xff;
		minRatio = (msrHigh>>8) & 0xff;
	}
	
#ifdef DEBUG
	 sprintf ( achBuffer, "minRatio=%d, maxRatio=%d\n",minRatio,maxRatio);
	 vConsoleWrite (achBuffer);
	 
	 vReadMsr(0x194,&msrHigh,&msrLow);
	 sprintf ( achBuffer, "FLEX_RATIO=%x:%x\n",msrHigh,msrLow);
	 vConsoleWrite (achBuffer);
#endif

	for( i=minRatio; i <= maxRatio; i++ )
     {
    	 /*Set the intended ratio*/
        vReadMsr(0x199,&msrHigh,&msrLow);

		#ifdef DEBUG
			 sprintf ( achBuffer, "0x199 before=%x,%x\n",msrHigh,msrLow);
			 vConsoleWrite (achBuffer);
		#endif

		msrLow &= 0xffff00ff;
        msrLow |= (i << 8) & 0xff00;
        vWriteMsr(0x199,msrHigh,msrLow);
		#ifdef DEBUG
			sprintf ( achBuffer, "0x199 After=%x,%x\n",msrHigh,msrLow);
			vConsoleWrite (achBuffer);
		#endif

		timeout=10;

         /*Check if the ratio has been set*/
         while( --timeout )
         {
        	 vDelay(10);
            vReadMsr(0x198,&msrHigh,&msrLow);

            if( ( (msrLow >> 8) & 0xff) == i )
            {
				 #ifdef DEBUG
					sprintf ( achBuffer, "Current P-State ratio=%x\n",((msrLow >> 8) & 0xff));
					vConsoleWrite (achBuffer);
				 #endif
				 break;
            }
         }

         if( !timeout )
         {
			#ifdef DEBUG
				  sprintf ( achBuffer, "Unable to set ratio=%x, Current %x\n", i,
						    ((msrLow >> 8) & 0xff));
				  vConsoleWrite (achBuffer);

				  vReadMsr(0x199,&msrHigh,&msrLow);
				  sprintf ( achBuffer, "Perf CTL=%x:%x\n",msrHigh,msrLow);
				  vConsoleWrite (achBuffer);

				  vReadMsr(0x198,&msrHigh,&msrLow);
				  sprintf ( achBuffer, "Perf STS=%x:%x\n",msrHigh,msrLow);
				  vConsoleWrite (achBuffer);
			#endif

            vReadMsr(0x19C,&msrHigh,&msrLow);
            if ((msrLow & 0x02) == 0x02)
            {
#ifdef DEBUG
            	sprintf ( achBuffer, "Therm STS=%x:%x\n",msrHigh,msrLow);
            	sysDebugWriteString(achBuffer);
#endif
            	*((UINT32*)ptr) =  SPEEDSTEP_PROCHOT_ERROR;
            	return (E__OK);
            }
			#if 0
			#ifdef DEBUG
				sprintf(achBuffer,"**FAIL**:Speed Step CPU Temperature:%d\n\n", getCPUTemp());
				sysDebugWriteString(achBuffer);
			#endif
			#endif
           *((UINT32*)ptr) =  SPEEDSTEP_ERROR + i;
           return (E__OK);
         }
     }

    *((UINT32*)ptr) =  E__OK;
    return (E__OK);
}
UINT32 brdStoreConfigChanges (void* ptr)
{
	UINT8 i;
	UINT32  bIoWindow;
	PCI_PFA	 pfa;

	//Storing initial generic LPC IO decode ranges to be restored later
	pfa = PCI_MAKE_PFA (0, 31, 0);
	pciStoredConfig.bBus=0;
	pciStoredConfig.bDevice=31;
	pciStoredConfig.bFunc=0;
	for(i=0;i<4;i++)
	{
		pciStoredConfig.pciRegData[i].regOffset=0x84+i*4;
		bIoWindow = PCI_READ_DWORD(pfa, 0x84+i*4);
		pciStoredConfig.pciRegData[i].dwValue=bIoWindow;
	}
	pciStoredConfig.nNumRegs=4;
	return E__OK;

} /* brdStoreConfigChanges () */
UINT32 brdRestoreConfigChanges (void* ptr)
{
	UINT8 i;
	PCI_PFA	 pfa;
  
	if(pciStoredConfig.nNumRegs>=8){
		//Something freaky. might have been called without storing anything before
		return E__OK;
	}
	//Restoring generic LPC IO decode ranges back to initial values
	pfa = PCI_MAKE_PFA (pciStoredConfig.bBus, pciStoredConfig.bDevice, pciStoredConfig.bFunc);
	for(i=0;i<pciStoredConfig.nNumRegs;i++)
	{
		PCI_WRITE_DWORD(pfa, 
			pciStoredConfig.pciRegData[i].regOffset,
			pciStoredConfig.pciRegData[i].dwValue);

	}
	pciStoredConfig.nNumRegs=0;//Resetting to be on safer side
	return E__OK;

} /* brdRestoreConfigChanges () */

UINT32 brdEnableSmicLpcIoWindow(void* ptr)
{
	UINT8	i;
	UINT32  bIoWindow;
	PCI_PFA	 pfa;
#ifdef DEBUG
	char achBuffer[80];
#endif
	pfa = PCI_MAKE_PFA (0, 31, 0);
	for(i=0;i<4;i++)
	{
		bIoWindow = PCI_READ_DWORD(pfa, 0x84+i*4);
		if((bIoWindow & 0xfff) == 0xCA1)
		{
			//Already Io Window is enabled, nothing to be done
			return E__OK;
		}
	}
	PCI_WRITE_DWORD(pfa, 0x90,0x00FC0CA1);
	#ifdef DEBUG
	sprintf(achBuffer,"value in Register 0x90 :%#x\n\n", PCI_READ_DWORD(pfa, 0x90));
	vConsoleWrite(achBuffer);
	#endif
	return E__OK;

} /* brdEnableWdLpcIoWindow() */


