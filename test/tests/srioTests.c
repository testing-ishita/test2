/************************************************************************
 *                                                                      *
 *      Copyright 2013 Concurrent Technologies, all rights reserved.    *
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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/srioTests.c,v 1.6 2014-10-29 09:31:46 hchalla Exp $
 *
 * $Log: srioTests.c,v $
 * Revision 1.6  2014-10-29 09:31:46  hchalla
 * Added support for both 3GBaud and 5GBaud link speed testing.
 *
 * Revision 1.5  2014-10-09 09:57:37  chippisley
 * Revised test initialization functions and added checks to prevent board resets caused by use of invalid (NULL) pointers.
 *
 * Revision 1.4  2014-09-19 09:36:29  mgostling
 * Add support for two TSI721 SRIO devices
 *
 * Revision 1.3  2014-06-13 10:28:27  mgostling
 * Resolve compiler warnings
 *
 * Revision 1.2  2014-03-06 13:51:12  hchalla
 * rechecked in srioTests.c due to windows tab alignments.
 *
 * Revision 1.1  2014-01-29 13:42:23  hchalla
 * Added SRIO tests.
 *
*
 */
 
#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <stdtypes.h>
#include <errors.h>
#include <bitops.h>
#include <bit/mem.h>
#include <private/debug.h>
#include <srio/rioerr.h>
#include <srio/rio_std.h>
#include <srio/rio_drv.h>
#include <srio/rio_ids.h>
#include <srio/rio_regs.h>
#include <srio/rio.h>
#include <srio/tsi721.h>
#include <bit/delay.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/io.h>
#include <bit/delay.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>
/*#include <bit/board_service.h>*/

 
static UINT32 rio_remote_init(void);
static UINT32 dSrioGetErrCode( int errorCode);
static UINT32 SrioRegisterLinkSpeedTest(struct rio_mport *mport);
static UINT32 SrioRegisterAccessTest(struct rio_mport *mport);
static UINT32 SrioExternalLoopBackTest(struct rio_mport *mport);
static UINT32 SrioScanBackPlaneDevs(void);

static dma_addr_t ib_phys = INB_PHYS_ADDRS;
 
static unsigned int net = 0;
extern unsigned char globalSRIOSpeed;
static struct rio_dev self;
static struct rio_mport *mport;


#define MAX_RIO_DEVS            10

typedef struct rio_dev_arr
{
	UINT8            netid;
	UINT16           destid;
	UINT8            mportId;
	UINT8            amcSlotId;
	struct rio_dev   *riodev;
	struct rio_mport *mport;
}rio_dev_arr_t;


static rio_dev_arr_t arr_rio_dev[MAX_RIO_DEVS];

/*****************************************************************************\
 *
 *  TITLE:  rio_test_init ()
 *
 *  ABSTRACT:  This function searches for the SRIO device registered on 
 *  	       this network and sets the master port on this network.
 *             
 *			
 *
 * 	RETURNS: SUCCESS or FAILURE
 *
\*****************************************************************************/
static UINT32 rio_test_init( void )
{
	struct rio_dev *rdev = NULL;
	char achBuffer[80];
	
	
	memset(arr_rio_dev,0,8*sizeof(rio_dev_arr_t));

	/* 
	 *  Find the Self Device in the given network
	 */
	
	/* Find any device registered for specified net */
	while ((rdev = rio_get_device(RIO_ANY_ID,
				      RIO_ANY_ID, rdev)) != NULL) 
	{	
		if ((rdev->net->id == net) || (rdev->net->id == 1) || (rdev->net->id == 2))
		{
			sprintf(achBuffer,"rdev->net->id=%d found\n",rdev->net->id);
			vConsoleWrite(achBuffer);
			break;
		}
	}
	
	/* Set destid assigned to this mport */
	if (rdev) 
	{
		self.destid = rdev->net->hport->host_deviceid;
		self.net = rdev->net;
		// sprintf((char *)&self.dev,"LOOPBACK_(%d:%d)\n", net, self.destid);
		// vConsoleWrite((char *)&self.dev);	     
		rdev = &self;
		mport = rdev->net->hport;
		
		return E__OK;			
	}
	else
	{
		vConsoleWrite("Error SRIO device not found\n");
		return (-ENODEV);
	}		
}

