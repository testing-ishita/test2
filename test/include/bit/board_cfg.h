#ifndef __board_cfg_h__
#define __board_cfg_h__
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
/* board_arch.h - board-specific arhitecture descriptions
 */
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/board_cfg.h,v 1.1 2013-09-04 07:35:26 chippisley Exp $
 *
 * $Log: board_cfg.h,v $
 * Revision 1.1  2013-09-04 07:35:26  chippisley
 * Import files into new source repository.
 *
 * Revision 1.26  2012/09/14 15:38:45  jthiru
 * CUTE Support for VX91X - C5043B1110, fixes to existing bugs
 *
 * Revision 1.25  2011/08/02 17:07:29  hchalla
 * Updated macros.
 *
 * Revision 1.24  2010/11/04 17:28:19  hchalla
 * Added LDT PIT board specific info.
 *
 * Revision 1.23  2010/10/25 15:35:33  jthiru
 * Added function prototypes
 *
 * Revision 1.22  2010/09/15 12:18:27  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.21  2010/09/10 09:52:15  cdobson
 * Added brdPostScanInit definition.
 *
 * Revision 1.20  2010/07/26 13:14:10  hmuneer
 * Source Cleanup
 *
 * Revision 1.19  2010/06/23 10:59:21  hmuneer
 * CUTE V1.01
 *
 * Revision 1.18  2010/05/05 15:48:16  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.17  2010/03/31 16:28:35  hmuneer
 * no message
 *
 * Revision 1.16  2010/01/19 12:04:09  hmuneer
 * vx511 v1.0
 *
 * Revision 1.15  2009/09/29 12:27:42  swilson
 * Add Application Flash support and make corrections to an 'extern' declaration.
 *
 * Revision 1.14  2009/06/12 10:58:36  hmuneer
 * no message
 *
 * Revision 1.13  2009/06/10 14:43:59  hmuneer
 * smbus.c added
 *
 * Revision 1.12  2009/06/09 16:05:35  swilson
 * Added new function and defines for getting FPU exception handling capabilities for
 *  board.
 *
 * Revision 1.11  2009/06/08 16:18:59  swilson
 * Add extern for brdGetNumCores.
 *
 * Revision 1.10  2009/06/08 09:54:08  hmuneer
 * TPA40
 *
 * Revision 1.9  2009/05/29 14:09:33  hmuneer
 * no message
 *
 * Revision 1.8  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.7  2009/05/18 09:25:29  hmuneer
 * no message
 *
 * Revision 1.6  2008/09/19 14:56:16  swilson
 * New board-specific hardware support.
 *
 * Revision 1.5  2008/07/24 14:52:24  swilson
 * Prototypes required for board-specific PCI and H/W initialization code.
 *
 * Revision 1.4  2008/07/18 12:24:39  swilson
 * Add functions for retrieving DRAM size.
 *
 * Revision 1.3  2008/05/16 14:36:24  swilson
 * Add test list management and test execution capabilities.
 *
 * Revision 1.2  2008/05/15 15:32:40  swilson
 * Add build and version tracking. Implement & test missing c-library functions.
 *
 * Revision 1.1  2008/05/12 15:04:40  swilson
 * Add IBIT console. Debugging various library functions.
 *
 */
 
/* includes */
#include <stdtypes.h>
#include <bit/bit.h>
#include <cute/idx.h>

/* defines */
#define	FWID_BASIC		0		/* minimum detail */
#define	FWID_FULL		1		/* maximum detail */
#define	FWMODE_BIT		0		/* minimum detail */
#define	FWMODE_CUTE		1		/* maximum detail */

/* typedefs */
typedef struct tagFwVersion
{
	char	chXorV;
	int		iVersion;
	int		iRevision;
	int		iSpin;
	int		iBuild;
} FW_VERSION;

typedef struct tagSccParams
{
	UINT8	bPort;			/* port number */
	char*	achPortName;	/* name used by board for this port */
	UINT16	wBaseAddr;		/* base address for port */
	UINT8	bIrq;			/* IRQ to use for this port */
} SCC_PARAMS;

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

typedef struct
{
	UINT16		memory;
	UINT8       HardwareRev;
	UINT8 		firmwareVersion;
} brd_info;

/* constants */

/* locals */

/* globals */

/* externals */

	/* app_flash.c */
extern AF_PARAMS* brdAppFlashGetParams (void);
extern void brdAppFlashSelectPage (UINT32 dPage);
extern void brdAppFlashEnable (void);

	/* cpu.c */
#define FPU_EXCEPTION_MF	0x00000001		/* CPU/chipset supports MF */
#define FPU_EXCEPTION_FERR	0x00000002		/* CPU/chipset supports FERR */
extern UINT8 brdGetNumCores (void);
extern UINT32 brdGetFpuExceptionMap (void);

	/* board_cfg.c */
