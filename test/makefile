
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

# MAKEFILE - Top-level Build Control File
#

# Version control:
#
# $Header: /home/cvs/cvsroot/CCT_BIT_2/makefile,v 1.4 2014-01-29 13:13:37 hchalla Exp $
#
# $Log: makefile,v $
# Revision 1.4  2014-01-29 13:13:37  hchalla
# Makefile modified for AM 94x SRIO Testing.
#
# Revision 1.3  2013-09-26 12:01:28  chippisley
# Removed obsolete HAL.
#
# Revision 1.2  2013/09/04 12:22:48  chippisley
# Added lwip.
#
# Revision 1.1  2013/09/04 07:07:10  chippisley
# Import files into new source repository.
#
# Revision 1.18  2012/03/27 08:08:11  cdobson
# Added code to save that last build target and to perform a make clean it a
# different target is built, otherwise libraries may have been built for a different
# board type.
#
# Revision 1.17  2011/02/01 12:12:06  hmuneer
# C541A141
#
# Revision 1.16  2011/01/20 10:01:13  hmuneer
# CA01A151
#
# Revision 1.15  2010/10/06 10:14:11  jthiru
# PBIT coding for VP717
#
# Revision 1.14  2010/09/15 12:27:23  hchalla
# Added support for TR 501 BIT/CUTE
#
# Revision 1.13  2010/07/26 13:14:09  hmuneer
# Source Cleanup
#
# Revision 1.12  2010/06/23 11:04:05  hmuneer
# CUTE V1.01
#
# Revision 1.11  2010/05/05 15:52:49  hmuneer
# Cute rev 0.1
#
# Revision 1.10  2009/06/29 16:05:32  swilson
# Improved parameter checking: check both 'no board' and 'invalid board' conditions.
#  Suppressed checking lines to make error message clearer.
#
# Revision 1.9  2009/06/01 10:07:04  swilson
# Fixed passing of RELARGS parameter to the board directory.
#
# Revision 1.8  2008/09/22 16:15:20  swilson
# Add parameter-passing to makefile so additional warnings can be passed to build and
#  board type can be specified.
#
# Revision 1.7  2008/06/20 12:16:51  swilson
# Foundations of PBIT and PCI allocation. Added facility to pass startup options into
#  C-code.
#
# Revision 1.6  2008/05/16 14:36:19  swilson
# Add test list management and test execution capabilities.
#
# Revision 1.5  2008/05/12 15:04:40  swilson
# Add IBIT console. Debugging various library functions.
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

str1	= "No board name defined, use BOARD=<board_name>."
str2	= "Board '$(BOARD)' not found."
str3	= "No Compilation Flag Present, use BITFLAG=<MAKE_BIT>/<MAKE_CUTE>/<MAKE_BIT_CUTE>"
str4	= "Compiling BIT Only"
str5	= "Compiling CUTE Only"
str6	= "Compiling Both BIT & CUTE"
str7	= "Wrong Compilation Flag, use BITFLAG=<MAKE_BIT>/<MAKE_CUTE>/<MAKE_BIT_CUTE>"

BOARD	=
WARN	=
BITFLAG = 
RELARGS =
STARTUPFLAG =
BTYPE =
LAST  := $(shell if test -f last ; then cat last; else echo none; fi)

# Primary Target
# --------------

#board/$(BOARD)/all: FORCE
all: FORCE
	@if test '$(BOARD)' = ''; then echo $(str1); exit 1; fi
	@if test -d board/$(BOARD); then echo ""; else echo $(str2); exit 2; fi  
	@if test '$(BITFLAG)' = ''; then echo $(str3); exit 4;\
	 elif test '$(BITFLAG)' = 'MAKE_BIT'; then echo $(str4);\
	 elif test '$(BITFLAG)' = 'MAKE_CUTE'; then echo $(str5);\
	 elif test '$(BITFLAG)' = 'MAKE_BIT_CUTE'; then echo $(str6);\
	 else echo $(str7); exit 5; fi
	@if test '$(LAST)' != '$(BOARD)'; then (make BOARD=$(BOARD) clean);fi
	@echo $(BOARD)>last
	(cd board/$(BOARD); make all RELARGS=$(RELARGS) WARN=$(WARN) \
	    BITFLAG=$(BITFLAG) STARTUPFLAG=$(STARTUPFLAG) BOARD=$(BOARD))
	

FORCE:

clean:
	@if test '$(BOARD)' = ''; then echo $(str1); exit 1; fi
	@if test -d board/$(BOARD); then echo ""; else echo $(str2); exit 2; fi  
	(cd board/$(BOARD); make clean)
	(cd blib; make clean)
	(cd clib; make clean)
	(cd conslib; make clean)
	(cd ibit; make clean)
	(cd mth; make clean)
	(cd kernel; make clean)
	(cd loader; make clean)
	(cd pbit; make clean)
	(cd sth; make clean)
	(cd tests; make clean)
	(cd arch; cd vme; make clean)
	(cd arch; cd cpci; make clean)	
	(cd arch; cd vpx; make clean)
	(cd arch; cd srio; make clean)	
	(cd xlib; cd interconnect; make clean)
	(cd xlib; cd sm3; make clean)	
	(cd xlib; cd idx; make clean)
	(cd FreeRTOS/Demo/CUTE; make clean; rm -rf *.o)
	(cd comm; cd serial; make clean)
	(cd vgalib; make clean)
	(cd debug; make clean)
	(cd lwip-1.4.0; make clean)

# Not used, HAL no longer required
# 	(cd hal; make clean)
