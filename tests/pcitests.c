
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

/* pcitests.c - PCI Bus related tests
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/pcitests.c,v 1.8 2015-03-13 10:34:23 mgostling Exp $
 * $Log: pcitests.c,v $
 * Revision 1.8  2015-03-13 10:34:23  mgostling
 * Fixed compiler warning
 *
 * Revision 1.7  2015-03-12 15:30:08  hchalla
 * Added more debug statements.
 *
 * Revision 1.6  2015-01-29 10:26:21  mgostling
 * Added PciADCR5Test.
 *
 * Revision 1.5  2015-01-23 08:59:53  gshirasat
 * Updated the AMC PCIe test to check for appropriate links in STH and MTH ( bench test ) mode
 *
 * Revision 1.4  2015-01-21 18:00:50  gshirasat
 * Added AMC specific PCIe tests
 *
 * Revision 1.3  2014-11-10 11:08:30  mgostling
 * Updated scanPCIEwidth to display additional messages for XMC cards.
 *
 * Revision 1.2  2013-11-25 12:03:28  mgostling
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 * Removed trailng spaces from source.
 * Turned off verbose output in PcieVPXTest.
 *
 * Revision 1.1  2013-09-04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.20  2011/08/26 15:50:41  hchalla
 * Initial Release of TR 80x V1.01
 *
 * Revision 1.19  2011/08/02 17:02:24  hchalla
 * *** empty log message ***
 *
 * Revision 1.18  2011/05/11 12:17:17  hchalla
 * TP 702 PBIT Related changes.
 *
 * Revision 1.17  2011/03/22 13:57:56  hchalla
 * Modified for VX 813 pcie tests file checked in on behalf of Harron Muneer.
 *
 * Revision 1.16  2011/02/28 11:59:20  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.15  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.14  2010/11/04 18:00:37  hchalla
 * Added Debug info and alignement of the code.
 *
 * Revision 1.13  2010/10/25 15:37:39  jthiru
 * Cosmetic changes
 *
 * Revision 1.12  2010/09/15 12:25:15  hchalla
 * Modified tests to  support TR 501 BIT/CUTE.
 *
 * Revision 1.11  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.10  2010/03/31 16:27:10  hmuneer
 * no message
 *
 * Revision 1.9  2010/02/26 10:40:01  jthiru
 * PCI test case modification
 *
 * Revision 1.8  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.7  2009/07/24 14:03:51  hmuneer
 * V1.02
 *
 * Revision 1.6  2009/06/12 10:58:23  hmuneer
 * no message
 *
 * Revision 1.5  2009/06/10 14:45:11  hmuneer
 * smbus and eeprom update
 *
 * Revision 1.4  2009/06/08 09:55:58  hmuneer
 * TPA40
 *
 * Revision 1.3  2009/05/29 14:01:22  hmuneer
 * Error Codes Fixed
 *
 * Revision 1.2  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.1  2009/05/18 09:25:30  hmuneer
 * no message
 *
 * Revision 1.0  Feb 9 2009
 *
 *
 */


/* includes */
/*****************************************************************************/
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/conslib.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <bit/conslib.h>
#include <bit/board_service.h>
#include <bit/io.h>
#include <bit/delay.h>
#include <private/cpu.h>
#include <private/debug.h>

#define PCI_ERROR_WIDTH_NO_BRIDGE				(E__BIT + 0x901)
#define PCI_ERROR_WIDTH_NO_CAPABILITY			(E__BIT + 0x902)
#define PCI_ERROR_WIDTH_NOT_FOUND				(E__BIT + 0x903)
#define PCI_ERROR_WIDTH_BAD_WIDTH				(E__BIT + 0x905)

#define  TRXMC_BAD_PARAMETERS					0x80007010
#define  TRXMC_NO_BRIDGE						0x80007020
#define  TRXMC_WRONG_BRIDGE						0x80007030
#define  TRXMC_WRONG_CARRIER					0x80007040


#define  VXSPCIE_LOOPBACK_NO_PORT_A				0x80008020
#define  VXSPCIE_LOOPBACK_NO_PORT_B				0x80008030
#define  VXSPCIE_LOOPBACK_NO_NT_DEVICE_A		0x80008040
#define  VXSPCIE_LOOPBACK_NO_NT_DEVICE_B		0x80008050
#define  VXSPCIE_LOOPBACK_NO_DEVICE_ACCESS		0x80008060
#define  VXSPCIE_LOOPBACK_NO_MEM_BAR			0x80008070
#define  VXSPCIE_LOOPBACK_MEM_ALLOCATION		0x80008080
#define  VXSPCIE_LOOPBACK_DATA_ERROR			0x80008090

#define  AMC_DATA_BUS_ERR						E__BIT
#define  AMC_DAT_RDWR_ERR     					E__BIT + 0x9010
#define  AMC_DAT_CONFIG_ERR						E__BIT + 0x9020
#define  AMC_TEST_NO_MEM						E__BIT + 0x9030
#define  AMC_BRIDGE_NOT_FOUND_ERR	 			E__BIT + 0x9040
#define  AMC_UNSUPPORTED_SERVICE				E__BIT + 0x9050
#define  AMC_PCIE_LINKSPEED_ERR					E__BIT + 0x9060
#define  AMC_PCIE_LINKWIDTH_ERR					E__BIT + 0x9070

#define  PERICOM_VID							0x12D8
#define  X110_DID								0xE110

#define  TEST_BLOCK_LENGTH						0x100000

static UINT32 pwLineCount;
static UINT8 (*vChkPXMC)(void);
static UINT8 (*vChkSatellite)(void);
static UINT8 (*vChkPeripheral)(void);
static UINT32 vsxLoopBack(UINT32 bar);

/* defines */
//
//#define DEBUG

/*****************************************************************************
 * wReg8BitPCITest: this function performs a bit-wise test on the specified
 * 8-bit register. A marching 1/0 pattern is used, a mask value specifies
 * which bits are included in the test.
 * RETURNS : E_OK or E__... error code */
