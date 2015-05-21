 
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vx91x.01x/DispConfig.c,v 1.1 2014-05-15 09:42:03 jthiru Exp $
 * $Log: DispConfig.c,v $
 * Revision 1.1  2014-05-15 09:42:03  jthiru
 * Adding vx91x board sources
 *
 * Revision 1.2  2014-04-22 16:43:50  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.1  2013-11-28 15:13:54  mgostling
 * Add support for VX813.09x
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.4  2011/06/07 15:04:34  hchalla
 * Initial version of PBIT for VX813.
 *
 * Revision 1.3  2011/05/16 14:30:16  hmuneer
 * Info Passing Support
 *
 * Revision 1.2  2011/03/22 13:20:03  hchalla
 * Modified Dispconfig.c to display the board configuration.
 *
 * Revision 1.1  2011/02/28 11:57:43  hmuneer
 * no message
 * 
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
#include "version.h"
#include "cctboard.h"

#if 0
/* defines */

//#define DEBUG
#define SCR_ERROR		(E__BIT + 0x1000)
#define  GPIO_BASE	    0x48
#define LPC_BRIDGE_DEV	31
#define GP_LVL2         0x38
/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

extern UINT8	bCpuCount;
extern void 	vGetEthMacAddr(	UINT8  bInstance, UINT8* pbAddress);

/* forward declarations */


static PTR48 tPtr1;
static volatile UINT32 mapRegSpace=0;
static UINT32 mHandle;
/*****************************************************************************
 * DispConfig: Display board configuration
 * RETURNS: None
 * */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
	UINT32	temp32, addr, rt = E__OK;
	UINT16	mem;
	UINT8	reg, temp;	
	char	achBuffer[80];	/* text formatting buffer */
	PCI_PFA	pfa;
