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
  * SPI based MRAM access spiMRAMTest.c
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
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
#include <private/sys_delay.h>



//SPI Command details
#  define INSTRUCTION_WREN				0x06
#  define INSTRUCTION_WRDIS				0x04
#  define INSTRUCTION_READ_STATUS    	0x05
#  define INSTRUCTION_WRITE_STATUS    	0x01
#  define INSTRUCTION_READ        		0x03
#  define INSTRUCTION_WRITE       		0x02
#  define INSTRUCTION_WAKE       		0xAB
#  define INSTRUCTION_SLEEP       		0xB9

#define SPI_DELAY 3
#define LPC_PCI_VENDOR_ID	0x8086
#define LPC_PCI_DEVICE_ID	0x1c4f
#define PCI_GPIO_BASE		0x48
#define GPIO_BASE_MASK		0x0000FF80


#define FRAM_EPERM     1  //Error Code Permission Issue
#define FRAM_EIO	   5  //Error Code I/O
#define FRAM_EINVAL    22 //Error Code for Invalid Argument

#define MAX_NUM_GPIO_LINES              		75
#define GPIO_OUTPUT                             0x00
#define GPIO_INPUT                              0x01


/*Register Map For GPIO - Ref  SCH External Design Spec(EDS) Pg 492 */
#define CW_GPIO_ENABLE_REG                      0x40
#define CW_GPIO_DIRECTION_SELECT_REG            0x44
#define CW_GPIO_DATA_REG                        0x48


#define BYTE_MASK					            0xff
#define MSB_MASK 					            0x80


#define SPI_MRAM 0xA000

#define SPI_MRAM_WRITE_TIMEOUT_ERROR	        (E__BIT + SPI_MRAM + 0x01)
#define SPI_MRAM_DATA_MISMATCH_ERROR            (E__BIT + SPI_MRAM + 0x02)
#define E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE (E__BIT + SPI_MRAM + 0x03)
#define E__MRAM_ERROR_EIO					    (E__BIT + SPI_MRAM + 0x04)
#define E__MRAM_EPERM						    (E__BIT + SPI_MRAM + 0x05)

#define MAX_NUM_GPIO_LINES          75
#define MAX_ADDRESSABLE_MEMORY1 (0xFFFF)

enum eSpiProtocol
{
	SPI=0,
	I2C
};

enum eSpiInterface
{
	GPIO=0,
	CPLD,
	FPGA
};

