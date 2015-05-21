
/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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
 * HELP: help screen text and management functions
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/mth/help.c,v 1.2 2013-09-26 12:42:10 chippisley Exp $
 * $Log: help.c,v $
 * Revision 1.2  2013-09-26 12:42:10  chippisley
 * Added support for network Soak Master Client.
 *
 * Revision 1.1  2013/09/04 07:44:08  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2012/11/02 14:17:36  chippisley
 * Added "TL" (task list) command help.
 *
 * Revision 1.3  2011/02/28 11:59:00  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.2  2010/06/24 15:47:44  hchalla
 *  Code Cleanup, Added headers and comments.
 *
 */



/* includes */

#include <stdio.h>
#include <bit/conslib.h>
#include "mth.h"
#include "config.h"

/* defines */

#define MAX_PAGE		4

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */

/*****************************************************************************\
 *
 *  TITLE:  iMTHHelpGetMaxPage ()
 *
 *  ABSTRACT:  return number of pages in the help system
 *
 * 	RETURNS: maximum page number
 *
\*****************************************************************************/

int iMTHHelpGetMaxPage (void)
{
	return (MAX_PAGE);

} /* iMTHHelpGetMaxPage () */


/*****************************************************************************\
 *
 *  TITLE:  vMTHHelpDisplayHeader ()
 *
 *  ABSTRACT:  display the help page header
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/

void vMTHHelpDisplayHeader
(
	int		iPageNum
)
{
	char	achBuffer[80];


	sprintf (achBuffer, "Command     Description                   Syntax Summary               Page %d/4",
				iPageNum);
	puts (achBuffer);
	puts ("----------- ----------------------------- -------------------------------------");
	puts ("");

} /* vMTHHelpDisplayHeader () */

/*****************************************************************************\
 *
 *  TITLE:  vMTHHelpDisplayPage ()
 *
 *  ABSTRACT:  display a page of help text
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/
void vMTHHelpDisplayPage
(
	int		iPageNum
)
{
	switch (iPageNum)
	{
		case (1) :
			puts ("Key:  <>  a required numeric value  []  an optional parameter");
			puts ("");
			puts ("HELP, H, ?  Help (this text)              HELP");
			puts ("");
			puts ("T,          Execute Test Case             T <test1> [<test2>] [<test3>]");
			puts ("                                          T <start_test> - <end_test>");
			puts ("                                          T {<test>}:<count>");
			puts ("                                          T {<test1> [<test2>]}:<count>");
			puts ("");
			puts ("TM          Test Menu                     TM [<test>]");
			puts ("SUM         Testing Summary               SUM [<test>]");
			puts ("Z           Zero Test Statistics          Z");
			puts ("TEA_HALT    Test Error Action: Halt       TEA_HALT");
			puts ("TEA_CONT    Test Error Action: Continue   TEA_CONT");
			break;

		case (2) :
			puts ("DB          Read Memory as Bytes          DB <addr> [<length>]");
			puts ("DW          Read Memory as Words          DW <addr> [<length>]");
			puts ("DD          Read Memory as Double-words   DD <addr> [<length>]");
			puts ("");
			puts ("SB          Set Memory Byte               SB <addr> <data> [<length>]");
			puts ("SW          Set Memory Word               SW <addr> <data> [<length>]");
			puts ("SD          Set Memory Double-word        SD <addr> <data> [<length>]");
			puts ("");
			puts ("IB          Read Byte                     IB <port> [<length>]");
			puts ("IW          Read Word                     IW <port> [<length>]");
			puts ("ID          Read Double-word              ID <port> [<length>]");
			puts ("");
			puts ("OB          Write Byte                    OB <port> <data> [<length>]");
			puts ("OW          Write Word                    OW <port> <data> [<length>]");
			puts ("OD          Write Double-word             OD <port> <data> [<length>]");
			break;

		case (3) :
			puts ("In the commands below, 'pfa' should be replaced with: [b:<bus>] <dev> [f:<fun>]");
			puts ("");
			puts ("PS          PCI Scan                      PS");
			puts ("");
			puts ("PRB         PCI Read Byte                 PRB pfa <reg> [<length>]");
			puts ("PRW         PCI Read Word                 PRW pfa <reg> [<length>]");
			puts ("PRD         PCI Read Double-word          PRD pfa <reg> [<length>]");
			puts ("PWB         PCI Write Byte                PWB pfa <reg> <data> [<length>]");
			puts ("PWW         PCI Write Word                PWW pfa <reg> <data> [<length>]");
			puts ("PWD         PCI Write Double Word         PWD pfa <reg> <data> [<length>]");
			break;

		case (4) :
			puts ("CUTEID      Display CUTE Version Info     CUTEID");
			puts ("RESET       Restart Board                 RESET");
			puts ("B           Boot to BIOS                  B");
#ifdef INCLUDE_DBGLOG
			puts ("DBLP        Display Debug Log             DBLP");
			puts ("DBLC        Clear Debug Log               DBLC");
			puts ("TCB         Display TCB                   TCB <addr>");
			puts ("RUN         Execute Function at address   RUN <addr>");
			puts ("LKADDR      Lookup <addr> and show symbol LKADDR <addr>");
#endif
#ifdef INCLUDE_NETWORK				
			puts ("NETSTATS    Display Network Statistics    NETSTATS");
#ifdef INCLUDE_LWIP_SMC
			puts ("SMCSTATS    Display SM Client Statistics  SMCSTATS");
#endif
#endif			
			puts ("TL          Display Task List             TL");
			puts ("R2          Set Radix = Binary            R2");
			puts ("R10         Set Radix = Decimal           R10");
			puts ("R16         Set Radix = Hexadecimal       R16");
			puts ("");
			puts (".d          Decimal Radix Subscript       <value>.d");
			puts (".h          Hexadecimal Radix Subscript   <value>.h");
			puts (".b          Binary Radix Subscript        <value>.b");
			break;

		default :
			puts ("- END -");
	}

} /* vMTHHelpDisplayPage () */



/*****************************************************************************\
 *
 *  TITLE:  iMTHHelpPromptNext ()
 *
 *  ABSTRACT:  Prompt for a key to determine next action within Help
 *
 * 	RETURNS: 0
 *
\*****************************************************************************/
int iMTHHelpPromptNext (void)
{
	int		iKey;


	puts ("");
	puts ("-- Press <Esc> to exit help; any other key for the next page. --");

	do {
		iKey = iGetExtdKeyPress ();

		if (iKey == ESC)
			return (1);

	} while (iKey == 0);

	return (0);

} /* iMTHHelpPromptNext () */

