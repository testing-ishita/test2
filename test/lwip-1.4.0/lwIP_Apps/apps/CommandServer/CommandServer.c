/*******************************************************************************
*
* Filename:	 CommandServer.c
*
* Description:	lwIP Command Server source file, modified for CCT CUTE.
*
* $Revision: 1.4 $
*
* $Date: 2014-01-29 13:28:34 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/lwIP_Apps/apps/CommandServer/CommandServer.c,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

/*
    FreeRTOS V7.0.2 - Copyright (C) 2011 Real Time Engineers Ltd.
	

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
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

/* Standard includes. */
#include "stdlib.h"

#include <private/atomic.h>

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/sockets.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Utils includes. */
#include "../../utils/CommandInterpreter.h"

#include "config.h"
#include "../debug/dbgLog.h"

#undef INCLUDE_CMDSVR_DEBUG

#ifdef INCLUDE_DEBUG_VGA
#include "../vgalib/vga.h"
extern int startLine;
#endif

#ifdef INCLUDE_CMDSVR_DEBUG
#warning "***** DEBUG ON *****"
// #define CMDSVRDBG(x)	do {sysDebugPrintf x;} while(0)
#define CMDSVRDBG(x)	do {DBLOG x;} while(0)
#else
#define CMDSVRDBG(x)
#endif

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE	20

/* Dimensions the buffer into which string outputs can be placed. */
#define cmdMAX_OUTPUT_SIZE	1024

static const signed char *pcWelcomeMessage = (signed char *) "FreeRTOS command server\r\nType help to view a list of registered commands.\r\nType quit to close connection.\r\n\r\n>";

/*-----------------------------------------------------------*/

void vBasicSocketsCommandInterpreterTask( void *pvParameters )
{
	long lSocket, lClientFd, lBytes, lAddrLen = sizeof( struct sockaddr_in );
	struct sockaddr_in sLocalAddr;
	struct sockaddr_in client_addr;
	signed char cInChar, cInputIndex;
	static signed char cInputString[ cmdMAX_INPUT_SIZE ], cOutputString[ cmdMAX_OUTPUT_SIZE ];
	portBASE_TYPE xReturned;
#ifdef INCLUDE_DEBUG_VGA
	portSHORT cpuNo;
	UINT32 taskNum;
	int currentLine;
	char achBuffer[80];
	
	currentLine = atomic32Inc( (UINT32 *) &startLine );
	vgaClearLine( currentLine );

	cpuNo = sPortGetCurrentCPU();
	taskNum = uxTaskNumGet( cpuNo, NULL );
#endif

	lSocket = lwip_socket(AF_INET, SOCK_STREAM, 0);

	if( lSocket >= 0 )
	{
		// val = 1; // set not blocking
		// lwip_ioctl( lSocket, FIONBIO, &val );
		
		memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
		sLocalAddr.sin_family = AF_INET;
		sLocalAddr.sin_len = sizeof(sLocalAddr);
		sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		sLocalAddr.sin_port = ntohs( ( ( unsigned short ) 23 ) );

		if( lwip_bind( lSocket, ( struct sockaddr *) &sLocalAddr, sizeof( sLocalAddr ) ) < 0 ) 
		{
			CMDSVRDBG(("cmdSvr: Error - socket bind failed\n"));
			lwip_close( lSocket );
			vTaskDelete( NULL );
		}

		if( lwip_listen( lSocket, 20 ) != 0 )
		{
			CMDSVRDBG(("cmdSvr: Error - failed to create listening socket\n"));
			lwip_close( lSocket );
			vTaskDelete( NULL );
		}
		
#ifdef INCLUDE_DEBUG_VGA
		sprintf( achBuffer, "CPU:%d T%02u: cmdSvr Waiting for connections...", cpuNo, taskNum );
		vgaPutsXY( 1, currentLine, achBuffer );
		vgaClearToEol();
#endif
		
		for( ;; )
		{	
			CMDSVRDBG(("cmdSvr: Waiting for connections...\n"));
			
			lClientFd = lwip_accept( lSocket, ( struct sockaddr * ) &client_addr, ( u32_t * ) &lAddrLen );

			if( lClientFd > 0L )
			{
				CMDSVRDBG(("cmdSvr: Connection accepted\n"));
				
#ifdef INCLUDE_DEBUG_VGA
				sprintf( achBuffer, "CPU:%d T%02u: cmdSvr Connection accepted", cpuNo, taskNum );
				vgaPutsXY( 1, currentLine, achBuffer );
				vgaClearToEol();
#endif
				
				lwip_send( lClientFd, pcWelcomeMessage, strlen( ( const char * ) pcWelcomeMessage ), 0 );

				cInputIndex = 0;
				memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );

				do
				{					
					lBytes = lwip_recv( lClientFd, &cInChar, sizeof( cInChar ), 0 );

					if( lBytes > 0L ) 
					{
						if( cInChar == '\n' )
						{
							/* Input has been terminated. */
							
							/* Did we get an input string */
							if (cInputIndex > 0)
							{
								/* Was the input a quit command? */
								if( strcmp( "quit", ( const char * ) cInputString ) == 0 )
								{
									/* Set lBytes to 0 to close the connection. */
									lBytes = 0L;
								}
								else
								{
									/* Pass the string to the command interpreter. */
									do
									{
										xReturned = xCmdIntProcessCommand( cInputString, cOutputString, cmdMAX_OUTPUT_SIZE );
										lwip_send( lClientFd, cOutputString, strlen( ( const char * ) cOutputString ), 0 );
	
									} while( xReturned != pdFALSE );
								}
							}
								
							/* Clear the input string ready to receive the next command. */
							cInputIndex = 0;
							memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );
							lwip_send( lClientFd, "\r\n>", strlen( "\r\n>" ), 0 );
						}
						else if( cInChar == '\b' )
						{	
							/* Backspace was pressed.  Erase the last 
							character in the string - if any. */
							if( cInputIndex > 0 )
							{
								cInputIndex--;
								cInputString[ cInputIndex ] = '\0';
							}
						}
						else
						{
							/* Check character is valid, i.e not a control character */
							if( (cInChar > 0x1f) && (cInChar < 0x7f) )
							{
								/* Add the character to the string entered so far. */  
								if( cInputIndex < cmdMAX_INPUT_SIZE )
								{
									cInputString[ cInputIndex ] = cInChar;
									cInputIndex++;
								}
							}
						}
					}

				} while( lBytes > 0L );

				 lwip_close( lClientFd );
				 // vTaskDelay( 1 );
				 CMDSVRDBG(("cmdSvr: Connection closed\n"));
				 
#ifdef INCLUDE_DEBUG_VGA
				sprintf( achBuffer, "CPU:%d T%02u: cmdSvr Connection closed", cpuNo, taskNum );
				vgaPutsXY( 1, currentLine, achBuffer );
				vgaClearToEol();
#endif
			}
		}
	}
	else
	{
		CMDSVRDBG(("cmdSvr: Error - failed to create stream socket\n"));
	}

	/* Will only get here if a listening socket could not be created. */
	vTaskDelete( NULL );
}