SPI_MRAM_INFO *pSpiMRAMData;
UINT32 spiPinEnable(UINT8 pin,UINT8 enable);
UINT32 dMRAM25H10Read(UINT8 *bBuffer,UINT32 dOffset, UINT32 dLength);
UINT32 dMRAM25H10Write(UINT8 *bBuffer,UINT32 dOffset,UINT32 dLength);
void spiDelay(UINT32 usecs);
void spiRxBuffer(UINT8 *rx_buf,UINT32 nbytes);
UINT8 spiRxByte(void);
UINT8 getMiso(void);
void spiTxBuffer(UINT8 *tx_buf,UINT32 nbytes);
void spiTxByte(UINT8 data);
void setMosi(UINT8 val);
void setSck(UINT8 val);
UINT8 gpio_isValid(UINT8 gpio_pin);
UINT32 spiSetValue(UINT8 pin,UINT32 value);
void vIoSetRegister(UINT32 address,UINT32 value);
UINT32 dIoGetRegister(UINT32 address);
UINT32 spiPinEnable(UINT8 pin,UINT8 enable);
void spiMRAMInit (void);
UINT32 spiGetDirection(UINT8 pin);
UINT32 spiSetDirection(UINT8 pin,UINT32 dir);
UINT32 spiGetValue(UINT8 pin);
void vMRAM25H10Sleep();
void vMRAM25H10Wake();
/*****************************************************************************\
 *
 *  TITLE:  spiMRAMInit ()
 *
 *  ABSTRACT:  This function initialise the spi interface and gets the offset of
 *				the SPI status and control register to access the SPI MRAM.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
void spiMRAMInit (void)
{
#ifdef DEBUG
	char achBuffer[80];
#endif
	UINT32 data = 0;

	board_service(SERVICE__BRD_GET_SPI_MRAM_INFO, NULL, &pSpiMRAMData);

#ifdef DEBUG
	sprintf(achBuffer,"pSpiMRAMData->GPIOBase: %#x\n",pSpiMRAMData->GPIOBase);
	vConsoleWrite(achBuffer);
	sprintf(achBuffer,"pSpiMRAMData->spiCs: %#x\n",pSpiMRAMData->spiCs);
	vConsoleWrite(achBuffer);
	sprintf(achBuffer,"pSpiMRAMData->spiClk: %#x\n",pSpiMRAMData->spiClk);
	vConsoleWrite(achBuffer);
	sprintf(achBuffer,"pSpiMRAMData->spiMosi: %#x\n",pSpiMRAMData->spiMosi);
	vConsoleWrite(achBuffer);
	sprintf(achBuffer,"pSpiMRAMData->spiMiso: %#x\n",pSpiMRAMData->spiMiso);
	vConsoleWrite(achBuffer);
#endif

	spiPinEnable(pSpiMRAMData->spiCs ,1);
	spiPinEnable(pSpiMRAMData->spiClk ,1);
	spiPinEnable(pSpiMRAMData->spiMosi,1);
	spiPinEnable(pSpiMRAMData->spiMiso,1);


	data = dIoGetRegister(pSpiMRAMData->GPIOBase + CW_GPIO_DIRECTION_SELECT_REG );
	data = data & (~(1 << (pSpiMRAMData->spiCs - 64))) & (~(1 << (pSpiMRAMData->spiMiso - 64))) & (~(1 << (pSpiMRAMData->spiMosi - 64))) & (~(1 << (pSpiMRAMData->spiClk - 64)));
	vIoSetRegister(pSpiMRAMData->GPIOBase+ CW_GPIO_DIRECTION_SELECT_REG,data);


	data = dIoGetRegister(pSpiMRAMData->GPIOBase + CW_GPIO_DATA_REG);
	data = data & (~(1 << (pSpiMRAMData->spiCs - 64))) & (~(1 << (pSpiMRAMData->spiMiso- 64))) & (~(1 << (pSpiMRAMData->spiMosi - 64))) & (~(1 << (pSpiMRAMData->spiClk - 64)));
	vIoSetRegister(pSpiMRAMData->GPIOBase + CW_GPIO_DATA_REG,data);

	spiSetDirection(pSpiMRAMData->spiCs, GPIO_OUTPUT);
	spiSetDirection(pSpiMRAMData->spiMosi,GPIO_OUTPUT);
	spiSetDirection(pSpiMRAMData->spiMiso,GPIO_INPUT);
	spiSetDirection(pSpiMRAMData->spiClk,GPIO_OUTPUT);

	data = dIoGetRegister(pSpiMRAMData->GPIOBase + CW_GPIO_ENABLE_REG);

#ifdef DEBUG
	sprintf(achBuffer,"Enable Register Val : 0x%x\n",data);
	vConsoleWrite(achBuffer);
#endif

	data = dIoGetRegister(pSpiMRAMData->GPIOBase + CW_GPIO_DATA_REG);

#ifdef DEBUG
	sprintf(achBuffer,"Data Register Val : 0x%x\n",data);
	vConsoleWrite(achBuffer);
#endif

	data = dIoGetRegister(pSpiMRAMData->GPIOBase + CW_GPIO_DIRECTION_SELECT_REG);

#ifdef DEBUG
	sprintf(achBuffer,"Direction Register Val : 0x%x\n",data);
	vConsoleWrite(achBuffer);

	sprintf(achBuffer,"Initialisation Complete\n");
	vConsoleWrite(achBuffer);
#endif

}


/******************************************************************************
*
* spigpio_pinEnable
*
* Enable the GPIO PIN
*
*
*
*
* RETURNS: 0 - Success , ERR_CAN_EINVAL - Failure
*
******************************************************************************/
UINT32 spiPinEnable(UINT8 pin,UINT8 enable)
{
	UINT32 address=0,value = 0;
#ifdef DEBUG
	char achBuffer[80];
#endif

	switch (pSpiMRAMData->wSpiInterface)
	{
		case GPIO:
#ifdef DEBUG
			sprintf(achBuffer,"Enabling GPIO Pin : %d\n",pin);
			vConsoleWrite(achBuffer);
#endif

			if(!gpio_isValid(pin))
				return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
			address = pSpiMRAMData->GPIOBase + CW_GPIO_ENABLE_REG;
			value = dIoGetRegister(address);
			pin = pin - 64;
			value = (enable ?(value | (1 << pin)):(value & ~(1 << pin)));
			vIoSetRegister(address,value);
			break;
		case FPGA:
		case CPLD:
#ifdef DEBUG
			sprintf(achBuffer,"Enabling FPGA/CPLD Pin\n");
			vConsoleWrite(achBuffer);
#endif
			address = pSpiMRAMData->GPIOBase;
			value = dIoGetRegister(address);
			value = (enable ?(value | (1 << pin)):(value & ~(1 << pin)));
			vIoSetRegister(address,value);
			break;
		default:
#ifdef DEBUG
			sprintf(achBuffer,"Unsupported Interface");
			vConsoleWrite(achBuffer);
#endif
			return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
	}
	return 0;
}



