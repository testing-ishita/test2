/*
 * board_info.h
 *
 *  Created on: 31 Mar 2011
 *      Author: Haroon
 */

#ifndef BOARD_INFO_H_
#define BOARD_INFO_H_


#define VP717_MEMORY_TOP 					0xFFFFFFFF
#define VP717_BIOS_START					0x200000
#define VP717_CUTE_START 					0x01

#define MEMORY_TOP 							VP717_MEMORY_TOP
#define BIOS_START 							VP717_BIOS_START
#define CUTE_START 							VP717_CUTE_START
#define CUTE_SIZE 							0x80000

#define SIG_1ST_STAGE						(0x31544942UL)	/* "BIT1", 'B' in LSB */
#define CUTE_1ST_STAGE						(0x45545543UL)	/* "CUTE", 'C' in LSB */
#define SIG_2ND_STAGE						(0x32544942UL)

#define SIGNATURE_OFFSET					4
#define STAGE_SIZE_OFFSET					12
#define FIELD_START							40
#define MIN_FIELD							12
#define TYPE_OFFSET							0
#define LENGTH_OFFSET	 					4
#define FIELD_HEAD_COUNT					8
#define FIELD_COUNT							24
#define FIELD_LENGTH	 					28
#define FIELD_CHECKSUM						32
#define HEADER_CHECKSUM						36

#define VX813_THRESHOLDS_SIZE 				8
#define VX813_TOTAL_THRESHOLDS_SIZE 		16
#define THRESHOLDS_SIZE 					VX813_THRESHOLDS_SIZE
#define TOTAL_THRESHOLDS_SIZE 				VX813_TOTAL_THRESHOLDS_SIZE


#define EFIELD_TEMPERATURE_GRADE_STRING 	0x00000001
#define EFIELD_TEMPERATURE_GRADE_DATA 		0x00000002
#define EFIELD_FORCE_DEBUG_ON				0x00000004
#define EFIELD_FORCE_DEBUG_OFF				0x00000008
#define EFIELD_OVERRIDE_STH_TEST_LIST		0x00000010
#define EFIELD_BOARD_NAME					0x0000000C
#define EFIELD_LAST_FIELD					0x80000000


#define EXT_INFO_NOT_PRSENT					0x00000001
#define EXT_INFO_BUFFER_TOO_SMALL			0x00000002
#define EXT_FIELD_NOT_PRSENT				0x00000003

#define BIOS_INFO_NOT_PRSENT				0x00000001
#define BIOS_INFO_BUFFER_TOO_SMALL			0x00000002
#define BIOS_FIELD_NOT_PRSENT				0x00000003

#define BYTE_SWAP( data )					(((data & 0x000000FFUL) << 24 ) |\
											 ((data & 0x0000FF00UL) << 8  ) |\
											 ((data & 0x00FF0000UL) >> 8  ) |\
											 ((data & 0xFF000000UL) >> 24 ));

typedef struct tagImageData
{
	UINT32	dImageSize;
	UINT32	dSize1;
	UINT32	dOffset2;
	UINT32	dSize2;

} IMAGE_DATA;



