
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

/* audioTest2.c - Audio Test
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/audioTest2.c,v 1.3 2014-03-12 11:04:05 cdobson Exp $
 * $Log: audioTest2.c,v $
 * Revision 1.3  2014-03-12 11:04:05  cdobson
 * Tidy up.
 *
 * Revision 1.2  2014-03-07 09:57:50  cdobson
 * Added AlwaysFitted field to HDA_CODEC. So we dont error on optional
 * codecs which are not fitted.
 *
 * Revision 1.1  2014-03-06 16:29:35  cdobson
 * New test based on audioTest.c. Uses configuration data from board directory.
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

#include <devices/hda.h>


/* defines */
//#define DEBUG
#define WAIT_CLK_HIGH	sysDelayMicroseconds (5)
#define WAIT_CLK_LOW	sysDelayMicroseconds (7)
#define EEP_WRITE		0x00
#define EEP_READ		0x01
#define AUDIO_ERROR		0x800

/* typedefs */
/* constants */
/* locals */
/* globals */


/* externals */
/* forward declarations */



static UINT32* dBaseAddress;


/*****************************************************************************
 * bGetNid: Get the NID for the required port type from the list of widgets
 *
 * RETURNS: NID or 0xff on error
 */
static UINT8 bGetNid (HDA_WIDGET *pWidgets, UINT8 bFunction)
{
  while ((pWidgets->Nid != 0) && (pWidgets->Function != 0))
  {
    if (pWidgets->Function == bFunction)
      return pWidgets->Nid;
    pWidgets++;
  }
  
  return 0xff;
}

/*****************************************************************************
 * vGetCodec: Get the codec entry matching the codec bus address
 *
 * RETURNS: None; *Codec is either NULL or pointer to codec 
 */
static void vGetCodec (HDA_DEVICE *HdaDevice, UINT8 bLinkId, HDA_CODEC **Codec)
{
  HDA_CODEC *CodecList;
  int n = 0;
  
  *Codec = NULL;
  
  CodecList = HdaDevice->CodecList;
  
  /* search list of codecs for matching hda link ID */
   
  while (CodecList[n].CodecId != 0)  
  {
    if (CodecList[n].LinkId == bLinkId)
    {
      *Codec = &CodecList[n];
      return;      
	}
	n++;
  }
}

/*****************************************************************************
 * dHdaSendCodecCmd: Send comand to codec
 *
 * RETURNS: returns E__OK or error code
 */

UINT32 dHdaSendCodecCmd (UINT8 bLinkId, UINT8 bNid, UINT16 wVerb, UINT8 bPayLoad, UINT32 *dVal, UINT32 *dVal2)
{
    UINT32    *cmdReg, *rspReg, *csReg;
    UINT32    val32, tmp32, timeout;

    /* Error Checking */
    
    if (dBaseAddress == 0)
       return E__FAIL ;

    cmdReg = dBaseAddress + 0x18 ; 
    rspReg = dBaseAddress + 0x19 ;
    csReg  = dBaseAddress + 0x1A ;

    /* Need to monkey with the shifts because */
    /* BORLAND implements the standard.       */
    
    tmp32   = bLinkId;
    tmp32 <<= 28    ;
    val32   = tmp32 ;

    tmp32   = bNid   ;
    tmp32 <<= 20    ;
    val32  |= tmp32 ;

    tmp32   = wVerb  ;
    tmp32 <<= 8     ;
    val32  |= tmp32 ;

    val32  |= bPayLoad ;

    /* Reset the command/status register */
    
     *(csReg) = 0x2 ;

    /* Load the command */
    
    *(cmdReg) = val32; 

	*(rspReg) = *dVal; 

    /* Transmit the command */
    
	*(csReg) = 0x01; 

	
    /* Wait for the Response */
    
	timeout  = 256;
    while ( ((*(csReg) & 0x3) != 0x2) && (timeout != 0) ) 
		timeout--; 

	if(timeout != 0)
	{
		if (dVal != NULL )
			*dVal = *(rspReg);
		*dVal2 = timeout;
		return E__OK ;
	}
	else
		return AUDIO_ERROR + 1 ;
    
} /* End of function dHdaSendCodecCmd() */


