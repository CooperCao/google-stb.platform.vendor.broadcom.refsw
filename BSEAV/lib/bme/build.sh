#!/bin/bash
#############################################################################
# Broadcom Proprietary and Confidential. © 2016 Broadcom.  All rights reserved.’
# See ‘License-BroadcomSTB-CM-Software.txt’ for terms and conditions.
# This is a helper script to build all BME dependicies. This includes
# Nexus,NxClient,common_drm,SRAI,Playready,curl and Widevine
#############################################################################
set -e

REFSW_TOP="${REFSW_TOP:-`readlink -f ../../../`}"
include_if_exists()
{
    [[ -f "$1" ]] && source "$1"  || true
}
include_if_exists $REFSW_TOP/app/build/.config
export PATH=$PATH:$CONFIG_TOOLCHAIN_PATH
export LINUX=$CONFIG_KERNEL_PATH

# Check inputs
if [ -z ${NEXUS_PLATFORM+x} ]; then echo "NEXUS_PLATFORM needs to be set. Aborting"; exit -1; fi
if [ -z ${B_REFSW_ARCH+x} ]; then echo "B_REFSW_ARCH needs to be set. Aborting"; exit -1; fi
if [ -z ${LINUX+x} ]; then echo "Linux Kernel path needs to be set. Aborting"; exit -1; fi
$B_REFSW_ARCH-g++ -v >/dev/null 2>&1 || { echo >&2 "Correct toolchain not found. Aborting"; exit 1;}

# Define top level folders
export BSEAV_TOP=$REFSW_TOP/BSEAV
export NEXUS_TOP=$REFSW_TOP/nexus
export MAGNUM=$REFSW_TOP/magnum

CMNDRM_DIR=$(make -sf build/drm.inc print-CMNDRM_DIR)
PR_PREBUILT_LIBS=$(make -sf build/drm.inc print-PR_PREBUILT_LIBS)
DRMROOTFS_LIBDIR=$(make -sf build/drm.inc print-DRMROOTFS_LIBDIR)

if [ -z ${B_REFSW_OBJ_ROOT+x} ]; then
    B_REFSW_OBJ_ROOT=$REFSW_TOP/$B_REFSW_OBJ_DIR
fi

OUTPUT_FOLDER=$B_REFSW_OBJ_ROOT
TARGET_FOLDER=$OUTPUT_FOLDER/target/usr/local
TARGET_LIB_FOLDER=$TARGET_FOLDER/lib
TARGET_MODULE_FOLDER=$TARGET_FOLDER/lib/modules
TARGET_INC_FOLDER=$TARGET_FOLDER/include
TARGET_BIN_FOLDER=$TARGET_FOLDER/bin
NEXUS_BIN_DIR=$OUTPUT_FOLDER/nexus/bin
NEXUS_BUILD_BREADCRUMB=$OUTPUT_FOLDER/.nexus
COMMON_DRM_BUILD_BREADCRUMB=$OUTPUT_FOLDER/.common_drm
SRAI_BUILD_BREADCRUMB=$OUTPUT_FOLDER/.srai
VERBOSE=n

# MS12 options
if [ $MS12_B == "y" ]; then
    export BDSP_MS12_SUPPORT=b
elif [ $MS12_C == "y" ]; then
    export BDSP_MS12_SUPPORT=c
else
    export BDSP_MS12_SUPPORT=n
fi
echo $BDSP_MS12_SUPPORT

# Multi-core builds
NUM_JOBS="${CONFIG_JLEVEL:-0}"
if [ "$NUM_JOBS" -eq "0" ]; then
  NUM_JOBS=`cat /proc/cpuinfo | grep processor | wc -l`;
fi

export MAKE_OPTIONS=-j$NUM_JOBS
INSTALL_OPTIONS=0755

function create_folders() {
    mkdir -p $TARGET_LIB_FOLDER
    mkdir -p $TARGET_LIB_FOLDER/pkgconfig
    mkdir -p $TARGET_MODULE_FOLDER
    mkdir -p $TARGET_INC_FOLDER
    mkdir -p $TARGET_BIN_FOLDER
    mkdir -p $TARGET_BIN_FOLDER/nxserver
}

