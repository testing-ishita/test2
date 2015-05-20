
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/conslib.h,v 1.2 2013-11-25 10:48:39 mgostling Exp $
 * $Log: conslib.h,v $
 * Revision 1.2  2013-11-25 10:48:39  mgostling
 * Added missing CVS headers
 *
 */

#ifndef __CONSLIB_h
	#define __CONSLIB_h


	/* BIT Error Action */

#define BEA_CONTINUE	0
#define BEA_HALT		1


	/* Error Codes */

#define E__REPEAT				1

#define E__ABORT_ENTRY			0x1000  /* soft errors */
#define E__NO_MORE_PARAMS		0x1001

#define E__UNKNOWN_SWITCH		0x4000	/* hard error codes */
#define E__FORMAT_ERROR			0x4001
#define E__VALUE_ERROR			0x4002


/* Key Codes */

#define BELL			0x07	/* special keys */
#define BS				0x08
#define HTAB			0x09
#define LF				0x0A
#define	CR				0x0D
#define	ESC				0x1B

#define CUR_UP			0x0141	/* cursor movement */
#define CUR_DOWN		0x0142
#define CUR_RIGHT		0x0143
#define CUR_LEFT		0x0144
#define CUR_HOME		0x0200	/* not supported on VT100 */
#define CUR_END			0x0200	/*			-"-			  */
#define CUR_INS			0x0200	/*			-"-			  */

#define CUR_DEL			0x7F


	/* Command Types */

#define TYPE__QUIT				1
#define TYPE__SWITCH			2
#define TYPE__HELP				3
#define TYPE__SYMBOL			7				
#define TYPE__DISPLAY_BYTE		10
#define TYPE__DISPLAY_WORD		11
#define TYPE__DISPLAY_DWORD		12
#define TYPE__DISPLAY_ASCII		13
#define TYPE__SET_BYTE          20
#define TYPE__SET_WORD          21
#define TYPE__SET_DWORD         22
#define TYPE__INPUT_BYTE        30
#define TYPE__INPUT_WORD        31
#define TYPE__INPUT_DWORD       32
#define TYPE__OUTPUT_BYTE       40
#define TYPE__OUTPUT_WORD       41
#define TYPE__OUTPUT_DWORD      42
#define TYPE__PCI_SCAN          50
#define TYPE__PCI_QUERY         51
#define TYPE__PCI_BRIDGE_SCAN   60
#define TYPE__PCI_READ_BYTE     70
#define TYPE__PCI_READ_WORD     71
#define TYPE__PCI_READ_DWORD    72
#define TYPE__PCI_WRITE_BYTE    80
#define TYPE__PCI_WRITE_WORD    81
#define TYPE__PCI_WRITE_DWORD   82
#define TYPE__CMOS_READ         91
#define TYPE__CMOS_WRITE        92
#define TYPE__CPUID		        100
#define TYPE__TEST				110
#define TYPE__UNKNOWN			-1

	/* Parameter Types */

#define	ARG__ADDR		0x0001
#define ARG__DATA		0x0002
#define ARG__LENGTH		0x0004
#define ARG__LOOP		0x0008
#define	ARG__BUS		0x0010
#define ARG__DEV		0x0020
#define ARG__FUNC		0x0040
#define ARG__REGISTER	0x0080



typedef struct tagArgs
{
	UINT32	dAddr;
	UINT32	dData;
	UINT32	dLength;
	UINT32	dCount;
	UINT16	wRegister;
	UINT8	bBus;
	UINT8	bDev;
	UINT8	bFunc;
	UINT8	bRegister;

} ARGS;



/* util.c */

extern void vReset (void);
extern void vBIOS (void);

extern void vRadix2 (void);
extern void vRadix10 (void);
extern void vRadix16 (void);
extern int  iGetRadix (void);

extern void vConfirmOn (void);
extern void vConfirmOff (void);
extern int  iGetConfirmFlag (void);

extern void	vSetRepeatIndex (int i);
extern int	iGetRepeatIndex (void);

extern void vBeaHalt (void);
extern void vBeaContinue (void);
extern int  iGetBeaFlag (void);

extern void vCacheOn (void);
extern void vCacheOff (void);



/* support.c */

extern int	iCheckForEsc (void);
extern int	iGetExtdKeyPress (void);

extern int	iFilterWhitespace (char* achString);


/* serveice.c */
extern void vDisplayAscii (int iCmdType, int iCmdCount, char* achUserArgs[]);

extern void vDisplayMemory (int iCmdType, int iCmdCount, char* achUserArgs[]);
extern void vSetMemory (int iCmdType, int iCmdCount, char* achUserArgs[]);

extern void vPortInput  (int iCmdType, int iCmdCount, char* achUserArgs[]);
extern void vPortOutput (int iCmdType, int iCmdCount, char* achUserArgs[]);

extern void vPciScan  (int iCmdType, int iCmdCount, char* achUserArgs[]);
extern void vPciRead  (int iCmdType, int iCmdCount, char* achUserArgs[]);
extern void vPciWrite (int iCmdType, int iCmdCount, char* achUserArgs[]);
extern void vPciQuery (int iCmdType, int iCmdCount, char* achUserArgs[]);
extern void vPciBridgeScan   (int iCmdType, int iCmdCount,	char* achUserArgs[]);
extern void vCpuid (int iCmdType, int iCmdCount, char* achUserArgs[]);

extern void vTestMenu (int iCmdType, int iCmdCount, char* achUserArgs[]);

extern void vGpf (int iCmdType, int iCmdCount, char* achUserArgs[]);

extern void vNvRead( int iCmdType,	int iCmdCount, char* achUserArgs[]);
extern void vNvWrite( int iCmdType, int iCmdCount,	char* achUserArgs[]);

/*command.c*/
extern void vSetDefaults (void);
extern void vRedrawPrompt (void);
extern void vWriteErrorString(char* achErrorString);
extern void vShowCommandLine (char* achCommand, int iCommandLen, int iCursorPos);
extern int	iExtractArgs (ARGS* psArgs, UINT16 wReqBitmap, UINT16 wOptBitmap,
							int iCmdCount, char* achCmdArgs[]);
extern void	vProcessUserCmd (char* achUserCmd, int* piCmdCount,
							char* achCmdArgs[]);
//removed from mth_utils.c
extern void	vExecPostProc (char* achCommand, char* achArg);
extern int	iExecTokenize (char* achCommand, char* tokens[]);
extern void vShowExecutionTime(UINT64	qStartTicks, UINT64	qEndTicks);

#endif

