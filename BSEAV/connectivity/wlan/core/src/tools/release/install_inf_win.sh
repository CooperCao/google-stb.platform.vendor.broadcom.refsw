#!/bin/bash
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# echo params $1 $2 $3 $4 $5 $6 $7

SRCDIR=$1
INFFILE=$2
WIN7WIN8=$3
NDISVERSTR=$4
ARCH=$5
NDISARCH=$6
DRVNAME=$7
DIRNAME=%DRIVER_FILENAME%
NDISMVER=%ARCH%
NDISSTR=%NDISARCH%

echo "Generating dhd inf  now "
	sed \
		-e "/;;DELETE/d" \
		-e "s/;$WIN7WIN8//gi"	\
		-e "s/%NDISVER%/$NDISVERSTR/gi" \
		-e "s/$DIRNAME/$DRVNAME/gi" \
		-e "s/$NDISMVER/$ARCH/gi" \
		-e "s/$NDISSTR/$NDISARCH/gi" \
	$SRCDIR/$INFFILE.inf > $SRCDIR/$INFFILE.inf.tmp;
	mv $SRCDIR/$INFFILE.inf.tmp $SRCDIR/$INFFILE.inf;
