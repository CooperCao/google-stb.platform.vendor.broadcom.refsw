#!/bin/bash

# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
# $Id: hndcvs_cvs_report.sh,v 1.17 2011-02-01 00:37:20 $

# cron/LSF user may not have a suitable environment
PATH=/tools/bin:/bin:/projects/hnd/tools/linux/bin
CVSROOT=/projects/cvsroot
export CVSROOT


if [ "$BASE_DIR" = "" ]; then
BASE_DIR=/projects/hnd_software/gallery/BOMS/
fi

OUT_DIR=""

if [ "$1" = "-d" ]; then
    DATE=$2
    shift 2
else
    DATE=`date +"%Y_%m_%d"`
fi

if [ "$1" = "-o" ]; then
    OUT_DIR=$2
    shift 2
fi

cvs co -p src/hndcvs/bomed_list.txt > bomed_list.txt
source  bomed_list.txt

if [ "$1" = "-t" ]; then
    BOM_LIST="$2"
    shift 2
fi

if [ "$BOM_LIST" = "" ]; then
    exit
fi

# suppress new lines and extra blanks
for var in $BOM_LIST; do
list=""
for l in ${!var}; do
    list="$list $l"
done

if test ! -f $BASE_DIR/$var/makefile; then
    echo missing directory $var
    mkdir -p $BASE_DIR/$var
    cvs co -p src/hndcvs/make_reports.mk > $BASE_DIR/$var/makefile
fi

if [ "$var" = "TOT" ]; then
    make -s BOM_LIST="$list" DATE=$DATE -C "$BASE_DIR""$var" modules-list-bom
else
    make -s BOM_LIST="$list" DATE=$DATE TAG=$var -C "$BASE_DIR""$var" modules-list-bom
fi
done

if [ "$OUT_DIR" != "" ]; then
    for dir in $BOM_LIST; do
	TAG=$dir
	REPORT="$BASE_DIR"$dir/hndcvs_cvsreport.modules-list-bom.$TAG.$DATE.html
	if test -e $REPORT; then
	    echo "${DATE}_b$TAG.html"
	    cp $REPORT $OUT_DIR/${DATE}_b$TAG.html
	fi
    done
fi
