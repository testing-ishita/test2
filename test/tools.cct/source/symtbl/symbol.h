/*******************************************************************************
*
* Filename:	 symbol.h
*
* Description:	Symbols header file.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 08:01:02 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/tools.cct/source/symtbl/symbol.h,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#ifndef _SYMBOL_TBL_H
#define _SYMBOL_TBL_H

typedef struct
{
	UINT32 	symAddr;	/* Symbol address */
	char 	symType;	/* Symbol type */
	char* 	symName;	/* Pointer to symbol name */
	
} SYMBOL;

#endif /* _SYMBOL_TBL_H */
