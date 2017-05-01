#!/bin/bash

if [ ! -d scripts ]; then
	echo "All toolchain scripts have to be run from top-level dir."
	return
fi

set -ex

#
# Binutils
#
BINUTILS_PKG=binutils-$BINUTILS_VERSION.tar.bz2
BINUTILS_DIR=binutils-$BINUTILS_VERSION
BINUTILS_URL=http://ftp.gnu.org/gnu/binutils/$BINUTILS_PKG

if [ ! -f $BINUTILS_PKG ]; then
	wget $BINUTILS_URL
fi

if [ ! -d $BINUTILS_DIR ]; then
	(tar jxvf $BINUTILS_PKG && patch -p0 < $BINUTILS_PATCH)
fi

if [ ! -d build-binutils ]; then
	mkdir build-binutils
fi

if [ ! -f $PREFIX/bin/$TARGET-ld ]; then
	cd build-binutils
	../$BINUTILS_DIR/configure --target=$TARGET --prefix="$PREFIX" \
		--with-sysroot --disable-nls --disable-werror \
		$BINUTILS_FLAGS
	make
	make install
	cd ..
fi

set +ex
