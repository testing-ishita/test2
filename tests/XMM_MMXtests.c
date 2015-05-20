
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

/* XMM_MMXtests.c - XMM and MMX related tests.
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/XMM_MMXtests.c,v 1.2 2014-04-22 16:36:32 swilson Exp $
 * $Log: XMM_MMXtests.c,v $
 * Revision 1.2  2014-04-22 16:36:32  swilson
 * NOTE: Ensure that the test interface tag only appears in function declarations, not comments. This tag was also designed to allow search+sort operations that output a list of functions and test names; if teh tag is used in comments then we get surious lines included in this list.
 *
 * Revision 1.1  2013-09-04 07:46:46  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.4  2010/06/29 10:12:33  hchalla
 * Code Cleanup, Added headers and comments.
 *
 * Revision 1.3  2009/05/29 14:06:45  hmuneer
 * Error Codes Fixed
 *
 * Revision 1.2  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.1  2009/05/18 09:16:39  hmuneer
 * no message
 *
 * Revision 1.0  Feb 9 2009
 *
 *
 */


/* includes */
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>	
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <private/cpu.h>

/* defines */
//#define DEBUG

#define MMX_REG_ERROR		(E__BIT + 0x01)	
#define MMX_ADD_ERROR		(E__BIT + 0x02)	
#define MMX_SUB_ERROR		(E__BIT + 0x03)	
#define MMX_MUL_ERROR		(E__BIT + 0x04)	

#define XMM_REG_ERROR		(E__BIT + 0x01)	
#define XMM_ADD_ERROR		(E__BIT + 0x02)		
#define XMM_SUB_ERROR		(E__BIT + 0x03)	
#define XMM_MUL_ERROR		(E__BIT + 0x04)	
	
#define XMMX_ADD			0
#define XMMX_SUB			1
#define XMMX_MUL			2


extern void	mmxTest(UINT32 operation, UINT64 *result, UINT64 op1, UINT64 op2);
extern void	mmxRead(UINT32 reg, UINT64 *data);
extern void	mmxWrite(UINT32 reg, UINT64 *data);

extern void	xmmTest(UINT32 operation, UINT8 *result, UINT8 *op1, UINT8 *op2);
extern void	xmmRead(UINT32 reg,  UINT8 *data );
extern void	xmmWrite(UINT32 reg, UINT8 *data );


/*****************************************************************************\
 *
 *  TITLE:  wReg64BitTest ()
 *
 *  ABSTRACT:  this function performs a bit-wise test on the specified
 * 64-bit MMX register. A marching 1/0 pattern is used
 *
 * 	RETURNS: E_OK or E__... error code.
 *
\*****************************************************************************/
UINT32 wReg64BitTest(UINT32	reg)
{
	UINT64  Backup;
	UINT64	bPattern;		/* current test bit-pattern */
	UINT32	bBitNum;		/* register bit number */
	UINT64	bTemp;			/* like it says! */
	#ifdef DEBUG
		char	achBuffer[128];	/* text formatting buffer */
	#endif

	mmxRead(reg, &Backup);

	for (bBitNum = 0; bBitNum < 64; bBitNum++)
	{
		bPattern = (1 << bBitNum) ;		/* Marching '1' */
		mmxWrite(reg, &bPattern);
		mmxRead(reg, &bTemp);	

		if (bTemp != bPattern)
		{	
			mmxWrite(reg, &Backup);			
			
			#ifdef DEBUG
				sprintf (achBuffer, "\nData error (1)[Reg:0x%x, bit:0x%x]: wrote 0x[%8x%8x], read 0x[%8x%8x]\n\n",
						 reg, bBitNum, (UINT32)(bPattern>>32), (UINT32)(bPattern),
						 (UINT32)(bTemp>>32), (UINT32)(bTemp));
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}

		bPattern = ~(1 << bBitNum) ;	/* Marching '0' */
		mmxWrite(reg, &bPattern);
		mmxRead(reg, &bTemp);	

		if (bTemp != bPattern)
		{		
			mmxWrite(reg, &Backup);		
			
			#ifdef DEBUG
				sprintf (achBuffer, "\nData error (0)[Reg:0x%x, bit:0x%x]: wrote 0x[%8x%8x], read 0x[%8x%8x]\n\n",
					 	 reg, bBitNum, (UINT32)(bPattern>>32), (UINT32)(bPattern),
						 (UINT32)(bTemp>>32), (UINT32)(bTemp));
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}
	}
	
	mmxWrite(reg, &Backup);	
	return (E__OK);

} /* wReg64BitTest () */


