/************************************************************************
 *                                                                      *
 *      Copyright 2012 Concurrent Technologies, all rights reserved.    *
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

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/tpm_tests.c,v 1.3 2014-03-03 16:31:52 mgostling Exp $
 *
 * $Log: tpm_tests.c,v $
 * Revision 1.3  2014-03-03 16:31:52  mgostling
 * Add suppport for AtmelTPM
 *
 * Revision 1.2  2013-11-25 13:01:18  mgostling
 * Added support for TRB1x.
 *
 * Revision 1.1  2013-09-04 07:46:45  chippisley
 * Import files into new source repository.
 *
 * Revision 1.2  2012/03/21 15:58:56  cdobson
 * Eliminate compiler warnings.
 *
 * Revision 1.1  2012/02/17 11:34:32  hchalla
 * Added initial version of TPM module test for PP 81x and PP 91x.
 *
 *
 */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>	
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
 
#include <bit/board_service.h>
#include <bit/delay.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>

#include <devices/tpm_test.h>

//#define DEBUG

char achBuffer[80];
static struct tpm_inf_dev tpmDev;

static UINT32 tpm_inf_init(void);
/* static UINT32 tpm_inf_suspend(void); */
/* static UINT32 tpm_inf_resume(void); */
/* static UINT8 tpm_inf_status(void); */
static UINT32 tpm_inf_send(UINT8 * buf, size_t count);
static UINT32 tpm_inf_recv(UINT8 * buf, size_t count);
static void tpm_wtx_abort(void);
static void tpm_wtx(void);
static void wait_and_send(UINT8 sendbyte);
static UINT32 wait(UINT32 wait_for_bit);
static UINT32 empty_fifo(UINT32 clear_wrfifo);
static  UINT8 tpmConfigIn(UINT8 offset);
static  void tpmConfigOut(UINT8 bData, UINT8 offset);
static  UINT8 tpmDataIn(UINT8 offset);
static  void tpmDataOut(UINT8 bData, UINT8 offset);
static UINT32 tpm_inf_self_test_full(void);
/* static UINT32 tpm_inf_startup(void); */
/* static UINT32 tpm_inf_continue_self_test(void); */
static UINT32 disable_lt_mem_ownership(void);


/*****************************************************************************\
 *
 *  TITLE:  tpmDataOut ()
 *
 *  ABSTRACT:  This function write into the data tpm io-port register at given 
 *             offset.
 *			
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
static  void tpmDataOut(UINT8 bData, UINT8 offset)
{
	vIoWriteReg(tpmDev.dDataRegs+offset, REG_8, bData);
}


/*****************************************************************************\
 *
 *  TITLE:  tpmDataOut ()
 *
 *  ABSTRACT:  This function read from the tpm io-port register at given offset.
 *			
 *
 * 	RETURNS: IO port register data value
 *
\*****************************************************************************/
static  UINT8 tpmDataIn(UINT8 offset)
{
	return (((UINT8)dIoReadReg(tpmDev.dDataRegs + offset, REG_8)));
}


/*****************************************************************************\
 *
 *  TITLE: tpmConfigOut ()
 *
 *  ABSTRACT:  This function writes into the tpm config IO port at given offset.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/

static  void tpmConfigOut(UINT8 bData, UINT8 offset)
{
	vIoWriteReg(tpmDev.dConfigPort+offset, REG_8, bData);
}


/*****************************************************************************\
 *
 *  TITLE: tpmConfigIn ()
 *
 *  ABSTRACT:  This function read from the tpm config IO port at given offset.
 *
 * 	RETURNS: IO Port config data
 *
\*****************************************************************************/

static  UINT8 tpmConfigIn(UINT8 offset)
{
	return (((UINT8)dIoReadReg(tpmDev.dConfigPort + offset, REG_8)));
}


static UINT32 number_of_wtx;



/*****************************************************************************\
 *
 *  TITLE: empty_fifo ()
 *
 *  ABSTRACT:  This function empties the fifo.
 *
 * 	RETURNS: Success or Error
 *
\*****************************************************************************/

