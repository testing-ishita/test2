
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

# MAKEFILE - interconnect
#


WARN	=

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../../include

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) -Wall $(WARN)
LDFLAGS = -s

TARGETLIB = libinterconnect.a

DESTDIR   = ../../board/$(BOARD)/bin

ifeq ($(BTYPE), VME)
	LIBOBJS	= vme_Interconnect.o
#	ICLIB   = libicvme.a
	ICLIB   = libinterconnect.a
endif

ifeq ($(BTYPE), CPCI)
	LIBOBJS	= cpci_Interconnect.o
#	ICLIB   = libiccpci.a
	ICLIB   = libinterconnect.a
endif

ifeq ($(BTYPE), SRIO)
	LIBOBJS	= cpci_Interconnect.o
#	ICLIB   = libiccpci.a
	ICLIB   = libinterconnect.a
endif

ifeq ($(BTYPE), VPX)
	LIBOBJS	= cpci_Interconnect.o
#	ICLIB   = libicvpx.a
	ICLIB   = libinterconnect.a
endif



# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB): $(ICLIB)
	cp $(ICLIB) $(DESTDIR)/$(TARGETLIB)

$(ICLIB): $(LIBOBJS)
	$(AR) -rcs $(ICLIB) $(LIBOBJS)
	$(AR) -tv $(ICLIB)


# Secondary Targets

ifeq ($(BTYPE), VME)
vme_Interconnect.o: vme_Interconnect.c					\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h \
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) vme_Interconnect.c
endif

ifeq ($(BTYPE), CPCI)
cpci_Interconnect.o: cpci_Interconnect.c				\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h \
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) cpci_Interconnect.c
endif

ifeq ($(BTYPE), SRIO)
cpci_Interconnect.o: cpci_Interconnect.c				\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h \
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) cpci_Interconnect.c
endif
ifeq ($(BTYPE), VPX)
cpci_Interconnect.o: cpci_Interconnect.c				\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h									\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h \
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) cpci_Interconnect.c
endif


# Minor Targets

clean:
	rm -f *.o
	rm -f *.o
	rm -f *.a