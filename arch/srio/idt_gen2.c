/*
 * IDT CPS Gen.2 Serial RapidIO switch family support
 *
 * Copyright 2010 Integrated Device Technology, Inc.
 * Alexandre Bounine <alexandre.bounine@idt.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <stdio.h>
#include <string.h>
#include <bit/delay.h>
#include <bit/mem.h>
#include <private/debug.h>
#include <stdtypes.h>
#include <list.h>
#include <srio/rioerr.h>
#include <srio/rio_std.h>
#include <srio/rio_drv.h>
#include <srio/rio_ids.h>
#include <srio/rio_regs.h>

#include <srio/rio.h>


#define LOCAL_RTE_CONF_DESTID_SEL	0x010070
#define LOCAL_RTE_CONF_DESTID_SEL_PSEL	0x0000001f

#define IDT_LT_ERR_REPORT_EN	0x03100c

#define IDT_PORT_ERR_REPORT_EN(n)	(0x031044 + (n)*0x40)
#define IDT_PORT_ERR_REPORT_EN_BC	0x03ff04

#define IDT_PORT_ISERR_REPORT_EN(n)	(0x03104C + (n)*0x40)
#define IDT_PORT_ISERR_REPORT_EN_BC	0x03ff0c
#define IDT_PORT_INIT_TX_ACQUIRED	0x00000020

#define IDT_LANE_ERR_REPORT_EN(n)	(0x038010 + (n)*0x100)
#define IDT_LANE_ERR_REPORT_EN_BC	0x03ff10

#define IDT_DEV_CTRL_1		0xf2000c
#define IDT_DEV_CTRL_1_GENPW		0x02000000
#define IDT_DEV_CTRL_1_PRSTBEH		0x00000001

#define IDT_CFGBLK_ERR_CAPTURE_EN	0x020008
#define IDT_CFGBLK_ERR_REPORT		0xf20014
#define IDT_CFGBLK_ERR_REPORT_GENPW		0x00000002

#define IDT_AUX_PORT_ERR_CAP_EN	0x020000
#define IDT_AUX_ERR_REPORT_EN	0xf20018
#define IDT_AUX_PORT_ERR_LOG_I2C	0x00000002
#define IDT_AUX_PORT_ERR_LOG_JTAG	0x00000001

#define	IDT_ISLTL_ADDRESS_CAP	0x021014

#define IDT_RIO_DOMAIN		0xf20020
#define IDT_RIO_DOMAIN_MASK		0x000000ff

#define IDT_PW_INFO_CSR		0xf20024

#define IDT_SOFT_RESET		0xf20040
#define IDT_SOFT_RESET_REQ		0x00030097

#define IDT_I2C_MCTRL		0xf20050
#define IDT_I2C_MCTRL_GENPW		0x04000000

#define IDT_JTAG_CTRL		0xf2005c
#define IDT_JTAG_CTRL_GENPW		0x00000002

#define IDT_LANE_CTRL(n)	(0xff8000 + (n)*0x100)
#define IDT_LANE_CTRL_BC	0xffff00
#define IDT_LANE_CTRL_GENPW		0x00200000
#define IDT_LANE_DFE_1_BC	0xffff18
#define IDT_LANE_DFE_2_BC	0xffff1c

#define IDT_PORT_OPS(n)		(0xf40004 + (n)*0x100)
#define IDT_PORT_OPS_GENPW		0x08000000
#define IDT_PORT_OPS_PL_ELOG		0x00000040
#define IDT_PORT_OPS_LL_ELOG		0x00000020
#define IDT_PORT_OPS_LT_ELOG		0x00000010
#define IDT_PORT_OPS_BC		0xf4ff04

#define IDT_PORT_ISERR_DET(n)	(0xf40008 + (n)*0x100)

#define IDT_ERR_CAP		0xfd0000
#define IDT_ERR_CAP_LOG_OVERWR		0x00000004

#define IDT_ERR_RD		0xfd0004

#define IDT_DEFAULT_ROUTE	0xde
#define IDT_NO_ROUTE		0xdf

static int
idtg2_route_add_entry(struct rio_mport *mport, UINT16 destid, UINT8 hopcount,
		       UINT16 table, UINT16 route_destid, UINT8 route_port)
{
	/*
	 * Select routing table to update
	 */
