#!/bin/bash

if [ ! -d scripts ]; then
	echo "All toolchain scripts have to be run from top-level dir."
	return
fi

set -ex

#
# Binutils
#
rm -rf binutils-$BINUTILS_VERSION
rm -rf build-binutils

#
# GCC source
#
rm -rf gcc-$GCC_VERSION

#
# GCC stage-1
#
rm -rf build-gcc-1

#
# MUSL
#
rm -rf musl-$MUSL_VERSION

#
# GCC stage 2
#
rm -rf build-gcc-2

#
# Target and prefix
#
rm -rf $PREFIX

set +ex
