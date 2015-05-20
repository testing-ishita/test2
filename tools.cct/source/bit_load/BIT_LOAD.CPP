/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>

#include "bigreal.h"


/* defines */

#define V_MAJOR			(1)
#define V_MINOR			(2)

#define SIG_1ST_STAGE	(0x31544942L)	/* "BIT1", 'B' in LSB */
#define CUTE_1ST_STAGE	(0x45545543L)	/* "CUTE", 'C' in LSB */
#define SIG_2ND_STAGE	(0x32544942L)	/* "BIT2"	-"-		  */

#define TEMP_ADDR		(0x01000000L)	/* 16MB */
#define MAX_LOAD_ADDR	(0x000E0000L)


/* typedefs */

typedef struct tagImageData
{
	UINT32	dImageSize;

	UINT32	dSize1;
	UINT32	dCsum1;

	UINT32	dOffset2;
	UINT32	dSize2;
	UINT32	dCsum2;

} IMAGE_DATA;


/* externals */

extern "C" void vCallFirstStage (UINT32 dLoadAddr, UINT32 dFlags);


/* forward declarations */

void vHelp (void);
void vInitCom1 (void);

int iGetLoadAddress (char* achParam, UINT32* pdLoadAddr);

int iReadImage (char* achParam, UINT32 dImageAddr, IMAGE_DATA* psImgDat);
int iParseImage (UINT32 dImageAddr, IMAGE_DATA* psImgDat);

void vCopyFirstStage (UINT32 dImageAddr, UINT32 dLoadAddr,
		IMAGE_DATA* psImgDat);
void vCopySecondStage (UINT32 dImageAddr, IMAGE_DATA* psImgDat);

void vEnableA20 (void);

int main
(
	int		argc,
	char*	argv[]
)
{
	IMAGE_DATA sImgDat;
	int		iArg;
	int		iPause = 0;
	
	UINT32	dLoadAddr = 0L;
	UINT32	dFlags = 0L;		// all disabled by default


	printf ("BIT Loader V%i.%02i"
			" - Copyright (c) 2008-2013 Concurrent Technologies\n\n",
			V_MAJOR, V_MINOR);

	/*
	 * Parse the command line parameters
	 */

	if (argc < 3)
	{
		vHelp ();
		return (1);
	}

	if (iGetLoadAddress (argv[2], &dLoadAddr) != 0)
		return (2);

	for (iArg = 3; iArg < argc; iArg++)
	{
		if (strcmpi (argv[iArg], "-debug") == 0)
		{
			dFlags |= 0x01L;	// enable COM1 debugging
			vInitCom1 ();
		}
		
		else if (strcmpi (argv[iArg], "-pause") == 0)
		{
			iPause = 1;			// pause before starting image
		}

		else if (strcmpi (argv[iArg], "-pbit_10s") == 0)
		{
			dFlags |= 0x02L;	// select 10 second PBIT mode
		}

		else if (strcmpi (argv[iArg], "-cute") == 0)
		{
			dFlags |= 0x08L;	// select cute mode
		}

		else if (strcmpi (argv[iArg], "-sth") == 0)
		{
			dFlags |= 0x02L;	// select cute mode
		}

		else if (strcmpi (argv[iArg], "-pbit_60s") == 0)
		{
			dFlags |= 0x12L;	// select 60 second PBIT mode
		}

		else if (strcmpi (argv[iArg], "-pbit_5m") == 0)
		{
			dFlags |= 0x22L;	// select 5 minute PBIT mode
		}

		else if (strcmpi (argv[iArg], "-pbit_10m") == 0)
		{
			dFlags |= 0x32L;	// select 10 minute PBIT mode
		}

		else if (strcmpi (argv[iArg], "-fail_bit") == 0)
		{
			dFlags |= 0x00L;	// select failure halts in BIT
		}

		else if (strcmpi (argv[iArg], "-fail_bios") == 0)
		{
			dFlags |= 0x40L;	// select failure halts in BIOS
		}

		else if (strcmpi (argv[iArg], "-fail_os") == 0)
		{
			dFlags |= 0x80L;	// select failure boots OS
		}

		else if (strcmpi (argv[iArg], "-fail_ibit") == 0)
		{
			dFlags |= 0xC0L;	// select failure boots IBIT
		}
		
		else
		{
			vHelp ();
			return (1);
		}
	}
	
	printf (". Load address: 0x%lX Flags: 0x%lX\n", dLoadAddr, dFlags);

	/*
	 * Load and call image
	 */

	(void)bGoBigReal ();

   vEnableA20();

	if (iReadImage (argv[1], TEMP_ADDR, &sImgDat) != 0)
		return (3);

	if (iParseImage (TEMP_ADDR, &sImgDat) != 0)
		return (20);

	/* printf (". Copy 1st stage to low memory\n"); */
	printf (". Copy 1st stage to: 0x%08lX size: %lu\n", dLoadAddr, sImgDat.dSize1);
	vCopyFirstStage (TEMP_ADDR, dLoadAddr, &sImgDat);

	/* printf (". Copy 2nd stage to address specified in 1st stage\n"); */
	printf (". Copy 2nd stage to: 0x%08lX size: %lu\n", sImgDat.dOffset2, sImgDat.dSize2);
	vCopySecondStage (TEMP_ADDR, &sImgDat);

	if (iPause)
	{
		printf ("\n. Press a key to continue\n");
		getch();
	}
	
	printf (". Calling 1st stage at: 0x%lX\n", dLoadAddr);
	vCallFirstStage (dLoadAddr, dFlags);

	/* Code will not return */

	return (0);

} /* main () */


