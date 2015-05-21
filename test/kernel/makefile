
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

# MAKEFILE - KERNEL build
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/kernel/makefile,v 1.2 2014-01-29 13:26:48 hchalla Exp $
#
# $Log: makefile,v $
# Revision 1.2  2014-01-29 13:26:48  hchalla
# Added support for SRIO for AM94x.
#
# Revision 1.1  2013-09-04 07:41:28  chippisley
# Import files into new source repository.
#
# Revision 1.22  2012/11/02 14:08:48  chippisley
# SMP changes, added lapic.s & atomic.s.
#
# Revision 1.21  2012/03/27 07:58:04  cdobson
# Added code to create the library file directory is it does not exist.
#
# Revision 1.20  2012/03/21 15:27:00  cdobson
# Changed makefile so that the library is not created every time make is called.
#
# Revision 1.19  2011/01/20 10:01:26  hmuneer
# CA01A151
#
# Revision 1.18  2010/09/15 12:22:07  hchalla
# Added support for TR 501 BIT/CUTE
#
# Revision 1.17  2010/07/26 13:14:10  hmuneer
# Source Cleanup
#
# Revision 1.16  2010/06/23 11:00:42  hmuneer
# CUTE V1.01
#
# Revision 1.15  2010/05/05 15:48:53  hmuneer
# Cute rev 0.1
#
# Revision 1.14  2009/05/20 08:59:48  swilson
# Add interrupt handler initialization and management functions.
#
# Revision 1.13  2008/09/29 13:27:05  swilson
# Clean up makefile include dependencies.
#
# Revision 1.12  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.11  2008/07/24 14:46:40  swilson
# Add PCI initialization, System-level semaphore and delay functions moved back here from BLIB as they really belong here now that a HAL delay function exists.
#
# Revision 1.10  2008/07/18 12:26:32  swilson
# Move bit_exec and delay to blib.
#
# Revision 1.9  2008/07/10 16:51:30  swilson
# Add calibrated delay fuunctions. Re-code loader to support SMP initialization and
#  add SMP init.
#
# Revision 1.8  2008/06/02 10:31:28  swilson
# Add cctExecuteBit() interface and modify IBIT to use this. Add support for complex
#  BIT execution expressions and BIT parameters.
#
# Revision 1.7  2008/05/29 10:09:47  swilson
# Add local implementation of HAL memory functions. Modify loader to configure the runtime GDT and IDT.
#
# Revision 1.6  2008/05/14 09:59:01  swilson
# Reorganization of kernel-level source and header files. Ensure dependencies in makefiles are up to date.
# 
# Revision 1.5  2008/05/12 15:02:43  swilson
# Various new files
#
# Revision 1.4  2008/04/24 16:05:50  swilson
# Further updates. Added HAL and BIT library - only I/O functions present at this point.
#
# Revision 1.3  2008/04/23 16:28:59  swilson
# Updates and new code centered on clib creation.
#
# Revision 1.2  2008/04/21 16:59:24  swilson
# Basic framework, now with stack, initialised data and C-to-asm linkage all apparently working.
#
#


WARN	=
BTYPE   =

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../include
BOARDINC = ../board/$(BOARD)
FREERTOSINC= -I../FreeRTOS/Demo/Common/include -I../FreeRTOS/Source/include -I../FreeRTOS/Demo/CUTE
LWIPDIR = ../lwip-1.4.0
LWIPINC = -I$(LWIPDIR)/src/include -I$(LWIPDIR)/src/include/ipv4 -I$(LWIPDIR)/ports/include -I$(LWIPDIR)/lwIP_Apps

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) -I$(BOARDINC) $(FREERTOSINC) $(LWIPINC) -DGCC_CUTE -Wall $(WARN)
# -Wa,-alsn=$@.lst
LDFLAGS = -s

TARGETLIB = libkernel.a

