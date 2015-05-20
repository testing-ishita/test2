/*******************************************************************************
*
* Filename: 	FreeRTOSConfig.h
* 
* Description:	FreeRTOS application configuration header, adapted for use with 
*               CCT boards.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 07:23:01 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/FreeRTOS/Demo/CUTE/FreeRTOSConfig.h,v $
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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H


/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION			1
#define configUSE_IDLE_HOOK				0
#define configUSE_TICK_HOOK				0
#define configTICK_RATE_HZ				( ( portTickType ) 1000 )
#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 512 )   /* in units of portSTACK_TYPE i.e. 512 * 4 = 2048 bytes */
#define configTOTAL_HEAP_SIZE			( ( size_t ) ( 512 * 1024 ) ) /* 64 * 1024 */
#define configMAX_TASK_NAME_LEN			( 16 )
#define configUSE_TRACE_FACILITY    	0
#define configGENERATE_RUN_TIME_STATS	0
#define configUSE_16_BIT_TICKS      	0 /* Set to 0, as 32-bit ticks must be used */ 
#define configIDLE_SHOULD_YIELD			1
#define configUSE_CO_ROUTINES 			0
#define configUSE_MUTEXES				1
#define configUSE_COUNTING_SEMAPHORES	1
#define configUSE_ALTERNATIVE_API		0
#define configUSE_RECURSIVE_MUTEXES		1
#define configCHECK_FOR_STACK_OVERFLOW	0 /* Do not use this option on the PC port. */
#define configUSE_APPLICATION_TASK_TAG	0
#define configQUEUE_REGISTRY_SIZE		0
#define configPORT_INLINE_MACROS 		1 /* Port macros: 1 = use inline code, 0 = function calls */
#define configMAX_CPU_NUM				( (unsigned char ) 4 ) /* Maximum number of CPU's (cores) we support */

#define configMAX_PRIORITIES		( ( unsigned portBASE_TYPE ) 10 )
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet        	1
#define INCLUDE_uxTaskPriorityGet       	1
#define INCLUDE_vTaskDelete             	1
#define INCLUDE_vTaskCleanUpResources   	0
#define INCLUDE_vTaskSuspend            	1
#define INCLUDE_vTaskDelayUntil				1
#define INCLUDE_vTaskDelay					1
#define INCLUDE_uxTaskGetStackHighWaterMark 0 /* Do not use this option on the PC port. */
#define INCLUDE_vTaskList					1
#define INCLUDE_xTaskResumeFromISR			0

#if ( configUSE_APPLICATION_TASK_TAG == 1 )
/* An example "task switched in" hook macro definition. */
#define traceTASK_SWITCHED_IN() xTaskCallApplicationTaskHook( NULL, ( void * ) 0xabcd )
#endif

extern void vMainQueueSendPassed( void );
#define traceQUEUE_SEND( pxQueue ) vMainQueueSendPassed()

#endif /* FREERTOS_CONFIG_H */
