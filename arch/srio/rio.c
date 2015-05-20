/*
 * RapidIO interconnect services
 * (RapidIO Interconnect Specification, http://www.rapidio.org)
 *
 * Copyright 2005 MontaVista Software, Inc.
 * Matt Porter <mporter@kernel.crashing.org>
 *
 * Copyright 2009 Integrated Device Technology, Inc.
 * Alex Bounine <alexandre.bounine@idt.com>
 * - Added Port-Write/Error Management initialization and handling
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

//#define DEBUG_SRIO

LIST_HEAD(rio_mports);
static unsigned char next_portid;


/**
 * rio_local_get_device_id - Get the base/extended device id for a port
 * @port: RIO master port from which to get the deviceid
 *
 * Reads the base/extended device id from the local device
 * implementing the master port. Returns the 8/16-bit device
 * id.
 */
UINT16 rio_local_get_device_id(struct rio_mport *port)
{
	UINT32 result;

	rio_local_read_config_32(port, RIO_DID_CSR, &result);

	return (RIO_GET_DID(port->sys_size, result));
}

/**
 * rio_request_inb_pwrite - request inbound port-write message service
 * @rdev: RIO device to which register inbound port-write callback routine
 * @pwcback: Callback routine to execute when port-write is received
 *
 * Binds a port-write callback function to the RapidIO device.
 * Returns 0 if the request has been satisfied.
 */
int rio_request_inb_pwrite(struct rio_dev *rdev,
	int (*pwcback)(struct rio_dev *rdev, union rio_pw_msg *msg, int step))
{
	int rc = 0;

	//spin_lock(&rio_global_list_lock);
	if (rdev->pwcback != NULL)
		rc = -ENOMEM;
	else
		rdev->pwcback = pwcback;

	//spin_unlock(&rio_global_list_lock);
	return rc;
}

/**
 * rio_release_inb_pwrite - release inbound port-write message service
 * @rdev: RIO device which registered for inbound port-write callback
 *
 * Removes callback from the rio_dev structure. Returns 0 if the request
 * has been satisfied.
 */
int rio_release_inb_pwrite(struct rio_dev *rdev)
{
	int rc = -ENOMEM;

	//spin_lock(&rio_global_list_lock);
	if (rdev->pwcback) {
		rdev->pwcback = NULL;
		rc = 0;
	}

	//spin_unlock(&rio_global_list_lock);
	return rc;
}

/**
 * rio_map_inb_region -- Map inbound memory region.
 * @mport: Master port.
 * @local: physical address of memory region to be mapped
 * @rbase: RIO base address assigned to this window
 * @size: Size of the memory region
 * @rflags: Flags for mapping.
 *
 * Return: 0 -- Success.
 *
 * This function will create the mapping from RIO space to local memory.
 */
int rio_map_inb_region(struct rio_mport *mport, dma_addr_t local,
			UINT64 rbase, UINT32 size, UINT32 rflags)
{
	int rc = 0;
	UINT32 dSize = 0;
	
	dSize = size;
	//unsigned long flags;

	if (!mport->ops->map_inb)
		return -1;
	//spin_lock_irqsave(&rio_mmap_lock, flags);
	rc = mport->ops->map_inb(mport, local, rbase, (UINT32)dSize, rflags);
	//spin_unlock_irqrestore(&rio_mmap_lock, flags);
	return rc;
}

/**
 * rio_unmap_inb_region -- Unmap the inbound memory region
 * @mport: Master port
 * @lstart: physical address of memory region to be unmapped
 */
void rio_unmap_inb_region(struct rio_mport *mport, dma_addr_t lstart)
{
	//unsigned long flags;
	if (!mport->ops->unmap_inb)
		return;
	//spin_lock_irqsave(&rio_mmap_lock, flags);
	mport->ops->unmap_inb(mport, lstart);
	//spin_unlock_irqrestore(&rio_mmap_lock, flags);
}

/**
 * rio_map_outb_window -- Map outbound Window.
 * @mport: Master port.
 * @physaddr: physical address of memory region to be mapped
 * @size: Size of the memory region
 * @rflags: Flags for mapping.
 *
 * Return: 0 -- Success.
 *
 * This function will create the mapping from RIO space to local memory.
 */
int rio_map_outb_window( struct rio_mport *mport, UINT8 win, UINT64 physaddr,
	 	 	 	 	 UINT64 size, UINT32 rflags )
{
	int rc = 0;
	//unsigned long flags;

	if (!mport->ops->map_outb)
		return -1;
	//spin_lock_irqsave(&rio_mmap_lock, flags);
	rc = mport->ops->map_outb(mport, win, 0,  physaddr, 0, size, rflags);
	//spin_unlock_irqrestore(&rio_mmap_lock, flags);

	return rc;
}

/**
 * rio_unmap_outb_window -- Unmap the outbound window
 * @mport: Master port
 * @lstart: physical address of memory region to be unmapped
 */
void rio_unmap_outb_window(struct rio_mport *mport, UINT8 win)
{
	//unsigned long flags;
	if (!mport->ops->unmap_inb)
		return;
	//spin_lock_irqsave(&rio_mmap_lock, flags);
	mport->ops->unmap_outb(mport, win );
	//spin_unlock_irqrestore(&rio_mmap_lock, flags);
}