/*****************************************************************************
 * vHdaEnableWidget: enable a pin widget
 *
 * RETURNS: None
 */
void vHdaEnableWidget (UINT8 bLinkId, UINT8 bNid, UINT8 bDataIn, char *pszText)
{
    UINT32  dData1 = 0, dData2 ;
	char	achBuffer[128];
    
    dHdaSendCodecCmd (bLinkId, bNid, 0xf07, 0x00, &dData1, &dData2) ;
    if (dData1 & 0x40)
    {
    	sprintf (achBuffer, "%s is enabled...\n", pszText);
    	vConsoleWrite (achBuffer) ;
    } 
	else 
	{
    	sprintf (achBuffer, "Enabling %s...\n", pszText);
		vConsoleWrite (achBuffer) ;
    	dHdaSendCodecCmd (bLinkId, bNid, 0x707, bDataIn, NULL, &dData2) ;
    
   		dHdaSendCodecCmd (bLinkId, bNid, 0xf07, 0x00, &dData1, &dData2) ;
		if ((dData1 & bDataIn) == 0)
		{
       		sprintf (achBuffer, "ERROR: Cannot Enable %s !!!\n", pszText) ;
    		vConsoleWrite (achBuffer) ;
       	}
   	}
 }
 
/*****************************************************************************
 * HdaEnableLines: enable pin widgets
 *
 * RETURNS: None
 */
void vHdaEnableLines (
  UINT8 bLinkId,				// codec bus address
  HDA_WIDGET *pWidgetList	// array of pin widgets
)
{
	UINT8  bNid;
	
    /* Enable Headphone */
    
    bNid = bGetNid(pWidgetList, HDA_HEADPHONE);
    if (bNid != 0xff) 
    {
	    vHdaEnableWidget (bLinkId, bNid, 0x40, "Headphone") ;
	} 
    
    /* Enable Line OUT */
    
    bNid = bGetNid(pWidgetList, HDA_LINE_OUT);
    if (bNid != 0xff)
    {
	    vHdaEnableWidget (bLinkId, bNid, 0x40, "Line OUT") ;
	} 
    
    /* Enable the Line IN */
    
    bNid = bGetNid(pWidgetList, HDA_LINE_IN);
    if (bNid != 0xff)
    {
	    vHdaEnableWidget (bLinkId, bNid, 0x20, "Line IN") ;
	} 
    
    /* Enable the MIC */
    
    bNid = bGetNid(pWidgetList, HDA_MICROPHONE);
    if (bNid != 0xff)
    {
	    vHdaEnableWidget (bLinkId, bNid, 0x20, "Microphone") ;
	} 

} /* End of function enableLines() */


/*****************************************************************************
 * HdaShowPresence: print jack presence
 *
 * RETURNS: None
 */
static void HdaShowPresence (UINT8 bLinkId, UINT8 bNid)
{
    UINT32  dData = 0, dData2 ;

	dHdaSendCodecCmd (bLinkId, bNid, 0xf09, 0x00, &dData, &dData2) ;
    if (dData & 0x80000000L)
    {
	    vConsoleWrite ("YES ") ;
	} 
	else 
	{
	   	vConsoleWrite ("NO ")  ;
	}
}

/*****************************************************************************
 * HdaStartTest: test audio codec
 *
 * RETURNS: None
 */
