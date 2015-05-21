/************************************************************************
 *                                                                      *
 *      Copyright 2014 Concurrent Technologies, all rights reserved.    *
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
/* hda.h - definitions for High Definition Audio
 */
/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/hda.h,v 1.2 2014-03-07 09:58:08 cdobson Exp $
*
* $Log: hda.h,v $
* Revision 1.2  2014-03-07 09:58:08  cdobson
* Added AlwaysFitted field to HDA_CODEC. So we dont error on optional
* codecs which are not fitted.
*
* Revision 1.1  2014-03-06 16:31:02  cdobson
* Definitions for new audio tests in audioTest2.c.
*
*
*/
#ifndef __HDA_H
#define __HDA_H

/* defines */

#define HDA_MAX_CODECS              8

/* Pin functions */
#define HDA_HEADPHONE				1
#define HDA_LINE_OUT				2
#define HDA_MICROPHONE				3
#define HDA_LINE_IN					4 


/* typedefs */

typedef const struct {
  UINT8			Nid;     	// NID for pin
  UINT8			Function;   // Pin function Line in/out headphone, mic...
} HDA_WIDGET;

typedef const struct {
  UINT8			LinkId;		// Codec bus id
  UINT32		CodecId;    // codec device id
  UINT8			AlwaysFitted;// always fitted or optional
  UINT8			TestNid;    // NID for general tests (PC Beep)
  HDA_WIDGET	*WidgetList;// pointer to array of widgets
} HDA_CODEC;

typedef const struct {
  PCI_PFA	 	pfa;		// PFA of HDA PCI device
  HDA_CODEC  	*CodecList;	// pointer to array of known codecs
} HDA_DEVICE;


#endif
