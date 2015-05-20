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

/*
 * board_service.h
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/board_service.h,v 1.23 2015-04-01 16:58:07 hchalla Exp $
 * $Log: board_service.h,v $
 * Revision 1.23  2015-04-01 16:58:07  hchalla
 * Added new board service #define SERVICE__BRD_GET_CPCI_BRIDGE_IMAGES.
 *
 * Revision 1.22  2015-04-01 16:45:07  hchalla
 * Added support for backplane CPCI tests.
 *
 * Revision 1.21  2015-03-17 16:57:07  mgostling
 * Added SERVICE__BRD_GET_VME_BRIDGELESS_REG
 *
 * Revision 1.20  2015-02-27 13:13:15  mgostling
 * Updated VME_DRV_INFO structure
 *
 * Revision 1.19  2015-02-25 17:57:57  hchalla
 * Added new VME Cute co-op memory tests.
 *
 * Revision 1.18  2015-01-29 10:39:42  mgostling
 * Add new board services.
 *
 * Revision 1.17  2015-01-21 18:01:45  gshirasat
 * Added AMC specific board services
 *
 * Revision 1.16  2014-11-10 09:36:20  mgostling
 * Add new service to verify RTC has reset properly and resolve issue if not.
 *
 * Revision 1.15  2014-10-15 11:19:38  mgostling
 * Update srioDeviceInfo structure definition.
 *
 * Revision 1.14  2014-10-09 09:51:11  chippisley
 * Added SERVICE__BRD_CHECK_SRIO_MASTER definition.
 *
 * Revision 1.13  2014-09-19 09:10:29  mgostling
 * Add support for two TSI721 SRIO devices
 *
 * Revision 1.6.2.1  2014-06-04 15:46:31  hchalla
 * Added support for dual srio chip.

 * Revision 1.12  2014-08-04 08:47:42  mgostling
 * Rationalise service call to obtain memory size from GPIO
 *
 * Revision 1.11  2014-05-12 16:10:15  jthiru
 * Adding support for VX91x and fixes to existing bugs
 *
 * Revision 1.10  2014-03-28 15:17:32  cdobson
 * Added board service for getting PCI device interrupt information.
 *
 * Revision 1.9  2014-03-27 11:57:58  mgostling
 * Added a service to obtain board specific GPIO loopback configurations
 *
 * Revision 1.8  2014-03-20 16:02:18  mgostling
 * Add new service to get switch connections for ADVR3 loopback test
 *
 * Revision 1.7  2014-03-06 16:32:47  cdobson
 * Added new service to get audio configuration.
 *
 * Revision 1.6  2014-01-29 13:18:15  hchalla
 * Added SRIO defines for SRIO 1 chip and SRIO 2 chip.
 *
 * Revision 1.5  2014-01-10 09:55:46  mgostling
 * Added new service to obtain DRAM size.
 *
 * Revision 1.4  2013-11-25 10:47:43  mgostling
 * Added support for TRB1x
 *
 *
 *  Created on: 26 Oct 2010
 *      Author: engineer
 */

#ifndef BOARD_SERVICE_H_
#define BOARD_SERVICE_H_

#include <stdtypes.h>
#include <bit/bit.h>
#include <cute/idx.h>

/*****************************************************************************
 * 																			 *
 * 							 SERVICE INTERFACE								 *
 * 																			 *
 *****************************************************************************/

#define SERVICE_NOT_SUPPORTED		 1
#define SERVICE_REQUEST_ERROR		 2

/* board_service:
 * 									Implements a standard interface to call board
 * 									specific functions, the service interface  does
 * 									not require every board to implement all services.
 * 									A board can implement a board specific service
 * 									without having any impact on other boards. This
 * 									allows for a clean and consistent way of calling
 * 									board specific functionality from rest of the
 * 									BIT/CUTE code.
 *
 * Parameters:
 * 		   ServiceID				Service ID number, all service ID numbers
 * 									are defined in board_service.h, each board
 * 									implements a subset of the defined services
 *
 * 		   *rt						Error code returned by the service itself, can
 *                                  be NULL if not required
 *
 * 		   *ptr						Pointer to a buffer to pass and retrieve data
 * 									from the service, can be NULL if not required
 * Returns:
 * 		   E__OK 					On successful service execution,*rt is
 * 									also E__OK so there is no need to check it
 *
 *         SERVICE_NOT_SUPPORTED    If there is no service corresponding to
 *         							supplied ServiceID
 *
 *         SERVICE_REQUEST_ERROR    If the service request returned an error
 *         						    condition, *rt contains the actual error
 *         						    code returned by the service
 */
