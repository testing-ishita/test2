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

/* superio.c - board-specific superio information 
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/vp91x.01x/superio.c,v 1.1 2015-01-29 11:08:24 mgostling Exp $
 * $Log: superio.c,v $
 * Revision 1.1  2015-01-29 11:08:24  mgostling
 * Import files into new source repository.
 *
 * Revision 1.2  2012/08/24 10:15:09  madhukn
 * BIT_VER Tool modified to generate new format [XorV][Version].[Revision]-[subversion]
 *
 * De-emphasis bit set for B:0 D:1 F:1 for XMC2 detection
 *
 * Test added to Board Temperature through Super IO
 *
 * Revision 1.1  2012/06/22 11:42:01  madhukn
 * Initial release
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 * */

/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <error_msg.h>
#include <bit/bit.h>
#include <bit/board_service.h>
#include <bit/io.h>
#include <bit/delay.h>
#include <bit/console.h>

/*
 * SIO Error codes start at    E__BIT 0x80000000 + 0x500
 * Global Error codes are 0x00 - 0x0F  
 * Global Reg Error codes are 0x10 to 0x1F
 * Logical Device global error codes are 0x20 to 0x2F
 * FDD Error codes are 0x30 to 0x3F
 * KBD Error codes are 0x40 to 0x4F
 * Mouse Error codes are 0x50 to 0x5F
 * Ser1 Error codes are 0x60 to 0x6F
 * Ser1 Error codes are 0x70 to 0x7F
 * */

#define SIO_ERRORE_BASE			(E__BIT + 0x10000)
#define GLOBAL_REG_ERROR_BASE	0x1000
#define FDD_ERROR_BASE			0x2000
#define KBD_ERROR_BASE			0x3000
#define MUS_ERROR_BASE			0x4000
#define SR1_ERROR_BASE			0x5000
#define SR2_ERROR_BASE			0x6000
#define TEMP_ERROR_BASE			0x8000

#define TEMP_READ_ERROR_DIODE1	(TEMP_ERROR_BASE+1)
#define TEMP_READ_ERROR_INTERNAL (TEMP_ERROR_BASE+2)
#define TEMP_READ_ERROR_DIODE2	(TEMP_ERROR_BASE+1)



	#define SUPERIO_NUM         1

	//SIO GLOBAL REGISTER DEFINATIONS
	//---------------------------------------------------------------------
	SIO_REG Global_Regs[] = { {"LDN"            ,  0X07, 0XFF, GLOBAL_REG_ERROR_BASE + 0x01},
							  {             0x00,  0x00, 0x00,                         0x00}
					        };



	//LOGICAL DEVICES ON THE SIO TO BE TESTED
	//----------------------------------------------------------------------
	SIO_DEVICE_REG Floppy_Regs[] = { {"FDD BASE ADDR 1" , 0x60, 0x07, 0x01},
									 {"FDD BASE ADDR 2" , 0x61, 0xFA, 0x02},
									 {                 0,    0,    0, 0x00}
								   };

	SIO_DEVICE_REG KBD_Regs[]    = { {                    0,    0,    0, 0x00}
								   };

	SIO_DEVICE_REG SER1_Regs[]   = { {"SER1 BASE ADDR 1", 0x60, 0x07, 0x01},
									 {"SER1 BASE ADDR 2", 0x61, 0xF8, 0x02},
									 {                 0,    0,    0, 0x00}
								   };

	SIO_DEVICE_REG SER2_Regs[]   = { {"SER2 BASE ADDR 1", 0x60, 0x07, 0x01},
									 {"SER2 BASE ADDR 2", 0x61, 0xF8, 0x02},
									 {                 0,    0,    0, 0x00}
								   };

	SIO_DEVICE_REG SER3_Regs[]   = { {"SER3 BASE ADDR 1", 0x60, 0x07, 0x01},
									 {"SER3 BASE ADDR 2", 0x61, 0xF8, 0x02},
									 {                 0,    0,    0, 0x00}
								   };

	SIO_Device SIO_Devices[]     = { {"Floppy Disk Drive", 0x00, Floppy_Regs, FDD_ERROR_BASE},
									 {"Keyboard"         , 0x07, KBD_Regs   , KBD_ERROR_BASE},
									 {"Serial Port 1"    , 0x04, SER1_Regs  , SR1_ERROR_BASE},
									 {"Serial Port 2"    , 0x05, SER2_Regs  , SR2_ERROR_BASE},
									 {"Serial Port 3"    , 0x0B, SER3_Regs  , SR2_ERROR_BASE},
									 {               0x00, 0x00,        0x00,           0x00}
								   };



	void SIOEnable()
	{
		vIoWriteReg(0x2e, REG_8, 0x55);
	}

	void SIODisable()
	{
		vIoWriteReg(0x2e, REG_8, 0xAA);
	}

	/*****************************************************************************
	* brdChkRtm: checks for the presence of RTM
	* This is a board and chip specific function thats why it is kept here
	* RETURNS: error code */
	UINT32 brdChkRtm ()
	{
		return E__OK;
	} 

		SIO_Info  SIOInfo[SUPERIO_NUM] = {
										{
										 0,							// SIO number
										 0x2e,						// IdxReg
										 0x2f,						// DataReg
										 0x07,						// LDNReg
									 	 0x20,						// SIDReg
										 0x7D,						// ChipID
										 SIOEnable,					// SIO enable function
										 SIODisable,				// SIO Disbale function
										 NULL,						// RTM presence check function
										 SIO_CHK_IDX | SIO_CHK_SID,	// GLobal Flags
										 Global_Regs,				// Global Regs to check
									 	 SIO_Devices,				// Logical devices to check
										 SIO_ERRORE_BASE
										}
									};


