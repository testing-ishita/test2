/*******************************************************************************
*
* Filename: 	queue.c
* 
* Description:	FreeRTOS queue functions, adapted for use with CCT boards.
*
* $Revision: 1.2 $
*
* $Date: 2013-10-08 07:20:21 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/FreeRTOS/Source/queue.c,v $
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

#include <stdlib.h>
#include <string.h>

#include <private/semaphore.h>
#include <private/atomic.h>
#include <private/spinlock.h>
#include <private/debug.h>

#undef INCLUDE_QUEUE_DEBUG /* define/undefine for debugging */

#if configUSE_ALTERNATIVE_API == 1
#error "***** ALTERNATIVE API NOT SUPPORTED *****"
#endif

#if configUSE_CO_ROUTINES == 1
#error "***** CO ROUTINES NOT SUPPORTED *****"
#endif

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"

#include "../debug/dbgLog.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/*-----------------------------------------------------------
 * PUBLIC LIST API documented in list.h
 *----------------------------------------------------------*/

/* Constants used with the cRxLock and cTxLock structure members. */
#define queueUNLOCKED					( ( INT32 ) -1 )
#define queueLOCKED_UNMODIFIED			( ( INT32 ) 0 )

#define queueERRONEOUS_UNBLOCK			( -1 )

/* For internal use only. */
#define	queueSEND_TO_BACK				( 0 )
#define	queueSEND_TO_FRONT				( 1 )

/* Effectively make a union out of the xQUEUE structure. */
#define pxMutexHolder					pcTail
#define uxQueueType						pcHead
#define uxRecursiveCallCount			pcReadFrom
#define queueQUEUE_IS_MUTEX				NULL

/* Semaphores do not actually store or copy data, so have an items size of
zero. */
#define queueSEMAPHORE_QUEUE_ITEM_LENGTH ( 0 )
#define queueDONT_BLOCK					 ( ( portTickType ) 0 )
#define queueMUTEX_GIVE_BLOCK_TIME		 ( ( portTickType ) 0 )

/*
 * Definition of the queue used by the scheduler.
 * Items are queued by copy, not reference.
 */
typedef struct QueueDefinition
{
	signed char *pcHead;				/*< Points to the beginning of the queue storage area. */
	signed char *pcTail;				/*< Points to the byte at the end of the queue storage area.  Once more byte is allocated than necessary to store the queue items, this is used as a marker. */

	signed char *pcWriteTo;				/*< Points to the free next place in the storage area. */
	signed char *pcReadFrom;			/*< Points to the last place that a queued item was read from. */

	xList xTasksWaitingToSend;			/*< List of tasks that are blocked waiting to post onto this queue.  Stored in priority order. */
	xList xTasksWaitingToReceive;		/*< List of tasks that are blocked waiting to read from this queue.  Stored in priority order. */

	volatile unsigned portBASE_TYPE uxMessagesWaiting;/*< The number of items currently in the queue. */
	unsigned portBASE_TYPE uxLength;	/*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
	unsigned portBASE_TYPE uxItemSize;	/*< The size of each items that the queue will hold. */

	INT32 xRxLock;						/*< Stores the number of items received from the queue (removed from the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
	INT32 xTxLock;						/*< Stores the number of items transmitted to the queue (added to the queue) while the queue was locked.  Set to queueUNLOCKED when the queue is not locked. */
	
	portSHORT cpuNo;					/* CPU we were on when queue created */
	
	SPINLOCK sLock;						/* Spinlock for access protection */
	
} xQUEUE;
/*-----------------------------------------------------------*/

/*
 * Inside this file xQueueHandle is a pointer to a xQUEUE structure.
 * To keep the definition private the API header file defines it as a
 * pointer to void.
 */
typedef xQUEUE * xQueueHandle;


/*
 * The queue registry is just a means for kernel aware debuggers to locate
 * queue structures.  It has no other purpose so is an optional component.
 */
#if configQUEUE_REGISTRY_SIZE > 0

	/* The type stored within the queue registry array.  This allows a name
	to be assigned to each queue making kernel aware debugging a little
	more user friendly. */
	typedef struct QUEUE_REGISTRY_ITEM
	{
		signed char *pcQueueName;
		xQueueHandle xHandle;
	} xQueueRegistryItem;

	/* The queue registry is simply an array of xQueueRegistryItem structures.
	The pcQueueName member of a structure being NULL is indicative of the
	array position being vacant. */
	xQueueRegistryItem xQueueRegistry[ configQUEUE_REGISTRY_SIZE ];

	/* Removes a queue from the registry by simply setting the pcQueueName
	member to NULL. */
	static void vQueueUnregisterQueue( xQueueHandle xQueue ) PRIVILEGED_FUNCTION;
	void vQueueAddToRegistry( xQueueHandle xQueue, signed char *pcQueueName ) PRIVILEGED_FUNCTION;
#endif

/*-----------------------------------------------------------*/

