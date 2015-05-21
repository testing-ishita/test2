#include <stdtypes.h>
#include <stdio.h>
#include <string.h>
#include <errors.h>


#include <bit/bit.h>
#include <bit/delay.h>
#include <bit/board_service.h>

#include <private/cpu.h>
#include <private/debug.h>

#include <cute/arch.h>
#include <cute/interconnect.h>
#include <cute/cute_errors.h>
#include <cute/sth.h>
#include <cute/packet.h>
#include <cute/sm3.h>
#include <cute/sm3_idx.h>

//#define DEBUG_SOAK

static uPacket sReplyPacket;
static struc_get_params_res params_res;
static saved_params params;

sm3vmesyscon syscon;

extern sm3vmeSlave asSlots[ MAX_SLAVES ];


static void   vSthHelp     ( UINT8 bSlot, uPacket *psPkt );
static void   vSthExecute  ( UINT8 bSlot, uPacket *psPkt );
static UINT8  get_byte     ( UINT8 bSlot, UINT8 *ptr );
static void   vSthClock    ( uPacket *sPkt );
static void   pTestExecResp( UINT8 bSlot, UINT8* pbData, UINT16 wMaxSize, UINT16 size );
static void   pHconReq     ( UINT8 bSlot, UINT8* pbData, UINT16 wMaxSize, UINT16 size );
static void   pPadWrReq    ( UINT8 bSlot, UINT8* pbData, UINT16 wMaxSize, UINT16 size );
static void   vpostErrorMsg( UINT8 bSlot, char* buffer );
static UINT32 vgetErrorMsg ( UINT8 bSlot, char* buffer );
static UINT32 retryCoopTest(UINT8 bSlot, UINT32 type);


/*****************************************************************************
 *  TITLE:  processSlavePackets ()									[SM3-VME]*
 *****************************************************************************/

static UINT8   rBuff[1024];

void processSlaveRxPackets( UINT8 bSlot )
{
	UINT8	parcel_type;
	UINT32	status;
	UINT16  length;
#ifdef DEBUG_SOAK
	UINT8   buff[64];
#endif
	do
	{
		status = wIDX_DetectParcel( bSlot, &parcel_type );

		if( status == E__OK )
		{
			if( parcel_type == 0xFF )
			{
				#ifdef DEBUG_SOAK
					sysDebugWriteString( "Invalid Parcel 0xFF\n" );
				#endif
				return;
			}

			status = wIDX_ReceiveParcel( bSlot, rBuff, 1024, &length );

			switch( parcel_type )
			{
				case EXECUTE_TEST_RESPONSE:
					 pTestExecResp( bSlot, rBuff, 1024, length );
					 break;

				case HARDWARE_CONFIGURATION_REQUEST:
					 pHconReq( bSlot, rBuff, 1024, length );
					 break;

				case GET_PARAMETERS_REQUEST:
					 pParamReq( bSlot, rBuff, 1024, length );
					 break;

				case PAD_WRITE_REQUEST:
					 pPadWrReq( bSlot, rBuff, 1024, length );
					 break;

				default:
					#ifdef DEBUG_SOAK
						sprintf( buff, "Slot %d, Unknown Parcel %x\n", bSlot, parcel_type );
						sysDebugWriteString( buff );
					#endif
					status = wIDX_BadParcel( bSlot, parcel_type, 1 );
					break;
			}
		}

	}while( status == E__OK );
}




/*****************************************************************************
 *  TITLE:  pPadWrReq()												[SM3-VME]*
 *****************************************************************************/