/*****************************************************************************\
 *
 *  TITLE:  rio_remote_init ()
 *
 *  ABSTRACT:  This function searches for the SRIO devices on the SRIO backplane 
 *             registered on this network and stores it in any array of master port
 *             structures.
 *             
 *			
 *
 * 	RETURNS: SUCCESS or FAILURE
 *
\*****************************************************************************/
static UINT32 rio_remote_init(void)
{
	struct rio_dev *rdev = NULL;
	char achBuffer[80];
	UINT32 dIndex=0;
	UINT32 destid = 0;

	
	dIndex = 0;
	rdev = NULL;  
	while (destid <= MAX_RIO_DEVS)
	{
		while ((rdev = rio_get_device(RIO_ANY_ID,
					RIO_ANY_ID, rdev)) != NULL) 
		{			
			if (rdev->destid == destid && rdev->net->id == net &&
				!(rdev->pef & RIO_PEF_SWITCH))
			{
				sprintf(achBuffer,"destid:%d rrdev->destid:%d dev->net->id=%d found\n",destid,rdev->destid,rdev->net->id);
				vConsoleWrite(achBuffer);	
				
				arr_rio_dev[dIndex].destid = rdev->destid;
				arr_rio_dev[dIndex].netid = rdev->net->id;
				arr_rio_dev[dIndex].riodev = rdev;
				arr_rio_dev[dIndex].mport = rdev->net->hport;
				arr_rio_dev[dIndex].mportId = rdev->net->hport->id;
				arr_rio_dev[dIndex].amcSlotId = rdev->net->hport->amc_slot_id;
				dIndex++;
				break;
			}
		}			
		rdev = NULL;
		destid++;
	}
	
	if ( dIndex == 0 )
	{
		vConsoleWrite("Error remote SRIO device not found\n");
		return (-ENODEV);
	}
	else
	{
		return E__OK;
	}
}

/*****************************************************************************\
 *
 *  TITLE:  SrioScanBackPlaneDevs ()
 *
 *  ABSTRACT:  This function checks all the master port devices on the backplane
 *  		for speed and link width.
 *			
 *
 * 	RETURNS: SUCCESS or FAILURE
 *
\*****************************************************************************/
static UINT32 SrioScanBackPlaneDevs(void)
{
	UINT32 dIndex,data=0;
	char achBuffer[80];
	char speed[50];
	UINT32 dRetValue;
	UINT32 lw_data=0;


	dIndex = 0;
	dRetValue = (-ENODEV);
	
	for (dIndex =0;dIndex < MAX_RIO_DEVS;dIndex++)
	{
		if (arr_rio_dev[dIndex].riodev != NULL)
		{
			rio_read_config_32(arr_rio_dev[dIndex].riodev,RIO_SP_CTL2,&data);		
			switch(data & RIO_SP_CTL2_SPEED_MSK )
			{
				case RIO_SP_CTL2_1250:
				{
					sprintf(speed,"1.250G Baud");
					break;
				}
				case RIO_SP_CTL2_2500:
				{
					sprintf(speed,"2.500G Baud");
					break;
				}
				case RIO_SP_CTL2_3125:
				{
					sprintf(speed,"3.125G Baud");
					break;
				}
				case RIO_SP_CTL2_5000:
				{
					sprintf(speed,"5.000G Baud");
					break;
				}
				case RIO_SP_CTL2_6250:
				{
					sprintf(speed,"6.250G Baud");
					break;
				}
				default:
				{
					sprintf(speed,"Unknown G Baud");
					break;
				}
			}

			rio_read_config_32(arr_rio_dev[dIndex].riodev,RIO_SP_CTL,&lw_data);			
			switch(lw_data & RIO_SP_CTL_LINK_WIDTH_MSK )
			{
				case RIO_SP_CTL_X4:
				{
					sprintf(achBuffer,"\n+++RIO_ENDPOINT_DEVICE: found %s @Link Speed:%s Link: X4+++\n", rio_name(arr_rio_dev[dIndex].riodev),                                                     (char*)speed);
					vConsoleWrite(achBuffer);							
					break;
				}
			        case RIO_SP_CTL_X2:
				{
					sprintf(achBuffer,"\n+++RIO_ENDPOINT_DEVICE: found %s @Link Speed:%s Link: X2+++\n", rio_name(arr_rio_dev[dIndex].riodev),                                                     (char*)speed);
					vConsoleWrite(achBuffer);							
					break;
				}
				case RIO_SP_CTL_X1L0:
				{
					sprintf(achBuffer,"\n+++RIO_ENDPOINT_DEVICE: found %s @Link Speed:%s Link: X1L0+++\n", rio_name(arr_rio_dev[dIndex].riodev),                                            (char*)speed);
					vConsoleWrite(achBuffer);							
					break;
				}
				case RIO_SP_CTL_X1LR:
				{
					sprintf(achBuffer,"\n+++RIO_ENDPOINT_DEVICE: found %s @Link Speed:%s Link: X1LR+++\n", rio_name(arr_rio_dev[dIndex].riodev),                                           (char*)speed);
					break;
				}
				default:
				{
					sprintf(achBuffer,"\n+++RIO_ENDPOINT_DEVICE: found %s @Link Speed:%s Link: Unknown+++\n", rio_name(arr_rio_dev[dIndex].riodev),                                        (char*)speed);
					vConsoleWrite(achBuffer);																	       break;
				}
			}
			
			dRetValue = E__OK;	
		}
	}
	
	return dRetValue;
}

