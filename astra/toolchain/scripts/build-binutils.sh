#!/bin/sh

set -ex

PWD=`pwd`

export TARGET=arm-tzos-musleabi
export PREFIX=$PWD/install

#
# Binutils
#
BINUTILS_VERSION=2.25.1
BINUTILS_PKG=binutils-$BINUTILS_VERSION.tar.bz2
BINUTILS_DIR=binutils-$BINUTILS_VERSION
BINUTILS_PATCH=binutils-$BINUTILS_VERSION-arm-tzos.patch
BINUTILS_URL=http://ftp.gnu.org/gnu/binutils/$BINUTILS_PKG
[ -f $BINUTILS_PKG ] ||
	wget $BINUTILS_URL
[ -d $BINUTILS_DIR ] ||
	(tar jxvf $BINUTILS_PKG && patch -p0 < $BINUTILS_PATCH)
[ -d build-binutils ] ||
	mkdir build-binutils
cd build-binutils
[ -f $PREFIX/bin/$TARGET-ld ] ||
	(../$BINUTILS_DIR/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror && make && make install)
cd ..
