
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

/* can.c - CAN Controller BIT test module
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/can.c,v 1.1 2013-09-04 07:46:42 chippisley Exp $
 *
 * $Log: can.c,v $
 * Revision 1.1  2013-09-04 07:46:42  chippisley
 * Import files into new source repository.
 *
 * Revision 1.5  2011/10/27 15:50:39  hmuneer
 * no message
 *
 * Revision 1.4  2011/01/20 10:01:26  hmuneer
 * CA01A151
 *
 * Revision 1.3  2009/06/11 09:57:47  swilson
 * Remove space at head of name. Shorten name as exceeds 32 chars.
 *
 * Revision 1.2  2009/06/10 10:12:54  jthiru
 * Added safe memset in tx header filling routine
 *
 * Revision 1.1  2009/06/05 14:31:10  jthiru
 * Initial check in for CAN test module
 *
 *
 */

 /* includes */
 
#include <stdtypes.h>
#include <errors.h>		
#include <bit/bit.h>
#include <stdtypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <devices/can.h>
#include <bit/hal.h>
 
#include <bit/pci.h>
#include <bit/io.h>
#include <bit/mem.h>
#include <bit/delay.h>
#include <devices/pci_devices.h>
 
 
/* #define DEBUG */
#ifdef DEBUG
		 static char buffer[128];
#endif
 
/* constants */

/* locals */

/* globals */


/* externals */
extern void vDelay(UINT32 dMsDelay);
extern UINT32 dGetPhysPtr (UINT32 dBase, UINT32 dLength, PTR48* pMemory, void* dlogaddr);
extern void vConsoleWrite(char* achMessage);
extern void vFreePtr(UINT32 dHandle);


/* forward declarations */
static void vGpioSetValue(UINT8 bgpioPin, UINT32 dValue);
static UINT8 bGpioGetValue(UINT8 bgpioPin);

struct spiGpioPlatformData pData;
struct gpioInfo gpioData;

static struct mcp2515_platform_data pMcpData;
static struct mcp2515_config_data config_data;


/*****************************************************************************
* bGpioIsValid: Function to check if GPIO pin number is valid 
*
* RETURNS: Test status
*/

UINT8 bGpioIsValid
(
	UINT8 bgpioPin
)
{
	/*Max GPIO is 14 - Adding check for valid GPIO No*/
	return (bgpioPin < MAX_NUM_GPIO_LINES);
}


/*****************************************************************************
* bGpioGetVal: Function to obtain the GPIO value 
*
* RETURNS: Test status
*/

UINT8 bGpioGetVal
(
	UINT8 bgpioPin
)
{
	if(!bGpioIsValid(bgpioPin))
		return ERR_INVALID;

	return(bGpioGetValue(bgpioPin));
}


/*****************************************************************************
* gpio_set_val: Function to set the GPIO value 
*
* RETURNS: Test status
*/

UINT8 gpio_set_val
(
	UINT8 bgpioPin,
	UINT32 dValue
)
{
	if(!bGpioIsValid(bgpioPin))
		return ERR_INVALID;

	vGpioSetValue(bgpioPin,dValue);
	return 0;
}


/*****************************************GPIO_SET_VALUE*****************************************************
        DESCRIPTION: This function is used to set or reset a particular GPIO Pin.
        INPUT: GPIO Pin Numbers,Value    OUTPUT : No Return Values
        STEP 1: Calculate Absolute address for GPIO Data Register = GPIO_BASE(PCI CONFIG SPACE READ) 
		+ DATA REG OFFSET(EDS Pg 494) 
        STEP 2: Read the Previous Data from the Data Register.
        STEP 3: Shift the bit dValue to appropriate bit for the GPIO Pin Number.
        STPE 4: Write the dValue into the Data Register.
***********************************************************************************************************/

static void vGpioSetValue
(	
	UINT8 bgpioPin, 
	UINT32 dValue
)
{
	UINT32 address,data = 0;
	address = gpioData.wBase + CW_GPIO_DATA_REG;
	data = dIoReadReg(address, REG_16);
	data = (dValue?(data | (1 << bgpioPin)):(data & ~(1 << bgpioPin)));
	vIoWriteReg(address, REG_16, data);
}


