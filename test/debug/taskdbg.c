/*******************************************************************************
*
* Filename:	 taskdbg.c
*
* Description:	Task Debug functions, development use only.
*
* $Revision: 1.3 $
*
* $Date: 2013-10-08 07:19:30 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/debug/taskdbg.c,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

/*******************************************************************************
*
* The following functions are for post mortem debug purposes only and are called
* by the Exception handler in dbgLog.c. DO NOT use elsewhere.
* 
* This file is included in task.c when INCLUDE_EXCEPTION_DEBUG is defined.
* 
*******************************************************************************/

#include <bit/conslib.h>
#include <symbol.h>


#warning "***** EXCEPTION DEBUG INCLUDED *****" 

typedef struct
{
	UINT32	addr;		/* target address */
	SYMBOL	symbl[3];	/* symbol values: less than, equal and greater than addr */
	
} LKUP_SYMBOL;


extern int  iExtractNumber (char* achString, UINT32* pdData, UINT32 dMinimum, UINT32 dMaximum);

extern char _text_start;
extern char _text_stop;

extern void dbgPrintVal( int radix, int width, UINT32 value );
extern void dbgPrintMsg( char* buffer );


/*******************************************************************************
*
* Exception Handler Functions
*
*******************************************************************************/ 

/*******************************************************************************
*
* vTdbgLkupSym
*
* Lookup given symbol name in the symbol table and return a point to the entry. 
* 
*
* RETURNS: point to symbol table entry or NULL if not found.
*
*******************************************************************************/
static SYMBOL *vTdbgLkupSym( char *pName )
{
#ifdef INCLUDE_SYMBOLS	
	UINT32 i;
	SYMBOL *pSym;


	if (symTblSize > 0)
	{
		for (i = 0; i < symTblSize; i++)
		{
			pSym = &symTbl[i];
			
			if (strcmp(pSym->pName, pName) == 0)
			{
				/* Found a matching symbol */
				return pSym;
			}
		}
	}
#else
	/* Stop warnings. */
	( void ) pName;
#endif

	return NULL;
}


#ifdef INCLUDE_SYMBOLS
/*******************************************************************************
*
* vTdbgCheckSymAddr
*
* Compare given symbol value with the given address.  
* If the value associated with the symbol is equal to or closer to it than the
* previous close values, the appropriate slot in the LKUP_SYMBOL array 
* is filled with the data for this symbol. 
* 
*
* RETURNS: point to symbol table entry or NULL if not found.
*
*******************************************************************************/
static void vTdbgCheckSymAddr( SYMBOL *pSym, LKUP_SYMBOL *pLkup )
{
	UINT32 value;
	
	
	value = (UINT32)pSym->pAddr;
	
	if (value < pLkup->addr)
	{
		if (value > (UINT32) pLkup->symbl[0].pAddr)
	    {
		    /* found closer symbol that is less than target */
		    pLkup->symbl[0].pAddr = pSym->pAddr;
		    pLkup->symbl[0].type = pSym->type;
		    pLkup->symbl[0].pName = pSym->pName;
	    }
	}
	else if (value == pLkup->addr)
	{
		/* found target address, fill in target entry */
		pLkup->symbl[1].pAddr = pSym->pAddr;
		pLkup->symbl[1].type = pSym->type;
		pLkup->symbl[1].pName = pSym->pName;
	}
	else if (value > pLkup->addr)
	{
		if ((value < (UINT32) pLkup->symbl[2].pAddr) || (pLkup->symbl[2].pAddr == NULL))
		{
			/* found closer symbol that is greater than target */
			pLkup->symbl[2].pAddr = pSym->pAddr;
			pLkup->symbl[2].type = pSym->type;
			pLkup->symbl[2].pName = pSym->pName;
		}
	}
}
#endif


/*******************************************************************************
*
* vTdbgPrintStack
*
* Print task stack on debug console. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
static void vTdbgPrintStack( volatile tskTCB *pxTCB, UINT32 stackEnd )
{
	UINT32 i;
	UINT32 *pStart;
	UINT32 count;
	
	
	dbgPrintMsg( "\nStack\n" );
	
	pStart = (UINT32 *) pxTCB->pxTopOfStack;
	
	if ( (stackEnd > 0) && (stackEnd > (UINT32) pStart) )
	{
		count = ((stackEnd - (UINT32) pStart) + 4) / 4;
		
		/* Limit stack size printed */
		if (count > 256)
		{
			count = 64;
		}
	}
	else
	{
		count = 64;
	}
	
	/* Print stack */
	for ( i = 0; i < count; i++ )
	{
		dbgPrintVal( 16, 8, (UINT32) pStart );
		dbgPrintMsg( " " );
		dbgPrintVal( 16, 8, (UINT32) *pStart );
		dbgPrintMsg( "\n" );
		pStart++;
	}
}


