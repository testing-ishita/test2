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
 *                                                                      *
 ************************************************************************/

/* tempsensor.c - To test the CPU, board temperature through SMBus interface 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/tempsensor.c,v 1.6 2015-03-10 15:44:41 mgostling Exp $
 *
 * $Log: tempsensor.c,v $
 * Revision 1.6  2015-03-10 15:44:41  mgostling
 * Fixed compiler warnings.
 * Updated TIMEOUT in tempsensor.h and removed local definition.
 *
 * Revision 1.5  2015-02-26 10:06:05  hchalla
 * Added conversion of negative temperature values read from NCT 7802Y.
 *
 * Revision 1.4  2015-02-25 18:04:14  hchalla
 * Added support for reading NCT7802 temperature sensor  using SMBUS interface.
 *
 * Revision 1.3  2013-11-25 12:58:05  mgostling
 * Removed the -ve temperature tests as discussed with SW as DTS does not reliably support -ve temperature.
 *
 * Revision 1.2  2013-10-08 07:13:39  chippisley
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 *
 * Revision 1.1  2013/09/04 07:46:45  chippisley
 * Import files into new source repository.
 *
 * Revision 1.14  2011/11/21 11:17:22  hmuneer
 * no message
 *
 * Revision 1.13  2011/10/27 15:50:39  hmuneer
 * no message
 *
 * Revision 1.12  2011/06/13 12:33:38  hmuneer
 * no message
 *
 * Revision 1.11  2011/05/16 14:32:12  hmuneer
 * Info Passing Support
 *
 * Revision 1.10  2011/03/22 14:05:36  hchalla
 * Added temperature threshold limits for new board VX 813 this needs to be board independent, a new board service needs to be added in next release.
 *
 * Revision 1.9  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.8  2010/10/04 10:47:36  hmuneer
 * vp717 cute v1.03 tempsensor fix
 *
 * Revision 1.7  2010/09/10 10:08:07  cdobson
 * Increased temerature limits.
 *
 * Revision 1.6  2010/06/23 11:01:57  hmuneer
 * CUTE V1.01
 *
 * Revision 1.5  2010/03/31 16:27:10  hmuneer
 * no message
 *
 * Revision 1.4  2010/01/19 12:04:49  hmuneer
 * vx511 v1.0
 *
 * Revision 1.3  2009/06/03 08:30:58  cvsuser
 * Tidy up test names
 *
 * Revision 1.2  2009/05/21 08:13:26  cvsuser
 * Shortened test name length
 *
 * Revision 1.1  2009/05/15 11:12:36  jthiru
 * Initial checkin for Temperature Sensor tests
 *
 *
 */

 /* includes */ 
#include <stdtypes.h>
#include <errors.h>		
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <bit/hal.h>
 
#include <bit/board_service.h>
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>
#include <bit/msr.h>
#include <devices/tempsensor.h>
 
#include <error_msg.h>
 
/* defines */
 
#define DEBUG
#ifdef DEBUG
	 static char buffer[128];
#endif

/* externals */
extern void vDelay(UINT32 dMsDelay);
extern void vConsoleWrite(char*	achMessage);


/*****************************************************************************
 * dInitSMB: this function detects a PCI SMB controller and performs basic
 * 				initialization.
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 dInitSMB 
( 
	DEVICE *psDev, 
	UINT32 bInstance 
)
{
    UINT32	dFound;
	PCI_PFA pfa;
	
	/* Initialise SMBUS controller */

	/* find the SMBUS controller by class code */
  	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
    dFound = iPciFindDeviceByClass (bInstance, 0x0C, 0x05, &pfa);
    if (dFound == E__DEVICE_NOT_FOUND)
	{
		return dFound;
	}
	psDev->bBus = PCI_PFA_BUS(pfa);
	psDev->bDev = PCI_PFA_DEV(pfa);
	psDev->bFunc = PCI_PFA_FUNC(pfa);
#ifdef DEBUG
	sprintf(buffer, "B: %d, D: %d, F: %d\n", PCI_PFA_BUS(pfa), PCI_PFA_DEV(pfa), PCI_PFA_FUNC(pfa));
	vConsoleWrite(buffer);