/*
 * - Variant specific information for example temperature grade, board type for example
 *   conduction cooled, and optional feature information like optional hardware interfaces
 *   that are optional but not reported by BIOS, will be passed to CUTE using a CUTE specific
 *   information structure appended to CUTE binary image in ROM. This will allow the same CUTE
 *   binary to be used for different variants of a board.
 *
 * - Optional hardware interfaces could include missing XMC/PMC sites, inaccessible interfaces due
 *   to conduction cooled heat-sinks, lack of front panel interfaces, or any other missing or additional
 *   features etc etc.
 *
*  - The structure has a 40 byte standard header and is defined below.
 * - Information following the header is organised in variable length fields
 *   each field is defined as follows
 *
 * typedef struct info_field
 * {
 * 		UINT32 type;
 * 		UINT32 length;
 * 		UINT8  buff[n];  //n = length
 * } infofield;
 *
 * After initialisation is complete, a board specific service will be called from main()
 * before doing anything else, to parse this structure, the function will locate the structure
 * in the CUTE ROM and parse it.
 *
 * This mechanism can be used to pass information to CUTE that is otherwise unavailable  by other
 * means. The information passed is/can be board specific, therefore it is parsed by a board specific
 * parser and obtained by rest of the CUTE using services.
 *
 * The information will be contained in a file formatted in a similar fashion to IPMI FRU.
 * It will be parsed by a derivative of bit_tool utility, which will calculate checksums on this
 * information and then append this information at the end of the cute image. Then the cute image
 * can be programmed into the ROM using uniprog/fpt etc.
 *
 * There will be a batch file called config.bat that will invoke this process. Multiple information files
 * can be present in a single CUTE release. CUTE can therefore be reconfigurable for different variants
 * of a board during programming process. All that will be required is to specify the correct board variant.
 *
 * CUTE.bat will be used to program the image, as is done currently
 *
 * */

typedef struct extended_info_struct
{
	char   signature[16];	// - This filed will contain the string "CUTE_CONFIGINFO\0"
	UINT32 Struct_ver;		// - 8bits Major, 8 bits minor, indicates structure version,
							//   this can be used to differentiate between different future versions
	UINT32 Info_ver;		// - 8Bits RFU, 8bits 'V'/'X', 8bits Major, 8 bits minor, indicates FRU file version
	UINT32 field_count;		// - total number of information fields following the header
	UINT32 length;			// - total length of information following the header
	UINT32 chksum;			// - checksum of the information following the header
	UINT32 header_chksum;	// - header's own checksum
	UINT8  present;			// - flag indicating if the information is present and correct
	UINT8  *data;			// - pointer to a buffer containing all the field data

} EXTENDED_INFO;



/*
 * - Information is passed by the BIOS to CUTE in a memory buffer
 * - The buffer has a 40 byte standard header and is defined below
 * - Information following the header is organised in variable length fields
 *   each field is defined as follows
 *
 * typedef struct info_field
 * {
 * 		UINT32 length;
 * 		UINT32 type;
 * 		UINT8  buff[n];  //n = length
 * } infofield;
 *
 * BIOS will pass a pointer to this buffer to CUTE in ESI, EDI will still contain
 * the standard startup flags, a new flag will be defined in EDI to indicate if  BSP has passed
 * the pointer to this structure in ESI, if this flag is not set then this would indicate that this feature
 * is not implemented. At CUTE start this pointer will be saved. After
 * initialisation is complete, a board specific function will be called from main(),
 * before doing anything else, to parse this buffer. Information from this buffer
 * can then be obtained by other sections of CUTE using board specific services.
 *
 * This mechanism can be used to pass information to CUTE that is otherwise unavailable
 * by other means. For example in case of TR501, VPX switch configuration information is
 * not available, therefore CUTE cannot determine if the switch is in correct configuration
 * as expected by BIOS. This makes testing the switch configuration virtually impossible.
 * By using this mechanism such information can be passed to CUTE. The information passed
 * is board specific, therefore it is parsed by a board specific parser and obtained by
 * rest of the CUTE using services.
 *
 * */

typedef struct bios_info_struct
{
	char   signature[16];	// - This filed will contain the string "CUTE_BIOS_INFO_\0"
	UINT32 Struct_ver;		// - 8bits Major, 8 bits minor, indicates structure version,
							//   this can be used to differentiate between different future versions
	UINT32 BIOS_ver;		// - BIOS version, 8Bits RFU, 8bits 'V'/'X', 8bits Major, 8 bits minor
	UINT32 field_count;		// - Total number of information fields following the header
	UINT32 length;			// - Total length of information following the header
	UINT32 chksum;			// - Checksum of the information following the header
	UINT32 header_chksum;	// - Header's own checksum
	UINT8  present;			// - flag indicating if the information is present and correct
	UINT8  *data;			// - Pointer to a buffer containing all the field data

} BIOS_INFO;


#endif /* BOARD_INFO_H_ */