#ifdef DEBUG_SRIO	
	sysDebugWriteString("\nIn:idtg2_route_add_entry\n");
#endif	
	if (table == RIO_GLOBAL_TABLE)
		table = 0;
	else
		table++;

	if (route_port == RIO_INVALID_ROUTE)
		route_port = IDT_DEFAULT_ROUTE;

	rio_mport_write_config_32(mport, destid, hopcount,
				  LOCAL_RTE_CONF_DESTID_SEL, table);

	/*
	 * Program destination port for the specified destID
	 */
	rio_mport_write_config_32(mport, destid, hopcount,
				  RIO_STD_RTE_CONF_DESTID_SEL_CSR,
				  (UINT32)route_destid);

	rio_mport_write_config_32(mport, destid, hopcount,
				  RIO_STD_RTE_CONF_PORT_SEL_CSR,
				  (UINT32)route_port);
	vDelay(10);
#ifdef DEBUG_SRIO	
    sysDebugWriteString("\nOut:idtg2_route_add_entry\n");
#endif    
	return 0;
}

static int
idtg2_route_get_entry(struct rio_mport *mport, UINT16 destid, UINT8 hopcount,
		       UINT16 table, UINT16 route_destid, UINT8 *route_port)
{
	UINT32 result;

	/*
	 * Select routing table to read
	 */
	if (table == RIO_GLOBAL_TABLE)
		table = 0;
	else
		table++;

	rio_mport_write_config_32(mport, destid, hopcount,
				  LOCAL_RTE_CONF_DESTID_SEL, table);

	rio_mport_write_config_32(mport, destid, hopcount,
				  RIO_STD_RTE_CONF_DESTID_SEL_CSR,
				  route_destid);

	rio_mport_read_config_32(mport, destid, hopcount,
				 RIO_STD_RTE_CONF_PORT_SEL_CSR, &result);

	if (IDT_DEFAULT_ROUTE == (UINT8)result || IDT_NO_ROUTE == (UINT8)result)
		*route_port = RIO_INVALID_ROUTE;
	else
		*route_port = (UINT8)result;

	return 0;
}

static int
idtg2_route_clr_table(struct rio_mport *mport, UINT16 destid, UINT8 hopcount,
		       UINT16 table)
{
	UINT32 i;

	/*
	 * Select routing table to read
	 */
	if (table == RIO_GLOBAL_TABLE)
		table = 0;
	else
		table++;

	rio_mport_write_config_32(mport, destid, hopcount,
				  LOCAL_RTE_CONF_DESTID_SEL, table);

	for (i = RIO_STD_RTE_CONF_EXTCFGEN;
	     i <= (RIO_STD_RTE_CONF_EXTCFGEN | 0xff);) {
		rio_mport_write_config_32(mport, destid, hopcount,
			RIO_STD_RTE_CONF_DESTID_SEL_CSR, i);
		rio_mport_write_config_32(mport, destid, hopcount,
			RIO_STD_RTE_CONF_PORT_SEL_CSR,
			(IDT_DEFAULT_ROUTE << 24) | (IDT_DEFAULT_ROUTE << 16) |
			(IDT_DEFAULT_ROUTE << 8) | IDT_DEFAULT_ROUTE);
		i += 4;
	}

	return 0;
}


static int
idtg2_set_domain(struct rio_mport *mport, UINT16 destid, UINT8 hopcount,
		       UINT8 sw_domain)
{
	/*
	 * Switch domain configuration operates only at global level
	 */
	rio_mport_write_config_32(mport, destid, hopcount,
				  IDT_RIO_DOMAIN, (UINT32)sw_domain);
	return 0;
}

