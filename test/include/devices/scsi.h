
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

/*
 * scsi.h
 *
 *  Created on: 7 Jun 2010
 *      Author: engineer
 */

#ifndef SCSI_H_
#define SCSI_H_

/* constant definitions (Configuration Registers) */

#define BASE_ADDRESS_ZERO		0x0010		/* I/O								*/
#define BASE_ADDRESS_ONE		0x0014		/* Memory							*/
#define RAM_BASE_ADDRESS		0x0018		/* Memory (Internal for Scripts)	*/
#define ROM_BASE_ADDRESS		0x0030		/* Memory (Expansion ROM)			*/

/* constant definitions (Identification Registers) */

#define vendor_SYMBIOS			0x1000
#define id_NCR53C810			0x0001
#define id_NCR53C825			0x0003
#define id_NCR53C860			0x0006
#define id_NCR53C875			0x000F
#define id_NCR53C895			0x000C
#define id_NCR53C896			0x000B
#define id_NCR53C895A			0x0012
#define id_NCR53C10x0			0x0021

/* constant definitions (Operational Registers) */

#define SCNTL0					0x0000
#define scntl0_ARB_mask			0xC0	/* arbitration mode					*/
#define scntl0_ARB_FULL			0xC0	/* arbitration mode is FULL			*/
#define scntl0_ARB_SIMPLE		0x00	/* arbitration mode is SIMPLE		*/
#define scntl0_START			0x20	/* start sequence					*/
#define scntl0_WATN				0x10	/* select with SATN/ on start seq.	*/
#define scntl0_EPC				0x08	/* enable parity checking			*/
#define scntl0_AAP				0x02	/* assert SATN/ on parity error		*/
#define scntl0_TRG				0x01	/* target mode						*/
#define SCNTL1					0x0001
#define scntl1_EXC				0x80	/* extra clock cycle of data setup	*/
#define scntl1_ADB				0x40	/* assert SCSI data bus				*/
#define scntl1_DHP				0x20	/* disable halt on parity error/ATN	*/
#define scntl1_CON				0x10	/* connected						*/
#define scntl1_RST				0x08	/* assert SCSI RST/ signal			*/
#define scntl1_AESP				0x04	/* assert even SCSI parity			*/
#define scntl1_IARB				0x02	/* immediate arbitration			*/
#define scntl1_SST				0x01	/* start SCSI transfer				*/
#define SCNTL2					0x0002
#define scntl2_SDU				0x80	/* SCSI disconnect unexpected		*/
#define scntl2_CHM				0x40	/* chained mode						*/
#define scntl2_SLPMD			0x20	/* SLPAR mode bit					*/
#define scntl2_SLPHBEN			0x10	/* SLPAR high byte enable			*/
#define scntl2_WSS				0x08	/* wide SCSI send					*/
#define scntl2_VUE0				0x04	/* vendor unique enhancements bit 0	*/
#define scntl2_VUE1				0x02	/* vendor unique enhancements bit 1	*/
#define scntl2_WSR				0x01	/* wide SCSI receive				*/
#define SCNTL3					0x0003
#define scntl3_ULTRA			0x80	/* ULTRA enable						*/
#define scntl3_SCF_mask			0x70	/* clock conversion factor (sync) 	*/
#define scntl3_SCF_SCLK8		0x70	/* Factor Frequency is SCLK / 8		*/
#define scntl3_SCF_SCLK6		0x60	/* Factor Frequency is SCLK / 6		*/
#define scntl3_SCF_SCLK4		0x50	/* Factor Frequency is SCLK / 4		*/
#define scntl3_SCF_SCLK3_dup	0x40	/* Factor Frequency is SCLK / 3		*/
#define scntl3_SCF_SCLK2		0x30	/* Factor Frequency is SCLK / 2		*/
#define scntl3_SCF_SCLK15		0x20	/* Factor Frequency is SCLK / 1.5	*/
#define scntl3_SCF_SCLK			0x10	/* Factor Frequency is SCLK			*/
#define scntl3_SCF_SCLK3		0x00	/* Factor Frequency is SCLK / 3		*/
#define scntl3_EWS				0x08	/* enable wide SCSI					*/
#define scntl3_CCF_mask			0x07	/* clock conversion factor			*/
#define scntl3_CCF_160			0x07	/* SCSI clock is 160 Mhz.			*/
#define scntl3_CCF_120			0x06	/* SCSI clock is 120 Mhz.			*/
#define scntl3_CCF_7501_80		0x05	/* SCSI clock is 75.01 to 80.0 Mhz.	*/
#define scntl3_CCF_5001_75_dup	0x04	/* SCSI clock is 50.01 to 75.0 Mhz.	*/
#define scntl3_CCF_3751_50		0x03	/* SCSI clock is 37.51 to 50.0 Mhz.	*/
#define scntl3_CCF_2501_375		0x02	/* SCSI clock is 25.01 to 37.5 Mhz.	*/
#define scntl3_CCF_1667_25		0x01	/* SCSI clock is 16.67 to 25.0 Mhz.	*/
#define scntl3_CCF_5001_75		0x00	/* SCSI clock is 50.01 to 75.0 Mhz.	*/

