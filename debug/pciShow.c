/*******************************************************************************
*
* Filename:	 pciShow.c
*
* Description:	PCI device show function, for debug use only.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 07:20:17 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/debug/pciShow.c,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/


#include <stdtypes.h>
#include <private/debug.h>

#include <bit/pci.h>

#include "FreeRTOS.h"


#undef PCISHOW_DEBUG

#ifdef PCISHOW_DEBUG
#warning "***** DEBUG ON *****" 
#define PCIDBG(x)	do {sysDebugPrintf x;} while(0)
#else
#define PCIDBG(x)
#endif

#ifdef PCISHOW_DEBUG

#define PCI_MAX_BUS   255
#define PCI_MAX_DEV   32
#define PCI_MAX_FUNC  8

/* PCI header type bits */

#define PCI_HEADER_TYPE_MASK    0x7f    /* mask for header type */
#define PCI_HEADER_PCI_CARDBUS  0x02    /* PCI to CardBus bridge */
#define PCI_HEADER_PCI_PCI      0x01    /* PCI to PCI bridge */
#define PCI_HEADER_TYPE_BRIDGE  0x01    /* Header Type 1 is not only a PCI2PCI bridge */
#define PCI_HEADER_TYPE0        0x00    /* normal device header */
#define PCI_HEADER_MULTI_FUNC   0x80    /* multi function device */

/* Standard device Type 0 configuration register offsets */

#define PCI_CFG_VENDOR_ID       0x00
#define PCI_CFG_DEVICE_ID       0x02
#define PCI_CFG_COMMAND         0x04
#define PCI_CFG_STATUS          0x06
#define PCI_CFG_REVISION        0x08
#define PCI_CFG_PROGRAMMING_IF  0x09
#define PCI_CFG_SUBCLASS        0x0a
#define PCI_CFG_CLASS           0x0b
#define PCI_CFG_CACHE_LINE_SIZE 0x0c
#define PCI_CFG_LATENCY_TIMER   0x0d
#define PCI_CFG_HEADER_TYPE     0x0e
#define PCI_CFG_BIST            0x0f
#define PCI_CFG_BASE_ADDRESS_0  0x10
#define PCI_CFG_BASE_ADDRESS_1  0x14
#define PCI_CFG_BASE_ADDRESS_2  0x18
#define PCI_CFG_BASE_ADDRESS_3  0x1c
#define PCI_CFG_BASE_ADDRESS_4  0x20
#define PCI_CFG_BASE_ADDRESS_5  0x24
#define PCI_CFG_CIS             0x28
#define PCI_CFG_SUB_VENDER_ID   0x2c
#define PCI_CFG_SUB_SYSTEM_ID   0x2e
#define PCI_CFG_EXPANSION_ROM   0x30
#define PCI_CFG_CAP_PTR         0x34
#define PCI_CFG_RESERVED_0      0x35
#define PCI_CFG_RESERVED_1      0x38
#define PCI_CFG_DEV_INT_LINE    0x3c
#define PCI_CFG_DEV_INT_PIN     0x3d
#define PCI_CFG_MIN_GRANT       0x3e
#define PCI_CFG_MAX_LATENCY     0x3f
#define PCI_CFG_SPECIAL_USE     0x41
#define PCI_CFG_MODE            0x43

/* PCI-to-PCI bridge Type 1 configuration register offsets */

#define PCI_CFG_PRIMARY_BUS     0x18
#define PCI_CFG_SECONDARY_BUS   0x19
#define PCI_CFG_SUBORDINATE_BUS 0x1a
#define PCI_CFG_SEC_LATENCY     0x1b
#define PCI_CFG_IO_BASE         0x1c
#define PCI_CFG_IO_LIMIT        0x1d
#define PCI_CFG_SEC_STATUS      0x1e
#define PCI_CFG_MEM_BASE        0x20
#define PCI_CFG_MEM_LIMIT       0x22
#define PCI_CFG_PRE_MEM_BASE    0x24
#define PCI_CFG_PRE_MEM_LIMIT   0x26
#define PCI_CFG_PRE_MEM_BASE_U  0x28
#define PCI_CFG_PRE_MEM_LIMIT_U 0x2c
#define PCI_CFG_IO_BASE_U       0x30
#define PCI_CFG_IO_LIMIT_U      0x32
#define PCI_CFG_ROM_BASE        0x38
#define PCI_CFG_BRG_INT_LINE    0x3c
#define PCI_CFG_BRG_INT_PIN     0x3d
#define PCI_CFG_BRIDGE_CONTROL  0x3e