/**
 * rio_config_outb_zone -- Map outbound zone.
 * @mport: Master port.
 * @win: Windows
 * @zone: Zone in the specified window
 * @destid: destination id
 * @rstart: RIO base address assigned to this window
 * @rflags: Flags for mapping.
 *
 * Return: 0 -- Success.
 *
 * This function will create the mapping from RIO space to local memory.
 */
int rio_config_outb_zone(struct rio_mport *mport, UINT8 win, UINT8 zone, UINT16 destid,
							UINT64 rstart , UINT32 flags )
{
	int rc = 0;
	//unsigned long sflags;

	if (!mport->ops->config_outb_zone)
		return -1;
	//spin_lock_irqsave(&rio_mmap_lock, sflags);
	rc = mport->ops->config_outb_zone( mport, win, zone, destid,
			rstart , flags );
	//spin_unlock_irqrestore(&rio_mmap_lock, sflags);
	return rc;
}

/**
 * rio_get_outb_pref_asinfo -- Get outbound prefetchable address space
 * @mport: Master port
 * @addr: physical address
 * @size: size
 */
void rio_get_outb_pref_asinfo( struct rio_mport *mport,UINT64 *addr, UINT64 *size )
{
	//unsigned long flags;
	if (!mport->ops->unmap_inb)
		return;
	//spin_lock_irqsave(&rio_mmap_lock, flags);
	mport->ops->get_outb_pref_asinfo(mport, addr, size );
	//spin_unlock_irqrestore(&rio_mmap_lock, flags);
}

/**
 * rio_get_outb_npref_asinfo -- Get outbound non-prefetchable address space
 * @mport: Master port
 * @addr: physical address
 * @size: size
 */
void rio_get_outb_npref_asinfo( struct rio_mport *mport,UINT64 *addr, UINT64 *size )
{
	//unsigned long flags;
	if (!mport->ops->unmap_inb)
		return;
	//spin_lock_irqsave(&rio_mmap_lock, flags);
	mport->ops->get_outb_npref_asinfo(mport, addr, size );
	//spin_unlock_irqrestore(&rio_mmap_lock, flags);
}

/**
 * rio_mport_get_physefb - Helper function that returns register offset
 *                      for Physical Layer Extended Features Block.
 * @port: Master port to issue transaction
 * @local: Indicate a local master port or remote device access
 * @destid: Destination ID of the device
 * @hopcount: Number of switch hops to the device
 */
UINT32
rio_mport_get_physefb(struct rio_mport *port, int local,
		      UINT16 destid, UINT8 hopcount)
{
	UINT32 ext_ftr_ptr;
	UINT32 ftr_header;

	ext_ftr_ptr = rio_mport_get_efb(port, local, destid, hopcount, 0);

	while (ext_ftr_ptr)  {
		if (local)
			rio_local_read_config_32(port, ext_ftr_ptr,
						 &ftr_header);
		else
			rio_mport_read_config_32(port, destid, hopcount,
						 ext_ftr_ptr, &ftr_header);

		ftr_header = RIO_GET_BLOCK_ID(ftr_header);
		switch (ftr_header) {

		case RIO_EFB_SER_EP_ID_V13P:
		case RIO_EFB_SER_EP_REC_ID_V13P:
		case RIO_EFB_SER_EP_FREE_ID_V13P:
		case RIO_EFB_SER_EP_ID:
		case RIO_EFB_SER_EP_REC_ID:
		case RIO_EFB_SER_EP_FREE_ID:
		case RIO_EFB_SER_EP_FREC_ID:

			return ext_ftr_ptr;

		default:
			break;
		}

		ext_ftr_ptr = rio_mport_get_efb(port, local, destid,
						hopcount, ext_ftr_ptr);
	}

	return ext_ftr_ptr;
}

/**
 * rio_get_comptag - Begin or continue searching for a RIO device by component tag
 * @comp_tag: RIO component tag to match
 * @from: Previous RIO device found in search, or %NULL for new search
 *
 * Iterates through the list of known RIO devices. If a RIO device is
 * found with a matching @comp_tag, a pointer to its device
 * structure is returned. Otherwise, %NULL is returned. A new search
 * is initiated by passing %NULL to the @from argument. Otherwise, if
 * @from is not %NULL, searches continue from next device on the global
 * list.
 */
struct rio_dev *rio_get_comptag(UINT32 comp_tag, struct rio_dev *from)
{
	struct list_head *n;
	struct rio_dev *rdev;

	//spin_lock(&rio_global_list_lock);
	n = from ? from->global_list.next : rio_devices.next;

	while (n && (n != &rio_devices)) {
		rdev = rio_dev_g(n);
		if (rdev->comp_tag == comp_tag)
			goto exit;
		n = n->next;
	}
	rdev = NULL;
exit:
	//spin_unlock(&rio_global_list_lock);
	return rdev;
}

/**
 * rio_set_port_lockout - Sets/clears LOCKOUT bit (RIO EM 1.3) for a switch port.
 * @rdev: Pointer to RIO device control structure
 * @pnum: Switch port number to set LOCKOUT bit
 * @lock: Operation : set (=1) or clear (=0)
 */