#endif
	psDev->wSMBIoRegs = (UINT16)dPciReadReg (pfa, 0x20, REG_16) & 0xFFFE;
#ifdef DEBUG
	sprintf(buffer, "SMBus IO register 0x%x\n", psDev->wSMBIoRegs);
	vConsoleWrite(buffer);
#endif
	/* Enable I/O address space decoding */
	vPciWriteReg(pfa, 0x4, REG_16, dPciReadReg(pfa, 0x4, REG_16) | 0x1);
	/* Check if SMBus is busy */
	if( dIoReadReg(psDev->wSMBIoRegs, REG_8) & 0x3F)
	{
		vDelay(1);
		vIoWriteReg(psDev->wSMBIoRegs, REG_8, dIoReadReg(psDev->wSMBIoRegs, REG_8));
		vDelay(1);
	}
	if( dIoReadReg(psDev->wSMBIoRegs, REG_8) & 0x3F)
		return E__SMBC_BUSY;

	return E__OK;
}


/*****************************************************************************
 * smb_tfr: this function receives response for the Thermal SMB device 
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 smb_tfr 
( 
	DEVICE *psDev,
	UINT32 read_write,
	UINT32 addr,
	UINT32 command,
	UINT8 bData
)
{
	UINT8 val, timeout;
	

	vIoWriteReg(psDev->wSMBIoRegs + 0xD, REG_8,  0x00);
	vIoWriteReg(psDev->wSMBIoRegs + 0x0, REG_8,  0x1E);
	vIoWriteReg(psDev->wSMBIoRegs + 0x3, REG_8, command );
	vIoWriteReg(psDev->wSMBIoRegs + 0x4, REG_8, (((addr & 0x7F)) | (read_write & 1)) );
	vIoWriteReg(psDev->wSMBIoRegs + 0x5, REG_8, bData );

	vIoWriteReg(psDev->wSMBIoRegs + 0x2, REG_8, 0x48 );
	vDelay(10);

	do{
		vDelay(1);
		val = dIoReadReg(psDev->wSMBIoRegs, REG_8);
		val &= 0x1E;
	}while((val == 0) && (timeout++ < TIMEOUT));

    // Check for errors

      if ((val & 0x1C) != 0)
               return (E_SMBRW_ERROR);

	if( dIoReadReg(psDev->wSMBIoRegs, REG_8) & 0x1)
		return E__SMBRD_TIMEOUT;
	
	   vIoWriteReg(psDev->wSMBIoRegs, REG_8, val);

   //vIoWriteReg(psDev->wSMBIoRegs, REG_8, dIoReadReg(psDev->wSMBIoRegs, REG_8) );

	psDev->data = dIoReadReg(psDev->wSMBIoRegs + 0x5, REG_8);

	return (E__OK);
}


/*****************************************************************************
 * dAccessThermal: this function detects the Thermal controller and performs 
 * 				device access test.
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 dAccessThermal 
( 
	DEVICE *psDev,
	TEMPERATURE_INFO* tempsensor
)
{
	UINT8 manuf_id, device_id;
	UINT32 dstatus;
	
	/* SMB transfer */
	dstatus = smb_tfr(psDev, 1, tempsensor->Addr, MID_REG,0);
	if( dstatus == E__OK)
		manuf_id = psDev->data;
	else
		return dstatus;
	
	dstatus = smb_tfr(psDev, 1, tempsensor->Addr, DID_REG,0);
	if( dstatus == E__OK)
		device_id = psDev->data;
	else
		return dstatus;
	
#ifdef DEBUG
		sprintf(buffer, "Manuf ID: 0x%x Dev ID: 0x%x\n", manuf_id, device_id);
		vConsoleWrite(buffer); 
#endif		
	/* Check results */
	if(( manuf_id == tempsensor->ManufID ) && ( device_id == tempsensor->DevID ))
		return E__OK;
	else
		return E__ACCESS_DATA;
	
}

