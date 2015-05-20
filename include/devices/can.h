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

/* can.h - definitions for CAN
 */

/* Version control:
*
* $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/can.h,v 1.1 2013-09-04 07:40:26 chippisley Exp $
*
* $Log: can.h,v $
* Revision 1.1  2013-09-04 07:40:26  chippisley
* Import files into new source repository.
*
* Revision 1.1  2009/06/05 14:33:23  jthiru
* Initial check in for CAN test module
*
*
*
*/

#define MAX_NUM_GPIO_LINES              	14
#define GPIO_OUTPUT                             0x0
#define GPIO_INPUT                              0x1
#define MCP2515_CLK                             25000000
/*#define INT32 int*/

#define CW_GPIO_ENABLE_REG                      0x00 
#define CW_GPIO_DIRECTION_SELECT_REG            0x04
#define CW_GPIO_DATA_REG                        0x08
#define CW_GPIO_POSEDGE_ENABLE_REG              0x10
#define CW_GPIO_GPE_ENABLE_REG                  0x14
#define CW_GPIO_SMI_ENABLE_REG                  0x18
#define CW_GPIO_TRIG_STATUS_REG                 0x1c

#define RW_GPIO_ENABLE_REG                      0x20 
#define RW_GPIO_DIRECTION_SELECT_REG            0x24
#define RW_GPIO_DATA_REG                        0x28
#define RW_GPIO_POSEDGE_ENABLE_REG              0x2c
#define RW_GPIO_NEGEDGE_ENABLE_REG              0x30
#define RW_GPIO_GPE_ENABLE_REG                  0x34
#define RW_GPIO_SMI_ENABLE_REG                  0x38
#define RW_GPIO_TRIG_STATUS_REG                 0x3c


#define MCP_PCI_VENDOR_ID 			0x8086 
#define MCP_PCI_DEVICE_ID 			0x8119
#define PCI_GPIO_BASE 				0x44 
#define GPIO_BASE_MASK 				0x0000FFC0

/*Refer Schematics - HW Input */
#define  MCP2515_SPI_CLK			5
#define  MCP2515_SPI_CS 			9
#define  MCP2515_SPI_MOSI			6
#define  MCP2515_SPI_MISO			2

#define MSB_MASK 				0x80
#define SPI_DELAY 				4 /*Was 2 Hari*/

#define MCP2515_READ_REG 	0x0
#define MCP2515_WRITE_REG	0x1
#define MCP2515_MODE_SET 	0x2
#define MCP2515_MODE_GET 	0x3
#define MCP2515_BITRATE_SET 	0x4
#define MCP2515_BITRATE_GET 	0x5
#define MCP2515_UPDATE_CDATA 	0x6
#define MCP2515_RETRIEVE_CDATA	0x7
#define MCP2515_READ_BIT 		0x8
#define MCP2515_WRITE_BIT 		0x9


#define MCP2515_FRAME_MAX_DATA_LEN 8


#define INSTRUCTION_WRITE       		0x02
#define INSTRUCTION_READ        		0x03
#define INSTRUCTION_BIT_MODIFY  		0x05
#define INSTRUCTION_RESET       		0xc0
#define INSTRUCTION_READ_STATUS    		0xa0
#define INSTRUCTION_RCV_STATUS    		0xb0
#define INSTRUCTION_LOAD_TXB(n) 		(0x40 + 2 * (n))
#define INSTRUCTION_READ_RXB(n) 		(0x90 + 4 * (n))
#define INSTRUCTION_REQ_SEND(n) 		(0x80 + (1 << n))

#define MAX_TX_BUFFERS		3
#define MAX_RX_BUFFERS		2


#define CANSTAT        		 	0x0e
#define CANCTRL       	 		0x0f
#define CANCTRL_REQOP_CONF      		0x80
#define CANCTRL_REQOP_MASK     		0xe0

#define MCP2515_NORMAL_MODE                   0x0
#define MCP2515_SLEEP_MODE                    0x1
#define MCP2515_LPBK_MODE                     0x2
#define MCP2515_LISTEN_ONLY_MODE              0x3
#define MCP2515_CONF_MODE                     0x4
#define CANCTRL_OSM                           0x08
#define TEC           			0x1c
#define REC           			0x1d
#define CNF1          			0x2a
#define CNF2          			0x29
#define CNF2_BTLMODE  			0x80
#define CNF3          			0x28
#define CNF3_SOF      			0x08
#define CNF3_WAKFIL   			0x04
#define CNF3_PHSEG2_MASK 			0x07
#define CANINTE      				0x2b
#define CANINTE_MERRE 			0x80
#define CANINTE_WAKIE 			0x40
#define CANINTE_ERRIE 			0x20
#define CANINTE_TX2IE 			0x10
#define CANINTE_TX1IE 			0x08
#define CANINTE_TX0IE 			0x04
#define CANINTE_RX1IE 			0x02
#define CANINTE_RX0IE 			0x01
#define CANINTF       			0x2c
#define CANINTF_MERRF 			0x80
#define CANINTF_WAKIF				0x40
#define CANINTF_ERRIF 			0x20
#define CANINTF_TX2IF 			0x10
#define CANINTF_TX1IF 			0x08
#define CANINTF_TX0IF 			0x04
#define CANINTF_RX1IF 			0x02
#define CANINTF_RX0IF 			0x01
#define EFLG          			0x2d
#define EFLG_RX1OVR   			0x80
#define EFLG_RX0OVR   			0x40
#define TXBCTRL(n)  				((n * 0x10) + 0x30)
#define TXBCTRL_TXREQ_BIT  			3
#define TXREQ_CLEAR_MASK 			0xf7
#define EXIDE_BIT					0x3
#define RXBCTRL(n)  				((n * 0x10) + 0x60)
#define RXBCTRL_ROLLOVER  			2


