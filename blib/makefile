
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

# MAKEFILE - BIT Library build
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/blib/makefile,v 1.3 2013-12-10 12:09:23 mgostling Exp $
#
# $Log: makefile,v $
# Revision 1.3  2013-12-10 12:09:23  mgostling
# Added new generic bitbanging I2C functions.
#
# Revision 1.2  2013-09-26 12:58:11  chippisley
# Removed obsolete HAL.
#
# Revision 1.1  2013/09/04 07:11:17  chippisley
# Import files into new source repository.
#
# Revision 1.13  2012/03/27 07:51:04  cdobson
# Added code to create the library directory if it does ont exist.
#
# Revision 1.12  2012/03/21 15:50:52  cdobson
# Makefile changed to prevent the library from being created every time
# make is called.
#
# Revision 1.11  2011/01/11 17:24:36  hchalla
# New board service function added to the code base and new board VX813.
#
# Revision 1.10  2010/06/23 10:47:28  hmuneer
# CUTE V1.01
#
# Revision 1.9  2008/09/29 13:27:05  swilson
# Clean up makefile include dependencies.
#
# Revision 1.8  2008/09/26 13:17:03  swilson
# Add bit_support.asm
#
# Revision 1.7  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.6  2008/07/24 14:36:48  swilson
# Move system functions delay.S and semaphore.S back to kernel. They really belong there. Add delay.c, which is an interface to the HAL delay function.
#
# Revision 1.5  2008/07/18 12:20:43  swilson
# Add bit_exec, delay and semaphore code moved here from kernel.
#
# Revision 1.4  2008/05/22 16:04:52  swilson
# Add HAL console write and memory access interfaces - memory is just a shell at present.
#
# Revision 1.3  2008/05/14 09:59:01  swilson
# Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
#
# Revision 1.2  2008/04/30 07:51:19  swilson
# Add PCI support library.
#
# Revision 1.1  2008/04/24 16:02:46  swilson
# Created BIT Library
#
#


WARN	=

AS 	= as
CC	= gcc
LD	= ld

FREERTOSINC= -I../FreeRTOS/Demo/Common/include -I../FreeRTOS/Source/include -I../FreeRTOS/Demo/CUTE
INCLUDE	= ../include
BOARDINC = ../board/$(BOARD)

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c $(FREERTOSINC) -I$(INCLUDE) -I$(BOARDINC) -DGCC_CUTE -Wall $(WARN)
LDFLAGS = -s

TARGETLIB = libblib.a

LIBOBJS	= bit_exec.o bit_support.o delay.o console.o io.o mem.o pci.o pciInt.o msr.o bitbangI2c.o

DESTDIR = ../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB): $(TARGETLIB)
	if test ! -d $(DESTDIR);then mkdir $(DESTDIR); fi 
	cp $(TARGETLIB) ../board/$(BOARD)/bin

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)

# Secondary Targets

bit_exec.o: bit_exec.c							\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/errors.h			\
	$(INCLUDE)/bit/board_service.h $(INCLUDE)/bit/hal.h			\
	$(INCLUDE)/bit/bit.h
	$(CC) $(CFLAGS) bit_exec.c

bit_support.o: bit_support.c						\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/bit/bit.h						\
	$(INCLUDE)/private/semaphore.h $(INCLUDE)/private/debug.h
	$(CC) $(CFLAGS) bit_support.c

console.o: console.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/hal.h $(INCLUDE)/bit/console.h			\
	$(INCLUDE)/private/semaphore.h
	$(CC) $(CFLAGS) console.c

delay.o: delay.c							\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/hal.h $(INCLUDE)/bit/delay.h
	$(CC) $(CFLAGS) delay.c

io.o: io.c								\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/hal.h $(INCLUDE)/bit/io.h
	$(CC) $(CFLAGS) io.c

mem.o: mem.c								\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/hal.h $(INCLUDE)/bit/mem.h
	$(CC) $(CFLAGS) mem.c

pci.o: pci.c								\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/hal.h $(INCLUDE)/bit/pci.h
	$(CC) $(CFLAGS) pci.c
	
pciInt.o: pciInt.c							\
	$(INCLUDE)/stdtypes.h					\
	$(INCLUDE)/bit/pci.h
	$(CC) $(CFLAGS) pciInt.c

msr.o: msr.S
	$(AS) $(AFLAGS) msr.S -o msr.o
	

bitbangI2c.o: bitbangI2c.c							\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h		\
	$(INCLUDE)/errors.h	$(INCLUDE)/bit/console.h	\
	$(INCLUDE)/bit/delay.h							\
	$(INCLUDE)/private/debug.h						
	$(CC) $(CFLAGS) bitbangI2c.c

# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
