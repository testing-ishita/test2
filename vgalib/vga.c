/******************************************************************************
*
* Filename:	 vga.c
*
* Description:	Simple text mode VGA driver.
*
* $Revision: 1.2 $
*
* $Date: 2013-10-08 07:21:08 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/vgalib/vga.c,v $
*
* Copyright 2012 Concurrent Technologies, Plc.
*
******************************************************************************/

/* includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <private/port_io.h>

#include "FreeRTOS.h"
#include "vga.h"
#include "semphr.h"

#include "config.h"

#ifdef INCLUDE_DEBUG_VGA

/* VGA should not be included in release builds, for debug only */
#warning "***** VGA INCLUDED *****" 

/* defines */


/* types */

typedef struct 				/* VGA device */
{
	UINT8  *memBase;		/* Video memory base */
	UINT8  *selReg;			/* Select register */
	UINT8  *valReg;			/* Value register */
	int 	curRow;			/* Current cursor row */
	int 	curCol;			/* Current cursor column */
	UINT8 	curShape;		/* Current cursor shape */
	UINT8	curMode;		/* Current cursor mode */
	UINT8  *curChrPos;		/* Current character position */
	UINT8	curAttrib;		/* Current attribute  */
	UINT8	defAttrib;		/* Current default attribute */
	int 	numRows;		/* Current screen rows */
	int		numCols;		/* Current screen column */
	int 	scrollStart;	/* Scroll region start */
	int		scrollEnd;		/* Scroll region end */
	BOOL	autoWrap;		/* Auto wrap mode */
	BOOL	scrollCheck;	/* Scroll check */
	UINT8  *charSet;		/* Character set */
	char	tabStops[80];	/* Tab stops */

} VGA_DEV;


/* externals */
extern int vsprintf( char* achBuffer, const char* achFormat, va_list ptr );


/* globals */
UINT8 vgaFrameBlank[]  = {32,32,32,32,32,32};
UINT8 vgaFrameSingle[] = {218,196,191,179,192,217};
UINT8 vgaFrameDouble[] = {201,205,187,186,200,188};


/* locals */

static xSemaphoreHandle vgaMutex = NULL;
static BOOL vgaInitComplete = FALSE;

static VGA_DEV vgaDev;	/* VGA device data */
static VGA_DEV *pVgaDev;


/* ASCII charater set, where 0 is non-printable */
static UINT8 vgaAsciiCharSet[256] = {
/* Standard ASCII Codes */
/*		   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F */
/* 0 */	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* 1 */	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
/* 2 */	  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
/* 3 */	  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
/* 4 */	  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
/* 5 */	  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
/* 6 */	  96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
/* 7 */	 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,   0,
/* Extended ASCII Codes */	 
/* 8 */	 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
/* 9 */	 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
/* A */	 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
/* B */	 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
/* C */	 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
/* D */	 208, 209, 211, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
/* E */	 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
/* F */	 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,   0,
};


/*******************************************************************************
*	Local Functions 
*******************************************************************************/


