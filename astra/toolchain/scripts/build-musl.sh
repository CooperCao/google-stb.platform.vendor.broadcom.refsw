#!/bin/sh

set -ex

PWD=`pwd`

export TARGET=arm-tzos-musleabi
export PREFIX=$PWD/install

#
# MUSL
#
MUSL_VERSION=1.1.12
MUSL_PKG=musl-$MUSL_VERSION.tar.gz
MUSL_DIR=musl-$MUSL_VERSION
MUSL_URL=http://www.musl-libc.org/releases/$MUSL_PKG
[ -f $MUSL_PKG ] ||
	wget $MUSL_URL
[ -d $MUSL_DIR ] ||
	tar zxvf $MUSL_PKG
[ -f $PREFIX/$TARGET/lib/libc.so ] ||
	(cd $MUSL_DIR &&
	 export PATH=$PREFIX/bin:$PATH &&
	./configure --prefix="$PREFIX/$TARGET" --enable-debug --enable-optimize --enable-shared CROSS_COMPILE="$TARGET-" CC="$TARGET-gcc" &&
	make &&
	make install && cd ..)
[ -d $PREFIX/$TARGET/sys-root/usr/include ] ||
	(mkdir -p $PREFIX/$TARGET/sys-root && mkdir -p $PREFIX/$TARGET/sys-root/usr && cd $PREFIX/$TARGET/sys-root/usr && ln -s ../../include include && cd ../../../..)
