/*******************************************************************************
*
* Filename: 	pciInt.c
* 
* Description:	PCI shared interrupt handling functions.
*
* $Revision: 1.2 $
*
* $Date: 2013-10-08 07:10:54 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/blib/pciInt.c,v $
*
* Copyright 2013 Concurrent Technologies.
*
*******************************************************************************/

#include <stdtypes.h>
#include <string.h>
#include <private/debug.h>
#include <private/spinlock.h>

#include "FreeRTOS.h"
#include "portable.h"
#include "semphr.h"


#define HEAD	node.next		/* First node in list */
#define TAIL	node.previous	/* Last node in list */

#define LL_FIRST(pList) (((LLIST *)(pList))->HEAD)
#define LL_NEXT(pNode)  (((LLIST_NODE *)(pNode))->next)    
#define LL_COUNT(pList) (((LLIST *)(pList))->count)

/* Linked List Node */
typedef struct _list_node
{
	struct _list_node *next;		/* Points at the next node in the list */
    struct _list_node *previous;	/* Points at the previous node in the list */
} LLIST_NODE;

/* Linked List Node Header */
typedef struct
{
    LLIST_NODE node;	/* Header list node */
    UINT32 count;		/* Number of nodes in list */
} LLIST;

/* PCI Interrupt Handler Linked List Node */
typedef struct
{
	LLIST_NODE node;		/* list node */
	void *handle;			/* allocation handle */
	VOIDFUNCPTR routine;	/* interrupt handler */
	UINT32 parameter;		/* parameter of the handler */
	
} PCI_INT_RTN;

/* PCI Interrupt Handler Linked List */
typedef struct
{
	LLIST list;			/* interrupt handler chain linked list */
	UINT8 *pCode;		/* pointer to ISR code wrapper */
	SPINLOCK sLock;		/* Spinlock for access protection */

} PCI_INT_LIST;


#undef INCLUDE_PCIINT_DEBUG

#ifdef INCLUDE_PCIINT_DEBUG
#warning "***** DEBUG ON *****"
#define PCIDBG(x)	do {sysDebugPrintf x;} while(0)
#else
#define PCIDBG(x)
#endif


#define PCI_INT_LINES    	32

#define IWC_INT_BASE        0
#define	IWC_INT_PARM		IWC_INT_BASE + 5	/* pciIntWrapperCode[05] */
#define	IWC_INT_FUNC		IWC_INT_PARM + 5	/* pciIntWrapperCode[10] */
#define	IWC_NUM_PARM		IWC_INT_FUNC + 6	/* pciIntWrapperCode[16] */
#define	IWC_INT_EXIT		IWC_NUM_PARM + 4	/* pciIntWrapperCode[20] */


static UINT8 pciIntWrapperCode[] =	/* Interrupt hander wrapper code template */
{
#if 0
	0xfa,							/* cli - lock out interrupts early */
#else
	0x90,							/* nop */
#endif
									/* Save registers */
	0x50,							/* pushl %eax */
	0x52,							/* pushl %edx */
	0x51,							/* pushl %ecx */
									/* Put ISR parameter on stack */
	0x68, 0x00, 0x00, 0x00, 0x00,	/* ISR parameter filled in at runtime */
									/* Call ISR function */
	0xe8, 0x00, 0x00, 0x00, 0x00,	/* ISR function filled in at runtime */
									/* Remove parameter from stack */
	0x83, 0xc4, 0x04,				/* pop parameter */
									/* Restore registers */
	0x59,							/* popl	%ecx */
	0x5a,							/* popl	%edx */
	0x58,							/* popl	%eax */
									/* Return to caller */
	0xc3,							/* rtn */
};

static PCI_INT_LIST pciIntList[PCI_INT_LINES];  /* Linked list of interrupt handlers */
static int pciIntInitStatus = FALSE;			/* Initialization status */
static xSemaphoreHandle pciIntMutex = NULL;		/* Access protection mutex */


#ifdef INCLUDE_PCIINT_DEBUG

