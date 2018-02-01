#!/bin/bash
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
    if [ _$file_sum = _$sum ]; then
        return 0;
    else
        echo "$sumtype sum for $file ($file_sum) != $sum !!"
        return 1;
    fi
}

if [ $# -lt 3 ] ; then
    cat <<_USAGE_
$0 usage:
  $0 meta key destination [noverify]
  noverify [optional] - indicates checksum is not to be verified
_USAGE_
    exit 1
fi
scriptdir=$(readlink -m $(dirname $0))
rootdir=$(readlink -m ${scriptdir}/../../.. )
fetch=${scriptdir}/fetch_binaries.sh
meta=$1
key=$2
destination=$3
##filename=$4
checksum="verify"
force_download=""
if [ $# -ge 4 ] ; then
    checksum=$4
fi

while read meta_key filesumtype filesum url ; do
    if [ _${meta_key} = _# ]; then
        continue
    fi
    if [ _"${key}" != _"${meta_key}" ]; then
        continue
    fi
    destination_file="${destination}/${key}"
    if [ -f $destination_file  ]; then
        if [ _"$checksum" != _"verify" ]; then
            exit 0; # destination file exists
        fi
        if test_sum $destination_file $filesumtype $filesum ; then
            exit 0; # destination file exists and matches checksum
        fi
    fi
    if [ -f $fetch ]; then
        tmp="tmp.${key//\//.}.$$"
        tmp_file="$destination/$tmp"
	while true; do
            rm -f $tmp_file
            bash $fetch $url "${key//\//.}.$filesumtype.$filesum" $destination $tmp $filesumtype $filesum ${force_download}
            if [ $? -eq 0 -a -f $tmp_file ]; then
		if [ _"$checksum" != _"verify" ]; then
                    mkdir -p $(dirname $destination_file)
                    mv $tmp_file $destination_file
                    exit 0; # destination file exists
		fi
		if test_sum $tmp_file $filesumtype $filesum ; then
                    mkdir -p $(dirname $destination_file)
                    mv $tmp_file $destination_file
                    exit 0; # destination file exists and matches checksum
		else
                    echo "$filesumtype checksum mismatch for $file" 1>&2
		    rm -f $tmp_file
		    [ -n "$force_download" ] && break
		    force_download="force"
		fi
            else
		echo "Failed to download $file" 1>&2
		exit 1
            fi
	done
	echo "Failed to verify file '$destination_file'" 1>&2
	exit 1
    fi
done <$meta
echo "Can't find binary metadata information about '$key' in $meta" 1>&2
exit 1;
