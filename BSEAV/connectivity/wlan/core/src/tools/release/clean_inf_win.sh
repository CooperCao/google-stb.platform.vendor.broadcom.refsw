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
# echo params $1 $2 $3

srcdir=$1
inffile=$2
win7win8=$3
win10=$4

echo "remove comments from inf  now "
	sed \
	-e "/$win7win8/d" \
	-e "/$win10/d" \
	$srcdir/$inffile.inf > $srcdir/$inffile.inf.tmp;
	mv $srcdir/$inffile.inf.tmp $srcdir/$inffile.inf;
