
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



#include <stdio.h>
#include <string.h>
#include <errors.h>
#include <bit/conslib.h>
#include <bit/bit.h>
#include <private/sys_delay.h>
#include <ctype.h>

static int iGetNumericArg (int* piArgIndex, int iCmdCount, char* achCmdArgs[],
							UINT32* pdTemp, UINT32 dMin, UINT32 dMax);
static void vIdentifyMissingParams (UINT16 wReqBitmap);
static void vSetDefaultArgs (ARGS* psArgs);
int  iExtractNumber (char* achString, UINT32* pdData, UINT32 dMinimum,
		                    UINT32 dMaximum);

/****************************************************************************
 * vRedrawPrompt: draw the command prompt, blanking the remaining line
 *
 * RETURNS: None
 */

void vRedrawPrompt (void)
{
	cputs ("\r");
	cputs ("                                                                              \r");
	cputs ("> ");

} /* vRedrawPrompt () */


/****************************************************************************
 * vWriteErrorString: print an error message
 *
 * RETURNS: None
 */

void vWriteErrorString
(
	char*	achErrorString
)
{
	cputs ("Error: ");
	puts (achErrorString);

} /* vWriteErrorString () */



/****************************************************************************
 * iExtractNumber: convert string to a numeric value, allowing radix override
 *
 * Uses either the default radix, or a radix override prefix to support
 * decimal, hexidecimal and binary formats.
 *
 * RETURNS: E__OK or an E__... error code
 */

int iExtractNumber
(
	char*	achString,
	UINT32*	pdData,
	UINT32	dMinimum,
	UINT32	dMaximum
)
{
	UINT32	dValue;				/* translated value */
	UINT32	dMult;				/* multiplier */
	int		iRadix = 0;
	int		iLength;
	int		iIndex;
	char	achBuffer[80];


	iLength = strlen (achString);

	/* Check for radix override in the number string */

	if ((iLength > 2) && (achString[iLength-2] == '.'))
	{
		if (achString[iLength-1] == 'H')
		{
			iRadix = 16;
		}

		else if (achString[iLength-1] == 'D')
		{
			iRadix = 10;
		}

		else if (achString[iLength-1] == 'B')
		{
			iRadix = 2;
		}
		else
		{
			sprintf (achBuffer, "Unknown numeric format: %s", achString);
			vWriteErrorString (achBuffer);

			return (E__FORMAT_ERROR);
		}
	}

	if (iRadix == 0)
	{
		iRadix = iGetRadix ();					/* no override, use default */
	}
	else
	{
		iLength -= 2;							/* remove override character */
		achString[iLength] = NULL;
	}


	/* Process sting using selected radix */

	dValue = 0;
	dMult  = 1;

	switch (iRadix)
	{
		case 2:

			for (iIndex = iLength - 1; iIndex >= 0; iIndex--)
			{
				if ((achString[iIndex] != '0') && (achString[iIndex] != '1'))
				{
					sprintf (achBuffer, "Invalid BINARY value: %s", achString);
					vWriteErrorString (achBuffer);

					return (E__FORMAT_ERROR);
				}

				dValue += dMult * (achString[iIndex] - '0');
				dMult *= 2;
			}

			break;

		case 10:

			for (iIndex = iLength - 1; iIndex >= 0; iIndex--)
			{
				if ((achString[iIndex] < '0') || (achString[iIndex] > '9'))
				{
					sprintf (achBuffer, "Invalid DECIMAL value: %s", achString);
					vWriteErrorString (achBuffer);

					return (E__FORMAT_ERROR);
				}

				dValue += dMult * (achString[iIndex] - '0');
				dMult *= 10;
			}

			break;

		case 16:

			for (iIndex = iLength - 1; iIndex >= 0; iIndex--)
			{
				if ((achString[iIndex] >= '0') &&
					(achString[iIndex] <= '9'))
				{
					dValue += dMult * (achString[iIndex] - '0');
				}

				else if ((achString[iIndex] >= 'A') &&
						 (achString[iIndex] <= 'F'))
				{
					dValue += dMult * (achString[iIndex] - 'A' + 10);
				}

				else
				{
					sprintf (achBuffer, "Invalid HEX value: %s", achString);
					vWriteErrorString (achBuffer);

					return (E__FORMAT_ERROR);
				}

				dMult *= 16;
			}

			break;
	}

	if ((dValue > dMaximum) || (dValue < dMinimum))
	{
		return (E__VALUE_ERROR);
	}

	*pdData = dValue;

	return (E__OK);

} /* iExtractNumber () */