#ifdef INCLUDE_QUEUE_DEBUG
#warning "***** DEBUG ON *****" 

void xQueueShow( xQueueHandle pxQueue )
{
	DBLOG("pxQueue: 0x%x cpuNo: %d\n", (UINT32) pxQueue, pxQueue->cpuNo );
	
	if (pxQueue != NULL)
	{
		DBLOG("pcHead                : 0x%x\n", (UINT32) pxQueue->pcHead);
		DBLOG("pcTail                : 0x%x\n", (UINT32) pxQueue->pcTail);
		
		DBLOG("pcWriteTo             : 0x%x\n", (UINT32) pxQueue->pcWriteTo);
		DBLOG("pcReadFrom            : 0x%x\n", (UINT32) pxQueue->pcReadFrom);
		
		DBLOG("xTasksWaitingToSend   : 0x%x entries: %u\n", &( pxQueue->xTasksWaitingToSend ), listCURRENT_LIST_LENGTH( &( pxQueue->xTasksWaitingToSend ) ) );
		DBLOG("xTasksWaitingToReceive: 0x%x entries: %u\n", &( pxQueue->xTasksWaitingToReceive ), listCURRENT_LIST_LENGTH( &( pxQueue->xTasksWaitingToReceive ) ) );
		
		DBLOG("uxMessagesWaiting     : %u\n", pxQueue->uxMessagesWaiting );
		DBLOG("uxLength              : %u\n", pxQueue->uxLength );
		DBLOG("uxItemSize            : %u\n", pxQueue->uxItemSize );
		
		DBLOG("xRxLock               : %d\n", pxQueue->xRxLock );
		DBLOG("xTxLock               : %d\n\n", pxQueue->xTxLock );
	}
}
#endif

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/

/*
 * Macro to mark a queue as locked.  Locking a queue prevents an ISR from
 * accessing the queue event lists. The queue spinlock must already be taken.
 */

#define prvLockQueue( pxQueue )						\
{													\
	if( pxQueue->xRxLock == queueUNLOCKED )			\
	{												\
		atomic32Set( (UINT32 *) &pxQueue->xRxLock, queueLOCKED_UNMODIFIED ); \
	}												\
	if( pxQueue->xTxLock == queueUNLOCKED )			\
	{												\
		atomic32Set( (UINT32 *) &pxQueue->xTxLock, queueLOCKED_UNMODIFIED ); \
	}												\
}
/*-----------------------------------------------------------*/

static void prvUnlockQueue( xQueueHandle pxQueue )
{
	portSHORT cpuNo;
	
	
	/* THIS FUNCTION MUST BE CALLED WITH THE SCHEDULER SUSPENDED. */

	/* The lock counts contains the number of extra data items placed or
	removed from the queue while the queue was locked.  When a queue is
	locked items can be added or removed, but the event lists cannot be
	updated. */
	
	portENTER_CRITICAL( &pxQueue->sLock, pdFALSE );
	{
		/* See if data was added to the queue while it was locked. */
		while( pxQueue->xTxLock > queueLOCKED_UNMODIFIED )
		{
			/* Data was posted while the queue was locked.  Are any tasks
			blocked waiting for data to become available? */
			if( !listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) )
			{
				/* Tasks that are removed from the event list will get added to
				the pending ready list as the scheduler is still suspended. */
				if( xTaskRemoveFromEventList( &cpuNo, &(pxQueue->xTasksWaitingToReceive), pdFALSE ) != pdFALSE )
				{
					/* The task waiting has a higher priority so record that a
					context	switch is required. */
					vTaskMissedYield( cpuNo );
				}

				atomic32Dec( (UINT32 *) &pxQueue->xTxLock );
			}
			else
			{
				break;
			}
		}

		atomic32Set( (UINT32 *) &pxQueue->xTxLock, queueUNLOCKED );
	}
	portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );

	/* Do the same for the Rx lock. */
	portENTER_CRITICAL( &pxQueue->sLock, pdFALSE );
	{
		while( pxQueue->xRxLock > queueLOCKED_UNMODIFIED )
		{
			if( !listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) )
			{
				if( xTaskRemoveFromEventList( &cpuNo, &(pxQueue->xTasksWaitingToSend), pdFALSE ) != pdFALSE )
				{
					vTaskMissedYield( cpuNo );
				}

				atomic32Dec( (UINT32 *) &pxQueue->xRxLock );
			}
			else
			{
				break;
			}
		}

		atomic32Set( (UINT32 *) &pxQueue->xRxLock, queueUNLOCKED );
	}
	portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );
}
/*-----------------------------------------------------------*/

