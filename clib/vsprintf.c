
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

/* VSPRINTF.C - 'virtual' sprintf ()
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/vsprintf.c,v 1.1 2013-09-04 07:17:58 chippisley Exp $
 *
 * $Log: vsprintf.c,v $
 * Revision 1.1  2013-09-04 07:17:58  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2008/05/15 15:32:40  swilson
 * Add build and version tracking. Implement & test missing c-library functions.
 *
 * Revision 1.3  2008/04/25 15:25:07  swilson
 * General organizational changes and additions.
 *
 * Revision 1.2  2008/04/24 16:05:50  swilson
 * Further updates. Added HAL and BIT library - only I/O functions present at this point.
 *
 * Revision 1.1  2008/04/23 16:28:59  swilson
 * Updates and new code centered on clib creation.
 *
 */


/* includes */

#include <stdtypes.h>
#include <stdarg.h>
#include <string.h>

/* defines */

#define	FMT_LEFT			0x00000001
#define	FMT_SIGN_PLUS		0x00000002
#define	FMT_SIGN_BLANK		0x00000004
#define	FMT_ALT				0x00000008
#define	FMT_ZERO			0x00000010
#define	FMT_SHORT_INT		0x00000020
#define	FMT_NEAR_PTR		0x00000040

/* typedefs */

/* constants */

const char achDigits[] = "0123456789ABCDEF";

/* locals */

/* globals */

/* externals */

/* forward declarations */

static int	iShowNumber (char* achBuffer, int* piPrintedChars, char* achInputBuff,
					int iCharCnt, int dFmtFlags, int iFmtWidth);

static int	itoa (char* s, int n);

static int	utoa (char* s, int radix, unsigned int n);


/****************************************************************************
 * vsprintf:
 *
 * NOTE: No floating point support!
 *
 * RETURNS:
 */

