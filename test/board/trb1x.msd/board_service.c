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
 *																		*
 ************************************************************************/

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/trb1x.msd/board_service.c,v 1.5 2014-08-04 08:47:42 mgostling Exp $
 *
 * $Log: board_service.c,v $
 * Revision 1.5  2014-08-04 08:47:42  mgostling
 * Rationalise service call to obtain memory size from GPIO
 *
 * Revision 1.4  2014-07-29 11:39:29  mgostling
 * Implement SERVICE__BRD_SKIP_ETH_IF so that the network interface
 * to the Universal Soak Master is not modified during ethernet device testing.
 *
 * Revision 1.3  2014-03-27 11:56:46  mgostling
 * Added a new test to vefify XMC differention IO signals.
 *
 * Revision 1.2  2014-01-10 09:58:50  mgostling
 * Added new service to obtain DRAM size.
 *
 * Revision 1.1  2013-11-25 14:21:34  mgostling
 * Added support for TRB1x.
 *
 *
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
	{SERVICE__BRD_GET_SCC_PORT_MAP, 		PF_CAST brdGetSccPortMap	    },
	{SERVICE__BRD_GET_SCC_PARAMS, 			PF_CAST brdGetSccParams   	    },
	{SERVICE__BRD_GET_MAX_DRAM_BELOW_4GB,	PF_CAST brdGetMaxDramBelow4GB   },
	{SERVICE__BRD_FLASH_LED,	    		PF_CAST brdFlashLed			    },
	{SERVICE__BRD_RESET,	    			PF_CAST brdReset			    },
	{SERVICE__BRD_USR_MTH_SWITCH,	    	PF_CAST brdCheckUserMthSwitch   },
	{SERVICE__BRD_GET_SOAK_PORT,	    	PF_CAST brdGetSokePort		    },
	{SERVICE__BRD_GET_NUM_CORES,	    	PF_CAST brdGetNumCores		    },
	{SERVICE__BRD_SELECT_ETH_IF,	    	PF_CAST brdSelectEthInterface	},
	{SERVICE__BRD_GET_FPU_EXCEPTION_MAP,	PF_CAST brdGetFpuExceptionMap   },
	{SERVICE__BRD_HARDWARE_INIT,	    	PF_CAST brdHwInit		    	},
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
	{SERVICE__BRD_GET_LED_INFO,				PF_CAST brdLEDinfo				},
	{SERVICE__BRD_GET_SPI_EEPROM_INFO,		PF_CAST brdSpiEEPROMInfo		},
	{SERVICE__BRD_GET_SERIAL_BAUD_INFO,		PF_CAST brdSerialBaudInfo		},
	{SERVICE__BRD_GET_IPMI_NMI, 			PF_CAST brdGetIPMINMI			},
	{SERVICE__BRD_CLEAR_IPMI_NMI, 		    PF_CAST brdClearIPMINMI			},
	{SERVICE__BRD_POST_SCAN_INIT,			PF_CAST brdPostScanInit   		},

#ifdef CUTE
	{SERVICE__BRD_INIT_ICR,					PF_CAST brdInitIcr				},
#endif
	{SERVICE__BRD_GET_FLASH_DRV_PARAMS,		PF_CAST brdGetFlashDriveParams	},
	{SERVICE__BRD_GET_RS485_PORT_MAP,       PF_CAST brdGetRS485PortMap      },
	{SERVICE__BRD_ENABLE_RS485_HD,			PF_CAST vEnRS485_hd				},
	{SERVICE__BRD_ENABLE_RS485_FD,			PF_CAST vEnRS485_fd				},
	{SERVICE__BRD_DISABLE_RS485,			PF_CAST vDisRS485				},
	{SERVICE__BRD_GET_SPI_MRAM_INFO,        PF_CAST brdMRAMinfo				},
	{SERVICE__BRD_GET_TRXMC_PCI_DB,			PF_CAST brdGetTrXmcPCIDB		},
	{SERVICE__BRD_CHK_PCIE_SW_CONFIG,		PF_CAST brdCheckPcieSwConfig	},
	{SERVICE__BRD_GET_TRXMC_PARENT_BUS,		PF_CAST brdGetTrXmcParentBus	},
	{SERVICE__BRD_CONFIG_VPX_BACKPLANE,     PF_CAST brdConfigureVPXNTPorts  },
	{SERVICE__BRD_GET_VPX_INSTANCE,	        PF_CAST brdGetVpxBrInstance     },
	{SERVICE__BRD_SET_INTEL_SPEED_STEP,	    PF_CAST brdSetSpeedStep     	},
	{SERVICE__BRD_DECODE_TPM_LPC_IOWINDOW,	PF_CAST brdDecodeTpmIo			},
	{SERVICE__BRD_ENABLE_SMIC_LPC_IOWINDOW, PF_CAST brdDecodeSmicIo			},
	{SERVICE__BRD_ENABLE_HDA,           	PF_CAST brdEnableHDA			},
	{SERVICE__BRD_SET_LDTPIT_INTR, 			PF_CAST brdLdtPitIntEnable		},
	{SERVICE__BRD_GET_DRAM_SIZE_FROM_GPIO,	PF_CAST brdGetDramSize			},
	{SERVICE__BRD_GET_ENXMC_GPIO_INFO,		PF_CAST brdGetXmcGpioInfo		},
	{SERVICE__BRD_SKIP_ETH_IF,				PF_CAST brdSkipEthInterface		},

	{SERVICE__UNKNOWN,						NULL                    		}
};

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


