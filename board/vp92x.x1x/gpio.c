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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp92x.x1x/gpio.c,v 1.1 2015-01-29 11:09:02 mgostling Exp $
 *
 * $Log: gpio.c,v $
 * Revision 1.1  2015-01-29 11:09:02  mgostling
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

//#define DEBUG
/* General Purpose IO Register */

#define GPIO_DIR_REG       0x310
#define GPIO_STATUS_REG       0x31C

#define GPIO0_DIR_DATA		(1)
#define GPIO1_DIR_DATA		(1<<1)
#define GPIO2_DIR_DATA		(1<<2)
#define GPIO3_DIR_DATA		(1<<3)
#define GPIO4_DIR_DATA		(1<<4)
#define GPIO5_DIR_DATA		(1<<5)
#define GPIO6_DIR_DATA		(1<<6)
#define GPIO7_DIR_DATA		(1<<7)

#if 0
#define GPIO0_DIR_SHIFT_BITS (0)
#define GPIO0_DIR_SHIFT_BITS (1)
#define GPIO0_DIR_SHIFT_BITS (2)
#define GPIO0_DIR_SHIFT_BITS (3)
#define GPIO0_DIR_SHIFT_BITS (4)
#define GPIO0_DIR_SHIFT_BITS (5)
#define GPIO0_DIR_SHIFT_BITS (6)
#define GPIO0_DIR_SHIFT_BITS (7)
#endif



#define  LH_EDGE_1      0x80    /* 0 = H->L generates interrupt, 1 = L->H */
#define  LH_EDGE_0      0x40    /*      -"- */
#define  GPIO_INTR_1    0x20    /* 0 = masked, 1 = interrupt enabled */
#define  GPIO_INTR_0    0x10    /*      -"- */
#define  GPI_1          0x08    /* 0 = GPIO is input, 1 = GPIO is output */
#define  GPI_0          0x04    /*      -"- */
#define  GPO_1          0x02    /* value read/driven via GPIO */
#define  GPO_0          0x01    /*      -"- */

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



/********************************************************************
 * wLoopbackTest
 *
 * Basic in/out loopback test for each GPIO pin
 ********************************************************************/
static UINT32 wLoopbackTest (void)
{
    UINT8   bTmp;
    char    achErrTxt [80];
	
	//Configure GPIO7 as output and all others  as input and test loopback
	vIoWriteReg (GPIO_DIR_REG,REG_8,0x80);
	vDelay(2);	
    bTmp = dIoReadReg (GPIO_STATUS_REG,REG_8);
	bTmp |= GPIO7_DIR_DATA;
	
	/*Write 1 to GPIO7 register*/
	vIoWriteReg(GPIO_STATUS_REG, REG_8,bTmp);
	vDelay(2);
	bTmp = dIoReadReg (GPIO_STATUS_REG,REG_8);
	vDelay(2);
	
	if(bTmp != 0xff)
	{
		sprintf (achErrTxt, "Expected bits '1' received '0' on all GPIOs\n");
        vConsoleWrite (achErrTxt);
		return E__ERR_1;
	}
	
	//Write 0 to GPIO 7 and check on other ports
	bTmp&= ~(GPIO7_DIR_DATA);
	vIoWriteReg(GPIO_STATUS_REG, REG_8,bTmp);
	vDelay(5);
	bTmp = dIoReadReg (GPIO_STATUS_REG,REG_8);
	if(bTmp != 0x00)
	{
		sprintf (achErrTxt, "Expected bits '0' received '1' on all GPIO\n");
        vConsoleWrite (achErrTxt);
		return E__ERR_2;
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
    return wTestStatus;
}