extern UINT16 brdGetConsolePort (void);
UINT16 brdGetSokePort (void);
extern UINT16 brdGetDebugPort (void);
extern UINT32 brdGetSccPortMap (void);
extern SCC_PARAMS* brdGetSccParams (UINT8 bPort);
extern UINT64 brdGetMaxDram (void);
extern UINT32 brdGetMaxDramBelow4GB (void);
extern UINT32 brdSelectEthInterface (UINT8 cnt, UINT8 Iface);
extern UINT32 brdSkipEthInterface (UINT32  dDidVid, UINT8 bInstance);
extern void brdFlashLed (void);
extern void brdReset (void);
UINT8 brdCheckUniverseJumper (void);
UINT8 brdCheckUserMthSwitch (void);

	/* hw_init.c */
extern void brdHwInit (void);
extern void brdPostScanInit(void);
extern void brdCuteInfo(brd_info *info);

	/* nvram.c */
extern void brdNvWrite (UINT32 dNvToken, UINT32 dData);
extern UINT32 brdNvRead (UINT32 dNvToken);

	/* pci_cfg.c */
extern PCI_PARAMS* brdPciGetParams (void);
extern void brdPciCfgHbStart ( UINT8 bHostBridge, UINT8 bRootBus, UINT32* pdMemAddr, UINT16* pwIoAddr);
extern void brdPciCfgHbEnd ( UINT8 bHostBridge, UINT8 bSubBus, UINT32* pdMemAddr, UINT16* pwIoAddr);
extern void brdPciRoutePirqToIrq (void);

	/* signon.c */
extern void brdDisplaySignon (int iLevel, int mode);
extern void brdGetVersionInfo (FW_VERSION* psVersionInfo);

	/* test_list.c */
extern TEST_ITEM* brdGetTestDirectory (void);

	/* watchdog.c */
extern void brdWatchdogEnable (void);
extern void brdWatchdogDisable (void);
extern void brdWatchdogPat (void);

	/* postLED.c */
extern void postON();
extern void postOFF();
extern void brdGetCoopTestTable(const COOP_ENTRY**	ppsCoopTestTable);

/* superio.c */
//----------------------------------------------------
typedef struct
{
	char*  Name;
	UINT8  RegID;
	UINT8  Mask;
	UINT32 ERR_CODE; // an error code of 0 indicates end of list
} SIO_DEVICE_REG;

typedef struct
{
	char*			Name;
	UINT8			DevLDN;
	SIO_DEVICE_REG *DeviceRegs; // no reg structure pointer indicates end of list
	UINT32			ERR_CODE;
} SIO_Device;

typedef struct
{
	char*  Name;
	UINT8  RegID;
	UINT8  Mask;
	UINT32 ERR_CODE; // an error code of 0 indicates end of list
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
	UINT16      GlobalFlags;//flags i.e. shall we check the IDX, and SID regs or not
	SIO_REG    *GlobalRegs;
	SIO_Device *SIODevices;
	UINT32      ErrCodeBase;
} SIO_Info;

// SIO GLOABL FLAGS
// ---------------------------------
#define SIO_CHK_IDX	0x0001
#define SIO_CHK_SID	0x0002

SIO_Info* brdGetSuperIO(void);
extern UINT16 brdGetSuperIONum (void);

/* pci.c */
//----------------------------------------------------
typedef struct
{
	const char*		Name;		// register name
	const UINT16	RegAddr;	// register address, a RegAddr of 0x00 indicates end of list
	const UINT8		Mask;		// register test mask	
} pci_Regs;

typedef struct
{
	PCI_PFA		pfa;		// how we do we talk to this node
	UINT8			pBUS;		// where does this device exist
	UINT8			sBUS;		// what's the secondary bus in case of a bridge
} pci_Vars;

typedef struct
{
	const char*		Name;		// Name of the device
	const UINT16	type;		// defined below, 0x00 means end of list
	const UINT16	functions;	// defined below 	
	const UINT16	VID;		// Vendor ID
   	const UINT16	DID;		// Device ID
	const UINT8		Device;		// What device number on the bus 0-31
	const UINT8		Function;	// What function number on a multifunction device 0-7	
	UINT8			Flags;		// Flag indicates, if this instance has been found and tested
	const pci_Regs*	Regs;		// registers on this device to test
	pci_Vars		Vars;		// dynamic parameters of PCI device
	const void*		MDevices;	// Multi funtion devices under this node *pci_Device list [recursive list]
	const void*		BDevices;	// Bus devices under this node *pci_Device list [recursive list]
	PCI_PFA			PXMCPFA;	// first PMC/XMC device
	const UINT32	ERR_CODE;	// base error code for this node
} pci_Device;