void vHdaStartTest (
  UINT8 bLinkId,             // codec bus address
  HDA_WIDGET *pWidgetList	// array of pin widgets
)
{
	UINT8   bNid;

    vConsoleWrite ("\nInsert Cables into Jacks...\n") ;
    vConsoleWrite (" Press any key to stop the test \n") ;

    while (kbhit () != 1)
    {
    	/* Headphone */
    	
		bNid = bGetNid(pWidgetList, HDA_HEADPHONE);
		if (bNid != 0xff)
		{
    		vConsoleWrite ("HEADPHONE = ") ;
    		HdaShowPresence (bLinkId, bNid);
	    }

    	/* Line OUT */
    	
		bNid = bGetNid(pWidgetList, HDA_LINE_OUT);
		if (bNid != 0xff)
		{
    		vConsoleWrite ("LINE OUT = ") ;
    		HdaShowPresence (bLinkId, bNid);
    	}

    	/* Line IN */
    	
		bNid = bGetNid(pWidgetList, HDA_LINE_IN);
		if (bNid != 0xff)
		{
    		vConsoleWrite ("LINE IN = ") ;
    		HdaShowPresence (bLinkId, bNid);
    	}

    	/* MIC */
    	
		bNid = bGetNid(pWidgetList, HDA_MICROPHONE);
		if (bNid != 0xff)
		{
    		vConsoleWrite ("MIC = ") ;
    		HdaShowPresence (bLinkId, bNid);
    	}
    	vConsoleWrite ("\r") ;
    }
	
    vConsoleWrite ("\n")  ;

} /* End of function vHdaStartTest () */




/*****************************************************************************
 * TAudioCodecTest: Test the Intel HD Audio
 *
 * RETURNS: None
 */
TEST_INTERFACE (HdaAudioCodecTest, "Audio Codec Presence Test")
{
	UINT32   dRt = E__OK, dData, dData1, BaseAddr;
	PCI_PFA	 pfa;
	char	 achBuffer[128];
	UINT32	 dHandle;
	PTR48	 p1;
	UINT16	 wDeviceId;
	UINT8	 bNid;
	UINT8    bLinkId = 0;
	HDA_DEVICE *pHdaList;
	HDA_CODEC  *pHdaCodec;

	board_service (SERVICE__BRD_GET_HDA_LIST, NULL, (void *)&pHdaList);
	pfa = pHdaList->pfa;

	wDeviceId = (UINT16) PCI_READ_WORD (pfa, 0x02);	
	sprintf ( achBuffer, "\nHDA PCI devid 0x%x\n", wDeviceId);	
	vConsoleWrite (achBuffer);	

	if (wDeviceId != 0xffff)
	{
		BaseAddr = PCI_READ_DWORD (pfa, 0x10);	
		BaseAddr &= 0xfffffff0;		
		sprintf ( achBuffer, "PCI BAR0 0x%x\n", BaseAddr);	
		vConsoleWrite (achBuffer);	

		if(BaseAddr != 0xfffffff0)
		{
			board_service (SERVICE__BRD_ENABLE_HDA, NULL, &pfa);

			dHandle = dGetPhysPtr (BaseAddr, 0x4000, &p1, &dBaseAddress);

			if (dHandle != E__FAIL)
			{
				while ((bLinkId < HDA_MAX_CODECS) && (dRt == E__OK))
				{
					vGetCodec (pHdaList, bLinkId, &pHdaCodec);
					/* read the codec id reg */
					dRt = dHdaSendCodecCmd (bLinkId, 0, 0xf00, 0, &dData, &dData1);
				
					if (dRt == E__OK)
					{
						// codec found, do we know about it ?
						if (pHdaCodec != NULL)
						{
							if (pHdaCodec->CodecId == dData)
							{
								sprintf (achBuffer, "Codec ID = 0x%x on Link %d, wait = %d\n", dData, bLinkId, dData1) ;
							}
							else
							{
								sprintf (achBuffer, "ERROR: Expected Codec ID 0x%x found %x on Link %d\n", 
												pHdaCodec->CodecId, dData, bLinkId) ;
								dRt = AUDIO_ERROR + 2;
							}
							vConsoleWrite (achBuffer);

							bNid = pHdaCodec->TestNid;
							dHdaSendCodecCmd (bLinkId, bNid, 0x3a0, 0x03, &dData, &dData1);
							dHdaSendCodecCmd (bLinkId, bNid, 0xba0, 0x00, &dData, &dData1);
							sprintf (achBuffer, "Rsp = 0x%x, wait = %d\n", dData, dData1) ;
							vConsoleWrite (achBuffer);						
						}	
						else
						{
							sprintf (achBuffer, "Unexpected Codec ID = 0x%x on Link %d, wait = %d\n", dData, bLinkId, dData1) ;
							vConsoleWrite (achBuffer);
						}
					} 
					else 
					{
						// no codec found. Do we expect one ?
						
						if ((pHdaCodec != NULL) && (pHdaCodec->AlwaysFitted))
						{
							sprintf (achBuffer, "ERROR: CODEC not found detected on Link %d !!!\n", bLinkId) ;
							vConsoleWrite (achBuffer);
							dRt = AUDIO_ERROR + 2;
						}
						else
						{
							dRt = E__OK;
						}
					}
					bLinkId++;
				}
			}
			else
			{
				dRt = AUDIO_ERROR + 3;
				vConsoleWrite ("\nError allocating pointers\n");
			}
		}
		else
		{
			dRt = AUDIO_ERROR + 4;
			vConsoleWrite ("\nWrong Base Address\n");
		}
	}
	else
	{
		dRt = AUDIO_ERROR + 5;
		vConsoleWrite ("\nIntel HD Audio not found\n");
	}

	return (dRt);
}