/* PCI base address mask bits */

#define PCI_MEMBASE_MASK    ~0xf  /* mask for memory base address */
#define PCI_IOBASE_MASK     ~0x3  /* mask for IO base address */
#define PCI_BASE_IO          0x1  /* IO space indicator */
#define PCI_BASE_BELOW_1M    0x2  /* memory locate below 1MB */
#define PCI_BASE_IN_64BITS   0x4  /* memory locate anywhere in 64 bits */
#define PCI_BASE_PREFETCH    0x8  /* memory prefetchable */


static const char *pLineStr = "-------------------------------------------------------------------------------\n";

/*******************************************************************************
*
* pciGetStorageDeviceName
*
* Get storage device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetStorageDeviceName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "SCSI bus controller"; break;
		case 1: result = "IDE controller"; break;
		case 2: result = "Floppy disk controller"; break;
		case 3: result = "IPI bus controller"; break;
		case 4: result = "RAID controller"; break;
		case 0x80: result = "Other storage device"; break;
		default: result = "Unknown storage device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetNetworkDeviceName
*
* Get network device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetNetworkDeviceName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Ethernet controller"; break;
		case 1: result = "Token Ring controller"; break;
		case 2: result = "FDDI controller"; break;
		case 3: result = "ATM controller"; break;
		case 4: result = "ISDN controller"; break;
		case 0x80: result = "Other network devce"; break;
		default: result = "Unknown network device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetDisplayDeviceName
*
* Get display device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetDisplayDeviceName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "VGA controller"; break;
		case 1: result = "XGA controller"; break;
		case 2: result = "3D controller"; break;
		case 0x80: result = "Other display device"; break;
		default: result = "Unknown display device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetMultimediaDeviceName
*
* Get multimedia device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetMultimediaDeviceName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Video device"; break;
		case 1: result = "Audio device"; break;
		case 2: result = "Telephony device"; break;
		case 0x80: result = "Other multimedia device"; break;
		default: result = "Unknown multimedia device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetMemoryControllerName
*
* Get memory device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetMemoryControllerName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "RAM controller"; break;
		case 1: result = "Flash controller"; break;
		case 0x80: result = "Other memory device"; break;
		default: result = "Unknown memory device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetBridgeName
*
* Get bridge device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetBridgeName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Host bridge"; break;
		case 1: result = "ISA bridge"; break;
		case 2: result = "EISA bridge"; break;
		case 3: result = "MCA bridge"; break;
		case 4: result = "PCI-PCI bridge"; break;
		case 5: result = "PCMCIA bridge"; break;
		case 6: result = "NuBus bridge"; break;
		case 7: result = "CardBus bridge"; break;
		case 8: result = "RACEway bridge"; break;
		case 9: result = "PCI-PCI bridge (Semi-transparent)"; break;
		case 0x80: result = "Other bridge device"; break;
		default: result = "Unknown bridge device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetCommsDeviceName
*
* Get comms device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetCommsDeviceName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Serial controller"; break;
		case 1: result = "Parallel port"; break;
		case 2: result = "Multiport serial controller"; break;
		case 3: result = "Modem"; break;
		case 0x80: result = "Other Comms device"; break;
		default: result = "Unknown Comms device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetSystemDeviceName
*
* Get system device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetSystemDeviceName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Interrupt controller"; break;
		case 1: result = "DMA controller"; break;
		case 2: result = "System timer"; break;
		case 3: result = "RTC controller"; break;
		case 4: result = "PCI hotplug controller"; break;
		case 0x80: result = "Other system device"; break;
		default: result = "Unknown system device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetInputDeviceName
*
* Get input device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetInputDeviceName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Keyboard controller"; break;
		case 1: result = "Digitizer"; break;
		case 2: result = "Mouse controller"; break;
		case 3: result = "Scanner controller"; break;
		case 4: result = "Gameport controller"; break;
		case 0x80: result = "Other input device"; break;
		default: result = "Unknown input device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetDockingStationName
*
* Get docking station name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetDockingStationName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Generic docking station"; break;
		case 0x80: result = "Other docking station"; break;
		default: result = "Unknown docking station"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetProcessorName
*
* Get processor device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetProcessorName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "386 processor"; break;
		case 1: result = "486 processor"; break;
		case 2: result = "Pentium processor"; break;
		case 0x10: result = "Alpha processor"; break;
		case 0x20: result = "PowerPC processor"; break;
		case 0x30: result = "MIPS processor"; break;
		case 0x40: result = "Co-processor"; break;
		default: result = "Unknown processor"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetSerialBusControllerName
*
* Get serial bus device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetSerialBusControllerName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "IEEE 1394 controller"; break;
		case 1: result = "ACCESS.bus controller"; break;
		case 2: result = "SSA controller"; break;
		case 3: result = "USB controller"; break;
		case 4: result = "Fibre channel controller"; break;
		case 5: result = "SMBus controller"; break;
		default: result = "Unknown serial device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetWirelessControllerName
*
* Get wireless device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetWirelessControllerName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "iRDA controller"; break;
		case 1: result = "IR controller"; break;
		case 2: result = "RF controller"; break;
		case 0x80: result = "Other wireless device"; break;
		default: result = "Unknown wireless device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetIOControllerName
*
* Get I/O controller device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetIOControllerName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Intelligent I/O controller"; break;
		default: result = "Unknown I/O device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetSateliteControllerName
*
* Get satelite controller device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetSateliteControllerName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 1: result = "TV satelite controller"; break;
		case 2: result = "Audio satelite controller"; break;
		case 3: result = "Voice satelite controller"; break;
		case 4: result = "Data satelite controller"; break;
		default: result = "Unknown satelite device"; break;
	}

	return result;
}


/*******************************************************************************
*
* pciGetSecurityControllerName
*
* Get security controller device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetSecurityControllerName( UINT8 subClass )
{
	char *result;
	
	switch ( subClass )
	{
		case 0: result = "Network en/decryption controller"; break;
		case 0x10: result = "Entertainment en/decryption controller"; break;
		case 0x80: result = "Other en/decryption controller"; break;
		default: result = "Unknown en/decryption controller"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetDataAcqusitionControllerName
*
* Get data acquistion device name for the given subclass. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetDataAcqusitionControllerName( UINT8 subClass )
{
	char *result;
	
	
	switch ( subClass )
	{
		case 0: result = "DPIO module"; break;		
		case 0x80: result = "Other data acquisition device";break;
		default: result = "Unknown data acquisition device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetDeviceClassName
*
* Get PCI device description for the given device class. 
* 
*
*
* RETURNS: pointer to device description.
*
*******************************************************************************/
static char *pciGetDeviceClassName( UINT32 classCode )
{
	UINT8 baseClass;
	UINT8 subClass;
	char *result;


	baseClass = (UINT8) (classCode >> 16);
	subClass =  (UINT8) ((classCode >> 8) & 0xff);

	/* PCIDBG(("(%02x  %02x) ", baseClass, subClass)); */

	switch ( baseClass )
	{
		case 0x1: result = pciGetStorageDeviceName( subClass ); break;
		case 0x2: result = pciGetNetworkDeviceName( subClass ); break;
		case 0x3: result = pciGetDisplayDeviceName( subClass ); break;
		case 0x4: result = pciGetMultimediaDeviceName( subClass ); break;
		case 0x5: result = pciGetMemoryControllerName( subClass ); break;
		case 0x6: result = pciGetBridgeName( subClass ); break;
		case 0x7: result = pciGetCommsDeviceName( subClass ); break;
		case 0x8: result = pciGetSystemDeviceName( subClass ); break;
		case 0x9: result = pciGetInputDeviceName( subClass ); break;
		case 0xa: result = pciGetDockingStationName( subClass ); break;
		case 0xb: result = pciGetProcessorName( subClass ); break;
		case 0xc: result = pciGetSerialBusControllerName( subClass ); break;
		case 0xd: result = pciGetWirelessControllerName( subClass ); break;
		case 0xe: result = pciGetIOControllerName( subClass ); break;
		case 0xf: result = pciGetSateliteControllerName( subClass ); break;
		case 0x10: result = pciGetSecurityControllerName( subClass ); break;
		case 0x11: result = pciGetDataAcqusitionControllerName( subClass ); break;

		default : result = "Unknown device"; break;
	}
	
	return result;
}