static void prvCopyDataToQueue( xQUEUE *pxQueue, const void *pvItemToQueue, portBASE_TYPE xPosition, int fromIsr )
{
	if( pxQueue->uxItemSize == ( unsigned portBASE_TYPE ) 0 )
	{
		#if ( configUSE_MUTEXES == 1 )
		{
			if( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX )
			{
				/* The mutex is no longer being held. */
				vTaskPriorityDisinherit( ( void * ) pxQueue->pxMutexHolder, fromIsr );
				pxQueue->pxMutexHolder = NULL;
			}
		}
		#endif
	}
	else if( xPosition == queueSEND_TO_BACK )
	{
		if (pxQueue->uxItemSize == 1)
		{
			*((char *) pxQueue->pcWriteTo) = *((char *) pvItemToQueue);
		}
		else
		{
			bcopy( (char *) pvItemToQueue, (char *) pxQueue->pcWriteTo, pxQueue->uxItemSize );
		}
		
		pxQueue->pcWriteTo += pxQueue->uxItemSize;
		if( pxQueue->pcWriteTo >= pxQueue->pcTail )
		{
			pxQueue->pcWriteTo = pxQueue->pcHead;
		}
	}
	else
	{
		if (pxQueue->uxItemSize == 1)
		{
			*((char *) pxQueue->pcReadFrom) = *((char *) pvItemToQueue);
		}
		else
		{
			bcopy( (char *) pvItemToQueue, (char *) pxQueue->pcReadFrom, pxQueue->uxItemSize );
		}
		
		pxQueue->pcReadFrom -= pxQueue->uxItemSize;
		if( pxQueue->pcReadFrom < pxQueue->pcHead )
		{
			pxQueue->pcReadFrom = ( pxQueue->pcTail - pxQueue->uxItemSize );
		}
	}

	atomic32Inc( (UINT32 *) &pxQueue->uxMessagesWaiting );
}
/*-----------------------------------------------------------*/

static void prvCopyDataFromQueue( xQUEUE * const pxQueue, const void *pvBuffer )
{
	if( (pvBuffer != NULL) && (pxQueue->uxItemSize > 0) )
	{
		pxQueue->pcReadFrom += pxQueue->uxItemSize;
		if( pxQueue->pcReadFrom >= pxQueue->pcTail )
		{
			pxQueue->pcReadFrom = pxQueue->pcHead;
		}
		
		if (pxQueue->uxItemSize == 1)
		{
			*((char *) pvBuffer) = *((char *) pxQueue->pcReadFrom);
		}
		else
		{
			bcopy( (char *) pxQueue->pcReadFrom, (char *) pvBuffer, pxQueue->uxItemSize );
		}
	}
}
/*-----------------------------------------------------------*/

static signed portBASE_TYPE prvIsQueueFull( const xQueueHandle pxQueue )
{
	return ( atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting ) == ( UINT32 ) pxQueue->uxLength );
}
/*-----------------------------------------------------------*/

static signed portBASE_TYPE prvIsQueueEmpty( const xQueueHandle pxQueue )
{
	return ( atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting ) == ( UINT32 ) 0 );
}

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------
 * PUBLIC QUEUE MANAGEMENT API documented in queue.h
 *----------------------------------------------------------*/

