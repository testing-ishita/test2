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

/* tsi148.h - definitions for tsi148 devices
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/include/devices/tsi148.h,v 1.8 2015-03-19 15:28:05 mgostling Exp $
 * $Log: tsi148.h,v $
 * Revision 1.8  2015-03-19 15:28:05  mgostling
 * Added some additional error codes and register bit definitions.
 *
 * Revision 1.7  2015-03-17 16:55:22  mgostling
 * Added dAcquireTsi148VMEBus anddReleaseTsi148VMEBus functions to obtain exclusive access to VME Bus on the backplane and avoid bus contention and BUSERR being generated.
 *
 * Revision 1.6  2015-03-10 13:05:18  mgostling
 * Added CVS headers
 *
 *
 */


#define PCI_BYTE					0x01	
#define PCI_WORD					0x02
#define PCI_DWORD					0x03	
#define PCI_CSR						0x4		/* PCI Configuration Space Control & Status */
#define PCI_VID						0x00	/* Vendor ID offset				*/
#define PCI_VENDOR_ID				0x00	/*		-"-						*/
#define PCI_DID						0x02	/* Device ID offset				*/
#define PCI_DEVICE_ID				0x02	/*		-"-	*/	

#define VID_TUNDRA		0x10E3
#define DID_TSI148		0x0148
#define TSI148_DEVID_VENID	0x014810E3

#define CR_CSR_OFS	0x7F000	/* Base offset for CR/CSR accesses = 508kBytes */
#define VCSR_BS			0xffc

#define OTAT0		0x011C	/* PCI Slave Image 0 - Attributes			*/
#define OTSAU0		0x0100	/*		    - Base address register upper	*/
#define OTSAL0		0x0104	/*		    - Base address register lower	*/
#define OTEAU0		0x0108	/*		    - Bound address register upper	*/
#define OTEAL0		0x010C	/*		    - Bound address register lower	*/
#define OTOFU0		0x0110	/*		    - Translation offset upper		*/
#define OTOFL0		0x0114	/*		    - Translation offset lower		*/

#define OTAT1		0x013C	/* PCI Slave Image 1 - Attributes			*/
#define OTSAU1		0x0120	/*		    - Base address register upper	*/
#define OTSAL1		0x0124	/*		    - Base address register lower	*/
#define OTEAU1		0x0128	/*		    - Bound address register upper	*/
#define OTEAL1		0x012C	/*		    - Bound address register lower	*/
#define OTOFU1		0x0130	/*		    - Translation offset upper		*/
#define OTOFL1		0x0134	/*		    - Translation offset lower		*/

#define OTAT2		0x015C	/* PCI Slave Image 2 - Attributes			*/
#define OTSAU2		0x0140	/*		    - Base address register upper	*/
#define OTSAL2		0x0144	/*		    - Base address register lower	*/
#define OTEAU2		0x0148	/*		    - Bound address register upper	*/
#define OTEAL2		0x014C	/*		    - Bound address register lower	*/
#define OTOFU2		0x0150	/*		    - Translation offset upper		*/
#define OTOFL2		0x0154	/*		    - Translation offset lower		*/


#define OTAT3		0x017C	/* PCI Slave Image 3 - Attributes			*/
#define OTSAU3		0x0160	/*		    - Base address register upper	*/
#define OTSAL3		0x0164	/*		    - Base address register lower	*/
#define OTEAU3		0x0168	/*		    - Bound address register upper	*/
#define OTEAL3		0x016C	/*		    - Bound address register lower	*/
#define OTOFU3		0x0170	/*		    - Translation offset upper		*/
#define OTOFL3		0x0174	/*		    - Translation offset lower		*/

#define OTAT4		0x019C
#define OTAT5		0x01BC
#define OTAT6		0x01DC
#define OTAT7		0x01FC

#define VIACK1		0x0204
#define VIACK2		0x0208
#define VIACK3		0x020C
#define VIACK4		0x0210
#define VIACK5		0x0214
#define VIACK6		0x0218
#define VIACK7		0x021C

#define RMWAU		0x0220
#define RMWAL		0x0224
#define RMWEN		0x0228
#define RMWC		0x022C
#define RMWS		0x0230