/*****************************************************************************
 * AudioJackTest: Test the audio jack
 *
 * RETURNS: None
 */
TEST_INTERFACE (HdaAudioJackTest, "Audio Codec Jack Test")
{
	UINT32   dRt = E__OK, dData, dData1, dBaseAddr;
	PCI_PFA	 pfa;
	char	 achBuffer[128];
	UINT32	 dHandle;
	PTR48	 p1;
	UINT16	 wDeviceId;
	UINT8    bLinkId = 0;
	HDA_DEVICE *HdaList;
	HDA_CODEC  *HdaCodec;

	board_service (SERVICE__BRD_GET_HDA_LIST, NULL, (void *)&HdaList);
	pfa = HdaList->pfa;

	wDeviceId =  (UINT16) PCI_READ_WORD (pfa, 0x02);	
	sprintf ( achBuffer, "\nHDA PCI devid 0x%x\n", wDeviceId);	
	vConsoleWrite (achBuffer);	

	if (wDeviceId != 0xffff)
	{
		dBaseAddr = PCI_READ_DWORD (pfa, 0x10);	
		dBaseAddr &= 0xfffffff0;		
		sprintf ( achBuffer, "PCI BAR0 0x%x\n", dBaseAddr);	
		vConsoleWrite (achBuffer);	

		if(dBaseAddr != 0xfffffff0)
		{		
			board_service (SERVICE__BRD_ENABLE_HDA, NULL, &pfa);
			dHandle = dGetPhysPtr (dBaseAddr, 0x4000, &p1, &dBaseAddress);			

			if ((dHandle != E__FAIL) )
			{
				while ((bLinkId < HDA_MAX_CODECS) && (dRt == E__OK))
				{
					vGetCodec (HdaList, bLinkId, &HdaCodec);
					
					/* read the codec id reg */
					
					dRt = dHdaSendCodecCmd (bLinkId, 0, 0xf00, 0, &dData, &dData1);
					
					if (dRt == E__OK)
					{
						// codec found, do we know about it ?
						
						if (HdaCodec != NULL)
						{
							HDA_WIDGET *WidgetList;
							if (HdaCodec->CodecId == dData)
							{
								sprintf (achBuffer, "Codec ID = 0x%x on Link %d, wait = %d\n", dData, bLinkId, dData1) ;
							}
							else
							{
								sprintf (achBuffer, "ERROR: Expected Codec ID 0x%x found %x on Link %d\n", 
												HdaCodec->CodecId, dData, bLinkId) ;
								dRt = AUDIO_ERROR + 2;
							}
							vConsoleWrite (achBuffer);
							WidgetList = HdaCodec->WidgetList;
							vHdaEnableLines(bLinkId, WidgetList);
							vHdaStartTest(bLinkId, WidgetList);
						}
						else
						{
							sprintf (achBuffer, "Unexpected Codec ID = 0x%x on Link %d, wait = %d\n", dData, bLinkId, dData1) ;
							vConsoleWrite (achBuffer);
						}
					}
					else 
					{
						if ((HdaCodec != NULL) && (HdaCodec->AlwaysFitted))
						{
							sprintf (achBuffer, "ERROR: CODEC not found detected on Link %d !!!\n", bLinkId) ;
							vConsoleWrite (achBuffer);
							dRt = AUDIO_ERROR + 2;
						}
						else
						{
							dRt = E__OK;
						}
					}
					bLinkId++;
				}
			}
			else
			{
				dRt = AUDIO_ERROR + 3;
				vConsoleWrite ("\nError allocating pointers\n");
			}
		}
		else
		{
			dRt = AUDIO_ERROR + 4;
			vConsoleWrite ("\nWrong Base Address\n");
		}
	}
	else
	{
		dRt = AUDIO_ERROR + 5;
		vConsoleWrite ("\nIntel HD Audio not found\n");
	}

	return dRt;
}