/*****************************************************************************\
 *
 *  TITLE:  SrioRegisterLinkSpeedTest ()
 *
 *  ABSTRACT:  This function reads the local srio mport link speed settings
 *  	       and checks if its negotiated properly for 5GBaud and X4 link,
 *  	       otherwise return error.
 *             
 *			
 *
 * 	RETURNS: SUCCESS or FAILURE
 *
\*****************************************************************************/
static UINT32 SrioRegisterLinkSpeedTest(struct rio_mport *mport)
{
	int err = 0;
	char achBuffer[80];	
	UINT32 data=0,lw_data=0, dRetValue=0;
	char speed[80];
	UINT16 vendor=0, device=0;
	
	
	if ( (mport == NULL) || (mport->ops == NULL) || (mport->ops->lcread == NULL) )
	{
		return (-ENODEV);
	}
	
	err = rio_local_read_config_32(mport,(UINT32)RIO_DEV_ID_CAR,&data);
	if (err != 0)
	{
		dRetValue = dSrioGetErrCode(err);
		return (dRetValue);
	}
	vendor = (UINT16)(data & 0xFFFF);
	device = (UINT16)((data>>16) & 0xFFFF);
	
	/*
	 *  Check the SRIO Link Speed 
	 */			 
	 err = rio_local_read_config_32(mport,(UINT32)RIO_SP_CTL2,&data);
	 if (err != 0)
	 {
		dRetValue = dSrioGetErrCode(err);
		return (dRetValue);
	 }
			
	switch(data & RIO_SP_CTL2_SPEED_MSK )
	{
		case RIO_SP_CTL2_1250:
		{
			sprintf(speed,"Speed: 1.250G Baud");
			break;
		}
		case RIO_SP_CTL2_2500:
		{
			sprintf(speed,"Speed: 2.500G Baud");
			break;
		}
		case RIO_SP_CTL2_3125:
		{
			sprintf(speed,"Speed: 3.125G Baud");
			break;
		}
		case RIO_SP_CTL2_5000:
		{
			sprintf(speed,"Speed: 5.000G Baud");
			break;
		}
		case RIO_SP_CTL2_6250:
		{
			sprintf(speed,"Speed: 6.250G Baud");
			break;
		}
		default:
		{
			sprintf(speed,"Speed: Unknown	G Baud");
			break;
		}
	}


	if ( globalSRIOSpeed == 0x4)
	{
		if ((data & RIO_SP_CTL2_SPEED_MSK) != RIO_SP_CTL2_5000)
		{
			sprintf (achBuffer,"Error Link %s not as expected\n", (char*)speed);
			vConsoleWrite(achBuffer);

			dRetValue = E__SRIO_ESPEED;
			return (dRetValue);
		}
	}
	else if (globalSRIOSpeed == 0x03)
	{
		if ((data & RIO_SP_CTL2_SPEED_MSK) != RIO_SP_CTL2_3125)
		{
			sprintf (achBuffer,"Error Link %s not as expected\n", (char*)speed);
			vConsoleWrite(achBuffer);
			
			dRetValue = E__SRIO_ESPEED;
			return (dRetValue);
		}
	}
			
	err = rio_local_read_config_32(mport,(UINT32)RIO_SP_CTL,&lw_data);			
	if (err != 0)
	{
		dRetValue = dSrioGetErrCode(err);
		return (dRetValue);
	}
			
	switch(lw_data & RIO_SP_CTL_LINK_WIDTH_MSK )
	{
		case RIO_SP_CTL_X4:
			sprintf (achBuffer,"\n+++RIO_SELF_PORT:%d:%d vendor:%04x,device:%04x @%s Link: X4+++\n",net, self.destid,vendor,device,(char*)speed);
			vConsoleWrite(achBuffer);						
			break;
					
		case RIO_SP_CTL_X2:
			sprintf (achBuffer,"\n+++RIO_SELF_PORT:%d:%d vendor:%04x,device:%04x @%s Link: X2+++\n",net, self.destid,vendor,device,(char*)speed);
			vConsoleWrite(achBuffer);						
			break;
					
		case RIO_SP_CTL_X1L0:
			sprintf (achBuffer,"\n+++RIO_SELF_PORT:%d:%d vendor:%04x,device:%04x @%s Link: X1L0+++\n",net, self.destid,vendor,device,(char*)speed);
			vConsoleWrite(achBuffer);											
			break;
					
		case RIO_SP_CTL_X1LR:
			sprintf (achBuffer,"\n+++RIO_SELF_PORT:%d:%d vendor:%04x,device:%04x @%s Link: X1LR+++\n",net, self.destid,vendor,device,(char*)speed);
			vConsoleWrite(achBuffer);																				
			break;
					
		 default:
			sprintf (achBuffer,"\n+++RIO_SELF_PORT:%d:%d vendor:%04x,device:%04x @%s Link: Unknown+++\n",net, self.destid,vendor,device,(char*)speed);
			vConsoleWrite(achBuffer);				 					
		    break;
	 }

	  if ( (lw_data & RIO_SP_CTL_LINK_WIDTH_MSK) != RIO_SP_CTL_X4)
	  {
		dRetValue = E__SRIO_ELINK;
		return (dRetValue);
	  }
	
	 return E__OK;
}



