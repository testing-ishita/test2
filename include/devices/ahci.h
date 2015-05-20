/************************************************************************
 *                                                                      *
 *      Copyright 2013 Concurrent Technologies, all rights reserved.    *
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
 * ahci.h
 */
 
/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/ahci.h,v 1.1 2013-11-25 14:27:22 mgostling Exp $
 * $Log: ahci.h,v $
 * Revision 1.1  2013-11-25 14:27:22  mgostling
 * New AHCI tests.
 *
 *
 *  Created on: 6 Aug 2013
 *      Author: rupadhyay
 */

#ifndef AHCI_H_
#define AHCI_H_

#define BIT0   (1 << 0 )
#define BIT1   (1 << 1 )
#define BIT2   (1 << 2 )
#define BIT3   (1 << 3 )
#define BIT4   (1 << 4 )
#define BIT5   (1 << 5 )
#define BIT6   (1 << 6 )
#define BIT7   (1 << 7 )
#define BIT8   (1 << 8 )
#define BIT9   (1 << 9 )
#define BIT10  (1 << 10)
#define BIT11  (1 << 11)
#define BIT12  (1 << 12)
#define BIT13  (1 << 13)
#define BIT14  (1 << 14)
#define BIT15  (1 << 15)
#define BIT16  (1 << 16)
#define BIT17  (1 << 17)
#define BIT18  (1 << 18)
#define BIT19  (1 << 19)
#define BIT20  (1 << 20)
#define BIT21  (1 << 21)
#define BIT22  (1 << 22)
#define BIT23  (1 << 23)
#define BIT24  (1 << 24)
#define BIT25  (1 << 25)
#define BIT26  (1 << 26)
#define BIT27  (1 << 27)
#define BIT28  (1 << 28)
#define BIT29  (1 << 29)
#define BIT30  (1 << 30)
#define BIT31  (1 << 31)


#define _8K_  (8 * 1024)

#define AHCI_HBA_CAP           0x00
#define AHCI_GLOBAL_HBA_CTRL   0x04
#define AHCI_VERSION_REG       0x10
#define AHCI_PORTS_IMPL        0x0C

#define AHCI_PxCLB            0x00
#define AHCI_PxCLB_U          0x04
#define AHCI_PxFB             0x08
#define AHCI_PxFB_U           0x0C
#define AHCI_PxIE             0x14
#define AHCI_PxCMD            0x18
#define AHCI_PxSSTS           0x28
#define AHCI_PxSCTL           0x2C
#define AHCI_PxCI             0x38
#define AHCI_PxSERR           0x30

#define E__PFA_DOESNT_EXIST      0x80000001
#define E__NOT_IN_AHCI_MODE      0x80000002
#define E__GARBAGE_ABAR          0x80000003
#define E__NO_DRIVES_ATTACHED    0x80000004
#define E__ERROR_IN_PIO_FIS      0x80000005
#define E__NO_PIO_FIS_RCVD       0x80000006
#define E__IDENTIFY_CMD_STUCK    0x80000007
#define E__READ_SECTOR_CMD_STUCK 0x80000008
#define E__ERROR_ENABLING_AHCI   0x80000009
#define E__ERROR_LYNX_POINT_AHCI 0x8000000A


#define CMDLIST_BAR   0x10000000
#define CMDTBL_BAR    0x10001000
#define PRDT_BAR      (CMDTBL_BAR + 0x80)
#define RCVDFIS_BAR   0x10008000
#define PHYSICAL_DATA_XFER_LOCATION 0x1000A000

#define MAX_INSTANCES_SUPPORTED 8


#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)
#define osReadDWord(regAddr)		 (*(UINT32*)(regAddr))

#define MARVELL_VID          0x1B4B
#define MARVELL_AHCI_DEVID   0x9235

#pragma pack(push, 1)
typedef struct rcvd_fis {
  UINT8 Rsvd[256]; // We are not really bothered with whats inside it.
} RcvdFis;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct prd_entry {
  UINT32 dba;       // Data base address
  UINT32 Rsvd[2];
  UINT16 dbc;       // Data Base count
  UINT16 Rsvd1;
} PrdEntry;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct cmd_table {
  UINT8     buffer[0x80]; // CmdFIS starts at offset 0
  PrdEntry  prd_entry0  ;
} CmdTable;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct cmd_header {
   UINT16   options;
   UINT16   prdtl;
   UINT32   prdbc;    // PRD Byte Count
   UINT32   ctba0;    // Command Table Base Address, MUST BE 128-bytes aligned.
   UINT32   Rsvd1;
   UINT32   Rsvd[4];
} CmdHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct cmd_list {
   CmdHeader   Header[32];
} CmdList;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct fis_reg_h2d
{
	// DWORD 0
	UINT8	fis_type;

	UINT8	rsv0:7;		// Reserved
	UINT8	c:1;		// Command/Control

	UINT8	command;	// Cmd reg
	UINT8	rsv4;

	// DWORD 1
	UINT8	lba0;		// LBA low register, 7:0
	UINT8	lba1;		// LBA mid register, 15:8
	UINT8	lba2;		// LBA high register, 23:16
	UINT8	device;		// Device register

	// DWORD 2
	UINT8	rsv3[4];		// Not of interest

	// DWORD 3
	UINT8   sector_count;   // How many sectors?
	UINT8	rsv2[3];	    // Not of interest

	// DWORD 4
	UINT8	rsv1[4];	// Reserved
} FisRegH2d;
#pragma pack(pop)

#endif /* AHCI_H_ */
