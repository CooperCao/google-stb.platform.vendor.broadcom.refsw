#!/bin/bash
#
# MacOS top level build script
# This script f called from macos native host
#
# $Copyright (C) 2004 Broadcom Corporation
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
TAG=

PRESERVE_LIST="${HOME}/src/tools/release/preserve_built_objects.txt"
# temporarily double preserve peroid to last 4 months
PRESERVE_CTIME=120
MISC_DIR=misc
# List of logs that can be created by build scripts and release makefiles
RLSLOG=,release.log
ERRORLOG=,build_errors.log
IGNORE_ERRORLOG=,build_errors_ignored.log
MAILLOG=,mail.log
MOVELOG=,move.log
ENVLOG=,env.log
DONGLE_RLSLOG=,dongle_image_release.log
DONGLE_ERRORLOG=,dongle_image_errors.log
WARN_FILE=_WARNING_PARTIAL_CONTENTS_DO_NOT_USE
WARN_VCTOOL_FILE=_WARNING_WRONG_CHECKOUT_FOUND
IGNORE_FILE=_SOME_BUILD_ERRORS_IGNORED
BUILD_SERVER_SNOWLEOPARD=hndmacbuild2.sj.broadcom.com
ADMINMAILTO="hnd-software-scm-list@broadcom.com"
SVNCMD="svn --non-interactive"
SVNTZ="`date '+%z'`"
BUILD_BASE_LINUX_GCLIENT=/projects/hnd_swbuild/build_linux/GCLIENT_TRANSITION
export VCTOOL=svn

# Centralized SVN checkout rules and definitions
CHECKOUT_DEFS_CVS="checkout-defs-cvs.mk"
CHECKOUT_RULES_CVS="checkout-rules-cvs.mk"
CHECKOUT_DEFS="checkout-defs.mk"
CHECKOUT_RULES="checkout-rules.mk"

SERVER_BASE_MACOS="/projects/hnd_swbuild/build_macos"
SERVER_BASE_MACOS_MOUNT="/projects/hnd/swbuild/build_macos"
SERVER_BASE_MACOS_USERS="/projects/hnd/swbuild/build_macos/USERS"

SW_VERS_SNOWLEOPARD=10.6
# Temporary hostname until directory services/nis issues are resolved
# on Lion build servers
BUILD_SERVER_LION=hndmacbuild4.sj.broadcom.com
SW_VERS_LION=10.7
BUILD_SERVER_ZIN=hndmacbuild5.sj.broadcom.com
SW_VERS_ZIN=10.8

FIND_BUILD_ERRORS="perl /projects/hnd_software/gallery/src/tools/build/find_build_errors"

MAILTO=
HOSTOS=$(uname -s)
NULL=/dev/null

ITERCMD="jot -w %d "

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

-c cutoff_time|svnrev
	This switch takes either cutoff time or svn revision number.
	* Cvs cutoff time (yyyy-mm-dd hh:mm format or other cvs/svn formats)
	  for checking out from tot/tob (default: none)
	  For SVN current timezone is appended by default during checkout
	  ('nightly' refers to midnight and 'now' or 'fix refer current time)
	* SVN revision can be r1234 or just 1234 format

-d dir
	Base directory in which to build (OPTIONAL).
	Has to be a local directory. Use in combination with -s option
        to store the build on the server

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

-r tag    
	CVS tag to use (default: HEAD).

-s dir
	Base directory to which copy built files (typically on server)
	NOTE: When using -t option to build from a private space
	NOTE: this is your workspace directory on network

-x builds_to_keep
	Remove old builds (default: keep last ${BUILDS_TO_KEEP_TOB} TOB builds; keep last ${BUILDS_TO_KEEP_TOT} TOT builds)

-p private_makefile
	Use private release makefile instead of checking out from cvs

-t testdir
	Source directory with files to override cvs pull.  Must be a CVS
	or SVN checkout.
	NOTE: (Optionally) Along with -t <testdir>, if -f "COPYSRC=1" is
	supplied, <testdir> is just copied over as is for build

EOF
    exit 1
}

CVSROOT_PROD=/projects/cvsroot

