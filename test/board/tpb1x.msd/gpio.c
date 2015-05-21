/************************************************************************
 *                                                                      *
 *      Copyright 2015 Concurrent Technologies, all rights reserved.    *
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/ppb1x.msd/gpio.c,v 1.2 2015-04-01 16:44:29 hchalla Exp $
 *
 * $Log: gpio.c,v $
 * Revision 1.2  2015-04-01 16:44:29  hchalla
 * Initial release of CUTE firmware 02.02.01.
 *
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

#define GP_IO_REG       0x213
#define GP_IO_DATA        0x03

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

#define GPIO_0_7_DIR    0x55
#define GPIO_8_14_DIR  0x2A

#define GPIO0_7_DATA    0x55
#define GPIO8_14_DATA  0x2A

#define GPIO_0_7_DATA_STATUS_REG               0x0220
#define GPIO_8_14_DATA_STATUS_REG             0x0221
#define GPIO_0_7_DIR_CTRL_REG                      0x0222
#define GPIO_8_14_DIR_CTRL_REG                    0x0223
#define GPIO_0_7_INTR_FLAG_STAT_REG          0x0224
#define GPIO_8_14_INTR_FLAG_STAT_REG        0x0225
#define GPIO_0_7_INTR_TYPE_CTRL_REG         0x0226
#define GPIO_8_14_INTR_TYPE_CTRL_REG       0x0227
#define GPIO_0_7_INTR_MODE_CTRL_REG        0x0228
#define GPIO_8_14_INTR_MODE_CTRL_REG      0x0229
#define GPIO_0_7_INTR_ENABLE_CTRL_REG     0x022A
#define GPIO_8_14_INTR_ENABLE_CTRL_REG   0x022B




#define GP2_IO_REG      0x31D
#define  LH_EDGE_2      0x40    /*      -"- */
#define  GPIO_INTR_2    0x10    /* 0 = masked, 1 = interrupt enabled */
#define  DIR_2          0x04    /* 0 = GPIO is input, 1 = GPIO is output */
#define  GPIO_2         0x01    /*      -"- */


/* Interrupt Configuration Register */
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



	bTemp = dIoReadReg (GP_IO_REG,REG_8);
    if ((bTemp & 0x33) == 0x33)
    {
    	bIrqStatus = IRQ_TEST_PASS;
    	vIoWriteReg (GP_IO_REG,REG_8,0x00);
    }
    else
    {
    	bIrqStatus = IRQ_TEST_FAIL;
    }

    vIoWriteReg (0x21D,REG_8,dIoReadReg (0x21D,REG_8) & ~(0x40) );
    return;
}


/********************************************************************
 * vIrqPassHandler
 *
 * Interrupt handler for correct IRQ.
 * Set PASS flag and clears hardware interrupt
 ********************************************************************/
