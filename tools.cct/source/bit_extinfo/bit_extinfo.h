/*
 * bit_extinfo.h
 *
 *  Created on: 6 Apr 2011
 *      Author: engineer
 */

#ifndef BIT_EXTINFO_H_
#define BIT_EXTINFO_H_


#define V_MAJOR					(1)
#define V_MINOR					(0)


#define INFO_SIZE				(0x1000L)


#define SIG_1ST_STAGE			(0x31544942L)	/* "BIT1", 'B' in LSB */
#define CUTE_1ST_STAGE			(0x45545543L)	/* "CUTE", 'C' in LSB */
#define SIG_2ND_STAGE			(0x32544942L)	/* "BIT2"	-"-		  */


typedef unsigned char			UINT8;
typedef unsigned short			UINT16;
typedef unsigned long			UINT32;


#define E_OK 					0x00
#define INFO_OPEN 				0x01
#define E_EOF 					0x02
#define E_FILE_TEXT 			0x03
#define BAD_ARGS				0x04
#define NO_CUTE_IMG				0x05
#define MEM_ALLOC				0x06
#define INVD_1ST_STAGE			0x07
#define INVD_2ND_STAGE			0x08
#define CUTE_READ				0x09
#define READ_SEEK_FAILED		0x0a
#define WRITE_SEEK_FAILED 		0x0b
#define WRITE_FAILED 			0x0c
#define INCORRECT_INFO_HEADER 	0x0d
#define GARBAGE_AT_THE_END    	0x0e
#define MALFORMED_VARIANT     	0x0f


#define SIGNATURE_OFFSET		4
#define STAGE_SIZE_OFFSET		12
#define FIELD_START				40
#define MIN_FIELD				12
#define TYPE_OFFSET				0
#define LENGTH_OFFSET	 		4
#define FIELD_HEAD_COUNT		8
#define FIELD_COUNT				24
#define FIELD_LENGTH	 		28
#define FIELD_CHECKSUM			32
#define HEADER_CHECKSUM			36


#define BYTE_SWAP( data )	(((data & 0x000000FFUL) << 24 ) |\
		                     ((data & 0x0000FF00UL) << 8  ) |\
		                     ((data & 0x00FF0000UL) >> 8  ) |\
		                     ((data & 0xFF000000UL) >> 24 ));


typedef struct Board_Variants
{
	UINT8	*Name;
	UINT16 	 length;
	UINT8	*data;
	void  	*nextVariant;

} BoardVariants;


typedef struct tagImageData
{
	UINT32	dImageSize;
	UINT32	dSize1;
	UINT32	dOffset2;
	UINT32	dSize2;

} IMAGE_DATA;


int iReadInfoImage (char* achParam, UINT8* vcount);
int iReadImage     (FILE* psImage, IMAGE_DATA* psImgDat);
int processVariant (BoardVariants  *variant);
int iReWriteImage  (FILE* psImage, IMAGE_DATA* psImgDat, BoardVariants  *variant);


#endif /* BIT_EXTINFO_H_ */