static UINT32 empty_fifo(UINT32 clear_wrfifo)
{
        UINT32 status;
        UINT32 check = 0;
        UINT32 i;

        if (clear_wrfifo)
        {
                for (i = 0; i < 4096; i++)
                {
                        status = tpmDataIn(WRFIFO);
                        if (status == 0xff)
                        {
                                if (check == 5)
                                        break;
                                else
                                        check++;
                        }
                }
        }
        /* Note: The values which are currently in the FIFO of the TPM
           are thrown away since there is no usage for them. Usually,
           this has nothing to say, since the TPM will give its answer
           immediately or will be aborted anyway, so the bData here is
           usually garbage and useless.
           We have to clean this, because the next communication with
           the TPM would be rubbish, if there is still some old bData
           in the Read FIFO.
         */
        i = 0;
        do
        {
                status = tpmDataIn(RDFIFO);
                status = tpmDataIn(STAT);
                i++;
                if (i == TPM_MAX_TRIES)
                        return E_TPM_IO_PORT;
        } while ((status & (1 << STAT_RDA)) != 0);

        return E__OK;
}

/*****************************************************************************\
 *
 *  TITLE: wait ()
 *
 *  ABSTRACT:  This function waits for a particular bit to get set.
 *
 * 	RETURNS: Success or Error
 *
\*****************************************************************************/


static UINT32 wait(UINT32 wait_for_bit)
{
        UINT32 status;
        UINT32 i;
        for (i = 0; i < TPM_MAX_TRIES; i++)
        {
                status = tpmDataIn(STAT);
                /* check the status-register if wait_for_bit is set */
                if (status & 1 << wait_for_bit)
                        break;
                vDelay(TPM_MSLEEP_TIME);
        }

        if (i == TPM_MAX_TRIES)
        {       /* timeout occurs */
                if (wait_for_bit == STAT_XFE)
				{
                        sprintf(achBuffer, "Timeout in wait(STAT_XFE)\n");
						vConsoleWrite(achBuffer);
				}
                if (wait_for_bit == STAT_RDA)
				{
                        sprintf(achBuffer, "Timeout in wait(STAT_RDA)\n");
						vConsoleWrite(achBuffer);
				}
                return E_TPM_IO_PORT;
        }
        return E__OK;
};



/*****************************************************************************\
 *
 *  TITLE: wait_and_send ()
 *
 *  ABSTRACT:  This function waits and then writes to the TPM WR FIFO.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/

static void wait_and_send(UINT8 sendbyte)
{
        wait(STAT_XFE);
        tpmDataOut(sendbyte, WRFIFO);
}

    /* Note: WTX means Waiting-Time-Extension. Whenever the TPM needs more
       calculation time, it sends a WTX-package, which has to be acknowledged
       or aborted. This usually occurs if you are hammering the TPM with key
       creation. Set the maximum number of WTX-packages in the definitions
       above, if the number is reached, the waiting-time will be denied
       and the TPM command has to be resend.
     */


/*****************************************************************************\
 *
 *  TITLE: tpm_wtx ()
 *
 *  ABSTRACT:  This function Waiting Time Extension, When TPM needs more calculation 
 *             time it sends a WTX Package.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
static void tpm_wtx(void)
{
        number_of_wtx++;
        sprintf(achBuffer, "Granting WTX (%02d / %02d)\n",
                 number_of_wtx, TPM_MAX_WTX_PACKAGES);
		vConsoleWrite(achBuffer);
        wait_and_send(TPM_VL_VER);
        wait_and_send(TPM_CTRL_WTX);
        wait_and_send(0x00);
        wait_and_send(0x00);
        vDelay(TPM_WTX_MSLEEP_TIME);
}


/*****************************************************************************\
 *
 *  TITLE: tpm_wtx_abort ()
 *
 *  ABSTRACT:  This function aborts the WTX Package.
 *             
 *
 * 	RETURNS: None
 *
\*****************************************************************************/

