#!/bin/bash

if [ ! -d scripts ]; then
	echo "All toolchain scripts have to be run from top-level dir."
	return
fi

set -ex

#
# Binutils
#
./scripts/build-binutils.sh

#
# GCC source
#
./scripts/build-gcc-src.sh

#
# GCC stage-1
#
./scripts/build-gcc-1.sh

#
# MUSL
#
./scripts/build-musl.sh

#
# GCC stage 2
#
./scripts/build-gcc-2.sh

set +ex