/*******************************************************************************
*
* vgaScrollScreen
*
* Scrolls the screen according to the scroll direction, SCROLL_FORWARD or 
* SCROLL_BACKWARD
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaScrollScreen( int pos, int lines, BOOL upDn, FAST UINT8 atr )
{
	FAST UINT8 *dest;
	FAST UINT8 *src;
	
	
	if ( (pos >= pVgaDev->scrollStart) && (pos <= pVgaDev->scrollEnd) )
	{
		if (upDn)
		{
			/* Scroll forward */
			
			if (pVgaDev->scrollStart + lines > pVgaDev->scrollEnd + 1)
			{
				lines = pVgaDev->scrollEnd - pVgaDev->scrollStart + 1;
			}
			
			for (dest = pVgaDev->memBase + pVgaDev->numCols * VGA_CHAR * 
					pVgaDev->scrollStart, src = pVgaDev->memBase + pVgaDev->numCols * 
					VGA_CHAR * (pVgaDev->scrollStart + lines); src < pVgaDev->memBase + 
					pVgaDev->numCols * VGA_CHAR * (pos + 1); *dest++ = *src++ );
			
			for (dest = pVgaDev->memBase + pVgaDev->numCols * VGA_CHAR * 
					(pos - (lines - 1)); dest < pVgaDev->memBase + pVgaDev->numCols *
					VGA_CHAR * (pos + 1); dest += VGA_CHAR )
			{
				*dest     = ' ';
				*(dest+1) = atr;
			}
		}
		else
		{
			/* Scroll backward */
			
			if (pVgaDev->scrollStart + lines > pVgaDev->scrollEnd + 1)
			{
				lines = pVgaDev->scrollEnd - pVgaDev->scrollStart + 1;
			}
			
			for (dest = pVgaDev->memBase + pVgaDev->numCols * VGA_CHAR * 
					(pVgaDev->scrollEnd + 1) - 1, src = pVgaDev->memBase + 
					pVgaDev->numCols * VGA_CHAR * (pVgaDev->scrollEnd - (lines - 1)) - 1;
					src > pVgaDev->memBase + pVgaDev->numCols * VGA_CHAR * pos - 1;
					*dest-- = *src-- );
			
			for (dest = pVgaDev->memBase + pVgaDev->numCols * VGA_CHAR * 
					(pos + lines) - 1; dest > pVgaDev->memBase + pVgaDev->numCols * 
					VGA_CHAR * pos - 1; dest -= VGA_CHAR )
			{
				*dest     = atr;
				*(dest-1) = ' ';
			}
		}
	}
}


/*******************************************************************************
*
* vgaSetCursorPos
*
* Set the cursor position to the specified location.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaSetCursorPos( void )
{
	if ( pVgaDev->curCol  < 0)
	{
		pVgaDev->curCol = 0;
	}
	else if (pVgaDev->curCol >= pVgaDev->numCols)
	{
		pVgaDev->curCol = pVgaDev->numCols - 1;
	}
	
	if ( pVgaDev->curRow < pVgaDev->scrollStart)
	{
		pVgaDev->curRow = pVgaDev->scrollStart;
	}
	else if ( pVgaDev->curRow >= pVgaDev->scrollEnd )
	{
		pVgaDev->curRow = pVgaDev->scrollEnd;
	}
	
	pVgaDev->curChrPos = (pVgaDev->memBase + pVgaDev->curRow *
			    pVgaDev->numCols * VGA_CHAR + pVgaDev->curCol * VGA_CHAR );
}


/*******************************************************************************
*
* vgaEraseScreen
*
* Erase the screen with the given character from the value of position:
* CLRSCRN_TO_CURSOR 	0
* CLRSCRN_FROM_CURSOR 	1
* CLRSCRN_ALL 			2
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaEraseScreen( int position, UINT8 eraseChar )
{
	FAST UINT16 *pStart;
	FAST UINT16 *pEnd;
	FAST UINT16 erase;	/* Erase character with attribute */
	
	
	erase = (pVgaDev->defAttrib << 8) + eraseChar;
	
	if ( position == CLRSCRN_TO_CURSOR )
	{ 
		/* Clear screen from start of display to the cursor */
		pStart  = (UINT16 *) pVgaDev->memBase;
		pEnd = (UINT16 *) (pVgaDev->curChrPos + VGA_CHAR);
	}
	else if ( position == CLRSCRN_FROM_CURSOR )	
	{ 
		/* Clear screen from cursor to end of the display */
		pStart  = (UINT16 *)pVgaDev->curChrPos;
		pEnd = (UINT16 *)(pVgaDev->memBase + 2048 * VGA_CHAR);
	}
	else 
	{
		/* Clear whole screen and move cursor to 0,0 */
		pStart  = (UINT16 *) pVgaDev->memBase;
		pEnd = (UINT16 *) (pVgaDev->memBase + 2048 * VGA_CHAR);
		pVgaDev->curCol = pVgaDev->curRow = 0;
		pVgaDev->curChrPos = (UINT8 *) pVgaDev->memBase;
	}
	
	while( pStart < pEnd )
	{
		*pStart = erase;
		pStart++;		 
	}
}