/*****************************************GPIO_GET_VALUE*****************************************************
        DESCRIPTION: This function is used to read status of particular GPIO Pin.
        INPUT: GPIO Pin Numbers     OUTPUT : Status of GPIO Pin(High/Low)
        STEP 1: Calculate Absolute address for GPIO Data Register = GPIO_BASE(PCI CONFIG SPACE READ) 
		+ DATA REG OFFSET(EDS Pg 494) 
        STEP 2: Read the Data from Data Register.
        STEP 3: Mask the bit and return the status.
***********************************************************************************************************/

static UINT8 bGpioGetValue(UINT8 bgpioPin)
{
	UINT16 dValue;
	UINT32 address;
	address = gpioData.wBase + CW_GPIO_DATA_REG;
	dValue = dIoReadReg(address, REG_16);
	dValue = (dValue & (1 << bgpioPin));
	return dValue;
}


/*****************************************GPIO_SET_DIRECTION************************************************
        DESCRIPTION: This function is used to set direction(INPUT/OUTPUT)for particular GPIO Pin.
        INPUT: GPIO Pin Numbers,Direction    OUTPUT : Error if Fails
        STEP 1: Calculate Absolute address for GPIO Data Register = GPIO_BASE(PCI CONFIG SPACE READ) 
                + DIRECTION REG OFFSET(EDS Pg 493) 
        STEP 2: Read the Previous Data from the Direction Register.
        STEP 3: Shift the direction dValue to appropriate bit for the GPIO Pin Number.
        STPE 4: Write the dValue into the Direction Register.
***********************************************************************************************************/

UINT8 bGpioSetDirection(UINT8 bgpioPin,UINT32 dir)
{
	UINT32 address,dValue = 0;

	if(!bGpioIsValid(bgpioPin))
		return ERR_INVALID;

	address = gpioData.wBase + CW_GPIO_DIRECTION_SELECT_REG;
	dValue = dIoReadReg(address, REG_16);
	dValue = (dValue | ( dir << bgpioPin));
	vIoWriteReg(address, REG_16, dValue);
	return 0;
}


/*****************************************GPIO_GET_DIRECTION************************************************
        DESCRIPTION: This function is used to read direction status of particular GPIO Pin.
        INPUT: GPIO Pin Numbers     OUTPUT : Direction of GPIO Pin(Input/Output)
        STEP 1: Calculate Absolute address for GPIO Direction Register = GPIO_BASE(PCI CONFIG SPACE READ) 
                 + DIRECTION REG OFFSET(EDS Pg 493) 
        STEP 2: Read the Data from Data Register.
        STEP 3: Mask the particular gpio bit and return the status.
***********************************************************************************************************/

UINT8 bGpioGetDirection(UINT8 bgpioPin)
{
	UINT32 address,dValue = 0,val=BYTE_MASK;

	if(!bGpioIsValid(bgpioPin))
		return ERR_INVALID;

	address = gpioData.wBase + CW_GPIO_DIRECTION_SELECT_REG;
	dValue = dIoReadReg(address, REG_16);
	val = ((dValue >> bgpioPin) & 0x1);
	return val;
}


/*****************************************GPIO_ENABLE*******************************************************
        DESCRIPTION: This function is used to Enable a particular GPIO Pin.
        INPUT: GPIO Pin Numbers,Enable/Disable    OUTPUT : Error if fails
        STEP 1: Calculate Absolute address for GPIO Data Register = GPIO_BASE(PCI CONFIG SPACE READ) 
                + ENABLE REG OFFSET(EDS Pg 493) 
        STEP 2: Read the Previous Data from the Enable Register.
        STEP 3: Shift the enable bit dValue to appropriate bit for the GPIO Pin Number.
        STPE 4: Write the dValue into the Enable Register.
***********************************************************************************************************/

UINT8 vGpioEnable(UINT8 bgpioPin,UINT8 enable)
{
    UINT32 address,dValue = 0;

    if(!bGpioIsValid(bgpioPin))
    	return ERR_INVALID;

    address = gpioData.wBase + CW_GPIO_ENABLE_REG;
    dValue = dIoReadReg(address, REG_16);
    dValue = (enable ?(dValue | (1 << bgpioPin)):(dValue & ~(1 << bgpioPin)));
    vIoWriteReg(address, REG_16, dValue);
    return 0;
}


