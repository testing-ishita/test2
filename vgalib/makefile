
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


WARN	=

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../include
FREERTOSINC= -I../FreeRTOS/Demo/Common/include -I../FreeRTOS/Source/include -I../FreeRTOS/Demo/CUTE
BOARDINC= -I../board/$(BOARD)

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) $(FREERTOSINC) $(BOARDINC) -DGCC_CUTE -Wall $(WARN)
# -Wa,-alsn=$@.lst
LDFLAGS = -s

TARGETLIB = libvga.a

LIBOBJS	=  vga.o

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

vga.o: vga.c					\
	$(INCLUDE)/stdio.h			\
	$(INCLUDE)/stdlib.h			\
	$(INCLUDE)/string.h			\
	../board/$(BOARD)/config.h	\
	./vga.h
	$(CC) $(CFLAGS) vga.c	
			
	
# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
	