static void vExtGPIOIrqHandler (void)
{
	UINT8 bTemp;

	bTemp = dIoReadReg (GPIO_0_7_DATA_STATUS_REG,REG_8);
    if ((bTemp & 0xFF) == 0xFF)
    {
    	bIrqStatus = IRQ_TEST_PASS;
    	vIoWriteReg (GPIO_0_7_DATA_STATUS_REG,REG_8,0x00);
    	vIoWriteReg(GPIO_0_7_INTR_FLAG_STAT_REG,REG_8,0x03);
    }
    else
    {
    	bIrqStatus = IRQ_TEST_FAIL;
    }

    vIoWriteReg (GPIO_0_7_INTR_ENABLE_CTRL_REG,REG_8,0x00);
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
    UINT32 dTemp = 0,rt = E__OK;



    pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
    wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

    // GPIO_57 to check if J4 connector fitted on CPCI board.
    dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);

    bVar = (UINT8)((dTemp >> 25) & 0x01);

    /*
     *  Default Board GPIO on RTM
     */
      vIoWriteReg (GP_IO_REG,REG_8,GP_IO_DATA);
      vDelay(10);
      bTmp = dIoReadReg (GP_IO_REG,REG_8);
      if ((bTmp & 0x33) != 0x33)
      {
          sprintf (achErrTxt, "Expected bits '0' & '1' as HI read LOW \n");
          vConsoleWrite (achErrTxt);
          rt = E__ERR_1;
      }

      vIoWriteReg (GP_IO_REG,REG_8,0x00);
      vDelay(10);
      bTmp = dIoReadReg (GP_IO_REG,REG_8);
      if ((bTmp & 0x33) != 0x00)
      {
          sprintf (achErrTxt, "Expected bits '0' & '1' as HI read LOW \n");
          vConsoleWrite (achErrTxt);
          rt = E__ERR_2;
      }


      if (bVar == 0x00)  //check the board variant support GPIO 0 to GPIO 14 on CPCI J4 Connector
      {
    	  if (rt == E__OK)
    	  {
			  /*
				*   Set GPIO 1,3,5,7 as output direction and GPIO 2,4,6,8 as input direction
				*
				*/
			   vIoWriteReg (GPIO_0_7_DIR_CTRL_REG,REG_8,GPIO_0_7_DIR);
			   vIoWriteReg (GPIO_8_14_DIR_CTRL_REG,REG_8,GPIO_8_14_DIR);

			   /*
				* Now test all the GPIO's by writing 1's to 9,11
				*/
			   vIoWriteReg (GPIO_0_7_DATA_STATUS_REG,REG_8,GPIO0_7_DATA);
			   bTmp =  dIoReadReg (GPIO_0_7_DATA_STATUS_REG,REG_8);
			   if ((bTmp & 0xFF) != 0xFF)
			   {
				   sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO1-7\n");
				   vConsoleWrite (achErrTxt);
				   return E__ERR_9;
			   }
			   bTmp =  dIoReadReg (GPIO_8_14_DATA_STATUS_REG,REG_8);
			   bTmp = bTmp & 0x01;
			   if (bTmp !=  0x1)
			   {
				   sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO8\n");
				   vConsoleWrite (achErrTxt);
				   return E__ERR_9;
			   }

			   vIoWriteReg (GPIO_8_14_DATA_STATUS_REG,REG_8,GPIO8_14_DATA);
			   bTmp =  dIoReadReg (GPIO_8_14_DATA_STATUS_REG,REG_8);
			   if ((bTmp & 0x7E) != 0x7E)
			   {
				   sprintf (achErrTxt, "Expected bit '1' received '0' on GPIO8-14\n");
				   vConsoleWrite (achErrTxt);
				   return E__ERR_10;
			   }

			   /*
				* Now test all the GPIO's by writing 0's to 9,11
				*/
			   /*
					* Now test all the GPIO's by writing 1's to 9,11
					*/
				   vIoWriteReg (GPIO_0_7_DATA_STATUS_REG,REG_8, 0);
				   bTmp =  dIoReadReg (GPIO_0_7_DATA_STATUS_REG,REG_8);
				   if ((bTmp & 0xFF) != 0x00)
				   {
					   sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO1-7\n");
					   vConsoleWrite (achErrTxt);
					   return E__ERR_11;
				   }
				   bTmp =  dIoReadReg (GPIO_8_14_DATA_STATUS_REG,REG_8);
				   bTmp = bTmp & 0x01;
				   if (bTmp !=  0x0)
				   {
					   sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO8\n");
					   vConsoleWrite (achErrTxt);
					   return E__ERR_12;
				   }

				   vIoWriteReg (GPIO_8_14_DATA_STATUS_REG,REG_8, 0 );
				   bTmp =  dIoReadReg (GPIO_8_14_DATA_STATUS_REG,REG_8);
				   if ((bTmp & 0x7E) != 0x00)
				   {
					   sprintf (achErrTxt, "Expected bit '0' received '1' on GPIO8-14\n");
					   vConsoleWrite (achErrTxt);
					   return E__ERR_13;
				   }
    	    }
      }

    return rt;
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
	bTmp =  dIoReadReg (0x21D,REG_8);
	 vIoWriteReg (0x21D,REG_8, bTmp | (0x20));
	 vDelay(4);

	sysEnableInterrupts();

	 vDelay(4);
    /*
     * Now test all the GPIO's by writing 1's to 1,3,5,7
     */
     vIoWriteReg (GP_IO_REG,REG_8,GP_IO_DATA);
    vDelay(400);

	//sprintf (achErrTxt, "Debug 10\n");
  //   vConsoleWrite (achErrTxt);
    if (bIrqStatus == IRQ_TEST_PASS )
    {
    	wTestStatus = E__OK;
		sprintf (achErrTxt, "BOARD GPIO IRQ5 PASS\n");
        vConsoleWrite (achErrTxt);
    }
    else
    {
    	wTestStatus = E__INT_0;
    	sprintf (achErrTxt, "BOARD GPIO IRQ5 FAIL\n");
    	vConsoleWrite (achErrTxt);
    }


    /* Mask and disable interrupt and uninstall our interrupt handler */
    if (sysMaskPin (irqNum, SYS_IOAPIC0) == -1)
		return (E__MASK_SLOT);

    sysInstallUserHandler (iVector, 0);

    vIoWriteReg (GP_IO_REG,REG_8,0x00);
	return wTestStatus;
}




