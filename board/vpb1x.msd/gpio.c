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
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vpb1x.msd/gpio.c,v 1.1 2015-02-25 17:50:45 hchalla Exp $
 *
 * $Log: gpio.c,v $
 * Revision 1.1  2015-02-25 17:50:45  hchalla
 * Initial Checkin for VP B1x board.
 *
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.2  2012/08/07 03:26:00  madhukn
 * *** empty log message ***
 *
 * Revision 1.1  2012/06/22 11:41:59  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:52  hchalla
 * Initial version of PP 91x sources.
 *
 *
 */
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
#include <bit/interrupt.h>

#include <bit/board_service.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>

#define LPC_BRIDGE_DEV	31
#define GPIO_BASE	    0x48

#define GP_LVL          0x0C
#define GP_LVL2        0x38
#define GP_LVL3       0x48

#define IRQ_TEST_PASS 0x00
#define IRQ_TEST_FAIL 0x01
#define IRQ_TEST_NONE 0x01

static UINT8 bIrqStatus = IRQ_TEST_NONE;        /* NONE, PASS or FAIL */

static void vGPIOIrqHandler (void);

//#define DEBUG
/* General Purpose IO Register */

#define GPIO_DIR_REG       0x310
#define GPIO1_7_OUTPUT  0x55
#define GPIO2_8_OUTPUT  0x88


#define GPIO_DATA_REG  0x31C
#define GPIO1_7_DATA  0x55
#define GPIO2_8_DATA  0xAA

#define GPIO_9_12_DIR_REG  0x213
#define GPIO_9_12_OUTPUT  0x05
#define GPIO_10_11_OUTPUT  0x0A

#define GPIO_9_12_DATA_REG 0x31B

#define GPIO_1_8_IER_REG       0x22F
#define GPIO_9_12_IER_REG     0x243
#define GPIO_1_8_ISR_REG       0x22C
#define GPIO_9_12_ISR_REG     0x242
#define GPIO_1_8_TRIG_REG     0x22E
#define GPIO_9_12_TRIG_REG   0x243

#define GP2_IO_REG      0x31D
#define  LH_EDGE_2      0x40    /*      -"- */
#define  GPIO_INTR_2    0x10    /* 0 = masked, 1 = interrupt enabled */
#define  DIR_2          0x04    /* 0 = GPIO is input, 1 = GPIO is output */
#define  GPIO_2         0x01    /*      -"- */

/* Interrupt Control Register */

#define INT_CONTROL_REG     0x215
#define  GPI_INT_FLAG       0x40    /* 0 = clear event, 1 = interrupt occured */
#define  SMS_ATN_INT_FLAG   0x20    /*      -"- */
#define  SMIC_INT_FLAG      0x10    /*      -"- */
#define  GPI_INT_ENA        0x04    /* 0 = disable, 1 = enable interrupt */
#define  SMS_ATN_INT_ENA    0x02    /*      -"- */
#define  SMIC_INT_ENA       0x01    /*      -"- */


/* Interrupt Configuration Register */

#define INT_CONFIG_REG  0x21D
#define  IPMI_STATUS    0x80    /* 1 = IPMI interrupt asserted, Wr.0 to clear */
#define  GDISC_INT_EN   0x10
#define  GDISC_INT_FLAG 0x20
#define  GPIO2_TRANS    0x04    /* 1 = GPIO transition trggered interrupt */
#define  GPIO1_TRANS    0x02    /* 1 = GPIO transition trggered interrupt */
#define  GPIO0_TRANS    0x01    /*      -"- */



#define E__ERR_0    (E__BIT)
#define E__ERR_1    (E__BIT+1)
#define E__ERR_2    (E__BIT+2)
#define E__ERR_3    (E__BIT+3)
#define E__ERR_4    (E__BIT+4)
#define E__ERR_5    (E__BIT+5)
#define E__ERR_6    (E__BIT+6)
#define E__ERR_7    (E__BIT+7)
#define E__ERR_8    (E__BIT+8)
#define E__ERR_9    (E__BIT+9)
#define E__ERR_10  (E__BIT+10)
#define E__ERR_11  (E__BIT+11)
#define E__ERR_12  (E__BIT+12)
#define E__ERR_13  (E__BIT+13)
#define E__ERR_14  (E__BIT+14)
#define E__ERR_15  (E__BIT+14)


#define E__INT_0    (E__BIT+0x10)
#define E__INT_1    (E__BIT+0x11)
#define E__INT_2    (E__BIT+0x12)
#define E__INT_3    (E__BIT+0x13)

#define E__INT_4    (E_EXTD_FAIL+0x14)

#define E__PIN_MAP		(E__BIT + 0x73)
#define E__UNMASK_SLOT	(E__BIT + 0x74)
#define E__MASK_SLOT	(E__BIT + 0x75)


