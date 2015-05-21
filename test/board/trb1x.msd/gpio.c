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
#include <bit/interrupt.h>

#include <bit/board_service.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>

#include "cctboard.h"

//#define DEBUG
/* General Purpose IO Registers */

#define GP_IO_REG1      0x213
#define  LH_EDGE_1      0x80    /* 0 = H->L generates interrupt, 1 = L->H */
#define  LH_EDGE_0      0x40    /*      -"- */
#define  GPIO_INTR_1    0x20    /* 0 = masked, 1 = interrupt enabled */
#define  GPIO_INTR_0    0x10    /*      -"- */
#define  DIR_1          0x08    /* 0 = GPIO is input, 1 = GPIO is output */
#define  DIR_0          0x04    /*      -"- */
#define  GPIO_1         0x02    /* value read/driven via GPIO */
#define  GPIO_0         0x01    /*      -"- */

#define GP_IO_REG2      0x31D
#define  LH_EDGE_3      0x80    /* 0 = H->L generates interrupt, 1 = L->H */
#define  LH_EDGE_2      0x40    /*      -"- */
#define  GPIO_INTR_3    0x20    /* 0 = masked, 1 = interrupt enabled */
#define  GPIO_INTR_2    0x10    /*      -"- */
#define  DIR_3          0x08    /* 0 = GPIO is input, 1 = GPIO is output */
#define  DIR_2          0x04    /*      -"- */
#define  GPIO_3         0x02    /* value read/driven via GPIO */
#define  GPIO_2         0x01    /*      -"- */

#define GP_IO_REG3      0x31B
#define  LH_EDGE_5      0x80    /* 0 = H->L generates interrupt, 1 = L->H */
#define  LH_EDGE_4      0x40    /*      -"- */
#define  GPIO_INTR_5    0x20    /* 0 = masked, 1 = interrupt enabled */
#define  GPIO_INTR_4    0x10    /*      -"- */
#define  DIR_5          0x08    /* 0 = GPIO is input, 1 = GPIO is output */
#define  DIR_4          0x04    /*      -"- */
#define  GPIO_5         0x02    /* value read/driven via GPIO */
#define  GPIO_4         0x01    /*      -"- */

/* Interrupt Control Register */

#define INT_CONTROL_REG     0x215
//#define  IPMI_MODE			0x80
#define  GPI_INT_FLAG       0x40    /* 0 = clear event, 1 = interrupt occured */
#define  SMS_ATN_INT_FLAG   0x20    /*      -"- */
#define  SMIC_INT_FLAG      0x10    /*      -"- */
#define  GPI_INT_ENA        0x04    /* 0 = disable, 1 = enable interrupt */
#define  SMS_ATN_INT_ENA    0x02    /*      -"- */
#define  SMIC_INT_ENA       0x01    /*      -"- */


/* Interrupt Configuration Register 1 */

#define INT_CFG_REG1    0x21D
#define  IPMI_STATUS    0x80    /* 1 = IPMI interrupt asserted, Wr.0 to clear */
#define  GDISC_INT_FLAG 0x20
#define  GDISC_INT_EN   0x10
#define  GPIO2_TRANS    0x04    /* 1 = GPIO transition triggered interrupt */
#define  GPIO1_TRANS    0x02    /* 1 = GPIO transition triggered interrupt */
#define  GPIO0_TRANS    0x01    /*      -"- */

/* Interrupt Configuration Register 2 */

#define INT_CFG_REG2    0x31C
#define  GPIO5_TRANS    0x04    /* 1 = GPIO transition triggered interrupt */
#define  GPIO4_TRANS    0x02    /* 1 = GPIO transition triggered interrupt */
#define  GPIO3_TRANS    0x01    /*      -"- */


