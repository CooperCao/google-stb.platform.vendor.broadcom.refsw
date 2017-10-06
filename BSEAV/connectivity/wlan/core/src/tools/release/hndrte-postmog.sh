#!/bin/bash
#
# Miscellaneous post-build sanitization that mogrify.pl was not built
# for.  Removes stuff that should not be part of the hndrte (dongle)
# release, which is implicitly partial source.
#
# Based on linux-postbuild.sh at this point, to preserve flexibility,
# but once release process stabilize should convert to regular mogrify
# if possible.
#
# Broadcom Proprietary and Confidential. Copyright (C) 2017,
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom.
#
# $Id$
#

#
# Simple filter of bad words in src/include/wlioctl.h
# No ioctls here yet until safe onces decided on.


#
# Simple filter of unannounced boardflags in src/include/bcmdevs.h
#

if [ -f src/include/bcmdevs.h ] ; then
TEMP=`mktemp src/include/bcmdevs.h.XXXXXX`
grep -v -f - src/include/bcmdevs.h > ${TEMP} <<EOF
BFL_ADCDIV
BFL_NOPLLDOWN
EOF
cp ${TEMP} src/include/bcmdevs.h
rm -f ${TEMP}
fi

#
# Simple filter of unannounced chips in src/include/bcmdevs.h
#

if [ -f src/include/bcmdevs.h ] ; then
TEMP=`mktemp src/include/bcmdevs.h.XXXXXX`
grep -v -f - src/include/bcmdevs.h > ${TEMP} <<EOF
BCM94306P50_BOARD
MPSG4306_BOARD
4306/gprs combo
BCM94306GPRS_BOARD
BCM5365/BCM4704 FPGA Bringup Board
BU5365_FPGA_BOARD
BCM4331_CHIP_ID
BCM4331_D11N2G_ID
BCM4331_D11N_ID
BCM4331_D11N5G_ID
# just in case
BCM4785_CHIP_ID
MIMO_FPGA_ID
# not sure
JINVANI
EOF
cp ${TEMP} src/include/bcmdevs.h
rm -f ${TEMP}
fi

#
# Simple filter of bad words in src/include/sbconfig.h
#

if [ -f src/include/sbconfig.h ] ; then
TEMP=`mktemp src/include/sbconfig.h.XXXXXX`
grep -v -f - src/include/sbconfig.h > ${TEMP} <<EOF
SB_ARM11
EOF
cp ${TEMP} src/include/sbconfig.h
rm -f ${TEMP}
fi

exit 0
