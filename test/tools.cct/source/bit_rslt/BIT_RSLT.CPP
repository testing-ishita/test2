/* includes */

#include <stdlib.h>
#include <stdio.h>

#include <conio.h>
#include <dos.h>
#include <string.h>

/* defines */

/* typedefs */

typedef unsigned char UINT8;
typedef unsigned int  UINT16;
typedef unsigned long UINT32;

typedef struct tagBebLog
{
	UINT16	wSequenceNumber;	/* unique identifier: 1-65534 */
	UINT32	dTimestamp;			/* seconds since 12:00am, 1st Jan 2005 */
	UINT16	wTestNumber;		/* failing Test number */
	UINT32	dErrorCode;			/* error from failing test */

} BEB_LOG;


typedef struct tagBitDirectory
{
	UINT32	dSignature1;		/* contains "_CCT", with '_' in the LSB */
	UINT32	dSignature2; 		/* contains "BIT_" with 'B' in the LSB */

	UINT8	bBitDirRev;			/* revision of the BIT Directory used */
	UINT8	bMajorRev;			/* firmware major version, bit-7 set = X */
	UINT8	bMinorRev;			/* firmware minor version */
	UINT8	bReserved;			/* reserved for factory use */

	UINT32	dTestListHead;		/* physical address of the test list */
	UINT32	dTestListSize;		/* size of the test list (bytes) */

	UINT32	dBitCodeHead;		/* physical address of the BIT code */
	UINT32	dBitCodeSize;		/* size of the BIT code */
	UINT32	dBitCodeEntry;		/* offset to cctExecuteBit() function */

	UINT32	dPbitResult;		/* error code from first failing PBIT test */
	UINT32	dPbitTestNumber;	/* number of failing PBIT test */

	BEB_LOG	asLogEntries[16];	/* historical failure log */

} BIT_DIRECTORY;

/* constants */

const int	aiMLen[] = {   0,  31,  59,  90, 120, 151,
						 181, 212, 243, 273, 304, 334 };

const int	aiMLenL[] = {   0,  31,  60,  91, 121, 152,
						 182, 213, 244, 274, 305, 335 };

const char	achSig[] = "$CCT_BIT";

/* locals */

/* globals */

/* externals */

/* forward declarations */

char* convertTimestamp (UINT32 dTs);




int main (void)
{
	BIT_DIRECTORY far* bd;

	UINT32	dErrorCode;
	UINT32	dTimestamp;
	UINT16	wTestNum;
	UINT16	wSequenceNum;
	UINT16	wBiosEdaSeg;
	UINT8 far* p;
	int		i, j, l;
	int		found;


	clrscr ();
	printf ("CCT BIT Results Decoder. V1.02\n\n");


	/* Get a pointer to the BIOS Extended Data Area */

	p = (UINT8 far*)MK_FP(0, 0x040E);			// get EDA segment
	wBiosEdaSeg = *(UINT16 far*)p;

	if (wBiosEdaSeg == 0)
	{
		fprintf (stderr, "BIOS Extended Data Segment not found.\n");
		return (1);
	}

	p = (UINT8 far*)MK_FP(wBiosEdaSeg, 0);		// convert to a pointer

	printf ("Found: BIOS EDA Segment at: %Fp\n", p);


	/* Scan the EDA segment for the $CCT_BIT signature */

	l = strlen (achSig);
	found = 0;

	for (i = 0; (i < 0x4000) && (found == 0); i++)	// max search size = 16kB
	{
		if (*(char far*)p == '$')
		{
			found = 1;

			for (j = 1; (j < l) && (found == 1); j++)
			{
				if (*(char far*)(p+j) != achSig[j])
					found = 0;
			}
		}

		p++;
	}

	if (found == 0)
	{
		fprintf (stderr, "$CCT_BIT signature was not found.\n");
		return (1);
	}

	p--;
	printf ("Found: $CCT_BIT signature at: %Fp\n\n", p);


	/* Report current (last) result */

	printf ("Current PBIT Result = 	");

	bd = (BIT_DIRECTORY far*)p;

	if (bd->dPbitResult == 0)
	{
		printf ("PASS (last test run = %i\n\n",
			(int)bd->dPbitTestNumber);
	}
	else
	{
		printf ("FAIL :: Test # %i, Error Code 0x%08lX\n\n",
			(int)bd->dPbitTestNumber, bd->dPbitResult);
	}

	/* Display the Error log */

	p += 40;

	for (i = 0; i < 16; i++)
	{
		wSequenceNum = *(UINT16 far*)(p + i*12);
		dTimestamp   = *(UINT32 far*)(p + i*12 + 2);
		wTestNum     = *(UINT16 far*)(p + i*12 + 6);
		dErrorCode   = *(UINT32 far*)(p + i*12 + 8);

		if ((wSequenceNum == 0) || (wSequenceNum == 0xFFFF))
			printf ("%2i: Empty Slot\n", i);
		else
		{
			printf ("%2i: #%04X %s, Test %i, Error %08lX\n",
				i,
				wSequenceNum,
				convertTimestamp (dTimestamp),
				wTestNum,
				dErrorCode);
        }
	}

	return (0);

} /* main () */


char*	convertTimestamp
(
	UINT32	dTs
)
{
	UINT32	s;
	UINT32	m;
	UINT32	h;
	UINT32	d;
	UINT32	mt;
	UINT32	y;

	UINT32	days;
	UINT32	ylen;

	int 	ly;
	int		i;

	static char	achDate[30];


	/* Get time of day */

	s = dTs % 60L;
	dTs -= s;

	m = dTs % (60L * 60L);
	dTs -= m;
	m /= 60L;

	h = dTs % (60L * 60L * 24L);
	dTs -= h;
	h /= (60L * 60L);

	/* Get year */

	days = dTs / (60L * 60L * 24L);

	y = 2005L;  	// epoch = 01/01/2005
	ylen = 365L;

	while (days >= ylen)
	{
		y++;
		days -= ylen;

		if ((y % 4) == 0)
			ylen = 366L;
		else
			ylen = 365L;
	}

	ly = 0;

	if ((y % 4) == 0)
		ly++;

	/* Get month and day-of-month */

	for (i = 0; i < 12; i++)
	{
		if (ly == 0)
		{
			if (aiMLen[i] <= days)
			{
				mt = i + 1;
				d = days - aiMLen[i] + 1;
			}
		}
		else
		{
			if (aiMLenL[i] <= days)
			{
				mt = i + 1;
				d = days - aiMLenL[i] + 1;
			}
		}
	}


	sprintf (achDate, "at %02li:%02li:%02li on %02li/%02li/%li",
				h, m, s, d, mt, y);

	return (achDate);

} /* convertTimestamp () */