int rio_set_port_lockout(struct rio_dev *rdev, UINT32 pnum, int lock)
{
	UINT32 regval;

	rio_read_config_32(rdev,
				 rdev->phys_efptr + RIO_PORT_N_CTL_CSR(pnum),
				 &regval);
	if (lock)
		regval |= RIO_PORT_N_CTL_LOCKOUT;
	else
		regval &= ~RIO_PORT_N_CTL_LOCKOUT;

	rio_write_config_32(rdev,
				  rdev->phys_efptr + RIO_PORT_N_CTL_CSR(pnum),
				  regval);
	return 0;
}

/**
 * rio_chk_dev_route - Validate route to the specified device.
 * @rdev:  RIO device failed to respond
 * @nrdev: Last active device on the route to rdev
 * @npnum: nrdev's port number on the route to rdev
 *
 * Follows a route to the specified RIO device to determine the last available
 * device (and corresponding RIO port) on the route.
 */
static int
rio_chk_dev_route(struct rio_dev *rdev, struct rio_dev **nrdev, int *npnum)
{
	UINT32 result;
	int p_port, rc = -EIO;
	struct rio_dev *prev = NULL;
#ifdef DEBUG_SRIO	
	char achBuffer[80];
#endif	
	

	/* Find switch with failed RIO link */
	while (rdev->prev && (rdev->prev->pef & RIO_PEF_SWITCH)) {
		if (!rio_read_config_32(rdev->prev, RIO_DEV_ID_CAR, &result)) {
			prev = rdev->prev;
			break;
		}
		rdev = rdev->prev;
	}

	if (prev == NULL)
		goto err_out;

	p_port = prev->rswitch->route_table[rdev->destid];

	if (p_port != RIO_INVALID_ROUTE) {
#ifdef DEBUG_SRIO	
		sprintf (achBuffer,"RIO: link failed on [%s]-P%d\n",rio_name(prev), p_port);
		sysDebugWriteString(achBuffer);			 
#endif		
		*nrdev = prev;
		*npnum = p_port;
		rc = 0;
	} else
	{
#ifdef DEBUG_SRIO	
		sprintf (achBuffer,"RIO: failed to trace route to %s\n", rio_name(rdev));
		sysDebugWriteString(achBuffer);	
#endif		
	}
err_out:
	return rc;
}

/**
 * rio_mport_chk_dev_access - Validate access to the specified device.
 * @mport: Master port to send transactions
 * @destid: Device destination ID in network
 * @hopcount: Number of hops into the network
 */
int
rio_mport_chk_dev_access(struct rio_mport *mport, UINT16 destid, UINT8 hopcount)
{
	int i = 0;
	UINT32 tmp;

	while (rio_mport_read_config_32(mport, destid, hopcount,
					RIO_DEV_ID_CAR, &tmp)) {
		i++;
		if (i == RIO_MAX_CHK_RETRY)
		{
#ifdef DEBUG_SRIO	
			sysDebugWriteString("FAILED:rio_mport_chk_dev_access()\n");
#endif			
			return -EIO;
		}
		vDelay(1);
	}

	return 0;
}

/**
 * rio_chk_dev_access - Validate access to the specified device.
 * @rdev: Pointer to RIO device control structure
 */
static int rio_chk_dev_access(struct rio_dev *rdev)
{
	return rio_mport_chk_dev_access(rdev->net->hport,
					rdev->destid, rdev->hopcount);
}

/**
 * rio_get_input_status - Sends a Link-Request/Input-Status control symbol and
 *                        returns link-response (if requested).
 * @rdev: RIO devive to issue Input-status command
 * @pnum: Device port number to issue the command
 * @lnkresp: Response from a link partner
 */
static int
rio_get_input_status(struct rio_dev *rdev, int pnum, UINT32 *lnkresp)
{
	UINT32 regval;
	int checkcount;

	if (lnkresp) {
		/* Read from link maintenance response register
		 * to clear valid bit */
		rio_read_config_32(rdev,
			rdev->phys_efptr + RIO_PORT_N_MNT_RSP_CSR(pnum),
			&regval);
		vDelay(50);
	}

	/* Issue Input-status command */
	rio_write_config_32(rdev,
		rdev->phys_efptr + RIO_PORT_N_MNT_REQ_CSR(pnum),
		RIO_MNT_REQ_CMD_IS);

	/* Exit if the response is not expected */
	if (lnkresp == NULL)
		return 0;

	checkcount = 3;
	while (checkcount--) {
		vDelay(50);
		rio_read_config_32(rdev,
			rdev->phys_efptr + RIO_PORT_N_MNT_RSP_CSR(pnum),
			&regval);
		if (regval & RIO_PORT_N_MNT_RSP_RVAL) {
			*lnkresp = regval;
			return 0;
		}
	}

	return -EIO;
}

/**
 * rio_clr_err_stopped - Clears port Error-stopped states.
 * @rdev: Pointer to RIO device control structure
 * @pnum: Switch port number to clear errors
 * @err_status: port error status (if 0 reads register from device)
 */
