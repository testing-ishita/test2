
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

/*
 * SUPPORT: support functions
 */


/* includes */

#include <stdio.h>

#include <private/sys_delay.h>

#include <bit/conslib.h>


/* defines */

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

void vTaskDelay( unsigned long xTicksToDelay );

/* forward declarations */


/****************************************************************************
 * iCheckForEsc: poll keyboard to see if user pressed ESC for loop breakout
 *
 * RETURNS: 1 if pressed, 0 otherwise
 */

int iCheckForEsc (void)
{
	int		iKeyPress;

	iKeyPress = iGetExtdKeyPress ();

	if (iKeyPress == 0x1B)
		return (1);

	return (0);

} /* iCheckforEsc () */


/****************************************************************************
 * iGetExtdKeyPress: poll keyboard to see if user pressed a key and return
 *
 * RETURNS: 0 if none pressed, otherwise extended (9-bit) keycode
 */

int iGetExtdKeyPress (void)
{
	int		iState = 0;
	int		iKey = 0;

	do
	{
		if(kbhit ())
	{
		iKey = getchar ();

		if (iKey == ESC)
		{
			if (iState == 0)
				iState = 1;
			else
				iState = 0;
		}

		else if (iKey == '[')
		{
			if (iState == 1)
				iState = 2;
			else
				iState = 0;
		}

		else
		{
			if (iState == 2)
				iKey += 0x100;

			iState = 0;
		}
		}
		else
			iState = 0;

		if(iState != 0)
		sysDelayMilliseconds (2);	/* 1 char @ 9600 baud ~= 1mS  */

	}while (iState != 0);
	
	if (iKey == 0)
	{
		/* No key pressed, let lower priority tasks run */
		vTaskDelay( 10 );
	}

	return (iKey);

} /* iGetExtdKeyPress () */


/****************************************************************************
 * iFilterWhitespace: replace TABs with spaces and remove multiple spaces
 *
 * The function returns the length of the processed string, so strings with
 * only spaces and TABs will have zero length on exit.
 *
 * RETURNS: the length of the processed string
 */

int iFilterWhitespace
(
	char*	achString
)
{
	int		iInIndex    = 0;
	int		iOutIndex   = 0;
	int		iWhitespace = 0;


	/* Loop until terminating NULL */

	do {
		/* Convert TAB to space */

		if (achString[iInIndex] == HTAB)
			achString[iInIndex] = ' ';

		/* Copy non-WS characters upto and including first WS */

		if (iWhitespace == 0)
		{
			/* Only need to copy if input pointer now leads output */

			if (iInIndex != iOutIndex)
				achString[iOutIndex] = achString[iInIndex];

			/* Check if character was WS */

			if (achString[iInIndex] == ' ')
				iWhitespace = 1;

			/* Ignore first char if WS */

			if ((iWhitespace == 0) || (iOutIndex > 0))
				iOutIndex++;
		}

		/* Ignore characters until next non-WS */

		else if (achString[iInIndex] != ' ')
		{
			iWhitespace = 0;

			if (iInIndex != iOutIndex)
				achString[iOutIndex] = achString[iInIndex];

			iOutIndex++;
		}

		iInIndex++;

	} while (achString[iInIndex] != NULL);

	/* If last char was whitespace, strip it */

	if ((iOutIndex > 0) && (achString[iOutIndex - 1] == ' '))
		iOutIndex--;

	/* Terminate the new string */

	achString[iOutIndex] = NULL;

	return (iOutIndex);

} /* iFilterWhitespace () */