static void tpm_wtx_abort(void)
{
        sprintf(achBuffer, "Aborting WTX\n");
		vConsoleWrite(achBuffer);
        wait_and_send(TPM_VL_VER);
        wait_and_send(TPM_CTRL_WTX_ABORT);
        wait_and_send(0x00);
        wait_and_send(0x00);
        number_of_wtx = 0;
        vDelay(TPM_WTX_MSLEEP_TIME);
}


/*****************************************************************************\
 *
 *  TITLE: tpm_inf_recv ()
 *
 *  ABSTRACT:  This function recieves data from the tpm interface.
 *             
 *
 * 	RETURNS: Size of data or Error.
 *
\*****************************************************************************/
static UINT32 tpm_inf_recv(UINT8 * buf, size_t count)
{
        UINT32 i;
        UINT32 ret;
        UINT32 size = 0;
        number_of_wtx = 0;

recv_begin:
        /* start receiving header */
        for (i = 0; i < 4; i++)
        {
                ret = wait(STAT_RDA);
                if (ret)
                        return E_TPM_IO_PORT;
                buf[i] = tpmDataIn(RDFIFO);
        }

        if (buf[0] != TPM_VL_VER)
        {
                sprintf(achBuffer,
                        "Wrong transport protocol implementation!\n");
				vConsoleWrite(achBuffer);
                return E_TPM_IO_PORT;
		}

        if (buf[1] == TPM_CTRL_DATA)
        {
                /* size of the bData received */
                size = ((buf[2] << 8) | buf[3]);

                for (i = 0; i < size; i++)
                {
                        wait(STAT_RDA);
                        buf[i] = tpmDataIn(RDFIFO);
                }

                if ((size == 0x6D00) && (buf[1] == 0x80))
                {
                        sprintf(achBuffer, "Error handling on vendor layer!\n");
						vConsoleWrite(achBuffer);
                        return E_TPM_IO_PORT;
                }

                for (i = 0; i < size; i++)
                        buf[i] = buf[i + 6];

                size = size - 6;
                return size;
        }

        if (buf[1] == TPM_CTRL_WTX)
        {
                sprintf(achBuffer, "WTX-package received\n");
						vConsoleWrite(achBuffer);
                if (number_of_wtx < TPM_MAX_WTX_PACKAGES)
                {
                        tpm_wtx();
                        goto recv_begin;
                }
                else
                {
                        tpm_wtx_abort();
                        goto recv_begin;
                }
        }

        if (buf[1] == TPM_CTRL_WTX_ABORT_ACK)
        {
                sprintf(achBuffer, "WTX-abort acknowledged\n");
						vConsoleWrite(achBuffer);
                return size;
        }

        if (buf[1] == TPM_CTRL_ERROR)
        {
                sprintf(achBuffer, "ERROR-package received:\n");
				vConsoleWrite(achBuffer);
                if (buf[4] == TPM_INF_NAK)
                        sprintf(achBuffer, "-> Negative acknowledgement"
                                " - retransmit command!\n");
						vConsoleWrite(achBuffer);
                return E_TPM_IO_PORT;
        }
        return E_TPM_IO_PORT;
}



