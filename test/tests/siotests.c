
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

/* superio.c - Super I/O related tests
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/siotests.c,v 1.1 2013-09-04 07:46:45 chippisley Exp $
 * $Log: siotests.c,v $
 * Revision 1.1  2013-09-04 07:46:45  chippisley
 * Import files into new source repository.
 *
 * Revision 1.4  2011/01/20 10:01:27  hmuneer
 * CA01A151
 *
 * Revision 1.3  2009/05/29 14:01:48  hmuneer
 * Error Codes Fixed
 *
 * Revision 1.2  2009/05/21 13:27:34  hmuneer
 * CPU Exec Test
 *
 * Revision 1.1  2009/05/18 09:25:30  hmuneer
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


/* defines */
//#define DEBUG

/* typedefs */
/* constants */
/* locals */
/* globals */
/* externals */
/* forward declarations */

/*****************************************************************************
 * wReg8BitTest: this function performs a bit-wise test on the specified 
 * 8-bit register. A marching 1/0 pattern is used, a mask value specifies
 * which bits are included in the test.
 * RETURNS : E_OK or E__... error code */
UINT32 wReg8BitTest( UINT16	dPortAddress, UINT8	bBitMask)
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
 * testSuperIODevices: Test the SuperIO Logical device presence, and perform
 *						R/W test on their config regs
 * RETURNS: error code
 */
UINT32 testSuperIODevices (SIO_Info* sio)
{	
	UINT32 rt          = E__OK;
	UINT8  countDevice = 0;
	UINT8  countReg    = 0;

	#ifdef DEBUG 
		char   achBuffer[80];
	#endif

	while( rt ==E__OK && (sio->SIODevices[countDevice].DeviceRegs != NULL) )
	{	
		#ifdef DEBUG
			sprintf(achBuffer, "\n\nTesting \"%s\" on SIO %d ", 
					sio->SIODevices[countDevice].Name,  
					sio->Num);
			vConsoleWrite (achBuffer);
		#endif

		vIoWriteReg(sio->IdxReg, REG_8, sio->LDNReg);
		vIoWriteReg(sio->DataReg, REG_8, sio->SIODevices[countDevice].DevLDN);
		
		countReg = 0;
		while( rt ==E__OK && (sio->SIODevices[countDevice].DeviceRegs[countReg].ERR_CODE != 0x00) )
		{
			#ifdef DEBUG
				sprintf(achBuffer, "\n\tTesting \"%s\" on Device \"%s\"", 
						sio->SIODevices[countDevice].DeviceRegs[countReg].Name,  
						sio->SIODevices[countDevice].Name);
				vConsoleWrite (achBuffer);
			#endif

			vIoWriteReg(sio->IdxReg, REG_8, sio->SIODevices[countDevice].DeviceRegs[countReg].RegID);
			rt = wReg8BitTest( sio->DataReg, sio->SIODevices[countDevice].DeviceRegs[countReg].Mask);

			if(rt != E__OK)	
			{		
				rt = sio->ErrCodeBase + 
					 sio->SIODevices[countDevice].ERR_CODE + 
					 sio->SIODevices[countDevice].DeviceRegs[countReg].RegID;
				#ifdef DEBUG
					vConsoleWrite ("FAILED!!!\n");	
				#endif
			}		
			#ifdef DEBUG
				else
					vConsoleWrite ("PASSED!!!");
			#endif

			countReg++;
		}

		countDevice++;
	}

	return rt;
}



/*****************************************************************************
 * testSuperIO: Test the SuperIO and perform R/W test on it's config regs
 * RETURNS: error code
 */