extern UINT32 board_service(UINT32 ServiceID, UINT32* rt, void* ptr);




/*****************************************************************************
 * 																			 *
 * 							   SERVICE IDs									 *
 * 																			 *
 *>>>>>>>>>      please note this list must be checked to       <<<<<<<<<<<<<*
 *>>>>>>>>> verify that all entries are unique after each merge <<<<<<<<<<<<<*
 * 																			 *
 *****************************************************************************/

#define SERVICE__BRD_GET_CONSOLE_PORT			1
#define SERVICE__BRD_GET_DEBUG_PORT				2
#define SERVICE__BRD_GET_SCC_PORT_MAP		 	3
#define SERVICE__BRD_GET_SCC_PARAMS				4
#define SERVICE__BRD_GET_MAX_DRAM_BELOW_4GB		5
#define SERVICE__BRD_FLASH_LED					6
#define SERVICE__BRD_RESET						7
#define SERVICE__BRD_USR_MTH_SWITCH				8
#define SERVICE__BRD_GET_SOAK_PORT				9
#define SERVICE__BRD_SELECT_ETH_IF				10
#define SERVICE__BRD_SKIP_ETH_IF				11
#define SERVICE__BRD_GET_NUM_CORES				12
#define SERVICE__BRD_GET_FPU_EXCEPTION_MAP		13
#define SERVICE__BRD_HARDWARE_INIT				14
#define SERVICE__BRD_POST_SCAN_INIT				15
#define SERVICE__BRD_CUTE_INFO					16
#define SERVICE__BRD_NV_READ					17
#define SERVICE__BRD_NV_WRITE					18
#define SERVICE__BRD_GET_PCI_PARAMS				19
#define SERVICE__BRD_PCI_CFG_HUB_START			20
#define SERVICE__BRD_PCI_CFG_HUB_END			21
#define SERVICE__BRD_PCI_ROUTE_PIRQ_IRQ			22
#define SERVICE__BRD_DISPLAY_SIGNON				23
#define SERVICE__BRD_GET_VERSION_INFO			24
#define SERVICE__BRD_GET_TEST_DIRECTORY			25
#define SERVICE__BRD_WATCHDOG_ENABLE			26
#define SERVICE__BRD_WATCHDOG_DISABLE			27
#define SERVICE__BRD_WATCHDOG_PAT				28
#define SERVICE__BRD_POST_ON					29
#define SERVICE__BRD_POST_OFF					30
#define SERVICE__BRD_GET_APPFLASH_PARAMS		31
#define SERVICE__BRD_SELECT_APPFLASH_PAGE		32
#define SERVICE__BRD_APPFLASH_ENABLE			33
#define SERVICE__BRD_GET_SIO					34
#define SERVICE__BRD_GET_SIO_NUM				35
#define SERVICE__BRD_GET_PCI_DB					36
#define SERVICE__BRD_GET_SCR_INFO				37
#define SERVICE__BRD_GET_LDPIT_INFO				38
#define SERVICE__BRD_GET_EEPROM_INFO			39
#define SERVICE__BRD_GET_SMBUS_INFO				40
#define SERVICE__BRD_GET_TEMPERATURE_INFO		41
#define SERVICE__BRD_GET_LED_INFO				42
#define SERVICE__BRD_GET_SPI_EEPROM_INFO		43
#define SERVICE__BRD_GET_SERIAL_BAUD_INFO		44
#define SERVICE__BRD_ENABLE_RS485_HD			45
#define SERVICE__BRD_ENABLE_RS485_FD			46
#define SERVICE__BRD_DISABLE_RS485				47
#define SERVICE__BRD_INIT_ICR					48
#define SERVICE__BRD_GET_COOP_TEST_TBL			49
#define SERVICE__BRD_UNIVERSE_JUMPER			50
#define SERVICE__BRD_GET_FLASH_DRV_PARAMS		51
#define SERVICE__BRD_GET_TRXMC_PCI_DB			52
#define SERVICE__BRD_CHK_PCIE_SW_CONFIG			53
#define SERVICE__BRD_GET_TRXMC_PARENT_BUS		54
#define SERVICE__BRD_GET_WATCHDOG_INFO			55
#define SERVICE__BRD_WATCHDOG_ENABLE_NMI		56
#define SERVICE__BRD_WATCHDOG_ENABLE_RESET		57
#define SERVICE__BRD_WATCHDOG_CHECK_SWITCH		58
#define SERVICE__BRD_GET_GPIO_INFO              59
#define SERVICE__BRD_CONFIGURE_VXS_NT_PORTS		60
#define SERVICE__BRD_GET_VXS_TRANS_PORTS		61
#define SERVICE__BRD_GET_RS485_PORT_MAP			62
#define SERVICE__BRD_CONFIG_CPCI_BACKPLANE		63
#define SERVICE__BRD_DECONFIG_CPCI_BACKPLANE	64
#define SERVICE__BRD_CHECK_CPCI_IS_SYSCON		65
#define SERVICE__BRD_GET_CPCI_INSTANCE			66
#define SERVICE__BRD_GET_BIOS_INFO				67
#define SERVICE__BRD_GET_EXTENDED_INFO			68
#define SERVICE__BRD_GET_TEMP_GRADE_STRING  	69
#define SERVICE__BRD_GET_FORCE_DEBUG_ON			70
#define SERVICE__BRD_GET_FORCE_DEBUG_OFF		71
#define SERVICE__BRD_GET_THRESHOLD_INFO			72
#define SERVICE__BRD_GET_STH_ORIDE_TEST_LIST	73
#define SERVICE__BRD_PROCESS_STH_ORIDE_LIST		74
#define SERVICE__BRD_GET_BIOS_VERSION			75
#define SERVICE__BRD_GET_BOARD_NAME				76
#define SERVICE__BRD_SET_INTEL_SPEED_STEP		77
#define SERVICE__BRD_GET_IPMI_NMI				78
#define SERVICE__BRD_CLEAR_IPMI_NMI				79
#define SERVICE__BRD_GET_SPI_MRAM_INFO			80
#define SERVICE__BRD_CONFIG_VPX_BACKPLANE		81
#define SERVICE__BRD_GET_VPX_INSTANCE			82
#define SERVICE__BRD_SKIP_ETH_BUS				83
#define SERVICE__BRD_LDT_CLEAR_INTR				84
#define SERVICE__BRD_GET_PCI_IOMASK				85
#define SERVICE__BRD_SERIAL_HARDWARE_INIT		86
#define SERVICE__BRD_GET_DS3905_INFO			87
#define SERVICE__BRD_UNLOCK_CCT_IOREGS			88
#define SERVICE__BRD_LOCK_CCT_IOREGS			89
#define SERVICE__BRD_GET_SLOT_ID				90
#define SERVICE__BRD_PCI_CFG_SKIP_IO_ALLOCATION 91
#define SERVICE__BRD_GET_DRAM_SIZE_FROM_GPIO	92
#define	SERVICE__BRD_SKIP_ETH_TEST				93
#define SERVICE__BRD_RESTORE_CONFIG_CHANGES		94
#define SERVICE__BRD_ENABLE_SMIC_LPC_IOWINDOW	96
#define SERVICE__BRD_STORE_CONFIG_CHANGES		95
#define SERVICE__BRD_CONFIG_SRIO_DEVICE			97
#define SERVICE__BRD_DECONFIG_SRIO_DEVICE		98
#define SERVICE__BRD_SET_LDTPIT_INTR			99
#define SERVICE__BRD_DECODE_TPM_LPC_IOWINDOW	100
#define SERVICE__BRD_ENABLE_HDA					101
#define SERVICE__BRD_GET_SRIO_INFO				102
#define SERVICE__BRD_DECONFIG_SRIO_DEVICE_2		103
#define SERVICE__BRD_CONFIG_SRIO_DEVICE_2		104
#define SERVICE__BRD_GET_HDA_LIST				105
#define SERVICE__BRD_GET_RTM_PCIE_SWITCH_INFO	106
#define SERVICE__BRD_GET_ENXMC_GPIO_INFO		107
#define SERVICE__BRD_GET_PCI_INT_INFO			108
#define SERVICE__BRD_GET_AHCI_PORT_INFO			109
#define SERVICE__BRD_SELECT_SPI_FWH				110
#define SERVICE__BRD_CHECK_DUAL_SRIO			111
#define SERVICE__BRD_CHECK_SRIO_MASTER			112
#define SERVICE__BRD_INIT_RTC					113
#define SERVICE__BRD_URTM_GPIO_TEST				114
#define SERVICE__BRD_GET_AMC_PCIE_LINKFINFO		115
#define SERVICE__BRD_ENABLE_WD_LPC_IOWINDOW		116
#define SERVICE__BRD_CHECK_ETH_PRESENCE			117
#define SERVICE__BRD_IS_TPM_FITTED				118
#define SERVICE__BRD_VME_BRIDGELESS_MODE		119
#define SERVICE__BRD_VME_BRIDGELESS_SLOTID		120
#define SERVICE__BRD_GET_SERIAL_NUMBER			121
#define SERVICE__BRD_GET_VME_BRIDGE_IMAGES		122
#define SERVICE__BRD_GET_TEMP_THRESHOLDS		123
#define SERVICE__BRD_GET_VME_BRIDGELESS_REG		124
#define SERVICE__BRD_GET_CPCI_BRIDGE_IMAGES		125
#define SERVICE__BRD_GET_CPCI_SLAVE_IMAGES 126
#define SERVICE__BRD_GET_CPCI_UPSTREAM_IMAGE 127
#define SERVICE__UNKNOWN						NULL




