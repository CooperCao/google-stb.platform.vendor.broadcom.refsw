#! /bin/bash
#
# build_epi412xx.sh - Do a checkout/release for epi412xx drivers & tools
#
# Copyright 1999 Epigram, Inc.
#
# $Id$

PATH=/tools/local/bin:/winnt/system32:/winnt:/tools/build

BUILD_DIR=$1
BUILD_BASE=$2

if [ "x${BUILD_DIR}" = "x" -o  "x${BUILD_DIR}" = "xtop" ]; then
	BUILD_DIR="BUILD_"`date '+%y%m%d'`
	TAG=
else
	TAG=" -r ${BUILD_DIR}"
fi;

if [ "x$BUILD_BASE" = "x" ]; then
	BUILD_BASE="//e/build"
fi;

cd ${BUILD_BASE}
if [ $? != 0 ]; then
    echo Cannot cd to ${BUILD_BASE}
    exit 1
fi;

if [ -d ${BUILD_DIR} ]; then
    # Find the next backup dir name
    let i=1
    let max=20
    while (( $i <= $max ))
    do
	if [ ! -d ${BUILD_DIR}-$i ]
	then
	    break
	fi;
	let i=i+1;
    done
    # Make the backup if we have not tried too many names
    if (( $i <= $max ))
    then
	PREV_BUILD="  (Renaming existing ${BUILD_DIR} to ${BUILD_DIR}-$i)"
	mv ${BUILD_DIR} ${BUILD_DIR}-$i
	if [ $? != 0 ]; then
	    echo Warning: Cannot rename old ${BUILD_DIR} to ${BUILD_DIR}-$i
	fi;
    else
	echo Warning: Failed to move ${BUILD_DIR} out of the way after
	echo trying $i directory names.
    fi;
fi;

BUILD_DIR_ORIG=${BUILD_DIR}
let i=0
while ! mkdir $BUILD_DIR
do
    echo Cannot create new ${BUILD_DIR}
    if (( $i < 10 ))
    then
	BUILD_DIR=${BUILD_DIR_ORIG}_${i}
        echo Trying ${BUILD_DIR} instead.
	let i=i+1;
    else
	echo "Giving up."
	exit 1;
    fi;
done

cd $BUILD_DIR
if [ $? != 0 ]; then
    echo Cannot cd to ${BUILD_DIR}
    exit 1
fi;

LOG=${BUILD_BASE}/${BUILD_DIR}/,build.log

echo "Running build_epi412xx.sh"				> $LOG 2>&1
date								>> $LOG 2>&1
echo "PWD = ${PWD}"						>> $LOG 2>&1
echo "BUILD_BASE = ${BUILD_BASE}"				>> $LOG 2>&1
echo "BUILD_DIR = ${BUILD_DIR}"					>> $LOG 2>&1
echo $PREV_BUILD						>> $LOG 2>&1
echo "TAG = ${TAG}"						>> $LOG 2>&1

export CVSROOT=":server:build@frodo:/epigram/cvsroot"
date								>> $LOG 2>&1
echo "Doing cvs checkout epi412xx-src, CVSROOT= ${CVSROOT}"	>> $LOG 2>&1
cvs co ${TAG} epi412xx-src > ,checkout.log 2>&1
cvsrc=$?
echo "Checkout returned: $cvsrc"				>> $LOG 2>&1

export RELEASE_DIR=${BUILD_BASE}/${BUILD_DIR}/release
export MAKE_MODE=unix
export LOCAL=c:/tools/local
export MSDEV=c:/tools/msdev/Studio
export MSSDK=c:/tools/msdev/mssdk
export DDK=c:/tools/msdev/ddk
export WDMDDK=c:/tools/msdev/nt50ddk

cd src
source tools/build/build_env.sh

date								>> $LOG 2>&1
echo "Doing gmake release, RELEASE_DIR = ${RELEASE_DIR}" >> $LOG 2>&1
gmake -k -w release > ../,release.log 2>&1
gmakerc=$?
echo \'gmake release\' returned: $gmakerc		>> $LOG 2>&1
fgrep -q '**' ../,release.log					>> $LOG 2>&1
fgrc=$?
echo \'fgrep release\' returned: $fgrc				>> $LOG 2>&1

cd ${BUILD_BASE}/${BUILD_DIR}
blat `basename $LOG` -t hnd-build-list@broadcom.com -f build@broadcom.com \
	-s "Build for ${BUILD_DIR} done"
