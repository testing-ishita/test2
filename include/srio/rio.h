/*
 * RapidIO interconnect services
 *
 * Copyright 2005 MontaVista Software, Inc.
 * Matt Porter <mporter@kernel.crashing.org>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <stdtypes.h>
#include <srio/rio_std.h>

#define CONFIG_RAPIDIO_ENABLE_RX_TX_PORTS 1

#define CONFIG_RAPIDIO_DISC_TIMEOUT (30*2)
#define RIO_MAX_CHK_RETRY	3

#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))


/* Structures */
struct rio_net_list{
	struct list_head element;
	struct rio_net	*net;
};

/* Functions internal to the RIO core code */

extern UINT32 rio_mport_get_feature(struct rio_mport *mport, int local, UINT16 destid,
				 UINT8 hopcount, int ftr);
extern UINT32 rio_mport_get_physefb(struct rio_mport *port, int local,
				 UINT16 destid, UINT8 hopcount);
extern UINT32 rio_mport_get_efb(struct rio_mport *port, int local, UINT16 destid,
			     UINT8 hopcount, UINT32 from);
extern int rio_mport_chk_dev_access(struct rio_mport *mport, UINT16 destid,
				    UINT8 hopcount);
extern int rio_create_sysfs_dev_files(struct rio_dev *rdev);
extern void rio_remove_sysfs_dev_files(struct rio_dev *rdev);
extern int rio_enum_mport(struct rio_mport *mport);
extern int rio_free_mport(struct rio_mport *mport);
extern int rio_disc_mport(struct rio_mport *mport);
extern void rio_free_devices( void );
extern int rio_std_route_add_entry(struct rio_mport *mport, UINT16 destid,
				   UINT8 hopcount, UINT16 table, UINT16 route_destid,
				   UINT8 route_port);
extern int rio_std_route_get_entry(struct rio_mport *mport, UINT16 destid,
				   UINT8 hopcount, UINT16 table, UINT16 route_destid,
				   UINT8 *route_port);
extern int rio_std_route_clr_table(struct rio_mport *mport, UINT16 destid,
				   UINT8 hopcount, UINT16 table);
extern int rio_set_port_lockout(struct rio_dev *rdev, UINT32 pnum, int lock);
extern struct rio_dev *rio_get_comptag(UINT32 comp_tag, struct rio_dev *from);
extern int rio_init_mports(void);

/* Structures internal to the RIO core code */
//extern struct device_attribute rio_dev_attrs[];
//extern struct bus_attribute rio_bus_attrs[];
//extern spinlock_t rio_global_list_lock;

extern struct rio_switch_ops __start_rio_switch_ops[];
extern struct rio_switch_ops __end_rio_switch_ops[];

/* Helpers internal to the RIO core code */
#define DECLARE_RIO_SWITCH_SECTION(section, name, vid, did, init_hook) \
	static const struct rio_switch_ops __rio_switch_##name __used \
	__section(section) = { vid, did, init_hook };

/**
 * DECLARE_RIO_SWITCH_INIT - Registers switch initialization routine
 * @vid: RIO vendor ID
 * @did: RIO device ID
 * @init_hook: Callback that performs switch-specific initialization
 *
 * Manipulating switch route tables and error management in RIO
 * is switch specific. This registers a switch by vendor and device ID with
 * initialization callback for setting up switch operations and (if required)
 * hardware initialization. A &struct rio_switch_ops is initialized with
 * pointer to the init routine and placed into a RIO-specific kernel section.
 */
#define DECLARE_RIO_SWITCH_INIT(vid, did, init_hook)		\
	DECLARE_RIO_SWITCH_SECTION(.rio_switch_ops, vid##did, \
			vid, did, init_hook)

#define RIO_GET_DID(size, x)	(size ? (x & 0xffff) : ((x & 0x00ff0000) >> 16))
#define RIO_SET_DID(size, x)	(size ? (x & 0xffff) : ((x & 0x000000ff) << 16))