/*****************************************************************************
* vGpioBaseInit: Set GPIO base
*
* RETURNS: None
*/

void vGpioBaseInit(UINT16 base)
{
	gpioData.wBase = base;
}

void vSetSck(UINT8 val)
{
	gpio_set_val(pData.dPinClk, val ? 1 : 0);
}

void vSetMosi(UINT8 val)
{
	gpio_set_val(pData.dPinMosi, val ? 1 : 0);
}

UINT8 bGetMiso(void)
{
	return bGpioGetVal(pData.dPinMiso) ? 1 : 0;
}

void vSpiGpioChipSelect(UINT8 on)
{
	gpio_set_val(pData.dPinCs, on? 1 : 0);
}

void vSpiGpioTxByte(UINT8 data)
{
	UINT8 i,bit;
	for(i=0;i<8;i++)
	{
		vSetSck(0);
		bit = ((data << i) & MSB_MASK);
		vSetMosi(bit);
		vSpiDelay(SPI_DELAY);
		vSetSck(1);
	}
}


void vSpiGpioTxBuf(UINT8 *tx_buf,UINT8 nbytes)
{
	UINT8 byte;

	for(byte=0;byte<nbytes;byte++)
		vSpiGpioTxByte(tx_buf[byte]);	

}


/*****************************************************************************
* bSpiGpioRxByte: Obtain Rx byte
*
* RETURNS: Data
*/

UINT8 bSpiGpioRxByte(void)
{
	UINT8 data=0,i,bit;
	for(i=0;i<8;i++)
	{
		vSetSck(0);
		bit = bGetMiso();
		data = data | (bit << (7 - i));
		vSpiDelay(SPI_DELAY);
		vSetSck(1);
	}
	return data;
}


/*****************************************************************************
* vSpiGpioRxBuf: Obtain Rx buffer
*
* RETURNS: None
*/

void vSpiGpioRxBuf(UINT8 *rx_buf,UINT8 nbytes)
{
	UINT8 byte;
	for(byte=0;byte<nbytes;byte++)	
		rx_buf[byte] = bSpiGpioRxByte();
}


/*****************************************************************************
* vSpiDelay: Function for delay
*
* RETURNS: None
*/

void vSpiDelay(UINT32 usecs)
{
	vDelay(usecs*10);
}


/*****************************************************************************
* vMcp2515HwInit: Function to reset the CAN controller
*
* RETURNS: None
*/

void vMcp2515HwInit(void)
{
	vMcp2515HwReset();
#ifdef DEBUG
	vConsoleWrite("MCP2515 : HW INIT DONE\n");
#endif
}


/*********************** Reset Instruction *********************************
	 DESCRIPTION : This function is used to RESET the CAN Controller.It reinitializes
	                         all the internal registers to default value.
	 INPUT :  No params
	 OUTPUT : No return Value

     STEP 1: Assert Chip Select.
     STEP 2: Transmit "RESET INSTRUCTION" on the MOSI Pin for next 8 clock cycles.
     STEP 5: Deassert Chip Select.	

************************************************************************/
void vMcp2515HwReset(void)
{
	vSpiGpioChipSelect(0);
	vSpiGpioTxByte(INSTRUCTION_RESET);
	vSpiGpioChipSelect(1);		
#ifdef DEBUG
   	vConsoleWrite("MCP2515 : HW-RESET Done\n");
#endif
}


/**********************************READ RX BUFFER - HEADER*************************************
	 DESCRIPTION: This function is used to Read the CAN header bytes from the RX Buffer.
	 INPUT : CAN Frame structure and the Receive Buffer ID
	 OUTPUT : No Return Values.

	 STEP 1: Assert Chip Select
	 STEP 2: Trasmit "RECEIVE RX BUFFER" instruction on MOSI with Tx Buffer ID appended on next 8 clock cycles.
	 STEP 3: For continuous 40 clock cycles data from MISO is read in.(This manipulates the Header)
	 STEP 4 : Deassert Chip select once the DLC bits are clocked in.

*********************************************************************************************/