#define VMCTRL		0x0234
	#define VMCTRL_VSA	0x08000000	/* VMEbus Stop Acknowledge			*/
	#define VMCTRL_VS	0x04000000	/* VMEbus Stop						*/
	#define VMCTRL_DHB	0x02000000	/* device has VMEbus				*/
	#define VMCTRL_DWB	0x01000000	/* device wants VMEbus				*/
	#define VMCTRL_TOFF	0x00007000	/* 64us wait between requests		*/
	#define VMCTRL_TON	0x00000700	/* 512us max time to keep bus		*/

#define VCTRL		0x0238		/* VMEbus control register				*/
	#define SFAILAI	0x00800000	/* sysfail auto slot id					*/

#define VSTAT		0x023C
	#define SCONS	0x00010000	/* system controller status				*/
	#define SYSFLS	0x00040000	/* sysfail status						*/

#define VEAU		0x0260
#define VEAL		0x0264
#define VEAT		0x0268

#define ITAT0		0x0318	/* VME Slave Image 0 - Attributes			*/
#define ITSAU0		0x0300	/*	        - Base Address Register upper	*/
#define ITSAL0		0x0304	/*	        - Base Address Register lower	*/
#define ITEAU0		0x0308	/*		    - Bound Address Register upper	*/
#define ITEAL0		0x030C	/*		    - Bound Address Register lower	*/
#define ITOFU0		0x0310	/*		    - Translation Offset upper		*/
#define ITOFL0		0x0314	/*		    - Translation Offset lower		*/

#define ITAT1		0x0338	/* VME Slave Image 1 - Attributes			*/
#define ITSAU1		0x0320	/*	        - Base Address Register upper	*/
#define ITSAL1		0x0324	/*	        - Base Address Register lower	*/
#define ITEAU1		0x0328	/*		    - Bound Address Register upper	*/
#define ITEAL1		0x032C	/*		    - Bound Address Register lower	*/
#define ITOFU1		0x0330	/*		    - Translation Offset upper		*/
#define ITOFL1		0x0334	/*		    - Translation Offset lower		*/

#define ITAT2		0x0358	/* VME Slave Image 2 - Attributes			*/
#define ITSAU2		0x0340	/*	        - Base Address Register upper	*/
#define ITSAL2		0x0344	/*	        - Base Address Register lower	*/
#define ITEAU2		0x0348	/*		    - Bound Address Register upper	*/
#define ITEAL2		0x034C	/*		    - Bound Address Register lower	*/
#define ITOFU2		0x0350	/*		    - Translation Offset upper		*/
#define ITOFL2		0x0354	/*		    - Translation Offset lower		*/

#define ITAT3		0x0378	/* VME Slave Image 3 - Attributes			*/
#define ITSAU3		0x0360	/*	        - Base Address Register upper	*/
#define ITSAL3		0x0364	/*	        - Base Address Register lower	*/
#define ITEAU3		0x0368	/*		    - Bound Address Register upper	*/
#define ITEAL3		0x036C	/*		    - Bound Address Register lower	*/
#define ITOFU3		0x0370	/*		    - Translation Offset upper		*/
#define ITOFL3		0x0374	/*		    - Translation Offset lower		*/

#define ITAT4		0x0398
#define ITAT5		0x03B8
#define ITAT6		0x03D8
#define ITAT7		0x03F8

#define CBAU		0x040C
#define CBAL		0x0410
#define CRGAT		0x0414

#define CROU		0x0418
#define CROL		0x041C
#define CRAT		0x0420

#define LMBAU		0x0424
#define LMBAL		0x0428
#define LMAT		0x042C

#define VICR		0x0440
#define INTEN		0x0448
#define INTEO		0x044C
#define INTS		0x0450
#define INTC		0x0454
#define INTM1		0x0458
#define INTM2		0x045C