#define SCID					0x0004
#define scid_RRE				0x40	/* enable response to reselection	*/
#define scid_SRE				0x20	/* enable response to selection		*/
#define scid_ENC_mask			0x0F	/* encoded chip, SCSI identifier	*/
#define SXFER					0x0005
#define sxfer_TP_mask			0xE0	/* SCSI synchronous transfer period	*/
#define sxfer_MO_mask			0x1F	/* maximum SCSI synchronous offset	*/
#define sxfer_MO_ASYNC			0x00	/* asynchronous mode				*/
#define SDID					0x0006
#define sdid_ENC_mask			0x0F	/* encoded destination SCSI ID		*/
#define GPREG					0x0007
#define gpreg_GPIO4				0x10	/* general purpose I/O, pin 4		*/
#define gpreg_GPIO3				0x08	/* general purpose I/O, pin 3		*/
#define gpreg_GPIO2				0x04	/* general purpose I/O, pin 2		*/
#define gpreg_GPIO1				0x02	/* general purpose I/O, pin 1		*/
#define gpreg_GPIO0				0x01	/* general purpose I/O, pin 0		*/

#define SFBR					0x0008
#define SOCL					0x0009
#define socl_REQ				0x80	/* assert SCSI REQ/ signal			*/
#define socl_ACK				0x40	/* assert SCSI ACK/ signal			*/
#define socl_BSY				0x20	/* assert SCSI BSY/ signal			*/
#define socl_SEL				0x10	/* assert SCSI SEL/ signal			*/
#define socl_ATN				0x08	/* assert SCSI ATN/ signal			*/
#define socl_MSG				0x04	/* assert SCSI MSG/ signal			*/
#define socl_CD					0x02	/* assert SCSI C_D/ signal			*/
#define socl_IO					0x01	/* assert SCSI I_O/ signal			*/
#define SSID					0x000A
#define ssid_VAL				0x80	/* SCSI valid						*/
#define ssid_ENID_mask			0x0F	/* encoded destination SCSI id		*/
#define SBCL					0x000B
#define sbcl_REQ				0x80	/* SREQ/ status						*/
#define sbcl_ACK				0x40	/* SACK/ status						*/
#define sbcl_BSY				0x20	/* SBSY/ status						*/
#define sbcl_SEL				0x10	/* SSEL/ status						*/
#define sbcl_ATN				0x08	/* SATN/ status						*/
#define sbcl_MSG				0x04	/* SMSG/ status						*/
#define sbcl_CD					0x02	/* SC_D/ status						*/
#define sbcl_IO					0x01	/* SI_O/ status						*/

