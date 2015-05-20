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
#-DDEBUG_SRIO
WARN	=
AS 	:= as
CC	:= gcc
LD	:= ld
INCLUDE	= ../../include
FREERTOS_INC = -I../../FreeRTOS/Demo/Common/include -I../../FreeRTOS/Source/include -I../../FreeRTOS/Demo/CUTE -I../../FreeRTOS/Source/portable/GCC/CUTE
AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) $(FREERTOS_INC) -Wall $(WARN) 
LDFLAGS = -s
TARGETLIB = libsrioArch.a
LIBOBJS	=  rio.o rio-access.o rio-driver.o rio-scan.o tsi721.o idt_gen2.o srioArch.o 
DESTDIR =  ../../board/$(BOARD)/bin
# Primary Target
# --------------
all: $(DESTDIR)/$(TARGETLIB)
$(DESTDIR)/$(TARGETLIB):  $(TARGETLIB)
	cp $(TARGETLIB) $(DESTDIR)
$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)
# Secondary Targets
rio.o: rio.c 					
	$(CC) $(CFLAGS) rio.c
rio-access.o: rio-access.c           
	$(CC) $(CFLAGS) rio-access.c
rio-driver.o: rio-driver.c						
	$(CC) $(CFLAGS) rio-driver.c
rio-scan.o: rio-scan.c							
	$(CC) $(CFLAGS) rio-scan.c
tsi721.o: tsi721.c	
	$(CC) $(CFLAGS) tsi721.c
srioArch.o: srioArch.c	
	$(CC) $(CFLAGS) srioArch.c
# Minor Targets
clean:
	rm -f *.o
	rm -f *.a
