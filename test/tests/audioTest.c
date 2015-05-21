
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

/* audioTest.c - Audio Test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/audioTest.c,v 1.2 2013-11-25 11:27:34 mgostling Exp $
 * $Log: audioTest.c,v $
 * Revision 1.2  2013-11-25 11:27:34  mgostling
 * Updated to support TRB1x codecs.
 *
 * Revision 1.1  2013-09-04 07:46:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.7  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.6  2010/07/26 13:14:10  hmuneer
 * Source Cleanup
 *
 * Revision 1.5  2010/03/31 16:27:10  hmuneer
 * no message
 *
 * Revision 1.4  2009/06/11 10:11:42  swilson
 * Fix test names that were not captitalised. Put actual test name in function comments,
 *  rather than 'test Interface' - this also makes grep output cleaner when checking
 *  for test names.
 *
 * Revision 1.3  2009/06/08 09:54:44  hmuneer
 * TPA40
 *
 * Revision 1.1  2009/05/15 15:54:06  hmuneer
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
#define WAIT_CLK_HIGH	sysDelayMicroseconds (5)
#define WAIT_CLK_LOW	sysDelayMicroseconds (7)
#define EEP_WRITE		0x00
#define EEP_READ		0x01
#define AUDIO_ERROR		0x800

#define	ICH9_HDA		0x293e
#define	LYNX_POINT_HDA	0x8c20

/* typedefs */
/* constants */
/* locals */
/* globals */


/* externals */
/* forward declarations */



static UINT32* BaseAddress;


/*****************************************************************************
 * sendCodecCmd: Send comand to codec
 *
 * RETURNS: returns E__OK or error code
 */

UINT32 sendCodecCmd (UINT8 linkId, UINT8 nid, UINT16 verb, UINT8 payLoad, UINT32 *val, UINT32 *val2)
{
    UINT32    *cmdReg, *rspReg, *csReg;
    UINT32    val32, tmp32, timeout;

    /* Error Checking */
    if (BaseAddress == 0)
       return E__FAIL ;

    cmdReg = BaseAddress + 0x18 ; 
    rspReg = BaseAddress + 0x19 ;
    csReg  = BaseAddress + 0x1A ;

    /* Need to monkey with the shifts because */
    /* BORLAND implements the standard.       */
    tmp32   = linkId;
    tmp32 <<= 28    ;
    val32   = tmp32 ;

    tmp32   = nid   ;
    tmp32 <<= 20    ;
    val32  |= tmp32 ;

    tmp32   = verb  ;
    tmp32 <<= 8     ;
    val32  |= tmp32 ;

    val32  |= payLoad ;

    /* Reset the command/status register */
     *(csReg) = 0x2 ;

    /* Load the command */
    *(cmdReg) = val32; 

	*(rspReg) = *val; 

    /* Transmit the command */
	*(csReg) = 0x01; 

	
    /* Wait for the Response */
	timeout  = 256;
    while ( ((*(csReg) & 0x3) != 0x2) && (timeout != 0) ) 
		timeout--; 

	if(timeout != 0)
	{
		if (val != NULL )
			*val = *(rspReg);
		*val2 = timeout;
		return E__OK ;
	}
	else
		return AUDIO_ERROR + 1 ;
    
} /* End of function sendCodecCmd() */


/*****************************************************************************
 * enableLines: enable output lines
 *
 * RETURNS: returns E__OK or error code
 */
