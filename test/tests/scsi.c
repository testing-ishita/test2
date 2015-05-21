
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
 * scsi.c
 *
 *  Created on: 7 Jun 2010
 *      Author: engineer
 */

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>
#include <bit/console.h>
#include <bit/bit.h>
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/hal.h>
 
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>
#include <devices/scsi.h>
#include <alloc.h>


//#define DEBUG
//#ifdef DEBUG
static char buffer[128];
//#endif

static UINT32 vInitScsiController  ( scsiCnt* scsiDev );
static UINT32 vDeInitScsiController( scsiCnt* scsiDev );
static void   vNcrSoftwareReset    ( scsiCnt* scsiDev );

static UINT32 wNcr53c895BistPci  ( scsiCnt* scsiDev );
static UINT32 wNcrTestByteAccess ( scsiCnt* scsiDev );
static UINT32 wNcrTestLongAccess ( scsiCnt* scsiDev );
static void   vNcrEmptyDmaFifo   ( scsiCnt* scsiDev );
static UINT32 wNcrTestFifoLanes  ( scsiCnt* scsiDev );
static UINT32 wNcrTestScriptDma  ( scsiCnt* scsiDev );

static UINT32 wControllerInterfaceTest ( scsiCnt* scsiDev1, scsiCnt* scsiDev2 );
static UINT32 wNcrTestInterfaceControl ( scsiCnt* scsiDevM, scsiCnt* scsiDevS );
static UINT32 wCheckResetLine          ( scsiCnt* scsiDevM, scsiCnt* scsiDevS );
static UINT32 wNcrTestInterfaceData    ( scsiCnt* scsiDevM, scsiCnt* scsiDevS, UINT8 n_bits );

static scsiCnt scsiFunc1;
static scsiCnt scsiFunc2;
static UINT32 scsiInt;
static int     vector0, vector1, vector2, vector3,vector4,vector5;

/******************************************************************************
* PMCSCSIGeneralTest
* Tests for PMC SCSI
* RETURNS: 0 on success else error code
******************************************************************************/
TEST_INTERFACE (PMCSCSIGeneralTest, "PMC SCSI General Tests")
{
	UINT32  rt = E__OK, done = 0;

	memset(&scsiFunc1, 0x00, sizeof(scsiCnt));

	scsiFunc1.Instance = 1;
	while( (done != 1) && (rt == E__OK) )
	{
		if(  iPciFindDeviceById( scsiFunc1.Instance, 0x1000, 0x0021, &scsiFunc1.pfa )
			 == E__OK)
		{

			sprintf(buffer ,"\nFound SCSI PMC160 at Bus:%d Dev:%d Func:%d\n",PCI_PFA_BUS(scsiFunc1.pfa),
					               PCI_PFA_DEV(scsiFunc1.pfa),PCI_PFA_FUNC(scsiFunc1.pfa));
			vConsoleWrite(buffer);

			#ifdef DEBUG
				sprintf(buffer ,"\nPMC160 func %d Found\n", scsiFunc1.Instance);
				vConsoleWrite(buffer);
			#endif

			rt = vInitScsiController(&scsiFunc1);
			if(rt == E__OK)
			{
				vNcrSoftwareReset (&scsiFunc1);
				rt = wNcr53c895BistPci (&scsiFunc1);
				vNcrSoftwareReset (&scsiFunc1);
				vDeInitScsiController(&scsiFunc1);
			}
			else
			{
				#ifdef DEBUG
					sprintf(buffer ,"\nPMC160 func %d Initialisation Failed\n",
							scsiFunc1.Instance);
					vConsoleWrite(buffer);
				#endif
				return E__SCSI_INIT;
			}
		}
		else
		{
			if(scsiFunc1.Instance == 1)
			{
				#ifdef DEBUG
					vConsoleWrite("PMC160 Not Found\n");
				#endif
				rt = E__SCSI_NO_DEVICE;
			}
			else
				done = 1;
		}
		scsiFunc1.Instance++;
	}

	return rt;
}



