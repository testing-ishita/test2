/*******************************************************************************
*
* Filename: 	port.c
* 
* Description:	FreeRTOS port functions, adapted for use with CCT boards.
*
* $Revision: 1.3 $
*
* $Date: 2013-09-26 10:44:06 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/FreeRTOS/Source/portable/GCC/CUTE/port.c,v $
*
* Copyright 2011-2012 Concurrent Technologies, Plc.
* 
* FreeRTOS V6.0.4 - Copyright (C) 2010 Real Time Engineers Ltd.
*
*******************************************************************************/


/* Includes */
#include <stdlib.h>

#include <private/debug.h>
#include <private/port_io.h>
#include <private/sys_delay.h>
#include <private/semaphore.h>
#include <private/atomic.h>

#include "FreeRTOS.h"
#include "task.h"
#include <bit/bit.h>
#include <bit/hal.h>

#include "config.h"
#include "../debug/dbgLog.h"

/* Defines */

#ifdef INCLUDE_DEBUG_VGA
#include "../vgalib/vga.h"
extern int startLine;
#endif

/* Local APIC register offsets */
#define LOAPIC_ID                       0x020   /* Local APIC ID Reg */
#define LOAPIC_VER                      0x030   /* Local APIC Version Reg */
#define LOAPIC_TPR                      0x080   /* Task Priority Reg */
#define LOAPIC_APR                      0x090   /* Arbitration Priority Reg */
#define LOAPIC_PPR                      0x0a0   /* Processor Priority Reg */
#define LOAPIC_LDR                      0x0d0   /* Logical Destination Reg */
#define LOAPIC_DFR                      0x0e0   /* Destination Format Reg */
#define LOAPIC_SVR                      0x0f0   /* Spurious Interrupt Reg */
#define LOAPIC_ISR                      0x100   /* In-service Reg */
#define LOAPIC_TMR                      0x180   /* Trigger Mode Reg */
#define LOAPIC_IRR                      0x200   /* Interrupt Request Reg */
#define LOAPIC_ESR                      0x280   /* Error Status Reg */
#define LOAPIC_ICRLO                    0x300   /* Interrupt Command Reg */
#define LOAPIC_ICRHI                    0x310   /* Interrupt Command Reg */
#define LOAPIC_TIMER                    0x320   /* LVT (Timer) */
#define LOAPIC_THERMAL                  0x330   /* LVT (Thermal) */
#define LOAPIC_PMC                      0x340   /* LVT (PMC) */
#define LOAPIC_LINT0                    0x350   /* LVT (LINT0) */
#define LOAPIC_LINT1                    0x360   /* LVT (LINT1) */
#define LOAPIC_ERROR                    0x370   /* LVT (ERROR) */
#define LOAPIC_TIMER_ICR                0x380   /* Timer Initial Count Reg */
#define LOAPIC_TIMER_CCR                0x390   /* Timer Current Count Reg */
#define LOAPIC_TIMER_CONFIG             0x3e0   /* Timer Divide Config Reg */

/* Macro to read from Local APIC register */
#define LOAPIC_READ( offset ) \
	*(volatile UINT32 *)(loapicBase + offset)

/* Macro to write to Local APIC register */ 
#define LOAPIC_WRITE( offset, value ) \
	*(volatile UINT32 *)(loapicBase + offset) = value


/* i8254 PIT defines */
#define PIT_BASE_ADR            0x40
#define PIT_TIM2_OFF            0x02
#define PIT_CTL_OFF             0x03
#define PIT_READ_CNT2           0x80
#define PIT_READ_STATE2         0xe8
#define PIT_TIM2_CLK_MASK       0x01
#define PIT_STATE_MASK          0x80
#define PIT_CNT0_MODE3          0x36
#define PIT_CNT2_MODE3          0xb6
#define PIT_CLK                 1193180
#define PIT_LOAPIC_OVHD         10 /* PIT overhead counts to read the local APIC counter */
#define PIT_REMAIN              0xffff - ((PIT_CLK/10) - 0xffff) + PIT_LOAPIC_OVHD


