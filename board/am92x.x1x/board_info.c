
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
/*
 * board_info.c
 *
 * - searches for and parses BIOS_INFO and  EXTENDED_INFO structures
 * - provides services to retrieve specific info obtained from these
 *   structures
 *
 *  Created on: 31 Mar 2011
 *      Author: Haroon
 */
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/am92x.x1x/board_info.c,v 1.3 2013-10-08 07:16:34 chippisley Exp $
 *
 * $Log: board_info.c,v $
 * Revision 1.3  2013-10-08 07:16:34  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.2  2013/09/27 12:27:16  chippisley
 * Added board service to retrieve slot ID.
 *
 * Revision 1.1  2013/09/04 07:13:27  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */

#include <stdtypes.h>
#include <stdio.h>
#include <string.h>

#include <private/port_io.h>
#include <private/sys_delay.h>
#include <private/debug.h>
#include <private/k_alloc.h>

#include <bit/bit.h>
#include <bit/mem.h>
#include <bit/delay.h>
#include <bit/board_service.h>

#include "board_info.h"


#undef DEBUG

static EXTENDED_INFO   cuteInfo;
static BIOS_INFO	   BIOSInfo;

static int 	  readExtendedImage (UINT32 const *psImage, IMAGE_DATA* psImgDat);
static int 	  checkExtendedInfo (UINT32 const *psImage, IMAGE_DATA* psImgDat);
static int 	  parseExtendedInfo (void);
static UINT32 getExtendedField  (UINT32 type, UINT8 *data, UINT32 size);
static UINT32 findExtendedField (UINT32 type);

static int 	  checkBiosInfo     (UINT32 const *psImage);
/* static UINT32 findBIOSField     (UINT32 type); */
static int    parseBiosInfo     (void);


extern UINT8 bGetAMCIpmbAddrs (void);


/* ****************************************************************************
 *								 EXTENDED INFO				 			      *
 * ****************************************************************************/

/***********************************************************************
 * brdGetExtendedInfo: searches for and verifies EXTENDED_INFO structure
 * ptr	  			 : NULL
 * RETURNS			 : E__OK on success
 */
UINT32 brdGetExtendedInfo (void* ptr)
{
	UINT32   cutestart;
	PTR48 	 tPtr1;
	volatile UINT32 mapCuteSpace=0;
	UINT32 	 mHandle;
	static   IMAGE_DATA imgData;
	UINT32   rt = E__OK;

	#ifdef DEBUG
		char    buffer[64];
	#endif

		/* Calculation of CUTE IMAGE start*/
	cutestart = MEMORY_TOP - CUTE_START;

	#ifdef DEBUG
		sprintf(buffer, "\n\n. CUTE image offset %x\n", cutestart);
		sysDebugWriteString (buffer);
	#endif

	mHandle = dGetPhysPtr(cutestart, CUTE_SIZE, &tPtr1, (void*)&mapCuteSpace);
	if(mHandle != NULL)
	{
		memset(&imgData, 0x00, sizeof(IMAGE_DATA));
		rt = readExtendedImage((UINT32*)mapCuteSpace, &imgData);
		if( rt == E__OK)
		{
			rt = checkExtendedInfo((UINT32*)mapCuteSpace, &imgData);
			if(rt == E__OK)
				rt = parseExtendedInfo();
		}
		vFreePtr(mHandle);
	}
	else
	{
		sysDebugWriteString ("\n* Memory Allocation Failed\n");
		rt = E__FAIL;
	}

	return rt;

} /* brdGetExtendedInfo () */


/*********************************************************************
 * findExtendedField: Searches for a specific filed in the info buffer
 * type	            : UINT32 data indicating filed type to locate
 * RETURNS          : Field offset or E__FAIL
 */