static int rio_clr_err_stopped(struct rio_dev *rdev, UINT32 pnum, UINT32 err_status)
{
	struct rio_dev *nextdev = rdev->rswitch->nextdev[pnum];
	UINT32 regval;
	UINT32 far_ackid, near_ackid;

#ifdef DEBUG_SRIO
	UINT32 far_linkstat;
	char achBuffer[80];
#endif

	if (err_status == 0)
		rio_read_config_32(rdev,
			rdev->phys_efptr + RIO_PORT_N_ERR_STS_CSR(pnum),
			&err_status);

	if (err_status & RIO_PORT_N_ERR_STS_PW_OUT_ES) {

#ifdef DEBUG_SRIO	
		sysDebugWriteString("RIO_EM: servicing Output Error-Stopped state\n");
#endif		
		/*
		 * Send a Link-Request/Input-Status control symbol
		 */
		if (rio_get_input_status(rdev, pnum, &regval)) {
#ifdef DEBUG_SRIO	
			sysDebugWriteString("RIO_EM: Input-status response timeout\n");
#endif			
			goto rd_err;
		}

#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_EM: SP%d Input-status response=0x%08x\n",
			 pnum, regval);
		sysDebugWriteString(achBuffer);
#endif		
		far_ackid = (regval & RIO_PORT_N_MNT_RSP_ASTAT) >> 5;
		
#ifdef DEBUG_SRIO
		far_linkstat = regval & RIO_PORT_N_MNT_RSP_LSTAT;
#endif
		rio_read_config_32(rdev,
			rdev->phys_efptr + RIO_PORT_N_ACK_STS_CSR(pnum),
			&regval);
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_EM: SP%d_ACK_STS_CSR=0x%08x\n", pnum, regval);
		sysDebugWriteString(achBuffer);
#endif		
		near_ackid = (regval & RIO_PORT_N_ACK_INBOUND) >> 24;
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_EM: SP%d far_ackID=0x%02x far_linkstat=0x%02x" \
			 " near_ackID=0x%02x\n",
			pnum, far_ackid, far_linkstat, near_ackid);
			sysDebugWriteString(achBuffer);
#endif			

		/*
		 * If required, synchronize ackIDs of near and
		 * far sides.
		 */
		if ((far_ackid != ((regval & RIO_PORT_N_ACK_OUTSTAND) >> 8)) ||
		    (far_ackid != (regval & RIO_PORT_N_ACK_OUTBOUND))) {
			/* Align near outstanding/outbound ackIDs with
			 * far inbound.
			 */
			rio_write_config_32(rdev,
				rdev->phys_efptr + RIO_PORT_N_ACK_STS_CSR(pnum),
				(near_ackid << 24) |
					(far_ackid << 8) | far_ackid);
			/* Align far outstanding/outbound ackIDs with
			 * near inbound.
			 */
			far_ackid++;
			if (nextdev)
				rio_write_config_32(nextdev,
					nextdev->phys_efptr +
					RIO_PORT_N_ACK_STS_CSR(RIO_GET_PORT_NUM(nextdev->swpinfo)),
					(far_ackid << 24) |
					(near_ackid << 8) | near_ackid);
			else
			{
#ifdef DEBUG_SRIO	
				sysDebugWriteString("RIO_EM: Invalid nextdev pointer (NULL)\n");
#endif				
			}
		}
rd_err:
		rio_read_config_32(rdev,
			rdev->phys_efptr + RIO_PORT_N_ERR_STS_CSR(pnum),
			&err_status);
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_EM: SP%d_ERR_STS_CSR=0x%08x\n", pnum, err_status);
		sysDebugWriteString(achBuffer);
#endif		
	}

	if ((err_status & RIO_PORT_N_ERR_STS_PW_INP_ES) && nextdev) {
		rio_get_input_status(nextdev,
				     RIO_GET_PORT_NUM(nextdev->swpinfo), NULL);
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_EM: servicing Input Error-Stopped state\n");
		sysDebugWriteString(achBuffer);		     
#endif		
		vDelay(50);

		rio_read_config_32(rdev,
			rdev->phys_efptr + RIO_PORT_N_ERR_STS_CSR(pnum),
			&err_status);
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_EM: SP%d_ERR_STS_CSR=0x%08x\n", pnum, err_status);
		sysDebugWriteString(achBuffer);
#endif		
	}

	return (err_status & (RIO_PORT_N_ERR_STS_PW_OUT_ES |
			      RIO_PORT_N_ERR_STS_PW_INP_ES)) ? 1 : 0;
}

/**
 * rio_inb_pwrite_handler - process inbound port-write message
 * @pw_msg: pointer to inbound port-write message
 *
 * Processes an inbound port-write message. Returns 0 if the request
 * has been satisfied.
 */