/* 
 * Saves the stack pointer for one task into its TCB, calls 
 * vTaskSwitchContext() to update the TCB being used, then restores the stack 
 * from the new TCB ready to run the task. 
 */
#if 0 // replaced as this corrupts the stack via ecx being changed in vTaskSwitchContext!
#define portSWITCH_CONTEXT()							\
{														\
	asm volatile( "lea %0, %%eax\n\t"					\
				  "movl (%%eax),%%ebx\n\t"				\
				  "movl 12(%%esp),%%ecx\n\t"			\
				  "movl %%esp, (%%ebx)\n\t"				\
				  "call _vTaskSwitchContext\n\t"		\
				  "lea %0, %%eax\n\t"					\
				  "movl (%%eax),%%ebx\n\t"				\
				  "movl (%%ebx),%%esp\n\t"				\
				  "movl  %%ecx, 12(%%esp)\n\t"			\
				  :/*No outputs*/						\
				  :"m" (pxCurrentTCB[cpuNo])			\
				  :"%eax","%ebx","%ecx"					\
			   );										\
}
#endif

/*
 * Load the stack pointer from the TCB of the task which is going to be first
 * to execute.  Then force an IRET so the registers and IP are popped off the
 * stack.
 */
#define  portFIRST_CONTEXT()							\
{														\
	asm volatile( "pushf \n\t"							\
				  "pop %%ax \n\t"						\
				  "and $0xbfff,%%ax \n\t"				\
				  "pushw %%ax \n\t"						\
				  "popf \n\t"							\
				  "lea %0, %%eax\n\t"					\
				  "movl (%%eax),%%ebx\n\t"				\
				  "movl (%%ebx),%%esp\n\t"				\
				  "add $0x4, %%esp \n\t"				\
				  "popl %%ebx\n\t"						\
				  "popl %%ebp\n\t"						\
				  "add $0x4, %%esp \n\t"				\
				  "popl %%gs\n\t"						\
				  "popl %%fs\n\t"						\
				  "popl %%es\n\t"						\
				  "popl %%ds\n\t"						\
				  "popal\n\t"							\
				  "iret"								\
				  :/*No outputs*/						\
				  :"m" (pxCurrentTCB[cpuNo])			\
				  :"%eax","%ebx"						\
			   );										\
}


typedef struct
{
	volatile UINT32 contextSwitchSuspended;	/* Scheduler running on this CPU */
	volatile UINT32 tickCount;				/* Tick count */
	volatile UINT32 tickApCount;			/* Tick AP count */
	volatile UINT32 yieldCpu;				/* Signal yield */
	volatile UINT32 yieldCount;				/* Yield count */
	volatile UINT32 yieldIntCount;			/* Yield interrupt count */
	UINT32	spare1;
	UINT32	spare2;
	
} PORT_CPU_STATUS;

/* Imports */

typedef void tskTCB;
extern volatile tskTCB * pxCurrentTCB[];

extern int sysIpiTick( UINT8 bApicId, UINT8 bVector );
extern void sysIntEnablePIC( int intLevel );
extern void sysIntDisablePIC( int intLevel );

extern UINT8 bCpuConfigured;
extern UINT8 abApicId[];
extern UINT8 lapicGetId( void );

/* Globals */

/* Locals */

static PORT_CPU_STATUS cpuStatus[configMAX_CPU_NUM];

#ifndef USE_PIC
static UINT32 loapicBase = 0xFEE00000;
#endif

#ifdef INCLUDE_PORT_DEBUG
#warning "***** DEBUG ON *****" 
#endif


#ifdef USE_PIC
#warning "***** APIC NOT INCLUDED *****" 
/*******************************************************************************
*
* prvSetTickFrequency
*
* Set tick frequency using PIT.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void prvSetTickFrequency( UINT32 ulTickRateHz )
{
	UINT32 ulOutput;

	/* Setup the 8245 to tick at the wanted frequency. */
	sysOutPort8( (PIT_BASE_ADR + PIT_CTL_OFF), PIT_CNT0_MODE3 );
	
	ulOutput = ((UINT32) PIT_CLK / ulTickRateHz);
   	sysOutPort8( PIT_BASE_ADR, (UINT8)(ulOutput & 0xff) );
	ulOutput >>= 8;
	sysOutPort8( PIT_BASE_ADR, (UINT8)(ulOutput & 0xff) );
}
#endif /* USE_PIC */