#define DCTL0		0x0500
#define DCTL1		0x0580
#define  DMA_START	0x02800000		// go, single transaction
#define DCSTA0		0x0504
#define DCSTA1		0x0584
#define  DMA_ERROR	0x10000000
#define  DMA_DONE	0x02000000
#define  DMA_BUSY	0x01000000
#define DSAU0		0x0520
#define DSAU1		0x05A0
#define DSAL0		0x0524
#define DSAL1		0x05A4
#define DDAU0		0x0528
#define DDAU1		0x05A8
#define DDAL0		0x052C
#define DDAL1		0x05AC
#define DSAT0		0x0530
#define DSAT1		0x05B0
#define  DMA_SRC_VME	0x10000000
#define  DMA_SRC_BLT	0x00000100
#define  DMA_SRC_D32	0x00000040
#define  DMA_SRC_A64	0x00000004
#define DDAT0		0x0534
#define DDAT1		0x05B4
#define  DMA_DST_VME	0x10000000
#define  DMA_DST_BLT	0x00000100
#define  DMA_DST_D32	0x00000040
#define  DMA_DST_A64	0x00000004
#define DNLAU0		0x0538
#define DNLAU1		0x05B8
#define DNLAL0		0x053C
#define DNLAL1		0x05BC
#define DCNT0 		0x0540
#define DCNT1		0x05C0
#define DDBS0		0x0544
#define DDBS1		0x05C4
#define SEMAR0		0x060C
#define SEMAR1		0x0608
#define TSIMBOX0	0x0610
#define TSIMBOX1	0x0614
#define TSIMBOX2	0x0618
#define TSIMBOX3	0x061C

#define CSRBCR		0x0FF4

/* TSI148 Bit settings */
#define TSI148_VEAT_VES			0x80000000L
#define TSI148_VEAT_VESCL		0x20000000L
#define TSI148_VEAT_VES_VME		0x00000080L
#define TSI148_VEAT_VESCL_VME	0x00000020L
#define TSI148_IMAGE_EN			0x00000080L

#define	INIT_BASIC			0
#define	INIT_FULL			1

#define ID__MONARCH			0
#define ID__SLAVE			1

/* TSI148 Error Codes */
#define PCI_SUCCESS			1

#define	E__TEST_NO_DEVICE		E__BIT 
#define E__TSI148_ACCESS		E__BIT  + 0x01
#define E__TEST_NO_MEM			E__BIT  + 0x02
#define E__TEST_NO_INT			E__BIT  + 0x03
#define E__TEST_VMEDMA_ERR		E__BIT  + 0x04
#define E__TEST_VMELB_FAIL		E__BIT  + 0x05
#define E__TEST_VMEBUSS_ERR		E__BIT  + 0x06
#define E__TES_VMENMI_ERR		E__BIT  + 0x07
#define E__VME_WERR				E__BIT  + 0x08
#define E__BSWERR				E__BIT  + 0x09
#define E__BSRERR				E__BIT  + 0x0A
#define E__VME_BUS_ACQUIRE		E__BIT  + 0x0B
#define E__VME_BUS_RELEASE		E__BIT  + 0x0C
#define E__VME_SIMPLE_XFER		E__BIT  + 0x10
#define E__VME_DMA_XFER			E__BIT  + 0x20
#define E__VME_DMA_BUSY			E__BIT  + 0x30

#define TESTPAT1                0x55550000
#define TESTPAT2                0xAAAA0000
#define BSTESTPAT1				0x5A6B7C8D
#define BSTESTPAT2				0x8D7C6B5A
#define VMELEN                  0x10000

#define BYTE_SWAP( reg )	(((reg & 0x000000FF) << 24 ) | ((reg & 0x0000FF00) << 8  ) | ((reg & 0x00FF0000) >> 8  ) | ((reg & 0xFF000000) >> 24 ));	 

#define CSR_SIZE				0x00080000		/* size of CR/CSR for one slot */


#define vWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define vWriteWord(regAddr,value)	(*(UINT16*)(regAddr) = value)
#define vWriteDWord(regAddr,value)	(*(UINT32*)(regAddr) = value)

#define vReadByte(regAddr)			(*(UINT8*)(regAddr))
#define vReadWord(regAddr)			(*(UINT16*)(regAddr))
#define vReadDWord(regAddr)			(*(UINT32*)(regAddr))