static UINT32 findExtendedField
	(
			UINT32 type		// field type
	)
{
	UINT32 count  = 0;
	UINT32 dTemp  = 0;
	UINT32 rt     = EXT_FIELD_NOT_PRSENT;

	if(cuteInfo.present != 1)
		return EXT_INFO_NOT_PRSENT;

	while(count < cuteInfo.length)
	{
		dTemp = *((UINT32*)(cuteInfo.data + count));

		if(dTemp == type)
		{
			rt = E__OK;
			break;
		}

		count += 4;
		dTemp  = *((UINT32*)(cuteInfo.data + count));
		count += dTemp + 4;
	}

	return rt;
} /* findExtendedField() */


/*********************************************************************
 * getExtendedField : Searches for a specific filed in the info buffer,
 * 					  then copies the field data to supplied buffer
 * type	            : UINT32 data indicating filed type to locate
 * data				: buffer to hold field data must be at least big
 * 					  enough to hold field data
 * size				: size of data buffer
 * RETURNS          : Field offset or E__FAIL
 */

static UINT32 getExtendedField
	(
			UINT32 type,		// field type
			UINT8 *data,		// buffer to hold field data
			UINT32 size			// size of the buffer
	)
{
	UINT32 length = 0;
	UINT32 count  = 0;
	UINT32 dTemp  = 0;
	UINT32 rt     = EXT_FIELD_NOT_PRSENT;
	UINT32 i      = 0;
#ifdef DEBUG
	char  buffer[64];
#endif

	if(cuteInfo.present != 1)
		return EXT_INFO_NOT_PRSENT;

	while(count < cuteInfo.length)
	{
		dTemp = *((UINT32*)(cuteInfo.data + count));

		if(dTemp == type)
		{
			count += 4;
			length = *((UINT32*)(cuteInfo.data + count));
			if(length > size)
			{
				#ifdef DEBUG
					sprintf(buffer, "\n* data buffer too small=%d, length=%d", size, length);
					sysDebugWriteString (buffer);
				#endif
				rt = EXT_INFO_BUFFER_TOO_SMALL;
				break;
			}

			//copy data here
			count += 4;
			for(i = 0; i < length; i++)
				data[i] = *(cuteInfo.data + count + i);

			rt = E__OK;
			break;
		}

		count += 4;
		dTemp  = *((UINT32*)(cuteInfo.data + count));
		count += dTemp + 4;
	}

	return rt;
} /* getExtendedField() */



/**********************************************************************
 * parseExtendedInfo: parses the info fields to make sure the information
 *                    is correct
 * RETURNS			: E__OK or error code
 */
static int parseExtendedInfo(void)
{
	UINT32 fields = 0;
	UINT32 count  = 0;
	UINT32 dTemp  = 0;
	UINT32 rt     = E__FAIL;
	UINT32 i      = 0;
	UINT8  buff[16];
	char  buffer[64];

	while(count < cuteInfo.length)
	{
		dTemp = *((UINT32*)(cuteInfo.data + count));

		if(dTemp == 0x80000000)
		{
			fields++;
			count += 4;
			dTemp = *((UINT32*)(cuteInfo.data + count));
			if(dTemp != 12)
			{
				sprintf(buffer, "\n* wrong length=%d", dTemp);
				sysDebugWriteString (buffer);
				rt = E__FAIL;
				break;
			}

			count += 4;
			for(i = 0; i < 16; i++)
				buff[i] = *(cuteInfo.data + count + i);
			buff[15] = '\0';

			if(strcmp( (char *)buff, "FIELD-END" ) != 0)
			{
				rt = E__FAIL;
				sprintf(buffer, "\n* Fields End Failed! found=%s", buff);
				sysDebugWriteString (buffer);
			}
			else
			{
				if(fields != cuteInfo.field_count)
				{
					rt = E__FAIL;
					sprintf(buffer, "\n* Fields=%d cuteInfo.field_count=%d", fields, cuteInfo.field_count);
					sysDebugWriteString (buffer);
				}
				else
					rt = E__OK;
			}

			break;
		}

		fields++;
		count += 4;
		dTemp  = *((UINT32*)(cuteInfo.data + count));
		count += dTemp + 4;
	}

	if(rt == E__OK)
	{
		cuteInfo.present = 1;
		#ifdef DEBUG
			sysDebugWriteString (". Info Verified\n");
		#endif
	}
	else
		sysDebugWriteString ("\n* Info NOT Verified\n");

	return rt;

}//parseExtendedInfo()


