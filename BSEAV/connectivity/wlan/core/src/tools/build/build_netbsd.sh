#!/bin/bash
#
# NetBSD top level build script
#   This script if called from linux host, it will make use of LSF
#   otherwise on native NetBSD, the build is performed on the localhost
#
# $Copyright (C) 2004 Broadcom Corporation
#
# Author: Prakash Dhavali
# Contact: hnd-software-scm-list
# 
# $Id$
#
# SVN: $HeadURL$
#

SCRIPT_VERSION='$Id$'

# Default options
BUILD_BASE=
BUILDS_TO_KEEP=
# default old build keep count for tot and tob builds
BUILDS_TO_KEEP_TOB=2
BUILDS_TO_KEEP_TOT=7
OVERRIDE=
LSFUSER=hwnbuild
TAG=
PRESERVE_LIST="/home/hwnbuild/src/tools/release/preserve_built_objects.txt"
# temporarily double preserve peroid to last 4 months
PRESERVE_CTIME=120
MISC_DIR=misc
ENVLOG=${MISC_DIR}/,env.log
MOVELOG=${MISC_DIR}/,move.log
RLSLOG=,release.log
DONGLE_RLSLOG=,dongle_image_release.log
DONGLE_ERRORLOG=,dongle_image_errors.log
ERRORLOG=,build_errors.log
FIND_BUILD_ERRORS="perl /projects/hnd_software/gallery/src/tools/build/find_build_errors"
SVNCMD="svn --non-interactive"
ADMINMAILTO="hnd-software-scm-list@broadcom.com"
export VCTOOL=svn

WARN_FILE=_WARNING_PARTIAL_CONTENTS_DO_NOT_USE
WARN_VCTOOL_FILE=_WARNING_WRONG_CHECKOUT_FOUND
IGNORE_FILE=_SOME_BUILD_ERRORS_IGNORED

MAILTO=
HOSTOS=$(uname -s)
NULL=/dev/null

if [ "${HOSTOS}" == "Linux" ]; then
   LSF=sj-wlanswbuild
   ITERCMD="seq"
else
   ITERCMD="jot -w %d "
fi

# When build is interrupted, append to build error log
function trap_func {
        signal_found=$(($?-128))

        echo "${TAG:-TOT} $BRAND full build log at: ${RLSLOG}" >> $ERRORLOG
        tail -10 ${RLSLOG}                                     >> $ERRORLOG
        echo ""                                                >> $ERRORLOG
        echo "WARN: Build termination detected at `date`"      >> $ERRORLOG
        echo "WARN: signal=$signal_found on $(hostname)"       >> $ERRORLOG
        exit 1
}

trap trap_func SIGHUP SIGINT SIGTERM

