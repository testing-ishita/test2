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

/* idtTempSensor.c - Checks the idt switch temperature sensor for correct threhold values 
 * of temperature with the current value of temperature.
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/board/tr803.39x/idtTempSensor.c,v 1.1 2015-01-29 11:07:37 mgostling Exp $
 *
 * $Log: idtTempSensor.c,v $
 * Revision 1.1  2015-01-29 11:07:37  mgostling
 * Import files into new source repository.
 *
 * Revision 1.1.1.1  2013-09-02 10:22:42  jthiru
 * Adding TRB12 CUTE
 *
 * Revision 1.1  2011/11/14 17:18:29  hchalla
 * Added new test for tr803 idt reading temperature test and other updates.
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
 
#include <error_msg.h>


typedef struct idtThreshold
{
	UINT8 BLTH:1;
	UINT8 ALTH:1;
	UINT8 BMTH:1;
	UINT8 AMTH:1;
	UINT8 BHTH:1;
	UINT8 AHTH:1;
}idtThreshold_t;

/* defines */
//#define DEBUG



/* externals */
//extern void vDelay(UINT32 dMsDelay);
//extern void vConsoleWrite(char*	achMessage);
extern void sysDebugWriteString(char*	buffer);

void vGetCurrentIdtTemp(UINT32 dTemp,UINT32 *pCurr,UINT32 *pLow, UINT32 *pHigh);
void vGetThresholdIdtTemp (UINT32 dTemp, idtThreshold_t  *pThreshold );
void vGetDTSTempSensor (UINT32 *pTemp);

/**********************************************************************************
 * IdtThermAccessTest: test function for accessing temperature sensor for idt switch
 *
 * RETURNS: Test status
 **********************************************************************************/

TEST_INTERFACE (idtTempSensorTest, "IDT Switch Temp Test")
{


	char  buffer[56];


	PTR48  tPtr;
	UINT32 mHandle;
	UINT32 pointer;
	UINT32 memRegAddr;
	PCI_PFA pfa;
	UINT8	bTemp=0;
	UINT32  DIDVID=0,dTemp=0;

	UINT32  dtest_status = E__OK;
	UINT32 dCurr=0,dLow=0,dHigh=0;
#if 0
	idtThreshold_t idtThreshold;
#endif

	/*PLX Switch  bus allocation */
	pfa = PCI_MAKE_PFA(0, 0x1, 0);
	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);
	pfa = PCI_MAKE_PFA(1, 0x0, 0);
	bTemp = PCI_READ_BYTE (pfa, 0x04);
	bTemp |= 0x07;
	PCI_WRITE_BYTE (pfa, 0x04, bTemp);

	DIDVID = (PCI_READ_DWORD (pfa, 0));


	sprintf(buffer, "PCI Read %x\n", DIDVID );
	sysDebugWriteString (buffer);


	if(DIDVID != 0xffffffff)
	{

		mHandle  = dGetPhysPtr(0xe0100000, 0x1000, &tPtr,(void*)(&pointer));


		sprintf(buffer, "Extended Read %x\n", *((UINT32*)pointer));
		sysDebugWriteString (buffer);

		*((UINT32*)(pointer + 0xff8)) = 0x3F1D4;	// addr
		memRegAddr = pointer + 0xFFC;
		*(UINT32*)memRegAddr = ((*(UINT32*)memRegAddr)  & ~(0x80000000) );
		dTemp = *(UINT32*)memRegAddr;

		sprintf(buffer, "Temperature Sensor Control %#x\n", dTemp);
		sysDebugWriteString (buffer);

		*((UINT32*)(pointer + 0xff8)) = 0x3F1D8;	// addr
		memRegAddr = pointer + 0xFFC;
		dTemp = *(UINT32*)memRegAddr;


		sprintf(buffer, "Temperature Sensor Status %#x\n", dTemp);
		sysDebugWriteString (buffer);

		vGetCurrentIdtTemp(dTemp,&dCurr,&dLow,&dHigh);
		sprintf(buffer, "IDT Switch Current Temperature %d ^C\n", dCurr);
		sysDebugWriteString (buffer);

		sprintf(buffer, "IDT Switch Low Temperature %d ^C\n", dLow);
		sysDebugWriteString (buffer);

		sprintf(buffer, "IDT Switch High Temperature %d ^C\n", dHigh);
		sysDebugWriteString (buffer);

#if 0
		*((UINT32*)(pointer + 0xff8)) = 0x3F1DC;	// addr
		memRegAddr = pointer + 0xFFC;
		dTemp = *(UINT32*)memRegAddr;


		sprintf(buffer, "Temperature Sensor Alarm %#x\n", dTemp);
		/*vConsoleWrite (buffer);*/


		vGetThresholdIdtTemp(dTemp,&idtThreshold);

		sprintf(buffer, "IDT Switch BLTH  %d\n", idtThreshold.BLTH);
		vConsoleWrite (buffer);

		sprintf(buffer, "IDT Switch ALTH  %d\n", idtThreshold.ALTH);
		vConsoleWrite (buffer);

		sprintf(buffer, "IDT Switch BMTH  %d\n", idtThreshold.BMTH);
		vConsoleWrite (buffer);

		sprintf(buffer, "IDT Switch AMTH  %d\n", idtThreshold.AMTH);
		vConsoleWrite (buffer);

		sprintf(buffer, "IDT Switch BHTH  %d\n", idtThreshold.BHTH);
		vConsoleWrite (buffer);

		sprintf(buffer, "IDT Switch AHTH  %d\n", idtThreshold.AHTH);
		vConsoleWrite (buffer);
#endif
		vFreePtr(mHandle);
	}


	dTemp = 0;
	vGetDTSTempSensor(&dTemp);
	sprintf(buffer, "\nCPU Temperature:%dC\n", dTemp);
	sysDebugWriteString(buffer);

	dtest_status = E__OK;
	return dtest_status;
}

