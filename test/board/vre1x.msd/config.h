/*******************************************************************************
*
* Filename:	 config.h
*
* Description:	Board specific configuration options.
*
* $Revision: 1.2 $
*
* $Date: 2014-07-24 16:09:41 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/board/vre1x.msd/config.h,v $
*
* Copyright 2012-2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#ifndef __config_h__
#define __config_h__

#define MAX_CONFIG_CPUS	1	  			/* Set max configured CPU's, overriding detection */
							    		/* Set 0 to use number of logical CPU cores found */

#define INCLUDE_NETWORK					/* define to include networking */
#define INCLUDE_LWIP_COMMAND_SERVER 	/* define to include LWIP command server */
#define INCLUDE_LWIP_SMC 				/* define to include LWIP Soak Master client */
#undef INCLUDE_STH_DEV_MODE				/* define to enable Slave Test Handler development mode */

#define USM_IP_ADDRESS	"192.67.84.241"	/* Universal Soak Master IP address */

#undef INCLUDE_DEBUG_VGA				/* define to use VGA for debug output */

#undef INCLUDE_DBGLOG					/* define to include debug log */

#undef INCLUDE_MAIN_DEBUG				/* define for main/initialization debug output */
#undef INCLUDE_TASK_DEBUG 				/* define for task debug output */
#undef INCLUDE_EXCEPTION_DEBUG			/* define for exception handling debug */

#undef INCLUDE_PORT_DEBUG 				/* define for sheduler debug output */

#undef INCLUDE_DEMO_TASKS				/* define to include DEMO tasks */

#undef INCLUDE_DEBUG_PORT_INIT			/* define to initialize debug serial port (if different from console port)*/

#endif /* __config_h__ */
