/*
 * RapidIO enumeration and discovery support
 *
 * Copyright 2005 MontaVista Software, Inc.
 * Matt Porter <mporter@kernel.crashing.org>
 *
 * Copyright 2009 Integrated Device Technology, Inc.
 * Alex Bounine <alexandre.bounine@idt.com>
 * - Added Port-Write/Error Management initialization and handling
 *
 * Copyright 2009 Sysgo AG
 * Thomas Moll <thomas.moll@sysgo.com>
 * - Added Input- Output- enable functionality, to allow full communication
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <stdtypes.h>
#include <errors.h>
#include <bitops.h>
#include <bit/delay.h>
#include <private/debug.h>
#include <private/cpu.h>
#include <private/k_alloc.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/debug.h>
#include <srio/rioerr.h>
#include <srio/rio_std.h>
#include <srio/rio_drv.h>
#include <srio/rio_ids.h>
#include <srio/rio_regs.h>

#include <srio/rio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "crhook.h"
#include "death.h"
#include "semphr.h"

// #define DEBUG_SRIO

   /*
    * Check at compile time that something is of a particular type.
    * Always evaluates to 1 so you may use it easily in comparisons.
    */
   #define typecheck(type,x) \
   ({      type __dummy; \
          typeof(x) __dummy2; \
          (void)(&__dummy == &__dummy2); \
          1; \
  })
  
  /*
   * Check at compile time that 'function' is a certain type, or is a pointer
   * to that type (needs to use typedef for the function type.)
   */
  #define typecheck_fn(type,function) \
  ({      typeof(type) __tmp = function; \
          (void)__tmp; \
  })


#define time_after(a,b)         \
         (typecheck(unsigned long, a) && \
          typecheck(unsigned long, b) && \
          ((long)(b) - (long)(a) < 0))

#define time_before(a,b)        time_after(b,a)

/*#define time_before(unknown, known) ((long)(unknown) - (long)(known) < 0)*/

/*extern int tsi500_switch_init(struct rio_dev *rdev, int do_enum);*/
extern int idtg2_switch_init(struct rio_dev *rdev, int do_enum);
/*extern int idtcps_switch_init(struct rio_dev *rdev, int do_enum);*/
/*extern int tsi568_switch_init(struct rio_dev *rdev, int do_enum);*/
/*extern int tsi57x_switch_init(struct rio_dev *rdev, int do_enum);*/

static struct rio_switch_ops rioSwitchTable[] = {
	/*	{ RIO_VID_TUNDRA, RIO_DID_TSI500, tsi500_switch_init },*/
		{ RIO_VID_IDT, 	  RIO_DID_IDTCPS1848, idtg2_switch_init  },
		{ RIO_VID_IDT,    RIO_DID_IDTCPS1616, idtg2_switch_init  },
		{ RIO_VID_IDT, 	  RIO_DID_IDTVPS1616, idtg2_switch_init	 },
		{ RIO_VID_IDT, 	  RIO_DID_IDTSPS1616, idtg2_switch_init	 },
		{ RIO_VID_IDT, 	  RIO_DID_IDTCPS1432, idtg2_switch_init  },
		/*{ RIO_VID_IDT, 	  RIO_DID_IDTCPS6Q,   idtcps_switch_init },
		{ RIO_VID_IDT, 	  RIO_DID_IDTCPS8,    idtcps_switch_init },
		{ RIO_VID_IDT, 	  RIO_DID_IDTCPS10Q,  idtcps_switch_init },
		{ RIO_VID_IDT, 	  RIO_DID_IDTCPS12,   idtcps_switch_init },
		{ RIO_VID_IDT, 	  RIO_DID_IDTCPS16,   idtcps_switch_init },
		{ RIO_VID_IDT, 	  RIO_DID_IDT70K200,  idtcps_switch_init },*/
		/*{ RIO_VID_TUNDRA, RIO_DID_TSI568, 	  tsi568_switch_init },*/
		/*{ RIO_VID_TUNDRA, RIO_DID_TSI572, 	  tsi57x_switch_init },
		{ RIO_VID_TUNDRA, RIO_DID_TSI574, 	  tsi57x_switch_init },
		{ RIO_VID_TUNDRA, RIO_DID_TSI577, 	  tsi57x_switch_init },
		{ RIO_VID_TUNDRA, RIO_DID_TSI578, 	  tsi57x_switch_init },*/
		{ 0 , 0, NULL },
};


LIST_HEAD(rio_devices);
LIST_HEAD(rio_netd);

static void rio_init_em(struct rio_dev *rdev);


static int next_destid = 0;
static int next_comptag = 1;

static int rio_mport_phys_table[] = 
{
	RIO_EFB_PAR_EP_ID,
	RIO_EFB_PAR_EP_REC_ID,
	RIO_EFB_SER_EP_ID,
	RIO_EFB_SER_EP_REC_ID,
	-1,
};


/**
 * rio_destid_alloc - Allocate next available destID for given network
 * @net: RIO network
 *
 * Returns next available device destination ID for the specified RIO network.
 * Marks allocated ID as one in use.
 * Returns RIO_INVALID_DESTID if new destID is not available.
 */
static UINT16 rio_destid_alloc(struct rio_net *net)
{
	int destid;
	struct rio_id_table *idtab = &net->destid_table;

	//spin_lock(&idtab->lock);
	destid = find_first_zero_bit(idtab->table, idtab->max);

	if (destid < idtab->max) {
		set_bit(destid, idtab->table);
		destid += idtab->start;
	} else
		destid = RIO_INVALID_DESTID;

	//spin_unlock(&idtab->lock);
	return (UINT16)destid;
}

/**
 * rio_destid_reserve - Reserve the specivied destID
 * @net: RIO network
 * @destid: destID to reserve
 *
 * Tries to reserve the specified destID.
 * Returns 0 if successfull.
 */
static int rio_destid_reserve(struct rio_net *net, UINT16 destid)
{
	int oldbit;
	struct rio_id_table *idtab = &net->destid_table;

	destid -= idtab->start;
	//spin_lock(&idtab->lock);
	oldbit = test_and_set_bit(destid, idtab->table);
	//spin_unlock(&idtab->lock);
	return oldbit;
}

/**
 * rio_destid_free - free a previously allocated destID
 * @net: RIO network
 * @destid: destID to free
 *
 * Makes the specified destID available for use.
 */
static void rio_destid_free(struct rio_net *net, UINT16 destid)
{
	struct rio_id_table *idtab = &net->destid_table;

	destid -= idtab->start;
	//spin_lock(&idtab->lock);
	clear_bit(destid, idtab->table);
	//spin_unlock(&idtab->lock);
}

/**
 * rio_destid_first - return first destID in use
 * @net: RIO network
 */
static UINT16 rio_destid_first(struct rio_net *net)
{
	int destid;
	struct rio_id_table *idtab = &net->destid_table;

	//spin_lock(&idtab->lock);
	destid = find_first_bit(idtab->table, idtab->max);
	if (destid >= idtab->max)
		destid = RIO_INVALID_DESTID;
	else
		destid += idtab->start;
	//spin_unlock(&idtab->lock);
	return (UINT16)destid;
}

