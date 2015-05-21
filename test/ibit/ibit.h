
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

/*  $Header: /home/cvs/cvsroot/CCT_BIT_2/ibit/ibit.h,v 1.1 2013-09-04 07:27:43 chippisley Exp $
 *
 *  $Log: ibit.h,v $
 *  Revision 1.1  2013-09-04 07:27:43  chippisley
 *  Import files into new source repository.
 *
 *  Revision 1.11  2010/05/05 15:46:14  hmuneer
 *  Cute rev 0.1
 *
 *  Revision 1.10  2009/09/29 12:22:55  swilson
 *  Add command for throwing a GPF (for testing the exception handling mechanism).
 *
 *  Revision 1.9  2009/05/21 12:47:51  swilson
 *  Add help screen externals
 *
 *  Revision 1.8  2008/09/29 13:27:48  swilson
 *  Increase size of command token.
 *
 *  Revision 1.7  2008/09/29 10:21:47  swilson
 *  Rename BIT error action functions.
 *
 *  Revision 1.6  2008/09/17 15:51:51  swilson
 *  Remove use of global variables in IBIT directory - a hang over from the MON2 code
 *   this was based on.
 *
 *  Revision 1.5  2008/09/17 13:46:44  swilson
 *  Improvements to Local IBIT handler: add test execution counters and functions for display and zeroing them. Add continue on error function. Improve layout of results displayed when a test executes; include run and fail counts. Add support for VT100 compatible cursor-key handling within command-line editor. Move BIT execution support functions to a separate file BIT_UTIL.C. Make 'reset' command more robust.
 *
 *  Revision 1.4  2008/06/02 12:27:19  swilson
 *  Typo and layout fixes
 *
 *  Revision 1.3  2008/05/16 14:36:22  swilson
 *  Add test list management and test execution capabilities.
 *
 *  Revision 1.2  2008/05/15 15:32:40  swilson
 *  Add build and version tracking. Implement & test missing c-library functions.
 *
 *  Revision 1.1  2008/05/12 14:46:42  swilson
 *  Local Interactive BIT command-line interface and utility functions - based on MON2
 *   0.2.-13.
 *
 */

#ifndef __mon2_h
	#define __mon2_h

/* includes */

/* defines */

/* typedefs */

typedef struct tagGlobalParams
{
	int		iConfirm;		/* write confirmation mode on/off */
	int		iRadix;			/* number radix */
	int		iRepeatIndex;	/* tracks repeatable commands */

} PARAMS;

typedef struct tagCommandType
{
	int		iCmdType;
	char	achCmdString[9];
	void	(*pfnCmdHandler)();

} CMD_TYPE;

/* constants */

/* locals */

/* globals */

/* externals */

	/* bit_utils.c */

extern void vSetBitErrorAction (int iState);
extern void	vExecParseCmd (int loops, int startTok, int endTok, char* pTokens[]);

extern UINT32 dInitTestCounters (void);
extern void vZeroTestStats (void);
extern void vGetTestStats (UINT16 wTestNum, UINT32* pdRunCount, UINT32* pdFailCount);


	/* command.c */

extern void vInitCmdHistory (void);
extern int	iGetUserCommand (char* achUserCmd);
extern int	iProcessCommandLine (int iArgc, char* achArgv[], int* piCmdCount,
							char* achCmdArgs[]);
extern int	iGetCommandType (char* achTypeString, int* piCmdIndex);
extern void	vCommandDispatcher (int iCmdIndex, int iCmdCount,
							char* achCmdArgs[]);

	/* help.c */

extern int iHelpGetMaxPage (void);
extern void vHelpDisplayHeader (int iPageNum);
extern void vHelpDisplayPage (int iPageNum);
extern int iHelpPromptNext (void); 



	/* service.c */
extern void vExecuteTest (int iCmdType, int iCmdCount, char* achUserArgs[]);

extern void vTestSummary (int iCmdType, int iCmdCount, char* achUserArgs[]);


	/* util.c */
extern void vZeroCounters (void);

extern void vHelpScreen	(void);

extern void vVersionInfo (void);


/* forward declarations */

#endif

