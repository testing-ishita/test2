/*******************************************************************************
*
* Filename:	 dbgLog.h
*
* Description:	Header for debug logging functions, development use only.
*
* $Revision: 1.2 $
*
* $Date: 2014-02-14 11:40:57 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/debug/dbgLog.h,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/


#ifndef __dbgLog_h__
#define __dbgLog_h__

#include "config.h"

#define DBLOG_ENTRY_NOT_USED 	1
#define DBLOG_OK				0
#define DBLOG_ERR				-1
#define DBLOG_ERR_INIT			-2
#define DBLOG_ERR_PARAM			-3

#define MAX_DBG_LOG_ENTRIES 1000  /* 10000 */
#define DBG_LOG_ENTRY_LENGTH 164		/* the maximum length of message */

UINT32 dbgLogGetNextEntry( void );
int dbgLogGetEntry( char *pcWriteBuffer, size_t xWriteBufferLen, UINT32 entry );
void dbgLogPrinf( char *format, ... );
void dbgLogClear( void );

#ifdef INCLUDE_DBGLOG

/* Variadic Macros */

#define DBLOG( format, ... ) dbgLogPrinf( format, ##__VA_ARGS__ )

#else

#define DBLOG( format, ... )

#endif

#endif /* __dbgLog_h__ */
