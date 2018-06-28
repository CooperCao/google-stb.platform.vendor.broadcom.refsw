#!/bin/bash
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


# Check Usage
if [ $# != 2 ]; then
	echo "Usage: gen_sdk.sh <SOURCE/BINARY> <Arm64/Arm32>"
	echo "	SOURCE: SDK with Kernel Sources"
	echo "	BINARY: SDK with Kernel binary"
	echo "	Arm64: SDK for 64 Bit Platform"
	echo "	Arm32: SDK for 32 Bit Platform"
	exit
fi

# Check Input Parameters
if [ $1 != 'SOURCE' ] && [ $1 != 'BINARY' ]; then
	echo "Incorrect option for Source/Binary SDK. Valid options SOURCE/BINARY"
	exit
fi

if [ $2 != 'Arm64' ] && [ $2 != 'Arm32' ]; then
	echo "Incorrect Platform option. Valid Options Arm32/Arm64"
	exit
fi

# Remove any old SDK directory
if [ -e astra_sdk ]; then
	rm -rf astra_sdk
fi

#set ASTRA_TOP
ASTRA_TOP=$PWD
echo $ASTRA_TOP

# Set OBJ Directory
# TZ obj top is derived from refsw obj root if defined
if [ "$B_REFSW_OBJ_ROOT" != "" ]; then
	objdir=$B_REFSW_OBJ_ROOT
elif [ "$B_REFSW_OBJ_DIR" != "" ]; then
	objdir=$ASTRA_TOP/../$B_REFSW_OBJ_DIR
else
	if [ $2 = 'Arm64' ]; then
		objdir=$ASTRA_TOP/../obj.aarch64
	elif [ $2 = 'Arm32' ]; then
		objdir=$ASTRA_TOP/../obj.arm
	fi
fi
if [ $2 = 'Arm64' ]; then
	bin=astra64.bin
elif [ $2 = 'Arm32' ]; then
	bin=astra32.bin
fi

echo $objdir

# Create new SDK
mkdir astra_sdk
touch astra_sdk/.astra_sdk
cp -rf $ASTRA_TOP/build astra_sdk/.

if [ $1 = 'BINARY' ]; then
# Build and copy Astra Kernel binary
	make -C $ASTRA_TOP/user TZ_ARCH=$2 distclean
	make -C $ASTRA_TOP/user TZ_ARCH=$2 BL31=n
	mkdir astra_sdk/bin
	cp $objdir/astra/user/$bin astra_sdk/bin/$bin

	mkdir astra_sdk/kernel
	cp -rf $ASTRA_TOP/kernel/api astra_sdk/kernel/.
	sdk_type=$1_$2
elif [ $1 = 'SOURCE' ]; then
# Create Bin Dirs
	make -C $ASTRA_TOP/user TZ_ARCH=$2 checkdirs

# Copy kernel Sources
	cp -rf $ASTRA_TOP/kernel astra_sdk/.
	sdk_type=$1_$2
fi

# Build and Copy monitor binary for 64 bit platforms only
if [ $2 = 'Arm64' ]; then
	make -C $ASTRA_TOP/monitor clean
	make -C $ASTRA_TOP/monitor SMM64=n
	if [ -e astra_sdk/bin ]; then
		cp $objdir/astra/monitor/mon64.bin astra_sdk/bin/.
	else
		mkdir astra_sdk/bin
		cp $objdir/astra/monitor/mon64.bin astra_sdk/bin/.
	fi
fi

# Copy linux driver module and user space lib
mkdir astra_sdk/linux
cp -rf $ASTRA_TOP/common astra_sdk/.
cp -rf $ASTRA_TOP/linux/bcm_astra/ astra_sdk/linux/.
cp -rf $ASTRA_TOP/linux/libbcm_astra/ astra_sdk/linux/.
cp -rf $ASTRA_TOP/linux/examples/ astra_sdk/linux/.


# Copy user/libtzioc
mkdir astra_sdk/user
cp -rf $ASTRA_TOP/user/libtzioc astra_sdk/user/.
cp -rf $ASTRA_TOP/user/astra_tapp astra_sdk/user/.

# Copy Toolchain
cp -rf $ASTRA_TOP/toolchain astra_sdk/.

# Remove any git references
find astra_sdk/. -name .git* -delete

# Generate the tar file
if [ -e astra_sdk_$sdk_type.tar ]; then
    rm -rf astra_sdk_$sdk_type.tar
fi
if [ -e $objdir/astra/astra_sdk_$sdk_type.tar ]; then
	echo Removing $objdir/astra/astra_sdk_$sdk_type.tar
    rm -rf $objdir/astra/astra_sdk_$sdk_type.tar
fi

tar -cvf astra_sdk_$sdk_type.tar astra_sdk/

# Move the tar file to OBJ dir
mv astra_sdk_$sdk_type.tar $objdir/astra/

# Remove temp folders
rm -rf astra_sdk
