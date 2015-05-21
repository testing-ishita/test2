
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

# MAKEFILE - Interactive BIT build
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/mth/makefile,v 1.1 2013-09-04 07:44:08 chippisley Exp $
#
# $Log: makefile,v $
# Revision 1.1  2013-09-04 07:44:08  chippisley
# Import files into new source repository.
#
# Revision 1.4  2012/03/21 16:04:05  cdobson
# Makefile changed to prevent the library from being created every time
# make is called.
#
# Revision 1.3  2011/02/01 12:12:07  hmuneer
# C541A141
#
# Revision 1.2  2011/01/20 10:01:26  hmuneer
# CA01A151
#
# Revision 1.1  2010/05/05 15:49:31  hmuneer
# Cute rev 0.1
#
# Revision 1.8  2009/05/21 12:48:04  swilson
# Add help text
#
# Revision 1.7  2008/09/29 13:38:49  swilson
# bit_utils.c renamed to ibit_utils.c
#
# Revision 1.6  2008/09/29 13:27:05  swilson
# Clean up makefile include dependencies.
#
# Revision 1.5  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.4  2008/09/17 13:46:44  swilson
# Improvements to Local IBIT handler: add test execution counters and functions for display and zeroing them. Add continue on error function. Improve layout of results displayed when a test executes; include run and fail counts. Add support for VT100 compatible cursor-key handling within command-line editor. Move BIT execution support functions to a separate file BIT_UTIL.C. Make 'reset' command more robust.
#
# Revision 1.3  2008/05/15 15:32:40  swilson
# Add build and version tracking. Implement & test missing c-library functions.
#
# Revision 1.2  2008/05/14 09:59:01  swilson
# Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
#
# Revision 1.1  2008/05/12 14:46:43  swilson
# Local Interactive BIT command-line interface and utility functions - based on MON2
#  0.2.-13.
#
#


WARN	=
BTYPE   =

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../include
BOARDINC = ../board/$(BOARD)

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) -I$(BOARDINC) -Wall $(WARN)
LDFLAGS = -s

TARGETLIB = libmth.a

LIBOBJS	= cli.o mth_utils.o command.o help.o service.o util.o

DESTDIR = ../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB):  $(TARGETLIB)
	cp $(TARGETLIB) $(DESTDIR)

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)


# Secondary Targets

cli.o: cli.c								\
	mth.h								\
	$(INCLUDE)/stdio.h $(INCLUDE)/errors.h				\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/cpu.h
	$(CC) $(CFLAGS) cli.c

mth_utils.o: mth_utils.c						\
	mth.h								\
	$(INCLUDE)/stdlib.h $(INCLUDE)/stdio.h $(INCLUDE)/string.h	\
	$(INCLUDE)/ctype.h $(INCLUDE)/setjmp.h				\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h			\
	$(INCLUDE)/bit/hal.h						\
	$(INCLUDE)/private/sys_delay.h
	$(CC) $(CFLAGS) mth_utils.c

command.o: command.c							\
	mth.h								\
	$(INCLUDE)/stdio.h $(INCLUDE)/string.h $(INCLUDE)/errors.h \
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) command.c -D $(BTYPE)

help.o: help.c				\
	mth.h					\
	$(INCLUDE)/stdio.h		\
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) help.c

service.o: service.c							\
	mth.h								\
	$(INCLUDE)/stdlib.h $(INCLUDE)/stdio.h $(INCLUDE)/string.h	\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/private/port_io.h $(INCLUDE)/private/mem_io.h 	\
	$(INCLUDE)/private/cpu.h					\
	$(INCLUDE)/bit/board_service.h $(INCLUDE)/bit/pci.h
	$(CC) $(CFLAGS) service.c

util.o: util.c								\
	mth.h								\
	$(INCLUDE)/stdio.h $(INCLUDE)/errors.h				\
	$(INCLUDE)/private/port_io.h $(INCLUDE)/private/cpu.h		\
	$(INCLUDE)/bit/board_service.h	\
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) util.c



# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
	