/****************************************************************************
 * vSetDefaultArgs: set command arguments to default values
 *
 * RETURNS: None
 */

static void vSetDefaultArgs
(
	ARGS*	psArgs
)
{
	psArgs->dAddr		= 0L;
	psArgs->dData		= 0L;
	psArgs->dLength		= 1L;			/* one item */
	psArgs->dCount		= 1L;			/* one itteration */
	psArgs->bBus		= 0;
	psArgs->bDev		= 0;
	psArgs->bFunc		= 0;
	psArgs->bRegister	= 0;

} /* vSetDefaultArgs () */




/****************************************************************************
 * vIdentifyMissingParams: convert missing parameter bitmap to error message
 *
 * RETURNS: None
 */

static void vIdentifyMissingParams
(
	UINT16	wReqBitmap
)
{
	char	achBuffer[80];
	int		iLength;
	int		iIndex;


	strcpy (achBuffer, "Command is missing ");


	if ((wReqBitmap & ARG__ADDR) != 0)
		strcat (achBuffer, "- Address");

	if ((wReqBitmap & ARG__DATA) != 0)
		strcat (achBuffer, "- Data");

	if ((wReqBitmap & ARG__LENGTH) != 0)
		strcat (achBuffer, "- Length");

	if ((wReqBitmap & ARG__LOOP) != 0)
		strcat (achBuffer, "- Loop Count");

	if ((wReqBitmap & ARG__BUS) != 0)
		strcat (achBuffer, "- Bus#");

	if ((wReqBitmap & ARG__DEV) != 0)
		strcat (achBuffer, "- Device#");

	if ((wReqBitmap & ARG__FUNC) != 0)
		strcat (achBuffer, "- Function#");

	if ((wReqBitmap & ARG__REGISTER) != 0)
		strcat (achBuffer, "- Register#");


	/* Convert all but first '-' to ',' */

	iLength = strlen (achBuffer);
	iIndex = 0;

	while (achBuffer[iIndex++] != '-');

	for (; iIndex < iLength; iIndex++)
	{
		if (achBuffer[iIndex] == '-')
			achBuffer[iIndex] = ',';
	}


	/* Write the error message */

	vWriteErrorString (achBuffer);

} /* vIdentifyMissingParams () */




/****************************************************************************
 * vShowCommandLine: draw command line, handling cursor and character deletes
 *
 * RETURNS: None
 */

void vShowCommandLine
(
	char*	achCommand,
	int		iCommandLen,
	int		iCursorPos
)
{
	int	i;


	/* Print out the full command */

	putchar ('\r');
	cputs ("> ");
	cputs (achCommand);

	/* Print a space to handle deletes, but move cursor back to line end */

	putchar (' ');
	putchar ('\b');

	/* If cursor is not at the line end, go back to start and print out the
	 * command up to the cursor
	 */

	if (iCommandLen != iCursorPos)
	{
		putchar ('\r');
		cputs ("> ");

		for (i = 0; i <	iCursorPos; i++)
			putchar (achCommand[i]);
	}

} /* vShowCommandLine () */


/****************************************************************************
 * vSetDefaults: set parameter defaults
 *
 * RETURNS: None
 */

void vSetDefaults (void)
{
	vConfirmOn ();
	vRadix16 ();
	vBeaHalt ();
	vSetRepeatIndex (-1);

} /* vSetDefaults () */