void  enableLines (UINT8 linkId, UINT16 deviceId)
{
    UINT32  data = 0, data2 ;
	UINT8  nid;
	
    /* Enable Headphone */
	if (deviceId == LYNX_POINT_HDA)
	{
		nid = 0x0b;
	}
	else
	{
		nid = 0x0d;
	}
	
    sendCodecCmd (linkId, nid, 0xf07, 0x00, &data, &data2) ;
    if (data & 0x40)
    {
       vConsoleWrite ("Headphone is enabled...\n") ;
    } 
	else 
	{
       vConsoleWrite ("Enabling Headphone...\n") ;
       sendCodecCmd (linkId, nid, 0x707, 0x40, NULL, &data2) ;
    
       sendCodecCmd (linkId, nid, 0xf07, 0x00, &data, &data2) ;
       if ((data & 0x40) == 0)
          vConsoleWrite ("ERROR: Cannot Enable Headphone !!!\n") ;
    } 
    
    /* Enable Line OUT */
    sendCodecCmd (linkId, 0x0e, 0xf07, 0x00, &data, &data2) ;
    if (data & 0x40)
    {
       vConsoleWrite ("Line OUT is enabled...\n") ;
    } 
	else 
	{
       vConsoleWrite ("Enabling Line OUT...\n") ;
       sendCodecCmd (linkId, 0x0e, 0x707, 0x40, NULL, &data2) ;
    
       sendCodecCmd (linkId, 0x0e, 0xf07, 0x00, &data, &data2) ;
       if ((data & 0x40) == 0)
          vConsoleWrite ("ERROR: Cannot Enable Line OUT !!!\n") ;
    } 
    
    /* Enable the Line IN */
    sendCodecCmd (linkId, 0x0f, 0xf07, 0x00, &data, &data2) ;
    if (data & 0x20)
    {
       vConsoleWrite ("Line IN is enabled...\n") ;
    } 
	else 
	{
       vConsoleWrite ("Enabling Line IN...\n") ;
       sendCodecCmd (linkId, 0x0f, 0x707, 0x20, NULL, &data2) ;
    
       sendCodecCmd (linkId, 0x0f, 0xf07, 0x00, &data, &data2) ;
       if ((data & 0x20) == 0)
          vConsoleWrite ("ERROR: Cannot Enable Line IN !!!\n") ;
    } 
    
    /* Enable the MIC */
	if (deviceId == LYNX_POINT_HDA)
	{
		nid = 0x0a;
	}
	else
	{
		nid = 0x10;
	}
	
    sendCodecCmd (linkId, nid, 0xf07, 0x00, &data, &data2) ;
    if (data & 0x20)
    {
       vConsoleWrite ("MIC is enabled...\n") ;
    } 
	else 
	{
       vConsoleWrite ("Enabling MIC...\n") ;
       sendCodecCmd (linkId, nid, 0x707, 0x20, NULL, &data2) ;
    
       sendCodecCmd (linkId, nid, 0xf07, 0x00, &data, &data2) ;
       if ((data & 0x20) == 0)
          vConsoleWrite ("ERROR: Cannot Enable MIC !!!\n") ;
    } 

} /* End of function enableLines() */



/*****************************************************************************
 * startTest: test audio codec
 *
 * RETURNS: returns E__OK or error code
 */
void startTest (UINT8 linkId, UINT16 deviceId)
{
    UINT32  data = 0, data2 ;
	UINT8   nid;

    vConsoleWrite ("\nInsert Cables into Jacks...\n") ;
    vConsoleWrite (" Press any key to stop the test \n") ;

    while (kbhit () != 1)
    {
       /* Headphone */
		if (deviceId == LYNX_POINT_HDA)
		{
			nid = 0x0b;
		}
		else
		{
			nid = 0x0d;
		}
       vConsoleWrite ("HEADPHONE = ") ;
       sendCodecCmd (linkId, nid, 0xf09, 0x00, &data, &data2) ;
       if (data & 0x80000000L)
       {
          vConsoleWrite ("YES ") ;
       } 
	   else 
	   {
          vConsoleWrite ("NO ")  ;
       }

       /* Line OUT */
       vConsoleWrite ("LINE OUT = ") ;
       sendCodecCmd (linkId, 0x0e, 0xf09, 0x00, &data, &data2) ;
       if (data & 0x80000000L)
       {
          vConsoleWrite ("YES ") ;
       } 
	   else 
	   {
          vConsoleWrite ("NO ")  ;
       }

       /* Line IN */
       vConsoleWrite ("LINE IN = ") ;
       sendCodecCmd (linkId, 0x0f, 0xf09, 0x00, &data, &data2) ;
       if (data & 0x80000000L)
       {
          vConsoleWrite ("YES ") ;
       } 
	   else
	   {
          vConsoleWrite ("NO ")  ;
       }

       /* MIC */
		if (deviceId == LYNX_POINT_HDA)
		{
			nid = 0x0a;
		}
		else
		{
			nid = 0x10;
		}
       vConsoleWrite ("MIC = ") ;
       sendCodecCmd (linkId, nid, 0xf09, 0x00, &data, &data2) ;
       if (data & 0x80000000L)
       {
          vConsoleWrite ("YES ") ;
       } 
	   else 
	   {
          vConsoleWrite ("NO ")  ;
       }
       vConsoleWrite ("\r") ;
    }
	
    vConsoleWrite ("\n")  ;

} /* End of function startTest () */