UINT32 testSuperIO (SIO_Info* sio)
{	
	UINT32 rt       = E__OK;
	UINT8  countReg = 0;

	#ifdef DEBUG 
		char   achBuffer[80];
	#endif

	while(rt == E__OK && (sio->GlobalRegs[countReg].ERR_CODE != 0x00))
	{
		#ifdef DEBUG
			sprintf(achBuffer, "\n\nTesting \"%s\" on SIO %d ", 
					sio->GlobalRegs[countReg].Name,  
					sio->Num);
			vConsoleWrite (achBuffer);
		#endif

		vIoWriteReg(sio->IdxReg, REG_8, sio->GlobalRegs[countReg].RegID);		
		rt = wReg8BitTest( sio->DataReg, sio->GlobalRegs[countReg].Mask);

		if(rt != E__OK)	
		{		
			rt = sio->ErrCodeBase + 0x1000 + sio->GlobalRegs[countReg].RegID;
			#ifdef DEBUG
				vConsoleWrite ("FAILED!!!\n");	
			#endif
		}	
		#ifdef DEBUG	
			else
				vConsoleWrite ("PASSED!!!");
		#endif

		countReg++;
	}

	return rt;
}



/*****************************************************************************
 * verifySuperIO: verifies the presence and ID of the SUperI/O chip
 * RETURNS: error code
 */
UINT32 verifySuperIO (SIO_Info* sio)
{	
	UINT32 rt = E__OK;
	UINT8  bTemp;

	#ifdef DEBUG 
		char   achBuffer[80];	
	#endif

	if(sio->GlobalFlags & SIO_CHK_IDX)
	{		
		vIoWriteReg (sio->IdxReg, REG_8, sio->SIDReg);
		bTemp = ((UINT8)dIoReadReg (sio->IdxReg, REG_8));			

		if(bTemp == sio->SIDReg)
			rt = E__OK;
		else
		{
			#ifdef DEBUG
				sprintf(achBuffer, "\nIDX [0x%2x] read Failed Actual[0x%2x] Expected[0x%2x] ", 
						sio->IdxReg,  bTemp, sio->SIDReg);
				vConsoleWrite (achBuffer);
			#endif
			rt = sio->ErrCodeBase + 0x05;
		}
	}

	if(rt == E__OK)
		if(sio->GlobalFlags & SIO_CHK_SID)
		{			
			vIoWriteReg (sio->IdxReg, REG_8, sio->SIDReg);
			bTemp = ((UINT8)dIoReadReg (sio->DataReg, REG_8));			
		
			if(bTemp == sio->ChipID)
				rt = E__OK;
			else
			{
				#ifdef DEBUG
					sprintf(achBuffer, "\nSID [0x%2x] read Failed Actual[0x%2x] Expected[0x%2x] ", 
							sio->SIDReg,  bTemp, sio->ChipID);
					vConsoleWrite (achBuffer);
				#endif
				rt = sio->ErrCodeBase + 0x06;
			}	
		}		

	return rt;
}



/*****************************************************************************
 * superIOTestWrapper:provides a standard interface to all SuperI/O tests
 * RETURNS: error code
 */
