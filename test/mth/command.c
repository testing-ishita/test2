
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
 * command.c: command interpretation and processing + support
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/mth/command.c,v 1.3 2014-07-24 14:05:12 mgostling Exp $
 * $Log: command.c,v $
 * Revision 1.3  2014-07-24 14:05:12  mgostling
 * Added condtitionals around TCB, RUN and LKADDR commands.
 *
 * Revision 1.2  2013-09-26 12:42:10  chippisley
 * Added support for network Soak Master Client.
 *
 * Revision 1.1  2013/09/04 07:44:08  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2012/11/02 14:17:25  chippisley
 * Added "TL" (task list) command.
 *
 * Revision 1.4  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.3  2010/11/04 17:43:15  hchalla
 * Added support to boot to bios from ibit prompt.
 *
 * Revision 1.2  2010/06/24 15:42:14  hchalla
 * Code Cleanup, Added headers and comments.
 *
 */




/* includes */

#include <stdio.h>
#include <string.h>
#include <errors.h>
#include <bit/conslib.h>
#include "mth.h"
#include "config.h"

/* defines */

#define PF_CAST			(void(*)())	/* function pointer cast */

#define MAX_COMMAND		77		/* must fit on a single screen line! */
#define MAX_HISTORY		16		/* 16 levels of command recall */


/* typedefs */

/* constants */

extern void vTdbgTcbShow( int iCmdType, int iCmdCount, char* achUserArgs[] );
extern void vTdbgRunFunc( int iCmdType, int iCmdCount, char* achUserArgs[] );
extern void vTdbgLkupSymbol( int iCmdType, int iCmdCount, char* achUserArgs[] );

static const CMD_TYPE	asCommands[] =
{
	{TYPE__QUIT, 			"RESET",	PF_CAST vReset				},
	{TYPE__SWITCH,			"R2",		PF_CAST vRadix2			    },
	{TYPE__SWITCH, 			"R10",		PF_CAST vRadix10 		    },
	{TYPE__SWITCH, 			"R16",		PF_CAST vRadix16		    },
	{TYPE__HELP, 			"HELP",		PF_CAST vMTHHelpScreen		},
	{TYPE__HELP, 			"H",		PF_CAST vMTHHelpScreen		},
	{TYPE__HELP, 			"?",		PF_CAST vMTHHelpScreen		},
	{TYPE__TEST,			"B",		PF_CAST vBIOS				},
	{TYPE__TEST,			"TL",		PF_CAST vMTHTaskList		},
#ifdef INCLUDE_DBGLOG
	{TYPE__TEST,			"DBLP",		PF_CAST vMTHDblPrint		},
	{TYPE__TEST,			"DBLC",		PF_CAST vMTHDblClear		},
#endif
#ifdef INCLUDE_TASK_DEBUG
	{TYPE__TEST,			"TCB",		PF_CAST vTdbgTcbShow		},
	{TYPE__SYMBOL,			"RUN",		PF_CAST vTdbgRunFunc		},
#endif
#ifdef INCLUDE_SYMBOLS
	{TYPE__TEST,			"LKADDR",	PF_CAST vTdbgLkupSymbol		},
#endif
#ifdef INCLUDE_NETWORK	
	{TYPE__TEST,			"NETSTATS",	PF_CAST vMTHNetStats		},
#ifdef INCLUDE_LWIP_SMC
	{TYPE__TEST,			"SMCSTATS",	PF_CAST vMTHSmcStats		},
#endif
#endif	
	{TYPE__HELP, 			"CUTEID",	PF_CAST vMTHVersionInfo		},
	{TYPE__DISPLAY_BYTE, 	"DB",		PF_CAST vDisplayMemory		},
	{TYPE__DISPLAY_WORD, 	"DW",		PF_CAST vDisplayMemory		},
	{TYPE__DISPLAY_DWORD, 	"DD",		PF_CAST vDisplayMemory		},
	{TYPE__SET_BYTE, 		"SB",		PF_CAST vSetMemory			},
	{TYPE__SET_WORD,	 	"SW",		PF_CAST vSetMemory			},
	{TYPE__SET_DWORD,	 	"SD",		PF_CAST vSetMemory			},
	{TYPE__INPUT_BYTE,	 	"IB",		PF_CAST vPortInput			},
	{TYPE__INPUT_WORD, 		"IW",		PF_CAST vPortInput			},
	{TYPE__INPUT_DWORD, 	"ID",		PF_CAST vPortInput			},
	{TYPE__OUTPUT_BYTE, 	"OB",		PF_CAST vPortOutput			},
	{TYPE__OUTPUT_WORD, 	"OW",		PF_CAST vPortOutput			},
	{TYPE__OUTPUT_DWORD, 	"OD",		PF_CAST vPortOutput			},
	{TYPE__PCI_SCAN,	 	"PS",		PF_CAST vPciScan		    },
	{TYPE__PCI_READ_BYTE,	"PRB",		PF_CAST vPciRead		    },
	{TYPE__PCI_READ_WORD, 	"PRW",		PF_CAST vPciRead		    },
	{TYPE__PCI_READ_DWORD, 	"PRD",		PF_CAST vPciRead		    },
	{TYPE__PCI_WRITE_BYTE, 	"PWB",		PF_CAST vPciWrite			},
	{TYPE__PCI_WRITE_WORD, 	"PWW",		PF_CAST vPciWrite			},
	{TYPE__PCI_WRITE_DWORD,	"PWD",		PF_CAST vPciWrite			},
	{TYPE__TEST,			"TM",		PF_CAST vTestMenu			},
	{TYPE__TEST,			"T",		PF_CAST vMTHExecuteTest		},
	{TYPE__TEST,			"SUM",		PF_CAST vMTHTestSummary		},
	{TYPE__TEST,			"Z",		PF_CAST vMTHZeroCounters	},
	{TYPE__TEST,			"TEA_HALT",	PF_CAST vBeaHalt			},
	{TYPE__TEST,			"TEA_CONT",	PF_CAST vBeaContinue		},
	{TYPE__UNKNOWN,		 	"",			NULL}
};