static UINT16 nct7802y_bank1_PECI_get(DEVICE *psDev,  TEMPERATURE_INFO* tempsensor, UINT8 *bTemp)
{
	unsigned short temp=0;
	UINT32 dstatus;
	UINT32 dtest_status = E__OK;

	UINT32 retry=5;

	dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0,1);
   	if( dtest_status != E__OK)
		return dstatus;

	dstatus = smb_tfr(psDev, 1, tempsensor->Addr, 0x17,0);
	if( dtest_status != E__OK)
		return dstatus;
	temp  = (psDev->data << 8) & 0xff00;
	dstatus = smb_tfr(psDev, 1, tempsensor->Addr, 0x18,0);

	if( dtest_status != E__OK)
		return dstatus;

	temp |= psDev->data;

	if (temp == 0x8000)
	{
		do {
			dstatus = smb_tfr(psDev, 1, tempsensor->Addr, 0x17,0);
			if( dtest_status != E__OK)
				return dstatus;
			temp  = (psDev->data << 8) & 0xff00;
			dstatus = smb_tfr(psDev, 1, tempsensor->Addr, 0x18,0);

			if( dtest_status != E__OK)
				return dstatus;

			temp |= psDev->data;
		}while ((temp == 0x8000) && retry--);
	}
	else
	{
		temp  = ((temp >> 6) & 0x3FF) | 0xFC00;
		temp = (~(temp-1) & 0xFFFF);
		temp  =  105 - temp;
		*bTemp = ((unsigned char)temp);
	}

	dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0,0);
   	if( dtest_status != E__OK)
		return dstatus;

    return E__OK;
}

static UINT32 nct7802y_bank0_RTD1_temp_get (DEVICE *psDev,  TEMPERATURE_INFO* tempsensor, UINT8 *bTemp)
{

	UINT32 dtest_status = 0;



	dtest_status = smb_tfr(psDev, 0, tempsensor->Addr, 0,0);
   	if( dtest_status != E__OK)
		return dtest_status;

   	dtest_status = smb_tfr(psDev, 0, tempsensor->Addr,0x22,0x45);
   	if( dtest_status != E__OK)
		return dtest_status;

   	dtest_status = smb_tfr(psDev, 1, tempsensor->Addr,0x01,0x0);
   	if( dtest_status != E__OK)
		return dtest_status;


	*bTemp = psDev->data;

	dtest_status = smb_tfr(psDev, 0, tempsensor->Addr, 0,0);
   	if( dtest_status != E__OK)
		return dtest_status;

   	dtest_status = smb_tfr(psDev, 0, tempsensor->Addr,0x22,0x45);
   	if( dtest_status != E__OK)
		return dtest_status;

   	dtest_status = smb_tfr(psDev, 1, tempsensor->Addr,0x01,0x0);
   	if( dtest_status != E__OK)
		return dtest_status;

   	*bTemp = psDev->data;

    return E__OK;
}


static UINT32 nct7802y_bank0_RTD2_temp_get (DEVICE *psDev,  TEMPERATURE_INFO* tempsensor, UINT8 *bTemp)
{
	UINT32 dtest_status = 0;

	dtest_status = smb_tfr(psDev, 0, tempsensor->Addr, 0,0);
   	if( dtest_status != E__OK)
		return dtest_status;

   	dtest_status = smb_tfr(psDev, 0, tempsensor->Addr,0x22,0x45);
   	if( dtest_status != E__OK)
		return dtest_status;

   	dtest_status = smb_tfr(psDev, 1, tempsensor->Addr,0x02,0x0);

   	if( dtest_status != E__OK)
		return dtest_status;

	*bTemp = psDev->data;

    return E__OK;
}

/*****************************************************************************
 * dThreshThermal: this function checks the temperature against threshold level 
 *
 * RETURNS: Test passed E__OK or error code.
 */

