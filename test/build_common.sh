#!/bin/sh

################################################################################
#
# Filename: 	build.sh
#
# Description:	Simple build script. 
#               Invoked from build)cute.sh and build_bit.sh
#
# $Revision: 1.1 $
#
# $Date: 2014-10-28 12:42:04 $
#
# $Source: /home/cvs/cvsroot/CCT_BIT_2/build_common.sh,v $
#
# Copyright 2012-2014 Concurrent Technologies, Plc.
#
################################################################################

# arguments are passed from build_cute.sh or build_bit.sh
# all arguments have been validated before invoking this script

boardName="$1"
busType="$2"
buildType="$3"
releaseType="$4"

# debug
#echo "$busType $boardName $buildType $releaseType"

################################################################################
if [ "$buildType" = "MAKE_CUTE" ]
then
	echo
	if [ "$releaseType" = "clean" ]
	then
		make BOARD=$boardName clean;
		exit
	elif [ "$releaseType" = "RELEASE" ]
	then
		echo "$busType $boardName RELEASE BUILD"
	elif [ "$releaseType" = "" ]
	then
		echo "$busType $boardName DEVELOPMENT BUILD"
	fi
	echo "$buildType USE_BOARD_SWITCHES"
	echo
	echo "Generating make.log..."
	make BOARD=$boardName BITFLAG=$buildType BTYPE=$busType RELARGS=$releaseType STARTUPFLAG=USE_BOARD_SWITCHES > make.log 2>&1
	echo "done"
else
	echo
	if [ "$releaseType" = "clean" ]
	then
		make BOARD=$boardName clean;
		exit
	elif [ "$releaseType" = "RELEASE" ]
	then
		echo "$busType $boardName RELEASE BUILD"
	elif [ "$releaseType" = "" ]
	then
		echo "$busType $boardName DEVELOPMENT BUILD"
	fi
	echo "$buildType USE_STARTUP_FLAGS"
	echo
	echo "Generating make.log..."
	make BOARD=$boardName BITFLAG=$buildType BTYPE=$busType RELARGS=$releaseType STARTUPFLAG=USE_STARTUP_FLAGS > make.log 2>&1
	echo "done"
fi