static void pPadWrReq( UINT8 bSlot,
				       UINT8* pbData,
				       UINT16 wMaxSize,
				       UINT16 size
				      )
{
	UINT32 i, actualL, len;
	char   buffs[128];
	char   buffp[128];
	char   errbuff[256];

	actualL = pbData[1];
	actualL = ((actualL << 8) & 0xff00) | pbData[0];

	#ifdef DEBUG_SOAK
		sprintf( buffs, "\nPad Write Req slot %d, size %d, actual %d\n", bSlot, size, actualL );
		sysDebugWriteString( buffs );
	#endif

	#ifdef DEBUG_SOAK
		sysDebugWriteString( "Print Controls:" );
	#endif
	for(i = 0; i < 3; i++)
	{
		sprintf( buffs, "  0x%x", pbData[2+i] );
		#ifdef DEBUG_SOAK
			sysDebugWriteString(buffs);
		#endif
	}

	memset(buffs, 0x00,128);
	#ifdef DEBUG_SOAK
		sysDebugWriteString("\nString: ");
	#endif

	len = 0;
	while( (len < actualL-4) && (len < 128) && (pbData[5+len] != '\0') )
	{
		buffs[len] = pbData[5+len];
		len++;
	}
	buffs[len] = '\0';
	#ifdef DEBUG_SOAK
		sysDebugWriteString(buffs);
	#endif

	memset( buffp, 0x00, 128 );
	#ifdef DEBUG_SOAK
		sysDebugWriteString( "\nParams: " );
	#endif
	for( i = 0; ((i < actualL-len) && ((i*3) < 127)); i++ )
	{
		sprintf( &buffp[i*3], "%0x ", pbData[len+i] );
	}
	buffp[i*3] = '\0';
	#ifdef DEBUG_SOAK
		sysDebugWriteString(buffp);
		sysDebugWriteString("\n\n");
	#endif

	sprintf(errbuff, "%s %s", buffs, buffp);
	errbuff[255] = '\0';
	vpostErrorMsg( bSlot, errbuff);
}



/*****************************************************************************
 *  TITLE:  pParamReq()												[SM3-VME]*
 *****************************************************************************/

UINT32 pParamReq( UINT8  bSlot,
		          UINT8* pbData,
		          UINT16 wMaxSize,
		          UINT16 size
		         )
{
	UINT32 rt = E__OK, i, count, length;
#ifdef DEBUG_SOAK
	char   buff[32];
#endif

	memset(&params, 0x00, sizeof(saved_params));
	memset(&params_res, 0x00, sizeof(struc_get_params_res));

	params.length = pbData[1];
	params.length = ((params.length << 8) & 0xff00) | pbData[0];

	#ifdef DEBUG_SOAK
		sprintf(buff, "\n\nPARAM Req slot %x, size %x, actual %x\n", bSlot, size, params.length );
		sysDebugWriteString(buff);
		#if 0
		for (i = 0; i<size; i++)
		{
			sprintf(buff, "0x%X  ", pbData[i] );
			sysDebugWriteString(buff);
		}
		sysDebugWriteString("\n");
		#endif
	#endif


	if(params.length == (size-2) )
	{
		params.count = pbData[2];
		count = 3;
		for(i = 0; i < params.count; i++)
		{
			params.type[i] = pbData[count];
			count++;
		}

		for(i = 0; i < params.count; i++)
		{
			switch(params.type[i])
			{
				case IDX_UINT8_TYPE:
					params.data[i] = pbData[count++];
					break;

				case IDX_UINT16_TYPE:
					params.data[i] = pbData[count+1];
					params.data[i] = ((params.data[i] << 8) & 0x0000ff00) | pbData[count];
					count+=2;
					break;

				case IDX_UINT32_TYPE:
					params.data[i] = pbData[count+3];
					params.data[i] = ((params.data[i] << 8) & 0xffffff00) | pbData[count+2];
					params.data[i] = ((params.data[i] << 8) & 0xffffff00) | pbData[count+1];
					params.data[i] = ((params.data[i] << 8) & 0xffffff00) | pbData[count];
					count+=4;
					break;

				default:
					break;
			}
		}

		#ifdef DEBUG_SOAK
			#if 0
				sprintf(buff, "\nparams.count %d\n", params.count );
				sysDebugWriteString(buff);

				for (i = 0; i<params.count; i++)
				{
					sprintf(buff, "Type = 0x%x, Data = 0x%x  \n", params.type[i], params.data[i] );
					sysDebugWriteString(buff);
					sysDebugWriteString("\n");
				}
			#endif
		#endif
	}
	else
	{
		#ifdef DEBUG_SOAK
			sysDebugWriteString("Bad Packet length\n");
		#endif
		rt = E__FAIL;
	}


	if(rt == E__OK)
	{
		if( vProcessSavedParams( bSlot, &params ) == 1 )
		{
			#ifdef DEBUG_SOAK
				#if 0
					sprintf(buff, "\nparams.count %d\n", params.count );
					sysDebugWriteString(buff);

					for (i = 0; i<params.count; i++)
					{
						sprintf(buff, "Type = 0x%x, Data = 0x%x  \n", params.type[i], params.data[i] );
						sysDebugWriteString(buff);
						sysDebugWriteString("\n");
					}
				#endif
			#endif

			length = 0;
			for( i = 0; i<params.count; i++ )
			{
				switch(params.type[i])
				{
					case IDX_UINT8_TYPE:
						params_res.data[length] = params.data[i];
						length++;
						break;

					case IDX_UINT16_TYPE:
						params_res.data[length]   = (UINT8) params.data[i];
						params_res.data[length+1] = (UINT8)(params.data[i]>>8 );
						length+=2;
						break;

					case IDX_UINT32_TYPE:
						params_res.data[length]   = (UINT8) params.data[i];
						params_res.data[length+1] = (UINT8)(params.data[i]>>8 );
						params_res.data[length+2] = (UINT8)(params.data[i]>>16);
						params_res.data[length+3] = (UINT8)(params.data[i]>>24);
						length+=4;
						break;

					default:
						break;
				}
			}

			if(bSlot != SysCon_ID)
			{
				params_res.parcel_type = GET_PARAMETERS_RESPONSE;
				params_res.length      = 2+length;
				params_res.status      = E__OK;

				#if 0 //ifdef DEBUG_SOAK
					sysDebugWriteString("Data to send\n");
					for (i = 0; i<(5+length); i++)
					{
						sprintf(buff, "0x%x  ", ((UINT8*)&params_res)[i] );
						sysDebugWriteString(buff);

					}
					sysDebugWriteString("\n");
				#endif

				rt = wIDX_SendParcel (bSlot, (UINT8*)&params_res, (5+length));

				if ((rt != E_OK) && (rt != E_RETRY_OK) )
				{
					#ifdef DEBUG_SOAK
						sprintf(buff, "PARAM RSP Failed %x\n", rt );
						sysDebugWriteString(buff);
					#endif
					return E__FAIL;
				}
			}
			else
			{
				for(i = 0; i<length; i++)
				{
					pbData[i] = params_res.data[i];
				}
			}
		}
		else
		{
			#ifdef DEBUG_SOAK
				sysDebugWriteString("No params supplied use default\n");
			#endif
			rt = E__FAIL;
		}
	}

	if(rt == E__FAIL)
	{
		rt = E__FAIL;

		if(bSlot != SysCon_ID)
		{
			params_res.parcel_type = GET_PARAMETERS_RESPONSE;
			params_res.length      = 2;
			params_res.status      = E_IDX_USE_DEFAULTS;

			rt = wIDX_SendParcel (bSlot, (UINT8*)&params_res, 5);

			if ((rt != E_OK) && (rt != E_RETRY_OK) )
			{
				#ifdef DEBUG_SOAK
					sprintf(buff, "PARAM RSP Failed %x\n", rt );
					sysDebugWriteString(buff);
				#endif
				return E__FAIL;
			}
		}
	}

	return rt;
}



