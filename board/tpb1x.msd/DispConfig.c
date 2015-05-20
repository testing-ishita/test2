 
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

/*
 * DispConfig.c - Display Board Configuration
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/DispConfig.c,v 1.2 2015-04-01 16:44:29 hchalla Exp $
 *
 * $Log: DispConfig.c,v $
 * Revision 1.2  2015-04-01 16:44:29  hchalla
 * Initial release of CUTE firmware 02.02.01.
 *
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
 * Revision 1.4  2014-04-22 16:38:12  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.3  2013-10-08 07:17:49  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.2  2013/09/26 12:55:52  chippisley
 * Added code to obtain MAC address from network driver.
 *
 * Revision 1.1  2013/09/04 07:13:49  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2012/04/12 12:31:48  hchalla
 * Added flag for ethernet to display the mac address while running t,999 test.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>	
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <bit/board_service.h>

#include <private/sys_delay.h>
#include "../tpb1x.msd/cctboard.h"
#include "../tpb1x.msd/config.h"
#include "../tpb1x.msd/version.h"

/* defines */
/*defines*/
#define I7_DEV   0
#define I7_VID   0x8086
#define I7_DID1  0x0044    /*host bridge*/
#define I7_DID2  0x0104    /*host bridge*/

#define TOLUD_HASWELL	0xBC
#define TOUUD_HASWELL	0xA8


#define  TOLUD        0xB0  /*top of low usable DRAM*/
#define  TOUUD        0xA2  /*top of upper usable DRAM*/
#define	SIZE_1MB		0x0100000L

#define TOLUD_SANDYBRIDGE  0xBC
#define TOUUD_SANDYBRIDGE  0xA8

//#define DEBUG
#define SCR_ERROR		(E__BIT + 0x1000)
#define  GPIO_BASE	    0x48
#define LPC_BRIDGE_DEV	31

#define GP_LVL          0x0C
#define GP_LVL2         0x38
/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT8 bCpuCount;
extern void vGetEthMacAddr(	UINT8  bInstance, UINT8* pbAddress,UINT8 bMode);
#ifdef INCLUDE_NETWORK
extern int geifGetMacAddr( char *pName, UINT8 *pMacAddr );
#endif
static void vGetSpeedString (UINT8 bCode, UINT8 *abText);
/* forward declarations */


static PTR48 tPtr1;
static volatile UINT32 mapRegSpace=0;
static UINT32 mHandle;

static void vI7_MemSize_Haswell(UINT32* pdMemSize)
{
    UINT32 TOLUD_bRegVal;
    UINT32 TOUUD_bRegVal_Lower;
    UINT32 TOUUD_bRegVal_Higher=0;
    UINT32 g4=0xFFFFFFFFL;
    UINT32 PCI_addr;
    PCI_PFA	pfa;


	pfa = PCI_MAKE_PFA (0, 0, 0);

    /*read the TOLUD register values*/
    TOLUD_bRegVal = (UINT32)dPciReadReg(pfa,TOLUD_HASWELL,REG_32);

     /*read the TOUUD lower register values*/
    TOUUD_bRegVal_Lower = (UINT32)dPciReadReg(pfa,TOUUD_HASWELL,REG_32);

     /*read the TOUUD upper register values*/
     TOUUD_bRegVal_Higher = (UINT32)dPciReadReg(pfa,(TOUUD_HASWELL + 4),REG_32);

     PCI_addr = g4 - TOLUD_bRegVal + 1;

     if(  TOUUD_bRegVal_Higher != 0  )
     {
     			*pdMemSize=  TOUUD_bRegVal_Higher * 4096;
     }
     else if ( TOUUD_bRegVal_Higher == 0 && TOUUD_bRegVal_Lower != 0 )
     {
     		  *pdMemSize = (TOUUD_bRegVal_Lower - PCI_addr)/0x100000;
     }
     else
     {
     		  *pdMemSize =  TOLUD_bRegVal/0x100000;
     }
 }