/*******************************************************************************
*
* pciGetMemSize
*
* Find the size from the given PCI device's BAR value.
* 
*
*
* RETURNS: size.
*
*******************************************************************************/
static UINT32 pciGetMemSize( UINT32 base, UINT32 maxbase, UINT32 mask )
{
	UINT32 size = 0;
	
	
	if ( (maxbase != 0) && (maxbase != 0xffffffff) )
	{		
		size = maxbase & mask; /* Find the significant bits */
		
		if (size > 0)
		{
			/* Get the lowest of them to find the decode size */
			/* and from that the extent.                     */
			size = (size & ~(size-1)) - 1;
		
			/* base == maxbase can be valid only if the */
			/* BAR has been programmed with all 1s.     */
			if (base == maxbase && ((base | size) & mask) != mask)
			{
				return 0;
			}
		}
	}
	
	return size;
}


/*******************************************************************************
*
* pciBaseAddressShow
*
* Show given devices base address information.
* 
*
*
* RETURNS: None.
*
*******************************************************************************/
static void pciBaseAddressShow( PCI_PFA pfa, UINT32 offset, char* msg )
{
	UINT32 startAddr;
	UINT32 endAddr;
	UINT32 size;


	startAddr = dPciReadReg(pfa, offset, REG_32);
	
	if ( (startAddr > 0) && (startAddr != 0xffffffff) )
	{
		vPciWriteReg(pfa, offset, REG_32, 0xffffffff);
		size = dPciReadReg(pfa, offset, REG_32);
		vPciWriteReg(pfa, offset, REG_32, startAddr);
	
		if (startAddr & PCI_BASE_IO)
		{
			size = pciGetMemSize( startAddr, size, (PCI_IOBASE_MASK & 0xffff) );
			
			startAddr &= PCI_IOBASE_MASK;
			endAddr = startAddr + size;
			
			if ( (startAddr > 0) && (size > 0) )
			{
				size += 1;
			
				if (size < 1024)
				{
			    	PCIDBG(("%s: 0x%08x-0x%08x I/O %u bytes\n", msg, startAddr, endAddr, size ));
			    }
			  	else if (size < 1048576)
			  	{
			    	PCIDBG(("%s: 0x%08x-0x%08x I/O %u KB\n", msg, startAddr, endAddr, (size/1024) ));
			    }
			  	else
				{ 
					PCIDBG(("%s: 0x%08x-0x%08x I/O %u MB\n", msg, startAddr, endAddr, (size/1048576) ));
				}
			}
			else
			{
				PCIDBG(("%s: 0x%08x\n", msg, startAddr ));
			}
		}
		else
		{
			size = pciGetMemSize( startAddr, size, PCI_MEMBASE_MASK );
			
			startAddr &= PCI_MEMBASE_MASK;
			endAddr = startAddr + size;
			
			if ( (startAddr > 0) && (size > 0) )
			{
				size += 1;
			
				if (size < 1024)
				{
			    	PCIDBG(("%s: 0x%08x-0x%08x MEM %u bytes\n", msg, startAddr, endAddr, size ));
			    }
			  	else if (size < 1048576)
			  	{
			    	PCIDBG(("%s: 0x%08x-0x%08x MEM %u KB\n", msg, startAddr, endAddr, (size/1024) ));
			    }
			  	else
				{ 
					PCIDBG(("%s: 0x%08x-0x%08x MEM %u MB\n", msg, startAddr, endAddr, (size/1048576) ));
				}
			}
			else
			{
				PCIDBG(("%s: 0x%08x\n", msg, startAddr));
			}
		}
	}
	else
	{
		PCIDBG(("%s: 0x%08x\n", msg, startAddr ));
	}
}


