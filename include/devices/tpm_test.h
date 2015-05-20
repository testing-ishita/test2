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
/*
 *  Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/tpm_test.h,v 1.3 2014-03-03 16:31:52 mgostling Exp $
*
* $Log: tpm_test.h,v $
* Revision 1.3  2014-03-03 16:31:52  mgostling
* Add suppport for AtmelTPM
*
* Revision 1.2  2013-11-25 11:03:10  mgostling
* Add TPM error codes.
*
* Revision 1.1  2013-09-04 07:40:28  chippisley
* Import files into new source repository.
*
* Revision 1.1  2012/02/17 11:32:17  hchalla
* Added new TPM interface test.
*
*
*/
#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>


#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)
#define osReadDWord(regAddr)		 (*(UINT32*)(regAddr))

/* Infineon specific definitions */
/* maximum number of WTX-packages */
#define TPM_LT_MEM_SPACE0 (0xFED40000L)
#define TPM_MAX_WTX_PACKAGES    10
/* vDelay-Time for WTX-packages */
#define TPM_WTX_MSLEEP_TIME     20
/* vDelay-Time --> Interval to check status register */
#define TPM_MSLEEP_TIME         3
/* gives number of max. vDelay()-calls before throwing timeout */
#define TPM_MAX_TRIES           5000
#define TPM_INFINEON_VENDOR_ID	0x15D1
#define TPM_ATMEL_VENDOR_ID		0x3204
#define TPM_ATMEL_DEVICE_ID		0x1114
#define TPM_ATMEL_VEN_DEV_ID	0x32041114L

#define TPM_INF_IO_PORT         0x0
#define TPM_INF_IO_MEM          0x1

#define TPM_INF_ADDR            0x0
#define TPM_INF_DATA            0x1

#define TPM_ADDR        0x4E

// Error codes returned by TPM