void mcp2515_hw_rx_header(struct mcp2515_frame *frame,UINT8 rx_buf_id)
{
	UINT8 rx_buf[5];

	vSpiGpioChipSelect(0);
	vSpiGpioTxByte(INSTRUCTION_READ_RXB(rx_buf_id));
	vSpiGpioRxBuf(rx_buf,5);
	vSpiGpioChipSelect(1);


#ifdef DEBUG	
	sprintf (buffer,"RXBUF HEADER VALUE:%#x\n",rx_buf[0]);
	vConsoleWrite(buffer);
	sprintf (buffer,"RXBUF HEADER VALUE:%#x\n",rx_buf[1]);
	vConsoleWrite(buffer);
	sprintf (buffer,"RXBUF HEADER VALUE:%#x\n",rx_buf[2]);
	vConsoleWrite(buffer);
	sprintf (buffer,"RXBUF HEADER VALUE:%#x\n",rx_buf[3]);
	vConsoleWrite(buffer);
	sprintf (buffer,"RXBUF HEADER VALUE:%#x\n",rx_buf[4]);
	vConsoleWrite(buffer);  
#endif

	frame->header.cdata.sid = (rx_buf[0] << 3) | (rx_buf[1] >> 5);
	frame->header.cdata.eide = (rx_buf[1] >> 3) & 0x1;
	frame->header.cdata.eid = (rx_buf[1] << 16) | (rx_buf[2] << 8) | rx_buf[3];
	frame->header.udata.rtr = (rx_buf[4] >> 6) & 0x1;
	frame->header.udata.dlc = rx_buf[4] & 0x0f;
#ifdef DEBUG
    sprintf(buffer,"CAN HEADER ID : 0x%x IDE : 0x%x EID : 0x%x rtr : 0x%x dlc : 0x%x\n",
    	frame->header.cdata.sid,frame->header.cdata.eide,frame->header.cdata.eid,
	    frame->header.udata.rtr,frame->header.udata.dlc);
    vConsoleWrite(buffer);
#endif
}


/**********************************READ RX BUFFER - DATA*************************************
	 DESCRIPTION: This function is used to READ the CAN data bytes from the RX Buffer.(Max 8 bytes)
	 INPUT : CAN Frame structure and the RECEIVER Buffer ID
	 OUTPUT : No Return Values.

	 STEP 1: Assert Chip Select
	 STEP 2: Transmit "RECEIVE RX BUFFER" instruction on MOSI with Rx Buffer ID appended on next 8 clock cycles.
	 STEP 3: Data bytes are read from MISO on continuous clock cycles(NrBytes = value of dlc).
	 STEP 4 : Deassert Chip select.

*********************************************************************************************/

void mcp2515_hw_rx_data(struct mcp2515_frame *frame, UINT8 rx_buf_id)
{
	UINT32 i;
	
	UINT8 rx_buf[12];

	vSpiGpioChipSelect(0);
	
	vSpiGpioTxByte(INSTRUCTION_READ_RXB(rx_buf_id) + 2);
	
	vSpiGpioRxBuf(rx_buf,frame->header.udata.dlc);

	vSpiGpioChipSelect(1);

	
	for(i=0; i < frame->header.udata.dlc;i++)
	{
#ifdef DEBUG             
		sprintf (buffer,"RXBUF DATA VALUE:%#x\n",rx_buf[i]);
        vConsoleWrite(buffer);
#endif        
		frame->data[i] = rx_buf[i];
	}

}


/**********************************MCP2515- RECEIVE *************************************
	 DESCRIPTION: This function is used to receive the CAN header and data bytes from the RX Buffer.
	 INPUT : RX Buf Id
	 OUTPUT : CAN Frame
*********************************************************************************************/

UINT8 mcp2515_hw_rx(struct mcp2515_frame *frame, UINT8 rx_buf_id)
{

	UINT8 rec;
	UINT8 canrxintf;
	
	rec = mcp2515_read_reg(REC);

        if (rec > 0)
	{
		return rec;
	}
	canrxintf = mcp2515_read_reg(CANINTF);
	if(canrxintf & (1 << rx_buf_id))
	{	
		mcp2515_hw_rx_header(frame,rx_buf_id);
		mcp2515_hw_rx_data(frame,rx_buf_id);
		canrxintf = (canrxintf ^ (1 << rx_buf_id));	
		mcp2515_write_reg(CANINTF,canrxintf);
	}
	else
	{
#ifdef DEBUG
		vConsoleWrite("NO RX INTERRUPT\n");	
#endif
		return ERR_RECEIVE;	
	}
	return 0;
}


