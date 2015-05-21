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

/* serial.h - definitions for serial port communication
 */
#ifndef SERIAL_H_
#define SERIAL_H_

UINT32 InitSerialComm(void);
UINT32 wSerialMessageGet( UINT8 *pbMessage,	UINT16 wMax, UINT16 *wLength );
UINT32 wSerialMessagePut( const UINT8 *pbMessage,UINT16 wLength, UINT32 dTimeout );
UINT32 Serial_detect_message ();
UINT8  serialSoakMasterReady (void);
void   serialSetPacketWaiting (void);

#endif /* SERIAL_H_ */