#define TPM_SUCCESS					 0 // 0x00000000 Successful completion of the operation
#define TPM_AUTHFAIL				 1 // 0x00000001 Authentication failed
#define TPM_BADINDEX				 2 // 0x00000002 The index to a PCR, DIR or other register is incorrect
#define TPM_BAD_PARAMETER			 3 // 0x00000003 One or more parameter is bad
#define TPM_AUDITFAILURE			 4 // 0x00000004 An operation completed successfully but the auditing of that operation failed.
#define TPM_CLEAR_DISABLED			 5 // 0x00000005 The clear disable flag is set and all clear operations now require physical access
#define TPM_DEACTIVATED				 6 // 0x00000006 The TPM is deactivated
#define TPM_DISABLED				 7 // 0x00000007 The TPM is disabled
#define TPM_DISABLED_CMD			 8 // 0x00000008 The target command has been disabled
#define TPM_FAIL					 9 // 0x00000009 The operation failed
#define TPM_BAD_ORDINAL				10 // 0x0000000A The ordinal was unknown or inconsistent
#define TPM_INSTALL_DISABLED		11 // 0x0000000B The ability to install an owner is disabled
#define TPM_INVALID_KEYHANDLE		12 // 0x0000000C The key handle can not be interpreted
#define TPM_KEYNOTFOUND				13 // 0x0000000D The key handle points to an invalid key
#define TPM_INAPPROPRIATE_ENC		14 // 0x0000000E Unacceptable encryption scheme
#define TPM_MIGRATEFAIL				15 // 0x0000000F Migration authorization failed
#define TPM_INVALID_PCR_INFO		16 // 0x00000010 PCR information could not be interpreted
#define TPM_NOSPACE					17 // 0x00000011 No room to load key.
#define TPM_NOSRK					18 // 0x00000012 There is no SRK set
#define TPM_NOTSEALED_BLOB			19 // 0x00000013 An encrypted blob is invalid or was not created by this TPM
#define TPM_OWNER_SET				20 // 0x00000014 There is already an Owner
#define TPM_RESOURCES				21 // 0x00000015 The TPM has insufficient internal resources to perform the requested action.
#define TPM_SHORTRANDOM				22 // 0x00000016 A random string was too short
#define TPM_SIZE					23 // 0x00000017 The TPM does not have the space to perform the operation.
#define TPM_WRONGPCRVAL				24 // 0x00000018 The named PCR value does not match the current PCR value.
#define TPM_BAD_PARAM_SIZE			25 // 0x00000019 The paramSize argument to the command has the incorrect value
#define TPM_SHA_THREAD				26 // 0x0000001A There is no existing SHA-1 thread.
#define TPM_SHA_ERROR				27 // 0x0000001B The calculation is unable to proceed because the existing SHA-1 thread has already encountered an error.
#define TPM_FAILEDSELFTEST			28 // 0x0000001C Self-test has failed and the TPM has shutdown.
#define TPM_AUTH2FAIL				29 // 0x0000001D The authorization for the second key in a 2 key function failed authorization
#define TPM_BADTAG					30 // 0x0000001E The tag value sent to for a command is invalid
#define TPM_IOERROR					31 // 0x0000001F An IO error occurred transmitting information to the TPM
#define TPM_ENCRYPT_ERROR			32 // 0x00000020 The encryption process had a problem.
#define TPM_DECRYPT_ERROR			33 // 0x00000021 The decryption process did not complete.
#define TPM_INVALID_AUTHHANDLE		34 // 0x00000022 An invalid handle was used.
#define TPM_NO_ENDORSEMENT			35 // 0x00000023 The TPM does not a EK installed
#define TPM_INVALID_KEYUSAGE		36 // 0x00000024 The usage of a key is not allowed
#define TPM_WRONG_ENTITYTYPE		37 // 0x00000025 The submitted entity type is not allowed
#define TPM_INVALID_POSTINIT		38 // 0x00000026 The command was received in the wrong sequence relative to TPM_Init and a subsequent TPM_Startup
#define TPM_INAPPROPRIATE_SIG		39 // 0x00000027 Signed data cannot include additional DER information
#define TPM_BAD_KEY_PROPERTY		40 // 0x00000028 The key properties in TPM_KEY_PARMs are not supported by this TPM
#define TPM_BAD_MIGRATION			41 // 0x00000029 The migration properties of this key are incorrect.
#define TPM_BAD_SCHEME				42 // 0x0000002A The signature or encryption scheme for this key is incorrect or not permitted in this situation.
#define TPM_BAD_DATASIZE			43 // 0x0000002B The size of the data (or blob) parameter is bad or inconsistent with the referenced key
#define TPM_BAD_MODE				44 // 0x0000002C A mode parameter is bad, such as capArea or subCapArea for TPM_GetCapability, physicalPresence parameter for TPM_PhysicalPresence, or migrationType for TPM_CreateMigrationBlob.
#define TPM_BAD_PRESENCE			45 // 0x0000002D Either the physicalPresence or physicalPresenceLock bits have the wrong value
#define TPM_BAD_VERSION				46 // 0x0000002E The TPM cannot perform this version of the capability
#define TPM_NO_WRAP_TRANSPORT		47 // 0x0000002F The TPM does not allow for wrapped transport sessions
#define TPM_AUDITFAIL_UNSUCCESSFUL	48 // 0x00000030 TPM audit construction failed and the underlying command was returning a failure code also
#define TPM_AUDITFAIL_SUCCESSFUL	49 // 0x00000031 TPM audit construction failed and the underlying command was returning success
#define TPM_NOTRESETABLE			50 // 0x00000032 Attempt to reset a PCR register that does not have the resettable attribute
#define TPM_NOTLOCAL				51 // 0x00000033 Attempt to reset a PCR register that requires locality and locality modifier not part of command transport
#define TPM_BAD_TYPE				52 // 0x00000034 Make identity blob not properly typed
#define TPM_INVALID_RESOURCE		53 // 0x00000035 When saving context identified resource type does not match actual resource
#define TPM_NOTFIPS					54 // 0x00000036 The TPM is attempting to execute a command only available when in FIPS mode
#define TPM_INVALID_FAMILY			55 // 0x00000037 The command is attempting to use an invalid family ID
#define TPM_NO_NV_PERMISSION		56 // 0x00000038 The permission to manipulate the NV storage is not available
#define TPM_REQUIRES_SIGN			57 // 0x00000039 The operation requires a signed command
#define TPM_KEY_NOTSUPPORTED		58 // 0x0000003A Wrong operation to load an NV key
#define TPM_AUTH_CONFLICT			59 // 0x0000003B NV_LoadKey blob requires both owner and blob authorization
#define TPM_AREA_LOCKED				60 // 0x0000003C The NV area is locked and not writable
#define TPM_BAD_LOCALITY			61 // 0x0000003D The locality is incorrect for the attempted operation
#define TPM_READ_ONLY				62 // 0x0000003E The NV area is read only and can’t be written to
#define TPM_PER_NOWRITE				63 // 0x0000003F There is no protection on the write to the NV area
#define TPM_FAMILYCOUNT 			64 // 0x00000040 The family count value does not match
#define TPM_WRITE_LOCKED 			65 // 0x00000041 The NV area has already been written to
#define TPM_BAD_ATTRIBUTES 			66 // 0x00000042 The NV area attributes conflict
#define TPM_INVALID_STRUCTURE 		67 // 0x00000043 The structure tag and version are invalid or inconsistent
#define TPM_KEY_OWNER_CONTROL 		68 // 0x00000044 The key is under control of the TPM Owner and can only be evicted by the TPM Owner.
#define TPM_BAD_COUNTER 			69 // 0x00000045 The counter handle is incorrect
#define TPM_NOT_FULLWRITE 			70 // 0x00000046 The write is not a complete write of the area
#define TPM_CONTEXT_GAP 			71 // 0x00000047 The gap between saved context counts is too large
#define TPM_MAXNVWRITES 			72 // 0x00000048 The maximum number of NV writes without an owner has been exceeded
#define TPM_NOOPERATOR 				73 // 0x00000049 No operator AuthData value is set
#define TPM_RESOURCEMISSING 		74 // 0x0000004A The resource pointed to by context is not loaded
#define TPM_DELEGATE_LOCK 			75 // 0x0000004B The delegate administration is locked
#define TPM_DELEGATE_FAMILY 		76 // 0x0000004C Attempt to manage a family other then the delegated family
#define TPM_DELEGATE_ADMIN 			77 // 0x0000004D Delegation table management not enabled
#define TPM_TRANSPORT_NOTEXCLUSIVE 	78 // 0x0000004E There was a command executed outside of an exclusive transport session
#define TPM_OWNER_CONTROL 			79 // 0x0000004F Attempt to context save a owner evict controlled key
#define TPM_DAA_RESOURCES 			80 // 0x00000050 The DAA command has no resources available to execute the command
#define TPM_DAA_INPUT_DATA0 		81 // 0x00000051 The consistency check on DAA parameter inputData0 has failed.
#define TPM_DAA_INPUT_DATA1 		82 // 0x00000052 The consistency check on DAA parameter inputData1 has failed.
#define TPM_DAA_ISSUER_SETTINGS 	83 // 0x00000053 The consistency check on DAA_issuerSettings has failed.
#define TPM_DAA_TPM_SETTINGS 		84 // 0x00000054 The consistency check on DAA_tpmSpecific has failed.
#define TPM_DAA_STAGE 				85 // 0x00000055 The atomic process indicated by the submitted DAA command is not the expected process.
#define TPM_DAA_ISSUER_VALIDITY 	86 // 0x00000056 The issuer’s validity check has detected an inconsistency
#define TPM_DAA_WRONG_W 			87 // 0x00000057 The consistency check on w has failed.
#define TPM_BAD_HANDLE 				88 // 0x00000058 The handle is incorrect
#define TPM_BAD_DELEGATE 			89 // 0x00000059 Delegation is not correct
#define TPM_BADCONTEXT 				90 // 0x0000005A The context blob is invalid
#define TPM_TOOMANYCONTEXTS 		91 // 0x0000005B Too many contexts held by the TPM
#define TPM_MA_TICKET_SIGNATURE 	92 // 0x0000005C Migration authority signature validation failure
#define TPM_MA_DESTINATION 			93 // 0x0000005D Migration destination not authenticated
#define TPM_MA_SOURCE 				94 // 0x0000005E Migration source incorrect
#define TPM_MA_AUTHORITY 			95 // 0x0000005F Incorrect migration authority