#define DSTAT					0x000C
#define dstat_DFE				0x80	/* DMA FIFO empty					*/
#define dstat_MDPE				0x40	/* master data parity error			*/
#define dstat_BFE				0x20	/* bus fault						*/
#define dstat_ABRT				0x10	/* aborted							*/
#define dstat_SSI				0x08	/* single step interrupt			*/
#define dstat_SIR				0x04	/* SCRIPTS interrupt received		*/
#define dstat_IID				0x01	/* illegal instruction detected		*/
#define SSTAT0					0x000D
#define sstat0_ILF				0x80	/* SIDL least significant byte full	*/
#define sstat0_ORF				0x40	/* SODR least significant byte full	*/
#define sstat0_OLF				0x20	/* SODL least signoficant byte full */
#define sstat0_AIP				0x10	/* arbitration in progress			*/
#define sstat0_LOA				0x08	/* lost arbitration					*/
#define sstat0_WOA				0x04	/* won arbitration					*/
#define sstat0_RST				0x02	/* SCSI RST/ signal					*/
#define sstat0_SDP0				0x01	/* SCSI SDP0/ parity signal			*/
#define SSTAT1					0x000E
#define sstat1_FF_mask			0xF0	/* FIFO flags						*/
#define sstat1_SDP0L			0x08	/* latched SCSI parity				*/
#define sstat1_MSG				0x04	/* SCSI MSG/ signal					*/
#define sstat1_CD				0x02	/* SCSI C_D/ signal					*/
#define sstat1_IO				0x01	/* SCSI I_O/ signal					*/
#define SSTAT2					0x000F
#define sstat2_ILF1				0x80	/* SIDL most significant byte full	*/
#define sstat2_ORF1				0x40	/* SODR most significant byte full	*/
#define sstat2_OLF1				0x20	/* SODL most significant byte full	*/
#define sstat2_FF4				0x10	/* FIFO flags, bit 4				*/
#define sstat2_SPL1				0x08	/* latched SCSI parity for SD15-8	*/
#define sstat2_DM				0x04	/* DIFFSENS mismatch				*/
#define sstat2_LDSC				0x02	/* last disconnect					*/
#define sstat2_SDP1				0x01	/* SCSI SDP1 signal					*/

#define DSA						0x0010

#define ISTAT					0x0014
#define istat_ABRT				0x80	/* abort operation					*/
#define istat_SRST				0x40	/* software reset					*/
#define istat_SIGP				0x20	/* signal process					*/
#define istat_SEM				0x10	/* semaphore						*/
#define istat_CON				0x08	/* connected						*/
#define istat_INT				0x04	/* interrupt on the fly				*/
#define istat_SIP				0x02	/* SCSI interrupt pending			*/
#define istat_DIP				0x01	/* DMA interrupt pending			*/

#define CTEST0					0x0018
#define CTEST1					0x0019
#define ctest1_FMT_mask			0xF0	/* byte empty in DMA FIFO			*/
#define ctest1_FFL_mask			0x0F	/* byte full in DMA FIFO			*/
#define CTEST2					0x001A
#define ctest2_DDIR				0x80	/* data transfer direction			*/
#define ctest2_SIGP				0x40	/* signal process					*/
#define ctest2_CIO				0x20	/* configured as I/O				*/
#define ctest2_CM				0x10	/* configured as memory				*/
#define ctest2_SRTCH			0x08	/* SCRATCH A/B operation			*/
#define ctest2_TEOP				0x04	/* SCSI true end of process			*/
#define ctest2_DREQ				0x02	/* data request status				*/
#define ctest2_DACK				0x01	/* data acknowledge status			*/
#define CTEST3					0x001B
#define ctest3_V_mask			0xF0	/* chip revision level				*/
#define ctest3_FLF				0x08	/* flush DMA FIFO					*/
#define ctest3_CLF				0x04	/* clear DMA FIFO					*/
#define ctest3_FM				0x02	/* fetch pin mode					*/
#define ctest3_WRIE				0x01	/* write and invalidate enable		*/

#define TEMP					0x001C

