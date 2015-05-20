
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

/* superio.c - Super I/O related tests
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/fpuTests.c,v 1.1 2013-09-04 07:46:43 chippisley Exp $
 * $Log: fpuTests.c,v $
 * Revision 1.1  2013-09-04 07:46:43  chippisley
 * Import files into new source repository.
 *
 * Revision 1.10  2011/02/28 11:59:20  hmuneer
 * CA01A161, TA01A121
 *
 * Revision 1.9  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.8  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.7  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.6  2010/03/31 16:27:10  hmuneer
 * no message
 *
 * Revision 1.5  2009/06/10 13:56:43  swilson
 * Fix MF error code modifier.
 *
 * Revision 1.4  2009/06/09 16:04:44  swilson
 * Re-structured the FPU Exception test to add support for internal MF exception, Test
 *  now gets board-specific details on which exception mechanismss are supported. Added
 *  more error codes to cover the additonal cases. Cleaned-up and commented existing
 *  code in-line with coding standards,
 *
 * Revision 1.3  2009/05/29 14:05:10  hmuneer
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
#include <bit/board_service.h>
#include <bit/interrupt.h>

#include <private/debug.h>
#include <private/cpu.h>

/* defines */

//#define DEBUG

#define FPU_ADD				0
#define FPU_SUB				1
#define FPU_MUL				2
#define FPU_DIV				3

	/* fpuSetup () operations */

#define FPU_READ_CW			0
#define FPU_WRITE_CW		1
#define FPU_INIT			2

	/* Error codes */
#define E__NOT_SUPPORTED		(E__BIT + 0x00000000)

#define E__FPU_ADD_ERROR		(E__BIT + 0x00000001)	
#define E__FPU_SUB_ERROR		(E__BIT + 0x00000002)	
#define E__FPU_MUL_ERROR		(E__BIT + 0x00000003)	
#define E__FPU_DIV_ERROR		(E__BIT + 0x00000004)	
#define E__FPU_FLG_ERROR		(E__BIT + 0x00000005)	

#define E__FPU_EXP_ERROR		(E__BIT + 0x00000006)	
#define E__FPU_INT_ERROR		(E__BIT + 0x00000007)	

#define E__FPU_FERR				(E__BIT + 0x00000100)	
#define E__FPU_MF				(E__BIT + 0x00000200)	

	/* FPU exception/interrupt defines */

#define FPU_MF				16				/* CPU H/W exception 16 */
#define FPU_FERR			13				/* FERR interrupts on APIC pin 13 */

/* typedefs */

/* constants */

/* locals */

static  int	fpuFlag;

/* globals */

/* externals */

extern int cpuSetNe (int iNewState);
extern void	fpuTest(UINT32 operation, volatile float *result, volatile float *op1, volatile float *op2);
extern void	fpuExpTest(UINT32 operation, float *result, float *op1, float *op2);
extern void	fpuSetup(UINT32 operation, UINT32 *op);
extern void	fpuGetFlags( UINT16 *flags);
extern void fpuClearFlags(UINT32 op);
/* forward declarations */

static void fpuHandler (void);
static int setupInt(int iPin, int iVector);
static int removeInt(int iPin, int iVector);
static UINT32 doFpuExceptionTest (int iPin, int iVector);



/*****************************************************************************
 * FPUTest: perform basic FPU math operations
 *
 * RETURNS: E__OK, or an E__BIT error code
 */

TEST_INTERFACE( FPUTest, "FPU Basic Test" )
{
	UINT32	rt = E__OK, op, op1;
	UINT16  flags;
	volatile float	a, b, r;
	//char  achBuffer[80];


	//set fpu control word to default, so exceptions are not generated
	//only flags are set

	fpuSetup(0, &op1);
	op  = op1;
	op |= 0x037f;
	fpuSetup(1, &op);

	a = 1.5;
	b = 2.5;
	fpuTest(FPU_ADD, &r, &a, &b);

	if((r >= 3.9) && (r <= 4.1))
		rt = E__OK;
	else
	{
		//sprintf(achBuffer,"\n E__FPU_ADD_ERROR=%f\n", r );
		//sysDebugWriteString (achBuffer);
		rt = E__FPU_ADD_ERROR;
	}

	//if(rt == E__OK)
	{
		a = 1.5;
		b = 2.5;
		fpuTest(FPU_SUB, &r, &a, &b);

		if( (r >= 0.9) && (r <= 1.1))
			rt = E__OK;
		else
		{
			//sprintf(achBuffer,"\n E__FPU_SUB_ERROR=%f\n", r );
			//sysDebugWriteString (achBuffer);
			rt = E__FPU_SUB_ERROR;	
	}
	}

	//if(rt == E__OK)
	{
		a = 1.5;
		b = 2.5;
		fpuTest(FPU_MUL, &r, &a, &b);

		if((r >= 3.70) &&  (r <= 3.80) )
			rt = E__OK;
		else
		{
			//sprintf(achBuffer,"\n E__FPU_MUL_ERROR=%f\n", r );
			//sysDebugWriteString (achBuffer);
			rt = E__FPU_MUL_ERROR;		
	}
	}

	//if(rt == E__OK)
	{
		a = 1.5;
		b = 3.75;
		fpuTest(FPU_DIV, &r, &a, &b);

		if((r >= 2.4) && (r <= 2.6) )
			rt = E__OK;
		else
		{
			//sprintf(achBuffer,"\n E__FPU_DIV_ERROR=%f\n", r );
			//sysDebugWriteString (achBuffer);
			rt = E__FPU_DIV_ERROR;		
	}
	}

	//if(rt == E__OK)
	{
		a = 0;
		b = 3.0;
		fpuTest(FPU_DIV, &r, &a, &b);

		//verify the devide by zero flag 		
		flags = 0;
		fpuGetFlags( &flags);		

		if((flags & 0x0004)  == 0x0004)
			rt = E__OK;
		else
		{
			//sprintf(achBuffer,"\n E__FPU_FLG_ERROR=%x\n", flags );
			//sysDebugWriteString (achBuffer);

			rt = E__FPU_FLG_ERROR;
	}
	}
	
	//restore control word
	fpuSetup(1, &op1);

	return rt;
}