/*******************************************************************************
*
* vgaCarriageReturn
*
* Do a carriage return.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaCarriageReturn( void )
{
	pVgaDev->curChrPos -= pVgaDev->curCol * VGA_CHAR;
	pVgaDev->curCol = 0;
}


/*******************************************************************************
*
* vgaTab
*
* Do a tab.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaTab( void )
{
	int	i;
	
	
	for (i = pVgaDev->curCol + 1; i < 80; i++)
	{
		if (pVgaDev->tabStops[i])
		{
			pVgaDev->curCol = i;
			break;
		}
	}
	
	if (pVgaDev->autoWrap && i >= 80)
	{
		pVgaDev->curCol = 0;
		pVgaDev->curRow++;
		pVgaDev->scrollCheck = TRUE;
	}
	
	pVgaDev->curChrPos = ( pVgaDev->memBase + 
		pVgaDev->curRow * pVgaDev->numCols * VGA_CHAR + 
		pVgaDev->curCol * VGA_CHAR);
}


/*******************************************************************************
*
* vgaLineFeed
*
* Do a line feed.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaLineFeed( void )
{
	pVgaDev->curChrPos += pVgaDev->numCols * VGA_CHAR;
	pVgaDev->curRow++;
	pVgaDev->scrollCheck = TRUE;
}


/*******************************************************************************
*
* vgaCursorOn
*
* Turn the cursor on.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaCursorOn( void )
{
	sysOutPort8( (UINT16) VGA_SEL_REG, 0x0a );
	sysOutPort8( (UINT16) VGA_VAL_REG, (pVgaDev->curShape & ~0x20) );
	sysOutPort8( (UINT16) VGA_SEL_REG, 0x0b );
	sysOutPort8( (UINT16) VGA_VAL_REG, pVgaDev->curMode );
}


/*******************************************************************************
*
* vgaCursorOff
*
* Turn the cursor off.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaCursorOff( void )
{
	sysOutPort8( (UINT16) VGA_SEL_REG, 0x0a );
	sysOutPort8( (UINT16) VGA_VAL_REG, 0x20 );
	sysOutPort8( (UINT16) VGA_SEL_REG, 0x0b );
	sysOutPort8( (UINT16) VGA_VAL_REG, 0x00 );
}


/*******************************************************************************
*
* vgaMoveCursor
*
* Move the cursor to the specified location.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaMoveCursor( FAST UINT16 pos )
{
	sysOutPort8( (UINT16)VGA_SEL_REG, 0x0e );
	sysOutPort8( (UINT16)VGA_VAL_REG, ((pos >> 8) & 0xff) );
	sysOutPort8( (UINT16)VGA_SEL_REG, 0x0f );
	sysOutPort8( (UINT16)VGA_VAL_REG, (pos & 0xff) );
}


/*******************************************************************************
*
* vgaCheckPos
*
* Check position and adjust (scroll) if necessary.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaCheckPos( void )
{
	if (pVgaDev->scrollCheck && pVgaDev->curChrPos >= 
		pVgaDev->memBase + pVgaDev->numCols * VGA_CHAR * (pVgaDev->scrollEnd + 1))
	{ 
		/* Forward scroll check */
		
		pVgaDev->curRow = pVgaDev->scrollEnd;
		
		vgaScrollScreen( pVgaDev->curRow, pVgaDev->scrollCheck,
							SCROLL_FORWARD, pVgaDev->defAttrib );
		
		while (pVgaDev->curChrPos >= pVgaDev->memBase + 
				pVgaDev->numCols * VGA_CHAR * (pVgaDev->scrollEnd + 1))
		{
			pVgaDev->curChrPos -= pVgaDev->numCols * VGA_CHAR;
		}
	}
	else if (pVgaDev->scrollCheck && pVgaDev->curChrPos < 
				pVgaDev->memBase + pVgaDev->curCol * VGA_CHAR * pVgaDev->scrollStart)
	{
		/* Backward scroll check */
		
		pVgaDev->curRow = pVgaDev->scrollStart;
		vgaScrollScreen( pVgaDev->curRow, pVgaDev->scrollCheck, 
							SCROLL_BACKWARD, pVgaDev->defAttrib );
					
		while (pVgaDev->curChrPos < pVgaDev->memBase + 
				pVgaDev->curCol * VGA_CHAR * pVgaDev->scrollStart)
		{
			pVgaDev->curChrPos += pVgaDev->numCols * VGA_CHAR;
		}
	}
	else if (pVgaDev->curChrPos > pVgaDev->memBase + 
				pVgaDev->numCols * VGA_CHAR * pVgaDev->numRows)
	{ 
		/* Out of range check (over) */
		
		while (pVgaDev->curChrPos > pVgaDev->memBase + 
				pVgaDev->numCols * VGA_CHAR * pVgaDev->numRows)
		{
			pVgaDev->curChrPos -= pVgaDev->numCols * VGA_CHAR;
			pVgaDev->curRow--;
		}
	}
	else if (pVgaDev->curChrPos < pVgaDev->memBase)
	{ 
		/* Out of range check (under) */
		
		while (pVgaDev->curChrPos < pVgaDev->memBase)
		{
			pVgaDev->curChrPos += pVgaDev->numCols * VGA_CHAR;
			pVgaDev->curRow++;
		}
	}
}	


