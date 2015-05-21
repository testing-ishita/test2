
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
# -I../../FreeRTOS/Demo/Common/include -I../../FreeRTOS/Source/include -I../../FreeRTOS/Source/portable/GCC/CUTE -I../../FreeRTOS/Demo/CUTE


WARN	=

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../../include
BOARDINC = ../../board/$(BOARD)

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) -I$(BOARDINC) -Wall $(WARN)
LDFLAGS = -s

TARGETLIB = libvmeArch.a

LIBOBJS	=  tsi148.o

DESTDIR = ../../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB):  $(TARGETLIB)
	cp $(TARGETLIB) $(DESTDIR)

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)


# Secondary Targets

tsi148.o: tsi148.c											\
	$(INCLUDE)/stdio.h $(INCLUDE)/errors.h					\
	$(INCLUDE)/private/port_io.h $(INCLUDE)/private/cpu.h	\
	$(INCLUDE)/bit/board_service.h							\
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) tsi148.c

	
# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
	