int vsprintf
(
	char*		achBuffer,
	const char*	achFormat,
	va_list		ptr
)
{
	UINT32	dFmtFlags;				/* active formatting flags for element */
	UINT32	dTemp;
	int		iPrintedChars;
	int		iFmtIndex;				/* format string index */
	int		iFmtWidth;				/* element width formatting counter */
	int		iCharCnt;
	char*	pStr;
	char	ch;
	char	achTempBuff[64 + 1];		/* supports 64-bit binary */


	iPrintedChars = 0;
	iFmtIndex = 0;
	ch = achFormat[iFmtIndex];

	while  (ch != '\0')
	{
		if (ch == '%')
		{
			iFmtWidth = 0;
			dFmtFlags = 0;

			ch = achFormat[++iFmtIndex];

			/* Flags */

			switch (ch)
			{
				case '%' :
					achBuffer[iPrintedChars++] = ch;
					break;

				case '-' :
					dFmtFlags |= FMT_LEFT;
					ch = achFormat[++iFmtIndex];
					break;

				case '+' :
				case ' ' :

					while ((ch == '+') || (ch == ' '))
					{
						if (ch == '+')
							dFmtFlags |= FMT_SIGN_PLUS;
						else
							dFmtFlags |= FMT_SIGN_BLANK;

						ch = achFormat[++iFmtIndex];
					}

					break;

				case '#' :
					dFmtFlags |= FMT_ALT;
					ch = achFormat[++iFmtIndex];
					break;
			}

			/* Width */

			switch (ch)
			{
				case '0':
					dFmtFlags |= FMT_ZERO;
					ch = achFormat[++iFmtIndex];
					break;

				case '*':
					iFmtWidth = va_arg (ptr, int);
					ch = achFormat[++iFmtIndex];
					break;
			}

			while ((ch >= '0') && (ch <= '9'))
			{
				iFmtWidth = (iFmtWidth * 10) + ch - '0';
				ch = achFormat[++iFmtIndex];
			}

			/* Size Modifiers */

			switch (ch)
			{
				case 'l':
				case 'L':
					ch = achFormat[++iFmtIndex];
					break;							/* ints are long by default */

				case 'h':
					dFmtFlags |= FMT_SHORT_INT;
					ch = achFormat[++iFmtIndex];
					break;

				case 'F':
					ch = achFormat[++iFmtIndex];
					break;							/* all data pointers are far by default */

				case 'N':
					dFmtFlags |= FMT_NEAR_PTR;
					ch = achFormat[++iFmtIndex];
					break;
			}

			/* Conversion Type */

			switch (ch)
			{
				case 'd' :		/* decimal signed integers */
				case 'i' :

					dTemp  = va_arg (ptr, int);

					if ((dFmtFlags & FMT_SHORT_INT) != 0)
						dTemp &= 0x0000FFFF;

					iCharCnt = itoa (achTempBuff, dTemp);

					(void)iShowNumber (achBuffer, &iPrintedChars, achTempBuff, iCharCnt, dFmtFlags, iFmtWidth);
					break;

				case 'o' :		/* unsigned octal integers */

					dTemp  = va_arg (ptr, int);

					if ((dFmtFlags & FMT_SHORT_INT) != 0)
						dTemp &= 0x0000FFFF;

					iCharCnt = utoa (achTempBuff, 8, dTemp);

					(void)iShowNumber (achBuffer, &iPrintedChars, achTempBuff, iCharCnt, dFmtFlags, iFmtWidth);
					break;

				case 'u' :		/* unsigned decimal integers */

					dTemp  = va_arg (ptr, int);

					if ((dFmtFlags & FMT_SHORT_INT) != 0)
						dTemp &= 0x0000FFFF;

					iCharCnt = utoa (achTempBuff, 10, dTemp);

					(void)iShowNumber (achBuffer, &iPrintedChars, achTempBuff, iCharCnt, dFmtFlags, iFmtWidth);
					break;

				case 'x' :		/* unsigned hexadecimal integers (lower case) */

					dTemp  = va_arg (ptr, int);

					if ((dFmtFlags & FMT_SHORT_INT) != 0)
						dTemp &= 0x0000FFFF;

					iCharCnt = utoa (achTempBuff, 16, dTemp);
					strlwr (achTempBuff);

					(void)iShowNumber (achBuffer, &iPrintedChars, achTempBuff, iCharCnt, dFmtFlags, iFmtWidth);
					break;

				case 'X' :		/* unsigned decimal integers (upper case)*/

					dTemp  = va_arg (ptr, int);

					if ((dFmtFlags & FMT_SHORT_INT) != 0)
						dTemp &= 0x0000FFFF;

					iCharCnt = utoa (achTempBuff, 16, dTemp);

					(void)iShowNumber (achBuffer, &iPrintedChars, achTempBuff, iCharCnt, dFmtFlags, iFmtWidth);
					break;

				case 'c' :		/* ASCII 8-bit character */

					dTemp  = va_arg (ptr, int);
					achBuffer[iPrintedChars++] = (char)dTemp;
					break;

				case 's' :		/* character string */

					pStr = va_arg (ptr, char*);	/* pointer */

					/* Do any align-right padding of string */

					if ((iFmtWidth > 0) && ((dFmtFlags & FMT_LEFT) == 0))
					{
						iCharCnt = 0;

						while (pStr[iCharCnt] != '\0')	/* get length of string */
							iCharCnt++;

						while (iCharCnt < iFmtWidth)
						{
							achBuffer[iPrintedChars++] = ' ';
							iCharCnt++;
						}
					}

					/* Display string until NULL character */

					iCharCnt = 0;

					while (pStr[iCharCnt] != '\0')
						achBuffer[iPrintedChars++] = pStr[iCharCnt++];

					/* Do any align-left padding to string */

					if ((iFmtWidth > 0) && ((dFmtFlags & FMT_LEFT) != 0))
					{
						while (iCharCnt < iFmtWidth)
						{
							achBuffer[iPrintedChars++] = ' ';
							iCharCnt++;
						}
					}		

					break;

				case 'n' :		/* count of chars written */

					break;

				case 'p' :		/* pointer */

					dTemp  = va_arg (ptr, int);		/* offset */

					iCharCnt = utoa (achTempBuff, 16, dTemp);
					(void)iShowNumber (achBuffer, &iPrintedChars, achTempBuff, iCharCnt, FMT_ZERO, 8);

					break;

				case 'f' :		/* floating point number */
				case 'e' :
				case 'g' :
				case 'E' :
				case 'G' :

					dTemp  = va_arg (ptr, int);		/* pull a 64-bit float from the stack */
					dTemp  = va_arg (ptr, int);

					achBuffer[iPrintedChars++] = '(';	/* we don't support floating point conversion (yet) */
					achBuffer[iPrintedChars++] = '*';
					achBuffer[iPrintedChars++] = ')';

					break;
			}
		}

		else
			achBuffer[iPrintedChars++] = ch;

		ch = achFormat[++iFmtIndex];
	}

	/* Terminate with a NULL */

	achBuffer[iPrintedChars++] = '\0';

	return (iPrintedChars);

} /* vsprintf () */