/******************************************************************************
*
* dIoGetRegister
*
* Helper function to get value into IO Port Register
*
*
*
*
* RETURNS: Value Read from Register
*
******************************************************************************/
UINT32 dIoGetRegister(UINT32 address)
{
	UINT32 value = 0;
	UINT16 wWidth=0;
#ifdef DEBUG
	char achBuffer[80];
#endif

	if(pSpiMRAMData->wSpiProtocolType == I2C)
		wWidth = pSpiMRAMData->bRegWidth;
	else
		wWidth = pSpiMRAMData->bRegWidth;

	switch (wWidth)
	{
		case REG_8:
			value = dIoReadReg((UINT16)address,REG_8);
			break;
		case REG_16:
				 value = dIoReadReg((UINT16)address,REG_16);
				 break;
			case REG_32:
				 value = dIoReadReg((UINT16)address,REG_32);
				 break;
			default:
#ifdef DEBUG
				sprintf(achBuffer,"Unsupported Reg Width\n");
				vConsoleWrite(achBuffer);
#endif
				break;
	}
	return value;
}



/******************************************************************************
*
* vIoSetRegister
*
* Helper function to set value into IO Port Register
*
*
*
*
* RETURNS: None
*
******************************************************************************/
void vIoSetRegister(UINT32 address,UINT32 value)
{
	UINT16 wWidth=0;
#ifdef DEBUG
	char achBuffer[80];
#endif

	if(pSpiMRAMData->wSpiProtocolType == I2C)
		wWidth = pSpiMRAMData->bRegWidth;
	else
		wWidth = pSpiMRAMData->bRegWidth;

		switch(wWidth)
		{
			case REG_8:
				vIoWriteReg((UINT16)address,REG_8,(UINT8)value);
				break;
			case REG_16:
				vIoWriteReg((UINT16)address,REG_16,(UINT16)value );
				break;
			case REG_32:
				vIoWriteReg((UINT16)address,REG_32,(UINT32)value);
				break;
			default:
#ifdef DEBUG
				sprintf(achBuffer,"Unsupported Reg Width\n");
				vConsoleWrite(achBuffer);
#endif
				break;
		}
}