/**
 * rio_destid_next - return next destID in use
 * @net: RIO network
 * @from: destination ID from which search shall continue
 */
static UINT16 rio_destid_next(struct rio_net *net, UINT16 from)
{
	int destid;
	struct rio_id_table *idtab = &net->destid_table;
#ifdef DEBUG_SRIO	
	char achBuffer[80];
#endif	

	//spin_lock(&idtab->lock);
	destid = find_next_bit(idtab->table, idtab->max, from);
#ifdef DEBUG_SRIO	
	sprintf (achBuffer,"1.rio_destid_next():destid:%d\n",destid);
	sysDebugWriteString(achBuffer);
#endif	
	
	if (destid >= idtab->max)
	{
		destid = RIO_INVALID_DESTID;
	}
	else
	{
		destid += idtab->start;
	}
#ifdef DEBUG_SRIO	
	sprintf (achBuffer,"2.rio_destid_next():destid:%d\n",destid);
	sysDebugWriteString(achBuffer);
#endif	
	//spin_unlock(&idtab->lock);
	return (UINT16)destid;
}

/**
 * rio_get_device_id - Get the base/extended device id for a device
 * @port: RIO master port
 * @destid: Destination ID of device
 * @hopcount: Hopcount to device
 *
 * Reads the base/extended device id from a device. Returns the
 * 8/16-bit device ID.
 */
static UINT16 rio_get_device_id(struct rio_mport *port, UINT16 destid, UINT8 hopcount)
{
	UINT32 result;

	rio_mport_read_config_32(port, destid, hopcount, RIO_DID_CSR, &result);

	return RIO_GET_DID(port->sys_size, result);
}

/**
 * rio_set_device_id - Set the base/extended device id for a device
 * @port: RIO master port
 * @destid: Destination ID of device
 * @hopcount: Hopcount to device
 * @did: Device ID value to be written
 *
 * Writes the base/extended device id from a device.
 */
static void rio_set_device_id(struct rio_mport *port, UINT16 destid, UINT8 hopcount, UINT16 did)
{
	rio_mport_write_config_32(port, destid, hopcount, RIO_DID_CSR,
				  RIO_SET_DID(port->sys_size, did));
}

/**
 * rio_local_set_device_id - Set the base/extended device id for a port
 * @port: RIO master port
 * @did: Device ID value to be written
 *
 * Writes the base/extended device id from a device.
 */
static void rio_local_set_device_id(struct rio_mport *port, UINT16 did)
{
	rio_local_write_config_32(port, RIO_DID_CSR, RIO_SET_DID(port->sys_size,
				did));
}

/**
 * rio_clear_locks- Release all host locks and signal enumeration complete
 * @net: RIO network to run on
 *
 * Marks the component tag CSR on each device with the enumeration
 * complete flag. When complete, it then release the host locks on
 * each device. Returns 0 on success or %-EINVAL on failure.
 */
static int rio_clear_locks(struct rio_net *net)
{
	struct rio_mport *port = net->hport;
	struct rio_dev *rdev;
	UINT32 result;
	int ret = 0;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif
	/* Release host device id locks */
	rio_local_write_config_32(port, RIO_HOST_DID_LOCK_CSR,
				  port->host_deviceid);
	rio_local_read_config_32(port, RIO_HOST_DID_LOCK_CSR, &result);
	if ((result & 0xffff) != 0xffff) {
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"\nRIO: badness when releasing host lock on master port, result %x\n",result);
		sysDebugWriteString(achBuffer);
#endif		
		ret = -EINVAL;
	}
	list_for_each_entry(rdev, &net->devices, net_list) {
		rio_write_config_32(rdev, RIO_HOST_DID_LOCK_CSR,
				    port->host_deviceid);
		rio_read_config_32(rdev, RIO_HOST_DID_LOCK_CSR, &result);
		if ((result & 0xffff) != 0xffff) {
#ifdef DEBUG_SRIO	
			sprintf(achBuffer,"RIO: badness when releasing host lock on vid %x did %x\n",rdev->vid, rdev->did);
			sysDebugWriteString(achBuffer);
#endif			
			ret = -EINVAL;
		}

		/* Mark device as discovered and enable master */
		rio_read_config_32(rdev,
				   rdev->phys_efptr + RIO_PORT_GEN_CTL_CSR,
				   &result);
		result |= RIO_PORT_GEN_DISCOVERED | RIO_PORT_GEN_MASTER;
		rio_write_config_32(rdev,
				    rdev->phys_efptr + RIO_PORT_GEN_CTL_CSR,
				    result);
	}

	return ret;
}

/**
 * rio_enum_host- Set host lock and initialize host destination ID
 * @port: Master port to issue transaction
 *
 * Sets the local host master port lock and destination ID register
 * with the host device ID value. The host device ID value is provided
 * by the platform. Returns %0 on success or %-1 on failure.
 */
static int rio_enum_host(struct rio_mport *port)
{
	UINT32 result;

	/* Set master port host device id lock */
	rio_local_write_config_32(port, RIO_HOST_DID_LOCK_CSR,
				  port->host_deviceid);

	rio_local_read_config_32(port, RIO_HOST_DID_LOCK_CSR, &result);
	if ((result & 0xffff) != port->host_deviceid)
		return -1;

	/* Set master port destid and init destid ctr */
	rio_local_set_device_id(port, port->host_deviceid);
	return 0;
}

/**
 * rio_device_has_destid- Test if a device contains a destination ID register
 * @port: Master port to issue transaction
 * @src_ops: RIO device source operations
 * @dst_ops: RIO device destination operations
 *
 * Checks the provided @src_ops and @dst_ops for the necessary transaction
 * capabilities that indicate whether or not a device will implement a
 * destination ID register. Returns 1 if true or 0 if false.
 */
static int rio_device_has_destid(struct rio_mport *port, int src_ops,
				 int dst_ops)
{
	UINT32 mask = RIO_OPS_READ | RIO_OPS_WRITE | RIO_OPS_ATOMIC_TST_SWP | RIO_OPS_ATOMIC_INC | RIO_OPS_ATOMIC_DEC | RIO_OPS_ATOMIC_SET | RIO_OPS_ATOMIC_CLR;

	return !!((src_ops | dst_ops) & mask);
}

/**
 * rio_is_switch- Tests if a RIO device has switch capabilities
 * @rdev: RIO device
 *
 * Gets the RIO device Processing Element Features register
 * contents and tests for switch capabilities. Returns 1 if
 * the device is a switch or 0 if it is not a switch.
 * The RIO device struct is freed.
 */
static int rio_is_switch(struct rio_dev *rdev)
{
	if (rdev->pef & RIO_PEF_SWITCH)
		return 1;
	return 0;
}

/**
 * rio_switch_init - Sets switch operations for a particular vendor switch
 * @rdev: RIO device
 * @do_enum: Enumeration/Discovery mode flag
 *
 * Searches the RIO switch ops table for known switch types. If the vid
 * and did match a switch table entry, then call switch initialization
 * routine to setup switch-specific routines.
 */