static int
idtg2_get_domain(struct rio_mport *mport, UINT16 destid, UINT8 hopcount,
		       UINT8 *sw_domain)
{
	UINT32 regval;

	/*
	 * Switch domain configuration operates only at global level
	 */
	rio_mport_read_config_32(mport, destid, hopcount,
				IDT_RIO_DOMAIN, &regval);

	*sw_domain = (UINT8)(regval & 0xff);

	return 0;
}

static int
idtg2_em_init(struct rio_dev *rdev)
{
	UINT32 regval;
	int i, tmp;
#ifdef DEBUG_SRIO	
	char achBuffer[80];
#endif	

	/*
	 * This routine performs device-specific initialization only.
	 * All standard EM configuration should be performed at upper level.
	 */

#ifdef DEBUG_SRIO	
	sprintf(achBuffer,"RIO: %s [%d:%d]\n", __func__, rdev->destid, rdev->hopcount);
	sysDebugWriteString(achBuffer);
#endif
	/* Set Port-Write info CSR: PRIO=3 and CRF=1 */
	rio_write_config_32(rdev, IDT_PW_INFO_CSR, 0x0000e000);

	/*
	 * Configure LT LAYER error reporting.
	 */

	/* Enable standard (RIO.p8) error reporting */
	rio_write_config_32(rdev, IDT_LT_ERR_REPORT_EN,
			REM_LTL_ERR_ILLTRAN | REM_LTL_ERR_UNSOLR |
			REM_LTL_ERR_UNSUPTR);

	/* Use Port-Writes for LT layer error reporting.
	 * Enable per-port reset
	 */
	rio_read_config_32(rdev, IDT_DEV_CTRL_1, &regval);
	rio_write_config_32(rdev, IDT_DEV_CTRL_1,
			regval | IDT_DEV_CTRL_1_GENPW | IDT_DEV_CTRL_1_PRSTBEH);

	/*
	 * Configure PORT error reporting.
	 */

	/* Report all RIO.p8 errors supported by device */
	rio_write_config_32(rdev, IDT_PORT_ERR_REPORT_EN_BC, 0x807e8037);

	/* Configure reporting of implementation specific errors/events */
	rio_write_config_32(rdev, IDT_PORT_ISERR_REPORT_EN_BC,
			    IDT_PORT_INIT_TX_ACQUIRED);

	/* Use Port-Writes for port error reporting and enable error logging */
	tmp = RIO_GET_TOTAL_PORTS(rdev->swpinfo);
	for (i = 0; i < tmp; i++) {
		rio_read_config_32(rdev, IDT_PORT_OPS(i), &regval);
		rio_write_config_32(rdev,
				IDT_PORT_OPS(i), regval | IDT_PORT_OPS_GENPW |
				IDT_PORT_OPS_PL_ELOG |
				IDT_PORT_OPS_LL_ELOG |
				IDT_PORT_OPS_LT_ELOG);
	}
	/* Overwrite error log if full */
	rio_write_config_32(rdev, IDT_ERR_CAP, IDT_ERR_CAP_LOG_OVERWR);

	/*
	 * Configure LANE error reporting.
	 */

	/* Disable line error reporting */
	rio_write_config_32(rdev, IDT_LANE_ERR_REPORT_EN_BC, 0);

	/* Use Port-Writes for lane error reporting (when enabled)
	 * (do per-lane update because lanes may have different configuration)
	 */
	tmp = (rdev->did == RIO_DID_IDTCPS1848) ? 48 : 16;
	for (i = 0; i < tmp; i++) {
		rio_read_config_32(rdev, IDT_LANE_CTRL(i), &regval);
		rio_write_config_32(rdev, IDT_LANE_CTRL(i),
				    regval | IDT_LANE_CTRL_GENPW);
	}

	/*
	 * Configure AUX error reporting.
	 */

	/* Disable JTAG and I2C Error capture */
	rio_write_config_32(rdev, IDT_AUX_PORT_ERR_CAP_EN, 0);

	/* Disable JTAG and I2C Error reporting/logging */
	rio_write_config_32(rdev, IDT_AUX_ERR_REPORT_EN, 0);

	/* Disable Port-Write notification from JTAG */
	rio_write_config_32(rdev, IDT_JTAG_CTRL, 0);

	/* Disable Port-Write notification from I2C */
	rio_read_config_32(rdev, IDT_I2C_MCTRL, &regval);
	rio_write_config_32(rdev, IDT_I2C_MCTRL, regval & ~IDT_I2C_MCTRL_GENPW);

	/*
	 * Configure CFG_BLK error reporting.
	 */

	/* Disable Configuration Block error capture */
	rio_write_config_32(rdev, IDT_CFGBLK_ERR_CAPTURE_EN, 0);

	/* Disable Port-Writes for Configuration Block error reporting */
	rio_read_config_32(rdev, IDT_CFGBLK_ERR_REPORT, &regval);
	rio_write_config_32(rdev, IDT_CFGBLK_ERR_REPORT,
			    regval & ~IDT_CFGBLK_ERR_REPORT_GENPW);

	/* set TVAL = ~50us */
	rio_write_config_32(rdev,
		rdev->phys_efptr + RIO_PORT_LINKTO_CTL_CSR, 0x8e << 8);

	return 0;
}

