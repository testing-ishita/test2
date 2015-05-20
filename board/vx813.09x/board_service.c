/*
 * board_service.c
 *
 *  Created on: 26 Oct 2010
 *      Author: engineer
 */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/board_service.h>
#include "services.h"

#define PF_CAST		(UINT32(*)())	/* function pointer cast */


const SERVICE_TYPE	serviceCommands[] =
{
	{SERVICE__BRD_GET_CONSOLE_PORT, 		PF_CAST brdGetConsolePort		},
	{SERVICE__BRD_GET_DEBUG_PORT, 		    PF_CAST brdGetDebugPort		    },
	{SERVICE__BRD_GET_SOAK_PORT,	    	PF_CAST brdGetSokePort		    },
	{SERVICE__BRD_GET_SCC_PORT_MAP, 		PF_CAST brdGetSccPortMap	    },
	{SERVICE__BRD_GET_SCC_PARAMS, 			PF_CAST brdGetSccParams   	    },
	{SERVICE__BRD_GET_MAX_DRAM_BELOW_4GB,	PF_CAST brdGetMaxDramBelow4GB   },
	{SERVICE__BRD_FLASH_LED,	    		PF_CAST brdFlashLed			    },
	{SERVICE__BRD_RESET,	    			PF_CAST brdReset			    },
	{SERVICE__BRD_USR_MTH_SWITCH,	    	PF_CAST brdCheckUserMthSwitch   },
	{SERVICE__BRD_SELECT_ETH_IF,	    	PF_CAST brdSelectEthInterface   },
	{SERVICE__BRD_GET_NUM_CORES,	    	PF_CAST brdGetNumCores		    },
	{SERVICE__BRD_GET_FPU_EXCEPTION_MAP,	PF_CAST brdGetFpuExceptionMap   },
	{SERVICE__BRD_HARDWARE_INIT,	    	PF_CAST brdHwInit		    	},
	{SERVICE__BRD_POST_SCAN_INIT,			PF_CAST brdPostScanInit   		},
	{SERVICE__BRD_CUTE_INFO,				PF_CAST brdCuteInfo		   		},
	{SERVICE__BRD_NV_WRITE,					PF_CAST brdNvWrite   			},
	{SERVICE__BRD_NV_READ,					PF_CAST brdNvRead		   		},
	{SERVICE__BRD_GET_PCI_PARAMS,			PF_CAST brdPciGetParams	   		},
	{SERVICE__BRD_PCI_CFG_HUB_START,		PF_CAST brdPciCfgHbStart   		},
	{SERVICE__BRD_PCI_CFG_HUB_END,			PF_CAST brdPciCfgHbEnd		   	},
	{SERVICE__BRD_PCI_ROUTE_PIRQ_IRQ,		PF_CAST brdPciRoutePirqToIrq	},
	{SERVICE__BRD_DISPLAY_SIGNON,			PF_CAST brdDisplaySignon	   	},
	{SERVICE__BRD_GET_VERSION_INFO,			PF_CAST brdGetVersionInfo		},
	{SERVICE__BRD_GET_TEST_DIRECTORY,		PF_CAST brdGetTestDirectory		},
	{SERVICE__BRD_POST_ON,					PF_CAST brdPostON				},
	{SERVICE__BRD_POST_OFF,					PF_CAST brdPostOFF				},
	{SERVICE__BRD_GET_APPFLASH_PARAMS,		PF_CAST brdAppFlashGetParams	},
	{SERVICE__BRD_SELECT_APPFLASH_PAGE,		PF_CAST brdAppFlashSelectPage	},
	{SERVICE__BRD_APPFLASH_ENABLE,			PF_CAST brdAppFlashEnable		},
	{SERVICE__BRD_GET_SIO,					PF_CAST brdGetSuperIO			},
	{SERVICE__BRD_GET_SIO_NUM,				PF_CAST brdGetSuperIONum		},
	{SERVICE__BRD_GET_PCI_DB,				PF_CAST brdGetPCIDB				},
	{SERVICE__BRD_GET_SCR_INFO,				PF_CAST brdGetSCR				},
	{SERVICE__BRD_GET_LDPIT_INFO,			PF_CAST brdLdtPitInfo			},
	{SERVICE__BRD_GET_EEPROM_INFO,			PF_CAST brdEEPROMinfo			},
	{SERVICE__BRD_GET_SMBUS_INFO,			PF_CAST brdSMBUSinfo			},
	{SERVICE__BRD_GET_TEMPERATURE_INFO,		PF_CAST brdGetTempinfo			},
	{SERVICE__BRD_GET_WATCHDOG_INFO,		PF_CAST brdGetWdogInfo			},
	{SERVICE__BRD_GET_LED_INFO,				PF_CAST brdLEDinfo				},
	{SERVICE__BRD_GET_SPI_EEPROM_INFO,		PF_CAST brdSpiEEPROMInfo		},
	{SERVICE__BRD_GET_SERIAL_BAUD_INFO,		PF_CAST brdSerialBaudInfo		},
#ifdef CUTE
	{SERVICE__BRD_INIT_ICR,					PF_CAST brdInitIcr				},
	{SERVICE__BRD_GET_COOP_TEST_TBL,		PF_CAST brdGetCoopTestTable		},
	{SERVICE__BRD_CONFIGURE_VXS_NT_PORTS,	PF_CAST brdConfigureVxsNTPorts	},
	{SERVICE__BRD_GET_VXS_TRANS_PORTS,		PF_CAST brdGetVxsTPorts			},
#endif
	{SERVICE__BRD_ENABLE_RS485_HD,			PF_CAST vEnRS485_hd				},
	{SERVICE__BRD_ENABLE_RS485_FD,			PF_CAST vEnRS485_fd				},
	{SERVICE__BRD_DISABLE_RS485,			PF_CAST vDisRS485				},
	{SERVICE__BRD_GET_RS485_PORT_MAP,       PF_CAST brdGetRS485PortMap      },
	{SERVICE__BRD_UNIVERSE_JUMPER,			PF_CAST brdCheckUniverseJumper	},
	{SERVICE__BRD_WATCHDOG_ENABLE_NMI,		PF_CAST brdWatchdogEnableNmi	},
	{SERVICE__BRD_WATCHDOG_ENABLE_RESET,	PF_CAST brdWatchdogEnableReset	},
	{SERVICE__BRD_WATCHDOG_CHECK_SWITCH,	PF_CAST brdWatchdogCheckSwitch	},
	{SERVICE__BRD_GET_GPIO_INFO,	        PF_CAST brdGetGPIOInfo       	},
	{SERVICE__BRD_GET_BIOS_INFO,			PF_CAST brdGetBiosInfo			},
	{SERVICE__BRD_GET_EXTENDED_INFO,		PF_CAST brdGetExtendedInfo		},
	{SERVICE__BRD_GET_TEMP_GRADE_STRING,	PF_CAST brdGetTempGradeString	},
	{SERVICE__BRD_GET_FORCE_DEBUG_ON,		PF_CAST brdGetForceDebugOn		},
	{SERVICE__BRD_GET_FORCE_DEBUG_OFF,		PF_CAST brdGetForceDebugOff		},
	{SERVICE__BRD_GET_THRESHOLD_INFO,		PF_CAST brdGetSensorThresholds	},
	{SERVICE__BRD_GET_STH_ORIDE_TEST_LIST,	PF_CAST brdGetSthTestList    	},
	{SERVICE__BRD_PROCESS_STH_ORIDE_LIST,	PF_CAST brdProcessSthTestList  	},
	{SERVICE__BRD_GET_BIOS_VERSION,	        PF_CAST brdGetBIOSVersion    	},
	{SERVICE__BRD_GET_BOARD_NAME,	        PF_CAST brdGetNameString    	},
	{SERVICE__BRD_SET_INTEL_SPEED_STEP,	    PF_CAST brdSetSpeedStep     	},
	{SERVICE__BRD_GET_ENXMC_GPIO_INFO,		PF_CAST brdGetXmcGpioInfo		},

	{SERVICE__UNKNOWN,						NULL                    		}
};

/*

	#include <bit/board_service.h>

	UINT16		port;
    UINT32 		rt;

    if(board_service(SERVICE__BRD_GET_CONSOLE_PORT, &rt, &port) == E__OK)
	{
		sysSccInit (port, BAUD_9600);
	}

*/

UINT32 board_service(UINT32 ServiceID, UINT32* rt, void* ptr)
{
	UINT32 i = 0;
	UINT32  rttemp;

	while(serviceCommands[i].iServiceID != NULL)
	{
		if(serviceCommands[i].iServiceID == ServiceID )
		{
			if(serviceCommands[i].pfnCmdHandler != NULL )
			{
				rttemp = (UINT32)(serviceCommands[i].pfnCmdHandler)(ptr);

				if(rt != NULL)
					*rt = rttemp;

				if(rttemp == E__OK)
					return E__OK;
				else
					return  SERVICE_REQUEST_ERROR;
			}
			else
				return SERVICE_NOT_SUPPORTED;
		}
		i++;
	}

	return SERVICE_NOT_SUPPORTED;
}


