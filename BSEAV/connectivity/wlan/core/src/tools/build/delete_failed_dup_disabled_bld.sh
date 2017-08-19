#!/bin/bash

## Broadcom Proprietary and Confidential. Copyright (C) 2017,
## All Rights Reserved.
## 
## This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
## the contents of this file may not be disclosed to third parties, copied
## or duplicated in any form, in whole or in part, without the prior
## written permission of Broadcom.
##
## Delete the build directories listed in the files passed to this script on
## command line.
## Usually called by find_failed_builds.sh -delete
##
## $Id: delete_failed_dup_disabled_bld.sh,v 12.5 2011/02/13 07:17:33 Exp $

filelist=$*

if [ -z "$filelist" ]; then
    echo "ERROR: Usage: $0 [<file-containing-list-of-failed-n-duplicate-builds>"
    exit 1
fi

for b in `cat $filelist | grep -v "^#" | col -b | grep "/projects/hnd.swbuild"`
do
    brand=${b%/*}
    disabled=
    if echo "$b" | grep -q -i "/_disabled/"
    then
        echo "NOTE: $b is a non-default build"
        echo "NOTE: "
        echo "[`date`] START: Delete $b"
        disabled=${brand%/*}
    elif [ -f "$b/,succeeded" ]; then
        echo "WARN: $b isn't a failed build, it may be a duplicate build"
        echo "WARN: FORCING SKIPPING OF SUCCEEDED BUILD BRANDS"
        continue
        if [ "$brand" == "win_external_wl" ]; then
           echo "INFO: $brand/$b shows gold releases, skipping it"
           found_releases=$(find $b/release/Win*/*Gold* $b/release/Win*/*Drop* 2> /dev/null)
           continue
        fi
    elif [ -f "$b/,build_errors.log" ]; then
        echo "[`date`] START: Delete $b"
        echo "--------- BEGIN BUILD ERRORS ---------"
        cat $b/,build_errors.log
        echo "--------- END BUILD ERRORS -----------"
    else
        echo "WARN:"
        echo "WARN: $b isn't a failed build"
        echo "WARN:"
        verify="$verify $b"
        continue
    fi
    pushd $b > /dev/null 2>&1; pwd -P; popd > /dev/null 2>&1
    rm -rf $b/*
    rmdir -v $b $brand $disabled # attempt to remove empty parent dirs
    echo "[`date`] END  : Delete $b"
    echo "============================================="
done

if [ -n "$verify" ]; then echo "Verify: $verify"; fi