/**********************************************************************
 * checkExtendedInfo : verifies the EXTENDED_INFO structure
 * psImage	         : CUTE ROM image pointer
 * imgData			 : CUTE ROM image information structure
 * RETURNS			 : E__OK or error code
 */
static int checkExtendedInfo
	(
		UINT32 const *psImage,	// CUTE ROM image pointer
		IMAGE_DATA   *imgData	// CUTE ROM image information structure
	)
{
	UINT32	dOffset = 0L;
	char   buffer[64];
	UINT32  rt = E__OK;
	UINT32  done;
	UINT8   *cuteInfoPtr;
	UINT32  count;
	UINT32  csum;
	UINT32	dTemp;

	#ifdef DEBUG
		sysDebugWriteString (". Parsing Info\n");
	#endif

	memset(&cuteInfo, 0x00, sizeof(cuteInfo));
	cuteInfoPtr = (UINT8*)&cuteInfo;

	dOffset = imgData->dSize1 + imgData->dSize2;
	done    = dOffset + FIELD_START; // read up to and including header checksum
	count   = 0;

	//copy header info to local structure
	while ( dOffset < done )
	{
		*(cuteInfoPtr+count) = *((UINT8*)psImage + dOffset);
		dOffset++;
		count++;
	}

	//check header checksum
	csum    = 0;
	dOffset = imgData->dSize1 + imgData->dSize2;
	done    = dOffset + HEADER_CHECKSUM; // read upto but not including header checksum
	for(; dOffset < done ; dOffset+=4)
	{
		dTemp  = *( (UINT32*)((UINT8*)psImage + dOffset));
		csum  += dTemp;
	}
	csum = -csum;

	if(csum == cuteInfo.header_chksum)
	{
		cuteInfo.signature[15] = '\0';//make sure that the string is null terminated
		if(strcmp( (char *) cuteInfo.signature, "CUTE_CONFIGINFO") == 0)
		{
			#ifdef DEBUG
				sysDebugWriteString (". signature:    " );
				sysDebugWriteString (cuteInfo.signature );
				sprintf(buffer, "\n. Struct_ver:   %08x", cuteInfo.Struct_ver);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. Info_ver:     %08x", cuteInfo.Info_ver);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. field_count:  %08x", cuteInfo.field_count);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. length:       %08x", cuteInfo.length);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. chksum:       %08x", cuteInfo.chksum);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. header_chksum:%08x\n", cuteInfo.header_chksum);
				sysDebugWriteString (buffer);
			#endif
		}
		else
		{
			sprintf ( buffer, "\n* Wrong CUTE_CONFIGINFO signature %s\n", cuteInfo.signature);
			sysDebugWriteString (buffer);
			rt = E__FAIL;
		}
	}
	else
	{
		sprintf ( buffer, "\n* header checksum failed 0x%08x 0x%08x\n",
		 		  csum, cuteInfo.header_chksum );
		sysDebugWriteString (buffer);

		rt = E__FAIL;
	}

	//check info checksum
	if(rt == E__OK)
	{
		dOffset = imgData->dSize1 + imgData->dSize2 + FIELD_START; // start after the header
		done    = dOffset + cuteInfo.length;
		csum    = 0;

		for(; dOffset < done ; dOffset+=4)
		{
			dTemp  = *( (UINT32*)((UINT8*)psImage + dOffset));
			csum  += dTemp;
		}
		csum = -csum;

		if(csum == cuteInfo.chksum)
		{
			#ifdef DEBUG
				sprintf ( buffer, ". Info checksum passed\n");
				sysDebugWriteString (buffer);
			#endif
		}
		else
		{
			sprintf ( buffer, "\n* Info checksum failed 0x%08x 0x%08x\n",
			 		  csum, cuteInfo.chksum );
			sysDebugWriteString (buffer);
			rt = E__FAIL;
		}
	}

	//allocate buffer and copy info to local buffer
	if(rt == E__OK)
	{
		cuteInfo.data = k_malloc (cuteInfo.length);
		if(cuteInfo.data ==  NULL)
		{
			rt = E__FAIL;
			sysDebugWriteString ("\n* Cannot allocate buffer using k_malloc");
		}

		if(rt == E__OK)
		{
			dOffset = imgData->dSize1 + imgData->dSize2 + FIELD_START; // start after the header
			done    = dOffset + cuteInfo.length;
			count   = 0;

			//copy info to local buffer
			while ( dOffset < done )
			{
				*(cuteInfo.data+count) = *( (UINT8*)psImage + dOffset);
				dOffset++;
				count++;
			}
		}
	}

	return (rt);

} /* checkExtendedInfo () */


