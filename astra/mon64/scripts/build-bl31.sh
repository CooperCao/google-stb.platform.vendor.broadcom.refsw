#!/bin/bash

if [ ! -d scripts ]; then
	echo "All mon64 scripts have to be run from top-level dir."
	return
fi

set -ex

# Default to release build
if [ -z $DEBUG ]; then export DEBUG=0; fi

# Check for the toolchain before doing a bunch of useless work
if [ ! -d /opt/toolchains/stbgcc-4.8-1.5/bin ]; then
	echo "Toolchain directory: /opt/toolchains/stbgcc-4.8-1.5/bin is missing"
	exit 1
fi

# Get the source code
if [ ! -d arm-tf ]; then
	git clone http://git-irv-02.irv.broadcom.com:8084/r/android/arm-tf.git -b swtrellis-2913
fi

# Build the bl31 using refsw toolchain
CFLAGS="-O0 -gdwarf-2" CROSS_COMPILE=/opt/toolchains/stbgcc-4.8-1.5/bin/aarch64-linux- make -C arm-tf DEBUG=$DEBUG PLAT=bcm97268a0 SPD=astra bl31

# Copy bl31.bin to bl31_astra.bin
if [ $DEBUG = "0" ] && [ -e arm-tf/build/bcm97268a0/release/bl31.bin ]; then
	cp arm-tf/build/bcm97268a0/release/bl31.bin bl31_astra.bin
fi
if [ $DEBUG = "1" ] && [ -e arm-tf/build/bcm97268a0/debug/bl31.bin ]; then
	cp arm-tf/build/bcm97268a0/debug/bl31.bin bl31_astra.bin
fi

set +ex
