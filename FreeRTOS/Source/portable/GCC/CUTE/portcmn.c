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


/*-----------------------------------------------------------
 * Components that can be compiled to either ARM or THUMB mode are
 * contained in port.c  The ISR routines, which can only be compiled
 * to ARM mode, are contained in this file.
 *----------------------------------------------------------*/

/*
	Changes from V3.2.4

	+ The assembler statements are now included in a single asm block rather
	  than each line having its own asm block.
*/


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#include <stdlib.h>
#include "FreeRTOS.h"

/*-----------------------------------------------------------*/

/* See header file for description. */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
	portSTACK_TYPE *pxOriginalSP;

	/* Place a few bytes of known values on the bottom of the stack. 
	This is just useful for debugging. */

	*pxTopOfStack = 0x11111111;
	pxTopOfStack--;
	*pxTopOfStack = 0x22222222;
	pxTopOfStack--;
	*pxTopOfStack = 0x33333333;
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE)pvParameters;
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE)pvParameters;
	pxTopOfStack--;

	/* We are going to start the scheduler using a return from interrupt
	instruction to load the program counter, so first there would be the
	status register and interrupt return address.  We make this the start 
	of the task. */
	*pxTopOfStack = portINITIAL_SW; 
	pxTopOfStack--;
	*pxTopOfStack = portCODE_SEG;
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE)pxCode ;
	pxTopOfStack--;
	
	/* We are going to setup the stack for the new task to look like
	the stack frame was setup by a compiler generated ISR.  We need to know
	the address of the existing stack top to place in the SP register within
	the stack frame.  pxOriginalSP holds SP before (simulated) pusha was 
	called. */
	pxOriginalSP = pxTopOfStack;

	/* The remaining registers would be pushed on the stack by our context 
	switch function.  These are loaded with values simply to make debugging
	easier. */
	*pxTopOfStack = (portSTACK_TYPE)pvParameters ;		/* AX */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0xCCCCCCCC;		/* CX */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE)pvParameters ;		/* DX */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0xBBBBBBBB;		/* BX */
	pxTopOfStack--;
	*pxTopOfStack =  ( portSTACK_TYPE )pxOriginalSP ;	/* SP */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0xBBBBBBBB;		/* BP */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x01234567;		/* SI */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0xDDDDDDDD;		/* DI */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE )portDATA_SEG;		/* DS */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) portDATA_SEG;	/* ES */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE)0 ;					/* FS */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE)0 ;					/* GS */
	pxTopOfStack--;
	*pxTopOfStack = 0;									/* EIP for the nested call*/
	pxTopOfStack--;
	*pxTopOfStack = 0x11111111;							/* EBP for the nested call*/
	pxTopOfStack--;
	*pxTopOfStack = 0x22222222;							/* EBX for the nested call*/
	pxTopOfStack--;										/* Compiler needs this so we add it*/

	return pxTopOfStack;
}


