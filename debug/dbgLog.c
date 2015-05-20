/*******************************************************************************
*
* Filename:	 dbgLog.c
*
* Description:	Debug logging functions, development use only.
*
* $Revision: 1.5 $
*
* $Date: 2014-02-14 11:40:57 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/debug/dbgLog.c,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#include <stdio.h>
#include <stdtypes.h>
#include <stdarg.h>
#include <private/debug.h>
#include <private/port_io.h>
#include <bit/board_service.h>

#include "FreeRTOS.h"
#include "task.h"

#undef USE_FREERTOS_SEM

#ifdef USE_FREERTOS_SEM
#include "semphr.h"
#else
#include <private/semaphore.h>
#endif

#include "config.h"
#include "dbgLog.h"


#ifdef INCLUDE_EXCEPTION_DEBUG

#define DBTXBYTE( port, val ) \
		sysOutPort8( port, val); \
		while( (sysInPort8(port + 5) & 0x40) == 0 ) \
					;

static UINT16 dbgComPortBase = 0x3f8;	/* 0x3f8 = COM1 0x2f8 = COM2 */

extern const char achDigits[];
extern void vTdbgPrintCurrentTCBs( void );
#endif


#ifdef INCLUDE_DBGLOG
#warning "***** DEBUG LOG INCLUDED *****" 

#undef STATIC_DBLOG					/* define to use static debug log */

#ifndef BOOL
#define BOOL 	int
#endif

#ifndef TRUE
#define TRUE 	1
#endif

#ifndef FALSE
#define FALSE 	0
#endif

#define LOG_ENTRY_FREE 	0x45455246 
#define LOG_ENTRY_USED 	0x44455355 

/* typedefs */
typedef struct
{
	UINT32 entryNum;				/* entry number */
	UINT32 dbLevel;					/* message debug level (currently not used) */
    char text[DBG_LOG_ENTRY_LENGTH];	/* message text */
    UINT32 inUse;					/* whether entry is valid */
    
} LOG_ENTRY;


/* locals */
static BOOL dbgLogInitComplete = FALSE;

#ifdef USE_FREERTOS_SEM
static xSemaphoreHandle dbgLogMutex = NULL;
#else
static SEMAPHORE dbLogSema = 0;
#endif

static BOOL dbgIsLogOpen = FALSE;
static LOG_ENTRY *dbgLogPtr = NULL;
static UINT32 dbgLogNextEntry = 0;

#ifdef STATIC_DBLOG
static LOG_ENTRY dbLog[MAX_DBG_LOG_ENTRIES];
#endif

static char pcDispBuffer[DBG_LOG_ENTRY_LENGTH + 8];

/* globals */

/* externals */
extern int vsprintf( char* achBuffer, const char* achFormat, va_list ptr );
extern int sysIsDebugopen( void );



/*******************************************************************************
*
* clearLogEntry
*
* Clear a given log entry. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
static void clearLogEntry( const int entry )
{
	int i;
	
    if ( (dbgLogPtr != NULL) && (entry < MAX_DBG_LOG_ENTRIES) )
    {
    	dbgLogPtr[entry].entryNum = 0;
 		dbgLogPtr[entry].dbLevel = 0;
 		
 		for (i = 0; i < DBG_LOG_ENTRY_LENGTH; i++ )
 		{
        	dbgLogPtr[entry].text[i] = '\0';
        }
        
		dbgLogPtr[entry].inUse = LOG_ENTRY_FREE;
    }
}


/*******************************************************************************
*
* dbLogClear
*
* Clear the debug log. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void dbgLogClear( void )
{
	int entry;
	
	
	if (dbgLogInitComplete == TRUE)
	{
#ifdef USE_FREERTOS_SEM
		xSemaphoreTake( dbgLogMutex, portMAX_DELAY );
#else
		sysSemaphoreLock( &dbLogSema );
#endif
	}
    
    for (entry = 0; entry < MAX_DBG_LOG_ENTRIES; entry++)
    {
        clearLogEntry( entry );
    }    
    
    dbgLogNextEntry = 0;    

	if (dbgLogInitComplete == TRUE)
	{
#ifdef USE_FREERTOS_SEM
		xSemaphoreGive( dbgLogMutex );
#else
		sysSemaphoreRelease( &dbLogSema );
#endif
	}
}


/*******************************************************************************
*
* dbgLogPrinf
*
* Log the given message. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void dbgLogPrinf( char *format, ... )
{
	va_list vp;
	
	
	if (dbgIsLogOpen == TRUE)
	{
#ifdef USE_FREERTOS_SEM
		xSemaphoreTake( dbgLogMutex, portMAX_DELAY );
#else
		sysSemaphoreLock( &dbLogSema );
#endif
	
		dbgLogPtr[dbgLogNextEntry].entryNum = dbgLogNextEntry;
		dbgLogPtr[dbgLogNextEntry].dbLevel = 0;
		
		/* format & save message */
		va_start(vp, format);
		vsprintf( &dbgLogPtr[dbgLogNextEntry].text[0], (const char *)format, vp );
		va_end(vp);
	
	    dbgLogPtr[dbgLogNextEntry].inUse = LOG_ENTRY_USED;
	    
	    dbgLogNextEntry = (dbgLogNextEntry + 1) % MAX_DBG_LOG_ENTRIES;
		
