/*
 * error_msg.c
 *
 *  Created on: 18 Jan 2011
 *      Author: engineer
 */

#include <stdtypes.h>
#include <errors.h>
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error_msg.h>

static char errorBuffer[64];
static char msgFlag = 0;

extern UINT8 sysGetTestHandler (void);


/* - VME soak master seems to be able to accept strings greater then 128 bytes
 * - CPCI soak master can handle strings of maximum length 63 bytes only
 * */

void postErrorMsg(char* buffer)
{
	UINT8 mode;
	mode = sysGetTestHandler();

	if( (mode == TH__MTH) || (mode == TH__LBIT) )
		return;
		
	memset(errorBuffer, 0x00, 64);
	strncpy(errorBuffer, buffer, 63);
	errorBuffer[63] = '\0';
	msgFlag = 1;
}

int getErrorMsg(char* buffer)
{
	if(msgFlag == 1)
	{
		msgFlag = 0;
		strncpy(buffer, errorBuffer, 64);
		memset(errorBuffer, 0x00, 64);
		return 1;
	}
	else
		return 0;
}
