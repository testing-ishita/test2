/*
 * RapidIO configuration space access support
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

#include <stdtypes.h>
#include <private/debug.h>
#include <srio/rio_std.h>
#include <list.h>

/*
 * These interrupt-safe spinlocks protect all accesses to RIO
 * configuration space and doorbell access.
 */
/*
 *  Wrappers for all RIO configuration access functions.  They just check
 *  alignment, do locking and call the low-level functions pointed to
 *  by rio_mport->ops.
 */

#define RIO_8_BAD 0
#define RIO_16_BAD (offset & 1)
#define RIO_32_BAD (offset & 3)

/**
 * RIO_LOP_READ - Generate rio_local_read_config_* functions
 * @size: Size of configuration space read (8, 16, 32 bits)
 * @type: C type of value argument
 * @len: Length of configuration space read (1, 2, 4 bytes)
 *
 * Generates rio_local_read_config_* functions used to access
 * configuration space registers on the local device.
 */
#define RIO_LOP_READ(size,type,len) \
int __rio_local_read_config_##size \
	(struct rio_mport *mport, UINT32 offset, type *value)		\
{									\
	int res;						\
	UINT32 data = 0;				\
	if (RIO_##size##_BAD) return RIO_BAD_SIZE;			\
	res = mport->ops->lcread(mport, mport->id, offset, len, &data);	\
	*value = (type)data;						\
	return res;							\
}

/**
 * RIO_LOP_WRITE - Generate rio_local_write_config_* functions
 * @size: Size of configuration space write (8, 16, 32 bits)
 * @type: C type of value argument
 * @len: Length of configuration space write (1, 2, 4 bytes)
 *
 * Generates rio_local_write_config_* functions used to access
 * configuration space registers on the local device.
 */
#define RIO_LOP_WRITE(size,type,len) \
int __rio_local_write_config_##size \
	(struct rio_mport *mport, UINT32 offset, type value)		\
{									\
	int res;							\
	if (RIO_##size##_BAD) return RIO_BAD_SIZE;			\
	res = mport->ops->lcwrite(mport, mport->id, offset, len, value);\
	return res;							\
}

RIO_LOP_READ(8, UINT8, 1)
RIO_LOP_READ(16, UINT16, 2)
RIO_LOP_READ(32, UINT32, 4)
RIO_LOP_WRITE(8, UINT8, 1)
RIO_LOP_WRITE(16, UINT16, 2)
RIO_LOP_WRITE(32, UINT32, 4)


/**
 * RIO_OP_READ - Generate rio_mport_read_config_* functions
 * @size: Size of configuration space read (8, 16, 32 bits)
 * @type: C type of value argument
 * @len: Length of configuration space read (1, 2, 4 bytes)
 *
 * Generates rio_mport_read_config_* functions used to access
 * configuration space registers on the local device.
 */
#define RIO_OP_READ(size,type,len) \
int rio_mport_read_config_##size \
	(struct rio_mport *mport, UINT16 destid, UINT8 hopcount, UINT32 offset, type *value)	\
{									\
	int res;							\
	UINT32 data = 0;							\
	if (RIO_##size##_BAD) return RIO_BAD_SIZE;			\
	res = mport->ops->cread(mport, mport->id, destid, hopcount, offset, len, &data); \
	*value = (type)data;						\
	return res;							\
}

/**
 * RIO_OP_WRITE - Generate rio_mport_write_config_* functions
 * @size: Size of configuration space write (8, 16, 32 bits)
 * @type: C type of value argument
 * @len: Length of configuration space write (1, 2, 4 bytes)
 *
 * Generates rio_mport_write_config_* functions used to access
 * configuration space registers on the local device.
 */
#define RIO_OP_WRITE(size,type,len) \
int rio_mport_write_config_##size \
	(struct rio_mport *mport, UINT16 destid, UINT8 hopcount, UINT32 offset, type value)	\
{									\
	int res;							\
	if (RIO_##size##_BAD) return RIO_BAD_SIZE;			\
	res = mport->ops->cwrite(mport, mport->id, destid, hopcount, offset, len, value); \
	return res;							\
}

RIO_OP_READ(8, UINT8, 1)
RIO_OP_READ(16, UINT16, 2)
RIO_OP_READ(32, UINT32, 4)
RIO_OP_WRITE(8, UINT8, 1)
RIO_OP_WRITE(16, UINT16, 2)
RIO_OP_WRITE(32, UINT32, 4)


