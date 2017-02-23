#!/bin/sh

set -ex

PWD=`pwd`

export TARGET=arm-tzos-musleabi
export PREFIX=$PWD/install

#
# GCC stage-1
#
GCC_VERSION=5.3.0
GCC_DIR=gcc-$GCC_VERSION
[ -d build-gcc-1 ] ||
	mkdir build-gcc-1
cd build-gcc-1
[ -f $PREFIX/bin/$TARGET-gcc ] ||
	(../$GCC_DIR/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot \
	--enable-languages=c --with-newlib \
	--disable-libssp --disable-nls \
	--disable-libquadmath --disable-threads --disable-decimal-float \
	--with-arch=armv7-a --with-float=softfp \
	--disable-shared --disable-libmudflap --disable-libgomp --disable-libatomic \
	&& make all-gcc && make all-target-libgcc && make install-gcc && make install-target-libgcc)
cd ..