/******************************************************************************
*
* spigpio_setValue
*
* Set the value of the GPIO PIN
*
*
*
*
* RETURNS: 0 - Success , E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE - Failure
*
******************************************************************************/
UINT32 spiSetValue(UINT8 pin,UINT32 value)
{
	UINT32 address=0,data = 0;
#ifdef DEBUG
	char achBuffer[80];
#endif

	switch(pSpiMRAMData->wSpiInterface)
	{
		case GPIO:
			if(!gpio_isValid(pin))
       		     return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;

			address = pSpiMRAMData->GPIOBase + CW_GPIO_DATA_REG;
			pin = pin - 64;
			data = dIoGetRegister(address);
			data = (value?(data | (1 << pin)):(data & ~(1 << pin)));
			vIoSetRegister(address,data);
#ifdef DEBUG
			sprintf(achBuffer,"address:%#x data:%#x\n",address,data);
			vConsoleWrite(achBuffer);
#endif

		break;
		case FPGA:
		case CPLD:
			address = pSpiMRAMData->GPIOBase;
			data = dIoGetRegister(address);
			data = (value?(data | (1 << pin)):(data & ~(1 << pin)));
			vIoSetRegister(address,data);
			break;
		default:
#ifdef DEBUG
			sprintf(achBuffer,"Unsupported Interface\n");
			vConsoleWrite(achBuffer);
#endif
			return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
	}
    return 0;
}



/******************************************************************************
*
* gpio_isValid
*
* Determine the validity of the GPIO PIN
*
*
*
*
* RETURNS: 0 if valid, 1 if invalid
*
******************************************************************************/
UINT8 gpio_isValid(UINT8 gpio_pin)
{
   return (gpio_pin < MAX_NUM_GPIO_LINES);
}


/******************************************************************************
*
* setSck
*
* Set the Value for SPI Clock
*
*
*
*
* RETURNS: NONE
*
******************************************************************************/
void setSck(UINT8 val)
{
		//vConsoleWrite("setSck");
        spiSetValue(pSpiMRAMData->spiClk, val ? 1 : 0);
}

/******************************************************************************
*
* setMosi
*
* Set the Value for SPI MOSI
*
*
*
*
* RETURNS: NONE
*
******************************************************************************/
void setMosi(UINT8 val)
{
		//vConsoleWrite("setMosi");
        spiSetValue(pSpiMRAMData->spiMosi, val ? 1 : 0);
}




/******************************************************************************
*
* getMiso
*
* Get the Value for SPI MISO
*
*
*
*
* RETURNS: status of the SPI MISO
*
******************************************************************************/
UINT8 getMiso(void)
{
        return (UINT8)spiGetValue(pSpiMRAMData->spiMiso) ? 1 : 0;
}

/******************************************************************************
*
* spiChipSelect
*
* Get the Value for SPI MISO
*
*
*
*
* RETURNS: NONE
*
******************************************************************************/
void setChipSelect(UINT8 on)
{
		//vConsoleWrite("setChipSelect");
		spiSetValue(pSpiMRAMData->spiCs, on? 1 : 0);
}


/******************************************************************************
*
* spiTxByte
*
* Transmit a Byte on SPI MOSI
*
*
*
*
* RETURNS: NONE
*
******************************************************************************/
void spiTxByte(UINT8 data)
{
         UINT8 i,bit=0;
         for(i=0;i<8;i++)
         {
                 setSck(0);
                 bit = ((data << i) & MSB_MASK);
                 setMosi(bit);
                 spiDelay(SPI_DELAY);
                 setSck(1);
         }
}


