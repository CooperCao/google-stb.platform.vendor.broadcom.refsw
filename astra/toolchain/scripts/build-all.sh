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


#
# GCC stage-1
#
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


#
# GCC stage 2
#
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
