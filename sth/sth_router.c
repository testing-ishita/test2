
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

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/sth/sth_router.c,v 1.2 2014-01-29 13:38:28 hchalla Exp $
 * $Log: sth_router.c,v $
 * Revision 1.2  2014-01-29 13:38:28  hchalla
 * Added support for SRIO SOAK.
 *
 * Revision 1.1  2013-09-04 07:45:36  chippisley
 * Import files into new source repository.
 *
 * Revision 1.8  2011/10/27 15:50:21  hmuneer
 * no message
 *
 * Revision 1.7  2011/02/01 12:12:07  hmuneer
 * C541A141
 *
 * Revision 1.6  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.5  2010/11/04 17:44:19  hchalla
 * Added support for CPCI backplane and debug info.
 *
 * Revision 1.4  2010/09/15 12:23:26  hchalla
 * Added support for TR 501 BIT/CUTE
 *
 * Revision 1.3  2010/06/24 13:27:24  hchalla
 * Code Cleanup, Added headers and comments.
 *
 */

/* includes */

#include <stdtypes.h>
#include <stdio.h>
#include <string.h>
#include <errors.h>

#include <private/cpu.h>

#include <cute/sth.h>

/*****************************************************************************\
 *
 *  TITLE:  vSTHRouter ()
 *
 *  ABSTRACT:  sth polls interconnect for service request
 *             parcels from a MTH.  When a parcel is received it performs
 *             the function requested and starts polling again until the MTH
 *             tells it to quit.
 *
 * 	RETURNS: NONE
 *
\*****************************************************************************/


void vSTHRouter (void)
{



#if defined(VPX)

	vpxSthInit();

	vpxSthLoop();

	sysHalt ();

#endif




#if defined(CPCI)

	cpciSthInit();

	cpciSthLoop();

	sysHalt ();

#endif


#if defined(SRIO)

	cpciSthInit();

	cpciSthLoop();

	sysHalt ();

#endif

#if defined(VME)

	vmeSthInit();

	vmeSthLoop();

	sysHalt ();

	#endif

} /* vPbitSequencer () */




