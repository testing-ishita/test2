/* includes */

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>
#include "bit_extinfo.h"

BoardVariants	*firstVariant;

int main
	(
		int argc,
		char* argv[]
	)
{
	UINT16			rt = E_OK;
	FILE			*cuteImage;
	UINT16			i, j;
	UINT8			count=0;
	BoardVariants  *temp;
	char			input;
	IMAGE_DATA      imgData;

    printf ( "\nBIT Image Extended Info Postprocessor V%i.%02i"
    		 "\nCopyright (c) 2011 Concurrent Technologies\n\n",
             V_MAJOR, V_MINOR);

    /* Parse the command line parameters */
	if (argc != 3)
	{
		fprintf (stderr, "Usage: bit_extinfo <cute_image> <info_file>\n\n");
		rt = BAD_ARGS;
	}

	/* open cute image file as read-only first to check if the file exists.*/
	if(rt == E_OK)
	{
		cuteImage = fopen (argv[1], "rb");
		if (cuteImage == NULL)
		{
			fprintf (stderr, "* Cannot open CUTE image file [%s]\n", argv[1]);
			rt = NO_CUTE_IMG;
		}
		else
		{
			fclose (cuteImage);						// close cute image file
			cuteImage =  fopen (argv[1], "rb+");	// reopen for appending

			memset(&imgData, 0x00, sizeof(IMAGE_DATA));
			rt = iReadImage( cuteImage, &imgData);
		}
	}

	/* open extended info image file and parse info.*/
	if(rt == E_OK)
	   rt = iReadInfoImage (argv[2], &count);

	/* Select a variant to program.*/
	if(rt == E_OK)
	{
	  	temp = firstVariant;
	  	printf("\n\nVariants found %d\n-----------------------\n", count);
	  	i = 1;

	  	do
	  	{
			printf("%d - %s\n", i, temp->Name);
			i++;
			temp = (BoardVariants*)temp->nextVariant;

	  	} while(temp!= NULL);

		printf("\n");
		do
		{
			printf("\rSelect Variant to program:- ");
			input = getche();
			j = atoi(&input);
		} while((j <=0) || (j > count) );
	}

	if(rt == E_OK)
	{
		//append variant info to cute file
		temp = firstVariant;
		i = 1;
		do
		{
			if(i == j)
			{
				printf("\n\n. Programming %s \n", temp->Name);
				break;
			}
			i++;
			temp = (BoardVariants*)temp->nextVariant;

		} while(temp!= NULL);

		rt = processVariant(temp);
		if(rt == E_OK)
			rt = iReWriteImage(cuteImage, &imgData, temp);
	}

	fclose (cuteImage);

	//cleanup the variant list
	do
	{
		temp = (BoardVariants*)firstVariant->nextVariant;
		free(firstVariant->Name);
		free(firstVariant->data);
		free(firstVariant);
		firstVariant = temp;

	}while(firstVariant != NULL);

	if(rt != E_OK)
		printf("\n. ERROR CODE %x\n", rt);

	return rt;

} /* main () */




/*****************************************************************************
 * wIsNumber
 * Check is string in achBuffer dec or hex number
 * RETURNS: E_OK if it is OK
 */
static UINT16 wIsNumber
	(
		char achBuffer[]
	)
{
	UINT16	wTestStatus = E_OK;
    UINT8	bIndex;

    if (strlen (achBuffer) > 4 || strlen (achBuffer) == 0)
    	wTestStatus = E_FILE_TEXT;	/* string is too long or empty */

    else if (strlen (achBuffer) >= 3 && achBuffer[0] == '0' &&
    		 toupper (achBuffer[1]) == 'X')
    {
        /* check hex number */
    	if (isxdigit (achBuffer[2]) == 0)
        	wTestStatus = E_FILE_TEXT;
        else if (strlen (achBuffer) == 4 && isxdigit (achBuffer[3]) == 0)
        	wTestStatus = E_FILE_TEXT;
    }
    else if (strlen (achBuffer) <= 3)
    {
    	for (bIndex = 0; bIndex < strlen (achBuffer); bIndex++)
        	if (isdigit (achBuffer[bIndex]) == 0)
            	wTestStatus = E_FILE_TEXT;
    }

    return wTestStatus;
}


