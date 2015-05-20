/************************************************************************
 *                                                                      *
 *      Copyright 2009 Concurrent Technologies, all rights reserved.    *
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
 *																		*
 ************************************************************************/
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>

#include <bit/board_service.h>
#include <bit/delay.h>

#include "cctboard.h"


/********************************************************************
 * brdGPIOTest
 *
 * This test will test the board specific GPIO lines.
 */
 TEST_INTERFACE (brdGPIOTest, "GPIO Test")
{
	UINT32 rt = E__OK;
	UINT8  bTmp;
	char	achBuffer[80];	/* text formatting buffer */


	// GPIO - Control Register Init
	vIoWriteReg (GPIO_DIR_CTL, REG_8, 0xff);
	vDelay(5);

	// GPO - Register
	vIoWriteReg(GPIO_CTL_STATUS,REG_8,0x00);
	vDelay(5);

	bTmp = dIoReadReg (GPI_STATUS, REG_8);

	if (bTmp != 0x00)
	{
			sprintf(achBuffer,"GPIO Reg 310h : 0x%x  Expected  :  0x00 \n",bTmp);
			vConsoleWrite(achBuffer);
			return E__FAIL;
	}

	vIoWriteReg(GPIO_CTL_STATUS,REG_8,0xff);
	vDelay(5);

	bTmp = dIoReadReg (GPI_STATUS, REG_8);


	if (bTmp != 0xff)
	{
			sprintf(achBuffer,"GPIO Reg 310h : 0x%x  Expected  :  0xff \n",bTmp);
			vConsoleWrite(achBuffer);
			return E__FAIL;
	}

	return rt;
}
