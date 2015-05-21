/*******************************************************************************
*
* Filename:	 ssmmp.h
*
* Description:	Universal Soak Master protocol header.
*
* $Revision: 1.4 $
*
* $Date: 2014-07-24 15:06:35 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/lwip-1.4.0/lwIP_Apps/apps/SmClient/ssmmp.h,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#ifndef _SSMMP_H_
#define _SSMMP_H_

#define SSMMP_PACKET_LEN            4096            //Max SSMMP Packet length
#define SSMMP_PACKET_ID             "SHAANTI"       //Packet ID fr SSMMP packets
#define SSMMP_TESTTITLE_LEN   		45              //Max length of the test title
#define SSMMP_TESTTAIL_LEN			100             //Max test tail length
#define SSMMP_BOARDNAME_SIZE		20              //Max board name size
#define SSMMP_BOARD_SERIAL_NUM      20              //Max board serial number
#define SSMMP_MAX_SLAVE_BOARDS		32              //Max slave boards supported per system controller
#define SSMMP_MAX_TESTS_PER_BOARD   80              //Max number of tests per board
#define SSMMP_MAX_ERR_DESC          1024            //Max length of test error description
#define SSMMP_MAX_STATUS_INFO       1024            //Max length of status information


//Opcodes - These are the commands that are sent by USM to the slave boards
enum ssmmpOpcode {
    SSMMP_RSINFO,                                   //Request slave information from the system controller
    SSMMP_RSINFO_ACK,                               //Slave information returned by the system controller
    SSMMP_AADDR,                                    //Address allocated by USM to all the slave boards
    SSMMP_AADDR_ACK,                                //Acknowledgement from the system controler after successful address allocation
    SSMMP_RTLIST,                                   //Test list request sent by USM to each slave board
    SSMMP_RTLIST_ACK,                               //Test list returned by slave boards
    SSMMP_ETEST,                                    //Execute test
    SSMMP_ETEST_ACK,                                //Test result
    SSMMP_RSTATUS,                                  //Request status information from slave board
    SSMMP_RSTATUS_ACK                               //Status information returned by slave board
};


//Error codes
#define USM_STH_RESP_ERROR		0x80000001          //No response from STH queue
#define USM_PKT_ERROR			0x80000002          //Packet processing error
#define USM_NSUPP_NUM_OF_BOARDS	0x80000003          //If the detected boards are more than SSMMP_MAX_SLAVE_BOARDS
#define USM_NSUPP_NUM_OF_TEST   0x80000004          //If the detected boards are more than SSMMP_MAX_TESTS_PER_BOARD

//Board Address Masks
#define USM_BOARDADDR_SLOTID_MASK   0x000000FF
#define USM_BOARDADDR_INST_MASK     0x0000FF00
#define USM_BOARDADDR_INST_SHIFT    0x8

//Board Address Allocation
#define USM_BOARDADDR_SLOTID_MECH   1               //The board ID is determined by the slot ID provided by
                                                    //the slave
#define USM_BOARDADDR_SMALLOC_MECH  2               //The board ID is assigned by USM


#pragma pack(1)

//SSMMP Packet Header
typedef struct				                        //All SSMMP packet will be prepended by this packet header
{
    char packetId[8];                               //All SSMMP packet will contain SSMMP_PACKETID in header
    char opcode;                                    //USM request opcode as defined in ssmmpOpcode
    UINT32 packetSize;                        		//Packet size excluding the SSMMPHEADER
    UINT32 result;                            		//Result of request packet execution from system controller
    
} SSMMPHEADER;


typedef struct				                        //USM sends this packet to system controller to get slave information
                                                    //This information corresponds to AA_SL packet in SM3 protocol
{
    SSMMPHEADER header;
    
} SSMMPRSLINFO;


typedef struct					                    //System controller return the following information for all the
{                                                   //slaves connected to it.This information corresponds to AA_SL
                                                    //packet in SM3 protocol along with some additional information

    UINT32 boardAddress;                      		//Slave board address. This is typically the slot address as per SM3
    UINT32 chassisId;                         		//Chassis Id on which the board resides.
    char boardName[SSMMP_BOARDNAME_SIZE];   		//Board name
    char serNumber[SSMMP_BOARD_SERIAL_NUM]; 		//Board serial number
    
} SSMMPSLAVEINFO;


typedef struct					                    //Acknowledgement packet from system controller to USM for SSMMP_RSINFO
{                                                   //opcode
    SSMMPHEADER header;
    UINT8 boardAddrDetectMechanism;                 //Specifies the board address detection mechanism
    UINT32 noOfSlaveBoards;                		    //Number of slaves detected by system controller.If system controller is
                                                    //the only board in the system , then this entry will be 1
    SSMMPSLAVEINFO slaveBoard[SSMMP_MAX_SLAVE_BOARDS];	//Slave board info.If system controller is
                                                    	//the only board in the system , then there will be just
                                                    	//one structure element
} SSMMPRSLINFOACK;


typedef struct										//USM sends this packet to system controller to assign slave addresses
{                                                   //This corresponds to AA_PORT command in SM3
    SSMMPHEADER header;
    UINT32 	noOfSlaveBoards;                		//Number of slaves board to whom to assign slave addresses
    UINT32 slaveBoardAddress[SSMMP_MAX_SLAVE_BOARDS]; 	//Slave board addresses in the same order as the information was
                                                        //provided by the system controller
} SSMMPAADDR;


typedef struct										//System controller acknowledgement for successfull  SSMMP_AADDR
{
    SSMMPHEADER header;
    
} SSMMPAADDRACK;


typedef struct										//USM sends this packet to request the test list from each slave boards
{                                                   //This corresponds to RBL in SM3 protocol
    SSMMPHEADER header;
    UINT32 boardAddress;                     		//Slave board address

} SSMMPRTLIST;


typedef	struct										//Test information
{
    UINT16  testNumber;                     		//Test number
    UINT16  testVersion;                    		//Test version
    char testTitle[SSMMP_TESTTITLE_LEN];  			//Test name

} SSMMPTESTLISTITEM;


typedef struct										//System controller sends this packet on behalf of addressed slave board
{                                                   //or itself with the supported test list.This corresponds to SBL in SM3
    SSMMPHEADER header;
    UINT32 boardAddress;                      		//Slave board address
    UINT32 noOfTests;                         		//Number of supported boards
    SSMMPTESTLISTITEM testList[SSMMP_MAX_TESTS_PER_BOARD];   //Test list
    
} SSMMPRTLISTACK;


typedef struct										//USM send this command to a slave board to execute a test
{                                                   //This corresponds to BGO in SM3 protocol
    SSMMPHEADER header;
    UINT32 boardAddress;                      		//Slave board address
    UINT32 testNumber;                        		//Test number
    char testTail[SSMMP_TESTTAIL_LEN];          	//Test tail
    
} SSMMPETEST;


typedef struct										//Slave board sends this packet on behalf of addressed slave board
{                                                   //or itself with the result of test execution requested in SSMMP_ETEST
                                                    //packet
    SSMMPHEADER header;
    UINT32 boardAddress;
    UINT32 testNumber;
    UINT32 testErrorCode;
    char errorDesc[SSMMP_MAX_ERR_DESC];
    
} SSMMPETESTACK;


typedef struct										// USM send this command to a slave board to request status information
{
	SSMMPHEADER header;
	UINT32 boardAddress;							// Slave board address
	char statusType;								// Status type requested 

} SSMMPRSTATUS;


typedef struct										// The slave board returns this packet containing the requested status information
{
	SSMMPHEADER header;
	UINT32 boardAddress;							// Slave board address
	char statusType;								// Status type requested
	char statusInfo[SSMMP_MAX_STATUS_INFO];			// Status information, can be a text string or an overlayed status structure 
                                                    // Note: the size of SSMMPRSTATUSACK must not exceed SSMMP_PACKET_LEN
                                                    // adjust value of SSMMP_MAX_STATUS_INFO as necessary
} SSMMPRSTATUSACK;


typedef union
{
	SSMMPHEADER header;
	
	SSMMPRSLINFO	rslInfo;
	SSMMPRSLINFOACK rslInfoAck;
	
	SSMMPAADDR 		aaddr;
	SSMMPAADDRACK 	aaddrAck;
	
	SSMMPRTLIST 	rtList;
	SSMMPRTLISTACK 	rtListAck; 
	
	SSMMPETEST 		exeTest;
	SSMMPETESTACK 	exeTestAck;
	
	SSMMPRSTATUS	rStatus;
	SSMMPRSTATUSACK rStatusAck;
	
} SSMMPACKET;

#pragma pack()

#endif /* _SSMMP_H_ */