#ifdef USE_FREERTOS_SEM
		xSemaphoreGive( dbgLogMutex );
#else
		sysSemaphoreRelease( &dbLogSema );
#endif
	}
}


#if 0  // test code to generate exception 
/*******************************************************************************
*
* dbgGenException
*
* Test function to generate an exception. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
static void dbgGenException( void )
{
	UINT32 tmp = 0xCDCDCDCD;
	
/******************************************************************************/
#warning "***** TEST EXCEPTION INCLUDED *****" 
    		// count = len / i;	/* generate a divide by zero exception */

    		*((char *) 0x08000000) = tmp;	/* generate page fault */
/******************************************************************************/  

}
#endif


/*******************************************************************************
*
* dbgLogGetNextEntry
*
* Get next debug log entry number. 
* 
*
* RETURNS: DBLOG_OK, DBLOG_ENTRY_NOT_USED or error code.
*
*******************************************************************************/
UINT32 dbgLogGetNextEntry( void )
{
	UINT32 result;
    
    if ( dbgLogInitComplete == TRUE  )
    {
	
#ifdef USE_FREERTOS_SEM
		xSemaphoreTake( dbgLogMutex, portMAX_DELAY );
#endif		
		
    	result = dbgLogNextEntry;
    
#ifdef USE_FREERTOS_SEM
		xSemaphoreGive( dbgLogMutex );
#endif

	}
	else
	{
		result = 0;
	}
	
	return result;
}


/*******************************************************************************
*
* dbgLogGetEntry
*
* Get an entry from the debug log. 
* 
*
* RETURNS: DBLOG_OK, DBLOG_ENTRY_NOT_USED or error code.
*
*******************************************************************************/
int dbgLogGetEntry( char *pcWriteBuffer, size_t xWriteBufferLen, UINT32 entry )
{
	int result;
    
    
    if ( (entry >= MAX_DBG_LOG_ENTRIES) && (xWriteBufferLen < (DBG_LOG_ENTRY_LENGTH + 2)) )
    {
		return DBLOG_ERR_PARAM;
	}
    
	if ( dbgLogInitComplete == TRUE  )
	{
#ifdef USE_FREERTOS_SEM
		xSemaphoreTake( dbgLogMutex, portMAX_DELAY );
#endif		
		
    	if ( dbgLogPtr[entry].inUse == LOG_ENTRY_USED ) 
    	{
#ifndef USE_FREERTOS_SEM
    		sysSemaphoreLock( &dbLogSema );
#endif
			sprintf( pcWriteBuffer, "%04u %s\r", entry, &dbgLogPtr[entry].text[0] );
			
#ifndef USE_FREERTOS_SEM
			sysSemaphoreRelease( &dbLogSema );
#endif

			result = DBLOG_OK;
    	}
    	else
    	{
			result = DBLOG_ENTRY_NOT_USED;
		}
    
#ifdef USE_FREERTOS_SEM
		xSemaphoreGive( dbgLogMutex );
#endif

	}
	else
	{
		result = DBLOG_ERR_INIT;
	}
	
	return result;
}


