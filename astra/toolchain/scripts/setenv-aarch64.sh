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

export TARGET=aarch64-tzos-musleabi
export PREFIX=$PWD/install

#
# Binutils
#
export BINUTILS_VERSION=2.26
export BINUTILS_PATCH=patches/binutils-$BINUTILS_VERSION-aarch64-tzos.patch
export BINUTILS_FLAGS=

#
# GCC
#
export GCC_VERSION=5.3.0
export GCC_SRC_PATCH=patches/gcc-$GCC_VERSION-aarch64-tzos.patch
export GCC_HDR_PATCH=patches/headers-aarch64-tzos.patch
export GCC_1_FLAGS="--with-arch=armv8-a --with-abi=lp64"
export GCC_2_FLAGS="--with-arch=armv8-a --with-abi=lp64 --enable-shared"

#
# MUSL
#
export MUSL_VERSION=1.1.14
export MUSL_FLAGS=
export MUSL_PATCH=patches/musl-$MUSL_VERSION-O_EXEC-tzos.patch
set +ex