# Usage
usage ()
{
cat <<EOF
Usage: $0 -d dir [other options]

-b brands
	Brands to build (default: all).
	For more than one brand use "brand1 brand2 brand3 ... brandX"

-c cutoff_time|scmrev
        This switch takes either cvs cutoff time or svn revision number.
        * Cvs cutoff time (yyyy-mm-dd hh:mm format or other cvs/svn formats)
          for checking out from tot/tob (default: none)
          For SVN current timezone is appended by default during checkout
          ('nightly' refers to midnight and 'now' or 'fix refer current time)
        * SVN revision can be r1234 or just 1234 format
        
-d dir
	Base directory in which to build (REQUIRED).

-f
        Pass gmake flags down to release makefile.
        Use this to build using CVS/HNDCVS instead of SVN

-g
        Use gnu-make to build [default]

-j
        Use electric make to build [optional]
        NOTE: This is not available for this platform yet

-m address
	Send mail when build is complete (default: none).

-p private_makefile
	Use private release makefile instead of checking out from cvs

-q queue
	LSF queue to use (default: ${LSF}) when cross-compiling from linux.
        Specify "none" to build locally. For native NetBSD builds, LSF is
        set to "none"

-r tag    
	CVS tag to use (default: HEAD).

-t testdir
	Source directory with files to override cvs pull.  Must be a CVS
	checkout.  (The "cvs diff" there is applied to the new checkout.)

-u
	Submit job(s) as current user, rather than hwnbuild

-x builds_to_keep
	Remove old builds (default: keep last ${BUILDS_TO_KEEP_TOB} TOB builds; keep last ${BUILDS_TO_KEEP_TOT} TOT builds)

EOF
    exit 1
}

# Set SUBMIT to any necessary rsh required for bsub
function setsubmit {
    local cluster=sj1;
    local user=${LSFUSER};
    local mycluster="";
    mycluster=$(/tools/bin/lsid 2> ${NULL} |
	egrep '^My cluster name'            |
	sed -e 's/My cluster name is //');

    SUBMIT=
    if [ "${mycluster}" != "${cluster}" ] || [ -z "$(hostname)" ] ||
	([ "$(whoami)" != "${user}" ] && ! rsh -l ${user} $(hostname) true 2> ${NULL}); then
	SUBMIT="rsh -l ${user} xlinux.sj.broadcom.com"
    else
	if [ "$(whoami)" != "${user}" ]; then
	    SUBMIT="rsh -l ${user} $(hostname)"
	fi
    fi

    if [ -n "${SUBMIT}" ] && ! ${SUBMIT} true; then
	echo "Error: can't submit as ${LSFUSER}?"
	exit
    fi
}

CVSROOT_PROD=/projects/cvsroot

# Set up environment variables
function setenviron {
    UNIX_PATH=/bin:/usr/bin:/usr/X11R6/bin:/sbin:/usr/sbin:/usr/X11R6/sbin:/usr/local/bin:/usr/local/sbin::/projects/hnd/tools/linux/bin:/home/hwnbuild/src/tools/build
    CROSSGCCLINKS=/projects/hnd/tools/linux/hndtools-x86-netbsd/bin

    # Equivalent CVS Snapshot that created above SVN Repo
    if [ -z "$CVS_SNAPSHOT" ]; then
        # This is a copy of above with svn+cvs_snapshot build fixes to makefiles
        export CVS_SNAPSHOT="/projects/hnd_swbuild_ext6_scratch/build_admin_ext6_scratch/10_cvs_snapshot_culled/2011-02-11_sj"
    fi

    CVSROOT_PROD=/projects/cvsroot
    CVSROOT_SNAPSHOT=$CVS_SNAPSHOT

    export PATH=${UNIX_PATH}:/usr/pkg/bin
    # CVS on netbsd has issues handling large files, so use pserver mode
    # until those large files in src/wl/sys are taken care of.
    #export CVSROOT=${CVSROOT_PROD}
    if [ -z "$CVSROOT" ]; then
        export CVSROOT=":pserver:$(id -un)@cvsps-sj1-1.sj.broadcom.com:${CVSROOT_PROD}"
    fi

    # Default SVN Repo root, if not set in environment
    if [ -z "$SVNROOT" ]; then
        # WLAN DEMO Repository
        # export SVNROOT="http://engcm-sj1-vm13.sj.broadcom.com/wlan_demo/2011-01-20_sj/proj"
        # WLAN Production Repository
        export SVNROOT="http://svn.sj.broadcom.com/svn/wlansvn/proj"
    fi
    export SVNBASE=$(echo $SVNROOT | sed -e 's%/proj%%')

    if [ ! -n "$REPO_URL" ]; then
       # Default repo url is trunk/tot
       export REPO_URL="${SVNROOT}/trunk"

       case "${TAG}" in
       *_BRANCH_*|*_TWIG_*)
           export REPO_URL="${SVNROOT}/branches/${TAG}"
           ;;
       *_REL_*)
           export TAG_PREFIX=$(echo $TAG | awk -F_ '{print $1}')
           export REPO_URL="${SVNROOT}/tags/${TAG_PREFIX}/${TAG}"
           ;;
       esac
    fi
}

# Command line switches this script takes
# WARN: Ensure that cmd line option blocks below are arranged alphabetically.
while getopts 'c:d:e:b:f:ghjm:p:q:r:t:ux:' OPT ; do
    case "$OPT" in
       c)
           # Cutoff time or svn revision to use during checkout
           # CVSCUTOFF is exported into the caller's environment
           export CVSCUTOFF=${OPTARG}
           # SVNCUTOFF is exported into the caller's environment
           export SVNCUTOFF="${OPTARG}"
           ;;
       d)
           BUILD_BASE=${OPTARG}
           ;;
       e)
           EXTRA_LOG_STATUS=${OPTARG}
           ;;
       b)
           BRANDS=(${OPTARG})
           ;;
       f)
           GMAKEFLAGS="${OPTARG}"
           ;;
       g)
           GMAKE=1
           ;;
       h)
           usage
           ;;
       j)
           #disabled# EMAKE=1
           ;;
       m)
           MAILTO=${OPTARG}
           ;;
       p)
           PVT_MAKEFILE=${OPTARG}
           ;;
       q)
           LSF=${OPTARG}
           ;;
       r)
           TAG=${OPTARG}
           ;;
       t)
           OVERRIDE=${OPTARG}
           LSFUSER=$(whoami)
           ;;
       u)
           LSFUSER=$(whoami)
           ;;
       x)
           BUILDS_TO_KEEP=${OPTARG}
           ;;
       ?)
           usage
           ;;
    esac
done

if echo "${GMAKEFLAGS}" | grep -q "VCTOOL=svn"; then
   export VCTOOL=svn
   # If build submitted for SVN, forcefully redirect it to
   # SVN_TRANSITION/SVN folder
   #disabled# if [ -n "$BUILD_BASE" ]; then
   #disabled#    if echo $BUILD_BASE | egrep -qv "SVN_TRANSITION/SVN"; then
   #disabled#       BUILD_BASE="${BUILD_BASE}/SVN_TRANSITION/SVN"
   #disabled#       echo "INFO: SVN build redirected to $BUILD_BASE"
   #disabled#    fi
   #disabled# fi
