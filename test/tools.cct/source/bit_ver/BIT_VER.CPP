/*******************************************************************************
*
* Filename:      bit_ver.cpp
*
* Description:  Version control utility.
*
* $Revision: 1.5 $
*
* $Date: 2014-12-09 14:10:44 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/tools.cct/source/bit_ver/BIT_VER.CPP,v $
*
* Copyright 2000-2014 Concurrent Technologies, Plc.
*
*******************************************************************************/

/* BIT_VER.EXE: version control utility
 *
 * This utility creates a 'C' code header file containing details of the
 * firmware version, target board and build date + time.
 *
 * The utility requires the file 'fw_id.h', from which it derives the base
 * firmware version and controls revision and build numbers during and after
 * the release procedure.
 *
 * The following rules are applied:
 *
 * 1) The base version and starting revision is controlled by the #define
 *    statement in fw_id.h
 * 2) Firmware uses 'X' to identify development revisions and 'V' for
 *    releases. New version format is: 2.01.01, development revisions: 2.02.01-rc01
 * 3) Development versions increment the BUILD number each time the utility
 *    is invoked.
 * 3) When releasing, the build number is always 1, all other numbers remain
 *    fixed at their previous development values.
 * 5) The first development build following a release increments the revision
 *    number by one and sets the build number to 1.
 *
 * Both version.h and fw_id.h are updated as appropriate.
 */


/* includes */

#ifdef __WATCOMC__
#include <stdio.h>
#else
// only needed for Microsoft Visual Studio.
#include "stdafx.h"
#endif
#include <string.h>
#include <time.h>

#define SW_VERSION 302

#define FWMODE_BIT		0
#define FWMODE_CUTE		1

typedef struct
{
	int		iVer;
	int		iRev1;
	int		iSpin;
	int		iRc;
	int		iBuildId;
	int		iRelease;
	char	chOldXorV;
} VERSION;

// set default versions for both build types
VERSION sCute = {2,1,1,1,1,0,'X'};
VERSION sBit  = {2,1,1,1,1,0,'X'};

/* forward declarations */
int iGetLine (FILE * psInput, char * achLine);
void vExtractVersion (char * aStr, VERSION * pVer);
int iCreateFwIdFile (char * pCuteStr, char * pBitStr);