/*******************************************************************************
*
* sPortGetCurrentCPU
*
* Get the current CPU number.
* 
* 
*
* RETURNS: Current CPU number.
*
*******************************************************************************/
portSHORT sPortGetCurrentCPU( void )
{
#ifndef USE_PIC
	UINT8 bApicId;
	portSHORT cpuNo;
	

	/* Get Local APIC ID */
	bApicId = lapicGetId();

	/* Try to match APIC ID to get CPU number */
	for (cpuNo = 0; cpuNo < bCpuConfigured; cpuNo++ )
	{
		if (abApicId[cpuNo] == bApicId)
		{
			return cpuNo;
		}
	}
#endif

	/* Not found, default to Boot processor */
	return 0;
}


/*******************************************************************************
*
* vPortYieldCPU
*
* Trigger task yield on given CPU number.
* 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vPortYieldCPU( portSHORT cpuNo )
{
	portSHORT currCpuNo;

	
	currCpuNo = sPortGetCurrentCPU();
	
	if (cpuNo < bCpuConfigured)
	{
		/* Set yield flag for this CPU */
		atomic32Set( (UINT32 *) &cpuStatus[cpuNo].yieldCpu, (UINT32) 1 );
		atomic32Inc( (UINT32 *) &cpuStatus[cpuNo].yieldCount );	
		
		/* Generate yield interrupt */
		if (cpuNo != currCpuNo)
		{
			sysSendIpi( abApicId[cpuNo], portSWITCH_INT_NUMBER );
		}
		else
		{
			asm volatile ( "int $0x41" );
		}
	}
}


/*******************************************************************************
*
* vPortSetContextSwitching
*
* Turn context switching on/off. When contextSwitchSuspended == pdFALSE then
* context switching is on.
* 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vPortSetContextSwitching( portSHORT cpuNo, int value )
{
	if (cpuNo < bCpuConfigured)
	{
		if (value)
		{
			atomic32Dec( (UINT32 *) &cpuStatus[cpuNo].contextSwitchSuspended );
		}
		else
		{
			atomic32Inc( (UINT32 *) &cpuStatus[cpuNo].contextSwitchSuspended );
		}
	}
}


#ifndef USE_PIC
/*******************************************************************************
*
* prvGenApTick
*
* Generate tick IPI for AP's.
* 
* 
*
* RETURNS: None.
*
*******************************************************************************/
static void prvGenApTick( void )
{
	portSHORT cpuNo;
	
	/* Generate tick IPI for AP's */
	for (cpuNo = 1; cpuNo < bCpuConfigured; cpuNo++ )
	{
		sysIpiTick( abApicId[cpuNo], portAP_TICK_INT_NUMBER );
		atomic32Inc( (UINT32 *) &cpuStatus[cpuNo].tickApCount );
	}
}
#endif