/*******************************************************************************
*
* dbgLogPrint
*
* Print the debug log. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void dbgLogPrint( const UINT32 dbLevel )
{
	int i;
	int len;
	UINT32 count;
    
    
    /* dbgGenException(); */
    
	puts("\n******************************* Debug Log *******************************");

	if ( dbgLogInitComplete == TRUE )
	{
#ifdef USE_FREERTOS_SEM
		xSemaphoreTake( dbgLogMutex, portMAX_DELAY );
#endif		
		count = 0;
		
    	for ( i = 0; i < MAX_DBG_LOG_ENTRIES; i++ )
    	{
        	if ( (dbgLogPtr[i].inUse == LOG_ENTRY_USED) && 
					(dbgLogPtr[i].dbLevel == dbLevel) )
        	{
#ifndef USE_FREERTOS_SEM
        		sysSemaphoreLock( &dbLogSema );
#endif
				sprintf( pcDispBuffer, "%04d %s", i, &dbgLogPtr[i].text[0] );
				
#ifndef USE_FREERTOS_SEM
				sysSemaphoreRelease( &dbLogSema );
#endif
				
				len = strlen( pcDispBuffer );
				
				/* Remove newline as puts will also add one */
				if ((len > 0) && (pcDispBuffer[len - 1] == '\n') )
				{
					pcDispBuffer[len - 1] = '\0';
				}
				
				puts( pcDispBuffer );
				
				count++;
				
				if (count % 50 == 0)
				{
#ifdef USE_FREERTOS_SEM
					xSemaphoreGive( dbgLogMutex );
#endif
					vTaskDelay( 10 );
					
#ifdef USE_FREERTOS_SEM
					xSemaphoreTake( dbgLogMutex, portMAX_DELAY );
#endif
				}
        	}
    	}

		sprintf( pcDispBuffer, "Next entry: %u\n", dbgLogNextEntry);
		puts( pcDispBuffer );
		
#ifdef USE_FREERTOS_SEM
		xSemaphoreGive( dbgLogMutex );
#endif

	}
	else
	{
		puts( "Error - log not initialized\n");
	}
}


/*******************************************************************************
*
* dbgLogOpen
*
* Open the debug log. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void dbgLogOpen( UINT32 dbLevel )
{
	if ( (dbgLogInitComplete == TRUE) && (dbgIsLogOpen == FALSE) )
	{
#ifdef USE_FREERTOS_SEM
		xSemaphoreTake( dbgLogMutex, portMAX_DELAY );
#else
		sysSemaphoreLock( &dbLogSema );
#endif

		dbgIsLogOpen = TRUE;
		
#ifdef USE_FREERTOS_SEM
		xSemaphoreGive( dbgLogMutex );
#else
		sysSemaphoreRelease( &dbLogSema );
#endif
	}
}


/*******************************************************************************
*
* dbgLogClose
*
* Close the debug log. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void dbgLogClose( UINT32 dbLevel )
{
	if ( (dbgLogInitComplete == TRUE) && (dbgIsLogOpen == TRUE) )
	{
#ifdef USE_FREERTOS_SEM
		xSemaphoreTake( dbgLogMutex, portMAX_DELAY );
#else
		sysSemaphoreLock( &dbLogSema );
#endif

		dbgIsLogOpen = FALSE;
		
#ifdef USE_FREERTOS_SEM
		xSemaphoreGive( dbgLogMutex );
#else
		sysSemaphoreRelease( &dbLogSema );
#endif
	}
}


/*******************************************************************************
*
* dbgLogInit
*
* This function initializes the debug log.
*
*
* RETURNS: None
* 
*******************************************************************************/
void dbgLogInit( void )
{
	if (dbgLogInitComplete == FALSE)
	{
	
#ifdef STATIC_DBLOG
		dbgLogPtr = &dbLog[0]; 
#else
		dbgLogPtr = (LOG_ENTRY *) 0x04C00000; // size: 1720000, max: 4MB (4194304)
#endif

#ifdef USE_FREERTOS_SEM
		dbgLogMutex = xSemaphoreCreateMutex();
		
		if (dbgLogMutex != NULL)
#endif
		{
			dbgLogClear();
	
			dbgLogInitComplete = TRUE;
			dbgIsLogOpen = TRUE;
		}
	}
}
#endif /* INCLUDE_DBGLOG */


#ifdef INCLUDE_EXCEPTION_DEBUG
#warning "***** EXCEPTION_DEBUG INCLUDED *****" 