/*****************************************************************************
 * 																			 *
 * 	Structures and other information associated with each service, if any 	 *
 * 																			 *
 *****************************************************************************/


/*	For :- SERVICE__BRD_GET_SCC_PARAMS
 * ***************************************************************************
 * */
typedef struct tagSccParams
{
	UINT8	bPort;			/* port number */
	char*	achPortName;	/* name used by board for this port */
	UINT16	wBaseAddr;		/* base address for port */
	UINT8	bIrq;			/* IRQ to use for this port */

} SCC_PARAMS;



/*	For :- SERVICE__BRD_SELECT_ETH_IF
 * ***************************************************************************
 * */
#define	E__TEST_WRONG_CONTROLLER		E__BIT + 0x01
#define	E__TEST_WRONG_INTERFACE			E__BIT + 0x02

typedef struct tagSelectEthIf
{
	UINT8	cnt;			/* controller */
	UINT8	Iface;			/* interface */

} SelEthIf_PARAMS;


#define ETH_MAC_DISP_MODE        1
#define ETH_MAC_NO_DISP_MODE 2
/*	For :- SERVICE__BRD_SKIP_ETH_IF
 * ***************************************************************************
 * */
typedef struct tagSkipEthIf
{
	UINT32  dDidVid;
	UINT8 	bInstance;
	UINT8 	bMode;
	PCI_PFA	pfa;
} SkipEthIf_PARAMS;


