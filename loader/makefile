
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

# MAKEFILE - LOADER build
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/loader/makefile,v 1.1 2013-09-04 07:42:08 chippisley Exp $
#
# $Log: makefile,v $
# Revision 1.1  2013-09-04 07:42:08  chippisley
# Import files into new source repository.
#
# Revision 1.8  2012/11/02 14:15:56  chippisley
# Added BTYPE.
#
# Revision 1.7  2011/10/27 15:50:07  hmuneer
# no message
#
# Revision 1.6  2010/06/23 11:00:52  hmuneer
# CUTE V1.01
#
# Revision 1.5  2010/05/05 15:49:10  hmuneer
# Cute rev 0.1
#
# Revision 1.4  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.3  2008/07/18 12:42:46  swilson
# Add OS discovery header.
#
# Revision 1.2  2008/04/21 16:59:24  swilson
# Basic framework, now with stack, initialised data and C-to-asm linkage all apparently working.
#
#

BITFLAG = 
BTYPE	= 
WARN	=
IMAGETYPE =

AS 	= as
CC	= gcc
LD	= ld

AFLAGS	= -I../include --fatal-warnings
CFLAGS	= -c -I../include -Wall $(WARN)
LDFLAGS = -s

OBJS1	= boot_real.o 


ifeq ($(BITFLAG), MAKE_CUTE)	
OBJS2	= boot_prot.o	  
else
OBJS2	= boot_prot.o header.o 	        
endif

OBJS3	= boot_comp.o


# Primary Target
# --------------

ifeq ($(IMAGETYPE), COMPRESSED)
all: boot_real.bin comp_loader.o ../lzlib/liblz.a loader.o

comp_loader.o: $(OBJS3) ../lzlib/liblz.a
	$(LD) -r -o comp_loader.o $(OBJS3) ../lzlib/liblz.a -Map comp_loader.map

else
all: boot_real.bin loader.o

endif

loader.o: $(OBJS2)
	$(LD) -r -o loader.o $(OBJS2)

boot_real.bin: $(OBJS1)
	$(LD) $(LDFLAGS) -Ttext 0 -e real_entry -o boot_real.out $(OBJS1) -Map boot.map
	objcopy -O binary -j .text boot_real.out boot_real.bin

	
# Secondary Targets

boot_real.o: boot_real.S
	$(AS) $(AFLAGS) --defsym $(BITFLAG)=1 --defsym $(BTYPE)=1 --defsym $(IMAGETYPE)=1 boot_real.S -o boot_real.o

boot_prot.o: boot_prot.S
	$(AS) $(AFLAGS) --defsym $(BTYPE)=1 boot_prot.S -o boot_prot.o

boot_comp.o: boot_comp.s
	$(AS) $(AFLAGS) --defsym $(BTYPE)=1 boot_comp.s -o boot_comp.o
	
../lzlib/liblz.a: FORCE
	(cd ../; cd lzlib; make all)

FORCE:
	
# ------------------------------------------------------------------------------

ifneq ($(BITFLAG), MAKE_CUTE)	
header.o: header.S
	$(AS) $(AFLAGS) header.S -o header.o
endif

# Minor Targets

clean:
	(cd ../; cd lzlib; make clean)
	rm -f *.o
	rm -f *.bin
	rm -f *.out
	rm -f *.map;
	