//	UINT64 dMem = 0;
	UINT32 dTemp = 0;
	UINT16 wGpioBase = 0;
	UINT8 bVal = 0;
	UINT8 bLanAddress[6];


	pfa = PCI_MAKE_PFA (0, 0, 0);
	temp32 = dPciReadReg(pfa, 0x00, REG_32);

	if( temp32 == 0x2a408086 )
	{		
		vConsoleWrite("GS45\n");				
		mem = dPciReadReg (pfa, 0xa2, REG_16)/1024;
	}
	else if(temp32 == 0x00448086 || temp32 == 0x01008086)
	{
		vConsoleWrite("Ibex Peak (or) Sandy Bridge\n");
		addr = dPciReadReg (pfa, 0x48, REG_32);
		addr &= 0xfffffff0;

		mHandle = dGetPhysPtr(addr,0x1000,&tPtr1,(void*)&mapRegSpace);

		if(mHandle == E__FAIL)
		{
			return E__FAIL;
		}

		mem =  (((*((UINT16*)(mapRegSpace+0x206))) & 0x03ff)*64)/1024;
		mem +=  (((*((UINT16*)(mapRegSpace+0x606))) & 0x03ff)*64)/1024;

		vFreePtr(mHandle);
	}
	else if (temp32 == 0x01048086)
	{
	    pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
		wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
		dTemp = dIoReadReg (wGpioBase+ GP_LVL2, REG_32);
		if ( ((dTemp >> 17) & (0x01)) == 0x01)
		{
			bVal = (0x00) | (0x1);
		}
		else if ( ((dTemp >> 18) & (0x01)) == 0x01)
		{
		    bVal = (bVal) | (0x2);
		}
		else if ( ((dTemp >> 20) & (0x01)) == 0x01)
		{
		     bVal = (bVal)| (0x4);
		}
		if ( ((dTemp >> 17) & (0x01)) == 0x00)
		{
       	    bVal = (bVal) & (0xFE);
		}
		else if ( ((dTemp >> 18) & (0x01)) == 0x00)
		{
		    bVal = (bVal) & (0xFD);
		}
		else if ( ((dTemp >> 20) & (0x01)) == 0x00)
		{
		     bVal = (bVal) & (0xFC);
		}

		switch (bVal)
		{
			case 0:
//				dMem = ((UINT64)4 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);
				vConsoleWrite("4GB Fitted\n");
				mem = 4;
				break;
			case 1:
//				dMem = ((UINT64)8 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);
				vConsoleWrite("8GB Fitted\n");
				mem = 8;
				break;
			case 2:
//				dMem = ((UINT64)16 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);
				vConsoleWrite("16GB Fitted\n");
				mem = 16;
				break;
			case 4:
//				dMem = ((UINT64)2 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);
				vConsoleWrite("2GB Fitted\n");
				mem = 2;
				break;
			default:
//				dMem = 0;
				mem = 0;
				vConsoleWrite("0GB Fitted\n");
				break;
		}
	}
	else
	{
		vConsoleWrite("other\n");
		mem = PCI_READ_WORD (pfa, 0xcc);	
		mem = mem & 0x1ff;
		mem = (mem * 128)/1024;
	}

	sprintf (achBuffer, "Num CPUs               : %d\n", (int)bCpuCount);
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "CPU Frequency          : %i.%02iGHz\n", sysGetCpuFrequency () / 1000,
															 (sysGetCpuFrequency () % 1000) / 10);
	vConsoleWrite (achBuffer);	
	sprintf (achBuffer, "Fitted RAM             : %dGB\n", mem);
	vConsoleWrite (achBuffer);	


	reg = ((UINT8)dIoReadReg(0x210, REG_8));
	sprintf (achBuffer, "Hardware Revision      : %c\n", ('A' +  ((reg >> 5)  & 0x07)) );
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Mode                   : %s\n", ((reg & 0x01) == 0x01?"BIOS":"CUTE"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "User/Test              : %s\n", ((reg & 0x02) == 0x02?"ON":"OFF"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Console Mode           : %s\n", ((reg & 0x04) == 0x04?"Serial":"VGA"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "COM Port Select        : %s\n", ((reg & 0x08) == 0x08?"COM2":"COM1"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Register Unlock Stat   : %s\n", ((reg & 0x10) == 0x10?"Register Unlocked":"Register Locked"));
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x211, REG_8));
	sprintf (achBuffer, "PMC1 Present           : %s\n", ((reg & 0x01) == 0x01?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "PMC2 Present           : %s\n", ((reg & 0x02) == 0x02?"Yes":"No"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "PMC/XMC Init Status    : %s\n", ((reg & 0x04) == 0x04?"Ready":"Not Ready"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC1 Present           : %s\n", ((reg & 0x08) == 0x08?"Yes":"NO"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "XMC2 Present           : %s\n", ((reg & 0x10) == 0x10?"Yes":"No"));
	vConsoleWrite (achBuffer);

	temp = (reg >> 5) & 0x07;
	switch ( temp )
	{
		case 0x00:
			sprintf (achBuffer, "PMC Frequency          : PCI 25 MHz\n");
			break;

		case 0x01:
			sprintf (achBuffer, "PMC Frequency          : PCI 50 MHz\n");
			break;

		case 0x02:
			sprintf (achBuffer, "PMC Frequency          : PCI-X 50 MHz\n");
			break;

		case 0x03:
			sprintf (achBuffer, "PMC Frequency          : PCI-X 100 MHz\n");
			break;

		case 0x04:
			sprintf (achBuffer, "PMC Frequency          : PCI 33 MHz\n");
			break;

		case 0x05:
			sprintf (achBuffer, "PMC Frequency          : PCI 66 MHz\n");
			break;

		case 0x06:
			sprintf (achBuffer, "PMC Frequency          : PCI-X 66 MHz\n");
			break;

		default:
			sprintf (achBuffer, "PMC Frequency          : UNKNOWN\n");
			break;
	}
	vConsoleWrite (achBuffer);


	reg = ((UINT8)dIoReadReg(0x212, REG_8));

	sprintf (achBuffer, "Watchdog NMI Event     : %s\n", ((reg & 0x01) == 0x01?"Occurred":"Not Occurred"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Push Button NMI Gen    : %s\n", ((reg & 0x02) == 0x02?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VME Backplane Reset    : %s\n", ((reg & 0x08) == 0x08?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "BDFAIL Signal Asserted : %s\n", ((reg & 0x10) == 0x10?"High":"Low"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "VME Sys Reset Signal   : %s\n", ((reg & 0x20) == 0x20?"Asserted":"False"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "TSI 148 NMI Event      : %s\n", ((reg & 0x40) == 0x40?"Occurred":"Not Occurred"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Front Panel NMI Event  : %s\n", ((reg & 0x80) == 0x80?"Occurred":"Not Occurred"));
	vConsoleWrite (achBuffer);

	reg = ((UINT8)dIoReadReg(0x213, REG_8));
	sprintf (achBuffer, "Watchdog Timeout Action: %s\n", ((reg & 0x04) == 0x04?"NMI":"Reset"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Watchdog Status        : %s\n", ((reg & 0x08) == 0x08?"Asserted":"Not Asserted"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Watchdog Control Status: %s\n", ((reg & 0x10) == 0x10?"Disabled In Hardware":"Under Software Control"));
	vConsoleWrite (achBuffer);
	sprintf (achBuffer, "Watchdog Software      : %s\n", ((reg & 0x20) == 0x20?"Enabled":"Disabled"));
	vConsoleWrite (achBuffer);

	/*
	 *  Write the Unlock sequence for reading 0x31x registers.
	 */
	vIoWriteReg(0x210, REG_8,0x43);
	vIoWriteReg(0x210, REG_8,0x43);
	vIoWriteReg(0x210, REG_8,0x54);
	vDelay(1);
	vIoWriteReg(0x210, REG_8,0x43);
	vIoWriteReg(0x210, REG_8,0x43);
	vIoWriteReg(0x210, REG_8,0x54);

	reg = ((UINT8)dIoReadReg(0x31F, REG_8));
	sprintf (achBuffer, "FPGA Revision          : %c\n", ('A'+reg));
	vConsoleWrite (achBuffer);
	/*
	 *  Lock again the registers 0x31x register.
	 */
	vIoWriteReg(0x210, REG_8,0x43);

	vConsoleWrite ("\nETHERNET\n-------------------\n");
	vGetEthMacAddr(1, bLanAddress);
	sprintf (achBuffer, "Ethernet 1 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

	vGetEthMacAddr(2, bLanAddress);
	sprintf (achBuffer, "Ethernet 2 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

	vGetEthMacAddr(3, bLanAddress);
	sprintf (achBuffer, "Ethernet 3 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

	vGetEthMacAddr(4, bLanAddress);
	sprintf (achBuffer, "Ethernet 4 MAC          : %02X %02X %02X %02X %02X %02X\n",
			 bLanAddress[0], bLanAddress[1], bLanAddress[2],
			 bLanAddress[3], bLanAddress[4], bLanAddress[5]);
	vConsoleWrite (achBuffer);

	return rt;
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
#endif


/* defines */

//#define DEBUG

#define SCR_ERROR               (E__BIT + 0x1000)
#define  GPIO_BASE          0x48
#define LPC_BRIDGE_DEV  31
#define GP_LVL2         0x38
#define GPIO_SEL2       0x34
#define GPIO49    0x20000
#define GPIO50    0x40000
#define GPIO52    0x100000
#define NO_OF_ETH_PORTS  4
/* typedefs */

/* constants */

/* locals */

/* globals */
#define LPC_BRIDGE_DEV  31
#define GPIO_BASE_ADDR  0x48

/* externals */

extern UINT8    bCpuCount;
extern void     vGetEthMacAddr( UINT8  bInstance, UINT8* pbAddress,UINT8 bMode);
extern UINT32 dGetNumberOfEthInstances(void);
/* forward declarations */


static PTR48 tPtr1;
static volatile UINT32 mapRegSpace=0;
static UINT32 mHandle;
/*****************************************************************************
 * DispConfig: Display board configuration
 * RETURNS: None
 * */
TEST_INTERFACE (DispConfig, "Display Board Configuration")
{
        UINT32  temp32, addr, rt = E__OK, reg32;
        UINT16  mem;
        UINT8   reg, temp;
        char    achBuffer[80];  /* text formatting buffer */
        PCI_PFA pfa;
        UINT32 dMem = 0,dTemp = 0;
        UINT16 wGpioBase = 0;
        UINT8 bVal = 0;
        UINT8 bLanAddress[6];


        pfa = PCI_MAKE_PFA (0, 0, 0);
        temp32 = dPciReadReg(pfa, 0x00, REG_32);

        if( temp32 == 0x2a408086 )
        {
                vConsoleWrite("GS45\n");
                mem = dPciReadReg (pfa, 0xa2, REG_16)/1024;
        }
        else if(temp32 == 0x00448086 || temp32 == 0x01008086)
        {
                vConsoleWrite("Ibex Peak (or) Sandy Bridge\n");
                addr = dPciReadReg (pfa, 0x48, REG_32);
                addr &= 0xfffffff0;

                mHandle = dGetPhysPtr(addr,0x1000,&tPtr1,(void*)&mapRegSpace);

                if(mHandle == E__FAIL)
                {
                        return E__FAIL;
                }

                mem =  (((*((UINT16*)(mapRegSpace+0x206))) & 0x03ff)*64)/1024;
                mem +=  (((*((UINT16*)(mapRegSpace+0x606))) & 0x03ff)*64)/1024;

                vFreePtr(mHandle);
        }
        else if (temp32 == 0x01048086)
        {
            pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
                wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);
                dTemp = dIoReadReg (wGpioBase+ GP_LVL2, REG_32);
                if ( ((dTemp >> 17) & (0x01)) == 0x01)
                {
                        bVal = (0x00) | (0x1);
                }
                else if ( ((dTemp >> 18) & (0x01)) == 0x01)
                {
                    bVal = (bVal) | (0x2);
                }
                else if ( ((dTemp >> 20) & (0x01)) == 0x01)
                {
                     bVal = (bVal)| (0x4);
                }
                if ( ((dTemp >> 17) & (0x01)) == 0x00)
                {
            bVal = (bVal) & (0xFE);
                }
                else if ( ((dTemp >> 18) & (0x01)) == 0x00)
                {
                    bVal = (bVal) & (0xFD);
                }
                else if ( ((dTemp >> 20) & (0x01)) == 0x00)
                {
                     bVal = (bVal) & (0xFC);
                }

                switch (bVal)
                {
                        case 0:
                                dMem = ((UINT64)4 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);
                                vConsoleWrite("4GB Fitted\n");
                                mem = 4;
                                break;
                        case 1:
                                dMem = ((UINT64)8 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);
                                vConsoleWrite("8GB Fitted\n");
                                mem = 8;
                                break;
                        case 2:
                                dMem = ((UINT64)16 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);
                                vConsoleWrite("16GB Fitted\n");
                                mem = 16;
                                break;
                        case 4:
                                dMem = ((UINT64)2 * (UINT64)1024 * (UINT64)1024 * (UINT64)1024);
                                vConsoleWrite("2GB Fitted\n");
                                mem = 2;
                                break;
                        default:
                                dMem = 0;
                                mem = 0;
                                vConsoleWrite("0GB Fitted\n");
                                break;
                }
        }
        else if(temp32 == 0x01548086)
        {
                vConsoleWrite("Ivy Bridge\n");

                //Determine Memory Fitted

                //read GIO base
                pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
                wGpioBase = PCI_READ_WORD(pfa, GPIO_BASE_ADDR);
                wGpioBase &= 0xFFFE;

                reg32 = dIoReadReg(wGpioBase + GPIO_SEL2, REG_32);
#ifdef DEBUG
                sprintf (achBuffer, "GPIO SEL: %#x\n", reg32);
                vConsoleWrite (achBuffer);
#endif
                vIoWriteReg (wGpioBase + GPIO_SEL2, REG_32,
                    (reg32 | GPIO49 | GPIO50 | GPIO52));
#ifdef DEBUG
                sprintf (achBuffer, "GPIO SEL: %#x\n", reg32);
                vConsoleWrite (achBuffer);
#endif
                //Read GPIO pins 49, 50, and 52
                reg32 = dIoReadReg(wGpioBase + GP_LVL2, REG_32);
#ifdef DEBUG
                sprintf (achBuffer, "GPIO VAL: %#x\n", reg32);
                vConsoleWrite (achBuffer);
#endif
                reg32 = (((reg32 & GPIO52)?1:0)|
                        (((reg32 & GPIO50)?1:0) << 1)|
                        (((reg32 & GPIO49)?1:0) << 2));
#ifdef DEBUG
                sprintf (achBuffer, "RAM Info: %#x\n", reg32);
                vConsoleWrite (achBuffer);
#endif
                switch(reg32)
                {
                        case 0:
                                mem = 4;//4GB
                                break;
                        case 1:
                                mem = 2;//2GB
                                break;
                        case 2:
                                mem = 16;//16GB
                                break;
                        case 4:
                                mem = 8;//2GB
                                break;
                        default:
                                mem = 0;//Unknown
                                break;
                }

        }
        else
        {
                vConsoleWrite("other\n");
                mem = PCI_READ_WORD (pfa, 0xcc);
                mem = mem & 0x1ff;
                mem = (mem * 128)/1024;
        }

        sprintf (achBuffer, "Num CPUs               : %d\n", (int)bCpuCount);
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "CPU Frequency          : %i.%02iGHz\n", sysGetCpuFrequency () / 1000,
                                                                                                                         (sysGetCpuFrequency () % 1000) / 10);
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Fitted RAM             : %dGB\n", mem);
        vConsoleWrite (achBuffer);


        reg = ((UINT8)dIoReadReg(0x210, REG_8));
        sprintf (achBuffer, "Hardware Revision      : %c\n", ('A' +  ((reg >> 5)  & 0x07)) );
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Mode                   : %s\n", ((reg & 0x01) == 0x01?"BIOS":"CUTE"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "User/Test              : %s\n", ((reg & 0x02) == 0x02?"ON":"OFF"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Console Mode           : %s\n", ((reg & 0x04) == 0x04?"Serial":"VGA"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "COM Port Select        : %s\n", ((reg & 0x08) == 0x08?"COM2":"COM1"));
        vConsoleWrite (achBuffer);
        /*sprintf (achBuffer, "Register Unlock Stat   : %s\n", ((reg & 0x10) == 0x10?"Register Unlocked":"Register Locked"));
        vConsoleWrite (achBuffer);*/


        reg = ((UINT8)dIoReadReg(0x211, REG_8));
        sprintf (achBuffer, "PMC1 Present           : %s\n", ((reg & 0x01) == 0x01?"Yes":"No"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "PMC2 Present           : %s\n", ((reg & 0x02) == 0x02?"Yes":"No"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "PMC/XMC Init Status    : %s\n", ((reg & 0x04) == 0x04?"Ready":"Not Ready"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "XMC1 Present           : %s\n", ((reg & 0x08) == 0x08?"Yes":"NO"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "XMC2 Present           : %s\n", ((reg & 0x10) == 0x10?"Yes":"No"));
        vConsoleWrite (achBuffer);

        temp = (reg >> 5) & 0x07;
        switch ( temp )
        {
                case 0x00:
                        sprintf (achBuffer, "PMC Frequency          : PCI 25 MHz\n");
                        break;

                case 0x01:
                        sprintf (achBuffer, "PMC Frequency          : PCI 50 MHz\n");
                        break;

                case 0x02:
                        sprintf (achBuffer, "PMC Frequency          : PCI-X 50 MHz\n");
                        break;

                case 0x03:
                        sprintf (achBuffer, "PMC Frequency          : PCI-X 100 MHz\n");
                        break;

                case 0x04:
                        sprintf (achBuffer, "PMC Frequency          : PCI 33 MHz\n");
                        break;

                case 0x05:
                        sprintf (achBuffer, "PMC Frequency          : PCI 66 MHz\n");
                        break;

                case 0x06:
                        sprintf (achBuffer, "PMC Frequency          : PCI-X 66 MHz\n");
                        break;

                default:
                        sprintf (achBuffer, "PMC Frequency          : UNKNOWN\n");
                        break;
        }
        vConsoleWrite (achBuffer);


        reg = ((UINT8)dIoReadReg(0x212, REG_8));

        sprintf (achBuffer, "Watchdog NMI Event     : %s\n", ((reg & 0x01) == 0x01?"Occurred":"Not Occurred"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Push Button NMI Gen    : %s\n", ((reg & 0x04) == 0x04?"Enabled":"Disabled"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "VME Backplane Reset    : %s\n", ((reg & 0x02) == 0x02?"Enabled":"Disabled"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "BDFAIL Signal Asserted : %s\n", ((reg & 0x10) == 0x10?"High":"Low"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "VME Sys Reset Signal   : %s\n", ((reg & 0x20) == 0x20?"Asserted":"False"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "TSI 148 NMI Event      : %s\n", ((reg & 0x40) == 0x40?"Occurred":"Not Occurred"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Front Panel NMI Event  : %s\n", ((reg & 0x80) == 0x80?"Occurred":"Not Occurred"));
        vConsoleWrite (achBuffer);

        reg = ((UINT8)dIoReadReg(0x213, REG_8));
        sprintf (achBuffer, "Watchdog Timeout Action: %s\n", ((reg & 0x04) == 0x04?"Reset":"NMI"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Watchdog Status        : %s\n", ((reg & 0x08) == 0x08?"Timed out":"OK"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Watchdog Control Status: %s\n", ((reg & 0x10) == 0x10?"Disabled In Hardware":"Under Software Control"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Watchdog Software      : %s\n", ((reg & 0x20) == 0x20?"Enabled":"Disabled"));
        vConsoleWrite (achBuffer);
        sprintf (achBuffer, "Watchdog System RESET  : %s\n", ((reg & 0x40) == 0x40?"Local & Sys":"Local only"));
        vConsoleWrite (achBuffer);

        /*
         *  Write the Unlock sequence for reading 0x31x registers.
         */
        vIoWriteReg(0x210, REG_8,0x43);
        vIoWriteReg(0x210, REG_8,0x43);
        vIoWriteReg(0x210, REG_8,0x54);
        vDelay(1);
        vIoWriteReg(0x210, REG_8,0x43);
        vIoWriteReg(0x210, REG_8,0x43);
        vIoWriteReg(0x210, REG_8,0x54);

        reg = ((UINT8)dIoReadReg(0x31F, REG_8));
        sprintf (achBuffer, "FPGA Revision          : %c\n", ('A'+reg));
        vConsoleWrite (achBuffer);
        /*
         *  Lock again the registers 0x31x register.
         */
        vIoWriteReg(0x210, REG_8,0x43);


        vConsoleWrite ("\nETHERNET\n-------------------\n");
        if(dGetNumberOfEthInstances() < NO_OF_ETH_PORTS)
        {
            /* Obtain First Instance MAC address */
            vGetEthMacAddr(1, bLanAddress, ETH_MAC_DISP_MODE);
            /* Display for second instance - no P0 indeed */
            bLanAddress [5]++;
                if (bLanAddress [5] == 0)
                {
                    bLanAddress [4]++;
                        if (bLanAddress [4] == 0)
                          bLanAddress [3]++;
                }
            sprintf (achBuffer, "Ethernet MAC          : %02X %02X %02X %02X %02X %02X\n",
                             bLanAddress[0], bLanAddress[1], bLanAddress[2],
                             bLanAddress[3], bLanAddress[4], bLanAddress[5]);
            vConsoleWrite (achBuffer);
        }
        else
        {
            vGetEthMacAddr(1, bLanAddress, ETH_MAC_DISP_MODE);
            sprintf (achBuffer, "Ethernet 1 MAC          : %02X %02X %02X %02X %02X %02X\n",
                             bLanAddress[0], bLanAddress[1], bLanAddress[2],
                             bLanAddress[3], bLanAddress[4], bLanAddress[5]);
            vConsoleWrite (achBuffer);

            vGetEthMacAddr(2, bLanAddress, ETH_MAC_DISP_MODE);
            sprintf (achBuffer, "Ethernet 2 MAC          : %02X %02X %02X %02X %02X %02X\n",
                             bLanAddress[0], bLanAddress[1], bLanAddress[2],
                             bLanAddress[3], bLanAddress[4], bLanAddress[5]);
            vConsoleWrite (achBuffer);

            vGetEthMacAddr(3, bLanAddress, ETH_MAC_DISP_MODE);
            sprintf (achBuffer, "Ethernet 3 MAC          : %02X %02X %02X %02X %02X %02X\n",
                             bLanAddress[0], bLanAddress[1], bLanAddress[2],
                             bLanAddress[3], bLanAddress[4], bLanAddress[5]);
            vConsoleWrite (achBuffer);

            vGetEthMacAddr(4, bLanAddress, ETH_MAC_DISP_MODE);
            sprintf (achBuffer, "Ethernet 4 MAC          : %02X %02X %02X %02X %02X %02X\n",
                             bLanAddress[0], bLanAddress[1], bLanAddress[2],
                             bLanAddress[3], bLanAddress[4], bLanAddress[5]);
            vConsoleWrite (achBuffer);
        }



        return rt;
}


void brdCuteInfo(brd_info *info)
{
        UINT8   reg;
        PCI_PFA pfa;

        pfa = PCI_MAKE_PFA (0, 0, 0);
        info->memory = dPciReadReg (pfa, 0xB0, REG_16);

        reg = ((UINT8)dIoReadReg(0x210, REG_8));
        info->HardwareRev = ('A' + ((reg >> 5) & 0x07));

        info->firmwareVersion = FW_VERSION_NUMBER;
}