#define DFIFO					0x0020
#define CTEST4					0x0021
#define ctest4_BDIS				0x80	/* burst disable					*/
#define ctest4_ZMOD				0x40	/* high impedance mode				*/
#define ctest4_FBL3				0x40	/* FIFO byte control enable			*/
#define ctest4_ZSD				0x20	/* SCSI data high impedance			*/
#define ctest4_SRTM				0x10	/* shadow register test mode		*/
#define ctest4_MPEE				0x08	/* master parity error enable		*/
#define ctest4_FBL2				0x04	/* FIFO byte control enable			*/
#define ctest4_FBL_mask			0x03	/* FIFO byte control mask			*/
#define CTEST5					0x0022
#define ctest5_ADCK				0x80	/* clock address incrementor		*/
#define ctest5_BBCK				0x40	/* clock byte counter				*/
#define ctest5_DFS				0x20	/* DMA FIFO size					*/
#define ctest5_MASR				0x10	/* control for set/reset pulses		*/
#define ctest5_DDIR				0x08	/* DMA direction					*/
#define ctest5_BL				0x04	/* burst length, bit 2				*/
#define ctest5_BL_2_transfer	0x00	/* burst length,   2-transfer burst	*/
#define ctest5_BL_4_transfer	0x00	/* burst length,   4-transfer burst */
#define ctest5_BL_8_transfer	0x00	/* burst length,   8-transfer burst */
#define ctest5_BL_16_transfer	0x00	/* burst length,  16-transfer burst */
#define ctest5_BL_32_transfer	0x04	/* burst length,  32-transfer burst */
#define ctest5_BL_64_transfer	0x04	/* burst length,  64-transfer burst */
#define ctest5_BL_128_transfer	0x04	/* burst length, 128-transfer burst */
#define ctest5_BO9				0x02	/* DFIFO byte offset counter, bit 9	*/
#define ctest5_BO8				0x01	/* DFIFO byte offset counter, bit 8	*/
#define CTEST6					0x0023

#define DBC						0x0024
#define DCMD					0x0027
#define DNAD					0x0028
#define DSP						0x002C
#define DSPS					0x0030
#define SCRATCHA				0x0034

#define DMODE					0x0038
#define dmode_BL_mask			0xC0	/* burst length (with ctest5_BL2)	*/
#define dmode_BL_2_transfer		0x00	/* burst length,   2-transfer burst	*/
#define dmode_BL_4_transfer		0x40	/* burst length,   4-transfer burst	*/
#define dmode_BL_8_transfer		0x80	/* burst length,   8-transfer burst	*/
#define dmode_BL_16_transfer	0xC0	/* burst length,  16-transfer burst	*/
#define dmode_BL_32_transfer	0x00	/* burst length,  32-transfer burst	*/
#define dmode_BL_64_transfer	0x40	/* burst length,  64-transfer burst	*/
#define dmode_BL_128_transfer	0x80	/* burst length, 128-transfer burst	*/
#define dmode_SIOM				0x20	/* source I/O-memory enable			*/
#define dmode_DIOM				0x10	/* destination I/O-mamory enable	*/
#define dmode_ERL				0x08	/* enable read line					*/
#define dmode_ERMP				0x04	/* enable read multiple				*/
#define dmode_BOF				0x02	/* burst op code fetch enable		*/
#define dmode_MAN				0x01	/* manual start mode				*/
#define DIEN					0x0039
#define dien_MDPE				0x40	/* master data parity error			*/
#define dien_BF					0x20	/* bus fault						*/
#define dien_ABRT				0x10	/* aborted							*/
#define dien_SSI				0x08	/* single-step interrupt			*/
#define dien_SIR				0x04	/* SCRIPTS interrupt received		*/
#define dien_IID				0x01	/* illegal instruction detected		*/
#define SBR						0x003A
#define DCNTL					0x003B
#define dcntl_CLSE				0x80	/* cache line size enable			*/
#define dcntl_PFF				0x40	/* pre-fetch flush					*/
#define dcntl_PFEN				0x20	/* pre-fetch enable					*/
#define dcntl_SSM				0x10	/* single step mode					*/
#define dcntl_IRQM				0x08	/* IRQ mode							*/
#define dcntl_STD				0x04	/* start DMA operations				*/
#define dcntl_IRQD				0x02	/* IRQ disable						*/
#define dcntl_COM				0x01	/* 53C700 compatibility				*/

#define ADDER					0x003C