/*	For :- SERVICE__BRD_SKIP_ETH_TEST
 * ***************************************************************************
 * */
typedef struct tagSkipEthTest
{
	UINT32  dDidVid;
	UINT8 	bInstance;
	UINT8 	bTestid;

} SkipEthTest_PARAMS;


/* For :-  SERVICE__BRD_GET_FPU_EXCEPTION_MAP
 * ***************************************************************************
 * */
#define FPU_EXCEPTION_MF	0x00000001		/* CPU/chipset supports MF */
#define FPU_EXCEPTION_FERR	0x00000002		/* CPU/chipset supports FERR */

/* For :-  SERVICE__BRD_CUTE_INFO */
typedef struct tagbrd_info
{
	UINT16		memory;
	UINT8       HardwareRev;
	UINT8 		firmwareVersion;
} brd_info;



/* For :-  SERVICE__BRD_NV_READ and SERVICE__BRD_NV_WRITE
 * ***************************************************************************
 * */
#define NV__TEST_NUMBER		0
#define NV__ERROR_CODE		1
#define NV__BIOS_BOOT		2
#define NV__RESULT_VALID	3

typedef struct tagNV_RW_Data
{
	UINT32	dNvToken;
	UINT32  dData;

} NV_RW_Data;


/* For :-  SERVICE__BRD_GET_PCI_PARAMS
 * ***************************************************************************
 * */
typedef struct tagPciParams
{
	UINT32		dHbMemAlignment;	/* resource alignments for host bridge */
	UINT16		wHbIoAlignment;

	UINT32		dPciMemBase;		/* start of PCI allocations (high address) */
	UINT16		wPciIoBase;

	UINT32		dPciMemLimit;		/* end of PCI allocations (high address) */
	UINT16		wPciIoLimit;

	UINT8		bHostBridges;		/* number of PCI host bridges in design */

	UINT8		bCsLatency;			/* chipset defined system-wide parameters */
	UINT8		bCsCacheLine;

} PCI_PARAMS;


/* For :-  SERVICE__BRD_PCI_CFG_HUB_START
 * ***************************************************************************
 * */
typedef struct tagPciCFGHubStart
{
	UINT8	bHostBridge;
	UINT8	bRootBus;
	UINT32*	pdMemAddr;
	UINT16*	pwIoAddr;

}PCI_HUB_START_PARAMS;



/* For :-  SERVICE__BRD_PCI_CFG_HUB_END
 * ***************************************************************************
 * */