/******************************************************************************
* PMCSCSIIFTest
* Tests for PMC SCSI Interface
* RETURNS: 0 on success else error code
******************************************************************************/
TEST_INTERFACE (PMCSCSIIFTest, "PMC SCSI Interface Tests")
{
	UINT32  rt = E__OK;

	memset(&scsiFunc1, 0x00, sizeof(scsiCnt));
	memset(&scsiFunc2, 0x00, sizeof(scsiCnt));

	scsiFunc1.Instance = 1;
	if(  iPciFindDeviceById( scsiFunc1.Instance, 0x1000, 0x0021, &scsiFunc1.pfa )
		 == E__OK)
	{
		#ifdef DEBUG
			sprintf(buffer ,"\nPMC160 func %d Found\n", scsiFunc1.Instance);
			vConsoleWrite(buffer);
		#endif

		scsiFunc2.Instance = 2;
		if(  iPciFindDeviceById( scsiFunc2.Instance, 0x1000, 0x0021, &scsiFunc2.pfa )
			 == E__OK)
		{
			#ifdef DEBUG
				sprintf(buffer ,"\nPMC160 func %d Found\n", scsiFunc2.Instance);
				vConsoleWrite(buffer);
			#endif
		}
		else
		{
			#ifdef DEBUG
				vConsoleWrite("PMC160 Function 2 Not Found\n");
			#endif
			rt = E__SCSI_NO_2_DEVICE;
		}
		if(rt==E__OK)
		{
			rt = vInitScsiController(&scsiFunc1);
			if(rt == E__OK)
			{
				rt = vInitScsiController(&scsiFunc2);
				if(rt != E__OK)
				{
					#ifdef DEBUG
						sprintf(buffer ,"\nPMC160 func %d Initialisation Failed\n",
								scsiFunc1.Instance);
						vConsoleWrite(buffer);
					#endif
						vDeInitScsiController(&scsiFunc1);
						return E__SCSI_INIT;
				}

				vNcrSoftwareReset (&scsiFunc1);
				vNcrSoftwareReset (&scsiFunc2);

				rt = wControllerInterfaceTest( &scsiFunc1, &scsiFunc2 );

				vDeInitScsiController(&scsiFunc1);
				vDeInitScsiController(&scsiFunc2);
			}
			else
			{
				#ifdef DEBUG
					sprintf(buffer ,"\nPMC160 func %d Initialisation Failed\n",
							scsiFunc1.Instance);
					vConsoleWrite(buffer);
				#endif
				return E__SCSI_INIT;
			}
		}
	}
	else
	{
		#ifdef DEBUG
			vConsoleWrite("PMC160 Not Found\n");
		#endif
		rt = E__SCSI_NO_DEVICE;
	}

	return rt;
}




/******************************************************************************
 * vInitScsiController - initializes the SCSI PCI configuration registers
 * This function initialises the SCSI PCI.
 * RETURNS: N/A
 */
static UINT32 vInitScsiController(	scsiCnt* scsiDev )
{
	UINT32  mapRegSpace=0;
	UINT32	dDeviceAddr;
	UINT16	wTemp;

	/* disable memory and I/O accesses */
	wTemp  = PCI_READ_WORD (scsiDev->pfa, PCI_COMMAND);
	wTemp &= ~(PCI_MEMEN | PCI_IOEN);
	PCI_WRITE_WORD (scsiDev->pfa, PCI_COMMAND, wTemp);

	/* Determine the various PCI addresses and create a pointer to the SCSI
	 * controller. Its SCSI address is derived from the general purpose inputs
	 * via the GPREG register.*/
	dDeviceAddr  = PCI_READ_DWORD (scsiDev->pfa, BASE_ADDRESS_ONE);
	dDeviceAddr &= PCI_MEM_BASE_MASK;

	#ifdef DEBUG
		sprintf(buffer,"BASE_ADDRESS_ONE %x\n",dDeviceAddr);
		vConsoleWrite(buffer);
	#endif

	scsiDev->mHandle = dGetPhysPtr(dDeviceAddr,128,&scsiDev->tPtr1,(void*)&mapRegSpace);
	if(scsiDev->mHandle == E__FAIL)
	{
		#ifdef DEBUG
			sprintf(buffer, "Can not Allocate Mem Mapped Reg Space for scsi func %d\n",
					scsiDev->Instance );
			vConsoleWrite(buffer);
		#endif
		return E__SCSI_MEM_ALLOC;
	}
	scsiDev->pDevice = (NCR53C8X5 *)mapRegSpace;

	//Create Script Buffer in memory and copy buffer
	scsiDev->dScriptAddr = (UINT32*)0x00400000;
	memcpy (&scsiDev->dScriptAddr [SCRIPT_BIST_OFFSET], SCRIPT, sizeof (SCRIPT));


	// this should be the prefered way but the scripts are looking at fixed mem locations
	/*
	mHandle2 = dGetPhysPtr(0x00200000,4096,&tPtr2,(void*)&dScriptAddr);
	if(mHandle2 != NULL)
	{
		memcpy (&dScriptAddr [SCRIPT_BIST_OFFSET], SCRIPT, sizeof (SCRIPT));
	}
	else
	{
		vConsoleWrite("Can not Allocate Script Buffer\n");
		vFreePtr(mHandle);
		return E__FAIL;
	} */

	/* set the maximum latency */
	PCI_WRITE_BYTE (scsiDev->pfa, PCI_LATENCY, 0x10);

	/* enable I/O, MEM, Bus Mastering and SERR */
	wTemp  = PCI_READ_WORD (scsiDev->pfa, PCI_COMMAND);
	wTemp &= ~0x0147;
	wTemp |=  0x0107;
	PCI_WRITE_WORD (scsiDev->pfa, PCI_COMMAND, wTemp);

	/* now access renabled, write SCSI ID */
	scsiDev->pDevice->scid |= (6 + scsiDev->Instance);

	#ifdef DEBUG
		sprintf(buffer,"Scsi Func %d pDevice->scid After %x\n", scsiDev->Instance,
				scsiDev->pDevice->scid);
		vConsoleWrite(buffer);
	#endif

	return E__OK;
}



