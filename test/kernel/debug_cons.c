
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

/* debug_cons.c - direct console I/O via serial port
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/debug_cons.c,v 1.2 2013-10-08 07:09:46 chippisley Exp $
 *
 * $Log: debug_cons.c,v $
 * Revision 1.2  2013-10-08 07:09:46  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:41:28  chippisley
 * Import files into new source repository.
 *
 * Revision 1.13  2011/06/07 14:57:19  hchalla
 * Removed the support for serial port baud rate below 9600, the minimum baud rate now is 9600.
 *
 * Revision 1.12  2011/05/16 14:31:33  hmuneer
 * Info Passing Support
 *
 * Revision 1.11  2011/03/22 13:43:43  hchalla
 * *** empty log message ***
 *
 * Revision 1.10  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.9  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.8  2010/09/15 12:21:41  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.7  2010/06/23 11:00:42  hmuneer
 * CUTE V1.01
 *
 * Revision 1.6  2008/09/29 10:09:32  swilson
 * Ensure debug channel can only be opened if debugging was enabled when the application
 *  started. Add comment about using vDebugWrite() in preference to the 'sys' function.
 *
 * Revision 1.5  2008/07/24 14:30:42  swilson
 * semaphore.h moved to private.
 *
 * Revision 1.4  2008/07/18 13:03:06  swilson
 * Add semaphore based locking around functions using shared (I/O) resources.
 *
 * Revision 1.3  2008/06/20 12:16:54  swilson
 * Foundations of PBIT and PCI allocation. Added facility to pass startup options into
 *  C-code.
 *
 * Revision 1.2  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.1  2008/05/12 14:56:07  swilson
 * Interface to the debug console.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdarg.h>

#include <private/debug.h>
#include <private/scc.h>
#include <private/semaphore.h>
#include <private/port_io.h>

#include <bit/bit.h>
 
#include <bit/board_service.h>

#include "FreeRTOS.h"
#include "semphr.h"


/* defines */

/* typedefs */

/* constants */

/* locals */

static int		iDebugChannel = 0;			/* default to closed */
static UINT16	wComPortBase = 0;


/* globals */

/* externals */
extern xSemaphoreHandle globalMutexPool[MAX_CUTEBIT_MUTEX];
extern int vsprintf( char* achBuffer, const char* achFormat, va_list ptr );

/* forward declarations */



/*****************************************************************************
 * sysDebugOpen: enable output to the debug channel
 *
 * RETURNS: None
 */

void sysDebugOpen (void)
{
	UINT8	bTmp;
	UINT16	port;
	UINT8 	baud;

	/* Only open if debugging enabled and currently closed */

	if ((bStartupDebugMode () == SF__DEBUG) && (iDebugChannel == 0))
	{
		if(board_service(SERVICE__BRD_GET_DEBUG_PORT, NULL, &port) == E__OK)
		{
			wComPortBase = port;
		}
		else
		{
			iDebugChannel = 0;
			return;
		}

		/* Program baud rate divisor */
		bTmp = sysInPort8(wComPortBase + LCR);
		sysOutPort8 (wComPortBase + LCR, bTmp | LCR_DLAB);	/* DLAB = 1 to access divisor*/

		baud = bGetBaudRate();
		switch(baud)
		{
			case 0:
				sysOutPort8 (wComPortBase + DLL, (UINT8)(BAUD_9600 & 0x00FF));
				break;

			case 1:
				sysOutPort8 (wComPortBase + DLL, (UINT8)(BAUD_19200 & 0x00FF));
				break;

			case 2:
				sysOutPort8 (wComPortBase + DLL, (UINT8)(BAUD_38400 & 0x00FF));
				break;

			case 3:
				sysOutPort8 (wComPortBase + DLL, (UINT8)(BAUD_57600 & 0x00FF));
				break;

			case 4:
				sysOutPort8 (wComPortBase + DLL, (UINT8)(BAUD_115000 & 0x00FF));
				break;

			default:
				sysOutPort8 (wComPortBase + DLL, (UINT8)(BAUD_9600 & 0x00FF));
				break;

		}

		sysOutPort8 (wComPortBase + DLH, (UINT8)(0));
		sysOutPort8 (wComPortBase + LCR, bTmp); // Clear DLAB bit

		/* Select 8N1 format */
		sysOutPort8 (wComPortBase + LCR, LCR_DATA8 | LCR_NONE | LCR_STOP1);

		/* Disable interrupts */
		sysOutPort8 (wComPortBase + IER, 0);

		/* Enable and reset FIFOs */
		sysOutPort8 (wComPortBase + FCR, FCR_FEN | FCR_CRF | FCR_CTF);

		iDebugChannel = 1;
	}

} /* sysDebugOpen () */


/*****************************************************************************
 * sysDebugClose: disable output to the debug channel
 *
 * RETURNS: None
 */