//32
typedef struct sOtRegs {
    UINT32 sOTSAU;
    UINT32 sOTSAL;
    UINT32 sOTEAU;
    UINT32 sOTEAL;
    UINT32 sOTOFU;
    UINT32 sOTOFL;
    UINT32 sOTBS;
    UINT32 sOTAT;
}OtRegs;

//32
typedef struct sItRegs {
    UINT32 sITSAU;
    UINT32 sITSAL;
    UINT32 sITEAU;
    UINT32 sITEAL;
    UINT32 sITOFU;
    UINT32 sITOFL;
    UINT32 sITAT;
    UINT32 sResvd;
} ItRegs;

//76
typedef struct sDmaRegs {
    UINT32 sDCTL;
    UINT32 sDSTA;
    UINT32 sDCSAU;
    UINT32 sDCSAL;
    UINT32 sDCDAU;
    UINT32 sDCDAL;
    UINT32 sDCLAU;
    UINT32 sDCLAL;
    UINT32 sDSAU;
    UINT32 sDSAL;
    UINT32 sDDAU;
    UINT32 sDDAL;
    UINT32 sDSAT;
    UINT32 sDDAT;
    UINT32 sDNLAU;
    UINT32 sDNLAL;
    UINT32 sDCNT;
    UINT32 sDDBS;
    UINT32 sResvd[14];
} DmaRegs;


typedef struct sTsiRegs {
    UINT32 sPCI_ID;
    UINT32 sPCI_CSR;
    UINT32 sPCI_CLASS;
    UINT32 sPCI_MISC0;
    UINT32 sPCI_BSL;
    UINT32 sPCI_BSU;
    UINT32 sResvd1[9];
    UINT32 sPCI_MISC1;
    UINT32 sResvd2[48];//256

    OtRegs sotRegs[8];       /* 0x100 */

    UINT32 sResvd3;			//516

    UINT32 sVIACK[7];        /* 0x204 */ //544

    UINT32 sRMWAU;           /* 0x220 */
    UINT32 sRMWAL;
    UINT32 sRMWEN;
    UINT32 sRMWC;
    UINT32 sRMWS;

    UINT32 sVMCTRL;
    UINT32 sVCTRL;
    UINT32 sVSTAT;

    UINT32 sPCSR;            /* 0x240 */
    UINT32 sResvd4[3];
    UINT32 sVMEFL;           /* 0x250 */
    UINT32 sResvd5[3];

    UINT32 sVEAU;            /* 0x260 */
    UINT32 sVEAL;
    UINT32 sVEAT;
    UINT32 sResvd6;

    UINT32 sEDPAU;           /* 0x270 */
    UINT32 sEDPAL;
    UINT32 sEDPXA;
    UINT32 sEDPXS;
    UINT32 sEDPAT;			//644

    UINT32 sResvd7[31];		//768

    ItRegs sitRegs[8];       /* 0x300 */ //1024

    UINT32 sGBAU;            /* 0x400 */
    UINT32 sGBAL;
    UINT32 sGCSRAT;

    UINT32 sCBAU;
    UINT32 sCBAL;
    UINT32 sCRGAT;

    UINT32 sCROU;
    UINT32 sCROL;
    UINT32 sCRAT;            /* 0x420 */ //1060

    UINT32 sLMBAU;
    UINT32 sLMBAL;
    UINT32 sLMAT;
    UINT32 sV64BCU;          /* 0x430 */
    UINT32 sV64BCL;
    UINT32 sBPGTR;
    UINT32 sBPCTR;
    UINT32 sVICR;            /* 0x440 */ //1092
    UINT32 sResvd8;
    UINT32 sINTEN;
    UINT32 sINTEO;
    UINT32 sINTS;            /* 0x450 */
    UINT32 sINTC;
    UINT32 sINTM1;
    UINT32 sINTM2;			//1120 at 0x45c
    UINT32 sResvd9[40];       /* 0x460 */

    DmaRegs sdmaRegs[2];     /* 0x500 */

    UINT32 sVIDDID;          /* 0x600 */
    UINT32 sCTLSTAT;
    UINT32 sSEM0;
    UINT32 sSEM1;
    UINT32 sMBOX[4];

} TSI148Regs;