UINT32 wReg8BitPCITest(PCI_PFA pfa, UINT16	dPortAddress, UINT8	bBitMask)
{
	UINT8   Backup;
	UINT8	bFixed;			/* fixed bit pattern (of those outside the mask) */
	UINT8	bPattern;		/* current test bit-pattern */
	UINT8	bBitNum;		/* register bit number */
	UINT8	bTemp;			/* like it says! */
	#ifdef DEBUG
		char	achBuffer[128];	/* text formatting buffer */
	#endif

	Backup = PCI_READ_BYTE(pfa, dPortAddress);
	bFixed = PCI_READ_BYTE(pfa, dPortAddress) & ~bBitMask;

	for (bBitNum = 0; bBitNum < 8; bBitNum++)
	{
		bPattern = ((1 << bBitNum) & bBitMask);		/* Marching '1' */
		PCI_WRITE_BYTE(pfa, dPortAddress, bPattern | bFixed);
		bTemp = PCI_READ_BYTE(pfa, dPortAddress) & bBitMask;

		if (bTemp != bPattern)
		{
			PCI_WRITE_BYTE(pfa, dPortAddress, Backup);

			#ifdef DEBUG
				sprintf (achBuffer, "\nData error (1)[Addr:0x%x]: wrote %02Xh, read %02Xh",
						 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}

		bPattern = (~(1 << bBitNum) & bBitMask);	/* Marching '0' */
		PCI_WRITE_BYTE(pfa, dPortAddress, bPattern | bFixed);
		bTemp =  PCI_READ_BYTE(pfa, dPortAddress) & bBitMask;

		if (bTemp != bPattern)
		{
			PCI_WRITE_BYTE(pfa, dPortAddress, Backup);

			#ifdef DEBUG
				sprintf (achBuffer, "\nData error (0)[Addr:0x%x]: wrote %02Xh, read %02Xh",
					 	 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}
	}

	PCI_WRITE_BYTE(pfa, dPortAddress, Backup);
	return (E__OK);

} /* wReg8BitTest () */



/*****************************************************************************
 * PciTetsDevice: Tests a specific PCI device based on initialized device Data
 * RETURNS: error code */
UINT32 PciTetsDevice (pci_Device *currPciDevice,  UINT8 DevType,  PCI_PFA pfa )
{
	UINT32			rt = E__OK;
	UINT8			l;
	const pci_Regs  *DevRegs;

	DevRegs = currPciDevice->Regs;

	l = 0;
	if(DevRegs != NULL)
	{
		while(DevRegs[l].RegAddr != 0x00 && (rt == E__OK))
		{
			rt = wReg8BitPCITest(pfa, DevRegs[l].RegAddr, DevRegs[l].Mask);
			l++;
		}
	}
	return rt;
}



/*****************************************************************************
 * PciFindDevice: Matches a specific PCI device to the initialized device Data
 * RETURNS: error code */
UINT32 PciFindDevice( pci_Device *currPciList, UINT8 DevType, UINT8* Idx,
		              PCI_PFA pfa, UINT16 VID, UINT16 DID, UINT8 pBUS,
					  UINT8 sBUS, UINT8 device, UINT8 function )
{
	UINT32	rt = E__FAIL;
	UINT8   l;

	l = 0;

	while(currPciList[l].type != 0)
	{
		if( (currPciList[l].VID      == VID         ) &&
			(currPciList[l].DID      == DID         ) &&
			(currPciList[l].Device   == device      ) &&
			(currPciList[l].Function == function    ) &&
			(currPciList[l].Flags    == PCI_NOTFOUND)
		  )
		{
			switch(DevType)
			{
				case PCI_BRIDGE:
					if(currPciList[l].type & PCI_BRIDGE)
					{
						currPciList[l].Vars.pfa  = pfa;
						currPciList[l].Vars.pBUS = pBUS;
						currPciList[l].Vars.sBUS = sBUS;
						currPciList[l].Flags     = PCI_FOUND;
						rt                       = E__OK;
					}
					break;

				case PCI_DEVICE:
					if(currPciList[l].type & PCI_DEVICE)
					{
						currPciList[l].Vars.pfa  = pfa;
						currPciList[l].Flags     = PCI_FOUND;
						rt                       = E__OK;
					}
					break;

				case PCI_CBUS:
					if(currPciList[l].type & PCI_CBUS)
					{
						currPciList[l].Vars.pfa  = pfa;
						currPciList[l].Flags     = PCI_FOUND;
						rt                       = E__OK;
					}
					break;

				default:
					break;
			};

			if(rt == E__OK)
				*Idx = l;

			break;
		}
		else if( (currPciList[l].VID == VID         ) &&
			(currPciList[l].DID      == (DID - 1)   ) &&
			(currPciList[l].Device   == device      ) &&
			(currPciList[l].Function == function    ) &&
			(currPciList[l].Flags    == PCI_NOTFOUND)
		  )
		{
			switch(DevType)
			{
				case PCI_DEVICE:
					if(currPciList[l].type & PCI_DEVICE)
					{
						currPciList[l].Vars.pfa  = pfa;
						currPciList[l].Flags     = PCI_FOUND;
						rt                       = E__OK;
					}
					break;

				default:
					break;
			};

			if(rt == E__OK)
				*Idx = l;

			break;
		}

		l++;
	}

	return rt;
}


UINT32 PciFindDeviceL( pci_Device *currPciList, UINT8 DevType, UINT8* Idx,
		               PCI_PFA pfa, UINT16 VID, UINT16 DID, UINT8 pBUS,
					   UINT8 sBUS, UINT8 device, UINT8 function )
{
	UINT32	rt = E__FAIL;
	UINT8   l;

	l = 0;

	while(currPciList[l].type != 0)
	{
		if( (currPciList[l].VID      == VID         ) &&
			(currPciList[l].DID      == DID         ) &&
			(currPciList[l].Device   == device      ) &&
			(currPciList[l].Function == function    )
		  )
		{
			switch(DevType)
			{
				case PCI_BRIDGE:
					if(currPciList[l].type & PCI_BRIDGE)
						rt  = E__OK;
					break;

				case PCI_DEVICE:
					if(currPciList[l].type & PCI_DEVICE)
						rt = E__OK;
					break;

				case PCI_CBUS:
					if(currPciList[l].type & PCI_CBUS)
						rt = E__OK;
					break;

				default:
					break;
			};

			if(rt == E__OK)
				*Idx = l;

			break;
		}
		else if( (currPciList[l].VID == VID         ) &&
			(currPciList[l].DID      == (DID - 1)   ) &&
			(currPciList[l].Device   == device      ) &&
			(currPciList[l].Function == function    )
		  )
		{
			switch(DevType)
			{
				case PCI_DEVICE:
					if(currPciList[l].type & PCI_DEVICE)
						rt = E__OK;
					break;

				default:
					break;
			};

			if(rt == E__OK)
				*Idx = l;

			break;
		}

		l++;
	}

	return rt;
}






/*****************************************************************************
 * PciTestWrapper: Provides a warapper for all PCI tests
 * RETURNS: error code */

static UINT8		level;// indicate the level of recursion
static UINT32		pci_Erro_Base;
static UINT8		bridges, devices;

UINT32 PciTestWrapper ( UINT8       bus,
						UINT8       device,
						UINT8       func,
						UINT8       ScanType,
						pci_Device* scanList,
						pci_Device* listParent,
		                pci_Device* pxmcParent
					  )
{
	PCI_PFA	    pfa;
	UINT16	    VID, DID, i, l;
	UINT32	    rt = E__OK, rtt = E__OK;
	UINT8	    cHeader, bMultiFunc, pBUS, sBUS, foundIdx;
	UINT8       scanStart = 0, scanEnd = 0;
	pci_Device* nextScanListM = NULL;
	pci_Device* nextScanListB = NULL;
	pci_Device* Parent = NULL;

	#ifdef DEBUG
		UINT16	j;
		char	achBuffer[128], temp[128];
	#endif

	level++;

	if(func == 1)
	{
		scanStart = 1;
		scanEnd   = 8;
	}
	else
	{
		scanStart = 0;
		scanEnd   = 32;
	}

	for( i = scanStart;
		 ((i<scanEnd)
			#ifndef DEBUG
				&& (rt==E__OK)
			#endif
		 );
		 i++
	   )
	{
		if(func == 0)
			pfa = PCI_MAKE_PFA (bus, i, 0);
		else
			pfa = PCI_MAKE_PFA (bus, device, i);

		VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);

		#ifdef DEBUG
			memset(achBuffer, 0x00, 128);
			memset(temp, 0x00, 32);
		#endif

		if((VID != 0xffff) && (VID != 0x0000))
		{
			DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);
			cHeader = PCI_READ_BYTE (pfa, PCI_HEADER_TYPE);

			if (PCI_PFA_FUNC (pfa) == 0)
				bMultiFunc = (cHeader & 0x80);

			switch (cHeader & 0x7F)
			{
				case 0 :/* conventional device */
					{
						devices++;

						#ifdef DEBUG
						achBuffer[0] = '\n';
							for(j = 1; j<level; j++ )
								achBuffer[j] = '\t';
						#endif

						if(ScanType != PCI_ONBOARDSCAN)
							rt = E__OK;
						else
							rt = pci_Erro_Base + 1;
						if(scanList != NULL)
						{
							rt = PciFindDevice ( scanList, PCI_DEVICE, &foundIdx, pfa, VID, DID,
												 0, 0, (func == 0? i : device), (func == 0? 0 : i)  );
							if( rt == E__OK)
							{
								#ifdef DEBUG
								//	if(bMultiFunc != NULL || (func > 0))
								//		sprintf(temp, "[D%d] [M%d] [FO]", (func == 0? i : device), (func == 0? 0 : i) );
								//	else
								//		sprintf(temp, "[D%d] [FO]", i);
								#endif

								if(scanList[foundIdx].MDevices != NULL)
									nextScanListM = (pci_Device*)scanList[foundIdx].MDevices;
								else
									nextScanListM = NULL;

								Parent = &scanList[foundIdx];
							}
							else
							{
								#ifdef DEBUG
								//	if(bMultiFunc != NULL || (func > 0))
								//		sprintf(temp, "[D%d] [M%d] [NF]", (func == 0? i : device), (func == 0? 0 : i) );
								//	else
								//		sprintf(temp, "[D%d] [NF]", i);
								#endif

								if(listParent != NULL)
								{
									if( (listParent->type & PCI_DEV_PRESENT) != PCI_DEV_PRESENT)
										rt = pci_Erro_Base + 2;
									else
										rt = 0x0000abcd;
								}
								else
									rt = pci_Erro_Base + 2;

								nextScanListM = NULL;
								Parent        = NULL;
							}
						}
						else
						{
							#ifdef DEBUG
								if(bMultiFunc != NULL || (func > 0))
									sprintf(temp, "[D%d] [M%d] [UN]", (func == 0? i : device), (func == 0? 0 : i) );
								else
									sprintf(temp, "[D%d] [B%d] [UN]", i, bus);
							#endif

							  if(pxmcParent != NULL)
							  {
								if(pxmcParent->BDevices != NULL)
							    {
									rtt =  PciFindDeviceL ( (pci_Device*)pxmcParent->BDevices, PCI_DEVICE, &foundIdx, pfa,
														   VID, DID, 0, 0, (func == 0? i : device), (func == 0? 0 : i)  );
							    }
								else
									rtt =E__FAIL;

								if(rtt != E__OK)
								{
									if( (pxmcParent->Flags & PCI_PXMCdone) == 0x00)
									{
										pxmcParent->Flags  |= PCI_PXMCdone;
										pxmcParent->PXMCPFA = pfa;
									}
								}
								else
									devices--;
							  }

							nextScanListM = NULL;
							Parent        = NULL;
						}

						#ifdef DEBUG
							if(scanList != NULL)
								sprintf ( achBuffer+j, "[De] %s \"%s\" P=%x, V=%x, D=%x", temp,
										(rt == E__OK? scanList[foundIdx].Name: "UNKNOWN"), pfa,
										VID, DID );
							else
								sprintf ( achBuffer+j, "[De] %s \"%s\" P=%x, V=%x, D=%x", temp,
										"UNKNOWN", pfa,	VID, DID );
							vConsoleWrite (achBuffer);
						#endif

						if((rt == E__OK) && (scanList != NULL))
						{
							rt = PciTetsDevice (&scanList[foundIdx], PCI_DEVICE, pfa );

							if(rt != E__OK)
								rt = pci_Erro_Base + scanList[foundIdx].ERR_CODE;
						}

						#ifndef DEBUG
							if(rt == E__OK)
						#endif
							if( (bMultiFunc!= NULL) && (func == 0) )
								rt = PciTestWrapper (bus, i, 1, ScanType, nextScanListM, Parent, NULL);

						if(rt == 0x0000abcd)
							rt = E__OK;
					}
					break;


				case 1 :		/* PCI-to-PCI bridge */
					{
						bridges++;

						#ifdef DEBUG
							achBuffer[0] = '\n';
							for(j = 1; j<level; j++ )
								achBuffer[j] = '\t';
						#endif

						pBUS = PCI_READ_BYTE (pfa, PCI_PRI_BUS);
						sBUS = PCI_READ_BYTE (pfa, PCI_SEC_BUS);

						if(ScanType != PCI_ONBOARDSCAN)
							rt = E__OK;
						else
							rt = pci_Erro_Base + 3;
						if(scanList != NULL)
						{
							rt = PciFindDevice ( scanList,  PCI_BRIDGE, &foundIdx, pfa, VID, DID, pBUS,
												 sBUS, (func == 0? i : device), (func == 0? 0 : i)     );
							if( rt == E__OK)
							{
								#ifdef DEBUG
									if(bMultiFunc != NULL || (func > 0))
										sprintf(temp, "[D%d] [M%d] [FO]", (func == 0? i : device), (func == 0? 0 : i) );
									else
										sprintf(temp, "[D%d] [FO]", i);
								#endif

								if(scanList[foundIdx].MDevices != NULL)
									nextScanListM = (pci_Device*)scanList[foundIdx].MDevices;
								else
									nextScanListM = NULL;

								if(scanList[foundIdx].BDevices != NULL)
									nextScanListB = (pci_Device*)scanList[foundIdx].BDevices;
								else
									nextScanListB = NULL;

								Parent = &scanList[foundIdx];
							}
							else
							{
								#ifdef DEBUG
									if(bMultiFunc != NULL || (func > 0))
										sprintf(temp, "[D%d] [M%d] [NF]", (func == 0? i : device), (func == 0? 0 : i) );
									else
										sprintf(temp, "[D%d] [NF]", i);
								#endif

								if(listParent != NULL)
								{
									if( (listParent->type & PCI_DEV_PRESENT) != PCI_DEV_PRESENT)
										rt = pci_Erro_Base + 4;
									else
										rt = 0x0000abcd;
								}
								else
									rt = pci_Erro_Base + 4;

								nextScanListM = NULL;
								nextScanListB = NULL;
								Parent        = NULL;
							}
						}
						else
						{
							#ifdef DEBUG
								if(bMultiFunc != NULL || (func > 0))
									sprintf(temp, "[D%d] [M%d] [UN]", (func == 0? i : device), (func == 0? 0 : i) );
								else
									sprintf(temp, "[D%d] [UN]", i);
							#endif

							  if(pxmcParent != NULL)
							  {
								if(pxmcParent->BDevices != NULL)
							    {
									rtt =  PciFindDeviceL ( (pci_Device*)pxmcParent->BDevices,  PCI_BRIDGE, &foundIdx,
															pfa,VID, DID, pBUS, sBUS, (func == 0? i : device),
															(func == 0? 0 : i));
							    }
								else
									rtt = E__FAIL;

								if(rtt != E__OK)
								{
									if( (pxmcParent->Flags & PCI_PXMCdone) == 0x00)
									{
										pxmcParent->Flags  |= PCI_PXMCdone;
										pxmcParent->PXMCPFA = pfa;
									}
								}
								else
									bridges--;
							  }

							nextScanListB = NULL;
							nextScanListM = NULL;
							Parent        = NULL;
						}


						#ifdef DEBUG
							if(scanList != NULL)
								sprintf( achBuffer+j, "[Br] %s \"%s\" P=%x, V=%x, D=%x, PB=0x%x, SB=0x%x",
										temp, (rt == E__OK? scanList[foundIdx].Name: "UNKNOWN"), pfa, VID,
										DID, pBUS, sBUS );
							else
								sprintf( achBuffer+j, "[Br] %s \"%s\" P=%x, V=%x, D=%x, PB=0x%x, SB=0x%x",
										 temp, "UNKNOWN", pfa, VID, DID, pBUS, sBUS );
							vConsoleWrite (achBuffer);
						#endif

						if((rt == E__OK) && (scanList != NULL))
						{
							rt = PciTetsDevice (&scanList[foundIdx], PCI_DEVICE, pfa );

							if(rt != E__OK)
								rt = pci_Erro_Base + scanList[foundIdx].ERR_CODE;
						}

						#ifndef DEBUG
							if(rt == E__OK)
						#endif
							if(bMultiFunc!= NULL && (func == 0))
								rt = PciTestWrapper (bus, i, 1, ScanType, nextScanListM, Parent, NULL);

						#ifndef DEBUG
							if(rt == E__OK)
						#endif
						{
							if( (ScanType == PCI_ONBOARDSCAN) && (scanList != NULL) &&

								(	( ((scanList[foundIdx].type & PXMC_PARENT) != PXMC_PARENT) &&
								      ((scanList[foundIdx].type & CPCI_PARENT) != CPCI_PARENT) &&
								      ((scanList[foundIdx].type & PCI_CARRIER) != PCI_CARRIER)   )
																									||
								    ( ((scanList[foundIdx].type & PXMC_PARENT) == PXMC_PARENT) &&
								      ((scanList[foundIdx].type & PCI_DEV_PRESENT) == PCI_DEV_PRESENT))
								)
							  )
								rt = PciTestWrapper (sBUS, 0, 0, ScanType, nextScanListB, Parent, NULL);
							else if(ScanType == PCI_PXMCSCAN)
								rt = PciTestWrapper (sBUS, 0, 0, ScanType, NULL, Parent, NULL);
						}

						if(rt == 0x0000abcd)
							rt = E__OK;
					}
					break;

				case 2 :		/* CardBus bridge */
					{
						#ifdef DEBUG
							sprintf( achBuffer, "\nCardBus PFA=%4x, VID=%2x, DID=%2x ", pfa, VID, DID );
							vConsoleWrite (achBuffer);
						#endif
					}
					break;

				default:
					break;
			}
		}
	}


	l = 0;
	#ifndef DEBUG
		if( rt == E__OK)
	#endif
	if(scanList != NULL)
	{
		while( (scanList[l].type != 0 )
			   #ifndef DEBUG
					&& (rt == E__OK)
			   #endif
			  )
		{
			if((scanList[l].Flags & PCI_FOUND) != PCI_FOUND)
			{
				if((scanList[l].type & PCI_OPTIONAL) == PCI_OPTIONAL)
				{
					#ifdef DEBUG
						achBuffer[0] = '\n';
						for(j = 1; j<level; j++ )
							achBuffer[j] = '\t';
						sprintf( achBuffer+j, "[OPTIONAL NOT FOUND] %s ", scanList[l].Name);
						vConsoleWrite (achBuffer);
					#endif
				}
				else if((scanList[l].type & CPCI_PARENT) == CPCI_PARENT)
				{
					if(vChkSatellite)
					{
						if((*vChkSatellite)() == 0)
						{
							#ifdef DEBUG
								achBuffer[0] = '\n';
								for(j = 1; j<level; j++ )
									achBuffer[j] = '\t';
								sprintf( achBuffer+j, "[CPCI Bridge] %s [In satellite Mode]", scanList[l].Name);
								vConsoleWrite (achBuffer);
							#endif
						}
						else
						{
							rt = pci_Erro_Base + 0x1000 + scanList[l].ERR_CODE;

							#ifdef DEBUG
								achBuffer[0] = '\n';
								for(j = 1; j<level; j++ )
									achBuffer[j] = '\t';
								sprintf( achBuffer+j, "[FAILED] %s rt %x", scanList[l].Name, rt);
								vConsoleWrite (achBuffer);
							#endif
						}
					}
					else
					{
							rt = pci_Erro_Base + 0x1000 + scanList[l].ERR_CODE;

							#ifdef DEBUG
								achBuffer[0] = '\n';
								for(j = 1; j<level; j++ )
									achBuffer[j] = '\t';
								sprintf( achBuffer+j, "[FAILED] %s rt %x", scanList[l].Name, rt);
								vConsoleWrite (achBuffer);
							#endif
					}
				}
				else
				{
					rt = pci_Erro_Base + 0x1000 + scanList[l].ERR_CODE;

					#ifdef DEBUG
						achBuffer[0] = '\n';
						for(j = 1; j<level; j++ )
							achBuffer[j] = '\t';
						sprintf( achBuffer+j, "[FAILED] %s rt %x", scanList[l].Name, rt);
						vConsoleWrite (achBuffer);
					#endif

				}
			}
			l++;
		}
	}
	level--;

	#ifdef DEBUG
		if(level == 0)
			vConsoleWrite ("\n\n");
	#endif

	return(rt);
}




