#############################################################################
# Copyright (C) 2018 Broadcom.
# The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
#
# This program is the proprietary software of Broadcom and/or its licensors,
# and may only be used, duplicated, modified or distributed pursuant to
# the terms and conditions of a separate, written license agreement executed
# between you and Broadcom (an "Authorized License").  Except as set forth in
# an Authorized License, Broadcom grants no license (express or implied),
# right to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software and all
# intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
# THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
# IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use all
# reasonable efforts to protect the confidentiality thereof, and to use this
# information only in connection with your use of Broadcom integrated circuit
# products.
#
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
# OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
# RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
# IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
# A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
# ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
# THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
# OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
# INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
# RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
# HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
# EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
# FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#############################################################################

#/bin/sh
export LD_LIBRARY_PATH=.

#update the time
ntpd -nqp pool.ntp.org &

PID=`pidof websocketd`
if [ -n "${PID}" ]; then
    echo "Killing existing websocketd [${PID}]"
    kill -9 $PID
fi

#PID=`pidof boa_server`
#if [ -n "${PID}" ]; then
#    echo "Killing existing boa [${PID}]"
#    kill -9 $PID
#fi


if [ ! -e ./scheduler_websocket ]; then
    echo "Error 'scheduler_websocket' not found.  'make scheduler_websocket' in BSEAV/tools/bmon/scheduler to build."
    exit 1
fi

if [ ! -e ./websocketd ]; then
    echo "Error 'websocketd' not found.  'make' in BSEAV/opensource/websocketd to build."
    exit 1
fi

if [ ! -e ./boa ]; then
    echo "Error 'boa' not found.  'make' in BSEAV/opensource/boa to build."
    exit 1
fi

# if not already, extend LD_LIBRARY_PATH and PATH to start with the current directory
if [[ ! ${LD_LIBRARY_PATH} == .:* ]]; then
    export LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH}
fi
if [[ ! ${PATH} == .:* ]]; then
    export PATH=.:${PATH}
fi

echo "Starting boa"
boa &

IPADDR=`ifconfig eth0 | grep Mask | awk '{print $2}'| cut -f2 -d:`
PORT=8787

echo "Starting websocketd on port:${PORT}"
websocketd --port ${PORT} ./scheduler_websocket >/tmp/websocketd.log 2>&1&
