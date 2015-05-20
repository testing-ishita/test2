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


/* includes */
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

/* defines */
//#define DEBUG

#define  DIR_3			0x80
#define  DIR_2			0x40
#define  DIR_1			0x20
#define  DIR_0			0x10
#define  GPIO_3			0x08
#define  GPIO_2			0x04
#define  GPIO_1			0x02
#define  GPIO_0			0x01

#define GPIO_LOOPBACK_ERR1		(E__BIT + 0x01)
#define GPIO_LOOPBACK_ERR2		(E__BIT + 0x05)

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

/* forward declarations */
UINT32 dGPIOLoopBackTest (GPIO_INFO *localGPIOs);
UINT32 dGPIOReadTest (GPIO_INFO *localGPIOs);

/*****************************************************************************
 * GPIOTest: Test the GPIOs
 *
 * RETURNS: None
 */
TEST_INTERFACE (GPIOTest, "GPIO Test")
{
	UINT32 dRet = E__OK;
	GPIO_INFO  *localGPIOs = NULL;
#ifdef DEBUG
	char	achErrTxt [80];
#endif

	board_service(SERVICE__BRD_GET_GPIO_INFO, NULL, &localGPIOs);


#ifdef DEBUG
		sprintf ( achErrTxt, "\nGPIOReg %#x, NumGPIOPins  %#x\n", localGPIOs->wGPIOReg,
				                                           localGPIOs->wNumGPIOPins);
		vConsoleWrite (achErrTxt);
#endif
	switch (localGPIOs->dTestType)
	{

		case 1:
			dRet = dGPIOLoopBackTest (localGPIOs);
			break;

		case 2:
			dRet = dGPIOReadTest (localGPIOs);
			break;

		default:
			break;

	}
	return dRet;
}


UINT32 dGPIOLoopBackTest (GPIO_INFO *localGPIOs)
{
	UINT8	bTmp = 0;
	UINT8	bInputPin = 0, bOutputPin = 0;
	UINT16  wErrVal = E__OK;

#ifdef DEBUG
	char	achErrTxt [80];
#endif


#ifdef DEBUG
		sprintf ( achErrTxt, "\nGPIOReg %#x, NumGPIOPins  %#x\n", localGPIOs->wGPIOReg,
				                                           localGPIOs->wNumGPIOPins);
		vConsoleWrite (achErrTxt);
#endif

	/* just loop back output to input */
	for ( bOutputPin = 0; bOutputPin < localGPIOs->wNumGPIOPins; bOutputPin++ )
	{

		vIoWriteReg(localGPIOs->wGPIOReg, REG_8, DIR_0 << bOutputPin);
		vDelay( 5 );
		bTmp = ((UINT8)dIoReadReg(localGPIOs->wGPIOReg, REG_8));

		for ( bInputPin = 0; bInputPin < localGPIOs->wNumGPIOPins; bInputPin++ )
		{
			if ( ( bTmp & ( GPIO_0 << bInputPin ) ) != 0 )
			{
#ifdef DEBUG
				sprintf( achErrTxt,
				         "Output Bit %d LO. Expected Input Bit %d LO read HI\n",
				         bOutputPin, bInputPin );
				sysDebugWriteString(achErrTxt);
#endif
				wErrVal = GPIO_LOOPBACK_ERR1 + localGPIOs->wNumGPIOPins;
			}
		}

		vIoWriteReg(localGPIOs->wGPIOReg, REG_8, ( DIR_0 | GPIO_0 ) << bOutputPin);
		vDelay ( 5 );
		bTmp = ((UINT8)dIoReadReg(localGPIOs->wGPIOReg, REG_8));

		for ( bInputPin = 0; bInputPin < localGPIOs->wNumGPIOPins; bInputPin++ )
		{
			if ( ( bTmp & ( GPIO_0 << bInputPin ) ) == 0 )
			{
#ifdef DEBUG
				sprintf( achErrTxt,
				         "Output Bit %d HI. Expected Input Bit %d HI read LO\n",
				         bOutputPin, bInputPin );
				sysDebugWriteString(achErrTxt);
#endif
				wErrVal = GPIO_LOOPBACK_ERR2 + localGPIOs->wNumGPIOPins;
			}
		}
	 }

	 vIoWriteReg(localGPIOs->wGPIOReg, REG_8, 0);
	 return wErrVal;
}

UINT32 dGPIOReadTest (GPIO_INFO *localGPIOs)
{
	UINT8	bTmp = 0;
	UINT16  wErrVal = E__OK;
	char	achErrTxt [80];


#ifdef DEBUG
		sprintf ( achErrTxt, "\nGPIOReg %#x, NumGPIOPins  %#x\n", localGPIOs->wGPIOReg,
				                                           localGPIOs->wNumGPIOPins);
		vConsoleWrite (achErrTxt);
#endif

		bTmp = ((UINT8)dIoReadReg(localGPIOs->wGPIOReg, REG_8));

		if ( (bTmp & 0x01) == 0x01 )
		{
			sprintf ( achErrTxt, "GPIO SWITCH 1: HIGH\n");
		}
		else
		{
			sprintf ( achErrTxt, "GPIO SWITCH 1: LOW\n");
		}
		vConsoleWrite (achErrTxt);

		if ( (bTmp & 0x02) == 0x02 )
		{
			sprintf ( achErrTxt, "GPIO SWITCH 2: HIGH\n");
		}
		else
		{
			sprintf ( achErrTxt, "GPIO SWITCH 2: LOW\n");
		}
		vConsoleWrite (achErrTxt);

		if ( (bTmp & 0x04) == 0x04 )
		{
			sprintf ( achErrTxt, "GPIO SWITCH 3: HIGH\n");
		}
		else
		{
			sprintf ( achErrTxt, "GPIO SWITCH 3: LOW\n");
		}
		vConsoleWrite (achErrTxt);

		if ( (bTmp & 0x08) == 0x08 )
		{
			sprintf ( achErrTxt, "GPIO SWITCH 4: HIGH\n");
		}
		else
		{
			sprintf ( achErrTxt, "GPIO SWITCH 4: LOW\n");
		}

		vConsoleWrite (achErrTxt);
		return wErrVal;
}

/*****************************************************************************
 * URTMGPIOTest: Test the uRTM GPIOs
 *
 * RETURNS: None
 */
TEST_INTERFACE (URTMGPIOTest, "MICRO RTM GPIO Test")
{
	UINT32 dRet=E__OK;

	board_service(SERVICE__BRD_URTM_GPIO_TEST, &dRet, NULL);

	return dRet;
}