/******************************************************************************
 * vInitScsiController - initializes the SCSI PCI configuration registers
 * This function initialises the SCSI PCI.
 * RETURNS: N/A
 */
static UINT32 vDeInitScsiController( scsiCnt* scsiDev )
{
	UINT16	wTemp;

	/* disable memory and I/O accesses */
	wTemp  = PCI_READ_WORD (scsiDev->pfa, PCI_COMMAND);
	wTemp &= ~(PCI_MEMEN | PCI_IOEN);
	PCI_WRITE_WORD (scsiDev->pfa, PCI_COMMAND, wTemp);

	//free allocated memory
	vFreePtr(scsiDev->mHandle);
	//vFreePtr(mHandle2);

	return E__OK;
}



/******************************************************************************
 *
 * vNcrSoftwareReset - LSI SCSI controller software reset
 *
 * Asserts SRST for 5 millisecs to reset the controller. All operating registers
 * are cleared to their respective default values and all SCSI signals are
 * deasserted. The SCSI RST/ signal is NOT asserted, nor are the ID Mode bit or
 * any of the PCI configuration registers cleared. Following the reset, the
 * clock mode is setup.
 *
 * RETURNS: n/a
 */
static void vNcrSoftwareReset ( scsiCnt* scsiDev )
{
  /* software reset */
  scsiDev->pDevice->istat = istat_SRST;
  vDelay(6);
  scsiDev->pDevice->istat  = 0;

  /* setup clock */
  scsiDev->pDevice->scntl3 = scntl3_SCF_SCLK2 | scntl3_EWS;
}




/******************************************************************************
 *
 * wNcrTestByteAccess - Test byte access to the NCR53C8X5 SCSI controller
 *
 * This function tests whether the processor can access individual bytes in the
 * SCSI controller's SCRATCH-A register by writing a test pattern in bytes and
 * reading the register in a single 32-bit read. The written and read values
 * are simply compared to determine if byte access was successful.
 *
 * RETURNS: E_OK if byte access is OK otherwise E_TEST_FAIL.
 */
static UINT32 wNcrTestByteAccess ( scsiCnt* scsiDev )
{
  UINT8  *pbyte, i;
  UINT32  pattern;

  pattern = BYTE_TEST_PATTERN;

  /* write the test pattern as individual bytes to the "scratch-A" register */

  pbyte = (UINT8*) &scsiDev->pDevice->scratchA;
  for (i = 0; i < sizeof (pattern); i++)
	{
	  *(pbyte + i) = (UINT8)pattern;
	  pattern >>= 8;
	}

  /* read the "scratch-A" register and compare it with the test pattern */

  if ((pattern = scsiDev->pDevice->scratchA) != BYTE_TEST_PATTERN)
	{
		#ifdef DEBUG
		  vConsoleWrite("\nwNcrTestByteAccess : FAIL \n");
		  sprintf (buffer, "Byte access to \"scratch-A\" register failed; "
				  "expected: %08X, read: %08X", BYTE_TEST_PATTERN, pattern);
		  vConsoleWrite(buffer);
		#endif
	  return E__SCSI_BYTE_ACCESS;
	}

#ifdef DEBUG
  vConsoleWrite("\nwNcrTestByteAccess : PASS \n");
#endif
	return E__OK;
}