void setPtrs(pci_DB* DB)
{
	vChkPXMC	   = DB->vChkPXMC;
	vChkSatellite  = DB->vChkSatellite;
	vChkPeripheral = DB->vChkPeripheral;
}


/*****************************************************************************
 * scanInit: Init the the PCI data structures for a new scan
 * RETURNS: None */
void scanInit(pci_Device* scanList)
{
	UINT8 l;

	l = 0;
	while(scanList[l].type != 0x00)
	{
		scanList[l].Flags = 0x00;
		memset(&scanList[l].Vars, 0x00, sizeof(pci_Vars));

		if(scanList[l].MDevices != NULL)
			scanInit((pci_Device*)scanList[l].MDevices);

		if(scanList[l].BDevices != NULL)
			scanInit((pci_Device*)scanList[l].BDevices);

		l++;
	}
}


/*****************************************************************************
 * scanPXMC: Scans PMC/XMC sites
 * RETURNS: None */

#define PCI_ERROR_PXMC		(E__BIT + 0x800)

UINT32 scanPXMC(pci_Device* scanList, UINT8 pxmc, UINT8 info)
{
	UINT32 rt = E__OK;
	UINT8  l;

	char   achBuffer[128];

	l = 0;
	while((scanList[l].type != 0x00) && (rt == E__OK))
	{
		if((scanList[l].type & PXMC_PARENT) && (scanList[l].type & PCI_BRIDGE) && (scanList[l].Flags & PCI_FOUND))
		{
			if(scanList[l].functions & pxmc)
			{
				bridges = 0;
				devices = 0;

				PciTestWrapper (scanList[l].Vars.sBUS, 0, 0, PCI_PXMCSCAN, NULL, &scanList[l], &scanList[l]);

				if(bridges > 0 || devices > 0)
					rt = E__OK;
				else if( scanList[l].functions & PXMC_OPTIONAL)
					rt = E__OK;
				else
					rt = PCI_ERROR_PXMC;

				if(info)
				{
					switch(scanList[l].functions & pxmc)
					{
						case PXMC_PMC1:
							vConsoleWrite ("\nPMC1: ");
							break;

						case PXMC_PMC2:
							vConsoleWrite ("\nPMC2: ");
							break;

						case PXMC_XMC1:
							vConsoleWrite ("\nXMC1: ");
							break;

						case PXMC_XMC2:
							vConsoleWrite ("\nXMC2: ");
							break;

						default:
							break;
					}

					sprintf( achBuffer, "Bridges = %d, Devices = %d\n", bridges, devices);
					vConsoleWrite (achBuffer);
				}
			}
		}

		if(rt == E__OK)
			if(scanList[l].MDevices != NULL)
				rt =scanPXMC((pci_Device*)scanList[l].MDevices, pxmc, info);

		if(rt == E__OK)
			if(scanList[l].BDevices != NULL)
				rt = scanPXMC((pci_Device*)scanList[l].BDevices, pxmc, info);

		l++;
	}

	return rt;
}



/*****************************************************************************
 * scanPXMC: Scans PMC/XMC sites
 * RETURNS: None */

