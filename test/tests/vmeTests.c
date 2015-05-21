/*****************************************************************************/
/*                                                                           */
/*              Copyright 2009 (C) Concurrent Technologies, Ltd.             */
/*                            All rights reserved.                           */
/*                                                                           */
/* The program below is supplied by Concurrent Technologies on the           */
/* understanding that no responsibility is assumed by Concurrent             */
/* Technologies for any errors contained therein. Furthermore, Concurrent    */
/* Technologies makes no commitment to update or keep current the program    */
/* code, and reserves the right to change its specifications at any time.    */
/*                                                                           */
/* Concurrent Technologies assumes no responsibility either for the use of   */
/* this code or for any infringements of the patent or other rights of third */
/* parties which may result from its use.                                    */
/*                                                                           */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* File Name       : vmeTests.c                                              */
/* Description     : VME Test Functions                                      */
/* Notes           :                                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#include <stdio.h>
#include <errors.h>
#include <bit/bit.h>
#include <bit/console.h>
#include <bit/io.h>
#include <bit/interrupt.h>
#include <bit/delay.h>
#include <bit/mem.h>

//#define DEBUG
#define UNIVERSE_ID          0x10E3
#define VID_TUNDRA           0x10E3
#define DEVID_UNIVERSE_2     0x0000
#define DEVID_TSI_148        0x0148
#define UNIVERSE_BASE	     0xD1000000		/* setup takes place at 1GB */
#define TESTPAT1                0x55550000
#define TESTPAT2                0xAAAA0000



#define INT_PIN_NUM          16  /* Found by experimentation was 18 hari */

#define	E__UIVERSE_NO_DEVICE		E__BIT
#define E__UIVERSE_INT_SETUP		E__BIT  + 0x02
#define E__UIVERSE_NO_INT			E__BIT  + 0x03
#define E__UIVERSE_STUCK_INT		E__BIT  + 0x04
#define E__TEST_NO_MEM			    E__BIT  + 0x05
#define E__MEMIO_RW_ERROR           E__BIT  + 0x06

#define osWriteByte(regAddr,value)	(*(UINT8*)(regAddr) = value)
#define osWriteWord(regAddr,value)  (*(UINT16*)(regAddr) = value)
#define osWriteDWord(regAddr,value)  (*(UINT32*)(regAddr) = value)

#define osReadByte(regAddr)			(*(UINT8*)(regAddr))
#define osReadWord(regAddr)			(*(UINT16*)(regAddr))
#define osReadDWord(regAddr)		(*(UINT32*)(regAddr))

UINT32  intCalled = 0 ;
UINT16  ioBase ;
UINT32	iUnivDev;					/* PCI device number for universe chip */


static PTR48 tPtr1;
static volatile UINT32 mapRegSpace=0;
static UINT32 mHandle;


UINT32 ReadWriteTest (UINT32 mapRegSpace);
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Function Name   : vmeDeviceAccessTest                                     */
/* Description     : Detects the VME Device                                  */
/* Notes           : Currently Supports the Tundra UniverseII and TSI 108    */
/*                                                                           */
/*---------------------------------------------------------------------------*/

TEST_INTERFACE (vmeDeviceAccessTest, "VME Device Access Test")
{
    PCI_PFA   pfa ;
    UINT32 regVal = 0,dRetVal;
#ifdef DEBUG
    char buffer[80];
#endif
    if (iPciFindDeviceById (1, VID_TUNDRA, DEVID_UNIVERSE_2, &pfa) == E__OK)
    {
       vConsoleWrite ("INFO: VME Device is Tundra Universe II\n") ;
		/*Enable the bus mastering and memory/io space access*/
		regVal=PCI_READ_WORD(pfa,PCI_COMMAND);
		regVal |= 0x06;
		PCI_WRITE_WORD(pfa,PCI_COMMAND,regVal);

		/*Get the memory mapped CRG*/
		regVal = PCI_READ_DWORD(pfa,PCI_BAR0);
		regVal &= 0xFFFFF000;

#ifdef DEBUG
		sprintf(buffer,"PCI CRG=%x\n",regVal);
		vConsoleWrite(buffer);
#endif

		mHandle = dGetPhysPtr(regVal,0x1000,&tPtr1,(void*)&mapRegSpace);

		if(mHandle == E__FAIL)
		{
			return E__TEST_NO_MEM;
		}

		if( (osReadDWord(mapRegSpace)) != 0x000010E3 )
		{
#ifdef DEBUG
			sprintf(buffer,"Unable to read from mapped CRG(%x):%x\n",
				mapRegSpace,osReadDWord(mapRegSpace));
			vConsoleWrite(buffer);
#endif
			vFreePtr(mHandle);
			return E__UIVERSE_NO_DEVICE;
		}

		dRetVal = ReadWriteTest (mapRegSpace);

		vFreePtr(mHandle);
        return dRetVal ;
    }

    vConsoleWrite ("ERR: Unable to Locate VME Device !!!\n") ;

    return E__UIVERSE_NO_DEVICE ;

} /* End of function vmeDeviceAccessTest () */



