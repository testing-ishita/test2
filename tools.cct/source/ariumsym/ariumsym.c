/*******************************************************************************
*
* Filename:	 ariumsym.c
*
* Description:	Arium emulator symbol file generation.
*
* $Revision: 1.1 $
*
* $Date: 2014-06-13 10:19:32 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/tools.cct/source/ariumsym/ariumsym.c,v $
*
* Copyright 2014 Concurrent Technologies, Plc.
*
*******************************************************************************/
/*
	This utility reads the SYMBOL.TXT file created by GNU nm utility and creates
	a symbol file in the TEXTSYM format for American Arium Emulator.

	Input parameters
*/


//              *************************
//              | include files         |
//              *************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <io.h>

//              *************************
//              | macros                |
//              *************************

#define VERSION     100
#define FALSE		0
#define TRUE		1
#define BUFSIZE		128


//              *************************
//              | module data           |
//              *************************

unsigned long dgroupSegment;
unsigned long runtimeSegment;
char buf[BUFSIZE];
char *mapfile;
char *symfile;
FILE *inFP;
FILE *outFP;

void parseCommandLine (int argc, char *argv[]);
void helpMsg(void);
void createSymbolFile(void);
void generateSymbol(char *segment, char *offset, char *symbol);
unsigned int htoi(char *str);
unsigned long htol(char *str);
unsigned char getHex(char asc);

//              *************************
//              | module code           |
//              *************************

/*
****************************************************************************
Input:

00001000 A _size_of_heap_commit__
00001000 A _size_of_stack_commit__
00100000 t .text
00100000 A _size_of_heap_reserve__
00100000 A _text_start
00100000 T s2_signature
0010000c T prot_entry
00100061 t skip_mem_init
00160f24 r .rdata
00160f24 R achDigits
001610dc r .rdata
0017a94c r .eh_frame
0017a9e0 d .data
0017a9e0 D dStartupFlags
0017a9e4 D dApEntry
0017a9e8 D dBiosInfo_addr
0017a9ec D bCpuCount
00180080 D _bss_start
00181000 i .drectve
00181020 i .drectve
001813a0 b .bss
001813a0 B startLine
001813a4 b dActiveTestHandler
001813a8 b buff.3659

****************************************************************************
Output:

TEXTSYM V1.0

GLOBAL  | 000FF7AC |  CODE  |  A20_BYTE
GLOBAL  | 000FF0A0 |  CODE  |  a20_off
GLOBAL  | 000FF5C1 |  CODE  |  A20_OFF_FAR
...
*/

/****************************************************************************
*                               main                                        *
*                                                                           *
****************************************************************************/

void main (int argc, char *argv[])
{

	int errorCode;

//	printf("\nEmulator Symbol File Generator V%d.%02d\n", VERSION/100, VERSION%100);
	errorCode = 0;

	parseCommandLine (argc, argv);

	outFP = fopen(symfile, "w");	// try to create symbol file
	if (outFP == NULL)
	{
		printf("Unable to create emulator symbol file: %s\n", symfile);
		errorCode = 2;
	}
	
	inFP = fopen(mapfile, "r");		// try to open input file
	if (inFP == NULL)
	{
		printf("Unable to open input file: %s\n", mapfile);
		errorCode = 2;
	}
	else
	{
		// generate symbols
		if (inFP != NULL)
		{
			if (outFP != NULL)
			{
				createSymbolFile();
			}
		}
	}

	if (outFP != NULL)
	{
		fclose(outFP);
	}

	printf("\n");
	exit(errorCode);
}

void parseCommandLine (int argc, char *argv[])
{
	char p_ident;					// options preceeded by '-' or '/'
	char p_code;					// option identifier
	int  i;
	int inputFileNameSeen;
	int outputFileNameSeen;

	// validate command line 

	if (argc != 5)					// wrong number of command line arguments provided
		helpMsg();

	inputFileNameSeen = FALSE;
	outputFileNameSeen = FALSE;
	
	for (i = 1; i < argc; i++)
	{
		p_ident = *(*(argv + i));				// '-' or '/'
		p_code  = toupper(*(*(argv + i) + 1));	// first char of option
	
		if ((p_ident == '-') || (p_ident == '/'))
		{
			switch (p_code)
			{
			case 'I':							// input filename
				mapfile = argv[i+1];
//				printf("Input: %s\n", mapfile);
				inputFileNameSeen = TRUE;
				++i;
				break;
	
			case 'O':							// output filename
				symfile = argv[i+1];
//				printf("Output: %s\n", symfile);
				outputFileNameSeen = TRUE;
				++i;
				break;
	
			default :
				printf("\nInvalid option.\n");
				helpMsg(); // invalid option
				break;
	
			}
	
		}
		else
		{
			printf("\nInvalid option prefix.\n");
			helpMsg(); // invalid option prefix
		}
	}
	
	if (!(inputFileNameSeen && outputFileNameSeen))
	{
		printf("\nInvalid command line.\n");
		helpMsg();
	}
}

void helpMsg(void)
{
	printf("\nUsage: ARIUMSYM -I <inputfile> -O <outputfile>\n\n");
	printf("Where -I  specifies the input file created by the GNU 'nm' utility\n");
	printf("      -O  specifies the output file for use with the emulator\n\n");
	printf("An option may be in upper or lower case and can be prefixed by '-' or '/'.\n");
	printf("There must be a space between an option and its argument.\n");
	printf("Both options must be specified and can be specified in any order.\n");
	exit(2);
}

void createSymbolFile(void)
{
	int lineNum;
	char *token1;
	char *token2;
	char *token3;

	fprintf(outFP, "TEXTSYM V1.1\n\n");
	lineNum = 0;

	while (fgets(buf, sizeof(buf), inFP) != NULL)
	{
		++lineNum;
		token1 = strtok(buf, " \n");
		token2 = strtok(NULL, " \n");
		token3 = strtok(NULL, " \n");

		if (*token3 != '.')		// ignore lines when first character of token is '.'
		{
			switch (*token2)	// symbol type
			{
				case 'T':		// code values
				case 't':
					fprintf(outFP, "GLOBAL  | 00000000%08lX |  CODE  |  %s\n", htol(token1), token3);
					break;

				case 'B':		// uninitialised data values
				case 'b':
				case 'C':		// common uninitialised data values
				case 'D':		// initialised data values
				case 'd':
				case 'R':		// readonly data values
				case 'r':
					// use LOCAL type for data symbols so that duplicate names do not cause a problem
					fprintf(outFP, "LOCAL   | 00000000%08lX |  DATA  |  %s\n", htol(token1), token3);
					break;

				default:		// ignore anything else
					break;
			}
		}
	}

	fclose(inFP);
}					   

unsigned long htol(char *str)
{
	// convert hex ascii string to hexadecimal 
	unsigned long val;

	for (val = 0; *str != 0; ++str)
	{
		val <<= 4;
		val += getHex(*str);
	}
	
	return val;
}

unsigned char getHex(char asc)
{
	// convert hex ascii char to binary
	unsigned char val;

	val = (unsigned char) (toupper(asc) - 0x30);
	if (val > 9)
	{		
		val -= 7;
	}
	return val;;
}
