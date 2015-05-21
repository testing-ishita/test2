
/************************************************************************
 *                                                                      *
 *      Copyright 2008 Concurrent Technologies, all rights reserved.    *
 *                                                                      *
 *      The program below is supplied by Concurrent Technologies        *
 *      on the understanding that no responsibility is assumed by       *
 *      Concurrent Technologies for any errors contained therein.       *
 *      Furthermore, Concurrent Technologies makes no commitment to     *
 *      update or keep current the program code, and reserves the       *
 *      right to change its specifications at any time.                 *
 *                                                                      *
 *      Concurrent Technologies assumes no responsibility either for    *
 *      the use of this code or for any infringements of the patent     *
 *      or other rights of third parties which may result from its use  *
 *                                                                      *
 ************************************************************************/

/* stdio.c - console I/O 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/stdio.c,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: stdio.c,v $
 * Revision 1.1  2013-09-04 07:17:58  chippisley
 * Import files into new source repository.
 *
 * Revision 1.11  2011/01/20 10:01:25  hmuneer
 * CA01A151
 *
 * Revision 1.10  2010/06/23 10:51:04  hmuneer
 * CUTE V1.01
 *
 * Revision 1.9  2010/05/05 15:44:53  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.8  2008/09/29 10:15:16  swilson
 * dBitFlags is obsolete, use function call to get test handler mode instead.
 *
 * Revision 1.7  2008/09/19 14:50:53  swilson
 * Non-debug output should only happen in IBIT mode.
 *
 * Revision 1.6  2008/09/17 13:51:57  swilson
 * Add function to retrieve cursor's 'X' coordinate. Incorportate this into the low-level
 *  character out functuion.
 *
 * Revision 1.5  2008/07/24 14:40:15  swilson
 * semaphore.h moved to private.
 *
 * Revision 1.4  2008/07/18 13:01:47  swilson
 * Add semaphore-based locking around functions using shared (I/O) resources.
 *
 * Revision 1.3  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.2  2008/05/12 14:50:53  swilson
 * Add implementation code for stdio functions.
 *
 * Revision 1.1  2008/04/25 15:22:23  swilson
 * New C-library modules. Basic structure, little content.
 *
 */


/* includes */

#include <stdio.h>
#include <stdarg.h>

#include <private/scc.h>
#include <private/semaphore.h>

#include <bit/bit.h>
 
#include <bit/board_service.h>

#include "FreeRTOS.h"
#include "semphr.h"

/* defines */

/* typedefs */

/* constants */

/* locals */

static int	iColumn = 0;

/* globals */

/* externals */
extern xSemaphoreHandle globalMutexPool[MAX_CUTEBIT_MUTEX];

extern int vsprintf (char* achBuffer, const char* achFormat, va_list ptr);


/* forward declarations */


/*****************************************************************************
 * getx: get the current screen column
 *
 * RETURNS: 0 based screen column
 */

int getx (void)
{
	return (iColumn);

} /* getx () */


/*****************************************************************************
 * putchar: write a character to stdout
 *
 * RETURNS: character written, EOF on error
 */

int putchar
(
	int		c
)
{
	UINT16		port;

	xSemaphoreTake(globalMutexPool[MUTEX_STDIO],portMAX_DELAY);

	/* Only display 'regular' console output in Local IBIT mode */

	if ((bGetTestHandler () == TH__LBIT) ||(bGetTestHandler () == TH__MTH))
	{
		iColumn++;

		if(board_service(SERVICE__BRD_GET_CONSOLE_PORT, NULL, &port) == E__OK)
		{
			sysSccTxChar (port, (UINT8)c);
		}

		/* Append <CR> to <LF> */

		if (c == '\n')
		{
			if(board_service(SERVICE__BRD_GET_CONSOLE_PORT, NULL, &port) == E__OK)
			{
				sysSccTxChar (port, '\r');
			}
			iColumn = 0;
		}
	}

	xSemaphoreGive(globalMutexPool[MUTEX_STDIO]);

	return (c);

} /* putchar () */


/*****************************************************************************
 * getchar: read a character from stdin
 *
 * RETURNS: character from stdin, EOF on error
 */

int getchar (void)
{
	int	rxChar;
	UINT16		port;

	xSemaphoreTake(globalMutexPool[MUTEX_STDIO],portMAX_DELAY);

	if(board_service(SERVICE__BRD_GET_CONSOLE_PORT, NULL, &port) == E__OK)
	{
		rxChar = (int)sysSccRxChar (port);
	}

	xSemaphoreGive(globalMutexPool[MUTEX_STDIO]);

	return (rxChar);

} /* getchar () */


/*****************************************************************************
 * puts: write a NULL terminated string to stdout, followed by a newline
 *
 * RETURNS: non-negative value, EOF on error
 */


int puts
(
	char*	s
)
{
	cputs (s);			/* write string to the console */  
	putchar ('\n');		/* append <LF> */

	return (0);

} /* puts () */


/*****************************************************************************
 * cputs: write a NULL terminated string to stdout
 *
 * RETURNS: non-negative value, EOF on error
 */

int cputs
(
	char*	s
)
{
	/* Write string to the console */  

	while (*s != '\0')
	{
		putchar (*s);
		s++;
	}

	return (0);

} /* cputs () */


/*****************************************************************************
 * gets: read a string from stdin, terminated by new-line or EOF
 *
 * The newline is stripped from the input and the string NULL terminated.
 *
 * RETURNS: pointer to text string; NULL on error or EOF
 */

char* gets
(
	char*	s
)
{
	int	c;
	char*	d = s;

	/* Read a string from the console */  

	c = getchar ();
	
	while (c != '\r')		/* repeat until newline */
	{
		*(d++) = (char)c;

		c = getchar ();
	}

	*d = '\0';				/* NULL terminate the string */

	return (s);

} /* gets () */


/****************************************************************************
 * sprintf:
 *
 * RETURNS: 
 */

int sprintf
(
	char*		achBuffer,
	const char*	achFormat,
	...
)
{
	va_list		ptr;
	int		iResult;


	va_start (ptr, achFormat);
	iResult = vsprintf (achBuffer, achFormat, ptr);

	return (iResult);

} /* sprintf () */


/****************************************************************************
 * kbhit: peek at input buffer to see if a key has been hit
 *
 * RETURNS: 1 if char available in input buffer
 */

int kbhit (void)
{
	int	rxStatus;
	UINT16		port;

	xSemaphoreTake(globalMutexPool[MUTEX_STDIO],portMAX_DELAY);

	if(board_service(SERVICE__BRD_GET_CONSOLE_PORT, NULL, &port) == E__OK)
	{
		rxStatus = sysSccPollRx (port);
	}
	xSemaphoreGive(globalMutexPool[MUTEX_STDIO]);

	return (rxStatus);

} /* kbhit () */