#define TPM_PERMANENTEK 			97 // 0x00000061 Attempt to revoke the EK and the EK is not revocable
#define TPM_BAD_SIGNATURE 			98 // 0x00000062 Bad signature of CMK ticket
#define TPM_NOCONTEXTSPACE 			99 // 0x00000063 There is no room in the context list for additional contexts

#define TPM_RETRY				  2048 // 0x00000800 The TPM is too busy to respond to the command immediately, but the command could be resubmitted at a later time.The TPM MAY return TPM_RETRY for any command at any time.
#define TPM_NEEDS_SELFTEST 		  2049 // 0x00000801 TPM_ContinueSelfTest has not been run.
#define TPM_DOING_SELFTEST 		  2050 // 0x00000802 The TPM is currently executing the actions of TPM_ContinueSelfTest because the ordinal required resources that have not been tested.
#define TPM_DEFEND_LOCK_RUNNING   2051 // 0x00000803 The TPM is defending against dictionary attacks and is in some time-out period.

// CUTE error codes

#define E_TPM_IO_PORT                                     (E__BIT  + 0x10)
#define E_TPM_NO_DEVICE                                 (E__BIT  + 0x20)
#define E__WRONG_RSP_CMD                           (E__BIT  + 0x30)
#define E__TPM_SELFTEST_FAIL                        (E__BIT  + 0x40)
#define E__TPM_NEEDS_SELF_TEST                  (E__BIT + 0x50)
#define E__TPM_STARTUP_FAIL                          (E__BIT + 0x60)

