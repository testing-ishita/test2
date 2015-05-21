
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

/* ledtest.c - LED Test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/NVMemTest.c,v 1.2 2015-03-05 11:46:32 hchalla Exp $
 * $Log: NVMemTest.c,v $
 * Revision 1.2  2015-03-05 11:46:32  hchalla
 * Disabling of NMI in register 0x61.
 *
 * Revision 1.1  2013-09-04 07:46:44  chippisley
 * Import files into new source repository.
 *
 * Revision 1.9  2012/02/17 11:35:16  hchalla
 * Added Support for PP 81x and PP 91x.
 *
 * Revision 1.8  2011/10/27 15:50:39  hmuneer
 * no message
 *
 * Revision 1.7  2011/03/22 13:56:49  hchalla
 * Added support for VX813 eeprom read/write.
 *
 * Revision 1.6  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.5  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.4  2009/06/10 14:45:11  hmuneer
 * smbus and eeprom update
 *
 * Revision 1.3  2009/06/08 09:55:58  hmuneer
 * TPA40
 *
 * Revision 1.2  2009/05/29 14:05:21  hmuneer
 * Error Codes Fixed
 *
 * Revision 1.1  2009/05/18 09:16:38  hmuneer
 * no message
 *
 * Revision 1.0  Feb 9 2009
 *
 *
 */


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


#define WAIT_CLK_HIGH	sysDelayMicroseconds (10) // original 5
#define WAIT_CLK_LOW	sysDelayMicroseconds (14) // original 7


#define EEP_WRITE		0x00
#define EEP_READ		0x01

#define NVMEM_BANK1_ERROR			(E__BIT + 0x01)
#define NVMEM_BANK2_ERROR			(E__BIT + 0x02)

#define NVMEM_NO_EEPROM_ERROR		(E__BIT + 0x01)
#define NVMEM_EEPROM_READ_ERROR		(E__BIT + 0x02)
#define NVMEM_EEPROM_WRITE_ERROR	(E__BIT + 0x03)
#define NVMEM_EEPROM_DATA_ERROR		(E__BIT + 0x04)

/* typedefs */
/* constants */
/* locals */
/* globals */

static EEPROM_INFO *eepromInfo;

/* externals */
/* forward declarations */

UINT8 MC24LC64_bNVReadByte(UINT16 Offset, UINT32* errCode);
UINT32 MC24LC64_NVWriteByte(UINT16 Offset, UINT8 bData);

UINT8 MC24LC16_bNVReadByte(UINT16 Offset, UINT32* errCode,UINT8 bBlock);
UINT32 MC24LC16_NVWriteByte(UINT16 Offset, UINT8 bData,UINT8 bBlock);


/*****************************************************************************
 * wReg8BitTest: this function performs a bit-wise test on the specified 
 * 8-bit register. A marching 1/0 pattern is used, a mask value specifies
 * which bits are included in the test.
 * RETURNS : E_OK or E__... error code */
