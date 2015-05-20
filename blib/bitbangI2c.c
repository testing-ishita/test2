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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/bitbangI2c.c,v 1.2 2014-11-14 16:59:04 mgostling Exp $
 * $Log: bitbangI2c.c,v $
 * Revision 1.2  2014-11-14 16:59:04  mgostling
 * Added some additional debugging messages.
 *
 * Revision 1.1  2013-12-10 12:04:34  mgostling
 * Generic bitbanging I2C functions.
 *
 *
 */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/delay.h>
#include <bit/bitbangI2c.h>
#include <private/debug.h>

/* defines */
//#define DEBUG

#define I2C_CLOCK_BIT	0x01				// bit 0
#define I2C_DATA_BIT	0x02				// bit 1
#define I2C_BUS_ERROR	0x80				// bit 7

// I2C address 'modifiers'

#define I2C_WRITE	0
#define I2C_READ	1

// Low level hardware interface macros
//
// These invoke the external hardware functions through a local data structure

#define READ_I2C_HW_INPUT	(*i2cHwIface.pReadI2cHwInput)
#define READ_I2C_HW_OUTPUT	(*i2cHwIface.pReadI2cHwOutput)
#define WRITE_I2C_HW		(*i2cHwIface.pWriteI2cHw)

/* typedefs */

typedef struct _i2cHwIfaceFunction_t
{
	UINT8 (*pReadI2cHwInput) (UINT8 bMask);
	UINT8 (*pReadI2cHwOutput) (void);
	void (*pWriteI2cHw) (UINT8 bVal, UINT8 bMask);
} BITBANG_I2C_HW_IFACE;

/* locals */

static BITBANG_I2C_HW_IFACE i2cHwIface;

/* forward declarations */



//========== I2C bit-banging protocol ================================
//
// all I2C transactions are built from these component parts
//
// I2C low level component protocol routines

static UINT8 i2c_in_byte(void);
static void i2c_out_byte(UINT8 value);
static void i2c_start(void);
static void i2c_stop(void);
static void i2c_send_ack(void);
static void i2c_send_nack(void);
static UINT8 i2c_read_ack(void);

// I2C low level bit protocol routines

static void i2c_set_data_high(void);
static void i2c_set_data_low(void);
static void i2c_set_clock_high(void);
static void i2c_set_clock_low(void);
static UINT8 i2c_read_data(void);
static UINT8 i2c_read_clock(void);
static UINT8 i2c_bus_idle(void);

/*****************************************************************************
 * bitbangI2cInitHwIface: Initialise pointers to low level hardware routines
 *
 * RETURNS: None
 */
void bitbangI2cInitHwIface (UINT8 (*pReadI2cHwFuncInput)(UINT8), 
							UINT8 (*pReadI2cHwFuncOutput)(void), 
							void (*pWriteI2cHwFunc)(UINT8, UINT8))
{
	i2cHwIface.pReadI2cHwInput = pReadI2cHwFuncInput;
	i2cHwIface.pReadI2cHwOutput = pReadI2cHwFuncOutput;
	i2cHwIface.pWriteI2cHw = pWriteI2cHwFunc;
}

/*****************************************************************************
 * bitbangI2cReadData: Read a byte of data from the device
 *
 * e.g. read a byte from the device, NO command sent to device
 *      (DALLAS 1803 read pot1 - returns value of POT1)
 *      (Read a byte from PHILIPS PCF8574 I/O Extender)
 *
 * RETURNS: E__OK or error
 */
UINT32 bitbangI2cReadData(UINT8 device, UINT8 *data)
{
	UINT8 ack;
#ifdef DEBUG
	char achBuffer[80];
#endif

	i2c_start();
	i2c_out_byte(device + I2C_READ);    // select required device
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X](read)\n", device);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A000;
	}
	*data = i2c_in_byte();              // now read the data
	i2c_send_nack();
	i2c_stop();

	return E__OK;
}

/*****************************************************************************
 * bitbangI2cReadDataBlock: Read a block of data from the device
 *
 * e.g. read a series of bytes from the device, NO command sent to device
 *      (DALLAS 1803 read both pots - returns values of POT1 & POT2)
 *
 * RETURNS: E__OK or error
 */