typedef struct tagPciCFGHubEnd
{
	UINT8	bHostBridge;
	UINT8	bSubBus;
	UINT32*	pdMemAddr;
	UINT16*	pwIoAddr;

}PCI_HUB_END_PARAMS;

/* For :-  SERVICE__BRD_PCI_CFG_SKIP_IO_ALLOCATION
 * ***************************************************************************
 * */
typedef struct tagPcicfgskipIOParams
{
	PCI_PFA	pfa;
	UINT8 skipStatus;

} PCI_CFG_SKIP_IO_ALLOC_PARAMS;


/* For :-  SERVICE__BRD_DISPLAY_SIGNON
 * ***************************************************************************
 * */
typedef struct tagDisplaySignon
{
	int iLevel;
	int mode;

} DISPLAY_SIGNON_INFO;




/* For :-  SERVICE__BRD_GET_VERSION_INFO
 * ***************************************************************************
 * */
typedef struct tagFwVersion
{
	char	chXorV;
	int		iVersion;
	int		iRevision;
	int		iSpin;
	int		iBuild;

} FW_VERSION;



/* For :-  SERVICE__BRD_GET_TEST_DIRECTORY
 * ***************************************************************************
 * */
typedef struct tagTestItem
{
	UINT16	wTestNum;				/* assigned test number */
	UINT16	wDuration;				/* pass/fail duration in mS */
	UINT32	dFlags;					/* test permissions flags */
	char*	achTestName;			/* pointer to test name string */
	UINT32	(*pTestFn)(UINT32*);	/* pointer to test interface function */

} TEST_ITEM;




/* For :-  SERVICE__BRD_GET_APPFLASH_PARAMS
 * ***************************************************************************
 * */
typedef struct tagAFParams
{
	UINT32	dManuf;
	UINT32	dDevice;
	UINT32	dSectorSize;
	UINT32	dNumSectors;
	UINT32	dPageBase;
	UINT32	dPageSize;
	char*	achName;

} AF_PARAMS;




/* For :-  SERVICE__BRD_GET_SIO
 * ***************************************************************************
 * */
#define SIO_CHK_IDX	0x0001
#define SIO_CHK_SID	0x0002

typedef struct
{
	char*  Name;
	UINT8  RegID;
	UINT8  Mask;
	UINT32 ERR_CODE; // an error code of 0 indicates
					 // end of list
} SIO_DEVICE_REG;

typedef struct
{
	char*			Name;
	UINT8			DevLDN;
	SIO_DEVICE_REG *DeviceRegs; // no reg structure pointer
								// indicates end of list
	UINT32			ERR_CODE;
} SIO_Device;


typedef struct
{
	char*  Name;
	UINT8  RegID;
	UINT8  Mask;
	UINT32 ERR_CODE; // an error code of 0
					 // indicates end of list
} SIO_REG;


typedef struct
{
	UINT8       Num;
	UINT16      IdxReg;
	UINT16      DataReg;
	UINT16      LDNReg;
	UINT16      SIDReg;
	UINT8       ChipID;
	void        (*vSIOEnable) (void);
	void        (*vSIODisbale)(void);
	UINT32      (*vChkRtm)    (void);
	UINT16      GlobalFlags;// flags i.e. shall we check
							// the IDX, and SID regs or not
	SIO_REG     *GlobalRegs;
	SIO_Device  *SIODevices;
	UINT32      ErrCodeBase;
} SIO_Info;



/* For :-  SERVICE__BRD_GET_PCI_DB
 * ***************************************************************************
 * */
// PCI GLOBAL FLAGS
// ---------------------------------
#define PCI_ONBOARDSCAN			0x00
#define PCI_PXMCSCAN			0x01
#define PCI_PXMCdone			0x02

//type
//--------------------------------------------
#define PCI_BRIDGE				0x01
#define PCI_DEVICE				0x02
#define PCI_CBUS				0x04
#define PXMC_PARENT				0x08
#define CPCI_PARENT				0x10
#define PCI_OPTIONAL			0x20
#define PCI_EXPRESS				0x40
#define PCI_DEV_PRESENT			0x80
#define PCI_EXPRESS_END			0x100
#define PCI_CARRIER				0x200

//functions
//---------------------------------------------
#define PCI_SINGLE_FUNC			0x01
#define PCI_MULTI_FUNC			0x02
#define PXMC_PMC1				0x10
#define PXMC_PMC2				0x20
#define PXMC_XMC1				0x80
#define PXMC_XMC2				0x40
#define PXMC_OPTIONAL			0x100