/*******************************************************************************
*
* prvPreemptiveTick
*
* Tick service routine used by the scheduler when preemptive scheduling is
* being used.
* 
* Note: the adjustment made for the stack pointer assumes the compiler allowed
*       24 or 40 bytes for local variables etc, at the begining of this function. 
*       If changes are made to the contents of this function, the value used
*       here may need to be increased/decreased. Check the assembly listing
*       file after compilation to determine the new value, then change the
*       value used below and re-compile.
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void prvPreemptiveTick( void )
{
	volatile portSHORT cpuNo;
	volatile tskTCB *pTCB;

	
	/* On entry: */
	/* pushl %ebp */
  	/* movl	%esp, %ebp */
  	/* subl	$24, %esp */
	
	cpuNo = sPortGetCurrentCPU();
	
	if ( atomic32Get( (UINT32 *) &cpuStatus[cpuNo].contextSwitchSuspended ) == (UINT32) pdFALSE)
	{
		pTCB = pxCurrentTCB[cpuNo];
		
		/* Get and save the stack pointer into the current TCB */
		asm volatile ( "movl %esp,%ecx" );										/* Get stack pointer and adjust for the subl added */

#if (__GNUC__ == 3 && __GNUC_MINOR__ == 4)
		asm volatile ( "addl $24, %ecx" );										/* by the compiler at the begining of this function */
#elif (__GNUC__ == 4 && ((__GNUC_MINOR__ == 5) || (__GNUC_MINOR__ == 7)))
		asm volatile ( "addl $40, %ecx" );										/* by the compiler at the begining of this function */
#else
#error "unsupported GCC version found, GCC 3.4.4 or 4.5.3 or 4.7.3 required"
#endif		
		asm volatile ( "lea %0, %%eax": :"m" (pxCurrentTCB[cpuNo]) : "%eax" ); 	/* Get address of current TCB pointer */
		asm volatile ( "movl (%eax),%ebx" );									/* Get current TCB address */
		asm volatile ( "movl %ecx, (%ebx)" );									/* Save the stack pointer into TCB. */	
		
		atomic32Inc( (UINT32 *) &cpuStatus[cpuNo].tickCount );
		
		if ( cpuNo == 0 )
		{
#ifndef USE_PIC
	
#ifdef INCLUDE_PORT_DEBUG
			if ( (cpuStatus[cpuNo].tickCount == 1) || (cpuStatus[cpuNo].tickCount % 1000 == 0) )
			{
				sysOutPort8( 0x61, sysInPort8( 0x61 ) ^ 0x2 ); /* Toggle POST LED */
			}
#endif
			/* Generate tick for AP's */
			prvGenApTick();
#endif
		}
	
		/* Increment tick and check for context switch */
		taskENTER_CRITICAL( cpuNo, pdTRUE );
		vTaskIncrementTick( cpuNo );
		vTaskSwitchContext( cpuNo );
		taskEXIT_CRITICAL( cpuNo, pdTRUE );
		
		/* If a context switch occurred */
		if (pxCurrentTCB[cpuNo] != pTCB)
		{
			/* Get and update the stack pointer from the (switched) TCB */
			asm volatile ( "lea %0, %%eax": :"m" (pxCurrentTCB[cpuNo]) : "%eax" ); 	/* Get address of current TCB pointer */
			asm volatile ( "movl (%eax),%ebx" );									/* Get current TCB address */
			asm volatile ( "movl (%ebx),%esp" );									/* Get the stack pointer from the TCB. */
		
			/* If this is the first time this task has been switched in */
			if (vTaskGetFirstExecute( pxCurrentTCB[cpuNo] ) == pdTRUE)
			{
				/* Clear first execute flag */
				vTaskSetFirstExecute( pxCurrentTCB[cpuNo], pdFALSE );
				
				/* pxPortInitialiseStack() configured the intial stack, */
				/* so we just need to setup the return address and then return */
				asm volatile ( "add $4, %esp" );
				asm volatile ( "movl 4(%ebp),%ecx" );		/* Get our return address (calling exception handler) */
				asm volatile (  "movl  %ecx, 8(%esp)" );	/* Put our return address on the stack */
				asm volatile ( "popl	%ebx" );
				asm volatile ( "popl	%ebp" );
				asm volatile ( "ret" );
			}
			else
			{
				/* Resume task from last running state */
				asm volatile ( "popl	%ebp" );
				asm volatile ( "ret" );
			}
		}
	}
	
	/* No context switch so we return with the existing context: */
	/* leave */
	/* ret */
}