/*****************************************************************************
 *  TITLE:  pHconReq()												[SM3-VME]*
 *****************************************************************************/
static void pHconReq( UINT8  bSlot,
					  UINT8* pbData,
					  UINT16 wMaxSize,
					  UINT16 size
					)
{
	UINT32 rt;
#ifdef DEBUG_SOAK
	char   buff[32];
#endif
	struc_hcon_res hcon_res;

	#ifdef DEBUG_SOAK
		sprintf(buff, "HCON Req size %x\n", size );
		sysDebugWriteString(buff);
	#endif

	hcon_res.parcel_type = HARDWARE_CONFIGURATION_RESPONSE;
	hcon_res.length      = 2;
	hcon_res.status      = E_IDX_USE_DEFAULTS;

	rt = wIDX_SendParcel( bSlot, (UINT8*)&hcon_res, 5 );

	if( (rt != E_OK) && (rt != E_RETRY_OK) )
	{
		#ifdef DEBUG_SOAK
			sprintf(buff, "HCON RSP Failed %x\n", rt );
			sysDebugWriteString(buff);
		#endif
		return;
	}
}


/*****************************************************************************
 *  TITLE:  pTestExecResp()											[SM3-VME]*
 *****************************************************************************/
static void pTestExecResp( UINT8  bSlot,
						   UINT8* pbData,
						   UINT16 wMaxSize,
						   UINT16 size
						 )
{
//	UINT16  i;
	char    buff[ 64 ];
	char*   ERROR_TEXT = "FAIL";
	UINT16  test_status;

	#ifdef DEBUG_SOAK
		sprintf( buff, "EXECUTE_TEST_RESPONSE Slot %d, length %d\n", bSlot, size );
		sysDebugWriteString( buff );
	#endif

	/* Initialise the input buffer. */
	memset( &sReplyPacket, 0, sizeof(sReplyPacket) );

	sReplyPacket.hdr.wSourceID = bSlot;
	sReplyPacket.hdr.wDestID   = SOAK_ID;

	test_status = pbData[2];
	test_status = ((test_status << 8) & 0xff00) |  pbData[1];
	#ifdef DEBUG_SOAK
		sprintf( buff, "Test ID %d, Status 0x%x\n", pbData[0], test_status );
		sysDebugWriteString( buff );
	#endif

	if( (asSlots[bSlot].coop[COOP_LM_DA].testNum == pbData[0]) &&
		(asSlots[bSlot].coop[COOP_LM_DA].status  == 1)	)
	{
		if( (test_status != E__OK) &&
			(asSlots[bSlot].coop[COOP_LM_DA].retry < 3) )
		{
			asSlots[bSlot].coop[COOP_LM_DA].retry++;
			#ifdef DEBUG_SOAK
				sprintf( buff,  "COOP Test COOP_LM_DA FAILED Retry %d\n",
						 asSlots[bSlot].coop[COOP_LM_DA].retry );
				sysDebugWriteString( buff );
			#endif

			if(retryCoopTest(bSlot, COOP_LM_DA) == E__OK)
				return;
			#ifdef DEBUG_SOAK
				else
					sysDebugWriteString( "RETRY FAILED!\n" );
			#endif
		}
		else if( (test_status != E__OK) &&
				(asSlots[bSlot].coop[COOP_LM_DA].retry >= 3))
		{
			#ifdef DEBUG_SOAK
				sysDebugWriteString( "COOP Test COOP_LM_DA FAILED after retries\n" );
			#endif
		}
		else
		{
		#ifdef DEBUG_SOAK
				sysDebugWriteString( "COOP Test COOP_LM_DA PASSED\n" );
		#endif
	}

		asSlots[bSlot].coop[COOP_LM_DA].status = 0;
		asSlots[bSlot].coop[COOP_LM_DA].retry  = 0;
	}


	if( (asSlots[bSlot].coop[COOP_LM_WR].testNum == pbData[0]) &&
		(asSlots[bSlot].coop[COOP_LM_WR].status  == 1)	)
	{
		if( (test_status != E__OK) &&
			(asSlots[bSlot].coop[COOP_LM_WR].retry < 3) )
		{
			asSlots[bSlot].coop[COOP_LM_WR].retry++;
			#ifdef DEBUG_SOAK
				sprintf( buff,  "COOP Test COOP_LM_WR FAILED Retry %d\n",
						 asSlots[bSlot].coop[COOP_LM_WR].retry );
				sysDebugWriteString( buff );
			#endif

			if (retryCoopTest(bSlot, COOP_LM_WR) == E__OK)
				return;
			#ifdef DEBUG_SOAK
				else
					sysDebugWriteString( "RETRY FAILED!\n" );
			#endif
		}
		else if( (test_status != E__OK) &&
				(asSlots[bSlot].coop[COOP_LM_WR].retry >= 3))
		{
		#ifdef DEBUG_SOAK
				sysDebugWriteString( "COOP Test COOP_LM_WR FAILED after retries\n" );
		#endif
	}
		else
		{
	#ifdef DEBUG_SOAK
				sysDebugWriteString( "COOP Test COOP_LM_WR PASSED\n" );
	#endif
		}

		asSlots[bSlot].coop[COOP_LM_WR].status = 0;
		asSlots[bSlot].coop[COOP_LM_WR].retry  = 0;
	}



	if( test_status == E__OK )
	{
		sReplyPacket.hdr.wFlags  = 0x820;
		sReplyPacket.hdr.wLength = 10;
	}
	else
	{
		sReplyPacket.hdr.wFlags  = 0x8A4;
		sReplyPacket.wData[5]    = test_status;
		if( vgetErrorMsg(bSlot, buff) == 1 )
		{
			strcpy( (char *)&sReplyPacket.bData[12], buff );
			sReplyPacket.hdr.wLength = 13 + strlen (buff) + 1;
		}
		else
		{
			strcpy( (char *)&sReplyPacket.bData[12], ERROR_TEXT );
			sReplyPacket.hdr.wLength = 13 + strlen (ERROR_TEXT) + 1;
		}
	}

	wPacketAddQueue( Q_TX, &sReplyPacket );
}