static UINT32 dThreshThermal 
( 
	DEVICE *psDev,
	TEMPERATURE_INFO* tempsensor
)
{

	TEMTHRESHOLD *tempthreshold;
	int tempconf, cputempu=0, brdtemp=0,brdtemp1=0,brdtemp2=0;
#ifdef DEBUG
	int cputemp, cputempl;
#endif
	UINT32 dtest_status = E__OK;

	UINT8 manuf_id, device_id;
	UINT32 dstatus;

	board_service(SERVICE__BRD_GET_TEMP_THRESHOLDS, NULL, &tempthreshold);

	/* SMB transfer */
	dstatus = smb_tfr(psDev, 1, tempsensor->Addr, MID_REG,0);
	if( dstatus == E__OK)
		manuf_id = psDev->data;
	else
		return dstatus;

	dstatus = smb_tfr(psDev, 1, tempsensor->Addr, DID_REG,0);
	if( dstatus == E__OK)
		device_id = psDev->data;
	else
		return dstatus;

#ifdef DEBUG
		sprintf(buffer, "Manuf ID: 0x%x Dev ID: 0x%x\n", manuf_id, device_id);
		vConsoleWrite(buffer);
#endif

	if(( manuf_id == 0xC3 ) && ( device_id == 0x21))
	{

	    	dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0x0,0x0);

		   if( dtest_status != E__OK)
			return dstatus;

			dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0x23,0x03);

		   if( dtest_status != E__OK)
			return dstatus;

		   dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0x22,0x45);

		   if( dtest_status != E__OK)
			return dstatus;

			dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0x0,0x1);

		   if( dtest_status != E__OK)
			return dstatus;


			dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0x1,0x81);

		   if( dtest_status != E__OK)
			return dstatus;


			dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0x02,0x0);
		   if( dtest_status != E__OK)
			return dstatus;

			dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0x03,0x33);
		   if( dtest_status != E__OK)
			return dstatus;

			dstatus = smb_tfr(psDev, 0, tempsensor->Addr, 0x04,0x00);

		    if( dtest_status != E__OK)
			return dstatus;

		   nct7802y_bank1_PECI_get(psDev, tempsensor, (UINT8*)&cputempu);
		   nct7802y_bank1_PECI_get(psDev, tempsensor, (UINT8*)&cputempu);
		   nct7802y_bank1_PECI_get(psDev, tempsensor, (UINT8*)&cputempu);

		   nct7802y_bank0_RTD1_temp_get(psDev, tempsensor, (UINT8*)&brdtemp1);
		   nct7802y_bank0_RTD2_temp_get(psDev, tempsensor, (UINT8*)&brdtemp2);

  		   if ((brdtemp1 & 0x80) == 0x80)
		   {
			   brdtemp1 = brdtemp1 - 255;
		   }

		   if ((brdtemp2 & 0x80) == 0x80)
		   {
			   brdtemp2 = brdtemp2 - 255;
		   }


#ifdef DEBUG
			sprintf(buffer, " cputempu %d Deg C brdtemp1 %d Deg C  brdtemp2  %d Deg C\n",cputempu,brdtemp1,brdtemp2);
			vConsoleWrite(buffer);
#endif
			if (cputempu >tempthreshold->bCpuTempThres )
				return E__CPUTEMPTHRESH;

			if (brdtemp1 >tempthreshold->bBoardTemp1Thres )
							return E__BRDTEMP1THRESH;

			if (brdtemp2 >tempthreshold->bBoardTemp2Thres )
							return E__BRDTEMP2THRESH;

			dtest_status = E__OK;

	}
	else
	{
		dtest_status = smb_tfr(psDev, 1, tempsensor->Addr, TEMP_CONF,0);
		if( dtest_status == E__OK)
			tempconf = psDev->data;
		else
			return dtest_status;

		dtest_status = smb_tfr(psDev, 1, tempsensor->Addr, BRD_TEMP,0);
		if( dtest_status == E__OK)
			brdtemp = psDev->data;
		else
			return dtest_status;

		dtest_status = smb_tfr(psDev, 1, tempsensor->Addr, CPU_TEMPL,0);
		if( dtest_status != E__OK)
		{
			return dtest_status;
		}

	#ifdef DEBUG
		cputempl = psDev->data;
	#endif

		dtest_status = smb_tfr(psDev, 1, tempsensor->Addr, CPU_TEMPU,0);
		if( dtest_status == E__OK)
			cputempu = psDev->data;
		else
			return dtest_status;

	#ifdef DEBUG
				sprintf(buffer, "Conf: 0x%x BTemp: 0x%x Ctmpl: 0x%x Ctmpu: 0x%x\n", tempconf, brdtemp, cputempl, cputempu);
				vConsoleWrite(buffer);
	#endif

		if(tempconf & ADT_EXTENDED_TEMP)
		{
			brdtemp -= ADT_EXTENDED_TEMP_OFFSET;
			cputempu -= ADT_EXTENDED_TEMP_OFFSET;
		}
		cputempu = TEMP_FROM_REG(cputempu);

	#ifdef DEBUG
			cputemp = ((cputempu << 0x3 ) + ( cputempl >> 0x5 ))/8;
			sprintf(buffer, "Brd Temp: 0x%x CPU Temp: 0x%x\n", brdtemp, cputemp);
			vConsoleWrite(buffer);
	#endif

		/* FWH Location h/w strapped - May Not be defined for all boards */
		{
			/* Extended grade */
			if((brdtemp < 70) && (brdtemp > -25))
				dtest_status = E__OK;
			else
				dtest_status = E__TEMPTHRESH;
		}
	}
	return dtest_status;
}