/*****************************************************************************\
 *
 *  TITLE:  SrioRegisterAccessTest ()
 *
 *  ABSTRACT:  This function reads the local srio mport Device ID CAR register
 *  	       and checks if its accessible and checks with know deive id
 *  	       and vendor id, otherwise return error.
 *             
 *			
 * 	RETURNS: SUCCESS or FAILURE
\*****************************************************************************/
static UINT32 SrioRegisterAccessTest(struct rio_mport *mport)
{

	int err;
	UINT32 dRetValue;
	UINT32 data=0;
	char achBuffer[80];
	UINT16 vendor=0,device=0;
	
	
	if ( (mport == NULL) || (mport->ops == NULL) || (mport->ops->lcread == NULL) )
	{
		return (-ENODEV);
	}
	
	err = rio_local_read_config_32(mport,(UINT32)RIO_DEV_ID_CAR,&data); 

	if (err != 0)
	{
		dRetValue = dSrioGetErrCode(err);
		return (dRetValue);
	}
	
	vendor = (UINT16)(data & 0xFFFF);
	device = (UINT16)((data>>16) & 0xFFFF);
	
	sprintf(achBuffer,"vendor:%04x,device:%04x\n",vendor,device);
	vConsoleWrite(achBuffer);	
	
	// check this is an IDT Tsi721
	if ((vendor != 0x0038) || (device != 0x80ab))
	{
		vConsoleWrite("Error in Register Read\n");
		dRetValue = E__SRIO_EREG_ACCESS;
	}
	else
	{
		vConsoleWrite("Register Access Test: PASS\n");
		dRetValue = E__OK;
	}

	return dRetValue;
}