fi

# If custom CVSROOT is supplied, use it override default setting
# If CVSROOT is set to dynamically changing CVS_SNAPSHOT, let it 
# be inherited from environment
if echo "${GMAKEFLAGS}" | grep -q "CVSROOT="; then

   export VCTOOL=cvs
   FIND_BUILD_ERRORS="$FIND_BUILD_ERRORS -cvs"

   export CVSROOT=$(echo "$GMAKEFLAGS" | fmt -1 | grep CVSROOT | awk -F= '{print $2}')
   if echo $CVSROOT | grep -q CVS_SNAPSHOT; then
        echo "Overriding CVSROOT to $CVS_SNAPSHOT"
        export CVSROOT="$CVS_SNAPSHOT"
        # Replace CVS_SNAPSHOT with actual value of CVS_SNAPSHOT in GMAKEFLAGS
        GMAKEFLAGS=$(echo "${GMAKEFLAGS}" | sed -e "s%CVS_SNAPSHOT%$CVS_SNAPSHOT%g")
        # If build submitted for CVN_SNASHOT, forcefully redirect it to
        # SVN_TRANSITION/CVS folder
        if [ -n "$BUILD_BASE" ]; then
           if echo $BUILD_BASE | egrep -qv "SVN_TRANSITION/CVS"; then
                BUILD_BASE="${BUILD_BASE}/SVN_TRANSITION/CVS"
                echo "INFO: SVN CVS_SNAPSHOT build redirected to $BUILD_BASE"
           fi
        fi
   fi
fi

BRANDS_REQUESTED=(${BRANDS[*]})

## build_config.sh centralizes the listing of brands across all branches
## and platforms. Default brands for this script comes from it
build_config_net=/projects/hnd/software/gallery/src/tools/build/build_config.sh
if [ -f "${build_config_net}" ]; then
   build_config=${build_config_net}
else
   build_config=`dirname $0`/build_config.sh
fi

show_old_builds=/projects/hnd/software/gallery/src/tools/build/show_old_builditers.pl
if [ ! -f "${show_old_builds}" ]; then
   show_old_builds=`dirname $0`/show_old_builditers.pl
fi

cfgec=0
if [ "${TAG}" == "" -a "${BRANDS[*]}" == "" ]; then
   if echo "${GMAKEFLAGS}" | grep -q "DAILY_BUILD=1"; then
      BRANDS=($(${build_config} -r DAILY_TOT -p netbsd 2> ${NULL})); cfgec=$?
      GMAKEFLAGS="`echo $GMAKEFLAGS | sed -e 's/DAILY_BUILD=1//g'`"
      echo "INFO: Daily TOT BRANDS = ${BRANDS[*]}"
   else
      BRANDS=($(${build_config} -r TRUNK -p netbsd 2> ${NULL})); cfgec=$?
   fi
elif [ "${TAG}" != "" -a "${BRANDS[*]}" == "" ]; then
   BRANDS=($(${build_config} -r ${TAG} -p netbsd 2> ${NULL})); cfgec=$?
fi

echo "Derived   BRANDS = ${BRANDS[*]}"
echo "Rrquested BRANDS_REQUESTED = ${BRANDS_REQUESTED[*]}"

if [ "${BRANDS_REQUESTED[*]}" == "" -a "${BRANDS[*]}" == "" ] || [ "${BRANDS_REQUESTED[*]}" == "" -a "$cfgec" != "0" ]; then
        echo "ERROR:"
        echo "ERROR: Default list of build brands can't be generated"
        echo "ERROR: or no brands were specified on command line with -b <brand>"
        echo "ERROR: Can't continue. Exiting"
        echo "ERROR:"
        exit $cfgec
fi

# Reset pseudo tags to their original values
if [ "$TAG" == "TOTUTF" ]; then
   unset TAG
fi

if echo "${TAG}" | grep -q -i "_BRANCH_\|_TWIG_"; then
   if [ ! -n "${BUILDS_TO_KEEP}" ]; then
      BUILDS_TO_KEEP=${BUILDS_TO_KEEP_TOB}
      echo "INFO: Set default build keep count to ${BUILDS_TO_KEEP_TOB} builds"
   fi
fi

if [ "${TAG}" == "" -o "${TAG}" == "HEAD" ]; then
   if [ ! -n "${BUILDS_TO_KEEP}" ]; then
      BUILDS_TO_KEEP=${BUILDS_TO_KEEP_TOT}
      echo "INFO: Set default build keep count to ${BUILDS_TO_KEEP_TOT} builds"
   fi
fi

if [ "${CVSCUTOFF}" != "" -a "${TAG}" != "" ]; then
   if echo "${TAG}" | grep -q -v "_BRANCH_\|_TWIG_\|HEAD"; then
      echo "ERROR: -cvscutoff and cvstag are mutually exclusive"
      usage
   fi
