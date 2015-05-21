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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/board_service.c,v 1.2 2015-04-01 16:44:29 hchalla Exp $
 *
 * $Log: board_service.c,v $
 * Revision 1.2  2015-04-01 16:44:29  hchalla
 * Initial release of CUTE firmware 02.02.01.
 *
 * Revision 1.1  2015-03-17 10:37:55  hchalla
 * Initial Checkin for PP B1x Board.
 *
 * Revision 1.2  2013-09-27 12:25:21  chippisley
 * Added board service to retrieve slot ID.
 *
 * Revision 1.1  2013/09/04 07:13:49  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:25:23  hchalla
 * Initial verion of PP 81x sources.
 *
 *
 */

#include <stdtypes.h>
#include <bit/bit.h>
#include <bit/board_service.h>

#include "../ppb1x.msd/services.h"



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
	{SERVICE__BRD_GET_SLOT_ID,				PF_CAST brdGetSlotId     		},
	
#ifdef CUTE
	{SERVICE__BRD_INIT_ICR,					PF_CAST brdInitIcr				},
#endif
	{SERVICE__BRD_GET_FLASH_DRV_PARAMS,		PF_CAST brdGetFlashDriveParams	},
	{SERVICE__BRD_GET_RS485_PORT_MAP,       PF_CAST brdGetRS485PortMap      },
	{SERVICE__BRD_ENABLE_RS485_HD,			PF_CAST vEnRS485_hd				},
	{SERVICE__BRD_ENABLE_RS485_FD,			PF_CAST vEnRS485_fd				},
	{SERVICE__BRD_DISABLE_RS485,			PF_CAST vDisRS485				},
	{SERVICE__BRD_GET_SPI_MRAM_INFO,        PF_CAST brdMRAMinfo	},
	{SERVICE__BRD_LDT_CLEAR_INTR,                    PF_CAST ldtClearInt },
	{SERVICE__BRD_CONFIG_CPCI_BACKPLANE,	PF_CAST brdConfigCpciBackplane	},
	{SERVICE__BRD_DECONFIG_CPCI_BACKPLANE,	PF_CAST brdDeConfigCpciBackplane},
	{SERVICE__BRD_CHECK_CPCI_IS_SYSCON,     PF_CAST brdCheckCpciIsSyscon    },
	{SERVICE__BRD_GET_CPCI_INSTANCE,	    PF_CAST brdGetCpciBrInstance    },
	{SERVICE__BRD_SET_INTEL_SPEED_STEP,	    PF_CAST brdSetSpeedStep     	},
	{SERVICE__BRD_GET_BIOS_INFO,			PF_CAST brdGetBiosInfo			},
	{SERVICE__BRD_GET_EXTENDED_INFO,		PF_CAST brdGetExtendedInfo		},
	{SERVICE__BRD_GET_BIOS_VERSION,	        PF_CAST brdGetBIOSVersion    	},
	{SERVICE__BRD_GET_BOARD_NAME,	        PF_CAST brdGetNameString    	},
	{SERVICE__BRD_SELECT_SPI_FWH,				PF_CAST brdSelectSpiFwh},
	{SERVICE__BRD_GET_TEMP_GRADE_STRING,	PF_CAST brdGetTempGradeString	},
	{SERVICE__BRD_SKIP_ETH_IF,	    		PF_CAST brdSkipEthInterface		},
	{SERVICE__BRD_GET_DS3905_INFO,			PF_CAST brdDS3905info			},
	{SERVICE__BRD_GET_DRAM_SIZE_FROM_GPIO,	PF_CAST brdGetDramSize			},
	{SERVICE__BRD_UNLOCK_CCT_IOREGS, PF_CAST brdUnLockCCTIoRegs},
	// {SERVICE__BRD_GET_SLOT_ID,                              PF_CAST brdGetSlotId},
	{SERVICE__BRD_LOCK_CCT_IOREGS, PF_CAST brdLockCCTIoRegs},
	{SERVICE__BRD_GET_CPCI_BRIDGE_IMAGES,	PF_CAST brdGetCpciDeviceImages	},
	{SERVICE__BRD_GET_CPCI_SLAVE_IMAGES,	PF_CAST brdGetCPCISlaveImages	},
	{SERVICE__BRD_GET_CPCI_UPSTREAM_IMAGE,	PF_CAST brdGetCPCIUpStreamImage	},



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