LIBOBJS	  = main.o debug_cons.o mmu.o pci_alloc.o scc.o  \
	  cpu.o sys_delay.o gdt.o idt.o ioapic.o mem_io.o 	\
	  port_io.o semaphore.o lapic.o atomic.o spinlock.o

DESTDIR   = ../board/$(BOARD)/bin

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB)

$(DESTDIR)/$(TARGETLIB): $(TARGETLIB)
	if test ! -d $(DESTDIR);then mkdir $(DESTDIR); fi 
	cp $(TARGETLIB) $(DESTDIR)

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)


# Secondary Targets

main.o: main.c								\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/stdio.h			\
	$(INCLUDE)/private/cpu.h $(INCLUDE)/private/debug.h		\
	$(INCLUDE)/private/scc.h $(INCLUDE)/private/semaphore.h		\
	$(INCLUDE)/private/sys_delay.h					\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/bit/hal.h \
	$(BOARDINC)/config.h
	$(CC) $(CFLAGS) main.c -D $(BITFLAG) -D $(STARTUPFLAG) -D $(BTYPE)

debug_cons.o: debug_cons.c						\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/private/debug.h $(INCLUDE)/private/scc.h		\
	$(INCLUDE)/private/semaphore.h					\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h
	$(CC) $(CFLAGS) debug_cons.c

mmu.o: mmu.c								\
	$(INCLUDE)/stdtypes.h $(INCLUDE)/errors.h			\
	$(INCLUDE)/private/cpu.h $(INCLUDE)/private/semaphore.h		\
	$(INCLUDE)/bit/board_service.h
	$(CC) $(CFLAGS) mmu.c

pci_alloc.o: pci_alloc.c						\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/bit/bit.h $(INCLUDE)/bit/board_service.h 		\
	$(INCLUDE)/bit/pci.h						\
	$(INCLUDE)/devices/pci_devices.h
	$(CC) $(CFLAGS) pci_alloc.c

scc.o: scc.c								\
	$(INCLUDE)/stdtypes.h						\
	$(INCLUDE)/private/port_io.h $(INCLUDE)/private/scc.h		\
	$(INCLUDE)/bit/board_service.h					\
	$(INCLUDE)/devices/scc16550.h
	$(CC) $(CFLAGS) scc.c


cpu.o: cpu.S
	$(AS) $(AFLAGS) cpu.S -o cpu.o

sys_delay.o: sys_delay.S	\
	$(INCLUDE)/asm/i386.h
	$(AS) $(AFLAGS) sys_delay.S -o sys_delay.o

gdt.o: gdt.S	\
	$(INCLUDE)/asm/i386.h
	$(AS) $(AFLAGS) gdt.S -o gdt.o

idt.o: idt.S	\
	$(INCLUDE)/asm/i386.h
	$(AS) $(AFLAGS) --defsym $(BTYPE)=1 idt.S -o idt.o 

ioapic.o: ioapic.S	\
	$(INCLUDE)/asm/i386.h
	$(AS) $(AFLAGS) ioapic.S -o ioapic.o
	
lapic.o: lapic.S	\
	$(INCLUDE)/asm/i386.h
	$(AS) $(AFLAGS) lapic.S -o lapic.o

mem_io.o: mem_io.S
	$(AS) $(AFLAGS) mem_io.S -o mem_io.o

port_io.o: port_io.S
	$(AS) $(AFLAGS) port_io.S -o port_io.o

semaphore.o: semaphore.S	\
	$(INCLUDE)/asm/i386.h
	$(AS) $(AFLAGS) semaphore.S -o semaphore.o

atomic.o: atomic.s	\
	$(INCLUDE)/asm/i386.h
	$(AS) $(AFLAGS) atomic.s -o atomic.o

spinlock.o: spinlock.c \
	$(INCLUDE)/stdtypes.h	\
	$(INCLUDE)/private/atomic.h \
	$(INCLUDE)/private/spinlock.h 
	$(CC) $(CFLAGS) spinlock.c

# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
	