static void rio_switch_init(struct rio_dev *rdev, int do_enum)
{
	int i=0,found=0;
#ifdef DEBUG_SRIO	
	char achBuffer[80];
#endif	

	while (rioSwitchTable[i].did != 0 ) {
		if ((rioSwitchTable[i].vid == rdev->vid) && (rioSwitchTable[i].did == rdev->did)) {
#ifdef DEBUG_SRIO	
			sprintf(achBuffer,"\nRIO: calling init routine for %s\n",
				 rio_name(rdev));
			sysDebugWriteString(achBuffer);
#endif			
			found=1;
			rioSwitchTable[i].init_hook(rdev, do_enum);
			break;
		}
		i++;
	}

	if ( found && (rdev->pef & RIO_PEF_STD_RT)) {
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nRIO: adding STD routing ops for %s\n",
			rio_name(rdev));
		sysDebugWriteString(achBuffer);
#endif		
		
		rdev->rswitch->add_entry = rio_std_route_add_entry;
		rdev->rswitch->get_entry = rio_std_route_get_entry;
		rdev->rswitch->clr_table = rio_std_route_clr_table;
	}

	if (!rdev->rswitch->add_entry || !rdev->rswitch->get_entry)
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nRIO: missing routing ops for %s\n",
		       rio_name(rdev));
		sysDebugWriteString(achBuffer);
#endif		
	}

}

/**
 * rio_add_device- Adds a RIO device to the device model
 * @rdev: RIO device
 *
 * Adds the RIO device to the global device list and adds the RIO
 * device to the RIO device list.  Creates the generic sysfs nodes
 * for an RIO device.
 */
static int rio_add_device(struct rio_dev *rdev)
{

	//spin_lock(&rio_global_list_lock);
	list_add_tail(&rdev->global_list, &rio_devices);
	//spin_unlock(&rio_global_list_lock);

	return 0;
}

/**
 * rio_enable_rx_tx_port - enable input receiver and output transmitter of
 * given port
 * @port: Master port associated with the RIO network
 * @local: local=1 select local port otherwise a far device is reached
 * @destid: Destination ID of the device to check host bit
 * @hopcount: Number of hops to reach the target
 * @port_num: Port (-number on switch) to enable on a far end device
 *
 * Returns 0 or 1 from on General Control Command and Status Register
 * (EXT_PTR+0x3C)
 */
inline int rio_enable_rx_tx_port(struct rio_mport *port,
				 int local, UINT16 destid,
				 UINT8 hopcount, UINT8 port_num) 
{

#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif	
#ifdef CONFIG_RAPIDIO_ENABLE_RX_TX_PORTS
	UINT32 regval;
	UINT32 ext_ftr_ptr;

	/*
	* enable rx input tx output port
	*/
#ifdef DEBUG_SRIO
	sprintf(achBuffer,"rio_enable_rx_tx_port(local = %d, destid = %d, hopcount = "
		 "%d, port_num = %d)\n", local, destid, hopcount, port_num);
	sysDebugWriteString(achBuffer);	 
    
    sysDebugWriteString("In rio_enable_rx_tx_port()\n");
#endif    
	ext_ftr_ptr = rio_mport_get_physefb(port, local, destid, hopcount);

	if (local) {
		rio_local_read_config_32(port, ext_ftr_ptr +
				RIO_PORT_N_CTL_CSR(0),
				&regval);
	} else {
		if (rio_mport_read_config_32(port, destid, hopcount,
		ext_ftr_ptr + RIO_PORT_N_CTL_CSR(port_num), &regval) < 0)
			return -EIO;
	}

	if (regval & RIO_PORT_N_CTL_P_TYP_SER) {
		/* serial */
		regval = regval | RIO_PORT_N_CTL_EN_RX_SER
				| RIO_PORT_N_CTL_EN_TX_SER;
	} else {
		/* parallel */
		regval = regval | RIO_PORT_N_CTL_EN_RX_PAR
				| RIO_PORT_N_CTL_EN_TX_PAR;
	}

	if (local) {
		rio_local_write_config_32(port, ext_ftr_ptr +
					  RIO_PORT_N_CTL_CSR(0), regval);
	} else {
		if (rio_mport_write_config_32(port, destid, hopcount,
		    ext_ftr_ptr + RIO_PORT_N_CTL_CSR(port_num), regval) < 0)
			return -EIO;
	}
#endif
#ifdef DEBUG_SRIO
	sysDebugWriteString("Out rio_enable_rx_tx_port()\n");
#endif	
	return 0;
}

/**
 * rio_setup_device- Allocates and sets up a RIO device
 * @net: RIO network
 * @port: Master port to send transactions
 * @destid: Current destination ID
 * @hopcount: Current hopcount
 * @do_enum: Enumeration/Discovery mode flag
 *
 * Allocates a RIO device and configures fields based on configuration
 * space contents. If device has a destination ID register, a destination
 * ID is either assigned in enumeration mode or read from configuration
 * space in discovery mode.  If the device has switch capabilities, then
 * a switch is allocated and configured appropriately. Returns a pointer
 * to a RIO device on success or NULL on failure.
 *
 */