/*****************************************************************************
* mcp2515_get_bit: Function to get register value in CAN controller  
*
* RETURNS: Register value
*/

UINT8 mcp2515_get_bit(UINT8 reg,UINT8 bitno)
{
	UINT8 regVal;
	regVal = mcp2515_read_reg(reg);
	return (regVal & (1 << bitno));
}


/*****************************************************************************
* mcp2515_set_bit: Function to set register in CAN controller  
*
* RETURNS: None
*/

void mcp2515_set_bit(UINT8 reg,UINT8 bitno,UINT8 val)
{
	UINT8 regVal;
	regVal = mcp2515_read_reg(reg);
	regVal = (val?(regVal | (1 << bitno)):(regVal & ~(1 << bitno)));
	mcp2515_write_reg(reg,regVal);
}


/*****************************************************************************
* mcp2515_loopbacktest: Function performing the loopback test
* RETURNS: Test status
*/

UINT32 mcp2515_loopbacktest(void)
{
	struct mcp2515_frame txframe,rxframe;
	UINT32 i;
	UINT32 error = 0;
	
	mcp2515_set_mode(MCP2515_LPBK_MODE); 

    txframe.header.udata.rtr = 0;
    txframe.header.udata.dlc = 8;

	for(i=0;i<8;i++)
		txframe.data[i] = 0xaa + i;

	mcp2515_hw_tx(&txframe,0);
	mcp2515_hw_rts(0);/*request to send*/
	mcp2515_hw_rx(&rxframe,0);

	for(i=0;i<8;i++)
		if(txframe.data[i] != rxframe.data[i])
			error++;		
        
	vIoWriteReg(0x80, REG_8, error);
	mcp2515_set_mode(MCP2515_NORMAL_MODE); /*set in Normal Mode*/
	return error;	
}


/*****************************************************************************
* mcp2515_set_mode: Function to set CAN controller in loopback mode 
*
* RETURNS: None
*/

void mcp2515_set_mode(UINT8 mode)
{
	UINT8 data;
	data = mcp2515_read_reg(CANCTRL);
	data = ((data | CANCTRL_REQOP_MASK) & (mode << 5));
	mcp2515_write_reg(CANCTRL,data);
}


/********************* Read Instruction *******************************
     DESCRIPTION : This function is used to read the CAN Controller Register.
     INPUT : Register Address (8 bits)
     OUTPUT : Register Value (8 bits)
     
     STEP 1: Assert Chip Select.
     STEP 2: Transmit "READ INSTRUCTION" on the MOSI Pin for next 8 clock cycles. 
     STEP 3: Send in the "REGISTER ADDRESS" on the MOSI Pin for next 8 clock cycles.
     STEP 4: Read the Data on MISO Pin for next 8 clock cycles.
     STEP 5: Deassert Chip Select.	

********************************************************************/

UINT8 mcp2515_read_reg(UINT8 reg)
{
	UINT8 val;
	vSpiGpioChipSelect(0);
	vSpiDelay(SPI_DELAY);
	vSpiGpioTxByte(INSTRUCTION_READ);
	vSpiGpioTxByte(reg);
	val = bSpiGpioRxByte();
	vSpiDelay(SPI_DELAY);
	vSpiGpioChipSelect(1);
	return val;
}


/********************* Write Instruction *******************************
	DESCRIPTION : This function is used to write the CAN Controller Register.
	INPUT : Register Address (8 bits) and Data to be written(8 bits)
	OUTPUT : No Return Values

	STEP 1: Assert Chip Select.
	STEP 2: Transmit "WRITE INSTRUCTION" on the MOSI Pin for next 8 clock cycles.
	STEP 3: Send in the "REGISTER" address on the MOSI Pin for 8 clock cycles. 
	STEP 4: Send in the Data on MOSI Pin for next 8 clock cycles.
	STEP 5: Deassert Chip Select.	
     
********************************************************************/

void mcp2515_write_reg(UINT8 reg,UINT8 val)
{
	vSpiGpioChipSelect(0);
	vSpiGpioTxByte(INSTRUCTION_WRITE);
	vSpiGpioTxByte(reg);
	vSpiGpioTxByte(val);
	vSpiGpioChipSelect(1);
}