UINT32 bitbangI2cReadDataBlock(UINT8 device, int replyLen, UINT8 *replyBuf)
{
	UINT8 ack;
	int cnt;
#ifdef DEBUG
	char achBuffer[80];
#endif

	i2c_start();
	i2c_out_byte(device + I2C_READ);	// select required device
	ack = i2c_read_ack();
	if (ack != 0 )				 		// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X](read)\n", device);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A000;
	}

	for (cnt = 1; cnt <= replyLen; ++cnt)
	{
	 	*replyBuf = i2c_in_byte();		// read data from device
		++replyBuf;
		if (cnt < replyLen)
		{
	        i2c_send_ack();             // if not the last data byte to be received - send acknowledgement
		}
	}
	i2c_send_nack();                    // end message - no more data required - tell pIC we are done
	i2c_stop();							// send stop to I2C Bus

	return E__OK;
}

/*****************************************************************************
 * bitbangI2cReadCommandData: Send a command and read a byte of data from the device
 *
 * e.g. read a byte from a specific address in an eeprom ('command' = address in this case)
 *      read the configuration register in DALLAS 1621 (command 0xAC - configure device)
 *
 * RETURNS: E__OK or error
 */
UINT32 bitbangI2cReadCommandData(UINT8 device, UINT8 command, UINT8 *data)
{
	UINT8 ack;
#ifdef DEBUG
	char achBuffer[80];
#endif

	i2c_start();
	i2c_out_byte(device + I2C_WRITE);   // select required device
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X](write)\n", device);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A000;
	}
	i2c_out_byte(command);				// send command/address
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "I2C device did not acknowledge command [%02X]\n", command);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A001;
	}
	i2c_start();                        // restart i2c sequence
	i2c_out_byte(device + I2C_READ);    // set read mode
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X](read)\n", device);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A000;
	}
	*data = i2c_in_byte();              // now read the data
	i2c_send_nack();
	i2c_stop();

	return E__OK;
}

/*****************************************************************************
 * bitbangI2cReadCommandDataBlock: Send a command and read a block of data from the device
 *
 * e.g. read a series of bytes from the device
 * 		(DALLAS 1621 command 0xAA read temperature - returns two bytes)
 *
 * RETURNS: E__OK or error
 */
UINT32 bitbangI2cReadCommandDataBlock(UINT8 device, UINT8 command, int replyLen, UINT8 *replyBuf)
{
	UINT8 ack;
	int cnt;
#ifdef DEBUG
	char achBuffer[80];
#endif

	i2c_start();
	i2c_out_byte(device + I2C_WRITE);   // select required device
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X](write)\n", device);
		i2c_stop();
		vConsoleWrite(achBuffer);
#endif
		return 0x8000A000;
	}
	i2c_out_byte(command);				// send command/address
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "I2C device did not acknowledge command [%02X]\n", command);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A001;
	}
	i2c_start();                        // restart i2c sequence
	i2c_out_byte(device + I2C_READ);    // set read mode
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X](read)\n", device);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A000;
	}
	for (cnt = 1; cnt <= replyLen; ++cnt)
	{
	 	*replyBuf = i2c_in_byte();		// read data from device
		++replyBuf;
		if (cnt < replyLen)
		{
	        i2c_send_ack();             // if not the last data byte to be received - send acknowledgement
		}
	}
	i2c_send_nack();                    // end message - no more data required - tell pIC we are done
	i2c_stop();

	return E__OK;
}

/*****************************************************************************
 * bitbangI2cWriteData: Write a byte of data to the device
 *
 * e.g. stand alone command for DALLAS 1621 (command 0xEE - start temperature conversion)
 *      write a single byte to the PHILIPS PCF8574 I/O Extender
 *
 * RETURNS: E__OK or error
 */
UINT32 bitbangI2cWriteData(UINT8 device, UINT8 data)
{
	UINT8 ack;
#ifdef DEBUG
	char achBuffer[80];
#endif

	i2c_start();
	i2c_out_byte(device + I2C_WRITE);   // select required device
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X] (write)\n", device);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A000;
	}

	i2c_out_byte(data);				    // send data [command]
	ack = i2c_read_ack();
	i2c_stop();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "I2C device did not acknowledge command/address [%02X]\n", data);
		vConsoleWrite(achBuffer);
