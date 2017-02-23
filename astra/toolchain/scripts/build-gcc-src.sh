#!/bin/sh

set -ex

PWD=`pwd`

export TARGET=arm-tzos-musleabi
export PREFIX=$PWD/install

#
# GCC
#
GCC_VERSION=5.3.0
GCC_PKG=gcc-$GCC_VERSION.tar.bz2
GCC_DIR=gcc-$GCC_VERSION
GCC_PATCH=gcc-$GCC_VERSION-arm-tzos.patch
GCC_URL=http://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/$GCC_PKG
[ -f $GCC_PKG ] ||
	wget $GCC_URL
[ -d $GCC_DIR ] ||
	(tar jxvf $GCC_PKG && patch -p0 < $GCC_PATCH)
[ -f $GCC_DIR/mpfr-2.4.2.tar.bz2 ] ||
	(cd $GCC_DIR && contrib/download_prerequisites && cd ..)