UINT32 scanPCIEwidth(pci_Device* scanList, UINT32 bkWidth)
{
	UINT32		rt = E__OK, l, reg, temp;
	UINT16		bMin, bMax, dMin, dMax, flag;
	pci_Device* scanDevice;
	PCI_PFA		pfa;

	char achBuffer[128];

	l = 0;
	while((scanList[l].type != 0x00) && (rt == E__OK))
	{
		flag = 0;

		if((scanList[l].type & PCI_EXPRESS) && (scanList[l].type & PCI_BRIDGE) &&
		   ((scanList[l].type & PCI_EXPRESS_END) != PCI_EXPRESS_END)
		  )
		{
			pwLineCount+=2;
			if (pwLineCount > 20)
			{
				if( (bGetTestHandler() == TH__LBIT) || (bGetTestHandler() == TH__MTH) )
				{
					puts ("** Press any key to continue **");
					while (iGetExtdKeyPress () == 0);
				}

				pwLineCount = 0;
			}

			if(scanList[l].Flags & PCI_FOUND)
			{
					sprintf( achBuffer, "\n%s\n----------------------------------------\n",
							 scanList[l].Name);
					vConsoleWrite (achBuffer);
					sysDebugWriteString (achBuffer);

				reg  = PCI_READ_BYTE (scanList[l].Vars.pfa, 0x34);
				temp = PCI_READ_DWORD (scanList[l].Vars.pfa, reg);
			}
			else if(scanList[l].type & PCI_OPTIONAL)
			{
				l++;
				continue;
			}
			else if(scanList[l].type & CPCI_PARENT)
			{
				if(vChkSatellite)
				{
					if((*vChkSatellite)() == 0)
					{
						l++;
						continue;
					}
					else
					{
						sprintf( achBuffer, "\n%s\n----------------------------------------\n",
								 scanList[l].Name);
						vConsoleWrite (achBuffer);
						sysDebugWriteString (achBuffer);
						vConsoleWrite ("Bridge NOT present/Bridge Failure\n");
						sysDebugWriteString ("Bridge NOT present/Bridge Failure\n");

						temp = 0xff;
						rt   = PCI_ERROR_WIDTH_NO_BRIDGE;
					}
				}
				else
				{
					sprintf( achBuffer, "\n%s\n----------------------------------------\n",
							 scanList[l].Name);
					vConsoleWrite (achBuffer);
					sysDebugWriteString (achBuffer);
					vConsoleWrite ("Bridge NOT present/Bridge Failure\n");
					sysDebugWriteString ("Bridge NOT present/Bridge Failure\n");

					temp = 0xff;
					rt   = PCI_ERROR_WIDTH_NO_BRIDGE;
				}
			}
			else
			{
				sprintf( achBuffer, "\n%s\n----------------------------------------\n",
						 scanList[l].Name);
				vConsoleWrite (achBuffer);
				sysDebugWriteString (achBuffer);
				vConsoleWrite ("Bridge NOT present/Bridge Failure\n");
				sysDebugWriteString ("Bridge NOT present/Bridge Failure\n");

				temp = 0xff;
				rt   = PCI_ERROR_WIDTH_NO_BRIDGE;
			}

			while( ((temp & 0x000000ff) != 0x10)  && (temp != 0xff) )
			{
				reg = ((temp >> 8) & 0x000000ff	);
				if((reg == 0xff) || (reg == 0x00))
				{
					temp = 0xff;
					rt   = PCI_ERROR_WIDTH_NO_CAPABILITY;
				}
				else
					temp = PCI_READ_DWORD (scanList[l].Vars.pfa, reg);
			}

			if(temp != 0xff)
			{
				pwLineCount+=2;


				temp = PCI_READ_DWORD (scanList[l].Vars.pfa, reg+0x0c);
				bMax = (temp >> 4) & 0x0000003f;

				sprintf( achBuffer, "Max Width = %d, ", bMax);
				vConsoleWrite (achBuffer);
				sysDebugWriteString (achBuffer);

				if(scanList[l].options->type & OPTION_MAX_PCIE_WIDTH)
				{
					if (bkWidth > 0)
					{
						bMax = bkWidth;
						sprintf( achBuffer, "Limit = %d, ", bMax);
						vConsoleWrite (achBuffer);
						sysDebugWriteString (achBuffer);
					}
					else if(bMax > scanList[l].options->data)
					{
						bMax = scanList[l].options->data;
						sprintf( achBuffer, "Limit = %d ", bMax);
						vConsoleWrite (achBuffer);
						sysDebugWriteString (achBuffer);
					}
				}

				temp = PCI_READ_WORD (scanList[l].Vars.pfa, reg+0x12);
				bMin = (temp >> 4) & 0x003f;

					sprintf( achBuffer, "Negotiated = %d, ", bMin);
					vConsoleWrite (achBuffer);
					sysDebugWriteString (achBuffer);

				temp = temp & 0x000f;
				if(temp == 0x0001)
					sprintf( achBuffer, "Type = Gen1\n");
				else if(temp == 0x0002)
					sprintf( achBuffer, "Type = Gen2\n");
				else if(temp == 0x0003)
					sprintf( achBuffer, "Type = Gen3\n");
				else
					sprintf( achBuffer, "Type = UNKNOWN [%x]\n", temp);
				vConsoleWrite (achBuffer);
				sysDebugWriteString (achBuffer);

				if( (scanList[l].BDevices != NULL) || (scanList[l].type & PXMC_PARENT) ||
					((scanList[l].type & PCI_EXPRESS) && (scanList[l].type & CPCI_PARENT))
				  )
				{
					if(scanList[l].BDevices != NULL)
					{
						scanDevice = (pci_Device*)scanList[l].BDevices;

							pwLineCount+=2;
							if (pwLineCount > 18)
							{
								if( (bGetTestHandler() == TH__LBIT) || (bGetTestHandler() == TH__MTH) )
								{
									puts ("** Press any key to continue **");
									while (iGetExtdKeyPress () == 0);
								}
								pwLineCount = 0;
							}
							sprintf( achBuffer, "\n\t%s\n\t----------------------------------------\n",
									 scanDevice[0].Name);
							vConsoleWrite (achBuffer);
							sysDebugWriteString (achBuffer);

						if(scanDevice[0].Flags & PCI_FOUND)
						{
							pfa = scanDevice[0].Vars.pfa;
						}
						else if(scanDevice[0].type & PCI_OPTIONAL)
						{
							if(rt == E__OK)
							{
								if(scanList[l].MDevices != NULL)
								{
									rt = scanPCIEwidth((pci_Device*)scanList[l].MDevices, bkWidth);
								}
							}
							l++;
							continue;
						}
						else if(scanDevice[0].type & CPCI_PARENT)
						{
							if(vChkSatellite)
							{
								if((*vChkSatellite)()==0)
								{
									if(rt == E__OK)
									{
										if(scanList[l].MDevices != NULL)
										{
											rt = scanPCIEwidth((pci_Device*)scanList[l].MDevices, bkWidth);
										}
									}
									l++;
									continue;
								}
								else
								{
									pfa = NULL;
									rt  = PCI_ERROR_WIDTH_NOT_FOUND;
								}
							}
							else
							{
								pfa = NULL;
								rt  = PCI_ERROR_WIDTH_NOT_FOUND;
							}
						}
						else
						{
							pfa = NULL;
							rt  = PCI_ERROR_WIDTH_NOT_FOUND;
						}
					}
					else
					{
						if( scanList[l].type & PXMC_PARENT )
						{
							pwLineCount+=2;
							if (pwLineCount > 18)
							{
								if( (bGetTestHandler() == TH__LBIT) || (bGetTestHandler() == TH__MTH) )
								{
									puts ("** Press any key to continue **");
									while (iGetExtdKeyPress () == 0);
								}
								pwLineCount = 0;
							}
							sprintf( achBuffer, "\n\tDevice on PMC/XMC Bridge\n\t----------------------------------------\n");
							vConsoleWrite (achBuffer);
							sysDebugWriteString (achBuffer);

							if(scanList[l].Flags & PCI_PXMCdone)
								pfa = scanList[l].PXMCPFA;
							else
							{
								if(vChkPXMC)
									temp = (*vChkPXMC)();
								else
									temp = 0;

								if ((scanList[l].functions & (PXMC_XMC1 + PXMC_XMC2)) != 0)
								{
									if ((temp & (PXMC_XMC1 + PXMC_XMC2)) != 0)
									{
										// card detected
										if(scanList[l].functions & PXMC_OPTIONAL)
										{
											l++;
											pwLineCount+=2;
											vConsoleWrite ("\tDevice NOT present\n");
											sysDebugWriteString (achBuffer);
											continue;
										}

										pfa = NULL;
										pwLineCount+=2;
										vConsoleWrite ("\tDevice Failure\n");
										sysDebugWriteString (achBuffer);
										rt = PCI_ERROR_WIDTH_NOT_FOUND;
									}
									else
									{
										// no card detected
										pfa = 0;
										pwLineCount+=2;
										vConsoleWrite ("\tDevice NOT present/Device Failure\n");
										sysDebugWriteString (achBuffer);
									}
								}
								if(rt == E__OK)
								{
										if(scanList[l].MDevices != NULL)
											rt = scanPCIEwidth((pci_Device*)scanList[l].MDevices, bkWidth);

									l++;
									continue;
								}
							}
						}

						if( scanList[l].type & CPCI_PARENT )
						{
							pwLineCount+=2;
							if (pwLineCount > 18)
							{
								if( (bGetTestHandler() == TH__LBIT) || (bGetTestHandler() == TH__MTH) )
								{
									puts ("** Press any key to continue **");
									while (iGetExtdKeyPress () == 0);
								}
								pwLineCount = 0;
							}
							sprintf( achBuffer, "\n\tDevice on BackPlane\n\t----------------------------------------\n");
							vConsoleWrite (achBuffer);
							sysDebugWriteString (achBuffer);

							if(scanList[l].Flags & PCI_PXMCdone)
								pfa = scanList[l].PXMCPFA;
							else
								pfa = NULL;
						}
					}

					if(pfa != NULL)
					{
						reg  = PCI_READ_BYTE ( pfa, 0x34);
						temp = PCI_READ_DWORD (pfa, reg);
						while(	((temp & 0x000000ff) != 0x10)  && (temp != 0xff) )
						{
							reg = ((temp >> 8) & 0x000000ff	);
							if(reg == 0xff || (reg == 0x00))
							{
								rt   = PCI_ERROR_WIDTH_NO_CAPABILITY;
								temp = 0xff;
							}
							else
								temp = PCI_READ_DWORD(pfa, reg);
						}

						if(temp != 0xff)
						{
							flag = 1;
							pwLineCount+=2;

							temp = PCI_READ_DWORD (pfa, reg+0x0c);
							dMax = (temp >> 4) & 0x0000003f;

								sprintf( achBuffer, "\tMax Width = %d, ", dMax);
								vConsoleWrite (achBuffer);
								sysDebugWriteString (achBuffer);


							temp = PCI_READ_WORD (pfa, reg+0x12);
							dMin = (temp >> 4) & 0x003f;

								sprintf( achBuffer, "Negotiated = %d, ", dMin);
								vConsoleWrite (achBuffer);
								sysDebugWriteString (achBuffer);

							temp = temp & 0x000f;
							if(temp == 0x0001)
								sprintf( achBuffer, "Type = Gen1\n");
							else if(temp == 0x0002)
								sprintf( achBuffer, "Type = Gen2\n");
							else if(temp == 0x0003)
								sprintf( achBuffer, "Type = Gen3\n");
							else
								sprintf( achBuffer, "Type = UNKNOWN [%x]\n", temp);
							vConsoleWrite (achBuffer);
							sysDebugWriteString (achBuffer);
						}
						else
						{
							pwLineCount+=2;
							vConsoleWrite ("\tNo Device Capability found\n");
							sysDebugWriteString (achBuffer);
						}
					}
					else
					{
						pwLineCount+=2;
						vConsoleWrite ("\tDevice NOT present/Device Failure\n");
						sysDebugWriteString (achBuffer);
					}
				}
				else
				{
					pwLineCount+=2;
					vConsoleWrite ("No Device Present on this Bridge\n");
					sysDebugWriteString (achBuffer);
				}
			}
			else
			{
				pwLineCount+=2;
				vConsoleWrite ("No Bridge Capability found\n");
				sysDebugWriteString (achBuffer);
			}
		}

		//match link widths here
		if((rt == E__OK) && (flag == 1))
		{
			if(dMin == bMin)
			{
				if(dMax >= bMax)
				{
					if(dMin != bMax)
						rt = PCI_ERROR_WIDTH_BAD_WIDTH;
				}
				else
				{
					if(dMin != dMax)
						rt = PCI_ERROR_WIDTH_BAD_WIDTH;
				}
			}
			else
				rt = PCI_ERROR_WIDTH_BAD_WIDTH; //error
		}

		if(rt == E__OK)
			if(scanList[l].MDevices != NULL)
				rt = scanPCIEwidth((pci_Device*)scanList[l].MDevices, bkWidth);

		if(rt == E__OK)
			if(scanList[l].BDevices != NULL)
				rt = scanPCIEwidth((pci_Device*)scanList[l].BDevices, bkWidth);

		l++;
	}

	return rt;
}



/*****************************************************************************
 * scanPXMC: Scans PMC/XMC sites
 * RETURNS: None */

#define PCI_ERROR_DEVICE_NOT_PRESENT	(E__BIT + 0xA01)
#define PCI_ERROR_BIST_FAILED			(E__BIT + 0xA02)
#define PCI_ERROR_BIST_TIMED_OUT		(E__BIT + 0xA03)
#define PCI_ERROR_BIST_NOT_COMPLETE		(E__BIT + 0xA04)

