#!/bin/sh

set -ex

PWD=`pwd`

export TARGET=arm-tzos-musleabi
export PREFIX=$PWD/install

#
# GCC stage 2
#
GCC_VERSION=5.3.0
GCC_DIR=gcc-$GCC_VERSION
[ -d build-gcc-2 ] ||
	mkdir build-gcc-2
cd build-gcc-2
[ -f $PREFIX/bin/$TARGET-g++ ] ||
	(../$GCC_DIR/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot \
	--enable-languages=c,c++ --disable-libmudflap \
	--disable-libsanitizer --disable-nls \
	--with-arch=armv7-a --with-float=softfp \
	&& make && make install)
cd ..
patch -p0 < headers-arm-tzos.patch
