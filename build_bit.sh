#!/bin/sh

################################################################################
#
# Filename: 	build_bit.sh
#
# Description:	Simple build script. 
#               Pass board name, bus type and build type to main build script.
#
# $Revision: 1.6 $
#
# $Date: 2015-04-02 11:46:16 $
#
# $Source: /home/cvs/cvsroot/CCT_BIT_2/build_bit.sh,v $
#
# Copyright 2014 Concurrent Technologies, Plc.
#
################################################################################

# Usage examples:
#   ./build_bit.sh amc1x
#   ./build_bit.sh trb1x release
#   ./build_bit.sh vre1x clean

buildType="MAKE_BIT"
busType=""
boardName=""

if [ "$1" = "am90x" ]
then
		busType="CPCI";
		boardName="am90x.x1x"
elif [ "$1" = "am91x" ]
then
		busType="CPCI";
		boardName="am91x.x1x"
elif [ "$1" = "am92x" ]
then
		busType="CPCI";
		boardName="am92x.x1x"
elif [ "$1" = "am93x" ]
then
		busType="SRIO";
		boardName="am93x.x1x"
elif [ "$1" = "am94x" ]
then
		busType="SRIO";
		boardName="am94x.x1x"
elif [ "$1" = "amc1x" ]
then
		busType="SRIO";
		boardName="amc1x.msd"
elif [ "$1" = "pc" ]
then
		# Virtual PC
		busType="PCBRD";
		boardName="pc.brd"
elif [ "$1" = "pp81x" ]
then
		busType="CPCI";
		boardName="pp81x.01x"
elif [ "$1" = "tr803" ]
then
		busType="VPX";
		boardName="tr803.39x"
elif [ "$1" = "trb1x" ]
then
		busType="VPX";
		boardName="trb1x.msd"
elif [ "$1" = "vp71x" ]
then
		busType="VME";
		boardName="vp717.x8x"
elif [ "$1" = "vp91x" ]
then
		busType="VME";
		boardName="vp91x.01x"
elif [ "$1" = "vp92x" ]
then
		busType="VME";
		boardName="vp92x.x1x"
elif [ "$1" = "vre1x" ]
then
		busType="VPX";
		boardName="vre1x.msd"
elif [ "$1" = "vx813" ]
then
		busType="VME";
		boardName="vx813.09x"
else
	echo;
	echo "'$1' - Unrecognised board name";
	exit
fi

if [ "$2" = "clean" ]
then 
	sh ./build_common.sh $boardName $busType $buildType clean
elif [ "$2" = "release" ]
then
	sh ./build_common.sh $boardName $busType $buildType RELEASE
elif [ "$2" = "" ]
then
	sh ./build_common.sh $boardName $busType $buildType
else
	echo;
	echo "'$2' - Unrecognised parameter";
	echo;
	echo "Valid command syntax:";
	echo "    ./build_bit.sh 'boardName'";
	echo "    ./build_bit.sh 'boardName' clean";
	echo "    ./build_bit.sh 'boardName' release"
fi