/*******************************************************************************
*
* vTdbgPrintBacktrace
*
* Print task backtrace (call stack) on debug console. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
static UINT32 vTdbgPrintBacktrace( UINT32 *eNextBP )
{      
    UINT32 *p, *pBP;
	UINT32 addr;
	UINT32 stackEnd;                                            
    int i, x;       
    
    
    stackEnd = 0;
    dbgPrintMsg( "\nBacktrace\n" );
    
    /* Run through the backtrace with 100 limit */     
    for(i = 0; eNextBP && i < 100; i++)       
    {                 
        pBP = eNextBP;
        
        /* Check EBP addres is within Kernel heap */
        /* Kernel heap: 0x04000000 - 0x0400FFFF   */
        if ( (*pBP < 0x04000000) || (*pBP > 0x0400FFFF) )
        {
        	/* EBP not within range, try next value */
        	eNextBP++;
        	continue;
		}
		
		dbgPrintVal( 16, 8, (UINT32) pBP );
		dbgPrintMsg( " EBP: " );
		dbgPrintVal( 16, 8, (UINT32) *pBP );
        
        addr = (UINT32) (*(UINT32 **)(pBP + 1));
		 
        dbgPrintMsg( " Return address: " );
		dbgPrintVal( 16, 8, addr );
		dbgPrintMsg( "\n" );
		
		/* Check addres is within Code section */
		if ( (addr < (UINT32) &_text_start) || (addr > (UINT32) &_text_stop) )
		{
			break;
		}
		
		/* Keep current Base Pointer */           
        eNextBP = *(UINT32 **)pBP;
                    
        /* Write potential arguments (20 bytes) */
		dbgPrintMsg( "         Arguments: " );          
        p = pBP + 2;
		                   
        for( x = 0; x < 5; p++, x++ )
		{
			dbgPrintVal( 16, 8, *(UINT32 *) p );
			dbgPrintMsg( " " );
		}              
                       
        dbgPrintMsg( "\n" );
		
		stackEnd = (UINT32) pBP;       
    }
    
    return stackEnd;
}


/*******************************************************************************
*
* vTdbgPrintTCB
*
* Print information from task control block on debug console. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
static void vTdbgPrintTCB( volatile tskTCB *pxTCB )
{
	unsigned short usStackRemaining;
	UINT32 stackEnd;
	
	dbgPrintMsg( (char*) &pxTCB->pcTaskName[0] );
	dbgPrintMsg( "\n" );
	
	dbgPrintMsg( "pxTopOfStack: 0x" );
	dbgPrintVal( 16, 8, (UINT32) pxTCB->pxTopOfStack );
	dbgPrintMsg( "\n" );
	
	dbgPrintMsg( "pxStack     : 0x" );
	dbgPrintVal( 16, 8, (UINT32) pxTCB->pxStack );
	dbgPrintMsg( "\n" );
	
	usStackRemaining = usTaskCheckFreeStackSpace( ( unsigned char * ) pxTCB->pxStack );
	
	dbgPrintMsg( "Stack       : " );
	dbgPrintVal( 10, 8, (UINT32) usStackRemaining );
	dbgPrintMsg( " remaining\n" );
	
	stackEnd = vTdbgPrintBacktrace( (UINT32 *) pxTCB->pxTopOfStack );
	
	vTdbgPrintStack( pxTCB, stackEnd );
}


/*******************************************************************************
*
* vTdbgPrintCurrentTCBs
*
* Print information from current task control blocks on debug console. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vTdbgPrintCurrentTCBs( void )
{
	int i;
	
	
	for ( i = 0; i < bCpuConfigured; i++ )
	{
		dbgPrintMsg( "CPU: " );
		dbgPrintVal( 10, 2, (UINT32) i );
		dbgPrintMsg( "\n" );
		vTdbgPrintTCB( pxCurrentTCB[i] );
		dbgPrintMsg( "\n" );
	}
}


/*******************************************************************************
*
* Debug Command Handler Functions
*
*******************************************************************************/ 

/*******************************************************************************
*
* vTdbgTcbShow
*
* Print task control block at given address. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vTdbgTcbShow( int iCmdType, int iCmdCount, char* achUserArgs[] )
{
	ARGS	sArgs;
	int		iStatus;
	
	
	if (iCmdType != 0)
	{
	   iCmdType = 0; // not used
	}
	
	iStatus = iExtractArgs( &sArgs, ARG__ADDR, 0, iCmdCount, achUserArgs );
	
	if (iStatus != E__OK)
	{
		return;
	}
	
	vTaskSuspendAll();
	vTdbgPrintTCB( (volatile tskTCB *) sArgs.dAddr );
	xTaskResumeAll();
}


/*******************************************************************************
*
* vTdbgTest
*
* Test function for vTdbgRunFunc.
* 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vTdbgTest( UINT32 arg1, UINT32 arg2 )
{
	UINT32 addr;
	
	sysDebugPrintf( "%s: arg1: %u arg2: %u\n", __FUNCTION__, arg1, arg2 );
	
	addr = 0x1863d8;
	
	/* Generate an exception */
	((VOIDFUNCPTR) addr)();
}