#endif
		return 0x8000A001;
	}

	vDelay(5);  		   	   	   	    // allow time for writes to take effect
	return E__OK;
}

/*****************************************************************************
 * bitbangI2cWriteDataBlock: Write a a block of data to the device
 *
 * e.g. write a series of bytes to device
 *      (write a series of bytes to PHILIPS PCF8574 I/O Extender)
 *
 * RETURNS: E__OK or error
 */
UINT32 bitbangI2cWriteDataBlock(UINT8 device, int dataLen, UINT8 *dataBuf)
{
	UINT8 ack;
	int cnt;
#ifdef DEBUG
	char achBuffer[80];
#endif

	i2c_start();
	i2c_out_byte(device + I2C_WRITE);   // select required device
	ack = i2c_read_ack();
	if (ack != 0)						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X] (write)\n", device);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A000;
	}

	for (cnt = 1; cnt <= dataLen; ++cnt)
	{
	 	i2c_out_byte(*dataBuf);			// send data
		ack = i2c_read_ack();
		if (ack != 0)                   // continue if ACK is 0
		{
#ifdef DEBUG
		// fatal error
			sprintf(achBuffer, "I2C device did not acknowledge data [%02X]\n", *dataBuf);
			vConsoleWrite(achBuffer);
#endif
			i2c_stop();
			return 0x8000A002;
		}
		++dataBuf;
	}
	i2c_stop();							// send stop to I2C Bus

	vDelay(5);  		   	   	   	    // allow time for writes to take effect
	return E__OK;
}

/*****************************************************************************
 * bitbangI2cWriteCommandData: Write a command and a byte of data to the device
 *
 * e.g. write a byte to a specific address in an eeprom ('command' = address in this case)
 *      write to configuration register in DALLAS 1621 (command 0xAC - configure device)
 *
 * RETURNS: E__OK or error
 */
UINT32 bitbangI2cWriteCommandData(UINT8 device, UINT8 command, UINT8 data)
{
	UINT8 ack;
#ifdef DEBUG
	char achBuffer[80];
#endif

	i2c_start();
	i2c_out_byte(device + I2C_WRITE);   // select required device
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X] (write)\n", device);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A000;
	}

	i2c_out_byte(command);				// send command/address
	ack = i2c_read_ack();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "I2C device did not acknowledge command/address [%02X]\n", command);
		vConsoleWrite(achBuffer);
#endif
		i2c_stop();
		return 0x8000A001;
	}
	i2c_out_byte(data);		   	   		// send data
	ack = i2c_read_ack();
	i2c_stop();
	if (ack != 0 )						// continue if ACK is 0
	{
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "I2C device did not acknowledge data [%02X]\n", data);
		vConsoleWrite(achBuffer);
#endif
		return 0x8000A002;
	}

	vDelay(5);  		   	   	   	    // allow time for writes to take effect
	return E__OK;
}

/*****************************************************************************
 * bitbangI2cWriteCommandDataBlock: Write a command and a block of data to the device
 *
 * e.g. write a block of data starting at a specific address in an eeprom ('command' = address in this case)
 *
 * **** Beware of limitations with the size of a block that can be written in one transaction
 * **** It is up to the calling program to deal with any such limitations
 *
 * RETURNS: E__OK or error
 */
UINT32 bitbangI2cWriteCommandDataBlock(UINT8 device, UINT8 command, int dataLen, UINT8 *dataBuf)
{
    UINT8 ack;
    int cnt;
#ifdef DEBUG
	char achBuffer[80];
#endif

    i2c_start();
    i2c_out_byte(device + I2C_WRITE);   // select required device
    ack = i2c_read_ack();
    if (ack != 0 )                      // continue if ACK is 0
    {
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "Device did not respond to I2C address [%02X] (write)\n", device);
		vConsoleWrite(achBuffer);
#endif
        i2c_stop();
        return 0x8000A000;
    }

    i2c_out_byte(command);              // send command/address
    ack = i2c_read_ack();
    if (ack != 0 )                      // continue if ACK is 0
    {
#ifdef DEBUG
		// fatal error
		sprintf(achBuffer, "I2C device did not acknowledge command/address [%02X]\n", command);
		vConsoleWrite(achBuffer);
#endif
        i2c_stop();
        return 0x8000A001;
    }

    for (cnt = 1; cnt <= dataLen; ++cnt)
    {
        i2c_out_byte(*dataBuf);         // send data
        ack = i2c_read_ack();
        if (ack != 0)                   // continue if ACK is 0
        {
#ifdef DEBUG
		// fatal error
			sprintf(achBuffer, "I2C device did not acknowledge data [%02X]\n", *dataBuf);
			vConsoleWrite(achBuffer);
#endif
            i2c_stop();
            return 0x8000A002;
        }
        ++dataBuf;
    }
    i2c_stop();                         // send stop to I2C Bus

    vDelay(5);                        // allow time for writes to take effect
    return E__OK;
}

