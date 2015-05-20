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
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>

#include <bit/board_service.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>
//#define DEBUG
#define GPIO_LOOPBACK_ERR0		(E__BIT + 0x01)
#define GPIO_LOOPBACK_ERR1		(E__BIT + 0x02)
#define GPIO_LOOPBACK_ERR2		(E__BIT + 0x03)

#define  DIR_0			0x08
#define  GPIO_0			0x01

GPIO_INFO  localGpio = {
								0x213,
								0x3,
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

/*****************************************************************************
 * brduRTMGPIOTest:
 * */
UINT32 brduRTMGPIOTest		(void *ptr)
{
	UINT32 GP_IO_REG =0x222;
	UINT8   bTmp;
	char	achErrTxt [80];
	UINT32 dRet = E__OK;

	bTmp = dIoReadReg (GP_IO_REG,REG_8);

	vIoWriteReg (GP_IO_REG,REG_8,0x00);
	vDelay(2);

	vIoWriteReg (GP_IO_REG,REG_8,0x04);
	vDelay(2);

	bTmp = dIoReadReg (GP_IO_REG,REG_8);

	if(bTmp == 0x05)
	{
		sprintf ( achErrTxt, "GPIO1 and GPIO3 are working\n");
	}
	else
	{
		sprintf ( achErrTxt, "GPIO1 and GPIO3 are not working\n");
		return (GPIO_LOOPBACK_ERR1);
	}
	vConsoleWrite (achErrTxt);

	vIoWriteReg (GP_IO_REG,REG_8,0x08);
	vDelay(2);
	bTmp = dIoReadReg (GP_IO_REG,REG_8);

	if(bTmp == 0x0A)
	{
		sprintf ( achErrTxt, "GPIO2 and GPIO4 are working\n");
	}
	else
	{
		sprintf ( achErrTxt, "GPIO2 and GPIO4 are not working\n");
		return (GPIO_LOOPBACK_ERR2);

	}
	vConsoleWrite (achErrTxt);

	return dRet;

}

/********************************************************************
 * brdGPIOTest
 *
 * This test will test the board specific GPIO lines.
 ********************************************************************/
TEST_INTERFACE (brdGPIOTest, "Board GPIO Test")
{
		UINT8	bTmp = 0;
		UINT8	bInputPin = 0, bOutputPin = 0;
		UINT32	wErrVal = E__OK;
		GPIO_INFO *localGPIOs = &localGpio;
	
#ifdef DEBUG
		char	achErrTxt [80];
#endif
	
	
#ifdef DEBUG
			sprintf ( achErrTxt, "\nGPIOReg %#x, NumGPIOPins  %#x\n", localGPIOs->wGPIOReg,
															   localGPIOs->wNumGPIOPins);
			vConsoleWrite (achErrTxt);
#endif
	
		/* just loop back output to input */
		/* GPO pins */
		for ( bOutputPin = 0; bOutputPin < 1; bOutputPin++ )
		{

			bTmp = ((UINT8)dIoReadReg(localGPIOs->wGPIOReg, REG_8));
			bTmp &= 0xF0;
			
			vIoWriteReg(localGPIOs->wGPIOReg, REG_8, DIR_0 << bOutputPin);
			vDelay( 5 );
			
			bTmp = ((UINT8)dIoReadReg(localGPIOs->wGPIOReg, REG_8));
		
			for ( bInputPin = 0; bInputPin < localGPIOs->wNumGPIOPins; bInputPin++ )
			{
				if ( ( bTmp & ( GPIO_0 << bInputPin ) ) == 0 )
				{
#ifdef DEBUG
					sprintf( achErrTxt,
							 "Output Bit %d HI. Expected Input Bit %d HI read LO\n",
							 bOutputPin, bInputPin );
					vConsoleWrite (achErrTxt);
					sysDebugWriteString(achErrTxt);
#endif
					wErrVal = GPIO_LOOPBACK_ERR1 + localGPIOs->wNumGPIOPins;
				}
			}

		
			
			bTmp = 0xFF;

			vIoWriteReg(localGPIOs->wGPIOReg, REG_8,  0xFF);
			vDelay ( 5 );
			
			vIoWriteReg(localGPIOs->wGPIOReg, REG_8,  (bTmp & (~( DIR_0 << bOutputPin))));
			vDelay ( 5 );
		
			bTmp = ((UINT8)dIoReadReg(localGPIOs->wGPIOReg, REG_8));

			for ( bInputPin = 0; bInputPin < localGPIOs->wNumGPIOPins; bInputPin++ )
			{
				if (( bTmp & (GPIO_0 << bInputPin))   != 0 )
				{
#ifdef DEBUG
					sprintf( achErrTxt,
							 "Output Bit %d LO. Expected Input Bit %d LO read HI\n",
							 bOutputPin, bInputPin );
					vConsoleWrite (achErrTxt);
					sysDebugWriteString(achErrTxt);
#endif
					wErrVal = GPIO_LOOPBACK_ERR2 + localGPIOs->wNumGPIOPins;
				}
			}
		 }
	
		 vIoWriteReg(localGPIOs->wGPIOReg, REG_8, 0);

		 return wErrVal;

}