#define E__ERR_0    (E__BIT)
#define E__ERR_1    (E__BIT+1)
#define E__ERR_2    (E__BIT+2)
#define E__ERR_3    (E__BIT+3)
#define E__ERR_4    (E__BIT+3)
#define E__ERR_5    (E__BIT+3)
#define E__INT_0    (E__BIT+0x10)
#define E__INT_1    (E__BIT+0x11)
#define E__INT_2    (E__BIT+0x12)
#define E__INT_3    (E__BIT+0x13)

#define E__INT_4    (E_EXTD_FAIL+0x14)

#define E__PIN_MAP		(E__BIT + 0x73)
#define E__UNMASK_SLOT	(E__BIT + 0x74)
#define E__MASK_SLOT	(E__BIT + 0x75)
#define E__GPIO_GROUP	(E__BIT + 0x76)


#define IRQ_TEST_PASS 0x00
#define IRQ_TEST_FAIL 0x01
#define IRQ_TEST_NONE 0x01

#ifdef DEBUG
	static char buffer[128];
#endif

static UINT8 bIrqStatus;        /* NONE, PASS or FAIL */


static void vIrqPassHandler (void);

/********************************************************************
 * vClearInt
 *
 * Clear the hardware interrupt enable and status bits
 ********************************************************************/
static void vClearInt (void)
{
    vIoWriteReg (GP_IO_REG1, REG_8,0);
    vIoWriteReg (GP_IO_REG2, REG_8,0);
    vIoWriteReg (GP_IO_REG3, REG_8,0);
    vIoWriteReg (INT_CFG_REG1, REG_8,0);
    vIoWriteReg (INT_CFG_REG2, REG_8,0);
}

/********************************************************************
 * vIrqPassHandler
 *
 * Interrupt handler for correct IRQ.
 * Set PASS flag and clears hardware interrupt
 ********************************************************************/
static void vIrqPassHandler (void)
{
    vClearInt();
    bIrqStatus = IRQ_TEST_PASS;
}

/********************************************************************
 * vIrqFailHandler
 *
 *
 * Interrupt handler for other IRQs.
 * Set FAIL flag and clears hardware interrupt
 ********************************************************************/
#if 0
static void vIrqFailHandler (void)
{
    vClearInt();
    bIrqStatus = IRQ_TEST_FAIL;
}
#endif

/********************************************************************
 * wLoopbackTest1
 *
 * Basic in/out loopback1 test for each GPIO_0, GPIO_1 and GPIO_2
 ********************************************************************/
static UINT32 wLoopbackTest1 (void)
{
    UINT8   bTmp;
    char    achErrTxt [80];

/*
 * This test requires GPIO_1 to be configured as a GPIO pin.
 * GPIO_0, GPIO_1 and GPIO_2 will need to be linked together.
 * It may be necessary to configure the hardware using IPMI controller.
 * as GPIO_1 can optionally be configured as a board reset input.
 * It has not yet been decided whether this will be done using the IPMI controller
 * or through an IO port. If IPMI controller is used to configure the GPIO_1 pin,
 * it must be configured prior to starting this test and the board reset for it to
 * take effect
 */

    /* make sure that GPIO_1 is not configured as 'mask reset' */
    /* code assumes that this bit can be set by IO register    */
    /* but may need to be set by IPMI controller               */
    bTmp = dIoReadReg (SER_VPX_CFG, REG_8);
    if ((bTmp & GPIO1_MODE) != 0)
    {
    	bTmp &= ~GPIO1_MODE;
        vIoWriteReg (SER_VPX_CFG, REG_8,  bTmp);
    }

    /* make GPIO_0 output, GPIO_1 and GPIO_2 input & no interrupts */
    vIoWriteReg (GP_IO_REG1, REG_8, DIR_0);
    vIoWriteReg (GP_IO_REG2, REG_8, 0);
    vIoWriteReg (GP_IO_REG3, REG_8, 0);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    if ((bTmp & GPIO_1) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_1 LO read HI (%02x)\n", bTmp);
	    vConsoleWrite (achErrTxt);
        return E__ERR_0;
    }

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    if ((bTmp & GPIO_2) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_2 LO read HI (%02x)\n", bTmp);
	    vConsoleWrite (achErrTxt);
        return E__ERR_0;
    }

#ifdef DEBUG
    vConsoleWrite ("11\n");
#endif

    /* output a 1 to GPIO_0 */

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_0);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    if ((bTmp & GPIO_1) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_1 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_1;
    }

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    if ((bTmp & GPIO_2) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_2 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_1;
    }

 /* now make GPIO_1 an output and GPIO_0 an input */