/*****************************************************************************
 * brdGetSuperIO: returns the SIO global data structure
 * RETURNS: SIO_Info* */
UINT32 brdGetSuperIO(void *ptr)
{
	*((SIO_Info**)ptr) = SIOInfo;

	return E__OK;
}



/*****************************************************************************
 * brdGetSuperIONum: returns the number of superio chips on this board
 * RETURNS: number of superio chips */
UINT32 brdGetSuperIONum (void *ptr)
{
	*((UINT16*)ptr) = SUPERIO_NUM;

	return E__OK;

} /* brdGetSuperIONum () */





/*****************************************************************************
 * ThermThreshTest: test function for temperature within threshold level
 *
 * RETURNS: Test status
 */

void BoardTempSIOInit(void)
{
	UINT16 address;
	UINT8  temp;

	vIoWriteReg(0x2e, REG_8, 0x55 );
	vIoWriteReg(0x2e, REG_8, 0x07 );
	vIoWriteReg(0x2f, REG_8, 0x0a );
	vIoWriteReg(0x2e, REG_8, 0x60 );
	vIoWriteReg(0x2f, REG_8, 0x6);
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




UINT32 BoardTempSIO(void)
{
	UINT16 address;
	UINT8  temp;
	UINT32 rt = E__OK;
	char buffer[128];


	vIoWriteReg(0x2e, REG_8, 0x55 );
	vIoWriteReg(0x2e, REG_8, 0x07 );
	vIoWriteReg(0x2f, REG_8, 0x0a );
	vIoWriteReg(0x2e, REG_8, 0x60 );
	vIoWriteReg(0x2f, REG_8, 0x6);
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
	//Read Board Temperature at Diode 1
	vIoWriteReg(address + 0x70, REG_8, 0x25 );
	temp = dIoReadReg(address + 0x71, REG_8);

	vDelay (25);

	vIoWriteReg(address + 0x70, REG_8, 0x25 );
	temp = dIoReadReg(address + 0x71, REG_8);

	if(temp == 128)
	{
		sprintf(buffer, "Temperature Read Error!\n");
		vConsoleWrite(buffer);
		rt = TEMP_READ_ERROR_DIODE1;

		sprintf(buffer, "0x%x Temperature Read Error", rt);
		postErrorMsg(buffer);
		return rt;
	}
	else if(temp < 128)
	{
		sprintf(buffer, "\nBoard Temperature at Diode-1:%d.C\n", temp);
		vConsoleWrite(buffer);
	}
	else
	{
		temp = 255 - temp;
		sprintf(buffer, "\nBoard Temperature at Diode-1:-%d.C\n", temp);
		vConsoleWrite(buffer);
	}

	
	//Read SuperIO Internal Temperature
	vIoWriteReg(address + 0x70, REG_8, 0x26 );
	temp = dIoReadReg(address + 0x71, REG_8);

	vDelay (25);

	vIoWriteReg(address + 0x70, REG_8, 0x26 );
	temp = dIoReadReg(address + 0x71, REG_8);

	if(temp == 128)
	{
		sprintf(buffer, "Temperature Read Error!\n");
		vConsoleWrite(buffer);
		rt = TEMP_READ_ERROR_INTERNAL;

		sprintf(buffer, "0x%x Temperature Read Error", rt);
		postErrorMsg(buffer);
		return rt;
	}
	else if(temp < 128)
	{
		sprintf(buffer, "\nSuperIO Internal Temperature:%d.C\n", temp);
		vConsoleWrite(buffer);
	}
	else
	{
		temp = 255 - temp;
		sprintf(buffer, "\nSuperIO Internal Temperature:-%d.C\n", temp);
		vConsoleWrite(buffer);
	}

	
	//Read Board Temperature at Diode-2
	
	vIoWriteReg(address + 0x70, REG_8, 0x27 );
	temp = dIoReadReg(address + 0x71, REG_8);

	vDelay (25);

	vIoWriteReg(address + 0x70, REG_8, 0x27 );
	temp = dIoReadReg(address + 0x71, REG_8);

	if(temp == 128)
	{
		sprintf(buffer, "Temperature Read Error!\n");
		vConsoleWrite(buffer);
		rt = TEMP_READ_ERROR_DIODE2;

		sprintf(buffer, "0x%x Temperature Read Error", rt);
		postErrorMsg(buffer);
		return rt;
	}
	else if(temp < 128)
	{
		sprintf(buffer, "\nBoard Temperature at Diode-2:%d.C\n", temp);
		vConsoleWrite(buffer);
	}
	else
	{
		temp = 255 - temp;
		sprintf(buffer, "\nnBoard Temperature at Diode-2:-%d.C\n", temp);
		vConsoleWrite(buffer);
	}

	return rt;
}


TEST_INTERFACE (BoardTemperatureSense, "Board Temperature Sense Test")
{
	UINT32 rt;
	
	board_service(SERVICE__BRD_STORE_CONFIG_CHANGES, NULL, NULL);

	board_service(SERVICE__BRD_ENABLE_WD_LPC_IOWINDOW, NULL, NULL);

	BoardTempSIOInit();

	rt = BoardTempSIO();

	board_service(SERVICE__BRD_RESTORE_CONFIG_CHANGES, NULL, NULL);

	return rt;
}