fi

if [ "${CVSCUTOFF}" == "nightly" ]; then
   CVSCUTOFF=`date '+%Y-%m-%d 00:00'`
   SVNCUTOFF="`date '+%Y-%m-%d 00:00'`"
fi

if echo "${SVNCUTOFF}" | egrep -q '^[[:digit:]][[:digit:]]:[[:digit:]][[:digit:]]$'; then
   SVNCUTOFF="`date "+%Y-%m-%d $SVNCUTOFF"`"
fi

if [ "${CVSCUTOFF}" == "now" -o "${CVSCUTOFF}" == "fix" ]; then
   CVSCUTOFF="`date '+%Y-%m-%d %H:%M:%S'`"
   SVNCUTOFF="`date '+%Y-%m-%d %H:%M:%S'`"
fi

# If -c specified a SVN revision fetch it and reset cutoff to null
if echo "${SVNCUTOFF}" | egrep -q "^(r|)[[:digit:]]+$"; then
   SVNREV=$(echo $SVNCUTOFF | sed -e 's/^r//g')
   unset SVNCUTOFF
   unset CVSCUTOFF
fi

# For every brand specified
if [ "${HOSTOS}" == "Linux" ]; then
   ITERLOOP="0 $((${#BRANDS[*]}-1))"
else
   ITERLOOP="${#BRANDS[*]} 0"
fi

for i in $(${ITERCMD} ${ITERLOOP}) ; do
    BRAND=${BRANDS[${i}]}
    # echo "BRAND[${i}] = $BRAND"
done

if [ "${OPTIND}" -le "$#" ]; then
    echo "ERROR: Some command line options are missing to $0"
    usage
fi

if [ "${BUILD_BASE}" = "" ]; then
    echo "ERROR: BUILD_BASE = ${BUILD_BASE} not defined or not found"
    usage
fi

