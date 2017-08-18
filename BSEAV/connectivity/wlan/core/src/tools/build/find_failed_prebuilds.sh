#!/bin/bash

## Broadcom Proprietary and Confidential. Copyright (C) 2017,
## All Rights Reserved.
## 
## This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
## the contents of this file may not be disclosed to third parties, copied
## or duplicated in any form, in whole or in part, without the prior
## written permission of Broadcom.
##
## Search the build storage directories for any failed prebuild (firmware) attempts
##
## $Id: find_failed_prebuilds.sh,v 12.1 2009/09/19 08:57:38 Exp $

uname | grep -i cygwin_nt > /dev/null 2>&1

if [ "$?" = "0" ]; then
   DRIVE="Z:"
else
   DRIVE=
fi

verbose=
if [ "$1" == "verbose" ]; then
   verbose=1
fi

BUILD_ROOT=/projects/hnd/swbuild
PREBUILD_DIRS=(${DRIVE}${BUILD_ROOT}/build_linux/TEMP/prebuild)

timestamp=`date '+%Y%m%d'`

# Output filenames
failed="${PWD}/failed_builds_${timestamp}.txt"
duplicate="${PWD}/duplicate_builds_${timestamp}.txt"
verify="${PWD}/verify_builds_${timestamp}.txt"

# Cleanup old history
rm -fv $failed;    touch $failed
rm -fv $duplicate; touch $duplicate
rm -fv $verify;    touch $verify

# Initialize the output files
echo -e "\nFAILED BUILDS TO BE DELETED as of `date`:\n"    > $failed
echo -e "\nDUPLICATE BUILDS TO BE CLEANED as of `date`:\n" > $duplicate
echo -e "\nOTHERS BUILDS TO BE VERIFIED   as of `date`:\n" > $verify

# Keep count of number of verify, failed and duplicate build iterations
vindex=1
findex=1
dindex=1

## Search all PREBUILD_DIRS (production and sw_archive_ro area)