/*******************************************************************************
*
* pciShowIsrCode
*
* Show given ISR wrapper code details.
*
*
* RETURNS: None
* 
*******************************************************************************/
static void pciShowIsrCode( UINT8 *pCode )
{
	PCIDBG(("pCode: 0x%x\n", (UINT32) pCode));
	
	if (pCode != NULL)
	{
/*
		PCIDBG(("%02d 0x%02x\n", IWC_INT_BASE, pCode[IWC_INT_BASE]));
		PCIDBG(("%02d 0x%02x\n", (IWC_INT_BASE + 1),pCode[1]));
		PCIDBG(("%02d 0x%02x\n", (IWC_INT_BASE + 2),pCode[2]));
		PCIDBG(("%02d 0x%02x\n", (IWC_INT_BASE + 3),pCode[3]));
*/		
		
		PCIDBG(("%02d 0x%02x parameter    : 0x%x\n", 
				(IWC_INT_PARM - 1), pCode[IWC_INT_PARM - 1],*((UINT32*)&pCode[IWC_INT_PARM])));
		PCIDBG(("%02d 0x%02x routine      : 0x%x = 0x%x\n",
				(IWC_INT_FUNC - 1), pCode[IWC_INT_FUNC - 1],*((UINT32*)&pCode[IWC_INT_FUNC]),
				((UINT32)&pCode[IWC_INT_FUNC + 4] + *(UINT32 *)&pCode[IWC_INT_FUNC])));
		
/*		
		PCIDBG(("%02d 0x%02x 0x%02x 0x%02x\n", (IWC_NUM_PARM - 2),pCode[IWC_NUM_PARM - 2],
				pCode[IWC_NUM_PARM - 1],pCode[IWC_NUM_PARM]));
		PCIDBG(("%02d 0x%02x\n", (IWC_NUM_PARM + 1),pCode[IWC_NUM_PARM + 1]));
		PCIDBG(("%02d 0x%02x\n", (IWC_NUM_PARM + 2),pCode[IWC_NUM_PARM + 2]));
		PCIDBG(("%02d 0x%02x\n", (IWC_NUM_PARM + 3),pCode[IWC_NUM_PARM + 3]));
		PCIDBG(("%02d 0x%02x\n", IWC_INT_EXIT,pCode[IWC_INT_EXIT]));
*/
	}
}

#endif /* INCLUDE_PCIINT_DEBUG */