/*****************************************************************************\
 *
 *  TITLE: tpm_inf_send ()
 *
 *  ABSTRACT:  This function sends a package on to TPM interface
 *             
 *
 * 	RETURNS: Data count or Error.
 *
\*****************************************************************************/
static UINT32 tpm_inf_send(UINT8 * buf, size_t count)
{
        UINT32 i;
        UINT32 ret;
        UINT8 count_high, count_low, count_4, count_3, count_2, count_1;

        /* Disabling Reset, LP and IRQC */
        tpmDataOut(RESET_LP_IRQC_DISABLE, CMD);

        ret = empty_fifo(1);
        if (ret)
        {
                sprintf(achBuffer, "Timeout while clearing FIFO\n");
				vConsoleWrite(achBuffer);
                return E_TPM_IO_PORT;
        }

        ret = wait(STAT_XFE);
        if (ret)
                return E_TPM_IO_PORT;

        count_4 = (count & 0xff000000) >> 24;
        count_3 = (count & 0x00ff0000) >> 16;
        count_2 = (count & 0x0000ff00) >> 8;
        count_1 = (count & 0x000000ff);
        count_high = ((count + 6) & 0xffffff00) >> 8;
        count_low = ((count + 6) & 0x000000ff);

        /* Sending Header */
        wait_and_send(TPM_VL_VER);
        wait_and_send(TPM_CTRL_DATA);
        wait_and_send(count_high);
        wait_and_send(count_low);

        /* Sending Data Header */
        wait_and_send(TPM_VL_VER);
        wait_and_send(TPM_VL_CHANNEL_TPM);
        wait_and_send(count_4);
        wait_and_send(count_3);
        wait_and_send(count_2);
        wait_and_send(count_1);

        /* Sending Data */
        for (i = 0; i < count; i++)
        {
                wait_and_send(buf[i]);
        }
        return count;
}

#if 0
/*****************************************************************************\
 *
 *  TITLE: tpm_inf_status ()
 *
 *  ABSTRACT:  This function reads the TPM interface status.
 *             
 *
 * 	RETURNS:  Register Status of the tpm.
 *
\*****************************************************************************/
static UINT8 tpm_inf_status(void)
{
        return tpmDataIn(STAT);
}
#endif