#define NR_FILTERS_BUFFER0				2
#define NR_FILTERS_BUFFER1				4


#define CAN_FRAME_MAX_DATA_LEN 			8
#define SPI_TRANSFER_BUF_LEN			(2*(6 + CAN_FRAME_MAX_DATA_LEN))
#define STD_DATA_FRAME				0
#define EXTD_DATA_FRAME				1

#define  ERR_OPEN			    01
#define  ERR_INVALID			02
#define  ERR_UNKNOWN_BOARD	  	03
#define  ERR_UNSUPPORTED_BOARD  04
#define  ERR_TRANSMIT			05
#define  ERR_RECEIVE			06
#define  ERR_COPY			    07
#define  BYTE_MASK			  	0xff

#define E__REG_ACCESS			E__BIT
#define E__LocalLB				E__BIT + 0x1


struct mcp{
	UINT32 regAddress;
	UINT32 data;
	UINT8 bitno;
	UINT32 mode;	
	UINT32 bitrate;
};

struct mcp2515_config_data {
	UINT16 sid;
	UINT8 eide;
	UINT32 eid;
};

struct mcp2515_user_data {
	UINT8 rtr;
	UINT8 dlc;
};

struct mcp2515_frame_header {
	struct mcp2515_config_data cdata;
	struct mcp2515_user_data udata;
};

struct mcp2515_frame {
        struct mcp2515_frame_header header;
        UINT8 data[MCP2515_FRAME_MAX_DATA_LEN];
};

struct mcp2515_txBufferConfig {
		UINT8 bufid;
		UINT8 bufPrio;
};

struct mcp2515_rxBufferConfig {
		UINT8 bufid;
		UINT8 msgRollOver;
		UINT8 opMode;
};



struct gpioInfo
{
      UINT16   wBase;
      UINT8    bGpioPin;
};

struct spiGpioPlatformData {
        UINT32 dPinClk;
        UINT32 dPinMiso;
        UINT32 dPinMosi;
        UINT32 dPinCs;
        UINT32 dGpioBase;
        UINT8 noSpiDelay;
};

/*GPIO Functions*/
UINT8 bGpioSetDirection(UINT8 gpio_pin,UINT32 dir);
UINT8 bGpioGetDirection(UINT8 gpio_pin);
UINT8 bGpioSetVal(UINT8 gpio_pin,UINT32 value);
UINT8 bGpioGetVal(UINT8 gpio_pin);
UINT8 bGpioEnable(UINT8 gpio_pin,UINT8 enable);
UINT8 bGpioIsValid(UINT8 gpio_pin);
void vGpioBaseInit(UINT16 base);


struct mcp2515_platform_data {
	UINT32 f_osc;
	void (*transceiver_enable)(UINT32 enable);
};

struct mcp2515 {
	UINT32 bit_rate;
	UINT32 reg;
	UINT8 *spi_transfer_buf;
 	UINT8 rx_over_errors;
 	UINT8 tx_packets;
 	UINT8 tx_bytes;
};

/*SPI-GPIO Functions*/
void vSpiDelay(UINT32 usecs);
void vSetSck(UINT8 val);
void vSetMosi(UINT8 val);
UINT8 bGetMiso(void);
void vSpiGpioChipSelect(UINT8 on);
void vSpiGpioTxByte( UINT8 data);
void vSpiGpioTxBuf(UINT8 *tx_buf,UINT8 nbytes);
UINT8 bSpiGpioRxByte(void);
void vSpiGpioRxBuf(UINT8 *rx_buf,UINT8 nbytes);
void vSpiGpioInit(void); 

UINT8 mcp2515_get_bit(UINT8 reg,UINT8 bitno);
void mcp2515_set_bit(UINT8 reg,UINT8 bitno,UINT8 val);
UINT8 mcp2515_read_reg(UINT8 reg);
void mcp2515_write_reg(UINT8 reg,UINT8 val);
void mcp2515_write_bits(UINT8 reg,UINT8 mask,UINT8 val);
UINT8 mcp2515_read_status(void);
UINT8 mcp2515_read_rxstatus(void);
void mcp2515_hw_init(void);
void vMcp2515HwReset(void);
void mcp2515_hw_wakeup(void);
void mcp2515_hw_tx_header(struct mcp2515_frame *frame,UINT8 tx_buf_id);
void mcp2515_hw_tx_data(struct mcp2515_frame *frame,UINT8 tx_buf_id);
UINT8 mcp2515_hw_tx(struct mcp2515_frame *frame,UINT8 tx_buf_id);
void mcp2515_hw_rx_header(struct mcp2515_frame *frame,UINT8 rx_buf_id);
void mcp2515_hw_rx_data(struct mcp2515_frame *frame,UINT8 rx_buf_id);
UINT8 mcp2515_hw_rx(struct mcp2515_frame *frame,UINT8 rx_buf_id);
void mcp2515_hw_rts(UINT8 tx_buf_id);
void mcp2515_set_mode(UINT8 mode);
UINT32 mcp2515_loopbacktest(void);
void vMcp2515RegisterAccess(void);