/****************************************************************************
 * iGetNumericArg: process a numeric argument
 *
 * RETURNS: E__OK or an E__... error code
 */

static int iGetNumericArg
(
	int*	piArgIndex,
	int		iCmdCount,
	char*	achCmdArgs[],
	UINT32*	pdTemp,
	UINT32	dMin,
	UINT32	dMax
)
{
	int		iParamFail = E__OK;
	int		iIndex;


	iIndex = *piArgIndex;

	while ((iIndex < iCmdCount) && (achCmdArgs[iIndex][0] == NULL))
		iIndex++;

	if (iIndex == iCmdCount)
		iParamFail = E__NO_MORE_PARAMS;

	else
	{
		iParamFail = iExtractNumber (achCmdArgs[iIndex++],
										pdTemp, dMin, dMax);
	}

	*piArgIndex = iIndex;

	return (iParamFail);

} /* iGetNumericArg () */





/****************************************************************************
 * iExtractArgs: extract command arguments from a command line package
 *
 * First all the ':' options are located, extracted and the strings set to
 * NULL. Then the mandatory arguments are extracted in order, finally any
 * optional arguments are extracted.
 *
 * RETURNS: E__OK or an E__... error code
 */

int iExtractArgs
(
	ARGS*	psArgs,			/* test arguments structure */
	UINT16	wReqBitmap,		/* bitmap defining mandatory arguments */
	UINT16	wOptBitmap,		/* bitmap defining optional arguments */
	int		iCmdCount,		/* number of command arguments passed in */
	char*	achCmdArgs[]	/* the command argument strings */
)
{
	UINT32	dTemp;
	int		iIndex;
	int		iParamFail = E__OK;
	char	achBuffer[80];


	vSetDefaultArgs (psArgs);		/* set defaults for those not supplied */

	/* first find any ':' options. Any that are found are removed
	 * from the 'required' bitmap
	 */

	for (iIndex = 1; iIndex < iCmdCount; iIndex++)
	{
		if  (achCmdArgs[iIndex][1] == ':')
		{
			switch (achCmdArgs[iIndex][0])
			{
				case 'B':	/* bus */

					iParamFail = iExtractNumber (&achCmdArgs[iIndex][2],
										&dTemp, 0, 31);
					psArgs->bBus = (UINT8)dTemp;

					wReqBitmap &= ~ARG__BUS;
					wOptBitmap &= ~ARG__BUS;

					if (iParamFail == E__VALUE_ERROR)
					{
						sprintf (achBuffer, "Invalid Bus Number. Must be 0-31");
						vWriteErrorString (achBuffer);
					}
					break;

				case 'F':	/* function */

					iParamFail = iExtractNumber (&achCmdArgs[iIndex][2],
										&dTemp, 0, 7);
					psArgs->bFunc = (UINT8)dTemp;

					wReqBitmap &= ~ARG__FUNC;
					wOptBitmap &= ~ARG__FUNC;

					if (iParamFail == E__VALUE_ERROR)
					{
						sprintf (achBuffer, "Invalid Function Number. Must be 0-7");
						vWriteErrorString (achBuffer);
					}
					break;

				case 'L':	/* loop */

					iParamFail = iExtractNumber (&achCmdArgs[iIndex][2],
										&psArgs->dCount, 0, 0xFFFFFFFFL);

					if (iParamFail == E__OK)
					{
						if (psArgs->dCount == 0)
							psArgs->dCount = 0xFFFFFFFFL;

						wReqBitmap &= ~ARG__LOOP;
						wOptBitmap &= ~ARG__LOOP;
					}
					break;

				default:
					break;
			}

			achCmdArgs[iIndex][0] = NULL;
		}

		if ((iParamFail != E__OK) && (iParamFail != E__NO_MORE_PARAMS))
			return (iParamFail);
	}


	/* Process mandetory non ':' parameters in order */

	iIndex = 1;

	if ((wReqBitmap & ARG__ADDR) != 0)		/* address first */
	{
		iParamFail = iGetNumericArg (&iIndex, iCmdCount, achCmdArgs,
										&dTemp, 0, 0xFFFFFFFFL);
		if (iParamFail == E__OK)
		{
			psArgs->dAddr = dTemp;
			wReqBitmap &= ~ARG__ADDR;
		}

		else
		{
			if (iParamFail == E__NO_MORE_PARAMS)
				vIdentifyMissingParams (wReqBitmap);

			return (iParamFail);
		}
	}

	if ((wReqBitmap & ARG__DEV) != 0)		/* or device first */
	{
		iParamFail = iGetNumericArg (&iIndex, iCmdCount, achCmdArgs,
										&dTemp, 0, 31);
		if (iParamFail == E__OK)
		{
			psArgs->bDev = (UINT8)dTemp;
			wReqBitmap &= ~ARG__DEV;
		}

		else
		{
			if (iParamFail == E__NO_MORE_PARAMS)
				vIdentifyMissingParams (wReqBitmap);

			else if (iParamFail == E__VALUE_ERROR)
			{
				sprintf (achBuffer, "Invalid Device Number. Must be 0-31");
				vWriteErrorString (achBuffer);
			}

			return (iParamFail);
		}
	}

	if ((wReqBitmap & ARG__REGISTER) != 0)	/* then register next */
	{
		iParamFail = iGetNumericArg (&iIndex, iCmdCount, achCmdArgs,
										&dTemp, 0, 0xFF);
		if (iParamFail == E__OK)
		{
			psArgs->bRegister = (UINT8)dTemp;
			wReqBitmap &= ~ARG__REGISTER;
		}

		else
		{
			if (iParamFail == E__NO_MORE_PARAMS)
				vIdentifyMissingParams (wReqBitmap);

			else if (iParamFail == E__VALUE_ERROR)
			{
				sprintf (achBuffer, "Invalid Register value. Must be 0-FFh");
				vWriteErrorString (achBuffer);
			}

			return (iParamFail);
		}
	}

	if ((wReqBitmap & ARG__DATA) != 0)		/* and / or data next */
	{
		iParamFail = iGetNumericArg (&iIndex, iCmdCount, achCmdArgs,
										&dTemp, 0, 0xFFFFFFFFL);
		if (iParamFail == E__OK)
		{
			psArgs->dData = dTemp;
			wReqBitmap &= ~ARG__DATA;
		}

		else
		{
			if (iParamFail == E__NO_MORE_PARAMS)
				vIdentifyMissingParams (wReqBitmap);

			return (iParamFail);
		}
	}


	/* Process optional non ':' parameters in order */

	if ((wOptBitmap & ARG__LENGTH) != 0)	/* length first */
	{
		iParamFail = iGetNumericArg (&iIndex, iCmdCount, achCmdArgs,
										&dTemp, 0, 0xFFFFFFFFL);
		if (iParamFail == E__OK)
			psArgs->dLength = dTemp;
	}

	return (E__OK);

} /* iExtractArgs () */



