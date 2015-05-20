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


#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>

#include <bit/bit.h>
#include <bit/console.h>
 
#include <bit/board_service.h>
#include <cute/idx.h>

#include <private/debug.h>

//#define DEBUG

#ifdef DEBUG
	static char buffer[128];
#endif

/* defines */
#define MAX_COOP_ENTRIES	12		/* defined by the soak firmware */
#define	TYPE_MSA2			2		/* extended info MSA firmware */

/* typedefs */

#pragma pack(1)
typedef struct tagBoardInfo
{
	UINT8	bFwType;
	UINT8	bFwVersion;
	UINT8	bFwRevision;
	UINT16	wInfoSize;
	UINT32	dDualPortSize;
	UINT32	dOffboardMemSize;
	UINT32	dOffboardMemLimit;

} BOARD_INFO;

typedef struct tagCoopInfo
{
	UINT16	wType;
	UINT8	bTestId;
	UINT8	bAux;

} COOP_INFO;

typedef struct tagSoakInfo
{
	BOARD_INFO	sBoardInfo;
	COOP_INFO	asCoopInfo[MAX_COOP_ENTRIES];

} SOAK_INFO;
#pragma pack()

/* typedefs */

/* constants */
const char achBoardText[] = "FW type <B>, version <B>.<B>, (hdr <W>)\n\r"	\
							"dp size <D>\n\r"								\
							"ob mem <D>-<D>\n\r";

const char achCoopText[] = ": <WT>,<BT>,<B>";

/* locals */

/* globals */

/* externals */

/* forward declarations */


/*****************************************************************************
 * PassingTest: test function that always returns E__OK
 *
 * RETURNS: None
 */

TEST_INTERFACE (CctTestInfo, "Retreive BIST Information")
{

    SOAK_INFO	sSoakInfo;			/* structure to hold info for soak */
    COOP_ENTRY* psCoopTestTable;	/* pointer to the COOP table */
    UINT8	bIndex;
    char	achMsgText[256];
    brd_info bInfo;
    UINT32 status;

#ifdef DEBUG
	sprintf(buffer, "Queried by soak master for coop test list\n");
	sysDebugWriteString(buffer);
#endif

    /* Begin packet with the board details */
    strcpy (achMsgText, achBoardText);

    board_service(SERVICE__BRD_CUTE_INFO, NULL, &bInfo);

    sSoakInfo.sBoardInfo.bFwType           = TYPE_MSA2;
    sSoakInfo.sBoardInfo.bFwVersion        = bInfo.firmwareVersion;
    sSoakInfo.sBoardInfo.bFwRevision       = bInfo.firmwareVersion;
    sSoakInfo.sBoardInfo.wInfoSize         = sizeof (BOARD_INFO);
    sSoakInfo.sBoardInfo.dDualPortSize     = (bInfo.memory << 16) - 0x100000;
    sSoakInfo.sBoardInfo.dOffboardMemSize  = (bInfo.memory << 16);
    sSoakInfo.sBoardInfo.dOffboardMemLimit = 0xBFFFFFFF;

	#ifdef DEBUG
		sprintf(buffer, "dDualPortSize %x\n", sSoakInfo.sBoardInfo.dDualPortSize);
		sysDebugWriteString(buffer);
		sprintf(buffer, "dOffboardMemSize %x\n", sSoakInfo.sBoardInfo.dOffboardMemSize);
		sysDebugWriteString(buffer);
	#endif

    board_service(SERVICE__BRD_GET_COOP_TEST_TBL, NULL, &psCoopTestTable);

    bIndex = 0;

    while (psCoopTestTable[bIndex].bTestId != 0)
    {
    	strcat (achMsgText, achCoopText);

		sSoakInfo.asCoopInfo[bIndex].wType   = psCoopTestTable[bIndex].wType;
		sSoakInfo.asCoopInfo[bIndex].bTestId = psCoopTestTable[bIndex].bTestId;
		sSoakInfo.asCoopInfo[bIndex].bAux    = psCoopTestTable[bIndex].bAux;

		#ifdef DEBUG
			sprintf(buffer, "asCoopInfo wType%x, bTestId %d, bAux %x\n",
					sSoakInfo.asCoopInfo[bIndex].wType,
					sSoakInfo.asCoopInfo[bIndex].bTestId,
					sSoakInfo.asCoopInfo[bIndex].bAux);
			sysDebugWriteString(buffer);
		#endif

		bIndex++;
    }

    /* Add a NULL COOP record to terminate the list */
    strcat (achMsgText, achCoopText);

    sSoakInfo.asCoopInfo[bIndex].wType		= 0;
    sSoakInfo.asCoopInfo[bIndex].bTestId	= 0;
    sSoakInfo.asCoopInfo[bIndex].bAux		= 0;

    /* The soak master needs the data packet, not the string, so we must
     * use the IDX write to send this
     */

   status=wIDX_Pwrite (NULL, achMsgText, (UINT8*)&sSoakInfo);

   if( status != E__OK )
   {
		#ifdef DEBUG
			sprintf(buffer, "T 127 Error sending coop information\n");
			sysDebugWriteString(buffer);
		#endif
   }
   else
   {
		#ifdef DEBUG
			sprintf(buffer, "T 127 Executed successfully (%x)\n",sSoakInfo.sBoardInfo.dDualPortSize);
			sysDebugWriteString(buffer);
		#endif
   }

    return (E__OK);

} /* PassingTest */
