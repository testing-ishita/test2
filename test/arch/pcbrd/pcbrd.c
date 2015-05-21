
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


#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>

#include <devices/tsi148.h>

#include <private/debug.h>

#include <cute/arch.h>
#include <cute/packet.h>


/*******************************************************************************
 * Dummy routines used when VME is not included
 * ****************************************************************************/

/*****************************************************************************
 * GetVmeBars: Identify board as Monarch or slave
 *
 * RETURNS: board type
 */
void GetVmeBars (VMEBARS *vme_Bars)
{
	vme_Bars->LocalDev = 0;
	vme_Bars->Intercon = 0;
	vme_Bars->CrCsr    = 0;
}

/*****************************************************************************
 * IdentifyAgent: Identify board as Monarch or slave
 *
 * RETURNS: board type
 */
UINT8 IdentifyAgent (void)
{
	return (ID__MONARCH);

} /* IdentifyAgent () */


/*****************************************************************************
 * bReadRtcSeconds: read current rtc time in seconds
 *
 * RETURNS: rtc seconds reading
 */
UINT8 bReadRtcSeconds (void)
{

	return 0;

} /* bReadRtcSeconds () */


/*****************************************************************************
 * InitialiseVMEDevice: Initialise TSI148
 *
 * RETURNS: success or error code
 */
UINT32 InitialiseVMEDevice(void)
{
	sysDebugWriteString("Initialise VME device failed!\n");
	
	return E__FAIL;
	

} /* InitialiseUniverse () */


/*****************************************************************************
 * configureCoopWindows: this configures 4MB windows from PCI->VME
 *
 * RETURNS: E__OK or E__FAIL
 */


void configureCoopWindows(sm3vmeSlave *asSlots, sm3vme_COOP *syscon_coop)
{
	
}


/*****************************************************************************
 * Int2Handler: this is the interrupt service routine for the TSI148 PCI
 * interrupt  - specifically it handles Level-2 interrupts for Auto-ID
 *
 * RETURNS: none
 */

//not sure if this works must be tested in a chasis without geographic address

UINT32 intHappened = 0;

void Int2Handler (void)
{
	intHappened++;

} /* Int2Handler () */



/*****************************************************************************
 * PerformVmeAutoId: this runs the Auto-ID process for either monarch or slave
 *
 * RETURNS: the logical slot number for this board
 */
UINT8 PerformVmeAutoId (void)
{
	
	return 0;

} /* PerformVmeAutoId () */


/*****************************************************************************
 * vmeCheckStatus: This function tests the status bits of TSI148
 * RETURNS: 1 if error condition set, otherwise 0;
 */
UINT8 vmeCheckStatus (void)
{
	return 1;
}



/*****************************************************************************
 * bIcReadByte: read interconnect byte
 *
 * RETURNS: data
 */
UINT32 bIcReadByte( UINT8 bSlot,	UINT16	offset, UINT8* data)
{
	
	*data = 0xff;
	return E__FAIL;
	
} /* vIcReadReg () */


/*****************************************************************************
 * bIcReadByte: write interconnect byte
 *
 * RETURNS: success or error code
 */
UINT32 bIcWriteByte( UINT8 bSlot,	UINT16	offset, UINT8* data)
{
	return E__FAIL;

} /* vIcReadReg () */



