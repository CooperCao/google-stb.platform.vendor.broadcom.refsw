#!/bin/bash

if [ ! -d scripts ]; then
	echo "All toolchain scripts have to be run from top-level dir."
	return
fi

set -ex

#
# MUSL
#
MUSL_PKG=musl-$MUSL_VERSION.tar.gz
MUSL_DIR=musl-$MUSL_VERSION
MUSL_URL=http://www.musl-libc.org/releases/$MUSL_PKG

if [ ! -f $MUSL_PKG ]; then
	wget $MUSL_URL
fi

if [ ! -d $MUSL_DIR ]; then
	(tar zxvf $MUSL_PKG  && patch -p0 < $MUSL_PATCH)
fi

if [ ! -f $PREFIX/$TARGET/lib/libc.so ]; then
	cd $MUSL_DIR
	export PATH=$PREFIX/bin:$PATH
	./configure --prefix="$PREFIX/$TARGET" \
		--enable-debug --enable-optimize --enable-shared \
		CROSS_COMPILE="$TARGET-" CC="$TARGET-gcc" \
		$MUSL_FLAGS
	make
	make install
	cd ..
fi

if [ ! -d $PREFIX/$TARGET/sys-root/usr/include ]; then
	mkdir -p $PREFIX/$TARGET/sys-root
	mkdir -p $PREFIX/$TARGET/sys-root/usr
	cd $PREFIX/$TARGET/sys-root/usr
	ln -s ../../include include
	cd ../../../..
fi

set +ex
