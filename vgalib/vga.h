/******************************************************************************
*
* Filename:	 vga.h
*
* Description:	VGA driver header.
*
* $Revision: 1.2 $
*
* $Date: 2013-10-08 07:21:08 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/vgalib/vga.h,v $
*
* Copyright 2012 Concurrent Technologies, Plc.
*
******************************************************************************/

#ifndef VGA_H
#define VGA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BOOL
#define BOOL	int
#endif

#ifndef TRUE
#define TRUE 	1
#endif

#ifndef FALSE
#define FALSE 	0
#endif

#ifndef FAST
#define FAST	register
#endif


/* Device definitions */

#define	VGA_MEM_BASE	(UINT8 *) 0xb8000
#define	VGA_SEL_REG		0x3d4
#define VGA_VAL_REG		0x3d5
#define	VGA_CHAR		2

/* Screen definitions */

#define	SCROLL_FORWARD		1		/* scroll direction forward */
#define	SCROLL_BACKWARD		0       /* scroll direction backward */
#define	CLRSCRN_TO_CURSOR 	0		/* clear screen to cursor */
#define	CLRSCRN_FROM_CURSOR 1		/* clear screen from cursor */
#define	CLRSCRN_ALL 		2		/* clear all the screen */
#define CURSOR_OFF    		0
#define CURSOR_ON    		1


/* Attribute  definitions */

#define ATTRIB_FG_BLACK			0x00
#define ATTRIB_FG_BLUE			0x01
#define ATTRIB_FG_GREEN			0x02
#define ATTRIB_FG_CYAN			0x03
#define ATTRIB_FG_RED			0x04
#define ATTRIB_FG_MAGENTA		0x05
#define ATTRIB_FG_BROWN			0x06
#define ATTRIB_FG_WHITE			0x07
#define ATTRIB_BRIGHT			0x08
#define ATTRIB_FG_GRAY			(ATTRIB_FG_BLACK   | ATTRIB_BRIGHT)
#define ATTRIB_FG_LIGHTBLUE		(ATTRIB_FG_BLUE    | ATTRIB_BRIGHT)
#define ATTRIB_FG_LIGHTGREEN	(ATTRIB_FG_GREEN   | ATTRIB_BRIGHT)
#define ATTRIB_FG_LIGHTCYAN		(ATTRIB_FG_CYAN    | ATTRIB_BRIGHT)
#define ATTRIB_FG_LIGHTRED		(ATTRIB_FG_RED     | ATTRIB_BRIGHT)
#define ATTRIB_FG_LIGHTMAGENTA	(ATTRIB_FG_MAGENTA | ATTRIB_BRIGHT)
#define ATTRIB_FG_YELLOW		(ATTRIB_FG_BROWN   | ATTRIB_BRIGHT)
#define ATTRIB_FG_BRIGHTWHITE	(ATTRIB_FG_WHITE   | ATTRIB_BRIGHT)
#define ATTRIB_BG_BLACK			0x00
#define ATTRIB_BG_BLUE			0x10
#define ATTRIB_BG_GREEN			0x20
#define ATTRIB_BG_CYAN			0x30
#define ATTRIB_BG_RED			0x40
#define ATTRIB_BG_MAGENTA		0x50
#define ATTRIB_BG_BROWN			0x60
#define ATTRIB_BG_WHITE			0x70
#define ATTRIB_BLINK			0x80
#define ATTRIB_CHAR_REV			0x0100

#define	INT_BLINK_MASK	(ATTRIB_BRIGHT | ATTRIB_BLINK)	/* intensity and blinking mask */

#define DEFAULT_FG		ATTRIB_FG_WHITE // ATTRIB_FG_BRIGHTWHITE
#define DEFAULT_BG		ATTRIB_BG_BLACK // ATTRIB_BG_BLUE
#define DEFAULT_ATR		(DEFAULT_FG | DEFAULT_BG)


void vgaDisplayInit( void );
void vgaSetAttr( unsigned int attr );
void vgaSetCursor( int style );
void vgaClearScreen( void );
void vgaGotoXY( int col, int row );
void vgaClearToEol( void );
void vgaClearLine( int row );

void vgaPutch( char ch );
void vgaPrinf( char *format, ... );
void vgaPuts( char *pStr );
void vgaPutchXY( int col, int row, char ch );
void vgaPrinfXY( int col, int row, char *format, ... );
void vgaPutsXY( int col, int row, char *pStr );

void vgaHorizLine( int startCol, int endCol, int row, char ch );
void vgaVertLine( int col, int startRow, char endRow, char ch );
void vgaFrame( int startCol, int endCol, int startRow, int endRow, char *frame );

#ifdef __cplusplus
}
#endif

#endif /* VGA_H */

