
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
# $Header: /home/cvs/cvsroot/CCT_BIT_2/sth/makefile,v 1.5 2015-01-29 10:31:53 mgostling Exp $
#
# $Log: makefile,v $
# Revision 1.5  2015-01-29 10:31:53  mgostling
# Simplified handling of $(BTYPE) parameter
#
# Revision 1.4  2014-01-29 13:32:40  hchalla
# Added suppport for SRIO for AM 94x.
#
# Revision 1.3  2013-11-25 11:21:30  mgostling
# Pass board type to secondary targets.
#
# Revision 1.2  2013-09-26 12:37:38  chippisley
# Added support for network Soak Master Client.
#
# Revision 1.1  2013/09/04 07:45:36  chippisley
# Import files into new source repository.
#
# Revision 1.5  2012/03/27 07:59:34  cdobson
# Makefile changed to prevent the library from being created every time
# make is called.
#
# Revision 1.4  2011/01/20 10:01:26  hmuneer
# CA01A151
#
# Revision 1.3  2010/09/15 12:23:26  hchalla
# Added support for TR 501 BIT/CUTE
#
# Revision 1.2  2010/06/23 11:01:12  hmuneer
# CUTE V1.01
#
# Revision 1.1  2010/05/05 15:49:54  hmuneer
# Cute rev 0.1
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
BTYPE   =


AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../include
BOARDINC = ../board/$(BOARD)
FREERTOSINC= -I../FreeRTOS/Source/include -I../FreeRTOS/Demo/CUTE

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) $(FREERTOSINC) -I$(BOARDINC) -D $(BTYPE) -DGCC_CUTE -Wall $(WARN)
LDFLAGS = -s

TARGETLIB = libsth.a

DESTDIR   = ../board/$(BOARD)/bin

ifeq ($(BTYPE), VME)
	LIBOBJS	= sth_router.o sthVme.o
#	STHLIB  = libsthvme.a
	STHLIB  = libsth.a
endif

ifeq ($(BTYPE), CPCI)
	LIBOBJS	= sth_router.o sthCpci.o
#	STHLIB  = libsthcpci.a
	STHLIB  = libsth.a
endif


ifeq ($(BTYPE), VPX)
	LIBOBJS	= sth_router.o sthVpx.o
#	STHLIB  = libsthvpx.a
	STHLIB  = libsth.a
endif

ifeq ($(BTYPE), SRIO)
	LIBOBJS	= sth_router.o sthCpci.o
#	STHLIB  = libsthcpci.a
	STHLIB  = libsth.a
endif
# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB): $(STHLIB)
	if test ! -d $(DESTDIR);then mkdir $(DESTDIR); fi 
	cp $(STHLIB) $(DESTDIR)/$(TARGETLIB)

$(STHLIB): $(LIBOBJS)
	$(AR) -rcs $(STHLIB) $(LIBOBJS)
	$(AR) -tv $(STHLIB)



# Secondary Targets

sth_router.o: sth_router.c								\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h	\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) sth_router.c -D $(BTYPE)

ifeq ($(BTYPE), VME)
sthVme.o: sthVme.c										\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h	\
	$(INCLUDE)/private/semaphore.h						\
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) sthVme.c
endif	

ifeq ($(BTYPE), CPCI)
sthCpci.o: sthCpci.c									\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h	\
	$(INCLUDE)/private/semaphore.h						\
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) sthCpci.c
endif	

ifeq ($(BTYPE), SRIO)
sthCpci.o: sthCpci.c									\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h	\
	$(INCLUDE)/private/semaphore.h						\
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) sthCpci.c
endif	

ifeq ($(BTYPE), VPX)
sthVpx.o: sthVpx.c										\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h	\
	$(INCLUDE)/private/semaphore.h						\
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) sthVpx.c
endif	

# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
	
