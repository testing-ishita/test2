
# ************************************************************************
# *                                                                      *
# *      Copyright 2008 Concurrent Technologies, all rights reserved.    *
# *                                                                      *
# *      The program below is supplied by Concurrent Technologies        *
# *      on the understanding that no responsibility is assumed by       *
# *      Concurrent Technologies for any errors contained therein.       *
# *      Furthermore, Concurrent Technologies makes no commitment to     *
# *      update or keep current the program code, and reserves the       *
# *      right to change its specifications at any time.                 *
# *                                                                      *
# *      Concurrent Technologies assumes no responsibility either for    *
# *      the use of this code or for any infringements of the patent     *
# *      or other rights of third parties which may result from its use  *
# *                                                                      *
# ************************************************************************

# MAKEFILE - CLIB build (C-Library)
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/clib/makefile,v 1.1 2013-09-04 07:17:58 chippisley Exp $
#
# $Log: makefile,v $
# Revision 1.1  2013-09-04 07:17:58  chippisley
# Import files into new source repository.
#
# Revision 1.13  2012/03/21 15:48:12  cdobson
# Makefile changed to prevent the library from being created every time
# make is called.
#
# Revision 1.12  2011/01/20 10:01:25  hmuneer
# CA01A151
#
# Revision 1.11  2010/06/23 10:51:04  hmuneer
# CUTE V1.01
#
# Revision 1.10  2010/04/23 09:07:38  swilson
# Add alloc.c for dynamic heap management.
#
# Revision 1.9  2008/09/29 13:27:05  swilson
# Clean up makefile include dependencies.
#
# Revision 1.8  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.7  2008/05/21 13:27:54  swilson
# Proving of mem...() and _fmem...() functions. Adding push/pop of EBX and ECX around assembler code.
#
# Revision 1.6  2008/05/15 15:32:40  swilson
# Add build and version tracking. Implement & test missing c-library functions.
#
# Revision 1.5  2008/05/14 09:59:01  swilson
# Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
#
# Revision 1.4  2008/05/12 15:04:40  swilson
# Add IBIT console. Debugging various library functions.
#
# Revision 1.3  2008/04/25 15:25:07  swilson
# General organizational changes and additions.
#
# Revision 1.2  2008/04/24 16:05:50  swilson
# Further updates. Added HAL and BIT library - only I/O functions present at this point.
#
# Revision 1.1  2008/04/23 16:28:59  swilson
# Updates and new code centered on clib creation.
#
#


WARN	=

AS 	= as
CC	= gcc
LD	= ld

FREERTOSINC= -I../FreeRTOS/Demo/Common/include -I../FreeRTOS/Source/include -I../FreeRTOS/Demo/CUTE
INCLUDE	= ../include

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c $(FREERTOSINC) -I$(INCLUDE) -DGCC_CUTE -Wall $(WARN) 
LDFLAGS = -s

TARGETLIB = libclib.a

LIBOBJS	  = string.o ctype.o vsprintf.o mem.o far_mem.o string_a.o stdio.o \
	        stdlib.o setjmp.o alloc.o error_msg.o bcopy.o

DESTDIR   = ../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) $(DESTDIR)

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)

# Secondary Targets

alloc.o: alloc.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/private/k_alloc.h
	$(CC) $(CFLAGS) alloc.c

ctype.o: ctype.c							\
	$(INCLUDE)/ctype.h
	$(CC) $(CFLAGS) ctype.c

stdio.o: stdio.c							\
	$(INCLUDE)/stdio.h $(INCLUDE)/stdarg.h				\
	$(INCLUDE)/private/scc.h $(INCLUDE)/private/semaphore.h		\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h
	$(CC) $(CFLAGS) stdio.c

stdlib.o: stdlib.c							\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/ctype.h $(INCLUDE)/stdlib.h
	$(CC) $(CFLAGS) stdlib.c

string.o: string.c							\
	$(INCLUDE)/string.h
	$(CC) $(CFLAGS) string.c

vsprintf.o: vsprintf.c							\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdarg.h $(INCLUDE)/string.h
	$(CC) $(CFLAGS) vsprintf.c

error_msg.o: error_msg.c							\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdarg.h $(INCLUDE)/string.h
	$(CC) $(CFLAGS) error_msg.c

mem.o: mem.S
	$(AS) $(AFLAGS) mem.S -o mem.o

far_mem.o: far_mem.S
	$(AS) $(AFLAGS) far_mem.S -o far_mem.o

string_a.o: string_a.S
	$(AS) $(AFLAGS) string_a.S -o string_a.o

setjmp.o: setjmp.S
	$(AS) $(AFLAGS) setjmp.S -o setjmp.o

bcopy.o: bcopy.s
	$(AS) $(AFLAGS) bcopy.s -o bcopy.o

# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
