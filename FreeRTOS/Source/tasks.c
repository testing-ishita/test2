/*******************************************************************************
*
* Filename: 	tasks.c
* 
* Description:	FreeRTOS task functions, adapted for use with CCT boards.
*
* $Revision: 1.4 $
*
* $Date: 2015-04-01 16:41:26 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/FreeRTOS/Source/tasks.c,v $
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <private/atomic.h>
#include <private/spinlock.h>
#include <private/debug.h>
#include <private/sys_delay.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#include "StackMacros.h"
#include "config.h"

#include "../debug/dbgLog.h"

#ifdef INCLUDE_TASK_DEBUG
#warning "***** DEBUG ON *****" 
#endif

#ifdef INCLUDE_DEBUG_VGA
#include "../vgalib/vga.h"
extern int startLine;
#endif

extern UINT8 bCpuCount;
extern UINT8 abApicId[];


#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/*
 * Macro to define the amount of stack available to the idle task.
 */
#define tskIDLE_STACK_SIZE	configMINIMAL_STACK_SIZE

typedef union tsUnion
{
    UINT64 i64;		/* 64bit integer */
    UINT32 i32[2];	/* 32bit integer * 2 */
} TS_UNION;

/*
 * Task control block.  A task control block (TCB) is allocated to each task,
 * and stores the context of the task.
 */
typedef struct tskTaskControlBlock
{
	volatile portSTACK_TYPE	*pxTopOfStack;		/*< Points to the location of the last item placed on the tasks stack.  THIS MUST BE THE FIRST MEMBER OF THE STRUCT. */

	#if ( portUSING_MPU_WRAPPERS == 1 )
		xMPU_SETTINGS xMPUSettings;				/*< The MPU settings are defined as part of the port layer.  THIS MUST BE THE SECOND MEMBER OF THE STRUCT. */
	#endif	
	
	xListItem				xGenericListItem;	/*< List item used to place the TCB in ready and blocked queues. */
	xListItem				xEventListItem;		/*< List item used to place the TCB in event lists. */
	unsigned portBASE_TYPE	uxPriority;			/*< The priority of the task where 0 is the lowest priority. */
	portSTACK_TYPE			*pxStack;			/*< Points to the start of the stack. */
	signed char				pcTaskName[ configMAX_TASK_NAME_LEN ];/*< Descriptive name given to the task when created.  Facilitates debugging only. */

	unsigned char uxCPUAffinity;	/* CPU affinity */
	
	short sTaskFirstExecute;		/* Whether has been started */
	
	unsigned portBASE_TYPE	uxTCBNumber;	/* TCB number */


	#if ( portSTACK_GROWTH > 0 )
		portSTACK_TYPE *pxEndOfStack;			/*< Used for stack overflow checking on architectures where the stack grows up from low memory. */
	#endif

	#if ( portCRITICAL_NESTING_IN_TCB == 1 )
		unsigned portBASE_TYPE uxCriticalNesting;
	#endif

	#if ( configUSE_MUTEXES == 1 )
		unsigned portBASE_TYPE uxBasePriority;	/*< The priority last assigned to the task - used by the priority inheritance mechanism. */
	#endif

	#if ( configUSE_APPLICATION_TASK_TAG == 1 )
		pdTASK_HOOK_CODE pxTaskTag;
	#endif

	#if ( configGENERATE_RUN_TIME_STATS == 1 )
		TS_UNION ulRunTimeCounter;		/*< Used for calculating how much CPU time each task is utilising. */
	#endif

} tskTCB;


/*
 * CPU task control.
 */
typedef struct
{	
	xList pxReadyTasksLists[ configMAX_PRIORITIES ];	/*< Prioritised ready tasks. */
	xList xDelayedTaskList1;							/*< Delayed tasks. */
	xList xDelayedTaskList2;							/*< Delayed tasks (two lists are used - one for delays that have overflowed the current tick count. */
	xList * volatile pxDelayedTaskList; 				/*< Points to the delayed task list currently being used. */
	xList * volatile pxOverflowDelayedTaskList;			/*< Points to the delayed task list currently being used to hold tasks that have overflowed the current tick count. */
	xList xPendingReadyList;							/*< Tasks that have been readied while the scheduler was suspended.  They will be moved to the ready queue when the scheduler is resumed. */

#if ( INCLUDE_vTaskDelete == 1 )

	volatile xList xTasksWaitingTermination;		/*< Tasks that have been deleted - but the their memory not yet freed. */
	volatile unsigned portBASE_TYPE uxTasksDeleted;

#endif

#if ( INCLUDE_vTaskSuspend == 1 )

	xList xSuspendedTaskList;						/*< Tasks that are currently suspended. */

#endif

	volatile unsigned portBASE_TYPE uxCurrentNumberOfTasks;
	
	volatile portTickType xTickCount;
	
	unsigned portBASE_TYPE uxTopUsedPriority;
	volatile unsigned portBASE_TYPE uxTopReadyPriority;
	
	volatile UINT32 xSchedulerRunning;
	volatile UINT32 uxSchedulerSuspended;
	volatile UINT32 uxContextSwitchSuspended;
	
	volatile unsigned portBASE_TYPE uxMissedTicks;
	volatile portBASE_TYPE xMissedYield;
	
	volatile portBASE_TYPE xNumOfOverflows;
	unsigned portBASE_TYPE uxTaskNumber;
	
#if ( configGENERATE_RUN_TIME_STATS == 1 )
	TS_UNION ulTaskSwitchedInTime;	/*< Holds the value of a timer/counter the last time a task was switched in. */
#endif

	SPINLOCK sLock;						/* Spinlock for access protection */

} CPUTASKCTRL;

/*
 * Some kernel aware debuggers require data to be viewed to be global, rather
 * than file scope.
 */
#ifdef portREMOVE_STATIC_QUALIFIER
	#define static
#endif

/*lint -e956 */

/* Number of CPU's (cores) we can use */
PRIVILEGED_DATA UINT8 bCpuConfigured = 1;

/* Total number of tasks */
PRIVILEGED_DATA UINT32 uxTotalTaskNumber = 0;

/* Idle task execution counters */
PRIVILEGED_DATA UINT32 idleCount[configMAX_CPU_NUM];

/* Initialization complete */
PRIVILEGED_DATA static short taskInitComplete = pdFALSE;

/* Global pointers to current TCB per CPU */
PRIVILEGED_DATA volatile tskTCB *pxCurrentTCB[configMAX_CPU_NUM];

/* File private variables. --------------------------------*/

/* Task control per CPU */
PRIVILEGED_DATA static CPUTASKCTRL cpuTaskCtrl[configMAX_CPU_NUM];

#if ( INCLUDE_vTaskList == 1 )
	PRIVILEGED_DATA static char pcStatusString[ 50 ];
#endif


/* Debugging and trace facilities private variables and macros. ------------*/

/*
 * The value used to fill the stack of a task when the task is created.  This
 * is used purely for checking the high water mark for tasks.
 */
#define tskSTACK_FILL_BYTE	( 0xa5 )

/*
 * Macros used by vListTask to indicate which state a task is in.
 */
#define tskBLOCKED_CHAR		( ( signed char ) 'B' )
#define tskREADY_CHAR		( ( signed char ) 'R' )
#define tskDELETED_CHAR		( ( signed char ) 'D' )
#define tskSUSPENDED_CHAR	( ( signed char ) 'S' )

/*
 * Macros and private variables used by the trace facility.
 */
#if ( configUSE_TRACE_FACILITY == 1 )

	#define tskSIZE_OF_EACH_TRACE_LINE			( ( unsigned long ) ( sizeof( unsigned long ) + sizeof( unsigned long ) ) )
	PRIVILEGED_DATA static volatile signed char * volatile pcTraceBuffer;
	PRIVILEGED_DATA static signed char *pcTraceBufferStart;
	PRIVILEGED_DATA static signed char *pcTraceBufferEnd;
	PRIVILEGED_DATA static signed portBASE_TYPE xTracing = pdFALSE;
	static unsigned portBASE_TYPE uxPreviousTask[configMAX_CPU_NUM];
#endif


/*-----------------------------------------------------------*/

/*
 * Several functions take an xTaskHandle parameter that can optionally be NULL,
 * where NULL is used to indicate that the handle of the currently executing
 * task should be used in place of the parameter.  This macro simply checks to
 * see if the parameter is NULL and returns a pointer to the appropriate TCB.
 */
#define prvGetTCBFromHandle( pxHandle ) ( ( pxHandle == NULL ) ? ( tskTCB * ) pxCurrentTCB[cpuNo] : ( tskTCB * ) pxHandle )


/*-----------------------------------------------------------*/

/*
 * Macro that writes a trace of scheduler activity to a buffer.  This trace
 * shows which task is running when and is very useful as a debugging tool.
 * As this macro is called each context switch it is a good idea to undefine
 * it if not using the facility.
 */
#if ( configUSE_TRACE_FACILITY == 1 )

	#define vWriteTraceToBuffer()																	\
	{																								\
		if( atomic32Get( (UINT32 *) &xTracing ) == (UINT32) pdTRUE ) )								\
		{																							\
			if( uxPreviousTask[cpuNo] != pxCurrentTCB[cpuNo]->uxTCBNumber )							\
			{																						\
				if( ( pcTraceBuffer + tskSIZE_OF_EACH_TRACE_LINE ) < pcTraceBufferEnd )				\
				{																					\
					uxPreviousTask[cpuNo] = pxCurrentTCB[cpuNo]->uxTCBNumber;						\
					*( unsigned long * ) pcTraceBuffer = ( unsigned long ) atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xTickCount ); \
					pcTraceBuffer += sizeof( unsigned long );										\
					*( unsigned long * ) pcTraceBuffer = ( unsigned long ) uxPreviousTask;			\
					pcTraceBuffer += sizeof( unsigned long );										\
				}																					\
				else																				\
				{																					\
					atomic32Set( (UINT32 *) &xTracing, pdFALSE );									\
				}																					\
			}																						\
		}																							\
	}

#else

	#define vWriteTraceToBuffer()

#endif
/*-----------------------------------------------------------*/

/*
 * Place the task represented by pxTCB into the appropriate ready queue for
 * the task.  It is inserted at the end of the list.  One quirk of this is
 * that if the task being inserted is at the same priority as the currently
 * executing task, then it will only be rescheduled after the currently
 * executing task has been rescheduled.
 */
#define prvAddTaskToReadyQueue( pxTCB, cpuNo )						\
{																	\
																	\
	if( pxTCB->uxPriority > cpuTaskCtrl[cpuNo].uxTopReadyPriority )	\
	{																\
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTopReadyPriority, pxTCB->uxPriority ); \
	}																\
	vListInsertEnd( (xList *) &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ pxTCB->uxPriority ] ), &( pxTCB->xGenericListItem ) );	\
}
/*-----------------------------------------------------------*/

/*
 * Macro that looks at the list of tasks that are currently delayed to see if
 * any require waking.
 *
 * Tasks are stored in the queue in the order of their wake time - meaning
 * once one tasks has been found whose timer has not expired we need not look
 * any further down the list.
 */
#define prvCheckDelayedTasks( cpuNo )																				\
{																													\
	register tskTCB *pxTCB;																								\
																													\
	while( ( pxTCB = ( tskTCB * ) listGET_OWNER_OF_HEAD_ENTRY( cpuTaskCtrl[cpuNo].pxDelayedTaskList ) ) != NULL )						\
	{																												\
		if( cpuTaskCtrl[cpuNo].xTickCount < listGET_LIST_ITEM_VALUE( &( pxTCB->xGenericListItem ) ) )									\
		{																											\
			break;																									\
		}																											\
		vListRemove( &( pxTCB->xGenericListItem ) );																\
		/* Is the task waiting on an event also? */																	\
		if( pxTCB->xEventListItem.pvContainer )																		\
		{																											\
			vListRemove( &( pxTCB->xEventListItem ) );																\
		}																											\
		prvAddTaskToReadyQueue( pxTCB, cpuNo );																		\
	}																												\
}


/* File private functions. --------------------------------*/

/*
 * Utility to ready a TCB for a given task.  Mainly just copies the parameters
 * into the TCB structure.
 */
static void prvInitialiseTCBVariables( tskTCB *pxTCB, const signed char * const pcName, unsigned portBASE_TYPE uxPriority, const xMemoryRegion * const xRegions, unsigned short usStackDepth ) PRIVILEGED_FUNCTION;

/*
 * Utility to ready all the lists used by the scheduler.  This is called
 * automatically upon the creation of the first task.
 */
static void prvInitialiseTaskLists( portSHORT cpuNo ) PRIVILEGED_FUNCTION;

