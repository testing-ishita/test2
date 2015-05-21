
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


/* includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errors.h>

#include <private/port_io.h>
#include <private/mem_io.h>
#include <private/cpu.h>
#include <bit/conslib.h>
 
#include <bit/pci.h>
#include <bit/mem.h>
#include <bit/board_service.h>


/* defines */

#define TYPE_MEM			0		/* PCI resource type */
#define TYPE_IO				1

#define PCI_SCANNING		0		/* PCI scan flags */
#define DEVICE_NOT_FOUND	1

static const char	achCpuidFeatures[][8] =
{
	"FPU ",		"VME ",		"DE ",	 	"PSE ",		/* 0-3	 */
	"TSC ",		"MSR ",		"PAE ",		"MCE ",		/* 4-8	 */
	"CXS ",		"APIC ",	"- ",		"SEP ",		/* 9-11	 */
	"MTRR ",	"PGE ",		"MCA ",		"CMOV ",	/* 12-15 */
	"PAT ",		"PSE-36 ",	"PSN ",		"CLFSH ",	/* 16-19 */
	"- ",		"DS ",		"ACPI ",	"MMX ",		/* 20-23 */
	"FXSR ",	"SSE ",		"SSE2 ",	"SS ",		/* 24-27 */
	"HTT ",		"TM ",		"- ",		"PBE ",		/* 28-31 */
};


/****************************************************************************
 * vPciBridgeScan: display bridge-type devices found by a PCI bus scan
 *
 * RETURNS: None
 */

static const char achBridgeType[][20] =
{
	"Host bridge",			/* 0 */
	"ISA bridge",			/* 1 */
	"EISA bridge",			/* 2 */
	"MCA bridge",			/* 3 */
	"PCI-to-PCI bridge",	/* 4 */
	"PCMCIA bridge",		/* 5 */
	"NuBus bridge",			/* 6 */
	"CardBus bridge",		/* 7 */
	"Other"
};



static void		vPciHeaderType0 (ARGS* psArgs);		/* PCI device */
static void		vPciHeaderType1 (ARGS* psArgs);		/* PCI-PCI bridge */
static void		vPciHeaderType2 (ARGS* sArgs);		/* CardBus bridge */



/****************************************************************************
 * vDisplayMemory: display of memory contents, formatted as 8, 16 or 32-bits
 *
 * RETURNS: none
 */