/*******************************************************************************
*
* prvYieldProcessor
*
* Yield processor to another task.
* 
* Note: the adjustment made for the stack pointer assumes the compiler allowed
*       24 or 40 bytes for local variables etc, at the begining of this function. 
*       If changes are made to the contents of this function, the value used
*       here may need to be increased/decreased. Check the assembly listing
*       file after compilation to determine the new value, then change the
*       value used below and re-compile.
*
* RETURNS: N/A.
*
*******************************************************************************/
static void prvYieldProcessor( void )
{
	volatile portSHORT cpuNo;
	volatile tskTCB *pTCB;

	
	/* On entry: */
	/* pushl %ebp */
  	/* movl	%esp, %ebp */
  	/* subl	$24, %esp */
  	
	cpuNo = sPortGetCurrentCPU();
	
	if ( atomic32Get( (UINT32 *) &cpuStatus[cpuNo].contextSwitchSuspended ) == (UINT32) pdFALSE)
	{	
		pTCB = pxCurrentTCB[cpuNo];
			
		/* Get and save the stack pointer into the current TCB */	
		asm volatile ( "movl %esp,%ecx" );										/* Get stack pointer and adjust for the subl added */

#if (__GNUC__ == 3 && __GNUC_MINOR__ == 4)
		asm volatile ( "addl $24, %ecx" );										/* by the compiler at the begining of this function */
#elif (__GNUC__ == 4 && ((__GNUC_MINOR__ == 5) || (__GNUC_MINOR__ == 7)))
		asm volatile ( "addl $40, %ecx" );										/* by the compiler at the begining of this function */
#else
#error "unsupported GCC version found, GCC 3.4.4 or 4.5.3 or 4.7.3 required"
#endif
		asm volatile ( "lea %0, %%eax": :"m" (pxCurrentTCB[cpuNo]) : "%eax" ); 	/* Get address of current TCB pointer */
		asm volatile ( "movl (%eax),%ebx" );									/* Get current TCB address */
		asm volatile ( "movl %ecx, (%ebx)" );									/* Save the stack pointer into TCB. */
			
		/* Get and clear yield flag for this CPU */
		if ( atomic32Set( (UINT32 *) &cpuStatus[cpuNo].yieldCpu, (UINT32) 0 ) != 0 )
		{	
			atomic32Inc( (UINT32 *) &cpuStatus[cpuNo].yieldIntCount );
			
			/* Check for context switch */
			taskENTER_CRITICAL( cpuNo, pdTRUE );
			vTaskSwitchContext( cpuNo );
			taskEXIT_CRITICAL( cpuNo, pdTRUE );
			
			/* If a context switch occurred */
			if (pxCurrentTCB[cpuNo] != pTCB)
			{
				/* Get and update the stack pointer from the (switched) TCB */
				asm volatile ( "lea %0, %%eax": :"m" (pxCurrentTCB[cpuNo]) : "%eax" ); 	/* Get address of current TCB pointer */
				asm volatile ( "movl (%eax),%ebx" );									/* Get current TCB address */
				asm volatile ( "movl (%ebx),%esp" );									/* Get the stack pointer from the TCB. */
			
				/* If this is the first time this task has been switched in */
				if (vTaskGetFirstExecute( pxCurrentTCB[cpuNo] ) == pdTRUE)
				{
					/* Clear first execute flag */
					vTaskSetFirstExecute( pxCurrentTCB[cpuNo], pdFALSE );
					
					/* pxPortInitialiseStack() configured the intial stack, */
					/* so we just need to setup the return address and then return */
					asm volatile ( "add $4, %esp" );
					asm volatile ( "movl 4(%ebp),%ecx" );		/* Get our return address (calling exception handler) */
					asm volatile ( "movl %ecx, 8(%esp)" );		/* Put our return address on the stack */
					asm volatile ( "popl %ebx" );
					asm volatile ( "popl %ebp" );
					asm volatile ( "ret" );
				}
				else
				{
					/* Resume task from last running state */
					asm volatile ( "popl %ebp" );
					asm volatile ( "ret" );
				}
			}
		}
	}
	
	/* No context switch so we return with the existing context: */
	/* leave */
	/* ret */
}