//pci_Options TYPES
//---------------------------------------------
#define OPTION_MAX_PCIE_WIDTH	0x01



#define PCI_NOTFOUND			0x00
#define PCI_FOUND				0x01
#define PCI_FAIL				0x02
#define PCI_PASS				0x04


typedef struct
{
	const char*		Name;		// register name
	const UINT16	RegAddr;	// register address, a RegAddr of
							    // 0x00 indicates end of list
	const UINT8		Mask;		// register test mask

} pci_Regs;


typedef struct
{
	PCI_PFA			pfa;		// how we do we talk to this node
	UINT8			pBUS;		// where does this device exist
	UINT8			sBUS;		// what's the secondary bus in
							    // case of a bridge
} pci_Vars;


typedef struct
{
	UINT32			type;		// Option type
	UINT32			flags;		// flags if any
	UINT32			data;		// option data

} pci_Options;

typedef struct
{
	const char*		Name;		// Name of the device
	const UINT16	type;		// defined above, 0x00 means end
								// of list
	const UINT16	functions;	// defined above
	const UINT16	VID;		// Vendor ID
   	const UINT16	DID;		// Device ID
	const UINT8		Device;		// What device number on the bus
								// 0-31
	const UINT8		Function;	// What function number on a multi-function
								// device 0-7
	UINT8			Flags;		// Flag indicates, if this instance has been
								// found and tested
	const pci_Regs*	Regs;		// registers on this device to test
	pci_Vars		Vars;		// dynamic parameters of PCI device
	const void*		MDevices;	// Multi function devices under this node
								// *pci_Device list [recursive list]
	const void*		BDevices;	// Bus devices under this node *pci_Device
								// list [recursive list]
	PCI_PFA			PXMCPFA;	// first PMC/XMC device
	const UINT32	ERR_CODE;	// base error code for this node
	pci_Options*	options;    // optional data associated with each device

} pci_Device;


typedef struct
{
	UINT8			Flags;
	UINT8			(*vChkPXMC)(void);		// checks the presence of PMC/XMC
											// cards

	UINT8			(*vChkSatellite)(void);	// checks if board is in satellite mode,
											// this function can also used to
											// return board specific options

	UINT8			(*vChkPeripheral)(void);// checks if board is in peripheral mode,
											// rtn = 0 Peripheral, 1 Sys controller,
											// this function is also used to return
											// board specific options like the presence
											// of ADCR2 in case of vp417
	pci_Device*		Devices0;
	const UINT32	ERR_BASE;

}pci_DB;



/* For :-  SERVICE__BRD_GET_SCR_INFO
 * ***************************************************************************
 * */
typedef struct
{
	char*		Name;
	UINT16		Port;  //port 0x00 means end of list
	UINT8		WRMask;
	UINT8		ROMask;
	UINT32		ErrorCode;

} SACReg;



/* For :-  SERVICE__BRD_GET_LDPIT_INFO
 * ***************************************************************************
 * */
enum eIrqMode
{
	POLL=0,
	IRQ
};

typedef struct
{
	UINT16 bTimerByte0;
	UINT16 bTimerByte1;
	UINT16 bTimerByte2;
	UINT16 bTimerByte3;
	UINT16 bLdtPitControl;
	UINT16 bIrqMode;

}LDTPIT_INFO;



/* For :-  SERVICE__BRD_GET_EEPROM_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT8	Bus;
	UINT8	Device;
	UINT8	Function;
	UINT16	GPIOBaseReg;
	UINT32	GPIOBase;

	UINT16  GP_IO_SEL;
	UINT32  GP_IO_SEL_BIT;

	UINT16  GP_LVL;
	UINT32  GP_LVL_CBIT;
	UINT32  GP_LVL_DBIT;

	UINT8	ADDR;
	UINT8	EEPROMSizeKbit;

} EEPROM_INFO;

/* For :-  SERVICE__BRD_GET_MRAM_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT8	Bus;
	UINT8	Device;
	UINT8	Function;
	UINT8   spiCs;
	UINT8   spiClk;
	UINT8   spiMiso;
	UINT8   spiMosi;
	UINT8	ADDR;
	UINT16	GPIOBaseReg;
	UINT16  wSpiInterface;
	UINT16  wSpiProtocolType;
	UINT32	GPIOBase;
	UINT8   bRegWidth;
} SPI_MRAM_INFO;



/* For :-  SERVICE__BRD_GET_SMBUS_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT8	  Bus;
	UINT8	  Device;
	UINT8	  Function;
	UINT16    GPIOBaseReg;
	UINT32    GPIOBase;
	UINT16    reg1;
	UINT8     mask1;
	UINT16    reg2;
	UINT8     mask2;

} SMBUS_INFO;


typedef struct
{
	UINT8	  DS3905_Reg1;
	UINT8	  DS3905_Reg2;
	UINT8	  DS3905_Reg3;
} DS3905_INFO;

/* For :-  SERVICE__BRD_GET_TEMPERATURE_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT8	  Addr;
	UINT8     ManufID;
	UINT8     DevID;

} TEMPERATURE_INFO;

/* typedefs */
typedef struct {
	UINT8	bCpuTempThres;			/* Cpu Temp Threshold	*/
	UINT8	bBoardTemp1Thres;			/* Board Temp Threshold	*/
	UINT8	bBoardTemp2Thres;			/*  		Board Temp Threshold	*/
} TEMTHRESHOLD;