/*****************************************************************************\
 *
 *  TITLE:  SrioExternalLoopBackTest ()
 *
 *  ABSTRACT:  This function will do external data loopback test for a given 
 *	       master port on the network. It opens an inbound and outboud 
 *  	       windows for the data transfer between PCIe -> SRIO address and
 *  	       compares the data between two inbound and outbound buffers, if 
 *  	       there are any errors it will return error other wise return success.
 *
 * 	RETURNS: SUCCESS or FAILURE
\*****************************************************************************/
static UINT32 SrioExternalLoopBackTest(struct rio_mport *mport)
{
	int err = 0;
	char achBuffer[80];
	UINT32 dRetValue = 0;
	UINT8 *outbMem;
	UINT8 *InbMem;
	UINT32 dHandle,dHandle1;
	PTR48 p4,p5;
	
	volatile UINT32 ib_base=0;
	volatile UINT32 ob_base=0;
	static UINT64 riobase=0x00000000;
	
	UINT64 addr=0,asize=0;
	UINT32 size = 0x400000,data =0;
	UINT8 bIteration=0;
	
	
	if ( (mport == NULL) || (mport->ops == NULL) || (mport->ops->lcread == NULL) )
	{
		return (-ENODEV);
	}
	
	/* Clear any previous errors */
	err = rio_local_write_config_32(mport,(UINT32)RIO_SP_ERR_DET,0); 
    if (err != 0)
    {
		 dRetValue = dSrioGetErrCode(err);
		 return (dRetValue);
    }

	for (bIteration =0; bIteration < 5; bIteration++)
	{

		dHandle = dGetPhysPtr (ib_phys, size, &p4, (void*)&ib_base);
		if(dHandle == E__FAIL)
		{

		    vConsoleWrite("Unable to allocate TX Desc\n"); 
		    err = (-ENOMEM);
		    dRetValue = dSrioGetErrCode(err);
		    return (dRetValue);
     		}
     	
     	   /*
     	    *  Set Ibound pattern
     	    */
     	     	 
               memset((UINT8*)ib_base,0x55,(size/8));
    
		riobase = (RIO_BASE_ADDRS) * (mport->amc_slot_id);
 	
		err = rio_map_inb_region(mport, ib_phys, riobase, (UINT32)size, 0);		 
		if (err) 
		{
			sprintf(achBuffer,"Request to map IB region failed for %s, err=%d\n",mport->name, err);
			vConsoleWrite(achBuffer);
		    	dRetValue = dSrioGetErrCode(err);
		   	return (dRetValue);
		}	

		/*
		 * Get the prefetchable outbound memory
		 */
		rio_get_outb_pref_asinfo( mport, &addr, &asize );

		sprintf(achBuffer,"Prefetchable PCI Memory Address=0x%08x\n", (UINT32) addr);		
		vConsoleWrite(achBuffer);	 
		
		sprintf(achBuffer,"Prefetchable PCI Memory Size=0x%08x\n", (UINT32) asize);
		vConsoleWrite(achBuffer);	 
		/*
		 * Map PCI -> RIO
		 */
		err = rio_map_outb_window( mport, 0 , addr , size, 0 );
		if (err) 
		{
			sprintf(achBuffer, "Request to map OB region failed for %s, err=%d\n",mport->name, err);
			vConsoleWrite(achBuffer);	 
		    	dRetValue = dSrioGetErrCode(err);
		   	return (dRetValue);
		}

		err = rio_config_outb_zone( mport, 0 , 0 , mport->host_deviceid, riobase , RIO_NWRITE_SWRITE);
		if (err) 
		{
			sprintf(achBuffer, "Request to map OB region failed for %s, err=%d\n",mport->name, err);
			vConsoleWrite(achBuffer);	 
		    	dRetValue = dSrioGetErrCode(err);
		   	return (dRetValue);
		}
		
	
		outbMem = (UINT8 *)((UINT32)addr);

		if( outbMem == NULL )
		{
			sprintf(achBuffer, "Request to map OB region failed for %s\n",mport->name);
			vConsoleWrite(achBuffer);	 
			rio_unmap_outb_window( mport, 0 );
			dRetValue = E__SRIO_EOUTB_MAP;
		        return (dRetValue);
		}
		
	       /*
		* Map the PCI address for kernel access
		*/
		dHandle1 = dGetPhysPtr (addr, size, &p5, (void*)&ob_base);					
		if(dHandle1 == E__FAIL)
		{
		    vConsoleWrite("Unable to allocate TX Desc\n"); 
		    err = (-ENOMEM);
		    dRetValue = dSrioGetErrCode(err);
		    return (dRetValue);
     		}
     	     	
		outbMem = (UINT8 *) ob_base;		
		memset((UINT8 *) outbMem,0xaa,(size/8));
		
		InbMem = (UINT8 *) ib_base;
		
		if (!memcmp(InbMem,outbMem,(size/8)))
		{
			vConsoleWrite("Loopback Test: PASS\n");
			dRetValue = E__SRIO_OK;
		}
		else
		{
			vConsoleWrite("Loopback Test: FAIL\n");
			dRetValue = E__SRIO_ELPBACK_DATA;
			break;
		}
		
 
		/*
		 * Unmap the mapped regions and Free Memory
		 */
		 if (dHandle1)
			vFreePtr(dHandle1);
			
		  if (dHandle)
		    vFreePtr(dHandle);

		 rio_unmap_inb_region(mport,ib_phys);
		 rio_unmap_outb_window( mport, 0 );
	 } 

	 /*
	  * Check for SRIO Port Errors
	  */			
	   err = rio_local_read_config_32(mport,(UINT32)RIO_SP_ERR_DET,&data); 
  	   if (err != 0)
	   {
			dRetValue = dSrioGetErrCode(err);
			return (dRetValue);
	   }

	   if ((data & RIO_CS_CRC_ERR_MASK) == RIO_CS_CRC_ERR_MASK)
	   {
	   		vConsoleWrite("Error control symbol with bad CRC\n");
			dRetValue = E__SRIO_CS_CRC;
	   }
	   else if ((data & RIO_CS_ILL_ID_MASK) == RIO_CS_ILL_ID_MASK)
	   {
			vConsoleWrite("Error unexpected control symbol ackID\n");
	        dRetValue = E__SRIO_CS_ILL_ID;
	   }
	   else if ((data & RIO_PKT_CRC_ERR_MASK) == RIO_PKT_CRC_ERR_MASK)
	   {
	   		vConsoleWrite("Error bad packet CRC\n");
			dRetValue = E__SRIO_PKT_CRC;
	   }
	   else if ((data & RIO_PKT_ILL_ACKID_MASK) == RIO_PKT_ILL_ACKID_MASK)
	   {
	   		vConsoleWrite("Error unexpected packet ackID\n");
			dRetValue = E__SRIO_PKT_ILL_ACKID;
	   }
	   else if ((data & RIO_PKT_ILL_SIZE_MASK) == RIO_PKT_ILL_SIZE_MASK)
	   {
	   		vConsoleWrite("Error illegal packet size\n");
			dRetValue = E__SRIO_PKT_ILL_SIZE;
	   }
	   else if ( (data & RIO_PROT_ERR_MASK) == RIO_PROT_ERR_MASK)
	   {
	   		vConsoleWrite("Error with protocol\n");
			dRetValue = E__SRIO_PROT_ERR;
	   }
	   else if ( (data & RIO_DELIN_ERR_MASK) == RIO_DELIN_ERR_MASK)
	   {
	   		vConsoleWrite("Error unaligned or invalid character received\n");
			dRetValue = E__SRIO_DELIN_ERR;
	   }
	   else if ( (data & RIO_CS_ACK_ILL_MASK) ==  RIO_CS_ACK_ILL_MASK)
	   {
	   		vConsoleWrite("Error unexpected acknowledge control symbol\n");
			dRetValue = E__SRIO_CS_ACK_ILL;
	   }

	   err = rio_local_write_config_32(mport,(UINT32)RIO_SP_ERR_DET,0); 
  	   if (err != 0)
	   {
			dRetValue = dSrioGetErrCode(err);
			return (dRetValue);
	   }

	   return dRetValue;	
}