UINT32 wReg8BitTestx( UINT16	dPortAddress, UINT8	bBitMask)
{
	UINT8   Backup;
	UINT8	bFixed;			/* fixed bit pattern (of those outside the mask) */
	UINT8	bPattern;		/* current test bit-pattern */
	UINT8	bBitNum;		/* register bit number */
	UINT8	bTemp;			/* like it says! */
#ifdef DEBUG
	char	achBuffer[80];	/* text formatting buffer */
#endif

	Backup = ((UINT8)dIoReadReg(dPortAddress, REG_8));
	bFixed = ((UINT8)dIoReadReg(dPortAddress, REG_8)) & ~bBitMask;

	for (bBitNum = 0; bBitNum < 8; bBitNum++)
	{
		bPattern = ((1 << bBitNum) & bBitMask);		/* Marching '1' */
		vIoWriteReg(dPortAddress, REG_8, bPattern | bFixed);
		bTemp = ((UINT8)dIoReadReg(dPortAddress, REG_8)) & bBitMask;

		if (bTemp != bPattern)
		{	
			vIoWriteReg(dPortAddress, REG_8, Backup);				
			
			#ifdef DEBUG
				sprintf (achBuffer, "Data error (1)[Addr:0x%x]: wrote %02Xh, read %02Xh",
						 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}

		bPattern = (~(1 << bBitNum) & bBitMask);	/* Marching '0' */
		vIoWriteReg(dPortAddress, REG_8, bPattern | bFixed);
		bTemp =  ((UINT8)dIoReadReg(dPortAddress, REG_8)) & bBitMask;

		if (bTemp != bPattern)
		{		
			vIoWriteReg(dPortAddress, REG_8, Backup);
			
			#ifdef DEBUG
				sprintf (achBuffer, "Data error (0)[Addr:0x%x]: wrote %02Xh, read %02Xh",
					 	 dPortAddress, bPattern, bTemp);
				vConsoleWrite (achBuffer);
			#endif
			return E__FAIL;
		}
	}
	
	vIoWriteReg(dPortAddress, REG_8, Backup);
	return (E__OK);

} /* wReg8BitTest () */





/*****************************************************************************
 * NVRAMTest: Test NVRAM
 *
 * RETURNS: None
 */
TEST_INTERFACE (NVRAMTest, "NVRAM Test")
{
	UINT32 rt = E__FAIL;
	UINT8  temp2;

	//bank 1 test
	vIoWriteReg(0x70, REG_8, 0x3d);
	temp2 = ((UINT8)dIoReadReg(0x71, REG_8));
	rt = wReg8BitTestx(0x71, 0xff);
	vIoWriteReg(0x71, REG_8, temp2);

	if(rt == E__OK)
	{
		//bank 2 test
		vIoWriteReg(0x72, REG_8, 0x3d);
		temp2 = ((UINT8)dIoReadReg(0x73, REG_8));
		rt = wReg8BitTestx(0x73, 0xff);
		vIoWriteReg(0x73, REG_8, temp2);

		if(rt!=E__OK)
			rt = NVMEM_BANK2_ERROR;
	}
	else
		rt = NVMEM_BANK1_ERROR;

	vIoWriteReg(0x70, REG_8, 0x80); // Hari: 02/03/2015
	return (rt);
}


/*****************************************************************************
 * EEPROMTest: Test the EEPROM
 *
 * RETURNS: None
 */
TEST_INTERFACE (EEPROMTest, "EEPROM Test")
{
	UINT32   rt = E__OK, err;
#ifdef DEBUG
	char	 achBuffer[128];
#endif
	UINT16	 i=0, retry;
	UINT8	 temp=0, temp2=0;
	UINT8	 pattern=0;

	board_service(SERVICE__BRD_GET_EEPROM_INFO, NULL, &eepromInfo);

	if(eepromInfo == NULL)
		return NVMEM_NO_EEPROM_ERROR;

	#ifdef DEBUG
		sprintf ( achBuffer, "\nGPIOBase 0x%x, lvl 0x%x, sel 0x%x\n", eepromInfo->GPIOBase, eepromInfo->GP_LVL, 
				 eepromInfo->GP_IO_SEL);	
		vConsoleWrite (achBuffer);	

	sprintf ( achBuffer, "\nSB 0x%x, cb 0x%x, db 0x%x\n", eepromInfo->GP_IO_SEL_BIT, 
			eepromInfo->GP_LVL_CBIT, eepromInfo->GP_LVL_DBIT);	
		vConsoleWrite (achBuffer);
	#endif


	if (eepromInfo->EEPROMSizeKbit == 64)
	{
		i       = 8165;//8174;
		pattern = 0x01;
		retry   = 0x00;
		err     = E__OK;
		while( (i < 8192) && (err == E__OK) )
		{
			temp = MC24LC64_bNVReadByte(i, &err);
			if(err == E__OK)
			{
				err = MC24LC64_NVWriteByte(i, pattern);
				if(err == E__OK)
				{
					vDelay (6);

					temp2 = MC24LC64_bNVReadByte(i, &err);
					if(err == E__OK)
					{
					#ifdef DEBUG
							sprintf ( achBuffer, "\nIdx %d, Data 0x%x, Expected 0x%x, err %x\n", i, temp2, pattern, err);
						vConsoleWrite (achBuffer);
					#endif
						if(temp2 != pattern)
							rt = NVMEM_EEPROM_DATA_ERROR;
					}
					else
					{
						#ifdef DEBUG
							sprintf ( achBuffer, "\nRead error 2 at Idx %d, Data 0x%x, err %x\n", i, temp2, err);
							vConsoleWrite (achBuffer);
						#endif
						rt = NVMEM_EEPROM_READ_ERROR;
					}

					pattern = pattern << 1;
					if(pattern == 0)
						pattern = 0x01;

					err = MC24LC64_NVWriteByte(i, temp);
					if(err != E__OK)
					{
						#ifdef DEBUG
							sprintf ( achBuffer, "\nWrite error 2 at Idx %d, Data 0x%x, err %x\n", i, temp, err);
							vConsoleWrite (achBuffer);
						#endif
						rt = NVMEM_EEPROM_WRITE_ERROR;
					}
					vDelay(6);
				}
				else
				{
					#ifdef DEBUG
						sprintf ( achBuffer, "\nWrite error 1 at Idx %d, Data 0x%x, err %x\n", i, pattern, err);
						vConsoleWrite (achBuffer);
					#endif
					rt = NVMEM_EEPROM_WRITE_ERROR;
				}
			}
			else
			{
				#ifdef DEBUG
					sprintf ( achBuffer, "\nRead error 1 at Idx %d, Data 0x%x, err %x\n", i, temp, err);
					vConsoleWrite (achBuffer);
				#endif
				rt = NVMEM_EEPROM_READ_ERROR;
			}

			if ( (rt != E__OK) && (retry < 3) )
			{
				retry ++;
				err = E__OK;
				rt  = E__OK;
	#ifdef DEBUG
				vConsoleWrite ("\n\tRetrying!\n");
	#endif
			}
			else
			{
			i++;
				retry = 0;
			}
		}
	}
	else if (eepromInfo->EEPROMSizeKbit == 16)
	{
		i       = 240;
		pattern = 0x01;
		retry   = 0x00;
		err     = E__OK;
		while( (i < 255) && (err == E__OK) )
		{
			temp = MC24LC16_bNVReadByte(i, &err,0x7);
			if(err == E__OK)
			{
				err = MC24LC16_NVWriteByte(i, pattern,0x7);
				if(err == E__OK)
				{
					vDelay (6);
				
					temp2 = MC24LC16_bNVReadByte(i, &err,0x7);
					if(err == E__OK)
					{
					#ifdef DEBUG
							sprintf ( achBuffer, "\nIdx %d, Data 0x%x, Expected 0x%x, err %x\n", i, temp2, pattern, err);
						vConsoleWrite (achBuffer);
					#endif
						if(temp2 != pattern)
							rt = NVMEM_EEPROM_DATA_ERROR;
					}
					else
					{
						#ifdef DEBUG
							sprintf ( achBuffer, "\nRead error 2 at Idx %d, Data 0x%x, err %x\n", i, temp2, err);
							vConsoleWrite (achBuffer);
						#endif
						rt = NVMEM_EEPROM_READ_ERROR;
					}

					pattern = pattern << 1;
					if(pattern == 0)
						pattern = 0x01;

					err = MC24LC16_NVWriteByte(i, temp,0x7);
					if(err != E__OK)
					{
						#ifdef DEBUG
							sprintf ( achBuffer, "\nWrite error 2 at Idx %d, Data 0x%x, err %x\n", i, temp, err);
							vConsoleWrite (achBuffer);
						#endif
						rt = NVMEM_EEPROM_WRITE_ERROR;
					}
					vDelay(6);
				}
				else
				{
					#ifdef DEBUG
						sprintf ( achBuffer, "\nWrite error 1 at Idx %d, Data 0x%x, err %x\n", i, pattern, err);
						vConsoleWrite (achBuffer);
					#endif
					rt = NVMEM_EEPROM_WRITE_ERROR;
				}
			}
			else
			{
				#ifdef DEBUG
					sprintf ( achBuffer, "\nRead error 1 at Idx %d, Data 0x%x, err %x\n", i, temp, err);
					vConsoleWrite (achBuffer);
				#endif
				rt = NVMEM_EEPROM_READ_ERROR;
			}

			if ( (rt != E__OK) && (retry < 3) )
			{
				retry ++;
				err = E__OK;
				rt  = E__OK;
	#ifdef DEBUG
				vConsoleWrite ("\n\tRetrying!\n");
	#endif
			}
			else
			{
			i++;
				retry = 0;
			}
		}
	}

	return (rt);
}






/*****************************************************************************
 * makeInput: Make GPIO an input
 * RETURNS: None */
void makeInput()
{
	UINT32   temp;

	temp  = dIoReadReg(eepromInfo->GPIOBase + eepromInfo->GP_IO_SEL, REG_32);
	temp |= eepromInfo->GP_IO_SEL_BIT;
	vIoWriteReg(eepromInfo->GPIOBase + eepromInfo->GP_IO_SEL, REG_32, temp);	 
}


/*****************************************************************************
 * makeOutput: Make GPIO an output
 * RETURNS: None */
void makeOutput()
{
	UINT32  temp;

	temp  = dIoReadReg(eepromInfo->GPIOBase + eepromInfo->GP_IO_SEL, REG_32);
	temp &= ~eepromInfo->GP_IO_SEL_BIT;
	vIoWriteReg(eepromInfo->GPIOBase + eepromInfo->GP_IO_SEL, REG_32, temp);	 
}


/*****************************************************************************
 * clk_High: Make CLK High
 * RETURNS: None */
void clk_High()
{
	UINT32   temp;
	temp  = dIoReadReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32);
	temp |= eepromInfo->GP_LVL_CBIT;
	vIoWriteReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32, temp);
}