signed portBASE_TYPE xQueueGenericSend( xQueueHandle pxQueue, const void * const pvItemToQueue, 
										portTickType xTicksToWait, portBASE_TYPE xCopyPosition )
{
	signed portBASE_TYPE xEntryTimeSet = pdFALSE;
	xTimeOutType xTimeOut;
	portSHORT myCpu;
	portSHORT eventCpuNo;
	signed portBASE_TYPE xYieldRequired;


	/* This function relaxes the coding standard somewhat to allow return
	statements within the function itself.  This is done in the interest
	of execution time efficiency. */
	
	myCpu = sPortGetCurrentCPU();
	eventCpuNo = myCpu;

/*	
	if (pvItemToQueue != NULL)
	{
		DBLOG( "%s %04d: cpuNo: %d pxQueue: 0x%08x pvItemToQueue: 0x%08x uxItemSize: %u\n", 
					__FUNCTION__, __LINE__, myCpu, (UINT32) pxQueue, (UINT32) pvItemToQueue, pxQueue->uxItemSize );
	}
*/	
	for( ;; )
	{
		portENTER_CRITICAL( &pxQueue->sLock, pdFALSE );
		{	
			/* Is there room on the queue now?  To be running we must be
			the highest priority task wanting to access the queue. */
			if( atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting ) < pxQueue->uxLength )
			{
				traceQUEUE_SEND( pxQueue );
				
				xYieldRequired = pdFALSE;
				
				prvCopyDataToQueue( pxQueue, pvItemToQueue, xCopyPosition, pdFALSE );

				/* If there was a task waiting for data to arrive on the
				queue then unblock it now. */
				
				if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
				{
					/* If the unblocked task has a priority higher than
						our own so yield immediately. */
					xYieldRequired = xTaskRemoveFromEventList( &eventCpuNo, &(pxQueue->xTasksWaitingToReceive), pdFALSE );
				}
				
				portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );
				
				if ( xYieldRequired == pdTRUE )
				{
					portYIELD( eventCpuNo );
				}
				
				/* Return to the original privilege level before exiting the
				function. */
				return pdPASS;
			}
			else
			{
				if( xTicksToWait == ( portTickType ) 0 )
				{
					/* The queue was full and no block time is specified (or
					the block time has expired) so leave now. */
					
					portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );
					
					/* Return to the original privilege level before exiting
					the function. */
					
					traceQUEUE_SEND_FAILED( pxQueue );
					
					return errQUEUE_FULL;
				}
				else if( xEntryTimeSet == pdFALSE )
				{
					/* The queue was full and a block time was specified so
					configure the timeout structure. */
					vTaskSetTimeOutState( myCpu, &xTimeOut );
					xEntryTimeSet = pdTRUE;
				}
			}
		}
		portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );
		

		/* Interrupts and other tasks can send to and receive from the queue
		now the critical section has been exited. */
		
		portENTER_CRITICAL( &pxQueue->sLock, pdFALSE );
		vTaskSuspendCpu( myCpu );
		prvLockQueue( pxQueue );
		portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );

		/* Update the timeout state to see if it has expired yet. */
		if( xTaskCheckForTimeOut( myCpu, &xTimeOut, &xTicksToWait ) == pdFALSE )
		{	
			/* If the queue is full, add us to the waiting to send list */
			if( prvIsQueueFull( pxQueue ) )
			{	
				traceBLOCKING_ON_QUEUE_SEND( pxQueue );
/*
				DBLOG( "%s %04d: cpuNo: %d pxQueue: 0x%08x pvItemToQueue: 0x%08x uxItemSize: %u\n", 
					__FUNCTION__, __LINE__, myCpu, (UINT32) pxQueue, (UINT32) pvItemToQueue, pxQueue->uxItemSize );
*/
					
				vTaskPlaceOnEventList( myCpu, &( pxQueue->xTasksWaitingToSend ), xTicksToWait );
				
				/* Unlocking the queue means queue events can effect the
				event list.  It is possible	that interrupts occurring now
				remove this task from the event	list again - but as the
				scheduler is suspended the task will go onto the pending
				ready last instead of the actual ready list. */
				prvUnlockQueue( pxQueue );

				/* Resuming the scheduler will move tasks from the pending
				ready list into the ready list - so it is feasible that this
				task is already in a ready list before it yields - in which
				case the yield will not cause a context switch unless there
				is also a higher priority task in the pending ready list. */
	
				if ( !xTaskResumeCpu( myCpu ) )
				{
					portYIELD( myCpu );
				}
			}
			else
			{
				/* Try again. */
				prvUnlockQueue( pxQueue );
				xTaskResumeCpu( myCpu );
			}
		}
		else
		{
			/* The timeout has expired. */
			prvUnlockQueue( pxQueue );
			xTaskResumeCpu( myCpu );

			/* Return to the original privilege level before exiting the
			function. */
			traceQUEUE_SEND_FAILED( pxQueue );
			return errQUEUE_FULL;
		}
	}
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xQueueGenericSendFromISR( xQueueHandle pxQueue, const void * const pvItemToQueue, 
												signed portBASE_TYPE *pxHigherPriorityTaskWoken,
												portSHORT *psCpuNo,
												portBASE_TYPE xCopyPosition  )
{
	signed portBASE_TYPE xReturn;
	

	/* Similar to xQueueGenericSend, except we don't block if there is no room
	in the queue.  Also we don't directly wake a task that was blocked on a
	queue read, instead we return a flag to say whether a context switch is
	required or not (i.e. has a task with a higher priority than us been woken
	by this	post). */
	
	portENTER_CRITICAL( &pxQueue->sLock, pdTRUE );
	{	
		// if( pxQueue->uxMessagesWaiting < pxQueue->uxLength )
		// if this is a binary semaphore: uxItemSize == 0 && pvItemToQueue == NULL
		// or there's room in the queue.
		if ( ((pxQueue->uxItemSize == 0) && (pvItemToQueue == NULL)) || 
			(atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting ) < pxQueue->uxLength) )
		{
			traceQUEUE_SEND_FROM_ISR( pxQueue );

			prvCopyDataToQueue( pxQueue, pvItemToQueue, xCopyPosition, pdTRUE );

			/* If the queue is locked we do not alter the event list.  This will
			be done when the queue is unlocked later. */
			if( (INT32) atomic32Get( (UINT32 *) &pxQueue->xTxLock ) == queueUNLOCKED )
			{	
				if( !listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) )
				{	
					if( xTaskRemoveFromEventList( psCpuNo, &(pxQueue->xTasksWaitingToReceive), pdTRUE ) != pdFALSE )
					{
						/* The task waiting has a higher priority so record that a
						context	switch is required. */
						*pxHigherPriorityTaskWoken = pdTRUE;
					}
				}
			}
			else
			{
				/* Increment the lock count so the task that unlocks the queue
				knows that data was posted while it was locked. */
				atomic32Inc( (UINT32 *) &pxQueue->xTxLock );
			}
			
			xReturn = pdPASS;
		}
		else
		{
			traceQUEUE_SEND_FROM_ISR_FAILED( pxQueue );
			xReturn = errQUEUE_FULL;
		}
	}
	portEXIT_CRITICAL( &pxQueue->sLock, pdTRUE );

	return xReturn;
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xQueueGenericReceive( xQueueHandle pxQueue, void * const pvBuffer, 
											portTickType xTicksToWait, portBASE_TYPE xJustPeeking )
{
	signed portBASE_TYPE xEntryTimeSet = pdFALSE;
	xTimeOutType xTimeOut;
	signed char *pcOriginalReadPosition;
	portSHORT myCpu;
	portSHORT eventCpuNo;
	signed portBASE_TYPE xYieldRequired;
	

	/* This function relaxes the coding standard somewhat to allow return
	statements within the function itself.  This is done in the interest
	of execution time efficiency. */
	
	myCpu = sPortGetCurrentCPU();
	eventCpuNo = myCpu;
	
	/*	
	if (pvBuffer != NULL)
	{
		DBLOG( "%s %04d: cpuNo: %u pxQueue: 0x%08x %u pvBuffer: 0x%08x uxItemSize: %u\n", 
					__FUNCTION__, __LINE__, myCpu, (UINT32) pxQueue, (UINT32) pvBuffer, pxQueue->uxItemSize );
	}
*/
	
	for( ;; )
	{
		portENTER_CRITICAL( &pxQueue->sLock, pdFALSE );
		{
			/* Is there data in the queue now?  To be running we must be
			the highest priority task wanting to access the queue. */
			
			if( atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting ) > 0 )
			{	
				xYieldRequired = pdFALSE;
				
				if( xJustPeeking == pdFALSE )
				{
					/* We are actually removing data. */
					atomic32Dec( (UINT32 *) &pxQueue->uxMessagesWaiting );
					
					prvCopyDataFromQueue( pxQueue, pvBuffer );
					
					traceQUEUE_RECEIVE( pxQueue );

					#if ( configUSE_MUTEXES == 1 )
					{
						if( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX )
						{
							/* Record the information required to implement
							priority inheritance should it become necessary. */
							pxQueue->pxMutexHolder = xTaskGetCurrentTaskHandle();
						}
					}
					#endif
					
					if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) == pdFALSE )
					{
/*
						if (pvBuffer != NULL)
						{
							DBLOG( "%s %04d: cpuNo: %u pxQueue: 0x%08x %u pvBuffer: 0x%08x uxItemSize: %u\n", 
										__FUNCTION__, __LINE__, myCpu, (UINT32) pxQueue, (UINT32) pvBuffer, pxQueue->uxItemSize );
						}
*/
						/* Make next task waiting to send ready to run */
						xYieldRequired = xTaskRemoveFromEventList( &eventCpuNo, &(pxQueue->xTasksWaitingToSend), pdFALSE );
					}
				}
				else
				{				
					/* Remember our read position as we are just peeking. */
					pcOriginalReadPosition = pxQueue->pcReadFrom;

					prvCopyDataFromQueue( pxQueue, pvBuffer );
				
					traceQUEUE_PEEK( pxQueue );

					/* We are not removing the data, so reset our read
					pointer. */
					pxQueue->pcReadFrom = pcOriginalReadPosition;

					/* The data is being left in the queue, so see if there are
					any other tasks waiting for the data. */
					if( !listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) )
					{
						/* Tasks that are removed from the event list will get added to
						the pending ready list as the scheduler is still suspended. */
						xYieldRequired = xTaskRemoveFromEventList( &eventCpuNo, &(pxQueue->xTasksWaitingToReceive), pdFALSE );
					}
				}
				
				portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );
				
				if (xYieldRequired == pdTRUE)
				{
					portYIELD( eventCpuNo );
				}

				return pdPASS;
			}
			else
			{			
				if( xTicksToWait == ( portTickType ) 0 )
				{
					/* The queue was empty and no block time is specified (or
					the block time has expired) so leave now. */
					
					portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );
					
					traceQUEUE_RECEIVE_FAILED( pxQueue );
					return errQUEUE_EMPTY;
				}
				else if( xEntryTimeSet == pdFALSE )
				{
					/* The queue was empty and a block time was specified so
					configure the timeout structure. */
					vTaskSetTimeOutState( myCpu, &xTimeOut );
					xEntryTimeSet = pdTRUE;
				}
			}
		}
		portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );
		

		/* Interrupts and other tasks can send to and receive from the queue
		now the critical section has been exited. */

		portENTER_CRITICAL( &pxQueue->sLock, pdFALSE );
		vTaskSuspendCpu( myCpu );
		prvLockQueue( pxQueue );
		portEXIT_CRITICAL( &pxQueue->sLock, pdFALSE );

		/* Update the timeout state to see if it has expired yet. */
		if( xTaskCheckForTimeOut( myCpu, &xTimeOut, &xTicksToWait ) == pdFALSE )
		{		
			if( prvIsQueueEmpty( pxQueue ) )
			{
				traceBLOCKING_ON_QUEUE_RECEIVE( pxQueue );

				#if ( configUSE_MUTEXES == 1 )
				{
					if( pxQueue->uxQueueType == queueQUEUE_IS_MUTEX )
					{
						vTaskPriorityInherit( ( void * ) pxQueue->pxMutexHolder, pdFALSE );
					}
				}
				#endif
/*
				DBLOG( "%s %04d: cpuNo: %d pxQueue: 0x%08x pvBuffer: 0x%08x uxItemSize: %u\n", 
						__FUNCTION__, __LINE__, myCpu, (UINT32) pxQueue, (UINT32) pvBuffer, pxQueue->uxItemSize );
*/
				vTaskPlaceOnEventList( myCpu, &( pxQueue->xTasksWaitingToReceive ), xTicksToWait );
				
				prvUnlockQueue( pxQueue );

				if ( !xTaskResumeCpu( myCpu ))
				{
					portYIELD( myCpu );
				}
			}
			else
			{
				/* Try again. */
				prvUnlockQueue( pxQueue );
				xTaskResumeCpu( myCpu );
			}
/*			
			DBLOG( "%s %04d: cpuNo: %d pxQueue: 0x%08x xTicksToWait: %u\n", 
						__FUNCTION__, __LINE__, myCpu, (UINT32) pxQueue, xTicksToWait );
*/				
		}
		else
		{
			prvUnlockQueue( pxQueue );
			xTaskResumeCpu( myCpu );
			traceQUEUE_RECEIVE_FAILED( pxQueue );
/*			
			DBLOG( "%s %04d: cpuNo: %d pxQueue: 0x%08x\n", 
						__FUNCTION__, __LINE__, myCpu, (UINT32) pxQueue );
*/			
			return errQUEUE_EMPTY;
		}
	}
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xQueueReceiveFromISR( xQueueHandle pxQueue, void * const pvBuffer, 
											signed portBASE_TYPE *pxTaskWoken, portSHORT *psCpuNo )
{
	signed portBASE_TYPE xReturn;
	

	portENTER_CRITICAL( &pxQueue->sLock, pdTRUE );
	{
		/* Check if there is data available. */
		if( atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting ) > ( unsigned portBASE_TYPE ) 0 )
		{
			traceQUEUE_RECEIVE_FROM_ISR( pxQueue );
			
			/* We are actually removing data. */
			prvCopyDataFromQueue( pxQueue, pvBuffer );
			
			atomic32Dec( (UINT32 *) &pxQueue->uxMessagesWaiting );

			/* If the queue is locked we will not modify the event list.  Instead
			we update the lock count so the task that unlocks the queue will know
			that an ISR has removed data while the queue was locked. */
			if( (INT32) atomic32Get( (UINT32 *) &pxQueue->xRxLock ) == queueUNLOCKED )
			{
				if( !listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToSend ) ) )
				{
					/* Make next task waiting to send ready to run */
					if( xTaskRemoveFromEventList( psCpuNo, &(pxQueue->xTasksWaitingToSend), pdTRUE ) != pdFALSE )
					{
						/* The task waiting has a higher priority so
						indicate a context switch. */
						*pxTaskWoken = pdTRUE;
					}
				}
			}
			else
			{
				/* Increment the lock count so the task that unlocks the queue
				knows that data was removed while it was locked. */
				atomic32Inc( (UINT32 *) &pxQueue->xRxLock );
			}
			
			xReturn = pdPASS;
		}
		else
		{
			xReturn = pdFAIL;
			traceQUEUE_RECEIVE_FROM_ISR_FAILED( pxQueue );
		}
	}
	portEXIT_CRITICAL( &pxQueue->sLock, pdTRUE );

	return xReturn;
}
/*-----------------------------------------------------------*/

