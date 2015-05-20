
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

# MAKEFILE - HAL build
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/hal/makefile,v 1.1 2013-09-04 07:26:55 chippisley Exp $
#
# $Log: makefile,v $
# Revision 1.1  2013-09-04 07:26:55  chippisley
# Import files into new source repository.
#
# Revision 1.9  2012/03/21 16:02:58  cdobson
# Makefile changed to prevent the library from being created every time
# make is called.
#
# Revision 1.8  2011/01/20 10:01:25  hmuneer
# CA01A151
#
# Revision 1.7  2008/09/29 13:27:05  swilson
# Clean up makefile include dependencies.
#
# Revision 1.6  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.5  2008/07/24 14:40:52  swilson
# Add hal_delay.c
#
# Revision 1.4  2008/05/22 16:04:52  swilson
# Add HAL console write and memory access interfaces - memory is just a shell at present.
#
# Revision 1.3  2008/05/14 09:59:01  swilson
# Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
#
# Revision 1.2  2008/04/30 07:51:19  swilson
# Add PCI support library.
#
# Revision 1.1  2008/04/24 16:03:55  swilson
# Created HAL.
#
#


WARN	=

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../include

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) -Wall $(WARN)
LDFLAGS = -s

TARGETLIB = libhal.a

LIBOBJS	= hal_api.o hal_cons.o hal_delay.o hal_io.o hal_mem.o hal_pci.o

DESTDIR = ../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) $(DESTDIR)

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)


# Secondary Targets

hal_api.o: hal_api.c							\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/setjmp.h			\
	$(INCLUDE)/bit/hal.h
	$(CC) $(CFLAGS) hal_api.c

hal_cons.o: hal_cons.c							\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/bit/hal.h
	$(CC) $(CFLAGS) hal_cons.c

hal_delay.o: hal_delay.c						\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/hal.h						\
	$(INCLUDE)/private/sys_delay.h
	$(CC) $(CFLAGS) hal_delay.c

hal_io.o: hal_io.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/private/port_io.h 					\
	$(INCLUDE)/bit/hal.h
	$(CC) $(CFLAGS) hal_io.c

hal_mem.o: hal_mem.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/hal.h
	$(CC) $(CFLAGS) hal_mem.c

hal_pci.o: hal_pci.c							\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/string.h			\
	$(INCLUDE)/bit/hal.h						\
	$(INCLUDE)/private/cpu.h
	$(CC) $(CFLAGS) hal_pci.c


# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
	