//========== I2C component protocol ================================
//
// all I2C transactions are built from these component parts
//

/*****************************************************************************
 * i2c_out_byte: Write a byte of data to the external device
 *
 * RETURNS: None
 */
static void i2c_out_byte(UINT8 value)
{
	int	cnt;

	for (cnt = 0; cnt < 8; ++cnt)
	{
		if ((value & 0x80) == 0)
			i2c_set_data_low();				// send 0
		else
			i2c_set_data_high();			// send 1

		i2c_set_clock_high();
		i2c_set_clock_low();				// clock out data bit
		value <<= 1;
	}
}

/*****************************************************************************
 * i2c_in_byte: Read a byte of data from the external device
 *
 * RETURNS: data value
 */
static UINT8 i2c_in_byte(void)
{
	UINT8 value;
	UINT8 bitVal;
	int	cnt;

	i2c_set_data_high();				// tristate data line

	// this bit is important!
	// I2C devices can hold the clock line low until ready to send data
	// so we need to tristate the CLK line and then wait for it to go high
	// before trying to read back any data

	i2c_set_clock_high();		 		// tristate the clock line
	cnt = 0;
	while ((i2c_read_clock() == 0) && (cnt < 10))
	{
		vDelay (5);
		++cnt;
	}

	if (i2c_read_clock() == 0)			// clock has not been released
	{
#ifdef DEBUG
		vConsoleWrite("I2C clock not released by target device\n");
#endif
		return 0xFF;
	}

	// now the device is ready to send back data

	value = 0;
	for (cnt = 0; cnt < 8; ++cnt)
	{
		i2c_set_clock_high();
		value <<= 1;
		bitVal = i2c_read_data();	// read data while clock is high
		if ((bitVal & I2C_BUS_ERROR) != 0)
		{
#ifdef DEBUG
			vConsoleWrite("I2C bus error reading data\n");
#endif
			value = 0xFF;
			break;
		}

		value |= bitVal;
		i2c_set_clock_low();
	}
	return value;
}

/*****************************************************************************
 * i2c_read_ack: Read the I2C ACK bit from the external device
 *
 * RETURNS: I2C ACK bit value
 */
static UINT8 i2c_read_ack(void)
{
	// I2C read acknowledge sequence

	UINT8 ack_bit;

	i2c_set_clock_low();
	i2c_set_data_high();			// data line must only change when clock is low
	i2c_set_clock_high();
	ack_bit = i2c_read_data();	// read acknowledge bit from slave
	i2c_set_clock_low();

	return ack_bit;
}

/*****************************************************************************
 * i2c_send_nack: Send the I2C NACK bit
 *
 * RETURNS: None
 */
void i2c_send_nack(void)
{
	i2c_set_clock_low();			// same as read acknowledge, but don't
	i2c_set_data_high();			// bother looking for acknowledge bit
	i2c_set_clock_high();
	i2c_set_clock_low();
}

/*****************************************************************************
 * i2c_send_ack: Send the I2C ACK bit
 *
 * RETURNS: None
 */
void i2c_send_ack(void)
{
	// I2C send acknowledge sequence

	i2c_set_clock_low();			// send acknowledge bit to slave controller
	i2c_set_data_low();
	i2c_set_clock_high();
	i2c_set_clock_low();
	i2c_set_data_high();
}

/*****************************************************************************
 * i2c_start: Send the I2C START signal
 *
 * RETURNS: None
 */