# Set up environment variables
function setenviron {
    UNIX_PATH=/bin:/usr/bin:/usr/X11R6/bin:/sbin:/usr/sbin:/usr/X11R6/sbin:/usr/local/bin:/usr/local/sbin
    CROSSGCCLINKS=/projects/hnd/tools/linux/hndtools-x86-netbsd/bin

    # Equivalent CVS Snapshot that created above SVN Repo
    if [ -z "$CVS_SNAPSHOT" ]; then
        # This is a copy of above with svn+cvs_snapshot build fixes to makefiles
        export CVS_SNAPSHOT="/projects/hnd_swbuild_ext6_scratch/build_admin_ext6_scratch/10_cvs_snapshot_culled/2011-02-11_sj"
    fi

    CVSROOT_PROD=/projects/cvsroot
    CVSROOT_SNAPSHOT=$CVS_SNAPSHOT

    # Temporarily add linux/bin path until svn wrappers are tested
    export PATH=${UNIX_PATH}:/projects/hnd/tools/linux/bin
    if [ -z "$CVSROOT" ]; then
        export CVSROOT=":pserver:$(id -un)@cvsps-sj1-1.sj.broadcom.com:${CVSROOT_PROD}"
    fi

    # Default SVN Repo root, if not set in environment
    if [ -z "$SVNROOT" ]; then
        # WLAN Production Repository
        export SVNROOT="http://svn.sj.broadcom.com/svn/wlansvn/proj"
        # NOTE: WLAN Production Repository - but accessed from alternate server
        # WARN: Talk to SVN Admins before using this server
        # WARN: Using this server needs credentials to be cached
        # export SVNROOT="http://engcm-sj1-06.sj.broadcom.com/svn/wlansvn/proj"
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

function uniquedir () {
    local base="$1";
    local -i i=0
    while [ -d ${base}${i} ]
      do
      if (( $i < 20 ))
      then
        let i=i+1;
      else
        echo "${FUNCNAME} Giving up: cannot create unique directory in ${base}"
        return 1;
      fi;
    done
    echo ${base}${i};
    return 0;
}

# Command line switches this script takes
# WARN: Ensure that cmd line option blocks below are arranged alphabetically.
while getopts 'b:c:d:e:f:hjm:p:r:s:t:x:' OPT ; do
    case "$OPT" in
       b)
           BRANDS=(${OPTARG})
           ;;
       c)
           # Cutoff time or svn revision to use during checkout
           # CVSCUTOFF is exported into the caller's environment
           export CVSCUTOFF=${OPTARG}
           # SVNCUTOFF is exported into the caller's environment
           export SVNCUTOFF="${OPTARG}"

           if [ "${SVNCUTOFF}" == "nightly" ]; then
              NIGHTLY_CUTOFF="`date '+%Y-%m-%d 00:00'`"
              BUILD_ARGS="$BUILD_ARGS -c '$NIGHTLY_CUTOFF'"
           else
              BUILD_ARGS="$BUILD_ARGS -c '$SVNCUTOFF'"
           fi

           ;;
       d)
           # For local builds
           BUILD_BASE=${OPTARG}
           BUILD_ARGS="$BUILD_ARGS -d '$BUILD_BASE'"
           ;;
       e)
           EXTRA_LOG_STATUS=${OPTARG}
           ;;
       f)
	   GMAKEFLAGS="${OPTARG}"
           BUILD_ARGS="$BUILD_ARGS -f '$GMAKEFLAGS'"
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
           BUILD_ARGS="$BUILD_ARGS -m '$MAILTO'"
           ;;
       p)
           PVT_MAKEFILE=${OPTARG}
           BUILD_ARGS="$BUILD_ARGS -p '$PVT_MAKEFILE'"
           ;;
       r)
           TAG=${OPTARG}
           BUILD_ARGS="$BUILD_ARGS -r '$TAG'"
           ;;
       s)
           SERVER_BASE=${OPTARG}
           BUILD_ARGS="$BUILD_ARGS -s '$SERVER_BASE'"
           ;;
       t)
           OVERRIDE=${OPTARG}
           BUILD_ARGS="$BUILD_ARGS -t '$OVERRIDE'"
           ;;
       x)
           BUILDS_TO_KEEP=${OPTARG}
           BUILD_ARGS="$BUILD_ARGS -x '$BUILDS_TO_KEEP'"
           ;;
       ?)
           usage
           ;;
    esac
done

# If either build_base or server_base is not set, match them
if [ ! -n "$BUILD_BASE" -a "$SERVER_BASE" ]; then
   BUILD_BASE=$SERVER_BASE
elif [ ! -n "$SERVER_BASE" -a "$BUILD_BASE" ]; then
   SERVER_BASE=$BUILD_BASE
fi

if echo $(hostname) | grep -q -i "${BUILD_SERVER}\|${BUILD_SERVER_SNOWLEOPARD}\|${BUILD_SERVER_LION}\|${BUILD_SERVER_ZIN}"; then
   if [ -n "$OVERRIDE" -a ! -n "${SERVER_BASE}" ]; then
      SERVER_BASE=${BUILD_BASE}
      echo "INFO: Build will directly be built at override dir ${SERVER_BASE}"
   fi
fi

case "${GMAKEFLAGS}" in
     *VCTOOL=cvs*|*CVSROOT=*)
           # If custom CVSROOT is supplied, use it override default setting
           # If CVSROOT is set to dynamically changing CVS_SNAPSHOT, let it 
           # be inherited from environment
           export VCTOOL=cvs
           FIND_BUILD_ERRORS="$FIND_BUILD_ERRORS -cvs"

           export CVSROOT=$(echo "$GMAKEFLAGS" | fmt -1 | grep CVSROOT | awk -F= '{print $2}')
           ;;

     *VCTOOL=svn*)
           export VCTOOL=svn
           ;;

     *COTOOL=gclient*)
           echo "GIVEN BUILD_BASE  = $BUILD_BASE";
           echo "GIVEN SERVER_BASE = $SERVER_BASE";

           # If GMAKEFLAGS contains COTOOL=gclient, export it to env
           # to pass down to $(MAKE)
           export COTOOL=gclient
           # If build submitted for GCLIENT, forcefully redirect it to
           # GCLIENT_TRANSITION/ folder
           if [ -n "$SERVER_BASE" ]; then
              if echo $SERVER_BASE | egrep -qv "GCLIENT_TRANSITION"; then
                 if [ "$BUILD_BASE" == "$SERVER_BASE" ]; then
                    BUILD_BASE="${SERVER_BASE}/GCLIENT_TRANSITION"
                 fi
                 SERVER_BASE="${SERVER_BASE}/GCLIENT_TRANSITION"
                 BUILD_ARGS="$BUILD_ARGS -s '$SERVER_BASE'"
                 echo "INFO: GCLIENT build redirected to $SERVER_BASE"
              fi
           fi
           echo "NEW BUILD_BASE = $BUILD_BASE";
           # BUILD_LINUX_DIR is exported by DHD build into Firmware build
           # process. Separate out GCLIENT fw builds from production SVN fw
           # builds
           export BUILD_LINUX_DIR=$BUILD_BASE_LINUX_GCLIENT
           ;;

     *REPO_URL=*)
           # Extract repository URL if specified on command line flag
           export REPO_URL=$(echo "$GMAKEFLAGS" | fmt -1 | awk -F= '/REPO_URL/{print $2}')
           echo "INFO: Setting custom SVN Repository URL to $REPO_URL"
           ;;

     *DEPS_URL=*)
           # Extract repository URL if specified on command line flag
           export DEPS_URL=$(echo "$GMAKEFLAGS" | fmt -1 | awk -F= '/DEPS_URL/{print $2}')
           echo "INFO: Setting custom GCLIENT Deps URL to $DEPS_URL"
           ;;