/*****************************************************************************\
 *
 *  TITLE: tpm_inf_init ()
 *
 *  ABSTRACT:  This function detects and initializes the TPM interface.
 *             
 *
 * 	RETURNS: E__OK or Error codes.
 *
\*****************************************************************************/
static UINT32 tpm_inf_init(void)
{
        UINT32 rc = 0;
        UINT8 iol, ioh;
        UINT32 vendorid[2];
        UINT32 version[2];
        UINT32 productid[2];
        char chipname[20];
        UINT16 wIoBaseReg = 0;
        UINT8 bTemp = 0,bTemp1=0;

       /*
        * The only known MMIO based Infineon TPM system provides
        * a single large mem region with the device config
        * registers at the default TPM_ADDR.  The bData registers
        * seem like they could be placed anywhere within the MMIO
        * region, but lets just put them at zero offset.
        */

        tpmDev.dConfigPort = TPM_ADDR;
        tpmDev.iotype = TPM_INF_IO_PORT;


        /* query chip for its vendor, its version number a.s.o. */
        tpmConfigOut(ENABLE_REGISTER_PAIR, TPM_INF_ADDR);
        tpmConfigOut(IDVENL, TPM_INF_ADDR);
        vendorid[1] = tpmConfigIn(TPM_INF_DATA);
        tpmConfigOut(IDVENH, TPM_INF_ADDR);
        vendorid[0] = tpmConfigIn(TPM_INF_DATA);
        tpmConfigOut(IDPDL, TPM_INF_ADDR);
        productid[1] = tpmConfigIn(TPM_INF_DATA);
        tpmConfigOut(IDPDH, TPM_INF_ADDR);
        productid[0] = tpmConfigIn(TPM_INF_DATA);
        tpmConfigOut(CHIP_ID1, TPM_INF_ADDR);
        version[1] = tpmConfigIn(TPM_INF_DATA);
        tpmConfigOut(CHIP_ID2, TPM_INF_ADDR);
        version[0] = tpmConfigIn(TPM_INF_DATA);

        tpmConfigOut(IOLIML, TPM_INF_ADDR);
        bTemp =  tpmConfigIn(TPM_INF_DATA);
        tpmConfigOut(IOLIMH, TPM_INF_ADDR);
        bTemp1 = tpmConfigIn(TPM_INF_DATA);
        wIoBaseReg = ((bTemp1 << 8)| (bTemp));
        tpmDev.dDataRegs = wIoBaseReg;

#ifdef DEBUG
        sprintf(achBuffer, "tpm_inf_init:\nIOLIML: %#x\nIOLIMH: %#x\n",bTemp, bTemp1);
        vConsoleWrite(achBuffer);

        sprintf(achBuffer, "productid[0]:%#x\n",productid[0]);
        vConsoleWrite(achBuffer);

        sprintf(achBuffer, "productid[1]:%#x\n",productid[1]);
        vConsoleWrite(achBuffer);

        sprintf(achBuffer, "version[0]:%#x\n",version[0]);
        vConsoleWrite(achBuffer);

        sprintf(achBuffer, "version[1]:%#x\n",version[1]);
        vConsoleWrite(achBuffer);
#endif
        switch ((productid[0] << 8) | productid[1])
        {
        	case 6:
        	    sprintf(chipname, "(SLD 9630 TT 1.1)\n");
        		break;

			case 11:
				sprintf(chipname, "(SLB 9635 TT 1.2)\n");
				break;
        
			default:
            	sprintf(chipname, "(No Chip Found)\n");
            	vConsoleWrite(chipname);
            	break;
        }

        if ((vendorid[0] << 8 | vendorid[1]) == TPM_INFINEON_VENDOR_ID)
        {

            /* configure TPM with IO-ports */
            tpmConfigOut(IOLIMH, TPM_INF_ADDR);
            tpmConfigOut((tpmDev.dDataRegs >> 8) & 0xff, TPM_INF_DATA);
            tpmConfigOut(IOLIML, TPM_INF_ADDR);
            tpmConfigOut((tpmDev.dDataRegs & 0xff), TPM_INF_DATA);

            /* control if IO-ports are set correctly */
            tpmConfigOut(IOLIMH, TPM_INF_ADDR);
            ioh = tpmConfigIn(TPM_INF_DATA);
            tpmConfigOut(IOLIML, TPM_INF_ADDR);
            iol = tpmConfigIn(TPM_INF_DATA);

            if ((ioh << 8 | iol) != tpmDev.dDataRegs)
            {
                sprintf(achBuffer,
                        "\nCould not set IO-bData registers to 0x%x\n",
                        tpmDev.dDataRegs);
				vConsoleWrite(achBuffer);
                rc = E_TPM_IO_PORT;
                goto err_release_region;
            }

            /* activate register */
            tpmConfigOut(TPM_DAR, TPM_INF_ADDR);
            tpmConfigOut(0x01, TPM_INF_DATA);
            tpmConfigOut(DISABLE_REGISTER_PAIR, TPM_INF_ADDR);

            /* disable RESET, LP and IRQC */
            tpmDataOut(RESET_LP_IRQC_DISABLE, CMD);

            /* Finally, we're done, print some infos */
            sprintf(achBuffer, "TPM found: \n"
                     "Config Base     :0x%x\n"
                     "IO  Base        :0x%x\n"
                     "Chip Version    :0x%02x%02x,\n"
                     "Vendor  ID      :0x%x%x (Infineon)\n"
                     "Product ID      :0x%02x%02x\n"
                     "Chip Name       :%s\n",
                     tpmDev.dConfigPort,
                     tpmDev.dDataRegs,
                     version[0], version[1],
                     vendorid[0], vendorid[1],
                     productid[0], productid[1], chipname);
			
			vConsoleWrite(achBuffer);

            return E__OK;
        } 
		else 
		{
            rc = E_TPM_NO_DEVICE;
            goto err_release_region;
        }

err_release_region:
        return rc;
}

#if 0
/*****************************************************************************\
 *
 *  TITLE: tpm_inf_suspend ()
 *
 *  ABSTRACT:  This function sends a suspend command to TPM interface.
 *             
 *
 * 	RETURNS: E__OK or Error codes.
 *
\*****************************************************************************/
static UINT32 tpm_inf_suspend(void)
{
        UINT32 rc = 0;

        {
                UINT8 savestate[] =
                {
                        0, 193, 		/* TPM_TAG_RQU_COMMAND */
                        0, 0, 0, 10,    /* blob length (in bytes) */
                        0, 0, 0, 152    /* TPM_ORD_SaveState */
                };
                sprintf(achBuffer, "saving TPM state\n");
				vConsoleWrite(achBuffer);
                rc = tpm_inf_send(savestate, sizeof(savestate));
                if (rc < 0)
                {
                        sprintf(achBuffer, "error while saving TPM state\n");
						vConsoleWrite(achBuffer);
                        return rc;
                }
        }
        return E__OK;
}
#endif

