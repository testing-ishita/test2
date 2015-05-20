#!/bin/sh

################################################################################
#
# Filename: 	build.sh
#
# Description:	Simple build script. 
#               Set boardName and busType below as appropriate.
#
# $Revision: 1.15 $
#
# $Date: 2015-04-02 11:46:16 $
#
# $Source: /home/cvs/cvsroot/CCT_BIT_2/build.sh,v $
#
# Copyright 2012-2013 Concurrent Technologies, Plc.
#
################################################################################

# Virtual PC
# busType="PCBRD"
# boardName="pc.brd"

# CPCI
busType="CPCI"
boardName=am90x.x1x
#boardName=am91x.x1x
#boardName=am92x.x1x
#boardName="pp81x.01x"

# SRIO
#busType="SRIO"
#boardName="am93x.x1x"
#boardName="am94x.x1x"
#boardName="amc1x.msd"

# VME
#busType="VME"
#boardName=vx813.09x
#boardName=vp717.x8x
#boardName=vp91x.01x
#boardName=vp92x.x1x

# VPX
#busType="VPX"
#boardName=tr803.39x
#boardName=trb1x.msd
#boardName=vre1x.msd


################################################################################

#buildType="MAKE_BIT"
buildType="MAKE_CUTE"

################################################################################


#releaseType="RELEASE"
releaseType=""

################################################################################
if [ "$buildType" = "MAKE_CUTE" ]
then
	echo
	if [ "$releaseType" = "RELEASE" ]
	then
		echo "$busType $boardName RELEASE BUILD"
	else
		echo "$busType $boardName DEVELOPMENT BUILD"
	fi
	echo "$buildType USE_BOARD_SWITCHES"
	echo
	if [ "$1" = "" ]
	then 
		make BOARD=$boardName BITFLAG=$buildType BTYPE=$busType RELARGS=$releaseType STARTUPFLAG=USE_BOARD_SWITCHES
	elif [ "$1" = "log" ]
	then
		echo "Generating make.log..."
		make BOARD=$boardName BITFLAG=$buildType BTYPE=$busType RELARGS=$releaseType STARTUPFLAG=USE_BOARD_SWITCHES > make.log 2>&1
		echo "done"
	elif [ "$1" = "clean" ]
	then
		make BOARD=$boardName clean
	else
		echo "Invalid argument given"
	fi
else
	echo
	if [ "$releaseType" = "RELEASE" ]
	then
		echo "$busType $boardName RELEASE BUILD"
	else
		echo "$busType $boardName DEVELOPMENT BUILD"
	fi
	echo "$buildType USE_STARTUP_FLAGS"
	echo
	if [ "$1" = "" ]
	then 
		make BOARD=$boardName BITFLAG=$buildType BTYPE=$busType RELARGS=$releaseType STARTUPFLAG=USE_STARTUP_FLAGS
	elif [ "$1" = "log" ]
	then
		echo "Generating make.log..."
		make BOARD=$boardName BITFLAG=$buildType BTYPE=$busType RELARGS=$releaseType STARTUPFLAG=USE_STARTUP_FLAGS > make.log 2>&1
		echo "done"
	elif [ "$1" = "clean" ]
	then
		make BOARD=$boardName clean
	else
		echo "Invalid argument given"
	fi
fi