static struct rio_dev *rio_setup_device(struct rio_net *net,
					struct rio_mport *port, UINT16 destid,
					UINT8 hopcount, int do_enum)
{
	int ret = 0;
	struct rio_dev *rdev;
	struct rio_switch *rswitch = NULL;
	/*int result=0, rdid=0;*/
	UINT32 result =0, rdid = 0;
	long size=0;
	UINT32 swpinfo = 0;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif

	size = sizeof(struct rio_dev);
	if (rio_mport_read_config_32(port, destid, hopcount,
				     RIO_PEF_CAR, &result))
		return NULL;

	if (result & (RIO_PEF_SWITCH | RIO_PEF_MULTIPORT)) {
		rio_mport_read_config_32(port, destid, hopcount,
					 RIO_SWP_INFO_CAR, &swpinfo);
		if (result & RIO_PEF_SWITCH) {
			size += (RIO_GET_TOTAL_PORTS(swpinfo) *
				sizeof(rswitch->nextdev[0])) + sizeof(*rswitch);
		}
	}

#ifdef DEBUG_SRIO
	sprintf (achBuffer,"swpinfo:%x\n",swpinfo);
	sysDebugWriteString(achBuffer);
#endif	
	
	rdev = (struct rio_dev*)k_malloc(size);
        memset(rdev,0,size);

	if (!rdev)
		return NULL;

	rdev->net = net;
	rdev->pef = result;
	rdev->swpinfo = swpinfo;
	rio_mport_read_config_32(port, destid, hopcount, RIO_DEV_ID_CAR,
				 &result);
	rdev->did = result >> 16;
	rdev->vid = result & 0xffff;
	rio_mport_read_config_32(port, destid, hopcount, RIO_DEV_INFO_CAR,
				 &rdev->device_rev);
	rio_mport_read_config_32(port, destid, hopcount, RIO_ASM_ID_CAR,
				 &result);
	rdev->asm_did = result >> 16;
	rdev->asm_vid = result & 0xffff;
	rio_mport_read_config_32(port, destid, hopcount, RIO_ASM_INFO_CAR,
				 &result);
	rdev->asm_rev = result >> 16;
	if (rdev->pef & RIO_PEF_EXT_FEATURES) {
		rdev->efptr = result & 0xffff;
		rdev->phys_efptr = rio_mport_get_physefb(port, 0, destid,
							 hopcount);

		rdev->em_efptr = rio_mport_get_feature(port, 0, destid,
						hopcount, RIO_EFB_ERR_MGMNT);
	}

	rio_mport_read_config_32(port, destid, hopcount, RIO_SRC_OPS_CAR,
				 &rdev->src_ops);
	rio_mport_read_config_32(port, destid, hopcount, RIO_DST_OPS_CAR,
				 &rdev->dst_ops);

	if (do_enum) {
		/* Assign component tag to device */
		if (next_comptag >= 0x10000) 
		{
#ifdef DEBUG_SRIO
			sysDebugWriteString("\nRIO: Component Tag Counter Overflow\n");
#endif			
			goto cleanup;
		}
		rio_mport_write_config_32(port, destid, hopcount,
					  RIO_COMPONENT_TAG_CSR, next_comptag);
		rdev->comp_tag = next_comptag++;
	}  else {
		rio_mport_read_config_32(port, destid, hopcount,
					 RIO_COMPONENT_TAG_CSR,
					 &rdev->comp_tag);
	}

	if (rio_device_has_destid(port, rdev->src_ops, rdev->dst_ops)) {
		if (do_enum) {
			rio_set_device_id(port, destid, hopcount, next_destid);
			rdev->destid = next_destid;
			next_destid = rio_destid_alloc(net);
		} else
			rdev->destid = rio_get_device_id(port, destid, hopcount);

		rdev->hopcount = 0xff;
	} else {
		/* Switch device has an associated destID which
		 * will be adjusted later
		 */
		rdev->destid = destid;
		rdev->hopcount = hopcount;
	}

	/* If a PE has both switch and other functions, show it as a switch */
	if (rio_is_switch(rdev)) 
	{
		rswitch = rdev->rswitch;
		rswitch->switchid = rdev->comp_tag & RIO_CTAG_UDEVID;
		rswitch->port_ok = 0;
		
		rswitch->route_table = k_malloc(sizeof(UINT8)*
					RIO_MAX_ROUTE_ENTRIES(port->sys_size));					
		
		if (!rswitch->route_table)
			goto cleanup;
							
		 memset(rswitch->route_table,0,sizeof(UINT8)*
					RIO_MAX_ROUTE_ENTRIES(port->sys_size));
							
		/* Initialize switch route table */
		for (rdid = 0; rdid < RIO_MAX_ROUTE_ENTRIES(port->sys_size);
				rdid++)
		{
			rswitch->route_table[rdid] = RIO_INVALID_ROUTE;
		}
		sprintf(rdev->dev.uc_dev_name,"%02x:s:%04x", rdev->net->id,
			     rswitch->switchid);
			     		
		rio_switch_init(rdev, do_enum);

		if (do_enum && rswitch->clr_table)
			rswitch->clr_table(port, destid, hopcount,
					   RIO_GLOBAL_TABLE);

		list_add_tail(&rswitch->node, &net->switches);

	} 
	else 
	{
		if (do_enum)
		{
			/*Enable Input Output Port (transmitter reviever)*/
			rio_enable_rx_tx_port(port, 0, destid, hopcount, 0);
		}
		 sprintf (rdev->dev.uc_dev_name,"%02x:e:%04x", rdev->net->id,
			     rdev->destid);		
	}

	rdev->dma_mask = DMA_BIT_MASK(32);
	ret = rio_add_device(rdev);
	if (ret)
		goto cleanup;

	return rdev;

cleanup:
	if (rswitch)
	k_free(rswitch);

	k_free(rdev);
	return NULL;
}

/**
 * rio_sport_is_active- Tests if a switch port has an active connection.
 * @port: Master port to send transaction
 * @destid: Associated destination ID for switch
 * @hopcount: Hopcount to reach switch
 * @sport: Switch port number
 *
 * Reads the port error status CSR for a particular switch port to
 * determine if the port has an active link.  Returns
 * %RIO_PORT_N_ERR_STS_PORT_OK if the port is active or %0 if it is
 * inactive.
 */
static int
rio_sport_is_active(struct rio_mport *port, UINT16 destid, UINT8 hopcount, int sport)
{
	UINT32 result = 0;
	UINT32 ext_ftr_ptr;

	ext_ftr_ptr = rio_mport_get_efb(port, 0, destid, hopcount, 0);

	while (ext_ftr_ptr) {
		rio_mport_read_config_32(port, destid, hopcount,
					 ext_ftr_ptr, &result);
		result = RIO_GET_BLOCK_ID(result);
		if ((result == RIO_EFB_SER_EP_FREE_ID) ||
		    (result == RIO_EFB_SER_EP_FREE_ID_V13P) ||
		    (result == RIO_EFB_SER_EP_FREC_ID))
			break;

		ext_ftr_ptr = rio_mport_get_efb(port, 0, destid, hopcount,
						ext_ftr_ptr);
	}

	if (ext_ftr_ptr)
		rio_mport_read_config_32(port, destid, hopcount,
					 ext_ftr_ptr +
					 RIO_PORT_N_ERR_STS_CSR(sport),
					 &result);

	return result & RIO_PORT_N_ERR_STS_PORT_OK;
}

/**
 * rio_lock_device - Acquires host device lock for specified device
 * @port: Master port to send transaction
 * @destid: Destination ID for device/switch
 * @hopcount: Hopcount to reach switch
 * @wait_ms: Max wait time in msec (0 = no timeout)
 *
 * Attepts to acquire host device lock for specified device
 * Returns 0 if device lock acquired or EINVAL if timeout expires.
 */
static int
rio_lock_device(struct rio_mport *port, UINT16 destid, UINT8 hopcount, int wait_ms)
{
	UINT32 result;
	int tcnt = 0;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif
	/* Attempt to acquire device lock */
	rio_mport_write_config_32(port, destid, hopcount,
				  RIO_HOST_DID_LOCK_CSR, port->host_deviceid);
	rio_mport_read_config_32(port, destid, hopcount,
				 RIO_HOST_DID_LOCK_CSR, &result);

	while (result != port->host_deviceid) {
		if (wait_ms != 0 && tcnt == wait_ms) {
#ifdef DEBUG_SRIO
			sprintf(achBuffer,"\nRIO: timeout when locking device %x:%x\n",
				destid, hopcount);
			sysDebugWriteString(achBuffer);	
#endif			
			return -EINVAL;
		}

		/* Delay a bit */
		vDelay(1);
		tcnt++;
		/* Try to acquire device lock again */
		rio_mport_write_config_32(port, destid,
			hopcount,
			RIO_HOST_DID_LOCK_CSR,
			port->host_deviceid);
		rio_mport_read_config_32(port, destid,
			hopcount,
			RIO_HOST_DID_LOCK_CSR, &result);
	}

	return 0;
}

/**
 * rio_unlock_device - Releases host device lock for specified device
 * @port: Master port to send transaction
 * @destid: Destination ID for device/switch
 * @hopcount: Hopcount to reach switch
 *
 * Returns 0 if device lock released or EINVAL if fails.
 */