void vGetCurrentIdtTemp(UINT32 dTemp,UINT32 *pCurr,UINT32 *pLow, UINT32 *pHigh)
{

	UINT32 dCurr=0;
	UINT32 dLow=0;
	UINT32 dHigh=0;

	dCurr = (dTemp) & (0x000000FF);
	dCurr = dCurr/2;
	*pCurr = dCurr;

	dLow = ((dTemp>>8) &(0x000000FF));
	dLow = dLow/2;
	*pLow = dLow;

	dHigh = ((dTemp>>16) & (0x000000FF));
	dHigh = dHigh/2;
	*pHigh = dHigh;
}

void vGetThresholdIdtTemp (UINT32 dTemp, idtThreshold_t  *pThreshold )
{

         if ((dTemp >> 24) & 0x1)
         {
             pThreshold->BLTH = 0x1;
         }
         else
         {
        	 pThreshold->BLTH = 0x0;
         }

         if ( (dTemp>>25) & 0x1)
         {
        	 pThreshold->ALTH = 0x1;
         }
         else
         {
        	 pThreshold->ALTH = 0x0;
         }

         if ( (dTemp>>26) & 0x1)
         {
        	 pThreshold->BMTH = 0x1;
         }
         else
         {
        	 pThreshold->BMTH = 0x0;
         }

         if ( (dTemp>>27) & 0x1)
         {
        	 pThreshold->AMTH = 0x1;
         }
         else
         {
        	 pThreshold->AMTH = 0x0;
         }

         if ( (dTemp>>28) & 0x1)
        {
        	 pThreshold->BHTH = 0x1;
        }
        else
        {
        	pThreshold->BHTH = 0x0;
        }

         if ( (dTemp>>29) & 0x1)
         {
        	 pThreshold->AHTH = 0x1;
         }
         else
         {
        	 pThreshold->AHTH = 0x0;
         }
}

void vGetDTSTempSensor (UINT32 *pTemp)
{
		UINT32 msw, lsw, prochot;
		int	readout;

		vReadMsr (0x19c, &msw, &lsw);
		readout = (lsw >> 16) & 0x000000ff;

		//read temperature Target
		vReadMsr (0x1a2, &msw, &lsw);
		prochot = (lsw >> 16) & 0x000000ff;

		readout = prochot-readout;

		*pTemp = readout;
}