/*****************************************************************************
 *  TITLE:  retryCoopTest ()										[SM3-VME]*
 *****************************************************************************/
static UINT32  retryCoopTest(UINT8 bSlot, UINT32 type)
{
	UINT32 rt = E__OK;
	char   buff[128];
	struc_test_ex_req test_ex_req;

	#ifdef DEBUG_SOAK
		sysDebugWriteString ("slave retryCoopTest()\n");
	#endif

	test_ex_req.parcel_type   = 0x12;
	test_ex_req.testing_level = 13;
	test_ex_req.erlevel       = 4;
	test_ex_req.error_action  = 0;
	test_ex_req.test_init     = 0;	/* don't run test "01H" - test initialisation */

	memset(asSlots[bSlot].errMsg, 0x00, 64 );

	if( type == COOP_LM_WR )
	{
		sprintf( buff, "%xh, %xh, 0h, 1h, 0h, 0h, 0h, 0h, 3h",
				 asSlots[bSlot].coop[COOP_LM_WR].offset,
				 asSlots[bSlot].coop[COOP_LM_WR].length );
		vSaveCommandTail( bSlot, (UINT8 *)buff );

		test_ex_req.test_id = asSlots[bSlot].coop[COOP_LM_WR].testNum;
	}


	if( type == COOP_LM_DA )
	{
		sprintf( buff, "%xh, %xh, 0h, 1h, 0h, 0h, 0h, 0h, 3h",
				 asSlots[bSlot].coop[COOP_LM_DA].offset,
				 asSlots[bSlot].coop[COOP_LM_DA].length );
		vSaveCommandTail( bSlot, (UINT8 *)buff );

		test_ex_req.test_id = asSlots[bSlot].coop[COOP_LM_DA].testNum;
	}

	// set BIST_REMOTE_PARCEL bit to indicate master that supports test primitives parcels
	interconnect_SetBit( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_REMOTE_PARCEL );

	rt = wIDX_SendParcel(bSlot, (UINT8*)&test_ex_req, 6);

	if( (rt != E__OK) && (rt != E__RETRY_OK) )
	{
		#ifdef DEBUG_SOAK
			sprintf (buff, "slave retryCoopTest() Parcel NOT accepted by slave %d, response %x\n", bSlot, rt);
			sysDebugWriteString (buff);
		#endif
		asSlots[bSlot].coop[COOP_LM_WR].status = 0;
		asSlots[bSlot].coop[COOP_LM_WR].retry  = 0;
		asSlots[bSlot].coop[COOP_LM_DA].status = 0;
		asSlots[bSlot].coop[COOP_LM_DA].retry  = 0;
	}

	return rt;
}