/*****************************************************************************\
 *
 *  TITLE: tpm_inf_self_test_full ()
 *
 *  ABSTRACT:  This function sends a self test command to TPM interface
 *             
 *
 * 	RETURNS: E__OK or Error codes.
 *
\*****************************************************************************/
static UINT32 tpm_inf_self_test_full(void)
{
        UINT32 rc = 0;
#ifdef DEBUG
	UINT32 ii=0;
#endif
        UINT8 result[10];
        UINT16 tpmTagRspCmd = 0;
        UINT32 tpmResult = 0;

        {
                UINT8 selftest[] =
                {
                        0, 193, 		/* TPM_TAG_RQU_COMMAND */
                        0, 0, 0, 10,    /* blob length (in bytes) */
                        0, 0, 0, 80    	/* TPM_ORD_SelfTestFull */
                };
                sprintf(achBuffer, "TPM SelfTestFull\n");
				vConsoleWrite(achBuffer);
                rc = tpm_inf_send(selftest, sizeof(selftest));
                if (rc < 0)
                {
                        sprintf(achBuffer, "error while TPM send\n");
						vConsoleWrite(achBuffer);
                        return rc;
                }

                vDelay(20);
                memset(result,0,sizeof(result));
                rc = tpm_inf_recv(result, sizeof(result));
                if (rc < 0)
                {
					sprintf(achBuffer, "error while TPM recv\n");
					vConsoleWrite(achBuffer);
					return rc;
                }

#ifdef DEBUG
                for (ii=0;ii<14;ii++)
                {
                	sprintf(achBuffer, "result[%d]:%#x\n",ii+1,result[ii]);
                	vConsoleWrite(achBuffer);
                }
                vConsoleWrite("\n");
#endif
                tpmTagRspCmd = ((result[0] << 8) | (result[1]));
                sprintf(achBuffer, "tpmTagRspCmd: %#x\n",tpmTagRspCmd);
                vConsoleWrite(achBuffer);
                if (tpmTagRspCmd != 0x00C4)
                {
                	return (E__WRONG_RSP_CMD);
                }

                tpmResult = ((result[6] << 24) | (result[7] << 16) | (result[8] << 8) | (result[9])) ;
                sprintf(achBuffer, "tpmResult: %#x\n",tpmResult);
                vConsoleWrite(achBuffer);
                if (tpmResult != E__OK)
                {
                	 tpmResult = E__TPM_SELFTEST_FAIL;
                	 return (tpmResult);
                }
        }
        return E__OK;
}

#if 0
/*****************************************************************************\
 *
 *  TITLE: tpm_inf_resume ()
 *
 *  ABSTRACT:  This function sends a resume command to TPM Inteferface.
 *             
 *
 * 	RETURNS: E__OK or Error codes.
 *
\*****************************************************************************/
static UINT32 tpm_inf_resume(void)
{
        /* Re-configure TPM after suspending */
        tpmConfigOut(ENABLE_REGISTER_PAIR, TPM_INF_ADDR);
        tpmConfigOut(IOLIMH, TPM_INF_ADDR);
        tpmConfigOut((tpmDev.dDataRegs >> 8) & 0xff, TPM_INF_DATA);
        tpmConfigOut(IOLIML, TPM_INF_ADDR);
        tpmConfigOut((tpmDev.dDataRegs & 0xff), TPM_INF_DATA);
        /* activate register */
        tpmConfigOut(TPM_DAR, TPM_INF_ADDR);
        tpmConfigOut(0x01, TPM_INF_DATA);
        tpmConfigOut(DISABLE_REGISTER_PAIR, TPM_INF_ADDR);
        /* disable RESET, LP and IRQC */
        tpmDataOut(RESET_LP_IRQC_DISABLE, CMD);
        return E__OK;
}

