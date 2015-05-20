/*
 * idx.h
 *
 *  Created on: 27 Apr 2010
 *      Author: engineer
 */

#ifndef IDX_H_
#define IDX_H_

#pragma pack (1)
typedef struct tagExecuteRequest
{
	UINT8	bTestId;
	UINT8	bTestingLevel;
	UINT8	bErrorReportingLevel;
	UINT8	bErrorAction;
	UINT8	bTestInit;

} EXECUTE_REQUEST;

typedef struct tagExecuteResponse
{
	UINT8	bParcelType;
	UINT8	bTestId;
	UINT16	wTestStatus;

} EXECUTE_RESPONSE;

typedef struct tagInvalidResponse
{
	UINT8	bParcelType;
	UINT8	bReqParcelType;

} INVALID_RESPONSE;

typedef struct tagExpectedTypes
{
	UINT8	bNumParams;
	UINT8	pbTypes[255];	/* can have 1-255 parameters */

} EXPECTED_TYPES;

typedef struct tagPrintControl
{
	UINT8	bErl;
	UINT8	bThisLine;
	UINT8	bColumn;

} PRINT_CONTROL;

#pragma pack()

#define IDX_UINT8_TYPE					0x01	/* parsed parameter types */
#define IDX_UINT16_TYPE					0x02
#define IDX_UINT32_TYPE					0x03
#define IDX_STRING_TYPE					0x10

/* CCT cooperating test types - values also used by the Soak Master */

#define CP_NONE				0 		/* Not a cooperating test. */
#define CP_DUAL_ADDR		1 		/* Dual addressing memory test. */
#define CP_READ_WRITE		2 		/* Read after write memory test. */
#define CP_EXECUTE			4 		/* Code execution from memory test. */
#define CP_INTERFACE		5 		/* Interface test: aux is if_type */
#define	CP_SUPONLY			6	 	/* Support required: aux=support_id */
#define	CP_SUPONLY2			7	 	/* Support required: aux=support_id */
#define	CP_SUPONLY3			13		/* another support only */
#define	CP_SUPONLY4			14		/* and another - just in case ! */


UINT32 wIDX_Pwrite ( PRINT_CONTROL*	psPrintControl,char*	achMessage,UINT8*	pbParams);
UINT32 wIDX_GetParams(EXPECTED_TYPES*	psExpectedTypes,UINT8*	pbDest);
UINT32 wIDX_ReceiveParcel( UINT8 bSlot, UINT8* pbData, UINT16 wMaxSize, UINT16* pwActualSize);
UINT32 wIDX_SendParcel( UINT8 bSlot, UINT8* pbData, UINT16 wSize );
UINT32 wIDX_DetectParcel( UINT8	bSlot, UINT8* pbType);
UINT32 wIDX_BadParcel( UINT8 bSlot, UINT8 bType, UINT8 bReplyFlag );

#endif /* IDX_H_ */
