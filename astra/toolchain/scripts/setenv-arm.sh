#!/bin/bash

case $0 in
*bash)
    ;;
*)
    echo "You must source this script, not run it. Do the one of following:"
    echo "    . script_name"
    echo "    source script_name"
    exit
esac

if [ ! -d scripts ]; then
	echo "All toolchain scripts have to be run from top-level dir."
	return
fi

set -ex

#
# Target and prefix
#
PWD=`pwd`

export TARGET=arm-tzos-musleabi
export PREFIX=$PWD/install

#
# Binutils
#
export BINUTILS_VERSION=2.25.1
export BINUTILS_PATCH=patches/binutils-$BINUTILS_VERSION-arm-tzos.patch
export BINUTILS_FLAGS=

#
# GCC
#
export GCC_VERSION=5.3.0
export GCC_SRC_PATCH=patches/gcc-$GCC_VERSION-arm-tzos.patch
export GCC_HDR_PATCH=patches/headers-arm-tzos.patch
export GCC_1_FLAGS="--with-arch=armv7-a --with-float=softfp"
export GCC_2_FLAGS="--with-arch=armv7-a --with-float=softfp"

#
# MUSL
#
export MUSL_VERSION=1.1.12
export MUSL_FLAGS=

set +ex