/*****************************************************************************
 * TAudioCodecTest: Test the Intel HD Audio
 *
 * RETURNS: None
 */
TEST_INTERFACE (AudioCodecTest, "Audio Codec Presence Test")
{
	UINT32   rt = E__OK, data, data1, BaseAddr;
	PCI_PFA	 pfa;
	char	 achBuffer[128];
	UINT32	 dHandle;
	PTR48	 p1;
	UINT16	 deviceId;
	UINT8	 nid;
	UINT8    linkId = 0;

	pfa = PCI_MAKE_PFA (0, 27, 0);

	deviceId = (UINT16) PCI_READ_WORD (pfa, 0x02);	
	sprintf ( achBuffer, "\n0x%x\n",deviceId);	
	vConsoleWrite (achBuffer);	

	if ((deviceId == ICH9_HDA) ||
		(deviceId == LYNX_POINT_HDA))
	{
		BaseAddr = PCI_READ_DWORD (pfa, 0x10);	
		BaseAddr &= 0xfffffff0;		
		sprintf ( achBuffer, "\n0x%x\n", BaseAddr);	
		vConsoleWrite (achBuffer);	

		if(BaseAddr != 0xfffffff0)
		{
			board_service (SERVICE__BRD_ENABLE_HDA, NULL, NULL);
	
			dHandle = dGetPhysPtr (BaseAddr, 0x4000, &p1, &BaseAddress);

			if ((dHandle != E__FAIL) )
			{
				rt = sendCodecCmd (linkId, 0, 0xf00, 0, &data, &data1);
				if ((rt != E__OK) && (deviceId == LYNX_POINT_HDA))
				{
					++linkId; // try external code on RTM
					rt = sendCodecCmd (linkId, 0, 0xf00, 0, &data, &data1);
				}
				
				if (rt == E__OK)
				{
					sprintf (achBuffer, "Codec ID = 0x%x on Link %d, wait = %d\n", data, linkId, data1) ;
					vConsoleWrite (achBuffer);	
				} 
				else 
				{
					vConsoleWrite ("ERROR: Incorrect CODEC detected !!!\n") ;
					rt = AUDIO_ERROR + 2;
				}

				if(rt == E__OK)
				{
					//read write and restore a few regs	
					if (deviceId == LYNX_POINT_HDA)
					{
						nid = 0x21;
					}
					else
					{
						nid = 0x14;
					}
					sendCodecCmd (linkId, nid, 0x3a0, 0x03, &data, &data1);
					sendCodecCmd (linkId, nid, 0xba0, 0x00, &data, &data1);
					sprintf (achBuffer, "Rsp = 0x%x, wait = %d\n", data, data1) ;
					vConsoleWrite (achBuffer);						
				}
			}
			else
			{
				rt = AUDIO_ERROR + 3;
				vConsoleWrite ("\nError allocating pointers\n");
			}
		}
		else
		{
			rt = AUDIO_ERROR + 4;
			vConsoleWrite ("\nWrong Base Address\n");
		}
	}
	else
	{
		rt = AUDIO_ERROR + 5;
		vConsoleWrite ("\nIntel HD Audio not found\n");
	}

	return (rt);
}




/*****************************************************************************
 * AudioJackTest: Test the audio jack
 *
 * RETURNS: None
 */