UINT8 bGetVarInfo (void)
{

	PCI_PFA pfa;
	UINT16 wGpioBase = 0;
	UINT8 bVal   = 0;
	UINT32 dTemp = 0;
	UINT8 bTemp = 0;
	char achBuffer[80];
	UINT8 bVarInfo=0;

	pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
	wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

	// GPIO_28
	dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

	bVal = (UINT8)((dTemp >> 28) & 0x01);

	//GPIO_39
	dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);
	dTemp =(UINT8) (dTemp>> 7) & 0x01;


	 bVarInfo = (UINT8) (dTemp<<1) |(bVal);

	 return bVarInfo;
}
/*****************************************************************************
 * DispConfig: Display board configuration
 *
 * RETURNS: None
 */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	temp32, addr, rt = E__OK;
	UINT16	mem;
	UINT8	reg;	
	UINT8 bVarInfo=0;
	char	achBuffer[80];	/* text formatting buffer */
	PCI_PFA	pfa;
	UINT32 dMem = 0,dTemp = 0;
	UINT16 wGpioBase = 0;
	UINT32 dVal = 0;
	UINT8 bLanAddress[6],bVal;

	board_service(SERVICE__BRD_UNLOCK_CCT_IOREGS, NULL,NULL);
    // obtain ram size in GB from IO strap settings
		// output parameter in bVal is GPIO strap setting
		// returned parameter in bVal is memory size
		board_service (SERVICE__BRD_GET_DRAM_SIZE_FROM_GPIO, NULL, (void *)&bVal);

#ifdef DEBUG
		sprintf (achBuffer, "bVal                  : %dGB\n", bVal);
		vConsoleWrite (achBuffer);
#endif

		vI7_MemSize_Haswell(&dMem);

#ifdef DEBUG
		sprintf (achBuffer, "MEMORY                  : %dGB\n", dMem/1024);
		vConsoleWrite (achBuffer);
