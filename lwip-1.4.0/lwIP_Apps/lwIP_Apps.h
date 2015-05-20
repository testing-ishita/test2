/*******************************************************************************
*
* Filename:	 lwIP_Apps.h
*
* Description:	lwIP Applications header file, modified for CCT CUTE.
*
* $Revision: 1.2 $
*
* $Date: 2014-07-24 14:34:27 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/lwIP_Apps/lwIP_Apps.h,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

/*
    FreeRTOS V7.0.1 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/
#ifndef LWIP_APPS_H
#define LWIP_APPS_H

#define LWIP_APPS_MAX_DEVICES		6

typedef struct
{
	UINT16 vid;			/* Vendor ID */
	UINT16 did;			/* Device ID */
	UINT8 irqNo;		/* Device IRQ number */
	UINT8 instance;		/* Device instance */
	
	ip_addr_t ipAddr;	/* IP address */
	ip_addr_t netMask;	/* Network mask */
	ip_addr_t gateway;	/* Default gateway */
	
}ETH_DEV;

typedef struct
{
	int num;			/* Number of Ethernet devices */
	ETH_DEV devices[LWIP_APPS_MAX_DEVICES];	/* Ethernet devices */
}ETH_DEV_PARAMS;


void lwIPAppsInit( void *pvArgument );

/* Functions used to obtain and release exclusive access to the Tx buffer.  The
Get function will block if the Tx buffer is not available - use with care! */
/* signed char *pcLwipAppsBlockingGetTxBuffer( void ); */
/* void vLwipAppsReleaseTxBuffer( void ); */

#endif /* LWIP_APPS_H */