int main
(
	int		argc,
	char *	argv[]
)
{
	FILE *		psFile;
	time_t		sDate;
	int			iStatus;
	int			iBuildType;
	int			iOldBuildId;
	int			iLine;
	int			iFwId;
	char		achBoard[11];
	char		achDate[80];
	char		achLine[128];
	char		ach1[128];
	char		ach2[128];
	char		ach3[128];
	char		ach4[128];
	char *		p;
	VERSION *	pVersion;		// version definition for this build
	char *		pVersionStr;
	VERSION *	pOther;			// the other version definition not being used for this build
	char *		pOtherStr;

	/* Sign on */

	printf ("\nVersion Builder %d.%02d (c) CCT 2000-2014\n", SW_VERSION / 100, SW_VERSION % 100);

	iFwId = 0;

	/* Check user arguments : board type must be supplied, 
	 *                      : BUILDTYPE must be supplied
	 *                      : RELEASE is optional
	 */

	if ((argc < 3) || (argc > 4))
	{
		fprintf (stderr, "Usage: bit_ver <boardname> <BUILDTYPE> [RELEASE]\n");
		fprintf (stderr, "<BUILDTYPE> = 'MAKE_BIT' or 'MAKE_CUTE'\n");
		return (1);
	}

	// verify BUILDTYPE

	if (strcmp (argv[2], "MAKE_CUTE") == 0)
	{
		iBuildType = FWMODE_CUTE;
		pVersion = &sCute;
		pVersionStr = ach3;
		pOther = &sBit;
		pOtherStr = ach4;
	}
		else if (strcmp (argv[2], "MAKE_BIT") == 0)
	{
		iBuildType = FWMODE_BIT;
		pVersion = &sBit;
		pVersionStr = ach4;
		pOther = &sCute;
		pOtherStr = ach3;
	}
	else
	{
			fprintf (stderr, "Error: unknown switch [%s]\n\n", argv[2]);
			fprintf (stderr, "Usage: bit_ver <boardname> <BUILDTYPE> [RELEASE]\n");
			fprintf (stderr, "<BUILDTYPE> = 'MAKE_BIT' or 'MAKE_CUTE'\n");
			return (1);
	}

	// check for RELEASE
	
	if (argc == 4)
	{
		if (strcmp (argv[3], "RELEASE") == 0)
		{
			pVersion->iRelease = 1;
		}
		else
		{
			fprintf (stderr, "Error: unknown switch [%s]\n\n", argv[2]);
			fprintf (stderr, "Usage: bit_ver <boardname> <BUILDTYPE> [RELEASE]\n");
			return (1);
		}
	}
	else
	{
		pVersion->iRelease = 0;
	}
	
	/* Convert board name argument to traditional format:-
	 *
	 *	      AABBB_CCC changes to AA BBB/CCC
	 */

	achBoard[0] = argv[1][0];
	achBoard[1] = argv[1][1];
	achBoard[2] = ' ';
	achBoard[3] = argv[1][2];
	achBoard[4] = argv[1][3];
	achBoard[5] = argv[1][4];
	achBoard[6] = '/';
	achBoard[7] = argv[1][6];
	achBoard[8] = argv[1][7];
	achBoard[9] = argv[1][8];
	achBoard[10] = '\0';

	/* Get the host's build date / time */

	time (&sDate);
	strcpy (ach1, ctime(&sDate));
	ach1[strlen (ach1) - 1] = 0;		/* strip terminating /n */

	/* Massage date/time into a more user-friendly form */

	p = strchr (ach1, ':') - 2;			/* find start of time */

	strcpy (ach2, p);					/* copy time and year */
	*p = '\0';							/* truncate after date */

	p = strchr (ach2, ' ');				/* find start of year */
	strcpy (ach3, p+1);					/* copy year */
	*p = '\0';							/* truncate after time */

	sprintf (achDate, "%s%s, %s", ach1, ach3, ach2);


	/* Open 'fw_id.h' - this file must exist */

	psFile = fopen ("fw_id.h", "r+");

	if (psFile == NULL)
	{
		fprintf (stderr, "Error opening fw_id.h\n");
		return (1);
	}

	/* Read FW_ID, CUTE_ID and BIT_ID definitions */
	for (iLine = 0; iLine < 2; ++iLine)
	{
		iStatus = iGetLine (psFile, achLine);
	
		if (iStatus != 0)
		{
			fprintf (stderr, "Error reading fw_id.h\n");
			fclose (psFile);
			return (1);
		}
	
		/* Parse definition from fw_id.h*/
		sscanf (achLine, "%s%s%s", ach1, ach2, ach3);

		
		// check for single FW_ID definition
		if(strcmp (ach2, "FW_ID") == 0) 
		{
			// #define      FW_ID   "X1.01"
			// #define      FW_ID   "V1.10"
			// #define      FW_ID   "V1.07-01"
			// #define      FW_ID   "2.01.01-rc01"
			vExtractVersion(ach3, pVersion);

			//create new fw_id.h with CUTE_ID and BIT_ID statements
			iFwId = 1;
			break;
		}
		else if ((strcmp (ach1, "#define") == 0) &&
				 (strcmp (ach2, "CUTE_ID") == 0) &&
				 (ach3[0] == '"'))
		{
			// #define      CUTE_ID "2.01.01"
			// #define      CUTE_ID "2.01.01-rc01"
			vExtractVersion(ach3, &sCute);
		}
		else if ((strcmp (ach1, "#define") == 0) &&
				 (strcmp (ach2, "BIT_ID") == 0) &&
				 (ach3[0] == '"'))
		{
			// #define      BIT_ID  "2.01.01"
			// #define      BIT_ID  "2.01.01-rc01"
			vExtractVersion(ach3, &sBit);
		}
		else
		{
			fprintf (stderr, "[fw_id.h] invalid format.\n");
			fprintf (stderr, "Single statement #define FW_ID \".....\" has been replaced with\n");
			fprintf (stderr, "two statements #define CUTE_ID \".....\" and #define BIT_ID\".....\"\n");
			fprintf (stderr, "CUTE_ID must be the first of the two statements.\n");
			fprintf (stderr, "Original formats:\n");
			fprintf (stderr, "     #define  FW_ID      \"X1.02\"\n");
			fprintf (stderr, "     #define  FW_ID      \"2.02.01-rc01\"\n");
			fprintf (stderr, "or\n");
			fprintf (stderr, "     #define  FW_ID      \"V1.07-01\"\n");
			fprintf (stderr, "     #define  FW_ID      \"2.02.01\"\n");
			fprintf (stderr, "replaced with:\n");
			fprintf (stderr, "     #define  CUTE_ID    \"2.02.01-rc01\"\n");
			fprintf (stderr, "     #define  BIT_ID     \"2.02.01-rc01\"\n");
			fprintf (stderr, "or\n");
			fprintf (stderr, "     #define  CUTE_ID    \"2.02.01\"\n");
			fprintf (stderr, "     #define  BIT_ID     \"2.02.01\"\n");
			return (1);
		}
	}

	/* Close 'fw_id.h' for now, we'll recreate in a moment if necessary */
	fclose (psFile);

	/* Check if we're switching from release to development or vice versa */

	if (((pVersion->iRelease == 0) && (pVersion->chOldXorV == 'V')) ||
		((pVersion->iRelease == 1) && (pVersion->chOldXorV == 'X')))
	{
		/* Check for a new development build after release and increment revision
		 *
		 * i.e. look for xxxx_ID string with a 'V' but no RELEASE flag
		 */
		if ((pVersion->iRelease == 0) && (pVersion->chOldXorV == 'V'))
		{
			pVersion->iRev1++;
		}

		/* need to update fw_id.h */
		iFwId = 1;

		/* Start build at 1 again */
		pVersion->iBuildId = 1;
	}
	else
	{
		/* Open the build ID - file need not exist */

		psFile = fopen ("version.h", "r+t");	    /* does file exist? */

		if (psFile == NULL)
		{
			psFile = fopen ("version.h", "wt");     /* create new */
	
			if (psFile == NULL)
			{
				fprintf (stderr, "Error creating file version.h\n");
				return (1);
			}
		}

		/* Read the build ID #define, if present. Increment the ID only for
		 * development builds, set to 1 otherwise.
		 */

		pVersion->iBuildId = 1;

		iStatus = iGetLine (psFile, achLine);

		if (iStatus == 0)
		{
			sscanf (achLine, "%s%s%d", ach1, ach2, &iOldBuildId);
	
			if ((strcmp (ach1, "#define") == 0) &&
				(strcmp (ach2, "FW_BUILD_NUMBER") == 0))
			{
				if ((pVersion->iRelease == 0) && (pVersion->chOldXorV == 'X'))
				{
					pVersion->iBuildId = iOldBuildId + 1;
				}
			}
		}

		/* Close the old file, delete it and create a new one. This done
		 * as 'release' file is shorter than 'development' file.
		 */

		fclose (psFile);
	}

	/* Check if we're switching from the old CUTE code base */
	if (pVersion->iVer < 2)
	{
		// reset version details for first release candidate using new format
		pVersion->iVer = 2;
		pVersion->iRev1 = 1;
		pVersion->iSpin = 1;
		pVersion->iRc = 1;
		pVersion->iBuildId = 1;
		pVersion->chOldXorV = 'X';
	}

	/* create new FW_REVISION_STRING for this build type */

	if (pVersion->iRelease == 0)
	{
		sprintf (pVersionStr, "\"%d.%02d.%02d-rc%02d\"", pVersion->iVer, pVersion->iRev1, pVersion->iSpin, pVersion->iRc);
	}
	else
	{
		sprintf (pVersionStr, "\"%d.%02d.%02d\"", pVersion->iVer, pVersion->iRev1, pVersion->iSpin);
	}

	/* maintain the FW_REVISION_STRING for unused BIT_ID or CUTE_ID definition */

	if (pOther->chOldXorV == 'X')
	{
		sprintf (pOtherStr, "\"%d.%02d.%02d-rc%02d\"", pOther->iVer, pOther->iRev1, pOther->iSpin, pOther->iRc);
	}
	else
	{
		sprintf (pOtherStr, "\"%d.%02d.%02d\"", pOther->iVer, pOther->iRev1, pOther->iSpin);
	}

	/* create or update fw_id.h */

	if (iFwId == 1)
	{
		if (iCreateFwIdFile (ach3, ach4))
		{
			return 1;	       // file error
		}
	}

	/* Open an empty file for writing */
	/* If the file exists, its contents are destroyed */
	psFile = fopen ("version.h", "wt");     /* create new */

	/* Write out the new version.h file */

	fprintf (psFile, "#define\tFW_BUILD_NUMBER\t\t%d\n",    pVersion->iBuildId);

	if (pVersion->iRelease == 0)
	{
		fprintf (psFile, "#define\tFW_X_OR_V\t\t\t'X'\n");
	}
	else
	{
		fprintf (psFile, "#define\tFW_X_OR_V\t\t\t'V'\n");
	}
	
	fprintf (psFile, "#define\tFW_VERSION_NUMBER\t%d\n",  pVersion->iVer);

	fprintf (psFile, "#define\tFW_REVISION_NUMBER\t%d\n", pVersion->iRev1);

	fprintf (psFile, "#define\tFW_SPIN_NUMBER\t\t%d\n", pVersion->iSpin);

	if (iBuildType == FWMODE_CUTE)
	{
		fprintf (psFile, "#define\tFW_REVISION_STRING\t%s\n", ach3);
	}
	else
	{
		fprintf (psFile, "#define\tFW_REVISION_STRING\t%s\n", ach4);
	}

	if (pVersion->iRelease == 0)
	{
		fprintf (psFile, "#define\tFW_MESSAGE_STRING\t\"NOT FOR RELEASE\"\n");
	}
	else
	{
		fprintf (psFile, "#define\tFW_MESSAGE_STRING\t\" \"\n");
	}

	fprintf (psFile, "#define\tBOARD_TYPE_STRING\t\"BIT Firmware for %s.\"\n", achBoard);
	
	fprintf (psFile, "#define\tBOARD_CUTE_STRING\t\"CUTE Firmware for %s.\"\n", achBoard);

	fprintf (psFile, "#define\tTIME_DATE_STRING\t\"Built on: %s\"\n", achDate);

	fprintf (psFile, "#define\tFW_BUILD_STRING\t\t\"Build: %d\"\n", pVersion->iBuildId);

	/* exit with success */

	fflush (psFile);

	fclose (psFile);
	return (0);
} /* main () */