/*******************************************************************************
*
* pciHeaderShow
*
* Show given devices header information.
* 
*
*
* RETURNS: None.
*
*******************************************************************************/
static void pciHeaderShow( PCI_PFA pfa, UINT32 classCode, UINT8 header )
{
/*
	PCIDBG(("   Vendor ID  : 0x%04x  Device ID : 0x%04x\n", 
			dPciReadReg(pfa, PCI_CFG_VENDOR_ID, REG_16), 
			dPciReadReg(pfa, PCI_CFG_DEVICE_ID, REG_16) ));
*/
			
	PCIDBG(("   Command    : 0x%04x  Status    : 0x%04x\n", 
				dPciReadReg(pfa, PCI_CFG_COMMAND, REG_16), 
				dPciReadReg(pfa, PCI_CFG_STATUS, REG_16) ));
			
	PCIDBG(("   Revision ID: 0x%02x    Class Code: 0x%06x\n", 
				dPciReadReg(pfa, PCI_CFG_REVISION, REG_8), 
				classCode ));
	
	PCIDBG(("   Cache Line Size: 0x%02x  Latency Timer: 0x%02x  Header Type: 0x%02x  BIST: 0x%02x\n", 
				dPciReadReg(pfa, PCI_CFG_CACHE_LINE_SIZE, REG_8), 
				dPciReadReg(pfa, PCI_CFG_LATENCY_TIMER, REG_8),
				header,
				dPciReadReg(pfa, PCI_CFG_BIST, REG_8) ));

	pciBaseAddressShow( pfa, PCI_CFG_BASE_ADDRESS_0, "   Base Address 0" );
	pciBaseAddressShow( pfa, PCI_CFG_BASE_ADDRESS_1, "   Base Address 1" );
	
	if ( (header & PCI_HEADER_PCI_PCI) == PCI_HEADER_PCI_PCI )
	{
		PCIDBG(("   Primary Bus: 0x%02x  Secondary Bus: 0x%02x\n", 
				dPciReadReg(pfa, PCI_CFG_PRIMARY_BUS, REG_8), 
				dPciReadReg(pfa, PCI_CFG_SECONDARY_BUS, REG_8) ));

		PCIDBG(("   Subordinate Bus: 0x%02x Secondary Latency Timer: 0x%02x\n", 
				dPciReadReg(pfa, PCI_CFG_SUBORDINATE_BUS, REG_8), 
				dPciReadReg(pfa, PCI_CFG_SEC_LATENCY, REG_8) ));

		PCIDBG(("   IO Base: 0x%02x  IO Limit: 0x%02x  Secondary Status: 0x%04x\n", 
				dPciReadReg(pfa, PCI_CFG_IO_BASE, REG_8), 
				dPciReadReg(pfa, PCI_CFG_IO_LIMIT, REG_8),
				dPciReadReg(pfa, PCI_CFG_SEC_STATUS, REG_16) ));
		
		PCIDBG(("   Memory Base: 0x%04x  Memory Limit: 0x%04x\n", 
				dPciReadReg(pfa, PCI_CFG_MEM_BASE, REG_16), 
				dPciReadReg(pfa, PCI_CFG_MEM_LIMIT, REG_16) ));

		PCIDBG(("   Prefetch Memory Base: 0x%04x  Prefetch Memory Limit: 0x%04x\n", 
				dPciReadReg(pfa, PCI_CFG_PRE_MEM_BASE, REG_16), 
				dPciReadReg(pfa, PCI_CFG_PRE_MEM_LIMIT, REG_16) ));
		
		PCIDBG(("   Prefetch Base  Upper 32 Bits: 0x%08x\n", 
				dPciReadReg(pfa, PCI_CFG_PRE_MEM_BASE_U, REG_32) ));
		PCIDBG(("   Prefetch Limit Upper 32 Bits: 0x%08x\n", 
				dPciReadReg(pfa, PCI_CFG_PRE_MEM_LIMIT_U, REG_32) ));

		PCIDBG(("   I/O Base Upper 32 Bits: 0x%04x  I/O Limit Upper 32 Bits: 0x%04x\n", 
				dPciReadReg(pfa, PCI_CFG_IO_BASE_U, REG_16), 
				dPciReadReg(pfa, PCI_CFG_IO_LIMIT_U, REG_16) ));

		PCIDBG(("   Expansion ROM Base Address: 0x%08x\n", 
				dPciReadReg(pfa, PCI_CFG_ROM_BASE, REG_32) ));
		
		PCIDBG(("   Interrupt Line: 0x%02x  Interrupt Pin: 0x%02x  Bridge Control: 0x%04x\n", 
				dPciReadReg(pfa, PCI_CFG_DEV_INT_LINE, REG_8), 
				dPciReadReg(pfa, PCI_CFG_DEV_INT_PIN, REG_8),
				dPciReadReg(pfa, PCI_CFG_BRIDGE_CONTROL, REG_16) ));
	}
	else
	{
		pciBaseAddressShow( pfa, PCI_CFG_BASE_ADDRESS_2, "   Base Address 2" );
		pciBaseAddressShow( pfa, PCI_CFG_BASE_ADDRESS_3, "   Base Address 3" );
		pciBaseAddressShow( pfa, PCI_CFG_BASE_ADDRESS_4, "   Base Address 4" );
		pciBaseAddressShow( pfa, PCI_CFG_BASE_ADDRESS_5, "   Base Address 5" );

		PCIDBG(("   Cardbus CIS Pointer: 0x%08x\n", 
					dPciReadReg(pfa, PCI_CFG_CIS, REG_32) ));
			
		PCIDBG(("   Subsystem Vendor ID: 0x%04x  Subsystem ID: 0x%04x\n", 
				dPciReadReg(pfa, PCI_CFG_SUB_VENDER_ID, REG_16), 
				dPciReadReg(pfa, PCI_CFG_SUB_SYSTEM_ID, REG_16) ));
	
		PCIDBG(("   Expansion ROM Base Address: 0x%08x\n", 
				dPciReadReg(pfa, PCI_CFG_EXPANSION_ROM, REG_32) ));
	
		PCIDBG(("   Capabilities Pointer: 0x%08x\n", 
				dPciReadReg(pfa, PCI_CFG_CAP_PTR, REG_32) ));
		
		PCIDBG(("   Interrupt Line: 0x%02x  Interrupt Pin: 0x%02x  Min_Gnt: 0x%02x  Max_Lat: 0x%02x\n", 
				dPciReadReg(pfa, PCI_CFG_DEV_INT_LINE, REG_8), 
				dPciReadReg(pfa, PCI_CFG_DEV_INT_PIN, REG_8),
				dPciReadReg(pfa, PCI_CFG_MIN_GRANT, REG_8),
				dPciReadReg(pfa, PCI_CFG_MAX_LATENCY, REG_8) ));
	}			
}