/********************* Read Status Instruction**************************************
	 DESCRIPTION : This function is used to read the status of read instruction.
	 INPUT :  No Params.
	 OUTPUT : Returns the status values. See Pg 67 of MCP2515 datasheet for detailed bit desc.

     STEP 1: Assert Chip Select.
     STEP 2: Transmit "READ STATUS INSTRUCTION" on the MOSI Pin for next 8 clock cycles. 
     STEP 3: Read the status on MISO Pin for next 8 clock cycles.
     STEP 4: Deassert Chip Select.	

*****************************************************************************/

UINT8 mcp2515_read_status(void)
{
	UINT8 status;
	vSpiGpioChipSelect(0);
	vSpiDelay(SPI_DELAY);
	vSpiGpioTxByte(INSTRUCTION_READ_STATUS);
	status = bSpiGpioRxByte();
	vSpiDelay(SPI_DELAY);
	vSpiGpioChipSelect(1);
	return status;
}


/********************* Read Rx Status Instruction *****************************************
	 DESCRIPTION : This function is used to read the receive status.
	 INPUT :  No Params.
	 OUTPUT : Returns the Rx status value. See Pg 67 of MCP2515 datasheet for detailed bit desc.

     STEP 1: Assert Chip Select.
     STEP 2: Transmit "RECEIVE STATUS INSTRUCTION" on the MOSI Pin for next 8 clock cycles. 
     STEP 3: Read the status on MISO Pin for next 8 clock cycles.
     STEP 4: Deassert Chip Select.	

**************************************************************************************/

UINT8 mcp2515_read_rxstatus(void)
{
	UINT8 status;
	vSpiGpioChipSelect(0);
	vSpiDelay(SPI_DELAY);
	vSpiGpioTxByte(INSTRUCTION_RCV_STATUS);
	status = bSpiGpioRxByte();
	vSpiDelay(SPI_DELAY);
	vSpiGpioChipSelect(1);
	return status;
}


/********************************** RTS Instruction **************************************
	 DESCRIPTION : This function is used to initiate tranmission of data from the TX Buffer
	  INPUT :  Transmit Buffer ID. (Can be 0,1 or 2)
	  OUTPUT: No return value.

     STEP 1: Assert Chip Select.
     STEP 2: Transmit "REQUEST TO SEND" on the MOSI Pin for next 8 clock cycles. 
     STEP 3: Deassert Chip Select.	

********************************************************************/

void mcp2515_hw_rts(UINT8 tx_buf_id)
{
        vSpiGpioChipSelect(0);
	vSpiGpioTxByte(INSTRUCTION_REQ_SEND(tx_buf_id));
        vSpiGpioChipSelect(1);
}


/**********************************LOAD TX BUFFER - HEADER*************************************
	 DESCRIPTION: This function is used to Load the CAN header bytes into the TX Buffer.
	 INPUT : CAN Frame structure and the Transmit Buffer ID
	 OUTPUT : No Return Values.

      STEP 1: Assert Chip Select
      STEP 2: Trasmit "LOAD TX BUFFER" instruction on MOSI with Tx Buffer ID appended on next 8 clock cycles.
      STEP 3: CAN frame structure is manipulated to fit into Tx-Buffer and clocked in for next clock cycles.
      STEP 4 : Deassert Chip select once the DLC bits are clocked out.

*********************************************************************************************/
	
void mcp2515_hw_tx_header(struct mcp2515_frame *frame, UINT8 tx_buf_id)
{
#ifdef DEBUG
	UINT32 i;
#endif
	
	UINT8 tx_buf [6];
			
	vSpiGpioChipSelect(0);
	memset(&config_data, 0, sizeof(config_data));
	tx_buf[0] = INSTRUCTION_LOAD_TXB(tx_buf_id);
	tx_buf[1] = config_data.sid >> 3;
	tx_buf[2] = ((config_data.sid << 5) | (config_data.eide << 3) | (config_data.eid >> 16));
	tx_buf[3] = config_data.eid >> 8;
	tx_buf[4] = config_data.eid;
	tx_buf[5] = ((frame->header.udata.rtr << 6) | (frame->header.udata.dlc)); 

#ifdef DEBUG	
	{
		for(i=0;i<6;i++)
		{
#ifdef DEBUG
		    sprintf (buffer,"TXBUF HEADER VALUE:%#x\n",tx_buf[i]);
            vConsoleWrite(buffer);
#endif
	    }
	}
#endif

	vSpiGpioTxBuf(tx_buf,6);
	vSpiGpioChipSelect(1);

}