/************************************************************
 * readExtendedImage : searches for and identifies CUTE image
 * psImage	         : CUTE ROM image pointer
 * imgData			 : CUTE ROM image information structure
 * RETURNS			 : E__OK or error code
 */
static int readExtendedImage
	(
	    UINT32 const *psImage,		// CUTE ROM image pointer
		IMAGE_DATA  *imgData		// CUTE ROM image information structure
	)
{
	UINT32	dOffset = 0L;
	UINT32	dTemp;
	char   buffer[64];
	UINT32  rt = E__OK;
	UINT32  done;

	#ifdef DEBUG
		sysDebugWriteString (". Reading image file\n");
	#endif

	done    = 0;
	dOffset = 0L;
	while ( (dOffset < CUTE_SIZE) && (done == 0))
	{
		dTemp = *( (UINT32*)((UINT8*)psImage + dOffset));

		/* Find First stage signature */
		if(dOffset == 0)
		{
			/* Check first-stage signature */
			if ( (dTemp != CUTE_1ST_STAGE) && (dTemp != SIG_1ST_STAGE) )
			{
				sprintf (buffer, "\n* Invalid first-stage signature:"
						 "expected 0x%08lX or 0x%08lX, got 0x%08X\n",
					  	 SIG_1ST_STAGE, CUTE_1ST_STAGE, dTemp);
				sysDebugWriteString (buffer);
				rt = 1;
			}
			#ifdef DEBUG
				else
					sysDebugWriteString (". CCT BIT/CUTE\n");
			#endif
		}

		/* Find First stage size */
		if((dOffset == 8) && (rt == E__OK))
		{
			imgData->dSize1 = *( (UINT32*)((UINT8*)psImage + dOffset));
			dOffset = imgData->dSize1;
			#ifdef DEBUG
				sprintf (buffer, ". part-1 size: 0x%08X\n", imgData->dSize1);
				sysDebugWriteString (buffer);
			#endif
			continue;
		}

		/* Find second stage signature */
		if((imgData->dSize1 > 0) && (rt == E__OK) )
			if(dOffset == imgData->dSize1)
			{
				/* Check second-stage signature */
				if ( (dTemp != SIG_2ND_STAGE) )
				{
					sprintf (buffer, "\n* Invalid Second-stage signature:"
							 "expected 0x%08lX, got 0x%08X\n",
						  	 SIG_2ND_STAGE, dTemp);
					sysDebugWriteString (buffer);
					rt = 2;
				}
				#ifdef DEBUG
					else
						sysDebugWriteString (". Second stage\n");
				#endif
			}

		/* Find second stage size */
		if((imgData->dSize1 > 0)  && (rt == E__OK))
			if(dOffset == imgData->dSize1 + 8)
			{
				imgData->dSize2 = *( (UINT32*)((UINT8*)psImage + dOffset));
				done = 1;
				#ifdef DEBUG
					sprintf (buffer, ". part-2 size: 0x%08X\n", imgData->dSize2);
					sysDebugWriteString (buffer);
				#endif
			}

		dOffset += 4;
	}

	return (rt);

} /* readExtendedImage () */