UINT32 pciBIST(pci_Device* scanList)
{
	UINT32 rt = E__OK, tm;
	UINT8  l;
	UINT8  reg;

	#ifdef DEBUG
		char   achBuffer[128];
	#endif

	l = 0;
	while((scanList[l].type != 0x00) && (rt == E__OK))
	{
		#ifdef DEBUG
			sprintf( achBuffer, "\n%s\n---------------------------\n", scanList[l].Name);
			vConsoleWrite (achBuffer);
		#endif

		if(scanList[l].Flags & PCI_FOUND)
		{
			reg  = PCI_READ_BYTE (scanList[l].Vars.pfa, 0x0f);
			if(reg & 0x80)
			{
				#ifdef DEBUG
					vConsoleWrite ("BIST Supported");
				#endif
				reg |= 0x40;
				PCI_WRITE_BYTE (scanList[l].Vars.pfa, 0x0f, reg);
				reg = PCI_READ_BYTE (scanList[l].Vars.pfa, 0x0f);
				tm  = 20;
				while((reg & 0x40) && (tm > 0))
				{
					vDelay(100);
					tm--;
					reg = PCI_READ_BYTE (scanList[l].Vars.pfa, 0x0f);
				}

				if((reg & 0x40) == 0x00)
				{
					#ifdef DEBUG
						vConsoleWrite ("\tCOMPLETE");
					#endif
					if((reg & 0x0f) == 0x00)
					{
						#ifdef DEBUG
							vConsoleWrite ("\tPASSED\n");
						#endif
					}
					else
					{
						rt   = PCI_ERROR_BIST_FAILED;
						#ifdef DEBUG
							vConsoleWrite ("\tFAILED\n");
						#endif
					}
				}
				else if(tm == 0)
				{
					rt   = PCI_ERROR_BIST_TIMED_OUT;
					#ifdef DEBUG
						vConsoleWrite ("\tTIMED OUT\n");//timed out
					#endif
				}
				else
				{
					rt   = PCI_ERROR_BIST_NOT_COMPLETE;
					#ifdef DEBUG
						vConsoleWrite ("\tDID NOT COMPLETE\n");//Did not complete
					#endif
				}
			}
			else
			{
				#ifdef DEBUG
					sprintf( achBuffer, "BIST NOT Supported [%x]\n", reg);
					vConsoleWrite (achBuffer);
				#endif
			}
		}
		else
		{
			if(scanList[l].type & PCI_OPTIONAL)
			{
				l++;
				continue;
			}
			else if(scanList[l].type & CPCI_PARENT)
			{
				if(vChkSatellite)
				{
					if((*vChkSatellite)() == 0)
					{
						l++;
						continue;
					}
					else
					{
						#ifdef DEBUG
							vConsoleWrite ("Device/Bridge NOT present/Failure\n");
						#endif
						rt = PCI_ERROR_DEVICE_NOT_PRESENT;
					}
				}
				else
				{
					#ifdef DEBUG
						vConsoleWrite ("Device/Bridge NOT present/Failure\n");
					#endif
					rt = PCI_ERROR_DEVICE_NOT_PRESENT;
				}
			}
			else
			{
				#ifdef DEBUG
					vConsoleWrite ("Device/Bridge NOT present/Failure\n");
				#endif
				rt = PCI_ERROR_DEVICE_NOT_PRESENT;
			}
		}


		if(rt == E__OK)
			if(scanList[l].MDevices != NULL)
				rt = pciBIST((pci_Device*)scanList[l].MDevices);

		if(rt == E__OK)
			if(scanList[l].BDevices != NULL)
				rt = pciBIST((pci_Device*)scanList[l].BDevices);
		l++;
	}

	return rt;
}


#define PCI_ERROR_CPCI_BRIDGE		(E__BIT + 0xB01)

/*****************************************************************************
 * scanPXMC: Scans PMC/XMC sites
 * RETURNS: None */
UINT32 CpciBr(pci_Device* scanList)
{
	UINT32 rt = E__OK;
	UINT8  l = 0;
	char   achBuffer[128];


	while(scanList[l].type != 0x00 && (rt == E__OK))
	{
		if(scanList[l].type & CPCI_PARENT)
		{
			sprintf( achBuffer, "\n%s", scanList[l].Name);
			vConsoleWrite (achBuffer);

			if(scanList[l].Flags & PCI_FOUND)
			{
				vConsoleWrite (" Found\n");
				rt = E__OK;
				l++;
				continue;
			}
			else
			{
				if(scanList[l].type & PCI_OPTIONAL)
				{
					vConsoleWrite (" Optional\n");
					rt = E__OK;
					l++;
					continue;
				}
				else
					rt = PCI_ERROR_CPCI_BRIDGE;
			}

		}

		if(rt == E__OK)
		{
			if(scanList[l].MDevices != NULL)
				rt = CpciBr((pci_Device*)scanList[l].MDevices);

		}

		if(rt == E__OK)
		{
			if(scanList[l].BDevices != NULL)
				rt = CpciBr((pci_Device*)scanList[l].BDevices);
		}

		l++;
	}

	return rt;
}


#define PCI_ERROR_CPCI_NO_DEVICE		(E__BIT + 0xC01)

/*****************************************************************************
 * scanCPCI: Scans CPCI Bus
 * RETURNS: None */
UINT32 scanCPCI(pci_Device* scanList, UINT8 info)
{
	UINT32 rt = E__OK;
	UINT8  l;
	char   achBuffer[128];


	l = 0;
	while((scanList[l].type != 0x00) && (rt == E__OK))
	{
		if((scanList[l].type & CPCI_PARENT))
		{
			if(scanList[l].Flags & PCI_FOUND)
			{
				bridges		  = 0;
				devices		  = 0;
				level         = 0;

				PciTestWrapper (scanList[l].Vars.sBUS, 0, 0, PCI_PXMCSCAN, NULL, &scanList[l], &scanList[l]);

				if(info)
				{
					sprintf( achBuffer, "Scanning on Secondary Bus of Bridge \"%s\"\n\tScan Result: Bridges = %d, Devices = %d\n\n",
					         scanList[l].Name, bridges, devices);
					vConsoleWrite (achBuffer);
				}
			}
			else
			{
				if (scanList[l].type & PCI_OPTIONAL)
				{
					rt = E__OK;
				}
				else
					rt = PCI_ERROR_CPCI_NO_DEVICE;
			}
			l++;
			continue;
		}

		if(rt == E__OK)
			if(scanList[l].MDevices != NULL)
				rt = scanCPCI((pci_Device*)scanList[l].MDevices, info);

		if(rt == E__OK)
			if(scanList[l].BDevices != NULL)
				rt = scanCPCI((pci_Device*)scanList[l].BDevices, info);

		l++;
	}

	return rt;
}




/*****************************************************************************
 * PciTest: Test the PCI On Board Devices
 * RETURNS: None */