#endif
		switch (bVal)
		{
			case 4:
			case 8:
			case 16:
			case 32:
				dMem = dMem/1024;
				mem = bVal;
#ifdef DEBUG
				sprintf (achBuffer, "%dGB Fitted\n", mem);
				vConsoleWrite (achBuffer);
#endif
				break;

			default:
				dMem = 0;
				mem = 0;
#ifdef DEBUG
				vConsoleWrite("0GB Fitted\n");
#endif
				break;
		}

	sprintf (achBuffer, "Num CPUs             : %d\n", (int)bCpuCount);
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "CPU Frequency        : %i.%02iGHz\n", sysGetCpuFrequency () / 1000,
															 (sysGetCpuFrequency () % 1000) / 10);
	vConsoleWrite (achBuffer);	
	sprintf (achBuffer, "Fitted RAM           : %dGB\n", mem);
	vConsoleWrite (achBuffer);	

	if (dMem == mem)
	{
		sprintf (achBuffer, "Memory Detected      : Valid\n");
		vConsoleWrite (achBuffer);
	}
	else
	{
		sprintf (achBuffer, "Memory Detected      : In-Valid\n");
		vConsoleWrite (achBuffer);
	}

	bVarInfo = bGetVarInfo();

	switch ( bVarInfo )
	{
		case 0x00:
			sprintf (achBuffer, "Board Variant        : K-Grade \n");
			break;

		case 0x01:
			sprintf (achBuffer, "Board Variant        : RFU \n");
			break;

		case 0x02:
			sprintf (achBuffer, "Board Variant        : E-Grade \n");
			break;

		case 0x03:
			sprintf (achBuffer, "Board Variant        : N-Grade \n");
			break;

		default:
			sprintf (achBuffer, "Board Variant         : UNKNOWN\n");
			break;
	}
	vConsoleWrite (achBuffer);

	       reg = ((UINT8)dIoReadReg(STATUS_REG_0, REG_8));
		   sprintf (achBuffer, "Operating Mode       : %s\n", ((reg & MODE_JUMPER) == MODE_JUMPER?"CUTE":"BIOS"));
		   vConsoleWrite (achBuffer);
		   sprintf (achBuffer, "User                 : %s\n", (reg & USER_JUMPER)==USER_JUMPER?"Soak":"MTH");
		   vConsoleWrite (achBuffer);
		   sprintf (achBuffer, "Console Mode         : %s\n", ((reg & CONS_JUMPER) == CONS_JUMPER?"VGA":"Serial"));
		   vConsoleWrite (achBuffer);
		   sprintf (achBuffer, "Console Port         : %s\n", ((reg & CONS_PORT) == CONS_PORT?"COM2":"COM1"));
		   vConsoleWrite (achBuffer);
		   sprintf (achBuffer, "Hardware Revision    : %c\n", ('A' + ((reg >> HW_REV_SHIFT) & 0x07)));
		   vConsoleWrite (achBuffer);

		   reg = ((UINT8)dIoReadReg(STATUS_REG_1, REG_8));
		   sprintf (achBuffer, "PMC2 Frequency       : ");
		   vConsoleWrite (achBuffer);
		   vGetSpeedString((reg & PMC2_FREQ) >> 2, (UINT8 *) achBuffer);
	       vConsoleWrite (achBuffer);

	       sprintf (achBuffer, "PMC1 Frequency       : ");
	       vConsoleWrite (achBuffer);
		   vGetSpeedString((reg & PMC1_FREQ) >> 5, (UINT8 *) achBuffer);
		   vConsoleWrite (achBuffer);


	       sprintf(achBuffer, "User LED             : %s\n", ((reg & USER_LED) == USER_LED?"ON":"OFF"));
	       vConsoleWrite (achBuffer);

	       reg = ((UINT8)dIoReadReg(STATUS_REG_2, REG_8));
	       sprintf (achBuffer, "PMC1 Present         : %s\n", ((reg & PMC1_PRESENT) == PMC1_PRESENT?"Yes":"No"));
		   vConsoleWrite (achBuffer);

		   sprintf (achBuffer, "PMC2 Present         : %s\n", ((reg & PMC2_PRESENT) == PMC2_PRESENT?"Yes":"No"));
		   vConsoleWrite (achBuffer);

	   	   sprintf (achBuffer, "PMC/XMC INIT Done    : %s\n", ((reg & PMC_EREADY) == PMC_EREADY?"Yes":"No"));
		   vConsoleWrite (achBuffer);

		   sprintf (achBuffer, "XMC1 Present         : %s\n", ((reg & XMC1_PRESENT) == XMC1_PRESENT?"Yes":"No"));
		   vConsoleWrite (achBuffer);
	       sprintf (achBuffer, "XMC2 Present         : %s\n", ((reg & XMC2_PRESENT) == XMC2_PRESENT?"Yes":"No"));
	       vConsoleWrite (achBuffer);

		   sprintf (achBuffer, "IPMI NMI             : %s\n", ((reg & IPMI_NMI) == IPMI_NMI?"Occured":"Not Occured"));
		   vConsoleWrite (achBuffer);
		   sprintf (achBuffer, "Front Panel NMI      : %s\n", ((reg & FP_NMI) == FP_NMI?"Occured":"Not Occured"));
		   vConsoleWrite (achBuffer);

		   reg = ((UINT8)dIoReadReg(CPCI_STATUS, REG_8));
		   sprintf (achBuffer, "System Controller    : %s\n", ((reg & CPCI_SYSEN) == CPCI_SYSEN?"Yes":"No"));
		   vConsoleWrite (achBuffer);
		   sprintf (achBuffer, "PCI Bus in Backplane : %s\n", ((reg & CPCI_PRESENT) == CPCI_PRESENT?"Yes":"No"));
		   vConsoleWrite (achBuffer);
	       sprintf (achBuffer, "Forced Satellite Mode: %s\n", ((reg & FORCE_SAT) == FORCE_SAT?"Yes":"No"));
	       vConsoleWrite (achBuffer);
	       sprintf (achBuffer, "CPCI Bus Frequency   : %s\n", ((reg & CPCI_FREQ) == CPCI_FREQ?"66MHz":"33MHz"));
	       vConsoleWrite (achBuffer);
	       sprintf (achBuffer, "CPCI DEG# NMI        : %s\n", ((reg & CPCI_DEG) == CPCI_DEG?"Occured":"Not Occured"));
	       vConsoleWrite (achBuffer);
	       sprintf (achBuffer, "CPCI FAL# NMI        : %s\n", ((reg & CPCI_FAL) == CPCI_FAL?"Occured":"Not Occured"));
	       vConsoleWrite (achBuffer);

	       reg = ((UINT8)dIoReadReg(HW_STATUS_REG0, REG_8));
	       sprintf (achBuffer, "J5 SATA4 Routing     : %s\n", ((reg & J4_SATA4_SELECT) == J4_SATA4_SELECT?"SATA4 PINS":"USB3 PORT1"));
	       vConsoleWrite (achBuffer);
	       sprintf (achBuffer, "J5 SATA5 Routing     : %s\n", ((reg & J5_SATA5_SELECT) == J5_SATA5_SELECT?"SATA5 PINS":"USB3 PORT2"));
	        vConsoleWrite (achBuffer);
	        sprintf (achBuffer, "Compact Flash Present: %s\n", ((reg & CF_PRESENT) == CF_PRESENT?"Yes":"No"));
	       	vConsoleWrite (achBuffer);
	       	sprintf (achBuffer, "No. of PMC Site      : %s\n", ((reg & ONE_PMC_SITE) == ONE_PMC_SITE?"One":"Two"));
	       	vConsoleWrite (achBuffer);
	       	sprintf (achBuffer, "SPI Boot Device      : %s\n", ((reg & SPI_DEV_SELECT) == SPI_DEV_SELECT?"First Device":"Second Device"));
	       	vConsoleWrite (achBuffer);


	       	reg = ((UINT8)dIoReadReg(HW_STATUS_REG1, REG_8));
	       	sprintf (achBuffer, "ACPI Disable Status  : %s\n", ((reg & ACPI_DISABLE) == ACPI_DISABLE?"Disable":"Enable"));
	       	vConsoleWrite (achBuffer);
	       	sprintf (achBuffer, "CPCI PME Wake Mask   : %s\n", ((reg & CPCI_PME_WAKE_MASK) == CPCI_PME_WAKE_MASK?"Enabled":"Disabled"));
	       	vConsoleWrite (achBuffer);
	       	sprintf (achBuffer, "CPCI PB Rst Wake Mask: %s\n", ((reg & CPCI_PBRST_WAKE_MASK) == CPCI_PBRST_WAKE_MASK?"Enabled":"Disabled"));
	       	vConsoleWrite (achBuffer);
	       	sprintf (achBuffer, "PButton S5 Wake Mask : %s\n", ((reg & FRONT_PB_WAKE_MASK) == FRONT_PB_WAKE_MASK?"Enabled":"Disabled"));
	       	vConsoleWrite (achBuffer);
	       	sprintf (achBuffer, "RTM Reset Wake Mask  : %s\n", ((reg & RTM_RST_WAKE_MASK) == RTM_RST_WAKE_MASK?"Enabled":"Disabled"));
	       	vConsoleWrite (achBuffer);
	       	sprintf (achBuffer, "IPMI Wake Mask       : %s\n", ((reg & IPMI_WAKE_MASK) == IPMI_WAKE_MASK?"Enabled":"Disabled"));
	       	 vConsoleWrite (achBuffer);
	       	sprintf (achBuffer, "PCH GPIO I2C Mux     : %s\n", ((reg & PCH_GPIO_I2C_MUX_SEL) == PCH_GPIO_I2C_MUX_SEL?"SATA/USB":"PCIe Repeater"));
	        vConsoleWrite (achBuffer);

	       	 reg = ((UINT8)dIoReadReg(HW_STATUS_REG2, REG_8));
	       	 sprintf (achBuffer, "NVStrap I2CBus enable: %s\n", ((reg & NVSTRAP_I2C_BUS_ENABLE) == NVSTRAP_I2C_BUS_ENABLE?"Enabled":"Disabled"));
	       	 vConsoleWrite (achBuffer);
	       	 sprintf (achBuffer, "XMC I2C bus enable   : %s\n", ((reg & XMC_I2C_BUS_ENABLE) == XMC_I2C_BUS_ENABLE?"Enabled":"Disabled"));
	       	  vConsoleWrite (achBuffer);

	       	  reg = ((UINT8)dIoReadReg(HW_STATUS_REG3, REG_8));
	       	  sprintf (achBuffer, "AMT Disable Status   : %s\n", ((reg & AMT_DISABLE) == AMT_DISABLE?"Disabled":"Enabled"));
	       	   vConsoleWrite (achBuffer);
	       	   sprintf (achBuffer, "Serial Over LAN      : %s\n", ((reg & SERIAL_OVER_LAN) == SERIAL_OVER_LAN?"Disabled":"Enabled"));
	       		vConsoleWrite (achBuffer);

	       		board_service(SERVICE__BRD_UNLOCK_CCT_IOREGS, NULL,NULL);

	       		reg = ((UINT8)dIoReadReg(FPGA_REV_REG, REG_8));
	       		sprintf (achBuffer, "FPGA Revision        : %02d.%02d\n",((reg&0xF0) >>4), (reg&0x0F));
	       		vConsoleWrite (achBuffer);




	       vConsoleWrite ("\nETHERNET\n-------------------\n");
	       
