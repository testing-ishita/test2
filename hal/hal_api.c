
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

/* hal_api.c - interface function for the built-in service provider
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/hal/hal_api.c,v 1.1 2013-09-04 07:26:54 chippisley Exp $
 *
 * $Log: hal_api.c,v $
 * Revision 1.1  2013-09-04 07:26:54  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2011/05/16 14:30:43  hmuneer
 * Info Passing Support
 *
 * Revision 1.7  2009/05/15 10:57:53  jthiru
 * Added SYS_UDELAY for microsec delay
 *
 * Revision 1.6  2008/09/17 13:48:05  swilson
 * Add support for runtime error reporting via longjmp().
 *
 * Revision 1.5  2008/07/24 14:41:37  swilson
 * Add HAL delay function.
 *
 * Revision 1.4  2008/05/22 16:04:52  swilson
 * Add HAL console write and memory access interfaces - memory is just a shell at present.
 *
 * Revision 1.3  2008/05/14 09:59:01  swilson
 * Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
 *
 * Revision 1.2  2008/04/30 07:51:19  swilson
 * Add PCI support library.
 *
 * Revision 1.1  2008/04/24 16:03:54  swilson
 * Created HAL.
 *
 */


/* includes */

#include <stdtypes.h>
#include <setjmp.h>

#include <bit/hal.h>
#include <alloc.h>

/* defines */

/* typedefs */

/* constants */

/* locals */

static PF_SYS_CALL		pfSysCall;


/* globals */

jmp_buf		sErrorRecoveryJb;		/* jump buffer for runtime error recovery */


/* externals */

extern void spIoReadByte (CCT_IO_REG* psParams);
extern void spIoReadWord (CCT_IO_REG* psParams);
extern void spIoReadDword (CCT_IO_REG* psParams);

extern void spIoWriteByte (CCT_IO_REG* psParams);
extern void spIoWriteWord (CCT_IO_REG* psParams);
extern void spIoWriteDword (CCT_IO_REG* psParams);

extern void spPciReadReg (CCT_PCI_REG* psParams);
extern void spPciWriteReg (CCT_PCI_REG* psParams);

extern void spPciExprReadReg (CCT_PCIE_REG* psParams);
extern void spPciExprWriteReg (CCT_PCIE_REG* psParams);

extern void spMemGetPhysPtr (CCT_MEM_PTR* psParams);
extern void spMemGetBuffer (CCT_MEM_PTR* psParams);
extern void spMemFreeMem (CCT_MEM_PTR* psParams);
extern void spMemFreeAll (CCT_MEM_PTR* psParams);

extern void spConsMessage (char* psParams);

extern void spDelay (UINT32* pdDelay);


/* forward declarations */

static void runtimeError (UINT32 dErrorCode);



/*****************************************************************************
 * cctRegisterServiceProvider: 
 *
 * RETURNS: E__OK or E__HAL_INVALID_PROVIDER
 */

UINT32 cctRegisterServiceProvider
(
	PF_SYS_CALL	pfServiceProvider
)
{
	if (pfServiceProvider == NULL)
		return (E__HAL_INVALID_PROVIDER);

	pfSysCall = pfServiceProvider;

	return (E__OK);

} /* cctRegisterServiceProvider () */


/*****************************************************************************
 * cctSysCall: 
 *
 * RETURNS: None
 */

void cctSysCall
(
	SYS_SRVC	tServiceId,
	void*		psParams
)
{
	UINT32	dStatus;


	/* Check we have a valid Service Provider */

	if (pfSysCall == NULL)
		runtimeError (E__HAL_INVALID_PROVIDER);

	/* Call indirectly to the Service Provider */

	dStatus = (pfSysCall)(tServiceId, psParams);

	if(dStatus != E__OK)
		runtimeError (dStatus);

} /* cctSysCall () */


/*****************************************************************************
 * cctServiceProvider: 
 *
 * RETURNS: E__OK, or E__... runtime error
 */

UINT32 cctServiceProvider
(
	SYS_SRVC	tServiceId,
	void*		psParams
)
{
	UINT32	dExitCode = E__OK;

	switch (tServiceId)
	{
		/* Port I/O */

		case SYS_IO_READ_8 :
			spIoReadByte ((CCT_IO_REG*)psParams);
			break;

		case SYS_IO_READ_16 :
			spIoReadWord ((CCT_IO_REG*)psParams);
			break;

		case SYS_IO_READ_32 :
			spIoReadDword ((CCT_IO_REG*)psParams);
			break;

		case SYS_IO_WRITE_8 :
			spIoWriteByte ((CCT_IO_REG*)psParams);
			break;

		case SYS_IO_WRITE_16 :
			spIoWriteWord ((CCT_IO_REG*)psParams);
			break;

		case SYS_IO_WRITE_32 :
			spIoWriteDword ((CCT_IO_REG*)psParams);
			break;

		/* PCI */

		case SYS_PCI_READ :
			spPciReadReg ((CCT_PCI_REG*)psParams);
			break;

		case SYS_PCI_WRITE :
			spPciWriteReg ((CCT_PCI_REG*)psParams);
			break;

		case SYS_PCIE_READ :
			spPciExprReadReg ((CCT_PCIE_REG*)psParams);
			break;

		case SYS_PCIE_WRITE :
			spPciExprWriteReg ((CCT_PCIE_REG*)psParams);
			break;

		/* Memory */

		case SYS_GET_PHYS_PTR :
			spMemGetPhysPtr ((CCT_MEM_PTR*)psParams);
			break;

		case SYS_GET_BUFFER :
			spMemGetBuffer ((CCT_MEM_PTR*)psParams);
			break;

		case SYS_FREE_MEM :
			spMemFreeMem ((CCT_MEM_PTR*)psParams);
			break;

		case SYS_FREE_ALL :
			spMemFreeAll ((CCT_MEM_PTR*)psParams);
			free_all();
			break;

		/* Console I/O */

		case SYS_MESSAGE :
			spConsMessage ((char*)psParams);
			break;

		/* Timed Delay */

		case SYS_DELAY :
			spDelay ((UINT32*)psParams);
			break;
			
		case SYS_UDELAY :
			spUDelay ((UINT32*)psParams);
			break;

		/* Default */

		default	 :
			dExitCode = E__HAL_SERVICE_NOT_PRESENT;
	}
	
	return (dExitCode);

} /* cctServiceProvider () */


/*****************************************************************************
 * runtimeError: pass control (and error code) back to highest level caller
 *
 * RETURNS: none (error code returned via longjmp)
 */

static void runtimeError
(
	UINT32	dErrorCode
)
{
	longjmp (sErrorRecoveryJb, dErrorCode);

} /* runtimeError () */
