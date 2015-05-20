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

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>

#include <bit/bit.h>
#include <bit/console.h>
#include <bit/board_service.h>
#include <bit/io.h>
#include <bit/delay.h>

#include "cctboard.h"

#define E__ERR_0    (E__BIT+1)
#define E__ERR_1    (E__BIT+2)

#if 0
GPIO_INFO  localGpio = {
								0x213,
								0x4,
								0x1
						};

/*****************************************************************************
 * brdGetTempinfo: returns the TEMPERATURE_INFO global data structure
 * RETURNS: TEMPERATURE_INFO* */
UINT32 brdGetGPIOInfo(void *ptr)
{
	*((GPIO_INFO**)ptr) = &localGpio;

	return E__OK;
}
#endif


/********************************************************************
 * brdGPIOTest
 *
 * This test will test the board specific GPIO lines.
 ********************************************************************/
TEST_INTERFACE (brdGPIOTest, "Board GPIO Test")
{
	UINT32	testStatus;
	UINT8	bTmp;
	char	achErrTxt [80];

	testStatus = E__OK;
	vIoWriteReg (GPIO_REG_1, REG_8, 0);				// set GPIO output low
	vDelay(5);

	bTmp = dIoReadReg (GPIO_REG_1, REG_8);
	if ((bTmp & (GPIO_INPUT_1 + GPIO_INPUT_2 + GPIO_INPUT_3)) != 0)
	{
		sprintf (achErrTxt, "Expected all GPIO inputs LO but read %02x\n", bTmp);
		vConsoleWrite (achErrTxt);
		testStatus =  E__ERR_0;
	}

	if (testStatus == E__OK)
	{
		vIoWriteReg (GPIO_REG_1, REG_8, GPIO_OUTPUT);				// set GPIO output high
		vDelay(5);

		bTmp = dIoReadReg (GPIO_REG_1, REG_8);
		if ((bTmp & (GPIO_INPUT_1 + GPIO_INPUT_2 + GPIO_INPUT_3)) != (GPIO_INPUT_1 + GPIO_INPUT_2 + GPIO_INPUT_3))
		{
			sprintf (achErrTxt, "Expected all GPIO inputs HI but read %02x\n", bTmp);
			vConsoleWrite (achErrTxt);
			testStatus =  E__ERR_1;
		}
	}

	return testStatus;
}