/*
 * The idle task, which as all tasks is implemented as a never ending loop.
 * The idle task is automatically created and added to the ready lists upon
 * creation of the first user task.
 *
 * The portTASK_FUNCTION_PROTO() macro is used to allow port/compiler specific
 * language extensions.  The equivalent prototype for this function is:
 *
 * void prvIdleTask( void *pvParameters );
 *
 */
static void prvIdleTask( void *pvParameters );

/*
 * Utility to free all memory allocated by the scheduler to hold a TCB,
 * including the stack pointed to by the TCB.
 *
 * This does not free memory allocated by the task itself (i.e. memory
 * allocated by calls to pvPortMalloc from within the tasks application code).
 */
#if ( ( INCLUDE_vTaskDelete == 1 ) || ( INCLUDE_vTaskCleanUpResources == 1 ) )

	static void prvDeleteTCB( tskTCB *pxTCB ) PRIVILEGED_FUNCTION;

#endif

/*
 * Used only by the idle task.  This checks to see if anything has been placed
 * in the list of tasks waiting to be deleted.  If so the task is cleaned up
 * and its TCB deleted.
 */
static void prvCheckTasksWaitingTermination( portSHORT cpuNo ) PRIVILEGED_FUNCTION;

/*
 * Allocates memory from the heap for a TCB and associated stack.  Checks the
 * allocation was successful.
 */
static tskTCB *prvAllocateTCBAndStack( unsigned short usStackDepth, portSTACK_TYPE *puxStackBuffer ) PRIVILEGED_FUNCTION;

/*
 * Called from vTaskList.  vListTasks details all the tasks currently under
 * control of the scheduler.  The tasks may be in one of a number of lists.
 * prvListTaskWithinSingleList accepts a list and details the tasks from
 * within just that list.
 *
 */
#if ( INCLUDE_vTaskList == 1 )

	static void prvListTaskWithinSingleList( const signed char *pcWriteBuffer, xList *pxList, signed char cStatus, TS_UNION ulTotalRunTime ) PRIVILEGED_FUNCTION;

#endif

/*
 * When a task is created, the stack of the task is filled with a known value.
 * This function determines the 'high water mark' of the task stack by
 * determining how much of the stack remains at the original preset value.
 */
#if ( ( INCLUDE_vTaskList == 1 ) || ( INCLUDE_uxTaskGetStackHighWaterMark == 1 ) )

	static UINT32 usTaskCheckFreeStackSpace( const unsigned char * pucStackByte ) PRIVILEGED_FUNCTION;

#endif


/*lint +e956 */


/*-----------------------------------------------------------
 * TASK CREATION API documented in task.h
 *----------------------------------------------------------*/

signed portBASE_TYPE xTaskGenericCreate( portSHORT cpuNo, pdTASK_CODE pxTaskCode, const signed char * const pcName, unsigned short usStackDepth, void *pvParameters, unsigned portBASE_TYPE uxPriority, xTaskHandle *pxCreatedTask, portSTACK_TYPE *puxStackBuffer, const xMemoryRegion * const xRegions )
{
	signed portBASE_TYPE xReturn;
	tskTCB * pxNewTCB;


#if defined(INCLUDE_TASK_DEBUG) || defined(INCLUDE_DEBUG_VGA)
	UINT32 taskNum;
	char achBuffer[80];
#endif

	if ( cpuNo >= bCpuConfigured )
	{
		/* default to CPU0 if out of range */
		cpuNo = 0;
	}

#if defined(INCLUDE_TASK_DEBUG) || defined(INCLUDE_DEBUG_VGA)
	taskNum = cpuTaskCtrl[cpuNo].uxTaskNumber + 1;
	sprintf( achBuffer, "CPU:%d Creating task T%02u: %s\n", cpuNo, taskNum, pcName);
#ifdef INCLUDE_TASK_DEBUG
	DBLOG( achBuffer );
#endif
#ifdef INCLUDE_DEBUG_VGA
	/* Just show the intial task created */
	if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xSchedulerRunning ) == (UINT32) pdFALSE )
	{
		vgaPutsXY( 0, startLine++, achBuffer );
	}
#endif
#endif

	/* Allocate the memory required by the TCB and stack for the new task,
	checking that the allocation was successful. */
	pxNewTCB = prvAllocateTCBAndStack( usStackDepth, puxStackBuffer );

	if( pxNewTCB != NULL )
	{
		portSTACK_TYPE *pxTopOfStack;

		#if( portUSING_MPU_WRAPPERS == 1 )
			/* Should the task be created in privileged mode? */
			portBASE_TYPE xRunPrivileged;
			if( ( uxPriority & portPRIVILEGE_BIT ) != 0x00 )
			{
				xRunPrivileged = pdTRUE;
			}
			else
			{
				xRunPrivileged = pdFALSE;
			}
			uxPriority &= ~portPRIVILEGE_BIT;
		#endif /* portUSING_MPU_WRAPPERS == 1 */

		/* Calculate the top of stack address.  This depends on whether the
		stack grows from high memory to low (as per the 80x86) or visa versa.
		portSTACK_GROWTH is used to make the result positive or negative as
		required by the port. */
		#if( portSTACK_GROWTH < 0 )
		{
			pxTopOfStack = pxNewTCB->pxStack + ( usStackDepth - 1 );
			pxTopOfStack = ( portSTACK_TYPE * ) ( ( ( unsigned long ) pxTopOfStack ) & ( ( unsigned long ) ~portBYTE_ALIGNMENT_MASK  ) );

#ifdef INCLUDE_TASK_DEBUG		
			DBLOG( " pxStack     : 0x%x-0x%x (%u * %u = %u bytes)\n", 
					(UINT32) pxNewTCB->pxStack, (UINT32) pxTopOfStack, 
					usStackDepth, sizeof(portSTACK_TYPE), (usStackDepth * sizeof(portSTACK_TYPE)) );
#endif
		}
		#else
		{
			pxTopOfStack = pxNewTCB->pxStack;

			/* If we want to use stack checking on architectures that use
			a positive stack growth direction then we also need to store the
			other extreme of the stack space. */
			pxNewTCB->pxEndOfStack = pxNewTCB->pxStack + ( usStackDepth - 1 );
		}
		#endif

		/* Setup the newly allocated TCB with the initial state of the task. */
		prvInitialiseTCBVariables( pxNewTCB, pcName, uxPriority, xRegions, usStackDepth );

		/* Initialize the TCB stack to look as if the task was already running,
		but had been interrupted by the scheduler.  The return address is set
		to the start of the task function. Once the stack has been initialised
		the	top of stack variable is updated. */
		#if( portUSING_MPU_WRAPPERS == 1 )
		{
			pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack, pxTaskCode, pvParameters, xRunPrivileged );
		}
		#else
		{
			pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack, pxTaskCode, pvParameters );

#ifdef INCLUDE_TASK_DEBUG			
			DBLOG( " pxTopOfStack: 0x%x pxTaskCode: 0x%x\n",
					(UINT32) pxNewTCB->pxTopOfStack, (UINT32) pxTaskCode );
#endif
		}
		#endif

		/* We are going to manipulate the task queues to add this task to a
		ready list, so must make sure no interrupts occur. */
		
		taskENTER_CRITICAL( cpuNo, pdFALSE );
		{		
			atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].uxCurrentNumberOfTasks );
			
			if( cpuTaskCtrl[cpuNo].uxCurrentNumberOfTasks == ( unsigned portBASE_TYPE ) 1 )
			{
				/* As this is the first task it must also be the current task. */
				pxCurrentTCB[cpuNo] = pxNewTCB;
	
				/* This is the first task to be created so do the preliminary
				initialisation required.  We will not recover if this call
				fails, but we will report the failure. */
				prvInitialiseTaskLists( cpuNo );
			}
			else
			{
				/* If the scheduler is not already running, make this task the
				current task if it is the highest priority task to be created
				so far. */
				if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xSchedulerRunning ) == (UINT32) pdFALSE )
				{
					if( pxCurrentTCB[cpuNo]->uxPriority <= uxPriority )
					{
						pxCurrentTCB[cpuNo] = pxNewTCB;
					}
				}
			}
	
			/* Remember the top priority to make context switching faster.  Use
			the priority in pxNewTCB as this has been capped to a valid value. */
			if( pxNewTCB->uxPriority > cpuTaskCtrl[cpuNo].uxTopUsedPriority )
			{
				atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTopUsedPriority, pxNewTCB->uxPriority );
			}
	
			/* Set TCB number. */
			pxNewTCB->uxTCBNumber = cpuTaskCtrl[cpuNo].uxTaskNumber;
			
			atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTaskNumber );
			atomic32Inc( (UINT32 *) &uxTotalTaskNumber );
	
			pxNewTCB->uxCPUAffinity = cpuNo;
			pxNewTCB->sTaskFirstExecute = pdTRUE;
			
			prvAddTaskToReadyQueue( pxNewTCB, cpuNo );
	
#ifdef INCLUDE_TASK_DEBUG			
			DBLOG( " TCB         : 0x%x Priority  : %lu Queue: 0x%x\n",
					(UINT32) pxNewTCB, pxNewTCB->uxPriority, 
					(UINT32) &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ pxNewTCB->uxPriority ] ) );
#endif			
	
			xReturn = pdPASS;
			traceTASK_CREATE( pxNewTCB );
		}
		taskEXIT_CRITICAL( cpuNo, pdFALSE );
	}
	else
	{
#ifdef INCLUDE_TASK_DEBUG		
		DBLOG( " failed to allocate stack depth: %u \n", (UINT32) usStackDepth );
#endif
		xReturn = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
		traceTASK_CREATE_FAILED( pxNewTCB );
	}

	if( xReturn == pdPASS )
	{
		if( ( void * ) pxCreatedTask != NULL )
		{
			/* Pass the TCB out - in an anonymous way.  The calling function/
			task can use this as a handle to delete the task later if
			required.*/
			*pxCreatedTask = ( xTaskHandle ) pxNewTCB;
		}

		if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xSchedulerRunning ) == (UINT32) pdTRUE )
		{
			/* If the created task is of a higher priority than the current task
			then it should run now. */
			if( pxCurrentTCB[cpuNo]->uxPriority < uxPriority )
			{
				portYIELD( cpuNo );
			}
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

#if ( INCLUDE_vTaskDelete == 1 )

	void vTaskDelete( xTaskHandle pxTaskToDelete )
	{
		tskTCB *pxTCB;
		portSHORT cpuNo;


		if (pxTaskToDelete == NULL)
		{
			/* We're deleting ourselves */
			cpuNo = sPortGetCurrentCPU();
		}
		else
		{
			cpuNo = ((tskTCB *) pxTaskToDelete)->uxCPUAffinity;
		}
			
#ifdef INCLUDE_TASK_DEBUG
		DBLOG( "Deleting task: xTaskHandle: 0x%x on CPU%d\n", (UINT32) pxTaskToDelete, cpuNo );
#endif
		taskENTER_CRITICAL( cpuNo, pdFALSE );
		{
			/* Ensure a yield is performed if the current task is being
			deleted. */
			if( pxTaskToDelete == pxCurrentTCB )
			{
				pxTaskToDelete = NULL;
			}
			
			/* If null is passed in then we are deleting ourselves. */
			pxTCB = prvGetTCBFromHandle( pxTaskToDelete );
	
			/* Remove task from the ready list and place in the	termination list.
			This will stop the task from be scheduled.  The idle task will check
			the termination list and free up any memory allocated by the
			scheduler for the TCB and stack. */
			vListRemove( &( pxTCB->xGenericListItem ) );
	
			/* Is the task waiting on an event also? */
			if( pxTCB->xEventListItem.pvContainer )
			{
				vListRemove( &( pxTCB->xEventListItem ) );
			}
	
			vListInsertEnd( ( xList * ) &(cpuTaskCtrl[cpuNo].xTasksWaitingTermination), &( pxTCB->xGenericListItem ) );
	
			/* Increment the ucTasksDeleted variable so the idle task knows
			there is a task that has been deleted and that it should therefore
			check the xTasksWaitingTermination list. */
			atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTasksDeleted );
	
			/* Increment the uxTaskNumberVariable also so kernel aware debuggers
			can detect that the task lists need re-generating. */
			atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTaskNumber );
	
			traceTASK_DELETE( pxTCB );
		}
		taskEXIT_CRITICAL( cpuNo, pdFALSE );

		/* Force a reschedule if we have just deleted the current task. */
		if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xSchedulerRunning ) != (UINT32) pdFALSE )
		{
			if( ( void * ) pxTaskToDelete == NULL )
			{
				portYIELD( cpuNo );
			}
		}
	}

#endif


/*-----------------------------------------------------------
 * TASK CONTROL API documented in task.h
 *----------------------------------------------------------*/

