
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


/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/mth/mth.h,v 1.2 2013-09-26 12:42:10 chippisley Exp $
 * $Log: mth.h,v $
 * Revision 1.2  2013-09-26 12:42:10  chippisley
 * Added support for network Soak Master Client.
 *
 * Revision 1.1  2013/09/04 07:44:08  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2012/11/02 14:17:55  chippisley
 * Added vMTHTaskList().
 *
 * Revision 1.3  2012/03/27 07:58:52  cdobson
 * Added extern to clear comiler warning.
 *
 * Revision 1.2  2010/06/24 15:52:52  hchalla
 * Added version control header.
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

	/* mth_utils.c */

//extern void vSetBitErrorAction (int iState);
extern void	vMTHExecParseCmd (int loops, int startTok, int endTok, char* pTokens[]);
extern UINT32 dMTHInitTestCounters (void);
extern void vMTHZeroTestStats (void);
extern void vMTHGetTestStats (UINT16 wTestNum, UINT32* pdRunCount, UINT32* pdFailCount);
extern int  iExtractNumber (char* achString, UINT32* pdData, UINT32 dMinimum, UINT32 dMaximum);


	/* command.c */
extern void vMTHInitCmdHistory (void);
extern int	iMTHGetUserCommand (char* achUserCmd);
extern int	iMTHProcessCommandLine (int iArgc, char* achArgv[], int* piCmdCount,
							char* achCmdArgs[]);
extern int	iMTHGetCommandType (char* achTypeString, int* piCmdIndex);
extern void	vMTHCommandDispatcher (int iCmdIndex, int iCmdCount,
							char* achCmdArgs[]);

	/* help.c */
extern int iMTHHelpGetMaxPage (void);
extern void vMTHHelpDisplayHeader (int iPageNum);
extern void vMTHHelpDisplayPage (int iPageNum);
extern int iMTHHelpPromptNext (void);


	/* service.c */
extern void vMTHExecuteTest (int iCmdType, int iCmdCount, char* achUserArgs[], int (*iMTHExtractArgs)());
extern void vMTHTestSummary (int iCmdType, int iCmdCount, char* achUserArgs[], int (*iMTHExtractArgs)());

	/* util.c */
extern void vMTHZeroCounters (void);
extern void vMTHHelpScreen	(void);
extern void vMTHVersionInfo (void);
extern void vMTHTaskList (void);

extern void vMTHDblPrint (void);
extern void vMTHDblClear (void);

extern void vMTHNetStats (void);
extern void vMTHSmcStats (void); 

/* forward declarations */

#endif