void i2c_start(void)
{
	// I2C start cycle

	if (!i2c_bus_idle())
	{
	    i2c_set_clock_low();		// these first three lines guarantee
	    i2c_set_data_high();		// that a valid RESTART sequence is  seen
	    i2c_set_clock_high();
	}
	i2c_set_data_low();				// start = data low while clock is high
	i2c_set_clock_low();			// set clock low for data transfer
}

/*****************************************************************************
 * i2c_stop: Send the I2C STOP signal and leave the bus idle
 *
 * RETURNS: None
 */
void i2c_stop(void)
{
	// I2C stop cycle

	i2c_set_clock_low();			// ensure that the data line is low (set
	i2c_set_data_low();         	// low while clock is low) so that a
	i2c_set_clock_high();       	// valid stop sequence is seen
	i2c_set_data_high();			// stop = data high while clock is high
}

//========== I2C low level protocol ================================
//
// all I2C transactions are built from these low level functions
//

/*****************************************************************************
 * i2c_set_data_high: Sets the I2C DATA signal high
 *
 * RETURNS: None
 */
void i2c_set_data_high(void)
{
	UINT8 value;

	// set DATA high - leave clock line untouched
	value = READ_I2C_HW_OUTPUT();
	value |= I2C_DATA_BIT;
	WRITE_I2C_HW(value, I2C_DATA_BIT);
}

/*****************************************************************************
 * i2c_set_data_low: Sets the I2C DATA signal low
 *
 * RETURNS: None
 */
void i2c_set_data_low(void)
{
	UINT8 value;

	// set DATA low - leave clock line untouched
	value = READ_I2C_HW_OUTPUT();
	value &= ~I2C_DATA_BIT;
	WRITE_I2C_HW(value, I2C_DATA_BIT);
}

/*****************************************************************************
 * i2c_set_clock_high: Sets the I2C CLOCK signal high
 *
 * RETURNS: None
 */
void i2c_set_clock_high(void)
{
	UINT8 value;

	// set CLOCK high - leave data line untouched
	value = READ_I2C_HW_OUTPUT();
	value |= I2C_CLOCK_BIT;
	WRITE_I2C_HW(value, I2C_CLOCK_BIT);
}

/*****************************************************************************
 * i2c_set_clock_low: Sets the I2C CLOCK signal low
 *
 * RETURNS: None
 */
void i2c_set_clock_low(void)
{
	UINT8 value;

	// set CLOCK low - leave data line untouched
	value = READ_I2C_HW_OUTPUT();
	value &= ~I2C_CLOCK_BIT;
	WRITE_I2C_HW(value, I2C_CLOCK_BIT);
}

/*****************************************************************************
 * i2c_read_clock: Reads the I2C CLOCK signal
 *
 * RETURNS: value of I2C CLOCK signal
 */
UINT8 i2c_read_clock(void)
{
	UINT8 value;

	// get current value of clock signal
	value = READ_I2C_HW_INPUT(I2C_CLOCK_BIT);
	return (value & I2C_CLOCK_BIT);		// return 0 or 1)
}

/*****************************************************************************
 * i2c_read_data: Reads the I2C DATA signal
 *
 * RETURNS: value of I2C DATA signal
 */
UINT8 i2c_read_data(void)
{
	UINT8 value;

	// get current value of data signal
	value = READ_I2C_HW_INPUT(I2C_DATA_BIT);
	if ((value & I2C_BUS_ERROR)!= 0)				// I2C bus error
		return (I2C_BUS_ERROR);
	else
		return ((value & I2C_DATA_BIT) >> 1);		// return 0 or 1 (bit 1 masked & shifted)
}

/*****************************************************************************
 * i2c_bus_idle: Check if the I2C bus is idle
 *
 * RETURNS: TRUE/FALSE
 */
UINT8 i2c_bus_idle(void)
{
	UINT8 value;

	// get current value of data and clock signals
	// bus is idle when both are high
	value = READ_I2C_HW_INPUT(I2C_DATA_BIT + I2C_CLOCK_BIT);
	if ((value & I2C_BUS_ERROR) != 0)
		return FALSE;				// I2C bus error : return bus not idle
	else
		return (value == I2C_DATA_BIT + I2C_CLOCK_BIT);		// TRUE/FALSE based on clock & data line values
}