/*******************************************************************************
*
* vgaWriteChar
*
* Write character to screen and increment position. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaWriteChar( unsigned char ch )
{
	/* If character is printable */
	if (pVgaDev->charSet[ch] != 0)
	{
		/* Write to screen */
		*(UINT16 *)pVgaDev->curChrPos = (pVgaDev->curAttrib << 8) + pVgaDev->charSet[ch];
		
		/* Increment position */
		if (pVgaDev->curCol >= pVgaDev->numCols - 1)
		{ 
			if (pVgaDev->autoWrap)
			{ 
				vgaCarriageReturn();
				vgaLineFeed();
				vgaCheckPos();
			}
		}
		else
		{
			pVgaDev->curCol++;
			pVgaDev->curChrPos += VGA_CHAR;
		}		
	}
	else
	{
		/* Handle position codes */
		switch (ch)
		{
			case '\t':		/* TAB code */
				vgaTab();
				break;
				
			case 0x0d:		/* CR code */
				vgaCarriageReturn();
				break;
				
			case '\n':		/* LF code */
				vgaCarriageReturn();
				vgaLineFeed();
				vgaCheckPos();
				break;
				
			default: break;
		}
	}
}


/*******************************************************************************
*
* vgaStatusInit
*
* Initialize the VGA Display status.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
static void vgaStatusInit( void )
{	
	/* VGA display status initialization */
	pVgaDev->memBase	   = VGA_MEM_BASE;
	pVgaDev->curAttrib     = DEFAULT_ATR;
	pVgaDev->defAttrib     = DEFAULT_ATR;
	pVgaDev->curChrPos     = pVgaDev->memBase;  /* Current  position */
	pVgaDev->numCols       = 80;                /* Number of columns */
	pVgaDev->numRows       = 25;                /* Number of text rows */
	pVgaDev->scrollStart   = 0;                 /* Scroll start */
	pVgaDev->scrollEnd     = 24;                /* Scroll end */
	pVgaDev->autoWrap      = TRUE;              /* Auto wrap mode */
	pVgaDev->scrollCheck   = FALSE;             /* Scroll flag off */
	pVgaDev->charSet       = vgaAsciiCharSet; 	/* Character set */
	
	memset( pVgaDev->tabStops, 0, sizeof(pVgaDev->tabStops) );
	
	pVgaDev->tabStops[ 0] = 1;
	pVgaDev->tabStops[ 8] = 1;
	pVgaDev->tabStops[16] = 1;
	pVgaDev->tabStops[24] = 1;
	pVgaDev->tabStops[32] = 1;
	pVgaDev->tabStops[40] = 1;
	pVgaDev->tabStops[48] = 1;
	pVgaDev->tabStops[56] = 1;
	pVgaDev->tabStops[64] = 1;
	pVgaDev->tabStops[72] = 1;
} 