/****************************************************************************
 * iShowNumber: display a formatted number for ...printf()
 *
 * RETURNS: none
 */

static int iShowNumber
(
	char*	achBuffer,		/* output buffer */
	int*	piPrintedChars,	/* number of characters sent to output */
	char*	achInputBuff,	/* the string to format */
	int		iCharCnt,	 	/* number of characters in string (excluding minus) */
	int		dFmtFlags,		/* flags to use in formatting number */
	int		iFmtWidth		/* requested display width */
)
{
	int		iCnt = 0;
	char	chSign = '\0';
	char	chPad = ' ';


	/* Alternate format for '%X' and '%x' */

	if ((dFmtFlags & FMT_ALT) != 0)
	{
		achBuffer[(*piPrintedChars)++] = '0';
		achBuffer[(*piPrintedChars)++] = 'x';
	}

	/* Handle +/-ve number formating */

	if (achInputBuff[0] != '-')	/* +ve prefix is optional */
	{
		if ((dFmtFlags & FMT_SIGN_PLUS) != 0)
		{
			chSign = '+';
			iCharCnt++;
		}
 		else if ((dFmtFlags & FMT_SIGN_BLANK) != 0)
		{
			chSign = ' ';
			iCharCnt++;
		}
	}

	else					/* -ve always has '-' prefix in string */
		iCharCnt++;

	/* Handle minimum width (right justify) */

	if ((dFmtFlags & FMT_LEFT) == 0)
	{
		if ((dFmtFlags & FMT_ZERO) != 0)
			chPad = '0';

		while (iCharCnt < iFmtWidth)
		{
			achBuffer[(*piPrintedChars)++] = chPad;
			iCharCnt++;
		}
	}

	/* Now sign char if required */

	if (chSign != '\0')
		achBuffer[(*piPrintedChars)++] = chSign;

	/* Display String */

	while (achInputBuff[iCnt] != 0)
		achBuffer[(*piPrintedChars)++] = achInputBuff[iCnt++];


	if ((dFmtFlags & FMT_LEFT) != 0)
	{
		while (iCharCnt < iFmtWidth)
		{
			achBuffer[(*piPrintedChars)++] = ' ';
			iCharCnt++;
		}
	}

	return (0);

} /* iShowNum () */


/****************************************************************************
 * itoa: convert an integer to a string
 *
 * RETURNS: none
 */

static int itoa
(
	char*	s,
	int		n
)
{
	/* Handle -ve numbers */

	if (n < 0)
	{
		n = -n;
		*(s++) = '-';
	}

	return (utoa (s, 10, n));

} /* itoa () */


/****************************************************************************
 * utoa: convert an unsigned integer to a string
 *
 * RETURNS: none
 */

static int utoa
(
	char*	s,
	int		radix,
	 unsigned int n
)
{
	int	i = 0;
	int	j = 0;
	int k;
	char ch;


	/* Handle n = zero */

	if (n == 0)
		s[i++] = '0';

	/* Convert +ve int to char string (reverse order) */

	while (n > 0)
	{
		s[i++] = achDigits[n % radix];
		n /= radix;
	}

	/* Save length for return and NULL terminate */

	k = i;
	s[i--] = 0;

	/* Reverse the string for printing */

	while (i > j)
	{
		ch = s[j];
		s[j++] = s[i];
		s[i--] = ch;
	}

	return (k);

} /* utoa () */


