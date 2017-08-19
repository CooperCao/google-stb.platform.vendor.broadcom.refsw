#!/bin/bash
#
# build_gnutools.sh - Do a checkout/release for the MIPS toolchain
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

DATE=`date '+%Y%m%d'`
# Format version string like epivers.sh
yyyy=$(date '+%Y')
mm=$(date '+%m')
dd=$(date '+%d')
m=${mm/#0/}
d=${dd/#0/}

# Figure out install directory
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

# Make base directory
BUILD_DIR="${BUILD_BASE}/${TAG:-NIGHTLY}"
if ! mkdir -p "${BUILD_DIR}" ; then
    echo "$(hostname): Creation of ${BUILD_DIR} failed"
    exit 1
fi
# Make new build directory
for i in $(seq 0 20) ; do
    [ "$i" = "20" ] && break
    mkdir "${BUILD_DIR}/${yyyy}.${m}.${d}.${i}" > /dev/null 2>&1 && break
done
if [ "$i" = "20" ] ; then
    echo "$(hostname): Creation of ${BUILD_DIR}/${yyyy}.${m}.${d}.${i} failed"
    exit 1
fi

# Enter build directory
BUILD_DIR="${BUILD_DIR}/${yyyy}.${m}.${d}.${i}"
cd "${BUILD_DIR}"
if [ $? != 0 ]; then
    echo Cannot cd to ${BUILD_DIR}
    exit 1
fi;

## Redirect stdout and stderr to ,build.log
#exec 3> ${BUILD_DIR}/,build.log
#exec 1>&3
#exec 2>&3

echo "BUILD_DIR=$BUILD_DIR"
echo "BRAND=$BRAND"
echo "TAG=$TAG"
echo "BUILD_BASE=$BUILD_BASE"
echo "INSTALL_DIR=$INSTALL_DIR"

echo -n "Building gnutools: "
date

export CVSROOT=/projects/cvsroot
echo "Doing cvs export gnu/glibc/linux, CVSROOT=${CVSROOT}"
cvs -Q export ${TAG:+-r $TAG} -kk gnu-tools > ${BUILD_DIR}/,checkout.log 2>&1
rc1=$?
# Checkout a linux tree without a tag
cvs -Q export -r HEAD -kk src/linux/linux >> ${BUILD_DIR}/,checkout.log 2>&1
rc2=$?
if [ $rc1 == 0 -a $rc2 == 0 ]; then
	echo "Checkout succeeded, $rc"
else
	echo "CHECKOUT FAILED, $rc1/$rc2"
	exit $rc1
fi

SRC=${BUILD_DIR}/src
GNU=${SRC}/gnu-20010422

cd ${GNU}

mkdir mipsel-nl
cd mipsel-nl

echo -n "Doing gnu-tools with no-libs: "
date

../configure -v --with-newlib --disable-shared --enable-languages=c \
	--prefix=${INSTALL_DIR}/ --target=mipsel-linux > ${BUILD_DIR}/,gnu-nl-config.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Configure of g-t-nl succeeded, $rc"
else
	echo "CONFIGURE g-t-nl FAILED, $rc"
	exit $rc
fi

gmake -k ALL_TARGET_MODULES= CONFIGURE_TARGET_MODULES= INSTALL_TARGET_MODULES= LANGUAGES="c" > ${BUILD_DIR}/,gnu-nl-build.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Make of g-t-nl succeeded, $rc"
else
	echo "MAKE g-t-nl FAILED, $rc"
	exit $rc
fi

gmake -k INSTALL_TARGET_MODULES= LANGUAGES="c" install > ${BUILD_DIR}/,gnu-nl-install.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Install g-t-nl succeeded, $rc"
else
	echo "INSTALL g-t-nl FAILED, $rc"
	exit $rc
fi

TARGET=mipsel-linux
INCDIR=$INSTALL_DIR/$TARGET/include
CROSS=$INSTALL_DIR/bin/$TARGET-

LINUX=${SRC}/linux/linux

echo -n "Doing linux includes: "
date

cd ${LINUX}

cp arch/mips/defconfig-bcm947xx .config && gmake oldconfig include/linux/version.h > ${BUILD_DIR}/,linux-oldconfig.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Linux oldconfig succeeded, $rc"
else
	echo "LINUX OLDCONFIG FAILED, $rc"
	exit $rc
fi

mkdir -p $INCDIR

cd include && tar -cf - asm asm-mips linux | tar -xf - -C $INCDIR
rc=$?
if [ $rc == 0 ]; then
	echo "Linux include dir copy succeeded, $rc"
else
	echo "L-I-D COPY FAILED, $rc"
	exit $rc
fi

GLIBC=${SRC}/linux/glibc

cd ${GLIBC}

mkdir mipsel-obj
mkdir mipsel-install

cd mipsel-obj

export PATH=${INSTALL_DIR}/bin:$PATH

export CC=${CROSS}gcc
export AS=${CROSS}as
export LD=${CROSS}ld
export AR=${CROSS}ar
export RANLIB=${CROSS}ranlib
export BUILD_CC=gcc

echo -n "Doing glibc: "
date

../configure --prefix= --host=${TARGET} --with-headers=${INCDIR} \
	--enable-kernel=2.3.99 --enable-add-ons --with-elf --disable-profile > ${BUILD_DIR}/,glibc-config.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Glibc Configure succeeded, $rc"
else
	echo "Glibc CONFIGURE FAILED, $rc"
	exit $rc
fi

gmake > ${BUILD_DIR}/,glibc-build.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo -n "Glibc Make succeeded, $rc: "
	date
else
	echo "Glibc MAKE FAILED, $rc"
	exit $rc
fi

gmake install_root=${GLIBC}/mipsel-install install > ${BUILD_DIR}/,glibc-install.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo -n "Glibc Install succeeded, $rc: "
	date
else
	echo "Glibc INSTALL FAILED, $rc"
	exit $rc
fi

cd ../mipsel-install/include && tar -cf - . | tar -xf - -C ${INCDIR}
rc=$?
if [ $rc == 0 ]; then
	echo "Glibc include Install succeeded, $rc"
else
	echo "GL-I-I FAILED, $rc"
	exit $rc
fi

cd ../lib && tar -cf - . | tar -xf - -C ${INSTALL_DIR}/${TARGET}/lib
rc=$?
if [ $rc == 0 ]; then
	echo "Glibc lib Install succeeded, $rc"
else
	echo "GL-L-I FAILED, $rc"
	exit $rc
fi

echo "Fixing up libc.so locations"
sed -e s%/lib/%"${INSTALL_DIR}/${TARGET}/lib/"%g \
	< "${INSTALL_DIR}/${TARGET}/lib/libc.so" \
	> "${INSTALL_DIR}/${TARGET}/lib/.new.libc.so"
rc=$?
if [ $rc == 0 ]; then
	echo "libc.so Fixup succeeded, $rc"
else
	echo "libc.so FIXUP FAILED, $rc"
	exit $rc
fi

mv -f "${INSTALL_DIR}/${TARGET}/lib/.new.libc.so" \
	"${INSTALL_DIR}/${TARGET}/lib/libc.so"
rc=$?
if [ $rc == 0 ]; then
	echo "libc.so Replace succeeded, $rc"
else
	echo "libc.so REPLACE FAILED, $rc"
	exit $rc
fi

# Clean up the environment

unset CC
unset AS
unset LD
unset AR
unset RANLIB
unset BUILD_CC
unset CFLAGS
unset ASFLAGS

# Now go back to src/gnu and do a full build & install
cd ${GNU}

mkdir mipsel
cd mipsel

echo -n "Doing full gnu-tools: "
date

../configure -v --enable-shared --enable-threads --enable-languages=c \
	--prefix=${INSTALL_DIR}/ --target=mipsel-linux > ${BUILD_DIR}/,gnu-full-config.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Configure for g-t-full succeeded, $rc"
else
	echo "CONFIGURE g-t-full FAILED, $rc"
	exit $rc
fi

gmake > ${BUILD_DIR}/,gnu-full-build.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Make for g-t-full succeeded, $rc"
else
	echo "MAKE g-t-full FAILED, $rc"
	exit $rc
fi

gmake install > ${BUILD_DIR}/,gnu-full-install.log 2>&1
rc=$?
if [ $rc == 0 ]; then
	echo "Install for g-t-full succeeded, $rc"
else
	echo "INSTALL g-t-full FAILED, $rc"
	exit $rc
fi

echo -n "build_gnutools done: "
date
