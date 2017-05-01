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


# WPA Supplicant setup
#   -q which removes wpa supplicant and wl drivers
#   -f forces reinstallation of wpa supplicant (not wl drivers)

export LD_LIBRARY_PATH=.
FORCE="false"
QUIT="false"

while getopts "fq" opt ;
do
    case $opt in
    f)
        FORCE="true"
    ;;
    q)
        QUIT="true"
    ;;
    esac
done

if [ ! -d /var/run/wpa_supplicant ]; then
    mkdir -p /var/run/wpa_supplicant
    FORCE="true"
fi

if [ "${QUIT}" == "true" ]; then
    TEST=`ps aux|grep wpa_supplicant|grep -v grep >/dev/null && echo 1`
    if [ ! -z $TEST ]; then
        echo "kill wpa_supplicant"
        killall -9 wpa_supplicant
        sleep 1
    fi

    if [ -f /tmp/udhcpc.wlan0.pid ]; then
        #remnants from previous instance - kill/remove
        echo "kill udhcpc for wlan0"
        kill -9 `cat /tmp/udhcpc.wlan0.pid`
        rm -f /tmp/udhcpc.wlan0.pid
    fi

    TEST=`lsmod|grep -w wl >/dev/null && echo 1`
    if [ ! -z $TEST ]; then
        echo "rmmod wl.ko"
        rmmod wl.ko
    fi

    TEST=`lsmod|grep -w wlan_plat > /dev/null && echo 1`
    if [ ! -z $TEST ]; then
	echo "rmmod wlan_plat.ko"
	rmmod wlan_plat.ko
    fi

    TEST=`lsmod|grep wakeup_drv >/dev/null && echo 1`
    if [ ! -z $TEST ]; then
        echo "rmmod wakeup_drv.ko"
        rmmod wakeup_drv.ko
    fi

    TEST=`lsmod|grep nexus >/dev/null && echo 1`
    if [ ! -z $TEST ]; then
        echo "rmmod nexus.ko"
        rmmod nexus.ko
    fi

    TEST=`lsmod|grep brcmv3d >/dev/null && echo 1`
    if [ ! -z $TEST ]; then
        echo "rmmod brcmv3d.ko"
        rmmod brcmv3d.ko
    fi
else
    TEST=`lsmod|grep -w wl >/dev/null && echo 1`
    TEST1=`lsmod|grep -w wlan_plat > /dev/null && echo 1`
    if [ "$TEST" != "1" ] || [ "$TEST1" != "1" ]; then
        if [ -e wl.ko ] && [ -e wlinstall ] && [ -e wlan_plat.ko ]; then
            wlinstall wl.ko wlan0
        fi
    fi

    TEST=`ps aux|grep wpa_supplicant|grep -v grep >/dev/NULL && echo 1`
    if [ "$TEST" != "1" ] || [ "${FORCE}" == "true" ]; then
        echo "start wpa_supplicant"
        wpa_supplicant -Dnl80211 -iwlan0 -c wpa_supplicant.conf -B
    fi
fi