#define IRQ_TEST_PASS 0x00
#define IRQ_TEST_FAIL 0x01
#define IRQ_TEST_NONE 0x01


/********************************************************************
 * vIrqPassHandler
 *
 * Interrupt handler for correct IRQ.
 * Set PASS flag and clears hardware interrupt
 ********************************************************************/
static void vGPIOIrqHandler (void)
{
	UINT8 bTemp;

    bTemp =  dIoReadReg (GPIO_DATA_REG,REG_8);

    if ((bTemp & 0x03) == 0x03){
    	bIrqStatus = IRQ_TEST_PASS;
    	vIoWriteReg (GPIO_DATA_REG,REG_8,0x00);
    }
    else
    {
    	bIrqStatus = IRQ_TEST_FAIL;
    }

    vIoWriteReg (GPIO_1_8_ISR_REG,REG_8,0x00);
    return;
}


/********************************************************************
 * wLoopbackTest
 *
 * Basic in/out loopback test for each GPIO pin
 ********************************************************************/
static UINT32 wLoopbackTest (void)
{
    UINT8   bTmp,bGPIOconfig;
    char    achErrTxt [80];

    PCI_PFA pfa;
    UINT16 wGpioBase = 0;
    UINT8 bVar   = 0;
    UINT32 dTemp = 0;



    pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
    wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

    // GPIO_58
    dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);

    bVar = (UINT8)((dTemp >> 26) & 0x01);

    /*
     *   Set GPIO 1,3,5,7 as output direction and GPIO 2,4,6,8 as input direction
     *
     */
    vIoWriteReg (GPIO_DIR_REG,REG_8,GPIO1_7_OUTPUT);

    /*
     * Now test all the GPIO's by writing 1's to 1,3,5,7
     */
    vIoWriteReg (GPIO_DATA_REG,REG_8,0x01);
    bTmp =  dIoReadReg (GPIO_DATA_REG,REG_8);
    if ((bTmp & 0x03) != 0x03)
    {
    	sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO-2\n");
    	vConsoleWrite (achErrTxt);
    	return E__ERR_1;
    }

    vIoWriteReg (GPIO_DATA_REG,REG_8,0x04);
    bTmp =  dIoReadReg (GPIO_DATA_REG,REG_8);
    if ((bTmp & 0x0C) != 0x0C)
    {
    	sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO-4\n");
    	vConsoleWrite (achErrTxt);
    	return E__ERR_2;
    }

    vIoWriteReg (GPIO_DATA_REG,REG_8,0x10);
    bTmp =  dIoReadReg (GPIO_DATA_REG,REG_8);
    if ((bTmp & 0x30) != 0x30)
    {
    	sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO-6\n");
    	vConsoleWrite (achErrTxt);
    	return E__ERR_3;
    }

    vIoWriteReg (GPIO_DATA_REG,REG_8,0x40);
    bTmp =  dIoReadReg (GPIO_DATA_REG,REG_8);
    if ((bTmp&0xC0) != 0xC0)
    {
    	sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO-8\n");
    	vConsoleWrite (achErrTxt);
    	return E__ERR_4;
    }

    /*
       * Now test all the GPIO's by writing 0's to 1,3,5,7
       */
      vIoWriteReg (GPIO_DATA_REG,REG_8,0x00);
      bTmp =  dIoReadReg (GPIO_DATA_REG,REG_8);
      if( (bTmp & 0x03) != 0x00)
      {
      	sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO-2\n");
      	vConsoleWrite (achErrTxt);
      	return E__ERR_5;
      }

      vIoWriteReg (GPIO_DATA_REG,REG_8,0x00);
      bTmp =  dIoReadReg (GPIO_DATA_REG,REG_8);
      if( (bTmp & 0x0C) != 0x00)
      {
        	sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO-4\n");
        	vConsoleWrite (achErrTxt);
        	return E__ERR_6;
       }

      vIoWriteReg (GPIO_DATA_REG,REG_8,0x00);
      bTmp =  dIoReadReg (GPIO_DATA_REG,REG_8);
      if( (bTmp & 0x30) != 0x00)
      {
        	sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO-6\n");
        	vConsoleWrite (achErrTxt);
        	return E__ERR_7;
       }

      vIoWriteReg (GPIO_DATA_REG,REG_8,0x00);
      bTmp =  dIoReadReg (GPIO_DATA_REG,REG_8);
      if( (bTmp & 0xC0) != 0x00)
      {
        	sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO-8\n");
        	vConsoleWrite (achErrTxt);
        	return E__ERR_8;
       }


      if (bVar == 0x01)  //check the board variant support GPIO 9 to GPIO 12
      {
		  /*
			*   Set GPIO 1,3,5,7 as output direction and GPIO 2,4,6,8 as input direction
			*
			*/
		   vIoWriteReg (GPIO_9_12_DIR_REG,REG_8,GPIO_9_12_OUTPUT);

		   /*
			* Now test all the GPIO's by writing 1's to 9,11
			*/
		   vIoWriteReg (GPIO_9_12_DATA_REG,REG_8,0x01);
		   bTmp =  dIoReadReg (GPIO_9_12_DATA_REG,REG_8);
		   if ((bTmp & 0x03) != 0x03)
		   {
			   sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO-10\n");
			   vConsoleWrite (achErrTxt);
			   return E__ERR_9;
		   }

		   vIoWriteReg (GPIO_9_12_DATA_REG,REG_8,0x04);
		   bTmp =  dIoReadReg (GPIO_9_12_DATA_REG,REG_8);
		   if ((bTmp & 0x0C) != 0x0C)
		   {
			   sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO-12\n");
			   vConsoleWrite (achErrTxt);
			   return E__ERR_10;
		   }

		   /*
			* Now test all the GPIO's by writing 0's to 9,11
			*/
			   vIoWriteReg (GPIO_9_12_DATA_REG,REG_8,0x00);
			 bTmp =  dIoReadReg (GPIO_9_12_DATA_REG,REG_8);
			 if ((bTmp & 0x03) != 0x00)
			 {
				 sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO-10\n");
				 vConsoleWrite (achErrTxt);
				 return E__ERR_11;
			 }

			 vIoWriteReg (GPIO_9_12_DATA_REG,REG_8,0x00);
			 bTmp =  dIoReadReg (GPIO_9_12_DATA_REG,REG_8);
			 if ((bTmp & 0x0C) != 0x00)
			 {
				 sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO-12\n");
				 vConsoleWrite (achErrTxt);
				 return E__ERR_12;
			 }

			vIoWriteReg (GPIO_DATA_REG,REG_8,0x00);
			vIoWriteReg (GPIO_9_12_DATA_REG,REG_8,0x00);
      }

    return E__OK;
}