void vHelp (void)
{
	fprintf (stderr, "Usage: loadcall <image_filename> <load_address> [-flags]\n\n"
					 "     <image_filename>  image file to load\n"
					 "     <load_address>    address where 1st-stage is loaded\n"
					 "\n"
					 "     -debug            optional flag, enables COM port debug output\n"
					 "     -pause            optional flag, pause before starting image\n"
					 "\n"
					 "     -cute             optional flag, jumps to cute if present\n"
			         "     -sth              optional flag, jumps to STH rather then MTH\n"
					 "\n"
					 "     -pbit_10s         PBIT mode, duration 10 seconds\n"
					 "     -pbit_60s                    duration 60 seconds\n"
					 "     -pbit_5m                     duration 5 minutes\n"
					 "     -pbit_10m                    duration 10 minutes\n"
					 "\n"
					 "     -fail_bit         PBIT failure halts in BIT\n"
					 "     -fail_bios        PBIT failure boots BIOS and halts\n"
					 "     -fail_os          PBIT failure boots OS\n"
					 "     -fail_ibit        PBIT failure boots BIOS and enters IBIT\n\n");
					 
} /* vHelp () */


void vInitCom1 (void)
{
	/* Init COM1 for 9600 baud, 8N1 */

	outportb (0x3FB, 0x80);		// DLAB = 1
	outportb (0x3F8, 0x0C);		// 12 = 9600
	outportb (0x3F9, 0x00);
	outportb (0x3FB, 0x03);		// DLAB = 0; 8,N,1
	outportb (0x3F9, 0x00);		// interrupts off
	outportb (0x3FC, 0x0c);		// modem lines off; no loopback

} /* vInitCom1 () */


int iGetLoadAddress
(
	char*	achParam,
	UINT32*	pdLoadAddr
)
{
	int		iStatus;


	/* Load Address */

	iStatus = sscanf (achParam, "%lx", pdLoadAddr);

	if (iStatus != 1)
	{
		fprintf (stderr, "* Load address [%s]: should be a hex number\n",
					achParam);
		return (2);
	}

	if (*pdLoadAddr >= MAX_LOAD_ADDR)
	{
		fprintf (stderr, "* Load address must be below 0x%lX\n",
					MAX_LOAD_ADDR);
		return (3);
	}

	return (0);

} /* iGetLoadAddress () */


