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
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/am92x.x1x/superio.c,v 1.1 2013-09-04 07:13:29 chippisley Exp $
 * $Log: superio.c,v $
 * Revision 1.1  2013-09-04 07:13:29  chippisley
 * Import files into new source repository.
 *
 * Revision 1.1  2012/02/17 11:26:51  hchalla
 * Initial version of PP 91x sources.
 *
 *
 * */

/* includes */

#include <stdtypes.h>
#include <bit/bit.h>
 
#include <bit/board_service.h>
#include <bit/io.h>
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


