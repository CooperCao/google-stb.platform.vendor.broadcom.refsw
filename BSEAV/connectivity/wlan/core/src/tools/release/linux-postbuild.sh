#!/bin/bash
#
# Miscellaneous post-build sanitization that mogrify.pl was not built
# for. Note that each section is bound by mogrification directives,
# which remain the primary differentiator of the various source
# release packages.
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

#ifndef WLSRC

#
# Simple filter of bad words in src/include/wlioctl.h
#


if [ -f src/include/wlioctl.h ] ; then
TEMP=`mktemp src/include/wlioctl.h.XXXXXX`
grep -v -f - src/include/wlioctl.h > ${TEMP} <<EOF
WLC_GET_PM
WLC_SET_PM
WLC_GET_PHY_NOISE
EOF
cp ${TEMP} src/include/wlioctl.h
rm -f ${TEMP}
fi

if [ -f src/include/wlioctl.h ] ; then
TEMP=`mktemp src/include/wlioctl.h.XXXXXX`

# Remove from /* Begin: tx_power* */ to /* End: tx_power* */
# Remove from /* Begin: wl_aci_args_t* */ to /* End: wl_aci_args_t* */
sed -e '/^\/\* Begin: tx_power/,/^\/\* End: tx_power/d' \
    -e '/^\/\* Begin: wl_aci_args_t/,/^\/\* End: wl_aci_args_t/d' \
    src/include/wlioctl.h > ${TEMP}
cp ${TEMP} src/include/wlioctl.h

# Remove line matches
grep -v -f - src/include/wlioctl.h > ${TEMP} <<EOF
WLC_GET_LOOP
WLC_SET_LOOP
WLC_DUMP_SCB
WLC_DUMP_RATE
WLC_SET_RATE_PARAMS
WLC_GET_PASSIVE_SCAN
WLC_SET_PASSIVE_SCAN
WLC_GET_ROAM_DELTA
WLC_SET_ROAM_DELTA
WLC_GET_ROAM_SCAN_PERIOD
WLC_SET_ROAM_SCAN_PERIOD
WLC_GET_WAKE
WLC_SET_WAKE
WLC_GET_D11CNTS
WLC_GET_FORCELINK
WLC_SET_FORCELINK
WLC_GET_PHYREG
WLC_SET_PHYREG
WLC_GET_RADIOREG
WLC_SET_RADIOREG
WLC_GET_UCANTDIV
WLC_SET_UCANTDIV
WLC_R_REG
WLC_W_REG
WLC_DIAG_LOOPBACK
WLC_RESET_D11CNTS
WLC_GET_MONITOR
WLC_SET_MONITOR
WLC_GET_LEGACY_ERP
WLC_SET_LEGACY_ERP
WLC_GET_RX_ANT
WLC_GET_ATIM
WLC_SET_ATIM
WLC_GET_PHYANTDIV
WLC_SET_PHYANTDIV
WLC_AP_RX_ONLY
WLC_GET_TX_PATH_PWR
WLC_SET_TX_PATH_PWR
WLC_GET_PKTCNTS
WLC_GET_IGNORE_BCNS
WLC_SET_IGNORE_BCNS
WLC_GET_SCB_TIMEOUT
WLC_SET_SCB_TIMEOUT
WLC_GET_UCFLAGS
WLC_SET_UCFLAGS
WLC_GET_PWRIDX
WLC_SET_PWRIDX
WLC_GET_TSSI
WLC_GET_SUP_RATESET_OVERRIDE
WLC_SET_SUP_RATESET_OVERRIDE
WLC_SET_FAST_TIMER
WLC_GET_FAST_TIMER
WLC_SET_SLOW_TIMER
WLC_GET_SLOW_TIMER
WLC_DUMP_PHYREGS
WLC_ENCRYPT_STRENGTH
WLC_DECRYPT_STATUS
WLC_GET_SCAN_UNASSOC_TIME
WLC_SET_SCAN_UNASSOC_TIME
WLC_GET_SCAN_HOME_TIME
WLC_SET_SCAN_HOME_TIME
WLC_GET_SCAN_NPROBES
WLC_SET_SCAN_NPROBES
WLC_GET_PRB_RESP_TIMEOUT
WLC_SET_PRB_RESP_TIMEOUT
WLC_GET_ATTEN
WLC_SET_ATTEN
WLC_GET_SHMEM
WLC_SET_SHMEM
WLC_SET_WSEC_TEST
WLC_SET_ASSOC_PREFER
WLC_GET_ASSOC_PREFER
WLC_SET_ROAM_PREFER
WLC_GET_ROAM_PREFER
WLC_GET_INTERFERENCE_MODE
WLC_SET_INTERFERENCE_MODE
WLC_GET_CHANNEL_QA
WLC_START_CHANNEL_QA
WLC_GET_PWROUT_PERCENTAGE
WLC_SET_PWROUT_PERCENTAGE
WLC_SET_BAD_FRAME_PREEMPT
WLC_GET_BAD_FRAME_PREEMPT
WLC_SET_LEAP_LIST
WLC_GET_LEAP_LIST
WLC_GET_CWMIN
WLC_SET_CWMIN
WLC_GET_CWMAX
WLC_SET_CWMAX
WLC_SET_GLACIAL_TIMER
WLC_GET_GLACIAL_TIMER
WLC_DUMP_RSSI
WLC_EVM
WLC_FREQ_ACCURACY
WLC_CARRIER_SUPPRESS
WLC_DUMP_RADIOREGS
WLC_GET_ACI_ARGS
WLC_SET_ACI_ARGS
WLC_MEASURE_REQUEST
WLC_INIT
WLC_SEND_QUIET
WLC_CHANNEL_SWITCH
WLC_GET_SCAN_PASSIVE_TIME
WLC_SET_SCAN_PASSIVE_TIME
WLC_LEGACY_LINK_BEHAVIOR
diag loopback choices
DIAG_LOOPBACK
54g modes
Rateset:
Extended Rateset:
Preamble:
Shortslot:
interference mitigation options
INTERFERE_NONE
NON_WLAN
WLAN_MANUAL
WLAN_AUTO
AUTO_ACTIVE
802.11h measurement types
WLC_MEASURE_TPC
WLC_MEASURE_CHANNEL_BASIC
WLC_MEASURE_CHANNEL_CCA
WLC_MEASURE_CHANNEL_RPI
Message levels
WL_ERROR_VAL
WL_TRACE_VAL
WL_PRHDRS_VAL
WL_PRPKT_VAL
WL_INFORM_VAL
WL_TMP_VAL
WL_OID_VAL
WL_RATE_VAL
WL_ASSOC_VAL
WL_PRUSR_VAL
WL_PS_VAL
WL_TXPWR_VAL
WL_GMODE_VAL
WL_DUAL_VAL
WL_WSEC_VAL
WL_WSEC_DUMP_VAL
WL_LOG_VAL
WL_NRSSI_VAL
WL_LOFT_VAL
WL_REGULATORY_VAL
WL_ACI_VAL
WL_RADAR_VAL
WL_NITRO_VAL
maximum channels
EOF
cp ${TEMP} src/include/wlioctl.h
rm -f ${TEMP}
fi

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