int iGetLine
(
	FILE*   psInput,
	char*   achLine
)
{
	int	     iIndex;
	int	     iComplete;
	char    ch;

	iIndex = 0;
	iComplete = -1;

	do {
		if (fread (&ch, 1, 1, psInput) == 1)
		{
			if (ch == '\n')
			{
				iComplete = 0;
			}

			achLine [iIndex++] = ch;
			achLine [iIndex]   = '\0';
		}

		else
			iComplete = 2;

	} while (iComplete == -1);


	if (feof (psInput) != 0)
	{
		iComplete = 1;
	}

	return (iComplete);
} /* iGetLine () */

/*
 * Extract version details from ID string
 */
void vExtractVersion 
(
	char * aStr,
	VERSION * pVer
)
{
	// need to handle multiple string formats

	// #define      FW_ID   "X1.01"
	// #define      FW_ID   "V1.10"
	// #define      FW_ID   "V1.07-01"
	// #define      FW_ID   "2.01.01-rc01"

	// #define      CUTE_ID "2.01.01"
	// #define      CUTE_ID "2.01.01-rc01"

	// #define      BIT_ID  "2.01.01"
	// #define      BIT_ID  "2.01.01-rc01"

	int  len;

	len = strlen (aStr);

	pVer->iSpin = 1;
	pVer->iRc = 1;

	if (len == 7)
	{
		// #define      FW_ID   "X1.01"
		// #define      FW_ID   "V1.10"

		pVer->iVer  = (aStr[2] - '0');
		pVer->iRev1 = ((aStr[4] - '0') * 10) + (aStr[5] - '0');
		pVer->chOldXorV = aStr[1];
	}
	else if (len == 10)
	{
		// #define      FW_ID   "V1.07-01"

		pVer->iVer  = (aStr[2] - '0');
		pVer->iRev1 = ((aStr[4] - '0') * 10) + (aStr[5] - '0');
		pVer->iSpin = ((aStr[7] - '0') * 10) + (aStr[8] - '0');
		pVer->chOldXorV = aStr[1];
	}
	else
	{
		// #define      FW_ID   "2.01.01-rc01"
		// #define      CUTE_ID "2.01.01"
		// #define      CUTE_ID "2.01.01-rc01"
		// #define      BIT_ID  "2.01.01"
		// #define      BIT_ID  "2.01.01-rc01"

		pVer->iVer  = (aStr[1] - '0');
		pVer->iRev1 = ((aStr[3] - '0') * 10) + (aStr[4] - '0');
		pVer->iSpin = ((aStr[6] - '0') * 10) + (aStr[7] - '0');

		if ( len > 9 )
		{
			// read release candidate number
			pVer->chOldXorV = 'X';
			pVer->iRc = ((aStr[11] - '0') * 10) + (aStr[12] - '0');
		}
		else
		{
			pVer->chOldXorV = 'V';
		}
	}
}