/*****************************************************************************
 *  TITLE:  processSlavePackets ()									[SM3-VME]*
 *****************************************************************************/
void processSlavePackets( UINT8 bSlot,
						  uPacket *psPkt
						 )
{
	/* check packet flags */
	if (psPkt->hdr.wFlags & FLAG_TST_ENT)
	   vSthExecute (bSlot, psPkt);
	else if (psPkt->hdr.wFlags & FLAG_CLK_SET)
	   vSthClock (psPkt);
	else
	{
		switch (psPkt->hdr.dType)
		{
			case id_RBL:				/* request bist list    */
				vSthHelp(bSlot, psPkt);
				break;

			case id_BGO:				/* execute bist 	    */
				vSthExecute(bSlot, psPkt);
				break;

			case id_SCT:				/* receive command tail	*/
				break;

			default:
				#ifdef DEBUG_SOAK
					sysDebugWriteString("slave Unknown packet\n");
				#endif
				break;
		}
	}
}



/*****************************************************************************
 *  TITLE:  vSthClock ()											[SM3-VME]*
 *****************************************************************************/
static void vSthClock( uPacket *sPkt )
{
    /* just ack the packet for now */
	#ifdef DEBUG_SOAK
		sysDebugWriteString ("Slave vSthClock()\n");
	#endif
}




/*****************************************************************************
 *  TITLE:  vSthHelp ()												[SM3-VME]*
 *****************************************************************************/
static UINT8 get_byte( UINT8 bSlot,
		               UINT8 *ptr
		              )
{
	UINT16 timer;
	UINT8  done, temp;
	UINT32 rt;
#ifdef DEBUG_SOAK
	char   buff[64];
#endif

	timer = done = 0;

	while ( (timer < 5000) && (!done) )			/* allow five seconds */
	{
		interconnect_ReadByte(bSlot, INTERCONNECT_BIST_SLAVE_STATUS, &temp );

		done = temp & BIST_OUT_DATA_VALID;	/* out data valid? */
		if (!done)
		{
			vDelay(1);		/* 1 millisecond delay */
			timer = timer + 1;
		}
	}

	if (!done)
	{
		#ifdef DEBUG_SOAK
			sysDebugWriteString("get_byte() timed out\n");
		#endif
		return 0;
	}

	rt = interconnect_ReceiveByte(bSlot, ptr);

	if ((rt != E__OK) && (rt != E__RETRY_OK))
	{
		#ifdef DEBUG_SOAK
			sprintf(buff, "get_byte() Failed, Slot %d, status = 0x%x\r\n", bSlot, rt);
			sysDebugWriteString(buff);
		#endif
		return 0;
	}

	return 1;
}