function usage() {
    echo "Usage: "
    echo "  build.sh [OPTIONS]"
    echo
    echo "Build script to build BME dependicies"
    echo
    echo "Options"
    echo
    echo "  -h --help              Display this help message and exit"
    echo "  -c --clean             Clean all targets"
    echo "  -v --verbose           Enable all build outputs to stdout"
}

function clean_build {
    rm $NEXUS_BUILD_BREADCRUMB || true
    rm $COMMON_DRM_BUILD_BREADCRUMB || true
    rm $SRAI_BUILD_BREADCRUMB || true
    make -C build/ clean
    make $MAKE_OPTIONS -C $REFSW_TOP/nexus/nxclient clean 1> $LOG_OUTPUT
    make $MAKE_OPTIONS -C $REFSW_TOP/BSEAV/lib/security/common_drm/ clean 1> $LOG_OUTPUT
    make $MAKE_OPTIONS -C $REFSW_TOP/BSEAV/lib/security/bcrypt clean 1> $LOG_OUTPUT
    make $MAKE_OPTIONS -C $REFSW_TOP/BSEAV/lib/drmrootfs clean 1> $LOG_OUTPUT
    rm -rf $B_REFSW_OBJ_ROOT/BSEAV/lib/security/thirdparty/widevine 1> $LOG_OUTPUT
    rm -rf $B_REFSW_OBJ_ROOT/BSEAV/opensource/protobuf 1> $LOG_OUTPUT
}

function nexus_build {
    if [ ! -f $NEXUS_BUILD_BREADCRUMB ]; then
        echo "[Building Nexus...]"
        make $MAKE_OPTIONS -C $REFSW_TOP/nexus/build nexus_headers 1> $LOG_OUTPUT
        make $MAKE_OPTIONS -C $REFSW_TOP/nexus/nxclient/ server 1> $LOG_OUTPUT
        install $NEXUS_BIN_DIR/libnxserver.a $TARGET_LIB_FOLDER
        install $NEXUS_BIN_DIR/libnxclient*.so $TARGET_LIB_FOLDER
        install $NEXUS_BIN_DIR/libnexus.so $TARGET_LIB_FOLDER
        rm $TARGET_MODULE_FOLDER/nexus.ko || true 1> $LOG_OUTPUT
        rm $TARGET_MODULE_FOLDER/bcmdriver.ko || true 1> $LOG_OUTPUT
        if [ "$NEXUS_MODE" == "proxy" ]; then
            install $NEXUS_BIN_DIR/nexus.ko $TARGET_MODULE_FOLDER
        else
            install $NEXUS_BIN_DIR/bcmdriver.ko $TARGET_MODULE_FOLDER
            install $NEXUS_BIN_DIR/libnexus_client.so $TARGET_LIB_FOLDER
        fi
        install $NEXUS_BIN_DIR/brcmv3d.ko $TARGET_MODULE_FOLDER || true
        install $NEXUS_BIN_DIR/nxserver $TARGET_BIN_FOLDER/nxserver/

        if [ $SAGE_SUPPORT == "y" ]; then
            install $NEXUS_BIN_DIR/sage* $TARGET_BIN_FOLDER/nxserver/
        fi
        install build/start $TARGET_BIN_FOLDER/nxserver/
        touch $NEXUS_BUILD_BREADCRUMB
    fi
}