/****************************************************************************
 * vProcessUserCmd: process a single line user command into arguments
 *
 * RETURNS: None
 */

void vProcessUserCmd
(
	char*	achUserCmd,		/* user typed command */
	int*	piCmdCount,		/* number of command elements */
	char*	achCmdArgs[]	/* array of command elements */
)
{
	int		iIndex;
	int		iCmdCount = 1;
	char*	pchStart;


	/* Ensure the user string has a space at the end (to assist scan) */

	strcat (achUserCmd, " ");


	/* Quickly; convert any TABs or commas to spaces */

	iIndex = 0;

	while (achUserCmd[iIndex] != NULL)
	{
		if ((achUserCmd[iIndex] == 0x09) || (achUserCmd[iIndex] == ','))
			achUserCmd[iIndex] = ' ';

		iIndex++;
	}

	/* Extract each argument from the command line, spaces will delimit */

	iIndex = 0;

	while (achUserCmd[iIndex] != NULL)
	{
		while (achUserCmd[iIndex] == ' ')
			iIndex++;

		pchStart = &achUserCmd[iIndex];

		if (achUserCmd[iIndex] != NULL)
		{
			while (achUserCmd[iIndex] != ' ')
				iIndex++;

			achUserCmd[iIndex++] = NULL;
			achCmdArgs[iCmdCount++] = pchStart;
		}
	}

	*piCmdCount = iCmdCount;

} /* vProcessUserCmd () */