/*******************************************************************************
*
* dbgPrintMsg
*
* Print message directly on debug console. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void dbgPrintMsg( char* buffer )
{
	if (sysIsDebugopen() != 0)
	{
		/* Write message to debug console */  
	
		while (*buffer != '\0')
		{
			DBTXBYTE( dbgComPortBase, *buffer );
	
			if (*buffer == '\n')
			{
				DBTXBYTE( dbgComPortBase, '\r' ); /* append CR to LF */
			}		
	
			buffer++;
		}
	}
}


/*******************************************************************************
*
* dbgPrintVal
*
* Print value directly on debug console. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void dbgPrintVal( int radix, int width, UINT32 value )
{
	int	i = 0;
	int j = 1;
	char s[16];


	/* Handle n = zero */
	if (value == 0)
	{
		s[i++] = '0';
	}

	/* Convert +ve int to char string (reverse order) */
	while (value > 0)
	{
		s[i++] = achDigits[value % radix];
		value /= radix;
	}
	
	/* NULL terminate */
	s[i--] = 0;

	/* Print leading zeros */
	while ( (i + j) < width )
	{
		DBTXBYTE( dbgComPortBase, '0' );
		j++;
	}
	
	/* Print value (reverse order) */
	while (i >= 0)
	{	
		DBTXBYTE( dbgComPortBase, s[i--] );
	}
}


/*******************************************************************************
*
* dbgExcHandler
*
* Exception handler. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
static void dbgExcHandler( UINT32 vector )
{
	UINT32 count = 0;

	
	/* We don't want to be interrupted */
	asm volatile ( "cli" );

	/* Print exception details */
	
	dbgPrintMsg( "Exception: " );
	switch (vector)
	{
		case 0x0:		dbgPrintMsg( "divide error\n" ); break;
		case 0x1:		dbgPrintMsg( "debug\n" ); break;
		case 0x2:		dbgPrintMsg( "NMI\n" ); break;
		case 0x3:		dbgPrintMsg( "breakpoint\n" ); break;
		case 0x4:		dbgPrintMsg( "overflow\n" ); break;
		case 0x5:		dbgPrintMsg( "bound\n" ); break;
		case 0x6:		dbgPrintMsg( "invalid opcode\n" ); break;
		case 0x7:		dbgPrintMsg( "device not available\n" ); break;
		case 0x8:		dbgPrintMsg( "double fault\n" ); break;
		case 0x9:		dbgPrintMsg( "co-processor overrun\n" ); break;
		case 0xa:		dbgPrintMsg( "invalid TSS\n" ); break;
		case 0xb:		dbgPrintMsg( "segment not present\n" ); break;
		case 0xc:		dbgPrintMsg( "stack fault\n" ); break;
		case 0xd:		dbgPrintMsg( "general protection fault\n" ); break;
		case 0xe:		dbgPrintMsg( "page fault\n" ); break;
		case 0xf:		dbgPrintMsg( "reserved!!\n" ); break;
		case 0x10:		dbgPrintMsg( "co-processor error\n" ); break;
		case 0x11:		dbgPrintMsg( "alignment check\n" ); break;
		case 0x12:		dbgPrintMsg( "machine check\n" ); break;
		case 0x13:		dbgPrintMsg( "streaming SIMD\n" ); break;
		default:		dbgPrintMsg( "unknown\n" ); break;
	}
	
	vTdbgPrintCurrentTCBs();

	/* Loop forever and flash user LED */
	
	for(;;)
	{
		count++;
		sysInPort8( 0x84); /* takes ~720ns */
		
		if ( (count % 173611) == 0 )
		{
			/* Toggle POST LED */
			sysOutPort8( 0x61, sysInPort8( 0x61 ) ^ 0x2 );
		}
	}
}


/*******************************************************************************
*
* dbgInstallExceptionHandlers
*
* Installs debug exception handlers. 
* 
*
* RETURNS: None.
*
*******************************************************************************/
void dbgInstallExceptionHandlers( void )
{
	UINT32 vector;
	
	
	if(board_service(SERVICE__BRD_GET_DEBUG_PORT, NULL, &dbgComPortBase) != E__OK)
	{
		dbgComPortBase = 0x3f8;	// use default
	}
	
	for ( vector = 0; vector < 0x20; vector++ )
	{
		sysInstallUserHandler( vector, dbgExcHandler );
	}
}

#endif /* INCLUDE_EXCEPTION_DEBUG */
 
