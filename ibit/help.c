
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

/*  $Header: /home/cvs/cvsroot/CCT_BIT_2/ibit/help.c,v 1.1 2013-09-04 07:27:43 chippisley Exp $
 *
 *  $Log: help.c,v $
 *  Revision 1.1  2013-09-04 07:27:43  chippisley
 *  Import files into new source repository.
 *
 *  Revision 1.3  2010/05/05 15:46:13  hmuneer
 *  Cute rev 0.1
 *
 *  Revision 1.2  2010/03/31 16:25:13  hmuneer
 *  ID command Changed to BITID
 *
 *  Revision 1.1  2009/05/21 12:43:18  swilson
 *  Help text and management routines
 *
 */

/* includes */

#include <stdio.h>
#include <bit/conslib.h>
#include "ibit.h"

/* defines */

#define MAX_PAGE		4

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */


/***************************************************************************
 * iHelpGetMaxPage: return number of pages in the help system
 *
 * Pages are numbered 1 .. MAX_PAGE
 *
 * RETURNS: maximum page number
 */

int iHelpGetMaxPage (void)
{
	return (MAX_PAGE);

} /* iHelpGetMaxPage () */


/***************************************************************************
 * vHelpDisplayHeader: display the help page header
 *
 * RETURNS: none
 */

void vHelpDisplayHeader 
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

} /* vHelpDisplayHeader () */


/***************************************************************************
 * vHelpDisplayPage: display a page of help text
 *
 * RETURNS: none
 */

void vHelpDisplayPage 
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
			puts ("T, BIT      Execute Test Case             T <test1> [<test2>] [<test3>]");
			puts ("                                          T <start_test> - <end_test>");
			puts ("                                          T {<test>}:<count>");
			puts ("                                          T {<test1> [<test2>]}:<count>");
			puts ("");
			puts ("TM          Test Menu                     TM [<test>]");
			puts ("SUM         Testing Summary               SUM [<test>]");
			puts ("Z           Zero Test Statistics          Z");
			puts ("BEA_HALT    BIT Error Action: Halt        BEA_HALT");
			puts ("BEA_CONT    BIT Error Action: Continue    BEA_CONT");
			break;

		case (2) :
			puts ("DB          Read Memory as Bytes          DB <addr> [<length>]");
			puts ("DW          Read Memory as Words          DW <addr> [<length>]");
			puts ("DD          Read Memory as Double-words   DD <addr> [<length>]");
			puts ("AB          Read Memory as Bytes + ASCII  AB <addr> [<length>]");
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
			puts ("In the commands below, 'pfa' should be relaced with: [b:<bus>] <dev> [f:<fun>]");
			puts ("");
			puts ("PS          PCI Scan                      PS");
			puts ("BS          PCI Bridge Scan               BS");
			puts ("PQ          PCI Query                     PQ pfa");
			puts ("");
			puts ("PRB         PCI Read Byte                 PRB pfa <reg> [<length>]");
			puts ("PRW         PCI Read Word                 PRW pfa <reg> [<length>]");
			puts ("PRD         PCI Read Double-word          PRD pfa <reg> [<length>]");
			puts ("PWB         PCI Write Byte                PWB pfa <reg> <data> [<length>]");
			puts ("PWW         PCI Write Word                PWW pfa <reg> <data> [<length>]");
			puts ("PWD         PCI Write Double Word         PWD pfa <reg> <data> [<length>]");
			puts ("");
			puts ("NVR         Non-volatile RAM Read         NVR <reg> [<length>]");
			puts ("NVW         Non-volatile RAM Write        NVR <reg> <data> [<length>]");
			break;

		case (4) :
			puts ("BITID       Display BIT Version Info      BITID");
			puts ("CPUID       Display Details from CPUID    CPUID");
			puts ("CA+         CPU Cache On                  CA+");
			puts ("CA-         CPU Cache Off                 CA-");
			puts ("RESET       Restart Board                 RESET");
			puts ("");
			puts ("R2          Set Radix = Binary            R2");
			puts ("R10         Set Radix = Decimal           R10");
			puts ("R16         Set Radix = Hexadecimal       R16");
			puts ("");
			puts ("C+          Write Confirm Mode = On       C+");
			puts ("C-          Write Confirm Mode = Off      C-");
			puts ("");
			puts (".d          Decimal Radix Subscript       <value>.d");
			puts (".h          Hexadecimal Radix Subscript   <value>.h");
			puts (".b          Binary Radix Subscript        <value>.b");
			break;

		default :
			puts ("- END -");
	}

} /* vHelpDisplayPage () */


/***************************************************************************
 * iHelpPromptNext: prompt for a key to determine next action within Help
 *
 * RETURNS: none
 */

int iHelpPromptNext (void)
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

} /* iHelpPromptNext () */