#if ( INCLUDE_vTaskDelayUntil == 1 )

	void vTaskDelayUntil( portTickType * const pxPreviousWakeTime, portTickType xTimeIncrement )
	{
		portTickType xTimeToWake;
		portBASE_TYPE xAlreadyYielded, xShouldDelay = pdFALSE;
		portSHORT cpuNo;


		cpuNo = sPortGetCurrentCPU();

		vTaskSuspendCpu( cpuNo );
		{
			/* Generate the tick time at which the task wants to wake. */
			xTimeToWake = *pxPreviousWakeTime + xTimeIncrement;

			if( cpuTaskCtrl[cpuNo].xTickCount < *pxPreviousWakeTime )
			{
				/* The tick count has overflowed since this function was
				lasted called.  In this case the only time we should ever
				actually delay is if the wake time has also	overflowed,
				and the wake time is greater than the tick time.  When this
				is the case it is as if neither time had overflowed. */
				if( ( xTimeToWake < *pxPreviousWakeTime ) && ( xTimeToWake > cpuTaskCtrl[cpuNo].xTickCount ) )
				{
					xShouldDelay = pdTRUE;
				}
			}
			else
			{
				/* The tick time has not overflowed.  In this case we will
				delay if either the wake time has overflowed, and/or the
				tick time is less than the wake time. */
				if( ( xTimeToWake < *pxPreviousWakeTime ) || ( xTimeToWake > cpuTaskCtrl[cpuNo].xTickCount ) )
				{
					xShouldDelay = pdTRUE;
				}
			}

			/* Update the wake time ready for the next call. */
			*pxPreviousWakeTime = xTimeToWake;

			if( xShouldDelay )
			{
				traceTASK_DELAY_UNTIL();

				/* We must remove ourselves from the ready list before adding
				ourselves to the blocked list as the same list item is used for
				both lists. */
				vListRemove( ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) );

				/* The list item will be inserted in wake time order. */
				listSET_LIST_ITEM_VALUE( &( pxCurrentTCB[cpuNo]->xGenericListItem ), xTimeToWake );

				if( xTimeToWake < cpuTaskCtrl[cpuNo].xTickCount )
				{
					/* Wake time has overflowed.  Place this item in the
					overflow list. */
					vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList, ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) );
				}
				else
				{
					/* The wake time has not overflowed, so we can use the
					current block list. */
					vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxDelayedTaskList, ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) );
				}
			}
		}
		xAlreadyYielded = xTaskResumeCpu( cpuNo );

		/* Force a reschedule if xTaskResumeCpu has not already done so, we may
		have put ourselves to sleep. */
		if( !xAlreadyYielded )
		{
			portYIELD( cpuNo );
		}
	}

#endif
/*-----------------------------------------------------------*/

#if ( INCLUDE_vTaskDelay == 1 )

	void vTaskDelay( portTickType xTicksToDelay )
	{
		portTickType xTimeToWake;
		signed portBASE_TYPE xAlreadyYielded = pdFALSE;
		portSHORT cpuNo;
		tskTCB *pxTCB;
	
		/* A delay time of zero just forces a reschedule. */
		if( xTicksToDelay > ( portTickType ) 0 )
		{
			cpuNo = sPortGetCurrentCPU();
			
			vTaskSuspendCpu( cpuNo );
			{
				traceTASK_DELAY();
				
				pxTCB = (tskTCB *) pxCurrentTCB[cpuNo];
				
				/* A task that is removed from the event list while the
				scheduler is suspended will not get placed in the ready
				list or removed from the blocked list until the scheduler
				is resumed.

				This task cannot be in an event list as it is the currently
				executing task. */

				/* Calculate the time to wake - this may overflow but this is
				not a problem. */
				xTimeToWake = cpuTaskCtrl[cpuNo].xTickCount + xTicksToDelay;

				/* We must remove ourselves from the ready list before adding
				ourselves to the blocked list as the same list item is used for
				both lists. */
				vListRemove( ( xListItem * ) &( pxTCB->xGenericListItem ) );

				/* The list item will be inserted in wake time order. */
				listSET_LIST_ITEM_VALUE( &( pxCurrentTCB[cpuNo]->xGenericListItem ), xTimeToWake );

				if( xTimeToWake < cpuTaskCtrl[cpuNo].xTickCount )
				{
					/* Wake time has overflowed.  Place this item in the
					overflow list. */
					vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList, ( xListItem * ) &( pxTCB->xGenericListItem ) );
				}
				else
				{
					/* The wake time has not overflowed, so we can use the
					current block list. */
					vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxDelayedTaskList, ( xListItem * ) &( pxTCB->xGenericListItem ) );
				}
			}
			xAlreadyYielded = xTaskResumeCpu( cpuNo );
		}

		/* Force a reschedule if xTaskResumeCpu has not already done so, we may
		have put ourselves to sleep. */
		if( !xAlreadyYielded )
		{
			portYIELD( cpuNo );
		}
	}

#endif
/*-----------------------------------------------------------*/

#if ( INCLUDE_uxTaskPriorityGet == 1 )

	unsigned portBASE_TYPE uxTaskPriorityGet( xTaskHandle pxTask )
	{
		tskTCB *pxTCB;
		unsigned portBASE_TYPE uxReturn;
		portSHORT cpuNo;
		

		if (pxTask == NULL)
		{
			/* We're getting the current task priority */
			cpuNo = sPortGetCurrentCPU();
		}
		else
		{
			cpuNo = ((tskTCB *) pxTask)->uxCPUAffinity;
		}
		
		taskENTER_CRITICAL( cpuNo, pdFALSE );
		{
			/* If null is passed in then we are getting the
			priority of the current task. */
			pxTCB = prvGetTCBFromHandle( pxTask );
			uxReturn = pxTCB->uxPriority;
		}
		taskEXIT_CRITICAL( cpuNo, pdFALSE );
		
		return uxReturn;
	}

#endif
/*-----------------------------------------------------------*/

#if ( INCLUDE_vTaskPrioritySet == 1 )

	void vTaskPrioritySet( xTaskHandle pxTask, unsigned portBASE_TYPE uxNewPriority )
	{
		tskTCB *pxTCB;
		unsigned portBASE_TYPE uxCurrentPriority, xYieldRequired;
		portSHORT cpuNo;
		
		
		if (pxTask == NULL)
		{
			/* We're setting ourselves */
			cpuNo = sPortGetCurrentCPU();
		}
		else
		{
			cpuNo = ((tskTCB *) pxTask)->uxCPUAffinity;
		}
		
		/* Ensure the new priority is valid. */
		if( uxNewPriority >= configMAX_PRIORITIES )
		{
			uxNewPriority = configMAX_PRIORITIES - 1;
		}
		
		xYieldRequired = pdFALSE;

		taskENTER_CRITICAL( cpuNo, pdFALSE );
		{
			if( pxTask == pxCurrentTCB )
			{
				pxTask = NULL;
			}
			
			/* If null is passed in here then we are changing the
				priority of the calling task. */
			pxTCB = prvGetTCBFromHandle( pxTask );
					
			traceTASK_PRIORITY_SET( pxTask, uxNewPriority );
	
			#if ( configUSE_MUTEXES == 1 )
			{
				uxCurrentPriority = pxTCB->uxBasePriority;
			}
			#else
			{
				uxCurrentPriority = pxTCB->uxPriority;
			}
			#endif
	
			if( uxCurrentPriority != uxNewPriority )
			{
				/* The priority change may have readied a task of higher
				priority than the current task. */
				if( uxNewPriority > uxCurrentPriority )
				{
					if( pxTask != NULL )
					{
						/* The priority of another task is being raised.  If we
						were raising the priority of the currently running task
						there would be no need to switch as it must have already
						been the highest priority task. */
						xYieldRequired = pdTRUE;
					}
				}
				else if( pxTask == NULL )
				{
					/* Setting our own priority down means there may now be another
					task of higher priority that is ready to execute. */
					xYieldRequired = pdTRUE;
				}
	
				#if ( configUSE_MUTEXES == 1 )
				{
					/* Only change the priority being used if the task is not
					currently using an inherited priority. */
					if( pxTCB->uxBasePriority == pxTCB->uxPriority )
					{
						pxTCB->uxPriority = uxNewPriority;
					}
	
					/* The base priority gets set whatever. */
					pxTCB->uxBasePriority = uxNewPriority;
				}
				#else
				{
					pxTCB->uxPriority = uxNewPriority;
				}
				#endif
	
				listSET_LIST_ITEM_VALUE( &( pxTCB->xEventListItem ), ( configMAX_PRIORITIES - ( portTickType ) uxNewPriority ) );
	
				/* If the task is in the blocked or suspended list we need do
				nothing more than change it's priority variable. However, if
				the task is in a ready list it needs to be removed and placed
				in the queue appropriate to its new priority. */
				if( listIS_CONTAINED_WITHIN( &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ uxCurrentPriority ] ), &( pxTCB->xGenericListItem ) ) )
				{
					/* The task is currently in its ready list - remove before adding
					it to it's new ready list.  As we are in a critical section we
					can do this even if the scheduler is suspended. */
					vListRemove( &( pxTCB->xGenericListItem ) );
					prvAddTaskToReadyQueue( pxTCB, cpuNo );
				}
			}
		}
		taskEXIT_CRITICAL( cpuNo, pdFALSE );
		
		if( xYieldRequired == pdTRUE )
		{
			portYIELD( cpuNo );
		}
	}

#endif
/*-----------------------------------------------------------*/

#if ( INCLUDE_vTaskSuspend == 1 )

	void vTaskSuspend( xTaskHandle pxTaskToSuspend )
	{
		tskTCB *pxTCB;
		portSHORT cpuNo;
#if defined(INCLUDE_TASK_DEBUG) && defined (INCLUDE_DBGLOG)
		UINT32 taskNum;
#endif

		if (pxTaskToSuspend == NULL)
		{
			/* We're suspending ourselves */
			cpuNo = sPortGetCurrentCPU();
		}
		else
		{
			cpuNo = ((tskTCB *) pxTaskToSuspend)->uxCPUAffinity;
		}
		
#if defined(INCLUDE_TASK_DEBUG) && defined (INCLUDE_DBGLOG)
		taskNum = uxTaskNumGet( cpuNo, pxTaskToSuspend );
		DBLOG( "Suspending task T%02u on CPU:%d\n", taskNum, cpuNo );
#endif

		taskENTER_CRITICAL( cpuNo, pdFALSE );
		{
			/* Ensure a yield is performed if the current task is being
			suspended. */
			if( pxTaskToSuspend == pxCurrentTCB[cpuNo] )
			{
				pxTaskToSuspend = NULL;
			}
	
			/* If null is passed in then we are suspending ourselves. */
			pxTCB = prvGetTCBFromHandle( pxTaskToSuspend );
	
			traceTASK_SUSPEND( pxTCB );
	
			/* Remove task from the ready/delayed list and place in the	suspended list. */
			vListRemove( &( pxTCB->xGenericListItem ) );
	
			/* Is the task waiting on an event also? */
			if( pxTCB->xEventListItem.pvContainer )
			{
				vListRemove( &( pxTCB->xEventListItem ) );
			}
	
			vListInsertEnd( ( xList * ) &(cpuTaskCtrl[cpuNo].xSuspendedTaskList), &( pxTCB->xGenericListItem ) );
		}	
		taskEXIT_CRITICAL( cpuNo, pdFALSE );

		/* We may have just suspended the current task. */
		if( ( void * ) pxTaskToSuspend == NULL )
		{
			portYIELD( cpuNo );
		}
	}

#endif
/*-----------------------------------------------------------*/

#if ( INCLUDE_vTaskSuspend == 1 )

	signed portBASE_TYPE xTaskIsTaskSuspended( xTaskHandle xTask )
	{
		portBASE_TYPE xReturn = pdFALSE;
		tskTCB *pxTCB;
		portSHORT cpuNo;

		
		if( xTask != NULL )
		{
			pxTCB = ( tskTCB * ) xTask;
			cpuNo = pxTCB->uxCPUAffinity;
			
			/* Is the task we are attempting to resume actually in the
			suspended list? */
			if( listIS_CONTAINED_WITHIN( &(cpuTaskCtrl[cpuNo].xSuspendedTaskList), &( pxTCB->xGenericListItem ) ) != pdFALSE )
			{
				/* Has the task already been resumed from within an ISR? */
				if( listIS_CONTAINED_WITHIN( &(cpuTaskCtrl[cpuNo].xPendingReadyList), &( pxTCB->xEventListItem ) ) != pdTRUE )
				{
					/* Is it in the suspended list because it is in the
					Suspended state?  It is possible to be in the suspended
					list because it is blocked on a task with no timeout
					specified. */
					if( listIS_CONTAINED_WITHIN( NULL, &( pxTCB->xEventListItem ) ) == pdTRUE )
					{
						xReturn = pdTRUE;
					}
				}
			}
		}
		return xReturn;
	}

