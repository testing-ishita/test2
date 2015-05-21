
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

# MAKEFILE - sm3
#


WARN	=

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../../include
BOARDINC = ../../board/$(BOARD)

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) -I$(BOARDINC) -Wall $(WARN)
LDFLAGS = -s

TARGETLIB = libsm3.a

LIBOBJS	= packet.o serialPacket.o autoid.o testParameters.o

DESTDIR = ../../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) ../../board/$(BOARD)/bin

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)


# Secondary Targets

packet.o: packet.c									\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h		\
	$(INCLUDE)/errors.h								\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 
	$(CC) $(CFLAGS) packet.c -D $(BTYPE)

serialPacket.o: serialPacket.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h		\
	$(INCLUDE)/errors.h								\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 
	$(CC) $(CFLAGS) serialPacket.c -D $(BTYPE)
	
autoid.o: autoid.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h		\
	$(INCLUDE)/errors.h								\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 
	$(CC) $(CFLAGS) autoid.c	-D $(BTYPE)
	
testParameters.o: testParameters.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h		\
	$(INCLUDE)/errors.h								\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 
	$(CC) $(CFLAGS) testParameters.c	-D $(BTYPE)
		
# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
