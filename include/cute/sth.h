/*
 * sth.h
 *
 *  Created on: 29 Apr 2010
 *      Author: engineer
 */

#ifndef STH_H_
#define STH_H_

#define TRUE 1
#define FALSE 0

typedef struct
{
	UINT8	first_test_id;
	UINT8	last_test_id;
} help_request_struct;

typedef struct
{
	UINT8	test_id;
	UINT8	testing_level;
	UINT8	error_reporting_level;
	UINT8	error_action;
	UINT8 	test_init;
} execute_request_struct;

typedef struct
{
	UINT8	parcel_type;
	UINT8	test_id;
	UINT16	test_status;
} execute_response_struct;


UINT16 vpxSthInit(void);
UINT32 vpxSthLoop(void);

UINT16 cpciSthInit(void);
UINT32 cpciSthLoop(void);

UINT8  vmeSthInit(void);
UINT32 vmeSthLoop(void);

#endif /* STH_H_ */