/*******************************************************************************
*
* pciIntListInit
*
* This function initializes the given list as an empty list.
* 
*
* RETURNS: TRUE if successful else FALS.
*
*******************************************************************************/
static int pciIntListInit( LLIST *pList )
{
	if (pList != NULL)
	{
		pList->HEAD	 = NULL;
		pList->TAIL  = NULL;
		pList->count = 0;
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*******************************************************************************
*
* pciIntListAddNode
*
* This function adds the give node to the end of the list.
* The new node is placed following the list node <pPrev>.
* If <pPrev> is NULL, the node is inserted at the head of the list. 
*
* RETURNS: TRUE if successful else FALSE.
*
*******************************************************************************/
static int pciIntListAddNode( LLIST *pList, LLIST_NODE *pNode )
{
	LLIST_NODE *pNext;
	LLIST_NODE *pPrev;


	if (pList != NULL)
	{
		pPrev = pList->TAIL;
		
		if (pPrev == NULL)
		{	
			/* New node is to be first in list */
			pNext = pList->HEAD;
			pList->HEAD = pNode;
		}
		else
		{
			/* Make prev node point fwd to new */
			pNext = pPrev->next;
			pPrev->next = pNode;
		}

		if (pNext == NULL)
		{
    		/* New node is to be last in list */
			pList->TAIL = pNode;
		}
		else
		{
			/* Make next node point back to new */
			pNext->previous = pNode;
		}

		/* Set pointers in new node, and update node count */
		pNode->next		= pNext;
		pNode->previous	= pPrev;

		pList->count++;
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*******************************************************************************
*
* pciIntHandlerCreate
*
* This function builds an ISR code wrapper for the specified C routine.
* This ISR code can then be connected to a specific vector address.
*
*
* RETURNS: a pointer to the newly created ISR code or NULL.
* 
*******************************************************************************/
static UINT8 *pciIntHandlerCreate( VOIDFUNCPTR routine, UINT32 parameter )
{
	UINT8 *pCode;
	
	
	/* Allocate memory for new ISR wrapper code */
	pCode = (UINT8 *) pvPortMalloc ( sizeof (pciIntWrapperCode) );
	
	if (pCode != NULL)
	{
		/* Copy ISR wrapper code template into new memory area */
		bcopy ((char *)pciIntWrapperCode, (char *)pCode, sizeof(pciIntWrapperCode));
		
		/* Setup wrapper code for our ISR */
		*(UINT32 *)&pCode[IWC_INT_PARM] = parameter;
		*(UINT32 *)&pCode[IWC_INT_FUNC] = (UINT32)routine -
						(UINT32)&pCode[IWC_INT_FUNC + 4];
						
#ifdef INCLUDE_PCIINT_DEBUG
		pciShowIsrCode( pCode );
#endif
	
	}
	else
	{
		PCIDBG(("%s: Error - memory allocation failed\n", __FUNCTION__));
	}
	
	return pCode;
}
    

/*******************************************************************************
*
* pciIntIsr
*
* This function executes multiple interrupt handlers for a PCI interrupt.
* Each interrupt handler is within the linked list for the given PCI interrupt.  
*
*
* RETURNS: None
* 
*******************************************************************************/
static void pciIntIsr( UINT32 irqNo )
{
    PCI_INT_RTN *pRtn;

	if (irqNo < PCI_INT_LINES)
	{
		portENTER_CRITICAL( &pciIntList[irqNo].sLock, pdTRUE );
		
	    for (pRtn = (PCI_INT_RTN *)LL_FIRST (&pciIntList[irqNo].list); pRtn != NULL;
	        	pRtn = (PCI_INT_RTN *)LL_NEXT (&pRtn->node))
	    {
	        (* pRtn->routine) (pRtn->parameter);
	    }
	    
	    portEXIT_CRITICAL( &pciIntList[irqNo].sLock, pdTRUE );
	}
}


/*******************************************************************************
*
* pciIntIntall
*
* This function install the ISR for a PCI interrupt at the appropriate vector.
* The ISR should only be installed once.
*
*
* RETURNS: TRUE if successful else FALSE.
* 
*******************************************************************************/
static int pciIntIntall( UINT8 irqNo )
{
	UINT8 *pCode;
	int vector;
	int result = FALSE;
	
	
	/* Check ISR code is not already installed */
	if ( (LL_COUNT( &pciIntList[irqNo].list ) == 0) &&
			(pciIntList[irqNo].pCode == NULL) )
	{
		vector = sysPinToVector( irqNo, SYS_IOAPIC0 );
		
		if (vector > 0)
		{
			/* Disable the interrupt */
			sysMaskPin( irqNo, SYS_IOAPIC0 );
			
			/* Create our wrapper code */
			pCode = (UINT8 *) pciIntHandlerCreate( (VOIDFUNCPTR) pciIntIsr, (UINT32) irqNo );
			
			if (pCode != NULL)
			{
				/* Install the ISR code in the vector */
				if (sysInstallUserHandler( vector, (VOIDFUNCPTR) pCode ) == 0)
				{
					/* Enable the interrupt */
					if (sysUnmaskPin( irqNo, SYS_IOAPIC0 ) == 0)
					{
						PCIDBG(("%s: IRQ%u enabled, using Vector: 0x%x\n", __FUNCTION__, irqNo, vector));
						
						/* Remember code address so it can be freed */
						pciIntList[irqNo].pCode = pCode;
						
						result = TRUE;
					}
					else
					{
						PCIDBG(("%s: Error - failed to enable IRQ%u\n", __FUNCTION__, irqNo));
						sysInstallUserHandler( vector, 0 );
						vPortFree( pCode );
					}
				}
				else
				{
					PCIDBG(("%s: Error - failed to install ISR\n", __FUNCTION__));
					vPortFree( pCode );
				}
			}
		}
		else
		{
			PCIDBG(("%s: Error - invalid vector\n", __FUNCTION__));
		}
	}
	else
	{
		/* ISR already installed */
		result = TRUE;
	}
	
	return result;
}


/*******************************************************************************
*
* pciIntConnect
*
* This function connects an interrupt handler to a shared PCI interrupt vector.
* A linked list is created for each shared interrupt in the system, 
* pciIntConnect adds the given routine to the linked list for the IRQ.   
*
*
* RETURNS: TRUE if successful else FALSE.
* 
*******************************************************************************/
int pciIntConnect( UINT8 irqNo, VOIDFUNCPTR routine, UINT32 parameter )
{
	PCI_INT_RTN *pRtn;
	int result = FALSE;
	
	
	PCIDBG(("%s: IRQ: %u routine: 0x%x parameter: 0x%x\n", 
			__FUNCTION__, irqNo, (UINT32) routine, parameter));
	
	if ( (pciIntInitStatus == TRUE) && (irqNo < PCI_INT_LINES) )
	{
		xSemaphoreTake( pciIntMutex, portMAX_DELAY );
		
		/* Install the PCI ISR */
		result = pciIntIntall( irqNo );

		if ( result )
		{
			/* Add our routine to the interrupt handler chain linked list */
			
			pRtn = (PCI_INT_RTN *) pvPortMalloc( sizeof (PCI_INT_RTN) );
			
			if (pRtn != NULL)
			{
				pRtn->handle	= (void *) pRtn; /* remember so we can be freed */
				pRtn->routine   = routine;
	    		pRtn->parameter = parameter;
	    		
	    		portENTER_CRITICAL( &pciIntList[irqNo].sLock, pdFALSE );
	    		result = pciIntListAddNode( &pciIntList[irqNo].list, &pRtn->node );
	    		portEXIT_CRITICAL( &pciIntList[irqNo].sLock, pdFALSE );
	    		
	    		if ( result )
	    		{
	    			PCIDBG(("%s: pciIntList[%u] count: %u\n", __FUNCTION__, 
								irqNo, LL_COUNT( &pciIntList[irqNo].list )));
								
					PCIDBG(("handle   : 0x%x\n", (UINT32) pRtn->handle));
					PCIDBG(("routine  : 0x%x\n", (UINT32) pRtn->routine));
					PCIDBG(("parameter: 0x%x\n", pRtn->parameter));
	    		}
	    		else
	    		{
					PCIDBG(("%s: Error - failed to add node\n", __FUNCTION__));
					vPortFree( pRtn );
					result = FALSE;
				}    		
			}
			else
			{
				PCIDBG(("%s: Error - memory allocation failed\n", __FUNCTION__));
			}
		}
		
		xSemaphoreGive( pciIntMutex );
	}
	
	return result;
} 


#ifdef INCLUDE_PCIINT_DEBUG
/*******************************************************************************
*
* pciIntListNthNode
*
* This function returns a pointer to the node specified by a number <nodenum>
* where the first node in the list is numbered 1.
* 
*
* RETURNS: A pointer to the Nth node, or NULL if there is no Nth node.
*
*******************************************************************************/
static LLIST_NODE *pciIntListNthNode( LLIST *pList, int nodenum )
{
    LLIST_NODE *pNode;


    /* Verify node number is in list */
    if ((pList == NULL) || (nodenum < 1) || (nodenum > pList->count))
    {
		return NULL;
	}
	else
	{
		pNode = pList->HEAD;
		
		while (--nodenum > 0)
		{
		    pNode = pNode->next;
		}
		
	    return pNode;
	}
}


/*******************************************************************************
*
* pciIntListShow
*
* This function displays the linked list for the given interrupt.
*  
*
*
* RETURNS: None
* 
*******************************************************************************/
void pciIntListShow( UINT8 irqNo )
{
	int i;
	int count;
	PCI_INT_RTN *pRtn;
	
	
	if ( (pciIntInitStatus == TRUE) && (irqNo < PCI_INT_LINES) )
	{
		xSemaphoreTake( pciIntMutex, portMAX_DELAY );
		
		count = (int) LL_COUNT( &pciIntList[irqNo].list );
		
		PCIDBG(("%s: pciIntList[%u] count: %u\n", __FUNCTION__, irqNo, count));
		PCIDBG(("pCode: 0x%x\n", (UINT32) pciIntList[irqNo].pCode));
		
		for ( i = 1; i <= count; i++ )
		{
			pRtn = (PCI_INT_RTN *) pciIntListNthNode( &pciIntList[irqNo].list, i );
			
			if (pRtn != NULL)
			{
				PCIDBG(("%02d: handle   : 0x%x\n", i, (UINT32) pRtn->handle));
				PCIDBG(("%02d: routine  : 0x%x\n", i, (UINT32) pRtn->routine));
				PCIDBG(("%02d: parameter: 0x%x\n", i, pRtn->parameter));
			}
		}
		
		xSemaphoreGive( pciIntMutex );
	}
}
#endif /* INCLUDE_PCIINT_DEBUG */


/*******************************************************************************
*
* pciIntInit
*
* This function performs the initialization required for PCI interrupt handling.
*  
*
*
* RETURNS: None
* 
*******************************************************************************/
void pciIntInit( void )
{
	int i;
	
	
	PCIDBG(("%s:\n", __FUNCTION__));
	
	 if (pciIntInitStatus == FALSE)
	 {
	    /* Initialize shared interrupt handler lists */
	    for (i = 0; i < PCI_INT_LINES; i++)
	    {
	        pciIntListInit(&pciIntList[i].list);
	        pciIntList[i].pCode = NULL;
	        spinLockInit( &pciIntList[i].sLock );
	    }
	    
	    pciIntMutex = xSemaphoreCreateMutex();
	    
	    if (pciIntMutex != NULL)
	    {
	    	pciIntInitStatus = TRUE;
	    }
	}
}