/********************************************************************
 * wInterruptTest
 *
 * Similar to loopback test but enable interrupt
 * generation
 ********************************************************************/
static UINT32 wInterruptTest (void)
{
	UINT8	bTmp;
	UINT8	bBit;
	char	achErrTxt [80];
	int		iVector= -1;
	UINT8 irqNum= 0;
	UINT32  wTestStatus = 10;

	bIrqStatus = IRQ_TEST_NONE;

	irqNum = 5;
	/* Find vector used by system for chosen IOAPIC pin */
	iVector = sysPinToVector (irqNum, SYS_IOAPIC0);



	if (iVector == -1)
		return (E__PIN_MAP);

	sysInstallUserHandler (iVector, vGPIOIrqHandler);

	if (sysUnmaskPin (irqNum, SYS_IOAPIC0) == -1)
		return (E__UNMASK_SLOT);




	/*
     *   Set GPIO 1,3,5,7 as output direction and GPIO 2,4,6,8 as input direction
     *
     */
	 vIoWriteReg (GPIO_1_8_TRIG_REG,REG_8,0x00);
	 vDelay(4);

    vIoWriteReg (GPIO_DIR_REG,REG_8,GPIO1_7_OUTPUT);
    vDelay(4);

    vIoWriteReg (GPIO_1_8_IER_REG,REG_8,0x03);
    vDelay(4);

	sysEnableInterrupts();

	 vDelay(4);
    /*
     * Now test all the GPIO's by writing 1's to 1,3,5,7
     */
    vIoWriteReg (GPIO_DATA_REG,REG_8,0x01);

    vDelay(400);

	//sprintf (achErrTxt, "Debug 10\n");
  //   vConsoleWrite (achErrTxt);
    if (bIrqStatus == IRQ_TEST_PASS )
    {
    	wTestStatus = E__OK;
		sprintf (achErrTxt, "GPIO IRQ5 PASS\n");
        vConsoleWrite (achErrTxt);
    }
    else
    {
    	wTestStatus = E__INT_0;
    	sprintf (achErrTxt, "GPIO IRQ5 FAIL\n");
    	vConsoleWrite (achErrTxt);
    }


    /* Mask and disable interrupt and uninstall our interrupt handler */
    if (sysMaskPin (irqNum, SYS_IOAPIC0) == -1)
		return (E__MASK_SLOT);

    sysInstallUserHandler (iVector, 0);

    vIoWriteReg (GPIO_DATA_REG,REG_8,0x00);
    vIoWriteReg (GPIO_1_8_IER_REG,REG_8,0x00);

	return wTestStatus;
}


/********************************************************************
 * brdGPIOTest
 *
 * This test will test the board specific GPIO lines.
 ********************************************************************/
TEST_INTERFACE (brdGPIOTest, "Board GPIO Test")
{
    UINT32  wTestStatus = 10;

   wTestStatus = wLoopbackTest ();

   if (wTestStatus == E__OK)
   {
	   wTestStatus = wInterruptTest();
   }
    return wTestStatus;
}