/******************************************************************************
 *
 * wNcrTestLongAccess - Test long word access to the NCR53C8X5 SCSI controller
 *
 * This function tests whether the processor can access long words in the
 * SCSI controller by writing 32-bit test patterns to the SCRATCH-B register
 * and subsequently comparing the SCRATCH-B register to determine if long word
 * accesses were successful. It is assumed that the SCRATCH-A register can be
 * read successfully.
 *
 * RETURNS: E_OK if long word access is OK otherwise E_TEST_FAIL.
 */
static UINT32 wNcrTestLongAccess ( scsiCnt* scsiDev )
{
  UINT32 pattern;
  UINT8  i;

  /*
   * Every test pattern is written to the SCRATCH-B register, a dummy read is
   * performed of the SCRATCH-A register and then the SCRATCH-B register is
   * read to determine if long word (i.e. 32 bit) accesses are successful.
   */

  for (i = 0; i < LENGTH (adLongPatterns); i++)
	{
	  scsiDev->pDevice->scratchB = adLongPatterns [i];
	  pattern = scsiDev->pDevice->scratchA;
	  pattern = scsiDev->pDevice->scratchB;

	  if (pattern != adLongPatterns [i])
		{
#ifdef DEBUG
		  vConsoleWrite("\nwNcrTestLongAccess : FAIL \n");
		  sprintf (buffer, "Long access to \"scratch-B\" register failed; "
				   "expected: %08X, read: %08X", adLongPatterns [i], pattern);
		  vConsoleWrite(buffer);
#endif
		  return E__SCSI_LONG_ACCESS;
		}
	}
#ifdef DEBUG
  vConsoleWrite("\nwNcrTestLongAccess : PASS \n");
#endif
  return E__OK;
}


/******************************************************************************
 *
 * vNcrEmptyDmaFifo - empties the DMA FIFO on LSI SCSI controller
 *
 * Asserts FLF and CLF for 1 millisec. FLF transfers any data residing in the
 * FIFO to memory. CLF clears all data pointers for the FIFO.
 *
 * RETURNS: n/a
 */
static void vNcrEmptyDmaFifo ( scsiCnt* scsiDev )
{
  /* flush and clear DMA FIFO */
	scsiDev->pDevice->ctest3 = ctest3_FLF | ctest3_CLF;
	vDelay(2);  /* this might need to be higher */
	scsiDev->pDevice->ctest3 = 0;
}


/******************************************************************************
 *
 * wNcrTestFifoLanes - Test byte lane access in the NCR53C8X5 SCSI controller
 *
 * This function tests the FIFO access for each byte lane in the SCSI controller
 * by simply writing a fixed sequence to the FIFO followed by reading that many
 * bytes from the FIFO.  The written and read values are simply compared to
 * determine if the FIFOs are functioning correctly.
 *
 * RETURNS: E_OK if FIFO lane accesses are OK otherwise E_TEST_FAIL.
 */
static UINT32 wNcrTestFifoLanes ( scsiCnt* scsiDev )
{
  UINT8  bFifo, i, numlanes;
  UINT16 j;

  /* read the device ID from PCI space */
  numlanes = 8;

  /* empty the DMA FIFO */
  vNcrEmptyDmaFifo (scsiDev);

  /* loop through byte lanes */

  for (i = 0; i < numlanes; i++)
  {
	  /* select the byte lane */
	  scsiDev->pDevice->ctest4 = ctest4_FBL3 | i;

	  /* fill the FIFO for this byte lane */
	  for (j = 0; j < LENGTH (bFifoLanesPattern); j++)
		  scsiDev->pDevice->ctest6 = bFifoLanesPattern [j];

	  /* read the FIFO and check that it matches the source data */
	  for (j = 0; j < LENGTH (bFifoLanesPattern); j++)
		{
		  if ((bFifo = scsiDev->pDevice->ctest6) != bFifoLanesPattern [j])
			{
#ifdef DEBUG
			  vConsoleWrite("\nwNcrTestFifoLanes : FAIL \n");
			  sprintf (buffer, "FIFO lane read/write failure at offset %u; "
					   "expected: %02X, read: %02X lane: %u", j,
					   bFifoLanesPattern [j], bFifo, i);
			  vConsoleWrite(buffer);
#endif
			  return E__SCSI_FIFO_LANES;
			}
		}
	}

  scsiDev->pDevice->ctest4 = 0;
#ifdef DEBUG
  vConsoleWrite("\nwNcrTestFifoLanes  : PASS \n");
#endif
  return E__OK;
}