/*****************************************************************************\
 *
 *  TITLE:  wReg128BitTest ()
 *
 *  ABSTRACT: This function performs a bit-wise test on the specified
 * 128-bit XMM register. A marching 1/0 pattern is used
 *
 * 	RETURNS: E_OK or E__... error code.
 *
\*****************************************************************************/
UINT32 wReg128BitTest(UINT32	reg)
{
	UINT16  i, bBitNum;
	UINT8   Backup[16];
	UINT8	bPattern[16];		/* current test bit-pattern */
	UINT8	bTemp[16];			/* like it says! */
	#ifdef DEBUG
		char	achBuffer[128];	/* text formatting buffer */
	#endif

	xmmRead(reg, Backup);

	for (bBitNum = 0; bBitNum < 8; bBitNum++)
	{
		for(i = 0; i<16; i++)
			bPattern[i] = (1 << bBitNum) ;		/* Marching '1' */

		xmmWrite(reg, bPattern);
		xmmRead(reg, bTemp);	

		for(i = 0; i< 16; i++)
		{
			if (bTemp[i] != bPattern[i])
			{	
				xmmWrite(reg, Backup);			
				
				#ifdef DEBUG
					sprintf ( achBuffer, "\nData error (1)[Reg:0x%x, byte:0x%x, bit:0x%x] 0x[", reg, i, bBitNum);		
					vConsoleWrite (achBuffer);
					for(j = 0; j< 16; j++)
					{
						sprintf ( achBuffer, "%2x", bTemp[j]);
						vConsoleWrite (achBuffer);
					}		
					vConsoleWrite ("]\n");
				#endif
				return E__FAIL;
			}
		}

		for(i = 0; i<16; i++)
			bPattern[i] = ~(1 << bBitNum) ;		/* Marching '0' */
		xmmWrite(reg, bPattern);
		xmmRead(reg, bTemp);	

		for(i = 0; i< 16; i++)
		{
			if (bTemp[i] != bPattern[i])
			{		
				xmmWrite(reg, Backup);		
			
				#ifdef DEBUG
					sprintf ( achBuffer, "\nData error (0)[Reg:0x%x, byte:0x%x, bit:0x%x] 0x[", reg, i, bBitNum);
					vConsoleWrite (achBuffer);
					for(j = 0; j< 16; j++)
					{
						sprintf ( achBuffer, "%2x", bTemp[j]);
						vConsoleWrite (achBuffer);
					}		
					vConsoleWrite ("]\n");
				#endif
				return E__FAIL;
			}
		}
	}
	
	xmmWrite(reg, Backup);	
	return (E__OK);

} /* wReg128BitTest () */



/*****************************************************************************\
 *
 *  TITLE:  MMXTest
 *
 *  ABSTRACT: Entry point for MMXTest.
 *
 * 	RETURNS: E_OK or E__... error code.
 *
\*****************************************************************************/

TEST_INTERFACE( MMXTest, "Intel MMX Test" )
{
	UINT32 i, rt = E__OK;
	UINT64 result;
#ifdef DEBUG
	char   achBuffer[128];
#endif

	for(i = 0; (i < 8) && (rt == E__OK); i++)
	{
		rt = wReg64BitTest(i);
		if( rt == E__OK)
		{
			#ifdef DEBUG
				sprintf ( achBuffer, "reg %d, pass\n ", i);
				vConsoleWrite (achBuffer);
			#endif
		}
		else
		{
			#ifdef DEBUG
				sprintf ( achBuffer, "reg %d, fail\n ", i);
				vConsoleWrite (achBuffer);
			#endif

			rt = MMX_REG_ERROR;
		}
	}

	if(rt == E__OK)
	{
		mmxTest(XMMX_ADD, &result, 0x1111111111111111LL, 0x3333333333333333LL);	
		if(result != 0x4444444444444444LL)
			rt = MMX_ADD_ERROR;
		#ifdef DEBUG
			sprintf( achBuffer, "Result 0x[%8x%8x]\n ", (UINT32)(result>>32), (UINT32)(result));
			vConsoleWrite (achBuffer);
		#endif
	}	

	if(rt == E__OK)
	{
		mmxTest(XMMX_SUB, &result, 0x1111111111111111LL, 0x3333333333333333LL);	
		if(result != 0x2222222222222222LL)
			rt = MMX_SUB_ERROR;
		#ifdef DEBUG
			sprintf( achBuffer, "Result 0x[%8x%8x]\n ", (UINT32)(result>>32), (UINT32)(result));
			vConsoleWrite (achBuffer);
		#endif
	}

	if(rt == E__OK)
	{
		mmxTest(XMMX_MUL, &result, 0x0002000200020002LL, 0x0004000400040004LL);	
		if(result != 0x0008000800080008LL)
			rt = MMX_MUL_ERROR;
		#ifdef DEBUG
			sprintf( achBuffer, "Result 0x[%8x%8x]\n ", (UINT32)(result>>32), (UINT32)(result));
			vConsoleWrite (achBuffer);
		#endif
	}

	return rt;
}

