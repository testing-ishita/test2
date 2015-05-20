
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

# MAKEFILE - test directory build
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/tests/makefile,v 1.13 2015-04-01 16:49:39 hchalla Exp $
#
# $Log: makefile,v $
# Revision 1.13  2015-04-01 16:49:39  hchalla
# Added support for CPCI backplane tests.
#
# Revision 1.12  2015-02-03 11:49:16  mgostling
# Added enpmc tests
#
# Revision 1.11  2014-11-24 15:55:24  mgostling
# Added new file xmRs2Test.c
#
# Revision 1.10  2014-11-04 15:16:26  mgostling
# Removed dependency for ipmi_m3.o as it is no longer required.
#
# Revision 1.9  2014-09-19 13:35:11  mgostling
# Added new file ipmi_M3.c
#
# Revision 1.8  2014-09-19 10:47:34  mgostling
# Added eth2.c to test library
#
# Revision 1.7  2014-03-28 15:20:25  cdobson
# Added wPciMPTGeneralTest
#
# Revision 1.6  2014-03-20 15:57:23  mgostling
# Added enxmc8619gpio.c
#
# Revision 1.5  2014-03-06 16:30:11  cdobson
# Added new file audioTest2.c.
#
# Revision 1.4  2014-02-14 11:34:29  chippisley
# Added BOARDINC.
#
# Revision 1.3  2014/01/29 13:40:27  hchalla
# Added support for SRIO tests.
#
# Revision 1.2  2013-11-25 14:28:13  mgostling
# Added AHCI and ADCR6 tests.
#
# Revision 1.1  2013-09-04 07:46:43  chippisley
# Import files into new source repository.
#
# Revision 1.25  2012/03/21 15:53:05  cdobson
# C Source file dependancies created automatically by the compiler.
#
# Revision 1.24  2012/02/17 11:35:16  hchalla
# Added Support for PP 81x and PP 91x.
#
# Revision 1.23  2011/08/02 17:01:59  hchalla
# *** empty log message ***
#
# Revision 1.22  2011/05/16 14:32:12  hmuneer
# Info Passing Support
#
# Revision 1.21  2011/05/11 12:17:17  hchalla
# TP 702 PBIT Related changes.
#
# Revision 1.20  2011/03/22 14:07:57  hchalla
# *** empty log message ***
#
# Revision 1.19  2011/01/20 10:01:27  hmuneer
# CA01A151
#
# Revision 1.18  2010/11/04 18:00:03  hchalla
# Updated make file.
#
# Revision 1.17  2010/10/06 10:14:01  jthiru
# PBIT coding for VP717
#
# Revision 1.16  2010/09/15 12:25:15  hchalla
# Modified tests to  support TR 501 BIT/CUTE.
#
# Revision 1.15  2010/06/23 11:01:57  hmuneer
# CUTE V1.01
#
# Revision 1.14  2010/01/19 12:04:49  hmuneer
# vx511 v1.0
#
# Revision 1.13  2009/12/16 14:10:32  sdahanayaka
# Added the VME and Watchdog Tests. Also fixed some PCI Bugs
#
# Revision 1.12  2009/09/29 12:31:15  swilson
# Application Flash test for Mirror Flash device.
#
# Revision 1.11  2009/06/08 16:12:24  swilson
# Add SMP.C.
#
# Revision 1.10  2009/06/05 14:32:31  jthiru
# Added SII3124 and CAN
#
# Revision 1.9  2009/05/21 08:16:41  cvsuser
# Added FWH test module
#
# Revision 1.8  2009/05/20 12:11:16  swilson
# Add interrupt test.
#
# Revision 1.7  2009/05/18 09:25:30  hmuneer
# no message
#
# Revision 1.6  2009/05/15 11:04:06  jthiru
# Added more test modules
#
# Revision 1.5  2009/05/05 12:20:17  swilson
# Remove UNIX-like line ends as breaks build on steve's machine
#
# Revision 1.4  2009/02/04 08:03:24  jthiru
# Added serial and ethernet compilation rule
#
# Revision 1.3  2008/09/29 13:27:05  swilson
# Clean up makefile include dependencies.
#
# Revision 1.2  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.1  2008/05/16 14:35:26  swilson
# Create trivial tests for checking test execution.
#
#