#define SIEN0					0x0040
#define sien0_MA				0x80	/* Phase mismatch/ATN report enable	*/
#define sien0_CMP				0x40	/* function complete				*/
#define sien0_SEL				0x20	/* selected							*/
#define sien0_RSL				0x10	/* reselected						*/
#define sien0_SGE				0x08	/* SCSI gross error					*/
#define sien0_UDC				0x04	/* unexpected disconnect			*/
#define sien0_RST				0x02	/* SCSI reset condition				*/
#define sien0_PAR				0x01	/* SCSI parity error				*/
#define SIEN1					0x0041
#define sien1_SBMC				0x10	/* SCSI bus mode change				*/
#define sien1_STO				0x04	/* selection or reselection timeout	*/
#define sien1_GEN				0x02	/* general purpose timer expired	*/
#define sien1_HTH				0x01	/* handshaking timer expired		*/
#define SIST0					0x0042
#define sist0_MA				0x80	/* Phase mismatch/ATN error			*/
#define sist0_CMP				0x40	/* function complete				*/
#define sist0_SEL				0x20	/* selected							*/
#define sist0_RSL				0x10	/* reselected						*/
#define sist0_SGE				0x08	/* SCSI gross error					*/
#define sist0_UDC				0x04	/* unexpected disconnect			*/
#define sist0_RST				0x02	/* SCSI RST/ received				*/
#define sist0_PAR				0x01	/* SCSI parity error				*/
#define SIST1					0x0043
#define sist1_SBMC				0x10	/* SCSI bus mode change				*/
#define sist1_STO				0x04	/* selection or reselection timeout	*/
#define sist1_GEN				0x02	/* general purpose timer expired	*/
#define sist1_HTH				0x01	/* handshaking timer expired		*/

#define SLPAR					0x0044
#define SWIDE					0x0045
#define MACNTL					0x0046
#define macntl_TYP_mask			0xF0	/* chip type						*/
#define macntl_DWR				0x08	/* dataWR (in local memory)			*/
#define macntl_DRD				0x04	/* dataRD (in local memory)			*/
#define macntl_PSCPT			0x02	/* pointer SCRIPTS (in local mem.)	*/
#define macntl_SCPTS			0x01	/* SCRIPTS (in local memory)		*/
#define GPCNTL					0x0047
#define gpcntl_ME				0x80	/* master enable					*/
#define gpcntl_FE				0x40	/* fetch enable						*/
#define gpcntl_GPIO4_EN			0x10	/* GP IO enable, pin 4				*/
#define gpcntl_GPIO3_EN			0x08	/* GP IO enable, pin 3				*/
#define gpcntl_GPIO2_EN			0x04	/* GP IO enable, pin 2				*/
#define gpcntl_GPIO1_EN			0x02	/* GP IO enable, pin 1				*/
#define gpcntl_GPIO0_EN			0x01	/* GP IO enable, pin 0				*/

#define STIME0					0x0048
#define stime0_HTH_mask			0xF0	/* handshake/handshake timer period	*/
#define stime0_SEL				0x0F	/* selection timeout				*/
#define STIME1					0x0049
#define stime1_HTHBA			0x40	/* HTH timer bus activity enable	*/
#define stime1_GENSF			0x20	/* gen purpose timer scale factor	*/
#define stime1_HTHSF			0x10	/* handshaking timer scale factor	*/
#define stime1_GEN_mask			0x0F	/* general purpose timer period		*/
#define RESPID0					0x004A
#define RESPID1					0x004B

#define STEST0					0x004C
#define stest0_SSAID_mask		0x00	/* SCSI selected as ID				*/
#define stest0_SLT				0x08	/* selection response logic test	*/
#define stest0_ART				0x04	/* arbitration priority test		*/
#define stest0_SOZ				0x02	/* SCSI synchronous offset zero		*/
#define stest0_SOM				0x01	/* SCSI synchronous offset maximum	*/
#define STEST1					0x004D
#define stest1_SCLK				0x80	/* SCSI clock select				*/
#define stest1_SISO				0x40	/* SCSI isolation test				*/
#define stest1_QEN				0x08	/* SCLK quadrupler enable			*/
#define stest1_QSEL				0x04	/* QSEL quadrupler select			*/
#define STEST2					0x004E
#define stest2_SCE				0x80	/* SCSI control enable				*/
#define stest2_ROF				0x40	/* reset SCSI offset				*/
#define stest2_DIF				0x20	/* SCSI differential mode			*/
#define stest2_SLB				0x10	/* SCSI loopback mode				*/
#define stest2_SZM				0x08	/* SCSI high-impedance mode			*/
#define stest2_AWS				0x04	/* always wide SCSI					*/
#define stest2_EXT				0x02	/* extend SREQ/SACK filtering		*/
#define stest2_LOW				0x01	/* SCSI low level mode				*/
#define STEST3					0x004F
#define stest3_TE				0x80	/* TolerANT enable					*/
#define stest3_STR				0x40	/* SCSI FIFO test read				*/
#define stest3_HSC				0x20	/* halt SCSI clock					*/
#define stest3_DSI				0x10	/* ignore single initiator response	*/
#define stest3_S16				0x08	/* 16 bit system					*/
#define stest3_TTM				0x04	/* timer test mode					*/
#define stest3_CSF				0x02	/* clear SCSI FIFO					*/
#define stest3_STW				0x01	/* SCSI FIFO test write				*/