/*
 * Create or update fw_id.h
 */
int iCreateFwIdFile
(
	char * pCuteStr,
	char * pBitStr
)
{
	FILE * psFile;

	/* Open an empty file for writing */
	/* If the file exists, its contents are destroyed */
	psFile = fopen ("fw_id.h", "wt");

	if (psFile == NULL)
	{
		fprintf (stderr, "Error creating file fw_id.h\n");
		return (1);
	}

	/* Write back the FW_ID file with the the update version */

	if (sCute.iRelease == 0)
	{
		fprintf (psFile, "#define\tCUTE_ID\t%s\n", pCuteStr);
	}
	else
	{
		fprintf (psFile, "#define\tCUTE_ID\t%s\n", pCuteStr);
	}
	if (sBit.iRelease == 0)
	{
		fprintf (psFile, "#define\tBIT_ID\t%s\n", pBitStr);
	}
	else
	{
		fprintf (psFile, "#define\tBIT_ID\t%s\n", pBitStr);
	}

	fprintf (psFile, "\n");
	fprintf (psFile, "/**************************************************************/\n");
	fprintf (psFile, "/* This file is maintained by BIT_VER.EXE %d.%02d                */\n", SW_VERSION / 100, SW_VERSION % 100);
	fprintf (psFile, "/*                                                            */\n");
	fprintf (psFile, "/* Only the rc version numbers need to be updated manually.   */\n");
	fprintf (psFile, "/*                                                            */\n");
	fprintf (psFile, "/* #define    FW_ID     \"...........\" has been replaced by    */\n");
	fprintf (psFile, "/* #define    CUTE_ID   \"...........\" and                     */\n");
	fprintf (psFile, "/* #define    BIT_ID    \"...........\"                         */\n");
	fprintf (psFile, "/*                                                            */\n");
	fprintf (psFile, "/* #define    CUTE_ID   \"...........\" should be defined first */\n");
	fprintf (psFile, "/**************************************************************/\n");

	fclose (psFile);
	return 0;
}
