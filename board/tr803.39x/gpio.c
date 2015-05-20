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

//#define DEBUG
/* General Purpose IO Register */

#define GP_IO_REG       0x213
#define  LH_EDGE_1      0x80    /* 0 = H->L generates interrupt, 1 = L->H */
#define  LH_EDGE_0      0x40    /*      -"- */
#define  GPIO_INTR_1    0x20    /* 0 = masked, 1 = interrupt enabled */
#define  GPIO_INTR_0    0x10    /*      -"- */
#define  DIR_1          0x08    /* 0 = GPIO is input, 1 = GPIO is output */
#define  DIR_0          0x04    /*      -"- */
#define  GPIO_1         0x02    /* value read/driven via GPIO */
#define  GPIO_0         0x01    /*      -"- */

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

static UINT8 bIrqStatus;        /* NONE, PASS or FAIL */


static void vIrqPassHandler (void);

/********************************************************************
 * vClearInt
 *
 * Clear the hardware interrupt enable and status bits
 ********************************************************************/
static void vClearInt (void)
{
    vIoWriteReg (GP_IO_REG, REG_8,0);
    vIoWriteReg (INT_CONFIG_REG, REG_8,0);
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
 B
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
 * wLoopbackTest
 *
 * Basic in/out loopback test for each GPIO pin
 ********************************************************************/
static UINT32 wLoopbackTest (void)
{
    UINT8   bTmp;
    char    achErrTxt [80];

    /* just loop back output to input */

    /* make GPIO0 output and GPIO1 input & no interrupts */

    vIoWriteReg (GP_IO_REG,REG_8,DIR_0);
    vDelay(5);
    bTmp = dIoReadReg (GP_IO_REG,REG_8);

    if ((bTmp & GPIO_1) != 0)
    {
        sprintf (achErrTxt, "Expected bit 0 LO read HI\n");
	    vConsoleWrite (achErrTxt);
        return E__ERR_0;
    }

#ifdef DEBUG
    vConsoleWrite ("1\n");
#endif

    /* output a 1 to gpio 0 */

    vIoWriteReg (GP_IO_REG,REG_8,bTmp | GPIO_0);
    vDelay(5);
    bTmp = dIoReadReg (GP_IO_REG,REG_8);

    if ((bTmp & GPIO_1) == 0)
    {
        sprintf (achErrTxt, "Expected bit 0 HI read LO\n");
        vConsoleWrite (achErrTxt);
        return E__ERR_1;
    }

 /* now make GPIO 1 an output and GPIO 0 an input */

#ifdef DEBUG
    vConsoleWrite ("2\n");
#endif

    vIoWriteReg (GP_IO_REG,REG_8, DIR_1);
    vDelay(5);
    bTmp = dIoReadReg (GP_IO_REG,REG_8);

    if ((bTmp & GPIO_0) != 0)
    {
        sprintf (achErrTxt, "Expected bit 1 LO read HI\n");
        vConsoleWrite (achErrTxt);
        return E__ERR_2;
    }

    /* output a 1 to gpio 1 */

#ifdef DEBUG
    vConsoleWrite ("3\n");
#endif

    vIoWriteReg (GP_IO_REG,REG_8,bTmp | GPIO_1);
    vDelay(5);
    bTmp = dIoReadReg (GP_IO_REG,REG_8);

    if ((bTmp & GPIO_0) == 0)
    {
        sprintf (achErrTxt, "Expected bit 1 HI read LO\n");
        vConsoleWrite (achErrTxt);
        return E__ERR_3;
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
    UINT8   bTmp;
	int		iVector= -1;
    char    achErrTxt [80];

#ifdef DEBUG
    vConsoleWrite ("4\n");
#endif

#ifdef DEBUG
    vConsoleWrite ("41\n");
#endif

    bIrqStatus = IRQ_TEST_NONE;
#ifdef DEBUG
    vConsoleWrite ("42\n");
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
    vConsoleWrite ("43\n");
#endif


#ifdef DEBUG
    vConsoleWrite ("5\n");
#endif

    bTmp = DIR_0 | LH_EDGE_1;   /* GPIO 0 o/p & GPIO 1 lo to hi interrupt */
    vIoWriteReg (GP_IO_REG,REG_8, bTmp);
    vIoWriteReg (INT_CONFIG_REG,REG_8, 0);            /* clear all interrupt status */
    vIoWriteReg (GP_IO_REG, REG_8,bTmp | GPIO_INTR_1);/* enable the interrupt */
#ifdef DEBUG
    vConsoleWrite ("6\n");
#endif

    vDelay(10);
#ifdef DEBUG
    vConsoleWrite ("7\n");
#endif

    bTmp = dIoReadReg (INT_CONFIG_REG,REG_8);                /* read status */
    if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS)) != 0)
    {
        sprintf (achErrTxt, "Interrupt status set (0x%x) when no interrupt generated\n", bTmp);
        vConsoleWrite (achErrTxt);
   	    /* Mask and disable interrupt and unnstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
			return (E__MASK_SLOT);

		sysInstallUserHandler (iVector, 0);
        return E__INT_0;
    }

#ifdef DEBUG
    vConsoleWrite ("8\n");
#endif

    vIoWriteReg (GP_IO_REG,REG_8,dIoReadReg (GP_IO_REG,REG_8) | GPIO_0);
    vDelay(10);

    /* check status */

#ifdef DEBUG
    vConsoleWrite ("9\n");
#endif

    if (bIrqStatus == IRQ_TEST_NONE)
    {
        vConsoleWrite ("No interrupt\n");
        bTmp = dIoReadReg(INT_CONFIG_REG,REG_8);                /* read status */
        if ((bTmp & (GPIO0_TRANS | GPIO1_TRANS)) == 0)
        {
            vIoWriteReg (GP_IO_REG,REG_8, 0);
            sprintf (achErrTxt, "Interrupt status not set (0x%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
#ifdef DEBUG
    vConsoleWrite ("91\n");
#endif

        if ((bTmp & GPIO0_TRANS) != 0)
        {
            vIoWriteReg (GP_IO_REG,REG_8,0);
            sprintf (achErrTxt, "Wrong Interrupt status set (%x) when interrupt generated\n", bTmp);
            vConsoleWrite (achErrTxt);
        }
        vIoWriteReg (GP_IO_REG,REG_8,0);
	   /* Mask and disable interrupt and unnstall our interrupt handler */
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);
		sysInstallUserHandler (iVector, 0);
        return E__INT_1;
    }
#ifdef DEBUG    
    vConsoleWrite ("A\n");
#endif

    if (bIrqStatus == IRQ_TEST_FAIL)
    {
        vIoWriteReg (GP_IO_REG,REG_8,0);
        vConsoleWrite ("Interrupt on wrong IRQ\n");
		if (sysMaskPin (5, SYS_IOAPIC0) == -1)
	 		return (E__MASK_SLOT);
		sysInstallUserHandler (iVector, 0);
        return E__INT_2;
    }

	if (bIrqStatus == IRQ_TEST_PASS)
	{
#ifdef DEBUG
    vConsoleWrite ("PASS: Got IRQ on PIN 5\n");
#endif
	}

#ifdef DEBUG
    vConsoleWrite ("B\n");
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
    UINT32  wTestStatus = 10;

    /* just loop back output to input */
   wTestStatus = wLoopbackTest ();
   /* now test interrupts */
   if (wTestStatus == E__OK)
    wTestStatus = wInterruptTest();
    return wTestStatus;

}