static int
idtg2_em_handler(struct rio_dev *rdev, UINT8 portnum)
{
	UINT32 regval, em_perrdet, em_ltlerrdet;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif

	rio_read_config_32(rdev,
		rdev->em_efptr + RIO_EM_LTL_ERR_DETECT, &em_ltlerrdet);
	if (em_ltlerrdet) {
		/* Service Logical/Transport Layer Error(s) */
		if (em_ltlerrdet & REM_LTL_ERR_IMPSPEC) {
			/* Implementation specific error reported */
			rio_read_config_32(rdev,
					IDT_ISLTL_ADDRESS_CAP, &regval);

#ifdef DEBUG_SRIO	
			sprintf(achBuffer,"RIO: %s Implementation Specific LTL errors" \
				 " 0x%x @(0x%x)\n",
				 rio_name(rdev), em_ltlerrdet, regval);
			sysDebugWriteString(achBuffer);	 
#endif			

			/* Clear implementation specific address capture CSR */
			rio_write_config_32(rdev, IDT_ISLTL_ADDRESS_CAP, 0);

		}
	}

	rio_read_config_32(rdev,
		rdev->em_efptr + RIO_EM_PN_ERR_DETECT(portnum), &em_perrdet);
	if (em_perrdet) {
		/* Service Port-Level Error(s) */
		if (em_perrdet & REM_PED_IMPL_SPEC) {
			/* Implementation Specific port error reported */

			/* Get IS errors reported */
			rio_read_config_32(rdev,
					IDT_PORT_ISERR_DET(portnum), &regval);

#ifdef DEBUG_SRIO	
			sprintf(achBuffer,"RIO: %s Implementation Specific Port" \
				 " errors 0x%x\n", rio_name(rdev), regval);
			sysDebugWriteString(achBuffer);		 
#endif			

			/* Clear all implementation specific events */
			rio_write_config_32(rdev,
					IDT_PORT_ISERR_DET(portnum), 0);
		}
	}

	return 0;
}

int idtg2_switch_init(struct rio_dev *rdev, int do_enum)
{
	
#ifdef DEBUG_SRIO	
	char achBuffer[40];
	sprintf(achBuffer,"RIO: %s for %s\n", __func__, rio_name(rdev));
	sysDebugWriteString(achBuffer);	
#endif	
	rdev->rswitch->add_entry = idtg2_route_add_entry;
	rdev->rswitch->get_entry = idtg2_route_get_entry;
	rdev->rswitch->clr_table = idtg2_route_clr_table;
	rdev->rswitch->set_domain = idtg2_set_domain;
	rdev->rswitch->get_domain = idtg2_get_domain;
	rdev->rswitch->em_init = idtg2_em_init;
	rdev->rswitch->em_handle = idtg2_em_handler;
	

	if (do_enum) {
		/* Ensure that default routing is disabled on startup */
		rio_write_config_32(rdev,
				    RIO_STD_RTE_DEFAULT_PORT, IDT_NO_ROUTE);
	}

	return 0;
}