/*****************************************************************************
 * clk_Low: Make CLK Low
 * RETURNS: None */
void clk_Low()
{
	UINT32   temp;
	temp  = dIoReadReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32);
	temp &= ~eepromInfo->GP_LVL_CBIT;
	vIoWriteReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32, temp);
}


/*****************************************************************************
 * data_High: Make Data high
 * RETURNS: None */
void data_High()
{
	UINT32   temp;
	temp  = dIoReadReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32);
	temp |= eepromInfo->GP_LVL_DBIT;
	vIoWriteReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32, temp);
}


/*****************************************************************************
 * data_Low: Make Data Low
 * RETURNS: None */
void data_Low()
{
	UINT32   temp;
	temp  = dIoReadReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32);
	temp &= ~eepromInfo->GP_LVL_DBIT;
	vIoWriteReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32, temp);
}


/*****************************************************************************
 * get_data: Read Data GPIO
 * RETURNS: None */
UINT8 get_data()
{
	UINT32   temp;
	temp  = dIoReadReg(eepromInfo->GPIOBase + eepromInfo->GP_LVL, REG_32);
	temp &= eepromInfo->GP_LVL_DBIT;
	if(temp>0)
		temp  = 1;
	else
		temp = 0;
	return ((UINT8)temp);
}