static int
rio_unlock_device(struct rio_mport *port, UINT16 destid, UINT8 hopcount)
{
	UINT32 result;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif
	/* Release device lock */
	rio_mport_write_config_32(port, destid,
				  hopcount,
				  RIO_HOST_DID_LOCK_CSR,
				  port->host_deviceid);
	rio_mport_read_config_32(port, destid, hopcount,
		RIO_HOST_DID_LOCK_CSR, &result);
	if ((result & 0xffff) != 0xffff) {
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nRIO: badness when releasing device lock %x:%x\n",
			 destid, hopcount);
		sysDebugWriteString(achBuffer);	 	
#endif		
		return -EINVAL;
	}

	return 0;
}

/**
 * rio_route_add_entry- Add a route entry to a switch routing table
 * @rdev: RIO device
 * @table: Routing table ID
 * @route_destid: Destination ID to be routed
 * @route_port: Port number to be routed
 * @lock: lock switch device flag
 *
 * Calls the switch specific add_entry() method to add a route entry
 * on a switch. The route table can be specified using the @table
 * argument if a switch has per port routing tables or the normal
 * use is to specific all tables (or the global table) by passing
 * %RIO_GLOBAL_TABLE in @table. Returns %0 on success or %-EINVAL
 * on failure.
 */
static int
rio_route_add_entry(struct rio_dev *rdev,
		    UINT16 table, UINT16 route_destid, UINT8 route_port, int lock)
{
	int rc;

#ifdef DEBUG_SRIO
    sysDebugWriteString ("In: rio_route_add_entry\n");
#endif    
	if (lock) {
		rc = rio_lock_device(rdev->net->hport, rdev->destid,
				     rdev->hopcount, 1000);
		if (rc)
			return rc;
	}

	rc = rdev->rswitch->add_entry(rdev->net->hport, rdev->destid,
				      rdev->hopcount, table,
				      route_destid, route_port);
	if (lock)
		rio_unlock_device(rdev->net->hport, rdev->destid,
				  rdev->hopcount);

#ifdef DEBUG_SRIO
	sysDebugWriteString ("Out: rio_route_add_entry\n");
#endif	
	return rc;
}

/**
 * rio_route_get_entry- Read a route entry in a switch routing table
 * @rdev: RIO device
 * @table: Routing table ID
 * @route_destid: Destination ID to be routed
 * @route_port: Pointer to read port number into
 * @lock: lock switch device flag
 *
 * Calls the switch specific get_entry() method to read a route entry
 * in a switch. The route table can be specified using the @table
 * argument if a switch has per port routing tables or the normal
 * use is to specific all tables (or the global table) by passing
 * %RIO_GLOBAL_TABLE in @table. Returns %0 on success or %-EINVAL
 * on failure.
 */
static int
rio_route_get_entry(struct rio_dev *rdev, UINT16 table,
		    UINT16 route_destid, UINT8 *route_port, int lock)
{
	int rc;

	if (lock) {
		rc = rio_lock_device(rdev->net->hport, rdev->destid,
				     rdev->hopcount, 1000);
		if (rc)
			return rc;
	}

	rc = rdev->rswitch->get_entry(rdev->net->hport, rdev->destid,
				      rdev->hopcount, table,
				      route_destid, route_port);
	if (lock)
		rio_unlock_device(rdev->net->hport, rdev->destid,
				  rdev->hopcount);

	return rc;
}

/**
 * rio_get_host_deviceid_lock- Reads the Host Device ID Lock CSR on a device
 * @port: Master port to send transaction
 * @hopcount: Number of hops to the device
 *
 * Used during enumeration to read the Host Device ID Lock CSR on a
 * RIO device. Returns the value of the lock register.
 */
static UINT16 rio_get_host_deviceid_lock(struct rio_mport *port, UINT8 hopcount)
{
	UINT32 result;

	rio_mport_read_config_32(port, RIO_ANY_DESTID(port->sys_size), hopcount,
				 RIO_HOST_DID_LOCK_CSR, &result);

	return (UINT16) (result & 0xffff);
}

/**
 * rio_enum_peer- Recursively enumerate a RIO network through a master port
 * @net: RIO network being enumerated
 * @port: Master port to send transactions
 * @hopcount: Number of hops into the network
 * @prev: Previous RIO device connected to the enumerated one
 * @prev_port: Port on previous RIO device
 *
 * Recursively enumerates a RIO network.  Transactions are sent via the
 * master port passed in @port.
 */
