#!/bin/sh
#############################################################################
# Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to the terms and
# conditions of a separate, written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
# no license (express or implied), right to use, or waiver of any kind with respect to the
# Software, and Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
# HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
# NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
# secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of Broadcom integrated circuit products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
# AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
# WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
# THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
# OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
# LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
# OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
# USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
# LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
# EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
# USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
# ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
# LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
# ANY LIMITED REMEDY.
#############################################################################


# WLAN drivers and WPA Supplicant setup (only option is -f which forces setup)

if [ ! -f wl.ko ] || [ ! -f wpa_supplicant ]; then
    # wlan and wpa_supplicant do not exist so do nothing
    exit 0
fi

export LD_LIBRARY_PATH=.
FORCE="false"

#find appropriate wlan driver configuration based on board
if [ ! -f /proc/device-tree/bolt/board ]; then
    echo "atlas_wlan_prep.sh ERROR: wlan driver config cannot be determined"
    exit -1
fi
WLAN_CONFIG_FILE=`cat /proc/device-tree/bolt/board | awk '{print tolower($0)}'`

while getopts 'f' opt ;
do
    case $opt in
    f)
        FORCE="true"
    ;;
    esac
done

if [ ! -d /var/run/wpa_supplicant ]; then
    mkdir -p /var/run/wpa_supplicant
    FORCE="true"
fi

if [ "${FORCE}" == "true" ]; then
    killall -9 wpa_supplicant
fi

if [ -f /tmp/udhcpc.wlan0.pid ]; then
    #remnants from previous instance - kill/remove
    kill -9 `cat /tmp/udhcpc.wlan0.pid`
    rm -f /tmp/udhcpc.wlan0.pid
fi

TEST=`lsmod|grep wl >/dev/null && echo 1`
if [ -z $TEST ] || [ "${FORCE}" == "true" ]; then
    wlinstall wl.ko wlan0 $WLAN_CONFIG_FILE.txt
fi

TEST=`pidof wpa_supplicant >/dev/null && echo 1`
if [ -z $TEST ] || [ "${FORCE}" == "true" ]; then
    wpa_supplicant -Dnl80211 -iwlan0 -c wpa_supplicant.conf -B
fi