/*****************************************************************************
 * I2C_Generate_Start: Generate a start condition
 * RETURNS: None */
void I2C_Generate_Start(void)	
{   
	data_High();                 //data has to be high before the clock    
	WAIT_CLK_HIGH;	
	clk_High();
	WAIT_CLK_HIGH;
	data_Low();                 //change the data while CLK is high
	WAIT_CLK_LOW;
	clk_Low();
}


/*****************************************************************************
 * I2C_Generate_Stop: Generate a stop condition
 * RETURNS: error code */
UINT32 I2C_Generate_Stop(void)	
{
	UINT8 retry = 9;   

	clk_Low();
	WAIT_CLK_LOW;
	data_High();
	while (retry--)
	{
		if (get_data() == 0)    // while the data line is low
		{                       // continue to pulse the clock
			clk_High();
			WAIT_CLK_HIGH;
			clk_Low();
		}
		else  
		{                       // when the data line actually is high
			data_Low();        // we can now execute a valid STOP
			WAIT_CLK_HIGH;
			clk_High();
			WAIT_CLK_HIGH;
			data_High();
			return(E__OK);
		 }
	}

	clk_High();
	return (E__FAIL);
}


/*****************************************************************************
 * I2C_Send_Byte: Send a byte to EEPROM
 * RETURNS: errro code */