int iReadImage
(
	char*	achParam,
	UINT32	dImageAddr,
	IMAGE_DATA* psImgDat
)
{
	FILE*	psImage;
	UINT32	dOffset = 0L;
	UINT8	bTemp;
	int		iStatus;
	long int save_pos, size_of_file;


	/* Open image file */

	psImage = fopen (achParam, "rb");

	if (psImage == NULL)
	{
		fprintf (stderr, "* Cannot open image file [%s]\n", achParam);
		return (1);
	}
	
	

    save_pos = ftell( psImage );
    fseek( psImage, 0L, SEEK_END );
    size_of_file = ftell( psImage );
    fseek( psImage, save_pos, SEEK_SET );

	

	/* Read image file into memory at 16MB */

	printf (". Loading file: %s to: 0x%08lX size: %lu\n", achParam, dImageAddr, size_of_file);

	dOffset = 0L;

	while (!feof (psImage))
	{
		iStatus = fread (&bTemp, sizeof (UINT8), 1, psImage);

		if (iStatus == 1)
		{
			vWriteByte32 (dImageAddr + dOffset, bTemp);
			dOffset++;
			
			printf (". Reading: %lu bytes\r", dOffset);
		}
		else if (!feof (psImage))
		{
			fprintf (stderr, "\n* Error reading from image file.\n");
			fclose (psImage);
			return (1);
		}
	}

	psImgDat->dImageSize = dOffset;

	printf ("\n");

	fclose (psImage);
	return (0);

} /* iReadImage () */


int iParseImage
(
	UINT32	dImageAddr,
	IMAGE_DATA* psImgDat
)
{
	UINT32	d2ndStart;
	UINT32	dOffset;
	UINT32	dTemp;


	/* Check first-stage signature */

	dTemp = dReadDword32 (dImageAddr);

	if (dTemp != SIG_1ST_STAGE)
	{
		if (dTemp != CUTE_1ST_STAGE)
		{
		fprintf (stderr, "* Invalid first-stage signature:"
						 "expected 0x%08lX, got 0x%08lX\n",
						  SIG_1ST_STAGE, dTemp);
		return (1);
		}
		else
			printf ("\n. Loading CCT CUTE\n");
	}
	else
	{
		for (dOffset = 1L; ((dOffset < psImgDat->dImageSize) && (dOffset<0x40)); dOffset++)
		{
			dTemp = dReadDword32(dImageAddr + dOffset);

			if (dTemp == CUTE_1ST_STAGE)
				break;
		}

		if (dOffset == 0x40)
			printf ("\n. Loading CCT BIT\n");
		else
			printf ("\n. Loading CCT BIT/CUTE\n");
	}

	/* Read parameters from first-stage */

	psImgDat->dCsum1 = dReadDword32 (dImageAddr + 4);
	psImgDat->dSize1 = dReadDword32 (dImageAddr + 8);
	printf (". 1st stage csum: 0x%08lX size: 0x%08lX\n", psImgDat->dCsum1, psImgDat->dSize1 );

	/* Read 2nd stage load address */
	psImgDat->dOffset2 = dReadDword32 (dImageAddr + 12);

	/* Find second-stage signature */

	for (dOffset = 1L; dOffset < psImgDat->dImageSize; dOffset++)
	{
		dTemp = dReadDword32 (dImageAddr + dOffset);

		if (dTemp == SIG_2ND_STAGE)
		{
			printf (". 2nd stage found at offset: 0x%lX\n", dOffset );
			break;
		}
	}

	if (dOffset == psImgDat->dImageSize)
	{
		fprintf (stderr, "* Could not find second-stage signature\n");
		return (1);
	}


	/* Verify first-stage image size */

	if (psImgDat->dSize1 != dOffset)
	{
		fprintf (stderr, "* First-stage size is invalid: "
						 "expected 0x%08lX, got 0x%08lX\n",
							psImgDat->dSize1, dOffset);
		return (1);
	}

	/* Checksum the first stage - starts with size field */

	dTemp = 0L;

	for (dOffset = 8L; dOffset < psImgDat->dSize1; dOffset++)
		dTemp += (UINT32)bReadByte32 (dImageAddr + dOffset);

	/* Verify first-stage checksum */

	if (psImgDat->dCsum1 != dTemp)
	{
		fprintf (stderr, "* First-stage checksum is invalid: "
						 "expected 0x%08lX, got 0x%08lX\n",
							psImgDat->dCsum1, dTemp);
		return (1);
	}


	/* Verify second-stage image size */

	d2ndStart = psImgDat->dSize1;


	psImgDat->dCsum2 = dReadDword32 (dImageAddr + d2ndStart + 4);
	psImgDat->dSize2 = dReadDword32 (dImageAddr + d2ndStart + 8);
	printf (". 2nd stage csum: 0x%08lX size: 0x%08lX\n", psImgDat->dCsum2, psImgDat->dSize2 );


	dTemp = psImgDat->dImageSize - d2ndStart;

	if (psImgDat->dSize2 != dTemp)
	{
		fprintf (stderr, "* Second-stage size is invalid: "
						 "expected 0x%08lX, got 0x%08lX\n",
							psImgDat->dSize2, dTemp);
		return (1);
	}

	/* Checksum the second stage - starts with size field */

	dTemp = 0L;

	for (dOffset =  8L; dOffset < psImgDat->dSize2; dOffset++)
		dTemp += (UINT32)bReadByte32 (dImageAddr + d2ndStart + dOffset);

	/* Verify first-stage checksum */

	if (psImgDat->dCsum2 != dTemp)
	{
		fprintf (stderr, "* Second-stage checksum is invalid: "
						 "expected 0x%08lX, got 0x%08lX\n",
							psImgDat->dCsum2, dTemp);
		return (1);
	}

	printf (". Image validated\n");

	return (0);

} /* iParseImage () */