/******************************************************************************
 *
 * scsiHandler - SCSI Interrupt Handler
 *
 * RETURNS: none
 */
static   void scsiHandler (void)
{
    scsiInt++;
}


/******************************************************************************
 *
 * InitInterrupts - initialise SCSI Interrupt
 *
 * RETURNS: none
 */
static void InitInterrupts()
{
	vector0 = sysPinToVector (16, SYS_IOAPIC0);
	sysInstallUserHandler (vector0, scsiHandler);
	sysUnmaskPin (16, SYS_IOAPIC0);

	vector1 = sysPinToVector (17, SYS_IOAPIC0);
	sysInstallUserHandler (vector1, scsiHandler);
	sysUnmaskPin (17, SYS_IOAPIC0);

	vector2 = sysPinToVector (18, SYS_IOAPIC0);
	sysInstallUserHandler (vector2, scsiHandler);
	sysUnmaskPin (18, SYS_IOAPIC0);

	vector3 = sysPinToVector (19, SYS_IOAPIC0);
	sysInstallUserHandler (vector3, scsiHandler);
	sysUnmaskPin (19, SYS_IOAPIC0);

	vector4 = sysPinToVector (20, SYS_IOAPIC0);
	sysInstallUserHandler (vector4, scsiHandler);
	sysUnmaskPin (20, SYS_IOAPIC0);

	vector5 = sysPinToVector (21, SYS_IOAPIC0);
	sysInstallUserHandler (vector5, scsiHandler);
	sysUnmaskPin (21, SYS_IOAPIC0);

}

/******************************************************************************
 *
 * DeInitInterrupts - de-initialise SCSI Interrupt
 *
 * RETURNS: none
 */
static void DeInitInterrupts()
{
	sysMaskPin (16, SYS_IOAPIC0) ;
	sysInstallUserHandler (vector0, 0) ;

	sysMaskPin (17, SYS_IOAPIC0) ;
	sysInstallUserHandler (vector1, 0) ;

	sysMaskPin (18, SYS_IOAPIC0) ;
	sysInstallUserHandler (vector2, 0) ;

	sysMaskPin (19, SYS_IOAPIC0) ;
	sysInstallUserHandler (vector3, 0) ;

	sysMaskPin (20, SYS_IOAPIC0) ;
	sysInstallUserHandler (vector4, 0) ;

	sysMaskPin (21, SYS_IOAPIC0) ;
	sysInstallUserHandler (vector5, 0) ;
}


/******************************************************************************
 *
 * scsiHandler - DMA by SCSI scripts
 *
 * This function performs a DMA transfer (memory to memory) with the SCSI
 * processor executing a SCRIPT program.
 *
 * RETURNS: E__OK if script DMA is OK otherwise E__FAIL
 */