struct tpm_inf_dev {

        UINT8  iotype;
        UINT32 dIndexOff; /* index register offset */

        UINT32 dDataRegs; /* Data registers */
        UINT32 dDataSize;

        UINT32 dConfigPort;       /* IO Port config index reg */
        UINT32 dConfigSize;
};



/* TPM header definitions */
enum infineon_tpm_header {
        TPM_VL_VER = 0x01,
        TPM_VL_CHANNEL_CONTROL = 0x07,
        TPM_VL_CHANNEL_PERSONALISATION = 0x0A,
        TPM_VL_CHANNEL_TPM = 0x0B,
        TPM_VL_CONTROL = 0x00,
        TPM_INF_NAK = 0x15,
        TPM_CTRL_WTX = 0x10,
        TPM_CTRL_WTX_ABORT = 0x18,
        TPM_CTRL_WTX_ABORT_ACK = 0x18,
        TPM_CTRL_ERROR = 0x20,
        TPM_CTRL_CHAININGACK = 0x40,
        TPM_CTRL_CHAINING = 0x80,
        TPM_CTRL_DATA = 0x04,
        TPM_CTRL_DATA_CHA = 0x84,
        TPM_CTRL_DATA_CHA_ACK = 0xC4
};

enum infineon_tpm_register {
        WRFIFO = 0x00,
        RDFIFO = 0x01,
        STAT = 0x02,
        CMD = 0x03
};

enum infineon_tpm_command_bits {
        CMD_DIS = 0x00,
        CMD_LP = 0x01,
        CMD_RES = 0x02,
        CMD_IRQC = 0x06
};


enum infineon_tpm_status_bits {
        STAT_XFE = 0x00,
        STAT_LPA = 0x01,
        STAT_FOK = 0x02,
        STAT_TOK = 0x03,
        STAT_IRQA = 0x06,
        STAT_RDA = 0x07
};


/* some outgoing values */
enum infineon_tpm_values {
        CHIP_ID1 = 0x20,
        CHIP_ID2 = 0x21,
        TPM_DAR = 0x30,
        RESET_LP_IRQC_DISABLE = 0x41,
        ENABLE_REGISTER_PAIR = 0x55,
        IOLIMH = 0x60,
        IOLIML = 0x61,
        DISABLE_REGISTER_PAIR = 0xAA,
        IDVENL = 0xF1,
        IDVENH = 0xF2,
        IDPDL = 0xF3,
        IDPDH = 0xF4
};