#ifndef NASSRC

#
# Simple filter of bad words in src/include/wlioctl.h
#

if [ -f src/include/wlioctl.h ] ; then
TEMP=`mktemp src/include/wlioctl.h.XXXXXX`
grep -v -f - src/include/wlioctl.h > ${TEMP} <<EOF
WLC_SCB_AUTHORIZE
WLC_SCB_DEAUTHORIZE
WLC_GET_KEY_TXIV
WLC_TKIP_COUNTERMEASURES
WLC_WDS_GET_WPA_SUP
EOF
cp ${TEMP} src/include/wlioctl.h
rm -f ${TEMP}
fi

#endif

#ifndef USBDEVSRC

#
# Simple filter of bad words in src/include/wlioctl.h
#

if [ -f src/include/wlioctl.h ] ; then
TEMP=`mktemp src/include/wlioctl.h.XXXXXX`
grep -v -f - src/include/wlioctl.h > ${TEMP} <<EOF
WLC_UNSET_CALLBACK
WLC_SET_CALLBACK
WLC_UPGRADE_STATUS
WLC_NVRAM_DUMP
WLC_REBOOT
EOF
cp ${TEMP} src/include/wlioctl.h
rm -f ${TEMP}
fi

#endif

#endif


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

# Also eliminate wlexpand, it's only available for internal
rm -f src/wl/config/wlexpand src/wl/config/wl.wlex

# __CONFIG_SES__ is for mogrifying out SES. This is temporary till
# we are allowed to ship it to all customers. Some prebuilt objects
# are built here, which is done premogrification
#ifndef __CONFIG_SES__
rm -rf components/router/ses
rm -rf components/router/mipsel/install/ses
rm -rf components/router/mipsel-uclibc/install/ses
#endif

exit 0
