
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
 * fpNMI.c
 *
 *  Created on: 4 Jun 2010
 *      Author: engineer
 */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/mem.h>
#include <bit/hal.h>
 
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>

#define INT_PIN_NUM				2

#define	E__INT_HANDLER_FAIL		E__BIT
#define E__NO_FP_NMI			E__BIT  + 0x01


#define osWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define osWriteWord(regAddr,value)  (*(UINT16*)(regAddr) = value)
#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define osReadByte(regAddr)			(*(UINT8*)(regAddr))
#define osReadWord(regAddr)			(*(UINT16*)(regAddr))
#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))



static UINT8 fpINT;


/*****************************************************************************
 * FailingTest: NMI Handler function
 *
 * RETURNS: None
 */
static void fpNMIHandler (void)
{
	fpINT = 1;
}


/******************************************************************************
* fpNMITest :  Front Panel NMI Test
* RETURNS: 0 on success else error code
******************************************************************************/
TEST_INTERFACE (fpNMITest, "Front Panel NMI Test")
{
	UINT8   bPort61;
	UINT16  GPI_disable_temp;

	PCI_PFA	pfa;
	UINT16	wAcpiBase;

	fpINT = 0;

	if (sysInstallUserHandler (INT_PIN_NUM, fpNMIHandler) != 0)
	{
		return E__INT_HANDLER_FAIL ;
	}

	vIoWriteReg(0x70, REG_8, 0x80);	/* disable NMI */
	bPort61 = dIoReadReg (0x61, REG_8) & 0x07;
	vIoWriteReg (0x61, REG_8, bPort61 | 0x08);
	vIoWriteReg (0x61, REG_8, bPort61);
	vIoWriteReg (0x70, REG_8, 0);	/* enable NMI (in RTC) */


	pfa = PCI_MAKE_PFA (0, 31, 0);
	wAcpiBase = 0xFFFE & PCI_READ_WORD (pfa, 0x40);
	
	/* Disable NMI to SMI */
	GPI_disable_temp = dIoReadReg (wAcpiBase + 0x68, REG_16);
	vIoWriteReg (wAcpiBase + 0x68, REG_16, 0x0800);


	vConsoleWrite("Waiting 5 Seconds for Front Panel NMI\n");
	vDelay(5000);

	vIoWriteReg (wAcpiBase + 0x68, REG_16, GPI_disable_temp);
	vIoWriteReg(0x70, REG_8, 0x80);	/* disable NMI */

	sysInstallUserHandler (2, 0);

	if(fpINT == 1)
		return E__OK;
	else
		return E__NO_FP_NMI;
}