int rio_inb_pwrite_handler(union rio_pw_msg *pw_msg)
{
	struct rio_dev *rdev;
	UINT32 err_status, em_perrdet, em_ltlerrdet;
	int rc, portnum;
#ifdef DEBUG_SRIO
	char achBuffer[80];
#endif

	rdev = rio_get_comptag((pw_msg->em.comptag & RIO_CTAG_UDEVID), NULL);
	if (rdev == NULL) {
#ifdef DEBUG_SRIO	
		/* Device removed or enumeration error */
		sprintf(achBuffer,"RIO: %s No matching device for CTag 0x%08x\n",
			__func__, pw_msg->em.comptag);
		sysDebugWriteString(achBuffer);	
#endif		
		return -EIO;
	}

#ifdef DEBUG_SRIO	
	sprintf(achBuffer,"RIO: Port-Write message from %s\n", rio_name(rdev));
	sysDebugWriteString(achBuffer);
#endif	

#ifdef DEBUG_PW
	{
	UINT32 i;
	for (i = 0; i < RIO_PW_MSG_SIZE/sizeof(UINT32);) {
			sprintf(achBuffer,"0x%02x: %08x %08x %08x %08x\n",
				 i*4, pw_msg->raw[i], pw_msg->raw[i + 1],
				 pw_msg->raw[i + 2], pw_msg->raw[i + 3]);
#ifdef DEBUG_SRIO	
				 sysDebugWriteString(achBuffer);
#endif				 
			i += 4;
	}
	}
#endif

	/* Call an external service function (if such is registered
	 * for this device). This may be the service for endpoints that send
	 * device-specific port-write messages. End-point messages expected
	 * to be handled completely by EP specific device driver.
	 * For switches rc==0 signals that no standard processing required.
	 */
	if (rdev->pwcback != NULL) {
		rc = rdev->pwcback(rdev, pw_msg, 0);
		if (rc == 0)
			return 0;
	}

	portnum = pw_msg->em.is_port & 0xFF;

	/* Check if device and route to it are functional:
	 * Sometimes devices may send PW message(s) just before being
	 * powered down (or link being lost).
	 */
	if (rio_chk_dev_access(rdev)) 
	{
#ifdef DEBUG_SRIO	
		sysDebugWriteString("RIO: device access failed - get link partner\n");
#endif		
		/* Scan route to the device and identify failed link.
		 * This will replace device and port reported in PW message.
		 * PW message should not be used after this point.
		 */
		if (rio_chk_dev_route(rdev, &rdev, &portnum)) 
		{
#ifdef DEBUG_SRIO	
			sprintf(achBuffer,"RIO: Route trace for %s failed\n",
				rio_name(rdev));
			sysDebugWriteString(achBuffer);	
#endif			
			return -EIO;
		}
		pw_msg = NULL;
	}

	/* For End-point devices processing stops here */
	if (!(rdev->pef & RIO_PEF_SWITCH))
		return 0;

	if (rdev->phys_efptr == 0) 
	{
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_PW: Bad switch initialization for %s\n",
			rio_name(rdev));
		sysDebugWriteString(achBuffer);	
#endif		
		return 0;
	}

	/*
	 * Process the port-write notification from switch
	 */
	if (rdev->rswitch->em_handle)
		rdev->rswitch->em_handle(rdev, portnum);

	rio_read_config_32(rdev,
			rdev->phys_efptr + RIO_PORT_N_ERR_STS_CSR(portnum),
			&err_status);
#ifdef DEBUG_SRIO	
	sprintf(achBuffer,"RIO_PW: SP%d_ERR_STS_CSR=0x%08x\n", portnum, err_status);
	sysDebugWriteString(achBuffer);	
#endif	

	if (err_status & RIO_PORT_N_ERR_STS_PORT_OK) {

		if (!(rdev->rswitch->port_ok & (1 << portnum))) 
		{
			rdev->rswitch->port_ok |= (1 << portnum);
			rio_set_port_lockout(rdev, portnum, 0);
			/* Schedule Insertion Service */
#ifdef DEBUG_SRIO	
			sprintf(achBuffer,"RIO_PW: Device Insertion on [%s]-P%d\n",
			       rio_name(rdev), portnum);
			sysDebugWriteString(achBuffer);	       
#endif			
		}

		/* Clear error-stopped states (if reported).
		 * Depending on the link partner state, two attempts
		 * may be needed for successful recovery.
		 */
		if (err_status & (RIO_PORT_N_ERR_STS_PW_OUT_ES |
				  RIO_PORT_N_ERR_STS_PW_INP_ES)) {
			if (rio_clr_err_stopped(rdev, portnum, err_status))
				rio_clr_err_stopped(rdev, portnum, 0);
		}
	}  else { /* if (err_status & RIO_PORT_N_ERR_STS_PORT_UNINIT) */

		if (rdev->rswitch->port_ok & (1 << portnum)) {
			rdev->rswitch->port_ok &= ~(1 << portnum);
			rio_set_port_lockout(rdev, portnum, 1);

			rio_write_config_32(rdev,
				rdev->phys_efptr +
					RIO_PORT_N_ACK_STS_CSR(portnum),
				RIO_PORT_N_ACK_CLEAR);
#ifdef DEBUG_SRIO	
			/* Schedule Extraction Service */
			sprintf(achBuffer,"RIO_PW: Device Extraction on [%s]-P%d\n",
			       rio_name(rdev), portnum);
			sysDebugWriteString(achBuffer);       
#endif			
		}
	}

	rio_read_config_32(rdev,
		rdev->em_efptr + RIO_EM_PN_ERR_DETECT(portnum), &em_perrdet);
	if (em_perrdet) {
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_PW: RIO_EM_P%d_ERR_DETECT=0x%08x\n",
			 portnum, em_perrdet);
		sysDebugWriteString(achBuffer);   	 