/*******************************************************************************
*
* vPortEndScheduler
*
* Undo any hardware/ISR setup that was performed by xPortStartScheduler() so
* the hardware is left in its original condition after the scheduler stops
* executing.
*
* RETURNS: N/A.
*
*******************************************************************************/
void vPortEndScheduler( void )
{
#ifdef USE_PIC

	atomic32Set( (UINT32 *) &cpuStatus[0].contextSwitchSuspended, (UINT32) pdTRUE );
	sysIntDisablePIC( 0 );
	
#else
	UINT32 regValue;
	portSHORT cpuNo;
	
	for (cpuNo = 0; cpuNo < bCpuConfigured; cpuNo++ )
	{
		atomic32Set( (UINT32 *) &cpuStatus[cpuNo].contextSwitchSuspended, (UINT32) pdTRUE );
	}
	
	/* Stop timer by masking the timer interrupt */
	regValue = LOAPIC_READ( LOAPIC_TIMER );
	regValue |= 0x00010000;
	LOAPIC_WRITE( LOAPIC_TIMER, regValue ) ;
	LOAPIC_WRITE( LOAPIC_TIMER_ICR, 0 );
		
#endif
			
}


#ifndef USE_PIC
/*******************************************************************************
*
* prvLoapicTimerConfig
*
* Configure the local APIC timer for out tick rate.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void prvLoapicTimerConfig( UINT32 clkFrequency )
{
	UINT32 cpuNo;
	UINT32 regValue;
	
	
	cpuNo = (UINT32) sPortGetCurrentCPU();
	
	if ((cpuNo == 0) && (clkFrequency > 0) )
	{
		/* Set a divide value in the configuration register */
		regValue = LOAPIC_READ( LOAPIC_TIMER_CONFIG );
		regValue = (regValue & (UINT32)~0xf) | 0xb; /* divide by 1 */
		LOAPIC_WRITE( LOAPIC_TIMER_CONFIG, regValue );
		
		/* Set mode and the vector in the local vector table */
		regValue = LOAPIC_READ( LOAPIC_TIMER );
		regValue = (regValue & 0xfffcff00);
		regValue |= 0x00020000 | portTICK_INT_NUMBER; /* auto reload/periodic */
		LOAPIC_WRITE( LOAPIC_TIMER, regValue );
		
		/* Set the initial count register value */
		/* maxTimerCount = (clkFrequency / ticksPerSecond) - 1 */  
		regValue = (clkFrequency / configTICK_RATE_HZ) - 1; 
		LOAPIC_WRITE( LOAPIC_TIMER_ICR, regValue );
	}
	else
	{	
		/* Timer not used, so just mask the timer interrupt */
		regValue = LOAPIC_READ( LOAPIC_TIMER );
		regValue |= 0x00010000;
		LOAPIC_WRITE( LOAPIC_TIMER, regValue );
		LOAPIC_WRITE( LOAPIC_TIMER_ICR, 0 );			
	}			  
}


/*******************************************************************************
*
* prvLoapicIntrConfig
*
* Perform local APIC timer interrupt configuration.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void prvLoapicIntrConfig( void )
{	
	UINT32 cpuNo;
	
	
	cpuNo = (UINT32) sPortGetCurrentCPU();
	
	if (cpuNo > 0)
	{
		/* LINT0 & LINT1 */
		LOAPIC_WRITE( LOAPIC_LINT0, 0x00000700 );
		LOAPIC_WRITE( LOAPIC_LINT1, 0x00000400 );		
	}
	
	/* Set Logical Destination Register to support broadcasts... */
	LOAPIC_WRITE( LOAPIC_LDR, ((UINT32)( 1 << (cpuNo + 24))) );
}