/******************************************************************************
*
* spiTxBuffer
*
* Transmit a Buffer on SPI MOSI
*
*
*
*
* RETURNS: NONE
*
******************************************************************************/
void spiTxBuffer(UINT8 *tx_buf,UINT32 nbytes)
{
	UINT32 byte=0;
#ifdef DEBUG
	UINT32 i=0;
	char achBuffer[80];
#endif
	for(byte=0;byte<nbytes;byte++)
		spiTxByte(tx_buf[byte]);
#ifdef DEBUG
	for (i=0; i<nbytes;i++)
	{
		sprintf(achBuffer,"tx_buf[%d]:%#x\t",i,tx_buf[i]);
		vConsoleWrite(achBuffer);
	}
	vConsoleWrite("\n");
#endif
}

/******************************************************************************
*
* spiRxByte
*
* Receive a Byte from SPI MISO
*
*
*
*
* RETURNS: Byte received from SPI MISO
*
******************************************************************************/
UINT8 spiRxByte(void)
{
        UINT8 data=0,i=0,bit=0;
        for(i=0;i<8;i++)
        {
                 setSck(0);
                 bit = getMiso();
                 data = data | (bit << (7 - i));
                 spiDelay(SPI_DELAY);
                 setSck(1);
        }
        return data;
}

/******************************************************************************
*
* spiRxBuffer
*
* Receive Buffer from SPI MISO
*
*
*
*
* RETURNS: Buffer received from SPI MISO
*
******************************************************************************/
void spiRxBuffer(UINT8 *rx_buf,UINT32 nbytes)
{
	UINT32 byte=0;
#ifdef DEBUG
	UINT32 i=0;
	char achBuffer[80];
#endif
	for(byte=0;byte<nbytes;byte++)
		rx_buf[byte] = spiRxByte();
#ifdef DEBUG
	for (i=0; i<nbytes;i++)
	{
		sprintf(achBuffer,"rx_buf[%d]:%#x\t",i,rx_buf[i]);
		vConsoleWrite(achBuffer);
	}
	vConsoleWrite("\n");
#endif
}

/******************************************************************************
*
* spiDelay
*
* SPI Delay function (in Microseconds)
*
*
*
*
* RETURNS: NONE
*
******************************************************************************/
void spiDelay(UINT32 usecs)
{
      sysDelayMicroseconds(usecs);
}

/******************************************************************************
*
* dMRAM25H10Write
*
* Write DATA to MRAM device
*
*
*
* RETURNS: NONE
*
******************************************************************************/

UINT32 dMRAM25H10Write(UINT8 *bBuffer,UINT32 dOffset,UINT32 dLength)
{

	UINT8 bCmd[5];
	char achBuffer[80];

	if((dOffset + dLength - 1) > MAX_ADDRESSABLE_MEMORY1)
	{
		sprintf(achBuffer,"Invalid Argument Offset + Length greater than Max Addressable Range\n");
		vConsoleWrite(achBuffer);
		return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
	}

#ifdef DEBUG
	sprintf(achBuffer,"MR25H10 : Address : 0x%x Count : 0x%x\n",dOffset,dLength);
	vConsoleWrite(achBuffer);
#endif

	setChipSelect(0);

	spiTxByte(INSTRUCTION_WREN); //Write Enable

	setChipSelect(1);

	bCmd[0] = INSTRUCTION_WRITE;
	bCmd[1] = 0x0;
	bCmd[2] = (UINT8) ((dOffset >> 8) & 0xff);
	bCmd[3] = (UINT8) (dOffset & 0xff);

	setChipSelect(0);

	spiTxBuffer(bCmd,4);  //Command

	spiTxBuffer(bBuffer,dLength); //Data

	setChipSelect(1);

	return (dLength);
}

/******************************************************************************
*
* dMRAM25H10Read
*
* Read the MRAM data
*
*
*
* RETURNS: NONE
*
******************************************************************************/