/* locals */

static int	iHistCount;
static char	achCommandHistory[MAX_HISTORY][MAX_COMMAND];

static int	iInsert = 0;	/* set to insert by default */


/* globals */

/* externals */

/* forward declarations */


static void vAddToHistory (char* achCommand);



/****************************************************************************
 * vInitCmdHistory: initialise the command history
 *
 * RETURNS: None
 */

void vMTHInitCmdHistory (void)
{
	int		iIndex;


	for (iIndex = 0; iIndex < MAX_HISTORY; iIndex++)
		achCommandHistory[iIndex][0] = NULL;

	iHistCount = 0;

} /* vInitCmdHistory () */





/****************************************************************************
 * iGetUserCommand: retreive a single-line command from the user
 *
 * This function supports full command editing and command history recall
 * using the cursor keys. Exits only when <CR> is entered, string can be
 * blank.
 *
 * RETURNS: E__OK or E__ABORT_ENTRY
 */

int iMTHGetUserCommand (char* achCommand)
{
	int		iIndex;
	int		iKey;
	int     reprint = 0;
	int		iLen	= 0;
	int		iHindex = 0;
	int		iXpos   = 0;


	/* Initialize user input field and cursor */
	cputs ("> ");

	/* Process user input */

	achCommand[0] = NULL;

	do {
		/* Get a key */

		reprint = 0;
		iKey = iGetExtdKeyPress ();

		/*******************************************************/
		/* Very special case - '=' as first char = repeat last */
		/*******************************************************/

		if ((iXpos == 0) && (iKey == '='))
		{
			puts ("");
			return (E__REPEAT);
		}

		/* Process whole-command based keys */

		if (iKey == ESC)
		{
			iHindex = 0;
			iXpos 	= 0;
			achCommand[0] = NULL;
			vRedrawPrompt ();
		}

		else if ((iKey == CUR_UP) && (iHindex < iHistCount))
		{
			reprint = 1;
			iHindex++;
			strcpy (achCommand, achCommandHistory[iHindex - 1]);
			iXpos = strlen (achCommand);
			vRedrawPrompt ();
		}

		else if ((iKey == CUR_DOWN) && (iHindex > 1))
		{
			reprint = 1;
			iHindex--;
			strcpy (achCommand, achCommandHistory[iHindex - 1]);
			iXpos = strlen (achCommand);
			vRedrawPrompt ();
		}

		/* Process character based keys */

		iLen = strlen (achCommand);		/* current command length */

		if (iKey == CUR_HOME)
		{
			reprint = 1;
			iXpos = 0;
		}

		else if (iKey == CUR_END)
		{
			reprint = 1;
			iXpos = iLen;
		}

		else if ((iKey == CUR_LEFT) && (iXpos > 0))
		{
			reprint = 1;
			iXpos--;
		}

		else if ((iKey == CUR_RIGHT) && (iXpos < iLen))
		{
			reprint = 1;
			iXpos++;
		}

		else if (iKey == CUR_INS)
		{
			reprint = 1;
			iInsert ^= 1;
		}

		else if ((iKey == CUR_DEL) && (iXpos < iLen))
		{
			reprint = 1;
			strcpy (&achCommand[iXpos], &achCommand[iXpos + 1]);
		}
		else if ((iKey == BS) && (iXpos > 0))
		{
			reprint = 1;
			iXpos--;
			strcpy (&achCommand[iXpos], &achCommand[iXpos + 1]);
		}

		else if ((iKey >= 0x20) && (iKey <= 0x7F) && (iXpos < MAX_COMMAND-1))  /* acceptable char? */
		{
			if (iInsert != 0)
			{
				reprint = 1;
				for (iIndex = iLen + 1; iIndex > iXpos; iIndex--)
					achCommand[iIndex] = achCommand[iIndex - 1];
			}
			else
				reprint = 0;

			putchar (iKey);
			achCommand[iXpos] = iKey;
			iXpos++;

			if (iXpos == (iLen + 1))
				achCommand[iXpos] = NULL;
		}

		/* Print the command line as is, but only if things have changed */

		if ( (iKey != 0) && (reprint == 1))
		{
			reprint = 0;

			vShowCommandLine (achCommand, iLen, iXpos);

			/* Set the cursor type */
		}

	} while (iKey != CR);

	putchar ('\n');

	/* If the result was an empty string, signal abort */

	if (iFilterWhitespace (achCommand) == 0)
		return (E__ABORT_ENTRY);


	vAddToHistory (achCommand);

	return (E__OK);

} /* iGetUserCommand () */