TEST_INTERFACE (PciTest, "PCI On Board Devices Test")
{
	UINT32  rt;
	pci_DB* DB;

	bridges		  = 0;
   	devices		  = 0;
	level         = 0;
	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	scanInit(DB->Devices0);

	rt = PciTestWrapper (0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
	if(rt == E__OK)
		DB->Flags = 0x01;

	return rt;
}



/*****************************************************************************
 * PciPXMCTest: Test PCI PMC/XMC Site
 * RETURNS: None */
TEST_INTERFACE (PciPXMCTest, "PCI PMC/XMC Site Test")
{
	UINT32  rt = E__OK;
	UINT8   pxmc;
	pci_DB* DB;

   	bridges       = 0;
   	devices       = 0;
	level         = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(vChkPXMC)
	{
		pxmc = (*vChkPXMC)();

		if(pxmc)
		{
			if(DB->Flags != 0x01)
			{
				scanInit(DB->Devices0);
				rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
				if(rt == E__OK)
					DB->Flags = 0x01;
				#ifdef DEBUG
					else
						vConsoleWrite("\nPCI On Board Devices Test Failed\n");
				#endif
			}

			if(DB->Flags == 0x01)
			{
				bridges       = 0;
				devices       = 0;
				level         = 0;

				if(pxmc & PXMC_PMC1)
				{
					rt    = scanPXMC(DB->Devices0, PXMC_PMC1, 1);
				}

				if(rt == E__OK)
				{
					bridges       = 0;
					devices       = 0;
					level         = 0;

					if(pxmc & PXMC_PMC2)
						rt    = scanPXMC(DB->Devices0, PXMC_PMC2, 1);
				}

				if(rt == E__OK)
				{
					bridges       = 0;
					devices       = 0;
					level         = 0;

					if(pxmc & PXMC_XMC1)
						rt    = scanPXMC(DB->Devices0, PXMC_XMC1, 1);
				}

				if(rt == E__OK)
				{
					bridges       = 0;
					devices       = 0;
					level         = 0;

					if(pxmc & PXMC_XMC2)
						rt    = scanPXMC(DB->Devices0, PXMC_XMC2, 1);
				}
			}
		}
		#ifdef DEBUG
			else
				vConsoleWrite ("\nNO PMC/XMC Fitted On This Board\n");
		#endif
	}
	#ifdef DEBUG
		else
			vConsoleWrite ("\nNO PMC/XMC Slot On This Board\n");
	#endif

	return rt;
}




/*****************************************************************************
 * PciTest: Test the PCI Express Port Widths
 * RETURNS: None */
TEST_INTERFACE (PciPW, "PCI Express Port Width Test")
{
	UINT32	rt;
	pci_DB*	DB;
	UINT8   pxmc;
	UINT32	bkWidth = 0;
	bridges       = 0;
	devices       = 0;
	level         = 0;
	pwLineCount   = 0;
#ifdef DEBUG
	char achBuffer[80];
#endif

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

#ifdef DEBUG
	sprintf(achBuffer,"adTestParams[0]: %x,adTestParams[1]:\n ",adTestParams[0],adTestParams[1]);
	sysDebugWriteString(achBuffer);
#endif
	if(adTestParams[0] == 2)
	{
		if (adTestParams[1] == 1)
		{
			bkWidth = adTestParams[2];
		}
		else
			return pci_Erro_Base;
	}
	else if (adTestParams[0] > 0)
		return pci_Erro_Base;

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper (0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite ("\nPCI On Board Devices Test Failed\n");
				sysDebugWriteString("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(DB->Flags == 0x01)
	{
		if(vChkPXMC)
		{
			pxmc = (*vChkPXMC)();
			if(pxmc)
			{
				scanPXMC(DB->Devices0, pxmc, 0);
			}
		}

		bridges = 0;
		devices = 0;
		level   = 0;
		scanCPCI(DB->Devices0, 0);

		rt = scanPCIEwidth(DB->Devices0, bkWidth);
	}

	return rt;
}




/*****************************************************************************
 * PciTest: Test the PCI Express Port Widths
 * RETURNS: None */
TEST_INTERFACE (PciBist, "PCI Devices BIST")
{
	UINT32		rt;
	pci_DB*		DB;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);


	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper (0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite ("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(DB->Flags == 0x01)
	{
		rt = pciBIST(DB->Devices0);
	}

	return rt;
}






/*****************************************************************************
 * PciTest: Test the Presence of CPCI bridge
 * RETURNS: None */
TEST_INTERFACE (CPciBridge, "CPCI Bridge Presence")
{
	UINT32		rt;
	pci_DB*		DB;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);

	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper (0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite ("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(vChkPeripheral)
	{
		if( (*vChkPeripheral)() == 0)
		{
			#ifdef DEBUG
				vConsoleWrite ("\nPeripheral Mode\n");
			#endif
			return E__OK;
		}
	}

	if(DB->Flags == 0x01)
	{
		if(vChkSatellite)
		{
			if( (*vChkSatellite)() == 1)
				rt = CpciBr(DB->Devices0);
			else
			{
				vConsoleWrite ("\nSatellite Mode\n");
				rt = E__OK;
			}
		}
		else
		{
			vConsoleWrite ("\nNO CPCI Bridge On This Board\n");
			rt = 2;
		}
	}

	return rt;
}



/*****************************************************************************
 * PciPXMCTest: Test the presence of devices on CPCI Bus
 * RETURNS: None */
TEST_INTERFACE (PciCPCITest, "PCI CPCI Bus Devices")
{
	UINT32  rt = E__OK;
	UINT8   cpci;
	pci_DB* DB;

   	bridges       = 0;
   	devices       = 0;
	level         = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(vChkSatellite)
	{
		cpci = (*vChkSatellite)();

		if(cpci)
		{
			if(vChkPeripheral)
			{
				if( (*vChkPeripheral)() == 0)
				{
					#ifdef DEBUG
						vConsoleWrite ("\nPeripheral Mode\n");
					#endif
					return E__OK;
				}
			}

			if(DB->Flags != 0x01)
			{
				scanInit(DB->Devices0);
				rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
				if(rt == E__OK)
					DB->Flags = 0x01;
				#ifdef DEBUG
					else
						vConsoleWrite("\nPCI On Board Devices Test Failed\n");
				#endif
			}

			if(DB->Flags == 0x01)
			{
				bridges = 0;
				devices = 0;
				level   = 0;
				rt      = scanCPCI(DB->Devices0, 1);
			}
		}
		#ifdef DEBUG
			else
				vConsoleWrite ("\nSatellite Mode\n");
		#endif
	}
	#ifdef DEBUG
		else
			vConsoleWrite ("\nNO CPCI Bridge On This Board\n");
	#endif

	return rt;
}







/*****************************************************************************
 * PciTest: Test the Presence of VPX bridge
 * RETURNS: None */
TEST_INTERFACE (VPXBridge, "VPX Bridge Presence")
{
	UINT32		rt;
	pci_DB*		DB;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper (0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite ("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(DB->Flags == 0x01)
	{
		rt = CpciBr(DB->Devices0);
	}

	return rt;
}



/*****************************************************************************
 * PciPXMCTest: Test the presence of devices on VPX Bus
 * RETURNS: None */
TEST_INTERFACE (PcieVPXTest, "PCIE VPX Bus Devices")
{
	UINT32  rt = E__OK;
	pci_DB* DB;

   	bridges       = 0;
   	devices       = 0;
	level         = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(DB->Flags == 0x01)
	{
		bridges = 0;
		devices = 0;
		level   = 0;
		rt      = scanCPCI(DB->Devices0, 0);
	}

	return rt;
}








/*****************************************************************************
 * scanPXMC: Scans PMC/XMC sites on AD CR2 PMC
 * RETURNS: None */

#define PCI_ERROR_PXMC		(E__BIT + 0x800)
static UINT8 SCANADCR2;

UINT32 scanADCR2(pci_Device* scanList, UINT8 pxmc)
{
	UINT32 rt = E__OK;
	UINT8  l;
	#ifdef DEBUG
		char   achBuffer[128];
	#endif

	l = 0;
	while((scanList[l].type != 0x00) && (rt == E__OK) && (SCANADCR2 == 0))
	{
		if((scanList[l].type & PCI_CARRIER) && (scanList[l].type & PCI_BRIDGE) &&
		   (scanList[l].Flags & PCI_FOUND)	)
		{
			SCANADCR2 = 1;

			if(pxmc)
			{
				bridges = 0;
				devices = 0;

				PciTestWrapper (scanList[l].Vars.sBUS, 0, 0, PCI_PXMCSCAN, NULL, &scanList[l], &scanList[l]);

				if(bridges > 0 || devices > 0)
					rt = E__OK;
				else
					rt = PCI_ERROR_PXMC;

				#ifdef DEBUG
					sprintf( achBuffer, "\nPXMC %x Bridges = %d, Devices = %d\n",
							 (scanList[l].functions & pxmc), bridges, devices);
					vConsoleWrite (achBuffer);
				#endif
			}

			break;
		}

		if((rt == E__OK) && (SCANADCR2 == 0))
			if(scanList[l].MDevices != NULL)
				rt =scanADCR2((pci_Device*)scanList[l].MDevices, pxmc);

		if((rt == E__OK) && (SCANADCR2 == 0))
			if(scanList[l].BDevices != NULL)
				rt = scanADCR2((pci_Device*)scanList[l].BDevices, pxmc);

		l++;
	}

	return rt;
}



/*****************************************************************************
 * PciPXMCTest: Test PCI PMC/XMC Site
 * This is a
 * RETURNS: None */
TEST_INTERFACE (PciADCR2Test, "AD CR2 PMC Test")
{
	UINT32  rt = E__OK;
	pci_DB* DB;
	UINT8   pxmc;

   	bridges       = 0;
   	devices       = 0;
	level         = 0;
	SCANADCR2     = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(vChkPeripheral)
	{
		pxmc = (*vChkPeripheral)();

		if(DB->Flags != 0x01)
		{
			scanInit(DB->Devices0);
			rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
			if(rt == E__OK)
				DB->Flags = 0x01;
			#ifdef DEBUG
				else
					vConsoleWrite("\nPCI On Board Devices Test Failed\n");
			#endif
		}

		if(DB->Flags == 0x01)
		{
			level = 0;
			rt    = scanADCR2(DB->Devices0, pxmc);
		}

		if ( (DB->Flags == 0x01) && (SCANADCR2 == 0))
		{
			vConsoleWrite ("PMC Carrier Bridge not Found\n");
			rt = 123;
		}
		else if ( (DB->Flags == 0x01) && (SCANADCR2 == 1))
		{
			if(pxmc == 0)
			{
				#ifdef DEBUG
					vConsoleWrite ("\nNO PMC/XMC Fitted on This Carrier Board\n");
				#endif
			}
		}
	}
	#ifdef DEBUG
		else
			vConsoleWrite ("\nNO PMC/XMC Carrier Support\n");
	#endif

	return rt;
}

/*****************************************************************************
 * PciPXMCTest: Test PCI PMC/XMC Site
 * This is a
 * RETURNS: None */
TEST_INTERFACE (PciADCRXTest, "AD CRX PMC Test")
{
	UINT32  rt = E__OK;
	pci_DB* DB;
	UINT8   pxmc;

   	bridges       = 0;
   	devices       = 0;
	level         = 0;
	SCANADCR2     = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(vChkPeripheral)
	{
		pxmc = (*vChkPeripheral)();

		if(DB->Flags != 0x01)
		{
			scanInit(DB->Devices0);
			rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
			if(rt == E__OK)
				DB->Flags = 0x01;
			#ifdef DEBUG
				else
					vConsoleWrite("\nPCI On Board Devices Test Failed\n");
			#endif
		}

		if(DB->Flags == 0x01)
		{
			level = 0;
			rt    = scanADCR2(DB->Devices0, pxmc);
		}

		if ( (DB->Flags == 0x01) && (SCANADCR2 == 0))
		{
			vConsoleWrite ("PMC Carrier Bridge not Found\n");
			rt = 123;
		}
		else if ( (DB->Flags == 0x01) && (SCANADCR2 == 1))
		{
			if(pxmc == 0)
			{
				#ifdef DEBUG
					vConsoleWrite ("\nNO PMC/XMC Fitted on This Carrier Board\n");
				#endif
			}
		}
	}
	#ifdef DEBUG
		else
			vConsoleWrite ("\nNO PMC/XMC Carrier Support\n");
	#endif

	return rt;
}



/*****************************************************************************
 * PciPXMCTest: Test PCI PMC/XMC Site
 * This is a
 * RETURNS: None */
TEST_INTERFACE (PciADCR5Test, "AD CR5 PMC Test")
{
	UINT32  rt = E__OK;
	pci_DB* DB;
	UINT8   pxmc = 0;
#ifdef DEBUG
		char	achBuffer[128];	/* text formatting buffer */
#endif

   	bridges       = 0;
   	devices       = 0;
	level         = 0;
	SCANADCR2     = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(vChkPeripheral)
	{
		pxmc = (*vChkPeripheral)();

		if(DB->Flags != 0x01)
		{
			scanInit(DB->Devices0);
			rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
			if(rt == E__OK)
				DB->Flags = 0x01;
			#ifdef DEBUG
				else
					vConsoleWrite("\nPCI On Board Devices Test Failed\n");
			#endif
		}

		if(DB->Flags == 0x01)
		{
			level = 0;
			rt    = scanADCR2(DB->Devices0, pxmc);
		}
		#ifdef DEBUG
			sprintf (achBuffer, "\n(DB->Flags = %d) && (SCANADCR2 = %d)",
						 DB->Flags ,SCANADCR2 );
			vConsoleWrite (achBuffer);
		#endif
		if(pxmc != 0)
		{
			if ( (DB->Flags == 0x01) && (SCANADCR2 == 0))
			{
				vConsoleWrite ("PMC Carrier Bridge not Found\n");
				rt = 123;
			}
		}
		else
		{
			if ( (DB->Flags == 0x01) && (SCANADCR2 == 1))
			{
				#ifdef DEBUG
					vConsoleWrite ("\nNO PMC/XMC Fitted on This Carrier Board\n");
				#endif
			}
			vConsoleWrite ("\nNO Second slot board interface available on this variant\n");
			return E__FAIL;
		}

	}
	#ifdef DEBUG
		else
			vConsoleWrite ("\nNO PMC/XMC Carrier Support\n");

	#endif

	return rt;
}



/*****************************************************************************
 * PciTest: Test the PCI On Board Devices
 * RETURNS: None */
// parameters ( "carrier type 1", "parent bridge 1")
TEST_INTERFACE (VpxCarrierTest, "VPX PMC/XMC Carrier Basic Test")
{
	UINT32  rt;
	pci_DB* DB;
	pci_DB* trXmcDB;
	UINT8	sBUS;
	UINT8	index = 0;
	PCIE_SW_CONFIG_INFO brInfo;

	board_service(SERVICE__BRD_CHK_PCIE_SW_CONFIG, NULL, &brInfo);

	//no bridge, only 1 x8 device!
	if(brInfo.count == 0)
		return TRXMC_NO_BRIDGE;

	//check if entered parameter is in range or else crash!!
	if(adTestParams[0] == 2)
	{
		if (adTestParams[1] == 1)
		{
			// bridges are numbered from 1 when given as parameter, locally they
			// are 0 based, but 0 is some times used by the device instance
			// in peripheral configuration
			if ((adTestParams[2] > 0) && (adTestParams[2] <= brInfo.count))
			{
				if(brInfo.tye == 0)
					index = adTestParams[2]-1;
				else
					index = adTestParams[2]; // skip device in peripheral config
			}
			else
				return TRXMC_WRONG_BRIDGE;
		}
		else
			return TRXMC_WRONG_CARRIER;
	}
	else if (adTestParams[0] > 0)
	{
		return TRXMC_BAD_PARAMETERS;
	}
	else
	{
		if(brInfo.tye == 0)
			index = 0;
		else
			index = 1; // skip device in peripheral config
	}

	bridges		  = 0;
   	devices		  = 0;
	level         = 0;
	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(DB->Flags == 0x01)
	{
		bridges	= 0;
   		devices = 0;
		level   = 0;

		board_service(SERVICE__BRD_GET_TRXMC_PCI_DB, NULL, &trXmcDB);
		pci_Erro_Base = trXmcDB->ERR_BASE;
		setPtrs(trXmcDB);
		scanInit(trXmcDB->Devices0);

		sBUS = index;
		board_service(SERVICE__BRD_GET_TRXMC_PARENT_BUS, NULL, &sBUS);

		rt = PciTestWrapper(sBUS, 0, 0, PCI_ONBOARDSCAN, trXmcDB->Devices0, NULL, NULL);
		if(rt == E__OK)
			trXmcDB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite("\nVPX PMC/XMC Carrier PCI On Board Devices Test Failed\n");
		#endif
	}

	return rt;
}




/*****************************************************************************
 * PciTest: Test the PCI Express Port Widths
 * RETURNS: None */
 // parameters ( "carrier type 1", "parent bridge", "max port width", "XMC present 1")

TEST_INTERFACE (trXmcPciPW, "VPX PMC/XMC Carrier PCIE Width")
{
	UINT32	rt;
	pci_DB*	DB;
	UINT8   pxmc = 0;
	UINT32	bkWidth = 0;
	pci_DB* trXmcDB;
	UINT8	sBUS = 0;
	UINT8	index = 0;
	PCIE_SW_CONFIG_INFO brInfo;

	pwLineCount   = 0;

	board_service(SERVICE__BRD_CHK_PCIE_SW_CONFIG, NULL, &brInfo);

	//no bridge, only 1 x8 device!
	if(brInfo.count == 0)
		return TRXMC_NO_BRIDGE;;

	//check if entered parameter is in range or else crash!!
	if(adTestParams[0] == 4)
	{
		if (adTestParams[1] == 1)// type 1 is TR XMC/30x
		{
			// bridges are numbered from 1 when given as parameter, locally they
			// are 0 based, but 0 is some times used by the device instance
			// in peripheral configuration
			if ((adTestParams[2] > 0) && (adTestParams[2] <= brInfo.count))
			{
				if(brInfo.tye == 0)
					index = adTestParams[2]-1;
				else
					index = adTestParams[2]; // skip device in peripheral config

				bkWidth = adTestParams[3];   // back plane lane width

				if(adTestParams[4] == 1)	 // is an XMC present
					pxmc = PXMC_XMC1;
			}
			else
				return TRXMC_WRONG_BRIDGE;
		}
		else
			return TRXMC_WRONG_CARRIER;
	}
	else if (adTestParams[0] > 0)
	{
		return TRXMC_BAD_PARAMETERS;
	}
	else
	{
		if(brInfo.tye == 0)
			index = 0;
		else
			index = 1; // skip device in peripheral config

		bkWidth = 0;  // no limit
		pxmc    = 0;  // no XMC
	}

	bridges		  = 0;
   	devices		  = 0;
	level         = 0;
	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(DB->Flags == 0x01)
	{
		bridges	= 0;
   		devices = 0;
		level   = 0;

		board_service(SERVICE__BRD_GET_TRXMC_PCI_DB, NULL, &trXmcDB);
		pci_Erro_Base = trXmcDB->ERR_BASE;
		setPtrs(trXmcDB);

		sBUS = index;
		board_service(SERVICE__BRD_GET_TRXMC_PARENT_BUS, NULL, &sBUS);

		if(trXmcDB->Flags != 0x01)
		{
			scanInit(trXmcDB->Devices0);

			rt = PciTestWrapper(sBUS, 0, 0, PCI_ONBOARDSCAN, trXmcDB->Devices0, NULL, NULL);
			if(rt == E__OK)
				trXmcDB->Flags = 0x01;
			#ifdef DEBUG
				else
					vConsoleWrite("\nVPX PMC/XMC Carrier PCI On Board Devices Test Failed\n");
			#endif
		}
	}


	if(trXmcDB->Flags == 0x01)
	{
		bridges	= 0;
   		devices = 0;
		level   = 0;

		if(pxmc)
			scanPXMC(trXmcDB->Devices0, pxmc, 0);

		bridges = 0;
		devices = 0;
		level   = 0;
		scanCPCI(trXmcDB->Devices0, 0);

		rt = scanPCIEwidth(trXmcDB->Devices0, bkWidth);
	}

	return rt;
}




/*****************************************************************************
 * PciTest: Test the PCI Express Port Widths
 * RETURNS: None */
 // parameters ( "carrier type", "parent bridge", "XMC 0/PMC 1")
TEST_INTERFACE (trXmcPXMCTest, "VPX PMC/XMC Carrier PMC/XMC Test")
{
	UINT32	rt;
	pci_DB*	DB;
	UINT8   pxmc = 0;
	pci_DB* trXmcDB;
	UINT8	sBUS = 0;
	UINT8	index = 0;
	PCIE_SW_CONFIG_INFO brInfo;

	board_service(SERVICE__BRD_CHK_PCIE_SW_CONFIG, NULL, &brInfo);

	//no bridge, only 1 x8 device!
	if(brInfo.count == 0)
		return TRXMC_NO_BRIDGE;

	//check if entered parameter is in range or else crash!!
	if(adTestParams[0] == 3)
	{
		if (adTestParams[1] == 1)// type 1 is TR XMC/30x
		{
			// bridges are numbered from 1 when given as parameter, locally they
			// are 0 based, but 0 is some times used by the device instance
			// in peripheral configuration
			if ((adTestParams[2] > 0) && (adTestParams[2] <= brInfo.count))
			{
				if(brInfo.tye == 0)
					index = adTestParams[2]-1;
				else
					index = adTestParams[2]; // skip device in peripheral config

				if(adTestParams[3] == 1)	 // is an XMC present
					pxmc = PXMC_PMC1;
				else
					pxmc = PXMC_XMC1;
			}
			else
				return TRXMC_WRONG_BRIDGE;
		}
		else
			return TRXMC_WRONG_CARRIER;
	}
	else if (adTestParams[0] > 0)
	{
		return TRXMC_BAD_PARAMETERS;
	}
	else
	{
		if(brInfo.tye == 0)
			index = 0;
		else
			index = 1; // skip device in peripheral config

		pxmc = PXMC_XMC1;
	}

	bridges		  = 0;
   	devices		  = 0;
	level         = 0;
	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(DB->Flags == 0x01)
	{
		bridges	= 0;
   		devices = 0;
		level   = 0;

		board_service(SERVICE__BRD_GET_TRXMC_PCI_DB, NULL, &trXmcDB);
		pci_Erro_Base = trXmcDB->ERR_BASE;
		setPtrs(trXmcDB);

		sBUS = index;
		board_service(SERVICE__BRD_GET_TRXMC_PARENT_BUS, NULL, &sBUS);

		if(trXmcDB->Flags != 0x01)
		{
			scanInit(trXmcDB->Devices0);

			rt = PciTestWrapper(sBUS, 0, 0, PCI_ONBOARDSCAN, trXmcDB->Devices0, NULL, NULL);
			if(rt == E__OK)
				trXmcDB->Flags = 0x01;
			#ifdef DEBUG
				else
					vConsoleWrite("\nVPX PMC/XMC Carrier PCI On Board Devices Test Failed\n");
			#endif
		}
	}


	if(trXmcDB->Flags == 0x01)
	{
		bridges	= 0;
   		devices = 0;
		level   = 0;

		if(pxmc)
			rt = scanPXMC(trXmcDB->Devices0, pxmc, 1);
	}

	return rt;
}





/*****************************************************************************
 * PciPXMCTest: Test the presence of devices on VPX Bus
 * RETURNS: None */
TEST_INTERFACE (VxsPcieConnectivity, "PCIE VXS Backplane Connectivity")
{
	UINT32  rt = E__OK;
	pci_DB* DB;

   	bridges       = 0;
   	devices       = 0;
	level         = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(DB->Flags == 0x01)
	{
		bridges = 0;
		devices = 0;
		level   = 0;
		rt      = scanCPCI(DB->Devices0, 1);
	}

	return rt;
}


/*****************************************************************************
 * vsxLoopBack: Performs a block write/read on the VXS bus
 * RETURNS: E__OK or error code */
static UINT32 vsxLoopBack(UINT32 bar)
{
	UINT32  	rt = E__OK;
	PTR48  tPtr;
	UINT32 mHandle;
	UINT32 pointer;
	UINT32 lbar;
	UINT32 i;
	UINT32 temp;
#ifdef DEBUG
	char   buffer[64];
#endif

	lbar = bar & 0xfffffff0;
	#ifdef DEBUG
		sprintf(buffer, "BAR %x\n", lbar );
		vConsoleWrite(buffer);
	#endif

	sysCacheDisable();

	mHandle  = dGetPhysPtr(lbar, 0x400000, &tPtr,(void*)(&pointer));

	if(mHandle != NULL)
	{
		for(i = 0; i<0x400000; i+=4)
		{
			 if (i % (0x40000) == 0)
			 {
				 vConsoleWrite(".");
			 }

			temp = ((i << 24) | (i << 16) | (i << 8) | i);
			*((UINT32*)(pointer+i)) = temp;

			if(temp != *((UINT32*)(pointer+i)) )
			{
				rt = VXSPCIE_LOOPBACK_DATA_ERROR;
				break;
			}
		}
	}
	else
		rt =VXSPCIE_LOOPBACK_MEM_ALLOCATION;

	vFreePtr(mHandle);
	sysCacheEnable();

	return rt;
}


/*****************************************************************************
 * PciPXMCTestA: perform a loopback on the device on port A
 * RETURNS: E__OK or error code */
TEST_INTERFACE (VxsPcieLoopBackA, "PCIE VXS Port A LoopBack")
{
	UINT32  	rt = E__OK;
	pci_DB* 	DB;
	VXS_T_PORTS ports;
	PCI_PFA		pfa;
	UINT16      reg16;
	UINT32      reg32;
#ifdef DEBUG
	char    	buffer[64];
#endif
   	bridges       = 0;
   	devices       = 0;
	level         = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(rt == E__OK)
	{
		memset(&ports, 0x00, sizeof(VXS_T_PORTS));
		board_service(SERVICE__BRD_GET_VXS_TRANS_PORTS, NULL, &ports);

		ports.portA->Flags 	&= ~PCI_PXMCdone;
		ports.portA->PXMCPFA = NULL;

		ports.portB->Flags 	&= ~PCI_PXMCdone;
		ports.portB->PXMCPFA = NULL;

		if(DB->Flags == 0x01)
		{
			bridges = 0;
			devices = 0;
			level   = 0;
			rt      = scanCPCI(DB->Devices0, 0);
		}
	}

	//Test port A
	if( rt == E__OK)
	{
		#ifdef DEBUG
			sprintf(buffer, "%s\n", ports.portA->Name );
			vConsoleWrite(buffer);
		#endif

		if(ports.portA->Flags & PCI_FOUND)
		{
			if(ports.portA->Flags & PCI_PXMCdone)
			{
				pfa = ports.portA->PXMCPFA;
				reg32 = PCI_READ_DWORD (pfa, 0);
				if(reg32 != 0xffffffff)
				{
					#ifdef DEBUG
						sprintf(buffer, "NT Device found %x\n", reg32 );
						vConsoleWrite(buffer);
					#endif

					reg16 = PCI_READ_WORD (pfa, 4);
					reg16 |= 0x0007;
					PCI_WRITE_WORD (pfa, 4, reg16);

					//read bar 14, if not present no window has been configured
					reg32 = PCI_READ_DWORD (pfa, 0x14);
					if(reg32 != 0)
					{
						rt = vsxLoopBack(reg32);
					}
					else
					{
						vConsoleWrite("BAR Not Allocated On Device On Port A\n");
						rt = VXSPCIE_LOOPBACK_NO_MEM_BAR;
					}
				}
				else
				{
					vConsoleWrite("Device Not Accessible on Port A\n");
					rt = VXSPCIE_LOOPBACK_NO_DEVICE_ACCESS;
				}
			}
			else
			{
				vConsoleWrite("No NT Device Found on Port A\n");
				rt = VXSPCIE_LOOPBACK_NO_NT_DEVICE_A;
			}
		}
		else
		{
			vConsoleWrite("Transparent Port A NOT Present\n");
			rt = VXSPCIE_LOOPBACK_NO_PORT_A;
		}
	}

	return rt;
}


/*****************************************************************************
 * PciPXMCTestA: perform a loopback on the device on port B
 * RETURNS: E__OK or error code */
TEST_INTERFACE (VxsPcieLoopBackB, "PCIE VXS Port B LoopBack")
{
	UINT32  	rt = E__OK;
	pci_DB* 	DB;
	VXS_T_PORTS ports;
	PCI_PFA		pfa;
	UINT16      reg16;
	UINT32      reg32;
#ifdef DEBUG
	char    	buffer[64];
#endif
   	bridges       = 0;
   	devices       = 0;
	level         = 0;

	board_service(SERVICE__BRD_GET_PCI_DB, NULL, &DB);
	pci_Erro_Base = DB->ERR_BASE;
	setPtrs(DB);

	if(DB->Flags != 0x01)
	{
		scanInit(DB->Devices0);
		rt = PciTestWrapper(0, 0, 0, PCI_ONBOARDSCAN, DB->Devices0, NULL, NULL);
		if(rt == E__OK)
			DB->Flags = 0x01;
		#ifdef DEBUG
			else
				vConsoleWrite("\nPCI On Board Devices Test Failed\n");
		#endif
	}

	if(rt == E__OK)
	{
		memset(&ports, 0x00, sizeof(VXS_T_PORTS));
		board_service(SERVICE__BRD_GET_VXS_TRANS_PORTS, NULL, &ports);

		ports.portA->Flags 	&= ~PCI_PXMCdone;
		ports.portA->PXMCPFA = NULL;

		ports.portB->Flags 	&= ~PCI_PXMCdone;
		ports.portB->PXMCPFA = NULL;

		if(DB->Flags == 0x01)
		{
			bridges = 0;
			devices = 0;
			level   = 0;
			rt      = scanCPCI(DB->Devices0, 0);
		}
	}

	//Test port B
	if(rt == E__OK)
	{
		#ifdef DEBUG
			sprintf(buffer, "%s\n", ports.portB->Name );
			vConsoleWrite(buffer);
		#endif

		if(ports.portB->Flags & PCI_FOUND)
		{
			if(ports.portB->Flags & PCI_PXMCdone)
			{
				pfa = ports.portB->PXMCPFA;
				reg32 = PCI_READ_DWORD (pfa, 0);
				if(reg32 != 0xffffffff)
				{
					#ifdef DEBUG
						sprintf(buffer, "NT Device found %x\n", reg32 );
						vConsoleWrite(buffer);
					#endif

					reg16 = PCI_READ_WORD (pfa, 4);
					reg16 |= 0x0007;
					PCI_WRITE_WORD (pfa, 4, reg16);

					//read bar 14, if not present no window has been configured
					reg32 = PCI_READ_DWORD (pfa, 0x14);
					if(reg32 != 0)
					{
						rt = vsxLoopBack(reg32);
					}
					else
					{
						vConsoleWrite("BAR Not Allocated On Device On Port B\n");
						rt = VXSPCIE_LOOPBACK_NO_MEM_BAR;
					}
				}
				else
				{
					vConsoleWrite("Device Not Accessible on Port B\n");
					rt = VXSPCIE_LOOPBACK_NO_DEVICE_ACCESS;
				}
			}
			else
			{
				vConsoleWrite("No NT Device Found on Port B\n");
				rt = VXSPCIE_LOOPBACK_NO_NT_DEVICE_B;
			}
		}
		else
		{
			vConsoleWrite("Transparent Port B NOT Present\n");
			rt = VXSPCIE_LOOPBACK_NO_PORT_B;
		}
	}


	return rt;
}

static UINT32 wDramReadWriteBlock( volatile UINT32 dAddr, UINT32 len  )
{
	UINT32	dEndAddr;
	UINT32	dData;
	UINT32	dTemp;			/* holds original memory contents */

	dEndAddr = dAddr + len;

	while (dAddr < dEndAddr)
	{
		dTemp = *(UINT32*)dAddr;		/* read original data */
		*(UINT32*)dAddr = ~dTemp;		/* write compliment */
		dData = *(UINT32*)dAddr;		/* re-read */

		if (dData != ~dTemp)			/* compare */
		{
        	return (AMC_DAT_RDWR_ERR);
		}

		*(UINT32*)dAddr = dTemp;		/* write back original data */
		dData = *(UINT32*)dAddr;		/* re-read */

		if (dData != dTemp)				/* compare */
		{
			return (AMC_DAT_RDWR_ERR);
		}

		dAddr += 4;
	}
	return (E__OK);

} /* wDramReadWriteBlock () */

/*****************************************************************************\
 *
 *  TITLE:  amcPcieDataTxfer
 *
 *  ABSTRACT:  test function that always returns E__OK
 *
 * 	RETURNS: E__OK.
 *
\*****************************************************************************/
TEST_INTERFACE (amcPcieDataTxfer, "AMC PCIe Data Transfer Test")
{
	PCI_PFA pfa;
	UINT32 regVal;
	UINT32 result;
	char	achBuffer[80];
	volatile UINT32 mapAddr;
	PTR48 tPtr;
	UINT32 handle;

	if (E__OK == iPciFindDeviceById (1, PERICOM_VID, X110_DID ,&pfa))
	{
		sprintf( achBuffer, "Found Pericom X110 NT Bridge\n");
		vConsoleWrite (achBuffer);

		PCI_WRITE_WORD (pfa, PCI_COMMAND, PCI_BMEN + PCI_MEMEN + PCI_IOEN);
		regVal = PCI_READ_DWORD (pfa, PCI_BAR2);

		if( regVal & ~0xF )
		{
			handle  = dGetPhysPtr ((regVal & ~0xF), TEST_BLOCK_LENGTH, &tPtr, (void*)(&mapAddr));

			if( handle == E__FAIL )
			{
				sprintf( achBuffer, "Unable to get map BAR2\n");
				vConsoleWrite (achBuffer);

				return (AMC_TEST_NO_MEM);
			}
			else
			{
				sprintf( achBuffer, "Carrying out RW test on remote memory mapped at %08X\n",regVal);
				vConsoleWrite (achBuffer);

				result = wDramReadWriteBlock(mapAddr,TEST_BLOCK_LENGTH);
				vFreePtr(handle);

				if( result == E__OK )
				{
					sprintf( achBuffer, "Remote memory r/w test - Passed \n");
				}
				else
				{
					sprintf( achBuffer, "Remote memory r/w test - Failed\n");
				}

				vConsoleWrite (achBuffer);

				return result;
			}
		}
		else
		{
			sprintf( achBuffer, "BAR 2 of X110 bridge has not been configured.\n");
			vConsoleWrite (achBuffer);

			return (AMC_DAT_CONFIG_ERR);
		}
	}
	else
	{
		sprintf( achBuffer, "Pericom X110 Pcie-Pci bridge not found.\n");
		vConsoleWrite (achBuffer);
		return (AMC_BRIDGE_NOT_FOUND_ERR);
	}
}


/*****************************************************************************\
 *
 *  TITLE:  amcPcieLinkTest
 *
 *  ABSTRACT:
 *
 * 	RETURNS: E__OK.
 *
\*****************************************************************************/
TEST_INTERFACE (amcPcieLinkTest, "AMC PCIe Link Test")
{
	UINT8 negWidth,negSpeed;
	UINT8 maxWidth,maxSpeed;
	UINT32 result;
	UINT32  reg, temp;
	AMC_PCIE_LINK_INFO portInfo;
	char	achBuffer[80];

	result = board_service(SERVICE__BRD_GET_AMC_PCIE_LINKFINFO, NULL, &portInfo);

	if( result != SERVICE_NOT_SUPPORTED)
	{
		reg  = PCI_READ_BYTE (portInfo.Pfa, 0x34);
		temp = PCI_READ_DWORD (portInfo.Pfa, reg);

		while( ((temp & 0x000000ff) != 0x10)  && (temp != 0xff) )
		{
			reg = ((temp >> 8) & 0x000000ff	);
			if((reg == 0xff) || (reg == 0x00))
			{
				temp = 0xff;
				result   = PCI_ERROR_WIDTH_NO_CAPABILITY;
			}
			else
				temp = PCI_READ_DWORD (portInfo.Pfa, reg);
		}

		if(temp != 0xff)
		{
			temp = PCI_READ_WORD (portInfo.Pfa, reg+0xC);
			maxWidth = (temp >> 4) & 0x003f;
			maxSpeed = temp & 0x000f;

			temp = PCI_READ_WORD (portInfo.Pfa, reg+0x12);
			negWidth = (temp >> 4) & 0x003f;
			negSpeed = temp & 0x000f;

			if( negWidth != maxWidth )
			{
				sprintf( achBuffer, "Incorrect PCIe link width. Expected x%d: Negotiated x%d\n",maxWidth,negWidth);
				vConsoleWrite (achBuffer);
				return ((AMC_PCIE_LINKWIDTH_ERR)|negWidth);
			}

			if( portInfo.gen != 0  && negSpeed != portInfo.gen )
			{
				sprintf( achBuffer, "Incorrect PCIe link speed. Expected %d: Negotiated %d\n",portInfo.gen,negSpeed);
				return ((AMC_PCIE_LINKSPEED_ERR)|negSpeed);
			}
			else if( portInfo.gen == 0 && negSpeed != maxSpeed )
			{
				sprintf( achBuffer, "Incorrect PCIe link speed. Expected %d: Negotiated %d\n",maxSpeed,negSpeed);
				return ((AMC_PCIE_LINKSPEED_ERR)|negSpeed);
			}
			else
			{
				return E__OK;
			}
		}
		else
		{
			return (AMC_UNSUPPORTED_SERVICE);
		}
	}
	else
	{
		return (AMC_UNSUPPORTED_SERVICE);
	}
}