/*****************************************************************************\
 *
 *  TITLE:  rio_test_de_init ()
 *
 *  ABSTRACT: This function clears the local data used.  
 *             
 *			
 *
 * 	RETURNS: nothing
 *
\*****************************************************************************/
static void rio_test_de_init(void)
{
	memset(arr_rio_dev,0,8*sizeof(rio_dev_arr_t));
}

/*****************************************************************************\
 *
 *  TITLE:  dSrioGetErrCode ()
 *
 *  ABSTRACT:  This function searches for mapping of error codes between srio and 
 *             test error codes.
 *             
 *			
 *
 * 	RETURNS: SUCCESS or FAILURE
\*****************************************************************************/
static UINT32 dSrioGetErrCode( int errorCode)
{
        SRIO_ERR_CODES *pError;
        UINT32 dSrioErrorCode;
#ifdef DEBUG
    vConsoleWrite ("errorCode:%#x ",errorCode);
#endif

        pError = (SRIO_ERR_CODES*) &SrioErrCodes[0];

        while (pError->srioTestErrorCode != 0xFFFFFFFF)
        {
#ifdef DEBUG
        printf ("pError->srioErrorCode:%#x \n",pError->srioErrorCode);
#endif
                if (pError->srioErrorCode == errorCode)
                {
                        break;
                }
                else
                {
                        pError++;
                }
        }

        dSrioErrorCode = (UINT32) pError->srioErrorCode;


        return dSrioErrorCode;
}