/*****************************************************************************\
 *
 *  TITLE:  XMMTest
 *
 *  ABSTRACT: Entry point for XMMTest.
 *
 * 	RETURNS: E_OK or E__... error code.
 *
\*****************************************************************************/
TEST_INTERFACE( XMMTest, "Intel XMM Test" )
{
	UINT32 i, j, rt = E__OK;
#ifdef DEBUG
	char   achBuffer[128];
#endif
	UINT8  buff1[16];
	UINT8  buff2[16];
	UINT8  res[16];

	for(i = 0; (i < 8) && (rt == E__OK); i++)
	{		
		rt = wReg128BitTest(i);		

		if( rt == E__OK)
		{
			#ifdef DEBUG
				sprintf ( achBuffer, "reg %d, pass\n ", i);
				vConsoleWrite (achBuffer);
			#endif
		}
		else
		{
			#ifdef DEBUG
				sprintf ( achBuffer, "reg %d, fail\n ", i);
				vConsoleWrite (achBuffer);
			#endif

			rt = XMM_REG_ERROR;
		}	
	}


	if(rt == E__OK)
	{
		for(j = 0; j< 16; j++)
		{
			buff1[j] =1;
			buff2[j] =1;
		}
		xmmTest(XMMX_ADD, res, buff1, buff2);

		for(j = 0; j< 16; j++)
		{
			if(res[j] != 0x02)
				rt = XMM_ADD_ERROR;
		}
			
		#ifdef DEBUG
			vConsoleWrite("\nResult 0x[");		
			for(j = 0; j< 16; j++)
			{
				sprintf( achBuffer, "%2x", res[j]);
				vConsoleWrite(achBuffer);
			}		
			vConsoleWrite ("]\n");	
		#endif
	}	
	

	if(rt == E__OK)
	{
		for(j = 0; j< 16; j++)
		{
			buff1[j] =1;
			buff2[j] =3;
		}
		xmmTest(XMMX_SUB, res, buff1, buff2);

		for(j = 0; j< 16; j++)
		{
			if(res[j] != 0x02)
				rt = XMM_SUB_ERROR;
		}
			
		#ifdef DEBUG
			vConsoleWrite("\nResult 0x[");		
			for(j = 0; j< 16; j++)
			{
				sprintf( achBuffer, "%2x", res[j]);
				vConsoleWrite(achBuffer);
			}		
			vConsoleWrite ("]\n");	
		#endif
	}	

	

	if(rt == E__OK)
	{
		memset(buff1,0x00,16);
		memset(buff2,0x00,16);
		for(j = 0; j< 16; j+=2)
		{
			buff1[j] =2;
			buff2[j] =2;
		}
		xmmTest(XMMX_MUL, res, buff1, buff2);

		for(j = 0; j< 16; j+=2)
		{
			if(res[j] != 0x04)
				rt = XMM_MUL_ERROR;
		}
			
		#ifdef DEBUG
			vConsoleWrite("\nResult 0x[");		
			for(j = 0; j< 16; j++)
			{
				sprintf( achBuffer, "%2x", res[j]);
				vConsoleWrite(achBuffer);
			}		
			vConsoleWrite ("]\n");	
		#endif
	}	
	

	return rt;
}