/********************************************************************
 * wInterruptTest
 *
 * Similar to loopback test but enable interrupt
 * generation
 ********************************************************************/
static UINT32 wExtInterruptTest (void)
{
	UINT8	bTmp;
	UINT8	bBit;
	char	achErrTxt [80];
	int		iVector= -1;
	UINT8 irqNum= 0;
	UINT32  wTestStatus = 10;
    UINT8 bVar   = 0;
    UINT16 wGpioBase = 0;
    PCI_PFA pfa;
    UINT32 dTemp=0;

    pfa = PCI_MAKE_PFA (0, LPC_BRIDGE_DEV, 0);
    wGpioBase = 0xFFFE & PCI_READ_WORD (pfa, GPIO_BASE);

    // GPIO_57 to check if J4 connector fitted on CPCI board.
    dTemp = dIoReadReg (wGpioBase + GP_LVL2, REG_32);

    bVar = (UINT8)((dTemp >> 25) & 0x01);

    if (bVar == 0)
    {
    	bIrqStatus = IRQ_TEST_NONE;
    	irqNum = 5;
    	/* Find vector used by system for chosen IOAPIC pin */
    	iVector = sysPinToVector (irqNum, SYS_IOAPIC0);

    	if (iVector == -1)
    		return (E__PIN_MAP);

    	sysInstallUserHandler (iVector, vExtGPIOIrqHandler);

    	if (sysUnmaskPin (irqNum, SYS_IOAPIC0) == -1)
    		return (E__UNMASK_SLOT);

    	/*
         *   Set GPIO 1,3,5,7 as output direction and GPIO 2,4,6,8 as input direction
         *
         */
    	 vIoWriteReg (GPIO_0_7_INTR_ENABLE_CTRL_REG,REG_8,0x03);
    	 vDelay(4);

    	 vIoWriteReg (GPIO_0_7_DIR_CTRL_REG,REG_8,GPIO_0_7_DIR);
        vDelay(4);



    	sysEnableInterrupts();

    	 vDelay(4);
        /*
         * Now test all the GPIO's by writing 1's to 1,3,5,7
         */
         vIoWriteReg (GPIO_0_7_DATA_STATUS_REG,REG_8,GPIO0_7_DATA);

         vDelay(400);

    	//sprintf (achErrTxt, "Debug 10\n");
      //   vConsoleWrite (achErrTxt);
        if (bIrqStatus == IRQ_TEST_PASS )
        {
        	wTestStatus = E__OK;
    		sprintf (achErrTxt, "EXT GPIO IRQ5 PASS\n");
            vConsoleWrite (achErrTxt);
        }
        else
        {
        	wTestStatus = E__INT_0;
        	sprintf (achErrTxt, "EXT GPIO IRQ5 FAIL\n");
        	vConsoleWrite (achErrTxt);
        }


        /* Mask and disable interrupt and uninstall our interrupt handler */
        if (sysMaskPin (irqNum, SYS_IOAPIC0) == -1)
    		return (E__MASK_SLOT);

        sysInstallUserHandler (iVector, 0);

        vIoWriteReg (GPIO_0_7_DATA_STATUS_REG,REG_8,0x00);

        return wTestStatus;
    }

    return E__OK;
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
	   wTestStatus =  wExtInterruptTest();
   }

   if (wTestStatus == E__OK)
   {
	   wTestStatus = wInterruptTest();
   }

    return wTestStatus;
}