static UINT32 wNcrTestScriptDma ( scsiCnt* scsiDev )
{

	UINT32 	stat, i;
	UINT16 	count;
	UINT8  	*smem, *tmem, pend, vect;

	/* set SCSI controller at known state */
	vNcrSoftwareReset (scsiDev);
	vNcrEmptyDmaFifo  (scsiDev);

	/* (after software reset, all operating registers are at default values) */
	/* setup registers for script processing */

	scsiDev->pDevice->scntl0 = scntl0_ARB_FULL;                    /* full arbitration mode */
	scsiDev->pDevice->dien   = dien_BF | dien_SIR | dien_IID;       /* allow some DMA ints */
	scsiDev->pDevice->scid   = 07; /* set our SCSI ID */

	scsiDev->pDevice->ctest5 = 0;    /* (contains part of dmode value) */
	scsiDev->pDevice->dmode  = 0xC9; /* burst length 16, enable read multiple, manual start */

	scsiDev->pDevice->dbc [0] = 0; /* clear DMA byte counter */
	scsiDev->pDevice->dbc [1] = 0;
	scsiDev->pDevice->dbc [2] = 0;

	smem = (UINT8*) 0x00460000;
	tmem = (UINT8*) 0x00470000;

  	for (i = 0; i < TEST_LENGTH; i++)
	{
  		smem[i] = i;
  		tmem[i] = 0xEE;
	}

	scsiInt = 0;

	InitInterrupts();

  	/* Start the Script Processor */
  	scsiDev->pDevice->dsp    =(UINT32)( scsiDev->dScriptAddr + SCRIPT_BIST_OFFSET);
  	scsiDev->pDevice->dcntl |= dcntl_STD;

  	/* wait for interrupt or timeout */
  	for (count = 1000; count > 0; count--)
	{
  		pend = scsiDev->pDevice->istat & (istat_SIP | istat_DIP);
  		if (pend != 0)
  			break;
  		vDelay(1);
	}

  /* The normal/correct situation at this point is a "SCRIPT Complete Int"
   * from the Script Processor, Check this condition first and act
   * accordingly. Any other possibility is an error condition. */

  /* error if no script response */
  if (!(pend & istat_DIP) || (count == 0))
	{
#ifdef DEBUG
	  if (count == 0)
		  vConsoleWrite ("\nTIMER HAS EXPIRED\n");

	  vConsoleWrite ("NO SCRIPT RESPONSE\n");
#endif
	  return E__SCSI_DMA_FAIL;
	}

  DeInitInterrupts();

  if(scsiInt > 0)
  {
	#ifdef DEBUG
	  sprintf (buffer, "SCSI INT on SYS_IOAPIC0, count %d\n", scsiInt) ;
	  vConsoleWrite (buffer);
	#endif
  }
  else
  {
#ifdef DEBUG
  	 sprintf (buffer, "NO SCSI INT on SYS_IOAPIC0 PIN \n") ;
  	 vConsoleWrite (buffer);
#endif
  	 return E__SCSI_NO_INT;
  }


  /* check interrupt received ok */
  stat = *((UINT32 *) &scsiDev->pDevice->dstat);// page 151
  vect = scsiDev->pDevice->dsps & 0x000000ff;   // page 172
  if (((stat & dstat_SIR) != dstat_SIR) || (vect != 0x0a))
	{
	  /* incorrect script end */
#ifdef DEBUG
	  sprintf (buffer, "WRONG SCRIPT RESPONSE, Term code %X. dstat %X\n",
			   vect, stat);
	  vConsoleWrite (buffer);
#endif
	  return E__SCSI_DMA_FAIL;
	}


  /* compare target now matches source memory area */

  for (i = 0; i < TEST_LENGTH; i++)
	{
	  if (smem[i] != tmem[i])
		{
#ifdef DEBUG
		  sprintf (buffer, "MEMORY COPY ERROR, OFFSET = %04XH, "
				   "EXP = %02XH, REC = %02XH\n", i, smem[i], tmem[i]);
		  vConsoleWrite (buffer);
#endif
		  return E__SCSI_DMA_FAIL;
		}
	}

#ifdef DEBUG
  vConsoleWrite("\nwNcrTestScriptDma  : PASS \n");
#endif
  return E__OK;
}



/******************************************************************************
 *
 * wNcr53c895BistPci - performs the NCR53C8X5 self-test.
 *
 * This function self-tests the Symbios Logic's NCR53C8X5 SCSI controller.
 *
 * The test checks that scratch registers are read/writable, SCSI and DMA FIFOs
 * are working correctly, and the script processor functions OK.
 *
 * RETURNS: E_OK if controller functioning correctly, otherwise error
 */

static const TEST tests [] = {
							  wNcrTestByteAccess,
							  wNcrTestLongAccess,
							  wNcrTestFifoLanes,
							  wNcrTestScriptDma,
                              NULL
							 };

static UINT32 wNcr53c895BistPci ( scsiCnt* scsiDev  )
{
  UINT32  wTestStatus;
  UINT16  i;

  i = 0;
  wTestStatus = E__OK;
  while(tests[i] != NULL && (wTestStatus == E__OK))
  {
	  wTestStatus = tests[i](scsiDev);

	  i++;
  }

  return wTestStatus;
}





/******************************************************************************
 *
 * wControllerInterfaceTest - performs an interface test
 *
 * This function performs a SCSI interface test between the PMC SCSI
 * controller and the host's SCSI 53C810.
 * This function controls the whole test, both master and slave parts of
 * the test. The test does the interface test twice, once with the HOST
 * SCSI 53C810 as master, and if this passes the test is rerun with the
 * SCSI 53C825 on the PMC as the master.
 *
 * RETURNS: wTestStatus (from the standard set of error codes)
 */
