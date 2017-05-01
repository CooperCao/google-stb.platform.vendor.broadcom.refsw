#!/bin/bash

if [ ! -d scripts ]; then
	echo "All toolchain scripts have to be run from top-level dir."
	return
fi

set -ex

#
# GCC stage-1
#
GCC_DIR=gcc-$GCC_VERSION

if [ ! -d build-gcc-1 ]; then
	mkdir build-gcc-1
fi

if [ ! -f $PREFIX/bin/$TARGET-gcc ]; then
	cd build-gcc-1
	../$GCC_DIR/configure --target=$TARGET --prefix="$PREFIX" \
		--with-sysroot \
		--enable-languages=c --with-newlib \
		--disable-libssp --disable-nls \
		--disable-libquadmath --disable-threads --disable-decimal-float \
		--disable-shared --disable-libmudflap --disable-libgomp --disable-libatomic \
		$GCC_1_FLAGS
	make all-gcc
	make all-target-libgcc
	make install-gcc
	make install-target-libgcc
	cd ..
fi

set +ex