/*****************************************************************************
 * vClosePciDevice: restore system once test complete
 *
 * RETURNS: none
 */

static void vClosePciDevice
( 
	DEVICE *psDev 
)
{
	UINT32	wPciCmd;
	PCI_PFA pfa;

	/* make sure the device IO access is disabled */
	pfa = PCI_MAKE_PFA (psDev->bBus, psDev->bDev, psDev->bFunc);
	wPciCmd = (UINT16)dPciReadReg (pfa, 0x04, REG_16);
	vPciWriteReg (pfa, 0x04, REG_16, wPciCmd & 0xFFFE);
	
} /* vClosePciDevice () */


/*****************************************************************************
 * ThermAccessTest: test function for accessing thermal controller
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (ThermAccessTest, "Thermal Sensor Access Test")
{
	TEMPERATURE_INFO* tempsensor;
	UINT32 dStatus, dtest_status = E__OK;
	DEVICE sDevice;
		
	board_service(SERVICE__BRD_GET_TEMPERATURE_INFO, NULL, &tempsensor);

	dStatus = dInitSMB (&sDevice, 1); // bInstance - Contoller instance

	dtest_status = dStatus;
	
	if (dStatus == E__OK)
	{
		// Access Thermal controller
		dtest_status = dAccessThermal(&sDevice, tempsensor);
		
#ifdef DEBUG
		sprintf(buffer, "Test status: 0x%x\n", dtest_status);
		vConsoleWrite(buffer); 
#endif		

		vClosePciDevice(&sDevice);

	}
	if(dStatus == E__DEVICE_NOT_FOUND)
		dtest_status = E__SMB_NOT_AVAIL;
	
	return dtest_status;
}


/*****************************************************************************
 * ThermThreshTest: test function for temperature within threshold level
 *
 * RETURNS: Test status
 */

TEST_INTERFACE (ThermThreshTest, "Temperature Sensor Threshold Test")
{
	TEMPERATURE_INFO* tempsensor;

	UINT32 dStatus, dtest_status = E__OK;
	DEVICE sDevice;
		
	board_service(SERVICE__BRD_GET_TEMPERATURE_INFO, NULL, &tempsensor);

	dStatus = dInitSMB (&sDevice, 1); // bInstance - Controller instance

	dtest_status = dStatus;
	
	if (dStatus == E__OK)
	{
		// Access Thermal controller
		dtest_status = dThreshThermal(&sDevice, tempsensor);
		
#ifdef DEBUG
		sprintf(buffer, "Test status: 0x%x\n", dtest_status);
		vConsoleWrite(buffer); 
#endif		

	vClosePciDevice(&sDevice);
	}
	if(dStatus == E__DEVICE_NOT_FOUND)
		dtest_status = E__SMB_NOT_AVAIL;
	
	return dtest_status;
}