#define SIDL					0x0050
#define STEST4					0x0052
#define stest4_SMODE_mask		0xC0	/* SCSI mode						*/
#define stest4_LOCK				0x20	/* frequency lock					*/

#define SODL					0x0054
#define CCNTL0					0x0055
#define CCNTL1					0x0056
#define	ccntl1_ZMODE			0x80	/* High impedance mode				*/

#define SBDL					0x0058
#define SCRATCHB				0x005C
#define SCRATCHC				0x0060
#define SCRATCHD				0x0064
#define SCRATCHE				0x0068
#define SCRATCHF				0x006C
#define SCRATCHG				0x0070
#define SCRATCHH				0x0074
#define SCRATCHI				0x0078
#define SCRATCHJ				0x007C



#define SCRIPT_BIST_OFFSET		0x00000F80
#define BUFFER_START			0x00060000
#define BUFFER_LENGTH			0x00020000
#define TEST_LENGTH				0x00010000


#define	BYTE_TEST_PATTERN		0x44332211
#define TIME_LIMIT          	1000



#define	E__SCSI_NO_DEVICE		E__BIT
#define E__SCSI_INIT			E__BIT  + 0x02
#define E__SCSI_NO_2_DEVICE		E__BIT  + 0x03
#define E__SCSI_MEM_ALLOC		E__BIT  + 0x04
#define E__SCSI_BYTE_ACCESS		E__BIT  + 0x05
#define E__SCSI_LONG_ACCESS		E__BIT  + 0x06
#define E__SCSI_FIFO_LANES		E__BIT  + 0x07
#define E__SCSI_INT_SETUP		E__BIT  + 0x08
#define E__SCSI_NO_INT			E__BIT  + 0x09
#define E__SCSI_DMA_FAIL		E__BIT  + 0x0A
#define E__SCSI_IF_CONTROL		E__BIT  + 0x0B
#define E__SCSI_IF_DATA			E__BIT  + 0x0C