UINT32 dMRAM25H10Read(UINT8 *bBuffer,UINT32 dOffset, UINT32 dLength)
{

	UINT8 bCmd[5],i=0;
	char achBuffer[80];

	if((dOffset + dLength - 1) > MAX_ADDRESSABLE_MEMORY1)
	{
		sprintf(achBuffer,"Invalid Argument Offset + Length greater than Max Addressable Range\n");
		vConsoleWrite(achBuffer);
		return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
	}

#ifdef DEBUG
	sprintf(achBuffer,"MR25H10 : Address : 0x%x Count : 0x%x\n",dOffset,dLength);
	vConsoleWrite(achBuffer);
#endif
	bCmd[0] = INSTRUCTION_READ;
	bCmd[1] = 0x0;
	bCmd[2] = (UINT8) ((dOffset >> 8) & 0xff);
	bCmd[3] = (UINT8) (dOffset & 0xff);

	for (i=0;i<4;i++)
	{
#ifdef DEBUG
		sprintf(achBuffer,"bCmd[%d]: %d\n",i,bCmd[i]);
		vConsoleWrite(achBuffer);
#endif
	}

	setChipSelect(0);

	spiTxBuffer(bCmd,4);

	spiRxBuffer(bBuffer,dLength);

	setChipSelect(1);

	return dLength;
}

/*****************************************************************************\
 *
 *  TITLE:  SpiEEPROMTest ()
 *
 *  ABSTRACT:  This function tests the SPI EEPROM by writing and reading a pattern
 *			   to memory offseet.
 *
 * 	RETURNS: None
 *
\*****************************************************************************/
TEST_INTERFACE (SpiMRAMTest, "SPI MRAM Test")
{
	UINT32   rt = E__OK;
	UINT8    bWrDataBuffer[0x20];
	UINT8    bRdDataBuffer[0x20];
	UINT8    bSaveDataBuffer[0x20];
#ifdef DEBUG
	char	 achBuffer[128];
#endif
	UINT32	 i = 0;

		/*
		 *  Initialise the SPI EEPROM and data buffers
		 */
		memset(bWrDataBuffer,0,sizeof(bWrDataBuffer));
		memset(bRdDataBuffer,0,sizeof(bRdDataBuffer));
		memset(bSaveDataBuffer,0,sizeof(bSaveDataBuffer));

		spiMRAMInit ();

		i = 0x00;

		for (i=0x00;i<0x20;i++)
		{
			bWrDataBuffer[i] = i;
		}

		rt = dMRAM25H10Read(bSaveDataBuffer,0x0,0x20);
		vDelay(4);

		if (rt != 0x20)
		{
			return (rt);
		}

		rt = dMRAM25H10Write(bWrDataBuffer,0x0,0x20);
		vDelay(4);

		if (rt != 0x20)
		{
			return (rt);
		}

		rt = dMRAM25H10Read(bRdDataBuffer,0x0,0x20);
		vDelay(4);

		if (rt != 0x20)
		{
			return (rt);
		}

		for (i=0;i<0x20;i++)
		{
#ifdef DEBUG
					sprintf (achBuffer,"bRdDataBuffer[%d]: %d\n",i,bRdDataBuffer[i]);
					vConsoleWrite(achBuffer);
#endif
					if (bWrDataBuffer[i] != bRdDataBuffer[i])
					{
						rt = SPI_MRAM_DATA_MISMATCH_ERROR;
						break;
					}
		}

		rt = dMRAM25H10Write(bSaveDataBuffer,0x0,0x20);

		if (rt != 0x20)
				return (rt);
			else
				return (E__OK);
}