#endif		
		/* Clear EM Port N Error Detect CSR */
		rio_write_config_32(rdev,
			rdev->em_efptr + RIO_EM_PN_ERR_DETECT(portnum), 0);
	}

	rio_read_config_32(rdev,
		rdev->em_efptr + RIO_EM_LTL_ERR_DETECT, &em_ltlerrdet);
	if (em_ltlerrdet) {
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"RIO_PW: RIO_EM_LTL_ERR_DETECT=0x%08x\n",
			 em_ltlerrdet);
		sysDebugWriteString(achBuffer); 	 
#endif		
		/* Clear EM L/T Layer Error Detect CSR */
		rio_write_config_32(rdev,
			rdev->em_efptr + RIO_EM_LTL_ERR_DETECT, 0);
	}

	/* Clear remaining error bits and Port-Write Pending bit */
	rio_write_config_32(rdev,
			rdev->phys_efptr + RIO_PORT_N_ERR_STS_CSR(portnum),
			err_status);

	return 0;
}



/**
 * rio_mport_get_efb - get pointer to next extended features block
 * @port: Master port to issue transaction
 * @local: Indicate a local master port or remote device access
 * @destid: Destination ID of the device
 * @hopcount: Number of switch hops to the device
 * @from: Offset of  current Extended Feature block header (if 0 starts
 * from	ExtFeaturePtr)
 */
UINT32
rio_mport_get_efb(struct rio_mport *port, int local, UINT16 destid,
		      UINT8 hopcount, UINT32 from)
{
	UINT32 reg_val;

	if (from == 0) {
		if (local)
			rio_local_read_config_32(port, RIO_ASM_INFO_CAR,
						 &reg_val);
		else
			rio_mport_read_config_32(port, destid, hopcount,
						 RIO_ASM_INFO_CAR, &reg_val);
		return reg_val & RIO_EXT_FTR_PTR_MASK;
	} else {
		if (local)
			rio_local_read_config_32(port, from, &reg_val);
		else
			rio_mport_read_config_32(port, destid, hopcount,
						 from, &reg_val);
		return RIO_GET_BLOCK_ID(reg_val);
	}
}

/**
 * rio_mport_get_feature - query for devices' extended features
 * @port: Master port to issue transaction
 * @local: Indicate a local master port or remote device access
 * @destid: Destination ID of the device
 * @hopcount: Number of switch hops to the device
 * @ftr: Extended feature code
 *
 * Tell if a device supports a given RapidIO capability.
 * Returns the offset of the requested extended feature
 * block within the device's RIO configuration space or
 * 0 in case the device does not support it.  Possible
 * values for @ftr:
 *
 * %RIO_EFB_PAR_EP_ID		LP/LVDS EP Devices
 *
 * %RIO_EFB_PAR_EP_REC_ID	LP/LVDS EP Recovery Devices
 *
 * %RIO_EFB_PAR_EP_FREE_ID	LP/LVDS EP Free Devices
 *
 * %RIO_EFB_SER_EP_ID		LP/Serial EP Devices
 *
 * %RIO_EFB_SER_EP_REC_ID	LP/Serial EP Recovery Devices
 *
 * %RIO_EFB_SER_EP_FREE_ID	LP/Serial EP Free Devices
 */
UINT32
rio_mport_get_feature(struct rio_mport * port, int local, UINT16 destid,
		      UINT8 hopcount, int ftr)
{
	UINT32 asm_info, ext_ftr_ptr, ftr_header;

	if (local)
		rio_local_read_config_32(port, RIO_ASM_INFO_CAR, &asm_info);
	else
		rio_mport_read_config_32(port, destid, hopcount,
					 RIO_ASM_INFO_CAR, &asm_info);

	ext_ftr_ptr = asm_info & RIO_EXT_FTR_PTR_MASK;

	while (ext_ftr_ptr) {
		if (local)
			rio_local_read_config_32(port, ext_ftr_ptr,
						 &ftr_header);
		else
			rio_mport_read_config_32(port, destid, hopcount,
						 ext_ftr_ptr, &ftr_header);
		if (RIO_GET_BLOCK_ID(ftr_header) == ftr)
			return ext_ftr_ptr;
		if (!(ext_ftr_ptr = RIO_GET_BLOCK_PTR(ftr_header)))
			break;
	}

	return 0;
}

/**
 * rio_get_asm - Begin or continue searching for a RIO device by vid/did/asm_vid/asm_did
 * @vid: RIO vid to match or %RIO_ANY_ID to match all vids
 * @did: RIO did to match or %RIO_ANY_ID to match all dids
 * @asm_vid: RIO asm_vid to match or %RIO_ANY_ID to match all asm_vids
 * @asm_did: RIO asm_did to match or %RIO_ANY_ID to match all asm_dids
 * @from: Previous RIO device found in search, or %NULL for new search
 *
 * Iterates through the list of known RIO devices. If a RIO device is
 * found with a matching @vid, @did, @asm_vid, @asm_did, the reference
 * count to the device is incrememted and a pointer to its device
 * structure is returned. Otherwise, %NULL is returned. A new search
 * is initiated by passing %NULL to the @from argument. Otherwise, if
 * @from is not %NULL, searches continue from next device on the global
 * list. The reference count for @from is always decremented if it is
 * not %NULL.
 */