/*****************************************************************************
 * fpuHandler: exception handler for FPU test
 *
 * RETURNS: E__OK or E__FPU_INT_ERROR
 */

static void fpuHandler (void)
{
	fpuFlag = 1;	
	fpuClearFlags(0);
	
} /* fpuHandler () */


/*****************************************************************************
 * removeInt: install interrupt handler and enable interrupt
 *
 * RETURNS: E__OK or E__FPU_INT_ERROR
 */

static int setupInt 
(
	int		iPin,		/* pin number, -1 if not a pin */
	int		iVector		/* vector number if not a pin */
)
{
	if (iPin != -1)
	{
		iVector = sysPinToVector (iPin, SYS_IOAPIC0);

		if (iVector == -1)
			return (E__FPU_INT_ERROR);
	}

	sysInstallUserHandler (iVector, fpuHandler);	

	if (iPin != -1)
	{
		if (sysUnmaskPin (iPin, SYS_IOAPIC0) == -1)
			return (E__FPU_INT_ERROR);

	}

	return E__OK;

} /* setupInt () */


/*****************************************************************************
 * removeInt: mask interrupt and uninstall handler
 *
 * RETURNS: E__OK or E__FPU_INT_ERROR
 */

static int removeInt
(
	int		iPin,		/* pin number, -1 if not a pin */
	int		iVector		/* vector number if not a pin */
)
{
	if (iPin != -1)
	{
		iVector = sysPinToVector (iPin, SYS_IOAPIC0);

		if (iVector == -1)
			return (E__FPU_INT_ERROR);

		if (sysMaskPin (iPin, SYS_IOAPIC0) == -1)
			return (E__FPU_INT_ERROR);

	}

	sysInstallUserHandler (iVector, 0);		/* uninstall handler */

	return E__OK;

} /* removeInt () */


/*****************************************************************************
 * FPUExpTest: check FPU exception handling
 *
 * RETURNS: E__OK, or an E__BIT error code
 */

TEST_INTERFACE( FPUExpTest, "FPU Exception Test" )
{
	UINT32	dExceptnMap;
	UINT32	rt;
	int		oldNe;

			
	rt = E__OK;


	/* Determine which exception handling methods are supported by the board.
	 * Error if none of the methods supported in this test.
	 */

	board_service(SERVICE__BRD_GET_FPU_EXCEPTION_MAP, NULL, &dExceptnMap);

	if ( (dExceptnMap & (FPU_EXCEPTION_MF | FPU_EXCEPTION_FERR)) == 0 )
		rt = E__NOT_SUPPORTED;


	/*
     * Test external exceptions via FERR?
     */

	if (dExceptnMap & FPU_EXCEPTION_FERR)
	{
		fpuFlag = 0;

		oldNe = cpuSetNe (0);		/* select external (FERR) reporting */
		
		rt = doFpuExceptionTest (FPU_FERR, 0);

		if (rt != E__OK)
			rt |= E__FPU_FERR;

		cpuSetNe (oldNe);
	}

	/*
     * Test internal exceptions via MF?
     */

	if (dExceptnMap & FPU_EXCEPTION_MF)
	{
		fpuFlag = 0;

		oldNe = cpuSetNe (1);		/* select internal (MF) reporting */
		
		rt = doFpuExceptionTest (-1, FPU_MF);

		if (rt != E__OK)
			rt |= E__FPU_MF;

		cpuSetNe (oldNe);
	}

	return rt;

} /* FPUExpTest () */



static UINT32 doFpuExceptionTest 
(
	int		iPin,		/* pin number, -1 if not a pin */
	int		iVector		/* vector number if not a pin */
)
{
	UINT32	rt;
	UINT32	op;
	UINT32	op1;
	float	a;
	float	b;
	float	r;	


	rt = E__OK;


	/* Set FPU control word to generate exceptions */

	op = 0x0340;			/* enable all FPU exceptions */
	fpuSetup (1, &op);

	vIoWriteReg (0xf0, REG_8, 0 );


	/* Install the exception handler and geberate an exception */

	if (setupInt (iPin, iVector) == E__OK)
	{
		a = 0;
		b = 3.0;
		fpuExpTest (3, &r, &a, &b);			

		vDelay (10);
		
		if ((fpuFlag != 1) )
			rt = E__FPU_EXP_ERROR;	
	}
	else
		rt = E__FPU_INT_ERROR;


	/* Reset FPU */

	fpuSetup (2, &op1);
	vIoWriteReg (0xf0, REG_8, 0 );	


	/* Uninstall the handler */

	if (removeInt (iPin, iVector) != E__OK)
		rt = E__FPU_INT_ERROR;

	return (rt);

} /* doFpuExceptionTest () */