UINT32 tempsenseSIO(void);
UINT32 tempsenseDTS(void);
void   tempsenseSIOInit(void);

#define AMBIENT_READ_ERROR		(E__BIT  +0x01)
#define AMBIENT_POS_THR_ERROR	(E__BIT  +0x02)
#define AMBIENT_NEG_THR_ERROR	(E__BIT  +0x03)
#define AMBIENT_NO_THRESHOLDS	(E__BIT  +0x04)
#define CPU_NO_THRESHOLDS		(E__BIT  +0x05)
#define CPU_POS_THR_ERROR		(E__BIT  +0x06)
#define CPU_NEG_THR_ERROR		(E__BIT  +0x07)
#define CPU_PROCHOT_ERROR       (E__BIT  +0x08)



TEST_INTERFACE (ThermSenseTest, "Thermal Sensor Test")
{
	UINT32 rt1, rt2;

	tempsenseSIOInit();

	rt1 = tempsenseSIO();

	rt2 = tempsenseDTS();

	if(rt2 != E__OK)
		return rt2;
	else if(rt1 != E__OK)
		return rt1;
	else
		return E__OK;
}




/*****************************************************************************
 * ThermThreshTest: test function for temperature within threshold level
 *
 * RETURNS: Test status
 */

void tempsenseSIOInit(void)
{
	UINT16 address;
	UINT8  temp;

	vIoWriteReg(0x2e, REG_8, 0x55 );
	vIoWriteReg(0x2e, REG_8, 0x07 );
	vIoWriteReg(0x2f, REG_8, 0x0a );
	vIoWriteReg(0x2e, REG_8, 0x60 );
	address = dIoReadReg(0x2f, REG_8);
	address = address << 8;
	vIoWriteReg(0x2e, REG_8, 0x61 );
	address |= dIoReadReg(0x2f, REG_8);

	vIoWriteReg(address + 0x70, REG_8, 0x40 );
	temp = dIoReadReg(address + 0x71, REG_8);
	temp |= 0x01;
	vIoWriteReg(address + 0x71, REG_8, temp );

	vDelay (25);

	vIoWriteReg(address + 0x70, REG_8, 0x26 );
	temp = dIoReadReg(address + 0x71, REG_8);

	vDelay (25);

	vIoWriteReg(address + 0x70, REG_8, 0x26 );
	temp = dIoReadReg(address + 0x71, REG_8);

	vDelay (25);
}



UINT32 tempsenseSIO(void)
{
	UINT16 address;
	UINT8  temp;
	UINT32 rt = E__OK;
	char buffer[128];
	Sensor_Threshold thresholds;

	thresholds.sensor = 1;
	rt = board_service(SERVICE__BRD_GET_THRESHOLD_INFO, NULL, &thresholds);
	if(rt != E__OK)
	{
		#ifdef DEBUG
			sprintf(buffer, "No Ambient Temperature Thresholds!\n");
			vConsoleWrite(buffer);
		#endif

		sprintf(buffer, "No Ambient Temperature Thresholds!");
		postErrorMsg(buffer);

		return AMBIENT_NO_THRESHOLDS;
	}


	vIoWriteReg(0x2e, REG_8, 0x55 );
	vIoWriteReg(0x2e, REG_8, 0x07 );
	vIoWriteReg(0x2f, REG_8, 0x0a );
	vIoWriteReg(0x2e, REG_8, 0x60 );
	address = dIoReadReg(0x2f, REG_8);
	address = address << 8;
	vIoWriteReg(0x2e, REG_8, 0x61 );
	address |= dIoReadReg(0x2f, REG_8);


#ifdef DEBUG
	sprintf(buffer, "Address:0x%x\n", address);
	vConsoleWrite(buffer);
#endif


	vIoWriteReg(address + 0x70, REG_8, 0x40 );
	temp = dIoReadReg(address + 0x71, REG_8);
	temp |= 0x01;
	vIoWriteReg(address + 0x71, REG_8, temp );
#ifdef DEBUG
	sprintf(buffer, "enable:0x%x\n\n", temp);
	vConsoleWrite(buffer);
#endif

	vDelay (25);

	vIoWriteReg(address + 0x70, REG_8, 0x26 );
	temp = dIoReadReg(address + 0x71, REG_8);

	vDelay (25);

		vIoWriteReg(address + 0x70, REG_8, 0x26 );
		temp = dIoReadReg(address + 0x71, REG_8);

	if(temp == 128)
	{
		sprintf(buffer, "Ambient Read Error!\n");
		vConsoleWrite(buffer);
		rt = AMBIENT_READ_ERROR;

		sprintf(buffer, "0x%x Ambient Read Error", rt);
		postErrorMsg(buffer);
	}
	else if(temp < 128)
	{
		sprintf(buffer, "\nAmbient temp:%dC\n", temp);
		vConsoleWrite(buffer);

		if(temp > thresholds.UCr /*95*/)
		{
			rt = AMBIENT_POS_THR_ERROR;
			sprintf(buffer, "0x%x Ambient +Ve Threshold [%dC] Exceeded! [%dC] ",
					rt, thresholds.UCr, temp);
			postErrorMsg(buffer);
		}

	}
	else
	{
		temp = 255 - temp;
		sprintf(buffer, "\nAmbient temp:-%dC\n", temp);
		vConsoleWrite(buffer);

		if(temp > thresholds.LCr /*40*/)
		{
			rt = AMBIENT_NEG_THR_ERROR;
			sprintf(buffer, "0x%x Ambient -Ve Threshold [-%dC] Exceeded! [-%dC] ",
					rt, thresholds.LCr, temp);
			postErrorMsg(buffer);
		}
	}

	return rt;
}