xQueueHandle xQueueCreate( unsigned portBASE_TYPE uxQueueLength, unsigned portBASE_TYPE uxItemSize )
{
	xQUEUE *pxNewQueue;
	size_t xQueueSizeInBytes;


	/* Allocate the new queue structure. */
	if( uxQueueLength > ( unsigned portBASE_TYPE ) 0 )
	{
		pxNewQueue = ( xQUEUE * ) pvPortMalloc( sizeof( xQUEUE ) );
		if( pxNewQueue != NULL )
		{
			/* Create the list of pointers to queue items.  The queue is one byte
			longer than asked for to make wrap checking easier/faster. */
			xQueueSizeInBytes = ( size_t ) ( uxQueueLength * uxItemSize ) + ( size_t ) 1;

			pxNewQueue->pcHead = ( signed char * ) pvPortMalloc( xQueueSizeInBytes );
			if( pxNewQueue->pcHead != NULL )
			{
				/* Initialise the queue members as described above where the
				queue type is defined. */
				pxNewQueue->pcTail = pxNewQueue->pcHead + ( uxQueueLength * uxItemSize );
				pxNewQueue->uxMessagesWaiting = 0;
				pxNewQueue->pcWriteTo = pxNewQueue->pcHead;
				pxNewQueue->pcReadFrom = pxNewQueue->pcHead + ( ( uxQueueLength - 1 ) * uxItemSize );
				pxNewQueue->uxLength = uxQueueLength;
				pxNewQueue->uxItemSize = uxItemSize;
				pxNewQueue->xRxLock = queueUNLOCKED;
				pxNewQueue->xTxLock = queueUNLOCKED;
				pxNewQueue->cpuNo = sPortGetCurrentCPU();
				
				/* Initialize spinlock */
				spinLockInit( &pxNewQueue->sLock );

				/* Likewise ensure the event queues start with the correct state. */
				vListInitialise( &( pxNewQueue->xTasksWaitingToSend ) );
				vListInitialise( &( pxNewQueue->xTasksWaitingToReceive ) );

#ifdef INCLUDE_QUEUE_DEBUG
				xQueueShow( pxNewQueue );
#endif

				traceQUEUE_CREATE( pxNewQueue );
				return  pxNewQueue;
			}
			else
			{
				traceQUEUE_CREATE_FAILED();
				vPortFree( pxNewQueue );
			}
		}
	}

	/* Will only reach here if we could not allocate enough memory or no memory
	was required. */
	return NULL;
}
/*-----------------------------------------------------------*/