void sysDebugClose (void)
{
	iDebugChannel = 0;

} /* sysDebugClose () */


/*****************************************************************************
 * sysIsDebugopen: returns the status of debug port
 *
 * RETURNS: None
 */

int sysIsDebugopen (void)
{
	return iDebugChannel;

} /* sysIsDebugopen () */


/*****************************************************************************
 * sysDebugPrintf: write formatted string to the debug console
 *                 total string lengh must not exceed two lines (160 characters) 
 *
 * Includes special handling for CR/LF
 *
 * NOTE: it is preferable to use vDebugWrite() as it is Test Handler agnostic.
 *
 * RETURNS: None
 */

void sysDebugPrintf( char *format, ... )
{
	int i;
	va_list vp;
	char achBuffer[161]; /* 2 lines max */ 
	
	
	/* Exit now is debug channel not open */

	if (iDebugChannel == 0)
		return;

	xSemaphoreTake(globalMutexPool[MUTEX_DEBUG_STR_WRITE],portMAX_DELAY);
	
	/* format message */
 	va_start(vp, format);
	vsprintf( achBuffer, (const char *)format, vp );
	va_end(vp);

	/* Write to the debug console */  
	
	i = 0;
	
	while ( (achBuffer[i] != '\0') && (i < 161) )
	{
		sysDebugWriteChar ( achBuffer[i] );

		if (achBuffer[i] == '\n')
			sysDebugWriteChar ('\r');		/* append a CR to LF */

		i++;
	}

	xSemaphoreGive(globalMutexPool[MUTEX_DEBUG_STR_WRITE]);

} /* sysDebugPrintf() */


/*****************************************************************************
 * sysDebugWriteString: write a string to the debug console
 *
 * Includes special handling for CR/LF
 *
 * NOTE: it is preferable to use vDebugWrite() as it is Test Handler agnostic.
 *
 * RETURNS: None
 */

void sysDebugWriteString
(
	char*	buffer		/* pointer to a NULL terminated string */
)
{
	/* Exit now is debug channel not open */

	if (iDebugChannel == 0)
		return;


	/* Write string to the debug console */  

	xSemaphoreTake(globalMutexPool[MUTEX_DEBUG_STR_WRITE],portMAX_DELAY);

	while (*buffer != '\0')
	{
		sysDebugWriteChar( *buffer );

		if (*buffer == '\n')
			sysDebugWriteChar ('\r');		/* append a CR to LF */

		buffer++;
	}

	xSemaphoreGive(globalMutexPool[MUTEX_DEBUG_STR_WRITE]);

} /* sysDebugWriteString () */


/*****************************************************************************
 * sysAPDebugWriteString: write a string to the debug console when initialising
 * 						  APs must not be used for anything else
 *
 * Includes special handling for CR/LF
 *
 * RETURNS: None
 */

void sysAPDebugWriteString
(
	char*	buffer		/* pointer to a NULL terminated string */
)
{
	/* Exit now is debug channel not open */

	if (iDebugChannel == 0)
		return;

	while (*buffer != '\0')
	{
		sysAPDebugWriteChar (*buffer);

		if (*buffer == '\n')
			sysAPDebugWriteChar ('\r');		/* append a CR to LF */

		buffer++;
	}

} /* sysDebugWriteString () */



/*****************************************************************************
 * sysDebugWriteChar: write a character to the debug console
 *
 * RETURNS: None
 */

void sysDebugWriteChar
(
	char	c		/* character to write */
)
{
	/* Exit now is debug channel not open */

	if (iDebugChannel == 0)
		return;

	/* Write the character */
	xSemaphoreTake(globalMutexPool[MUTEX_DEBUG_CHAR_WRITE],portMAX_DELAY);

	sysSccTxChar (wComPortBase, c);

	xSemaphoreGive(globalMutexPool[MUTEX_DEBUG_CHAR_WRITE]);

} /* sysDebugWriteChar () */



/*****************************************************************************
 * sysAPDebugWriteChar: write a character to the debug console
 *
 * RETURNS: None
 */

void sysAPDebugWriteChar
(
	char	c		/* character to write */
)
{
	/* Exit now is debug channel not open */

	if (iDebugChannel == 0)
		return;

	sysSccTxChar (wComPortBase, c);

} /* sysDebugWriteChar () */




/*****************************************************************************
 * sysDebugFlush: wait until TX buffers are empty
 *
 * RETURNS: None
 */

void sysDebugFlush (void)
{
	/* Exit now is debug channel not open */

	if (iDebugChannel == 0)
		return;

	/* Wait for TX buffers to empty */

	xSemaphoreTake(globalMutexPool[MUTEX_DEBUG_FLUSH_STR],portMAX_DELAY);
	sysSccFlushTx (wComPortBase);
	xSemaphoreGive(globalMutexPool[MUTEX_DEBUG_FLUSH_STR]);

} /* sysDebugFlush () */

