/*******************************************************************************
*
* Filename:	 symbol.h
*
* Description:	Symbols header file.
*
* $Revision: 1.1 $
*
* $Date: 2013-09-04 07:28:50 $
*
* $Source: /home/cvs/cvsroot/CCT_BIT_2/include/symbol.h,v $
*
* Copyright 2013 Concurrent Technologies, Plc.
*
*******************************************************************************/

#ifndef _SYMBOL_TBL_H
#define _SYMBOL_TBL_H

typedef struct
{
	char* 	pAddr;		/* Pointer to symbol address */
	char 	type;		/* Symbol type */
	char* 	pName;		/* Pointer to symbol name */
	
} SYMBOL;

extern SYMBOL symTbl[];
extern UINT32 symTblSize;

#endif /* _SYMBOL_TBL_H */
