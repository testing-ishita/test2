/*******************************************************************************
*
* Filename: 	portmacro.h
* 
* Description:	FreeRTOS port macro header, adapted for use with CCT boards.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 07:24:53 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/FreeRTOS/Source/portable/GCC/CUTE/portmacro.h,v $
*
* Copyright 2011-2012 Concurrent Technologies, Plc.
* 
* FreeRTOS V6.0.4 - Copyright (C) 2010 Real Time Engineers Ltd.
*
*******************************************************************************/

/*
    FreeRTOS V6.0.4 - Copyright (C) 2010 Real Time Engineers Ltd.

    ***************************************************************************
    *                                                                         *
    * If you are:                                                             *
    *                                                                         *
    *    + New to FreeRTOS,                                                   *
    *    + Wanting to learn FreeRTOS or multitasking in general quickly       *
    *    + Looking for basic training,                                        *
    *    + Wanting to improve your FreeRTOS skills and productivity           *
    *                                                                         *
    * then take a look at the FreeRTOS eBook                                  *
    *                                                                         *
    *        "Using the FreeRTOS Real Time Kernel - a Practical Guide"        *
    *                  http://www.FreeRTOS.org/Documentation                  *
    *                                                                         *
    * A pdf reference manual is also available.  Both are usually delivered   *
    * to your inbox within 20 minutes to two hours when purchased between 8am *
    * and 8pm GMT (although please allow up to 24 hours in case of            *
    * exceptional circumstances).  Thank you for your support!                *
    *                                                                         *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
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

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>	
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <bit/interrupt.h>
 
/*-----------------------------------------------------------
 * Port specific definitions.  
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		int
#define portSTACK_TYPE	unsigned portLONG
#define portBASE_TYPE	portLONG

typedef unsigned portLONG portTickType;
#define portMAX_DELAY ( portTickType ) 0xffffffff


/*-----------------------------------------------------------*/

/* Critical section definitions. portENTER_CRITICAL must be defined as a
macro for portable.h to work properly. 
*/

#define portENTER_CRITICAL	spinLockTake
#define portEXIT_CRITICAL	spinLockGive
	
#if ( configPORT_INLINE_MACROS == 1 )

#define portDISABLE_INTERRUPTS( lkey ) do \
    { \
    	asm volatile ("pushf; cli; popl %0; " \
		      : "=g" (lkey) : : "memory"); \
    } while ((0))
    
#define portENABLE_INTERRUPTS( lkey ) do \
    { \
    	asm volatile ("testl $0x200,%0; jz 0f; sti; 0: " \
		      : : "g" (lkey) : "cc", "memory"); \
    } while ((0))

#else

#define portDISABLE_INTERRUPTS( lkey ) do \
    { \
    	lkey = sysCpuIntLock(); \
    } while ((0))
    
#define portENABLE_INTERRUPTS( lkey ) do \
    { \
    	sysCpuIntUnlock( lkey ); \
    } while ((0))
    
#endif

/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSWITCH_INT_NUMBER 	0x41	// task yield vector
#define portTICK_INT_NUMBER 	0x42	// timer tick vector
#define portSTART_AP_INT_NUMBER 0x43	// start AP sheduling vector
#define portAP_TICK_INT_NUMBER 	0x44	// AP tick vector

#define portYIELD				vPortYieldCPU
#define portSTACK_GROWTH		( -1 )
#define portTICK_RATE_MS        ( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portINITIAL_SW			( ( portSTACK_TYPE ) 0x00010202 )	/* Start the tasks with interrupts enabled. */
#define portBYTE_ALIGNMENT		( 4 )
#define portCODE_SEG			0x0008
#define portDATA_SEG			0x0010
/*-----------------------------------------------------------*/

/* Compiler specifics. */
#define portINPUT_BYTE( xAddr )				dIoReadReg( xAddr,REG_8)
#define portOUTPUT_BYTE( xAddr, ucValue )	vIoWriteReg(xAddr,REG_8,ucValue)
#define portNOP() asm volatile ( "nop" )
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vTaskFunction, pvParameters ) void vTaskFunction( void *pvParameters )
#define portTASK_FUNCTION( vTaskFunction, pvParameters ) void vTaskFunction( void *pvParameters )

__inline void portSWITCH_CONTEXT( void );
__inline void  portFIRST_CONTEXT( void );

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() sysGetTimestamp();

#ifdef __cplusplus
}
#endif


#endif /* PORTMACRO_H */


