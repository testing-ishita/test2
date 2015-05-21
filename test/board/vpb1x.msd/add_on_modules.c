

#include <stdtypes.h>
#include <stdio.h>
#include <errors.h>		/* bit/errors.h ? or BIT_ERROR macro */
#include <bit/bit.h>
#include <bit/console.h>


EXTERN_INTERFACE (AhciInterruptTest)
EXTERN_INTERFACE (AhciRegisterTest)
EXTERN_INTERFACE (AhciDriveAttachTest)

#define E__PFA_DOESNT_EXIST      0x80000001

#define MARVELL_VID          0x1B4B
#define MARVELL_AHCI_DEVID   0x9235


typedef UINT32 (*FUNC_PTR) (UINT32*);

typedef struct tagAddOnModuleType
{
	UINT16      VendorId;
	UINT16      DeviceId;
	FUNC_PTR    FuncPtr;
	UINT32      Flags;
} ADDON_MODULE_TYPE;

static const ADDON_MODULE_TYPE sAddonCardTypes[] =
{
  /*  Vendor-ID             Device-ID             BIST function    Flags */
	{ MARVELL_VID,	   MARVELL_AHCI_DEVID,	  dAhciInterruptTest,   0		},
	{ MARVELL_VID,	   MARVELL_AHCI_DEVID,	   dAhciRegisterTest,   0		},
	{ MARVELL_VID,	   MARVELL_AHCI_DEVID,  dAhciDriveAttachTest,   0		},

	{ 0,		                        0,			        NULL,   0		}
};

TEST_INTERFACE (Site1Test, "PMC/XMC 1 Test")
{
	PCI_PFA	    pfa;
	UINT8       SecondaryBus;
	UINT16	    VID, DID;
	UINT16	    Site2VID, Site2DID;
	UINT16      counter = 0;
	UINT16      MaxCardsSupported = sizeof(sAddonCardTypes)/sizeof(ADDON_MODULE_TYPE);
	UINT8       SubTest = 0;
	UINT8       SubTestCount = 0;


	if ( (adTestParams[1] > 3) || (adTestParams[1] < 1) )
		adTestParams[1] = 0;

	SubTest = adTestParams[1];

	// Do we have site-1 populated?
	pfa = PCI_MAKE_PFA (0, 1, 1);

	SecondaryBus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);


	if(!SecondaryBus) return E__PFA_DOESNT_EXIST;

	pfa = PCI_MAKE_PFA (SecondaryBus, 0, 0);
	VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
	DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

	// XM600/650 specific stuff!
	if ( (VID == MARVELL_VID) && (DID == MARVELL_AHCI_DEVID) )
	{
	   // Low level ahci test expects, second param as port no.
	   // Port no. is fixed as 1 for XM600/650
	   adTestParams[2] = 1;
	}
	else
	{
		return E__PFA_DOESNT_EXIST;
	}



	// Do we have site-2 populated with MARVELL?
	pfa = PCI_MAKE_PFA (0, 1, 0);

	SecondaryBus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);
	pfa = PCI_MAKE_PFA (SecondaryBus, 0, 0);

	Site2VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
	Site2DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

	// If both the sites are populated, site-1 is instance 2 and site-2 is instance 1.
	if (  (Site2VID == MARVELL_VID) && (Site2DID == MARVELL_AHCI_DEVID) )
	{
		adTestParams[1] = 3;
	}
	else
	{
		adTestParams[1] = 2;
	}


    for (counter = 0; counter < MaxCardsSupported; counter++)
    {
       if ( (VID == sAddonCardTypes[counter].VendorId) && (DID == sAddonCardTypes[counter].DeviceId) )
       {
    		SubTestCount = SubTestCount + 1;

    		if( (SubTest == SubTestCount) || (SubTest == 0) )
    	    {
    			UINT32 ExecutionResult;

    			ExecutionResult = (*sAddonCardTypes[counter].FuncPtr)(adTestParams);

    			// Bail out at the first error.
    			if (ExecutionResult != E__OK) return ExecutionResult;
    	    }
       }
    }

	return E__OK;
}


TEST_INTERFACE (Site2Test, "PMC/XMC 2 Test")
{
	PCI_PFA	    pfa;
	UINT8       SecondaryBus;
	UINT16	    VID, DID;
	UINT16      counter = 0;
	UINT16      MaxCardsSupported = sizeof(sAddonCardTypes)/sizeof(ADDON_MODULE_TYPE);
	UINT8       SubTest = 0;
	UINT8       SubTestCount = 0;


	if ( (adTestParams[1] > 3) || (adTestParams[1] < 1) )
		adTestParams[1] = 0;

	SubTest = adTestParams[1];

	// Do we have site-2 populated?
	pfa = PCI_MAKE_PFA (0, 1, 0);

	SecondaryBus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);

	if(!SecondaryBus) return E__PFA_DOESNT_EXIST;

	pfa = PCI_MAKE_PFA (SecondaryBus, 0, 0);
	VID = PCI_READ_WORD (pfa, PCI_VENDOR_ID);
	DID = PCI_READ_WORD (pfa, PCI_DEVICE_ID);

	// XM600/650 specific stuff!
	if ( (VID == MARVELL_VID) && (DID == MARVELL_AHCI_DEVID) )
	{
	   // Low level ahci test expects, second param as port no.
	   // Port no. is fixed as 1 for XM600/650
	   adTestParams[2] = 2;
	}
	else
	{
		return E__PFA_DOESNT_EXIST;
	}


	// Do we have site-1 populated?
	pfa = PCI_MAKE_PFA (0, 1, 1);

	SecondaryBus = PCI_READ_BYTE (pfa, PCI_SEC_BUS);


	// Site-2 is always instance 1, when its own its own and when both sites are populated.
	adTestParams[1] = 2;


    for (counter = 0; counter < MaxCardsSupported; counter++)
    {
       if ( (VID == sAddonCardTypes[counter].VendorId) && (DID == sAddonCardTypes[counter].DeviceId) )
       {
   		   SubTestCount = SubTestCount + 1;

   		   if( (SubTest == SubTestCount) || (SubTest == 0) )
    	   {
				UINT32 ExecutionResult;

				ExecutionResult = (*sAddonCardTypes[counter].FuncPtr)(adTestParams);

				// Bail out at the first error.
				if (ExecutionResult != E__OK) return ExecutionResult;
    	   }
       }
    }

	return E__OK;
}