/*****************************************************************************
 * AudioSoundTest: Test the audio sound generation
 *
 * RETURNS: None
 */
TEST_INTERFACE (HdaAudioSoundTest, "Audio Codec Tone Generation Test")
{
	UINT32   dRt = E__OK, dData, dData1, dBaseAddr, i;
	PCI_PFA	 pfa;
	char	 achBuffer[128];
	UINT32	 dHandle;
	PTR48	 p1;
	UINT8    testType;
	UINT16   wDeviceId;
	UINT8	 bNid;
	UINT8    bLinkId = 0;
	HDA_DEVICE *HdaList;
	HDA_CODEC  *HdaCodec;

	board_service (SERVICE__BRD_GET_HDA_LIST, NULL, (void *)&HdaList);
	pfa = HdaList->pfa;

	sprintf (achBuffer, "Number of parameters = %d\n", adTestParams[0]);
	vConsoleWrite (achBuffer);

	if(adTestParams[0] == 0)
		testType = 0;//default test
	else
	{
		if(adTestParams[1] == 1)
		{
			if(adTestParams[0] != 2)
				dRt = E__FAIL;
			else 
				testType = 1;
		}
		else if(adTestParams[1] == 2)
		{
			if(adTestParams[0] != 4)
				dRt = E__FAIL;
			else 
				testType = 2;
		}
		else
			dRt = E__FAIL;
	}

	for (i = 1; i <= adTestParams[0]; i++)
	{
		sprintf (achBuffer, "%2d: 0x%08X\n", i, adTestParams[i]);
		vConsoleWrite (achBuffer);
	}

	if(dRt == E__OK)
	{
		wDeviceId = (UINT16) PCI_READ_WORD (pfa, 0x02);	
		sprintf ( achBuffer, "HDA PCI devid 0x%x\n", wDeviceId);	
		vConsoleWrite (achBuffer);		

		if (wDeviceId != 0xffff)
		{
			dBaseAddr = PCI_READ_DWORD (pfa, 0x10);	
			dBaseAddr &= 0xfffffff0;		
			sprintf ( achBuffer, "PCI BAR0 0x%x\n", dBaseAddr);	
			vConsoleWrite (achBuffer);	

			if(dBaseAddr != 0xfffffff0)
			{			
				board_service (SERVICE__BRD_ENABLE_HDA, NULL, &pfa);
				
				dHandle = dGetPhysPtr (dBaseAddr, 0x4000, &p1, &dBaseAddress);			

				if ((dHandle != E__FAIL) )
				{
					while ((bLinkId < HDA_MAX_CODECS) && (dRt == E__OK))
					{
						vGetCodec (HdaList, bLinkId, &HdaCodec);
						/* read the codec id reg */
						dRt = dHdaSendCodecCmd (bLinkId, 0, 0xf00, 0, &dData, &dData1);
						
						if ( dRt == E__OK)
						{
							// codec found, do we know about it ?
							if (HdaCodec != NULL)
							{
								HDA_WIDGET *WidgetList;
								if (HdaCodec->CodecId == dData)
								{
									sprintf (achBuffer, "Codec ID = 0x%x on Link %d, wait = %d\n", dData, bLinkId, dData1) ;
									vConsoleWrite (achBuffer);
									
									vConsoleWrite ("  Press any key to stop the test \n") ;
									WidgetList = HdaCodec->WidgetList;
									vHdaEnableLines(bLinkId, WidgetList);					
					
									bNid = HdaCodec->TestNid;
									dHdaSendCodecCmd (bLinkId, bNid, 0x3a0, 0x03, &dData, &dData1);
									dHdaSendCodecCmd (bLinkId, bNid, 0xba0, 0x00, &dData, &dData1);
									sprintf (achBuffer, "Rsp = 0x%x, wait = %d\n", dData, dData1) ;
									vConsoleWrite (achBuffer);	

									dHdaSendCodecCmd (bLinkId, bNid, 0xf00, 0x09, &dData, &dData1);
									sprintf (achBuffer, "Rsp = 0x%x, wait = %d\n", dData, dData1) ;
									vConsoleWrite (achBuffer);	

									dHdaSendCodecCmd (bLinkId, bNid, 0xf00, 0x12, &dData, &dData1);
									sprintf (achBuffer, "Rsp = 0x%x, wait = %d\n", dData, dData1) ;
									vConsoleWrite (achBuffer);	

									if(testType == 0)
									{
										while (kbhit () != 1)
										for(i = 1; i<256; i++)
										{						
											dHdaSendCodecCmd (bLinkId, bNid, 0x70a, i, &dData, &dData1);
											vDelay(5);
										}						
									}
									else if(testType == 1)
									{
										vConsoleWrite ("\n1\n");
										dHdaSendCodecCmd (bLinkId, bNid, 0x70a, (UINT8)adTestParams[2], &dData, &dData1);	
										while (kbhit () != 1);												
									}
									else if(testType == 2)
									{
										while (kbhit () != 1)
										for(i = adTestParams[2]; i<adTestParams[3]; i++)
										{						
											dHdaSendCodecCmd (bLinkId, bNid, 0x70a, i, &dData, &dData1);
											vDelay(adTestParams[4]);
										}						
									}


									dHdaSendCodecCmd (bLinkId, bNid, 0x70a, 0, &dData, &dData1);
								}
								else
								{
									sprintf (achBuffer, "ERROR: Expected Codec ID 0x%x found %x on Link %d\n", 
													HdaCodec->CodecId, dData, bLinkId) ;
									dRt = AUDIO_ERROR + 2;
								}
							}
							else
							{
								sprintf (achBuffer, "Unexpected Codec ID = 0x%x on Link %d, wait = %d\n", dData, bLinkId, dData1) ;
								vConsoleWrite (achBuffer);
							}
						}
						else 
						{
							if ((HdaCodec != NULL) && (HdaCodec->AlwaysFitted))
							{
								sprintf (achBuffer, "ERROR: CODEC not found detected on Link %d !!!\n", bLinkId) ;
								vConsoleWrite (achBuffer);
								dRt = AUDIO_ERROR + 2;
							}
							else
							{
								dRt = E__OK;
							}
						}	
						bLinkId++;
					}
				}
				else
				{
					dRt = AUDIO_ERROR + 3;
					vConsoleWrite ("\nError allocating pointers\n");
				}
			}
			else
			{
				dRt = AUDIO_ERROR + 4;
				vConsoleWrite ("\nWrong Base Address\n");
			}
		}
		else
		{
			dRt = AUDIO_ERROR + 5;
			vConsoleWrite ("\nIntel HD Audio not found\n");
		}
	}

	return (dRt);
}