esac # GMAKEFLAGS

BRANDS_REQUESTED=(${BRANDS[*]})

echo "BUILD_ARGS [`uname -n`]: $BUILD_ARGS"

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
      BRANDS=($(${build_config} -r DAILY_TOT -p macos 2> ${NULL})); cfgec=$?
      echo "INFO: Daily TOT BRANDS = ${BRANDS[*]}"
      GMAKEFLAGS="`echo $GMAKEFLAGS | sed -e 's/DAILY_BUILD=1//g'`"
   else
      BRANDS=($(${build_config} -r TRUNK -p macos 2> ${NULL})); cfgec=$?
   fi
elif [ "${TAG}" != "" -a "${BRANDS[*]}" == "" ]; then
   BRANDS=($(${build_config} -r ${TAG} -p macos 2> ${NULL})); cfgec=$?
fi

echo "Derived   BRANDS = ${BRANDS[*]}"
echo "Requested BRANDS_REQUESTED = ${BRANDS_REQUESTED[*]}"

if [ "${BRANDS_REQUESTED[*]}" == "" -a "$cfgec" != "0" ]; then
        echo "ERROR:"
        echo "ERROR: Default list of build brands can't be generated"
        echo "ERROR: build_config at $build_config had non-zero exit code ($cfgec)"
        echo "ERROR: Can't continue. Exiting"
        echo "ERROR:"
        exit $cfgec
fi

# Reset pseudo tags to their original value
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
   CVSCUTOFF="`date '+%Y-%m-%d 00:00'`"
   SVNCUTOFF="`date '+%Y-%m-%d 00:00'`"
fi

if echo "${SVNCUTOFF}" | egrep -q '^[[:digit:]][[:digit:]]:[[:digit:]][[:digit:]]$'; then
   SVNCUTOFF=`date "+%Y-%m-%d $SVNCUTOFF"`
fi

if [ "${CVSCUTOFF}" == "now" -o "${CVSCUTOFF}" == "fix" ]; then
   CVSCUTOFF="`date '+%Y-%m-%d %H:%M:%S'`"
   SVNCUTOFF="`date '+%Y-%m-%d %H:%M:%S'`"
fi

# If -c specified a SVN revision, fetch it and reset cutoff to null
if echo "${SVNCUTOFF}" | egrep -q "^(r|)[[:digit:]]+$"; then
   SVNREV=$(echo $SVNCUTOFF | sed -e 's/^r//g')
   unset SVNCUTOFF
   unset CVSCUTOFF
fi

if echo ${SERVER_BASE} | grep -q -i /projects/hnd; then
   WEB_URL="http://home.sj.broadcom.com"
fi

# For every brand specified
ITERLOOP="${#BRANDS[*]} 0"

for i in $(${ITERCMD} ${ITERLOOP}) ; do
    BRAND=${BRANDS[${i}]}
    echo "BRAND[${i}] = $BRAND"
done

if [ "${OPTIND}" -le "$#" ]; then
    echo "ERROR: Some command line options are missing to $0"
    usage
fi

# if srcdir specified, get absolute path and generate custom suffix
if [ -n "$OVERRIDE" ]; then
    if ! (cd $OVERRIDE); then 
	echo "ERROR: $(hostname): Cannot get to srcdir $OVERRIDE"
	exit 1
    fi
    OVERRIDE=$(cd $OVERRIDE && pwd)
    
    cvs status -l $OVERRIDE > ${NULL} 2>&1; cvs_ovstatus=$?
    svn info $OVERRIDE > ${NULL} 2>&1; svn_ovstatus=$?

    if [ "$cvs_ovstatus" != "0" -a "$svn_ovstatus" != "0" ]; then
        echo "ERROR: $(hostname): $OVERRIDE not a CVS or SVN checkout?"
        exit 1
    fi
    #OVSUFFIX=${OVERRIDE//\//_} OVSUFFIX=${OVSUFFIX/#_/-custom-}
    OVSUFFIX="-private-build"
else
    OVSUFFIX=
fi