int iReadInfoImage
	(
		char* achParam,
		UINT8* vcount
	)
{
	FILE 	*psFileFru;
	UINT16 	wTestStatus = E_OK;
    UINT16	wNumber;
    char	achBuffer[200];
    char 	achTemp [200];
	UINT8   start = 0, count = 0;
	UINT8   linecount = 0;
	char    localName[16];
	UINT16	wLength=0;
	UINT8	*pbImage;

	BoardVariants  *temp;
	BoardVariants  *currentVarient;

	firstVariant   = NULL;
	currentVarient = firstVariant;

	pbImage = (UINT8*)malloc (INFO_SIZE);
    if (pbImage == NULL)
    {
    	fprintf (stderr, "Unable to allocate info buffer.\n\n");
       	return MEM_ALLOC;
    }

	memset(localName, 0x00, 16);
	sprintf(localName, "%s", "BOARD_VARIENT");

	psFileFru = fopen (achParam, "r"); //open as text file
	if (psFileFru == NULL)
    {
    	printf ("\nCan't open Info file %s\n", achParam);
    	free(pbImage);
    	return INFO_OPEN;
    }

	while(wTestStatus == E_OK)
	{
		if (fscanf (psFileFru, "%s", achBuffer) == EOF)	/* check EOF */
		{
			wTestStatus = E_EOF;
		}
		else
		{
			if (achBuffer [0] == ';')	/* check if this line is a comment */
			{
				linecount++;

				memset(achTemp, 0x00, 200);
				count = 0;

				do
				{
					achTemp [count] = fgetc (psFileFru);

				} while (achTemp [count] != '\n' && achTemp [count++] != EOF);

				continue;
			}


			if (achBuffer [0] == '#')	/* check if this line is a tag */
			{
				linecount++;

				memset(achTemp, 0x00, 200);
				count = 0;

				do
				{
					achTemp [count] = fgetc (psFileFru);

				} while (achTemp [count] != '\n' && achTemp [count++] != EOF);

				if (achTemp [0] == EOF || (achTemp [count--] == EOF))
				{
					wTestStatus =  E_EOF;
					continue;
				}
				else
				{
					if(start == 1)
					{
						//printf(" Variant name %s \n", achTemp);
						start = 2;

						temp = malloc(sizeof(BoardVariants));
						memset(temp, 0x00, sizeof(BoardVariants));

						if(firstVariant ==  NULL)
						{
							firstVariant = temp;
							currentVarient = temp;
						}
						else
						{
							currentVarient->nextVariant = (void*)temp;
							currentVarient = temp;
						}

						currentVarient->Name = malloc(strlen(achTemp));
						sprintf(currentVarient->Name, "%s", achTemp );
						(*vcount)++;
						continue;
					}
					else if(strstr(achTemp, localName)!=NULL)
					{
						if(start == 0)
						{
							//printf(" %s Variant Found at, line %d, %s\n", localName, linecount, achTemp);
							start = 1;
							continue;
						}
						else
						{
							start = 0;

							currentVarient->length = wLength;
							currentVarient->data = malloc(wLength);
							memcpy(currentVarient->data, pbImage, wLength );
							wLength = 0;
							//printf("looking for next record\n");
							continue;
						}
					}
					else
					{
						continue;
					}
				}
			}


			if(start == 0)
			{
				memset(achTemp, 0x00, 200);
				count = 0;
				linecount++;
				do
				{
					achTemp [count] = fgetc (psFileFru);
				}
				while (achTemp [count] != '\n' && achTemp [count++] != EOF);

				if (achTemp [0] == EOF || (achTemp [count--] == EOF))
				{
					wTestStatus = E_EOF;
					continue;
				}
				else
				{
					continue;
				}
			}


			linecount++;

			if (strlen (achBuffer) > 0 && achBuffer [strlen(achBuffer) - 1] == ',') /* ignore ',' at end */
				achBuffer [strlen (achBuffer) - 1] = 0x0;

			if (strlen (achBuffer) == 1 && achBuffer[0] == 0x27)/* check for ' ' */
			{
				if (fscanf (psFileFru, "%s", achTemp) != EOF)
				{
					if (strlen (achTemp) > 0 &&	achTemp [strlen(achTemp) - 1] == ',')/* ignore ',' at end */
						achTemp [strlen (achTemp) - 1] = 0x0;

					if (strlen (achTemp) == 1 && achTemp[0] == 0x27)/* ' ' */
					 	pbImage [(wLength)++] = 0x20;
					else
						wTestStatus = E_FILE_TEXT;
				}
				else
					wTestStatus = E_FILE_TEXT;
			}

			else if (strlen (achBuffer) == 3 && achBuffer[0] == 0x27 &&	achBuffer[2] == 0x27)
				pbImage [(wLength)++] = achBuffer[1];  /* convert char which is in form 'a' */

			else if (wIsNumber (achBuffer) == E_OK)	   /* convert dec or hex number */
			{
				sscanf (achBuffer, "%i", &wNumber);

				if (wNumber < 256)
					pbImage [(wLength)++] = (UINT8) wNumber;
				else
					wTestStatus = E_FILE_TEXT;	/* number is not byte */
			}
			else
				wTestStatus = E_FILE_TEXT;
		}
	}

    if (wTestStatus == E_FILE_TEXT)
    	printf ("\nError to convert text: %s\n", achBuffer);

	fclose (psFileFru);
	free(pbImage);

	if( (wTestStatus == E_EOF) && (*vcount >0) )
	{
		return E_OK;
	}
	else
		return wTestStatus;
}



