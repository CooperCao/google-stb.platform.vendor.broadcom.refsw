#!/bin/bash
#
# build_newtools.sh - Do a checkout/release for the MIPS toolchain
#
# Copyright 2001 Broadcom Corporation
#
# $Id$
#

## Modified version of build_gnutools to build the new(er) version
## of the toolchain (3.2.3) by basically following src/gnu/Makefile.
## Includes checkout and build of uClibc.

## Notes:
##  - Removed unused -o, -b (OSSUB, BRAND).
##  - Added funky -t option (TESTING), but should really just tag
##    correctly and use CVS, removing -t when ready.
##  - Changed TAG parsing to assume src/gnu tag like TOOLS_n_n_n.
##  - Could consider a switch to build different toolchain versions?

## More changes:
##  - Instead of NAME_n_n_n[_n], accept NAME_n_n_n[_n][-m] tags;
##    install directory suffix becomes -n.n.n[-n][-bcm-m].
##  - Modify new bcmver.h files (gcc/gcc and binutils/gas) to put
##    the BCM rev in, so it gets in version.c and as.c strings.
##  - Extract all sources using tag (don't use HEAD for some).

usage ()
{
cat <<EOF
    Usage: $script -d base_dir [-r tag] [-i install_base]
    -d base_dir
              The base directory where the source will be extracted to and
              the release will be built.  (This is now REQUIRED, in order
              to avoid inadvertantly clobbering existing directories.

              NOTE: Must be absolute path.

    -i install_base
              The base directory where the resulting binaries will be
	      installed and run from.  If no -i option is provided, the
              default install_base is /opt/brcm.
	
	      NOTE: The structure of uClibc means that the install_base
	      directory will be compiled into mipsel-uclibc-gcc as the
	      place to get mipsel-linux-gcc; it MUST be the final path.

              NOTE: Must be absolute path.

    -r tag    Use this option if you want to force extraction of sources
              with a particular CVS tag, e.g. \"-r ILINE10_REL_1_2\".  If
              not specified HEAD is used.

    -t        Test: build directly in base_dir (no tagged/dated subdirs);
              use the sources already there in src (no CVS pull).  (Also
	      uses base_dir as the default install_base if no -i given.)

    -R repository
              Repository path to use (defaults to /projects/cvsroot).

EOF
    exit 1;
}

function checkname {
	if [ ! -f $2 ]; then
		echo "WARNING: $1 has been $3" >> $LOG 2>&1
	fi
}

function checkrc {
	if [ $# -lt 2 ]; then
		echo "Script error ${1:-(no rc)}"
		exit ${rc:-1}
	fi
	rc=$1; shift
	if [ $rc == 0 ]; then
		echo "$* succeeded."
	else
		echo "$* failed, $rc" | tr 'a-z' 'A-Z'
		exit $rc
	fi
}
	
## Copied from our gnu 3.2.3 makefile, where comment says PR2473 applies
## when the TLB is active (i.e. Linux user mode only).
WITH_BCM4710A0=--with-bcm4710a0

BUILD_BASE=
INSTALL_BASE="/opt/brcm"
INSTALL_SET=

TAG=
TESTING=

    while getopts ':r:d:i:htR:' OPT
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
	    INSTALL_SET=TRUE
	    ;;
	t)
            TESTING=TRUE
            ;;
	R)
	    TESTCVS=$OPTARG;
	    ;;
	h)
	    usage;
	    ;;
	:)
	    echo -e "\nMissing required argument for option \"$OPTARG\".\n";
	    usage;
	    ;;
	?)
	    echo -e "\nUnrecognized option - \"$OPTARG\".\n";
	    usage;
	    ;;
	esac
    done

if [ "$OPTIND" -le "$#" ]; then
    usage;
fi

# Allow [[ ]] pattern match, save old in case we want to restore
OLDGLOB=`shopt -p extglob`
shopt -s extglob

if [ -z "$BUILD_BASE" ]; then
    echo -e "\nBase build directory must be specified.\n";
    usage;
fi

