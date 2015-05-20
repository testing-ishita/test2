/*
 * RapidIO driver support
 *
 * Copyright 2005 MontaVista Software, Inc.
 * Matt Porter <mporter@kernel.crashing.org>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <stdio.h>
#include <string.h>

#include <private/debug.h>
#include <srio/rio_std.h>
#include <srio/rio_drv.h>
#include <srio/rio_ids.h>
#include <srio/rio_regs.h>

#include <srio/rio.h>

#define DRV_NAME        "riobus"
#define DRV_VERSION     "0.2"
#define DRV_AUTHOR      "Matt Porter <mporter@kernel.crashing.org>"
#define DRV_DESC        "RapidIO Bus Driver"

int riohdid0=-1; 
int riohdid1=-1;

#if 0
/**
 *  rio_match_device - Tell if a RIO device has a matching RIO device id structure
 *  @id: the RIO device id structure to match against
 *  @rdev: the RIO device structure to match against
 *
 *  Used from driver probe and bus matching to check whether a RIO device
 *  matches a device id structure provided by a RIO driver. Returns the
 *  matching &struct rio_device_id or %NULL if there is no match.
 */
static const struct rio_device_id *rio_match_device(const struct rio_device_id
						    *id,
						    const struct rio_dev *rdev)
{
	while (id->vid || id->asm_vid) {
		if (((id->vid == RIO_ANY_ID) || (id->vid == rdev->vid)) &&
		    ((id->did == RIO_ANY_ID) || (id->did == rdev->did)) &&
		    ((id->asm_vid == RIO_ANY_ID)
		     || (id->asm_vid == rdev->asm_vid))
		    && ((id->asm_did == RIO_ANY_ID)
			|| (id->asm_did == rdev->asm_did)))
			return id;
		id++;
	}
	return NULL;
}
#endif