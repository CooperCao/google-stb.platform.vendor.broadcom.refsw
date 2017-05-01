#!/bin/bash

if [ ! -d scripts ]; then
	echo "All toolchain scripts have to be run from top-level dir."
	return
fi

set -ex

#
# GCC stage 2
#
GCC_DIR=gcc-$GCC_VERSION

if [ ! -d build-gcc-2 ]; then
	mkdir build-gcc-2
fi

if [ ! -f $PREFIX/bin/$TARGET-g++ ]; then
	cd build-gcc-2
	../$GCC_DIR/configure --target=$TARGET --prefix="$PREFIX" \
		--with-sysroot \
		--enable-languages=c,c++ --disable-libmudflap \
		--disable-libsanitizer --disable-nls \
		$GCC_2_FLAGS
	make
	make install
	cd ..
fi

patch -p0 < $GCC_HDR_PATCH

set +ex