#ifdef DEBUG
    vConsoleWrite ("21\n");
#endif

    vIoWriteReg (GP_IO_REG1, REG_8, DIR_1);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    if ((bTmp & GPIO_0) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_0 LO read HI (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_2;
    }

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    if ((bTmp & GPIO_2) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_2 LO read HI (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_2;
    }

    /* output a 1 to GPIO_1 */

#ifdef DEBUG
    vConsoleWrite ("31\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_1);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    if ((bTmp & GPIO_0) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_0 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_3;
    }

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    if ((bTmp & GPIO_2) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_2 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_3;
    }

    /* now make GPIO_2 an output and GPIO_1 an input */

#ifdef DEBUG
       vConsoleWrite ("41\n");
#endif

	vIoWriteReg (GP_IO_REG1, REG_8, 0);
	vIoWriteReg (GP_IO_REG2, REG_8, DIR_2);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    if ((bTmp & GPIO_0) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_0 LO read HI (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_4;
    }

    if ((bTmp & GPIO_1) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_1 LO read HI (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_4;
    }

    /* output a 1 to GPIO_2 */

#ifdef DEBUG
      vConsoleWrite ("51\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_2);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    if ((bTmp & GPIO_0) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_0 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_5;
    }

    if ((bTmp & GPIO_1) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_1 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_5;
    }
	
	// reset GP_IO_REG2
    vIoWriteReg (GP_IO_REG2, REG_8, 0);

    return E__OK;
}

/********************************************************************
 * wLoopbackTest2
 *
 * Basic in/out loopback1 test for each GPIO_3, GPIO_4 and GPIO_5
 ********************************************************************/
static UINT32 wLoopbackTest2 (void)
{
    UINT8   bTmp;
    char    achErrTxt [80];

/*
 * GPIO_3, GPIO_4 and GPIO_5 will need to be linked together.
 * These GPIOs are a build option and may not be present
 */

    /* make GPIO_3 output, GPIO_4 and GPIO_5 input & no interrupts */
    vIoWriteReg (GP_IO_REG1,REG_8, 0);
    vIoWriteReg (GP_IO_REG2,REG_8, DIR_3);
    vIoWriteReg (GP_IO_REG3,REG_8, 0);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    if ((bTmp & GPIO_4) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_4 LO read HI (%02x)\n", bTmp);
	    vConsoleWrite (achErrTxt);
        return E__ERR_0;
    }

    if ((bTmp & GPIO_5) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_5 LO read HI (%02x)\n", bTmp);
	    vConsoleWrite (achErrTxt);
        return E__ERR_0;
    }

#ifdef DEBUG
    vConsoleWrite ("12\n");
#endif

    /* output a 1 to GPIO_3 */

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_3);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    if ((bTmp & GPIO_4) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_4 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_1;
    }

    if ((bTmp & GPIO_5) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_5 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_1;
    }

 /* now make GPIO_4 an output and GPIO_3 an input */

#ifdef DEBUG
    vConsoleWrite ("22\n");
#endif

    vIoWriteReg (GP_IO_REG2, REG_8, 0);
    vIoWriteReg (GP_IO_REG3, REG_8, DIR_4);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    if ((bTmp & GPIO_3) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_3 LO read HI (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_2;
    }

    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    if ((bTmp & GPIO_5) != 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_5 LO read HI (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_2;
    }

    /* output a 1 to GPIO_4 */

#ifdef DEBUG
    vConsoleWrite ("32\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp | GPIO_4);
    vDelay(5);

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    if ((bTmp & GPIO_3) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_3 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_3;
    }

    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    if ((bTmp & GPIO_5) == 0)
    {
        sprintf (achErrTxt, "Expected bit GPIO_5 HI read LO (%02x)\n", bTmp);
        vConsoleWrite (achErrTxt);
        return E__ERR_3;
    }

    /* now make GPIO_5 an output and GPIO_4 an input */

   #ifdef DEBUG
       vConsoleWrite ("42\n");
   #endif

       vIoWriteReg (GP_IO_REG3, REG_8, DIR_5);
       vDelay(5);

       bTmp = dIoReadReg (GP_IO_REG2, REG_8);
       if ((bTmp & GPIO_3) != 0)
       {
           sprintf (achErrTxt, "Expected bit GPIO_3 LO read HI (%02x)\n", bTmp);
           vConsoleWrite (achErrTxt);
           return E__ERR_4;
       }

       bTmp = dIoReadReg (GP_IO_REG3, REG_8);
       if ((bTmp & GPIO_4) != 0)
       {
           sprintf (achErrTxt, "Expected bit GPIO_4 LO read HI (%02x)\n", bTmp);
           vConsoleWrite (achErrTxt);
           return E__ERR_4;
       }

       /* output a 1 to GPIO_2 */

   #ifdef DEBUG
       vConsoleWrite ("52\n");
   #endif

       bTmp = dIoReadReg (GP_IO_REG3, REG_8);
       vIoWriteReg (GP_IO_REG3, REG_8, bTmp | GPIO_5);
       vDelay(5);

       bTmp = dIoReadReg (GP_IO_REG2, REG_8);
       if ((bTmp & GPIO_3) == 0)
       {
           sprintf (achErrTxt, "Expected bit GPIO_3 HI read LO (%02x)\n", bTmp);
           vConsoleWrite (achErrTxt);
           return E__ERR_5;
       }

       bTmp = dIoReadReg (GP_IO_REG3, REG_8);
       if ((bTmp & GPIO_4) == 0)
       {
           sprintf (achErrTxt, "Expected bit GPIO_4 HI read LO (%02x)\n", bTmp);
           vConsoleWrite (achErrTxt);
           return E__ERR_5;
       }

    return E__OK;
}


/********************************************************************
 * wInterruptTest1
 *
 * Similar to loopback test but enable interrupt
 * generation for GPIO_0, GPIO_1 and GPIO_2
 ********************************************************************/
static UINT32 wInterruptTest1 (void)
{
    UINT8   bTmp;
	int		iVector= -1;
    char    achErrTxt [80];


    bIrqStatus = IRQ_TEST_NONE;
#ifdef DEBUG
    vConsoleWrite ("i11\n");
#endif

	/* Find vector used by system for chosen IOAPIC pin */
	iVector = sysPinToVector (5, SYS_IOAPIC0);

	if (iVector == -1)
		return (E__PIN_MAP);

	sysInstallUserHandler (iVector, vIrqPassHandler);

	if (sysUnmaskPin (5, SYS_IOAPIC0) == -1)
		return (E__UNMASK_SLOT);

	 sysEnableInterrupts();

#ifdef DEBUG
	vConsoleWrite ("i21\n");
#endif

	/* GPIO_0 output                    */
	/* GPIO_1 input, lo to hi interrupt */
	/* GPIO_2 input no interrupt        */
    vIoWriteReg (GP_IO_REG1, REG_8, DIR_0);
    vIoWriteReg (GP_IO_REG2, REG_8, 0);
    vIoWriteReg (GP_IO_REG3, REG_8, 0);
    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | LH_EDGE_1); 	 /* enable the interrupt type*/
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	 /* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	 /* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_INTR_1); /* enable the interrupt */

    vDelay(10);
	
#ifdef DEBUG
    vConsoleWrite ("i311\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG1, REG_8);                /* read status */
    if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i411\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_0);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("i511\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG1, REG_8);                /* read status */
        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("i611\n");
#endif

        if ((bTmp & (GPIO0_TRANS | GPIO2_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
		
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("i711\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG1,REG_8,0);
        vIoWriteReg (GP_IO_REG2,REG_8,0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }

	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_1 LO to HI\n");
#endif
	}

	/* GPIO_0 output                    */
	/* GPIO_1 input, hi to lo interrupt */
	/* GPIO_2 input no interrupt        */
    vIoWriteReg (GP_IO_REG1, REG_8, 0);
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	 /* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	 /* clear all interrupt status */
    vIoWriteReg (GP_IO_REG1, REG_8, DIR_0);				 /* set GPIO_0 as output       */
    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_0);		 /* set GPIO_0 HI              */
    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_INTR_1); /* enable GPIO_1 HI to LO interrupt */

    vDelay(10);

#ifdef DEBUG
    vConsoleWrite ("i811\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG1, REG_8);                /* read status */
    if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i911\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp & ~GPIO_0);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("iA11\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG1,REG_8);                /* read status */
        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("iB11\n");
#endif

        if ((bTmp & (GPIO0_TRANS | GPIO2_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8,0);
            vIoWriteReg (GP_IO_REG2, REG_8,0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG1,REG_8,0);
        vIoWriteReg (GP_IO_REG2,REG_8,0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("iC11\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG1,REG_8,0);
        vIoWriteReg (GP_IO_REG2,REG_8,0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }


	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_1 HI to LO\n");
#endif
	}

	/* GPIO_0 input, no interrupt       */
	/* GPIO_1 output                    */
	/* GPIO_2 input, lo to hi interrupt */
    vIoWriteReg (GP_IO_REG1, REG_8, DIR_1);
    vIoWriteReg (GP_IO_REG2, REG_8, LH_EDGE_2);
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	 /* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	 /* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_INTR_2); /* enable the interrupt */

    vDelay(10);

#ifdef DEBUG
    vConsoleWrite ("i312\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG1, REG_8);                /* read status */
    if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i412\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_1);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("i512\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG1, REG_8);                /* read status */
        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("i612\n");
#endif

        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG    
    vConsoleWrite ("i712\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }

	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_2 LO to HI\n");
#endif
	}

	/* GPIO_0 input, no interrupt       */
	/* GPIO_1 output                    */
	/* GPIO_2 input, hi to lo interrupt */
    vIoWriteReg (GP_IO_REG2, REG_8, 0);
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	/* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	/* clear all interrupt status */
    vIoWriteReg (GP_IO_REG1, REG_8, DIR_1);				/* set GPIO_1 as output       */
    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_1);		/* set GPIO_1 HI              */
    vIoWriteReg (GP_IO_REG2, REG_8, GPIO_INTR_2); 		/* enable GPIO_2 HI to LO interrupt */

    vDelay(10);

#ifdef DEBUG
    vConsoleWrite ("i812\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG1, REG_8);                /* read status */
    if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i912\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp & ~GPIO_1);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("iA12\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG1, REG_8);                /* read status */
        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
#ifdef DEBUG
        vConsoleWrite ("iB12\n");
#endif

        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("iC12\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }


	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_2 HI to LO\n");
#endif
	}

	/* GPIO_0 input, lo to hi interrupt */
	/* GPIO_1 input, no interrupt       */
	/* GPIO_2 output                    */
    vIoWriteReg (GP_IO_REG1, REG_8, LH_EDGE_0);
    vIoWriteReg (GP_IO_REG2, REG_8, DIR_2);
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	 /* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	 /* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG1, REG_8);
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp | GPIO_INTR_0); /* enable the interrupt */

    vDelay(10);

#ifdef DEBUG
    vConsoleWrite ("i310\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG1, REG_8);                /* read status */
    if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i410\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_2);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("i510\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG1, REG_8);                /* read status */
        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("i610\n");
#endif

        if ((bTmp & (GPIO1_TRANS | GPIO2_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("i710\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }

	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_0 LO to HI\n");
#endif
	}

	/* GPIO_0 input, hi to lo interrupt */
	/* GPIO_1 input, no interrupt       */
	/* GPIO_2 output                    */
    vIoWriteReg (GP_IO_REG1, REG_8, 0);
    vIoWriteReg (GP_IO_REG2, REG_8, DIR_2);				/* set GPIO_2 as output       */
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	/* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	/* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_2);		/* set GPIO_2 HI              */
    vIoWriteReg (GP_IO_REG1, REG_8, bTmp);
    vIoWriteReg (GP_IO_REG1, REG_8, GPIO_INTR_0); 		/* enable GPIO_2 HI to LO interrupt */

    vDelay(10);

#ifdef DEBUG
    vConsoleWrite ("i810\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG1, REG_8);                /* read status */
    if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i910\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp & ~GPIO_2);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("iA10\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG1, REG_8);                /* read status */
        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS | GPIO2_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("iB10\n");
#endif

        if ((bTmp & (GPIO1_TRANS | GPIO1_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG1, REG_8, 0);
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("iC10\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG1, REG_8, 0);
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }


	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_0 HI to LO\n");
#endif
	}

#ifdef DEBUG
    vConsoleWrite ("iD1\n");
#endif

	if (sysMaskPin (5, SYS_IOAPIC0) == -1)
 		return (E__MASK_SLOT);

	sysInstallUserHandler (iVector, 0);

    return E__OK;
}

/********************************************************************
 * wInterruptTest2
 *
 * Similar to loopback test but enable interrupt
 * generation for GPIO_3, GPIO_4 and GPIO_5
 ********************************************************************/
static UINT32 wInterruptTest2 (void)
{
    UINT8   bTmp;
	int		iVector= -1;
    char    achErrTxt [80];

    bIrqStatus = IRQ_TEST_NONE;
#ifdef DEBUG
    vConsoleWrite ("i12\n");
#endif

	/* Find vector used by system for chosen IOAPIC pin */
	iVector = sysPinToVector (5, SYS_IOAPIC0);

	if (iVector == -1)
		return (E__PIN_MAP);

	sysInstallUserHandler (iVector, vIrqPassHandler);

	if (sysUnmaskPin (5, SYS_IOAPIC0) == -1)
		return (E__UNMASK_SLOT);

	sysEnableInterrupts();

#ifdef DEBUG
	vConsoleWrite ("i22\n");
#endif


	/* GPIO_3 output                    */
	/* GPIO_4 input, lo to hi interrupt */
	/* GPIO_5 input no interrupt        */
    vIoWriteReg (GP_IO_REG1, REG_8, 0);
    vIoWriteReg (GP_IO_REG2, REG_8, DIR_3);
    vIoWriteReg (GP_IO_REG3, REG_8, LH_EDGE_4);
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	 /* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	 /* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp | GPIO_INTR_4); /* enable the interrupt */

#ifdef DEBUG
    vConsoleWrite ("i324\n");
#endif

    vDelay(10);

    bTmp = dIoReadReg (INT_CFG_REG2, REG_8);                /* read status */
    if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i424\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_3);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("i524\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG2, REG_8);                /* read status */
        if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("i624\n");
#endif

        if ((bTmp & (GPIO3_TRANS | GPIO5_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("i724\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }

	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_4 LO to HI\n");
#endif
	}

	/* GPIO_3 output                    */
	/* GPIO_4 input, ho to lo interrupt */
	/* GPIO_5 input no interrupt        */
    vIoWriteReg (GP_IO_REG2, REG_8, DIR_3);				/* set GPIO_3 as output       */
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	/* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	/* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_3);		/* set GPIO_3 HI              */
    vIoWriteReg (GP_IO_REG3, REG_8, GPIO_INTR_4); 		/* enable GPIO_1 HI to LO interrupt */

    vDelay(10);

#ifdef DEBUG
    vConsoleWrite ("i824\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG2, REG_8);                /* read status */
    if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i924\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp & ~GPIO_3);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("iA24\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG2, REG_8);                /* read status */
        if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("iB24\n");
#endif

        if ((bTmp & (GPIO3_TRANS | GPIO5_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("iC24\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }


	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_4 HI to LO\n");
#endif
	}

	/* GPIO_3 input, no interrupt       */
	/* GPIO_4 output                    */
	/* GPIO_5 input, lo to hi interrupt */
    vIoWriteReg (GP_IO_REG2, REG_8, 0);
    vIoWriteReg (GP_IO_REG3, REG_8, DIR_4);
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	 /* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	 /* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp | LH_EDGE_5); 	/* enable the interrupt type */
    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp | GPIO_INTR_5); /* enable the interrupt */

#ifdef DEBUG
    vConsoleWrite ("i325\n");
#endif

    vDelay(10);

    bTmp = dIoReadReg (INT_CFG_REG2, REG_8);                /* read status */
    if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i425\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp | GPIO_4);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("i525\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG2, REG_8);                /* read status */
        if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("i625\n");
#endif

        if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("i725\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }

	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_4 LO to HI\n");
#endif
	}

	/* GPIO_3 input, no interrupt       */
	/* GPIO_4 output                    */
	/* GPIO_5 input, hi to lo interrupt */
    vIoWriteReg (GP_IO_REG2, REG_8, 0);
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	/* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	/* clear all interrupt status */
    vIoWriteReg (GP_IO_REG3, REG_8, DIR_4);				/* set GPIO_4 as output       */
    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp | GPIO_4);		/* set GPIO_4 HI              */
    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_INTR_5);	/* enable GPIO_5 HI to LO interrupt */

    vDelay(10);

#ifdef DEBUG
    vConsoleWrite ("i825\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG2, REG_8);                /* read status */
    if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i925\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp & ~GPIO_4);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("iA25\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG2, REG_8);                /* read status */
        if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
#ifdef DEBUG
        vConsoleWrite ("iB25\n");
#endif

        if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("iC25\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }


	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_5 HI to LO\n");
#endif
	}

	/* GPIO_3 input, lo to hi interrupt */
	/* GPIO_4 input, no interrupt       */
	/* GPIO_5 output                    */
    vIoWriteReg (GP_IO_REG2, REG_8, LH_EDGE_3);
    vIoWriteReg (GP_IO_REG3, REG_8, DIR_5);
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	 /* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	 /* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG2, REG_8);
    vIoWriteReg (GP_IO_REG2, REG_8, bTmp | GPIO_INTR_3); /* enable the interrupt */

#ifdef DEBUG
    vConsoleWrite ("i323\n");
#endif

    vDelay(10);

    bTmp = dIoReadReg (INT_CFG_REG2, REG_8);                /* read status */
    if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i423\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp | GPIO_5);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("i523\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG2, REG_8);                /* read status */
        if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG1,REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("i623\n");
#endif

        if ((bTmp & (GPIO4_TRANS | GPIO5_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG2, REG_8, 0);
            vIoWriteReg (GP_IO_REG3, REG_8, 0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("i723\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG2, REG_8, 0);
        vIoWriteReg (GP_IO_REG3, REG_8, 0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }

	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_3 LO to HI\n");
#endif
	}

	/* GPIO_3 input, hi to lo interrupt */
	/* GPIO_4 input, no interrupt       */
	/* GPIO_5 output                    */
    vIoWriteReg (GP_IO_REG2, REG_8, 0);
    vIoWriteReg (GP_IO_REG3, REG_8, DIR_5);				/* set GPIO_5 as output       */
    vIoWriteReg (INT_CFG_REG1, REG_8, 0);            	/* clear all interrupt status */
    vIoWriteReg (INT_CFG_REG2, REG_8, 0);            	/* clear all interrupt status */
    bTmp = dIoReadReg (GP_IO_REG3, REG_8);
    vIoWriteReg (GP_IO_REG3, REG_8, bTmp | GPIO_5);		/* set GPIO_5 HI              */
    vIoWriteReg (GP_IO_REG2, REG_8, GPIO_INTR_3); 		/* enable GPIO_3 HI to LO interrupt */

    vDelay(10);

#ifdef DEBUG
    vConsoleWrite ("i823\n");
#endif

    bTmp = dIoReadReg (INT_CFG_REG2,REG_8);                /* read status */
    if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("i923\n");
#endif

    bTmp = dIoReadReg (GP_IO_REG3,REG_8);
    vIoWriteReg (GP_IO_REG3,REG_8,bTmp & ~GPIO_5);		/* trigger interrupt */
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("iA23\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CFG_REG2,REG_8);                /* read status */
        if ((bTmp & (GPIO3_TRANS | GPIO4_TRANS | GPIO5_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG2,REG_8, 0);
            vIoWriteReg (GP_IO_REG3,REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }

#ifdef DEBUG
        vConsoleWrite ("iB23\n");
#endif

        if ((bTmp & (GPIO4_TRANS | GPIO5_TRANS)) != 0)
        {
            vIoWriteReg (GP_IO_REG2,REG_8,0);
            vIoWriteReg (GP_IO_REG3,REG_8,0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG2,REG_8,0);
        vIoWriteReg (GP_IO_REG3,REG_8,0);
	   /* Mask and disable interrupt and uninstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }

#ifdef DEBUG
    vConsoleWrite ("iC23\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG2,REG_8,0);
        vIoWriteReg (GP_IO_REG3,REG_8,0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }


	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
		vConsoleWrite ("PASS: Got IRQ for GPIO_3 HI to LO\n");
#endif
	}

#ifdef DEBUG
    vConsoleWrite ("iD2\n");
#endif

	if (sysMaskPin (5, SYS_IOAPIC0) == -1)
 		return (E__MASK_SLOT);

	sysInstallUserHandler (iVector, 0);

    return E__OK;
}

/********************************************************************
 * brdGPIOTest
 *
 * This test will test the board specific GPIO lines.
 ********************************************************************/
TEST_INTERFACE (brdGPIOTest, "Board GPIO Test")
{
    UINT32  wTestStatus = E__OK;
	UINT8	bInstance = 0;

	if(adTestParams[0] != 0)
		bInstance = adTestParams[1];
	else
		++bInstance;

	
#ifdef DEBUG
	sprintf(buffer, "adTestParams: %d:%d, bInstance: %d\n", adTestParams[0], adTestParams[1], bInstance);
	vConsoleWrite(buffer); 
#endif

	if (bInstance > 2)
	{
#ifdef DEBUG
		sprintf(buffer, "Invalid GPIO group: %d\n", bInstance);
		vConsoleWrite(buffer); 
#endif
		wTestStatus = E__GPIO_GROUP;
	}
	
	if (wTestStatus == E__OK)
	{
		if (bInstance == 1)
		{
			wTestStatus = wLoopbackTest1 ();
    
			/* now test interrupts */
			if (wTestStatus == E__OK)
				wTestStatus = wInterruptTest1();
		}
	}

	if (wTestStatus == E__OK)
	{
		if (bInstance == 2)
		{
			wTestStatus = wLoopbackTest2 ();

			/* now test interrupts */
			if (wTestStatus == E__OK)
				wTestStatus = wInterruptTest2();
		}
	}
	
    return wTestStatus;
}