/*******************************************************************************
*
* pciListDevs
*
* Scan the PCI buses and display information about each device. 
* 
*
*
* RETURNS: None.
*
*******************************************************************************/
void pciListDevs( int verbose )
{
	int bus;
	int dev;
	int func;
	PCI_PFA	pfa;
	UINT16 vid;
	UINT16 did;
	UINT8 irq;
	UINT8 header;
	UINT32 classCode;
	int count;


	func = 0;
	count = 0;
	
	PCIDBG(("\nNo Bus Dev Func VID    DID    Class    IRQ  Description\n%s", pLineStr));

	for ( bus = 0; bus < PCI_MAX_BUS; bus++ )
	{
		for ( dev = 0; dev < PCI_MAX_DEV; dev++ )
		{
			pfa = PCI_MAKE_PFA( bus, dev, func );
			
			vid = dPciReadReg( pfa, PCI_CFG_VENDOR_ID, REG_16 );
			
			if (vid != 0xffff)
			{
				count++;
				
				did = dPciReadReg( pfa, PCI_CFG_DEVICE_ID, REG_16 );
				irq = dPciReadReg( pfa, PCI_CFG_DEV_INT_LINE, REG_8 );
				classCode = dPciReadReg( pfa, PCI_CFG_REVISION, REG_32 );
				classCode = classCode >> 8;
				
				PCIDBG(("%02d %03d %02d  %02d   0x%04x 0x%04x 0x%06x %-3u  %s\n",
						count, bus, dev, func, vid, did, classCode, irq, pciGetDeviceClassName( classCode )));
						
				header = dPciReadReg( pfa, PCI_CFG_HEADER_TYPE, REG_8 );
				
				if (verbose)
				{
					pciHeaderShow( pfa, classCode, header );
				}
				
				if ( (header & PCI_HEADER_MULTI_FUNC) == PCI_HEADER_MULTI_FUNC )
				{
					/* device is multifunction */	
					for ( func = 1; func < PCI_MAX_FUNC; func++ )
					{
						pfa = PCI_MAKE_PFA( bus, dev, func );
						
						vid = dPciReadReg( pfa, PCI_CFG_VENDOR_ID, REG_16 );
						
						if (vid != 0xffff)
						{
							count++;
							
							did = dPciReadReg( pfa, PCI_CFG_DEVICE_ID, REG_16 );
							irq = dPciReadReg( pfa, PCI_CFG_DEV_INT_LINE, REG_8 );
							classCode = dPciReadReg( pfa, PCI_CFG_REVISION, REG_32 );
							classCode = classCode >> 8;
							
							PCIDBG(("%02d %03d %02d  %02d   0x%04x 0x%04x 0x%06x %-3u  %s\n",
									count, bus, dev, func, vid, did, classCode, irq, pciGetDeviceClassName( classCode )));
									
							if (verbose)
							{
								header = dPciReadReg( pfa, PCI_CFG_HEADER_TYPE, REG_8 );
								pciHeaderShow( pfa, classCode, header );
							}
						}
					}

					func = 0;	
				}
			}
		}
	}
	
	PCIDBG(("\n"));
}
#else

void pciListDevs( void )
{

}

#endif // PCISHOW_DEBUG
