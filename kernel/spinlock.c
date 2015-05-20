/*******************************************************************************
*
* Filename:	 spinlock.c
*
* Description:	Spinlock functions.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 07:41:29 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/kernel/spinlock.c,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/


#include <private/atomic.h>
#include <private/debug.h>
#include <private/atomic.h>
#include <private/spinlock.h>

#undef SPINLOCK_DEBUG

#ifdef SPINLOCK_DEBUG
#warning "***** DEBUG ON *****" 
#define SPLDBG(x)	do {sysDebugPrintf x;} while(0)
#else
#define SPLDBG(x)
#endif


/* Spinlock status */

#define SPIN_LOCK_EMPTY			0
#define SPIN_LOCK_BUSY			1
#define SPIN_LOCK_INTERESTED	2
#define SPIN_LOCK_ACKNOWLEDGED	3
#define SPIN_LOCK_NOBODY		255


#define SPIN_LOCK_MAX_SPIN   1000

#define MEM_BARRIER_R()		asm volatile ("lfence" ::: "memory")
#define MEM_BARRIER_W()		asm volatile ("sfence" ::: "memory")
#define MEM_BARRIER_RW()	asm volatile ("mfence" ::: "memory")


#define SPIN_INT_LOCK( lkey ) do \
    { \
    	asm volatile ("pushf; cli; popl %0; " \
		      : "=g" (lkey) : : "memory"); \
    } while ((0))
    
#define SPIN_INT_UNLOCK( lkey ) do \
    { \
    	asm volatile ("testl $0x200,%0; jz 0f; sti; 0: " \
		      : : "g" (lkey) : "cc", "memory"); \
    } while ((0))


extern UINT8 bCpuConfigured;

extern void vTaskSetContextSwitching( portSHORT cpuNo, int value );


/*******************************************************************************
*
* spinLockInit
*
* This function initializes the given spinlock.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void spinLockInit( SPINLOCK *pLock )
{
	int i;
	
	if (pLock != NULL)
	{
		pLock->key = 0;
		pLock->owner = SPIN_LOCK_NOBODY;
		
		for (i = 0; i < (int) configMAX_CPU_NUM; i++)
		{
			pLock->cpu[i].status = SPIN_LOCK_EMPTY;
			pLock->cpu[i].nest = 0;
		}
	}
	else
	{
		SPLDBG(("%s: Error - pLock invalid\n", __FUNCTION__));
	}
}


/*******************************************************************************
*
* spinLockTake
*
* This function takes the given spinlock.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void spinLockTake( SPINLOCK *pLock, int fromIsr )
{
	int i;
	UINT32 owner;
	UINT32 key;
	UINT32 myCpu;
	UINT32 lockTaken;
	
	
	if (pLock != NULL)
	{
		myCpu = (UINT32) sPortGetCurrentCPU();
		
		lockTaken = 0;
		
		SPIN_INT_LOCK( key );
	
		/* Increment the nest count */
		pLock->cpu[myCpu].nest++;
		
		/* If nobody has taken the lock, then take it */
		if (atomic32Cas( &pLock->owner, SPIN_LOCK_NOBODY, myCpu ) == 1)
		{
			SPLDBG(("%s: CPU:%u line: %d\n", __FUNCTION__, myCpu, __LINE__));
			pLock->key = key;
			lockTaken = 1;
		}
		else
		{
			/* Indicate we wan't to take the lock */
			atomic32Set( &pLock->cpu[myCpu].status, SPIN_LOCK_INTERESTED );
			
			/* Spin until we successfully take the lock */
			
			do
			{
				i = 0;
				
				do
				{
					owner = atomic32Get( &pLock->owner );
					
					/* If the lock is available now, then take it */
					if (owner == myCpu || ((owner == SPIN_LOCK_NOBODY) &&
						(atomic32Cas( &pLock->owner, SPIN_LOCK_NOBODY, myCpu ) == 1)))
					{
						SPLDBG(("%s: CPU:%u line: %d\n", __FUNCTION__, myCpu, __LINE__));
						pLock->key = key;
						lockTaken = 1;
					}
					else
					{
						i++;
					}
				}
				while ( (i < SPIN_LOCK_MAX_SPIN) && (lockTaken == 0) );
			
				if (lockTaken == 0)
				{
					/* Try to indicate we might be busy with interrupts for while    */
					/* If not then the previous owner must have given the spin lock, */
					/* try again to take it, as this time we should succeed          */
					if (atomic32Cas( &pLock->cpu[myCpu].status, SPIN_LOCK_INTERESTED, SPIN_LOCK_BUSY ) == 1)
					{
						/* We didn't get the lock this time */
						
						/* If not at interrupt level, disable task preemption */
						/* to guard the periods where interrupts are unlocked */
						if (fromIsr == 0)
						{
							vTaskSetContextSwitching( myCpu, pdFALSE );
						}
						
						/* Allow ISR's to run momentarily. This will happen only */ 
						/* if interrupts were originally unlocked on this CPU    */
						SPIN_INT_UNLOCK( key );
						
						SPIN_INT_LOCK( key );
						
						if (fromIsr == 0)
						{
							vTaskSetContextSwitching( myCpu, pdTRUE );
						}
						
						/* Indicate we're ready to take the lock again */
						atomic32Set( &pLock->cpu[myCpu].status, SPIN_LOCK_INTERESTED );
					}
				}
			}
			while (lockTaken == 0);
		}
		
		MEM_BARRIER_RW();
	}
	else
	{
		SPLDBG(("%s: Error - pLock invalid\n", __FUNCTION__));
	}
}