/* ****************************************************************************
 *								 	 BIOS INFO				 			      *
 * ****************************************************************************/

/************************************************************
 * brdGetBiosInfo	 : searches for and identifies BIOS_INFO
 * psImage	         : Pointer to BIOS_INFO structure
 * RETURNS			 : E__OK or error code
 */
UINT32 brdGetBiosInfo
	(
		void* ptr
	)
{
#ifdef DEBUG
	char  buffer[64];
#endif
	PTR48 	 tPtr1;
	volatile UINT32 mapBIOSSpace=0;
	UINT32 	 mHandle;
	UINT32   rt = E__OK;

	// check if the BIOS INFO flag is set
	if(bGetBiosInfoPresent())
	{
		#ifdef DEBUG
			sprintf (buffer, "\n. BIOS INFO Offset 0x%X\n", (UINT32)(*((UINT32*)ptr)));
			sysDebugWriteString (buffer);
		#endif
		mHandle = dGetPhysPtr((UINT32)(*((UINT32*)ptr)), 1024, &tPtr1, (void*)&mapBIOSSpace);
		if(mHandle != NULL)
		{
			rt = checkBiosInfo((UINT32*)mapBIOSSpace);

			if(rt == E__OK)
				rt = parseBiosInfo();

			vFreePtr(mHandle);
		}
		else
		{
			sysDebugWriteString ("\n* Memory Allocation Failed\n");
			rt = E__FAIL;
		}

	}

	return rt;

} /* brdGetBiosInfo () */


/************************************************************
 * checkBiosInfo	 : verifies BIOS_INFO
 * psImage	         : Pointer to BIOS_INFO structure
 * RETURNS			 : E__OK or error code
 */