WARN	=

AS 	= as
CC	= gcc
LD	= ld

INCLUDE	= ../include
BOARDINC = ../board/$(BOARD)
FREERTOSINC= -I../FreeRTOS/Demo/Common/include -I../FreeRTOS/Source/include -I../FreeRTOS/Demo/CUTE

AFLAGS	= -I$(INCLUDE) --fatal-warnings
CFLAGS	= -c -I$(INCLUDE) -I$(BOARDINC) $(FREERTOSINC) -Wall $(WARN) -D $(BITFLAG) -D $(BTYPE) -DGCC_CUTE
LDFLAGS = -s

TARGETLIB = libbittest.a

C_OBJS	= pass.o fail.o random.o serial.o eth.o ide.o ram.o usb.o		\
		  tempsensor.o keybd.o siotests.o pcitests.o ledtest.o			\
		  NVMemTest.o audioTest.o videoTest.o RTCTests.o PITTests.o		\
		  SCRTests.o ipmi.o watchDogtests.o LDT-PITtests.o 				\
		  SpeedSteptests.o XMM_MMXtests.o fpuTests.o SMBUS.o 			\
		  interrupt.o fwh.o sii3124.o can.o smp.o mirrorflash.o			\
		  tsi148.o cpciTests.o spiFwh.o vmeTests.o fpNMI.o mouse.o scsi.o \
		  spieepromtest.o enxmc8619.o gpioTests.o spiMRAMTest.o 		\
		  tpm_tests.o xmRs1Test.o xmRs2Test.o ahciTest.o adcr6.o		\
		  srioTests.o audioTest2.o plx8733.o enxmc8619gpio.o lsimpt.o	\
		  eth2.o enpmc.o enpmcgpio.o


S_OBJS   = fpu.o xmmx.o

LIBOBJS	= $(C_OBJS) $(S_OBJS)

ifneq ($(BITFLAG), MAKE_BIT)
	ifeq ($(BTYPE), VME)
		C_OBJS   += cctInfo.o
		CFLAGS += -DCUTE
	endif
	
	ifeq ($(BTYPE), VPX)
		C_OBJS   += coopRam.o
		CFLAGS += -DCUTE
	endif
	
	ifeq ($(BTYPE), CPCI)
		C_OBJS   += coopRam.o
		C_OBJS   += cctInfo.o
		CFLAGS += -DCUTE
	endif
endif

DESTDIR = ../board/$(BOARD)/bin
DEPEND  = make.dep

# Primary Target
# --------------

all: $(DESTDIR)/$(TARGETLIB) $(DEPEND)

$(DEPEND): $(C_OBJS:.o=.c)
	$(CC) -E -M -I$(INCLUDE) -I$(BOARDINC) $(FREERTOSINC) -DGCC_CUTE $(C_OBJS:.o=.c) >$(DEPEND)

$(DESTDIR)/$(TARGETLIB):  $(TARGETLIB)
	cp $(TARGETLIB) $(DESTDIR)

$(TARGETLIB): $(LIBOBJS)
	$(AR) -rcs $(TARGETLIB) $(LIBOBJS)
	$(AR) -tv $(TARGETLIB)

include $(DEPEND)

# Secondary Targets

xmmx.o: xmmx.S
	$(AS) $(AFLAGS) xmmx.S -o xmmx.o

fpu.o: fpu.S
	$(AS) $(AFLAGS) fpu.S -o fpu.o


# Minor Targets

clean:
	rm -f *.o
	rm -f *.a
	rm $(DEPEND)