if [ -n "$PVT_MAKEFILE" ] && [ $((${#BRANDS[*]}-1)) -gt 1 ]; then
   echo "ERROR: '-p <private-makefile>' can't be used with '${BRANDS[*]}'"
   echo "ERROR: Specify only one build brand with '-b <build-brand>'"
   exit 1
fi

if [ -n "$PVT_MAKEFILE" ] && [ ! -s ${PVT_MAKEFILE} ]; then
   echo "ERROR: $(hostname): Private makefile '${PVT_MAKEFILE}' not found or empty"
   exit 1
fi

# Set up environment variables in case not running as hwnbuild
setenviron

# Redirect user's private builds (from /proj/users area) away from prod dirs
echo "SERVER_BASE ${SERVER_BASE} == ${SERVER_BASE_MACOS} or ${SERVER_BASE_MACOS_MOUNT}"
echo "REPO_URL == ${REPO_URL}"
echo "SVNBASE  == ${SVNBASE}"

if [ "${SERVER_BASE}" == "${SERVER_BASE_MACOS}" -o "${SERVER_BASE}" == "${SERVER_BASE_MACOS_MOUNT}" ]; then
   # User's full builds are redirected to a separate scratch disk
   # they are identified by users svn area name
   if echo "${REPO_URL}" | grep -q "wlansvn/users"; then
      export BUILD_USERDIR=$(echo $REPO_URL | sed -e "s%$SVNBASE/%%" -e "s%^users/%%" -e "s%/%_%g")
      echo "BUILD_USERDIR=$BUILD_USERDIR"
      export SERVER_BASE=$SERVER_BASE_MACOS_USERS/$BUILD_USERDIR

      if ! mkdir -p "${SERVER_BASE}" ; then
         echo "ERROR: $(hostname): Creation of ${SERVER_BASE} failed"
         exit 1
      else
         chmod ug+w $SERVER_BASE
      fi
      echo "INFO: Overriding SERVER_BASE to $SERVER_BASE"
      BUILD_BASE=$SERVER_BASE
   fi # REPO_URL=wlansvn/users
fi # SERVER_BASE

for i in $(${ITERCMD} ${ITERLOOP}) ; do
    BRAND=${BRANDS[${i}]}

    if echo $BRAND | grep -i -q "barolo\|lion"; then
       sw_vers=$(sw_vers -productVersion)
       if echo $sw_vers | grep -i -q "${SW_VERS_LION}"; then
          echo "INFO: "
          echo "INFO: Running $BRAND build on Barolo or Lion ($sw_vers) system"
          echo "INFO: "
       else
          # Signals like Ctrl-C don't get propogated correctly in this model
          echo "INFO: "
          echo "INFO: Barolo or Lion brand detected, redirecting the"
          echo "INFO: $BRAND build to Barolo or Lion macos build server"
          echo "INFO: "
          build_script=$0
          if echo $build_script | grep -i -q -v "^/"; then
               build_script=`pwd`/`basename $0`
          fi
          echo ssh -l $LOGNAME $BUILD_SERVER_LION \
               $build_script "$BUILD_ARGS" -b $BRAND
          (ssh -l $LOGNAME $BUILD_SERVER_LION \
               $build_script "$BUILD_ARGS" -b $BRAND) &
          sleep 10
          continue
       fi
    fi


    if echo $BRAND | grep -i -q "zin"; then
       sw_vers=$(sw_vers -productVersion)
       if echo $sw_vers | grep -i -q "${SW_VERS_ZIN}"; then
          echo "INFO: "
          echo "INFO: Running $BRAND build on Zin ($sw_vers) system"
          echo "INFO: "
       else
          echo "INFO: "
          echo "INFO: Zin brand detected, redirecting the"
          echo "INFO: $BRAND build to Zin macos build server"
          echo "INFO: "
          build_script=$0
          if echo $build_script | grep -i -q -v "^/"; then
               build_script=`pwd`/`basename $0`
          fi
          echo ssh -l $LOGNAME $BUILD_SERVER_ZIN \
               $build_script "$BUILD_ARGS" -b $BRAND
          (ssh -l $LOGNAME $BUILD_SERVER_ZIN \
               $build_script "$BUILD_ARGS" -b $BRAND) &
          sleep 10
          continue
       fi
    fi

    # Format version string like epivers.sh
    yyyy=$(date '+%Y')
    mm=$(date '+%m')
    dd=$(date '+%d')
    m=${mm/#0/}
    d=${dd/#0/}

    # Make base directory
    BUILD_DIR="${BUILD_BASE}/${TAG:-NIGHTLY}"
    BUILD_DIR="${BUILD_DIR}/${BRAND}"

    # Rarely if macos server can't get to nfs path due to glitch, retry after some small delay
    MKDIRTRY_LOOP="5 0"
    for k in $(${ITERCMD} ${MKDIRTRY_LOOP}) ; do
        if [ ! -d "${BUILD_DIR}" ]; then
           echo "DBG: $(hostname) [$k] Trying to create non-existant ${BUILD_DIR}"
           sleep 5
           mkdir -pv "${BUILD_DIR}"
        else
           echo "DBG: $(hostname) [$k] Found pre-existing ${BUILD_DIR}"
           break
        fi
    done

    # By this time $BUILD_DIR should exist, if not it is a hard error
    if ! mkdir -p "${BUILD_DIR}" ; then
	echo "ERROR: $(hostname): Creation of ${BUILD_DIR} failed"
	continue
    fi

    # search new build directory and delete previous fail builds
    # try up to 20 times
    BUILDTRY_LOOP="20 0"
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
	mkdir -p "${BUILD_DIR}" > ${NULL} 2>&1
	if [ "$?" != "0" ]; then
	    echo "$(hostname): Creation of ${BUILD_DIR} failed"
	    continue
	fi
    fi
    cd "${BUILD_DIR}"

    if [ -n "$OVERRIDE" ] && [ -f ${OVERRIDE}/tools/release/${BRAND}.mk ]; then
        echo "Copy private ${OVERRIDE}/tools/release/${BRAND}.mk -> release.mk"
        cp -pv ${OVERRIDE}/tools/release/${BRAND}.mk release.mk
    	if [ "$?" != "0" ] ; then
	    echo "$(hostname): Copy of ${OVERRIDE}/tools/release/${BRAND}.mk failed"
	    continue
    	fi
    fi

    if [ -n "$OVERRIDE" ]; then
       # If COPYSRC is set, OVERRIDE is copied as is with no cvs patching
       # with rest of $TAG (or TOT)
       if echo $GMAKEFLAGS | egrep -i "COPYSRC=1"; then
          export SRC_CHECKOUT_DISABLED=1
          echo "Copying sources from $OVERRIDE to $BUILD_DIR"
          echo "Copying sources from $OVERRIDE to $BUILD_DIR" >> ${RLSLOG}
          cp -rp $OVERRIDE $BUILD_DIR 2>> ${RLSLOG}
          if [ "$?" != "0" ]; then
             echo "ERROR: Copying from private tree to build directory failed"
             exit 1
          fi
          if echo "$BRAND" | egrep -i "external|mfgtest"; then
             echo "Stripping .svn bits from private external build"
             find $BUILD_DIR/src -type d -name ".svn" -print0 | \
                xargs -t -L1 -0 rm -rf
          fi
          # Now set OVERRIDE to null to ensure that release makefiles
          # don't try to merge current workspace to $TAG or TOT (if no tag)
          export OVERRIDE=
       fi
    else
       if [ "$PVT_MAKEFILE" -a -s "$PVT_MAKEFILE" ]; then
          rm -f release.mk
          echo "Copying custom $PVT_MAKEFILE to release.mk"
          cp -pv $PVT_MAKEFILE release.mk
       else
          # If version control tool requested is svn, then get release 
          # makefile from SVN
          if [ "$VCTOOL" == "svn" ]; then
             echo "$SVNCMD export ${SVNCUTOFF:+-r \{'$SVNCUTOFF $SVNTZ'\}} ${SVNREV:+-r $SVNREV} ${REPO_URL}/src/tools/release/${BRAND}.mk"
             $SVNCMD export ${SVNCUTOFF:+-r \{"$SVNCUTOFF $SVNTZ"\}} ${SVNREV:+-r $SVNREV} ${REPO_URL}/src/tools/release/${BRAND}.mk release.mk
             vctoolrc=$?
          else
             $VCTOOL -Q -d $CVSROOT co -p src/tools/release/${CHECKOUT_DEFS_CVS} > ${CHECKOUT_DEFS}
             $VCTOOL -Q -d $CVSROOT co -p src/tools/release/${CHECKOUT_RULES_CVS} > ${CHECKOUT_RULES}
             echo "$VCTOOL -Q co ${TAG:+-r $TAG} ${CVSCUTOFF:+-D \"$CVSCUTOFF\"} -p \"src/tools/release/${BRAND}.mk\""
             $VCTOOL -Q co ${TAG:+-r $TAG} ${CVSCUTOFF:+-D "$CVSCUTOFF"} -p "src/tools/release/${BRAND}.mk" > release.mk
             vctoolrc=$?
          fi

          if [ "$vctoolrc" != "0" ] ; then
             echo "ERROR: $(hostname): Checkout of src/tools/release/${BRAND}.mk failed"
             continue
          fi
       fi
    fi

    # Echo status information
    echo "PWD=${PWD}"
    echo "TAG=${TAG}"
    [ -n "$OVERRIDE" ] && echo "OVERRIDE=${OVERRIDE}"
    echo "BRAND=${BRAND}"
    echo "SERVER_VERSION=$(sw_vers -productVersion)"
    echo "$(hostname): Build started at $(date)"
    echo "BUILD_DIR = ${BUILD_DIR}"
    if [ "${CVSCUTOFF}" != "" ]; then
       echo "CVS CUTOFF= ${CVSCUTOFF}"
    fi
    if [ "${SVNCUTOFF}" != "" ]; then
       echo "SVN CUTOFF= ${SVNCUTOFF}"
    fi
    if [ "${SVNREV}" != "" ]; then
       echo "SVNREV    = ${SVNREV}"
    fi
    if [ "${GMAKEFLAGS}" != "" ]; then
       echo "GMAKEFLAGS = ${GMAKEFLAGS}"
    fi
    echo ""

    # For debugging set following and it is overrides value on checkout-*.mk
    # export HNDSVN_CMD=sparse_dbg.sh

    # Build information
    echo "Building ${BRAND}"                 >> ${RLSLOG}
    echo "START_TIME      = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}
    echo "BUILD_DIR       = ${BUILD_DIR}"    >> ${RLSLOG}
    echo "BUILD_BASE      = ${BUILD_BASE}"   >> ${RLSLOG}
    echo "BUILD_HOSTOS    = ${HOSTOS}"       >> ${RLSLOG}
    echo "BUILD_HOST      = ${HOSTNAME}"     >> ${RLSLOG}
    echo "BUILD_USER      = ${USER}"         >> ${RLSLOG}
    echo "TAG             = ${TAG}"          >> ${RLSLOG}
    if [ "${CVSCUTOFF}" != "" ]; then
       echo "CVS CUTOFF      = ${CVSCUTOFF}" >> ${RLSLOG}
    fi
    if [ "${SVNCUTOFF}" != "" ]; then
       echo "SVN CUTOFF      = ${SVNCUTOFF} ${SVNTZ}" >> ${RLSLOG}
    fi
    if [ "${SVNREV}" != "" ]; then
       echo "SVNREV    = ${SVNREV}"          >> ${RLSLOG}
    fi
    if [ "${GMAKEFLAGS}" != "" ]; then
       echo "GMAKEFLAGS      = ${GMAKEFLAGS}">> ${RLSLOG}
    fi
    if [ -s "${PVT_MAKEFILE}" ]; then
       echo "*PVT MAKEFILE   = ${PVT_MAKEFILE}" >> ${RLSLOG}
    fi
    if [ "${EXTRA_LOG_STATUS}" != "" ]; then 
       echo "-----------------------------------------">> ${RLSLOG} 
       echo "EXTRA LOG STATUS:"                        >> ${RLSLOG} 
       echo "${EXTRA_LOG_STATUS}"                      >> ${RLSLOG} 
    fi 

    echo ""                                    >> ${RLSLOG}

    mkdir -pv ${MISC_DIR}

    if [ ${#MAILTO} -ne 0 ]; then
       cat ${RLSLOG} > ${MISC_DIR}/${MAILLOG}
    fi
    env | sort | awk -F= '{printf "%20s = %-s\n",$1,$2}'> ${MISC_DIR}/${ENVLOG}
    echo "PID=$$" | awk -F= '{printf "%20s = %-s\n",$1,$2}' \
                                                       >> ${MISC_DIR}/${ENVLOG}
    set | grep PID | awk -F= '{printf "%20s = %-s\n",$1,$2}' \
                                                       >> ${MISC_DIR}/${ENVLOG}
    echo "--------------------------------------------">> ${MISC_DIR}/${ENVLOG}
    echo "Make version used: `which make`"             >> ${MISC_DIR}/${ENVLOG}
    make --version                                     >> ${MISC_DIR}/${ENVLOG}
    echo "--------------------------------------------">> ${MISC_DIR}/${ENVLOG}
    grep BUILD_CONFIG_VERSION $build_config_net        >> ${MISC_DIR}/${ENVLOG}
    echo "SCRIPT VERSION = $SCRIPT_VERSION"            >> ${MISC_DIR}/${ENVLOG}
    echo "--------------------------------------------">> ${MISC_DIR}/${ENVLOG}

    touch $WARN_FILE
    # Build
    make -w -f "release.mk" ${TAG:+TAG=$TAG} \
			     ${CVSROOT:+CVSROOT="$CVSROOT"} \
			     ${OVERRIDE:+OVERRIDE=$OVERRIDE} \
			     ${CVSCUTOFF:+CVSCUTOFF="$CVSCUTOFF"} \
			     ${SVNCUTOFF:+SVNCUTOFF="$SVNCUTOFF"} \
			     ${SVNREV:+SVNREV="$SVNREV"} \
			     ${VCTOOL:+VCTOOL="$VCTOOL"} \
			     ${COTOOL:+COTOOL="$COTOOL"} \
			     ${GMAKEFLAGS:+$GMAKEFLAGS} \
			     >> ${RLSLOG} 2>&1

    makerc=$?

    # For external and mfgtest builds, see if any .svn folders are still
    # remaining in src or release folders
    if echo "$BRAND" | egrep -i "external|mfgtest"; then
       find src -type d -name ".svn" -print > ${MISC_DIR}/src_dot_svn_found.log 2> $NULL
       find release -type d -name ".svn" -print > ${MISC_DIR}/release_dot_svn_found.log 2> $NULL
       tar_packages=$(find release -type f -name "*.tar" -o -name "*.tar.gz" -print)
       for pkg in $tar_packages
       do
           pkgname=$(basename $pkg)
           if echo $pkg | egrep -q ".tar.gz"; then
              tar tzf $pkg > ${MISC_DIR}/${pkgname}.txt
              if egrep -q "\.svn" ${MISC_DIR}/${pkgname}.txt; then
                 echo "Package $pkg contains .svn folders" ${MISC_DIR}/targz_dot_svn_found.log
              fi
              rm -f ${MISC_DIR}/${pkgname}.txt
           else
              tar tf $pkg > ${MISC_DIR}/${pkgname}.txt
              if egrep -q "\.svn" ${MISC_DIR}/${pkgname}.txt; then
                 echo "Package $pkg contains .svn folders" ${MISC_DIR}/tar_dot_svn_found.log
              fi
              rm -f ${MISC_DIR}/${pkgname}.txt
           fi
       done

       # Notify Build Admin if any .svn folders sneak in when the build is successful
       if [ -s "${MISC_DIR}/src_dot_svn_found.log" -o -s "${MISC_DIR}/release_dot_svn_found.log" -o -s "${MISC_DIR}/tar_dot_svn_found.log" -o -s "${MISC_DIR}/targz_dot_svn_found.log" ]; then
          if [ -f ",succeeded" ]; then
             cat ${MISC_DIR}/src_dot_svn_found.log \
                 ${MISC_DIR}/release_dot_svn_found.log \
                 ${MISC_DIR}/tar_dot_svn_found.log \
                 ${MISC_DIR}/targz_dot_svn_found.log 2> $NULL | col -b | \
             mail -s "ERROR: .svn folders found at $BUILD_DIR" $ADMINMAILTO
          fi
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
        buildresult="(FAILED)"
        [ -d "release" ] && touch release/$WARN_FILE
    else
        touch ,succeeded
        rm -f $WARN_FILE

        ## Sometimes make exits with '0', even when there are errors
        errors=$(grep "Error [0-9]\+[[:space:]]*$" ${RLSLOG} | wc -l | sed -e 's/[[:space:]]//g')
        if [ "${errors}" != "0" ] ; then
           echo "$(hostname): Build succeeded with ERRORS IGNORED at $(date)"
           buildresult="(ERRORS IGNORED)"
           [ -d "release" ] && touch release/$IGNORE_FILE
           touch $IGNORE_FILE
        else
           if [ -f "$WARN_VCTOOL_FILE" ]; then
              echo "$(hostname): WARN: Built with wrong cvs or svn checkout"
           else
              echo "$(hostname): Build succeeded at $(date)"
           fi
           buildresult=""
        fi
    fi

    if [ -n "${MAILTO}" ]; then
       echo "" >> ${MISC_DIR}/${MAILLOG}
       echo "RELEASE LOG = ${WEB_URL}/${BUILD_DIR}/${RLSLOG}" >> ${MISC_DIR}/${MAILLOG}
       echo "" >> ${MISC_DIR}/${MAILLOG}
    fi
    # Build failed
    if [ ! -f ",succeeded" ]; then
        # Strip out cluttered lengthy build path prefixes when generating
        # error messages. Build email/message and RLSLOG has that info already.
        BDIR=$(pwd | sed -e 's/\//./g' -e 's/\\/./g').
        BDIR_UX=$(pwd -P| sed -e 's/\//./g' -e 's/\\/./g').
        # Generate ${ERRORLOG} for use by build_summary script
        rm -f ${ERRORLOG}
        echo "${TAG:-TOT} $BRAND full build log at: ${RLSLOG}" > ${ERRORLOG}
        echo "Filtered Errors:"                                >> ${ERRORLOG}
        echo ""                                                >> ${ERRORLOG} 
        ${FIND_BUILD_ERRORS} -p macos ${TAG:+-r $TAG} ${RLSLOG} | \
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
           echo -e "\n  === Start: Dongle Build Errors ===\n"  >> ${ERRORLOG}
           cat   "${DONGLE_ERRORLOG}"                            >> ${ERRORLOG}
           echo -e "\n  ==== End  : Dongle Build Errors ===\n" >> ${ERRORLOG}
        fi

        echo "Build Errors: "
        if [ -s "${ERRORLOG}" ]; then
           unix2dos ${ERRORLOG} > ${NULL} 2>&1
           cat "${ERRORLOG}" | col -b
        fi

        if [ -n "${MAILTO}" ]; then
           echo "Build failed!" >> ${MISC_DIR}/${MAILLOG}
           cat ${ERRORLOG}      >> ${MISC_DIR}/${MAILLOG}
        fi
    fi

    echo "ANALYSIS_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    # Default build keep counts are  $BUILDS_TO_KEEP_TOT or BUILDS_TO_KEEP_TOB
    # If current build fails, then keep count is extended automatically below
    if [ "${BUILDS_TO_KEEP}" != "" ]; then 
       echo "Old Builds To delete:" >> ${MISC_DIR}/${MOVELOG}
       if [ ! -f ",succeeded" ]; then
          BUILDS_TO_KEEP=$(expr ${BUILDS_TO_KEEP} + $(expr ${BUILDS_TO_KEEP} / 2))
          echo "Build preserve time (extended): ${BUILDS_TO_KEEP}" >> ${MISC_DIR}/${MOVELOG}
       else
          echo "Build preserve time : ${BUILDS_TO_KEEP}"           >> ${MISC_DIR}/${MOVELOG}
       fi
       oldbuilds_local="$(perl ${show_old_builds} -keep_count ${BUILDS_TO_KEEP} -brand_dir ${BRAND_DIR})"
       for bld in ${oldbuilds_local}; do
           if echo $bld | egrep -q -i -v "/${TAG:-NIGHTLY}/${BRAND}/"; then
              echo "WARN: '$bld' not a build" >> ${MOVEOG}
              continue;
           fi
           echo "`date '+%Y-%m-%d %H:%M:%S'` Start Delete: $bld" >> ${MISC_DIR}/${MOVELOG}
           rm -rf ${bld} >> ${MISC_DIR}/${MOVELOG} 2>&1
           echo "`date '+%Y-%m-%d %H:%M:%S'` End   Delete: $bld" >> ${MISC_DIR}/${MOVELOG}
       done
       if [ "${SERVER_BASE}" -a -d "${SERVER_BASE}" ]; then
          oldbuilds_server="$(perl ${show_old_builds} -keep_count ${BUILDS_TO_KEEP} -brand_dir ${SERVER_BASE}/${TAG:-NIGHTLY}/${BRAND})"
          for bld in ${oldbuilds_server}; do
              if [ ! -d "$bld" ]; then
                 echo "WARN: $bld not a build" >> ${MOVEOG}
                 continue;
              fi
              echo "`date '+%Y-%m-%d %H:%M:%S'` Start Delete: $bld" >> ${MISC_DIR}/${MOVELOG}
              rm -rf ${bld} >> ${MISC_DIR}/${MOVELOG} 2>&1
              echo "`date '+%Y-%m-%d %H:%M:%S'` End   Delete: $bld" >> ${MISC_DIR}/${MOVELOG}
          done
       fi
    fi

    echo "OLD_BUILDS_REMOVED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    ## Preserve critical files for longer time than usual nightly recycle duration
    if echo ${TAG:-NIGHTLY} | grep -i -q "_BRANCH_\|_TWIG_\|NIGHTLY"; then
       if [ "${LOGNAME}" == "hwnbuild" -a -d "${SERVER_BASE}/PRESERVED" -a -f ${PRESERVE_LIST} ]; then
          preservedir="${SERVER_BASE}/PRESERVED"
          preservedir="${preservedir}/${TAG:-NIGHTLY}"
          preserved_brand_flag=0
          PRESERVELOG=${preservedir}/,preserve.log

          # Remove old preserved build items
          if [ -d "${preservedir}/${BRAND}" ]; then
             rm -rf $(find ${preservedir}/${BRAND} -maxdepth 1 -mindepth 1 -not -mtime -${PRESERVE_CTIME})
             echo "rm -rf $(find ${preservedir}/${BRAND} -maxdepth 1 -mindepth 1 -not -mtime -${PRESERVE_CTIME})" >> ${PRESERVELOG}
          fi

          preservedir="${preservedir}/${BRAND}"
          preservedir="${preservedir}/${yyyy}.${m}.${d}.${iteration}"
          for keep in $(cat ${PRESERVE_LIST} | sed -e "s/[[:space:]]//g" | egrep -v "^#" | egrep "build_macos:${BRAND}:")
          do
              preserved_brand_flag=1
              mkdir -pv ${preservedir}
              ## PRESERVERLIST format build_macos : BRAND : file_to_preserve
              file=$(echo $keep | awk -F: '{print $3}') 
              echo "cp $file ${preservedir}/$file" >> ${PRESERVELOG}
              (tar cpf - $file | ( cd $preservedir; tar xpf - )) >> ${PRESERVELOG} 2>&1
          done
          if [ "$preserved_brand_flag" != "0" ]; then
              [ -f "$RLSLOG" ] && cp -v ${RLSLOG} $preservedir
              [ -f "${ERRORLOG}" ] && cp -v ${ERRORLOG} $preservedir
              [ -f "${IGNORE_ERRORLOG}" ] && cp -v ${IGNORE_ERRORLOG} $preservedir
              [ -f ",succeeded" ] && cp -v ,succeeded $preservedir
              [ -f "${MISC_DIR}/${ENVLOG}" ] && cp -v ${MISC_DIR}/${ENVLOG} $preservedir
              [ -f "${DONGLE_RLSLOG}" ] && cp -v ${DONGLE_RLSLOG} $preservedir
              [ -f "${DONGLE_ERRORLOG}" ] && cp -v ${DONGLE_ERRORLOG} $preservedir
              gzip -f -r -q -9 $preservedir
              echo "==================================================" >> ${PRESERVELOG}
          fi
       fi
    fi

    echo "CRITICAL_FILES_PRESERVED = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG}

    localdir=${BUILD_DIR}
    serverdir=$localdir
    ## Copy over to server directory
    if [ "${BUILD_BASE}" != "${SERVER_BASE}" -a -d "${SERVER_BASE}" ]; then
       serverdir=$(uniquedir "${SERVER_BASE}/${TAG:-NIGHTLY}/${BRAND}/${yyyy}.${m}.${d}.")
       echo "Copying from $localdir -to- $serverdir" >> ${MISC_DIR}/${MOVELOG}
       mkdir -p ${serverdir}
       cd $localdir
       echo "tar cf - ./* | ( cd $serverdir; tar xvf - )" >> ${MISC_DIR}/${MOVELOG}
       tar cf - ./* | ( cd $serverdir; tar xf - ) >> ${MISC_DIR}/${MOVELOG} 2>&1
       ## Resetting permissions on $serverdir to read-only
       echo "Resetting permissions on $serverdir to read-only"
       find $serverdir \( -perm -g+w -o -perm -o+w \) ! -type l -depth -print | xargs -n1 -t chmod go-w > ${NULL} 2>&1
    fi

    

    ## Preserve log files indefinitely
    logsdir="$SERVER_BASE/LOGS"
    if [ -d "${logsdir}" ]; then
       logsdir="$logsdir/${serverdir#$SERVER_BASE}"
       echo "Copying build logs to $logsdir"

       # create the log directory
       mkdir -pv $logsdir

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

       echo "Resetting permissions on $logsdir to read-only"
       find $logsdir \( -perm -g+w -o -perm -o+w \) ! -type l -depth -print | xargs -n1 -t chmod go-w > ${NULL} 2>&1
    else 
       echo "BUILD_COMPLETE = $(date '+%Y-%m-%d %H:%M:%S')" >> ${RLSLOG} 
    fi
 
    # If caller has stripped domain name append it as
    # snowleopard mac systems don't relay emails correctly
    # to broadcom mailhost without fully qualified domain name
    for rcpt in $MAILTO; do
        if echo $rcpt | grep -vqi "@"; then
           RCPTS="$RCPTS $rcpt@broadcom.com"
        else
           RCPTS="$RCPTS $rcpt"
        fi
    done

    if [ -n "${MAILTO}" -a -s "${MISC_DIR}/${MAILLOG}" ]; then
       mailsubject="Build $buildresult for ${TAG:-NIGHTLY}:${BRAND} done on $(hostname)"
       mail -s "$mailsubject" ${RCPTS} < ${MISC_DIR}/${MAILLOG}
 
    fi

done # for BRANDS