static int checkBiosInfo
	(
		UINT32 const *psImage		// pointer to BIOS_INFO
	)
{
	UINT32	dOffset = 0L;
	char   buffer[64];
	UINT32  rt = E__OK;
	UINT32  done;
	UINT32   *biosInfoPtr;
	UINT32  count;
	UINT32  csum;
	UINT32	dTemp;

	#ifdef DEBUG
		sysDebugWriteString (". Parsing BIOS Info\n");
	#endif

	memset(&BIOSInfo, 0x00, sizeof(BIOSInfo));
	biosInfoPtr = (UINT32*)&BIOSInfo;

	dOffset = 0;
	done    = FIELD_START; // read upto and including header checksum
	count   = 0;

	//copy header info to local structure
	while ( dOffset < done )
	{
		*((UINT8*)biosInfoPtr+count) = *( (UINT8*)psImage + dOffset);
		dOffset++;
		count  ++;
	}

	//check header checksum
	csum    = 0;
	dOffset = 0;
	done    = HEADER_CHECKSUM; // read upto but not including header checksum
	for(; dOffset < done ; dOffset+=4)
	{
		dTemp  = *( (UINT32*)((UINT8*)psImage + dOffset));
		csum  += dTemp;
	}
	csum = -csum;

	if(csum == BIOSInfo.header_chksum)
	{
		BIOSInfo.signature[15] = '\0';//make sure that the string is null terminated
		if(strcmp( (char *) BIOSInfo.signature, "CUTE_BIOS_INFO_") == 0)
		{
			#ifdef DEBUG
				sysDebugWriteString (". signature:    " );
				sysDebugWriteString (BIOSInfo.signature );
				sprintf(buffer, "\n. Struct_ver:   %08x", BIOSInfo.Struct_ver);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. BIOS_ver:     %08x", BIOSInfo.BIOS_ver);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. field_count:  %08x", BIOSInfo.field_count);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. length:       %08x", BIOSInfo.length);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. chksum:       %08x", BIOSInfo.chksum);
				sysDebugWriteString (buffer);
				sprintf(buffer, "\n. header_chksum:%08x\n", BIOSInfo.header_chksum);
				sysDebugWriteString (buffer);
			#endif
		}
		else
		{
			sprintf ( buffer, "\n* Wrong CUTE_BIOS_INFO signature %s\n", BIOSInfo.signature);
			sysDebugWriteString (buffer);
			rt = E__FAIL;
		}
	}
	else
	{
		sprintf ( buffer, "\n* header checksum failed 0x%08x 0x%08x\n",
		 		  csum, BIOSInfo.header_chksum );
		sysDebugWriteString (buffer);

		rt = E__FAIL;
	}

	//check info checksum
	if(rt == E__OK)
	{
		dOffset = FIELD_START; // start after the header
		done    = dOffset + BIOSInfo.length;
		csum    = 0;

		for(; dOffset < done ; dOffset+=4)
		{
			dTemp  = *( (UINT32*)((UINT8*)psImage + dOffset));
			csum  += dTemp;
		}
		csum = -csum;

		if(csum == BIOSInfo.chksum)
		{
			#ifdef DEBUG
				sprintf ( buffer, ". Info checksum passed\n");
				sysDebugWriteString (buffer);
			#endif
		}
		else
		{
			sprintf ( buffer, "\n* Info checksum failed 0x%08x 0x%08x\n",
			 		  csum, BIOSInfo.chksum );
			sysDebugWriteString (buffer);
			rt = E__FAIL;
		}
	}

	//allocate buffer and copy info to local buffer
	if(rt == E__OK)
	{
		BIOSInfo.data = k_malloc (BIOSInfo.length);
		if(BIOSInfo.data ==  NULL)
		{
			rt = E__FAIL;
			sysDebugWriteString ("\n* Cannot allocate buffer using k_malloc");
		}

		if(rt == E__OK)
		{
			dOffset = FIELD_START; // start after the header
			done    = dOffset + BIOSInfo.length;
			count   = 0;

			//copy info to local buffer
			while ( dOffset < done )
			{
				*(BIOSInfo.data+count) = *( (UINT8*)psImage + dOffset);
				dOffset++;
				count++;
			}
		}
	}

	return (rt);

} /* checkBiosInfo () */



/**********************************************************************
 * parseBiosInfo    : parses the BIOS info fields to make sure the information
 *                    is correct
 * RETURNS			: E__OK or error code
 */
static int parseBiosInfo
	(
		void
	)
{
	UINT32 fields = 0;
	UINT32 count  = 0;
	UINT32 dTemp  = 0;
	UINT32 rt     = E__FAIL;
	UINT32 i      = 0;
	UINT8  buff[16];
	char  buffer[64];

	while(count < BIOSInfo.length)
	{
		dTemp = *((UINT32*)(BIOSInfo.data + count));

		if(dTemp == 0x80000000)
		{
			fields++;
			count += 4;
			dTemp = *((UINT32*)(BIOSInfo.data + count));
			if(dTemp != 12)
			{
				sprintf(buffer, "\n* wrong length=%d", dTemp);
				sysDebugWriteString (buffer);
				rt = E__FAIL;
				break;
			}

			count += 4;
			for(i = 0; i < 16; i++)
				buff[i] = *(BIOSInfo.data + count + i);
			buff[15] = '\0';

			if(strcmp( (char *) buff, "FIELD-END" ) != 0)
			{
				rt = E__FAIL;
				sprintf(buffer, "\n* Fields End Failed! found=%s", buff);
				sysDebugWriteString (buffer);
			}
			else
			{
				if(fields != BIOSInfo.field_count)
				{
					rt = E__FAIL;
					sprintf(buffer, "\n* Fields=%d BIOSInfo.field_count=%d", fields, BIOSInfo.field_count);
					sysDebugWriteString (buffer);
				}
				else
					rt = E__OK;
			}
			break;
		}

		fields++;
		count += 4;
		dTemp  = *((UINT32*)(BIOSInfo.data + count));
		count += dTemp + 4;
	}

	if(rt == E__OK)
	{
		BIOSInfo.present = 1;
		#ifdef DEBUG
			sysDebugWriteString (". Info Verified\n");
		#endif
	}
	else
		sysDebugWriteString ("\n* Info NOT Verified\n");

	return rt;

}//parseBiosInfo()