#endif
/*-----------------------------------------------------------*/

#if ( INCLUDE_vTaskSuspend == 1 )

	void vTaskResume( xTaskHandle pxTaskToResume )
	{
		tskTCB *pxTCB;
		portSHORT cpuNo;
		unsigned portBASE_TYPE xYieldRequired;
		
		
		/* The parameter cannot be NULL as it is impossible to resume the
			currently executing task. */
		if( pxTaskToResume != NULL )
		{
			xYieldRequired = pdFALSE;
			
			pxTCB = ( tskTCB * ) pxTaskToResume;
			
			cpuNo = pxTCB->uxCPUAffinity;
			
			taskENTER_CRITICAL( cpuNo, pdFALSE );
			{
				/* Cannot be the currently executing task it's impossible to resume the
				currently executing task. */
				if( pxTCB != pxCurrentTCB[cpuNo]  )
				{
					/* Remove the task from whichever list it is currently in, and place
					it in the ready list. */
				
					if( xTaskIsTaskSuspended( pxTCB ) == pdTRUE )
					{
						traceTASK_RESUME( pxTCB );
	
						/* As we are in a critical section we can access the ready
						lists even if the scheduler is suspended. */
						vListRemove(  &( pxTCB->xGenericListItem ) );
						prvAddTaskToReadyQueue( pxTCB, cpuNo );
	
						/* We may have just resumed a higher priority task. */
						if( pxTCB->uxPriority >= pxCurrentTCB[cpuNo]->uxPriority )
						{
							/* This yield may not cause the task just resumed to run, but
							will leave the lists in the correct state for the next yield. */
							xYieldRequired = pdTRUE;
						}
					}
				}
			}
			taskEXIT_CRITICAL( cpuNo, pdFALSE );
			
			if (xYieldRequired == pdTRUE)
			{
				portYIELD( cpuNo );
			}
		}
	}

#endif

/*-----------------------------------------------------------*/

#if ( ( INCLUDE_xTaskResumeFromISR == 1 ) && ( INCLUDE_vTaskSuspend == 1 ) )

	portBASE_TYPE xTaskResumeFromISR( xTaskHandle pxTaskToResume )
	{
		portBASE_TYPE xYieldRequired = pdFALSE;
		tskTCB *pxTCB;
		portSHORT cpuNo;


		if( pxTaskToResume != NULL )
		{
			pxTCB = ( tskTCB * ) pxTaskToResume;
			
			cpuNo = pxTCB->uxCPUAffinity;
			
			taskENTER_CRITICAL( cpuNo, pdFALSE );
			{
				if( xTaskIsTaskSuspended( pxTCB ) == pdTRUE )
				{
					traceTASK_RESUME_FROM_ISR( pxTCB );
		
					if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended ) == (UINT32) pdFALSE )
					{
						xYieldRequired = ( pxTCB->uxPriority >= pxCurrentTCB[cpuNo]->uxPriority );
						vListRemove(  &( pxTCB->xGenericListItem ) );
						prvAddTaskToReadyQueue( pxTCB, cpuNo );
					}
					else
					{
						/* We cannot access the delayed or ready lists, so will hold this
						task pending until the scheduler is resumed, at which point a
						yield will be performed if necessary. */
						vListInsertEnd( ( xList * ) &( cpuTaskCtrl[cpuNo].xPendingReadyList ), &( pxTCB->xEventListItem ) );
					}
				}
			}
			taskEXIT_CRITICAL( cpuNo, pdFALSE );
		}
		
		return xYieldRequired;
	}

#endif


/*-----------------------------------------------------------
 * PUBLIC SCHEDULER CONTROL documented in task.h
 *----------------------------------------------------------*/


void vTaskStartScheduler( void )
{
	UINT8 cpuNo;
	UINT32 key;
	char taskName[8];
	portBASE_TYPE xReturn;

#ifdef INCLUDE_TASK_DEBUG
	char achBuffer[80];
#endif

	for (cpuNo = 0; cpuNo < bCpuConfigured; cpuNo++ )
	{
		sprintf( taskName, "IDLE%u", cpuNo );
		
		/* Add the idle task(s) at the lowest priority. */
		xReturn = xTaskCreate( cpuNo, prvIdleTask, ( signed char * ) taskName, tskIDLE_STACK_SIZE, ( void * ) NULL, ( tskIDLE_PRIORITY | portPRIVILEGE_BIT ), ( xTaskHandle * ) NULL );
		
		if( xReturn != pdPASS )
		{
#ifdef INCLUDE_TASK_DEBUG			
			sprintf( achBuffer, "Failed to create %s task\n", taskName );
			sysDebugWriteString( achBuffer );
#endif
			break;
		}
	}

	if( xReturn == pdPASS )
	{
		/* Interrupts are turned off here, to ensure a tick does not occur
		before or during the call to xPortStartScheduler().  The stacks of
		the created tasks contain a status word with interrupts switched on
		so interrupts will automatically get re-enabled when the first task
		starts to run.

		STEPPING THROUGH HERE USING A DEBUGGER CAN CAUSE BIG PROBLEMS IF THE
		DEBUGGER ALLOWS INTERRUPTS TO BE PROCESSED. */
		portDISABLE_INTERRUPTS( key );
		
		taskInitComplete = pdTRUE;

		/* If configGENERATE_RUN_TIME_STATS is defined then the following
		macro must be defined to configure the timer/counter used to generate
		the run time counter time base. */
		portCONFIGURE_TIMER_FOR_RUN_TIME_STATS();

		/* Setting up the timer tick is hardware specific and thus in the
		portable interface. */
		if( xPortStartScheduler() )
		{
			/* Should not reach here as if the scheduler is running the
			function will not return. */
		}
		else
		{
			/* Should only reach here if a task calls xTaskEndScheduler(). */
		}
		
#ifdef INCLUDE_TASK_DEBUG			
		sprintf( achBuffer, "Warning - Scheduler exited\n" );
		sysDebugWriteString( achBuffer );
#endif
		
		/* Halt */	
		for(;;)
		{
			vDelay( 1000 );
		}
	}
}
/*-----------------------------------------------------------*/

void vTaskEndScheduler( void )
{
	portSHORT cpuNo;
	UINT32 key;

	/* Stop the scheduler */

	for ( cpuNo = 0; cpuNo < bCpuConfigured; cpuNo++ )
	{	
		taskENTER_CRITICAL( cpuNo, pdFALSE );
		
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].uxContextSwitchSuspended, pdTRUE );
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended, pdTRUE );
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].xSchedulerRunning, pdFALSE );
	}

	portDISABLE_INTERRUPTS( key );
	
	vPortEndScheduler();
}
/*----------------------------------------------------------*/

void vTaskSuspendCpu( portSHORT cpuNo )
{
	if (cpuNo < bCpuConfigured)
	{
		atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended );
		taskENTER_CRITICAL( cpuNo, pdFALSE );
	}
}

void vTaskSuspendAll( void )
{
	portSHORT cpuNo;
	
	for ( cpuNo = 0; cpuNo < bCpuConfigured; cpuNo++ )
	{
		vTaskSuspendCpu( cpuNo );
	}
}

/*----------------------------------------------------------*/

signed portBASE_TYPE xTaskResumeCpu( portSHORT cpuNo )
{
	register tskTCB *pxTCB;
	signed portBASE_TYPE xYieldRequired = pdFALSE;


	if (cpuNo < bCpuConfigured)
	{
		/* On entry we must have previously taken the task lock with a call to 
		vTaskSuspendCpu(). */
		
		/* It is possible that an ISR caused a task to be removed from an event
		list while the scheduler was suspended.  If this was the case then the
		removed task will have been added to the xPendingReadyList.  Once the
		scheduler has been resumed it is safe to move all the pending ready
		tasks from this list into their appropriate ready list. */
		
		atomic32Dec( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended );

		if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended ) == (UINT32) pdFALSE )
		{
			if( cpuTaskCtrl[cpuNo].uxCurrentNumberOfTasks > ( unsigned portBASE_TYPE ) 0 )
			{
				/* Move any readied tasks from the pending list into the
				appropriate ready list. */
				while( ( pxTCB = ( tskTCB * ) listGET_OWNER_OF_HEAD_ENTRY(  ( ( xList * ) &(cpuTaskCtrl[cpuNo].xPendingReadyList) ) ) ) != NULL )
				{
					vListRemove( &( pxTCB->xEventListItem ) );
					vListRemove( &( pxTCB->xGenericListItem ) );
					prvAddTaskToReadyQueue( pxTCB, cpuNo );

					/* If we have moved a task that has a priority higher than
					the current task then we should yield. */
					if( pxTCB->uxPriority >= pxCurrentTCB[cpuNo]->uxPriority )
					{
						xYieldRequired = pdTRUE;
					}
				}

				/* If any ticks occurred while the scheduler was suspended then
				they should be processed now.  This ensures the tick count does not
				slip, and that any delayed tasks are resumed at the correct time. */
				if( cpuTaskCtrl[cpuNo].uxMissedTicks > ( unsigned portBASE_TYPE ) 0 )
				{
					while( cpuTaskCtrl[cpuNo].uxMissedTicks > ( unsigned portBASE_TYPE ) 0 )
					{
						vTaskIncrementTick( cpuNo );
						
						atomic32Dec( (UINT32 *) &cpuTaskCtrl[cpuNo].uxMissedTicks );
					}

					/* As we have processed some ticks it is appropriate to yield
					to ensure the highest priority task that is ready to run is
					the task actually running. */
					#if configUSE_PREEMPTION == 1
					{
						xYieldRequired = pdTRUE;
					}
					#endif
				}

				if( cpuTaskCtrl[cpuNo].xMissedYield == pdTRUE )
				{
					xYieldRequired = pdTRUE;
					atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].xMissedYield, pdFALSE );
				}
			}
		}
		
		taskEXIT_CRITICAL( cpuNo, pdFALSE );
		
		if( xYieldRequired == pdTRUE )
		{
			portYIELD( cpuNo );
		}
	}
	
	return xYieldRequired;
}

signed portBASE_TYPE xTaskResumeAll( void )
{
	portSHORT i;
	portSHORT cpuNo;
	signed portBASE_TYPE xAlreadyYielded = 0;
	
	
	/* Resume CPU's in reverse order to suspend */
	cpuNo = bCpuConfigured - 1;
	
	for ( i = 0; i < bCpuConfigured; i++ )
	{
		if ( xTaskResumeCpu( cpuNo ) )
		{
			xAlreadyYielded++;
		}
		
		cpuNo--;
	}
	
	return xAlreadyYielded;
}


/*-----------------------------------------------------------
 * PUBLIC TASK UTILITIES documented in task.h
 *----------------------------------------------------------*/


portTickType xTaskGetTickCount( void )
{
	portTickType xTicks;
	portSHORT cpuNo;
	UINT32 key;


	cpuNo = sPortGetCurrentCPU();

	portDISABLE_INTERRUPTS( key );
	{
		xTicks = atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xTickCount );
	}
	portENABLE_INTERRUPTS( key );

	return xTicks;
}
/*-----------------------------------------------------------*/

unsigned portBASE_TYPE uxTaskGetNumberOfTasks( void )
{
	UINT32 cpuNo;
	
	
	cpuNo = (UINT32) sPortGetCurrentCPU();
	
	return atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].uxCurrentNumberOfTasks );
}
/*-----------------------------------------------------------*/