void vDisplayMemory
(
	int   iCmdType,
	int   iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	UINT32	dIndex;
	UINT32	dLoop;
	UINT32	dData;
	int		iStatus;
	char	achLine[80];
	char	achItem[10];

	volatile UINT32	 mapMemSpace;
	PTR48 	 tPtr1;
	UINT32 	 mHandle;
	char     buffer[64];

	iStatus = iExtractArgs (&sArgs,
							ARG__ADDR,
							ARG__LENGTH | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	if (iGetRepeatIndex () > 0)
		sArgs.dAddr += sArgs.dLength * iGetRepeatIndex ();

	/*	Allocate memory
	 * ----------------
	 * - Memory is now always allocated before access
	 * */
	mHandle = dGetPhysPtr(sArgs.dAddr,sArgs.dLength, &tPtr1,(void*)&mapMemSpace);
	if(mHandle ==  NULL)
	{
		sprintf (buffer, "Memory allocation failed for address %x\n", sArgs.dAddr );
		puts (buffer);
		return;
	}

	/* Perform the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
				sprintf (achLine, "(R) Mem. %08Xh:", sArgs.dAddr + dIndex);

			switch (iCmdType)
			{
				case TYPE__DISPLAY_BYTE:

					dData = sysReadMem8 (mapMemSpace + dIndex);
					sprintf (achItem, " %02X", (UINT8)dData);
					dIndex += 1;
					break;

				case TYPE__DISPLAY_WORD:

					dData = sysReadMem16 (mapMemSpace + dIndex);
					sprintf (achItem, " %04X", (UINT16)dData);
					dIndex += 2;
					break;

				case TYPE__DISPLAY_DWORD:

					dData = sysReadMem32 (mapMemSpace + dIndex);
					sprintf (achItem, " %08X", dData);
					dIndex += 4;
					break;
			};

			strcat (achLine, achItem);

			if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
				 (dIndex >= sArgs.dLength) )
			{
				puts (achLine);
			}
		}

		iStatus = iCheckForEsc ();
	}

	/* Deallocate memory*/
	if(mHandle != NULL)
	{
		vFreePtr(mHandle);
	}

} /* vDisplayMemory () */




/****************************************************************************
 * vDisplayAscii: display memory contents as 8-bit and ASCII values
 *
 * RETURNS: none
 */

void vDisplayAscii
(
	int   iCmdType,
	int   iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	UINT32	dIndex;
	UINT32	dLoop;
	UINT8	bData;
	int		iStatus;
	char	achLine1[81];
	char	achLine2[81];
	char	achItem1[10];
	char	achItem2[10];

	volatile UINT32	 mapMemSpace;
	PTR48 	 tPtr1;
	UINT32 	 mHandle;
	char     buffer[64];

	(void)iCmdType;   	/* reference params to prevent compiler errors */


	iStatus = iExtractArgs (&sArgs,
							ARG__ADDR,
							ARG__LENGTH | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	if (iGetRepeatIndex () > 0)
		sArgs.dAddr += sArgs.dLength * iGetRepeatIndex ();

	/*	Allocate memory
	 * ----------------
	 * - Memory is now always allocated before access
	 * */
	mHandle = dGetPhysPtr(sArgs.dAddr,sArgs.dLength, &tPtr1,(void*)&mapMemSpace);
	if(mHandle ==  NULL)
	{
		sprintf (buffer, "Memory allocation failed for address %x\n", sArgs.dAddr );
		puts (buffer);
		return;
	}

	/* Perform the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
			{
				sprintf (achLine1, "Asc %08Xh:", sArgs.dAddr + dIndex);
				sprintf (achLine2, " ");
			}

			bData = sysReadMem8 (mapMemSpace + dIndex);
			sprintf (achItem1, " %02X", bData);

			if ((bData >= 0x20) && (bData <= 0x7F))
				sprintf (achItem2, "%c", bData);
			else
				sprintf (achItem2, " ");

			dIndex += 1;

			strcat (achLine1, achItem1);
			strcat (achLine2, achItem2);

			if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
				 (dIndex >= sArgs.dLength) )
			{
				strcat (achLine1, achLine2);
				puts (achLine1);
			}
		}

		iStatus = iCheckForEsc ();
	}

	/* Deallocate memory*/
	if(mHandle != NULL)
	{
		vFreePtr(mHandle);
	}

} /* vDisplayAscii () */


/****************************************************************************
 * vSetMemory: write memory contents with 8, 16 or 32 bit value
 *
 * RETURNS: none
 */

void vSetMemory
(
	int   iCmdType,
	int   iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	UINT32	dIndex;
	UINT32	dLoop;
	UINT32	dData;
	int		iStatus;
	char	achLine[80];
	char	achItem[10];

	volatile UINT32	 mapMemSpace;
	PTR48 	 tPtr1;
	UINT32 	 mHandle;
	char     buffer[64];

	iStatus = iExtractArgs (&sArgs,
							ARG__ADDR | ARG__DATA,
							ARG__LENGTH | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	/*	Allocate memory
	 * ----------------
	 * - Memory is now always allocated before access
	 * */
	mHandle = dGetPhysPtr(sArgs.dAddr,sArgs.dLength, &tPtr1,(void*)&mapMemSpace);
	if(mHandle ==  NULL)
	{
		sprintf (buffer, "Memory allocation failed for address %x\n", sArgs.dAddr );
		puts (buffer);
		return;
	}

	/* Perform the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
				sprintf (achLine, "(W) Mem. %08Xh:", sArgs.dAddr + dIndex);

			switch (iCmdType)
			{
				case TYPE__SET_BYTE:

					sysWriteMem8 (mapMemSpace + dIndex, (UINT8)sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = sysReadMem8 (mapMemSpace + dIndex);
						sprintf (achItem, " %02X", (UINT8)dData);
					}

					dIndex += 1;
					break;

				case TYPE__SET_WORD:

					sysWriteMem16 (mapMemSpace + dIndex, (UINT16)sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = sysReadMem16 (mapMemSpace + dIndex);
						sprintf (achItem, " %04X", (UINT16)dData);
					}

					dIndex += 2;
					break;

				case TYPE__SET_DWORD:

					sysWriteMem32 (mapMemSpace + dIndex, sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = sysReadMem32 (mapMemSpace + dIndex);
						sprintf (achItem, " %08X", dData);
					}

					dIndex += 4;
					break;
			};

			if (iGetConfirmFlag () != 0)
			{
				strcat (achLine, achItem);

				if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
					 (dIndex >= sArgs.dLength) )
				{
					puts (achLine);
				}
			}
		}

		iStatus = iCheckForEsc ();
	}

	/* Deallocate memory*/
	if(mHandle != NULL)
	{
		vFreePtr(mHandle);
	}

} /* vSetMemory () */


/****************************************************************************
 * vPortInput: display I/O port contents, formatted as 8, 16 or 32 bit
 *
 * RETURNS: none
 */

void vPortInput
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	UINT32	dIndex;
	UINT32	dLoop;
	UINT32	dData;
	int		iStatus;
	char	achLine[80];
	char	achItem[10];


	iStatus = iExtractArgs (&sArgs,
							ARG__ADDR,
							ARG__LENGTH | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	if (iGetRepeatIndex () > 0)
		sArgs.dAddr += sArgs.dLength * iGetRepeatIndex ();

	/* Perform the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
			{
				sprintf (achLine, "(R) I/O %04Xh:",
							(UINT16)(sArgs.dAddr + dIndex));
			}

			switch (iCmdType)
			{
				case TYPE__INPUT_BYTE:

					dData = sysInPort8 ((UINT16)(sArgs.dAddr + dIndex));
					sprintf (achItem, " %02X", (UINT8)dData);
					dIndex += 1;
					break;

				case TYPE__INPUT_WORD:

					dData = sysInPort16 ((UINT16)(sArgs.dAddr + dIndex));
					sprintf (achItem, " %04X", (UINT16)dData);
					dIndex += 2;
					break;

				case TYPE__INPUT_DWORD:

					dData = sysInPort32 ((UINT16)(sArgs.dAddr + dIndex));
					sprintf (achItem, " %08X", dData);
					dIndex += 4;
					break;
			};

			strcat (achLine, achItem);

			if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
				 (dIndex >= sArgs.dLength) )
			{
				puts (achLine);
			}
		}

		iStatus = iCheckForEsc ();
	}

} /* vPortInput () */


/****************************************************************************
 * vPortOutput: write I/O port with 8, 16 or 32 bit value
 *
 * RETURNS: none
 */

void vPortOutput
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	UINT32	dIndex;
	UINT32	dLoop;
	UINT32	dData;
	int		iStatus;
	char	achLine[80];
	char	achItem[10];


	iStatus = iExtractArgs (&sArgs,
							ARG__ADDR | ARG__DATA,
							ARG__LENGTH | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	/* Perform the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
			{
				sprintf (achLine, "(W) I/O %04Xh:",
							(UINT16)(sArgs.dAddr + dIndex));
			}

			switch (iCmdType)
			{
				case TYPE__OUTPUT_BYTE:

					sysOutPort8 ((UINT16)(sArgs.dAddr + dIndex),
								(UINT8)sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = sysInPort8 ((UINT16)(sArgs.dAddr + dIndex));
						sprintf (achItem, " %02X", (UINT8)dData);
					}

					dIndex += 1;
					break;

				case TYPE__OUTPUT_WORD:

					sysOutPort16 ((UINT16)(sArgs.dAddr + dIndex),
								(UINT16)sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = sysInPort16 ((UINT16)(sArgs.dAddr + dIndex));
						sprintf (achItem, " %04X", (UINT16)dData);
					}

					dIndex += 2;
					break;

				case TYPE__OUTPUT_DWORD:

					sysOutPort32 ((UINT16)(sArgs.dAddr + dIndex), sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = sysInPort32 ((UINT16)(sArgs.dAddr + dIndex));
						sprintf (achItem, " %08X", dData);
					}

					dIndex += 4;
					break;
			};

			if (iGetConfirmFlag () != 0)
			{
				strcat (achLine, achItem);

				if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
					 (dIndex >= sArgs.dLength) )
				{
					puts (achLine);
				}
			}
		}

		iStatus = iCheckForEsc ();
	}

} /* vPortOutput () */



/****************************************************************************
 * vPciScan: display results of a PCI bus scan
 *
 * RETURNS: none
 */

void vPciScan
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	PCI_PFA	pfa;
	UINT16	wThisVid;		/* vendor ID at this address */
	UINT16	wThisDid;		/* device ID at this address */
	UINT16	wStatus;		/* status of the device scan */
	UINT8	bMultiFunc;		/* non-zero indicates multi-function device */
	UINT8	bHiBus;			/* records the current highest bus to scan */
	UINT8	bThisIrq;		/* IRQ used by device */
	UINT8	bTemp;
	UINT8	bBus;
	UINT8	bDev;
	UINT8	bFunc;
	char	achLine[80];
	char	achIrq[10];
	int		iHost   = -1;	/* start at -1 so first host is bus-0 */
	int		iPciPci = 0;  	/* count PCI-PCI bridges */
	int		iLineCount;


	(void)iCmdType;   	/* reference params to prevent compiler errors */
	(void)iCmdCount;
	(void)achUserArgs;


	bFunc = 0;		/* start at PCI address zero */
	bDev  = 0;
	bBus  = 0;

	bMultiFunc	= 0;
	bHiBus		= 0;

	wStatus = PCI_SCANNING;		/* indicate bus scan in progress */

	/* Perform the operation */

	iLineCount = 0;

	while (wStatus == PCI_SCANNING)
	{
		pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);

		wThisVid = (UINT16) dPciReadReg (pfa, PCI_VENDOR_ID, REG_16);
		wThisDid = (UINT16) dPciReadReg (pfa, PCI_DEVICE_ID, REG_16);

		bThisIrq = (UINT8) dPciReadReg (pfa, PCI_INT_LINE, REG_16);
		if ((bThisIrq > 0) && (bThisIrq <= 15))
			sprintf (achIrq, "IRQ = %02Xh", bThisIrq);
		else
			strcpy (achIrq, "");

		if (wThisVid != 0xFFFF)
		{
			iLineCount++;

			if (iLineCount > 17)
			{
				puts ("** Press any key to continue **");

				while (iGetExtdKeyPress () == 0);
				iLineCount = 0;
			}

			sprintf (achLine, "Bus: %d Dev: %2d Func: %d = Vendor: %04Xh Device: %04Xh %s",
						bBus, bDev, bFunc, wThisVid, wThisDid, achIrq);
			puts (achLine);


			if (bFunc == 0)		/* see if multi-func */
			{
				bTemp = (UINT8) dPciReadReg (pfa, PCI_HEADER_TYPE, REG_8);
				bMultiFunc = (bTemp & 0x80) || ((bBus == 0) && (bDev == 0));
			}

			/* Check for bridge - class code = 06 */

			bTemp = (UINT8) dPciReadReg (pfa, PCI_BASE_CLASS, REG_8);
			if (bTemp == 0x06)
			{
				bTemp = (UINT8) dPciReadReg (pfa, PCI_SUB_CLASS, REG_8);
				if ((bTemp == 0x00) && ((bBus == 0) && (bDev == 0)))	/* Host */
					iHost++;

				else if (bTemp == 0x04)	/* PCI-PCI */
				{
					iPciPci++;

					/* Read the sub-bus as there may hard-coded be gaps */

					bTemp = (UINT8) dPciReadReg (pfa, PCI_SUB_BUS, REG_8);
					if (bTemp > bHiBus)
						bHiBus = bTemp;
				}

				/* Max PCI bus number is at least = Host + PCI bridges */

				if ((UINT8)(iHost + iPciPci) > bHiBus)
					bHiBus = (UINT8)(iHost + iPciPci);
			}
		}

		if (bMultiFunc)		/* multi-func. devices increment function */
		{
			(bFunc)++;

			if (bFunc == 8)	/* last function of device ? */
			{
				bFunc = 0;	/* reset function number */
				(bDev)++;		/* increment device */
				bMultiFunc = 0;	/* clear multi-function flag */
			}
		}

		else			/* single function devices increment device */
			(bDev)++;

		if (bDev == 32)		/* last device on bus ? */
		{
			bDev = 0;			/* reset device number, increment bus */

			if (bBus == bHiBus)		 /* last scannable bus ? */
				wStatus = DEVICE_NOT_FOUND;
			else
				(bBus)++;
		}

	} /* while (wStatus == PCI_SCANNING) */

	puts ("");

	/* Summarise bus topology */

	sprintf (achLine, "Highest Bus: %d, Number of Host Bridges: %d, Number of PCI-PCI bridges: %d",
						bHiBus, iHost + 1, iPciPci);
	puts (achLine);

} /* vPciScan () */


/****************************************************************************
 * vPciRead: display PCI configuraton space as 8, 16 or 32-bit
 *
 * RETURNS: none
 */

void vPciRead
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	PCI_PFA	pfa;
	UINT32	dIndex;
	UINT32	dLoop;
	UINT32	dData;
	int		iStatus;
	char	achLine[80];
	char	achItem[10];


	iStatus = iExtractArgs (&sArgs,
							ARG__DEV | ARG__REGISTER,
							ARG__BUS | ARG__FUNC | ARG__LENGTH | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	pfa = PCI_MAKE_PFA (sArgs.bBus, sArgs.bDev, sArgs.bFunc);


	/* Perform the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
				sprintf (achLine, "(R) PCI Config. %02Xh:",
							sArgs.bRegister + dIndex);

			switch (iCmdType)
			{
				case TYPE__PCI_READ_BYTE:

					dData = dPciReadReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_8);
					sprintf (achItem, " %02X", (UINT8)dData);
					dIndex += 1;
					break;

				case TYPE__PCI_READ_WORD:

					dData = dPciReadReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_16);
					sprintf (achItem, " %04X", (UINT16)dData);
					dIndex += 2;
					break;

				case TYPE__PCI_READ_DWORD:

					dData = dPciReadReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_32);
					sprintf (achItem, " %08X", dData);
					dIndex += 4;
					break;
			};

			strcat (achLine, achItem);

			if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
				 (dIndex >= sArgs.dLength) )
			{
				puts (achLine);
			}
		}

		iStatus = iCheckForEsc ();
	}

} /* vPciRead () */


/****************************************************************************
 * vPciWrite: write PCI configuration space register as 8, 16 or 32 bits
 *
 * RETURNS:
 */

void vPciWrite
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	PCI_PFA	pfa;
	UINT32	dIndex;
	UINT32	dLoop;
	UINT32	dData;
	int		iStatus;
	char	achLine[80];
	char	achItem[10];


	iStatus = iExtractArgs (&sArgs,
							ARG__DEV | ARG__REGISTER | ARG__DATA,
							ARG__BUS | ARG__FUNC | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	pfa = PCI_MAKE_PFA (sArgs.bBus, sArgs.bDev, sArgs.bFunc);


	/* Perfom the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
				sprintf (achLine, "(W) PCI config. %02Xh:",
							sArgs.bRegister + dIndex);

			switch (iCmdType)
			{
				case TYPE__PCI_WRITE_BYTE:

					vPciWriteReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_8, sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = dPciReadReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_8);
						sprintf (achItem, " %02X", (UINT8)dData);
					}

					dIndex += 1;
					break;

				case TYPE__PCI_WRITE_WORD:

					vPciWriteReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_16, sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = dPciReadReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_16);
						sprintf (achItem, " %04X", (UINT16)dData);
					}

					dIndex += 2;
					break;

				case TYPE__PCI_WRITE_DWORD:

					vPciWriteReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_32, sArgs.dData);

					if (iGetConfirmFlag () != 0)
					{
						dData = dPciReadReg (pfa, (UINT8)(sArgs.bRegister + dIndex), REG_32);
						sprintf (achItem, " %08X", dData);
					}

					dIndex += 4;
					break;
			};

			if (iGetConfirmFlag () != 0)
			{
				strcat (achLine, achItem);

				if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
					 (dIndex >= sArgs.dLength) )
				{
					puts (achLine);
				}
			}
		}

		iStatus = iCheckForEsc ();
	}

} /* vPciWrite () */


/****************************************************************************
 * vPciQuery: display detailed information regarding a specific PCI device
 *
 * RETURNS: none
 */

void vPciQuery
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	PCI_PFA	pfa;
	UINT32	dTemp;
	UINT16	wVendor;
	UINT16	wDevice;
	UINT16	wTemp;
	UINT8	bIntPin;
	UINT8	bIntLine;
	UINT8	bHeader;
	UINT8	bTemp;
	int		iStatus;
	char	achLine[80];


	(void)iCmdType;   	/* reference param to prevent compiler errors */

	iStatus = iExtractArgs (&sArgs,
							ARG__DEV, ARG__BUS | ARG__FUNC,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	pfa = PCI_MAKE_PFA (sArgs.bBus, sArgs.bDev, sArgs.bFunc);


	/*
	 * Extract the Data and display - standard header part
	 */

	/* Vendor and Device names */

	wVendor = (UINT16)dPciReadReg (pfa, PCI_VENDOR_ID, REG_16);

	if (wVendor == 0xFFFF)
	{
		puts ("No Device Present");
		return;
	}

	wDevice = (UINT16)dPciReadReg (pfa, PCI_DEVICE_ID, REG_16);

	sprintf (achLine, "Vendor: %04Xh  Device: %04Xh", wVendor, wDevice);
	puts (achLine);

	/* Revision */

	bTemp = (UINT8) dPciReadReg (pfa, PCI_REVISION_ID, REG_8);

	bHeader = (UINT8)dPciReadReg (pfa, PCI_HEADER_TYPE, REG_8);
	bHeader &= 0x7F;	/* strip MF bit */

	sprintf (achLine, "Revision: %02Xh  Header Type: %02Xh", bTemp, bHeader);
	puts (achLine);

	/* Class Code */

	dTemp = dPciReadReg (pfa, 0x08, REG_32);

	sprintf (achLine, "Base Class: %02Xh, Sub Class: %02Xh, Interface: %02Xh",
				(UINT8)((dTemp & 0xFF000000L) >> 24),
				(UINT8)((dTemp & 0x00FF0000L) >> 16),
				(UINT8)((dTemp & 0x0000FF00L) >> 8));
	puts (achLine);

	/* Memory, I/O and Bust Master enables */

	wTemp = (UINT16) dPciReadReg (pfa, 0x04, REG_16);

	if ((wTemp & 0x0001) == 0)
		strcpy (achLine, "I/O: disabled,  ");
	else
		strcpy (achLine, "I/O: enabled,  ");

	if ((wTemp & 0x0002) == 0)
		strcat (achLine, "Memory: disabled,  ");
	else
		strcat (achLine, "Memory: enabled,  ");

	if ((wTemp & 0x0004) == 0)
		strcat (achLine, "Bus Master: disabled");
	else
		strcat (achLine, "Bus Master: enabled");

	puts (achLine);

	/* Interrupts */

	bIntPin  = (UINT8)dPciReadReg (pfa, 0x3D, REG_8);
	bIntLine = (UINT8)dPciReadReg (pfa, 0x3C, REG_8);

	if (bIntPin == 0)
	{
		sprintf (achLine, "No Interrupt Pin");
	}

	else if (bIntLine == 0xFF)
	{
		sprintf (achLine, "Interrupt Pin: INT%c  Interrupt Line: Not Configured",
					'A' + bIntPin - 1);
	}

	else
	{
		sprintf (achLine, "Interrupt Pin: INT%c  Interrupt Line: IRQ%d",
					'A' + bIntPin - 1, bIntLine);
	}

	puts (achLine);


	/* Process the remainder according to type */

	switch (bHeader)
	{
		case 0 :
			puts ("- PCI Device -");
			vPciHeaderType0 (&sArgs);
			break;

		case 1 :
			puts ("- PCI-to-PCI Bridge -");
			vPciHeaderType1 (&sArgs);
			break;

		case 2 :
			puts ("- CardBus Bridge -");
			vPciHeaderType2 (&sArgs);
			break;

		default :
			sprintf (achLine, "Unsupported header type: %02Xh", bHeader);
			puts (achLine);
			break;
	};

} /* vPciQuery () */


/****************************************************************************
 * vPciHeaderType0: display PCI type-0, device-specific register details
 *
 * RETURNS: None
 */

static void vPciHeaderType0
(
	ARGS*	psArgs
)
{
	PCI_PFA	pfa;
	UINT32	dAddr;
	UINT32	dSize;
	UINT32	dTemp;
	int		iBarIndex;
	char	achLine[80];
	char	achType[10];


	pfa = PCI_MAKE_PFA (psArgs->bBus, psArgs->bDev, psArgs->bFunc);

	/* Base Address Registers */

	for (iBarIndex = 0; iBarIndex <= 5; iBarIndex++)
	{
		dTemp = dPciReadReg (pfa, 0x10 + (iBarIndex * 4), REG_32);

		if ((dTemp & 0x00000001L) == 0) /* Memory */
		{
			dAddr = dTemp & 0xFFFFFFF0L;

			vPciWriteReg (pfa, 0x10 + (iBarIndex * 4), REG_32, 0xFFFFFFF0L);

			dSize = dPciReadReg (pfa, 0x10 + (iBarIndex * 4), REG_32);
			dSize = ~(dSize & 0xFFFFFFF0L) + 1;

			strcpy (achType, "Mem");
		}

		else /* I/O */
		{
			dAddr = dTemp & 0xFFFFFFFCL;

			vPciWriteReg (pfa, 0x10 + (iBarIndex * 4), REG_32, 0xFFFFFFFCL);

			dSize = dPciReadReg (pfa, 0x10 + (iBarIndex * 4), REG_32);
			dSize = ~(dSize & 0xFFFFFFFCL) + 1;
			dSize = dSize & 0x0000FFFF;

			strcpy (achType, "I/O");
		}

		vPciWriteReg (pfa, 0x10 + (iBarIndex * 4), REG_32, dTemp);

		if (dSize != 0L)
		{
			sprintf (achLine, "BAR %02Xh (%s) - Base Address: %08Xh, Size: %Xh",
				(iBarIndex * 4 + 0x10), achType, dAddr, dSize);

			puts (achLine);
		}
	}

	/* Expansion ROM */

	dTemp = dPciReadReg (pfa, 0x30, REG_32);
	dAddr = dTemp & 0xFFFFFC00L;

	vPciWriteReg (pfa, 0x30, REG_32, 0xFFFFFC00L);

	dSize = dPciReadReg (pfa, 0x30, REG_32);
	dSize = ~(dSize & 0xFFFFFC00L) + 1;

	vPciWriteReg (pfa, 0x30, REG_32, dTemp);

	if (dSize != 0L)
	{
		sprintf (achLine, "Expansion ROM - Base Address: %08Xh, Size: %Xh, ",
				dAddr, dSize);

		if ((dTemp & 0x00000001L) == 0)
			strcat (achLine, "Disabled");
		else
			strcat (achLine, "Enabled");

		puts ("");
		puts (achLine);
	}

} /* vPciHeaderType0 () */


/****************************************************************************
 * vPciHeaderType1: display PCI type-1, device-specific register details
 *
 * RETURNS: None
 */

static void vPciHeaderType1
(
	ARGS*	psArgs
)
{
	PCI_PFA	pfa;
	UINT32	dAddr;
	UINT32	dSize;
	UINT32	dTemp;
	UINT32	dEx1;
	UINT32	dEx2;
	UINT16	wTemp;
	int		iBarIndex;
	char	achLine[80];
	char	achType[10];


	pfa = PCI_MAKE_PFA (psArgs->bBus, psArgs->bDev, psArgs->bFunc);

	/* Base Address Registers */

	for (iBarIndex = 0; iBarIndex <= 1; iBarIndex++)
	{
		dTemp = dPciReadReg (pfa, PCI_BAR0 + (iBarIndex * 4), REG_32);

		if ((dTemp & 0x00000001L) == 0) /* Memory */
		{
			dAddr = dTemp & 0xFFFFFFF0L;

			vPciWriteReg (pfa, PCI_BAR0 + (iBarIndex * 4), REG_32, 0xFFFFFFF0L);

			dSize = dPciReadReg (pfa, PCI_BAR0 + (iBarIndex * 4), REG_32);
			dSize = ~(dSize & 0xFFFFFFF0L) + 1;

			strcpy (achType, "Mem");
		}

		else /* I/O */
		{
			dAddr = dTemp & 0xFFFFFFFCL;

			vPciWriteReg (pfa, PCI_BAR0 + (iBarIndex * 4), REG_32, 0xFFFFFFFCL);

			dSize = dPciReadReg (pfa, PCI_BAR0 + (iBarIndex * 4), REG_32);
			dSize = ~(dSize & 0xFFFFFFFCL) + 1;
			dSize = dSize & 0x0000FFFF;

			strcpy (achType, "I/O");
		}

		vPciWriteReg (pfa, PCI_BAR0 + (iBarIndex * 4), REG_32, dTemp);

		if (dSize != 0L)
		{
			sprintf (achLine, "BAR %02Xh (%s) - Base Address: %08Xh, Size: %Xh",
				(iBarIndex * 4 + 0x10), achType, dAddr, dSize);

			puts (achLine);
		}
	}

	/* Bus numbering */

	dTemp = dPciReadReg (pfa, PCI_PRI_BUS, REG_32);

	sprintf (achLine, "Primary Bus: %d, Secondary Bus: %d, Subordinate Bus: %d",
				(UINT8)dTemp , (UINT8)(dTemp >> 8), (UINT8)(dTemp >> 16) );
	puts (achLine);

	/* I/O Window */

	wTemp = (UINT16)dPciReadReg (pfa, PCI_IO_BASE, REG_16);
	dEx1 = dPciReadReg (pfa, PCI_IO_UBASE, REG_16);
	dEx2 = dPciReadReg (pfa, PCI_IO_UBASE, REG_16);

	if ( ((wTemp & 0x00F0) >> 4) <= ((wTemp & 0xF000) >> 12) )	/* Base <= Limit? */
	{
		if ((wTemp & 0x0F0F) == 0x0101)
		{
			sprintf (achLine, "I/O window      - Base Address: %04X:%X000h,"
							  "         Limit: %04X:%XFFFh",
					(UINT16)dEx1, ((wTemp & 0x00F0) >> 4),
					(UINT16)dEx2, ((wTemp & 0xF000) >> 12));
		}

		else
		{
			sprintf (achLine, "I/O window      - Base Address: %X000h,     "
							  "         Limit: %XFFFh",
					((wTemp & 0x00F0) >> 4),
					((wTemp & 0xF000) >> 12));
		}
	}

	else
		sprintf (achLine, "I/O window      - disabled");

	puts (achLine);

	/* Memory Window */

	dTemp = dPciReadReg (pfa, PCI_MEM_BASE, REG_32);

	if ( ((dTemp & 0x0000FFFFL) >> 4) <= (dTemp >> 20) )	/* Base <= Limit? */
	{
		sprintf (achLine, "Memory window   - Base Address: %03X00000h,"
						  "          Limit: %03XFFFFFh",
				(UINT16)((dTemp & 0x0000FFFFL) >> 4) ,
				(UINT16)(dTemp >> 20) );
	}
	else
		sprintf (achLine, "Memory window   - disabled");

	puts (achLine);

	/* Prefetchable Memory Window */

	dTemp = dPciReadReg (pfa, PCI_PREF_BASE, REG_32);
	dEx1  = dPciReadReg (pfa, PCI_PREF_UBASE, REG_32);
	dEx2  = dPciReadReg (pfa, PCI_PREF_ULIMIT, REG_32);

	if ( ((dTemp & 0x0000FFFFL) >> 4) <= (dTemp >> 20) )	/* Base <= Limit? */
	{
		sprintf (achLine, "Prefetch window - Base Address: %08X:%03X00000h,"
						  " Limit: %08X:%03XFFFFFh",
				dEx1, (UINT16)((dTemp & 0x0000FFFFL) >> 4) ,
				dEx2, (UINT16)(dTemp >> 20) );
	}

	else
		sprintf (achLine, "Prefetch window - disabled");

	puts (achLine);

	/* Expansion ROM */

	dTemp = dPciReadReg (pfa, PCI_ROM_BAR, REG_32);
	dAddr = dTemp & 0xFFFFFC00L;

	vPciWriteReg (pfa, PCI_ROM_BAR, REG_32, 0xFFFFFC00L);

	dSize = dPciReadReg (pfa, PCI_ROM_BAR, REG_32);
	dSize = ~(dSize & 0xFFFFFC00L) + 1;

	vPciWriteReg (pfa, PCI_ROM_BAR, REG_32, dTemp);

	if (dSize != 0L)
	{
		sprintf (achLine, "Expansion ROM - Base Address: %08Xh, Size: %Xh, ",
				dAddr, dSize);

		if ((dTemp & 0x00000001L) == 0)
			strcat (achLine, "Disabled");
		else
			strcat (achLine, "Enabled");

		puts ("");
		puts (achLine);
	}

} /* vPciHeaderType1 () */


/****************************************************************************
 * vPciHeaderType2: display PCI type-2, device-specific register details
 *
 * RETURNS: None
 */

static void vPciHeaderType2
(
	ARGS*	psArgs
)
{
	PCI_PFA	pfa;
	UINT32	dTemp1;
	UINT32	dTemp2;
	UINT16	wTemp1;
	UINT16	wTemp2;
	char	achLine[80];


	pfa = PCI_MAKE_PFA (psArgs->bBus, psArgs->bDev, psArgs->bFunc);

	/* Bus numbering */

	dTemp1 = dPciReadReg (pfa, PCI_PRI_BUS, REG_32);

	sprintf (achLine, "Primary Bus: %d, CardBus: %d, Subordinate Bus: %d",
				(UINT8)dTemp1 , (UINT8)(dTemp1 >> 8), (UINT8)(dTemp1 >> 16) );
	puts (achLine);
	puts ("");

	/* I/O Windows */

	wTemp1 = (UINT16)dPciReadReg (pfa, 0x2C, REG_16);
	wTemp2 = (UINT16)dPciReadReg (pfa, 0x30, REG_16);

	if (wTemp1 != 0L)
		sprintf (achLine, "I/O window 0    - Base Address: %04Xh, Limit: %04Xh",
					wTemp1, wTemp2);
	else
		sprintf (achLine, "I/O window 0    - disabled");

	puts (achLine);

	wTemp1 = (UINT16)dPciReadReg (pfa, 0x34, REG_16);
	wTemp2 = (UINT16)dPciReadReg (pfa, 0x38, REG_16);

	if (wTemp1 != 0L)
		sprintf (achLine, "I/O window 1    - Base Address: %04Xh, Limit: %04Xh",
					wTemp1, wTemp2);
	else
		sprintf (achLine, "I/O window 1    - disabled");

	puts (achLine);

	/* Memory Windows */

	dTemp1 = dPciReadReg (pfa, 0x1C, REG_32);
	dTemp2 = dPciReadReg (pfa, 0x20, REG_32);

	if (dTemp1 != 0L)
		sprintf (achLine, "Memory window 0 - Base Address: %08Xh, Limit: %08Xh",
					dTemp1, dTemp2);
	else
		sprintf (achLine, "Memory window 0 - disabled");

	puts (achLine);

	dTemp1 = dPciReadReg (pfa, 0x24, REG_32);
	dTemp2 = dPciReadReg (pfa, 0x28, REG_32);

	if (dTemp1 != 0L)
		sprintf (achLine, "Memory window 1 - Base Address: %08Xh, Limit: %08Xh",
					dTemp1, dTemp2);
	else
		sprintf (achLine, "Memory window 1 - disabled");

	puts (achLine);

} /* vPciHeaderType2 () */



/****************************************************************************
 * vPciBridgeScan: perform a PCI bridge scan
 *
 * RETURNS: None
 */
void vPciBridgeScan
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	PCI_PFA	pfa;
	UINT16	wThisVid;		/* vendor ID at this address */
	UINT16	wThisDid;		/* device ID at this address */
	UINT16	wStatus;		/* status of the device scan */
	UINT8	bMultiFunc;		/* non-zero indicates multi-function device */
	UINT8	bHiBus;			/* records the current highest bus to scan */
	UINT8	bTemp;
	UINT8	bBus;
	UINT8	bDev;
	UINT8	bFunc;
	char	achLine[80];
	int		iHost = -1;		/* start at -1 so first host is bus-0 */
	int		iPciPci = 0;  	/* count PCI-PCI bridges */


	(void)iCmdType;   	/* reference params to prevent compiler errors */
	(void)iCmdCount;
	(void)achUserArgs;


	bFunc = 0;		/* start at PCI address zero */
	bDev  = 0;
	bBus  = 0;

	bMultiFunc	= 0;
	bHiBus		= 0;

	wStatus = PCI_SCANNING;		/* indicate bus scan in progress */

	/* Perform the operation */

	while (wStatus == PCI_SCANNING)
	{
		pfa = PCI_MAKE_PFA (bBus, bDev, bFunc);

		wThisVid = (UINT16) dPciReadReg (pfa, PCI_VENDOR_ID, REG_16);
		wThisDid = (UINT16) dPciReadReg (pfa, PCI_DEVICE_ID, REG_16);

		if (wThisVid != 0xFFFF)
		{
			if (bFunc == 0)		/* see if multi-func */
			{
				bTemp = (UINT8) dPciReadReg (pfa, PCI_HEADER_TYPE, REG_8);
				bMultiFunc = (bTemp & 0x80) || ((bBus == 0) && (bDev == 0));
			}

			/* Check for bridge - class code = 06 */

			bTemp = (UINT8) dPciReadReg (pfa, PCI_BASE_CLASS, REG_8);
			if (bTemp == 0x06)
			{
				bTemp = (UINT8) dPciReadReg (pfa, PCI_SUB_CLASS, REG_8);

				if (bTemp > 7)	/* force type to 'other' if not known */
					bTemp = 8;

				sprintf (achLine, "Bus: %d Dev: %2d Func: %d = Vendor: %04Xh Device: %04Xh  %s",
							bBus, bDev, bFunc, wThisVid, wThisDid, achBridgeType[bTemp]);
				puts (achLine);

				if (bTemp == 0x00)		/* Host */
					iHost++;

				else if (bTemp == 0x04)	/* PCI-PCI */
				{
					iPciPci++;

					bTemp = (UINT8) dPciReadReg (pfa, PCI_SUB_BUS, REG_8);
					if (bTemp > bHiBus)
						bHiBus = bTemp;
				}

				/* Max PCI bus number is at least = Host + PCI bridges */

				if ((UINT8)(iHost + iPciPci) > bHiBus)
					bHiBus = (UINT8)(iHost + iPciPci);
			}
		}

		if (bMultiFunc)		/* multi-func. devices increment function */
		{
			(bFunc)++;

			if (bFunc == 8)	/* last function of device ? */
			{
				bFunc = 0;	/* reset function number */
				(bDev)++;		/* increment device */
				bMultiFunc = 0;	/* clear multi-function flag */
			}
		}

		else			/* single function devices increment device */
			(bDev)++;

		if (bDev == 32)		/* last device on bus ? */
		{
			bDev = 0;			/* reset device number, increment bus */

			if (bBus == bHiBus)		 /* last scannable bus ? */
				wStatus = DEVICE_NOT_FOUND;
			else
				(bBus)++;
		}

	} /* while (wStatus == PCI_SCANNING) */

	puts ("");

	/* Summarise bus topology */

	sprintf (achLine, "Highest Bus: %d, Number of Host Bridges: %d, Number of PCI-PCI bridges: %d",
						bHiBus, iHost + 1, iPciPci);
	puts (achLine);

} /* vPciBridgeScan () */



/****************************************************************************
 * vCpuid: display key details returned by the CPUID instruction
 *
 * RETURNS: None
 */

void vCpuid
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	UINT32	dHighest;
	UINT8	bBitPos;
	int		iLoop;
	char	achLine[80];
	struct
	{
		UINT32	eax;
		UINT32	ebx;	/* B,D,C order for easy display of CPU manuf. string */
		UINT32	edx;
		UINT32	ecx;
		UINT8	bNull;
	} sRegs;


	(void)iCmdType;   	/* reference params to prevent compiler errors */
	(void)iCmdCount;
	(void)achUserArgs;

	sRegs.bNull = 0;

	/* CPUID 0 */

	sysCpuid (0, (void*)&sRegs);

	sprintf (achLine, "CPU string      = %s", (char*)&sRegs.ebx);
	puts (achLine);

	dHighest = sRegs.eax;

	/* CPUID 1 */

	if (dHighest >= 1)
	{
		sysCpuid (1, (void*)&sRegs);

		sprintf (achLine, "CPU Type        = %lX", (sRegs.eax & 0x00003000L) >> 12);
		puts (achLine);

		sprintf (achLine, "CPUID           = %lX", (sRegs.eax & 0x00000FFFL));
		puts (achLine);

		sprintf (achLine, "                = Family %X, Model %X, Stepping %X. Extd. (Family %X, Model %X)",
				(int)(sRegs.eax & 0x00000F00L) >> 8,
				(int)(sRegs.eax & 0x000000F0L) >> 4,
				(int)(sRegs.eax & 0x0000000FL),
				(int)(sRegs.eax & 0x000F0000L) >> 16,
				(int)(sRegs.eax & 0x0FF00000L) >> 20);
		puts (achLine);

		sprintf (achLine, "Brand ID        = %02lX", (sRegs.ebx & 0x000000FFL));
		puts (achLine);
		sprintf (achLine, "APIC ID         = %02lX", (sRegs.ebx & 0xFF000000L) >> 24);
		puts (achLine);
		sprintf (achLine, "Cache Line Size = %d", (int)(sRegs.ebx & 0x0000FF00L) >> 5);	/* REG*8 */
 		puts (achLine);

		/* Feature flags */

		sprintf (achLine, "Feature flags   = %08X", sRegs.edx);
		puts (achLine);

		sprintf (achLine, "                = ");
		bBitPos = 31;

		while (bBitPos != 255)
		{
			if (sRegs.edx & (1L << bBitPos))
				strcat (achLine, achCpuidFeatures[bBitPos]);

			if (bBitPos == 16)
			{
				puts (achLine);
				sprintf (achLine, "                = ");
			}

			bBitPos--;
		}

		puts (achLine);

	}

	/* CPUID 0x80000000 plus */

	sysCpuid (0x80000000L, (void*)&sRegs);
	dHighest = sRegs.eax;

	if (dHighest >= 0x80000004L)
	{
		sprintf (achLine, "Brand String    = ");

		iLoop = strlen (achLine);

		sysCpuid (0x80000002L, (void*)&sRegs);
		memcpy (&achLine[iLoop],  &sRegs.eax, 4);
		memcpy (&achLine[iLoop + 4],  &sRegs.ebx, 4);
		memcpy (&achLine[iLoop + 8],  &sRegs.ecx, 4);
		memcpy (&achLine[iLoop + 12], &sRegs.edx, 4);

		sysCpuid (0x80000003L, (void*)&sRegs);
		memcpy (&achLine[iLoop + 16], &sRegs.eax, 4);
		memcpy (&achLine[iLoop + 20], &sRegs.ebx, 4);
		memcpy (&achLine[iLoop + 24], &sRegs.ecx, 4);
		memcpy (&achLine[iLoop + 28], &sRegs.edx, 4);

		sysCpuid (0x80000004L, (void*)&sRegs);
		memcpy (&achLine[iLoop + 32], &sRegs.eax, 4);
		memcpy (&achLine[iLoop + 36], &sRegs.ebx, 4);
		memcpy (&achLine[iLoop + 40], &sRegs.ecx, 4);
		memcpy (&achLine[iLoop + 44], &sRegs.edx, 4);

		achLine[iLoop + 48] = (char)0; /* should be NULL terminated, but just in case */

		puts (achLine);
	}

} /* vCpuid () */


/***************************************************************************
 * vTestMenu: display a list of available tests, or details for a test
 *
 * RETURNS: none
 */

void vTestMenu
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	TEST_ITEM*	psTestList;
	UINT16	wTestNum;
	char	achBuffer[81];
	int		iIndex;
	int		iLineCount;
	int		iTestCount;


	/* Get the test number, in decimal */

	if (iCmdCount > 1)
		wTestNum = atoi (achUserArgs[1]);
	else
		wTestNum = 0;


	/* Get the test directory */
	board_service(SERVICE__BRD_GET_TEST_DIRECTORY, NULL, &psTestList);

	if (psTestList == NULL)
		return;


	/* Display the directory, formatted for an 80 column screen */

	if (wTestNum == 0)
	{
		iLineCount = 0;
		iTestCount = 0;

		for (iIndex = 0; psTestList[iIndex].wTestNum != 0; iIndex++)
		{
			// check if any test permissions are set
			if (psTestList[iIndex].dFlags != 0)
			{
				++iTestCount;
				sprintf (achBuffer, "%4d: %-33s",
									psTestList[iIndex].wTestNum,
									psTestList[iIndex].achTestName);
				cputs (achBuffer);

				if ((iTestCount & 1) == 1)	/* add spaces for column separation */
				{
					cputs ("  ");
				}
				else
				{
					puts ("");			/* terminate line, prevents display issue  */
										/* seen on some boards at high baud rates */
					iLineCount++;		/* count full lines */
				}

				/* Pause every 20 full lines, provided there is more to display */
	
				if ((iLineCount >= 20) && (psTestList[iIndex + 1].wTestNum != 0))
				{
					puts ("** Press any key to continue **");

					while (iGetExtdKeyPress () == 0);
					iLineCount = 0;
				}
			}
		}

		/* Need to terminate last line correctly if only one test */

		if ((iTestCount & 1) == 1)
			puts ("");

		return;
	}

	/* Find an individual test */

	for (iIndex = 0; psTestList[iIndex].wTestNum != 0; iIndex++)
	{
		if (psTestList[iIndex].wTestNum == wTestNum)
			break;
	}

	if (psTestList[iIndex].wTestNum == 0)
	{
		puts ("Test not found.");
		return;
	}

	/* Display test details */

	sprintf (achBuffer, "Test %d: %s",
						psTestList[iIndex].wTestNum,
						psTestList[iIndex].achTestName);
	puts (achBuffer);

	puts ("Test mode flags:");

	cputs ("    PBIT 10 Seconds: ");
	(psTestList[iIndex].dFlags & BITF__PBIT_10S) ? puts ("Yes") : puts ("No");

	cputs ("    PBIT 30 Seconds: ");
	(psTestList[iIndex].dFlags & BITF__PBIT_30S) ? puts ("Yes") : puts ("No");

	cputs ("    PBIT 60 Seconds: ");
	(psTestList[iIndex].dFlags & BITF__PBIT_60S) ? puts ("Yes") : puts ("No");

	cputs ("    PBIT Test Mode:  ");
	(psTestList[iIndex].dFlags & BITF__PBIT_TEST) ? puts ("Yes") : puts ("No");

	cputs ("    IBIT:            ");
	(psTestList[iIndex].dFlags & BITF__LBIT) ? puts ("Yes") : puts ("No");

	sprintf (achBuffer, "Duration: %u mS", psTestList[iIndex].wDuration);
	puts (achBuffer);

} /* vTestMenu () */


/***************************************************************************
 * vGpf: throw a GPF exception (used in testing only)
 *
 * RETURNS: none (should not return)
 */

void vGpf
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	(void)iCmdType;
	(void)iCmdCount;
	(void)achUserArgs;

	puts ("Throw GPF:\n");

	_sysThrowGpf ();

	puts ("GPF did not occur!");

} /* vGpf () */



/****************************************************************************
 * vNvRead: display contents of CMOS NVRAM
 *
 * RETURNS: None
 */

void vNvRead
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	UINT32	dIndex;
	UINT32	dLoop;
	UINT16	wNvBase;
	UINT8	bData;
	int		iStatus;
	char	achLine[80];
	char	achItem[10];


	(void)iCmdType;   	/* reference params to prevent compiler errors */

	iStatus = iExtractArgs (&sArgs,
							ARG__ADDR,
							ARG__LENGTH | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	/* Perform the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
			{
				sprintf (achLine, "(R) CMOS %02Xh:",
							(UINT16)(sArgs.dAddr + dIndex));
			}

			if ((UINT8)(sArgs.dAddr + dIndex) < 0x80)
				wNvBase = 0x70;
			else
				wNvBase = 0x72;

			sysOutPort8 (wNvBase, (UINT8)(sArgs.dAddr + dIndex));
			bData = sysInPort8 (wNvBase + 1);

			sprintf (achItem, " %02X", bData);
			dIndex += 1;

			strcat (achLine, achItem);

			if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
				 (dIndex >= sArgs.dLength) )
			{
				puts (achLine);
			}
		}

		iStatus = iCheckForEsc ();
	}

} /* vNvRead () */


/****************************************************************************
 * vNvWrite: write to CMOS NVRAM
 *
 * RETURNS: None
 */

void vNvWrite
(
	int iCmdType,
	int iCmdCount,
	char* achUserArgs[]
)
{
	ARGS	sArgs;
	UINT32	dIndex;
	UINT32	dLoop;
    UINT16	wNvBase;
	UINT8	bData;
	int		iStatus;
	char	achLine[80];
	char	achItem[10];


	(void)iCmdType;   	/* reference params to prevent compiler errors */

	iStatus = iExtractArgs (&sArgs,
							ARG__ADDR | ARG__DATA,
							ARG__LENGTH | ARG__LOOP,
							iCmdCount, achUserArgs);
	if (iStatus != E__OK)
		return;

	/* Perform the operation */

	for (dLoop = 0; (dLoop < sArgs.dCount) && (iStatus == 0); dLoop++)
	{
		for (dIndex = 0; dIndex < sArgs.dLength;)
		{
			if ((dIndex & 0x0000000FL) == 0L)
			{
				sprintf (achLine, "(W) CMOS %02Xh:",
							(UINT16)(sArgs.dAddr + dIndex));
			}

			if ((UINT8)(sArgs.dAddr + dIndex) < 0x80)
				wNvBase = 0x70;
			else
				wNvBase = 0x72;

			sysOutPort8 (wNvBase, (UINT8)(sArgs.dAddr + dIndex));
			sysOutPort8 (wNvBase + 1, (UINT8)sArgs.dData);

			if (iGetConfirmFlag () != 0)
			{
				bData = sysInPort8 (wNvBase + 1);
				sprintf (achItem, " %02X", bData);
			}

			dIndex += 1;

			if (iGetConfirmFlag () != 0)
			{
				strcat (achLine, achItem);

				if ( ((dIndex != 0) && ((dIndex & 0xFL) == 0x0L)) ||
					 (dIndex >= sArgs.dLength) )
				{
					puts (achLine);
				}
			}
		}

		iStatus = iCheckForEsc ();
	}

} /* vNvWrite () */