#if 0
/*********************************************************************
 * findBiosField    : Searches for a specific filed in the BIOS info buffer
 * type	            : UINT32 data indicating filed type to locate
 * RETURNS          : Field offset or E__FAIL
 */
static UINT32 findBiosField
	(
		UINT32 type		// field type
	)
{
	UINT32 count  = 0;
	UINT32 dTemp  = 0;
	UINT32 rt     = BIOS_FIELD_NOT_PRSENT;

	if(BIOSInfo.present != 1)
		return BIOS_INFO_NOT_PRSENT;

	while(count < BIOSInfo.length)
	{
		dTemp = *((UINT32*)(BIOSInfo.data + count));

		if(dTemp == type)
		{
			rt = E__OK;
			break;
		}

		count += 4;
		dTemp  = *((UINT32*)(BIOSInfo.data + count));
		count += dTemp + 4;
	}

	return rt;
} /* findBiosField() */
#endif


/*********************************************************************
 * getBiosField     : Searches for a specific filed in the BIOS info buffer,
 * 					  then copies the field data to supplied buffer
 * type	            : UINT32 data indicating filed type to locate
 * data				: buffer to hold field data must be at least big
 * 					  enough to hold field data
 * size				: size of data buffer
 * RETURNS          : Field offset or E__FAIL
 */

static UINT32 getBiosField
	(
		UINT32 type,		// field type
		UINT8 *data,		// buffer to hold field data
		UINT32 size			// size of the buffer
	)
{
	UINT32 length = 0;
	UINT32 count  = 0;
	UINT32 dTemp  = 0;
	UINT32 rt     = BIOS_FIELD_NOT_PRSENT;
	UINT32 i      = 0;
#ifdef DEBUG
	char  buffer[64];
#endif

	if(BIOSInfo.present != 1)
	{
		sysDebugWriteString ("BIOS INFO not present\n");
		return BIOS_INFO_NOT_PRSENT;
	}
	while(count < BIOSInfo.length)
	{
		dTemp = *((UINT32*)(BIOSInfo.data + count));

		if(dTemp == type)
		{
			count += 4;
			length = *((UINT32*)(BIOSInfo.data + count));
			if(length > size)
			{
				#ifdef DEBUG
					sprintf(buffer, "\n* data buffer too small=%d, length=%d", size, length);
					sysDebugWriteString (buffer);
				#endif
				rt = BIOS_INFO_BUFFER_TOO_SMALL;
				break;
			}

			//copy data here
			count += 4;
			for(i = 0; i < length; i++)
				data[i] = *(BIOSInfo.data + count + i);

			rt = E__OK;
			break;
		}

		count += 4;
		dTemp  = *((UINT32*)(BIOSInfo.data + count));
		count += dTemp + 4;
	}

	return rt;
} /* getBiosField() */



/* *************************************************************************** *
 *								 	 SERVICES				 			       *
 * *************************************************************************** */




/*********************************************************************
 * brdGetForceDebugOn   : Checks if EFIELD_FORCE_DEBUG_ON Field is present
 * ptr	            	: NULL
 * RETURNS          	: E__OK or E__FAIL
 */
UINT32 brdGetForceDebugOn (void* ptr)
{
	UINT32	rt;

   	rt = findExtendedField(EFIELD_FORCE_DEBUG_ON);

	return rt;

} /* brdGetForceDebugOn () */



/*********************************************************************
 * brdGetForceDebugOff  : Checks if EFIELD_FORCE_DEBUG_OFF Field is present
 * ptr	            	: NULL
 * RETURNS          	: E__OK or E__FAIL
 */