int iReadImage
	(
		FILE*		psImage,
		IMAGE_DATA* psImgDat
	)
{
	UINT32	dOffset = 0L;
	UINT8	bTemp;
	int		iStatus;
	UINT8   imgBuff[4];
	UINT8   buffCount = 0;
	UINT32	dTemp;

	printf (". Reading image file\n");

	if(fseek(psImage, 0, SEEK_SET)!=0)
	{
		printf ("CUTE READ SEEK FAILED\n");
		return READ_SEEK_FAILED;
	}

	dOffset = 0L;

	while (!feof (psImage))
	{
		iStatus = fread (&bTemp, sizeof (UINT8), 1, psImage);

		if (iStatus == 1)
        {
        	if(buffCount > 3)
        		buffCount = 0;

        	imgBuff[buffCount++] = bTemp;
			dOffset++;

			if(dOffset == SIGNATURE_OFFSET)
			{
				/* Check first-stage signature */
				dTemp = *(UINT32*)(imgBuff);

				if ( (dTemp != CUTE_1ST_STAGE) && (dTemp != SIG_1ST_STAGE) )
				{
					fprintf (stderr, "* Invalid first-stage signature:"
							 "expected 0x%08lX, got 0x%08lX\n",
						  	 SIG_1ST_STAGE, dTemp);
					return INVD_1ST_STAGE;
				}
				else
					printf (". CCT BIT/CUTE\n");
			}

			if(dOffset == STAGE_SIZE_OFFSET)
			{
				psImgDat->dSize1 = *(UINT32*)(imgBuff);
				printf (".  part-1 size: 0x%08lX\n", psImgDat->dSize1);
			}

			if(psImgDat->dSize1 > 0)
				if(dOffset == psImgDat->dSize1 + SIGNATURE_OFFSET)
				{
					/* Check second-stage signature */
					dTemp = *(UINT32*)(imgBuff);

					if ( (dTemp != SIG_2ND_STAGE) )
					{
						fprintf (stderr, "* Invalid first-stage signature:"
								 "expected 0x%08lX, got 0x%08lX\n",
							   	 SIG_2ND_STAGE, dTemp);
						return INVD_2ND_STAGE;
					}
					else
						printf (". Second stage\n");
				}

			if(psImgDat->dSize1 > 0)
				if(dOffset == psImgDat->dSize1 + STAGE_SIZE_OFFSET)
				{
					psImgDat->dSize2 = *(UINT32*)(imgBuff);
					printf (".  part-2 size: 0x%08lX\n", psImgDat->dSize2);
				}
        }
		else if (!feof (psImage))
		{
			printf ("* Error reading from image file.\n");
			return CUTE_READ;
		}

	}

	psImgDat->dImageSize = dOffset;
	printf (". Read 0x%08lX bytes.\n", dOffset);
	return (0);

} /* iReadImage () */