static int rio_enum_peer(struct rio_net *net, struct rio_mport *port,
			 UINT8 hopcount, struct rio_dev *prev, int prev_port)
{
	struct rio_dev *rdev;
	UINT32 regval=0;
	int tmp=0;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif

	if (rio_mport_chk_dev_access(port,
			RIO_ANY_DESTID(port->sys_size), hopcount)) {
#ifdef DEBUG_SRIO
		sysDebugWriteString("\nRIO: device access check failed\n");
#endif		
		return -1;
	}

	if (rio_get_host_deviceid_lock(port, hopcount) == port->host_deviceid) {
#ifdef DEBUG_SRIO
		sysDebugWriteString("\nRIO: PE already discovered by this host\n");
#endif		
		/*
		 * Already discovered by this host. Add it as another
		 * link to the existing device.
		 */
		rio_mport_read_config_32(port, RIO_ANY_DESTID(port->sys_size),
				hopcount, RIO_COMPONENT_TAG_CSR, &regval);

		if (regval) {
			rdev = rio_get_comptag((regval & 0xffff), NULL);

			if (rdev && prev && rio_is_switch(prev)) {
#ifdef DEBUG_SRIO
				sprintf(achBuffer,"\nRIO: redundant path to %s\n",
					 rio_name(rdev));
				sysDebugWriteString(achBuffer);	 
#endif				
				prev->rswitch->nextdev[prev_port] = rdev;
			}
		}

		return 0;
	}
	
	/* Attempt to acquire device lock */
	rio_mport_write_config_32(port, RIO_ANY_DESTID(port->sys_size),
				  hopcount,
				  RIO_HOST_DID_LOCK_CSR, port->host_deviceid);
	while ((tmp = rio_get_host_deviceid_lock(port, hopcount))
	       < port->host_deviceid) {
		/* Delay a bit */
		vDelay(1);
		/* Attempt to acquire device lock again */
		rio_mport_write_config_32(port, RIO_ANY_DESTID(port->sys_size),
					  hopcount,
					  RIO_HOST_DID_LOCK_CSR,
					  port->host_deviceid);
	}
	
	if (rio_get_host_deviceid_lock(port, hopcount) > port->host_deviceid) 
	{
#ifdef DEBUG_SRIO
		sysDebugWriteString(
		    "\nRIO: PE locked by a higher priority host...retreating\n");
#endif		
		return -1;
	}

	/* Setup new RIO device */
	rdev = rio_setup_device(net, port, RIO_ANY_DESTID(port->sys_size),
					hopcount, 1);
	if (rdev) {
		/* Add device to the global and bus/net specific list. */
		list_add_tail(&rdev->net_list, &net->devices);
		rdev->prev = prev;
		if (prev && rio_is_switch(prev))
			prev->rswitch->nextdev[prev_port] = rdev;
	} else
		return -1;

	if (rio_is_switch(rdev)) 
	{
		int sw_destid;
		int cur_destid;
		int sw_inport;
		UINT16 destid;
		int port_num;

		sw_inport = RIO_GET_PORT_NUM(rdev->swpinfo);
		if (rio_route_add_entry(rdev, RIO_GLOBAL_TABLE,
				    port->host_deviceid, sw_inport, 0) < 0)
		{
#ifdef DEBUG_SRIO
			sysDebugWriteString("FAILURE: rio_route_add_entry()\n");
#endif			
		}
						
		rdev->rswitch->route_table[port->host_deviceid] = sw_inport;

		destid = rio_destid_first(net);
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nRIO: destid:%d\n",destid);
		sysDebugWriteString(achBuffer);
#endif		
		while (destid != RIO_INVALID_DESTID && destid < next_destid) 
		{
			if (destid != port->host_deviceid) 
			{
#ifdef DEBUG_SRIO
				sysDebugWriteString("Adding Another Route Entry\n");
#endif				
				if (rio_route_add_entry(rdev, RIO_GLOBAL_TABLE,
						    destid, sw_inport, 0) < 0)
				{
#ifdef DEBUG_SRIO
					sysDebugWriteString("FAILURE: rio_route_add_entry\n");
#endif					
				}
								
				rdev->rswitch->route_table[destid] = sw_inport;
			}
			destid = rio_destid_next(net, destid + 1);			
#ifdef DEBUG_SRIO
			sprintf(achBuffer,"\nRIO: next destid:%d\n",destid);
			sysDebugWriteString(achBuffer);
#endif			
		}

#ifdef DEBUG_SRIO
		sprintf(achBuffer,
		    "\n1.RIO: found %s (vid %x did %x) with %d ports\n",
		    rio_name(rdev), rdev->vid, rdev->did,
		    RIO_GET_TOTAL_PORTS(rdev->swpinfo));
		sysDebugWriteString(achBuffer);
#endif		
		    
		sw_destid = next_destid;
		for (port_num = 0;
		     port_num < RIO_GET_TOTAL_PORTS(rdev->swpinfo);
		     port_num++) {
			if (sw_inport == port_num) {
				rio_enable_rx_tx_port(port, 0,
					      RIO_ANY_DESTID(port->sys_size),
					      hopcount, port_num);
				rdev->rswitch->port_ok |= (1 << port_num);
				continue;
			}

			cur_destid = next_destid;

			if (rio_sport_is_active
			    (port, RIO_ANY_DESTID(port->sys_size), hopcount,
			     port_num)) 
			{
#ifdef DEBUG_SRIO
				 sprintf(achBuffer,"\nRIO: scanning device on port %d\n",port_num);
				 sysDebugWriteString(achBuffer);   
#endif				 
				rio_enable_rx_tx_port(port, 0,
					      RIO_ANY_DESTID(port->sys_size),
					      hopcount, port_num);
				rdev->rswitch->port_ok |= (1 << port_num);
				if (rio_route_add_entry(rdev, RIO_GLOBAL_TABLE,
						RIO_ANY_DESTID(port->sys_size),
						port_num, 0) < 0)
				{
#ifdef DEBUG_SRIO
					sysDebugWriteString("FAILURE: rio_route_add_entry()\n");
#endif
			    }			

				if (rio_enum_peer(net, port, hopcount + 1,
						  rdev, port_num) < 0)
					return -1;

				/* Update routing tables */
				destid = rio_destid_next(net, cur_destid + 1);
				if (destid != RIO_INVALID_DESTID) {
					for (destid = cur_destid;
					     destid < next_destid;) {
						if (destid != port->host_deviceid) {
							rio_route_add_entry(rdev,
								    RIO_GLOBAL_TABLE,
								    destid,
								    port_num,
								    0);
							rdev->rswitch->
								route_table[destid] =
								port_num;
						}
						destid = rio_destid_next(net,
								destid + 1);
					}
				}
			} else {
				/* If switch supports Error Management,
				 * set PORT_LOCKOUT bit for unused port
				 */
				if (rdev->em_efptr)
					rio_set_port_lockout(rdev, port_num, 1);

				rdev->rswitch->port_ok &= ~(1 << port_num);
			}
		}

		/* Direct Port-write messages to the enumeratiing host */
		if ((rdev->src_ops & RIO_SRC_OPS_PORT_WRITE) &&
		    (rdev->em_efptr)) {
			rio_write_config_32(rdev,
					rdev->em_efptr + RIO_EM_PW_TGT_DEVID,
					(port->host_deviceid << 16) |
					(port->sys_size << 15));
		}

		rio_init_em(rdev);

		/* Check for empty switch */
		if (next_destid == sw_destid)
			next_destid = rio_destid_alloc(net);

		rdev->destid = sw_destid;
	} 
	else
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\n2.RIO: found %s (vid %x did %x)\n",
		    rio_name(rdev), rdev->vid, rdev->did);
		sysDebugWriteString(achBuffer);    
#endif		
	}

	return 0;
}

/**
 * rio_enum_complete- Tests if enumeration of a network is complete
 * @port: Master port to send transaction
 *
 * Tests the PGCCSR discovered bit for non-zero value (enumeration
 * complete flag). Return %1 if enumeration is complete or %0 if
 * enumeration is incomplete.
 */
static int rio_enum_complete(struct rio_mport *port)
{
	UINT32 regval;

	rio_local_read_config_32(port, port->phys_efptr + RIO_PORT_GEN_CTL_CSR,
				 &regval);
	return (regval & RIO_PORT_GEN_DISCOVERED) ? 1 : 0;
}

/**
 * rio_disc_peer- Recursively discovers a RIO network through a master port
 * @net: RIO network being discovered
 * @port: Master port to send transactions
 * @destid: Current destination ID in network
 * @hopcount: Number of hops into the network
 * @prev: previous rio_dev
 * @prev_port: previous port number
 *
 * Recursively discovers a RIO network.  Transactions are sent via the
 * master port passed in @port.
 */
static int
rio_disc_peer(struct rio_net *net, struct rio_mport *port, UINT16 destid,
	      UINT8 hopcount, struct rio_dev *prev, int prev_port)
{
	UINT8 port_num, route_port;
	struct rio_dev *rdev;
	UINT16 ndestid;

#ifdef DEBUG_SRIO
	char achBuffer[80];
	sysDebugWriteString("Line 1128\n");
#endif	
	/* Setup new RIO device */
	if ((rdev = rio_setup_device(net, port, destid, hopcount, 0))) {
		/* Add device to the global and bus/net specific list. */
		list_add_tail(&rdev->net_list, &net->devices);
		rdev->prev = prev;
		if (prev && rio_is_switch(prev))
			prev->rswitch->nextdev[prev_port] = rdev;
	} else
		return -1;

	if (rio_is_switch(rdev)) {
		/* Associated destid is how we accessed this switch */
		rdev->destid = destid;

#ifdef DEBUG_SRIO
		sprintf(achBuffer,
		    "\n3.RIO: found %s (vid %x did %x) with %d ports\n",
		    rio_name(rdev), rdev->vid, rdev->did,
		    RIO_GET_TOTAL_PORTS(rdev->swpinfo));
		sysDebugWriteString(achBuffer);    
#endif		
		for (port_num = 0;
		     port_num < RIO_GET_TOTAL_PORTS(rdev->swpinfo);
		     port_num++) {
			if (RIO_GET_PORT_NUM(rdev->swpinfo) == port_num)
				continue;

			if (rio_sport_is_active
			    (port, destid, hopcount, port_num)) 
			{
#ifdef DEBUG_SRIO
				sprintf(achBuffer,
				    "\nRIO: scanning device on port %d\n",
				    port_num);
				sysDebugWriteString(achBuffer);    
#endif				

				rio_lock_device(port, destid, hopcount, 1000);

				for (ndestid = 0;
				     ndestid < RIO_ANY_DESTID(port->sys_size);
				     ndestid++) {
					rio_route_get_entry(rdev,
							    RIO_GLOBAL_TABLE,
							    ndestid,
							    &route_port, 0);
					if (route_port == port_num)
						break;
				}

				if (ndestid == RIO_ANY_DESTID(port->sys_size))
					continue;
				rio_unlock_device(port, destid, hopcount);
				if (rio_disc_peer(net, port, ndestid,
					hopcount + 1, rdev, port_num) < 0)
					return -1;
			}
		}
	} 
	else
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\n4.RIO: found %s (vid %x did %x)\n",
		    rio_name(rdev), rdev->vid, rdev->did);
		sysDebugWriteString(achBuffer);    