if [[ $BUILD_BASE != /* ]]; then
    echo -e "\nBase directory must be an absolute path.\n";
    usage;
fi

if [[ $INSTALL_BASE != /* ]]; then
    echo -e "\nInstall directory must be an absolute path.\n";
    usage;
fi

DATE=`date '+%Y%m%d'`
# Format version string like epivers.sh
yyyy=$(date '+%Y')
mm=$(date '+%m')
dd=$(date '+%d')
m=${mm/#0/}
d=${dd/#0/}

# Figure out install directory name(s) based on tag
suffix=
if [ x${TAG} == x ]; then
    # No tag -- use date, note if test (not a CVS pull)
    suffix=${TESTING:+TEST}${DATE}
    BCMTAG=${DATE}
else
    # Accept NAME_N1_N2_N3[_N4][-N5] (e.g. MIPSEL_3_2_3-2),
    # produce corresponding suffix of N1.N2.N3[-N4][-bcm-N5]
    vnums=($(IFS="- " vnums=(${TAG}); echo ${vnums[*]}))
    GNUTAG=${vnums[0]}
    BCMTAG=${vnums[1]}

    if [[ $GNUTAG == +([A-Za-z0-9])_+([0-9])_+([0-9])_+([0-9])?(_+([0-9])) ]]; then
	tag=($(IFS="_ " tag=(${GNUTAG}); echo ${tag[*]}))
	case ${#tag[*]} in
	4)
	    suffix=${tag[1]}.${tag[2]}.${tag[3]}
	    ;;
	5)
	    suffix=${tag[1]}.${tag[2]}.${tag[3]}-${tag[4]}
	    ;;
	esac
    fi

    if [ -n "$suffix" ] && [[ $BCMTAG = ?(+([0-9])) ]]; then
	suffix="$suffix${BCMTAG:+-bcm-${BCMTAG}}"
    else
	echo "($TAG) not in expected tag format NAME_N1_N2_N3[_N4][-N5]."
	echo "(Where Nx are numeric, [] brackets indicate optional components.)"
	exit
    fi
fi

echo Based on tag ${TAG:-(none)}, will use install suffix ${suffix}.
echo Will use BCMVER based on BCMTAG=${BCMTAG}

# Find/create build base, make dated build directory
if [ -n "$TESTING" ]; then
    BUILD_DIR="${BUILD_BASE}"
else
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
  BUILD_DIR="${BUILD_DIR}/${yyyy}.${m}.${d}.${i}"
fi

if [ -n "$TESTING" ] && [ -z "$INSTALL_SET" ]; then
    echo "Test build w/o explicit -i, will install in build directory"
    INSTALL_BASE=${BUILD_DIR}
fi

INSTALL_DIR=${INSTALL_BASE}/hndtools-mipsel-linux-${suffix}
INSTALL_UCLIBC=${INSTALL_BASE}/hndtools-mipsel-uclibc-${suffix}

## Redirect stdout and stderr to ,build.log
#exec 3> ${BUILD_DIR}/,build.log
#exec 1>&3
#exec 2>&3

echo "TAG=$TAG"
echo "BUILD_BASE=$BUILD_BASE"
echo "BUILD_DIR=$BUILD_DIR"
echo "INSTALL_DIR=$INSTALL_DIR"

########################################################################
### Setup finished (and displayed); start actually doing things...

echo -n "Building gnutools: "
date

# Enter build directory
cd "${BUILD_DIR}"
if [ $? != 0 ]; then
    echo Cannot cd to ${BUILD_DIR}
    exit 1
fi;

if [ -z "$TESTING" ]; then
    unset CVSREAD
    export CVSROOT=/projects/cvsroot
    [ -n "$TESTCVS" ] && export CVSROOT=${TESTCVS}

    # Get gnu (binutils, glibc, gcc) and uClibc, using tag or HEAD
    echo "Export of gnu and uClibc: CVSROOT=${CVSROOT}"
    echo "cvs -Q export -r ${TAG:-HEAD} -kk src/gnu src/uClibc" \
	> ${BUILD_DIR}/,checkout.log 2>&1
    cvs -Q export -r ${TAG:-HEAD} -kk src/gnu src/uClibc \
	>> ${BUILD_DIR}/,checkout.log 2>&1
    checkrc $? "Export of gnu, uClibc"

    # Get linux and include, using tag or HEAD
    echo "Export of linux and include: CVSROOT=${CVSROOT}"
    echo "cvs -Q export -r ${TAG:-HEAD} -kk src/linux/linux src/include" \
	>> ${BUILD_DIR}/,checkout.log 2>&1
    cvs -Q export -r ${TAG:-HEAD} -kk src/linux/linux src/include \
	>> ${BUILD_DIR}/,checkout.log 2>&1
    checkrc $? "Export of linux, include"
else
    for sources in gnu/{binutils,gcc,glibc} uClibc linux/linux include; do
	if [ ! -d src/${sources} ]; then
	    echo "Test build error: src/${sources} not in ${BUILD_DIR}."
	    exit 1
	fi
    done
    for file in src/gnu/glibc/manual/libc.info* src/gnu/glibc/po/sv.mo; do
	if [ -e ${file} ] && [ ! -w ${file} ]; then
	    echo "Install glibc would fail: requires writable files!!"
	    echo "Files: src/gnu/glibc/manual/libc.info* and src/gnu/glibc/po/sv.mo."
	    exit 1
	fi
    done
fi

## Generate bcmver.h files to put BCM rev in gcc and as version strings.
echo "#define BCMVER \" (rev $BCMTAG)\"" > src/gnu/gcc/gcc/bcmver.h
echo "#define BCMVER \" (rev $BCMTAG)\"" > src/gnu/binutils/gas/bcmver.h

# Build in a separate obj directory easier cleaning
if [ -e obj ]; then
    echo "WILL NOT BUILD: ${BUILD_DIR}/obj already exists!"
    exit 1
fi
mkdir obj

OBJ=${BUILD_DIR}/obj
SRC=${BUILD_DIR}/src
GNU=${SRC}/gnu

TARGET=mipsel-linux
INCDIR=${INSTALL_DIR}/$TARGET/include
CROSS=${INSTALL_DIR}/bin/$TARGET-
LINUX=${SRC}/linux/linux

# Make sure built binaries can be used
export PATH=${INSTALL_DIR}/bin:$PATH

#######################################################################
# Define subfunctions to do the pieces.  Intended to execute in order,
# but this way might allow quicker rebuild of subunits when testing.
# (I.e. could add options to start or stop partway through.)
#######################################################################

#######################
# Start with binutils
binutils() {
    echo -n "Doing binutils: "; date

    mkdir -p ${OBJ}/binutils; cd ${OBJ}/binutils
    ${GNU}/binutils/configure \
	--prefix=${INSTALL_DIR} --target=mipsel-linux ${WITH_BCM4710A0} \
	> ${BUILD_DIR}/,gnu-binutils-config.log 2>& 1
    checkrc $? "Configure of binutils"

    gmake > ${BUILD_DIR}/,gnu-binutils-build.log 2>& 1
    checkrc $? "Make of binutils"

    gmake install > ${BUILD_DIR}/,gnu-binutils-install.log 2>& 1
    checkrc $? "Install of binutils"

    # Copy additional static libraries
    ## This part is from the Makefile, which says: Install static libraries.
    ## The script did not do this; maybe they were not required in an earlier
    ## version -- or maybe they aren't now?  It would seem that libiberty.a
    ## need not be copied (it's done the make install above) ...

    cp libiberty/libiberty.a ${INSTALL_DIR}/lib
    cp opcodes/libopcodes.a ${INSTALL_DIR}/lib
    cp bfd/libbfd.a ${INSTALL_DIR}/lib

    echo -n "Done binutils: "; date
}

############################
# Move on to bootstrap gcc
xgcc() {
    echo -n "Doing xgcc: "; date

    ## The Makefile added 'disable-threads' and 'WITH_BCM4710A0' to the
    ## configure command; they were not in the script.  (Makefile did not
    ## have the -v option, but that's really irrelevant.)

    ## The 'make' and 'install' commands in the script included variable
    ## settings of 'INSTALL_TARGET_MODULES=' and 'LANGUAGES="c"', the 'make'
    ## line also included 'ALL_TARGET_MODULES='.  These have been left out,
    ## as they were not in the Makefile.  The only effect I can determine is
    ## that we produce gcov with xgcc; the script apparently did not?

    mkdir -p ${OBJ}/xgcc; cd ${OBJ}/xgcc
    ${GNU}/gcc/configure -v \
	--prefix=${INSTALL_DIR} --target=mipsel-linux ${WITH_BCM4710A0} \
	--with-newlib --disable-shared --disable-threads --enable-languages=c \
	> ${BUILD_DIR}/,gnu-xgcc-config.log 2>& 1
    checkrc $? "Configure of xgcc"

    gmake -k > ${BUILD_DIR}/,gnu-xgcc-build.log 2>& 1
    checkrc $? "Make of xgcc"

    gmake -k install > ${BUILD_DIR}/,gnu-xgcc-install.log 2>&1
    checkrc $? "Install of xgcc"

    echo -n "Done xgcc: "; date
}

###################################################
# Generate kernel headers so xgcc can build glibc
kheaders() {
    echo -n "Doing linux includes: "; date

    ## The Makefile added the make in src/include and the make dep in linux;
    ## script did not have them.  For consistency with Makefile, keeping them
    ## here as well.  (Also using the -p option to tar, which wasn't in the
    ## original script either.)

    cd ${SRC}/include
    gmake > ${BUILD_DIR}/,src-include.log 2>&1
    checkrc $? "Make src/include"

    cd ${LINUX}
    cp arch/mips/defconfig-bcm947xx .config && \
	gmake oldconfig include/linux/version.h > ${BUILD_DIR}/,linux-oldconfig.log 2>&1
    checkrc $? "Linux oldconfig"
    gmake dep > ${BUILD_DIR}/,linux-dep.log 2>&1
    checkrc $? "Linux dep"

    ############################
    # Install the headers now
    mkdir -p $INCDIR
    cd ${LINUX}/include && tar -cpf - asm asm-mips linux | tar -xpf - -C $INCDIR
    checkrc $? "Linux include dir copy"

    echo -n "Done linux includes: "; date
}

#######################################
# Now ready to build glibc using xgcc
glibc() {
    ## Several differences between the Makefile and the old script here.
    ## Configuration:
    ##  - Script did not use a 'CFLAGS' definition for the configure command.
    ##  - The script had 'with-elf' and 'disable-profile'; droping them for now.
    ##  - The script had 'enable-kernel=2.3.99'; might we need this in future
    ##    (specifying current kernel 2.4.20, for example) in case we rebuild
    ##    tools later that must support rebuilding the router with this kernel?
    ##  - Makefile uses 'build=i686-pc-linux-gnu', while the script seems to
    ##    try and get this effect by defining CC/AS/LD/AR/RANLIB explicitly
    ##    (as cross-tools ${INSTALL_DIR}/bin/$TARGET-<tool>) and defining
    ##    BUILD_CC as gcc.  Not clear that this works as well; certainly not
    ##    the same.  Use Makefile method for now (though it might be nice to
    ##    use the output of glibc/scripts/config.guess rather than explicitly
    ##    defining the vaule for --build?)
    ## Install:
    ##  - Makefile specifies actual location as install_root; script uses a
    ##    separate location, then copies over 'include' and 'lib' subdirs.
    ##  - Makefile copies over some files for optimization (optinfo).
    ##  - Script used sed instead of perl to fix libc.so.  (Is the case
    ##    insensitivity of the perl line important?)
    ## General:
    ##  - The script added the INSTALL_DIR to the PATH here, while Makefile
    ##    did it at the top; we do it before xgcc (which needs it); is this
    ##    an added requirement in the newer gcc?

    echo -n "Doing glibc: "; date

    mkdir -p ${OBJ}/glibc; cd ${OBJ}/glibc
    CFLAGS="-O2 -g -finline-limit=10000" ${GNU}/glibc/configure \
	--build=i686-pc-linux-gnu --host=${TARGET} --prefix= \
	--with-headers=${INCDIR} --enable-add-ons \
	> ${BUILD_DIR}/,glibc-config.log 2>&1
    checkrc $? "Configure glibc"

    gmake > ${BUILD_DIR}/,glibc-build.log 2>&1
    checkrc $? "Make of glibc"

    gmake install install_root=${INSTALL_DIR}/${TARGET} > ${BUILD_DIR}/,glibc-install.log 2>&1
    checkrc $? "Install glibc"

    # Fix libc.so to point to real install location
    echo "Fixing up libc.so locations"
    perl -i -p -e s@/lib/@${INSTALL_DIR}/${TARGET}/lib/@ig \
	${INSTALL_DIR}/${TARGET}/lib/libc.so
    checkrc $? "Fixup of libc.so"

    # Install archives for optimization
    mkdir -p ${INSTALL_DIR}/${TARGET}/lib/optinfo &&\
	cp ${OBJ}/glibc/libc{.map,_pic.a} ${OBJ}/glibc/elf/so{init,fini}.os \
	${INSTALL_DIR}/${TARGET}/lib/optinfo
    checkrc $? "Populating optinfo"

    echo -n "Done glibc: "; date
}

############################
# Finally, do the full gcc
fullgcc() {
    echo -n "Doing full gcc: "; date

    mkdir -p ${OBJ}/gcc; cd ${OBJ}/gcc

    ## Differences in Makefile and old script configuration:
    ##   - Script had 'enable-threads' and 'enable-shared'; did these
    ##     need to be removed?  (Are they now on by default?)
    ##   - Script had only 'c' enabled; why do we use 'c,c++' now?
    ##   - Makefile adds WITH_BCM4710A0 (why isn't this in the script?)
    ##   - Makefile adds "with-headers"; why did script leave that out?
    ##     (Could this have a bearing on mobility of currenty binaries?)

    ${GNU}/gcc/configure -v \
	${WITH_BCM4710A0} --target=${TARGET} --prefix=${INSTALL_DIR} \
	--with-headers=${INSTALL_DIR}/${TARGET}/include --enable-languages=c,c++ \
	> ${BUILD_DIR}/,gnu-full-config.log 2>&1
    checkrc $? "Configure full gcc"

    gmake > ${BUILD_DIR}/,gnu-full-build.log 2>&1
    checkrc $? "Make full gcc"

    gmake install > ${BUILD_DIR}/,gnu-full-install.log 2>&1
    checkrc $? "Install full gcc"

    echo -n "Done full gcc: "; date
}

###########################
# Ok, let's do uClibc too
uclibc() {
    echo -n "Doing uClibc: "; date

    # Modify .config to reflect our install
    cd ${SRC}/uClibc
    mv .config .config.old
    checkrc $? "Backup of .config"
    sed -e 's:^KERNEL_SOURCE=.*:KERNEL_SOURCE="'${INSTALL_DIR}/mipsel-linux'"': \
	-e 's:^DEVEL_PREFIX=.*:DEVEL_PREFIX="'${INSTALL_UCLIBC}'"': \
	< .config.old > .config
    checkrc $? Edit of .config

    # Build/install this new config
    make clean > ${BUILD_DIR}/,uclibc-clean.log 2>&1
    checkrc $? "Clean in uClibc"

    make oldconfig > ${BUILD_DIR}/,uclibc-config.log 2>&1
    checkrc $? "Config in uClibc"

    make > ${BUILD_DIR}/,uclibc-build.log 2>&1
    checkrc $? "Make in uClibc"

    make install > ${BUILD_DIR}/,uclibc-install.log 2>&1
    checkrc $? "Install of uClibc"

    echo -n "Done uClibc: "; date
}

############################
# Make the ordered calls...
binutils
xgcc
kheaders
glibc
fullgcc
uclibc

echo -n "build_newtools done: "
date