typedef struct
{
	UINT8			Flags;
	UINT8			(*vChkPXMC)(void);		// checks the presence of PMC/XMC cards
	UINT8			(*vChkSatellite)(void);	// checks if board is in satellite mode, this function can also used to
											// return board specific options
	UINT8			(*vChkPeripheral)(void);// checks if board is in peripheral mode,  rtn = 0 Peripheral, 1 Sys controller,
											// this function is also used to return board specific options like the presence
											// of ADCR2 in case of vp417
	pci_Device*		Devices0;
	const UINT32	ERR_BASE;
}pci_DB;

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
#define PCI_DEV_PRESENT				0x80
#define PCI_EXPRESS_END				0x100
#define PCI_CARRIER				0x200

//functions
//---------------------------------------------
#define PCI_SINGLE_FUNC			0x01
#define PCI_MULTI_FUNC			0x02
#define PXMC_PMC1				0x10
#define PXMC_PMC2				0x20
#define PXMC_XMC1				0x40
#define PXMC_XMC2				0x80
#define PCI_NOTFOUND			0x00
#define PCI_FOUND				0x01
#define PCI_FAIL				0x02
#define PCI_PASS				0x04

pci_DB* brdGetPCIDB(void);

/* SCR.c */
//----------------------------------------------------
typedef struct 
{
	char*		Name;
	UINT16	Port;  //port 0x00 means end of list
	UINT8		WRMask;
	UINT8		ROMask;
	UINT32	ErrorCode;
} SACReg;

SACReg* brdGetSCR(void);

/* msr.s and xmmx.s */
//----------------------------------------------------
extern void	vReadMsr (UINT32 dIndex, UINT32* dMsb, UINT32* dLsb);
extern void	vWriteMsr (UINT32 dIndex, UINT32 dMsb, UINT32 dLsb);
extern void	mmxTest(UINT32 operation, UINT64 *result, UINT64 op1, UINT64 op2);
extern void	mmxRead(UINT32 reg, UINT64 *data);
extern void	mmxWrite(UINT32 reg, UINT64 *data);
extern void	xmmTest(UINT32 operation, UINT8 *result, UINT8 *op1, UINT8 *op2);
extern void	xmmRead(UINT32 reg,  UINT8 *data );
extern void	xmmWrite(UINT32 reg, UINT8 *data );
extern void	fpuTest(UINT32 operation, float *result, float *op1, float *op2);
extern void	fpuExpTest(UINT32 operation, float *result, float *op1, float *op2);
extern void	fpuSetup(UINT32 operation, UINT32 *op);
extern void	fpuGetFlags( UINT16 *flags);
extern void fpuClearFlags(UINT32 op);

/* forward declarations */
typedef struct
{
	UINT16 bTimerByte0;
	UINT16 bTimerByte1;
	UINT16 bTimerByte2;
	UINT16 bTimerByte3;
	UINT16 bLdtPitControl;
	UINT16 bIrqMode;
}LDTPIT_INFO;

LDTPIT_INFO *ldtPitInfo (void);

//eeprom.c
//----------------------------------------------------
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
} EEPROM_INFO;

EEPROM_INFO* brdEEPROMinfo(void);

//eeprom.c
//----------------------------------------------------
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

SPI_MRAM_INFO* brdMRAMinfo(void);

//smbus.c
//----------------------------------------------------
typedef struct 
{
	UINT8	  Bus;
	UINT8	  Device;
	UINT8	  Function;
	UINT16  GPIOBaseReg;
	UINT32  GPIOBase;
	UINT16  reg1;
	UINT8  mask1;
	UINT16  reg2;
	UINT8  mask2;
} SMBUS_INFO;

SMBUS_INFO* brdSMBUSinfo(void);

//temperature.c
//----------------------------------------------------
typedef struct
{
	UINT8	  Addr;
	UINT8     ManufID;
	UINT8     DevID;
} TEMPERATURE_INFO;

TEMPERATURE_INFO* brdGetTempinfo(void);

//LEDs.c
//----------------------------------------------------
typedef struct 
{
	UINT16  port;
	UINT8	  bit;
} LED_INFO;

LED_INFO* brdLEDinfo(void);

typedef struct
{
	UINT16 EEPROMCntrl;
}SPIEEPROM_INFO;

SPIEEPROM_INFO *spiBrdEEPROMInfo (void);

typedef struct
{
	UINT32 dBaudRate;
	UINT16 wBaudDiv;
	UINT32 dBaudHigh;
	UINT32 dBaudLow;
}SERIALBAUD_INFO;

SERIALBAUD_INFO *serialBrdBaudInfo (void);

void vEnRS485_hd(void);
void vEnRS485_fd(void);
void vDisRS485(void);

#endif