/******************************************************************************
*
* spigpio_setDirection
*
* Set the Direction of the GPIO PIN
*
*
*
*
* RETURNS: 0 - Success , ERR_CAN_EINVAL - Failure
*
******************************************************************************/
UINT32 spiSetDirection(UINT8 pin,UINT32 dir)
{
        UINT32 address=0,value = 0;
#ifdef DEBUG
        char achBuffer[80];
#endif

		switch(pSpiMRAMData->wSpiInterface)
		{
			case GPIO:
#ifdef DEBUG
				vConsoleWrite("In GPIO SPI Interface\n");
#endif
				if(!gpio_isValid(pin))
					return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;

				address = pSpiMRAMData->GPIOBase + CW_GPIO_DIRECTION_SELECT_REG;
				pin = pin - 64;
				value = dIoGetRegister(address);
				value = (value | ( dir << pin));
				vIoSetRegister(address,value);
				break;

			case FPGA:
			case CPLD:
#ifdef DEBUG
				vConsoleWrite("In FPGA/CPLD SPI Interface\n");
#endif
				address =pSpiMRAMData->GPIOBase;
				value = dIoGetRegister(address);
				value = (value | ( dir << pin));
				vIoSetRegister(address,value);
			break;
			default:
#ifdef DEBUG
				sprintf(achBuffer,"Unsupported Interface");
				vConsoleWrite(achBuffer);
#endif
				return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
		}
	return 0;
}


/******************************************************************************
*
* spigpio_getDirection
*
* Get the Direction of the GPIO PIN
*
*
*
*
* RETURNS: Direction of the GPIO PIN , ERR_CAN_EINVAL - Failure
*
******************************************************************************/

UINT32 spiGetDirection(UINT8 pin)
{
        UINT32 address=0,value = 0,val=BYTE_MASK;
#ifdef DEBUG
        char achBuffer[80];
#endif

		switch(pSpiMRAMData->wSpiInterface)
		{
			case GPIO:
#ifdef DEBUG
					vConsoleWrite("In GPIO Spi Interface\n");
#endif
			        if(!gpio_isValid(pin))
            		    return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;

					address = pSpiMRAMData->GPIOBase + CW_GPIO_DIRECTION_SELECT_REG;
			        value = dIoGetRegister(address);
					pin = pin - 64;
			        val = ((value >> pin) & 0x1);
					break;

			case FPGA:
			case CPLD:
#ifdef DEBUG
					vConsoleWrite("In CPLD/FPGA Spi Interface\n");
#endif
					address = pSpiMRAMData->GPIOBase;
					value = dIoGetRegister(address);
					val = ((value >> pin) & 0x1);
					break;
			default:
#ifdef DEBUG
					sprintf(achBuffer,"Unsupported Interface");
					vConsoleWrite(achBuffer);
#endif
					return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
		}

        return val;
}

/******************************************************************************
*
* spigpio_getValue
*
* Get the value of the GPIO PIN
*
*
*
*
* RETURNS: Status of the GPIO Pin
*
******************************************************************************/
UINT32 spiGetValue(UINT8 pin)
{
	UINT16 value=0;
	UINT32 address=0;
#ifdef DEBUG
	char achBuffer[80];
#endif

	switch(pSpiMRAMData->wSpiInterface)
	{
		case GPIO:
#ifdef DEBUG
			vConsoleWrite("In GPIO Spi Interface\n");
#endif
			if(!gpio_isValid(pin))
				return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
			address = pSpiMRAMData->GPIOBase + CW_GPIO_DATA_REG;
			pin = pin - 64;
			value = dIoGetRegister(address);
			value = (value & (1 << pin));
			break;
		case FPGA:
		case CPLD:
#ifdef DEBUG
			vConsoleWrite("In FPGA/CPLD Spi Interface\n");
#endif
			address = pSpiMRAMData->GPIOBase;
			value = dIoGetRegister(address);
			value = (value & (1 << pin));
			break;
		default:
#ifdef DEBUG
			sprintf(achBuffer,"Unsupported Interface");
			vConsoleWrite(achBuffer);
#endif
			return E__SPI_MRAM_ERROR_UNSUPPORTED_INTERFACE;
	}

	return value;
}

void vMRAM25H10Wake()
{

	setChipSelect(0);
	spiTxByte(INSTRUCTION_WAKE); //Wake MRAM
	setChipSelect(1);
}

void vMRAM25H10Sleep()
{

	setChipSelect(0);
	spiTxByte(INSTRUCTION_SLEEP); //Sleep MRAM
	setChipSelect(1);
}