if [ ${#MAILTO} -ne 0 ]; then
    BUILDSTATUS="-u ${MAILTO}"
else
    BUILDSTATUS="-o ${NULL}"
fi

# Submit myself to LSF
if [ "${LSF}" != "none" -a "${HOSTOS}" == "Linux" ] ; then

    if [ "${LSFUSER}" = "" ]; then
        echo "ERROR: Can't determine current user?"
        exit 1
    fi

    # Quote all arguments
    while [ "$1" ] ; do
        args="${args} \"${1}\""
        shift
    done
    # Submit myself to the batch daemon.
    # The -J option creates as many jobs as there are brands.
    setsubmit
    if [ -n "${SUBMIT}" ]; then
        ${SUBMIT} /tools/bin/bsub -R r32bit -L /bin/bash ${BUILDSTATUS} -q ${LSF} -J"${LSFUSER}[1-${#BRANDS[*]}]" "$(which $0)" ${args} -q none
    else
        eval $(echo /tools/bin/bsub -R r32bit -L /bin/bash ${BUILDSTATUS} -q ${LSF} -J"${LSFUSER}[1-${#BRANDS[*]}]" "$(which $0)" ${args} -q none)
    fi
    exit $?
fi

# if srcdir specified, get absolute path and generate custom suffix
if [ -n "$OVERRIDE" ]; then
    if ! (cd $OVERRIDE); then 
	echo "ERROR: $(hostname): Cannot get to srcdir $OVERRIDE"
	exit 1
    fi
    OVERRIDE=$(cd $OVERRIDE && pwd)
    if ! cvs status -l $OVERRIDE > ${NULL}; then
	echo "ERROR: $(hostname): $OVERRIDE not a CVS checkout?"
	exit 1
    fi
    #OVSUFFIX=${OVERRIDE//\//_} OVSUFFIX=${OVSUFFIX/#_/-custom-}
    OVSUFFIX="-private-build"
else
    OVSUFFIX=
fi

# once submitted, "build_netbsd.sh -q none(in bsub)" leads to here. Build one by one
if [ "${HOSTOS}" == "Linux" -a -n "${LSB_JOBINDEX}" ] ; then
    i=$((${LSB_JOBINDEX}-1))
    BRANDS=(${BRANDS[${i}]})
fi

if [ -n "$PVT_MAKEFILE" ] && [ $((${#BRANDS[*]}-1)) -gt 1 ]; then
   echo "ERROR: '-p <private-makefile>' can't be used with '${BRANDS[*]}'"
   echo "ERROR: Specify only one build brand with '-b <build-brand>'"
   exit 1
fi

if [ -n "$PVT_MAKEFILE" ] && [ ! -s ${PVT_MAKEFILE} ]; then
   echo "ERROR: $(hostname): Private makefile '${PVT_MAKEFILE}' not found."
   exit 1
fi

# Set up environment variables in case not running as hwnbuild
setenviron

for i in $(${ITERCMD} ${ITERLOOP}) ; do
    BRAND=${BRANDS[${i}]}

    # Format version string like epivers.sh
    yyyy=$(date '+%Y')
    mm=$(date '+%m')
    dd=$(date '+%d')
    m=${mm/#0/}
    d=${dd/#0/}

    # Make base directory
    BUILD_DIR="${BUILD_BASE}/${TAG:-NIGHTLY}"
    BRAND_DIR="${BUILD_DIR}/${BRAND}"
    BUILD_DIR="${BUILD_DIR}/${BRAND}"
    if ! mkdir -p "${BUILD_DIR}" ; then
	echo "ERROR: $(hostname): Creation of ${BUILD_DIR} failed"
	continue
    fi

    # search new build directory and delete previous fail builds
    # try up to 20 times
    if [ "${HOSTOS}" == "Linux" ]; then
       BUILDTRY_LOOP="0 20"
    else
       BUILDTRY_LOOP="20 0"
    fi
    iteration=0
    for j in $(${ITERCMD} ${BUILDTRY_LOOP}) ; do
	BUILD_DCHECK="${BUILD_DIR}/${yyyy}.${m}.${d}.${j}${OVSUFFIX}"
	if [ -e "${BUILD_DCHECK}" ]
	then
	        #echo "Dir ${BUILD_DIR}/${yyyy}.${m}.${d}.${j}${OVSUFFIX} exists"
		iteration=$(( j + 1 ))
	fi

	# delete previous fail build
	if [ -e "${BUILD_DCHECK}/${RLSLOG}" ]
	then
        	log="${BUILD_DCHECK}/${RLSLOG}"
        	errorlog="${BUILD_DCHECK}/${ERRORLOG}"
		if [ -f "${errorlog}" ]
		then
		        #echo "Deleting Failed ${BUILD_DCHECK} dir"
			rm -rf "${BUILD_DCHECK}"
		fi
	fi

	[ "$j" = "20" ] && break
    done

    BUILD_DIR="${BUILD_DIR}/${yyyy}.${m}.${d}.${iteration}${OVSUFFIX}"

    # Make and enter new build directory
    if ((${iteration} > 20)) ; then
	echo "$(hostname): Creation of ${BUILD_DIR} failed (exceeded iterations)"
	continue
    else
	mkdir "${BUILD_DIR}" > ${NULL} 2>&1
	if [ "$?" != "0" ]; then
	    echo "$(hostname): Creation of ${BUILD_DIR} failed"
	    continue
	fi
    fi
    cd "${BUILD_DIR}"
    mkdir -p ${MISC_DIR}

    if [ "$PVT_MAKEFILE" -a -s "$PVT_MAKEFILE" ]; then
       rm -f release.mk
       echo "Copying custom $PVT_MAKEFILE to release.mk"
       cp $PVT_MAKEFILE release.mk
    else
       # Forward compatibility with GUB - mark the revision of each build with a file.
       if [[ -n "$SVNCUTOFF" ]]; then
          atpath=$MISC_DIR/AT_$(echo $SVNCUTOFF$(date +%z) | tr -d ': -')
          touch $atpath ||:
       fi

       # If version control tool requested is svn, then get release 
       # makefile from SVN
       if [ "$VCTOOL" == "svn" ]; then
          echo "$SVNCMD export -q ${REPO_URL}/src/tools/release/${BRAND}.mk"
          $SVNCMD export -q "${REPO_URL}/src/tools/release/${BRAND}.mk" release.mk
          vctoolrc=$?
       else
          echo "$VCTOOL -Q co ${TAG:+-r $TAG} ${SVNCUTOFF:+-D \"$SVNCUTOFF\"} -p \"src/tools/release/${BRAND}.mk\""
          $VCTOOL -Q co ${TAG:+-r $TAG} ${SVNCUTOFF:+-D "$SVNCUTOFF"} -p "src/tools/release/${BRAND}.mk" > release.mk
          vctoolrc=$?
       fi
    fi

    if [ "$vctoolrc" != "0" ] ; then
       echo "ERROR: $(hostname): Checkout of src/tools/release/${BRAND}.mk failed"
            continue
    fi

    if [ -n "$OVERRIDE" ] && [ -f ${OVERRIDE}/tools/release/${BRAND}.mk ]; then
        echo "Copy private ${OVERRIDE}/tools/release/${BRAND}.mk -> release.mk"
        cp ${OVERRIDE}/tools/release/${BRAND}.mk release.mk
    	if [ "$?" != "0" ] ; then
	    echo "$(hostname): Copy of ${OVERRIDE}/tools/release/${BRAND}.mk failed"
	    continue
    	fi
    fi

    # Echo status information
    echo "PWD=${PWD}"
    echo "TAG=${TAG}"
    [ -n "$OVERRIDE" ] && echo "OVERRIDE=${OVERRIDE}"
    echo "BRAND=${BRAND}"
    echo "$(hostname): Build started at $(date)"
    echo "BUILD_DIR = ${BUILD_DIR}"
    echo ""

    # Build information
    echo "Building ${BRAND}"                 >> ${RLSLOG}
    echo "START_TIME      = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}
    echo "BUILD_DIR       = ${BUILD_DIR}"    >> ${RLSLOG}
    echo "BUILD_BASE      = ${BUILD_BASE}"   >> ${RLSLOG}
    echo "BUILD_HOSTOS    = ${HOSTOS}"       >> ${RLSLOG}
    echo "BUILD_HOST      = ${HOSTNAME}"     >> ${RLSLOG}
    echo "BUILD_USER      = ${USER}"         >> ${RLSLOG}
    echo "TAG             = ${TAG}"          >> ${RLSLOG}
    if [ "${SVNCUTOFF}" != "" ]; then
       echo "SVN CUTOFF      = ${SVNCUTOFF}"
       echo "SVN CUTOFF      = ${SVNCUTOFF} ${SVNTZ}" >> ${RLSLOG}
    fi
    if [ "${GMAKEFLAGS}" != "" ]; then
       echo "GMAKEFLAGS      = ${GMAKEFLAGS}"
       echo "GMAKEFLAGS      = ${GMAKEFLAGS}">> ${RLSLOG}
    fi
    if [ -s "${PVT_MAKEFILE}" ]; then
       echo "*PVT MAKEFILE   = ${PVT_MAKEFILE}" >> ${RLSLOG}
    fi
    if [ "${EXTRA_LOG_STATUS}" != "" ]; then
       echo "-----------------------------------------"
       echo "EXTRA LOG STATUS:"
       echo "${EXTRA_LOG_STATUS}"
    fi

    echo ""                                  >> ${RLSLOG}
    env | sort | awk -F= '{printf "%20s = %-s\n",$1,$2}' > ${ENVLOG}
    echo "--------------------------------------------" >> ${ENVLOG}
    echo "Make version used: `which gmake 2> ${NULL}`"  >> ${ENVLOG}
    gmake --version                                     >> ${ENVLOG}
    echo "--------------------------------------------" >> ${ENVLOG}
    grep BUILD_CONFIG_VERSION $build_config_net         >> ${ENVLOG}
    echo "SCRIPT VERSION = $SCRIPT_VERSION"             >> ${ENVLOG}
    echo "--------------------------------------------" >> ${ENVLOG}


    touch $WARN_FILE
    gmake -f "release.mk" ${TAG:+TAG=$TAG} BRAND=${BRAND} \
	${OVERRIDE:+OVERRIDE=$OVERRIDE} \
	${SVNREV:+SVNREV=$SVNREV}  \
	${SVNCUTOFF:+SVNCUTOFF="$SVNCUTOFF"}  \
	${GMAKEFLAGS:+$GMAKEFLAGS} >> ${RLSLOG} 2>&1

    # Record the final exit code from master gmake step
    makerc=$?

    # For external and mfgtest builds, see if any .svn folders are still
    # remaining in src or release folders
    if echo "$BRAND" | egrep -i "external|mfgtest"; then
       find src -type d -name ".svn" -print > ${MISC_DIR}/src_dot_svn_found.log 2> $NULL
       find release -type d -name ".svn" -print > ${MISC_DIR}/release_dot_svn_found.log 2> $NULL
       tar_packages=$(find release -type d -name "*.tar" -o -name "*.tar.gz" -print)
       for pkg in $tar_packages
       do
           if echo $pkg | egrep -q ".tar.gz"; then
              if tar tzf $pkg | egrep -q "\.svn"; then
                 echo "Package $pkg contains .svn folders" ${MISC_DIR}/tar_dot_svn_found.log
              fi
           else
              if tar tf $pkg | egrep -q "\.svn"; then
                 echo "Package $pkg contains .svn folders" ${MISC_DIR}/targz_dot_svn_found.log
              fi
           fi
       done

       # Notify Build Admin if any .svn folders sneak in
       if [ -s "${MISC_DIR}/src_dot_svn_found.log" -o -s "${MISC_DIR}/release_dot_svn_found.log" -o -s "${MISC_DIR}/tar_dot_svn_found.log" -o -s "${MISC_DIR}/targz_dot_svn_found.log" ]; then
          cat ${MISC_DIR}/src_dot_svn_found.log \
              ${MISC_DIR}/release_dot_svn_found.log \
              ${MISC_DIR}/tar_dot_svn_found.log \
              ${MISC_DIR}/targz_dot_svn_found.log 2> $NULL | col -b | \
          mail -s "WARN: .svn folders found in $BUILD_DIR" $ADMINMAILTO
       fi
    fi

    echo "MAKE_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    # If a user explicitly asks for CVS build and new checkout-rules haven't
    # been included to create _CVS_BUILD flag file, let build script create 
    # those flag files to easily identify builds
    if echo "${GMAKEFLAGS}" | grep -q "CVSROOT="; then
       if [ ! -f "_CVS_BUILD" ]; then 
          echo "INFO: Marking $TAG $BRAND as a CVS build explicitly"
          echo "$CVSROOT" > _CVS_BUILD
       fi 
    fi

    # Echo status information
    if [ "$makerc" != "0" ] ; then
	echo "$(hostname): Build FAILED at $(date) with $makerc exit code"
	[ -d "release" ] && touch release/$WARN_FILE
    else
        touch ,succeeded
        rm -f $WARN_FILE
        # If central release makefile has yet been transitioned, then
        # svn build will still checkout with hndcvs. Mark those instances
        # as build failure. _SUBVERSION_BUILD or _CVS_SNAPSHOT_BUILD are
        # created svn checkout rules and must be present for svn builds
        if [ "$VCTOOL" == "svn" -a ! -f "_SUBVERSION_BUILD" -a ! -f "_GCLIENT_BUILD" ]; then
           rm -f ,succeeded
           touch $WARN_VCTOOL_FILE
        fi
        if echo "$CVSROOT" | egrep -q -i "cvs_snapshot"; then
           if [ ! -f "_CVS_SNAPSHOT_BUILD" ]; then
              rm -f ,succeeded
              touch $WARN_VCTOOL_FILE
           fi
        fi

        ## Sometimes make exits with '0', even when there are errors
        errors=$(grep "Error [0-9]\+[[:space:]]*$" ${RLSLOG} | wc -l | sed -e 's/[[:space:]]//g')
        if [ "${errors}" != "0" ] ; then
           echo "$(hostname): Build succeeded with ERRORS IGNORED at $(date)"
	   [ -d "release" ] && touch release/$IGNORE_FILE
	   touch $IGNORE_FILE
        else
           if [ -f "$WARN_VCTOOL_FILE" ]; then
              echo "$(hostname): WARN: Built with wrong cvs or svn checkout"
           else
              echo "$(hostname): Build succeeded at $(date)"
           fi
        fi
    fi
    
    # Build failed
    if [ ! -f ",succeeded" ] ; then
        # Strip out cluttered lengthy build path prefixes when generating
        # error messages. Build email/message and RLSLOG has that info already.
        BDIR=$(pwd | sed -e 's/\//./g' -e 's/\\/./g').
        BDIR_UX=$(pwd -P| sed -e 's/\//./g' -e 's/\\/./g').
        # Generate ${ERRORLOG} for use by build_summary script
        rm -f ${ERRORLOG}
        echo "${TAG:-TOT} $BRAND full build log at: ${RLSLOG}" > ${ERRORLOG}
        echo "Filtered Errors:"                                >> ${ERRORLOG}
        echo ""                                                >> ${ERRORLOG} 
        ${FIND_BUILD_ERRORS} -p netbsd ${TAG:+-r $TAG} ${RLSLOG} | \
                perl -ne "s/$BDIR//gi; s/$BDIR_UX//gi; print" | \
                perl -ne 's/^(\d+)[:-]/[$1] /g; print' >> ${ERRORLOG}

        # If find_build_errors can't filter out errors, when the build
        # fails, show last 15 lines from ,release.log
        found_errors=$(cat $ERRORLOG)
        if [ ! "$found_errors" ]; then
           tail -15 $RLSLOG | \
                perl -ne "s/$BDIR//gi; s/$BDIR_UX//gi; print" | \
                perl -ne 's/^(\d+)[:-]/[$1] /g; print' >> ${ERRORLOG}
        fi

        if [ -s "${DONGLE_ERRORLOG}" ]; then
           echo -e "\n  === Start: Dongle Build Errors ===\n" >> ${ERRORLOG}
           cat  "${DONGLE_ERRORLOG}"                            >> ${ERRORLOG}
           echo -e "\n  === End  : Dongle Build Errors ===\n" >> ${ERRORLOG}
        fi

        echo "Build Errors: "
        if [ -s "${ERRORLOG}" ]; then
           unix2dos ${ERRORLOG} > ${NULL} 2>&1
           cat "${ERRORLOG}" | col -b
        fi
    fi

    echo "ANALYSIS_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    # Default build keep counts are  $BUILDS_TO_KEEP_TOT or BUILDS_TO_KEEP_TOB
    # If current build fails, then keep count is extended automatically below
    if [ "${BUILDS_TO_KEEP}" != "" ] ; then 
       echo "Old Builds To delete:" > ${MOVELOG}
       if [ ! -f ",succeeded" ]; then
          BUILDS_TO_KEEP=$(expr ${BUILDS_TO_KEEP} + $(expr ${BUILDS_TO_KEEP} / 2))
          echo "Build preserve time (extended): ${BUILDS_TO_KEEP}" >> ${MOVELOG}
       else
          echo "Build preserve time : ${BUILDS_TO_KEEP}" >> ${MOVELOG}
       fi

       oldbuilds="$(perl ${show_old_builds} -keep_count ${BUILDS_TO_KEEP} -brand_dir ${BRAND_DIR})"
       for bld in ${oldbuilds}; do
           if echo $bld | egrep -q -i -v "/${TAG:-NIGHTLY}/${BRAND}/"; then
              echo "WARN: $bld not a build" >> ${MOVEOG}
              continue;
           fi
           echo "`date '+%Y-%m-%d %H:%M:%S'` Start Delete: $bld" >> ${MOVELOG}
           rm -rf ${bld} >> ${MOVELOG} 2>&1
           echo "`date '+%Y-%m-%d %H:%M:%S'` End   Delete: $bld" >> ${MOVELOG}
       done
    fi

    echo "OLD_BUILDS_REMOVED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    ## Preserve critical files for longer time than usual nightly recycle duration
    if echo ${TAG:-NIGHTLY} | grep -i -q "_BRANCH_\|_TWIG_\|NIGHTLY"; then
       if [ "${USER}" == "hwnbuild" -a -d "${BUILD_BASE}/PRESERVED" -a -f "${PRESERVE_LIST}" ]; then
          preservedir="${BUILD_BASE}/PRESERVED"
          preservedir="${preservedir}/${TAG:-NIGHTLY}"
          preserved_brand_flag=0
          PRESERVELOG=${preservedir}/,preserve.log

          # Remove old preserved build items
          if [ -d "${preservedir}/${BRAND}" ]; then
             rm -rf $(find ${preservedir}/${BRAND} -maxdepth 1 -mindepth 1 ! -mtime -${PRESERVE_CTIME})
          fi

          preservedir="${preservedir}/${BRAND}"
          preservedir="${preservedir}/${yyyy}.${m}.${d}.${iteration}"

          for keep in $(cat ${PRESERVE_LIST} | sed -e "s/[[:space:]]//g" | egrep -v "^#" | egrep "build_netbsd:${BRAND}:")
          do
              preserved_brand_flag=1
              mkdir -p ${preservedir}
              ## PRESERVERLIST format build_netbsd : BRAND : file_to_preserve
              file=$(echo $keep | awk -F: '{print $3}') 
              echo "cp $file ${preservedir}/$file" >> ${PRESERVELOG}
              (tar cpf - $file | ( cd $preservedir; tar xpf - )) >> ${PRESERVELOG} 2>&1
          done
          if [ "$preserved_brand_flag" != "0" ]; then
              [ -f "$RLSLOG" ] && cp -v ${RLSLOG} $preservedir
              [ -f "${ERRORLOG}" ] && cp -v ${ERRORLOG} $preservedir
              [ -f "${IGNORE_ERRORLOG}" ] && cp -v ${IGNORE_ERRORLOG} $preservedir
              [ -f ",succeeded" ] && cp -v ,succeeded $preservedir
              [ -f "${ENVLOG}" ] && cp -v ${ENVLOG} $preservedir
              [ -f "${DONGLE_RLSLOG}" ] && cp -v ${DONGLE_RLSLOG} $preservedir
              [ -f "${DONGLE_ERRORLOG}" ] && cp -v ${DONGLE_ERRORLOG} $preservedir
              gzip -f -r -q -9 $preservedir
              echo "==================================================" >> ${PRESERVEDIR}
          fi
       fi
    fi

    echo "CRITICAL_FILES_PRESERVED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    ## Resetting permissions on $BUILD_DIR to read-only
    echo "Resetting permissions on $BUILD_DIR to read-only"
    if [ "${HOSTOS}" == "Linux" ]; then
       find $BUILD_DIR -perm +22 ! -type l -depth -print | xargs -l1 -r chmod go-w > ${NULL} 2>&1
    else
       find $BUILD_DIR \( -perm -g+w -o -perm -o+w \) ! -type l -depth -print | xargs -n1 -t chmod go-w > ${NULL} 2>&1
    fi

    ## Preserve log files indefinitely
    logsdir="${BUILD_BASE}/LOGS";
    if [ "${LSFUSER}" == "hwnbuild" -a -d "${logsdir}" ]; then
       logsdir="$logsdir/${BUILD_DIR#$BUILD_BASE}"
       echo "Copying build logs to $logsdir" >> ${RLSLOG}

       # create the log directory
       mkdir -p $logsdir

       # copy the logs
       cp -p _* $logsdir 2>/dev/null
       cp -p profile.log $logsdir 2>/dev/null
       cp -p cfiles $logsdir 2>/dev/null
       cp -p hfiles $logsdir 2>/dev/null
       cp -p *.mk $logsdir 2>/dev/null
       cp -p mogrify $logsdir 2>/dev/null
       cp -p list $logsdir 2>/dev/null
       cp -p dongle_image_temp_build_spawned $logsdir 2>/dev/null

       echo "BUILD_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG} 
    
       # copy release log after build complete message is written to it 
       cp -p ,* $logsdir 2>/dev/null 
    else 
       echo "BUILD_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG} 
    fi

done