/*******************************************************************************
*
* prvLoapicGetClkFreq
*
* Calculate the local APIC counter frequency using i8254 PIT as a reference.
* 
* 
*
* RETURNS: the local APIC counter frequency.
*
*******************************************************************************/
static UINT32 prvLoapicGetClkFreq( void )
{
	UINT32 regValue,remainCount;
    UINT32 startCount,endCount;
    unsigned char currentState, lastState;
    UINT32 maxCountValue;
	
	
	maxCountValue = 0xffffffff;
	
	/* Set the divide value */
	regValue = LOAPIC_READ( LOAPIC_TIMER_CONFIG );
	regValue = (regValue & (UINT32)~0xf) | 0xb; /* divide by 1 */
	LOAPIC_WRITE( LOAPIC_TIMER_CONFIG, regValue );
	
	/* Set the initial local APIC count value */
	LOAPIC_WRITE( LOAPIC_TIMER_ICR, (maxCountValue - 1) );
	
	/* Initialize the PIT timer2 as the referrence clock */
	sysOutPort8( (PIT_BASE_ADR + PIT_CTL_OFF), PIT_CNT2_MODE3 );
	
	/* Set the PIT initial count value to 65535 */
	sysOutPort8( (PIT_BASE_ADR + PIT_TIM2_OFF), 0xff ); /* write the low byte */
	sysOutPort8( (PIT_BASE_ADR + PIT_TIM2_OFF), 0xff ); /* write the high byte */
	
	/* Enable PIT timer 2 clock */
	sysOutPort8( 0x61, (sysInPort8( 0x61 ) | PIT_TIM2_CLK_MASK) );        
	
	/* Read the starting count from the local APIC timer */
	startCount = maxCountValue - LOAPIC_READ( LOAPIC_TIMER_CCR );
	
	/* Poll the PIT status for 1 count period (1 period = from 65535 to 0) */
	lastState = 0x80;    
	
	do
	{
		sysOutPort8( (PIT_BASE_ADR + PIT_CTL_OFF), PIT_READ_STATE2 );
		currentState = sysInPort8( (PIT_BASE_ADR + PIT_TIM2_OFF) ) & PIT_STATE_MASK;
	}
	while(lastState == currentState);
	
	/* Poll the PIT counter until 50ms is reached */
	do
	{
		sysOutPort8( (PIT_BASE_ADR + PIT_CTL_OFF), PIT_READ_CNT2 );
		remainCount = sysInPort8( (PIT_BASE_ADR + PIT_TIM2_OFF) );
		remainCount += sysInPort8( (PIT_BASE_ADR + PIT_TIM2_OFF) ) << 8;
	} 
	while (remainCount > PIT_REMAIN);
	
	/* Read the ending count from the local APIC timer */
	endCount = maxCountValue - LOAPIC_READ( LOAPIC_TIMER_CCR ); 
	
	/* Disable PIT timer 2 clock */
	sysOutPort8( 0x61, (sysInPort8( 0x61 ) & ~PIT_TIM2_CLK_MASK) );
	
	/* Calculate and return the clock rate of local APIC timer */	
   	return (UINT32)((endCount - startCount) * 20);
}


/*******************************************************************************
*
* xPortStartAPScheduling
*
* Start AP scheduling.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void xPortStartAPScheduling( void )
{	
	portSHORT cpuNo;

	
	cpuNo = sPortGetCurrentCPU();
	
	if (cpuNo > 0)
	{
		prvLoapicIntrConfig();
		
		prvLoapicTimerConfig( 0 );
		
		vTaskSetFirstExecute( pxCurrentTCB[cpuNo], pdFALSE );
		
		vTaskStartSchedulerRunning( cpuNo );
		
		atomic32Dec( (UINT32 *) &cpuStatus[cpuNo].contextSwitchSuspended );
		
		/* Kick off the scheduler by setting up the */
		/* context of the first task on this CPU.   */
		portFIRST_CONTEXT();
	}
}
#endif /* USE_PIC */