void vCopyFirstStage
(
	UINT32 dImageAddr,
	UINT32 dLoadAddr,
	IMAGE_DATA* psImgDat
)
{
	UINT32	dOffset;
	UINT8	bTemp;


	/* Copy data to sub-megabyte address */

	for (dOffset = 0L; dOffset < psImgDat->dSize1; dOffset++)
	{
		bTemp = bReadByte32 (dImageAddr + dOffset);
		vWriteByte32 (dLoadAddr + dOffset, bTemp);
	}

} /* vCopyFirstStage () */


void vCopySecondStage
(
	UINT32 dImageAddr,
	IMAGE_DATA* psImgDat
)
{
	UINT32	dOffset;
	UINT8	bTemp;


	/* Copy data to 2nd stage address */

	for (dOffset = 0L; dOffset < psImgDat->dSize2; dOffset++)
	{
		bTemp = bReadByte32 (dImageAddr + psImgDat->dSize1 + dOffset);
		vWriteByte32 (psImgDat->dOffset2 + dOffset, bTemp);
	}
	
} /* vCopySecondStage () */


void vEnableA20 (void)
{
	UINT32	dDataAt1MB;
	UINT32	dTemp;
	UINT8	bTemp;


	/* First try to enable A20 using port-92 */

	bTemp = inportb (0x92);

	if (bTemp != 0xFF)
	{
		outportb (0x92, bTemp | 0x02);		// enable A20?

		disable ();
		dDataAt1MB = dReadDword32 (0x100000L);	// save data at 1MB

		vWriteDword32 (0x100000L, 0x12345678L);	// set test pattern at 1MB
		dTemp = dReadDword32 (0L);				// read data at 0

		vWriteDword32 (0x100000L, dDataAt1MB);	// restore data at 1MB
		enable ();

		if (dTemp != 0x12345678L)			// if pattern does not appears at
			return;							// offset  0, A20 set
	}

	/* Get here if port-92 failed */


	/* Enable A20 via keyboard controller */

	while ((inportb (0x64) & 0x02) != 0)
		delay (1);

	outportb (0x64, 0xD1);
	while ((inportb (0x64) & 0x02) != 0)
		delay (1);

	outportb (0x60, 0xDF);
	while ((inportb (0x64) & 0x02) != 0)
		delay (1);

	outportb (0x64, 0xFF);
	while ((inportb (0x64) & 0x02) != 0)
		delay (1);

} /* vEnableA20 () */