static UINT32 wControllerInterfaceTest( scsiCnt* scsiDev1, scsiCnt* scsiDev2 )
{
	UINT32 wTestStatus = E__OK;

	/* do control bus walk test with both host and PMC as masters */

#ifdef DEBUG
	vConsoleWrite ("PCI SCSI - control interface test (host as master)\n");
#endif
	if ((wTestStatus = wNcrTestInterfaceControl( scsiDev1, scsiDev2 )) != E__OK)
		return (E__SCSI_IF_CONTROL);

#ifdef DEBUG
	vConsoleWrite ("PCI SCSI - control interface test (PMC as master)\n");
#endif
	if ((wTestStatus = wNcrTestInterfaceControl( scsiDev2, scsiDev1 )) != E__OK)
		return (E__SCSI_IF_CONTROL);


	// do data bus walk test with both host and PMC as masters
#ifdef DEBUG
	vConsoleWrite ("PCI SCSI - data interface test (host as master)\n");
#endif
	if ((wTestStatus = wNcrTestInterfaceData (scsiDev1, scsiDev2, 8)) != E__OK)
	  return (E__SCSI_IF_DATA);

#ifdef DEBUG
	vConsoleWrite ("PCI SCSI - data interface test (PMC as master)\n");
#endif
	if ((wTestStatus =	wNcrTestInterfaceData (scsiDev2, scsiDev1, 8)) != E__OK)
	  return (E__SCSI_IF_DATA);

	return wTestStatus;
}




/******************************************************************************
 *
 * wCheckResetLine - checks the reset line between master and slave devices
 *
 * This function checks the connection between master and slave by asserting
 * and then deasserting SCSI RESET in the master and checking for matching
 * signals in the slave.
 *
 * RETURNS: wTestStatus (from the standard set of error codes)
 */
static UINT32 wCheckResetLine( scsiCnt* scsiDevM, scsiCnt* scsiDevS )
{
  UINT32 tst = E__OK;
  UINT32 tmo;

  /* assert reset line in master and check in slave */
  scsiDevM->pDevice->scntl1 = scntl1_RST;
  for (tmo = TIME_LIMIT; tmo > 0; tmo--)
	{
	  vDelay (2);
	  if ((scsiDevS->pDevice->sstat0 & sstat0_RST) == sstat0_RST)
		break;
	}

  if (tmo == 0)
	{
#ifdef DEBUG
	  vConsoleWrite ("TIMED OUT on reset (asserted) check.\n");
#endif
	  tst = E__FAIL;
	}

  vDelay (5);

  /* now deassert reset line in master and check in slave */

  scsiDevM->pDevice->scntl1 = 0;
  for (tmo = TIME_LIMIT; tmo > 0; tmo--)
	{
	  vDelay (2);
	  if ((scsiDevS->pDevice->sstat0 & sstat0_RST) != sstat0_RST)
		break;
	}

  if (tmo == 0)
  {
#ifdef DEBUG
	  vConsoleWrite ("TIMED OUT on reset (deasserted) check.\n");
#endif
	  tst = E__FAIL;
  }

	return tst;
}


/******************************************************************************
 *
 * wNcrTestInterfaceControl - some sort of interface test
 *
 * This function performs a test on the master SCSI control bus with the
 * the slave reading and checking the bits.
 *
 * RETURNS: wTestStatus (from the standard set of error codes)
 */

static UINT32 wNcrTestInterfaceControl( scsiCnt* scsiDevM, scsiCnt* scsiDevS )
{
  volatile UINT8      bSbcl [3];

  UINT32 tmo;
  UINT32 tst;
  UINT8  loop, pattern;

  /* check that neither device is asserting SCSI RESET */
  if ((scsiDevM->pDevice->sstat0 & sstat0_RST) == sstat0_RST)
	{
#ifdef DEBUG
	  vConsoleWrite ("Error - MASTER SCSI reset asserted\n");
#endif
	  return E__FAIL;
	}

  if ((scsiDevM->pDevice->sstat0 & sstat0_RST) == sstat0_RST)
	{
#ifdef DEBUG
	  vConsoleWrite ("Error - SLAVE SCSI reset asserted\n");
#endif
	  return E__FAIL;
	}


  /* check the reset line to test comms between master and slave */
  if ((tst = wCheckResetLine (scsiDevM, scsiDevS)) != E__OK)
  {
#ifdef DEBUG
	  vConsoleWrite ("Reset Line Connectivity Error\n");
#endif
	  return E__FAIL;
  }

  /* allow master to control all SCSI lines */
  scsiDevM->pDevice->stest2 = stest2_SCE;


  /* loop through all control lines */
  for (loop = 0; loop < 8; loop++)
	{
	  vDelay(10);

	  /* assert SCSI signal */
	  pattern = 1 << loop;
	  scsiDevM->pDevice->socl = pattern;

	  /* now read control lines and check they match signal */
	  for (tmo = TIME_LIMIT; tmo > 0; tmo--)
		{
		  vDelay (1);

		  do  /* debounce */
			{
			  bSbcl [0] = scsiDevS->pDevice->sbcl;
			  bSbcl [1] = scsiDevS->pDevice->sbcl;
			  bSbcl [2] = scsiDevS->pDevice->sbcl;
			} while ((bSbcl [0] != bSbcl [1]) || (bSbcl [1] != bSbcl [2]));

		  /* ignore bit 1 since 895 and 1000 don't let it toggle */
		  if ((bSbcl[0] & 0xFD) == (pattern & 0xFD))
			break;
		}

	  /* check for timeout error */
	  if (tmo == 0)
		{
#ifdef DEBUG
		  sprintf (buffer, "Error - Timeout on SLAVE waiting for CTRL "
				   "bit %d [received %02Xh]\n", loop, bSbcl [0]);
		  vConsoleWrite (buffer);
#endif
		  tst = E__FAIL;
		  break;
		}

	  /* test the reset line */
	  if ((tst = wCheckResetLine (scsiDevM, scsiDevS)) != E__OK)
		break;

	}

  return tst;
}