/*****************************************************************************\
 *
 *  TITLE: tpm_inf_continue_self_test ()
 *
 *  ABSTRACT:  This function sends a ContinueSelfTest command to TPM Interface.
 *             
 *
 * 	RETURNS: E__OK or Error codes.
 *
\*****************************************************************************/
static UINT32 tpm_inf_continue_self_test(void)
{
        UINT32 rc = 0;
#ifdef DEBUG
	UINT32 ii=0;
#endif
        UINT8 result[10];
        UINT16 tpmTagRspCmd = 0;
        UINT32 tpmResult = 0;

        {
                UINT8 startup[] =
                {
                        0, 193, 		/* TPM_TAG_RQU_COMMAND */
                        0, 0, 0, 10,    /* blob length (in bytes) */
                        0, 0, 0, 83     /* TPM_ORD_ContinueSelfTest */
                };
                sprintf(achBuffer, "TPM ContinueSelfTest\n");
				vConsoleWrite(achBuffer);
                rc = tpm_inf_send(startup, sizeof(startup));
                if (rc < 0)
                {
                        sprintf(achBuffer, "error while TPM send\n");
						vConsoleWrite(achBuffer);
                        return rc;
                }

                vDelay(25);
                memset(result,0,sizeof(result));
                rc = tpm_inf_recv(result, sizeof(result));
                if (rc < 0)
                {
					sprintf(achBuffer, "error while TPM recv\n");
					vConsoleWrite(achBuffer);
					return rc;
                }

#ifdef DEBUG
                for (ii=0;ii<10;ii++)
                {
                	sprintf(achBuffer, "result[%d]:%#x\n",ii+1,result[ii]);
                	vConsoleWrite(achBuffer);
                }
                vConsoleWrite("\n");
#endif
                tpmTagRspCmd = ((result[0] << 8) | (result[1]));
            	sprintf(achBuffer, "tpmTagRspCmd: %#x\n",tpmTagRspCmd);
                 vConsoleWrite(achBuffer);
                if (tpmTagRspCmd != 0x00C4)
                {
                          	return (E__WRONG_RSP_CMD);
                 }

                 tpmResult = ((result[6] << 24) | (result[7] << 16) | (result[8] << 8) | (result[9])) ;
                 sprintf(achBuffer, "tpmResult: %#x\n",tpmResult);
                 vConsoleWrite(achBuffer);
                  if (tpmResult != E__OK)
                  {
                       	 tpmResult = E__TPM_STARTUP_FAIL;
                       	 return (tpmResult);
                   }
        }
        return E__OK;
}