#if ( INCLUDE_vTaskList == 1 )

	void vTaskList( signed char *pcWriteBuffer )
	{
		unsigned portBASE_TYPE uxQueue;
		TS_UNION ulTotalRunTime;
		portSHORT cpuNo;
	

		/* This is a VERY costly function that should be used for debug only.
		It leaves interrupts disabled for a LONG time. */
		
		ulTotalRunTime.i64 = portGET_RUN_TIME_COUNTER_VALUE();
		
		pcWriteBuffer[ 0 ] = ( signed char ) 0x00;
		
		#if ( configGENERATE_RUN_TIME_STATS == 1 )
		{
#warning "***** RUNTIME STATS ENABLED *****"
			sprintf( ( char * ) pcWriteBuffer, "\r\nTask Name%*sCPU%*sUsage%*sStatus%*sPriority%*sStack%*sTCB\r\n",
					7, "", 2, "", 3, "", 2, "", 3, "", 2, "" );
		}
		#else
		{
			sprintf( ( char * ) pcWriteBuffer, "\r\nTask Name%*sCPU%*sStatus%*sPriority%*sStack%*sTCB\r\n",
					7, "", 2, "", 2, "", 3, "", 2, "" );
		}
		#endif
		
		for (cpuNo = 0; cpuNo < bCpuConfigured; cpuNo++ )
		{
			vTaskSuspendCpu( cpuNo );
			{
				/* Run through all the lists that could potentially contain a TCB and
				report the task name, state and stack high water mark. */
	
				uxQueue = cpuTaskCtrl[cpuNo].uxTopUsedPriority + 1;
	
				do
				{
					uxQueue--;
	
					if( !listLIST_IS_EMPTY( &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ uxQueue ] ) ) )
					{
						prvListTaskWithinSingleList( pcWriteBuffer, ( xList * ) &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ uxQueue ] ), tskREADY_CHAR, ulTotalRunTime );
					}
					
				}while( uxQueue > ( unsigned short ) tskIDLE_PRIORITY );
	
				if( !listLIST_IS_EMPTY( cpuTaskCtrl[cpuNo].pxDelayedTaskList ) )
				{
					prvListTaskWithinSingleList( pcWriteBuffer, ( xList * ) cpuTaskCtrl[cpuNo].pxDelayedTaskList, tskBLOCKED_CHAR, ulTotalRunTime );
				}
	
				if( !listLIST_IS_EMPTY( cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList ) )
				{
					prvListTaskWithinSingleList( pcWriteBuffer, ( xList * ) cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList, tskBLOCKED_CHAR, ulTotalRunTime );
				}
	
				#if( INCLUDE_vTaskDelete == 1 )
				{
					if( !listLIST_IS_EMPTY( &(cpuTaskCtrl[cpuNo].xTasksWaitingTermination) ) )
					{
						prvListTaskWithinSingleList( pcWriteBuffer, ( xList * ) &(cpuTaskCtrl[cpuNo].xTasksWaitingTermination), tskDELETED_CHAR, ulTotalRunTime );
					}
				}
				#endif
	
				#if ( INCLUDE_vTaskSuspend == 1 )
				{
					if( !listLIST_IS_EMPTY( &(cpuTaskCtrl[cpuNo].xSuspendedTaskList) ) )
					{
						prvListTaskWithinSingleList( pcWriteBuffer, ( xList * ) &(cpuTaskCtrl[cpuNo].xSuspendedTaskList), tskSUSPENDED_CHAR, ulTotalRunTime );
					}
				}
				#endif
			}
			xTaskResumeCpu( cpuNo );
		}
	}

#endif

/*----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	void vTaskStartTrace( signed char * pcBuffer, unsigned long ulBufferSize )
	{
		int i;
		
		vTaskSuspendAll();
		{
			pcTraceBuffer = ( signed char * )pcBuffer;
			pcTraceBufferStart = pcBuffer;
			pcTraceBufferEnd = pcBuffer + ( ulBufferSize - tskSIZE_OF_EACH_TRACE_LINE );
			for ( i = 0; i < configMAX_CPU_NUM; i++)
			{
				uxPreviousTask[i] = 255;
			}
			
			atomic32Set( (UINT32 *) &xTracing, pdTRUE );
		}
		vTaskResumeAll();
	}

#endif
/*----------------------------------------------------------*/

#if ( configUSE_TRACE_FACILITY == 1 )

	unsigned long ulTaskEndTrace( void )
	{
		unsigned long ulBufferLength;
	

		atomic32Set( (UINT32 *) &xTracing, pdFALSE );

		ulBufferLength = ( unsigned long ) ( pcTraceBuffer - pcTraceBufferStart );

		return ulBufferLength;
	}

#endif


/*******************************************************************************
*
* vTaskLockTake
*
* Takes the task control spinlock for the given CPU. 
* Assumes CPU number is valid.
* 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vTaskLockTake( portSHORT cpuNo, int fromIsr )
{
	portENTER_CRITICAL( &cpuTaskCtrl[cpuNo].sLock, fromIsr );
}


/*******************************************************************************
*
* vTaskLockGive
*
* Gives the task control spinlock for the given CPU.
* Assumes CPU number is valid.
* 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vTaskLockGive( portSHORT cpuNo, int fromIsr )
{
	portEXIT_CRITICAL( &cpuTaskCtrl[cpuNo].sLock, fromIsr );
}


/*******************************************************************************
*
* vTaskStartSchedulerRunning
*
* Starts scheduler running. Must be called only from xPortStartScheduler() and
* xPortStartAPScheduling().
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vTaskStartSchedulerRunning( portSHORT cpuNo )
{
	if (cpuNo < bCpuConfigured)
	{
		atomic32Dec( (UINT32 *) &cpuTaskCtrl[cpuNo].uxContextSwitchSuspended );
		atomic32Dec( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended );
		atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].xSchedulerRunning );
	}
}


/*******************************************************************************
*
* vTaskSetContextSwitching
*
* Turn context switching on/off. When uxContextSwitchSuspended == pdFALSE then
* context switching is on.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vTaskSetContextSwitching( portSHORT cpuNo, int value )
{	
	if (cpuNo < bCpuConfigured)
	{
		if (value)
		{
			atomic32Dec( (UINT32 *) &cpuTaskCtrl[cpuNo].uxContextSwitchSuspended );
			vPortSetContextSwitching( cpuNo, value );
		}
		else
		{
			atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].uxContextSwitchSuspended );
			vPortSetContextSwitching( cpuNo, value );
		}
	}
}
	

/*******************************************************************************
*
* vTaskGetFirstExecute
*
* Determines if the given task has yet to be executed. 
* 
* 
*
* RETURNS: True or False.
*
*******************************************************************************/
short vTaskGetFirstExecute( volatile void *pxTCB )
{
	if (((volatile tskTCB *)pxTCB)->sTaskFirstExecute == pdTRUE)
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}


/*******************************************************************************
*
* vTaskSetFirstExecute
*
* Sets the given tasks first execute status. 
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vTaskSetFirstExecute( volatile void *pxTCB, short value )
{
	((volatile tskTCB *)pxTCB)->sTaskFirstExecute = value;
}


/*******************************************************************************
*
* uxTaskNumGet
*
* Gets the task number.
* 
* Note: the task numbers start at 1 (uxTCBNumber + 1). 
* 
* 
*
* RETURNS: Task number or 0.
*
*******************************************************************************/
unsigned portBASE_TYPE uxTaskNumGet( portSHORT cpuNo, xTaskHandle pxTask )
{
	tskTCB *pxTCB;
	unsigned portBASE_TYPE uxReturn;
	
	
	taskENTER_CRITICAL( cpuNo, pdFALSE );
	{
		/* If null is passed in here then we are getting the current task. */
		pxTCB = prvGetTCBFromHandle( pxTask );
		uxReturn = pxTCB->uxTCBNumber + 1;
	}
	taskEXIT_CRITICAL( cpuNo, pdFALSE );
	
	return uxReturn;
}


/*-----------------------------------------------------------
 * SCHEDULER INTERNALS AVAILABLE FOR PORTING PURPOSES
 * documented in task.h
 *----------------------------------------------------------*/


void vTaskIncrementTick( portSHORT cpuNo )
{	
	/* Called by the portable layer each time a tick interrupt occurs.
	Increments the tick then checks to see if the new tick value will cause any
	tasks to be unblocked. */
	if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended ) == (UINT32) pdFALSE )
	{
		atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].xTickCount );
		
		if( cpuTaskCtrl[cpuNo].xTickCount == ( portTickType ) 0 )
		{
			xList *pxTemp;

			/* Tick count has overflowed so we need to swap the delay lists.
			If there are any items in pxDelayedTaskList here then there is
			an error! */
			pxTemp = cpuTaskCtrl[cpuNo].pxDelayedTaskList;
			cpuTaskCtrl[cpuNo].pxDelayedTaskList = cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList;
			cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList = pxTemp;
			
			atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].xNumOfOverflows );
		}

		/* See if this tick has made a timeout expire. */
		prvCheckDelayedTasks( cpuNo );
	}
	else
	{
		atomic32Inc( (UINT32 *) &cpuTaskCtrl[cpuNo].uxMissedTicks );

		/* The tick hook gets called at regular intervals, even if the
		scheduler is locked. */
		#if ( configUSE_TICK_HOOK == 1 )
		{
			extern void vApplicationTickHook( void );

			vApplicationTickHook();
		}
		#endif
	}

	#if ( configUSE_TICK_HOOK == 1 )
	{
		extern void vApplicationTickHook( void );

		/* Guard against the tick hook being called when the missed tick
		count is being unwound (when the scheduler is being unlocked. */
		if( cpuTaskCtrl[cpuNo].uxMissedTicks == 0 )
		{
			vApplicationTickHook();
		}
	}
	#endif

	traceTASK_INCREMENT_TICK( cpuTaskCtrl[cpuNo].xTickCount );
}
/*-----------------------------------------------------------*/

#if ( ( INCLUDE_vTaskCleanUpResources == 1 ) && ( INCLUDE_vTaskSuspend == 1 ) )

	void vTaskCleanUpResources( void )
	{
		unsigned short usQueue;
		volatile tskTCB *pxTCB;
		portSHORT cpuNo;

		cpuNo = sPortGetCurrentCPU();
		
		usQueue = ( unsigned short ) cpuTaskCtrl[cpuNo].uxTopUsedPriority + ( unsigned short ) 1;

		/* Remove any TCB's from the ready queues. */
		do
		{
			usQueue--;
			
			while( !listLIST_IS_EMPTY( &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ usQueue ] ) ) )
			{
				listGET_OWNER_OF_NEXT_ENTRY( pxTCB, &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ usQueue ] ) );
				vListRemove( ( xListItem * ) &( pxTCB->xGenericListItem ) );

				prvDeleteTCB( ( tskTCB * ) pxTCB );
			}
			
		}while( usQueue > ( unsigned short ) tskIDLE_PRIORITY );

		/* Remove any TCB's from the delayed queue. */
		while( !listLIST_IS_EMPTY( &(cpuTaskCtrl[cpuNo].xDelayedTaskList1) ) )
		{
			listGET_OWNER_OF_NEXT_ENTRY( pxTCB, &(cpuTaskCtrl[cpuNo].xDelayedTaskList1) );
			vListRemove( ( xListItem * ) &( pxTCB->xGenericListItem ) );

			prvDeleteTCB( ( tskTCB * ) pxTCB );
		}

		/* Remove any TCB's from the overflow delayed queue. */
		while( !listLIST_IS_EMPTY( &(cpuTaskCtrl[cpuNo].xDelayedTaskList2) ) )
		{
			listGET_OWNER_OF_NEXT_ENTRY( pxTCB, &(cpuTaskCtrl[cpuNo].xDelayedTaskList2) );
			vListRemove( ( xListItem * ) &( pxTCB->xGenericListItem ) );

			prvDeleteTCB( ( tskTCB * ) pxTCB );
		}

		while( !listLIST_IS_EMPTY( &(cpuTaskCtrl[cpuNo].xSuspendedTaskList) ) )
		{
			listGET_OWNER_OF_NEXT_ENTRY( pxTCB, &(cpuTaskCtrl[cpuNo].xSuspendedTaskList) );
			vListRemove( ( xListItem * ) &( pxTCB->xGenericListItem ) );

			prvDeleteTCB( ( tskTCB * ) pxTCB );
		}
	}

#endif
/*-----------------------------------------------------------*/

#if ( configUSE_APPLICATION_TASK_TAG == 1 )

	void vTaskSetApplicationTaskTag( xTaskHandle xTask, pdTASK_HOOK_CODE pxTagValue )
	{
		tskTCB *xTCB;
		portSHORT cpuNo;
		

		/* If xTask is NULL then we are setting our own task hook. */
		if( xTask == NULL )
		{
			cpuNo = sPortGetCurrentCPU();
			
			xTCB = ( tskTCB * ) pxCurrentTCB[cpuNo];
		}
		else
		{
			cpuNo = ((tskTCB *) xTask)->uxCPUAffinity;
			
			xTCB = ( tskTCB * ) xTask;
		}

		/* Save the hook function in the TCB.  A critical section is required as
		the value can be accessed from an interrupt. */
		taskENTER_CRITICAL( cpuNo, pdFALSE );
		{
			xTCB->pxTaskTag = pxTagValue;
		}
		taskEXIT_CRITICAL( cpuNo, pdFALSE );
	}

#endif
/*-----------------------------------------------------------*/