TEST_INTERFACE (AudioJackTest, "Audio Codec Jack Test")
{
	UINT32   rt = E__OK, data, data1, BaseAddr;
	PCI_PFA	 pfa;
	char	 achBuffer[128];
	UINT32	 dHandle;
	PTR48	 p1;
	UINT16	 deviceId;
	UINT8    linkId = 0;

	pfa = PCI_MAKE_PFA (0, 27, 0);

	deviceId =  (UINT16) PCI_READ_WORD (pfa, 0x02);	
	sprintf ( achBuffer, "\n0x%x\n", deviceId);	
	vConsoleWrite (achBuffer);	

	if ((deviceId == ICH9_HDA) ||
		(deviceId == LYNX_POINT_HDA))
	{
		BaseAddr = PCI_READ_DWORD (pfa, 0x10);	
		BaseAddr &= 0xfffffff0;		
		sprintf ( achBuffer, "\n0x%x\n", BaseAddr);	
		vConsoleWrite (achBuffer);	

		if(BaseAddr != 0xfffffff0)
		{		
			board_service (SERVICE__BRD_ENABLE_HDA, NULL, NULL);
			dHandle = dGetPhysPtr (BaseAddr, 0x4000, &p1, &BaseAddress);			

			if ((dHandle != E__FAIL) )
			{
				rt = sendCodecCmd (linkId, 0, 0xf00, 0, &data, &data1);
				if ((rt != E__OK) && (deviceId == LYNX_POINT_HDA))
				{
					++linkId; // try external code on RTM
					rt = sendCodecCmd (linkId, 0, 0xf00, 0, &data, &data1);
				}
				
				if ( rt == E__OK)
				{
					sprintf (achBuffer, "Codec ID = 0x%x on Link %d, wait = %d\n", data, linkId, data1) ;
					vConsoleWrite (achBuffer);	
				} 
				else 
				{
					vConsoleWrite ("ERROR: Incorrect CODEC detected !!!\n") ;
					rt = AUDIO_ERROR + 2;
				}

				if(rt == E__OK)
				{
					enableLines(linkId, deviceId);
					startTest(linkId, deviceId);
				}
			}
			else
			{
				rt = AUDIO_ERROR + 3;
				vConsoleWrite ("\nError allocating pointers\n");
			}
		}
		else
		{
			rt = AUDIO_ERROR + 4;
			vConsoleWrite ("\nWrong Base Address\n");
		}
	}
	else
	{
		rt = AUDIO_ERROR + 5;
		vConsoleWrite ("\nIntel HD Audio not found\n");
	}

	return rt;
}



/*****************************************************************************
 * AudioSoundTest: Test the audio sound generation
 *
 * RETURNS: None
 */
