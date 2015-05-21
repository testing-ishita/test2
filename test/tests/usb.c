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

/* usb.c - To test USB interface 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/usb.c,v 1.4 2014-05-12 16:10:15 jthiru Exp $
 *
 * $Log: usb.c,v $
 * Revision 1.4  2014-05-12 16:10:15  jthiru
 * Adding support for VX91x and fixes to existing bugs
 *
 * Revision 1.3  2013-11-25 13:13:37  mgostling
 * Added support for USB3 on TRB1x.
 *
 * Revision 1.2  2013-10-08 07:13:39  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:45  chippisley
 * Import files into new source repository.
 *
 * Revision 1.9  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.8  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.7  2010/10/06 10:13:31  jthiru
 * PBIT coding for VP717
 *
 * Revision 1.6  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.5  2010/02/26 10:41:23  jthiru
 * Debug prints in #ifdef DEBUG
 *
 * Revision 1.4  2009/06/04 07:12:00  swilson
 * Fix test name missed in tidy-up
 *
 * Revision 1.3  2009/06/03 08:30:58  cvsuser
 * Tidy up test names
 *
 * Revision 1.2  2009/05/21 13:36:26  cvsuser
 * Added support to execute test without parameters
 *
 * Revision 1.1  2009/05/15 11:13:03  jthiru
 * Initial checkin for USB tests
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
#include <bit/hal.h>
 
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>
#include <devices/usb.h>


/* defines */

//#define DEBUG
#ifdef DEBUG
		static char buffer[128];
#endif

/* constants */

/* locals */

/* globals */
static UINT32 dhandle1, dhandle2;
static PTR48 ptr1, ptr2;

/* externals */
extern UINT32 dGetPhysPtr (UINT32 dBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);
extern void vConsoleWrite(char*	achMessage);
extern void vFreePtr(UINT32 dHandle);
extern void vDelay(UINT32 dMsDelay);

/* forward declarations */


/*******************************************************************
 * dInitUsb: Find the USB controller instance and initialise the
 * memory, I/O and Bus mastering
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dInitUsb
(
	DEVICE *psDev,
	UINT8 bInstance
)
{
    UINT32	dFound;
    UINT16	wPciCmd;

	PCI_PFA pfa;
	psDev->wUSBIoRegs = 0;
	psDev->wUSBMemRegs = 0;

	/* find the USB controller by class code */
    dFound = iPciFindDeviceByClass (bInstance, 0x0C, 0x03, &pfa);
    if (dFound == E__DEVICE_NOT_FOUND)
	{
		return dFound;
	}
	psDev->bBus = PCI_PFA_BUS(pfa);
	psDev->bDev = PCI_PFA_DEV(pfa);
	psDev->bFunc = PCI_PFA_FUNC(pfa);
	
	// 0x30:XHCI, 0x20:EHCI, 0x00:UHCI
	psDev->bCtrlType = (UINT8) dPciReadReg(pfa, 0x09, REG_8);

#ifdef DEBUG
	sprintf(buffer, "B: %d, D: %d, F: %d\n", PCI_PFA_BUS(pfa), PCI_PFA_DEV(pfa), PCI_PFA_FUNC(pfa));
	vConsoleWrite(buffer);
	sprintf(buffer, "Type: 0x%x\n", psDev->bCtrlType);
	vConsoleWrite(buffer);
