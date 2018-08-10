#!/bin/bash

# Check Usage
if [ $# != 2 ]; then
	echo "Usage: gen_sdk.sh <SOURCE/BINARY> <Arm64/Arm32>"
	echo "	SOURCE: SDK with Kernel Sources"
	echo "	BINARY: SDK with Kernel binary"
	echo "	Arm64: SDK for 64 Bit Platform"
	echo "	Arm32: SDK for 32 Bit Platform"
	exit
fi

# Check Input Parameters
if [ $1 != 'SOURCE' ] && [ $1 != 'BINARY' ]; then
	echo "Incorrect option for Source/Binary SDK. Valid options SOURCE/BINARY"
	exit
fi

if [ $2 != 'Arm64' ] && [ $2 != 'Arm32' ]; then
	echo "Incorrect Platform option. Valid Options Arm32/Arm64"
	exit
fi

# Remove any old SDK directory
if [ -e astra_sdk ]; then
	rm -rf astra_sdk
fi

#set ASTRA_TOP
ASTRA_TOP=$PWD
echo $ASTRA_TOP

# Set OBJ Directory
# TZ obj top is derived from refsw obj root if defined
if [ "$B_REFSW_OBJ_ROOT" != "" ]; then
	objdir=$B_REFSW_OBJ_ROOT
elif [ "$B_REFSW_OBJ_DIR" != "" ]; then
	objdir=$ASTRA_TOP/../$B_REFSW_OBJ_DIR
else
	if [ $2 = 'Arm64' ]; then
		objdir=$ASTRA_TOP/../obj.aarch64
	elif [ $2 = 'Arm32' ]; then
		objdir=$ASTRA_TOP/../obj.arm
	fi
fi

echo $objdir

# Create new SDK
mkdir astra_sdk
cp -rf $ASTRA_TOP/build astra_sdk/.

if [ $1 = 'BINARY' ]; then
# Build and copy Astra Kernel binary
	make -C $ASTRA_TOP/user TZ_ARCH=$2 distclean
	make -C $ASTRA_TOP/user TZ_ARCH=$2 BL31=n
	mkdir astra_sdk/bin
	cp $objdir/astra/user/astra.bin astra_sdk/bin/astra_$2.bin

	mkdir astra_sdk/kernel
	cp -rf $ASTRA_TOP/kernel/api astra_sdk/kernel/.
	sdk_type=$1_$2
elif [ $1 = 'SOURCE' ]; then
# Create Bin Dirs
	make -C $ASTRA_TOP/user TZ_ARCH=$2 checkdirs

# Copy kernel Sources
	cp -rf $ASTRA_TOP/kernel astra_sdk/.
	sdk_type=$1_$2
fi

# Build and Copy monitor binary for 64 bit platforms only
if [ $2 = 'Arm64' ]; then
	make -C $ASTRA_TOP/monitor clean
	make -C $ASTRA_TOP/monitor
	if [ -e astra_sdk/bin ]; then
		cp $objdir/astra/monitor/mon64.bin astra_sdk/bin/.
	else
		mkdir astra_sdk/bin
		cp $objdir/astra/monitor/mon64.bin astra_sdk/bin/.
	fi
fi

# Copy linux driver module and user space lib
mkdir astra_sdk/linux
cp -rf $ASTRA_TOP/linux/bcm_astra/ astra_sdk/linux/.
cp -rf $ASTRA_TOP/linux/libbcm_astra/ astra_sdk/linux/.

# Copy user/libtzioc
mkdir astra_sdk/user
cp -rf $ASTRA_TOP/user/libtzioc astra_sdk/user/.

# Copy Toolchain
cp -rf $ASTRA_TOP/toolchain astra_sdk/.

# Remove any git references
find astra_sdk/. -name .git* -delete

# Generate the tar file
if [ -e astra_sdk_$sdk_type.tar ]; then
    rm -rf astra_sdk_$sdk_type.tar
fi
if [ -e $objdir/astra/astra_sdk_$sdk_type.tar ]; then
	echo Removing $objdir/astra/astra_sdk_$sdk_type.tar
    rm -rf $objdir/astra/astra_sdk_$sdk_type.tar
fi

tar -cvf astra_sdk_$sdk_type.tar astra_sdk/

# Move the tar file to OBJ dir
mv astra_sdk_$sdk_type.tar $objdir/astra/

# Remove temp folders
rm -rf astra_sdk