/****************************************************************************
 * iProcessCommandLine: process arguments supplied as a command line
 *
 * RETURNS: E__OK or an E__... error code
 */

int iMTHProcessCommandLine
(
	int		iArgc,			/* number of arguments supplied */
	char*	achArgv[],		/* arguments supplied */
	int*	piCmdCount,		/* number of command arguments */
	char*	achCmdArgs[]	/* command arguments */
)
{
	int		iCmdCount;		/* command argument count */
	int		iCmdIndex;		/* index to command parameters */
	int		iType;			/* command type code */
	int		iIndex;			/* argument processing index */
	char	achBuffer[80];


	/* Process each element in the command, act on switches and trivial
	 * commands now, otherwise construct array of command arguments for
	 * later processing
	 */

	iCmdCount = 0;

	for (iIndex = 1; iIndex < iArgc; iIndex++)
	{
	
		if ( iIndex == 1 )
		{
			strupr (achArgv[iIndex]);

			iType = iMTHGetCommandType (achArgv[iIndex], &iCmdIndex);
		}
		else if ( iIndex == 2 )
		{
			/* For symbol name lookup we don't want all uppercase */
			if ( iType != TYPE__SYMBOL ) 
			{
				strupr (achArgv[iIndex]);
			} 
		}
		else
		{
			strupr (achArgv[iIndex]);
		}
		
		/* sysDebugPrintf( "%d %s\n", iIndex, achArgv[iIndex]); */

		/* Process switches here */

		if (iType == TYPE__SWITCH)
		{
			(void) (asCommands[iCmdIndex].pfnCmdHandler) ();
		}

		/* Help? */

		else if (iType == TYPE__HELP)
		{
			(void) (asCommands[iCmdIndex].pfnCmdHandler) ();
		}

		/* For command and parameters, process into an argument array */

		else
		{
			if ((iCmdCount == 0) && (iType == TYPE__UNKNOWN))
			{
				sprintf (achBuffer, "unknown command/switch: %s\n",
									achArgv[iIndex]);
				vWriteErrorString (achBuffer);

				return (E__UNKNOWN_SWITCH);
			}

			achCmdArgs[iCmdCount++] = achArgv[iIndex];
		}
	}

	*piCmdCount = iCmdCount;

	return (E__OK);

} /* iProcessCommandLine () */


/****************************************************************************
 * iGetCommandType: look up the supplied command
 *
 * RETURNS: command type, or TYPE__UNKNOWN if not found
 */

int iMTHGetCommandType
(
	char*	achTypeString,
	int*	piCmdIndex
)
{
	*piCmdIndex = 0;

	while (asCommands[*piCmdIndex].iCmdType != TYPE__UNKNOWN)
	{
		if (strcmp (achTypeString, asCommands[*piCmdIndex].achCmdString) == 0)
			return (asCommands[*piCmdIndex].iCmdType);

		(*piCmdIndex)++;
	}

	return (TYPE__UNKNOWN);

} /* iGetCommandType () */


/****************************************************************************
 * vCommandDispatcher: call the appropriate command handler
 *
 * RETURNS: none (errors handled local to each function )
 */

void vMTHCommandDispatcher
(
	int		iCmdIndex,		/* command type index */
	int		iCmdCount,		/* number of command arguments */
	char*	achCmdArgs[]	/* array of command arguments */
)
{
	(asCommands[iCmdIndex].pfnCmdHandler)
					(
						asCommands[iCmdIndex].iCmdType,
						iCmdCount,
						achCmdArgs
					);

} /* vCommandDispatcher () */




/****************************************************************************
 * vAddToHistory: add a string to the command history
 *
 * If the history buffer is full the oldest is lost when the new one is
 * added. Newest entry is 0.
 *
 * RETURNS: None
 */

static void vAddToHistory
(
	char*	achCommand
)
{
	int iIndex;


	for (iIndex = (MAX_HISTORY - 1); iIndex >= 1; iIndex--)
		strcpy (achCommandHistory[iIndex], achCommandHistory[iIndex - 1]);

	strcpy (achCommandHistory[0], achCommand);

	if (iHistCount < MAX_HISTORY)
		iHistCount++;

} /* vAddToHistory () */