#endif
	/* make sure the device IO, memory, BMDMA access is enabled */
	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	wPciCmd = (UINT16)dPciReadReg (pfa, 0x04, REG_16);
	vPciWriteReg(pfa, 0x04, REG_16, wPciCmd | 0x07);
		
	if(dPciReadReg(pfa, 0x10, REG_32) == 0x00)
	{
		psDev->wUSBIoRegs = dPciReadReg(pfa, 0x20, REG_16) & 0xFFFE;
	}
	else
		psDev->wUSBMemRegs = dPciReadReg(pfa, 0x10, REG_32) & 0xFFFFFFF0;
		
	if(psDev->wUSBMemRegs != 0)
	{
		dhandle1 = dGetPhysPtr(psDev->wUSBMemRegs, 0x10, &ptr1, &(psDev->caplogaddr));
		if(dhandle1 == E__FAIL)
		{
		#ifdef DEBUG
				vConsoleWrite("Unable to allocate Mem BAR memory \n"); 
		#endif
			return(E__USB_NO_MEM);
		}

		dhandle2 = dGetPhysPtr(psDev->wUSBMemRegs + (*(UINT8*)(psDev->caplogaddr)), 
					0x800, &ptr2, &(psDev->oplogaddr));
		if(dhandle2 == E__FAIL)
		{
		#ifdef DEBUG
				vConsoleWrite("Unable to allocate Mem BAR memory \n"); 
		#endif
			return(E__USB_NO_MEM);
		}
#ifdef DEBUG
				sprintf(buffer, "IO Regs: 0x%x, MEM BAR: 0x%x caplogadr: 0x%x oplogadr: 0x%x size: 0x%x\n", 
					psDev->wUSBIoRegs, psDev->wUSBMemRegs, (UINT32)(psDev->caplogaddr), (UINT32)(psDev->oplogaddr), 
					(*(UINT8*)(psDev->caplogaddr)));
				vConsoleWrite(buffer);
#endif
	}
	return (E__OK);
}


/*******************************************************************
 * dregaccesstest: Test the register access by writing ones and zeros
 * and reading back on a scratch pad register
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dregaccesstest
(
	DEVICE *psDev
)
{	
	PCI_PFA pfa=0;
	UINT16 wPciDid=0;
	
	
	if(psDev->wUSBIoRegs != 0)
	{
		vIoWriteReg(psDev->wUSBIoRegs + 0x4, REG_8, dIoReadReg(psDev->wUSBIoRegs + 0x4, REG_8) | (1 << 4));
		if(dIoReadReg(psDev->wUSBIoRegs + 0x4, REG_8) & (1 << 4))
		{
			vIoWriteReg(psDev->wUSBIoRegs + 0x4, REG_8, dIoReadReg(psDev->wUSBIoRegs + 0x4, REG_8) & ~(1 << 4));
			if(!(dIoReadReg(psDev->wUSBIoRegs + 0x4, REG_8) & (1 << 4)))
				return E__OK;
			else 
			{
				#ifdef DEBUG
					vConsoleWrite("Written 0 read 1 \n"); 
				#endif
				return E__USB_REG_ACCESS;
			}
		}
		else
		{
			#ifdef DEBUG
				vConsoleWrite("Written 1 read 0 \n"); 
			#endif
			return E__USB_REG_ACCESS;
		}
	}
	else
	{
		#ifdef DEBUG
			sprintf(buffer, "Mem BAR: 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
				(UINT32)(psDev->caplogaddr), (UINT32)(psDev->oplogaddr), 
				(UINT32)((UINT16*)(psDev->caplogaddr)),
				(UINT32)((UINT16*)(psDev->caplogaddr) + 0x1),
				(UINT32)(*((UINT16*)(psDev->caplogaddr) + 0x1)));
  			vConsoleWrite(buffer); 
		#endif

  		pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
  		wPciDid = (UINT16)dPciReadReg (pfa, 0x02, REG_16);
  		if (wPciDid == 0x1e31)
  		{
  			if ((*((UINT16*)(psDev->caplogaddr) + 0x1) == 0x800) || ((*((UINT16*)(psDev->caplogaddr) + 0x1) == 0x100)))
  		  	{
  		  		return E__OK;
  		  	}
  		  	else
  		  	{
  		  		return E__USB_REG_ACCESS;
  		  	}
  		}
		else
 		{
  			if (*((UINT16*)(psDev->caplogaddr) + 0x1) == 0x100)
  			{
  				return E__OK;
  			}
  			else
  			{
  				return E__USB_REG_ACCESS;
  			}
	    }
	}
}


/*******************************************************************
 * dusbsts: Report Port status changes
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dusbsts
(
	DEVICE *psDev
)
{
	UINT16	whp[10] = {0}, wspd[10] = {0}; 
	UINT32	dhp[10] = {0};
	UINT8 	i;
	UINT8	portCnt;
	UINT32  regAddr;
	UINT32	regVal;

	char buf[80];
	
	if(psDev->wUSBIoRegs != 0)
	{
		for(i = 0; i < 2; i++) /* No. of UHCI IO port is 2 per function*/
		{
			whp[i] = dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) & 0x2; 
			if(whp[i])
			{
				if(dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) & 0x1)
				{
					sprintf(buf, "Device Inserted in UHCI port: %d\n", i);
					vConsoleWrite(buf);
					vIoWriteReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16, dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) | 0x2);
				}
				else
				{
					sprintf(buf, "Device Removed from UHCI port: %d\n", i);
					vConsoleWrite(buf);
					vIoWriteReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16, dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) | 0x2);
				}
			}
			wspd[i] = dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) & 0x100;
			if(wspd[i])
			{
				sprintf(buf, "Low Speed device attached to UHCI port: %d\n", i);
				vConsoleWrite(buf);
			}
		}