#if ( configUSE_MUTEXES == 1 )

	xQueueHandle xQueueCreateMutex( void )
	{
		xQUEUE *pxNewQueue;


		/* Allocate the new queue structure. */
		pxNewQueue = ( xQUEUE * ) pvPortMalloc( sizeof( xQUEUE ) );
		if( pxNewQueue != NULL )
		{
			/* Information required for priority inheritance. */
			pxNewQueue->pxMutexHolder = NULL;
			pxNewQueue->uxQueueType = queueQUEUE_IS_MUTEX;

			/* Queues used as a mutex no data is actually copied into or out
			of the queue. */
			pxNewQueue->pcWriteTo = NULL;
			pxNewQueue->pcReadFrom = NULL;

			/* Each mutex has a length of 1 (like a binary semaphore) and
			an item size of 0 as nothing is actually copied into or out
			of the mutex. */
			pxNewQueue->uxMessagesWaiting = 0;
			pxNewQueue->uxLength = 1;
			pxNewQueue->uxItemSize = 0;
			pxNewQueue->xRxLock = queueUNLOCKED;
			pxNewQueue->xTxLock = queueUNLOCKED;
			pxNewQueue->cpuNo = sPortGetCurrentCPU();
			
			/* Initialize spinlock */
			spinLockInit( &pxNewQueue->sLock );

			/* Ensure the event queues start with the correct state. */
			vListInitialise( &( pxNewQueue->xTasksWaitingToSend ) );
			vListInitialise( &( pxNewQueue->xTasksWaitingToReceive ) );
			
			/* Start with the semaphore in the expected state. */
			xQueueGenericSend( pxNewQueue, NULL, 0, queueSEND_TO_BACK );

			traceCREATE_MUTEX( pxNewQueue );
		}
		else
		{
			traceCREATE_MUTEX_FAILED();
		}

		return pxNewQueue;
	}