/* For :-  SERVICE__BRD_GET_LED_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT16  port;
	UINT8	  bit;

} LED_INFO;


/* For :-  SERVICE__BRD_GET_GPIO_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT16  wGPIOReg;
	UINT16	wNumGPIOPins;
    UINT32  dTestType;
} GPIO_INFO;



/* For :-  SERVICE__BRD_GET_SPI_EEPROM_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT16 EEPROMCntrl;

}SPIEEPROM_INFO;




/* For :-  SERVICE__BRD_GET_SERIAL_BAUD_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT32 dBaudRate;
	UINT16 wBaudDiv;
	UINT32 dBaudHigh;
	UINT32 dBaudLow;

}SERIALBAUD_INFO;



/* For :-  SERVICE__BRD_GET_COOP_TEST_TBL
 * ***************************************************************************
 * */
typedef struct tagCoopEntry
{
	UINT8	bTestId;
	UINT16	wType;
	UINT8	bAux;

} COOP_ENTRY;



/* For :-  SERVICE__BRD_GET_FLASH_DRV_PARAMS
 * ***************************************************************************
 * */
typedef struct tagFlashDrvParams
{
	UINT8	bInstance;
	UINT8	bport;

} FLASH_DRV_PARAMS;

/* For :-  SERVICE__BRD_GET_VPX_INSTANCE
 * ***************************************************************************
 * */
typedef struct tagVXSInstanceParams
{
	UINT8	bInstance[2];
} VPX_INSTANCE_PARAMS;

/* For :-  SERVICE__BRD_CHK_PCIE_SW_CONFIG
 * ***************************************************************************
 * */
typedef struct tag_PCIE_SW_CONFIG_INFO
{
	UINT8	tye;  // 0 = syscon, 1 = peripheral
	UINT8	count;// number of bridges

} PCIE_SW_CONFIG_INFO;


enum eWdogType
{
	SIO=0,
	CPLD_FPGA
};

/* For :-  SERVICE__BRD_GET_WDOG_INFO
 * ***************************************************************************
 * */
typedef struct
{
	UINT8	  bIntPin;
	UINT16    wWdogIoPort;
	UINT8     bWdogEnBit;
	UINT8     bWdogType;
} WATCHDOG_INFO;




/* For :-  SERVICE__BRD_GET_VXS_TRANS_PORTS
 * ***************************************************************************
 * */
typedef struct
{
	pci_Device*	  portA;
	pci_Device*   portB;
} VXS_T_PORTS;



/* For :-  SERVICE__BRD_GET_TEMP_GRADE_STRING
 * ***************************************************************************
 * */
typedef struct temp_string
{
	UINT32  size;
	UINT8   *buffer;
} Temp_String_Info;

#define TeSt_NO_EXTENDED_INFO		0x00000001
#define TeSt_BUFFER_TOO_SMALL		0x00000002
#define TeSt_NO_TEMP_STRING			0x00000003

/* For :-  SERVICE__BRD_GET_THRESHOLD_INFO
 * ***************************************************************************
 * */
typedef struct SensorThresholds
{
	UINT32 sensor;
	int    LCr;
	int    UCr;
} Sensor_Threshold;

#define SeTh_NO_EXTENDED_INFO		0x00000001
#define SeTh_BUFFER_TOO_SMALL		0x00000002
#define SeTh_NO_THRESHOLD_INFO		0x00000003
#define SeTh_WRONG_SENSOR_NUMBER	0x00000004



/* For :-  SERVICE__BRD_GET_STH_ORIDE_TEST_LIST
 * ***************************************************************************
 * */
typedef struct sthoverridelist
{
	UINT32  size;
	UINT8   *buffer;
} STH_Override_List;