UINT32 brdGetForceDebugOff (void* ptr)
{
	UINT32	rt;

   	rt = findExtendedField( EFIELD_FORCE_DEBUG_OFF);

	return rt;

} /* brdGetForceDebugOff () */








/*********************************************************************
 * brdGetSthTestList    : Board service to retrieve temperature grade
 * 						  string
 * ptr	            	: pointer to Temp_String_Info
 * RETURNS          	: E__OK or E__FAIL
 */
UINT32 brdGetSthTestList (void* ptr)
{
	UINT32	rt;

   	rt = getExtendedField( EFIELD_OVERRIDE_STH_TEST_LIST,	((STH_Override_List*)ptr)->buffer,
                           ((STH_Override_List*)ptr)->size );
	return rt;

} /* brdGetTempGradeString () */




/*********************************************************************
 * brdGetBIOSVersion    : Board service to retrieve BIOS Version
 * 						  string
 * ptr	            	: pointer to buffer to contain the string
 * 						  buffer must be at least 8 bytes
 * RETURNS          	: E__OK
 */
UINT32 brdGetBIOSVersion (void* ptr)
{

   sprintf((char *)ptr, "%c%X.%02X",
		   ((BIOSInfo.BIOS_ver >> 16)& 0x000000ff),
		   ((BIOSInfo.BIOS_ver >> 8)& 0x000000ff),
		   ( BIOSInfo.BIOS_ver & 0x000000ff)
		   );

	return E__OK;

} /* brdGetBIOSVersion () */


/*********************************************************************
 * brdGetNameString		: Board service to retrieve Board name string
 * ptr	            	: pointer to Temp_String_Info
 * RETURNS          	: E__OK or E__FAIL
 */
UINT32 brdGetNameString (void* ptr)
{
	UINT32	rt;

   	rt = getBiosField( EFIELD_BOARD_NAME, ((Temp_String_Info*)ptr)->buffer,
                      ((Temp_String_Info*)ptr)->size );

	((Temp_String_Info*)ptr)->buffer[((Temp_String_Info*)ptr)->size-1] = '\0';

	return rt;

} /* brdGetNameString () */

/*********************************************************************
 * brdGetTempGradeString: Board service to retrieve temperature grade
 * 						  string
 * ptr	            	: pointer to Temp_String_Info
 * RETURNS          	: E__OK or E__FAIL
 */
UINT32 brdGetTempGradeString (void* ptr)
{
	UINT32	rt;

   	rt = getExtendedField( EFIELD_TEMPERATURE_GRADE_STRING,	((Temp_String_Info*)ptr)->buffer,
                           ((Temp_String_Info*)ptr)->size );

	((Temp_String_Info*)ptr)->buffer[((Temp_String_Info*)ptr)->size-1] = '\0';

	return rt;

} /* brdGetTempGradeString () */


/*********************************************************************
 * brdGetSlotId: Board service to retrieve slot ID
 * 						  
 * ptr	            	: pointer to slot ID
 * RETURNS          	: E__OK or E__FAIL
 */
UINT32 brdGetSlotId( void* ptr )
{
	UINT8 amcIpmbAddrs;
	UINT8 amcSlotId = 0;
	
	
	amcIpmbAddrs = bGetAMCIpmbAddrs();

#ifdef DEBUG	
	sysDebugPrintf("%s: my_slot: 0x%04x amcSlotId: 0x%02x\n", __FUNCTION__, my_slot, amcSlotId);
#endif
	
	if ( amcIpmbAddrs > 0 )
	{
		amcSlotId = ((amcIpmbAddrs - 0x72)/2)+1;

#ifdef DEBUG		
		sysDebugPrintf("%s: amcSlotId: 0x%02x\n", __FUNCTION__, amcSlotId);
#endif
		
		*((UINT16*)ptr) = (UINT16) amcSlotId;
		
		return E__OK;
	}
	else
	{
		*((UINT16*)ptr) = 0xffff;
				
		return E__FAIL;
	}
}