#if ( configUSE_APPLICATION_TASK_TAG == 1 )

	pdTASK_HOOK_CODE xTaskGetApplicationTaskTag( xTaskHandle xTask )
	{
		tskTCB *xTCB;
		pdTASK_HOOK_CODE xReturn = 0;
		portSHORT cpuNo;
		

		/* If xTask is NULL then we are setting our own task hook. */
		if( xTask == NULL )
		{
			cpuNo = sPortGetCurrentCPU();
			
			xTCB = ( tskTCB * ) pxCurrentTCB[cpuNo];
		}
		else
		{
			cpuNo = ((tskTCB *) xTask)->uxCPUAffinity;
			
			xTCB = ( tskTCB * ) xTask;
		}

		/* Save the hook function in the TCB.  A critical section is required as
		the value can be accessed from an interrupt. */

		taskENTER_CRITICAL( cpuNo, pdFALSE );
		{
			xReturn = xTCB->pxTaskTag;
		}
		taskEXIT_CRITICAL( cpuNo, pdFALSE );
		
		return xReturn;
	}

#endif
/*-----------------------------------------------------------*/

#if ( configUSE_APPLICATION_TASK_TAG == 1 )

	portBASE_TYPE xTaskCallApplicationTaskHook( xTaskHandle xTask, void *pvParameter )
	{
		tskTCB *xTCB;
		portBASE_TYPE xReturn = pdFAIL;
		portSHORT cpuNo;


		cpuNo = sPortGetCurrentCPU();

		/* If xTask is NULL then we are calling our own task hook. */
		if( xTask == NULL )
		{
			xTCB = ( tskTCB * ) pxCurrentTCB[cpuNo];
		}
		else
		{
			xTCB = ( tskTCB * ) xTask;
		}
		
		if((xTCB != NULL) && (xTCB->uxCPUAffinity == cpuNo))
		{
			if( xTCB->pxTaskTag != NULL )
			{
				xReturn = xTCB->pxTaskTag( pvParameter );
			}
		}

		return xReturn;
	}

#endif
/*-----------------------------------------------------------*/

void vTaskSwitchContext( portSHORT cpuNo )
{

	if( (atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended ) != (UINT32) pdFALSE) ||
		(atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].uxContextSwitchSuspended ) != (UINT32) pdFALSE ) )
	{
		/* The scheduler or context switching is currently suspended - do not allow a context switch. */
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].xMissedYield, pdTRUE );
		return;
	}

	traceTASK_SWITCHED_OUT();

	#if ( configGENERATE_RUN_TIME_STATS == 1 )
	{
		TS_UNION ulTempCounter;

		/* Read time stamp counter */
		ulTempCounter.i64 = portGET_RUN_TIME_COUNTER_VALUE();
		
		/* Add the amount of time the task has been running to the accumulated
		time so far.  The time the task started running was stored in
		ulTaskSwitchedInTime.  Note that there is no overflow protection here
		so count values are only valid until the timer overflows. 
		The RDTSC instruction is used to read the time stamp counter.
		Intel guarantees that the time-stamp counter frequency and configuration will
		be such that it will not wraparound within 10 years after being reset to 0.
		*/
		pxCurrentTCB[cpuNo]->ulRunTimeCounter.i64 += ( ulTempCounter.i64 - cpuTaskCtrl[cpuNo].ulTaskSwitchedInTime.i64 );
		cpuTaskCtrl[cpuNo].ulTaskSwitchedInTime.i64 = ulTempCounter.i64;
	}
	#endif

	taskFIRST_CHECK_FOR_STACK_OVERFLOW();
	taskSECOND_CHECK_FOR_STACK_OVERFLOW();

	/* Find the highest priority queue that contains ready tasks. */
	
	while( listLIST_IS_EMPTY( &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ cpuTaskCtrl[cpuNo].uxTopReadyPriority ] ) ) )
	{
		atomic32Dec( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTopReadyPriority );
	}
	
	/* listGET_OWNER_OF_NEXT_ENTRY walks through the list, so the tasks of the
	same priority get an equal share of the processor time. */
	listGET_OWNER_OF_NEXT_ENTRY( pxCurrentTCB[cpuNo], &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ cpuTaskCtrl[cpuNo].uxTopReadyPriority ] ) );


	traceTASK_SWITCHED_IN();
	vWriteTraceToBuffer();
}
/*-----------------------------------------------------------*/

void vTaskPlaceOnEventList( portSHORT cpuNo, const xList * const pxEventList, portTickType xTicksToWait )
{
	portTickType xTimeToWake;
	volatile tskTCB *pxTCB;
	
	
	/* THIS FUNCTION MUST BE CALLED WITH INTERRUPTS DISABLED OR THE
	SCHEDULER SUSPENDED. */
	
	taskENTER_CRITICAL( cpuNo, pdFALSE );
	{
		pxTCB = pxCurrentTCB[cpuNo];
/*
		DBLOG( "%s %04d: cpuNo: %u pxEventList: 0x%x xEventListItem: 0x%x\n", 
						__FUNCTION__, __LINE__, cpuNo, (UINT32) pxEventList, 
						(UINT32) &( pxTCB->xEventListItem ) );
*/
		/* Place the event list item of the TCB in the appropriate event list.
		This is placed in the list in priority order so the highest priority task
		is the first to be woken by the event. */
		/* vListInsert( ( xList * ) pxEventList, ( xListItem * ) &( pxCurrentTCB[cpuNo]->xEventListItem ) ); */
		vListInsert( ( xList * ) pxEventList, ( xListItem * ) &( pxTCB->xEventListItem ) );
		
		/* DBLOG( "%s %04d: cpuNo: %u\n", __FUNCTION__, __LINE__, cpuNo); */
	
		/* We must remove ourselves from the ready list before adding ourselves
		to the blocked list as the same list item is used for both lists.  We have
		exclusive access to the ready lists as the scheduler is locked. */
		/* vListRemove( ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) ); */
		vListRemove( ( xListItem * ) &( pxTCB->xGenericListItem ) );
	
#if ( INCLUDE_vTaskSuspend == 1 )
		{
			if( xTicksToWait == portMAX_DELAY )
			{
				/* Add ourselves to the suspended task list instead of a delayed task
				list to ensure we are not woken by a timing event.  We will block
				indefinitely. */
				/* vListInsertEnd( ( xList * ) &(cpuTaskCtrl[cpuNo].xSuspendedTaskList), ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) ); */
				vListInsertEnd( ( xList * ) &(cpuTaskCtrl[cpuNo].xSuspendedTaskList), ( xListItem * ) &( pxTCB->xGenericListItem ) );
				/* DBLOG( "%s %04d: cpuNo: %u\n", __FUNCTION__, __LINE__, cpuNo ); */
			}
			else
			{
				/* Calculate the time at which the task should be woken if the event does
				not occur.  This may overflow but this doesn't matter. */
				xTimeToWake = cpuTaskCtrl[cpuNo].xTickCount + xTicksToWait;
	
				/* listSET_LIST_ITEM_VALUE( &( pxCurrentTCB[cpuNo]->xGenericListItem ), xTimeToWake ); */
				listSET_LIST_ITEM_VALUE( &( pxTCB->xGenericListItem ), xTimeToWake );
	
				if( xTimeToWake < cpuTaskCtrl[cpuNo].xTickCount )
				{
					/* DBLOG( "%s %04d: cpuNo: %u\n", __FUNCTION__, __LINE__, cpuNo ); */
					/* Wake time has overflowed.  Place this item in the overflow list. */
					/* vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList, ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) ); */
					vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList, ( xListItem * ) &( pxTCB->xGenericListItem ) );
				}
				else
				{
					/* DBLOG( "%s %04d: cpuNo: %u\n", __FUNCTION__, __LINE__, cpuNo ); */
					/* The wake time has not overflowed, so we can use the current block list. */
					/* vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxDelayedTaskList, ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) ); */
					vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxDelayedTaskList, ( xListItem * ) &( pxTCB->xGenericListItem ) );
				}
			}
		}
#else
		{
				/* Calculate the time at which the task should be woken if the event does
				not occur.  This may overflow but this doesn't matter. */
				xTimeToWake = cpuTaskCtrl[cpuNo].xTickCount + xTicksToWait;
	
				listSET_LIST_ITEM_VALUE( &( pxCurrentTCB[cpuNo]->xGenericListItem ), xTimeToWake );
	
				if( xTimeToWake < cpuTaskCtrl[cpuNo].xTickCount )
				{
					/* Wake time has overflowed.  Place this item in the overflow list. */
					vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList, ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) );
				}
				else
				{
					/* The wake time has not overflowed, so we can use the current block list. */
					vListInsert( ( xList * ) cpuTaskCtrl[cpuNo].pxDelayedTaskList, ( xListItem * ) &( pxCurrentTCB[cpuNo]->xGenericListItem ) );
				}
		}
#endif
		
		/* DBLOG( "%s %04d: cpuNo: %u\n", __FUNCTION__, __LINE__, cpuNo ); */
	}
	taskEXIT_CRITICAL( cpuNo, pdFALSE );
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xTaskRemoveFromEventList( portSHORT *psCpuNo, const xList * const pxEventList, int fromIsr )
{
	tskTCB *pxUnblockedTCB;
	portBASE_TYPE xReturn;
	

	/* THIS FUNCTION MUST BE CALLED WITH INTERRUPTS DISABLED OR THE
	SCHEDULER SUSPENDED.  It can also be called from within an ISR. */

	/* The event list is sorted in priority order, so we can remove the
	first in the list, remove the TCB from the delayed list, and add
	it to the ready list.

	If an event is for a queue that is locked then this function will never
	get called - the lock count on the queue will get modified instead.  This
	means we can always expect exclusive access to the event list here. */
	
	pxUnblockedTCB = ( tskTCB * ) listGET_OWNER_OF_HEAD_ENTRY( pxEventList );
	
	taskENTER_CRITICAL( pxUnblockedTCB->uxCPUAffinity, fromIsr );
	{
		vListRemove( &( pxUnblockedTCB->xEventListItem ) );
		
		*psCpuNo = pxUnblockedTCB->uxCPUAffinity;
	
		if( atomic32Get( (UINT32 *) &cpuTaskCtrl[pxUnblockedTCB->uxCPUAffinity].uxSchedulerSuspended ) == (UINT32) pdFALSE )
		{
/*
			DBLOG( "%s %04d: myCpu: %u *psCpuNo: %u pxEventList 0x%x xEventListItem: 0x%x\n", 
						__FUNCTION__, __LINE__, sPortGetCurrentCPU(), pxUnblockedTCB->uxCPUAffinity, 
						(UINT32) pxEventList, (UINT32) &pxUnblockedTCB->xEventListItem );
*/
			vListRemove( &( pxUnblockedTCB->xGenericListItem ) );
			prvAddTaskToReadyQueue( pxUnblockedTCB, pxUnblockedTCB->uxCPUAffinity );
		}
		else
		{
/*
			DBLOG( "%s %04d: myCpu: %u *psCpuNo: %u pxEventList 0x%x xEventListItem: 0x%x\n", 
						__FUNCTION__, __LINE__, sPortGetCurrentCPU(), pxUnblockedTCB->uxCPUAffinity, 
						(UINT32) pxEventList, (UINT32) &pxUnblockedTCB->xEventListItem );
*/
			/* We cannot access the delayed or ready lists, so will hold this
			task pending until the scheduler is resumed. */
			vListInsertEnd( ( xList * ) &( cpuTaskCtrl[pxUnblockedTCB->uxCPUAffinity].xPendingReadyList ), &( pxUnblockedTCB->xEventListItem ) );
		}
	
		if( pxUnblockedTCB->uxPriority >= pxCurrentTCB[pxUnblockedTCB->uxCPUAffinity]->uxPriority )
		{
			/* Return true if the task removed from the event list has
			a higher priority than the calling task.  This allows
			the calling task to know if it should force a context
			switch now. */
			xReturn = pdTRUE;
		}
		else
		{
			xReturn = pdFALSE;
		}
	
/*	
		DBLOG( "%s %04d: myCpu: %u *psCpuNo: %u pxEventList 0x%x xEventListItem: 0x%x\n", 
						__FUNCTION__, __LINE__, sPortGetCurrentCPU(), pxUnblockedTCB->uxCPUAffinity, 
						(UINT32) pxEventList, (UINT32) &pxUnblockedTCB->xEventListItem );
*/
	}	
	taskEXIT_CRITICAL( pxUnblockedTCB->uxCPUAffinity, fromIsr );
	
	return xReturn;
}
/*-----------------------------------------------------------*/

void vTaskSetTimeOutState( portSHORT cpuNo, xTimeOutType * const pxTimeOut )
{		
	pxTimeOut->xOverflowCount = atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xNumOfOverflows );
	pxTimeOut->xTimeOnEntering = atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xTickCount );
}
/*-----------------------------------------------------------*/

