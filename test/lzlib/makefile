
# ************************************************************************
# *                                                                      *
# *      Copyright 2013 Concurrent Technologies, all rights reserved.    *
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


WARN =

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../include
FREERTOSINC= -I../FreeRTOS/Demo/Common/include -I../FreeRTOS/Source/include -I../FreeRTOS/Demo/CUTE

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) $(FREERTOSINC) -DGCC_CUTE -Wall
LDFLAGS = -s

TARGETLIB = liblz.a

LIBOBJS	=  lz.o

DESTDIR = ../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(TARGETLIB)

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)


# Secondary Targets
# -----------------

lz.o: lz.c	\
	./lz.h
	$(CC) $(CFLAGS) lz.c	
			
	
# Minor Targets
# -------------

clean:
	rm -f *.o
	rm -f *.a
	