#ifdef DEBUG
		sprintf(buffer, "Port 0 Sts: 0x%x Port 1 Sts: 0x%x\n", 
			dIoReadReg(psDev->wUSBIoRegs + 0x10, REG_16),
			dIoReadReg(psDev->wUSBIoRegs + 0x12, REG_16));
		vConsoleWrite(buffer);
#endif
	}
	else
	{
		if (psDev->bCtrlType == 0x30)	//XHCI
		{
			portCnt = (UINT8) (*((UINT32*)(psDev->caplogaddr) + 0x1) >> 24);
#ifdef DEBUG
			sprintf(buffer, "No Ports: 0x%x\n", portCnt);
			vConsoleWrite(buffer);
#endif
			regAddr = (UINT32)((UINT32*)(psDev->oplogaddr) + 0x100);
			for(i = 0; i < portCnt; i++) 
			{
				if (i > 0)
					regAddr += 0x10;
					
				regVal = *((UINT32 *) regAddr);
#ifdef DEBUG
				sprintf(buf, "Register address: %#X = %#X\n", regAddr, regVal);
				vConsoleWrite(buf);
#endif
				dhp[i] = regVal & 0x2; 
				if(dhp[i])
				{
					if(regVal & 0x1)
					{
						sprintf(buf, "Device Inserted in EHCI port: %d\n", i);
						vConsoleWrite(buf);
						*((UINT32*) regAddr) |= 0x2;	// this may not work from S/W 
					}
					else
					{
						sprintf(buf, "Device Removed from EHCI port: %d\n", i);
						vConsoleWrite(buf);
						*((UINT32*) regAddr) |= 0x2;	// this may not work from S/W
					}
				}
			}
		}
		else		// EHCI
		{
			portCnt = (UINT8) (*((UINT32*)(psDev->caplogaddr) + 0x1)  & 0xF);
			regAddr = (UINT32)((UINT32*)(psDev->oplogaddr) + 0x11);
#ifdef DEBUG
			sprintf(buffer, "No Ports: 0x%x\n", portCnt);
			vConsoleWrite(buffer);
#endif
			for(i = 0; i < portCnt; i++) 
			{
				if (i > 0)
					regAddr += 4;
					
				regVal = *((UINT32 *) regAddr);
#ifdef DEBUG
				sprintf(buf, "Register address: %#X = %#X\n", regAddr, regVal);
				vConsoleWrite(buf);
#endif
				dhp[i] = regVal & 0x2; 			// port enabled
				if(dhp[i])
				{
					if(regVal & 0x1)
					{
						sprintf(buf, "Device Inserted in EHCI port: %d\n", i);
						vConsoleWrite(buf);
						*((UINT32*) regAddr) |= 0x2; 			// this may not work from S/W
					}
					else
					{
						sprintf(buf, "Device Removed from EHCI port: %d\n", i);
						vConsoleWrite(buf);
						*((UINT32*) regAddr) |= 0x2; 			// this may not work from S/W 
					}
				}
			}
/*		
#ifdef DEBUG
		sprintf(buffer, "No Port: 0x%x Port 0 Sts: 0x%x Port 1 Sts: 0x%x Port 2: 0x%x Port 3 :0x%x\n",
			*((UINT32*)(psDev->caplogaddr) + 0x1) & 0xF,
			*((UINT32*)(psDev->oplogaddr) + 0x11),
			*((UINT32*)(psDev->oplogaddr) + 0x12),
			*((UINT32*)(psDev->oplogaddr) + 0x13),
			*((UINT32*)(psDev->oplogaddr) + 0x14));
		vConsoleWrite(buffer);
#endif
*/
		}
	}
	return E__OK;
}