for idir in $(seq 0 ${#PREBUILD_DIRS[*]}); do
    builddir=${PREBUILD_DIRS[$idir]}
    [ ! -n "$builddir" ] && continue
    BUILD_DIR="${builddir}"
    echo "BUILD_DIR1=$BUILD_DIR"
    [ ! -d "$BUILD_DIR" ] && continue
    echo -e "\n===== START: SCAN $(basename $builddir) BUILDS =====\n"
    echo "#==== START: FAILED $(basename $builddir) BUILDS ====" >> ${failed}
    echo "#==== START: DUPLICATE $(basename $builddir) BUILDS ====" >> ${duplicate}
    echo "#==== START: VERIFY $(basename $builddir) BUILDS ====" >> ${verify}

    ## Search all builds in a build_<dir>. Sample prebuild dir layout
    ## /projects/hnd/swbuild/build_linux/TEMP/prebuild/ce_external_dongle_sdio/RAPTOR2_REL_4_217_46

    cd $BUILD_DIR
    ## Search all DHD/host side builds
    HOSTBRANDS=(`ls -1`)
    echo "HOSTBRANDS = ${#HOSTBRANDS[*]}"
    for ibrand in $(seq 0 ${#HOSTBRANDS[*]}); do
        brandname=${HOSTBRANDS[$ibrand]}
        [ ! -n "$brandname" ] && continue
        BUILD_DIR="${builddir}/${brandname}"
        [ ! -d "$BUILD_DIR" ] && continue
        [ -n "$verbose" ] && echo " -- Searching $BUILD_DIR"

        ## Search all tags in a build

        cd $BUILD_DIR
        TAGS=(`ls -1`)
        for itag in $(seq 0 ${#TAGS[*]}); do
            tagname=${TAGS[$itag]}
            if echo $tagname | grep -iq "NIGHTLY\|_BRANCH_\|_TWIG_"; then
               continue
            fi
            [ ! -n "$tagname" ] && continue
            BUILD_DIR="${builddir}/${brandname}/${tagname}"
            [ ! -d "$BUILD_DIR" ] && continue

    	    ## Search all DHD/host side builds

            cd $BUILD_DIR
            PREBRANDS=(`ls -1`)
            for iprebrand in $(seq 0 ${#PREBRANDS[*]}); do
                ## Search all pre-build firmware brands
            	prebrand="${PREBRANDS[$iprebrand]}"
            	BUILD_DIR="${builddir}/${brandname}/${tagname}/${prebrand}"

            	cd $BUILD_DIR
                ATTEMPTS=(`echo 2* | fmt -1 | perl -ne 's/\.(\d)\.(\d)\./\.0$1\.0$2\./g; s/\.(\d)\./\.0$1\./g; print $_' | sort -r | xargs echo`)
                succeeded=0
                duplicate_build=0
                failed_build=0
                verify_build=0
                keep_build=0
                ## Search all build iterations
                for iattempt in $(seq 0 ${#ATTEMPTS[*]}); do
                    attemptnum=${ATTEMPTS[$iattempt]}
                    attemptnum=`echo $attemptnum | perl -ne 's/\.0(\d)\.0(\d)\./\.$1\.$2\./g; s/\.0(\d)\./\.$1\./g; print $_'`
                    [ ! -n "$attemptnum" ] && continue
                    BUILD_DIR="${builddir}/${brandname}/${tagname}/${prebrand}/${attemptnum}"
                    BUILD_ITER=`echo $BUILD_DIR | sed -e "s%${BUILD_ROOT}/%%g"`
                    [ ! -d "$BUILD_DIR" ] && continue
                    cd $BUILD_DIR
                    releaselog=",release.log"
                    errorlog=",build_errors.log"
                    succeededlog=",succeeded"

                    if [ -s "$releaselog" ]
                    then
                       if [ -f "${errorlog}" ]
                       then
                          # Build iteration failed
                          echo $findex | \
                               xargs printf "%3d: Failed   : ${BUILD_ITER}\n"
                          echo "${BUILD_DIR}" >> ${failed}
                          findex=$(($findex + 1))
                          failed_build=1
                          ## In rare situations, there are errors, but succeeded
                          ## is generated (or it is a manual created build)
                          errors=$(grep "Error [0-9]\+[[:space:]]*$" ${releaselog})
                          if [ -n "${errors}" ]; then
                             echo $vindex | \
                                  xargs printf "%3d: Verify   : ${BUILD_ITER}\n"
                             echo "${BUILD_DIR}" >> ${verify}
                             verify_build=1
                             vindex=$(($vindex + 1))
                          fi
                       elif [ -f "${succeededlog}" ]; then
                          # Build iteration succeeded, check for duplicates
                          if [ $succeeded -gt 0 ]; then
                             if [ "$keep_build" != "1" ]; then
                                echo "   : *Keep*   : ${prev_build_iter}"
                                keep_build=1
                             fi
                             echo $dindex | \
                                  xargs printf "%3d: Duplicate: ${BUILD_ITER}\n"
                             echo "${BUILD_DIR}" >> ${duplicate}
                             dindex=$(($dindex + 1))
                             duplicate_build=1
                          fi
                          succeeded=$(($succeeded + 1))
                          prev_build_iter=${BUILD_ITER}
                       else
                          # Neither errorlog nor succeeded log exists!
                          echo $vindex | \
                               xargs printf "%3d: Verify   : ${BUILD_ITER}\n"
                          echo "${BUILD_DIR}" >> ${verify}
                          verify_build=1
                          vindex=$(($vindex + 1))
                       fi # errorlog

                    else # releaselog is missing

                       echo $vindex | \
                            xargs printf "%3d: Verify   : ${BUILD_ITER}\n"
                       echo "${BUILD_DIR}" >> ${verify}
                       verify_build=1
                       vindex=$(($vindex + 1))

                    fi # releaselog
                done #iterations/attempt
            done #prebuild-brands

        if [ -n "$verbose" -a "$duplicate_build" == "1" ]; then
           echo "     All Iters=`basename $builddir`/$brandname/$brandname/`echo ${ATTEMPTS[*]} | perl -ne 's/\.0(\d)\.0(\d)\./\.$1\.$2\./g; s/\.0(\d)\./\.$1\./g; print $_'`"
        fi
        done #brand

    done #build

    echo "#==== END  : FAILED $(basename $builddir) BUILDS ====" >> ${failed}
    echo "#==== END  : DUPLICATE $(basename $builddir) BUILDS ====" >> ${duplicate}
    echo "#==== END  : VERIFY $(basename $builddir) BUILDS ====" >> ${verify}
    echo -e "\n===== END  : SCAN $(basename $builddir) BUILDS =====\n"
done #dir

unix2dos $failed $duplicate $verify  > /dev/null 2>&1
findex=$(($findex - 1))
dindex=$(($dindex - 1))
vindex=$(($vindex - 1))
echo ""
echo "Found $findex FAILED pre-build iterations to delete (in $failed)"
echo "Found $dindex DUPLICATE pre-build iterations to clean (in $duplicate)"
echo "Found $vindex OTHER pre-build iterations to verify (in $verify)"