/*****************************************************************************\
 *
 *  TITLE: CCT_SrioRegisterAccessTest ()
 *
 *  ABSTRACT:  This function will discover the self master port RIO from enumerated 
 *             SRIO list and does a register access test on given master port.
 *             
 *
 * 	RETURNS: Returns E__OK or error.
 *
\*****************************************************************************/
TEST_INTERFACE (CCT_SrioRegisterAccessTest, "SRIO Register Access Test")
{
	UINT32 dRetValue;

	dRetValue = rio_test_init();
	
	if (dRetValue == E__OK)
	{
		dRetValue = SrioRegisterAccessTest(mport);
	}
	
    rio_test_de_init();

	return dRetValue;

}

/*****************************************************************************\
 *
 *  TITLE: CCT_SrioRegisterLinkSpeedTest()
 *
 *  ABSTRACT:  This function will discover the self master port RIO from enumerated 
 *             SRIO list and does a Link Speed test on given master port.
 *             
 *
 * 	RETURNS: Returns E__OK or error.
 *
\*****************************************************************************/
TEST_INTERFACE (CCT_SrioRegisterLinkSpeedTest, "SRIO Link Speed Test")
{
	UINT32 dRetValue;

	dRetValue = rio_test_init();
	
	if (dRetValue == E__OK)
	{
		dRetValue = SrioRegisterLinkSpeedTest(mport);	
	}
    
	rio_test_de_init();

	return dRetValue;

}

/*****************************************************************************\
 *
 *  TITLE: CCT_SrioExternalLoopBackTest()
 *
 *  ABSTRACT:  This function will discover the self master port RIO from enumerated 
 *             SRIO list and does a external data transfer loopback test on given 
 *             master port.
 *             
 *
 * 	RETURNS: Returns E__OK or error.
 *
\*****************************************************************************/
TEST_INTERFACE (CCT_SrioExternalLoopBackTest, "SRIO External Loopback Test")
{
	UINT32 dRetValue;

	dRetValue = rio_test_init();
	
	if (dRetValue == E__OK)
	{
		dRetValue = SrioExternalLoopBackTest(mport);	
	}
	
    rio_test_de_init();

	return dRetValue;

}


/*****************************************************************************\
 *
 *  TITLE: CCT_SrioScanBackPlaneDevs()
 *
 *  ABSTRACT:  This function will discover the endpoind devices on the SRIO backplane.
 *             
 *             
 *             
 *
 * 	RETURNS: Returns E__OK or error.
 *
\*****************************************************************************/
TEST_INTERFACE (CCT_SrioScanBackPlaneDevs, "SRIO Scan Backplane Devices")
{
	UINT32 dRetValue;

	dRetValue = rio_remote_init();
	
	if (dRetValue == E__OK)
	{
		dRetValue = SrioScanBackPlaneDevs();	
	}
	
    rio_test_de_init();

	return dRetValue;
}