/*******************************************************************
 * dusbocsts: Report Port status changes
 *
 * RETURNS: E__OK or an E__... BIT error code
 */

static UINT32 dusbocsts
(
	DEVICE *psDev
)
{
	UINT16	woc[10]; 
	UINT32	doc[32];
	UINT8 i;
	UINT8 portCnt;

#ifdef DEBUG
	char buf[80];
#endif
	UINT32 dtest_status;

	memset (woc, sizeof(woc), 0);
	memset (doc, sizeof(doc), 0);

	if(psDev->wUSBIoRegs != 0)
	{
		for(i = 0; i < 2; i++) /* No. of UHCI IO port is 2 per function*/
		{
			woc[i] = dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) & 0x800;
			if(woc[i])
			{
				if(dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) & 0x400)
				{
#ifdef DEBUG
					sprintf(buf, "Over current Indicated in UHCI port: %d\n", i);
					vConsoleWrite(buf);
#endif
					dtest_status = E__USB_OVERCURRENT;
				}
				else
				{
#ifdef DEBUG
					sprintf(buf, "No Over current Indicated in UHCI port: %d\n", i);
					vConsoleWrite(buf);
#endif
					dtest_status = E__OK;
				}
				vIoWriteReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16, dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) | 0x800);
			}
			else
			{
				if(dIoReadReg(psDev->wUSBIoRegs + 0x10 + i*2, REG_16) & 0x400)
				{
#ifdef DEBUG
					sprintf(buf, "Over current Indicated in UHCI port: %d\n", i);
					vConsoleWrite(buf);
#endif
					dtest_status = E__USB_OVERCURRENT;
				}
				else
				{
#ifdef DEBUG
					sprintf(buf, "No Over current Indicated in UHCI port: %d\n", i);
					vConsoleWrite(buf);
#endif
					dtest_status = E__OK;
				}
			}		
		}			
#ifdef DEBUG
		sprintf(buffer, "Port 0 Sts: 0x%x Port 1 Sts: 0x%x 0x%x\n",
			dIoReadReg(psDev->wUSBIoRegs + 0x10, REG_16),
			dIoReadReg(psDev->wUSBIoRegs + 0x12, REG_16), dtest_status);
		vConsoleWrite(buffer);