/***************************************************************************
 * vMTHExecPostProc: post processing of the command line for a 'T' command
 *
 * Reconstruct the original typed command, but with some additional text
 * processing to ensure all syntactic elements are separated by spaces
 *
 * RETURNS: none
 */

void vExecPostProc
(
	char*	achCommand,
	char*	achArg
)
{
	int		i;
	int		n;
	char	last = 0;


	n = strlen (achCommand);

	if (n > 0)
	{
		if (achCommand[n-1] != ' ')
			achCommand[n++] = ' ';

		last = achCommand[n-1];
	}

	/* Process argument */

	for (i = 0; achArg[i] != NULL; i++)
	{
		/* 0-9, A-F */

		if (isxdigit (achArg[i]))
			achCommand[n++] = achArg[i];

		/* Formatting characters */

		else if ((achArg[i] == '{') ||
			(achArg[i] == '}') ||
			(achArg[i] == '(') ||
			(achArg[i] == ')') ||
			(achArg[i] == ':') ||
			(achArg[i] == '-'))
		{
			if ((n > 0) && (last != ' '))
				achCommand[n++] = ' ';

			achCommand[n++] = achArg[i];
			achCommand[n++] = ' ';
		}

		/* Therefore, must be a space */

		else
		{
			if ((n > 0) && (last != ' '))
				achCommand[n++] = ' ';
		}

		last = achCommand[n-1];
	}

	/* Don't end with a space */

	if (last == ' ')
		achCommand[n-1] = 0;

} /* vExecPostProc () */


/***************************************************************************
 * iExecTokenize: break the reconstructed command into tokens
 *
 * The command line currently has all elements separated by spaces
 *
 * RETURNS: number of tokens created
 */

int iExecTokenize
(
	char*	achCommand,
	char*	tokens[]
)
{
	int		t = 0;
	int		i;


	tokens[t++] = achCommand;

	for (i = 0; *achCommand != NULL; i++)
	{
		if (*achCommand == ' ')
		{
			tokens[t++] = achCommand + 1;
			*achCommand = 0;
		}

		achCommand++;
	}

	return t;

} /* iExecTokenize () */


/***************************************************************************
 * vShowExecutionTime: compute and display test execution time
 *
 * There is no built-in div64, so we need to reduce the dividend to 32-bits
 * before proceding with the calculation.
 *
 * At 2GHz, the carry into upper 32-bits occurs after 2.147 seconds, so if we
 * divide by 4096, that gives ~2h25 before the count wraps AND a timed
 * resolution of ~4mS.
 *
 * This mechanism has count headroom even at 3 * this CPU frequency.
 *
 * RETURNS: none
 */

void vShowExecutionTime
(
	UINT64	qStartTicks,
	UINT64	qEndTicks
)
{
	UINT32	dExecTime;
	char	achBuffer[40];

	dExecTime = (UINT32)((qEndTicks - qStartTicks) >> 12);	/* divide count delta by 4096 */
	dExecTime = dExecTime / sysGetCpuFrequency ();			/* divide by frequency in MHz */
	dExecTime = (dExecTime * 4096) / 1000;					/* convert result to mS */

	if (dExecTime >= 10)	/* minimum count displayed here is 10mS */
	{
		sprintf (achBuffer, "** Execution time: %d.%02d S **\n",
				dExecTime / 1000, (dExecTime % 1000) / 10);
		vDebugWrite (achBuffer);
	}
	else
		vDebugWrite ("** Execution time: < 0.01 S **\n");

} /* vShowExecutionTime () */