/*****************************************************************************
 *  TITLE:  vSthHelp()												[SM3-VME]*
 *****************************************************************************/
static void vSthHelp( UINT8 bSlot,
		              uPacket *psPkt
		            )
{
	#ifdef DEBUG_SOAK
		sysDebugWriteString( "slave vSthHelp()\n" );
	#endif

	help   helpsizes;
	UINT8  temp;
	UINT32 ii;
	UINT8  thcw_low;
	UINT8  thcw_high;
	UINT32 rt;
	UINT8  test_name[32];
	UINT8  test_number;
#ifdef DEBUG_SOAK
	UINT8  buff[128];
#endif

	uPacket*   psBistList;
	UINT8	   *test_entry;

	struc_test_help_req test_help_req;

	memset (&sReplyPacket, 0, sizeof(sReplyPacket));

	psBistList  = &sReplyPacket;
	test_entry  = (UINT8 *)&psBistList->hdr.dType;
	test_entry += sizeof (psBistList->hdr.dType);

	for (ii=0; ii < 10; ii++)
	{
		interconnect_ReadByte (bSlot, 2 + ii, &temp);
		*test_entry++ = temp;
	}

	test_help_req.first_test_id = 1;
	test_help_req.last_test_id  = 255;
	test_help_req.parcel_type   = 0x10;

	rt = wIDX_SendParcel (bSlot, (UINT8*)&test_help_req, 3);

	if (rt != E__OK && rt != E__RETRY_OK)
	{
		#ifdef DEBUG_SOAK
			sprintf (buff, "slave vSthHelp() Parcel NOT accepted by slave %d, response %x\n", bSlot, rt);
			sysDebugWriteString (buff);
		#endif
		return;
	}

	if( !get_byte(bSlot, &temp) )
		return;	/* read header byte */

	if (temp == 0xFF)
	{
		#ifdef DEBUG_SOAK
			sysDebugWriteString ("slave vSthHelp() Wrong Response Parcel 0xFF");
		#endif
		return;
	}

	if (temp != 0x11)
	{
		#ifdef DEBUG_SOAK
			sysDebugWriteString ("slave vSthHelp() Parcel type is not 0x11");
		#endif
		return;
	}

	if (!get_byte(bSlot, &helpsizes.help_data.help_low))
	{
		#ifdef DEBUG_SOAK
			sysDebugWriteString ("slave vSthHelp() Failed to get Low byte of help response");
		#endif
		return;
	}

	if (!get_byte(bSlot, &helpsizes.help_data.help_high))
	{
		#ifdef DEBUG_SOAK
			sysDebugWriteString ("slave vSthHelp() Failed to get Low byte of help response");
		#endif
	 	return;
	}

	#ifdef DEBUG_SOAK
		sprintf (buff, "slave vSthHelp() Help Size 0x%x\n", helpsizes.help_size );
		sysDebugWriteString (buff);
	#endif

	/*  The smallest help packet with tests in it will have at least four bytes.
		One for the single test number, two for it's THCW, and one for a NULL
		terminator on a zero-length test name. We'll get this when none of the
		tests requested exist.
	 */
	if (helpsizes.help_size < 4)
	{
		#ifdef DEBUG_SOAK
			sysDebugWriteString ("slave vSthHelp() Help Size less then 4");
		#endif
		return;
	}

	while (helpsizes.help_size > 0)
	{
		if (!get_byte(bSlot, &test_number))
			return;

		if (!get_byte(bSlot, &thcw_low))
			return;

		if (!get_byte(bSlot, &thcw_high))
			return;

		helpsizes.help_size -= 3;

		for (ii=0; ii < 32 ; ii++)
		{
			if (!get_byte(bSlot, &test_name[ii])) return;
			helpsizes.help_size -= 1;
			if ((helpsizes.help_size == 0) || (test_name[ii] == 0)) break;
		}
		test_name[ii] = 0;

		//skip test 127
		if(test_number == 127)
			continue;

		// Skip NON SOAK Test
		if( (thcw_low & 0x08) != 0x08)
			continue;

		if(test_number == COOP_LM_WR_ID)
		{
			asSlots[bSlot].coop[COOP_LM_WR].testNum = COOP_LM_WR_ID;
			asSlots[bSlot].coop[COOP_LM_WR].coopNum = ALT_COOP_LM_WR_ID;
		}

		if(test_number == COOP_LM_DA_ID)
		{
			asSlots[bSlot].coop[COOP_LM_DA].testNum = COOP_LM_DA_ID;
			asSlots[bSlot].coop[COOP_LM_DA].coopNum = ALT_COOP_LM_DA_ID;
		}

		*((UINT16*)test_entry) = (UINT16)test_number;
		test_entry +=2;
		*((UINT16*)test_entry) = 0x331E;
		test_entry +=2;

		memcpy(test_entry, test_name, strlen((char *)test_name));
		test_entry   += strlen((char *)test_name);
		*test_entry++ = '\0';
	}

	if(asSlots[bSlot].coop[COOP_LM_WR].testNum == COOP_LM_WR_ID)
	{
		*((UINT16*)test_entry) = ALT_COOP_LM_WR_ID;
		test_entry+=2;

		*((UINT16*)test_entry) = 0x331E;
		test_entry+=2;

		memcpy (test_entry, ALT_COOP_LM_WR_STR, strlen(ALT_COOP_LM_WR_STR));

		test_entry += strlen(ALT_COOP_LM_WR_STR);
		*test_entry++ = '\0';
	}

	if(asSlots[bSlot].coop[COOP_LM_DA].testNum == COOP_LM_DA_ID)
	{
		*((UINT16*)test_entry) = ALT_COOP_LM_DA_ID;
		test_entry+=2;

		*((UINT16*)test_entry) = 0x331E;
		test_entry+=2;

		memcpy (test_entry, ALT_COOP_LM_DA_STR, strlen(ALT_COOP_LM_DA_STR));

		test_entry += strlen(ALT_COOP_LM_DA_STR);
		*test_entry++ = '\0';
	}

	*test_entry = '\0';

	/* Send the bist list packet. */
	psBistList->hdr.wSourceID = bSlot;
	psBistList->hdr.wDestID   = SOAK_ID;
	psBistList->hdr.wFlags    = 0xa0;
	psBistList->hdr.wLength   = test_entry - (UINT8*)psBistList + 1;
	psBistList->hdr.dType     = (UINT32)id_SBL;

	wPacketAddQueue (Q_TX, psBistList);
}