UINT32 superIOTestWrapper (UINT8  Snum, UINT8 Sopt)
{
	SIO_Info *SIOs;
	UINT16	 superIoNum;
	UINT32	 rt = E__OK;
	#ifdef DEBUG 
		char	 achBuffer[80];		
	#endif

	board_service(SERVICE__BRD_GET_SIO_NUM, NULL, &superIoNum);

	if(superIoNum > 0)
	{
		#ifdef DEBUG 				
			sprintf (achBuffer, "\nSuperIO Num = %2x\n", superIoNum);
			vConsoleWrite (achBuffer);
		#endif
		rt = E__OK;
	}
	else 
	{	
		#ifdef DEBUG		
			sprintf (achBuffer, "\nNO SuperI/O on this board");
			vConsoleWrite (achBuffer);	
		#endif
		rt = SIOs->ErrCodeBase + 0x01;
	}

	if(rt == E__OK)
	{
		board_service(SERVICE__BRD_GET_SIO, NULL, &SIOs);
		if(Snum > (superIoNum-1) )		
		{	
			#ifdef DEBUG			
				sprintf (achBuffer, "\nTest Error, SIO %d Does Not Exist on This Board\n", Snum);
				vConsoleWrite (achBuffer);
			#endif
			rt = SIOs->ErrCodeBase + 0x02;
		}	
		
		if(rt == E__OK)
		{
			if(SIOs[Snum].vChkRtm != NULL)
			{
				if((*SIOs[Snum].vChkRtm)() == E__OK)
				{	
					#ifdef DEBUG
						vConsoleWrite ("\nRTM with SuperI/O present\n");
					#endif			
					rt = E__OK;								
				}	
				else
				{
					#ifdef DEBUG
						vConsoleWrite ("\nRTM with SuperI/O not present\n");
					#endif					
					rt = SIOs->ErrCodeBase + 0x03;	
				}
			}
			else
				rt = E__OK;

			if(rt == E__OK)
			{				
				if(SIOs[Snum].vSIOEnable != NULL)
					(*SIOs[Snum].vSIOEnable)();					
			
				rt = verifySuperIO (&SIOs[Snum]);	
				if(rt == E__FAIL)
				{		
					rt = SIOs->ErrCodeBase + 0x04;
					#ifdef DEBUG			
						vConsoleWrite ("\nSuperI/O Not Found / Not Working\n");	
					#endif	
				}			
			}
		}			
	}
	
	//perfrom the Basic SuperI/O test
	if((rt == E__OK) && (Sopt == 0))
	{	
		rt = testSuperIO (&SIOs[Snum]);
		if(rt == E__OK)
		{
			#ifdef DEBUG 							
				sprintf (achBuffer, "\nSuperIO %d PASS!\n", SIOs[Snum].Num);
				vConsoleWrite (achBuffer);
			#endif			
		}
		else// superIO failed, because the detailed register test failed
		{	
			#ifdef DEBUG			
				sprintf (achBuffer, "\nSuperIO %d FAIL!\n", SIOs[Snum].Num);
				vConsoleWrite (achBuffer);	
			#endif		
		}
	}
	
	//perfrom the SuperI/O Device test
	if((rt == E__OK) && (Sopt == 1))
	{	
		rt = testSuperIODevices (&SIOs[Snum]);
		if(rt == E__OK)
		{
			#ifdef DEBUG 								
				sprintf (achBuffer, "\nSuperIO %d Devices Test PASS!\n", SIOs[Snum].Num); 									
				vConsoleWrite (achBuffer);
			#endif			
		}
		else// superIO failed, because the detailed register test failed
		{			
			#ifdef DEBUG			
				sprintf (achBuffer, "\nSuperIO %d Devices Test Fail FAIL!\n", SIOs[Snum].Num);
				vConsoleWrite (achBuffer);	
			#endif		
		}
	}

	if(superIoNum > 0)
	{
		if(SIOs[Snum].vSIODisbale != NULL)
			(*SIOs[Snum].vSIODisbale)();
	}	

	return (rt);
}



/*****************************************************************************
 * SuperIo2Test: Test the Super I/O 2 chip
 * RETURNS: None
 */
TEST_INTERFACE (SuperIo2Test, "SuperI/O-2 Test")
{
	return superIOTestWrapper (1, 0);
}



/*****************************************************************************
 * SuperIo1Test: Test the Super I/O 1 chip
 * RETURNS: None
 */
TEST_INTERFACE (SuperIo1Test, "SuperI/O Test")
{
	return superIOTestWrapper (0, 0);

} /* SuperIoTest */



/*****************************************************************************
 * SuperIoDeviceTest: Test the SuperI/O 2 Logical devices for this board
 * RETURNS: None
 */
TEST_INTERFACE (SuperIo2DeviceTest, "SuperI/O 2 Device Test")
{
	return superIOTestWrapper (1, 1);

} /* SuperIoTest */


/*****************************************************************************
 * SuperIoDeviceTest: Test the SuperI/O 1 Logical devices for this board
 * RETURNS: None
 */
TEST_INTERFACE (SuperIo1DeviceTest, "SuperI/O Device Test")
{
	return superIOTestWrapper (0, 1);

} /* SuperIoTest */