/**********************************LOAD TX BUFFER - DATA*************************************
	 DESCRIPTION: This function is used to Load the CAN data bytes into the TX Buffer.(Max 8 bytes)
	 INPUT : CAN Frame structure and the Transmit Buffer ID
	 OUTPUT : No Return Values.

	 STEP 1: Assert Chip Select
	 STEP 2: 	  Transmit "LOAD TX BUFFER" instruction on MOSI with Tx Buffer ID appended on next 8 clock cycles.
	 STEP 3: Data bytes are copied from the CAN Frame structure and clocked  out for next clock cycles.
	 STEP 4 : Deassert Chip select.

*********************************************************************************************/

void mcp2515_hw_tx_data(struct mcp2515_frame *frame, UINT8 tx_buf_id)
{
	UINT32 i;
	
	UINT8 tx_buf [12];

	vSpiGpioChipSelect(0);
	
	tx_buf[0] = INSTRUCTION_LOAD_TXB(tx_buf_id) + 1;
	
	for ( i = 0; i < frame->header.udata.dlc; i++)
		tx_buf[i+1] = frame->data[i];

#ifdef DEBUG	       
	for(i=1; i < frame->header.udata.dlc + 1;i++)
	{
        sprintf (buffer,"TXBUF DATA VALUE:%#x\n",tx_buf[i]);
        vConsoleWrite(buffer);
    }
#endif
    
	vSpiGpioTxBuf(tx_buf,(frame->header.udata.dlc + 1));
	vSpiGpioChipSelect(1);
	mcp2515_set_bit(TXBCTRL(tx_buf_id),TXBCTRL_TXREQ_BIT,1);
}


/**********************************MCP2515- TRANSMIT *************************************
	 DESCRIPTION: This function is used to Load the CAN header and data bytes into the TX Buffer.
	 INPUT : CAN Frame and TX Buf Id
	 OUTPUT : No Return Values.
*********************************************************************************************/

UINT8 mcp2515_hw_tx(struct mcp2515_frame *frame, UINT8 tx_buf_id)
{
	UINT8 txbctrl_status = 0,intf,txreq_clear = 0,tec,count = 0;
	
	txbctrl_status = mcp2515_read_reg(TXBCTRL(tx_buf_id));
	txreq_clear = txbctrl_status & TXREQ_CLEAR_MASK;
	
	if(!txreq_clear)
	{
		txreq_clear = txbctrl_status & TXREQ_CLEAR_MASK;
		mcp2515_write_reg(TXBCTRL(tx_buf_id),txreq_clear);
	}
	

	mcp2515_hw_tx_header(frame,tx_buf_id);
	mcp2515_hw_tx_data(frame,tx_buf_id);
	mcp2515_hw_rts(tx_buf_id);/*request to send*/

	do
	{
		intf = mcp2515_read_reg(CANINTF);
		intf = intf | (1 <(tx_buf_id +2));
		vSpiDelay(SPI_DELAY);
		count++;
	}while((!intf) || (count > 10));

	tec = mcp2515_read_reg(TEC);

	if(!intf || (tec > 0)){
#ifdef DEBUG
		vConsoleWrite("Transmit Error\n");
#endif
		return ERR_TRANSMIT;		
	}

	intf = mcp2515_read_reg(CANINTF);
	intf = intf & (~(1 <(tx_buf_id + 2)));
	mcp2515_write_reg(CANINTF,intf);

	return 0;
}


/*****************************************SPI_GPIO_INIT*****************************************************
	 DESCRIPTION: This function is used to initialise the GPIO pins for various functionalities.

	 INPUT : 	  No params
	 OUTPUT: 	  No Return Values

 STEP 1: Read the PCI Config Space Reg : 0x44 to get the GPIO Base Address
	 STEP 2: 	  Initialise the platform specific pin details and base address
 STEP 3: Enable the GPIO Pins which are in use
 STPE 4: Set Direction for each GPIO PIN 
		  CLK-OUTPUT ; MISO-INPUT ; MOSI - OUTPUT ; CS - OUTPUT 
 
***********************************************************************************************************/

