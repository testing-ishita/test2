/*******************************************************************************
*
* Filename:	 spinlock.h
*
* Description:	Header file for spinlock functions.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 07:40:42 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/include/private/spinlock.h,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#ifndef __spinlock_h__
#define __spinlock_h__


/* includes */
#include <stdtypes.h>
#include "FreeRTOS.h"


typedef struct
{
	UINT32 status;		/* Status of this CPU's take of the spinlock */ 
	UINT32 nest;		/* Number attempting a take on this CPU */
	
} SPINLOCK_CPUSTATE;

typedef struct
{
	UINT32 owner;		/* CPU number of the owner */
	UINT32 key;			/* The interrupt key for this lock */
	SPINLOCK_CPUSTATE cpu[configMAX_CPU_NUM];
	
} SPINLOCK;


void spinLockInit( SPINLOCK *pLock );
void spinLockTake( SPINLOCK *pLock, int fromIsr );
void spinLockGive( SPINLOCK *pLock, int fromIsr );


#endif /* __spinlock_h__ */