/*****************************************************************************\
 *
 *  TITLE: tpm_inf_startup ()
 *
 *  ABSTRACT:  This function sends a startup command to TPM Interface.
 *             
 *
 * 	RETURNS: E__OK or Error codes.
 *
\*****************************************************************************/
static UINT32 tpm_inf_startup(void)
{
        UINT32 rc = 0;
#ifdef DEBUG
	UINT32 ii=0;
#endif
        UINT8 result[10];
        UINT16 tpmTagRspCmd = 0;
        UINT32 tpmResult = 0;

        {
                UINT8 startup[] =
                {
                        0, 193, 		/* TPM_TAG_RQU_COMMAND */
                        0, 0, 0, 12,    /* blob length (in bytes) */
                        0, 0, 0, 153,   /* TPM_ORD_StartUp */
                        0, 1			/* startup type */
                };
                sprintf(achBuffer, "TPM Startup\n");
				vConsoleWrite(achBuffer);
                rc = tpm_inf_send(startup, sizeof(startup));
                if (rc < 0)
                {
                        sprintf(achBuffer, "error while TPM send\n");
						vConsoleWrite(achBuffer);
                        return rc;
                }

                vDelay(25);
                memset(result,0,sizeof(result));
                rc = tpm_inf_recv(result, sizeof(result));
                if (rc < 0)
                {
					sprintf(achBuffer, "error while TPM recv\n");
					vConsoleWrite(achBuffer);
					return rc;
                }

#ifdef DEBUG
                for (ii=0;ii<10;ii++)
                {
                	sprintf(achBuffer, "result[%d]:%#x\n",ii+1,result[ii]);
                	vConsoleWrite(achBuffer);
                }
                vConsoleWrite("\n");
#endif
                tpmTagRspCmd = ((result[0] << 8) | (result[1]));
            	sprintf(achBuffer, "tpmTagRspCmd: %#x\n",tpmTagRspCmd);
                 vConsoleWrite(achBuffer);
                if (tpmTagRspCmd != 0x00C4)
                {
                          	return (E__WRONG_RSP_CMD);
                 }

                 tpmResult = ((result[6] << 24) | (result[7] << 16) | (result[8] << 8) | (result[9])) ;
                 sprintf(achBuffer, "tpmResult: %#x\n",tpmResult);
                 vConsoleWrite(achBuffer);
                  if (tpmResult != E__OK)
                  {
                       	 tpmResult = E__TPM_STARTUP_FAIL;
                       	 return (tpmResult);
                   }
        }
        return E__OK;
}
#endif


/*****************************************************************************\
 *
 *  TITLE: disable_lt_mem_ownership ()
 *
 *  ABSTRACT:  This function disable the LT memory owner ship.
 *             
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
static UINT32 disable_lt_mem_ownership(void)
{
	PTR48 tPtr1;
	volatile UINT32 mapRegSpace=0;
	UINT32 mHandle=0;
	UINT32 dVenDevId=0;
	UINT32 dTemp=0;

	mHandle = dGetPhysPtr(TPM_LT_MEM_SPACE0,0x1000,&tPtr1,(void*)&mapRegSpace);

	dVenDevId = osReadDWord(mapRegSpace+0x0F00);
	dTemp = dVenDevId & 0x0000FFFFL;

	if ((dTemp == TPM_INFINEON_VENDOR_ID) || (dTemp == TPM_ATMEL_DEVICE_ID))
	{
#ifdef DEBUG
		sprintf(achBuffer, "Found TPM Memory\n");
		vConsoleWrite(achBuffer);
#endif
	}

	if (dTemp == TPM_INFINEON_VENDOR_ID)
	{
		dTemp = osReadDWord(mapRegSpace+0x0000);
		dTemp &= 0x000000FF;
    
		if (dTemp == 0x000000A1)						// interface active and ready
		{
			osWriteDWord(mapRegSpace+0x0000,0x20);		// release this interface
		}
	}
	
	vFreePtr(mHandle);

	return dVenDevId;
}

/*****************************************************************************\
 *
 *  TITLE: TPMTest ()
 *
 *  ABSTRACT:  This function will detects and initialize the TPM Interface on SuperIO
 *			    and performs the TPM Selftest.
 *
 * 	RETURNS: Returns E__OK or error.
 *
\*****************************************************************************/
TEST_INTERFACE (TPMTest, "TPM Interface Test")
{
	UINT32 rc = E__OK;
	UINT32 dVenDevId;

	board_service (SERVICE__BRD_DECODE_TPM_LPC_IOWINDOW, NULL, NULL);

	// this needs to be done first in order to get access to legacy I/O
	dVenDevId = disable_lt_mem_ownership();

	// For ATMEL device, only need to check for its presence 
	if (dVenDevId == TPM_ATMEL_VEN_DEV_ID)
		return E__OK;

	rc = tpm_inf_init();

	if (rc == E__OK)
		rc = tpm_inf_self_test_full ();

	if (rc == E__OK)
	{
		vConsoleWrite("TPM SelfTest Full: PASS\n");
	}
	else
	{
		vConsoleWrite("TPM SelfTest Full: FAIL\n");
	}

	return rc;
}