#if 0
static void vgaTest( void )
{
	UINT8 achBuffer[60];
	int i;
	
	vgaFrame( 0, 79, 0, 24, vgaFrameDouble );
	vgaGotoXY( 1, 1 );
	

	for ( i = 0; i < sizeof(vgaAsciiCharSet); i++ )
	{
		sprintf( achBuffer, "vgaAsciiCharSet[%03d] %3d 0x%02x\n", i, vgaAsciiCharSet[i], vgaAsciiCharSet[i] );
		sysDebugWriteString(achBuffer);
	}
	
	for ( i = 0; i < sizeof(vgaFrameDouble); i++ )
	{
		sprintf( achBuffer, "vgaFrameDouble[%02d]  %3d 0x%02x\n", i, vgaFrameDouble[i], vgaFrameDouble[i] );
		sysDebugWriteString(achBuffer);
	}
	
	for ( i = 0; i < sizeof(vgaFrameSingle); i++ )
	{
		sprintf( achBuffer, "vgaFrameSingle[%02d]  %3d 0x%02x\n", i, vgaFrameSingle[i], vgaFrameSingle[i] );
		sysDebugWriteString(achBuffer);
	}
}
#endif

/*******************************************************************************
*	Global Functions 
*******************************************************************************/


/*******************************************************************************
*
* vgaDisplayInit
*
* Initialize the VGA display.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaDisplayInit( void )
{
	if (vgaInitComplete == FALSE)
	{
		pVgaDev = &vgaDev;
	
		vgaMutex = xSemaphoreCreateMutex();
		
		if (vgaMutex != NULL)
		{
			/* Get cursor shape and mode */
		    sysOutPort8( (UINT16) VGA_SEL_REG, 0x0a );
		    pVgaDev->curShape = sysInPort8( (UINT16) VGA_VAL_REG );
		    sysOutPort8( (UINT16) VGA_SEL_REG, 0x0b );
		    pVgaDev->curMode = sysInPort8 ((UINT16) VGA_VAL_REG );
	    
			/* Set initial status */
		    vgaStatusInit();
		
		    /* Clear screen and position cursor at 0,0 */
		    vgaEraseScreen( CLRSCRN_ALL, ' ' );
		    vgaCursorOn();
		    
		    vgaInitComplete = TRUE;
		    
		    // vgaTest();
		}
		else
		{
			vgaInitComplete = FALSE;
		}
	}
}