int iReWriteImage
	(
		FILE          *psImage,
		IMAGE_DATA	  *psImgDat,
		BoardVariants *variant
	)
{
	UINT32	dOffset = 0L;
	UINT8	bTemp;
	int		iStatus;

	if (fseek(psImage, (psImgDat->dSize1 + psImgDat->dSize2), SEEK_SET) != 0)
	{
		printf ("\n seek failed\n");
		return WRITE_SEEK_FAILED;
	}

	for (dOffset = 0L; dOffset < variant->length; dOffset++)
	{
		bTemp = variant->data[dOffset];
		iStatus = fwrite (&bTemp, sizeof (UINT8), 1, psImage);

		if (iStatus != 1)
		{
			printf ("FAIL\n");

			fprintf (stderr, "* Error writing to image file.\n");
			fclose (psImage);
			return WRITE_FAILED ;
		}
	}

	printf ("OK, Wrote 0x%08lX bytes.\n", dOffset);

	return (0);

} /* iReWriteImage () */



int processVariant
	(
		BoardVariants  *variant
	)
{
	UINT16  wTestStatus = E_OK;
	UINT32	dTemp, csum, fields, length, counter;

	if(variant->length < FIELD_START )
	{
		printf("\nIncorrect INFO Header\n");
		return(INCORRECT_INFO_HEADER);
	}

	length  = 0;
	fields  = 0;
	counter = FIELD_START;// start after the header

	// check if there is any data after the header
	// count number of fields and calculate length
	while(  variant->length > (UINT16 ) counter )
	{
		//check if there is a minimum field
		if(  variant->length > (UINT16)(counter + MIN_FIELD) )
		{
			fields++;
			dTemp   = *(UINT32*)(variant->data + counter + LENGTH_OFFSET );
			dTemp   = dTemp;
			length += FIELD_HEAD_COUNT + dTemp;
			counter = counter + dTemp + FIELD_HEAD_COUNT;
		}
		else
		{
			printf("\nGarbage at the end of field section\n");
			return(GARBAGE_AT_THE_END);
		}
	}

	//check if everything is consistent
	if((length + FIELD_START) != variant->length)
	{
		printf("\nMalformed Variant, bytes %d\n", (length + FIELD_START));
		return(MALFORMED_VARIANT);
	}

	//calculate field checksum
	csum = 0;
	for(counter = 0; counter < length ; counter+=4)
	{
		dTemp = *(UINT32*)(variant->data + counter + FIELD_START);
		dTemp = dTemp;
		csum += dTemp;
	}

	*(UINT32*)(variant->data + FIELD_COUNT )    = fields;
	*(UINT32*)(variant->data + FIELD_LENGTH )   = length;
	*(UINT32*)(variant->data + FIELD_CHECKSUM ) = (-csum);

	//calculate header checksum
	csum = 0;
	for(counter = 0; counter < FIELD_START ; counter+=4)
	{
		dTemp = *(UINT32*)(variant->data + counter );
		csum += dTemp;
	}

	*(UINT32*)(variant->data + HEADER_CHECKSUM ) = (-csum);

	for(counter = 0; counter<variant->length ; counter+=4)
	{
		dTemp   = *(UINT32*)(variant->data + counter );
#if 0
		printf("offset 0x%08lX\t", counter);
		printf("Data 0x%08lX\t", dTemp );
		printf("%c %c %c %c\n", (UINT8)(dTemp>>24),
								(UINT8)(dTemp>>16),
								(UINT8)(dTemp>>8),
								(UINT8)(dTemp));
#endif
	}

	return wTestStatus;

}