/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Function Name   : vmeIntHandler                                           */
/* Description     : vme Interrupt Handler                                   */
/* Notes           :                                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

static   void vmeIntHandler (void)
{
    UINT32 data ;
#if 0
    /* Read the Status Reg */
    data = dIoReadReg (ioBase + 0x304, REG_32) ;

    /* ...and make sure it's the SW Interrupt */
    if (data & 0x00002000)
    {
       intCalled = 1 ;

       /* Remove the SW Interrupt */
       data  = dIoReadReg (ioBase + 0x300, REG_32) ;
       data &= 0xffffdfff ;
       vIoWriteReg (ioBase + 0x300, REG_32, data) ;

       /* and re-Mask (i.e Mask) the Pin */
       sysMaskPin (INT_PIN_NUM, SYS_IOAPIC0) ;
    }
#endif

    /* Read the Status Reg */
    data = osReadDWord(mapRegSpace+0x304);

    /* ...and make sure it's the SW Interrupt */
    if (data & 0x00002000)
    {
       intCalled = 1 ;

       /* Remove the SW Interrupt */
       data = osReadDWord(mapRegSpace+0x300);
       data &= 0xffffdfff ;
       osWriteDWord(mapRegSpace+0x300,data);

       /* and re-Mask (i.e Mask) the Pin */
       sysMaskPin (INT_PIN_NUM, SYS_IOAPIC0) ;
    }


} /* End of function vmeIntHandler() */




/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Function Name   : universeIntTest                                         */
/* Description     : Universe-II Interrupt Test                              */
/* Notes           :                                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/

TEST_INTERFACE (universeIntTest, "Universe-II Interrupt Test")
{
    PCI_PFA pfa    ;
    UINT32  data   ;
    int     vector ;
    UINT32  loop   ;
    int     stat   ;
    UINT32 regVal;

    if (iPciFindDeviceById (1, VID_TUNDRA, DEVID_UNIVERSE_2, &pfa) != E__OK)
    {
       vConsoleWrite ("ERR: Cannot find Tundra Universe-II !!!\n") ;
       return E__UIVERSE_NO_DEVICE ;
    }

    //enable UNIVERSE-II
    PCI_WRITE_WORD (pfa, 0x04, 0x0107);


	/*Get the memory mapped CRG*/
		regVal = PCI_READ_DWORD(pfa,PCI_BAR0);
		regVal &= 0xFFFFF000;

#ifdef DEBUG
		sprintf(buffer,"PCI CRG=%x\n",regVal);
		vConsoleWrite(buffer);
#endif

		mHandle = dGetPhysPtr(regVal,0x1000,&tPtr1,(void*)&mapRegSpace);

		if(mHandle == E__FAIL)
		{
			return E__TEST_NO_MEM;
		}

		if( (osReadDWord(mapRegSpace)) != 0x000010E3 )
		{
#ifdef DEBUG
			sprintf(buffer,"Unable to read from mapped CRG(%x):%x\n",
				mapRegSpace,osReadDWord(mapRegSpace));
			vConsoleWrite(buffer);
#endif
			vFreePtr(mHandle);
			return E__UIVERSE_NO_DEVICE;
		}

		data = osReadDWord(mapRegSpace+0x304);
	    if (data)
	    	osWriteDWord (ioBase + 0x304, data) ;

#if 0
    /* Get the IO Base */
    ioBase  = (UINT16) dPciReadReg (pfa, 0x14, 2) ;
    ioBase &= 0xfffe ;

    /* Clear all interrupt status bits */
    data = dIoReadReg (ioBase + 0x304, REG_32) ;

    if (data)
       vIoWriteReg (ioBase + 0x304, REG_32, data) ;
#endif

    /* Install the Interrupt Handler but before doing that, the */
    /* vector associated with the pin needs to be obtained.     */
    if ((vector = sysPinToVector (INT_PIN_NUM, SYS_IOAPIC0)) == -1)
    {
       vConsoleWrite ("ERR: sysPinToVector (22) Failed !!!\n") ;
       return E__UIVERSE_INT_SETUP ;
    }

    if (sysInstallUserHandler (vector, vmeIntHandler) != 0)
    {
       vConsoleWrite ("ERR: sysInstallUserHandler() Failed !!!\n")  ;
       return E__UIVERSE_INT_SETUP ;
    }

    /* Now, Unmask the Pin... */
    if (sysUnmaskPin (INT_PIN_NUM, SYS_IOAPIC0) == -1)
    {
       vConsoleWrite ("ERR: sysUnmaskPin() Failed !!!\n") ;
       sysInstallUserHandler (vector, 0) ;
       return E__UIVERSE_INT_SETUP ;
    }

    /* ...and Enable Interrupts */
    sysEnableInterrupts() ;

