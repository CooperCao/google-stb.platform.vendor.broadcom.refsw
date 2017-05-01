#!/bin/bash

if [ ! -d scripts ]; then
	echo "All toolchain scripts have to be run from top-level dir."
	return
fi

set -ex

#
# GCC source
#
GCC_PKG=gcc-$GCC_VERSION.tar.bz2
GCC_DIR=gcc-$GCC_VERSION
GCC_URL=http://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/$GCC_PKG

if [ ! -f $GCC_PKG ]; then
	wget $GCC_URL
fi

if [ ! -d $GCC_DIR ]; then
	tar jxvf $GCC_PKG
	patch -p0 < $GCC_SRC_PATCH
fi

if [ ! -f $GCC_DIR/mpfr-2.4.2.tar.bz2 ]; then
	cd $GCC_DIR
	contrib/download_prerequisites
	cd ..
fi

set +ex
