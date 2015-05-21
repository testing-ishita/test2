
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

# MAKEFILE - Power-on BIT test handler
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/pbit/makefile,v 1.1 2013-09-04 07:44:50 chippisley Exp $
#
# $Log: makefile,v $
# Revision 1.1  2013-09-04 07:44:50  chippisley
# Import files into new source repository.
#
# Revision 1.5  2012/03/21 16:04:35  cdobson
# Makefile changed to prevent the library from being created every time
# make is called.
#
# Revision 1.4  2011/01/20 10:01:26  hmuneer
# CA01A151
#
# Revision 1.3  2008/09/29 13:27:05  swilson
# Clean up makefile include dependencies.
#
# Revision 1.2  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.1  2008/06/20 12:16:54  swilson
# Foundations of PBIT and PCI allocation. Added facility to pass startup options into
#  C-code.
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

TARGETLIB = libpbit.a

LIBOBJS	= sequencer.o

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

sequencer.o: sequencer.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) sequencer.c



# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
	