UINT32 tempsenseDTS(void)
{
	UINT32 msw, lsw, prochot;
	char buffer[128];
	int	readout;
	UINT32 rt;
	Sensor_Threshold thresholds;

	thresholds.sensor = 0;
	rt = board_service(SERVICE__BRD_GET_THRESHOLD_INFO, NULL, &thresholds);
	if(rt != E__OK)
	{
		#ifdef DEBUG
			sprintf(buffer, "No CPU Temperature Thresholds!\n");
			vConsoleWrite(buffer);
		#endif

		sprintf(buffer, "No CPU Temperature Thresholds!");
		postErrorMsg(buffer);

		return CPU_NO_THRESHOLDS;
	}

	vReadMsr (0x19c, &msw, &lsw);
	readout = (lsw >> 16) & 0x000000ff;
#ifdef DEBUG
	sprintf(buffer, "thermal status: MSW:0x%x, LSW:0x%x, Raw Temp:0x%x\n",
		    msw, lsw, readout);
	vConsoleWrite(buffer);
#endif

	//read temperature Target
	vReadMsr (0x1a2, &msw, &lsw);
	prochot = (lsw >> 16) & 0x000000ff;
#ifdef DEBUG
	sprintf(buffer, "temperature Target: MSW:0x%x, LSW:0x%x, Prochot:0x%x\n",
			msw, lsw, prochot);
	vConsoleWrite(buffer);
#endif

	readout = prochot-readout;

	sprintf(buffer, "\nCPU Temperature:%dC\n", readout);
	vConsoleWrite(buffer);

	if( readout > thresholds.UCr)
   	{
		vConsoleWrite("POS ERROR\n");
		sprintf(buffer, "0x%x CPU +Ve Threshold [%dC] Exceeded! [%dC] ",
		        CPU_POS_THR_ERROR, thresholds.UCr, readout);
		postErrorMsg(buffer);
		vConsoleWrite(buffer);
		return CPU_POS_THR_ERROR;
	}
#if 0
	else if( readout < (-((int)thresholds.LCr)))
	{
		vConsoleWrite("NEG ERROR\n");
		sprintf(buffer, "0x%x CPU -Ve Threshold [-%dC] Exceeded! [%dC] ",
		        CPU_NEG_THR_ERROR, thresholds.LCr, readout);
		postErrorMsg(buffer);
		vConsoleWrite(buffer);
		return CPU_NEG_THR_ERROR;
	}
#endif
	else
	{	vConsoleWrite("OK\n");
		return E__OK;
	}

}

