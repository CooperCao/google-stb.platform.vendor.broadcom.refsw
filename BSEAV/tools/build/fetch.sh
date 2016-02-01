#!/bin/sh
#############################################################################
#    (c)2014 Broadcom Corporation
#
# This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
set -e
set -u
test_sum() {
    file=$1;sumtype=$2;sum=$3;
    file_sum=`${sumtype}sum $file|cut -f1 -d" "`
    if [ _$file_sum != _$sum ]; then
        echo "Not matching ${sumtype} checksum for $file ($sum,$file_sum)"
        exit 1
    fi
}

if [ $# -ne 2 ] ; then
    cat <<_USAGE_
$0 usage:
  $0 meta dest
_USAGE_
fi
meta=$1
dest=$2
B_REFSW_CACHE_DIR=${B_REFSW_CACHE_DIR:-/tmp}
while read file sum url sumtype; do
    if [ _${file} = _# ]; then
        continue
    fi
    if [ -z "$sumtype" ]
    then
        sumtype=md5
    fi
    cache_file="${B_REFSW_CACHE_DIR}/$file"
    dest_file="$dest/$file"
    if [ -f $cache_file ]; then
        echo "using $cache_file"
        test_sum $cache_file $sumtype $sum
        cp -f $cache_file $dest_file
    elif [ -d $B_REFSW_CACHE_DIR -a -w $B_REFSW_CACHE_DIR ]; then
        tmp_file=${cache_file}.$USER.$$
        wget -O $tmp_file $url
        test_sum $tmp_file $sumtype $sum
        mv $tmp_file $cache_file
        cp -f $cache_file $dest_file
    else
        echo "Can't use cache $B_REFSW_CACHE_DIR"
        tmp_file=${dest_file}.$USER.$$
        wget -O $tmp_file $url
        test_sum $tmp_file $sumtype $sum
        mv $tmp_file $dest_file
    fi
    exit 0
done <$meta
echo "Invalid META file" $meta
exit 1