/******************************************************************************
 *
 * wNcrTestInterfaceData - master SCSI data bus test
 *
 * This function performs a test on the master SCSI data bus with the slave
 * reading and checking the bits. Assumes both controllers are NCR53C8x5.
 *
 * RETURNS: test status (from the standard set of error codes)
 */

static UINT32 wNcrTestInterfaceData( scsiCnt* scsiDevM, scsiCnt* scsiDevS, UINT8 n_bits )
{
  volatile UINT8      bSbdl [3];
  UINT32 tmo, tst;
  UINT16 pattern, loop;


  /* check the reset line to test comms between master and slave */
   if ((tst = wCheckResetLine (scsiDevM, scsiDevS)) != E__OK)
   {
#ifdef DEBUG
 	  vConsoleWrite ("Reset Line Connectivity Error\n");
#endif
 	  return E__FAIL;
   }

  /* 16- or 8-bit data transfers */
  if (n_bits != 8)
	{
	  /* enable wide SCSI */
	  scsiDevM->pDevice->scntl3 |= scntl3_EWS;
	  scsiDevS->pDevice->scntl3 |= scntl3_EWS;
	}
  else
	{
	  /* disable wide SCSI */
	  scsiDevM->pDevice->scntl3 &= ~scntl3_EWS;
	  scsiDevS->pDevice->scntl3 &= ~scntl3_EWS;
	}

  /* drive source's data onto SCSI bus */

  scsiDevM->pDevice->scntl1 |= scntl1_ADB;
  scsiDevM->pDevice->socl    = 0;
  scsiDevM->pDevice->stest2  = stest2_SCE;

  /* loop through data lines (8 or 16) */
  for (loop = 0; loop < n_bits; loop++)
	{
	  vDelay(10);

	  /* assert a data line */
	  pattern = (UINT16) 1 << loop;
	  scsiDevM->pDevice->sodl = pattern;

	  /* now read back data and check it matches */
	  for (tmo = TIME_LIMIT; tmo > 0; tmo--)
		{
		  vDelay (1);
		  do  /* debounce */
			{
			  bSbdl [0] = scsiDevS->pDevice->sbdl;
			  bSbdl [1] = scsiDevS->pDevice->sbdl;
			  bSbdl [2] = scsiDevS->pDevice->sbdl;
			} while ((bSbdl [0] != bSbdl [1]) || (bSbdl [1] != bSbdl [2]));

		  if (bSbdl[0] == pattern)
			break;
		}

	  /* check for timeout error */
	  if (tmo == 0)
		{
#ifdef DEBUG
		  sprintf (buffer, "Time out on SLAVE waiting for DATA "
				   "bit %d [received %02Xh]\n", loop, bSbdl [0]);
		  vConsoleWrite (buffer);
#endif
		  tst = E__FAIL;
		  break;
		}

	  if ((tst = wCheckResetLine (scsiDevM, scsiDevS)) != E__OK)
	    {
#ifdef DEBUG
	   	  vConsoleWrite ("Reset Line Connectivity Error\n");
#endif
	   	  tst = E__FAIL;
	   	  break;
	    }
	}

  vNcrSoftwareReset (scsiDevM);
  vNcrSoftwareReset (scsiDevS);

  return tst;
}