/*******************************************************************************
*
* spinLockGive
*
* This function gives the given spinlock.
*
*
* RETURNS: None.
* 
*******************************************************************************/
void spinLockGive( SPINLOCK *pLock, int fromIsr )
{
	UINT32 myCpu;
	UINT32 nextCpu;
	UINT32 key;
	UINT32 status;
	UINT32 lockTaken;
	
	
	if (pLock != NULL)
	{
		key = pLock->key;
		myCpu = pLock->owner;
		lockTaken = 1;
		
		SPLDBG(("%s: CPU:%u\n", __FUNCTION__, myCpu));
		
		if ( (myCpu != SPIN_LOCK_NOBODY) && (myCpu < configMAX_CPU_NUM) )
		{
			if (pLock->cpu[myCpu].nest > 0)
			{
				pLock->cpu[myCpu].nest--;
			}
			
			if (pLock->cpu[myCpu].nest == 0)
			{
				status = SPIN_LOCK_EMPTY;
			}
			else
			{
				status = SPIN_LOCK_BUSY;
			}
			
			pLock->cpu[myCpu].status = status;
			
			nextCpu = myCpu;
			
			do
			{
				if (nextCpu++ >= (UINT32) bCpuConfigured)
				{
					nextCpu = 0;
				}
				
				if (nextCpu == myCpu)
				{
					/* We're done and nobody else wants the lock */
					nextCpu = SPIN_LOCK_NOBODY;
					lockTaken = 0;
				}
				else
				{
					/* Check if somebody else on the next CPU is ready to take the lock */
					if (atomic32Cas( &pLock->cpu[nextCpu].status, SPIN_LOCK_INTERESTED, SPIN_LOCK_ACKNOWLEDGED ) == 1)
					{
						lockTaken = 0;
					}
				}
			}
			while( lockTaken == 1 );
			
			MEM_BARRIER_RW();
			atomic32Set( &pLock->owner, nextCpu );
		
			/* We no longer own spinlock, so don't touch its members after here */
		
			SPIN_INT_UNLOCK( key );
		}
	}
	else
	{
		SPLDBG(("%s: Error - pLock invalid\n", __FUNCTION__));
	}
}