struct rio_dev *rio_get_asm(UINT16 vid, UINT16 did,
			    UINT16 asm_vid, UINT16 asm_did, struct rio_dev *from)
{
	struct list_head *n;
	struct rio_dev *rdev;

	//WARN_ON(in_interrupt());
	//spin_lock(&rio_global_list_lock);
	n = from ? from->global_list.next : rio_devices.next;

	while (n && (n != &rio_devices)) {
		rdev = rio_dev_g(n);
		if ((vid == RIO_ANY_ID || rdev->vid == vid) &&
		    (did == RIO_ANY_ID || rdev->did == did) &&
		    (asm_vid == RIO_ANY_ID || rdev->asm_vid == asm_vid) &&
		    (asm_did == RIO_ANY_ID || rdev->asm_did == asm_did))
			goto exit;
		n = n->next;
	}
	rdev = NULL;
      exit:
	/*rio_dev_put(from);*/
	/*rdev = rio_dev_get(rdev);*/
	//spin_unlock(&rio_global_list_lock);
	return rdev;
}

/**
 * rio_get_device - Begin or continue searching for a RIO device by vid/did
 * @vid: RIO vid to match or %RIO_ANY_ID to match all vids
 * @did: RIO did to match or %RIO_ANY_ID to match all dids
 * @from: Previous RIO device found in search, or %NULL for new search
 *
 * Iterates through the list of known RIO devices. If a RIO device is
 * found with a matching @vid and @did, the reference count to the
 * device is incrememted and a pointer to its device structure is returned.
 * Otherwise, %NULL is returned. A new search is initiated by passing %NULL
 * to the @from argument. Otherwise, if @from is not %NULL, searches
 * continue from next device on the global list. The reference count for
 * @from is always decremented if it is not %NULL.
 */
struct rio_dev *rio_get_device(UINT16 vid, UINT16 did, struct rio_dev *from)
{
	return rio_get_asm(vid, did, RIO_ANY_ID, RIO_ANY_ID, from);
}

/**
 * rio_std_route_add_entry - Add switch route table entry using standard
 *   registers defined in RIO specification rev.1.3
 * @mport: Master port to issue transaction
 * @destid: Destination ID of the device
 * @hopcount: Number of switch hops to the device
 * @table: routing table ID (global or port-specific)
 * @route_destid: destID entry in the RT
 * @route_port: destination port for specified destID
 */
int rio_std_route_add_entry(struct rio_mport *mport, UINT16 destid, UINT8 hopcount,
		       UINT16 table, UINT16 route_destid, UINT8 route_port)
{
#ifdef DEBUG_SRIO	
	char achBuffer[80];
	sysDebugWriteString("In:rio_std_route_add_entry\n");
		
	sprintf (achBuffer,"I.route_destid:%x\n",route_destid);
	sysDebugWriteString(achBuffer);
	
	sprintf (achBuffer,"I.route_port:%x\n",route_port);
	sysDebugWriteString(achBuffer);		
#endif	
	
	if (table == RIO_GLOBAL_TABLE) 
	{
		rio_mport_write_config_32(mport, destid, hopcount,
				RIO_STD_RTE_CONF_DESTID_SEL_CSR,
				(UINT32)route_destid);
		vDelay(10);		
		rio_mport_write_config_32(mport, destid, hopcount,
				RIO_STD_RTE_CONF_PORT_SEL_CSR,
				(UINT32)route_port);
		vDelay(10);		
	}
#ifdef DEBUG_SRIO	
	sysDebugWriteString("Out: rio_std_route_add_entry\n");
#endif	
	
	vDelay(10);
	return 0;
}

/**
 * rio_std_route_get_entry - Read switch route table entry (port number)
 *   associated with specified destID using standard registers defined in RIO
 *   specification rev.1.3
 * @mport: Master port to issue transaction
 * @destid: Destination ID of the device
 * @hopcount: Number of switch hops to the device
 * @table: routing table ID (global or port-specific)
 * @route_destid: destID entry in the RT
 * @route_port: returned destination port for specified destID
 */
int rio_std_route_get_entry(struct rio_mport *mport, UINT16 destid, UINT8 hopcount,
		       UINT16 table, UINT16 route_destid, UINT8 *route_port)
{
	UINT32 result;

	if (table == RIO_GLOBAL_TABLE) {
		rio_mport_write_config_32(mport, destid, hopcount,
				RIO_STD_RTE_CONF_DESTID_SEL_CSR, route_destid);
		rio_mport_read_config_32(mport, destid, hopcount,
				RIO_STD_RTE_CONF_PORT_SEL_CSR, &result);

		*route_port = (UINT8)result;
	}

	return 0;
}

/**
 * rio_std_route_clr_table - Clear swotch route table using standard registers
 *   defined in RIO specification rev.1.3.
 * @mport: Master port to issue transaction
 * @destid: Destination ID of the device
 * @hopcount: Number of switch hops to the device
 * @table: routing table ID (global or port-specific)
 */