/*****************************************************************************
 *  TITLE:     vSthExecute											[SM3-VME]*
 *
 *  ABSTRACT:  "vSthExecute" completes reading in the TEST EXECUTE REQUEST
 *             PARCEL, invokes test one if test_init flag is set to 0FFH,
 *             and then invokes the test via the Test Handler Service
 *             "CPCI_test_execute".  After a test has completed or the init
 *             test failed, vSthExecute sends a TEST EXECUTE RESPONSE PARCEL,
 *             and returns to "sth".
 *****************************************************************************/
static void vSthExecute( UINT8 bSlot,
		                 uPacket *psPkt
		                )
{
 	UINT32 rt, inprogress, i;
	char   buff[128];
	struc_test_ex_req test_ex_req;

	#ifdef DEBUG_SOAK
		sysDebugWriteString ("slave vSthExecute()\n");
	#endif

	if( (psPkt->sBgo.wTestId == 23) || (psPkt->sBgo.wTestId == 25) )
	{
		#ifdef DEBUG_SOAK
			sprintf (buff, "Co-operating test for slave %d, test %d\n", bSlot, psPkt->sBgo.wTestId);
			sysDebugWriteString (buff);
		#endif
	}

	test_ex_req.parcel_type   = 0x12;
	test_ex_req.test_id       = psPkt->sBgo.wTestId;
	test_ex_req.testing_level = 13;
	test_ex_req.erlevel       = 4;
	test_ex_req.error_action  = 0;
	test_ex_req.test_init     = 0;	/* don't run test "01H" - test initialisation */

	memset(asSlots[bSlot].errMsg, 0x00, 64 );
	rt = vSaveCommandTail( bSlot, &psPkt->bData[12] );

	if( (asSlots[bSlot].coop[COOP_LM_WR].coopNum == psPkt->sBgo.wTestId) &&
		(asSlots[bSlot].coop[COOP_LM_WR].offset  != NULL) )
	{
		if( rt == E__FAIL)
		{
			sprintf( buff, "%xh, %xh, 0h, 1h, 0h, 0h, 0h, 0h, 3h",
					 asSlots[bSlot].coop[COOP_LM_WR].offset,
					 asSlots[bSlot].coop[COOP_LM_WR].length );
			vSaveCommandTail( bSlot, (UINT8 *)buff );
		}

		inprogress = 0;
		for(i =10; i< MAX_SLAVES-1; i++)
		{
			if( (asSlots[i].coop[COOP_LM_WR].status == 1) ||
				(asSlots[i].coop[COOP_LM_DA].status == 1) )
			{
				inprogress = 1;
			}
		}
		if(inprogress == 0)
			configureCoopWindows(asSlots, syscon.syscon_coop);

		test_ex_req.test_id = asSlots[bSlot].coop[COOP_LM_WR].testNum;
		asSlots[bSlot].coop[COOP_LM_WR].status = 1;
		asSlots[bSlot].coop[COOP_LM_WR].retry  = 0;
	}


	if( (asSlots[bSlot].coop[COOP_LM_DA].coopNum == psPkt->sBgo.wTestId) &&
		(asSlots[bSlot].coop[COOP_LM_DA].offset  != NULL) )
	{
		if(rt == E__FAIL)
		{
			sprintf( buff, "%xh, %xh, 0h, 1h, 0h, 0h, 0h, 0h, 3h",
					 asSlots[bSlot].coop[COOP_LM_DA].offset,
					 asSlots[bSlot].coop[COOP_LM_DA].length );
			vSaveCommandTail( bSlot, (UINT8 *)buff );
		}

		inprogress = 0;
		for(i =10; i< MAX_SLAVES-1; i++)
		{
			if( (asSlots[i].coop[COOP_LM_WR].status == 1) ||
				(asSlots[i].coop[COOP_LM_DA].status == 1) )
			{
				inprogress = 1;
			}
		}
		if(inprogress == 0)
			configureCoopWindows(asSlots, syscon.syscon_coop);

		test_ex_req.test_id = asSlots[bSlot].coop[COOP_LM_DA].testNum;
		asSlots[bSlot].coop[COOP_LM_DA].status = 1;
		asSlots[bSlot].coop[COOP_LM_DA].retry  = 0;
	}

	if(test_ex_req.test_id == 69)
	{
		// clear BIST_REMOTE_PARCEL bit to indicate master that supports test primitives parcels
		rt = interconnect_ClearBit( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_REMOTE_PARCEL );
	}
	else
	{
	// set BIST_REMOTE_PARCEL bit to indicate master that supports test primitives parcels
	rt = interconnect_SetBit( bSlot, INTERCONNECT_BIST_MASTER_STATUS, BIST_REMOTE_PARCEL );
	}

	if( (rt != E__OK) && (rt != E__RETRY_OK) )
	{
		#ifdef DEBUG_SOAK
			sprintf (buff, "slave vSthExecute() Could not set BIST_REMOTE_PARCEL bit for slave %d, response %x\n", bSlot, rt);
			sysDebugWriteString (buff);
		#endif
		return;
	}

	rt = wIDX_SendParcel(bSlot, (UINT8*)&test_ex_req, 6);

	if( (rt != E__OK) && (rt != E__RETRY_OK) )
	{
		#ifdef DEBUG_SOAK
			sprintf (buff, "slave vSthExecute() Parcel NOT accepted by slave %d, response %x\n", bSlot, rt);
			sysDebugWriteString (buff);
		#endif
		return;
	}
}




/*****************************************************************************
 *  TITLE:  postErrorMsg()											[SM3-VME]*
 *****************************************************************************/
static void vpostErrorMsg( UINT8 bSlot,
		                   char* buffer
		                  )
{
	memset( asSlots[bSlot].errMsg, 0x00, 64 );
	strncpy( (char *)asSlots[bSlot].errMsg, buffer, 63 );
	asSlots[bSlot].errMsg[63] = '\0';
	asSlots[bSlot].msgFlag = 1;
}



/*****************************************************************************
 *  TITLE:  getErrorMsg()											[SM3-VME]*
 *****************************************************************************/
static UINT32 vgetErrorMsg( UINT8 bSlot,
		                    char* buffer
		                   )
{
	if( asSlots[bSlot].msgFlag == 1 )
	{
		asSlots[bSlot].msgFlag = 0;
		strncpy( buffer, (char *)asSlots[bSlot].errMsg, 64 );
		memset( asSlots[bSlot].errMsg, 0x00, 64 );
		return 1;
	}
	else
		return 0;
}