/*******************************************************************************
*
* vTdbgRunFunc
*
* Execute given function with optional arguments. The symbol table is search
* to find the execution address of the given function name.
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vTdbgRunFunc( int iCmdType, int iCmdCount, char* achUserArgs[] )
{
	UINT32 symAddr;
	UINT32 argVal[2];
	SYMBOL *pSym;
	
	
	if (iCmdType != 0)
	{
	   iCmdType = 0; // not used
	}
	
	if ( (iCmdCount > 1) && (iCmdCount < 5) )
	{
		/* Find the name in the symbol table */
		pSym = vTdbgLkupSym( achUserArgs[1] );
		
		if (pSym != NULL)
		{
			/* Get the address */
			symAddr = (UINT32) pSym->pAddr;
			
			/* If the symbol is in the text (code) section we can execute it */
			if (pSym->type == 'T')
			{
				sysDebugPrintf( "%s: %s at: 0x%x\n", achUserArgs[0], achUserArgs[1], symAddr);
			
				/* Get arguments and run */
				switch( iCmdCount )
				{
					case 3: iExtractNumber( achUserArgs[2], &argVal[0], 0, 0xFFFFFFFFL );
							((VOIDFUNCPTR) symAddr)( argVal[0] );
							break;
							
					case 4: iExtractNumber( achUserArgs[2], &argVal[0], 0, 0xFFFFFFFFL );
							iExtractNumber( achUserArgs[3], &argVal[1], 0, 0xFFFFFFFFL );
							((VOIDFUNCPTR) symAddr)( argVal[0], argVal[1] );
							break;
					
					default: ((VOIDFUNCPTR) symAddr)(); break;
				}
			}
			else
			{
				sysDebugPrintf( "Symbol: %s at: 0x%x is not executable\n", achUserArgs[1], symAddr);
			}
		}
		else
		{
			sysDebugPrintf("Symbol: %s not found\n", achUserArgs[1]);
		}
	}
	else
	{
		sysDebugPrintf("Invalid numbers of arguments\n");
	}
}


/*******************************************************************************
*
* vTdbgLkupSymbol
*
* Lookup the given address in the symbol table and show the closest/matching 
* symbols.
* 
*
* RETURNS: None.
*
*******************************************************************************/
void vTdbgLkupSymbol( int iCmdType, int iCmdCount, char* achUserArgs[] )
{
#ifdef INCLUDE_SYMBOLS	
	UINT32 i;
	LKUP_SYMBOL lkup;


	iCmdType = 0; /* not used */
	
	if ( iCmdCount == 2 )
	{
		iExtractNumber( achUserArgs[1], &lkup.addr, 0, 0xFFFFFFFFL );
		
		sysDebugPrintf("Looking for symbol at: 0x%08x\n", lkup.addr);
	
		if (symTblSize > 0)
		{
			for (i = 0; i < 3; i++)
			{
				lkup.symbl[i].pAddr = NULL;
				lkup.symbl[i].type = 0;
				lkup.symbl[i].pName = NULL;
			}
			
			for (i = 0; i < symTblSize; i++)
			{
				vTdbgCheckSymAddr( &symTbl[i], &lkup );
			}
			
			for (i = 0; i < 3; i++)
			{
				if ( lkup.symbl[i].pName != NULL )
				{
					sysDebugPrintf("0x%08x %c %s\n", (UINT32) lkup.symbl[i].pAddr, 
								lkup.symbl[i].type, lkup.symbl[i].pName );
				}
			}
		}
	}
	else
	{
		sysDebugPrintf("Invalid numbers of arguments\n");
	}
#else

	/* Stop warnings. */
	( void ) iCmdType;
	( void ) iCmdCount;
	( void ) achUserArgs[1];

	sysDebugPrintf("Symbol table not present\n");

#endif
}


void vTaskListShow( void )
{
	portSHORT cpuNo;
	unsigned portBASE_TYPE i;
	
	for ( cpuNo = 0; cpuNo < bCpuConfigured; cpuNo++ )
	{
		vTaskSuspendCpu( cpuNo );
		
		for ( i = 0; i < configMAX_PRIORITIES; i++)
		{
			sysDebugPrintf( "ReadyTasksLists[%02d]    : 0x%08x\n", i, (UINT32) &( cpuTaskCtrl[cpuNo].pxReadyTasksLists[i]) );
		}
		
		sysDebugPrintf( "DelayedTaskList1       : 0x%08x\n", (UINT32) &( cpuTaskCtrl[cpuNo].xDelayedTaskList1) );
		sysDebugPrintf( "DelayedTaskList2       : 0x%08x\n", (UINT32) &( cpuTaskCtrl[cpuNo].xDelayedTaskList2) );
		sysDebugPrintf( "PendingReadyList       : 0x%08x\n", (UINT32) &( cpuTaskCtrl[cpuNo].xPendingReadyList) );
		sysDebugPrintf( "TasksWaitingTermination: 0x%08x\n", (UINT32) &( cpuTaskCtrl[cpuNo].xTasksWaitingTermination) );
		sysDebugPrintf( "SuspendedTaskList      : 0x%08x\n\n", (UINT32) &( cpuTaskCtrl[cpuNo].xSuspendedTaskList) );
		
		xTaskResumeCpu( cpuNo );
	}
}
