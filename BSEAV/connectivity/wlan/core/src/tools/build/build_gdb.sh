#!/bin/bash
#
# build_gdb.sh - Do a checkout/release for the MIPS gdb
#
# Copyright 2001 Broadcom Corporation
#
# $Id$
#

usage ()
{
cat <<EOF
    Usage: $script [-r tag] [-b brand] [-d base_dir] [-i install_base] [-o]
    -r tag    Use this option if you want to force extraction of sources
              with a particular CVS tag, e.g. \"-r ILINE10_REL_1_2\".

    -b brand  The brand for this build  If no brand is mentioned, the
              default is \"brcm\".  For any brand you select there must
              be two corresponding files checked in to CVS called
              src/tools/release/<brand>.cfg and
              src/tools/release/release_<brand>.sh

    -d base_dir
              The base directory where the source will be extracted to and
              the release will be built.  If no -d option is provided,
              the default base_dir is /projects/hnd/sw/build/build_gnutools

    -i install_base
              The base directory where the resulting binaries will be
	      installed and run from.  If no -i option is provided, the
              default install_base is /opt/brcm.

EOF
    exit 1;
}

function checkname {
	if [ ! -f $2 ]; then
		echo "WARNING: $1 has been $3" >> $LOG 2>&1
	fi
}


BUILD_BASE="/projects/hnd/swbuild/build_gnutools"
INSTALL_BASE="/opt/brcm"

TAG=
BRAND=
OSSUB=

    while getopts ':r:d:i:b:o:h' OPT
    do
	case "$OPT" in
	r)
	    TAG="$OPTARG";
	    ;;
	d)
	    BUILD_BASE=$OPTARG;
	    ;;
	i)
	    INSTALL_BASE=$OPTARG;
	    ;;
	b)
	    BRAND=$OPTARG;
	    ;;
	h)
	    usage;
	    ;;
	:)
	    echo "Missing required argument for option \"$OPTARG\".\n";
	    usage;
	    ;;
	?)
	    echo "Unrecognized option - \"$OPTARG\".\n";
	    usage;
	    ;;
	esac
    done

if [ "$OPTIND" -le "$#" ]; then
    usage;
fi

RELDIR=${TAG:-"NIGHTLY"}
DATE=`date '+%y%m%d'`
BUILD_DIR=${BUILD_DIR:-${TAG:-"BUILD_"${DATE}}}

if [ x${TAG} == x ];
then
    # If no tag given, use the date for the install dir
    INSTALL_DIR=${INSTALL_BASE}/hndtools-mipsel-linux-${DATE}
else
    # Parse the tag to come up with the install dir suffix
    IFS="_ " tag=(${TAG})
    unset IFS
    case ${#tag[*]} in
    4)
	suf=${tag[2]}.${tag[3]}
	;;
    5)
	suf=${tag[2]}.${tag[3]}${tag[4]}
	;;
    6)
	suf=${tag[2]}.${tag[3]}${tag[4]}-${tag[5]}
	;;
    *)
	echo "TAG format: $TAG is not recognized"
	exit
	;;
    esac
    INSTALL_DIR=${INSTALL_BASE}/hndtools-mipsel-linux-${suf}
fi


echo "BUILD_DIR=$BUILD_DIR"
echo "BRAND=$BRAND"
echo "TAG=$TAG"
echo "BUILD_BASE=$BUILD_BASE"
echo "INSTALL_DIR=$INSTALL_DIR"


cd ${BUILD_BASE}
if [ $? != 0 ]; then
    echo Cannot cd to ${BUILD_BASE}
    exit 1
fi;

# Add a level of indirection (TAG or date) to the release
mkdir ${RELDIR}
cd ${RELDIR}
if [ $? != 0 ]; then
	echo Cannot cd to ${RELDIR}
	exit 1
fi

BUILD_DIR_ORIG=${BUILD_DIR}
let i=0
while ! mkdir $BUILD_DIR
do
    echo Cannot create new ${BUILD_DIR}
    if (( $i < 20 ))
    then
	BUILD_DIR=${BUILD_DIR_ORIG}_${i}
        echo Trying ${BUILD_DIR} instead.
	let i=i+1;
    else
	echo "Giving up."
	exit 1;
    fi;
done

cd $BUILD_DIR
if [ $? != 0 ]; then
    echo Cannot cd to ${BUILD_DIR}
    exit 1
fi;

# OK, this is where we build
TOP=${BUILD_BASE}/${RELDIR}/${BUILD_DIR}

LOG=${TOP}/,build.log
# Redirect stdout and stderr to build.log
#exec 3> $LOG
#exec 1>&3
#exec 2>&3

echo -n "Building gdb: "
date

echo "PWD = ${PWD}, "
echo "BUILD_BASE = ${BUILD_BASE}, "
echo "BUILD_DIR = ${BUILD_DIR}, "
echo "INSTALL_DIR = ${INSTALL_DIR}, "
echo "TAG = ${TAG}"
echo

set -xv
export CVSROOT=/projects/cvsroot
echo "Doing cvs checkout src/gnu-20011102, CVSROOT=${CVSROOT}"
cvs co ${TAG:+-r $TAG} src/gnu-20011102 > ${TOP}/,checkout-gdb.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Checkout succeeded, $rc"
else
	echo "CHECKOUT FAILED, $rc"
	exit $rc
fi

SRC=${TOP}/src
GNU=${SRC}/gnu-20011102

cd ${GNU}

rm -rf binutils boehm-gc cgen contrib fastjar gas gcc gprof ld libchill \
    libf2c libffi libgloss libjava libobjc libstdc++-v3 newlib proto-toplev \
    winsup zlib

find . -name configure -print | xargs touch

mkdir mipsel
cd mipsel

echo -n "Doing gdb: "
date

../configure -v --prefix=${INSTALL_DIR}/ --target=mipsel-linux > ${TOP}/,gdb-config.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Configure of gdb succeeded, $rc"
else
	echo "CONFIGURE gdb FAILED, $rc"
	exit $rc
fi

gmake -k > ${TOP}/,gdb-build.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Make of gdb succeeded, $rc"
else
	echo "MAKE gdb FAILED, $rc"
	exit $rc
fi

gmake -k install > ${TOP}/,gdb-install.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Install gdb succeeded, $rc"
else
	echo "INSTALL gdb FAILED, $rc"
	exit $rc
fi

echo -n "build_gdb done: "
date