#if 0
    /* ...and Create the SW Interrupt */
    data  = dIoReadReg (ioBase + 0x300, REG_32) ;
    data |= 0x2000 ;
    vIoWriteReg (ioBase + 0x300, REG_32, data) ;
#endif
    data = osReadDWord(mapRegSpace+0x300);
    data |= 0x2000;
    osWriteDWord(mapRegSpace+0x300,data);
    /* Wait for the Interrupt */
    loop = 0 ;
    stat = E__OK ;

    while (!intCalled)
    {
        vDelay (1) ;

        if (loop++ == 1000) /* 1 Second */
        {
           stat = E__UIVERSE_NO_INT ;
           break ;
        }
    }

    if (stat == E__UIVERSE_NO_INT)
    {
#if 0
       /* Remove the SW Interrupt */
       data  = dIoReadReg (ioBase + 0x300, REG_32) ;
       data &= 0xffffdfff ;
       vIoWriteReg (ioBase + 0x300, REG_32, data) ;
#endif
       data = osReadDWord(mapRegSpace+0x300);
       data &= 0xffffdfff ;
       osWriteDWord(mapRegSpace+0x300,data);
       /* and re-Mask (i.e Mask) the Pin */
       sysMaskPin (INT_PIN_NUM, SYS_IOAPIC0) ;
       vConsoleWrite ("ERR: Didn't Receive Interrupt !!!\n") ;
    }

    intCalled = 0 ;

    /* and Uninstall the Interrupt Handler */
     sysInstallUserHandler (vector, 0) ;

     vConsoleWrite ("Checking Stuck Interrupt\n") ;
#if 0
     /* ...and Create the SW Interrupt */
     data  = dIoReadReg (ioBase + 0x300, REG_32) ;
     data |= 0x2000 ;
     vIoWriteReg (ioBase + 0x300, REG_32, data) ;
#endif
     data = osReadDWord(mapRegSpace+0x300);
     data |= 0x2000;
     osWriteDWord(mapRegSpace+0x300,data);
     /* Wait for the Interrupt */
     loop = 0 ;
     stat = E__OK ;

     while (!intCalled)
     {
         vDelay (1) ;

         if (loop++ == 1000) /* 1 Second */
         {
            stat = E__UIVERSE_NO_INT ;
            break ;
         }
     }

     if (stat == E__UIVERSE_NO_INT)
     {
           /* Remove the SW Interrupt */
#if 0
        data  = dIoReadReg (ioBase + 0x300, REG_32) ;
        data &= 0xffffdfff ;
        vIoWriteReg (ioBase + 0x300, REG_32, data) ;
#endif
        data = osReadDWord(mapRegSpace+0x300);
        data &= 0xffffdfff ;
        osWriteDWord(mapRegSpace+0x300,data);

        /* and re-Mask (i.e Mask) the Pin */
        sysMaskPin (INT_PIN_NUM, SYS_IOAPIC0) ;
        vConsoleWrite ("STUCK Interrupt PASS\n") ;
        stat = E__OK;
     }
     else
     {
    	 intCalled = 0;
    	 vConsoleWrite ("STUCK Interrupt FAIL\n") ;
    	 stat = E__UIVERSE_STUCK_INT;
     }

     vFreePtr(mHandle);
    return stat ;

} /* End of function universeIntTest () */



/*****************************************************************************
 * ReadWriteTest: Read / write test of PCI3 slave BS register.
 *
 * RETURNS: Test passed E__OK or error code.
 */
UINT32 ReadWriteTest(UINT32 mapRegSpace)
{
	UINT32 dBackUp = 0;

	dBackUp = osReadDWord(mapRegSpace+0x140);

	osWriteDWord(mapRegSpace+0x140,TESTPAT1);
    if (osReadDWord(mapRegSpace+0x140) != TESTPAT1)
        return E__MEMIO_RW_ERROR;

    osWriteDWord(mapRegSpace+0x140,TESTPAT2);
    if (osReadDWord(mapRegSpace+0x140) != TESTPAT2)
        return E__MEMIO_RW_ERROR;

    osWriteDWord(mapRegSpace+0x140,dBackUp);
    return E__OK;
}