#endif		
	}
		

	return 0;
}

/**
 * rio_mport_is_active- Tests if master port link is active
 * @port: Master port to test
 *
 * Reads the port error status CSR for the master port to
 * determine if the port has an active link.  Returns
 * %RIO_PORT_N_ERR_STS_PORT_OK if the  master port is active
 * or %0 if it is inactive.
 */
static int rio_mport_is_active(struct rio_mport *port)
{
	UINT32 result = 0;
	UINT32 ext_ftr_ptr;
	int *entry = rio_mport_phys_table;

	do {
		if ((ext_ftr_ptr =
		     rio_mport_get_feature(port, 1, 0, 0, *entry)))
			break;
	} while (*++entry >= 0);

	if (ext_ftr_ptr)
		rio_local_read_config_32(port,
					 ext_ftr_ptr +
					 RIO_PORT_N_ERR_STS_CSR(port->index),
					 &result);

	return result & RIO_PORT_N_ERR_STS_PORT_OK;
}

/**
 * rio_alloc_net- Allocate and configure a new RIO network
 * @port: Master port associated with the RIO network
 * @do_enum: Enumeration/Discovery mode flag
 * @start: logical minimal start id for new net
 *
 * Allocates a RIO network structure, initializes per-network
 * list heads, and adds the associated master port to the
 * network list of associated master ports. Returns a
 * RIO network pointer on success or %NULL on failure.
 */
static struct rio_net *rio_alloc_net(struct rio_mport *port,
					       int do_enum, UINT16 start)
{
	struct rio_net *net = NULL;
	struct rio_net_list *nList=NULL;


	/*net = kzalloc(sizeof(struct rio_net), GFP_KERNEL);*/
	net = (struct rio_net*)k_malloc(sizeof(struct rio_net));
	
	memset(net,0,sizeof(struct rio_net));

	if (net && do_enum) {	
		net->destid_table.table = k_malloc(BITS_TO_LONGS(RIO_MAX_ROUTE_ENTRIES(port->sys_size))*sizeof(long));
	
	if (net->destid_table.table != NULL)
		memset(net->destid_table.table,0,BITS_TO_LONGS(RIO_MAX_ROUTE_ENTRIES(port->sys_size))*sizeof(long));

		if (net->destid_table.table == NULL) 
		{
#ifdef DEBUG_SRIO
			sysDebugWriteString("\nRIO: failed to allocate destID table\n");
#endif			
			k_free(net);
			net = NULL;
		} else {
			net->destid_table.start = start;
			net->destid_table.max =
					RIO_MAX_ROUTE_ENTRIES(port->sys_size);
			//spin_lock_init(&net->destid_table.lock);
		}
	}

	if (net) {
		INIT_LIST_HEAD(&net->node);
		INIT_LIST_HEAD(&net->devices);
		INIT_LIST_HEAD(&net->switches);
		INIT_LIST_HEAD(&net->mports);
		list_add_tail(&port->nnode, &net->mports);
		net->hport = port;
		net->id = port->id;

		//nList = kzalloc( sizeof(struct rio_net_list) , GFP_KERNEL );
		nList = (struct rio_net_list*)k_malloc(sizeof(struct rio_net_list));			
		memset(nList,0,sizeof(struct rio_net_list));
				
		if( nList ){
			nList->net = net;
			list_add_tail( &nList->element, &rio_netd );
		}
		else
		{
			k_free(net);
			net = NULL;			
		}
	}
	return net;
}

/**
 * rio_update_route_tables- Updates route tables in switches
 * @net: RIO network to run update on
 *
 * For each enumerated device, ensure that each switch in a system
 * has correct routing entries. Add routes for devices that where
 * unknown dirung the first enumeration pass through the switch.
 */
static void rio_update_route_tables(struct rio_net *net)
{
	struct rio_dev *rdev, *swrdev;
	struct rio_switch *rswitch;
	UINT8 sport;
	UINT16 destid;

	list_for_each_entry(rdev, &net->devices, net_list) {

		destid = rdev->destid;

		list_for_each_entry(rswitch, &net->switches, node) {

			if (rio_is_switch(rdev)	&& (rdev->rswitch == rswitch))
				continue;

			if (RIO_INVALID_ROUTE == rswitch->route_table[destid]) {
				swrdev = sw_to_rio_dev(rswitch);

				/* Skip if destid ends in empty switch*/
				if (swrdev->destid == destid)
					continue;

				sport = RIO_GET_PORT_NUM(swrdev->swpinfo);

				if (rswitch->add_entry)	{
					rio_route_add_entry(swrdev,
						RIO_GLOBAL_TABLE, destid,
						sport, 0);
					rswitch->route_table[destid] = sport;
				}
			}
		}
	}
}

/**
 * rio_init_em - Initializes RIO Error Management (for switches)
 * @rdev: RIO device
 *
 * For each enumerated switch, call device-specific error management
 * initialization routine (if supplied by the switch driver).
 */
static void rio_init_em(struct rio_dev *rdev)
{
	if (rio_is_switch(rdev) && (rdev->em_efptr) &&
	    (rdev->rswitch->em_init)) 
	{
		rdev->rswitch->em_init(rdev);
	}
}

#if 0 //Date :03/03/2014
/**
 * rio_pw_enable - Enables/disables port-write handling by a master port
 * @port: Master port associated with port-write handling
 * @enable:  1=enable,  0=disable
 */
static void rio_pw_enable(struct rio_mport *port, int enable)
{
	if (port->ops->pwenable)
		port->ops->pwenable(port, enable);
}
#endif

/**
 * rio_enum_mport- Start enumeration through a master port
 * @mport: Master port to send transactions
 *
 * Starts the enumeration process. If somebody has enumerated our
 * master port device, then give up. If not and we have an active
 * link, then start recursive peer enumeration. Returns %0 if
 * enumeration succeeds or %-EBUSY if enumeration fails.
 */