#define StOv_NO_EXTENDED_INFO		0x00000001
#define StOv_BUFFER_TOO_SMALL		0x00000002
#define StOv_NO_OR_TEST_LIST_INFO	0x00000003


/* For :-  SERVICE__BRD_GET_RTM_PCIE_SWITCH_INFO
 * ***************************************************************************
 * */
typedef struct rtmPcieSwitchPortInfo
{
	UINT8	rtmPort;		/* RTM port number*/
	UINT8	hostInstance;	/* link host instance (upstream of NT Link port of RTM switch) */
	UINT8	linkPort;		/* link port number*/
	UINT16	hostVid;		/* link host port vendor Id */
	UINT16	hostDid;		/* link host port Device Id */
} RTM_PCIE_SWITCH_PORT_INFO;

typedef struct rtmPcieSwitchInfo
{
	UINT8	rtmSwitchInstance;
	UINT8	totalPorts;
	UINT8	rtmPorts;
	UINT8	rtmHostSwitchInstance;	/* instance of host switch that RTM switch is is connected to (upstream port) */ 
	UINT8	rtmHostPort;			/* port number of host switch that RTM upstream port is connected to */
	UINT16	rtmHostVid;				/* RTM host switch vendor Id */
	UINT16	rtmHostDid;				/* RTM host port Device Id */
	RTM_PCIE_SWITCH_PORT_INFO	*rtmPortInfo;
} RTM_PCIE_SWITCH_INFO;

/* For :- SERVICE__BRD_GET_PCI_INT_INFO
 * ***************************************************************************
 * */
typedef struct pciInterruptInfo
{
	PCI_PFA Pfa;            /* PFA of the device to get interrupt information for. */
	UINT8   IrqNo;          /* Returned interrupt number */
	UINT8   ApicNo;         /* Returned APIC number      */
} PCI_INTERRUPT_INFO;

/* For :- SERVICE__BRD_GET_SRIO_INFO
 * ***************************************************************************
 * */
typedef const struct srioDeviceList
{
	UINT8	srioAddr;					/* address of SRIO device on the SMBUS */
	UINT8	eepromAddr;					/* address of SRIO device's eeprom on the SMBUS */
} SRIO_DEVICE_SMBUS_ADDRESS;

typedef struct srioDeviceInfo
{
	UINT8						numDev;				/* number of devices */
	UINT8						devFitted;			/* bitmap for fitted devices */
	SRIO_DEVICE_SMBUS_ADDRESS *	srioDeviceList;		/* address of SRIO devices structure */
} SRIO_DEVICE_SMBUS_INFO;

/* For :- SERVICE__BRD_GET_AMC_PCIE_LINKINFO
 * ***************************************************************************
 * */
typedef struct amcPcieLinkInfo
{
	PCI_PFA Pfa;            /* PFA of the PCIe downstream port whose link needs to be checked. */
	UINT8   gen;            /* Intended link speed */
	UINT8   linkWidth;      /* Intended link width */
} AMC_PCIE_LINK_INFO;

/* For :- SERVICE__BRD_INIT_VME_BRIDGE and SERVICE__BRD_GET_VME_BRIDGE_IMAGES
 * ***************************************************************************
 * */
typedef struct vme_drv_info_t
{
	UINT32 deviceId;				// PCI vendor & device ID
	UINT32 mapRegSpace;				// Bridge register address space
	UINT32 *outboundAddr1;			// 16MB outbound image at 0x55000000
	UINT32 *outboundAddr2;			// 16MB outbound image at 0xAA000000
	UINT32 *outboundAddr3;			// 16MB outbound image at 0x55000000 for DMA
	UINT32 *outboundAddr4;			// 16MB outbound image at 0xAA000000 for DMA
	UINT8  slot;					// zero based slot number for this board
} VME_DRV_INFO;


/* For :- SERVICE__BRD_GET_CPCI_BRIDGE_IMAGES
 * ***************************************************************************
 * */
#define MAX_SLAVES			32
#define MAX_SLOTS			32
#define MAX_BOARDS     (MAX_SLAVES/2)

typedef struct cpci_drv_info_t
{
	UINT32 deviceId;				// PCI vendor & device ID
	UINT32 downStreamAddress[MAX_BOARDS];				// Bridge register address space
	UINT32 upStreamAddress;
	UINT8 slaveBoardPresent[MAX_BOARDS];
	UINT8 slaveBoardSlotId[MAX_BOARDS];
	UINT8  Currnetslot;					// zero based slot number for this board
	PCI_PFA	pfa;
} CPCI_DRV_INFO;


#endif /* BOARD_SERVICE_H_ */