/*******************************************************************************
*
* xPortStartScheduler
*
* Start the scheduler.
* 
* 
*
* RETURNS: pdTRUE.
*
*******************************************************************************/
portBASE_TYPE xPortStartScheduler( void )
{
	portSHORT cpuNo;
	UINT32 clkFrequency;
	int	iVector;
	
#ifdef USE_PIC
	int intLevel;	
#else
	UINT8 i;
#endif

#if defined(INCLUDE_PORT_DEBUG) || defined(INCLUDE_DEBUG_VGA)
	char achBuffer[80];
#endif
	
	cpuNo = sPortGetCurrentCPU();
	
#ifdef USE_PIC
	clkFrequency = PIT_CLK;
	intLevel = 0; /* System tick is IRQ0 */
	iVector = sysPinToVector( intLevel, SYS_IOAPIC0 );
#else
	clkFrequency = prvLoapicGetClkFreq();
	iVector = portTICK_INT_NUMBER;
#endif

	
#if defined(INCLUDE_PORT_DEBUG) || defined(INCLUDE_DEBUG_VGA)
#ifdef USE_PIC
	sprintf( achBuffer, "CPU:%d %u MHz, PIT clock: %u\n", cpuNo, 
				sysGetCpuFrequency(), clkFrequency );
#else	
	sprintf( achBuffer, "CPU:%d %u MHz, APIC clock: %u\n", cpuNo, 
				sysGetCpuFrequency(), clkFrequency );
#endif
#ifdef INCLUDE_PORT_DEBUG
	DBLOG( achBuffer );
#endif

#ifdef INCLUDE_DEBUG_VGA
	startLine++;
	vgaPutsXY( 0, startLine++, achBuffer );
	sprintf( achBuffer, "System tick: %lu ms (%lu Hz) Vector: 0x%x\n", 
				portTICK_RATE_MS, configTICK_RATE_HZ, iVector );
	vgaPutsXY( 0, startLine++, achBuffer );
#endif
#endif /* INCLUDE_PORT_DEBUG || INCLUDE_DEBUG_VGA */

#ifdef USE_PIC
	/* Install interrupt handlers */
	sysInstallUserHandler( iVector, prvPreemptiveTick );
	sysInstallUserHandler( portSWITCH_INT_NUMBER, prvYieldProcessor );
	
	/* This is called with interrupts already disabled. */
	prvSetTickFrequency( configTICK_RATE_HZ );
	
	/* Enable System tick interrupt */
	sysIntEnablePIC( intLevel );
#else
	/* Install interrupt handlers */
	sysInstallUserHandler( iVector, prvPreemptiveTick );						// timer tick 			vector 0x41
    sysInstallUserHandler( portSWITCH_INT_NUMBER, prvYieldProcessor ); 			// task yield 			vector 0x42
    sysInstallUserHandler( portSTART_AP_INT_NUMBER, xPortStartAPScheduling );	// start AP sheduling	vector 0x43
    sysInstallUserHandler( portAP_TICK_INT_NUMBER, prvPreemptiveTick );			// AP tick 				vector 0x44
	
	/* Configure Local APIC interrupt handling */
	prvLoapicIntrConfig();
	
	/* Configure Local APIC timer for our tick rate */
	prvLoapicTimerConfig( clkFrequency );
#endif

	DBLOG( "Starting tasks...\n" );

	vTaskStartSchedulerRunning( cpuNo );
	vTaskSetFirstExecute( pxCurrentTCB[cpuNo], pdFALSE );

#ifndef USE_PIC	
	/* Start scheduling on the AP's */
	for (i = 1; i < bCpuConfigured; i++ )
	{
		sysSendIpi( abApicId[i], portSTART_AP_INT_NUMBER );
		
		while( atomic32Get( (UINT32 *) &cpuStatus[i].contextSwitchSuspended ) != (UINT32) pdFALSE );
	}
#endif

	atomic32Dec( (UINT32 *) &cpuStatus[0].contextSwitchSuspended );
	
	/* Kick off the scheduler by setting up the */
	/* context of the first task on this CPU.   */
	portFIRST_CONTEXT();

	
	return pdTRUE;
}


/*******************************************************************************
*
* vPortDataInit
*
* Initialize our local data.
* 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vPortDataInit( void )
{
	UINT8 i;
	
	
	memset( (void *) cpuStatus, pdFALSE, sizeof(cpuStatus));
	
	for (i = 0; i < configMAX_CPU_NUM; i++ )
	{
		atomic32Set( (UINT32 *) &cpuStatus[i].contextSwitchSuspended, (UINT32) pdTRUE );
	}
}

