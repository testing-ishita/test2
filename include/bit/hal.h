#ifndef __hal_h__
	#define __hal_h__


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

/* hal.h - definitions for HAL and built-in service provider
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/bit/hal.h,v 1.1 2013-09-04 07:35:26 chippisley Exp $
 *
 * $Log: hal.h,v $
 * Revision 1.1  2013-09-04 07:35:26  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.7  2010/05/05 15:48:16  hmuneer
 * Cute rev 0.1
 *
 * Revision 1.6  2009/05/15 10:59:18  jthiru
 * Added SYS_UDELAY for microsec delay
 *
 * Revision 1.5  2009/02/02 17:26:54  jthiru
 * Added logical address member to CCT_MEM_PTR
 *
 * Revision 1.4  2008/07/24 14:49:52  swilson
 * Add HAL delay function.
 *
 * Revision 1.3  2008/05/22 16:04:57  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 * Revision 1.2  2008/04/30 07:51:21  swilson
 * Add PCI support library.
 *
 * Revision 1.1  2008/04/24 16:04:38  swilson
 * Updated includes to support HAL and BIT library.
 *
 */


/* includes */

#include <stdtypes.h>
#include <errors.h>
#include <bit/pci.h>


/* defines */

	/* HAL Errors */

#define E__HAL_SERVICE_NOT_PRESENT	(E__HAL + 0x00000001)
#define E__HAL_PORT_ACCESS_DENIED	(E__HAL + 0x00000002)
#define E__HAL_DEVICE_ACCESS_DENIED	(E__HAL + 0x00000003)
#define E__HAL_MEM_ADDRESS_DENIED	(E__HAL + 0x00000004)
#define E__HAL_BUFF_REQUEST_DENIED	(E__HAL + 0x00000005)
#define E__HAL_INVALID_PROVIDER		(E__HAL + 0x00000010)
#define E__HAL_UNDEFINED			(E__HAL + 0x000000FF)


/* typedefs */

	/* HAL Services */

typedef enum tagSysSrvc
{
	SYS_IO_READ_8		= 0x00000001,
	SYS_IO_WRITE_8,
	SYS_IO_READ_16,
	SYS_IO_WRITE_16,
	SYS_IO_READ_32,
	SYS_IO_WRITE_32,

	SYS_PCI_READ		= 0x00000010,
	SYS_PCI_WRITE,

	SYS_PCIE_READ		= 0x00000018,
	SYS_PCIE_WRITE,

	SYS_GET_PHYS_PTR	= 0x00000020,
	SYS_GET_BUFFER,

	SYS_FREE_MEM		= 0x00000030,

	SYS_FREE_ALL		= 0x00000040,

	SYS_MESSAGE			= 0x00000050,

	SYS_DELAY			= 0x00000060,
	
	SYS_UDELAY			= 0x00000070

}  SYS_SRVC;

typedef UINT32 (*PF_SYS_CALL)(SYS_SRVC, void*);

	/* Service Parameter-passing Structures */

typedef struct tagCctMemPtr		/* Service Interface: Memory Pointer */
{
	UINT64		qPhysicalAddress;	/* physical address of allocated block */
	UINT32		dBlockSize;			/* size of required block */
	UINT32		dHandle	;			/* identifier used to track this allocation */
	PTR48  		pBlock;				/* pointer to allocated block, 16:32 format */
	UINT32		dlogaddr;			/* logical address */

} CCT_MEM_PTR;

typedef struct tagCctIoReg		/* Service Interface: I/O Register R/W */
{
	UINT32		dPort;			/* port number */
	UINT32		dData;			/* data value */ 

} CCT_IO_REG;

typedef struct tagCctPciReg		/* Service Interface: PCI Register R/W */
{
	PCI_PFA		pfa;			/* PCI Function Address */
	UINT8		bRegister;		/* register offset */
	REG_SIZE 	size;			/* register size */
	UINT32		dData;			/* data value */ 

} CCT_PCI_REG;

typedef struct tagCctPcieReg	/* Service Interface: PCI Express Register R/W */
{
	PCI_PFA		pfa;			/* PCI Function Address */
	UINT16		wRegister;		/* register offset 0 - 4095 */
	REG_SIZE 	size;			/* register size */
	UINT32		dData;			/* data value */ 

} CCT_PCIE_REG;


/* externals */

extern UINT32 cctRegisterServiceProvider (PF_SYS_CALL pfServiceProvider);

extern void cctSysCall (SYS_SRVC tServiceId, void* psParams);

extern void spUDelay (UINT32* pdDelay);

/* forward declarations */


#endif