UINT32 I2C_Send_Byte(UINT8 byte_to_send)
{
	UINT8 retry = 5;
	UINT8 index;  // bit position counter 0x80 -> 0x40 -> ... -> 0x01
	UINT8   ack = 1;  // the acknowledge bit read from HW
   
	clk_Low();
	for(index = 0x80; index != 0; index >>= 1)  // MSB first
    {
		WAIT_CLK_LOW;
		if (byte_to_send & index)
			data_High();
		else
			data_Low();
		WAIT_CLK_LOW;
		clk_High();
		WAIT_CLK_HIGH;
		clk_Low();
    }   


	makeInput();
	while(retry>0 && (ack != 0))
	{	
		WAIT_CLK_HIGH;
		clk_High();
		WAIT_CLK_HIGH;
		ack = get_data();
		clk_Low();
		retry--;	
	}
	makeOutput();

	if(ack==0)
		return E__OK;
	else
		return E__FAIL;  
}               



/*****************************************************************************
 * I2C_Get_Byte: Get a byte from EEPROM
 * RETURNS: byte read */
UINT8 I2C_Get_Byte(UINT8 ack)
{
	unsigned char index;  // bit position counter 0x80 -> 0x40 -> ... -> 0x01
	unsigned char X;      // value being read from HW to return to caller
   

	makeInput();   // set for a read
	X       = 0;
	for(index = 0x80; index != 0; index >>= 1)     // MSB first
    {
		WAIT_CLK_HIGH;
		clk_High(); 
		WAIT_CLK_LOW;
		if (get_data() == 0x01)
			X |= index;
		clk_Low();
    }
 
	makeOutput(); 

	if(ack)
		data_High();//nack
	else
		data_Low();//ack
  
	WAIT_CLK_HIGH;
	
	return(X);     
}


/*****************************************************************************
 * MC24LC64_bNVReadByte: Read Byte from EEPROM at a particular address
 * RETURNS: byte read */
UINT8 MC24LC64_bNVReadByte(UINT16 Offset, UINT32* errCode)
{
    UINT8 bData = 0;
	UINT32 ack  = E__FAIL;

	I2C_Generate_Start();
	ack = I2C_Send_Byte(eepromInfo->ADDR + EEP_WRITE);	


	*errCode = 0;
    if(ack == E__OK)
    {
        /* write address offset MSB */
    	ack = I2C_Send_Byte((UINT8)(Offset >> 8));

        if(ack == E__OK)
        {
            /* write address offset LSB */
        	ack = I2C_Send_Byte((UINT8)(Offset));
            
        	if (ack == E__OK)
            {
                /* write command to read */
            	I2C_Generate_Start();
            	ack  = I2C_Send_Byte(eepromInfo->ADDR + EEP_READ);

            	if (ack == E__OK)
                {
                    /* read the byte (no ACK needed) */
                	bData = I2C_Get_Byte(1);
					*errCode = 0;
                }   
				else
					*errCode = 4;				
            } 
			else
				*errCode = 3;			
        } 
		else
			*errCode = 2;		
    }
	else
		*errCode = 1;
            
    ack = I2C_Generate_Stop();
	if(ack == E__FAIL)
		*errCode |= 10;

	return bData;
}



/*****************************************************************************
 * MC24LC64_NVWriteByte: Write Byte to EEPROM at a particular address
 * RETURNS: error code */
