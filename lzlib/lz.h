/*******************************************************************************
*
* Filename:	 lz.h
*
* Description:	LZ77 Compress/Decompress functions taken from the free
*               Basic Compression Library by Marcus Geelnard, see below.
*               
*               CCT Modifications:
*               1. Function return value changed to unsigned int.
*               2. Added LZ_UncompressImage() function.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 07:43:05 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lzlib/lz.h,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

/*************************************************************************
* Name:        lz.h
* Author:      Marcus Geelnard
* Description: LZ77 coder/decoder interface.
* Reentrant:   Yes
*-------------------------------------------------------------------------
* Copyright (c) 2003-2006 Marcus Geelnard
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would
*    be appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
*    distribution.
*
* Marcus Geelnard
* marcus.geelnard at home.se
*************************************************************************/

#ifndef _lz_h_
#define _lz_h_

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
* Function prototypes
*************************************************************************/

unsigned int LZ_Compress( unsigned char *in, unsigned char *out,
                 unsigned int insize );
unsigned int LZ_CompressFast( unsigned char *in, unsigned char *out,
                     unsigned int insize, unsigned int *work );
void LZ_Uncompress( unsigned char *in, unsigned char *out,
                    unsigned int insize );

void LZ_UncompressImage( unsigned int startAddress );

#ifdef __cplusplus
}
#endif

#endif /* _lz_h_ */
