
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

# MAKEFILE - idx
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

TARGETLIB = libidx.a

LIBOBJS	= idxxsa.o autoid.o packet.o serialPacket.o testParameters.o sm3_idx.o\
          slaveParameters.o

DESTDIR = ../../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) $(DESTDIR)

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)
	
idxxsa.o: idxxsa.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) idxxsa.c


autoid.o: autoid.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) autoid.c

packet.o: packet.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) packet.c
	
serialPacket.o: serialPacket.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) serialPacket.c


testParameters.o: testParameters.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) testParameters.c
	
sm3_idx.o: sm3_idx.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) sm3_idx.c	


slaveParameters.o: slaveParameters.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/errors.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) slaveParameters.c	
	

# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