#ifdef INCLUDE_NETWORK /* DO NOT call vGetEthMacAddr() in tests\eth.c when the network is included */
	       geifGetMacAddr( "ge0", bLanAddress );
#else
	       vGetEthMacAddr2(1, bLanAddress,ETH_MAC_DISP_MODE);
#endif
		   sprintf (achBuffer, "Ethernet 1 MAC       : %02X %02X %02X %02X %02X %02X\n",
				 bLanAddress[0], bLanAddress[1], bLanAddress[2],
				 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
		   vConsoleWrite (achBuffer);

		   vGetEthMacAddr2(2, bLanAddress,ETH_MAC_DISP_MODE);

		   sprintf (achBuffer, "Ethernet 2 MAC       : %02X %02X %02X %02X %02X %02X\n",
				 bLanAddress[0], bLanAddress[1], bLanAddress[2],
				 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
		   vConsoleWrite (achBuffer);

		   vGetEthMacAddr2(3, bLanAddress,ETH_MAC_DISP_MODE);

		   sprintf (achBuffer, "Ethernet 3 MAC       : %02X %02X %02X %02X %02X %02X\n",
				 bLanAddress[0], bLanAddress[1], bLanAddress[2],
				 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
		vConsoleWrite (achBuffer);
		
		vGetEthMacAddr2(4, bLanAddress,ETH_MAC_DISP_MODE);

		sprintf (achBuffer, "Ethernet 4 MAC       : %02X %02X %02X %02X %02X %02X\n",
					 bLanAddress[0], bLanAddress[1], bLanAddress[2],
					 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
			vConsoleWrite (achBuffer);

       		pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
       		wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

       		// GPIO_27
       		dTemp = dIoReadReg (wGpioBase + GP_LVL, REG_32);

       		if ((dTemp & 0x8000000) == 0x0000000)
       		{
       			vGetEthMacAddr2(5, bLanAddress,ETH_MAC_DISP_MODE);

       			sprintf (achBuffer, "Ethernet 5 MAC       : %02X %02X %02X %02X %02X %02X\n",
       						 bLanAddress[0], bLanAddress[1], bLanAddress[2],
       						 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
       				vConsoleWrite (achBuffer);

       				vGetEthMacAddr2(6, bLanAddress,ETH_MAC_DISP_MODE);

       				sprintf (achBuffer, "Ethernet 6 MAC       : %02X %02X %02X %02X %02X %02X\n",
       							 bLanAddress[0], bLanAddress[1], bLanAddress[2],
       							 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
       					vConsoleWrite (achBuffer);
       		}
		board_service(SERVICE__BRD_LOCK_CCT_IOREGS, NULL,NULL);

	return rt;
}


static void vGetSpeedString (UINT8 bCode, UINT8 *abText)
{
	switch(bCode)
	{
		case PMC_25:
			strcpy ((char*)abText, "25MHz\n");
			break;
		case PMC_50:
			strcpy ((char*)abText, "50MHz\n");
			break;
		case PMC_33:
			strcpy ((char*)abText, "33MHz\n");
			break;
		case PMC_66:
			strcpy ((char*)abText, "66MHz\n");
			break;
		case PMCX_50:
			strcpy ((char*)abText, "50MHz\n");
			break;
		case PMCX_66:
			strcpy ((char*)abText, "66MHz\n");
			break;
		case PMCX_100:
			strcpy ((char*)abText, "100MHz\n");
			break;
		case PMCX_133:
			strcpy ((char*)abText, "133MHz\n");
			break;
	}
}


void brdCuteInfo(brd_info *info)
{
	UINT8	reg;
	PCI_PFA	pfa;

	pfa = PCI_MAKE_PFA (0, 0, 0);
	info->memory = dPciReadReg (pfa, 0xB0, REG_16);

	reg = ((UINT8)dIoReadReg(0x210, REG_8));
	info->HardwareRev = ('A' + ((reg >> 5) & 0x07));

	info->firmwareVersion = FW_VERSION_NUMBER;
}