int rio_enum_mport(struct rio_mport *mport)
{
	struct rio_net *net = NULL;
	int rc = 0;
#ifdef DEBUG_SRIO
	char achBuffer[80];

	sprintf(achBuffer,"\nRIO: enumerate master port %d, %s\n", mport->id,
	       mport->name);
	 sysDebugWriteString(achBuffer);      
#endif	 
	/* If somebody else enumerated our master port device, bail. */
	if (rio_enum_host(mport) < 0) 
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nRIO: master port %d device has been enumerated by a remote host\n",
		       mport->id);
		 sysDebugWriteString(achBuffer);      
#endif		 
		rc = -EBUSY;
		goto out;
	}

	/* If master port has an active link, allocate net and enum peers */
	if (rio_mport_is_active(mport)) {
		net = rio_alloc_net(mport, 1, 0);
		if (!net) 
		{
#ifdef DEBUG_SRIO
			sysDebugWriteString("\nRIO: failed to allocate new net\n");
#endif			
			rc = -ENOMEM;
			goto out;
		}

		/* reserve mport destID in new net */
		rio_destid_reserve(net, mport->host_deviceid);

		/* Enable Input Output Port (transmitter reviever) */
		rio_enable_rx_tx_port(mport, 1, 0, 0, 0);

		/* Set component tag for host */
		rio_local_write_config_32(mport, RIO_COMPONENT_TAG_CSR,
					  next_comptag++);

		next_destid = rio_destid_alloc(net);

		if (rio_enum_peer(net, mport, 0, NULL, 0) < 0) 
		{
			/* A higher priority host won enumeration, bail. */
#ifdef DEBUG_SRIO
			sprintf(achBuffer,"\nRIO: master port %d device has lost enumeration to a remote host\n",mport->id);
			sysDebugWriteString(achBuffer);
#endif			
			rio_clear_locks(net);
			rc = -EBUSY;
			goto out;
		}
		/* free the last allocated destID (unused) */
		rio_destid_free(net, next_destid);
		rio_update_route_tables(net);
		rio_clear_locks(net);
		//rio_pw_enable(mport, 1); //Date:03/03/2014
	} 
	else 
	{
#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nRIO: host device %d master port %d link inactive\n",
		       mport->host_deviceid, mport->id);
		sysDebugWriteString(achBuffer);       
#endif		
		rc = -EINVAL;
	}

      out:
	return rc;
}

/**
 * rio_build_route_tables- Generate route tables from switch route entries
 * @net: RIO network to run route tables scan on
 *
 * For each switch device, generate a route table by copying existing
 * route entries from the switch.
 */
static void rio_build_route_tables(struct rio_net *net)
{
	struct rio_switch *rswitch;
	struct rio_dev *rdev;
	int i;
	UINT8 sport;

	list_for_each_entry(rswitch, &net->switches, node) {
		rdev = sw_to_rio_dev(rswitch);

		rio_lock_device(net->hport, rdev->destid,
				rdev->hopcount, 1000);
		for (i = 0;
		     i < RIO_MAX_ROUTE_ENTRIES(net->hport->sys_size);
		     i++) {
			if (rio_route_get_entry(rdev, RIO_GLOBAL_TABLE,
						i, &sport, 0) < 0)
				continue;
			rswitch->route_table[i] = sport;
		}

		rio_unlock_device(net->hport, rdev->destid, rdev->hopcount);
	}
}

/**
 * rio_disc_mport- Start discovery through a master port
 * @mport: Master port to send transactions
 *
 * Starts the discovery process. If we have an active link,
 * then wait for the signal that enumeration is complete.
 * When enumeration completion is signaled, start recursive
 * peer discovery. Returns %0 if discovery succeeds or %-EBUSY
 * on failure.
 */
int rio_disc_mport(struct rio_mport *mport)
{
	struct rio_net *net = NULL;
	unsigned long to_end;

//#ifdef DEBUG_SRIO
	char achBuffer[80];
	UINT32 regval;

	sprintf(achBuffer,"\nRIO: discover master port %d, %s\n", mport->id,
	       mport->name);
	sysDebugWriteString(achBuffer);       
//#endif
	/* If master port has an active link, allocate net and discover peers */
	if (rio_mport_is_active(mport)) 
	{
//#ifdef DEBUG_SRIO
		sysDebugWriteString("\nSRIO: wait for enumeration to complete...\n");
//#endif

		// Warning: can't use xTaskGetTickCount(), until the scheduler is started!!
		// to_end = xTaskGetTickCount() + CONFIG_RAPIDIO_DISC_TIMEOUT;
		to_end = CONFIG_RAPIDIO_DISC_TIMEOUT;
		
		// while (time_before(xTaskGetTickCount(), to_end))
		while (to_end > 0)
		{
			if (rio_enum_complete(mport))
				goto enum_done;
				
			--to_end;
			vDelay(10*1000);  
		}

//#ifdef DEBUG_SRIO
		sprintf(achBuffer,"\nRIO: discovery timeout on mport %d %s (GEN_CTL_CSR: 0x%08x)\n",
			 mport->id, mport->name, 
			 rio_local_read_config_32(mport, mport->phys_efptr + RIO_PORT_GEN_CTL_CSR, &regval));
		sysDebugWriteString(achBuffer);	 
//#endif
		goto bail;
enum_done:
//#ifdef DEBUG_SRIO
		sysDebugWriteString("\nSRIO: ... enumeration done\n");
//#endif

		net = rio_alloc_net(mport, 0, 0);
		if (!net) 
		{
#ifdef DEBUG_SRIO
			sysDebugWriteString("\nRIO: Failed to allocate new net\n");
#endif			
			goto bail;
		}
		
		/* Read DestID assigned by enumerator */
		rio_local_read_config_32(mport, RIO_DID_CSR,
					 (UINT32 *) &mport->host_deviceid);
		mport->host_deviceid = RIO_GET_DID(mport->sys_size,
						   mport->host_deviceid);

		if (rio_disc_peer(net, mport, RIO_ANY_DESTID(mport->sys_size),
					0, NULL, 0) < 0) 
		{
#ifdef DEBUG_SRIO
			sprintf(achBuffer,"\nRIO: master port %d device has failed discovery\n",
			       mport->id);
			sysDebugWriteString(achBuffer);       
#endif			
			goto bail;
		}
		rio_build_route_tables(net);
	}

	return 0;
bail:
	return -EBUSY;
}

/**
 * rio_free_devices- Release all the rio devies
 *
 *
 *
 */
void rio_free_devices( void )
{
	struct rio_net_list *nList,*nListTemp;
	struct rio_dev *rdev,*rdevTemp;
	/*
	 * Delete all the devices added
	 */
	//spin_lock(&rio_global_list_lock);
	list_for_each_entry_safe( rdev, rdevTemp, &rio_devices , global_list )
	{
		list_del( &rdev->global_list);
		if( rio_is_switch( rdev ) && rdev->rswitch->route_table )
		{
			k_free( rdev->rswitch->route_table );
		}
	}
	//spin_unlock(&rio_global_list_lock);

	/*
	 * Delete all the networks
	 */
	list_for_each_entry_safe( nList, nListTemp, &rio_netd, element )
	{
		list_del( &nList->element );
		k_free( nList->net );
		k_free( nList );
	}
}
