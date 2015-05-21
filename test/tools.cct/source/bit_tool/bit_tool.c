/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>

/* defines */

#define V_MAJOR			(1)
#define V_MINOR			(0)

#define ROM_SIZE		(0x80000L)

#define SIG_1ST_STAGE	(0x31544942L)	/* "BIT1", 'B' in LSB */
#define CUTE_1ST_STAGE	(0x45545543L)	    /* "CUTE", 'C' in LSB */
#define SIG_2ND_STAGE	(0x32544942L)	/* "BIT2"	-"-		  */


/* typedefs */

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned long	UINT32;

typedef struct tagImageData
{
	UINT32	dImageSize;

	UINT32	dSize1;
	UINT32	dCsum1;

	UINT32	dOffset2;
	UINT32	dSize2;
	UINT32	dCsum2;

} IMAGE_DATA;

/* globals */

/* forward declarations */

int iReadImage (char* achParam, UINT8* pbImage, IMAGE_DATA* psImgDat);
int iReWriteImage (char* achParam, UINT8* pbImage, IMAGE_DATA* psImgDat);

int iParseImage (UINT8* pbImage, IMAGE_DATA* psImgDat);



int main
(
	int		argc,
	char*	argv[]
)
{
	UINT8*	pbImage = NULL;

	IMAGE_DATA sImgDat;


    printf ("\nBIT Image Preprocessor V%i.%02i"
    		" - Copyright (c) 2008 Concurrent Technologies\n\n",
            V_MAJOR, V_MINOR);

    pbImage = (UINT8*)malloc (ROM_SIZE);

    if (pbImage == NULL)
    {
    	fprintf (stderr, "Unable to allocate image buffer.\n\n");
        return (1);
    }

	/*
	 * Parse the command line parameters
	 */

	if (argc != 2)
	{
		fprintf (stderr, "Usage: bit_tool <image_filename>\n\n");
		return (1);
	}

	/*
	 * Update image file with checksums and lengths.
	 */

	if (iReadImage (argv[1], pbImage, &sImgDat) != 0)
		return (3);

	printf (". Update image length and checksums.\n");

	if (iParseImage (pbImage, &sImgDat) != 0)
		return (10);

	if (iReWriteImage (argv[1], pbImage, &sImgDat) != 0)
		return (11);

	return (0);

} /* main () */


int iReadImage
(
	char*	achParam,
	UINT8*	pbImage,
	IMAGE_DATA* psImgDat
)
{
	FILE*	psImage;
	UINT32	dOffset = 0L;
	UINT8	bTemp;
	int		iStatus;


	/* Open image file */

	psImage = fopen (achParam, "rb");

	if (psImage == NULL)
	{
		fprintf (stderr, "* Cannot open image file [%s]\n", achParam);
		return (1);
	}

	/* Read image file into memory at 16MB */

	printf (". Reading image file %s: ", achParam);

	dOffset = 0L;

	while (!feof (psImage))
	{
		iStatus = fread (&bTemp, sizeof (UINT8), 1, psImage);

		if (iStatus == 1)
        {
			*(pbImage + dOffset) = bTemp;
			dOffset++;
        }
        
		else if (!feof (psImage))
		{
			printf ("FAIL\n");

			fprintf (stderr, "* Error reading from image file.\n");
			fclose (psImage);
			return (1);
		}

	}

	psImgDat->dImageSize = dOffset;

	printf ("OK, Read 0x%08lX bytes.\n", dOffset);

	fclose (psImage);
	return (0);

} /* iReadImage () */