#endif
	}
	else
	{
	    if (psDev->bCtrlType == 0x30)       //XHCI
	    {
			portCnt = *((UINT32*)(psDev->caplogaddr) + 0x1) >> 24;
#ifdef DEBUG
            sprintf(buf, "XHCI portCnt: %d\n", portCnt);
            vConsoleWrite(buf);
#endif
	        for(i = 0; i < portCnt; i++)
	        {
	            doc[i] = *((UINT32*)(psDev->oplogaddr) + 0x100 + (i * 0x10)) & 0x100000;
	            if(doc[i])
                {
                    if(*((UINT32*)(psDev->oplogaddr) + 0x100 + (i * 0x10)) & 0x8)
                    {
#ifdef DEBUG
                        sprintf(buf, "Over Current Condition in XHCI port: %d\n", i);
                        vConsoleWrite(buf);
#endif
                        dtest_status = E__USB_OVERCURRENT;
                        *((UINT32*)(psDev->oplogaddr) + 0x100 + (i * 0x10)) |= 0x100000;
                    }
                    else
                    {
#ifdef DEBUG
                        sprintf(buf, "No Over Current Condition in XHCI port: %d\n", i);
                        vConsoleWrite(buf);
#endif
                        dtest_status = E__OK;
                        *((UINT32*)(psDev->oplogaddr) + 0x100 + (i * 0x10)) |= 0x100000;
                    }
                }
	            else
                {
                    if(*((UINT32*)(psDev->oplogaddr) + 0x100 + (i * 0x10)) & 0x8)
                    {
#ifdef DEBUG
                        sprintf(buf, "Over Current Condition in XHCI port: %d\n", i);
                        vConsoleWrite(buf);
#endif
                        dtest_status = E__USB_OVERCURRENT;
                    }
                    else
                    {
#ifdef DEBUG
                        sprintf(buf, "No Over Current Condition in XHCI port: %d\n", i);
                        vConsoleWrite(buf);
#endif
                        dtest_status = E__OK;
                    }
                }
	        }
	    }
	    else // EHCI
	    {
			for(i = 0; i < (*((UINT32*)(psDev->caplogaddr) + 0x1) & 0xF); i++) 
			{
				doc[i] = *((UINT32*)(psDev->oplogaddr) + 0x11 + i) & 0x20; 
				if(doc[i])
				{
					if(*((UINT32*)(psDev->oplogaddr) + 0x11 + i) & 0x10)
					{
#ifdef DEBUG
						sprintf(buf, "Over Current Condition in EHCI port: %d\n", i);
						vConsoleWrite(buf);
#endif
						dtest_status = E__USB_OVERCURRENT;
						*((UINT32*)(psDev->oplogaddr) + 0x11 + i) |= 0x20;
					}
					else
					{
#ifdef DEBUG
						sprintf(buf, "No Over Current Condition in EHCI port: %d\n", i);
						vConsoleWrite(buf);
#endif
						dtest_status = E__OK;
						*((UINT32*)(psDev->oplogaddr) + 0x11 + i) |= 0x20;
					}
				}
				else
				{
					if(*((UINT32*)(psDev->oplogaddr) + 0x11 + i) & 0x10)
					{
#ifdef DEBUG
						sprintf(buf, "Over Current Condition in EHCI port: %d\n", i);
						vConsoleWrite(buf);
#endif
						dtest_status = E__USB_OVERCURRENT;
					}
					else
					{
#ifdef DEBUG
						sprintf(buf, "No Over Current Condition in EHCI port: %d\n", i);
						vConsoleWrite(buf);
#endif
						dtest_status = E__OK;
					}
				}
			}
	    }
#ifdef DEBUG
		sprintf(buffer, "Port 0 Sts: 0x%x Port 1 Sts: 0x%x Port 2: 0x%x Port 3 :0x%x 0x%x \n",
			*((UINT32*)(psDev->oplogaddr) + 0x11),
			*((UINT32*)(psDev->oplogaddr) + 0x12),
			*((UINT32*)(psDev->oplogaddr) + 0x13),
			*((UINT32*)(psDev->oplogaddr) + 0x14), dtest_status);
		vConsoleWrite(buffer);
#endif
	}
	return dtest_status;
}


/*****************************************************************************
 * vClosePciDevice: restore system once test complete
 *
 * RETURNS: none
 */

static void vClosePciDevice
( 
	DEVICE *psDev 
)
{
	UINT32	wPciCmd;
	PCI_PFA pfa;

	/* make sure the device IO, memory, BMDMA access is disabled */
	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	wPciCmd = (UINT16)dPciReadReg (pfa, 0x04, REG_16);
	vPciWriteReg (pfa, 0x04, REG_16, wPciCmd & 0xFFF8);

} /* vClosePciDevice () */


