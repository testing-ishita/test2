#ifndef __bitbangI2c_h__
	#define __bitbangI2c_h__

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

/* bitbangI2c.h - I2C Bit-banging interface routines
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/bitbangI2c.h,v 1.1 2013-12-10 12:08:54 mgostling Exp $
 *
 * $Log: bitbangI2c.h,v $
 * Revision 1.1  2013-12-10 12:08:54  mgostling
 * Generic bitbanging I2C functions.
 *
 *
 */

/* includes */

#include <stdtypes.h>


/* defines */
#define I2C_CLOCK_BIT	0x01				// bit 0
#define I2C_DATA_BIT	0x02				// bit 1
#define I2C_BUS_ERROR	0x80				// bit 7

/* typedefs */

/* constants */

/* locals */

/* globals */

/* externals */

//========== External I2C hardware interface ================================
//
// Requirements for external hw interface routines:
//
//  I2C passed parameters values
//
//	Bit 0 of UINT8 value = I2C CLOCK signal
//	Bit 1 of UINT8 value = I2C DATA signal
//	Bit 7 set = error
//  All other bits must be set to zero
//

// obtain pointers to external hw interface routines
void bitbangI2cInitHwIface (UINT8 (*pReadI2cHwInputFunc)(UINT8), 
							UINT8 (*pReadI2cHwOutputFunc)(void), 
							void (*pWriteI2cHwFunc)(UINT8, UINT8));


// ==================== I2C high level routines ==========================
//
// **** See comment for i2cWriteCommandDataBlock function regarding limitations
//

// I2C device interface routines
// **** See comments for usage and limitations

#define bitbangI2cWriteCommand bitbangI2cWriteData

UINT32 bitbangI2cWriteData(UINT8 device, UINT8 data);
// e.g. stand alone command for DALLAS 1621 (command 0xEE - start temperature conversion)
//      write a single byte to the PHILIPS PCF8574 I/O Extender

UINT32 bitbangI2cWriteDataBlock(UINT8 device, int dataLen, UINT8 *dataBuf);
// e.g. write a series of bytes to device
//      (Write a series of bytes to PHILIPS PCF8574 I/O Extender)

UINT32 ibitbangI2cWriteCommandData(UINT8 device, UINT8 command, UINT8 data);
// e.g. write a byte to a specific address in an eeprom ('command' = address in this case)
//      write to configuration register in DALLAS 1621 (command 0xAC - configure device)

UINT32 bitbangI2cWriteCommandDataBlock(UINT8 device, UINT8 command, int dataLen, UINT8 *dataBuf);
// e.g. write a block of data starting at a specific address in an eeprom ('command' = address in this case)
//
// **** Beware of limitations with the size of a block that can be written in one transaction
// **** It is up to the program that calls this routine to deal with any such limitations
//

UINT32 bitbangI2cReadData(UINT8 device, UINT8 *data);
// e.g. read a byte from the device, NO command sent to device
//      (DALLAS 1803 read pot1 - returns value of POT1)
//      (Read a byte from PHILIPS PCF8574 I/O Extender)

UINT32 bitbangI2cReadDataBlock(UINT8 device, int replyLen, UINT8 *replyBuf);
// e.g. read a series of bytes from the device, NO command sent to device
//      (DALLAS 1803 read both pots - returns values of POT1 & POT2)

UINT32 bitbangI2cReadCommandData(UINT8 device, UINT8 command, UINT8 *data);
// e.g. read a byte from a specific address in an eeprom ('command' = address in this case)
//      read the configuration register in DALLAS 1621 (command 0xAC - configure device)

UINT32 bitbangI2cReadCommandDataBlock(UINT8 device, UINT8 command, int replyLen, UINT8 *replyBuf);
// e.g. read a series of bytes from the device
// 		(DALLAS 1621 command 0xAA read temperature - returns two bytes)

/* forward declarations */

#endif