portBASE_TYPE xTaskCheckForTimeOut( portSHORT cpuNo, xTimeOutType * const pxTimeOut, portTickType * const pxTicksToWait )
{
	portBASE_TYPE xReturn;
	
	
	taskENTER_CRITICAL( cpuNo, pdFALSE );
	{
#if ( INCLUDE_vTaskSuspend == 1 )
		/* If INCLUDE_vTaskSuspend is set to 1 and the block time specified is
		the maximum block time then the task should block indefinitely, and
		therefore never time out. */
		if( *pxTicksToWait == portMAX_DELAY )
		{
			xReturn = pdFALSE;
		}
		else /* We are not blocking indefinitely, perform the checks below. */
#endif
		{
			if( ( cpuTaskCtrl[cpuNo].xNumOfOverflows != pxTimeOut->xOverflowCount ) && 
			( ( portTickType ) cpuTaskCtrl[cpuNo].xTickCount >= ( portTickType ) pxTimeOut->xTimeOnEntering ) )
			{
				/* The tick count is greater than the time at which vTaskSetTimeout()
				was called, but has also overflowed since vTaskSetTimeOut() was called.
				It must have wrapped all the way around and gone past us again. This
				passed since vTaskSetTimeout() was called. */
				xReturn = pdTRUE;
			}
			else if( ( ( portTickType ) ( ( portTickType ) cpuTaskCtrl[cpuNo].xTickCount - ( portTickType ) pxTimeOut->xTimeOnEntering ) ) < ( portTickType ) *pxTicksToWait )
			{
				/* Not a genuine timeout. Adjust parameters for time remaining. */
				*pxTicksToWait -= ( ( portTickType ) cpuTaskCtrl[cpuNo].xTickCount - ( portTickType ) pxTimeOut->xTimeOnEntering );
				vTaskSetTimeOutState( cpuNo, pxTimeOut );
				xReturn = pdFALSE;
			}
			else
			{
				xReturn = pdTRUE;
			}
		}
	}
	taskEXIT_CRITICAL( cpuNo, pdFALSE );

	return xReturn;
}
/*-----------------------------------------------------------*/

void vTaskMissedYield( portSHORT cpuNo )
{
	
	atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].xMissedYield, pdTRUE );
}

/*
 * -----------------------------------------------------------
 * The Idle task.
 * ----------------------------------------------------------
 *
 * The portTASK_FUNCTION() macro is used to allow port/compiler specific
 * language extensions.  The equivalent prototype for this function is:
 *
 * void prvIdleTask( void *pvParameters );
 *
 * Warning - blocking the idle task in any way could cause a scenario 
 * where no tasks are available to enter the Running state.  
 */
 
static void prvIdleTask( void *pvParameters )
{
	portSHORT cpuNo;
	
	
	/* Stop warnings. */
	( void ) pvParameters;
	
	cpuNo = sPortGetCurrentCPU();

#ifdef INCLUDE_TASK_DEBUG	
	// DBLOG( "Executing IDLE Task on CPU:%d\n", cpuNo );
#endif
	
	for( ;; )
	{
		atomic32Inc( &idleCount[cpuNo] );
		
		/* See if any tasks have been deleted. */
		prvCheckTasksWaitingTermination( cpuNo );

		#if ( configUSE_PREEMPTION == 0 )
		{
			/* If we are not using preemption we keep forcing a task switch to
			see if any other task has become available.  If we are using
			preemption we don't need to do this as any task becoming available
			will automatically get the processor anyway. */
			taskYIELD( cpuNo );
		}
		#endif

		#if ( ( configUSE_PREEMPTION == 1 ) && ( configIDLE_SHOULD_YIELD == 1 ) )
		{
			/* When using preemption tasks of equal priority will be
			timesliced.  If a task that is sharing the idle priority is ready
			to run then the idle task should yield before the end of the
			timeslice.

			A critical region is not required here as we are just reading from
			the list, and an occasional incorrect value will not matter.  If
			the ready list at the idle priority contains more than one task
			then a task other than the idle task is ready to execute. */
			if( listCURRENT_LIST_LENGTH( &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ tskIDLE_PRIORITY ] ) ) > ( unsigned portBASE_TYPE ) 1 )
			{
				taskYIELD( cpuNo );
			}
		}
		#endif

		#if ( configUSE_IDLE_HOOK == 1 )
		{
			extern void vApplicationIdleHook( void );

			/* Call the user defined function from within the idle task.  This
			allows the application designer to add background functionality
			without the overhead of a separate task.
			NOTE: vApplicationIdleHook() MUST NOT, UNDER ANY CIRCUMSTANCES,
			CALL A FUNCTION THAT MIGHT BLOCK. */
			vApplicationIdleHook();
		}
		#endif
	}
} /*lint !e715 pvParameters is not accessed but all task functions require the same prototype. */


/*-----------------------------------------------------------
 * File private functions documented at the top of the file.
 *----------------------------------------------------------*/


static void prvInitialiseTCBVariables( tskTCB *pxTCB, const signed char * const pcName, unsigned portBASE_TYPE uxPriority, const xMemoryRegion * const xRegions, unsigned short usStackDepth )
{
	/* Store the function name in the TCB. */
	#if configMAX_TASK_NAME_LEN > 1
	{
		/* Don't bring strncpy into the build unnecessarily. */
		strncpy( ( char * ) pxTCB->pcTaskName, ( const char * ) pcName, ( unsigned short ) configMAX_TASK_NAME_LEN );
	}
	#endif
	pxTCB->pcTaskName[ ( unsigned short ) configMAX_TASK_NAME_LEN - ( unsigned short ) 1 ] = '\0';

	/* This is used as an array index so must ensure it's not too large.  First
	remove the privilege bit if one is present. */
	if( uxPriority >= configMAX_PRIORITIES )
	{
		uxPriority = configMAX_PRIORITIES - 1;
	}

	pxTCB->uxPriority = uxPriority;
	#if ( configUSE_MUTEXES == 1 )
	{
		pxTCB->uxBasePriority = uxPriority;
	}
	#endif

	vListInitialiseItem( &( pxTCB->xGenericListItem ) );
	vListInitialiseItem( &( pxTCB->xEventListItem ) );

	/* Set the pxTCB as a link back from the xListItem.  This is so we can get
	back to	the containing TCB from a generic item in a list. */
	listSET_LIST_ITEM_OWNER( &( pxTCB->xGenericListItem ), pxTCB );

	/* Event lists are always in priority order. */
	listSET_LIST_ITEM_VALUE( &( pxTCB->xEventListItem ), configMAX_PRIORITIES - ( portTickType ) uxPriority );
	listSET_LIST_ITEM_OWNER( &( pxTCB->xEventListItem ), pxTCB );

	#if ( portCRITICAL_NESTING_IN_TCB == 1 )
	{
		pxTCB->uxCriticalNesting = ( unsigned portBASE_TYPE ) 0;
	}
	#endif

	#if ( configUSE_APPLICATION_TASK_TAG == 1 )
	{
		pxTCB->pxTaskTag = NULL;
	}
	#endif

	#if ( configGENERATE_RUN_TIME_STATS == 1 )
	{
		pxTCB->ulRunTimeCounter.i32[1] = 0;
		pxTCB->ulRunTimeCounter.i32[0] = 0;
	}
	#endif

	#if ( portUSING_MPU_WRAPPERS == 1 )
	{
		vPortStoreTaskMPUSettings( &( pxTCB->xMPUSettings ), xRegions, pxTCB->pxStack, usStackDepth );
	}
	#else
	{
		( void ) xRegions;
		( void ) usStackDepth;
	}
	#endif
}
/*-----------------------------------------------------------*/

#if ( portUSING_MPU_WRAPPERS == 1 )

	void vTaskAllocateMPURegions( xTaskHandle xTaskToModify, const xMemoryRegion * const xRegions )
	{
		tskTCB *pxTCB;
		portSHORT cpuNo;
		short sValidTask;


		cpuNo = sPortGetCurrentCPU();
		sValidTask = pdTRUE;
		
		if (xTaskToModify != NULL)
		{
			if (((tskTCB *) xTaskToModify)->uxCPUAffinity != cpuNo)
			{
				sValidTask = pdFALSE;
			}
		}
		
		if(sValidTask == pdTRUE)
		{
			if( xTaskToModify == pxCurrentTCB[cpuNo] )
			{
				xTaskToModify = NULL;
			}
	
			/* If null is passed in here then we are modifying ourselves. */
			pxTCB = prvGetTCBFromHandle( xTaskToModify );
	
	        vPortStoreTaskMPUSettings( &( pxTCB->xMPUSettings ), xRegions, NULL, 0 );
	    }
	}
	/*-----------------------------------------------------------*/
#endif

static void prvInitialiseTaskLists( portSHORT cpuNo )
{
	unsigned portBASE_TYPE uxPriority;


	for( uxPriority = 0; uxPriority < configMAX_PRIORITIES; uxPriority++ )
	{
		vListInitialise( ( xList * ) &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ uxPriority ] ) );
	}

	vListInitialise( ( xList * ) &(cpuTaskCtrl[cpuNo].xDelayedTaskList1) );
	vListInitialise( ( xList * ) &(cpuTaskCtrl[cpuNo].xDelayedTaskList2) );
	vListInitialise( ( xList * ) &(cpuTaskCtrl[cpuNo].xPendingReadyList) );

	#if ( INCLUDE_vTaskDelete == 1 )
	{
		vListInitialise( ( xList * ) &(cpuTaskCtrl[cpuNo].xTasksWaitingTermination) );
	}
	#endif

	#if ( INCLUDE_vTaskSuspend == 1 )
	{
		vListInitialise( ( xList * ) &(cpuTaskCtrl[cpuNo].xSuspendedTaskList) );
	}
	#endif

	/* Start with pxDelayedTaskList using list1 and the pxOverflowDelayedTaskList
	using list2. */
	cpuTaskCtrl[cpuNo].pxDelayedTaskList = &(cpuTaskCtrl[cpuNo].xDelayedTaskList1);
	cpuTaskCtrl[cpuNo].pxOverflowDelayedTaskList = &(cpuTaskCtrl[cpuNo].xDelayedTaskList2);
}
/*-----------------------------------------------------------*/

static void prvCheckTasksWaitingTermination( portSHORT cpuNo )
{
	#if ( INCLUDE_vTaskDelete == 1 )
	{
		portBASE_TYPE xListIsEmpty;
		tskTCB *pxTCB = NULL;
		

		/* ucTasksDeleted is used to prevent this from being called
		too often in the idle task. */
		if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTasksDeleted ) > (UINT32) 0 )
		{
			taskENTER_CRITICAL( cpuNo, pdFALSE );
			{
				xListIsEmpty = listLIST_IS_EMPTY( &(cpuTaskCtrl[cpuNo].xTasksWaitingTermination) );
	
				if( !xListIsEmpty )
				{
					pxTCB = ( tskTCB * ) listGET_OWNER_OF_HEAD_ENTRY( ( ( xList * ) &(cpuTaskCtrl[cpuNo].xTasksWaitingTermination) ) );
					vListRemove( &( pxTCB->xGenericListItem ) );
					
					atomic32Dec( (UINT32 *) &cpuTaskCtrl[cpuNo].uxCurrentNumberOfTasks );
					
					atomic32Dec( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTasksDeleted );
					
					atomic32Dec( (UINT32 *) &uxTotalTaskNumber );
				}
			}
			taskEXIT_CRITICAL( cpuNo, pdFALSE );
			
			if (pxTCB != NULL)
			{
#ifdef INCLUDE_TASK_DEBUG		
				DBLOG( "Terminating %s task on CPU%d\n", pxTCB->pcTaskName, cpuNo );
#endif
				prvDeleteTCB( pxTCB );
			}
		}
	}
	#endif
}
/*-----------------------------------------------------------*/

static tskTCB *prvAllocateTCBAndStack( unsigned short usStackDepth, portSTACK_TYPE *puxStackBuffer )
{
	tskTCB *pxNewTCB;

	/* Allocate space for the TCB.  Where the memory comes from depends on
	the implementation of the port malloc function. */
	pxNewTCB = ( tskTCB * ) pvPortMalloc( sizeof( tskTCB ) );

	if( pxNewTCB != NULL )
	{
		/* Allocate space for the stack used by the task being created.
		The base of the stack memory stored in the TCB so the task can
		be deleted later if required. */
		pxNewTCB->pxStack = ( portSTACK_TYPE * ) pvPortMallocAligned( ( ( ( size_t )usStackDepth ) * sizeof( portSTACK_TYPE ) ), puxStackBuffer );

		if( pxNewTCB->pxStack == NULL )
		{
			/* Could not allocate the stack.  Delete the allocated TCB. */
			vPortFree( pxNewTCB );
			pxNewTCB = NULL;
		}
		else
		{
			/* Just to help debugging. */
			memset( pxNewTCB->pxStack, tskSTACK_FILL_BYTE, usStackDepth * sizeof( portSTACK_TYPE ) );
		}
	}

	return pxNewTCB;
}
/*-----------------------------------------------------------*/