int rio_std_route_clr_table(struct rio_mport *mport, UINT16 destid, UINT8 hopcount,
		       UINT16 table)
{
	UINT32 max_destid = 0xff;
	UINT32 i, pef, id_inc = 1, ext_cfg = 0;
	UINT32 port_sel = RIO_INVALID_ROUTE;

	if (table == RIO_GLOBAL_TABLE) {
		rio_mport_read_config_32(mport, destid, hopcount,
					 RIO_PEF_CAR, &pef);

		if (mport->sys_size) {
			rio_mport_read_config_32(mport, destid, hopcount,
						 RIO_SWITCH_RT_LIMIT,
						 &max_destid);
			max_destid &= RIO_RT_MAX_DESTID;
		}

		if (pef & RIO_PEF_EXT_RT) {
			ext_cfg = 0x80000000;
			id_inc = 4;
			port_sel = (RIO_INVALID_ROUTE << 24) |
				   (RIO_INVALID_ROUTE << 16) |
				   (RIO_INVALID_ROUTE << 8) |
				   RIO_INVALID_ROUTE;
		}

		for (i = 0; i <= max_destid;) {
			rio_mport_write_config_32(mport, destid, hopcount,
					RIO_STD_RTE_CONF_DESTID_SEL_CSR,
					ext_cfg | i);
			rio_mport_write_config_32(mport, destid, hopcount,
					RIO_STD_RTE_CONF_PORT_SEL_CSR,
					port_sel);
			i += id_inc;
		}
	}

	vDelay(10);
	return 0;
}

static void rio_fixup_device(struct rio_dev *dev)
{
}

static int rio_init(void)
{
	struct rio_dev *dev = NULL;

	while ((dev = rio_get_device(RIO_ANY_ID, RIO_ANY_ID, dev)) != NULL) {
		rio_fixup_device(dev);
	}
	return 0;
}

/*static struct workqueue_struct *rio_wq;*/

struct rio_disc_work {
	struct rio_mport	*mport;
};

static void disc_work_handler(struct rio_mport *mport)
{
#ifdef DEBUG_SRIO
	char achBuffer[80];

	sprintf(achBuffer,"RIO: discovery work for mport %d %s\n",
		 mport->id, mport->name);
 	sysDebugWriteString(achBuffer); 
#endif	
	rio_disc_mport(mport);
}

int rio_init_mports(void)
{
	struct rio_mport *port;
	int n = 0;
#ifdef DEBUG_SRIO	
	char achBuffer[80];
#endif
	if (!next_portid)
		return -ENODEV;

	
#ifdef DEBUG_SRIO	
	sysDebugWriteString ("In RIO: rio_init_mports()\n");
#endif	
	/*
	 * First, run enumerations and check if we need to perform discovery
	 * on any of the registered mports.
	 */
	list_for_each_entry(port, &rio_mports, node) 
	{
#ifdef DEBUG_SRIO	
		sprintf(achBuffer,"port->host_deviceid:%d\n",port->host_deviceid);
		sysDebugWriteString(achBuffer);
#endif		
		if (port->host_deviceid >= 0)
		{					
#ifdef DEBUG_SRIO	
			sysDebugWriteString("In enumeration port..........\n");
#endif			
			rio_enum_mport(port);
		}
		else
			n++;
	}

	if (!n)
		goto no_disc;

	/*
	 * If we have mports that require discovery schedule a discovery work
	 * for each of them. If the code below fails to allocate needed
	 * resources, exit without error to keep results of enumeration
	 * process (if any).
	 * TODO: Implement restart of dicovery process for all or
	 * individual discovering mports.
	 */
	n = 0;
	list_for_each_entry(port, &rio_mports, node) {
		if (port->host_deviceid < 0) {
			disc_work_handler(port);
			n++;
		}
	}

#ifdef DEBUG_SRIO	
	sysDebugWriteString("RIO: destroy discovery workqueue\n");
#endif

no_disc:
	rio_init();

#ifdef DEBUG_SRIO	
sysDebugWriteString ("Out RIO: rio_init_mports()\n\n");
#endif
	return 0;
}



static int hdids[RIO_MAX_MPORTS + 1];
extern int riohdid0;
extern int riohdid1;

static int rio_get_hdid(int index)
{
	/*We currently support only two ports*/
	if( riohdid0 != -1 && riohdid1 != -1 ) {
		hdids[0] = 2;
		hdids[1] = riohdid0;
		hdids[2] = riohdid1;
	}else if ( riohdid0 != -1 ) {
		hdids[0] = 1;
		hdids[1] = riohdid0;
	}else if ( riohdid1 != -1 ) {
		hdids[0] = 1;
		hdids[1] = riohdid1;
	}

	if (!hdids[0] || hdids[0] <= index || index >= RIO_MAX_MPORTS)
		return -1;

	return hdids[index + 1];
}

int rio_register_mport(struct rio_mport *port)
{
#ifdef DEBUG_SRIO	
	char achBuffer[80];
	sysDebugWriteString("In RIO: rio_register_mport\n");
#endif	
	if (next_portid >= RIO_MAX_MPORTS) 
	{
		sysDebugWriteString("RIO: reached specified max number of mports\n");
		return 1;
	}

	port->id = next_portid++;
	port->host_deviceid = rio_get_hdid(port->id);
	list_add_tail(&port->node, &rio_mports);
#ifdef DEBUG_SRIO	
	sysDebugWriteString("Out RIO: rio_register_mport\n");
	sprintf(achBuffer,"port->host_deviceid:%d\n",port->host_deviceid);
	sysDebugWriteString(achBuffer);
#endif	
	return 0;
}

void rio_unregister_mport(struct rio_mport *port)
{
	list_del ( &port->node );
}