#endif /* configUSE_MUTEXES */
/*-----------------------------------------------------------*/


#if configUSE_RECURSIVE_MUTEXES == 1

	portBASE_TYPE xQueueGiveMutexRecursive( xQueueHandle pxMutex )
	{
		portBASE_TYPE xReturn;


		/* If this is the task that holds the mutex then pxMutexHolder will not
		change outside of this task.  If this task does not hold the mutex then
		pxMutexHolder can never coincidentally equal the tasks handle, and as
		this is the only condition we are interested in it does not matter if
		pxMutexHolder is accessed simultaneously by another task.  Therefore no
		mutual exclusion is required to test the pxMutexHolder variable. */
		if( pxMutex->pxMutexHolder == xTaskGetCurrentTaskHandle() )
		{
			traceGIVE_MUTEX_RECURSIVE( pxMutex );

			/* uxRecursiveCallCount cannot be zero if pxMutexHolder is equal to
			the task handle, therefore no underflow check is required.  Also,
			uxRecursiveCallCount is only modified by the mutex holder, and as
			there can only be one, no mutual exclusion is required to modify the
			uxRecursiveCallCount member. */
			( pxMutex->uxRecursiveCallCount )--;

			/* Have we unwound the call count? */
			if( pxMutex->uxRecursiveCallCount == 0 )
			{
				/* Return the mutex.  This will automatically unblock any other
				task that might be waiting to access the mutex. */
				xQueueGenericSend( pxMutex, NULL, queueMUTEX_GIVE_BLOCK_TIME, queueSEND_TO_BACK );
			}

			xReturn = pdPASS;
		}
		else
		{
			/* We cannot give the mutex because we are not the holder. */
			xReturn = pdFAIL;

			traceGIVE_MUTEX_RECURSIVE_FAILED( pxMutex );
		}

		return xReturn;
	}