TEST_INTERFACE (AudioSoundTest, "Audio Codec Tone Generation Test")
{
	UINT32   rt = E__OK, data, data1, BaseAddr, i;
	PCI_PFA	 pfa;
	char	 achBuffer[128];
	UINT32	 dHandle;
	PTR48	 p1;
	UINT8    testType;
	UINT16   deviceId;
	UINT8	 nid;
	UINT8    linkId = 0;

	sprintf (achBuffer, "Number of parameters = %d\n", adTestParams[0]);
	vConsoleWrite (achBuffer);

	if(adTestParams[0] == 0)
		testType = 0;//default test
	else
	{
		if(adTestParams[1] == 1)
		{
			if(adTestParams[0] != 2)
				rt = E__FAIL;
			else 
				testType = 1;
		}
		else if(adTestParams[1] == 2)
		{
			if(adTestParams[0] != 4)
				rt = E__FAIL;
			else 
				testType = 2;
		}
		else
			rt = E__FAIL;
	}

	for (i = 1; i <= adTestParams[0]; i++)
	{
		sprintf (achBuffer, "%2d: 0x%08X\n", i, adTestParams[i]);
		vConsoleWrite (achBuffer);
	}

	if(rt == E__OK)
	{
		pfa = PCI_MAKE_PFA (0, 27, 0);

		deviceId = (UINT16) PCI_READ_WORD (pfa, 0x02);	
		sprintf ( achBuffer, "\n0x%x\n", deviceId);	
		vConsoleWrite (achBuffer);		

		if ((deviceId == ICH9_HDA) ||
			(deviceId == LYNX_POINT_HDA))
		{
			BaseAddr = PCI_READ_DWORD (pfa, 0x10);	
			BaseAddr &= 0xfffffff0;		
			sprintf ( achBuffer, "\n0x%x\n", BaseAddr);	
			vConsoleWrite (achBuffer);	

			if(BaseAddr != 0xfffffff0)
			{			
				board_service (SERVICE__BRD_ENABLE_HDA, NULL, NULL);
				
				dHandle = dGetPhysPtr (BaseAddr, 0x4000, &p1, &BaseAddress);			

				if ((dHandle != E__FAIL) )
				{
					rt = sendCodecCmd (linkId, 0, 0xf00, 0, &data, &data1);
					if ((rt != E__OK) && (deviceId == LYNX_POINT_HDA))
					{
						++linkId; // try external code on RTM
						rt = sendCodecCmd (linkId, 0, 0xf00, 0, &data, &data1);
					}
				
					if ( rt == E__OK)
					{
						sprintf (achBuffer, "Codec ID = 0x%x on Link %d, wait = %d\n", data, linkId, data1) ;
						vConsoleWrite (achBuffer);	
					} 
					else 
					{
						vConsoleWrite ("ERROR: Incorrect CODEC detected !!!\n") ;
						rt = AUDIO_ERROR + 2;
					}	

					if(rt == E__OK)
					{
						vConsoleWrite ("  Press any key to stop the test \n") ;
						enableLines(linkId, dHandle);					
					
						if (deviceId == LYNX_POINT_HDA)
						{
							nid = 0x21;
						}
						else
						{
							nid = 0x14;
						}
						sendCodecCmd (linkId, nid, 0x3a0, 0x03, &data, &data1);
						sendCodecCmd (linkId, nid, 0xba0, 0x00, &data, &data1);
						sprintf (achBuffer, "Rsp = 0x%x, wait = %d\n", data, data1) ;
						vConsoleWrite (achBuffer);	

						sendCodecCmd (linkId, nid, 0xf00, 0x09, &data, &data1);
						sprintf (achBuffer, "Rsp = 0x%x, wait = %d\n", data, data1) ;
						vConsoleWrite (achBuffer);	

						sendCodecCmd (linkId, nid, 0xf00, 0x12, &data, &data1);
						sprintf (achBuffer, "Rsp = 0x%x, wait = %d\n", data, data1) ;
						vConsoleWrite (achBuffer);	

						if(testType == 0)
						{
							while (kbhit () != 1)
							for(i = 1; i<256; i++)
							{						
								sendCodecCmd (linkId, nid, 0x70a, i, &data, &data1);
								vDelay(5);
							}						
						}
						else if(testType == 1)
						{
							vConsoleWrite ("\n1\n");
							sendCodecCmd (linkId, nid, 0x70a, (UINT8)adTestParams[2], &data, &data1);	
							while (kbhit () != 1);												
						}
						else if(testType == 2)
						{
							while (kbhit () != 1)
							for(i = adTestParams[2]; i<adTestParams[3]; i++)
							{						
								sendCodecCmd (linkId, nid, 0x70a, i, &data, &data1);
								vDelay(adTestParams[4]);
							}						
						}


						sendCodecCmd (linkId, nid, 0x70a, 0, &data, &data1);
					}
				}
				else
				{
					rt = AUDIO_ERROR + 3;
					vConsoleWrite ("\nError allocating pointers\n");
				}
			}
			else
			{
				rt = AUDIO_ERROR + 4;
				vConsoleWrite ("\nWrong Base Address\n");
			}
		}
		else
		{
			rt = AUDIO_ERROR + 5;
			vConsoleWrite ("\nIntel HD Audio not found\n");
		}
	}

	return (rt);
}