/*****************************************************************************
 * UsbRegTest: test function for accessing USB Registers
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (UsbRegTest, "USB Register Access Test")
{
	UINT32 dPciStatus;
	UINT32 dtest_status;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	DEVICE sDevice;	

	if(adTestParams[0])
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
	}
	else
	{
		bLoop = 1;				
		bInstance++;		
	}
	do {
#ifdef DEBUG
		sprintf(buffer, "Instance to find: %d\n", bInstance);
		vConsoleWrite(buffer); 
#endif
		dPciStatus = dInitUsb (&sDevice, bInstance);
		
		if (dPciStatus == E__OK)
		{			
			dtest_status = dregaccesstest (&sDevice);
#ifdef DEBUG
			sprintf(buffer, "Test status: 0x%x\n", dtest_status);
			vConsoleWrite(buffer); 
#endif				
		}

		else if ( (bLoop == 0) || 
				((bLoop == 1) && (bInstance == 1)) || 
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of USB Controller"); 
			#endif
			return (E__USB_NOT_FOUND);
		}

		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}

	return (dtest_status);

} /* UsbRegTest */


/*****************************************************************************
 * UsbDeviceCheck: test function for reporting USB device status changes
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (UsbDeviceCheck, "USB Device Status Reporting Test")
{
	UINT32 dPciStatus;
	UINT32 dtest_status = 0;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	DEVICE sDevice;	

	if(adTestParams[0])
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
	}
	else
	{
		bLoop = 1;				
		bInstance++;		
	}
	do {
#ifdef DEBUG
		sprintf(buffer, "Instance to find: %d\n", bInstance);
		vConsoleWrite(buffer); 
#endif
		dPciStatus = dInitUsb (&sDevice, bInstance);
		
		if (dPciStatus == E__OK)
		{			
			dtest_status = dusbsts (&sDevice);
#ifdef DEBUG
			sprintf(buffer, "Test status: 0x%x\n", dtest_status);
			vConsoleWrite(buffer); 
#endif				
		}

		else if ( (bLoop == 0) || 
				((bLoop == 1) && (bInstance == 1)) || 
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of USB Controller"); 
			#endif
			return (E__USB_NOT_FOUND);
		}

		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}

	return (dtest_status);

} /* UsbDeviceCheck */


/*****************************************************************************
 * UsbOCCheck: test function for reporting Over current presence in a port
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (UsbOCCheck, "USB Port Over Current Status")
{
	UINT32 dPciStatus;
	UINT32 dtest_status;
	UINT8 bInstance = 0, bLoop = 0, i = 1;
	DEVICE sDevice;	

	if(adTestParams[0])
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
	}
	else
	{
		bLoop = 1;				
		bInstance++;		
	}
	do {
#ifdef DEBUG
		sprintf(buffer, "Instance to find: %d\n", bInstance);
		vConsoleWrite(buffer); 
#endif
		dPciStatus = dInitUsb (&sDevice, bInstance);
		
		if (dPciStatus == E__OK)
		{			
			dtest_status = dusbocsts (&sDevice);
#ifdef DEBUG
			sprintf(buffer, "Test status: 0x%x\n", dtest_status);
			vConsoleWrite(buffer); 
#endif				
		}

		else if ( (bLoop == 0) || 
				((bLoop == 1) && (bInstance == 1)) || 
				((bLoop == 1) && (adTestParams[1] != 0)))
		{
			#ifdef DEBUG  
      			vConsoleWrite("Unable to locate instance of USB Controller"); 
			#endif
			return (E__USB_NOT_FOUND);
		}

		else		/* soft exit if cannot find controller in loop mode */
			bLoop = 0;

		if(dtest_status == E__OK)
		{
			if(adTestParams[1] == 0)
			{
				bInstance++;
			}
			else
			{
				if(i < adTestParams[0])
				{
					i++;
					bInstance = adTestParams[i];
					bLoop = 1;
				}
				else
					bLoop = 0;
			}
		}
	} while ((bLoop == 1) && (dtest_status == E__OK));	/* test all requested instances */

	vClosePciDevice(&sDevice);
	if(dtest_status != E__OK)
	{
		dtest_status += bInstance - 1;
#ifdef DEBUG
		sprintf(buffer, "Error in inst.: %d\n", bInstance);
		vConsoleWrite(buffer);
#endif
	}

	return (dtest_status);

} /* UsbOCCheck */