#if ( INCLUDE_vTaskList == 1 )

	static void prvListTaskWithinSingleList( const signed char *pcWriteBuffer, xList *pxList, 
												signed char cStatus, TS_UNION ulTotalRunTime )
	{
		volatile tskTCB *pxNextTCB, *pxFirstTCB;
		UINT32 usStackRemaining;
#if ( configGENERATE_RUN_TIME_STATS == 1 )
		UINT32 ulStatsAsPercentage;
#endif

		/* Write the details of all the TCB's in pxList into the buffer. */
		listGET_OWNER_OF_NEXT_ENTRY( pxFirstTCB, pxList );
		do
		{
			listGET_OWNER_OF_NEXT_ENTRY( pxNextTCB, pxList );
			#if ( portSTACK_GROWTH > 0 )
			{
				usStackRemaining = usTaskCheckFreeStackSpace( ( unsigned char * ) pxNextTCB->pxEndOfStack );
			}
			#else
			{
				usStackRemaining = usTaskCheckFreeStackSpace( ( unsigned char * ) pxNextTCB->pxStack );
			}
			#endif
			
			#if ( configGENERATE_RUN_TIME_STATS == 1 )
			{
				/* Divide by zero check. */
				if( ulTotalRunTime.i64 > 0 )
				{
					/* Has the task run at all? */
					if( pxNextTCB->ulRunTimeCounter.i64 == 0 )
					{
						/* The task has used no CPU time at all. */
						ulStatsAsPercentage = 0;
					}
					else
					{
						/* What percentage of the total run time has the task used?
						This will always be rounded down to the nearest integer. */
						ulStatsAsPercentage = (UINT32) (( 100.0 * (long double) pxNextTCB->ulRunTimeCounter.i64 ) / (long double) ulTotalRunTime.i64);
					}
					
					if( ulStatsAsPercentage > 0 )
					{
						sprintf( pcStatusString, ( char * ) "%-16s %u   %3u%%\t%c\t%u\t%5u\t%u\r\n", 
								pxNextTCB->pcTaskName, pxNextTCB->uxCPUAffinity, ulStatsAsPercentage,
								cStatus, ( unsigned int ) pxNextTCB->uxPriority, 
								usStackRemaining, (unsigned int) pxNextTCB->uxTCBNumber );
					}
					else
					{
						/* If the percentage is zero here then the task has
						consumed less than 1% of the total run time. */
						sprintf( pcStatusString, ( char * ) "%-16s %u    <1%%\t%c\t%u\t%5u\t%u\r\n", 
								pxNextTCB->pcTaskName, pxNextTCB->uxCPUAffinity,
								cStatus, ( unsigned int ) pxNextTCB->uxPriority, 
								usStackRemaining, (unsigned int) pxNextTCB->uxTCBNumber );
					}			
				}
			}
			#else
			{
			    /* Just to stop compiler warnings */ 
			    if( ulTotalRunTime.i64 > 0 )
                { 
				    ulTotalRunTime.i64 = 0;
				}
				
				sprintf( pcStatusString, ( char * ) "%-16s %u\t%c\t%u\t%5u\t%u\r\n", 
						pxNextTCB->pcTaskName, pxNextTCB->uxCPUAffinity,
						cStatus, ( unsigned int ) pxNextTCB->uxPriority, 
						usStackRemaining, (unsigned int) pxNextTCB->uxTCBNumber );
			}
			#endif
			
			strcat( ( char * ) pcWriteBuffer, ( char * ) pcStatusString );

		} while( pxNextTCB != pxFirstTCB );
	}

#endif

/*-----------------------------------------------------------*/

#if ( ( INCLUDE_vTaskList == 1 ) || ( INCLUDE_uxTaskGetStackHighWaterMark == 1 ) )

	static UINT32 usTaskCheckFreeStackSpace( const unsigned char * pucStackByte )
	{
		register UINT32 usCount = 0;

		while( *pucStackByte == tskSTACK_FILL_BYTE )
		{
			pucStackByte -= portSTACK_GROWTH;
			usCount++;
		}

		/* Comment out the next line to show value in bytes   */
		/* (unused to high water mark) and not in stack units */
		
		/* usCount /= sizeof( portSTACK_TYPE ); */

		return usCount;
	}

#endif
/*-----------------------------------------------------------*/

#if ( INCLUDE_uxTaskGetStackHighWaterMark == 1 )

	unsigned portBASE_TYPE uxTaskGetStackHighWaterMark( xTaskHandle xTask )
	{
		tskTCB *pxTCB;
		portSHORT cpuNo;
		portSHORT myCpu;
		unsigned char *pcEndOfStack;
		unsigned portBASE_TYPE uxReturn;
		
		
		myCpu = sPortGetCurrentCPU();
		if (xTask == NULL)
		{
			/* We're using the current task */
			cpuNo = myCpu;
		}
		else
		{
			cpuNo = ((tskTCB *) xTask)->uxCPUAffinity;
		}
			
		pxTCB = prvGetTCBFromHandle( xTask );

		#if portSTACK_GROWTH < 0
		{
			pcEndOfStack = ( unsigned char * ) pxTCB->pxStack;
		}
		#else
		{
			pcEndOfStack = ( unsigned char * ) pxTCB->pxEndOfStack;
		}
		#endif

		uxReturn = ( unsigned portBASE_TYPE ) usTaskCheckFreeStackSpace( pcEndOfStack );
		
		return uxReturn;
	}

#endif
/*-----------------------------------------------------------*/

#if ( ( INCLUDE_vTaskDelete == 1 ) || ( INCLUDE_vTaskCleanUpResources == 1 ) )

	static void prvDeleteTCB( tskTCB *pxTCB )
	{
		/* Free up the memory allocated by the scheduler for the task.  It is up to
		the task to free any memory allocated at the application level. */
		vPortFreeAligned( pxTCB->pxStack );
		vPortFree( pxTCB );
	}

#endif


/*-----------------------------------------------------------*/

#if ( INCLUDE_xTaskGetCurrentTaskHandle == 1 )

	xTaskHandle xTaskGetCurrentTaskHandle( void )
	{
		xTaskHandle xReturn;
		portSHORT cpuNo;
		UINT32 key;


		cpuNo = sPortGetCurrentCPU();
		
		portDISABLE_INTERRUPTS( key );
		xReturn = (xTaskHandle) pxCurrentTCB[cpuNo];
		portENABLE_INTERRUPTS( key );

		return xReturn;
	}

#endif

/*-----------------------------------------------------------*/

#if ( INCLUDE_xTaskGetSchedulerState == 1 )

	portBASE_TYPE xTaskGetSchedulerState( void )
	{
		portSHORT cpuNo;
		portBASE_TYPE xReturn;
		
		cpuNo = sPortGetCurrentCPU();

		if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].xSchedulerRunning ) == (UINT32) pdFALSE )
		{
			xReturn = taskSCHEDULER_NOT_STARTED;
		}
		else
		{
			if( atomic32Get( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended ) == (UINT32) pdFALSE )
			{
				xReturn = taskSCHEDULER_RUNNING;
			}
			else
			{
				xReturn = taskSCHEDULER_SUSPENDED;
			}
		}

		return xReturn;
	}

#endif
/*-----------------------------------------------------------*/

#if ( configUSE_MUTEXES == 1 )

	void vTaskPriorityInherit( xTaskHandle * const pxMutexHolder, int fromIsr )
	{
		tskTCB * const pxTCB = ( tskTCB * ) pxMutexHolder;
		portSHORT cpuNo;


		if( pxTCB != NULL )
		{
			cpuNo = pxTCB->uxCPUAffinity;
			
			if (pxTCB->uxPriority < pxCurrentTCB[cpuNo]->uxPriority) 
			{
				taskENTER_CRITICAL( cpuNo, fromIsr );
				{
					/* Adjust the mutex holder state to account for its new priority. */
					listSET_LIST_ITEM_VALUE( &( pxTCB->xEventListItem ), (configMAX_PRIORITIES - ( portTickType ) pxCurrentTCB[cpuNo]->uxPriority) );
		
					/* If the task being modified is in the ready state it will need to
					be moved in to a new list. */
					if( listIS_CONTAINED_WITHIN( &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[ pxTCB->uxPriority ] ), &( pxTCB->xGenericListItem ) ) )
					{
						vListRemove( &( pxTCB->xGenericListItem ) );
		
						/* Inherit the priority before being moved into the new list. */
						pxTCB->uxPriority = pxCurrentTCB[cpuNo]->uxPriority;
						prvAddTaskToReadyQueue( pxTCB, cpuNo );
					}
					else
					{
						/* Just inherit the priority. */
						pxTCB->uxPriority = pxCurrentTCB[cpuNo]->uxPriority;
					}
				}
				taskEXIT_CRITICAL( cpuNo, fromIsr );
			}
		}
	}

#endif
/*-----------------------------------------------------------*/

#if ( configUSE_MUTEXES == 1 )

	void vTaskPriorityDisinherit( xTaskHandle * const pxMutexHolder, int fromIsr )
	{
		portSHORT cpuNo;
		tskTCB * const pxTCB = ( tskTCB * ) pxMutexHolder;


		if( pxTCB != NULL )
		{	
			if( pxTCB->uxPriority != pxTCB->uxBasePriority )
			{
				cpuNo = pxTCB->uxCPUAffinity;
				
				taskENTER_CRITICAL( cpuNo, fromIsr );
				{
					/* We must be the running task to be able to give the mutex back.
					Remove ourselves from the ready list we currently appear in. */
					vListRemove( &( pxTCB->xGenericListItem ) );
	
					/* Disinherit the priority before adding ourselves into the new
					ready list. */
					pxTCB->uxPriority = pxTCB->uxBasePriority;
					listSET_LIST_ITEM_VALUE( &( pxTCB->xEventListItem ), configMAX_PRIORITIES - ( portTickType ) pxTCB->uxPriority );
					prvAddTaskToReadyQueue( pxTCB, cpuNo );
				}
				taskEXIT_CRITICAL( cpuNo, fromIsr );
			}
		}
	}

#endif
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------
 * Initialization Functions 
 *----------------------------------------------------------*/

/*******************************************************************************
*
* vTaskDataInit
*
* Initializes local data, must be called early in statup sequence, from
* main.c.  
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/ 
void vTaskDataInit( UINT8 bCpusAvailable )
{
	portSHORT cpuNo;
	

	/* Setup the number of CPU's we can use */
	
	/* If manual override */
	if (MAX_CONFIG_CPUS > 0)
	{
		if (bCpuCount == 2)
		{
			bCpuConfigured = 1;
		}
		else
		{
			/* Set to user configured value */
			bCpuConfigured = MAX_CONFIG_CPUS;
		}
	}
	else if ( bCpusAvailable > 0 )
	{
		if (bCpusAvailable > configMAX_CPU_NUM)
		{
			bCpuConfigured = configMAX_CPU_NUM;
		}
		else
		{
			bCpuConfigured = bCpusAvailable;
		}
	}
	else
	{
		bCpuConfigured = 1;
	}


#ifdef INCLUDE_TASK_DEBUG	
	DBLOG( "CPU's found: %u configured: %u\n", bCpuCount, bCpuConfigured );
#endif

	/* Initialize port data */
	vPortDataInit();

	/* Initialize our data */
	memset( cpuTaskCtrl, 0, sizeof(cpuTaskCtrl) );
	memset( pxCurrentTCB, 0, sizeof(pxCurrentTCB) );
	memset( idleCount, 0, sizeof(idleCount) );
	
	/* Initialize for each CPU */
	for (cpuNo = 0; cpuNo < configMAX_CPU_NUM; cpuNo++ )
	{
		/* Make sure scheduler is initially suspended */
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].uxContextSwitchSuspended, pdTRUE );
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].uxSchedulerSuspended, pdTRUE );
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].xSchedulerRunning, pdFALSE );
	
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTopUsedPriority, tskIDLE_PRIORITY );
		atomic32Set( (UINT32 *) &cpuTaskCtrl[cpuNo].uxTopReadyPriority, tskIDLE_PRIORITY );
		
		spinLockInit( &cpuTaskCtrl[cpuNo].sLock );
		
#ifdef INCLUDE_TASK_DEBUG
		if (cpuNo < bCpuConfigured)
		{
			DBLOG( "CPU%d ApicId: 0x%02x\n", cpuNo, abApicId[cpuNo] );
		}
#endif
	}
}

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------
 * Include/Exclude Additional Debug Functions 
 *----------------------------------------------------------*/
#ifdef INCLUDE_EXCEPTION_DEBUG
#include "../../debug/taskdbg.c"
#endif

/*-----------------------------------------------------------*/