int iReWriteImage
(
	char*	achParam,
	UINT8*	pbImage,
	IMAGE_DATA* psImgDat
)
{
	UINT32	dOffset = 0L;

	FILE*	psImage;
	UINT8	bTemp;
	int		iStatus;


	/* Open image file */

	psImage = fopen (achParam, "wb");

	if (psImage == NULL)
	{
		fprintf (stderr, "* Cannot open image file [%s]\n", achParam);
		return (1);
	}

	/* Write memory at 16MB to image file */

	printf (". Rewrite image file %s: ", achParam);

	for (dOffset = 0L; dOffset < psImgDat->dImageSize; dOffset++)
	{
		bTemp = *(pbImage + dOffset);
		iStatus = fwrite (&bTemp, sizeof (UINT8), 1, psImage);

		if (iStatus != 1)
		{
			printf ("FAIL\n");

			fprintf (stderr, "* Error writing to image file.\n");
			fclose (psImage);
			return (1);
		}
	}

	printf ("OK, Wrote 0x%08lX bytes.\n", dOffset);

	fclose (psImage);
	return (0);

} /* iReWriteImage () */


int iParseImage
(
	UINT8*	pbImage,
	IMAGE_DATA* psImgDat
)
{
	UINT32	d2ndStart = 0L;
	UINT32	dOffset;
	UINT32	dTemp;
	UINT32  flag=0;


	/* Check first-stage signature */

	dTemp = *(UINT32*)(pbImage + 0);

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
			printf ("\n. CCT CUTE\n");
	}
	else
	{
		for (dOffset = 1L; ((dOffset < psImgDat->dImageSize) && (dOffset<0x40)); dOffset++)
		{
			dTemp = *(UINT32*)(pbImage + dOffset);
	
			if (dTemp == CUTE_1ST_STAGE)
				break;
		}

		if (dOffset == 0x40)
			printf ("\n. CCT BIT\n");
		else
			printf ("\n. CCT BIT/CUTE\n");
	}

	/* Read parameters from first-stage */

	psImgDat->dOffset2 = *(UINT32*)(pbImage + 12);

    printf ("\n  part-2 addr: 0x%08lX\n", psImgDat->dOffset2);

	/* Find second-stage signature */

	for (dOffset = 1L; dOffset < psImgDat->dImageSize; dOffset++)
	{
		dTemp = *(UINT32*)(pbImage + dOffset);

		if (dTemp == SIG_2ND_STAGE)
			break;
	}

	if (dOffset == psImgDat->dImageSize)
	{
		fprintf (stderr, "* Could not find second-stage signature\n");
		return (1);
	}


	/*  Update first-stage image size */

	*(UINT32*)(pbImage + 8) = dOffset;
	psImgDat->dSize1 = dOffset;

	/* Checksum the first stage - starts with size field */

	dTemp = 0L;

	for (dOffset = 8L; dOffset < psImgDat->dSize1; dOffset++)
		dTemp += (UINT32)*(pbImage + dOffset);

	/* Update first-stage checksum */

	*(UINT32*)(pbImage + 4) = dTemp;
	psImgDat->dCsum1 = dTemp;

    printf ("\n  part-1 size: 0x%08lX\n", psImgDat->dSize1);
    printf ("  part-1 csum: 0x%08lX\n", psImgDat->dCsum1);


	/* Update second-stage image size */

	d2ndStart = dOffset;
	dTemp = psImgDat->dImageSize - d2ndStart;

	*(UINT32*)(pbImage + d2ndStart + 8) = dTemp;
	psImgDat->dSize2 = dTemp;

	/* Checksum the second stage - starts with size field */

	dTemp = 0L;

	for (dOffset = 8L; dOffset < psImgDat->dSize2; dOffset++)
		dTemp += (UINT32)*(pbImage + d2ndStart + dOffset);

	/* Verify or Update first-stage checksum */

	*(UINT32*)(pbImage + d2ndStart + 4) = dTemp;
	psImgDat->dCsum2 = dTemp;

    printf ("\n  part-2 size: 0x%08lX\n", psImgDat->dSize2);
    printf ("  part-2 csum: 0x%08lX\n\n", psImgDat->dCsum2);

	return (0);

} /* iParseImage () */