function curl_build {
    if [ $BME_ENABLE_WIDEVINE == "y" ]; then
        echo "[Building Curl...]"
        CURL_LIB_FOLDER=$(make -sf build/drm.inc print-CURL_LIB_FOLDER)
        CURL_SOURCE_PATH=$(make -sf build/drm.inc print-CURL_SOURCE_PATH)
        mkdir -p $TARGET_INC_FOLDER/curl
        make -C $REFSW_TOP/BSEAV/opensource/curl all 1> $LOG_OUTPUT
        install $CURL_LIB_FOLDER/*.so* $TARGET_LIB_FOLDER
        install $CURL_LIB_FOLDER/*.a $TARGET_LIB_FOLDER
        cp -af $CURL_SOURCE_PATH/include/curl/*.h $TARGET_INC_FOLDER/curl
     fi
}

function common_drm_build {
    COMMONDRM_LIBDIR=$B_REFSW_OBJ_ROOT/BSEAV/lib/security/common_drm/lib/$CMNDRM_DIR
    if [ $BME_ENABLE_COMMONDRM == "y" ]; then
        if [ ! -f $COMMON_DRM_BUILD_BREADCRUMB ]; then
            echo "[Building Common DRM...]"
            make $MAKE_OPTIONS -C $REFSW_TOP/BSEAV/lib/security/bcrypt install 1> $LOG_OUTPUT
            make $MAKE_OPTIONS -C $REFSW_TOP/BSEAV/lib/drmrootfs install 1> $LOG_OUTPUT
            make $MAKE_OPTIONS -C $REFSW_TOP/BSEAV/lib/security/common_drm/ install 1> $LOG_OUTPUT
            if [ "$BME_ENABLE_PLAYREADY" == "y" ]; then
                make $MAKE_OPTIONS -C $REFSW_TOP/BSEAV/lib/security/common_drm/ playready install 1> $LOG_OUTPUT
            fi
            install $B_REFSW_OBJ_ROOT/BSEAV/lib/security/bcrypt/libbcrypt.so $TARGET_LIB_FOLDER
            install $DRMROOTFS_LIBDIR/libdrmrootfs.so $TARGET_LIB_FOLDER
            install -m $INSTALL_OPTIONS $COMMONDRM_LIBDIR/libcmndrm*.so $TARGET_LIB_FOLDER
            touch $COMMON_DRM_BUILD_BREADCRUMB
        fi
    fi
}

function srai_build {
    if [ $SAGE_SUPPORT == "y" ]; then
        if [ ! -f $SRAI_BUILD_BREADCRUMB ]; then
            echo "[Building Srai ...]"
            make $MAKE_OPTIONS -C $REFSW_TOP/BSEAV/lib/security/sage/srai 1> $LOG_OUTPUT
            install -m $INSTALL_OPTIONS $B_REFSW_OBJ_ROOT/BSEAV/lib/security/sage/srai/*.so $TARGET_LIB_FOLDER
            touch $SRAI_BUILD_BREADCRUMB
        fi
    fi
}

function widevine_build {
    if [ $BME_ENABLE_WIDEVINE == "y" ]; then
        echo "[Building Widevine libraries...]"
        make -C $REFSW_TOP/BSEAV/opensource/protobuf install_to_nexus_bin 1> $LOG_OUTPUT
        make -C $REFSW_TOP/BSEAV/lib/security/third_party/widevine/CENC3x cenc_only 1> $LOG_OUTPUT
    fi
}

function playready_build {
    if [ $BME_ENABLE_PLAYREADY == "y" ]; then
        install $B_REFSW_OBJ_ROOT/BSEAV/lib/security/common_drm/libcmndrmprdy.so $TARGET_LIB_FOLDER
        install $PR_PREBUILT_LIBS $TARGET_LIB_FOLDER
    fi
}

function  bme_build {
    echo "[Building BME ...]"
    cd build
    make
}

function print_build_info {
    echo "###################################################"
    echo "Build parameters: "
    echo "NEXUS_PLATFORM=$NEXUS_PLATFORM"
    echo "BCHP_VER=$BCHP_VER"
    echo "B_REFSW_DEBUG=$B_REFSW_DEBUG"
    echo "B_REFSW_ARCH=$B_REFSW_ARCH"
    echo "Jobs= $NUM_JOBS"
    echo "Output: $B_REFSW_OBJ_ROOT"
    echo "###################################################"
}

CLEAN_ONLY=n
while : ; do
    case "$1" in
        -h|--help)
            usage;
            exit 0;
            shift ;;

        -c|--clean)
            CLEAN_ONLY=y
            shift ;;
        -v|--verbose)
            VERBOSE=y;
            shift ;;
        *)
            break ;;
    esac
done

LOG_OUTPUT=/dev/null
if [ $VERBOSE == "y" ]; then
    LOG_OUTPUT=/dev/stdout
fi

if [ $CLEAN_ONLY == "y" ]; then
    clean_build;
    exit 0;
fi
print_build_info

create_folders
nexus_build
common_drm_build
srai_build
widevine_build
playready_build
curl_build
bme_build