void vMcp2515GpioInit( void )
{
	UINT32 dGpioBaseAddress;
	UINT8 bBus = 0, bDev = 0x1F, bFunc = 0;
	PCI_PFA pfa;
	
	pfa = PCI_MAKE_PFA(bBus, bDev, bFunc);
	dGpioBaseAddress = dPciReadReg(pfa, PCI_GPIO_BASE, REG_32);
	dGpioBaseAddress &= GPIO_BASE_MASK;

#ifdef DEBUG        
	sprintf(buffer,"GPIO Addrs: %#x\n",dGpioBaseAddress);
	vConsoleWrite(buffer);
#endif     

	/*Initialise the GPIO Base Address*/
	vGpioBaseInit(dGpioBaseAddress);

	/*Initialise Platform specific Data*/
	pData.dPinCs = MCP2515_SPI_CS;
	pData.dPinMosi = MCP2515_SPI_MOSI;
	pData.dPinMiso = MCP2515_SPI_MISO;
	pData.dPinClk = MCP2515_SPI_CLK;
	pData.dGpioBase = dGpioBaseAddress;

	/*GPIO Enable Register*/
	vGpioEnable(pData.dPinCs,1);
	vGpioEnable(pData.dPinClk,1);
	vGpioEnable(pData.dPinMosi,1);
	vGpioEnable(pData.dPinMiso,1);

	vIoWriteReg(dGpioBaseAddress + 0x04, REG_16, 0x00);
	vIoWriteReg(dGpioBaseAddress + 0x08, REG_16, 0x00);

	/*Configure GPIO as Input/Output for SPI*/
	bGpioSetDirection(pData.dPinCs, GPIO_OUTPUT); 
	bGpioSetDirection(pData.dPinMosi,GPIO_OUTPUT); 
	bGpioSetDirection(pData.dPinMiso,GPIO_INPUT); 
	bGpioSetDirection(pData.dPinClk,GPIO_OUTPUT); 
}


/*****************************************************************************
* dMcp2515RegisterAccess: test function for CAN controller access test 
*
* RETURNS: Test status
*/

UINT32 dMcp2515RegisterAccess(void)
{
	UINT8 ReadReg;
	UINT8 ReadRegSave;
	UINT8 WriteReg = 0x55;
	
	ReadReg = mcp2515_read_reg(CNF1);
	ReadRegSave = ReadReg;
	mcp2515_write_reg(CNF1,WriteReg);
	ReadReg = mcp2515_read_reg(CNF1);
	
	if (ReadReg != WriteReg)
	{
#ifdef DEBUG
	   vConsoleWrite ("Register Access Failed\n");
#endif
	   mcp2515_write_reg(CNF1,ReadRegSave);
	   return E__REG_ACCESS;
	}
	else
	{
#ifdef DEBUG
	    vConsoleWrite("Register Access PASS\n");
#endif
	    mcp2515_write_reg(CNF1,ReadRegSave);
		return E__OK;
     }
}


/*****************************************************************************
* CanCtrlAccessTest: test function for CAN controller access test 
*
* RETURNS: Test status
*/

TEST_INTERFACE (CanCtrlAccessTest, "CAN Controller Access Test")
{
	UINT32 dtest_status;

	vMcp2515GpioInit();
	vMcp2515HwInit();
	pMcpData.f_osc = MCP2515_CLK;
	dtest_status = dMcp2515RegisterAccess();

	return (dtest_status);
} /* CanCtrlAccessTest */


/*****************************************************************************
* CanCtrllbTest: test function for CAN controller access test 
*
* RETURNS: Test status
*/

TEST_INTERFACE (CanCtrllbTest, "CAN Controller Local Loop Test")
{
	UINT32 dtest_status;

	vMcp2515GpioInit();
	vMcp2515HwInit();
	pMcpData.f_osc = MCP2515_CLK;
	if(!mcp2515_loopbacktest())
	{
		dtest_status = E__OK;
    }
	else
		dtest_status = E__LocalLB;

	return (dtest_status);
} /* CanCtrllbTest */