UINT32 MC24LC64_NVWriteByte(UINT16 Offset, UINT8 bData)
{
	UINT32 ack  = E__FAIL;
	char achBuffer[64];

	/* write command to write */
	I2C_Generate_Start();
	ack = I2C_Send_Byte(eepromInfo->ADDR + EEP_WRITE);

	if (ack == E__OK)
    {
    	/* write address offset MSB */
    	ack = I2C_Send_Byte((UINT8)(Offset >> 8));
        
    	if (ack == E__OK)
        {
        	/* write address offset LSB */
        	ack = I2C_Send_Byte((UINT8)(Offset));
            
        	if (ack == E__OK)
            {
            	/* write the data byte */
            	ack = I2C_Send_Byte(bData);
				if (ack != E__OK)
				{
					sprintf ( achBuffer, "\n\tDATA byte not Acked, offset %x, data %x err %x\n", Offset, bData, ack);
					vConsoleWrite (achBuffer);
            }
				else
					I2C_Generate_Stop();
        }
			else	
			{
				sprintf ( achBuffer, "\n\tAddress LSB not Acked, offset %x, data %x err %x\n", Offset, bData, ack);
				vConsoleWrite (achBuffer);
            }
        }
		else	
		{
			sprintf ( achBuffer, "\n\tAddress MSB not Acked, offset %x, data %x err %x\n", Offset, bData, ack);
			vConsoleWrite (achBuffer);
		}
    }
	else	
	{
		sprintf ( achBuffer, "\n\tEEPROM Address not Acked, offset %x, data %x err %x\n", Offset, bData, ack);
		vConsoleWrite (achBuffer);
    }	

	I2C_Generate_Stop();
	return ack;
}

/*****************************************************************************
 * MC24LC16_bNVReadByte: Read Byte from EEPROM at a particular address
 * RETURNS: byte read */
UINT8 MC24LC16_bNVReadByte(UINT16 Offset, UINT32* errCode,UINT8 bBlock)
{
    UINT8 bData = 0;
	UINT32 ack  = E__FAIL;

	I2C_Generate_Start();
	ack = I2C_Send_Byte((eepromInfo->ADDR | (bBlock<<1))+ EEP_WRITE);

	*errCode = 0;
    if(ack == E__OK)
    {
        if(ack == E__OK)
        {
            /* write address offset LSB */
        	ack = I2C_Send_Byte((UINT8)(Offset));

        	if (ack == E__OK)
            {
                /* write command to read */
            	I2C_Generate_Start();
            	ack  = I2C_Send_Byte( (eepromInfo->ADDR| (bBlock<<1)) + EEP_READ);

            	if (ack == E__OK)
                {
                    /* read the byte (no ACK needed) */
                	bData = I2C_Get_Byte(1);
					*errCode = 0;
                }
				else
					*errCode = 4;
            }
			else
				*errCode = 3;
        }
		else
			*errCode = 2;
    }
	else
		*errCode = 1;

    ack = I2C_Generate_Stop();
	if(ack == E__FAIL)
		*errCode |= 10;

	return bData;
}



/*****************************************************************************
 * MC24LC64_NVWriteByte: Write Byte to EEPROM at a particular address
 * RETURNS: error code */
UINT32 MC24LC16_NVWriteByte(UINT16 Offset, UINT8 bData,UINT8 bBlock)
{
	UINT32 ack  = E__FAIL;
	char achBuffer[64];

	/* write command to write */
	I2C_Generate_Start();
	ack = I2C_Send_Byte((eepromInfo->ADDR| (bBlock << 1)) + EEP_WRITE);

	if (ack == E__OK)
    {
       	if (ack == E__OK)
        {
        	/* write address offset LSB */
        	ack = I2C_Send_Byte((UINT8)(Offset));

        	if (ack == E__OK)
            {
            	/* write the data byte */
            	ack = I2C_Send_Byte(bData);
				if (ack != E__OK)
				{
					sprintf ( achBuffer, "\n\tDATA byte not Acked, offset %x, data %x err %x\n", Offset, bData, ack);
					vConsoleWrite (achBuffer);
            }
				else
					I2C_Generate_Stop();
        }
			else
			{
				sprintf ( achBuffer, "\n\tAddress LSB not Acked, offset %x, data %x err %x\n", Offset, bData, ack);
				vConsoleWrite (achBuffer);
            }
        }
		else
		{
			sprintf ( achBuffer, "\n\tAddress MSB not Acked, offset %x, data %x err %x\n", Offset, bData, ack);
			vConsoleWrite (achBuffer);
		}
    }
	else
	{
		sprintf ( achBuffer, "\n\tEEPROM Address not Acked, offset %x, data %x err %x\n", Offset, bData, ack);
		vConsoleWrite (achBuffer);
    }

	I2C_Generate_Stop();
	return ack;
}