/* type definitions */
typedef struct
{
	UINT8	scntl0;					/* SCSI Control 0					*/
	UINT8	scntl1;					/* SCSI Control 1					*/
	UINT8	scntl2;					/* SCSI Control 2					*/
	UINT8	scntl3;					/* SCSI Control 3					*/
	UINT8	scid;					/* SCSI Chip ID						*/
	UINT8	sxfer;					/* SCSI Transfer					*/
	UINT8	sdid;					/* SCSI Destination ID				*/
	UINT8	gpreg;					/* General Purpose Bits				*/
	UINT8	sfbr;					/* SCSI First Byte Received			*/
	UINT8	socl;					/* SCSI Output Control Latch		*/
	UINT8	ssid;					/* SCSI Selector ID					*/
	UINT8	sbcl;					/* SCSI Bus Control Lines			*/
	UINT8	dstat;					/* DMA Status						*/
	UINT8	sstat0;					/* SCSI Status 0					*/
	UINT8	sstat1;					/* SCSI Status 1					*/
	UINT8	sstat2;					/* SCSI Status 2					*/
	UINT32	dsa;					/* Data Structure Address			*/
	UINT8	istat;					/* Interrupt Status					*/
	UINT8	reserved_15_17 [3];		/* RESERVED FOR FUTURE USE (RFU)	*/
	UINT8	ctest0;					/* Chip Test 0 (RFU)				*/
	UINT8	ctest1;					/* Chip Test 1						*/
	UINT8	ctest2;					/* Chip Test 2						*/
	UINT8	ctest3;					/* Chip Test 3						*/
	UINT32	temp;					/* Temporary register				*/
	UINT8	dfifo;					/* DMA FIFO							*/
	UINT8	ctest4;					/* Chip Test 4						*/
	UINT8	ctest5;					/* Chip Test 5						*/
	UINT8	ctest6;					/* Chip Test 6						*/
	UINT8	dbc [3];				/* DMA Byte Counter					*/
	UINT8	dcmd;					/* DMA Command						*/
	UINT32	dnad;					/* DMA Next Address for Data		*/
	UINT32	dsp;					/* DMA SCRIPTS Pointer				*/
	UINT32	dsps;					/* DMA SCRIPTS Pointer Save			*/
	UINT32	scratchA;				/* General Purpose Scratch Pad "A"	*/
	UINT8	dmode;					/* DMA Mode							*/
	UINT8	dien;					/* DMA Interrupt Enable				*/
	UINT8	sbr;					/* Scratch Byte Register			*/
	UINT8	dcntl;					/* DMA Control						*/
	UINT32	adder;					/* sum output of internal adder		*/
	UINT8	sien0;					/* SCSI Interrupt Enable 0			*/
	UINT8	sien1;					/* SCSI Interrupt Enable 1			*/
	UINT8	sist0;					/* SCSI Interrupt Status 0			*/
	UINT8	sist1;					/* SCSI Interrupt Status 1			*/
	UINT8	slpar;					/* SCSI Longitudinal Parity			*/
	UINT8	swide;					/* SCSI Wide Residue Data			*/
	UINT8	macntl;					/* Memory Access Control			*/
	UINT8	gpcntl;					/* General Purpose Control			*/
	UINT8	stime0;					/* SCSI Timer 0						*/
	UINT8	stime1;					/* SCSI Timer 1						*/
	UINT8	respid0;				/* Response ID 0					*/
	UINT8	respid1;				/* Response ID 1					*/
	UINT8	stest0;					/* SCSI Test 0						*/
	UINT8	stest1;					/* SCSI Test 1						*/
	UINT8	stest2;					/* SCSI Test 2						*/
	UINT8	stest3;					/* SCSI Test 3						*/
	UINT16	sidl;					/* SCSI Input Data Latch			*/
	UINT8	stest4;					/* SCSI Test 4						*/
	UINT8	reserved_53;			/* RESERVED FOR FUTURE USE (RFU)	*/
	UINT16	sodl;					/* SCSI Output Data Latch			*/
	UINT8	ccntl0;					/* Chip Control 0					*/
	UINT8	ccntl1;					/* Chip Control 1					*/
	UINT16	sbdl;					/* SCSI Bus Data Lines				*/
	UINT8	reserved_5A_5B [2];		/* RESERVED FOR FUTURE USE (RFU)	*/
	UINT32	scratchB;				/* General Purpose Scratch Pad "B"	*/
	UINT32	scratchC;				/* General Purpose Scratch Pad "C"	*/
	UINT32	scratchD;				/* General Purpose Scratch Pad "D"	*/
	UINT32	scratchE;				/* General Purpose Scratch Pad "E"	*/
	UINT32	scratchF;				/* General Purpose Scratch Pad "F"	*/
	UINT32	scratchG;				/* General Purpose Scratch Pad "G"	*/
	UINT32	scratchH;				/* General Purpose Scratch Pad "H"	*/
	UINT32	scratchI;				/* General Purpose Scratch Pad "I"	*/
	UINT32	scratchJ;				/* General Purpose Scratch Pad "J"	*/
} NCR53C8X5;


typedef struct
{
	UINT32	   Instance;
	PCI_PFA	   pfa;
	PTR48      tPtr1;
	PTR48	   tPtr2;
	UINT32     mHandle;
	UINT32	   mHandle2;
	volatile   NCR53C8X5  *pDevice;
	UINT32*    dScriptAddr;

} scsiCnt;



typedef UINT32 (*TEST) (scsiCnt* scsiDev);

const UINT32 adLongPatterns [] = {0x55555555, 0xAAAAAAAA,
                                  0x55AA55AA, 0xAA55AA55};

const UINT8 bFifoLanesPattern [] = {0x01, 0x02, 0x03, 0x04,
                                    0x05, 0x06, 0x07, 0x08,
                                    0x09, 0x10, 0x11, 0x12,
                                    0x13, 0x14, 0x15, 0x16};
const UINT32	SCRIPT[] =
{
	0xc0010000, 0x00460000,	0x00470000,
	0x98080000,	0x0000000A,
	0x98080000, 0x0000000B
};


#endif /* SCSI_H_ */