#endif /* configUSE_RECURSIVE_MUTEXES */
/*-----------------------------------------------------------*/

#if configUSE_RECURSIVE_MUTEXES == 1

	portBASE_TYPE xQueueTakeMutexRecursive( xQueueHandle pxMutex, portTickType xBlockTime )
	{
		portBASE_TYPE xReturn;


		/* Comments regarding mutual exclusion as per those within
		xQueueGiveMutexRecursive(). */

		traceTAKE_MUTEX_RECURSIVE( pxMutex );

		if( pxMutex->pxMutexHolder == xTaskGetCurrentTaskHandle() )
		{
			( pxMutex->uxRecursiveCallCount )++;
			xReturn = pdPASS;
		}
		else
		{
			xReturn = xQueueGenericReceive( pxMutex, NULL, xBlockTime, pdFALSE );

			/* pdPASS will only be returned if we successfully obtained the mutex,
			we may have blocked to reach here. */
			if( xReturn == pdPASS )
			{
				( pxMutex->uxRecursiveCallCount )++;
			}
		}

		return xReturn;
	}

#endif /* configUSE_RECURSIVE_MUTEXES */
/*-----------------------------------------------------------*/

#if configUSE_COUNTING_SEMAPHORES == 1

	xQueueHandle xQueueCreateCountingSemaphore( unsigned portBASE_TYPE uxCountValue, 
												unsigned portBASE_TYPE uxInitialCount )
	{
		xQueueHandle pxHandle;


		pxHandle = xQueueCreate( ( unsigned portBASE_TYPE ) uxCountValue, queueSEMAPHORE_QUEUE_ITEM_LENGTH );

		if( pxHandle != NULL )
		{
			atomic32Set( (UINT32 *) &pxHandle->uxMessagesWaiting, uxInitialCount );

			traceCREATE_COUNTING_SEMAPHORE();
		}
		else
		{
			traceCREATE_COUNTING_SEMAPHORE_FAILED();
		}

		return pxHandle;
	}

#endif /* configUSE_COUNTING_SEMAPHORES */
/*-----------------------------------------------------------*/

unsigned portBASE_TYPE uxQueueMessagesWaiting( const xQueueHandle pxQueue )
{
	return atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting );
}
/*-----------------------------------------------------------*/

unsigned portBASE_TYPE uxQueueMessagesWaitingFromISR( const xQueueHandle pxQueue )
{
	return atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting );
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xQueueIsQueueEmptyFromISR( const xQueueHandle pxQueue )
{
	return ( atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting ) == ( UINT32 ) 0 );
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xQueueIsQueueFullFromISR( const xQueueHandle pxQueue )
{
	return ( atomic32Get( (UINT32 *) &pxQueue->uxMessagesWaiting ) == ( UINT32 ) pxQueue->uxLength );
}
/*-----------------------------------------------------------*/

void vQueueDelete( xQueueHandle pxQueue )
{
	traceQUEUE_DELETE( pxQueue );
	vQueueUnregisterQueue( pxQueue );
	vPortFree( pxQueue->pcHead );
	vPortFree( pxQueue );
}
/*-----------------------------------------------------------*/

#if configQUEUE_REGISTRY_SIZE > 0

	void vQueueAddToRegistry( xQueueHandle xQueue, signed char *pcQueueName )
	{
		unsigned portBASE_TYPE ux;

		/* See if there is an empty space in the registry.  A NULL name denotes
		a free slot. */
		for( ux = 0; ux < configQUEUE_REGISTRY_SIZE; ux++ )
		{
			if( xQueueRegistry[ ux ].pcQueueName == NULL )
			{
				/* Store the information on this queue. */
				xQueueRegistry[ ux ].pcQueueName = pcQueueName;
				xQueueRegistry[ ux ].xHandle = xQueue;
				break;
			}
		}
	}

#endif
	/*-----------------------------------------------------------*/

#if configQUEUE_REGISTRY_SIZE > 0

	static void vQueueUnregisterQueue( xQueueHandle xQueue )
	{
		unsigned portBASE_TYPE ux;

		/* See if the handle of the queue being unregistered in actually in the
		registry. */
		for( ux = 0; ux < configQUEUE_REGISTRY_SIZE; ux++ )
		{
			if( xQueueRegistry[ ux ].xHandle == xQueue )
			{
				/* Set the name to NULL to show that this slot if free again. */
				xQueueRegistry[ ux ].pcQueueName = NULL;
				break;
			}
		}

	}

#endif