/*******************************************************************************
*
* vgaSetAttr
*
* Sets attributes.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaSetAttr( unsigned int attr )
{
	FAST UINT8 curAttrib;
	
	
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		curAttrib = (UINT8) (attr & 0xff);
		
		if ( attr & ATTRIB_CHAR_REV )
		{
			curAttrib = (curAttrib & INT_BLINK_MASK) | (((curAttrib >> 4) | ((curAttrib << 4) & 0x7)));
		}
		
		pVgaDev->curAttrib = curAttrib;   /* Set the attribute */
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaSetCursor
*
* Set cursor on/off.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaSetCursor( int style )
{
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		switch (style)
		{
			case CURSOR_OFF: vgaCursorOff(); break;
			
			case CURSOR_ON : vgaCursorOn(); break;
			
			default: break;
		}
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaClearScreen
*
* Clear screen and position cursor at 0,0.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaClearScreen( void )
{
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* Clear screen and position cursor at 0,0 */
		vgaEraseScreen( CLRSCRN_ALL, ' ' );
		
		vgaCheckPos();
		
		/* Move the cursor to the set position */
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaGotoXY
*
* Move cursor to position given.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaGotoXY( int col, int row )
{
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* Set cursor position */
		pVgaDev->curRow = row;
		pVgaDev->curCol = col;
		vgaSetCursorPos();
		vgaCheckPos();
		
		/* Move the cursor to the set position */
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaClearToEol
*
* Clear from current position to end of line.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaClearToEol( void )
{
	FAST UINT16 *pStart;
	FAST UINT16 *pEnd;
	FAST UINT16	erase;	/* Erase character */
	
	
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		erase = (pVgaDev->defAttrib << 8 ) + ' ';
		
		/* Set start and end positions */
		pStart  = (UINT16 *) pVgaDev->curChrPos;
		pEnd = (UINT16 *) (pVgaDev->memBase + pVgaDev->curRow  * 
							pVgaDev->numCols * VGA_CHAR + (pVgaDev->numCols - 1) * VGA_CHAR);
		
		/* Erase */					
		while( pStart < pEnd )
		{
			*pStart = erase;
			pStart++;				
		}
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaClearLine
*
* Clear given line.
* 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaClearLine( int row )
{
	FAST UINT16 *pStart;
	FAST UINT16 *pEnd;
	FAST UINT16	erase;	/* Erase character */
	
	
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		erase = (pVgaDev->defAttrib << 8 ) + ' ';
		
		/* Set the row */
		pVgaDev->curRow = row;
		vgaSetCursorPos();
		vgaCheckPos();
		
		/* Set start and end positions */
		pStart  = (UINT16 *) (pVgaDev->memBase + pVgaDev->curRow * pVgaDev->numCols * VGA_CHAR);
		pEnd = (UINT16 *) (pStart + (pVgaDev->numCols - 1) * VGA_CHAR);
		
		pVgaDev->curChrPos = (UINT8 *) pStart;
		pVgaDev->curCol = 0;
		
		/* Erase */					
		while( pStart < pEnd )
		{
			*pStart = erase;
			pStart++;				
		}
		
		/* Move the cursor to start position */
		vgaSetCursorPos();
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaPutch
*
* Write character to screen at current location. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaPutch( char ch )
{

	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* Write character */ 
		vgaWriteChar( ch );
		
		/* Move the cursor to new position */
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
	    
	    xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaPrinf
*
* Write formated string to screen at current location. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaPrinf( char *format, ... )
{
	int i;
	va_list vp;
	char achBuffer[81]; /* 1 line max */ 
	
	
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* format message */
	 	va_start(vp, format);
		vsprintf( achBuffer, (const char *)format, vp );
		va_end(vp);
		
		/* Write string */ 
		for ( i = 0; (i < 80) && (achBuffer[i] != '\0'); i++ )
		{
			vgaWriteChar( achBuffer[i] );
		}
		
		/* Move the cursor to new position */
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaPuts
*
* Write string to screen at current location. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaPuts( char *pStr )
{
	
	if ( (vgaInitComplete == TRUE) && (pStr != NULL) )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
	
		/* Write string */ 
		while ( *pStr != '\0' )
		{
			vgaWriteChar( *pStr );
			pStr++;
		}
		
		/* Move the cursor to new position */
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
	    
	    xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaPutchXY
*
* Write character to screen at given col/row. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaPutchXY( int col, int row, char ch )
{
	BOOL autoWrap;
	
	
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* Disable autoWrap */
		autoWrap = pVgaDev->autoWrap;
		pVgaDev->autoWrap = FALSE;
	
		/* Move new to position */
		pVgaDev->curRow = row;
		pVgaDev->curCol = col;
		vgaSetCursorPos();
		vgaCheckPos();
		
		/* Write character */ 
		vgaWriteChar( ch );
		
		/* Move the cursor to new position */
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
	    
	    /* Restore autoWrap */
	    pVgaDev->autoWrap = autoWrap;
	    
	    xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaPrinf
*
* Write formated string to screen at given col/row. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaPrinfXY( int col, int row, char *format, ... )
{
	int i;
	va_list vp;
	char achBuffer[81]; /* 1 line max */ 
	BOOL autoWrap;
	
	
	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* Disable autoWrap */
		autoWrap = pVgaDev->autoWrap;
		pVgaDev->autoWrap = FALSE;
	
		/* Move new to position */
		pVgaDev->curRow = row;
		pVgaDev->curCol = col;
		vgaSetCursorPos();
		vgaCheckPos();
		
		/* format message */
	 	va_start(vp, format);
		vsprintf( achBuffer, (const char *)format, vp );
		va_end(vp);
		
		/* Write string */ 
		for ( i = 0; (i < 80) && (achBuffer[i] != '\0'); i++ )
		{
			vgaWriteChar( achBuffer[i] );
		}
		
		/* Move the cursor to new position */
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
	    
	    /* Restore autoWrap */
	    pVgaDev->autoWrap = autoWrap;
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaPutsXY
*
* Write string to screen at given col/row. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaPutsXY( int col, int row, char *pStr )
{
	BOOL autoWrap;
	
	
	if ( (vgaInitComplete == TRUE) && (pStr != NULL) )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* Disable autoWrap */
		autoWrap = pVgaDev->autoWrap;
		pVgaDev->autoWrap = FALSE;
	
		/* Move new to position */
		pVgaDev->curRow = row;
		pVgaDev->curCol = col;
		vgaSetCursorPos();
		vgaCheckPos();
		
		/* Write string */ 
		while ( *pStr != '\0' )
		{
			vgaWriteChar( *pStr );
			pStr++;
			
			if (pVgaDev->curCol >= (pVgaDev->numCols - 1))
			{
				break;
			}
		}
			
		/* Move the cursor to new position */
	    vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
	    
	    /* Restore autoWrap */
	    pVgaDev->autoWrap = autoWrap;
	    
	    xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaHorizLine
*
* Draw horizontal line of characters. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaHorizLine( int startCol, int endCol, int row, char ch )
{
    int i;
    BOOL autoWrap;
    
    
    if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* Disable autoWrap */
		autoWrap = pVgaDev->autoWrap;
		pVgaDev->autoWrap = FALSE;
		
	    /* Move new to position */
		pVgaDev->curRow = row;
		pVgaDev->curCol = startCol;
		vgaSetCursorPos();
		vgaCheckPos();
	    
	    for (i = startCol; i <= endCol; i++)
	    {
			/* Write character */ 
			vgaWriteChar( ch );
			
			if (pVgaDev->curCol >= (pVgaDev->numCols - 1))
			{
				break;
			}
	    }
	    
	    /* Move the cursor to new position */
		vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
		
		/* Restore autoWrap */
		pVgaDev->autoWrap = autoWrap;
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaVertLine
*
* Draw vertical line of characters. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaVertLine( int col, int startRow, char endRow, char ch )
{
	int i;
	BOOL autoWrap;
	

	if ( vgaInitComplete == TRUE )
	{
		xSemaphoreTake( vgaMutex, portMAX_DELAY );
		
		/* Disable autoWrap */
		autoWrap = pVgaDev->autoWrap;
		pVgaDev->autoWrap = FALSE;

	    /* Move new to position */
		pVgaDev->curRow = startRow;
		pVgaDev->curCol = col;
		vgaSetCursorPos();
		vgaCheckPos();
	    
	    for (i = startRow; i <= endRow; i++)
	    {
			/* Write character */ 
			vgaWriteChar( ch );
			
			if (pVgaDev->curRow < (pVgaDev->numRows - 1))
			{
				startRow++;
				pVgaDev->curRow = startRow;
				pVgaDev->curCol = col;
				vgaSetCursorPos();
				vgaCheckPos();
			}
			else
			{
				break;
			}	
	    }
	    
	    /* Move the cursor to new position */
		vgaMoveCursor( ((pVgaDev->curChrPos - pVgaDev->memBase) / VGA_CHAR) );
		
		/* Restore autoWrap */
		pVgaDev->autoWrap = autoWrap;
		
		xSemaphoreGive( vgaMutex );
	}
}


/*******************************************************************************
*
* vgaFrame
*
* Draw frame of characters. 
* 
*
* RETURNS: N/A.
*
*******************************************************************************/
void vgaFrame( int startCol, int endCol, int startRow, int endRow, char *frame )
{
	vgaPutchXY( startCol, startRow, frame[0] );
	vgaHorizLine( (startCol + 1), (endCol - 1), startRow, frame[1] );
	vgaPutchXY( endCol, startRow, frame[2] );
	vgaVertLine( startCol, (startRow + 1), (endRow - 1), frame[3] );
	vgaVertLine( endCol, (startRow + 1), (endRow - 1), frame[3] );
	vgaPutchXY( startCol, endRow, frame[4] );
	vgaHorizLine( (startCol + 1), (endCol - 1), endRow, frame[1] );
	vgaPutchXY( endCol, endRow, frame[5] );
}

#endif /* INCLUDE_DEBUG_VGA */
