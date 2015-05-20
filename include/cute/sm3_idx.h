/************************************************************************
 *                                                                      *
 *      Copyright 2011 Concurrent Technologies, all rights reserved.    *
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
 * sm3_idx.h
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/cute/sm3_idx.h,v 1.2 2013-11-25 11:00:20 mgostling Exp $
 * $Log: sm3_idx.h,v $
 * Revision 1.2  2013-11-25 11:00:20  mgostling
 * Updates for Cygwin 1.7.25 GCC 4.7.3-1, fixed various compiler warnings.
 * Added missing CVS headers.
 *
*/

#ifndef SM3_IDX_H_
#define SM3_IDX_H_

/***************************************************************************
    MISC. STATUS LITERALS
***************************************************************************/

#define E_OK                                0x0000
#define E_COMMUNICATION_ERROR               0x8005
#define E_RECORD_NOT_FOUND                  0x8006
#ifndef E_NOT_PENDING
#define E_NOT_PENDING                       0x8007
#endif
#define E_RETRY_OK                          0x800A
#ifndef E_LENGTH_OVERRUN
#define E_LENGTH_OVERRUN                    0x800B
#endif
#define E_RECORD_LENGTH                     0x800C
#define E_INVALID_SLOT                      0x8020

/***************************************************************************
    IDX STATUS LITERALS
***************************************************************************/

#define E_IDX_TEST_PASS                         0x0000
#define E_IDX_TIME                              0x0001
#define E_IDX_ACTION_QUIT                       0x0003
#define E_IDX_LIMIT                             0x0004
#define E_IDX_BAD_HEADER                        0x0062
#define E_IDX_CHECKSUM                          0x0064
#define E_IDX_EOF                               0x0065
#define E_IDX_NO_MEM                            0x0068
#define E_IDX_REC_FORMAT                        0x0069
#define E_IDX_REC_LENGTH                        0x006A
#define E_IDX_LOADER_SUPPORT                    0x006F
#define E_IDX_MISCOMPARE                        0x0071
#define E_IDX_USER_ABORT                        0x0302
#define E_IDX_USE_DEFAULTS                      0x0307
#define E_IDX_SYNTAX                            0x0308
#define E_IDX_BAD_INPUT                         0x030A
#define E_IDX_PROMPT_ME                     	0x030B
#define E_IDX_NO_INPUT                          0x030C
#define E_IDX_TEST_NOT_FOUND                    0x031F
#define E_IDX_GDT_FULL                          0x0326
#define E_IDX_SELECTOR_OWNER                    0x0327
#define E_IDX_PERMISSION_DENIED                 0x0351
#define E_IDX_LENGTH_OVERRUN                    0x0352
#define E_IDX_CPU_TYPE                          0x0360
#define E_IDX_OBJECT_MODULE_FORMAT              0x0361
#define E_IDX_TRANSFER_MODE                     0x0362
#define E_IDX_TEST_PRIMITIVES                   0x0363
#define E_IDX_TEST_LIMIT                        0x0364
#define E_IDX_PARAM                             0x8004
#define E_IDX_TEST_FAIL                         0x8085


/***************************************************************************
    MASTER TEST HANDLER STATUS LITERALS
***************************************************************************/

#define E_MTH_NO_AGENT                          0x8080
#define E_MTH_WRONG_PARCEL_TYPE                 0x8082
#define E_MTH_NON_IDX                           0x8083
#define E_MTH_IDX_RETURN                        0x8086
#define E_MTH_SKIP_INT                          0x8087
#define E_MTH_NO_BISTSUP                        0x8088
#define E_MTH_EVEN_VENDOR						0x8089


typedef struct
{
	UINT8 help_low;
 	UINT8 help_high;
}struc_len;

typedef union
{
 	UINT16	   help_size;
 	struc_len help_data;
}help;

typedef struct
{
	UINT8 parcel_type;
	UINT8 first_test_id;
	UINT8 last_test_id;
}struc_test_help_req;

typedef struct
{
	UINT8  parcel_type;
	UINT16 len;
	UINT8 test_id;
	UINT16 th_cntrl_word;
	UINT8 ascii_name[31];
	UINT8 delimiter;
}struc_test_help_res;

typedef struct
{
	UINT8 parcel_type;
	UINT8 test_id;
	UINT8 testing_level;
	UINT8 erlevel;
	UINT8 error_action;
	UINT8 test_init;
}struc_test_ex_req;

typedef struct
{
	UINT8  parcel_type;
	UINT8  test_id;
	UINT16 status;
}struc_test_ex_res;

#pragma pack(push, 1)
typedef struct
{
	UINT8  parcel_type;
	UINT16 length;
	UINT16 status;
}struc_hcon_res;

#pragma pack(pop)

#endif /* SM3_IDX_H_ */
